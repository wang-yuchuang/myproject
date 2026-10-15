/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_REBOOT_CFG_H
#define BOARDS_REBOOT_CFG_H

#ifdef CONFIG_REBOOT

#define SDRV_GENERAL_REG(n) (APB_RSTGEN_SEC_BASE + (uint32_t)((0x50 + (n)*4) << 10))

extern void board_reboot_init(void);
extern int32_t boot_mp();
extern int32_t boot_ap1();
extern int32_t boot_ap2();
#endif

#endif /* BOARDS_REBOOT_CFG_H */
