/**
 * @file au_agent.h
 * @brief audio agent header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_AGENT_H__
#define __AU_AGENT_H__

#include <cmsis_os2.h>
#include <stdint.h>
#include <stdlib.h>

#define AU_AGENT_PATH_SIZE 256

typedef enum {
    AU_AGENT_OP_NONE = 0,
    AU_AGENT_OP_START = 1,
    AU_AGENT_OP_STOP = 2,
} au_agent_op_type;

typedef enum {
    AU_AGENT_ST_IDLE = 0,
    AU_AGENT_ST_PREPARE = 1,
    AU_AGENT_ST_RUNNING = 2,
    AU_AGENT_ST_STOP = 3,
} au_agent_status;

typedef enum {
    AU_AGENT_TYPE_NONE = 0,
    AU_AGENT_TYPE_CYCLIC = 1,
    AU_AGENT_TYPE_ONCE = 2,
} au_agent_type;

/* pcm dataflow description */
typedef struct {
    char path[AU_AGENT_PATH_SIZE];
    int priority;
    au_agent_op_type op_code;
    au_agent_type work_type;
} au_agent_params_t;

typedef struct {
    au_agent_params_t *params;
    au_agent_op_type op_code;
    char path[AU_AGENT_PATH_SIZE];
    uint8_t *pcm_data;
    size_t pcm_size;
    size_t period_len;
    size_t period_count;
    int priority;
    au_agent_type work_type;
    osEventFlagsId_t agent_start_event;
    osEventFlagsId_t agent_done_event;

    osThreadId_t *thread;
    void *i2s_handle;
    struct dma_desc *desc_tx;
    struct dma_chan *dma_chan;

    au_agent_status status;
    uint32_t result_error;
} au_agent_t;

typedef void *au_agent_handle_t;

/**
 * @brief
 *
 * @return true
 * @return false
 */
bool au_agent_init(void);
/**
 * @brief
 *
 * @param handle
 * @param res_id
 * @return true
 * @return false
 */
bool au_agent_open(au_agent_handle_t *handle, uint32_t res_id);
/**
 * @brief
 *
 * @param handle
 * @return true
 * @return false
 */
bool au_agent_close(au_agent_handle_t handle);
/**
 * @brief
 *
 * @param handle
 * @param agent_params
 * @return true
 * @return false
 */
bool au_agent_operation(au_agent_handle_t handle,
                        au_agent_params_t *agent_params);

#endif
