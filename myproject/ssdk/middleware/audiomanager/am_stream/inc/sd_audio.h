/**
 * @file sd_audio.h
 * @brief audio param header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __SD_AUDIO_H__
#define __SD_AUDIO_H__

/// sample width
enum sd_audioflow_sample_width_t {
    SD_AUDIO_SAMPLE_WIDTH_NO_INIT =
        0,                        ///< audio sample resolution not initialized
    SD_AUDIO_SAMPLE_WIDTH_8BITS,  ///< set sample width to 8bits
    SD_AUDIO_SAMPLE_WIDTH_16BITS, ///< set sample width to 16bits
    SD_AUDIO_SAMPLE_WIDTH_20BITS, ///< set sample width to 20bits
    SD_AUDIO_SAMPLE_WIDTH_24BITS, ///< set sample width to 24bits
    SD_AUDIO_SAMPLE_WIDTH_32BITS, ///< set sample width to 32bits
};

#endif