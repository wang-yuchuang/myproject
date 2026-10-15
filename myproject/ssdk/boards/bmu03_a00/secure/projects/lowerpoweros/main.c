/**
 * @file main.c
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <debug.h>

#ifdef CONFIG_CLI
#include <CLI.h>
#endif

#ifdef CONFIG_RPMSG_SERVICE
#include <rpmsg_service.h>
#include <rpmsg_service_cfg.h>
#endif

#ifdef CONFIG_DISK
#include "lib/disk/disk.h"
#endif

#ifdef CONFIG_OSPI
#include "ospi_cfg.h"
#endif

#ifdef CONFIG_SAVE_DDR_INFO_IN_EMMC
#include "mmc_cfg.h"
#endif

#include <str.h>

int main(void)
{

#ifdef CONFIG_DISK
    disk_init();
#endif

#ifdef CONFIG_OSPI
    board_norflash_init();
#endif

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif
#ifdef CONFIG_SAVE_DDR_INFO_IN_EMMC
    board_mmc_init();
#endif

    str_enter();
}
