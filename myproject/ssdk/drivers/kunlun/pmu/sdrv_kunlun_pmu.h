/*
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __PMU_DRV_H
#define __PMU_DRV_H
#include <types.h>
#include <compiler.h>
#include <debug.h>

__BEGIN_CDECLS;

/* pmu register offset */
#define PMU_EXT_CRTL            (0x00)
#define PMU_RTC_CTRL            (0x04)
#define PMU_GF_CTRL             (0x08)
#define PMU_RST_CTRL            (0x0c)
#define PMU_CTRL_0              (0x10)
#define PMU_CTRL_1              (0x14)
#define PMU_CTRL_2              (0x18)
#define PMU_CTRL_3              (0x1c)
#define PMU_INTR                (0x20)
#define PMU_STS_0               (0x30)
#define PMU_STS_1               (0x34)
#define PMU_STS_2               (0x38)
#define PMU_STS_3               (0x3c)


#define PMU_INIT                0x01
#define PMU_PWRUP               0x02
#define PMU_ON                  0x04
#define PMU_PWRDOWN             0x08
#define PMU_OFF                 0x10

/* power control id */
#define PWR_CTRL_0              0
#define PWR_CTRL_1              1
#define PWR_CTRL_2              2
#define PWR_CTRL_3              3

#define PWR_CTRL_INPUT          1
#define PWR_CTRL_OUTPUT         0
#define PWR_CTRL_OUTPUT_MANUAL  0
#define PWR_CTRL_OUTPUT_AUTO    1
#define PWR_CTRL_OUTPUT_LOW     0
#define PWR_CTRL_OUTPUT_HIGH    1

#define DISABLE                 0
#define ENABLE                  1
#define LOW_LEVEL               0
#define HIGH_LEVEL              1


typedef struct sdrv_pmu_lld_device {
	const char *name;
	int32_t id;
	uint32_t init_flag;
	volatile uint8_t *base_paddr;
	volatile uint8_t *base_vaddr;
	void *priv_data;
} pmudev_t;

#define sdrv_pmu_lld_dbg(str, x...) \
    do { ssdk_printf(SSDK_DEBUG, "%s:%d: "str, __func__, __LINE__, ## x); } while (0)
#define sdrv_pmu_lld_err(str, x...) \
    do { ssdk_printf(SSDK_ERR, "%s:%d: "str, __func__, __LINE__, ## x); } while (0)

// #define sdrv_pmu_lld_dbg(str, x...)
// #define sdrv_pmu_lld_err(str, x...)

// int32_t sdrv_pmu_lld_init(addr_t base);
// int32_t sdrv_pmu_lld_exit(addr_t base);

/**************************************************
 * sdrv_pmu_lld_get_status
 *
 * status value 0x01 - INIT Mode
 *              0x02 - PWRUP Mode
 *              0x04 - ON Mode
 *              0x08 - PWRDOWN Mode
 *              0x10 - OFF Mode
 *              < 0  - error
 ************************************************/
int32_t sdrv_pmu_lld_get_status(addr_t base, int *status);

int32_t sdrv_pmu_lld_get_event_source(addr_t base, uint32_t *event);

int32_t sdrv_pmu_lld_clean_single_event_source(addr_t base, int event_id);

int32_t sdrv_pmu_lld_clean_all_event_source(addr_t base);

int32_t sdrv_pmu_lld_powerdown(addr_t base, int32_t status);

/******************************************************************************
 * sdrv_pmu_lld_set_powerdown_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerdown_delay(addr_t base, int id, int delay);


/******************************************************************************
 * sdrv_pmu_lld_set_powerup_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerup_delay(addr_t base, int id, int delay);

/******************************************************************************
 * sdrv_pmu_lld_set_powerctrl_powerdown_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerctrl_powerdown_delay(addr_t base, int ctrl_id, int delay);

/******************************************************************************
 * sdrv_pmu_lld_set_powerctrl_powerup_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerctrl_powerup_delay(addr_t base, int ctrl_id, int delay);

int32_t sdrv_pmu_lld_set_powerctrl_io_mode(addr_t base, int ctrl_id, int mode);

int32_t sdrv_pmu_lld_set_powerctrl_out_mode(addr_t base, int ctrl_id, int mode);

int32_t sdrv_pmu_lld_set_powerctrl_out_ctrl(addr_t base, int ctrl_id, int out);

int32_t sdrv_pmu_lld_get_powerctrl_input_status(addr_t base, int ctrl_id, int *status);

int32_t sdrv_pmu_lld_set_internal_powerdown_enable(addr_t base, int id, int enable);

int32_t sdrv_pmu_lld_set_internal_powerdown_polarity(addr_t base, int id, int polarity);

int32_t sdrv_pmu_lld_get_internal_powerdown_status(addr_t base, int id, int *status);

int32_t sdrv_pmu_lld_set_external_reset_enable(addr_t base, int id, int enable);

int32_t sdrv_pmu_lld_set_external_reset_polarity(addr_t base, int id, int polarity);

int32_t sdrv_pmu_lld_set_external_reset_debounce_enable(addr_t base, int id, int enable);

int32_t sdrv_pmu_lld_set_external_reset_debounce_delay(addr_t base, int id, int delay);

int32_t sdrv_pmu_lld_get_external_reset_status(addr_t base, int id, int *status);

int32_t sdrv_pmu_lld_set_internal_wakeup_enable(addr_t base, int id, int enable);

int32_t sdrv_pmu_lld_set_external_wakeup_enable(addr_t base, int id, int enable);

int32_t sdrv_pmu_lld_set_external_wakeup_polarity(addr_t base, int id, int polarity);

int32_t sdrv_pmu_lld_set_external_wakeup_debounce_enable(addr_t base, int id, int enable);

int32_t sdrv_pmu_lld_set_external_wakeup_debounce_delay(addr_t base, int id, int delay);

int32_t sdrv_pmu_lld_get_external_wakeup_status(addr_t base, int id, int *status);

int32_t sdrv_pmu_lld_set_glitch_filter_delay(addr_t base, int delay);

int32_t sdrv_pmu_lld_set_glitch_filter_enable(addr_t base, int multiEnable);

int32_t sdrv_pmu_lld_set_por_powerdown_delay(addr_t base, int delay);

int32_t sdrv_pmu_lld_set_por_auto_powerdown(addr_t base, int enable);

int32_t sdrv_pmu_lld_set_por_force_powerdown(addr_t base, int enable);

__END_CDECLS;
#endif
