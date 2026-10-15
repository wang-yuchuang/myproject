#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cmsis_os2.h"
#include "CLI_type.h"

#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "irq_num.h"
#include "regs_base.h"
#include "param.h"

#ifdef CONFIG_ARCH_CHIP_X9_SEC
static const uint32_t gpio_sec_irqs[] = {
    GPIO2_GPIO_0_INTR_NUM,
    GPIO2_GPIO_1_INTR_NUM,
    GPIO2_GPIO_2_INTR_NUM,
    GPIO2_GPIO_3_INTR_NUM,
    GPIO2_GPIO_4_INTR_NUM,
    GPIO2_GPIO_5_INTR_NUM,
};

gpio_dev_t g_gpio_sec;
#endif

static void gpio_irq_handler(struct gpio_dev *dev, int pin, void *arg)
{
    printf("gpio_riq_handler: %d\n", pin);
}

/**
 * @brief GPIO read write test.
 *
 * Verified on X9 Ref A02 on Safety & Secure cores. Note that to control
 * a GPIO from Safety or Secure R5, the pin must have been assigned
 * to the GPIO controller of the core, i.e., GPIO_1 for Safety R5, and
 * GPIO_2 for Secure R5. See port_cfg.h
 *
 * > gpio read {pin}
 * > gpio write {pin} {value}
 * > gpio pulse {pin} {count}
 * > gpio irq {pin}
 */
static int gpio_test_main(int argc, char *argv[])
{
    gpio_dev_t *gpio_dev;
    int gpio = atoi(argv[1]);
    int value;

#if defined(CONFIG_ARCH_CHIP_X9_SAF) || defined(CONFIG_ARCH_CHIP_G9_SAF)
    extern gpio_dev_t g_gpio_dev;
    gpio_dev = &g_gpio_dev;
#elif defined(CONFIG_ARCH_CHIP_X9_SEC)
    /* Initialize the GPIO controller. */
    sdrv_gpio_init(&g_gpio_sec, APB_GPIO2_BASE,
                   ARRAY_SIZE(gpio_sec_irqs), gpio_sec_irqs);

    gpio_dev = &g_gpio_sec;
#else
#error Unknown kunlun board!
#endif

    if (!strcmp(argv[0], "read")) {
        gpio_config(gpio_dev, gpio, GPIO_IN);
        value = gpio_get(gpio_dev, gpio);
        printf("GPIO read %d: %d\n", gpio, value);
    } else if (!strcmp(argv[0], "write")) {
        value = atoi(argv[2]);
        printf("GPIO write %d: %d\n", gpio, value);
        gpio_config(gpio_dev, gpio, GPIO_OUT);
        gpio_set(gpio_dev, gpio, !!value);
    } else if (!strcmp(argv[0], "pulse")) {
        value = atoi(argv[2]);
        gpio_config(gpio_dev, gpio, GPIO_OUT);

        while (value--) {
            gpio_set(gpio_dev, gpio, true);
            osDelay(1);
            gpio_set(gpio_dev, gpio, false);
            osDelay(1);
        }
    } else if (!strcmp(argv[0], "irq")) {
        gpio_config(gpio_dev, gpio, GPIO_INTR_BOTH_EDGE);
        gpio_attach_irq(gpio_dev, gpio, gpio_irq_handler, NULL);
    }

    return CLI_FALSE;
}
CLI_CMD("gpio", "GPIO read/write/ISR test", gpio_test_main);

#if defined(CONFIG_ARCH_CHIP_X9_SAF) || defined(CONFIG_ARCH_CHIP_G9_SAF)
static int pinmux_test_main(int argc, char *argv[])
{
    extern pinctrl_t g_pinctrl_safety;
    extern pinctrl_t g_pinctrl_ap;

    int pin = atoi(argv[1]);
    pinctrl_t *pinctrl = pin < 48 ? &g_pinctrl_safety : &g_pinctrl_ap;

    /* Configure the pin as GPIO */
    pinctrl_config(pinctrl, pin, PINCTRL_CONFIG_FUNCTION, PAD_MUX_FUNC_GPIO);

    return CLI_FALSE;
}
CLI_CMD("pinmux", "pinmux test", pinmux_test_main);
#endif
