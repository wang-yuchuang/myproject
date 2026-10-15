/*
 * logflag.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */
#ifndef _LOG_FLAG_H_
#define _LOG_FLAG_H_
#include <arch.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>

#define EXP_LOG_CYCLE_RESTART       (3)
/*
 * BIT : 16         15       14       13      12     9          8     6     4     2     0
 * FUNC: | dumpflag | AP2_EXP | AP_EXP | G_EXP | REV | SAF_UART | AP1 | MPC | SEC | SAF |
 */
#define EXP_LOG_STATE_REG       (APB_RSTGEN_SEC_BASE + 0x1b000)  /* 0xf841b000 */
#define EXP_LOG_RST_CNT_MASK        (0x3)
#define EXP_LOG_RST_CNT_BITS        (0x2)

#define EXP_SAF_UART_FLAG_SHIFT     (8)
#define EXP_SAF_UART_FLAG_MASK      (0x1)

#define EXP_LOG_GLOBAL_FLAG_SHIFT   (12)
#define EXP_LOG_GLOBAL_FLAG_MASK    (0x1)

/* transfer the ap exception from safety to preloader */
#define EXP_LOG_AP_EXP_FLAG_SHIFT   (13)
#define EXP_LOG_AP_EXP_FLAG_MASK    (0x1)

#define EXP_AP2_EXP_FLAG_SHIFT      (14)
#define EXP_AP2_EXP_FLAG_MASK       (0x1)

#define EXP_CRASHDUMP_FLAG_SHIFT    (15)
#define EXP_CRASHDUMP_FLAG_MASK     (0x1)

typedef enum {
    FLAG_ID_SAF = 0,
    FLAG_ID_SEC,
    FLAG_ID_MPC,
    FLAG_ID_AP1,
    FLAG_ID_MAX
} flag_id_t;

int sdrv_exp_rst_cnt_get(flag_id_t id);
void sdrv_exp_rst_cnt_set(flag_id_t id, int val);
void sdrv_exp_rst_cnt_add(flag_id_t id, int val);
int sdrv_is_loop_exp_restart(flag_id_t id);
void sdrv_global_exp_flag_set(int val);
void sdrv_ap_exp_flag_set(int val);
int sdrv_ap_exp_flag_get(void);
void sdrv_ap2_exp_flag_set(int val);
int sdrv_ap2_exp_flag_get(void);
void sdrv_crashdump_flag_set(int val);
int sdrv_crashdump_flag_get(void);
void sdrv_saf_uart_flag_set(int val);
int sdrv_saf_uart_flag_get(void);
#endif
