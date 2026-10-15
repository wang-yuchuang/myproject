#ifndef __SBC_SYS__
#define __SBC_SYS__

#include <cmsis_os2.h>
#include "SBC_TLE94x1.h"
#include "TLE94x1.h"
/**
 * @brief   Create Trigger WTD Task.
 *
 * @param   Trigger Initial Watchdog Period
 * @retval  None
 */
osThreadId_t SBC_Create_WD_Trigger_Task(uint32_t period);

/**
 * @brief   Suspend Trigger WTD Task for watchdog debug
 *
 * @retval  None
 */
void SBC_Suspend_WD_Trigger_Task(void);

/**
 * @brief   Set Trigger WTD Task Period.
 *
 * @param   Trigger Watchdog Period
 * @retval  None
 */
void SBC_Set_WD_Trigger_Period(uint32_t period);
/**
 * @brief   Get Trigger WTD Task Period.
 *
 * @param   Trigger Watchdog Period
 * @retval  None
 */
uint32_t SBC_Get_WD_Trigger_Period(void);

/**
 * @brief   Reset SBC
 *
 * @retval  SBC_ErrorCode   See description of SBC_ErrorCode.
 */
SBC_ErrorCode SBC_Soft_Reset(void);

/**
 * @brief Set SBC mode to STOP, and stop wdt
 *
 */
void SBC_Mode_Stop_Without_WTD(void);

/**
 * @brief Set SBC mode to NORMAL
 *
 * [not fixed: set to normal, will trigger reset.]
 */
void SBC_Set_Mode_Normal(void);

#endif
