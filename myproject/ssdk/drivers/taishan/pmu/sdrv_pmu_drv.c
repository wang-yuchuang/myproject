/**
 * @file sdrv_pmu_drv.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <debug.h>
#include <string.h>
#include <types.h>

#include "driver/taishan/pmu/sdrv_pmu_drv.h"

#include "driver/irq.h"
#include "sdrv_pmu.h"

#define EXTRA_FMT "sdrv_pmu:"
#define SDRV_PMU_ERR(format, ...)                                              \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_PMU_WARN(format, ...)                                             \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_PMU_INFO(format, ...)                                             \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/**
 * @brief enable button ctrl
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_bc_ctrl_t pmu button control type
 * @return NONE
 */
void sdrv_pmu_bc_enable(sdrv_pmu_dev_t *device,
                        const sdrv_pmu_bc_ctrl_t *ctrl)
{
    sdrv_pmu_bc_ctrl_t sdrv_pmu_bc_ctrl_cfg;

    memset(&sdrv_pmu_bc_ctrl_cfg, 0x0, sizeof(sdrv_pmu_bc_ctrl_t));
    sdrv_pmu_bc_ctrl_cfg.sensitive = ctrl->sensitive;
    sdrv_pmu_bc_ctrl_cfg.polarity = ctrl->polarity;
    sdrv_pmu_bc_ctrl_cfg.filterBypass = ctrl->filterBypass;
    sdrv_pmu_bc_ctrl_cfg.windowEnable = ctrl->windowEnable;

    sdrv_pmu_lld_bc_init(device->base, &sdrv_pmu_bc_ctrl_cfg);
}

/**
 * @brief disable button ctrl
 *
 * @param[in] device pmu device property
 * @return NONE
 */
void sdrv_pmu_bc_disable(sdrv_pmu_dev_t *device)
{
    sdrv_pmu_lld_bc_deinit(device->base);
}

/**
 * @brief button ctrl filter, effective only when the button edge sensitive
 * detect mode
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_filter_t filter type include enable or disable hardware
 * filter, filter edge and filter counter and interval
 * @return NONE
 */
void sdrv_pmu_bc_filter_enable(sdrv_pmu_dev_t *device,
                               const sdrv_pmu_filter_t *filter)
{
    /* TODO: no lld drive now */
}

/**
 * @brief set the button ctrl window, effective only when button level sensitive
 * detect mode. default only enable min ON/OFF window detect
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_bc_window_t type define the window monitor time and
 * enable or disable
 * @return uart dev
 */
void sdrv_pmu_bc_window_detect(sdrv_pmu_dev_t *device,
                               const sdrv_pmu_bc_window_t *window)
{
    if (window->max_on_enable == 0x1) {
        sdrv_pmu_lld_bc_max_on_window_enable(device->base,
                                             window->max_on_enable);
        sdrv_pmu_lld_bc_max_on_window(device->base, window->max_on_time);
    }

    if (window->max_off_enable == 0x1) {
        sdrv_pmu_lld_bc_max_off_window_enable(device->base,
                                              window->max_off_enable);
        sdrv_pmu_lld_bc_max_off_window(device->base, window->max_off_time);
    }

    sdrv_pmu_lld_bc_min_on_window(device->base, window->min_on_time);
    sdrv_pmu_lld_bc_min_off_window(device->base, window->min_off_time);
}

/**
 * @brief enable pmu wakeup
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_wakeup_t pmu wakeup property, define the wakeup polarity,
 * detect mode, suppport internale wakeup
 * @return NONE
 */
void sdrv_pmu_wakeup_enable(sdrv_pmu_dev_t *device,
                            const sdrv_pmu_wakeup_t *ctrl)
{
    sdrv_pmu_lld_wakeup_init(device->base, ctrl);
}

/**
 * @brief disable pmu wakeup
 *
 * @param[in] device pmu device property
 * @return NONE
 */
void sdrv_pmu_wakeup_disable(sdrv_pmu_dev_t *device)
{
    sdrv_pmu_lld_wakeup_deinit(device->base);
}

/**
 * @brief wakeup filter, effective only when edge detect mode enable
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_filter_t filter type include enable or disable hardware
 * filter, filter edge and filter counter and interval
 * @return NONE
 */
void sdrv_pmu_wakeup_filter(sdrv_pmu_dev_t *device, uint8_t chn,
                            sdrv_pmu_filter_t *filter)
{
    sdrv_pmu_lld_wakeup_filter(device->base, chn, filter->filter_cnt,
                               filter->filter_interval);
}

/**
 * @brief get wakeup reason:
 *         0:wakeup by button control
 *         1:rtc periodic interrupt
 *         2:wakeup button
 *         3:auto wakeup which is triggered by hardware power down request.
 *
 * @param[in] device pmu device property
 * @return wakeup resaon
 */
uint32_t sdrv_pmu_wakeup_reason(sdrv_pmu_dev_t *device)
{
    return sdrv_pmu_lld_get_wakeup_reason(device->base);
}

/**
 * @brief trigger pmu power down
 *
 * @param[in] device pmu device property
 */
void sdrv_pmu_pwrdown_request(sdrv_pmu_dev_t *device)
{
    sdrv_pmu_lld_pwrdown_request(device->base);
}

/**
 * @brief pwr_ctrl enable or disable, usually contrl the external regulator
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr ctrl id range 0~3
 * @param[in] enable:(ON)1 disbale:(OFF)0
 * @return always return 0
 */
uint32_t sdrv_pmu_pwrctrl_enable(sdrv_pmu_dev_t *device,
                                 sdrv_pmu_mode_t pmu_mode, uint8_t id,
                                 uint8_t enable)
{
    return sdrv_pmu_lld_set_pwrctrl_enable(device->base, pmu_mode, id, enable);
}

/**
 * @brief pwr_on enable or disable, contrl the internal regulator(LDO/DCDC)
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr on id range 0~3
 * @param[in] enable:(ON)1 disbale:(OFF)0
 * @return always return 0
 */
uint32_t sdrv_pmu_pwron_enable(sdrv_pmu_dev_t *device,
                               sdrv_pmu_mode_t pmu_mode, uint8_t id,
                               uint8_t enable)
{
    return sdrv_pmu_lld_set_pwron_enable(device->base, pmu_mode, id, enable);
}

/**
 * @brief set the delay time between power trigger and pwr_ctrl assert/de-assert
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr ctrl id range 0~3
 * @param[in] delay[4bit] total delay time = 2^delay*(2*delay_adj+1)
 * @param[in] delay_adj[2bit]
 * @return always return 0
 */
uint32_t sdrv_pmu_pwrctrl_delay(sdrv_pmu_dev_t *device,
                                sdrv_pmu_mode_t pmu_mode, uint8_t id,
                                uint8_t delay, uint8_t delay_adj)
{
    return sdrv_pmu_lld_set_pwrctrl_delay(device->base, pmu_mode, id, delay,
                                          delay_adj);
}

/**
 * @brief set the delay time between power trigger and pwr_on assert/de-assert
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr on id range 0~3
 * @param[in] delay[4bit]    total delay time = 2^delay*(2*delay_adj+1)
 * @param[in] delay_adj[2bit]
 * @return always return 0
 */
uint32_t sdrv_pmu_pwron_delay(sdrv_pmu_dev_t *device,
                              sdrv_pmu_mode_t pmu_mode, uint8_t id,
                              uint8_t delay, uint8_t delay_adj)
{
    return sdrv_pmu_lld_set_pwron_delay(device->base, pmu_mode, id, delay,
                                        delay_adj);
}

/**
 * @brief pwr_ctrl assert/de-assert auto when power state transiton
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr ctrl id range 0~3
 * @param[in] mode 1:auto 0:manual
 * @return always return 0
 */
uint32_t sdrv_pmu_pwrctrl_auto(sdrv_pmu_dev_t *device,
                               sdrv_pmu_mode_t pmu_mode, uint8_t id,
                               uint8_t mode)
{
    return sdrv_pmu_lld_set_pwrctrl_mode_transtion(device->base, pmu_mode, id,
            mode);
}

/**
 * @brief pwr_on assert/de-assert auto when power state transiton
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr ctrl id range 0~3
 * @param[in] mode 1:auto 0:manual
 * @return always return 0
 */
uint32_t sdrv_pmu_pwron_auto(sdrv_pmu_dev_t *device, sdrv_pmu_mode_t pmu_mode,
                             uint8_t id, uint8_t mode)
{
    return sdrv_pmu_lld_set_pwron_transtion_mode(device->base, pmu_mode, id,
            mode);
}

/**
 * @brief pwr_ctrl polarity, low active or high active
 *
 * @param[in] device pmu device property
 * @param[in] id pwr ctrl id range 0~3
 * @param[in] polarity 0:high active 1:low active
 * @return always return 0
 */
uint32_t sdrv_pmu_pwrctrl_polarity(sdrv_pmu_dev_t *device, uint8_t id,
                                   uint8_t polarity)
{
    return sdrv_pmu_lld_set_pwrctrl_polarity(device->base, id, polarity);
}

/**
 * @brief power on/off transition by sf domain or ap domain
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] id pwr on id range 0~3
 * @param[in] target 00:safe 01:ap
 * @return always return 0
 */
uint32_t sdrv_pmu_pwron_target(sdrv_pmu_dev_t *device,
                               sdrv_pmu_mode_t pmu_mode, uint8_t id,
                               uint8_t target)
{
    return sdrv_pmu_lld_set_pwron_target(device->base, pmu_mode, id, target);
}

/**
 * @brief pwr_on ctrl domain power on off config
 *
 * @param[in] device pmu device property
 * @param[in] pmu_mode RUN/SLEEP/HIBERNATE/RTC/OFF
 * @param[in] domain 0:SF, 1:AP
 * @param[in] on_off ON(true), OFF(false)
 * @return always return 0
 */
uint32_t sdrv_pmu_power_on_off(sdrv_pmu_dev_t *device,
                               sdrv_pmu_mode_t pmu_mode, uint32_t domain,
                               bool on_off)
{
    if (domain == 0 || domain == 1) {
        sdrv_pmu_lld_set_power_on_off(device->base, pmu_mode, domain, on_off);
    }

    return 0;
}

/**
 * @brief pmu init
 *
 * @param[in] device pmu device
 * @param[in] pmu_cfg pmu config
 */
void sdrv_pmu_init(sdrv_pmu_dev_t *dev, sdrv_pmu_config_t *pmu_cfg)
{
    dev->base = pmu_cfg->base;
    dev->irq = pmu_cfg->irq;

    return;
}

/**
 * @brief pmu deinit
 *
 * @param device
 * @return int32_t
 */
int32_t sdrv_pmu_deinit(sdrv_pmu_dev_t *device)
{
    return 0;
}
