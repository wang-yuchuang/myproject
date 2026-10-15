/**
 * @file flops_main.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: flops test main
 *
 * Revision History:
 * -----------------
 */

#include "CLI_type.h"

extern void flops_main();
extern int whetstone_main(void);
extern void linpack_main(void);

int flops_test(int argc, char *argv[])
{
    flops_main();
    return CLI_FALSE;
}

int whetstone_test(int argc, char *argv[])
{
    whetstone_main();
    return CLI_FALSE;
}

int linpack_test(int argc, char *argv[])
{
    linpack_main();
    return CLI_FALSE;
}

CLI_CMD("flops", "start flops test", flops_test);
CLI_CMD("whetstone", "start whetstone test", whetstone_test);
CLI_CMD("linpack", "start linpack test", linpack_test);
