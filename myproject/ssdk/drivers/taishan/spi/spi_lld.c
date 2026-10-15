#include "spi_lld.h"

#include "arch/cache.h"
#include "../../taishan/dma/sdrv_dmac.h"
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @return {*}
 */
int32_t lld_spi_init(struct lld_spi_module *bus)
{
    int32_t ret = 0;
    bus->state = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_init) {
        ret = bus->bus_ops->spi_init(bus);
        if (!ret) {
            bus->state |= SPI_STATE_INITED;
        } else {
            ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
            ret = -ERR_LOW_LEVEL;
        }
    } else {
        ssdk_printf(SSDK_DEBUG, "%s error no bus or ops \n", __FUNCTION__);
        ret = -ERR_NO_BUS;
    }
    return ret;
}

/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {enum spi_tim_mode} mode
 * @return {*}
 */
int32_t lld_spi_set_clock_polarity_and_phase(struct lld_spi_module *bus,
                                             enum spi_tim_mode mode)
{
    int32_t ret = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_set_clock_polarity_and_phase) {
        if (bus->state & SPI_STATE_INITED) {
            bus->bus_ops->spi_set_clock_polarity_and_phase(bus, mode);
        } else {
            ret = -ERR_NO_INIT;
        }
    } else {
        ret = -ERR_NO_BUS;
    }
    return ret;
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {uint8_t} width
 * @return {*}
 */
int32_t lld_spi_set_data_width(struct lld_spi_module *bus, uint8_t width)
{
    int32_t ret = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_set_data_width) {
        if (bus->state & SPI_STATE_INITED) {
            ret = bus->bus_ops->spi_set_data_width(bus, width);
            if (ret) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ret = -ERR_NO_INIT;
        }
    } else {
        ret = -ERR_NO_BUS;
    }
    return ret;
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {uint32_t} rate
 * @return {*}
 */
int32_t lld_spi_set_baud_rate(struct lld_spi_module *bus, uint32_t rate)
{
    int32_t ret = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_set_baud_rate) {
        if (bus->state & SPI_STATE_INITED) {
            ret = bus->bus_ops->spi_set_baud_rate(bus, rate);
            if (ret) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ret = -ERR_NO_INIT;
        }
    } else {
        ret = -ERR_NO_BUS;
    }

    return ret;
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {enum spi_mode} mode
 * @return {*}
 */
int32_t lld_spi_set_mode(struct lld_spi_module *bus, enum spi_mode mode)
{
    int32_t ret = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_set_mode) {
        if (bus->state & SPI_STATE_INITED) {
            ret = bus->bus_ops->spi_set_mode(bus, mode);
            if (ret) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ret = -ERR_NO_INIT;
        }
    } else {
        ret = -ERR_NO_BUS;
    }
    return ret;
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {enum spi_com_mode} dir
 * @return {*}
 */
int32_t lld_spi_set_direction(struct lld_spi_module *bus, enum spi_com_mode dir)
{
    int32_t ret = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_set_direction) {
        if (bus->state & SPI_STATE_INITED) {
            ret = bus->bus_ops->spi_set_direction(bus, dir);
            if (ret) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ret = -ERR_NO_INIT;
        }
    } else {
        ret = -ERR_NO_BUS;
    }
    return ret;
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {enum spi_ssp_mode} mode
 * @return {*}
 */
int32_t lld_spi_set_ti_or_motorola_mode(struct lld_spi_module *bus,
                                        enum spi_ssp_mode mode)
{
    int32_t ret = 0;
    if (bus && bus->bus_ops && bus->bus_ops->spi_set_ti_or_motorola_mode) {
        if (bus->state & SPI_STATE_INITED) {
            ret = bus->bus_ops->spi_set_ti_or_motorola_mode(bus, mode);
            if (ret) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        }
    } else {
        ret = -ERR_NO_BUS;
    }
    return ret;
}
/**
 * @description:Duplex mode transmit receive
 * @param {lld_spi_device*} dev
 * @param {uint32_t*} ptxdata
 * @param {uint32_t*} prxdata
 * @param {uint16_t} size
 * @param {uint32_t} timeout
 * @return {*}
 */
int32_t lld_spi_transmit_receive(struct lld_spi_module *bus,
                                 struct lld_spi_device *dev, uint32_t *ptxdata,
                                 uint32_t *prxdata, uint16_t size,
                                 uint32_t timeout)
{
    int32_t ret = 0;

    if (dev && dev->bus) {
        bus = dev->bus;
        if (!timeout)
            return -ERR_API_OPS;
    } else {
        timeout = 0;
    }
    if (!bus || !bus->bus_ops || !bus->bus_ops->spi_transmit_receive) {
        ret = -ERR_NO_BUS;
    } else if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & BUS_BUSY_MASK)) {
            if (ptxdata)
                bus->state |= SPI_STATE_BUSY_TX;
            if (prxdata)
                bus->state |= SPI_STATE_BUSY_RX;
            ret = bus->bus_ops->spi_transmit_receive(bus, dev, ptxdata, prxdata,
                                                     size, timeout);
            if (ret < 0) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_BUSY);
            ret = -ERR_BUSY;
        }
    } else {
        ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_NO_INIT);
        ret = -ERR_NO_INIT;
    }
    return ret;
}
/**
 * @description:
 * @param {lld_spi_device*} dev
 * @param {uint32_t} *ptxdata
 * @param {uint32_t} *prxdata
 * @param {uint16_t} size
 * @return {*}
 */
int32_t lld_spi_transmit_receive_irq(struct lld_spi_module *bus,
                                     struct lld_spi_device *dev,
                                     uint32_t *ptxdata, uint32_t *prxdata,
                                     uint16_t size)
{
    int32_t ret = 0;

    if (dev && dev->bus)
        bus = dev->bus;
    if (!bus || !bus->bus_ops || !bus->bus_ops->spi_transmit_receive_irq) {
        ret = -ERR_NO_BUS;
    } else if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & BUS_BUSY_MASK)) {
            if (ptxdata)
                bus->state |= SPI_STATE_IRQ_TX | SPI_STATE_BUSY_TX;
            if (prxdata)
                bus->state |= SPI_STATE_IRQ_RX | SPI_STATE_BUSY_RX;
            ret = bus->bus_ops->spi_transmit_receive_irq(bus, dev, ptxdata,
                                                         prxdata, size);
            if (ret < 0) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
                bus->state &= ~(BUS_BUSY_MASK | BUS_BUSY_IRQ_MASK);
            }
        } else {
            ret = -ERR_BUSY;
        }
    } else {
        ret = -ERR_NO_INIT;
    }
    return ret;
}
int32_t lld_spi_transmit_receive_dma(struct lld_spi_module *bus,
                                     struct lld_spi_device *dev,
                                     uint32_t *ptxdata, uint32_t *prxdata,
                                     uint16_t size)
{
    int32_t ret = 0;

    if (dev && dev->bus)
        bus = dev->bus;
    if (!bus || !bus->bus_ops || !bus->bus_ops->spi_transmit_receive_dma) {
        ret = -ERR_NO_BUS;
    } else if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & BUS_BUSY_MASK)) {
            if (ptxdata)
                bus->state |= SPI_STATE_BUSY_TX | SPI_STATE_DMA_TX;
            if (prxdata)
                bus->state |= SPI_STATE_BUSY_RX | SPI_STATE_DMA_RX;
            ret = bus->bus_ops->spi_transmit_receive_dma(bus, dev, ptxdata,
                                                         prxdata, size);
            if (ret < 0) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
                bus->state &= ~(BUS_BUSY_MASK | BUS_BUSY_DMA_MASK);
            }
            ssdk_printf(SSDK_DEBUG, "%s dma bus%p @%08x len%d\n", __FUNCTION__,
                        bus, bus->async.prxdata.vel, bus->async.len);
        } else {
            ret = -ERR_BUSY;
            ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_BUSY);
        }
    } else {
        ret = -ERR_NO_INIT;
        ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_NO_INIT);
    }
    return ret;
}
/**
 * @description:vector transmission mode transmit receive
 * @param {lld_spi_device*} dev
 * @param {uint32_t*} ptxdata
 * @param {uint32_t*} prxdata
 * @param {uint16_t} size
 * @param {uint32_t} timeout
 * @return {*}
 */
int32_t lld_spi_vector_transmit_receive(struct lld_spi_module *bus,
                                        struct lld_spi_device *dev,
                                        struct lld_spi_async *vector,
                                        uint32_t timeout)
{

    int32_t ret = 0;
    if (dev && dev->bus) {
        bus = dev->bus;
        if (!timeout)
            return -ERR_API_OPS;
    } else {
        timeout = 0;
    }
    if (dev && dev->bus) {
        bus = dev->bus;
    }
    if (!bus || !bus->bus_ops || !bus->bus_ops->spi_vector_transmit_receive) {
        ret = -ERR_NO_BUS;
    } else if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & BUS_BUSY_MASK)) {
            vector->priv = dev;
            vector->expect_len = 0;
            if (vector->ptxdata.vel)
                bus->state |= SPI_STATE_BUSY_TX;
            if (vector->prxdata.vel)
                bus->state |= SPI_STATE_BUSY_RX;
            ret = bus->bus_ops->spi_vector_transmit_receive(bus, dev, vector,
                                             timeout);
            if (ret < 0) {
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_BUSY);
            ret = -ERR_BUSY;
        }
    } else {
        ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_NO_INIT);
        ret = -ERR_NO_INIT;
    }
    return ret;
}
/**
 * @description:vector transmission mode transmit receive
 * @param {lld_spi_device*} dev
 * @param {uint32_t*} ptxdata
 * @param {uint32_t*} prxdata
 * @param {uint16_t} size
 * @param {uint32_t} timeout
 * @return {*}
 */
int32_t lld_spi_vector_transmit_receive_irq(struct lld_spi_module *bus,
                                            struct lld_spi_device *dev,
                                            struct lld_spi_async *vector)
{
    int32_t ret = 0;
    if (dev && dev->bus) {
        bus = dev->bus;
    }
    if (!bus || !bus->bus_ops ||
        !bus->bus_ops->spi_vector_transmit_receive_irq) {
        ret = -ERR_NO_BUS;
    } else if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & BUS_BUSY_MASK)) {
            if (vector->ptxdata.vel)
                bus->state |= SPI_STATE_BUSY_TX | SPI_STATE_IRQ_TX;
            if (vector->prxdata.vel)
                bus->state |= SPI_STATE_BUSY_RX | SPI_STATE_IRQ_RX;
            vector->priv = dev;
            vector->expect_len = 0;
            ret =
                bus->bus_ops->spi_vector_transmit_receive_irq(bus, dev, vector);
            if (ret < 0) {
                bus->state &= ~(BUS_BUSY_MASK | BUS_BUSY_IRQ_MASK);
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            } else {
                /* async mode will check and clean it */
            }
        } else {
            ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_BUSY);
            ret = -ERR_BUSY;
        }
    } else {
        ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_NO_INIT);
        ret = -ERR_NO_INIT;
    }
    return ret;
}
/**
 * @description:vector transmission mode transmit receive
 * @param {lld_spi_device*} dev
 * @param {uint32_t*} ptxdata
 * @param {uint32_t*} prxdata
 * @param {uint16_t} size
 * @param {uint32_t} timeout
 * @return {*}
 */
int32_t lld_spi_vector_transmit_receive_dma(struct lld_spi_module *bus,
                                            struct lld_spi_device *dev,
                                            struct lld_spi_async *vector)
{
    int32_t ret = 0;
    if (dev && dev->bus) {
        bus = dev->bus;
    }
    if (!bus || !bus->bus_ops ||
        !bus->bus_ops->spi_vector_transmit_receive_dma) {
        ret = -ERR_NO_BUS;
    } else if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & BUS_BUSY_MASK)) {
            /* async mode will check and clean it */
            if (vector->ptxdata.vel)
                bus->state |= SPI_STATE_BUSY_TX | SPI_STATE_DMA_TX;
            if (vector->prxdata.vel)
                bus->state |= SPI_STATE_BUSY_RX | SPI_STATE_DMA_RX;
            vector->priv = dev;
            vector->expect_len = 0;
            ret =
                bus->bus_ops->spi_vector_transmit_receive_dma(bus, dev, vector);
            if (ret < 0) {
                bus->state &= ~(BUS_BUSY_MASK | BUS_BUSY_DMA_MASK);
                ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
                ret = -ERR_LOW_LEVEL;
            }
        } else {
            ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_BUSY);
            ret = -ERR_BUSY;
        }
    } else {
        ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_NO_INIT);
        ret = -ERR_NO_INIT;
    }
    return ret;
}
#ifdef CONFIG_DEBUG
static void show_state(struct lld_spi_module *bus)
{
    if (bus) {
        ssdk_printf(SSDK_DEBUG,
                    "bus %p t:%d r:%d l:%d e:%d cur_r:%d n:%p p:%p s:%08x\n",
                    bus, bus->async.tx_cur, bus->async.rx_cur, bus->async.len,
                    bus->async.expect_len, bus->async.cur_remian,
                    bus->async.next, bus->async.priv, bus->state);
    }
}
#endif

void mid_spi_polling(struct lld_spi_module *bus)
{
    int32_t ret = -1;
    int32_t ops_ret;
    uint32_t irq_state;
    struct lld_spi_async *inherit;
    if (!(bus->state & SPI_STATE_INITED)) {
        ssdk_printf(SSDK_ERR, "%s error not inited\n", __FUNCTION__);
        return;
    }
    if (bus->bus_ops && bus->bus_ops->spi_irq_state)
        ret = bus->bus_ops->spi_irq_state(bus, &irq_state);
    if (ret < 0) {
        ssdk_printf(SSDK_DEBUG, "%s error: %d\n", __FUNCTION__, ret);
    } else {
        show_state(bus);
        if (bus->state & BUS_BUSY_DMA_MASK) {
            ret = SPI_TX_FIFO_WRITE | SPI_RX_FIFO_READ;
        }
        if (bus->state & SPI_STATE_BUSY_TX && (ret & SPI_TX_FIFO_WRITE)) {
            if ((bus->async.tx_cur < bus->async.expect_len) &&
                !(bus->state & SPI_STATE_DMA_TX)) {
                /* feed data to hw ,dma mode soft do not need do this */
                spi_write_remain(bus);
            }
            if (bus->async.tx_cur >= bus->async.expect_len) {
                /* tx except satisfy ,If you need to receive,
                    just turn off the send trigger, otherwise all turn off*/
                if (bus->state & SPI_STATE_BUSY_RX)
                    bus->bus_ops->spi_setup_irq_mask(bus, SPI_TX_WRITE_REQ);
                else
                    bus->bus_ops->spi_setup_irq_mask(
                        bus, SPI_TX_WRITE_REQ | SPI_TRASPORT_FINISH);

                if (bus->async.expect_len == bus->async.len) {
                    /* vector item end check next need to transmit ? */
                    if (!bus->async.next) {
                        /*
                            vector end and not have others need to process
                            ,Tx Finished
                        */
                        bus->state &= ~SPI_STATE_DMA_TX;
                        bus->state &= ~SPI_STATE_IRQ_TX;
                        bus->state &= ~SPI_STATE_BUSY_TX;
                        ssdk_printf(SSDK_DEBUG, "%s bus%p tx pass\n",
                                    __FUNCTION__, bus);
                    } else {
                        /* vector next */
                        if (bus->state & SPI_STATE_BUSY_RX) {
                            /* do not start a new transmit, Should be started
                                when the reception is complete */
                        } else {
                            /* inherit from last item dev */
                            inherit = bus->async.next;
                            inherit->priv = bus->async.priv;
                            inherit->expect_len = 0;
                            if (bus->state & BUS_BUSY_DMA_MASK) {
                                ops_ret = bus->bus_ops
                                              ->spi_vector_transmit_receive_dma(
                                                  bus, bus->async.priv,
                                                  bus->async.next);
                                ssdk_printf(SSDK_DEBUG, "ops_ret %d\n",
                                            ops_ret);
                            } else {
                                ops_ret = bus->bus_ops
                                              ->spi_vector_transmit_receive_irq(
                                                  bus, bus->async.priv,
                                                  bus->async.next);
                                ssdk_printf(SSDK_DEBUG, "ops_ret %d\n",
                                            ops_ret);
                            }
                        }
                    }
                } else {
                    /* vector item continue  */
                    if (bus->state & SPI_STATE_BUSY_RX) {
                        /* do not start a new transmit, Should be started
                            when the reception is complete */
                    } else {
                        bus->async.cur_remian =
                            bus->async.len - bus->async.tx_cur;
                        if (bus->state & BUS_BUSY_DMA_MASK) {
                            ops_ret =
                                bus->bus_ops->spi_vector_transmit_receive_dma(
                                    bus, bus->async.priv, &bus->async);
                            ssdk_printf(SSDK_DEBUG, "ops_ret %d\n", ops_ret);
                        } else {
                            ops_ret =
                                bus->bus_ops->spi_vector_transmit_receive_irq(
                                    bus, bus->async.priv, &bus->async);
                            ssdk_printf(SSDK_DEBUG, "ops_ret %d\n", ops_ret);
                        }
                    }
                }
            }
        }
        if (bus->state & SPI_STATE_BUSY_RX && (ret & SPI_RX_FIFO_READ)) {
            /* check next need to transmit ? */
            if ((bus->async.rx_cur < bus->async.expect_len) &&
                !(bus->state & SPI_STATE_DMA_RX)) {
                /* read data to hw ,dma mode do not need to do anything */
                spi_read_remain(bus);
            }
            /* !!! Cannot be implemented using if-else must independent to check
             */
            if (bus->async.rx_cur == bus->async.expect_len) {
                /* rx except satisfy , all turn off*/
                bus->bus_ops->spi_setup_irq_mask(bus, SPI_TX_WRITE_REQ |
                                                          SPI_RX_READ_REQ |
                                                          SPI_TRASPORT_FINISH);
                if (bus->async.expect_len == bus->async.len) {
                    /* vector item end check next need to transmit ? */
                    if (!bus->async.next) {
                        /*
                            vector end and not have others  need to process
                            ,Rx Finished
                        */
                        bus->state &= ~SPI_STATE_DMA_RX;
                        bus->state &= ~SPI_STATE_IRQ_RX;
                        bus->state &= ~SPI_STATE_BUSY_RX;
                        ssdk_printf(SSDK_DEBUG, "%s bus%p rx pass\n",
                                    __FUNCTION__, bus);
                    } else {
                        inherit = bus->async.next;
                        inherit->priv = bus->async.priv;
                        inherit->expect_len = 0;
                        /* vector next */
                        if (bus->state & BUS_BUSY_DMA_MASK) {
                            ops_ret =
                                bus->bus_ops->spi_vector_transmit_receive_dma(
                                    bus, bus->async.priv, bus->async.next);
                            ssdk_printf(SSDK_DEBUG, "ops_ret %d\n", ops_ret);
                        } else {
                            ops_ret =
                                bus->bus_ops->spi_vector_transmit_receive_irq(
                                    bus, bus->async.priv, bus->async.next);
                            ssdk_printf(SSDK_DEBUG, "ops_ret %d\n", ops_ret);
                        }
                    }
                } else {
                    bus->async.cur_remian = bus->async.len - bus->async.rx_cur;
                    /* vector item continue  */
                    if (bus->state & BUS_BUSY_DMA_MASK) {
                        ops_ret = bus->bus_ops->spi_vector_transmit_receive_dma(
                            bus, bus->async.priv, &bus->async);
                        ssdk_printf(SSDK_DEBUG, "ops_ret %d\n", ops_ret);
                    } else {
                        ops_ret = bus->bus_ops->spi_vector_transmit_receive_irq(
                            bus, bus->async.priv, &bus->async);
                        ssdk_printf(SSDK_DEBUG, "ops_ret %d\n", ops_ret);
                    }
                }
            }
        }

        ssdk_printf(SSDK_DEBUG, "irq_sta reg%08x\n", irq_state);
        if (bus->bus_ops->spi_clr_irq_state)
            bus->bus_ops->spi_clr_irq_state(bus, irq_state);
    }
}
/**
 * @description:
 * @param {lld_spi_module} *bus
 * @param {uint32_t*} prxdata
 * @param {uint16_t} size
 * @return {*}
 */
int32_t lld_spi_async_polling(struct lld_spi_module *bus)
{
    int32_t ret = -1;
    if (bus->state & SPI_STATE_INITED) {
        if (!(bus->state & SPI_STATE_IRQ_ACT)) {
            mid_spi_polling(bus);
        }
        ssdk_printf(SSDK_DEBUG, "%s bus%p state=%08x\n", __FUNCTION__, bus,
                    bus->state);
        if (bus->state & BUS_BUSY_STATUS_MASK)
            ret = 1;
        else
            ret = 0;
    } else {
        ret = -ERR_NO_INIT;
        ssdk_printf(SSDK_DEBUG, "%s E:%d\n", __FUNCTION__, -ERR_NO_INIT);
    }
    return ret;
}

int lld_spi_handle_irq(uint32_t irq, void *arg)
{
    struct lld_spi_module *bus = (struct lld_spi_module *)arg;
    ssdk_printf(SSDK_DEBUG, "\n\n%s %p Irq:%08x\n", __FUNCTION__, bus,
                bus->state);
    if (bus->state & SPI_STATE_INITED) {
        mid_spi_polling(bus);
    }
    return 0;
}

int32_t lld_spi_dma_start(struct lld_spi_module *bus) { return 0; }

int32_t lld_spi_dma_pause(struct lld_spi_module *bus) { return 0; }

int32_t lld_spi_dma_resume(struct lld_spi_module *bus) { return 0; }

int32_t lld_spi_dma_stop(struct lld_spi_module *bus) { return 0; }
