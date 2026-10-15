/*
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 */

#ifndef __SDRV_COMMON_REG_H__
#define __SDRV_COMMON_REG_H__

#include <reg.h>
#include <regs_base.h>
#include <types.h>

#ifndef SDRV_GENERAL_REG
#define SDRV_GENERAL_REG(n) (APB_RSTGEN_SEC_BASE + (uint32_t)((0x50 + (n)*4) << 10))
#endif

#define SDRV_REG_ROM 0
#define SDRV_REG_REMAP 1
#define SDRV_REG_SDPE 2
#define SDRV_REG_MCAL 3
#define SDRV_REG_BOOTREASON 4
#define SDRV_REG_HWID 5
#define SDRV_REG_STATUS 6
#define SDRV_REG_DDR_FREQ 7

/* Fields for SDRV_REG_STATUS Register */
/* The NOC has been initialized */
#define SDRV_REG_STATUS_NOC_INIT_DONE (1<<0)
/* The DDR has been initialized */
#define SDRV_REG_STATUS_DDR_INIT_DONE (1<<1)
/* The firewall early config done */
#define SDRV_REG_STATUS_FIREWALL_EARLY_DONE (1<<2)
/* The firewall formal config done */
#define SDRV_REG_STATUS_FIREWALL_CFG_DONE (1<<3)
/* The sdpe load done */
#define SDRV_REG_STATUS_SDPE_LD_DONE (1<<4)
/* The safety handover done */
#define SDRV_REG_STATUS_HANDOVER_DONE (1<<5)
/* The str rtc flag reg read done */
#define SDRV_REG_STATUS_STR_REG_READ_DONE (1<<6)
/* The str status */
#define SDRV_REG_STATUS_STR_RESUME (1<<7)
/* The dil load safety to DDR done */
#define SDRV_REG_STATUS_LOAD_SAFETY_DONE (1<<8)
/* The bootloader done */
#define SDRV_REG_STATUS_BOOT_DONE (1<<9)
/* fuse init reboot requirement */
#define SDRV_REG_STATUS_FUSE_INIT_REBOOT_REQ (1<<10)
/* The partition attr flag reg read done */
#define SDRV_REG_STATUS_PARTITION_ATTR_READ_DONE (1<<11)
/* The partition attr : A active */
#define SDRV_REG_STATUS_PARTITION_ACTIVE_A (1<<12)
/* The partition attr : B active */
#define SDRV_REG_STATUS_PARTITION_ACTIVE_B (1<<13)
/* The partition attr : A unbootable */
#define SDRV_REG_STATUS_PARTITION_UNBOOTABLE_A (1<<14)
/* The partition attr : B unbootable */
#define SDRV_REG_STATUS_PARTITION_UNBOOTABLE_B (1<<15)
/* The partition attr : A successful */
#define SDRV_REG_STATUS_PARTITION_SUCCESSFUL_A (1<<16)
/* The partition attr : B successful */
#define SDRV_REG_STATUS_PARTITION_SUCCESSFUL_B (1<<17)
/* The partition attr : A retry */
#define SDRV_REG_STATUS_PARTITION_RETRY_A (3<<18)   //18~19
/* The partition attr : B retry */
#define SDRV_REG_STATUS_PARTITION_RETRY_B (3<<20)   //20~21

static inline uint32_t sdrv_common_reg_get_u32(uint32_t reg)
{
    return readl(SDRV_GENERAL_REG(reg));
}

static inline void sdrv_common_reg_set_u32(uint32_t value, uint32_t reg)
{
    writel(value, SDRV_GENERAL_REG(reg));
}

static inline uint32_t sdrv_common_reg_get_value(uint32_t reg, uint32_t mask)
{
    return readl(SDRV_GENERAL_REG(reg)) & mask;
}

static inline void sdrv_common_reg_set_value(uint32_t reg, uint32_t value,
        uint32_t mask)
{
    uint32_t tmp_value;
    tmp_value = readl(SDRV_GENERAL_REG(reg)) & ~mask;
    tmp_value |= value & mask;
    writel(tmp_value, SDRV_GENERAL_REG(reg));
}

#endif
