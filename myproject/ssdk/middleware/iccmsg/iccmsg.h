
#ifndef __ICCMSG_H__
#define __ICCMSG_H__
#include <rpmsg_channel.h>
#include <cmsis_os2.h>
#include "iccmsg_dpr.h"


// 同步回调函数指针类型
typedef int (*iccmsg_rx_cb_t)(void *user_ctx, const void *data, uint32_t len);

// app 接收消息结构体
typedef struct {
    uint8_t id;
    uint32_t len;
    uint8_t *data;
}__attribute__ ((packed)) iccmsg_rx_msg_t;


/* 多帧重组上下文 (per-instance，替代函数级 static 状态) */
typedef struct {
    bool active;                    /* 是否有活跃的多帧重组 */
    uint8_t id;                     /* 当前重组消息 ID */
    uint32_t bytes;                 /* 已接收字节数 */
    iccmsg_data_info_t info;        /* 多帧头信息 */
    iccmsg_rx_msg_t *msg;           /* 重组消息缓冲 */
} iccmsg_reassembly_ctx_t;

/* 关键统计项 */
typedef struct {
    uint32_t tx_cnt;
    uint32_t rx_cnt;
    uint32_t ack_timeout_cnt;
    uint32_t reassembly_fail_cnt;
    uint32_t crc_fail_cnt;
} iccmsg_stats_t;


typedef struct{
    /* iccmsg信息管理 */
    char                name[32];       // iccmsg 名称
    uint16_t            ch_ept;         // ipcc 与 virtio 通道关联的 endpoint
    rpmsg_channel_t     *ipcc_ch;       // ipcc 通道实例
    rpmsg_channel_t     *virtio_ch;     // vitio 通道实例
    struct list_node    node;
    osThreadId_t        tx_task;        // 异步发送线程句柄
    osPriority_t        tx_task_priority; // 异步发送线程优先级，默认24；建议高于业务线程(25)以减少队列积压
    uint16_t            tx_task_stack_size;
    // 数据处理配置
    frame_conf_t frame_cfg;             // app传入，协议帧定义

    // 异步队列
    osMessageQueueId_t rx_queue;        // 接收消息队列, 传输合并后的original data，类型: iccmsg_rx_msg_t*，生产端:iccmsg 消费端：app
    osMessageQueueId_t tx_queue;        // 发送消息队列, 传输发送过程数据，类型: iccmsg_que_msg_t* ,生产端：iccmsg 消费端：iccmsg
    osEventFlagsId_t event_id;          // iccmsg事件

    // 同步回调（高实时性）
    iccmsg_rx_cb_t ipcc_rx_cb;          // app传入，同步消息处理函数
    void *user_ctx;

    // iccmsg 收发原始帧记录
    iccmsg_cache_ctx_t rx_ctx;          // 接收协议帧记录, 链表
    iccmsg_cache_ctx_t tx_ctx;          // 发送协议帧记录, 链表


    /* 多帧重组上下文 (per-instance，替代 static 状态) */
    iccmsg_reassembly_ctx_t reassembly;

    /* 关键统计项 */
    iccmsg_stats_t stats;

    /* ACK 精确匹配 */
    bool ack_waiting;                   /* 当前是否有待匹配 ACK */
    uint8_t ack_wait_id;                /* 当前等待 ACK 的消息 ID */
}iccmsg_t;


// 初始化
int iccmsg_init(void);

// app 创建 iccmsg实例，并返回实例地址，NULL失败
iccmsg_t *iccmsg_add(const char *name, frame_conf_t conf, iccmsg_rx_cb_t cb, void *ctx);

// 销毁通道
int iccmsg_remove(iccmsg_t *iccmsg);

// iccmsg 数据发送
int iccmsg_send(iccmsg_t *iccmsg, int sync_flag, uint8_t id, const void *data, uint32_t len,iccmsg_msg_status_t ack_stat);

// 通过iccmsg 名称查找iccmsg实例
iccmsg_t *find_iccmsg(const char *name);

// 展示所有iccsmg 实例
void iccmsg_list_all(void);

// 获取iccmsg已有的实例个数
int get_iccmsg_num(void);

#endif