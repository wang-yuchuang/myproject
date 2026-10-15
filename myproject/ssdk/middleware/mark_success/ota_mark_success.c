/*
 * ota_mark_success.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#if defined(CONFIG_MARK_SUCCESS) && defined(CONFIG_PROPERTY)
#include <string.h>
#include <debug.h>
#include <types.h>
#include <param.h>
#include "board.h"
#include <sd_property.h>
#include "ota_mark_success.h"

#if CONFIG_MP
const char* success_property_name = "cdm.rw.ota_status_mp";
#elif CONFIG_SAFETY
const char* success_property_name = "cdm.rw.ota_status_safety";
#else
const char* success_property_name = NULL;
#endif

static const char *success_val = "1";

int mark_system_success()
{
    if(!success_property_name ||
        (0 != sdrv_property_set(-1, (void*)success_property_name, success_val))) {
        ssdk_printf(SSDK_CRIT, "mark success flag failed\n");
        return -1;
    }
    ssdk_printf(SSDK_CRIT, "mark success ok : %s\n", success_property_name);
    return 0;
}
#endif /* CONFIG_MARK_SUCCESS */
/* EOF */