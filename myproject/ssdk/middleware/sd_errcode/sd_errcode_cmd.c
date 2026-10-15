/*
 * sd_errcode_cmd.c
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: error code validation.
 *
 **/

#include <reg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <CLI_type.h>
#include <sd_errcode.h>

static void print_usage(void)
{
    printf("errcode_test command usage:\n");
    printf("errcode_test set errorcode userdata\n");
    printf("errcode_test transfer ip errorcode userdata\n");
    printf("errcode_test dump|clear storage_id\n");
    printf("errcode_test set_strid storage_id\n");
    printf("For example:\n");
    printf("errcode_test set 0x12345678 0x1122\n");
    printf("errcode_test transfer 192.168.1.10 0x12345678 0x3344\n");
    printf("errcode_test dump 2\n");
    printf("errcode_test clear 3\n");
    printf("errcode_test set_strid 2\n");
    printf("errcode_test get_strid\n");
}

static int errcode_test(int argc, char *argv[])
{
    uint32_t errorcode = 0xFFFFFFFF;
    uint32_t userdata = 0xFFFFFFFF;
    int storage_id = 0;
    char ip[20] = {0};
    uint8_t mod_id = 0;
    uint8_t mod_no = 0;
    uint16_t tmp_errcode = 0;

    if (argc < 1) {
        print_usage();
        return CLI_FALSE;
    }

    if (!strcmp(argv[0], "set")) {
        if (argc != 3) {
            print_usage();
            return CLI_FALSE;
        }
        errorcode = strtoul(argv[1], NULL, 16);
        userdata = strtoul(argv[2], NULL, 16);
        mod_id = errorcode>>24;
        mod_no = (errorcode>>16)&0xFF;
        tmp_errcode = errorcode&0xFFFF;
        printf("errorcode = 0x%x, userdata = 0x%x\n", errorcode, userdata);
        printf("mod_id = 0x%02x, mod_no = 0x%02x, errcode = 0x%04x\n", mod_id, mod_no, tmp_errcode);

        ERRCODE_PRINT(mod_id, mod_no, tmp_errcode, userdata);
    }
    else if (!strcmp(argv[0], "transfer")) {
        if (argc != 4) {
            print_usage();
            return CLI_FALSE;
        }
        strcpy(ip, argv[1]);
        errorcode = strtoul(argv[2], NULL, 16);
        userdata = strtoul(argv[3], NULL, 16);
        mod_id = errorcode>>24;
        mod_no = (errorcode>>16)&0xFF;
        tmp_errcode = errorcode&0xFFFF;
        printf("ip = %s, userdata = 0x%x, errorcode = 0x%x\n", ip, userdata, errorcode);
        printf("mod_id = 0x%02x, mod_no = 0x%02x, errcode = 0x%04x\n", mod_id, mod_no, tmp_errcode);

        errcode_t error_code = ERRCODE_ITEM(mod_id, mod_no, tmp_errcode, userdata);
        errcode_transfer(ip, &error_code);
    }
    else if (!strcmp(argv[0], "dump")) {
        storage_id = strtoul(argv[1], NULL, 16);
        errcode_dump(storage_id, NULL, 0);
    }
    else if (!strcmp(argv[0], "clear")) {
        storage_id = strtoul(argv[1], NULL, 16);
        errcode_clear(storage_id);
    }
    else if (!strcmp(argv[0], "set_strid")) {
        storage_id = strtoul(argv[1], NULL, 16);

        system_errcode_set_storage_id(storage_id);
    }
    else if (!strcmp(argv[0], "get_strid")) {
        storage_id = system_errcode_get_storage_id();
        printf("get_strid = %d\n", storage_id);
    }
    else {
        print_usage();
    }

    return CLI_FALSE;
}

#if CONFIG_ERRCODE
CLI_CMD("errcode_test", "set|transfer|dump|clear storage_id [errorcode] [userdata]", errcode_test);
#endif

