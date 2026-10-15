//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_RPMSG_SERVICE

#include <rpmsg_service.h>
#include <driver/taishan/mbox/sdrv_taishan_mbox_drv.h>
#include "rpmsg_service_cfg.h"

#ifndef CONFIG_CORE_SHM_SIZE
#define CONFIG_CORE_SHM_SIZE 0x0
#endif

rpmsg_service_t g_rpmsg_services[RPMSG_SERVICES_NUM] = {
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SF, MBOX_RPROC_SP0, 0x5f8000, 0x8000 - CONFIG_CORE_SHM_SIZE, true},
};

#endif /* CONFIG_RPMSG_SERVICE */
