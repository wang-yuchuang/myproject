/*
* asw_g_use.h
*
* Copyright (c) 2021 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 07/08/2022 BILLY create this file
*/

#ifndef __ASW_G_USE_H__
#define __ASW_G_USE_H__

//application id
enum ASW_APP_ID {
    ASW_APP_NONE = 0,
    ASW_APP_EMIRROR,
};


struct asw_app_dc {
    int id;

    int mode;//0:bilinear, 1: cubic spline.
    int buf_size_num_lines;
    int t_delta;
    int out_buf[2];
    int buf_index;
};

struct asw_app_asw {
    int id;

    int out_buf[2];//only for csi-asw handshake
    int out_index;
    int ext_lut_x[2];
    int ext_lut_y[2];
    int swap;
};


#endif
