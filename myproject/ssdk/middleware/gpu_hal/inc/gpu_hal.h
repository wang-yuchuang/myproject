/*
* g2d_hal.h
*
* Copyright (c) 2012 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 2022/06/01  create this file
*/
#ifndef __G2D_HAL_H__
#define __G2D_HAL_H__

#include <types.h>
#include <stdbool.h>
#include "driver/kunlun/g2dlite/g2dlite.h"

#ifdef CONFIG_ASW

typedef enum {
    ASW_TRIANGLE_TOP_LEFT,
    ASW_TRIANGLE_BOTTOM_LEFT,
    ASW_TRIANGLE_TOP_RIGHT,
    ASW_TRIANGLE_BOTTOM_RIGHT,
} asw_trianle_type;

#endif

void hal_g2dlite_init(void **handle);
void hal_g2dlite_update(void *handle, struct g2dlite_input *input);
void hal_g2dlite_blend(void *handle, struct g2dlite_input *input);
void hal_g2dlite_scaler(void *handle, struct g2dlite_input *input);
void hal_g2dlite_csc(void *handle, struct g2dlite_input *input);
void hal_g2dlite_rotaion(void *handle, struct g2dlite_input *input);
void hal_g2dlite_fill_rect(void *handle, u32 color, u8 g_alpha,
    addr_t aaddr, u8 bpa, u32 astride, struct g2dlite_output_cfg *output);

/**
 *@in:
 *@@addr:input buffer address
 *@@width@height: input buffer size = in->width * in->height (uint is Word)
 *@@stride:input buffer pitch (uint is Byte)
 *@out:
 *@@addr:out buffer address
 *@@width@height: input buffer size = out->width * out->height (uint is Word)
 *@@stride:out buffer pitch (uint is Byte), must align to 128 bits
 */
void hal_g2dlite_fastcopy(void *handle, struct fcopy_t *in,
    struct fcopy_t *out);
void hal_g2dlite_clut_setting(void *handle, char *clut_table);
#endif //__G2D_HAL_H__
