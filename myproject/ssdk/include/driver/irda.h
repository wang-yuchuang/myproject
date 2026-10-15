/**
 * @file irda.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef IRDA_H
#define IRDA_H

#include <types.h>
#include <cmsis_os2.h>

#define IRDA_OK                 0
#define IRDA_N_OK               1

#define IRDA_WAIT_FOREVER       0xffffffff

/**
 * @brief IrDA configure.
 */
typedef struct irda_config {
    paddr_t         base;
    int             irq;
    uint32_t        carrier_freq;
    uint32_t        clk_freq;
} irda_config_t;

typedef void (*irda_cb_t)(void *arg, uint8_t result);

typedef struct irda_dev irda_dev_t;

/**
 * @brief IrDA operation.
 */
typedef struct irda_ops {
    void (*init)(irda_dev_t *dev, const irda_config_t *config);
    void (*deinit)(irda_dev_t *dev);
    uint8_t (*send)(irda_dev_t *dev, const char *data,
                    uint8_t len, uint32_t timeout);
    uint8_t (*send_async)(irda_dev_t *dev, const char *data,
                          uint8_t len, irda_cb_t cb, void *arg);
    uint8_t (*recv)(irda_dev_t *dev, char *data,
                    uint8_t len, uint32_t timeout);
    uint8_t (*recv_async)(irda_dev_t *dev, char *data,
                          uint8_t len, irda_cb_t cb, void *arg);
} irda_ops_t;

/**
 * @brief IrDA device.
 */
typedef struct irda_dev {
    paddr_t             base;
    int                 irq;
    uint32_t            carrier_freq;
    uint32_t            clk_freq;
    osMutexId_t         lock;
    const irda_ops_t    *ops;
} irda_dev_t;

/**
 * @brief IrDA init.
 *
 * @param[in] device irda dev
 * @param[in] config irda config
 */
void irda_init(irda_dev_t *device, const irda_config_t *config);

/**
 * @brief IrDA deinit.
 *
 * @param[in] device irda dev
 */
void irda_deinit(irda_dev_t *device);

/**
 * @brief IrDA send data.
 *
 * @param[in] device irda dev
 * @param[in] data send data address
 * @param[in] len send data len
 * @param[in] timeout send timeout value
 * @return send result
 */
uint8_t irda_send(irda_dev_t *device, const char *data,
                  uint8_t len, uint32_t timeout);

/**
 * @brief IrDA send data in aync mode.
 *
 * @param[in] device irda dev
 * @param[in] data send data address
 * @param[in] len send data len
 * @param[in] cb callback function
 * @param[in] arg callback arg
 * @return send result
 */
uint8_t irda_send_async(irda_dev_t *device, const char *data,
                        uint8_t len, irda_cb_t cb, void *arg);

/**
 * @brief IrDA receive data.
 *
 * @param[in] device irda dev
 * @param[in] data receive data buffer address
 * @param[in] len receive data len
 * @param[in] timeout receive timeout value
 * @return receive result
 */
uint8_t irda_receive(irda_dev_t *device, char *data,
                     uint8_t len, uint32_t timeout);

/**
 * @brief IrDA receive data in aync mode.
 *
 * @param[in] device irda dev
 * @param[in] data receive data buffer address
 * @param[in] len receive data len
 * @param[in] cb callback function
 * @param[in] arg callback arg
 * @return receive result
 */
uint8_t irda_receive_async(irda_dev_t *device, char *data,
                           uint8_t len, irda_cb_t cb, void *arg);

#endif /* IRDA_H */
