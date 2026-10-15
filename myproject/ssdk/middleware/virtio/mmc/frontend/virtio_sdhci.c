#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <debug.h>
#include <errno.h>
#include <compiler.h>
#include <param.h>
#include <arch/cache.h>
#include <board.h>
#include <malloc.h>
#include <driver/irq.h>

#define SDHCI_DESC_ADDR_64BITS (1)
#include <driver/kunlun/mmc/mmc_sdhci.h>
#include <driver/kunlun/mmc/sdhci.h>
#include <driver/memmap.h>

#include <dev/virtio.h>
#include <virtio_mmc.h>

#define SDHCI_TOUT_TIMER_CLK (1000000)
#define MAX_TUNING_LOOP 140
#define lower_32_bits(n) ((u32)(n))
#define upper_32_bits(n) ((u32)(((n) >> 16) >> 16))
#define IS_CACHE_LINE_ALIGNED(addr) !((addr_t)(addr) & (CONFIG_ARCH_CACHE_LINE - 1))

#define INT_STACK_SZ 1024

#define ADMA_BOUNDARY_SIZE (0x8000000)
#define CALC_BOUNDARY(addr, boundary) ((addr + boundary) & ~(boundary - 1))

/* vq length */
#define TX_LEN 64
#define RX_LEN 16

#define IS_ARCH_CACHE_LINE_ALIGNED(addr) !((addr_t)(addr) & (CONFIG_ARCH_CACHE_LINE - 1))

//#define DEBUG_SDHCI 1
#ifdef DEBUG_SDHCI
#define DBG(...) ssdk_printf(SSDK_CRIT, __VA_ARGS__)
#else
#define DBG(...)
#endif

static struct desc_entry g_dma_sg_list[ADMA_DESC_LIST_SIZE] __ALIGNED(CONFIG_ARCH_CACHE_LINE);

enum {
    VQ_TX = 0,
    VQ_RX,
    VQ_COUNT,
};

struct virtio_sdhci_cmd_req {
    uint64_t id;        /* Command identifier */
    uint16_t priority;   /* command priority field */
    uint16_t cdb_type;   /* mmc cmd descriptor type */
    struct virtio_mmc_sdhci_cmd_desc cdb;   /* mmc cmd descriptor data */
};

struct virtio_mmc_req {
    uint32_t id;
    void *req_data;
    uint32_t req_data_len;
    struct virtio_sdhci_cmd_req *cmd;
    struct virtio_mmc_cmd_resp *cmd_resp;
};

struct virtio_mmc {
    struct sdhci_host *host;
    u64 features;
    struct virtio_device *dev;
    int dev_priority;
    struct virtio_mmc_config config;
};

static struct virtio_mmc g_virtio_mmc = {0};
static struct virtio_mmc_req g_mmc_req = {0};
static uint32_t g_erase_start = 0;
static uint32_t g_erase_end = 0;

/*
 * Function: sdhci reset
 * Arg     : Host structure & mask to write to reset register
 * Return  : None
 * Flow:   : Reset the host controller
 */
void vsdhci_reset(struct sdhci_host *host, uint8_t mask)
{
    return;
}

static void _vsdhci_adma_write_desc(struct sdhci_host *host, void **desc,
                                   addr_t addr, int len, unsigned int attr)
{
    struct desc_entry *dma_desc = *desc;

    dma_desc->addr_l = lower_32_bits((addr_t)addr);
    dma_desc->addr_h = upper_32_bits((addr_t)addr);
    dma_desc->tran_att = attr | FV_ADMA2_ATTR_LEN(len);

    *desc += sizeof(struct desc_entry);
}

static void vsdhci_adma_write_desc(struct sdhci_host *host, void **desc,
                                  addr_t addr, int len, unsigned int attr)
{
    int offset;
    addr_t boundary_addr;

    boundary_addr = CALC_BOUNDARY(addr, ADMA_BOUNDARY_SIZE);

    if ((addr + len) > boundary_addr) {
        offset = boundary_addr - addr;
        _vsdhci_adma_write_desc(host, desc, addr, offset, attr);
        addr += offset;
        len -= offset;
    }

    _vsdhci_adma_write_desc(host, desc, addr, len, attr);
}

/*
 * Function: sdhci prep desc table
 * Arg     : Pointer data & length
 * Return  : Pointer to desc table
 * Flow:   : Prepare the adma table as per the sd spec v 4.0
 */
static struct desc_entry *vsdhci_prep_desc_table(struct sdhci_host *host,
        paddr_t data, uint32_t len)
{
    uint32_t sg_len = 0;
    uint32_t remain = 0;
    uint32_t i;
    uint32_t table_len = 0;
    uint32_t desc_len_max = 0;
    uint32_t tran_len = 0;
    uint32_t adma_attr = 0;
    struct desc_entry *desc = NULL;
    addr_t boundary_addr = CALC_BOUNDARY(data, ADMA_BOUNDARY_SIZE);

    if (host->caps.addr_64bit_v4) {
        desc_len_max = SDHCI_ADMA2_DESC_LINE_SZ_V4;
    }
    else {
        desc_len_max = SDHCI_ADMA_DESC_LINE_SZ;
    }

    /* Calculate the number of entries in desc table */
    sg_len = len / desc_len_max;
    remain = len - (sg_len * desc_len_max);

    /* Allocate sg_len + 1 entries if there are remaining bytes at the end
        */
    if (remain) {
        sg_len++;
    }

    /* For workaround the snps adma boundary, alloc one more desc entry */
    /* across adma boundary, alloc one more desc entry */
    if ((data + len) > boundary_addr)
        table_len = ((sg_len + 2) * sizeof(struct desc_entry));
    else
        table_len = ((sg_len + 1) * sizeof(struct desc_entry));

    memset(g_dma_sg_list, 0, ROUNDUP(table_len, CONFIG_ARCH_CACHE_LINE));
    desc = g_dma_sg_list;

    for (i = 0; i < sg_len; i++) {
        tran_len = MIN(len, desc_len_max);
        adma_attr = SDHCI_ADMA_DATA_VALID;
        vsdhci_adma_write_desc(host, (void **)&desc, data, tran_len, adma_attr);
        data += tran_len;
        len -= tran_len;
    }

    /* Add the last entry - nop, end, valid */
    adma_attr = SDHCI_ADMA_NOP_END_VALID;
    vsdhci_adma_write_desc(host, (void **)&desc, 0, 0, adma_attr);

    arch_clean_invalidate_cache_range((addr_t)g_dma_sg_list,
                                      ROUNDUP(table_len, CONFIG_ARCH_CACHE_LINE));

    for (i = 0; i < sg_len + 1; i++) {
        DBG("\n %s: g_dma_sg_list: addr_l: 0x%x addr_h: 0x%x attr: 0x%x\n", __func__,
            g_dma_sg_list[i].addr_l, g_dma_sg_list[i].addr_h, g_dma_sg_list[i].tran_att);
    }

    return g_dma_sg_list;
}

/*
 * Function: sdhci adma transfer
 * Arg     : Host structure & command stucture
 * Return  : Pointer to desc table
 * Flow    : 1. Prepare descriptor table
 *           2. Write adma register
 *           3. Write block size & block count register
 */
static struct desc_entry *vsdhci_adma_transfer(struct sdhci_host *host,
        struct mmc_command *cmd)
{
    uint32_t num_blks = 0;
    uint32_t sz;
    paddr_t data;
    struct desc_entry *adma_desc;
    num_blks = cmd->data.num_blocks;
    data = p2ap((paddr_t)cmd->data.data_ptr);

    /*
     * Some commands send data on DAT lines which is less
     * than SDHCI_MMC_BLK_SZ, in that case trying to read
     * more than the data sent by the card results in data
     * CRC errors. To avoid such errors allow data to pass
     * the required block size, if the block size is not
     * passed use the default value
     */
    if (cmd->data.blk_sz) {
        sz = num_blks * cmd->data.blk_sz;
    }
    else {
        sz = num_blks * SDHCI_MMC_BLK_SZ;
    }

    /* Prepare adma descriptor table */
    if (!host->virtio_mmc) {
        adma_desc = vsdhci_prep_desc_table(host, data, sz);
    } else {
        adma_desc = cmd->data.sg_list;
    }

    return adma_desc;
}

static inline bool vsdhci_data_line_cmd(struct mmc_command *cmd)
{
    return cmd->data_present || cmd->resp_type & SDHCI_CMD_RESP_R1B;
}

static inline uint16_t vsdhci_cale_timeout(uint64_t us, uint32_t freq)
{
    uint16_t i = 0;
    uint64_t t;

    for (i = 0; i < 0xF; i++) {
        t = 1 << (13 + i);

        if (t / freq * 1000 * 1000 > us) {
            break;
        }
    }

    return MIN(i, 0xE);
}

/*
 * Function: actual sdhci send command
 * Arg     : Host structure & command stucture
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Prepare the command register
 *           2. If data is present, prepare adma table
 *           3. Run the command
 *           4. Check for command results & take action
 */
static uint32_t _vsdhci_send_command(struct sdhci_host *host,
                                    struct mmc_command *cmd)
{
    uint32_t ret = 0;
    uint32_t resp_type = 0;
    uint16_t trans_mode = 0;
    uint32_t flags;
    struct virtio_mmc *virtio_mmc = &g_virtio_mmc;
    struct virtio_device *dev = virtio_mmc->dev;
    uint16_t i;
    struct vring_desc *desc;

    DBG("\n %s: START: cmd:%04d, arg:0x%08x, resp_type:0x%04x, data_present:%d\n",
           __func__, cmd->cmd_index, cmd->argument, cmd->resp_type,
           cmd->data_present);

    if (cmd->cmd_index == CMD35_ERASE_GROUP_START) {
        g_erase_start = cmd->argument;

        return 0;
    } else if (cmd->cmd_index == CMD36_ERASE_GROUP_END) {
        g_erase_end = cmd->argument;

        return 0;
    }

    cmd->error = 0;

    if (cmd->data_present) {
        if (!host->virtio_mmc) {
            ASSERT(cmd->data.data_ptr);
            ASSERT(IS_CACHE_LINE_ALIGNED(cmd->data.data_ptr));
        } else {
            ASSERT(cmd->data.sg_list);
        }
    }

    host->cmd = cmd;

    switch (cmd->resp_type) {
        case SDHCI_CMD_RESP_R1:
        case SDHCI_CMD_RESP_R3:
        case SDHCI_CMD_RESP_R6:
        case SDHCI_CMD_RESP_R7:
            /* Response of length 48 have 32 bits
             * of response data stored in RESP0[0:31]
             */
            resp_type = SDHCI_CMD_RESP_48;
            break;

        case SDHCI_CMD_RESP_R2:
            /* Response of length 136 have 120 bits
             * of response data stored in RESP0[0:119]
             */
            resp_type = SDHCI_CMD_RESP_136;
            break;

        case SDHCI_CMD_RESP_R1B:
            /* Response of length 48 have 32 bits
             * of response data stored in RESP0[0:31]
             * & set CARD_BUSY status if card is busy
             */
            resp_type = SDHCI_CMD_RESP_48_BUSY;
            break;

        case SDHCI_CMD_RESP_NONE:
            resp_type = SDHCI_CMD_RESP_NONE;
            break;

        default:
            ssdk_printf(SSDK_CRIT, "Invalid response type for the command\n");
            return 1;
    };

    flags = (resp_type << SDHCI_CMD_RESP_TYPE_SEL_BIT);

    if (cmd->data_present || cmd->cmd_index == CMD21_SEND_TUNING_BLOCK)
        flags |= (1 << SDHCI_CMD_DATA_PRESENT_BIT);

    flags |= (cmd->cmd_type << SDHCI_CMD_CMD_TYPE_BIT);

    /* Enable Command CRC & Index check for commands with response
     * R1, R6, R7 & R1B. Also only CRC check for R2 response
     */
    switch (cmd->resp_type) {
        case SDHCI_CMD_RESP_R1:
        case SDHCI_CMD_RESP_R6:
        case SDHCI_CMD_RESP_R7:
        case SDHCI_CMD_RESP_R1B:
            flags |=
                (1 << SDHCI_CMD_CRC_CHECK_BIT) | (1 << SDHCI_CMD_IDX_CHECK_BIT);
            break;

        case SDHCI_CMD_RESP_R2:
            flags |= (1 << SDHCI_CMD_CRC_CHECK_BIT);
            break;

        default:
            break;
    };

    /* Check if data needs to be processed */
    if (cmd->data_present) {
        /*
         * clean any stale cache lines ensure cpu data buffer flush in memory,
         * and forbid cpu write back cache on unpredictable time.
         */
        arch_clean_invalidate_cache_range(
            (addr_t)cmd->data.data_ptr,
            (cmd->data.blk_sz) ? (cmd->data.num_blocks * cmd->data.blk_sz)
                            : (cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
        cmd->data.sg_list = vsdhci_adma_transfer(host, cmd);
        memcpy(&host->data_cmd, cmd, sizeof(struct mmc_command));
    }

    if (cmd->trans_mode == SDHCI_MMC_READ)
        trans_mode |= SDHCI_READ_MODE;

    /* Set the Transfer mode */
    if (cmd->data_present) {
        /* Enable DMA */
        trans_mode |= SDHCI_DMA_EN;

        /*
         * Enable auto cmd23 or cmd12 for multi block transfer
         * based on what command card supports
         */
        if (cmd->data.num_blocks > 1) {
            if (cmd->cmd23_support) {
                trans_mode |=
                    SDHCI_TRANS_MULTI | SDHCI_AUTO_CMD23_EN | SDHCI_BLK_CNT_EN;
            }
            else
                trans_mode |=
                    SDHCI_TRANS_MULTI | SDHCI_AUTO_CMD12_EN | SDHCI_BLK_CNT_EN;
        }
    }

    /* clear the share buf before reuse */
    memset(g_mmc_req.cmd, 0x00,
           ROUNDUP(sizeof(struct virtio_sdhci_cmd_req), CONFIG_ARCH_CACHE_LINE));
    memset(g_mmc_req.cmd_resp, 0x00,
           ROUNDUP(sizeof(struct virtio_mmc_cmd_resp), CONFIG_ARCH_CACHE_LINE));

    g_mmc_req.cmd->cdb_type = VIRTIO_MMC_CDB_SDHCI;
    g_mmc_req.cmd->cdb.cmd_index = cmd->cmd_index;
    g_mmc_req.cmd->cdb.argument = cmd->argument;
    g_mmc_req.cmd->cdb.cmd23_support = cmd->cmd23_support;
    g_mmc_req.cmd->cdb.cmd_retry = cmd->cmd_retry;
    g_mmc_req.cmd->cdb.cmd_timeout = cmd->cmd_timeout;
    g_mmc_req.cmd->cdb.part_type = PART_ACCESS_DEFAULT;
    g_mmc_req.cmd->cdb.resp_type = cmd->resp_type;
    g_mmc_req.cmd->cdb.buf_mode = VIRTIO_MMC_SDHCI_BUF_NORMAL;
    g_mmc_req.req_data = 0;
    g_mmc_req.req_data_len = 0;

    if (cmd->data_present) {
        g_mmc_req.cmd->cdb.sector = cmd->argument & 0xffff;
        g_mmc_req.cmd->cdb.num_sectors = cmd->data.num_blocks;

        if (cmd->cmd_index == CMD8_SEND_EXT_CSD) {
            g_mmc_req.req_data = cmd->data.data_ptr;
            g_mmc_req.req_data_len = 512u;
            arch_clean_invalidate_cache_range((vaddr_t)g_mmc_req.req_data,
                                              g_mmc_req.req_data_len);
        } else {
            g_mmc_req.cmd->cdb.buf_mode =
                VIRTIO_MMC_SDHCI_BUF_AMDA2_64BITS_DESC;
            g_mmc_req.req_data = cmd->data.sg_list;
            g_mmc_req.req_data_len = sizeof(void *);
        }
    } else {
        if (cmd->cmd_index == CMD38_ERASE) {
            g_mmc_req.cmd->cdb.sector = g_erase_start;
            g_mmc_req.cmd->cdb.num_sectors = g_erase_end - g_erase_start + 1;
        }
    }

    /* prepare desc */
    desc = virtio_alloc_desc_chain(dev, VQ_TX, 3, &i);
    if (desc == NULL) {
        printf("not enough desc\n");
        return 0;
    }
    desc->addr = p2ap((paddr_t)g_mmc_req.cmd);
    desc->len = sizeof(struct virtio_sdhci_cmd_req);
    desc->flags |= VRING_DESC_F_NEXT;
    arch_clean_invalidate_cache_range((vaddr_t)g_mmc_req.cmd, desc->len);

    /* set up the descriptor pointing to the buffer */
    desc = virtio_desc_index_to_desc(dev, VQ_TX, desc->next);
    desc->addr = p2ap((paddr_t)g_mmc_req.req_data);
    desc->len = g_mmc_req.req_data_len;
    desc->flags |= VRING_DESC_F_NEXT;

    desc = virtio_desc_index_to_desc(dev, VQ_TX, desc->next);
    desc->addr = p2ap((paddr_t)g_mmc_req.cmd_resp);
    desc->len = sizeof(struct virtio_mmc_cmd_resp);
    desc->flags = VRING_DESC_F_WRITE;
    arch_clean_invalidate_cache_range((vaddr_t)g_mmc_req.cmd_resp, desc->len);

    virtio_submit_chain(dev, VQ_TX, i);
    virtio_kick(dev, VQ_TX);

    if (!host->tuning_in_progress) {
        osSemaphoreAcquire(host->cmd_sema, osWaitForever);
    }

    ret |= host->cmd->error;

    if (ret && cmd->resp_type == SDHCI_CMD_RESP_R1B) {
        goto cmd_out;
    }

    /*
     * If has data and sync mode, or cmd has busy status bit,
     * need wait for data complete signal.
     */
    if ((host->data_cmd.data_present && !host->async_mode) ||
            cmd->resp_type == SDHCI_CMD_RESP_R1B) {
        if (!host->tuning_in_progress) {
            osSemaphoreAcquire(host->data_complete_sema, osWaitForever);
        }

        if (host->data_cmd.data_present) {
            ret |= host->data_cmd.error;
            host->data_cmd.data_present = 0;
        }
        else {
            ret |= host->cmd->error;
        }
    }

cmd_out:
    host->cmd = NULL;
    ret = g_mmc_req.cmd_resp->status;
    if(ret) {
        vsdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
    }

    memcpy(cmd->resp, g_mmc_req.cmd_resp, 16u);

    DBG("\n %s: END: host:0x%x cmd:%04d, arg:0x%08x, resp:0x%08x 0x%08x "
        "0x%08x 0x%08x\n",
        __func__, host->base, cmd->cmd_index, cmd->argument, cmd->resp[0],
        cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    return ret;
}

/*
 * Function: sdhci send command
 * Arg     : Host structure & command stucture
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Prepare the command register
 *           2. If data is present, prepare adma table
 *           3. Run the command
 *           4. Check for command results & take action
 */
uint32_t vsdhci_send_command(struct sdhci_host *host, struct mmc_command *cmd)
{
    uint32_t ret = 0;
    uint32_t cmd_retries = 3;

    do {
        ret = _vsdhci_send_command(host, cmd);

        if (ret && cmd_retries) {
            ssdk_printf(SSDK_CRIT,"\n %s: cmd retry: cmd:%04d, arg:0x%08x, resp_type:0x%04x, data_present:%d retries:%d\n",
                   __func__, cmd->cmd_index, cmd->argument, cmd->resp_type,
                   cmd->data_present, cmd_retries);
            cmd_retries--;
            cmd->error = 0;
        }
        else
            break;

    }
    while (cmd_retries);

    return ret;
}

static int vsdhci_irq_handle(struct virtio_device *dev,
        uint ring, const struct vring_used_elem *e)
{
    struct virtio_mmc *vmmc = dev->priv;
    struct sdhci_host *host = vmmc->host;
    struct mmc_command *cmd = host->cmd;

    if (ring == VQ_TX) {
        u16 i = e->id;
        while (i < TX_LEN) {
            struct vring_desc *desc = virtio_desc_index_to_desc(dev, ring, i);
            u16 n = (desc->flags & VRING_DESC_F_NEXT) ? desc->next : TX_LEN;

            virtio_free_desc(dev, ring, i);
            i = n;
        }
        if (!osSemaphoreGetCount(host->cmd_sema))
                osSemaphoreRelease(host->cmd_sema);

        if (cmd->data_present || cmd->resp_type == SDHCI_CMD_RESP_R1B) {
            if (!osSemaphoreGetCount(host->data_sema))
                osSemaphoreRelease(host->data_sema);
        }
    } else {
        // TODO handle RX vq
    }

    return 0;
}

static void vsdhci_data_thread(void *arg)
{
    struct sdhci_host *host = arg;
    struct mmc_command *data_cmd;
    struct mmc_device *dev = NULL;

    for (;;) {
        osSemaphoreAcquire(host->data_sema, osWaitForever);
        DBG("\n %s:sdhci data thread in! host %p, host parent %p\n", __func__, host, host->parent);

        if (host->parent) {
            dev = host->parent;
        }

        data_cmd = &host->data_cmd;

        if (!host->data_cmd.data_present) {
            /* If no data cmd, may be RESP_R1B type cmd. */
            if (host->cmd->error) {
                ssdk_printf(SSDK_CRIT, "sdhci busy error! 0x%08x\n", host->cmd->error);
            }

            if (!osSemaphoreGetCount(host->data_complete_sema))
                osSemaphoreRelease(host->data_complete_sema);
            continue;
        }
        else {
            if (data_cmd->error) {
                ssdk_printf(SSDK_CRIT, "sdhci data error! 0x%08x\n", data_cmd->error);
            }
        }

        if (data_cmd->trans_mode == SDHCI_MMC_READ) {
            /*
             * Read can be performed on block size < SDHCI_MMC_BLK_SZ,
             * make sure to flush the data only for the read size instead.
             */
            arch_invalidate_cache_range(
                (addr_t)data_cmd->data.data_ptr,
                (data_cmd->data.blk_sz)
                ? (data_cmd->data.num_blocks * data_cmd->data.blk_sz)
                : (data_cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
        }

        /* Free the scatter/gather list */
        if (data_cmd->data.sg_list) {
            data_cmd->data.sg_list = NULL;
        }

        if (dev && dev->event_handle) {
            data_cmd->data_present = 0;
            dev->opt_result = data_cmd->error;
            dev->event_handle(dev->opt_type, dev->opt_result);
        }
        else {
            DBG("\n %s:signal complete event!\n", __func__);
            if (!osSemaphoreGetCount(host->data_complete_sema))
                osSemaphoreRelease(host->data_complete_sema);
        }
    }

    return;
}

/*
 * Function: sdhci init
 * Arg     : Host structure
 * Return  : None
 * Flow:   : 1. Reset the controller
 *           2. Read the capabilities register & populate the host
 *           controller capabilities for use by other functions
 *           3. Enable the power control
 *           4. Set initial bus width
 *           5. Set Adma mode
 *           6. Enable the error status
 */
void vsdhci_init(struct sdhci_host *host)
{
    uint32_t caps[2];
    struct virtio_device *dev = NULL;

    if (g_mmc_req.cmd == NULL) {
        g_mmc_req.cmd =
            memalign(CONFIG_ARCH_CACHE_LINE,
                     ROUNDUP(sizeof(struct virtio_sdhci_cmd_req), CONFIG_ARCH_CACHE_LINE));
    }

    if (g_mmc_req.cmd == NULL) {
        printf("mmc front failed to allocate cmd addr!!! \n");
        goto err_init;
    }

    if (g_mmc_req.cmd_resp == NULL) {
        g_mmc_req.cmd_resp =
            memalign(CONFIG_ARCH_CACHE_LINE,
                     ROUNDUP(sizeof(struct virtio_mmc_cmd_resp), CONFIG_ARCH_CACHE_LINE));
    }

    if (g_mmc_req.cmd_resp == NULL) {
        printf("mmc front failed to allocate cmd_resp addr!!! \n");
        goto err_init;
    }


    vsdhci_reset(host, SDHCI_SOFT_RESET);

    host->caps.spec_version = 0x5;
    /* Read the capabilities register & store the info */
    caps[0] = 0x3d6dc881u;
    caps[1] = 0x08008077u;
    DBG("\n %s: Host version: %d capability: cap1:0x%08x, cap2: 0x%08x\n",
        __func__, host->caps.spec_version, caps[0], caps[1]);

    host->caps.base_clk_rate =
        (caps[0] & SDHCI_CLK_RATE_MASK) >> SDHCI_CLK_RATE_BIT;
    host->caps.base_clk_rate *= 1000000;
    /* Get the max block length for mmc */
    host->caps.max_blk_len =
        (caps[0] & SDHCI_BLK_LEN_MASK) >> SDHCI_BLK_LEN_BIT;

    /* 8 bit Bus width */
    if (caps[0] & SDHCI_8BIT_WIDTH_MASK) {
        host->caps.bus_width_8bit = 1;
    }

    /* Adma2 support */
    if (caps[0] & SDHCI_BLK_ADMA_MASK) {
        host->caps.adma2_support = 1;
    }

    /* V4 64bit address support */
    if (caps[0] & SDHCI_CAP_ADDR_64BIT_V4) {
        host->caps.addr_64bit_v4 = 1;
    }

    /* V3 64bit address support */
    if (caps[0] & SDHCI_CAP_ADDR_64BIT_V3) {
        host->caps.addr_64bit_v3 = 1;
    }

    /* Supported voltage, do nothing when it has been set */
    if (host->caps.voltage) {
        ;
    }
    else if (caps[0] & SDHCI_3_3_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_3;
    }
    else if (caps[0] & SDHCI_3_0_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_0;
    }
    else if (caps[0] & SDHCI_1_8_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_1_8;
    }

    /* DDR mode support */
    host->caps.ddr_support = (caps[1] & SDHCI_DDR50_MODE_MASK) ? 1 : 0;
    /* SDR50 mode support */
    host->caps.sdr50_support = (caps[1] & SDHCI_SDR50_MODE_MASK) ? 1 : 0;
    /* SDR104 mode support */
    host->caps.sdr104_support = (caps[1] & SDHCI_SDR104_MODE_MASK) ? 1 : 0;

    host->cmd_sema = osSemaphoreNew(1, 0, NULL);
    host->data_sema = osSemaphoreNew(1, 0, NULL);
    host->data_complete_sema = osSemaphoreNew(1, 0, NULL);

    osThreadAttr_t sdhci_data_attr = {
        .stack_size = 4096,
        .priority = osPriorityNormal,
    };

    char sdhci_name[128] = {0};
    sprintf(sdhci_name, "vsdhci_data%d", 1);
    sdhci_data_attr.name = sdhci_name;
    osThreadNew(vsdhci_data_thread, host, &sdhci_data_attr);

#ifdef SUPPORT_VIRTIO_MMC_BACKEND_IN_SAF
    virtio_mmio_find_timeout(0, 99, &dev, 2000u);
#else
    virtio_mmio_find_timeout(1, 99, &dev, 2000u);
#endif
    if (!dev) {
        printf("fail to find mmc virtio sample\n");
        goto err_init;
    }

    /* reset device to start */
    virtio_reset_device(dev);

    /* check features here */
    uint64_t features = virtio_mmio_read_device_features(dev);

    /* settle down features here */
    virtio_mmio_write_driver_features(dev, 0);

    /* read configs */
    volatile struct virtio_mmc_config *cfg = dev->config_ptr;
    hexdump((void *)cfg, sizeof(struct virtio_mmc_config));

    /* we've seen the device */
    virtio_status_acknowledge_driver(dev);

    /* alloc rings */
    virtio_alloc_ring(dev, VQ_TX, TX_LEN);

    dev->irq_driver_callback = vsdhci_irq_handle;

    virtio_status_driver_ok(dev);

    g_virtio_mmc.features = features;
    g_virtio_mmc.dev = dev;
    g_virtio_mmc.host = host;
    dev->priv = &g_virtio_mmc;

    printf("virtio mmc frontend ready!!! \n");
    return;

err_init:
    if (g_mmc_req.cmd != NULL) {
        free(g_mmc_req.cmd);
        g_mmc_req.cmd = NULL;
    }

    if (g_mmc_req.cmd_resp != NULL) {
        free(g_mmc_req.cmd_resp);
        g_mmc_req.cmd_resp = NULL;
    }
}

/*
 * Function: sdhci reinit
 * Arg     : Host structure
 * Return  : None
 * Flow:   : 1. Reset the controller
 *           2. Read the capabilities register & populate the host
 *           controller capabilities for use by other functions
 *           3. Enable the power control
 *           4. Set initial bus width
 *           5. Set Adma mode
 *           6. Enable the error status
 */
void vsdhci_reinit(struct sdhci_host *host, uint8_t voltage)
{
    uint32_t caps[2];

    vsdhci_reset(host, SDHCI_SOFT_RESET);

    host->caps.spec_version = 0x5;
    /* Read the capabilities register & store the info */
    caps[0] = 0x3d6dc881u;
    caps[1] = 0x08008077u;
    DBG("\n %s: Host version: %d capability: cap1:0x%08x, cap2: 0x%08x\n",
        __func__, host->caps.spec_version, caps[0], caps[1]);

    if (voltage) {
        host->caps.voltage = voltage;
    }
    else if (caps[0] & SDHCI_3_3_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_3;
    }
    else if (caps[0] & SDHCI_3_0_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_0;
    }
    else if (caps[0] & SDHCI_1_8_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_1_8;
    }

    vsdhci_reset(host, SDHCI_SOFT_RESET);
}
