/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */

#include "ex_ti926.h"

#define TI926_DEVICE_ID 0x58
#define TI926_DESER_ADDR 0x2C

enum ti926_status {
    DESER_IDLE,
    DESER_STREAM_ON,
};

struct ti926_dev {
    struct vdev_device vdev;
    void *i2c_handle;
    uint8_t deser_addr;

    uint32_t gpio_pwn;
    uint32_t gpio_intn;

    osMutexId_t lock;
    uint32_t status;
};

int test_pattern = 0;

static int ti926_write_reg(struct ti926_dev *deser, uint8_t reg, uint8_t val)
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

static int ti926_read_reg(struct ti926_dev *deser, uint8_t reg, uint8_t *val)
{
    uint8_t buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));
    ret = i2c_write_read(deser->i2c_handle, deser->deser_addr, &reg, 1, buf, 1);
    if (ret < 0) {
        deser_err("%s: error: read reg=0x%02x\n", __func__, reg);
        return ret;
    }

    *val = buf[0];
    return 0;
}

uint8_t reg_list[] = {
    0x3A, 0x41, 0x44, 0x56, 0x64, 0x65, 0x66,
    0x67, 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5,
};

static int read_all_reg(struct ti926_dev *deser)
{
    int cnt, j;
    uint8_t reg, val;
    for (reg = 0x00; reg <= 0x17; reg++) {
        ti926_read_reg(deser, reg, &val);
        printf("ti926 [0x%02x]=0x%02x\n", reg, val);
    }

    for (reg = 0x1c; reg <= 0x2c; reg++) {
        ti926_read_reg(deser, reg, &val);
        printf("ti926 [0x%02x]=0x%02x\n", reg, val);
    }

    cnt = sizeof(reg_list);
    for (j = 0; j < cnt; j++) {
        reg = reg_list[j];
        ti926_read_reg(deser, reg, &val);
        printf("ti926 [0x%02x]=0x%02x\n", reg, val);
    }
    return 0;
}

static int ti926_check_chip_id(struct ti926_dev *deser)
{
    int ret;
    int i = 0;
    uint8_t chip_id = 0;

    deser_info("%s\n", __func__);
    while (i++ < 30) {
        ret = ti926_read_reg(deser, 0x00, &chip_id);
        if (ret) {
            deser_err("%s: failed to read chip identifier\n", __func__);
            continue;
        }

        if (chip_id != TI926_DEVICE_ID) {
            deser_err("%s: wrong chip identifier, expected 0x%x(ti926), got "
                      "0x%x\n",
                      __func__, TI926_DEVICE_ID, chip_id);
            ret = -1;
            continue;
        } else {
            deser_info("%s: chip identifier, expected 0x%x(ti926), got 0x%x\n",
                       __func__, TI926_DEVICE_ID, chip_id);
            break;
        }
    }
    if (ret == 0)
        read_all_reg(deser);

    return ret;
}

static int ti926_init_setup(struct ti926_dev *deser)
{
    uint8_t val;
    int ret;

    ret = ti926_read_reg(deser, 0x01, &val);
    if (ret < 0) {
        deser_err("ti926 fail to read 0x01, ret=%d\n", ret);
        return ret;
    }
    deser_err("ti926 read [0x01]=0x%02x\n", val);

    if (test_pattern) {
        // 1280x480 test pattern
        ti926_write_reg(deser, 0x66, 0x0e); // config,b'0000|[Ve|He|Vp|Hp]
        ti926_write_reg(deser, 0x67, 0x02);
        ti926_write_reg(deser, 0x66, 0x03); // clock divider
        ti926_write_reg(deser, 0x67, 0x03);
        ti926_write_reg(deser, 0x66, 0x04); // HT 8least
        ti926_write_reg(deser, 0x67, 0x54);
        ti926_write_reg(deser, 0x66, 0x05); // VT 4least|HT 4most
        ti926_write_reg(deser, 0x67, 0xb5);
        ti926_write_reg(deser, 0x66, 0x06); // VT 8most
        ti926_write_reg(deser, 0x67, 0x20);
        ti926_write_reg(deser, 0x66, 0x07); // H 8least
        ti926_write_reg(deser, 0x67, 0x00);
        ti926_write_reg(deser, 0x66, 0x08); // V 4least|H 4most
        ti926_write_reg(deser, 0x67, 0x05);
        ti926_write_reg(deser, 0x66, 0x09); // V 8most
        ti926_write_reg(deser, 0x67, 0x1e);
        ti926_write_reg(deser, 0x66, 0x0a); // HS
        ti926_write_reg(deser, 0x67, 0x1c);
        ti926_write_reg(deser, 0x66, 0x0b); // VS
        ti926_write_reg(deser, 0x67, 0x0d);
        ti926_write_reg(deser, 0x66, 0x0c); // HBP
        ti926_write_reg(deser, 0x67, 0x38);
        ti926_write_reg(deser, 0x66, 0x0d); // VBP
        ti926_write_reg(deser, 0x67, 0x1c);
        ti926_write_reg(deser, 0x65, 0x04);
        ti926_write_reg(deser, 0x64, 0x11);
    } else {
        ti926_write_reg(deser, 0x64, 0x10);
        ti926_write_reg(deser, 0x29, 0xA0);
    }

    deser_info("ti926 init setup done\n");

    read_all_reg(deser);

    return 0;
}

extern gpio_dev_t g_gpio_safety;

static int ti926_s_power(struct vdev_device *vdev, int enable)
{
    struct ti926_dev *deser;
    uint32_t pin;
    gpio_dev_t *gpio_dev = &g_gpio_safety;

    if (!vdev) {
        return -1;
    }
    deser = (struct ti926_dev *)vdev->priv_data;
    pin = deser->gpio_pwn; /* GPIO_C14 */

    deser_info("%s: pin %d,  enable %d\n", __func__, pin, enable);
    gpio_set(gpio_dev, pin, enable ? true : false);
    usleep_range(10000, 11000);
    return 0;
}

static int ti926_enum_mbus_fmt(struct vdev_device *vdev,
                               struct vdev_mbus_framefmt *fme)
{
    struct ti926_dev *deser;

    if ((vdev == NULL) || (fme == NULL)) {
        return -1;
    }

    deser = (struct ti926_dev *)vdev->priv_data;

    if (fme->index > 0) {
        fme->index = 0;
        return -1;
    }

    fme->code = deser->vdev.fmt.code;
    fme->field = deser->vdev.fmt.field;
    fme->colorspace = deser->vdev.fmt.colorspace;
    return 0;
}

static int ti926_enum_frame_size(struct vdev_device *vdev,
                                 struct vdev_frame_size_enum *fse)
{
    struct ti926_dev *deser;

    if (!vdev || !fse) {
        return -1;
    }

    deser = (struct ti926_dev *)vdev->priv_data;

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

static int ti926_enum_frame_interval(struct vdev_device *vdev,
                                     struct vdev_frame_interval_enum *fie)
{
    return 0;
}

static int ti926_get_frame_interval(struct vdev_device *vdev,
                                    struct vdev_fract *fi)
{
    if (!vdev || !fi) {
        return -1;
    }

    *fi = vdev->frame_interval;
    return 0;
}

static int ti926_set_frame_interval(struct vdev_device *vdev,
                                    struct vdev_fract fi)
{
    return 0;
}

static int ti926_get_fmt(struct vdev_device *vdev,
                         struct vdev_mbus_framefmt *fmt)
{
    if (!vdev) {
        return -1;
    }

    *fmt = vdev->fmt;
    return 0;
}

static int ti926_set_fmt(struct vdev_device *vdev,
                         struct vdev_mbus_framefmt fmt)
{
    return 0;
}

static int ti926_s_stream(struct vdev_device *vdev, int enable)
{
    int ret = 0;
    struct ti926_dev *deser;

    if (!vdev) {
        return -1;
    }

    deser = (struct ti926_dev *)vdev->priv_data;
    osMutexAcquire(deser->lock, 0);

    if (enable == 1) {
        if (deser->status == DESER_STREAM_ON) {
            deser_info("%s: already stream on\n", __func__);
            goto exit;
        }

        ret = ti926_init_setup(deser);
        if (ret < 0)
            goto exit;

        deser->status = DESER_STREAM_ON;

    } else {
        deser->status = DESER_IDLE;
        ti926_write_reg(deser, 0x64, 0x10);
        ti926_write_reg(deser, 0x65, 0x00);
    }

exit:
    osMutexRelease(deser->lock);
    return ret;
}

static int ti926_deinit(struct vdev_device *vdev)
{
    struct ti926_dev *deser;

    if (!vdev) {
        return -1;
    }

    deser = (struct ti926_dev *)vdev->priv_data;
    osMutexDelete(deser->lock);
    free(deser);

    return 0;
}

static const struct vdev_dev_ops ti926_vdev_ops = {
    .s_power = ti926_s_power,
    .enum_mbus_fmt = ti926_enum_mbus_fmt,
    .enum_frame_size = ti926_enum_frame_size,
    .enum_frame_interval = ti926_enum_frame_interval,
    .g_frame_interval = ti926_get_frame_interval,
    .s_frame_interval = ti926_set_frame_interval,
    .get_fmt = ti926_get_fmt,
    .set_fmt = ti926_set_fmt,
    .s_stream = ti926_s_stream,
    .close = ti926_deinit,
};

struct ti926_dev *g_deser = NULL;

struct vdev_device *ti926_init(void *init_data)
{
    int ret;
    struct ti926_dev *deser;
    struct camera_res_cfg *cfg = (struct camera_res_cfg *)init_data;

    deser = malloc(sizeof(struct ti926_dev));
    if (!deser || !cfg || !cfg->i2c_handle)
        return NULL;

    memset(deser, 0, sizeof(struct ti926_dev));
    deser->i2c_handle = cfg->i2c_handle;
    deser->deser_addr = TI926_DESER_ADDR;

    deser->gpio_pwn = cfg->gpio[0];
    deser->gpio_intn = cfg->gpio[1];

    deser->vdev.priv_data = (void *)deser;

    ti926_s_power(&deser->vdev, 1);
    ret = ti926_check_chip_id(deser);
    if (ret == 0)
        ti926_s_stream(&deser->vdev, 0);
    ti926_s_power(&deser->vdev, 0);
    if (ret < 0)
        goto err;

    deser->lock = osMutexNew(NULL);

    deser->vdev.ep.bus_type = VDEV_MBUS_PARALLEL;
    deser->vdev.frame_interval.numerator = 1;
    if (test_pattern) {
        deser->vdev.frame_interval.denominator = 83;
        deser->vdev.fmt.width = 1280;
        deser->vdev.fmt.height = 480;
    } else {
        deser->vdev.frame_interval.denominator = 56;
        deser->vdev.fmt.width = 1920;
        deser->vdev.fmt.height = 720;
    }
    deser->vdev.fmt.code = VDEV_MBUS_FMT_RGB24;
    deser->vdev.fmt.field = VDEV_FIELD_NONE;
    deser->vdev.fmt.colorspace = VDEV_COLORSPACE_SRGB;
    deser->vdev.ops = ti926_vdev_ops;

    g_deser = deser;

    return &deser->vdev;

err:
    free(deser);
    return NULL;
}

int user_test(int read, uint8_t reg, uint8_t *val)
{
    if (g_deser == NULL || val == NULL)
        return 0;

    if (reg > 0xf8) {
        read_all_reg(g_deser);
        return 0;
    }

    if (read) {
        ti926_read_reg(g_deser, reg, val);
        printf("user read reg[0x%02x]=0x%02x\n", reg, *val);
    } else {
        printf("user write reg[0x%02x]=0x%02x\n", reg, *val);
        ti926_write_reg(g_deser, reg, *val);
    }
    return 0;
}
