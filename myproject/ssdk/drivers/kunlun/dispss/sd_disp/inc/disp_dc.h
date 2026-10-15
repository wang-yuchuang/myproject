/*
* disp_dc.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 06/28/2019 BI create this file
*/
#ifndef __DISP_DC_H__
#define __DISP_DC_H__

/* register update TYPEs */
#define DC_NO_UPDATE              0x0000
#define DC_GP_UPDATE              0x0001
#define DC_SP_UPDATE              0x0002
#define DC_DP0_UPDATE             0x0004
#define DC_DP1_UPDATE             0x0004
#define DC_TCON_UPDATE            0x0008
#define DC_MLC_UPDATE             0x0010
#define DC_GAMMA_UPDATE           0x0020
#define DC_DITHER_UPDATE          0x0040
#define DC_CSC_UPDATE             0x0080
#define DC_CRC_UPDATE             0x0800

enum {
    CLEAR_SPIPE_LAYER = 0b01,
    CLEAR_GPIPE_LAYER = 0b10,
    CLEAR_ALL_LAYERS =  0b11
};

typedef struct dc_profile_t
{
  unsigned long   base_addr;
  unsigned int irq;
  unsigned int frame_w;
  unsigned int frame_h;
  unsigned int layer_n;
} dc_profile_t;

typedef enum {
    MLC_LAYER_SP,
    MLC_LAYER_GP,
    MLC_LAYER_DP0,
    MLC_LAYER_DP1,
    MLC_LAYER_NUM
} mlc_layer_index;

typedef enum {
    MLC_PATH_BG,
    MLC_PATH_SP,
    MLC_PATH_GP,
    MLC_PATH_DP0,
    MLC_PATH_DP1,
    MLC_PATH_NUM
} mlc_path_index;

struct tcon_timing {
    unsigned int hact;
    unsigned int htol;
    unsigned int hsbp;
    unsigned int hsync;

    unsigned int vact;
    unsigned int vtol;
    unsigned int vsbp;
    unsigned int vsync;
};
#endif
