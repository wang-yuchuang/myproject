/**
 * @file am_stream.h
 * @brief audio manager stream header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AM_STREAM_H__
#define __AM_STREAM_H__

#include <cmsis_os2.h>
#include <stdint.h>

#define ID_NUM_FOR_STREAM 2
#define AM_IPCC_DATA_LEN_MAX 32
#define AM_STREAM_CMD_ERR_MASK 0x80

typedef enum {
    AM_STREAM_MODE_CLIENT = 0,
    AM_STREAM_MODE_SERVER = 1,
} am_stream_mode_t;

typedef enum {
    AM_STREAM_TYPE_PLAYBACK = 0,
    AM_STREAM_TYPE_CAPTURE = 1,
} am_stream_type_t;

typedef enum {
    AM_STREAM_START,
    AM_STREAM_STOP,
    AM_STREAM_PAUSE,
    AM_STREAM_SET_PARAMS,
    AM_STREAM_REQUEST,
    AM_STREAM_BEAT,
    AM_STREAM_SUSPEND,
    AM_STREAM_RESUME,
} am_stream_cmd_t;

struct am_stream_ipcc_data {
    char data[AM_IPCC_DATA_LEN_MAX];
    int32_t data_len;
    uint8_t cmd;
    uint8_t param;
    uint8_t dcf_id;
    uint8_t reserved1;
} __attribute__((packed));

typedef struct {
    uint8_t op_code;
    uint8_t stream_cmd;
    uint8_t stream_id;
    uint8_t stream_type;
    uint32_t buffer_tail_ptr;
    uint8_t  status;
    uint8_t reserved[3];
} __attribute__((packed)) am_stream_ctrl_t;

typedef struct {
    uint8_t stream_fmt;
    uint8_t stream_channels;
    uint16_t stream_rate;
    uint32_t buffer_addr;
    uint32_t period_size;
    uint32_t period_count;
} __attribute__((packed)) am_stream_param_t;

typedef struct {
    am_stream_ctrl_t stream_ctrl;
    am_stream_param_t stream_params;
} am_stream_rpmsg_info_t;

typedef struct am_stream_dev am_stream_dev_t;

typedef struct {
    int (*init)(am_stream_dev_t *dev);
    int (*open)(am_stream_dev_t *dev, uint8_t stream_id, void *param);
    int (*start)(am_stream_dev_t *dev, uint8_t stream_id, void *param);
    int (*stop)(am_stream_dev_t *dev, uint8_t stream_id, void *param);
    int (*pause)(am_stream_dev_t *dev, uint8_t stream_id, void *param);
    int (*suspend)(am_stream_dev_t *dev, uint8_t stream_id, void *param);
    int (*resume)(am_stream_dev_t *dev, uint8_t stream_id, void *param);
    int (*deinit)(am_stream_dev_t *dev);
    int (*send_msg)(int stream_index, char *msg_buf, uint16_t msg_len);
} am_stream_ops_t;

typedef struct {
    uint8_t stream_id;
    uint8_t stream_type;
    uint32_t buffer_ptr;
    uint32_t buffer_size;

    am_stream_param_t stream_param;
} am_stream_t;

struct am_stream_dev {
    uint16_t stream_cnt;
    am_stream_ops_t ops;
    am_stream_t *stream;
    osMutexId_t am_dev_mutex;
    void *private_data;
};

int am_stream_register_platform(am_stream_dev_t *dev);
int am_stream_destory(am_stream_dev_t *dev);
int am_stream_do_rpc_message(am_stream_dev_t *dev, am_stream_cmd_t cmd,
                             uint8_t stream_id, void *pack_data,
                             am_stream_mode_t mode);

#endif