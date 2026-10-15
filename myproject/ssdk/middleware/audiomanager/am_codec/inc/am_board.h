/**
 * @file am_board.h
 * @brief audiomanager board header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AM_BOARD_H__
#define __AM_BOARD_H__

#include <cmsis_os2.h>
#include <lib/list.h>
#include <stdio.h>

#include "am_codec.h"
#include "am_debug.h"
#include "am_path.h"

typedef enum {
    AM_BOARD_UNINITED = 0,
    AM_BOARD_INITED = 1,
    AM_BOARD_RESETING = 2,
    AM_BOARD_RESTORING = 3,
} am_board_status;

#define BOARD_NAME_MAX_LEN 32

typedef struct am_board am_board_t;

typedef struct am_board_ops {
    bool (*init)(am_board_t *board);
    bool (*deinit)(am_board_t *board);
    bool (*reset)(am_board_t *board);
    bool (*exception_check)(am_board_t *board);
} am_board_ops_t;

struct am_board {
    char name[BOARD_NAME_MAX_LEN];
    struct list_node codecs;
    int codec_numb;
    struct list_node paths;
    int path_numb;
    int active_path_numb;
    am_board_status status;
    osMutexId_t path_lock;
    bool deamon_on;
    am_board_ops_t *ops;
};

bool am_board_add_path(am_board_t *board, am_path_t *path);
bool am_board_remove_path(am_board_t *board, am_path_t *path);
am_path_t *am_board_find_path(am_board_t *board, const char *path_name);
bool am_board_add_codec(am_board_t *board, am_codec_t *codec);
bool am_board_remove_codec(am_board_t *board, am_codec_t *codec);
am_codec_t *am_board_find_codec(am_board_t *board, const char *name);
bool am_board_initialize(am_board_t *board);
void am_board_dump(am_board_t *board);

#endif
