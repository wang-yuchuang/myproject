/**
 * @file fastboot_common.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: fastboot common interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef  USBD_FASTBOOT_COMMON_H
#define  USBD_FASTBOOT_COMMON_H

#include  <Class/FastBoot/usbd_fb.h>

typedef unsigned char fastboot_t;

typedef struct fastboot_dev {
    fastboot_t      cls_nbr;
    unsigned char   dev_nbr;
} fastboot_dev_t;


void fastboot_register_cmd(const char *prefix, USBD_FB_CMD_HDL handle);
void fastboot_register_var(const char *name, const char *value);

fastboot_t *fastboot_common_init(void *dl_addr, uint32_t dl_sz);
void fastboot_common_okay(fastboot_t *fb, const char *reason);
void fastboot_common_fail(fastboot_t *fb, const char *reason);
void fastboot_common_info(fastboot_t *fb, const char *reason);
void fastboot_common_stop(fastboot_t *fb);



#endif

