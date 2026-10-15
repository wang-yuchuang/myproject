/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */


#include "cam_conf.h"
#include "cam_hal.h"
#include "cam_service.h"
#include "sd_errcode.h"

#ifdef CONFIG_CSI_SVC_VIA_UNILINK
#include <ulink_service.h>

/* to satisfy lwip api */
#define iovec iovec
typedef unsigned int nfds_t;
#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/socket.h>
#define SERVER_PORT 17217

static int sock = -1;
static struct sockaddr_in client_addr = {};
#endif

#define SCS_LOG_CRI  SSDK_CRIT
#define SCS_LOG_NOTE SSDK_ALERT
#define SCS_LOG_INFO SSDK_INFO
#define SCS_LOG_DBG  SSDK_DEBUG

#define CAMERA_TIMEOUT_MS 5000
#define RESTART_NUM 2
#define MCHAN_TIMEOUT_MS 50

static struct cam_device s_cdev[MAX_CAM_INST];
static bool binitialized;

extern dma_dev_t *dma_dev;

int handle_camera_request(struct scs_ioctl_cmd *ctl, struct scs_ioctl_result *r, int id);
int create_camera_notifier(struct cam_device *cam);
rpc_call_result_t cam_svr_ioctl_cb(rpc_server_handle_t hserver, rpc_call_request_t *request);

static void notify_ap2_client(struct cam_device *cam, paddr_t addr,
                                uint64_t timestamp);
static void dma_chan_free(struct cam_device *cam);

struct cam_config cam_confs[CAMERA_NUM] = {
    {
        false,
        CAMERA_0,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI0,
    },
    {
        false,
        CAMERA_1,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI0_1,
    },
    {
        false,
        CAMERA_2,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI0_2,
    },
    {
        false,
        CAMERA_3,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI0_3,
    },
    {
        false,
        CAMERA_4,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI1_0,
    },
    {
        false,
        CAMERA_5,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI1_1,
    },
    {
        false,
        CAMERA_6,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI1_2,
    },
    {
        false,
        CAMERA_7,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI1_3,
    },
    {
        false,
        CAMERA_8,
        {
            MBOX_RPROC_AP1,
            MBOX_RPROC_AP2,
        },
        MBOX_INDEX_CAMERA_CSI2,
    },
};

static uint32_t to_client_fmt(uint32_t local_fmt)
{
    switch (local_fmt) {
    case VDEV_PIX_FMT_YUYV:
        return V4L2_PIX_FMT_YUYV;

    case VDEV_PIX_FMT_UYVY:
        return V4L2_PIX_FMT_UYVY;

    case VDEV_PIX_FMT_NV16:
        return V4L2_PIX_FMT_NV16;

    case VDEV_PIX_FMT_NV61:
        return V4L2_PIX_FMT_NV61;

    case VDEV_PIX_FMT_YUV422P:
        return V4L2_PIX_FMT_YUV422P;

    case VDEV_PIX_FMT_RGB24:
        return V4L2_PIX_FMT_RGB24;

    case VDEV_PIX_FMT_BGR24:
        return V4L2_PIX_FMT_BGR24;

    case VDEV_PIX_FMT_RGB16:
        return V4L2_PIX_FMT_RGB565;

    default:
        return V4L2_PIX_FMT_YUYV;
    }

    return V4L2_PIX_FMT_YUYV;
}

const char *cmd_to_str(u16 cmd)
{
	switch(cmd)
		{
		case SCS_OP_DEV_OPEN:
			return "SCS_OP_DEV_OPEN";
		case SCS_OP_ENUM_FORMAT:
			return "SCS_OP_ENUM_FORMAT";
		case SCS_OP_ENUM_FRAMESIZE:
			return "SCS_OP_ENUM_FRAMESIZE";
		case SCS_OP_GET_FORMAT:
			return "SCS_OP_GET_FORMAT";
		case SCS_OP_SET_FORMAT:
			return "SCS_OP_SET_FORMAT";
		case SCS_OP_QUEUE_SETUP:
			return "SCS_OP_QUEUE_SETUP";
		case SCS_OP_GET_BUFINFO:
			return "SCS_OP_GET_BUFINFO";
		case SCS_OP_SET_BUFINFO:
			return "SCS_OP_SET_BUFINFO";
		case SCS_OP_QBUF:
			return "SCS_OP_QBUF";
		case SCS_OP_STREAM_ON:
			return "SCS_OP_STREAM_ON";
		case SCS_OP_DQBUF:
			return "SCS_OP_DQBUF";
		case SCS_OP_STREAM_OFF:
			return "SCS_OP_STREAM_OFF";
		case SCS_OP_DEV_CLOSE:
			return "SCS_OP_DEV_CLOSE";
		case SCS_OP_ENUM_FRAMEINTERVALS:
			return "SCS_OP_ENUM_FRAMEINTERVALS";
		default:
			dprintf(SCS_LOG_CRI, "%s:%d, cmd %d ----------\n",
			__func__,__LINE__,cmd);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_OP,cmd);
			return "NO support!";
	}
}

static inline struct buf_item *index_buf_item_peek(int index, struct list_node *h)
{
	struct buf_item *p_b = NULL;

	if (!list_is_empty(h)) {
		list_for_every_entry(h, p_b, struct buf_item, list) {
			if (p_b->index == index) {
				return p_b;
			}
		}
	}

	return NULL;
}

static inline size_t get_client_buf_cnt(int client, struct list_node *h)
{
	struct buf_item *p_b = NULL;
    size_t cnt = 0;
	uint32_t client_id;

	if (!list_is_empty(h)) {
		list_for_every_entry(h, p_b, struct buf_item, list) {
			client_id = (p_b->index & 0xffff0000) >> 16;
			if (client_id == client) {
				cnt++;
			}
		}
	}

	return cnt;
}

static inline struct buf_item *client_buf_item_pop(int client, struct list_node *h)
{
    struct buf_item *p_b = NULL;
    struct buf_item *tp_b = NULL;
    uint32_t client_id;

    if (!list_is_empty(h)) {
        list_for_every_entry_safe(h, p_b, tp_b, struct buf_item, list) {
            client_id = (p_b->index & 0xffff0000) >> 16;
            if (client_id == client) {
                list_delete(&p_b->list);
                return p_b;
            }
        }
    }

    return NULL;
}

static inline struct buf_item *index_buf_item_pop(int index, struct list_node *h)
{
	struct buf_item *p_b = NULL;
	struct buf_item *tp_b = NULL;

	if (!list_is_empty(h)) {
		list_for_every_entry_safe(h, p_b, tp_b, struct buf_item, list) {
			if (p_b->index == index) {
				list_delete(&p_b->list);
				return p_b;
			}
		}
	}

	return NULL;
}

    static inline void print_buf_item_index(struct list_node *h)
    {
        struct buf_item *p_b = NULL;
        struct buf_item *tp_b = NULL;

        if (!list_is_empty(h)) {
            list_for_every_entry_safe(h, p_b, tp_b, struct buf_item, list) {
                dprintf(SCS_LOG_NOTE, "buf index:%x  ", p_b->index);
            }
        }
        dprintf(SCS_LOG_NOTE, "\n");
    }

static inline struct buf_item *buf_item_pop(struct list_node *h)
{
	struct buf_item *p_b = NULL;
	struct list_node *node = NULL;

	if (!list_is_empty(h)) {
		node = list_remove_tail(h);
		p_b = containerof(node, struct buf_item, list);
	}

	return p_b;

}

static inline struct local_item *thread_id_local_item_peek(int thread_id, struct list_node *h)
{
	struct local_item *p_b = NULL;

	if (!list_is_empty(h)) {
		list_for_every_entry(h, p_b, struct local_item, list) {
			if (p_b->thread_id == thread_id) {
				return p_b;
			}
		}
	}

	return NULL;
}

static inline struct local_item *remote_id_local_item_peek(int remote_id, struct list_node *h)
{
	struct local_item *p_b = NULL;

	if (!list_is_empty(h)) {
		list_for_every_entry(h, p_b, struct local_item, list) {
			if (p_b->remote_id == remote_id) {
				return p_b;
			}
		}
	}

	return NULL;
}

static inline struct local_item *thread_id_local_item_pop(int thread_id, struct list_node *h)
{
	struct local_item *p_b = NULL;
	struct local_item *tp_b = NULL;

	if (!list_is_empty(h)) {
		list_for_every_entry_safe(h, p_b, tp_b, struct local_item, list) {
			if (p_b->thread_id == thread_id) {
				list_delete(&p_b->list);
				return p_b;
			}
		}
	}

	return NULL;
}


static inline struct local_item *local_item_pop(struct list_node *h)
{
	struct local_item *p_b = NULL;
	struct list_node *node = NULL;

	if (!list_is_empty(h)) {
		node = list_remove_tail(h);
		p_b = containerof(node, struct local_item, list);
	}

	return p_b;

}

static uint32_t to_local_fmt(uint32_t client_fmt)
{
    switch (client_fmt) {
    case V4L2_PIX_FMT_YUYV:
        return VDEV_PIX_FMT_YUYV;

    case V4L2_PIX_FMT_UYVY:
        return VDEV_PIX_FMT_UYVY;

    case V4L2_PIX_FMT_NV16:
        return VDEV_PIX_FMT_NV16;

    case V4L2_PIX_FMT_NV61:
        return VDEV_PIX_FMT_NV61;

    case V4L2_PIX_FMT_YUV422P:
        return VDEV_PIX_FMT_YUV422P;

    case V4L2_PIX_FMT_RGB24:
        return VDEV_PIX_FMT_RGB24;

    case V4L2_PIX_FMT_BGR24:
        return VDEV_PIX_FMT_BGR24;

    case V4L2_PIX_FMT_RGB565:
        return VDEV_PIX_FMT_RGB16;

    default:
        return VDEV_PIX_FMT_YUYV;
    }

    return VDEV_PIX_FMT_YUYV;
}

int set_fmt_info(struct cam_device *cam)
{
    int ret = 0;

    switch (cam->pixfmt) {
    case V4L2_PIX_FMT_UYVY:
    case V4L2_PIX_FMT_YUYV:
    case V4L2_PIX_FMT_RGB565:
        cam->planes = 1;
        cam->size[0] = cam->w * cam->h * 2;
        cam->size[1] = 0;
        cam->size[2] = 0;
        break;

    case V4L2_PIX_FMT_RGB24:
    case V4L2_PIX_FMT_BGR24:
        cam->planes = 1;
        cam->size[0] = cam->w * cam->h * 3;
        cam->size[1] = 0;
        cam->size[2] = 0;
        break;

    default:
        dprintf(SCS_LOG_CRI, "%s cam%d unsupported format 0x%08x\n", __func__,
                cam->cam_id, cam->pixfmt);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_IOCTRL_FAIL,cam->pixfmt);
        ret = -1;
        break;
    }

    return ret;
}

#if 1
static void stream_wait_pending(struct cam_device *cam, int id)
{
    struct client *c = &cam->clients[id];
    int loop = 0;
	int pend_buf_cnt = 0;
	struct buf_item *bufi;
	struct buf_item *tmp;
	uint32_t client_id;
    int i = 0;

    /* wait for pending done: all buffer is pop up from driver */
    dprintf(SCS_LOG_NOTE, "%s %d:c%d wait\n", __func__, __LINE__, id);

    while (loop++ < 20) {
		osMutexAcquire(cam->buf_lock, osWaitForever);
		if (!list_is_empty(&c->pend_buf_q)) {
			pend_buf_cnt = list_length(&c->pend_buf_q);
		}

		//Buf is not returned due to dma copy delay and needs to be released forcibly
		//AP buf needs to be returned in full(only ap1/2)
		list_for_every_entry_safe(&cam->user_buf_q, bufi, tmp, struct buf_item, list) {
			client_id = (bufi->index & 0xffff0000) >> 16;
			if ((client_id == RPMSG_CLIENT) || (client_id == RPMSG_CLIENT_AP2)) {
                if (client_id == id) {
                    dprintf(SCS_LOG_NOTE, "c%d, Forced release buf index:0x%x, user_cnt %d\n",
                        id, bufi->index, bufi->user_cnt);
                    for (i = 0; i < MAX_CLIENT; i++) {
                        dprintf(SCS_LOG_NOTE, "client%d, user %d; ",
                            i, bufi->client_cnt[i]);
                        bufi->client_cnt[i] = 0;
                    }
                    dprintf(SCS_LOG_NOTE, "\n");

                    bufi->user_cnt = 0;
                    list_delete(&bufi->list);
                    bufi->busy = false;
                    list_add_head(&cam->clients[client_id].pend_buf_q, &bufi->list);
                }
			}
		}
		osMutexRelease(cam->buf_lock);

		osMutexAcquire(cam->run_lock, osWaitForever);
        if ((pend_buf_cnt == c->buf_cnt) || (cam->stream_cnt == 0)) {
			osMutexRelease(cam->run_lock);
            break;
        }
		osMutexRelease(cam->run_lock);
		wait_completion_timeout(&c->pend_event, 30);
    }

	if (loop >= 20) {
		dprintf(SCS_LOG_NOTE, "c%d wait timeout, pend_buf_cnt:%d, buf_cnt:%d\n",
			id, pend_buf_cnt, c->buf_cnt);
	}

    dprintf(SCS_LOG_NOTE, "%s %d:c%d wait done\n", __func__, __LINE__, id);
}
#endif
#ifdef CONFIG_CSI_SVC_VIA_UNILINK
static void camera_message_handler(void *arg)
{
    ssize_t count;
    struct scs_ioctl_cmd ctl;
    struct scs_ioctl_result r;
    struct sockaddr addr;
    socklen_t addrlen = sizeof(addr);

    dprintf(SCS_LOG_NOTE, "camera ulink msg thread start\n");

    for (;;) {
        count = recvfrom(sock, &ctl, sizeof(ctl), 0, &addr, &addrlen);
        if (count == -1 && errno == EAGAIN)
            continue;

        if (count < 0) {
            dprintf(SCS_LOG_CRI, "fail to recv from client: %d\n", errno);
	    ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_UNILINK_RECV_FAIL,errno);
            goto err;
        }

        /*
         * Settle down transfer api in open request. Assume only one
         * connection (socket or rpmsg) comes at a time for simplicity.
         */
        if (ctl.op == SCS_OP_DEV_OPEN) {
            memcpy(&client_addr, &addr, sizeof(client_addr));
        }

		if (ctl.op == SCS_OP_QUEUE_SETUP) {
			ctl.msg.queue.m_type = SCS_MEMORY_MMAP;
		}
        memset(&r, 0, sizeof(r));
        handle_camera_request(&ctl, &r, SOCKET_CLIENT);

        count = sendto(sock, &r, sizeof(r), 0, &addr, sizeof(addr));
        if (count < 0) {
            dprintf(SCS_LOG_CRI, "fail to send to client: %d\n", errno);
			ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_UNILINK_SEND_FAIL,errno);
            goto err;
        }
    }

err:
    close(sock);
    sock = -1;

    dprintf(SCS_LOG_NOTE, "camera ulink msg thread stop\n");
    osThreadExit();
}

static void start_message_handler(void)
{
    struct sockaddr_in local_addr;
    struct timeval tv;
    osThreadAttr_t attr = {
        .priority = osPriorityNormal,
    };
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        dprintf(SCS_LOG_CRI, "fail to start socket: %d\n", errno);
        return;
    }

    local_addr.sin_family = PF_INET;
    local_addr.sin_port = htons(SERVER_PORT);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) == -1) {
        dprintf(SCS_LOG_CRI, "fail to bind port %d: %d\n", SERVER_PORT, errno);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_UNILINK_BIND_PORT_FAIL,errno);
        close(sock);
        sock = -1;
        return;
    }

    tv.tv_sec = 60;
    tv.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        dprintf(SCS_LOG_CRI, "fail to set timeout: %d\n", errno);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_UNILINK_SET_TIMEOUT_FAIL,errno);
        close(sock);
        sock = -1;
        return;
    }

    dprintf(SCS_LOG_NOTE, "server started at port %d\n", SERVER_PORT);

    osThreadNew(camera_message_handler, NULL, &attr);
}

static void notify_ulink_client(struct cam_device *cam, paddr_t addr,
                                uint64_t timestamp)
{
    struct client *local = &cam->clients[LOCAL_CLIENT];
    struct buf_item *buf = NULL;
    uint32_t index = 0;

    osMutexAcquire(local->ubuf_lock, osWaitForever);
	if (!list_is_empty(&local->upend_buf_q)) {
		buf = buf_item_pop(&local->upend_buf_q);
	}
    osMutexRelease(local->ubuf_lock);

    if (buf) {
        index = buf->index & 0xffff;
        dprintf(SCS_LOG_DBG, "dma copy img %d from 0x%lx to 0x%lx at %d\n", index,
            (unsigned long)addr, (unsigned long)buf->addrs[0], (uint32_t)current_time());
#if ULINK_SIDE_A
        ulink_hpl_dma_copy(COPY_DIR_SSA_TO_SSB,
                           (uint64_t)buf->addrs[0],
                           (uint64_t)addr, cam->size[0]);
#elif ULINK_SIDE_B
        ulink_hpl_dma_copy(COPY_DIR_SSB_TO_SSA,
                           (uint64_t)buf->addrs[0],
                           (uint64_t)addr, cam->size[0]);
#endif

        struct scs_ioctl_result r;
        r.op = SCS_OP_DQBUF;
        r.msg.g_bufinfo.index = index;
        r.msg.g_bufinfo.timestamp = (uint32_t)(timestamp & 0xFFFFFFFF);
        r.msg.g_bufinfo.timestamp_h = (uint32_t)((timestamp >> 32) & 0xFFFFFFFF);

        dprintf(SCS_LOG_DBG, "notify ulink %u\n", r.msg.data[1]);

        ssize_t count;
        count = sendto(sock, &r, sizeof(r), 0, (struct sockaddr *)&client_addr,
                       sizeof(client_addr));
        if (count < 0) {
            dprintf(SCS_LOG_CRI, "fail to send buffer to cient: %d\n", errno);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_UNILINK_SEND_BUF_FAIL,errno);
			osMutexAcquire(local->ubuf_lock, osWaitForever);
			list_add_head(&local->upend_buf_q, &buf->list);
			osMutexRelease(local->ubuf_lock);
        }
    }
}
#endif

int check_client_err(struct cam_device *cam, int client, int restart)
{
    uint8_t data[10];
    uint32_t len = 10;
	int mchan_id = 0;
	int ret = 0;
	int i = 0;

	switch (client) {
		case RPMSG_CLIENT:
			mchan_id = SCS_RPMSG_CLIENT_AP1;
			break;
		case RPMSG_CLIENT_AP2:
			mchan_id = SCS_RPMSG_CLIENT_AP2;
			break;
		default:
			return 0;
	}

	data[0] = (uint8_t)cam->cam_id;
	data[1] = 0xff;//buf index,Verify use
	if (cam->mchan[mchan_id]) {
		for (i = 0; i < restart; i++) {
			ret = osMutexAcquire(cam->clients[client].mchan_lock,
				MCHAN_TIMEOUT_MS);
			if (ret == 0) {
                ret = mbox_send(cam->mchan[mchan_id], data, len, 1000);
				osMutexRelease(cam->clients[client].mchan_lock);
				if (ret == 0)
					break;
				dprintf(SCS_LOG_NOTE, "%s %d: client%d, mbox_send %d\n",
					__func__,__LINE__, client, ret);

			} else {
				dprintf(SCS_LOG_NOTE, "%s %d: client%d, osMutexAcquire Timeout%d\n",
					__func__,__LINE__, client, ret);
			}
		}
	}

	return ret;
}

int safe_cam_notify_agent(struct cam_device *cam,
        uint32_t index, uint64_t timestamp)
{
    struct client *local = &cam->clients[LOCAL_CLIENT];
    struct client *rpmsg = &cam->clients[RPMSG_CLIENT];
    struct client *rpmsg_ap2 = &cam->clients[RPMSG_CLIENT_AP2];
    uint8_t data[10];
    uint32_t len = 10;
    struct event_item *evt = NULL;
    uint32_t client_id;
    int ret = 0;
    paddr_t addr = 0;

	client_id = (index & 0xffff0000) >> 16;

    data[0] = (uint8_t)cam->cam_id;
    data[1] = index & 0xffff;
    memcpy((void *)&data[2], (void *)&timestamp, 8);

    osMutexAcquire(cam->buf_lock, osWaitForever);
    if ((cam->done_buf != NULL) &&
            (cam->done_buf->index == index)) {
        addr = cam->done_buf->addrs[0];
    }
    osMutexRelease(cam->buf_lock);

#ifdef CONFIG_CSI_SVC_VIA_UNILINK
	struct local_item *p_local = NULL;

	p_local = remote_id_local_item_peek(SOCKET_CLIENT, &local->user_local_q);
	if (p_local != NULL) {
		if (p_local->client_sts == SCS_STREAM_ON) {
            if (addr != 0)
			    notify_ulink_client(cam, addr, timestamp);
		}
	}
#endif

    if (rpmsg->client_sts == SCS_STREAM_ON && client_id == RPMSG_CLIENT) {
		//dcf_do_notify(cam->notifier, data, len);//notify linux dqbuf cb
        if (cam->mchan[SCS_RPMSG_CLIENT_AP1]) {
            ret = osMutexAcquire(cam->clients[RPMSG_CLIENT].mchan_lock,
                                        MCHAN_TIMEOUT_MS);
            if (ret == 0) {
                ret =  mbox_send(cam->mchan[SCS_RPMSG_CLIENT_AP1], data, len, 1000);
                osMutexRelease(cam->clients[RPMSG_CLIENT].mchan_lock);
                if (ret < 0) {
                    completion_done(&cam->err_process);
                    dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d, mbox_send %d, buf %x\n",
                        __func__, __LINE__, cam->cam_id, RPMSG_CLIENT, ret, index);
                } else {
                    /* May be updated by qbuf and dqbuf, resulting in empty donebuf */
                    osMutexAcquire(cam->buf_lock, osWaitForever);
                    if ((cam->done_buf != NULL) && (cam->done_buf->index == index)) {
                    cam->done_buf->user_cnt++;
                    cam->done_buf->client_cnt[RPMSG_CLIENT]++;
                    } else {
                        dprintf(SCS_LOG_NOTE, "%s %d: client%d, done_buf %p\n",
                                __func__,__LINE__, RPMSG_CLIENT, cam->done_buf);
                    }
                    osMutexRelease(cam->buf_lock);
                }
            } else {
                dprintf(SCS_LOG_NOTE, "%s %d: client%d, osMutexAcquire Timeout %d\n",
                       __func__,__LINE__, RPMSG_CLIENT, ret);
            }
        }
    }

    if (rpmsg_ap2->client_sts == SCS_STREAM_ON) {
        if (client_id == RPMSG_CLIENT_AP2) {
            if (cam->mchan[SCS_RPMSG_CLIENT_AP2]) {
                ret = osMutexAcquire(cam->clients[RPMSG_CLIENT_AP2].mchan_lock,
                                             MCHAN_TIMEOUT_MS);
                if (ret == 0) {
                    ret =  mbox_send(cam->mchan[SCS_RPMSG_CLIENT_AP2], data, len, 1000);
                    osMutexRelease(cam->clients[RPMSG_CLIENT_AP2].mchan_lock);
                    if (ret < 0) {
                        completion_done(&cam->err_process);
                        dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d, mbox_send %d, buf %x\n",
                                __func__, __LINE__, cam->cam_id, RPMSG_CLIENT_AP2, ret, index);
                    } else {
                        osMutexAcquire(cam->buf_lock, osWaitForever);
                        if ((cam->done_buf != NULL) && (cam->done_buf->index == index)) {
                        cam->done_buf->user_cnt++;
                        cam->done_buf->client_cnt[RPMSG_CLIENT_AP2]++;
                        } else {
                            dprintf(SCS_LOG_NOTE, "%s %d: client%d, done_buf %p\n",
                                    __func__,__LINE__, RPMSG_CLIENT_AP2, cam->done_buf);
                        }
                        osMutexRelease(cam->buf_lock);
                    }
                } else {
                    dprintf(SCS_LOG_NOTE, "%s %d: client%d, osMutexAcquire Timeout %d\n",
                            __func__,__LINE__, RPMSG_CLIENT_AP2, ret);
                }
            }
         } else {
            osMutexAcquire(cam->dma_chan_lock, osWaitForever);
            if ((NULL != cam->dma_chan) && (addr != 0))
               notify_ap2_client(cam, addr, timestamp);
            osMutexRelease(cam->dma_chan_lock);
         }
   }

	if (local->client_sts == SCS_STREAM_ON) {
		osMutexAcquire(local->lock, osWaitForever);
		list_for_every_entry(&local->user_event_q, evt, struct event_item, list) {
			//dprintf(SCS_LOG_NOTE, "%s %d &evt->event	%x\n",	__func__,__LINE__, &evt->event);
			completion_done(&evt->event);

		}
		osMutexRelease(local->lock);
	}

    return 0;
}

void cam_qbuf_all(struct cam_device *cam, struct list_node *head, int client, bool check_client)
{
    struct buf_item *buf = NULL;
    int index;
    struct vdev_buffer cam_qbuf;
    int qbuf_cnt = 0;

    int hw_buf_cnt = list_length(&cam->hw_buf_q);

	while (!list_is_empty(head)) {
		buf = buf_item_pop(head);
		index = buf->index;
		cam_qbuf.buffer_idx = index;
		cam_qbuf.paddr[0] = (void *)(paddr_t)(buf->addrs[0]);
		cam_qbuf.paddr[1] = (void *)(paddr_t)(buf->addrs[1]);
		cam_qbuf.paddr[2] = (void *)(paddr_t)(buf->addrs[2]);

		hal_cam_qbuf(cam->cam_handle, &cam_qbuf);
        list_add_tail(&cam->hw_buf_q, &buf->list);
        qbuf_cnt++;
	}

    if (check_client && (qbuf_cnt == 0) && (hw_buf_cnt < 2)) {
        osMutexAcquire(cam->err_lock, osWaitForever);
        if (cam->err_flag[client] == false) {
            completion_done(&cam->err_process);
            dprintf(SCS_LOG_NOTE, "%s %d: hw_buf_cnt:%d\n",	__func__,__LINE__, hw_buf_cnt);
            #if 0
            int ret = 0;
            ret = check_client_err(cam, client, RESTART_NUM);
            if (ret != 0) {
                cam->err_flag[client] = true;
                completion_done(&cam->err_process);
                dprintf(SCS_LOG_NOTE, "%s %d: client%d, check_client_err%d\n",
                    __func__,__LINE__, client, ret);
            }
            #endif
        }
        osMutexRelease(cam->err_lock);
    }
}

static void camera_frame_handler(void *arg)
{
    int ret, timeout, i;
    struct cam_device *cam = (struct cam_device *)arg;
    //struct vdev_buffer cam_qbuf;
    struct vdev_buffer cam_dqbuf;
    uint32_t index;
	uint32_t client_id;

    struct client *local = &cam->clients[LOCAL_CLIENT];
    struct client *rpmsg = &cam->clients[RPMSG_CLIENT];
    struct client *rpmsg_ap2 = &cam->clients[RPMSG_CLIENT_AP2];
    //struct list_node *node = NULL;

    struct buf_item *buf = NULL;

    while (1) {

        wait_completion_timeout(&cam->trigger_frm_thread, osWaitForever);

        if (cam->frm_thread_should_exit)
            break;

       // dprintf(SCS_LOG_NOTE, "%s: cam%d wakeup\n", __func__, cam->cam_id);

        cam->frm_cnt = 0;
        cam->frame_run = true;
        while (1) {
			osMutexAcquire(cam->run_lock, osWaitForever);
			if (cam->stream_cnt == 0) {
				osMutexRelease(cam->run_lock);
				break;
			}
			osMutexRelease(cam->run_lock);

            i = 0;
            do {
                cam_dqbuf.timeout_ms = 1500;
                ret = hal_cam_dqbuf(cam->cam_handle, &cam_dqbuf);
                if (ret == 0) {
                    timeout = 0;
                    break;
                }
                break;
                timeout = 1;
            } while (0);

			index = -1;

            osMutexAcquire(cam->run_lock, osWaitForever);
			osMutexAcquire(cam->buf_lock, osWaitForever);

			if (cam->done_buf != NULL) {
				cam->done_buf->busy = false;
				if (cam->done_buf->user_cnt == 0) {
					//dprintf(SCS_LOG_CRI,
							  //"%s:%d cam%d buf index:%x is no qbuf, usercnt %d\n",
							  //__func__, __LINE__, cam->cam_id, cam->done_buf->index, cam->done_buf->user_cnt);
					client_id = (cam->done_buf->index & 0xffff0000) >> 16;
					index = cam->done_buf->index & 0xffff;
					list_add_head(&cam->clients[client_id].pend_buf_q,
					&cam->done_buf->list);
					if ((client_id == RPMSG_CLIENT) || (client_id == RPMSG_CLIENT_AP2)) {
						if (list_length(&cam->clients[client_id].pend_buf_q) == cam->clients[client_id].buf_cnt)
						completion_done(&cam->clients[client_id].pend_event);
					}
					cam->done_buf = NULL;
				} else {
					dprintf(SCS_LOG_CRI,
							  "%s:%d cam%d buf index:%x, usercnt %d, user_buf_q L:%d\n",
							  __func__, __LINE__, cam->cam_id, cam->done_buf->index,
							  cam->done_buf->user_cnt, list_length(&cam->user_buf_q));
                    buf->is_copy = false;
					list_add_head(&cam->user_buf_q, &cam->done_buf->list);
                    print_buf_item_index(&cam->user_buf_q);
					cam->done_buf = NULL;
				}
			}

			if (rpmsg->stream_cnt) {
				cam_qbuf_all(cam, &rpmsg->pend_buf_q, RPMSG_CLIENT, true);
			} else if (rpmsg_ap2->stream_cnt) {
				cam_qbuf_all(cam, &rpmsg_ap2->pend_buf_q, RPMSG_CLIENT_AP2, true);
			} else if (local->stream_cnt) {
				cam_qbuf_all(cam, &local->pend_buf_q, LOCAL_CLIENT, false);
			} else {
				dprintf(SCS_LOG_CRI,
						  "%s:%d cam%d not client stream on\n",
						  __func__, __LINE__, cam->cam_id);
			}

            if (ret || timeout) {
				dprintf(SCS_LOG_CRI,
						  "%s:%d cam%d user_buf_q L:%d\n",
						  __func__, __LINE__, cam->cam_id, list_length(&cam->user_buf_q));
                print_buf_item_index(&cam->user_buf_q);
                dprintf(SCS_LOG_CRI,
                        "%s:%d cam%d dqbuf ret %d, timeout %d\n",
                        __func__, __LINE__, cam->cam_id, ret, timeout);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_DQ_TIMEOUT,cam->cam_id);
				osMutexRelease(cam->buf_lock);
                osMutexRelease(cam->run_lock);
                continue;
            }

            //buf = &cam->clients[client_id].bufs[index];
            buf = index_buf_item_pop(cam_dqbuf.buffer_idx, &cam->hw_buf_q);
            client_id = (cam_dqbuf.buffer_idx & 0xffff0000) >> 16;
            if (buf != NULL) {
                index = cam_dqbuf.buffer_idx & 0xffff;
                buf->busy = true;
                //buf->index = index;
                buf->user_cnt = 0;
                for (i = 0; i < MAX_CLIENT; i++) {
                    buf->client_cnt[i] = 0;
                }
                buf->qbuf_cnt = 0;
                buf->time_stamp = cam_dqbuf.timestamp;
                if ((cam->clients[client_id].client_sts == SCS_CLOSE) ||
                    (cam->clients[client_id].client_sts == SCS_STREAM_OFF)) {
                    dprintf(SCS_LOG_NOTE, "client%d is %s\n",
                        client_id, (cam->clients[client_id].client_sts ==
                        SCS_CLOSE) ? "CLOSE" : "STREAM_OFF");
                    dprintf(SCS_LOG_NOTE,
                    "cbuf%d, Forced release buf index:0x%x\n",
                    client_id, cam_dqbuf.buffer_idx);

                    if (cam->clients[client_id].client_sts == SCS_STREAM_OFF)
                        list_add_head(&cam->clients[client_id].pend_buf_q,
                            &buf->list);

                    osMutexRelease(cam->buf_lock);
                    osMutexRelease(cam->run_lock);
                    continue;
                }
                cam->done_buf = buf;
            } else {
                dprintf(SCS_LOG_NOTE, "cam%d, error hw_buf_q no buf index:0x%x\n",
                        cam->cam_id, cam_dqbuf.buffer_idx);
                osMutexRelease(cam->buf_lock);
                osMutexRelease(cam->run_lock);
                continue;
            }
			osMutexRelease(cam->buf_lock);
            osMutexRelease(cam->run_lock);

            if ((cam->frm_cnt & 0xf) == 110) {
                for (i = 0; i < cam->planes; i++) {
                    uint8_t *pval = cam->clients[client_id].bufs[index].vaddrs[0];
                    dprintf(
                        SCS_LOG_DBG,
                        "plane%d, paddr:%p, addr:%p: %02x%02x%02x%02x "
                        "%02x%02x%02x%02x %02x%02x%02x%02x %02x%02x%02x%02x\n",
                        i, cam_dqbuf.paddr[0], pval, pval[0], pval[1], pval[2],
                        pval[3], pval[4], pval[5], pval[6], pval[7], pval[8],
                        pval[9], pval[10], pval[11], pval[12], pval[13],
                        pval[14], pval[15]);
                }
            }
			//dprintf(SCS_LOG_CRI, "%s %d cam->done_buf 0x%x, index:%d\n",
			//__func__,__LINE__, cam->done_buf, cam->done_buf->index);

            safe_cam_notify_agent(cam, cam_dqbuf.buffer_idx, cam_dqbuf.timestamp);//notify local linux
        }
		cam->frame_run = false;
		completion_done(&cam->frame_idle);
    }

    dprintf(SCS_LOG_NOTE, "cam%d camera_frame_handler exit\n", cam->cam_id);
    cam->frm_thread_exit = 1;
    osThreadExit();
}

int cam_service_probe_device(struct cam_device *cam, int instance)
{
    osThreadAttr_t attr = {
        .priority = osPriorityNormal,
    };

    if (cam_confs[instance].enabled) {
        cam->enabled = true;
        cam->sync = false;
        cam->user_cnt = 0;
        cam->streaming_map = 0;
        cam->open_map = 0;
        cam->cam_handle = NULL;
        cam->rproc = cam_confs[instance].rproc;
        cam->mbox_addr = cam_confs[instance].mbox_addr;
        cam->cam_id = cam_confs[instance].cam_id;

        if (cam->cam_id >= CAMERA_NUM) {
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_PROBE_FAIL,cam->cam_id);
            goto err_ret;
        }

        cam->frm_thread_should_exit = 0;
        cam->frm_thread_exit = 0;
        completion_init(&cam->trigger_frm_thread);
        cam->frm_thread =
            (void *)osThreadNew(camera_frame_handler, (void *)cam, &attr);

        dprintf(SCS_LOG_NOTE, "%s:cam_id:%d probe ok\n", __func__, cam->cam_id);
        return 0;
    }

err_ret:
    cam->enabled = false;
    return -1;
}

int camera_init(struct cam_device *cam)
{
    int i, ret = 0;
    camera_handle *handle = NULL;
    uint32_t fmt_list[MAX_PIXFMT_COUNT];

    memset(fmt_list, 0xf, sizeof(fmt_list));
    handle = hal_cam_get_instance(cam->cam_id);
    if (handle == NULL) {
        return -1;
    } else {
        ret = hal_cam_init(handle);
        if (ret != 0) {
            cam->cam_handle = handle;
            goto init_err;
        }

        cam->cam_handle = handle;
        dprintf(SCS_LOG_NOTE, "cam%d init OK\n", cam->cam_id);

        hal_cam_get_size(cam->cam_handle, &cam->w, &cam->h);

        dprintf(SCS_LOG_NOTE, "cam%d: getsize %d %d\n", cam->cam_id, cam->w, cam->h);

        if (hal_cam_enum_format(cam->cam_handle, fmt_list) < 0) {
            cam->fmts_size = 0;
            goto err;
        }

        for (i = 0; i < MAX_PIXFMT_COUNT; i++) {
            if (fmt_list[i] >= VDEV_PIX_FMT_NUM)
                break;

            cam->fmts[i] = to_client_fmt(fmt_list[i]);
            dprintf(SCS_LOG_INFO, "cam%d: no%d, fmt 0x%08x\n", cam->cam_id, i,
                    cam->fmts[i]);

            if (i == 0)
                cam->pixfmt = cam->fmts[i];
        }
        cam->fmts_size = i;
    }

    dprintf(SCS_LOG_NOTE, "cam%d init done\n", cam->cam_id);

    return 0;

err:
    hal_cam_deinit(cam->cam_handle);
init_err:
    hal_cam_put_instance(cam->cam_handle);
    cam->cam_handle = NULL;
    dprintf(SCS_LOG_CRI, "cam%d init failed\n", cam->cam_id);
    return -1;
}

void __stream_off(struct cam_device *cam, struct client *c, int id)
{
    struct vdev_buffer cam_buf;
	struct local_item *p_local = NULL;
	int i = 0;
    struct client *local = &cam->clients[LOCAL_CLIENT];
    struct client *rpmsg = &cam->clients[RPMSG_CLIENT];
    struct client *rpmsg_ap2 = &cam->clients[RPMSG_CLIENT_AP2];

	if (c->client_sts < SCS_STREAM_ON || cam->cam_sts < SCS_STREAM_ON) {
		dprintf(SCS_LOG_CRI, "cam%d: stream off err\n", cam->cam_id);
		return;
	}

	if (id == LOCAL_CLIENT) {
		p_local = thread_id_local_item_peek(cam->cur_thread_id, &c->user_local_q);
		if (p_local == NULL) {
			dprintf(SCS_LOG_CRI, "%s:%d cam%d: client%d, thread:%x is no open\n",__func__, __LINE__,
			cam->cam_id, id, cam->cur_thread_id);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_LOCAL_USER_NULL,cam->cam_id);
			return;
		} else {
			p_local->client_sts = SCS_STREAM_OFF;
		}
	}

	c->stream_cnt--;

	osMutexAcquire(cam->run_lock, osWaitForever);
	cam->stream_cnt--;

	if (c->stream_cnt > 0) {
		osMutexRelease(cam->run_lock);
		return;
	}
	c->client_sts = SCS_STREAM_OFF;

    if (rpmsg->stream_cnt) {
        cam_qbuf_all(cam, &rpmsg->pend_buf_q, RPMSG_CLIENT, false);
    } else if (rpmsg_ap2->stream_cnt) {
        cam_qbuf_all(cam, &rpmsg_ap2->pend_buf_q, RPMSG_CLIENT_AP2, false);
    } else if (local->stream_cnt) {
        cam_qbuf_all(cam, &local->pend_buf_q, LOCAL_CLIENT, false);
    } else {
        dprintf(SCS_LOG_CRI,
                    "%s:%d cam%d not client stream on\n",
                    __func__, __LINE__, cam->cam_id);
    }

	osMutexAcquire(cam->dma_chan_lock, osWaitForever);
	if (((id == RPMSG_CLIENT) || (id == RPMSG_CLIENT_AP2)) && (NULL != cam->dma_chan)) {
		dma_chan_free(cam);
	}
	osMutexRelease(cam->dma_chan_lock);

	if (cam->stream_cnt > 0) {
		osMutexRelease(cam->run_lock);
		return;
	}
	osMutexRelease(cam->run_lock);

	cam->cam_sts = SCS_STREAM_OFF;

	hal_cam_stop(cam->cam_handle);

	while(cam->frame_run) {
	    if (wait_completion_timeout(&cam->frame_idle, 30) < 0) {
			if (++i > 100) {
				dprintf(SCS_LOG_CRI, "%s:%d frame_idle error %d ----\n", __func__, __LINE__, i);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_WAIT_FRMIDLE_FAIL,cam->cam_id);
				break;
			}
			dprintf(SCS_LOG_CRI, "%s:%d frame_idle timeout %d ----\n", __func__, __LINE__, i);
	    } else {
			break;
	    }
    }

	osMutexAcquire(cam->buf_lock, osWaitForever);
	if (!list_is_empty(&cam->user_buf_q)) {
		struct buf_item *bufi;
		struct buf_item *tmp;
		list_for_every_entry_safe(&cam->user_buf_q, bufi, tmp, struct buf_item, list) {
			if (bufi->user_cnt == 0) {
				list_delete(&bufi->list);
				cam_buf.buffer_idx = bufi->index;
				cam_buf.paddr[0] = (void *)(paddr_t)(bufi->addrs[0]);
				cam_buf.paddr[1] = (void *)(paddr_t)(bufi->addrs[1]);
				cam_buf.paddr[2] = (void *)(paddr_t)(bufi->addrs[2]);

				hal_cam_qbuf(cam->cam_handle, &cam_buf);
                list_add_tail(&cam->hw_buf_q, &bufi->list);
			} else {
				/* If other clients do not return buf before off, an exception will occur */
				list_delete(&bufi->list);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_CAM_STS,cam->cam_id);
				dprintf(SCS_LOG_CRI, "%s:%d err buf index %x user cnt %d\n",
				__func__, __LINE__, bufi->index, bufi->user_cnt);
				dprintf(SCS_LOG_CRI, "%s:%d Forcible release buf index %x user cnt %d\n",
				__func__, __LINE__, bufi->index, bufi->user_cnt);
			}
		}
	}

    if (cam->done_buf != NULL) {
        if (cam->done_buf->user_cnt != 0) {
            dprintf(SCS_LOG_CRI, "%s:%d done_buf in user\n",
            __func__, __LINE__);
        } else {
            cam->done_buf->busy = false;
            cam_buf.buffer_idx = cam->done_buf->index;
            cam_buf.paddr[0] = (void *)(paddr_t)(cam->done_buf->addrs[0]);
            cam_buf.paddr[1] = (void *)(paddr_t)(cam->done_buf->addrs[1]);
            cam_buf.paddr[2] = (void *)(paddr_t)(cam->done_buf->addrs[2]);
            dprintf(SCS_LOG_CRI, "qbuf. 0x%05x\n", cam_buf.buffer_idx);
            hal_cam_qbuf(cam->cam_handle, &cam_buf);
            list_add_tail(&cam->hw_buf_q, &cam->done_buf->list);
            cam->done_buf = NULL;
        }
    }

	osMutexRelease(cam->buf_lock);

	return;
}
static void dma_transfer_done_cb(uint32_t status, uint32_t param, void *context)
{
	struct cam_device *cam = (struct cam_device *)context;

	if (DMA_COMPLETED != status) {
		dprintf(SCS_LOG_CRI, "%s:%d cam%d: dma status %d\n",
			__func__, __LINE__, cam->cam_id, status);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_CAM_STS,status);
	}

	completion_done(&cam->transfer_done);
}

static void dma_chan_req(struct cam_device *cam)
{
	if (NULL == cam->dma_chan) {
		cam->dma_chan = dma_req_chan_with_id(dma_dev, 1);
		if (NULL == cam->dma_chan) {
			dprintf(SCS_LOG_CRI, "alloc dma_chan failed\n");
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_DMACHAN_NULL,cam->cam_id);
			return;
		}
		cam->dma_chan->context = (void *)cam;
		cam->dma_chan->irq_cb = dma_transfer_done_cb;

	} else {
		dprintf(SCS_LOG_CRI, "%s:%d cam%d: dma_chan is not NULL\n",
			__func__, __LINE__, cam->cam_id);
	}

	return;
}

static int dma_transfer(struct cam_device *cam, paddr_t src_addr, paddr_t des_addr)
{
	uint8_t tr_ret = 0;
	void *src = (void *)(ap2p((paddr_t)src_addr));
	void *des = (void *)(ap2p((paddr_t)des_addr));

	tr_ret = dma_prep_memcpy(cam->dma_chan, des, src,
			cam->size[0], DMA_INTERRUPT);

	if (0 == tr_ret) {
		dprintf(SCS_LOG_CRI, "%s:%d cam%d: set transfer config err\n",
			__func__, __LINE__, cam->cam_id);
		return -1;
	}
	dma_flush_cache((addr_t)src_addr, cam->size[0]);
	dma_invalidate_cache((addr_t)des_addr, cam->size[0]);

	dma_start(cam->dma_chan);

	wait_completion_timeout(&cam->transfer_done, osWaitForever);

	dma_stop(cam->dma_chan);

	return 0;
}

static void notify_ap2_client(struct cam_device *cam, paddr_t addr,
                                uint64_t timestamp)
{
    struct client *rpmsg_ap2 = &cam->clients[RPMSG_CLIENT_AP2];
    struct buf_item *buf = NULL;
    uint32_t index = -1;
	uint8_t data[10];
    uint32_t len = 10;
	int ret = 0;
    int i = 0;
    struct buf_item *buf_tmp = NULL;

    osMutexAcquire(cam->buf_lock, osWaitForever);
	if (!list_is_empty(&rpmsg_ap2->pend_buf_q)) {
		buf = buf_item_pop(&rpmsg_ap2->pend_buf_q);
	} else {
		dprintf(SCS_LOG_CRI, "%s:%d cam%d: pend_buf_q is empty, user q %d\n",
			__func__, __LINE__, cam->cam_id, list_length(&cam->user_buf_q));
		for (i = 0; i < list_length(&cam->user_buf_q); i++) {
		        buf_tmp = index_buf_item_peek(i, &cam->user_buf_q);
		        for (i = 0; i < MAX_CLIENT; i++) {
		                dprintf(SCS_LOG_NOTE, "client%d, buf %x, user %d; ",
		                        i, buf_tmp->index, buf_tmp->client_cnt[i]);
		        }
		}
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_PEND_BUFQ_NULL,cam->cam_id);
	}
    osMutexRelease(cam->buf_lock);

    if (buf) {
        index = buf->index & 0xffff;
        dprintf(SCS_LOG_DBG, "dma copy img %d from 0x%lx to 0x%lx at %d\n", index,
            (unsigned long)addr, (unsigned long)buf->addrs[0], (uint32_t)current_time());

		ret = dma_transfer(cam, addr, buf->addrs[0]);
		if (ret < 0) {
			dprintf(SCS_LOG_CRI, "%s:%d cam%d: dma_transfer error %d\n",
				__func__, __LINE__, cam->cam_id, ret);
			osMutexAcquire(cam->buf_lock, osWaitForever);
			list_add_head(&rpmsg_ap2->pend_buf_q, &buf->list);
			osMutexRelease(cam->buf_lock);
			return;
		}

		data[0] = (uint8_t)cam->cam_id;
		data[1] = index;
		memcpy((void *)&data[2], (void *)&timestamp, 8);

        ret = osMutexAcquire(cam->clients[RPMSG_CLIENT_AP2].mchan_lock,
                                     MCHAN_TIMEOUT_MS);
        if (ret == 0) {
            ret =  mbox_send(cam->mchan[SCS_RPMSG_CLIENT_AP2], data, len, 1000);
            osMutexRelease(cam->clients[RPMSG_CLIENT_AP2].mchan_lock);
            if (ret < 0) {
                dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d, mbox_send error %d, buf %x\n",
                    __func__, __LINE__, cam->cam_id, RPMSG_CLIENT_AP2, ret, index);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_DCF_NOTIFY_ERROR,cam->cam_id);
                osMutexAcquire(cam->buf_lock, osWaitForever);
                list_add_head(&rpmsg_ap2->pend_buf_q, &buf->list);
                osMutexRelease(cam->buf_lock);

                osMutexAcquire(cam->err_lock, osWaitForever);
                if (cam->err_flag[RPMSG_CLIENT_AP2] == false)
                    completion_done(&cam->err_process);
                osMutexRelease(cam->err_lock);
            } else {
                osMutexAcquire(cam->buf_lock, osWaitForever);
                buf->user_cnt++;
                buf->client_cnt[RPMSG_CLIENT_AP2]++;
                buf->is_copy = true;
                list_add_head(&cam->user_buf_q, &buf->list);
                osMutexRelease(cam->buf_lock);
            }
        } else {
            dprintf(SCS_LOG_NOTE, "%s %d: client%d, osMutexAcquire Timeout%d\n",
                 __func__,__LINE__, RPMSG_CLIENT_AP2, ret);
        }
    }

	return;
}

static void dma_chan_free(struct cam_device *cam)
{
	if (NULL != cam->dma_chan) {
		dma_free_chan(cam->dma_chan);
		cam->dma_chan = NULL;
	} else {
		dprintf(SCS_LOG_CRI, "%s:%d cam%d: dma_chan is NULL\n",
			__func__, __LINE__, cam->cam_id);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_DMACHAN_NULL,cam->cam_id);
	}
}

int process_camera_request(struct cam_device *cam, struct scs_ioctl_cmd *ctl,
                           struct scs_ioctl_result *r, int remote_id)
{
    int ret = 0;
    uint32_t i = 0;
    uint32_t index = 0;
    struct client *c;
    struct client *local = &cam->clients[LOCAL_CLIENT];
    struct client *rpmsg = &cam->clients[RPMSG_CLIENT];
    struct client *rpmsg_ap2 = &cam->clients[RPMSG_CLIENT_AP2];
    struct vdev_buffer cam_buf;
    struct list_node *node = NULL;
	struct buf_item *pb = NULL;
	struct local_item *p_local = NULL;
	int id = remote_id;
	int buf_client = -1;

	if (remote_id == SOCKET_CLIENT) {
		id = LOCAL_CLIENT;
	}

	buf_client = id;

    c = &cam->clients[id];
    dprintf(SCS_LOG_DBG, "\n%s: cam%d cid %d pri %d, op %d\n", __func__,
            cam->cam_id, id, c->primary, ctl->op);
    if (c->user_cnt == 0 && ctl->op != SCS_OP_DEV_OPEN) {
        dprintf(SCS_LOG_CRI, "Fail,need open first,op:%d\n", ctl->op);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_CAM_NOT_ENABLE,ctl->op);
        if (ctl->op == SCS_OP_STREAM_OFF)
            return 0;

        return -ENOENT;
    }

    switch (ctl->op) {
    case SCS_OP_DEV_OPEN:
        dprintf(SCS_LOG_NOTE, "cam%d: open\n", cam->cam_id);
        dprintf(SCS_LOG_NOTE, "size %d %d\n",
                (uint32_t)sizeof(struct scs_ioctl_cmd),
                (uint32_t)sizeof(struct scs_ioctl_result));

		dprintf(SCS_LOG_NOTE, "%s:%d %s cam%d:client%d, cam user %d, client user %d\n",
		 __func__, __LINE__, cmd_to_str(ctl->op), cam->cam_id, id, cam->user_cnt, c->user_cnt);

		c->user_cnt++;

		if (id == LOCAL_CLIENT) {
			p_local = local_item_pop(&c->pend_local_q);
			if (p_local == NULL) {
				dprintf(SCS_LOG_CRI, "%s:%d cam%d: pend_local_q is NULL\n", __func__, __LINE__, cam->cam_id);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_LOCAL_USER_NULL,cam->cam_id);
				ret = -1;
				c->user_cnt--;
				break;
			} else {
				p_local->client_sts = SCS_OPEN;
				p_local->thread_id = cam->cur_thread_id;
				p_local->remote_id = remote_id;
				list_add_head(&c->user_local_q, &p_local->list);
				dprintf(SCS_LOG_CRI, "%s:%d cam%d: p_local->thread_id is %x\n",
				__func__, __LINE__, cam->cam_id, p_local->thread_id);
			}

		}

		c->client_sts = SCS_OPEN;
		osMutexAcquire(cam->err_lock, osWaitForever);
		cam->err_flag[id] = false;
		osMutexRelease(cam->err_lock);
		c->buf_cnt = 0;
		cam->user_cnt++;
        c->dev = cam;
        cam->open_map |= (1 << id);

        if (cam->user_cnt > 1) {
        	break;
		}

		if (cam->cam_sts > SCS_CLOSE) {
        	break;
		}

		cam->cam_sts = SCS_OPEN;

        ret = camera_init(cam);
        if (ret < 0) {
            c->client_sts = 0;
            cam->cam_sts = 0;
			cam->user_cnt--;
			cam->open_map &= ~(1 << id);
	        c->dev = NULL;
			c->user_cnt--;

            dprintf(SCS_LOG_CRI, "cam%d: cam_dev_init() fail\n", cam->cam_id);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_LOCAL_USER_NULL,cam->cam_id);
            ret = -ERR_BUSY;
            r->msg.data[0] = -EBUSY;
            break;
        }

        set_fmt_info(cam);

        osMutexAcquire(cam->buf_lock, osWaitForever);
		cam->done_buf = NULL;
        list_initialize(&cam->user_buf_q);
        list_initialize(&cam->hw_buf_q);
        osMutexRelease(cam->buf_lock);

        r->msg.data[0] = 0;
        dprintf(SCS_LOG_NOTE, "cam%d: SCS_OP_DEV_OPEN done\n", cam->cam_id);

        break;

    case SCS_OP_ENUM_FORMAT: {
        uint32_t found = 0;

        dprintf(SCS_LOG_NOTE, "cam%d: SCS_OP_ENUM_FORMAT index %d\n",
                cam->cam_id, ctl->msg.fmt.index);
        if (cam->streaming_map == 0) {
            for (i = 0; i < cam->fmts_size; i++) {
                if (i == ctl->msg.fmt.index) {
                    r->msg.fmt.fmt = cam->fmts[i];
                    r->msg.fmt.index = i;
                    found = 1;
                    dprintf(SCS_LOG_INFO,
                            "cam%d: enum format: 0x%08x, index %d\n",
                            cam->cam_id, r->msg.fmt.fmt, i);
                    break;
                }
            }
        } else {
            if (ctl->msg.fmt.index == 0) {
                r->msg.fmt.index = 0;
                r->msg.fmt.fmt = cam->pixfmt;
                found = 1;
                dprintf(SCS_LOG_INFO, "cam%d: enum format: 0x%08x, index %d\n",
                        cam->cam_id, r->msg.fmt.fmt, ctl->msg.fmt.index);
            }
        }

        if (found == 0) {
            r->msg.fmt.index = 0;
            ret = 0;
        }

        dprintf(SCS_LOG_NOTE,
                "cam%d: SCS_OP_ENUM_FORMAT done index %d, fmt 0x%08x\n",
                cam->cam_id, r->msg.fmt.index, r->msg.fmt.fmt);
        break;
    }

    case SCS_OP_ENUM_FRAMESIZE: {
        if (ctl->msg.fsz.index > 0) {
            r->msg.fitv.index = 0;
            ret = 0;
            break;
        }
        r->msg.fsz.type = V4L2_FRMSIZE_TYPE_DISCRETE;
        r->msg.fsz.width = cam->w;
        r->msg.fsz.height = cam->h;
        r->msg.fsz.index = 0;

        dprintf(SCS_LOG_NOTE, "cam%d: enum framesize: (%d %d), index=%d\n",
                cam->cam_id, cam->w, cam->h, r->msg.fsz.index);
        break;
    }

    case SCS_OP_ENUM_FRAMEINTERVALS: {
        if (ctl->msg.fitv.index > 0) {
            r->msg.fitv.index = 0;
            ret = 0;
            break;
        }
        r->msg.fitv.type = V4L2_FRMSIZE_TYPE_DISCRETE;
        r->msg.fitv.index = 0;
        ret = hal_cam_get_fps(cam->cam_handle, &r->msg.fitv.denominator);
        if (ret) {
            dprintf(SCS_LOG_NOTE, "ret:%d \n", ret);
            break;
        }

        r->msg.fitv.numerator = 1;

        dprintf(SCS_LOG_NOTE, "cam%d: enum frameinterval: (%d), index=%d\n",
                cam->cam_id, r->msg.fitv.denominator, r->msg.fitv.index);
        break;
    }

    case SCS_OP_GET_FORMAT:
        r->msg.g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        r->msg.g_fmt.width = cam->w;
        r->msg.g_fmt.height = cam->h;
        r->msg.g_fmt.pixelformat = cam->pixfmt;
        dprintf(SCS_LOG_NOTE, "cam%d: get fmt: 0x%08x\n", cam->cam_id,
                cam->pixfmt);
        break;

    case SCS_OP_SET_FORMAT:
        dprintf(SCS_LOG_NOTE,
                "cam%d: SCS_OP_SET_FORMAT 0x%08x, pri %d, (%d %d), (%d %d)\n",
                cam->cam_id, ctl->msg.s_fmt.pixelformat, c->primary, cam->w,
                cam->h, ctl->msg.s_fmt.width, ctl->msg.s_fmt.height);

		if (c->client_sts >= SCS_SET_FORMAT) {
			r->msg.g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			r->msg.g_fmt.width = cam->w;
			r->msg.g_fmt.height = cam->h;
			r->msg.g_fmt.pixelformat = cam->pixfmt;
			break;
		}

		c->client_sts = SCS_SET_FORMAT;

        if (c->client_sts > cam->cam_sts) {
            uint32_t local_fmt;
            if ((cam->w != ctl->msg.s_fmt.width) ||
                (cam->h != ctl->msg.s_fmt.height)) {
                // ret = -1;
                // goto rt;
                dprintf(SCS_LOG_NOTE,
                        "cam%d: SCS_OP_SET_FORMAT 0x%08x, pri %d, (%d %d), (%d "
                        "%d) mismatch\n",
                        cam->cam_id, ctl->msg.s_fmt.pixelformat, c->primary,
                        cam->w, cam->h, ctl->msg.s_fmt.width,
                        ctl->msg.s_fmt.height);
            }

            local_fmt = to_local_fmt(ctl->msg.s_fmt.pixelformat);
            ret = hal_cam_set_format(cam->cam_handle, local_fmt);
            if (ret < 0) {
                dprintf(SCS_LOG_INFO, "cam%d: hal_cam_set_format format err\n", cam->cam_id);
	            goto rt;
            }
            cam->pixfmt = ctl->msg.s_fmt.pixelformat;
            set_fmt_info(cam);
			cam->cam_sts = SCS_SET_FORMAT;
        }

        // may cause error if open at the same time
        r->msg.g_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        r->msg.g_fmt.width = cam->w;
        r->msg.g_fmt.height = cam->h;
        r->msg.g_fmt.pixelformat = cam->pixfmt;

        dprintf(SCS_LOG_NOTE, "cam%d: set format: 0x%08x, size %d x %d\n",
                cam->cam_id, cam->pixfmt, cam->w, cam->h);

        break;

    case SCS_OP_QUEUE_SETUP:
        dprintf(SCS_LOG_NOTE, "cam%d: queue setup: %d, %d\n", cam->cam_id,
                ctl->msg.queue.num_bufs, ctl->msg.queue.num_planes);
        uint8_t *vaddr = NULL;
        int cnt = 0;
		if (remote_id == SOCKET_CLIENT) {
			osMutexAcquire(c->ubuf_lock, osWaitForever);
			list_initialize(&c->upend_buf_q);
			for (uint32_t m = 0; m < MAX_CAM_BUF; m++) {
				list_initialize(&c->ubufs[m].list);
				for (i = 0; i < 3; i++) {
					c->ubufs[m].addrs[i] = 0;
				}
			}
			osMutexRelease(c->ubuf_lock);
		}
		//in the light of local & ulink,
		//Local client support users,
		//Ulink is implemented as a local client
		if (c->client_sts >= SCS_QUEUE_SETUP &&
			remote_id != RPMSG_CLIENT && remote_id != RPMSG_CLIENT_AP2) {
			r->msg.queue.bufs = c->buf_cnt;
			r->msg.queue.planes = cam->planes;
			r->msg.queue.size[0] = cam->size[0];
			r->msg.queue.size[1] = cam->size[1];
			break;
		}

		c->client_sts = SCS_QUEUE_SETUP;

		if (cam->cam_sts == SCS_STREAM_OFF && cam->user_cnt == 1) {
				cam->cam_sts = SCS_QUEUE_SETUP;
                osMutexAcquire(cam->buf_lock, osWaitForever);
                list_initialize(&cam->hw_buf_q);
                osMutexRelease(cam->buf_lock);
		}

        if (ctl->msg.queue.num_bufs > MAX_CAM_BUF)
            r->msg.queue.bufs = MAX_CAM_BUF;
        else
            r->msg.queue.bufs = ctl->msg.queue.num_bufs;

        r->msg.queue.planes = cam->planes;
        r->msg.queue.size[0] = cam->size[0];
        r->msg.queue.size[1] = cam->size[1];

        /*
         * for those who don't call SCS_OP_DEV_CLOSE when exiting...
         */
        osMutexAcquire(cam->buf_lock, osWaitForever);

        c->buf_cnt = r->msg.queue.bufs;
        list_initialize(&c->pend_buf_q);
		c->qbuf_max = -1;

        cnt = get_client_buf_cnt(id, &cam->hw_buf_q);
        for (i = 0; i < cnt; i++) {
            client_buf_item_pop(id, &cam->hw_buf_q);
        }

		for (uint32_t m = 0; m < MAX_CAM_BUF; m++) {
			list_initialize(&c->bufs[m].list);
			c->bufs[m].user_cnt = 0;
			for (i = 0; i < MAX_CLIENT; i++) {
				c->bufs[m].client_cnt[i] = 0;
			}
			c->bufs[m].busy = false;
			c->bufs[m].qbuf_cnt = 0;
			for (i = 0; i < 3; i++) {
				c->bufs[m].addrs[i] = 0;
				c->bufs[m].vaddrs[i] = NULL;
			}
		}

        osMutexRelease(cam->buf_lock);

        dprintf(SCS_LOG_NOTE, "cam%d: queue setup OK: %d, %d, size= %d %d\n",
                cam->cam_id, r->msg.queue.bufs, r->msg.queue.planes,
                r->msg.queue.size[0], r->msg.queue.size[1]);
		c->m_type = ctl->msg.queue.m_type;
		if (ctl->msg.queue.m_type == SCS_MEMORY_MMAP) {
			vaddr = (uint8_t *)malloc(r->msg.queue.bufs * cam->size[0]);
			if (vaddr == NULL) {
				dprintf(SCS_LOG_CRI, "cam%d: %d malloc buffer failed size %d\n",
						cam->cam_id, id, cam->size[0]);
				ret = -1;
				goto rt;
			}
			osMutexAcquire(cam->buf_lock, osWaitForever);
			for (uint32_t i = 0; i < c->buf_cnt; i++) {
				c->bufs[i].index = i | (id << 16);
	            c->bufs[i].addrs[0] =
	            (uint32_t)p2ap((paddr_t)(vaddr + cam->size[0] * i));
				c->bufs[i].vaddrs[0] = vaddr + i * cam->size[0];

	            list_add_head(&c->pend_buf_q, &c->bufs[i].list);
				dprintf(SCS_LOG_NOTE, "cam%d: buf index=%d addr:0x%x\n",
						cam->cam_id, i, c->bufs[i].addrs[0]);
			}
			osMutexRelease(cam->buf_lock);
		}

        break;

    case SCS_OP_GET_BUFINFO:
        dprintf(SCS_LOG_NOTE, "cam%d: get bufinfo: index=%d\n", cam->cam_id,
                r->msg.g_bufinfo.index);
        index = r->msg.g_bufinfo.index;

        if (index >= c->buf_cnt) {
            dprintf(SCS_LOG_CRI, "cam%d: c.%d get buf index %d error\n",
                    cam->cam_id, id, r->msg.g_bufinfo.index);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_BUF_INDEX,index);
            ret = -1;
            goto rt;
        }

		osMutexAcquire(cam->buf_lock, osWaitForever);
        r->msg.g_bufinfo.addr[0] = c->bufs[index].addrs[0];
        r->msg.g_bufinfo.addr[1] = c->bufs[index].addrs[1];
		osMutexRelease(cam->buf_lock);
        break;

    case SCS_OP_SET_BUFINFO://set buf addr, mmap not set
        dprintf(SCS_LOG_NOTE, "cam%d: set bufinfo: index=%x, addr=0x%x 0x%x\n",
                cam->cam_id, ctl->msg.s_bufinfo.index,
                ctl->msg.s_bufinfo.addr[0], ctl->msg.s_bufinfo.addr[1]);

        index = ctl->msg.s_bufinfo.index;
		if (remote_id == SOCKET_CLIENT) {
			osMutexAcquire(c->ubuf_lock, osWaitForever);
			c->ubufs[index].index = index;
			c->ubufs[index].addrs[0] = ctl->msg.s_bufinfo.addr[0];
			c->ubufs[index].addrs[1] = ctl->msg.s_bufinfo.addr[1];
			list_add_head(&c->upend_buf_q, &c->ubufs[index].list);
			osMutexRelease(c->ubuf_lock);
			break;
		}
		if (c->m_type == SCS_MEMORY_MMAP) {
            dprintf(SCS_LOG_CRI, "invalid operation\n");
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_OP,c->m_type);
            ret = -1;
			break;
		}
        uint8_t *target_addr[3] = {NULL, NULL, NULL};

        if (index >= c->buf_cnt) {
            dprintf(SCS_LOG_CRI, "cam%d: c.%d set buf index %x error\n",
                    cam->cam_id, id, ctl->msg.s_bufinfo.index);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_BUF_INDEX,index);
            ret = -1;
            goto rt;
        }

        for (i = 0; i < cam->planes; i++)
            target_addr[i] =//get vaddr
                (uint8_t *)(ap2p((paddr_t)ctl->msg.s_bufinfo.addr[i]));
		//get paddr
		osMutexAcquire(cam->buf_lock, osWaitForever);
		c->bufs[index].index = index | (id << 16);
        c->bufs[index].addrs[0] = (paddr_t)ctl->msg.s_bufinfo.addr[0];
        c->bufs[index].addrs[1] = (paddr_t)ctl->msg.s_bufinfo.addr[1];
        c->bufs[index].vaddrs[0] = target_addr[0];
        c->bufs[index].vaddrs[1] = target_addr[1];
        c->bufs[index].vaddrs[2] = target_addr[2];
		list_add_head(&c->pend_buf_q, &c->bufs[index].list);
		osMutexRelease(cam->buf_lock);
		dprintf(SCS_LOG_CRI, "cam%d: client%d set buf index %x\n",
				cam->cam_id, id, c->bufs[index].index);

        break;
    case SCS_OP_QBUF:
        dprintf(SCS_LOG_DBG,
                "cam%d: qbuf: ctl->msg.s_bufinfo.index=%d primary %d\n",
                cam->cam_id, ctl->msg.s_bufinfo.index, c->primary);

		if (id != RPMSG_CLIENT && id != RPMSG_CLIENT_AP2)
			 buf_client = (ctl->msg.s_bufinfo.index & 0xffff0000) >> 16;

		index = ctl->msg.s_bufinfo.index & 0xffff;

		//dprintf(SCS_LOG_CRI, "%s:%d cam%d s_bufinfo index:%d, qbuf_max %d\n",
		//	__func__, __LINE__, cam->cam_id, index, cam->qbuf_max);
		if (index >= cam->clients[buf_client].buf_cnt) {
			dprintf(SCS_LOG_CRI, "%s:%d cam%d: client%d qbuf index %d error\n",
			__func__, __LINE__, cam->cam_id, id, index);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_INVALID_BUF_INDEX,index);
			ret = -1;
			goto rt;
		}
#ifdef CONFIG_CSI_SVC_VIA_UNILINK
		if (remote_id == SOCKET_CLIENT) {
			p_local = remote_id_local_item_peek(remote_id, &c->user_local_q);
			if (p_local == NULL) {
				dprintf(SCS_LOG_CRI,
						  "%s:%d cam%d no ulink client%d\n", __func__, __LINE__, cam->cam_id, remote_id);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_REMOTE_USER_NULL,remote_id);
				ret = -1;
				goto rt;
			} else {
				if (p_local->client_sts < SCS_STREAM_ON) {

				} else {
					osMutexAcquire(c->ubuf_lock, osWaitForever);
					list_add_head(&c->upend_buf_q, &c->ubufs[index].list);
					osMutexRelease(c->ubuf_lock);
					goto rt;
				}
			}
		}
#endif
		if (c->client_sts < SCS_STREAM_ON) {
			if (cam->cam_sts < SCS_STREAM_ON) {
				osMutexAcquire(cam->buf_lock, osWaitForever);
				if (c->qbuf_max < index || c->qbuf_max < 0) {
					c->qbuf_max = index;
					cam_buf.buffer_idx = index | (buf_client << 16);
					cam_buf.paddr[0] = (void *)(paddr_t)(cam->clients[buf_client].bufs[index].addrs[0]);
					cam_buf.paddr[1] = (void *)(paddr_t)(cam->clients[buf_client].bufs[index].addrs[1]);
					cam_buf.paddr[2] = (void *)(paddr_t)(cam->clients[buf_client].bufs[index].addrs[2]);
					pb = index_buf_item_pop(index | (buf_client << 16), &c->pend_buf_q);

					if (pb != NULL) {
						hal_cam_qbuf(cam->cam_handle, &cam_buf);
                        list_add_tail(&cam->hw_buf_q, &pb->list);
					}
					dprintf(SCS_LOG_CRI,
							  "%s:%d cam%d buf index:%x\n",
							  __func__, __LINE__, cam->cam_id, cam_buf.buffer_idx);

					osMutexRelease(cam->buf_lock);
					break;
				} else {
					dprintf(SCS_LOG_CRI, "%s:%d cam%d s_bufinfo index:%d, qbuf_max %d\n",
							__func__, __LINE__, cam->cam_id, index, c->qbuf_max);
					c->qbuf_max = index;
					osMutexRelease(cam->buf_lock);
					goto rt;
				}

			} else {
				dprintf(SCS_LOG_CRI, "%s:%d cam%d s_bufinfo index:%d, qbuf_max %d\n",
						__func__, __LINE__, cam->cam_id, index, c->qbuf_max);
				c->qbuf_max = index;
				goto rt;
			}
		} else {
			if (cam->cam_sts < SCS_STREAM_ON) {
				dprintf(SCS_LOG_CRI, "%s:%d cam%d invalid SCS_OP_QBUF operation index:%d, qbuf_max %d\n",
						__func__, __LINE__, cam->cam_id, index, c->qbuf_max);
				ret = -1;
				goto rt;
			} else {
				if (id == LOCAL_CLIENT) {
					p_local = thread_id_local_item_peek(cam->cur_thread_id, &c->user_local_q);
					if (p_local == NULL) {
						dprintf(SCS_LOG_CRI, "cam%d: client%d, thread:%x is no open\n",
						cam->cam_id, id, cam->cur_thread_id);
					} else {
						if (p_local->client_sts < SCS_STREAM_ON)
							break;
					}
				}

				osMutexAcquire(cam->buf_lock, osWaitForever);
				if (cam->done_buf != NULL) {
					if ((index | (buf_client << 16)) != cam->done_buf->index) {
						pb = index_buf_item_peek(index | (buf_client << 16), &cam->user_buf_q);
						if (pb == NULL) {
							dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d cbuf%d invalid SCS_OP_QBUF operation index:%x\n",
								__func__, __LINE__, cam->cam_id, id, buf_client, ctl->msg.s_bufinfo.index);
							ret = -1;
							osMutexRelease(cam->buf_lock);
							goto rt;
						} else {
							pb->user_cnt--;
                            pb->client_cnt[remote_id]--;
							if (pb->user_cnt == 0) {
								list_delete(&pb->list);
								pb->busy = false;
                                if (pb->is_copy == false) {
								    dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d cbuf%d SCS_OP_QBUF operation not timely, index:%d\n",
								            __func__, __LINE__, cam->cam_id, id, buf_client, index);
                                    ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_QBUF_NOT_TIMELY,cam->cam_id);
                                }
								goto QBUF;
							}
							dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d cbuf%d invalid SCS_OP_QBUF operation index:%d\n",
								__func__, __LINE__, cam->cam_id, id, buf_client, index);
							osMutexRelease(cam->buf_lock);
							goto rt;
						}
					} else {
						cam->done_buf->user_cnt--;
                        cam->done_buf->client_cnt[remote_id]--;
						cam->done_buf->qbuf_cnt++;
						if ((cam->done_buf->qbuf_cnt == cam->stream_cnt)) {
							if (cam->done_buf->user_cnt != 0) {
								ret = -1;
								osMutexRelease(cam->buf_lock);
								goto rt;
							}
							cam->done_buf->busy = false;
							cam->done_buf = NULL;
							//osMutexRelease(cam->buf_lock);

							goto QBUF;
						}
						osMutexRelease(cam->buf_lock);
						goto rt;
					}
				} else {
					pb = index_buf_item_peek(index | (buf_client << 16), &cam->user_buf_q);
					if (pb == NULL) {
						dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d cbuf%d invalid SCS_OP_QBUF operation index:%d\n",
							__func__, __LINE__, cam->cam_id, id, buf_client, index);
						ret = -1;
						osMutexRelease(cam->buf_lock);
						goto rt;

					} else {
						pb->user_cnt--;
                        pb->client_cnt[remote_id]--;
						if (pb->user_cnt == 0) {
							list_delete(&pb->list);
							pb->busy = false;
                            if (pb->is_copy == false) {
							    dprintf(SCS_LOG_CRI, "%s:%d cam%d c%d cbuf%d SCS_OP_QBUF operation not timely, index:%d\n",
							        __func__, __LINE__, cam->cam_id, id, buf_client, index);
                            }

							goto QBUF;
						}
						dprintf(SCS_LOG_CRI, "%s:%d cam%d invalid SCS_OP_QBUF operation index:%d\n",
							__func__, __LINE__, cam->cam_id, index);
						osMutexRelease(cam->buf_lock);
						goto rt;
					}
				}
			}
		}
QBUF:

		//
		list_add_head(&cam->clients[buf_client].pend_buf_q, &cam->clients[buf_client].bufs[index].list);

		//Because of the logic of switching bufs,
		//the AP (12) needs to be notified after all bufs are recycled,
		//and the stream off interface exits waiting
		//The local client will get the buf of the AP directly
		if ((id == LOCAL_CLIENT) && ((buf_client == RPMSG_CLIENT) || (buf_client == RPMSG_CLIENT_AP2))) {
			if (list_length(&cam->clients[buf_client].pend_buf_q) == cam->clients[buf_client].buf_cnt)
			completion_done(&cam->clients[buf_client].pend_event);
		}

		if (rpmsg->stream_cnt) {
			pb = buf_item_pop(&rpmsg->pend_buf_q);
		} else if (rpmsg_ap2->stream_cnt) {
			pb = buf_item_pop(&rpmsg_ap2->pend_buf_q);
		} else if (local->stream_cnt) {
			pb = buf_item_pop(&local->pend_buf_q);
		} else {
			pb = NULL;
		}

		if (pb != NULL) {
			cam_buf.buffer_idx = pb->index;
	        cam_buf.paddr[0] = (void *)(paddr_t)(pb->addrs[0]);
	        cam_buf.paddr[1] = (void *)(paddr_t)(pb->addrs[1]);
	        cam_buf.paddr[2] = (void *)(paddr_t)(pb->addrs[2]);

	        hal_cam_qbuf(cam->cam_handle, &cam_buf);
            list_add_tail(&cam->hw_buf_q, &pb->list);
		}
        osMutexRelease(cam->buf_lock);
        r->msg.g_bufinfo.index = ctl->msg.s_bufinfo.index;

        dprintf(SCS_LOG_DBG,
                "cam%d: qbuf done: ctl->msg.s_bufinfo.index=%d primary %d\n",
                cam->cam_id, ctl->msg.s_bufinfo.index, c->primary);
        break;

    case SCS_OP_DQBUF:
        dprintf(SCS_LOG_DBG,
                "cam%d: dqbuf: ctl->msg.s_bufinfo.index=%d primary %d\n",
                cam->cam_id, ctl->msg.s_bufinfo.index, c->primary);
        r->msg.g_bufinfo.index = ctl->msg.s_bufinfo.index;

        break;

    case SCS_OP_STREAM_ON:
        dprintf(SCS_LOG_NOTE, "cam%d: stream on...\n", cam->cam_id);
        //int should_wakeup = 0;

		if (c->client_sts < SCS_QUEUE_SETUP) {
			dprintf(SCS_LOG_CRI, "cam%d: stream on err\n", cam->cam_id);
			break;
		}

		c->stream_cnt++;

		if (id == LOCAL_CLIENT) {
			p_local = thread_id_local_item_peek(cam->cur_thread_id, &c->user_local_q);
			if (p_local == NULL) {
				dprintf(SCS_LOG_CRI, "cam%d: client%d, thread:%x is no open\n",
				cam->cam_id, id, cam->cur_thread_id);
			} else {
				p_local->client_sts = SCS_STREAM_ON;
			}
		}

		osMutexAcquire(cam->run_lock, osWaitForever);

		cam->stream_cnt++;
		osMutexRelease(cam->run_lock);

		if (c->client_sts == SCS_STREAM_ON) {
			break;
		}

		c->client_sts = SCS_STREAM_ON;

		if ((id == RPMSG_CLIENT) || (id == RPMSG_CLIENT_AP2)) {
			if ((rpmsg->client_sts == SCS_STREAM_ON) && (rpmsg_ap2->client_sts == SCS_STREAM_ON)) {
		        osMutexAcquire(cam->dma_chan_lock, osWaitForever);
				dma_chan_req(cam);
		        osMutexRelease(cam->dma_chan_lock);
			}
		}

		if (cam->cam_sts == SCS_STREAM_ON) {
			break;
		}

		cam->cam_sts = SCS_STREAM_ON;

        hal_cam_start(cam->cam_handle);

		completion_done(&cam->trigger_frm_thread);

        dprintf(SCS_LOG_NOTE, "cam%d: stream on...done\n", cam->cam_id);
        break;

    case SCS_OP_STREAM_OFF:
        dprintf(SCS_LOG_NOTE, "cam%d: stream off...\n", cam->cam_id);

		__stream_off(cam, c, id);

        dprintf(SCS_LOG_NOTE, "cam%d: stream off done...\n", cam->cam_id);
        break;

    case SCS_OP_DEV_CLOSE:
        dprintf(SCS_LOG_NOTE, "cam%d: close...\n", cam->cam_id);

		if (id == LOCAL_CLIENT) {
			p_local = thread_id_local_item_peek(cam->cur_thread_id, &c->user_local_q);
			if (p_local != NULL && p_local->client_sts == SCS_STREAM_ON) {
				__stream_off(cam, c, id);
                stream_wait_pending(cam, id);
			}

			p_local = thread_id_local_item_pop(cam->cur_thread_id, &c->user_local_q);
			if (p_local == NULL) {
				dprintf(SCS_LOG_CRI, "%s:%d cam%d: client%d, thread:%x is no open\n",
								__func__, __LINE__,
								cam->cam_id, id, cam->cur_thread_id);

			} else {
				dprintf(SCS_LOG_CRI, "%s:%d cam%d: client%d, thread:%x close\n",
								__func__, __LINE__,
								cam->cam_id, id, cam->cur_thread_id);

				p_local->client_sts = SCS_CLOSE;
				p_local->thread_id = 0;
				list_add_head(&c->pend_local_q, &p_local->list);
			}
		} else {
			if (c->client_sts == SCS_STREAM_ON) {
				__stream_off(cam, c, id);
                stream_wait_pending(cam, id);
			}
		}

		c->user_cnt--;

		if (c->user_cnt > 0) {
			break;
		}
		c->client_sts = SCS_CLOSE;

        osMutexAcquire(cam->buf_lock, osWaitForever);
		if (c->m_type == SCS_MEMORY_MMAP) {
            if (c->bufs[0].vaddrs[0] != NULL) {
			    free(c->bufs[0].vaddrs[0]);
                c->bufs[0].vaddrs[0] = NULL;
            }
		}

        osMutexRelease(cam->buf_lock);

		osMutexAcquire(c->lock, osWaitForever);
		while (!list_is_empty(&c->user_event_q)) {
			dprintf(SCS_LOG_NOTE, "%s:%d  cam%d:user%d, cam user %d, client user %d, user_event_q %d\n",
			__func__, __LINE__, cam->cam_id, id, cam->user_cnt,
			c->user_cnt, list_length(&c->user_event_q));
			node = list_remove_tail(&c->user_event_q);
			if (node != NULL) {
				list_add_head(&c->pend_event_q, node);
			}
		}

		osMutexRelease(c->lock);

		cam->user_cnt--;
		if (cam->user_cnt > 0) {
			break;
		}
		cam->cam_sts = SCS_CLOSE;

        hal_cam_deinit(cam->cam_handle);
        hal_cam_put_instance(cam->cam_handle);
        cam->cam_handle = NULL;

        osMutexAcquire(cam->buf_lock, osWaitForever);
        list_initialize(&cam->user_buf_q);
        list_initialize(&cam->hw_buf_q);
		cam->done_buf = NULL;
        osMutexRelease(cam->buf_lock);

        dprintf(SCS_LOG_NOTE, "cam%d: close...done\n", cam->cam_id);
        break;

    default:
        break;
    }

rt:
    dprintf(SCS_LOG_DBG, "%s cam%d cid %d, cmd=%d [time:%d] exit\n", __func__,
            cam->cam_id, id, ctl->op, current_time());

    if (!(ctl->op == SCS_OP_DQBUF || ctl->op == SCS_OP_QBUF))
		dprintf(SCS_LOG_NOTE, "%s:%d %s cam%d, client%d, cam user %d, client user %d, stream cnt %d\n",
	 __func__, __LINE__, cmd_to_str(ctl->op), cam->cam_id, id, cam->user_cnt, c->user_cnt, cam->stream_cnt);


    return ret;
}

int client_to_rproc(int client_id)
{
	switch (client_id) {
	case RPMSG_CLIENT:
	   return DP_CA_AP1;
	case RPMSG_CLIENT_AP2:
	   return DP_CA_AP2;
	default:
	   dprintf(SCS_LOG_CRI, "%s:%d client_id %d %s\n",
		   __func__, __LINE__, client_id,
		   "is exception handling is not supported");
	   return -1;
	}
}

void cam_off_close(struct cam_device *cam, int client)
{
	rpc_call_request_t request;
    struct scs_ioctl_cmd *ctl = (struct scs_ioctl_cmd *)&request.param[0];
	u16 rproc;
	int ret = 0;

	osMutexAcquire(cam->run_lock, osWaitForever);
	if (cam->clients[client].client_sts != SCS_CLOSE) {
		osMutexRelease(cam->run_lock);
		ret = client_to_rproc(client);
		if (ret >= 0) {
			rproc = ret;

			if (cam->clients[client].client_sts != SCS_STREAM_OFF) {
				ctl->op = SCS_OP_STREAM_OFF;
				ctl->instance = ((rproc << 8) & 0xff00) |
						(cam->instance & 0x00ff);
				/* off */
				cam_svr_ioctl_cb(NULL, &request);
			}

			ctl->op = SCS_OP_DEV_CLOSE;
			ctl->instance = ((rproc << 8) & 0xff00) |
					(cam->instance & 0x00ff);
			/* close */
			cam_svr_ioctl_cb(NULL, &request);
		} else {

		}
	} else {
		osMutexRelease(cam->run_lock);
	}
}

//Exception handling task
void cam_err_process_task(void *arg)
{
    struct cam_device *cam = (struct cam_device *)arg;
    int i = 0;
	//bool err_flag = false;
	int ret = 0;

   while (1) {
		if (wait_completion_timeout(&cam->err_process, 500) < 0) {
			/* check client active */
			for (i = 0; i < MAX_CLIENT; i++) {
				osMutexAcquire(cam->run_lock, osWaitForever);
				if (cam->clients[i].client_sts != SCS_CLOSE) {
					osMutexRelease(cam->run_lock);

					osMutexAcquire(cam->err_lock, osWaitForever);
					if (cam->err_flag[i] == false) {
						ret = check_client_err(cam, i, RESTART_NUM);
						if (ret != 0) {
							cam->err_flag[i] = true;
							dprintf(SCS_LOG_NOTE, "%s %d: client%d, check_client_err %d\n",
							__func__,__LINE__, i, ret);
						}
					}
					osMutexRelease(cam->err_lock);
				} else {
					osMutexRelease(cam->run_lock);
				}
			}
		}

		for (i = 0; i < MAX_CLIENT; i++) {
			osMutexAcquire(cam->err_lock, osWaitForever);
			if (cam->err_flag[i]) {
				osMutexRelease(cam->err_lock);
				cam_off_close(cam, i);
			} else {
				osMutexRelease(cam->err_lock);
			}
		}
   }

   dprintf(SCS_LOG_NOTE, "cam%d cam_err_process_task exit\n", cam->cam_id);
}

//call process_camera_request
static void camera_handle_task(void *arg)
{
    struct cam_device *cam = (struct cam_device *)arg;

    while (1) {
        wait_completion_timeout(&cam->trigger_request, osWaitForever);
        cam->ret_val = process_camera_request(cam, cam->ctl, cam->r, cam->cid);
        completion_done(&cam->request_done);
    }

    dprintf(SCS_LOG_NOTE, "cam%d camera_handle_task exit\n", cam->cam_id);
    osThreadExit();
}

int handle_camera_request(struct scs_ioctl_cmd *ctl, struct scs_ioctl_result *r,
                          int id)
{
    int ret;
    struct cam_device *cam;
    uint32_t cam_id;

    cam_id = ctl->instance & 0x00ff;
    dprintf(SCS_LOG_DBG, "\n%s: cam%d cid %d op %d\n", __func__, cam_id, id, ctl->op);

    cam = &s_cdev[cam_id];

    osMutexAcquire(cam->dev_lock, osWaitForever);
	cam->cur_thread_id = (uint32_t)osThreadGetId();

    cam->ctl = ctl;
    cam->r = r;
    cam->cid = id;
    completion_done(&cam->trigger_request);
    wait_completion_timeout(&cam->request_done, osWaitForever);
    ret = cam->ret_val;

    osMutexRelease(cam->dev_lock);

    dprintf(SCS_LOG_DBG, "%s: cam%d cid %d done ret %d\n", __func__,
            cam->cam_id, id, ret);
    return ret;
}

/* for rpmesg client */
rpc_call_result_t cam_svr_ioctl_cb(rpc_server_handle_t hserver,
                                   rpc_call_request_t *request)
{
    rpc_call_result_t result = {
        0,
    };
    struct scs_ioctl_cmd *ctl = (struct scs_ioctl_cmd *)&request->param[0];
    struct scs_ioctl_result *r = (struct scs_ioctl_result *)&result.result[0];
    int client_index = RPMSG_CLIENT;
    u16 rproc = (ctl->instance >> 8) & 0xff;

    if (!request) {
        result.retcode = ERR_INVALID_ARGS;
        return result;
    }

    result.ack = request->cmd;

    if (rproc == MBOX_RPROC_AP2) {
        client_index = RPMSG_CLIENT_AP2;
    }

    result.retcode = handle_camera_request(ctl, r, client_index);

    if (ctl->op == SCS_OP_STREAM_OFF) {
        uint32_t cam_id = ctl->instance & 0x00ff;
        struct cam_device *cam = &s_cdev[cam_id];
        if (cam->stream_cnt != 0) {
            stream_wait_pending(cam, client_index);
        }
  	}

    return result;
}

int create_camera_notifier(struct cam_device *cam)
{
    sdrv_mbox_chan_req_t chan_req;

    chan_req.dest_addr = cam->mbox_addr;
    chan_req.src_addr = cam->mbox_addr;
    for (int32_t i = 0; i < SCS_RPMSG_CLIENT_NUM; i++) {
        if (cam->rproc[i] != -1) {
            chan_req.rproc = cam->rproc[i];
            cam->mchan[i] = mbox_request_channel(g_mbox_dev, &chan_req);
            if (NULL == cam->mchan[i]) {
                dprintf(SCS_LOG_CRI, "cam mbox request channel failed for client: %d!!\n", i);
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_MBOX_REQ_CHANNEL_FAIL,i);
                return -1;
            }
            dprintf(SCS_LOG_CRI, "cam%d create ap%d notifier\n",
                                    cam->cam_id, i + 1);
        }
        else {
            cam->mchan[i] = NULL;
        }
    }

    return 0;
}

void *scs_open(uint32_t cam_id)
{
    struct cam_device *cam;
    struct scs_ioctl_result r;
    struct scs_ioctl_cmd ctl = {
        .op = SCS_OP_DEV_OPEN,
        .instance = (u16)cam_id,
    };

    if (cam_id >= CAMERA_NUM) {
        return NULL;
    }

    cam = &s_cdev[cam_id];
    if (cam->enabled == 0) {
        dprintf(SCS_LOG_CRI, "cam%d is not enable\n", cam_id);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_CAM_NOT_ENABLE,cam_id);
        return NULL;
    }

    if (scs_ioctl(cam, &ctl, &r) < 0) {
        return NULL;
    }

    return (void *)cam;
}

void scs_close(void *handle)
{
    struct cam_device *cam = (struct cam_device *)handle;
    struct scs_ioctl_result r;
    struct scs_ioctl_cmd ctl;

    if (!handle) {
        return;
    }

    ctl.op = SCS_OP_DEV_CLOSE;
    ctl.instance = (u16)cam->cam_id;
    scs_ioctl(handle, &ctl, &r);
}
//local ioctl
int scs_ioctl(void *handle, struct scs_ioctl_cmd *ctl,
              struct scs_ioctl_result *r)
{
    int ret = 0;
    struct cam_device *cam = (struct cam_device *)handle;
    struct client *local;
    uint32_t index = 0;
    uint64_t timestamp = 0;
    struct list_node *node = NULL;
    struct event_item *evt = NULL;
    uint32_t client_id = 0;

    if (!handle || !ctl || !r) {
        return -1;
    }

    ctl->instance = (u16)cam->cam_id;
    local = &cam->clients[LOCAL_CLIENT];
    if (ctl->op != SCS_OP_DQBUF) {
        ret = handle_camera_request(ctl, r, LOCAL_CLIENT);//to ioctl
        if (ctl->op == SCS_OP_STREAM_OFF) {
			if (cam->stream_cnt != 0 && (local->stream_cnt == 0)) {
	            stream_wait_pending(cam, LOCAL_CLIENT);
            }
        }
        return ret;
    }

    osMutexAcquire(local->lock, osWaitForever);
    node = list_remove_tail(&local->pend_event_q);
    if (node == NULL) {
		dprintf(SCS_LOG_CRI, "local pend_event_q is empty\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_LOCAL_PEND_EVENTQ_EMPTY,cam->cam_id);
		osMutexRelease(local->lock);
        ret = -1;
        return ret;
    }

	evt = containerof(node, struct event_item, list);
	list_add_head(&local->user_event_q, &evt->list);
    osMutexRelease(local->lock);
	//dprintf(SCS_LOG_NOTE, "%s %d &evt->event	%x\n",  __func__,__LINE__, &evt->event);
    ret = wait_completion_timeout(&evt->event, 1500);//DQBUF wait

    osMutexAcquire(local->lock, osWaitForever);
	list_delete(&evt->list);
	list_add_head(&local->pend_event_q, &evt->list);
    osMutexRelease(local->lock);

    if (ret < 0) {
		dprintf(SCS_LOG_CRI, "event_wait_timeout %d\n", ret);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_DQ_TIMEOUT,cam->cam_id);
        ret = -1;
		return ret;
	}

    //osMutexAcquire(cam->dev_lock, osWaitForever);//Avoid garbled code
    osMutexAcquire(cam->buf_lock, osWaitForever);
    if (cam->done_buf == NULL) {
        ret = -1;
		dprintf(SCS_LOG_CRI, "%s:%d done_buf is null\n", __func__, __LINE__);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,CAMSERVICE_MOD,ERROR_CAMSERVICE_DONEBUF_NULL,cam->cam_id);
    } else {
        if (local->m_type == SCS_MEMORY_USERPTR) {
            client_id = (cam->done_buf->index & 0xffff0000) >> 16;
            if (client_id == LOCAL_CLIENT) {
                index = cam->done_buf->index;
                timestamp = cam->done_buf->time_stamp;
                cam->done_buf->user_cnt++;
                cam->done_buf->client_cnt[LOCAL_CLIENT]++;
                r->msg.g_bufinfo.timestamp = (uint32_t)(timestamp & 0xFFFFFFFF);
                r->msg.g_bufinfo.timestamp_h = (uint32_t)((timestamp >> 32) & 0xFFFFFFFF);

                r->msg.g_bufinfo.index = index;
                r->msg.g_bufinfo.addr[0] = cam->done_buf->addrs[0];
            } else {
                r->msg.g_bufinfo.index = 0xff;//switch user
                r->msg.g_bufinfo.addr[0] = cam->done_buf->addrs[0];
            }
        } else if (local->m_type == SCS_MEMORY_MMAP) {
            index = cam->done_buf->index;
            timestamp = cam->done_buf->time_stamp;
            cam->done_buf->user_cnt++;
            cam->done_buf->client_cnt[LOCAL_CLIENT]++;
            r->msg.g_bufinfo.timestamp = (uint32_t)(timestamp & 0xFFFFFFFF);
            r->msg.g_bufinfo.timestamp_h = (uint32_t)((timestamp >> 32) & 0xFFFFFFFF);

            r->msg.g_bufinfo.index = index;
            r->msg.g_bufinfo.addr[0] = cam->done_buf->addrs[0];
        } else {
            dprintf(SCS_LOG_CRI, "%s:%d Unsupported local m_type:%d\n",
                __func__, __LINE__, local->m_type);
            ret = -1;
        }
    }
    osMutexRelease(cam->buf_lock);

    return ret;
}

void cam_service_init(void)
{
    int ret = 0, j, k, m;
    struct cam_device *cam;
    osThreadAttr_t attr = {
        .priority = osPriorityNormal,
    };
    rpc_server_impl_t devfuncs[] = {
        {MOD_RPC_REQ_SCS_IOCTL, cam_svr_ioctl_cb, IPCC_RPC_NO_FLAGS},
    };

    if (binitialized)
        return;

    for (j = 0; j < (sizeof(cam_confs) / sizeof(cam_confs[0])); j++) {
        ret = cam_service_probe_device(&s_cdev[j], j);

        if (ret)
            continue;

        cam = &s_cdev[j];

        for (k = 0; k < MAX_CLIENT; k++) {
            struct client *c = &cam->clients[k];

            c->dev = NULL;
            c->user_cnt = 0;
            c->stream_cnt = 0;
            c->primary = 0;
            c->streaming = 0;
			c->lock = osMutexNew(NULL);
			c->ubuf_lock = osMutexNew(NULL);
            c->mchan_lock = osMutexNew(NULL);
            list_initialize(&c->user_event_q);
            list_initialize(&c->pend_event_q);
			list_initialize(&c->pend_buf_q);
			list_initialize(&c->upend_buf_q);
			completion_init(&c->pend_event);

			for (m = 0; m < MAX_CAM_BUF; m++) {
				list_initialize(&c->ubufs[m].list);
				list_initialize(&c->bufs[m].list);
			}

			list_initialize(&c->user_local_q);
			list_initialize(&c->pend_local_q);

            for (m = 0; m < MAX_LOCAL_USER; m++) {
				list_initialize(&c->locals[m].list);
                list_initialize(&c->events[m].list);
				completion_init(&c->events[m].event);
	            list_add_head(&c->pend_event_q, &c->events[m].list);
	            list_add_head(&c->pend_local_q, &c->locals[m].list);
            }
        }
        cam->buf_lock = osMutexNew(NULL);
		list_initialize(&cam->user_buf_q);
        list_initialize(&cam->hw_buf_q);
		cam->done_buf = NULL;

        completion_init(&cam->err_process);
        completion_init(&cam->transfer_done);
        completion_init(&cam->trigger_request);
        completion_init(&cam->request_done);
        completion_init(&cam->frame_idle);//notify stream off
        osThreadNew(camera_handle_task, (void *)cam, &attr);
        osThreadNew(cam_err_process_task, (void *)cam, &attr);

        cam->err_lock = osMutexNew(NULL);
        cam->dev_lock = osMutexNew(NULL);
        cam->run_lock = osMutexNew(NULL);
        cam->dma_chan_lock = osMutexNew(NULL);
        osMutexAcquire(cam->dev_lock, osWaitForever);
        create_camera_notifier(cam);
        osMutexRelease(cam->dev_lock);
    }

    start_ipcc_rpc_service(devfuncs, ARRAY_SIZE(devfuncs));
#ifdef CONFIG_CSI_SVC_VIA_UNILINK
    /* port is consumed even no client would connect */
    start_message_handler();
#endif

    binitialized = true;
}
