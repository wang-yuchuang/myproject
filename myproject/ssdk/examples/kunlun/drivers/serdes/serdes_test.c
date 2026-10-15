/**
 * @file serdes_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes test implement file
 *
 * Revision History:
 * -----------------
*/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <CLI.h>
#include <CLI_type.h>
#include <debug.h>
#include <board.h>
#include <reg.h>
#include <driver/timer.h>
#include <timer_cfg.h>
#include <serdes_cfg.h>
#include <driver/kunlun/serdes/serdes.h>

#define get_systick() timer_get_global_time(g_syscnt_timer)

#define EXTRA_FMT   "serdes_test: "
#define TOSTRING(x) (#x)
#define TEST_SERDES_LOG(format, ...) ssdk_printf(SSDK_EMERG, EXTRA_FMT "[tick: %lld]" format, get_systick()/1000, ##__VA_ARGS__)

/* type of subcommand function */
typedef void (*cmd_entry)(serdes_dev_t** dev, int32_t argc, char* argv[]);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;

static unsigned long int str2hex(const char* str)
{
    static char buf[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    unsigned long int integer = 0;

    int32_t pos = strlen(str);
    unsigned long base = 1;
    while (--pos >= 0) {
        for (int32_t i = 0; i < sizeof(buf); i++) {
            if (str[pos] == buf[i] ||
                ((str[pos] == buf[i] + 'A' - 'a') &&
                 (i >= 10))) {
                integer += i * base;
            }
        }
        base *= 16;
    }

    return integer;
}

void check_serdes_link(serdes_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t dev_index = atoi(argv[0]);

    serdes_check_serdes_link(dev[dev_index]);
}

/**
 * @brief read serdes register
 *
 * @param dev serdes device arry
 * @param argc 3
 * @param argv1 serdes dev index
 * @param argv2 ser or des type
 * @param argv3 read register address
 */
void read_serdes_register(serdes_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t dev_index = atoi(argv[0]);
    int32_t dev_type = atoi(argv[1]);
    int32_t reg_address = str2hex(argv[2]);
    uint8_t read_num = atoi(argv[3]);
    uint32_t buf[256] = {0};

    int32_t (*serdes_fun)(serdes_dev_t*, uint32_t, uint32_t*) = NULL;

    switch (dev_type) {
        case 0:
            serdes_fun = serdes_ser_reg_read;
            break;
        case 1:
            serdes_fun = serdes_des_reg_read;
            break;
        default:
            break;
    }

    if (serdes_fun != NULL) {
        for (int32_t i = 0; i < read_num; i++) {
            serdes_fun(dev[dev_index], reg_address + i, &buf[i]);
        }
    }

    printf("read %s - %s register 0x%x :\n"
            , dev[dev_index]->name
            , dev_type == 0 ? "ser" : "des"
            , reg_address);

    for (int32_t i = 0; i < read_num; i++) {
        if (i % 0x10 == 0) {
            printf("\n0x%02x: ", reg_address + i);
        }
        printf("0x%02x ", buf[i]);
    }
}

/**
 * @brief write serdes register
 *
 * @param dev serdes device arry
 * @param argc 4
 * @param argv1 serdes dev index
 * @param argv2 ser or des type
 * @param argv3 write register address
 * @param argv4 write value
 */
void write_serdes_register(serdes_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t dev_index = atoi(argv[0]);
    int32_t dev_type = atoi(argv[1]);
    int32_t reg_address = str2hex(argv[2]);
    uint32_t val = str2hex(argv[3]);


    switch (dev_type) {
        case 0:
            serdes_ser_reg_write(dev[dev_index], reg_address, val);
            break;
        case 1:
            serdes_des_reg_write(dev[dev_index], reg_address, val);
            break;
        default:
            break;
    }

    printf("write %s - %s register 0x%x val is: 0x%x\n"
            , dev[dev_index]->name
            , dev_type == 0 ? "ser" : "des"
            , reg_address
            , val);

    // read_serdes_register(dev, argc, argv);
}

/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"check_link"   , 1  , check_serdes_link},
    {"read_reg"     , 4  , read_serdes_register},
    {"write_reg"    , 4  , write_serdes_register},
    {NULL}
};

static void subCmd_help_msg()
{
    printf("serdes_test [sub cmd]:\n");
    printf("    check_link: check serdes link status\n");
    printf("    read_reg: read serdes register\n");
    printf("    write_reg: write serdes register\n");
    return;
}

int32_t serdes_test_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;
    serdes_dev_t** dev_ptr = NULL;

    // parameter check
    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    // test init
    // board_serdes_init();
    dev_ptr = g_serdes_dev;

    // command parser
    int32_t i = -1;
    int32_t subcmd_parnum = argc - validpar_index;
    while (subcmd_parnum >= 0 &&
            s_subCmd_mgr_tbl[++i].sub_cmd != NULL) {
        if (strncmp(argv[validpar_index - 1], s_subCmd_mgr_tbl[i].sub_cmd, strlen(argv[validpar_index - 1])) == 0 &&
            strlen(argv[validpar_index - 1]) == strlen(s_subCmd_mgr_tbl[i].sub_cmd)) {
            if (subcmd_parnum == s_subCmd_mgr_tbl[i].par_num) {
                s_subCmd_mgr_tbl[i].entry(dev_ptr, subcmd_parnum, &argv[validpar_index]);
            }
            else {
                TEST_SERDES_LOG("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        TEST_SERDES_LOG("do not support sub cmd: %s for serdes_test\n", argv[1]);
        return 0;
    }

    return 0;
}

CLI_CMD("serdes_test", "serdes function test", serdes_test_main);


/* end of file serdes_test.c */