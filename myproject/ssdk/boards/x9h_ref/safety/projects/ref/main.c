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

#ifdef CONFIG_CDNS_OSPI
#include "ospi_cfg.h"
#endif

#ifdef CONFIG_ERRCODE
#include <sd_errcode.h>
#endif /* end CONFIG_ERRCODE */

int main(void)
{
    printf("Hello SSDK!\n");

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

#if defined CONFIG_CDNS_OSPI && defined CONFIG_DISK_NOR_FLASH
    board_norflash_init();
#endif

#ifdef CONFIG_ERRCODE
    system_errcode_init();
#endif
}
