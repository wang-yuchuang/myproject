/*
 * scr_lld.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SCR driver header.
 *
 * Revision History:
 * -----------------
 */
#ifndef _SCR_LLD_H
#define _SCR_LLD_H

#include "driver/scr.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t scr_lld_get(const scr_dev_t *dev, const scr_signal_t *signal);
bool scr_lld_set(const scr_dev_t *dev, const scr_signal_t *signal,
                 uint32_t value);
bool scr_lld_lock(const scr_dev_t *dev, const scr_signal_t *signal);
bool scr_lld_is_locked(const scr_dev_t *dev, const scr_signal_t *signal);

#ifdef CONFIG_SCR_FUSA
typedef void (*scr_callback_t)(uint32_t value, void *user_arg);
void scr_fusa_enable_it(const scr_dev_t *dev, scr_callback_t cb,
                        void *user_arg);
/*return: 0x1 int status as expected;0x2 int status not as expected
    0x4：int clr as expected; 0x8: int clr not as expected
*/
uint32_t scr_fusa_chk_lockstep(const scr_dev_t *dev,
                               const scr_signal_t *signal);
uint32_t scr_fusa_ej_get(const scr_dev_t *dev, const scr_signal_t *signal);
void scr_fusa_ej_set(const scr_dev_t *dev, const scr_signal_t *signal,
                     uint32_t value);
void scr_fusa_ej_enable(const scr_dev_t *dev, uint32_t value);
void scr_permission_set(const scr_dev_t *dev, uint32_t did, uint32_t secp,
                        uint32_t rwp);
void scr_rs_set(const scr_dev_t *dev, const scr_signal_t *signal, uint32_t did,
                uint32_t en);
uint32_t scr_fs_clear_lserr_status(const scr_dev_t *dev);
uint32_t scr_fs_clear_err_status(const scr_dev_t *dev);

void scr_fs_ej(const scr_dev_t *dev, uint32_t offset, uint32_t data);
void scr_fusa_ro_mask(const scr_dev_t *dev);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _SCR_LLD_H */
