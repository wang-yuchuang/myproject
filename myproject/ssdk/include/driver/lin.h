/**
 * @file lin.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef LIN_H
#define LIN_H

#include <types.h>
#include <cmsis_os2.h>
#include <driver/clk.h>

#define LIN_OK                  0
#define LIN_N_OK                1

#define LIN_PDU_LENGTH          8   /* data size */
#define LIN_FRAME_DATA_LENGTH   9   /* data(8) + checksum(1) */

#define LIN_AUTO_BAUD           0xffffffff

/**
 * @brief LIN mode.
 */
typedef enum lin_mode {
    LIN_MASTER,
    LIN_SLAVE
} lin_mode_t;

/**
 * @brief LIN status.
 */
typedef enum lin_status {
    /** LIN frame operation return value.
     *  Development or production error occurred */
    LIN_NOT_OK,

    /** LIN frame operation return value.
     *  Successful transmission. */
    LIN_TX_OK,

    /** LIN frame operation return value.
     *  Ongoing transmission (Header or Response). */
    LIN_TX_BUSY,

    /** LIN frame operation return value.
     *  Erroneous header transmission such as:
     *  - Mismatch between sent and read back data
     *  - Identifier parity error or
     *  - Physical bus error */
    LIN_TX_HEADER_ERROR,

    /** LIN frame operation return value.
     *  Erroneous response transmission such as:
     *  - Mismatch between sent and read back data
     *  - Physical bus error */
    LIN_TX_ERROR,

    /** LIN frame operation return value.
     *  Reception of correct response. */
    LIN_RX_OK,

    /** LIN frame operation return value. Ongoing reception: at
     *  least one response byte has been received, but the
     *  checksum byte has not been received. */
    LIN_RX_BUSY,

    /** LIN frame operation return value.
     *  Erroneous response reception such as:
     *  - Framing error
     *  - Overrun error
     *  - Checksum error or
     *  - Short response */
    LIN_RX_ERROR,

    /** LIN frame operation return value.
     *  No response byte has been received so far. */
    LIN_RX_NO_RESPONSE,

    /** LIN channel state return value.
     *  Normal operation; the related LIN channel is ready to
     *  transmit next header. No data from previous frame
     *  available (e.g. after initialization) */
    LIN_OPERATIONAL,

    /** LIN channel state return value.
     *  Sleep mode operation; in this mode wake-up detection
     *  from slave nodes is enabled. */
    LIN_CH_SLEEP
} lin_status_t;

/**
 * @brief LIN checksum mode.
 */
typedef enum lin_cs_mode {
    LIN_ENHANCED_CS  = 0,   /* Enhanced checksum */
    LIN_CLASSIC_CS = 1      /* Classic checksum */
} lin_cs_mode_t;

/**
 * @brief LIN response type.
 */
typedef enum lin_response_type {
    LIN_MASTER_RESPONSE = 0, /* Response is generated from this (master) node */
    LIN_SLAVE_RESPONSE  = 1, /* Response is generated from a remote slave node */
    LIN_SLAVE_TO_SLAVE  = 2  /* Response is generated from one slave to another slave */
} lin_response_type_t;

/**
 * @brief LIN Channel status.
 */
typedef enum lin_ch_status {
    LIN_CH_SLEEP_PENDING,
    LIN_CH_SLEEP_STATE,
    LIN_CH_OPERATIONAL
} lin_ch_status_t;

/**
 * @brief LIN pdu info.
 */
typedef struct lin_pdu {
    uint8_t         pid;
    uint8_t         cs;
    uint8_t         drc;
    uint8_t         dl;
    uint8_t         *sdu;
} lin_pdu_t;

/**
 * @brief LIN controller configure.
 */
typedef struct lin_config {
    uint32_t        hwid;
    paddr_t         base;
    int             irq;
    uint32_t        baud;
    clk_node_t      *clk;
    lin_mode_t      mode;
    lin_cs_mode_t   cs_mode;  // only for slave
} lin_config_t;


typedef struct lin_dev lin_dev_t;

/**
 * @brief LIN operation.
 */
typedef struct lin_ops {
    void (*init)(lin_dev_t *dev);
    void (*deinit)(lin_dev_t *dev);
    uint8_t (*wake_up)(lin_dev_t *dev);
    uint8_t (*send_frame)(lin_dev_t *dev, const lin_pdu_t *pdu);
    lin_status_t (*get_status)(lin_dev_t *dev, uint8_t **sdu_ptr);
    uint8_t (*abort)(lin_dev_t *dev);
} lin_ops_t;

/**
 * @brief LIN device.
 */
typedef struct lin_dev {
    uint32_t            hwid;
    paddr_t             base;
    int                 irq;
    uint32_t            baud;
    clk_node_t          *clk;
    uint32_t            clk_freq;
    lin_mode_t          mode;
    lin_cs_mode_t       cs_mode;

    osMutexId_t         lock;
    lin_ch_status_t     ch_status;

    uint32_t            timeout;

    const lin_ops_t    *ops;

    void               *priv;
} lin_dev_t;

/**
 * @brief LIN init.
 *
 * @param[in] device lin dev
 * @param[in] config lin config
 */
void lin_init(lin_dev_t *device, const lin_config_t *config);

/**
 * @brief LIN deinit.
 *
 * @param[in] device lin dev
 */
void lin_deinit(lin_dev_t *device);

/**
 * @brief LIN send frame.
 *
 * @param[in] device lin dev
 * @param[in] pdu send pdu info
 * @return send result
 */
uint8_t lin_send_frame(lin_dev_t *device, const lin_pdu_t *pdu);

/**
 * @brief LIN abort send frame.
 *
 * @param[in] device lin dev
 * @return abort result
 */
uint8_t lin_abort(lin_dev_t *device);

/**
 * @brief LIN check wakeup.
 *
 * @param[in] device lin dev
 * @return check wakeup result
 */
uint8_t lin_check_wakeup(lin_dev_t *device);

/**
 * @brief LIN goto sleep.
 *
 * @param[in] device lin dev
 * @return goto sleep result
 */
uint8_t lin_goto_sleep(lin_dev_t *device);

/**
 * @brief LIN goto sleep internal.
 *
 * @param[in] device lin dev
 * @return goto sleep result
 */
uint8_t lin_goto_sleep_internal(lin_dev_t *device);

/**
 * @brief LIN wakeup.
 *
 * @param[in] device lin dev
 * @return wakeup result
 */
uint8_t lin_wakeup(lin_dev_t *device);

/**
 * @brief LIN wakeup internal.
 *
 * @param[in] device lin dev
 * @return wakeup result
 */
uint8_t lin_wakeup_internal(lin_dev_t *device);

/**
 * @brief LIN get status.
 *
 * @param[in] device lin dev
 * @param[out] sdu_ptr send or receive sdu
 * @return status
 */
lin_status_t lin_get_status(lin_dev_t *device, uint8_t **sdu_ptr);

#endif /* LIN_H */
