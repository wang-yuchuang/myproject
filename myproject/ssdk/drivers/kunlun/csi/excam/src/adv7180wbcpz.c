#include <sys/types.h>
#include "exvdev.h"
#include "driver/kunlun/csi/cam_res.h"

#define ADV7180_WIDTH  720
#define ADV7180_HEIGHT  640

#define ADV7180_STD_AD_PAL_BG_NTSC_J_SECAM		0x0
#define ADV7180_STD_AD_PAL_BG_NTSC_J_SECAM_PED		0x1
#define ADV7180_STD_AD_PAL_N_NTSC_J_SECAM		0x2
#define ADV7180_STD_AD_PAL_N_NTSC_M_SECAM		0x3
#define ADV7180_STD_NTSC_J				0x4
#define ADV7180_STD_NTSC_M				0x5
#define ADV7180_STD_PAL60				0x6
#define ADV7180_STD_NTSC_443				0x7
#define ADV7180_STD_PAL_BG				0x8
#define ADV7180_STD_PAL_N				0x9
#define ADV7180_STD_PAL_M				0xa
#define ADV7180_STD_PAL_M_PED				0xb
#define ADV7180_STD_PAL_COMB_N				0xc
#define ADV7180_STD_PAL_COMB_N_PED			0xd
#define ADV7180_STD_PAL_SECAM				0xe
#define ADV7180_STD_PAL_SECAM_PED			0xf

#define ADV7180_REG_INPUT_CONTROL			0x0000
#define ADV7180_INPUT_CONTROL_INSEL_MASK		0x0f

#define ADV7180_REG_OUTPUT_CONTROL			0x0003
#define ADV7180_REG_EXTENDED_OUTPUT_CONTROL		0x0004
#define ADV7180_EXTENDED_OUTPUT_CONTROL_NTSCDIS	 0x07//0xC5//0xC5

#define ADV7180_REG_AUTODETECT_ENABLE			0x0007
#define ADV7180_AUTODETECT_DEFAULT			0x7f
/* Contrast */
#define ADV7180_REG_CON		0x0008	/*Unsigned */
#define ADV7180_CON_MIN		0
#define ADV7180_CON_DEF		128
#define ADV7180_CON_MAX		255
/* Brightness*/
#define ADV7180_REG_BRI		0x000a	/*Signed */
#define ADV7180_BRI_MIN		-128
#define ADV7180_BRI_DEF		0
#define ADV7180_BRI_MAX		127
/* Hue */
#define ADV7180_REG_HUE		0x000b	/*Signed, inverted */
#define ADV7180_HUE_MIN		-127
#define ADV7180_HUE_DEF		0
#define ADV7180_HUE_MAX		128

#define ADV7180_REG_CTRL		0x000e
#define ADV7180_CTRL_IRQ_SPACE		0x20

#define ADV7180_REG_PWR_MAN		0x0f
#define ADV7180_PWR_MAN_ON		0x04
#define ADV7180_PWR_MAN_OFF		0x24
#define ADV7180_PWR_MAN_RES		0x80

#define ADV7180_REG_STATUS1		0x0010
#define ADV7180_STATUS1_IN_LOCK		0x01
#define ADV7180_STATUS1_AUTOD_MASK	0x70
#define ADV7180_STATUS1_AUTOD_NTSM_M_J	0x00
#define ADV7180_STATUS1_AUTOD_NTSC_4_43 0x10
#define ADV7180_STATUS1_AUTOD_PAL_M	0x20
#define ADV7180_STATUS1_AUTOD_PAL_60	0x30
#define ADV7180_STATUS1_AUTOD_PAL_B_G	0x40
#define ADV7180_STATUS1_AUTOD_SECAM	0x50
#define ADV7180_STATUS1_AUTOD_PAL_COMB	0x60
#define ADV7180_STATUS1_AUTOD_SECAM_525	0x70

#define ADV7180_REG_IDENT 0x0011
#define ADV7180_ID_7180 0x18

#define ADV7180_REG_STATUS3		0x0013
#define ADV7180_REG_ANALOG_CLAMP_CTL	0x0014
#define ADV7180_REG_SHAP_FILTER_CTL_1	0x0017
#define ADV7180_REG_CTRL_2		0x001d
#define ADV7180_REG_VSYNC_FIELD_CTL_1	0x0031
#define ADV7180_REG_MANUAL_WIN_CTL_1	0x003d
#define ADV7180_REG_MANUAL_WIN_CTL_2	0x003e
#define ADV7180_REG_MANUAL_WIN_CTL_3	0x003f
#define ADV7180_REG_LOCK_CNT		0x0051
#define ADV7180_REG_CVBS_TRIM		0x0052
#define ADV7180_REG_CLAMP_ADJ		0x005a
#define ADV7180_REG_RES_CIR		0x005f
#define ADV7180_REG_DIFF_MODE		0x0060

#define ADV7180_REG_ICONF1		0x2040
#define ADV7180_ICONF1_ACTIVE_LOW	0x01
#define ADV7180_ICONF1_PSYNC_ONLY	0x10
#define ADV7180_ICONF1_ACTIVE_TO_CLR	0xC0
/* Saturation */
#define ADV7180_REG_SD_SAT_CB	0x00e3	/*Unsigned */
#define ADV7180_REG_SD_SAT_CR	0x00e4	/*Unsigned */
#define ADV7180_SAT_MIN		0
#define ADV7180_SAT_DEF		128
#define ADV7180_SAT_MAX		255

#define ADV7180_IRQ1_LOCK	0x01
#define ADV7180_IRQ1_UNLOCK	0x02
#define ADV7180_REG_ISR1	0x2042
#define ADV7180_REG_ICR1	0x2043
#define ADV7180_REG_IMR1	0x2044
#define ADV7180_REG_IMR2	0x2048
#define ADV7180_IRQ3_AD_CHANGE	0x08
#define ADV7180_REG_ISR3	0x204A
#define ADV7180_REG_ICR3	0x204B
#define ADV7180_REG_IMR3	0x204C
#define ADV7180_REG_IMR4	0x2050

#define ADV7180_REG_NTSC_V_BIT_END	0x00E6
#define ADV7180_NTSC_V_BIT_END_MANUAL_NVEND	0x4F

#define ADV7180_REG_VPP_SLAVE_ADDR	0xFD
#define ADV7180_REG_CSI_SLAVE_ADDR	0xFE

#define ADV7180_REG_ACE_CTRL1		0x4080
#define ADV7180_REG_ACE_CTRL5		0x4084
#define ADV7180_REG_FLCONTROL		0x40e0
#define ADV7180_FLCONTROL_FL_ENABLE 0x1

#define ADV7180_REG_RST_CLAMP	0x809c
#define ADV7180_REG_AGC_ADJ1	0x80b6
#define ADV7180_REG_AGC_ADJ2	0x80c0

#define ADV7180_CSI_REG_PWRDN	0x00
#define ADV7180_CSI_PWRDN	0x80

#define ADV7180_INPUT_CVBS_AIN1 0x00
#define ADV7180_INPUT_CVBS_AIN2 0x01
#define ADV7180_INPUT_CVBS_AIN3 0x02
#define ADV7180_INPUT_CVBS_AIN4 0x03
#define ADV7180_INPUT_CVBS_AIN5 0x04
#define ADV7180_INPUT_CVBS_AIN6 0x05
#define ADV7180_INPUT_SVIDEO_AIN1_AIN2 0x06
#define ADV7180_INPUT_SVIDEO_AIN3_AIN4 0x07
#define ADV7180_INPUT_SVIDEO_AIN5_AIN6 0x08
#define ADV7180_INPUT_YPRPB_AIN1_AIN2_AIN3 0x09
#define ADV7180_INPUT_YPRPB_AIN4_AIN5_AIN6 0x0a

#define ADV7180_I2C_SLAVE_ID 0x21
#define ADV7180_DEFAULT_CSI_I2C_ADDR 0x44
#define ADV7180_DEFAULT_VPP_I2C_ADDR 0x42

#define ADV7180_FLAG_RESET_POWERED	(1 << 0)
#define ADV7180_FLAG_V2			BIT(1 << 1)
#define ADV7180_FLAG_MIPI_CSI2		BIT(1 << 2)
#define ADV7180_FLAG_I2P		BIT(1 << 3)

struct adv7180_pixfmt
{
    uint32_t code;
    uint32_t colorspace;
};

enum adv7180_frame_rate {
    ADV7180_15_FPS = 0,
    ADV7180_30_FPS,
    ADV7180_45_FPS,
    ADV7180_60_FPS,
    ADV7180_NUM_FRAMERATES,
};

struct adv7180_dev {
    void *i2c_client;
    u8 i2c_addr;
    unsigned int i2c_bus_num;
    uint32_t pwdn_gpio;
    uint32_t reset_gpio;

    struct vdev_device vdev;
    struct vdev_mbus_framefmt fmt;
    osMutexId_t lock;
    bool streaming;
    unsigned int register_page;
};

extern gpio_dev_t g_gpio_dev;

static const int adv7180_framerates[] = {
    [ADV7180_15_FPS] = 15,
    [ADV7180_30_FPS] = 30,
    [ADV7180_45_FPS] = 45,
    [ADV7180_60_FPS] = 60,
};

static const struct adv7180_pixfmt adv7180_formats[] = {
    {
        VDEV_MBUS_FMT_UYVY,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_YUYV,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_UYVYSP,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_YUYVSP,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_YUV444,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_YUV420SP,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_YUV420XP,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_RGB565,
        VDEV_COLORSPACE_SRGB,
    },
    {
        VDEV_MBUS_FMT_RGB565X,
        VDEV_COLORSPACE_SRGB,
    },
};

static inline struct adv7180_dev *to_adv7180_dev(struct vdev_device *vdev)
{
    return containerof(vdev, struct adv7180_dev, vdev);
}

int adv7180_read_reg_8(void *client, u8 address, u8 reg, u8 *val)
{
    u8 buf[1];
    int ret = 0;

    memset(buf, 0, sizeof(buf));
    ret = i2c_read_reg8(client, address, reg, buf, 1);
    if (ret < 0) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
        return ret;
    }

    *val = buf[0];
    return 0;
}

int adv7180_write_reg_8(void *client, u8 address, u8 reg, u8 val)
{
    int ret;

    ret = i2c_write_reg8(client, address, reg, &val, 1);
    if (ret < 0) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
        return ret;
    }

    return 0;
}

void adv7180_pwdn(struct adv7180_dev *sensor, bool enable)
{
    gpio_dev_t *gpio_dev;
    gpio_dev = &g_gpio_dev;

    if (sensor->pwdn_gpio < 0) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
        return;
    }

    gpio_config(gpio_dev, sensor->pwdn_gpio, GPIO_OUT);
    gpio_set(gpio_dev, sensor->pwdn_gpio, !!enable);
}

void adv7180_reset(struct adv7180_dev *sensor, bool enable)
{
    gpio_dev_t *gpio_dev;
    gpio_dev = &g_gpio_dev;

    if (sensor->reset_gpio < 0) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
        return;
    }

    gpio_config(gpio_dev, sensor->reset_gpio, GPIO_OUT);
    gpio_set(gpio_dev, sensor->reset_gpio, !!enable);
}

static int adv7180_select_page(struct adv7180_dev *sensor, unsigned int page)
{
	int ret = 0;
    uint8_t val;

	if (sensor->register_page == page) {
		return 0;
	}

    val = (uint8_t)page;
    ret = i2c_write_reg8(sensor->i2c_client, sensor->i2c_addr, ADV7180_REG_CTRL, &val, 1);
	if (ret < 0) {
		sensor_err("<%d, %s>err:(0x%x, 0x%x)\n", __LINE__, __func__, ADV7180_REG_CTRL, page);
		return ret;
	}

	sensor->register_page = page;
	return 0;
}

static int adv7180_write(struct adv7180_dev *sensor, unsigned int reg,
	unsigned int value)
{
	int ret = 0;
    uint8_t val;

	adv7180_select_page(sensor, reg >> 8);
    val = (uint8_t)value;
    ret = i2c_write_reg8(sensor->i2c_client, sensor->i2c_addr, reg & 0xff, &val, 1);
	if (ret < 0) {
		sensor_err("<%d, %s>err:(0x%x, 0x%x)\n", __LINE__, __func__, reg, value);
		return ret;
	}

	return 0;
}

static int adv7180_setting(struct adv7180_dev *sensor)
{
    u8 data;
    u8 chipid = 0;

    /* read chipid */
    adv7180_read_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x11, &chipid);
    sensor_info("<%d, %s>:chipid=0x%x\n", __LINE__, __func__, chipid);
    if ((chipid != 0x1E) &&  (chipid !=0x1C) &&  (chipid !=0x43)) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
    	return -1;
    }

    /* 设置色度/彩度/亮度等          */
    //adv7180_write(state, ADV7180_REG_CON, 0x80);
    //adv7180_write(state, ADV7180_REG_BRI, 0x00);
    adv7180_write(sensor, ADV7180_REG_CON, 160);
    adv7180_write(sensor, ADV7180_REG_BRI, 150-128);

    adv7180_write(sensor, ADV7180_REG_SD_SAT_CB, 0x80);
    adv7180_write(sensor, ADV7180_REG_SD_SAT_CR, 0x80);
    adv7180_write(sensor, ADV7180_REG_HUE, 0x00);

    /* ADI required write */
    adv7180_write(sensor, 0x80d9, 0x44);
    adv7180_write(sensor, ADV7180_REG_FLCONTROL, ADV7180_FLCONTROL_FL_ENABLE);
    adv7180_write(sensor, ADV7180_REG_PWR_MAN, ADV7180_PWR_MAN_RES);
    adv7180_write(sensor, ADV7180_REG_EXTENDED_OUTPUT_CONTROL, ADV7180_EXTENDED_OUTPUT_CONTROL_NTSCDIS);

    /* ITU-R BT.656-4 compatible */
    adv7180_write(sensor, ADV7180_REG_EXTENDED_OUTPUT_CONTROL, ADV7180_EXTENDED_OUTPUT_CONTROL_NTSCDIS);

    /* Manually set V bit end position in NTSC mode */
    adv7180_write(sensor, ADV7180_REG_NTSC_V_BIT_END,
    ADV7180_NTSC_V_BIT_END_MANUAL_NVEND);
    adv7180_write(sensor, ADV7180_REG_INPUT_CONTROL,0x50);

    usleep_range(10000, 10100);
    adv7180_write(sensor, 0x0f, 0x04);
    usleep_range(10000, 10100);

    adv7180_write(sensor, 0x00, 0x50);
    adv7180_write(sensor, 0x14, 0x30);
    adv7180_write(sensor, 0x0f, 0x40);
    adv7180_write(sensor, 0x01, 0x80);
    adv7180_write(sensor, 0x19, 0xb1);
    adv7180_write(sensor, 0x2c, 0x12);
    adv7180_write(sensor, 0x31, 0x02);
    adv7180_write(sensor, 0x37, 0x81);
    adv7180_write(sensor, 0x51, 0x89);
    adv7180_write(sensor, 0x3a, 0x16);
    adv7180_write(sensor, 0x3d, 0xa2);
    adv7180_write(sensor, 0x3e, 0x6a);
    adv7180_write(sensor, 0x3f, 0xa0);
    adv7180_write(sensor, 0x51, 0x9b);
    adv7180_write(sensor, 0x0e, 0x85);
    adv7180_write(sensor, 0xb6, 0x0c);
    adv7180_write(sensor, 0xbe, 0xc0);
    adv7180_write(sensor, 0xc0, 0xc0);
    adv7180_write(sensor, 0xd1, 0xb9);
    adv7180_write(sensor, 0xd6, 0x6d);
    adv7180_write(sensor, 0xd9, 0x44);
    adv7180_write(sensor, 0x0e, 0x80);
    adv7180_write(sensor, 0x55, 0x81);
    adv7180_write(sensor, 0x0e, 0x00);
    adv7180_write(sensor, 0x0C, 0x02);
    adv7180_write(sensor, 0x0D, 0x88);

    /* set to NTSC */
    adv7180_write_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x00, 0x40);

    data = 0;
    adv7180_read_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x08, &data);
    sensor_info("<%d, %s>:contrast=0x%x\n", __LINE__, __func__, data);

    data = 0;
    adv7180_read_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x0A, &data);
    sensor_info("<%d, %s>:brightness=0x%x\n", __LINE__, __func__, data);

    data = 0;
    adv7180_read_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x0B, &data);
    sensor_info("<%d, %s>:hue=0x%x\n", __LINE__, __func__, data);

    data = 0;
    adv7180_read_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x00, &data);
    sensor_info("<%d, %s>:channel=0x%x\n", __LINE__, __func__, data);

    data = 0;
    adv7180_read_reg_8(sensor->i2c_client, sensor->i2c_addr, 0x07, &data);
    sensor_info("<%d, %s>:reg07=0x%x\n", __LINE__, __func__, data);

    return 0;
}

/* fixed the hot-plug camera picture is jagged bug. */
static int adv7180_s_power(struct vdev_device *vdev, int on)
{
    struct adv7180_dev *sensor = to_adv7180_dev(vdev);

    if (on) {
        /* add power-on sequence here */
        adv7180_pwdn(sensor, 1);
        adv7180_reset(sensor, 1);
        usleep_range(5000, 5100);

        /* add setting here */
        adv7180_setting(sensor);
    }
    else {
        /* add power-off sequence here */
        adv7180_pwdn(sensor, 0);
        adv7180_reset(sensor, 0);
    }

    return 0;
}

static int adv7180_get_fmt(struct vdev_device *vdev,
                          struct vdev_mbus_framefmt *fmt)
{
    struct adv7180_dev *sensor = to_adv7180_dev(vdev);

    osMutexAcquire(sensor->lock, 0);

    *fmt = sensor->vdev.fmt;

    osMutexRelease(sensor->lock);

    return 0;
}

static int adv7180_set_fmt(struct vdev_device *vdev,
                          struct vdev_mbus_framefmt format)
{
    struct adv7180_dev *sensor = to_adv7180_dev(vdev);

	if(format.code != sensor->fmt.code)
		return -EINVAL;

	return 0;
}

static int adv7180_enum_fmt(struct vdev_device *vdev,
                           struct vdev_mbus_framefmt *fmt)
{
    if (!fmt || (fmt->index >= (sizeof(adv7180_formats) / sizeof(adv7180_formats[0])))) {
        return -1;
    }

    fmt->code = adv7180_formats[fmt->index].code;
    fmt->colorspace = adv7180_formats[fmt->index].colorspace;
    fmt->field = VDEV_FIELD_INTERLACED;

    return 0;
}

static int adv7180_enum_frame_size(struct vdev_device *vdev,
                                  struct vdev_frame_size_enum *fse)
{
    if (fse->index > 0) {
        fse->index = 0;
        return -1;
    }

    fse->min_width = ADV7180_WIDTH;
    fse->max_width = ADV7180_WIDTH;
    fse->min_height = ADV7180_HEIGHT;
    fse->max_height = ADV7180_HEIGHT;

    return 0;
}

static int adv7180_enum_frame_interval(struct vdev_device *vdev,
                                      struct vdev_frame_interval_enum *fie)
{
    return 0;
}

static int adv7180_g_frame_interval(struct vdev_device *vdev,
                                   struct vdev_fract *fi)
{
    struct adv7180_dev *sensor = to_adv7180_dev(vdev);

    osMutexAcquire(sensor->lock, 0);
    *fi = sensor->vdev.frame_interval;
    osMutexRelease(sensor->lock);
    return 0;
}

static int adv7180_s_frame_interval(struct vdev_device *vdev,
                                   struct vdev_fract frame_interval)
{
    return 0;
}

static int adv7180_s_stream(struct vdev_device *vdev, int enable)
{
    struct adv7180_dev *sensor = to_adv7180_dev(vdev);
    int ret = 0;

    osMutexAcquire(sensor->lock, 0);
    sensor->streaming = enable;
    osMutexRelease(sensor->lock);

    return ret;
}

static int adv7180_exit(struct vdev_device *vdev)
{
    struct adv7180_dev *sensor = to_adv7180_dev(vdev);

    osMutexDelete(sensor->lock);
    free(sensor);
    return 0;
}

static const struct vdev_dev_ops adv7180_vdev_ops = {
    .s_power = adv7180_s_power,
    .enum_mbus_fmt = adv7180_enum_fmt,
    .enum_frame_size = adv7180_enum_frame_size,
    .enum_frame_interval = adv7180_enum_frame_interval,
    .g_frame_interval = adv7180_g_frame_interval,
    .s_frame_interval = adv7180_s_frame_interval,
    .get_fmt = adv7180_get_fmt,
    .set_fmt = adv7180_set_fmt,
    .s_stream = adv7180_s_stream,
    .close = adv7180_exit,
};

struct vdev_device *adv7180_init(void *init_data)
{
    struct adv7180_dev *sensor = NULL;
    struct vdev_mbus_framefmt *fmt;
    struct camera_res_cfg *cfg = (struct camera_res_cfg *)init_data;

    sensor_info("<%d, %s>adv7180_init start V1.2\n", __LINE__, __func__);
    sensor = malloc(sizeof(*sensor));
    if (!sensor) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
        return NULL;
    }
    memset(sensor, 0, sizeof(struct adv7180_dev));

    sensor->i2c_client = cfg->i2c_handle;
    sensor->i2c_addr = 0x21;
    sensor->i2c_bus_num = 0;
    sensor->pwdn_gpio = -1;     /*PortConf_PIN_OSPI2_DATA1*/
    sensor->reset_gpio = -1;    /*PortConf_PIN_OSPI2_DATA1*/

    fmt = &sensor->vdev.fmt;
    fmt->code = VDEV_MBUS_FMT_UYVY;
    fmt->colorspace = VDEV_COLORSPACE_SRGB;
    fmt->width = ADV7180_WIDTH;
    fmt->height = ADV7180_HEIGHT;
    fmt->field_w[0] = 720;
    fmt->field_h[0] = 254;
    fmt->field_w[1] = 720;
    fmt->field_h[1] = 253;
    fmt->field = VDEV_FIELD_INTERLACED;

    sensor->vdev.ep.bus_type = VDEV_MBUS_BT656;
    sensor->vdev.ep.flags = VDEV_MBUS_PCLK_SAMPLE_RISING;
    sensor->vdev.frame_interval.numerator = 1;
    sensor->vdev.frame_interval.denominator = adv7180_framerates[ADV7180_30_FPS];
    sensor->vdev.ops = adv7180_vdev_ops;

    sensor->lock = osMutexNew(NULL);
    if (sensor->lock == NULL) {
        sensor_err("<%d, %s>err!\n", __LINE__, __func__);
        goto err;
    }
    sensor_info("<%d, %s>adv7180_init end \n", __LINE__, __func__);

    return &sensor->vdev;

err:
    if (sensor != NULL)
        free(sensor);

    return NULL;
}

struct external_camera_api adv7180_interface = {
    .name = "adv7180_dev",
    .init = adv7180_init,
};

