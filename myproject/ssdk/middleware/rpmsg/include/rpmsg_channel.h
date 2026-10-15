#ifndef RPMSG_CHANNEL_H_
#define RPMSG_CHANNEL_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "rpmsg.h"

typedef int status_t;
typedef void *rpmsg_channel_t;
typedef int (*rpmsg_channel_cb)(void *data, int len, unsigned long src, void *arg);

rpmsg_channel_t rpmsg_channel_create(struct rpmsg_instance *dev, char *name, uint32_t addr);

int rpmsg_channel_reset(rpmsg_channel_t rpchn);

status_t rpmsg_channel_start(rpmsg_channel_t ch, rpmsg_channel_cb cb, void *cb_arg);

int rpmsg_channel_max_payload(rpmsg_channel_t ch);

status_t rpmsg_channel_destroy(rpmsg_channel_t ch);

status_t rpmsg_channel_send(rpmsg_channel_t ch, uint32_t dst_addr, void *data, int len, uint32_t timeout);

void *rpmsg_channel_alloc_send_buffer(rpmsg_channel_t ch, int *len, uint32_t timeout);

status_t rpmsg_channel_send_nocopy(rpmsg_channel_t ch, uint32_t dst_addr, void *data, int len);

status_t rpmsg_channel_recv(rpmsg_channel_t ch, void *buff, int buff_size, int *len, uint32_t timeout);

status_t rpmsg_channel_recvfrom(rpmsg_channel_t ch, uint32_t *src, void *buff, int buff_size, int *len, uint32_t timeout);

rpmsg_channel_t rpmsg_channel_from_endpoint(struct rpmsg_endpoint *ept);

status_t rpmsg_channel_announce_info(rpmsg_channel_t ch, bool *need_announce, bool *announced);

status_t rpmsg_channel_set_announced(rpmsg_channel_t ch, bool status);

char *rpmsg_channel_name(rpmsg_channel_t ch);

status_t rpmsg_channel_get_type_rproc(rpmsg_channel_t ch, uint32_t *type, uint32_t *rproc);

#if defined(__cplusplus)
}
#endif

#endif /* RPMSG_CHANNEL_H_ */
