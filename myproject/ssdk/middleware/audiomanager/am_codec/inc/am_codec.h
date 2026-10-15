/**
 * @file am_codec.h
 * @brief audiomanager codec header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AM_CODEC_H__
#define __AM_CODEC_H__

#include <cmsis_os2.h>
#include <lib/list.h>
#include <stdio.h>

#include "am_debug.h"

typedef struct am_codec am_codec_t;
struct am_board;

#define CODEC_NAME_MAX_LEN 32

typedef bool (*ctl_put)(am_codec_t *codec, unsigned int ctl, unsigned int val);
typedef bool (*ctl_get)(am_codec_t *codec, unsigned int ctl, unsigned int *val);
typedef bool (*writeable_reg_t)(am_codec_t *codec, unsigned int reg);
typedef bool (*readable_reg_t)(am_codec_t *codec, unsigned int reg);

typedef enum {
    PROTOCOL_TYPE_UNKNOWN = 0,
    PROTOCOL_TYPE_I2C,
    PROTOCOL_TYPE_SPI,
    PROTOCOL_TYPE_GPIO,
    PROTOCOL_TYPE_NUMB,
} codec_protocol_t;

typedef enum {
    CTL_NORMAL_TYPE = 0,
    CTL_GAIN_TYPE = 1,
    CTL_COEFF_TYPE = 2,
    CTL_GPIO_TYPE = 3,
} AM_CTL_TYPE;

typedef enum {
    CACHE_INVALID = 0,
    CACHE_CLEAN = 1,
    CACHE_DIRTY = 2,
} AM_REG_CACHE_STATUS;

typedef struct {
    unsigned int ctl_id;
    unsigned int ctl_reg;
    unsigned int ctl_mask;
    unsigned int ctl_ref;
    unsigned int ctl_type;
    ctl_put put;
    ctl_get get;
} am_ctl_t;

typedef struct {
    unsigned int reg_addr;
    unsigned int value;
    unsigned int status;
} am_reg_t;

typedef struct am_codec_ops {
    bool (*initialize)(am_codec_t *codec);
    bool (*read_reg)(am_codec_t *codec, unsigned int reg, unsigned int *val);
    bool (*write_reg)(am_codec_t *codec, unsigned int reg, unsigned int val);
    bool (*burn_fw)(am_codec_t *codec, int fw_no);
    bool (*send_param)(am_codec_t *codec, int mode, int param_size,
                       unsigned char *param);
    bool (*query_param)(am_codec_t *codec, int mode, int param_size,
                        unsigned char *param);
    bool (*set_cofficient)(am_codec_t *codec, int mode, int param_size,
                           unsigned char *param);
    bool (*writeable_reg)(am_codec_t *codec, unsigned int reg);
    bool (*readable_reg)(am_codec_t *codec, unsigned int reg);
} am_codec_ops_t;

struct am_codec {
    char name[32];
    am_reg_t *am_regs;
    am_ctl_t *am_ctls;
    unsigned int regs_numb;
    unsigned int ctls_numb;
    struct list_node node;
    am_codec_ops_t *ops;
    void *driver_data;
};

am_ctl_t *am_codec_find_virctl(am_codec_t *codec, unsigned int id);
bool am_codec_set_virctl_handler(am_codec_t *codec, unsigned int ctl_name,
                                 ctl_put put, ctl_get get);
bool am_codec_virctl_find_run_put(am_codec_t *codec, unsigned int id,
                                  unsigned int val);
bool am_codec_virctl_find_run_get(am_codec_t *codec, unsigned int id,
                                  unsigned int *val);
am_codec_t *am_codec_create(const char *codec_name, am_codec_ops_t *codec_ops,
                            am_reg_t *am_regs, unsigned int regs_numb,
                            am_ctl_t *am_ctls, unsigned int ctls_numb);
void am_codec_destroy(am_codec_t *codec);

inline void am_codec_set_drvdata(am_codec_t *codec, void *drv_data)
{
    if (!codec) {
        printf("%s err codec is null \r\n", __FUNCTION__);
        return;
    }

    codec->driver_data = drv_data;
}

inline void *am_codec_get_drvdata(am_codec_t *codec)
{

    if (!codec) {
        printf("%s err codec is null \r\n", __FUNCTION__);
        return NULL;
    }

    return codec->driver_data;
}

#endif
