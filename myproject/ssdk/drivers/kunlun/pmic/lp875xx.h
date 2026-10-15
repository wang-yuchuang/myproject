/********************************************************
 *          Copyright(c) 2020   Semidrive               *
 ********************************************************/

#ifndef __LP_875XX_H
#define __LP_875XX_H
#include <stdbool.h>
#include <types.h>
#include <driver/i2c.h>
#include <cmsis_os2.h>
#include <pmic_cfg.h>

#define LP875XX_BUCK0_IDX   0
#define LP875XX_BUCK1_IDX   1
#define LP875XX_BUCK2_IDX   2
#define LP875XX_BUCK3_IDX   3

#define LP875XX_PIN_EN1 0
#define LP875XX_PIN_EN2 1
#define LP875XX_PIN_EN3 2

enum vdd_id_e {
    VDDSFT_0V8,
    VDDSFT_1V8,
    VDD_AP,
    VDD_GPU,
    VDD_CPU,
    VDDQ_1V1,
    VDDIO_1V8,
    VDDQLP_0V6,
    VDD_ID_MAX,
};

struct power_item_t {
    enum vdd_id_e vdd_id;
    struct pmic_dev_t *dev;
    int index;
};

struct pmic_dev_t {
    void *handle;
    i2c_adap_dev_t *i2c_adap;
    int slaveaddr;
    int max_item;
    void *priv_data;
    bool (*init)(struct pmic_dev_t *dev);
    bool (*set_vol)(struct pmic_dev_t *dev, int index, int mv);
    uint32_t (*get_vol)(struct pmic_dev_t *dev, int index);
    uint32_t (*get_cur)(struct pmic_dev_t *dev, int index);
    bool (*set_power_ctrl_by_pin)(struct pmic_dev_t *dev, int index,
                                  int pin_idx, bool isctrl);
    bool (*set_fpwm)(struct pmic_dev_t *dev, int index, bool ispwm);
    bool (*deinit)(struct pmic_dev_t *dev);
};

bool pmic_dev_add_lp875xx(struct pmic_dev_t *dev, i2c_adap_dev_t *i2c_bus,
                          int slaveaddr, uint8_t bucks);
bool lp875xx_set_power_ctrl_by_pin(struct pmic_dev_t *dev, int index,
                                   int pin_idx, bool isctrl);
void pmic_config_before_str(void);
#endif
