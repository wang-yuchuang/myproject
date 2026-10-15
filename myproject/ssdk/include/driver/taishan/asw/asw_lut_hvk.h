/*
* asw_lut_hvk.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 04/01/2021 BI create this file
*/

#ifndef __HVTK_H__
#define __HVTK_H__
#include <math.h>
#include <types.h>


struct hvkt_fabc {
    float fa0[3];
    float fa1[3];
    float fa2[3];

    float fb0[3];
    float fb1[3];
    float fb2[3];

    float fc0[3];
    float fc1[3];
    float fc2[3];
};

struct hvkt_param {
    int chn;
    bool fw_map;

    float top_left_x[3];
    float top_left_y[3];
    float top_right_x[3];
    float top_right_y[3];
    float bottom_left_x[3];
    float bottom_left_y[3];
    float bottom_right_x[3];
    float bottom_right_y[3];

    /*out size*/
    int hsize;
    int vsize;

    /*input size*/
    int hsize_in;
    int vsize_in;

    int lut_h_size;
    int lut_v_size;
    int lut_h_step;
    int lut_v_step;

    int extend;//lut start extend

    int lut_addr;
    int m_v_addr;
    int alpha_addr;

    int ext_lut_addr;

    struct hvkt_fabc * fabc;
    bool is_external_lut;
    /*only for bilinear*/
    /*0: 16bit 1-bit sign + 11-bit integer + 4-bit fraction */
    /*1: 32bit 13-bit sign + 11-bit integer + 8-bit fraction*/
    int external_lut_format;
    int lut_real_addr[6];//rx,ry,gx,gy,bx,by.

    //rotation
    int flip_type;
};

struct sdrv_lut_hvk_ops {
    int (*lut_table_gen)(struct hvkt_param *hvkt);
    int (*lut_m_v_gen)(struct hvkt_param *hvkt);
    int (*lut_rotate_table_gen)(double angle, struct hvkt_param *hvkt);
};

extern struct sdrv_lut_hvk_ops lut_hvk_ops;
#endif //__HVTK_H__
