/*
 * arm.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARMV7R common register interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef ARMV7R_ARM_H
#define ARMV7R_ARM_H

#define MODE_USR            0x10
#define MODE_FIQ            0x11
#define MODE_IRQ            0x12
#define MODE_SVC            0x13
#define MODE_ABT            0x17
#define MODE_UND            0x1B
#define MODE_SYS            0x1F

#define MODE_MASK           0x1F

/* SCTLR */

#define SCTLR_M             (1 << 0)
#define SCTLR_A             (1 << 1)
#define SCTLR_C             (1 << 2)
#define SCTLR_CCP15BEN      (1 << 5)
#define SCTLR_B             (1 << 7)
#define SCTLR_SW            (1 << 10)
#define SCTLR_Z             (1 << 11)
#define SCTLR_I             (1 << 12)
#define SCTLR_V             (1 << 13)
#define SCTLR_RR            (1 << 14)
#define SCTLR_BR            (1 << 17)
#define SCTLR_DZ            (1 << 19)
#define SCTLR_FI            (1 << 21)
#define SCTLR_U             (1 << 22)
#define SCTLR_VE            (1 << 24)
#define SCTLR_EE            (1 << 25)
#define SCTLR_NMFI          (1 << 27)
#define SCTLR_TE            (1 << 30)
#define SCTLR_IE            (1 << 31)

/* PSR */

#define PSR_MODE_SHIFT      (0)
#define PSR_MODE_MASK       (0x1f << PSR_MODE_SHIFT)
#define PSR_MODE_USR        (MODE_USR << PSR_MODE_SHIFT)
#define PSR_MODE_FIQ        (MODE_FIQ << PSR_MODE_SHIFT)
#define PSR_MODE_IRQ        (MODE_IRQ << PSR_MODE_SHIFT)
#define PSR_MODE_SVC        (MODE_SVC << PSR_MODE_SHIFT)
#define PSR_MODE_ABT        (MODE_ABT << PSR_MODE_SHIFT)
#define PSR_MODE_UND        (MODE_UND << PSR_MODE_SHIFT)
#define PSR_MODE_SYS        (MODE_SYS << PSR_MODE_SHIFT)
#define PSR_T_BIT           (1 << 5)
#define PSR_F_BIT           (1 << 6)
#define PSR_I_BIT           (1 << 7)
#define PSR_A_BIT           (1 << 8)
#define PSR_E_BIT           (1 << 9)
#define PSR_IT27_SHIFT      (10)
#define PSR_IT27_MASK       (0x3f << PSR_IT27_SHIFT)
#define PSR_GE_SHIFT        (16)
#define PSR_GE_MASK         (15 << PSR_GE_SHIFT)
#define PSR_J_BIT           (1 << 24)
#define PSR_IT01_SHIFT      (25)
#define PSR_IT01_MASK       (3 << PSR_IT01_SHIFT)
#define PSR_Q_BIT           (1 << 27)
#define PSR_V_BIT           (1 << 28)
#define PSR_C_BIT           (1 << 29)
#define PSR_Z_BIT           (1 << 30)
#define PSR_N_BIT           (1 << 31)

#endif
