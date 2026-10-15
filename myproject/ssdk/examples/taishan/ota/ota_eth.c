/**
 * @file ota_eth.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ota from usb host application.
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <types.h>
#include <param.h>
#include <board.h>
#include <debug.h>
#include <CLI_type.h>
#include <arch/cache.h>
#include <cmsis_os2.h>
#include <lib/cksum.h>
#include <lib/sd_boot_img/sd_boot_img.h>
#include <lib/disk/disk.h>
#include "ota.h"
#include "lwip/apps/tftp_client.h"
#include "lwip/apps/tftp_server.h"
#include "ota_eth.h"

#if LWIP_UDP

/* Define this to a server IP string */
#ifndef CONFIG_OTA_TFTP_SERVER_IP
#define CONFIG_OTA_TFTP_SERVER_IP "10.18.252.75"
#endif

#define DATA_TRANS_SZ 2048
#define FILE_BUF_SZ (BPT_SIZE + DATA_TRANS_SZ)

struct __file_content;
static void tftp_close(void *handle);
static void *tftp_open(const char *fname, const char *mode, u8_t is_write);
static int tftp_read(void *handle, void *buf, int bytes);
static int tftp_write(void *handle, struct pbuf *p);
static void tftp_error(void *handle, int err, const char *msg, int size);

typedef int (*ota_write)(struct __file_content *f, uint8_t *pbuf,
                         uint32_t pbuf_sz);

typedef struct __file_content {
    uint32_t sz;
    uint32_t bpt_parsed;
    uint8_t *buf;
    DISK_STATE_T *ds;
    ota_write write;
    uint8_t max_bpt_num;
    uint8_t min_bpt_num;
    uint8_t inst_num;
    uint8_t init_done;
    struct MD5Context context;
    char *file_name;
} file_content;

static const struct tftp_context tftp = {
    tftp_open,
    tftp_close,
    tftp_read,
    tftp_write,
    tftp_error
};

static file_content file;
extern uint8_t md5_write[MD5_LEN];

/**
 * @brief align the "size" to "aligned" boundry
 * @param size iput number
 * @param aligned  aligned number
 * @return uint64_t lowest common multiple
 */
static uint64_t round_up(uint64_t size, uint64_t aligned)
{
    uint64_t mod = 0;

    if (aligned == 0 || size < aligned)
        return aligned;

    /* Sometimes, 'aligned' is not equal to power of 2 */
    mod = size % aligned;

    size += mod ? aligned - mod : 0;
    return size;
}

/**
 * @brief validate the remote file's bpt
 * @param f pointer to the file (the handle)
 * @param bpt_buf recived data buffer
 * @param size data buffer size
 * @return int 0 is successful, other is fail
 */
static int validate_bpt(file_content *f, const uint8_t *bpt_buf, uint32_t size)
{
    int ret = -1;
    struct bpt *bpt = NULL;
    uint32_t crc32_val = 0;
    DISK_STATE_T *ds = f->ds;
    uint8_t max_bpt_num = f->max_bpt_num;
    uint8_t min_bpt_num = f->min_bpt_num;

    if (!bpt_buf || size < BPT_SIZE ) {
        ERROR("bpt calloc failed\n");
        goto end;
    }

    bpt = (struct bpt *)osAllocAlign(512, sizeof(struct bpt));

    if (!bpt) {
        ERROR("bpt calloc failed\n");
        goto end;
    }

    memset((uint8_t *)bpt, 0, sizeof(struct bpt));

    if (0 != get_bpt_info(bpt, (uint8_t *)bpt_buf, size)) {
        ERROR("get bpt info from file failed\n");
        goto end;
    }

    crc32_val = sfs_crc32(0, (uint8_t *)bpt_buf, BPT_CRC32_OFFSET);

    DBG("update file bpt->tag                  = 0x%08x\n", bpt->tag);
    DBG("update file bpt->size                 = 0x%x\n",   bpt->size);
    DBG("update file bpt->sec_version          = 0x%08x\n", bpt->sec_version);
    DBG("update file bpt->hash_alg             = 0x%x\n",   bpt->hash_alg);
    DBG("update file bpt->crc32                = 0x%08x(crc32_val=0x%08x)\n",
        bpt->crc32, crc32_val);
    DBG("update file bpt->pac_serial_num       = 0x%08x\n", bpt->pac_serial_num);
    DBG("update file bpt->inver_pac_serial_num = 0x%08x\n",
        bpt->inver_pac_serial_num);

    if (crc32_val != bpt->crc32) {
        ERROR("CRC error, bpt->crc32=0x%08x, crc32_val=0x%08x\n", bpt->crc32,
              crc32_val);
        goto end;
    }

    if (check_bpt_for_update(ds, bpt, max_bpt_num, min_bpt_num)) {
        ERROR("ota check error\n");
        goto end;
    }

    ret = 0;
end:

    if (bpt)
        osFree(bpt);

    return ret;
}

/**
 * @brief ota write
 * @param f pointer to the file (the handle)
 * @param pbuf recived data buffer
 * @param pbuf_sz data buffer size
 * @return int 0 is successful, other is fail
 */
static int ota_file_write(file_content *f, uint8_t *pbuf, uint32_t pbuf_sz)
{
    DISK_STATE_T *ds = f->ds;
    uint32_t sector_size = 0;
    uint32_t block_size = 0;
    uint8_t min_bpt_num = INVALID_BPT_NUMBER;
    uint8_t inst_num = 0;
    static uint64_t addr = INVALID_VERSION;

    if (!f || !pbuf) {
        DBG("invalid params\n");
        return -1;
    }

    if (pbuf_sz == 0)
        return 0;

    ASSERT(pbuf_sz <= DATA_TRANS_SZ);
    ASSERT(pbuf_sz % ds->block_size == 0);
    ASSERT(f->sz % ds->block_size == 0);

    sector_size = ds->erase_size;
    block_size = ds->block_size;
    min_bpt_num = f->min_bpt_num;
    inst_num = f->inst_num;

    if (f->sz < BPT_SIZE) {
        ASSERT((FILE_BUF_SZ - f->sz) >= pbuf_sz);
        memcpy(f->buf + f->sz, pbuf, pbuf_sz);
        f->sz += pbuf_sz;
        DBG("received:%d bytes\n", f->sz);
        return 0;
    }
    else {
        if (!f->bpt_parsed) {

            if (validate_bpt(f, f->buf, f->sz)) {
                DBG("failed to validate_bpt\n");
                return -1;
            }

            f->bpt_parsed = 1;

            addr = ds->bpt_info[min_bpt_num].bpt_addr;
            DBG("target addr 0x%llx\n", addr);

            if (strstr(ds->disk_name, "flash")) {
                ASSERT(addr % sector_size == 0);
                DBG("erase addr 0x%llx, len 0x%llx......\n", addr, round_up(f->sz,
                        sector_size));

                if (disk_erase_group(ds->disk_inst[inst_num], addr / sector_size,
                                     round_up(f->sz, sector_size) / sector_size, DISK_ERASE_DEFAULT)) {
                    ERROR("disk_erase_group addr = 0x%llx error\n", addr);
                    return -1;
                }
            }

            ASSERT(addr % block_size == 0);
            DBG("write addr 0x%llx, len 0x%x......\n", addr, f->sz);

            if (disk_write_block(ds->disk_inst[inst_num], addr / block_size,
                                 (uint8_t *)f->buf, f->sz / block_size)) {
                ERROR("write data error!\n");
                return -1;
            }

            MD5Update(&(f->context), (uint8_t *)f->buf, f->sz);
        }

        addr = ds->bpt_info[min_bpt_num].bpt_addr + f->sz;
        memcpy(f->buf, pbuf, pbuf_sz);

        if (f->sz > ds->bpt_info[min_bpt_num].max_size) {
            DBG("file size = %d is bigger than ds->bpt_info[%d].max_size = %lld\n",
                f->sz, min_bpt_num, ds->bpt_info[min_bpt_num].max_size);
            return -1;
        }

        if (strstr(ds->disk_name, "flash")) {
            if (addr % sector_size == 0) {
                DBG("erase addr 0x%llx, len 0x%llx......\n", addr, round_up(pbuf_sz,
                        sector_size));

                if (disk_erase_group(ds->disk_inst[inst_num], addr / sector_size,
                                     round_up(pbuf_sz, sector_size) / sector_size, DISK_ERASE_DEFAULT)) {
                    ERROR("disk_erase_group addr = 0x%llx error\n", addr);
                    return -1;
                }
            }
        }

        ASSERT(addr % block_size == 0);
        DBG("write addr 0x%llx, len 0x%x......\n", addr, pbuf_sz);

        if (disk_write_block(ds->disk_inst[inst_num], addr / block_size,
                             (uint8_t *)f->buf, pbuf_sz / block_size)) {
            ERROR("write data error!\n");
            return -6;
        }

        MD5Update(&(f->context), (uint8_t *)f->buf, pbuf_sz);

        f->sz += pbuf_sz;
        return 0;
    }
}

/**
 * @brief create a tftp handle
 *
 * @param ds download disk state
 * @param fname remote file name
 * @param max_bpt_num the boot package with the biggest PSN
 * @param min_bpt_num the boot package with the minimum PSN
 * @param inst_num the disk inst number
 * @return void* handle point to the file_content
 */
void *create_tftp_handle(DISK_STATE_T *ds, const char *fname,
                         uint8_t max_bpt_num, uint8_t min_bpt_num, uint8_t inst_num)
{
    if (!ds) {
        ERROR("ds is NULL\n");
        return NULL;
    }

    memset(&file, 0x0, sizeof(file_content));
    file.buf = osAllocAlign(512, FILE_BUF_SZ);
    file.write = ota_file_write;
    file.ds = ds;
    file.max_bpt_num = max_bpt_num;
    file.min_bpt_num = min_bpt_num;
    file.inst_num = inst_num;
    file.init_done = 1;
    file.file_name = (char *)fname;
    memset(md5_write, 0, MD5_LEN);
    MD5Init(&(file.context));

    return (void *)&file;
}

/**
 * @brief tftp open
 *
 * @param fname file name
 * @param mode
 * @param is_write
 * @return the file handle
 */
static void *tftp_open(const char *fname, const char *mode, u8_t is_write)
{
    LWIP_UNUSED_ARG(mode);
    DBG("tftp_open\n");

    if (file.init_done)
        return (void *)&file;
    else
        return NULL;
}

/**
 * @brief tftp close
 *
 * @param handle tftp handle
 */
static void tftp_close(void *handle)
{
    file_content *f = (file_content *)handle;
    DISK_STATE_T *ds = f->ds;

    MD5Final(md5_write, &(f->context));
    md5_read_check(ds, f->inst_num, ds->bpt_info[f->min_bpt_num].bpt_addr, f->sz);

    if (f && f->buf) {
        DBG("file sz:%d\n", f->sz);

        if (f->sz < BPT_SIZE) {
            DBG("the received file has no invalid bpt header!\n");
        }

        osFree(f->buf);
    }

    file.ds = NULL;
    memset(&f, 0x0, sizeof(file_content));
    return;
}

/**
 * @brief tftp_read and transfor to remote server
 *
 * @param handle tftp handle
 * @param buf read buffer
 * @param bytes read length
 * @return int read cnt
 */
static int tftp_read(void *handle, void *buf, int bytes)
{
    DBG("tftp_read\n");
    return bytes;
}

/**
 * @brief tfrp write to local storage
 *
 * @param handle tftp handle
 * @param p data pointer
 * @return int 0 is success , other is failed
 */
static int tftp_write(void *handle, struct pbuf *p)
{
    file_content *f = (file_content *)handle;

    while (p != NULL) {
        if (f->write(f, p->payload, p->len)) {
            return -1;
        }

        p = p->next;
    }

    return 0;
}

/**
 * @brief tftp error
 *
 * @param handle tftp handle
 * @param err
 * @param msg
 * @param size
 */
static void tftp_error(void *handle, int err, const char *msg, int size)
{
    char message[100];

    LWIP_UNUSED_ARG(handle);

    memset(message, 0, sizeof(message));
    MEMCPY(message, msg, LWIP_MIN(sizeof(message) - 1, (size_t)size));

    DBG("TFTP error: %d (%s)", err, message);
}


/**
 * @brief tftp client init
 * @param handle tftp handle
 * @return uint8_t 0 is success , other is failed
 */
uint8_t ota_tftp_init_client(void *handle)
{
    err_t err;
    ip_addr_t srv;
    static bool client_inited = false;

    if (!handle || !((file_content *)handle)->init_done) {
        ERROR("handle is null or handle not init yet\n");
        return 1;
    }

    if (1 != ipaddr_aton(CONFIG_OTA_TFTP_SERVER_IP, &srv)) {
        ERROR("ipaddr_aton failed\n");
        return 1;
    }

    if (!client_inited) {
        err = tftp_init_client(&tftp);

        if (ERR_OK != err) {
            ERROR("tftp_init_client failed\n");
            return 1;
        }

        client_inited = true;
    }

    err = tftp_get(handle, &srv, TFTP_PORT, ((file_content *)handle)->file_name,
                   TFTP_MODE_OCTET);

    if (ERR_OK != err) {
        ERROR("tftp_get failed\n");
        return 1;
    }

    return 0;
}

#endif
