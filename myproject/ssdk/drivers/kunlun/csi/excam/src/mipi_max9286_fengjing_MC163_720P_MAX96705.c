/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <sys/types.h>
#include "sdrv_serdes_core.h"

#ifdef CONFIG_mipi_max9286_fengjing_MC163_720P_MAX96705
#include "poc-max2008x.h"
#include "tca_gpio_cfg.h"

#define MAX9286_DEVICE_ID 0x40
#define MAX96705_DEVICE_ID 0x41
#define MAX_CAM_NUM 4

#define DES_1C_HIM_ENABLE 0xf0
/* Register 0x49 */
#define MAX9286_VIDEO_DETECT_MASK   0x0f
/* Register 0x27 */
#define MAX9286_LOCKED          (1<<7)
// Register 0x31
#define MAX9286_FSYNC_LOCKED    (1<<6)

// 0x15 disable csi output
#define DES_REG15_CSIOUT_DISABLE 0x03
// 0x01 disable fs
#define DES_REG1_FS_DISABLE 0xc3
#define DES_REG1_FS_EN 0x40
// 0x1c enable him
#define DES_REG1C_HIM_ENABLE 0xf4
#define DES_REGA_ALLLINK_EN 0xff
#define DES_REG12_YUV8_DBL 0xf3
#define DES_REGC_HVEN 0x91
#define DES_REG69_AUTO_MASK 0x30
#define DES_REG_PERIOD 0x80
#define DES_REG0_INPUTLINK_EN 0xef
#define DES_REG

#define SER_REG4_CFGLINK_EN 0x43
#define SER_REG4_VIDEOLINK_EN 0x83
#define SER_REG7_DBL_HV_EN 0x84
#define SER_REG67_DBLALIGN 0xe7

static int max9286_fengjing_initialization(struct serdes_dev_t *serdes);
static void max9286_out_enable(struct serdes_dev_t *serdes, bool enable);

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

static int max9286_write_reg(struct serdes_dev_t *serdes, u8 reg, u8 val)
{
    void *client = serdes->i2c_client;
    int ret;
    u8 addr;

    addr = serdes->addr_des;
    ret = serdes_write_reg_8(client, addr, reg, val);
    return ret;
}

static int max9286_read_reg(struct serdes_dev_t *serdes, u8 reg, u8 *val)
{
    void *client = serdes->i2c_client;
    int ret;
    u8 addr;

    addr = serdes->addr_des;
    ret = serdes_read_reg_8(client, addr, reg, val);
    return ret;
}

static bool max9286_get_video_lock(struct serdes_dev_t *serdes)
{
    u8 video_lock_status = 0;
    u8 val = 0;
    int ret;

    ret = max9286_read_reg(serdes, 0x27, &val);

    if (ret < 0)
        return false;

    video_lock_status = val & 0xf;

    if ((video_lock_status & serdes->serdes_streamon_bitmap) !=
            serdes->current_lock_status) {
        return true;
    }

    return false;
}

static void max9286_power(struct serdes_dev_t *serdes, bool enable)
{
    serdes_para_t *pPara;

    if (serdes == NULL) {
        return;
    }

    pPara = serdes->priv;

    if (pPara == NULL) {
        return;
    }

    gpio_config(&g_gpio_tca6408, serdes->power_gpio, GPIO_OUT);
    gpio_set(&g_gpio_tca6408, serdes->power_gpio,enable);
}

static void max20086_power(struct serdes_dev_t *serdes, bool enable)
{
    struct poc_cfg max20086_cfg;

    max20086_cfg.poc_i2c_addr = serdes->addr_poc;
    max20086_cfg.gpioext_addr = serdes->addr_extgpio;
    max20086_cfg.poc_gpio = serdes->poc_gpio;
    max20086_cfg.gpioext_name = serdes->gpioext_name;
    serdes_info("20086: channel 0%x enable %d\n", serdes->channel, enable);
    poc_power(serdes->i2c_client, &max20086_cfg, serdes->channel, enable);
}

int max9286_check_chip_id(struct serdes_dev_t *serdes)
{
    int ret = 0;
    u8 chip_id = 0;

    if (serdes == NULL) {
        return -EINVAL;
    }

    max9286_power(serdes, 1);
    usleep_range(10000, 11000);
    ret = max9286_read_reg(serdes, 0x1e, &chip_id);

    if (ret) {
        serdes_err("%s: failed to read chip identifier\n", __func__);
    }

    if (chip_id != MAX9286_DEVICE_ID) {
        serdes_err(
            "%s: wrong chip identifier, expected 0x%x(max9286), got 0x%x\n",
            __func__, MAX9286_DEVICE_ID, chip_id);
        ret = -ENXIO;
    }

    serdes_err("chip id 0x%x\n", chip_id);
    return ret;
}

int max9286_get_video_links(struct serdes_dev_t *serdes)
{
    u8 val;
    int ret;

    if (serdes == NULL) {
        return -EINVAL;
    }

    ret = max9286_read_reg(serdes, 0x49, &val);

    if (ret < 0)
        return -EIO;

    return (val & 0xf)&serdes->serdes_streamon_bitmap;
}

int max9286_get_vsync_link_lock(struct serdes_dev_t *serdes)
{
    u8 val;
    int ret;
    int lock_status;

    ret = max9286_read_reg(serdes, 0x27, &val);

    if (ret < 0)
        return -EIO;

    lock_status = val & 0xf;

    return lock_status;
}

static int max9286_get_link_lock_ex(struct serdes_dev_t *serdes)
{
    u8 val;
    int link_lock = 0;
    int vsync_lock = max9286_get_vsync_link_lock(serdes);

    if (vsync_lock != 0xf) { //check if link lock is ok
        max9286_read_reg(serdes, 0x49, &val);
        link_lock = val & 0xf;

        if (link_lock == 0xf) { //all links online,reinit
            max9286_fengjing_initialization(serdes);

            if (serdes->serdes_status.stream_cnt > 0) {
                max9286_out_enable(serdes, 1);
            }

            //check link status after  reinit
            max9286_read_reg(serdes, 0x49, &val);
            link_lock = val & 0xf;
        }
    }
    else {
        link_lock = 0xf;
    }

    return link_lock;
}

#define MAX_CHECK_COUNT (10)
#define MAX_CHECK_VIDEO_COUNT (250)
/*
 * max9286_check_video_links() - Make sure video links are detected and locked
 *
 * Performs safety checks on video link status. Make sure they are detected
 * and all enabled links are locked.
 *
 * Returns 0 for success, -EIO for errors.
 */
int max9286_check_video_links_ex(struct serdes_dev_t *serdes, u8 channel_mask)
{
    unsigned int i;
    int ret;
    u8 val;

    if (serdes == NULL) {
        return -EINVAL;
    }

    /*
     * Make sure valid video links are detected.
     * The delay is not characterized in de-serializer manual, wait up
     * to 5 ms.
     */
    for (i = 0; i < MAX_CHECK_VIDEO_COUNT; i++) {
        ret = max9286_read_reg(serdes, 0x49, &val);

        if (ret < 0)
            return -EIO;

        if ((val & MAX9286_VIDEO_DETECT_MASK) == channel_mask)
            break;

        usleep_range(350, 500);
    }

    if (i == MAX_CHECK_VIDEO_COUNT) {
        if (val == 0) {
            serdes_err("Unable to detect video links: 0x%02x\n", val);
            return -EIO;
        }
    }

    /* Make sure all enabled links are locked (4ms max). */
    if (channel_mask == 0xf) {
        for (i = 0; i < MAX_CHECK_COUNT; i++) {
            ret = max9286_read_reg(serdes, 0x27, &val);

            if (ret < 0)
                return -EIO;

            if (val & MAX9286_LOCKED)
                break;

            usleep_range(400, 500);
        }

        if (i == MAX_CHECK_COUNT) {
            serdes_err("Not all enabled links locked: 0x27 val 0x%x\n", val);
            return -EIO;
        }

        serdes_info(
            "Successfully detected video links after %u loops: 0x%02x\n",
            i, serdes->channel);
    }

    return 0;
}

/*
 * max9286_check_config_link() - Detect and wait for configuration links
 *
 * Determine if the configuration channel is up and settled for a link.
 *
 * Returns 0 for success, -EIO for errors.
 */
static int max9286_check_config_link(struct serdes_dev_t *serdes)
{
    unsigned int config_mask = 0x0f << 4;
    unsigned int i;
    u8 val;
    int ret;

    if (serdes == NULL) {
        return -EINVAL;
    }

    /*
     * Make sure requested configuration links are detected.
     * The delay is not characterized in the chip manual: wait up
     * to 5 milliseconds.
     */
    for (i = 0; i < MAX_CHECK_COUNT; i++) {
        ret = max9286_read_reg(serdes, 0x49, &val);

        if (ret < 0)
            return -EIO;

        val &= 0xf0;

        if (val == config_mask)
            break;

        usleep_range(500, 600);
    }

    if (val != config_mask) {
        serdes_err(
            "Unable to detect configuration links: 0x%02x expected 0x%02x\n",
            ret, config_mask);

        if (val == 0)
            return -EIO;
    }

    serdes_info(
        "Successfully detected configuration links after %u loops: 0x%02x\n",
        i, config_mask);
    return 0;
}

/*
 * Detect for frame sync lock
 */
int max9286_check_fsync_locked(struct serdes_dev_t *serdes)
{
    u8 val;
    unsigned int i;
    int ret;

    if (serdes == NULL) {
        return -EINVAL;
    }

    for (i = 0; i < MAX_CHECK_COUNT; i++) {
        ret = max9286_read_reg(serdes, 0x31, &val);

        if (ret < 0)
            return -EIO;

        if (val & MAX9286_FSYNC_LOCKED)
            break;

        usleep_range(350, 500);
    }

    if (i == MAX_CHECK_COUNT) {
        serdes_err("Fsync not locked reg 0x%02x\n", val);
        return -EIO;
    }

    serdes_info("Successfully detected frame sync after %u loops: 0x%02x\n",
               i, serdes->channel);
    return 0;
}

int max9286_reverse_channel_setup(struct serdes_dev_t *serdes)
{
    int i = 0;
    serdes_para_t *pPara;

    if (serdes == NULL) {
        return -EINVAL;
    }

    pPara = serdes->priv;

    if (pPara == NULL) {
        return -EINVAL;
    }

    /* enable forward/reverse control channel */
    max9286_write_reg(serdes, 0x0A, DES_REGA_ALLLINK_EN);
    max9286_check_video_links_ex(serdes, 0xf);
    /*
     * configuration link enable
     * this will enable all the conf link of serializers with addr 0x40
     * it is required to set the gmsl1 serializer to CLINKEN and back to
     * SEREN when a valid PCLK is available to avoid iic read/write failure
     * In this case it should be 0x43(CLINKEN)->0x83(SEREN)
     */
    max96705_write_reg(serdes->i2c_client, serdes->addr_ser,
                       0x04, SER_REG4_CFGLINK_EN);
    usleep_range(5000, 6000);
    // dbl, hven should be match with the serdesializer
    max96705_write_reg(serdes->i2c_client, serdes->addr_ser,
                       0x07, SER_REG7_DBL_HV_EN);
    // dbl align
    max96705_write_reg(serdes->i2c_client, serdes->addr_ser,
                       0x67, SER_REG67_DBLALIGN);
    max9286_check_config_link(serdes);

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
        /*
         * enable each forward control channel at one time
         * that is to make sure only one forward channel is available at one
         * time to make the i2c addr remapping
         */
        max9286_write_reg(serdes, 0x0A, 0x01 << i);
        max9286_check_config_link(serdes);
        max96705_map_i2c_addr(serdes->i2c_client,
                              serdes->addr_ser,
                              pPara->remap_addr_ser[i]);
    }

    // at last enable all the forward/reverse control channel
    max9286_write_reg(serdes, 0x0A, DES_REGA_ALLLINK_EN);
    return 0;
}

static serdes_param_reg8_t max9286_init_reg[] = {
    { 0x0A, 0x00, 0 }, /*disable forward/reverse control channel */
    { 0x15, 0x03, 2000 }, // disable csi output
    { 0x01, 0xc3, 0 }, // disable fs
    { 0x1c, 0xf4, 2000 }, // him enable
    { 0x12, 0xf3, 0 }, /* enable dbl double double input mode(2 bits for
                        one cycle)csi-2 output yuv422 8bit */
    { 0x0c, 0x91, 0 }, // use HS/VS encoding
    { 0x63, 0x00, 0 }, // 63, 64 for csi data output
    { 0x64, 0x00, 0 },
    { 0x69, 0x30, 0 }, // auto mask enabled
    { 0x19, 0x80, 0 }, // timing period
};
int max9286_initial_setup(struct serdes_dev_t *serdes)
{
    int ret = 0;
    int i = 0;
    int reglen = ARRAY_SIZE(max9286_init_reg);

    if (serdes == NULL) {
        return -EINVAL;
    }

    // disable poc power during 9286 initial setup
    max20086_power(serdes, false);

    for (i = 0; i < reglen; i++) {
        ret = max9286_write_reg(serdes, max9286_init_reg[i].nRegAddr,
                                max9286_init_reg[i].nRegValue);

        if (ret) {
            serdes_err("max9286 write fail!\n");
            return -EIO;
        }

        if (max9286_init_reg[i].nDelay > 0)
            usleep_range(max9286_init_reg[i].nDelay,
                         max9286_init_reg[i].nDelay + 1000);
    }

    max20086_power(serdes, true);
    return 0;
}

static serdes_param_reg8_t max96705_gmsl_set_reg[] = {
    { 0x43, 0x25, 0 }, // set vs delay
    { 0x45, 0x01, 0 },
    { 0x47, 0x26, 5000 },
    { 0x20, 0x07, 0 }, // set crossbar
    { 0x21, 0x06, 0 },
    { 0x22, 0x05, 0 },
    { 0x23, 0x04, 0 },
    { 0x24, 0x03, 0 },
    { 0x25, 0x02, 0 },
    { 0x26, 0x01, 0 },
    { 0x27, 0x00, 0 },
    { 0x30, 0x17, 0 },
    { 0x31, 0x16, 0 },
    { 0x32, 0x15, 0 },
    { 0x33, 0x14, 0 },
    { 0x34, 0x13, 0 },
    { 0x35, 0x12, 0 },
    { 0x36, 0x11, 0 },
    { 0x37, 0x10, 1000 },
};
int max9286_max96705_gmsl_link_setup(struct serdes_dev_t *serdes)
{
    int i = 0;
    int j = 0;
    int reglen = ARRAY_SIZE(max96705_gmsl_set_reg);
    serdes_para_t *pPara;

    if (serdes == NULL) {
        return -EINVAL;
    }

    pPara = serdes->priv;

    if (pPara == NULL) {
        return -EINVAL;
    }

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
        if (serdes->i2c_client == NULL)
            return -EINVAL;

        for (j = 0; j < reglen; j++) {
            max96705_write_reg(serdes->i2c_client,
                               pPara->remap_addr_ser[i],
                               max96705_gmsl_set_reg[j].nRegAddr,
                               max96705_gmsl_set_reg[j].nRegValue);

            if (max96705_gmsl_set_reg[j].nDelay > 0)
                usleep_range(max96705_gmsl_set_reg[j].nDelay,
                             max96705_gmsl_set_reg[j].nDelay +
                             100);
        }
    }

    // enable input link/vsync from camera
    max9286_write_reg(serdes, 0x0, DES_REG0_INPUTLINK_EN);
    /*
     * fs, manual pclk
     * pclk 72MHz fps 25 the number of pclk in one frame = 72MHz/25 = 2880,000=0x2bf200
     */
    max9286_write_reg(serdes, 0x06, 0x00);
    max9286_write_reg(serdes, 0x07, 0xf2);
    max9286_write_reg(serdes, 0x08, 0x2b);
    usleep_range(10000, 11000);

    for (i = 0; i < MAX_SERAILIZER_NUM; i++) {
        // enable all serial links
        max96705_write_reg(serdes->i2c_client, pPara->remap_addr_ser[i],
                           0x04, 0x83);
        usleep_range(5000, 6000);
    }

    usleep_range(5000, 6000);
    // internal fs, debug for output
    max9286_write_reg(serdes, 0x1, DES_REG1_FS_EN);
    return 0;
}

static void max9286_out_enable(struct serdes_dev_t *serdes, bool enable)
{
    if (serdes == NULL) {
        return ;
    }

    if (enable == true)
        max9286_write_reg(serdes, 0x15, 0x9b);

    if (enable == false)
        max9286_write_reg(serdes, 0x15, 0x13);
}

static void max9286_serdes_power(struct serdes_dev_t *serdes, bool enable)
{
    if (enable) {
        max9286_power(serdes, 0);
        usleep_range(10000, 11000);
        max9286_power(serdes, 1);
        usleep_range(10000, 11000);
    }
    else {
        max9286_power(serdes, 0);
        usleep_range(10000, 11000);
    }
}

static int max9286_fengjing_initialization(struct serdes_dev_t *serdes)
{
    int ret = 0;

    if (serdes == NULL) {
        return -EINVAL;
    }

    ret = max9286_initial_setup(serdes);

    if (ret < 0)
        return ret;

    max9286_reverse_channel_setup(serdes);
    max9286_max96705_gmsl_link_setup(serdes);
    return ret;
}

serdes_para_t max9286_fengjing_MC163_MAX96705_720p = {
    .module_name = "max9286_fengjing_MC163_MAX96705_720p",
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
    .serdes_power = max9286_serdes_power,
    .serdes_init = max9286_fengjing_initialization,
    .serdes_start = max9286_out_enable,
    .serdes_check_link_status = max9286_get_video_links,
    .serdes_check_link_status_ex = max9286_get_link_lock_ex,
    .serdes_check_video_lock_status = max9286_get_video_lock,
};

#endif
