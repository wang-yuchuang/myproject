/**
*   @file    Mcu_Delay.h
*
*   @brief   Mcu performance monitor counter driver for udelay.
*   @details .
*/

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/

#ifndef MCU_DELAY_H
#define MCU_DELAY_H

//#include <Std_Types.h>
#include "Std_Types.h"

uint32 read_pmccntr(void);
void Mcu_PmuCounterInit(void);
boolean Mcu_Cnt_IsEnabled(void);
void Mcu_udelay(uint32 us);
uint32 Mcu_UsToTicks(uint32 us);
uint32 Mcu_TicksToUs(uint32 ticks);

#endif  /*MCU_DELAY_H*/
