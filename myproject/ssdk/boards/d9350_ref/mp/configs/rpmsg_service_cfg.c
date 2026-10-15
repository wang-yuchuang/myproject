//*****************************************************************************
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_RPMSG_SERVICE

#include <rpmsg_service.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include "rpmsg_service_cfg.h"

rpmsg_service_t g_rpmsg_services[RPMSG_SERVICES_NUM] = {
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SEC, MBOX_RPROC_SAF, -1, -1, false},
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SEC, MBOX_RPROC_AP1, -1, -1, false},
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SEC, MBOX_RPROC_AP2, -1, -1, false},
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SEC, MBOX_RPROC_AP1, 0x36000000, 0x200000, false},
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SEC, MBOX_RPROC_AP2, 0x36200000, 0x200000, false}
};

#endif /* CONFIG_RPMSG_SERVICE */
