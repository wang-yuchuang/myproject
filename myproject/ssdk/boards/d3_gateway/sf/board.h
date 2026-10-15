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

#define CORE_SP0_RUNNING  0x1U
#define CORE_SP1_RUNNING  0x2U
#define CORE_SX0_RUNNING  0x4U
#define CORE_SX1_RUNNING  0x8U

extern unsigned int g_core_running;

#ifdef CONFIG_CU
extern bool g_console_occupid;
#endif

#endif /* BOARDS_BOARD_H */