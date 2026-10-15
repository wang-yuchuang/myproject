/**
 * @file ssdrv_uart_drv.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv pmu driver interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_SDRV_PMU_H
#define INCLUDE_DRV_SDRV_PMU_H

#include <compiler.h>
#include <types.h>

#include "driver/taishan/pmu/pmu.h"
#include "sdrv_pmu.h"

#include "sdrv_pmu.h"


#ifndef ASSEMBLY

__BEGIN_CDECLS
typedef struct sdrv_pmu_dev sdrv_pmu_dev_t;

/* sdrv acmp configurations */
typedef struct sdrv_pmu_config {
    paddr_t                base;
    int                    irq;
} sdrv_pmu_config_t;

/**
 * @brief PMU low level device.
 */
typedef struct sdrv_pmu_dev {
    paddr_t base;
    int irq;
    void *priv;
} sdrv_pmu_dev_t;

__END_CDECLS

/**
 * @brief enable button ctrl
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_bc_ctrl_t pmu button control type
 * @return NONE
 */
void sdrv_pmu_bc_enable(sdrv_pmu_dev_t *device,
                        const sdrv_pmu_bc_ctrl_t *cfg);

/**
 * @brief disable button ctrl
 *
 * @param[in] device pmu device property
 * @return NONE
 */
void sdrv_pmu_bc_disable(sdrv_pmu_dev_t *device);

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
                               const sdrv_pmu_filter_t *filter);

/**
 * @brief set the button ctrl window, effective only when button level sensitive
 * detect mode. default only enable min ON/OFF window detect
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_bc_window_t type define the window monitor time and
 * enable or disable
 * @return uart dev
 */
void sdrv_pmu_bc_window_detect(sdrv_pmu_dev_t *device,
                               const sdrv_pmu_bc_window_t *window);

/**
 * @brief enable pmu wakeup
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_wakeup_t pmu wakeup property, define the wakeup polarity,
 * detect mode, suppport internale wakeup
 * @return NONE
 */
void sdrv_pmu_wakeup_enable(sdrv_pmu_dev_t *device,
                            const sdrv_pmu_wakeup_t *ctrl);

/**
 * @brief disable pmu wakeup
 *
 * @param[in] device pmu device property
 * @return NONE
 */
void sdrv_pmu_wakeup_disable(sdrv_pmu_dev_t *device);

/**
 * @brief wakeup filter, effective only when edge detect mode enable
 *
 * @param[in] device pmu device property
 * @param[in] sdrv_pmu_filter_t filter type include enable or disable hardware
 * filter, filter edge and filter counter and interval
 * @return NONE
 */
void sdrv_pmu_wakeup_filter(sdrv_pmu_dev_t *device, uint8_t chn,
                            sdrv_pmu_filter_t *filter);

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
uint32_t sdrv_pmu_wakeup_reason(sdrv_pmu_dev_t *device);

/**
 * @brief trigger pmu power down
 *
 * @param[in] device pmu device property
 */
void sdrv_pmu_pwrdown_request(sdrv_pmu_dev_t *device);

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
                                 uint8_t enable);

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
                                uint8_t delay, uint8_t delay_adj);

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
                               uint8_t mode);

/**
 * @brief pwr_ctrl polarity, low active or high active
 *
 * @param[in] device pmu device property
 * @param[in] id pwr ctrl id range 0~3
 * @param[in] polarity 0:high active 1:low active
 * @return always return 0
 */
uint32_t sdrv_pmu_pwrctrl_polarity(sdrv_pmu_dev_t *device, uint8_t id,
                                   uint8_t polarity);

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
                               uint8_t enable);

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
                              uint8_t delay, uint8_t delay_adj);

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
                             uint8_t id, uint8_t mode);

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
                               uint8_t target);

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
                               bool on_off);

/**
 * @brief sdrv pmu init
 *
 * @param[in] device pmu device
 * @param[in] pmu_cfg pmu config
 */
void sdrv_pmu_init(sdrv_pmu_dev_t *dev, sdrv_pmu_config_t *pmu_cfg);

#endif

#endif
