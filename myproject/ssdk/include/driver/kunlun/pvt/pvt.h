/**
 * @file pvt.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:pvt device API header file
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_PVT_H
#define INCLUDE_PVT_H
#include <cmsis_os2.h>
#include <types.h>

/** enum typedef section */
typedef enum pvt_alarm_mode pvt_alarm_mode_enum_t;
typedef enum pvt_alarm_type pvt_alarm_type_enum_t;
typedef enum pvt_integer_data_get pvt_integer_get_enum_t;
typedef enum pvt_integer_data_set pvt_integer_set_enum_t;
typedef enum pvt_float_data_get pvt_float_get_enum_t;

/** structrue typedef section */
/* pvt device typedef */
typedef struct pvt_dev pvt_dev_t;
typedef struct pvt_alarm_para pvt_alarm_para_t;
typedef struct pvt_data pvt_data_t;

/**
 * @brief pvt operation structure
 *
 */
typedef struct tag_pvt_ops {
    int32_t (*init)(pvt_dev_t* dev);
    int32_t (*deinit)(pvt_dev_t* dev);
    int32_t (*set_alarm)(pvt_dev_t* dev
                         , pvt_alarm_mode_enum_t mode
                         , pvt_alarm_type_enum_t type
                         , const pvt_alarm_para_t* para);
    int32_t (*clr_alarm)(pvt_dev_t* dev, int32_t chanId);
    int32_t (*enable_alarm)(pvt_dev_t* dev, int32_t chanId);
    int32_t (*disable_alarm)(pvt_dev_t* dev, int32_t chanId);
    int32_t (*get_pvt)(pvt_dev_t* dev, pvt_data_t* pvtData);
    int32_t (*integer_data_get)(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t chanId, int32_t* data);
    int32_t (*float_data_get)(pvt_dev_t* dev, pvt_float_get_enum_t type, int32_t chanId, float* data);
    int32_t (*integer_data_set)(pvt_dev_t* dev, pvt_integer_set_enum_t type, int32_t data);
}pvt_ops_t;

/**
 * @brief pvt device
 *
 */
struct pvt_dev {
    paddr_t base;
    bool is_init;
    pvt_ops_t* ops;
    osMutexId_t mutex;
    void* priv;
};



/** function declare section */

/**
 * @brief pvt init
 *
 * @param dev[in] pvt device
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_init(pvt_dev_t* dev);

/**
 * @brief pvt deinit
 *
 * @param dev[in] pvt device
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_deinit(pvt_dev_t* dev);

/**
 * @brief set pvt alarm with type and callback, The alarm notification will not
 * take effect immediately. invoker need to call the @pvt_enable_alarm function
 * after get the valid channel id to enbale alarm notice.
 *
 * @param dev[in] pvt device
 * @param mode[in] alarm mode, temperature or voltage
 * @param type[in] alarm type, hyst high/low or hsyt raising/falling
 * @param para[in] alarm config parameter, include physical high/low value,
 * callback function and cb para
 * @return int32_t channel id
 * @retval >=0 sucess, valid channel id
 * @retval -1 failed
 */
int32_t pvt_set_alarm(pvt_dev_t* dev
                      , pvt_alarm_mode_enum_t mode
                      , pvt_alarm_type_enum_t type
                      , const pvt_alarm_para_t* para);

/**
 * @brief clear pvt alarm channel info
 *
 * @param dev[in] pvt device
 * @param chanId[in] channel id which is the retval of @pvt_set_alarm
 *  function
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_clr_alarm(pvt_dev_t* dev, int32_t chanId);

/**
 * @brief enable pvt alarm notice, need invoke this function after
 * @pvt_set_alarm or while the alarm has occurred and is restored already, need
 * enable alarm again.
 *
 * @param dev[in] pvt device
 * @param chanId[in] channel id which is the retval of @pvt_set_alarm
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_enable_alarm(pvt_dev_t* dev, int32_t chanId);

/**
 * @brief disalbe pvt alarm notice, but Keep alarm configuration information.
 *
 * @param dev[in] pvt device
 * @param chanId[in] channel id which is the retval of @pvt_set_alarm
 * @return int32_t
 */
int32_t pvt_disable_alarm(pvt_dev_t* dev, int32_t chanId);

/**
 * @brief get pvt data, only can invoke this function while no alarm
 * information is set
 *
 * @param dev[in] pvt device
 * @param pvtData[out] pvt data structure pointer for out data
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_get_pvt(pvt_dev_t* dev, pvt_data_t* pvtData);

/**
 * @brief get integer data about pvt, data type ref @pvt_integer_get_enum_t
 *
 * @param dev[in] pvt device
 * @param type[in] data type
 * @param chanId[in] alarm channel id, some data type need assign channel id,
 * if not, set it to -1 while invoke this func
 * @param data[out] data pointer for out data
 * @return int32_t
 * @retval 0 sucess
 * @retval 1 failed
 */
int32_t pvt_integer_data_get(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t chanId, int32_t* data);

/**
 * @brief get flaot data about pvt, data type ref @pvt_float_get_enum_t
 *
 * @param dev[in] pvt device
 * @param type[in] data type
 * @param chanId[in] alarm channel id, some data type need assign channel id,
 * if not, set it to -1 while invoke this func
 * @param data[out] data pointer for out data
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_float_data_get(pvt_dev_t* dev, pvt_float_get_enum_t type, int32_t chanId, float* data);

/**
 * @brief set integer data about pvt, data type ref @pvt_integer_set_enum_t
 *
 * @param dev[in] pvt device
 * @param type[in] data type
 * @param data[in] data value
 * @return int32_t
 * @retval 0 sucess
 * @retval -1 failed
 */
int32_t pvt_integer_data_set(pvt_dev_t* dev, pvt_integer_get_enum_t type, int32_t data);

#endif /* INCLUDE_PVT_H */
/* end of header file pvt.h */