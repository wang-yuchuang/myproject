/**
 * @file g2dlite.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __G2D_DEMO_H
#define __G2D_DEMO_H


unsigned long blend_gpipe_bypass(u32 fmt, u32 w, u32 h, void *addr);
unsigned long yuv_to_rgb(u32 fmt, u32 w, u32 h, void *addr);
unsigned long yuv_to_rgb_cpu(u32 fmt, u32 w, u32 h, void *addr);
unsigned long blend_2_pipe(u32 fmt, u32 w, u32 h, void *addr);
unsigned long scaler_gpipe(u32 fmt, u32 w, u32 h, void *addr);
unsigned long rotation_test(u32 fmt, u32 w, u32 h, void *addr);
unsigned long fill_rect(u32 fmt, u32 w, u32 h, void *addr);
unsigned long fastcopy(u32 fmt, u32 w, u32 h, void *addr);
unsigned long porter_duff(u32 fmt, u32 w, u32 h, void *addr);
unsigned long yuv_to_yuv(u32 fmt, u32 w, u32 h, void *addr);
#endif //__G2D_DEMO_H