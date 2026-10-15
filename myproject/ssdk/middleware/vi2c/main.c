/*
 * Copyright (c) 2020 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include "CLI_type.h"
#include <rpmsg_service.h>
#include <rpmsg_channel.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <i2c_cfg.h>
#include <driver/i2c.h>

#define VI2C_DBG_EN    0

#if VI2C_DBG_EN
#define VI2C_DBG(...) printf(__VA_ARGS__)
#else
#define VI2C_DBG(...)
#endif

#define VI2C_RPMSG_TYPE RPMSG_TYPE_IPCC
#define VI2C_RPMSG_RRPOC_AP1 MBOX_RPROC_AP1
#define VI2C_RPMSG_RRPOC_AP2 MBOX_RPROC_AP2
#define VI2C_RPMSG_RRPOC_NUM (2)
#define RPMSG_VI2C_EPT   (73)
#define RPMSG_VI2C_SAFETY_NAME  "safety,vi2c"
#define RPMSG_VI2C_MP_NAME  "mp,vi2c"

#ifdef CONFIG_MP
#define RPMSG_VI2C_NAME  RPMSG_VI2C_MP_NAME
#else
#define RPMSG_VI2C_NAME  RPMSG_VI2C_SAFETY_NAME
#endif

#define I2C_M_RD                     0x1
#define MAX_I2C_NUM                  16

#define PRINT_I2C_MSG
#ifdef PRINT_I2C_MSG
static int print_i2c_msg = -1;
#endif

#define PAYLOAD_HEAD_LEN             sizeof(common_head_t)
#define PAYLOAD_MAX_LEN              112
#define PAYLOAD_DATA_LEN             (PAYLOAD_MAX_LEN - PAYLOAD_HEAD_LEN)

#define VI2C_INIT_COMPLETE_EVENT_FLAG 1u

static uint8_t I2C_HEAD = 0;
static uint8_t I2C_DATA = 1;
static uint8_t I2C_END = 2;

static int vi2c_rpmsg_rproc[VI2C_RPMSG_RRPOC_NUM] = {
    [0] = VI2C_RPMSG_RRPOC_AP1,
    [1] = VI2C_RPMSG_RRPOC_AP2,
};

#if 0
const uint32_t i2c_res_idx[MAX_I2C_NUM] = {
    RES_I2C_I2C1,
    RES_I2C_I2C2,
    RES_I2C_I2C3,
    RES_I2C_I2C4,
    RES_I2C_I2C5,
    RES_I2C_I2C6,
    RES_I2C_I2C7,
    RES_I2C_I2C8,
    RES_I2C_I2C9,
    RES_I2C_I2C10,
    RES_I2C_I2C11,
    RES_I2C_I2C12,
    RES_I2C_I2C13,
    RES_I2C_I2C14,
    RES_I2C_I2C15,
    RES_I2C_I2C16,
};
#endif
typedef struct __common_head_t {
    uint8_t adap_num;
    uint8_t msg_type;
    uint16_t time_flag;
} __attribute__((packed)) common_head_t;

typedef struct __head_info_t {
    common_head_t common_head;
    uint16_t msg_len;
    uint16_t msg_num;
} __attribute__((packed)) head_info_t;

typedef struct __data_info_t {
    common_head_t common_head;
    char i2c_data[PAYLOAD_DATA_LEN];
} __attribute__((packed)) data_info_t;

typedef struct __end_info_t {
    common_head_t common_head;
    int8_t status;
} __attribute__((packed)) end_info_t;

typedef struct __payload_t {
    common_head_t common_head;
    char buf[PAYLOAD_DATA_LEN];
} __attribute__((packed)) payload_t;

struct r_i2c_msg_info {
    uint8_t *r_msg;
    uint8_t *r_msg_p;
    uint16_t htime_flag;
    uint16_t dtime_flag;
    uint16_t etime_flag;
    uint16_t r_len;
    uint16_t r_num;
    uint16_t r_accept_len;
};

struct t_i2c_msg_info {
    uint16_t time_flag;
};

struct vi2c_service_info_t {
    bool binitialized;
    osMessageQueueId_t queue;
    osThreadId_t       thd;
    osEventFlagsId_t initial_event;
    rpmsg_channel_t channel;
    payload_t *payload;
    int payload_len;
    struct r_i2c_msg_info r_info[MAX_I2C_NUM + 1];
    struct t_i2c_msg_info t_info[MAX_I2C_NUM + 1];
};

static struct vi2c_service_info_t vi2c_service_info[VI2C_RPMSG_RRPOC_NUM];

struct i2c_msg_head {
    uint16_t addr;
    uint16_t flags;
    uint16_t len;
};

struct i2c_msg_data {
    uint8_t *buf;
};

struct vi2c_msg {
    struct i2c_msg_head head;
    struct i2c_msg_data data;
};

typedef struct {
    uint8_t* buf;
    uint32_t len;
    uint32_t cnt;
}que_msg_t;

void switch_to_vi2c_msg(uint8_t *r_msg, struct vi2c_msg msgs[], uint16_t num)
{
    struct i2c_msg_head *msgs_h = (struct i2c_msg_head *)r_msg;

    for (int i = 0; i < num; i++) {
        msgs[i].head.addr = msgs_h->addr;
        msgs[i].head.flags = msgs_h->flags;
        msgs[i].head.len = msgs_h->len;
        msgs[i].data.buf = (uint8_t *)++msgs_h;

        if (!(msgs[i].head.flags & I2C_M_RD))
            msgs_h = (struct i2c_msg_head *)((uint8_t *)msgs_h + msgs[i].head.len);
    }
}

void pack_readback(uint8_t adap_num, uint16_t time_flag, uint8_t *t_data,
                   uint16_t t_data_len, head_info_t *head, end_info_t *end,
                   uint8_t *t_payload, int8_t status)
{
    int quotient_size = t_data_len / PAYLOAD_DATA_LEN;
    int remainder_size = t_data_len % PAYLOAD_DATA_LEN;
    data_info_t *data = (data_info_t *)t_payload;

    for (int i = 0; i < quotient_size; i++) {
        data->common_head.adap_num = adap_num;
        data->common_head.msg_type = I2C_DATA;
        data->common_head.time_flag = time_flag;
        memcpy(data->i2c_data, t_data, PAYLOAD_DATA_LEN);
        t_data += PAYLOAD_DATA_LEN;
        data++;
    }

    if (remainder_size) {
        data->common_head.adap_num = adap_num;
        data->common_head.msg_type = I2C_DATA;
        data->common_head.time_flag = time_flag;
        memcpy(data->i2c_data, t_data, remainder_size);
    }

    head->common_head.adap_num = adap_num;
    head->common_head.msg_type = I2C_HEAD;
    head->common_head.time_flag = time_flag;
    head->msg_len = t_data_len;

    end->common_head.adap_num = adap_num;
    end->common_head.msg_type = I2C_END;
    end->common_head.time_flag = time_flag;
    end->status = status;
}

static void send_readback(rpmsg_channel_t channel, head_info_t *head, end_info_t *end,
                   uint8_t *t_payload, uint16_t t_payload_len)
{
    int ret;
    int quotient_size = t_payload_len / PAYLOAD_MAX_LEN;
    int remainder_size = t_payload_len % PAYLOAD_MAX_LEN;
    data_info_t *data = (data_info_t *)t_payload;

    ret = rpmsg_channel_send(channel, RPMSG_VI2C_EPT, head, sizeof(head_info_t), 1000);
    if (ret < 0) {
        printf("vi2c[%u] send head fail\n", head->common_head.adap_num);
    }

    for (int i = 0; i < quotient_size; i++) {
        ret = rpmsg_channel_send(channel, RPMSG_VI2C_EPT, data, PAYLOAD_MAX_LEN, 1000);
        if (ret < 0) {
            printf("vi2c[%u] send data1 fail\n", data->common_head.adap_num);
            break;
        }
        data++;
    }

    if (remainder_size) {
        ret = rpmsg_channel_send(channel, RPMSG_VI2C_EPT, data, remainder_size, 1000);
        if (ret < 0) {
            printf("vi2c[%u] send data2 fail\n", data->common_head.adap_num);
        }
    }

    ret = rpmsg_channel_send(channel, RPMSG_VI2C_EPT, end, sizeof(end_info_t), 1000);
    if (ret < 0) {
        printf("vi2c[%u] send end fail\n", end->common_head.adap_num);
    }
}

void vi2c_message_process2(struct vi2c_service_info_t *vi2c,
                           uint8_t adap_num)
{
    int ret = 0;
    uint16_t i2c_msg_num = 0;
    struct vi2c_msg *msgs = NULL;
    struct i2c_msg* local_msgs = NULL;
    uint8_t *t_data = NULL;
    uint8_t *t_data_p = NULL;
    uint16_t t_data_len = 0;
    uint8_t *t_payload = NULL;
    uint16_t t_payload_len = 0;
    head_info_t head_info = {0};
    end_info_t end_info = {0};
    int8_t status = -1;

    i2c_adap_dev_t* i2c_handle = i2c_get_adap_dev(adap_num + 1);
    if (NULL == i2c_handle){
        printf("%s i2c%u get handle failed\n", __func__, adap_num + 1);
        return;
    }

    /*
    ** analyze i2c msg data send from remote
    ** basic format from remote :
    ** struct i2c_msg {
    **  __u16 addr; //slave addr
    **  __u16 flags; //read(1) or write(0) flag
    **  __u16 len; // len of buf
    **  __u8 *buf; //buf with data to send or buf empty to receive
    ** };
    **
    ** if falgs = read no need buf from remote
    ** i2c msg data received format :
    ** [addr][flags][len][buf]...[addr][flags][len]...[addr][flags][len][buf]
    **
    ** define the same struct compatible with remote struct i2c_msg
    ** struct vi2c_msg {
    **  struct i2c_msg_head head;
    **  struct i2c_msg_data data;
    ** };
    */
    i2c_msg_num = vi2c->r_info[adap_num].r_num;
    msgs = malloc(sizeof(struct vi2c_msg) * i2c_msg_num);
    if (!msgs) {
        printf("vi2c[%u] alloc mem for msgs fail\n", adap_num);
        goto end;
    }
    local_msgs = malloc(sizeof(struct i2c_msg) * i2c_msg_num);
    if (!local_msgs) {
        printf("vi2c[%u] alloc mem for msgs fail\n", adap_num);
        goto end;
    }

    switch_to_vi2c_msg(vi2c->r_info[adap_num].r_msg, msgs, i2c_msg_num);

    for (int i = 0; i < i2c_msg_num; i++) {
        if (msgs[i].head.flags & I2C_M_RD) {
            t_data_len += msgs[i].head.len;
        }
    }

    if (t_data_len > 0) {
        t_data = malloc(t_data_len);
        if (!t_data) {
            printf("vi2c[%u] alloc mem for t_data fail\n", adap_num);
            // free(msgs);
            goto end;
        }
    }

    t_data_p = t_data;

    for (int i = 0; i < i2c_msg_num; i++) {
        if (msgs[i].head.flags & I2C_M_RD) {
            msgs[i].data.buf = t_data_p;
            t_data_p = t_data_p + msgs[i].head.len;
        }
    }

    for (int32_t i = 0; i < i2c_msg_num; i++) {
        local_msgs[i].addr = msgs[i].head.addr;
        local_msgs[i].len = msgs[i].head.len;
        local_msgs[i].rw_flag = msgs[i].head.flags;
        local_msgs[i].buf = msgs[i].data.buf;
        local_msgs[i].polling = 0;
        local_msgs[i].addr_flag = I2C_ADDR7B;
    }

    status = i2c_transfer(i2c_handle, (struct i2c_msg *)local_msgs, i2c_msg_num);
    if(status >= 0)
        status = i2c_msg_num;

    t_payload_len = t_data_len + (t_data_len / PAYLOAD_DATA_LEN + !!
                                  (t_data_len % PAYLOAD_DATA_LEN)) * PAYLOAD_HEAD_LEN;

    if (t_payload_len > 0) {
        t_payload = malloc(t_payload_len);
        if (!t_payload) {
            printf("vi2c[%u] alloc mem for t_payload fail\n", adap_num);
            goto end;
        }
    }

    vi2c->t_info[adap_num].time_flag = vi2c->r_info[adap_num].htime_flag;

    pack_readback(adap_num, vi2c->t_info[adap_num].time_flag, t_data, t_data_len,
                  &head_info, &end_info, t_payload, status);

    send_readback(vi2c->channel, &head_info, &end_info, t_payload, t_payload_len);

#ifdef PRINT_I2C_MSG

    if (print_i2c_msg == adap_num) {
        for (int i = 0; i < vi2c->r_info[adap_num].r_len; i++) {
            printf("vi2c[%u] receive data[%d]=%#x\n",
                   adap_num, i, vi2c->r_info[adap_num].r_msg[i]);
        }

        for (int i = 0; i < t_payload_len; i++) {
            printf("vi2c[%u] payload data[%d]=%#x\n",
                   adap_num, i, t_payload[i]);
        }
    }

#endif

end:
    if (msgs) {
        free(msgs);
    }

    if (local_msgs) {
        free(local_msgs);
    }

    if (t_data) {
        free(t_data);
    }

    if (t_payload) {
        free(t_payload);
    }
}

int vi2c_message_process(struct vi2c_service_info_t *vi2c)
{
    /*
    ** receive rpmsg payload format
    **
    ** head payload : [adap_num /1byte] [msg_type /1byte] [time_flag /2bytes] [msg_len /2bytes] [msg_num /2bytes]
    ** data payload : [adap_num /1byte] [msg_type /1byte] [time_flag /2bytes] [msg /nbytes]
    ** end payload : [adap_num /1byte] [msg_type /1byte] [time_flag /2bytes]
    **
    ** adap_num : the physical i2c adapter num
    ** msg_type : head or data or end flag
    ** time_flag : a time flag check if head/msg/end payload is the same time send from remote
    ** msg_len : the valid i2c_msg len
    ** msg_num : the valid i2c_msg num
    ** msg : the i2c_msg data
    **
    */
    uint8_t adap_num = vi2c->payload->common_head.adap_num;
    uint8_t msg_type = vi2c->payload->common_head.msg_type;
    uint16_t time_flag = vi2c->payload->common_head.time_flag;

    if (adap_num > MAX_I2C_NUM || msg_type > I2C_END) {
        printf("adap_num = %u, msg_type = %u, err\n", adap_num, msg_type);
        return 0;
    }

    if (msg_type == I2C_HEAD) {
        vi2c->r_info[adap_num].htime_flag = time_flag;
        head_info_t *r_head = (head_info_t *)(vi2c->payload);

        if (vi2c->r_info[adap_num].r_msg)
            free(vi2c->r_info[adap_num].r_msg);

        vi2c->r_info[adap_num].r_accept_len = 0;
        vi2c->r_info[adap_num].r_len = r_head->msg_len;
        vi2c->r_info[adap_num].r_num = r_head->msg_num;
        vi2c->r_info[adap_num].r_msg = malloc(vi2c->r_info[adap_num].r_len);
        vi2c->r_info[adap_num].r_msg_p = vi2c->r_info[adap_num].r_msg;
    }
    else if (msg_type == I2C_DATA) {
        vi2c->r_info[adap_num].dtime_flag = time_flag;
        data_info_t *r_data = (data_info_t *)(vi2c->payload);
        uint16_t len = vi2c->payload_len - PAYLOAD_HEAD_LEN;

        if (vi2c->r_info[adap_num].r_msg
                && (vi2c->r_info[adap_num].htime_flag == vi2c->r_info[adap_num].dtime_flag)) {

            vi2c->r_info[adap_num].r_accept_len += len;

            if (vi2c->r_info[adap_num].r_accept_len > vi2c->r_info[adap_num].r_len) {
                printf("adap[%u] r_accept_len = %u, r_len = %u, err\n", adap_num,
                       vi2c->r_info[adap_num].r_accept_len, vi2c->r_info[adap_num].r_len);
                return 0;
            }

            memcpy(vi2c->r_info[adap_num].r_msg_p, r_data->i2c_data, len);
            vi2c->r_info[adap_num].r_msg_p += len;
        }
    }
    else if (msg_type == I2C_END) {
        vi2c->r_info[adap_num].etime_flag = time_flag;
        end_info_t *r_end = (end_info_t *)(vi2c->payload);
        uint16_t len = vi2c->r_info[adap_num].r_msg_p - vi2c->r_info[adap_num].r_msg;

        if (vi2c->r_info[adap_num].r_msg
                && (len == vi2c->r_info[adap_num].r_len)
                && (vi2c->r_info[adap_num].htime_flag == vi2c->r_info[adap_num].dtime_flag)
                && (vi2c->r_info[adap_num].htime_flag == vi2c->r_info[adap_num].etime_flag)) {
            vi2c_message_process2(vi2c, adap_num);
        }
        else {
            printf("vi2c[%u] err:r_msg=%p, len=%u, r_len=%u, htime=%u, dtime=%u, etime=%u\n",
                   adap_num, vi2c->r_info[adap_num].r_msg, len, vi2c->r_info[adap_num].r_len,
                   vi2c->r_info[adap_num].htime_flag, vi2c->r_info[adap_num].dtime_flag,
                   vi2c->r_info[adap_num].etime_flag);
        }

        if (vi2c->r_info[adap_num].r_msg) {
            free(vi2c->r_info[adap_num].r_msg);
            vi2c->r_info[adap_num].r_msg = NULL;
            vi2c->r_info[adap_num].r_msg_p = NULL;
            vi2c->r_info[adap_num].r_len = 0;
            vi2c->r_info[adap_num].r_accept_len = 0;
        }
    }

    return 0;
}

static int rpmsg_common_cb(void *data, int len, unsigned long src, void *arg)
{
    struct vi2c_service_info_t *vi2c = (struct vi2c_service_info_t *)arg;
    // memset(vi2c->payload, 0, PAYLOAD_MAX_LEN);
    // memcpy(vi2c->payload, data, len);

    // VI2C_DBG("==== RPMSG recv data len = %d===== \n", len);
    // char* buf = (char*)data;
    // for (int i = 0; i < len; i++) {
    //     VI2C_DBG("0x%x ", buf[i]);
    // }
    // VI2C_DBG("\n");
    // vi2c->payload_len = len;
    // vi2c_message_process(vi2c);

    static uint32_t cnt = 0;
    que_msg_t que_msg;
    que_msg.buf = malloc(sizeof(payload_t));
    if (que_msg.buf == NULL) {
        printf("malloc memory for msg failed in %s\n", __func__);
        return 0;
    }
    memset(que_msg.buf, 0x00, len);
    memcpy(que_msg.buf, data, len);
    que_msg.len = len;
    que_msg.cnt = ++cnt;

    if (osMessageQueuePut(vi2c->queue, &que_msg, 0, 0) != osOK) {
        // release memory
        free(que_msg.buf);
    }

    return 0;
}

static void vi2c_task_thread(void* args)
{
    struct vi2c_service_info_t *vi2c = (struct vi2c_service_info_t *)args;
    que_msg_t que_msg;
    int32_t ret = 0;
    while (1) {
        ret = osMessageQueueGet(vi2c->queue, &que_msg, 0, osWaitForever);
        if (ret == osOK) {
            vi2c->payload = (payload_t *)que_msg.buf;
            vi2c->payload_len = que_msg.len;
            vi2c_message_process(vi2c);

            VI2C_DBG("==== RPMSG recv data len = %d, cnt = %d ===== \n", que_msg.len, que_msg.cnt);
            char* buf = (char*)que_msg.buf;
            for (int i = 0; i < que_msg.len; i++) {
                VI2C_DBG("0x%x ", buf[i]);
            }
            VI2C_DBG("\n");
            free(que_msg.buf);
        }
        else {
            ssdk_printf(SSDK_ERR, "osMessageQueueGet failed.\n");
        }
    }

    osThreadExit();
}

static void virtual_i2c_init_task(void* args)
{
    for(int i = 0; i < VI2C_RPMSG_RRPOC_NUM; i++) {
        struct vi2c_service_info_t *vi2c = &vi2c_service_info[i];
        struct rpmsg_instance *rpmsg_dev;
        int ret = 0;

        if (vi2c->binitialized) {
            printf("vi2c[%d] has been initialized\n", i);
            goto exit;
        }

        // vi2c->payload = malloc(sizeof(payload_t));
        // if (!vi2c->payload) {
        //     printf("vi2c[%d] alloc mem for payload fail\n", i);
        //     return;
        // }

        vi2c->queue = osMessageQueueNew(10, sizeof(que_msg_t), NULL);
        if (vi2c->queue == NULL) {
            ssdk_printf(SSDK_ERR, "queue create failed.\n");
            goto exit;
        }

        osThreadAttr_t attr = {
            .name = "vi2c",
            .stack_size = 2048,
        };
        vi2c->thd = osThreadNew(vi2c_task_thread, vi2c, &attr);

        /* get a rpmsg instance */
        rpmsg_dev = rpmsg_get_instance(VI2C_RPMSG_TYPE, vi2c_rpmsg_rproc[i]);
        if (rpmsg_dev == NULL) {
            printf("%s: get %d:%d rpmsg device fail, i = %d\n", __func__, VI2C_RPMSG_TYPE, VI2C_RPMSG_RRPOC_AP1, i);
            free(vi2c->payload);
            goto exit;
        }

        /* create a rpmsg channel */
        vi2c->channel = rpmsg_channel_create(rpmsg_dev, RPMSG_VI2C_NAME, RPMSG_VI2C_EPT);

        if (vi2c->channel == NULL) {
            printf("%s: rpmsg_channel_create fail, i = %d\n", __func__, i);
            free(vi2c->payload);
            goto exit;
        }
        /* start the rpmsg channel */
        ret = rpmsg_channel_start(vi2c->channel, rpmsg_common_cb, vi2c);
        if (ret != 0) {
            ssdk_printf(SSDK_CRIT, "virtual_i2c_init: start rpmsg channel failed. i = %d\n", i);
            rpmsg_channel_destroy(vi2c->channel);
            free(vi2c->payload);
            goto exit;
        }

        vi2c->binitialized = true;
    }

    ssdk_printf(SSDK_ALERT, "vi2c init succeeded\n");

exit:
    osThreadExit();
}

void virtual_i2c_init(void)
{
    osThreadAttr_t attr = {
        .name = "vi2c_init",
    };
    osThreadNew(virtual_i2c_init_task, NULL, &attr);
}

#ifdef PRINT_I2C_MSG
static int cmd_print_i2c_msg(int argc, char *argv[])
{
    print_i2c_msg = atoi(argv[0]);
    printf("vi2c print_i2c_msg=%d\n", print_i2c_msg);
    return CLI_FALSE;
}

CLI_CMD("vi2c", "enable print i2c msg", cmd_print_i2c_msg);
#endif


