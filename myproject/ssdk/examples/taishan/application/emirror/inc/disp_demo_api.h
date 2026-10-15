/*
* asw_demo.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/14/2021 BILLY create this file
*/
#ifndef __DISP_DEMO_API__
#define __DISP_DEMO_API__

#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/asw/asw.h>

struct csi_hsk_info {
    int src_ry_ba;
    int src_gu_ba;
    int src_bv_ba;

    int format;
    int stride_ry;
    int stride_gu;
    int stride_bv;
};


extern struct dc_dev g_dc_dev;
extern struct asw_dev aswdev;

int asw_case_csi_dc_hsdk(int mode, struct csi_hsk_info *info);
int dc_wait_csi_irq(struct dc_dev *dev);
int asw_case_csi_hsdk(int mode, struct csi_hsk_info *info);
int dc_csi_display(struct dc_dev *dev);
#endif
