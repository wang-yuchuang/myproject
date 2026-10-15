/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __VPU_PROTO_TOOL_H__
#define __VPU_PROTO_TOOL_H__

#include "vpu.pb-c.h"
#include "vpu_types.h"

#define VPU_CODA988_IDX 0
#define VPU_WAVE412_IDX 1

#define MAX_GOP_NUM 8
#define MAX_GDI_IDX 30

#define DISPLAY_IDX_FLAG_SEQ_END -1
#define DISPLAY_IDX_FLAG_NO_FB -3
#define DECODED_IDX_FLAG_NO_FB -1
#define DECODED_IDX_FLAG_SKIP -2

#ifdef SUPPORT_ROI_50
#define MAX_ROI_NUMBER 50
#else
#define MAX_ROI_NUMBER 10
#endif

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif

#define PUT_BYTE(_p, _b) *_p++ = (unsigned char)_b;

#define PUT_BUFFER(_p, _buf, _len)                                                                 \
    osal_memcpy(_p, _buf, _len);                                                                   \
    _p += _len;

#define PUT_LE32(_p, _var)                                                                         \
    *_p++ = (unsigned char)((_var) >> 0);                                                          \
    *_p++ = (unsigned char)((_var) >> 8);                                                          \
    *_p++ = (unsigned char)((_var) >> 16);                                                         \
    *_p++ = (unsigned char)((_var) >> 24);

#define PUT_BE32(_p, _var)                                                                         \
    *_p++ = (unsigned char)((_var) >> 24);                                                         \
    *_p++ = (unsigned char)((_var) >> 16);                                                         \
    *_p++ = (unsigned char)((_var) >> 8);                                                          \
    *_p++ = (unsigned char)((_var) >> 0);

#define PUT_LE16(_p, _var)                                                                         \
    *_p++ = (unsigned char)((_var) >> 0);                                                          \
    *_p++ = (unsigned char)((_var) >> 8);

#define PUT_BE16(_p, _var)                                                                         \
    *_p++ = (unsigned char)((_var) >> 8);                                                          \
    *_p++ = (unsigned char)((_var) >> 0);

typedef enum {
    STD_AVC,
    STD_VC1,
    STD_MPEG2,
    STD_MPEG4,
    STD_H263,
    STD_DIV3,
    STD_RV,
    STD_AVS,
    STD_THO = 9,
    STD_VP3,
    STD_VP8,
    STD_HEVC,
    STD_VP9,
    STD_MAX
} CodStd;

typedef enum {
    NOT_PACKED = 0,
    PACKED_YUYV,
    PACKED_YVYU,
    PACKED_UYVY,
    PACKED_VYUY,
} PackedFormatNum;

typedef enum {
    VP9_NONE,
    VP9_SAMESCALE, //(1 << 0)
    VP9_DOWNSCALE, //(1 << 1)
} VP9ScaleBit;

/**
 * @brief  This is an enumeration type for representing product IDs.
 */
typedef enum {
    PRODUCT_ID_980,
    PRODUCT_ID_960 = 1,
    PRODUCT_ID_950 = 1, // same with CODA960
    PRODUCT_ID_7503,
    PRODUCT_ID_320,
    PRODUCT_ID_410,
    PRODUCT_ID_4102,
    PRODUCT_ID_420,
    PRODUCT_ID_412,
    PRODUCT_ID_7Q,
    PRODUCT_ID_420L,
    PRODUCT_ID_510,
    PRODUCT_ID_512,
    PRODUCT_ID_420SN,
    PRODUCT_ID_420DU,
    PRODUCT_ID_NONE,
} ProductId;

typedef enum {
    VDI_LITTLE_ENDIAN = 0, /* 64bit LE */
    VDI_BIG_ENDIAN,        /* 64bit BE */
    VDI_32BIT_LITTLE_ENDIAN,
    VDI_32BIT_BIG_ENDIAN,
    /* WAVE410 ONLY */
    VDI_128BIT_BIG_ENDIAN = 16,
    VDI_128BIT_BE_BYTE_SWAP,
    VDI_128BIT_BE_WORD_SWAP,
    VDI_128BIT_BE_WORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_SWAP,
    VDI_128BIT_BE_DWORD_BYTE_SWAP,
    VDI_128BIT_BE_DWORD_WORD_SWAP,
    VDI_128BIT_BE_DWORD_WORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_WORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_WORD_SWAP,
    VDI_128BIT_LE_DWORD_BYTE_SWAP,
    VDI_128BIT_LE_DWORD_SWAP,
    VDI_128BIT_LE_WORD_BYTE_SWAP,
    VDI_128BIT_LE_WORD_SWAP,
    VDI_128BIT_LE_BYTE_SWAP,
    VDI_128BIT_LITTLE_ENDIAN = 31,
    VDI_ENDIAN_MAX
} EndianMode;

/**
* @brief    This is an enumeration for declaring a type of framebuffer that is allocated when
VPU_DecAllocateFrameBuffer() and VPU_EncAllocateFrameBuffer() function call.
*/
typedef enum {
    FB_TYPE_CODEC, /**< A framebuffer type used for decoding or encoding  */
    FB_TYPE_PPU,   /**< A framebuffer type used for post processor  */
} FramebufferAllocType;

/**
 * @brief    This is an enumeration type for representing picture types.
 */
typedef enum {
    PIC_TYPE_I = 0,      /**< I picture */
    PIC_TYPE_P = 1,      /**< P picture */
    PIC_TYPE_B = 2,      /**< B picture (except VC1) */
    PIC_TYPE_VC1_BI = 2, /**< VC1 BI picture (VC1 only) */
    PIC_TYPE_VC1_B = 3,  /**< VC1 B picture (VC1 only) */
    PIC_TYPE_D = 3, /**< D picture in MPEG2 that is only composed of DC coefficients (MPEG2 only) */
    PIC_TYPE_S =
        3, /**< S picture in MPEG4 that is an acronym of Sprite and used for GMC (MPEG4 only)*/
    PIC_TYPE_VC1_P_SKIP = 4,           /**< VC1 P skip picture (VC1 only) */
    PIC_TYPE_MP4_P_SKIP_NOT_CODED = 4, /**< Not Coded P Picture in mpeg4 packed mode */
    PIC_TYPE_IDR = 5,                  /**< H.264/H.265 IDR picture */
    PIC_TYPE_MAX                       /**< No Meaning */
} PicType;

/**
 * @brief    This is an enumeration type for representing interrupt bit positions for CODA series.
 */
typedef enum {
    INT_BIT_INIT = 0,
    INT_BIT_SEQ_INIT = 1,
    INT_BIT_SEQ_END = 2,
    INT_BIT_PIC_RUN = 3,
    INT_BIT_FRAMEBUF_SET = 4,
    INT_BIT_ENC_HEADER = 5,
    INT_BIT_DEC_PARA_SET = 7,
    INT_BIT_DEC_BUF_FLUSH = 8,
    INT_BIT_USERDATA = 9,
    INT_BIT_DEC_FIELD = 10,
    INT_BIT_DEC_MB_ROWS = 13,
    INT_BIT_BIT_BUF_EMPTY = 14,
    INT_BIT_BIT_BUF_FULL = 15
} InterruptBit;

/**
 * @brief   This is an enumeration type for representing bitstream handling modes in decoder.
 */
typedef enum {
    BS_MODE_INTERRUPT, /**< API returns right after buffer empty and the video decoder is waiting
                          for more data(bitstream). */
    BS_MODE_RESERVED,  /**< Reserved for the future */
    BS_MODE_PIC_END,   /**< API returns after the video decoder finishes error concealment for the
                          rest of the frame. */
} BitStreamMode;

/**
 * @brief    This is an enumeration type for specifying frame buffer types when tiled2linear or
 * wtlEnable is used.
 */
typedef enum {
    FF_NONE = 0,  /**< Frame buffer type when tiled2linear or wtlEnable is disabled */
    FF_FRAME = 1, /**< Frame buffer type to store one frame */
    FF_FIELD = 2, /**< Frame buffer type to store top field or bottom field separately */
} FrameFlag;

/**
 * @brief   This is an enumeration type for representing the way of writing chroma data in planar
 * format of frame buffer.
 */
typedef enum {
    CBCR_ORDER_NORMAL,  /**< Cb data are written in Cb buffer, and Cr data are written in Cr buffer.
                         */
    CBCR_ORDER_REVERSED /**< Cr data are written in Cb buffer, and Cb data are written in Cr buffer.
                         */
} CbCrOrder;

/************************************************************************/
/* Error codes                                                          */
/************************************************************************/

/**
* @brief    This is an enumeration for declaring return codes from API function calls.
The meaning of each return code is the same for all of the API functions, but the reasons of
non-successful return might be different. Some details of those reasons will be
briefly described in the API definition chapter. In this chapter, the basic meaning
of each return code will be presented.
*/
typedef enum {
    RETCODE_SUCCESS,                    /**< This means that operation was done successfully.  */  /* 0  */
    RETCODE_FAILURE,                    /**< This means that operation was not done successfully. When un-recoverable decoder error happens such as header parsing errors, this value will be returned from VPU API.  */
    RETCODE_INVALID_HANDLE,             /**< This means that the given handle for current API function call was invalid (for example, not initialized yet, improper function call for the given handle, etc.).  */
    RETCODE_INVALID_PARAM,              /**< This means that the given argument parameters (for example, input data structure) was invalid (not initialized yet or not valid anymore). */
    RETCODE_INVALID_COMMAND,            /**< This means that the given command was invalid (for example, undefined, or not allowed in the given instances).  */
    RETCODE_ROTATOR_OUTPUT_NOT_SET,     /**< This means that rotator output buffer was not allocated even though postprocessor (rotation, mirroring, or deringing) is enabled. */  /* 5  */
    RETCODE_ROTATOR_STRIDE_NOT_SET,     /**< This means that rotator stride was not provided even though postprocessor (rotation, mirroring, or deringing) is enabled.  */
    RETCODE_FRAME_NOT_COMPLETE,         /**< This means that frame decoding operation was not completed yet, so the given API function call cannot be allowed.  */
    RETCODE_INVALID_FRAME_BUFFER,       /**< This means that the given source frame buffer pointers were invalid in encoder (not initialized yet or not valid anymore).  */
    RETCODE_INSUFFICIENT_FRAME_BUFFERS, /**< This means that the given numbers of frame buffers were not enough for the operations of the given handle. This return code will only be received when calling VPU_DecRegisterFrameBuffer() or VPU_EncRegisterFrameBuffer() function. */
    RETCODE_INVALID_STRIDE,             /**< This means that the given stride was invalid (for example, 0, not a multiple of 8 or smaller than picture size). This return code is only allowed in API functions which set stride.  */   /* 10 */
    RETCODE_WRONG_CALL_SEQUENCE,        /**< This means that current API function call was invalid considering the allowed sequences between API functions (for example, missing one crucial function call before this function call).  */
    RETCODE_CALLED_BEFORE,              /**< This means that multiple calls of current API function for a given instance are invalid. */
    RETCODE_NOT_INITIALIZED,            /**< This means that VPU was not initialized yet. Before calling any API functions, the initialization API function, VPU_Init(), should be called at the beginning.  */
    RETCODE_USERDATA_BUF_NOT_SET,       /**< This means that there is no memory allocation for reporting userdata. Before setting user data enable, user data buffer address and size should be set with valid value. */
    RETCODE_MEMORY_ACCESS_VIOLATION,    /**< This means that access violation to the protected memory has been occurred. */   /* 15 */
    RETCODE_VPU_RESPONSE_TIMEOUT,       /**< This means that VPU response time is too long, time out. */
    RETCODE_INSUFFICIENT_RESOURCE,      /**< This means that VPU cannot allocate memory due to lack of memory. */
    RETCODE_NOT_FOUND_BITCODE_PATH,     /**< This means that BIT_CODE_FILE_PATH has a wrong firmware path or firmware size is 0 when calling VPU_InitWithBitcode() function.  */
    RETCODE_NOT_SUPPORTED_FEATURE,      /**< This means that host application uses an API option that is not supported in current hardware.  */
    RETCODE_NOT_FOUND_VPU_DEVICE,       /**< This means that host application uses the undefined product ID. */   /* 20 */
    RETCODE_CP0_EXCEPTION,              /**< This means that coprocessor exception has occurred. (WAVE only) */
    RETCODE_STREAM_BUF_FULL,            /**< This means that stream buffer is full in encoder. */
    RETCODE_ACCESS_VIOLATION_HW,        /**< This means that GDI access error has occurred. It might come from violation of write protection region or spec-out GDI read/write request. (WAVE only) */
    RETCODE_QUERY_FAILURE,              /**< This means that query command was not successful (WAVE5 only) */
    RETCODE_QUEUEING_FAILURE,           /**< This means that commands cannot be queued (WAVE5 only) */
    RETCODE_RESOURECE_CONSTRAINTS,      /**< This means that vpu does not have enough resources to create new instance> */
} RetCode;

/**
 * @brief   This is an enumeration type for representing chroma formats of the frame buffer and
 * pixel formats in packed mode.
 */
typedef enum {
    FORMAT_420 = 0, /* 8bit */
    FORMAT_422,     /* 8bit */
    FORMAT_224,     /* 8bit */
    FORMAT_444,     /* 8bit */
    FORMAT_400,     /* 8bit */

    /* Little Endian Perspective     */
    /*     | addr 0  | addr 1  |     */
    FORMAT_420_P10_16BIT_MSB = 5, /* lsb | 00xxxxx |xxxxxxxx | msb */
    FORMAT_420_P10_16BIT_LSB,     /* lsb | xxxxxxx |xxxxxx00 | msb */
    FORMAT_420_P10_32BIT_MSB,     /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */
    FORMAT_420_P10_32BIT_LSB,     /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */

    /* 4:2:2 packed format */
    /* Little Endian Perspective     */
    /*     | addr 0  | addr 1  |     */
    FORMAT_422_P10_16BIT_MSB, /* lsb | 00xxxxx |xxxxxxxx | msb */
    FORMAT_422_P10_16BIT_LSB, /* lsb | xxxxxxx |xxxxxx00 | msb */
    FORMAT_422_P10_32BIT_MSB, /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */
    FORMAT_422_P10_32BIT_LSB, /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */

    FORMAT_YUYV, /**< 8bit packed format : Y0U0Y1V0 Y2U1Y3V1 ... */
    FORMAT_YUYV_P10_16BIT_MSB,
    /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(YUYV) format(1Pixel=2Byte) */
    FORMAT_YUYV_P10_16BIT_LSB,
    /* lsb | xxxxxxxxxx000000 | msb */ /**< 10bit packed(YUYV) format(1Pixel=2Byte) */
    FORMAT_YUYV_P10_32BIT_MSB,
    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(YUYV) format(3Pixel=4Byte) */
    FORMAT_YUYV_P10_32BIT_LSB,
    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(YUYV) format(3Pixel=4Byte) */

    FORMAT_YVYU, /**< 8bit packed format : Y0V0Y1U0 Y2V1Y3U1 ... */
    FORMAT_YVYU_P10_16BIT_MSB,
    /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(YVYU) format(1Pixel=2Byte) */
    FORMAT_YVYU_P10_16BIT_LSB,
    /* lsb | xxxxxxxxxx000000 | msb */ /**< 10bit packed(YVYU) format(1Pixel=2Byte) */
    FORMAT_YVYU_P10_32BIT_MSB,
    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(YVYU) format(3Pixel=4Byte) */
    FORMAT_YVYU_P10_32BIT_LSB,
    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(YVYU) format(3Pixel=4Byte) */

    FORMAT_UYVY, /**< 8bit packed format : U0Y0V0Y1 U1Y2V1Y3 ... */
    FORMAT_UYVY_P10_16BIT_MSB,
    /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(UYVY) format(1Pixel=2Byte) */
    FORMAT_UYVY_P10_16BIT_LSB,
    /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(UYVY) format(1Pixel=2Byte) */
    FORMAT_UYVY_P10_32BIT_MSB,
    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(UYVY) format(3Pixel=4Byte) */
    FORMAT_UYVY_P10_32BIT_LSB,
    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(UYVY) format(3Pixel=4Byte) */

    FORMAT_VYUY, /**< 8bit packed format : V0Y0U0Y1 V1Y2U1Y3 ... */
    FORMAT_VYUY_P10_16BIT_MSB,
    /* lsb | 000000xxxxxxxxxx | msb */ /**< 10bit packed(VYUY) format(1Pixel=2Byte) */
    FORMAT_VYUY_P10_16BIT_LSB,
    /* lsb | xxxxxxxxxx000000 | msb */ /**< 10bit packed(VYUY) format(1Pixel=2Byte) */
    FORMAT_VYUY_P10_32BIT_MSB,
    /* lsb |00xxxxxxxxxxxxxxxxxxxxxxxxxxx| msb */ /**< 10bit packed(VYUY) format(3Pixel=4Byte) */
    FORMAT_VYUY_P10_32BIT_LSB,
    /* lsb |xxxxxxxxxxxxxxxxxxxxxxxxxxx00| msb */ /**< 10bit packed(VYUY) format(3Pixel=4Byte) */

    FORMAT_MAX,
} FrameBufferFormat;

/**
 * @brief This is an enumeration type for representing map types for frame buffer.
 */
typedef enum {
    /**
    @verbatim
    Linear frame map type

    NOTE: Products earlier than CODA9 can only set this linear map type.
    @endverbatim
    */
    LINEAR_FRAME_MAP = 0,
    TILED_FRAME_V_MAP = 1,         /**< Tiled frame vertical map type */
    TILED_FRAME_H_MAP = 2,         /**< Tiled frame horizontal map type */
    TILED_FIELD_V_MAP = 3,         /**< Tiled mixed field vertical map type */
    TILED_MIXED_V_MAP = 4,         /**< Tiled mixed field vertical map type */
    TILED_FRAME_MB_RASTER_MAP = 5, /**< Tiled frame horizontal map type */
    TILED_FIELD_MB_RASTER_MAP = 6, /**< Tiled frame horizontal map type */
    TILED_FRAME_NO_BANK_MAP = 7,
    /**< Tiled frame no bank map. WAVE4 and CODA7L do not support tiledmap type for frame buffer. */ // coda980 only
    TILED_FIELD_NO_BANK_MAP = 8,
    /**< Tiled field no bank map. WAVE4 and CODA7L do not support tiledmap type for frame buffer. */ // coda980 only
    LINEAR_FIELD_MAP = 9,
    /**< Linear field map type. WAVE4 and CODA7L do not support tiledmap type for frame buffer. */ // coda980 only
    CODA_TILED_MAP_TYPE_MAX = 10,
    COMPRESSED_FRAME_MAP = 10,
    /**< Compressed framebuffer type */ // WAVE4 only
    ARM_COMPRESSED_FRAME_MAP = 12,
    /**< AFBC(ARM Frame Buffer Compression) compressed frame map type */ // AFBC enabled WAVE412
                                                                         // only
    PVRIC_COMPRESSED_FRAME_MAP = 14,
    /**< PVRIC FBC compressed frame map type */ // pvricFbcEnabled WAVE412 only
    TILED_MAP_TYPE_MAX
} TiledMapType;

/************************************************************************/
/* Utility macros                                                       */
/************************************************************************/
#define VPU_ALIGN4(_x) (((_x) + 0x03) & ~0x03)
#define VPU_ALIGN8(_x) (((_x) + 0x07) & ~0x07)
#define VPU_ALIGN16(_x) (((_x) + 0x0f) & ~0x0f)
#define VPU_ALIGN32(_x) (((_x) + 0x1f) & ~0x1f)
#define VPU_ALIGN64(_x) (((_x) + 0x3f) & ~0x3f)
#define VPU_ALIGN128(_x) (((_x) + 0x7f) & ~0x7f)
#define VPU_ALIGN256(_x) (((_x) + 0xff) & ~0xff)
#define VPU_ALIGN512(_x) (((_x) + 0x1ff) & ~0x1ff)
#define VPU_ALIGN4096(_x) (((_x) + 0xfff) & ~0xfff)
#define VPU_ALIGN16384(_x) (((_x) + 0x3fff) & ~0x3fff)

Int32 vpu_proto_calc_stride(Uint32 width, Uint32 height, FrameBufferFormat format,
                            BOOL cbcrInterleave, TiledMapType mapType, VP9ScaleBit vp9Scale);

Int32 vpu_proto_calc_luma_size(Int32 productId, Int32 stride, Int32 height,
                               FrameBufferFormat format, BOOL cbcrIntl, TiledMapType mapType);

Int32 vpu_proto_calc_chroma_size(Int32 productId, Int32 stride, Int32 height,
                                 FrameBufferFormat format, BOOL cbcrIntl, TiledMapType mapType);

Int32 vpu_proto_calc_frame_size(Int32 productId, Int32 stride, Int32 height, TiledMapType mapType,
                                FrameBufferFormat format, BOOL interleave);

#endif //__VPU_PROTO_TOOL_H__