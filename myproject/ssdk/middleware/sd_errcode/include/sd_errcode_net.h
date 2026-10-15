/*
 * sd_errcode_net.h
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * errcode to remote server head file
 *
 * Revision History:
 * -----------------
 */

#ifndef _SD_ERRCODE_NET_H_
#define _SD_ERRCODE_NET_H_

#include <sys/types.h>
#include "errcode.h"

#define ERRCODE_NET_SEND_ASYNC 0
#define REMOTE_SERVER_IP_MAX 16

int errcode_set_to_net(const char* ip, uint8_t *data_buffer, uint16_t length);
int errcode_open_socket(const char* ip, uint16_t port);
int errcode_close_socket(void);
int errcode_send_socket(uint8_t *data_buffer, uint16_t length);
bool is_remote_server_changed(const char* ip);
bool is_need_create_socket(void);

int errcode_net_init(void);
int errcode_net_deinit(void);
#endif //_SD_ERRCODE_NET_H_
