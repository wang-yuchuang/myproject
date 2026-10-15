/*
 * sdrv_g2dlite.c
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
#include <driver/irq.h>
#include <driver/kunlun/g2dlite/g2dlite.h>
#include <driver/kunlun/g2dlite/g2dlite_core.h>
#include <driver/kunlun/g2dlite/g2dlite_drv_log.h>
#include <stdlib.h>
#include <string.h>

osMutexId_t g2dlite_lock;

int sdrv_g2dlite_probe(struct g2dlite_device *dev,
                       const struct g2dlite_config *g2dlite_cfg)
{
    G2DLITE_LOG_INFO("enter\n");

    dev->reg_base = g2dlite_cfg->base;
    dev->cmd_info[0].arg =
        (unsigned int *)malloc(G2DLITE_CMDFILE_MAX_MEM * sizeof(unsigned int));
    if (dev->cmd_info[0].arg == NULL) {
        G2DLITE_LOG_ERR("malloc cmd_info failed\n");
        return -1;
    }
    G2DLITE_LOG_INFO("dev->cmd_info[0].arg = 0x%p\n", dev->cmd_info[0].arg);

    for (int i = 1; i < G2DLITE_CMDFILE_MAX_NUM; i++) {
        dev->cmd_info[i].arg =
            dev->cmd_info[i - 1].arg + G2DLITE_CMDFILE_MAX_LEN * 2;
    }

    dev->write_mode = G2DLITE_CMD_WRITE;
    dev->ops = &ops;
    dev->irq = g2dlite_cfg->irq;
    dev->irq_handler = ops.irq_handler;

    dev->done_sema = osSemaphoreNew(1, 0, NULL);
    g2dlite_lock = osMutexNew(NULL);

    if (dev->ops->init)
        dev->ops->init(dev);

    irq_attach(dev->irq, dev->irq_handler, dev);
    irq_enable(dev->irq);

    G2DLITE_LOG_INFO("leave\n");

    return 0;
}

int sdrv_g2dlite_update(struct g2dlite_device *dev, struct g2dlite_input *input)
{
    int ret = -1;

    osStatus_t sts = osMutexAcquire(g2dlite_lock, 0);
    if (sts != osOK) {
        goto exit;
    }

    if (dev->ops->update) {
        ret = dev->ops->update(dev, input);
        if (ret) {
            G2DLITE_LOG_ERR("update failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("update is null\n");
        return ret;
    }

    if (dev->ops->reset)
        dev->ops->reset(dev);

    if (dev->ops->enable) {
        ret = dev->ops->enable(dev);
        if (ret) {
            G2DLITE_LOG_ERR("enable failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("enable is null\n");
        return ret;
    }

    osSemaphoreAcquire(dev->done_sema, osWaitForever);
    osMutexRelease(g2dlite_lock);

exit:
    return ret;
}

int sdrv_g2dlite_rotaion(struct g2dlite_device *dev,
                         struct g2dlite_input *input)
{
    int ret = -1;

    osStatus_t sts = osMutexAcquire(g2dlite_lock, 0);
    if (sts != osOK) {
        goto exit;
    }

    if (dev->ops->rotation) {
        ret = dev->ops->rotation(dev, input);
        if (ret) {
            G2DLITE_LOG_ERR("rotation failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("rotation is null\n");
        return ret;
    }

    if (dev->ops->reset)
        dev->ops->reset(dev);

    if (dev->ops->enable) {
        ret = dev->ops->enable(dev);
        if (ret) {
            G2DLITE_LOG_ERR("enable failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("enable is null\n");
        return ret;
    }

    osSemaphoreAcquire(dev->done_sema, osWaitForever);
    osMutexRelease(g2dlite_lock);

exit:
    return ret;
}

int sdrv_g2dlite_fill_rect(struct g2dlite_device *dev,
                           struct g2dlite_bg_cfg *bgcfg,
                           struct g2dlite_output_cfg *output)
{
    int ret = -1;

    osStatus_t sts = osMutexAcquire(g2dlite_lock, 0);
    if (sts != osOK) {
        goto exit;
    }

    if (dev->ops->fill_rect) {
        ret = dev->ops->fill_rect(dev, bgcfg, output);
        if (ret) {
            G2DLITE_LOG_ERR("fill_rect failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("fill_rect is null\n");
        return ret;
    }

    if (dev->ops->reset)
        dev->ops->reset(dev);

    if (dev->ops->enable) {
        ret = dev->ops->enable(dev);
        if (ret) {
            G2DLITE_LOG_ERR("enable failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("enable is null\n");
        return ret;
    }

    osSemaphoreAcquire(dev->done_sema, osWaitForever);
    osMutexRelease(g2dlite_lock);

exit:
    return 0;
}

int sdrv_g2dlite_fastcopy(struct g2dlite_device *dev, struct fcopy_t *in,
                          struct fcopy_t *out)
{
    int ret = -1;

    osStatus_t sts = osMutexAcquire(g2dlite_lock, 0);
    if (sts != osOK) {
        goto exit;
    }

    if (dev->ops->fastcopy) {
        ret = dev->ops->fastcopy(dev, in, out);
        if (ret) {
            G2DLITE_LOG_ERR("fastcopy failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("fastcopy is null\n");
        return ret;
    }

    if (dev->ops->reset)
        dev->ops->reset(dev);

    if (dev->ops->enable) {
        ret = dev->ops->enable(dev);
        if (ret) {
            G2DLITE_LOG_ERR("enable failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("enable is null\n");
        return ret;
    }

    osSemaphoreAcquire(dev->done_sema, osWaitForever);
    osMutexRelease(g2dlite_lock);

exit:
    return 0;
}

int sdrv_g2dlite_clut_setting(struct g2dlite_device *dev, char *clut_table)
{
    int ret = -1;

    if (dev->ops->clut_index_set) {
        ret = dev->ops->clut_index_set(dev, clut_table);
        if (ret) {
            G2DLITE_LOG_ERR("clut setting failed\n");
            return ret;
        }
    } else {
        G2DLITE_LOG_ERR("clut index set is null\n");
        return ret;
    }
    return 0;
}
