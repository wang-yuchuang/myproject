/**
 * @file disp_panels.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <disp_hal.h>
#include "disp_panels.h"
#ifdef CONFIG_DS90UB9XX
#include "ds90ub9xx.h"
#endif

#ifdef CONFIG_ADV7511
const unsigned char adv_init_table[] = {
  0x41, 0x00,
  0x98, 0x03,
  0x9A, 0xE0,
  0x9C, 0x30,
  0x9D, 0x61,
  0xA2, 0xA4,
  0xA3, 0xA4,
  0xE0, 0xD0,
  0x55, 0x12,
  0xF9, 0x00,
  0x9D, 0x61,
  0x15, 0x00,
  0x48, 0x00,
  0x16, 0x36,
  0x17, 0x00,
  0x18, 0x00,
  0xAF, 0x06,
  0x4C, 0x00,
  0x40, 0x00,
  0x00, 0x00
};
#endif

/* if support dual link through read second 941 check */
bool is_need_checklink(i2c_adap_dev_t *adap, uint32_t slave_addr, uint8_t reg)
{
    uint8_t val = 0;

    static bool checkinit = false;
    static bool result = false;

    if (checkinit)
        return result;

#ifdef CONFIG_DS90UB9XX
    int ret = -1;

    ret = ds90ub9xx_read_reg(adap, slave_addr, reg, &val);
    if (ret < 0) {
        LOGE("sec deserdes don't exit\n");
        result = false;
        goto out;
    }
#endif

    if (val == 0)
        result = false;
    else
        result = true;

    LOGD("link check:%d val:0x%x\n", result, val);

out:
    checkinit = true;   // indicate has inited

    return result;
}

int panel_check_link(struct sdm_panel_info * panel_info)
{
    int ret = 0;

    LOGD("%s enter\n", __func__);

#ifdef CONFIG_DS90UB9XX
    i2c_adap_dev_t *adap = (i2c_adap_dev_t *)panel_info->serdes->adap;
    uint32_t ser_addr = panel_info->serdes->ser_addr;
    uint32_t des_addr = panel_info->serdes->des_addr;
    struct sdm_panel *panel = panel_info->panel;

    ds90ub9xx_write_reg(adap, ser_addr, 0x03, 0x08); //I2C Pass-through Port0/Port1

    ds90ub9xx_write_reg(adap, ser_addr, 0x17, 0x9e); //config I2C Pass All

    ret = ds90ub94x_wait_linklock(adap, ser_addr);
    if (ret)
        LOGD("wait link lock timeout\n");

    ret = ds90ub94x_read_gpio_reg(adap, des_addr, GPIO6_REG);
    if (ret == 1) {
        panel->fps = 60;
    }
    else {
        panel->fps = 56;
    }
    LOGD("read gpio%d -> 0x%x fps:%d\n", GPIO6_REG, ret, panel->fps);
#endif
    LOGD("%s down\n", __func__);

    return ret;
}


#ifdef CONFIG_ADV7511
int adv_init(unsigned char bus_id)
{
    int i = 0;
    unsigned char val;
    unsigned short Rev0;

    HAL_I2CBusReadByte(bus_id, TXDEV_MAIN_MAP,
            TXHAL_REG_OFFSET(0, 0x00),
            (unsigned char *)&Rev0);
    LOGD("adv chip revision = %x\n", Rev0);

    while (adv_init_table[i]) {
        HAL_I2CBusWriteByte(bus_id, TXDEV_MAIN_MAP,
                TXHAL_REG_OFFSET(0, adv_init_table[i]),
                adv_init_table[i+1]);
        HAL_I2CBusReadByte(bus_id, TXDEV_MAIN_MAP,
                TXHAL_REG_OFFSET(0, adv_init_table[i]),
                &val);
        i += 2;
    }

    return 0;
}
#endif

int send_i2c_command(struct sdm_panel *panel) {
    LOGD("request a i2c handle");
    LOGD("send i2c command %p\n", panel->cmd_data);
    LOGD("release i2c handle");
    return -1;
}

int send_spi_command(struct sdm_panel *panel) {
    LOGD("send spi command %p\n", panel->cmd_data);
    return -1;
}

int sdm_connect_panel(display_handle *handle, int sub_id, struct sdm_panel *panel)
{
    int ret = hal_sdm_panel_connect(handle, sub_id, panel);

    switch (panel->if_type) {
        case IF_TYPE_HDMI:
#ifdef CONFIG_ADV7511
            ret = adv_init(sub_id);
#endif
            break;
        case IF_TYPE_LVDS:
        case IF_TYPE_DSI:
        case IF_TYPE_PARALLEL:
            break;
        default:
            LOGD("invalid panel if_type: %d\n", panel->if_type);
            return -2;
    }

    if (ret) {
        LOGD("panel init failed in driver: %d\n", ret);
        return -1;
    }

    switch (panel->cmd_intf)
    {
    case IF_TYPE_I2C:
        return send_i2c_command(panel);
    case IF_TYPE_SPI:
        return send_spi_command(panel);
    default:
        break;
    }

    //LOGD("panel %s conncted success\n", panel->panel_name);

    return 0;
}

int sdm_panel_probe(display_handle *handle, int sub_id, struct sdm_panel_info *panels[], int n_panels)
{
    int found = -1;

    for (int i = 0; i < n_panels; i++)
    {
        struct sdm_panel *p = panels[i]->panel;
        //LOGD("check panel %s...\n", p->panel_name);
        if (0 == sdm_connect_panel(handle, sub_id, p)) {
            found = i;
            break;
        }
    }

    if (found == -1) {
        LOGD("can not found a valid panel");
        return -1;
    }

    return found;
}

int sdm_panel_disconnect(display_handle *handle, int sub_id, struct sdm_panel *panel)
{
    return hal_sdm_panel_disconnect(handle, sub_id, panel);
}

int sdm_panel_default_pwmbl_init(struct backlight_info *backlight, int dispid)
{
    int ret = 0;

    LOGD("%s enter\n", __func__);
#ifdef CONFIG_PWM

    if (!backlight->base) {
        LOGD("disp[%d] base is not configured,or not required\n", dispid);
        return -1;
    }

    if (!backlight->pwm_cfg) {
        LOGD("disp[%d] pwm_cfg is not configured,or not required\n", dispid);
        return -1;
    }

    if (!backlight->pwm_ins) {
        LOGD("disp[%d] pwm_ins is not configured,or not required\n", dispid);
        return -1;
    }

    if (backlight->priv) {
#ifdef CONFIG_DS90UB9XX
        struct serdes_info *serdes = (struct serdes_info *)backlight->priv;
        i2c_adap_dev_t *adap = (i2c_adap_dev_t *)serdes->adap;
        uint32_t des_addr = serdes->des_addr;
        uint32_t ser_addr = serdes->ser_addr;

        drv_pwm_simple_cmp_cfg_t *blcmp_cfg = &backlight->pwm_cfg->cmp_cfg[backlight->pwm_chn];
        ret = ds90ub94x_948_gpio_config(adap, ser_addr, des_addr, GPIO0, OUTPUT, DES_PORT0);
        if (ret < 0)
            LOGE("%s backlight GPIO0 config error !\n", __func__);

        ret = ds90ub94x_read_gpio_reg(adap, des_addr, GPIO6_REG);
        if (ret == 1)
            blcmp_cfg->phase = DRV_PWM_PHASE_POLARITY_POS;
        else
            blcmp_cfg->phase = DRV_PWM_PHASE_POLARITY_NEG;
#endif
    }

    pwm_instance_t *pwm_ins = backlight->pwm_ins;

    drv_pwm_instance_cfg_t pwm_ins_cfg = {
        .base = backlight->base,
        .phy_num = backlight->pwm_cfg->phy_num,
    };

    pwm_ins->occupied = 0;
    drv_pwm_instance_init(pwm_ins, &pwm_ins_cfg);
    //panel_check_phase(backlight->pwm_cfg);
    drv_pwm_simple_init(pwm_ins->pwmc, backlight->pwm_cfg,
        &pwm_ins->pwm_simple_ctx);
    drv_pwm_cmp_out_start(pwm_ins->pwmc);
#endif

    LOGD("%s exit\n", __func__);

    return ret;
}

