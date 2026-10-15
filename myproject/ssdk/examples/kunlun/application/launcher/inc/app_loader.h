/**
 * @file app_loader.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv launcher application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_APP_LOADER_H__
#define __SD_APP_LOADER_H__
#include "launcher.h"

extern void app_loader_init(launcher_ctx_t* launcher);
extern void app_loader_deinit(launcher_ctx_t* launcher);

#endif /* __SD_APP_LOADER_H__ */
/* EOF */
