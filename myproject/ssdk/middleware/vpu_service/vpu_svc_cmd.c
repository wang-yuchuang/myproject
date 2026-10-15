/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <cmsis_os2.h>

#include "vpu_svc_cmd.h"
#include "vpuapi.h"
#include "vpu_instance.h"

#define VPU_MUTEX_IDLE_STATE -1
#define VPU_MUTEX_REQUEST_TIMEOUT 500

//#define VPU_IPC_DEBUG

typedef struct vpu_msg_process {
    send_pack_to_ap send_pack_func;
    osMutexId_t debug_lock;
} vpu_msg_process_t;

vpu_msg_process_t *g_vpu_process;


#define send_pack(pack)                                                                            \
    g_vpu_process->send_pack_func(pack->ipcc_id, pack->dev_id, (char *)pack,                       \
                                  sizeof(vpu_ipcc_data_t))


void vpu_sevice_deinit_msg_process(void)
{
    if (g_vpu_process) {

#ifdef VPU_IPC_DEBUG
        osMutexDelete(g_vpu_process->debug_lock);
#endif

        free(g_vpu_process);
        g_vpu_process = NULL;
    }
}

int vpu_service_init_msg_process(send_pack_to_ap send_pack_func)
{
    int ret = 0;
    if (!send_pack_func)
        return -1;

    g_vpu_process = malloc(sizeof(vpu_msg_process_t));
    memset(g_vpu_process, 0x0, sizeof(vpu_msg_process_t));

    g_vpu_process->send_pack_func = send_pack_func;

#ifdef VPU_IPC_DEBUG
    g_vpu_process->debug_lock = osMutexNew(NULL);
#endif

    ret = vpu_instance_init_coda988();
    if (ret < 0)
        return ret;

    ret = vpu_instance_init_wave521();
    if (ret < 0)
        return ret;

    return vpu_instance_init_wave412();
}

void vpu_service_send_heartbeat_pack(int ipcc_id, int dev_id)
{
    vpu_ipcc_data_t pack;
    memset(&pack, 0x0, sizeof(vpu_ipcc_data_t));
    pack.cmd = VPU_IPC_HEARTBEAT;
    pack.ipcc_id = ipcc_id;
    pack.dev_id = dev_id;
    g_vpu_process->send_pack_func(ipcc_id, dev_id, (char *)&pack, sizeof(vpu_ipcc_data_t));
}

// sync func
void vpu_service_process_the_request(vpu_ipcc_data_t *pack, int this_dev_id)
{
    int ret = 0;

    if (!g_vpu_process || !g_vpu_process->send_pack_func || !pack || pack->dev_id != this_dev_id)
        return;

    if (pack->cmd_param_full_length != pack->current_packed_end_offset) {
        // buffer splicing is not supported yet, and an error is returned
        pack->return_value = VPU_IPC_RETURN_ERROR;
        send_pack(pack);
        return;
    }

    switch (pack->cmd) {
    case VPU_IPC_REQUEST_LOCK:
        pack->return_value = VPU_IPC_RETUERN_OK;
        send_pack(pack);
        break;
    case VPU_IPC_RELEASE_LOCK:
        pack->return_value = VPU_IPC_RETUERN_OK;
        send_pack(pack);
        break;
    case VPU_IPC_HEARTBEAT:
        // heartbeat packet
        pack->return_value = VPU_IPC_RETUERN_OK;
        send_pack(pack);
        break;
    default:
        // leave vpu hal to deal with this msg
        ret = vpu_instance_process_msg(pack);
        if (ret < 0)
            pack->return_value = VPU_IPC_RETURN_ERROR;
        else
            pack->return_value = VPU_IPC_RETUERN_OK;
        send_pack(pack);
        break;
    }

    return;
}