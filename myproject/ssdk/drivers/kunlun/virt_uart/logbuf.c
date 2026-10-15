#include <stdlib.h>
#include <debug.h>
#include <string.h>
#include <assert.h>

#include "driver/kunlun/virt_uart/logbuf.h"

#define SDRV_LOGBUF_MAGIC		0x20210618
#define SDRV_LOGBUF_BANK_HEADER_SIZE    0x40
#define SDRV_LOGBUF_HEADER_SIZE		0x80

/* bank buffer flag */
#define SDLOG_BANK_BUF_FLAG_OVERRIDE    (1 << 0)

/* timestamp feature */
#define SDRV_LOGBUF_FEATURE_TIMESTAMP	(1 << 0)

#ifndef IS_POWER_OF_2
#define IS_POWER_OF_2(x) ((x) > 0 && (((x) & ((x) - 1)) == 0))
#endif

/*
 * to keep the following structure alignment, we need follow
 * points below:
 * - use architecture-independment types (eg: int, char)
 * - use single-byte or four-byte alignment
 */

/* reserved 128 byte to describe the logbuf header */
typedef union logbuf_hdr {
    struct logbuf_header {
        uint32_t magic;
        uint32_t logoff[SDRV_CORE_MAX];
        uint32_t logsize[SDRV_CORE_MAX];
        uint32_t status;
        uint32_t uart_mask;
        uint32_t feature;
    } header;
    uint8_t pad[SDRV_LOGBUF_HEADER_SIZE];
} logbuf_hdr_t;

/* reserved 64 byte to describe per bank header */
typedef union logbuf_bk_hdr {
    struct logbuf_bk_header {
        uint32_t tail;      /* read  counter */
        uint32_t tail1;     /* read  counter */
        uint32_t head;      /* write counter */
        uint32_t size;      /* bank buffer size */
        uint32_t linepos;       /* pos value corresponding to '\n' */
        uint32_t flag;
    } header;
    uint8_t pad[SDRV_LOGBUF_BANK_HEADER_SIZE];
} logbuf_bk_hdr_t;

void *sdrv_logbuf_get_bank_hdr(core_type_t ctype)
{
    logbuf_bk_hdr_t *bank_hdr;

    logbuf_hdr_t *hdr = (logbuf_hdr_t *)SDRV_LOGBUF_HEADER;

      /* invalid logbuf header */
    if (SDRV_LOGBUF_HEADER == 0x0) {
        ssdk_printf(SSDK_ALERT, "invalid logbuf header\n");
        return NULL;
     }

    if ((hdr->header.magic != SDRV_LOGBUF_MAGIC) || (ctype >= SDRV_CORE_MAX)) {
        ssdk_printf(SSDK_ALERT, "magic and ctype error\n");
        return NULL;
    }

    bank_hdr = (logbuf_bk_hdr_t *)((long)(SDRV_LOGBUF_HEADER +
                                          hdr->header.logoff[ctype]));

    /* the bank buffer size must be a power of two */
    bank_hdr->header.size = hdr->header.logsize[ctype] -
                            SDRV_LOGBUF_BANK_HEADER_SIZE;

    if (!IS_POWER_OF_2(bank_hdr->header.size))
        return NULL;

    return (void *)bank_hdr;
}

int sdrv_logbuf_putc(void *hdr, char c)
{
    char *bbuf;
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    bbuf = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
    bbuf[bk_hdr->header.head] = c;

    if (c == '\n')
        bk_hdr->header.linepos = bk_hdr->header.head;

    bk_hdr->header.head++;

    if (bk_hdr->header.head >= bk_hdr->header.size)
        bk_hdr->header.flag |= SDLOG_BANK_BUF_FLAG_OVERRIDE;

    bk_hdr->header.head = bk_hdr->header.head & (bk_hdr->header.size - 1);
    return 0;
}

/* copy entire ring buffer in the two segments */
int sdrv_logbuf_get_last_buf_and_size(void *hdr, char **buf, uint32_t* size, uint32_t len)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;

    if (!buf || !size)
        return -1;

    if (len != SDRV_LOGBUG_SEGMENT_NUM)
        return -1;

    if (bk_hdr->header.flag & SDLOG_BANK_BUF_FLAG_OVERRIDE) {
        buf[0] = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE + bk_hdr->header.head);
        size[0] = bk_hdr->header.size - bk_hdr->header.head;
        buf[1] = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
        size[1] = bk_hdr->header.head;
    }
    else {
        buf[0] = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
        size[0] = bk_hdr->header.head;
        buf[1] = NULL;
        size[1] = 0;
    }

    return 0;
}

int sdrv_logbuf_get_new_buf(void *hdr, char *buf, uint32_t size)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    char *dsrc = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
    uint32_t buf_len = 0;

    if (!buf)
        return -1;

    while ((bk_hdr->header.tail != bk_hdr->header.head) && (buf_len < size)) {
        buf[buf_len++] = dsrc[bk_hdr->header.tail];
        bk_hdr->header.tail++;
        bk_hdr->header.tail = bk_hdr->header.tail & (bk_hdr->header.size - 1);
    }

    return buf_len;
}

int sdrv_logbuf_update_tail(void *hdr)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    bk_hdr->header.tail = bk_hdr->header.linepos + 1;
    return 0;
}

void sdrv_logbuf_clear(void *hdr)
{
    logbuf_bk_hdr_t *bk_hdr = (logbuf_bk_hdr_t *)hdr;
    char *buf = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
    int i = 0;

    do {
        bk_hdr->header.head = 0;
        bk_hdr->header.tail = 0;
        bk_hdr->header.tail1 = 0;
        bk_hdr->header.linepos = 0;
        bk_hdr->header.flag = 0;
        memset(buf, 0x0, bk_hdr->header.size);
    } while ((i++ < 5) && ((bk_hdr->header.head != 0)
             || (bk_hdr->header.flag != 0)
             || (bk_hdr->header.tail != 0)
             || (bk_hdr->header.tail1 != 0)));
}

int sdrv_logbuf_is_enable(void)
{
    logbuf_hdr_t *hdr = (logbuf_hdr_t *)SDRV_LOGBUF_HEADER;

    if (hdr->header.magic != SDRV_LOGBUF_MAGIC)
        return false;

    return hdr->header.status;
}

int timestamp_is_enable(void)
{
    logbuf_hdr_t *hdr = (logbuf_hdr_t *)SDRV_LOGBUF_HEADER;

    if (hdr->header.magic != SDRV_LOGBUF_MAGIC)
        return false;

    return !!(hdr->header.feature | SDRV_LOGBUF_FEATURE_TIMESTAMP);
}

void sdrv_logbuf_clear_buf(void* hdr)
{
    logbuf_bk_hdr_t* bk_hdr = hdr;
    char* buf = (char*)((long)bk_hdr+SDRV_LOGBUF_BANK_HEADER_SIZE);
    int i = 0;

    do {
        bk_hdr->header.head = 0;
        bk_hdr->header.tail = 0;
        bk_hdr->header.tail1 = 0;
        bk_hdr->header.linepos = 0;
        bk_hdr->header.flag = 0;
        memset(buf, 0x0, bk_hdr->header.size);
    } while ((i++ < 5) && ((bk_hdr->header.head != 0)
             || (bk_hdr->header.flag != 0)
             || (bk_hdr->header.tail != 0)
             || (bk_hdr->header.tail1 != 0)));
}

int sdrv_logbuf_get_new_log_size(void* hdr)
{
    logbuf_bk_hdr_t* bk_hdr = hdr;
    int delta;

    if (bk_hdr->header.head >= bk_hdr->header.tail1)
        delta = bk_hdr->header.head - bk_hdr->header.tail1;
    else
        delta = bk_hdr->header.size + bk_hdr->header.head -
                bk_hdr->header.tail1;

    return delta;
}

int sdrv_logbuf_get_bank_buf_size(void* hdr)
{
    if (hdr) {
        logbuf_bk_hdr_t* bk_hdr = hdr;
        return bk_hdr->header.size;
    }
    return -1;
}

int sdrv_logbuf_get_new_buf_and_size(void* hdr, char** buf, int* size, int len)
{
    logbuf_bk_hdr_t *bk_hdr = hdr;

    if (!buf || !size)
        return -1;

    if (len != SDRV_LOGBUG_SEGMENT_NUM)
        return -1;

    if (bk_hdr->header.head >= bk_hdr->header.tail1) {
        buf[0] = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE +
                        bk_hdr->header.tail1);
        size[0] = bk_hdr->header.head - bk_hdr->header.tail1;
        buf[1] = NULL;
        size[1] = 0;
    } else {
        buf[0] = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE +
                        bk_hdr->header.tail1);
        size[0] = bk_hdr->header.size - bk_hdr->header.tail1;
        buf[1] = (char *)((long)bk_hdr + SDRV_LOGBUF_BANK_HEADER_SIZE);
        size[1] = bk_hdr->header.head;
    }

    return 0;
}

void sdrv_logbuf_update_bk_tail1(void* hdr)
{
    logbuf_bk_hdr_t* bk_hdr = hdr;

    /* used to mark the start position of the next copy */
    bk_hdr->header.tail1 = bk_hdr->header.head;
}
