/******************************************************
 *   @file    Iram_Ip.h
 *
 *   @brief   IRAM(Internal SRAM) header file.
 *   @details
 *
 *******************************************************/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#ifndef _IRAM_IP_H_
#define _IRAM_IP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cdefs.h"
#include "Std_Types.h"
#include "CompilerEx.h"
#include "Iram_Ctrl_reg.h"
//#include "common_hdr.h"

#define BANK_READ_DATA_DECODING0_INJ_OFFSET(bank_index, byte_index) ((BANK0_DATA_31_0_INJ_OFF) + ((bank_index)*16) + ((byte_index)*4))
#define BANK_READ_ECC_DECODING0_INJ_OFFSET(bank_index)  ((BANK0_ECC_INJ_OFF) + ((bank_index)*4))
#define BANK_READ_DATA_DECODING1_INJ_OFFSET(byte_index) ((RDATA_31_0_INJ_OFF) + ((byte_index)*4))
#define BANK_READ_ECC_DECODING1_INJ_OFFSET  (RECC_INJ_OFF)
#define BANK_WRITE_DATA_DECODING1_INJ_OFFSET(byte_index) ((WDATA_31_0_INJ_OFF) + ((byte_index)*4))
#define BANK_WRITE_ECC_DECODING1_INJ_OFFSET (WECC_INJ_OFF)

#define  IRAMC_REG_BIT_NUM                        (32U)                   /*Bits number of one iramc register*/
#define  INJECT_IRAM_1BITERR                      (1U)                    /*Inject 1 bit error for iram*/
#define  INJECT_IRAM_2BITERR                      (2U)                    /*Inject 2 bits error for iram*/


typedef enum {
    IRAMCtrl1 = 0U,
    IRAMCtrl2,
    IRAMCtrl3,
    IRAMCtrl4
} IramCtlType;

typedef enum {
    ONE_BIT_IRAM_ERR = 0U,
    TWO_BIT_IRAM_ERR
} Iram_Error_BitType;

typedef enum {
    RDDATADE0 = 0U,    /*Inject data error in read path decoding0*/
    RDECCDE0,          /*Inject ecc error in read path decoding0*/
    RDDATADE1,         /*Inject data error in read path decoding1*/
    RDECCDE1,          /*Inject eccerror in read path decoding1*/
    WRDATADE1,         /*Inject data error in write path decoding1*/
    WRECCDE1           /*Inject ecc error in write path decoding1*/
} IramInjErrType;

typedef enum {
    MEM_ERR_STA = 0U,   /*Iram memory  interrupt status enable when inject error in iram memory*/
    MEM_ERR_SIG,        /*Iram memory  interrupt signal enable when inject error in iram memory*/
    UNCOR_ERR_STA,      /*Iram uncorrect interrupt status when inject error in AXI bus*/
    UNCOR_ERR_SIG,      /*Iram uncorrect interrupt signal when inject error in AXI bus*/
    COR_ERR_STA,        /*Iram correct interrupt status when inject error in AXI bus*/
    COR_ERR_SIG         /*Iram correct interrupt signal when inject error in AXI bus*/
} IramEnStaType;

typedef enum {
    IRAM_MEM_INT = 0U,  /*Iram memory interrupt status when inject error in iram memory*/
    IRAM_UNCORR_INT,    /*Iram uncorrct interrupt status when inject error in AXI bus*/
    IRAM_CORR_INT       /*Iram correct interrupt status when inject error in AXI bus*/
} IramStaType;

/* Get Iram Controller address*/
FUNC(void, IRAMC_CODE) IramCtrl_GetBaseAddr(void);

/*Inject data/ecc error in Iram memory*/
FUNC(void, IRAMC_CODE) Iram_Error_Injection
(
    VAR(Iram_Error_BitType, AUTOMATIC)       Error_bit,
    VAR(IramInjErrType, AUTOMATIC)           Iram_Inj_Err,
    VAR(IramCtlType, AUTOMATIC)              IramCtl,
    VAR(uint8, AUTOMATIC)                    Bank_num,
    VAR(uint8, AUTOMATIC)                    Inject_bit,
    VAR(boolean, AUTOMATIC)                  Enable
)_SECTION(.tcm_func);

/* Enable interrupt status for iram injection error*/
FUNC(void, IRAMC_CODE) Iram_Inject_Err_Int_StaSig_En
(
    VAR(IramEnStaType, AUTOMATIC)  Iram_En_Sta,
    VAR(IramCtlType, AUTOMATIC)    IramCtl,
    VAR(uint8, AUTOMATIC)          Start_Bit,
    VAR(uint8, AUTOMATIC)          Bit_Num,
    VAR(uint32, AUTOMATIC)         Value
)_SECTION(.tcm_func);

/* Get interrupt status for inject error*/
FUNC(boolean, IRAMC_CODE) Get_Iram_Inj_Err_Int_Status
(
    VAR(IramStaType, AUTOMATIC)  Iram_Sta,
    VAR(IramCtlType, AUTOMATIC)  IramCtl,
    VAR(uint32, AUTOMATIC)       Signal_bit
)_SECTION(.tcm_func);

/* Clear interrupt status for inject error*/
FUNC(void, IRAMC_CODE) Clr_Iram_Inj_Err_Int_Status
(
    VAR(IramStaType, AUTOMATIC)  Iram_Sta,
    VAR(IramCtlType, AUTOMATIC)  IramCtl,
    VAR(uint32, AUTOMATIC)       Signal_bit
)_SECTION(.tcm_func);

#ifdef __cplusplus
}
#endif

#endif
