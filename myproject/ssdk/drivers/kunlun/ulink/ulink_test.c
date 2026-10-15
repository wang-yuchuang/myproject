/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <app.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <err.h>
#include <dcf.h>
#include <kernel/thread.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>

#include "ulink.h"

#define REG_BASE (0x6a000000)
#define IRQ_TEST (0x10)

static ulink_channel_t *ch_test;
static ulink_channel_t *ch_call;
static ulink_channel_t *ch_saf;

static void test_recive(ulink_channel_t *ch, uint8_t *data, uint32_t size)
{
    uint32_t i;
    for (i = 0; i < size; i++)
        printf("0x%x ", data[i]);
    printf("\n");
}

static void test_call(ulink_channel_t *ch, uint8_t *data, uint32_t size)
{
    if (size != 4)
        printf("size error\n");

    if (memcmp(data, "test", 4))
        printf("code error\n");

    ulink_channel_send(ch, (uint8_t *)"suc", 3);
}


static void ulink_test_main(const struct app_descriptor *app, void *args)
{
    ulink_irq_set_route(IRQ_TEST, IRQ_TEST, DP_CA_AP1, 0x42,NULL);
    ch_test = ulink_request_channel(DP_CA_AP1, 0xdef, test_recive);
    ch_call = ulink_request_channel(DP_CA_AP1, 0xca, test_call);
    ch_saf = ulink_request_channel(DP_CR5_SAF, 0xdef, test_recive);
}

int do_ulink_test(int argc, const cmd_args *argv)
{
    if (!strcmp(argv[1].str, "help")) {
        printf("rrrg wreg irq sendmsg sendsaf\n");
    } else if (!strcmp(argv[1].str, "wreg")) {
        if (argc > 2)
            writel(argv[2].u, REG_BASE);
    } else if (!strcmp(argv[1].str, "rreg")) {
        printf("%x\n", readl(REG_BASE));
    } else if (!strcmp(argv[1].str, "irq")) {
        ulink_trigger_irq(IRQ_TEST);
    } else if (!strcmp(argv[1].str, "sendmsg")) {
        if (argc > 2)
            ulink_channel_send(ch_test, (uint8_t*)argv[2].str, strlen(argv[2].str));
    } else if (!strcmp(argv[1].str, "sendsaf")) {
        if (argc > 2)
            ulink_channel_send(ch_saf, (uint8_t*)argv[2].str, strlen(argv[2].str));
    }else {
        printf("error cmd");
    }
    return 0;
}

#if defined(WITH_LIB_CONSOLE)
STATIC_COMMAND_START
STATIC_COMMAND("ulink", "ulink test", (console_cmd)&do_ulink_test)
STATIC_COMMAND_END(ulink_test);
#endif

APP_START(ulink_test)
.entry = (app_entry)ulink_test_main,
APP_END

