/**
 * @file sdm_panel.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __SDM_PANEL_H__
#define __SDM_PANEL_H__

#include <stdint.h>
#include <disp_common.h>
#include <disp_link_types.h>

#define MAX_DATA 100

enum {
    IF_TYPE_INVALID = -1,
    IF_TYPE_NONE,
    IF_TYPE_DSI,
    IF_TYPE_LVDS,
    IF_TYPE_INTERNAL = IF_TYPE_LVDS,
    IF_TYPE_I2C,
    IF_TYPE_SPI,
    IF_TYPE_HDMI,
    IF_TYPE_PARALLEL
};

enum {
    LVDS_MAP_FORMAT_NC = -1,
    LVDS_MAP_FORMAT_JEIDA,
    LVDS_MAP_FORMAT_SWPG
};

enum {
    LVDS_DATA_MAP_NC = -1,
    LVDS_DATA_MAP_RGB,
    LVDS_DATA_MAP_BGR
};

enum {
    LVDS_PAD_TXDRV_VOL_55_220MV = 0,
    LVDS_PAD_TXDRV_VOL_110_275MV,
    LVDS_PAD_TXDRV_VOL_165_330MV,
    LVDS_PAD_TXDRV_VOL_220_385MV
};

enum {
    LVDS_PAD_TXDRV_CUR_RTO_P8 = 0,  //-8%
    LVDS_PAD_TXDRV_CUR_RTO_0,       //0%
    LVDS_PAD_TXDRV_CUR_RTO_9,       //9%
    LVDS_PAD_TXDRV_CUR_RTO_20        //20%
};

typedef enum
{
    LCM_POLARITY_FALLING  = 0,
    LCM_POLARITY_RISING = 1
} LCM_POLARITY;

typedef enum
{
    LCM_POLARITY_HIGH_ACTIVE = 0,
    LCM_POLARITY_LOW_ACTIVE  = 1,
} LCM_SYNC_POLARITY;

enum {
    DSI_COLOR_CODE_16BIT_CONFIG1 = 1,
    DSI_COLOR_CODE_16BIT_CONFIG2 = 2,
    DSI_COLOR_CODE_16BIT_CONFIG3 = 3,
    DSI_COLOR_CODE_18BIT_CONFIG1 = 4,
    DSI_COLOR_CODE_18BIT_CONFIG2 = 5,
    DSI_COLOR_CODE_24BIT = 6,
    DSI_COLOR_CODE_20BIT_YCC422_LOOSELY = 7,
    DSI_COLOR_CODE_24BIT_YCC422 = 8,
    DSI_COLOR_CODE_16BIT_YCC422 = 9,
    DSI_COLOR_CODE_30BIT = 10,
    DSI_COLOR_CODE_36BIT = 11,
    DSI_COLOR_CODE_12BIT_YCC420 = 12,
    DSI_COLOR_CODE_DSC24 = 13,
};

struct display_timing {
    uint32_t tcon_clk;
    uint32_t pll_clk;
    uint32_t hactive;
    uint32_t hfront_porch;
    uint32_t hback_porch;
    uint32_t hsync_len;
    uint32_t vactive;
    uint32_t vfront_porch;
    uint32_t vback_porch;
    uint32_t vsync_len;

    /* polarity parameters */
    LCM_POLARITY dsp_clk_pol;
    LCM_SYNC_POLARITY de_pol;
    LCM_SYNC_POLARITY vsync_pol;
    LCM_SYNC_POLARITY hsync_pol;

    uint32_t map_format;
    uint32_t data_map_rb_swap;
};

struct sis_info {
    uint32_t pos_x;
    uint32_t pos_y;
    uint32_t width;
    uint32_t height;
};

struct dsi_cmd_desc {
    uint8_t data_type;
    uint8_t wait;
    uint8_t wc_h;
    uint8_t wc_l;
    uint8_t playload[MAX_DATA];
};

struct info_mipi {
    uint16_t burst_mode;
    uint16_t video_bus_width;
    uint32_t lane_number;
    uint32_t phy_freq;
    uint16_t color_coding;
    bool use_dcs;
    bool out_csi;
    struct dsi_cmd_desc *init_data;
    struct dsi_cmd_desc *sleep_data;
};

struct info_lvds {
    uint8_t vbias_sel;
    uint8_t txen;
    uint8_t txdrv_vol;
    uint8_t txdrv_cur;
    uint8_t tx_cm;
    uint8_t rterm_en;
    uint8_t bias_en;
    uint8_t dual_port_swap;
};

struct rle_info {
    int en;
    int data_size;
    int y_len;
    int y_checksum;
};

struct crc32block {
    unsigned int block_use;
    unsigned int cfg_start_x;
    unsigned int cfg_start_y;
    unsigned int cfg_end_x;
    unsigned int cfg_end_y;
    unsigned int expect_crc;
    unsigned int stride;
};

struct crc_info {
    int global_enable;
    struct crc32block block[MAX_CRC_BLOCK_NUM];
};

enum serdes_link{
    SER_DUAL_LINK = 0x0,
    SER_SINGLE_LINK = 0x1,
};

enum serdes_flags {
    SERDES_PORTA_LINESWAP = 1 << 0,
    SERDES_PORTB_LINESWAP = 1 << 1,
    SERDES_PORTAB_SWAP    = 1 << 2,
    DESERDES_PORTAB_SWAP  = 1 << 3,
};

struct serdes_info {
    char *name;
    void *adap;
    uint32_t ser_addr;
    uint32_t des_addr;
    uint32_t flags;
    enum serdes_link link_status;
};

struct backlight_info {
    uint32_t base;
    void *priv;
#ifdef CONFIG_PWM
    pwm_instance_t *pwm_ins;
    drv_pwm_simple_cfg_t *pwm_cfg;
    drv_pwm_chn_t pwm_chn;
#endif
};
struct sdm_panel_info {
    struct sdm_panel *panel;
    struct serdes_info *serdes;
    struct backlight_info *backlight;
};

struct sdm_panel {
    const char *panel_name;
    int if_type;
    int cmd_intf;
    uint32_t width_mm;
    uint32_t height_mm;
    int fps;
    bool re_if_init;
    // for panel voltage tuning
    uint8_t *cmd_data;
    uint8_t *reset_timing;
    struct display_timing *display_timing;
    struct info_mipi *mipi;
    struct info_lvds *lvds;
    int sis_num;
    struct sis_info *sis;
    struct sis_info *rtos_screen;
    int (*panel_pre_power_on)(struct sdm_panel_info *);
    int (*panel_post_begin)(struct sdm_panel_info *);
    int (*panel_post_end)(struct sdm_panel_info *);
    int (*panel_backlight_init)(struct backlight_info *, int);
};

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_t;

struct sdm_buffer {
    int layer;
    int layer_dirty;
    int layer_en;
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
    struct rle_info rle;//only spipe have this function.
};

struct sdm_post_config {
    size_t n_bufs;
    struct sdm_buffer *bufs;
    size_t custom_data_size;
    void *custom_data;
    bool post_noblock;
    bool post_relase;
    bool psot_no_wait_vsync;
};

struct sdm_pq_params {
    float contrast; //k
    float luminance;//l
    float saturation;//s
    float chroma;//h
};
#endif //__SDM_PANEL_H__
