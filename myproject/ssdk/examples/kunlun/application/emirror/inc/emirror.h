/**
 * @file emirror.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_EMIRROR_H__
#define __SD_EMIRROR_H__
#include "launcher.h"

extern void emirror_init(void* token);
extern void emirror_entry(void* token);
extern void emirror_notifier(int notify);
extern void emirror_set_primary_cam(int cam);
extern int emirror_get_primary_cam();
extern int emirror_set_evt(msg_t* msg);
extern uint32_t emirror_get_prim_cam();
#endif /* __SD_EMIRROR_H__ */
/* EOF */
