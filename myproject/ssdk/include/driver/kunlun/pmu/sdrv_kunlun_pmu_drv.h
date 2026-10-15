/**
 * @file sdrv_kunlun_pmu_drv.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: kunlun pmu driver impl header
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SDRV_PMU_DRV_H
#define INCLUDE_SDRV_PMU_DRV_H

#include <types.h>
#include <spinlock.h>

#include "pmu.h"

#define PMU_PWRPIN_NUM          PMU_PWRCTRL_INVALID
#define PMU_TRIGGER_CHAN_NUM    2

/* pmu polarity macro define */
#define PMU_POLARITY_LOW_ACTIVE     0
#define PMU_POLARITY_HIGH_ACTIVE    1
#define PMU_POLARITY_CFG_DEFAULT    PMU_POLARITY_LOW_ACTIVE

/* pmu pin cotrol macro define */
#define PMU_PWRPINCTL_INPUT_MODE    1
#define PMU_PWRPINCTL_OUTPUT_MODE   0
#define PMU_PWRPINCTL0_DEFAULT      PMU_PWRPINCTL_OUTPUT_MODE
#define PMU_PWRPINCTL1_DEFAULT      PMU_PWRPINCTL_OUTPUT_MODE
#define PMU_PWRPINCTL2_DEFAULT      PMU_PWRPINCTL_OUTPUT_MODE
#define PMU_PWRPINCTL3_DEFAULT      PMU_PWRPINCTL_INPUT_MODE

/* pmu pin out mode macro define */
#define PMU_PWRPIN_OUTCTL_AUTO      0
#define PMU_PWRPIN_OUTCTL_MANUAL    1
#define PMU_PWRPIN0_OUTCTL_DEFAULT  PMU_PWRPIN_OUTCTL_AUTO
#define PMU_PWRPIN1_OUTCTL_DEFAULT  PMU_PWRPIN_OUTCTL_AUTO
#define PMU_PWRPIN2_OUTCTL_DEFAULT  PMU_PWRPIN_OUTCTL_MANUAL
#define PMU_PWRPIN3_OUTCTL_DEFAULT  PMU_PWRPIN_OUTCTL_MANUAL

/* pmu control status macro define */
#define PMU_FUNC_DISABLE            0
#define PMU_FUNC_ENABLE             1


/* pmu state macro define */
#define PMU_INIT                0x01
#define PMU_PWRUP               0x02
#define PMU_ON                  0x04
#define PMU_PWRDOWN             0x08
#define PMU_OFF                 0x10

/* enumeration typedef */
typedef enum pmu_pwrCtl_pinId_enum pmu_pwrCtl_pinId_enum_t;
typedef enum pmu_event_type_enum pmu_event_type_enum_t;
typedef enum pmu_debounce_time_enum pmu_debounce_time_enum_t;
typedef enum pmu_GF_time_enum pmu_GF_time_enum_t;
typedef enum pmu_delay_time_enum pmu_delay_time_enum_t;

/**
 * @brief pmu control pin enum
 *
 */
enum pmu_pwrCtl_pinId_enum {
    PMU_PWRCTRL0,
    PMU_PWRCTRL1,
    PMU_PWRCTRL2,
    PMU_PWRCTRL3,

    PMU_PWRCTRL_INVALID,
};

/**
 * @brief pmu event and status enum
 *
 */
enum pmu_event_type_enum {
    PMU_INT_PWRDWN_0    = 1 << 0,
    PMU_INT_PWRDWN_1    = 1 << 1,
    PMU_EXT_RESET_0     = 1 << 2,
    PMU_EXT_RESET_1     = 1 << 3,
    PMU_EXT_WAKEUP_0    = 1 << 4,
    PMU_EXT_WAKEUP_1    = 1 << 5,
    PMU_RTC_WAKEUP_0    = 1 << 6,
    PMU_RTC_WAKEUP_1    = 1 << 7,
    PMU_MULTI_EVT_ALL   = ((1 << 8) - 1),

    PMU_INVALID,
};

/**
 * @brief debounce tiem enum, with 1 cycle offset
 *
 */
enum pmu_debounce_time_enum {
    PMU_DEBOUNCE_62US,      /** (2-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_94US,      /** (3-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_125US,     /** (4-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_250US,     /** (8-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_500US,     /** (16-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_1MS,       /** (32-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_2MS,       /** (64-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_4MS,       /** (128-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_10MS,      /** (320*1-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_50MS,      /** (320*5-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_100MS,     /** (320*10-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_200MS,     /** (320*20-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_500MS,     /** (320*50-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_1S,        /** (320*100-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_2S,        /** (320*200-1) cycle of rtc clock (32KHz) */
    PMU_DEBOUNCE_4S,        /** (320*400-1) cycle of rtc clock (32KHz) */

    PMU_DEBOUNCE_TIME_INVALID,
};


/**
 * @brief glitch filter time enum
 *
 */
enum pmu_GF_time_enum {
    PMU_GF_2_CYCLE,
    PMU_GF_3_CYCLE,
    PMU_GF_4_CYCLE,
    PMU_GF_8_CYCLE,
    PMU_GF_16_CYCLE,
    PMU_GF_32_CYCLE,
    PMU_GF_64_CYCLE,
    PMU_GF_128_CYCLE,

    PMU_GF_TIME_INVALID,
};

/**
 * @brief glitch filter ctl bit enum
 *
 */
enum pmu_GF_enable_enum {
    PMU_GF_ENABLE_EXT_RST0          = 1 << 0,
    PMU_GF_ENABLE_EXT_RST1          = 1 << 1,
    PMU_GF_ENABLE_EXT_WAKEUP0       = 1 << 2,
    PMU_GF_ENABLE_EXT_WAKEUP1       = 1 << 3,
    PMU_GF_ENABLE_INT_REQPD0        = 1 << 4,
    PMU_GF_ENABLE_INT_REQPD1        = 1 << 5,
    PMU_GF_ENABLE_AP_RST            = 1 << 6,
    PMU_GF_ENABLE_SAFETY_RST        = 1 << 7,
    PMU_GF_ENABLE_AP_PWRSTABLE      = 1 << 8,
    PMU_GF_ENABLE_SAFETY_PWRSTABLE  = 1 << 9,
    PMU_GF_ENABLE_RTC_PWRSTABLE     = 1 << 10,
    PMU_GF_ENABLE_ALL               = ((1 << 11) - 1),

    PMU_GF_ENABLE_INVALID,
};

/**
 * @brief pmu control delay time enum
 *
 */
enum pmu_delay_time_enum {
    PMU_CTRL_DELAY_0_US,
    PMU_CTRL_DELAY_30_US,
    PMU_CTRL_DELAY_60_US,
    PMU_CTRL_DELAY_120_US,
    PMU_CTRL_DELAY_250_US,
    PMU_CTRL_DELAY_500_US,
    PMU_CTRL_DELAY_750_US,
    PMU_CTRL_DELAY_1_MS,
    PMU_CTRL_DELAY_2_MS,
    PMU_CTRL_DELAY_3_MS,
    PMU_CTRL_DELAY_4_MS,
    PMU_CTRL_DELAY_5_MS,
    PMU_CTRL_DELAY_6_MS,
    PMU_CTRL_DELAY_7_MS,
    PMU_CTRL_DELAY_8_MS,
    PMU_CTRL_DELAY_16_MS,

    PMU_CTRL_DELAY_INVALID,
};

/**
 * @brief pmu por delay time enum
 *
 */
enum pmu_por_pwrdown_enum {
    PMU_POR_PD_DELAY_2_CYCLE,
    PMU_POR_PD_DELAY_4_CYCLE,
    PMU_POR_PD_DELAY_8_CYCLE,
    PMU_POR_PD_DELAY_16_CYCLE,
    PMU_POR_PD_DELAY_128_CYCLE,
    PMU_POR_PD_DELAY_1024_CYCLE,
    PMU_POR_PD_DELAY_4096_CYCLE,
    PMU_POR_PD_DELAY_16384_CYCLE,

    PMU_POR_PD_DELAY_INVALID,
};

/* pmu config structure declare */
struct pmu_pwrCtrl_cfg;
struct pmu_pwrPinCtrl_cfg;
struct pmu_reqPwrDown_cfg;
struct pmu_extReset_cfg;
struct pmu_extWakeup_cfg;

/* typedef for config */
typedef struct pmu_pwrCtrl_cfg pmu_pwrCtrl_cfg_t;
typedef struct pmu_pwrPinCtrl_cfg pmu_pwrPinCtrl_cfg_t;
typedef struct pmu_interTrigger_cfg pmu_interTrigger_cfg_t;
typedef struct pmu_extTrigger_cfg pmu_extTrigger_cfg_t;
typedef struct pmu_gf_cfg pmu_gf_cfg_t;
typedef struct pmu_por_cfg pmu_por_cfg_t;


/**
 * @brief pmu power control config
 *
 */
struct pmu_pwrCtrl_cfg {
    uint8_t pwrup_delay;
    uint8_t pwrdown_delay;
};

/**
 * @brief pmu pwrpin control config
 *
 */
struct pmu_pwrPinCtrl_cfg {
    uint8_t pinId;
    uint8_t pwrup_delay;
    uint8_t pwrdown_delay;
    uint8_t io_mode;
    uint8_t out_mode;
};

/**
 * @brief internal trigger config
 * include require powerdown and RTC wake up
 *
 */
struct pmu_interTrigger_cfg {
    bool enable;
    uint8_t chanId;
    uint8_t pol;
};

/**
 * @brief extern trigger config
 * include wakeup or reset
 *
 */
struct pmu_extTrigger_cfg {
    bool enanle;
    bool debounce_enable;
    uint8_t chanId;
    uint8_t pol;
    uint8_t debounce_delay;
};

/**
 * @brief pmu glitch filter config
 *
 */
struct pmu_gf_cfg {
    uint32_t enables;
    uint8_t gf_time;
};


/**
 * @brief pmu por control config
 *
 */
struct pmu_por_cfg {
    bool    porpd_auto;
    uint8_t porpd_delay;
};


/**
 * @brief kunlun pmu config structure
 *
 */
typedef struct pmu_config {
    paddr_t base;
    uint8_t irq;
    pmu_pwrCtrl_cfg_t* pwr_ctl;
    pmu_pwrPinCtrl_cfg_t* pwrPin_ctl[PMU_MAX_PWRPIN_NUM + 1];
    pmu_interTrigger_cfg_t* req_pwrdown[PMU_MAX_TRIGGER_CHAN_NUM + 1];
    pmu_interTrigger_cfg_t* rtc_wakeup[PMU_MAX_TRIGGER_CHAN_NUM + 1];
    pmu_extTrigger_cfg_t* ext_wakeup[PMU_MAX_TRIGGER_CHAN_NUM + 1];
    pmu_extTrigger_cfg_t* ext_reset[PMU_MAX_TRIGGER_CHAN_NUM + 1];
    pmu_gf_cfg_t* gf_ctl;
    pmu_por_cfg_t* por_ctl;
}sdrv_pmu_config_t;


/* sdrv pmu device */
typedef struct sdrv_pmu_dev {
    pmu_dev_t dev;
    const sdrv_pmu_config_t* cfg;
    spin_lock_t lock;
} sdrv_pmu_dev_t;


pmu_dev_t* sdrv_pmu_probe(sdrv_pmu_dev_t* sdrv_pmu_ptr, const sdrv_pmu_config_t* cfg_ptr);

#endif /* INCLUDE_SDRV_PMU_DRV_H */
/* end of header file sdrv_kunlun_pmu_drv.h */