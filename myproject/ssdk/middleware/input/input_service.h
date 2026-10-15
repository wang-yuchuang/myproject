/**
 * @file input_service.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __INPUT_SERVICE_H__
#define __INPUT_SERVICE_H__
/* =========================== include file session ======================== */
#include <compiler.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */
#define TS_MSG_EXT_FLG              0x40000000       /**< support 30 external event max */
#define TS_REPORT_DO_DEFAULT_FLG    0x80000000

#define TS_ANRDOID_MAIN_MBOX_ADDR  (0x70)
#define TS_ANRDOID_AUX1_MBOX_ADDR  (0x71)
#define TS_ANRDOID_AUX2_MBOX_ADDR  (0x72)
#define TS_ANRDOID_AUX3_MBOX_ADDR  (0x73)

#define TS_COORD_METADATA_SIZE sizeof(struct touch_coord_data)
/* =========================== enumeration define session ================== */
/** ts msg type enum */
typedef enum {
    TS_MSG_COORD_UPDATE,
    TS_MSG_TS_IOCTL,
    TS_MSG_REGISTER_CB,
    TS_MSG_TS_EXIT,

    /** extern msg type */
    TS_MSG_AVM_START    = TS_MSG_EXT_FLG | (1 << 0),
    TS_MSG_AVM_EXIT     = TS_MSG_EXT_FLG | (1 << 1),
    TS_MSG_TRANS_TO_AP1 = TS_MSG_EXT_FLG | (1 << 2),
    TS_MSG_TRANS_TO_AP2 = TS_MSG_EXT_FLG | (1 << 3),
    TS_MSG_SHARE        = TS_MSG_EXT_FLG | (1 << 4),
}ts_msg_enmu_t;

/** ts function domain type */
typedef enum {
    TS_DO_ANRDOID_MAIN,     /**< AP1 INFOTAINMENT */
    TS_DO_ANRDOID_AUX1,     /**< AP1 ENTERTAINMENT */
    TS_DO_ANRDOID_AUX2,
    TS_DO_ANRDOID_AUX3,
    TS_DO_AP2_MAIN,         /**< AP2 INFOTAINMENT */
    TS_DO_AP2_AUX1,         /**< AP2 ENTERTAINMENT */

    TS_DO_CTRLPANEL_MAIN,   /**< MP CTRLPANEL */
    TS_DO_CTRLPANEL_AUX1,
    TS_DO_FAST_AVM,         /**< MP INFOTAINMENT */
    TS_DO_MAX,
}ts_domain_type_enum_t;

#define TS_SUPPORT_ANRDOID_MAIN   (1<<TS_DO_ANRDOID_MAIN)
#define TS_SUPPORT_ANRDOID_AUX1   (1<<TS_DO_ANRDOID_AUX1)
#define TS_SUPPORT_ANRDOID_AUX2   (1<<TS_DO_ANRDOID_AUX2)
#define TS_SUPPORT_ANRDOID_AUX3   (1<<TS_DO_ANRDOID_AUX3)
#define TS_SUPPORT_AP2_MAIN       (1<<TS_DO_AP2_MAIN)
#define TS_SUPPORT_AP2_AUX1       (1<<TS_DO_AP2_AUX1)
#define TS_SUPPORT_CTRLPANEL_MAIN (1<<TS_DO_CTRLPANEL_MAIN)
#define TS_SUPPORT_CTRLPANEL_AUX1 (1<<TS_DO_CTRLPANEL_AUX1)

/* =========================== structure define session ==================== */
/** touch region structure */
struct xy_range_info {
    uint16_t x_max;
    uint16_t y_max;
    int16_t x_offset;
    int16_t y_offset;
};
/** touch service config for screen and aribitration */
typedef struct {
    int32_t screen_id;
    int32_t ts_domain;
    struct xy_range_info xy_region;
    int32_t arbitrate;                  /**< aribitration for same touch data report to different ts domain */
    int32_t pri;                        /**< priority reserved for future */
}ts_report_config_t;

/** report touch data structure */
//  __PACKED
struct touch_coord_data {
    uint8_t id;
    uint16_t x;
    uint16_t y;
    uint16_t w;
};
typedef struct {
    uint8_t key_value;
    uint8_t touch_num;
    struct touch_coord_data coord_data[10];
}ts_touch_report_data_t;

/* Let these definition compatiable with another OS side */
enum sts_op_type {
    STS_OP_GET_CONFIG,
    STS_OP_SET_INITED,
};

/* Do not exceed 16 bytes so far */
struct sts_ioctl_cmd {
    uint16_t op;
    uint16_t instance;
    union {
        struct {
            uint16_t what;
            uint16_t why;
            uint32_t how;
        } s;
    } msg;
};

/* Do not exceed 16 bytes so far */
struct sts_ioctl_result {
    uint16_t op;
    uint16_t instance;
    union {
        /** used for get_config */
        struct {
            uint16_t version;
            uint16_t id;
            uint16_t vendor;

            uint16_t abs_x_max;
            uint16_t abs_y_max;
        } cfg;
        uint8_t data[12];
    } msg;
};
/* =========================== typedef define session ====================== */
/** local module register for tsdomain data */
typedef int32_t (*ts_callback)(int32_t tsDomain, ts_touch_report_data_t* repData, void* usrCtx);

/* =========================== function declare session ==================== */
/**
 * @brief start touch service, report touch data to local or remote function domain
 *
 * @param repCfg[IN] report aribitration config table
 * @param tblSize[IN] report aribitration config table size
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 falied
 */
int32_t touch_service_start(ts_report_config_t* repCfg, int32_t tblSize);

/**
 * @brief stop touch service
 *
 * @return int32_t
 */
int32_t touch_service_stop();

/**
 * @brief register callback for specified touch service function domain,
 *  ref @ts_domain_type_enum_t enum, one tsDomain type, only support one callback
 *  function register.
 *
 * @param tsDomain[IN] touch service function domain ref to @ts_domain_type_enum_t
 * @param cb[IN] callback function for touch data
 * @param usrCtx[IN] usr context
 * @return int32_t
 */
int32_t touch_service_register_domain_callback(ts_domain_type_enum_t tsDomain
                                                , ts_callback cb
                                                , void* usrCtx);

/**
 * @brief send msg to touch service
 *
 * @param msgType ref to @ts_msg_enmu_t,
 * @return int32_t
 */
int32_t touch_service_send_msg(int32_t msgType);

/**
 * @brief check specified tsDomain touch region is pressed or not
 *
 * @param tsDomain[IN] touch service function domain ref to @ts_domain_type_enum_t
 * @return true pressed
 * @return false no pressed
 */
bool touch_service_ts_domain_is_pressed(ts_domain_type_enum_t tsDomain);

/**
 * @brief get current tsDomain touch coord point info, position x and position y
 *
 * @param x[OUT] position x
 * @param y[OUT] position y
 * @param tsDomain[IN] touch service function domain ref to @ts_domain_type_enum_t
 * @return int32_t
 */
int32_t touch_service_ts_domain_get_point(int32_t* x, int32_t* y, ts_domain_type_enum_t tsDomain);

void set_avm_cast_enable(bool enabled);

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __INPUT_SERVICE_H__ */
/* end of header file input_service.h */
