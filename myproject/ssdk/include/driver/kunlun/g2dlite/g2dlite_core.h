/*
 * g2dlite_core.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 * 011, 04/28/2020 BI create this file
 */
#ifndef __G2DLITE_CORE_H__
#define __G2DLITE_CORE_H__
#include <driver/kunlun/g2dlite/g2dlite_drv_log.h>
#include <kunlun/dispss/sd_disp/inc/disp_data_type.h>
#include <reg.h>
#include <string.h>

enum {
    G2DLITE_FUN_BLEND = 0b001,
    G2DLITE_FUN_ROTATION = 0b010,
};

typedef enum {
    G2DLITE_CPU_WRITE,
    G2DLITE_CMD_WRITE,
} g2dlite_write_mode;

typedef enum {
    G2DLITE_MLC_GP,
    G2DLITE_MLC_SP,
    G2DLITE_MLC_MAX
} g2dlite_mlc_layer;

typedef enum {
    MLC_PATH_BG,
    MLC_PATH_GP,
    MLC_PATH_SP,
    MLC_PATH_MAX
} g2dlite_mlc_path;

typedef struct {
    unsigned long base_addr;
    unsigned int irq;
    unsigned int layer_n;
} g2dlite_profile_t;

static inline unsigned int reg_value(unsigned int val, unsigned int src,
                                     unsigned int shift, unsigned int mask)
{
    return (src & ~mask) | ((val << shift) & mask);
}

static inline unsigned int g2dlite_read(unsigned long base, unsigned int reg)
{
    return readl(base + reg);
}

static inline void g2dlite_write(unsigned long base, unsigned int reg,
                                 unsigned int value)
{
    writel(value, base + reg);
#if 0
    G2DLITE_LOG_TREACE("w(0x%lx, 0x%08x), r(0x%08x)\n", (base + reg),
        value, readl(base + reg));
#endif
}
static inline void g2dlite_cwrite(struct g2dlite_device *dev, int cmd_id,
                                  unsigned int reg, unsigned int value)
{
    if (dev->write_mode == G2DLITE_CMD_WRITE) {
        cmdfile_info *cmd = &dev->cmd_info[cmd_id];
        cmd->arg[cmd->len++] = reg;
        cmd->arg[cmd->len++] = value;
#if 0
        G2DLITE_LOG_TREACE("cmd_id[%d] w(0x%lx, 0x%08x)\n", cmd_id, (dev->reg_base + reg), value);
#endif
        if (!cmd->dirty)
            cmd->dirty = 1;
    } else {
        writel(value, dev->reg_base + reg);
#if 0
        G2DLITE_LOG_TREACE("w(0x%lx, 0x%08x), r(0x%08x)\n", (dev->reg_base + reg),
            value, readl(dev->reg_base + reg));
#endif
    }
}

#endif //__G2DLITE_CORE_H__
