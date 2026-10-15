#include <app.h>
#include <dw_pcie.h>
#include <event.h>
#include <spinlock.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <timer.h>
#include <ulink_service.h>

#define COPY_DIR_PSEUDO_EXIT -1

#define server_ip_ddf ULINK_HPL_DMA_COPY_SERVER_ADDR
#define server_port ULINK_HPL_DMA_COPY_SERVER_PORT

#define TASK_QUEUE_MAX_LENGTH 16

struct hpl_dma_copy_req {
    int id;
    struct ulink_copy_param param;
};

struct hpl_dma_copy_rst {
    int id;
    int retval;
};

struct task_desc {
    struct list_node list;
    struct hpl_dma_copy_req req;
    struct hpl_dma_copy_rst rst;
    int sock;
    int remote;
    int async;
    struct sockaddr_in addr;
    ulink_copy_callback callback;
    void *data;
    event_t done;
};

struct service_context {
    int sockfd;
    uint32_t server_ip;

    spin_lock_t task_lock;
    struct list_node idle_tasks;
    struct list_node pending_tasks;
    struct list_node async_tasks;
    struct task_desc raw_tasks[TASK_QUEUE_MAX_LENGTH];

    thread_t *worker_thread;
    event_t start_worker;
    thread_t *async_thread;
    event_t start_async;
};

static struct service_context g_service;

static inline int is_valid(struct ulink_copy_param *param) {
    return param->dir >= COPY_DIR_SSA_TO_SSB &&
           param->dir <= COPY_DIR_SSB_TO_SSA &&
           param->pri >= COPY_PRI_HIGHEST && param->pri <= COPY_PRI_LOWEST;
}

static struct task_desc *try_get_task_from_queue(struct service_context *cxt,
                                                 struct list_node *queue) {
    struct task_desc *task = NULL;

    spin_lock(&cxt->task_lock);
    if (list_length(queue)) {
        task = list_remove_head_type(queue, struct task_desc, list);
    }
    spin_unlock(&cxt->task_lock);

    return task;
}

static struct task_desc *find_task_with_id(struct service_context *cxt,
                                           struct list_node *queue, int id) {
    struct task_desc *task = NULL;
    int found = false;

    spin_lock(&cxt->task_lock);
    list_for_every_entry(queue, task, struct task_desc, list) {
        if (task->req.id == id) {
            found = true;
            break;
        }
    }
    if (found) {
        task->list.prev->next = task->list.next;
        task->list.next->prev = task->list.prev;
    } else {
        task = NULL;
    }
    spin_unlock(&cxt->task_lock);

    return task;
}

static void push_back_to_queue(struct service_context *cxt,
                               struct list_node *queue, struct task_desc *task,
                               bool ordered) {
    struct task_desc *cur;
    spin_lock(&cxt->task_lock);
    if (ordered) {
        list_for_every_entry(queue, cur, struct task_desc, list) {
            if (cur->req.param.pri > task->req.param.pri) {
                break;
            }
        }
        cur->list.prev->next = &task->list;
        task->list.prev = cur->list.prev;
        task->list.next = &cur->list;
        cur->list.prev = &task->list;
    } else {
        list_add_tail(queue, &task->list);
    }
    spin_unlock(&cxt->task_lock);
}

#if ULINK_HPL_DMA_COPY_SERVER

int ulink_copy(struct ulink_copy_param *param) {
    if (!param || !is_valid(param)) {
        return -1;
    }

    struct service_context *cxt = &g_service;
    struct task_desc *task;

    task = try_get_task_from_queue(cxt, &cxt->idle_tasks);
    if (!task) {
        return -EBUSY;
    }

    memcpy(&task->req.param, param, sizeof(*param));
    task->remote = 0;
    task->async = 0;

    push_back_to_queue(cxt, &cxt->pending_tasks, task, true);
    event_signal(&cxt->start_worker, true);

    event_wait(&task->done);

    int ret = task->rst.retval;

    push_back_to_queue(cxt, &cxt->idle_tasks, task, false);

    return ret;
}

int ulink_copy_async(struct ulink_copy_param *param, ulink_copy_callback cb,
                     void *data) {
    if (!param || !is_valid(param)) {
        return -1;
    }

    struct service_context *cxt = &g_service;
    struct task_desc *task;

    task = try_get_task_from_queue(cxt, &cxt->idle_tasks);
    if (!task) {
        return -EBUSY;
    }

    memcpy(&task->req.param, param, sizeof(*param));
    task->remote = 0;
    task->async = 1;
    task->callback = cb;
    task->data = data;

    push_back_to_queue(cxt, &cxt->pending_tasks, task, true);
    event_signal(&cxt->start_worker, true);

    return 0;
}

#else

int ulink_copy(struct ulink_copy_param *param) {
    if (!param || !is_valid(param)) {
        return -1;
    }

    struct service_context *cxt = &g_service;

    int fd;
    if ((fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        dprintf(CRITICAL, "fail to create socket: %d\n", errno);
        return -1;
    }

    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = cxt->server_ip;

    struct hpl_dma_copy_req req = {
        .id = -1,
        .param = *param,
    };

    ssize_t len = sendto(fd, &req, sizeof(req), 0,
                         (const struct sockaddr *)&addr, sizeof(addr));
    if (len == -1) {
        dprintf(CRITICAL, "fail to send: %d\n", errno);
        close(fd);
        return -1;
    }

    struct hpl_dma_copy_rst rst;
    len = recv(fd, &rst, sizeof(rst), 0);
    if (len == -1) {
        dprintf(CRITICAL, "fail to recv: %d\n", errno);
        close(fd);
        return -1;
    }

    close(fd);
    return rst.retval;
}

int ulink_copy_async(struct ulink_copy_param *param, ulink_copy_callback cb,
                     void *data) {
    if (!param || !is_valid(param) || !cb) {
        return -EINVAL;
    }

    struct service_context *cxt = &g_service;
    if (!cxt->async_thread) {
        return -ENODEV;
    }

    struct task_desc *task;
    task = try_get_task_from_queue(cxt, &cxt->idle_tasks);
    if (!task) {
        return -EBUSY;
    }

    task->callback = cb;
    task->data = data;

    memcpy(&task->req.param, param, sizeof(*param));

    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(server_port);
    addr.sin_addr.s_addr = cxt->server_ip;

    ssize_t len = sendto(cxt->sockfd, &task->req, sizeof(task->req), 0,
                         (const struct sockaddr *)&addr, sizeof(addr));
    if (len == -1) {
        dprintf(CRITICAL, "fail to send: %d\n", errno);
        push_back_to_queue(cxt, &cxt->idle_tasks, task, false);
        return -1;
    }

    push_back_to_queue(cxt, &cxt->async_tasks, task, false);

    return 0;
}

#endif

static void ulink_copy_init_task_items(struct service_context *cxt) {
    struct task_desc *task;

    spin_lock_init(&cxt->task_lock);
    list_initialize(&cxt->idle_tasks);
    list_initialize(&cxt->pending_tasks);
    list_initialize(&cxt->async_tasks);
    for (int i = 0; i < TASK_QUEUE_MAX_LENGTH; i++) {
        task = &cxt->raw_tasks[i];
        task->req.id = i;
        event_init(&task->done, false, EVENT_FLAG_AUTOUNSIGNAL);
        list_initialize(&task->list);
        list_add_tail(&cxt->idle_tasks, &task->list);
    }
}

static int ulink_copy_do_copy(struct ulink_copy_param *param) {
    struct kunlun_pcie pcie = {
#if CONFIG_ULINK_SIDE_A
        .dma_base = p2v(0x1c002000),
#elif CONFIG_ULINK_SIDE_B
        .dma_base = p2v(PCIE1_DBI_BASE + PCIE_DMA_OFFSET),
#endif
    };
#if CONFIG_ULINK_SIDE_A
    const int channel = 6;
#elif CONFIG_ULINK_SIDE_B
    const int channel = 7;
#endif
    int ret;

    if (param->dir == COPY_DIR_SSA_TO_SSB) {
        ret = kunlun_pcie_dma_read(&pcie, channel, param->dst, param->src,
                                   param->size);
    } else if (param->dir == COPY_DIR_SSB_TO_SSA) {
        ret = kunlun_pcie_dma_write(&pcie, channel, param->dst, param->src,
                                    param->size);
    } else {
        ret = -EINVAL;
    }

    return ret;
}

static int ulink_copy_async_thread(void *arg) {
    struct service_context *cxt = arg;
    struct task_desc *task;
    struct hpl_dma_copy_rst rst;
    ssize_t len;

    for (;;) {
#if ULINK_HPL_DMA_COPY_SERVER
        event_wait(&cxt->start_async);

        while ((task = try_get_task_from_queue(cxt, &cxt->async_tasks))) {
            task->callback(task->rst.retval, task->data);
            push_back_to_queue(cxt, &cxt->idle_tasks, task, false);
        }
#else
        len = recv(cxt->sockfd, &rst, sizeof(rst), 0);
        if (len == -1 && errno == EAGAIN)
            continue;

        if (len == -1) {
            dprintf(CRITICAL, "fail to recv: %d\n", errno);
            close(cxt->sockfd);
            cxt->sockfd = -1;
            break;
        }

        task = find_task_with_id(cxt, &cxt->async_tasks, rst.id);
        if (!task) {
            dprintf(CRITICAL, "no such task %d\n", rst.id);
            continue;
        }

        task->callback(rst.retval, task->data);
        push_back_to_queue(cxt, &cxt->idle_tasks, task, false);
#endif
    }

    dprintf(ALWAYS, "exiting async thread\n");

    return 0;
}

static void ulink_copy_start_async_thread(struct service_context *cxt) {
#if !ULINK_HPL_DMA_COPY_SERVER
    if ((cxt->sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        dprintf(CRITICAL, "fail to create socket: %d\n", errno);
        return;
    }

    struct sockaddr_in addr = {
        .sin_family = PF_INET,
        .sin_port = htons(server_port + 1),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    if (bind(cxt->sockfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
        dprintf(CRITICAL, "fail to bind port %d: %d\n", server_port, errno);
        close(cxt->sockfd);
        cxt->sockfd = -1;
        return;
    }

    struct timeval tv = {
        .tv_sec = 60,
        .tv_usec = 0,
    };
    if (setsockopt(cxt->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        dprintf(CRITICAL, "fail to set timeout: %d\n", errno);
        close(cxt->sockfd);
        cxt->sockfd = -1;
        return;
    }
#endif

    cxt->async_thread =
        thread_create("ulink-service-callback", ulink_copy_async_thread, cxt,
                      DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    if (!cxt->async_thread) {
        dprintf(CRITICAL, "fail to start async thread\n");
#if !ULINK_HPL_DMA_COPY_SERVER
        close(cxt->sockfd);
        cxt->sockfd = -1;
#endif
        return;
    }

    event_init(&cxt->start_async, false, EVENT_FLAG_AUTOUNSIGNAL);
    thread_detach_and_resume(cxt->async_thread);
}

#if ULINK_HPL_DMA_COPY_SERVER
static int ulink_copy_worker_thread(void *arg) {
    struct service_context *cxt = arg;
    struct task_desc *task;
    ssize_t len;
    int ret;

    for (;;) {
        event_wait(&cxt->start_worker);

        while ((task = try_get_task_from_queue(cxt, &cxt->pending_tasks))) {
            ret = ulink_copy_do_copy(&task->req.param);
            if (task->remote) {
                task->rst.retval = ret;
                len = sendto(cxt->sockfd, &task->rst, sizeof(task->rst), 0,
                             (const struct sockaddr *)&task->addr,
                             sizeof(task->addr));
                if (len < 0) {
                    dprintf(CRITICAL, "fail to send: %d ret %d\n", errno, len);
                }

                push_back_to_queue(cxt, &cxt->idle_tasks, task, false);
            } else if (task->async) {
                push_back_to_queue(cxt, &cxt->async_tasks, task, false);
                event_signal(&cxt->start_async, false);
            } else {
                event_signal(&task->done, false);
            }
        }
    }

    return 0;
}

static void ulink_copy_start_worker_thread(struct service_context *cxt) {
    cxt->worker_thread =
        thread_create("ulink-copy-worker", ulink_copy_worker_thread, cxt,
                      DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    if (!cxt->worker_thread) {
        dprintf(CRITICAL, "fail to start worker thread\n");
        return;
    }

    event_init(&cxt->start_worker, false, EVENT_FLAG_AUTOUNSIGNAL);
    thread_detach_and_resume(cxt->worker_thread);
}

static int ulink_copy_server_thread(void *arg) {
    struct service_context *cxt = arg;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    ssize_t len;
    struct hpl_dma_copy_req req;
    struct task_desc *task;

    dprintf(ALWAYS, "ulink copy service started at %d\n", server_port);

    for (;;) {
        memset(&addr, 0, sizeof(addr));
        len = recvfrom(cxt->sockfd, &req, sizeof(req), 0,
                       (struct sockaddr *)&addr, &addrlen);
        if (len == -1 && errno == EAGAIN)
            continue;

        if (len < 0) {
            dprintf(CRITICAL, "fail to recv: %d ret %d\n", errno, len);
            break;
        }

        if (req.param.dir == COPY_DIR_PSEUDO_EXIT) {
            dprintf(ALWAYS, "'exit' cmd received\n");
            break;
        }

        task = try_get_task_from_queue(cxt, &cxt->idle_tasks);
        if (task) {
            memcpy(&task->req.param, &req.param, sizeof(req.param));
            memcpy(&task->addr, &addr, sizeof(addr));
            task->rst.id = req.id;
            task->remote = 1;
            task->async = 0;
            push_back_to_queue(cxt, &cxt->pending_tasks, task, true);
            event_signal(&cxt->start_worker, false);
        } else {
            dprintf(CRITICAL, "no idle task\n");

            struct hpl_dma_copy_rst rst = {
                .id = req.id,
                .retval = -EBUSY,
            };

            len = sendto(cxt->sockfd, &rst, sizeof(rst), 0,
                         (const struct sockaddr *)&addr, addrlen);
            if (len < 0) {
                dprintf(CRITICAL, "fail to send: %d ret %d\n", errno, len);
                break;
            }
        }
    }

    dprintf(ALWAYS, "exiting ulink copy service...\n");

    return 0;
}

static void ulink_copy_start_server_thread(struct service_context *cxt) {
    if ((cxt->sockfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        dprintf(CRITICAL, "fail to create socket: %d\n", errno);
        return;
    }

    struct sockaddr_in addr = {
        .sin_family = PF_INET,
        .sin_port = htons(server_port),
        .sin_addr.s_addr = htonl(INADDR_ANY),
    };
    if (bind(cxt->sockfd, (const struct sockaddr *)&addr, sizeof(addr)) == -1) {
        dprintf(CRITICAL, "fail to bind port %d: %d\n", server_port, errno);
        close(cxt->sockfd);
        cxt->sockfd = -1;
        return;
    }

    struct timeval tv = {
        .tv_sec = 60,
        .tv_usec = 0,
    };
    if (setsockopt(cxt->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        dprintf(CRITICAL, "fail to set timeout: %d\n", errno);
        close(cxt->sockfd);
        cxt->sockfd = -1;
        return;
    }

    thread_t *thread = thread_create("ulink-service", ulink_copy_server_thread,
                                     cxt, DEFAULT_PRIORITY, DEFAULT_STACK_SIZE);
    if (!thread) {
        dprintf(CRITICAL, "fail to start server thread\n");
        close(cxt->sockfd);
        cxt->sockfd = -1;
        return;
    }

    thread_detach_and_resume(thread);
}
#endif

#if !ULINK_HPL_DMA_COPY_SERVER
static uint32_t parse_addr(const char *addr) {
    char ip[16];
    strncpy(ip, addr, 16);
    ip[15] = 0;

    /*
     * makefile converts "xxx.xxx.xxx.xxx" to "xxx_xxx_xxx_xxx" and we need to
     * convert it back
     */
    char *s = ip, *p;
    while ((p = strchr(s, '_'))) {
        *p = '.';
        s = p + 1;
    }

    uint32_t result;
    inet_pton(PF_INET, ip, &result);

    return result;
}
#endif

static void ulink_copy_service_entry(const struct app_descriptor *app,
                                     void *args) {
    struct service_context *cxt = &g_service;

    memset(cxt, 0, sizeof(*cxt));
#if !ULINK_HPL_DMA_COPY_SERVER
    cxt->server_ip = parse_addr(server_ip_ddf);
#endif
    ulink_copy_init_task_items(cxt);
    ulink_copy_start_async_thread(cxt);
#if ULINK_HPL_DMA_COPY_SERVER
    ulink_copy_start_worker_thread(cxt);
    ulink_copy_start_server_thread(cxt);
#endif
}

APP_START(ulink_copy_service)
    .entry = ulink_copy_service_entry,
APP_END
