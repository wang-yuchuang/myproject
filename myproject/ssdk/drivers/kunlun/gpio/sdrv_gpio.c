/**
 * @file sdrv_gpio.c
 * @brief Kunlun GPIO driver.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>
#include <bits.h>
#include <debug.h>
#include <reg.h>

#include "scr_hw.h"
#include "regs_base.h"

#include "driver/irq.h"
#include "driver/scr.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "scr_cfg.h"


/*
 * GPIO register groups. A register group is composed of several
 * continuous 32 bit registers, each bit mapped to one GPIO. For
 * example, direction of GPIO 119 is set by bit 23 (119%32) of register
 * GPIO_DIR[3] (119/32).
 */
typedef enum gpio_reg {
    GPIO_DIR                = 0x2000,
    GPIO_DATA_IN            = 0x2200,
    GPIO_DATA_OUT           = 0x2400,
    GPIO_INT_EN             = 0x2600,
    GPIO_INT_MASK           = 0x2800,
    GPIO_INT_TYPE           = 0x2a00,
    GPIO_INT_POLARITY       = 0x2c00,
    GPIO_INT_BOTH_EDGE      = 0x2e00,
    GPIO_INT_STATUS         = 0x3000,   /* interrupt status after masking */
    GPIO_INT_STATUS_UNMASK  = 0x3200,   /* raw interrupt status, without mask */
    GPIO_INT_EDGE_CLR       = 0x3400,
    GPIO_INT_LEVEL_SYNC     = 0x3600,
    GPIO_INT_DEBOUNCE_EN    = 0x3800,
    GPIO_INT_CLK_SEL        = 0x3a00,
    GPIO_INT_PCLK_ACTIVE    = 0x3c00,
} gpio_reg_e;

/* GPIO controller register bits. */
#define DIR_IN                          0
#define DIR_OUT                         1
#define INT_TYPE_LEVEL_SENSITIVE        0
#define INT_TYPE_EDGE_SENSITIVE         1
#define INT_POLARITY_LOW_OR_NEGATIVE    0
#define INT_POLARITY_HIGH_OR_POSITIVE   1
#define INT_BOTH_EDGE_SINGLE            0
#define INT_BOTH_EDGE_BOTH              1

#define _gpio_reg_offset(pin)           ((pin) / 32 * 0x10)
#define _gpio_bit_offset(pin)           ((pin) % 32)
#define _gpio_mask(pin)                 (1ul << _gpio_bit_offset(pin))

/* Domain MUX bit, used by GPIO_MUX1 and GPIO_MUX2. */
#define GPIO_MUX_SAFETY_DOMAIN          0
#define GPIO_MUX_AP_DOMAIN              1

/* Non-safety domain GPIO MUX bits, used by GPIO_MUX_SEC and GPIO_MUX_SAF. */
#define GPIO_SEL_GPIO_CTRL2             0
#define GPIO_SEL_GPIO_CTRL3             1
#define GPIO_SEL_GPIO_CTRL4             2
#define GPIO_SEL_GPIO_CTRL5             3

/* Number of GPIOs controlled by GPIO_MUX1/SCR_SAF. */
#define GPIO_MUX1_START                 0
#ifdef CONFIG_BOARD_X9SP
#define GPIO_MUX1_NR                    89
#else
#define GPIO_MUX1_NR                    48
#endif

/* Number of GPIOs controlled by GPIO_MUX2/SCR_SEC. */
#define GPIO_MUX2_START                 GPIO_MUX1_NR
#ifdef CONFIG_BOARD_X9SP
#define GPIO_MUX2_NR                    156
#else
#define GPIO_MUX2_NR                    108
#endif

#define is_safety_domain_ctrl(ctrl)     (ctrl == GPIO_CTRL1)
#define is_ap_domain_ctrl(ctrl)         (ctrl >= GPIO_CTRL2 && \
                                         ctrl <= GPIO_CTRL5)

#define is_mux1_gpio(pin) \
    ((pin) >= GPIO_MUX1_START && (pin) < GPIO_MUX1_START + GPIO_MUX1_NR)
#define is_mux2_gpio(pin) \
    ((pin) >= GPIO_MUX2_START && (pin) < GPIO_MUX2_START + GPIO_MUX2_NR)

#ifdef CONFIG_BOARD_X9SP
/* MUX1 control signals for GPIO[1:48], one bit for each pin. */
static scr_signal_t g_gpio_mux1_signals[] = {
    SCR_SAFETY__scr_gpio_mux1_sel_15_0,
    SCR_SAFETY__scr_gpio_mux1_sel_31_16,
    SCR_SAFETY__scr_gpio_mux1_sel_47_32,
    SCR_SAFETY__scr_gpio_mux1_sel_63_48,
    SCR_SAFETY__scr_gpio_mux1_sel_79_64,
    SCR_SAFETY__scr_gpio_mux1_sel_88_80,
};

/* MUX2 control signals for GPIO[49:155], one bit for each pin. */
static scr_signal_t g_gpio_mux2_signals[] = {
    SCR_SEC__scr_gpio_mux2_sel_15_0,
    SCR_SEC__scr_gpio_mux2_sel_31_16,
    SCR_SEC__scr_gpio_mux2_sel_47_32,
    SCR_SEC__scr_gpio_mux2_sel_63_48,
    SCR_SEC__scr_gpio_mux2_sel_79_64,
    SCR_SEC__scr_gpio_mux2_sel_95_80,
    SCR_SEC__scr_gpio_mux2_sel_111_96,
    SCR_SEC__scr_gpio_mux2_sel_127_112,
    SCR_SEC__scr_gpio_mux2_sel_143_128,
    SCR_SEC__scr_gpio_mux2_sel_155_144,
};

/* GPIO_MUX_SAF control signals for GPIO[1:48], two bits for each pin. */
static scr_signal_t g_gpio_mux_saf_signals[] = {
    SCR_SAFETY__scr_gpio_mux_saf_sel_15_0,
    SCR_SAFETY__scr_gpio_mux_saf_sel_31_16,
    SCR_SAFETY__scr_gpio_mux_saf_sel_47_32,
    SCR_SAFETY__scr_gpio_mux_saf_sel_63_48,
    SCR_SAFETY__scr_gpio_mux_saf_sel_79_64,
    SCR_SAFETY__scr_gpio_mux_saf_sel_95_80,
    SCR_SAFETY__scr_gpio_mux_saf_sel_111_96,
    SCR_SAFETY__scr_gpio_mux_saf_sel_127_112,
    SCR_SAFETY__scr_gpio_mux_saf_sel_143_128,
    SCR_SAFETY__scr_gpio_mux_saf_sel_159_144,
    SCR_SAFETY__scr_gpio_mux_saf_sel_175_160,
    SCR_SAFETY__scr_gpio_mux_saf_sel_177_176,
};

/* GPIO_MUX_SEC control signals for GPIO[49:155], two bits for each pin. */
static scr_signal_t g_gpio_mux_sec_signals[] = {
    SCR_SEC__scr_gpio_mux_sec_sel_15_0,
    SCR_SEC__scr_gpio_mux_sec_sel_31_16,
    SCR_SEC__scr_gpio_mux_sec_sel_47_32,
    SCR_SEC__scr_gpio_mux_sec_sel_63_48,
    SCR_SEC__scr_gpio_mux_sec_sel_79_64,
    SCR_SEC__scr_gpio_mux_sec_sel_95_80,
    SCR_SEC__scr_gpio_mux_sec_sel_111_96,
    SCR_SEC__scr_gpio_mux_sec_sel_127_112,
    SCR_SEC__scr_gpio_mux_sec_sel_143_128,
    SCR_SEC__scr_gpio_mux_sec_sel_159_144,
    SCR_SEC__scr_gpio_mux_sec_sel_175_160,
    SCR_SEC__scr_gpio_mux_sec_sel_191_176,
    SCR_SEC__scr_gpio_mux_sec_sel_207_192,
    SCR_SEC__scr_gpio_mux_sec_sel_223_208,
    SCR_SEC__scr_gpio_mux_sec_sel_239_224,
    SCR_SEC__scr_gpio_mux_sec_sel_255_240,
    SCR_SEC__scr_gpio_mux_sec_sel_271_256,
    SCR_SEC__scr_gpio_mux_sec_sel_287_272,
    SCR_SEC__scr_gpio_mux_sec_sel_303_288,
    SCR_SEC__scr_gpio_mux_sec_sel_311_304,
};

/*
 * Map SOC pin index to GPIO controller pin index. The index mapping rule
 * applies to GPIO controller, and GPIO mux (GPIO_MUX1 and GPIO_MUX2).
 *
 *  SOC PIN Index       GPIO Controller PIN Index
 */
static int _gpio_index(unsigned int pin)
{
    if (pin < 25)
        return pin + 64;
    else if (pin <= 88)
        return pin - 25;
    else if (pin <= 136)
        return pin + 108;
    else
        return pin - 48;
}

#else
/* MUX1 control signals for GPIO[1:48], one bit for each pin. */
static scr_signal_t g_gpio_mux1_signals[] = {
    SCR_SAFETY__gpio_mux1_gpio_sel_15_0,
    SCR_SAFETY__gpio_mux1_gpio_sel_31_16,
    SCR_SAFETY__gpio_mux1_gpio_sel_47_32,
};

/* MUX2 control signals for GPIO[49:155], one bit for each pin. */
static scr_signal_t g_gpio_mux2_signals[] = {
    SCR_SEC__gpio_mux2_gpio_sel_15_0,
    SCR_SEC__gpio_mux2_gpio_sel_31_16,
    SCR_SEC__gpio_mux2_gpio_sel_47_32,
    SCR_SEC__gpio_mux2_gpio_sel_63_48,
    SCR_SEC__gpio_mux2_gpio_sel_79_64,
    SCR_SEC__gpio_mux2_gpio_sel_95_80,
    SCR_SEC__gpio_mux2_gpio_sel_107_96,
};

/* GPIO_MUX_SAF control signals for GPIO[1:48], two bits for each pin. */
static scr_signal_t g_gpio_mux_saf_signals[] = {
    SCR_SAFETY__gpio_mux_saf_gpio_sel_15_0,
    SCR_SAFETY__gpio_mux_saf_gpio_sel_31_16,
    SCR_SAFETY__gpio_mux_saf_gpio_sel_47_32,
    SCR_SAFETY__gpio_mux_saf_gpio_sel_63_48,
    SCR_SAFETY__gpio_mux_saf_gpio_sel_79_64,
    SCR_SAFETY__gpio_mux_saf_gpio_sel_95_80,
};

/* GPIO_MUX_SEC control signals for GPIO[49:155], two bits for each pin. */
static scr_signal_t g_gpio_mux_sec_signals[] = {
    SCR_SEC__gpio_mux_sec_gpio_sel_15_0,
    SCR_SEC__gpio_mux_sec_gpio_sel_31_16,
    SCR_SEC__gpio_mux_sec_gpio_sel_47_32,
    SCR_SEC__gpio_mux_sec_gpio_sel_63_48,
    SCR_SEC__gpio_mux_sec_gpio_sel_79_64,
    SCR_SEC__gpio_mux_sec_gpio_sel_95_80,
    SCR_SEC__gpio_mux_sec_gpio_sel_111_96,
    SCR_SEC__gpio_mux_sec_gpio_sel_127_112,
    SCR_SEC__gpio_mux_sec_gpio_sel_143_128,
    SCR_SEC__gpio_mux_sec_gpio_sel_159_144,
    SCR_SEC__gpio_mux_sec_gpio_sel_175_160,
    SCR_SEC__gpio_mux_sec_gpio_sel_191_176,
    SCR_SEC__gpio_mux_sec_gpio_sel_207_192,
    SCR_SEC__gpio_mux_sec_gpio_sel_215_208,
};
/*
 * Map SOC pin index to GPIO controller pin index. The index mapping rule
 * applies to GPIO controller, and GPIO mux (GPIO_MUX1 and GPUO_MUX_SAF).
 *
 *  SOC PIN Index       GPIO Controller PIN Index
 *  [0:23]              [24:47]
 *  [24:47]             [0:23]
 *  [48:155]            [48:155]
 */
static int _gpio_index(unsigned int pin)
{
    if (pin < 24)
        return pin + 24;
    else if (pin < 48)
        return pin - 24;
    else
        return pin;
}
#endif

static uint32_t _gpio_reg(paddr_t base, gpio_reg_e reg, unsigned int pin)
{
    return base + reg + _gpio_reg_offset(pin);
}

static void _gpio_reg_write(paddr_t base, gpio_reg_e reg, unsigned int pin,
                            bool val)
{
    uint32_t v = readl(_gpio_reg(base, reg, pin));
    v &= ~_gpio_mask(pin);
    v |= val << _gpio_bit_offset(pin);
    writel(v, _gpio_reg(base, reg, pin));
}

static bool _gpio_reg_read(paddr_t base, gpio_reg_e reg, unsigned int pin)
{
    uint32_t val = readl(_gpio_reg(base, reg, pin));
    return !!(val & _gpio_mask(pin));
}

/* Update GPIO_MUX SCR. */
static void set_gpio_mux_scr(scr_dev_t* dev, scr_signal_t *sig, uint32_t pos,
                             uint32_t width, uint32_t val)
{
    uint32_t v = scr_get(dev, sig);
    v &= ~(BIT_MASK(width) << pos);
    v |= (val << pos);
    scr_set(dev, sig, v);
}

/*
 * Assign GPIO pin to safety or non-safety domian, by configuring top level
 * MUX1 or MUX2.
 */
static void assign_domain(unsigned int pin, int domain)
{
    scr_signal_t *sig;
    scr_dev_t dev;

    if (is_mux1_gpio(pin)) {
        dev = g_scr_safety;
        sig = &g_gpio_mux1_signals[pin / 16];
    }
    else {
        dev = g_scr_sec;
        sig = &g_gpio_mux2_signals[(pin - GPIO_MUX2_START) / 16];
    }

    set_gpio_mux_scr(&dev, sig, pin % 16, 1, domain);
}

/*
 * Assign GPIO pin GPIO controller.
 */
static void assign_controller(unsigned int pin, sdrv_gpio_ctrl_e ctrl)
{
    static const uint32_t gpio_sel[GPIO_CTRL_MAX] = {
        [GPIO_CTRL2] = GPIO_SEL_GPIO_CTRL2,
        [GPIO_CTRL3] = GPIO_SEL_GPIO_CTRL3,
        [GPIO_CTRL4] = GPIO_SEL_GPIO_CTRL4,
        [GPIO_CTRL5] = GPIO_SEL_GPIO_CTRL5,
    };

    /*
     * Controller configration is required when the pin is assigned to
     * non-safety domain controllers.
     */
    if (is_ap_domain_ctrl(ctrl)) {
        scr_dev_t *dev;
        scr_signal_t *sig;

        if (is_mux1_gpio(pin)) {
            dev = &g_scr_safety;
            sig = &g_gpio_mux_saf_signals[pin / 8];
        }
        else {
            dev = &g_scr_sec;
            sig = &g_gpio_mux_sec_signals[(pin - GPIO_MUX2_START) / 8];
        }

        set_gpio_mux_scr(dev, sig, (pin % 8) * 2, 2, gpio_sel[ctrl]);
    }
}

/**
 * @brief Assign GPIO pin to GPIO controller.
 *
 * The SOC has 5 GPIO controllers, all of which can controll all GPIO
 * pins. GPIO MUX moudules are designed to make sure one GPIO pin is
 * accessable by just one GPIO controller.
 *
 * 1) MUX1 and MUX2
 *
 * MUX1 and MUX2 choose whether a GPIO pin is accessable by safety
 * domain (GPIO controller 1) or non-safety(AP) domain (GPIO controller 2~5).
 * MUX1 manages GPIO[1:48], using SCR_SAFETY. MUX2 manages GPIO[49:155],
 * using SCR_SEC.
 *
 * 2) GPIO_MUX_SEC and GPIO_MUX_SAF
 *
 * When a GPIO is assigned to AP domain by MUX1 or MUX2, we need to assign
 * it to one specific AP domain GPIO controller, i.e., GPIO controller 2~5.
 * The assignment is controlled by AP domain GPIO MUX (GPIO_MUX_SEC and
 * GPIO_MUX_SAF). GPIO_MUX_SAF manages GPIO[1:48], and GPIO_MUX_SAF manages
 * GPIO[49:155].
 *
 * @param[in] pin SOC pin index.
 * @param[in] ctrl GPIO controller the pin is assigned to.
 *
 * @note This function is not part of standard GPIO API.
 */
void sdrv_gpio_assign_controller(unsigned int pin, sdrv_gpio_ctrl_e ctrl)
{
    /* Map SOC pin index to GPIO MUX index. */
    pin = _gpio_index(pin);

    /* Assign the pin to GPIO controller. */
    if (is_mux1_gpio(pin) || is_mux2_gpio(pin)) {
        if (is_safety_domain_ctrl(ctrl)) {
            /* Configure MUX1/2. */
            assign_domain(pin, GPIO_MUX_SAFETY_DOMAIN);
        }
        else {
            /* Configure MUX1/2. */
            assign_domain(pin, GPIO_MUX_AP_DOMAIN);
            /* Configure GPIO_MUX_SEC and GPIO_MUX_SAF. */
            assign_controller(pin, ctrl);
        }
    }
}

/**
 * @brief Initialize the GPIO controller.
 *
 * @param[in] ctrl GPIO controller.
 */
static void sdrv_gpio_initialize(struct gpio_dev *dev)
{
    return;
}

static bool sdrv_gpio_getpin(struct gpio_dev *dev, unsigned int pin)
{
    return _gpio_reg_read(dev->base, GPIO_DATA_IN, _gpio_index(pin));
}

static void sdrv_gpio_setpin(struct gpio_dev *dev, unsigned int pin, bool val)
{
    _gpio_reg_write(dev->base, GPIO_DATA_OUT, _gpio_index(pin), val);
}

/*
 * This function returns the "unmasked" interrupt status of the pin, i.e.,
 * the status is active when the signal is alive, and its interrupt
 * is enabled by GPIO_INT_MASK & GPIO_INT_EN.
 */
static bool sdrv_gpio_int_status(struct gpio_dev *dev, unsigned int pin)
{
    return _gpio_reg_read(dev->base, GPIO_INT_STATUS, _gpio_index(pin));
}

static void sdrv_gpio_config(struct gpio_dev *dev, unsigned int pin,
                             gpio_type_e type)
{
    pin = _gpio_index(pin);

    if (type == GPIO_OUT) {
        _gpio_reg_write(dev->base, GPIO_DIR, pin, DIR_OUT);
    }
    else {
        /* All other modes are input modes. */
        _gpio_reg_write(dev->base, GPIO_DIR, pin, DIR_IN);

        /* Configure interrupt trigger. */
        switch (type) {
            case GPIO_INTR_HIGH_LEVEL:
                _gpio_reg_write(dev->base, GPIO_INT_TYPE, pin,
                                INT_TYPE_LEVEL_SENSITIVE);
                _gpio_reg_write(dev->base, GPIO_INT_POLARITY, pin,
                                INT_POLARITY_HIGH_OR_POSITIVE);
                break;

            case GPIO_INTR_LOW_LEVEL:
                _gpio_reg_write(dev->base, GPIO_INT_TYPE, pin,
                                INT_TYPE_LEVEL_SENSITIVE);
                _gpio_reg_write(dev->base, GPIO_INT_POLARITY, pin,
                                INT_POLARITY_LOW_OR_NEGATIVE);
                break;

            case GPIO_INTR_RISING_EDGE:
                _gpio_reg_write(dev->base, GPIO_INT_BOTH_EDGE, pin,
                                INT_BOTH_EDGE_SINGLE);
                _gpio_reg_write(dev->base, GPIO_INT_TYPE, pin,
                                INT_TYPE_EDGE_SENSITIVE);
                _gpio_reg_write(dev->base, GPIO_INT_POLARITY, pin,
                                INT_POLARITY_HIGH_OR_POSITIVE);
                break;

            case GPIO_INTR_FALLING_EDGE:
                _gpio_reg_write(dev->base, GPIO_INT_BOTH_EDGE, pin,
                                INT_BOTH_EDGE_SINGLE);
                _gpio_reg_write(dev->base, GPIO_INT_TYPE, pin,
                                INT_TYPE_EDGE_SENSITIVE);
                _gpio_reg_write(dev->base, GPIO_INT_POLARITY, pin,
                                INT_POLARITY_LOW_OR_NEGATIVE);
                break;

            case GPIO_INTR_BOTH_EDGE:
                _gpio_reg_write(dev->base, GPIO_INT_BOTH_EDGE, pin,
                                INT_BOTH_EDGE_BOTH);
                _gpio_reg_write(dev->base, GPIO_INT_TYPE, pin,
                                INT_TYPE_EDGE_SENSITIVE);
                break;

            default:
                break;
        }
    }
}

static void sdrv_gpio_enable_irq(struct gpio_dev *dev, unsigned int pin,
                                 bool enable)
{
    pin = _gpio_index(pin);

    if (enable) {
        _gpio_reg_write(dev->base, GPIO_INT_MASK, pin, 0);
        _gpio_reg_write(dev->base, GPIO_INT_EN, pin, 1);
    }
    else {
        _gpio_reg_write(dev->base, GPIO_INT_MASK, pin, 1);
        _gpio_reg_write(dev->base, GPIO_INT_EN, pin, 0);
    }
}

/*
 * Controller interrupt handler for all 6 interrupts generated by this
 * GPIO controller.
 */
static int sdrv_gpio_handle_irq(uint32_t irq, void *arg)
{
    struct gpio_dev *dev = arg;
    gpio_irq_desc_t *desc;
    gpio_irq_desc_t *temp;

    /* Execute IRQ handler for each registered pin, if it's active. */
    list_for_every_entry_safe(&dev->irq_desc, desc, temp, gpio_irq_desc_t, node) {
        unsigned int pin = _gpio_index(desc->pin);

        if (_gpio_reg_read(dev->base, GPIO_INT_STATUS, pin)) {
            ASSERT(desc->handler);
            desc->handler(dev, desc->pin, desc->arg);

            /*
             * Clear edge sensitive interrupts. Level sensitive interrupts
             * should have been cleared from interrupt source, by the pin
             * irq handler.
             */
            _gpio_reg_write(dev->base, GPIO_INT_EDGE_CLR, pin, 1);
        }
    }

    return 0;
}

static gpio_ops_t sdrv_gpio_ops = {
    .init = sdrv_gpio_initialize,
    .config = sdrv_gpio_config,
    .set = sdrv_gpio_setpin,
    .get = sdrv_gpio_getpin,
    .enable_irq = sdrv_gpio_enable_irq,
    .int_status = sdrv_gpio_int_status,
};

/**
 * @brief Initialize the sdrv gpio controller.
 *
 * @param[in] dev Common gpio device.
 * @param[in] base sdrv gpio controller base address.
 * @param[in] irq_cnt Number of IRQ lines of this controller.
 * @param[in] irqs IRQ number array.
 */
void sdrv_gpio_init(gpio_dev_t *dev, paddr_t base, int irq_cnt,
                    const uint32_t *irqs)
{
    /*
     * The GPIO controller has 6 interrupts, so we don't register
     * ISR handler to the common GPIO framework, but handle the
     * interrupts locally.
     */
    for (int i = 0; i < irq_cnt; i++) {
        irq_attach(irqs[i], sdrv_gpio_handle_irq, dev);
        irq_enable(irqs[i]);
    }

    gpio_init(dev, base, 0, &sdrv_gpio_ops);
}
