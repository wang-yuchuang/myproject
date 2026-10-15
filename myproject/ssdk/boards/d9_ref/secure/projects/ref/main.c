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

#ifdef CONFIG_RPMSG_SERVICE
#include <rpmsg_service.h>
#include <rpmsg_service_cfg.h>
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_SDRV_VIRT_CONSOLE
#include <sdshell.h>
#endif

#ifdef CONFIG_SAVE_DDR_INFO_IN_EMMC
#include "str.h"
#endif

int main(void)
{
    printf("Hello <%s> secure SSDK!\n", CONFIG_BOARD_NAME_STR);

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_SAVE_DDR_INFO_IN_EMMC
    str_slave_commu_init();
#endif

#ifdef CONFIG_SDRV_VIRT_CONSOLE
    sdshell_console_init();
#endif

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif
}
