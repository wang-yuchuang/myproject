/**
 * @file sdrv_jpu.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: jpu sdriver implement
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <assert.h>
#include <cmsis_os2.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <malloc.h>
#include <arch/cache.h>
#include <regs_base.h>
#include <driver/irq.h>
#include <irq_num.h>
#include <driver/memmap.h>

#include "jpuapifunc.h"
#include "jpulog.h"

#include <driver/kunlun/vpu/sdrv_jpu.h>
#include <driver/kunlun/vpu/vpu.h>
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */

/* =========================== marco define session ======================== */
/*
 *   When jpu implements the wrap aroud of the bs buffer,
 *   the minimum reserved space is JPU_GBU_WRAP_SIZE (1024),
 *   so the bs buffer cannot be filled, and half of the value is used here
 */
#define DEFAULT_FEEDING_SIZE (STREAM_BUF_SIZE / 2)

#define ENC_SRC_BUF_NUM 1
#define DEC_SRC_BUF_NUM 2
#define PAGE_SIZE 4096
#define JPU_REGISTER_SIZE 0x1000
#define JPU_INTRPT_REQ_NUM MJPEG_O_INTRPT_REQ_NUM

#define JPU_EVENT_INST1_INT     0x01
#define JPU_EVENT_WORK_REQ      0x02
#define JPU_EVENT_EXIT_NOTIFY   0x04

#define CHAN_EVENT_WORK_ERR     0x01
#define CHAN_EVENT_WORK_DONE    0x02
#define CHAN_EVENT_WORK_BS_DONE 0x04
/* =========================== static value define session ================= */
const int hal_jpu_inv_scan_table[64] = {
    0,  1,  5,  6,  14, 15, 27, 28, 2,  4,  7,  13, 16, 26, 29, 42, 3,  8,  12, 17, 25, 30,
    41, 43, 9,  11, 18, 24, 31, 40, 44, 53, 10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38,
    46, 51, 55, 60, 21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63};
static BYTE hal_jpu_global_huff_bit[8][256] = {
    // DC index 0 (Luminance DC) Bits
    {0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00},
    // AC index 0 (Luminance AC) Bits
    {0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 0x04, 0x04, 0x00, 0x00, 0x01,
     0x7D},
    // DC index 1 (Chrominance DC) Bits
    {0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00,
     0x00},
    // AC index 1 (Chrominance AC) Bits
    {0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05, 0x04, 0x04, 0x00, 0x01, 0x02,
     0x77},
    {0},
    {0},
    {0},
    {0}};
static BYTE hal_jpu_global_huff_val[8][256] = {
    // DC index 0 (Luminance DC) HuffValue
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B},
    // AC index 0 (Luminance AC) HuffValue
    {0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61,
     0x07, 0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
     0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x25,
     0x26, 0x27, 0x28, 0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
     0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63, 0x64,
     0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
     0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
     0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
     0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3,
     0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
     0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA},
    // DC index 1 (Chrominance DC) HuffValue
    {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B},
    // AC index 1 (Chrominance AC)) HuffValue
    {0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61,
     0x71, 0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33,
     0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34, 0xE1, 0x25, 0xF1, 0x17, 0x18,
     0x19, 0x1A, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
     0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x63,
     0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
     0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
     0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
     0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA,
     0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
     0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA},
    {0},
    {0},
    {0},
    {0}};
static short hal_jpu_global_q_mat_tab[4][64] = {
    // Quality 3 Luma
    {
        0x0006, 0x0004, 0x0004, 0x0004, 0x0005, 0x0004, 0x0006, 0x0005, 0x0005, 0x0006, 0x0009,
        0x0006, 0x0005, 0x0006, 0x0009, 0x000B, 0x0008, 0x0006, 0x0006, 0x0008, 0x000B, 0x000C,
        0x000A, 0x000A, 0x000B, 0x000A, 0x000A, 0x000C, 0x0010, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x0010, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
    },
    // Quality 3 Chroma
    {
        0x0007, 0x0007, 0x0007, 0x000D, 0x000C, 0x000D, 0x0018, 0x0010, 0x0010, 0x0018, 0x0014,
        0x000E, 0x000E, 0x000E, 0x0014, 0x0014, 0x000E, 0x000E, 0x000E, 0x000E, 0x0014, 0x0011,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x0011, 0x0011, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x0011, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
        0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C, 0x000C,
    },
    {0},
    {0}};

// event_t hal_jpu_interrupt_signal;
osEventFlagsId_t hal_jpu_interrupt_signal;
unsigned int hal_jpu_interrupt_status;

/* =========================== function implementation session ============= */
int allocate_dma_memory(jpu_buffer_t *vb)
{
    vb->virt_addr = (unsigned long)memalign(1024, vb->size);
    vb->phys_addr = (unsigned long)p2ap((paddr_t)vb->virt_addr);
    vb->base = vb->phys_addr;

    if (!vb->virt_addr || !vb->phys_addr)
        return -1;

    if (jdi_register_memory(vb) < 0)
        return -1;

    return 0;
}

void free_dma_memory(jpu_buffer_t *vb)
{
    if (vb && vb->virt_addr) {
        jdi_unregister_memory(vb);
        free((void *)vb->virt_addr);
        memset(vb, 0x0, sizeof(jpu_buffer_t));
    }
}

static int32_t jpu_interrupt_handler(uint32_t irq_num, void *arg)
{
    unsigned int int_res;

    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)arg;
    // get interrupt reason
    JPU_InterruptSortOut(NULL, &int_res);

    hal_jpu_interrupt_status = int_res;
    JPU_DisableInterrput();
    osEventFlagsSet(sdrv_jpu_ptr->jpu_event, JPU_EVENT_INST1_INT);

    return 0;
}

int hal_jpu_wait_interrupt(sdrv_jpu_drv_t* jpuDrv, int timeout)
{
    int ret = 0;

    if (osErrorTimeout == osEventFlagsWait(jpuDrv->jpu_event, JPU_EVENT_INST1_INT, osFlagsWaitAny, timeout)) {

        JLOG(ERR, "[%s][%d] vpu waiting interrupt timedout", __func__, __LINE__);
        return -1;
    }

    ret = hal_jpu_interrupt_status;
    hal_jpu_interrupt_status = 0;

    if (!ret) {
        JLOG(ERR, "[%s][%d] No interrupt status was read", __func__, __LINE__);
        return -2;
    }

    return ret;
}

void get_frame_buf_stride(FrameFormat subsample, CbCrInterLeave cbcrIntlv, PackedFormat packed,
                          BOOL scalerOn, Uint32 width, Uint32 height, Uint32 bytePerPixel,
                          Uint32 *oLumaStride, Uint32 *oLumaHeight, Uint32 *oChromaStride,
                          Uint32 *oChromaHeight)
{
    Uint32 lStride, cStride;
    Uint32 lHeight, cHeight;
    lStride = JPU_CEIL(8, width);
    lHeight = height;
    cHeight = height / 2;

    if (packed == PACKED_FORMAT_NONE) {
        Uint32 chromaDouble = (cbcrIntlv == CBCR_SEPARATED) ? 1 : 2;

        switch (subsample) {
        case FORMAT_400:
            cStride = 0;
            cHeight = 0;
            break;

        case FORMAT_420:
            cStride = (lStride / 2) * chromaDouble;
            cHeight = (height + 1) / 2;
            break;

        case FORMAT_422:
            cStride = (lStride / 2) * chromaDouble;
            cHeight = height;
            break;

        case FORMAT_440:
            cStride = lStride * chromaDouble;
            cHeight = (height + 1) / 2;
            break;

        case FORMAT_444:
            cStride = lStride * chromaDouble;
            cHeight = height;
            break;

        default:
            cStride = 0;
            lStride = 0;
            cHeight = 0;
            break;
        }
    } else {
        switch (packed) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_VYUY:
            lStride = JPU_CEIL(32, lStride * 2);
            cStride = 0;
            cHeight = 0;
            break;

        case PACKED_FORMAT_444:
            lStride = lStride * 3;
            cStride = 0;
            cHeight = 0;
            break;

        default:
            lStride = 0;
            cStride = 0;
            break;
        }
    }

    if (scalerOn == TRUE) {
        /* Luma stride */
        if (subsample == FORMAT_420 || subsample == FORMAT_422 ||
            (PACKED_FORMAT_422_YUYV <= packed && packed <= PACKED_FORMAT_422_VYUY)) {
            lStride = JPU_CEIL(32, lStride);
        } else {
            lStride = JPU_CEIL(16, lStride);
        }

        /* Chroma stride */
        if (cbcrIntlv == CBCR_SEPARATED) {
            if (subsample == FORMAT_444) {
                cStride = JPU_CEIL(16, cStride);
            } else {
                cStride = JPU_CEIL(8, cStride);
            }
        } else {
            cStride = JPU_CEIL(32, cStride);
        }
    } else {
        if (subsample == FORMAT_420 || subsample == FORMAT_422) {
            lStride = JPU_CEIL(16, lStride);
        } else {
            lStride = JPU_CEIL(8, lStride);
        }
    }

    if (subsample == FORMAT_420 || subsample == FORMAT_440) {
        lHeight = JPU_CEIL(16, lHeight);
    } else {
        lHeight = JPU_CEIL(8, lHeight);
    }

    cHeight = JPU_CEIL(8, cHeight);
    lStride *= bytePerPixel;
    cStride *= bytePerPixel;

    if (oLumaStride)
        *oLumaStride = lStride;

    if (oLumaHeight)
        *oLumaHeight = lHeight;

    if (oChromaStride)
        *oChromaStride = cStride;

    if (oChromaHeight)
        *oChromaHeight = cHeight;
}

int calculate_frame_size(int picWidth, int picHeight, Uint32 bitDepth, FrameFormat format,
                         PackedFormat packed)
{
    int size;
    int nY = 0;
    int lumaSize, chromaSize = 0;
    Uint32 bytesPerPixel = (bitDepth + 7) / 8;

    switch (format) {
    case FORMAT_420:
        nY = picHeight;
        chromaSize = (picWidth / 2) * (picHeight / 2);
        break;

    case FORMAT_440:
        nY = picHeight;
        chromaSize = (picWidth) * (picHeight / 2);
        break;

    case FORMAT_422:
        nY = picHeight;
        chromaSize = (picWidth / 2) * picHeight;
        break;

    case FORMAT_444:
        nY = picHeight;
        chromaSize = picWidth * picHeight;
        break;

    case FORMAT_400:
        nY = picHeight;
        chromaSize = 0;
        break;

    default:
        nY = 0;
        chromaSize = 0;
        return 0;
    }

    if (packed) {
        if (packed == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;
    size = lumaSize + chromaSize * 2;
    size *= bytesPerPixel;

    return size;
}

// JPU_DecGetBitstreamBuffer cannot calculate how much space
// is available for writing, and rdPtr is not an accurate value.
// for FEEDING_METHOD_FRAME_SIZE mode, the bs buffer needs to be
// cleaned up in time after decoding a frame
// int jpu_feeding_bs_buffer(hal_jpu_hw_instance_t *jpu)
int jpu_feeding_bs_buffer(jpu_chan_ins_t* chanIns)
{
    PhysicalAddress wrPtr;
    int room;
    void *virtual_wrPrt = NULL;
    int rightSize = 0;
    int feedSize = 0;
    int expect_size = 0;
    int fill_input_size = 0;

    JPU_DecGetBitstreamBuffer(chanIns->handle, NULL, &wrPtr, &room);

    // Minimum of room / DEFAULT_FEEDING_SIZE
    expect_size = room < DEFAULT_FEEDING_SIZE ? room : DEFAULT_FEEDING_SIZE;

    if ((wrPtr + expect_size) >= (chanIns->bs_stream.phys_addr + chanIns->bs_stream.size)) {
        // right size
        rightSize = chanIns->bs_stream.phys_addr + chanIns->bs_stream.size - wrPtr;

        virtual_wrPrt = (void *)(chanIns->bs_stream.virt_addr + (wrPtr - chanIns->bs_stream.phys_addr));
        if (chanIns->fill_buffer_callback != NULL) {
            chanIns->fill_buffer_callback(chanIns->user_private_ptr, rightSize, virtual_wrPrt,
                                    &fill_input_size);
        }
        if (fill_input_size < 0) {
            JLOG(ERR, "[%s][%d] fill_buffer_callback failed, ret:%d", __func__, __LINE__,
                 fill_input_size);
            return -1;
        }

        arch_clean_cache_range((addr_t)((unsigned long)virtual_wrPrt), fill_input_size);
        feedSize += fill_input_size;

        // left size
        if (fill_input_size == rightSize) {
            expect_size -= fill_input_size;
            wrPtr = chanIns->bs_stream.phys_addr;

            virtual_wrPrt = (void *)(chanIns->bs_stream.virt_addr);
            chanIns->fill_buffer_callback(chanIns->user_private_ptr, expect_size, virtual_wrPrt,
                                      &fill_input_size);
            if (fill_input_size < 0) {
                JLOG(ERR, "[%s][%d] fill_buffer_callback failed, ret:%d", __func__, __LINE__,
                     fill_input_size);
                return -1;
            }
            arch_clean_cache_range((addr_t)((unsigned long)virtual_wrPrt), fill_input_size);
            feedSize += fill_input_size;
        }
    } else {
        virtual_wrPrt = (void *)(chanIns->bs_stream.virt_addr + (wrPtr - chanIns->bs_stream.phys_addr));
        chanIns->fill_buffer_callback(chanIns->user_private_ptr, expect_size, virtual_wrPrt,
                                  &fill_input_size);
        if (fill_input_size < 0) {
            JLOG(ERR, "[%s][%d] fill_buffer_callback failed, ret:%d", __func__, __LINE__,
                 fill_input_size);
            return -1;
        }
        arch_clean_cache_range((addr_t)((unsigned long)virtual_wrPrt), fill_input_size);
        feedSize += fill_input_size;
    }

    JPU_DecUpdateBitstreamBuffer(chanIns->handle, feedSize);

    JLOG(INFO,
         "[%s][%d] wrPtr:%x jpu->bs_stream.phys_addr:%x  expect_size:%d "
         "room:%d rightSize:%d feedSize:%d jpu->bs_stream.size:%d\n",
         __func__, __LINE__, wrPtr, chanIns->bs_stream.phys_addr, expect_size, room, rightSize,
         feedSize, chanIns->bs_stream.size);

    return feedSize;
}

// int jpu_get_output_buffer_yuv_ptr(hal_jpu_hw_instance_t *jpu)
int jpu_get_output_buffer_yuv_ptr(jpu_chan_ins_t* chanIns)
{
    FrameBuffer *fb;
    int nY = 0;
    int lumaSize, chromaSize = 0, chromaWidth = 0, chromaHeight = 0;
    int chromaStride_i = 0;
    Uint32 bytesPerPixel = (chanIns->dec_output.bit_depth + 7) / 8;

    {
        chanIns->dec_output.y_ptr = NULL;
        chanIns->dec_output.u_ptr = NULL;
        chanIns->dec_output.v_ptr = NULL;
        chanIns->dec_output.y_size = 0;
        chanIns->dec_output.u_size = 0;
        chanIns->dec_output.v_size = 0;
    }

    if (chanIns->dec_output.current_output_index < 0) {
        JLOG(ERR, "error index:%d jpu has no effective output buffer\n",
             chanIns->dec_output.output_frame_size);
        return -1;
    }

    fb = &chanIns->dec_frameBuf[chanIns->dec_output.current_output_index];

    int picWidth = chanIns->dec_output.output_width;
    int picHeight = chanIns->dec_output.output_height;
    chromaStride_i = fb->strideC;

    switch (fb->format) {
    case FORMAT_420:
        nY = picHeight;
        chromaSize = (picWidth / 2) * (picHeight / 2);
        chromaWidth = picWidth / 2;
        chromaHeight = nY;
        break;

    case FORMAT_440:
        nY = picHeight;
        chromaSize = (picWidth) * (picHeight / 2);
        chromaWidth = picWidth;
        chromaHeight = nY;
        break;

    case FORMAT_422:
        nY = picHeight;
        chromaSize = (picWidth / 2) * picHeight;
        chromaWidth = (picWidth / 2);
        chromaHeight = nY * 2;
        break;

    case FORMAT_444:
        nY = picHeight;
        chromaSize = picWidth * picHeight;
        chromaWidth = picWidth;
        chromaHeight = nY * 2;
        break;

    case FORMAT_400:
        nY = picHeight;
        chromaSize = 0;
        chromaWidth = 0;
        chromaHeight = 0;
        break;

    default:
        return 0;
    }

    if (chanIns->dec_output.output_packed_format) {
        if (chanIns->dec_output.output_packed_format == PACKED_FORMAT_444)
            picWidth *= 3;
        else
            picWidth *= 2;

        chromaSize = 0;
    }

    lumaSize = picWidth * nY;
    lumaSize *= bytesPerPixel;
    chromaSize *= bytesPerPixel;
    picWidth *= bytesPerPixel;
    chromaWidth *= bytesPerPixel;

    if (chanIns->dec_output.output_cbcrInterleave) {
        chromaSize = chromaSize * 2;
        chromaWidth = chromaWidth * 2;
        // chromaStride_i = chromaStride_i;
    }

    if (chanIns->dec_output.output_packed_format) {
        JLOG(ERR, "[%s][%d] packed format, set chroma width to 0\n", __func__, __LINE__);
        chromaWidth = 0;
    }

    if (((unsigned int)picWidth == fb->stride) && (chromaWidth == chromaStride_i)) {
        chanIns->dec_output.y_ptr = (void *)(chanIns->dec_frame_buffer_stream.virt_addr +
                                         (fb->bufY - chanIns->dec_frame_buffer_stream.phys_addr));
        chanIns->dec_output.y_size = lumaSize;
        arch_invalidate_cache_range((addr_t)((unsigned long)chanIns->dec_output.y_ptr), lumaSize);

        if (chanIns->dec_output.output_packed_format)
            return 0;

        if (chanIns->dec_output.output_cbcrInterleave) {
            chanIns->dec_output.u_ptr = (void *)(chanIns->dec_frame_buffer_stream.virt_addr +
                                             (fb->bufCb - chanIns->dec_frame_buffer_stream.phys_addr));
            chanIns->dec_output.u_size = chromaSize;
            arch_invalidate_cache_range((addr_t)((unsigned long)chanIns->dec_output.u_ptr), chromaSize);
        } else {
            chanIns->dec_output.u_ptr = (void *)(chanIns->dec_frame_buffer_stream.virt_addr +
                                             (fb->bufCb - chanIns->dec_frame_buffer_stream.phys_addr));
            chanIns->dec_output.u_size = chromaSize;
            arch_invalidate_cache_range((addr_t)((unsigned long)chanIns->dec_output.u_ptr), chromaSize);

            chanIns->dec_output.v_ptr = (void *)(chanIns->dec_frame_buffer_stream.virt_addr +
                                             (fb->bufCr - chanIns->dec_frame_buffer_stream.phys_addr));
            chanIns->dec_output.v_size = chromaSize;
            arch_invalidate_cache_range((addr_t)((unsigned long)chanIns->dec_output.v_ptr), chromaSize);
        }
    } else {
        JLOG(ERR,
             "[%s][%d] nY=%d picWidth:%d fb->stride:%d chromaWidth:%d  chromaHeight:%d "
             "chromaStride_i:%d\n",
             __func__, __LINE__, nY, picWidth, fb->stride, chromaWidth, chromaHeight,
             chromaStride_i);
        return -1;
    }

    return 0;
}

int jpu_dec_register_frame_buffer(jpu_chan_ins_t* chanIns, jpu_buffer_t *frame_buffer,
                                  int from_fd)
{
    int ret = 0;
    int needFrameBufCount = DEC_SRC_BUF_NUM;
    unsigned int last_addr;
    JpgDecInfo *decInfo = &chanIns->handle->JpgInfo->decInfo;
    Uint32 fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
    Uint32 fbLumaSize, fbChromaSize, fbSize;
    Uint32 bytePerPixel = (decInfo->bitDepth + 7) / 8;

    get_frame_buf_stride(chanIns->dec_output.output_format, chanIns->dec_output.output_cbcrInterleave,
                         chanIns->dec_output.output_packed_format, FALSE, chanIns->dec_output.output_width,
                         chanIns->dec_output.output_height, bytePerPixel, &fbLumaStride, &fbLumaHeight,
                         &fbChromaStride, &fbChromaHeight);
    fbLumaSize = fbLumaStride * fbLumaHeight;
    fbChromaSize = fbChromaStride * fbChromaHeight;

    if (chanIns->dec_output.output_cbcrInterleave == CBCR_SEPARATED) {
        /* fbChromaSize MUST be zero when format is packed mode */
        fbSize = fbLumaSize + 2 * fbChromaSize;
    } else {
        /* Semi-planar */
        fbSize = fbLumaSize + fbChromaSize;
    }

    if (from_fd) {
        JLOG(ERR, "TODO: support register frame buffer from external memery\n");
        return -1;
    } else {
        frame_buffer->size = fbSize * needFrameBufCount;

        if (allocate_dma_memory(frame_buffer) < 0) {
            JLOG(ERR, "Fail to allocate frame buffer size=%d\n", frame_buffer->size);
            return -1;
        }

        JLOG(INFO, "frame_buffer size:%d, addr:%x\n", frame_buffer->size, frame_buffer->phys_addr);
    }

    last_addr = frame_buffer->phys_addr;
    for (int i = 0; i < needFrameBufCount; i++) {
        chanIns->dec_frameBuf[i].bufY = last_addr;
        last_addr += fbLumaSize;
        last_addr = JPU_CEIL(8, last_addr);

        if (fbChromaSize) {
            chanIns->dec_frameBuf[i].bufCb = last_addr;
            last_addr += fbChromaSize;
            last_addr = JPU_CEIL(8, last_addr);

            if (chanIns->dec_output.output_cbcrInterleave == CBCR_SEPARATED) {
                chanIns->dec_frameBuf[i].bufCr = last_addr;
                last_addr += fbChromaSize;
                last_addr = JPU_CEIL(8, last_addr);
            }
        } else {
            chanIns->dec_frameBuf[i].bufCb = 0;
            chanIns->dec_frameBuf[i].bufCr = 0;
        }
        chanIns->dec_frameBuf[i].stride = fbLumaStride;
        chanIns->dec_frameBuf[i].strideC = fbChromaStride;
        chanIns->dec_frameBuf[i].fbLumaHeight = fbLumaHeight;
        chanIns->dec_frameBuf[i].fbChromaHeight = fbChromaHeight;
        chanIns->dec_frameBuf[i].endian = decInfo->frameEndian;
        chanIns->dec_frameBuf[i].format = chanIns->dec_output.output_format;
        chanIns->dec_frameBuf[i].fbsize = fbSize;
    }

    ret =
        JPU_DecRegisterFrameBuffer(chanIns->handle, chanIns->dec_frameBuf, needFrameBufCount, chanIns->dec_frameBuf[0].stride);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecRegisterFrameBuffer failed Error code is 0x%x \n", ret);
        return -1;
    }

    return 0;
}

int jpu_dec_and_wait_interrupt(jpu_chan_ins_t* chanIns, int dec_to_fd, sdrv_jpu_drv_t* jpuDrv)
{
    int ret = 0;
    int int_reason = 0;
    static int frameIdx = 0;

    JpgDecParam decParam = {0};
    JpgDecOutputInfo outputInfo = {0};
    JpgRet jpu_ret = JPG_RET_SUCCESS;
    JpgDecInfo *decInfo = &chanIns->handle->JpgInfo->decInfo;
    chanIns->dec_output.decode_finish = false;

    while (1) {
        if (chanIns->dec_feedong_method == FEED_METHOD_FRAME_SIZE &&
            decInfo->streamWrPtr == decInfo->streamBufStartAddr) {
            /*
             for FEED_METHOD_FRAME_SIZE mode,
             streamRdPtr is greater than or equal to streamWrPtr,
             so the buffer must be filled before decoding,
             otherwise jpu may directly decode the bs data of the previous frame
            */
            ret = jpu_feeding_bs_buffer(chanIns);
            if (ret < 0) {
                JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
                return ret;
            }
        }

        jpu_ret = JPU_DecStartOneFrame(chanIns->handle, &decParam);
        if (jpu_ret != JPG_RET_SUCCESS && jpu_ret != JPG_RET_EOS) {
            if (jpu_ret == JPG_RET_BIT_EMPTY) {
                JLOG(INFO, "BITSTREAM NOT ENOUGH!\n");

                ret = jpu_feeding_bs_buffer(chanIns);
                if (ret < 0) {
                    JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
                    return ret;
                }
                continue;
            }

            JLOG(ERR, "JPU_DecStartOneFrame failed Error code is 0x%x \n", jpu_ret);
            return -1;
        }

        if (jpu_ret == JPG_RET_EOS) {
            JLOG(INFO, "DECODE FINISH\n");
            JPU_DecGetOutputInfo(chanIns->handle, &outputInfo); // unlock

            chanIns->dec_output.decode_finish = TRUE;
            chanIns->dec_output.current_output_index = -1;

            break;
        }

        while (1) {
            if ((int_reason = hal_jpu_wait_interrupt(jpuDrv, JPU_INTERRUPT_TIMEOUT_MS)) == -1) {
                JLOG(ERR, "[%s][%d] Error : timeout happened\n", __func__, __LINE__);
                JPU_SWReset(chanIns->handle);
                break;
            }

            if (int_reason == -2) {
                JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance \n");
                break;
            }

            if (int_reason &
                ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_SLICE_DONE))) {
                JLOG(INFO, "int_reason: %08x\n", int_reason);
                break;
            }

            if (int_reason & (1 << INT_JPU_BIT_BUF_EMPTY)) {
                JLOG(INFO, " %s interrupt reason %#x INT_JPU_BIT_BUF_EMPTY now \n", __func__,
                     int_reason);

                ret = jpu_feeding_bs_buffer(chanIns);
                if (ret < 0) {
                    JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
                    return ret;
                }

                JPU_ClrStatus(chanIns->handle, (1 << INT_JPU_BIT_BUF_EMPTY));
                JPU_EnableInterrput(decInfo->intrEnableBit);
            }
        }

        jpu_ret = JPU_DecGetOutputInfo(chanIns->handle, &outputInfo);
        if (jpu_ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_DecGetOutputInfo failed Error code is 0x%x \n", jpu_ret);
            return -1;
        }

        int dec_fail = 0;
        if (outputInfo.decodingSuccess == 0) {
            dec_fail = 1;
            JLOG(ERR, "JPU_DecGetOutputInfo decode fail framdIdx %d \n", frameIdx);
        }

        JLOG(INFO,
             "frameIdx:%d, outputInfo.indexFrameDisplay:%d outputInfo.bytePosFrameStart:%x "
             "outputInfo.ecsPtr:%x outputInfo.consumedByte:%d  outputInfo.rdPtr:%x "
             "outputInfo.wrPtr:%x outputInfo.frameCycle:%d\n",
             frameIdx, outputInfo.indexFrameDisplay, outputInfo.bytePosFrameStart,
             outputInfo.ecsPtr, outputInfo.consumedByte, outputInfo.rdPtr, outputInfo.wrPtr,
             outputInfo.frameCycle);

        chanIns->dec_output.current_output_index = outputInfo.indexFrameDisplay;

        if (!dec_to_fd) {
            ret = jpu_get_output_buffer_yuv_ptr(chanIns);
            if (ret < 0) {
                JLOG(ERR, "jpu_get_output_buffer_yuv_ptr filed\n");
                return -1;
            }
        }

        if (outputInfo.indexFrameDisplay == -1) // end of sequence decoding
            break;

        if (outputInfo.numOfErrMBs) {
            int errRstIdx, errPosX, errPosY;
            errRstIdx = (outputInfo.numOfErrMBs & 0x0F000000) >> 24;
            errPosX = (outputInfo.numOfErrMBs & 0x00FFF000) >> 12;
            errPosY = (outputInfo.numOfErrMBs & 0x00000FFF);
            JLOG(ERR, "Error restart Idx : %d, MCU x:%d, y:%d, in Frame : %d \n", errRstIdx,
                 errPosX, errPosY, frameIdx);
            dec_fail = 1;
        }

        if (chanIns->dec_feedong_method == FEED_METHOD_FRAME_SIZE)
            JPU_DecSetRdPtrEx(chanIns->handle, decInfo->streamBufStartAddr, TRUE);

        frameIdx++;
        if (dec_fail)
            return -1;
        break;
    }

    return 0;
}

/**************************************ENC**************************************************/

static void calc_slice_height(JpgEncOpenParam *encOP, Uint32 sliceHeight)
{
    Uint32 rotation = encOP->rotation;
    Uint32 width = (rotation == 90 || rotation == 270) ? encOP->picHeight : encOP->picWidth;
    Uint32 height = (rotation == 90 || rotation == 270) ? encOP->picWidth : encOP->picHeight;
    FrameFormat format = encOP->sourceFormat;

    if (rotation == 90 || rotation == 270) {
        if (format == FORMAT_422)
            format = FORMAT_440;
        else if (format == FORMAT_440)
            format = FORMAT_422;
    }

    encOP->sliceHeight = (sliceHeight == 0) ? height : sliceHeight;

    if (encOP->sliceHeight != height) {
        if (format == FORMAT_420 || format == FORMAT_422)
            encOP->restartInterval = width / 16;
        else
            encOP->restartInterval = width / 8;

        if (format == FORMAT_420 || format == FORMAT_440)
            encOP->restartInterval *= (encOP->sliceHeight / 16);
        else
            encOP->restartInterval *= (encOP->sliceHeight / 8);

        encOP->sliceInstMode = TRUE;
    }
}

int set_enc_params(jpu_chan_ins_t* chanIns, JpgEncOpenParam *encOP)
{
    encOP->bitstreamBuffer = chanIns->bs_stream.phys_addr;
    encOP->bitstreamBufferSize = chanIns->bs_stream.size;
    encOP->picWidth = chanIns->enc_input.pic_width;
    encOP->picHeight = chanIns->enc_input.pic_height;
    encOP->sourceFormat = chanIns->enc_input.frame_format;
    encOP->restartInterval = 0;
    encOP->streamEndian = 0;
    encOP->frameEndian = 0;

    if (chanIns->enc_input.packageFormat == 1 || chanIns->enc_input.packageFormat == 2)
        encOP->chromaInterleave = CBCR_INTERLEAVE;
    else
        encOP->chromaInterleave = CBCR_SEPARATED;

    memcpy(encOP->huffVal, hal_jpu_global_huff_val, 4 * 256);
    memcpy(encOP->huffBits, hal_jpu_global_huff_bit, 4 * 256);
    for (int i = 0; i < 64; i++) {
        encOP->qMatTab[DC_TABLE_INDEX0][hal_jpu_inv_scan_table[i]] =
            hal_jpu_global_q_mat_tab[DC_TABLE_INDEX0][i];
        encOP->qMatTab[AC_TABLE_INDEX0][hal_jpu_inv_scan_table[i]] =
            hal_jpu_global_q_mat_tab[AC_TABLE_INDEX0][i];
    }
    memcpy(encOP->qMatTab[DC_TABLE_INDEX1], encOP->qMatTab[DC_TABLE_INDEX0], 64 * sizeof(short));
    memcpy(encOP->qMatTab[AC_TABLE_INDEX1], encOP->qMatTab[AC_TABLE_INDEX0], 64 * sizeof(short));

    encOP->jpg12bit = 0;
    encOP->q_prec0 = 0;
    encOP->q_prec1 = 0;
    encOP->packedFormat = (PackedFormat)(chanIns->enc_input.packageFormat - 2);
    if (encOP->packedFormat >= PACKED_FORMAT_MAX)
        encOP->packedFormat = PACKED_FORMAT_NONE;
    encOP->pixelJustification = 0;
    encOP->tiledModeEnable = 0;
    encOP->sliceHeight = 0;
    encOP->intrEnableBit =
        ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_FULL));
    encOP->sliceInstMode = 0;
    encOP->rotation = 0;
    encOP->mirror = 0;

    if (encOP->packedFormat == PACKED_FORMAT_444 && encOP->sourceFormat != FORMAT_444) {
        JLOG(ERR, "Invalid operation mode : In case of using packed mode."
                  "sourceFormat must be FORMAT_444\n");
        return -1;
    }

    calc_slice_height(encOP, encOP->sliceHeight);

    JLOG(INFO, "------------------------------ ENCODER PARAM -------------------\n");
    JLOG(INFO, "[picWidth           ]: %d\n", encOP->picWidth);
    JLOG(INFO, "[picHeight          ]: %d\n", encOP->picHeight);
    JLOG(INFO, "[streamEndian       ]: %d\n", encOP->streamEndian);
    JLOG(INFO, "[FrameEndia         ]: %d\n", encOP->frameEndian);
    JLOG(INFO, "[sliceHeight        ]: %d\n", encOP->sliceHeight);
    JLOG(INFO, "[rotation           ]: %d\n", encOP->rotation);
    JLOG(INFO, "[tiledModeEnable    ]: %d\n", encOP->tiledModeEnable);
    JLOG(INFO, "[mirror             ]: %d\n", encOP->mirror);
    JLOG(INFO, "[srcFrameFormat     ]: %d\n", encOP->sourceFormat);
    JLOG(INFO, "[chromaInterleave   ]: %d\n", encOP->chromaInterleave);
    JLOG(INFO, "[packedFormat       ]: %d\n", encOP->packedFormat);
    JLOG(INFO, "[restartInterval    ]: %d\n", encOP->restartInterval);
    JLOG(INFO, "[jpg12bit           ]: %d\n", encOP->jpg12bit);
    JLOG(INFO, "[q_prec0            ]: %d\n", encOP->q_prec0);
    JLOG(INFO, "[q_prec1            ]: %d\n", encOP->q_prec1);
    JLOG(INFO, "[pixelJustification ]: %d\n", encOP->pixelJustification);
    JLOG(INFO, "[tiledModeEnable    ]: %d\n", encOP->tiledModeEnable);
    JLOG(INFO, "[sliceHeight        ]: %d\n", encOP->sliceHeight);
    JLOG(INFO, "[tiledModeEnable    ]: %d\n", encOP->tiledModeEnable);
    JLOG(INFO, "[sliceInstMode      ]: %d\n", encOP->sliceInstMode);
    JLOG(INFO, "----------------------------------------------------------------\n");

    return 0;
}

int calulate_enc_input_buffer_info(jpu_chan_ins_t *chanIns, JpgEncOpenParam *encOP)
{
    Uint32 lSize = 0, cSize = 0, divc = 1;
    Uint32 Bpp = (encOP->jpg12bit == TRUE) ? 2 : 1; // the size of each pixel component
    Uint32 divw, divh;
    divw = divh = 1;
    FrameFormat format;

    if (encOP->packedFormat == PACKED_FORMAT_NONE) {
        format = encOP->sourceFormat;
    } else {
        switch (encOP->packedFormat) {
        case PACKED_FORMAT_422_YUYV:
        case PACKED_FORMAT_422_UYVY:
        case PACKED_FORMAT_422_YVYU:
        case PACKED_FORMAT_422_VYUY:
            format = FORMAT_422;
            break;

        case PACKED_FORMAT_444:
            format = FORMAT_444;
            break;

        default:
            format = FORMAT_MAX;
            break;
        }
    }

    switch (format) {
    case FORMAT_400:
        /* No chroma data */
        divw = divh = 0;
        break;

    case FORMAT_420:
        divw = 2;
        divh = 2;
        break;

    case FORMAT_422:
        divw = 2;
        divh = 1;
        break;

    case FORMAT_440:
        divw = 1;
        divh = 2;
        break;

    case FORMAT_444:
        break;

    default:
        JLOG(WARN, "%s:%d NOT SUPPORTED YUV FORMAT: %d\n", __FUNCTION__, __LINE__, format);
        break;
    }

    divc = divw * divh;
    lSize = encOP->picWidth * encOP->picHeight * Bpp;
    cSize =
        divc == 0 ? 0 : JPU_CEIL(2, encOP->picWidth) * JPU_CEIL(2, encOP->picHeight) * Bpp / divc;
    chanIns->enc_input.frameSize = lSize + 2 * cSize;
    chanIns->enc_input.lumaSize = lSize;
    chanIns->enc_input.chromaSize = cSize;
    chanIns->enc_input.lumaLineWidth = encOP->picWidth * Bpp;
    chanIns->enc_input.lumaHeight = encOP->picHeight;
    chanIns->enc_input.chromaLineWidth = divw == 0 ? 0 : encOP->picWidth * Bpp / divw; // note: floor
    chanIns->enc_input.chromaHeight = divh == 0 ? 0 : encOP->picHeight / divh;

    if (encOP->packedFormat != PACKED_FORMAT_NONE) {
        if (encOP->packedFormat == PACKED_FORMAT_444) {
            chanIns->enc_input.lumaLineWidth *= 3;
        } else {
            chanIns->enc_input.lumaLineWidth *= 2;
        }

        chanIns->enc_input.chromaLineWidth = 0;
        chanIns->enc_input.chromaHeight = 0;
    } else {
        if (encOP->chromaInterleave != CBCR_SEPARATED) {
            chanIns->enc_input.chromaLineWidth *= 2;
        }
    }

    return 0;
}

int jpu_enc_int(jpu_chan_ins_t* chanIns)
{
    JpgRet ret = JPG_RET_SUCCESS;
    Uint32 framebufWidth = 0, framebufHeight = 0;
    JpgEncOpenParam *encOP = (JpgEncOpenParam *)malloc(sizeof(JpgEncOpenParam));
    if (!encOP) {
        JLOG(ERR, "mall JpgEncOpenParam failed");
        return -1;
    }

    if (!chanIns || chanIns->bs_stream.size <= 0) {
        JLOG(ERR, "jpu_enc_ints Invalid input parameter\n");
        goto EXIT_FREE_MALLOC_SAPCE;
    }

    // 0. enc params
    ret = set_enc_params(chanIns, encOP);
    if (ret < 0) {
        JLOG(ERR, "set_enc_params error \n");
        goto EXIT_FREE_MALLOC_SAPCE;
    }

    // 1. open enc
    ret = JPU_EncOpen(&chanIns->handle, encOP);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_EncOpen failed Error code is 0x%x \n", ret);
        goto EXIT_FREE_MALLOC_SAPCE;
    }

    // 2.alloca frame buffer and register to jpu
    int bEnStuffByte = 0;
    int needFrameBufCount = ENC_SRC_BUF_NUM;
    int bitDepth = (encOP->jpg12bit == 0) ? 8 : 12;
    JPU_EncGiveCommand(chanIns->handle, SET_JPG_USE_STUFFING_BYTE_FF, &bEnStuffByte);
    framebufWidth = (encOP->sourceFormat == FORMAT_420 || encOP->sourceFormat == FORMAT_422)
                        ? JPU_CEIL(16, encOP->picWidth)
                        : JPU_CEIL(8, encOP->picWidth);
    framebufHeight = (encOP->sourceFormat == FORMAT_420 || encOP->sourceFormat == FORMAT_440)
                         ? JPU_CEIL(16, encOP->picHeight)
                         : JPU_CEIL(8, encOP->picHeight);

    {
        uint64_t last_addr;
        Uint32 fbLumaStride, fbLumaHeight, fbChromaStride, fbChromaHeight;
        Uint32 fbLumaSize, fbChromaSize, fbSize;
        Uint32 bytePerPixel = (bitDepth + 7) / 8;

        if (encOP->rotation == 90 || encOP->rotation == 270) {
            if (encOP->sourceFormat == FORMAT_422)
                encOP->sourceFormat = FORMAT_440;
            else if (encOP->sourceFormat == FORMAT_440)
                encOP->sourceFormat = FORMAT_422;
        }

        get_frame_buf_stride(encOP->sourceFormat, encOP->chromaInterleave, encOP->packedFormat, 0,
                             framebufWidth, framebufHeight, bytePerPixel, &fbLumaStride,
                             &fbLumaHeight, &fbChromaStride, &fbChromaHeight);
        fbLumaSize = fbLumaStride * fbLumaHeight;
        fbChromaSize = fbChromaStride * fbChromaHeight;

        if (encOP->chromaInterleave == CBCR_SEPARATED) {
            /* fbChromaSize MUST be zero when format is packed mode */
            fbSize = fbLumaSize + 2 * fbChromaSize;
        } else {
            /* Semi-planar */
            fbSize = fbLumaSize + fbChromaSize;
        }

        chanIns->enc_frame_buffer_stream.size = fbSize * needFrameBufCount;

        if (allocate_dma_memory(&chanIns->enc_frame_buffer_stream) < 0) {
            JLOG(ERR, "Fail to allocate frame buffer size=%d\n", chanIns->enc_frame_buffer_stream.size);
            goto ERR_INIT_ENC;
        }

        last_addr = chanIns->enc_frame_buffer_stream.phys_addr;

        // clear frame buffer
        memset((void *)chanIns->enc_frame_buffer_stream.virt_addr, 0x00,
               chanIns->enc_frame_buffer_stream.size);
        arch_clean_cache_range((addr_t)(chanIns->enc_frame_buffer_stream.virt_addr),
                               chanIns->enc_frame_buffer_stream.size);

        for (int i = 0; i < needFrameBufCount; i++) {
            chanIns->enc_frameBuf[i].bufY = last_addr;
            last_addr += fbLumaSize;
            last_addr = JPU_CEIL(8, last_addr);

            if (fbChromaSize) {
                chanIns->enc_frameBuf[i].bufCb = last_addr;
                last_addr += fbChromaSize;
                last_addr = JPU_CEIL(8, last_addr);

                if (encOP->chromaInterleave == CBCR_SEPARATED) {
                    chanIns->enc_frameBuf[i].bufCr = last_addr;
                    last_addr += fbChromaSize;
                    last_addr = JPU_CEIL(8, last_addr);
                }
            } else {
                chanIns->enc_frameBuf[i].bufCb = 0;
                chanIns->enc_frameBuf[i].bufCr = 0;
            }
            chanIns->enc_frameBuf[i].stride = fbLumaStride;
            chanIns->enc_frameBuf[i].strideC = fbChromaStride;
            chanIns->enc_frameBuf[i].fbChromaHeight = fbLumaHeight;
            chanIns->enc_frameBuf[i].fbLumaHeight = fbChromaHeight;
            chanIns->enc_frameBuf[i].endian = encOP->frameEndian;
            chanIns->enc_frameBuf[i].format = encOP->sourceFormat;
            chanIns->enc_frameBuf[i].fbsize = fbSize;
        }
    }

    JLOG(INFO, "framebuffer stride = %d, width = %d, height = %d\n", chanIns->enc_frameBuf[0].stride,
         framebufWidth, framebufHeight);

    // 3.calulate input buffer info
    calulate_enc_input_buffer_info(chanIns, encOP);

    free(encOP);
    return 0;

ERR_INIT_ENC:
    if (chanIns->enc_frame_buffer_stream.size > 0)
        free_dma_memory(&chanIns->enc_frame_buffer_stream);

    memset(chanIns->enc_frameBuf, 0, sizeof(chanIns->enc_frameBuf));

    JPU_EncClose(chanIns->handle);

EXIT_FREE_MALLOC_SAPCE:
    if (encOP)
        free(encOP);

    return -1;
}

int copy_yuv_data(jpu_chan_ins_t *chanIns, PhysicalAddress fbAddr, Uint32 fbStride,
                  Uint32 fbHeight, Uint32 dataStride, Uint32 dataHeight)
{
    PhysicalAddress addr = fbAddr;
    int feed_size = 0;

    for (unsigned int i = 0; i < dataHeight; i++) {
        chanIns->fill_buffer_callback(chanIns->user_private_ptr, dataStride,
                                  (void *)(chanIns->enc_frame_buffer_stream.virt_addr +
                                           (addr - chanIns->enc_frame_buffer_stream.phys_addr)),
                                  &feed_size);

        if ((unsigned int)feed_size != dataStride) {
            JLOG(ERR, "Not getting enough input buffer\n");
            return -1;
        }

        addr += fbStride;
    }
    arch_clean_cache_range((addr_t)(chanIns->enc_frame_buffer_stream.virt_addr +
                                    (fbAddr - chanIns->enc_frame_buffer_stream.phys_addr)),
                           addr - fbAddr);
    return 0;
}

int jpu_enc_feed_input_buff(jpu_chan_ins_t *chanIns, FrameBuffer *fb)
{
    BOOL success = TRUE;
    PackedFormat packed_format = chanIns->handle->JpgInfo->encInfo.packedFormat;

    // clear frame buffer, already do it when first apply for the framebuffer
    // memset(jdi_get_memory(fb->bufY), 0x00, fb->fbsize);

    switch (chanIns->enc_input.frame_format) {
    case FORMAT_400:
        if (copy_yuv_data(chanIns, fb->bufY, fb->stride, fb->fbLumaHeight, chanIns->enc_input.lumaLineWidth,
                          chanIns->enc_input.lumaHeight) < 0)
            success = FALSE;
        break;
    case FORMAT_420:
    case FORMAT_422:
    case FORMAT_440:
    case FORMAT_444: // pEncConfig->packedFormat
        if (packed_format == PACKED_FORMAT_NONE) {
            if (copy_yuv_data(chanIns, fb->bufY, fb->stride, fb->fbLumaHeight,
                              chanIns->enc_input.lumaLineWidth, chanIns->enc_input.lumaHeight) < 0)
                success = FALSE;

            if (chanIns->handle->JpgInfo->encInfo.chromaInterleave == CBCR_SEPARATED) {
                if (copy_yuv_data(chanIns, fb->bufCb, fb->strideC, fb->fbChromaHeight,
                                  chanIns->enc_input.chromaLineWidth, chanIns->enc_input.chromaHeight) < 0)
                    success = FALSE;
                if (copy_yuv_data(chanIns, fb->bufCr, fb->strideC, fb->fbChromaHeight,
                                  chanIns->enc_input.chromaLineWidth, chanIns->enc_input.chromaHeight) < 0)
                    success = FALSE;
            } else {
                if (copy_yuv_data(chanIns, fb->bufCb, fb->strideC, fb->fbChromaHeight,
                                  chanIns->enc_input.chromaLineWidth, chanIns->enc_input.chromaHeight) < 0)
                    success = FALSE;
            }
        } else {
            if (copy_yuv_data(chanIns, fb->bufY, fb->stride, fb->fbLumaHeight,
                              chanIns->enc_input.lumaLineWidth, chanIns->enc_input.lumaHeight) < 0)
                success = FALSE;
        }
        break;
    default:
        JLOG(ERR, "%s:%d NOT SUPPORTED YUV FORMAT:%d\n", __FUNCTION__, __LINE__,
             chanIns->enc_input.frame_format);
        success = FALSE;
        break;
    }

    return success == FALSE ? -1 : 0;
}

static int send_output_buffer_to_user(jpu_chan_ins_t *chanIns, jpu_buffer_t bs,
                                      JpgEncOutputInfo *pEncOutput)
{
    JpgRet ret = JPG_RET_SUCCESS;
    PhysicalAddress paBsBufEnd;
    PhysicalAddress paBsBufWrPtr;
    PhysicalAddress paBsBufRdPtr;
    Int32 size = 0;
    Uint32 loadSize = 0;
    paBsBufEnd = bs.phys_addr + bs.size;

    if (pEncOutput) {
        paBsBufWrPtr = pEncOutput->streamWrPtr;
        paBsBufRdPtr = pEncOutput->streamRdPtr;
        size = (Uint32)(paBsBufWrPtr - paBsBufRdPtr);
    } else {
        if (JPG_RET_SUCCESS !=
            (ret = JPU_EncGetBitstreamBuffer(chanIns->handle, &paBsBufRdPtr, &paBsBufWrPtr, &size))) {
            JLOG(ERR, "<%s:%d> JPU_EncGetBitstreamBuffer failed Error code is 0x%x \n",
                 __FUNCTION__, __LINE__, ret);
            return -1;
        }
    }

    loadSize = size;

    if (loadSize > 0) {
        int room = 0;

        if ((paBsBufRdPtr + size) > paBsBufEnd) {
            room = paBsBufEnd - paBsBufRdPtr;
            arch_invalidate_cache_range((addr_t)(bs.virt_addr + (paBsBufRdPtr - bs.phys_addr)),
                                        room);
            chanIns->read_buffer_callback(chanIns->user_private_ptr,
                                      (void *)(bs.virt_addr + (paBsBufRdPtr - bs.phys_addr)), room);

            arch_invalidate_cache_range((addr_t)(bs.virt_addr), (size - room));
            chanIns->read_buffer_callback(chanIns->user_private_ptr, (void *)bs.virt_addr, (size - room));
        } else {
            arch_invalidate_cache_range((addr_t)(bs.virt_addr + (paBsBufRdPtr - bs.phys_addr)),
                                        size);
            chanIns->read_buffer_callback(chanIns->user_private_ptr,
                                      (void *)(bs.virt_addr + (paBsBufRdPtr - bs.phys_addr)), size);
        }

        if (pEncOutput && (ENCODE_STATE_FRAME_DONE == pEncOutput->encodeState)) {
            loadSize = STREAM_END_SIZE; // after done of one frame. set the current wrPtr to to the
                                        // base address in the bitstream buffer.
        }

        ret = JPU_EncUpdateBitstreamBuffer(chanIns->handle, loadSize);

        if (JPG_RET_SUCCESS != ret) {
            JLOG(ERR, "<%s:%d> VPU_EncUpdateBitstreamBuffer failed Error code is 0x%x \n",
                 __FUNCTION__, __LINE__, ret);
            return -1;
        }
    }
    return 0;
}

static int32_t jpu_enc_and_wait_interrupt(jpu_chan_ins_t* chanIns, sdrv_jpu_drv_t* jpuDrv)
{
    int ret = 0;
    int int_reason = 0;
    int src_frame_idx = 0;
    static int frameIdx = 0;

    JpgEncParam encParam = {0};
    JpgEncOutputInfo outputInfo = {0};
    JpgEncParamSet encHeaderParam = {0};

    outputInfo.encodeState = ENCODE_STATE_NEW_FRAME;

    while (1) {
        if (outputInfo.encodeState == ENCODE_STATE_NEW_FRAME) { // means new frame start
            src_frame_idx = (frameIdx % ENC_SRC_BUF_NUM);

            encHeaderParam.size = chanIns->bs_stream.size;
            encHeaderParam.pParaSet = malloc(STREAM_BUF_SIZE);
            encHeaderParam.headerMode = ENC_HEADER_MODE_NORMAL;
            encHeaderParam.quantMode = JPG_TBL_NORMAL;
            encHeaderParam.huffMode = JPG_TBL_NORMAL;
            encHeaderParam.disableAPPMarker = 0;
            encHeaderParam.enableSofStuffing = TRUE;

            if (encHeaderParam.pParaSet) {
                // make picture header
                JPU_EncGiveCommand(
                    chanIns->handle, ENC_JPG_GET_HEADER,
                    &encHeaderParam); // return exact header size int endHeaderparam.siz;
                JLOG(INFO, "JPU_EncGiveCommand[ENC_JPG_GET_HEADER] header size=%d\n",
                     encHeaderParam.size);

                chanIns->read_buffer_callback(chanIns->user_private_ptr, encHeaderParam.pParaSet,
                                          encHeaderParam.size);
                free(encHeaderParam.pParaSet);
            }

            // fill input buffer
            ret = jpu_enc_feed_input_buff(chanIns, &chanIns->enc_frameBuf[src_frame_idx]);
            if (ret < 0) {
                JLOG(ERR, "jpu_enc_feed_input_buff failed Error code is 0x%x \n", ret);
                return -1;
            }

            encParam.sourceFrame = &chanIns->enc_frameBuf[src_frame_idx];
        }

        // Start encoding a frame.
        ret = JPU_EncStartOneFrame(chanIns->handle, &encParam);

        if (ret != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncStartOneFrame failed Error code is 0x%x \n", ret);
            return -1;
        }

        while (1) {
            int_reason = hal_jpu_wait_interrupt(jpuDrv, JPU_INTERRUPT_TIMEOUT_MS);

            if (int_reason == -1) {
                JLOG(ERR, "[%s][%d] Error : timeout happened\n", __func__, __LINE__);
                JPU_SWReset(chanIns->handle);
                return -1;
            }

            if (int_reason == -2) {
                JLOG(ERR, "Interrupt occurred. but this interrupt is not for my instance\n");
                return -1;
            }

            // Must catch PIC_DONE interrupt before catching EMPTY interrupt
            if (int_reason & (1 << INT_JPU_DONE) || int_reason & (1 << INT_JPU_SLICE_DONE)) {
                break;
            }

            if (int_reason & (1 << INT_JPU_BIT_BUF_FULL)) {
                JLOG(INFO, "INT_JPU_BIT_BUF_FULL interrupt issued\n");
                send_output_buffer_to_user(chanIns, chanIns->bs_stream, NULL);

                JPU_ClrStatus(chanIns->handle, (1 << INT_JPU_BIT_BUF_FULL));
                JPU_EnableInterrput(chanIns->handle->JpgInfo->encInfo.intrEnableBit);
            }
        }

        if ((ret = JPU_EncGetOutputInfo(chanIns->handle, &outputInfo)) != JPG_RET_SUCCESS) {
            JLOG(ERR, "JPU_EncGetOutputInfo failed Error code is 0x%x \n", ret);
            return -1;
        }

        send_output_buffer_to_user(chanIns, chanIns->bs_stream, &outputInfo);

        if (outputInfo.encodeState == ENCODE_STATE_SLICE_DONE) {
            JLOG(INFO, "Enc Slice: %d, rdPtr=0x%x, wrPtr=0x%x, slice height=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.encodedSliceYPos);
        } else {
            JLOG(INFO, "Enc: %d, rdPtr=0x%x, wrPtr=0x%x cycles=%d\n", frameIdx,
                 outputInfo.streamRdPtr, outputInfo.streamWrPtr, outputInfo.frameCycle);
            frameIdx++;
            break;
        }
    }
    return 0;
}

static void jpu_workfunc(void* usrCtx)
{
    vpu_dev_t* dev = (vpu_dev_t *)usrCtx;
    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev->priv;
    jpu_chan_ins_t* chan = NULL;
    int32_t work_ret = -1;

    sdrv_jpu_ptr->is_running = true;
    while (sdrv_jpu_ptr->is_running) {
        int32_t ret = osEventFlagsWait(sdrv_jpu_ptr->work_event
                                , JPU_EVENT_WORK_REQ | JPU_EVENT_EXIT_NOTIFY
                                , osFlagsWaitAny
                                , 1000);
        if (ret == osErrorTimeout) {
            // time out
        }
        else if (ret & JPU_EVENT_WORK_REQ) {
            chan = NULL;
            if (osMessageQueueGet(sdrv_jpu_ptr->queue, &chan, 0, 0) != osOK) {
                JLOG(ERR, "get MessageQueue error\n");
                continue;
            }
            else if (chan != NULL){
                switch (chan->type) {
                    case 0:
                        work_ret = jpu_dec_and_wait_interrupt(chan, 0, sdrv_jpu_ptr);
                        break;
                    case 1:
                        work_ret = jpu_enc_and_wait_interrupt(chan, sdrv_jpu_ptr);
                        break;
                    default:
                        break;
                }

                // work status notify
                if (work_ret >= 0) {
                    if (chan->dec_output.decode_finish) {
                        osEventFlagsSet(chan->chan_event, CHAN_EVENT_WORK_BS_DONE);
                        work_ret = VPU_CODEC_RET_BITSTREAM_DONE;
                    }
                    else {
                        osEventFlagsSet(chan->chan_event, CHAN_EVENT_WORK_DONE);
                        work_ret = VPU_CODEC_RET_FRAME_DONE;
                    }
                }
                else {
                    osEventFlagsSet(chan->chan_event, CHAN_EVENT_WORK_ERR);
                    work_ret = VPU_CODEC_RET_ERROR;
                }

                // callback function for async interface notify
                if (chan->sync_callback != NULL) {
                    chan->sync_callback(chan->chan_id, work_ret, chan->sync_args);
                }

                // modify current channel working status
                osMutexAcquire(sdrv_jpu_ptr->mutex, osWaitForever);
                chan->is_working = false;
                osMutexRelease(sdrv_jpu_ptr->mutex);
            }
            else {
                // invalid channle
            }
        }
        else if (ret & JPU_EVENT_EXIT_NOTIFY) {
            JLOG(INFO, "exit event receive\n");
            sdrv_jpu_ptr->is_running = false;
            // exit workfunction
            break;
        }
        else {
            // do nothing
        }
    }

    osThreadExit();
}

static int32_t sdrv_jpu_init(vpu_dev_t* dev)
{
    if (dev == NULL || dev->priv == NULL) {
        JLOG(ERR, "patameter error for %s.\n", __func__);
        return -1;
    }

    // mutex init
    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev->priv;

    sdrv_jpu_ptr->jpu_event = osEventFlagsNew(NULL);
    sdrv_jpu_ptr->work_event = osEventFlagsNew(NULL);
    sdrv_jpu_ptr->mutex = osMutexNew(NULL);
    sdrv_jpu_ptr->chan_id = 0;
    memset(sdrv_jpu_ptr->chan, 0x00, sizeof(sdrv_jpu_ptr->chan));
    sdrv_jpu_ptr->queue = osMessageQueueNew(10, sizeof(void *), NULL);

    // init jpu hardware
    int32_t ret = -1;
    ret = JPU_Init(dev->cfg_ptr->base, JPU_REGISTER_SIZE);
    if (ret != JPG_RET_SUCCESS && ret != JPG_RET_CALLED_BEFORE) {
        JLOG(ERR, "JPU_Init failed Error code is 0x%x \n", ret);
        return ret;
    }

    irq_attach(dev->cfg_ptr->irq_num, jpu_interrupt_handler, sdrv_jpu_ptr);
    irq_enable(dev->cfg_ptr->irq_num);

    // get version
    JPU_GetVersionInfo(&sdrv_jpu_ptr->api_version
                        , &sdrv_jpu_ptr->hw_version
                        , &sdrv_jpu_ptr->product_id);
    if (sdrv_jpu_ptr->product_id != PRODUCT_ID_CODAJ12) {
        JLOG(ERR, "Error JPU HW_PRODUCT_ID=0x%x is not match with 0x%x\n"
                , sdrv_jpu_ptr->product_id
                , PRODUCT_ID_CODAJ12);
        return ret;
    }
    else {
        JLOG(INFO, "JPU Version API_VERSION=0x%x, HW_REVISION=%#x, HW_PRODUCT_ID=0x%x\n"
                    , sdrv_jpu_ptr->api_version
                    , sdrv_jpu_ptr->hw_version
                    , sdrv_jpu_ptr->product_id);
    }

    osThreadAttr_t thd_attr = {
        .name = "jpuTask",
        .stack_size = 4096,
    };
    sdrv_jpu_ptr->thread_id = osThreadNew(jpu_workfunc, (void *)dev, &thd_attr);
    return 0;
}

static int32_t decoder_optData_process(decoder_opt_t* optPtr, jpu_chan_ins_t* chanIns)
{
    chanIns->user_private_ptr = optPtr->user_private_ptr;
    chanIns->fill_buffer_callback = optPtr->fill_buffer_callback;
    chanIns->dec_feedong_method = optPtr->dec_feeding_method;
    chanIns->dec_output.output_cbcrInterleave = optPtr->output_cbcrInterleave;
    chanIns->dec_output.output_packed_format = optPtr->output_packed_format;
    chanIns->dec_output.output_format = optPtr->output_format;
    return 0;
}

static int32_t jpu_decChannel_init(jpu_chan_ins_t** chan, decoder_opt_t* optPtr)
{
    int feedingSize = 0;

    Uint32 framebufWidth = 0, framebufHeight = 0;
    Uint32 decodingWidth, decodingHeight;
    Uint32 displayWidth, displayHeight;

    FrameFormat output_format;
    JpgDecOpenParam decOP = {0};
    JpgDecInitialInfo initialInfo = {0};
    JpgRet ret = JPG_RET_SUCCESS;

    // alloc chan memory
    *chan = (jpu_chan_ins_t *)malloc(sizeof(jpu_chan_ins_t));
    memset(*chan, 0x00, sizeof(jpu_chan_ins_t));
    if (*chan == NULL) {
        JLOG(ERR, "alloc chan memory error for %s\n", __func__);
        return -1;
    }
    jpu_chan_ins_t* chan_ins = *chan;

    // alloc bs frame
    chan_ins->bs_stream.size = STREAM_BUF_SIZE;
    if (allocate_dma_memory(&chan_ins->bs_stream) < 0) {
        JLOG(ERR, "alloc bitstream buffer error for %s.\n", __func__);
        free(*chan);
        *chan = NULL;
        return -1;
    }

    // decoder option data process
    decoder_optData_process(optPtr, chan_ins);

    // set dec default values
    decOP.streamEndian = 0;
    decOP.frameEndian = 0;
    decOP.bitstreamBuffer = chan_ins->bs_stream.phys_addr;
    decOP.bitstreamBufferSize = chan_ins->bs_stream.size;
    // set virtual address mapped of physical address
    decOP.pBitStream = (BYTE *)chan_ins->bs_stream.virt_addr;   // lint !e511
    decOP.chromaInterleave =
        chan_ins->dec_output.output_cbcrInterleave;             // Cb data and Cr data are located in each separate plan
    decOP.packedFormat = chan_ins->dec_output.output_packed_format;  // PACKED_FORMAT_NONE for defalut
    decOP.roiEnable = 0;
    decOP.roiOffsetX = 0;
    decOP.roiOffsetY = 0;
    decOP.roiWidth = 0;
    decOP.roiHeight = 0;
    decOP.rotation = 0;
    decOP.mirror = 0;
    decOP.pixelJustification = 0;
    // defalut output format set to FORMAT_444
    // Only valid when output_packed_format is set to PACKED_FORMAT_NONE
    decOP.outputFormat =
        (chan_ins->dec_output.output_format == FORMAT_MAX) ? FORMAT_444 : chan_ins->dec_output.output_format;
    decOP.intrEnableBit =
        ((1 << INT_JPU_DONE) | (1 << INT_JPU_ERROR) | (1 << INT_JPU_BIT_BUF_EMPTY));

    JLOG(INFO, "------------------------------ DECODER OPTIONS ------------------------------\n");
    JLOG(INFO, "[streamEndian           ]: %d\n", decOP.streamEndian);
    JLOG(INFO, "[frameEndian            ]: %d\n", decOP.frameEndian);
    JLOG(INFO, "[chromaInterleave       ]: %d\n", decOP.chromaInterleave);
    JLOG(INFO, "[packedFormat           ]: %d\n", decOP.packedFormat);
    JLOG(INFO, "[roiEnable              ]: %d\n", decOP.roiEnable);
    JLOG(INFO, "[roiOffsetX             ]: %d\n", decOP.roiOffsetX);
    JLOG(INFO, "[roiOffsetY             ]: %d\n", decOP.roiOffsetY);
    JLOG(INFO, "[bitstreamBuffer        ]: 0x%08x\n", decOP.pBitStream);
    JLOG(INFO, "[bitstreamBufferSize    ]: %#x\n", decOP.bitstreamBufferSize);
    JLOG(INFO, "[roiWidth               ]: %d\n", decOP.roiWidth);
    JLOG(INFO, "[roiHeight              ]: %d\n", decOP.roiHeight);
    JLOG(INFO, "[rotation               ]: %d\n", decOP.rotation);
    JLOG(INFO, "[mirror                 ]: %d\n", decOP.mirror);
    JLOG(INFO, "[pixelJustification     ]: %d\n", decOP.pixelJustification);
    JLOG(INFO, "[outputFormat           ]: %d\n", decOP.outputFormat);
    JLOG(INFO, "[intrEnableBit          ]: %d\n", decOP.intrEnableBit);
    JLOG(INFO, "-----------------------------------------------------------------------------\n");

    // 1. open dec
    ret = JPU_DecOpen(&chan_ins->handle, &decOP);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecOpen failed Error code is 0x%x \n", ret);
        goto ERR_INIT_DEC;
    }

    // 2. fill input buffer to dec jpg header
    feedingSize = jpu_feeding_bs_buffer(chan_ins);
    if (feedingSize < 0) {
        JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", feedingSize);
        goto ERR_INIT_DEC;
    }
    ret = JPU_DecGetInitialInfo(chan_ins->handle, &initialInfo);
    if (ret != JPG_RET_SUCCESS) {
        JLOG(ERR, "JPU_DecGetInitialInfo failed Error code is 0x%x\n", ret);
        goto ERR_INIT_DEC;
    }

    JLOG(INFO, "-------------------------------- Initial Info -------------------------------\n");
    JLOG(INFO, "[source picture size  ]: (%d X %d)\n", initialInfo.picWidth, initialInfo.picHeight);
    JLOG(INFO, "[sourceFormat         ]: %d\n", initialInfo.sourceFormat);
    JLOG(INFO, "[needFrameBufCount    ]: %d\n", initialInfo.minFrameBufferCount);
    JLOG(INFO, "[bitDepth             ]: %d\n", initialInfo.bitDepth);
    JLOG(INFO, "-----------------------------------------------------------------------------\n");

    //@TODO alin is config
    // 3.dec params set
    bool align_to_16_bit = false;
    if (align_to_16_bit) {
        framebufWidth = JPU_CEIL(16, initialInfo.picWidth);
        framebufHeight = JPU_CEIL(16, initialInfo.picHeight);
    } else {
        if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_422 ||
            decOP.outputFormat == FORMAT_420 || decOP.outputFormat == FORMAT_422)
            framebufWidth = JPU_CEIL(16, initialInfo.picWidth);
        else
            framebufWidth = JPU_CEIL(8, initialInfo.picWidth);

        if (initialInfo.sourceFormat == FORMAT_420 || initialInfo.sourceFormat == FORMAT_440 ||
            decOP.outputFormat == FORMAT_420 || decOP.outputFormat == FORMAT_440)
            framebufHeight = JPU_CEIL(16, initialInfo.picHeight);
        else
            framebufHeight = JPU_CEIL(8, initialInfo.picHeight);
    }

    // TODO: support scale
    // scalerOn = (BOOL)(decConfig.iHorScaleMode || decConfig.iVerScaleMode);
    // decodingWidth = framebufWidth >> decConfig.iHorScaleMode; //0(none), 1(1/2), 2(1/4), 3(1/8)
    // decodingHeight = framebufHeight >> decConfig.iVerScaleMode; //0(none), 1(1/2), 2(1/4), 3(1/8)
    // if (decOP.packedFormat != PACKED_FORMAT_NONE && decOP.packedFormat != PACKED_FORMAT_444)
    // {
    // 	// When packed format, scale-down resolution should be multiple of 2.
    // 	decodingWidth = JPU_CEIL(2, decodingWidth);
    // }
    decodingWidth = framebufWidth;
    decodingHeight = framebufHeight;

    // TODO:support rotation
    // Uint32 temp = decodingWidth;
    // decodingWidth = (decOP.rotation == 90 || decOP.rotation == 270) ? decodingHeight :
    // decodingWidth; decodingHeight = (decOP.rotation == 90 || decOP.rotation == 270) ? temp :
    // decodingHeight;

    if (decOP.roiEnable == TRUE) {
        decodingWidth = framebufWidth = initialInfo.roiFrameWidth;
        decodingHeight = framebufHeight = initialInfo.roiFrameHeight;
    }

    // TODO: support scale
    // if (0 != decConfig.iHorScaleMode || 0 != decConfig.iVerScaleMode)
    // {
    //     displayWidth = JPU_FLOOR(2, (framebufWidth >> decConfig.iHorScaleMode));
    //     displayHeight = JPU_FLOOR(2, (framebufHeight >> decConfig.iVerScaleMode));
    // }
    // else
    {
        displayWidth = decodingWidth;
        displayHeight = decodingHeight;
    }

    output_format =
        (decOP.outputFormat == FORMAT_MAX) ? initialInfo.sourceFormat : decOP.outputFormat;

    chan_ins->dec_output.pic_width = initialInfo.picWidth;
    chan_ins->dec_output.pic_height = initialInfo.picHeight;
    chan_ins->dec_output.pic_format = initialInfo.sourceFormat;
    chan_ins->dec_output.output_width = decodingWidth;
    chan_ins->dec_output.output_height = decodingHeight;
    chan_ins->dec_output.output_cbcrInterleave = decOP.chromaInterleave;
    chan_ins->dec_output.output_packed_format = decOP.packedFormat;
    chan_ins->dec_output.output_format = output_format;
    chan_ins->dec_output.bit_depth = initialInfo.bitDepth;
    chan_ins->dec_output.output_frame_size = calculate_frame_size(
        chan_ins->dec_output.output_width, chan_ins->dec_output.output_height, chan_ins->dec_output.bit_depth,
        chan_ins->dec_output.output_format, chan_ins->dec_output.output_packed_format);

    //@TODO dec_to_fd is config
    // 4.alloca frame buffer and register to jpu
    bool dec_to_fd = false;
    if (!dec_to_fd) {
        ret = jpu_dec_register_frame_buffer(chan_ins, &chan_ins->dec_frame_buffer_stream, 0);
        if (ret < 0)
            goto ERR_INIT_DEC;
    }

    JLOG(INFO, "[source picture size  ]: (%d X %d)\n", initialInfo.picWidth, initialInfo.picHeight);
    JLOG(INFO, "[decode picture size  ]: (%d X %d)\n", decodingWidth, decodingHeight);
    JLOG(INFO, "[display width size   ]: (%d     )\n", displayWidth);
    JLOG(INFO, "[display height size  ]: (%d     )\n", displayHeight);
    JLOG(INFO, "[outputformat         ]: %d\n", output_format);
    JLOG(INFO, "[needFrameBufCount    ]: %d\n", initialInfo.minFrameBufferCount);
    JLOG(INFO, "[bitDepth             ]: %d\n", initialInfo.bitDepth);
    JLOG(INFO, "[packedFormat         ]: %d\n", decOP.packedFormat);

    return 0;
ERR_INIT_DEC:
    if (chan_ins->dec_frame_buffer_stream.size > 0) {
        free_dma_memory(&chan_ins->dec_frame_buffer_stream);
    }

    JPU_DecSetRdPtrEx(chan_ins->handle, chan_ins->bs_stream.phys_addr, TRUE);
    memset(chan_ins->dec_frameBuf, 0, sizeof(chan_ins->dec_frameBuf));
    JPU_DecClose(chan_ins->handle);

    // channel
    free(*chan);
    *chan = NULL;
    return -1;
}

static int32_t jpu_decChannel_reset(jpu_chan_ins_t* chan)
{
    int ret = 0;
    JpgDecInfo *decInfo;

    if (!chan || chan->bs_stream.size <= 0 || !chan->fill_buffer_callback) {
        JLOG(ERR, "Invalid jpu channel\n");
        return -1;
    }

    decInfo = &chan->handle->JpgInfo->decInfo;

    chan->dec_output.decode_finish = FALSE;
    JPU_DecSetRdPtrEx(chan->handle, decInfo->streamBufStartAddr, TRUE);
    decInfo->streamEndflag = 0;

    //feed bs buffer for next decode
    ret = jpu_feeding_bs_buffer(chan);
    if (ret < 0) {
        JLOG(ERR, "jpu_feeding_bs_buffer failed, errno :%d\n", ret);
        return ret;
    }

    return ret;
}

static int32_t encoder_optData_process(encoder_opt_t* optData, jpu_chan_ins_t* chanIns)
{
    chanIns->user_private_ptr = optData->user_private_ptr;
    chanIns->fill_buffer_callback = optData->fill_buffer_callback;
    chanIns->read_buffer_callback = optData->read_buffer_callback;
    chanIns->enc_input.pic_width = optData->pic_width;
    chanIns->enc_input.pic_height = optData->pic_height;
    chanIns->enc_input.frame_format = optData->frame_format;
    chanIns->enc_input.packageFormat = optData->package_format;

    return 0;
}

static int32_t jpu_encChannel_init(jpu_chan_ins_t** chan, encoder_opt_t* optPtr)
{
    // alloc chan memory
    *chan = (jpu_chan_ins_t *)malloc(sizeof(jpu_chan_ins_t));
    if (*chan == NULL) {
        JLOG(ERR, "alloc chan memory error for %s\n", __func__);
        return -1;
    }
    jpu_chan_ins_t* chan_ins = *chan;

    encoder_optData_process(optPtr, chan_ins);

    if (!chan_ins || chan_ins->bs_stream.size <= 0 || !chan_ins->fill_buffer_callback ||
        !chan_ins->read_buffer_callback || chan_ins->enc_input.pic_width <= 0 ||
        chan_ins->enc_input.pic_height <= 0) {
        JLOG(ERR, "jpu_enc_init Invalid input parameter\n");
        JLOG(ERR,
             "jpu:%p  jpu->bs_stream.size:%d jpu->fill_buffer_callback:%p, "
             "jpu->read_buffer_callback:%p "
             "jpu->enc_input.pic_width:%d jpu->enc_input.pic_height:%d\n",
             chan_ins, chan_ins->bs_stream.size, chan_ins->fill_buffer_callback, chan_ins->read_buffer_callback,
             chan_ins->enc_input.pic_width, chan_ins->enc_input.pic_height);
        return -1;
    }

    // alloc bs frame
    chan_ins->bs_stream.size = STREAM_BUF_SIZE;
    if (allocate_dma_memory(&chan_ins->bs_stream) < 0) {
        JLOG(ERR, "alloc bitstream buffer error for %s.\n", __func__);
        free(*chan);
        *chan = NULL;
        return -1;
    }

    if (jpu_enc_int(chan_ins) == 0) {
        return 0;
    }
    else {
        free(chan_ins);
        *chan = NULL;
        return -1;
    }
}

static int32_t sdrv_jpu_request_channel(vpu_dev_t* dev, int32_t chanType, void* optData)
{
    if (dev == NULL || optData == NULL || dev->priv == NULL) {
        JLOG(ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev;
    int32_t ret = -1;

    for (int32_t i = 0; i < MAX_JPU_CHAN_INS_NUM; i++) {
        if (sdrv_jpu_ptr->chan[i] == NULL) {
            switch (chanType) {
                case 0:
                    ret = jpu_decChannel_init(&sdrv_jpu_ptr->chan[i], optData);
                    break;
                case 1:
                    ret = jpu_encChannel_init(&sdrv_jpu_ptr->chan[i], optData);
                    break;
            }
            // channel init ok
            if (ret == 0) {
                sdrv_jpu_ptr->chan[i]->type = chanType;
                sdrv_jpu_ptr->chan[i]->chan_id = sdrv_jpu_ptr->chan_id++;
                sdrv_jpu_ptr->chan[i]->is_working = false;
                sdrv_jpu_ptr->chan[i]->chan_event = osEventFlagsNew(NULL);
                JLOG(ERR, "init %s channel %d sucess for jpu %s.\n"
                        , chanType == 0 ? "decoding" : "encoding"
                        , sdrv_jpu_ptr->chan[i]->chan_id
                        , dev->dev_name);
                return sdrv_jpu_ptr->chan[i]->chan_id;
            }
            else {  // channel init failed
                sdrv_jpu_ptr->chan[i] = NULL;
                JLOG(ERR, "init %s channel error for jpu %s.\n"
                        , chanType == 0 ? "decoding" : "encoding"
                        , dev->dev_name);
                return -1;
            }
        }
    }

    // chan init error
    JLOG(WARN, "there is no valid channel for jpu\n");
    return -1;
}

static int32_t jpu_channle_release(jpu_chan_ins_t** chan)
{
    if (*chan == NULL) {
        return -1;
    }

    jpu_chan_ins_t* chan_ins = *chan;

    if (chan_ins->type == 0 &&
        chan_ins->dec_frame_buffer_stream.size > 0) {
        free_dma_memory(&chan_ins->dec_frame_buffer_stream);
    }
    else if (chan_ins->type == 1 &&
        chan_ins->enc_frame_buffer_stream.size > 0) {
        free_dma_memory(&chan_ins->enc_frame_buffer_stream);
    }
    else {
        // do nothing
    }

    JPU_DecClose(chan_ins->handle);

    if (chan_ins->bs_stream.size > 0) {
        free_dma_memory(&chan_ins->bs_stream);
    }

    osEventFlagsDelete(chan_ins->chan_event);

    free(*chan);
    *chan = NULL;

    return 0;
}

static int32_t sdrv_jpu_release_channel(vpu_dev_t* dev, uint16_t chanId)
{
    if (dev == NULL || dev->priv == NULL) {
        JLOG(ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev->priv;
    jpu_chan_ins_t* chan_ins = NULL;

    for (int32_t i = 0; i < MAX_JPU_CHAN_INS_NUM; i++) {
        if ((chan_ins = sdrv_jpu_ptr->chan[i]) != NULL &&
            chan_ins->chan_id == chanId) {
            if (!chan_ins->is_working) {
                // release channel
                return jpu_channle_release(&sdrv_jpu_ptr->chan[i]);
            }
            else {
                JLOG(ERR, "vpu %s current channle %d is working\n", dev->dev_name, chan_ins->chan_id);
                return -1;
            }
        }
    }

    JLOG(ERR, "invalid channel id: %d of %s for release channel\n", chanId, dev->dev_name);
    return -1;
}

static jpu_chan_ins_t* jpu_chan_work_req(vpu_dev_t* dev, uint16_t chanId, void (cb)(uint16_t, int32_t, void*), void* usrCtx)
{
    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev->priv;

    osMutexAcquire(sdrv_jpu_ptr->mutex, osWaitForever);
    jpu_chan_ins_t* chan_ins = NULL;
    for (int32_t i = 0; i < MAX_JPU_CHAN_INS_NUM; i++) {
        if ((chan_ins = sdrv_jpu_ptr->chan[i]) != NULL &&
            chan_ins->chan_id == chanId) {
            if (chan_ins->is_working) {
                JLOG(WARN, "vpu %s channel %d current is working for %s\n"
                            , dev->dev_name
                            , chanId
                            , chan_ins->type == 0 ? "decoding" : "encoding");
                break;
            }
            else {
                osMessageQueuePut(sdrv_jpu_ptr->queue, &chan_ins, 0, 0);
                chan_ins->is_working = true;
                chan_ins->sync_callback = cb;
                chan_ins->sync_args = usrCtx;
                osMutexRelease(sdrv_jpu_ptr->mutex);
                // notify work task
                osEventFlagsSet(sdrv_jpu_ptr->work_event, JPU_EVENT_WORK_REQ);
                return chan_ins;
            }
        }
    }
    osMutexRelease(sdrv_jpu_ptr->mutex);
    return NULL;
}

static int32_t sdrv_jpu_chan_work_async(vpu_dev_t* dev, uint16_t chanId, void (cb)(uint16_t, int32_t, void*), void* usrCtx)
{
    if (dev == NULL || dev->priv == NULL) {
        JLOG(ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    if (jpu_chan_work_req(dev, chanId, cb, usrCtx) != NULL) {
        return 0;
    }
    else {
        return -1;
    }
}

static int32_t sdrv_jpu_chan_work_sync(vpu_dev_t* dev, uint16_t chanId)
{
    if (dev == NULL || dev->priv == NULL) {
        JLOG(ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    jpu_chan_ins_t* chan_ins = NULL;
    if ((chan_ins = jpu_chan_work_req(dev, chanId, NULL, NULL)) != NULL) {
        osEventFlagsClear(chan_ins->chan_event
                            , CHAN_EVENT_WORK_DONE | CHAN_EVENT_WORK_ERR | CHAN_EVENT_WORK_BS_DONE);
        int32_t ret_flgs = osEventFlagsWait(chan_ins->chan_event
                            , CHAN_EVENT_WORK_DONE | CHAN_EVENT_WORK_ERR | CHAN_EVENT_WORK_BS_DONE
                            , osFlagsWaitAny
                            , 1000);
        // channel return ok
        if (ret_flgs == osFlagsErrorTimeout) {
            return -1;
        }
        else if (ret_flgs & CHAN_EVENT_WORK_DONE) {
            return VPU_CODEC_RET_FRAME_DONE;
        }
        else if (ret_flgs & CHAN_EVENT_WORK_BS_DONE){
            return VPU_CODEC_RET_BITSTREAM_DONE;
        }
        else if (ret_flgs & CHAN_EVENT_WORK_ERR) {
            return VPU_CODEC_RET_ERROR;
        }
        else {
            return VPU_CODEC_RET_ERROR;
        }
    }
    else {
            return VPU_CODEC_RET_ERROR;
    }
}

static int32_t sdrv_jpu_deinit(vpu_dev_t* dev)
{
    if (dev == NULL || dev->priv == NULL) {
        JLOG(ERR, "parameter error for %s.\n", __func__);
        return -1;
    }

    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev->priv;
    for(int32_t i = 0; i < MAX_JPU_CHAN_INS_NUM; i++) {
        if (sdrv_jpu_ptr->chan[i] != NULL) {
            if (!sdrv_jpu_ptr->chan[i]->is_working) {
                jpu_channle_release(&sdrv_jpu_ptr->chan[i]);
            }
            else {
                JLOG(ERR, "error for deinit vpu %s, channel %d is working.\n"
                        , dev->dev_name
                        , sdrv_jpu_ptr->chan[i]->chan_id);
                return -1;
            }
        }
    }

    osEventFlagsSet(sdrv_jpu_ptr->work_event, JPU_EVENT_EXIT_NOTIFY);

    while (sdrv_jpu_ptr->is_running) {
        osDelay(1);
    }

    // relase os resource
    osEventFlagsDelete(sdrv_jpu_ptr->work_event);
    osEventFlagsDelete(sdrv_jpu_ptr->jpu_event);
    osMutexDelete(sdrv_jpu_ptr->mutex);
    osMessageQueueDelete(sdrv_jpu_ptr->queue);
    // osThreadTerminate(sdrv_jpu_ptr->thread_id);

    // disable irq
    irq_disable(dev->cfg_ptr->irq_num);

    // jpu device deinit
    JPU_DeInit();

    return 0;
}

jpu_chan_ins_t* check_chanId(vpu_dev_t* dev, uint16_t chanId)
{
    sdrv_jpu_drv_t* sdrv_jpu_ptr = (sdrv_jpu_drv_t *)dev->priv;
    jpu_chan_ins_t* chan_ins = NULL;

    for (int32_t i = 0; i < MAX_JPU_CHAN_INS_NUM; i++) {
        if ((chan_ins = sdrv_jpu_ptr->chan[i]) != NULL &&
            chan_ins->chan_id == chanId) {
            break;
        }
    }

    return chan_ins;
}

static int32_t sdrv_jpu_dev_ctrl(vpu_dev_t* dev, uint16_t chanId, uint16_t ctlCmd, void* ctlData)
{
    if (dev == NULL ||
        dev->priv == NULL) {
        JLOG(ERR, "parameter for %s\n", __func__);
        return -1;
    }

    jpu_chan_ins_t* chan_ins = NULL;
    switch (ctlCmd) {
        case VPU_CODEC_CTLCMD_GET_DECOUT_DATA:
            if ((chan_ins = check_chanId(dev, chanId)) == NULL) {
                JLOG(ERR, "invalid chanId: %d for control device %s\n", chanId, dev->dev_name);
                return -1;
            }
            else {
                if (chan_ins->is_working) {
                    JLOG(ERR, "current chanId: %d of device: %s is working\n", chanId, dev->dev_name);
                    return -1;
                }
                else if (ctlData != NULL) {
                    ((decoder_output_t *)(ctlData))->is_valid =
                        (chan_ins->dec_output.current_output_index >= 0 ? true : false);
                    ((decoder_output_t *)(ctlData))->y_ptr = chan_ins->dec_output.y_ptr;
                    ((decoder_output_t *)(ctlData))->u_ptr = chan_ins->dec_output.u_ptr;
                    ((decoder_output_t *)(ctlData))->v_ptr = chan_ins->dec_output.v_ptr;
                    ((decoder_output_t *)(ctlData))->output_height = chan_ins->dec_output.output_height;
                    ((decoder_output_t *)(ctlData))->output_width = chan_ins->dec_output.output_width;
                    ((decoder_output_t *)(ctlData))->pic_width = chan_ins->dec_output.pic_width;
                    ((decoder_output_t *)(ctlData))->pic_height = chan_ins->dec_output.pic_height;
                }
                else {
                    // ctlData ptr is invalid
                }
            }
            break;
        case VPU_CODEC_CTLCMD_DEC_RESET:
            {
                if ((chan_ins = check_chanId(dev, chanId)) == NULL) {
                    JLOG(ERR, "invalid chanId: %d for control device %s\n", chanId, dev->dev_name);
                    return -1;
                }
                else {
                    if (chan_ins->is_working) {
                        JLOG(ERR, "current chanId: %d of device: %s is working\n", chanId, dev->dev_name);
                        return -1;
                    }
                    else {
                        jpu_decChannel_reset(chan_ins);
                    }
                }
            }
            break;
        default:
            break;
    }

    return 0;

}

static vpu_ops_t jpu_ops = {
    .init = sdrv_jpu_init,
    .deinit = sdrv_jpu_deinit,
    .acquire_chan = sdrv_jpu_request_channel,
    .release_chan = sdrv_jpu_release_channel,
    .chan_codec_async = sdrv_jpu_chan_work_async,
    .chan_codec_sync = sdrv_jpu_chan_work_sync,
    .device_ctrl = sdrv_jpu_dev_ctrl,
};

vpu_dev_t *jpu_probe(sdrv_jpu_drv_t* jpuDrv, vpu_config_t* cfg)
{
    if (jpuDrv == NULL || cfg == NULL) {
        return NULL;
    }

    vpu_dev_t *dev = &jpuDrv->dev;
    dev->cfg_ptr = cfg;
    dev->dev_name = cfg->dev_name;
    dev->ops = &jpu_ops;
    dev->priv = jpuDrv;

    if (vpu_init(dev) == 0) {
        return dev;
    }
    else {
        return NULL;
    }
    return NULL;
}
/* end of file sdrv_jpu.c */
