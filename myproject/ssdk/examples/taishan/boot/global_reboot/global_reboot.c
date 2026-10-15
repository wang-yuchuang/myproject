/**
 * @file global_reboot.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <types.h>
#include <param.h>
#include <string.h>
#include "rstgen_cfg.h"
#include <cmsis_os2.h>
#ifdef CONFIG_REBOOT
#include <reboot.h>
#endif
#include <errno.h>

#ifdef CONFIG_CLI
#include <CLI.h>

#define ROM_CTRL_BOOT_PIN_OVERRIDE_BIT_OFFSET 0x1
#define ROM_CTRL_BOOT_PIN_OVERRIDE_ENABLE_BIT_OFFSET 0x9

static void useage(void)
{
    ssdk_printf(SSDK_CRIT,"Usage: \n\treboot [decimal number]: reboot to the corresponding number boot pin mode");
    ssdk_printf(SSDK_CRIT,"\n\texample, reboot 8: reboot to the USB download mode");
    ssdk_printf(SSDK_CRIT,"\n\texample, reboot 0: reboot to the Flash mode");
    ssdk_printf(SSDK_CRIT,"\n\texample, reboot 2: reboot to the eMMC mode");
    ssdk_printf(SSDK_CRIT,"\n\texample, reboot 4: reboot to the SD2 mode");
    ssdk_printf(SSDK_CRIT,"Usage: \n\treboot clear: clear the reboot pin");
}

static void mark_boot_pin_flag(uint8_t boot_pin)
{
    uint32_t val = 0;
    val = reset_ctl_general_read(&reset_general_reg_rom_ctrl);
    val |= ((boot_pin << ROM_CTRL_BOOT_PIN_OVERRIDE_BIT_OFFSET) & 0x1E);
    val |= (0x1 << ROM_CTRL_BOOT_PIN_OVERRIDE_ENABLE_BIT_OFFSET);
    reset_ctl_general_write(&reset_general_reg_rom_ctrl, val);
}

static void clear_boot_pin_flag()
{
    uint32_t val = 0;
    val = reset_ctl_general_read(&reset_general_reg_rom_ctrl);
    val &= ~0x1E;
    val &= ~(0x1 << ROM_CTRL_BOOT_PIN_OVERRIDE_ENABLE_BIT_OFFSET);
    reset_ctl_general_write(&reset_general_reg_rom_ctrl, val);
}

static int glb_reset(int argc, char *argv[])
{
    int boot_pin = 0;
    switch (argc)
    {
        case 0:
            ssdk_printf(SSDK_CRIT,"reboot without flag");
            clear_boot_pin_flag();
            break;

        case 1:
            boot_pin = strtoul(argv[0], NULL, 10);
            if (errno == 0 && ((boot_pin > 0) || !strcmp(argv[0], "0")) && (boot_pin <= 15)) {
                mark_boot_pin_flag(boot_pin);
                ssdk_printf(SSDK_CRIT,"reboot with boot bin flag= %d", boot_pin);
            }
            else {
                useage();
                return CLI_FALSE;
            }
            break;

        default:
            useage();
            return CLI_FALSE;
    }
    /* jsut for finishing the uart print*/
    osDelay(osKernelGetTickFreq()/100);

#ifdef CONFIG_TAISHAN_REBOOT_GLOBAL
    reboot_global_ctrl();
#endif
    return CLI_FALSE;
}
CLI_CMD("reboot", "\r\nreboot:\r\nglobal reset\r\n", glb_reset);
#endif
