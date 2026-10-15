/**
 * @file mjpeg.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <cmsis_os2.h>
#include <stdio.h>
#include <string.h>

#include "mjpeg.h"

#include "ff.h"

static const uint8_t jpg_signature[] = {0xFF, 0xD8};

static inline size_t mjpeg_in_func(JDEC *jd, uint8_t *buff, size_t nbyte)
{
    jpg_session *dev = (jpg_session *)jd->device;

    if (buff) {
        if (dev->bits_offset + nbyte > dev->bit_size) {
            nbyte = dev->bit_size - dev->bits_offset;
        }
        memcpy(buff, dev->bit_stream + dev->bits_offset, nbyte);
        dev->bits_offset += nbyte;

        return nbyte;
    } else { /* skip data from input stream */
        if (dev->bits_offset + nbyte > dev->bit_size) {
            nbyte = dev->bit_size - dev->bits_offset;
        }
        dev->bits_offset += nbyte;

        return nbyte;
    }
}

static inline jformat mjpeg_get_subsample_format(JDEC *jdec)
{
    uint8_t sample = 0;

    sample = (jdec->msx << 4) | jdec->msy;

    if (sample == 0x11)
        return JDF_444;
    else if (sample == 0x21)
        return JDF_422;
    else if (sample == 0x22)
        return JDF_420;

    return JDF_420; /* default to yuv420 now */
}

/* if the input is from the file, load the file into ram,
   otherwise use the input data directly
*/
jpg_session *mjpeg_dec_init(io_source_type type, const void *data,
                            uint32_t size)
{
    jpg_session *dec_session = NULL;
    if (data == NULL)
        return NULL;

    dec_session = (jpg_session *)osAlloc(sizeof(jpg_session));
    if (dec_session == NULL)
        return NULL;

    memset(dec_session, 0x00, sizeof(jpg_session));
    dec_session->src_type = type;
    dec_session->bits_offset = 0;
    dec_session->work_buf = osAlloc(MJPEG_WORKBUFF_SIZE);
    if (dec_session->work_buf == NULL) {
        osFree(dec_session);
        return NULL;
    }

    if (dec_session->src_type == JPEG_IO_SOURCE_C_ARRAY) {
        // input data is stored in the ram already
        dec_session->bit_stream = (uint8_t *)data;
        dec_session->bit_size = size;
    } else if (dec_session->src_type == JPEG_IO_SOURCE_DISK) {
        FIL fp;
        FRESULT rc;
        UINT count = 0;
        char file_name[128] = {0};
        strcpy(file_name, (char *)data);
        rc = f_open(&fp, file_name, FA_READ);

        if (rc) {
            osFree(dec_session->work_buf);
            osFree(dec_session);
            printf("file open failed (rc=%d)\n", rc);
            return NULL;
        }

        dec_session->bit_size = f_size(&fp);

        dec_session->bit_stream =
            (uint8_t *)osAllocAlign(32, dec_session->bit_size);
        f_read(&fp, dec_session->bit_stream, dec_session->bit_size, &count);
        f_close(&fp); /* close the MJPEG file */
    }

    return dec_session;
}

JRESULT mjpeg_dec_get_header_info(jpg_session *dec_session,
                                  jpg_out_info *out_info)
{
    JRESULT res;

    /* prepare to decompress header info*/
    res = jd_prepare(&dec_session->jdec, mjpeg_in_func, dec_session->work_buf,
                     MJPEG_WORKBUFF_SIZE, dec_session);

    if (res == JDR_OK) {
        /* it is ready to dcompress and image info is available here */
        out_info->width = dec_session->jdec.aligned_width;
        out_info->height = dec_session->jdec.aligned_height;
        out_info->crop.x = 0;
        out_info->crop.y = 0;
        out_info->crop.w = dec_session->jdec.width;
        out_info->crop.h = dec_session->jdec.height;
        out_info->format = mjpeg_get_subsample_format(&dec_session->jdec);
        dec_session->first_parse = 1;
    } else {
        printf("jd_prepare() failed (rc=%d)\n", res);
    }

    return res;
}

JRESULT mjpeg_dec_parser_next_frame(jpg_session *dec_session)
{
    JRESULT res;
    uint32_t search_range = dec_session->bit_size;
    uint8_t *bits = NULL;

    if (dec_session->first_parse) {
        /* prepare return with ok, so we do not need to parser the first frame*/
        dec_session->first_parse = 0;
        return JDR_OK;
    }

    /* the previous decoding might have read more data than it needed, rewind */
    dec_session->bits_offset -= CONFIG_JD_SZBUF;

    if (dec_session->bits_offset + sizeof(jpg_signature) <
        dec_session->bit_size) {
        search_range -= sizeof(jpg_signature);
    } else {
        return JDR_INP;
    }

    for (bits = dec_session->bit_stream + dec_session->bits_offset;
         dec_session->bits_offset < search_range;
         dec_session->bits_offset++, bits++) {

        if (bits[0] == 0xFF && bits[1] == 0xD8) {
            /* find the start of next jpeg */
            res = jd_prepare(&dec_session->jdec, mjpeg_in_func,
                             dec_session->work_buf, MJPEG_WORKBUFF_SIZE,
                             dec_session);
            if (res == JDR_OK) {
                return JDR_OK;
            } else {
                return JDR_INP;
            }
        }
    }

    return JDR_INP;
}

JRESULT mjpeg_dec_process_frame(jpg_session *dec_session, uint8_t *buf,
                                uint32_t y_size, uint32_t u_size)
{
    JRESULT res;

    res = jd_decomp_opt(&dec_session->jdec, buf, buf + y_size,
                        buf + y_size + u_size);
    if (res == JDR_OK) {
        /* the decompressed image in the frame buffer here. */
        // printf("\rDecompression succeeded.\n");
    } else {
        printf("jd_decomp() failed (rc=%d)\n", res);
    }

    return res;
}

void mjpeg_dec_deinit(jpg_session *dec_session)
{
    if (dec_session && dec_session->work_buf)
        osFree(dec_session->work_buf);

    if (dec_session && dec_session->src_type == JPEG_IO_SOURCE_DISK &&
        dec_session->bit_stream)
        osFree(dec_session->bit_stream);

    osFree(dec_session);
}
