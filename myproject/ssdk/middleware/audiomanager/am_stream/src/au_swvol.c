/**
 * @file au_swvol.c
 * @brief audio source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "au_swvol.h"

#include "math.h"

/* ausw volume */
#define AU_SWVOLUME_MINDB -30
#define AU_SWVOLUME_MAXDB +0

static int au_swvol_init(int *scaler, int mindb, int maxdb)
{
    double tabdb[MAX_VOL_ARRAY_SIZE];
    double tabf[MAX_VOL_ARRAY_SIZE];
    int z, i;
    double max_level = MAX_VOL_ARRAY_SIZE;

    for (i = 0; i < MAX_VOL_ARRAY_SIZE; i++) {
        tabdb[i] = mindb + (maxdb - mindb) * i / max_level;
        tabf[i] = pow(10.0, tabdb[i] / 20.0);
        scaler[i] = (int)((1 << QX) * tabf[i]);
    }

    z = -mindb * MAX_VOL_ARRAY_SIZE / (maxdb - mindb);
    z = MAX(z, 0);
    z = MIN(z, MAX_VOL_MAXVALUE);
    scaler[0] = 0;          /* mute */
    scaler[z] = (1 << Q14); /* 0db */
    return z;
}

const int16_t au_sw_gain[MAX_VOL_ARRAY_SIZE] = {
    0,     536,   554,   574,   594,   614,   636,   658,   681,   704,   729,
    754,   780,   808,   836,   865,   895,   926,   958,   992,   1026,  1062,
    1099,  1137,  1177,  1218,  1260,  1304,  1349,  1396,  1445,  1495,  1547,
    1601,  1657,  1714,  1774,  1836,  1900,  1966,  2034,  2105,  2178,  2254,
    2332,  2413,  2497,  2584,  2674,  2767,  2864,  2963,  3066,  3173,  3283,
    3398,  3516,  3638,  3765,  3896,  4031,  4172,  4317,  4467,  4622,  4783,
    4950,  5122,  5300,  5484,  5675,  5873,  6077,  6288,  6507,  6734,  6968,
    7210,  7461,  7721,  7989,  8267,  8555,  8853,  9161,  9479,  9809,  10150,
    10503, 10869, 11247, 11638, 12043, 12462, 12896, 13344, 13809, 14289, 14786,
    15300, 16384,
};

int16_t au_swvol_voltogain(int vol)
{
    int16_t gain = 0;

    if (vol > 100) {
        ssdk_printf(SSDK_ERR, "%s: err vol > 100\n", __func__);
        vol = 100;
    }

    if (vol < 0) {
        ssdk_printf(SSDK_ERR, "%s: err vol < 0 \n", __func__);
        vol = 0;
    }

    gain = au_sw_gain[vol];
    return gain;
}

int32_t au_swvol_apply_s16(uint8_t *out, uint8_t *in, int32_t sample_count,
                           int16_t vol)
{
    int16_t input;
    int32_t sample;
    int16_t *pIn = (int16_t *)in;
    int16_t *pOut = (int16_t *)out;
    int16_t mixed = 0;
    int i = 0;
    int16_t gain = au_swvol_voltogain(vol);

    for (; i < sample_count; i++) {
        input = *pIn++;
        sample = (int32_t)input * gain;
        sample = sample >> QX;

        if (sample > MAX_16BIT)
            sample = MAX_16BIT;
        else if (sample < MIN_16BIT)
            sample = MIN_16BIT;

        *pOut++ += (int16_t)sample;
    }

    return sample_count;
}
