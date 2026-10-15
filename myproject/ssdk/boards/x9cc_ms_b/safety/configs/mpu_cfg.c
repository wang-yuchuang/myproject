/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_ARCH_USE_MPU

#include <types.h>
#include <param.h>
#include <arch/armv7-r/mpu.h>
#include <image_cfg.h>
#include <regs_base.h>

#include "mpu_cfg.h"
#include "driver/kunlun/virt_uart/logbuf.h"

const mpu_config_t board_mpu_config[] = {
    {0, 0x40, MPU_REGION_NORMAL_RO},
    {CONFIG_MEM_BASE, 0x40, MPU_REGION_NORMAL_RO},
    /* ospi1 indirect address space */
    {OSPI1_BASE + 0x4000000 - 0x200, 0x200, MPU_REGION_DEVICE},
    /* pciex1a space */
    {0x20000000, 0x10000000, MPU_REGION_DEVICE},
    /* pciex1b space */
    {0x30000000, 0x10000000, MPU_REGION_DEVICE},
    /* pciex4 space */
    {0x40000000, 0x10000000, MPU_REGION_DEVICE},
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
    /* Disable all region first to avoid regions left over by rom or bootloader
     * making safety os behaviour unpredictable.
     */
    mpu_clear_region();

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
