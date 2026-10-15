/*
 * heap_newlib.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: use newlib heap.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <malloc.h>
#include <debug.h>
#include <arch/irq.h>

extern const unsigned long _heap_start;
extern const unsigned long _heap_end;

/* current heap end */
static char *gcCurrentHeapAddr = (char *)&_heap_start;

/*-----------------------------------------------------------*/

void *_sbrk_r(struct _reent *p, int incr)
{
    char *preHeapEnd;

    preHeapEnd = gcCurrentHeapAddr;

    if ((gcCurrentHeapAddr + incr) > (char *)&_heap_end) {
        return (char *)-1;
    }

    gcCurrentHeapAddr += incr;

    return preHeapEnd;
}

/*-----------------------------------------------------------*/

void __malloc_lock(struct _reent *p)
{
    ASSERT(!arch_in_irq_mode());
}

/*-----------------------------------------------------------*/

void __malloc_unlock(struct _reent *p)
{
    ASSERT(!arch_in_irq_mode());
}
