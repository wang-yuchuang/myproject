/**
 * @file au_capture.h
 * @brief audio capture header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AUDIO_CAPTURE_H__
#define __AUDIO_CAPTURE_H__

#include <stdint.h>
#include <stdlib.h>

#include "au_pcm_stream.h"
#include "au_source.h"

/**
 * @brief au_capture_init
 *
 * @return true
 * @return false
 */
bool au_capture_init(void);
/**
 * @brief au_capture_open
 *
 * @param vol
 * @param source
 * @return pcm_stream_handle_t
 */
pcm_stream_handle_t au_capture_open(uint16_t vol, au_source_t *source);
/**
 * @brief au_capture_close
 *
 * @param handle
 * @return int
 */
int au_capture_close(pcm_stream_handle_t handle);

/**
 * @brief au_capture_start
 *
 * @param handle
 * @return int
 */
int au_capture_start(pcm_stream_handle_t handle);

/**
 * @brief au_capture_stop
 *
 * @param handle
 * @return int
 */
int au_capture_stop(pcm_stream_handle_t handle);

/**
 * @brief au_capture_pause
 *
 * @param handle
 * @return int
 */
int au_capture_pause(pcm_stream_handle_t handle);

/**
 * @brief au_capture_resume
 *
 * @param handle
 * @return int
 */
int au_capture_resume(pcm_stream_handle_t handle);

/**
 * @brief au_capture_read
 *
 * @param handle
 * @param in
 * @param size
 * @return int
 */
int au_capture_read(pcm_stream_handle_t handle, uint8_t *out, int32_t size);

#endif
