/**
 * @file reset.c
 * @brief reset common driver.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdlib.h>

#include "driver/reset.h"

/**
 * @brief Assert a reset signal.
 * The reset singal will stay asserted until reset_deassert() is called.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_assert(struct reset_sig *rst_sig)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(rst_sig && rst_sig->rst_ctl);

    ctl = rst_sig->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->assert) {
        ret = ctl->ops->assert(ctl, rst_sig->id);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Deassert a reset signal.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_deassert(struct reset_sig *rst_sig)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(rst_sig && rst_sig->rst_ctl);

    ctl = rst_sig->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->deassert) {
        ret = ctl->ops->deassert(ctl, rst_sig->id);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Assert a reset signal, and deassert automatically.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_reset(struct reset_sig *rst_sig)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(rst_sig && rst_sig->rst_ctl);

    ctl = rst_sig->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->reset) {
        ret = ctl->ops->reset(ctl, rst_sig->id);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Check reset signal status.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if deasserted, positive if asserted,
 *         or a negative error code.
 */
int32_t reset_ctl_status(struct reset_sig *rst_sig)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(rst_sig && rst_sig->rst_ctl);

    ctl = rst_sig->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->status) {
        ret = ctl->ops->status(ctl, rst_sig->id);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Read reset general reg.
 *
 * @param[in] general general reg.
 * @return general reg value.
 */
uint32_t reset_ctl_general_read(struct reset_general_reg *general)
{
    uint32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(general && general->rst_ctl);

    ctl = general->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->read_general) {
        ret = ctl->ops->read_general(ctl, general->id);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Write reset general reg.
 *
 * @param[in] general general reg.
 * @param[in] val write value.
 * @return 0 if success,
 *         or a negative error code.
 */
int32_t reset_ctl_general_write(struct reset_general_reg *general, uint32_t val)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(general && general->rst_ctl);

    ctl = general->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->write_general) {
        ret = ctl->ops->write_general(ctl, general->id, val);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Write reset general reg bit.
 *
 * @param[in] general general reg.
 * @param[in] start write start bit.
 * @param[in] width write bit width.
 * @param[in] val write value.
 * @return 0 if success,
 *         or a negative error code.
 */
int32_t reset_ctl_general_write_bit(struct reset_general_reg *general, uint8_t start, uint8_t width, uint32_t val)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(general && general->rst_ctl);

    ctl = general->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->write_general) {
        ret = ctl->ops->write_general_bit(ctl, general->id, start, width, val);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}

/**
 * @brief Initialize the reset controller.
 *
 * @param[in] rst_ctl Common reset controller.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_init(struct reset_ctl *rst_ctl)
{
    int32_t ret = 0;

    ASSERT(rst_ctl && rst_ctl->ops);

    spin_lock_init(&rst_ctl->lock);

    if (rst_ctl->ops->init) {
        ret = rst_ctl->ops->init(rst_ctl);
    }

    return ret;
}

/**
 * @brief Deinitialize the reset controller.
 *
 * @param[in] rst_ctl Common reset controller.
 */
void reset_ctl_deinit(struct reset_ctl *rst_ctl)
{
    irq_state_t state;

    state = spin_lock_irqsave(&rst_ctl->lock);

    if (rst_ctl->ops->deinit) {
        rst_ctl->ops->deinit(rst_ctl);
    }

    spin_unlock_irqrestore(&rst_ctl->lock, state);
}

/**
 * @brief Global reset.
 *
 * @param[in] rst_glb_ctl Global reset controller.
 */
void reset_ctl_global_reset(struct reset_glb_ctl *rst_glb_ctl)
{
    ASSERT(rst_glb_ctl);

    struct reset_ctl *rst_sf_ctl = rst_glb_ctl->rst_sf_ctl;
    struct reset_ctl *rst_ap_ctl = rst_glb_ctl->rst_ap_ctl;

    ASSERT(rst_sf_ctl && rst_ap_ctl && rst_sf_ctl->ops);

    if (rst_sf_ctl->ops->global_reset) {
        rst_sf_ctl->ops->global_reset(rst_sf_ctl, rst_ap_ctl);
    }

    return;
}

/**
 * @brief Config reset signal assert/deassert in lowpower mode.
 *
 * @param[in] rst_sig Reset signal.
 * @param[in] mode lowpower mode
 * @param[in] val 0:assert, 1:deassert
 *
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_lowpower_set(struct reset_sig *rst_sig, enum reset_lowpower_mode mode, uint32_t val)
{
    int32_t ret = -1;
    struct reset_ctl *ctl;
    irq_state_t state;

    ASSERT(rst_sig && rst_sig->rst_ctl);

    ctl = rst_sig->rst_ctl;

    state = spin_lock_irqsave(&ctl->lock);

    if (ctl->ops->lowpower_set) {
        ret = ctl->ops->lowpower_set(ctl, rst_sig->id, mode, val);
    }

    spin_unlock_irqrestore(&ctl->lock, state);

    return ret;
}
