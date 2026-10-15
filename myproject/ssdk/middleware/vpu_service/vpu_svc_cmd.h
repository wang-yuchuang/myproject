/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __VPU_SVC_CMD_H__
#define __VPU_SVC_CMD_H__

#define VPU_MAX_IPCC_DEVICE 3
#define VPU_IPCC_DATA_LEN_MAX 468
#define VPU_MAX_MESSAGE_QUEUE_COUNT 100

#define VPU_IPC_RETURN_INVAILD 0x00
#define VPU_IPC_RETUERN_OK 0x01
#define VPU_IPC_RETURN_ERROR 0xff

typedef enum {
    VPU_DEVICES_CODA988,
    VPU_DEVICES_WAVE412,
    VPU_DEVICES_WAVE521,
    VPU_DEVICES_MAX,
} vpu_devices_t;

typedef enum {
    VPU_IPC_REQUEST_LOCK,
    VPU_IPC_RELEASE_LOCK,
    VPU_IPC_HEARTBEAT,
    VPU_IPC_CLEAR_INSTANCE,

    VPU_IPC_DEC_OPEN = 0x10,
    VPU_IPC_DEC_GIVE_COMMAND,
    VPU_IPC_DEC_GET_BITSTREAM_BUFFER,
    VPU_IPC_DEC_UPDATE_BITSTREAMER_BUFFER,
    VPU_IPC_DEC_PIC_END_MODE_UPDATE_BS_BUFFER,
    VPU_IPC_DEC_SEQINIT,
    VPU_IPC_DEC_REGISTER_DECODE_BUFFER,
    VPU_IPC_DEC_SET_RD_PTR,
    VPU_IPC_DEC_ONE_FRAMER,
    VPU_IPC_DEC_FLUSH,
    VPU_IPC_DEC_CLOSE,

    VPU_IPC_SWRESET = 0x20,

    VPU_IPC_ENC_OPEN = 0x30,
    VPU_IPC_ENC_GIVE_COMMAND,
    VPU_IPC_ENC_GET_INITIAL_INFO,
    VPU_IPC_ENC_REGISTER_ENCODE_BUFFER,
    VPU_IPC_ENC_GET_HEAD,
    VPU_IPC_ENC_ONE_FRAMER,
    VPU_IPC_ENC_CLOSE,
} vpu_ipc_cmd_t;

typedef int (*send_pack_to_ap)(int ipcc_devces_index, int vpu_devices_index, char *buf, int len);

typedef struct vpu_ipcc_data {
    uint8_t cmd;
    uint8_t ipcc_id;
    uint8_t dev_id;
    uint8_t return_value;
    uint8_t cmd_param[VPU_IPCC_DATA_LEN_MAX];
    int32_t cmd_param_full_length;
    int32_t current_packed_start_offset;
    int32_t current_packed_end_offset;
    int32_t reserve0;
    int32_t reserve1;
} __attribute__((packed)) vpu_ipcc_data_t;

int vpu_service_init_msg_process(send_pack_to_ap send_pack_func);
void vpu_sevice_deinit_msg_process(void);

void vpu_service_process_the_request(vpu_ipcc_data_t *pack, int dev_id);

#endif // __VPU_SVC_CMD_H__