#include <cmsis_os2.h>
#include <debug.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <param.h>
#include <pinmux_cfg.h>
#include <regs_base.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "CLI.h"
#include "driver/taishan/epwm/sdrv_epwm_drv.h"
#include "driver/taishan/xtrg/sdrv_xtrg_drv.h"
#include "irq_num.h"
#include "pwm_cfg.h"
#include "reg.h"

extern pwm_dev_t *g_pwm_1;
extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;

#define GPIO_INSTANCE(pin)                                                     \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

#define PINCTRL_INSTANCE(pin)                                                  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

static xtrg_dev_t xtrg_dev_hfre = {
    .base = APB_XTRG_BASE,
    .irq = XTRG_FUNC_INTR_NUM,
};

static pwm_state_t state_1 = {
    .period = 2000000000,
    .duty = 1000000000,
};

static int analog_out_test(int argc, char *argv[])
{
    pinctrl_t *pinctrl;
    xtrg_config_t xtrg_config;

    int pin = atoi(argv[0]);
    int xTRG_IO = atoi(argv[1]);

    pinctrl = PINCTRL_INSTANCE(pin);
    pinctrl_config(pinctrl, pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT5);

    memset(&xtrg_config, 0, sizeof(xtrg_config));
    xtrg_config.sig_i = EPWM_CMP0(0);
    xtrg_config.sig_o = IO_o(xTRG_IO);
    sdrv_xtrg_io_out_config(&xtrg_dev_hfre, &xtrg_config, true);

    pwm_config(g_pwm_1, &state_1);
    pwm_start(g_pwm_1);

    return CLI_FALSE;
}

CLI_CMD("analog_out_demo", "\r\nanalog_out_test demo\r\n", analog_out_test);
