/**
 * @file boardinfo.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <reg.h>
#include <regs_base.h>
#include <sdrv_extra_gpr.h>
#include <boardinfo.h>

#ifndef RSTGEN_GENERAL_REG
#define RSTGEN_GENERAL_REG(n) ((0x50 + (n)*4) << 10)
#endif

#define RSTGEN_HWID_REG (APB_RSTGEN_SEC_BASE + RSTGEN_GENERAL_REG(5))

#define SUPPORT_EXTRA_GPR 1

struct sd_hwid_usr g_hwid_usr = {0};
static struct sd_hwid_usr g_hwid_usr_dummy = {
    .magic = HW_ID_MAGIC,
    .ver = 1,
    .v.v1 = {
        .chipid = 0,
        .featurecode = 0,
        .speed_grade = 0,
        .temp_grade = 0,
        .pkg_type = 0,
        .revision = 0,
        .board_type = BOARD_TYPE_UNKNOWN,
        .board_id_major = BOARDID_MAJOR_UNKNOWN,
        .board_id_minor = 0,
    },
};

struct sd_hwid_usr get_fullhw_id(void)
{
    struct sd_hwid_usr *usr;
    uint32_t v;

    v = readl(RSTGEN_HWID_REG);
    usr = (struct sd_hwid_usr *)&v;

    if (usr->magic != HW_ID_MAGIC) {
        ssdk_printf(SSDK_WARNING, "%s: get_fullhw_id failed.\n", __func__);
    }

    return *usr;
}

static bool is_valid_hwid_usr(struct sd_hwid_usr *id)
{
    return id->magic == HW_ID_MAGIC;
}

static bool init_hwid_from_system(struct sd_hwid_usr *id)
{
    struct sd_hwid_usr *usr = NULL;
    uint32_t v;
#if SUPPORT_EXTRA_GPR
    uint32_t ret;
    ret = sdrv_extra_gpr_get(SDRV_EXTRA_GPR_NAME_HWID, &v);
    if (!ret) {
        usr = (struct sd_hwid_usr *)&v;
    }
    else {
        printf("fail to get extra gpr: %s, error code: %d\n", SDRV_EXTRA_GPR_NAME_HWID, ret);
    }
#else
    int count = 10000;

    do {
        v = readl(_ioaddr(RSTGEN_HWID_REG));
        usr = (struct sd_hwid_usr *)&v;
    } while (usr->magic != HW_ID_MAGIC && count-- > 0);
#endif

    if (!usr || usr->magic != HW_ID_MAGIC) {
#if !SUPPORT_FAST_BOOT
        printf("get hwid timeout, will use the dummy one?\n");
#endif
        usr = &g_hwid_usr_dummy;
    }

    *id = *usr;
    return true;
}

int get_part_id(enum part_e part)
{
    if (!is_valid_hwid_usr(&g_hwid_usr))
        init_hwid_from_system(&g_hwid_usr);

    if (g_hwid_usr.magic != HW_ID_MAGIC)
        ssdk_printf(SSDK_WARNING,"magic num is not right, %d\n", g_hwid_usr.magic);

    if (g_hwid_usr.ver == 1) {
        struct version1 *v = &(g_hwid_usr.v.v1);
        GET_PART_ID(part, v);
    }

err:
    printf("code is too old to parse the id\n");
    return 0;
}

