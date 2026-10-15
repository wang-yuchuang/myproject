/*
 * Copyright (C) 2020-2030 Semidrive, Inc. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "exvdev.h"

#ifdef CONFIG_OV5640
#include "ex_ov5640.h"
struct external_camera_api ov5640_interface = {
    .name = "ov5640",
    .init = ex_ov5640_init,
};
#endif

#ifdef CONFIG_MAX96706
#include "ex_max96706.h"
struct external_camera_api max96706_interface = {
    .name = "max96706",
    .init = max96706_init,
};
#endif

#ifdef CONFIG_TI926
#include "ex_ti926.h"
struct external_camera_api ti926_interface = {
    .name = "ti926",
    .init = ti926_init,
};
#endif

#ifdef CONFIG_VIRTUAL_CAMERA
#include "ex_vCamera.h"
struct external_camera_api vcamera_interface = {
    .name = "vcamera",
    .init = ex_vCamera_init,
};
#endif

struct external_camera_api dummy_interface = {
    .name = "dummy",
    .init = NULL,
};

struct external_camera_api *excam_api_list[] = {
#ifdef CONFIG_OV5640
    &ov5640_interface,
#endif

#ifdef CONFIG_MAX96706
    &max96706_interface,
#endif

#ifdef CONFIG_TI926
    &ti926_interface,
#endif

#ifdef CONFIG_VIRTUAL_CAMERA
    &vcamera_interface,
#endif

    &dummy_interface,
};

const void *get_excam_api(const char *name)
{
	int i, num;

	num = sizeof(excam_api_list) / sizeof(excam_api_list[0]);
	for (i = 0; i < num; i++) {
		if ((excam_api_list[i]->init != NULL) && !strcmp(excam_api_list[i]->name, name)) {
			return excam_api_list[i];
		}
	}

	return NULL;
}
