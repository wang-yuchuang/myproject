/**
 * @file pvt_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt test implement
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
#include <driver/kunlun/pvt/pvt.h>
#include <pvt_cfg.h>

#define get_systick() timer_get_global_time(g_syscnt_timer)

#define EXTRA_FMT   "pvt_test: "
#define TOSTRING(x) (#x)
#define TEST_PVT_LOG(format, ...) ssdk_printf(SSDK_EMERG, EXTRA_FMT "[tick: %lld]" format, get_systick()/1000, ##__VA_ARGS__)

#define KUNLUN_DEVICE_TYPE  PVT_DEVICE_TYPE_ULVT

/* type of subcommand function */
typedef void (*cmd_entry)(pvt_dev_t** dev, int32_t argc, char* argv[]);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;


static void pvt_irq_handle(uint16_t chanId, void* argv)
{
    pvt_dev_t* dev_ins = argv;
    TEST_PVT_LOG("alarm channel: %d interrupt callback, ins base: 0x%x\n", chanId, dev_ins->base);
}

static pvt_dev_t* get_dev_ins(pvt_dev_t** dev, int32_t ins_id)
{
    if (ins_id < PVT_DEV_INS_NUM &&
        ins_id >= 0) {
        return dev[ins_id];
    }
    else {
        return NULL;
    }

}

void test_pvt_get_pvt(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    pvt_data_t data;
    pvt_get_pvt(dev_ins, &data);

    TEST_PVT_LOG("get pvt data, temp: %5.2f, voltage: %5.2f, process: %d\n"
                    , data.temp
                    , data.voltage
                    , data.process);

}

/**
 * @brief set alarm
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] alarm mode, 0:temp  1:voltage
 * @param argv [2] interrupt type 0:hsyth 1:hystl 2:hsytr 3:hyst:f
 * @param argv [3] thresh heigh
 * @param argv [4] thresh low
 */
void test_pvt_set_alarm(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t alarm_mode = atoi(argv[1]);
    int32_t alarm_type = atoi(argv[2]);
    float thresh_heigh = (float)atoi(argv[3]);
    float thresh_low   = (float)atoi(argv[4]);
    int32_t chanId = -1;

    pvt_alarm_para_t para;
    memset(&para, 0x00, sizeof(para));

    para.high_val = thresh_heigh;
    para.low_val = thresh_low;
    para.cb_fun  = pvt_irq_handle;
    para.args = dev_ins;

    chanId = pvt_set_alarm(dev_ins, alarm_mode, alarm_type, &para);

    TEST_PVT_LOG("set alarm channel id is: %d\n", chanId);
}

/**
 * @brief enable alarm
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] alarm channel id
 */
void test_pvt_enable_alarm(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t chan_id = atoi(argv[1]);
    pvt_enable_alarm(dev_ins, chan_id);
    TEST_PVT_LOG("enable alarm channel %d\n", chan_id);
}

/**
 * @brief clear alarm
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] alarm channel id
 */
void test_pvt_clr_alarm(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t chan_id = atoi(argv[1]);
    pvt_clr_alarm(dev_ins, chan_id);
    TEST_PVT_LOG("clear alarm channel %d\n", chan_id);
}

/**
 * @brief disable alarm
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] alarm channel id
 */
void test_pvt_disable_alarm(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t chan_id = atoi(argv[1]);
    pvt_disable_alarm(dev_ins, chan_id);
    TEST_PVT_LOG("disable alarm channel %d\n", chan_id);
}

/**
 * @brief get integer data
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] data type
 * @param argv [2] channel id
 */
void test_pvt_get_integer_data(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t data_type = atoi(argv[1]);
    int32_t channel_id = atoi(argv[2]);
    int32_t out_data = 0;
    pvt_integer_data_get(dev_ins, data_type, channel_id, &out_data);

    TEST_PVT_LOG("get %d dev ins %d data and value is: %d \n", atoi(argv[0]), data_type, out_data);
}

/**
 * @brief get float data
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] data type
 * @param argv [2] channel id
 */
void test_pvt_get_float_data(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t data_type = atoi(argv[1]);
    int32_t channel_id = atoi(argv[2]);
    float out_data = 0.0;

    pvt_float_data_get(dev_ins, data_type, channel_id, &out_data);

    TEST_PVT_LOG("get %d dev ins %d data and value is: %5.2f \n", atoi(argv[0]), data_type, out_data);
}

/**
 * @brief get float data
 *
 * @param dev
 * @param argc
 * @param argv [0] pvt ins index
 * @param argv [1] data type
 * @param argv [2] data val
 */
void test_pvt_set_integer_data(pvt_dev_t** dev, int32_t argc, char* argv[])
{
    pvt_dev_t* dev_ins = NULL;
    if ((dev_ins = get_dev_ins(dev, atoi(argv[0]))) == NULL) {
        TEST_PVT_LOG("dev ins id: %d is invalid\n", atoi(argv[0]));
        return;
    }

    int32_t data_type = atoi(argv[1]);
    int32_t data_val = atoi(argv[2]);
    pvt_integer_data_set(dev_ins, data_type, data_val);

    TEST_PVT_LOG("set %d dev ins %d data and value is: %d\n", atoi(argv[0]), data_type, data_val);
}

/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"get_pvt"       , 1, test_pvt_get_pvt},
    {"set_alarm"     , 5, test_pvt_set_alarm},
    {"clr_alarm"     , 2, test_pvt_clr_alarm},
    {"enable_alarm"  , 2, test_pvt_enable_alarm},
    {"disable_alarm" , 2, test_pvt_disable_alarm},
    {"get_int"       , 3, test_pvt_get_integer_data},
    {"get_float"     , 3, test_pvt_get_float_data},
    {"set_int"       , 3, test_pvt_set_integer_data},
    {NULL}
};

void subCmd_help_msg()
{
    printf("get_pvt [dev ins] \n");
    printf("set_alarm [dev ins] [alarm mode] [alarm type] [high val] [low val]\n");
    printf("clr_alarm [dev ins] [chanId]\n");
    printf("enable_alarm [dev ins] [chanId]\n");
    printf("disbale_alarm [dev ins] [chanId]\n");
    printf("get_int [dev ins] [data type] [chanId]\n");
    printf("get_float [dev ins] [data type] [chanId]\n");
    printf("set_int [dev ins] [data type] [data val]\n");
}

int32_t pvt_test_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;
    pvt_dev_t** dev_ptr = NULL;

    // parameter check
    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "help", strlen("help")) == 0 ||
        strncmp(argv[0], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    // test init
    // board_pvt_init();
    dev_ptr = g_pvt_dev_ptr;

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
                TEST_PVT_LOG("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        TEST_PVT_LOG("do not support sub cmd: %s for pvt_test\n", argv[0]);
        return 0;
    }

    return 0;
}

CLI_CMD("pvt_test", "pvt function test", pvt_test_main);

/* end of file pvt_test.c */