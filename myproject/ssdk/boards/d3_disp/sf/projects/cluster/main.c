/**
 * @file main.c
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#if defined(CONFIG_TAISHAN_XSPI) && defined(CONFIG_DISK_NOR_FLASH)
#include "norflash_cfg.h"
#endif

#if defined(CONFIG_MMC) && defined(CONFIG_DISK_MMC)
#include "mmc_cfg.h"
#endif

#if defined(CONFIG_FEE_BLOCK) || defined(CONFIG_FEE_EEPROM)
#include "fee_cfg.h"
#endif

#ifdef CONFIG_CLI
#include <CLI.h>
#endif

#ifdef CONFIG_APP_EMIRROR
extern int emirror_run(void *data);
#endif

#if defined(CONFIG_REBOOT) && defined(CONFIG_TAISHAN_REBOOT)
#include "reboot_cfg.h"
#endif

#ifdef CONFIG_APP_LAUNCHER
#include "launcher.h"
#endif
int main(void)
{
    printf("Hello cluster SSDK!\n");

#if defined(CONFIG_TAISHAN_XSPI) && defined(CONFIG_DISK_NOR_FLASH)
    board_norflash_init();
#endif

#if defined(CONFIG_MMC) && defined(CONFIG_DISK_MMC)
    board_mmc_init();
#endif

#if defined(CONFIG_FEE_BLOCK) || defined(CONFIG_FEE_EEPROM)
    board_fee_init();
#endif

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

#ifdef CONFIG_APP_EMIRROR
    emirror_run(NULL);
#endif

#if defined(CONFIG_REBOOT) && defined(CONFIG_TAISHAN_REBOOT)
    board_reboot_init();
#endif

#ifdef CONFIG_APP_LAUNCHER
    launcher_entry();
#endif
}
