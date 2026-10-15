/*
* g2dlite_api.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 07/28/2020 BI create this file
*/

#ifndef __G2DLITE_API_H__
#define __G2DLITE_API_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <types.h>
#include <stdbool.h>
#include "driver/taishan/g2dlite/g2dlite.h"


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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif //__G2DLITE_API_H__
