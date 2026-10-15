/**
 * @file gpio.h
 * @brief GPIO common driver.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <debug.h>
#include <stdlib.h>

#include "driver/gpio.h"
#include "driver/irq.h"

#define gpio_lock(dev) \
    irq_state_t state = spin_lock_irqsave(&dev->lock);
#define gpio_unlock(dev) \
    spin_unlock_irqrestore(&dev->lock, state);

/* Common GPIO IRQ handler. */
static int gpio_handle_irq(uint32_t irq, void *arg)
{
    gpio_dev_t *dev = arg;
    gpio_irq_desc_t *desc;
    gpio_irq_desc_t *temp;

    /* Execute IRQ handler for each registered pin, if it's active. */
    list_for_every_entry_safe(&dev->irq_desc, desc, temp, gpio_irq_desc_t, node) {
        if (gpio_int_status(dev, desc->pin)) {
            desc->handler(dev, desc->pin, desc->arg);
        }

        if (gpio_async_int_status(dev, desc->pin)) {
            desc->handler(dev, desc->pin, desc->arg);
        }
    }

    return 0;
}

static bool _gpio_attach_irq(gpio_dev_t *dev, unsigned int pin,
                             gpio_irq_handler_t handler, void *arg, bool sync)
{
    gpio_irq_desc_t *desc = NULL;
    bool ret = false;

    gpio_lock(dev);

    if (handler != NULL) {
        /* Enable IRQ handler for this pin. */
        list_for_every_entry(&dev->irq_desc, desc, gpio_irq_desc_t, node) {
            if (desc->pin == pin) {
                desc->handler = handler;    /* update handler if necessary */
                desc->arg = arg;
                break;
            }
        }

        if (desc == NULL || desc->pin != pin) {
            desc = malloc(sizeof *desc);
            if (desc != NULL) {
                desc->pin = pin;
                desc->handler = handler;
                desc->arg = arg;
                list_add_tail(&dev->irq_desc, &desc->node);
            }
        }

        if (desc != NULL) {
            if (sync && dev->ops->enable_irq) {
                dev->ops->enable_irq(dev, pin, true);
            }
            else if (!sync && dev->ops->enable_async_irq) {
                dev->ops->enable_async_irq(dev, pin, true);
            }

            ret = true;
        }
    }
    else {
        gpio_irq_desc_t *temp;

        /* Disable IRQ handler for this pin. */
        list_for_every_entry_safe(&dev->irq_desc, desc, temp,
                                  gpio_irq_desc_t, node) {
            if (desc->pin == pin) {
                if (sync && dev->ops->enable_irq) {
                    dev->ops->enable_irq(dev, pin, false);
                }
                else if (!sync && dev->ops->enable_async_irq) {
                    dev->ops->enable_async_irq(dev, pin, false);
                }

                list_delete(&desc->node);
                free(desc);
                ret = true;
                break;
            }
        }
    }

    gpio_unlock(dev);

    return ret;
}

/**
 * @brief Configure pin input/output mode, and interrupt type..
 *
 * @param[in] ctrl GPIO controller.
 * @param[in] pin Controller local pin index.
 * @param[in] type Pin type.
 */
void gpio_config(gpio_dev_t *dev, unsigned int pin, gpio_type_e type)
{
    gpio_lock(dev);
    dev->ops->config(dev, pin, type);
    gpio_unlock(dev);
}

/**
 * @brief Set pin level.
 *
 * @param[in] ctrl GPIO controller.
 * @param[in] pin Controller local pin index.
 * @param[in] level Pin level.
 */
void gpio_set(gpio_dev_t *dev, unsigned int pin, bool level)
{
    gpio_lock(dev);
    dev->ops->set(dev, pin, level);
    gpio_unlock(dev);
}

/**
 * @brief Get pin level.
 *
 * @param[in] ctrl GPIO controller.
 * @param[in] pin Controller local pin index.
 * @return Pin level.
 */
bool gpio_get(gpio_dev_t *dev, unsigned int pin)
{
    bool level;

    gpio_lock(dev);
    level = dev->ops->get(dev, pin);
    gpio_unlock(dev);

    return level;
}

/**
 * @brief Attach interrupt handler to pins.
 *
 * @param[in] dev Common gpio device.
 * @param[in] pin Controller pin number to attach the irq handler to.
 * @param[in] handler Pin irq handler. NULL to disable the pin interrupt.
 * @param[in] arg Pin irq arg.
 */
bool gpio_attach_irq(gpio_dev_t *dev, unsigned int pin,
                     gpio_irq_handler_t handler, void *arg)
{
    return _gpio_attach_irq(dev, pin, handler, arg, true);
}

/**
 * @brief Get interrupt status.
 *
 * @param[in] ctrl GPIO controller.
 * @param[in] pin Controller local pin index.
 */
bool gpio_int_status(gpio_dev_t *dev, unsigned int pin)
{
    bool val = false;

    gpio_lock(dev);
    if (dev->ops->int_status)
        val = dev->ops->int_status(dev, pin);
    gpio_unlock(dev);

    return val;
}

/**
 * @brief Control pin interrupt.
 *
 * @param[in] ctrl GPIO controller.
 * @param[in] pin Controller local pin index.
 * @param[in] en enable/disable.
 */
void gpio_int_enable(gpio_dev_t *dev, unsigned int pin, bool en)
{
    gpio_lock(dev);

    if (dev->ops->enable_irq)
        dev->ops->enable_irq(dev, pin, en);

    gpio_unlock(dev);
}

bool gpio_attach_async_irq(gpio_dev_t *dev, unsigned int pin,
                           gpio_irq_handler_t handler, void *arg)
{
    return _gpio_attach_irq(dev, pin, handler, arg, false);
}

void gpio_async_int_enable(gpio_dev_t *dev, unsigned int pin, bool en)
{
    gpio_lock(dev);

    if (dev->ops->enable_async_irq)
        dev->ops->enable_async_irq(dev, pin, en);

    gpio_unlock(dev);
}

bool gpio_async_int_status(gpio_dev_t *dev, unsigned int pin)
{
    bool val = false;

    gpio_lock(dev);

    if (dev->ops->async_int_status)
        val = dev->ops->async_int_status(dev, pin);

    gpio_unlock(dev);

    return val;
}

/**
 * @brief Initialize the GPIO controller.
 *
 * @param[in] dev Common gpio device.
 * @param[in] base sdrv gpio controller base address.
 * @param[in] irq Controller interrupt number. 0 if no IRQ required.
 * @param[in] ops Controller operations.
 */
void gpio_init(gpio_dev_t *dev, paddr_t base, uint32_t irq, gpio_ops_t *ops)
{
    dev->base = base;
    dev->irq = irq;

    /* get, set, and config functions must be implemented. */
    ASSERT(ops && ops->get && ops->set && ops->config);
    dev->ops = ops;

    spin_lock_init(&dev->lock);
    list_initialize(&dev->irq_desc);

    if (irq > 0) {
        irq_attach(irq, gpio_handle_irq, dev);
        irq_enable(irq);
    }

    if (dev->ops->init)
        dev->ops->init(dev);
}

/**
 * @brief De-initialize the GPIO controller.
 *
 * @param[in] dev Common gpio device.
 */
void gpio_deinit(gpio_dev_t *dev)
{
    gpio_irq_desc_t *desc;
    gpio_irq_desc_t *temp;

    gpio_lock(dev);

    if (dev->ops->deinit)
        dev->ops->deinit(dev);

    if (dev->irq > 0) {
        irq_disable(dev->irq);
        irq_detach(dev->irq);
    }

    gpio_unlock(dev);

    list_for_every_entry_safe(&dev->irq_desc, desc, temp,
                              gpio_irq_desc_t, node) {
        list_delete(&desc->node);
        free(desc);
    }
}
