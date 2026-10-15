/*
 * sd_errcode_net.c
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * transfer errcode to remote server
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <sys/socket.h>
#include "sd_errcode_net.h"
#include "errcode_cfg.h"

#define ERRCODE_MAX_QUEUE_COUNT 100

typedef struct {
    uint8_t *data;
    uint16_t length;
} mesg_desc_t;

struct errcode_net_context {
    int sockfd;
    osMutexId_t mutex;
    osMessageQueueId_t queue;
    osThreadId_t send_thread;
    struct sockaddr_in addr;
    char ip[REMOTE_SERVER_IP_MAX];
};

static struct errcode_net_context g_service;

bool is_remote_server_changed(const char* ip)
{
    struct errcode_net_context *cxt = &g_service;

    return (ip && strcmp(ip, cxt->ip));
}

bool is_need_create_socket(void)
{
    struct errcode_net_context *cxt = &g_service;

    return (cxt->sockfd < 0);
}

int errcode_open_socket(const char* ip, uint16_t port)
{
    struct errcode_net_context *cxt = &g_service;
    int ret = ERRCODE_OK;
    int local_sock = -1;
    struct sockaddr_in addr;
    ip4_addr_t remote_ip;
    uint16_t remote_port = htons(REMOTE_SERVER_PORT);
    char ip_addr_str[REMOTE_SERVER_IP_MAX];

    if (ip) {
        strcpy(ip_addr_str, ip);
    }
    else {
        snprintf(ip_addr_str, sizeof(ip_addr_str), "%s", REMOTE_SERVER_IP);
    }

    if (port) {
        remote_port = port;
    }

    inet_aton(ip_addr_str, &remote_ip);
    addr.sin_len = sizeof(addr);
    addr.sin_family = PF_INET;
    addr.sin_port = remote_port;

    inet_addr_from_ip4addr(&addr.sin_addr, &remote_ip);

    printf("errcode_open_socket start Remote ip = %s\n", ip_addr_str);

    if ((local_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("fail to start socket!\n");
        return ERRCODE_NG;
    }
    cxt->sockfd = local_sock;
    printf("errcode_open_socket sockfd = %d\n", cxt->sockfd);
    memcpy(&cxt->addr, &addr, sizeof(struct sockaddr_in));
    memcpy(cxt->ip, ip_addr_str, sizeof(ip_addr_str));

    return ret;
}

int errcode_close_socket(void)
{
    struct errcode_net_context *cxt = &g_service;
    printf("errcode_close_socket start!\n");
    if (cxt->sockfd >= 0) {
        close(cxt->sockfd);
    }
    cxt->sockfd = -1;

    return ERRCODE_OK;
}

int errcode_net_init(void)
{
    int ret = 0;
    struct errcode_net_context *cxt = &g_service;

    memset(cxt, 0, sizeof(*cxt));

#if ERRCODE_NET_SEND_ASYNC
    cxt->mutex = osMutexNew(NULL);
    if (cxt->mutex == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create mutex failed.\r\n", __func__);
         return ERRCODE_NG;
    }

    cxt->queue = osMessageQueueNew(ERRCODE_MAX_QUEUE_COUNT, sizeof(mesg_desc_t), NULL);
    if (cxt->queue == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create queue failed.\r\n", __func__);
        return ERRCODE_NG;
    }

    errcode_net_send_thread(cxt);
#endif
    ret = errcode_open_socket(NULL, 0);
    if (ret != 0) {
        printf("%s: errcode_open_socket failed.\n", __func__);
    }

    return ret;
}

int errcode_net_deinit(void)
{
    struct errcode_net_context *cxt = &g_service;

    errcode_close_socket();
#if ERRCODE_NET_SEND_ASYNC
    if (cxt->queue) {
        osMessageQueueDelete(cxt->queue);
    }

    if (cxt->mutex) {
        osMutexDelete(cxt->mutex);
    }

    if (cxt->send_thread) {
        osThreadTerminate(cxt->send_thread);
    }
#endif

    return ERRCODE_OK;
}

#if ERRCODE_NET_SEND_ASYNC
static void errcode_send_async_thread(void *arg) {
    struct errcode_net_context *cxt = arg;
    mesg_desc_t message;
    ssize_t count;
    osStatus_t ret;

    for (;;) {
        ret = osMessageQueueGet(cxt->queue, &message, NULL, osWaitForever);
        if (ret != osOK) {
            ssdk_printf(SSDK_WARNING, "%s: message queue get failed.\n", __func__);
            printf("message queue get failed.\n");
            continue;
        }

        {
            ERRCODE_DEBUG_PRINT("errcode_send_async_thread length = %d\n", message.length);
            ERRCODE_DEBUG_PRINT("[");
            for (int i = 0; i < message.length; i++) {
                ERRCODE_DEBUG_PRINT(" %02x", message.data[i]);
            }
            ERRCODE_DEBUG_PRINT("]\n");
        }

        if (cxt->sockfd < 0) {
            printf("Invalid sockfd = %d, Reopen socket\n", cxt->sockfd);
            ret = errcode_open_socket(NULL, 0);
            if (ret == -1) {
                printf("Reopen socket failed!\n");
                continue;
            }
        }
        count = sendto(cxt->sockfd, message.data, message.length, 0, (struct sockaddr *)&cxt->addr, sizeof(cxt->addr));
        if (count != message.length) {
            printf("fail to send socket:%d(%s) count = %d, length = %d\n", errno, strerror(errno), (int)count, message.length);
        }

        free(message.data);
    }

    return;
}

static void errcode_net_send_thread(struct errcode_net_context *cxt) {
    osThreadAttr_t errcode_net_task_attr = {
        .name = "errcode_send_task",
        .stack_size = 4096,
        .priority = osPriorityNormal2,
    };

    cxt->send_thread = osThreadNew(errcode_send_async_thread, (void *)cxt, &errcode_net_task_attr);
    if (!cxt->send_thread) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        osMutexDelete(cxt->mutex);
        osMessageQueueDelete(cxt->queue);
        return;
    }
}

int errcode_send_socket(uint8_t *data_buffer, uint16_t length)
{
    struct errcode_net_context *cxt = &g_service;
    osStatus_t ret;
    mesg_desc_t message;

    ERRCODE_DEBUG_PRINT("errcode_send_socket length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    message.data = malloc(length+1);
    memcpy(message.data, data_buffer, length);
    message.length = length;

    ret = osMessageQueuePut(cxt->queue, &message, 0, 0);
    if (ret != osOK) {
        printf("%s: message queue put failed.\r\n", __func__);
        return ERRCODE_NG;
    }

    {
        //Just for test
        int ret1 = osMessageQueueGetCount(cxt->queue);
        int ret2 = osMessageQueueGetCapacity(cxt->queue);
        int ret3 = osMessageQueueGetMsgSize(cxt->queue);
        int ret4 = osMessageQueueGetSpace(cxt->queue);
        printf("Count=%d, Capacity=%d, MsgSize=%d, Space=%d\n", ret1, ret2, ret3, ret4);
    }

    return ERRCODE_OK;
}

#else
int errcode_send_socket(uint8_t *data_buffer, uint16_t length)
{
    struct errcode_net_context *cxt = &g_service;
    int ret = ERRCODE_OK;
    ssize_t count;

    ERRCODE_DEBUG_PRINT("errcode_send_socket length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    if (cxt->sockfd < 0) {
        printf("Invalid sockfd = %d, Reopen socket\n", cxt->sockfd);
        ret = errcode_open_socket(NULL, 0);
        if (ret == ERRCODE_NG) {
            printf("Reopen socket failed!\n");
            return ret;
        }
    }

    count = sendto(cxt->sockfd, data_buffer, length, 0, (struct sockaddr *)&cxt->addr, sizeof(cxt->addr));
    ERRCODE_DEBUG_PRINT("%s %d count:%ld, length=%d\n", __func__, __LINE__, count, length);

    if (count != length) {
        printf("fail to send socket:%d(%s) count = %d, length = %d\n", errno, strerror(errno), (int)count, length);
        close(cxt->sockfd);
        cxt->sockfd = -1;

        return ERRCODE_NG;
    }

    return ret;
}

int errcode_set_to_net(const char* ip, uint8_t *data_buffer, uint16_t length)
{
    int ret = ERRCODE_OK;
    uint16_t port;
    int local_sock = -1;
    struct sockaddr_in addr;
    ip4_addr_t remote_ip;
    char ip_addr_str[32];
    ssize_t count;

    ERRCODE_DEBUG_PRINT("errcode_set_to_net length = %d\n", length);
    ERRCODE_DEBUG_PRINT("[");
    for (int i = 0; i < length; i++) {
        ERRCODE_DEBUG_PRINT(" %02x", data_buffer[i]);
    }
    ERRCODE_DEBUG_PRINT("]\n");

    if (ip) {
        strcpy(ip_addr_str, ip);
    }
    else {
        snprintf(ip_addr_str, sizeof(ip_addr_str), "%s", REMOTE_SERVER_IP);
    }

    port = htons(REMOTE_SERVER_PORT);
    inet_aton(ip_addr_str, &remote_ip);
    addr.sin_len = sizeof(addr);
    addr.sin_family = PF_INET;
    addr.sin_port = port;

    inet_addr_from_ip4addr(&addr.sin_addr, &remote_ip);

    ERRCODE_DEBUG_PRINT("errcode_set_to_net start Remote ip = %s\n", ip_addr_str);

    if ((local_sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        printf("fail to start socket! errno = %d\n", errno);
        return ERRCODE_NG;
    }

    count = sendto(local_sock, data_buffer, length, 0, (struct sockaddr *)&addr, sizeof(addr));
    ERRCODE_DEBUG_PRINT("%s %d count:%ld\n", __func__, __LINE__, count);

    if (count != length) {
        printf("fail to send socket:%d(%s) count = %d, length = %d\n", errno, strerror(errno), (int)count, length);
        close(local_sock);

        return ERRCODE_NG;
    }

    close(local_sock);

    return ret;
}
#endif
