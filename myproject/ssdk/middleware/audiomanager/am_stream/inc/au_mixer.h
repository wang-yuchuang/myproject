/**
 * @file au_mix.h
 * @brief audio mix header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __AUDIO_MIXER_H__
#define __AUDIO_MIXER_H__

#include <stdint.h>
#include <stdlib.h>

#include "au_pcm_stream.h"
#include "au_source.h"

/**
 * @brief au_mixer_init
 *
 * @return true
 * @return false
 */
bool au_mixer_init(void);
/**
 * @brief au_playback_open
 *
 * @param vol
 * @param source
 * @return pcm_stream_handle_t
 */
pcm_stream_handle_t au_playback_open(uint16_t vol, au_source_t *source);

/**
 * @brief au_playback_close
 *
 * @param handle
 * @return int
 */
int au_playback_close(pcm_stream_handle_t handle);

/**
 * @brief au_playback_start
 *
 * @param handle
 * @return int
 */
int au_playback_start(pcm_stream_handle_t handle);

/**
 * @brief au_playback_stop
 *
 * @param handle
 * @return int
 */
int au_playback_stop(pcm_stream_handle_t handle);

/**
 * @brief au_playback_pause
 *
 * @param handle
 * @return int
 */
int au_playback_pause(pcm_stream_handle_t handle);

/**
 * @brief au_playback_resume
 *
 * @param handle
 * @return int
 */
int au_playback_resume(pcm_stream_handle_t handle);

/**
 * @brief au_playback_get_vol
 *
 * @param handle
 * @return int
 */
int au_playback_get_vol(pcm_stream_handle_t handle);

/**
 * @brief au_playback_set_vol
 *
 * @param handle
 * @param vol
 * @return int
 */
int au_playback_set_vol(pcm_stream_handle_t handle, uint16_t vol);

/**
 * @brief au_playback_write
 *
 * @param handle
 * @param in
 * @param size
 * @return int
 */
int au_playback_write(pcm_stream_handle_t handle, uint8_t *in, int32_t size);

#endif