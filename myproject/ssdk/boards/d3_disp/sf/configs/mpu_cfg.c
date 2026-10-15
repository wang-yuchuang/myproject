
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

#include "mpu_cfg.h"
#include "regs_base.h"

const mpu_config_t board_mpu_config[] = {
    {0, 0x40, MPU_REGION_NORMAL_RO},
    {CONFIG_MEM_BASE, 0x40, MPU_REGION_NORMAL_RO},
    {0x1000000, 0x400000, MPU_REGION_DEVICE},
    {SEIP_SEIP_BASE, 0x100000, MPU_REGION_DEVICE},
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
