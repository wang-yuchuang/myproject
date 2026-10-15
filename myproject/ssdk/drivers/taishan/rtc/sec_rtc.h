#ifndef _TAISHAN_RTC_H
#define _TAISHAN_RTC_H

#include <regs_base.h>
#include <stdint.h>

#include "sec_rtc_reg.h"

#ifdef __cplusplus
#define __I volatile /**< Defines 'read-only'  permissions */
#else
#define __I volatile const /**< Defines 'read-only'  permissions */
#endif
#define __O volatile  /**< Defines 'write-only' permissions */
#define __IO volatile /**< Defines 'read/write' permissions */

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
typedef struct {
    __IO uint32_t SEC_RTC_CTRL;               /*offset 0*/
    __IO uint32_t RTC_H;                      /*offset 4*/
    __IO uint32_t RTC_L;                      /*offset 8*/
    __IO uint32_t AUTO_ADJUST;                /*offset c*/
    __IO uint32_t TIMER_H;                    /*offset 10*/
    __IO uint32_t TIMER_L;                    /*offset 14*/
    __IO uint32_t WAKEUP_CTRL;                /*offset 18*/
    __IO uint32_t PERIODICAL_CTRL;            /*offset 1c*/
    __IO uint32_t VIOLATION_INT;              /*offset 20*/
    __IO uint32_t APB_MONITOR_ERR_INJ_EN;     /*offset 24*/
    __IO uint32_t APB_MONITOR_ERR_INJ_ECC;    /*offset 28*/
    __IO uint32_t APB_MONITOR_ERR_INJ_WDATA;  /*offset 2C*/
    __IO uint32_t SAFETY_ERR_IRQ_STAT;        /*offset 0x30*/
    __IO uint32_t SAFETY_ERR_IRQ_STAT_EN;     /*offset 0x34*/
    __IO uint32_t SAFETY_ERR_IRQ_SIG_EN;      /*offset 0x38*/
    __IO uint32_t IRQ_ERR_INJ;                /*offset 0x3C*/
    __IO uint32_t IRQ_ERR_EJ_EN;              /*offset 0x40*/
    __IO uint32_t GP0;                        /*offset 0x44*/
    __IO uint32_t GP1;                        /*offset 0X48*/
    __IO uint32_t GP2;                        /*offset 0x4c*/
    __IO uint32_t GP3;                        /*offset 0x50*/
    __IO uint32_t RTC_H_HOLD_SHADOW;          /*offset 0x54*/
    __IO uint32_t RTC_L_HOLD_SHADOW;          /*offset 0x58*/
    __IO uint32_t RTC_WORKING_MODE;           /*offset 0x5c*/
    __IO uint32_t RTC_REGISTER_CROSS_CLOCK;   /*offset 60*/
    __IO uint32_t RTC_PARITY_ERR_INT_STAT;    /*offset 0x64*/
    __IO uint32_t RTC_PARITY_ERR_INT_STAT_EN; /*offset 0x68*/
    __IO uint32_t RTC_PARITY_ERR_INT_SIG_EN;  /*offset 0x6c*/
    __IO uint32_t PRDATAINJ;                  /*offset 0x70*/
    __IO uint32_t RTC_OUT_ERR_INJ;            /*offset 0x74*/
} Rtc;

#endif

#define RTC1 ((Rtc *)APB_RTC1_BASE)
#define RTC2 ((Rtc *)APB_RTC2_BASE)

#define SEC_RTC_OUT_WKREQ 0
#define SEC_RTC_OUT_OVDIS 1

typedef union {
    struct {
        uint32_t wdata : 1;
        uint32_t ecc : 1;
        uint32_t parity : 1;
    };
    uint32_t v;
} rtc_ej_t;

/**
 * \brief Get 48-bit tick
 * \param rtc Pointer to Rtc instance
 * \return ticks
 */
uint64_t sec_rtc_get_tick(Rtc *rtc);
void sec_rtc_set_tick(Rtc *rtc, uint64_t v);

/**
 * \brief Enable instance
 * \param rtc Pointer to Rtc instance
 * \param en 0-enable rtc,1-disable rtc
 * \return fail?
 */
int sec_rtc_local_enable(Rtc *rtc, uint32_t en);
void sec_rtc_set_alarm_tick(Rtc *rtc, uint64_t val);
uint64_t sec_rtc_get_alarm_tick(Rtc *rtc);
uint8_t sec_rtc_is_alarm_enable(Rtc *rtc);
/**
 * \brief Set RTC controller
 *
 * \param rtc1 Master instance
 * \param rtc2 Slave instance
 * \param ctrl1 master configuration
 * \param ctrl2 slave configuration
 */
void sec_rtc_set_rtc_ctrl(Rtc *rtc1, Rtc *rtc2, uint32_t ctrl1, uint32_t ctrl2);

/**
 * \brief Set ticks of 2 rtc
 *
 * \param rtc1 Master instance
 * \param rtc2 Slave instance
 * \param tick1 master ticks
 * \param tick2 slave ticks(equal to master)
 */
void sec_rtc_set_ms_tick(Rtc *rtc1, Rtc *rtc2, uint64_t tick1, uint64_t tick2);
void sec_rtc_set_ms_timer(Rtc *rtc1, Rtc *rtc2, uint64_t tick1, uint64_t tick2);

/**
 * \brief Set periodical
 *
 * \param rtc Pointer to rtc instance
 * \param freq  32k/(2^freq+1)
 * \param en 0- enable period int,1-disable
 */
void sec_rtc_set_periodical(Rtc *rtc, uint8_t freq, uint8_t en);
uint8_t sec_rtc_get_periodical_freq(Rtc *rtc);
/**
 * \brief Get the violation status
 * \param rtc Pointer to Rtc instance
 * \param vio 0--get overflow status,1--get disable status,x--get register
 * \return status of register
 */
uint32_t sec_rtc_get_violation_status(Rtc *rtc, uint8_t vio);
/**
 * \brief Set RTC Working mode
 *
 * \param master Master instance
 * \param slave Slave instance
 * \param m_ctup maste counting mode: 0--counting up,1--couning down
 * \param s_ctup slave counting mode: 0--counting up,1--couning down
 */
void sec_rtc_ms_cfg(Rtc *master, Rtc *slave, uint8_t m_ctup, uint8_t s_ctup);

/**
 * \brief Update to 32k clock domain
 *
 * \param master Rtc instance
 * \param slave  Rtc instance
 */
void sec_rtc_set_cross_clk_en(Rtc *rtc1, Rtc *rtc2);

/**
 * \brief Get the tick of shadow register
 * \param rtc Pointer to Rtc instance
 * \param tick Pointer to the address to store tick
 * \return value of the shadow register
 */
uint64_t sec_rtc_get_hld_tick(Rtc *rtc, uint64_t *tick);
void sec_trigger_wakeup_req(Rtc *rtc1, Rtc *rtc2, uint32_t wait_cycle);
void sec_rtc_auto_adjust(Rtc *rtc, uint32_t ctrl);
/**
 * \brief Get the status of wakeup
 * \param rtc Pointer to Rtc instance
 * \return wakeup status
 */
uint8_t sec_rtc_get_wakeup_status(Rtc *rtc);

/**
 * \brief Get the status of wakeup
 * \param rtc Pointer to Rtc instance
 * \param type WKUP_CLEAR,WKUP_OV_CLR,WKUP_DISABLE_CLR,Others
 */
void sec_rtc_clear_wakeup(Rtc *rtc, uint8_t type);
void sec_rtc_clear_alarm_status(Rtc *rtc);
/**
 * \brief Enable wakeup or not
 * \param rtc Pointer to Rtc instance
 * \param type WKUP_ENABLE,WKUP_IRQ,WKUP_REQ,Others
 * \param en 0-enable,1-disable
 */
void sec_rtc_wakeup_enable(Rtc *rtc, uint8_t type, uint8_t en);
void sec_rtc_set_violation(Rtc *rtc1, uint32_t rtc1v);
void _rtc_clk_sync(Rtc *rtc, uint32_t clk);
uint32_t sec_rtc_get_parity_status(Rtc *rtc);
void sec_rtc_clear_parity_status(Rtc *rtc);
void sec_rtc_apb_monitor_ej_clear(Rtc *rtc);
uint32_t sec_rtc_get_errirq_status(Rtc *rtc, uint8_t err_type);
void sec_rtc_clear_errirq_status(Rtc *rtc);
void sec_rtc_fusa_enable(Rtc *rtc1, Rtc *rtc2, uint32_t enable);
void sec_rtc_ej(Rtc *rtc, rtc_ej_t ej, uint32_t val);
void sec_rtc_err_sig_enable(Rtc *rtc, uint8_t sig, uint32_t en);

/**
 * \brief Enable out_ej & do injection
 * \param rtc Pointer to Rtc instance
 * \param type Selection of out: 0 >wakeup_req,1:ov_dis
 * \param en 1:enable ej,0:disable
 */
void sec_rtc_out_ej(Rtc *rtc, uint8_t type, uint8_t en);

/**
 * \brief Get the type of out_ej.
 * \return Type of the out_ej, 0 > wake_req, 1  > ov_dis, 2 > no out_ej.
 */
uint8_t sec_rtc_get_out_ej(Rtc *rtc);

/**
 * \brief Cancel ej op,while still enable
 * \param type  wakeup_req/ov_dis/all.
 */
void sec_rtc_out_ej_cancel(Rtc *rtc, uint8_t type);
#endif