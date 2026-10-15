/**
 * @file spi.c
 * @brief spi driver
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <compiler.h>
#include <debug.h>
#include <driver/irq.h>
#include <param.h>
#include <reg.h>
#include <spinlock.h>
#include <string.h>

#include <driver/spi.h>
#include "regs_base.h"
#include <sd_errcode.h>

#define SPI_THREAD_EVENT (1u)
#define SPI_SYNC_EVENT (1u)
#define SPI_CTRL_NUM (32u)

static struct spi_controller *spi_ctrl_register_table[SPI_CTRL_NUM];

int spi_slave_transfer(struct spi_controller *ctrl, void *txbuf, void *rxbuf,
                       uint32_t len)
{
    ASSERT(ctrl);

    struct spi_transfer t = {
        .tx_buf = txbuf,
        .rx_buf = rxbuf,
        .len = len,
    };

    struct spi_msg msg = {
        .trans_arr = &t,
        .trans_num = 1,
    };

    if (ctrl->ctrl_stat)
        return ctrl->ctrl_stat;

    if (!ctrl->ctrl_cfg.slave) {
        ssdk_printf(SSDK_CRIT, "spi%u, not slave mode\n", ctrl->ctrl_cfg.id);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_CONFIG, 1);
        return SPI_ERR_CFG;
    }

    if (ctrl->ctrl_cfg.slave_buf_len != len) {
        printf("spi%u, slv len not match\n", ctrl->ctrl_cfg.id);
        return SPI_ERR_MSG_LEN;
    }

    if (!txbuf && !rxbuf) {
        ssdk_printf(SSDK_ERR, "spi%u, slv buf null %p\n", ctrl->ctrl_cfg.id, txbuf);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_MSGLEN, 1);
        return SPI_ERR_BUF_NULL;
    }

    if (osKernelGetState() < osKernelRunning) {
        ssdk_printf(SSDK_ERR, "spi%u, kernel not running\n", ctrl->ctrl_cfg.id);
        return SPI_ERR_KERNEL;
    }

    ctrl->trans_stat = ctrl->ops->spi_slv_transfer(ctrl, &msg);
    return ctrl->trans_stat;
}

int spi_init(struct spi_device *dev, struct spi_controller *ctrl,
             const struct spi_device_config *dev_cfg)
{
    ASSERT(dev && ctrl && dev_cfg);

    memset(dev, 0, sizeof(struct spi_device));

    if (dev_cfg->mode & (~(ctrl->ctrl_cfg.mode))) {
        ssdk_printf(SSDK_ERR, "slave mode error. slave/master: %u/%u\n", dev_cfg->mode,
                    ctrl->ctrl_cfg.mode);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_INSTANCE, 0);
        return SPI_ERR_DEV_MODE;
    }

    if (dev_cfg->freq_hz < ctrl->min_freq_hz ||
            dev_cfg->freq_hz > ctrl->max_freq_hz) {
        ssdk_printf(SSDK_ERR, "slave freq error. slave/master: %u/%u-%u\n",
                    dev_cfg->freq_hz, ctrl->min_freq_hz, ctrl->max_freq_hz);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_INSTANCE, 1);
        return SPI_ERR_DEV_FREQ;
    }

    if (!(SPI_BPW_MASK(dev_cfg->bits_per_word) & ctrl->bits_per_word_mask)) {
        ssdk_printf(SSDK_ERR, "slave bits_per_word error. master: %u\n",
                    ctrl->bits_per_word_mask);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_INSTANCE, 2);
        return SPI_ERR_DEV_BPW;
    }

    if (dev_cfg->chip_select >= ctrl->ctrl_cfg.num_cs) {
        ssdk_printf(SSDK_ERR, "cs error. slave/master: %d/%d\n", dev_cfg->chip_select,
                    ctrl->ctrl_cfg.num_cs);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_INSTANCE, 3);
        return SPI_ERR_DEV_CS;
    }

    dev->ctrl = ctrl;
    dev->dev_cfg.mode = dev_cfg->mode;
    dev->dev_cfg.freq_hz = dev_cfg->freq_hz;
    dev->dev_cfg.bits_per_word = dev_cfg->bits_per_word;
    dev->dev_cfg.chip_select = dev_cfg->chip_select;
    dev->dev_cfg.cs_gpio = dev_cfg->cs_gpio;
    dev->dev_cfg.cs_gpio_enable = dev_cfg->cs_gpio_enable;

    return 0;
}

void spi_deinit(struct spi_device *dev) { ASSERT(dev); }

static void spi_thread_func(void *arg)
{
    irq_state_t state;
    struct spi_controller *ctrl = arg;

    while (1) {
        osEventFlagsWait(ctrl->thread_event, SPI_THREAD_EVENT, osFlagsWaitAll,
                         osWaitForever);

        while (!list_is_empty(&ctrl->msg_list)) {
            state = spin_lock_irqsave(&ctrl->spin_lock);
            struct spi_msg *cur_msg = list_remove_head_type(&ctrl->msg_list, struct spi_msg,
                                      node);
            spin_unlock_irqrestore(&ctrl->spin_lock, state);

            if (!cur_msg)
                continue;

            ctrl->trans_stat = ctrl->ops->spi_transfer(ctrl, cur_msg);

            if (cur_msg->cb)
                cur_msg->cb(ctrl->trans_stat, cur_msg->cb_param);
            else
                osEventFlagsSet(ctrl->sync_event, SPI_SYNC_EVENT);
        }
    }
}

static int spi_transfer_msg(struct spi_device *dev, struct spi_msg *msg,
                            bool sync_flag)
{
    struct spi_controller *ctrl = dev->ctrl;
    ASSERT(dev && ctrl && msg);

    if (ctrl->ctrl_stat)
        return ctrl->ctrl_stat;

    for (int i = 0; i < msg->trans_num; i++) {
        if (!SPI_IS_VALID_MSG_LEN(msg->trans_arr[i].len, dev->dev_cfg.bits_per_word)) {
            ssdk_printf(SSDK_ERR, "spi%u, trans[%d] len error, len/bpw: %u/%d\n",
                        ctrl->ctrl_cfg.id, i, msg->trans_arr[i].len, dev->dev_cfg.bits_per_word);
            ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_MSGLEN, 0);
            return SPI_ERR_MSG_LEN;
        }

        if (!msg->trans_arr[i].tx_buf && !msg->trans_arr[i].rx_buf) {
            ssdk_printf(SSDK_ERR, "spi%u, trans[%d]:buf null %p\n", ctrl->ctrl_cfg.id, i,
                        msg->trans_arr[i].tx_buf);
            ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_MSGLEN, 1);
            return SPI_ERR_BUF_NULL;
        }
    }

    if (!sync_flag) {
        if (ctrl->ctrl_cfg.opmode == SPI_POLL) {
            ssdk_printf(SSDK_ERR, "spi not support async for poll\n");
            return SPI_ERR_POLL;
        }

        if (osKernelGetState() < osKernelRunning) {
            ssdk_printf(SSDK_ERR, "spi not support async before os started\n");
            return SPI_ERR_KERNEL;
        }

        if (!msg->cb) {
            ssdk_printf(SSDK_ERR, "spi async need callback\n");
            return SPI_ERR_CALLBACK;
        }
    }
    else {
        msg->cb = NULL;
        msg->cb_param = NULL;
    }

    msg->dev = dev;

    if ((ctrl->ctrl_cfg.opmode == SPI_POLL)
            || (osKernelGetState() < osKernelRunning)) {
        ctrl->trans_stat = ctrl->ops->spi_transfer(ctrl, msg);
        return ctrl->trans_stat;
    }

    irq_state_t state = spin_lock_irqsave(&ctrl->spin_lock);
    list_add_tail(&ctrl->msg_list, &msg->node);
    osEventFlagsSet(ctrl->thread_event, SPI_THREAD_EVENT);
    spin_unlock_irqrestore(&ctrl->spin_lock, state);

    if (sync_flag) {
        osEventFlagsWait(ctrl->sync_event, SPI_SYNC_EVENT, osFlagsWaitAll,
                         osWaitForever);
        return ctrl->trans_stat;
    }

    return 0;
}

int spi_transfer_sync(struct spi_device *dev, struct spi_msg *msg)
{
    int ret = 0;

    ASSERT(dev && msg);

    if (dev->ctrl->ctrl_cfg.slave) {
        ssdk_printf(SSDK_CRIT, "spi%u err not master\n", dev->ctrl->ctrl_cfg.id);
        ERRCODE_PRINT(MOD_ID_SPI, dev->ctrl->ctrl_cfg.id, ESPI_CONFIG, 1);
        return SPI_ERR_CFG;
    }

    osStatus_t stat = osMutexAcquire(dev->ctrl->mutex_lock, osWaitForever);
    if (stat == osErrorISR) {
        ssdk_printf(SSDK_CRIT, "spi%u err call in irq cxt\n", dev->ctrl->ctrl_cfg.id);
        ERRCODE_PRINT(MOD_ID_SPI, dev->ctrl->ctrl_cfg.id, ESPI_IRQ, 0);
        return SPI_ERR_IRQ;
    }

    ret = spi_transfer_msg(dev, msg, true);
    osMutexRelease(dev->ctrl->mutex_lock);

    return ret;
}

int spi_transfer_async(struct spi_device *dev, struct spi_msg *msg)
{
    ASSERT(dev && msg);

    if (dev->ctrl->ctrl_cfg.slave) {
        ssdk_printf(SSDK_CRIT, "spi%u err not master\n", dev->ctrl->ctrl_cfg.id);
        ERRCODE_PRINT(MOD_ID_SPI, dev->ctrl->ctrl_cfg.id, ESPI_CONFIG, 1);
        return SPI_ERR_CFG;
    }

    return spi_transfer_msg(dev, msg, false);
}

struct spi_controller *spi_get_controller(int id)
{
    for (int i = 0; i < SPI_CTRL_NUM; i++) {
        if (spi_ctrl_register_table[i]
                && (spi_ctrl_register_table[i]->ctrl_cfg.id == id))
            return spi_ctrl_register_table[i];
    }

    return NULL;
}

int spi_controller_init(struct spi_controller *ctrl,
                        const struct spi_controller_config *config)
{
    char spi_name[20] = {0};
    ASSERT(ctrl && config);
    ctrl->ctrl_cfg.id = config->id;
    ctrl->ctrl_cfg.base = config->base;
    ctrl->ctrl_cfg.irq = config->irq;
    ctrl->ctrl_cfg.mode = config->mode;
    ctrl->ctrl_cfg.opmode = config->opmode;
    ctrl->ctrl_cfg.num_cs = config->num_cs;
    ctrl->ctrl_cfg.slave = config->slave;
    ctrl->ctrl_cfg.slave_bits_per_word = config->slave_bits_per_word;
    ctrl->ctrl_cfg.slave_buf_len = config->slave_buf_len;

    list_initialize(&ctrl->msg_list);
    spin_lock_init(&ctrl->spin_lock);
    ctrl->mutex_lock = osMutexNew(NULL);
    ctrl->thread_event = osEventFlagsNew(NULL);
    ctrl->sync_event = osEventFlagsNew(NULL);

    snprintf(spi_name, sizeof(spi_name), "spi%u_thread", ctrl->ctrl_cfg.id);
    osThreadAttr_t spi_attr = {.name = spi_name, .priority = osPriorityHigh};
    ctrl->spi_thread = osThreadNew(spi_thread_func, ctrl, &spi_attr);

    ctrl->ctrl_stat = ctrl->ops->spi_init(ctrl);
    if (ctrl->ctrl_stat) {
        ssdk_printf(SSDK_CRIT, "spi%u init failed\n", ctrl->ctrl_cfg.id);
        ERRCODE_PRINT(MOD_ID_SPI, ctrl->ctrl_cfg.id, ESPI_CONFIG, 0);
        return ctrl->ctrl_stat;
    }

    for (int i = 0; i < SPI_CTRL_NUM; i++) {
        if (!spi_ctrl_register_table[i]) {
            spi_ctrl_register_table[i] = ctrl;
            break;
        }
    }

    return 0;
}

void spi_controller_deinit(struct spi_controller *ctrl) { ASSERT(ctrl); }

