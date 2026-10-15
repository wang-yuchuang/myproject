/**
 * @file fastavm_g2dlite.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv fastavm application.
 *
 * Revision History:
 * -----------------
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "fastavm"

#include <string.h>
#include <driver/kunlun/g2dlite/g2dlite.h>
#include <disp_data_type.h>
#include "fastavm_cfg.h"
#include "fastavm_def.h"
#include "fastavm.h"
#include "app_log.h"

#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif

#define MAX(a,b) ((signed)(a) > (signed)(b) ? (a) : (b))
#define MIN(a,b) ((signed)(a) < (signed)(b) ? (a) : (b))

static int fastavm_g2dlite_input_init(void* buf, uint32_t w, uint32_t h)
{
    if (buf) {
        struct g2dlite_input* input = (struct g2dlite_input*)buf;
        input->layer_num = 2;

        /* primary camera */
        input->layer[0].layer = 0;
        input->layer[0].layer_en = 1;
        input->layer[0].fmt = COLOR_YUYV;
        input->layer[0].zorder = 0;
        input->layer[0].src.x = 0;
        input->layer[0].src.y = MAX(0, (h-CAM_HEIGHT))/2;
        input->layer[0].src.w = MIN(CAM_WIDTH, (w-AVM_WIDTH));
        input->layer[0].src.h = MIN(h, CAM_HEIGHT);
        input->layer[0].src_stride[0] = CAM_WIDTH*2;
        input->layer[0].dst.x = AVM_WIDTH+MAX(0, (w-AVM_WIDTH-CAM_WIDTH))/2;
        input->layer[0].dst.y = MAX(0, (h-CAM_HEIGHT))/2;
        input->layer[0].dst.w = MIN(CAM_WIDTH, w-AVM_WIDTH);
        input->layer[0].dst.h = MIN(h, CAM_HEIGHT);
        input->layer[0].ckey.en = 0;
        input->layer[0].blend = BLEND_PIXEL_NONE;
        input->layer[0].alpha = 0xFF;

        APP_DBG("src[0]: x:%d, y:%d, w:%d, h:%d", \
                        input->layer[0].src.x, input->layer[0].src.y, \
                        input->layer[0].src.w, input->layer[0].src.h);
        APP_DBG("dst[0]: x:%d, y:%d, w:%d, h:%d", \
                        input->layer[0].dst.x, input->layer[0].dst.y, \
                        input->layer[0].dst.w, input->layer[0].dst.h);

        /* AVM */
        input->layer[1].layer = 1;
        input->layer[1].layer_en = 1;
        input->layer[1].fmt = COLOR_RGB888;
        input->layer[1].zorder = 1;
        input->layer[1].src.x = 0;
        input->layer[1].src.y = MAX(0, (h-AVM_HEIGHT))/2;
        input->layer[1].src.w = AVM_WIDTH;
        input->layer[1].src.h = MIN(h, AVM_HEIGHT);
        input->layer[1].src_stride[0] = AVM_WIDTH*3;
        input->layer[1].dst.x = MAX(0, (w-AVM_WIDTH-CAM_WIDTH))/2;
        input->layer[1].dst.y = MAX(0, (h-AVM_HEIGHT))/2;
        input->layer[1].dst.w = AVM_WIDTH;
        input->layer[1].dst.h = MIN(h, AVM_HEIGHT);
        input->layer[1].ckey.en = 0;
        input->layer[1].blend = BLEND_PIXEL_NONE;
        input->layer[1].alpha = 0xFF;

        APP_DBG("src[1]: x:%d, y:%d, w:%d, h:%d", \
                        input->layer[1].src.x, input->layer[1].src.y, \
                        input->layer[1].src.w, input->layer[1].src.h);
        APP_DBG("dst[1]: x:%d, y:%d, w:%d, h:%d", \
                        input->layer[1].dst.x, input->layer[1].dst.y, \
                        input->layer[1].dst.w, input->layer[1].dst.h);

        input->output.width = w;
        input->output.height = h;
        input->output.fmt = COLOR_RGB888;
        input->output.stride[0] = w*3;
        input->output.rotation = 0;
        return 0;
    }
    return -1;
}

fastavm_g2dlite_ops_t fastavm_g2dlite_ops = {
    .p_inputInit = fastavm_g2dlite_input_init,
};
/* EOF */
