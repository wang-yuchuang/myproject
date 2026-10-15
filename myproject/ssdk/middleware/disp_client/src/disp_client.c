/*
 * SEMIDRIVE Copyright Statement
 * Copyright (c) SEMIDRIVE. All rights reserved
 *
 * This software and all rights therein are owned by SEMIDRIVE, and are
 * protected by copyright law and other relevant laws, regulations and
 * protection. Without SEMIDRIVE's prior written consent and/or related rights,
 * please do not use this software or any potion thereof in any form or by any
 * means. You may not reproduce, modify or distribute this software except in
 * compliance with the License. Unless required by applicable law or agreed to
 * in writing, software distributed under the License is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * You should have received a copy of the License along with this program.
 * If not, see <http://www.semidrive.com/licenses/>.
 *
 * Description: disp client source code
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <board.h>
#include "disp_hal.h"
#include "disp_client.h"

static struct disp_client *g_disp_client = NULL;

int disp_client_add(display_handle *handle, struct sdm_post_config *post)
{
    int ret = 0;
    int index = 0;
    struct disp_client *client = g_disp_client;
    struct list_node *node_head = NULL;

    struct client_msg_st *client_msg_old = NULL;
    struct client_msg_st *client_msg = malloc(sizeof(struct client_msg_st));
    if (!client_msg) {
        ssdk_printf(DISP_CLIENT_LOG_ERROR, "malloc client_msg err\n");
        return -1;
    }

    ssdk_printf(DISP_CLIENT_LOG_DEBUG, "add displayid:%d layeren:%d\n",
        handle->display_id, post->bufs->layer_en);

    index = handle->display_id;

    memcpy(&client_msg->handle, handle, sizeof(display_handle));
    client_msg->post = post;

    osMutexAcquire(client->m_lock[index], 0);
    node_head = &client->node_head[index];

    /* list isn't empty, clear list */
    while(!list_is_empty(node_head)) {
        client_msg_old = list_remove_head_type(node_head, struct client_msg_st, node);
        if (!client_msg_old) {
            continue;
        }

        client_msg_old->post->post_relase = true;
        free(client_msg_old);
    }

    list_add_head(node_head, &client_msg->node);
    osMutexRelease(client->m_lock[index]);

    return 0;
}

int disp_client_clear(display_handle *handle)
{
    struct disp_client *client = g_disp_client;
    struct list_node *node_head = NULL;
    struct client_msg_st *client_msg_old = NULL;
    int index = handle->display_id;

    node_head = &client->node_head[index];

    osMutexAcquire(client->m_lock[index], 0);
    while(!list_is_empty(node_head)) {
        client_msg_old = list_remove_head_type(node_head, struct client_msg_st, node);
        if (!client_msg_old) {
            continue;
        }

        client_msg_old->post->post_relase = true;

        free(client_msg_old);
    }
    osMutexRelease(client->m_lock[index]);

    return 0;
}

static void disp_client_post(struct disp_client *client)
{
    int ret;
    bool post_enable = false;
    struct client_msg_st *client_msg;
    struct list_node *node_head = NULL;

    for (int i = 0; i < SCREEN_MAX; i++) {
        node_head = &client->node_head[i];

        ret = list_is_empty(node_head);
        if (ret)
            continue;

        osMutexAcquire(client->m_lock[i], 0);
        client_msg = list_remove_head_type(node_head, struct client_msg_st, node);
        if (!client_msg) {
            osMutexRelease(client->m_lock[i]);
            continue;
        }
        osMutexRelease(client->m_lock[i]);
        hal_sdm_post_config(&client_msg->handle, client_msg->post);
        client_msg->post->post_relase = true;

        ssdk_printf(DISP_CLIENT_LOG_DEBUG, "post displayid:%d\n",
            client_msg->handle.display_id);

        free(client_msg);
    }
}

static bool is_need_post(struct disp_client *client)
{
    bool ret;

    for (int i = 0; i < SCREEN_MAX; i++) {
        ret = list_is_empty(&client->node_head[i]);
        if (!ret)
            return true;
    }

    return !ret;
}

static void disp_client_thread(void *arg)
{
    struct disp_client *client = arg;

    while (1)
    {
        if (is_need_post(client)) {
            disp_client_post(client);
        } else
            osDelay(1);
    }
}

void disp_client_init(void)
{
    struct disp_client *client = NULL;

    g_disp_client = malloc(sizeof(struct disp_client));
    if (!g_disp_client) {
        ssdk_printf(DISP_CLIENT_LOG_ERROR, "alloc g_disp_client err!\n");
        return;
    }

    client = g_disp_client;

    memset(client, 0, sizeof(struct disp_client));
    osThreadAttr_t dc_attr = {.name = "disp_client", .stack_size = 4096, .priority = osPriorityNormal};
    client->client_thread = osThreadNew(disp_client_thread, (void *)client, &dc_attr);
    if (!client->client_thread) {
        ssdk_printf(DISP_CLIENT_LOG_ERROR, "disp client thread create err\n");
        return;
    }

    for (int i = 0; i < SCREEN_MAX; i++) {
        list_initialize(&client->node_head[i]);
        client->m_lock[i] = osMutexNew(NULL);
    }

    ssdk_printf(DISP_CLIENT_LOG_DEBUG, "disp_client_init success!\n");
}