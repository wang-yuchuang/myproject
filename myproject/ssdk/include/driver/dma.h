/**
 * @file  dma.h
 * @brief dma api header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef __DMA_H__
#define __DMA_H__

#include <cmsis_os2.h>
#include <lib/list.h>
#include <types.h>
#include <spinlock.h>

/**
 * @brief dma status.
 */
typedef enum dma_status {
    DMA_COMPLETED = 0,
    DMA_IN_PROGRESS,
    DMA_PAUSED,
    DMA_ERR,
    DMA_PENDING,
    DMA_INVALID = -1,
} dma_status_e;

/**
 * @brief dma transfer direction.
 */
typedef enum dma_tr_direction {
    DMA_MEM2MEM = 0,
    DMA_MEM2DEV,
    DMA_DEV2MEM,
    DMA_DEV2DEV,
} dma_tr_direction_e;

/**
 * @brief dma transaction type.
 */
typedef enum dma_transaction_type {
    DMA_MEMCPY = 0,
    DMA_MEMSET = 1,
    DMA_DEVICE = 2,
    DMA_CYCLIC = 3,
    DMA_TYPE_END,
} dma_transaction_type_e;

/**
 * @brief dma transfer bus width.
 */
typedef enum dma_dev_bus_width {
    DMA_DEV_BUSWIDTH_UNDEFINED = 0,
    DMA_DEV_BUSWIDTH_1_BYTE = 1,
    DMA_DEV_BUSWIDTH_2_BYTES = 2,
    DMA_DEV_BUSWIDTH_4_BYTES = 4,
    DMA_DEV_BUSWIDTH_8_BYTES = 8,
    DMA_DEV_BUSWIDTH_16_BYTES = 16,
    DMA_DEV_BUSWIDTH_32_BYTES = 32,
    DMA_DEV_BUSWIDTH_64_BYTES = 64,
} dma_dev_bus_width_e;

/**
 * @brief dma transfer flag(enable or disable interrupt).
 */
typedef enum dma_ctrl_flags {
    DMA_INTERRUPT = (1 << 0),
    DMA_PENDING_TIMEOUT = (1 << 1),
} dma_ctrl_flags_t;

/**
 * @brief abstract dma channel for common driver.
 */
typedef struct dma_dev dma_dev_t;
typedef void (*dma_irq_handle)(uint32_t status, uint32_t param, void *context);

typedef struct dma_chan {
    spin_lock_t chan_lock;
    struct list_node node;
    dma_dev_t *device;
    uint8_t priority;
    uint32_t chan_id;
    uint16_t allocated;
    dma_status_e chan_status;
    dma_ctrl_flags_t flags;
    dma_irq_handle irq_cb;
    uint32_t irq_id;
    void *context;
} dma_chan_t;

/**
 * @brief dma transfer config (dev2mem/mem2dev).
 */
typedef struct dma_dev_cfg {
    dma_tr_direction_e direction;
    addr_t src_addr;
    addr_t dst_addr;
    dma_dev_bus_width_e src_addr_width;
    dma_dev_bus_width_e dst_addr_width;
    /* Maximum number of words (in units of the dma_dev_bus_width_e, not bytes)
     * sent in one DMA req.
     */
    uint32_t src_maxburst;
    uint32_t dst_maxburst;
} dma_dev_cfg_t;

/**
 * @brief abstract dma ops.
 */
typedef struct dma_ops {
    /* init the controller. */
    int (*init)(dma_dev_t *dev);
    /* request transfer chan with/without specific channel. */
    dma_chan_t *(*request_chan)(dma_dev_t *dev, uint32_t *id);
    /* free requested chan. */
    void (*release_chan)(dma_chan_t *chan);
    /* get memcpy dma transfer config. */
    bool (*prep_memcpy)(dma_chan_t *chan, void *dst, void *src, size_t buf_len,
                        uint32_t flags);
    /* get memcpy64 dma transfer config. */
    bool (*prep_memcpy64)(dma_chan_t *chan, uint64_t dst, uint64_t src, size_t buf_len,
                        uint32_t flags);
    /* get memset dma transfer config. */
    bool (*prep_memset)(dma_chan_t *chan, uint8_t val, void *buf_addr,
                        size_t buf_len, uint32_t flags);
    /* get mem2dev/dev2mem dma transfer config. */
    bool (*prep_dev)(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                     uint32_t flags);
    /* get mem2dev/dev2mem loop dma transfer config. */
    bool (*prep_cyclic)(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                        size_t period_len, uint32_t flags);
    /* set dma transfer config. */
    void (*config_chan)(dma_chan_t *chan, const dma_dev_cfg_t *cfg);
    /* enable dma ch. */
    void (*enable_chan)(dma_chan_t *chan);
    /* disable dma ch. */
    bool (*disable_chan)(dma_chan_t *chan);
    /*  dma channel cpu polling wait. */
    dma_status_e (*sync_wait)(dma_chan_t *chan, uint32_t timeout);
    /* deint control. */
    void (*deinit)(dma_dev_t *dev);
} dma_ops_t;

/**
 * @brief abstract dma control device.
 */
typedef struct dma_dev {
    osMutexId_t dev_lock;
    paddr_t dma_base;
    paddr_t mux_base;
    uint32_t irq_id;
    uint8_t priority;
    struct list_node channels;
    uint32_t channel_nr;
    dma_ops_t *ops;
    void *priv;
} dma_dev_t;

/**
 * @brief init dma control
 *
 * @param[in] dev dma control instance,for details, see dma_dev_t
 * @return 0 if OK, or a negative error code.
 */
int dma_init(dma_dev_t *dev);

/**
 * @brief deinit dma control
 *
 * @param[in] dev dma control instance,for details, see dma_dev_t
 */
void dma_deinit(dma_dev_t *dev);

/**
 * @brief request channel random
 *
 * @param[in] dev dma control instance,for details,see dma_dev_t
 * @return channel info
 */
dma_chan_t *dma_req_chan(dma_dev_t *dev);

/**
 * @brief request channel by id
 *
 * @param[in] dev dma control instance,for details,see dma_dev_t
 * @param[in] id request specified channel
 * @return channel info
 */
dma_chan_t *dma_req_chan_with_id(dma_dev_t *dev, uint32_t id);

/**
 * @brief free channel
 *
 * @param[in] chan requested channel info
 */
void dma_free_chan(dma_chan_t *chan);

/**
 * @brief dma device config (only for dev2mem or mem2dev)
 *
 * @param[in] chan requested channel info
 * @param[in] cfg  devcie config,for details, see dma_dev_cfg_t
 */
void dma_config_chan(dma_chan_t *chan, const dma_dev_cfg_t *cfg);

/**
 * @brief get mem2mem copy config
 *
 * @param[in] chan requested channel info
 * @param[in] dst  dst address
 * @param[in] src  src address
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_memcpy(dma_chan_t *chan, void *dst, void *src, size_t buf_len,
                     uint32_t flags);

/**
 * @brief get mem2mem copy transfer config
 *
 * @param[in] chan requested channel info
 * @param[in] dst  dst address
 * @param[in] src  src address
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_memcpy64(dma_chan_t *chan, uint64_t dst, uint64_t src, size_t buf_len,
                     uint32_t flags);

/**
 * @brief get mem2mem set config
 *
 * @param[in] chan requested channel info
 * @param[in] val  set value
 * @param[in] buf_addr  dst address
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_memset(dma_chan_t *chan, uint8_t val, void *buf_addr,
                     size_t buf_len, uint32_t flags);

/**
 * @brief get mem2dev or dev2mem transfer config
 *
 * @param[in] chan requested channel info
 * @param[in] buf_addr  dst address or src address (with dma_dev_config api)
 * @param[in] buf_len transfer length
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_dev(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                  uint32_t flags);

/**
 * @brief get mem2dev or dev2mem transfer config (loop)
 *
 * @param[in] chan requested channel info
 * @param[in] buf_addr  dst address or src address (with dma_dev_config api)
 * @param[in] buf_len transfer length
 * @param[in] period_len package data length in a time
 * @param[in] flags enable or disable interrupt,see dma_ctrl_flags_t
 * @return true/false
 */
bool dma_prep_cyclic(dma_chan_t *chan, void *buf_addr, size_t buf_len,
                     size_t period_len, uint32_t flags);

/**
 * @brief start transfer
 *
 * @param[in] chan requested channel info
 */
void dma_start(dma_chan_t *chan);

/**
 * @brief terminate transfer
 *
 * @param[in] chan requested channel info
 * @return true/false
 */
bool dma_stop(dma_chan_t *chan);

/**
 * @brief sync wait dma status
 *
 * @param[in] chan arequested channel info
 * @param[in] timeout wait time out
 * @return dma_status_e (completed in_progress err paused)
 */
dma_status_e dma_sync_wait(dma_chan_t *chan, uint32_t timeout);

/**
 * @brief alloc no-cache buffer for transfer
 *
 * @param[in] size alloc length
 * @return alloced buffer address
 */
void *dma_alloc_memory(size_t size);

/**
 * @brief free alloced buffer
 *
 * @param[in] dma_addr alloced buffer address
 */
void dma_free_memory(void *dma_addr);

/**
 * @brief flush cache for prevent data inconsistency
 *
 * @param[in] dma_addr buffer address
 * @param[in] size  buffer length
 */
void dma_flush_cache(addr_t dma_addr, size_t size);

/**
 * @brief flush invalidate for prevent data inconsistency
 *
 * @param[in] dma_addr  buffer address
 * @param[in] size  buffer length
 */
void dma_invalidate_cache(addr_t dma_addr, size_t size);

#endif
