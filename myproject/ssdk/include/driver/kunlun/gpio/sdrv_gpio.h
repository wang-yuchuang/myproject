/**
 * @file sdrv_gpio.h
 * @brief Kunlun GPIO driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef _SDRV_GPIO_H
#define _SDRV_GPIO_H

#include <types.h>
#include "driver/gpio.h"

/**
 * @brief Chip controller ID.
 *
 * There're 5 GPIO controllers in the SOC, all of which accessable
 * by cores. However each controller has unique interrupt connection
 * to GICs. For example GPIO1 is wired to GIC1, so software running
 * on Safety R5 should always use GPIO1.
*/
typedef enum sdrv_gpio_ctrl {
    GPIO_CTRL1 = 1,
    GPIO_SAFETY = GPIO_CTRL1,
    GPIO_CTRL2,
    GPIO_SECURE = GPIO_CTRL2,
    GPIO_CTRL3,
    GPIO_MP = GPIO_CTRL3,
    GPIO_CTRL4,
    GPIO_AP1 = GPIO_CTRL4,
    GPIO_CTRL5,
    GPIO_AP2 = GPIO_CTRL5,
    GPIO_CTRL_MAX
} sdrv_gpio_ctrl_e;

void sdrv_gpio_assign_controller(unsigned int pin, sdrv_gpio_ctrl_e ctrl);
void sdrv_gpio_init(gpio_dev_t *dev, paddr_t base, int irq_cnt,
                    const uint32_t *irqs);

#endif /* _SDRV_GPIO_H */
