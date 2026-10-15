/**
 * @file rpmsg_service.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <string.h>
#include <param.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <rpmsg_ns.h>
#include <rpmsg_virtio.h>
#include <rpmsg_ipcc.h>
#include <rpmsg_channel.h>
#include <mbox_cfg.h>
#include <sdrv_common_reg.h>

#if defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2)
#if CONFIG_BOARD_X9SP
#include "taihang_sem_lld_hw.h"
#else /* CONFIG_BOARD_X9SP */
#include "kunlun_sem_lld_hw.h"
#endif /* CONFIG_BOARD_X9SP */
#ifdef CONFIG_SYS_DIAGNOSIS
#include <sys_diagnosis.h>
#endif /* end CONFIG_SYS_DIAGNOSIS */
#ifdef CONFIG_KUNLUN_MBOX
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#endif /* end CONFIG_KUNLUN_MBOX */
#endif /* end defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2) */

#include "rpmsg_service.h"
#include "rpmsg_echo.h"
#include "rpmsg_service_cfg.h"

#ifdef CONFIG_USE_IPCC_RPC
#include "rpc/dcf_rpc.h"
#endif /* end CONFIG_USE_IPCC_RPC */

static struct rpmsg_instance *rpmsg_devs[RPMSG_TYPE_NUM][CONFIG_RPMSG_RPROC_NUM];
static struct rpmsg_lite_instance rpmsg_virtio_devs[CONFIG_RPMSG_RPROC_NUM];
static struct rpmsg_ipcc_instance rpmsg_ipcc_devs[CONFIG_RPMSG_RPROC_NUM];

#if defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2)
typedef int32_t (*signal_callback)(int32_t signal, void* args);

typedef struct tag_observer_ins {
    int32_t signal;
    void* usrCtx;
    signal_callback cb;
}observer_inst_t;

typedef struct tag_observer_mgr {
    mbox_dev_t** dev_ptr;
    mbox_chan_t* chan;
    observer_inst_t ins[3];
    osThreadId_t tid;
    osMessageQueueId_t msg_que;
}observer_mgr_t;

static observer_mgr_t s_observer_mgr[] = {
    { // 0 recv from safety
        .dev_ptr = &g_mbox_dev,
        .chan = NULL,
        .tid = NULL,
        .msg_que = NULL,
    },
    { // 1 recv from secure
        .dev_ptr = &g_mbox_dev,
        .chan = NULL,
        .tid = NULL,
        .msg_que = NULL,
    },
};
int c_observer_mgr = ARRAY_SIZE(s_observer_mgr);
#endif /* defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2) */

#ifdef CONFIG_KUNLUN_MBOX
/**
 * @brief rpmsg gen mbox addr.
 *
 * @param[in] curr_proc current proc.
 * @param[in] remote_proc remote proc.
 * @return result.
 */
static int rpmsg_mbox_addr_gen(uint32_t base_addr, int curr_proc, int remote_proc)
{
    int i = 0;
    int base = 0;
    int a, b;

    if (curr_proc == remote_proc) {
        return -1;
    }

    a = MIN(curr_proc, remote_proc);
    b = MAX(curr_proc, remote_proc);

    while (i < b) {
        base += i++;
    }

    return base_addr + base + a;
}
#endif /* CONFIG_KUNLUN_MBOX */

/**
 * @brief rpmsg ns callback.
 *
 * @param[in] new_ept new endpoint id.
 * @param[in] new_ept_name new endpoint name.
 * @param[in] flags flags.
 * @param[in] user_data user data.
 */
static void rpmsg_ns_callback(unsigned int new_ept,
                              const char *new_ept_name,
                              unsigned long flags,
                              void *user_data)
{
    ssdk_printf(SSDK_NOTICE, "%s: name:%s, addr:%d, notify:%s\n",
                __func__, new_ept_name, new_ept,
                (flags == RPMSG_NS_CREATE ? "create" : "destory"));

#ifdef CONFIG_RPMSG_WITH_LK
    struct rpmsg_instance *rpmsg_dev = user_data;

    if (rpmsg_dev->rpmsg_type == RPMSG_TYPE_IPCC) {
        struct rpmsg_endpoint *local_ept;

        list_for_every_entry(&rpmsg_dev->endpoints, local_ept,
                             struct rpmsg_endpoint, node) {
            bool need_announce, announced;
            rpmsg_channel_t chan;
            int ret;

            chan = rpmsg_channel_from_endpoint(local_ept);
            rpmsg_channel_announce_info(chan, &need_announce, &announced);

            if (chan && need_announce && !announced) {
                ssdk_printf(SSDK_DEBUG, "rpmsg_ns_callback: announce %s\n",
                    rpmsg_channel_name(chan));

                ret = rpmsg_ns_announce(rpmsg_dev, local_ept,
                    rpmsg_channel_name(chan), RPMSG_NS_CREATE);

                if (ret == RPMSG_SUCCESS) {
                    rpmsg_channel_set_announced(chan, true);
                }
                else {
                    ssdk_printf(SSDK_ERR, "rpmsg_ns_callback: announce %s failed.\n",
                        rpmsg_channel_name(chan));
                }
            }
        }
    }
#endif /* CONFIG_RPMSG_WITH_LK */
}

/**
 * @brief rpmsg virtio dev init.
 *
 * @param[in] service rpmsg service.
 * @param[in] rpmsg_dev rpmsg device.
 * @param[in] rpmsg_virtio_dev rpmsg virtio device.
 * @return result.
 */
static int rpmsg_virtio_init(rpmsg_service_t *service,
                             struct rpmsg_instance *rpmsg_dev,
                             struct rpmsg_lite_instance *rpmsg_virtio_dev)
{
    struct rpmsg_lite_config rpmsg_config;
    rpmsg_ns_handle ns_handle;
    int ret;
#ifdef CONFIG_KUNLUN_MBOX
    int addr;
#endif /* CONFIG_KUNLUN_MBOX */

    rpmsg_config.self_proc = service->self_proc;
    rpmsg_config.remote_proc = service->remote_proc;
    rpmsg_config.shm_phys_base = service->shm_phys_base;
    rpmsg_config.shm_size = service->shm_size;
    rpmsg_config.is_master = service->is_master;

#ifdef CONFIG_KUNLUN_MBOX
    addr = rpmsg_mbox_addr_gen(MBOX_ADDR_RPMSG_VIRTIO, service->self_proc, service->remote_proc);
    rpmsg_config.src_addr = addr;
    rpmsg_config.dst_addr = addr;
#endif /* CONFIG_KUNLUN_MBOX */

    rpmsg_config.mbox_ins = g_mbox_dev;

    ret = rpmsg_lite_probe(rpmsg_dev, rpmsg_virtio_dev, &rpmsg_config);
    if (ret != RPMSG_SUCCESS) {
        ssdk_printf(SSDK_ERR, "rpmsg_lite_probe failed.\n");
        return -1;
    }

    ns_handle = rpmsg_ns_bind(rpmsg_dev, rpmsg_ns_callback, rpmsg_dev);
    if (ns_handle == NULL || ns_handle->ept == NULL) {
        ssdk_printf(SSDK_ERR, "rpmsg_ns_bind failed.\n");
        return -1;
    }

    return 0;
}

/**
 * @brief rpmsg ipcc dev init.
 *
 * @param[in] service rpmsg service.
 * @param[in] rpmsg_dev rpmsg device.
 * @param[in] rpmsg_virtio_dev rpmsg virtio device.
 * @return result.
 */
static int rpmsg_ipcc_init(rpmsg_service_t *service,
                           struct rpmsg_instance *rpmsg_dev,
                           struct rpmsg_ipcc_instance *rpmsg_ipcc_dev)
{
    struct rpmsg_ipcc_config rpmsg_config;
    rpmsg_ns_handle ns_handle;
#ifdef CONFIG_KUNLUN_MBOX
    int addr;
#endif /* CONFIG_KUNLUN_MBOX */
    int ret;

    rpmsg_config.self_proc = service->self_proc;
    rpmsg_config.remote_proc = service->remote_proc;

#ifdef CONFIG_KUNLUN_MBOX
    addr = rpmsg_mbox_addr_gen(MBOX_ADDR_RPMSG_IPCC, service->self_proc, service->remote_proc);
    rpmsg_config.src_addr = addr;
    rpmsg_config.dst_addr = addr;
#endif /* CONFIG_KUNLUN_MBOX */

    rpmsg_config.mbox_ins = g_mbox_dev;
    rpmsg_config.is_master = service->is_master;

    ret = rpmsg_ipcc_probe(rpmsg_dev, rpmsg_ipcc_dev, &rpmsg_config);
    if (ret != RPMSG_SUCCESS) {
        ssdk_printf(SSDK_ERR, "rpmsg_ipcc_probe failed.\n");
        return -1;
    }

    ns_handle = rpmsg_ns_bind(rpmsg_dev, rpmsg_ns_callback, rpmsg_dev);
    if (ns_handle == NULL || ns_handle->ept == NULL) {
        ssdk_printf(SSDK_ERR, "rpmsg_ns_bind failed.\n");
        return -1;
    }

    return 0;
}

#if defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2)

static void ap_observer_task(void* args)
{
    int32_t ret;
    observer_mgr_t* obsMgr_ptr = (observer_mgr_t*)args;
    uint16_t signal;

    while (1) {
        ret = osMessageQueueGet(obsMgr_ptr->msg_que, &signal, NULL, osWaitForever);
        if (ret != osOK) {
            ssdk_printf(SSDK_EMERG, "get observer message queue error \n");
            continue;
        }
        else if (signal != -1) {
            for (int32_t i = 0; i < ARRAY_SIZE(obsMgr_ptr->ins); i++) {
                if (obsMgr_ptr->ins[i].signal == signal &&
                    obsMgr_ptr->ins[i].cb != NULL) {
                    obsMgr_ptr->ins[i].cb(signal, obsMgr_ptr->ins[i].usrCtx);
                }
            }
        }
        else {
            break;
        }
    }
    osThreadExit();
}

static int32_t ap_observer_mbox_callback(void* args, uint8_t* data, uint32_t len)
{
    if (args == NULL || data == NULL) {
        ssdk_printf(SSDK_EMERG, "parameter error for %s \n", __func__);
        return -1;
    }
    observer_mgr_t* obsMgr_ptr = (observer_mgr_t *)args;

    uint16_t signal = *((uint16_t *)(data));
    osMessageQueuePut(obsMgr_ptr->msg_que, &signal, 0, 0);

    return 0;
}

static int32_t ap_observer_reister_cb(signal_callback cb, void* args, int32_t signal)
{
    for (int i = 0; i < c_observer_mgr; i++) {
        observer_mgr_t* obsMgr_ptr = &(s_observer_mgr[i]);

        if (obsMgr_ptr->msg_que == NULL) {
            // create messageque
            obsMgr_ptr->msg_que = osMessageQueueNew(10, sizeof(uint16_t), NULL);
            if (obsMgr_ptr->msg_que == NULL) {
                continue;
            }
        }

        if (obsMgr_ptr->tid == NULL) {
            // create task
            osThreadAttr_t attr = {
                .name = "ap_observer",
                .priority = 9,
                .stack_size = 1024,
            };
            obsMgr_ptr->tid = osThreadNew(ap_observer_task, (void*)obsMgr_ptr, &attr);
            if (obsMgr_ptr->tid == NULL) {
                continue;
            }
        }

        if (obsMgr_ptr->chan == NULL) {
            sdrv_mbox_chan_req_t chan_req;
            chan_req.rproc = i;
            chan_req.dest_addr = 0x7F;
            chan_req.src_addr = 0x7F;
            obsMgr_ptr->chan = mbox_request_channel(*obsMgr_ptr->dev_ptr, &chan_req);
            if (obsMgr_ptr->chan == NULL) {
                ssdk_printf(SSDK_EMERG, "request mbox channel for ap domain observer error\n");
                continue;
            }

            mbox_set_callback(obsMgr_ptr->chan, ap_observer_mbox_callback, obsMgr_ptr);

            // init ins
            for (int32_t i = 0; i < ARRAY_SIZE(obsMgr_ptr->ins); i++) {
                obsMgr_ptr->ins[i].signal = -1;
            }
        }

        int32_t cnt = 0;
        for (int32_t cnt = 0; cnt < ARRAY_SIZE(obsMgr_ptr->ins); cnt++) {
            if (obsMgr_ptr->ins[cnt].signal == -1) {
                obsMgr_ptr->ins[cnt].signal = signal;
                obsMgr_ptr->ins[cnt].cb = cb;
                obsMgr_ptr->ins[cnt].usrCtx = args;
                break;
            }
        }

        if (cnt >= ARRAY_SIZE(obsMgr_ptr->ins)) {
            ssdk_printf(SSDK_WARNING, "no availid ins for reigster signal: %d \n", signal);
            continue;
        }
    }

    return 0;
}

/**
 * @brief rpmsg service wait link
 *
 * @param service rpmsg service
 * @return true
 * @return false
 */
bool rpmsg_device_wait_link_up(rpmsg_service_t* service)
{
    struct rpmsg_instance *rpmsg_dev = rpmsg_devs[service->rpmsg_type][service->remote_proc];

    if (rpmsg_dev != NULL &&
        rpmsg_dev->ins_ops->set_link_status != NULL) {
        rpmsg_dev->ins_ops->set_link_status(rpmsg_dev, RPMSG_LINKSTATE_DOWN);
    }

    if (rpmsg_dev != NULL &&
        !service->is_master) {
        int retry = 100;
        while (!rpmsg_is_link_up(rpmsg_dev)) {
            if (retry-- == 0) {
                ssdk_printf(SSDK_ERR, "rpmsg_service_init wait remote[%d:%d] failed for 20s.\n",
                    service->rpmsg_type, service->remote_proc);
                // rpmsg_deinit(rpmsg_dev);
                // rpmsg_devs[service->rpmsg_type][service->remote_proc] = NULL;
                // osFree(rpmsg_dev);
                // return false;
                retry = 100;
            }

            rpmsg_kick_remote(rpmsg_dev);
            osDelay(200 * osKernelGetTickFreq() / 1000);
        }
    }

    return true;
}

void rpmsg_device_reset_work(void* data)
{
    if (!data) {
        ssdk_printf(SSDK_EMERG, "error for data of param is null\n");
        goto exit;
    }

    rpmsg_service_t *rdev = data;
    struct rpmsg_instance *rpmsg_dev = rpmsg_devs[rdev->rpmsg_type][rdev->remote_proc];
    struct rpmsg_endpoint* endpoint = NULL;

    if (!rpmsg_dev) {
        ssdk_printf(SSDK_EMERG, "error for rpmsg_devs[%d][%d] is null\n", rdev->rpmsg_type, rdev->remote_proc);
        goto exit;
    }

    if (!rdev->is_master && rdev->rpmsg_type == RPMSG_TYPE_VIRTIO) {
        rpmsg_lite_remote_reinit(rpmsg_dev);
    }

    if (!rpmsg_device_wait_link_up(rdev)) {
        ssdk_printf(SSDK_EMERG, "error for rpmsg device reset to work, link up error\n");
        goto exit;
    }

    if (rdev->rpmsg_type == RPMSG_TYPE_IPCC) {
        list_for_every_entry(&rpmsg_dev->endpoints, endpoint, struct rpmsg_endpoint, node) {
            rpmsg_channel_t* chan = (rpmsg_channel_t *)endpoint->priv;
            if (chan != NULL) {
                rpmsg_channel_set_announced(chan, false);
                ssdk_printf(SSDK_EMERG, "clear announced of channel: %s for reconnect communication\n"
                            , rpmsg_channel_name(chan));
            }
        }
    } else {
        list_for_every_entry (&rpmsg_dev->endpoints, endpoint,
                              struct rpmsg_endpoint, node) {
            rpmsg_channel_t *chan = (rpmsg_channel_t *)endpoint->priv;
            if (chan != NULL) {
                rpmsg_channel_reset(chan);
                ssdk_printf(SSDK_EMERG, "clear announced of channel: %s for reconnect communication\n"
                            , rpmsg_channel_name(chan));
            }
        }
    }

exit:
    osThreadExit();
}

int32_t rpmsg_reset_signal_cb(int32_t sig, void* args)
{
    int32_t rproc = -1;
    char name_buf[32];

    switch (sig) {
        case WDT6____ovflow_int:
            rproc = MBOX_RPROC_AP2;
            break;
        case WDT5____ovflow_int:
            rproc = MBOX_RPROC_AP1;
            break;
        default:
            break;
    }

    for (int32_t i = 0; i < RPMSG_SERVICES_NUM; i++) {
        if (g_rpmsg_services[i].remote_proc == rproc) {
            ssdk_printf(SSDK_EMERG, "rpmsg ins for rproc: %d, type: %d observe reboot\n", rproc, g_rpmsg_services[i].rpmsg_type);
            sprintf(name_buf, "rpmsg_reset-%d-%d", rproc, g_rpmsg_services[i].rpmsg_type);
            osThreadAttr_t attr = {
                .name = name_buf,
                .priority = 9,
            };
            osThreadNew(rpmsg_device_reset_work, &g_rpmsg_services[i], &attr);
        }
    }

    return 0;
}

static void rpmsg_rproc_reset_observer(int32_t rproc)
{
    static bool rproc_flg[MBOX_RPROC_VDSP];
    static uint32_t rproc_nums = 0;

    rproc_flg[rproc] = true;
    if (++rproc_nums < RPMSG_SERVICES_NUM) {
        return;
    }

    if (rproc_flg[MBOX_RPROC_AP1] == true) {
#ifdef CONFIG_OBSERVE_AP1
        // register ap1 reboot handle
#ifdef CONFIG_SYS_DIAGNOSIS
        sysd_register_daemon_handler(rpmsg_reset_signal_cb, NULL, WDT5____ovflow_int);
#else  /* !CONFIG_SYS_DIAGNOSIS */
        ap_observer_reister_cb(rpmsg_reset_signal_cb, NULL, WDT5____ovflow_int);
#endif /* end CONFIG_SYS_DIAGNOSIS */
#endif /* end CONFIG_OBSERVE_AP1 */
    }

    if (rproc_flg[MBOX_RPROC_AP2] == true) {
#ifdef CONFIG_OBSERVE_AP2
        // register ap2 reboot handle
#ifdef CONFIG_SYS_DIAGNOSIS
        sysd_register_daemon_handler(rpmsg_reset_signal_cb, NULL, WDT6____ovflow_int);
#else  /* !CONFIG_SYS_DIAGNOSIS */
        ap_observer_reister_cb(rpmsg_reset_signal_cb, NULL, WDT6____ovflow_int);
#endif /* end CONFIG_SYS_DIAGNOSIS */
#endif /* end CONFIG_OBSERVE_AP2 */
    }
}
#endif /* end defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2) */

static void rpmsg_service_init_thread(void *arg)
{
    rpmsg_service_t *service = arg;
    struct rpmsg_instance *rpmsg_dev;
    int ret = 0;

    if (service == NULL) {
        goto exit;
    }

    if (service->rpmsg_type == RPMSG_TYPE_VIRTIO) {
        /* wait to death if ddr not ready */
        while (!sdrv_common_reg_get_value(SDRV_REG_STATUS,
                    SDRV_REG_STATUS_DDR_INIT_DONE))
            osDelay(10 * 1000);
    }

    rpmsg_dev = osAlloc(sizeof(struct rpmsg_instance));
    if (!rpmsg_dev) {
        ssdk_printf(SSDK_ERR, "rpmsg_dev malloc fail.\n");
        goto exit;
    }

    memset(rpmsg_dev, 0x00, sizeof(struct rpmsg_instance));
    rpmsg_devs[service->rpmsg_type][service->remote_proc] = rpmsg_dev;

    if (service->rpmsg_type == RPMSG_TYPE_VIRTIO) {
        struct rpmsg_lite_instance *rpmsg_virtio_dev = &rpmsg_virtio_devs[service->remote_proc];
        ret = rpmsg_virtio_init(service, rpmsg_dev, rpmsg_virtio_dev);
    }
    else if (service->rpmsg_type == RPMSG_TYPE_IPCC) {
        struct rpmsg_ipcc_instance *rpmsg_ipcc_dev = &rpmsg_ipcc_devs[service->remote_proc];
        ret = rpmsg_ipcc_init(service, rpmsg_dev, rpmsg_ipcc_dev);
    }
    else {
        ret = -1;
        ssdk_printf(SSDK_ERR, "rpmsg type error:%d.\n", service->rpmsg_type);
    }

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "rpmsg_service_init error.\n");
        rpmsg_devs[service->rpmsg_type][service->remote_proc] = NULL;
        osFree(rpmsg_dev);
        goto exit;
    }

#ifdef CONFIG_RPMSG_WITH_LK
    int32_t retry = 200;
    while (!rpmsg_is_link_up(rpmsg_dev)) {
        if (retry-- == 0) {
            ssdk_printf(SSDK_ERR, "rpmsg_service_init wait remote[%d:%d] failed.\n",
                service->rpmsg_type, service->remote_proc);
            rpmsg_deinit(rpmsg_dev);
            rpmsg_devs[service->rpmsg_type][service->remote_proc] = NULL;
            osFree(rpmsg_dev);
            goto exit;
        }

        if (!service->is_master) {
            rpmsg_kick_remote(rpmsg_dev);
        }

        osDelay(200 * osKernelGetTickFreq() / 1000);
    }
#endif /* CONFIG_RPMSG_WITH_LK */

    rpmsg_service_echo_init(rpmsg_dev, service->rpmsg_type);

#ifdef CONFIG_USE_IPCC_RPC
    dcf_rpc_service_init(service);
#endif

#if defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2)
    rpmsg_rproc_reset_observer(service->remote_proc);
#endif /* end defined(CONFIG_OBSERVE_AP1) || defined(CONFIG_OPSERVE_AP2) */

exit:
    osThreadExit();
}

/**
 * @brief rpmsg service init.
 *
 * @return result.
 */
int rpmsg_service_init(uint8_t number, rpmsg_service_t *services)
{
    uint8_t cnt;
    int ret = 0;
    osThreadId_t tid;

    for (cnt = 0; cnt < number; cnt++) {
        rpmsg_service_t *service = &services[cnt];
        osThreadAttr_t attr = {
            .name = "rpmsg_init",
            .priority = 28,   /* 高于 CAN 收报(26): 保证 echo 通道建立/链路处理及时 */
        };
        tid = osThreadNew(rpmsg_service_init_thread,
                          (void *)service, &attr);
        ASSERT(tid);
    }

    return ret;
}

/**
 * @brief rpmsg service deinit.
 *
 */
void rpmsg_service_deinit(void)
{
    uint8_t i, j;

#ifdef CONFIG_USE_IPCC_RPC
    // deinit dcf rpc service
    dcf_rpc_service_deinit();
#endif /* end CONFIG_USE_IPCC_RPC */

    for (i = 0; i < CONFIG_RPMSG_RPROC_NUM; i++) {
        for (j = 0; j < RPMSG_TYPE_NUM; j++) {
            if (rpmsg_devs[j][i] != NULL) {
                osFree((void *)rpmsg_devs[j][i]);
                rpmsg_devs[j][i] = NULL;
            }
        }
    }
}

/**
 * @brief get rpmsg dev.
 *
 * @param[in] rpmsg_type rpmsg type.
 * @param[in] rproc remote rproc.
 *
 * @return init result.
 */
struct rpmsg_instance *rpmsg_get_instance(uint8_t rpmsg_type, uint8_t rproc)
{
    if ((rpmsg_type >= RPMSG_TYPE_NUM) || (rproc >= CONFIG_RPMSG_RPROC_NUM)) {
        return NULL;
    }

    return rpmsg_devs[rpmsg_type][rproc];
}
