/**
 * @file cluster.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv animation application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CLUSTER_H__
#define __SD_CLUSTER_H__
#include "launcher.h"

extern void cluster_init(void* token);
extern void cluster_entry(void* token);
extern void cluster_notifier(int notify);
extern void cluster_set_primary_cam(int cam);
extern int cluster_get_primary_cam();
extern int cluster_set_evt(msg_t* msg);

#endif /* __SD_CLUSTER_H__ */
/* EOF */
