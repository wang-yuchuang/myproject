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
#include <err.h>
#include <sys/types.h>
#include <lib/console.h>
#include <lib/bytes.h>
#include <lib/reg.h>
#include <app.h>
#include <thread.h>
#include <event.h>
#include <rp_ioctl.h>

#include "sdm_display.h"
#include <res.h>
#include "disp_data_type.h"
#include "display_service.h"
#include "display_share.h"
#include "sys_priority.h"

#define iovec iovec
typedef unsigned int nfds_t;
#define LWIP_TIMEVAL_PRIVATE 0
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <string.h>
#include <lwip/opt.h>

#define RECV_DATA         sizeof(struct disp_frame_info)

static struct dc_share g_ulink_dc_share = {0};
extern int fps(void);

static int disp_unilink_ioctl(void)
{
    struct display_server *disp;
    int ret = 0;
	struct disp_frame_info *info;
	unsigned int size;

    disp = &g_ulink_dc_share.display_server;
	info = (struct disp_frame_info *)disp->recv_data;
	size = disp->recv_data_len;

    if (info->cmd == DISP_CMD_CLEAR) {
        dprintf(0, "recive info->cmd is DISP_CMD_CLEAR \n");
    }

    mutex_acquire(&disp->dev_lock);
    switch(info->cmd) {
        case DISP_CMD_START:
        {
            dprintf(0, "disp cmd start \n");
        }
        break;
        case DISP_CMD_SET_FRAMEINFO:
			if (size != sizeof(struct disp_frame_info)) {
                ret = ERR_NOT_VALID;
                dprintf(0, "size != sizeof(struct disp_frame_info) \n");
                return ret;
            }
			//TODO: get SSB framebuffer in SSA safety
			//framebuffer addr :
            info->addr_l = 0x7b570000;
            //TODO: color is error
            info->format = DRM_FORMAT_ARGB8888;
			//set fb addr to info
            ret = disp_ioctl_set_frameinfo(&g_ulink_dc_share, info);
            {
                display_handle *handle = hal_get_display_handle(1);
                struct sdm_post_config *post = &g_ulink_dc_share.post_configs[disp->on_screen];
                post->post_noblock = true;
                sdm_post(handle, post);
                dprintf(1, "fps = %d\n", fps());
            }
            break;
        case DISP_CMD_SHARING_WITH_MASK:
        {
            dprintf(0, "disp cmd sharing with mask \n");
            break;
        }
        case DISP_CMD_CLEAR:
        {
            dprintf(0, "disp cmd clear \n");
            display_handle *handle = hal_get_display_handle(1);


            sdm_clear_display(handle);
            break;
        }
        default:
        {
            dprintf(0, "disp cmd is error : 0x%x \n", info->cmd);
            break;
        }
    }

    mutex_release(&disp->dev_lock);


    return ret;
}

static int socket_send_to_client(struct display_server *disp)
{
	struct sockaddr_in addr;
    int ret = 0;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(8888);
    addr.sin_addr.s_addr = inet_addr("172.20.2.41");
    ret = sendto(disp->sock, disp->recv_data, RECV_DATA, 0, (const struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        dprintf(ALWAYS, "send to clietn faild ret = %d \n", ret);
        return -1;
    } else {
        dprintf(1,"send to client succesed , ret = %d \n", ret);
    }
    return 0;
}

static int socket_thread(void *arg)
{
    int ret = 0;
    struct display_server *disp = arg;
	uint32_t sin_size;
    struct sockaddr client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
	 while(1)
	{
		disp->recv_data_len = lwip_recvfrom(disp->sock, disp->recv_data, RECV_DATA, 0, &client_addr, &client_addrlen);
        if (disp->recv_data_len == -1 && errno == EAGAIN)
            continue;

        if (disp->recv_data_len < 0) {
            dprintf(CRITICAL, "disp ulink socket fail to recv from client: %s\n", strerror(errno));
            ret = disp->recv_data_len;
            break;
        }

        struct disp_frame_info * info = (struct disp_frame_info *)disp->recv_data;
        if (info->cmd == DISP_CMD_CLEAR) {
            dprintf(0, "socket thread recive info cmd is clear \n");
        }
		dprintf(1, "recv %d len data, RECV_DATA = %d \n",disp->recv_data_len, RECV_DATA);
		disp_unilink_ioctl();
	    //socket write back
        // socket_send_to_client(disp);
    }

    if (disp->sock >= 0)
        lwip_close(disp->sock);

    dprintf(ALWAYS, "disp unilink recv thread stop\n");
    display_handle *handle = hal_get_display_handle(1);
    sdm_clear_display(handle);
    return ret;

}

void disp_ulink_service_init(void)
{
    int ret;
    struct display_server *disp;
	disp = &g_ulink_dc_share.display_server;

	if (disp->binitialized)
		return;

	struct sockaddr_in server_addr;

	disp->recv_data = (char *)memalign(4096, RECV_DATA);
	if (disp->recv_data == NULL)
	{
		dprintf(CRITICAL, "No memory\n");
		goto __exit;
	}

	disp->sock = lwip_socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	 if (disp->sock < 0)
	{
	    dprintf(CRITICAL, "Socket error\n");
	    goto __exit;
	}
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = inet_addr("172.20.2.32");
	server_addr.sin_port = htons(8888);
	memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
	if (lwip_bind(disp->sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		dprintf(CRITICAL, "Unable to bind\n");
		goto __exit;
	}

    struct timeval tv;
    tv.tv_sec = 60;
    tv.tv_usec = 0;
    if (setsockopt(disp->sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        dprintf(CRITICAL, "fail to set timeout: %d\n", errno);
        goto __exit;
    }

	disp->socket_thread = thread_create("socket", socket_thread, disp, THREAD_PRI_DISP_MAIN + 1, 4096);
	thread_detach_and_resume(disp->socket_thread);
    dprintf(CRITICAL, "safety disp ulink service start#################\n");

    mutex_init(&disp->dev_lock);

    mutex_acquire(&disp->dev_lock);
    sdm_post_init(&g_ulink_dc_share);
    mutex_release(&disp->dev_lock);
    disp->binitialized = true;
	return;

__exit:
	if (disp->sock >= 0) lwip_close(disp->sock);
	if (disp->recv_data) free(disp->recv_data);

}

void disp_ulink_service_entry(const struct app_descriptor *app, void *args)
{
    disp_ulink_service_init();
}

APP_START(disp_ulink)
 .entry = (app_entry)disp_ulink_service_entry,
APP_END

