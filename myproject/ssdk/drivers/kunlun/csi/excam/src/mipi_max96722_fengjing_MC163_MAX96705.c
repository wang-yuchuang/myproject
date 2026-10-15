/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */

#include "sdrv_serdes_core.h"
#ifdef CONFIG_mipi_max96722_fengjing_MC163_MAX96705

#include <sys/types.h>
#include "poc-max2008x.h"
#include "tca_gpio_cfg.h"

#define MAX96712_DEVICE_ID 0xa0
#define MAX96722_DEVICE_ID 0xa1
#define MAX96705_DEVICE_ID 0x41

static int max96722_initialization_fengjing(struct serdes_dev_t *serdes);
static int max96705_read_reg(void *client, u8 address, u8 reg, u8 *val)
{
    return serdes_read_reg_8(client, address, reg, val);
}

static int max96705_write_reg(void *client, u8 address, u8 reg, u8 val)
{
    return serdes_write_reg_8(client, address, reg, val);
}

/*This is a function for i2c address remapping The original address is 0x40 and
  needs to be ramapped for different channel access from soc*/
/* for avm normally the
   channel 0 0x40 -->0x48
   channel 1 0x40 -->0x49
   channel 2 0x40 -->0x4a
   channel 3 0x40 -->0x4b */
static int max96705_map_i2c_addr(void *client, u8 address_src, u8 address_dest)
{
    int ret;
    u8 reg = 0x00;

    if (client == NULL) {
        serdes_err("no client %s %d", __func__, __LINE__);
        return -ENODEV;
    }

    ret = max96705_write_reg(client, address_src, reg, address_dest << 1);

    if (ret < 0) {
        serdes_err("%s:  chip 0x%02x error: reg=0x%02x, val=0x%02x\n",
                   __func__, address_src, reg, address_dest << 1);
        return ret;
    }

    serdes_info("%s:  chip 0x%02x : reg=0x%02x, val=0x%02x\n", __func__,
                address_src, reg, address_dest << 1);
    return 0;
}

static int max96705_check_chip_id(void *client, u8 address)
{
    int ret = 0;
    u8 chip_id = 0;
    u8 reg = 0x1E;

    ret = max96705_read_reg(client, address, reg, &chip_id);

    if (ret < 0) {
        serdes_err("%s:chip 0x%02x error: reg=0x%02x ret %d\n", __func__,
                   address, reg, ret);
        return ret;
    }

    if (chip_id != MAX96705_DEVICE_ID) {
        serdes_err("max96705 dev chipid = 0x%02x expected 0x%x\n",
                   chip_id, MAX96705_DEVICE_ID);
        return -ENODEV;
    }

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
    ret = poc_power(serdes->i2c_client, &max20087_cfg, serdes->channel, enable);
    return ret;
}

static int max96722_check_chip_id(struct serdes_dev_t *serdes)
{
    int ret = 0;
    u8 chip_id = 0;

    max96722_power(serdes, 1);
    usleep_range(10000, 11000);

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
static void max96722_mipi_enable(struct serdes_dev_t *serdes, bool en)
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

// for both gmsl1 and gmsl2
// 0x1dc 0x1fc 0x21c 0x23c 0x25c 0x27c 0x29c 0x2bc bit0 video lock
static int max96722_channel_inited(struct serdes_dev_t *serdes)
{
    int channel = serdes->channel;
    serdes_para_t *pPara = (serdes_para_t *)serdes->priv;;
    u8 video_lock_status = 0;
    int i = 0;
    int ret = -1;

    if (serdes->sync == 1) {
        for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
            ret = max96705_check_chip_id(serdes->i2c_client, pPara->remap_addr_ser[i]);
            if (ret == 0)
                video_lock_status |= (1 << i);
        }
    }
    else {
        ret = max96705_check_chip_id(serdes->i2c_client, pPara->remap_addr_ser[channel]);
        if (ret == 0)
            video_lock_status = 0x1;
    }

    return video_lock_status;
}

//check if video lock is on,if not,reinit and then check the link status
static int max96722_gmsl1_get_link_lock_ex(struct serdes_dev_t *serdes)
{
    int i = 0;
    u8 value = 0;
    u8 lock_status = 0;
    u8 video_lock_status = 0;

    video_lock_status = max96722_channel_inited(serdes);
    serdes_info("%s video_lock_status 0x%x channel %d\n", __func__,video_lock_status,
                serdes->channel);

    if (serdes->sync == 1) {
        if (video_lock_status != 0xf) { //reinit when video link lock is lost
            //do reinit when the lock status is not zero
            max96722_initialization_fengjing(serdes);

            if (serdes->serdes_status.stream_cnt > 0) {
                max96722_mipi_enable(serdes, 1);
            }

            usleep_range(100000, 110000);;//wait to get link locked

            //check link status after  reinit
            for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
                max96722_read_reg(serdes, 0xbcb + i * 0x100, &value);
                value = value & 0x1;
                lock_status |= (value << i);
            }
        }
        else {
            lock_status = 0xf;
        }
    }
    else {
        if (video_lock_status == 0) { //reinit when video link lock is lost
            max96722_initialization_fengjing(serdes);

            if (serdes->serdes_status.stream_cnt > 0) {
                max96722_mipi_enable(serdes, 1);
            }

            usleep_range(100000, 110000);;//wait to get link locked
            //check link status after  reinit
            max96722_read_reg(serdes, 0xbcb + serdes->channel * 0x100, &value);
            lock_status = value & 0x1;
        }
        else {
            lock_status = 1;
        }
    }

    return lock_status;
}

// for gmsl2 only(only gmsl2 uses pipeline)
// 0x108 0x11a 0x12c 0x13e 0x150 0x168 0x17a 0x18c bit6 video pipeline lock
static int max96722_video_pipeline_lock(struct serdes_dev_t *serdes)
{
    int channel = serdes->channel;
    u8 value;

    max96722_read_reg(serdes, 0x108 + channel * 0x12, &value);
    // serdes_info("addr 0x%x value 0x%x\n", 0x1dc+channel*0x12,value);
    return (value >> 6) & 0x1;
}

static int max96722_gmsl1_get_link_lock(struct serdes_dev_t *serdes)
{
    int i = 0;
    u8 value = 0;
    u8 lock_status = 0;

    for (i = 0; i < 4; i++) {
        max96722_read_reg(serdes, 0xbcb + i * 0x100, &value);
        value = value & 0x1;
        lock_status |= (value << i);
    }

    return lock_status & serdes->serdes_streamon_bitmap;
}

// 0xbcb 0xccb 0xdcb 0xecb
static int max96722_gmsl1_link_lock(struct serdes_dev_t *serdes, int channel)
{
    u8 value = 0;

    max96722_read_reg(serdes, 0xbcb + channel * 0x100, &value);
    serdes_info("value 0x%x\n", value);

    return value & 0x1;
}

static int max96722_packet_cnt(struct serdes_dev_t *serdes)
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

#define FRAMESYNC_USE

static serdes_param_reg16_t max96722_reg[] = {
    // Begin of Script
    // MAX96712 Link Initialization to pair with GMSL1 Serializers
    { 0x0006, 0x00, 0x0 }, // Disable all links

    // Tx/Rx rate Selection
    { 0x0010, 0x11, 0x00 }, // 3Gbps

    // Power up sequence for GMSL1 HIM capable serializers; Also apply to Ser
    // with power up status unknown
    // Turn on HIM on MAX96712
    { 0x0C06, 0xEF, 0x0 },

#ifndef ONE_PORT
    { 0x0B06, 0xEF, 0x0 },
    //{0x0C06,0xEF, 0x0},
    { 0x0D06, 0xEF, 0x0 },
    { 0x0E06, 0xEF, 0x0 },
#endif
    // disable HS/VS processing
    { 0x0C0F, 0x01, 0x0 },
#ifndef ONE_PORT
    { 0x0B0F, 0x01, 0x0 },
    { 0x0D0F, 0x01, 0x0 },
    { 0x0E0F, 0x01, 0x0 },
#endif
    { 0x0C07, 0x84, 0xa },

#ifndef ONE_PORT
    { 0x0B07, 0x84, 0x0 },
    { 0x0D07, 0x84, 0x0 },
    { 0x0E07, 0x84, 0x0 },
#endif

    // YUV MUX mode
    { 0x041A, 0xF0, 0x0 },

    // Set i2c path
    //{0x0B04,0x03, 0x0},
    //{0x0B0D,0x81, 0x0},// 设置96706的I2C通讯

    // MAX96712 MIPI settings
    { 0x08A0, 0x04, 0x0 },
    { 0x08A2, 0x30, 0x0 },
    { 0x00F4, 0x01, 0x0 }, // enable 4 pipeline 0xF, enable pipeline 0 0x1
    { 0x094A, 0xc0, 0x0 }, // Mipi data lane count b6-7 1/2/4lane
    { 0x08A3, 0xE4, 0x0 }, // 0x44 4 lanes data output, 0x4E lane2/3 data output.
    // 0xE4 lane0/1 data output.

    // BPP for pipe lane 0 set as 1E(YUV422)
    { 0x040B, 0x80, 0x0 },
    { 0x040C, 0x00, 0x0 },
    { 0x040D, 0x00, 0x0 },
    { 0x040E, 0x5E, 0x0 },
    { 0x040F, 0x7E, 0x0 },
    { 0x0410, 0x7A, 0x0 },
    { 0x0411, 0x48, 0x0 },
    { 0x0412, 0x20, 0x0 },

    // lane speed set
    { 0x0415, 0xEA, 0x0 }, // phy0 lane speed set 0xEF for 1.5g
    { 0x0418, 0xEA, 0x0 }, // phy1 lane speed set bit0-4 for lane speed. 10001 0xf1 for 1.5g

    // Mapping settings
    { 0x090B, 0x07, 0x0 },
    { 0x092D, 0x15, 0x0 },
    { 0x090D, 0x1E, 0x0 },
    { 0x090E, 0x1E, 0x0 },
    { 0x090F, 0x00, 0x0 },
    { 0x0910, 0x00, 0x0 }, // frame sync frame start map
    { 0x0911, 0x01, 0x0 },
    { 0x0912, 0x01, 0x0 },

    //{0x0903,0x80, 0x0},//bigger than 1.5Gbps， Deskew can works.

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

#ifdef FRAMESYNC_USE
//-------------- Frame Sync --------------/
// set FSYNC period to 25M/30 CLK cycles. PCLK at 25MHz

#ifdef ONE_PORT
    { 0x04A2, 0x20, 0x0 },
#else
    { 0x04A2, 0x00, 0x0 },
#endif

    { 0x04A7, 0x0F, 0x0 },
    { 0x04A6, 0x42, 0x0 },
    { 0x04A5, 0x40, 0x0 },

    { 0x04AA, 0x00, 0x0 },
    { 0x04AB, 0x00, 0x0 },

// AUTO_FS_LINKS = 0, FS_USE_XTAL = 1, FS_LINK_[3:0] = 0? GMSL1
#ifdef ONE_PORT
    { 0x04AF, 0x42, 0x0 },
#else
    { 0x04AF, 0x4F, 0x0 },
#endif
    { 0x04A0, 0x00, 0x0 },

    { 0x0B08, 0x10, 0x0 },
    { 0x0C08, 0x10, 0x0 },
    { 0x0D08, 0x10, 0x0 },
    { 0x0E08, 0x10, 0x0 },
#endif

    // For debug
    { 0x0001, 0xcc, 0x0 },
    { 0x00FA, 0x10, 0x0 },
};

static serdes_param_reg8_t max96705_reg[] = {
    { 0x07, 0x84, 1000 },
    { 0x43, 0x25, 0 },
    { 0x45, 0x01, 0 },
    { 0x47, 0x26, 1000 },
    { 0x67, 0xc4, 2000 },
};

int max96722_max96705_init_des(struct serdes_dev_t *serdes)
{
    int reglen = 0;
    int i = 0;

    reglen = ARRAY_SIZE(max96722_reg);

    for (i = 0; i < reglen; i++) {
        max96722_write_reg(serdes, max96722_reg[i].nRegAddr,
                           max96722_reg[i].nRegValue);
    }

    return 0;
}

int max96722_max96705_init_ser(struct serdes_dev_t *serdes)
{
    // Camera Module Power On/Off
    u8 value;
    int i = 0;
    int j = 0;
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

    reglen = ARRAY_SIZE(max96705_reg);
    max20087_power(serdes, 0);
    usleep_range(1000, 1100);
    max20087_power(serdes, 1);
    udelay(120000);
    max96722_write_reg(serdes, 0x6, 0x0f);
    usleep_range(10000, 11000);//wait link to be stable
    serdes->ser_online_bitmap = 0;

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
        lock = max96722_gmsl1_link_lock(serdes, i);

        if (lock == 0) {
            serdes_err("channel %d not locked\n", i);
            continue;
        }

        max96722_read_reg(serdes, 0xb04 + i * 0x100, &value);
        max96722_write_reg(serdes, 0xb04 + i * 0x100,
                           value | (1 << 1)); //open current reverse channel
        usleep_range(100, 110);

        //close other reverse channel
        for (j = 0; j < MAX_SERAILIZER_NUM; j++) {
            if (j != i) {
                max96722_read_reg(serdes, 0xb04 + j * 0x100, &value);
                max96722_write_reg(serdes, 0xb04 + j * 0x100, value & (0xfd));
                usleep_range(100, 110);
            }
        }

        if (max96705_check_chip_id(serdes->i2c_client,
                                   serdes->addr_ser) != 0) {
            serdes_err("%s  ch %d wrong chip identifier\n", __func__, i);
            continue;
        }

        max96705_map_i2c_addr(serdes->i2c_client,
                              serdes->addr_ser,
                              pPara->remap_addr_ser[i]);
        serdes->ser_online_bitmap |= (1 << i);

        for (j = 0; j < reglen; j++) {
            max96705_write_reg(serdes->i2c_client,
                               pPara->remap_addr_ser[i],
                               max96705_reg[j].nRegAddr,
                               max96705_reg[j].nRegValue);

            if (max96705_reg[j].nDelay > 0)
                usleep_range(max96705_reg[j].nDelay,
                             max96705_reg[j].nDelay + 100);
        }
    }

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) { //open all the reverse channel
        max96722_read_reg(serdes, 0xb04 + i * 0x100, &value);
        max96722_write_reg(serdes, 0xb04 + i * 0x100, value | (1 << 1));
        usleep_range(100, 110);
    }

    serdes_info("online bitmap 0x%x\n", serdes->ser_online_bitmap);
    usleep_range(5000, 5100);

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

static int max96722_initialization_fengjing(struct serdes_dev_t *serdes)
{
    int ret = 0;
    serdes_err("%s %d\n", __func__, __LINE__);
    max96722_max96705_init_des(serdes);
    ret = max96722_max96705_init_ser(serdes);
    return ret;
}

serdes_para_t max96722_fengjing_MC163_MAX96705_720p = {
    .module_name = "max96722_fengjing_MC163_MAX96705_720p",
    .bus_type = VDEV_MBUS_CSI2,
    .numerator = 1,
    .colorspace = VDEV_COLORSPACE_SRGB,
    .field = VDEV_FIELD_NONE,

    //these fields are per camera
    .mbus_code[0] = VDEV_MBUS_FMT_UYVY,
    .mbus_code[1] = VDEV_MBUS_FMT_UYVY,
    .mbus_code[2] = VDEV_MBUS_FMT_UYVY,
    .mbus_code[3] = VDEV_MBUS_FMT_UYVY,
    .fps[0] = 25,
    .fps[1] = 25,
    .fps[2] = 25,
    .fps[3] = 25,
    .remap_addr_ser[0] = 0x48,
    .remap_addr_ser[1] = 0x49,
    .remap_addr_ser[2] = 0x4a,
    .remap_addr_ser[3] = 0x4b,

    .serdes_power = max96722_serdes_power,
    .serdes_init = max96722_initialization_fengjing,
    .serdes_start = max96722_mipi_enable,
    .serdes_check_link_status = max96722_gmsl1_get_link_lock,
    .serdes_check_link_status_ex = max96722_gmsl1_get_link_lock_ex,
    .serdes_check_video_lock_status = max96722_check_video_lock_status,
};

#endif

