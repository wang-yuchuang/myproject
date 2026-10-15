/**
 * @file reset.h
 * @brief reset driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef RESET_H_
#define RESET_H_

#include <debug.h>
#include <types.h>
#include <spinlock.h>

struct reset_ctl;

enum reset_lowpower_mode {
    RESET_LP_HIB   = 0,
    RESET_LP_SLEEP = 1,
};

/**
 * @brief reset hardware driver function table
 */
typedef struct reset_ctl_ops {
    /* initialize reset controller */
    int32_t (*init)(struct reset_ctl *rst_ctl);

    /* deinitialize reset controller */
    void (*deinit)(struct reset_ctl *rst_ctl);

    /* assert a reset signal */
    int32_t (*assert)(struct reset_ctl *rst_ctl, uint32_t id);

    /* deassert a reset signal */
    int32_t (*deassert)(struct reset_ctl *rst_ctl, uint32_t id);

    /* assert a reset signal, and deassert automatically */
    int32_t (*reset)(struct reset_ctl *rst_ctl, uint32_t id);

    /* check reset signal status */
    int32_t (*status)(struct reset_ctl *rst_ctl, uint32_t id);

    /* read reset general reg */
    uint32_t (*read_general)(struct reset_ctl *rst_ctl, uint32_t id);

    /* write reset general reg */
    int32_t (*write_general)(struct reset_ctl *rst_ctl, uint32_t id, uint32_t val);

    /* write reset general reg */
    int32_t (*write_general_bit)(struct reset_ctl *rst_ctl, uint32_t id, uint8_t start, uint8_t width, uint32_t val);

    /* global reset */
    void (*global_reset)(struct reset_ctl *sf_rst_ctl, struct reset_ctl *ap_rst_ctl);

    /* config reset sig assert/deassert in LP mode */
    int32_t (*lowpower_set)(struct reset_ctl *rst_ctl, uint32_t id, enum reset_lowpower_mode mode, uint32_t val);
} reset_ctl_ops_t;

/**
 * @brief abstract reset controller
 */
typedef struct reset_ctl {
    spin_lock_t         lock;
    reset_ctl_ops_t     *ops;
    void                *priv;
} reset_ctl_t;

/**
 * @brief abstract reset signal
 */
typedef struct reset_sig {
    struct reset_ctl    *rst_ctl;
    uint32_t            id;
} reset_sig_t;

/**
 * @brief abstract reset general reg
 */
typedef struct reset_general_reg {
    struct reset_ctl    *rst_ctl;
    uint32_t            id;
} reset_general_reg_t;

/**
 * @brief global reset controller
 */
typedef struct reset_glb_ctl {
    struct reset_ctl    *rst_sf_ctl;
    struct reset_ctl    *rst_ap_ctl;
} reset_glb_ctl_t;

/**
 * @brief Initialize the reset controller.
 *
 * @param[in] rst_ctl Common reset controller.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_init(struct reset_ctl *rst_ctl);

/**
 * @brief Deinitialize the reset controller.
 *
 * @param[in] rst_ctl Common reset controller.
 */
void reset_ctl_deinit(struct reset_ctl *rst_ctl);

/**
 * @brief Assert a reset signal.
 * The reset singal will stay asserted until reset_deassert() is called.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_assert(struct reset_sig *rst_sig);

/**
 * @brief Deassert a reset signal.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_deassert(struct reset_sig *rst_sig);

/**
 * @brief Assert a reset signal, and deassert automatically.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_reset(struct reset_sig *rst_sig);

/**
 * @brief Check reset signal status.
 *
 * @param[in] rst_sig Reset signal.
 * @return 0 if deasserted, positive if asserted,
 *         or a negative error code.
 */
int32_t reset_ctl_status(struct reset_sig *rst_sig);

/**
 * @brief Read reset general reg.
 *
 * @param[in] general general reg.
 * @return general reg value.
 */
uint32_t reset_ctl_general_read(struct reset_general_reg *general);

/**
 * @brief Write reset general reg.
 *
 * @param[in] general general reg.
 * @param[in] val write value.
 * @return 0 if success,
 *         or a negative error code.
 */
int32_t reset_ctl_general_write(struct reset_general_reg *general, uint32_t val);

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
int32_t reset_ctl_general_write_bit(struct reset_general_reg *general, uint8_t start, uint8_t width, uint32_t val);

/**
 * @brief Global reset.
 *
 * @param[in] rst_glb_ctl Global reset controller.
 */
void reset_ctl_global_reset(struct reset_glb_ctl *rst_glb_ctl);

/**
 * @brief Config reset signal assert/deassert in lowpower mode.
 *
 * @param[in] rst_sig Reset signal.
 * @param[in] mode lowpower mode
 * @param[in] val 0:assert, 1:deassert
 *
 * @return 0 if OK, or a negative error code.
 */
int32_t reset_ctl_lowpower_set(struct reset_sig *rst_sig, enum reset_lowpower_mode mode, uint32_t val);

#endif /* RESET_H_ */
