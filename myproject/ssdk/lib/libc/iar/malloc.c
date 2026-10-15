/**
 * @file malloc.c
 * @brief IAR malloc code using RTOS heap.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <iar_dlmalloc.h>
#include <cmsis_os2.h>

void *malloc( size_t s )
{
    if (s > 0)
        return osAlloc(s);
    return NULL;
}

void *memalign( size_t a, size_t s )
{
    if (s > 0)
        return osAllocAlign(a, s);
    return NULL;
}

void *calloc( size_t n, size_t s )
{
    void *p = malloc(s *= n);
    if (p)
        memset (p, 0, s);
    return p;
}

void free( void *pv )
{
    if (pv)
        osFree(pv);
}

void *realloc( void *p, size_t s )
{
    void *np;

    if (!p)
        return osAlloc(s);

    if (!s) {
        osFree(p);
        return 0;
    }

    np = osAlloc(s);
    if (np) {
        memcpy(np, p, s);
        osFree(p);
    }

    return np;
}
