/**
 * @file reboot.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef REBOOT_H
#define REBOOT_H

#ifndef CONFIG_TAISHAN_REBOOT
#include <types.h>
#include <cmsis_os2.h>

#include "driver/scr.h"
#include "driver/mbox.h"
#include "driver/reset.h"


typedef enum reboot_cpu_id{
    CPU_ID_MIN = 0x0,
    CPU_ID_CPU1,
    CPU_ID_CPU2,
    CPU_ID_MP,
    CPU_ID_SEC,
    CPU_ID_SAF,
    CPU_ID_MAX
}reboot_cpu_id;

struct scr_signal_item {
    scr_dev_t* scr_dev;
    scr_signal_t *scr_sig;
    uint32_t *scr_value;
    uint32_t  num;
};

/**
 * @brief reboot configurations.
 */
struct reboot_cfg {
    uint32_t id;
    addr_t seeker_base;
    uint32_t seeker_size;
    addr_t target_addr;
    char *pt_name;
    struct scr_signal_item scr_entry_signal;
    reset_sig_t *rst_cpu;
    mbox_dev_t *mbox;
    bool imgcpy_skip;
};

#else

#ifndef MAX_GPT_NAME_SIZE
#define MAX_GPT_NAME_SIZE 72
#endif

typedef enum {
    SF = 0,
    SP0,
    SP1,
    SX0,
    SX1,
    SP,
    SX,
    MAX_CORE_ID,
} boot_core_id_t;

struct scr_signal_item {
    scr_dev_t* scr_dev;
    scr_signal_t *scr_sig;
    uint32_t *scr_value;
    uint32_t  num;
};

/**
 * @brief reboot configurations.
 */
struct reboot_cfg {
    uint32_t id;
    addr_t target_addr;
    uint32_t target_len;
    char disk_name[MAX_GPT_NAME_SIZE + 1];
    char *pt_name;
    struct scr_signal_item scr_entry_signal;
    reset_sig_t *rst_cpu;
};

#endif

/**
 * @brief reboot ctrl
 */
void reboot_ctrl(struct reboot_cfg *reboot_array);

/**
 * @brief global reboot control
 */
void reboot_global_ctrl(void);

#endif /* REBOOT_H */
