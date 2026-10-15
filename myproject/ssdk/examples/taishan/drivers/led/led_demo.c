/**
 * file: led_demo.c
 *
 * Description: LED1302 on/off/flash
 *
 */
#include <CLI_type.h>
#include <board.h>
#include <cmsis_os2.h>
#include <driver/gpio.h>
#include <driver/irq.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/btm/sdrv_btm_drv.h>
#include <driver/timer.h>
#include <pinmux_cfg.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static sdrv_btm_dev_t g_sdrv_btm_timer1;
static timer_dev_t *timer1;
static int timer1_cnt;
static osSemaphoreId_t timer_sem;

extern gpio_dev_t g_gpio_safety;
extern gpio_dev_t g_gpio_ap;

static int led_pin;

#define GPIO_INSTANCE(pin)                                                     \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_gpio_safety : &g_gpio_ap)

extern pinctrl_t g_pinctrl_safety;
extern pinctrl_t g_pinctrl_ap;

#define PINCTRL_INSTANCE(pin)                                                  \
    (((pin) < TAISHAN_SAFETY_PIN_NUM) ? &g_pinctrl_safety : &g_pinctrl_ap)

static const sdrv_btm_cfg_t timer1_config = {
    .base = DEVICE_BASE(BTM2),
    .irq = DEVICE_INTR(BTM2_O_BTM),
    .tmr_id = SDRV_BTM_G1,
    .tmr_cfg = {
        .si_val = 23,
        .inc_val = 1,
        .frc_rld_rst_cnt_en = true,
        .term_use_mode = SDRV_BTM_DIRECT,
        .cmp_use_mode = SDRV_BTM_DIRECT,
        .cnt_dir = SDRV_BTM_CNT_UP,
        .cnt_mode = SDRV_BTM_CONTINOUS_MODE,
    }};

static void timer1_init(void)
{
    timer1 = sdrv_btm_probe(&g_sdrv_btm_timer1, &timer1_config);
    timer_init(timer1);
}

static void led_set(int level)
{
    gpio_dev_t *gpio_dev;
    pinctrl_t *pinctrl;

    gpio_dev = GPIO_INSTANCE(led_pin);
    pinctrl = PINCTRL_INSTANCE(led_pin);

    pinctrl_config(pinctrl, led_pin, PINCTRL_CONFIG_FUNCTION, PIN_MUX_ALT0);

    gpio_config(gpio_dev, led_pin, GPIO_OUT);

    gpio_set(gpio_dev, led_pin, level);
}

static void Timer1Handle(void *arg)
{
    static int level;
    level ^= 1;
    led_set(level);
    timer1_cnt++;
    if (timer1_cnt >= 20) {
        osSemaphoreRelease(timer_sem);
        timer1_cnt = 0;
    }
}

static void led_flash_test(void)
{
    timer_sem = osSemaphoreNew(1, 0, NULL);
    timer1_init();

    timer_set_callback(timer1, Timer1Handle, NULL);

    timer_start(timer1, TMR_TYPE_PERIOD, 1000);

    osSemaphoreAcquire(timer_sem, osWaitForever);

    osSemaphoreDelete(timer_sem);
    timer_stop(timer1);
}

static int led_demo_main(int argc, char *argv[])
{
    if (argc == 0) {
        return CLI_FALSE;
    }

    if (!strcmp(argv[0], "on")) {
        led_pin = atoi(argv[1]);
        led_set(0);
    } else if (!strcmp(argv[0], "off")) {
        led_pin = atoi(argv[1]);
        led_set(1);
    } else if (!strcmp(argv[0], "flash")) {
        led_pin = atoi(argv[1]);
        led_flash_test();
    } else {
        printf("cmd error:%s\n", argv[0]);
    }

    return CLI_FALSE;
}

CLI_CMD("led_demo", "\r\nled_demo:\r\n", led_demo_main);
