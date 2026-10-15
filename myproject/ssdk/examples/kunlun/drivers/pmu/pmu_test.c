/**
 * @file pmu_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: test pmu
 *
 * Revision History:
 * -----------------
*/

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <CLI_type.h>
#include <debug.h>
#include <board.h>
#include <reg.h>

#include <pmu_cfg.h>
#include <driver/kunlun/pmu/pmu.h>

#define EXTRA_FMT   "pmu_test: "
#define TOSTRING(x) (#x)
#define TEST_PMU_LOG(format, ...) ssdk_printf(SSDK_EMERG, EXTRA_FMT format, ##__VA_ARGS__)


typedef void (*cmd_entry)(pmu_dev_t* dev, int32_t argc, char* argv[]);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;

void test_pmu_set_state(pmu_dev_t* dev, int32_t argc, char* argv[])
{
    int32_t cur_sta = 0;
    int32_t state = 0;

    state = atoi(argv[0]);

    pmu_get_status(dev, &cur_sta);
    TEST_PMU_LOG("current state is: %d befor pwrdown\n", cur_sta);

    pmu_change_state(dev, state);

    pmu_get_status(dev, &cur_sta);
    TEST_PMU_LOG("current state is: %d after pwrdown\n", cur_sta);
}

void test_pmu_get_status(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    int32_t cur_sta = 0;

    pmu_get_status(dev, &cur_sta);
    TEST_PMU_LOG("current state is: %d \n", cur_sta);
}

void test_pmu_get_event_source(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    uint32_t evt_src = 0;

    pmu_get_event_source(dev, &evt_src);
    TEST_PMU_LOG("current event src is: 0x%x\n", evt_src);
}

void test_pmu_clear_evt_source(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    uint32_t mutiEvt = 0;

    mutiEvt = atoi(argv[0]);

    test_pmu_get_event_source(dev, 0, NULL);

    pmu_clear_event_source(dev, mutiEvt);
    udelay(10);

    test_pmu_get_event_source(dev, 0, NULL);
}

void test_pmu_get_trigger_status(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    int32_t sta = 0;
    int32_t type = 0;

    type = atoi(argv[0]);

    pmu_get_triger_status(dev, type, &sta);
    TEST_PMU_LOG("trigger of %d type status is: %d \n", type, sta);
}

void test_pmu_set_pwrPin_out_status(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    int32_t pinId;
    int32_t lev;

    pinId = atoi(argv[0]);
    lev = atoi(argv[1]);

    pmu_set_pwr_out_status(dev, pinId, lev);
    TEST_PMU_LOG("set outpin: %d level is %s\n", pinId, lev == 0 ? "low" : "high");
}

void test_pmu_get_pwrPin_input_status(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    int32_t lev = 0;

    int32_t pinId;

    pinId = atoi(argv[0]);

    pmu_get_pwr_input_status(dev, pinId, &lev);
    TEST_PMU_LOG("get input pin: %d level is %s\n", pinId, lev == 0 ? "low" : "high");

}

void test_pmu_force_por_status(pmu_dev_t* dev, int32_t argc, char * argv[])
{
    int32_t sta = 0;

    sta = atoi(argv[0]);

    pmu_set_por_force_pwrdown(dev, sta);
    TEST_PMU_LOG("set por force pwrdown status: %s \n", sta == 0 ? "disable" : "enable");
}
/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"get_state",   0, test_pmu_get_status},
    {"set_state",   1, test_pmu_set_state},
    {"get_evtsrc",  0, test_pmu_get_event_source},
    {"clr_evtsrc",  1, test_pmu_clear_evt_source},
    {"get_triger",  1, test_pmu_get_trigger_status},
    {"set_ctlpin",  2, test_pmu_set_pwrPin_out_status},
    {"get_ctlpin",  1, test_pmu_get_pwrPin_input_status},
    {"set_porsta",  1, test_pmu_force_por_status},
    {NULL}
};

static void subCmd_help_msg()
{
    printf("usage: pmu_test [sub cmd] <cmd parameter ...>\n");
    printf("example:\n");
    printf("pmu_test get_state:\n\tget current pmu state \n");
    printf("pmu_test set_state 8:\n\tset pmu to powdown \n");
    printf("pmu_test get_evtsrc:\n\tget evt source\n");
    printf("pmu_test clr_evtsrc:\n\tclear evt source\n");
    printf("pmu_test get_triger:\n\tget current trigger status, include wakeup or reset\n");
    printf("pmu_test set_ctlpin:\n\tset outpin status\n");
    printf("pmu_test get_ctlpin:\n\tget inputpin status\n");
    printf("pmu_test set_porsta:\n\tset por state\n");
}

int32_t test_pmu_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;

    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "-h", strlen("-h")) == 0) {
            subCmd_help_msg();
            return 0;
    }

    board_pmu_init();
    pmu_dev_t* dev_ptr = g_pmu_dev_ptr;
    if (dev_ptr == NULL) {
        TEST_PMU_LOG("error for get pmu device\n");
        return -1;
    }

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
                TEST_PMU_LOG("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        TEST_PMU_LOG("do not support sub cmd: %s for pmu_test\n", argv[1]);
        return 0;
    }

    return 0;
}


CLI_CMD("pmu_test", "pmu function test", test_pmu_main);
/* end of file pmu_test.c */