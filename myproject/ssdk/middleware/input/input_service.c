/**
 * @file input_service.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <types.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include <cmsis_os2.h>
#include <CLI.h>

#include <rpc/dcf_msg.h>
#include <rpc/dcf_rpc.h>

#ifdef CONFIG_TOUCH_KUNLUN_PLATFORM
#include <driver/kunlun/touch/touch.h>
#endif /* end CONFIG_TOUCH_KUNLUN_PLATFORM */

#ifdef CONFIG_KUNLUN_MBOX
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#endif /* end CONFIG_KUNLUN_MBOX */

#include <driver/mbox.h>

#include <touch_cfg.h>
#include <mbox_cfg.h>
#include <sd_property.h>
#include <board.h>
#include "input_service.h"
/* =========================== marco define session ======================== */
#define TS_MUTEX_DELAY_TIME     10
#define TS_MESSAGE_QUE_SIZE     20

/** log macro define */
#define INPUT_DEBUG_LVL      SSDK_WARNING
#define EXTRA_FMT   "touch service: "

#if INPUT_DEBUG_LVL >= SSDK_ERR
#define INPUT_ERR(format, ...) \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define INPUT_ERR(format, ...)
#endif

#if INPUT_DEBUG_LVL >= SSDK_WARNING
#define INPUT_WARN(format, ...) \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define INPUT_WARN(format, ...)
#endif

#if INPUT_DEBUG_LVL >= SSDK_INFO
#define INPUT_INFO(format, ...) \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define INPUT_INFO(format, ...)
#endif

#if INPUT_DEBUG_LVL >= SSDK_INFO
#define INPUT_DEBUG(format, ...) \
    ssdk_printf(SSDK_DEBUG, EXTRA_FMT format, ##__VA_ARGS__)
#else
#define INPUT_DEBUG(format, ...)
#endif
/* =========================== enumeration define session ================== */
typedef struct ts_report_container ts_report_container_t;
/** ts container report ts data function type */
typedef int32_t (*noticer)(ts_report_container_t* repCon);

/* =========================== structure define session ==================== */
/** report touch data container, include touch device/ts_domain and other
 *  relate infomation
 */
struct ts_report_container {
    touch_dev_t* dev;
    int32_t screen_id;
    int32_t ts_domain;

    struct xy_range_info* xy_region_ptr;
    ts_touch_report_data_t report_data;

    noticer notice_fun;                             /**< notice to remote */
    ts_callback cb;                                 /**< notice to local */
    ts_report_config_t* ts_cfg;
    void* usr_ctx;

    bool active;
};

/** touch service instance structure */
typedef struct ts_service_ins {
    ts_report_container_t rep_con[TS_DO_MAX];
    osThreadId_t     thd;
    osMutexId_t      mutex;
    osMessageQueueId_t msg_que;
    ts_report_config_t* rep_cfg_ptr;
    int32_t rep_cfg_size;
    bool is_runing;
}ts_service_ins_t;

/** touch service touch data msg structure */
typedef struct {
    ts_report_container_t* rep_con;
    void* data;
} ts_touch_msg_t;

/** touch service io control msg structure */
typedef struct {
    void* data;
} ts_ioctl_msg_t;

/** touch service register callback msg structure */
typedef struct {
    int32_t ts_domain;
    ts_callback cb;
    void* arg;
} ts_register_msg_t;

/** touch service msg structure */
typedef struct {
    int32_t msg_type;
    union {
        ts_touch_msg_t touch_msg;           /**< touch callback msg */
        ts_ioctl_msg_t ioctl_msg;           /**< ioctl callback msg */
        ts_register_msg_t reg_msg;          /**< register local noticer msg */
    };
}ts_msg_t;

/** mbox chan req config */
typedef struct {
    int32_t addr;
    int32_t rproc;
} ts_mboxChan_reqCfg_t;

typedef struct {
    mbox_chan_t* mbox_chan;
    ts_mboxChan_reqCfg_t req_cfg;
} ts_mbox_chan_t;
/* =========================== function declare session ==================== */
// int32_t ts_register_notice(int32_t tsDomain, ts_callback cb, void* usrCtx);
static int32_t local_domain_noticer(ts_report_container_t* repCon);
static int32_t remote_domain_android_main_noticer(ts_report_container_t* repCon);
static int32_t remote_domain_android_aux1_noticer(ts_report_container_t* repCon);
static int32_t remote_domain_android_aux2_noticer(ts_report_container_t* repCon);
static int32_t remote_domain_android_aux3_noticer(ts_report_container_t* repCon);
static int32_t remote_domain_ap2_main_noticer(ts_report_container_t* repCon);
static int32_t remote_domain_ap2_aux1_noticer(ts_report_container_t* repCon);

/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
static ts_service_ins_t s_ts_service_ins = {
    .is_runing = false,
};

static noticer s_domain_noticer_tbl[] = {
    [TS_DO_ANRDOID_MAIN] =
        remote_domain_android_main_noticer,
    [TS_DO_ANRDOID_AUX1] =
        remote_domain_android_aux1_noticer,
    [TS_DO_ANRDOID_AUX2] =
        remote_domain_android_aux2_noticer,
    [TS_DO_ANRDOID_AUX3] =
        remote_domain_android_aux3_noticer,
    [TS_DO_AP2_MAIN] =
        remote_domain_ap2_main_noticer,
    [TS_DO_AP2_AUX1] =
        remote_domain_ap2_aux1_noticer,
    [TS_DO_CTRLPANEL_MAIN] =
        local_domain_noticer,
    [TS_DO_CTRLPANEL_AUX1] =
        local_domain_noticer,
    [TS_DO_FAST_AVM] =
        local_domain_noticer
};

static ts_mbox_chan_t s_domain_chan_tbl[] = {
    [TS_DO_ANRDOID_MAIN] =
        {.mbox_chan = NULL,     {.addr = TS_ANRDOID_MAIN_MBOX_ADDR, .rproc = MBOX_RPROC_AP1}},
    [TS_DO_ANRDOID_AUX1] =
        {.mbox_chan = NULL,     {.addr = TS_ANRDOID_AUX1_MBOX_ADDR, .rproc = MBOX_RPROC_AP1}},
    [TS_DO_ANRDOID_AUX2] =
        {.mbox_chan = NULL,     {.addr = TS_ANRDOID_AUX2_MBOX_ADDR, .rproc = MBOX_RPROC_AP1}},
    [TS_DO_ANRDOID_AUX3] =
        {.mbox_chan = NULL,     {.addr = TS_ANRDOID_AUX3_MBOX_ADDR, .rproc = MBOX_RPROC_AP1}},
    [TS_DO_AP2_MAIN] =
        {.mbox_chan = NULL,     {.addr = TS_ANRDOID_MAIN_MBOX_ADDR, .rproc = MBOX_RPROC_AP2}},
    [TS_DO_AP2_AUX1] =
        {.mbox_chan = NULL,     {.addr = TS_ANRDOID_AUX1_MBOX_ADDR, .rproc = MBOX_RPROC_AP2}},
};
/* =========================== function implementation session ============= */

static int32_t ts_send_msg(ts_msg_t* msgPtr)
{
    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;

    osMessageQueuePut(ts_ins_ptr->msg_que, msgPtr, 0, osWaitForever);
    return 0;
}

static bool is_ts_runing()
{
    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;

    return ts_ins_ptr->is_runing;
}

#ifdef CONFIG_INPUT_PRINT_REPORT_POINT
static void print_tsdomain_touch_point(ts_report_container_t* repCon)
{
    if (repCon->report_data.touch_num < 0) {
        printf("current tsdomain: %d no touch point\n", repCon->ts_domain);
        return;
    }
    else {
        printf("report data for screen: %d to tsDomain: %d\n", repCon->screen_id, repCon->ts_domain);
        for (int32_t i = 0; i < repCon->report_data.touch_num; i++) {
            printf("point %d: (%d, %d)\n"
                    , i + 1
                    , repCon->report_data.coord_data[i].x
                    , repCon->report_data.coord_data[i].y);
        }
    }
}
#else
#define print_tsdomain_touch_point(x)
#endif /* end CONFIG_INPUT_PRINT_REPORT_POINT */

static int32_t remote_domain_chan_request(int32_t domain)
{
    ts_mbox_chan_t* ts_mchan = NULL;

    if (domain >= ARRAY_SIZE(s_domain_chan_tbl)) {
        ssdk_printf(SSDK_EMERG, "invalid domain %d for request channel.\n", domain);
        return -1;
    }

    ts_mchan = &s_domain_chan_tbl[domain];
    sdrv_mbox_chan_req_t chan_req;
    chan_req.rproc = ts_mchan->req_cfg.rproc;
    chan_req.dest_addr = ts_mchan->req_cfg.addr;
    chan_req.src_addr = ts_mchan->req_cfg.addr;

    ts_mchan->mbox_chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (ts_mchan->mbox_chan == NULL) {
        ssdk_printf(SSDK_EMERG, "request mbox channel error for touch service domain: %d.\n", domain);
        return -1;
    }

    return 0;
}

static int32_t remote_domain_chan_release()
{
    ts_mbox_chan_t* ts_mchan = NULL;

    for (int32_t cnt = 0; cnt < ARRAY_SIZE(s_domain_chan_tbl); cnt++) {
        ts_mchan = &s_domain_chan_tbl[cnt];

        if (ts_mchan->mbox_chan != NULL) {
            mbox_release_channel(ts_mchan->mbox_chan);
        }
    }

    return 0;
}

static int32_t remote_domain_chan_send_data(ts_report_container_t* repCon)
{
    int32_t ts_domain = repCon->ts_domain;
    mbox_chan_t* chan = s_domain_chan_tbl[ts_domain].mbox_chan;

    // invoke callback, for local test sensor data
    if (repCon->cb != NULL) {
        repCon->cb(repCon->ts_domain, &repCon->report_data, repCon->usr_ctx);
    }

    int32_t len = 2 + repCon->report_data.touch_num * TS_COORD_METADATA_SIZE;
    return mbox_send(chan, (uint8_t *)(&repCon->report_data), len, 10);
}

static int32_t local_domain_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    if (repCon == NULL) {
        return -1;
    }

    // invoke callback
    if (repCon->cb != NULL) {
        return repCon->cb(repCon->ts_domain, &repCon->report_data, repCon->usr_ctx);
    }

    return -1;
}

static int32_t remote_domain_android_main_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    print_tsdomain_touch_point(repCon);
    remote_domain_chan_send_data(repCon);
    return 0;
}

static int32_t remote_domain_android_aux1_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    print_tsdomain_touch_point(repCon);
    remote_domain_chan_send_data(repCon);
    //@TODO use rpmsg notify touch data
    return 0;
}

static int32_t remote_domain_android_aux2_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    print_tsdomain_touch_point(repCon);
    remote_domain_chan_send_data(repCon);
    //@TODO use rpmsg notify touch data
    return 0;
}

static int32_t remote_domain_android_aux3_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    print_tsdomain_touch_point(repCon);
    remote_domain_chan_send_data(repCon);
    //@TODO use rpmsg notify touch data
    return 0;
}

static int32_t remote_domain_ap2_main_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    print_tsdomain_touch_point(repCon);
    remote_domain_chan_send_data(repCon);
    return 0;
}

static int32_t remote_domain_ap2_aux1_noticer(ts_report_container_t* repCon)
{
    INPUT_DEBUG("%s entry\n", __func__);
    print_tsdomain_touch_point(repCon);
    remote_domain_chan_send_data(repCon);
    return 0;
}

bool avm_cast_enable_flag = false;
uint32_t press_interval = 0 ,time_start = 0;
ts_coord_point_t coord_data_start;
#define PRESS_INTERVAL_TIME 500
#define COORD_START_X 1850
#define COORD_PRESS_X_OFFSET 250

void set_avm_cast_enable(bool enabled){
    avm_cast_enable_flag = enabled;
}


void avm_cast_check_back_event(ts_msg_t msg){
    ts_coord_info_t* coord_data = msg.touch_msg.data;
    if(time_start == 0){
        time_start = current_time();
        coord_data_start = coord_data->point[0];
    }else{
        press_interval = current_time() - time_start;
        printf("press_interval :%d,coord_data_start.x:%d,data->coord_data[0].x:%d\n",
                        press_interval,coord_data_start.x,coord_data->point[0].x);
        if(press_interval > PRESS_INTERVAL_TIME){
            coord_data_start = coord_data->point[0];
            time_start = current_time();
        }else{
            if(coord_data_start.x >= COORD_START_X){
                if(coord_data_start.x  - coord_data->point[0].x > COORD_PRESS_X_OFFSET){
                    system_property_set(DMP_ID_AVM_CAST_STATUS,0);
                    printf( "DMP_ID_AVM_SDCAST_STATUS 0 --------------\n");
                    time_start = current_time();
                }
            }
        }
    }
}

static int32_t touch_data_cb_process_coord(ts_coord_info_t* coordInfo, void* usrCtx)
{
    if (!is_ts_runing()) {
        return -1;
    }

    ts_msg_t msg;
    memset(&msg, 0x00, sizeof(msg));

    msg.msg_type = TS_MSG_COORD_UPDATE;
    msg.touch_msg.rep_con = (ts_report_container_t *)usrCtx;

    INPUT_DEBUG("%s process, ts_domain: %d, active sta: %d,screen_id:%d\n", __func__, msg.touch_msg.rep_con->ts_domain, msg.touch_msg.rep_con->active, msg.touch_msg.rep_con->screen_id);
    // current tsdomain is inactive, skip it
    if (!msg.touch_msg.rep_con->active) {
        return 0;
    }

    ts_coord_info_t* coord_data = malloc(sizeof(ts_coord_info_t));
    if (coord_data == NULL) {
        INPUT_ERR("malloc memory error while process touch coord msg\n");
        return -1;
    }

    memcpy(coord_data, coordInfo, sizeof(ts_coord_info_t));
    msg.touch_msg.data = coord_data;

    if(avm_cast_enable_flag == true){
        if(msg.touch_msg.rep_con->ts_domain == TS_DO_ANRDOID_MAIN && msg.touch_msg.rep_con->screen_id == 1)
        {
            avm_cast_check_back_event(msg);
            return 0;
        }
    }

    ts_send_msg(&msg);
    return 0;
}

static int32_t touch_data_callback(int32_t scId, touch_notice_t* notice, void* usrCtx)
{
    if (notice == NULL || usrCtx == NULL) {
        INPUT_ERR("invalid parameter for %s\n", __func__);
        return -1;
    }

    ts_report_container_t* rep_con_ptr = (ts_report_container_t *)usrCtx;

    //@NOTE log code only for debug
    static int32_t active_flg = 0;
    if (!rep_con_ptr->active) {
        if (active_flg == 0) {
            ssdk_printf(SSDK_EMERG,"[input]%s:%d, can not dispatch to ts domain because of inactive.\n", __func__, __LINE__);
            active_flg = 1;
        }
        return 0;
    }

    // check scId
    //@NOTE log code only for degbug
    static int32_t screen_flg = 0;
    if (rep_con_ptr->screen_id != scId) {
        if (screen_flg == 0) {
            ssdk_printf(SSDK_EMERG,"[input]%s:%d, screenId: %d, scId: %d, addr: %p, can not dispatch to ts domain because of scId err\n", __func__, __LINE__, rep_con_ptr->screen_id, scId, rep_con_ptr);
            screen_flg = 1;
        }
        return -1;
    }

    switch (notice->evt_type) {
        case TS_NOTICE_EVT_COORD:
            touch_data_cb_process_coord(&notice->coord, usrCtx);
            break;
        default:
            break;
    }

    return 0;
}

static int32_t ts_trans_coordInfo2ReportData(const ts_coord_info_t* coordInfo, ts_report_container_t* repCon)
{
    if (coordInfo->touch_num < 0) {
        // do not have invalid touch point coord info
        return -1;
    }

    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;
    ts_touch_report_data_t* rep_data_ptr = &repCon->report_data;
    int32_t x_offset = repCon->xy_region_ptr->x_offset;
    int32_t y_offset = repCon->xy_region_ptr->y_offset;
    int32_t x_max = x_offset + repCon->xy_region_ptr->x_max;
    int32_t y_max = y_offset + repCon->xy_region_ptr->y_max;

    INPUT_DEBUG("xoff: %d, yoff: %d, xmax: %d, ymax: %d\n", x_offset, y_offset, x_max, y_max);
    osMutexAcquire(ts_ins_ptr->mutex, osWaitForever);
    //@TODO key status is not support in touch
    if (coordInfo->have_key) {
        rep_data_ptr->key_value = coordInfo->have_key;
    }
    rep_data_ptr->touch_num = 0;
    int32_t tmp_number = 0;
    for (int32_t cnt = 0; cnt < coordInfo->touch_num; cnt++) {
        if (coordInfo->point[cnt].x >= x_offset &&
            coordInfo->point[cnt].y >= y_offset &&
            coordInfo->point[cnt].x < x_max &&
            coordInfo->point[cnt].y < y_max) {
            rep_data_ptr->coord_data[tmp_number].id = coordInfo->point[cnt].id;
            rep_data_ptr->coord_data[tmp_number].x = coordInfo->point[cnt].x - x_offset;
            rep_data_ptr->coord_data[tmp_number].y = coordInfo->point[cnt].y - y_offset;
            rep_data_ptr->coord_data[tmp_number].w = coordInfo->point[cnt].w;
            tmp_number++;
        }
        else {
            // current touch point is not in current domain region
        }
    }
    rep_data_ptr->touch_num = tmp_number;
    osMutexRelease(ts_ins_ptr->mutex);
    return 0;
}

static int32_t ts_work_process_msg_coord_update(ts_touch_msg_t* touchMsg)
{
    if (touchMsg == NULL ||
        touchMsg->data == NULL ||
        touchMsg->rep_con == NULL) {
        INPUT_ERR("parameter error for %s\n", __func__);
        return -1;
    }
    ts_coord_info_t* coord_info_ptr = (ts_coord_info_t *)touchMsg->data;

    // trans coord data format
    ts_trans_coordInfo2ReportData(coord_info_ptr, touchMsg->rep_con);

    // report touch data for remote or local apps
    if (touchMsg->rep_con->notice_fun != NULL) {
        touchMsg->rep_con->notice_fun(touchMsg->rep_con);
    }

    // free coord data buffer
    free(coord_info_ptr);

    return 0;
}

static int32_t ts_work_process_msg_register_cb(ts_register_msg_t* regMsg)
{
    if (regMsg->ts_domain >= TS_DO_MAX) {
        INPUT_ERR("ts_domain %d is outrange\n", regMsg->ts_domain);
        return -1;
    }

    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;

    ts_ins_ptr->rep_con[regMsg->ts_domain].cb = regMsg->cb;
    ts_ins_ptr->rep_con[regMsg->ts_domain].usr_ctx = regMsg->arg;

    return 0;
}

static int32_t ts_work_process_msg_ioctl(ts_ioctl_msg_t* ioctlMsg)
{

    return 0;
}

static int32_t ts_check_msg(int32_t msgType, ts_report_config_t* repCfg, int32_t size, int32_t* domainsArry)
{
    ASSERT(domainsArry);
    int32_t cnt = 0;

    for (int32_t i = 0; i < size; i++) {
        if (repCfg[i].arbitrate & msgType) {
            domainsArry[cnt++] = repCfg[i].ts_domain;
        }
    }

    return cnt;
}

static bool is_touch_region_overlap(struct xy_range_info* region1, struct xy_range_info* region2)
{
    if (region1->x_offset + region1->x_max > region2->x_offset &&
        region2->x_offset + region2->x_max > region1->x_offset &&
        region1->y_offset + region1->y_max > region2->y_offset &&
        region2->y_offset + region2->y_max > region1->y_offset) {
        return true;
    }
    else {
        return false;
    }
}

static int32_t ts_work_process_msg_aribitration(int32_t ariMsg)
{
    INPUT_DEBUG(" %s entry \n", __func__);
    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;
    int32_t domains[TS_DO_MAX];
    int32_t ret = 0;

    ret = ts_check_msg(ariMsg, ts_ins_ptr->rep_cfg_ptr, ts_ins_ptr->rep_cfg_size, domains);
    if (!ret) {
        INPUT_ERR("current msg: 0x%x can not find in config tbl\n", ariMsg);
        return -1;
    }

    for (int32_t i = 0; i < ret; i++) {
        int32_t ts_domain = domains[i];
        int32_t screen_id = ts_ins_ptr->rep_con[ts_domain].screen_id;
        ts_report_container_t* cur_con_item = &ts_ins_ptr->rep_con[ts_domain];

        INPUT_INFO(" %s process, ts_domain: %d \n", __func__, ts_domain);

        for (int32_t cnt = 0; cnt < TS_DO_MAX; cnt++) {
            if (screen_id == ts_ins_ptr->rep_con[cnt].screen_id &&
                is_touch_region_overlap(cur_con_item->xy_region_ptr
                                        , ts_ins_ptr->rep_con[cnt].xy_region_ptr) &&
                (ariMsg & ts_ins_ptr->rep_con[cnt].ts_cfg->arbitrate) == 0) {

                INPUT_INFO(" %s process, inactive: %d \n", __func__, cnt);
                // inactive
                ts_ins_ptr->rep_con[cnt].active = false;
                // clear touch data
                ts_ins_ptr->rep_con[cnt].report_data.touch_num = 0;
            }
        }

        // active ts_domain related to the msg
        ts_ins_ptr->rep_con[ts_domain].active = true;
    }

    return 0;
}

static void ts_work_fun(void* arg)
{
    ts_service_ins_t* ts_ins_ptr = (ts_service_ins_t *)arg;
    int32_t ret = 0;
    ts_msg_t msg;
    uint32_t arbitrate_msg = 0;

    ts_ins_ptr->is_runing = true;

    INPUT_INFO("input workfunc: %s entry\n", __func__);
    while (ts_ins_ptr->is_runing ||
            osMessageQueueGetCount(ts_ins_ptr->msg_que) != 0) {
        ret = osMessageQueueGet(ts_ins_ptr->msg_que, &msg, NULL, osWaitForever);

        if (ret != osOK) {
            INPUT_ERR("get message error, retval is: %d\n", ret);
            continue;
        }

        switch (msg.msg_type) {
            case TS_MSG_COORD_UPDATE:
                ts_work_process_msg_coord_update(&msg.touch_msg);
                break;
            case TS_MSG_REGISTER_CB:
                ts_work_process_msg_register_cb(&msg.reg_msg);
                break;
            case TS_MSG_TS_IOCTL:
                ts_work_process_msg_ioctl(&msg.ioctl_msg);
                break;
            case TS_MSG_TS_EXIT:
                ts_ins_ptr->is_runing = false;
                break;
            default:
                if (msg.msg_type & TS_MSG_EXT_FLG) {
                    arbitrate_msg = msg.msg_type & (TS_MSG_EXT_FLG - 1);
                    ts_work_process_msg_aribitration(arbitrate_msg);

                }
                else {
                    // do nothing
                }
                break;
        }
    }

    osThreadExit();

    // release system resource
    osMessageQueueDelete(ts_ins_ptr->msg_que);
    osMutexDelete(ts_ins_ptr->mutex);

    // release channel
    remote_domain_chan_release();
    return;
}

static int safe_ts_get_remote_domain(uint16_t instance)
{
    ts_domain_type_enum_t ts_domain;

    if (instance & TS_SUPPORT_ANRDOID_MAIN)
        ts_domain = TS_DO_ANRDOID_MAIN;
    else if (instance & TS_SUPPORT_ANRDOID_AUX1)
        ts_domain = TS_DO_ANRDOID_AUX1;
    else if (instance & TS_SUPPORT_ANRDOID_AUX2)
        ts_domain = TS_DO_ANRDOID_AUX2;
    else if (instance & TS_SUPPORT_ANRDOID_AUX3)
        ts_domain = TS_DO_ANRDOID_AUX3;
    else if (instance & TS_SUPPORT_AP2_MAIN)
        ts_domain = TS_DO_AP2_MAIN;
    else if (instance & TS_SUPPORT_AP2_AUX1)
        ts_domain = TS_DO_AP2_AUX1;
    else if (instance & TS_SUPPORT_CTRLPANEL_MAIN)
        ts_domain = TS_DO_CTRLPANEL_MAIN;
    else if (instance & TS_SUPPORT_CTRLPANEL_AUX1)
        ts_domain = TS_DO_CTRLPANEL_AUX1;
    else
        ts_domain = TS_DO_MAX;

    return ts_domain;
}

static rpc_call_result_t safe_ts_ioctl_cb(rpc_server_handle_t hserver,
        rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct sts_ioctl_cmd *ctl = (struct sts_ioctl_cmd *) &request->param[0];
    struct sts_ioctl_result *r = (struct sts_ioctl_result *) &result.result[0];
    touch_dev_t* dev;
    int ts_remote_domain;
    result.ack = request->cmd;

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        ssdk_printf(SSDK_EMERG, "%s:instance=%#x, request err\n", __func__, ctl->instance);
        return result;
    }

    ts_remote_domain = safe_ts_get_remote_domain(ctl->instance);

    if (ts_remote_domain >= TS_DO_MAX) {
        result.retcode = ERR_INVALID_ARGS;
        ssdk_printf(SSDK_EMERG, "%s:instance=%#x, instance err\n", __func__, ctl->instance);
        return result;
    }

    dev = s_ts_service_ins.rep_con[ts_remote_domain].dev;
    struct xy_range_info* xy_range_ptr = s_ts_service_ins.rep_con[ts_remote_domain].xy_region_ptr;

    if (!dev) {
        result.retcode = ERR_NOT_IMPLEMENTED;
        //dprintf(ALWAYS, "%s:instance=%#x, dev err\n", __func__, ctl->instance);
        return result;
    }

    tsCtrl_vendorInfo_t vendor_info;
    switch (ctl->op) {
        case STS_OP_GET_CONFIG:
            memset(&vendor_info, 0x00, sizeof(vendor_info));
            touch_device_control(dev, TS_CONTROL_GET_VENDOR_INFO, &vendor_info, NULL);

            r->msg.cfg.version = vendor_info.version;
            r->msg.cfg.id = vendor_info.id;
            r->msg.cfg.vendor = vendor_info.vendor;

            r->msg.cfg.abs_x_max = xy_range_ptr->x_max;
            r->msg.cfg.abs_y_max = xy_range_ptr->y_max;

            result.retcode = 0;
            INPUT_INFO("input ioctl get config\n");
            break;

        case STS_OP_SET_INITED:
            // if (!dev->set_inited) {
            //     result.retcode = ERR_NOT_CONFIGURED;
            //     dprintf(ALWAYS, "%s:instance=%#x, set inited none\n", __func__, ctl->instance);
            // }
            // else {
            //     mutex_acquire(&dev->dev_lock);

            //     if (!dev->inited_flag) {
            //         dev->set_inited(dev->vendor_priv);
            //         dev->inited_flag = true;
            //         result.retcode = 0;
            //     }
            //     else {
            //         result.retcode = 0;
            //     }

            //     mutex_release(&dev->dev_lock);
            // }
            result.retcode = 0;
            break;

        default:
            result.retcode = ERR_INVALID_ARGS;
            ssdk_printf(SSDK_EMERG, "%s:instance=%#x, op err\n", __func__, ctl->instance);
            break;
    }

    ssdk_printf(SSDK_EMERG, "%s:instance=%#x, cmd=%d done\n", __func__, ctl->instance, ctl->op);
    return result;
}

static void touch_service_property_cb(int pid, int old_val, int new_val, void* param)
{
    if (DMP_ID_TOUCH_STATUS != pid) return;
    if (old_val == new_val) return;

    switch (new_val) {
        case 0:
            touch_service_send_msg(TS_MSG_TRANS_TO_AP1);
            break;
        case 1:
            touch_service_send_msg(TS_MSG_TRANS_TO_AP2);
            break;
        case 2:
            touch_service_send_msg(TS_MSG_SHARE);
            break;
        default:
            break;
    }

    return;
}

int32_t touch_service_start(ts_report_config_t* repCfg, int32_t tblSize)
{
    if (repCfg == NULL || tblSize <= 0) {
        INPUT_ERR("parameter error for %s\n", __func__);
        return -1;
    }
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_STS_IOCTL, safe_ts_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };


    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;
    touch_dev_t** touch_arry = g_touch_dev;

    if (ts_ins_ptr->is_runing) {
        return 0;
    }

    // register property callback
    system_property_observe(DMP_ID_TOUCH_STATUS, touch_service_property_cb, NULL);

    // start ipcc rpc service
    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));
    memset(ts_ins_ptr->rep_con, 0x00, sizeof(ts_ins_ptr->rep_con));

    // init system resource
    // ts_ins_ptr->event = osEventFlagsNew(NULL);
    ts_ins_ptr->msg_que = osMessageQueueNew(TS_MESSAGE_QUE_SIZE, sizeof(ts_msg_t), NULL);
    ts_ins_ptr->mutex = osMutexNew(NULL);


    // init for report container
    for (int32_t cnt = 0; cnt < tblSize; cnt++) {
        int32_t ts_domain = repCfg[cnt].ts_domain;
        int32_t screen_id = repCfg[cnt].screen_id;

        if (ts_domain >= TS_DO_MAX ||
            screen_id >= SCREEN_MAX) {
            INPUT_ERR("invalid config item for input, ts_domain: %d, screen id: %d, skip it\n", ts_domain, screen_id);
            continue;
        }
        // request mbox channel
        remote_domain_chan_request(ts_domain);

        // container init
        ts_ins_ptr->rep_con[ts_domain].dev = touch_arry[screen_id];
        ts_ins_ptr->rep_con[ts_domain].screen_id = screen_id;
        ts_ins_ptr->rep_con[ts_domain].ts_domain = ts_domain;
        ts_ins_ptr->rep_con[ts_domain].ts_cfg = &repCfg[cnt];
        ts_ins_ptr->rep_con[ts_domain].xy_region_ptr = &repCfg[cnt].xy_region;
        ts_ins_ptr->rep_con[ts_domain].notice_fun = s_domain_noticer_tbl[ts_domain];
        ts_ins_ptr->rep_con[ts_domain].cb = NULL;
        ts_ins_ptr->rep_con[ts_domain].usr_ctx = NULL;
        // register callback
        touch_register_callback(touch_arry[screen_id], touch_data_callback, &ts_ins_ptr->rep_con[ts_domain]);

        ssdk_printf(SSDK_EMERG, "[input]init for touch service, register for screen id: %d, ts domain: %d , ptr: %p\n", screen_id, ts_domain, &ts_ins_ptr->rep_con[ts_domain]);
        // active container item
        if (repCfg[cnt].arbitrate & TS_REPORT_DO_DEFAULT_FLG) {
            ts_ins_ptr->rep_con[ts_domain].active = true;
        }

        INPUT_DEBUG("region: 0x%x , xoff: %d, for ts_domain: %d, active sta: %d\n"
                , (addr_t)&repCfg[cnt].xy_region
                , repCfg[cnt].xy_region.x_offset
                , ts_domain
                , ts_ins_ptr->rep_con[ts_domain].active);
    }

    ts_ins_ptr->rep_cfg_ptr = repCfg;
    ts_ins_ptr->rep_cfg_size = tblSize;

    // start touch service task
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "input";
    attr.priority = osPriorityAboveNormal;
    attr.stack_size = 2048;
    ts_ins_ptr->thd = osThreadNew(ts_work_fun, ts_ins_ptr, &attr);

    return 0;
}

int32_t touch_service_stop()
{
    if (!is_ts_runing()) {
        return -1;
    }

    ts_msg_t msg;
    memset(&msg, 0x00, sizeof(msg));

    msg.msg_type = TS_MSG_TS_EXIT;

    ts_send_msg(&msg);

    return 0;
}

int32_t touch_service_register_domain_callback(ts_domain_type_enum_t tsDomain, ts_callback cb, void* usrCtx)
{
    if (!is_ts_runing()) {
        return -1;
    }

    if (cb == NULL) {
        INPUT_ERR("parameter error for %s\n", __func__);
        return -1;
    }

    ts_msg_t msg;
    memset(&msg, 0x00, sizeof(msg));

    msg.msg_type = TS_MSG_REGISTER_CB;
    msg.reg_msg.ts_domain = tsDomain;
    msg.reg_msg.cb = cb;
    msg.reg_msg.arg = usrCtx;

    ts_send_msg(&msg);

    return 0;
}

int32_t touch_service_send_msg(int32_t msgType)
{
    if (!is_ts_runing()) {
        return -1;
    }

    if (msgType & TS_MSG_EXT_FLG) {
        ts_msg_t msg;
        memset(&msg, 0x00, sizeof(msg));

        msg.msg_type = msgType;

        ts_send_msg(&msg);

        return 0;
    }
    else {
        INPUT_ERR("current msg type: 0x%x is not extern msg, can not put with current inferface\n", msgType);
        return -1;
    }
}

bool touch_service_ts_domain_is_pressed(ts_domain_type_enum_t tsDomain)
{
    if (!is_ts_runing() ||
        tsDomain >= TS_DO_MAX ||
        tsDomain < 0) {
        return false;
    }

    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;
    bool is_pressed = false;

    osMutexAcquire(ts_ins_ptr->mutex, osWaitForever);
    if (ts_ins_ptr->rep_con[tsDomain].report_data.touch_num > 0) {
        is_pressed = true;
    }
    else {
        is_pressed = false;
    }
    osMutexRelease(ts_ins_ptr->mutex);

    return is_pressed;
}

int32_t touch_service_ts_domain_get_point(int32_t* x, int32_t* y, ts_domain_type_enum_t tsDomain)
{
    if (!is_ts_runing() ||
        tsDomain >= TS_DO_MAX ||
        tsDomain < 0) {
        return -1;
    }

    if (x == NULL ||
        y == NULL) {
        return -1;
    }

    ts_service_ins_t* ts_ins_ptr = &s_ts_service_ins;
    osMutexAcquire(ts_ins_ptr->mutex, osWaitForever);
    *x = ts_ins_ptr->rep_con[tsDomain].report_data.coord_data[0].x;
    *y = ts_ins_ptr->rep_con[tsDomain].report_data.coord_data[0].y;
    osMutexRelease(ts_ins_ptr->mutex);

    return 0;
}

#if CONFIG_INPUT_SERVICE_DUMMY_DATA

struct tsdomain_trans {
    char* ts_domain_str;
    ts_report_container_t* ts_domain_addr;
};

/**
 * @brief send dummy touch data
 *
 * @param argc cmd arg number is 4
 * @param argv[0] tsdoamin
 * @param argv[1] touch point number;
 *                <=0 x point will auto plus 100 unit
 *                >0 & <=10: report argv[1] numbers touch point, x, y auto plus
 *                >10 invlaid, result same as =1
 * @param argv[2] x pos
 * @param argv[3] y pos
 * @return int32_t
 */
int32_t send_touch_data(int32_t argc, char* argv[])
{
    struct tsdomain_trans trans_tbl[] = {
        {"anmain",      &s_ts_service_ins.rep_con[TS_DO_ANRDOID_MAIN]},
        {"anaux1",      &s_ts_service_ins.rep_con[TS_DO_ANRDOID_AUX1]},
        {"anaux2",      &s_ts_service_ins.rep_con[TS_DO_ANRDOID_AUX2]},
        {"anaux3",      &s_ts_service_ins.rep_con[TS_DO_ANRDOID_AUX3]},
        {"ap2main",     &s_ts_service_ins.rep_con[TS_DO_AP2_MAIN]},
        {"ap2aux1",     &s_ts_service_ins.rep_con[TS_DO_AP2_AUX1]},
        {"cpmain",      &s_ts_service_ins.rep_con[TS_DO_CTRLPANEL_MAIN]},
        {"cpaux1",      &s_ts_service_ins.rep_con[TS_DO_CTRLPANEL_AUX1]},
        {"fastavm",     &s_ts_service_ins.rep_con[TS_DO_FAST_AVM]},
        {NULL,     NULL},
    };

    ts_coord_info_t coor_data;
    memset(&coor_data, 0x00, sizeof(coor_data));

    // confirm ts domain
    ts_report_container_t* rep_con_ptr = NULL;
    int32_t cnt = 0;
    while (trans_tbl[cnt].ts_domain_str != NULL) {
        if (strcmp(trans_tbl[cnt].ts_domain_str, argv[0]) == 0) {
            rep_con_ptr = trans_tbl[cnt].ts_domain_addr;
            break;
        }
        cnt++;
    }
    if (rep_con_ptr == NULL) {
        printf("invalid ts domain %s, valid list is: \n", argv[0]);
        cnt = 0;
        while (trans_tbl[cnt].ts_domain_str != NULL) {
            printf("-- %s\n", trans_tbl[cnt++].ts_domain_str);
        }
        return -1;
    }

    int32_t x = atoi(argv[2]);
    int32_t y = atoi(argv[3]);

    // confirm touch number
    int32_t touch_num = atoi(argv[1]);
    if (touch_num <= 0) {
        coor_data.touch_num = 1;
        for (int32_t i = 0; i < 10; i++) {
            coor_data.point[0].x = x++;
            coor_data.point[0].y = y;
            printf("send touch point: (%d, %d) to %s domain\n", x - 1, y, argv[0]);
            touch_data_cb_process_coord(&coor_data, rep_con_ptr);
            // osDelay(1);
        }
    }
    else if (touch_num == 1 || touch_num > 10) {
        coor_data.touch_num = 1;
        coor_data.point[0].x = x;
        coor_data.point[0].y = y;
        printf("send touch point: (%d, %d) to %s domain\n", x, y, argv[0]);
        touch_data_cb_process_coord(&coor_data, rep_con_ptr);
    }
    else {
        coor_data.touch_num = touch_num;
        for (int32_t i = 0; i < touch_num; i++) {
            coor_data.point[i].x = x++;
            coor_data.point[i].y = y++;
        }
        printf("send touch point: (%d, %d) to %s domain\n", x , y, argv[0]);
        touch_data_cb_process_coord(&coor_data, rep_con_ptr);
    }

    return 0;
}

static void input_test_usage(void)
{
    printf("\ninput commands:\n");
    printf("\tinput devices ;list device info\n");
    printf("\tinput send tsdoamin touch_num x y ;input service dummy data\n");
}

int32_t input_main(int32_t argc, char* argv[])
{
    if (argc == 0) {
        printf("input test: invalid param.\n");
        input_test_usage();
        return CLI_FALSE;
    }

    if (strcmp(argv[0], "devices") == 0) {
        // list device info
    }
    else if (strcmp(argv[0], "send") == 0 && argc == 5) {
        // send touch point to ts domain
        send_touch_data(argc - 1, &argv[1]);
    }
    else {
        // show help
        printf("input test: invalid param.\n");
        input_test_usage();
    }
    return CLI_FALSE;
}

CLI_CMD("input", "input service dummy data", input_main);
#endif /* end  */

/* end of file input_service.c */
