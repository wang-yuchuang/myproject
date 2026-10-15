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
#include "image_cfg.h"

rpmsg_service_t g_rpmsg_services[RPMSG_SERVICES_NUM] = {
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SEC, MBOX_RPROC_SAF, SAF_SEC_MEMBASE, SAF_SEC_MEMSIZE, true},
    {RPMSG_TYPE_VIRTIO, MBOX_RPROC_SEC, MBOX_RPROC_AP2, SEC_AP2_MEMBASE, SEC_AP2_MEMSIZE, false},
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SEC, MBOX_RPROC_SAF, 0, 0, true},
    {RPMSG_TYPE_IPCC, MBOX_RPROC_SEC, MBOX_RPROC_AP2, 0, 0, false},
};
#endif /* CONFIG_RPMSG_SERVICE */
