/**
 * @file vlin.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef VLIN_H
#define VLIN_H

#include <types.h>
#include <driver/lin.h>

#define E_OK            0
#define E_NOT_OK        1

#define LIN_INVALID_ID  0x0
#define LIN_UART8_ID    0x40428016
#define LIN_UART7_ID    0x40427015
#define LIN_UART6_ID    0x40426014
#define LIN_UART5_ID    0x40425013
#define LIN_UART4_ID    0x40424012
#define LIN_UART3_ID    0x40423011
#define LIN_UART2_ID    0x40422010
#define LIN_UART1_ID    0x4042100F

typedef enum {
    LIN_IFC_SCI1,
    LIN_IFC_SCI2,
    LIN_IFC_SCI3,
    LIN_IFC_SCI4,
    LIN_IFC_CHN_MAX,
} Lin_IfcChannel;

//data length select
typedef enum {
    LIN_CHAR_5BITS = 0,
    LIN_CHAR_6BITS,
    LIN_CHAR_7BITS,
    LIN_CHAR_8BITS,
} Lin_CharBits;

//stop bits
typedef enum {
    LIN_STOP_1BIT = 0,
    LIN_STOP_1_5BIT,
    LIN_STOP_2BIT,
} Lin_StopBits;

//parity bits
typedef enum {
    LIN_ODD_PARITY = 0,
    LIN_EVEN_PARITY,
    LIN_NO_PARITY,
} Lin_ParityMode;

typedef enum {
    LIN_RX_FIFO_CHAR_1 = 0,    //0: 1 character in FIFO
    LIN_RX_FIFO_QUARTER_FULL,  //1: FIFO 1/4 full
    LIN_RX_FIFO_HALF_FULL,     //2: FIFO 1/2 full
    LIN_RX_FIFO_FULL_2,        //3: FIFO 2 less than full
} Lin_FifoRxTrigger;

typedef enum {
    LIN_TX_FIFO_EMPTY = 0,     //0: FIFO Empty
    LIN_TX_FIFO_CHA_2,         //1: 2 characters in FIFO
    LIN_TX_FIFO_QUARTER_FULL,  //2: FIFO 1/4 full
    LIN_TX_FIFO_HALF_FULL,     //3: FIFO 1/2 full
} Lin_FifoTxTrigger;

typedef uint8_t Lin_FramePidType;
typedef uint8_t Lin_FrameDlType;
typedef uint8_t Std_ReturnType;
typedef lin_status_t Lin_StatusType;
typedef lin_cs_mode_t Lin_FrameCsModelType;
typedef lin_response_type_t Lin_FrameResponseType;

typedef struct Lin_PduType {
    uint8_t Pid;
    uint8_t Cs;
    uint8_t Drc;
    uint8_t Dl;
    uint8_t *SduPtr;
} Lin_PduType;

typedef struct Lin_ControllerConfigType {
    uint32_t hrdChannel;
    uint32_t sclk;
    uint32_t baud;
    uint8_t data_bits;
    uint8_t stop_bits;
    uint8_t parity;
    uint8_t loopback_enable;
    uint8_t fifo_enable;
    uint8_t rx_trigger;
    uint8_t tx_trigger;
} Lin_ControllerConfigType;

typedef struct Lin_ConfigType {
    uint32_t Count;
    Lin_ControllerConfigType *Config;
} Lin_ConfigType;

/**
 * @brief LIN init.
 *
 * @param[in] device lin dev
 * @param[in] config lin config
 */
void Lin_Init(const Lin_ConfigType *Config);

/**
 * @brief LIN deinit.
 *
 * @param[in] device lin dev
 */
void Lin_DeInit(void);

/**
 * @brief LIN send frame.
 *
 * @param[in] device lin dev
 * @param[in] pdu send pdu info
 * @return send result
 */
Std_ReturnType Lin_SendFrame(uint8_t Channel,
                             const Lin_PduType *PduInfoPtr);

/**
 * @brief LIN check wakeup.
 *
 * @param[in] device lin dev
 * @return check wakeup result
 */
Std_ReturnType Lin_CheckWakeup(uint8_t Channel);

/**
 * @brief LIN goto sleep.
 *
 * @param[in] device lin dev
 * @return goto sleep result
 */
Std_ReturnType Lin_GoToSleep(uint8_t Channel);

/**
 * @brief LIN goto sleep internal.
 *
 * @param[in] device lin dev
 * @return goto sleep result
 */
Std_ReturnType Lin_GoToSleepInternal(uint8_t Channel);

/**
 * @brief LIN wakeup.
 *
 * @param[in] device lin dev
 * @return wakeup result
 */
Std_ReturnType Lin_Wakeup(uint8_t Channel);

/**
 * @brief LIN wakeup internal.
 *
 * @param[in] device lin dev
 * @return wakeup result
 */
Std_ReturnType Lin_WakeupInternal(uint8_t Channel);

/**
 * @brief LIN get status.
 *
 * @param[in] device lin dev
 * @param[out] sdu_ptr send or receive sdu
 * @return status
 */
Lin_StatusType Lin_GetStatus(uint8_t Channel, uint8_t **Lin_SduPtr);

#endif /* VLIN_H */
