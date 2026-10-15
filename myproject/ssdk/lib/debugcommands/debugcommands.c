/*
 * Copyright (c) 2008-2012 Travis Geiselbrecht
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <types.h>
#include <debug.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <cmsis_os2.h>
#include "CLI_type.h"
#include <board.h>
#include <string.h>
#include <arch.h>


#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#define DEFAULT_STACK_SIZE 4096

// define a macro that unconditionally swaps
#define SWAP_32(x) \
    (((uint32_t)(x) << 24) | (((uint32_t)(x) & 0xff00) << 8) |(((uint32_t)(x) & 0x00ff0000) >> 8) | ((uint32_t)(x) >> 24))
#define SWAP_16(x) \
    ((((uint16_t)(x) & 0xff) << 8) | ((uint16_t)(x) >> 8))
#define SWAP_64(x) ((((uint64_t)(SWAP_32((uint64_t)(x)))) << 32) | (SWAP_32(((uint64_t)(x)) >> 32)))

static int cmd_display_mem_dw(int argc, char *argv[])
{
    /* save the last address and len so we can continue where we left off */
    static unsigned long address;
    static size_t len = 0;
    char *end_ptr;

    if (argc < 2 && len == 0) {
        printf("not enough arguments\n");
        printf("dw [-l] [-b] [address] [length]\n");
        return CLI_FALSE;
    }

    int size = 4;

    uint byte_order = BYTE_ORDER;
    int argindex = 0;
    bool read_address = false;
    while (argc > argindex) {
        if (!strcmp(argv[argindex], "-l")) {
            byte_order = LITTLE_ENDIAN;
        }
        else if (!strcmp(argv[argindex], "-b")) {
            byte_order = BIG_ENDIAN;
        }
        else if (!read_address) {
            address = strtoul(argv[argindex], &end_ptr, 16);
            if (*end_ptr || errno) {
                printf("dw: invalid address 0x%lx %d", address, errno);
                return CLI_FALSE;
            }
            read_address = true;
        }
        else {
            len = strtoul(argv[argindex], &end_ptr, 16);
            if (*end_ptr || errno) {
                printf("dw: invalid length 0x%x %d", (unsigned int)len, errno);
                return CLI_FALSE;
            }
        }

        argindex++;
    }

    unsigned long stop = address + len;
    int count = 0;

    if ((address & (size - 1)) != 0) {
        printf("dw: unaligned address, cannot display\n");
        return CLI_FALSE;
    }

#if WITH_KERNEL_VM
    /* preflight the start address to see if it's mapped */
    if (vaddr_to_paddr((void *)address) == 0) {
        printf("ERROR: address 0x%lx is unmapped\n", address);
        return CLI_FALSE;
    }
#endif

    for ( ; address < stop; address += size) {
        if (count == 0)
            printf("0x%08lx: ", address);
        switch (size) {
            case 4: {
                uint32_t val = (byte_order != BYTE_ORDER) ?
                               SWAP_32(*(uint32_t *)address) :
                               *(uint32_t *)address;
                printf("%08x ", val);
                break;
            }
            case 2: {
                uint16_t val = (byte_order != BYTE_ORDER) ?
                               SWAP_16(*(uint16_t *)address) :
                               *(uint16_t *)address;
                printf("%04hx ", val);
                break;
            }
            case 1:
                printf("%02hhx ", *(uint8_t *)address);
                break;
        }
        count += size;
        if (count == 16) {
            printf("\n");
            count = 0;
        }
    }
    if (count != 0)
        printf("\n");

    return CLI_FALSE;
}

static int cmd_display_mem_dh(int argc, char *argv[])
{
    /* save the last address and len so we can continue where we left off */
    static unsigned long address;
    static size_t len = 0;
    char *end_ptr;

    if (argc < 2 && len == 0) {
        printf("not enough arguments\n");
        printf("dh [-l] [-b] [address] [length]\n");
        return CLI_FALSE;
    }

    int size = 2;

    uint byte_order = BYTE_ORDER;
    int argindex = 0;
    bool read_address = false;
    while (argc > argindex) {
        if (!strcmp(argv[argindex], "-l")) {
            byte_order = LITTLE_ENDIAN;
        }
        else if (!strcmp(argv[argindex], "-b")) {
            byte_order = BIG_ENDIAN;
        }
        else if (!read_address) {
            address = strtoul(argv[argindex], &end_ptr, 16);
            if (*end_ptr || errno) {
                printf("dh: invalid address 0x%lx %d", address, errno);
                return CLI_FALSE;
            }
            read_address = true;
        }
        else {
            len = strtoul(argv[argindex], &end_ptr, 16);
            if (*end_ptr || errno) {
                printf("dh: invalid length 0x%x %d", (unsigned int)len, errno);
                return CLI_FALSE;
            }
        }

        argindex++;
    }

    unsigned long stop = address + len;
    int count = 0;

    if ((address & (size - 1)) != 0) {
        printf("dh: unaligned address, cannot display\n");
        return CLI_FALSE;
    }

#if WITH_KERNEL_VM
    /* preflight the start address to see if it's mapped */
    if (vaddr_to_paddr((void *)address) == 0) {
        printf("ERROR: address 0x%lx is unmapped\n", address);
        return CLI_FALSE;
    }
#endif

    for ( ; address < stop; address += size) {
        if (count == 0)
            printf("0x%08lx: ", address);
        switch (size) {
            case 4: {
                uint32_t val = (byte_order != BYTE_ORDER) ?
                               SWAP_32(*(uint32_t *)address) :
                               *(uint32_t *)address;
                printf("%08x ", val);
                break;
            }
            case 2: {
                uint16_t val = (byte_order != BYTE_ORDER) ?
                               SWAP_16(*(uint16_t *)address) :
                               *(uint16_t *)address;
                printf("%04hx ", val);
                break;
            }
            case 1:
                printf("%02hhx ", *(uint8_t *)address);
                break;
        }
        count += size;
        if (count == 16) {
            printf("\n");
            count = 0;
        }
    }

    if (count != 0)
        printf("\n");

    return CLI_FALSE;
}

static int cmd_display_mem_db(int argc, char *argv[])
{
    /* save the last address and len so we can continue where we left off */
    static unsigned long address;
    static size_t len = 0;
    char *end_ptr;

    if (argc < 2 && len == 0) {
        printf("not enough arguments\n");
        printf("db [-l] [-b] [address] [length]\n");
        return CLI_FALSE;
    }

    int size = 1;

    uint byte_order = BYTE_ORDER;
    int argindex = 0;
    bool read_address = false;
    while (argc > argindex) {
        if (!strcmp(argv[argindex], "-l")) {
            byte_order = LITTLE_ENDIAN;
        }
        else if (!strcmp(argv[argindex], "-b")) {
            byte_order = BIG_ENDIAN;
        }
        else if (!read_address) {
            address = strtoul(argv[argindex], &end_ptr, 16);
            if (*end_ptr || errno) {
                printf("db: invalid address 0x%lx %d", address, errno);
                return CLI_FALSE;
            }
            read_address = true;
        }
        else {
            len = strtoul(argv[argindex], &end_ptr, 16);
            if (*end_ptr || errno) {
                printf("db: invalid length 0x%x %d", (unsigned int)len, errno);
                return CLI_FALSE;
            }
        }

        argindex++;
    }

    unsigned long stop = address + len;
    int count = 0;

    if ((address & (size - 1)) != 0) {
        printf("db: unaligned address, cannot display\n");
        return CLI_FALSE;
    }

#if WITH_KERNEL_VM
    /* preflight the start address to see if it's mapped */
    if (vaddr_to_paddr((void *)address) == 0) {
        printf("ERROR: address 0x%lx is unmapped\n", address);
        return CLI_FALSE;
    }
#endif

    for ( ; address < stop; address += size) {
        if (count == 0)
            printf("0x%08lx: ", address);
        switch (size) {
            case 4: {
                uint32_t val = (byte_order != BYTE_ORDER) ?
                               SWAP_32(*(uint32_t *)address) :
                               *(uint32_t *)address;
                printf("%08x ", val);
                break;
            }
            case 2: {
                uint16_t val = (byte_order != BYTE_ORDER) ?
                               SWAP_16(*(uint16_t *)address) :
                               *(uint16_t *)address;
                printf("%04hx ", val);
                break;
            }
            case 1:
                printf("%02hhx ", *(uint8_t *)address);
                break;
        }
        count += size;
        if (count == 16) {
            printf("\n");
            count = 0;
        }
    }

    if (count != 0)
        printf("\n");

    return CLI_FALSE;
}

static int cmd_modify_mem_mw(int argc, char *argv[])
{
    int size;
    char *end_ptr;

    if (argc < 2) {
        printf("not enough arguments\n");
        printf("mw <address> <val>\n");
        return CLI_FALSE;
    }

    size = 4;

    unsigned long address = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("mw: invalid address 0x%lx %d", address, errno);
        return CLI_FALSE;
    }

    unsigned long val = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("mw: invalid val 0x%lx %d", val, errno);
        return CLI_FALSE;
    }

    if ((address & (size - 1)) != 0) {
        printf("mw: unaligned address, cannot modify\n");
        return CLI_FALSE;
    }

    switch (size) {
        case 4:
            *(uint32_t *)address = (uint32_t)val;
            break;
        case 2:
            *(uint16_t *)address = (uint16_t)val;
            break;
        case 1:
            *(uint8_t *)address = (uint8_t)val;
            break;
    }

    return CLI_FALSE;
}

static int cmd_modify_mem_mh(int argc, char *argv[])
{
    int size;
    char *end_ptr;

    if (argc < 2) {
        printf("not enough arguments\n");
        printf("mh <address> <val>\n");
        return CLI_FALSE;
    }

    size = 2;

    unsigned long address = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("mh: invalid address 0x%lx %d", address, errno);
        return CLI_FALSE;
    }

    unsigned long val = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("mh: invalid val 0x%lx %d", val, errno);
        return CLI_FALSE;
    }

    if ((address & (size - 1)) != 0) {
        printf("mh: unaligned address, cannot modify\n");
        return CLI_FALSE;
    }

    switch (size) {
        case 4:
            *(uint32_t *)address = (uint32_t)val;
            break;
        case 2:
            *(uint16_t *)address = (uint16_t)val;
            break;
        case 1:
            *(uint8_t *)address = (uint8_t)val;
            break;
    }

    return CLI_FALSE;
}

static int cmd_modify_mem_mb(int argc, char *argv[])
{
    int size;
    char *end_ptr;

    if (argc < 2) {
        printf("not enough arguments\n");
        printf("mb <address> <val>\n");
        return CLI_FALSE;
    }

    size = 1;

    unsigned long address = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("mb: invalid address 0x%lx %d", address, errno);
        return CLI_FALSE;
    }

    unsigned long val = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("mb: invalid val 0x%lx %d", val, errno);
        return CLI_FALSE;
    }

    if ((address & (size - 1)) != 0) {
        printf("mb: unaligned address, cannot modify\n");
        return CLI_FALSE;
    }

    switch (size) {
        case 4:
            *(uint32_t *)address = (uint32_t)val;
            break;
        case 2:
            *(uint16_t *)address = (uint16_t)val;
            break;
        case 1:
            *(uint8_t *)address = (uint8_t)val;
            break;
    }

    return CLI_FALSE;
}

static int cmd_fill_mem_fw(int argc, char *argv[])
{
    int size;
    char *end_ptr;

    if (argc < 3) {
        printf("not enough arguments\n");
        printf("fw <address> <len> <val>\n");
        return CLI_FALSE;
    }

    size = 4;

    unsigned long address = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fw: invalid address 0x%lx %d", address, errno);
        return CLI_FALSE;
    }

    unsigned long len = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fw: invalid length 0x%lx %d", len, errno);
        return CLI_FALSE;
    }
    unsigned long stop = address + len;
    unsigned int val = strtoul(argv[2], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fw: invalid value 0x%x %d", val, errno);
        return CLI_FALSE;
    }

    if ((address & (size - 1)) != 0) {
        printf("fw: unaligned address, cannot modify\n");
        return CLI_FALSE;
    }

    for ( ; address < stop; address += size) {
        switch (size) {
            case 4:
                *(uint32_t *)address = (uint32_t)val;
                break;
            case 2:
                *(uint16_t *)address = (uint16_t)val;
                break;
            case 1:
                *(uint8_t *)address = (uint8_t)val;
                break;
        }
    }

    return CLI_FALSE;
}

static int cmd_fill_mem_fh(int argc, char *argv[])
{
    int size;
    char *end_ptr;

    if (argc < 3) {
        printf("not enough arguments\n");
        printf("fh <address> <len> <val>\n");
        return CLI_FALSE;
    }

    size = 2;

    unsigned long address = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fh: invalid address 0x%lx %d", address, errno);
        return CLI_FALSE;
    }

    unsigned long len = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fh: invalid length 0x%lx %d", len, errno);
        return CLI_FALSE;
    }
    unsigned long stop = address + len;
    unsigned int val = strtoul(argv[2], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fh: invalid value 0x%x %d", val, errno);
        return CLI_FALSE;
    }

    if ((address & (size - 1)) != 0) {
        printf("fh: unaligned address, cannot modify\n");
        return CLI_FALSE;
    }

    for ( ; address < stop; address += size) {
        switch (size) {
            case 4:
                *(uint32_t *)address = (uint32_t)val;
                break;
            case 2:
                *(uint16_t *)address = (uint16_t)val;
                break;
            case 1:
                *(uint8_t *)address = (uint8_t)val;
                break;
        }
    }

    return CLI_FALSE;
}

static int cmd_fill_mem_fb(int argc, char *argv[])
{
    int size;
    char *end_ptr;

    if (argc < 3) {
        printf("not enough arguments\n");
        printf("fb <address> <len> <val>\n");
        return CLI_FALSE;
    }

    size = 1;

    unsigned long address = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid address 0x%lx %d", address, errno);
        return CLI_FALSE;
    }

    unsigned long len = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid length 0x%lx %d", len, errno);
        return CLI_FALSE;
    }
    unsigned long stop = address + len;
    unsigned int val = strtoul(argv[2], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid value 0x%x %d", val, errno);
        return CLI_FALSE;
    }

    if ((address & (size - 1)) != 0) {
        printf("fb: unaligned address, cannot modify\n");
        return CLI_FALSE;
    }

    for ( ; address < stop; address += size) {
        switch (size) {
            case 4:
                *(uint32_t *)address = (uint32_t)val;
                break;
            case 2:
                *(uint16_t *)address = (uint16_t)val;
                break;
            case 1:
                *(uint8_t *)address = (uint8_t)val;
                break;
        }
    }

    return CLI_FALSE;
}

static int cmd_copy_mem(int argc, char *argv[])
{
    char *end_ptr;
    if (argc < 3) {
        printf("not enough arguments\n");
        printf("mc <source address> <target address> <len>\n");
        return CLI_FALSE;
    }

    unsigned long source = strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid source 0x%lx %d", source, errno);
        return CLI_FALSE;
    }

    unsigned long target = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid target 0x%lx %d", target, errno);
        return CLI_FALSE;
    }

    unsigned long len = strtoul(argv[2], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid len 0x%lx %d", len, errno);
        return CLI_FALSE;
    }

    memcpy((void *)target, (const void *)source, len);

    return CLI_FALSE;
}

static int cmd_memtest(int argc, char *argv[])
{
    char *end_ptr;
    if (argc < 2) {
        printf("not enough arguments\n");
        printf("mtest <base> <len>\n");
        return CLI_FALSE;
    }

    uint32_t *ptr;
    size_t len;

    ptr = (uint32_t *)strtoul(argv[0], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid ptr 0x%p %d", ptr, errno);
        return CLI_FALSE;
    }

    len = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || errno) {
        printf("fb: invalid len 0x%x %d", len, errno);
        return CLI_FALSE;
    }

    size_t i;
    // write out
    printf("writing first pass...");
    for (i = 0; i < len / 4; i++) {
        ptr[i] = i;
    }
    printf("done\n");

    // verify
    printf("verifying...");
    for (i = 0; i < len / 4; i++) {
        if (ptr[i] != i)
            printf("error at %p\n", &ptr[i]);
    }
    printf("done\n");

    return CLI_FALSE;
}

static int cmd_chain(int argc, char *argv[])
{
    // if (argc < 3) {
    //     printf("not enough arguments\n");
    //     printf("%s <address>\n", argv[0].str);
    //     return -1;
    // }

    // arch_chain_load(argv[1].p, (ulong)argv[2].p, 0, 0, 0);

    return CLI_FALSE;
}

static void mdelay(int ms)
{
    udelay(ms*1000);
}

static int cmd_sleep_s(int argc, char *argv[])
{
    uint32_t t = 1000; /* default to 1 second */
    char *end_ptr;

    if (argc >= 1) {
        t = strtoul(argv[0], &end_ptr, 16);
        if (*end_ptr || errno) {
            printf("sleep: invalid t 0x%x %d", t, errno);
            return CLI_FALSE;
        }
        t *= 1000;
    }

    mdelay(t);

    return CLI_FALSE;
}

static int cmd_sleep_ms(int argc, char *argv[])
{
    uint32_t t = 1000; /* default to 1 second */
    char *end_ptr;

    if (argc >= 1) {
        t = strtoul(argv[0], &end_ptr, 16);
        if (*end_ptr || errno) {
            printf("sleep: invalid t 0x%x %d", t, errno);
            return CLI_FALSE;
        }
    }

    mdelay(t);

    return CLI_FALSE;
}

static int cmd_crash(int argc, char *argv[])
{
    /* should crash */
    volatile uint32_t *ptr = (void *)1;
    *ptr = 1;

    /* if it didn't, panic the system */
    PANIC();

    return CLI_FALSE;
}

static int cmd_stackstomp(int argc, char *argv[])
{
    for (size_t i = 0; i < DEFAULT_STACK_SIZE * 2; i++) {
        uint8_t death[i];

        memset(death, 0xaa, i);
        mdelay(1);
    }

    printf("survived.\n");

    return CLI_FALSE;
}

CLI_CMD("dw", "display memory in words", cmd_display_mem_dw);
CLI_CMD("dh", "display memory in halfwords", cmd_display_mem_dh);
CLI_CMD("db", "display memory in bytes", cmd_display_mem_db);

CLI_CMD("mw", "modify word of memory", cmd_modify_mem_mw);
CLI_CMD("mh", "modify halfword of memory", cmd_modify_mem_mh);
CLI_CMD("mb", "modify byte of memory", cmd_modify_mem_mb);

CLI_CMD("fw", "fill range of memory by word", cmd_fill_mem_fw);
CLI_CMD("fh", "fill range of memory by halfword", cmd_fill_mem_fh);
CLI_CMD("fb", "fill range of memory by byte", cmd_fill_mem_fb);
CLI_CMD("mc", "copy a range of memory", cmd_copy_mem);

CLI_CMD("crash", "intentionally crash", cmd_crash);
CLI_CMD("stackstomp", "intentionally overrun the stack", cmd_stackstomp);

#if LK_DEBUGLEVEL > 1
CLI_CMD("mtest", "simple memory test", cmd_memtest);
#endif
CLI_CMD("chain", "chain load another binary", cmd_chain);
CLI_CMD("sleep", "sleep number of seconds\r\n\tsleep 5", cmd_sleep_s);
CLI_CMD("sleepm", "sleep number of milliseconds\r\n\tsleepm 1000", cmd_sleep_ms);