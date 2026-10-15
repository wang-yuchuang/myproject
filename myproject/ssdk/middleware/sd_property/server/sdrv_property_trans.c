
#include "sdrv_property_trans.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>
#include <rpmsg_service.h>
#include <rpmsg_echo.h>
#include <rpmsg_channel.h>
#include <driver/timer.h>
#include <driver/kunlun/syscnt/sdrv_syscnt_drv.h>
#include <CLI_type.h>
#include <rpc/dcf_rpc.h>
#include <rpc/rpmsg_common.h>
#include <property_cfg.h>

#define SDRV_PROPERTY         "sdrv-property"

#define PROP_WRONG 0
#define PROP_NEED_INIT 1
#define PROP_SERVER_INITED 2
#define PROP_CLIENT_INITED 3
#define PROP_DEV_MAX_NAME 10

#define PROP_START_MAGIC 0x7E7E
#define PROP_END_MAGIC 0x7B7B

#define PROP_HAVE_NEXT_PACK 0
#define PROP_LAST_PACK 1

#define PROCESS_OVER 1

#define MTU_MAX_NUM 600

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
    osEventFlagsId_t new_msg_event;
    int status;
    unsigned long recv_id;
    unsigned long remote_id;
    domain_cpu_id_t cpu_id;

    osMutexId_t send_msg_lock;
    prop_msg_head_t *recv_head;
    char recv_msg[MAX_RECV_LENGTH];
    prop_msg_head_t *send_head;
    char send_msg[MAX_SEND_LENGTH];
    char* msg_str;
    msg_reply_t reply;
} prop_trans_dev_t;

static prop_trans_dev_t prop_trans_dev[DP_CPU_MAX] = {
// #if CONFIG_DCF_HAS_LO
    [DP_CR5_SAF] = {.cpu_id = DP_CR5_SAF, .status = PROP_NEED_INIT, .name = "prop-saf" },
// #endif

// #if CONFIG_DCF_HAS_SEC
    [DP_CR5_SEC] = {.cpu_id = DP_CR5_SEC, .status = PROP_NEED_INIT, .name = "prop-sec" },
// #endif

#if CONFIG_DCF_HAS_MP
    [DP_CR5_MPC] = {.cpu_id = DP_CR5_MPC, .status = PROP_NEED_INIT, .name = "prop-mp" },
#endif

// #if CONFIG_DCF_HAS_AP1
    [DP_CA_AP1] = {.cpu_id = DP_CA_AP1, .status = PROP_NEED_INIT, .name = "prop-ap" },
// #endif

// #if CONFIG_DCF_HAS_AP2
    [DP_CA_AP2] = {.cpu_id = DP_CA_AP2, .status = PROP_NEED_INIT, .name = "prop-ap" },
// #endif
};

static int someone_syncing = 0;
static osMutexId_t g_sync_lock;

extern char* get_list_str(int flag, char* id_list, int id_list_len, int cpu_id);
extern int get_property_fllower(int property_id, int* last);
extern int set_property_fllower(int property_id, int flag, int cpu_id);
extern int sdrv_property_list_set_val(int property_id, void* name, void* val, int store);

int prop_ping(prop_trans_dev_t *dev)
{
    // save remote id
    dev->remote_id = dev->recv_id;

    memset(dev->send_msg, 0x00, MAX_SEND_LENGTH);
    memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->recv_msg + MSG_HEAD_SIZE, dev->recv_head->len);
    dev->reply = (msg_reply_t) {PROP_CMD_PACK, PROP_LAST_PACK, dev->recv_head->len};

    return 0;
}

int prop_req_list(prop_trans_dev_t *dev, str_flag_enum flag, prop_cmd_enum snd_cmd)
{
    dev->msg_str = get_list_str(flag, dev->recv_msg + MSG_HEAD_SIZE, dev->recv_head->len, dev->cpu_id);
    // ssdk_printf(SSDK_ERR, "%s, len[%d] str:\n%s\n", __func__, strlen(dev->msg_str), dev->msg_str);

    osMutexAcquire(g_sync_lock, osWaitForever);
    if (snd_cmd == PROP_CMD_SND_ALL || snd_cmd == PROP_CMD_SND_LIST)
        someone_syncing++;
    osMutexRelease(g_sync_lock);

    if (MAX_DATA_LENGTH < strlen(dev->msg_str)) {
        memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->msg_str, MAX_DATA_LENGTH);
        dev->reply = (msg_reply_t) {snd_cmd, PROP_HAVE_NEXT_PACK, MAX_DATA_LENGTH};
    }
    else {
        memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->msg_str, strlen(dev->msg_str));
        dev->reply = (msg_reply_t) {snd_cmd, PROP_LAST_PACK, strlen(dev->msg_str)};
    }

    return 0;
}

int prop_req_list_next_pack(prop_trans_dev_t *dev, prop_cmd_enum snd_cmd)
{
    uint32_t* recv_len = (uint32_t *)(dev->recv_msg + MSG_HEAD_SIZE);
    ssdk_printf(SSDK_ERR, "recv_len: %d\n", *recv_len);
    if (*recv_len >= strlen(dev->msg_str)) {
        ssdk_printf(SSDK_ERR, "recv_over\n");
        free(dev->msg_str);
        dev->msg_str = NULL;
    }
    else {
        if (MAX_DATA_LENGTH < strlen(dev->msg_str + *recv_len)) {
            memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->msg_str + *recv_len, MAX_DATA_LENGTH);
            dev->reply = (msg_reply_t) {snd_cmd, PROP_HAVE_NEXT_PACK, MAX_DATA_LENGTH};
        }
        else {
            memcpy(dev->send_msg + MSG_HEAD_SIZE, dev->msg_str + *recv_len, strlen(dev->msg_str + *recv_len));
            dev->reply = (msg_reply_t) {snd_cmd, PROP_LAST_PACK, strlen(dev->msg_str + *recv_len)};
        }
    }

    return 0;
}

int prop_client_inited(prop_trans_dev_t *dev)
{
    int ret = 0;

    if (PROP_SERVER_INITED == dev->status) {
        dev->status = PROP_CLIENT_INITED;
        ret = PROCESS_OVER;

        osMutexAcquire(g_sync_lock, osWaitForever);
        someone_syncing--;
        osMutexRelease(g_sync_lock);
    }
    else {
        ssdk_printf(SSDK_ERR, "%s: status error[%d]\n", __func__, dev->status);
        ret = -1;
    }
    ssdk_printf(SSDK_ERR, "%s: [%s][%d]client init [%d]\n", __func__, dev->name, dev->cpu_id, dev->status);

    return ret;
}

int prop_add_fllower(prop_trans_dev_t *dev)
{
    int ret = 0;

    uint32_t* recv_id = (uint32_t *)(dev->recv_msg + MSG_HEAD_SIZE);
    uint32_t* recv_flag = (uint32_t *)(dev->recv_msg + MSG_HEAD_SIZE + sizeof(uint32_t));

    if (*recv_flag) {
        set_property_fllower(*recv_id, *recv_flag, dev->cpu_id);
    }

    ret = PROCESS_OVER;
    return ret;
}

int prop_client_set_val(prop_trans_dev_t *dev)
{
    int ret = 0;
    char* name = NULL;
    char* val = NULL;

    int* recv_id = (int *)(dev->recv_msg + MSG_HEAD_SIZE);
    char* str = dev->recv_msg + MSG_HEAD_SIZE + sizeof(int);

    ssdk_printf(SSDK_ERR, "client set val[%d][%s]\n", *recv_id, str);

    char* sparator = strstr(str, "\n");
    if (NULL == sparator) return -1;

    int idx = sparator - str;
    if (0 == idx) {
        // no name
        ssdk_printf(SSDK_ERR, "no name!!\n");
        name = NULL;
    }
    else {
        *sparator = '\0';
        name = str;
    }
    val = str + idx + 1;

    // ssdk_printf(SSDK_ERR, "client set val[%d][%s][%s]\n", *recv_id, name, val);

    sdrv_property_list_set_val(*recv_id, name, val, 0);

    ret = PROCESS_OVER;
    return ret;
}

static int prop_msg_processer(prop_trans_dev_t *dev)
{
    int ret = 0;
    /*clear send data*/
    osMutexAcquire(dev->send_msg_lock, osWaitForever);
    memset(dev->send_msg, 0, MAX_SEND_LENGTH);
    osMutexRelease(dev->send_msg_lock);

    /*chose action from cmd*/
    switch (dev->recv_head->cmd) {
        case PROP_CMD_PING :
            ret = prop_ping(dev);
            break;
        case PROP_CMD_REQ_LIST :
            ret = prop_req_list(dev, STR_NAME, PROP_CMD_SND_LIST);
            break;
        case PROP_CMD_SND_LIST_ACK :
            ret = prop_req_list_next_pack(dev, PROP_CMD_SND_LIST);
            break;
        case PROP_CMD_REQ_ALL :
            ret = prop_req_list(dev, STR_NAME_VAL, PROP_CMD_SND_ALL);
            break;
        case PROP_CMD_SND_ALL_ACK :
            ret = prop_req_list_next_pack(dev, PROP_CMD_SND_ALL);
            break;
        case PROP_CMD_REQ_VAL :
            ret = prop_req_list(dev, STR_VAL, PROP_CMD_SND_VAL);
            break;
        case PROP_CMD_SND_VAL_ACK :
            ret = prop_req_list_next_pack(dev, PROP_CMD_SND_VAL);
            break;
        case PROP_CMD_INITED :
            ret = prop_client_inited(dev);
            break;
        case PROP_CMD_PROP_ADD_ACK :
            ret = prop_add_fllower(dev);
            break;
        case PROP_CMD_SET_VAL :
            ret = prop_client_set_val(dev);
            break;
        default:
            ssdk_printf(SSDK_ERR, "property msg_processer: error, unknown cmd: %d\n",
                    dev->recv_head->cmd);
            ret = -1;
            break;
    }

    return ret;
}

static int prop_msg_checker(prop_trans_dev_t *dev)
{
    prop_msg_head_t *recv_head =  dev->recv_head;

    if (recv_head->len > MAX_DATA_LENGTH) {
        ssdk_printf(SSDK_ERR, "property msg_checker:recv msg len error");
        return -1;
    }

    if (recv_head->flag1 != PROP_START_MAGIC) {
        ssdk_printf(SSDK_ERR, "property msg_checker:recv msg flag1 is 0x%04x, expect 0x%04x \n",
                recv_head->flag1, PROP_START_MAGIC);
        return -1;
    }

    if (recv_head->flag2 != PROP_END_MAGIC) {
        ssdk_printf(SSDK_ERR, "property msg_checker:recv msg flag2 is 0x%04x, expect 0x%04x \n",
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
        ssdk_printf(SSDK_ERR, "%s: length error, data_len =%d\n", __func__, data_len);
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


    // ssdk_printf(SSDK_ERR, CRITICAL, "update monitor sendmsg:\n");
    // msg_printer(dev->send_msg, total_len);
    // ssdk_printf(SSDK_ERR, "len[%d] str:\n%s\n", data_len, dev->send_msg + MSG_HEAD_SIZE);

    /*send msg*/
    ret = rpmsg_channel_send(dev->chn, dev->remote_id, (void *)dev->send_msg, total_len, 1000);
    if (ret) {
        ssdk_printf(SSDK_ERR, "%s: send data error, ret=%d\n", __func__, ret);
        return -1;
    }

    return 0;
}

int prop_update_fllower(int property_id, char* update_msg, int msg_len)
{
    int remote_id = 0;
    prop_trans_dev_t *dev = NULL;
    int idx = 0;

    if (NULL == update_msg) return -1;

    remote_id = get_property_fllower(property_id, &idx);
    while(-1 != remote_id) {
        // send update msg to fllower
        dev = &prop_trans_dev[remote_id];

        if (PROP_CLIENT_INITED != dev->status) {
            osDelay(1);
            continue;
        }

        // ssdk_printf(SSDK_ERR, "[%d]send update to fllower[%s][%d]\n", property_id, dev->name, dev->cpu_id);

        // int* propid = (int*)update_msg;
        // char* val = update_msg + sizeof(int);

        // ssdk_printf(SSDK_ERR, "send update [%d][%s]\n", *propid, val);

        osMutexAcquire(dev->send_msg_lock, osWaitForever);
        memcpy(dev->send_msg + MSG_HEAD_SIZE, update_msg, msg_len);
        dev->reply = (msg_reply_t) {PROP_CMD_VAL_UPDATE, PROP_LAST_PACK, msg_len};

        // 开机时，res/emmc多线程发送，可能导致接收端处理不过来
        osDelay(10);
        prop_msg_sender(dev);
        osMutexRelease(dev->send_msg_lock);

        // next remote
        remote_id = get_property_fllower(property_id, &idx);
    }

    return 0;
}

int prop_send_item_add(char* update_msg, int msg_len)
{
    prop_trans_dev_t *dev = NULL;

    if (NULL == update_msg) return -1;

    for (int i = 0; i < DP_CPU_MAX; i++) {
        dev = &prop_trans_dev[i];
        if (PROP_CLIENT_INITED != dev->status) continue;

        ssdk_printf(SSDK_ERR, "send item add to[%s][%d]\n", dev->name, dev->cpu_id);

        int* propid = (int*)update_msg;
        char* val = update_msg + sizeof(int);
        ssdk_printf(SSDK_ERR, "send update [%d][%s]\n", *propid, val);

        osMutexAcquire(dev->send_msg_lock, osWaitForever);
        memcpy(dev->send_msg + MSG_HEAD_SIZE, update_msg, msg_len);
        dev->reply = (msg_reply_t) {PROP_CMD_PROP_ADD, PROP_LAST_PACK, msg_len};

        // 开机时，res/emmc多线程发送，可能导致接收端处理不过来
        osDelay(10);
        prop_msg_sender(dev);
        osMutexRelease(dev->send_msg_lock);
    }

    return 0;
}

int sdrv_property_trans_get_syncing()
{
    return someone_syncing;
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

    rxbuf = malloc(MTU_MAX_NUM);
    if (!rxbuf) {
        ssdk_printf(SSDK_ERR, "No memory for rpmsg channel\n");
        return;
    }

    // while (rpchn->state == DCF_STATE_Connected) {
    while (1) {
        memset(rxbuf, 0, MTU_MAX_NUM);
        ret = rpmsg_channel_recv(rpchn,
                     (char*) rxbuf, MTU_MAX_NUM, &recved, RPMSG_BLOCK);
        if (ret < 0) {
            // rpchn->err_cnt++;
            ssdk_printf(SSDK_ERR, ":%s: fail to read data, ret: %d\n", __func__, ret);
            continue;
        }
        // rpchn->rx_cnt++;
        // rpchn->rx_bytes += recved;
        dev->recv_id = src;

        ssdk_printf(SSDK_ERR, "%s: rx %d bytes from %ld\n", __func__, recved, src);

        memset(dev->recv_msg, 0, MAX_RECV_LENGTH);
        memcpy(dev->recv_msg, (char *)rxbuf, MIN((unsigned int)recved, MAX_RECV_LENGTH));
        osEventFlagsSet(dev->new_msg_event, PROPERTY_EVENT_COMPLETE);
        osDelay(100);
    }

    // rpchn->state = DCF_STATE_Closed;
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
        ssdk_printf(SSDK_ERR, "[%s][%d] recv one msg[%d]\n", dev->name, dev->cpu_id, dev->recv_head->cmd);
        // ssdk_printf(SSDK_ERR, "msg:\n%s\n", dev->recv_msg);

        /*check msg*/
        ret = prop_msg_checker(dev);

        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "%s: msg format error\n", __func__);
            continue;
        }

        /*process msg*/
        ret = prop_msg_processer(dev);

        if (ret) {
            if (PROCESS_OVER == ret) {
                ssdk_printf(SSDK_ERR, "%s: msg process over\n", __func__);
                continue;
            }
            ssdk_printf(SSDK_ERR, "%s: msg process error\n", __func__);
            continue;
        }

        /*send reply msg*/
        ret = prop_msg_sender(dev);

        if (ret) {
            ssdk_printf(SSDK_ERR, "%s: msg send error\n", __func__);
            continue;
        }
    }

    osThreadExit();
}

static void sdrv_property_trans_process(void *arg)
{
    ssdk_printf(SSDK_ERR, "%s start\n", __func__);
    prop_trans_dev_t *dev = (prop_trans_dev_t *)arg;
    int ret = 0;
    int chn_create_retry = 200;
    struct rpmsg_instance *rpmsg_dev;

    if (dev->cpu_id == board_property_get_this_proc()) goto exit;
    if (PROP_NEED_INIT > dev->status) goto exit;

    dev->send_msg_lock = osMutexNew(NULL);

    memset(dev->recv_msg, 0x00, MAX_RECV_LENGTH);
    dev->recv_head = (prop_msg_head_t *)dev->recv_msg;
    dev->send_head = (prop_msg_head_t *)dev->send_msg;
    // dev->cpu_id = i;

    while(1) {
        rpmsg_dev = rpmsg_get_instance(RPMSG_TYPE_VIRTIO, dev->cpu_id);
        dev->chn = rpmsg_channel_create(rpmsg_dev, dev->name, SDRV_PROPERTY_EPT);
        if (NULL == dev->chn) {
            // ssdk_printf(SSDK_ERR, "[%s] create channel[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
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
    }

    dev->new_msg_event = osEventFlagsNew(NULL);

    // process thread
    osThreadAttr_t handler_attr = {.name = "prop_msg_handler", .stack_size = 4096, .priority = osPriorityNormal};
    dev->recv_handler = osThreadNew(prop_msg_handler, (void*)(dev), &handler_attr);
    if (NULL == dev->recv_handler) {
        ssdk_printf(SSDK_ERR, "[%s] create process thread[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    // loop recv thread
    dev->status = PROP_SERVER_INITED;

    ret = rpmsg_channel_start(dev->chn, NULL, NULL);
    if (0 > ret) {
        ssdk_printf(SSDK_ERR, "[%s] start channel[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    osThreadAttr_t callback_attr = {.name = "prop_msg_callback", .stack_size = 4096, .priority = osPriorityNormal};
    dev->loop_handler = osThreadNew(prop_msg_callback, (void*)(dev), &callback_attr);
    if (NULL == dev->loop_handler) {
        ssdk_printf(SSDK_ERR, "[%s] create loop thread[%d][%s] failed!!\n", __func__, dev->cpu_id, dev->name);
        rpmsg_channel_destroy(dev->chn);
        goto exit;
    }

    ssdk_printf(SSDK_ERR, "create channel[%d][%d][%s] successed!!\n", dev->cpu_id, SDRV_PROPERTY_EPT, dev->name);

exit:
    osThreadExit();
}

void sdrv_property_trans_init(void)
{
    g_sync_lock = osMutexNew(NULL);

    for (int i = 0; i < DP_CPU_MAX; i++) {
        osThreadAttr_t attr;
        memset(&attr, 0x00, sizeof(attr));
        attr.name = "sdrv_property_trans_process";
        attr.priority = osPriorityNormal;
        attr.stack_size = 2048;

        osThreadId_t thr = osThreadNew(sdrv_property_trans_process, (void*)(&(prop_trans_dev[i])), &attr);
        if (NULL == thr) {
            ssdk_printf(SSDK_ERR, "create trans init thread[%d][%d][%s] failed!!\n", i, SDRV_PROPERTY_EPT, prop_trans_dev[i].name);
        }
    }
}
