/**
 * @file main.c
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#if defined(CONFIG_MMC) && defined(CONFIG_DISK_MMC)
#include "mmc_cfg.h"
#endif

#ifdef CONFIG_CLI
#include <CLI.h>
#endif

#ifdef CONFIG_RPMSG_SERVICE
#include <rpmsg_service.h>
#include "rpmsg_service_cfg.h"
#endif

#if defined(CONFIG_DLOADER)
#include "dloader.h"
#endif

#if defined(CONFIG_REBOOT) && defined(CONFIG_TAISHAN_REBOOT)
#include "reboot_cfg.h"
#endif

#ifdef CONFIG_RESETGEN
#include "rstgen_cfg.h"
#endif

int main(void)
{
    printf("Hello SSDK!\n");

#if defined(CONFIG_MMC) && defined(CONFIG_DISK_MMC)
    board_mmc_init();
#endif

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

#if defined(CONFIG_DLOADER)
    dloader_init();
#endif

#ifdef CONFIG_RESETGEN
    reset_ctl_general_write(&reset_general_reg_sf_boot, CORE_SF_BOOT_FINISH);
#endif

#if defined(CONFIG_REBOOT) && defined(CONFIG_TAISHAN_REBOOT)
    board_reboot_init();
#endif
}
