/*
 * sd_errcode.h
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * errcode library head file
 *
 * Revision History:
 * -----------------
 */

#ifndef _SD_ERRCODE_H_
#define _SD_ERRCODE_H_

#include <sys/types.h>
#include "errcode.h"

/* 
 * Storage device index
 */
typedef enum {
    STR_ID_INVALID = -1,
    STR_ID_OSPI1  = 0,
    STR_ID_OSPI2 = 1,
    STR_ID_EMMC1  = 2,
    STR_ID_EMMC2 = 3,
    STR_ID_SOCKET = 4,
    STR_ID_MAX
} storage_id_t;

typedef struct {
    mod_id_t mod_id;
    int mod_no;
    uint16_t error_code;
    uint32_t user_data;
} errcode_t;

#define ERRCODE_SUPPORT_OSPI       (1 << 0)
#define ERRCODE_SUPPORT_EMMC       (1 << 1)
#define ERRCODE_SUPPORT_NET        (1 << 2)

#define ERRCODE_ITEM(_mod_id, _mod_no, _error_code, _user_data)  \
    {   \
    .mod_id = _mod_id,    \
    .mod_no = _mod_no,  \
    .error_code = _error_code, \
    .user_data = _user_data, \
    }

#define ERRCODE_PRINT(_mod_id, _mod_no, _error_code, _user_data)  \
    do{ errcode_t error_code = ERRCODE_ITEM(_mod_id, _mod_no, _error_code, _user_data);  \
    errcode_set(&error_code);}while(0)

/**
 * @brief errcode early init
 *
 * @return int
 * @retval ERRCODE_OK success
 * @retval ERRCODE_NG failed
 */
int system_errcode_early_init(void);

/**
 * @brief errcode init
 *
 * @return int
 * @retval ERRCODE_OK success
 * @retval ERRCODE_NG failed
 */
int system_errcode_init(void);

/**
 * @brief errcode deinit
 *
 * @return int
 * @retval ERRCODE_OK success
 * @retval ERRCODE_NG failed
 */
int system_errcode_deinit(void);

/**
 * @brief set an errcode
 * @param errcode[IN] an errcode info
 * @return ERR_HANDLE
 */
ERR_HANDLE errcode_set(errcode_t* errcode);

/**
 * @brief set an errcode and specify storage device
 * @param errcode[IN] an errcode info
 * @param strid[IN] storage device index
 * @return ERR_HANDLE
 */
ERR_HANDLE errcode_set_by_strid(errcode_t* errcode, storage_id_t strid);

/**
 * @brief transfer an errcode to remote server
 * @param ip[IN] remote server ip
 * @param errcode[IN] an errcode info
 * @return int
 * @retval ERRCODE_OK success
 * @retval ERRCODE_NG failed
 */
int errcode_transfer(const char* ip, errcode_t* errcode);

/**
 * @brief dump all errcode
 * @param strid[IN] storage device index
 * @param bufsize[IN] buffer size
 * @param buf[out] storage dump errcode
 * @return int
 * @retval >=0 dump errcode number
 * @retval -1 failed
 */
int errcode_dump(storage_id_t strid, char* buf, int bufsize);

/**
 * @brief clear all errcode
 * @param strid[IN] storage device index
 * @return int
 * @retval ERRCODE_OK success
 * @retval ERRCODE_NG failed
 */
int errcode_clear(storage_id_t strid);

/**
 * @brief set storage device
 * @param strid[IN] storage device index
 * @return int
 * @retval ERRCODE_OK success
 * @retval ERRCODE_NG failed
 */
int system_errcode_set_storage_id(storage_id_t strid);

/**
 * @brief get current storage device
 * @return int storage device index
 */
int system_errcode_get_storage_id();
#endif //_SD_ERRCODE_H_