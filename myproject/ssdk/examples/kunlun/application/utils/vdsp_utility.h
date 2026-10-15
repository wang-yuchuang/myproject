/**
 * @file vdsp_utility.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv launcher application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __VDSP_UTILITY_H__
#define __VDSP_UTILITY_H__
#include <stdbool.h>
#include <memmap.h>
#include <regs_base.h>
#include <driver/scr.h>
#include <rstgen_cfg.h>
#include <driver/reset.h>
#include <scr_hw.h>
#include <param.h>
#include <malloc.h>
#include <elf.h>
#include <res_loader.h>
#include <scr_cfg.h>
#include <image_cfg.h>
#include "app_log.h"
        
#ifndef LOG_TAG
#define LOG_TAG "vdsp"
#endif

static inline int utils_vdsp_stall()
{
    APP_INFO("");
    const scr_signal_t vdsp_altresetvec = SCR_SEC__vdsp_altresetvec_31_4;
    const scr_signal_t vdsp_statvectorsel = SCR_SEC__vdsp_statvectorsel;
    const scr_signal_t vdsp_runstall = SCR_SEC__vdsp_runstall;

    if (!scr_set(&g_scr_sec, &vdsp_altresetvec, (uint32_t)(VDSP_MEMBASE>>4))) {
        goto err;
    }
    if (!scr_set(&g_scr_sec, &vdsp_statvectorsel, 1)) {
        goto err;
    }
    if (!scr_set(&g_scr_sec, &vdsp_runstall, 1)) {
        goto err;
    }

    return 0;

err:
    return -1;
}

static inline int utils_vdsp_core_reset()
{
    APP_INFO("");
    return (int)reset_ctl_reset(&rstsig_core_vdsp);
}

static inline int utils_vdsp_load_mapping_tbl(const char* mtbl_path, void** vdsp_map_tbl)
{
    APP_INFO("");
    if (!mtbl_path || !vdsp_map_tbl) {
        return -1;
    }

    int mtbl_size = ROUNDUP(res_size(mtbl_path), 32);
    uint32_t mtbl = (uint32_t)(ap2p(VDSP_MEMBASE)+VDSP_MEMSIZE-mtbl_size);
    ASSERT(!(mtbl%32));
    uint8_t* map_tlb = (uint8_t*)mtbl;
    int res = res_load(mtbl_path, (void*)map_tlb, mtbl_size, 0);
    *vdsp_map_tbl = map_tlb;
    APP_INFO("res load %s, mtbl:0x%08x, mtbl_path:%s", res<0 ? "fail" : "success", mtbl, mtbl_path);
    return 0;
}

static inline int utils_vdsp_load_firmware(const char* elf_path)
{
    APP_INFO("");
    if (!elf_path) {
        return -1;
    }

    int elf_size = ROUNDUP(res_size(elf_path), 32);
    void *elf = memalign(32, elf_size);
    if (!elf) {
        return -1;
    }

    if (res_load(elf_path, elf, elf_size, 0) < 0) {
        APP_ERR("res_load %s fail", elf_path);
        return -1;
    }

    elf_handle_t elf_handle;
    if (elf_open_handle_memory(&elf_handle, (void*)elf, elf_size) < 0) {
        APP_ERR("elf_open_handle_memory fail");
        return -1;
    }

    status_t ret = elf_load(&elf_handle);
    elf_close_handle(&elf_handle);
    free(elf);
    elf = NULL;
    return ret != 0 ? -1 : 0;
}

static inline int utils_vdsp_go()
{
    APP_INFO("");
    const scr_signal_t vdsp_runstall = SCR_SEC__vdsp_runstall;
    uint32_t value;
    value = scr_get(&g_scr_sec, &vdsp_runstall);
    bool ret = scr_set(&g_scr_sec, &vdsp_runstall, 0);
    value = scr_get(&g_scr_sec, &vdsp_runstall);
    return ret ? 0 : -1;
}
#endif /* __VDSP_UTILITY_H__ */
/* EOF */
