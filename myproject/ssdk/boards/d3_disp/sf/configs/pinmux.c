#ifdef CONFIG_PINCTRL

#include <types.h>
#include <param.h>
#include <board.h>
#include <driver/pinctrl.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/gpio.h>
#include <driver/taishan/gpio/taishan_gpio.h>
#include <driver/taishan/dispss/lvds_reg.h>
#include <driver/taishan/dispss/disp_reg_rw.h>
#include "pinmux_cfg.h"

static taishan_pinctrl_t pinctrl_safety_dev = {
    .base            = APB_IOMUXC_SF_BASE,
    .pad_reg_offset  = TAISHAN_PAD_CONFIG_REG_OFFSET,
    .mux_reg_offset  = TAISHAN_MUX_CONFIG_REG_OFFSET,
};

static taishan_pinctrl_t pinctrl_ap_dev = {
    .base            = APB_IOMUXC_AP_BASE,
    .pad_reg_offset  = TAISHAN_PAD_CONFIG_REG_OFFSET,
    .mux_reg_offset  = TAISHAN_MUX_CONFIG_REG_OFFSET,
};

static taishan_gpio_dev_t gpio_safety_dev = {
    .base      = APB_GPIO_SF_BASE,
    .pin_start = TAISHAN_SAFETY_PIN_START,
    .pin_num   = TAISHAN_SAFETY_PIN_NUM,
    .irqs      = {
        GPIO_SF_SYNC_DGPIO_INTR_NUM,
        GPIO_SF_SYNC_GRP_0_INTR_NUM,
        GPIO_SF_SYNC_GRP_1_INTR_NUM,
        GPIO_SF_SYNC_GRP_2_INTR_NUM,
        GPIO_SF_SYNC_GRP_3_INTR_NUM,
        GPIO_SF_SYNC_GRP_4_INTR_NUM,
    },
};

static taishan_gpio_dev_t gpio_ap_dev = {
    .base      = APB_GPIO_AP_BASE,
    .pin_start = TAISHAN_AP_PIN_START,
    .pin_num   = TAISHAN_AP_PIN_NUM,
    .irqs      = {
        GPIO_AP_SYNC_DGPIO_INTR_NUM,
        GPIO_AP_SYNC_GRP_0_INTR_NUM,
        GPIO_AP_SYNC_GRP_1_INTR_NUM,
        GPIO_AP_SYNC_GRP_2_INTR_NUM,
        0,
        0,
    },
};

pinctrl_t g_pinctrl_safety = {
    .pin_start = TAISHAN_SAFETY_PIN_START,
    .pin_nr = TAISHAN_SAFETY_PIN_NUM,
};

pinctrl_t g_pinctrl_ap = {
    .pin_start = TAISHAN_AP_PIN_START,
    .pin_nr = TAISHAN_AP_PIN_NUM,
};

gpio_dev_t g_gpio_safety;

gpio_dev_t g_gpio_ap;

static void board_port_disp_mux_config_clr(void)
{
    paddr_t base = APB_DISP_MUX_BASE;
    volatile unsigned int val;

    for (int i = 0; i < 5; i++) {
        val = disp_read(base, LVDS_CH0_PAD_SET_(i));
        val = reg_value(0, val, CH0_RTERM_EN_SHIFT, CH0_RTERM_EN_MASK);
        val = reg_value(0, val, CH0_TXEN_SHIFT, CH0_TXEN_MASK);
        val = reg_value(0, val, CH0_RXEN_SHIFT, CH0_RXEN_MASK);
        disp_write(base, LVDS_CH0_PAD_SET_(i), val);
    }

    val = disp_read(base, LVDS_CH0_PAD_COM_SET_(0));
    val = reg_value(0, val, CH0_RTERM_SHIFT, CH0_RTERM_MASK);
    disp_write(base, LVDS_CH0_PAD_COM_SET_(0), val);
}

static gpio_type_e get_gpio_type_from_intcfg(pin_interrupt_config_e intcfg)
{
    gpio_type_e gpio_type;

    switch (intcfg) {
        case PIN_INTERRUPT_DISABLED:
            gpio_type = GPIO_IN;
            break;
        case PIN_INTERRUPT_HIGH_LEVEL:
            gpio_type = GPIO_INTR_HIGH_LEVEL;
            break;
        case PIN_INTERRUPT_LOW_LEVEL:
            gpio_type = GPIO_INTR_LOW_LEVEL;
            break;
        case PIN_INTERRUPT_RISING_EDGE:
            gpio_type = GPIO_INTR_RISING_EDGE;
            break;
        case PIN_INTERRUPT_FALLING_EDGE:
            gpio_type = GPIO_INTR_FALLING_EDGE;
            break;
        case PIN_INTERRUPT_BOTH_EDGE:
            gpio_type = GPIO_INTR_BOTH_EDGE;
            break;
        default:
            gpio_type = GPIO_IN;
            break;
    }

    return gpio_type;
}

static void do_port_init(pin_settings_config_t *settings, unsigned int num)
{
    bool lvds_pin_used = false;

    for (unsigned int i = 0; i < num; i++) {
        pin_settings_config_t *config = &settings[i];
        pinctrl_t *pinctrl = (config->pin_index < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap;
        gpio_dev_t *gpio = (config->pin_index < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap;

        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FUNCTION, config->mux);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE, config->open_drain);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_PULL, config->pull_config);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_DRIVE_STRENGTH, config->drive_strength);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_SLEW_RATE, config->slew_rate);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_INPUT_SCHMITT, config->input_select);
        pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_FORCE_INPUT, config->force_input);

        if (config->mux == PIN_MUX_ALT0) {
            gpio_type_e gpio_type = GPIO_OUT;

            if (config->data_direction == PIN_INPUT_DIRECTION) {
                gpio_type = get_gpio_type_from_intcfg(config->interrupt_config);
            }

            gpio_config(gpio, config->pin_index, gpio_type);
            gpio_set(gpio, config->pin_index, config->initial_value);
        }
        else if ((config->pin_index >= GPIO_L0) && (config->pin_index <= GPIO_L9)
                 && (config->mux == PIN_MUX_ALT1)) {
            lvds_pin_used = true;
        }

        if ((config->mode_select != PIN_MODE_DIGITAL) &&
            (config->pin_index >= GPIO_A0) && (config->pin_index <= GPIO_C15)) {
            if (config->mode_select == PIN_MODE_SELECT_NORMAL) {
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_MODE_SELECT, PINCTRL_MODE_SELECT_NORMAL);
            }
            else if (config->mode_select == PIN_MODE_DOWN_CONVERSION) {
                pinctrl_config(pinctrl, config->pin_index, PINCTRL_CONFIG_MODE_SELECT, PINCTRL_MODE_SELECT_DOWN_COVERSION);
            }
        }
    }

    if (!lvds_pin_used) {
        board_port_disp_mux_config_clr();
    }
}

void board_port_early_init(void)
{
    taishan_pinctrl_init(&g_pinctrl_safety, &pinctrl_safety_dev);
    taishan_pinctrl_init(&g_pinctrl_ap, &pinctrl_ap_dev);
    taishan_gpio_early_init(&g_gpio_safety, &gpio_safety_dev);
    taishan_gpio_early_init(&g_gpio_ap, &gpio_ap_dev);

#if NUM_OF_CONFIGURED_PINS > 0
    do_port_init(g_pin_init_config, NUM_OF_CONFIGURED_PINS);
#endif
}

void board_port_init(void)
{
    taishan_gpio_init(&g_gpio_safety, &gpio_safety_dev);
    taishan_gpio_init(&g_gpio_ap, &gpio_ap_dev);
}

#endif /* CONFIG_PINCTRL */
