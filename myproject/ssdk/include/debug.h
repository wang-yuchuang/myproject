/*
 * debug.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: debug interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef _INCLUDE_DEBUG_H
#define _INCLUDE_DEBUG_H

#include <types.h>
#include <stdio.h>
#include <compiler.h>

/* SSDK debug level */
#define SSDK_EMERG     0  /* System is unusable */
#define SSDK_ALERT     1  /* Action must be taken immediately */
#define SSDK_CRIT      2  /* Critical conditions */
#define SSDK_ERR       3  /* Error conditions */
#define SSDK_WARNING   4  /* Warning conditions */
#define SSDK_NOTICE    5  /* Normal, but significant, condition */
#define SSDK_INFO      6  /* Informational message */
#define SSDK_DEBUG     7  /* Debug-level message */

/* SSDK printf */
#ifdef CONFIG_DEBUG
#define ssdk_printf(level, x...)    do { if ((level) <= CONFIG_DEBUG_LEVEL) {printf(x); } } while (0)
#define PANIC()     ssdk_panic((const uint8_t *)__FILE__, (int)__LINE__)
#define ASSERT(x)   do { if (!(x)) PANIC(); } while (0)
#else
#define ssdk_printf(level, x...)
#define PANIC()
#define ASSERT(x)
#endif

#if defined(CONFIG_DEBUG) || defined(ATB_SIGNER)
#define DBG_ARRAY_DUMP(array, sz)    \
        do {\
            U8 tmp_xyz[8];\
            memset(tmp_xyz,0, sizeof(tmp_xyz));\
            for (U32 i_xyz = 0; i_xyz < (sz); i_xyz++) {\
                tmp_xyz[i_xyz%8] = (array)[i_xyz];\
                if (((i_xyz % 8 ) == 7) || (i_xyz == ((sz) - 1))) {\
                    ssdk_printf(SSDK_INFO, "0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x,\n",\
                            tmp_xyz[0], tmp_xyz[1], tmp_xyz[2], tmp_xyz[3],\
                            tmp_xyz[4], tmp_xyz[5], tmp_xyz[6], tmp_xyz[7]);\
                }\
            }\
        }while(0)
#else
#define DBG_ARRAY_DUMP(array, sz)
#endif

__BEGIN_CDECLS

/*
 * panic function.
 *
 * @param[in] filename Panic file name.
 * @param[in] linenum  Panic line num.
 */
#ifdef CONFIG_DEBUG
void ssdk_panic(const uint8_t *filename, int linenum);
#else
#define ssdk_panic(filename, linenum)
#endif

/*
 * hex dump function.
 *
 * @param[in] ptr dump data address.
 * @param[in] len dump data length.
 */
#ifdef CONFIG_DEBUG
void hexdump(const void *ptr, size_t len);
#else
#define hexdump(ptr, len)
#endif

/*
 * hex dump8 function with display address.
 *
 * @param[in] ptr dump data address.
 * @param[in] len dump data length.
 * @param[in] disp_addr display address.
 */
#ifdef CONFIG_DEBUG
void hexdump8_ex(const void *ptr, size_t len, uint64_t disp_addr);
static inline void hexdump8(const void *ptr, size_t len)
{
    hexdump8_ex(ptr, len, (addr_t)ptr);
}
#else
#define hexdump8_ex(ptr, len, disp_addr)
#endif

/* spin the cpu for a period of (short) time */
void spin(uint32_t usecs);

__END_CDECLS

#endif
