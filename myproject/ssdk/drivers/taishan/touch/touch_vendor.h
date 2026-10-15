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

/*
 **************************************************************
 *************************ATTENTION****************************
 **************************************************************
 * must add new vendor touch ic probe function declaration here
 * for system call your probe
 */

extern int goodix_probe_device(const struct touch_device *ts_dev);

#endif


