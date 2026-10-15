/**
 * @file  dw_dma.c
 * @brief dma hardware interface.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 */
#include <cmsis_os2.h>
#include <debug.h>
#include <string.h>
#include <spinlock.h>
#include <sd_errcode.h>

#include "driver/kunlun/dma/dw_dma.h"
#include "driver/memmap.h"

#include "arch/cache.h"
#include "bits.h"
#include "driver/irq.h"
#include "dw_dmac.h"
#include "board.h"

#ifndef DDR_DATA
#define DDR_DATA
#endif

#define DW_DMA_LLI_BUF_MAX_LEN 120

typedef enum {
    AW_BUFFERABLE = 1 << 0,
    AW_MODIFIABLE = 1 << 1,
    AW_OTHER_ALLOCATE = 1 << 2,
    AW_ALLOCATE = 1 << 3,
} awcache;

typedef enum {
    AR_BUFFERABLE = 1 << 0,
    AR_MODIFIABLE = 1 << 1,
    AR_ALLOCATE = 1 << 2,
    AR_OTHER_ALLOCATE = 1 << 3,
} arcache;

static unsigned long
    dw_dma_lli_bitmap[BITMAP_NUM_WORDS(DW_DMA_LLI_BUF_MAX_LEN)] DDR_DATA= {0};
static lli_list_t dw_dma_lli_buf[DW_DMA_LLI_BUF_MAX_LEN] DDR_DATA= {0};
static spin_lock_t lli_list_lock;

/**
 * @brief dma irq callback
 *
 * @param[in] irq irq id
 * @param[in] arg private data point (see dma_dev_t)
 */
static int dw_dma_irq_handler(uint32_t irq, void *arg)
{
    dma_dev_t *dev = (dma_dev_t *)arg;
    uint32_t ch_intstatus = 0;
    dma_status_e status = DMA_COMPLETED;
    dma_chan_t *cptr = NULL;
    uint32_t chan_id = 0;

    list_for_every_entry (&dev->channels, cptr, dma_chan_t, node) {
        chan_id = cptr->chan_id;

        if (dw_dmac_ch_has_int_status(dev->dma_base, chan_id)) {
            ch_intstatus = dw_dmac_get_ch_int_stat(dev->dma_base, chan_id);
            dw_dmac_clr_ch_int_stat(dev->dma_base, chan_id, ch_intstatus);

            if (cptr->irq_cb) {
                status = dw_dmac_irq_status_handle(ch_intstatus);
                if (DMA_ERR == status)
                    ERRCODE_PRINT(MOD_ID_DMA,
                                  dev->dma_base + (DMAC_CH_REG_STEP * chan_id),
                                  DMA_ERR_INTERRUPT, ch_intstatus);
                cptr->irq_cb(status, ch_intstatus, cptr->context);
            }
        }
    }

    return 0;
}

/**
 * @brief dw_dma chan request
 *
 * @param[in] dev dma control instance
 * @param[in] id  random if null
 * @return chan info
 */
static dma_chan_t *dw_dma_chan_req(dma_dev_t *dev, uint32_t *id)
{
    dma_chan_t *cptr = NULL;

    list_for_every_entry (&dev->channels, cptr, dma_chan_t, node) {

        if (0 == cptr->allocated) {
            if ((NULL == id) || (*id == cptr->chan_id)) {
                cptr->irq_cb = NULL;
                cptr->context = NULL;
                cptr->allocated = 1;
                return cptr;
            }
        }
    }

    ssdk_printf(SSDK_CRIT,
                "DMA(%#08x) no available channel for request", dev->dma_base);
    ERRCODE_PRINT(MOD_ID_DMA, dev->dma_base, DMA_ERR_NO_CHAN_AVAILABLE, 0);
    return NULL;
}

/**
 * @brief set dma device config (only for mem2dev/dev2mem)
 *
 * @param[in] chan requested chan info
 * @param[in] cfg device config
 */
static void dw_dma_dev_config(dma_chan_t *chan, const dma_dev_cfg_t *cfg)
{
    ASSERT((NULL != chan) && (NULL != cfg));
    irq_state_t state;

    state = spin_lock_irqsave(&chan->chan_lock);

    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);

    memcpy(&dwc->dev_cfg, cfg, sizeof(*cfg));

    spin_unlock_irqrestore(&chan->chan_lock, state);
}

/**
 * @brief check chan valid
 *
 * @param[in] chan requested chan info
 * @param[in] addr address(src/dst)
 */
static int dw_dma_validate_address(dma_chan_t *chan, paddr_t src_addr,
                                   paddr_t dst_addr)
{
    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    uint8_t valid_flag = 0;

    for (int i = 0; i < dwc->dmac_cfg.dw_dma_cfg->addr_table_len; i++) {
        if (src_addr >= dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr &&
            src_addr < dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr +
                           dwc->dmac_cfg.dw_dma_cfg->addr_table[i].len)
            valid_flag |= 0x01;

        if (dst_addr >= dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr &&
            dst_addr < dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr +
                           dwc->dmac_cfg.dw_dma_cfg->addr_table[i].len)
            valid_flag |= 0x02;
    }

    if (valid_flag == 0x03)
        return 0;
    else {
        ssdk_printf(SSDK_CRIT,
                    "invalid address(sar: %#08x, dar: %#08x), flag: %#x",
                    src_addr, dst_addr, valid_flag);
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_INVALID_ADDR,
                      valid_flag);
        return -1;
    }
}

/**
 * @brief check chan valid
 *
 * @param[in] chan requested chan info
 * @param[in] addr address(src/dst)
 */
static int dw_dma_validate_address64(dma_chan_t *chan, uint64_t src_addr,
                                   uint64_t dst_addr)
{
    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    uint8_t valid_flag = 0;

    for (int i = 0; i < dwc->dmac_cfg.dw_dma_cfg->addr_table_len; i++) {
        if (src_addr >= dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr &&
            src_addr < dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr +
                           dwc->dmac_cfg.dw_dma_cfg->addr_table[i].len)
            valid_flag |= 0x01;

        if (dst_addr >= dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr &&
            dst_addr < dwc->dmac_cfg.dw_dma_cfg->addr_table[i].base_addr +
                           dwc->dmac_cfg.dw_dma_cfg->addr_table[i].len)
            valid_flag |= 0x02;
    }

    if (valid_flag == 0x03)
        return 0;
    else {
        ssdk_printf(SSDK_CRIT,
                    "invalid address(sar: %#llx, dar: %#llx), flag: %#x",
                    src_addr, dst_addr, valid_flag);
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base, DMA_ERR_INVALID_ADDR,
                      chan->chan_id);
        return -1;
    }
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
static void dw_dma_free_lli(lli_list_t *list)
{
    lli_list_t *curr = NULL;
    lli_list_t *first = list;
    lli_list_t *next = NULL;
    irq_state_t state;

    state = spin_lock_irqsave(&lli_list_lock);

    if (first) {
        list_for_every_entry_safe (&first->dw_list, curr, next, lli_list_t,
                                lli_node) {
            list_delete(&curr->lli_node);
            bitmap_clear(dw_dma_lli_bitmap, curr->bitmap_index);
        }
    }

    spin_unlock_irqrestore(&lli_list_lock, state);

exit:
    return;
}

/**
 * @brief create link list
 *
 * @return alloced lli_list_t data point
 */
static lli_list_t *dw_dma_get_lli_ptr(void)
{
    lli_list_t *lli_ptr = NULL;
    int index = 0;
    irq_state_t state;

    state = spin_lock_irqsave(&lli_list_lock);

    index = bitmap_ffz(dw_dma_lli_bitmap, DW_DMA_LLI_BUF_MAX_LEN);

    if (-1 != index) {
        lli_ptr = &dw_dma_lli_buf[index];

        list_initialize(&lli_ptr->dw_list);

        lli_ptr->phy_addr = (uint64_t)p2ap((addr_t)(&lli_ptr->lli_item));
        lli_ptr->bitmap_index = index;

        bitmap_set(dw_dma_lli_bitmap, index);
    }

    spin_unlock_irqrestore(&lli_list_lock, state);

    return lli_ptr;
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
static lli_list_t *dw_dma_set_lli(dmac_coeff_t *dmac_cfg, uint32_t len,
                                  uint32_t blk_size, void *param,
                                  dma_transaction_type_e type,
                                  dma_tr_direction_e direction)
{
    lli_list_t *first = NULL;
    lli_list_t *curr = NULL;
    lli_list_t *prev = NULL;
    size_t tr_count = 0;
    size_t tr_len = 0;
    ch_ctl_t ctrl_cfg = {0};
    uint32_t period_len = ((DMA_CYCLIC == type) ? *((uint32_t *)param) : 0);

    dmac_cfg->block_transfer_size =
        bytes2block(blk_size, len, dmac_cfg->src_tr_width, &tr_count);

    dw_dmac_get_ctrl_cfg(dmac_cfg, &ctrl_cfg);

    for (uint32_t offset = 0; offset < len; offset += tr_count) {
        curr = dw_dma_get_lli_ptr();
        if (!curr) {
            ssdk_printf(SSDK_CRIT,
                        "DMA(%08x), chan(%d), LLI resource no available",
                        dmac_cfg->dma_base, dmac_cfg->chan_id);
            ERRCODE_PRINT(MOD_ID_DMA, dmac_cfg->dma_base,
                          DMA_ERR_NO_LLI_RES_AVAILABLE, dmac_cfg->chan_id);
            goto err_lli;
        }

        switch (type) {
        case DMA_CYCLIC:
        case DMA_DEVICE:
            if (DMA_MEM2DEV == direction) {
                curr->lli_item.sar = dmac_cfg->src_addr + offset;
                curr->lli_item.dsr = dmac_cfg->dst_addr;
            } else {
                curr->lli_item.sar = dmac_cfg->src_addr;
                curr->lli_item.dsr = dmac_cfg->dst_addr + offset;
            }
            break;
        case DMA_MEMCPY:
            curr->lli_item.sar = dmac_cfg->src_addr + offset;
            curr->lli_item.dsr = dmac_cfg->dst_addr + offset;
            break;
        case DMA_MEMSET:
            curr->lli_item.sar = dmac_cfg->src_addr;
            curr->lli_item.dsr = dmac_cfg->dst_addr + offset;
            break;
        default:
            break;
        }

        tr_len = ((DMA_CYCLIC == type) ? period_len : (len - offset));

        curr->lli_item.block_size =
            bytes2block(blk_size, tr_len, dmac_cfg->src_tr_width, &tr_count) -
            1;

        tr_count = ((DMA_CYCLIC == type) ? period_len : tr_count);

        curr->lli_item.ctl = ctrl_cfg.vh;
        curr->lli_item.ctl <<= 32;
        curr->lli_item.ctl |= ctrl_cfg.vl;

        if (!first)
            first = curr;
        else
            prev->lli_item.llp = curr->phy_addr;

        list_add_tail(&first->dw_list, &curr->lli_node);
        prev = curr;
    }

    dmac_cfg->llp = first->phy_addr;
    if (DMA_CYCLIC == type) {
        curr->lli_item.llp = first->phy_addr;
    } else {
        curr->lli_item.ctl |= DW_DMAC_LLI_LAST_OFFSET;
        curr->lli_item.llp = 0;
    }

    list_for_every_entry (&first->dw_list, curr, lli_list_t, lli_node) {
        arch_clean_cache_range((addr_t)&curr->lli_item, sizeof(lli_list_t));
    }

    return first;
err_lli:
    if (NULL != first)
        dw_dma_free_lli(first);

    return NULL;
}

/**
 * @brief set dma common config
 *
 * @param[in] dmac_cfg abstracted dmac config info
 * @param[in] chan     requested dma channel info
 */
static void dw_dma_set_dmac_common_cfg(dmac_coeff_t *dmac_cfg, dma_chan_t *chan)
{
    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    dmac_cfg->dma_base = chan->device->dma_base;
    dmac_cfg->mux_base = chan->device->mux_base;
    dmac_cfg->chan_id = chan->chan_id;

    dmac_cfg->src_transfer_type = DMA_TRANSFER_TYPE_LLI;
    dmac_cfg->dst_transfer_type = DMA_TRANSFER_TYPE_LLI;

    dmac_cfg->channel_pri = dwc->chan.priority;
    if (DMA_MEMSET == dwc->tr_type) {
        dmac_cfg->arlen = 0;
        dmac_cfg->awlen = 0;
    } else {
        dmac_cfg->arlen = 3;
        dmac_cfg->awlen = 3;
    }
    dmac_cfg->arlen_en = 1;
    dmac_cfg->awlen_en = 1;

    dmac_cfg->hs_sel_src = 0;
    dmac_cfg->hs_sel_dst = 0;
    dmac_cfg->llp = 0;

    dmac_cfg->src_msize = dwc->dev_cfg.src_maxburst > 1 ?
                          (32-__builtin_clz(dwc->dev_cfg.src_maxburst)) - 2 : 0;
    dmac_cfg->dst_msize = dwc->dev_cfg.dst_maxburst > 1 ?
                          (32-__builtin_clz(dwc->dev_cfg.dst_maxburst)) - 2 : 0;
    dmac_cfg->dma_transfer_type = dwc->dev_cfg.direction;
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
static bool dw_dma_config_transfer(dma_chan_t *chan, void *addr, size_t len,
                                   void *param, uint32_t flags,
                                   dma_transaction_type_e type)
{
    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    dmac_coeff_t dmac_cfg = {0};
    lli_list_t *lli_ptr = NULL;
    bool set_int_flag = 0;

    if (NULL == addr || 0 == len) {
        ssdk_printf(SSDK_CRIT, "%s %d invalid arguments", __func__, __LINE__);
        ERRCODE_PRINT(MOD_ID_DMA,
                      chan->device->dma_base,
                      DMA_ERR_INVALID_ARGUMENTS, chan->chan_id);
        return false;
    }

    dwc->tr_type = type;
    dmac_cfg.dw_dma_cfg = dwc->dmac_cfg.dw_dma_cfg;

    if (DMA_MEMCPY == type || DMA_MEMSET == type)
        dwc->dev_cfg.direction = DMA_MEM2MEM;

    dw_dma_set_dmac_common_cfg(&dmac_cfg, chan);

    switch (dwc->dev_cfg.direction) {
    case DMA_MEM2DEV:
        dmac_cfg.src_addr = p2ap((addr_t)addr);
        dmac_cfg.dst_addr = p2ap(dwc->dev_cfg.dst_addr);
        if (DMA_CYCLIC == type)
            dmac_cfg.src_tr_width =
                __builtin_ctz((addr_t)addr | (*(size_t *)param) | 4);
        else
            dmac_cfg.src_tr_width = __builtin_ctz((addr_t)addr | len | 4);
        dmac_cfg.dst_tr_width = __builtin_ctz(dwc->dev_cfg.dst_addr_width);
        dmac_cfg.sinc = DMA_TRANSFER_ADDR_INCREMENT;
        dmac_cfg.dinc = DMA_TRANSFER_ADDR_NOCHANGE;
        break;
    case DMA_DEV2MEM:
        dmac_cfg.src_addr = p2ap(dwc->dev_cfg.src_addr);
        dmac_cfg.dst_addr = p2ap((addr_t)addr);
        dmac_cfg.src_tr_width = __builtin_ctz(dwc->dev_cfg.src_addr_width);
        if (DMA_CYCLIC == type)
            dmac_cfg.dst_tr_width =
                __builtin_ctz((addr_t)addr | (*(size_t *)param) | 4);
        else
            dmac_cfg.dst_tr_width = __builtin_ctz((addr_t)addr | len | 4);
        dmac_cfg.sinc = DMA_TRANSFER_ADDR_NOCHANGE;
        dmac_cfg.dinc = DMA_TRANSFER_ADDR_INCREMENT;
        break;
    case DMA_MEM2MEM:
        if (DMA_MEMCPY == type) {
            dmac_cfg.src_addr = p2ap((addr_t)addr);
            dmac_cfg.dst_addr = p2ap((addr_t)param);
            dmac_cfg.src_tr_width =
                __builtin_ctz((addr_t)addr | (addr_t)param | len | 4);
            dmac_cfg.dst_tr_width = dmac_cfg.src_tr_width;
            dmac_cfg.sinc = DMA_TRANSFER_ADDR_INCREMENT;
            dmac_cfg.dinc = DMA_TRANSFER_ADDR_INCREMENT;
            dmac_cfg.src_msize = DMA_BURST_TR_16ITEMS;
            dmac_cfg.dst_msize = DMA_BURST_TR_16ITEMS;
        } else if (DMA_MEMSET == type) {
            dwc->memset_val = *((uint8_t *)param);
            dmac_cfg.src_addr = p2ap((addr_t)(&dwc->memset_val));
            dmac_cfg.dst_addr = p2ap((addr_t)addr);
            dmac_cfg.src_transfer_type = DMA_TRANSFER_TYPE_CONTINUOUS;
            dmac_cfg.dst_transfer_type = DMA_TRANSFER_TYPE_LLI;
            dmac_cfg.src_tr_width = DMA_TRANSFER_WIDTH_8BITS;
            dmac_cfg.dst_tr_width = dmac_cfg.src_tr_width;
            dmac_cfg.sinc = DMA_TRANSFER_ADDR_NOCHANGE;
            dmac_cfg.dinc = DMA_TRANSFER_ADDR_INCREMENT;
            dmac_cfg.src_msize = DMA_BURST_TR_1ITEM;
            dmac_cfg.dst_msize = DMA_BURST_TR_16ITEMS;
        }
        dmac_cfg.ar_cache = AR_BUFFERABLE | AR_MODIFIABLE;
        dmac_cfg.aw_cache = AW_BUFFERABLE | AW_MODIFIABLE;
        break;
    default:
        break;
    }

    if (0 !=
        dw_dma_validate_address(chan, dmac_cfg.src_addr, dmac_cfg.dst_addr)) {
        ssdk_printf(
            SSDK_CRIT,
            "invalid address chan: %p, src_addr: 0x%llx, dst_addr: 0x%llx\n",
            chan, dmac_cfg.src_addr, dmac_cfg.dst_addr);
        return false;
    }

    set_int_flag = (DMA_INTERRUPT & flags) ? true : false;
    uint8_t int_type = (type == DMA_CYCLIC) ? 1 : 0;

    if (!set_int_flag)
        dw_dmac_ch_intsignal_cfg(chan->device->dma_base, chan->chan_id, 0);
    else
        dw_dmac_ch_intsignal_cfg(chan->device->dma_base, chan->chan_id,
                                 0xffffffffffffffff);

    dw_dmac_set_int_stat(&dmac_cfg, set_int_flag, int_type);

    if(dwc->lli_list_ptr != NULL) {
        dw_dma_free_lli(dwc->lli_list_ptr);
    }

    lli_ptr = dw_dma_set_lli(&dmac_cfg, len, dwc->blk_size, param, type,
                             dwc->dev_cfg.direction);

    if (NULL == lli_ptr) {
        return false;
    }

    irq_state_t state;
    state = spin_lock_irqsave(&dwc->chan.chan_lock);

    memcpy(&dwc->dmac_cfg, &dmac_cfg, sizeof(dmac_cfg));

    dwc->len = len;
    dwc->chan.flags = flags;
    dwc->lli_list_ptr = lli_ptr;
    dwc->dmac_cfg.channel_pri = dwc->chan.priority;

    if (DMA_MEMSET == dwc->tr_type) {
        arch_clean_cache_range((addr_t)ap2p(dmac_cfg.src_addr), 1);
    }

    spin_unlock_irqrestore(&dwc->chan.chan_lock, state);

    return true;
}

/**
 * @brief dma memcpy64 config
 *
 * @param[in] chan abstracted dma channel info
 * @param[in] addr src buffer address
 * @param[in] len  data size
 * @param[in] param for differnt transaction type,represent different parameters
 * @param[in] flags  enable/disable interrupt
 * @param[in] type  transaction type
 * @return true/false
 */
static bool dw_dma_config_memcpy64(dma_chan_t *chan, uint64_t addr, size_t len,
                                   uint64_t param, uint32_t flags,
                                   dma_transaction_type_e type)
{
    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    dmac_coeff_t dmac_cfg = {0};
    lli_list_t *lli_ptr = NULL;
    irq_state_t state;
    bool set_int_flag = 0;

    if (0 == addr || 0 == len) {
        ssdk_printf(SSDK_CRIT, "%s %d invalid arguments", __func__, __LINE__);
        ERRCODE_PRINT(MOD_ID_DMA, chan->device->dma_base,
                      DMA_ERR_INVALID_ARGUMENTS, chan->chan_id);
        return false;
    }

    dwc->tr_type = type;
    dmac_cfg.dw_dma_cfg = dwc->dmac_cfg.dw_dma_cfg;

    dwc->dev_cfg.direction = DMA_MEMCPY;

    dw_dma_set_dmac_common_cfg(&dmac_cfg, chan);

            dmac_cfg.src_addr = lp2ap((uint64_t)addr);
            dmac_cfg.dst_addr = lp2ap((uint64_t)param);
            dmac_cfg.src_tr_width =
                __builtin_ctz(addr | param | len | 4);
            dmac_cfg.dst_tr_width = dmac_cfg.src_tr_width;
            dmac_cfg.sinc = DMA_TRANSFER_ADDR_INCREMENT;
            dmac_cfg.dinc = DMA_TRANSFER_ADDR_INCREMENT;
            dmac_cfg.src_msize = DMA_BURST_TR_16ITEMS;
            dmac_cfg.dst_msize = DMA_BURST_TR_16ITEMS;

    if (0 !=
        dw_dma_validate_address64(chan, dmac_cfg.src_addr, dmac_cfg.dst_addr))
        return false;

    set_int_flag = (DMA_INTERRUPT & flags) ? true : false;
    uint8_t int_type = (type == DMA_CYCLIC) ? 1 : 0;

    dw_dmac_set_int_stat(&dmac_cfg, set_int_flag, int_type);

    if(dwc->lli_list_ptr != NULL) {
        dw_dma_free_lli(dwc->lli_list_ptr);
    }

    lli_ptr = dw_dma_set_lli(&dmac_cfg, len, dwc->blk_size, NULL, type,
                             dwc->dev_cfg.direction);

    if (NULL == lli_ptr) {
        return false;
    }

    state = spin_lock_irqsave(&chan->chan_lock);

    memcpy(&dwc->dmac_cfg, &dmac_cfg, sizeof(dmac_cfg));

    dwc->len = len;
    dwc->chan.flags = flags;
    dwc->lli_list_ptr = lli_ptr;
    dwc->dmac_cfg.channel_pri = dwc->chan.priority;

    if (DMA_MEMSET == dwc->tr_type) {
        arch_clean_cache_range((addr_t)ap2p(dmac_cfg.src_addr), 1);
    }

    spin_unlock_irqrestore(&chan->chan_lock, state);

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
static bool dw_dma_prep_memcpy(dma_chan_t *chan, void *dst, void *src,
                               size_t buf_len, uint32_t flags)
{
    return dw_dma_config_transfer(chan, src, buf_len, dst, flags, DMA_MEMCPY);
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
static bool dw_dma_prep_memcpy64(dma_chan_t *chan, uint64_t dst, uint64_t src,
                               size_t buf_len, uint32_t flags)
{
    return dw_dma_config_memcpy64(chan, src, buf_len, dst, flags, DMA_MEMCPY);
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
static bool dw_dma_prep_memset(dma_chan_t *chan, uint8_t val, void *buf_addr,
                               size_t buf_len, uint32_t flags)
{
    return dw_dma_config_transfer(chan, buf_addr, buf_len, &val, flags,
                                  DMA_MEMSET);
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
static bool dw_dma_prep_dev(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                            uint32_t flags)
{
    return dw_dma_config_transfer(chan, buf_addr, buf_len, NULL, flags,
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
static bool dw_dma_prep_cyclic(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                               size_t period_len, uint32_t flags)
{
    return dw_dma_config_transfer(chan, buf_addr, buf_len, &period_len, flags,
                                  DMA_CYCLIC);
}

/**
 * @brief enable channel
 *
 * @param[in] chan requested channel info
 */
static void dw_dma_ch_enable(dma_chan_t *chan)
{
    ASSERT(NULL != chan);

    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    irq_state_t state;

    state = spin_lock_irqsave(&dwc->chan.chan_lock);

    dw_dmac_set_cfg(dwc->dmac_cfg);

    dw_dmac_enable_ch(dwc->dmac_cfg.dma_base, dwc->dmac_cfg.chan_id);

    spin_unlock_irqrestore(&dwc->chan.chan_lock, state);
}

/**
 * @brief disable channel
 *
 * @param[in] chan requested channel info
 */
static bool dw_dma_ch_disable(dma_chan_t *chan)
{
    ASSERT(NULL != chan);
    bool ret = true;
    irq_state_t state;

    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);

    state = spin_lock_irqsave(&dwc->chan.chan_lock);

    ret = dw_dmac_disable_ch(dwc->dmac_cfg);

    spin_unlock_irqrestore(&dwc->chan.chan_lock, state);
    return ret;
}

/**
 * @brief get channel status
 *
 * @param[in] chan requested channel info
 */

static dma_status_e dw_dma_get_chan_status(dma_chan_t *chan)
{
    ASSERT(NULL != chan);

    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);
    dma_status_e status = DMA_INVALID;
    uint32_t ch_intstatus =
        dw_dmac_get_ch_int_stat(dwc->dmac_cfg.dma_base, dwc->dmac_cfg.chan_id);

    dw_dmac_clr_ch_int_stat(dwc->dmac_cfg.dma_base, dwc->dmac_cfg.chan_id,
                            ch_intstatus);
    status = dw_dmac_irq_status_handle(ch_intstatus);

    // printf("ch_intstatus: %#08x, status: %d\n", ch_intstatus, status);
    if (DMA_PAUSED == status || DMA_ERR == status || DMA_COMPLETED == status)
        return status;
    else if (dw_dmac_ch_tr_done(dwc->dmac_cfg.dma_base,
                                dwc->dmac_cfg.chan_id)) {
        return DMA_COMPLETED;
    }
    return DMA_IN_PROGRESS;
}

/**
 * @brief dma sync wait
 *
 * @param[in] chan requested channel info
 * @param[in] timeout limited tick
 */
static dma_status_e dw_dma_sync_wait(dma_chan_t *chan, uint32_t timeout)
{
    ASSERT(NULL != chan);
    dma_status_e status = DMA_INVALID;
    for (int i = 0; i < timeout; i++) {
        status = dw_dma_get_chan_status(chan);
        if (status != DMA_IN_PROGRESS)
            return status;
        osDelay(1);
    }
    return DMA_ERR;
}
/**
 * @brief dw_dma chan free
 *
 * @param[in] chan requested chan info
 */
static void dw_dma_chan_free(dma_chan_t *chan)
{
    ASSERT(NULL != chan);

    dw_dma_chan_t *dwc = to_dw_dma_chan(chan);

    if (dwc->dmac_cfg.dma_base != 0) {
        if (dw_dmac_is_ch_enabled(dwc->dmac_cfg.dma_base,
                                  dwc->dmac_cfg.chan_id)) {
            ssdk_printf(
                SSDK_WARNING,
                "Warnning, DMA[%#x] Channel[%d] is not disable ,but free "
                "channel.\n",
                dwc->dmac_cfg.dma_base, dwc->dmac_cfg.chan_id);
            dw_dmac_disable_ch(dwc->dmac_cfg);
        }
        dw_dmac_ch_int_cfg(dwc->dmac_cfg.dma_base, dwc->dmac_cfg.chan_id, 0);
    }

    chan->allocated = 0;

     if (dwc->lli_list_ptr != NULL) {
        dw_dma_free_lli(dwc->lli_list_ptr);
        dwc->lli_list_ptr = NULL;
    }

}

/**
 * @brief dma ops
 *
 */
static dma_ops_t dma_ops = {
    .init = NULL,
    .request_chan = dw_dma_chan_req,
    .release_chan = dw_dma_chan_free,
    .prep_memcpy = dw_dma_prep_memcpy,
    .prep_memcpy64 = dw_dma_prep_memcpy64,
    .prep_memset = dw_dma_prep_memset,
    .prep_dev = dw_dma_prep_dev,
    .prep_cyclic = dw_dma_prep_cyclic,
    .config_chan = dw_dma_dev_config,
    .enable_chan = dw_dma_ch_enable,
    .disable_chan = dw_dma_ch_disable,
    .sync_wait = dw_dma_sync_wait,
    .deinit = NULL,
};

/**
 * @brief dw_dma init
 *
 * @param[in] dev dma control instance
 * @param[in] cfg dma control config,see dw_dma_cfg_t
 */
void dw_dma_init(dma_dev_t *dev, dw_dma_cfg_t *cfg)
{
    uint32_t chan_size = 0;

    ASSERT(NULL != dev);

    dw_dma_chan_t *dw_chan = NULL;

    dev->ops = &dma_ops;

    dma_init(dev);

    chan_size = sizeof(dw_dma_chan_t) * dev->channel_nr;
    dw_chan = (dw_dma_chan_t *)osAlloc(chan_size);
    ASSERT(NULL != dw_chan);

    memset((void*)dw_chan,0,chan_size);

    spin_lock_init(&lli_list_lock);

    for (int i = 0; i < dev->channel_nr; i++) {
        spin_lock_init(&dw_chan[i].chan.chan_lock);
        dw_chan[i].chan.chan_id = i;
        dw_chan[i].chan.allocated = 0;
        dw_chan[i].chan.device = dev;
        dw_chan[i].chan.priority = dev->priority;
        dw_chan[i].blk_size = DW_DMAC_MAX_BLK_SIZE;
        dw_chan[i].dmac_cfg.dw_dma_cfg = cfg;

        list_add_tail(&dev->channels, &dw_chan[i].chan.node);
        dw_dmac_init_mux(dev->mux_base, i);
    }

    irq_attach(dev->irq_id, dw_dma_irq_handler, dev);
    irq_enable(dev->irq_id);
}

