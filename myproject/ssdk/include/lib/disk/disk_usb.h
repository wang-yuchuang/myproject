/**
 * @file disk_usb.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_USB_H
#define DISK_USB_H

#include <lib/disk/disk.h>


int register_usb_disk(struct disk_info *info);
int unregister_usb_disk(struct disk_info *info);

#endif /* DISK_MMC_H_ */
