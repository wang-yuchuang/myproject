/**
 * @file gpio.h
 * @brief Common GPIO driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef _GPIO_H
#define _GPIO_H

#include <types.h>
#include <spinlock.h>
#include <lib/list.h>

/**
 * @brief Common GPIO type.
 */
typedef enum gpio_type {
    GPIO_OUT,                   /* OUTPUT */
    GPIO_IN,                    /* INPUT. No interrupt. */
    GPIO_INTR_HIGH_LEVEL,       /* INPUT. Interrupt on high level. */
    GPIO_INTR_LOW_LEVEL,        /* INPUT. Interrupt on low level. */
    GPIO_INTR_RISING_EDGE,      /* INPUT. Interrupt on rising edge. */
    GPIO_INTR_FALLING_EDGE,    /* INPUT. Interrupt on falling edge. */
    GPIO_INTR_BOTH_EDGE,        /* INPUT. Interrupt on both edges. */
    GPIO_TYPE_MAX,
} gpio_type_e;

struct gpio_dev;

/**
 * @brief GPIO hardware driver function table.
 */
typedef struct gpio_ops {
    /* Initialize the controller. */
    void (*init)(struct gpio_dev *dev);

    /* De-initialize the controller. */
    void (*deinit)(struct gpio_dev *dev);

    /* Config pin. */
    void (*config)(struct gpio_dev *dev, unsigned int pin, gpio_type_e type);

    /* Set pin leve. */
    void (*set)(struct gpio_dev *dev, unsigned int pin, bool val);

    /* Get pin level. */
    bool (*get)(struct gpio_dev *dev, unsigned int pin);

    /* Enable pin interrupt. */
    void (*enable_irq)(struct gpio_dev *dev, unsigned int pin, bool enable);

    /* Get pin interrupt status. */
    bool  (*int_status)(struct gpio_dev *dev, unsigned int pin);

    /* Enable pin async-interrupt. */
    void (*enable_async_irq)(struct gpio_dev *dev, unsigned int pin, bool enable);

    /* Get pin async-interrupt status. */
    bool  (*async_int_status)(struct gpio_dev *dev, unsigned int pin);
} gpio_ops_t;

/**
 * @brief GPIO pin interrupt handler.
 *
 * This handler is registered by GPIO users, to handle interrupts
 * from specific pins (not the controller itself).
 */
typedef void (*gpio_irq_handler_t)(struct gpio_dev *dev, int pin, void *arg);

/**
 * @brief Pin interrupt descriptor.
 */
typedef struct gpio_irq_desc {
    struct list_node    node;

    /* controller local pin index */
    uint32_t            pin;

    /* pin IRQ handler */
    gpio_irq_handler_t  handler;

    /* pin IRQ arg */
    void                *arg;
} gpio_irq_desc_t;

/**
 * @brief Abstract GPIO device.
 */
typedef struct gpio_dev {
    paddr_t             base;
    spin_lock_t         lock;

    /* controller interrupt number */
    int                 irq;

    /* pin interrupt descriptors */
    struct list_node    irq_desc;

    /* GPIO chip driver data */
    void                *dev;

    /* GPIO chip driver functions */
    gpio_ops_t          *ops;
} gpio_dev_t;

void gpio_init(gpio_dev_t *dev, paddr_t base, uint32_t irq, gpio_ops_t *ops);
void gpio_config(gpio_dev_t *dev, unsigned int pin, gpio_type_e type);
void gpio_set(gpio_dev_t *dev, unsigned int pin, bool val);
bool gpio_get(gpio_dev_t *dev, unsigned int pin);
bool gpio_attach_irq(gpio_dev_t *dev, unsigned int pin,
                     gpio_irq_handler_t handler, void *arg);
void gpio_int_enable(gpio_dev_t *dev, unsigned int pin, bool en);
bool gpio_int_status(gpio_dev_t *dev, unsigned int pin);

bool gpio_attach_async_irq(gpio_dev_t *dev, unsigned int pin,
                           gpio_irq_handler_t handler, void *arg);
void gpio_async_int_enable(gpio_dev_t *dev, unsigned int pin, bool en);
bool gpio_async_int_status(gpio_dev_t *dev, unsigned int pin);

#endif
