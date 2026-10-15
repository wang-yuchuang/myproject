/**
 * @file  dw_dma.h
 * @brief dma hardware api header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 */
#ifndef __DW_DMA_H__
#define __DW_DMA_H__

#include <lib/list.h>
#include <types.h>

#include "driver/dma.h"

/**
 * @brief dmac mux direction
 */
typedef enum {
    DMA_MUX_RD = 0x1, /* RD PERi to MEM -> RX */
    DMA_MUX_WR = 0x2, /* MEM to PERI -> TX */
    DMA_MUX_BOTH = (DMA_MUX_RD | DMA_MUX_WR),
} DMA_MUX_DIR;

/**
 * @brief dmac transfer addr capability
 */
typedef struct dw_dma_addr_range {
    uint64_t base_addr;
    uint64_t len;
} dw_dma_addr_range_t;

/**
 * @brief dmac mux port
 */
typedef struct dw_dma_mux_param {
    uint32_t port;
    uint64_t start_addr;
    uint64_t len;
    DMA_MUX_DIR direct;
} dw_dma_mux_param_t;

/**
 * @brief dmac hardware config
 */
typedef struct dw_dma_cfg {
    dw_dma_addr_range_t *addr_table;
    uint32_t addr_table_len;
    dw_dma_mux_param_t *mux_table;
    uint32_t mux_table_len;
} dw_dma_cfg_t;

/**
 * @brief abstract dmac config.
 */
typedef struct dmac_coeff {
    paddr_t dma_base;             /* dmac base address */
    paddr_t mux_base;             /* dmac_mux base address */
    uint32_t chan_id;             /* choose channel 1~8 */
    uint64_t src_addr;            /* src addr ,64bit for dma */
    uint64_t dst_addr;            /* dst addr ,64bit for dma */
    uint32_t block_transfer_size; /* block transfer size  : the number of data
                                     (length is set by src_tr_width) in a DMA
                                     block transfer (1~n) */
    uint32_t src_transfer_type;   /* src transter
                                     type:single;lli;shadow;reload;continuous */
    uint32_t dst_transfer_type;   /* dst transfer
                                     type:single;lli;shadow;reload;continuous */
    uint32_t src_msize; /* src burst transaction length :number of item writtem
                           to dst for each handshake request */
    uint32_t dst_msize; /* dst burst transaction length :number of item writtem
                           to dst for each handshake request */
    uint32_t src_tr_width; /* dst transfer width : map to axi burst awsize */
    uint32_t dst_tr_width; /* src transfer width : map to axi burst arsize */
    uint32_t dma_transfer_type; /* 1:mem to peripheral;2:peripheral to mem;dma
                                   is controller */
    uint32_t channel_pri;       /* channel priority :0~31 */
    uint32_t hs_number;         /* choose handshake interface :0~143 */
    uint32_t arlen;             /* axi read length */
    uint32_t awlen;             /* axi write length */
    uint32_t arlen_en;          /* axi read length eb */
    uint32_t awlen_en;          /* axi write length eb */
    uint32_t ar_cache;          /* axi read memory type */
    uint32_t aw_cache;          /* axi write memory type */
    uint32_t sinc; /* 0:increase,1:nochange  ---now chip not surpport nochange
                      mode,fix later */
    uint32_t dinc; /* 0:increase,1:nochange */
    uint32_t hs_sel_src; /* 0:hard-hs,1:soft-hs */
    uint32_t hs_sel_dst; /* 0:hard-hs,1:soft-hs */
    uint32_t
        dmac_int_en; /* 1: dmac enable interrupt enable 0:disable interrupt */
    uint32_t dmac_int_mask; /* only use in dmac_irq_evt_handle functions. */
    /* LLI related functions */
    uint64_t llp;                /* first linked list pointer. */
    dw_dma_cfg_t *dw_dma_cfg;    /* dmac hardware config */
} dmac_coeff_t;

/**
 * @brief dmac link list
 */
typedef struct dw_dmac_lli {
    uint64_t sar; /* source address */
    uint64_t dsr; /* dest address */
    uint32_t block_size;
    uint32_t reserved1;
    uint64_t llp; /* linked_list_pointer */
    uint64_t ctl;
    uint32_t sstat_write_back;
    uint32_t dstat_write_back;
    uint64_t llp_status;
    uint64_t reserved2;
} dw_dmac_lli_t __attribute__((__aligned__(64)));

/**
 * @brief abstract link list
 */
typedef struct lli_list {
    dw_dmac_lli_t lli_item;
    uint64_t phy_addr;
    struct list_node lli_node; /* lli_node */
    struct list_node dw_list;  /* lli_list */
    int bitmap_index;
} lli_list_t __attribute__((__aligned__(64)));

/**
 * @brief abstract dw_dma channel
 */
typedef struct dw_dma_chan {
    dma_chan_t chan;
    dma_dev_cfg_t dev_cfg;
    dma_tr_direction_e direction;
    dma_transaction_type_e tr_type;
    uint8_t memset_val;
    uint32_t blk_size;
    uint32_t len;
    lli_list_t *lli_list_ptr;
    dmac_coeff_t dmac_cfg;
} dw_dma_chan_t __attribute__((__aligned__(64)));

/**
 * @brief transfer dma_chan_t to dw_dma_chan_t.
 *
 * @param[in] chan  abstracted dma channel info
 * @return dw_dma_chan_t data point
 */
static inline dw_dma_chan_t *to_dw_dma_chan(dma_chan_t *chan)
{
    return containerof(chan, dw_dma_chan_t, chan);
}

/**
 * @brief dw_dma init
 *
 * @param[in] dev dma control instance
 * @param[in] cfg dma control config,see dw_dma_cfg_t
 */
void dw_dma_init(dma_dev_t *dev, dw_dma_cfg_t *cfg);

#endif