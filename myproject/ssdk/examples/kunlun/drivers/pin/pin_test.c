#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bits.h>

#include "cmsis_os2.h"
#include "CLI_type.h"
#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "irq_num.h"
#include "regs_base.h"
#include "param.h"
#include "pinmux_cfg.h"
#include "reg.h"

/**
 * @brief GPIO read write test.
 *
 * Note that to control
 * a GPIO from Safety or Secure R5 or AP1, the pin must have been assigned
 * to the GPIO controller of the core, i.e., GPIO_1 for Safety R5, and
 * GPIO_2 for Secure R5. See port_cfg.h
 *
 */

#define PULL_UP 2
#define PULL_DOWN 1

#if defined(CONFIG_BOARD_D9_AP2)
    extern gpio_dev_t g_gpio_ap2;
    gpio_dev_t *gpio_dev = &g_gpio_ap2;
#elif defined(CONFIG_ARCH_CHIP_D9_SEC) || defined(CONFIG_SECURE)
    extern gpio_dev_t g_gpio_secure;
    gpio_dev_t *gpio_dev = &g_gpio_secure;
#elif defined(CONFIG_BOARD_D9_AP1)
    extern gpio_dev_t g_gpio_ap1;
    gpio_dev_t *gpio_dev = &g_gpio_ap1;
#else
    extern gpio_dev_t g_gpio_dev;
    gpio_dev_t *gpio_dev = &g_gpio_dev;
#endif

#ifdef CONFIG_BOARD_X9SP
    #define SAF_PIN_MAX 89
    #define EMMC_PIN_MIN 221
#else
    #define SAF_PIN_MAX 48
    #define EMMC_PIN_MIN 132
#endif

extern pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS];
extern pinctrl_t g_pinctrl_ap;
extern pinctrl_t g_pinctrl_safety;

#define GPIO_HELP "gpio help:"\
        "\r\ngpio r        idx        ;read index status" \
        "\r\ngpio w        idx val    ;set as output val(0 or 1)" \
        "\r\ngpio pulse    idx times  ;1s pulse,default times:5" \
        "\r\ngpio setinput idx        ;set as input"

#define GPIO_HELP1 "\r\ngpio setfunc  idx  func  ;set as funcX" \
        "\r\ngpio setctrl  idx ctrler ;set ctrlerX(1~5)." \
        "\r\ngpio setpad   idx  pull drive  ;set pull and drive" \
        "\r\n pull:0~no pull idx<132,Down:1 Up:3" \
        "\r\n pull:idx>=132,Up:1 Down:2" \
        "\r\n drive:0~push_pull 1~Open_drain"

#define GPIO_DATA_DIR_OFFSET 0x2000
#define GPIO_DATA_IN_OFFSET  0x2200
#define GPIO_DATA_OUT_OFFSET 0x2400
#define GPIO_CTR_DATA_OUT_OFFSET 2
#define GPIO_CTR_DATA_IN_OFFSET 1

static void gpio_irq_handler(struct gpio_dev *dev, int pin, void *arg)
{
    printf("gpio_riq_handler: %d\n", pin);
}

#ifdef CONFIG_BOARD_X9SP
static int get_io_index(unsigned int pin)
{
    if (pin < 25)
        return pin + 64;
    else if (pin <= SAF_PIN_MAX)
        return pin - 25;
    else if (pin <= 136)
        return pin + 108;
    else
        return pin - 48;
}
#else
static int get_io_index(unsigned int pin)
{
    if (pin < 24)
        return pin + 24;
    else if (pin < SAF_PIN_MAX)
        return pin - 24;
    else
        return pin;
}
#endif

static inline paddr_t pin_reg(paddr_t base, uint32_t offset, uint32_t pin)
{
    if (pin >= SAF_PIN_MAX){
        pin = pin - SAF_PIN_MAX;
    }
    return base + ((offset + pin * 4) << 10);
}

//get input/output addr
static uint32_t get_gpio_addr(paddr_t base, uint32_t offset, unsigned int pin)
{
    return base + offset + ((pin) / 32 * 0x10);
}

// get controller_id addr
static uint32_t get_gpio_controller_id_addr(unsigned int pinidx)
{
    paddr_t base = APB_GPIO1_BASE;
    pin_settings_config_t *set_config = &g_pin_init_config[pinidx];

    switch (set_config->controller_id) {
        case PIN_CONTROLLER1: base = APB_GPIO1_BASE; break;
        case PIN_CONTROLLER2: base = APB_GPIO2_BASE; break;
        case PIN_CONTROLLER3: base = APB_GPIO3_BASE; break;
        case PIN_CONTROLLER4: base = APB_GPIO4_BASE; break;
        case PIN_CONTROLLER5: base = APB_GPIO5_BASE; break;
        default: break;
    }

    return base;
}

#define GPIO_CTR_OFFSET 0x0000
// get GPIO_CTRn reg addr
static uint32_t get_gpio_ctr_n_addr(paddr_t base, unsigned int io_index)
{
    return base + GPIO_CTR_OFFSET + (io_index* 0x10);
}

// get GPIO_CTRn direction reg addr
static uint32_t get_gpio_ctr_dire_addr(paddr_t base,  unsigned int pin)
{
    return base + GPIO_CTR_OFFSET + (pin * 0x10);
}

struct gpio_config{
    char *name;
    uint32_t shift;
    uint32_t width;
    paddr_t base;
    uint32_t offset;
    unsigned int pin;
    uint32_t pin_value;
};

struct gpio_config    g_pin_config[]={
    {"function       ", PAD_MUX_FUNC_SHIFT, PAD_MUX_FUNC_WIDTH, 0, 0, 0, 0},
    {"force_input    ", PAD_MUX_FORCE_INPUT_SHIFT, PAD_MUX_FORCE_INPUT_WIDTH, 0, 0, 0, 0},
    {"drive          ", PAD_MUX_OPEN_DRAIN_SHIFT, PAD_MUX_OPEN_DRAIN_WIDTH, 0, 0, 0, 0},
    {"pull           ", PAD_PULL_SHIFT, PAD_PULL_WIDTH, 0, 0, 0, 0},
    {"drive_strength ", PAD_DRIVE_STRENGTH_SHIFT, PAD_DRIVE_STRENGTH_WIDTH, 0, 0, 0, 0},
    {"schmitt        ", PAD_INPUT_SCHMITT_SHIFT, PAD_INPUT_SCHMITT_WIDTH, 0, 0, 0, 0},
    {"slew_rate      ", PAD_SLEW_RATE_SHIFT, PAD_SLEW_RATE_WIDTH, 0, 0, 0, 0},
};

static uint32_t get_pin_value(struct gpio_config *config)
{
    uint32_t addr = pin_reg(config->base, config->offset, config->pin);
    uint32_t v = readl(addr);
    uint32_t v1 = (v >> config->shift) & (BIT_MASK(config->width));
    config->pin_value = v1;
    printf("%s addr=0x%x value=0x%.4x (shft=0x%x width=0x%x)\n", config->name,
            addr,v, config->shift, config->width);
    return config->pin_value;
}

static uint32_t get_pin_value_from_name(char *name)
{
    uint32_t s = sizeof(g_pin_config)/sizeof(struct gpio_config);
    struct gpio_config * cfg;

    for (int i = 0; i < s; i++) {
        cfg = &g_pin_config[i];
        if (strstr(cfg->name, name) != NULL) {
            return cfg->pin_value;
        }
    }
    return -1;
}

static uint32_t init_pin_config(pinctrl_t *pinctrl, unsigned int pinidx)
{
    struct gpio_config *cfg;
    sdrv_pinctrl_t *ctrl;
    pin_settings_config_t *set_config = &g_pin_init_config[pinidx];

    for (int i = 0; i < sizeof(g_pin_config)/sizeof(struct gpio_config); i++) {
        ctrl = pinctrl->dev;
        cfg = &g_pin_config[i];
        cfg->base = ctrl->base;
        cfg->pin = set_config->pin_index;
        if ((strstr(cfg->name,"function") != NULL)
            || (strstr(cfg->name,"force_input") != NULL)
            || (strcmp(cfg->name,"drive          ") == 0) ) {
            cfg->offset = ctrl->mux_reg_offset;
        } else {
            //pull drive_strength input schmit slew_rate use pad offset
            cfg->offset = ctrl->pad_reg_offset;
        }
        get_pin_value(cfg);
    }

    return -1;
}

static int pin_read_status(gpio_dev_t *dev, unsigned int pinidx)
{
    pin_settings_config_t *set_config = &g_pin_init_config[pinidx];

    int io_idx = get_io_index(pinidx);
    uint32_t controller_id_addr = get_gpio_controller_id_addr(pinidx);
    uint32_t gpio_ctr_addr = get_gpio_ctr_n_addr(controller_id_addr, io_idx);
    uint32_t gpio_ctr_value = readl(gpio_ctr_addr);
    uint32_t direction = gpio_ctr_value & 0x01;
    uint32_t gpio_ctr_data_in = (gpio_ctr_value >> GPIO_CTR_DATA_IN_OFFSET) & 0x01;
    uint32_t gpio_ctr_data_out = (gpio_ctr_value >> GPIO_CTR_DATA_OUT_OFFSET) & 0x01;

    printf("###########gpio_%d status:###############\n", pinidx);
    printf("gpio_ctr addr=0x%.4x value=0x%.4x\n", gpio_ctr_addr, gpio_ctr_value);

    if (0 == set_config->mux)
        printf("controller_id   =%d(1~saf 2~sec 3~mp 4~ap1 5~ap2)\n",
                set_config->controller_id);

#if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
    //mp or ap1 can't get mux regs
    if (pinidx >= SAF_PIN_MAX) {
#else
    #ifdef CONFIG_BOARD_X9SP
        //this tool can only use in saf for x9sp
        if (pinidx < SAF_PIN_MAX) {
    #endif
#endif

    pinctrl_t *pinctrl = pinidx < SAF_PIN_MAX ? &g_pinctrl_safety : &g_pinctrl_ap;
    init_pin_config(pinctrl, pinidx);
    uint32_t func = get_pin_value_from_name("function");
    uint32_t force_input = get_pin_value_from_name("force_input");
    uint32_t drive = get_pin_value_from_name("drive");
    uint32_t pull_config = get_pin_value_from_name("pull");
    uint32_t drive_strength = get_pin_value_from_name("drive_strength");
    uint32_t input_select = get_pin_value_from_name("schmitt");
    uint32_t slew_rate = get_pin_value_from_name("slew_rate");

    if (pinidx < EMMC_PIN_MIN){
        if((pull_config&0x01) != 0)
            drive=0;
    }else{
        if((pull_config&0x03) != 0)
            drive=0;
    }

    printf("func          =%d (0~gpio)\n",func);
    printf("pull_config   =%d ([NoPull:0] [idx<132,Down:1 Up:3] [idx>=132,Up:1 Down:2])\n", pull_config);
    printf("drive         =%d(0~push pull(depend on pull_config); 1~Open drain;)\n", drive);
    printf("drive_strength=%d\n", drive_strength);
    printf("slew_rate     =%d\n", slew_rate);
    printf("input_select  =%d(select:0~CMOS_SCHMITT 1~CMOS)\n", input_select );
    printf("force_input   =%d\n", force_input);
#if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
    }
#else
#ifdef CONFIG_BOARD_X9SP
    }
#endif
#endif
    printf("data_direction  =%s\n",(direction == 0)? "input":"output");
    if (direction == 0)
        printf("data            =0x%x\n", gpio_ctr_data_in);
    else
        printf("data            =0x%x\n", gpio_ctr_data_out);
    printf("#########################################\n");
    return 0;
}

static int pin_setinput(gpio_dev_t *dev, unsigned int pinidx)
{
    pin_settings_config_t *config = &g_pin_init_config[pinidx];

    gpio_config(dev, pinidx, GPIO_IN);
    config->data_direction = 0;
    pin_read_status(dev, pinidx);
    printf("gpio_%d is set as input now.\n", pinidx);
    return 0;
}

static int pin_write(gpio_dev_t *dev, unsigned int pinidx, unsigned int val)
{
    gpio_config(dev, pinidx, GPIO_OUT);   //set as output first
    gpio_set(dev, pinidx, !!val);

    pin_settings_config_t *config = &g_pin_init_config[pinidx];
    config->data_direction = PIN_OUTPUT_DIRECTION;
    config->initial_value = !!val;

    pin_read_status(dev, pinidx);
    printf("set gpio_%d as output %s\n", pinidx, (val != 0)? "HIGH":"LOW");

    return 0;
}

static int pin_setctrler(gpio_dev_t *dev, unsigned int pinidx, unsigned int val)
{
    if (val > 5) {
        printf("input pararm error:ctrler max is 5\n");
        return -1;
    }

    pin_settings_config_t *config = &g_pin_init_config[pinidx];
    config->controller_id = val;
    sdrv_gpio_assign_controller(pinidx, config->controller_id);
    pin_read_status(dev, pinidx);
    printf("idx=%d's ctrler=%d\n",pinidx, (unsigned int)config->controller_id);
    return 0;
}

static int pin_setfunc(gpio_dev_t *dev, unsigned int pinidx, unsigned int func)
{
    if (func > PAD_MUX_FUNC_ALT7) {
        printf("input pararm error:pin func max is %d\n",PAD_MUX_FUNC_ALT7);
        return -1;
    }
#ifdef CONFIG_BOARD_X9SP
    #if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
        pinctrl_t *pinctrl = &g_pinctrl_ap;
    #else
        pinctrl_t *pinctrl = &g_pinctrl_safety;
    #endif
#else
    pinctrl_t *pinctrl = pinidx < SAF_PIN_MAX ? &g_pinctrl_safety : &g_pinctrl_ap;
#endif
    pinctrl_config(pinctrl, pinidx, PINCTRL_CONFIG_FUNCTION, func);

    pin_settings_config_t *config = &g_pin_init_config[pinidx];
    config->mux = func;
    pin_read_status(dev, pinidx);
    return 0;
}

static int pin_setpad(gpio_dev_t *dev, unsigned int pinidx,
                unsigned int pull, unsigned int drive)
{
    if ((pinidx < 132 && pull > 3) || (pinidx >= 132 && pull > 2)) {
        printf("input pararm error:pull max is 3.\n");
        printf("[NoPull:0] [idx<132,Down:1 Up:3] [idx>=132,Up:1 Down:2]\n");
        return -1;
    }
    if (drive > 2) {
        printf("drive input pararm error\n");
        printf("drive: 0~push pull; 1~Open drain;\n");
        return -1;
    }
#ifdef CONFIG_BOARD_X9SP
    #if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
        pinctrl_t *pinctrl = &g_pinctrl_ap;
    #else
        pinctrl_t *pinctrl = &g_pinctrl_safety;
    #endif
#else
    pinctrl_t *pinctrl = pinidx < SAF_PIN_MAX ? &g_pinctrl_safety : &g_pinctrl_ap;
#endif

    if (pinidx < EMMC_PIN_MIN){
        if(pull == 3)
            pull=PULL_UP;
        else if(pull == 2)
            pull=PULL_DOWN;
    }else{
        if(pull == 1)
            pull=PULL_UP;
        else if(pull == 2)
            pull=PULL_DOWN;
    }

    pinctrl_config(pinctrl, pinidx, PINCTRL_CONFIG_PULL, pull);
    if (drive == 1)
        pinctrl_config(pinctrl, pinidx, PINCTRL_CONFIG_DRIVE, 0);
    else
        pinctrl_config(pinctrl, pinidx, PINCTRL_CONFIG_DRIVE, 2);

    pin_settings_config_t *config = &g_pin_init_config[pinidx];
    config->pull_config = pull;
    config->open_drain = (drive == 0)? 1 : 0;
    pin_read_status(dev, pinidx);
    return 0;
}

static int gpio_usage(void)
{
    printf("%s", GPIO_HELP);
    #ifndef CONFIG_BOARD_D9_AP1
        printf("%s", GPIO_HELP1);
    #endif
    return 0;
}

static int pin_test_main(int argc, char *argv[])
{
    int gpio;
    int value;
    int times;

    if (!argc) {
        gpio_usage();
        return 0;
    } else if (argc == 1) {
        if (strcmp(argv[0], "-h") == 0)
            gpio_usage();
        return 0;
    } else {
        gpio = atoi(argv[1]);
        if (gpio > NUM_OF_CONFIGURED_PINS) {
            printf("input param error,idx shoud < %d", NUM_OF_CONFIGURED_PINS);
        return -1;
        }
    }

    if (!strcmp(argv[0], "read") || !strcmp(argv[0], "r")) {
        pin_read_status(gpio_dev, gpio);
    } else if (!strcmp(argv[0], "write") || !strcmp(argv[0], "w")) {
        value = atoi(argv[2]);
        pin_write(gpio_dev, gpio, value);
    } else if (!strcmp(argv[0], "setinput")) {
        pin_setinput(gpio_dev, gpio);
    }
#if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
    if (gpio >= SAF_PIN_MAX) {
#endif
    if (!strcmp(argv[0], "setctrl")) {
        value = atoi(argv[2]);
        pin_setctrler(gpio_dev, gpio, value);
    } else if (!strcmp(argv[0], "setfunc")) {
        int func = atoi(argv[2]);
        pin_setfunc(gpio_dev, gpio, func);
    } else if (!strcmp(argv[0], "setpad")) {
        int pull = atoi(argv[2]);
        int drive = atoi(argv[3]);
        pin_setpad(gpio_dev, gpio, pull, drive);
    }
#if defined(CONFIG_MP) || defined(CONFIG_BOARD_D9_AP1)
    } else {
        printf("*PIN %d belongs to safety domain.\n", gpio);
        printf("*MP or AP has no access to setctrl/setfunc/setpad.");
    }
#endif
    if (!strcmp(argv[0], "pulse")) {
        if (argc < 3)
            times = 5;
        else
            times = atoi(argv[2]);
        while (times--) {
            pin_write(gpio_dev, gpio, 0);
            osDelay(1000);
            pin_write(gpio_dev, gpio, 1);
            osDelay(1000);
        }
    } else if (!strcmp(argv[0], "irq")) {
        gpio_config(gpio_dev, gpio, GPIO_INTR_BOTH_EDGE);
        gpio_attach_irq(gpio_dev, gpio, gpio_irq_handler, NULL);
    }

    return CLI_FALSE;
}

CLI_CMD("gpio", "gpio test command", pin_test_main);
