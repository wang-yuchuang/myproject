/**
 * @file am_path.h
 * @brief audiomanager path header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AM_PATH_H__
#define __AM_PATH_H__

#include <stdio.h>

#include "am_codec.h"

typedef struct am_path am_path_t;
struct am_board;

#define AM_PATH_IDLE "AM_PATH_IDLE"

typedef enum {
    CTL_WRITE_CACHE = 0,
    CTL_WRITE_NOCACHE,
    CTL_DELAY,
} route_cmd_type_t;

#define INVALID_PATH_ID (0x7ff)
#define INVALID_ORDER_INDEX (INVALID_PATH_ID)

#define PATH_NAME_MAX_LEN 128

typedef enum {
    PATH_INACTIVE = 0,
    PATH_ACTIVE,
} AM_PATH_STATUS;

/**
 * @brief Audio Path attribute
 *
 */
typedef enum {
    PATH_EXCLUSIVE = 0,
    PATH_SUPPRESS_EXTRA,
    PATH_NORMAL
} AM_PATH_ATTRIBUTE;

typedef struct {
    unsigned int active;
    unsigned int order;
    unsigned int volume;
    unsigned int mute;
    unsigned int attribute;
} am_path_status_t;

typedef struct path_setting_table {
    char codec_name[CODEC_NAME_MAX_LEN];
    route_cmd_type_t cmd;
    unsigned int ctl_name;
    unsigned char ctl_value;
} path_setting_table_t;

typedef struct am_path_ops {
    bool (*enter)(am_path_t *path);
    bool (*exit)(am_path_t *path);
    bool (*get_path_vol)(am_path_t *path, unsigned int *vol);
    bool (*set_path_vol)(am_path_t *path, unsigned int vol);
    bool (*set_path_mute)(am_path_t *path, bool mute);
    bool (*get_path_mute)(am_path_t *path, bool *mute);
    bool (*send_param)(am_path_t *path, int mode, int param_size,
                       unsigned char *param);
    bool (*query_param)(am_path_t *path, int mode, int param_size,
                        unsigned char *param);
} am_path_ops_t;

struct am_path {
    char name[PATH_NAME_MAX_LEN];
    am_path_status_t status;
    struct list_node node;
    path_setting_table_t *path_table;
    unsigned int table_size;
    struct am_board *board;
    am_path_ops_t *ops;
};

#endif
