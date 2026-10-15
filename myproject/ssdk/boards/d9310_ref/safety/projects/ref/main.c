/**
 * @file main.c
 * @brief main source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef CONFIG_CLI
#include <CLI.h>
#endif

#ifdef CONFIG_APP_REBOOT_SS
#include <app_reboot_ss.h>
#endif /* end CONFIG_APP_REBOOT_SS */

#ifdef CONFIG_REBOOT
#include "reboot_cfg.h"
#endif

#ifdef CONFIG_RPMSG_SERVICE
#include <rpmsg_service.h>
#include <rpmsg_service_cfg.h>
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_OSPI
#include "ospi_cfg.h"
#endif

int main(void)
{
#ifdef CONFIG_APP_REBOOT_SS
    app_reboot_ss_init(WDT6____ovflow_int);
#endif /* end CONFIG_APP_REBOOT_SS */

#ifdef CONFIG_REBOOT
    board_reboot_init();
#endif

#ifdef CONFIG_OSPI
    board_norflash_init();
#endif

    printf("Hello <%s> safety SSDK!\n", CONFIG_BOARD_NAME_STR);

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif
}
