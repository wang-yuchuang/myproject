/**
 * @file input_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: input service test
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <cmsis_os2.h>
#include <CLI.h>
#include <CLI_type.h>
#include <debug.h>
#include <board.h>
#include <reg.h>
#include <driver/timer.h>
#include <timer_cfg.h>

#include <input_cfg.h>
#include <input_service.h>
#define get_systick() timer_get_global_time(g_syscnt_timer)

#define EXTRA_FMT   "input_test: "
#define TOSTRING(x) (#x)
#define TEST_INPUT_LOG(format, ...) ssdk_printf(SSDK_EMERG, EXTRA_FMT "[tick: %lld]" format, get_systick()/1000, ##__VA_ARGS__)

/* type of subcommand function */
typedef void (*cmd_entry)(int32_t argc, char* argv[]);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;

/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

static int32_t ts_cb(int32_t tsDomain, ts_touch_report_data_t* repData, void* usrCtx)
{
    printf("receive domain: %d touch data, touch point number: %d\n", tsDomain, repData->touch_num);
    for (int32_t i = 0; i < repData->touch_num; i++) {
        printf("(%d, %d)\n", repData->coord_data[i].x, repData->coord_data[i].y);
    }
    return 0;
}

static void test_touch_service_start(int32_t argc, char* argv[])
{
    printf("start input service\n");
    touch_service_start(g_ts_report_cfg_tbl, TS_REPORT_CFG_TBL_SIZE);
}

static void test_touch_service_stop(int32_t argc, char* argv[])
{
    printf("stop input service\n");
    touch_service_stop();
}

static void test_touch_service_sendMsg(int32_t argc, char* argv[])
{
    touch_service_send_msg(atoi(argv[0]) | TS_MSG_EXT_FLG);
    printf("send %d msg to touch service\n", atoi(argv[0]));
}

static void test_touch_service_get_touchData(int32_t argc, char* argv[])
{
    int32_t ts_domain = atoi(argv[0]);
    if (touch_service_ts_domain_is_pressed(ts_domain)) {
        int32_t x, y = 0;
        touch_service_ts_domain_get_point(&x, &y, ts_domain);
        printf("current domain: %d touch point is: (%d, %d)\n", ts_domain, x, y);
    }
    else {
        printf("current domain: %d touch is not pressed\n", ts_domain);
    }
}

/**
 * @brief register callback for specifed function domain
 *
 * @param argc 1
 * @param argv[0] function domain refs @
 */
static void test_touch_register_cb(int32_t argc, char* argv[])
{
    if (atoi(argv[0]) >= TS_DO_MAX || atoi(argv[0]) < 0) {
        printf("invalid function domain:%s for touch\n", argv[0]);
        printf("\t-- %d: %s\n", TS_DO_ANRDOID_MAIN, "TS_DO_ANRDOID_MAIN");
        printf("\t-- %d: %s\n", TS_DO_ANRDOID_AUX1, "TS_DO_ANRDOID_AUX1");
        printf("\t-- %d: %s\n", TS_DO_ANRDOID_AUX2, "TS_DO_ANRDOID_AUX2");
        printf("\t-- %d: %s\n", TS_DO_ANRDOID_AUX3, "TS_DO_ANRDOID_AUX3");
        printf("\t-- %d: %s\n", TS_DO_AP2_MAIN, "TS_DO_AP2_MAIN");
        printf("\t-- %d: %s\n", TS_DO_AP2_AUX1, "TS_DO_AP2_AUX1");
        printf("\t-- %d: %s\n", TS_DO_CTRLPANEL_MAIN, "TS_DO_CTRLPANEL_MAIN");
        printf("\t-- %d: %s\n", TS_DO_CTRLPANEL_AUX1, "TS_DO_CTRLPANEL_AUX1");
        printf("\t-- %d: %s\n", TS_DO_FAST_AVM, "TS_DO_FAST_AVM");
    }
    else {
        printf("register callback for touch function domain: %s\n", argv[0]);
        touch_service_register_domain_callback(atoi(argv[0]), ts_cb, NULL);
    }
}

/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"start"    , 0, test_touch_service_start},
    {"stop"     , 0, test_touch_service_stop},
    {"sendmsg"  , 1, test_touch_service_sendMsg},
    {"getdata"  , 1, test_touch_service_get_touchData},
    {"regcb"    , 1, test_touch_register_cb},
    {NULL}
};

static void subCmd_help_msg()
{
    printf("input test help msg\n");
    printf("input_test start. \n");
    printf("input_test stop. \n");
    printf("input_test sendmsg [msgType]  -> send specified msg to input service. 0: start avm, 1: stop avm\n");
    printf("input_test getdata [tsDomain] -> get specified touch function domain touch data\n");
    printf("input_test regcb [tsDomain] -> register specified touch function domain callback\n");
    return;
}

int32_t input_test_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;

    // parameter check
    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    // command parser
    int32_t i = -1;
    int32_t subcmd_parnum = argc - validpar_index;
    while (subcmd_parnum >= 0 &&
            s_subCmd_mgr_tbl[++i].sub_cmd != NULL) {
        if (strncmp(argv[validpar_index - 1], s_subCmd_mgr_tbl[i].sub_cmd, strlen(argv[validpar_index - 1])) == 0 &&
            strlen(argv[validpar_index - 1]) == strlen(s_subCmd_mgr_tbl[i].sub_cmd)) {
            if (subcmd_parnum == s_subCmd_mgr_tbl[i].par_num) {
                s_subCmd_mgr_tbl[i].entry(subcmd_parnum, &argv[validpar_index]);
            }
            else {
                TEST_INPUT_LOG("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        TEST_INPUT_LOG("do not support sub cmd: %s for input_test\n", argv[0]);
        return 0;
    }

    return 0;
}

CLI_CMD("input_test", "input function test", input_test_main);
/* end of file  input_test.c */