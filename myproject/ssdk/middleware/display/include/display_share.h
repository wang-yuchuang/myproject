#ifndef __DISPLAY_SHARE_H__
#define __DISPLAY_SHARE_H__

#include "driver/kunlun/g2dlite/g2dlite.h"


#define CONFIG_DISP_THREAD      0   /* 1 : Digging holes with G2D  0 : hardware_layer_cast */
#define CONFIG_VSYNC_THREAD     1   /* wait for mailbox */
#define DISPLAY_ID_RPC          1   /* 0 : cluster, 1 : main screen, 2 : sub screen */
#define MAX_BUFFERS             3

#define SHARE_MASK_WIDTH        1920
#define SHARE_MASK_HEIGHT       720


#define DISP_MAILBOX_ADDR (0x90)

#define abs(x) (((x) >= 0)? (x) : (-(x)))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define fourcc_code(a, b, c, d) ((u32)(a) | ((u32)(b) << 8) | \
                 ((u32)(c) << 16) | ((u32)(d) << 24))

/* Remote IOC command */
#define DISP_CMD_START              RP_IOWR('d', 0, struct disp_frame_info)
#define DISP_CMD_SET_FRAMEINFO      RP_IOWR('d', 1, struct disp_frame_info)
#define DISP_CMD_SHARING_WITH_MASK  RP_IOWR('d', 2, struct disp_frame_info)
#define DISP_CMD_CLEAR              RP_IOWR('d', 3, struct disp_frame_info)
#define DISP_CMD_DCRECOVERY         RP_IOWR('d', 5, struct disp_recovery_info)
#define DISP_CMD_DISABLE_VSYNC      RP_IOWR('d', 6, struct disp_frame_info)
#define DISP_CMD_ENABLE_VSYNC       RP_IOWR('d', 7, struct disp_frame_info)

#define DRM_FORMAT_BIG_ENDIAN	(1<<31) /* format is big endian instead of little endian */

/* 16 bpp RGB */
#define DRM_FORMAT_ARGB4444     fourcc_code('A', 'R', '1', '2') /* [15:0] A:R:G:B 4:4:4:4 little endian */
#define DRM_FORMAT_BGRA4444     fourcc_code('B', 'A', '1', '2') /* [15:0] B:G:R:A 4:4:4:4 little endian */
#define DRM_FORMAT_ARGB1555     fourcc_code('A', 'R', '1', '5') /* [15:0] A:R:G:B 1:5:5:5 little endian */
#define DRM_FORMAT_RGB565       fourcc_code('R', 'G', '1', '6') /* [15:0] R:G:B 5:6:5 little endian */
#define DRM_FORMAT_BGR565       fourcc_code('B', 'G', '1', '6') /* [15:0] B:G:R 5:6:5 little endian */

/* 24 bpp RGB */
#define DRM_FORMAT_RGB888       fourcc_code('R', 'G', '2', '4') /* [23:0] R:G:B little endian */
#define DRM_FORMAT_BGR888       fourcc_code('B', 'G', '2', '4') /* [23:0] B:G:R little endian */

/* 32 bpp RGB */
#define DRM_FORMAT_ARGB8888     fourcc_code('A', 'R', '2', '4') /* [31:0] A:R:G:B 8:8:8:8 little endian */
#define DRM_FORMAT_ABGR8888     fourcc_code('A', 'B', '2', '4') /* [31:0] A:B:G:R 8:8:8:8 little endian */
#define DRM_FORMAT_RGBA8888     fourcc_code('R', 'A', '2', '4') /* [31:0] R:G:B:A 8:8:8:8 little endian */
#define DRM_FORMAT_BGRA8888     fourcc_code('B', 'A', '2', '4') /* [31:0] B:G:R:A 8:8:8:8 little endian */

#define DRM_FORMAT_XRGB8888     fourcc_code('X', 'R', '2', '4') /* [31:0] x:R:G:B 8:8:8:8 little endian */
#define DRM_FORMAT_XBGR8888     fourcc_code('X', 'B', '2', '4') /* [31:0] x:B:G:R 8:8:8:8 little endian */
#define DRM_FORMAT_RGBX8888     fourcc_code('R', 'X', '2', '4') /* [31:0] R:G:B:x 8:8:8:8 little endian */
#define DRM_FORMAT_BGRX8888     fourcc_code('B', 'X', '2', '4') /* [31:0] B:G:R:x 8:8:8:8 little endian */

#define DRM_FORMAT_ARGB2101010  fourcc_code('A', 'R', '3', '0') /* [31:0] A:R:G:B 2:10:10:10 little endian */
#define DRM_FORMAT_YUY2         fourcc_code('Y', 'U', 'Y', 'V') /* [31:0] YUYV little endian */
struct dc_share {
    void *g2d;
    uint32_t *mask_base;
    uint32_t *outbufs[MAX_BUFFERS];

    struct sdm_buffer bufs[MAX_BUFFERS];
    struct sdm_post_config post_configs[MAX_BUFFERS];
    struct display_server display_server;
    bool skip_post;
    bool blend_ready;
};

int sdm_post_init(struct dc_share *dc_s);
int g2d_handle_init(struct dc_share *dc_s);
void g2d_mask_blending(struct dc_share *dc_s);
int disp_ioctl_set_frameinfo(struct dc_share *dc_s, void *arg);

#endif

