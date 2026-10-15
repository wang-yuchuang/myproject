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

#ifdef CONFIG_LETTER_SHELL
#include <shell_port.h>
#include <shell.h>
#endif

#if defined(__GNUC__) && defined(LIBCPU_VERSION)
extern const char libcpu_msg[];
#endif

int main(void)
{
    printf("Hello <%s> AP SSDK!\n", CONFIG_BOARD_NAME_STR);

#if defined(__GNUC__) && defined(LIBCPU_VERSION)
    printf("%s\n", libcpu_msg);
#endif

#ifdef CONFIG_CLI
    CLIConsoleStart();
#endif

#ifdef CONFIG_LETTER_SHELL
	uartShellInit();
	shellTask(&shell);
#endif
}
