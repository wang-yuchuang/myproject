/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef SOC_SEIP_REG_H
#define SOC_SEIP_REG_H

#define MBOX_SOCBASE            (0x021d8000)
#define rSOCMBOX_CMD_D0         *((volatile unsigned int *)(MBOX_SOCBASE + 0x00))
#define rSOCMBOX_CMD_D1         *((volatile unsigned int *)(MBOX_SOCBASE + 0x04))
#define rSOCMBOX_CMD_D2         *((volatile unsigned int *)(MBOX_SOCBASE + 0x08))
#define rSOCMBOX_CMD_D3         *((volatile unsigned int *)(MBOX_SOCBASE + 0x0C))
#define rSOCMBOX_CMD_D4         *((volatile unsigned int *)(MBOX_SOCBASE + 0x10))
#define rSOCMBOX_CMD_D5         *((volatile unsigned int *)(MBOX_SOCBASE + 0x14))
#define rSOCMBOX_CMD_D6         *((volatile unsigned int *)(MBOX_SOCBASE + 0x18))
#define rSOCMBOX_CMD_D7         *((volatile unsigned int *)(MBOX_SOCBASE + 0x1C))
#define rSOCMBOX_RSP_D0         *((volatile unsigned int *)(MBOX_SOCBASE + 0x20))
#define rSOCMBOX_INT_STA        *((volatile unsigned int *)(MBOX_SOCBASE + 0x24))
#define rSOCMBOX_INT_EN         *((volatile unsigned int *)(MBOX_SOCBASE + 0x28))

#endif
