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
#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <newlib.h>
#include <reent.h>

#include "FreeRTOS.h"
#include "task.h"

#if !defined(configUSE_NEWLIB_REENTRANT) ||  (configUSE_NEWLIB_REENTRANT!=1)
  #warning "#define configUSE_NEWLIB_REENTRANT 1 // Required for thread-safety of newlib"
#endif

#if configUSE_MALLOC_FAILED_HOOK
extern void vApplicationMallocFailedHook(void);
#endif

/* current heap end */
static char *gcCurrentHeapAddr = (char *)&_heap_start;

/*-----------------------------------------------------------*/

void *_sbrk_r(struct _reent *p, int incr)
{
    char *preHeapEnd;

    vTaskSuspendAll();

    preHeapEnd = gcCurrentHeapAddr;

    if ((gcCurrentHeapAddr + incr) > (char *)&_heap_end) {
#if configUSE_MALLOC_FAILED_HOOK
        vApplicationMallocFailedHook();
#endif
        xTaskResumeAll();
        return (char *)-1;
    }

    gcCurrentHeapAddr += incr;

    xTaskResumeAll();

    return preHeapEnd;
}

/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xSize ) PRIVILEGED_FUNCTION
{
    if (xSize > 0)
        return malloc(xSize);
    return NULL;
}

/*-----------------------------------------------------------*/

void *pvPortMallocAligned( size_t xWantedSize ,size_t xBoundary ) PRIVILEGED_FUNCTION
{
    if (xWantedSize > 0)
        return memalign(xBoundary, xWantedSize);
    return NULL;
}

/*-----------------------------------------------------------*/

void vPortFree( void *pv ) PRIVILEGED_FUNCTION
{
    if (pv)
        free(pv);
};

/*-----------------------------------------------------------*/

size_t xPortGetFreeHeapSize( void ) PRIVILEGED_FUNCTION
{
    struct mallinfo info = mallinfo();
    uint32_t heapremain = (char *)&_heap_end - gcCurrentHeapAddr;
    return info.fordblks + heapremain;
}

/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void ) PRIVILEGED_FUNCTION
{
}

/*-----------------------------------------------------------*/

void __malloc_lock(struct _reent *p)
{
    vTaskSuspendAll();
}

/*-----------------------------------------------------------*/

void __malloc_unlock(struct _reent *p)
{
    (void)xTaskResumeAll();
}
