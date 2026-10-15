/**
 * @file uart.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef UART_H
#define UART_H

#include <types.h>
#include <cmsis_os2.h>

#define UART_AUTO_BAUD           0xffffffff

/**
 * @brief Number of bits in one character.
 */
typedef enum uart_dls {
    UART_CHAR_5_BITS,
    UART_CHAR_6_BITS,
    UART_CHAR_7_BITS,
    UART_CHAR_8_BITS
} uart_dls_t;

/**
 * @brief Number of stop bits.
 */
typedef enum uart_stop {
    UART_STOP_1_BIT,
    UART_STOP_1_5_BIT,
    UART_STOP_2_BIT
} uart_stop_t;

/**
 * @brief UART parity type.
 */
typedef enum uart_parity {
    UART_NO_PARITY,
    UART_ODD_PARITY,
    UART_EVEN_PARITY
} uart_parity_t;

/**
 * @brief UART configurations.
 */
typedef struct uart_config {
    paddr_t         base;
    int             irq;
    uint32_t        baud;
    uint32_t        clk_freq;
    uart_dls_t      data_bits;
    uart_stop_t     stop_bits;
    uart_parity_t   parity;
    bool            hw_inited;
    bool            dma_en;
    uint8_t         txwmlvl;    // byte: txwmlvl + 1
    uint8_t         rxmwlvl;    // byte: rxwmlvl + 1
} uart_config_t;

typedef void (*uart_cb_t)(void *arg);

typedef struct uart_dev uart_dev_t;

/* uart operation */
typedef struct uart_ops {
    void (*earlyinit)(uart_dev_t *dev, const uart_config_t *cfg);
    void (*init)(uart_dev_t *dev);
    void (*deinit)(uart_dev_t *dev);
    void (*set_baudrate)(uart_dev_t *dev, uint32_t baud);
    uint8_t (*getc)(uart_dev_t *dev);
    void (*putc)(uart_dev_t *dev, uint8_t c);
    void (*raw_putc)(uart_dev_t *dev, uint8_t c);
    void (*transmit)(uart_dev_t *dev, const char *data,
                     size_t size);
    void (*raw_transmit)(uart_dev_t *dev, const char *data,
                     size_t size);
    void (*receive)(uart_dev_t *dev, char *data, size_t size);
    size_t (*transmit_async)(uart_dev_t *dev, const char *data,
                             size_t size);
    size_t (*receive_async)(uart_dev_t *dev, char *data,
                            size_t size);
    void (*enable_txint)(uart_dev_t *dev, bool enable);
    void (*enable_rxint)(uart_dev_t *dev, bool enable);
    void (*enable_rxfovf)(uart_dev_t *dev, bool enable);
} uart_ops_t;

/**
 * @brief DW UART low level device.
 */
typedef struct uart_dev {
    paddr_t             base;
    int                 irq;
    osMutexId_t         lock;

    uint32_t            baud;
    uint32_t            clk_freq;
    const char          *tx_ptr;    /* pointer to current Tx character */
    size_t              tx_remain;  /* number of remaining Tx characters */
    uart_cb_t           tx_cb;      /* tx callback */
    void                *tx_arg;    /* tx callback arg */

    char                *rx_ptr;    /* pointer to current Rx character */
    size_t              rx_remain;  /* number of remaining Rx characters */
    uart_cb_t           rx_cb;      /* rx callback */
    void                *rx_arg;    /* rx callback arg */

    uart_cb_t           rxfovf_cb;      /* rx fifo overflow callback */
    void                *rxfovf_arg;    /* rx fifo overflow callback arg */

    const uart_ops_t    *ops;
    void                *priv;
} uart_dev_t;

/**
 * @brief Asynchronous UART Rx.
 */
bool uart_receive_async(uart_dev_t *device, char *data,
                        size_t size, uart_cb_t rx_cb);

/**
 * @brief UART receive fifo overflow enable and set callback func.
 */
void uart_receive_fovf_enable(uart_dev_t *device, bool enable, uart_cb_t rxfovf_cb);
/**
 * @brief Asynchronous UART Rx ISR.
 */
bool uart_receive_async_isr(uart_dev_t *device, char *data,
                        size_t size, uart_cb_t rx_cb);

/**
 * @brief Asynchronous UART Tx.
 */
bool uart_transmit_async(uart_dev_t *device, const char *data,
                         size_t size, uart_cb_t tx_cb);

/**
 * @brief Configure baud rate.
 */
void uart_set_baudrate(uart_dev_t *device, uint32_t baud);

/**
 * @brief Synchronous UART Rx.
 */
void uart_receive(uart_dev_t *device, char *data, size_t size);

/**
 * @brief Synchronous UART Tx.
 */
void uart_transmit(uart_dev_t *device, const char *data,
                   size_t size);

void uart_raw_transmit(uart_dev_t *device, const char *data,
                   size_t size);

uint8_t uart_getc(uart_dev_t *device);

void uart_putc(uart_dev_t *device, uint8_t c);

void uart_raw_putc(uart_dev_t *device, uint8_t c);

/**
 * @brief Early UART hardware initialization.
 */
void uart_earlyinit(uart_dev_t *device, const uart_config_t *cfg);

void uart_init(uart_dev_t *device);

void uart_deinit(uart_dev_t *device);

/**
 * @brief UART receive chars, called by irq handler.
 */
void uart_receive_chars(uart_dev_t *device);

/**
 * @brief UART receive fifo overflow, called by irq handler.
 */
void uart_receive_fovf(uart_dev_t *device);

/**
 * @brief UART send chars, called by irq handler.
 */
void uart_xmit_chars(uart_dev_t *device);

#endif /* UART_H */
