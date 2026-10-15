/*
 * irq_wrapper.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: irq wrapper interface.
 *
 * Revision History:
 * -----------------
 */

#include <types.h>
#include <debug.h>

#include "arch.h"
#include "driver/irq.h"

void __WEAK BareMetalBadIRQHandler(void)
{
    ssdk_printf(SSDK_ALERT, "IRQ timeout!\n");

    while (1);
}

void BareMetalIRQHandler(void)
{
#ifdef USE_STACK_COLORATION
    if (!arch_check_exception_stack()) {
        ssdk_printf(SSDK_ALERT, "exception stack overflow!\n");
        return;
    }
#endif

    if (-1 == irq_dispatch()) {
        /* fiq timeout */
        BareMetalBadIRQHandler();
    }
}

void __WEAK BareMetalBadFIQHandler(void)
{
    ssdk_printf(SSDK_ALERT, "FRQ timeout!\n");

    while (1);
}

void BareMetalFIQHandler(void)
{
    if (-1 == irq_dispatch()) {
        /* irq timeout */
        BareMetalBadFIQHandler();
    }
}
