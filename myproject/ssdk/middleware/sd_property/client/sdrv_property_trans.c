#include <stdlib.h>
#include <string.h>
#include <CLI.h>
#include <cmsis_os2.h>
#include <stdio.h>
#include <debug.h>
#include <board.h>
// #include "dcf.h"

#include <rpmsg_channel.h>
#include <rpmsg_service.h>
#include <rpc/rpmsg_common.h>
#include "sdrv_property_list.h"
#include "sdrv_property_trans.h"

#define SDRV_PROPERTY         "sdrv-property"

#define PROP_WRONG 0
#define PROP_NEED_INIT 1
#define PROP_PING_INITED 2
#define PROP_CLIENT_INITED 3
#define PROP_DEV_MAX_NAME 10

#define PROP_START_MAGIC 0x7E7E
#define PROP_END_MAGIC 0x7B7B

#define PROP_HAVE_NEXT_PACK 0
#define PROP_LAST_PACK 1

#define PROCESS_OVER 1

typedef enum str_flag {
    STR_NAME = 0,
    STR_NAME_VAL = 1,
    STR_VAL = 2,
    STR_MAX,
} str_flag_enum;

typedef enum prop_cmd {
    PROP_CMD_PING,  // 检查通路状态
    PROP_CMD_PACK,  // 检查回复

    PROP_CMD_REQ_LIST,  // data: null   client向manager请求列表
    PROP_CMD_SND_LIST,  // data: name   manager向client回复列表
    PROP_CMD_SND_LIST_ACK, // data: recv len  有分包情况时，client向manager回复上一包接收完了

    PROP_CMD_REQ_ALL,   // data: null
    PROP_CMD_SND_ALL,   // data: name + val
    PROP_CMD_SND_ALL_ACK, // data: recv len

    PROP_CMD_REQ_VAL,   // data: id
    PROP_CMD_SND_VAL,   // data: val
    PROP_CMD_SND_VAL_ACK, // data: recv len

    PROP_CMD_INITED,  // client准备好后，才开始接收更新(manager再发送更新)

    PROP_CMD_SET_VAL,     // 通知manager更新val    -> 设置时，防止更新不及时，是否应该先请求
    PROP_CMD_SET_VAL_ACK, // 当前property已经存在，设置时已经存在？

    PROP_CMD_VAL_UPDATE,  // 广播val发生了变化
    PROP_CMD_PROP_ADD,    // 广播追加新项
    PROP_CMD_PROP_ADD_ACK, // 回复是否追踪

    PROP_CMD_MAX,
} prop_cmd_enum;

struct prop_msg_head {
    uint16_t flag1;  //0x7e7e
    uint16_t len;  // data_len
    uint8_t cmd;
    uint8_t end;   // last package?
    // uint32_t crc;
    uint16_t flag2;  //0x7b7b
} __attribute__((packed));

typedef struct prop_msg_head prop_msg_head_t;

typedef struct msg_reply {
    prop_cmd_enum reply_cmd;
    uint8_t      reply_end;
    uint16_t     reply_data_len;
} msg_reply_t;

#define MSG_HEAD_SIZE (sizeof(prop_msg_head_t))
#define MAX_DATA_LENGTH 470
#define MAX_SEND_LENGTH (MAX_DATA_LENGTH + MSG_HEAD_SIZE)
#define MAX_RECV_LENGTH (MAX_DATA_LENGTH + MSG_HEAD_SIZE)

typedef struct prop_trans_dev_struct {
    char name[PROP_DEV_MAX_NAME];
    struct rpmsg_channel* chn;
    osThreadId_t* recv_handler;
    osThreadId_t* loop_handler;
    osThreadId_t* ping_handler;
    osEventFlagsId_t new_msg_event;
    int status;
    unsigned long recv_id;
    unsigned long remote_id;
    domain_cpu_id_t cpu_id;

    prop_msg_head_t *recv_head;
    char recv_msg[MAX_RECV_LENGTH];
    prop_msg_head_t *send_head;
    char send_msg[MAX_SEND_LENGTH];
    char* msg_str;
    int msg_str_len;
    msg_reply_t reply;
} prop_trans_dev_t;

static prop_trans_dev_t g_prop_trans_dev = {.status = PROP_NEED_INIT, .name = "prop-sec", .cpu_id = DP_CR5_MPC };

static char ping_msg[] = "PING TEST";

static int prop_msg_sender(prop_trans_dev_t *propdev);
int prop_client_send_set_val(char* update_msg, int msg_len)
{
    if (PROP_CLIENT_INITED != g_prop_trans_dev.status) return -1;
    if (NULL == update_msg) return -1;

    memset(g_prop_trans_dev.send_msg, 0x00, MAX_SEND_LENGTH);

    // send to manager
    memcpy(g_prop_trans_dev.send_msg + MSG_HEAD_SIZE, update_msg, msg_len);
    g_prop_trans_dev.reply = (msg_reply_t) {PROP_CMD_SET_VAL, PROP_LAST_PACK, msg_len};

    prop_msg_sender(&g_prop_trans_dev);

    return 0;
}

int prop_ping_ack(prop_trans_dev_t *dev)
{
    // 通信通路OK，可以开始发送请求list
    if (0 == strncmp(ping_msg, dev->recv_msg + MSG_HEAD_SIZE, strlen(ping_msg))) {
        printf("tran establish ok!!\n");
        g_prop_trans_dev.status = PROP_PING_INITED;

#if 1
        // request list name
        memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
        dev->reply = (msg_reply_t) {PROP_CMD_REQ_LIST, PROP_LAST_PACK, 0};
#else
        // request all list
        return PROCESS_OVER;
        memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
        dev->reply = (msg_reply_t) {PROP_CMD_REQ_ALL, PROP_LAST_PACK, 0};
#endif
    }
    // memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
    // memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->recv_msg + MSG_HEAD_SIZE, dev->recv_head->len);
    // dev->reply = (msg_reply_t) {PROP_CMD_INITED, PROP_LAST_PACK, dev->recv_head->len};

    return 0;
}

int prop_send_id_list(prop_trans_dev_t *dev)
{
    if (dev->msg_str) {
        free(dev->msg_str);
        dev->msg_str = NULL;
        dev->msg_str_len = 0;
    }
    dev->msg_str = get_list_str(&dev->msg_str_len);

    // 能超过470个吗？
    // 超过470个后，可以按位传递参数470*8 = 3760
    if (MAX_DATA_LENGTH < dev->msg_str_len) {
        memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->msg_str, MAX_DATA_LENGTH);
        dev->reply = (msg_reply_t) {PROP_CMD_REQ_VAL, PROP_LAST_PACK, MAX_DATA_LENGTH};
    }
    else {
        memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->msg_str, dev->msg_str_len);
        dev->reply = (msg_reply_t) {PROP_CMD_REQ_VAL, PROP_LAST_PACK, dev->msg_str_len};
    }

    return 0;
}

int prop_recv_list(prop_trans_dev_t *dev, str_flag_enum flag, prop_cmd_enum snd_cmd)
{
    dev->msg_str_len += dev->recv_head->len;

    char* tmp = (char*)malloc(dev->msg_str_len + 1);
    if (!tmp) {
        printf("buff malloc failed!\n");
        return -1;
    }
    memset(tmp, 0x00, dev->msg_str_len + 1);

    if (dev->msg_str) {
        memcpy(tmp, dev->msg_str, dev->msg_str_len - dev->recv_head->len);
        free(dev->msg_str);
        dev->msg_str = NULL;
    }

    memcpy(tmp + dev->msg_str_len - dev->recv_head->len, dev->recv_msg + MSG_HEAD_SIZE, dev->recv_head->len);
    dev->msg_str = tmp;

    if (PROP_LAST_PACK != dev->recv_head->end) {
        memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
        (*(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE)) = dev->msg_str_len;
        dev->reply = (msg_reply_t) {snd_cmd, PROP_LAST_PACK, sizeof(uint32_t)};
    }
    else {
        sdrv_property_str2list(flag, dev->msg_str, dev->msg_str_len);
        if (dev->msg_str) {
            free(dev->msg_str);
            dev->msg_str = NULL;
            dev->msg_str_len = 0;
        }

        // inited
        if (PROP_CMD_SND_ALL_ACK == snd_cmd) {
            memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
            (*(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE)) = dev->msg_str_len;
            dev->reply = (msg_reply_t) {PROP_CMD_INITED, PROP_LAST_PACK, sizeof(uint32_t)};
            dev->status = PROP_CLIENT_INITED;
        }
        else if (PROP_CMD_SND_LIST_ACK == snd_cmd) {
            prop_send_id_list(dev);
        }
        else if (PROP_CMD_SND_VAL_ACK == snd_cmd) {
            memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
            (*(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE)) = dev->msg_str_len;
            dev->reply = (msg_reply_t) {PROP_CMD_INITED, PROP_LAST_PACK, sizeof(uint32_t)};
            dev->status = PROP_CLIENT_INITED;
        }

    }

    return 0;
}

int prop_recv_update(prop_trans_dev_t *dev)
{
    char* recv_data = dev->recv_msg + MSG_HEAD_SIZE;
    int* property_id = (int*)recv_data;
    char* val = recv_data + sizeof(int);

    sdrv_property_list_set_val(*property_id, val);
    return PROCESS_OVER;
}

int prop_recv_add(prop_trans_dev_t *dev)
{
    char* recv_data = dev->recv_msg + MSG_HEAD_SIZE;
    int* property_id = (int*)recv_data;
    char* val = recv_data + sizeof(int);
    int ret_id = -1;

    // printf("recv add [%d][%s]\n", *property_id, val);

    sdrv_property_str2list(STR_NAME_VAL, val, strlen(val));

    ret_id = chdeck_domain_set_fllower(*property_id);

    memset(dev->send_msg, 0, MAX_SEND_LENGTH);
    // 1表示关注，0表示不关注
    (*(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE)) = *property_id;
    (*(uint32_t *)(dev->send_msg + MSG_HEAD_SIZE + sizeof(uint32_t))) = ((-1 == ret_id) ? 0 : 1);
    dev->reply = (msg_reply_t) {PROP_CMD_PROP_ADD_ACK, PROP_LAST_PACK, 2 * sizeof(uint32_t)};

    return 0;
}

static int prop_msg_processer(prop_trans_dev_t *dev)
{
    /*clear send data*/
    memset(dev->send_msg, 0, MAX_SEND_LENGTH);
    int ret = 0;

    /*chose action from cmd*/
    switch (dev->recv_head->cmd) {
        case PROP_CMD_PACK :
            ret = prop_ping_ack(dev);
            break;
        case PROP_CMD_SND_LIST :
            ret = prop_recv_list(dev, STR_NAME, PROP_CMD_SND_LIST_ACK);
            break;
        case PROP_CMD_SND_VAL :
            ret = prop_recv_list(dev, STR_VAL, PROP_CMD_SND_VAL_ACK);
            break;
        case PROP_CMD_SND_ALL :
            // ret = prop_recv_list(dev, STR_NAME_VAL, PROP_CMD_SND_ALL_ACK);
            break;
        case PROP_CMD_VAL_UPDATE :
            ret = prop_recv_update(dev);
            break;
        case PROP_CMD_PROP_ADD :
            ret = prop_recv_add(dev);
            break;
        default:
            printf("property msg_processer: error, unknown cmd: %d\n",
                    dev->recv_head->cmd);
            ret = -1;
    }

    return ret;
}

static int prop_msg_checker(prop_trans_dev_t *dev)
{
    prop_msg_head_t *recv_head =  dev->recv_head;

    if (recv_head->len > MAX_DATA_LENGTH) {
        printf("property msg_checker:recv msg len error");
        return -1;
    }

    if (recv_head->flag1 != PROP_START_MAGIC) {
        printf("property msg_checker:recv msg flag1 is 0x%04x, expect 0x%04x \n",
                recv_head->flag1, PROP_START_MAGIC);
        return -1;
    }

    if (recv_head->flag2 != PROP_END_MAGIC) {
        printf("property msg_checker:recv msg flag2 is 0x%04x, expect 0x%04x \n",
                recv_head->flag2, PROP_END_MAGIC);
        return -1;
    }

    return 0;
}

static int prop_msg_sender(prop_trans_dev_t *dev)
{
    prop_msg_head_t *send_head;
    prop_cmd_enum cmd = dev->reply.reply_cmd;
    uint16_t data_len = dev->reply.reply_data_len;
    uint8_t end = dev->reply.reply_end;
    int ret = -1;
    uint16_t total_len;

    if ( data_len > MAX_DATA_LENGTH) {
        printf("%s: length error, data_len =%d\n", __func__, data_len);
        return -1;
    }

    total_len = data_len + MSG_HEAD_SIZE;
    send_head = dev->send_head;
    /*clean the buffer*/
    memset(dev->send_msg, 0, MSG_HEAD_SIZE);
    /*head*/
    send_head->flag1    =   PROP_START_MAGIC;
    send_head->len      =   data_len;
    send_head->cmd      =   cmd;
    send_head->end      =   end;
    send_head->flag2    =   PROP_END_MAGIC;
    /*data is already put in the send_msg*/
    /*crc*/
    // send_head->crc      =   crc32(0, dev->send_msg, total_len);

    /*send msg*/
    ret = rpmsg_channel_send(dev->chn, SDRV_PROPERTY_EPT, (void *)dev->send_msg, total_len, 1000);
    if (ret) {
        printf("%s: send data error, ret=%d\n", __func__, ret);
        return -1;
    }

    if (PROP_CMD_REQ_VAL == send_head->cmd) {
        if (NULL != dev->msg_str) {
            free(dev->msg_str);
            dev->msg_str = NULL;
            dev->msg_str_len = 0;
        }
    }

    return 0;
}

static void prop_msg_callback(void *arg)
{
    prop_trans_dev_t *dev = (prop_trans_dev_t *)arg;
    struct rpmsg_channel *rpchn;
    unsigned char *rxbuf = NULL;
    unsigned long src = SDRV_PROPERTY_EPT;
    int ret;
    int recved = 0;

    rpchn = dev->chn;
    if (!rpchn) {
        return;
    }

    rxbuf = malloc(MAX_RECV_LENGTH);
    if (!rxbuf) {
        printf("No memory for rpmsg channel\n");
        return;
    }

    //@TODO recv msg while channel status active
    while (1) {
        memset(rxbuf, 0, MAX_RECV_LENGTH);
        ret = rpmsg_channel_recv(rpchn, (char *)rxbuf, MAX_RECV_LENGTH, &recved, osWaitForever);
        if (ret < 0) {
            // rpchn->err_cnt++;
            printf(":%s: fail to read data, ret: %d\n", __func__, ret);
            continue;
        }
        // rpchn->rx_cnt++;
        // rpchn->rx_bytes += recved;
        dev->recv_id = src;

        printf("%s: rx %d bytes from %ld\n", __func__, recved, src);

        memset(dev->recv_msg, 0, MAX_RECV_LENGTH);
        memcpy(dev->recv_msg, (char *)rxbuf, MIN((unsigned int)recved, MAX_RECV_LENGTH));
        osEventFlagsSet(dev->new_msg_event, PROPERTY_EVENT_COMPLETE);
        osDelay(100);
    }

    if (rxbuf)
        free(rxbuf);

    osThreadExit();
}

static void prop_msg_handler(void *arg)
{
    prop_trans_dev_t *dev = (prop_trans_dev_t *)arg;
    int ret = -1;

    while (true) {
        osEventFlagsWait(dev->new_msg_event, PROPERTY_EVENT_COMPLETE, osFlagsWaitAll, osWaitForever);
        printf("[%s][%d] recv one msg[%d]\n", dev->name, dev->cpu_id, dev->recv_head->cmd);
        // printf("msg:\n%s\n", dev->recv_msg);

        /*check msg*/
        ret = prop_msg_checker(dev);

        if (ret < 0) {
            printf("%s: msg format error\n", __func__);
            continue;
        }

        /*process msg*/
        ret = prop_msg_processer(dev);

        if (ret) {
            if (PROCESS_OVER == ret) {
                printf("%s: msg process over\n", __func__);
                continue;
            }
            printf("%s: msg process error\n", __func__);
            continue;
        }

        /*send reply msg*/
        ret = prop_msg_sender(dev);

        if (ret) {
            printf("%s: msg send error\n", __func__);
            continue;
        }
    }

    osThreadExit();
}

static void prop_ping_check(void *arg)
{
    prop_trans_dev_t *dev = (prop_trans_dev_t *)arg;
    // int ret = -1;
    int count = 0;

    while(1) {
        count++;
        memcpy(dev->send_msg + MSG_HEAD_SIZE, ping_msg, strlen(ping_msg));
        dev->reply = (msg_reply_t) {PROP_CMD_PING, PROP_LAST_PACK, strlen(ping_msg)};
        prop_msg_sender(dev);

        osDelay(100);
        if (PROP_NEED_INIT == g_prop_trans_dev.status) continue;
        break;
    };

    printf("exit prop_ping_check[%d]\n", count);

    osThreadExit();
}

static void sdrv_property_trans_process(void *arg)
{
    printf("%s start\n", __func__);
    prop_trans_dev_t *dev = (prop_trans_dev_t *)arg;
    int ret = 0;
    int chn_create_retry = 200;

    if (PROP_NEED_INIT > dev->status) goto exit;

    memset(dev->recv_msg, 0x00, MAX_RECV_LENGTH);
    dev->recv_head = (prop_msg_head_t *)dev->recv_msg;
    dev->send_head = (prop_msg_head_t *)dev->send_msg;

    while(1) {
        dev->chn = rpmsg_channel_create(rpmsg_get_instance(RPMSG_TYPE_VIRTIO, dev->cpu_id)
                                                    , dev->name
                                                    , SDRV_PROPERTY_EPT);
        if (NULL == dev->chn) {
            // printf("[%s] create channel[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        }
        else {
            break;
        }
        osDelay(200 * osKernelGetTickFreq() / 1000);
        chn_create_retry--;
        if (chn_create_retry < 0) {
            printf("[%s] create channel[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
            goto exit;
        }
    };

    dev->new_msg_event = osEventFlagsNew(NULL);

    // process thread
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "prop_msg_handler";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;
    dev->recv_handler = osThreadNew(prop_msg_handler, (void*)(dev), &attr);
    if (NULL == dev->recv_handler) {
        printf("[%s] create process thread[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    // loop recv thread
    // dev->status = PROP_SERVER_INITED;

    memset(&attr, 0x00, sizeof(attr));
    attr.name = "prop_msg_callback";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;

    dev->loop_handler = osThreadNew(prop_msg_callback, (void*)(dev), &attr);
    if (NULL == dev->loop_handler) {
        printf("[%s] create loop thread[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    ret = rpmsg_channel_start(dev->chn, NULL, NULL);
    if (0 > ret) {
        printf("[%s] start channel[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    memset(&attr, 0x00, sizeof(attr));
    attr.name = "prop_ping_check";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;
    dev->ping_handler = osThreadNew(prop_ping_check, (void*)(dev), &attr);
    if (NULL == dev->ping_handler) {
        printf("[%s] create ping thread[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    printf("create channel[%d][%d][%s] successed!!\n", dev->cpu_id, SDRV_PROPERTY_EPT, dev->name);

exit:
    osThreadExit();
}

void sdrv_property_trans_init(void)
{
    osThreadAttr_t attr;
    memset(&attr, 0x00, sizeof(attr));
    attr.name = "sdrv_property_trans_process";
    attr.priority = osPriorityNormal;
    attr.stack_size = 2048;

    osThreadId_t thr = osThreadNew(sdrv_property_trans_process, (void*)(&g_prop_trans_dev), &attr);
    if (NULL == thr) {
        printf("[%s] create trans init thread failed!!\n", __func__);
    }
}
