#ifndef __BACKLIGHT_H__
#define __BACKLIGHT_H__

#include "pwm_cfg.h"
#include <cmsis_os2.h>

enum backlight_op_type {
    BL_OP_GET_BRIGHT,
    BL_OP_SET_BRIGHT,
};

struct backlight_device {
    bool binitialized;
    int screenid;
    pwm_instance_t* ins;
    int pwm_ch;
    int duty;
    osMutexId_t bl_lock;
};

struct bl_ioctl_cmd {
    u32 op;
    union {
        struct {
            u8 screenid;
            u8 brightness;
        } ctl;
        u8 data[8];
    }u;
};

struct bl_ioctl_result {
    u32 op;
    union {
        struct {
            u8 screenid;
            u8 brightness;
        } ctl;
        u8 data[8];
    }u;
};

void backlight_service_init();
int backlight_duty_set(int screenid, int duty);
int backlight_duty_get(int screenid);

#endif