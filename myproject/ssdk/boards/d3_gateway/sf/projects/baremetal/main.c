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

#ifdef CONFIG_RESETGEN
#include "rstgen_cfg.h"
#endif

int main(void)
{
    printf("Hello SSDK!\n");

#ifdef CONFIG_RESETGEN
    reset_ctl_general_write(&reset_general_reg_sf_boot, CORE_SF_BOOT_FINISH);
#endif

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif
}
