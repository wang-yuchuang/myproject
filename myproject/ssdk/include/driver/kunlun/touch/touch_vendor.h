/**
 * @file touch_core.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Semidrive touch framework file
 *
 * Revision History:
 * -----------------
 */
/**
 * @file touch_vendor.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Semidrive touch framework file
 *
 * Revision History:
 * -----------------
 */

#ifndef _TOUCH_VENDOR_H_
#define _TOUCH_VENDOR_H_

#include "touch.h"
/*
 **************************************************************
 *************************ATTENTION****************************
 **************************************************************
 * must add new vendor touch ic probe function declaration here
 * for system call your probe
 */

extern touch_dev_t* goodix_touch_probe(sdrv_touch_handle* touchDrv, const touch_config_t* cfgPtr);
extern touch_dev_t* cyttsp6_touch_probe(sdrv_touch_handle* touchDrv, const touch_config_t* cfgPtr);

#endif
