#ifndef __ICCMSG_HAL_H__
#define __ICCMSG_HAL_H__
#include "iccmsg_ll.h"
#include "iccmsg_dpr.h"
#include "iccmsg_queue.h"
#include <semaphore.h>

#define MAX_EVENTS                          256
#define DEFAULT_QUEUE_SIZE                  256
#define QUEUE_TIME_OUT_MS                   10


typedef struct {
    int msg_rx_cnt;
    int ipcc_rx_cnt;
    int virtio_rx_cnt;

    iccmsg_ll_t ll;
    int epoll_fd;
    struct epoll_event events[MAX_EVENTS];

    iccmsg_dpr_t dpr;

    pthread_t rx_thread;
    thread_safe_queue_t rx_que;

    pthread_t tx_thread;
    thread_safe_queue_t tx_que;
    sem_t ack_sem;

} iccmsg_hal_t;


iccmsg_hal_t *iccmsg_hal_init(int remote_ept, frame_conf_t fc, char *app_name);
int send_ack(int dev_fd,const frame_conf_t *fc);
ssize_t iccmsg_read(iccmsg_hal_t *hal, iccmsg_channel_type_t ch, const frame_conf_t *fc, uint8_t *buf);
ssize_t handle_char_dev_write(int dev_fd , const frame_conf_t *fc, uint8_t *buf, uint32_t len);
int iccmsg_write(iccmsg_hal_t *hal, int sync_flag, uint8_t id, const void *data, uint32_t len, iccmsg_msg_status_t ack_stat);
void iccmsg_hal_test(int choice, int times);

#endif

