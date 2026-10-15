/*********************************************************************************
 *   @file    Sem_Ip.h
 *
 *   @brief   SEM(System Error Manager) header file.
 *   @details
 *             There're 2 SEM modules, i.e., SEM1 and SEM2. They're same IPs with
 *             same input signals and configuration interfaces. The difference is
 *             SEM2 outputs to GICs only, while SEM1 has 8 output targets.
 *
 **********************************************************************************/

/***********************************************************************************
 *        Copyright(c) 2020    Semidrive                                           *
 *        All rights reserved.                                                     *
 ***********************************************************************************/
#ifndef _SEM_IP_H_
#define _SEM_IP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "CompilerEx.h"
#include "sem_hw.h"
#include "Std_Types.h"
//#include "common_hdr.h"

#define COMMON_SET                      (0x0U)  /* SEM common settings.*/
#define INT_ENABLE_OVRD_OFFSET          (0x08U) /* interrupt enable override offset.*/
#define INT_ENABLE_OVRD_EN              (0x0U)  /* interrupt enable override enable.*/
#define INT_ENABLE_OVRD_VAL             (0x08U) /* interrupt enable override value.*/

#define ERROR_WAVE_SEL                  (0x0BU) /* waveform selection for normal/recoverable error. */
#define EI_GLOBAL_EN                    (0x10U) /* error injection global enable. */
#define EI_GLOBAL_EN_LOCK               (0x11U) /* error injection global enable lock bit. */
#define SIG_MON_GLB_EN                  (0x17)  /* signal monitor global enable. */
#define CMP_ENABLE                      (0x18)  /* compare output enable bits for int 0~7. */

#define SEM_INT_ENABLE(output, input)   (0x100U + ((output) * 0x80) + ((input) / 32 * 4))
#define SEM_INT_STATUS(input)           (0x80U  + ((input) / 32 * 4))
#define SEM_TGT_VALUE(signal_id)        (0x500U + (signal_id) / 32 * 4)
#define SEM_MON_ENABLE(signal_id)       (0x580U + (signal_id) / 32 * 4)

/*Sem input interrupt total numbers*/
#define SEM_INPUT_INT_TOTAL             319U
/*Enable sem interrupt output to cpu*/
#define SEM_INT2_ENABLE                 (0x200U)

typedef enum {
    SEMgr1 = 0x0U,
    SEMgr2
} Sem_Id;

/* SEM interrupt output targets*/
typedef enum {
    SEM_OUTPUT_PAD0 = 0, /* PAD0, recoverable error. */
    SEM_OUTPUT_PAD1,     /* PAD1, fatal error. */
    SEM_OUTPUT_CPU,      /* GIC 1~5. Note that SEM2 has only GIC connection. */
    SEM_OUTPUT_SYS,      /* global reset. */
    SEM_OUTPUT_DC0,      /* DC output 0. */
    SEM_OUTPUT_DC1,      /* DC output 1. */
    SEM_OUTPUT_DC2,      /* DC output 2. */
    SEM_OUTPUT_DC3       /* DC output 3. */
} Sem_Int_Output;

/* Get SEM address*/
void Sem_Ip_GetBaseAddr(void);

/* Enable interrupt override and set value*/
void Sem_Interrupt_Ovrd_Enable
(
    Sem_Id semid,
    Sem_Int_Output output,
    boolean enable
);

/* Enable interrupt for sem output*/
void Sem_Interrupt_Enable
(
    Sem_Id semid,
    Sem_Int_Input  input,
    Sem_Int_Output output,
    boolean enable
);

/* Get interrupt status for sem */
boolean Sem_Interrupt_Status
(
    Sem_Id semid,
    Sem_Int_Input input
);

/* Read-Modify-Write for sem reg*/
void Sem_Read_Modify_Write_Reg
(
    uint32 reg_addr,
    uint32 mask,
    uint32 val
);

/* Sem enable error injection*/
void Sem_Enable_Eic
(
    Sem_Id semid,
    boolean enable
);

/*Set target level (high or low) for specified signal.*/
void Sem_Monitor_Set_Sig
(
    Sem_Id semid,
    enum sem_monitor_sig signal_id,
    boolean enable
);

/* Enable/Disable monitoring of specified signal. */
void Sem_Monitor_Enable_Sig
(
    Sem_Id semid,
    enum sem_monitor_sig signal_id,
    boolean enable
);

/*Enable/Disable signal compare interrupts to output 0~7.*/
void Sem_Monitor_Enable_Intr
(
    Sem_Id semid,
    Sem_Int_Output output,
    boolean enable
);

void Sem_Set_Error_Div(Sem_Id semid);

#ifdef __cplusplus
}
#endif

#endif
