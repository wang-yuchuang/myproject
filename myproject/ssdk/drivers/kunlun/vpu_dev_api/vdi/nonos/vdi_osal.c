//------------------------------------------------------------------------------
// File: vdi_osal.c
//
// Copyright (c) 2006, Chips & Media.  All rights reserved.
//------------------------------------------------------------------------------
//#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(WIN32) ||
// defined(__MINGW32__) #elif defined(linux) || defined(__linux) || defined(ANDROID) #else

#include "vpuconfig.h"
#include "vputypes.h"
#include <stdarg.h>
#include <string.h>
#include <arch/cache.h>
#include <cmsis_os2.h>
#include "../vdi_osal.h"

#include <driver/timer.h>
#include <timer_cfg.h>

void VpuLogMsg(int level, const char *format, ...)
{
    if (level != ERR)
        return;

    va_list ptr;
    char logBuf[MAX_PRINT_LENGTH] = {0};

    va_start(ptr, format);
    vsnprintf(logBuf, MAX_PRINT_LENGTH, format, ptr);
    va_end(ptr);

    printf(logBuf);
}

void *osal_memcpy_form_cpu_to_device(void *dst, const void *src, int count)
{
    void *ret = memcpy(dst, src, count);
    arch_clean_cache_range((addr_t)dst, count);
    return ret;
}

void *osal_memcpy_form_device_to_cpu(void *dst, const void *src, int count)
{
    arch_invalidate_cache_range((addr_t)src, count);
    void *ret = memcpy(dst, src, count);
    return ret;
}

void *osal_memcpy(void *dst, const void *src, int count) { return memcpy(dst, src, count); }

void *osal_memset(void *dst, int val, int count) { return memset(dst, val, count); }

int osal_memcmp(const void *src, const void *dst, int size) { return memcmp(src, dst, size); }

void *osal_malloc(int size) { return malloc(size); }

void *osal_realloc(void *ptr, int size) { return realloc(ptr, size); }

void osal_free(void *p) { free(p); }

int osal_fflush(osal_file_t fp) { return 1; }

int osal_feof(osal_file_t fp) { return 0; }

osal_file_t osal_fopen(const char *osal_file_tname, const char *mode) { return NULL; }

size_t osal_fwrite(const void *p, int size, int count, osal_file_t fp) { return 0; }

size_t osal_fread(void *p, int size, int count, osal_file_t fp) { return 0; }

long osal_ftell(osal_file_t fp) { return 0; }

int osal_fseek(osal_file_t fp, long offset, int origin) { return 0; }

int osal_fclose(osal_file_t fp) { return 0; }
int osal_fscanf(osal_file_t fp, const char *_Format, ...) { return 1; }

int osal_fprintf(osal_file_t fp, const char *_Format, ...)
{
    va_list ptr;
    char logBuf[MAX_PRINT_LENGTH] = {0};

    va_start(ptr, _Format);

    vsnprintf(logBuf, MAX_PRINT_LENGTH, _Format, ptr);

    va_end(ptr);

    printf(logBuf);

    return 1;
}

int osal_kbhit(void) { return 0; }
int osal_getch(void) { return -1; }

int osal_flush_ch(void) { return -1; }

uint64_t osal_gettime(void) { return timer_get_global_time(g_syscnt_timer) / 1000; }

//------------------------------------------------------------------------------
// math related api
//------------------------------------------------------------------------------
#ifndef I64
typedef long long I64;
#endif

// 32 bit / 16 bit ==> 32-n bit remainder, n bit quotient
static int fixDivRq(int a, int b, int n)
{
    I64 c;
    I64 a_36bit;
    I64 mask, signBit, signExt;
    int i;

    // DIVS emulation for BPU accumulator size
    // For SunOS build
    mask = 0x0F;
    mask <<= 32;
    mask |= 0x00FFFFFFFF; // mask = 0x0FFFFFFFFF;
    signBit = 0x08;
    signBit <<= 32; // signBit = 0x0800000000;
    signExt = 0xFFFFFFF0;
    signExt <<= 32; // signExt = 0xFFFFFFF000000000;

    a_36bit = (I64)a;

    for (i = 0; i < n; i++) {
        c = a_36bit - (b << 15);
        if (c >= 0)
            a_36bit = (c << 1) + 1;
        else
            a_36bit = a_36bit << 1;

        a_36bit = a_36bit & mask;
        if (a_36bit & signBit)
            a_36bit |= signExt;
    }

    a = (int)a_36bit;
    return a; // R = [31:n], Q = [n-1:0]
}

int math_div(int number, int denom)
{
    int c;
    c = fixDivRq(number, denom, 17); // R = [31:17], Q = [16:0]
    c = c & 0xFFFF;
    c = (c + 1) >> 1; // round
    return (c & 0xFFFF);
}
