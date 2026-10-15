/**
 * @file au_pcm_stream.h
 * @brief audio pcm stream header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AUDIO_PCM_STREAM__
#define __AUDIO_PCM_STREAM__

#include <cmsis_os2.h>
#include <lib/list.h>
#include <stdint.h>
#include <stdlib.h>

#include "au_source.h"
#include "ringbuffer.h"

#define PCM_STREAM_MINORS 16
#define MGR_PCM_STREAMS 0

typedef enum {
    AU_PCM_STREAM_ST_OPENED = 1,
    AU_PCM_STREAM_ST_IDLE = 2,
    AU_PCM_STREAM_ST_RUNNING = 3,
    AU_PCM_STREAM_ST_PAUSED = 4,
    AU_PCM_STREAM_ST_STOPPED = 5,
} au_pcm_stream_status_t;

typedef enum {
    AU_PCM_STREAM_OP_NONE = 0,
    AU_PCM_STREAM_OP_WAKE = 1,
} au_pcm_stream_op_type;

typedef struct au_pcm_stream *pcm_stream_handle_t;
typedef int (*pcm_stream_callback)(pcm_stream_handle_t pcm_stream, uint8_t *out,
                                   int32_t size);

typedef struct au_pcm_stream {
    int id;
    uint16_t vol;
    osEventFlagsId_t pcm_stream_sync;
    int16_t result_sync;
    au_pcm_stream_status_t status;
    int32_t size;
    ringbuf_t rb;
    osMutexId_t pcm_stream_mutex;
    uint8_t pcm_stream_type;
    struct list_node mixer_node;
    struct list_node capture_node;
    struct list_node mgr_node;
    uint8_t *out_temp;
    osThreadId_t *thread;
    osSemaphoreId_t sem;
    bool running;
    bool exited;
    au_source_t source;
} au_pcm_stream_t;

typedef struct au_pcm_stream_minor {
    au_pcm_stream_t pcm_stream;
    bool allocated;
} au_pcm_stream_minor_t;

typedef struct au_pcm_stream_mgr {

    au_pcm_stream_minor_t pcm_stream_minors[PCM_STREAM_MINORS];
    osMutexId_t mgr_mutex;
} au_pcm_stream_mgr_t;

void au_pcm_streams_init(void);
au_pcm_stream_t *pcm_stream_open(au_source_t *source, int32_t size,
                                 uint16_t vol);
int pcm_stream_close(pcm_stream_handle_t pcm_stream);
int pcm_stream_write(pcm_stream_handle_t pcm_stream, uint8_t *in, int32_t size);
int pcm_stream_get_used(pcm_stream_handle_t pcm_stream);
int pcm_stream_get_available(pcm_stream_handle_t pcm_stream);
int pcm_stream_get_id(pcm_stream_handle_t pcm_stream);
int pcm_stream_read(pcm_stream_handle_t pcm_stream, uint8_t *out, int32_t size);
bool pcm_stream_is_active(pcm_stream_handle_t pcm_stream);
bool pcm_stream_is_idle(pcm_stream_handle_t pcm_stream);
void pcm_stream_post(pcm_stream_handle_t pcm_stream);
void pcm_stream_wait(pcm_stream_handle_t pcm_stream);
void pcm_stream_start(pcm_stream_handle_t pcm_stream);
void pcm_stream_stop(pcm_stream_handle_t pcm_stream);
void pcm_stream_pause(pcm_stream_handle_t pcm_stream);
void pcm_stream_resume(pcm_stream_handle_t pcm_stream);
void pcm_stream_set_vol(pcm_stream_handle_t pcm_stream, uint16_t vol);
uint16_t pcm_stream_get_vol(pcm_stream_handle_t pcm_stream);

#endif
