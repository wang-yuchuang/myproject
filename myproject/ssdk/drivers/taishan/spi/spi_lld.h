#ifndef MLD_SPI_H_
#define MLD_SPI_H_
#include <debug.h>
#include <driver/irq.h>
#include <stdint.h>
#ifndef NULL
#define NULL 0
#endif

enum spi_tim_mode { MODE0 = 0, MODE1, MODE2, MODE3 };
enum spi_mode { MASTER = 0, SLAVE };

enum spi_com_mode { FULL_DUPLEX = 0, HALF_DUPLEX, RX_ONLY, TX_ONLY };

enum spi_ssp_mode { MOTOROLA = 0, TI };

enum spi_nss_sel { NSS_0 = 0, NSS_1, NSS_2, NSS_3, NSS_IO };

enum spi_state {
    SPI_STATE_UNINIT = 0x0,
    SPI_STATE_INITED = 0x1,
    SPI_STATE_BUSY_TX = 0x02,
    SPI_STATE_BUSY_RX = 0x04,
    SPI_STATE_IRQ_ACT = 0x08,
    SPI_STATE_DMA_TX = 0x10,
    SPI_STATE_DMA_RX = 0x20,
    SPI_STATE_IRQ_TX = 0x40,
    SPI_STATE_IRQ_RX = 0x80,
#ifdef CONFIG_VALIDATION
    SPI_STATE_IS_SLAVE = 0x100,
#endif
};

enum ret_err {
    ERR_NONE = 0,
    ERR_NO_INIT = 1,
    ERR_API_OPS,
    ERR_NO_BUS,
    ERR_LOW_LEVEL,
    ERR_BUSY,
};

enum spi_dir {
    SPI_DIR_RX = 0,
    SPI_DIR_TX = 1,
};

enum spi_ops_mode {
    OP_MODE_NORMAL = 0,
    OP_MODE_IRQ,
    OP_MODE_DMA,
};

enum fifo_state {
    SPI_RX_FIFO_READ = (0x01 << 0),
    SPI_TX_FIFO_WRITE = (0x01 << 1)
};

enum irq_type {
    SPI_TRASPORT_FINISH = 0x01 << 0,
    SPI_RX_READ_REQ = 0x01 << 1,
    SPI_TX_WRITE_REQ = 0x01 << 2,
};
#define BUS_BUSY_MASK (SPI_STATE_BUSY_TX | SPI_STATE_BUSY_RX)
#define BUS_BUSY_DMA_MASK (SPI_STATE_DMA_TX | SPI_STATE_DMA_RX)
#define BUS_BUSY_IRQ_MASK (SPI_STATE_IRQ_TX | SPI_STATE_IRQ_RX)
#define BUS_FIFO_STATE_MASK (SPI_RX_FIFO_READ | SPI_TX_FIFO_WRITE)
#define BUS_BUSY_ASYNC_MASK (BUS_BUSY_DMA_MASK | BUS_BUSY_IRQ_MASK)
#define BUS_BUSY_STATUS_MASK (BUS_BUSY_MASK | BUS_BUSY_ASYNC_MASK)
#define BLOCK_TIMEOUT_MAX (0U)

struct lld_spi_module;
struct lld_spi_device;
struct lld_spi_async;

struct lld_spi_ops {
    int32_t (*spi_init)(struct lld_spi_module *bus);
    int32_t (*spi_deinit)(struct lld_spi_module *bus);
    void (*spi_set_clock_polarity_and_phase)(struct lld_spi_module *bus,
                                             enum spi_tim_mode mode);
    int32_t (*spi_set_data_width)(struct lld_spi_module *bus, int width);
    int32_t (*spi_set_baud_rate)(struct lld_spi_module *bus, int rate);
    int32_t (*spi_set_mode)(struct lld_spi_module *bus, enum spi_mode mode);
    int32_t (*spi_set_direction)(struct lld_spi_module *bus,
                                 enum spi_com_mode dir);
    void (*spi_write_data)(struct lld_spi_module  *bus, uint32_t data);
    uint32_t (*spi_read_data)(struct lld_spi_module  *bus);
    uint32_t (*spi_can_write)(struct lld_spi_module  *bus);
    uint32_t (*spi_can_read)(struct lld_spi_module  *bus);
    void (*spi_setup_irq_mask)(struct lld_spi_module *bus, uint32_t type);
    void (*spi_clr_irq_state)(struct lld_spi_module *bus, uint32_t clr);
    int32_t (*spi_set_ti_or_motorola_mode)(struct lld_spi_module *bus,
                                           enum spi_ssp_mode mode);
    int32_t (*spi_set_predev_config)(struct lld_spi_module *bus,
                                     struct lld_spi_device *dev);
    int32_t (*spi_transmit_receive)(struct lld_spi_module *bus,
                                    struct lld_spi_device *dev,
                                    uint32_t *ptxdata, uint32_t *prxdata,
                                    uint16_t size, uint32_t timeout);
    int32_t (*spi_transmit_receive_irq)(struct lld_spi_module *bus,
                                        struct lld_spi_device *dev,
                                        uint32_t *ptxdata, uint32_t *prxdata,
                                        uint16_t size);
    int32_t (*spi_transmit_receive_dma)(struct lld_spi_module *bus,
                                        struct lld_spi_device *dev,
                                        uint32_t *ptxdata, uint32_t *prxdata,
                                        uint16_t size);
    int32_t (*spi_vector_transmit_receive)(struct lld_spi_module *bus,
                                           struct lld_spi_device *dev,
                                           struct lld_spi_async *vector,
                                           uint32_t timeout);
    int32_t (*spi_vector_transmit_receive_irq)(struct lld_spi_module *bus,
                                               struct lld_spi_device *dev,
                                               struct lld_spi_async *vector);
    int32_t (*spi_vector_transmit_receive_dma)(struct lld_spi_module *bus,
                                               struct lld_spi_device *dev,
                                               struct lld_spi_async *vector);
    int32_t (*spi_irq_state)(struct lld_spi_module *bus, uint32_t *irq_status);
};

enum fifo_width {
    SPI_DATA_WIDTH_BYTE = 8,
    SPI_DATA_WIDTH_HALF_WORD = 16,
    SPI_DATA_WIDTH_WORD = 32
};

union spi_data_ptr {
    /* data */
    uint32_t  vel;
    uint8_t  *u8_ptr;
    uint16_t *u16_ptr;
    uint32_t *u32_ptr;
};

struct lld_spi_async {
    /* async mode need this*/
    uint32_t len;
    /* prxdata */
    union spi_data_ptr prxdata;
    /* ptxdata */
    union spi_data_ptr ptxdata;
    /* data_width */
    enum fifo_width width_type;
    /* priv */
    void *priv;
    /* priv for ssdk */
    void *msg;
    /* list to transmit */
    struct lld_spi_async *next;
    /* prxdata */
    uint32_t rx_cur;
    /* ptxdata */
    uint32_t tx_cur;
    /* remain of this item */
    uint32_t cur_remian;
    /* once except of this item */
    uint32_t expect_len;
};

struct lld_spi_module {
    volatile uint32_t base;
    /* irq num */
    volatile uint32_t irq;
    /* status */
    enum spi_state state;
    /* spi common ops*/
    struct lld_spi_ops *bus_ops;
    /* ip special ops and params */
    void *priv;
    /* slave mode params */
    void *dev_mode;
    /* asycn mode need this*/
    struct lld_spi_async async;
};

struct lld_spi_device {
    /* spi bus ops*/
    struct lld_spi_module *bus;
    /* device special ops and params */
    void *priv;
};

/**
 * @description:
 * @param {lld_spi_device} *dev
 * @param {void*} priv
 * @return {*}
 */
static inline void lld_spi_dev_init(struct lld_spi_device *dev, void *priv)
{
    dev->priv = priv;
}
/**
 * @description:
 * @param {lld_spi_device} *dev
 * @param {lld_spi_module*} bus
 * @param {void*} priv
 * @return {*}
 */
static inline void lld_spi_dev_attach_bus(struct lld_spi_device *dev,
                                          struct lld_spi_module *bus)
{
    dev->bus = bus;
    if (bus->bus_ops && bus->bus_ops->spi_set_predev_config)
        bus->bus_ops->spi_set_predev_config(bus, dev);
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {void} *bus_priv
 * @param {void*} dev_mode
 * @return {*}
 */
static inline void lld_spi_bus_set_privdata(struct lld_spi_module *bus,
                                            void *bus_priv, void *dev_mode)
{
    bus->priv = bus_priv;
    bus->dev_mode = dev_mode;
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {uint32_t} base
 * @param {lld_spi_ops*} ops
 * @return {*}
 */
static inline void lld_spi_bus_create(struct lld_spi_module *bus, uint32_t base,
                                      uint32_t irq, struct lld_spi_ops *ops)
{
    bus->base = base;
    bus->bus_ops = ops;
    bus->irq = irq;
}
void mid_spi_polling(struct lld_spi_module *bus);

int lld_spi_handle_irq(uint32_t irq, void *arg);

int32_t lld_spi_init(struct lld_spi_module *bus);

int32_t lld_spi_set_clock_polarity_and_phase(struct lld_spi_module *bus,
                                             enum spi_tim_mode mode);

int32_t lld_spi_set_data_width(struct lld_spi_module *bus, uint8_t width);

int32_t lld_spi_set_baud_rate(struct lld_spi_module *bus, uint32_t rate);

int32_t lld_spi_set_mode(struct lld_spi_module *bus, enum spi_mode mode);

int32_t lld_spi_set_direction(struct lld_spi_module *bus,
                              enum spi_com_mode dir);

int32_t lld_spi_set_ti_or_motorola_mode(struct lld_spi_module *bus,
                                        enum spi_ssp_mode mode);

int32_t lld_spi_transmit_receive(struct lld_spi_module *bus,
                                 struct lld_spi_device *dev, uint32_t *ptxdata,
                                 uint32_t *prxdata, uint16_t size,
                                 uint32_t timeout);

int32_t lld_spi_transmit_receive_dma(struct lld_spi_module *bus,
                                     struct lld_spi_device *dev,
                                     uint32_t *ptxdata, uint32_t *prxdata,
                                     uint16_t size);

int32_t lld_spi_transmit_receive_irq(struct lld_spi_module *bus,
                                     struct lld_spi_device *dev,
                                     uint32_t *ptxdata, uint32_t *prxdata,
                                     uint16_t size);

int32_t lld_spi_vector_transmit_receive(struct lld_spi_module *bus,
                                        struct lld_spi_device *dev,
                                        struct lld_spi_async *vector,
                                        uint32_t timeout);

int32_t lld_spi_vector_transmit_receive_irq(struct lld_spi_module *bus,
                                            struct lld_spi_device *dev,
                                            struct lld_spi_async *vector);

int32_t lld_spi_vector_transmit_receive_dma(struct lld_spi_module *bus,
                                            struct lld_spi_device *dev,
                                            struct lld_spi_async *vector);

int32_t lld_spi_async_polling(struct lld_spi_module *bus);
#ifdef CONFIG_VALIDATION
#include "driver/taishan/vic/sdrv_vic.h"
#endif
static inline void lld_spi_init_irq(struct lld_spi_module *bus)
{
    irq_attach(bus->irq, lld_spi_handle_irq, (void *)bus);
    irq_enable(bus->irq);
#ifdef CONFIG_VALIDATION
    if (bus->state & SPI_STATE_IS_SLAVE)
        irq_lld_set_priority(bus->irq, 2);
#endif
    bus->state |= SPI_STATE_IRQ_ACT;
}

static inline void lld_spi_deinit_irq(struct lld_spi_module *bus)
{
    irq_disable(bus->irq);
    bus->state &= ~SPI_STATE_IRQ_ACT;
}

static inline int32_t
lld_spi_master_transmit_receive(struct lld_spi_device *dev, uint32_t *ptxdata,
                                uint32_t *prxdata, uint16_t size,
                                uint32_t timeout)
{
    return lld_spi_transmit_receive(dev->bus, dev, ptxdata, prxdata, size,
                                    timeout);
}
static inline int32_t lld_spi_master_transmit(struct lld_spi_device *dev,
                                              uint32_t *ptxdata, uint16_t size,
                                              uint32_t timeout)
{
    return lld_spi_transmit_receive(dev->bus, dev, ptxdata, NULL, size,
                                    timeout);
}
static inline int32_t lld_spi_master_receive(struct lld_spi_device *dev,
                                             uint32_t *prxdata, uint16_t size,
                                             uint32_t timeout)
{
    return lld_spi_transmit_receive(dev->bus, dev, NULL, prxdata, size,
                                    timeout);
}

static inline int32_t lld_spi_slave_transmit_receive(struct lld_spi_module *bus,
                                                     uint32_t *ptxdata,
                                                     uint32_t *prxdata,
                                                     uint16_t size)
{
    return lld_spi_transmit_receive(bus, NULL, ptxdata, prxdata, size, 0);
}
static inline int32_t lld_spi_slave_transmit(struct lld_spi_module *bus,
                                             uint32_t *ptxdata, uint16_t size)
{
    return lld_spi_transmit_receive(bus, NULL, ptxdata, NULL, size, 0);
}
static inline int32_t lld_spi_slave_receive(struct lld_spi_module *bus,
                                            uint32_t *prxdata, uint16_t size)
{
    return lld_spi_transmit_receive(bus, NULL, NULL, prxdata, size, 0);
}

static inline int32_t
lld_spi_master_transmit_receive_dma(struct lld_spi_device *dev,
                                    uint32_t *ptxdata, uint32_t *prxdata,
                                    uint16_t size)
{
    return lld_spi_transmit_receive_dma(dev->bus, dev, ptxdata, prxdata, size);
}
static inline int32_t lld_spi_master_transmit_dma(struct lld_spi_device *dev,
                                                  uint32_t *ptxdata,
                                                  uint16_t size)
{
    return lld_spi_transmit_receive_dma(dev->bus, dev, ptxdata, NULL, size);
}
static inline int32_t lld_spi_master_receive_dma(struct lld_spi_device *dev,
                                                 uint32_t *prxdata,
                                                 uint16_t size)
{
    return lld_spi_transmit_receive_dma(dev->bus, dev, NULL, prxdata, size);
}

static inline int32_t
lld_spi_slave_transmit_receive_dma(struct lld_spi_module *bus,
                                   uint32_t *ptxdata, uint32_t *prxdata,
                                   uint16_t size)
{
    return lld_spi_transmit_receive_dma(bus, NULL, ptxdata, prxdata, size);
}
static inline int32_t lld_spi_slave_transmit_dma(struct lld_spi_module *bus,
                                                 uint32_t *ptxdata,
                                                 uint16_t size)
{
    return lld_spi_transmit_receive_dma(bus, NULL, ptxdata, NULL, size);
}
static inline int32_t lld_spi_slave_receive_dma(struct lld_spi_module *bus,
                                                uint32_t *prxdata,
                                                uint16_t size)
{
    return lld_spi_transmit_receive_dma(bus, NULL, NULL, prxdata, size);
}

static inline int32_t
lld_spi_master_transmit_receive_irq(struct lld_spi_device *dev,
                                    uint32_t *ptxdata, uint32_t *prxdata,
                                    uint16_t size)
{
    return lld_spi_transmit_receive_irq(dev->bus, dev, ptxdata, prxdata, size);
}
static inline int32_t lld_spi_master_transmit_irq(struct lld_spi_device *dev,
                                                  uint32_t *ptxdata,
                                                  uint16_t size)
{
    return lld_spi_transmit_receive_irq(dev->bus, dev, ptxdata, NULL, size);
}
static inline int32_t lld_spi_master_receive_irq(struct lld_spi_device *dev,
                                                 uint32_t *prxdata,
                                                 uint16_t size)
{
    return lld_spi_transmit_receive_irq(dev->bus, dev, NULL, prxdata, size);
}

static inline int32_t
lld_spi_slave_transmit_receive_irq(struct lld_spi_module *bus,
                                   uint32_t *ptxdata, uint32_t *prxdata,
                                   uint16_t size)
{
    return lld_spi_transmit_receive_irq(bus, NULL, ptxdata, prxdata, size);
}
static inline int32_t lld_spi_slave_transmit_irq(struct lld_spi_module *bus,
                                                 uint32_t *ptxdata,
                                                 uint16_t size)
{
    return lld_spi_transmit_receive_irq(bus, NULL, ptxdata, NULL, size);
}
static inline int32_t lld_spi_slave_receive_irq(struct lld_spi_module *bus,
                                                uint32_t *prxdata,
                                                uint16_t size)
{
    return lld_spi_transmit_receive_irq(bus, NULL, NULL, prxdata, size);
}

static inline int32_t spi_write_remain(struct lld_spi_module *bus)
{
    uint32_t remain;
    uint32_t len = 0;

    if (!bus)
        return -1;

    if (!bus->async.ptxdata.vel)
        return 0;

    remain = bus->async.expect_len -  bus->async.tx_cur;

    if (remain <= 0)
        return 0;

    if (bus->bus_ops->spi_can_write && bus->bus_ops->spi_write_data) {
        while (remain && bus->bus_ops->spi_can_write(bus)) {
            if (bus->async.width_type == SPI_DATA_WIDTH_BYTE) {
                bus->bus_ops->spi_write_data(bus, *(bus->async.ptxdata.u8_ptr + bus->async.tx_cur));
            } else if (bus->async.width_type == SPI_DATA_WIDTH_HALF_WORD) {
                bus->bus_ops->spi_write_data(bus, *(bus->async.ptxdata.u16_ptr + bus->async.tx_cur));
            } else {
                bus->bus_ops->spi_write_data(bus, *(bus->async.ptxdata.u32_ptr + bus->async.tx_cur));
            }

            bus->async.tx_cur++;
            len++;
            remain--;
        }
    } else {
        return -1;
    }

    return len;
}

static inline int32_t spi_read_remain(struct lld_spi_module *bus)
{
    uint32_t remain;
    uint32_t len = 0;

    if (!bus)
        return -1;

    if (!bus->async.prxdata.vel)
        return 0;

    remain = bus->async.expect_len -  bus->async.rx_cur;

    if ( remain <= 0)
        return 0;

    if (bus->bus_ops->spi_read_data && bus->bus_ops->spi_can_read) {
        while (remain && bus->bus_ops->spi_can_read(bus)) {
            if (bus->async.width_type == SPI_DATA_WIDTH_BYTE) {
                *(bus->async.prxdata.u8_ptr + bus->async.rx_cur) = bus->bus_ops->spi_read_data(bus);
            } else if (bus->async.width_type == SPI_DATA_WIDTH_HALF_WORD) {
                *(bus->async.prxdata.u16_ptr + bus->async.rx_cur) = bus->bus_ops->spi_read_data(bus);
            } else {
                *(bus->async.prxdata.u32_ptr + bus->async.rx_cur) = bus->bus_ops->spi_read_data(bus);
            }

            bus->async.rx_cur++;
            len++;
            remain--;
        }
    } else {
        return -1;
    }
    return len;
}

static inline void sync_finished_callback(struct lld_spi_module *bus)
{
    bus->state &= ~(SPI_STATE_BUSY_TX | SPI_STATE_BUSY_RX);
}
#endif
