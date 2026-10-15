/*
 * Copyright (c) 2020 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: display service API prototype
 *
 */

#ifndef __DISPLAY_SERVICE_H__
#define __DISPLAY_SERVICE_H__

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <debug.h>
#include <reg.h>
#include <cmsis_os2.h>
#include <board.h>
#include <types.h>

#include "driver/kunlun/mbox/sdrv_mbox_drv.h"

#ifndef u16
typedef unsigned short u16;
#endif

typedef uint8_t u8;
typedef uint32_t u32;

#define DISPLAY_COMPLETE_EVENT_FLAG 1u

/* Remote IOC helper */
#define RP_IOC_DATA(ctl)		&ctl->u.data[0]

/* Define common data */
struct display_server {
	osMutexId_t dev_lock;
	u32 features;
	bool binitialized;

	osThreadId_t *disp_thread;
	osEventFlagsId_t disp_event;
	osThreadId_t *disp_vsync_thread;
	osEventFlagsId_t disp_vsync_event;
	int on_screen;
	u16 mbox_addr;
	int rproc;
	mbox_chan_t* mchan;
	/* communication level stuff */
	// hal_mb_client_t client;
	// hal_mb_chan_t *mchan;

	int sock;
	int connect;
	char *recv_data;
	int recv_data_len;
	osThreadId_t *socket_thread;
};

struct disp_frame_info {
	u32 addr_l;
	u32 addr_h;
	u32 format;
	u16 width;
	u16 height;
	u16 pos_x;
	u16 pos_y;
	u16 pitch;
	u16 mask_id;
	u32 cmd;
}__attribute__ ((packed));

/* Do not exceed 32 bytes so far */
struct disp_ioctl_cmd {
	u32 op;
	union {
		u8 data[28];
		struct disp_frame_info fi;
	} u;
};

struct disp_ioctl_ret {
	u32 op;
	union {
		u8 data[8];
	} u;
};

#endif //__DISPLAY_SERVICE_H__
