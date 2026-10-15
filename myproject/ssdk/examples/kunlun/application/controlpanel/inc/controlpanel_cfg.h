/**
 * @file controlpanel_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CONTROLPANEL_CFG_H__
#define __SD_CONTROLPANEL_CFG_H__

enum {
#ifdef CONFIG_USE_GUI_LVGL
    CP_LVGL_TASK = 0,
#endif
    CP_TASK_CNT,
};

#define CP_MASK(i) (1<<(i))

#endif /* __SD_CONTROLPANEL_CFG_H__ */
/* EOF */
