/*
 * scr.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: System Control Register driver.
 *
 * Revision History:
 * -----------------
 */
#ifndef _SCR_H
#define _SCR_H

#include <compiler.h>
#include <spinlock.h>
#include <types.h>

#define TYPE_RW 0
#define TYPE_RO 1
#define TYPE_L16 2
#define TYPE_L31 3
#define TYPE_R16W16 4

#define TYPE_MAX 5

#define DOMAIN_ID_MSK 3
#define DOMAIN_RS_SEC 0
#define DOMAIN_RS_NSEC 2
#define DOMAIN_RS_PRI 4
#define DOMAIN_RS_USER 6
#define ACCESS_RW 0
#define ACESS_RO 1
#define ACCESS_WO 2
#define ACCESS_NULL 3

typedef struct scr_signal {
    uint16_t type; /* signal type */
    uint16_t start_bit;
    uint16_t width;
} scr_signal_t;

typedef struct scr_dev {
    paddr_t base;
    spin_lock_t lock;
    uint32_t reg_off[TYPE_MAX]; /* register offset for different types. */
    int reg_shift;              /* register address shift */
} scr_dev_t;

/*
 * Refer to board specific "scr_hw.h" for predefined scr_signals.
 */
uint32_t scr_get(scr_dev_t *dev, const scr_signal_t *signal);
bool scr_set(scr_dev_t *dev, const scr_signal_t *signal, uint32_t value);
bool scr_lock(scr_dev_t *dev, const scr_signal_t *signal);
bool scr_is_locked(scr_dev_t *dev, const scr_signal_t *signal);
#endif /* _SCR_H */
