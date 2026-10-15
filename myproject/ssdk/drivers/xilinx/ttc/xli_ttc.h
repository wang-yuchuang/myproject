/**
 * @file xli_ttc.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Xilinx ttc driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef DRV_LLD_XLI_TTC_H
#define DRV_LLD_XLI_TTC_H

#include <types.h>
#include <compiler.h>

/** @name Register Map
 *
 * Register offsets from the base address of the device.
 *
 * @{
 */
#define XLI_TTC_CLK_CNTRL_OFFSET         0x00000000U  /**< Clock Control Register */
#define XLI_TTC_CNT_CNTRL_OFFSET         0x0000000CU  /**< Counter Control Register*/
#define XLI_TTC_COUNT_VALUE_OFFSET       0x00000018U  /**< Current Counter Value */
#define XLI_TTC_INTERVAL_VAL_OFFSET      0x00000024U  /**< Interval Count Value */
#define XLI_TTC_MATCH_0_OFFSET           0x00000030U  /**< Match 1 value */
#define XLI_TTC_MATCH_1_OFFSET           0x0000003CU  /**< Match 2 value */
#define XLI_TTC_MATCH_2_OFFSET           0x00000048U  /**< Match 3 value */
#define XLI_TTC_ISR_OFFSET               0x00000054U  /**< Interrupt Status Register */
#define XLI_TTC_IER_OFFSET               0x00000060U  /**< Interrupt Enable Register */
/* @} */

/** @name Clock Control Register
 * Clock Control Register definitions
 * @{
 */
#define XLI_TTC_CLK_CNTRL_PS_EN_MASK     0x00000001U  /**< Prescale enable */
#define XLI_TTC_CLK_CNTRL_PS_VAL_MASK    0x0000001EU  /**< Prescale value */
#define XLI_TTC_CLK_CNTRL_PS_VAL_SHIFT            1U  /**< Prescale shift */
#define XLI_TTC_CLK_CNTRL_PS_DISABLE             16U  /**< Prescale disable */
#define XLI_TTC_CLK_CNTRL_SRC_MASK       0x00000020U  /**< Clock source */
#define XLI_TTC_CLK_CNTRL_EXT_EDGE_MASK  0x00000040U  /**< External Clock edge */
/* @} */

/** @name Counter Control Register
 * Counter Control Register definitions
 * @{
 */
#define XLI_TTC_CNT_CNTRL_DIS_MASK       0x00000001U /**< Disable the counter */
#define XLI_TTC_CNT_CNTRL_INT_MASK       0x00000002U /**< Interval mode */
#define XLI_TTC_CNT_CNTRL_DECR_MASK      0x00000004U /**< Decrement mode */
#define XLI_TTC_CNT_CNTRL_MATCH_MASK     0x00000008U /**< Match mode */
#define XLI_TTC_CNT_CNTRL_RST_MASK       0x00000010U /**< Reset counter */
#define XLI_TTC_CNT_CNTRL_EN_WAVE_MASK   0x00000020U /**< Enable waveform */
#define XLI_TTC_CNT_CNTRL_POL_WAVE_MASK  0x00000040U /**< Waveform polarity */
#define XLI_TTC_CNT_CNTRL_RESET_VALUE    0x00000021U /**< Reset value */
/* @} */

/** @name Current Counter Value Register
 * Current Counter Value Register definitions
 * @{
 */
#define XLI_TTC_COUNT_VALUE_MASK         0xFFFFFFFFU /**< 32-bit counter value */
/* @} */

/** @name Interval Value Register
 * Interval Value Register is the maximum value the counter will count up or
 * down to.
 * @{
 */
#define XLI_TTC_INTERVAL_VAL_MASK        0xFFFFFFFFU /**< 32-bit Interval value*/
/* @} */

/** @name Match Registers
 * Definitions for Match registers, each timer counter has three match
 * registers.
 * @{
 */
#define XLI_TTC_MATCH_MASK               0xFFFFFFFFU /**< 32-bit Match value */
#define XLI_TTC_NUM_MATCH_REG            3U /**< Num of Match reg */
/* @} */

/** @name Interrupt Registers
 * Following register bit mask is for all interrupt registers.
 *
 * @{
 */
#define XLI_TTC_IXR_INTERVAL_MASK        0x00000001U  /**< Interval Interrupt */
#define XLI_TTC_IXR_MATCH_0_MASK         0x00000002U  /**< Match 1 Interrupt */
#define XLI_TTC_IXR_MATCH_1_MASK         0x00000004U  /**< Match 2 Interrupt */
#define XLI_TTC_IXR_MATCH_2_MASK         0x00000008U  /**< Match 3 Interrupt */
#define XLI_TTC_IXR_CNT_OVR_MASK         0x00000010U  /**< Counter Overflow */
#define XLI_TTC_IXR_ALL_MASK             0x0000001FU  /**< All valid Interrupts */
/* @} */

#ifndef ASSEMBLY

__BEGIN_CDECLS

/**
 * @brief xli ttc init.
 *
 * @param[in] base xli ttc address
 */
void xli_ttc_lld_init(uint32_t base);

/**
 * @brief xli ttc set timer start.
 *
 * @param[in] base xli ttc address
 * @param[in] time_out time_out time out value(count)
 */
void xli_ttc_lld_timer_start(uint32_t base, uint32_t count);

/**
 * @brief xli ttc stop.
 *
 * @param[in] base xli ttc address
 */
void xli_ttc_lld_timer_stop(uint32_t base);

/**
 * @brief xli ttc get current cnt value.
 *
 * @param[in] base xli ttc address
 * @return current time(count)
 */
uint32_t xli_ttc_lld_get_cur_cnt(uint32_t base);

/**
 * @brief xli ttc clear int status.
 *
 * @param[in] base xli ttc address
 */
void xli_ttc_lld_clear_int(uint32_t base);

__END_CDECLS

#endif

#endif
