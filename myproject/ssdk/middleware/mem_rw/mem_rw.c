/**
 * @file mem_rw.c
 * @brief
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "CLI_type.h"

/* dw/dh/db start len */
static int cmd_display_mem(int argc, char *argv[])
{
    char            *end_ptr;
    char            size;
    unsigned long   reg_start;
    unsigned long   reg_len;
    void            *p;
    void            *p_end;
    int             i;

    if (argc != 3)
        return CLI_FALSE;

    errno = 0;
    reg_start = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || ((reg_start == ULONG_MAX) && errno)) {
        printf("start addr err %s %#lX %d", argv[1], reg_start, errno);
        return CLI_FALSE;
    }

    errno = 0;
    reg_len = strtoul(argv[2], &end_ptr, 16);
    if (*end_ptr || ((reg_len == ULONG_MAX) && errno)) {
        printf("length err %s %#lX %d", argv[2], reg_len, errno);
        return CLI_FALSE;
    }

    if ((void *)(reg_start + reg_len) <= (void *)reg_start) {
        printf("invalid parameter %#lX %#lX", reg_start, reg_len);
        return CLI_FALSE;
    }

    switch (argv[0][1]) {
    case 'w':
        size = 4;
        break;
    case 'h':
        size = 2;
        break;
    default:
        size = 1;
        break;
    }

    printf("\n%08lX    0%12c%12c%12c", reg_start & ~0xFFFFul, '4', '8', 'C');

    p = (void *)(reg_start & ~0xFul);
    p_end = (void *)(reg_start + reg_len);

    while (p < p_end) {
        printf("\n    %04lX    ", (uintptr_t)p & 0xFFFFul);
        for (i = 0; i < 16; ) {
            if ((p >= (void *)reg_start) && (p < p_end)) {
                switch (size) {
                case 4:
                    printf("%08X    ", *(uint32_t *)p);
                    break;
                case 2:
                    printf("%04X  ", *(uint16_t *)p);
                    break;
                default:
                    printf("%02X ", *(uint8_t *)p);
                    break;
                }
            } else {
                printf("%*c", size * 3, ' ');
            }
            i += size;
            p = (char *)p + size;
        }
    }

    printf("\n");

    return CLI_FALSE;
}

static int _cli_inter_display_mem(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int argc = 0;
    char *argv[CLI_MAX_ARGS];

    argc = CLITokenizeCommand(pcCommandString, argv + 1);
    argv[0] = (char *)pcCommandString;
    argc++;

    return cmd_display_mem(argc, argv);
}
static const char _cli_help_display_word[] = "dw <start_addr_hex> <len_hex>"
                "\r\n\tRead and display the memory range or registers by [word].";
static const char _cli_help_display_hword[] = "\r\ndh <start_addr_hex> <len_hex>:"
                "\r\n\tRead and display the memory range or registers by [half_word].";
static const char _cli_help_display_byte[] = "\r\ndb <start_addr_hex> <len_hex>:"
                "\r\n\tRead and display the memory range or registers by [byte].";
__USED static const CLI_Command_Definition_t _cli_cmd_display_mem[] __SECTION(".commands") = {
    {"dw", _cli_help_display_word, _cli_inter_display_mem},
    {"dh", _cli_help_display_hword, _cli_inter_display_mem},
    {"db", _cli_help_display_byte, _cli_inter_display_mem},
};

static int cmd_modify_mem(int argc, char *argv[])
{
    char            *end_ptr;
    char            type;
    unsigned long   reg_addr;
    unsigned long   reg_data;

    if (argc != 3)
        return CLI_FALSE;

    errno = 0;
    reg_addr = strtoul(argv[1], &end_ptr, 16);
    if (*end_ptr || ((reg_addr == ULONG_MAX) && errno)) {
        printf("addr err %s %#lX %d", argv[1], reg_addr, errno);
        return CLI_FALSE;
    }

    errno = 0;
    reg_data = strtoul(argv[2], &end_ptr, 16);
    if (*end_ptr || ((reg_data == ULONG_MAX) && errno)) {
        printf("data err %s %#lX %d", argv[2], reg_data, errno);
        return CLI_FALSE;
    }

    type = argv[0][1];
    if (((type == 'w') && (reg_addr & 0x3ul)) ||
        ((type == 'h') && (reg_addr & 0x1ul))) {
        printf("mem addr %#lX doesnot match type %c\n", reg_addr, type);
        return CLI_FALSE;
    }

    switch (type) {
    case 'w':
        *(volatile uint32_t *)reg_addr = reg_data;
        break;
    case 'h':
        *(volatile uint16_t *)reg_addr = reg_data;
        break;
    case 'b':
        *(volatile uint8_t *)reg_addr = reg_data;
        break;
    default:
        return CLI_FALSE;
    }

    printf("write 0x%lX to 0x%lX\n", reg_data, reg_addr);

    return CLI_FALSE;
}

static int _cli_inter_modify_mem(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    int argc = 0;
    char *argv[CLI_MAX_ARGS];

    argc = CLITokenizeCommand(pcCommandString, argv + 1);
    argv[0] = (char *)pcCommandString;
    argc++;

    return cmd_modify_mem(argc, argv);
}
static const char _cli_help_modify_word[] = "\r\nmw <start_addr_hex> <data_hex>:\r\n"
                " Write a [word] to memory or register.\r\n";
static const char _cli_help_modify_hword[] = "\r\nmh <start_addr_hex> <data_hex>:\r\n"
                " Write a [half_word] to memory or register.\r\n";
static const char _cli_help_modify_byte[] = "\r\nmb <start_addr_hex> <data_hex>:\r\n"
                " Write a [byte] to memory or register.\r\n";
__USED static const CLI_Command_Definition_t _cli_cmd_modify_mem[] __SECTION(".commands") = {
    {"mw", _cli_help_modify_word, _cli_inter_modify_mem},
    {"mh", _cli_help_modify_hword, _cli_inter_modify_mem},
    {"mb", _cli_help_modify_byte, _cli_inter_modify_mem},
};
