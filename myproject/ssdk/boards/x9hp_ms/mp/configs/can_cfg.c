//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include <can_cfg.h>
#include <vcan_client.h>
#include <CLI_type.h>

extern int vcan_init(struct dcf_front_device *fdev, int id);

struct dcf_front_device can_dev[] = {
	{
		.name = "vircan",
		.srv_id = 107,
		.ept_id = 107,
		.can_bind = 2,
		.ept_lock = NULL,
		.sock = 0,
	}
};

static int res_count = ARRAY_SIZE(can_dev);

void board_vcan_init(void)
{
	for(int count = 0; count < res_count; count++) {
		vcan_init(&can_dev[count], count);
	}
}

static int vcaninit(int argc, char *argv[])
{
	int ret = CLI_FALSE;

	for(int count = 0; count < res_count; count++) {
		ret = vcan_init(&can_dev[count], count);
	}

	return ret;
}

CLI_CMD("vcaninit", "can device init", vcaninit);