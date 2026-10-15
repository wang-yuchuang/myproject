/**
 * @file fastavm.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_FASTAVM_H__
#define __SD_FASTAVM_H__
#include "launcher.h"

extern void fastavm_init(void* token);
extern void fastavm_entry(void* token);
extern void fastavm_notifier(int notify);
extern void fastavm_set_primary_cam(int cam);
extern int fastavm_get_primary_cam();
extern int fastavm_set_evt(msg_t* msg);
extern uint32_t fastavm_get_prim_cam();


#endif /* __SD_FASTAVM_H__ */
/* EOF */
