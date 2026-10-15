/**
 * @file board.h
 * @brief board config header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef BOARDS_BOARD_H
#define BOARDS_BOARD_H

#include <compiler.h>
#include <regs_base.h>
#include <irq_num.h>
#include <board/board.h>

#define DEVICE_BASE(dev) _DEVICE_BASE(dev)
#define _DEVICE_BASE(dev) APB_##dev##_BASE

#define DEVICE_INTR(dev) _DEVICE_INTR(dev)
#define _DEVICE_INTR(dev) dev##_INTR_NUM

#ifdef CONFIG_REBOOT
typedef enum boot_cpu_id{
    BOOT_ID_MIN = 0x0,
    BOOT_ID_CPU1,
    BOOT_ID_CPU2,
    BOOT_ID_MP,
    BOOT_ID_SEC,
    BOOT_ID_SAF,
    BOOT_ID_MAX
}boot_cpu_id;
#endif

#endif /* BOARDS_BOARD_H */