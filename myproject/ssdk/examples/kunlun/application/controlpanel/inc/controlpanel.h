/**
 * @file controlpanel.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv controlpanel application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CONTROLPANEL_H__
#define __SD_CONTROLPANEL_H__
#include "launcher.h"

extern void cp_init(void* token);
extern void cp_entry(void* token);
extern void cp_notifier(int notify);
extern void cp_set_primary_cam(int cam);
extern int cp_get_primary_cam();
extern int cp_set_evt(msg_t* msg);

#endif /* __SD_CONTROLPANEL_H__ */
/* EOF */
