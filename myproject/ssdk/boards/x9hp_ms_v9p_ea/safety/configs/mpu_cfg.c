
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_ARCH_USE_MPU

#include <types.h>
#include <param.h>
#include <arch/armv7-r/mpu.h>
#include <image_cfg.h>

#include "mpu_cfg.h"
#include "driver/kunlun/virt_uart/logbuf.h"

const mpu_config_t board_mpu_config[] = {
    {0, 0x40, MPU_REGION_NORMAL_RO},
    {CONFIG_MEM_BASE, 0x40, MPU_REGION_NORMAL_RO},
    /* ospi1 indirect address space */
    {0x7FFFE00, 0x200, MPU_REGION_DEVICE},
    /* pcie1 space */
    {0x10000000, 0x10000000, MPU_REGION_DEVICE},
    /* pcie2 space */
    {0x20000000, 0x10000000, MPU_REGION_DEVICE},
    {SAF_SEC_MEMBASE, SAF_SEC_MEMSIZE, MPU_REGION_STRONGORDERED},
    {SAF_MP_MEMBASE,  SAF_MP_MEMSIZE,  MPU_REGION_STRONGORDERED},
    {SAF_AP1_MEMBASE, SAF_AP1_MEMSIZE, MPU_REGION_STRONGORDERED},
    {SAF_AP2_MEMBASE, SAF_AP2_MEMSIZE, MPU_REGION_STRONGORDERED},
    /* logbuf sapce */
    {SEC_MEMBASE, SDRV_LOGBUF_SIZE, MPU_REGION_NORMAL_NONCACHEABLE},
    /* ramoops size */
    {SEC_MEMBASE + SDRV_LOGBUF_SIZE, SDRV_RAMOOPS_SIZE, MPU_REGION_NORMAL_NONCACHEABLE}
};

void board_mpu_init(void)
{
    uint32_t index = 0;

    mpu_enable(false);

    for (uint32_t i = 0; i < ARRAY_SIZE(board_mpu_config); i++) {
        const mpu_config_t *config = &board_mpu_config[i];
        if (config->size > 0) {
            mpu_add_region(index++, config->addr, config->size, config->type);
        }
    }

    if (index > 0) {
        mpu_enable(true);
    }
}

#endif
