/**
 * @file sdrv_kunlun_pmu_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: kunlun pmu driver impl
 *
 * Revision History:
 * -----------------
*/
#include <debug.h>
#include <types.h>
#include <string.h>
#include <board.h>

#include <driver/kunlun/pmu/sdrv_kunlun_pmu_drv.h>
#include <../drivers/kunlun/pmu/sdrv_kunlun_pmu.h>
#include <driver/kunlun/pmu/pmu.h>

#define SDRV_PMU_DEBUG_LVL      SSDK_WARNING

#define EXTRA_FMT   "sdrv_pmu:"

#if SDRV_PMU_DEBUG_LVL >= SSDK_ERR
#define SDRV_PMU_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_PMU_ERR(format, ...)
#endif

#if SDRV_PMU_DEBUG_LVL >= SSDK_WARNING
#define SDRV_PMU_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_PMU_WARN(format, ...)
#endif

#if SDRV_PMU_DEBUG_LVL >= SSDK_INFO
#define SDRV_PMU_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define SDRV_PMU_INFO(format, ...)
#endif


static int32_t sdrv_pmu_init(pmu_dev_t* dev);
static int32_t sdrv_pmu_deinit(pmu_dev_t* dev);
static int32_t sdrv_pmu_get_status(pmu_dev_t* dev, int32_t* sta);
static int32_t sdrv_pmu_force_pwrdown(pmu_dev_t* dev, int32_t sta);
static int32_t sdrv_pmu_get_event_source(pmu_dev_t* dev, uint32_t* allEvtSrc);
static int32_t sdrv_pmu_clear_event_source(pmu_dev_t* dev, uint32_t multiEvt);
static int32_t sdrv_pmu_get_triger_status(pmu_dev_t* dev, int32_t type, int32_t* sta);
static int32_t sdrv_pmu_set_out_pwrpin(pmu_dev_t* dev, int32_t pinId, int32_t lev);
static int32_t sdrv_pmu_get_input_pwrpin(pmu_dev_t* dev, int32_t pinId, int32_t* lev);
static int32_t sdrv_pmu_set_por_force_pwrdown(pmu_dev_t* dev, int32_t enable);

// static pmu_ops_t g_sdrv_pmu_ops = {

// };

static pmu_ops_t s_sdrv_pmu_ops = {
    .init = sdrv_pmu_init,
    .deinit = sdrv_pmu_deinit,
    .get_status = sdrv_pmu_get_status,
    .change_state = sdrv_pmu_force_pwrdown,
    .get_event_source = sdrv_pmu_get_event_source,
    .clear_event_source = sdrv_pmu_clear_event_source,
    .get_triger_status = sdrv_pmu_get_triger_status,
    .set_pwr_out_status = sdrv_pmu_set_out_pwrpin,
    .get_pwr_input_status = sdrv_pmu_get_input_pwrpin,
    .set_por_force_pwrdown = sdrv_pmu_set_por_force_pwrdown,
};

/**
 * @brief init pmu device
 *
 * @param dev pmu device
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 fail
 */
int32_t sdrv_pmu_init(pmu_dev_t* dev)
{
    sdrv_pmu_dev_t *sdrv_pmu_ptr = dev->priv;
    sdrv_pmu_config_t* sdrv_cfg_ptr = sdrv_pmu_ptr->cfg;

    // spin_lock_init(&sdrv_pmu_ptr->lock);

    // config pwr control
    if (sdrv_cfg_ptr->pwr_ctl != NULL) {
        uint8_t pwrup_delay = sdrv_cfg_ptr->pwr_ctl->pwrup_delay;
        uint8_t pwrdown_delay = sdrv_cfg_ptr->pwr_ctl->pwrdown_delay;

        // ssdk_printf(SSDK_DEBUG, "[debug]up delay: %d, down delay: %d \n", pwrup_delay, pwrdown_delay);
        sdrv_pmu_lld_set_powerup_delay(dev->base, 0, pwrup_delay);
        sdrv_pmu_lld_set_powerdown_delay(dev->base, 0, pwrdown_delay);
    }
    else {
        // do nothing, use chip default
    }

    // config power control pin config
    int32_t cnt = 0;
    while (sdrv_cfg_ptr->pwrPin_ctl[cnt] != NULL &&
            cnt < PMU_PWRPIN_NUM) {
        uint8_t pinId = sdrv_cfg_ptr->pwrPin_ctl[cnt]->pinId;
        uint8_t pwrup_delay = sdrv_cfg_ptr->pwrPin_ctl[cnt]->pwrup_delay;
        uint8_t pwrdown_delay = sdrv_cfg_ptr->pwrPin_ctl[cnt]->pwrdown_delay;
        uint8_t io_mode = sdrv_cfg_ptr->pwrPin_ctl[cnt]->io_mode;
        uint8_t out_mode = sdrv_cfg_ptr->pwrPin_ctl[cnt]->out_mode;
        ssdk_printf(SSDK_DEBUG, "[debug]pin %d config: up delay: %d, down delay: %d, io mode: %d, out mode: %d\n"
                                , pinId
                                , pwrup_delay
                                , pwrdown_delay
                                , io_mode
                                , out_mode);

        sdrv_pmu_lld_set_powerctrl_powerdown_delay(dev->base, pinId, pwrdown_delay);
        sdrv_pmu_lld_set_powerctrl_powerup_delay(dev->base, pinId, pwrup_delay);
        sdrv_pmu_lld_set_powerctrl_io_mode(dev->base, pinId, io_mode);
        sdrv_pmu_lld_set_powerctrl_out_mode(dev->base, pinId, out_mode);

        cnt++;
    }

    // req power down config
    cnt = 0;
    while (sdrv_cfg_ptr->req_pwrdown[cnt] != NULL &&
            cnt < PMU_TRIGGER_CHAN_NUM) {
        uint8_t chanId = sdrv_cfg_ptr->req_pwrdown[cnt]->chanId;
        int32_t enable = sdrv_cfg_ptr->req_pwrdown[cnt]->enable;
        int32_t pol = sdrv_cfg_ptr->req_pwrdown[cnt]->pol;

        // ssdk_printf(SSDK_DEBUG, "[debug]reqdown chan: %d config: enable: %d, pol: %d\n", chanId, enable, pol);
        sdrv_pmu_lld_set_internal_powerdown_polarity(dev->base, chanId, pol);
        sdrv_pmu_lld_set_internal_powerdown_enable(dev->base, chanId, enable);

        cnt++;
    }

    // rtc wakeup config
    cnt = 0;
    while (sdrv_cfg_ptr->rtc_wakeup[cnt] != NULL &&
            cnt < PMU_TRIGGER_CHAN_NUM) {
        uint8_t chanId = sdrv_cfg_ptr->rtc_wakeup[cnt]->chanId;
        int32_t enable = sdrv_cfg_ptr->rtc_wakeup[cnt]->enable;
        // ssdk_printf(SSDK_DEBUG, "[debug]rtc wakeup chan: %d config: enable: %d\n", chanId, enable);

        sdrv_pmu_lld_set_internal_wakeup_enable(dev->base, chanId, enable);
        cnt++;
    }

    // external wakeup config
    cnt = 0;
    while (sdrv_cfg_ptr->ext_wakeup[cnt] != NULL &&
            cnt < PMU_TRIGGER_CHAN_NUM) {
        uint8_t chanId = sdrv_cfg_ptr->ext_wakeup[cnt]->chanId;
        int32_t enable = sdrv_cfg_ptr->ext_wakeup[cnt]->enanle;
        int32_t pol = sdrv_cfg_ptr->ext_wakeup[cnt]->pol;
        int32_t debounce_enable = sdrv_cfg_ptr->ext_wakeup[cnt]->debounce_enable;
        int32_t debounce_delay = sdrv_cfg_ptr->ext_wakeup[cnt]->debounce_delay;

        // ssdk_printf(SSDK_DEBUG, "[debug]ext wakeup chan: %d config: enable: %d, pol: %d, dbenable: %d, db delay: %d\n"
        //                         , chanId
        //                         , enable
        //                         , pol
        //                         , debounce_enable
        //                         , debounce_delay);

        sdrv_pmu_lld_set_external_wakeup_debounce_delay(dev->base, chanId, debounce_delay);
        sdrv_pmu_lld_set_external_wakeup_debounce_enable(dev->base, chanId, debounce_enable);
        sdrv_pmu_lld_set_external_wakeup_polarity(dev->base, chanId, pol);
        sdrv_pmu_lld_set_external_wakeup_enable(dev->base, chanId, enable);
        cnt++;
    }

    // external reset config
    cnt = 0;
    while (sdrv_cfg_ptr->ext_reset[cnt] != NULL &&
            cnt < PMU_TRIGGER_CHAN_NUM) {
        uint8_t chanId = sdrv_cfg_ptr->ext_reset[cnt]->chanId;
        int32_t enable = sdrv_cfg_ptr->ext_reset[cnt]->enanle;
        int32_t pol = sdrv_cfg_ptr->ext_reset[cnt]->pol;
        int32_t debounce_enable = sdrv_cfg_ptr->ext_reset[cnt]->debounce_enable;
        int32_t debounce_delay = sdrv_cfg_ptr->ext_reset[cnt]->debounce_delay;
        // ssdk_printf(SSDK_DEBUG, "[debug]ext reset chan: %d config: enable: %d, pol: %d, dbenable: %d, db delay: %d\n"
        //                         , chanId
        //                         , enable
        //                         , pol
        //                         , debounce_enable
        //                         , debounce_delay);

        sdrv_pmu_lld_set_external_reset_debounce_delay(dev->base, chanId, debounce_delay);
        sdrv_pmu_lld_set_external_reset_debounce_enable(dev->base, chanId, debounce_enable);
        // should wait at least 40ns (more than one cycle in 32KHz) between
        // EXT_RESET_B_0_POL setting and EXT_RESET_B_0_EN setting
        sdrv_pmu_lld_set_external_reset_polarity(dev->base, chanId, pol);
        udelay(1);
        sdrv_pmu_lld_set_external_reset_enable(dev->base, chanId, enable);
        cnt++;
    }

    // external Glitch filter configuration
    if (sdrv_cfg_ptr->gf_ctl != NULL) {
        int32_t enable = sdrv_cfg_ptr->gf_ctl->enables;
        int32_t gf_time = sdrv_cfg_ptr->gf_ctl->gf_time;
        // ssdk_printf(SSDK_DEBUG, "[debug]gf config: enable: %d, gf time: %d\n"
        //                         , enable
        //                         , gf_time);

        sdrv_pmu_lld_set_glitch_filter_enable(dev->base, enable);
        sdrv_pmu_lld_set_glitch_filter_delay(dev->base, gf_time);
    }
    else {
        // do nothing, use chip default
    }

    // por config
    if (sdrv_cfg_ptr->por_ctl != NULL) {
        int32_t is_auto = sdrv_cfg_ptr->por_ctl->porpd_auto;
        int32_t delay = sdrv_cfg_ptr->por_ctl->porpd_delay;
        // ssdk_printf(SSDK_DEBUG, "[debug]por config: is auto: %d, delay time: %d\n"
        //                         , is_auto
        //                         , delay);

        sdrv_pmu_lld_set_por_auto_powerdown(dev->base, is_auto);
        sdrv_pmu_lld_set_por_powerdown_delay(dev->base, delay);
    }
    else {
        // do nothing, use chip default
    }

    return 0;
}

int32_t sdrv_pmu_deinit(pmu_dev_t* dev)
{
    SDRV_PMU_INFO("sdrv pmu deinit \n");
    return 0;
}

int32_t sdrv_pmu_get_status(pmu_dev_t* dev, int32_t* sta)
{
    if (dev == NULL || sta == NULL) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    return sdrv_pmu_lld_get_status(dev->base, sta);
}

int32_t sdrv_pmu_force_pwrdown(pmu_dev_t* dev, int32_t sta)
{
    if (dev == NULL || sta != PMU_PWRDOWN) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    // Software can force PMU to enter PMU_OFF
    // state by setting this field bit from 0x0 to 0x1
    sdrv_pmu_lld_powerdown(dev->base, 0);
    udelay(1);
    sdrv_pmu_lld_powerdown(dev->base, 1);

    return 0;
}

int32_t sdrv_pmu_get_event_source(pmu_dev_t* dev, uint32_t* allEvtSrc)
{
    if (dev == NULL || allEvtSrc == NULL) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    return sdrv_pmu_lld_get_event_source(dev->base, allEvtSrc);
}

int32_t sdrv_pmu_clear_event_source(pmu_dev_t* dev, uint32_t multiEvt)
{
    if (dev == NULL) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    if (multiEvt == PMU_MULTI_EVT_ALL) {
        return sdrv_pmu_lld_clean_all_event_source(dev->base);
    }
    else {
        int32_t cnt = 0;
        while (((1 << cnt) & PMU_MULTI_EVT_ALL) != 0) {
            if (((1 << cnt) & multiEvt) != 0) {
                sdrv_pmu_lld_clean_single_event_source(dev->base, cnt);
            }
            else {
                // do nothing, current evt is not clear
            }
            cnt ++;
        }
    }

    return 0;
}
int32_t sdrv_pmu_get_triger_status(pmu_dev_t* dev, int32_t type, int32_t* sta)
{
    if (dev == NULL || sta == NULL) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    int32_t (*func)(addr_t base, int32_t chan, int32_t* sta);
    int32_t chan = 0;

    func = NULL;

    switch (type) {
        case PMU_INT_PWRDWN_0:
            func = sdrv_pmu_lld_get_internal_powerdown_status;
            chan = 0;
            break;
        case PMU_INT_PWRDWN_1:
            func = sdrv_pmu_lld_get_internal_powerdown_status;
            chan = 1;
            break;
        case PMU_EXT_RESET_0:
            func = sdrv_pmu_lld_get_external_reset_status;
            chan = 0;
            break;
        case PMU_EXT_RESET_1:
            func = sdrv_pmu_lld_get_external_reset_status;
            chan = 1;
            break;
        case PMU_EXT_WAKEUP_0:
            func = sdrv_pmu_lld_get_external_wakeup_status;
            chan = 0;
            break;
        case PMU_EXT_WAKEUP_1:
            func = sdrv_pmu_lld_get_external_wakeup_status;
            chan = 1;
            break;
        default:
            func = NULL;
            SDRV_PMU_ERR("invalid type for %s.\n", __FUNCTION__);
            break;
    }

    //@TODO convert logic level to real status
    if (func != NULL) {
        return func(dev->base, chan, sta);
    }
    else {
        return -1;
    }
}

int32_t sdrv_pmu_set_out_pwrpin(pmu_dev_t* dev, int32_t pinId, int32_t lev)
{
    if (dev == NULL || pinId >= PMU_PWRCTRL_INVALID ||
        lev < 0 || lev > 1) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    //@TODO set out level deps config
    return sdrv_pmu_lld_set_powerctrl_out_ctrl(dev->base, pinId, lev);
}

int32_t sdrv_pmu_get_input_pwrpin(pmu_dev_t* dev, int32_t pinId, int32_t* lev)
{
    if (dev == NULL || pinId >= PMU_PWRCTRL_INVALID) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    //@TODO get input status deps config
    return sdrv_pmu_lld_get_powerctrl_input_status(dev->base, pinId, lev);
}

int32_t sdrv_pmu_set_por_force_pwrdown(pmu_dev_t* dev, int32_t enable)
{
    if (dev == NULL) {
        SDRV_PMU_ERR("%s parameter error.\n", __FUNCTION__);
        return -1;
    }

    return sdrv_pmu_lld_set_por_force_powerdown(dev->base, enable);
}

/**
 * @brief sdrv pmu device probe
 *
 * @param sdrv_pmu sdrv pmu dev
 * @param cfg pmu config
 * @return pmu_dev_t*  pmu device pointer
 */
pmu_dev_t* sdrv_pmu_probe(sdrv_pmu_dev_t* sdrv_pmu_ptr, const sdrv_pmu_config_t* cfg_ptr)
{
    pmu_dev_t* dev;

    if (sdrv_pmu_ptr == NULL || cfg_ptr == NULL) {
        SDRV_PMU_ERR("invalid parameter for pmu device probeing \n");
        return NULL;
    }

    sdrv_pmu_ptr->cfg = cfg_ptr;

    dev = &sdrv_pmu_ptr->dev;
    dev->base = cfg_ptr->base;
    dev->ops = &s_sdrv_pmu_ops;
    dev->priv = sdrv_pmu_ptr;

    pmu_init(dev);

    return dev;
}

/* end of file sdrv_kunlun_pmu_drv.c */