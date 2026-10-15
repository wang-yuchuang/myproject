/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include "vpu_proto_tool.h"
#include <debug.h>

Int32 vpu_proto_calc_stride(Uint32 width, Uint32 height, FrameBufferFormat format,
                            BOOL cbcrInterleave, TiledMapType mapType, VP9ScaleBit vp9Scale)
{
    Uint32 lumaStride = 0;
    Uint32 chromaStride = 0;

    lumaStride = VPU_ALIGN32(width);

    if (height > width) {
        if ((mapType >= TILED_FRAME_V_MAP && mapType <= TILED_MIXED_V_MAP) ||
            mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP)
            width = VPU_ALIGN16(height); // TiledMap constraints
    }

    if (mapType == LINEAR_FRAME_MAP || mapType == LINEAR_FIELD_MAP) {
        Uint32 twice = 0;

        twice = cbcrInterleave == TRUE ? 2 : 1;
        switch (format) {
        case FORMAT_420:
            /* nothing to do */
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_LSB:
            lumaStride = VPU_ALIGN32(width) * 2;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
            if (vp9Scale & VP9_SAMESCALE || vp9Scale & VP9_DOWNSCALE) {
                lumaStride = VPU_ALIGN32(((width + 11) / 12) * 16);
                chromaStride = (((width / 2) + 11) * twice / 12) * 16;
            } else {
                width = VPU_ALIGN32(width);
                lumaStride = ((VPU_ALIGN16(width) + 11) / 12) * 16;
                chromaStride = ((VPU_ALIGN16(width / 2) + 11) * twice / 12) * 16;
                // if ( isWAVE410 == TRUE ) {
                if ((chromaStride * 2) > lumaStride) {
                    lumaStride = chromaStride * 2;
                }
                //}
            }
            if (cbcrInterleave == TRUE) {
                lumaStride = MAX(lumaStride, chromaStride);
            }
            break;
        case FORMAT_422:
            /* nothing to do */
            break;
        case FORMAT_YUYV: // 4:2:2 8bit packed
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            lumaStride = VPU_ALIGN32(width) * 2;
            break;
        case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
            lumaStride = VPU_ALIGN32(width) * 4;
            break;
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            lumaStride = VPU_ALIGN32(width * 2) * 2;
            break;
        default:
            break;
        }
    } else if (mapType == COMPRESSED_FRAME_MAP) {
        switch (format) {
        case FORMAT_420:
        case FORMAT_422:
        case FORMAT_YUYV:
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_422_P10_32BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
        case FORMAT_YUYV_P10_16BIT_MSB:
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            lumaStride = VPU_ALIGN32(VPU_ALIGN16(width) * 5) / 4;
            lumaStride = VPU_ALIGN32(lumaStride);
            break;
        default:
            return -1;
        }
    } else if (mapType == PVRIC_COMPRESSED_FRAME_MAP) {
        switch (format) {
        case FORMAT_420: // 8bit
            if (vp9Scale & VP9_SAMESCALE)
                lumaStride = VPU_ALIGN64(width); // 1:1 and VP9
            else
                lumaStride = VPU_ALIGN32(width);
            break;
        case FORMAT_420_P10_16BIT_LSB: // 10bit
        case FORMAT_420_P10_16BIT_MSB: // 10bit
            if (vp9Scale & VP9_SAMESCALE)
                lumaStride = VPU_ALIGN64(width) * 2; // 1:1 and VP9
            else
                lumaStride = VPU_ALIGN32(width * 2);
            break;
        default:
            return -1;
        }

    } else if (mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP) {
        lumaStride = (width > 2048) ? 4096 : (width > 1024) ? 2048 : (width > 512) ? 1024 : 512;
    } else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP) {
        lumaStride = VPU_ALIGN32(width);
    } else {
        width = (width < height) ? height : width;
        lumaStride = (width > 2048) ? 4096 : (width > 1024) ? 2048 : (width > 512) ? 1024 : 512;
    }

    return lumaStride;
}

Int32 vpu_proto_calc_luma_size(Int32 productId, Int32 stride, Int32 height,
                               FrameBufferFormat format, BOOL cbcrIntl, TiledMapType mapType)
{
    Int32 unit_size_hor_lum, unit_size_ver_lum, size_dpb_lum, field_map, size_dpb_lum_4k;
    UNREFERENCED_PARAMETER(cbcrIntl);
    UNREFERENCED_PARAMETER(format);

    if (mapType == TILED_FIELD_V_MAP || mapType == TILED_FIELD_NO_BANK_MAP ||
        mapType == LINEAR_FIELD_MAP) {
        field_map = 1;
    } else {
        field_map = 0;
    }

    if (mapType == LINEAR_FRAME_MAP || mapType == LINEAR_FIELD_MAP) {
        size_dpb_lum = stride * height;
    } else if (mapType == COMPRESSED_FRAME_MAP) {
        if (productId == PRODUCT_ID_420SN)
            size_dpb_lum = stride * VPU_ALIGN32(height + 4);
        else
            size_dpb_lum = stride * height;
    } else if (mapType == PVRIC_COMPRESSED_FRAME_MAP) {
        Uint32 total_tile_num, hdr_size, data_size;
        total_tile_num =
            (stride >> 5) * (VPU_ALIGN8(height) >>
                             3); // stride = align32(width) for 8bit, align32(width*2) for 10bit
        hdr_size = total_tile_num * 4;                      // bit
        hdr_size = VPU_ALIGN256(VPU_ALIGN8(hdr_size) >> 3); // bit => byte
        data_size = total_tile_num * 256;
        size_dpb_lum = hdr_size + data_size;
        ssdk_printf(SSDK_INFO,
                    "[API] %s PVRIC_COMPRESSED_FRAME_MAP size_dpb_lum %d, stride %d, height %d, "
                    "total_tile_num %d, hdr_size %d, data_size %d\n",
                    __func__, size_dpb_lum, stride, height, total_tile_num, hdr_size, data_size);
    } else if (mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP) {
        unit_size_hor_lum = stride;
        unit_size_ver_lum =
            (((height >> field_map) + 127) / 128) * 128; // unit vertical size is 128 pixel (8MB)
        size_dpb_lum = unit_size_hor_lum * (unit_size_ver_lum << field_map);
    } else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP) {
        if (productId == PRODUCT_ID_960) {
            size_dpb_lum = stride * height;

            // aligned to 8192*2 (0x4000) for top/bot field
            // use upper 20bit address only
            size_dpb_lum_4k = ((size_dpb_lum + 16383) / 16384) * 16384;

            if (mapType == TILED_FIELD_MB_RASTER_MAP) {
                size_dpb_lum_4k = ((size_dpb_lum_4k + (0x8000 - 1)) & ~(0x8000 - 1));
            }

            size_dpb_lum = size_dpb_lum_4k;
        } else {
            size_dpb_lum = stride * (height >> field_map);
            size_dpb_lum_4k = ((size_dpb_lum + (16384 - 1)) / 16384) * 16384;
            size_dpb_lum = size_dpb_lum_4k << field_map;
        }
    } else {
        if (productId == PRODUCT_ID_960) {
            ssdk_printf(SSDK_CRIT, "[API][%s] TODO: support coda 960\n", __func__);
            return 0;
        } else { // productId != 960
            unit_size_hor_lum = stride;
            unit_size_ver_lum =
                (((height >> field_map) + 63) / 64) * 64; // unit vertical size is 64 pixel (4MB)
            size_dpb_lum = unit_size_hor_lum * (unit_size_ver_lum << field_map);
        }
    }

    ssdk_printf(SSDK_INFO, "[API] %s mapType %d size_dpb_lum %d, stride %d, height %d\n", __func__, mapType,
                size_dpb_lum, stride, height);

    return size_dpb_lum;
}

Int32 vpu_proto_calc_chroma_size(Int32 productId, Int32 stride, Int32 height,
                                 FrameBufferFormat format, BOOL cbcrIntl, TiledMapType mapType)
{
    Int32 chr_size_y, chr_size_x;
    Int32 chr_vscale, chr_hscale;
    Int32 unit_size_hor_chr, unit_size_ver_chr;
    Int32 size_dpb_chr, size_dpb_chr_4k;
    Int32 field_map;

    unit_size_hor_chr = 0;
    unit_size_ver_chr = 0;

    chr_hscale = 1;
    chr_vscale = 1;

    switch (format) {
    case FORMAT_420_P10_16BIT_LSB:
    case FORMAT_420_P10_16BIT_MSB:
    case FORMAT_420_P10_32BIT_LSB:
    case FORMAT_420_P10_32BIT_MSB:
    case FORMAT_420:
        chr_hscale = 2;
        chr_vscale = 2;
        break;
    case FORMAT_224:
        chr_vscale = 2;
        break;
    case FORMAT_422:
    case FORMAT_422_P10_16BIT_LSB:
    case FORMAT_422_P10_16BIT_MSB:
    case FORMAT_422_P10_32BIT_LSB:
    case FORMAT_422_P10_32BIT_MSB:
        chr_hscale = 2;
        break;
    case FORMAT_444:
    case FORMAT_400:
    case FORMAT_YUYV:
    case FORMAT_YVYU:
    case FORMAT_UYVY:
    case FORMAT_VYUY:
    case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
    case FORMAT_YUYV_P10_16BIT_LSB:
    case FORMAT_YVYU_P10_16BIT_MSB:
    case FORMAT_YVYU_P10_16BIT_LSB:
    case FORMAT_UYVY_P10_16BIT_MSB:
    case FORMAT_UYVY_P10_16BIT_LSB:
    case FORMAT_VYUY_P10_16BIT_MSB:
    case FORMAT_VYUY_P10_16BIT_LSB:
    case FORMAT_YUYV_P10_32BIT_MSB:
    case FORMAT_YUYV_P10_32BIT_LSB:
    case FORMAT_YVYU_P10_32BIT_MSB:
    case FORMAT_YVYU_P10_32BIT_LSB:
    case FORMAT_UYVY_P10_32BIT_MSB:
    case FORMAT_UYVY_P10_32BIT_LSB:
    case FORMAT_VYUY_P10_32BIT_MSB:
    case FORMAT_VYUY_P10_32BIT_LSB:
        break;
    default:
        return 0;
    }

    if (mapType == TILED_FIELD_V_MAP || mapType == TILED_FIELD_NO_BANK_MAP ||
        mapType == LINEAR_FIELD_MAP) {
        field_map = 1;
    } else {
        field_map = 0;
    }

    if (mapType == LINEAR_FRAME_MAP || mapType == LINEAR_FIELD_MAP) {
        switch (format) {
        case FORMAT_420:
            unit_size_hor_chr = stride / 2;
            unit_size_ver_chr = height / 2;
            break;
        case FORMAT_420_P10_16BIT_LSB:
        case FORMAT_420_P10_16BIT_MSB:
            unit_size_hor_chr = stride / 2;
            unit_size_ver_chr = height / 2;
            break;
        case FORMAT_420_P10_32BIT_LSB:
        case FORMAT_420_P10_32BIT_MSB:
            unit_size_hor_chr = VPU_ALIGN16(stride / 2);
            unit_size_ver_chr = height / 2;
            break;
        case FORMAT_422:
        case FORMAT_422_P10_16BIT_LSB:
        case FORMAT_422_P10_16BIT_MSB:
        case FORMAT_422_P10_32BIT_LSB:
        case FORMAT_422_P10_32BIT_MSB:
            unit_size_hor_chr = VPU_ALIGN32(stride / 2);
            unit_size_ver_chr = height;
            break;
        case FORMAT_YUYV:
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
        case FORMAT_YUYV_P10_16BIT_MSB: // 4:2:2 10bit packed
        case FORMAT_YUYV_P10_16BIT_LSB:
        case FORMAT_YVYU_P10_16BIT_MSB:
        case FORMAT_YVYU_P10_16BIT_LSB:
        case FORMAT_UYVY_P10_16BIT_MSB:
        case FORMAT_UYVY_P10_16BIT_LSB:
        case FORMAT_VYUY_P10_16BIT_MSB:
        case FORMAT_VYUY_P10_16BIT_LSB:
        case FORMAT_YUYV_P10_32BIT_MSB:
        case FORMAT_YUYV_P10_32BIT_LSB:
        case FORMAT_YVYU_P10_32BIT_MSB:
        case FORMAT_YVYU_P10_32BIT_LSB:
        case FORMAT_UYVY_P10_32BIT_MSB:
        case FORMAT_UYVY_P10_32BIT_LSB:
        case FORMAT_VYUY_P10_32BIT_MSB:
        case FORMAT_VYUY_P10_32BIT_LSB:
            unit_size_hor_chr = 0;
            unit_size_ver_chr = 0;
            break;
        default:
            break;
        }
        size_dpb_chr = (format == FORMAT_400) ? 0 : unit_size_ver_chr * unit_size_hor_chr;
    } else if (mapType == COMPRESSED_FRAME_MAP) {
        switch (format) {
        case FORMAT_420:
        case FORMAT_YUYV: // 4:2:2 8bit packed
        case FORMAT_YVYU:
        case FORMAT_UYVY:
        case FORMAT_VYUY:
            if (productId == PRODUCT_ID_420SN)
                size_dpb_chr = VPU_ALIGN16(stride / 2 + 16) * VPU_ALIGN32(height + 4); // [SNID-17]
            else
                size_dpb_chr = VPU_ALIGN16(stride / 2) * height;
            break;
        default:
            /* 10bit */
            stride = VPU_ALIGN64(stride / 2) + 12; /* FIXME: need width information */
            size_dpb_chr = VPU_ALIGN32(stride) * VPU_ALIGN4(height);
            break;
        }
        size_dpb_chr = size_dpb_chr / 2;
    } else if (mapType == PVRIC_COMPRESSED_FRAME_MAP) {
        Uint32 total_tile_num, hdr_size, data_size;
        total_tile_num =
            (stride >> 5) * (VPU_ALIGN8(height >> 1) >>
                             3); // stride = align32(width) for 8bit, align32(width*2) for 10bit
        hdr_size = total_tile_num * 4;                      // bit
        hdr_size = VPU_ALIGN256(VPU_ALIGN8(hdr_size) >> 3); // bit => byte
        data_size = total_tile_num * 256;
        size_dpb_chr = (hdr_size + data_size) >> 1; // x2 will be calculated outside
        ssdk_printf(SSDK_INFO,
                    "[API] %s PVRIC_COMPRESSED_FRAME_MAP size_dpb_chr %d, stride %d, height %d, "
                    "total_tile_num %d, hdr_size %d, data_size %d\n",
                    __func__, size_dpb_chr, stride, height, total_tile_num, hdr_size, data_size);
    } else if (mapType == TILED_FRAME_NO_BANK_MAP || mapType == TILED_FIELD_NO_BANK_MAP) {
        chr_size_y = (height >> field_map) / chr_hscale;
        chr_size_x = stride / chr_vscale;

        unit_size_hor_chr = (chr_size_x > 2048)   ? 4096
                            : (chr_size_x > 1024) ? 2048
                            : (chr_size_x > 512)  ? 1024
                                                  : 512;
        unit_size_ver_chr =
            ((chr_size_y + 127) / 128) * 128; // unit vertical size is 128 pixel (8MB)

        size_dpb_chr =
            (format == FORMAT_400) ? 0 : (unit_size_hor_chr * (unit_size_ver_chr << field_map));
    } else if (mapType == TILED_FRAME_MB_RASTER_MAP || mapType == TILED_FIELD_MB_RASTER_MAP) {
        if (productId == PRODUCT_ID_960) {
            chr_size_x = stride / chr_hscale;
            chr_size_y = height / chr_hscale;
            size_dpb_chr = chr_size_y * chr_size_x;

            // aligned to 8192*2 (0x4000) for top/bot field
            // use upper 20bit address only
            size_dpb_chr_4k = ((size_dpb_chr + 16383) / 16384) * 16384;

            if (mapType == TILED_FIELD_MB_RASTER_MAP) {
                size_dpb_chr_4k = ((size_dpb_chr_4k + (0x8000 - 1)) & ~(0x8000 - 1));
            }

            size_dpb_chr = size_dpb_chr_4k;
        } else {
            size_dpb_chr =
                (format == FORMAT_400) ? 0 : ((stride * (height >> field_map)) / chr_hscale);
            size_dpb_chr_4k = ((size_dpb_chr + (16384 - 1)) / 16384) * 16384;
            size_dpb_chr = size_dpb_chr_4k << field_map;
        }
    } else {
        if (productId == PRODUCT_ID_960) {
            ssdk_printf(SSDK_CRIT, "[API][%s] TODO: support coda 960\n", __func__);
            return 0;
        } else { // productId != 960
            chr_size_y = (height >> field_map) / chr_hscale;
            chr_size_x = cbcrIntl == TRUE ? stride : stride / chr_vscale;

            unit_size_hor_chr = (chr_size_x > 2048)   ? 4096
                                : (chr_size_x > 1024) ? 2048
                                : (chr_size_x > 512)  ? 1024
                                                      : 512;
            unit_size_ver_chr =
                ((chr_size_y + 63) / 64) * 64; // unit vertical size is 64 pixel (4MB)

            size_dpb_chr =
                (format == FORMAT_400) ? 0 : unit_size_hor_chr * (unit_size_ver_chr << field_map);
            size_dpb_chr /= (cbcrIntl == TRUE ? 2 : 1);
        }
    }

    ssdk_printf(SSDK_INFO, "[API] %s mapType %d size_dpb_chr %d, stride %d, height %d\n", __func__, mapType,
                size_dpb_chr, stride, height);

    return size_dpb_chr;
}

Int32 vpu_proto_calc_frame_size(Int32 productId, Int32 stride, Int32 height, TiledMapType mapType,
                                FrameBufferFormat format, BOOL interleave)
{
    Int32 size_dpb_lum, size_dpb_chr, size_dpb_all;
    Int32 size_mvcolbuf = 0;

    size_dpb_lum = vpu_proto_calc_luma_size(productId, stride, height, format, interleave, mapType);
    size_dpb_chr =
        vpu_proto_calc_chroma_size(productId, stride, height, format, interleave, mapType);
    size_dpb_all = size_dpb_lum + size_dpb_chr * 2;

    if (productId == PRODUCT_ID_320) {
        size_mvcolbuf = VPU_ALIGN32(stride) * VPU_ALIGN32(height);
        size_mvcolbuf = (size_mvcolbuf * 3) / 2;
        size_mvcolbuf = (size_mvcolbuf + 4) / 5;
        size_mvcolbuf = VPU_ALIGN8(size_mvcolbuf);
        size_dpb_all += size_mvcolbuf;
    }

    return size_dpb_all;
}
