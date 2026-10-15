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
 * Description: disp client data structure
 */
#ifndef __DISP_CLIENT_H
#define __DISP_CLIENT_H
#include <lib/list.h>
#include <cmsis_os2.h>

#define DISP_CLIENT_LOG_ERROR 3
#define DISP_CLIENT_LOG_DEBUG 7

struct client_msg_st {
    struct list_node node;

    display_handle handle;
    struct sdm_post_config *post;
};

struct disp_client {
    struct list_node node_head[SCREEN_MAX];
    osThreadId_t *client_thread;
    osMutexId_t m_lock[SCREEN_MAX];
};

void disp_client_init(void);
int disp_client_add(display_handle *handle, struct sdm_post_config *post);
int disp_client_clear(display_handle *handle);
#endif