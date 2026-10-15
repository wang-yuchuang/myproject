/*
 * property.h
 *
 * Copyright (c) 2019 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * property library heafile
 *
 * Revision History:
 * -----------------
 */

#ifndef __SDRV_PROPERTY_H__
#define __SDRV_PROPERTY_H__

#include <cmsis_os2.h>

/* property flags
 * INITD: inited for local property
 * LOCAL: maintained in this domain, orelse in other domain specified by rproc
 * RO:    readonly once initialized for first time
 * WO:    writeonly
 */
#define SYS_PROP_F_INITD           (1)
#define SYS_PROP_F_WR              (2)
#define SYS_PROP_F_RO              (4)
#define SYS_PROP_F_WO              (8)
#define PROPERTY_EVENT_COMPLETE    (1)

typedef void (*system_on_changed_cb)(int, int, int, void*);
typedef void (*property_on_changed_cb)(int, void*, void*, void*, void*);

#ifndef _def_domain_cpu_id_t
#define _def_domain_cpu_id_t
typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
#if (defined ULINK_SIDE_A) || (defined ULINK_SIDE_B)
    R_DP_CR5_SAF = DP_CPU_MAX,
    R_DP_CR5_SEC,
    R_DP_CR5_MPC,
    R_DP_CA_AP1,
    R_DP_CA_AP2,
    R_DP_DSP_V,
    ALL_DP_CPU_MAX,
#endif
} domain_cpu_id_t;
#endif

typedef enum {
    DMP_ID_DDR_STATUS  = 0,
    DMP_ID_PORT_STATUS = 1,
    DMP_ID_PLL_CLK_STATUS  = 2,
    DMP_ID_HANDOVER_STATUS = 3,
    DMP_ID_DC_STATUS   = 4,
    DMP_ID_VDSP_STATUS = 5,
    DMP_ID_VPU_STATUS  = 6,
    DMP_ID_I2C_STATUS   = 7,
    DMP_ID_REBOOT_STATUS   = 8,
    DMP_ID_AVM_STATUS = 9,
    DMP_ID_BA_STATUS = 10,
    DMP_ID_DMS_STATUS = 11,
    DMP_ID_CLUSTER_STATUS = 12,
    DMP_ID_CP_STATUS = 13,
    DMP_ID_RVM_STATUS = 14,
    DMP_ID_UPDATE_STATUS = 15,
    DMP_ID_ANDROID_BOOT_STATUS = 16,
    DMP_ID_CONTROLPANEL_STATUS = 17,
    DMP_ID_CVBS_STATUS = 18,
    DMP_ID_RECOVERY_STATUS = 19,
    DMP_ID_TOUCH_STATUS = 20,
    DMP_ID_AVM_CAST_STATUS = 21,
    DMP_ID_OTA_STATUS = 22,
    DMP_ID_MAX
} domain_property_id_t;

#define PROP_CB_MAX 10
/*
 * Here defines property area
 */
struct sys_property_value {
    char* val;
    char* name;
    int id; // 键值，始终会顺序增加。
    int flags;
    int owner;
    int fllower[10]; // DP_CPU_MAX

    void *callback[PROP_CB_MAX];
    void *usr_data[PROP_CB_MAX];
    int cb_flag[PROP_CB_MAX];
    osEventFlagsId_t event;
};

struct sys_prop_store {
    int index;
    const char *key;
    bool need_store;
};

int sdrv_property_get(int property_id, void* name, void* val);
// property_id < 0 add item
// 0 <=property_id < list_max update
int sdrv_property_set(int property_id, void* name, void* val);
int sdrv_property_init(struct sys_property_value *property_table, int num);
int sdrv_property_wait_condition(int property_id, void* name, void* expect);
int sdrv_property_wait_timeout(int property_id, void* name, void* expect, uint32_t ms);
int sdrv_property_observe(int property_id, void* name, property_on_changed_cb cb, void *data);

// 旧接口实现
int system_property_get(int property_id, int* val);
int system_property_set(int property_id, int val);
int system_property_wait_condition(int property_id, int expect);
int system_property_wait_timeout(int property_id, int expect, uint32_t ms);
int system_property_observe(int property_id, system_on_changed_cb cb, void *data);
inline int start_property_service(struct sys_property_value *property_table, int num)
{
    return sdrv_property_init(property_table, num);
}

#endif //__SDRV_PROPERTY_H__
