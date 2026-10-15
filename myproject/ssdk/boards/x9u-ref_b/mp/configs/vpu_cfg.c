/**
 * @file vpu_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: vpu config implement file
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#ifdef CONFIG_VPU
#include <driver/kunlun/vpu/vpu.h>
#include <driver/kunlun/vpu/sdrv_jpu.h>

#include "vpu_cfg.h"
#include "regs_base.h"
#include "irq_num.h"

/* =========================== global value define session ================= */
vpu_dev_t* jpu_dev = NULL;

/* =========================== static value define session ================= */
/** mjpeg jpu driver */
static sdrv_jpu_drv_t sdrv_jpu;

/** vpu config */
static vpu_config_t cfg = {
    .dev_name = "mjpeg-jpu",
    .base = APB_MJPEG_BASE,
    .irq_num = MJPEG_O_REQ_INTR_NUM,
};

/* =========================== function implementation session ============= */
void board_vpu_init()
{
    jpu_dev = jpu_probe(&sdrv_jpu, &cfg);
}

#endif /* end CONFIG_VPU */
/* end of file vpu_cfg.c */