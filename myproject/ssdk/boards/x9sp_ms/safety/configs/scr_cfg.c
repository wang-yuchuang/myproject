/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_SCR
#include <regs_base.h>
#include <driver/scr.h>
#include <debug.h>

#include "scr_cfg.h"
#include "scr_hw.h"

// #define AP_ACCESS   1
// #define SAF_ACCESS  0

/* scr device */
scr_dev_t g_scr_safety = {
    .base = APB_SCR_SAF_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 10,
    .reg_off = {
        [TYPE_RW] = 0,
        [TYPE_RO] = 0x100,
        [TYPE_L16] = 0x200,
        [TYPE_L31] = 0x400,
        [TYPE_R16W16] = 0x600,
    },
};

scr_dev_t g_scr_sec = {
    .base = APB_SCR_SEC_BASE,
    .lock = SPIN_LOCK_INITIAL_VALUE,
    .reg_shift = 10,
    .reg_off = {
        [TYPE_RW] = 0,
        [TYPE_RO] = 0x100,
        [TYPE_L16] = 0x200,
        [TYPE_L31] = 0x400,
        [TYPE_R16W16] = 0x600,
    },
};

// scr_dev_t g_scr_hpi = {
//     .base = APB_SCR_HPI_BASE,
//     .lock = SPIN_LOCK_INITIAL_VALUE,
//     .reg_shift = 0,
//     .reg_off = {
//         [TYPE_RW] = 0,
//         [TYPE_RO] = 0x100,
//         [TYPE_L16] = 0x200,
//         [TYPE_L31] = 0x400,
//         [TYPE_R16W16] = 0x600,
//     },
// };

// typedef struct {
//     scr_dev_t* scr_dev;
//     scr_signal_t signal;
//     uint32_t access_mode;
// }res_access_ctl_t;

// typedef struct {
//     scr_dev_t* scr_dev;
//     scr_signal_t src_sel_1_0;
//     scr_signal_t xtal_cg_en;
//     scr_signal_t fs_rc_en;
//     scr_signal_t fs_xtal_en;
// }res_fsrefclk_config_t;

// res_access_ctl_t s_access_ctl_tbl[] = {
//     {NULL}
// };

// res_fsrefclk_config_t s_fsrefclk_config_tbl[] = {
//     {
//         &g_scr_safety,
//         SCR_SAFETY__fsrefclk_saf_src_sel_1_0,
//         SCR_SAFETY__fsrefclk_saf_xtal_cg_en,
//         SCR_SAFETY__fsrefclk_saf_fs_rc_en,
//         SCR_SAFETY__fsrefclk_saf_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_sec_src_sel_1_0,
//         SCR_SEC__fsrefclk_sec_xtal_cg_en,
//         SCR_SEC__fsrefclk_sec_fs_rc_en,
//         SCR_SEC__fsrefclk_sec_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_cpu1_src_sel_1_0,
//         SCR_SEC__fsrefclk_cpu1_xtal_cg_en,
//         SCR_SEC__fsrefclk_cpu1_fs_rc_en,
//         SCR_SEC__fsrefclk_cpu1_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_cpu2_src_sel_1_0,
//         SCR_SEC__fsrefclk_cpu2_xtal_cg_en,
//         SCR_SEC__fsrefclk_cpu2_fs_rc_en,
//         SCR_SEC__fsrefclk_cpu2_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_gpu1_src_sel_1_0,
//         SCR_SEC__fsrefclk_gpu1_xtal_cg_en,
//         SCR_SEC__fsrefclk_gpu1_fs_rc_en,
//         SCR_SEC__fsrefclk_gpu1_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_gpu2_src_sel_1_0,
//         SCR_SEC__fsrefclk_gpu2_xtal_cg_en,
//         SCR_SEC__fsrefclk_gpu2_fs_rc_en,
//         SCR_SEC__fsrefclk_gpu2_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_his_src_sel_1_0,
//         SCR_SEC__fsrefclk_his_xtal_cg_en,
//         SCR_SEC__fsrefclk_his_fs_rc_en,
//         SCR_SEC__fsrefclk_his_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_vpu_src_sel_1_0,
//         SCR_SEC__fsrefclk_vpu_xtal_cg_en,
//         SCR_SEC__fsrefclk_vpu_fs_rc_en,
//         SCR_SEC__fsrefclk_vpu_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_vsn_src_sel_1_0,
//         SCR_SEC__fsrefclk_vsn_xtal_cg_en,
//         SCR_SEC__fsrefclk_vsn_fs_rc_en,
//         SCR_SEC__fsrefclk_vsn_fs_xtal_en
//     },
//     {
//         &g_scr_sec,
//         SCR_SEC__fsrefclk_disp_src_sel_1_0,
//         SCR_SEC__fsrefclk_disp_xtal_cg_en,
//         SCR_SEC__fsrefclk_disp_fs_rc_en,
//         SCR_SEC__fsrefclk_disp_fs_xtal_en
//     },
//     {NULL}
// };

// void fsrefclk_config()
// {
//     int32_t i = 0;

//     while (s_fsrefclk_config_tbl[i].scr_dev != NULL) {
//         res_fsrefclk_config_t* fsrefclk_config_ptr = &s_fsrefclk_config_tbl[i++];
//         scr_set(fsrefclk_config_ptr->scr_dev, &fsrefclk_config_ptr->fs_rc_en, 0);
//         scr_set(fsrefclk_config_ptr->scr_dev, &fsrefclk_config_ptr->fs_xtal_en, 0);
//         scr_set(fsrefclk_config_ptr->scr_dev, &fsrefclk_config_ptr->src_sel_1_0, 0);
//         scr_set(fsrefclk_config_ptr->scr_dev, &fsrefclk_config_ptr->fs_rc_en, 1);
//         scr_set(fsrefclk_config_ptr->scr_dev, &fsrefclk_config_ptr->fs_xtal_en, 1);
//         scr_set(fsrefclk_config_ptr->scr_dev, &fsrefclk_config_ptr->xtal_cg_en, 1);
//     }
// }

void board_scr_init()
{
//     int32_t i = 0;

//     fsrefclk_config();

//     while (s_access_ctl_tbl[i].scr_dev != NULL) {
//         res_access_ctl_t* access_ctl_ptr = &s_access_ctl_tbl[i++];

//         scr_set(access_ctl_ptr->scr_dev
//             , &access_ctl_ptr->signal
//             , access_ctl_ptr->access_mode);
//     }

    /*  need to disable ospi remap(which is done by DIL) befrore ospi training
        because ospi "training" use direct read */
    scr_signal_t ospi1_remap_en = SCR_SAFETY__remap_ospi1_remap_en;
    scr_set(&g_scr_safety, &ospi1_remap_en, 0);
}

#endif
