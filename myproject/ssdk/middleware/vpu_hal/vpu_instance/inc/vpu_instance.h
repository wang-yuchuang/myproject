/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __VPU_INSTANCE_H__
#define __VPU_INSTANCE_H__

#include <vpu_svc_cmd.h>

int vpu_instance_init_coda988(void);
void vpu_instance_deinit_coda988(void);

int vpu_instance_init_wave412(void);
void vpu_instance_deinit_wave412(void);

int vpu_instance_init_wave521(void);
void vpu_instance_deinit_wave521(void);

int vpu_instance_process_msg(vpu_ipcc_data_t *pack);

#endif // __VPU_INSTANCE_H__