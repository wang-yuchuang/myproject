/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include "driver/kunlun/pwm/pwm_drv.h"
#include <backlight.h>
#include <disp_cfg.h>
#include "rpc/dcf_rpc.h"
#include <sdm_display.h>

#define SVC_BACKLIGHT_LOG 0

static struct backlight_device backlight_dev[] = {
    {//screen 1
        false,
        0,
        NULL, // RES_PWM_PWM5,
        DRV_PWM_CHN_A,
        BACKLIGHT_PWM_DUTY,
    },
    { //screen 2
        false,
        1,
        NULL, // RES_PWM_PWM3,
        DRV_PWM_CHN_A,
        BACKLIGHT_PWM_DUTY,
    },
    {//screen 3
        false,
        2,
        NULL, // RES_PWM_PWM3,
        DRV_PWM_CHN_B,
        BACKLIGHT_PWM_DUTY,
    },
    {//screen 4
        false,
        3,
#if (defined(TARGET_REFERENCE_X9U_A) || defined(TARGET_REFERENCE_X9U_B))
        &g_pwm5, // RES_PWM_PWM5,
        DRV_PWM_CHN_B,
#else
        &g_pwm4, // RES_PWM_PWM4,
        DRV_PWM_CHN_D,
#endif
        BACKLIGHT_PWM_DUTY,
    },
    {//screen 5
        false,
        4,
#if (defined(TARGET_REFERENCE_X9U_A) || defined(TARGET_REFERENCE_X9U_B))
        NULL, // RES_PWM_PWM4,
        DRV_PWM_CHN_D,
#else
        NULL, // RES_PWM_PWM5,
        DRV_PWM_CHN_B,
#endif
        BACKLIGHT_PWM_DUTY,
    },
};

static int pwm_duty_set(struct backlight_device *bl, u8 duty)
{

    if(duty > BACKLIGHT_PWM_MAX_DUTY || duty < 0) {
        ssdk_printf(0, "err: backlight pwm duty set value inval! input duty = %d \n", duty);
        return 0;
    }

    drv_pwm_simple_duty_set(bl->ins->pwmc, bl->pwm_ch, &(bl->ins->pwm_simple_ctx), duty);
    bl->duty = duty;
    ssdk_printf(SVC_BACKLIGHT_LOG, "backlight pwm bl->screenid[%d] duty set: %d !\n", bl->screenid, duty);

    return 1;
}

static rpc_call_result_t backlight_ioctl_cb(rpc_server_handle_t hserver, rpc_call_request_t *request)
{
    rpc_call_result_t result = {0,};
    struct bl_ioctl_cmd *ctl = (struct bl_ioctl_cmd *) &request->param[0];
    struct bl_ioctl_result *r = (struct bl_ioctl_result *) &result.result[0];
    struct backlight_device *bl = &backlight_dev[ctl->u.ctl.screenid];

    if (!bl->binitialized)
        return result;

    result.ack = request->cmd;
    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    osMutexAcquire(bl->bl_lock, 0);
    switch(ctl->op) {
    case BL_OP_SET_BRIGHT:
        pwm_duty_set(bl, ctl->u.ctl.brightness);
        backlight_dev[ctl->u.ctl.screenid].duty = ctl->u.ctl.brightness;
        break;
    case BL_OP_GET_BRIGHT:
        r->u.ctl.screenid = bl->screenid;
        r->u.ctl.brightness = backlight_dev[ctl->u.ctl.screenid].duty;
        ssdk_printf(0, "%s() screen%d bl_brightness ioctl_get %d \n",
            __func__, r->u.ctl.screenid, r->u.ctl.brightness);
        break;
    default:
        break;
    }
    result.retcode = 0;
    osMutexRelease(bl->bl_lock);
    ssdk_printf(SVC_BACKLIGHT_LOG, "%s op=%d\n", __func__, ctl->op);

    return result;
}

static void backlight_rpc_server_init(void)
{
    static int rpc_server_initialized = 0;
    static rpc_server_impl_t s_blfuncs[] = {
        {MOD_RPC_REQ_BL_IOCTL, backlight_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };

    if (rpc_server_initialized)
        return;

    start_ipcc_rpc_service(s_blfuncs, ARRAY_SIZE(s_blfuncs));
    rpc_server_initialized = 1;
}

int backlight_duty_set(int screenid, int duty)
{
    int ret = -1;
    if (screenid < NOT_FOUND || screenid > SCREEN_MAX) {
        ssdk_printf(0, "%s() error: screen%d input is not a valid value!\n",
            __func__, screenid);
        return ret;
    }

    struct backlight_device *bl = &backlight_dev[screenid];

    if (!bl->binitialized) {
        printf("bl->screenid = %d is Not initialized, bl->binitialized = %d \n",
            bl->screenid, bl->binitialized);
        return ret;
    }

    if (pwm_duty_set(bl, duty))
        ret = 0;

    backlight_dev[screenid].duty = duty;

    return ret;
}

int backlight_duty_get(int screenid)
{
    int ret = -1;
    if (screenid < NOT_FOUND || screenid > SCREEN_MAX) {
        ssdk_printf(0, "%s() error: screen%d input is not a valid value!\n",
            __func__, screenid);
        return ret;
    }

    struct backlight_device *bl = &backlight_dev[screenid];

    ret = bl->duty;

    return ret;
}

void backlight_service_init()
{
    display_handle *handle = NULL;

    for (int disp_id = 0; disp_id < SCREEN_MAX; disp_id++)
    {
        handle = hal_get_display_handle(disp_id);
        if (!handle) {
            ssdk_printf(0, "display:%d is invalid\n", disp_id);
            continue;
        }

        struct backlight_device *bl = &backlight_dev[disp_id];
        struct backlight_info *bl_info = handle->panels_info[handle->found_panel]->backlight;
        struct sdm_panel *panel = handle->panels_info[handle->found_panel]->panel;

        if (!bl_info) {
            ssdk_printf(0, "disp[%d] bl info isn't set\n", handle->display_id);
            continue;
        }

        if (panel->panel_backlight_init) {
            panel->panel_backlight_init(bl_info, handle->display_id);
        }

        bl->screenid = handle->display_id;
        bl->ins = bl_info->pwm_ins;
        bl->pwm_ch = bl_info->pwm_chn;
        bl->duty = bl_info->pwm_cfg->cmp_cfg[bl_info->pwm_chn].duty;

        if (bl->binitialized)
            return;

        bl->bl_lock = osMutexNew(NULL);

        osMutexAcquire(bl->bl_lock, 0);

        backlight_rpc_server_init();

        osMutexRelease(bl->bl_lock);

        bl->binitialized = true;
    }
}
