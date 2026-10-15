/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_UPDATE_MONITOR

#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include "rpmsg_service_cfg.h"
#include "update_monitor_cfg.h"


update_monitor_cfg_t g_update_mon_cfg[UPDATE_MON_NUM] = {
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SEC, MBOX_RPROC_AP1, DISK_TYPE_OSPI1},
};

#endif
