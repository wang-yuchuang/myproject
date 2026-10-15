/*
 * errcode_cfg.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_ERRCODE
#include <debug.h>
#include <types.h>
#include <param.h>
#include "board.h"
#include "errcode_cfg.h"

storage_id_t g_strid = STR_ID_OSPI1;

#endif /* CONFIG_ERRCODE */
/* EOF */
