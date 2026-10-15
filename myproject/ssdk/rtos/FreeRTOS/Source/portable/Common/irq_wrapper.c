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

void OS_CRITICAL_CODE vApplicationIRQHandler(void)
{
#ifdef USE_STACK_COLORATION
    if (!arch_check_exception_stack()) {
        ssdk_printf(SSDK_ALERT, "exception stack overflow!\n");
        return;
    }
#endif

    irq_dispatch();
}
