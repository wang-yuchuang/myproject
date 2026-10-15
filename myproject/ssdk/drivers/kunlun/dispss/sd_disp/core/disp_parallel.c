/*
* disp_lvds.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 16/10/2019 BI create this file
*/
#include <disp_drv_log.h>
#include <disp_interface.h>
#include <reg.h>
#include <stdlib.h>
#include <string.h>


static int parallel_init(struct intf_device *dev)
{
    return 0;
}

static int parallel_deinit(struct intf_device *dev)
{
    return 0;
}

static void parallel_config(struct intf_device *dev, struct sdm_panel *panel)
{
    return;
}

DISP_MODULE_DRIVER disp_driver_parallel = {
    .module = DISP_IF_MODULE_PARALLEL,
    .init   = parallel_init,
    .deinit = parallel_deinit,
    .config = parallel_config,
};

