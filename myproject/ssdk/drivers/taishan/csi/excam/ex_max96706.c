/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */

#include "ex_max96706.h"

#define MAX96706_DEVICE_ID 0x4A
#define MAX96705_DEVICE_ID 0x41

#define MAX96706_DESER_ADDR 0x78
#define MAX96705_SERER_ADDR 0x40
#define MAX96705_ISP_ADDR 0xA8
#define TCA9539_ADDR 0x74

enum max96706_status {
    DESER_IDLE,
    DESER_STREAM_ON,
};

struct max96706_dev {
    struct vdev_device vdev;
    void *i2c_handle;
    uint8_t deser_addr;
    uint8_t serer_addr;
    uint8_t isp_addr;
    uint8_t gpioext_addr;

    uint32_t gpio_pwn;
    uint32_t gpio_gpi;
    uint32_t gpio_poc;

    void *port_handle;
    void *dio_handle;

    osMutexId_t lock;
    uint32_t status;
};

static int max96706_poc_power(struct max96706_dev *deser, int enable);

static int max96706_write_reg(struct max96706_dev *deser, uint8_t reg,
                              uint8_t val)
{
    uint8_t buf[2];
    int ret = 0;

    deser_info("%s: addr=0x%x, reg=0x%x, val=0x%x\n", __func__,
               deser->deser_addr, reg, val);
    buf[0] = reg;
    buf[1] = val;
    ret = i2c_write(deser->i2c_handle, deser->deser_addr, buf, 2);
    if (ret < 0) {
        deser_err("%s: error: reg=0x%x, val=%x\n", __func__, reg, val);
        return ret;
    }

    return ret;
}

static int max96706_read_reg(struct max96706_dev *deser, uint8_t reg,
                             uint8_t *val)
{
    uint8_t buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));
    ret = i2c_write_read(deser->i2c_handle, deser->deser_addr, &reg, 1, buf, 1);
    if (ret < 0) {
        deser_err("%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }
    deser_info("%s: addr=0x%x, reg=0x%x, val=0x%x\n", __func__,
               deser->deser_addr, reg, buf[0]);

    *val = buf[0];
    return 0;
}

static int max96705_write_reg(struct max96706_dev *deser, uint8_t reg,
                              uint8_t val)
{
    uint8_t buf[2];
    int ret = 0;

    deser_info("%s: addr=0x%x, reg=0x%x, val=0x%x\n", __func__,
               deser->serer_addr, reg, val);
    buf[0] = reg;
    buf[1] = val;
    ret = i2c_write(deser->i2c_handle, deser->serer_addr, buf, 2);
    if (ret < 0) {
        deser_err("%s: error: reg=0x%x, val=%x\n", __func__, reg, val);
        return ret;
    }

    return ret;
}

static int max96705_read_reg(struct max96706_dev *deser, uint8_t reg,
                             uint8_t *val)
{
    uint8_t buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));
    ret = i2c_write_read(deser->i2c_handle, deser->serer_addr, &reg, 1, buf, 1);
    if (ret < 0) {
        deser_err("%s: error: read reg=0x%x\n", __func__, reg);
        return ret;
    }
    deser_info("%s: addr=0x%x, reg=0x%x, val=0x%x\n", __func__,
               deser->serer_addr, reg, buf[0]);

    *val = buf[0];
    return 0;
}

static int max96706_check_chip_id(struct max96706_dev *deser)
{
    int ret;
    int i = 0;
    uint8_t chip_id = 0;

    deser_info("%s\n", __func__);
    while (i++ < 30) {
        ret = max96706_read_reg(deser, 0x1E, &chip_id);
        if (ret) {
            deser_err("%s: failed to read chip identifier\n", __func__);
            continue;
        }

        if (chip_id != MAX96706_DEVICE_ID) {
            deser_err("%s: wrong chip identifier, expected 0x%x(max96706), got "
                      "0x%x\n",
                      __func__, MAX96706_DEVICE_ID, chip_id);
            continue;
        } else {
            deser_info(
                "%s: chip identifier, expected 0x%x(max96706), got 0x%x\n",
                __func__, MAX96706_DEVICE_ID, chip_id);
            break;
        }
    }

    return 0;
}

static int max96705_check_chip_id(struct max96706_dev *deser)
{
    int ret;
    int i = 0;
    uint8_t chip_id = 0;

    deser_info("%s\n", __func__);

    while (i++ < 30) {
        ret = max96705_read_reg(deser, 0x1E, &chip_id);
        if (ret) {
            deser_err("%s: failed to read chip identifier\n", __func__);
            continue;
        }

        if (chip_id != MAX96705_DEVICE_ID) {
            deser_err("%s: wrong chip identifier, expected 0x%x(max96705), got "
                      "0x%x\n",
                      __func__, MAX96705_DEVICE_ID, chip_id);
            continue;
        } else {
            deser_info(
                "%s: chip identifier, expected 0x%x(max96705), got 0x%x\n",
                __func__, MAX96705_DEVICE_ID, chip_id);
            return 0;
        }
    }

    return -1;
}

static int max96706_init_setup(struct max96706_dev *deser)
{
    uint8_t val;
    int i, ret;

    val = 0;
    ret = max96706_read_reg(deser, 0x06, &val);
    if (ret < 0) {
        deser_err("max96706 fail to read 0x06=%d\n", ret);
        return ret;
    }

    val |= 0x80;
    max96706_write_reg(deser, 0x06, val);
    usleep_range(1000, 1100);

    //[7]dbl, [2]hven, [1]cxtp
    val = 0;
    max96706_read_reg(deser, 0x07, &val);
    deser_info("%s, reg[0x07]=0x%x\n", __func__, val);

    if (val != 0x86) {
        max96706_write_reg(deser, 0x07, 0x86);
        usleep_range(10000, 11000);
    }

    // invert hsync
    val = 0;
    max96706_read_reg(deser, 0x02, &val);
    val |= 0x80;
    max96706_write_reg(deser, 0x02, val);
    usleep_range(3000, 3100);

    // disable output
    val = 0;
    max96706_read_reg(deser, 0x04, &val);
    deser_info("%s, reg[0x04]=0x%x\n", __func__, val);
    val = 0x47;
    max96706_write_reg(deser, 0x04, val);
    usleep_range(10000, 11000);

    max96706_poc_power(deser, 1);
    usleep_range(100000, 110000);

    // dcs
    val = 0;
    max96706_read_reg(deser, 0x5, &val);
    deser_info("%s, reg[0x05]=0x%x\n", __func__, val);
    val |= 0x40;
    max96706_write_reg(deser, 0x5, val);
    val = 0;
    max96706_read_reg(deser, 0x5, &val);

    // Retry for i2c address comflict in defaul 7b i2c address 0x40.
    for (i = 0; i < 3; i++) {
        ret = max96705_check_chip_id(deser);

        if (ret < 0) {
            deser_crit("%s: times %d not found 96705, ret=%d\n", __func__, i,
                       ret);
            usleep_range(20000, 20002);
        } else {
            deser_err("%s: found 96705, ret=%d\n", __func__, ret);
            break;
        }
    }
    deser_info("%s: check 96705, ret=%d\n", __func__, ret);
    if (ret < 0)
        return ret;

#ifdef MAX96706_MULTI_SERER
    // Change I2C: not required for only one serer.
    usleep_range(1000, 2000);
    max96705_write_reg(deser, 0x00, deser->serer_addr << 1);
    usleep_range(5000, 6000);
    max96705_write_reg(deser, 0x01, deser->deser_addr << 1);
    usleep_range(10000, 11000);
#endif

    if (deser->gpio_gpi) {
        deser_err("%s: error: gpi is not supported\n", __func__);
#ifdef MAX96706_GPI_EN
        max96706_gpi_power(deser, 9, 1);
        usleep_range(100, 200);
        max96706_gpi_power(deser, 9, 0);
        usleep_range(100, 200);
        max96706_gpi_power(deser, 9, 1);
#endif
    } else {
        val = 0;
        max96705_read_reg(deser, 0x0f, &val);
        val |= 0x81;
        max96705_write_reg(deser, 0x0f, val);
        usleep_range(100, 200);
    }

    // enable dbl, hven
    max96705_write_reg(deser, 0x07, 0x84);
    usleep_range(200000, 210000);

    return 0;
}

#ifdef _CONFIG_OS_SAFETY_
#include "tca9539.h"
// poc-gpios = <&cam_gpios 6 0>;
// Power for camera module
static int max96706_poc_power(struct max96706_dev *deser, int enable)
{
    struct tca9539_device *pd;
    int port = 7;

    deser_info("%s: en %d, port %d\n", __func__, enable, port);

    pd = tca9539_init(10, TCA9539_ADDR);
    if (pd == NULL) {
        deser_err("%s: init tca9359 error!\n", __func__);
        return -1;
    }

    pd->ops.output_enable(pd, port);
    thread_sleep(50); // 50 ms

    pd->ops.output_val(pd, port, 0);
    thread_sleep(50);

    if (enable) {
        pd->ops.output_val(pd, port, 1);
        thread_sleep(100);
    }

    tca9539_deinit(pd);
    return 0;
}

extern const domain_res_t g_gpio_res;
extern const domain_res_t g_iomuxc_res;

// pwdn-gpios = <&port4c 19 0>;
// Power for Rx max96706/712
static int max96706_s_power(struct vdev_device *vdev, int enable)
{
    struct max96706_dev *deser;
    bool ioret;
    void *port_handle;
    void *dio_handle = NULL;
    int pin_index = PortConf_PIN_OSPI2_DATA1;
    const Port_PinModeType MODE_GPIO_OSPI2_DATA1 = {
        ((uint32_t)PORT_PAD_POE__DISABLE | PORT_PAD_IS__OUT |
         PORT_PAD_SR__FAST | PORT_PAD_DS__MID1 | PORT_PIN_IN_NO_PULL),
        ((uint32_t)PORT_PIN_MUX_FV__MIN | PORT_PIN_MUX_FIN__MIN |
         PORT_PIN_OUT_PUSHPULL | PORT_PIN_MODE_GPIO),
    };

    deser = (struct max96706_dev *)vdev->priv_data;
    deser_info("%s: enable=%d.\n", __func__, enable);

    /* Port setup */
    if (deser->port_handle != NULL)
        return 0;

    ioret = hal_port_creat_handle(&port_handle, g_iomuxc_res.res_id[0]);
    if (ioret == false) {
        deser_err("%s: port creat failed.\n", __func__);
        return -1;
    }
    deser_info("%s: port_handle=%p, pin=%d\n", __func__, port_handle,
               pin_index);

    ioret =
        hal_port_set_pin_mode(port_handle, pin_index, MODE_GPIO_OSPI2_DATA1);

    hal_port_release_handle(&port_handle);

    ioret = hal_dio_creat_handle(&dio_handle, g_gpio_res.res_id[0]);
    if (ioret == false) {
        deser_err("%s: dio creat failed.\n", __func__);
        return -1;
    }
    deser_info("%s: dio_handle=%p.\n", __func__, dio_handle);

    hal_dio_set_channel_direction(dio_handle, pin_index, DIO_CHANNEL_OUT);
    hal_dio_write_channel(dio_handle, pin_index, 0);
    thread_sleep(10);

    if (enable == 1) {
        hal_dio_write_channel(dio_handle, pin_index, 1);
        thread_sleep(10);
    }

    hal_dio_release_handle(&dio_handle);
    thread_sleep(10);

    deser_info("%s: enable=%d. done\n", __func__, enable);

    return 0;
}
#else

extern gpio_dev_t g_gpio_ap;

static int max96706_poc_power(struct max96706_dev *deser, int enable)
{
    uint32_t pin = deser->gpio_poc; /* GPIO_D36 */
    gpio_dev_t *gpio_dev = &g_gpio_ap;

    deser_info("%s: pin %d, en %d\n", __func__, pin, enable);
    gpio_set(gpio_dev, pin, enable ? true : false);

    return 0;
}

static int max96706_s_power(struct vdev_device *vdev, int enable) { return 0; }
#endif

static int max96706_enum_mbus_fmt(struct vdev_device *vdev,
                                  struct vdev_mbus_framefmt *fme)
{
    struct max96706_dev *deser;

    if ((vdev == NULL) || (fme == NULL)) {
        return -1;
    }

    deser = (struct max96706_dev *)vdev->priv_data;

    if (fme->index > 0) {
        fme->index = 0;
        return -1;
    }

    fme->code = deser->vdev.fmt.code;
    fme->field = deser->vdev.fmt.field;
    fme->colorspace = deser->vdev.fmt.colorspace;
    return 0;
}

static int max96706_enum_frame_size(struct vdev_device *vdev,
                                    struct vdev_frame_size_enum *fse)
{
    struct max96706_dev *deser;

    if (!vdev || !fse) {
        return -1;
    }

    deser = (struct max96706_dev *)vdev->priv_data;

    if (fse->index > 0) {
        fse->index = 0;
        return -1;
    }

    fse->min_width = deser->vdev.fmt.width;
    fse->max_width = deser->vdev.fmt.width;
    fse->min_height = deser->vdev.fmt.height;
    fse->max_height = deser->vdev.fmt.height;

    return 0;
}

static int max96706_enum_frame_interval(struct vdev_device *vdev,
                                        struct vdev_frame_interval_enum *fie)
{
    return 0;
}

static int max96706_get_frame_interval(struct vdev_device *vdev,
                                       struct vdev_fract *fi)
{
    if (!vdev || !fi) {
        return -1;
    }

    *fi = vdev->frame_interval;
    return 0;
}

static int max96706_set_frame_interval(struct vdev_device *vdev,
                                       struct vdev_fract fi)
{
    return 0;
}

static int max96706_get_fmt(struct vdev_device *vdev,
                            struct vdev_mbus_framefmt *fmt)
{
    if (!vdev) {
        return -1;
    }

    *fmt = vdev->fmt;
    return 0;
}

static int max96706_set_fmt(struct vdev_device *vdev,
                            struct vdev_mbus_framefmt fmt)
{
    return 0;
}

static int max96706_s_stream(struct vdev_device *vdev, int enable)
{
    int ret = 0;
    uint8_t val;
    struct max96706_dev *deser;

    if (!vdev) {
        return -1;
    }

    deser = (struct max96706_dev *)vdev->priv_data;
    osMutexAcquire(deser->lock, 0);

    if (enable == 1) {
        if (deser->status == DESER_STREAM_ON) {
            deser_info("%s: already stream on\n", __func__);
            goto exit;
        }

        ret = max96706_init_setup(deser);
        if (ret < 0)
            goto exit;

        usleep_range(1000, 1100);
        max96706_read_reg(deser, 0x04, &val);
        val &= ~(0x40);
        max96706_write_reg(deser, 0x04, val);
        usleep_range(1000, 1100);

        deser->status = DESER_STREAM_ON;

    } else {
        max96706_poc_power(deser, 0);

        max96706_read_reg(deser, 0x04, &val);
        val |= 0x40;
        max96706_write_reg(deser, 0x04, val);
        usleep_range(1000, 1100);
        deser->status = DESER_IDLE;
    }

exit:
    osMutexRelease(deser->lock);
    return ret;
}

static int max96706_deinit(struct vdev_device *vdev)
{
    struct max96706_dev *deser;

    if (!vdev) {
        return -1;
    }

    deser = (struct max96706_dev *)vdev->priv_data;
    osMutexDelete(deser->lock);
    free(deser);

    return 0;
}

static const struct vdev_dev_ops max96706_vdev_ops = {
    .s_power = max96706_s_power,
    .enum_mbus_fmt = max96706_enum_mbus_fmt,
    .enum_frame_size = max96706_enum_frame_size,
    .enum_frame_interval = max96706_enum_frame_interval,
    .g_frame_interval = max96706_get_frame_interval,
    .s_frame_interval = max96706_set_frame_interval,
    .get_fmt = max96706_get_fmt,
    .set_fmt = max96706_set_fmt,
    .s_stream = max96706_s_stream,
    .close = max96706_deinit,
};

struct vdev_device *max96706_init(void *init_data)
{
    int ret;
    struct max96706_dev *deser;
    struct camera_res_cfg *cfg = (struct camera_res_cfg *)init_data;

    deser = malloc(sizeof(struct max96706_dev));
    if (!deser || !cfg || !cfg->i2c_handle)
        return NULL;

    memset(deser, 0, sizeof(struct max96706_dev));
    deser->i2c_handle = cfg->i2c_handle;
    deser->deser_addr = MAX96706_DESER_ADDR;
    deser->serer_addr = MAX96705_SERER_ADDR;
    deser->isp_addr = MAX96705_ISP_ADDR;
    deser->gpioext_addr = TCA9539_ADDR;

    deser->gpio_pwn = cfg->gpio[0];
    deser->gpio_gpi = cfg->gpio[1];
    deser->gpio_poc = cfg->gpio[2];
    deser->vdev.priv_data = (void *)deser;

    max96706_poc_power(deser, 0);
    max96706_s_power(&deser->vdev, 1);
    ret = max96706_check_chip_id(deser);
    if (ret == 0)
        max96706_s_stream(&deser->vdev, 0);
    max96706_s_power(&deser->vdev, 0);
    if (ret < 0)
        goto err;

    deser->lock = osMutexNew(NULL);

    deser->vdev.ep.bus_type = VDEV_MBUS_PARALLEL2;
    deser->vdev.frame_interval.numerator = 1;
    deser->vdev.frame_interval.denominator = 25;
    deser->vdev.fmt.width = 1280;
    deser->vdev.fmt.height = 720;
    deser->vdev.fmt.code = VDEV_MBUS_FMT_UYVY;
    deser->vdev.fmt.field = VDEV_FIELD_NONE;
    deser->vdev.fmt.colorspace = VDEV_COLORSPACE_SRGB;
    deser->vdev.ops = max96706_vdev_ops;

    return &deser->vdev;

err:
    free(deser);
    return NULL;
}
