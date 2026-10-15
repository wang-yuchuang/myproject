/**
 * @file buffer.h
 * @brief display subsystem header file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __UTIL_FORMAT_H
#define __UTIL_FORMAT_H

extern bool noblock;

struct color_rgb24 {
    unsigned int value:24;
} __attribute__((__packed__));

enum util_yuv_order {
	YUV_YCbCr = 1,
	YUV_YCrCb = 2,
	YUV_YC = 4,
	YUV_CY = 8,
};

struct util_yuv_info {
	enum util_yuv_order order;
	unsigned int xsub;
	unsigned int ysub;
	unsigned int chroma_stride;
};

struct util_color_component {
    unsigned int length;
    unsigned int offset;
};

struct util_rgb_info {
    struct util_color_component red;
    struct util_color_component green;
    struct util_color_component blue;
    struct util_color_component alpha;
};

struct util_format_info {
    uint32_t format;
    const char *name;
    const struct util_rgb_info rgb;
    const struct util_yuv_info yuv;
};

struct bo {
    int fmt;
    int src_width;
    int src_height;
    int dst_width;
    int dst_height;
    void * addr[4];
    int bpp;
    int stride;
    int start_x;
    int start_y;
};

const struct util_format_info *util_format_info_find(uint32_t format);

void fill_smpte(const struct util_format_info *info, void *planes[3],
               unsigned int width, unsigned int height,
               unsigned int stride);

int bo_init(struct bo *bo);
void bo_free(struct bo *bo);
void dshow_post(int display_id, struct bo *bo);

#endif //__UTIL_FORMAT_H
