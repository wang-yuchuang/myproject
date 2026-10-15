//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_RPMSG_SERVICE
#include <image_cfg.h>
#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include "rpmsg_service_cfg.h"

rpmsg_service_t g_rpmsg_services[RPMSG_SERVICES_NUM] = {
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SAF, MBOX_RPROC_SEC, -1, -1, true},
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SAF, MBOX_RPROC_AP1, -1, -1, false},
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SAF, MBOX_RPROC_AP2, -1, -1, false},
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SAF, MBOX_RPROC_AP1, SAF_AP1_MEMBASE, SAF_AP1_MEMSIZE, false},
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SAF, MBOX_RPROC_AP2, SAF_AP2_MEMBASE, SAF_AP2_MEMSIZE, false}
};

#endif /* CONFIG_RPMSG_SERVICE */
