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
#endif /* end CONFIG_RPMSG_SERVICE */

int main(void)
{
    printf("Hello <%s> Secure SSDK!\n", CONFIG_BOARD_NAME_STR);

#ifdef CONFIG_RPMSG_SERVICE
    rpmsg_service_init(RPMSG_SERVICES_NUM, g_rpmsg_services);
#endif /* end CONFIG_RPMSG_SERVICE */

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif
}
