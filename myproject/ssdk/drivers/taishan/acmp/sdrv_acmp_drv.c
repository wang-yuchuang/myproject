/**
 * @file sdrv_acmp_drv.c
 * @brief semidrive acmp driver
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>
#include <bits.h>
#include <debug.h>
#include <reg.h>

#include <driver/irq.h>
#include <driver/taishan/sdrv_acmp_drv.h>

#include "sdrv_acmp.h"

/**
 * @brief acmp irq handler
 *
 * @param[in] irq irq number
 * @param[in] arg irq arg
 * return irq handler result
 */
static int acmp_irq_handler(uint32_t irq, void *arg)
{
    acmp_dev_t *dev = arg;

    irq_state_t flag;
    uint32_t irq_state;
    int ret = 0;

    ASSERT(dev);

    flag = spin_lock_irqsave(&dev->lock);

    irq_state = sdrv_acmp_lld_get_irq_state(dev->base);

    if (irq_state) {
        if (dev->callback) {
            ret = dev->callback(dev, irq_state);
        }

        sdrv_acmp_lld_clear_irq(dev->base, irq_state);
    }

    spin_unlock_irqrestore(&dev->lock, flag);

    return ret;
}

/**
 * @brief acmp init
 *
 * @param[in] dev acmp dev
 * @param[in] acmp_cfg sdrv acmp config
 * @param[in] callback acmp callback
 */
void acmp_init(acmp_dev_t *dev, acmp_config_t *cfg, acmp_callback callback)
{
    uint32_t i = 0;
    dev->base = cfg->base;
    dev->irq = cfg->irq;
    dev->callback = callback;

    spin_lock_init(&dev->lock);

    sdrv_acmp_lld_init(dev->base);

    /* config mode */
    sdrv_acmp_lld_config_mode(dev->base, cfg->mode);

    if (cfg->mode == ACMP_IT_MODE) {
        sdrv_acmp_lld_config_it_mode(dev->base, cfg->qstart, cfg->qend, cfg->cur_entry_pt);

        sdrv_acmp_lld_config_timer(dev->base, cfg->timer_compare_val, cfg->timer_terminal_val);
    }

    /* config csel */
    if ((cfg->csel_num > 0) && (cfg->csel_num <= ACMP_MAX_CSEL_NUM)) {
        for (i = 0; i < cfg->csel_num; i++) {
            sdrv_acmp_lld_config_csel(dev->base, &cfg->csel_cfg[i]);
        }
    }

    /* config index */
    if ((cfg->index_num > 0) && (cfg->index_num <= ACMP_MAX_INDEX_NUM)) {
        for (i = 0; i < cfg->index_num; i++) {
            sdrv_acmp_lld_config_index(dev->base, &cfg->index_cfg[i]);
        }
    }

    /* set low power req_n delay */
    if (cfg->req_n_dly) {
        sdrv_acmp_lld_low_power(dev->base, cfg->req_n_dly);
    }

    /* competitor recv q channel low power request */
    if (cfg->qch_cmp_en) {
        sdrv_acmp_lld_rcv_qch_cmp(dev->base, cfg->qch_cmp_en);
    }

    /* dac recv q channel low power request */
    if (cfg->qch_dac_en) {
        sdrv_acmp_lld_rcv_qch_dac(dev->base, cfg->qch_dac_en);
    }

    /* start it generate */
    if (cfg->mode == ACMP_IT_MODE) {
        sdrv_acmp_lld_it_gen_start(dev->base, cfg->tg_start);
    }

    if (dev->irq >= 0) {
        irq_attach(dev->irq, acmp_irq_handler, dev);
        irq_enable(dev->irq);
    }
}

/**
 * @brief acmp start
 *
 * @param[in] dev acmp dev
 * @param[in] start acmp start
 */
void acmp_start(acmp_dev_t *dev, bool start)
{
    sdrv_acmp_lld_start(dev->base, start);
}

/**
 * @brief acmp IT mode trigger start
 *
 * @param[in] dev acmp dev
 * @param[in] qstart sdrv acmp trigger start id number for loop
 * @param[in] qend sdrv acmp trigger end id number for loop
 * @param[in] cur_entry_pt sdrv acmp trigger first access id number
 * @param[in] start acmp IT mode trigger start
 */
void acmp_it_trigger_start(acmp_dev_t *dev, uint8_t qstart, uint8_t qend, uint8_t cur_entry_pt,
                           bool start)
{
    sdrv_acmp_lld_config_it_mode(dev->base, qstart, qend, cur_entry_pt);

    sdrv_acmp_lld_it_gen_start(dev->base, start);
}

/**
 * @brief acmp power ctrl
 *
 * @param[in] dev acmp dev
 * @param[in] down 0 power on (default), 1 power down.
 */
void acmp_power_ctrl(acmp_dev_t *dev, bool down)
{
    sdrv_acmp_lld_power_down_cmp(dev->base, down);

    sdrv_acmp_lld_power_down_dac(dev->base, down);
}

/**
 * @brief acmp set callback
 *
 * @param[in] dev acmp dev
 * @param[in] callback acmp callback.
 */
void acmp_set_callback(acmp_dev_t *dev, acmp_callback callback)
{
    dev->callback = callback;
}

/**
 * @brief acmp deinit
 *
 * @param[in] dev acmp dev
 */
void acmp_deinit(acmp_dev_t *dev)
{
    if (dev->irq > 0) {
        irq_disable(dev->irq);
        irq_detach(dev->irq);
    }

    sdrv_acmp_lld_deinit(dev->base);
}
