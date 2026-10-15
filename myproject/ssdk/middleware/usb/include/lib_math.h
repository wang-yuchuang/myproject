/**
 * @file lib_math.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Mathematic library header file for USB Device & Host.
 *
 * Revision History:
 * -----------------
 */


#ifndef  LIB_MATH_MODULE_PRESENT
#define  LIB_MATH_MODULE_PRESENT


#include  <cpu.h>
#include  <cpu_core.h>

#include  <lib_def.h>


/*
*********************************************************************************************************
*                                            MATH_IS_PWR2()
*
* Description : Determine if a value is a power of 2.
*
* Argument(s) : nbr           Value.
*
* Return(s)   : DEF_YES, 'nbr' is a power of 2.
*
*               DEF_NO,  'nbr' is not a power of 2.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  MATH_IS_PWR2(nbr)                                 ((((nbr) != 0u) && (((nbr) & ((nbr) - 1u)) == 0u)) ? DEF_YES : DEF_NO)


/*
*********************************************************************************************************
*                                        MATH_ROUND_INC_UP_PWR2()
*
* Description : Round value up to the next (power of 2) increment.
*
* Argument(s) : nbr           Value to round.
*
*               inc           Increment to use. MUST be a power of 2.
*
* Return(s)   : Rounded up value.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  MATH_ROUND_INC_UP_PWR2(nbr, inc)                  (((nbr) & ~((inc) - 1)) + (((nbr) & ((inc) - 1)) == 0 ? 0 : (inc)))


/*
*********************************************************************************************************
*                                          MATH_ROUND_INC_UP()
*
* Description : Round value up to the next increment.
*
* Argument(s) : nbr           Value to round.
*
*               inc           Increment to use.
*
* Return(s)   : Rounded up value.
*
* Caller(s)   : Application.
*
* Note(s)     : none.
*********************************************************************************************************
*/

#define  MATH_ROUND_INC_UP(nbr, inc)                       (((nbr) + ((inc) - 1)) / (inc) * (inc))


#endif
