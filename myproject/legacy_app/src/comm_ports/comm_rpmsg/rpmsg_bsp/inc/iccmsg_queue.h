#ifndef __ICCMSG_QUEUE_H__
#define __ICCMSG_QUEUE_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/queue.h>
#include "iccmsg_dpr.h"


#define MAX_MESSAGE_SIZE 512
#define DEFAULT_QUEUE_SIZE 100

// 消息结构体
typedef struct {
    int id;
    char *data;
    size_t data_len;
    struct timeval timestamp;  // 时间戳用于延迟测量
    iccmsg_msg_status_t state;
    iccmsg_channel_type_t ch_type;
} message_t;

// 队列节点
typedef struct msg_node {
    message_t msg;
    TAILQ_ENTRY(msg_node) entries;
} msg_node_t;

// 消息队列结构体
typedef TAILQ_HEAD(msg_queue_head, msg_node) msg_queue_t;

typedef struct {
    msg_queue_t queue;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int closed;
    int max_size;
    long total_messages_sent;
    long total_messages_received;
    long total_bytes_sent;
    long total_bytes_received;
} thread_safe_queue_t;

// 测试参数结构
typedef struct {
    int message_size;
    int message_count;
    int num_threads;
    int queue_size;
    thread_safe_queue_t *queue;  // 添加队列指针
} test_params_t;

// 统计信息
typedef struct {
    double throughput_mbps;
    double avg_latency_us;
    struct timeval start_time;
    struct timeval end_time;
    long total_messages;
    long total_bytes;
    long total_latency_us;
} test_stats_t;


int init_queue(thread_safe_queue_t *tsq, int max_size);
int destroy_queue(thread_safe_queue_t *tsq);
int send_message(thread_safe_queue_t *tsq, const message_t *msg);
int receive_message(thread_safe_queue_t *tsq, message_t *msg);
int receive_message_timeout(thread_safe_queue_t *tsq, message_t *msg, int timeout_ms);
int iccmsg_queue_test();

#endif
