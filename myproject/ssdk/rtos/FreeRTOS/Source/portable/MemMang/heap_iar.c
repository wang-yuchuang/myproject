/*
 * heap_iar.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: use iar heap.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <iar_dlmalloc.h>

#include "FreeRTOS.h"
#include "task.h"

#if configUSE_MALLOC_FAILED_HOOK
extern void vApplicationMallocFailedHook(void);
#endif

/*-----------------------------------------------------------*/

void *pvPortMalloc( size_t xSize ) PRIVILEGED_FUNCTION
{
    if (xSize > 0)
        return malloc(xSize);
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
    struct mallinfo info = __iar_dlmallinfo();
    return info.fordblks;
}

/*-----------------------------------------------------------*/

void vPortInitialiseBlocks( void ) PRIVILEGED_FUNCTION
{
}
