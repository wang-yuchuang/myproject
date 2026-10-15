/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */

#include "sdrv_serdes_core.h"

#ifdef CONFIG_mipi_max96722_sengyun_IMX390C_5200_MAX9295

#include <sys/types.h>
#include "poc-max2008x.h"
#include "tca_gpio_cfg.h"

#define MAX9295A_DEVICE_ID 0x91
#define MAX9295B_DEVICE_ID 0x93
#define MAX96712_DEVICE_ID 0xa0
#define MAX96722_DEVICE_ID 0xa1

static int max96722_initialization_sengyun(struct serdes_dev_t *serdes);
int max9295_read_reg(void *client, u8 address, u16 reg, u8 *val)
{
    return serdes_read_reg_16(client, address, reg, val);
}

int max9295_write_reg(void *client, u8 address, u16 reg, u8 val)
{
    return serdes_write_reg_16(client, address, reg, val);
}

/*This is a function for i2c address remapping The original address is 0x40
  needs to be ramapped for different channel access from soc*/
/* channel 0 0x40 -->0x48
   channel 1 0x40 -->0x49
   channel 2 0x40 -->0x4a
   channel 3 0x40 -->0x4b */
int max9295_map_i2c_addr(void *client, u8 address_src, u8 address_dest)
{
    int ret;
    u16 reg = 0x00;

    ret = max9295_write_reg(client, address_src, reg, address_dest << 1);

    if (ret < 0) {
        serdes_err("%s:  chip 0x%02x error: reg=0x%02x, val=0x%02x\n",
                   __func__, address_src, reg, address_dest << 1);
        return ret;
    }

    serdes_info("%s:  chip 0x%02x : reg=0x%02x, val=0x%02x\n", __func__,
                address_src, reg, address_dest << 1);
    return 0;
}

int max9295_check_chip_id(void *client, u8 address)
{
    int ret = 0;
    u8 chip_id = 0;
    u16 reg = 0x0d;

    ret = max9295_read_reg(client, address, reg, &chip_id);

    if (ret < 0) {
        serdes_info("%s:chip 0x%02x error: reg=0x%02x ret %d\n",
                    __func__, address, reg, ret);
        return ret;
    }

    serdes_err("max9295 dev chipid = 0x%02x\n", chip_id);

    if ((chip_id != MAX9295A_DEVICE_ID) && (chip_id != MAX9295B_DEVICE_ID))
        return -ENODEV;

    return 0;
}


static int max96722_write_reg(struct serdes_dev_t *serdes, u16 reg, u8 val)
{
    void *client = serdes->i2c_client;
    int ret;
    u8 addr;

    addr = serdes->addr_des;
    ret = serdes_write_reg_16(client, addr, reg, val);
    return ret;
}

static int max96722_read_reg(struct serdes_dev_t *serdes, u16 reg, u8 *val)
{
    void *client = serdes->i2c_client;
    int ret;
    u8 addr;

    addr = serdes->addr_des;
    ret = serdes_read_reg_16(client, addr, reg, val);
    return ret;
}

// Power for Rx max96722/712
static void max96722_power(struct serdes_dev_t *serdes, bool enable)
{
    if (strcmp(serdes->gpioext_name, TCA6408_NAME) == 0) {
        gpio_config(&g_gpio_tca6408, serdes->power_gpio, GPIO_OUT);
        gpio_set(&g_gpio_tca6408, serdes->power_gpio, enable);
    }
    if (strcmp(serdes->gpioext_name, TCA9539_NAME) == 0) {
        serdes_err("gpioext name %s, line %d\n",serdes->gpioext_name, __LINE__);
        gpio_config(&g_gpio_tca9539, serdes->power_gpio, GPIO_OUT);
        gpio_set(&g_gpio_tca9539, serdes->power_gpio, enable);
    }
    usleep_range(10000, 11000);
}

// Power for camera module
static int max20087_power(struct serdes_dev_t *serdes, bool enable)
{
    int ret;
    struct poc_cfg max20087_cfg;

    max20087_cfg.poc_i2c_addr = serdes->addr_poc;
    max20087_cfg.gpioext_addr = serdes->addr_extgpio;
    max20087_cfg.poc_gpio = serdes->poc_gpio;
    max20087_cfg.gpioext_name = serdes->gpioext_name;
    ret = poc_power(serdes->i2c_client, &max20087_cfg, 0xf, enable);
    return ret;
}

static int max96722_check_chip_id(struct serdes_dev_t *serdes)
{
    int ret = 0;
    u8 chip_id = 0;

    max96722_power(serdes, 1);

    ret = max96722_read_reg(serdes, 0x000d, &chip_id);

    if (ret < 0) {
        serdes_err("%s: failed to read max96722 chipid.\n", __func__);
        return ret;
    }

    if (chip_id == MAX96722_DEVICE_ID || chip_id == MAX96712_DEVICE_ID) {
        serdes_err("max96722/12 chipid = 0x%02x\n", chip_id);
    }
    else {
        serdes_err("%s: wrong chip identifier, expected 0x%x(max96722), "
                   "0x%x(max96712) got 0x%x\n",
                   __func__, MAX96722_DEVICE_ID, MAX96712_DEVICE_ID,
                   chip_id);
        return -EIO;
    }

    return ret;
}

static void max96722_preset(struct serdes_dev_t *serdes)
{
    // 0x00,0x13,0x75,
    max96722_write_reg(serdes, 0x0013, 0x75);
    usleep_range(10000, 11000);
}

// enable mipi output to soc
void max96722_mipi_enable(struct serdes_dev_t *serdes, bool en)
{
    //0x04,0x0B,0x42
    if (en == 1) {
        max96722_write_reg(serdes, 0x08A0, 0x04);
        max96722_write_reg(serdes, 0x040b, 0x40);
        usleep_range(30000, 31000);
        max96722_write_reg(serdes, 0x08A0, 0x84);
        max96722_write_reg(serdes, 0x040b, 0x42);
    }
    else {
        max96722_write_reg(serdes, 0x08A0, 0x04);
        max96722_write_reg(serdes, 0x040b, 0x40);
    }
}

// enable gmsl link 0x06 bit0~3 for the corresponding channel link enable
static void max96722_link_enable(struct serdes_dev_t *serdes, int en)
{
    u8 value = 0;

    if (en == 1) {
        max96722_read_reg(serdes, 0x0006, &value);

        if (serdes->channel != 0xf)
            value = value | (1 << serdes->channel);
        else
            value = value | serdes->channel;

        max96722_write_reg(serdes, 0x0006, value);
    }
    else {
        max96722_read_reg(serdes, 0x0006, &value);

        if (serdes->channel != 0xf)
            value = value & (~(1 << serdes->channel));
        else
            value = value & (~serdes->channel);

        max96722_write_reg(serdes, 0x0006, value);
    }

    serdes_info("value 0x%x, enable %d\n", value, en);
}

//for both gmsl1 and gmsl2
//0x1dc 0x1fc 0x21c 0x23c 0x25c 0x27c 0x29c 0x2bc bit0 video lock
static int max96722_video_channel_lock(struct serdes_dev_t *serdes)
{
    int channel = serdes->channel;
    u8 value;
    u8 video_lock_status = 0;
    int i = 0;

    if (serdes->sync == 1) {
        for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
            max96722_read_reg(serdes, 0x1dc + i * 0x20, &value);
            value = value & 0x1;
            video_lock_status |= (value << i);
        }
    }
    else {
        max96722_read_reg(serdes, 0x1dc + channel * 0x20, &value);
        video_lock_status = value & 0x1;
    }

    //serdes_info("addr 0x%x value 0x%x\n", 0x1dc+channel*0x20,value);
    return video_lock_status;
}

//this is a function to determine if the current max96722 video status is valid and if a reinit is needed
// true -- the current status is not ok and a reinit is needed
// false  -- the current status is ok and no reinit is needed
static bool max96722_check_video_lock_status(struct serdes_dev_t *serdes)
{
    int i = 0;
    u8 value = 0;
    u8 video_lock_status = 0;

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
        max96722_read_reg(serdes, 0x1dc + i * 0x20, &value);
        value = value & 0x1;
        video_lock_status |= (value << i);
    }

    if ((video_lock_status & serdes->serdes_streamon_bitmap) !=
            serdes->current_lock_status)
        return true;

    if (video_lock_status == 0 && (serdes->serdes_streamon_bitmap != 0)) {
        max96722_read_reg(serdes, 0x6, &value);
        value = value & 0xf;

        if (value ==
                0) { //this is a corner case that the stream on is not zero but all the links are disabled,probably a plug out happened during a reinit. We need a reinit here
            return true;
        }
    }

    return false;
}

//check if video lock is on,if not,reinit and then check the link status
static int max96722_gmsl2_get_link_lock_ex(struct serdes_dev_t *serdes)
{
    int i = 0;
    u8 value = 0;
    u8 lock_status = 0;
    u8 video_lock_status = 0;
    u8 channel = serdes->channel;

    video_lock_status = max96722_video_channel_lock(serdes);

    if (serdes->sync == 1) {
        if (video_lock_status != 0xf) { //reinit when video link lock is lost
            max96722_initialization_sengyun(serdes);

            if (serdes->serdes_status.stream_cnt > 0) {
                max96722_mipi_enable(serdes, 1);
            }

            usleep_range(100000, 110000);//wait to get link locked
            //check link status after reinit
            max96722_read_reg(serdes, 0x1a, &value);
            value = (value >> 3) & 0x1;
            lock_status |= value;

            for (i = 1; i < MAX_SERAILIZER_NUM; i++) {
                max96722_read_reg(serdes, 0xa + i - 1, &value);
                value = (value >> 3) & 0x1;
                lock_status |= (value << i);
            }
        }
        else {
            lock_status = 0xf;
        }
    }
    else {
        if (video_lock_status == 0) { //reinit when video link lock is lost
            max96722_initialization_sengyun(serdes);

            if (serdes->serdes_status.stream_cnt > 0) {
                max96722_mipi_enable(serdes, 1);
            }

            usleep_range(100000, 110000);//wait to get link locked

            //check link status after  reinit
            if (channel > 0)
                max96722_read_reg(serdes, 0xa + channel - 1, &value);

            if (channel == 0)
                max96722_read_reg(serdes, 0x1a, &value);

            lock_status = (value >> 3) & 0x1;
        }
        else {
            lock_status = 1;
        }
    }

    return lock_status;
}

static int max96722_get_gmsl2_link_lock(struct serdes_dev_t *serdes)
{
    u8 lock_status = 0;
    u8 value;
    int i;

    max96722_read_reg(serdes, 0x1a, &value);
    value = (value >> 3) & 0x1;
    lock_status |= value;

    for (i = 1; i < 4; i++) {
        max96722_read_reg(serdes, 0xa + i - 1, &value);
        value = (value >> 3) & 0x1;
        lock_status |= (value << i);
    }

    return lock_status & serdes->serdes_streamon_bitmap;
}

// 0x1a linkA bit 3
// 0xa  linkB bit 3
// 0xb  linkC bit 3
// 0xc  linkD bit 3
static int max96722_gmsl2_link_lock(struct serdes_dev_t *serdes, int channel)
{
    u8 value;

    if (channel > 0)
        max96722_read_reg(serdes, 0xa + channel - 1, &value);

    if (channel == 0)
        max96722_read_reg(serdes, 0x1a, &value);

    return (value >> 3) & 0x1;
}

int max96722_packet_cnt(struct serdes_dev_t *serdes)
{
    u8 value;

    max96722_read_reg(serdes, 0x8d0, &value);
    // serdes_info("addr 0x%x value 0x%x\n", 0x8d0, value);
    return value;
}

// check if all the links are stream off 0x6 bit0~3
static bool max96722_if_all_stream_off(struct serdes_dev_t *serdes)
{
    u8 value = 0;

    max96722_read_reg(serdes, 0x0006, &value);
    value = value & 0xf;

    if (value == 0)
        return true;
    else
        return false;
}

static serdes_param_reg16_t max9295_reg[] = {
    { 0x02be, 0x10, 0x0 },
    { 0x0318, 0x5e, 0x0 },
    { 0x02d3, 0x84, 0x0 },
};

static serdes_param_reg16_t max96722_reg_max9295[] = {
    { 0x0006, 0xf0, 0x0 },
    { 0x0010, 0x11, 0x0 },
    { 0x0011, 0x11, 0x0 },
    { 0x08A0, 0x04, 0x0 }, /*PHY 2x4 mode*/
    { 0x08A2, 0x30, 0x0 }, // enable all the phys
    { 0x08A3, 0xE4, 0x0 }, // phy1 D0->D2 D1->D3 phy0 D0->D0 D1->D1
    { 0x040B, 0x80, 0x0 }, // pipe0 BPP 0x10: datatype 0x1e csi output disable
    { 0x040E, 0x5E, 0x0 }, // bit0~5 pipe0:0x1e bit 7:6 pipe1 data type highest 2 bits
    { 0x040F, 0x7E, 0x0 }, // bit7:4 pipe2 highest 4 bits bit3:0 pipe1 low bits
    { 0x0410, 0x7A, 0x0 }, // bit7:2 pipe3 6 bits bit1:0 pipe2 low 2 bits
    { 0x0411, 0x48, 0x0 }, // bit7:5 pipe2 high bits bit4:0 pipe1 BPP 0x10:datatype 0x1e
    { 0x0412, 0x20, 0x0 }, // bit6:2 pipe3 BPP 0x10:0x1e bit1:0 pipe 2 BPP lower bits
    { 0x0418, 0xEF, 0x0 }, // phy1 lane speed set bit0-4 for lane speed. 0xEF for 1.5G
    { 0x090B, 0x07, 0x0 },
    { 0x092D, 0x15, 0x0 },
    { 0x090D, 0x1E, 0x0 },
    { 0x090E, 0x1E, 0x0 },
    { 0x090F, 0x00, 0x0 },
    { 0x0910, 0x00, 0x0 },
    { 0x0911, 0x01, 0x0 },
    { 0x0912, 0x01, 0x0 },
    { 0x094B, 0x07, 0x0 },
    { 0x096D, 0x15, 0x0 },
    { 0x094D, 0x1E, 0x0 },
    { 0x094E, 0x5E, 0x0 },
    { 0x094F, 0x00, 0x0 },
    { 0x0950, 0x40, 0x0 },
    { 0x0951, 0x01, 0x0 },
    { 0x0952, 0x41, 0x0 },
    { 0x098B, 0x07, 0x0 },
    { 0x09AD, 0x15, 0x0 },
    { 0x098D, 0x1E, 0x0 },
    { 0x098E, 0x9E, 0x0 },
    { 0x098F, 0x00, 0x0 },
    { 0x0990, 0x80, 0x0 },
    { 0x0991, 0x01, 0x0 },
    { 0x0992, 0x81, 0x0 },
    { 0x09CB, 0x07, 0x0 },
    { 0x09ED, 0x15, 0x0 },
    { 0x09CD, 0x1E, 0x0 },
    { 0x09CE, 0xDE, 0x0 },
    { 0x09CF, 0x00, 0x0 },
    { 0x09D0, 0xC0, 0x0 },
    { 0x09D1, 0x01, 0x0 },
    { 0x09D2, 0xC1, 0x0 },
    { 0x04A0, 0x04, 0x0 }, // manual frame sync on MPF2
    { 0x04A2, 0x00, 0x0 }, // Turn off auto master link selection
    { 0x04AA, 0x00, 0x0 }, // Disable overlap window
    { 0x04AB, 0x00, 0x0 },
    { 0x04AF, 0xC0, 0x0 }, // AUTO_FS_LINKS = 0, FS_USE_XTAL = 1, FS_LINK_[3:0] = 0
    { 0x04A7, 0x0C, 0x0 }, // set FSYNC period to 25M/30 CLK cycles. PCLK at 25MHz
    { 0x04A6, 0xB7, 0x0 },
    { 0x04A5, 0x35, 0x0 },
    { 0x04B1, 0x38, 0x0 }, // FSYNC TX ID is 7
};

int max96722_max9295_init_des(struct serdes_dev_t *serdes)
{
    int i = 0;
    int reglen = 0;

    max96722_check_chip_id(serdes);
    max96722_preset(serdes);
    max96722_mipi_enable(serdes, 0);
    reglen = ARRAY_SIZE(max96722_reg_max9295);

    for (i = 0; i < reglen; i++) {
        max96722_write_reg(serdes, max96722_reg_max9295[i].nRegAddr,
                           max96722_reg_max9295[i].nRegValue);
    }

    return 0;
}

int max96722_max9295_init_ser(struct serdes_dev_t *serdes)
{
    int i = 0, j = 0;
    u8 value = 0;
    int reglen = 0;
    int lock = 0;

    serdes_para_t *pPara;

    if (serdes == NULL) {
        serdes_err("no serdes pointer\n");
        return -EINVAL;
    }

    pPara = (serdes_para_t *)serdes->priv;

    if (pPara == NULL) {
        serdes_err("no parameter pointer\n");
        return -EINVAL;
    }

    max20087_power(serdes, 0);
    usleep_range(10000, 11000);
    max20087_power(serdes, 1);
    udelay(120000);
    reglen = ARRAY_SIZE(max9295_reg);
    serdes->ser_online_bitmap = 0;
    max96722_write_reg(serdes, 0x6, 0xff);//open all the links
    udelay(120000);//wait for link stable

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
        lock = max96722_gmsl2_link_lock(serdes, i);

        if (lock == 0) {
            serdes_info("finally channel %d not locked\n", i);
            continue;
        }

        value = 0xff & (~(1 << (i * 2)));
        max96722_write_reg(serdes, 0x3, value);
        usleep_range(100, 110);

        if (max9295_check_chip_id(serdes->i2c_client,
                                  serdes->addr_ser) != 0) {
            serdes_info("%s: could not find 9295 on channel %d for 96722\n", __func__, i);
            continue;
        }
        else {
            max9295_map_i2c_addr(serdes->i2c_client, serdes->addr_ser,
                                 pPara->remap_addr_ser[i]);
            serdes->ser_online_bitmap |= (1 << i);
        }

        for (j = 0; j < reglen; j++) {
            max9295_write_reg(serdes->i2c_client, pPara->remap_addr_ser[i],
                              max9295_reg[j].nRegAddr,
                              max9295_reg[j].nRegValue);
        }
    }

    max96722_write_reg(serdes, 0x3, 0x0);//open all the reverse channel
    usleep_range(200000, 201000);
    serdes_info("online bitmap 0x%x\n", serdes->ser_online_bitmap);

    if (serdes->ser_online_bitmap == 0)
        return -ENODEV;

    return 0;
}

static void max96722_serdes_power(struct serdes_dev_t *serdes, bool enable)
{
    if (enable) {
        max96722_power(serdes, 0);
        usleep_range(10000, 11000);
        max96722_power(serdes, 1);
        usleep_range(10000, 11000);
    }
    else {
        max96722_power(serdes, 0);
        usleep_range(10000, 11000);
    }
}

static int max96722_initialization_sengyun(struct serdes_dev_t *serdes)
{
    int ret = 0;
    max96722_max9295_init_des(serdes);
    ret = max96722_max9295_init_ser(serdes);
    return ret;
}

serdes_para_t max96722_sengyun_IMX390c_5200_GMSL2_1080p = {
    .module_name = "max96722_sengyun_IMX390c_5200_GMSL2_1080p",
    .bus_type = VDEV_MBUS_CSI2,
    .numerator = 1,
    .colorspace = VDEV_COLORSPACE_SRGB,
    .field = VDEV_FIELD_NONE,

    //these fields are per camera
    .mbus_code[0] = VDEV_MBUS_FMT_UYVY,
    .mbus_code[1] = VDEV_MBUS_FMT_UYVY,
    .mbus_code[2] = VDEV_MBUS_FMT_UYVY,
    .mbus_code[3] = VDEV_MBUS_FMT_UYVY,
    .fps[0] = 30,
    .fps[1] = 30,
    .fps[2] = 30,
    .fps[3] = 30,
    .remap_addr_ser[0] = 0x48,
    .remap_addr_ser[1] = 0x49,
    .remap_addr_ser[2] = 0x4a,
    .remap_addr_ser[3] = 0x4b,

    .serdes_power = max96722_serdes_power,
    .serdes_init = max96722_initialization_sengyun,
    .serdes_start = max96722_mipi_enable,

    .serdes_check_link_status = max96722_get_gmsl2_link_lock,
    .serdes_check_link_status_ex = max96722_gmsl2_get_link_lock_ex,
    .serdes_check_video_lock_status = max96722_check_video_lock_status,
};
#endif
