/*
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 */

#include "sdrv_serdes_core.h"
#ifdef CONFIG_dvp_max96706_fengjing_MC163_MAX96705
#include "tca_gpio_cfg.h"

#define MAX96706_CHECK_LINK_COUNT (100)
#define MAX96706_DEVICE_ID 0x4A
#define MAX96705_DEVICE_ID 0x41

static int max96706_fengjing_initialization(struct serdes_dev_t *serdes);
static void max96706_output_enable(struct serdes_dev_t *serdes, bool enable);
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

static int max96706_write_reg(struct serdes_dev_t *serdes, u8 reg, u8 val)
{
    void *client = serdes->i2c_client;
    u8 addr = serdes->addr_des;
    return serdes_write_reg_8(client, addr, reg, val);
}

static int max96706_read_reg(struct serdes_dev_t *serdes, u8 reg, u8 *val)
{
    void *client = serdes->i2c_client;
    u8 addr = serdes->addr_des;
    return serdes_read_reg_8(client, addr, reg, val);
}

int max96706_get_lock_status(struct serdes_dev_t *serdes)
{
    u8 val = 0;

    max96706_read_reg(serdes, 0x04, &val);
    val = (val >> 7) & 0x1;
    return val & serdes->serdes_streamon_bitmap;
}

static int max96706_get_lock_status_ex(struct serdes_dev_t *serdes)
{
    u8 val = 0;
    u8 link_status = 0;
    int ret = 0;
    int i;

    serdes_para_t *pPara;

    if (serdes == NULL) {
        serdes_err("no serdes pointer\n");
        return -EINVAL;
    }

    pPara = (serdes_para_t *)serdes->priv;

    if (pPara == NULL) {
        serdes_err("no para\n");
    }
    // If fail, retry
    i = 0;
    do {
        ret = max96705_read_reg(serdes->i2c_client, pPara->remap_addr_ser[0], 0x1e, &val);
        if (ret < 0) {
            i++;
            serdes_err("%s:chip 0x%02x error: reg=0x%02x ret %d,retry:%d\n",
                __func__, pPara->remap_addr_ser[0], 0x1e, ret, i);
            usleep_range(10000,12000);
        }
    } while (ret < 0 && i < 3);

    if (val != MAX96705_DEVICE_ID) {
        max96706_read_reg(serdes, 0x04, &val);
        link_status = (val >> 7) & 0x1;

        if (link_status == 1) {//link status is locked,reinit
            max96706_fengjing_initialization(serdes);

            if (serdes->serdes_status.stream_cnt > 0) {
                max96706_output_enable(serdes, 1);
            }
        }
    }
    else {
        link_status = 1;
    }

    return link_status;
}

static void max96706_output_enable(struct serdes_dev_t *serdes, bool enable)
{
    u8 val = 0;

    max96706_read_reg(serdes, 0x04, &val);

    if (enable == true) {
        val &= ~(0x40); // enable output. bit 6 set 0 to enable
    }
    else if (enable == false) {
        val |= 0x40;
    }

    max96706_write_reg(serdes, 0x04, val);
    usleep_range(1000, 1100);
}

//extern const domain_res_t g_gpio_res;

void max96706_power(struct serdes_dev_t *serdes, bool enable)
{
    //int ret = 0;
    //void *dio_handle = NULL;

    gpio_dev_t *gpio_dev;
    extern gpio_dev_t g_gpio_dev;
    gpio_dev = &g_gpio_dev;

    if (serdes->power_gpio < 0) {
        printf("%s pwdn_gpio invalid, gpio:%d\n", __func__,
               serdes->power_gpio);
        return;
    }

	gpio_config(gpio_dev, serdes->power_gpio, GPIO_OUT);
	gpio_set(gpio_dev, serdes->power_gpio, !!enable);

    usleep_range(5000, 6000);
}

void max96706_poc_power(struct serdes_dev_t *serdes, bool enable)
{
	gpio_config(&g_gpio_tca9539, serdes->poc_gpio, GPIO_OUT);
	gpio_set(&g_gpio_tca9539, serdes->poc_gpio,enable);

    usleep_range(5000, 6000);
}

void max96706_gpi_enable(struct serdes_dev_t *serdes, bool enable)
{

    gpio_config(&g_gpio_tca9539, serdes->gpi_gpio, GPIO_OUT);
	gpio_set(&g_gpio_tca9539, serdes->gpi_gpio,enable);

    usleep_range(5000, 6000);
}

int max96706_check_chip_id(struct serdes_dev_t *serdes)
{
    int ret = 0;
    u8 chip_id = 0;

    max96706_power(serdes, 1);
    usleep_range(10000, 11000);
    ret = max96706_read_reg(serdes, 0x1e, &chip_id);
    serdes_err("chip id 0x%x\n", chip_id);

    if (ret) {
        serdes_err("%s: failed to read chip identifier\n", __func__);
    }

    if (chip_id != MAX96706_DEVICE_ID) {
        serdes_err(
            "%s: wrong chip identifier, expected 0x%x(max96706), got 0x%x\n",
            __func__, MAX96706_DEVICE_ID, chip_id);
        ret = -ENXIO;
    }

    return ret;
}

static void max96706_serdes_power(struct serdes_dev_t *serdes, bool enable)
{
    if (enable) {
        max96706_power(serdes, 0);
        usleep_range(10000, 11000);
        max96706_power(serdes, 1);
        usleep_range(10000, 11000);
    }
    else {
        max96706_power(serdes, 0);
        usleep_range(10000, 11000);
    }
}


static int max96706_fengjing_initialization(struct serdes_dev_t *serdes)
{
    int ret = 0;
    int loop = 0;
    u8 val;
    u8 chip_id;

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

    ret = max96706_read_reg(serdes, 0x1e, &chip_id);
    if (ret) {
        serdes_err("%s: failed to read chip identifier\n", __func__);
        return -EINVAL;
    }
    if (chip_id != MAX96706_DEVICE_ID) {
        serdes_err(
            "%s: wrong chip identifier, expected 0x%x(max96706), got 0x%x\n",
            __func__, MAX96706_DEVICE_ID, chip_id);
        return -EINVAL;
    }

    max96706_poc_power(serdes, 0);
    // set him
    max96706_read_reg(serdes, 0x06, &val);
    val |= 0x80;
    max96706_write_reg(serdes, 0x06, val);

    //[7]dbl, [2]hven, [1]cxtp
    max96706_write_reg(serdes, 0x07, 0x86);

    // invert hsync
    max96706_read_reg(serdes, 0x02, &val);
    val |= 0x80;
    max96706_write_reg(serdes, 0x02, val);

    // disable output enable forward/reverse control channel
    max96706_read_reg(serdes, 0x04, &val);
    val |= 0x47;
    max96706_write_reg(serdes, 0x04, val);

    max96706_poc_power(serdes, 1);

    for (loop = 0; loop < MAX96706_CHECK_LINK_COUNT; loop++) {
        max96706_read_reg(serdes, 0x04, &val);
        val = (val >> 7) & 0x1;

        if (val == 1)
            break;

        usleep_range(1000, 1100);
        max96706_poc_power(serdes, 1);
    }

    if (val != 1) {
        serdes_err("wait lock status fail\n");
        return -1;
    }
    // driver current selection
    max96706_read_reg(serdes, 0x5, &val);
    serdes_info("96706, reg=0x5, val=0x%x\n", val);
    val |= 0x40;
    max96706_write_reg(serdes, 0x5, val);
    usleep_range(10000, 11000);

    ret = max96705_check_chip_id(serdes->i2c_client, serdes->addr_ser);

    if (ret != 0) {
        serdes_err("%s: not found 96705, ret=%d\n", __func__, ret);
        return -1;
    }

    max96705_map_i2c_addr(serdes->i2c_client, serdes->addr_ser,
                          pPara->remap_addr_ser[0]);
    usleep_range(2000, 3000);
    max96705_write_reg(serdes->i2c_client, pPara->remap_addr_ser[0], 0x01,
                       (serdes->addr_des) << 1);
    usleep_range(10000, 11000);
    max96706_gpi_enable(serdes, 1);
    if (serdes->gpi_gpio == 0) {
        val = 0;
        max96705_read_reg(serdes->i2c_client, pPara->remap_addr_ser[0], 0x0f, &val);
        val |= 0x81;
        max96705_write_reg(serdes->i2c_client, pPara->remap_addr_ser[0], 0x0f, val);
        usleep_range(100, 200);
    }

    // enable dbl, hven
    max96705_write_reg(serdes->i2c_client, pPara->remap_addr_ser[0], 0x07, 0x84);

    return 0;
}


//extern const domain_res_t g_gpio_res;
//extern const domain_res_t g_iomuxc_res;
extern void max96706_poc_power(struct serdes_dev_t *serdes, bool enable);
extern void max96706_gpi_enable(struct serdes_dev_t *serdes, bool enable);

// pwdn-gpios = <&port4c 19 0>;
// Power for Rx max96706/712
static int max96706_s_power(struct vdev_device *vdev, int enable)
{
    struct serdes_dev_t *serdes;
    serdes = (struct serdes_dev_t *)vdev->priv_data;

    if (vdev == NULL) {
        return -1;
    }

    if (enable == 1) {
        max96706_fengjing_initialization(serdes);
    }
    else {
        max96706_poc_power(serdes, 0);
        max96706_gpi_enable(serdes, 0);
    }

    serdes_info("%s: enable=%d. done\n", __func__, enable);

    return 0;
}

serdes_para_t max96706_fengjing_MC163_MAX96705_720p = {
    .module_name = "max96706_fengjing_MC163_MAX96705_720p",
    .bus_type = VDEV_MBUS_PARALLEL2,
    .numerator = 1,
    .colorspace = VDEV_COLORSPACE_SRGB,
    .field = VDEV_FIELD_NONE,

    //these fields are per camera
    .mbus_code[0] = VDEV_MBUS_FMT_UYVY,
    .fps[0] = 25,
    .remap_addr_ser[0] = 0x4c,

    .serdes_power = max96706_serdes_power,
    .serdes_init = max96706_fengjing_initialization,
    .serdes_start = max96706_output_enable,
    .serdes_check_link_status = max96706_get_lock_status,
    .serdes_check_link_status_ex = max96706_get_lock_status_ex,
};

#endif

