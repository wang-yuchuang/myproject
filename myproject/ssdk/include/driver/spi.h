/**
 * @file spi.h
 * @brief Common spi driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef _SPI_H_
#define _SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <cmsis_os2.h>
#include <debug.h>
#include <lib/list.h>
#include <spinlock.h>
#include <stdlib.h>
#include <types.h>

/* spi err type */
#define SPI_ERR_INIT -1001
#define SPI_ERR_CFG -1002
#define SPI_ERR_IRQ -1003
#define SPI_ERR_MSG_LEN -1004
#define SPI_ERR_BUF_NULL -1005
#define SPI_ERR_POLL -1006
#define SPI_ERR_KERNEL -1007
#define SPI_ERR_CALLBACK -1008
#define SPI_ERR_DEV_MODE -1009
#define SPI_ERR_DEV_FREQ -1010
#define SPI_ERR_DEV_BPW -1011
#define SPI_ERR_DEV_CS -1012

/* spi mode flags */
#define SPI_CPHA 0x01 /* clock phase */
#define SPI_CPOL 0x02 /* clock polarity */
#define SPI_MODE_0 (0 | 0)
#define SPI_MODE_1 (0 | SPI_CPHA)
#define SPI_MODE_2 (SPI_CPOL | 0)
#define SPI_MODE_3 (SPI_CPOL | SPI_CPHA)
#define SPI_LOOP 0x04    /* spi internal loopback mode, debug purpuse */
#define SPI_CS_WORD 0x08 /* toggle cs after each word */

/* spi operation mode type */
#define SPI_POLL 0x1 /* spi ctrl use polling */
#define SPI_INTP 0x2 /* spi ctrl use interrupt */
#define SPI_DMA  0x3 /* spi ctrl use dma */

#define SPI_BPW_MASK(bits) (1ul << ((bits)-1))
#define SPI_BPW_RANGE_MASK(min, max)                                           \
    (0xffffffff - (1ul << ((min)-1)) + 1) & (0xffffffff >> (32 - 1 - ((max)-1)))

#define SPI_IS_VALID_MSG_LEN(len, bits) !((len) % DIV_ROUND_UP((bits), 8))

struct spi_controller;
struct spi_device;
typedef void (*spi_callback)(int trans_stat, void *cb_param);

struct spi_transfer {
    const void *tx_buf;
    void *rx_buf;
    uint32_t len;
};

struct spi_msg {
    struct spi_transfer *trans_arr;
    int trans_num;   /* cs keep until n trans_num end */
    spi_callback cb; /* callback for async transfer */
    void *cb_param;  /* callback parameter */
    struct spi_device *dev; /* client spi device */
    struct list_node node; /* client ignore and cannot touch */
};

struct spi_controller_ops {
    int (*spi_init)(struct spi_controller *ctrl);
    void (*spi_deinit)(struct spi_controller *ctrl);
    int (*spi_transfer)(struct spi_controller *ctrl, struct spi_msg *msg);
    int (*spi_slv_transfer)(struct spi_controller *ctrl, struct spi_msg *msg);
};

/**
 * @brief spi device configurations.
 */
struct spi_device_config {
    uint32_t mode;
    uint32_t freq_hz;
    uint8_t bits_per_word;
    uint8_t chip_select;
    uint16_t cs_gpio;    /* gpio num for chip select */
    bool cs_gpio_enable; /* enable gpio as chip select */
    bool lsb_first;      /* byte order */
    bool byte_swap;      /* byte swap */
    uint32_t start_delay;/* delay from effective chip select to start of transmission */
    uint32_t end_delay;  /* delay from end of transmission to invalid chip select */
};

/**
 * @brief spi controller basic configurations.
 */
struct spi_controller_config {
    uint8_t id;
    paddr_t base;
    int irq;
    uint32_t mode;
    int opmode; /* master opmode: dma/poll/interrupt */
    int num_cs; /* master max support chipselect */
    bool slave; /* spi controller as slave */
    uint8_t slave_bits_per_word; /* bpw of spi controller as slave */
    uint32_t slave_buf_len; /* transfer buf len of spi controller as slave */
};

/*
 * @brief spi device.
 */
struct spi_device {
    struct spi_device_config dev_cfg;
    struct spi_controller *ctrl;
};

/*
 * @brief spi controller.
 */
struct spi_controller {
    struct spi_controller_config ctrl_cfg;
    const struct spi_controller_ops *ops;
    uint32_t bits_per_word_mask;
    uint32_t min_freq_hz;
    uint32_t max_freq_hz;
    struct list_node msg_list;
    spin_lock_t spin_lock;
    osMutexId_t mutex_lock;
    osThreadId_t spi_thread;
    osEventFlagsId_t thread_event;
    osEventFlagsId_t sync_event;
    int trans_stat;
    int ctrl_stat;
    void *priv;
};

/**
 * @brief spi controller as slave transfer.
 *
 * @param[in] ctrl/spi controller
 * @param[in] txbuf/spi tx buf
 * @param[in] rxbuf/spi rx buf
 * @param[in] len/spi buf len,must equal to slave_buf_len in spi_cfg.c
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
int spi_slave_transfer(struct spi_controller *ctrl, void *txbuf, void *rxbuf,
                       uint32_t len);

/**
 * @brief spi device init.
 *
 * @param[in] dev/spi device
 * @param[in] ctrl/spi controller
 * @param[in] dev_cfg/spi device config
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
int spi_init(struct spi_device *dev, struct spi_controller *ctrl,
             const struct spi_device_config *dev_cfg);

/**
 * @brief spi device deinit.
 *
 * @param[in] dev/spi device
 */
void spi_deinit(struct spi_device *dev);

/**
 * @brief spi master sync transfer with slave
 *
 * @param[in] dev/spi device
 * @param[in] msg/spi msg
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
int spi_transfer_sync(struct spi_device *dev, struct spi_msg *msg);

/**
 * @brief spi master async transfer with slave
 *
 * @param[in] dev/spi device
 * @param[in] msg/spi msg
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
int spi_transfer_async(struct spi_device *dev, struct spi_msg *msg);

/**
 * @brief spi master sync read with slave
 *
 * @param[in] dev/spi device
 * @param[in] buf/spi rx buf
 * @param[in] len/rx buf len
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
static inline int spi_read(struct spi_device *dev, void *buf, uint32_t len)
{
    struct spi_transfer t = {
        .rx_buf = buf,
        .len = len,
    };

    struct spi_msg msg = {
        .trans_arr = &t,
        .trans_num = 1,
    };

    return spi_transfer_sync(dev, &msg);
}

/**
 * @brief spi master sync write with slave
 *
 * @param[in] dev/spi device
 * @param[in] buf/spi tx buf
 * @param[in] len/tx buf len
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
static inline int spi_write(struct spi_device *dev, void *buf, uint32_t len)
{
    struct spi_transfer t = {
        .tx_buf = (const void *)buf,
        .len = len,
    };

    struct spi_msg msg = {
        .trans_arr = &t,
        .trans_num = 1,
    };

    return spi_transfer_sync(dev, &msg);
}

/**
 * @brief spi master sync write then read with slave
 *
 * @param[in] dev/spi device
 * @param[in] txbuf/spi tx buf
 * @param[in] rxbuf/spi rx buf
 * @param[in] txlen/tx buf len
 * @param[in] rxlen/rx buf len
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
static inline int spi_write_then_read(struct spi_device *dev, void *txbuf,
                                      void *rxbuf, uint32_t txlen, uint32_t rxlen)
{
    struct spi_transfer t[2] = {
        {
            .tx_buf = (const void *)txbuf,
            .len = txlen,
        },
        {
            .rx_buf = rxbuf,
            .len = rxlen,
        },
    };

    struct spi_msg msg = {
        .trans_arr = t,
        .trans_num = 2,
    };

    return spi_transfer_sync(dev, &msg);
}

/**
 * @brief spi master parallel write and read with slave
 *
 * @param[in] dev/spi device
 * @param[in] txbuf/spi tx buf
 * @param[in] rxbuf/spi rx buf
 * @param[in] len/tx and rx buf len
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
static inline int spi_parallel_rw(struct spi_device *dev, void *txbuf,
                                  void *rxbuf, uint32_t len)
{
    struct spi_transfer t = {
        .tx_buf = (const void *)txbuf,
        .rx_buf = rxbuf,
        .len = len,
    };

    struct spi_msg msg = {
        .trans_arr = &t,
        .trans_num = 1,
    };

    return spi_transfer_sync(dev, &msg);
}

/**
 * @brief get spi controller instance.
 *
 * @param[in] id spi controller num
 *
 * @retval NULL: fail
 */
struct spi_controller *spi_get_controller(int id);

/**
 * @brief spi controller init.
 *
 * @param[in] spi controller
 * @param[in] spi controller config
 *
 * @retval = 0: success
 * @retval < 0: fail
 */
int spi_controller_init(struct spi_controller *ctrl,
                        const struct spi_controller_config *ctrl_cfg);

/**
 * @brief spi controller init.
 *
 * @param[in] spi controller
 */
void spi_controller_deinit(struct spi_controller *ctrl);

#ifdef __cplusplus
}
#endif

#endif
