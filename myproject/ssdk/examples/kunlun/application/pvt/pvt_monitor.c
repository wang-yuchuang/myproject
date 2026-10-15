/*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
*/
/* =========================== include file session ======================== */
#include <debug.h>
#include <string.h>
#include <cmsis_os2.h>
#include <stdlib.h>
#include <lib/list.h>
#include <CLI.h>

#include <driver/kunlun/pvt/pvt.h>
#include <driver/kunlun/pvt/sdrv_pvt_drv.h>
#include <pvt_cfg.h>

#include "pvt_app.h"
/* =========================== marco define session ======================== */
#define LOCAL_TRACE 1 //close local trace 1->0

#define PVT_MONITOR_WAIT_TIME CONFIG_PVT_MONITOR_WAIT_TIME // seconds
#define PVT_MONITOR_WAIT_TIME_VALUE (PVT_MONITOR_WAIT_TIME*1000)

// #define PVT_ALARM_TYPE_0 0
// #define PVT_ALARM_TYPE_1 1

// #define PVT_ALARM_INT_ENABLE 1
// #define PVT_ALARM_INT_DISABLE 0

// #define PVT_ALARM_INT_REGISTER 1
// #define PVT_ALARM_INT_UNREGISTER 0


// #define PVT_MON_INS0_ALARM      0x1
// #define PVT_MON_INS1_ALARM      0x2
#define PVT_MON_INS_NUM 2

/** pvt monitor type */
#define PVT_MON_TYPE_HYST_HIGH  0
#define PVT_MON_TYPE_HYST_LOW   1

#define PVT_MON_RELEASE_DELTA   3
/* =========================== enumeration define session ================== */
/** pvt monitor message enum */
typedef enum {
    PVT_MON_MSG_INS0_ALARM,
    PVT_MON_MSG_INS1_ALARM,
    PVT_MON_MSG_REG_CB,
    PVT_MON_MSG_DEINIT,

    PVT_MON_MSG_TMO,
}pvt_mon_msg_enum_t;

/* =========================== structure define session ==================== */
/** pvt monitor callback function list structure */
typedef struct {
    struct list_node node;
    cb_t cb;
    void* args;
}pvt_mon_cb_t;

/** pvt monitor config structure */
typedef struct {
    int32_t monitor_type;
    float temp_valh;
    float temp_vall;
}pvt_monitor_cfg_t;

/** pvt monitor instance structure */
typedef struct {
    const int32_t ins;
    const int32_t msg;
    int32_t chan_id;
    bool alarm_sta;
    struct list_node cb_list;
    pvt_monitor_cfg_t pvt_mon_cfg;
}pvt_monitor_t;

/** pvt monitor msg structure */
typedef struct {
    int32_t ins;
    cb_t cb;
    void* args;
}msg_reg_cb_t;
typedef struct {
    int32_t msg_type;
    union {
        msg_reg_cb_t reg_cb;
    };
}pvt_mon_msg_t;

/** pvt monitor commandline */
typedef void (*cmd_entry)(int32_t argc, char* argv[]);
typedef struct subCmd_mgr {
    const char* sub_cmd;
    int32_t par_num;
    cmd_entry entry;
}subCmd_mgr_t;

/* =========================== function declare session ==================== */
/* =========================== static value define session ================= */
/** pvt monitor instance */
static pvt_monitor_t s_pvt_monitor[2] = {
    {
        .ins = 0,
        .msg = PVT_MON_MSG_INS0_ALARM,
        .chan_id = -1,
        .alarm_sta = false,
        .pvt_mon_cfg = {
            .monitor_type = CONFIG_PVT_MONITOR_TYPE_0,
            .temp_valh = CONFIG_PVT_MONITOR_VAL_H_0,
            .temp_vall = CONFIG_PVT_MONITOR_VAL_L_0,
        }
    },
    {
        .ins = 1,
        .msg = PVT_MON_MSG_INS1_ALARM,
        .chan_id = -1,
        .alarm_sta = false,
        .pvt_mon_cfg = {
            .monitor_type = CONFIG_PVT_MONITOR_TYPE_1,
            .temp_valh = CONFIG_PVT_MONITOR_VAL_H_1,
            .temp_vall = CONFIG_PVT_MONITOR_VAL_L_1,
        }
    }
};

/** pvt monitor message queue */
static osMessageQueueId_t s_pvt_mon_msgque = NULL;
/** pvt monitor device pointer */
static pvt_dev_t* s_pvt_dev = NULL;
/** pvt monitor init flag */
static bool s_is_init = false;
/* =========================== function implementation session ============= */
int32_t send_msg2que(const pvt_mon_msg_t* msg)
{
    return osMessageQueuePut(s_pvt_mon_msgque, msg, 0, 0);
}

void pvt_monitor_handle(uint16_t chanId, void* args)
{
    pvt_monitor_t* pvt_mon_ins = (pvt_monitor_t *)args;

    pvt_mon_msg_t msg;
    msg.msg_type = pvt_mon_ins->msg;
    send_msg2que(&msg);

    return;
}

int32_t alarm_msg_notify(pvt_monitor_t* ins, int32_t type)
{
    pvt_mon_cb_t* cbs = NULL;
    list_for_every_entry(&ins->cb_list, cbs, pvt_mon_cb_t, node) {
        if (cbs->cb != NULL) {
            cbs->cb(ins->ins, type, cbs->args);
        }
    }

    return 0;
}

static void monitor_alarm_handle(int32_t monIns, int32_t type, void* args)
{
    ssdk_printf(SSDK_EMERG, "%s: monitor ins: %d(%s) notice alarm %s\n"
                            , __func__
                            , monIns
                            , (char *)args
                            , type == PVT_MON_ALARM_SET ? "happen" : "release");
}

int32_t process_pvt_mon_timeout_msg()
{
    if (s_pvt_dev == NULL) {
        ssdk_printf(SSDK_ERR, "invalid device for temperature monitor\n");
        return -1;
    }

    float temp = 0;
    if (pvt_float_data_get(s_pvt_dev, PVT_GET_FLOAT_CUR_TEMP, -1, &temp) == -1) {
        ssdk_printf(SSDK_ERR, "pvt get temprature data error for dev: %d\n", CONFIG_PVT_MONITOR_DEV);
        return -1;
    }

    bool clr_flg = false;

    for (int32_t i = 0; i < PVT_MON_INS_NUM; i++) {
        pvt_monitor_t* mon_ins = &s_pvt_monitor[i];
        clr_flg = false;

        switch (mon_ins->pvt_mon_cfg.monitor_type) {
            // hyst high
            case PVT_MON_TYPE_HYST_HIGH:
                if (temp < ((float)mon_ins->pvt_mon_cfg.temp_vall - PVT_MON_RELEASE_DELTA)) {
                    clr_flg = true;
                }
                break;
            // hyst low
            case PVT_MON_TYPE_HYST_LOW:
                if (temp > ((float)mon_ins->pvt_mon_cfg.temp_valh + PVT_MON_RELEASE_DELTA)) {
                    clr_flg = true;
                }
                break;
            default:
                break;
        }

        if (clr_flg && mon_ins->alarm_sta) {
            mon_ins->alarm_sta = false;
            // invoke clear notify
            alarm_msg_notify(mon_ins, 1);

            // enable channel
            pvt_enable_alarm(s_pvt_dev, mon_ins->chan_id);
        }
    }

    return 0;
}

int32_t process_pvt_mon_alarm_msg(int32_t alarmEvt)
{
    int32_t ins = 0;

    if (alarmEvt == PVT_MON_MSG_INS0_ALARM) {
        ins = 0;
    }
    else if (alarmEvt == PVT_MON_MSG_INS1_ALARM) {
        ins = 1;
    }
    else {
        ssdk_printf(SSDK_ERR, "invalid monitor ins for %s.", __func__);
        return -1;
    }

    pvt_monitor_t* mon_ins = &s_pvt_monitor[ins];

    mon_ins->alarm_sta = true;
    // invoke alarm notify
    alarm_msg_notify(mon_ins, 0);

    return 0;
}

int32_t process_pvt_mon_reg_callback_msg(msg_reg_cb_t* cbMsg)
{
    if (cbMsg->ins >= PVT_MON_INS_NUM) {
        ssdk_printf(SSDK_ERR, "invalid monitor instance: %d for %s\n", cbMsg->ins, __func__);
        return -1;
    }

    // register callback
    pvt_monitor_t* mon_ins = &s_pvt_monitor[cbMsg->ins];
    pvt_mon_cb_t* cb_node = malloc(sizeof(pvt_mon_cb_t));
    if (cb_node == NULL) {
        ssdk_printf(SSDK_ALERT, "malloc memory in %s\n", __func__);
        return -1;
    }

    cb_node->cb = cbMsg->cb;
    cb_node->args = cbMsg->args;
    list_add_tail(&mon_ins->cb_list, &cb_node->node);

    return 0;
}

void pvt_monitor_work_fun(void* args)
{
    uint32_t ret = 0;
    pvt_mon_msg_t msg;
    bool exit = false;

    s_is_init = true;
    while (!exit) {
        ret = osMessageQueueGet(s_pvt_mon_msgque, &msg, NULL, PVT_MONITOR_WAIT_TIME_VALUE);
        if (ret == osErrorTimeout) {
            msg.msg_type = PVT_MON_MSG_TMO;
        }

        switch (msg.msg_type) {
            case PVT_MON_MSG_TMO:
                process_pvt_mon_timeout_msg();
                break;
            case PVT_MON_MSG_INS0_ALARM:
            case PVT_MON_MSG_INS1_ALARM:
                process_pvt_mon_alarm_msg(msg.msg_type);
                break;
            case PVT_MON_MSG_REG_CB:
                process_pvt_mon_reg_callback_msg(&msg.reg_cb);
                break;
            case PVT_MON_MSG_DEINIT:
                exit = true;
                break;
            default:
                break;
        }
    }

    // clr channel & release list
    for (int32_t i = 0; i < PVT_MON_INS_NUM; i++) {
        pvt_monitor_t* mon_ins = &s_pvt_monitor[i];

        pvt_clr_alarm(s_pvt_dev, mon_ins->chan_id);
        mon_ins->chan_id = -1;

        pvt_mon_cb_t* cbs = NULL;
        list_for_every_entry(&mon_ins->cb_list, cbs, pvt_mon_cb_t, node) {
            if (cbs != NULL) {
                free(cbs);
            }
        }
    }

    // release system resource
    osMessageQueueDelete(s_pvt_mon_msgque);
    s_is_init = false;
    osThreadExit();
    return ;
}

int32_t pvt_monitor_init()
{
    if (s_is_init) {
        ssdk_printf(SSDK_EMERG, "pvt monitor is already init.\n");
        return -1;
    }
    // system resource init
    s_pvt_mon_msgque = osMessageQueueNew(10, sizeof(pvt_mon_msg_t), NULL);

    // get pvt monitor device
    s_pvt_dev = g_pvt_dev_ptr[CONFIG_PVT_MONITOR_DEV];

    // list init && chan request
    for (int32_t i = 0; i < PVT_MON_INS_NUM; i++) {
        pvt_monitor_t* mon_ins = &s_pvt_monitor[i];

        list_initialize(&mon_ins->cb_list);

        pvt_alarm_para_t para;
        para.cb_fun = pvt_monitor_handle;
        para.args = mon_ins;
        para.high_val = mon_ins->pvt_mon_cfg.temp_valh;
        para.low_val = mon_ins->pvt_mon_cfg.temp_vall;

        // set alarm will auto enable chann
        mon_ins->chan_id = pvt_set_alarm(s_pvt_dev
                                        , PVT_ALARM_MODE_TEMP
                                        , mon_ins->pvt_mon_cfg.monitor_type
                                        , &para);
    }

    // register noticer
    char* args = NULL;
    for (int32_t i = 0; i < PVT_MON_INS_NUM; i++) {
        args = (i == 0 ? "ins0" : "ins1");
        pvt_monitor_register_noticer(i, monitor_alarm_handle, args);
    }

    // create task
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "pvt_mon";
    attr.stack_size = 1024;
    attr.priority = osPriorityNormal;
    osThreadNew(pvt_monitor_work_fun, NULL, &attr);

    return 0;
}

int32_t pvt_monitor_deinit()
{
    if (!s_is_init) {
        ssdk_printf(SSDK_EMERG, "pvt monitor is already deinit.\n");
        return -1;
    }
    pvt_mon_msg_t msg;
    msg.msg_type = PVT_MON_MSG_DEINIT;

    send_msg2que(&msg);

    return 0;
}

int32_t pvt_monitor_register_noticer(int32_t monIns, cb_t cb, void* args)
{
    pvt_mon_msg_t msg;
    msg.msg_type = PVT_MON_MSG_REG_CB;

    msg.reg_cb.ins = monIns;
    msg.reg_cb.cb = cb;
    msg.reg_cb.args = args;

    send_msg2que(&msg);

    return 0;
}


#ifdef CONFIG_CLI
/**
 * @brief
 *
 * @param argc
 * @param argv
 */
static void pvt_mon_cmd_show_sta(int32_t argc, char* argv[])
{
    pvt_monitor_t* mon_ins = NULL;

    for (int32_t i = 0; i < PVT_MON_INS_NUM; i++) {
        mon_ins = &s_pvt_monitor[i];

        ssdk_printf(SSDK_EMERG, "pvt monitor ins: %d info:\n", i);
        ssdk_printf(SSDK_EMERG, " -- alarm type: %s\n"
                                , (mon_ins->pvt_mon_cfg.monitor_type == PVT_MON_TYPE_HYST_LOW)
                                ? "hyst low"
                                : "hyst high");
        ssdk_printf(SSDK_EMERG, " -- alrm threshold hvalue: %5.2f; lvalue: %5.2f\n"
                                , mon_ins->pvt_mon_cfg.temp_valh
                                , mon_ins->pvt_mon_cfg.temp_vall);
        ssdk_printf(SSDK_EMERG, " -- alrm status: %s\n", mon_ins->alarm_sta ? "true" : "false");
    }

    return ;
}

static void pvt_mon_cmd_register_noticer(int32_t argc, char* argv[])
{
    int32_t ins_id = atoi(argv[0]);
    char* args = NULL;

    if (ins_id >= PVT_MON_INS_NUM || ins_id < 0) {
        ssdk_printf(SSDK_EMERG, "invalid monitor instance id: %d \n", ins_id);
        return;
    }

    args = (ins_id == 0 ? "ins0" : "ins1");

    pvt_monitor_register_noticer(ins_id, monitor_alarm_handle, args);

    ssdk_printf(SSDK_EMERG, "register noticer for monitor %s\n", args);
    return;
}

static void pvt_mon_cmd_start(int32_t argc, char* argv[])
{
    ssdk_printf(SSDK_EMERG, "start pvt monitor\n");
    pvt_monitor_init();
}

static void pvt_mon_cmd_stop(int32_t argc, char* argv[])
{
    ssdk_printf(SSDK_EMERG, "stop pvt monitor\n");
    pvt_monitor_deinit();
}

/**
 * @brief config
 *
 * @param argc 3
 * @param argv[0] monitor ins index
 * @param argv[1] monitor ins[x] config hsyt high value
 * @param argv[2] monitor ins[x] config hsyt low value
 */
static void pvt_mon_cmd_config(int32_t argc, char* argv[])
{
    int32_t ins_id = atoi(argv[0]);
    int32_t hyst_h = atoi(argv[1]);
    int32_t hyst_l = atoi(argv[2]);
    if (ins_id >= PVT_MON_INS_NUM || ins_id < 0) {
        ssdk_printf(SSDK_EMERG, "invalid ins id: %d \n", ins_id);
        return;
    }

    pvt_monitor_cfg_t* mon_ins_cfg = &s_pvt_monitor[ins_id].pvt_mon_cfg;
    mon_ins_cfg->temp_valh = hyst_h;
    mon_ins_cfg->temp_vall = hyst_l;
    ssdk_printf(SSDK_EMERG, "ins%d config hyst_h temp: %d, hyst_l temp: %d\n",ins_id, hyst_h, hyst_l);

}

/* sub command manager table */
static subCmd_mgr_t s_subCmd_mgr_tbl[] = {
    {"show_sta" , 0, pvt_mon_cmd_show_sta},
    {"reg_cb"   , 1, pvt_mon_cmd_register_noticer},
    {"config"   , 3, pvt_mon_cmd_config},
    {"start"    , 0, pvt_mon_cmd_start},
    {"stop"     , 0, pvt_mon_cmd_stop},
    {NULL}
};

static void subCmd_help_msg()
{
    printf("start -> start pvt monitor\n");
    printf("stop  -> stop pvt monitor\n");
    printf("config [ins id] [temp high] [temp low] -> config pvt monitor ins temp value\n");
    printf("show_sta  -> show pvt monitor instance config info and monitor status\n");
    printf("reg_cb [mon_ins]  -> register callback for specified monitor instance, valid {mon_ins} is 0/1\n");
}

static int32_t pvt_mon_main(int32_t argc, char* argv[])
{
    int32_t validpar_index = 1;

    // parameter check
    if (argc == 0 ||
        strncmp(argv[0], "?", strlen("?")) == 0 ||
        strncmp(argv[0], "help", strlen("help")) == 0 ||
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
                printf("parameter number invalid for sub cmd: %s\n", s_subCmd_mgr_tbl[i].sub_cmd);
            }
            break;
        }
        else {
            // continue check subcmd
        }
    }

    if (i == -1 ||
        s_subCmd_mgr_tbl[i].sub_cmd == NULL) {
        printf("do not support sub cmd: %s for pvt_mon\n", argv[0]);
        return 0;
    }

    return 0;
}

CLI_CMD("pvt_mon", "pvt function test", pvt_mon_main);

#endif /* end CONFIG_CLI */
/* end of file  */
