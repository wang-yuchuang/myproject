/**
 * @file input_cfg.c
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
#ifdef CONFIG_INPUT_SERVICE
#include <types.h>
#include <stdint.h>


#include <input_service.h>
#include <driver/kunlun/touch/touch.h>

#include "input_cfg.h"
/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
ts_report_config_t g_ts_report_cfg_tbl[TS_REPORT_CFG_TBL_SIZE] = {
    /** DSI1 */
    {
        .screen_id = INFOTAINMENT,
        .ts_domain = TS_DO_ANRDOID_MAIN,
        .xy_region = {
            .x_offset = 0,
            .y_offset = 0,
            .x_max = 1920,
            .y_max = 720,
        },
        .arbitrate = TS_REPORT_DO_DEFAULT_FLG | TS_MSG_AVM_EXIT,
    },
    {
        .screen_id = INFOTAINMENT,
        .ts_domain = TS_DO_FAST_AVM,
        .xy_region = {
            .x_offset = 0,
            .y_offset = 0,
            .x_max = 1920,
            .y_max = 720,
        },
        .arbitrate = TS_MSG_AVM_START,
    },
    /** DSI2 */
    {
        .screen_id = ENTERTAINMENT,
        .ts_domain = TS_DO_ANRDOID_AUX1,
        .xy_region = {
            .x_offset = 0,
            .y_offset = 0,
            .x_max = 1920,
            .y_max = 720,
        },
        .arbitrate = TS_REPORT_DO_DEFAULT_FLG,
    },
    // /** test for screen division */
    // {
    //     .screen_id = ENTERTAINMENT,
    //     .ts_domain = TS_DO_ANRDOID_AUX2,
    //     .xy_region = {
    //         .x_offset = 960,
    //         .y_offset = 0,
    //         .x_max = 960,
    //         .y_max = 720,
    //     },
    //     .arbitrate = TS_REPORT_DO_DEFAULT_FLG,
    // },
    /** LVDS4 */
    {
        .screen_id = CONTROLPANEL,
        .ts_domain = TS_DO_CTRLPANEL_MAIN,
        .xy_region = {
            .x_offset = 0,
            .y_offset = 0,
            .x_max = 1920,
            .y_max = 720,
        },
        .arbitrate = TS_REPORT_DO_DEFAULT_FLG,
    }

    // /** test for screen combine */
    // {
    //     .screen_id = CLUSTER,
    //     .ts_domain = TS_DO_CTRLPANEL_AUX1,
    //     .xy_region = {
    //         .x_offset = -1920,
    //         .y_offset = 0,
    //         .x_max = 3840,
    //         .y_max = 720,
    //     },
    //     .arbitrate = TS_REPORT_DO_DEFAULT_FLG,
    // }
};
#endif /* end CONFIG_INPUT_SERVICE */
/* end of file input_cfg.c */