/**
 * @file animation.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv boot animation application.
 *
 * Revision History:
 * -----------------
 */

#ifndef __SD_ANIMATION_H__
#define __SD_ANIMATION_H__

extern void ani_init(void* token);
extern void ani_entry(void* token);
extern void ani_notifier(int notify);
extern int ani_set_evt(msg_t* msg);

#endif /* __SD_ANIMATION_H__ */
/* EOF */
