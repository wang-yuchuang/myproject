#include <cmsis_os2.h>
#include <debug.h>
#include <errno.h>
#include <rpc/rpmsg_common.h>
#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <shm_protocol.h>
#include <shm_service.h>

#define SHM_HANDSHAKE_RETRY 10
#define SHM_HANDSHAKE_TIMEOUT 500
#define SHM_CMD_RETRY 5
#define SHM_CMD_TIMEOUT 200

typedef struct {
    rpmsg_channel_t chan;
    osMutexId_t lock;
    uint32_t seq;
    int hs_done;
} shm_client_t;

static int shm_client_process_msg_locked(shm_client_t *cl, shm_msg_t *req,
                                         shm_msg_t *reply, int timeout,
                                         int send_retry, int recv_retry) {
    uint32_t src;
    int seq, retry, len;
    int ret = 0;

    seq = cl->seq++;
    req->id = seq;

    retry = 0;
    do {
        ret = rpmsg_channel_send(cl->chan, SHM_EPT, (char *)req,
                                 sizeof(shm_msg_t), timeout);
    } while (ret < 0 && retry++ < send_retry);

    if (ret < 0 && retry == send_retry)
        return ret;

    retry = 0;

retry:
    if (retry++ == recv_retry)
        return ret;

    len = sizeof(shm_msg_t);
    ret = rpmsg_channel_recvfrom(cl->chan, &src, (char *)reply, sizeof(*reply),
                                 &len, timeout);
    if (ret)
        goto retry;

#if 0
    printf("[shm] %d bytes from %lu, magic %x cmd { %x, %x } id { %u, %u }\n",
           len, src, reply->magic, req->cmd, reply->cmd, req->id, reply->id);
#endif

    if (!shm_rx_check(reply, len, src) || (reply->cmd != req->cmd) ||
        (reply->id < req->id)) {
        ret = -EINVAL;
        goto retry;
    }

    return 0;
}

static int shm_client_process_msg(shm_client_t *cl, shm_msg_t *req,
                                  shm_msg_t *reply, int timeout, int send_retry,
                                  int recv_retry) {
    int ret = 0;

    osMutexAcquire(cl->lock, osWaitForever);

    if (!cl->hs_done) {
        shm_msg_t msg, hs;
        shm_msg_init(&msg, 0, SHM_CMD_HANDSHAKE);

        ret = shm_client_process_msg_locked(cl, &msg, &hs, 100, 1, 10);
        if (ret < 0) {
            ret = -ENODEV;
            goto release;
        }

        cl->hs_done = 1;
    }

    ret = shm_client_process_msg_locked(cl, req, reply, timeout, send_retry,
                                        recv_retry);

release:
    osMutexRelease(cl->lock);
    return ret;
}

static int shm_client_alloc_mem(shm_handle_t h, uint32_t align, uint32_t size,
                                uint16_t flags, uint16_t mode, const char *name,
                                uint64_t *addr) {
    shm_client_t *cl = h;
    shm_msg_t req, reply;
    int ret;

    shm_msg_init(&req, 0, SHM_CMD_ALLOC);
    req.align = align;
    req.size = size;
    req.flags = flags;
    req.mode = mode;
    if (name)
        strncpy(req.name, name, SHM_NAME_MAXLEN);

    ret = shm_client_process_msg(cl, &req, &reply, SHM_CMD_TIMEOUT,
                                 SHM_CMD_RETRY, SHM_CMD_RETRY);

    if (!ret) {
        ret = reply.retval;
        if (!ret)
            *addr = reply.addr;
    }

    return ret;
}

static void shm_client_free_mem(shm_handle_t h, uint64_t addr) {
    shm_client_t *cl = h;
    shm_msg_t req, reply;

    shm_msg_init(&req, 0, SHM_CMD_FREE);
    req.addr = addr;

    shm_client_process_msg(cl, &req, &reply, SHM_CMD_TIMEOUT, SHM_CMD_RETRY,
                           SHM_CMD_RETRY);
}

static int shm_client_find_mem(shm_handle_t h, const char *name,
                               uint64_t *addr) {
    shm_client_t *cl = h;
    shm_msg_t req, reply;
    int ret;

    shm_msg_init(&req, 0, SHM_CMD_FIND);
    if (name)
        strncpy(req.name, name, SHM_NAME_MAXLEN);

    ret = shm_client_process_msg(cl, &req, &reply, SHM_CMD_TIMEOUT,
                                 SHM_CMD_RETRY, SHM_CMD_RETRY);

    if (!ret) {
        ret = reply.retval;
        if (!ret)
            *addr = reply.addr;
    }

    return ret;
}

static int shm_client_begin_access(shm_handle_t h, uint64_t addr,
                                   uint32_t *size, char *name) {
    shm_client_t *cl = h;
    shm_msg_t req, reply;
    int ret;

    shm_msg_init(&req, 0, SHM_CMD_LOCK);
    req.addr = addr;

    ret = shm_client_process_msg(cl, &req, &reply, SHM_CMD_TIMEOUT,
                                 SHM_CMD_RETRY, SHM_CMD_RETRY);

    if (!ret) {
        ret = reply.retval;
        if (!ret) {
            *size = reply.size;
            if (name)
                strcpy(name, reply.name);
        }
    }

    return ret;
}

static void shm_client_end_access(shm_handle_t h, uint64_t addr) {
    shm_client_t *cl = h;
    shm_msg_t req, reply;

    shm_msg_init(&req, 0, SHM_CMD_UNLOCK);
    req.addr = addr;

    shm_client_process_msg(cl, &req, &reply, SHM_CMD_TIMEOUT, SHM_CMD_RETRY,
                           SHM_CMD_RETRY);
}

static int shm_client_init(shm_handle_t *h) {
    shm_client_t *cl;
    int ret = 0;

    if (!h)
        return -EINVAL;

    cl = malloc(sizeof(shm_client_t));
    if (!cl)
        return -ENOMEM;

    /* create share memory channel */
    struct rpmsg_instance *dev =
        rpmsg_get_instance(RPMSG_TYPE_IPCC, __SERVER_RPROC);
    if (!dev) {
        ssdk_printf(SSDK_CRIT, "[shm] no ipcc dev\n");
        ret = -ENODEV;
        goto fail1;
    }

    cl->chan = rpmsg_channel_create(dev, "shm", SHM_EPT);
    if (!cl->chan) {
        ssdk_printf(SSDK_CRIT, "[shm] ipcc chan fail\n");
        ret = -ENODEV;
        goto fail1;
    }

    rpmsg_channel_start(cl->chan, NULL, NULL);

    /* necessary initialization */
    cl->lock = osMutexNew(NULL);
    cl->seq = 0;
    cl->hs_done = 0;

    *h = cl;
    return 0;

fail1:
    free(cl);

    return ret;
}

static void shm_client_dump(shm_handle_t h) {
    shm_client_t *cl = h;

    printf("======= client info =======\n");
    printf("server rproc: %d, msg seq: %u\n", __SERVER_RPROC, cl->seq);
    printf("\n");
}

const shm_ops_t *shm_get_ops(void) {
    static const shm_ops_t __shm_ops = {
        .alloc_mem = shm_client_alloc_mem,
        .free_mem = shm_client_free_mem,
        .find_mem = shm_client_find_mem,
        .begin_access = shm_client_begin_access,
        .end_access = shm_client_end_access,
        .init = shm_client_init,
        .dump = shm_client_dump,
    };

    return &__shm_ops;
}
