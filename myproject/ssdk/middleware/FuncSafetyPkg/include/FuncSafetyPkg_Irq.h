/**
 *   @file    FUNCSAFETYPKG_Irq.h
 *
 *   @brief   Function safety package irq file.
 *   @details
 */

/********************************************************
 *        Copyright(c) 2020    Semidrive                *
 *        All rights reserved.                          *
 ********************************************************/
#ifndef FUNCSAFETYPKG_IRQ_H
#define FUNCSAFETYPKG_IRQ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FuncSafetyPkg.h"
#include "Mcal.h"


//ISR(Sem_Isr);
extern int Sem_Isr(uint32 irq, void *arg);



#ifdef __cplusplus
}
#endif

#endif
