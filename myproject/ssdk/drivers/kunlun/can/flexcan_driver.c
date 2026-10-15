/**
 * @file flexcan_driver.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdint.h>
#include <string.h>
#include <bits.h>
#include <reg.h>
#include <regs_base.h>

#include "driver/kunlun/can/flexcan_driver.h"

#include "driver/can.h"
#include "driver/irq.h"
#include "flexcan_lld.h"

/* defines whether the bit rate is switched inside a CAN FD format frame */
#define CANFDBRS_EN 1

/*! @brief FlexCAN handle structure definition. */
typedef struct _flexcan_handle flexcan_handle_t;
typedef void (*flexcan_transfer_callback_t)(uint8_t ch, flexcan_status_t status,
                                            uint32_t result, void *userData);

/*! @brief FlexCAN handle structure. */
struct _flexcan_handle {
    /* Peripheral base address. */
    void *base_addr;
    /* Callback function. */
    flexcan_transfer_callback_t callback;
    /* FlexCAN callback function parameter.*/
    void *userData;
    /* The buffer for received data from Message Buffers. */
    flexcan_frame_t *volatile pMBFrameBuf[FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER];
    /* The buffer for received data from Rx FIFO. */
    flexcan_frame_t *volatile pRxFifoFrameBuf;
    /* Message Buffer transfer state. */
    volatile uint8_t mbState[FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER];
    /* Rx FIFO transfer state. */
    volatile uint8_t rxFifoState;
};

/* Array of FlexCAN handle. */
static flexcan_handle_t gFlexcanHandle[MAX_FLEXCAN_CH];
/* Bit map used to record if a MB is used. */
static uint32_t g_mb_used_mask[MAX_FLEXCAN_CH][2];
/* Array used to record lowest number of Tx mailbox */
static uint8_t g_origin_txMB_index[MAX_FLEXCAN_CH];
/* Array to save paddding value for tx mailboxes*/
static uint8_t g_send_padding_val[MAX_FLEXCAN_CH][FLEXCAN_HAS_MESSAGE_BUFFER_MAX_NUMBER];

/**
 * @brief config tx_mb and set mb is used.
 *
 * @param[in] device specific can device.
 * @param[in] mbIdx mailbox index.
 * @param[in] tx_by_interrupt if true, Message Buffer Interrupt enabled.
 */
void flexcan_set_tx_mb_config(can_device_t *device, uint8_t mbIdx,
                              bool tx_by_interrupt)
{
    ASSERT(device);

    CAN_Type *base = (CAN_Type *)(device->base);
    flexcan_lld_set_tx_mb_config(base, mbIdx, tx_by_interrupt);

    /* Set MB is used. */
    g_mb_used_mask[device->ch][BITMAP_INT(mbIdx)] |=
        1U << BITMAP_BIT_IN_INT(mbIdx);
}

/**
 * @brief config rx_mb and set mb is used.
 *
 * @param[in] device specific can device.
 * @param[in] mbIdx mailbox index.
 * @param[in] config rx mailbox config.
 */
void flexcan_set_rx_mb_config(can_device_t *device, uint8_t mbIdx,
                              const flexcan_rx_mb_config_t *config)
{
    ASSERT(device && config);

    CAN_Type *base = (CAN_Type *)(device->base);
    flexcan_lld_set_rx_mb_config(base, mbIdx, config);

    /* Set MB is used. */
    g_mb_used_mask[device->ch][BITMAP_INT(mbIdx)] |=
        1U << BITMAP_BIT_IN_INT(mbIdx);
}

/**
 * @brief send can frame and return immediately.
 *
 * @param[in] device specific can device.
 * @param[in] xfer FlexCAN Message Buffer transfer.
 * @param[in] padding_val set a value for padding.
 * @retval FLEXCAN_SUCCESS        Start Tx Message Buffer sending process
 * successfully.
 * @retval FLEXCAN_FAIL           Write Tx Message Buffer failed.
 * @retval FLEXCAN_TX_BUSY        Tx Message Buffer is in use.
 */
flexcan_status_t flexcan_send_nonblocking(can_device_t *device,
                                          flexcan_mb_transfer_t *xfer,
                                          uint8_t padding_val)
{
    /* Assertion. */
    ASSERT(device);
    ASSERT(device->ch < MAX_FLEXCAN_CH);
    ASSERT(xfer);

    flexcan_handle_t *handle = &gFlexcanHandle[device->ch];
    ASSERT(handle);

    CAN_Type *base = (CAN_Type *)(device->base);

    ASSERT(xfer->mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, xfer->mbIdx));

    /* Check if Message Buffer is idle. */
    if (FLEXCAN_StateIdle == handle->mbState[xfer->mbIdx]) {
        /* Distinguish transmit type. */
        if (FLEXCAN_FrameTypeRemote == xfer->pFrame->type) {
            handle->mbState[xfer->mbIdx] = FLEXCAN_StateTxRemote;
            /* Register user Frame buffer to receive remote Frame. */
            handle->pMBFrameBuf[xfer->mbIdx] = xfer->pFrame;
        }
        else {
            handle->mbState[xfer->mbIdx] = FLEXCAN_StateTxData;
        }

        if (FLEXCAN_SUCCESS == flexcan_lld_write_tx_mb(base, xfer->mbIdx,
                                                       xfer->pFrame,
                                                       padding_val)) {
            return FLEXCAN_SUCCESS;
        }
        else {
            handle->mbState[xfer->mbIdx] = FLEXCAN_StateIdle;
            return FLEXCAN_FAIL;
        }
    }
    else {
        /* Asking a busy mb to send means tx is not ready.
         * It's recommended to call flow stop to wait the mb to be idle,
         * which resumed in the irq context.
         *
         * If an unmatched mb transmit finish and call flow resume,
         * another flow stop would happen until this busy mb's tx is done.
         *
         * This method can prevent endless send try while
         * this mailbox is busy to transmit last frame.
         */
        can_tx_stop(device);
        return FLEXCAN_TX_BUSY;
    }
}

/**
 * @brief receive can frame from rx mailbox.this function returns immediately.
 *
 * @param[in] device specific can device.
 * @param[in] xfer FlexCAN Message Buffer transfer.
 * @retval FLEXCAN_SUCCESS if successful.
 * @retval FLEXCAN_RX_BUSY if RxMB is busy.
 */
flexcan_status_t flexcan_receive_nonblocking(can_device_t *device,
                                             flexcan_mb_transfer_t *xfer)
{
    /* Assertion. */
    ASSERT(device);
    ASSERT(xfer);

    flexcan_handle_t *handle = &gFlexcanHandle[device->ch];
    /* Assertion. */
    ASSERT(handle);

    CAN_Type *base = (CAN_Type *)(handle->base_addr);

    /* Assertion. */
    ASSERT(xfer);
    ASSERT(xfer->mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, xfer->mbIdx));

    /* Check if Message Buffer is idle. */
    if (FLEXCAN_StateIdle == handle->mbState[xfer->mbIdx]) {
        handle->mbState[xfer->mbIdx] = FLEXCAN_StateRxData;

        /* Register Message Buffer. */
        handle->pMBFrameBuf[xfer->mbIdx] = xfer->pFrame;

        /* Enable Message Buffer Interrupt. */
        flexcan_lld_enable_mb_int(base, xfer->mbIdx);

        return FLEXCAN_SUCCESS;
    }
    else {
        return FLEXCAN_RX_BUSY;
    }
}

/*!
 * @brief Receive message from Rx FIFO using IRQ.
 *
 * @param device specific can device.
 * @param xfer FlexCAN Rx FIFO transfer structure, refer to #flexcan_fifo_transfer_t.
 * @retval FLEXCAN_SUCCESS        Start Rx Message Buffer receiving process successfully.
 * @retval FLEXCAN_RX_FIFO_BUSY       - Rx FIFO is currently in use.
 */
flexcan_status_t flexcan_receive_fifo_nonblocking(can_device_t *device,
                                                  flexcan_fifo_transfer_t *xfer)
{
    /* Assertion. */
    ASSERT(device && xfer);

    flexcan_handle_t *handle = &gFlexcanHandle[device->ch];
    ASSERT(handle);

    CAN_Type *base = (CAN_Type *)(handle->base_addr);

    /* Check if Message Buffer is idle. */
    if (FLEXCAN_StateIdle == handle->rxFifoState) {
        handle->rxFifoState = FLEXCAN_StateRxFifo;

        /* Register Message Buffer. */
        handle->pRxFifoFrameBuf = xfer->pFrame;

        /* Enable Message Buffer Interrupt. */
        flexcan_lld_enable_mb_int(base, RX_FIFO_FRAME_AVL_MB_ID);
        flexcan_lld_enable_mb_int(base, RX_FIFO_ALMOST_FULL_MB_ID);
        flexcan_lld_enable_mb_int(base, RX_FIFO_OVERFLOW_MB_ID);

        return FLEXCAN_SUCCESS;
    }
    else {
        return FLEXCAN_RX_FIFO_BUSY;
    }
}

/**
 * @brief Abort interrupt driven message send process.
 *
 * This function aborts interrupt driven message send process.
 *
 * @param[in] device specific can device.
 * @param mbIdx The FlexCAN Message Buffer index.
 */
void flexcan_abort_mb_send(can_device_t *device, uint8_t mbIdx)
{
    ASSERT(device);

    flexcan_handle_t *handle = &gFlexcanHandle[device->ch];

    ASSERT(handle);

    CAN_Type *base = (CAN_Type *)device->base;

    ASSERT(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, mbIdx));

    /* Disable Message Buffer Interrupt. */
    flexcan_lld_disable_mb_int(base, mbIdx);

    /* Un-register handle. */
    handle->pMBFrameBuf[mbIdx] = 0U;

    /* Clean Message Buffer. */
    flexcan_set_tx_mb_config(device, mbIdx, false);

    handle->mbState[mbIdx] = FLEXCAN_StateIdle;
}

/**
 * @brief Abort interrupt driven message receive process.
 *
 * This function abort interrupt driven message receive process.
 *
 * @param[in] device specific can device.
 * @param mbIdx The FlexCAN Message Buffer index.
 */
void flexcan_abort_mb_receive(can_device_t *device, uint8_t mbIdx)
{
    ASSERT(device);

    flexcan_handle_t *handle = &gFlexcanHandle[device->ch];

    ASSERT(handle);

    CAN_Type *base = (CAN_Type *)device->base;

    ASSERT(mbIdx <= (base->MCR & CAN_MCR_MAXMB_MASK));
    ASSERT(!flexcan_lld_is_mb_occupied(base, mbIdx));

    /* Disable Message Buffer Interrupt. */
    flexcan_lld_disable_mb_int(base, mbIdx);

    /* Un-register handle. */
    handle->pMBFrameBuf[mbIdx] = 0U;
    handle->mbState[mbIdx] = FLEXCAN_StateIdle;
}

/**
 * @brief config flexcan message buffer for receive.
 *
 * @param[in] device specific can device.
 * @param[in] mb_idx The FlexCAN Message Buffer index.
 * @param[in] rx_mb_cfg config id and mask for filter.
 * @param[in] rx_by_interrupt read RxMB by irq or poll.
 */
static void flexcan_configRxMB(can_device_t *device,
                                uint8_t mb_idx,
                                rx_mb_cfg_t *rx_mb_cfg,
                                bool rx_by_interrupt)
{
    ASSERT(device && rx_mb_cfg);

    flexcan_rx_mb_config_t rxmbcfg;
    uint32_t mask;
    CAN_Type *base = (CAN_Type *)device->base;

    if (!rx_mb_cfg->isExtendedFrame)
        mask = (rx_mb_cfg->rxIDFilterMask << CAN_ID_STD_SHIFT) &
                                                    CAN_ID_STD_MASK;
    else
        mask = (rx_mb_cfg->rxIDFilterMask << CAN_ID_EXT_SHIFT) &
                                (CAN_ID_STD_MASK | CAN_ID_EXT_MASK);

    flexcan_lld_set_rx_individual_mask(base, mb_idx, mask);

    if (!rx_mb_cfg->isRemoteFrame)
        rxmbcfg.type = FLEXCAN_FrameTypeData;
    else
        rxmbcfg.type = FLEXCAN_FrameTypeRemote;

    if (!rx_mb_cfg->isExtendedFrame)
        rxmbcfg.format = FLEXCAN_STANDARD_FRAME;
    else
        rxmbcfg.format = FLEXCAN_EXTEND_FRAME;

    rxmbcfg.id = rx_mb_cfg->frameId;
    flexcan_set_rx_mb_config(device, mb_idx, &rxmbcfg);

    if (rx_by_interrupt) {
        flexcan_mb_transfer_t transfer = {NULL, mb_idx};
        flexcan_receive_nonblocking(device, &transfer);
    }
}

/**
 * @brief config flexcan message buffer for transmit
 *
 * @param[in] device specific can device.
 * @param[in] mb_idx The FlexCAN Message Buffer index.
 * @param[in] tx_by_interrupt check TxMB transmit done by irq or poll.
 */
static void flexcan_configTxMB(can_device_t *device, uint8_t mb_idx, bool tx_by_interrupt)
{
    flexcan_set_tx_mb_config(device, mb_idx, tx_by_interrupt);
}


/**
 * @brief config RX FIFO.
 *
 * @param[in] device specific can device.
 * @param[in] config config for RX fifo.
 * @return occupied mbs for fifo including id_filter_table.
 */
static uint8_t flexcan_configRxfifo(can_device_t *device, flexcan_rx_fifo_config_t *config)
{
    ASSERT(device&&config);

    uint8_t used_mb_for_fifo = 0;

    CAN_Type *base = (CAN_Type *)device->base;

    flexcan_lld_set_rx_fifo_config(base, config);
    flexcan_fifo_transfer_t transfer = {NULL};
    flexcan_receive_fifo_nonblocking(device, &transfer);
    /* each group of eight filters occupies two mb,
    * besides fifo occupies mb[0-5].
    * calculate mb used for fifo.
    */
    used_mb_for_fifo = config->idFilterNum / 8 * 2 + 6;

    /* if idFilterNum greater than used_mb_for_fifo,
    * elements with the number of used_mb_for_fifo
    * can be affected by RXIMR[0 : used_mb_for_fifo - 1],
    * remaing elements can only be affected by RXMGMASK.
    */
    for (uint8_t i = 0; i < used_mb_for_fifo; i++) {
        flexcan_lld_set_rx_individual_mask(base, i,
                    config->idFilterTable[i].rxIDFilterMask);
    }

    /* mark MB for fifo used. */
    for (uint8_t i = 0; i < used_mb_for_fifo; i++) {
        g_mb_used_mask[device->ch][BITMAP_INT(i)] |=
                                    1u << BITMAP_BIT_IN_INT(i);
    }

    return used_mb_for_fifo;
}

/**
 * @brief Initializes the FlexCAN handle.
 *
 * @param[in] ch can channel.
 * @param[in] reg_base Peripheral base address.
 * @param[in] useData specific can device.
 */
void flexcan_create_handle(uint8_t ch, void *reg_base, void *userData)
{
    ASSERT(ch < MAX_FLEXCAN_CH);
    ASSERT(reg_base);

    /* Save the context in global variables to support the double weak
     * mechanism. */
    flexcan_handle_t *handle = &gFlexcanHandle[ch];

    /* Register base address. */
    handle->base_addr = reg_base;
    handle->userData = userData;
}

/**
 * @brief This function get config values to initialize flexcan
 *
 * @param[in] cfg used to initialize flexcan
 * @param[in] config set in flexcan_cfg
 */
void flexcan_set_defaultconfig(flexcan_config_t *cfg,
                               const flexcan_controller_cfg_t *config)
{
    ASSERT(config);

    /* Initialize FlexCAN Module config struct with default value. */
    cfg->clkSrc = FLEXCAN_ClkSrcPeri; /* 80Mhz */

    cfg->enableListenOnly = false;
    if (config->enableSelfWakeup)
        cfg->enableSelfWakeup = true;
    else
        cfg->enableSelfWakeup = false;

    if (config->enableIndividMask)
        cfg->enableIndividMask = true;
    else
        cfg->enableIndividMask = false;

    cfg->enableDoze = false;

    cfg->nominalBitTiming.preDivider = config->nominal_bit_timing.preDivider;
    cfg->nominalBitTiming.rJumpwidth = config->nominal_bit_timing.rJumpwidth;
    cfg->nominalBitTiming.propSeg = config->nominal_bit_timing.propSeg;
    cfg->nominalBitTiming.phaseSeg1 = config->nominal_bit_timing.phaseSeg1;
    cfg->nominalBitTiming.phaseSeg2 = config->nominal_bit_timing.phaseSeg2;

    if (config->is_canfd_enabled) {
        cfg->enableCANFD = true;

        cfg->dataBitTiming.preDivider = config->data_bit_timing.preDivider;
        cfg->dataBitTiming.rJumpwidth = config->data_bit_timing.rJumpwidth;
        cfg->dataBitTiming.propSeg = config->data_bit_timing.propSeg;
        cfg->dataBitTiming.phaseSeg1 = config->data_bit_timing.phaseSeg1;
        cfg->dataBitTiming.phaseSeg2 = config->data_bit_timing.phaseSeg2;

        cfg->can_fd_cfg.enableBRS = true;
        cfg->can_fd_cfg.enableISOCANFD = true;
        cfg->can_fd_cfg.enableTDC = true;
        if (config->fd_tdc != 0)
            cfg->can_fd_cfg.TDCOffset = config->fd_tdc;
        else {
            cfg->can_fd_cfg.TDCOffset = config->data_bit_timing.preDivider *
                (config->data_bit_timing.propSeg + config->data_bit_timing.phaseSeg1);
        }

        cfg->can_fd_cfg.r0_mb_data_size = CAN_FD_64BYTES_PER_MB;
        cfg->can_fd_cfg.r1_mb_data_size = CAN_FD_64BYTES_PER_MB;
    }
    else {
        cfg->enableCANFD = false;
    }

    if (config->is_loopback_enabled) {
        /* when loopback enabled, TDC isn't allowed */
        cfg->enableLoopBack = true;
        cfg->can_fd_cfg.enableTDC = false;
    }
    else
        cfg->enableLoopBack = false;
}

/**
 * @brief convert can frame to flexcan frame to send.
 *
 * @param[in] frame flexcan frame.
 * @param[in] buf buf for sending.
 */
void flexcan_buf2frame(flexcan_frame_t *frame, can_frame_t *buf)
{
    ASSERT(buf);

    frame->id = buf->id;
    frame->format = buf->ide;
    frame->type = buf->rtr;
    frame->isCANFDFrame = buf->fd;
    if (buf->fd)
        frame->isCANFDBrsEn = CANFDBRS_EN;

    frame->length = buf->len;
    memcpy(&frame->dataBuffer, buf->data, buf->len);
}

/**
 * @brief convert received flexcan frame to can frame.
 *
 * @param[in] buf buf for receivind.
 * @param[in] frame flexcan frame from rx mailbox.
 */
void flexcan_frame2buf(can_frame_t *buf, flexcan_frame_t *framebuf)
{
    ASSERT(framebuf);

    if (framebuf->format == FLEXCAN_STANDARD_FRAME) {
        buf->ide = CAN_STDID;
        buf->id = framebuf->id;
    }
    else {
        buf->ide = CAN_EXTID;
        buf->id = framebuf->id;
    }

    if (framebuf->type == FLEXCAN_FrameTypeData) {
        buf->rtr = CAN_DTR;
    }
    else if (framebuf->type == FLEXCAN_FrameTypeRemote) {
        buf->rtr = CAN_RTR;
    }

    buf->fd = framebuf->isCANFDFrame;

    buf->len = framebuf->length;
    memcpy(buf->data, &framebuf->dataBuffer, framebuf->length);
}

/**
 * @brief set flexcan to given mode.
 *
 * @param[in] can_dev specific can device.
 * @param[in] mode operation mode.
 */
static int flexcan_setmode(can_device_t *can_dev, enum can_mode mode)
{
    ASSERT(can_dev);

    CAN_Type *base = (CAN_Type *)can_dev->base;
    uint32_t mcrTemp;
    uint32_t ctrl1Temp;

    (void)flexcan_lld_freeze(base, true);
    mcrTemp = base->MCR;
    ctrl1Temp = base->CTRL1;

    if (mode == CAN_LOOPBACK_MODE || mode == CAN_LOOPBACKANLISTEN_MODE) {
        ctrl1Temp &= ~CAN_CTRL1_LOM_MASK;
        ctrl1Temp |= CAN_CTRL1_LPB_MASK;
        mcrTemp &= ~CAN_MCR_SRXDIS_MASK;
    }
    else {
        ctrl1Temp &= ~CAN_CTRL1_LPB_MASK;
        /* Disable the self reception feature when FlexCAN is not in loopback
         * mode. */
        mcrTemp |= CAN_MCR_SRXDIS_MASK;
    }

    if (mode == CAN_LISTEN_MODE || mode == CAN_LOOPBACKANLISTEN_MODE)
        ctrl1Temp |= CAN_CTRL1_LOM_MASK;
    else
        ctrl1Temp &= ~CAN_CTRL1_LOM_MASK;

    base->CTRL1 = ctrl1Temp;
    base->MCR = mcrTemp;

    (void)flexcan_lld_freeze(base, false);

    return 0;
}

#ifndef CONFIG_SDRV_FLEXCAN_ENH
struct can_sleep_scr {
    uint32_t base;
    uint8_t stop_pos;
    uint8_t doze_pos;
};

static const struct can_sleep_scr gCanSleepSCR[4] = {
    {APB_SCR_SAF_BASE + (0x600U << 10), 4U, 5U},
    {APB_SCR_SAF_BASE + (0x604U << 10), 4U, 5U},
    {APB_SCR_SAF_BASE + (0x608U << 10), 4U, 5U},
    {APB_SCR_SAF_BASE + (0x60CU << 10), 4U, 5U},
};

static void flexcan_request_sleep(can_device_t *can_dev, bool request)
{
    CAN_Type *base = (CAN_Type *)can_dev->base;

    if (can_dev->ch > 4)
        return;

    const struct can_sleep_scr *s = &gCanSleepSCR[can_dev->ch];

    if (base->MCR & CAN_MCR_DOZE_MASK) {
        RMWREG32(s->base, s->doze_pos, 1, !!request);
    }
    else {
        RMWREG32(s->base, s->stop_pos, 1, !!request);
    }
}

#else

static void flexcan_request_sleep(can_device_t *can_dev, bool request)
{
    CAN_Type *base = (CAN_Type *)can_dev->base;

    /* {lowpower_en,lowpower_mode} */
    base->LOWPOWER_MODE = !!request << 1 | 0x1;
}
#endif

/**
 * @brief initialize flexcan controller.
 *
 * @param[in] can_dev specific can device.
 * @param[in] config flexcan confg.
 * @param[in] flexcan_mb_cfg mailboxes config.
 */
static int flexcan_init(can_device_t *can_dev,
                        const flexcan_controller_cfg_t *config,
                        flexcan_mb_config_t *flexcan_mb_cfg)
{
    ASSERT(can_dev && config);

    uint8_t ch = can_dev->ch;
    uint8_t rx_mb_cnt, tx_mb_cnt, used_mbs = 0;
    int ret = 0;

    CAN_Type *base = (CAN_Type *)can_dev->base;

    flexcan_config_t cfg;

    flexcan_set_defaultconfig(&cfg, config);
    cfg.maxMbNum = flexcan_mb_cfg->maxMBNum;

    ret = flexcan_lld_init(base, &cfg);
    if (ret) {
        ssdk_printf(SSDK_ERR, "fail to config FlexCAN[%d]\n", ch);
        return ret;
    }

    flexcan_create_handle(ch, (void *)base, can_dev);

    if (config->is_rx_fifo_needed) {
        used_mbs = flexcan_configRxfifo(can_dev, flexcan_mb_cfg->fifo_cfg);
    }

    rx_mb_cnt = flexcan_mb_cfg->rx_mb_count;
    ssdk_printf(SSDK_INFO, "used_mbs=%d rx_mb_cnt=%d\n", used_mbs, rx_mb_cnt);

    for (uint8_t idx = 0; idx < rx_mb_cnt; idx++) {
        rx_mb_cfg_t *rx_mb_cfg = &flexcan_mb_cfg->rx_mb_cfg[idx];
        uint8_t mb_idx = idx + used_mbs;
        /* true means rx by irq */
        flexcan_configRxMB(can_dev, mb_idx, rx_mb_cfg, true);
    }

    used_mbs += rx_mb_cnt;  /* used for fifo && rx_mb */

    tx_mb_cnt = flexcan_mb_cfg->tx_mb_count;
    for (uint8_t idx = 0; idx < tx_mb_cnt; idx++) {
        uint8_t mb_idx = idx + used_mbs;
        g_send_padding_val[ch][mb_idx] = flexcan_mb_cfg->tx_mb_cfg[idx].paddingVal;
        flexcan_configTxMB(can_dev, mb_idx, true);
    }

    g_origin_txMB_index[can_dev->ch] = used_mbs;
    used_mbs += tx_mb_cnt;

    ssdk_printf(SSDK_INFO, "use_mbs=%d\n", used_mbs);
    ASSERT(used_mbs <= flexcan_mb_cfg->maxMBNum);

    flexcan_lld_enable_interrupts(base, FLEXCAN_BusOffInterruptEnable
#if EN_ERR_INT
                                    | FLEXCAN_ErrorInterruptEnable
#endif
#if EN_WARNING_INT
                                    | FLEXCAN_RxWarningInterruptEnable
                                    | FLEXCAN_TxWarningInterruptEnable
#endif
#if EN_WAKE_UP_INT
                                    | FLEXCAN_WakeUpInterruptEnable
#endif
    );

    ret = can_init(can_dev, (struct can_buf_cfg *)&config->buf_cfg);
    if (ret) {
        ssdk_printf(SSDK_ERR, "fail to alloc can buf\n");
        return ret;
    }

    return ret;
}

/**
 * @brief This function handles the FlexCAN Error, Message Buffer and Rx FIFO
 * IRQ request.
 *
 * @param[in] irq interrupt number.
 * @param[in] arg argument provided to the interrupt handler.
 */
static int flexcan_irq_handler(uint32_t irq, void *arg)
{
    struct can_device *can_dev = (struct can_device *)arg;
    uint8_t ch = can_dev->ch;
    ASSERT(ch < MAX_FLEXCAN_CH);

    flexcan_handle_t *handle = &gFlexcanHandle[ch];
    ASSERT(handle);

    CAN_Type *base = (CAN_Type *)(handle->base_addr);
    uint32_t errStatus = base->ESR1;
    uint8_t mb_num = ((base->MCR) & CAN_MCR_MAXMB_MASK) + 1U;
    uint8_t mbIdx;
    bool get_int_req = false;
    uint32_t int_req = 0U;

    /*ssdk_printf(SSDK_INFO,
                "\nCAN[%d] interrupt occurs, IMASK1 = 0x%x, IFLAG1 = 0x%x, "
                "IMASK2 = 0x%x, IFLAG2 = 0x%x, errStatus=0x%x\n",
                ch, base->IMASK1, base->IFLAG1, base->IMASK2, base->IFLAG2,
                errStatus);*/

    /* FlexCAN error interrupt handling. */
    if (errStatus & (FLEXCAN_TxWarningIntFlag | FLEXCAN_RxWarningIntFlag |
                     FLEXCAN_BusOffIntFlag | FLEXCAN_ErrorIntFlag |
                     FLEXCAN_WakeUpIntFlag | FLEXCAN_ErrorFlag)) {
        /* Clear error interrupt flags. */
        flexcan_lld_clear_err_status_flag(
            base, FLEXCAN_TxWarningIntFlag | FLEXCAN_RxWarningIntFlag |
                      FLEXCAN_BusOffIntFlag | FLEXCAN_ErrorIntFlag |
                      FLEXCAN_WakeUpIntFlag | FLEXCAN_ErrorFlag);

        if (errStatus & CAN_ESR1_BOFFINT_MASK)
            can_err_notify(can_dev, CAN_BUSOFF);
        else if(errStatus & CAN_ESR1_WAKINT_MASK)
            can_err_notify(can_dev, CAN_WAKEUP);
        else if(errStatus & CAN_ESR1_ACKERR_MASK)
            can_err_notify(can_dev, CAN_NO_ACK);
        else if(errStatus & (CAN_ESR1_BIT0ERR_MASK | CAN_ESR1_BIT1ERR_MASK))
            can_err_notify(can_dev, CAN_BIT_ERROR);

    }

#ifdef CONFIG_SDRV_FLEXCAN_ENH

#define ERR_IRQ_STATUS_PSLVERR_MASK     0x200000

    uint32_t err_irq_status = base->ERR_IRQ_STATUS;
    ssdk_printf(SSDK_INFO, "ERR_IRQ_STATUS = %#x\n",
                            err_irq_status);

    if (err_irq_status) {
        base->ERR_IRQ_STATUS = err_irq_status;
        if (err_irq_status & ERR_IRQ_STATUS_PSLVERR_MASK) {
            ssdk_printf(SSDK_ERR,
                        "access to reserved address results in error\n");
        }
    }

#endif

    /* FlexCAN message buffer interrupt handling. */
    for (mbIdx = 0U; mbIdx < mb_num; mbIdx++) {
        /* Every 32 MBs share one IMASK register and one IFLAG register. */
        if ((mbIdx & 31U) == 0U) {
            get_int_req = true;
        }

        /* A MB is checked only if it is used. */
        if (BIT_SET(g_mb_used_mask[ch][BITMAP_INT(mbIdx)],
                    BITMAP_BIT_IN_INT(mbIdx))) {
            if (get_int_req) {
                /* Read interrupt status: IMASK & IFLAG. */
                int_req = flexcan_lld_get_mb_int_req(base, mbIdx >> 5);
                get_int_req = false;
            }

            /* Check if the MB's interrupt is generated. */
            if (int_req & (1U << (mbIdx & 31U))) {

                /* Rx FIFO interrupt handling. */
                if ((handle->rxFifoState != FLEXCAN_StateIdle) &&
                    (mbIdx >= RX_FIFO_FRAME_AVL_MB_ID) &&
                    (mbIdx <= RX_FIFO_OVERFLOW_MB_ID)) {
                    do {
                        switch (1U << mbIdx) {
                        case FLEXCAN_RxFifoOverflowFlag:
                            can_err_notify(can_dev, CAN_RX_OVERFLOW);
                            break;

                        case FLEXCAN_RxFifoWarningFlag:
                            break;

                        case FLEXCAN_RxFifoFrameAvlFlag:
                            can_read(can_dev, RX_FIFO_MB_ID);
                            break;

                        default:
                            break;
                        }

                        /* Clear message buffer interrupt flag. */
                        flexcan_lld_clear_mb_int_flag(base, mbIdx);

                        /* Calling Callback Function if has one. */
                        if (handle->callback != NULL) {
                        }
                    } while (flexcan_lld_get_mb_int_status_flag(base, mbIdx) !=
                             0U);
                }
                else {
                    switch (handle->mbState[mbIdx]) {
                    case FLEXCAN_StateRxData:
                    case FLEXCAN_StateRxRemote:
                        can_read(can_dev, mbIdx);
                        break;

                    case FLEXCAN_StateTxData:
                        handle->mbState[mbIdx] = FLEXCAN_StateIdle;
                        can_tx_resume(can_dev);
                        break;

                    case FLEXCAN_StateTxRemote:
                        handle->mbState[mbIdx] = FLEXCAN_StateRxRemote;
                        break;

                    default:
                        break;
                    }
                    /* Clear message buffer interrupt flag. */
                    flexcan_lld_clear_mb_int_flag(base, mbIdx);
                }
            }
        }
    }
    return 0;
}

/**
 * @brief perform data transfer for flexcan.
 *
 * @param[in] can_dev specific can device.
 * @param[in] can_buf buf for sending.
 */
static int flexcan_send(can_device_t *can_dev, can_buf_t *can_buf)
{
    ASSERT(can_dev && can_buf);

    int ret;
    uint8_t padding_val;
    flexcan_frame_t frame;
    /* decide which mailbox to send */
    uint8_t sendMB = g_origin_txMB_index[can_dev->ch] + can_buf->mb_idx;
    can_frame_t *buf = &can_buf->frame;

    flexcan_buf2frame(&frame, buf);

    flexcan_mb_transfer_t Xfer;
    Xfer.mbIdx = sendMB;
    Xfer.pFrame = &frame;
    padding_val = g_send_padding_val[can_dev->ch][sendMB];
    ret = flexcan_send_nonblocking(can_dev, &Xfer, padding_val);

    return ret;
}

/**
 * @brief receive data on specefic mailbox.
 *
 * @param[in] can_dev specific can device.
 * @param[in] can_buf buf for receive.
 * @param[in] index index of mailbox to read.
 */
static int flexcan_recv(struct can_device *can_dev, can_frame_t *buf,
                        uint8_t index)
{
    ASSERT(can_dev);
    ASSERT(buf);

    int ret;
    CAN_Type *base = (CAN_Type *)can_dev->base;
    flexcan_frame_t framebuf;

    if ((base->MCR & CAN_MCR_RFEN_MASK) && index == RX_FIFO_MB_ID)
        ret = flexcan_lld_read_rx_fifo(base, &framebuf);
    else
        ret = flexcan_lld_read_rx_mb(base, index, &framebuf);

    if (!ret || (ret == FLEXCAN_RX_OVERFLOW)) {
        /* RX_OVERFLOW means MB is overwritten into a full buffer.
         * In this scene, function chooses to reserve the new frame
         * instead of dropping such frame.
         */
        if (ret == FLEXCAN_RX_OVERFLOW) {
            can_err_notify(can_dev, CAN_RX_OVERFLOW);
        }

        flexcan_frame2buf(buf, &framebuf);
        return 0;
    }
    else
        return ret;
}

/**
 * @brief check whether specefic mailbox receive a frame.
 *
 * @param[in] can_dev specific can device.
 * @param[in] mb_idx index of mailbox to read.
 */
static bool flexcan_check_recvmsg(can_device_t *can_dev, uint8_t mb_idx)
{
    ASSERT(can_dev);

    CAN_Type *base = (CAN_Type *)(can_dev->base);

    return flexcan_lld_get_mb_iflag(base, mb_idx);
}

static struct can_ops flex_can_ops = {
    .sendmsg = flexcan_send,
    .recvmsg = flexcan_recv,
    .check_recvmsg = flexcan_check_recvmsg,
    .set_mode = flexcan_setmode,
    .request_sleep = flexcan_request_sleep
};


void flexcan_probe(const flexcan_controller_cfg_t *config,
                   flexcan_mb_config_t *flexcan_mb_cfg)
{
    ASSERT(config);
    
    int ret;
    can_device_t * can_dev = can_alloc();

    if (can_dev == NULL) {
        ssdk_printf(SSDK_CRIT, "flexcan register failed\n");
        return;
    }
    can_dev->ch = config->channel;
    can_dev->base = config->base;
    can_dev->ops = &flex_can_ops;

    ret = flexcan_init(can_dev, config, flexcan_mb_cfg);
    if (ret) {
        ssdk_printf(SSDK_CRIT, "flexcan_probe failed\n");
        return;
    }

    can_dev->irq_nr = config->irq;
    if (can_dev->irq_nr >= 0) {
        irq_attach(can_dev->irq_nr, flexcan_irq_handler, can_dev);
        irq_enable(can_dev->irq_nr);
    }
}
