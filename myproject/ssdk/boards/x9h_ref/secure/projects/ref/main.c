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

int main(void)
{
    printf("Hello SSDK!\n");

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif
}