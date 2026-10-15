/**
 * @file au_swvol.h
 * @brief audio software volume header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AUDIO_SWVOL_H__
#define __AUDIO_SWVOL_H__

#include <stdint.h>
#include <stdlib.h>

#define MAX_VOL_MAXVALUE 100
#define MAX_VOL_MINVALUE 0
#define MAX_VOL_ARRAY_SIZE 101
#define Q14 14
#define QX Q14

#define MAX_16BIT (32767)
#define MIN_16BIT (-32768)

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

int32_t au_swvol_apply_s16(uint8_t *out, uint8_t *in, int32_t sample_count,
                           int16_t vol);

#endif