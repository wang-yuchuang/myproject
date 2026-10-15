/*
 * Copyright (c) 2020 Semidrive Semiconductor Inc.
 * All rights reserved.
 *
 * Description: display service implementation
 *
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <sys/types.h>
#include "app_log.h"

#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <string.h>
#include <lwip/opt.h>
#include <sd_property.h>
#include "input_service.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "avmcast"

#define SIN_PORT  2035

#define MSG_CAM_CAST_START "cast_start"
#define MSG_CAM_CAST_STOP  "cast_stop"

osMutexId_t dev_lock;
osThreadId_t *socket_thread;
int sock_fd;
int connect;

static int socket_send_to_client(char *buf,int buf_len)
{
	struct sockaddr_in addr;
    int ret = 0;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(SIN_PORT);
    addr.sin_addr.s_addr = inet_addr("172.20.2.36");
    ret = sendto(sock_fd, buf, buf_len, 0, (const struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        APP_CRIT("send to clietn faild ret = %d \n", ret);
        return -1;
    } else {
        APP_CRIT("send to client succesed , ret = %d \n", ret);
    }
    return 0;
}

static void avm_cast_status_change(int pid, int old_val, int new_val, void* param){
     APP_CRIT("pid:%d, new_val:%x, old_val:%x\n", pid, new_val, old_val);
    if(pid == DMP_ID_AVM_CAST_STATUS ){
        if (new_val == 1) {
            set_avm_cast_enable(true);
            char *send_data = MSG_CAM_CAST_START;
            socket_send_to_client(send_data,strlen(send_data));
        }else{
            set_avm_cast_enable(false);
            char *send_data = MSG_CAM_CAST_STOP;
            socket_send_to_client(send_data,strlen(send_data));
        }
    }
}

#ifdef ENABLE_SOCKET_RECV
static void qcast_socket_thread(void *arg)
{
    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof(client_addr);

    char recv_data[256];
    int recv_data_len;

	while(1)
	{
		recv_data_len = lwip_recvfrom(sock_fd, recv_data, sizeof(recv_data), 0, &client_addr, &client_addrlen);
        if (recv_data_len == -1 && errno == EAGAIN){
            // APP_CRIT("recv_data_len -1 errno eagin\n");
             continue;
        }

        if (recv_data_len < 0) {
            APP_CRIT("avm cast socket fail to recv from client\n");
            break;
        }

         APP_CRIT("recv %d len data, recv_data:%s\n",recv_data_len,recv_data);
    }

    if (sock_fd >= 0)
        lwip_close(sock_fd);
}
#endif

void avm_cast_service_init(void)
{
	struct sockaddr_in server_addr;

	sock_fd = lwip_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock_fd < 0)
	{
	    APP_CRIT("Socket error\n");
	    goto __exit;
	}
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(SIN_PORT);
	memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
	if (lwip_bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		APP_CRIT("Unable to bind\n");
		goto __exit;
	}

    #ifdef ENABLE_SOCKET_RECV
    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        APP_CRIT("fail to set timeout: %d\n", errno);
        goto __exit;
    }

    osThreadAttr_t attr = {
        .name = "avmcast",
        .stack_size = 4096,
    };

    socket_thread= osThreadNew(qcast_socket_thread, NULL, &attr);
    if (!socket_thread) {
        APP_CRIT("new thread");
        goto __exit;
    }
    #endif

    APP_CRIT("MP avmcast service start#################\n");

    system_property_observe(DMP_ID_AVM_CAST_STATUS, avm_cast_status_change, NULL);
	return;

__exit:
    APP_CRIT("MP avmcast service start error!!!\n");
	if (sock_fd >= 0) lwip_close(sock_fd);
}

