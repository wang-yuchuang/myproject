/**
 * @file container.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv container library.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_CONTAINER_H__
#define __SD_CONTAINER_H__
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <lib/list.h>
#include <cmsis_os2.h>
#include "app_log.h"

#ifndef LOG_TAG
#define LOG_TAG "container"
#endif

typedef struct _container_t container_t;
typedef struct _issue_token_t issue_token_t;
typedef struct _msg_token_t msg_token_t;
typedef struct _issue_wait_async_t issue_wait_async_t;

struct _container_t {
    int id;
    void* content;
    unsigned long size;
    struct list_node node;
};

static inline container_t* container_new(int id, unsigned long size)
{
    container_t* cont = (container_t*)malloc(sizeof(container_t));
    if (cont) {
        cont->id = id;
        cont->size = 0;
        if (size > 0) {
            cont->content = (void*)malloc(size);
            memset(cont->content, 0, size);
            cont->size = size;
        }
    }
    return cont;
}

static inline void container_delete(container_t** cont)
{
    if (*cont) {
        if ((*cont)->content) {
            free((*cont)->content);
            (*cont)->content = NULL;
            (*cont)->size = 0;
        }
        free((*cont));
        (*cont) = NULL;
    }
}

static inline int container_carry_on(container_t* cont, void* content)
{
    if (cont && cont->content && cont->size > 0 && content) {
        memcpy(cont->content, content, cont->size);
        return 0;
    }
    return -1;
}

static inline int container_carry_down(container_t* cont, void* content)
{
    if (cont && cont->content && cont->size > 0 && content) {
        memcpy(content, cont->content, cont->size);
        return 0;
    }
    return -1;
}

struct _issue_token_t {
    const char* magic; /* supplier/consumer */
    osEventFlagsId_t dereference; /* reference means all issue are referenced */
    uint32_t flags;
    uint32_t supplier_num;
    uint32_t consumer_num;
    uint32_t issue_num;
    osMessageQueueId_t issue_q;
    struct list_node issue_list;
    struct _issue_token_t* supplier;
    struct list_node consumer_list;
    struct list_node node;
};

struct _issue_wait_async_t {
    issue_token_t* issue;
    container_t* container;
};

static inline issue_token_t* issue_token_new(const char* magic)
{
    issue_token_t* issue = (issue_token_t*)malloc(sizeof(issue_token_t));
    if (issue) {
        issue->magic = magic;
        issue->dereference = NULL;
        issue->flags = 0;
        issue->supplier_num = 0;
        issue->consumer_num = 0;
        issue->issue_num = 0;
        issue->issue_q = NULL;
        issue->supplier = NULL;
        list_initialize(&issue->issue_list);
        list_initialize(&issue->consumer_list);
        list_clear_node(&issue->node);
    }
    return issue;
}

static inline void issue_token_delete(issue_token_t** issue)
{
    if (*issue) {
        free(*issue);
        *issue = NULL;
    }
}

static inline int issue_token_init(issue_token_t* issue, uint32_t issue_num, unsigned long cont_size, int auto_derefence)
{
    if (issue && issue->magic) {
        if (strstr(issue->magic, "supp")) {
            // TODO:
        }
        if (strstr(issue->magic, "cons")) {
            APP_DBG("issue->magic:%s, issue_num:%d", issue->magic, issue_num);
            issue->dereference = osEventFlagsNew(NULL);
            issue->issue_q = osMessageQueueNew(issue_num, sizeof(container_t*), NULL);
            issue->issue_num = issue_num;
            osEventFlagsClear(issue->dereference, 0x00ffffff); /* only low 24bits valid */
            for (int i = 0; i < issue_num; i++) {
                container_t* container = container_new(i, cont_size);
                if (container) {
                    APP_DBG("create container and insert to list tail");
                    list_add_tail(&issue->issue_list, &container->node);
                    issue->flags |= (1<<i);
                    if (auto_derefence) {
                        osEventFlagsSet(issue->dereference, (1<<i));
                        APP_DBG("%s:%x", issue->magic, osEventFlagsGet(issue->dereference));
                    }
                }
            }
        }
        return 0;
    }
    return -1;
}

static inline void issue_token_deinit(issue_token_t* issue)
{
    if (issue) {
        osEventFlagsDelete(issue->dereference);
        osMessageQueueReset(issue->issue_q);
        osMessageQueueDelete(issue->issue_q);
        issue->issue_q = NULL;
        issue->issue_num = 0;
        container_t* container = NULL;
        while (NULL != (container = list_peek_head_type(&issue->issue_list, container_t, node))) {
            list_delete(&container->node);
            container_delete(&container);
            container = NULL;
        }
    }
}

static inline void issue_token_serialization(int tk_num, ...)
{
    if (tk_num < 2) return;
    va_list args;
    va_start(args, tk_num);
    issue_token_t* supplier = va_arg(args, issue_token_t*);
    APP_DBG("supplier->magic:%s", supplier->magic);
    if (supplier) {
        for (int i = 0; i < tk_num-1; i++) {
            issue_token_t* consumer = va_arg(args, issue_token_t*);
            if (consumer) {
                APP_DBG("consumer->magic:%s", consumer->magic);
                list_add_tail(&supplier->consumer_list, &consumer->node);
                supplier->consumer_num++;
                consumer->supplier = supplier;
            }
        }
    }
    va_end(args);
}

static inline int issue_token_send_async(issue_token_t* issue, void* content, uint32_t timeout)
{
    if (issue) {
        int send_cnt = 0;
        issue_token_t* consumer = list_peek_head_type(&issue->consumer_list, issue_token_t, node);
        while (consumer) {
            container_t* container = list_peek_head_type(&consumer->issue_list, container_t, node);
            if (container) {
                uint32_t dereferenced = osEventFlagsWait(consumer->dereference, (1<<container->id), osFlagsNoClear, 1000);
                APP_DBG("dereference:0x%x", dereferenced);
                if (dereferenced&(1<<container->id)) {
                    osEventFlagsClear(consumer->dereference, (1<<container->id));
                    list_delete(&container->node);
                    list_add_tail(&consumer->issue_list, &container->node);
                    container_carry_on(container, content);
                    osStatus_t ret = osMessageQueuePut(consumer->issue_q, &container, 0, 1000);
                    APP_DBG("ret:%d", ret);
                    if (ret == osOK) {
                        send_cnt++;
                    }
                }
            }
            consumer = list_next_type(&issue->consumer_list, &consumer->node, issue_token_t, node);
        }
        return send_cnt == issue->consumer_num ? 0 : -1;
    }
    return -1;
}

static inline issue_wait_async_t issue_token_send_one_async(issue_token_t* issue, const char* consumer_magic, void* content, uint32_t timeout)
{
    if (issue && consumer_magic) {
        issue_token_t* consumer = list_peek_head_type(&issue->consumer_list, issue_token_t, node);
        while (consumer) {
            if (strstr(consumer->magic, consumer_magic))
                break;
            consumer = list_next_type(&issue->consumer_list, &consumer->node, issue_token_t, node);
        }
        if (consumer) {
            container_t* container = list_peek_head_type(&consumer->issue_list, container_t, node);
            if (container) {
                uint32_t dereferenced = osEventFlagsWait(consumer->dereference, (1<<container->id), osFlagsNoClear, 1000);
                APP_DBG("container:%p, id:%d, dereferenced:%p, %x", container, container->id, consumer->dereference, dereferenced);
                if (dereferenced&(1<<container->id)) {
                    osEventFlagsClear(consumer->dereference, (1<<container->id));
                    list_delete(&container->node);
                    list_add_tail(&consumer->issue_list, &container->node);
                    container_carry_on(container, content);
                    osStatus_t ret = osMessageQueuePut(consumer->issue_q, &container, 0, 1000);
                    APP_DBG("ret:%d, dereference:%p, 0x%x", ret, consumer->dereference, osEventFlagsGet(consumer->dereference));
                    return ret == osOK ? (issue_wait_async_t){ consumer, container } : (issue_wait_async_t){ NULL, NULL };
                }
            }
        }
    }
    return (issue_wait_async_t){ NULL, NULL };
}

static inline int issue_token_send_sync(issue_token_t* issue, void* content, int* wait_condition)
{
    if (issue && wait_condition) {
        int i = 0;
        int send_success = 0;

        /* send */
        issue_token_t* consumer = list_peek_head_type(&issue->consumer_list, issue_token_t, node);
        APP_DBG("consumer:%p", consumer);
        while (consumer) {
            container_t* container = list_peek_head_type(&consumer->issue_list, container_t, node);
            if (container) {
                uint32_t dereferenced = osEventFlagsWait(consumer->dereference, (1<<container->id), osFlagsNoClear, 1000);
                APP_DBG("dereference:0x%x", dereferenced);
                if (dereferenced&(1<<container->id)) {
                    osEventFlagsClear(consumer->dereference, (1<<container->id));
                    list_delete(&container->node);
                    list_add_tail(&consumer->issue_list, &container->node);
                    container_carry_on(container, content);
                    osStatus_t ret = osMessageQueuePut(consumer->issue_q, &container, 0, 1000);
                    APP_DBG("ret:%d", ret);
                    if (ret == osOK) {
                        send_success |= (1<<i);
                    }
                }
            }
            consumer = list_next_type(&issue->consumer_list, &consumer->node, issue_token_t, node);
            i++;
        }

        /* wait dereference */
        i = 0;
        consumer = list_peek_head_type(&issue->consumer_list, issue_token_t, node);
        while (*wait_condition && send_success) {
            APP_DBG("wait dereference:%x", send_success);
            if (!consumer) {
                i = 0;
                consumer = list_peek_head_type(&issue->consumer_list, issue_token_t, node);
                if (!consumer) {
                    return -1;
                }
            }
            if (send_success&(1<<i)) {
                container_t* container = list_peek_tail_type(&consumer->issue_list, container_t, node);
                APP_DBG("container:%p", container);
                if (container) {
                    uint32_t dereferenced = osEventFlagsWait(consumer->dereference, (1<<container->id), osFlagsNoClear, 10);
                    APP_DBG("dereferenced:%d", dereferenced);
                    if (dereferenced&(1<<container->id)) {
                        send_success &= ~(1<<i);
                    }
                }
            }
            consumer = list_next_type(&issue->consumer_list, &consumer->node, issue_token_t, node);
            i++;
        }
        return 0;
    }
    return -1;
}

static inline int issue_token_wait_one_dereferenced(issue_wait_async_t* wait_issue, int* wait_condition)
{
    if (wait_issue && wait_issue->issue && wait_issue->container && wait_condition) {
        while (*wait_condition) {
            uint32_t dereferenced = osEventFlagsWait(wait_issue->issue->dereference, (1<<(wait_issue->container->id)), osFlagsNoClear, 10);
            if (dereferenced&(1<<(wait_issue->container->id)))
                break;
        }
        return 0;
    }
    return -1;
}

static inline container_t* issue_token_receive(issue_token_t* issue, uint32_t timeout)
{
    if (issue && issue->issue_q) {
        container_t* container = NULL;
        osStatus_t ret = osMessageQueueGet(issue->issue_q, &container, 0, timeout);
        APP_DBG("ret:%d", ret);
        return container;
    }
    return NULL;
}

static inline void issue_token_dereference(issue_token_t* issue, container_t* container)
{
    if (issue && container) {
        uint32_t dereference = osEventFlagsSet(issue->dereference, (1<<container->id));
        APP_DBG("dereference:%p, %x, id:%d", issue->dereference, dereference, container->id);
    }
}

static inline int issue_token_is_dereference(issue_token_t* issue, container_t* container)
{
    if (issue && container) {
        uint32_t dereference = osEventFlagsGet(issue->dereference);
        return (dereference&(1<<container->id)) ? 0 : -1;
    }
    return -1;
}

static inline void issue_token_clear(issue_token_t* issue)
{
    if (issue) {
        if (issue->dereference) {
            osEventFlagsClear(issue->dereference, 0x00ffffff); /* only low 24bits valid */
        }
        if (strnstr(issue->magic, "cons", strlen("cons"))) {
            struct list_node* phead = list_peek_head(&issue->issue_list);
            struct list_node* pnode = phead->next;
            while (phead != pnode) {
                container_t* container = containerof(pnode, container_t, node);
                if (container && container->content && container->size > 0) {
                    memset(container->content, 0, container->size);
                }
                pnode = pnode->next;
            }
        }
        if (issue->issue_q) {
            osMessageQueueReset(issue->issue_q);
        }
        if (issue->dereference) {
            for (int i = 0; i < issue->issue_num; i++) {
                osEventFlagsSet(issue->dereference, (1<<i));
            }
        }
    }
}

struct _msg_token_t {
    const char* magic;
    uint32_t msg_num;
    osMessageQueueId_t msg_q;
    struct list_node empty_list;
    struct list_node used_list;
};

static inline msg_token_t* msg_token_new(const char* magic, uint32_t num, unsigned long size)
{
    msg_token_t* msg_tk = (msg_token_t*)malloc(sizeof(msg_token_t));
    if (msg_tk) {
        msg_tk->magic = magic;
        msg_tk->msg_q = osMessageQueueNew(num, sizeof(void*), NULL);
        list_initialize(&msg_tk->empty_list);
        list_initialize(&msg_tk->used_list);
        for (int i = 0; i < num; i++) {
            container_t* container = container_new(i, size);
            if (container) {
                list_add_tail(&msg_tk->empty_list, &container->node);
            }
        }
    }
    return msg_tk;
}

static inline void msg_token_delete(msg_token_t** msg_tk)
{
    msg_token_t* tk = *msg_tk;
    if (tk) {
        osMessageQueueReset(tk->msg_q);
        osMessageQueueDelete(tk->msg_q);
        tk->msg_q = NULL;
        container_t* container = NULL;
        while (NULL != (container = list_peek_head_type(&tk->empty_list, container_t, node))) {
            list_delete(&container->node);
            container_delete(&container);
            container = NULL;
        }
        while (NULL != (container = list_peek_head_type(&tk->used_list, container_t, node))) {
            list_delete(&container->node);
            container_delete(&container);
            container = NULL;
        }
        free(tk);
        *msg_tk = NULL;
    }
}

static inline int msg_token_send(msg_token_t* msg_tk, void* content, uint32_t timeout)
{
    container_t* container = NULL;
    if (msg_tk && content) {
        container = list_peek_head_type(&msg_tk->empty_list, container_t, node);
        if (!container) { /* all containers were referenced */
            // TODO:
            // wait dereference?
        }
        else {
            list_delete(&container->node);
            container_carry_on(container, content);
            list_add_tail(&msg_tk->used_list, &container->node);
            return osMessageQueuePut(msg_tk->msg_q, &container, 0, timeout) == osOK ? 0 : -1;
        }
    }
    return -1;
}

static inline int msg_token_recv(msg_token_t* msg_tk, void* content, uint32_t timeout)
{
    container_t* container = NULL;
    if (msg_tk && content) {
        if (osOK == osMessageQueueGet(msg_tk->msg_q, &container, 0, timeout)) {
            list_delete(&container->node);
            container_carry_down(container, content);
            list_add_tail(&msg_tk->empty_list, &container->node);
            return 0;
        }
    }
    return -1;
}
#endif /* __SD_CONTAINER_H__ */
/* EOF */
