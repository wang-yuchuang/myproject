/***********************************************************************************************
 *   @file    Fuse_Ip.h
 *
 *   @brief   fuse  file.
 *   @details
 *             Description: Fuse controller provides interface for TSMC fuse macro access.
 *             It can provide ECC or redundancy function to detect or recover fuse bits error
 *             in fuse macro.
 *
 *************************************************************************************************/

/*************************************************************************************************
 *        Copyright(c) 2020    Semidrive                                                         *
 *        All rights reserved.                                                                   *
 *************************************************************************************************/
#ifndef _FUSE_IP_H_
#define _FUSE_IP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CompilerEx.h"
#include "Std_Types.h"
#include "fuse_ctrl_reg.h"

#define FUSE_SIG_ERR_ADDR_OFF(position) \
                              (ECC_SIG_ERROR0_OFF + (position/32)*4)

#define FUSE_MUL_ERR_ADDR_OFF(position) \
                              (ECC_MUL_ERROR0_OFF + (position/32)*4)

#define FUSE_ECC_DATA_CORR(position) \
                            (ECC_DATA_CORR_0_OFF + ((position)*8U))

#define FUSE_ECC_CORR(position) \
                            (ECC_CORR_0_OFF + ((position)*8U))

#define Inject_Err_Mode_Nm (0x06U)

#define FUSE_MISC_CFG_ADDR_OFF(misc_cfg_id) \
                            (0x12A4 + ((misc_cfg_id/32)*4))

/*Error bit injection type: 1/2bits*/
typedef enum {
    ONE_BIT_FUSE_ERR = 0U,
    TWO_BIT_FUSE_ERR
} Fuse_Error_BitType;

typedef enum {
    RDDATAECC = 0U, /*Error injection for read data for ECC mode*/
    ECCCODEECC,     /*Error injection for ECC code for ECC mode*/
    RDDATARED,      /*Error injection for read data for redundancy mode*/
    REDDATARED,     /*Error injection for redundancy data for redundancy mode*/
    RDCORRDATAECC,  /*Error injection for read correction data for ECC mode*/
    ECCCORRCODEECC  /*Error injection for ECC correction code for ECC mode*/
} FuseInjectErrType;

typedef enum {
    RDDATAECCCOR = 0U, /*Error injection for read data for ECC mode*/
    ECCCODEECCCOR,     /*Error injection for ECC code for ECC mode*/
    RDDATAREDCOR,      /*Error injection for read data for redundancy mode*/
    REDDATAREDCOR      /*Error injection for redundancy data for redundancy mode*/
} FuseCorrectErrType;

typedef enum {
    SIGERR = 0U,       /*Single error for ECC mode*/
    MULERR,            /*Multiple error for ECC mode*/
    REDERR,            /*Redundancy error for redundancy mode*/
    LOCKREDERR,        /*Lock redundancy error for redundancy mode*/
    NONERR             /*None error*/
} FuseErrStaType;

typedef enum {
    FUSE_INT_STA = 0U,   /*Fuse interrupt status enable*/
    FUSE_INT_SIG         /*Fuse interrupt signal enable*/
} FuseEnStaType;

/* Get value from Fuse Ctrl eg:error or busy status*/
FUNC(boolean, FUSE_CODE) Get_Fuse_Ctrl_Value
(
    VAR(uint32, AUTOMATIC) signal_bit
);

FUNC(void, AUTOMATIC) Clr_Fuse_Interrupt_Status
(
    VAR(uint32, AUTOMATIC) signal_bit
);

FUNC(void, FUSE_CODE) Set_Fuse_Trig_Value
(
    VAR(boolean,  AUTOMATIC) enable
);

FUNC(boolean, FUSE_CODE) Get_Fuse_Trig_Value
(
    VAR(uint32, AUTOMATIC) signal_bit
);

FUNC(void, FUSE_CODE) Set_Fuse_Sema_Lock
(
    VAR(uint8,  AUTOMATIC) enable
);

/*Get sigle error status for function fuse 0~159(8~167) for ECC mode
  Get multile error status for function fuse1 0~159(8~167) for ECC mode
  Get red error status for function fuse2 0~19(168~187)
  Get lock red error status for lock function fuse 0~7(188~195)
*/
FUNC(boolean, FUSE_CODE) Get_Fuse_Error_Status
(
    VAR(FuseErrStaType, AUTOMATIC) Err_Sta,
    VAR(uint8, AUTOMATIC) wordPosition
);

/*Clr sigle error status for function fuse 0~159(8~167) for ECC mode
  Clr multile error status for function fuse1 0~159(8~167) for ECC mode
  Clr red error status for function fuse2 0~19(168~187)
  Clr lock red error status for lock function fuse 0~7(188~195)
*/
FUNC(void, FUSE_CODE) Clr_Fuse_Error_Status
(
    VAR(FuseErrStaType, AUTOMATIC) Err_Sta,
    VAR(uint8, AUTOMATIC) wordPosition
);

/*Check bit if corrected 0~31bit:RdData RdEcc RedData RedEcc*/
FUNC(boolean, FUSE_CODE) Check_Fuse_ECC_Data_Bit_Corr
(
    VAR(FuseCorrectErrType, AUTOMATIC) cor_err_mod,
    VAR(uint8, AUTOMATIC) wordPosition,
    VAR(uint8, AUTOMATIC) wordBit
);

FUNC(boolean, AUTOMATIC) Get_Fuse_Interrupt_Status
(
    VAR(uint32, AUTOMATIC) signal_bit
);

/*Enable fuse interrupt signal/status*/
FUNC(void, FUSE_CODE) Fuse_Interrupt_SigSta_Enable
(
    VAR(FuseEnStaType, AUTOMATIC)  Fuse_En_Sta,
    VAR(uint8, AUTOMATIC)          Start_Bit,
    VAR(uint8, AUTOMATIC)          Bit_Num,
    VAR(uint32, AUTOMATIC)         Value    
);

/* Set ECC fuse Addr 196~235 */
FUNC(void, FUSE_CODE) Set_Check_Fuse_Addr
(
    VAR(uint8, AUTOMATIC) wordPosition
);

/*Inject 1/2 bits error enable*/
FUNC(void, FUSE_CODE) Fuse_Inject_Error_En
(
    VAR(Fuse_Error_BitType, AUTOMATIC) Error_bit,
    VAR(FuseInjectErrType, AUTOMATIC) InjectErrMode,
    VAR(uint8, AUTOMATIC)   wordBit,
    VAR(boolean, AUTOMATIC) enable
);


#ifdef __cplusplus
}
#endif

#endif
