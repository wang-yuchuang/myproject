/*
 * tca9539.c
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
#include "driver/kunlun/gpio/tca9539.h"
#include "driver/gpio.h"

#define TCA9539_REG_INPUT_PORT0     0x0
#define TCA9539_REG_INPUT_PORT1     0x1
#define TCA9539_REG_OUTPUT_PORT0    0x2
#define TCA9539_REG_OUTPUT_PORT1    0x3
#define TCA9539_REG_INVERS_PORT0    0x4
#define TCA9539_REG_INVERS_PORT1    0x5
#define TCA9539_REG_CONFIG_PORT0    0x6
#define TCA9539_REG_CONFIG_PORT1    0x7

#define DEVICE_ADDR  0x74
//0->p00
//1->p01
//2->p02
//3->p03
//4->p04
//5->p05
//6->p06
//7->p07

//8->p10
//9->p11
//10->p12
//11->p13
//12->p14
//13->p15
//14->p16
//15->p17

static int tca9539_write_reg(i2c_adap_dev_t *i2c_dev, uint8_t reg, uint8_t val)
{
    int ret;

    ret = i2c_write_reg8(i2c_dev, DEVICE_ADDR, reg, &val, 1);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: i2c=%d, addr=0x%x, reg=%x, val=%x\n",
                __func__, i2c_dev->id, DEVICE_ADDR, reg, val);
        return ret;
    }

    return 0;
}

static int tca9539_read_reg(i2c_adap_dev_t *i2c_dev, uint8_t reg, uint8_t *val)
{
    int ret;

    ret = i2c_read_reg8(i2c_dev, DEVICE_ADDR, reg, val, 1);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: i2c=%d, addr=0x%x, read reg=%x\n", __func__,
                i2c_dev->id, DEVICE_ADDR, reg);
        return ret;
    }

    return 0;
}

static int tca9539_output_enable(struct gpio_dev *dev,           int pin)
{
    int ret = 0;
    uint8_t reg, val = 0;

    if (pin >= TCA9539_P00 && pin <= TCA9539_P07) {
        reg = TCA9539_REG_CONFIG_PORT0;
    }
    else if (pin >= TCA9539_P10 && pin <= TCA9539_P17) {
        pin -= TCA9539_P10;
        reg = TCA9539_REG_CONFIG_PORT1;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return ret;
    }

    ret = tca9539_read_reg((i2c_adap_dev_t *)dev->dev, reg, &val);
    val &= ~(1 << (pin));
    ret = tca9539_write_reg((i2c_adap_dev_t *)dev->dev, reg, val);

    return ret;
}

static int tca9539_input_enable(struct gpio_dev *dev, int pin)
{
    int ret = 0;
    uint8_t reg, val = 0;

    if (pin >= TCA9539_P00 && pin <= TCA9539_P07) {
        reg = TCA9539_REG_CONFIG_PORT0;
    }
    else if (pin >= TCA9539_P10 && pin <= TCA9539_P17) {
        reg = TCA9539_REG_CONFIG_PORT1;
        pin -= TCA9539_P10;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return ret;
    }

    ret = tca9539_read_reg((i2c_adap_dev_t *)dev->dev, reg, &val);

    val |= 1 << (pin);
    ssdk_printf(SSDK_INFO, "%s: reg:0x%x, val:0x%x\n", __func__, reg, val);
    ret = tca9539_write_reg((i2c_adap_dev_t *)dev->dev, reg, val);

    return ret;
}

static void tca9539_init(struct gpio_dev *dev)
{
    return;
}

static void tca9539_config(struct gpio_dev *dev, unsigned int pin,
                             gpio_type_e type)
{
    int ret;

    if (type == GPIO_OUT) {
        ret = tca9539_output_enable(dev, pin);
    }
    else {
        ret = tca9539_input_enable(dev, pin);
    }

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: ret=%d\n", __func__, ret);
    }

    return;
}

static void tca9539_set_pin(struct gpio_dev *dev, unsigned int pin, bool val)
{
    int ret;
    uint8_t reg, v = 0;

    if (pin >= TCA9539_P00 && pin <= TCA9539_P07) {
        reg = TCA9539_REG_OUTPUT_PORT0;
    }
    else if (pin >= TCA9539_P10 && pin <= TCA9539_P17) {
        reg = TCA9539_REG_OUTPUT_PORT1;
        pin -= TCA9539_P10;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return;
    }

    ret = tca9539_read_reg((i2c_adap_dev_t *)dev->dev, reg, &v);

    if (val == 1)
        v |= 1 << (pin);
    else
        v &= ~(1 << (pin));

    ret = tca9539_write_reg((i2c_adap_dev_t *)dev->dev, reg, v);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: ret=%d\n", __func__, ret);
    }

    return;
}

static bool tca9539_get_pin(struct gpio_dev *dev, unsigned int pin)
{
    bool ret = 0;
    uint8_t reg, val = 0;

    if (pin >= TCA9539_P00 && pin <= TCA9539_P07) {
        reg = TCA9539_REG_INPUT_PORT0;
    }
    else if (pin >= TCA9539_P10 && pin <= TCA9539_P17) {
        reg = TCA9539_REG_INPUT_PORT1;
        pin -= TCA9539_P10;
    }
    else {
        ssdk_printf(SSDK_ERR, "error port index: %d\n", pin);
        return ret;
    }

    tca9539_read_reg((i2c_adap_dev_t *)dev->dev, reg, &val);
    ssdk_printf(SSDK_INFO, "reg=0x%x, val=0x%x, \n", reg, val);
    ret = (val & (1 << (pin))) >> (pin);

    return ret;
}

static void tca9539_deinit(struct gpio_dev *dev)
{
    return;
}

static gpio_ops_t tca9539_ops = {
    .init = tca9539_init,
    .config = tca9539_config,
    .set = tca9539_set_pin,
    .get = tca9539_get_pin,
    .deinit = tca9539_deinit,
};

void tca9539_gpio_init(gpio_dev_t *dev, i2c_adap_dev_t *i2c_dev, int irq_cnt,
                    const uint32_t *irqs)
{
    dev->dev = (void*)i2c_dev;

    gpio_init(dev, 0, 0, &tca9539_ops);
}

