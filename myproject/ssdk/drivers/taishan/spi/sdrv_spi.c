
#include "driver/taishan/spi/sdrv_spi.h"

#include <driver/spi.h>
#include "spi_lld.h"
#include "taishan_ip_spi.h"

static int32_t ops_spi_controller_init(struct spi_controller *controller)
{
    int ret;
    struct lld_spi_module *bus = (struct lld_spi_module *)controller->priv;
    ssdk_printf(SSDK_DEBUG, "%s bus: %p bus_p: %p \n", __func__, bus,
                bus->priv);
    if (bus) {
        ret = lld_spi_init(bus);
        if (ret) {
            ssdk_printf(SSDK_ERR, "%s error ret:%d\n", __func__, ret);
            return -1;
        }
    } else {
        ssdk_printf(SSDK_ERR, "%s no bus error\n", __func__);
        return -1;
    }
    bus->state |= SPI_STATE_IRQ_ACT;
    controller->min_freq_hz = 1000000;
    controller->max_freq_hz = 134000000;
    controller->bits_per_word_mask = SPI_BPW_RANGE_MASK(4, 32);
    controller->num_chipselect = 4;
    ssdk_printf(SSDK_DEBUG, "%s bus:%p sta:%08x\n", __func__, bus, bus->state);
    return 0;
}

static void ops_spi_controller_deinit(struct spi_controller *controller)
{
    struct lld_spi_module *bus = (struct lld_spi_module *)controller->priv;
    ssdk_printf(SSDK_DEBUG, "%s bus %p  bus_dev:%p bus_p:%p \n", __func__, bus,
                bus->dev_mode, bus->priv);
    if (bus) {
        if (controller->slave) {
            if (bus->dev_mode)
                osFree(bus->dev_mode);
        }
        if (bus->priv)
            osFree(bus->priv);
        osFree(bus);
        controller->priv = NULL;
    } else {
        ssdk_printf(SSDK_ERR, "%s error no bus.\n", __func__);
    }
}

static int32_t swap_params(struct spi_dev_priv *priv, struct spi_device *dev)
{
    if (dev->controller && dev->controller->clk) {
        priv->mode = dev->mode;
        priv->tim_ctrl = FV_SPI_TIM_CTRL_START_DLY(dev->start_delay) |
                         FV_SPI_TIM_CTRL_END_DLY(dev->end_delay) |
                         FV_SPI_TIM_CTRL_FRM_DLY(0);
        uint32_t div = (clk_get_rate(dev->controller->clk) / (dev->freq_hz * 2));
        priv->baud_div = div > 0 ? div - 1 : 0;
        priv->width = dev->bits_per_word;
        priv->nss_idx = dev->chip_select;
        priv->flags = 0;
        if (dev->is_lsb)
            priv->flags = F_DEV_LSB;
        if (dev->byte_swap)
            priv->flags = F_DEV_SWAP;
    } else {
        ssdk_printf(SSDK_ERR, "%s error no controller or clk node.\n",
                    __func__);
        return -1;
    }
    return 0;
}

static int32_t ops_spi_transfer(struct spi_controller *controller,
                                struct spi_msg *msg)
{
    uint8_t div = 0;
    struct lld_spi_module *bus = (struct lld_spi_module *)controller->priv;
    struct lld_spi_device lld_dev;
    struct lld_spi_device *sdev = NULL;
    struct spi_dev_priv dev_param;
    // default is slave mode
    struct spi_dev_priv* device_priv_ptr = bus->dev_mode;

    ssdk_printf(SSDK_DEBUG, "%s bus: %p bus_p: %p  sta:%08x \n", __func__, bus,
                bus->priv, bus->state);
    if (bus) {
        if (!controller->slave) {
            if (msg->dev) {
                lld_dev.priv = &dev_param;
                lld_dev.bus = bus;
                if (swap_params(&dev_param, msg->dev))
                    return -1;
                sdev = &lld_dev;
                // temp master dev
                device_priv_ptr =  &dev_param;
            } else {
                ssdk_printf(SSDK_ERR, "%s msg no dev error \n", __func__);
                return -1;
            }
        }else if(!bus->dev_mode){
            ssdk_printf(SSDK_ERR, "%s slave mode controller no dev parms \n", __func__);
            return -1;
        }

        if(device_priv_ptr->width <= SPI_DATA_WIDTH_BYTE){
            div = 0;
        }else if(device_priv_ptr->width <= SPI_DATA_WIDTH_HALF_WORD){
            div = 1;
        }else{
            div = 2;
        }

        int32_t ret = lld_spi_transmit_receive(
            bus, sdev, (uint32_t *)msg->tx_buf, (uint32_t *)msg->rx_buf,
            msg->len >> div, msg->timeout);
        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "%s error %d sta:%08x\n", __func__, ret,
                        bus->state);
            return -1;
        }
        return 0;
    } else {
        ssdk_printf(SSDK_ERR, "%s error no bus\n", __func__);
        return -1;
    }
}

static int32_t sd_spi_transfer_async(struct spi_controller *controller,
                                     struct spi_msg *msg)
{
    int32_t ret;
    uint8_t div = 0;
    struct lld_spi_module *bus = (struct lld_spi_module *)controller->priv;

    struct lld_spi_device lld_dev;
    struct lld_spi_device *sdev = NULL;
    struct spi_dev_priv dev_param;
    struct spi_bus_priv *priv = bus->priv;
        // default is slave mode
    struct spi_dev_priv* device_priv_ptr = bus->dev_mode;

    ssdk_printf(SSDK_DEBUG, "%s bus: %p bus_p: %p sta:%08x\n", __func__, bus,
                bus->priv, bus->state);
    if (bus) {
        if (bus->state & BUS_BUSY_STATUS_MASK) {
            return 0;
        }
        if (!controller->slave) {
            if (msg->dev) {
                lld_dev.priv = &dev_param;
                lld_dev.bus = bus;
                if (swap_params(&dev_param, msg->dev))
                    return -1;
                sdev = &lld_dev;
                device_priv_ptr = &dev_param;
            } else {
                ssdk_printf(SSDK_ERR, "%s msg no dev error \n", __func__);
                return -1;
            }
        }else if(!bus->dev_mode){
            ssdk_printf(SSDK_ERR, "%s slave mode controller no dev parms \n", __func__);
            return -1;
        }

         if(device_priv_ptr->width <= SPI_DATA_WIDTH_BYTE){
            div = 0;
        }else if(device_priv_ptr->width <= SPI_DATA_WIDTH_HALF_WORD){
            div = 1;
        }else{
            div = 2;
        }

        msg->inited = 1;
        if (priv->flags & (SPI_DMA_TX_ENABLE | SPI_DMA_RX_ENABLE))
            ret =
                lld_spi_transmit_receive_dma(bus, sdev, (uint32_t *)msg->tx_buf,
                                             (uint32_t *)msg->rx_buf, msg->len >> div);
        else
            ret =
                lld_spi_transmit_receive_irq(bus, sdev, (uint32_t *)msg->tx_buf,
                                             (uint32_t *)msg->rx_buf, msg->len >> div);
        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "%s  error %d \n", __func__, ret);
            return -1;
        }
        bus->async.msg = msg;
        return 0;
    } else {
        ssdk_printf(SSDK_ERR, "%s  error no bus.\n", __func__);
        return -1;
    }
}

static void process_msg_list(struct spi_controller *control)
{
    irq_state_t state;
    struct spi_msg *msg;
    if (control) {
        state = spin_lock_irqsave(&control->spin_lock);
        msg = list_peek_head_type(&control->msg, struct spi_msg, list);
        if (msg) {
            sd_spi_transfer_async(control, msg);
            list_delete(&msg->list);
        }
        spin_unlock_irqrestore(&control->spin_lock, state);
    }
}

static int32_t ops_spi_transfer_async(struct spi_controller *controller,
                                      struct spi_msg *msg)
{
    irq_state_t state;

    ASSERT(controller && msg->len);

    state = spin_lock_irqsave(&controller->spin_lock);
    list_add_tail(&controller->msg, &msg->list);
    spin_unlock_irqrestore(&controller->spin_lock, state);
    process_msg_list(controller);
    return 0;
}

static int ops_spi_handle_irq(uint32_t irq, void *arg)
{
    struct spi_msg *msg;
    struct spi_controller *controller = (struct spi_controller *)arg;
    struct lld_spi_module *bus = (struct lld_spi_module *)controller->priv;
    ssdk_printf(SSDK_DEBUG, "%s bus: %p bus_p: %p sta:%08x\n", __func__, bus,
                bus->priv, bus->state);
    msg = (struct spi_msg *)bus->async.msg;
    mid_spi_polling(bus);
    if (!(bus->state & SPI_STATE_BUSY_RX)) {
        msg->rx_remain = 0;
    }
    if (!(bus->state & SPI_STATE_BUSY_TX)) {
        msg->tx_remain = 0;
    }
    if (!(bus->state & BUS_BUSY_STATUS_MASK)) {
        msg->xfer_complete = 1;
        if (msg->cb)
            msg->cb(msg);
        process_msg_list(controller);
    }
    return 0;
}

static struct spi_controller_ops sdrv_spi_ops = {
    .spi_init = ops_spi_controller_init,
    .spi_deinit = ops_spi_controller_deinit,
    .spi_transfer = ops_spi_transfer,
    .spi_transfer_async = ops_spi_transfer_async,
    .spi_handle_irq = ops_spi_handle_irq,
};

int32_t sdrv_spi_controller_probe(struct spi_controller *controller,
                                  const struct spi_config *config,
                                  const struct spi_controller_config *cfg)
{
    ASSERT(controller && cfg);
    ssdk_printf(SSDK_ERR, "%s \n", __func__);

    struct spi_dev_priv *dev_priv = NULL;
    struct spi_bus_priv *bus_priv = osAlloc(sizeof(struct spi_bus_priv));
    struct lld_spi_module *bus = osAlloc(sizeof(struct lld_spi_module));
    if (!bus_priv || !bus) {
        ssdk_printf(SSDK_ERR, "osAlloc error bus_priv:%p bus:%p\n", bus_priv,
                    bus);
        return -1;
    }
    ssdk_printf(SSDK_DEBUG, "bus_priv:%p bus:%p\n", bus_priv, bus);
    bus_priv->idle = config->idle, bus_priv->timeout = config->timeout,
    bus_priv->slave_nss_p = config->slave_nss_pol, bus_priv->dma = config->dmac;
    bus_priv->flags = config->bus_flags;
    if (cfg->slave) {
        bus_priv->flags |= SPI_MODE_SLAVE_ENABLE;
        dev_priv = osAlloc(sizeof(struct spi_dev_priv));
        if (!dev_priv) {
            ssdk_printf(SSDK_ERR, "osAlloc error dev_priv %p\n", dev_priv);
            return -1;
        }
        dev_priv->flags = config->dev_flags;
        dev_priv->tim_ctrl = config->tim_ctrl;
        dev_priv->nss_idx = config->dev_nss_idx;

        dev_priv->mode = cfg->mode & 0x3;
        dev_priv->width = cfg->bits_per_word_in_slave;
        /*slave mast highest sampling rate*/
        dev_priv->baud_div = 0;
    }

    lld_spi_bus_create(bus, cfg->base, cfg->irq, &semidrive_bus_ops);
    lld_spi_bus_set_privdata(bus, bus_priv, dev_priv);
    controller->priv = (void *)bus;
    controller->ops = &sdrv_spi_ops;

    return spi_controller_init(controller, cfg);
}