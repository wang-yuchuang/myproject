#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <debug.h>
#include <types.h>
#include <param.h>
#include <board.h>
#include <malloc.h>
#include <lib/disk/disk.h>
#include <lib/partition/partition_parser.h>
#include "str_store.h"
#include "str_internal.h"
#include "mmc_cfg.h"

#define STR_TASK_SIZE   2048

static char init_emmc[]   = "init_emmc";   /* safety to secure */
static char write_emmc[]  = "write_emmc";  /* safety to secure */
static char emmc_inited[] = "emmc_inited"; /* secure to safety */
static char save_comp[]   = "save_comp";   /* secure to safety */
static char emmc_writed[] = "emmc_writed"; /* secure to safety */
static char msg_unknow[]  = "msg_unknow";  /* secure to safety */

static struct rpmsg_channel_t *str_chan = NULL;

static DISK_INFO_T *disk;
static uint64_t ptn;
static uint64_t size;

static int commu_state;
static int wr_count;

static char *str_rbuf;
static char *str_wbuf;
static uint32_t buf_sz;

static void str_mmc_init(const char *name)
{
    int bk_size;

    disk_init();

    board_mmc_init();

    disk = disk_dev_init(name);
    if (!disk)
        return;

    size = disk_ptdev_get_ptn_and_size(disk, "ddr_ioretention", &ptn);
    if (size < 0)
        return;

    bk_size = disk_get_block_size(disk->disk_inst);
    buf_sz = ROUNDUP(sizeof(struct str_info_t), bk_size);
    str_rbuf = memalign(CACHE_LINE, buf_sz); /* for dma transform, not use malloc(buf_sz); */
    if (!str_rbuf) {
        ssdk_printf(SSDK_ALERT, "failed to allocate str read bufffer\n");
        return;
    }
    str_wbuf = memalign(CACHE_LINE, buf_sz); /* for dma transform, not use malloc(buf_sz); */
    if (!str_wbuf) {
        free(str_rbuf);
        str_rbuf = NULL;
        ssdk_printf(SSDK_ALERT, "failed to allocate str write bufffer\n");
        return;
    }
    memset(str_rbuf, 0x0, buf_sz);
    memset(str_wbuf, 0x0, buf_sz);

    if (disk_read(disk->disk_inst, ptn, (uint8_t *)str_rbuf, buf_sz)) {
        free(str_wbuf);
        free(str_rbuf);
        str_wbuf = NULL;
        str_rbuf = NULL;
        ssdk_printf(SSDK_ALERT, "read storage failed\n");
        return;
    }

    sms_send_msg(str_chan, emmc_inited, strlen(emmc_inited));
}

static int str_event_handle(void *data, int len, unsigned long src,
                            void *arg)
{
    int i;
    struct str_info_t *p1 = (struct str_info_t *)str_rbuf;
    struct str_info_t *p2 = (struct str_info_t *)str_wbuf;

    if (data) {
        switch (commu_state) {
            case IDLE_STAT:
                if ((strlen(init_emmc) == (unsigned int)len) &&
                        (0 == strncmp((char *)data, init_emmc, len))) {
                    /* secure r5 receive init emmc request:
                    * 1.start a thread to init emmc
                    * 2.response "emmc_inited" to safety r5 in thread
                    */
                    commu_state = INIT_EMMC;
                    str_mmc_init("mmc0");
                }
                else {
                    sms_send_msg(str_chan, msg_unknow, strlen(msg_unknow));
                }
            break;

            case INIT_EMMC:
                /* if receive "write emmc" request. then save data in emmc,
                 * else save str info in buffer.
                 */
                if ((strlen(write_emmc) == (unsigned int)len) &&
                        (0 == strncmp((char *)data, write_emmc, len))) {
                    if (is_need_update((struct str_info_t *)str_rbuf,
                                       (struct str_info_t *)str_wbuf)) {
                        ssdk_printf(SSDK_ALERT, "need save the data\n");

                        for (i = 0; i < 8; i++)
                            p2->reserved[i] = p1->reserved[i];

                        disk_write(disk->disk_inst, ptn, (uint8_t *)str_wbuf, buf_sz);

                    }
                    else {
                        ssdk_printf(SSDK_INFO, "have save the data at last time, ignore\n");
                    }
                    disk_dev_deinit(disk);
                    if (str_rbuf) {
                        free(str_rbuf);
                        str_rbuf = NULL;
                    }
                    if (str_wbuf) {
                        free(str_wbuf);
                        str_wbuf = NULL;
                    }
                    sms_send_msg(str_chan, emmc_writed, strlen(emmc_writed));
                }
                else {
                    for (i = 0; i < len; i++) {
                        *(str_wbuf + wr_count) = *((char *)data + i);
                        wr_count++;
                    }

                    sms_send_msg(str_chan, save_comp, strlen(save_comp));
                }
            break;
        }
    }

    return 0;
}

void str_slave_commu_init(void)
{
    struct rpmsg_instance *sms_dev;

    sms_dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, DP_CR5_SAF);
    if (sms_dev) {
        str_chan = rpmsg_channel_create(sms_dev, "req-save-ddr-info", SEC_SAVE_DDR_INFO);
        if (str_chan) {
            commu_state = IDLE_STAT;
            rpmsg_channel_start(str_chan, str_event_handle, NULL);
            sms_send_msg(str_chan, init_emmc, strlen(init_emmc));
        }
        else {
            ssdk_printf(SSDK_CRIT, "failed create ipcc channel\n");
        }
    }
    else {
        ssdk_printf(SSDK_CRIT, "failed to get rpmsg device\n");
    }
}
