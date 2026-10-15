/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/
#include "lp875xx.h"
#include "debug.h"
#include <assert.h>
#include "string.h"
#include <board.h>

#define LP876XX_MAX_BUCKS 4

const static int vol_reg[] = {0x0a, 0x0c, 0x0e, 0x10};
const static int pin_ctrl_reg[] = {0x02, 0x04, 0x06, 0x08};
const static int cur_reg[] = {0x25, 0x26, 0x27};/* buck id, curr high bits, curr low bits*/
const static int cur_reg_mask[] = {0xff, 0x3, 0xff};

static int find_vol_reg_by_index(int index)
{
    return vol_reg[index];
}

static uint32_t convert_val_to_vol(uint8_t val)
{
    if (val >= 0xa && val <= 0x18)
        return (val - 0xa) * 10 + 600;
    else if (val > 0x18 && val <= 0x9e)
        return (val - 0x18) * 5 + 735;
    else if (val > 0x9e && val <= 0xff)
        return (val - 0x9e) * 20 + 1420;
    else
        ASSERT(0);
    return 0;
}

static uint8_t convert_vol_to_val(uint32_t mv)
{
    if (mv >= 600 && mv <= 730) {
        return 0xa + ((mv - 600) + 9) / 10;
    }
    else if (mv > 730 && mv <= 1400) {
        return 0x18 + ((mv - 735) + 4) / 5;
    }
    else if (mv > 1400 && mv <= 3360) {
        return 0x9e + ((mv - 1420) + 19) / 20;
    }
    else {
        ASSERT(0);
    }
    return 0;
}

static int find_pin_ctrl_reg_by_index(int index)
{
    return pin_ctrl_reg[index];
}

static uint8_t update_pin_ctrl_to_val(uint8_t val, int pin_idx,
                                      bool isctrl)
{
    if (!isctrl) {
        val &= ~(1 << 6);
        return val;
    }

    val |= 1 << 6;
    val &= ~(0x3 << 4);
    val |= (pin_idx & 0x3) << 4;
    return val;
}

static uint8_t update_fpwm_to_val(uint8_t val, bool ispwm)
{
    if (ispwm) {
        val |= 1 << 1;
    }
    else {
        val &= ~(1 << 1);
    }

    return val;
}

static bool lp875xx_set_vol(struct pmic_dev_t *dev, int index, int mv)
{
    uint8_t reg = find_vol_reg_by_index(index);
    uint8_t val = convert_vol_to_val(mv);

    if (i2c_write_reg8(dev->i2c_adap, dev->slaveaddr, reg, &val, 1)) {
        uint8_t rdata = 0;
        i2c_read_reg8(dev->i2c_adap, dev->slaveaddr, reg, &rdata, 1);

        if (rdata == val)
            return true;
        else
            return false;
    }

    return false;
}
static uint32_t lp875xx_get_vol(struct pmic_dev_t *dev, int index)
{
    uint8_t reg = find_vol_reg_by_index(index);
    uint8_t data = 0;

    if (i2c_read_reg8(dev->i2c_adap, dev->slaveaddr, reg, &data, 1)) {
        return convert_val_to_vol(data);
    }

    return 0;
}

static bool lp875xx_set_fpwm(struct pmic_dev_t *dev, int index, bool ispwm)
{
    uint8_t reg = find_pin_ctrl_reg_by_index(index);
    uint8_t val = 0;

    i2c_read_reg8(dev->i2c_adap, dev->slaveaddr, reg, &val, 1);
    val = update_fpwm_to_val(val, ispwm);
    i2c_write_reg8(dev->i2c_adap, dev->slaveaddr, reg, &val, 1);

    return true;
}

bool lp875xx_set_power_ctrl_by_pin(struct pmic_dev_t *dev, int index,
                                   int pin_idx, bool isctrl)
{
    uint8_t reg = find_pin_ctrl_reg_by_index(index);
    uint8_t val = 0;

    i2c_read_reg8(dev->i2c_adap, dev->slaveaddr, reg, &val, 1);
    val = update_pin_ctrl_to_val(val, pin_idx, isctrl);
    i2c_write_reg8(dev->i2c_adap, dev->slaveaddr, reg, &val, 1);

    return true;
}

uint32_t lp875xx_get_cur(struct pmic_dev_t *dev, int index)
{
    uint8_t data = index;
    uint32_t curr = 0;
    i2c_write_reg8(dev->i2c_adap, dev->slaveaddr, cur_reg[0], &data, 1);
    udelay(100);
    i2c_read_reg8(dev->i2c_adap, dev->slaveaddr, cur_reg[1], &data, 1);
    curr = data & cur_reg_mask[1];
    curr <<= 8;
    i2c_read_reg8(dev->i2c_adap, dev->slaveaddr, cur_reg[2], &data, 1);
    curr |= data & cur_reg_mask[2];
    return curr * 20;
}

bool pmic_dev_add_lp875xx(struct pmic_dev_t *dev, i2c_adap_dev_t *i2c_bus,
                          int slaveaddr, uint8_t bucks)
{
    if (!dev)
        return false;

    memset(dev, 0, sizeof(struct pmic_dev_t));
    dev->i2c_adap = i2c_bus;
    dev->slaveaddr = slaveaddr;
    dev->max_item = bucks > LP876XX_MAX_BUCKS ? LP876XX_MAX_BUCKS : bucks;
    dev->set_vol = lp875xx_set_vol;
    dev->get_vol = lp875xx_get_vol;
    dev->get_cur = lp875xx_get_cur;
    dev->set_power_ctrl_by_pin = lp875xx_set_power_ctrl_by_pin;
    dev->set_fpwm = lp875xx_set_fpwm;
    return true;
}

void pmic_config_before_str(void)
{
    int i = 0;
    struct power_item_t *item;

    if (g_power_configs == NULL || g_power_item_count == 0) {
        ssdk_printf(SSDK_ERR, "%s : g_power_configs or g_power_item_count is null", __func__);
        return;
    }

    for (i = 0; i < g_power_item_count; i++) {
        item = &g_power_configs[i];
        if (item->vdd_id == VDDQ_1V1) {
            if (lp875xx_set_fpwm(item->dev, item->index, false) != true)
                ssdk_printf(SSDK_ERR, "%s : lp875xx set fpwm error", __func__);
        }
    }

    for (i = 0; i < g_power_item_count; i++) {
        item = &g_power_configs[i];
        if (item->vdd_id == VDDQ_1V1) {
            if (lp875xx_set_power_ctrl_by_pin(item->dev, item->index, LP875XX_PIN_EN1, false) != true)
                ssdk_printf(SSDK_ERR, "%s : lp875xx set power ctrl by pin error", __func__);
        }
    }
}