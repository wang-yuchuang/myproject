#ifndef _TAMPER_MONITOR_H
#define _TAMPER_MONITOR_H

#include <regs_base.h>
#include <stdint.h>

#include "tamper_monitor_reg.h"

#ifdef __cplusplus
#define __I volatile /**< Defines 'read-only'  permissions */
#else
#define __I volatile const /**< Defines 'read-only'  permissions */
#endif
#define __O volatile  /**< Defines 'write-only' permissions */
#define __IO volatile /**< Defines 'read/write' permissions */

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
typedef struct {
    __IO uint32_t LFSR_CTR;                   /*offset 0*/
    __IO uint32_t LFSR_SEED;                  /*offset 0x4*/
    __IO uint32_t Reserved0[2];               /*offset 0x8*/
    __IO uint32_t PIN_TAMPER[6];              /*offset 0x10*/
    __IO uint32_t Reserved1[6];               /*offset 0x28*/
    __IO uint32_t ON_DIE_TAMPER_0;            /*offset 0x40*/
    __IO uint32_t Reserved2[3];               /*offset 0x44*/
    __IO uint32_t CLOCK_TAMPER;               /*offset 0x50*/
    __IO uint32_t CLOCK_TAMPER_THRESHOLD;     /*offset 0x54*/
    __IO uint32_t Reserved3[2];               /*offset 0x58*/
    __IO uint32_t APB_MONITOR_ERR_INJ_EN;     /*offset 60*/
    __IO uint32_t APB_MONITOR_ERR_INJ_ECC;    /*offset 0x64*/
    __IO uint32_t APB_MONITOR_ERR_INJ_WDATA;  /*offset 0x68*/
    __IO uint32_t Reserved4;                  /*offset 0x6c*/
    __IO uint32_t SAFETY_ERR_IRQ_STAT;        /*offset 0x70*/
    __IO uint32_t SAFETY_ERR_IRQ_STAT_EN;     /*offset 0x74*/
    __IO uint32_t SAFETY_ERR_IRQ_SIG_EN;      /*offset 0x78*/
    __IO uint32_t IRQ_ERR_INJ;                /*offset 0x7c*/
    __IO uint32_t TAMPER_MONITOR_EJ_EN;       /*offset 0x80*/
    __IO uint32_t REG_PARITY_ERR_INT_STAT;    /*offset 0x84*/
    __IO uint32_t REG_PARITY_ERR_INT_STAT_EN; /*offset 0x88*/
    __IO uint32_t REG_PARITY_ERR_INT_SIG_EN;  /*offset 0x8c*/
    __IO uint32_t PRDATAINJ;                  /*offset 0x90*/
    __IO uint32_t TM_OUT_ERR_INJ;             /*offset 0X94*/

} TamperMonitor;

#endif
#define TMONITOR ((TamperMonitor *)APB_TM_BASE)

#define TAMPER_VIO_EJ_ONDIE 0
#define TAMPER_VIO_EJ_PIN 1
#define TAMPER_VIO_EJ_CLOCK 2
#define TAMPER_VIO_EJ_NULL 3

/**
 * \brief Enable out error_inject of TM and inject
 * \param type  Selection for ej: ondie/pin/clock.
 * \param en  1:enable & ej ,0:disable.
 */
void tm_out_ej(uint8_t type, uint8_t en);

/**
 * \brief Enable TM EJ
 * \param en  1:enable & ej ,0:disable.
 */
void tm_ej_en(uint8_t en);

/**
 * \brief Get the type of out_ej.
 * \return Type of the out_ej, 0  > ondie, 1  > pin, 2 > clock.
 */
uint8_t tm_chk_out_ej(void);

/**
 * \brief Cancel ej op,while still enable
 * \param type  ondie/pin/clock.
 */
void tm_out_ej_cancel(uint8_t type);

#endif