/*
 * errcode_cfg.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef __ERRCODE_CFG_H__
#define __ERRCODE_CFG_H__
#ifdef CONFIG_ERRCODE
#include <sd_errcode.h>

#define REMOTE_SERVER_IP "172.20.1.10"
#define REMOTE_SERVER_PORT 18180

extern storage_id_t g_strid;

#endif /* CONFIG_ERRCODE*/
#endif /* __ERRCODE_CFG_H__ */
/* EOF */
