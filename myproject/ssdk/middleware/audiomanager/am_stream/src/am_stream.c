/**
 * @file am_stream.c
 * @brief audio manager stream source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "am_stream.h"

#include "au_rpc_svc.h"

static am_stream_t *am_stream_get_instance(am_stream_dev_t *dev,
                                           uint8_t stream_id)
{
    if (!dev) {
        return NULL;
    }

    return (am_stream_t *)(dev->stream + stream_id);
}

static int am_stream_get_ctrl_info(am_stream_ctrl_t *ctrl,
                                   am_stream_t *am_stream, am_stream_cmd_t cmd)
{
    if (!ctrl || !am_stream) {
        return -1;
    }

    if (AM_STREAM_REQUEST == cmd) {
        ctrl->op_code = OP_PCM_STREAM_REQ;

        am_stream->buffer_ptr++;
        /* cyclic to first */
        if (am_stream->buffer_ptr >= am_stream->stream_param.period_count) {
            am_stream->buffer_ptr = 0;
        }
        ctrl->buffer_tail_ptr = am_stream->buffer_ptr;
    } else {
        ctrl->op_code = OP_PCM_STREAM_IND;
        ctrl->buffer_tail_ptr = am_stream->buffer_ptr;
    }

    ctrl->stream_cmd = cmd;
    ctrl->stream_id = am_stream->stream_id;
    ctrl->stream_type = am_stream->stream_type;

    return 0;
}

static int am_stream_request(am_stream_dev_t *dev, uint8_t stream_id)
{
    am_stream_ctrl_t stream_ctrl = {0};
    am_stream_ops_t *ops = NULL;
    am_stream_t *stream = NULL;
    struct am_stream_ipcc_data ipcc_msg = {0};
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    am_stream_get_ctrl_info(&stream_ctrl, stream, AM_STREAM_REQUEST);
    ipcc_msg.cmd = OP_PCM_STREAM_REQ;
    ipcc_msg.data_len = sizeof(am_stream_ctrl_t);
    memcpy(&ipcc_msg.data[0], &stream_ctrl, ipcc_msg.data_len);

    osMutexAcquire(dev->am_dev_mutex,osWaitForever);
    if (ops && ops->send_msg) {
        ret = ops->send_msg(stream_id, (char *)&ipcc_msg,
                      sizeof(struct am_stream_ipcc_data));
    }
    osMutexRelease(dev->am_dev_mutex);

    return ret;
}

static int am_stream_set_hw_pararms(am_stream_dev_t *dev, uint8_t stream_id,
                                    uint8_t type, am_stream_param_t param,
                                    am_stream_mode_t mode)
{
    am_stream_t *stream = NULL;
    am_stream_ops_t *ops = NULL;
    am_stream_rpmsg_info_t am_stream_rpmsg = {0};
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    if (mode == AM_STREAM_MODE_SERVER) {
        stream->stream_id = stream_id;
        stream->stream_type = type;
        stream->buffer_ptr = 0;
        stream->buffer_size = param.period_size * param.period_count;

        memcpy(&stream->stream_param, &param, sizeof(am_stream_param_t));

        if (ops && ops->open) {
            ret = ops->open(dev, stream_id, (void *)stream);
        }
    } else {
        am_stream_get_ctrl_info(&am_stream_rpmsg.stream_ctrl, stream,
                                AM_STREAM_SET_PARAMS);
        memcpy(&am_stream_rpmsg.stream_params, &param,
               sizeof(am_stream_param_t));

        if (ops && ops->send_msg) {
            ret = ops->send_msg(stream_id, (char *)&am_stream_rpmsg,
                        sizeof(am_stream_rpmsg_info_t));
        }
    }

    return ret;
}

static int am_stream_start(am_stream_dev_t *dev, uint8_t stream_id,
                           am_stream_mode_t mode)
{
    am_stream_t *stream = NULL;
    am_stream_ops_t *ops = NULL;
    am_stream_ctrl_t stream_ctrl = {0};
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    if (mode == AM_STREAM_MODE_SERVER) {
        if (ops && ops->start) {
            ret = ops->start(dev, stream_id, (void *)stream);
        }
    } else {
        am_stream_get_ctrl_info(&stream_ctrl, stream, AM_STREAM_START);
        if (ops && ops->send_msg) {
            ret = ops->send_msg(stream_id, (char *)&stream_ctrl,
                          sizeof(am_stream_ctrl_t));
        }
    }

    return ret;
}

static int am_stream_stop(am_stream_dev_t *dev, uint8_t stream_id,
                          am_stream_mode_t mode)
{
    am_stream_t *stream = NULL;
    am_stream_ctrl_t stream_ctrl = {0};
    am_stream_ops_t *ops = NULL;
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    if (mode == AM_STREAM_MODE_SERVER) {
        if (ops && ops->stop) {
            ret = ops->stop(dev, stream_id, (void *)stream);
        }
    } else {
        am_stream_get_ctrl_info(&stream_ctrl, stream, AM_STREAM_STOP);
        if (ops && ops->send_msg) {
            ret = ops->send_msg(stream_id, (char *)&stream_ctrl,
                          sizeof(am_stream_ctrl_t));
        }
    }

    return ret;
}

static int am_stream_pause(am_stream_dev_t *dev, uint8_t stream_id,
                           am_stream_mode_t mode)
{
    am_stream_t *stream = NULL;
    am_stream_ctrl_t stream_ctrl = {0};
    am_stream_ops_t *ops = NULL;
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    if (mode == AM_STREAM_MODE_SERVER) {
        if (ops && ops->pause) {
            ret = ops->pause(dev, stream_id, (void *)stream);
        }
    } else {
        am_stream_get_ctrl_info(&stream_ctrl, stream, AM_STREAM_PAUSE);
        if (ops && ops->send_msg) {
            ret = ops->send_msg(stream_id, (char *)&stream_ctrl,
                          sizeof(am_stream_ctrl_t));
        }
    }

    return ret;
}

static int am_stream_supend(am_stream_dev_t *dev, uint8_t stream_id,
                            am_stream_mode_t mode)
{
    am_stream_ctrl_t stream_ctrl = {0};
    am_stream_t *stream = NULL;
    am_stream_ops_t *ops = NULL;
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    if (mode == AM_STREAM_MODE_SERVER) {
        if (ops && ops->suspend) {
            ret = ops->suspend(dev, stream_id, (void *)stream);
        }
    } else {
        am_stream_get_ctrl_info(&stream_ctrl, stream, AM_STREAM_SUSPEND);
        if (ops && ops->send_msg) {
            ret = ops->send_msg(stream_id, (char *)&stream_ctrl,
                          sizeof(am_stream_ctrl_t));
        }
    }

    return ret;
}

static int am_stream_resume(am_stream_dev_t *dev, uint8_t stream_id,
                            am_stream_mode_t mode)
{
    am_stream_ctrl_t stream_ctrl = {0};
    am_stream_t *stream = am_stream_get_instance(dev, stream_id);
    am_stream_ops_t *ops = NULL;
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    if (mode == AM_STREAM_MODE_SERVER) {
        if (ops && ops->resume) {
            ret = ops->resume(dev, stream_id, (void *)stream);
        }
    } else {
        am_stream_get_ctrl_info(&stream_ctrl, stream, AM_STREAM_RESUME);
        if (ops && ops->send_msg) {
            ret = ops->send_msg(stream_id, (char *)&stream_ctrl,
                          sizeof(am_stream_ctrl_t));
        }
    }

    return ret;
}

int am_stream_response(am_stream_dev_t *dev, uint8_t stream_id, uint8_t cmd,int ret_code)
{
    am_stream_ctrl_t stream_ctrl = {0};
    am_stream_ops_t *ops = NULL;
    am_stream_t *stream = NULL;
    struct am_stream_ipcc_data ipcc_msg = {0};
    int ret = 0;

    if ((dev == NULL) || (stream_id >= dev->stream_cnt * ID_NUM_FOR_STREAM)) {
        return -1;
    }

    ops = &dev->ops;
    stream = am_stream_get_instance(dev, stream_id);

    am_stream_get_ctrl_info(&stream_ctrl, stream, cmd);
    ipcc_msg.cmd = OP_PCM_STREAM_IND;
    if (ret_code != 0) /* occur fail */
        ipcc_msg.cmd |= AM_STREAM_CMD_ERR_MASK;
    ipcc_msg.data_len = sizeof(am_stream_ctrl_t);
    memcpy(&ipcc_msg.data[0], &stream_ctrl, ipcc_msg.data_len);

    osMutexAcquire(dev->am_dev_mutex,osWaitForever);
    if (ops && ops->send_msg) {
        ret = ops->send_msg(stream_id, (char *)&ipcc_msg,
                      sizeof(struct am_stream_ipcc_data));
    }
    osMutexRelease(dev->am_dev_mutex);

    return ret;
}

int am_stream_do_rpc_message(am_stream_dev_t *dev, am_stream_cmd_t cmd,
                             uint8_t stream_id, void *pack_data,
                             am_stream_mode_t mode)
{
    int ret = 0;
    am_stream_ctrl_t *am_ctrl = NULL;
    am_stream_param_t *am_param = NULL;

    switch (cmd) {
    case AM_STREAM_START:
        ret = am_stream_start(dev, stream_id, mode);
        break;
    case AM_STREAM_STOP:
        ret = am_stream_stop(dev, stream_id, mode);
        break;
    case AM_STREAM_PAUSE:
        ret = am_stream_pause(dev, stream_id, mode);
        break;
    case AM_STREAM_SET_PARAMS:
        am_ctrl = (am_stream_ctrl_t *)pack_data;
        am_param =
            (am_stream_param_t *)((char *)pack_data + sizeof(am_stream_ctrl_t));

        ret = am_stream_set_hw_pararms(dev, stream_id, am_ctrl->stream_type,
                                       *am_param, mode);
        break;
    case AM_STREAM_SUSPEND:
        ret = am_stream_supend(dev, stream_id, mode);
        break;
    case AM_STREAM_RESUME:
        ret = am_stream_resume(dev, stream_id, mode);
        break;
    case AM_STREAM_REQUEST:
        ret = am_stream_request(dev, stream_id);
        return ret;
    case AM_STREAM_BEAT:
        break;
    default:
        break;
    }

    am_stream_response(dev, stream_id, cmd, ret);

    return ret;
}

int am_stream_register_platform(am_stream_dev_t *dev)
{
    int ret = 0;
    am_stream_t *am_ptr = NULL;

    if (!dev) {
        return -1;
    }

    dev->am_dev_mutex = osMutexNew(NULL);

    am_ptr = malloc(dev->stream_cnt * ID_NUM_FOR_STREAM * sizeof(am_stream_t));

    if (am_ptr == NULL) {
        return -1;
    }

    dev->stream = am_ptr;

    if (dev->ops.init) {
        ret = dev->ops.init(dev);
    }

    return ret;
}

int am_stream_destory(am_stream_dev_t *dev)
{
    int ret = 0;

    if (!dev) {
        return -1;
    }

    if (dev->ops.deinit) {
        ret = dev->ops.deinit(dev);
    }

    if (dev->stream) {
        free(dev->stream);
    }

    return ret;
}