/**
 * @file goodix.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: goodix vendor tocuh driver
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <board.h>
#include <driver/i2c.h>
#include <driver/taishan/touch/touch_core.h>
#include <driver/taishan/touch/touch_event.h>

#define GTP_I2C_POLLING

#define GOODIX_EVENT_COMPLETE   0x1
#define GOODIX_CONTACT_SIZE     8
#define GOODIX_MAX_CONTACTS     10
#define GOODIX_VENDOR_ID        0x0416
#define GTP_READ_COOR_ADDR      0x814E
#define GOODIX_REG_ID           0x8140

struct goodix_ts_data {
    int id;
    int version;
    int vendor;
    osEventFlagsId_t event;
    osThreadId_t thread;
    const struct touch_device *ts_dev;
};

void ts_mdelay(int ms)
{
    for (int i = 0; i < ms; i++)
        udelay(1000);
}

#ifndef GTP_I2C_POLLING
static int gtp_i2c_read(struct goodix_ts_data *goodix, uint8_t *buf, int len)
{
    int ret = 0;
    ret = i2c_write_read(*goodix->ts_dev->adap, goodix->ts_dev->i2c_addr, buf,
                         2, buf + 2, len - 2);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: read error, ret=%d\n", __func__, ret);
        return ret;
    }

    return 0;
}

static int gtp_i2c_write(struct goodix_ts_data *goodix, uint8_t *buf, int len)
{
    int ret = 0;
    ret = i2c_write(*goodix->ts_dev->adap, goodix->ts_dev->i2c_addr, buf, len);
    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: write error, ret=%d.\n", __func__, ret);
        return ret;
    }

    return ret;
}
#else
int gtp_i2c_read(struct goodix_ts_data *goodix, uint8_t *buf, int len)
{
    int ret;
    struct i2c_msg msgs[2] = {
        {
            .addr = goodix->ts_dev->i2c_addr,
            .buf = buf,
            .len = 2,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
        {
            .addr = goodix->ts_dev->i2c_addr,
            .buf = buf + 2,
            .len = len - 2,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_R,
            .polling = 1,
        },
    };
    ret = i2c_transfer(*goodix->ts_dev->adap, msgs, 2);
    return ret;
}

int gtp_i2c_write(struct goodix_ts_data *goodix, uint8_t *buf, int len)
{
    int ret;
    struct i2c_msg msgs[1] = {
        {
            .addr = goodix->ts_dev->i2c_addr,
            .buf = buf,
            .len = len,
            .addr_flag = I2C_ADDR7B,
            .rw_flag = I2C_M_W,
            .polling = 1,
        },
    };
    ret = i2c_transfer(*goodix->ts_dev->adap, msgs, 1);
    return ret;
}
#endif

static int goodix_read_version(struct goodix_ts_data *goodix)
{
    uint8_t data[8] = {GOODIX_REG_ID >> 8, GOODIX_REG_ID & 0xFF};
    char id_str[5];
    int id, version;

    gtp_i2c_read(goodix, data, 8);

    if (0)
        ssdk_printf(SSDK_ERR, "%s: %#x, %#x, %#x, %#x, %#x, %#x.\n",
                    __func__, data[2], data[3], data[4], data[5], data[6], data[7]);

    memcpy(id_str, data + 2, 4);
    id = atoi(id_str);
    version = data[6] + (data[7] << 8);

    if (!id)
        return -1;
    else {
        goodix->id = id;
        goodix->version = version;
        goodix->vendor = GOODIX_VENDOR_ID;
    }

    if (0)
        ssdk_printf(SSDK_ERR, "%s: id:%#x, version:%#x\n", __func__, id, version);

    return 0;
}

static int goodix_reset_device(struct goodix_ts_data *goodix)
{
    gpio_config(goodix->ts_dev->rst_gpiodev, goodix->ts_dev->rst_pin, GPIO_OUT);
    gpio_config(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin, GPIO_OUT);
    gpio_set(goodix->ts_dev->rst_gpiodev, goodix->ts_dev->rst_pin, 0);
    gpio_set(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin, 0);
    gpio_set(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin,
             (goodix->ts_dev->i2c_addr == 0x14));
    ts_mdelay(10);
    gpio_set(goodix->ts_dev->rst_gpiodev, goodix->ts_dev->rst_pin, 1);
    ts_mdelay(20);
    gpio_set(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin, 0);
    ts_mdelay(70);
    return 0;
}

static int goodix_ts_read_input_report(struct goodix_ts_data *goodix, uint8_t *data)
{
    int count = 0;
    int touch_num = 0;
    uint8_t end_cmd[3] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF, 0};
    /* store first finger point */
    uint8_t point_data[11] = {GTP_READ_COOR_ADDR >> 8, GTP_READ_COOR_ADDR & 0xFF};
    /* store second~tenth finger point, if do have */
    uint8_t point_data1[74] = {GTP_READ_COOR_ADDR >> 8, (GTP_READ_COOR_ADDR + 1 + GOODIX_CONTACT_SIZE) & 0xFF};

    if (!goodix)
        return -1;

    while (count < 10) {
        gtp_i2c_read(goodix, point_data, 11);

        if (point_data[2] & (0x1 << 7)) {
            touch_num = point_data[2] & 0xf;
            memcpy(data, point_data + 2, 9);

            if (touch_num > 1) {
                gtp_i2c_read(goodix, point_data1, 2 + GOODIX_CONTACT_SIZE * (touch_num - 1));
                memcpy(data + 9, point_data1 + 2, GOODIX_CONTACT_SIZE * (touch_num - 1));
            }

            gtp_i2c_write(goodix, end_cmd, 3);
            return touch_num;
        }

        count++;
        osDelay(2);
    }

    gtp_i2c_write(goodix, end_cmd, 3);
    return 0;
}

static void goodix_ts_thread(void *arg)
{
    struct goodix_ts_data *goodix = arg;
    uint8_t pdata[81] = {0,};
    int len, touch_num;
    struct touch_report_data report_data;

    while (1) {
        osEventFlagsWait(goodix->event, GOODIX_EVENT_COMPLETE, osFlagsWaitAll, osWaitForever);
        touch_num = goodix_ts_read_input_report(goodix, pdata);

        if (0)
            ssdk_printf(SSDK_ERR, "%s: 0x%x, (0x%x, 0x%x, 0x%x, 0x%x)\n", __func__,
                        pdata[0], pdata[3], pdata[2], pdata[5], pdata[4]);

        if ((touch_num < 0) || (pdata[0] == 0)) {
            gpio_int_enable(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin, true);
            continue;
        }

        report_data.head_data.key_value = pdata[0] & (1 << 4);
        report_data.head_data.touch_num = touch_num;

        for (int j = 0; j < touch_num; j++) {
            report_data.coord_data[j].id = *((uint8_t *)(pdata + 1 + GOODIX_CONTACT_SIZE * j));
            report_data.coord_data[j].x = *((uint16_t *)(pdata + 2 + GOODIX_CONTACT_SIZE * j));
            report_data.coord_data[j].y = *((uint16_t *)(pdata + 4 + GOODIX_CONTACT_SIZE * j));
            report_data.coord_data[j].w = *((uint16_t *)(pdata + 6 + GOODIX_CONTACT_SIZE * j));
        }

        len = TS_HEAD_METADATA_SIZE + touch_num * TS_COORD_METADATA_SIZE;
        touch_event_report(&report_data, len, goodix->ts_dev->screen_id);
        gpio_int_enable(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin, true);
    }
}

static void goodix_irq_handler(struct gpio_dev *dev, int pin, void *arg)
{
    struct goodix_ts_data *goodix = arg;
    gpio_int_enable(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin, false);
    osEventFlagsSet(goodix->event, GOODIX_EVENT_COMPLETE);
}

static int goodix_config_device(struct goodix_ts_data *goodix)
{
    goodix->event = osEventFlagsNew(NULL);
    goodix->thread = osThreadNew(goodix_ts_thread, goodix, NULL);
    gpio_config(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin,
                GPIO_INTR_FALLING_EDGE);
    gpio_attach_irq(goodix->ts_dev->irq_gpiodev, goodix->ts_dev->irq_pin,
                    goodix_irq_handler, goodix);
    return 0;
}

int goodix_probe_device(const struct touch_device *ts_dev)
{
    int ret;
    struct goodix_ts_data *goodix = NULL;

    goodix = malloc(sizeof(struct goodix_ts_data));
    if (!goodix) {
        ssdk_printf(SSDK_ERR, "%s, alloc mem fail\n", __func__);
        return -1;
    }

    memset(goodix, 0, sizeof(struct goodix_ts_data));
    goodix->ts_dev = ts_dev;
    int retry = 0;

    do {
        goodix_reset_device(goodix);

        if (!goodix_read_version(goodix))
            break;
    } while (++retry < 3);

    if (retry >= 3) {
        ssdk_printf(SSDK_ERR, "%s: read version fail\n", __func__);
        goto err;
    }

    ret = goodix_config_device(goodix);
    if (ret) {
        ssdk_printf(SSDK_ERR, "%s:config device fail\n", __func__);
        goto err;
    }

    ret = touch_event_init_info(&goodix->ts_dev->coord_cfg, goodix->ts_dev->screen_id);
    if (ret) {
        ssdk_printf(SSDK_ERR, "%s:event init fail\n", __func__);
        goto err;
    }

    ssdk_printf(SSDK_ERR, "%s, ======done ok\n", __func__);
    return 0;
err:
    free(goodix);
    return -1;
}

