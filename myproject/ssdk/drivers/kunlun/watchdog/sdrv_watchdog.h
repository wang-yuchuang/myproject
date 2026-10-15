/**
 * @file sdrv_watchdog.h
 * @brief semidrive watchdog driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef SDRV_WATCHDOG_H_
#define SDRV_WATCHDOG_H_

#include <compiler.h>
#include <types.h>

#define WDG_CTRL (0x00u)
#define WDG_CTRL_SOFT_RST (0u)
#define WDG_CTRL_WDG_EN (1u)
#define WDG_CTRL_CLK_SRC_LSB (2u)
#define WDG_CTRL_CLK_SRC_MASK (0x7u)
#define WDG_CTRL_WTC_SRC (5u)
#define WDG_CTRL_AUTO_RESTART (6u)
#define WDG_CTRL_DBG_HALT_EN (7u)
#define WDG_CTRL_WDG_EN_SRC (8u)
#define WDG_CTRL_SELFTEST_TRIG (9u)
#define WDG_CTRL_WDG_EN_STA (10u)
#define WDG_CTRL_PRE_DIV_NUM_LSB (16u)
#define WDG_CTRL_PRE_DIV_NUM_MASK (0xFFFFu)

#define WDG_WTC (0x04u)
#define WDG_WTC_VAL_MASK (0xFFFFFFFFu)

#define WDG_WRC_CTRL (0x08u)
#define WDG_WRC_CTRL_MODEM0 (0u)
#define WDG_WRC_CTRL_MODEM1 (1u)
#define WDG_WRC_CTRL_SEQ_REFR (2u)
#define WDG_WRC_CTRL_REFR_TRIG (3u)

#define WDG_WRC_VAL (0x0Cu)
#define WDG_WRC_VAL_MASK (0xFFFFFFFFu)

#define WDG_WRC_SEQ (0x10u)
#define WDG_WRC_SEQ_MASK (0xFFFFFFFFu)

#define WDG_RST_CTRL (0x14u)
#define WDG_RST_CTRL_RST_CNT_MASK (0xFFFFu)
#define WDG_RST_CTRL_INT_RST_EN (16u)
#define WDG_RST_CTRL_INT_RST_MODE (17u)
#define WDG_RST_CTRL_WDG_RST_EN (18u)
#define WDG_RST_CTRL_RST_WIN_LSB (20u)
#define WDG_RST_CTRL_RST_WIN_MASK (0xFFu)

#define WDG_EXT_RST_CTRL (0x18u)
#define WDG_EXT_RST_CTRL_RST_CNT_MASK (0xFFFFu)
#define WDG_EXT_RST_CTRL_EXT_RST_EN (16u)
#define WDG_EXT_RST_CTRL_EXT_RST_MODE (17u)
#define WDG_EXT_RST_CTRL_RST_WIN_LSB (20u)
#define WDG_EXT_RST_CTRL_RST_WIN_MASK (0xFFu)
#define WDG_EXT_RST_CTRL_EXT_RST_REQ_POL (28u)

#define WDG_CNT (0x1Cu)
#define WDG_TWS (0x20u)

#define WDG_INT (0x24u)
#define WDG_INT_ILL_WIN_REFE_INT_EN (0u)
#define WDG_INT_ILL_SEQ_REFE_INT_EN (1u)
#define WDG_INT_OVERFLOW_INT_EN (2u)
#define WDG_INT_ILL_WIN_REFE_INT_STA (3u)
#define WDG_INT_ILL_SEQ_REFE_INT_STA (4u)
#define WDG_INT_OVERFLOW_INT_STA (5u)
#define WDG_INT_ILL_WIN_REFE_INT_CLR (6u)
#define WDG_INT_ILL_SEQ_REFE_INT_CLR (7u)
#define WDG_INT_OVERFLOW_INT_CLR (8u)

#ifdef CONFIG_SDRV_WDOG_TAISHAN
#define WDG_RST_REQ_MON (0x28u)
#endif // CONFIG_SDRV_WDOG_TAISHAN

#define WDG_LOCK (0x40u)
#define WDG_LOCK_CTL_LOCK (0u)
#define WDG_LOCK_WTC_LOCK (1u)
#define WDG_LOCK_WRC_LOCK (2u)
#define WDG_LOCK_RST_LOCK (3u)
#define WDG_LOCK_EXT_RST_LOCK (4u)
#define WDG_LOCK_INT_LOCK (5u)
#define WDG_LOCK_CLK_SRC_LOCK (6u)

#define __IO volatile

/* watchdog clock source */
#define MHZ (1000 * 1000)
#define KHZ 1000
#define WDG_MAIN_CLK 24 * MHZ
#define WDG_BUS_CLK 250 * MHZ
#define WDG_EXT_CLK 26 * MHZ
#define WDG_TIE_OFF 0
#define WDG_LP_CLK 32 * KHZ

#define WDG_READ_CTRL(base) ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_ctrl))
#define WDG_READ_WTC(base) ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_wtc))
#define WDG_READ_WRC_CTL(base)                                                 \
    ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_wrc_ctl))
#define WDG_READ_WRC_VAL(base)                                                 \
    ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_wrc_val))
#define WDG_READ_WRC_SEQ(base)                                                 \
    ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_wrc_seq))
#define WDG_READ_RST_CTL(base)                                                 \
    ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_rst_ctl))
#define WDG_READ_EXT_RST_CTL(base)                                             \
    ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_ext_rst_ctl))
#define WDG_READ_CNT(base) ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_cnt))
#define WDG_READ_TSW(base) ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_tsw))
#define WDG_READ_INT(base) ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_int))

#ifdef CONFIG_SDRV_WDOG_TAISHAN
#define WDG_READ_RST_REQ_MON(base)                                             \
    ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_rst_req_mon))
#endif // CONFIG_SDRV_WDOG_TAISHAN

#define WDG_READ_LOCK(base) ((uint32_t)(((wdg_reg_type_t *)(base))->wdg_lock))

#define WDG_WRITE_CTRL(base, val)                                              \
    ((wdg_reg_type_t *)(base))->wdg_ctrl = (uint32_t)(val)
#define WDG_WRITE_WTC(base, val)                                               \
    ((wdg_reg_type_t *)(base))->wdg_wtc = (uint32_t)(val)
#define WDG_WRITE_WRC_CTL(base, val)                                           \
    ((wdg_reg_type_t *)(base))->wdg_wrc_ctl = (uint32_t)(val)
#define WDG_WRITE_WRC_VAL(base, val)                                           \
    ((wdg_reg_type_t *)(base))->wdg_wrc_val = (uint32_t)(val)
#define WDG_WRITE_WRC_SEQ(base, val)                                           \
    ((wdg_reg_type_t *)(base))->wdg_wrc_seq = (uint32_t)(val)
#define WDG_WRITE_RST_CTL(base, val)                                           \
    ((wdg_reg_type_t *)(base))->wdg_rst_ctl = (uint32_t)(val)
#define WDG_WRITE_EXT_RST_CTL(base, val)                                       \
    ((wdg_reg_type_t *)(base))->wdg_ext_rst_ctl = (uint32_t)(val)
#define WDG_WRITE_CNT(base, val)                                               \
    ((wdg_reg_type_t *)(base))->wdg_cnt = (uint32_t)(val)
#define WDG_WRITE_TSW(base, val)                                               \
    ((wdg_reg_type_t *)(base))->wdg_tsw = (uint32_t)(val)
#define WDG_WRITE_INT(base, val)                                               \
    ((wdg_reg_type_t *)(base))->wdg_int = (uint32_t)(val)

#ifdef CONFIG_SDRV_WDOG_TAISHAN
#define WDG_WRITE_RST_REQ_MON(base, val)                                       \
    ((wdg_reg_type_t *)(base))->wdg_rst_req_mon = (uint32_t)(val)
#endif // CONFIG_SDRV_WDOG_TAISHAN

#define WDG_WRITE_LOCK(base, val)                                              \
    ((wdg_reg_type_t *)(base))->wdg_lock = (uint32_t)(val)

typedef struct wdg_reg_type {
    __IO uint32_t wdg_ctrl;
    __IO uint32_t wdg_wtc;
    __IO uint32_t wdg_wrc_ctl;
    __IO uint32_t wdg_wrc_val;
    __IO uint32_t wdg_wrc_seq;
    __IO uint32_t wdg_rst_ctl;
    __IO uint32_t wdg_ext_rst_ctl;
    __IO uint32_t wdg_cnt;
    __IO uint32_t wdg_tsw;
    __IO uint32_t wdg_int;

#ifdef CONFIG_SDRV_WDOG_TAISHAN
    __IO uint32_t wdg_rst_req_mon;
    __IO uint32_t nulldata[5];
#endif // CONFIG_SDRV_WDOG_TAISHAN

#ifdef CONFIG_SDRV_WDOG_KUNLUN
    __IO uint32_t nulldata[6];
#endif // CONFIG_SDRV_WDOG_KUNLUN

    __IO uint32_t wdg_lock;
} wdg_reg_type_t;

/*This section defines the fresh mode select*/
typedef enum wdg_mechanism_mode {
    WDG_MECHANISM_MODE1 = 0x1U, /*!< normal refresh mode */
    WDG_MECHANISM_MODE2 = 0x2U, /*!< window limit refresh mode */
    WDG_MECHANISM_MODE3 = 0x3U, /*!< seq delta refresh mode */
} wdg_mechanism_mode_e;

/*watchdog clock prescaler. bit16~bit31*/
typedef enum wdg_clock_prescaler {
    WDG_CLOCK_PRESCALER_DIV1 = 0x0U, /*!< div by 1 */
    WDG_CLOCK_PRESCALER_DIV2 = 0x1U, /*!< div by 2 */
    WDG_CLOCK_PRESCALER_DIV3 = 0x2U, /*!< div by 3 */
    WDG_CLOCK_PRESCALER_DIV4 = 0x3U, /*!< div by 4 */
    WDG_CLOCK_PRESCALER_DIV5 = 0x4U, /*!< div by 5 */
    WDG_CLOCK_PRESCALER_DIV6 = 0x5U, /*!< div by 6 */
    WDG_CLOCK_PRESCALER_DIV7 = 0x6U, /*!< div by 7 */
    WDG_CLOCK_PRESCALER_DIV8 = 0x7U, /*!< div by 8 */
} wdg_clock_prescaler_e;

typedef enum wdg_clock_source {
    WDG_CLOCK_MAIN = 0x0U,
    WDG_CLOCK_BUS = 0x1U,
    WDG_CLOCK_EXT = 0x2U,
    WDG_CLOCK_TIEOFF = 0x3U,
    WDG_CLOCK_LP = 0x4U,
    WDG_CLOCK_MAX = 0x5U,
} wdg_clock_source_e;

uint32_t sdrv_watchdog_lld_reg_poll_value(wdg_reg_type_t *base, uint32_t reg,
                                          uint32_t start, uint32_t width,
                                          uint32_t value, uint32_t retrycount);
void sdrv_watchdog_lld_source_select(wdg_reg_type_t *regs, bool enable_from_reg,
                                     bool wtc_from_reg);
void sdrv_watchdog_lld_config_clock_source(wdg_reg_type_t *regs,
                                           wdg_clock_source_e clk);
void sdrv_watchdog_lld_config_terminal_count(wdg_reg_type_t *regs,
                                             uint32_t timeout);
void sdrv_watchdog_lld_config_refresh_mechanism(wdg_reg_type_t *regs,
                                                wdg_mechanism_mode_e mode);
void sdrv_watchdog_lld_wdt_module_enable(wdg_reg_type_t *regs);
void sdrv_watchdog_lld_wdt_module_disable(wdg_reg_type_t *regs);
void sdrv_watchdog_lld_wdt_soft_reset(wdg_reg_type_t *regs);
void sdrv_watchdog_lld_refresh_trigger(wdg_reg_type_t *regs);

#endif /* SDRV_WATCHDOG_H_ */