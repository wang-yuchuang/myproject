/*
* driver/taishan/dispss/disp.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 11/12/2021 BILLY create this file
*/
#ifndef DISP_CTRL_H__
#define DISP_CTRL_H__
#include <types.h>
#include <cmsis_os2.h>
#include <driver/taishan/asw/asw_g_use.h>

typedef enum {
    PIPE_TYPE_SPIPE,
    PIPE_TYPE_GPIPE,
    PIPE_TYPE_NUM
} pipe_type_t;

typedef enum {
    MASTER_MODE = 0,
    SLAVE_MODE,
} ms_mode_t;

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

enum {
    CLEAR_SPIPE_LAYER = 1,
    CLEAR_GPIPE_LAYER,
    CLEAR_ALL_LAYERS
};

enum {
    IF_TYPE_INVALID = -1,
    IF_TYPE_NONE,
    IF_TYPE_DSI,
    IF_TYPE_LVDS,
    IF_TYPE_INTERNAL = IF_TYPE_LVDS,
    IF_TYPE_I2C,
    IF_TYPE_SPI,
    IF_TYPE_HDMI
};

enum {
    LVDS_MAP_FORMAT_NC = -1,
    LVDS_MAP_FORMAT_JEIDA,
    LVDS_MAP_FORMAT_SWPG
};

enum {
    CSI_DC_1_1 = 0,
    CSI_DC_1_2,
    CSI_DC_1_3,
    CSI_DC_1_4
};

typedef enum {
    LCM_POLARITY_RISING  = 0,
    LCM_POLARITY_FALLING = 1
} LCM_POLARITY;

enum {
    CRC32_MODE_INTERNAL = 0,
    CRC32_MODE_EXTERNAL = 1
};

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_t;

struct surface {
    int id;
    int dirty;
    int en;
    int fmt;
    rect_t src;
    unsigned long addr[4];//YUVA
    int src_stride[4];
    rect_t start;
    rect_t dst;
    int ckey_en;
    int ckey;
    int alpha_en;
    char alpha;
    int z_order;
    int security;
};

struct crc32block {
    unsigned int block_use;
    unsigned int cfg_start_x;
    unsigned int cfg_start_y;
    unsigned int cfg_end_x;
    unsigned int cfg_end_y;
    unsigned int expect_crc;
};

struct crc_info {
    int block_num;
    int mode;//0:internal 1:external
    struct crc32block *blocks;
};

struct post_cfg {
    char num_layers;
    struct surface *layers;
};

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

struct display_timing {
    unsigned int tcon_clk;
    unsigned int hactive;
    unsigned int hfront_porch;
    unsigned int hback_porch;
    unsigned int hsync_len;
    unsigned int vactive;
    unsigned int vfront_porch;
    unsigned int vback_porch;
    unsigned int vsync_len;
    float fps;

    /* polarity parameters */
    LCM_POLARITY dsp_clk_pol;
    LCM_POLARITY de_pol;
    LCM_POLARITY vsync_pol;
    LCM_POLARITY hsync_pol;

    unsigned int map_format;
};

struct dc_config {
    paddr_t base;
    int irq;
    struct crc_info *crc32;
};

struct dc_dev {
    paddr_t base;
    int irq;

    const struct dc_operations *ops;
    struct sdrv_panel *panel;
    int last_zorder[2];
    struct asw_app_dc app;
    struct crc_info *crc32;

    osSemaphoreId_t sem_dect_done;
    osSemaphoreId_t sem_vsync_delay_done;
    unsigned int vsync_wait;
};

struct dc_operations {
    int (*init)(struct dc_dev *dev);
    int (*reset)(struct dc_dev *dev);
    int (*update)(struct dc_dev *dev, struct post_cfg *post);
    int (*update_layer)(struct dc_dev *dev, struct surface *layer);
    void (*triggle)(struct dc_dev *dev);
    unsigned int (*check_triggle_status)(struct dc_dev *dev);
    void (*vsync_enable)(struct dc_dev *dev, bool enable);
    void (*enable)(struct dc_dev *dev);
    int (*irq_handler)(uint32_t irq, void *arg);
    void (*hsdk_enable)(struct dc_dev *dev, int mode, bool enable);
    int (*csi_mask_ratio_set)(struct dc_dev *dev, int mask_count, int ratio);
    int (*csi_timing_dectet_enable)(struct dc_dev *dev, bool enable);
    void (*set_tcon_after_detect)(struct dc_dev * dev, int ratio_num);
    void (*tcon_auto_adj_enable)(struct dc_dev *dev, bool enable);
    void (*tcon_csi_delay_set)(struct dc_dev *dev);
    void (*update_csi_detect)(struct dc_dev *dev, struct post_cfg *post,
        int mask_count, int ratio, bool enable);
    void (*set_bg_color)(struct dc_dev *dev, int color);
};

struct sdrv_panel {
    const char *panel_name;
    int if_type;
    int cmd_intf;
    int pixel_bpp;
    struct display_timing *timing;
};

extern struct dc_dev g_dc_dev;
extern struct dc_operations dc_ops;

int sdrv_paral_to_csi_en(bool enable);
int sdrv_disp_mux_init(struct sdrv_panel *panel);
int sdrv_crc_mode_set(bool is_internal);

int sdrv_adv_init(void);
int sdrv_lvds_init(struct sdrv_panel *panel);

void sdrv_panel_probe(    struct sdrv_panel *panel);
void sdrv_display_init(struct dc_dev *dev);
void sdrv_display_logo(struct dc_dev *dev,  unsigned long logo);
void sdrv_display_blank(struct dc_dev *dev);
int sdrv_display_get_width(struct dc_dev *dev);
int sdrv_display_get_height(struct dc_dev *dev);
void sdrv_display_set_bg_color(struct dc_dev *dev, int color);
void sdrv_display_post(struct dc_dev *dev, struct post_cfg *post);
void sdrv_dc_probe(struct dc_dev *dev, const struct dc_config *dc_cfg);
void sdrv_dc_handshake(struct dc_dev *dev,bool enable, int mode, struct post_cfg *post);
void sdrv_update_csi_detect(struct dc_dev *dev, struct post_cfg *post,
    int mask_count, int ratio, bool enable);

/*multi-master*/
/**
 * mm_display_test - one layer input YUV422P, one layer input ARGB8888
 * @y_addr: YUV422P y channel base address, size = 1280*720
 * @u_addr: YUV422P u channel base address, size = 1280*720/2
 * @v_addr: YUV422P v channel base address, size = 1280*720/2
 * @rgb_addr: ARGB8888 base address, size = 1280*720*4
 */
int mm_display_test(int y_addr, int u_addr, int v_addr, int rgb_addr);

/**
 * mm_gama_test - input YUV422P, output YUYV
 * @in_y_addr: YUV422P y channel base address, size = 1280*720
 * @in_u_addr: YUV422P u channel base address, size = 1280*720/2
 * @in_v_addr: YUV422P v channel base address, size = 1280*720/2
 * @out_addr: output buffer address, size = 1280*720*2
 */
int mm_gama_test(int in_y_addr, int in_u_addr, int in_v_addr, int out_addr);

#endif //DISP_CTRL_H__

