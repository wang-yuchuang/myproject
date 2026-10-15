/*
 * sdrv_extra_gpr.c
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <arch/cache.h>
#include <debug.h>
#ifdef __IMAGE_CFG_H__
#include <image_cfg.h>
#endif
#include <reg.h>
#include <regs_base.h>
#include <string.h>

#include "sdrv_extra_gpr.h"

#ifdef DIL_IMAGES_MEMBASE
#define IMG_BACKUP_LOW_BASE (DIL_IMAGES_MEMBASE)
#define IMG_BACKUP_LOW_SZ (DIL_IMAGES_MEMSIZE)
#else
#define IMG_BACKUP_LOW_BASE (0x0)
#define IMG_BACKUP_LOW_SZ (0x0)
#endif

#ifndef SDRV_GENERAL_REG
#define SDRV_GENERAL_REG(n)                                                    \
    (APB_RSTGEN_SEC_BASE + (uint32_t)((0x50 + (n)*4) << 10))
#endif

#ifndef SDRV_REG_HWID
#define SDRV_REG_HWID 5
#endif

static inline uint32_t sdrv_common_reg_get_u32(uint32_t reg)
{
    return readl(_ioaddr(SDRV_GENERAL_REG(reg)));
}

typedef struct sdrv_extra_gpr {
    uint32_t key;
    uint32_t value;
} sdrv_extra_gpr_t;

typedef struct sdrv_extra_gpr_head {
    volatile uint32_t magic;
    volatile uint32_t count;
    volatile sdrv_extra_gpr_t gpr[0];
} sdrv_extra_gpr_head_t;

static addr_t sdrv_extra_gpr_addr_base_get(void)
{
    uint32_t addr_base;
    uint32_t magic;

    addr_base = sdrv_common_reg_get_u32(SDRV_REG_HWID);
    magic = addr_base & (uint32_t)SDRV_EXTRA_GPR_ADDR_MASK;

    if (magic != SDRV_EXTRA_GPR_ADDR_MAGIC) {
        ssdk_printf(SSDK_ERR, "sdrv extra gpr addr base is not valid\n");
        return 0;
    } else {
        addr_base &= ~(uint32_t)SDRV_EXTRA_GPR_ADDR_MASK;
    }

    if (addr_base >= IMG_BACKUP_LOW_BASE &&
        addr_base <= IMG_BACKUP_LOW_BASE + IMG_BACKUP_LOW_SZ) {
        return (addr_t)addr_base;
    } else {
        ssdk_printf(SSDK_ERR, "sdrv extra gpr addr base is not valid\n");
        return 0;
    }
}

static uint32_t sdrv_extra_gpr_key_translate(const char *gpr_name)
{
    int len = strlen(gpr_name);
    uint32_t key = 0;

    if (len > SDRV_EXTRA_GPR_NAME_SIZE || len == 0) {
        ssdk_printf(SSDK_ERR, "sdrv extra gpr name length range must be 1-%d\n",
                    SDRV_EXTRA_GPR_NAME_SIZE);
        return 0;
    }

    while (len > 0) {
        len--;
        key |= gpr_name[len] << (8 * len);
    }

    return key;
}

uint32_t sdrv_extra_gpr_get(const char *name, uint32_t *value)
{
    addr_t addr_base = 0;
    sdrv_extra_gpr_head_t *sdrv_extra_gpr_head = NULL;
    uint32_t key;
    uint32_t ret;

    if (!name || !value) {
        ssdk_printf(SSDK_ERR, "invalid sdrv extra gpr para\n");
        ret = SDRV_EXTRA_GPR_ERROR_INVALID_NAME;
        return ret;
    }

    addr_base = sdrv_extra_gpr_addr_base_get();

    if (!addr_base) {
        ssdk_printf(SSDK_ERR, "fail to get sdrv extra gpr addr base\n");
        ret = SDRV_EXTRA_GPR_ERROR_INVALID_ADDR_BASE;
        return ret;
    }

    key = sdrv_extra_gpr_key_translate(name);

    if (!key) {
        ssdk_printf(SSDK_ERR, "invalid sdrv extra gpr name\n");
        ret = SDRV_EXTRA_GPR_ERROR_INVALID_NAME;
        return ret;
    }

    arch_invalidate_cache_range(addr_base, SDRV_EXTRA_GPR_TOTAL_SIZE);
    sdrv_extra_gpr_head = (sdrv_extra_gpr_head_t *)addr_base;

    if (sdrv_extra_gpr_head->magic != SDRV_EXTRA_GPR_MAGIC) {
        ssdk_printf(SSDK_ERR, "sdrv extra gpr have not been initialized\n");
        ret = SDRV_EXTRA_GPR_ERROR_NOT_INITIALIZED;
        goto out;
    }

    for (uint32_t i = 0; i < sdrv_extra_gpr_head->count; i++) {
        if (sdrv_extra_gpr_head->gpr[i].key == key) {
            *value = sdrv_extra_gpr_head->gpr[i].value;
            ret = SDRV_EXTRA_GPR_OK;
            goto out;
        }
    }

    ssdk_printf(SSDK_ERR, "fail to find extra gpr\n");
    ret = SDRV_EXTRA_GPR_ERROR_FIND_KEY_FAIL;

out:
    arch_clean_invalidate_cache_range(addr_base, SDRV_EXTRA_GPR_TOTAL_SIZE);
    return ret;
}

uint32_t sdrv_extra_gpr_set(const char *name, uint32_t value)
{
    addr_t addr_base = 0;
    sdrv_extra_gpr_head_t *sdrv_extra_gpr_head = NULL;
    uint32_t ret;
    uint32_t num = 0;
    uint32_t key;

    if (!name) {
        ssdk_printf(SSDK_ERR, "invalid sdrv extra gpr para\n");
        ret = SDRV_EXTRA_GPR_ERROR_INVALID_NAME;
        return ret;
    }

    addr_base = sdrv_extra_gpr_addr_base_get();

    if (!addr_base) {
        ssdk_printf(SSDK_ERR, "fail to get sdrv extra gpr addr base\n");
        ret = SDRV_EXTRA_GPR_ERROR_INVALID_ADDR_BASE;
        return ret;
    }

    key = sdrv_extra_gpr_key_translate(name);

    if (!key) {
        ssdk_printf(SSDK_ERR, "invalid sdrv extra gpr name\n");
        ret = SDRV_EXTRA_GPR_ERROR_INVALID_NAME;
        return ret;
    }

    arch_invalidate_cache_range(addr_base, SDRV_EXTRA_GPR_TOTAL_SIZE);
    sdrv_extra_gpr_head = (sdrv_extra_gpr_head_t *)addr_base;

    if (sdrv_extra_gpr_head->magic != SDRV_EXTRA_GPR_MAGIC) {
        ssdk_printf(SSDK_ERR, "sdrv extra gpr have not been initialized\n");
        ret = SDRV_EXTRA_GPR_ERROR_NOT_INITIALIZED;
        goto out;
    }

    for (uint32_t i = 0; i < sdrv_extra_gpr_head->count; i++) {
        if (sdrv_extra_gpr_head->gpr[i].key == key) {
            ssdk_printf(SSDK_ERR, "you have set this sdrv extra gpr\n");
            sdrv_extra_gpr_head->gpr[i].value = value;
            ret = SDRV_EXTRA_GPR_OK;
            goto out;
        }
    }

    if (sdrv_extra_gpr_head->count >= SDRV_EXTRA_GPR_MAX_COUNT) {
        ssdk_printf(SSDK_ERR, "sdrv extra gpr is full\n");
        ret = SDRV_EXTRA_GPR_ERROR_FULL;
        goto out;
    }

    num = sdrv_extra_gpr_head->count;
    sdrv_extra_gpr_head->gpr[num].key = key;
    sdrv_extra_gpr_head->gpr[num].value = value;
    sdrv_extra_gpr_head->count++;
    ret = SDRV_EXTRA_GPR_OK;

out:
    arch_clean_invalidate_cache_range(addr_base, SDRV_EXTRA_GPR_TOTAL_SIZE);
    return ret;
}
