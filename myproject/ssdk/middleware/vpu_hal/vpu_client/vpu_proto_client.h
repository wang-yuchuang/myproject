/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef _VPU_PROTO_CLIENT_H_
#define _VPU_PROTO_CLIENT_H_

#include "../../vpu_service/vpu_svc_cmd.h"

int vpu_proto_send_msg_to_service(int core_idx, vpu_ipcc_data_t *pack);
void vpu_virtio_local_client_init(void);

#endif /* _VPU_PROTO_CLIENT_H_ */
