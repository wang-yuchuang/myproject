/**
 * @file am_api.h
 * @author shao yi
 * @brief define audio manager's api
 * @version 0.1
 * @date 2021-01-11
 *
 * @copyright Copyright (c) 2021 Semidrive Semiconductor
 *
 */

#ifndef __AM_API_H__
#define __AM_API_H__

#include "errcode.h"

/**
 * @brief am_init
 *
 * @return int
 */
int am_init(void);

/**
 * @brief am_start_path
 *
 * @param path_name
 * @param vol
 * @return AM_RESULT
 */
AM_RESULT am_start_path(const char *path_name, int vol);

/**
 * @brief am_stop_path
 *
 * @param path_name
 * @return AM_RESULT
 */
AM_RESULT am_stop_path(const char *path_name);

/**
 * @brief am_switch_path
 *
 * @param from_path_name
 * @param to_path_name
 * @return AM_RESULT
 */
AM_RESULT am_switch_path(const char *from_path_name, const char *to_path_name);

/**
 * @brief am_mute_path
 *
 * @param path_name
 * @param mute
 * @return AM_RESULT
 */
AM_RESULT am_mute_path(const char *path_name, bool mute);

/**
 * @brief am_set_path_vol
 *
 * @param path_name
 * @param volume
 * @return AM_RESULT
 */
AM_RESULT am_set_path_vol(const char *path_name, int volume);

/**
 * @brief am_get_path_stat
 *
 * @param path_name
 * @param active
 * @return AM_RESULT
 */
AM_RESULT am_get_path_stat(const char *path_name, bool *active);

/**
 * @brief am_get_path_vol
 *
 * @param path_name
 * @param vol
 * @return AM_RESULT
 */
AM_RESULT am_get_path_vol(const char *path_name, unsigned int *vol);

/**
 * @brief am_get_path_mute_stat
 *
 * @param path_name
 * @param mute
 * @return AM_RESULT
 */
AM_RESULT am_get_path_mute_stat(const char *path_name, bool *mute);

/**
 * @brief
 *
 * @param path_name
 * @param mode
 * @param param_size
 * @param param
 * @return AM_RESULT
 */
AM_RESULT am_send_path_param(const char *path_name, int mode, int param_size,
                             unsigned char *param);
/**
 * @brief am_query_path_param
 *
 * @param path_name
 * @param mode
 * @param param_size
 * @param param
 * @return AM_RESULT
 */
AM_RESULT am_query_path_param(const char *path_name, int mode, int param_size,
                              unsigned char *param);

/**
 * @brief am_suspend
 *
 */
void am_suspend(void *);

/**
 * @brief am_resume
 *
 */
void am_resume(void *);

#endif
