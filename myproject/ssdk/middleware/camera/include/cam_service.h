/**
 * @file cam_service.h
 * @brief camera service header file
 *
 * @copyright Copyright (c) 2022  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#ifndef __CAM_SERVICE_H__
#define __CAM_SERVICE_H__

#include <driver/memmap.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include "rpc/rpmsg_rpc.h"
#include "rpc/dcf_rpc.h"
#include "mbox_cfg.h"
#include "sdrv-cam-os-def.h"
#include "scs_com.h"
#include "cam_conf.h"
#include "driver/dma.h"

#define MAX_CAM_INST 9
#define MAX_CAM_BUF 16
#define MAX_PIXFMT_COUNT 8
#define MAX_LOCAL_USER 5

#ifndef _def_domain_cpu_id_t
#define _def_domain_cpu_id_t
typedef enum {
    DP_CR5_SAF,
    DP_CR5_SEC,
    DP_CR5_MPC,
    DP_CA_AP1,
    DP_CA_AP2,
    DP_DSP_V,
    DP_CPU_MAX,
    R_DP_CR5_SAF = DP_CPU_MAX,
    R_DP_CR5_SEC,
    R_DP_CR5_MPC,
    R_DP_CA_AP1,
    R_DP_CA_AP2,
    R_DP_DSP_V,
    ALL_DP_CPU_MAX,
} domain_cpu_id_t;
#endif

struct cam_device;
enum cam_staus {
    SCS_CLOSE,
	SCS_OPEN,
    SCS_SET_FORMAT,
    SCS_QUEUE_SETUP,
    SCS_STREAM_ON,
    SCS_STREAM_OFF,
};

enum client_id {
    LOCAL_CLIENT,
    RPMSG_CLIENT,//AP1
    SOCKET_CLIENT,
    RPMSG_CLIENT_AP2,
    MAX_CLIENT,
};

struct buf_item {
    uint32_t index;
    uint64_t time_stamp;
    uint8_t user_cnt;
	bool busy;
    bool is_copy;
	int qbuf_cnt;
    paddr_t addrs[3];
    uint8_t *vaddrs[3];
    int client_cnt[MAX_CLIENT];
    struct list_node list;
};

struct local_item {
    uint32_t thread_id;
	enum cam_staus client_sts;
    struct list_node list;
	int remote_id;//client id
};

struct event_item {
    completion_t event;
    struct list_node list;
};

struct client {
    struct cam_device *dev;
    /*
     * assume below operations issued consecutively:
     * group a:
     *   open
     *   init buf
     *   stream on
     * group b:
     *   stream off
     *   close
     */
    int user_cnt;
    int primary;
    int streaming;
    int pending;
    int switching;
    int waiting;

    osMutexId_t mchan_lock;

	struct buf_item ubufs[MAX_CAM_BUF];
    struct list_node upend_buf_q;
    osMutexId_t ubuf_lock;

	struct event_item events[MAX_LOCAL_USER];
    struct list_node user_event_q;
    struct list_node pend_event_q;
    osMutexId_t lock;

    struct list_node user_local_q;
	struct local_item locals[MAX_LOCAL_USER];
    struct list_node pend_local_q;


	int qbuf_max;
    uint32_t m_type;

    uint32_t buf_cnt;

    struct buf_item bufs[MAX_CAM_BUF];
    //spin_lock_t buf_lock;//user cam_device buflock
    struct list_node pend_buf_q;
	completion_t pend_event;

	enum cam_staus client_sts;
	uint32_t stream_cnt;
};

#define another(id) (!(id))

/* Define common csi data */
struct cam_device {
    osMutexId_t dev_lock;

    // bool binitialized;
    bool enabled;
    bool sync;
    uint32_t user_cnt;
    uint32_t streaming_map;
    uint32_t open_map;

    void *cam_handle;
    uint32_t cam_id;
    uint32_t fmts[MAX_PIXFMT_COUNT];
    uint32_t fmts_size;
    uint32_t pixfmt;
    uint32_t w;
    uint32_t h;
    uint32_t planes;
    uint32_t size[3];

    void *frm_thread;
    int frm_thread_should_exit;
    int frm_thread_exit;
    completion_t trigger_frm_thread;
    int frm_cnt;
    uint32_t cur_thread_id;

    completion_t frame_idle;//wait frm_thread idle
    bool frame_run;//notify frm_thread camera set stream off
    osMutexId_t run_lock;//

    struct list_node user_buf_q;
    //struct list_node pend_buf_q;
	struct buf_item *done_buf;
    osMutexId_t buf_lock;
    struct list_node hw_buf_q;

	enum cam_staus cam_sts;

	uint32_t stream_cnt;
    // struct csi_buffer *cb;

    /* communication level stuff */
    //struct dcf_notifier *notifier;
    mbox_chan_t *mchan[SCS_RPMSG_CLIENT_NUM];
    int32_t     *rproc;
    uint16_t mbox_addr;
    uint16_t instance;

    /*ap1 copy ap2*/
    dma_chan_t *dma_chan;
    completion_t transfer_done;
    osMutexId_t dma_chan_lock;

    struct client clients[MAX_CLIENT];

    completion_t err_process;
    osMutexId_t err_lock;
    bool err_flag[MAX_CLIENT];

    completion_t trigger_request;
    completion_t request_done;
    struct scs_ioctl_cmd *ctl;
    struct scs_ioctl_result *r;
    int cid;
    int ret_val;
};

/* api for safety local application */
void *scs_open(uint32_t cam_id);
int scs_ioctl(void *handle, struct scs_ioctl_cmd *ctl,
              struct scs_ioctl_result *r);
void scs_close(void *handle);

void cam_service_init(void);

#endif /* __CAM_SERVICE_H__ */
