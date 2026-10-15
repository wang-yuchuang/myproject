/**
 * @file vcan.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef VCAN_H
#define VCAN_H

#include <types.h>

typedef enum flexcan_controller_id {
    CAN1 = 0,
    CAN2 = 1,
    CAN3 = 2,
    CAN4 = 3,
    CAN5 = 4,
    CAN6 = 5,
    CAN7 = 6,
    CAN8 = 7,
    CAN9 = 8,
    CAN10 = 9,
    CAN11 = 10,
    CAN12 = 11,
    CAN13 = 12,
    CAN14 = 13,
    CAN15 = 14,
    CAN16 = 15,
    CAN17 = 16,
    CAN18 = 17,
    CAN19 = 18,
    CAN20 = 19,
    CAN_CTRL_CONFIG_CNT
} flexcan_controller_id_t;

typedef enum flexcan_fd_data_size {
    CAN_FD_8BYTES_PER_MB = 0,       /*!< 8 bytes per message buffer. */
    CAN_FD_16BYTES_PER_MB = 1,      /*!< 16 bytes per message buffer. */
    CAN_FD_32BYTES_PER_MB = 2,      /*!< 32 bytes per message buffer. */
    CAN_FD_64BYTES_PER_MB = 3       /*!< 64 bytes per message buffer. */
} flexcan_fd_data_size_t;

/*! @brief FlexCAN clock source. */
typedef enum flexcan_clock_source {
    FLEXCAN_ClkSrcOsc = 0,          /*!< FlexCAN Protocol Engine clock from Oscillator. */
    FLEXCAN_ClkSrcPeri = 1          /*!< FlexCAN Protocol Engine clock from Peripheral Clock. */
} flexcan_clock_source_t;

typedef enum flexcan_rx_fifo_filter_type {
    FLEXCAN_RxFifoFilterTypeA = 0,  /*!< One full ID (standard and extended) per ID Filter element. */
    FLEXCAN_RxFifoFilterTypeB = 1,  /*!< Two full standard IDs or two partial 14-bit ID slices per ID Filter Table element. */
    FLEXCAN_RxFifoFilterTypeC = 2,  /*!< Four partial 8-bit Standard or extended ID slices per ID Filter Table element. */
    FLEXCAN_RxFifoFilterTypeD = 3   /*!< All frames rejected. */
} flexcan_rx_fifo_filter_type_t;

typedef enum flexcan_rx_fifo_priority {
    FLEXCAN_RxFifoPrioLow = 0,      /*!< Matching process start from Rx Message Buffer first*/
    FLEXCAN_RxFifoPrioHigh = 1      /*!< Matching process start from Rx FIFO first*/
} flexcan_rx_fifo_priority_t;

/*! @brief FlexCAN frame type. */
typedef enum flexcan_frame_type {
    FLEXCAN_FrameTypeData = 0,      /*!< Data frame type attribute. */
    FLEXCAN_FrameTypeRemote = 1     /*!< Remote frame type attribute. */
} flexcan_frame_type_t;

/*! @brief FlexCAN frame format. */
typedef enum flexcan_frame_format {
    FLEXCAN_STANDARD_FRAME = 0,     /*!< Standard frame format attribute. */
    FLEXCAN_EXTEND_FRAME = 1        /*!< Extend frame format attribute. */
} flexcan_frame_format_t;


typedef uint8_t Std_ReturnType;
typedef uint16_t Can_HwHandleType;
typedef uint32_t Can_IdType;

typedef enum {
    CAN_CS_UNINIT = 0U,
    CAN_CS_STARTED,
    CAN_CS_STOPPED,
    CAN_CS_SLEEP,
    CAN_CS_WAKEUP,
    CAN_CS_INIT
} Can_ControllerStateType;

typedef enum {
    CAN_ERRORSTATE_ACTIVE = 0U,
    CAN_ERRORSTATE_PASSIVE,
    CAN_ERRORSTATE_BUSOFF
} Can_ErrorStateType;

typedef struct flexcan_fd_config {
    uint8_t enableISOCANFD;
    uint8_t enableBRS;
    uint8_t enableTDC;
    uint8_t TDCOffset;
    flexcan_fd_data_size_t r0_mb_data_size;
    flexcan_fd_data_size_t r1_mb_data_size;
} flexcan_fd_config_t;

typedef struct flexcan_timing_config {
    uint16_t preDivider;
    uint8_t rJumpwidth;
    uint8_t propSeg;
    uint8_t phaseSeg1;
    uint8_t phaseSeg2;
} flexcan_timing_config_t;

typedef struct flexcan_config {
    flexcan_clock_source_t clkSrc;
    uint8_t maxMbNum;
    bool enableLoopBack;
    bool enableListenOnly;
    bool enableSelfWakeup;
    uint8_t enableIndividMask;
    bool enableDoze;
    bool enableCANFD;
    flexcan_timing_config_t nominalBitTiming;
    flexcan_timing_config_t dataBitTiming;
    flexcan_fd_config_t can_fd_cfg;
} flexcan_config_t;

typedef struct flexcan_rx_fifo_filter_table {
    uint32_t filter_code;
    uint32_t filter_mask;
} flexcan_rx_fifo_filter_table_t;

typedef struct flexcan_rx_fifo_config {
    uint8_t idFilterNum;
    flexcan_rx_fifo_filter_type_t idFilterType;
    flexcan_rx_fifo_priority_t priority;
    flexcan_rx_fifo_filter_table_t *filter_tab;
} flexcan_rx_fifo_config_t;

typedef struct rx_mb_frame {
    uint32_t frameId;
    uint32_t rxIDFilterMask;
    flexcan_frame_type_t frameType;
    flexcan_frame_format_t frameFormat;
} rx_mb_frame_t;

typedef struct tx_mb_ctrl {
    uint8_t paddingVal;
} tx_mb_ctrl_t;

typedef struct Can_ControllerConfig {
    uint8_t controllerId;
    uint32_t baseAddr;
    uint32_t irq_num;
    uint32_t  RxProcessType;
    uint32_t  TxProcessType;
    uint32_t  BusoffProcessType;
    uint32_t  WakeupProcessType;
    flexcan_config_t flexcanCfg;
} Can_ControllerConfig;

typedef struct Can_MBConfig {
    uint32_t hwObjId;
    uint8_t controllerId;
    uint8_t mbId;
    uint8_t polling;
    uint16_t CanHwObjectCount;
    uint8_t CanHandleType;
    rx_mb_frame_t rx;
    tx_mb_ctrl_t tx;
} Can_MBConfig;

typedef struct Can_RxFIFOConfig {
    uint32_t hwObjId;
    uint8_t controllerId;
    uint8_t polling;
    flexcan_rx_fifo_config_t flexcanRxFIFOCfg;
} Can_RxFIFOConfig;

typedef struct Can_BaudRateConfig {
    uint8_t controllerId;
    flexcan_timing_config_t nomianlBitTimingCfg;
    flexcan_timing_config_t dataBitTimingCfg;
    flexcan_fd_config_t can_fd_cfg;
} Can_BaudRateConfig;

typedef struct Can_ConfigType {
    uint8_t controllerCount;
    uint8_t rxFifoCount;
    uint8_t baudRateCfgCount;
    uint16_t rxCount;
    uint16_t txCount;
    Can_ControllerConfig *ctrllerCfg;
    Can_MBConfig *rxMBCfg;
    Can_MBConfig *txMBCfg;
    Can_RxFIFOConfig *rxFIFOCfg;
    Can_BaudRateConfig *baudRateCfg;
} Can_ConfigType;

typedef struct Can_PduType {
    uint16_t swPduHandle;
    uint8_t length;
    uint32_t id;
    uint8_t *sdu;
} Can_PduType;

/*
 * @brief Can init.
 *
 * @param[in] config can config.
 */
void Can_Init(const Can_ConfigType* Config);

/*
 * @brief Can Deinit.
 */
void Can_DeInit(void);

/*
 * @brief Can set baudrate.
 *
 * @param[in] Controller controller id.
 * @param[in] BaudRateConfigID baudrate id.
 * @return result.
 */
Std_ReturnType Can_SetBaudrate(uint8_t Controller, uint16_t BaudRateConfigID);

/*
 * @brief Can set mode.
 *
 * @param[in] Controller controller id.
 * @param[in] Transition state type.
 * @return result.
 */
Std_ReturnType Can_SetControllerMode(uint8_t Controller, Can_ControllerStateType Transition);

/*
 * @brief Can disable interrupt.
 *
 * @param[in] Controller controller id.
 */
void Can_DisableControllerInterrupts(uint8_t Controller);

/*
 * @brief Can enable interrupt.
 *
 * @param[in] Controller controller id.
 */
void Can_EnableControllerInterrupts(uint8_t Controller);

/*
 * @brief Can check wakeup.
 *
 * @param[in] Controller controller id.
 */
void Can_CheckWakeup(uint8_t Controller);

/*
 * @brief Can get error state.
 *
 * @param[in] ControllerId controller id.
 * @param[out] ErrorStatePtr error state.
 * @return result.
 */
Std_ReturnType Can_GetControllerErrorState(uint8_t Controller, Can_ErrorStateType* ErrorStatePtr);

/*
 * @brief Can get controller mode.
 *
 * @param[in] ControllerId controller id.
 * @param[out] ControllerModePtr controller mode.
 * @return result.
 */
Std_ReturnType Can_GetControllerMode(uint8_t Controller, Can_ControllerStateType* ControllerModePtr);

/*
 * @brief Can write.
 *
 * @param[in] Hth Hth id.
 * @param[out] PduInfo pdu info.
 * @return result.
 */
Std_ReturnType Can_Write(Can_HwHandleType Hth, const Can_PduType* PduInfo);

/*
 * @brief Can main function read.
 */
void Can_MainFunction_Read(void);

/*
 * @brief Can main function write.
 */
void Can_MainFunction_Write(void);

#endif
