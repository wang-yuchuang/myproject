/*
 * g2dlite.h
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
#ifndef __G2DLITE_H__
#define __G2DLITE_H__
#include <cmsis_os2.h>
#include <disp_config.h>

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define G2DLITE_LAYER_MAX_NUM 2
/*LEN:"1"means one pair of 32-bit register addr + 32-bit register value*/
#define G2DLITE_CMDFILE_MAX_LEN 512
#define G2DLITE_CMDFILE_MAX_NUM 10

#define G2DLITE_CMDFILE_MAX_MEM                                                \
    (G2DLITE_CMDFILE_MAX_LEN * G2DLITE_CMDFILE_MAX_NUM * 2)

enum { BLEND_PIXEL_NONE = 0, BLEND_PIXEL_PREMULTI, BLEND_PIXEL_COVERAGE };

typedef enum {
    G2DLITE_TYPE_GPIPE,
    G2DLITE_TYPE_SPIPE,
    G2DLITE_TYPE_APIPE,
    G2DLITE_TYPE_NUM
} pipe_type_t;

typedef enum {
    ROTATION_TYPE_NONE = 0x0,
    ROTATION_TYPE_ROT_90 = 0x1,
    ROTATION_TYPE_HFLIP = 0x2,
    ROTATION_TYPE_VFLIP = 0x4,
    ROTATION_TYPE_ROT_180 = ROTATION_TYPE_VFLIP | ROTATION_TYPE_HFLIP,
    ROTATION_TYPE_ROT_270 =
        ROTATION_TYPE_ROT_90 | ROTATION_TYPE_VFLIP | ROTATION_TYPE_HFLIP,
    ROTATION_TYPE_VF_90 = ROTATION_TYPE_VFLIP | ROTATION_TYPE_ROT_90,
    ROTATION_TYPE_HF_90 = ROTATION_TYPE_HFLIP | ROTATION_TYPE_ROT_90,
} rotation_type;

typedef enum { PD_NONE = 0, PD_SRC = 0x1, PD_DST = 0x2 } PD_LAYER_TYPE;

typedef enum {
    CLEAR = 0,
    SRC,
    DST,
    SRC_OVER,
    DST_OVER,
    SRC_IN,
    DST_IN,
    SRC_OUT,
    DST_OUT,
    SRC_ATOP,
    DST_ATOP,
    XOR,
    DARKEN,
    LIGHTEN,
    MULTIPLY,
    SCREEN,
    ADD,
    OVERLAY,
    SRC_SUB,
    DES_SUB
} pd_mode_t;

struct rect_t {
    u32 x;
    u32 y;
    u32 w;
    u32 h;
};

struct ckey_range_t {
    u32 r_dn;
    u32 r_up;
    u32 g_dn;
    u32 g_up;
    u32 b_dn;
    u32 b_up;
};

struct ckey_t {
    u8 en;
    u32 alpha;
    struct ckey_range_t range;
};

struct rle_t {
    u32 en;
    u32 rle_y_len;
    u32 rle_y_checksum;
    u32 rle_data_size;
};

struct clut_t {
    u32 en;
    addr_t addr;
};

struct pd_t {
    u32 en;
    u32 zorder;
    pd_mode_t mode;
    u8 alpha_need;
};

struct fcopy_t {
    addr_t addr;
    u32 width;
    u32 height;
    u32 stride;
};

struct g2dlite_bg_cfg {
    u32 en;
    u32 color;
    u8 g_alpha;
    u8 zorder;

    addr_t aaddr;
    u8 bpa;
    u32 astride;
    PD_LAYER_TYPE pd_type;
};

struct g2dlite_input_cfg {
    u8 layer;
    u8 layer_en;
    u32 fmt;
    u32 zorder;
    struct rect_t src;
    u64 addr[4]; // YUVA
    u32 src_stride[4];
    struct rect_t dst;
    u32 blend;
    u8 alpha;
    struct ckey_t ckey;
    struct rle_t rle;
    struct clut_t clut;
    PD_LAYER_TYPE pd_type;
};

struct g2dlite_output_cfg {
    u32 width;
    u32 height;
    u32 fmt;
    u64 o_x;
    u64 o_y;
    u64 addr[4];
    u32 stride[4];
    u32 rotation;
};

struct g2dlite_input {
    u8 layer_num;
    struct pd_t pd_info;
    struct g2dlite_bg_cfg bg_layer;
    struct g2dlite_input_cfg layer[G2DLITE_LAYER_MAX_NUM];
    struct g2dlite_output_cfg output;
};

struct g2dlite_config {
    u32 base;
    u32 irq;
};
/**
 *struct cmdfile_info - command mode cmdfile info.
 *
 *arg: buffer for store 32-bit register and 32-bit register value
 *len: command file length(real data number-1),("1"means one pair
 *     of 32-bit register addr + 32-bit register value).
 */

typedef struct {
    unsigned char dirty;
    unsigned int len;
    unsigned int *arg;
} cmdfile_info;

struct g2dlite_device {
    unsigned long reg_base;
    cmdfile_info cmd_info[G2DLITE_CMDFILE_MAX_NUM];
    unsigned char write_mode;
    unsigned int irq;
    int (*irq_handler)(u32 irq, void *arg);
    osSemaphoreId_t done_sema;
    const struct g2dlite_ops *ops;
    unsigned int status;
};

struct g2dlite_ops {
    int (*init)(struct g2dlite_device *dev);
    int (*reset)(struct g2dlite_device *dev);
    int (*clut_index_set)(struct g2dlite_device *dev, char *clut_src);
    int (*update)(struct g2dlite_device *dev, struct g2dlite_input *ins);
    int (*rotation)(struct g2dlite_device *dev, struct g2dlite_input *ins);
    int (*fill_rect)(struct g2dlite_device *dev, struct g2dlite_bg_cfg *bgcfg,
                     struct g2dlite_output_cfg *output);
    int (*fastcopy)(struct g2dlite_device *dev, struct fcopy_t *in,
                    struct fcopy_t *out);
    int (*enable)(struct g2dlite_device *dev);
    int (*irq_handler)(u32 irq, void *arg);
};

extern const struct g2dlite_ops ops;
int sdrv_g2dlite_probe(struct g2dlite_device *dev,
                       const struct g2dlite_config *g2dlite_cfg);
int sdrv_g2dlite_update(struct g2dlite_device *dev,
                        struct g2dlite_input *input);
int sdrv_g2dlite_rotaion(struct g2dlite_device *dev,
                         struct g2dlite_input *input);
int sdrv_g2dlite_fill_rect(struct g2dlite_device *dev,
                           struct g2dlite_bg_cfg *bgcfg,
                           struct g2dlite_output_cfg *output);
int sdrv_g2dlite_fastcopy(struct g2dlite_device *dev, struct fcopy_t *in,
                          struct fcopy_t *out);
int sdrv_g2dlite_clut_setting(struct g2dlite_device *dev, char *clut_table);

#endif //__G2DLITE_H__
