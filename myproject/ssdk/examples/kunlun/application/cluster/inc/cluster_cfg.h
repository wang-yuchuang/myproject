/**
 * @file cluster_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CLUSTER_CFG_H__
#define __SD_CLUSTER_CFG_H__

enum {
#ifdef CONFIG_USE_GUI_LVGL
    CLUSTER_LVGL_TASK = 0,
#endif
    CLUSTER_TASK_CNT,
};

#define CLUSTER_MASK(i) (1<<(i))

#endif /* __SD_CLUSTER_CFG_H__ */
/* EOF */
