#ifndef __ICCMSG_DEMO_H__
#define __ICCMSG_DEMO_H__

//此测试依赖R核作为测试服务器接收测试命令配合测试
#define ICCMSG_TEST_REMOTE_EPT      500
#define ICCMSG_TEST_REMOTE_NAME     "iccmsg_test"
#define ICCMSG_TEST_LOCAL_EPT       501
#define ICCMSG_TEST_LOCAL_NAME      "iccmsg_test_ap"
#define BUF_SIZE                    1024
#define MAX_EVENTS                  256
#define ICCMSG_TEST_MULTI_MSG_SIZE  (1024)

#define ICCMSG_TEST_NUM             10
#define ICCMSG_TEST_MSG_HEAD        0xA0
#define ICCMSG_TEST_MSG_TAIL        0xA5
#define ICCMSG_RX_QUEUE_SIZE        32




typedef enum {
    ICCMSG_DEMO_CMD_RSV = 0,

    IPCC_TX_NACK,
    IPCC_TX_ACK,
    VIRTIO_MULTI_NACK,
    VIRTIO_MULTI_ACK,

    ICCMSG_DEMO_CMD_MAX,
} iccmsg_demo_cmd_t;



/*
    iccmsg demo 测试命令:
    sec-tx,ap-rx,cmd:
    1. ipcc 单帧同步发送 无ACK
    2. ipcc 单帧异步发送 需要ACK
    3. virtio 单帧同步发送 无ACK
    4. virtio 单帧异步发送 需要ACK
    5. virtio 多帧同步发送 无ACK
    6. virtio 多帧异步发送 需要ACK
*/
#define CMD_SINGLE_SYNC_ECHO_NACK               1   // 单帧同步 NACK 回传测试 server: sec  client: ap
#define CMD_SINGLE_ASYNC_ECHO_ACK               2   // 单帧异步 NACK 回传测试 server: sec  client: ap
#define CMD_MULTI_SYNC_ECHO_NACK                3
#define CMD_MULTI_ASYNC_ECHO_ACK                4   // 多帧异步 ACK 回传测试 server: sec  client: ap
#define CMD_SINGLE_ASYNC_ECHO_NACK              5   // 多帧异步 ACK 回传测试 server: sec  client: ap
#define CMD_MULTI_ASYNC_ECHO_NACK              6   // 多帧异步 NACK 回传测试 server: sec  client: ap

typedef struct {
    uint32_t cmd;       // 消息命令
    uint32_t cnt;       // 测试记录
}__attribute__((packed)) iccmsg_test_msg_t;

typedef struct {
    uint32_t cmd;       // 消息命令
    uint32_t cnt;       // 测试记录
    uint8_t data[ICCMSG_TEST_MULTI_MSG_SIZE];
}__attribute__((packed)) iccmsg_test_multi_msg_t;



#endif

