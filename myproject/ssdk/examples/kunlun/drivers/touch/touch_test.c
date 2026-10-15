/**
 * @file touch_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: touch test implement file
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
#include <touch_cfg.h>
#include <serdes_cfg.h>
#include <driver/kunlun/touch/touch.h>

#define get_systick() timer_get_global_time(g_syscnt_timer)

#define EXTRA_FMT   "serdes_test: "
#define TOSTRING(x) (#x)
#define TEST_TOUCH_LOG(format, ...) ssdk_printf(SSDK_EMERG, EXTRA_FMT "[tick: %lld]" format, get_systick()/1000, ##__VA_ARGS__)

/* type of subcommand function */
typedef void (*cmd_entry)(touch_dev_t** dev, int32_t argc, char* argv[]);
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

int32_t touch_sensor_data_callback(int32_t scId, touch_notice_t* notice, void* usrCtx)
{
    if (notice == NULL) {
        TEST_TOUCH_LOG("invalid parameter for %s.\n", __func__);
        return -1;
    }

    TEST_TOUCH_LOG("receive screen id: %d touch sensor data, type is: ", scId);

    switch(notice->evt_type) {
        case TS_NOTICE_EVT_COORD:
            printf("%s, point num: %d\n", TOSTRING(TS_NOTICE_EVT_COORD), notice->coord.touch_num);
            for (int32_t i = 0; i < notice->coord.touch_num; i++) {
                printf("  (%d, %d, %d, %d)\n"
                        , notice->coord.point[i].id
                        , notice->coord.point[i].x
                        , notice->coord.point[i].y
                        , notice->coord.point[i].w);
            }
            break;
        default:
            break;
    }

    return 0;
}

/**
 * @brief read touch register for specify len
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv[0] touch device index
 * @param argv[1] register addr
 * @param argv[2] read length
 */
void touch_test_read_reg(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    uint32_t reg_addr = str2hex(argv[1]);
    uint8_t read_num = atoi(argv[2]);

    uint32_t buf[256] = {0};

    for (int32_t i = 0; i < read_num; i++) {
        touch_reg_read(dev[touch_index], reg_addr + i, &buf[i]);
    }

    printf("touch %s register read value is: \n",dev[touch_index]->name);
    for (int32_t i = 0; i < read_num; i++) {
        if (i % 0x10 == 0) {
            printf("\n0x%04x: ", reg_addr + i);
        }
        printf("0x%02x ", buf[i]);
    }
}

/**
 * @brief write touch register for specify value
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv[0] touch device index
 * @param argv[1] register addr
 * @param argv[2] write value
 */
void touch_test_write_reg(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    uint32_t reg_addr = str2hex(argv[1]);
    uint32_t reg_val = atoi(argv[2]);

    touch_reg_write(dev[touch_index], reg_addr, reg_val);
    printf("touch register write addr: 0x%x, val: %d\n", reg_addr, reg_val);
}

/**
 * @brief check touch device online status
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv[0] touch device index
 */
void touch_test_check_status(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    if (touch_online_check(dev[touch_index]) == 0) {
        printf("touch connect ok\n");
    }
    else {
        printf("touch connect ng\n");
    }
}

/**
 * @brief board touch init
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv null
 */
void touch_test_probe(touch_dev_t** dev, int32_t argc, char* argv[])
{
    board_touch_init();
}

/**
 * @brief register callback for touch sensor data
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv[0] touch device index
 */
void touch_test_register_cb(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    touch_register_callback(dev[touch_index], touch_sensor_data_callback, NULL);
    printf("touch test register callback for touch sensor \n");
}

/**
 * @brief register callback for touch sensor data
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv[0] touch device index
 * @param argv[1] touch control type
 */
void touch_test_device_control(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    int32_t ctrl_type = atoi(argv[1]);

    touch_device_control(dev[touch_index], ctrl_type, NULL, NULL);
}

/**
 * @brief register callback for touch sensor data
 *
 * @param dev touch device arry
 * @param argc arguments counter
 * @param argv[0] touch device index
 * @param argv[1] touch control type
 */
void touch_test_device_reboot(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);

    touch_deinit(dev[touch_index]);
    udelay(1000);
    touch_init(dev[touch_index]);
}

void touch_test_stress_test(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    int32_t cnt_max = atoi(argv[1]);
    int32_t test_cnt = 0;
    while(test_cnt < cnt_max) {
        printf("[%lld]start reboot touch device, cnt: %d, rang max: %d\n", get_systick(), test_cnt++, cnt_max);
        touch_test_device_reboot(dev, argc, argv);
        osDelay(2000);
        printf("[%lld]start check touch online status\n", get_systick());
        if (touch_online_check(dev[touch_index]) == 0) {
            printf("[%lld]check online status is ok\n", get_systick());
            continue;
        }
        else {
            printf("[%lld]check online status is ng\n", get_systick());
            break;
        }
    }
}
void touch_test_send_flg_l(touch_dev_t** dev, int32_t argc, char* argv[])
{
    int32_t touch_index = atoi(argv[0]);
    touch_test_send_flg(dev[touch_index]);
}
/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"probe"    , 0, touch_test_probe},
    {"read_reg" , 3, touch_test_read_reg},
    {"write_reg", 3, touch_test_write_reg},
    {"check"    , 1, touch_test_check_status},
    {"register" , 1, touch_test_register_cb},
    {"dev_ctrl" , 2, touch_test_device_control},
    {"reboot"   , 1, touch_test_device_reboot},
    {"stress_test", 1, touch_test_stress_test},
    {"send"     , 1, touch_test_send_flg_l},
    {NULL}
};

static void subCmd_help_msg()
{
    printf("touch test help msg\n");
    printf("touch_test probe: \n");
    printf("touch_test read_reg [devIndex] [regAddress] [readLen] -> read touch register\n");
    printf("touch_test write_reg [devIndex] [regAddress] [val] -> write touch register\n");
    printf("touch_test check [devIndex] -> check touch device online status\n");
    printf("touch_test register [devIndex] -> register touch sensor data callback\n");
    printf("touch_test dev_ctrl [devIndex] [ctrlType]-> touch device control. 0: reset dev \n");
    printf("touch_test reboot [devIndex] -> reboot touch device \n");
    return;
}

int32_t touch_test_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;
    touch_dev_t** dev_ptr = NULL;

    // parameter check
    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    dev_ptr = g_touch_dev;

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
                TEST_TOUCH_LOG("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        TEST_TOUCH_LOG("do not support sub cmd: %s for touch_test\n", argv[1]);
        return 0;
    }

    return 0;
}

CLI_CMD("touch_test", "touch function test, touch_test -h for more information", touch_test_main);
/* end of file touch_test.c */