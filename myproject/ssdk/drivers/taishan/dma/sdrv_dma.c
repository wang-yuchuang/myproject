/*
 * @FilePath: drivers\dma\sdrv_dma.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * @Author: mengmeng.chang
 * @Date: 2021-09-26 19:57:04
 * @Descripttion:
 * Semidrive DMA high level driver
 * @Revision History:
 * ----------------
 */

#include <cmsis_os2.h>
#include <debug.h>
#include <param.h>
#include <string.h>

#include "driver/taishan/dma/sdrv_dma.h"

#include "arch/cache.h"
#include "bits.h"
#include "board.h"
#include "driver/irq.h"
#include "sdrv_dmac.h"

#define SDRV_DMA_LOG 0
#define SDRV_DMA_LLI_BUF_MAX_LEN (ROUNDUP(128, CONFIG_ARCH_CACHE_LINE))
#define SDRV_DMA_LLI_BUF_LBC_MAX_LEN (ROUNDUP(64, CONFIG_ARCH_CACHE_LINE))

/**
 * @brief abstract link list
 */
typedef struct lli_list {
    uint32_t phy_addr;
    struct list_node lli_node; /* lli_node */
    struct list_node lli_head; /* lli_list */
    sdrv_dmac_lli_node_t lli_item;
    int bitmap_index;
} lli_list_t;

static unsigned long
    sdrv_dma_lli_bitmap[BITMAP_NUM_WORDS(SDRV_DMA_LLI_BUF_MAX_LEN)]
    __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};
#ifdef CONFIG_SDRV_DMA_USE_LBC
static unsigned long
    sdrv_dma_local_buf_bitmap[BITMAP_NUM_WORDS(SDRV_DMA_LLI_BUF_LBC_MAX_LEN)]
    __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};
#endif
static lli_list_t sdrv_dma_lli_buf[SDRV_DMA_LLI_BUF_MAX_LEN]
    __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};

typedef struct lli_desc {
    uint32_t phy_addr;
    lli_list_t * mem_base;
    unsigned long * mem_bitmap;
    uint32_t mem_size;
}lli_desc_t;

/**
 * @brief abstract dmac config.
 */
typedef struct dmac_coeff {
    paddr_t base;              /* dmac base address */
    sdrv_dma_cfg_t *hw_cfg;    /* dmac hardware config */
    sdrv_dmac_ch_cfg_t ch_cfg; /* dmac channel config */
} dmac_coeff_t;

/**
 * @brief abstract sdrv_dma channel
 */
typedef struct sdrv_dma_chan {
    dma_chan_t chan;
    dma_dev_cfg_t dev_cfg;
    dma_tr_direction_e direction;
    dma_transaction_type_e tr_type;
    uint8_t memset_val;
    uint32_t blk_size;
    uint32_t len;
    uint32_t ps_cnt;
    dmac_coeff_t dmac_cfg;
    lli_desc_t lli_desc;
} sdrv_dma_chan_t;

/**
 * @brief transfer dma_chan_t to sdrv_dma_chan_t.
 *
 * @param[in] chan  abstracted dma channel info
 * @return sdrv_dma_chan_t data point
 */
static inline sdrv_dma_chan_t *to_sdrv_dma_chan(dma_chan_t *chan)
{
    return containerof(chan, sdrv_dma_chan_t, chan);
}

static osMutexId_t lli_list_lock = NULL;

/**
 * @brief dma irq callback
 *
 * @param[in] irq irq id
 * @param[in] arg private data point (see dma_dev_t)
 */
static int sdrv_dma_irq_handler(uint32_t irq, void *arg)
{
    dma_dev_t *dev = (dma_dev_t *)arg;
    uint32_t chan_id = 0;
    uint32_t chan_int_sta = 0;
    dma_chan_t *cptr = NULL;
    list_for_every_entry (&dev->channels, cptr, dma_chan_t, node) {
        if (irq == cptr->irq_id) {
            chan_id = cptr->chan_id;
            chan_int_sta = sdrv_dmac_get_int_state(dev->dma_base, chan_id);
            // LOGI("Status: %#x \n", chan_int_sta);
            sdrv_dmac_clr_int(dev->dma_base, chan_id, chan_int_sta);
            if (cptr->irq_cb) {
                cptr->irq_cb(sdrv_dmac_irq_status_handle(chan_int_sta),
                             chan_int_sta, cptr->context);
            }
        }
    }

    return 0;
}

/**
 * @brief sdrv_dma chan request
 *
 * @param[in] dev dma control instance
 * @param[in] id  random if null
 * @return chan info
 */
static dma_chan_t *sdrv_dma_chan_req(dma_dev_t *dev, uint32_t *id)
{
    dma_chan_t *cptr = NULL;
    sdrv_dma_chan_t *dmac = NULL;
    int32_t req_ret = 0;

    LOGI("id = %d\n", id ? *id : -1);
    list_for_every_entry (&dev->channels, cptr, dma_chan_t, node) {
        LOGV("get cptr: %p, cptr->allocated: %d, chan_id: %d\n", cptr,
             cptr->allocated, cptr->chan_id);
        if (0 == cptr->allocated) {
            cptr->irq_cb = NULL;
            cptr->context = NULL;
            cptr->chan_status = DMA_COMPLETED;
            if ((NULL == id) || (*id == cptr->chan_id)) {
                cptr->allocated = 1;
                req_ret = 1;
                break;
            }
        }
    }
    LOGV("get cptr: %p, cptr->allocated: %d, chan_id: %d\n", cptr,
         cptr->allocated, cptr->chan_id);

    if (req_ret && cptr) {
        dmac = to_sdrv_dma_chan(cptr);
        dmac->dmac_cfg.ch_cfg.ps_cnt =
            sdrv_dmac_get_ch_pscnt(dev->dma_base, cptr->chan_id);

        LOGI("request ch_cfg: %p, dma_base: %#x, chann: %d, get ps_cnt: %d\n",
             &(dmac->dmac_cfg.ch_cfg), dev->dma_base, cptr->chan_id,
             dmac->dmac_cfg.ch_cfg.ps_cnt);
        return cptr;
    } else {
        return NULL;
    }
}

/**
 * @brief set dma device config (only for mem2dev/dev2mem)
 *
 * @param[in] chan requested chan info
 * @param[in] cfg device config
 */
static void sdrv_dma_dev_config(dma_chan_t *chan, dma_dev_cfg_t *cfg)
{
    ASSERT((NULL != chan) && (NULL != cfg));
    osMutexAcquire(chan->chan_lock, osWaitForever);
    LOGI("copy dev cfg, cfg dst addr: %#x\n", cfg->dst_addr);
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);

    memcpy(&dmac->dev_cfg, cfg, sizeof(*cfg));

    osMutexRelease(chan->chan_lock);
}

/**
 * @brief check chan valid
 *
 * @param[in] chan requested chan info
 * @param[in] addr address(src/dst)
 */
static int sdrv_dma_validate_address(dma_chan_t *chan, paddr_t src_addr,
                                     paddr_t dst_addr)
{
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);
    uint8_t valid_flag = 0;

    for (int i = 0; i < dmac->dmac_cfg.hw_cfg->addr_table_len; i++) {
        if (src_addr >= dmac->dmac_cfg.hw_cfg->addr_table[i].base_addr &&
            src_addr < dmac->dmac_cfg.hw_cfg->addr_table[i].base_addr +
                           dmac->dmac_cfg.hw_cfg->addr_table[i].len)
            valid_flag |= 0x01;

        if (dst_addr >= dmac->dmac_cfg.hw_cfg->addr_table[i].base_addr &&
            dst_addr < dmac->dmac_cfg.hw_cfg->addr_table[i].base_addr +
                           dmac->dmac_cfg.hw_cfg->addr_table[i].len)
            valid_flag |= 0x02;
    }

    if (valid_flag & 0x03)
        return 0;
    else
        return -1;
}

/**
 * @brief data transfer length split
 *
 * @param[in] blk_size single maximum length
 * @param[in] bytes transfer size
 * @param[in] width bus width
 * @param[in] len calculated transfer length
 * @return transfer size / bus width
 */
static uint32_t bytes2block(uint32_t blk_size, size_t bytes, unsigned int width,
                            size_t *len)
{
    uint32_t block;

    if ((bytes >> width) > blk_size) {
        block = blk_size;
        *len = block << width;
    } else {
        block = bytes >> width;
        *len = bytes;
    }

    return block;
}

/**
 * @brief free link list
 *
 * @param[in] list lli_list_t data point
 */
static void sdrv_dma_free_lli(lli_list_t *list)
{
    lli_list_t *curr = NULL;
    lli_list_t *first = list;
    lli_list_t *next = NULL;

    IRQ_SAVE

    list_for_every_entry_safe (&first->lli_head, curr, next, lli_list_t,
                               lli_node) {
        list_delete(&curr->lli_node);

        bitmap_clear(sdrv_dma_lli_bitmap, curr->bitmap_index);
    }

    IRQ_RESTORE
}

/**
 * @brief: reclaim memory for continued use by subsequent applications
 *
 * @param {lli_desc_t *} lli_desc
 * @return {*}
 */
static void sdrv_dma_free_lli_desc(lli_desc_t *lli_desc)
{
    lli_list_t *curr = NULL;
    lli_list_t *first = (lli_list_t *)lli_desc->phy_addr;
    lli_list_t *next = NULL;
    LOGI("first : %p lli_desc: %p\n", first, lli_desc);
    if (first == NULL)
        return;
    IRQ_SAVE

    list_for_every_entry_safe (&first->lli_head, curr, next, lli_list_t,
                               lli_node) {
        list_delete(&curr->lli_node);

        bitmap_clear(lli_desc->mem_bitmap, curr->bitmap_index);
    }
    lli_desc->phy_addr = 0;
    IRQ_RESTORE
}

/**
 * @brief: alloc linklist node memory, memory base relate by lli_desc
 *
 * @param {lli_desc_t *} lli_desc
 * @return {lli_list_t*} linklist node memory base
 */
static lli_list_t *sdrv_dma_get_lli_desc(lli_desc_t *lli_desc)
{
    lli_list_t *lli_ptr = NULL;
    int index = 0;

    osMutexAcquire(lli_list_lock, osWaitForever);
    index = bitmap_ffz(lli_desc->mem_bitmap, lli_desc->mem_size);

    if (-1 != index) {
        lli_ptr = &lli_desc->mem_base[index];

        list_initialize(&lli_ptr->lli_head);

        lli_ptr->phy_addr = (uint32_t)(addr_t)(&lli_ptr->lli_item);
        // LOGI("lli_ptr->phy_addr: %#x \n", lli_ptr->phy_addr);
        lli_ptr->bitmap_index = index;

        bitmap_set(lli_desc->mem_bitmap, index);
    }

    osMutexRelease(lli_list_lock);

    return lli_ptr;
}

static int32_t sdrv_dma_get_muxid(sdrv_dma_mux_param_t *tbl, uint32_t tbl_len,
                                  DMA_MUX_DIR direct, paddr_t periph_addr)
{
    int32_t mux_id = -1;

    for (int i = 0; i < tbl_len; i++) {
        if (periph_addr == tbl[i].start_addr + tbl[i].len) {
            if ((direct & tbl[i].direct) > 0) {
                mux_id = tbl[i].port;
                break;
            }
        }
    }
    return mux_id;
}

/**
 * @brief: get the largest multiple of 2
 *         such as input 65 return 64.
 *
 * @param {uint32_t} value
 * @return {*}
 */
static uint32_t get_align_sz(uint32_t value)
{
    uint32_t val = 0;
    val = 31 - __builtin_clz(value);
    val = 1 << val;
    return val;
}

/**
 * @brief set link list
 *
 * @param[in] dmac_cfg abstracted dmac config info
 * @param[in] len      data size
 * @param[in] blk_size single maximum length
 * @param[in] param for differnt transaction type,represent different parameters
 * @param[in] type transaction type
 * @param[in] direction transfer direction
 */
static lli_list_t *sdrv_dma_set_lli(dmac_coeff_t *dmac_cfg, uint32_t len,
                                    uint32_t blk_size, void *param,
                                    dma_transaction_type_e type,
                                    dma_tr_direction_e direction,
                                    lli_desc_t *lli_desc)
{
    lli_list_t *first = NULL;
    lli_list_t *curr = NULL;
    lli_list_t *prev = NULL;
    uint32_t offset = 0;
    uint32_t tr_len = 0;
    uint32_t burst_len = 0;
    uint32_t period_len = ((DMA_CYCLIC == type) ? *((uint32_t *)param) : 0);
    sdrv_dmac_ll_cfg_t ll_config;

    memcpy(&ll_config, &dmac_cfg->ch_cfg.ll_config, sizeof(sdrv_dmac_ll_cfg_t));
    LOGI("len: %d, blk_size: %d, period_len: %d\n", len, blk_size, period_len);

    while (offset < len) {
        curr = sdrv_dma_get_lli_desc(lli_desc);
        if (!curr)
            goto err_lli;

        memset(&curr->lli_item, 0, sizeof(sdrv_dmac_lli_node_t));

        switch (type) {
        case DMA_CYCLIC:
        case DMA_DEVICE:
            if (DMA_MEM2DEV == direction) {
                curr->lli_item.src_addr =
                    (uint32_t)(dmac_cfg->ch_cfg.ll_config.src + offset);
                curr->lli_item.dst_addr =
                    (uint32_t)(dmac_cfg->ch_cfg.ll_config.dst);
                burst_len = dmac_cfg->ch_cfg.ll_config.dst_bl;
            } else { // dev -> mem
                curr->lli_item.src_addr =
                    (uint32_t)(dmac_cfg->ch_cfg.ll_config.src);
                curr->lli_item.dst_addr =
                    (uint32_t)(dmac_cfg->ch_cfg.ll_config.dst + offset);
                burst_len = dmac_cfg->ch_cfg.ll_config.src_bl;
            }
            // dma_prep_dev
            tr_len = len > blk_size ? blk_size : len;
            tr_len = (tr_len / burst_len) * burst_len;
            break;
        case DMA_MEMCPY:
            curr->lli_item.src_addr =
                (uint32_t)(dmac_cfg->ch_cfg.ll_config.src + offset);
            curr->lli_item.dst_addr =
                (uint32_t)(dmac_cfg->ch_cfg.ll_config.dst + offset);
            if (first)
                ll_config.lli_trig_mode = DMA_TRIGGER_BY_INTERNAL_EVENT;
            tr_len = len > blk_size ? blk_size : len;
            break;
        case DMA_MEMSET:
            curr->lli_item.src_addr =
                (uint32_t)(dmac_cfg->ch_cfg.ll_config.src);
            curr->lli_item.dst_addr =
                (uint32_t)(dmac_cfg->ch_cfg.ll_config.dst + offset);
            if (first)
                ll_config.lli_trig_mode = DMA_TRIGGER_BY_INTERNAL_EVENT;
            tr_len = len > blk_size ? blk_size : len;
            break;
        default:
            break;
        }
        if (DMA_CYCLIC == type) {
            tr_len = period_len;
        } else {
            if (offset + tr_len >= len) { // last
                switch (dmac_cfg->ch_cfg.ll_config.lop_mode) {
                case DMA_LOOP_MODE_0: // m2m, r2m
                case DMA_LOOP_MODE_1: // m2p, p2m
                    tr_len = len - offset;
                    break;
                case DMA_LOOP_MODE_2: // m2p, p2m
                    tr_len = get_align_sz(len - offset);
                    break;
                default:
                    LOGE("unsupport loop mode \n");
                    break;
                }
            }
        }

        ll_config.buf_size = tr_len;
        if (!first) {
            first = curr;
            dmac_cfg->ch_cfg.ll_config.linklist_addr =
                (uint32_t)&first->lli_item;
        } else {
            prev->lli_item.link_addr = (uint32_t)&curr->lli_item;
            arch_clean_cache_range((addr_t)&prev->lli_item,
                                   sizeof(sdrv_dmac_lli_node_t));
        }
        if (DMA_CYCLIC != type) {
            if (offset + tr_len >= len) {
                ll_config.last_mad = 1;
                ll_config.first_mad = 0;
            } else if (offset == 0) {
                ll_config.first_mad = 1;
                ll_config.last_mad = 0;
            } else {
                ll_config.first_mad = 0;
            }
        }
        sdrv_dmac_format_mad(&ll_config,
                             &curr->lli_item); // format mad memory
        sdrv_dmac_lli_node_t *node = &curr->lli_item;
        LOGV("cur %p  : %#x %#x %#x %#x %#x %#x %#x %#x\n", node,
             node->src_addr, node->dst_addr, node->transaction_ctrl,
             node->block_cfg, node->operat_mode, node->operat_data,
             node->mad_crc, node->link_addr);
        list_add_tail(&first->lli_head, &curr->lli_node);
        prev = curr;
        offset += tr_len;
    }

    lli_desc->phy_addr = (uint32_t)first; // Virtual addresses are not supported
    LOGI("lli_desc->phy: %#x , lli_desc: %p\n", lli_desc->phy_addr, lli_desc);
    if (DMA_CYCLIC == type) {
        curr->lli_item.link_addr = (uint32_t)&first->lli_item;
    } else {
        curr->lli_item.link_addr = 0;
    }
    arch_clean_cache_range((addr_t)&first->lli_item,
                           sizeof(sdrv_dmac_lli_node_t));
    arch_clean_cache_range((addr_t)&curr->lli_item,
                           sizeof(sdrv_dmac_lli_node_t));
    return first;

err_lli:
    LOGE("set lli failed\n");
    if (NULL != first)
        sdrv_dma_free_lli_desc(lli_desc);
    return NULL;
}

/**
 * @brief set dma common config
 *
 * @param[in] dmac_cfg abstracted dmac config info
 * @param[in] chan     requested dma channel info
 */
static void sdrv_dma_set_dmac_common_cfg(dmac_coeff_t *dmac_cfg,
                                         dma_chan_t *chan)
{
    uint32_t xfer_items = 0;
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);
    dmac_cfg->base = chan->device->dma_base;
    dmac_cfg->ch_cfg.chan_id = chan->chan_id;
    dmac_cfg->ch_cfg.ll_config.src_bl =
        dmac->dev_cfg.src_maxburst * dmac->dev_cfg.src_addr_width;
    dmac_cfg->ch_cfg.ll_config.dst_bl =
        dmac->dev_cfg.dst_maxburst * dmac->dev_cfg.dst_addr_width;
    dmac_cfg->ch_cfg.ll_config.transfer_mode = DMA_TRANSFER_MODE_LINKLIST;
    dmac_cfg->ch_cfg.ll_config.buffer_mode = DMA_SINGLE_BUFFER;
    dmac_cfg->ch_cfg.ll_config.lop_mode = DMA_LOOP_MODE_1;
    if (dmac_cfg->ch_cfg.ll_config.lop_mode == DMA_LOOP_MODE_2)
    {
        /* check src/dst burst len */
        xfer_items = 1u << (31 - __builtin_clz(dmac->dev_cfg.src_maxburst));
        if (xfer_items != dmac->dev_cfg.src_maxburst)
            LOGI("WARNING, src items not a multiple of 2, every req will xfer %d "
                 "items \n",
                 xfer_items);
        xfer_items = 1u << (31 - __builtin_clz(dmac->dev_cfg.dst_maxburst));
        if (xfer_items != dmac->dev_cfg.dst_maxburst)
            LOGI("WARNING, dst items not a multiple of 2, every req will xfer %d "
                 "items \n",
                 xfer_items);
    }
    // TODO: will support loop mode 1, than supports variable-length transfers
}

/**
 * @brief dma transfer config
 *
 * @param[in] chan abstracted dma channel info
 * @param[in] addr src buffer address
 * @param[in] len  data size
 * @param[in] param for differnt transaction type,represent different parameters
 * @param[in] flags  enable/disable interrupt
 * @param[in] type  transaction type
 * @return true/false
 */
static bool sdrv_dma_config_transfer(dma_chan_t *chan, void *addr, size_t len,
                                     void *param, uint32_t flags,
                                     dma_transaction_type_e type)
{
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);
    dmac_coeff_t dmac_cfg = {0};
    sdrv_dmac_ch_cfg_t *ch_cfg = NULL;
    lli_list_t *lli_ptr = NULL;
    uint32_t burst_len = 0;
    int32_t muxid = -1;
    if (NULL == addr || 0 == len) {
        LOGE("Invalid arguments");
        return false;
    }

    dmac->tr_type = type;
    dmac_cfg.hw_cfg = dmac->dmac_cfg.hw_cfg; // dma control desc, such as irq
                                             // num and muxid table and etc....
    ch_cfg = &dmac_cfg.ch_cfg;
    if (DMA_MEMCPY == type || DMA_MEMSET == type)
    {
        dma_local_buf_t lbc;
        sdrv_dmac_get_local_buf_prop(chan->device->dma_base, &lbc);
        dmac->dev_cfg.direction = DMA_MEM2MEM;
#ifdef CONFIG_SDRV_DMA_USE_LBC
        dmac->lli_desc.mem_base = (lli_list_t *)(lbc.buf_base);
        dmac->lli_desc.mem_bitmap = sdrv_dma_local_buf_bitmap;
        dmac->lli_desc.mem_size = SDRV_DMA_LLI_BUF_LBC_MAX_LEN;
#else
        dmac->lli_desc.mem_base = sdrv_dma_lli_buf;
        dmac->lli_desc.mem_bitmap = sdrv_dma_lli_bitmap;
        dmac->lli_desc.mem_size = SDRV_DMA_LLI_BUF_MAX_LEN;
#endif
    } else {
        dmac->lli_desc.mem_base = sdrv_dma_lli_buf;
        dmac->lli_desc.mem_bitmap = sdrv_dma_lli_bitmap;
        dmac->lli_desc.mem_size = SDRV_DMA_LLI_BUF_MAX_LEN;
    }

    sdrv_dma_set_dmac_common_cfg(&dmac_cfg, chan);
    switch (dmac->dev_cfg.direction) {
    case DMA_MEM2MEM:
        if (DMA_MEMCPY == type) {
            ch_cfg->ll_config.lop_mode = DMA_LOOP_MODE_0; // high performance
            ch_cfg->ll_config.switch_event_ctrl =
                DMA_SWT_EVT_CTL_CONTINUE_WTH_INT;
            ch_cfg->ll_config.src = (uint8_t *)(addr_t)addr;
            ch_cfg->ll_config.dst = (uint8_t *)(addr_t)param;
            ch_cfg->ll_config.src_bz = DMA_BEAT_SIZE_8_BYTES;
            ch_cfg->ll_config.dst_bz = DMA_BEAT_SIZE_8_BYTES;
            ch_cfg->ll_config.src_port_sel = DMA_PORT_AXI64;
            ch_cfg->ll_config.dst_port_sel = DMA_PORT_AXI64;
            ch_cfg->ll_config.src_bt = DMA_BURST_INCREMENTAL;
            ch_cfg->ll_config.dst_bt = DMA_BURST_INCREMENTAL;
            ch_cfg->ll_config.flow_ctrl = DMA_DIR_MEM2MEM;
            ch_cfg->ll_config.lli_trig_mode = DMA_TRIGGER_BY_SOFTWARE;

            // memcpy high performance
            ch_cfg->ll_config.src_cache = 0;
            ch_cfg->ll_config.dst_cache = 1;
            sdrv_dmac_set_ch_rd_outstanding(chan->device->dma_base,
                                            chan->chan_id, 0xf);
            sdrv_dmac_set_ch_wr_outstanding(chan->device->dma_base,
                                            chan->chan_id, 0xf);
        } else if (DMA_MEMSET == type) {
            dmac->memset_val = *((uint8_t *)param);
            uint32_t memset_val = dmac->memset_val | dmac->memset_val << 8 |
                                  dmac->memset_val << 16 |
                                  dmac->memset_val << 24;
            ch_cfg->ll_config.lop_mode = DMA_LOOP_MODE_0; // high performance
            ch_cfg->ll_config.switch_event_ctrl =
                DMA_SWT_EVT_CTL_CONTINUE_WTH_INT;
            ch_cfg->ll_config.src = 0x0;
            ch_cfg->ll_config.dst = (uint8_t *)(addr_t)addr;
            ch_cfg->ll_config.src_bz = DMA_BEAT_SIZE_8_BYTES;
            ch_cfg->ll_config.dst_bz = DMA_BEAT_SIZE_8_BYTES;
            ch_cfg->ll_config.src_port_sel = DMA_PORT_AXI64;
            ch_cfg->ll_config.dst_port_sel = DMA_PORT_AXI64;
            ch_cfg->ll_config.src_bt = DMA_BURST_FIXED;
            ch_cfg->ll_config.dst_bt = DMA_BURST_INCREMENTAL;
            ch_cfg->ll_config.flow_ctrl = DMA_DIR_REG2MEM;
            ch_cfg->ll_config.lli_trig_mode = DMA_TRIGGER_BY_SOFTWARE;
            sdrv_dmac_set_local_val(chan->device->dma_base, chan->chan_id,
                                    memset_val);
            // memset high performance
            ch_cfg->ll_config.src_cache = 0;
            ch_cfg->ll_config.dst_cache = 1;
            sdrv_dmac_set_ch_rd_outstanding(chan->device->dma_base,
                                            chan->chan_id, 0xf);
            sdrv_dmac_set_ch_wr_outstanding(chan->device->dma_base,
                                            chan->chan_id, 0xf);
        }
        break;
    case DMA_MEM2DEV:
        ch_cfg->ll_config.switch_event_ctrl = DMA_SWT_EVT_CTL_CONTINUE_WTH_INT;
        ch_cfg->ll_config.src = (uint8_t *)addr;
        ch_cfg->ll_config.dst = (uint8_t *)dmac->dev_cfg.dst_addr;

        ch_cfg->ll_config.src_bz = __builtin_ctz(dmac->dev_cfg.src_addr_width);
        ch_cfg->ll_config.dst_bz = __builtin_ctz(dmac->dev_cfg.dst_addr_width);
        ch_cfg->ll_config.src_port_sel = DMA_PORT_AXI64;
        ch_cfg->ll_config.dst_port_sel = DMA_PORT_AHB32;
        ch_cfg->ll_config.src_bt = DMA_BURST_INCREMENTAL;
        ch_cfg->ll_config.dst_bt = DMA_BURST_FIXED;
        ch_cfg->ll_config.flow_ctrl = DMA_DIR_MEM2DEV;
        ch_cfg->ll_config.lli_trig_mode = DMA_TRIGGER_BY_HARDWARE;
        ch_cfg->ll_config.src_cache = 0;
        ch_cfg->ll_config.dst_cache = 0;
        burst_len = ch_cfg->ll_config.dst_bl;
        muxid = sdrv_dma_get_muxid(dmac_cfg.hw_cfg->mux_table,
                                   dmac_cfg.hw_cfg->mux_table_len, DMA_MUX_WR,
                                   (uint32_t)ch_cfg->ll_config.dst);
        if (muxid >= 0) {
            ch_cfg->ll_config.mux_id = muxid;
            sdrv_dmac_set_ch_muxid(chan->device->dma_base, chan->chan_id,
                                   ch_cfg->ll_config.mux_id);
        } else {
            LOGE("invalid muxid");
            return false;
        }
        break;
    case DMA_DEV2MEM:
        ch_cfg->ll_config.src = (uint8_t *)dmac->dev_cfg.src_addr;
        ch_cfg->ll_config.dst = (uint8_t *)addr;

        ch_cfg->ll_config.switch_event_ctrl = DMA_SWT_EVT_CTL_CONTINUE_WTH_INT;
        ch_cfg->ll_config.src_bz = __builtin_ctz(dmac->dev_cfg.src_addr_width);
        ch_cfg->ll_config.dst_bz = __builtin_ctz(dmac->dev_cfg.dst_addr_width);
        ch_cfg->ll_config.src_port_sel = DMA_PORT_AHB32;
        ch_cfg->ll_config.dst_port_sel = DMA_PORT_AXI64;
        ch_cfg->ll_config.src_bt = DMA_BURST_FIXED;
        ch_cfg->ll_config.dst_bt = DMA_BURST_INCREMENTAL;
        ch_cfg->ll_config.flow_ctrl = DMA_DIR_DEV2MEM;
        ch_cfg->ll_config.lli_trig_mode = DMA_TRIGGER_BY_HARDWARE;
        ch_cfg->ll_config.src_cache = 0;
        ch_cfg->ll_config.dst_cache = 0;
        burst_len = ch_cfg->ll_config.src_bl;
        muxid = sdrv_dma_get_muxid(dmac_cfg.hw_cfg->mux_table,
                                   dmac_cfg.hw_cfg->mux_table_len, DMA_MUX_RD,
                                   (uint32_t)ch_cfg->ll_config.src);
        if (muxid >= 0) {
            ch_cfg->ll_config.mux_id = muxid;
            sdrv_dmac_set_ch_muxid(chan->device->dma_base, chan->chan_id,
                                   ch_cfg->ll_config.mux_id);
        } else {
            LOGE("invalid muxid");
            return false;
        }
        break;
    default:
        break;
    }
    // TODO: Check src addr and dst addr.
    if (DMA_INTERRUPT & flags) {

        uint32_t int_type = DMA_CH_INT_EN_CH_ABORT;
        if (type == DMA_CYCLIC) {
            int_type = int_type | DMA_CH_INT_EN_EVERY_MAD_DONE;
        } else {
            int_type = int_type | DMA_CH_INT_EN_LST_MAD_DONE;
        }
        sdrv_dmac_set_int_state(chan->device->dma_base, chan->chan_id,
                                int_type);
    }

        lli_ptr = sdrv_dma_set_lli(
            &dmac_cfg, len,
            sdrv_dmac_get_max_block_size(ch_cfg->ll_config.lop_mode, burst_len), param,
            type, dmac->dev_cfg.direction, &dmac->lli_desc);

    if (NULL == lli_ptr) {
        return false;
    }
    osMutexAcquire(chan->chan_lock, osWaitForever);

    dmac->chan.flags = flags;
    dmac->len = len;
    sdrv_dmac_set_mad(chan->device->dma_base, chan->chan_id,
                       &lli_ptr->lli_item);

    osMutexRelease(chan->chan_lock);

    return true;
}

/**
 * @brief dma memcpy transfer config
 *
 * @param[in] chan requested dma channel info
 * @param[in] dst dst address
 * @param[in] src src address
 * @param[in] buf_len  data size
 * @param[in] flags  enable/disable interrupt
 * @return true/false
 */
static bool sdrv_dma_prep_memcpy(dma_chan_t *chan, void *dst, void *src,
                                 size_t buf_len, uint32_t flags)
{
    // FIXME: buf_len not an integer multiple of 2 is not supported
    return sdrv_dma_config_transfer(chan, src, buf_len, dst, flags, DMA_MEMCPY);
}

/**
 * @brief dma memset transfer config
 *
 * @param[in] chan requested dma channel info
 * @param[in] val set value
 * @param[in] buf_addr dst address
 * @param[in] buf_len  data size
 * @param[in] flags  enable/disable interrupt
 * @return true/false
 */
static bool sdrv_dma_prep_memset(dma_chan_t *chan, uint8_t val, void *buf_addr,
                                 size_t buf_len, uint32_t flags)
{
    return sdrv_dma_config_transfer(chan, buf_addr, buf_len, &val, flags,
                                    DMA_MEMSET);
}
static uint32_t sdrv_dma_get_maxburst_bytes(dma_chan_t *chan)
{
    uint32_t bytes = 0;
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);
    switch (dmac->dev_cfg.direction) {
    case DMA_MEM2DEV:
        bytes = dmac->dev_cfg.dst_maxburst * dmac->dev_cfg.dst_addr_width;
        break;
    case DMA_DEV2MEM:
        bytes = dmac->dev_cfg.src_maxburst * dmac->dev_cfg.src_addr_width;
        break;
    case DMA_MEM2MEM:
    case DMA_DEV2DEV:
        LOGI("Unsupport direction \n");
        break;
    }
    return bytes;
}
static bool sdrv_dma_check_cfg(dma_chan_t *chan)
{
    bool ret = false;
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);

    ret = sdrv_dma_get_maxburst_bytes(chan) <= dmac->blk_size;
    if (!ret)
        LOGE("maxburst too large, must be less than or equal to %d \n",
             dmac->blk_size / dmac->dev_cfg.src_addr_width);
    return ret;
}
/**
 * @brief dma dev2mem/mem2dev transfer config
 *
 * @param[in] chan requested dma channel info
 * @param[in] buf_addr src/dst address
 * @param[in] buf_len  data size
 * @param[in] flags  enable/disable interrupt
 * @return true/false
 */
static bool sdrv_dma_prep_dev(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                              uint32_t flags)
{
    if (!sdrv_dma_check_cfg(chan))
        return false;
    LOGI("buf_addr: %p\n", buf_addr);
    return sdrv_dma_config_transfer(chan, buf_addr, buf_len, NULL, flags,
                                    DMA_DEVICE);
}

/**
 * @brief dma dev2mem/mem2dev loop transfer config
 *
 * @param[in] chan requested dma channel info
 * @param[in] buf_addr src/dst address
 * @param[in] buf_len  data size
 * @param[in] flags  enable/disable interrupt
 * @return true/false
 */
static bool sdrv_dma_prep_cyclic(dma_chan_t *chan, void *buf_addr,
                                 size_t buf_len, size_t period_len,
                                 uint32_t flags)
{
    // check params
    bool ret = false;
    uint32_t maxburst_bytes = sdrv_dma_get_maxburst_bytes(chan);
    LOGI("buf_len: %d, period_len: %d, maxburst_bytes: %d \n", buf_len,
         period_len, maxburst_bytes);

    ((period_len % maxburst_bytes) == 0) ? (ret = true) : (ret = false);
    if (!ret) {
        LOGE("the period_len(%d) must be an integer multiple of the amount "
             "of "
             "data per req(%d)\n",
             period_len, maxburst_bytes);
        return ret;
    }
    return sdrv_dma_config_transfer(chan, buf_addr, buf_len, &period_len, flags,
                                    DMA_CYCLIC);
}

/**
 * @brief enable channel
 *
 * @param[in] chan requested channel info
 */
static void sdrv_dma_ch_enable(dma_chan_t *chan)
{
    ASSERT(NULL != chan);
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);
    LOGV("enable dmac: %p, chan: %d\n", (void *)chan->device->dma_base,
         chan->chan_id);

    osMutexAcquire(dmac->chan.chan_lock, osWaitForever);

    dmac->chan.chan_status = DMA_IN_PROGRESS;
    sdrv_dmac_enable_ch(chan->device->dma_base, chan->chan_id,
                        dmac->dmac_cfg.ch_cfg.ps_cnt);
    LOGI("en : %d\n",
         sdrv_dmac_get_ch_en(chan->device->dma_base, chan->chan_id));
    if (dmac->dev_cfg.direction == DMA_MEM2MEM)
        sdrv_dmac_set_ch_sw_handshake(chan->device->dma_base, chan->chan_id);
    osMutexRelease(dmac->chan.chan_lock);
}

/**
 * @brief disable channel
 *
 * @param[in] chan requested channel info
 */
static bool sdrv_dma_ch_disable(dma_chan_t *chan)
{
    ASSERT(NULL != chan);
    bool ret = true;
    uint32_t chann_en = 0;
    sdrv_dma_chan_t *dmac = to_sdrv_dma_chan(chan);

    osMutexAcquire(dmac->chan.chan_lock, osWaitForever);
    chann_en = sdrv_dmac_get_ch_en(chan->device->dma_base, chan->chan_id);

    if (chann_en) {
        ret = sdrv_dmac_set_ch_cmd(chan->device->dma_base, chan->chan_id,
                                   DMA_CH_CTL_STAT_ABORT) == 0
                  ? true
                  : false;
    }
    dmac->dmac_cfg.ch_cfg.ps_cnt =
        sdrv_dmac_get_ch_pscnt(chan->device->dma_base, chan->chan_id);
    dmac->chan.chan_status = DMA_COMPLETED;
    osMutexRelease(dmac->chan.chan_lock);

    return ret;
}

/**
 * @brief sdrv_dma chan free
 *
 * @param[in] chan requested chan info
 */
static void sdrv_dma_chan_free(dma_chan_t *chan)
{
    ASSERT(NULL != chan);
    LOGV("release dma: %#x, chann: %d\n", chan->device->dma_base,
         chan->chan_id);
    sdrv_dma_chan_t *sdc = to_sdrv_dma_chan(chan);

    osMutexAcquire(sdc->chan.chan_lock, osWaitForever);

    chan->allocated = 0;

    osMutexRelease(sdc->chan.chan_lock);

    sdrv_dma_free_lli_desc(&sdc->lli_desc);
}

/**
 * @brief get channel status when linklist single
 *
 * @param[in] chan requested channel info
 */
static dma_status_e sdrv_dmac_get_chan_status(dma_chan_t *chan)
{
    ASSERT(NULL != chan);

    // Don't change channel status here
    if (sdrv_dmac_get_ch_lastmad_done(chan->device->dma_base, chan->chan_id))
        return DMA_COMPLETED;
    else
        return DMA_IN_PROGRESS;
}

/**
 * @brief dma ops
 *
 */
static dma_ops_t dma_ops = {
    .request_chan = sdrv_dma_chan_req,
    .release_chan = sdrv_dma_chan_free,
    .prep_memcpy = sdrv_dma_prep_memcpy,
    .prep_memset = sdrv_dma_prep_memset,
    .prep_dev = sdrv_dma_prep_dev,
    .prep_cyclic = sdrv_dma_prep_cyclic,
    .config_chan = sdrv_dma_dev_config,
    .enable_chan = sdrv_dma_ch_enable,
    .disable_chan = sdrv_dma_ch_disable,
    .get_ch_status = sdrv_dmac_get_chan_status,
};

/**
 * @brief sdrv_dma init
 *
 * @param[in] dev dma control instance
 * @param[in] cfg dma control config,see sdrv_dma_cfg_t
 */
void sdrv_dma_init(dma_dev_t *dev, sdrv_dma_cfg_t *cfg)
{
    ASSERT(NULL != dev);
    LOGV("sdrv dma inited, dev: %p, base: 0x%08x\n", dev, dev->dma_base);
    sdrv_dma_chan_t *sdrv_chan;
    uint32_t pool_sz = 0;

    dev->ops = &dma_ops;

    dma_init(dev);
    sdrv_dmac_reset_fifo(dev->dma_base);
#ifdef CONFIG_SDRV_DMA_1_0
    sdrv_dmac_core_reset_irq_sta(dev->dma_base);
#endif
    pool_sz = sizeof(sdrv_dma_chan_t) * dev->channel_nr;
    sdrv_chan = osAlloc(pool_sz);
    memset((void *)sdrv_chan, 0, pool_sz);

    LOGV("Alloc dma chann len: %d, channels: %d\n", sizeof(sdrv_dma_chan_t),
         dev->channel_nr);
    ASSERT(NULL != sdrv_chan);

    lli_list_lock = osMutexNew(NULL);
    ASSERT(NULL != lli_list_lock);

    for (int i = 0; i < dev->channel_nr; i++) {
        sdrv_chan[i].chan.chan_lock = osMutexNew(NULL);
        ASSERT(NULL != sdrv_chan->chan.chan_lock);
        sdrv_chan[i].chan.chan_id = i;
        sdrv_chan[i].chan.irq_id = cfg->irq_id_table[i];
        sdrv_chan[i].chan.allocated = 0;
        sdrv_chan[i].chan.device = dev;
        sdrv_chan[i].chan.chan_status = DMA_COMPLETED;
        sdrv_chan[i].chan.priority = dev->priority;
        sdrv_chan[i].blk_size = SDRV_DMAC_MAX_BLK_SIZE;

        // Notice: cfg don't local variables, must global variables or heap
        sdrv_chan[i].dmac_cfg.hw_cfg = cfg;
        LOGV("New chann: %p (%d)\n", &sdrv_chan[i].chan,
             sdrv_chan[i].chan.chan_id);
        list_add_tail(&dev->channels, &sdrv_chan[i].chan.node);
    }

    for (int i = 0; i < cfg->irq_num; i++) {
        irq_attach(cfg->irq_id_table[i], sdrv_dma_irq_handler, dev);
        irq_enable(cfg->irq_id_table[i]);
    }
}
