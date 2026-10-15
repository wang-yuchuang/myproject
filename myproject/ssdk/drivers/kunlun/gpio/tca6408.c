/*
 * tca6408.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include "driver/irq.h"
#include "driver/kunlun/gpio/tca6408.h"
#include "driver/gpio.h"

#define TCA6408_REG_INPUT_PORT     0x0
#define TCA6408_REG_OUTPUT_PORT    0x1
#define TCA6408_REG_INVERS_PORT    0x2
#define TCA6408_REG_CONFIG_PORT    0x3

#define DEVICE_ADDR  0x20

/* extern config: address can be re-cfg in board */
tca6408_conf_t g_tca6408_conf = { .addr = DEVICE_ADDR };

static int tca6408_write_reg(i2c_adap_dev_t *i2c_dev,  uint8_t reg, uint8_t val)
{
    int ret;

    ret = i2c_write_reg8(i2c_dev, g_tca6408_conf.addr, reg, &val, 1);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: i2c=%d, addr=0x%x, reg=%x, val=%x\n",
                __func__, i2c_dev->id, g_tca6408_conf.addr, reg, val);
        return ret;
    }

    return 0;
}

static int tca6408_read_reg(i2c_adap_dev_t *i2c_dev, uint8_t reg, uint8_t *val)
{
    int ret;

    ret = i2c_read_reg8(i2c_dev, g_tca6408_conf.addr, reg, val, 1);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: i2c=%d, addr=0x%x, read reg=%x\n", __func__,
                i2c_dev->id, g_tca6408_conf.addr, reg);
        return ret;
    }

    return 0;
}

static int tca6408_output_enable(struct gpio_dev *dev, int pin)
{
    int ret = 0;
    uint8_t reg, val = 0;

    if (pin >= TCA6408_P00 && pin <= TCA6408_P07) {
        reg = TCA6408_REG_CONFIG_PORT;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return ret;
    }

    ret = tca6408_read_reg((i2c_adap_dev_t *)dev->dev, reg, &val);
    ssdk_printf(SSDK_ERR, "[TCA6408] read [0x%02x][0x%02x]\n", reg, val);
    val &= ~(1 << (pin));
    ret = tca6408_write_reg((i2c_adap_dev_t *)dev->dev, reg, val);
    ssdk_printf(SSDK_ERR, "[TCA6408] write [0x%02x][0x%02x]\n", reg, val);

    return ret;
}

static int tca6408_input_enable(struct gpio_dev *dev, int pin)
{
    int ret = 0;
    uint8_t reg, val = 0;

    if (pin >= TCA6408_P00 && pin <= TCA6408_P07) {
        reg = TCA6408_REG_CONFIG_PORT;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return ret;
    }

    ret = tca6408_read_reg((i2c_adap_dev_t *)dev->dev, reg, &val);

    val |= 1 << (pin);
    ssdk_printf(SSDK_INFO, "%s: reg:0x%x, val:0x%x\n", __func__, reg, val);
    ret = tca6408_write_reg((i2c_adap_dev_t *)dev->dev, reg, val);

    return ret;
}

static void tca6408_init(struct gpio_dev *dev)
{
    ssdk_printf(SSDK_ERR, "%s in\n", __func__);
    return;
}

static void tca6408_config(struct gpio_dev *dev, unsigned int pin,
                             gpio_type_e type)
{
    int ret;

    if (type == GPIO_OUT) {
        ret = tca6408_output_enable(dev, pin);
    }
    else {
        ret = tca6408_input_enable(dev, pin);
    }

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: ret=%d\n", __func__, ret);
    }

    return;
}

static void tca6408_set_pin(struct gpio_dev *dev, unsigned int pin, bool val)
{
    int ret;
    uint8_t reg, v = 0;

    if (pin >= TCA6408_P00 && pin <= TCA6408_P07) {
        reg = TCA6408_REG_OUTPUT_PORT;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return;
    }

    ret = tca6408_read_reg((i2c_adap_dev_t *)dev->dev, reg, &v);
    ssdk_printf(SSDK_ERR, "[TCA6408] read [0x%02x][0x%02x]\n", reg, v);

    if (val == 1)
        v |= 1 << (pin);
    else
        v &= ~(1 << (pin));

    ret = tca6408_write_reg((i2c_adap_dev_t *)dev->dev, reg, v);
    ssdk_printf(SSDK_ERR, "[TCA6408] write [0x%02x][0x%02x]\n", reg, v);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: ret=%d\n", __func__, ret);
    }

    return;
}

static bool tca6408_get_pin(struct gpio_dev *dev, unsigned int pin)
{
    bool ret = 0;
    uint8_t reg, val = 0;

    if (pin >= TCA6408_P00 && pin <= TCA6408_P07) {
        reg = TCA6408_REG_INPUT_PORT;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return ret;
    }

    tca6408_read_reg((i2c_adap_dev_t *)dev->dev, reg, &val);
    ssdk_printf(SSDK_INFO, "reg=0x%x, val=0x%x, \n", reg, val);
    ret = (val & (1 << (pin))) >> (pin);

    return ret;
}

static void tca6408_deinit(struct gpio_dev *dev)
{
    return;
}

static gpio_ops_t tca6408_ops = {
    .init = tca6408_init,
    .config = tca6408_config,
    .set = tca6408_set_pin,
    .get = tca6408_get_pin,
    .deinit = tca6408_deinit,
};

void tca6408_gpio_init(gpio_dev_t *dev, i2c_adap_dev_t *i2c_dev, int irq_cnt,
                    const uint32_t *irqs)
{
    dev->dev = (void*)i2c_dev;

    gpio_init(dev, 0, 0, &tca6408_ops);
}

