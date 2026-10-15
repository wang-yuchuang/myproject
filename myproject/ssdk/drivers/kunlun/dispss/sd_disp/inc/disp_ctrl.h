/**
 * @file disp_ctrl.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DISP_CTRL_H__
#define __DISP_CTRL_H__
#include <sdm_panel.h>
#include <string.h>
#include <cmsis_os2.h>

#define MAX_DISP_NUM    5

typedef enum {
    PIPE_TYPE_SPIPE,
    PIPE_TYPE_GPIPE,
    PIPE_TYPE_NUM
} pipe_type_t;

typedef enum {
    MASTER_MODE = 0,
    SLAVE_MODE,
} ms_mode_t;

struct ms_info_t {
    int ms_en;
    int ms_mode;
    int start;
    int end;
};

enum {
    CRC32_MODE_INTERNAL = 0,
    CRC32_MODE_EXTERNAL = 1
};

struct dc_device {
    int display_id;
    int dc_idx;
    struct ms_info_t ms_info;
    int mlc_select;
    unsigned int irq;
    unsigned long  base;
    double dsp_clk;
    double ratio;
    unsigned int aflu_time;
    osMutexId_t mutex_refresh;
    int last_zorder[PIPE_TYPE_NUM];
    int vsync_wait;
    struct dc_operations *ops;
    bool run_status;
    int dp_res;
    int (*vsync_callback)(int display_id, int64_t timestamp);
};

struct dc_operations {
    int (*init)(struct dc_device *dev, struct sdm_panel *panel);
    int (*reset)(struct dc_device *dev);
    int (*update)(struct dc_device *dev, struct sdm_post_config *post);
    void (*clear_layers)(struct dc_device *dev, uint8_t mask, uint8_t z_order);
    void (*triggle)(struct dc_device *dev);
    unsigned int (*check_triggle_status)(struct dc_device *dev);
    void (*vsync_enable)(struct dc_device *dev, bool enable);
    void (*enable)(struct dc_device *dev);
#ifdef _CONFIG_OS_SAFETY_
    enum handler_return (*irq_handler)(void *arg);
#else
    int (*irq_handler)(uint32_t irq, void *arg);
#endif
    int (*csc_set)(struct dc_device *dev, struct sdm_pq_params *pq);
    unsigned int (*check_tcon_status)(struct dc_device *dev);
    void (*crc32_set)(struct dc_device *dev, struct crc_info *info);
    void (*crc32_disable)(struct dc_device *dev, struct crc_info *info);
    unsigned int (*crc32_get_result)(struct dc_device *dev, uint8_t block_index);
    void (*crc32_enable_irq)(struct dc_device *dev, uint8_t block_index);
    void (*crc32_disable_irq)(struct dc_device *dev, uint8_t block_index);
    int (*safety_config)(struct dc_device *dev, struct sdm_buffer *layer);
    void (*safety_exit)(struct dc_device *dev);
};

int disp_vsync_callback(int display_id);
#endif //__DISP_CTRL_H__