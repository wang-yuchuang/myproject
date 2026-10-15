/**
 * @file flexcan_lld.c
 * @low level sdrv flexcan driver file.
 *
 * @Copyright (c) 2021  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#include <debug.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "flexcan_lld.h"

/***************Internal function prototypes***************/

static inline flexcan_status_t flexcan_lld_enter_freeze_mode(CAN_Type *base,
                                                             bool allowTimeout);
static inline flexcan_status_t flexcan_lld_exit_freeze_mode(CAN_Type *base,
                                                            bool allowTimeout);
static inline void flexcan_lld_disable_mem_err_detection(CAN_Type *base);
static inline void flexcan_lld_fd_clean_smb_region(CAN_Type *base);
static void flexcan_lld_fd_init(CAN_Type *base,
                                const flexcan_fd_config_t *config);
static inline bool flexcan_lld_is_fd_enabled(CAN_Type *base);
static uint32_t *flexcan_lld_get_msg_buf_addr(CAN_Type *base, uint8_t msgBufId);
static uint8_t flexcan_lld_compute_dlc_val(uint8_t payloadSize);
static uint8_t flexcan_lld_compute_payload_len(uint8_t dlc_val);
static void flexcan_lld_copy_from_mb(uint32_t *addr, uint8_t *data,
                                     uint8_t len);
static void flexcan_lld_copy_to_mb(uint32_t *addr, uint8_t *data, uint8_t len);
static inline void flexcan_lld_copy8(uint32_t *src, uint32_t *dest);
static inline void flexcan_lld_copy16(uint32_t *src, uint32_t *dest);
static inline void flexcan_lld_copy32(uint32_t *src, uint32_t *dest);
static inline bool flexcan_lld_ext_nominal_bit_timing(
                             const flexcan_timing_config_t *nominal_bit_timing);

/***************Internal variables***************/

/***************Function implementation***************/

/* Internal function implementation start */

static inline flexcan_status_t flexcan_lld_enter_freeze_mode(CAN_Type *base,
                                                             bool allowTimeout)
{
    ASSERT(base);

    uint32_t timeoutCnt = FLEXCAN_TIMEOUT_COUNTER;
    flexcan_status_t retVal = FLEXCAN_SUCCESS;

    /* Set Freeze, Halt bits. */
    base->MCR |= CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK;

    /* Wait until the FlexCAN Module enter freeze mode. */
    while ((!(base->MCR & CAN_MCR_FRZACK_MASK)) && (timeoutCnt > 0U)) {
        if (allowTimeout) {
            timeoutCnt--;
        }
    }

    /* Return failed status if timeout. */
    if (!(base->MCR & CAN_MCR_FRZACK_MASK)) {
        ssdk_printf(SSDK_INFO, "\nEnter freeze mode timeout\n");
        retVal = FLEXCAN_FAIL;
    }

    return retVal;
}

static inline flexcan_status_t flexcan_lld_exit_freeze_mode(CAN_Type *base,
                                                            bool allowTimeout)
{
    ASSERT(base);

    uint32_t timeoutCnt = FLEXCAN_TIMEOUT_COUNTER;
    flexcan_status_t retVal = FLEXCAN_SUCCESS;

    /* Clear Freeze, Halt bits. */
    base->MCR &= ~(CAN_MCR_FRZ_MASK | CAN_MCR_HALT_MASK);

    /* Wait until the FlexCAN Module exit freeze mode. */
    while ((base->MCR & CAN_MCR_FRZACK_MASK) && (timeoutCnt > 0U)) {
        if (allowTimeout) {
            timeoutCnt--;
        }
    }

    /* Return failed status if timeout. */
    if (base->MCR & CAN_MCR_FRZACK_MASK) {
        ssdk_printf(SSDK_INFO, "\nExit freeze mode timeout\n");
        retVal = FLEXCAN_FAIL;
    }

    return retVal;
}

static inline void flexcan_lld_disable_mem_err_detection(CAN_Type *base)
{
    ASSERT(base);

    /* Enable write of MECR register */
    base->CTRL2 |= CAN_CTRL2_ECRWRE_MASK;
    /* Enable write of MECR */
    base->MECR &= ~CAN_MECR_ECRWRDIS_MASK;
    /* Disable Error Detection and Correction mechanism,
     * that will set CAN in Freez Mode in case of trigger */
    base->MECR &= ~CAN_MECR_NCEFAFRZ_MASK;
    /* Disable write of MECR */
    base->CTRL2 |= CAN_CTRL2_ECRWRE_MASK;
}

static inline void flexcan_lld_fd_clean_smb_region(CAN_Type *base)
{
    ASSERT(base);

    uint16_t start = CAN_FD_SMB_START_ADDR_OFFSET;

    /* Enable unrestricted write access to FlexCAN memory. */
    base->CTRL2 |= CAN_CTRL2_WRMFRZ_MASK;

    /* Clear CAN FD SMB region to avoid non-correctable errors. */
    while (start < CAN_FD_SMB_END_ADDR_OFFSET) {
        *((uint32_t *)((uint8_t *)base + start)) = 0U;
        start += 4U;
    }

    /* Enable write access restriction. */
    base->CTRL2 &= ~CAN_CTRL2_WRMFRZ_MASK;
}

static void flexcan_lld_fd_init(CAN_Type *base,
                                const flexcan_fd_config_t *config)
{
    ASSERT(base && config);

    uint32_t fdctrlTemp = base->FDCTRL;

    /* Enable Bit Rate Switch? */
    if (config->enableBRS) {
        fdctrlTemp |= CAN_FDCTRL_FDRATE_MASK;
    }
    else {
        fdctrlTemp &= ~CAN_FDCTRL_FDRATE_MASK;
    }

    /* Enable Transceiver Delay Compensation? */
    if (config->enableTDC) {
        fdctrlTemp |= CAN_FDCTRL_TDCEN_MASK;
        /* Set TDC offset. */
        fdctrlTemp &= ~CAN_FDCTRL_TDCOFF_MASK;
        fdctrlTemp |= CAN_FDCTRL_TDCOFF(config->TDCOffset);
    }
    else {
        fdctrlTemp &= ~CAN_FDCTRL_TDCEN_MASK;
    }

    /* Set message buffer data size for region 0. */
    fdctrlTemp &= ~CAN_FDCTRL_MBDSR0_MASK;
    fdctrlTemp |= CAN_FDCTRL_MBDSR0(config->r0_mb_data_size);

#if FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER > 32

    /* Set message buffer data size for region 1. */
    fdctrlTemp &= ~CAN_FDCTRL_MBDSR1_MASK;
    fdctrlTemp |= CAN_FDCTRL_MBDSR1(config->r1_mb_data_size);

#endif

#if FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER > 64

    /* Set message buffer data size for region 2. */
    fdctrlTemp &= ~CAN_FDCTRL_MBDSR2_MASK;
    fdctrlTemp |= CAN_FDCTRL_MBDSR2(config->r2_mb_data_size);

#endif

#if FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER > 96

    /* Set message buffer data size for region 3. */
    fdctrlTemp &= ~CAN_FDCTRL_MBDSR3_MASK;
    fdctrlTemp |= CAN_FDCTRL_MBDSR3(config->r3_mb_data_size);

#endif

    /* Update FDCTRL register. */
    base->FDCTRL = fdctrlTemp;

    /* Enable ISO CAN FD? */
    if (config->enableISOCANFD) {
        base->CTRL2 |= CAN_CTRL2_ISOCANFDEN_MASK;
    }
}

static inline bool flexcan_lld_is_fd_enabled(CAN_Type *base)
{
    ASSERT(base);

    return (((base->MCR & CAN_MCR_FDEN_MASK) >> CAN_MCR_FDEN_SHIFT) != 0U);
}

static uint32_t *flexcan_lld_get_msg_buf_addr(CAN_Type *base, uint8_t msgBufId)
{
    ASSERT(base);

    uint32_t *msgBufAddr;

    if (flexcan_lld_is_fd_enabled(base)) {
        uint8_t msgBufRegionIdx;
        uint8_t fdctrl_mbdsrx;
        uint8_t payloadSize;
        uint8_t msgBufSize;
        uint8_t regionMaxMBNum;

        ssdk_printf(SSDK_DEBUG, "CAN base 0x%x MB %d ", (unsigned int)base, msgBufId);
        msgBufAddr = (uint32_t *)base->MB;

        for (msgBufRegionIdx = 0U; msgBufRegionIdx < REGION_NUM;
             msgBufRegionIdx++) {
            /* Get MBDSRx bits from FDCTRL register. */
            fdctrl_mbdsrx =
                (((base->FDCTRL) >> (16U + msgBufRegionIdx * 3U)) & 3U);
            /* Get message buffer data size in bytes. */
            payloadSize = 1U << (fdctrl_mbdsrx + 3U);
            /* Get message buffer size in words. */
            msgBufSize = (payloadSize + 8U) >> 2U;
            /* Maxium MB index in the region. */
            if (fdctrl_mbdsrx == 0U) {
                regionMaxMBNum = REGION_8BYTES_MB_NUM;
            }
            else if (fdctrl_mbdsrx == 1U) {
                regionMaxMBNum = REGION_16BYTES_MB_NUM;
            }
            else if (fdctrl_mbdsrx == 2U) {
                regionMaxMBNum = REGION_32BYTES_MB_NUM;
            }
            else {
                regionMaxMBNum = REGION_64BYTES_MB_NUM;
            }

            if (msgBufId < regionMaxMBNum) {
                msgBufAddr += (msgBufId * msgBufSize);
                break;
            }
            else {
                msgBufAddr += PER_REGION_SIZE_IN_WORD;
                msgBufId -= regionMaxMBNum;
            }
        }
    }
    else {
        msgBufAddr = (uint32_t *)&base->MB[msgBufId];
    }

    ssdk_printf(SSDK_DEBUG, "adress = 0x%x\n", (unsigned int)msgBufAddr);
    return msgBufAddr;
}

static uint8_t flexcan_lld_compute_dlc_val(uint8_t payloadSize)
{
    uint8_t ret_dlc_val = 0xFFU;
    static const uint8_t payload_code[65] = {
        0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U,
        /* 9 to 12 payload have DLC Code 12 Bytes */
        CAN_DLC_VALUE_12_BYTES, CAN_DLC_VALUE_12_BYTES, CAN_DLC_VALUE_12_BYTES,
        CAN_DLC_VALUE_12_BYTES,
        /* 13 to 16 payload have DLC Code 16 Bytes */
        CAN_DLC_VALUE_16_BYTES, CAN_DLC_VALUE_16_BYTES, CAN_DLC_VALUE_16_BYTES,
        CAN_DLC_VALUE_16_BYTES,
        /* 17 to 20 payload have DLC Code 20 Bytes */
        CAN_DLC_VALUE_20_BYTES, CAN_DLC_VALUE_20_BYTES, CAN_DLC_VALUE_20_BYTES,
        CAN_DLC_VALUE_20_BYTES,
        /* 21 to 24 payload have DLC Code 24 Bytes */
        CAN_DLC_VALUE_24_BYTES, CAN_DLC_VALUE_24_BYTES, CAN_DLC_VALUE_24_BYTES,
        CAN_DLC_VALUE_24_BYTES,
        /* 25 to 32 payload have DLC Code 32 Bytes */
        CAN_DLC_VALUE_32_BYTES, CAN_DLC_VALUE_32_BYTES, CAN_DLC_VALUE_32_BYTES,
        CAN_DLC_VALUE_32_BYTES, CAN_DLC_VALUE_32_BYTES, CAN_DLC_VALUE_32_BYTES,
        CAN_DLC_VALUE_32_BYTES, CAN_DLC_VALUE_32_BYTES,
        /* 33 to 48 payload have DLC Code 48 Bytes */
        CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES,
        CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES,
        CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES,
        CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES,
        CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES, CAN_DLC_VALUE_48_BYTES,
        CAN_DLC_VALUE_48_BYTES,
        /* 49 to 64 payload have DLC Code 64 Bytes */
        CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES,
        CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES,
        CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES,
        CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES,
        CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES, CAN_DLC_VALUE_64_BYTES,
        CAN_DLC_VALUE_64_BYTES};

    if (payloadSize <= 64U) {
        ret_dlc_val = payload_code[payloadSize];
    }
    else {
        /* The argument is not a valid payload size,
           so return invalid value 0xFF. */
    }

    return ret_dlc_val;
}

static uint8_t flexcan_lld_compute_payload_len(uint8_t dlc_val)
{
    uint8_t ret_payload_len = 0U;

    if (dlc_val <= 8U) {
        ret_payload_len = dlc_val;
    }
    else if (dlc_val == CAN_DLC_VALUE_12_BYTES) {
        ret_payload_len = 12U;
    }
    else if (dlc_val == CAN_DLC_VALUE_16_BYTES) {
        ret_payload_len = 16U;
    }
    else if (dlc_val == CAN_DLC_VALUE_20_BYTES) {
        ret_payload_len = 20U;
    }
    else if (dlc_val == CAN_DLC_VALUE_24_BYTES) {
        ret_payload_len = 24U;
    }
    else if (dlc_val == CAN_DLC_VALUE_32_BYTES) {
        ret_payload_len = 32U;
    }
    else if (dlc_val == CAN_DLC_VALUE_48_BYTES) {
        ret_payload_len = 48U;
    }
    else if (dlc_val == CAN_DLC_VALUE_64_BYTES) {
        ret_payload_len = 64U;
    }
    else {
        /* Do nothing. */
    }

    return ret_payload_len;
}

static void flexcan_lld_copy_from_mb(uint32_t *addr, uint8_t *data, uint8_t len)
{
    uint8_t temp_buf[64];

    if (len <= 8U) {
        flexcan_lld_copy8(addr, (uint32_t *)temp_buf);
    }
    else if (len <= 16U) {
        flexcan_lld_copy16(addr, (uint32_t *)temp_buf);
    }
    else if (len <= 32U) {
        flexcan_lld_copy32(addr, (uint32_t *)temp_buf);
    }
    else {
        flexcan_lld_copy32(addr, (uint32_t *)temp_buf);
        flexcan_lld_copy32(addr + 8U, (uint32_t *)(temp_buf + 32U));
    }

    memcpy(data, temp_buf, len);
}

static void flexcan_lld_copy_to_mb(uint32_t *addr, uint8_t *data, uint8_t len)
{
    if (len <= 8U) {
        flexcan_lld_copy8((uint32_t *)data, addr);
    }
    else if (len <= 16U) {
        flexcan_lld_copy16((uint32_t *)data, addr);
    }
    else if (len <= 32U) {
        flexcan_lld_copy32((uint32_t *)data, addr);
    }
    else {
        flexcan_lld_copy32((uint32_t *)data, addr);
        flexcan_lld_copy32((uint32_t *)(data + 32U), addr + 8U);
    }
}

static inline void flexcan_lld_copy8(uint32_t *src, uint32_t *dest)
{
    __ASM volatile("ldmia %0, {r4-r5} \n"
#if CORE_LITTLE_ENDIAN
                     "rev r4, r4        \n"
                     "rev r5, r5        \n"
#endif
                     "stmia %1, {r4-r5}   "
                     :
                     : "r"(src), "r"(dest)
                     : "r4", "r5", "memory");
}

static inline void flexcan_lld_copy16(uint32_t *src, uint32_t *dest)
{
    __ASM volatile("ldmia %0, {r4-r7} \n"
#if CORE_LITTLE_ENDIAN
                     "rev r4, r4        \n"
                     "rev r5, r5        \n"
                     "rev r6, r6        \n"
                     "rev r7, r7        \n"
#endif
                     "stmia %1, {r4-r7}   "
                     :
                     : "r"(src), "r"(dest)
                     : "r4", "r5", "r6", "r7", "memory");
}

static inline void flexcan_lld_copy32(uint32_t *src, uint32_t *dest)
{
    __ASM volatile("ldmia %0, {r4-r11}\n"
#if CORE_LITTLE_ENDIAN
                     "rev r4, r4        \n"
                     "rev r5, r5        \n"
                     "rev r6, r6        \n"
                     "rev r7, r7        \n"
                     "rev r8, r8        \n"
                     "rev r9, r9        \n"
                     "rev r10, r10      \n"
                     "rev r11, r11      \n"
#endif
                     "stmia %1, {r4-r11}  "
                     :
                     : "r"(src), "r"(dest)
                     : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11",
                       "memory");
}

static inline bool flexcan_lld_ext_nominal_bit_timing(
                             const flexcan_timing_config_t *nominal_bit_timing)
{
    if ((nominal_bit_timing->preDivider - 1U > 0xFFU) ||
        (nominal_bit_timing->rJumpwidth - 1U > 3U) ||
        (nominal_bit_timing->propSeg - 1U > 7U) ||
        (nominal_bit_timing->phaseSeg1 - 1U > 7U) ||
        (nominal_bit_timing->phaseSeg2 - 1U > 7U)) {
        return true;
    }
    else {
        return false;
    }
}

/* Internal function implementation end */

/***************API Function implementation***************/

/* API function implementation start */

flexcan_status_t flexcan_lld_init(CAN_Type *base, const flexcan_config_t *config)
{
    ASSERT(base && config);

    uint32_t mcrTemp;
    uint32_t ctrl1Temp;
    flexcan_status_t retVal = FLEXCAN_SUCCESS;

    /* Assertion. */
    ASSERT(config);
    ASSERT((config->maxMbNum > 0) &&
           (config->maxMbNum <= FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER));

#if CONFIG_SDRV_FLEXCAN_ENH
    /* operate soft reset of SemidDrive enhancements. */
    base->SOFT_RESET = 0x0;
#endif

    /* Reset to known status.*/
    flexcan_lld_reset(base);

    /* Check if FlexCAN Module already Enabled before calling Module_Config. */
    if (!(base->MCR & CAN_MCR_MDIS_MASK)) {
        retVal = flexcan_lld_enable(base, false);
    }

    if (retVal != FLEXCAN_SUCCESS)
        return retVal;

    /* Protocol-Engine clock source selection, This bit must be set
     * when FlexCAN Module in Disable Mode.
     */
    if (FLEXCAN_ClkSrcOsc == config->clkSrc) {
        base->CTRL1 &= ~CAN_CTRL1_CLKSRC_MASK;
    }
    else {
        base->CTRL1 |= CAN_CTRL1_CLKSRC_MASK;
    }

    /* Enable FlexCAN Module before configuration. */
    retVal = flexcan_lld_enable(base, true);
    if (retVal != FLEXCAN_SUCCESS)
        return retVal;

    /* Enter Fraze Mode. */
    retVal = flexcan_lld_enter_freeze_mode(base, false);
    if (retVal != FLEXCAN_SUCCESS)
        return retVal;

    /* Save current MCR value. */
    mcrTemp = base->MCR;
    /* Save current CTRL1 value. */
    ctrl1Temp = base->CTRL1;

    /* Set the maximum number of Message Buffers */
    mcrTemp =
        (mcrTemp & ~CAN_MCR_MAXMB_MASK) | CAN_MCR_MAXMB(config->maxMbNum - 1);

    /* Disable busoff automatic recovering. */
    ctrl1Temp |= CAN_CTRL1_BOFFREC_MASK;

    /* Enable Loop Back Mode? */
    if (config->enableLoopBack) {
        ctrl1Temp &= ~CAN_CTRL1_LOM_MASK;
        ctrl1Temp |= CAN_CTRL1_LPB_MASK;
        mcrTemp &= ~CAN_MCR_SRXDIS_MASK;
    }
    else {
        ctrl1Temp &= ~CAN_CTRL1_LPB_MASK;
        /* Disable the self reception feature when FlexCAN is not in loopback
         * mode. */
        mcrTemp |= CAN_MCR_SRXDIS_MASK;
        /* Enable Listen-Only Mode? */
        if (config->enableListenOnly) {
            ctrl1Temp |= CAN_CTRL1_LOM_MASK;
        } else {
            ctrl1Temp &= ~CAN_CTRL1_LOM_MASK;
        }
    }

    /* Enable Self Wake Up Mode? */
    if (config->enableSelfWakeup) {
        mcrTemp |= CAN_MCR_SLFWAK_MASK;
    }
    else {
        mcrTemp &= ~CAN_MCR_SLFWAK_MASK;
    }

    /* Enable Individual Rx Masking? */
    if (config->enableIndividMask) {
        mcrTemp |= CAN_MCR_IRMQ_MASK;
    }
    else {
        mcrTemp &= ~CAN_MCR_IRMQ_MASK;
    }

#if FLEXCAN_HAS_DOZE_MODE_SUPPORT

    /* Enable Doze Mode? */
    if (config->enableDoze) {
        mcrTemp |= CAN_MCR_DOZE_MASK;
    }
    else {
        mcrTemp &= ~CAN_MCR_DOZE_MASK;
    }

#endif

    /* Enable CAN FD operation? */
    if (config->enableCANFD) {
        mcrTemp |= CAN_MCR_FDEN_MASK;
        flexcan_lld_fd_init(base, &config->can_fd_cfg);
    }
    else {
        mcrTemp &= ~CAN_MCR_FDEN_MASK;
        ctrl1Temp |= CAN_CTRL1_SMP_MASK;
    }

    /* Save CTRL1 Configuration. */
    base->CTRL1 = ctrl1Temp;
    /* Save MCR Configuation. */
    base->MCR = mcrTemp;

    /* Baud Rate Configuration.*/
    if (config->enableCANFD) {
        flexcan_lld_fd_set_timing_config(base, &config->nominalBitTiming,
                                         &config->dataBitTiming);
    }
    else {
        flexcan_lld_classic_set_timing_config(base, &config->nominalBitTiming);
    }

    /* Re-freeze FlexCAN Module after Config finish,
     * because it may be unfreezed during config.
     * It's upper layer software's responsibility
     * to unfreeze FlexCAN when ready to take part
     * in communication on the bus.
     */
    retVal = flexcan_lld_enter_freeze_mode(base, true);
    if (retVal != FLEXCAN_SUCCESS)
        return retVal;

    return retVal;
}

flexcan_status_t flexcan_lld_enable(CAN_Type *base, bool enable)
{
    ASSERT(base);

    uint32_t timeoutCnt = FLEXCAN_TIMEOUT_COUNTER;
    flexcan_status_t retVal = FLEXCAN_SUCCESS;

    if (enable) {
        base->MCR &= ~CAN_MCR_MDIS_MASK;

        /* Wait FlexCAN exit from low-power mode. */
        while ((base->MCR & CAN_MCR_LPMACK_MASK) && (timeoutCnt > 0)) {
            timeoutCnt--;
        }

        if (base->MCR & CAN_MCR_LPMACK_MASK) {
            retVal = FLEXCAN_FAIL;
            ssdk_printf(SSDK_INFO, "\nExit low-power mode timeout\n");
        }
    }
    else {
        base->MCR |= CAN_MCR_MDIS_MASK;

        /* Wait FlexCAN enter low-power mode. */
        while (!(base->MCR & CAN_MCR_LPMACK_MASK) && (timeoutCnt > 0)) {
            timeoutCnt--;
        }

        if (!(base->MCR & CAN_MCR_LPMACK_MASK)) {
            retVal = FLEXCAN_FAIL;
            ssdk_printf(SSDK_INFO, "\nEnter low-power mode timeout\n");
        }
    }

    return retVal;
}

flexcan_status_t flexcan_lld_freeze(CAN_Type *base, bool freeze)
{
    ASSERT(base);

    flexcan_status_t retVal;

    if (freeze) {
        retVal = flexcan_lld_enter_freeze_mode(base, true);
    }
    else {
        retVal = flexcan_lld_exit_freeze_mode(base, true);
    }

    return retVal;
}

void flexcan_lld_reset(CAN_Type *base)
{
    ASSERT(base);

    uint8_t i;
    bool moduleEnabled;

    /* The module must should be first exit from low power
     * mode, and then soft reset can be applied.
     */
    if (base->MCR & CAN_MCR_MDIS_MASK) {
        moduleEnabled = false;
        flexcan_lld_enable(base, true);
    }
    else {
        /* Enter Disable mode and then exit,
         * to force exiting from any low power mode.
         */
        flexcan_lld_enable(base, false);
        flexcan_lld_enable(base, true);
        moduleEnabled = true;
    }

#if (FLEXCAN_HAS_DOZE_MODE_SUPPORT != 0)
    /* De-ASSERT DOZE Enable Bit. */
    base->MCR &= ~CAN_MCR_DOZE_MASK;
#endif

    /* Wait until FlexCAN exit from any Low Power Mode. */
    while (base->MCR & CAN_MCR_LPMACK_MASK) {}

    /* Assert Soft Reset Signal. */
    base->MCR |= CAN_MCR_SOFTRST_MASK;

    /* Wait until FlexCAN reset completes. */
    while (base->MCR & CAN_MCR_SOFTRST_MASK) {}

    /* Reset MCR rigister. */
#if FLEXCAN_HAS_GLITCH_FILTER
    base->MCR |= CAN_MCR_WRNEN_MASK | CAN_MCR_WAKSRC_MASK |
                 CAN_MCR_MAXMB(FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER - 1);
#else
    base->MCR |= CAN_MCR_WRNEN_MASK |
                 CAN_MCR_MAXMB(FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER - 1);
#endif

    /* Reset CTRL1 and CTRL2 rigister. */
    base->CTRL1 = 0U;
    base->CTRL2 =
        CAN_CTRL2_TASD(0x16) | CAN_CTRL2_RRS_MASK | CAN_CTRL2_EACEN_MASK;

    /* Clean all individual Rx Mask of Message Buffers. */
    for (i = 0; i < FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER; i++) {
        base->RXIMR[i] = 0x3FFFFFFF;
    }

    /* Enable unrestricted write access to FlexCAN memory. */
    base->CTRL2 |= CAN_CTRL2_WRMFRZ_MASK;

    /* Clean Global Mask of Message Buffers. */
    base->RXMGMASK = 0x3FFFFFFF;
    /* Clean Global Mask of Message Buffer 14. */
    base->RX14MASK = 0x3FFFFFFF;
    /* Clean Global Mask of Message Buffer 15. */
    base->RX15MASK = 0x3FFFFFFF;
    /* Clean Global Mask of Rx FIFO. */
    base->RXFGMASK = 0x3FFFFFFF;

    /* Reset FDCTRL register. */
    base->FDCTRL = 0x80000000;

    /* Clean Message Buffer region. */
    for (i = 0; i < FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER; i++) {
        base->MB[i].CS = 0x0;
        base->MB[i].ID = 0x0;
        base->MB[i].WORD0 = 0x0;
        base->MB[i].WORD1 = 0x0;
    }

    /* avoid ecc error, clean reserved area 96Byte */
    for(uint8_t i=0; i<24; i++)
    {
        *(((uint32_t *)&base->RESERVED_4[0])+i) = 0x0U;
    }

    /* Clean CAN FD SMB region. */
    flexcan_lld_fd_clean_smb_region(base);

    /* Enable write access restriction. */
    base->CTRL2 &= ~CAN_CTRL2_WRMFRZ_MASK;

    if (!moduleEnabled) {
        flexcan_lld_enable(base, false);
    }
}

void flexcan_lld_classic_set_timing_config(
    CAN_Type *base, const flexcan_timing_config_t *config)
{
    /* Assertion. */
    ASSERT(config && base);

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    if (!flexcan_lld_ext_nominal_bit_timing(config)) {
        base->CBT &= ~CAN_CBT_BTF_MASK;

        /* Cleaning previous Timing Setting. */
        base->CTRL1 &= ~(CAN_CTRL1_PRESDIV_MASK | CAN_CTRL1_RJW_MASK |
                         CAN_CTRL1_PSEG1_MASK | CAN_CTRL1_PSEG2_MASK |
                         CAN_CTRL1_PROPSEG_MASK);

        /* Updating Timing Setting according to configuration structure. */
        base->CTRL1 |= (CAN_CTRL1_PRESDIV(config->preDivider - 1U) |
                        CAN_CTRL1_RJW(config->rJumpwidth - 1U) |
                        CAN_CTRL1_PROPSEG(config->propSeg - 1U) |
                        CAN_CTRL1_PSEG1(config->phaseSeg1 - 1U) |
                        CAN_CTRL1_PSEG2(config->phaseSeg2 - 1U));
    }
    else {
        /* Cleaning previous arbitration phase Timing Setting. */
        base->CBT &=
            ~(CAN_CBT_EPRESDIV_MASK | CAN_CBT_ERJW_MASK | CAN_CBT_EPSEG1_MASK |
              CAN_CBT_EPSEG2_MASK | CAN_CBT_EPROPSEG_MASK);

        /* Updating arbitration phase Timing Setting according to configuration
         * structure. */
        base->CBT |= (CAN_CBT_BTF(1U) | /* Use CBT instead of CTRL1. */
                      CAN_CBT_EPRESDIV(config->preDivider - 1U) |
                      CAN_CBT_ERJW(config->rJumpwidth - 1U) |
                      CAN_CBT_EPROPSEG(config->propSeg - 1U) |
                      CAN_CBT_EPSEG1(config->phaseSeg1 - 1U) |
                      CAN_CBT_EPSEG2(config->phaseSeg2 - 1U));
    }

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_fd_set_timing_config(
    CAN_Type *base, const flexcan_timing_config_t *arbitrPhaseConfig,
    const flexcan_timing_config_t *dataPhaseConfig)
{
    /* Assertion. */
    ASSERT(base && arbitrPhaseConfig && dataPhaseConfig);

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    /* Cleaning previous arbitration phase Timing Setting. */
    base->CBT &=
        ~(CAN_CBT_EPRESDIV_MASK | CAN_CBT_ERJW_MASK | CAN_CBT_EPSEG1_MASK |
          CAN_CBT_EPSEG2_MASK | CAN_CBT_EPROPSEG_MASK);

    /* Updating arbitration phase Timing Setting according to configuration
     * structure. */
    base->CBT |= (CAN_CBT_BTF(1U) | /* Use CBT instead of CTRL1. */
                  CAN_CBT_EPRESDIV(arbitrPhaseConfig->preDivider - 1U) |
                  CAN_CBT_ERJW(arbitrPhaseConfig->rJumpwidth - 1U) |
                  CAN_CBT_EPROPSEG(arbitrPhaseConfig->propSeg - 1U) |
                  CAN_CBT_EPSEG1(arbitrPhaseConfig->phaseSeg1 - 1U) |
                  CAN_CBT_EPSEG2(arbitrPhaseConfig->phaseSeg2 - 1U));

    /* Cleaning previous data phase Timing Setting. */
    base->FDCBT &= ~(CAN_FDCBT_FPRESDIV_MASK | CAN_FDCBT_FRJW_MASK |
                     CAN_FDCBT_FPSEG1_MASK | CAN_FDCBT_FPSEG2_MASK |
                     CAN_FDCBT_FPROPSEG_MASK);

    /* Updating data phase Timing Setting according to configuration structure.
     */
    base->FDCBT |= (CAN_FDCBT_FPRESDIV(dataPhaseConfig->preDivider - 1U) |
                    CAN_FDCBT_FRJW(dataPhaseConfig->rJumpwidth - 1U) |
                    CAN_FDCBT_FPROPSEG(dataPhaseConfig->propSeg) |
                    CAN_FDCBT_FPSEG1(dataPhaseConfig->phaseSeg1 - 1U) |
                    CAN_FDCBT_FPSEG2(dataPhaseConfig->phaseSeg2 - 1U));

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_activate_can_fd(CAN_Type *base, bool enable)
{
    ASSERT(base);

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    /* Activate/De-activate CAN FD operation
       according to the value of enable. */
    if (enable) {
        base->MCR |= CAN_MCR_FDEN_MASK;
    }
    else {
        base->MCR &= ~CAN_MCR_FDEN_MASK;
    }

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_set_rx_mb_global_mask(CAN_Type *base, uint32_t mask)
{
    ASSERT(base);

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    /* Setting Global Mask value. */
    base->RXMGMASK = mask;
    base->RX14MASK = mask;
    base->RX15MASK = mask;

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_set_rx_fifo_global_mask(CAN_Type *base, uint32_t mask)
{
    ASSERT(base);

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    /* Setting Rx FIFO Global Mask value. */
    base->RXFGMASK = mask;

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_set_rx_individual_mask(CAN_Type *base, uint8_t maskIdx,
                                        uint32_t mask)
{
    ASSERT(base);
    ASSERT(maskIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    /* Setting Rx Individual Mask value. */
    base->RXIMR[maskIdx] = mask;

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_set_tx_mb_config(CAN_Type *base, uint8_t mbIdx,
                                  bool tx_by_interrupt)
{
    ASSERT(base);

    uint32_t *msgBufAddr;

    /* Assertion. */
    ASSERT(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, mbIdx));

    /* Get message buffer address. */
    msgBufAddr = flexcan_lld_get_msg_buf_addr(base, mbIdx);

    /* CS filed: Inactivate Message Buffer. */
    mb_cs_field(msgBufAddr) = 0U;

    /* Clean ID filed. */
    mb_id_field(msgBufAddr) = 0U;

    /* Tx by interrupt or polling? */
    if (tx_by_interrupt) {
        /* Enable Message Buffer Interrupt. */
        flexcan_lld_enable_mb_int(base, mbIdx);
    }
    else {
        /* Disable Message Buffer Interrupt. */
        flexcan_lld_disable_mb_int(base, mbIdx);
    }
}

void flexcan_lld_set_rx_mb_config(CAN_Type *base, uint8_t mbIdx,
                                  const flexcan_rx_mb_config_t *config)
{
    /* Assertion. */
    ASSERT(base);
    ASSERT(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, mbIdx));
    ASSERT(config);

    uint32_t *msgBufAddr;
    uint32_t cs_temp = 0U;
    /* Get message buffer address. */
    msgBufAddr = flexcan_lld_get_msg_buf_addr(base, mbIdx);

    /* CS field: Inactivate Message Buffer. */
    mb_cs_field(msgBufAddr) = 0U;

    /* ID field: Clean Message Buffer content. */
    mb_id_field(msgBufAddr) = 0U;

    if (config->format == FLEXCAN_EXTEND_FRAME) {
        /* Setup Message Buffer ID. */
        mb_id_field(msgBufAddr) =
            (config->id) & (CAN_ID_EXT_MASK | CAN_ID_STD_MASK);
        /* Setup Message Buffer format. */
        cs_temp |= CAN_CS_IDE_MASK;
    }
    else {
        /* Setup Message Buffer ID. */
        mb_id_field(msgBufAddr) =
            ((config->id) << CAN_ID_STD_SHIFT) & CAN_ID_STD_MASK;
    }

    /* Activate Rx Message Buffer. */
    cs_temp |= CAN_CS_CODE(FLEXCAN_RxMbEmpty);
    mb_cs_field(msgBufAddr) = cs_temp;
}

void flexcan_lld_set_rx_fifo_config(CAN_Type *base,
                                    const flexcan_rx_fifo_config_t *config)
{
    /* Assertion. */
    ASSERT(base);
    ASSERT(config);
    ASSERT(config->idFilterNum <= 128);

    /* Rx FIFO can only be enabled when CAN FD feature is disabled. */
    ASSERT(!(base->MCR & CAN_MCR_FDEN_MASK));

    volatile uint32_t *idFilterRegion = (volatile uint32_t *)(&base->MB[6].CS);
    uint8_t setup_mb, i, rffn = 0;

    /* Get the setup_mb value. */
    setup_mb = (base->MCR & CAN_MCR_MAXMB_MASK) >> CAN_MCR_MAXMB_SHIFT;
    setup_mb = (setup_mb < FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER)
                   ? setup_mb
                   : FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER;

    /* Enter Fraze Mode. */
    (void)flexcan_lld_enter_freeze_mode(base, false);

    /* Determine RFFN value. */
    for (i = 0; i <= 0xF; i++) {
        if ((8 * (i + 1)) >= config->idFilterNum) {
            rffn = i;
            ASSERT(((setup_mb - 8) - (2 * rffn)) > 0);

            base->CTRL2 =
                (base->CTRL2 & ~CAN_CTRL2_RFFN_MASK) | CAN_CTRL2_RFFN(rffn);
            break;
        }
    }

    /* Clean ID filter table occuyied Message Buffer Region. */
    rffn = (rffn + 1) * 8;

    for (i = 0; i < rffn; i++) {
        idFilterRegion[i] = 0x0;
    }

    /* Disable unused Rx FIFO Filter. */
    for (i = config->idFilterNum; i < rffn; i++) {
        idFilterRegion[i] = 0xFFFFFFFFU;
    }

    /* Copy ID filter table to Message Buffer Region. */
    for (i = 0; i < config->idFilterNum; i++) {
        idFilterRegion[i] = config->idFilterTable[i].id;
        ssdk_printf(SSDK_INFO, "\nCAN Rx FIFO ID filter table[%d] = %x\n", i,
                    idFilterRegion[i]);
    }

    /* Setup ID Fitlter Type. */
    switch (config->idFilterType) {
    case FLEXCAN_RxFifoFilterTypeA:
        base->MCR = (base->MCR & ~CAN_MCR_IDAM_MASK) | CAN_MCR_IDAM(0x0);
        break;

    case FLEXCAN_RxFifoFilterTypeB:
        base->MCR = (base->MCR & ~CAN_MCR_IDAM_MASK) | CAN_MCR_IDAM(0x1);
        break;

    case FLEXCAN_RxFifoFilterTypeC:
        base->MCR = (base->MCR & ~CAN_MCR_IDAM_MASK) | CAN_MCR_IDAM(0x2);
        break;

    case FLEXCAN_RxFifoFilterTypeD:
        /* All frames rejected. */
        base->MCR = (base->MCR & ~CAN_MCR_IDAM_MASK) | CAN_MCR_IDAM(0x3);
        break;

    default:
        break;
    }

    /* Setting Message Reception Priority. */
    if (config->priority == FLEXCAN_RxFifoPrioHigh) {
        /* Matching starts from Rx FIFO and continues on Mailboxes. */
        base->CTRL2 &= ~CAN_CTRL2_MRP_MASK;
    }
    else {
        /* Matching starts from Mailboxes and continues on Rx FIFO. */
        base->CTRL2 |= CAN_CTRL2_MRP_MASK;
    }

    /* Enable Rx Message FIFO. */
    base->MCR |= CAN_MCR_RFEN_MASK;

    /* Exit Fraze Mode. */
    (void)flexcan_lld_exit_freeze_mode(base, false);
}

void flexcan_lld_enable_interrupts(CAN_Type *base, uint32_t mask)
{
    ASSERT(base);

    /* Solve Wake Up Interrupt. */
    if (mask & FLEXCAN_WakeUpInterruptEnable) {
        base->MCR |= CAN_MCR_WAKMSK_MASK;
    }

    /* Solve others. */
    base->CTRL1 |= (mask & (~((uint32_t)FLEXCAN_WakeUpInterruptEnable)));
}

void flexcan_lld_disable_interrupts(CAN_Type *base, uint32_t mask)
{
    ASSERT(base);

    /* Solve Wake Up Interrupt. */
    if (mask & FLEXCAN_WakeUpInterruptEnable) {
        base->MCR &= ~CAN_MCR_WAKMSK_MASK;
    }

    /* Solve others. */
    base->CTRL1 &= ~(mask & (~((uint32_t)FLEXCAN_WakeUpInterruptEnable)));
}

flexcan_status_t flexcan_lld_write_tx_mb(CAN_Type *base, uint8_t mbIdx,
                                         const flexcan_frame_t *txFrame,
                                         uint8_t padding_val)
{
    /* Assertion. */
    ASSERT(base);
    ASSERT(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, mbIdx));
    ASSERT(txFrame);
    ASSERT(((!txFrame->isCANFDFrame) && (txFrame->length <= 8)) ||
           ((txFrame->isCANFDFrame) && (txFrame->length <= 64)));

    uint32_t cs_temp = 0;
    uint32_t *msgBufAddr = flexcan_lld_get_msg_buf_addr(base, mbIdx);
    uint8_t dlc_val = flexcan_lld_compute_dlc_val(txFrame->length);
    uint8_t payload_len = flexcan_lld_compute_payload_len(dlc_val);
    uint32_t *mb_data = mb_data_field(msgBufAddr);

    /* Check if Message Buffer is activated. */
    if (CAN_CS_CODE(FLEXCAN_TxMbDataOrRemote) !=
        (mb_cs_field(msgBufAddr) & CAN_CS_CODE_MASK)) {
        /* Inactive Tx Message Buffer. */
        mb_cs_field(msgBufAddr) =
            (mb_cs_field(msgBufAddr) & ~CAN_CS_CODE_MASK) |
            CAN_CS_CODE(FLEXCAN_TxMbInactive);

        if (FLEXCAN_EXTEND_FRAME == txFrame->format) {
            /* Fill Message ID field. */
            mb_id_field(msgBufAddr) = txFrame->id;
            /* Fill Message Format field. */
            cs_temp |= CAN_CS_SRR_MASK | CAN_CS_IDE_MASK;
        }
        else {
            /* Fill Message ID field. */
            mb_id_field(msgBufAddr) =
                ((txFrame->id) << CAN_ID_STD_SHIFT) & CAN_ID_STD_MASK;
        }

        /* Fill Message Type field. */
        if (txFrame->isCANFDFrame) {
            cs_temp |= CAN_CS_EDL_MASK | CAN_CS_SRR_MASK;

            if (txFrame->isCANFDBrsEn) {
                cs_temp |= CAN_CS_BRS_MASK;
            }
        }
        else {
            if (txFrame->type == FLEXCAN_FrameTypeRemote) {
                cs_temp |= CAN_CS_RTR_MASK;
            }
        }

        if (txFrame->length < payload_len) {
            /* Pad unspecified data in CAN FD frames > 8 bytes. */
            memset((void *)(txFrame->dataBuffer + txFrame->length),
                   padding_val, payload_len - txFrame->length);
        }
        cs_temp |= CAN_CS_CODE(FLEXCAN_TxMbDataOrRemote) | CAN_CS_DLC(dlc_val);

        flexcan_lld_copy_to_mb(mb_data, (uint8_t *)txFrame->dataBuffer, payload_len);

        /* Activate Tx Message Buffer. */
        mb_cs_field(msgBufAddr) = cs_temp;

        return FLEXCAN_SUCCESS;
    }
    else {
        /* Tx Message Buffer is activated, return immediately. */
        ssdk_printf(SSDK_WARNING, "Write MB[%d] failed, MB cs field = 0x%x\n", mbIdx,
                    mb_cs_field(msgBufAddr));
        return FLEXCAN_FAIL;
    }
}

flexcan_status_t flexcan_lld_read_rx_mb(CAN_Type *base, uint8_t mbIdx,
                                        flexcan_frame_t *rxFrame)
{
    /* Assertion. */
    ASSERT(base);
    ASSERT(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, mbIdx));
    ASSERT(rxFrame);

    uint32_t cs_temp;
    uint8_t rx_code;
    uint32_t *msgBufAddr = flexcan_lld_get_msg_buf_addr(base, mbIdx);
    uint8_t rx_len;
    uint32_t *mb_data = mb_data_field(msgBufAddr);

    /* Read CS field of Rx Message Buffer to lock Message Buffer. */
    cs_temp = mb_cs_field(msgBufAddr);
    /* Get Rx Message Buffer Code field. */
    rx_code = (cs_temp & CAN_CS_CODE_MASK) >> CAN_CS_CODE_SHIFT;

    /* Check to see if Rx Message Buffer is busy. */
    if ((rx_code == FLEXCAN_RxMbFull) || (rx_code == FLEXCAN_RxMbOverrun)) {
        /* Get the message ID and format. */
        if (cs_temp & CAN_CS_IDE_MASK) {
            /* Solve Extend ID. */
            rxFrame->format = FLEXCAN_EXTEND_FRAME;
            /* Store Message ID. */
            rxFrame->id =
                mb_id_field(msgBufAddr) & (CAN_ID_EXT_MASK | CAN_ID_STD_MASK);
        }
        else {
            /* Solve Standard ID. */
            rxFrame->format = FLEXCAN_STANDARD_FRAME;
            /* Store Message ID. */
            rxFrame->id =
                (mb_id_field(msgBufAddr) & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
        }

        if (cs_temp & CAN_CS_EDL_MASK) {
            rxFrame->isCANFDFrame = true;
            /* CAN FD doesn't support remote frame. */
            rxFrame->type = FLEXCAN_FrameTypeData;
        }
        else {
            rxFrame->isCANFDFrame = false;
            /* Get the message type. */
            if (cs_temp & CAN_CS_RTR_MASK) {
                rxFrame->type = FLEXCAN_FrameTypeRemote;
            }
            else {
                rxFrame->type = FLEXCAN_FrameTypeData;
            }
        }

        /* Get the message length. */
        rx_len = (cs_temp & CAN_CS_DLC_MASK) >> CAN_CS_DLC_SHIFT;
        rxFrame->length = flexcan_lld_compute_payload_len(rx_len);

        /* Store Message Payload. */
        flexcan_lld_copy_from_mb(mb_data, rxFrame->dataBuffer, rxFrame->length);

        /* Read free-running timer to unlock Rx Message Buffer. */
        (void)base->TIMER;

        /* Set Rx Message Buffer to Empty. */
        mb_cs_field(msgBufAddr) =
            (mb_cs_field(msgBufAddr) & ~CAN_CS_CODE_MASK) |
            CAN_CS_CODE(FLEXCAN_RxMbEmpty);

        if (rx_code == FLEXCAN_RxMbFull) {
            return FLEXCAN_SUCCESS;
        }
        else {
            return FLEXCAN_RX_OVERFLOW;
        }
    }
    else {
        /* Read free-running timer to unlock Rx Message Buffer. */
        (void)base->TIMER;

        ssdk_printf(SSDK_WARNING, "Read CAN MB[%d] failed, MB cs field = 0x%x\n", mbIdx,
                    mb_cs_field(msgBufAddr));
        return FLEXCAN_FAIL;
    }
}

flexcan_status_t flexcan_lld_read_rx_fifo(CAN_Type *base,
                                          flexcan_frame_t *rxFrame)
{
    /* Assertion. */
    ASSERT(base && rxFrame);

    uint32_t cs_temp;
    uint32_t *msgBufAddr = flexcan_lld_get_msg_buf_addr(base, RX_FIFO_MB_ID);
    uint32_t *mb_data = mb_data_field(msgBufAddr);

    /* Check if Rx FIFO is Enabled. */
    if (base->MCR & CAN_MCR_RFEN_MASK) {
        /* Read CS field of Rx Message Buffer to lock Message Buffer. */
        cs_temp = base->MB[0].CS;

        /* Read data from Rx FIFO output port. */
        /* Get the message ID and format. */
        if (cs_temp & CAN_CS_IDE_MASK) {
            /* Solve Extend ID. */
            rxFrame->format = FLEXCAN_EXTEND_FRAME;
            /* Store Message ID. */
            rxFrame->id = base->MB[0].ID & (CAN_ID_EXT_MASK | CAN_ID_STD_MASK);
        }
        else {
            /* Solve Standard ID. */
            rxFrame->format = FLEXCAN_STANDARD_FRAME;
            /* Store Message ID. */
            rxFrame->id =
                (base->MB[0].ID & CAN_ID_STD_MASK) >> CAN_ID_STD_SHIFT;
        }

        /* Get the message type. */
        if (cs_temp & CAN_CS_RTR_MASK) {
            rxFrame->type = FLEXCAN_FrameTypeRemote;
        }
        else {
            rxFrame->type = FLEXCAN_FrameTypeData;
        }

        /* Get the message length. */
        rxFrame->length = (cs_temp & CAN_CS_DLC_MASK) >> CAN_CS_DLC_SHIFT;

        /* Store Message Payload. */
        flexcan_lld_copy_from_mb(mb_data, rxFrame->dataBuffer, rxFrame->length);

        /* Read free-running timer to unlock Rx Message Buffer. */
        (void)base->TIMER;

        /* Store ID Filter Hit Index. */
        rxFrame->idHit = (uint8_t)(base->RXFIR & CAN_RXFIR_IDHIT_MASK);

        return FLEXCAN_SUCCESS;
    }
    else {
        return FLEXCAN_FAIL;
    }
}

/* API function implementation end */