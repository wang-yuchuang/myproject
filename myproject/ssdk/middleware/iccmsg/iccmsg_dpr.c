#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <rpmsg.h>
#include "CLI_type.h"
#include <rpmsg_service.h>
#include <rpmsg_channel.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include "iccmsg_endian.h"
#include "iccmsg_dpr.h"



uint64_t get_timestamp_ns(void)
{
    // return (uint64_t)osKernelGetTickCount() * (1000000000ULL / osKernelGetTickFreq());
    return (uint64_t)osKernelGetTickCount();
}

int iccmsg_frm_cfg(frame_conf_t* frm_cfg, 
                    uint8_t *head, 
                    uint8_t head_len, 
                    uint8_t *tail, 
                    uint8_t tail_len, 
                    checksum_t chksum){
    frame_conf_t *p_opr = frm_cfg;
    // 指针有效性检查
    if (!p_opr || !head || !tail ){
        ssdk_printf(SSDK_ERR,"%s: null pointer!\n",__func__);
        return -1;
    }
    // 参数有效性检查
    if(head_len > ICCMSG_FRAME_HEAD_MAX_LEN ||
        tail_len > ICCMSG_FRAME_TAIL_MAX_LEN ||
        chksum > CRC32){
        ssdk_printf(SSDK_ERR,"%s: unavailable param!\n",__func__);
        return -1;
    }
    // 传参复制
    memcpy(p_opr->frame_head,head,head_len);
    p_opr->head_len = head_len;
    memcpy(p_opr->frame_tail,tail,tail_len);
    p_opr->tail_len = tail_len;
    p_opr->checksum_type = chksum;

    return 0;
}

int get_frm_data_pos(const frame_conf_t *fc){
    if(NULL == fc)
        return -1;
    return (fc->head_len + ICCMSG_FRAME_DATA_LEN_LEN);
}

int get_origin_data_pos(const frame_conf_t *fc){
    if(NULL == fc)
        return -1;
    return (fc->head_len + ICCMSG_FRAME_DATA_LEN_LEN + sizeof(iccmsg_data_type_t));
}


int get_frame_data_len(const uint8_t *buf, const frame_conf_t *fc)
{
    uint8_t * p_opr = NULL;
    uint16_t data_len;
    if(NULL == buf)
        return -1;
    p_opr = &buf[fc->head_len];
    // 获取数据长度
    if(is_little_endian())
        data_len = LOAD_LE16(p_opr);
    else
        data_len = LOAD_BE16(p_opr);
    return (int)data_len;
}

bool iccmsg_verify(const uint8_t *buf, uint32_t len, const frame_conf_t *fc)
{
    uint32_t i,opr_pos,data_len,rcv_checksum,calc_checksum;
    const uint8_t *payload = NULL;

    if (!buf || !fc ) {
        ssdk_printf(SSDK_ERR, "iccmsg_verify unavailable param.\n");
        return false;
    }

    // 校验帧头
    for (i = 0; i < fc->head_len; i++) {
        if (buf[i] != fc->frame_head[i]) {
            ssdk_printf(SSDK_ERR, "iccmsg_verify wrong frame head.fc->frame_head[%d] = 0x%x != buf[%d] = 0x%x.\n", i, fc->frame_head[i], i, buf[i]);
            return false;
        }
    }

    // 校验帧尾
    opr_pos = len - fc->tail_len;
    for (int i = 0; i < fc->tail_len; i++) {
        if (buf[opr_pos + i] != fc->frame_tail[i]) {
            ssdk_printf(SSDK_ERR, "iccmsg_verify wrong frame: fc->tail_len = %d, tail.fc->frame_tail[%d] = 0x%x != buf[%d] = 0x%x.\n", \
                                    fc->tail_len, i, fc->frame_tail[i], opr_pos + i, buf[opr_pos + i]);
            return false;
        }
    }

    // 获取数据长度
    data_len = get_frame_data_len(buf,fc);
    ssdk_printf(SSDK_DEBUG, "iccmsg_verify frame data len = %d.\n",data_len);
    if (data_len + fc->head_len + 2 + 4 + fc->tail_len > len){
        ssdk_printf(SSDK_ERR, "iccmsg_verify unavailable frame len: data_len = %d.\n",data_len);
        return false;
    }
            
    /* 获取frame_checksum */
    opr_pos = len - fc->tail_len - 4;
    if(is_little_endian())
        rcv_checksum = LOAD_LE32(&buf[opr_pos]);
    else
        rcv_checksum = LOAD_BE32(&buf[opr_pos]);

    calc_checksum = 0;
    payload = &buf[opr_pos-data_len];
    ssdk_printf(SSDK_DEBUG, "iccmsg_verify opr_pos = %d, data_len = %d. payload data:\n",opr_pos, data_len);
    for(i = 0; i < data_len; i++){
        ssdk_printf(SSDK_DEBUG, "0x%x,",payload[i]);
    }
    ssdk_printf(SSDK_DEBUG, "\n");
    if (fc->checksum_type == CRC32) {
        calc_checksum = sd_crc32(0, payload, data_len);
    } else if (fc->checksum_type == SUM) {
        for (uint32_t i = 0; i < data_len; i++) {
            calc_checksum += payload[i];
        }
    }
    
    if(rcv_checksum != calc_checksum){ 
        /* 完整性校验失败，丢弃 */
        ssdk_printf(SSDK_ERR, "iccmsg_verify checksum wrong: calc = 0x%x, rcv = 0x%x.\n",calc_checksum, rcv_checksum);
        return false;
    }

    return true;
}

int iccmsg_pack(uint8_t *dest, 
                const uint8_t *orig_data, 
                uint32_t orig_len, 
                const frame_conf_t *fc,
                iccmsg_data_type_t data_type)
{
    uint32_t idx = 0;
    uint32_t calc_checksum = 0;
    uint16_t data_len = 0;

    if(!dest || !fc) return -1;
    if(data_type.type != DATA_TYPE_ACK){
        if(!orig_data || orig_len == 0) return -1;
        data_len = orig_len + sizeof(iccmsg_data_type_t);
    }

    //处理帧头
    memcpy(dest, fc->frame_head, fc->head_len);
    idx += fc->head_len;

    // 处理data_len
    if(is_little_endian()){
        STORE_LE16(&dest[idx], data_len);
    }
    else{
        STORE_BE16(&dest[idx], data_len);
    }
    idx += 2;

    // 处理data_type
    memcpy(&dest[idx], (uint8_t *)&data_type, sizeof(iccmsg_data_type_t));
    idx +=sizeof(iccmsg_data_type_t);

    // 除了ACK，处理原始数据
    if(data_type.type != DATA_TYPE_ACK){
        
        memcpy(&dest[idx], orig_data, orig_len);
        idx += orig_len;
    }
    // crc32校验
    if (fc->checksum_type == CRC32) {
        calc_checksum = sd_crc32(0, &dest[fc->head_len+2], data_len);
    } else if (fc->checksum_type == SUM) {
        for (uint32_t i = 0; i < (data_len); i++) {
            calc_checksum += dest[fc->head_len+2+i];
        }
    }
    if(is_little_endian()){
        STORE_LE32(&dest[idx], calc_checksum);
    }
    else{
        STORE_BE32(&dest[idx], calc_checksum);
    }
    idx += 4;

    //处理帧尾
    memcpy(&dest[idx], fc->frame_tail, fc->tail_len);
    idx += fc->tail_len;

    return idx; //返回帧长度
}

// 传入trace_ctx 实例进行初始化，num为缓存消息数量
int iccmsg_cache_init(iccmsg_cache_ctx_t *ctx, uint32_t num)
{
    // 初始化实例空间
    if (NULL == ctx && !num){
        ssdk_printf(SSDK_ERR,"eiccmsg_cach_init input unavailable param.\n");
        return -1;
    } 
    memset(ctx, 0x00, sizeof(iccmsg_cache_ctx_t));
    // 新增异步锁
    ctx->lock = osMutexNew(NULL);
    // 初始化消息链表
    if(false == list_in_list(&ctx->list)){
        list_initialize(&ctx->list);
    }
    //链表限制长度
    ctx->max_num = num;
    return 0;
}

void iccmsg_cache_deinit(iccmsg_cache_ctx_t *ctx)
{
    iccmsg_que_msg_t *p_quemsg = NULL;
    if (!ctx) return;

    // 释放链表空间，清除所有链表节点
    if(false == list_is_empty(&ctx->list)){
        list_for_every_entry(&ctx->list,p_quemsg,iccmsg_que_msg_t,node){
            // 删除节点
            list_delete(&p_quemsg->node);
            // 释放消息内存
            if(NULL != p_quemsg->data)
                ExtFree(p_quemsg->data);
            ExtFree(p_quemsg);
        }
    }

}


iccmsg_que_msg_t * iccmsg_cache_msg_new(uint8_t *frame_buf,uint32_t len,iccmsg_direction_t dir, 
                                        iccmsg_channel_type_t ch_type, iccmsg_msg_status_t ack_stat)
{
    iccmsg_que_msg_t *p_opr = NULL;
    p_opr = ExtMalloc(sizeof(iccmsg_que_msg_t));
    if(NULL == p_opr){
        return p_opr;
    }
    memset(p_opr,0,sizeof(iccmsg_que_msg_t));

    p_opr->data = ExtMalloc(len);
    if(NULL == p_opr->data){
        ExtFree(p_opr);
        p_opr = NULL;
        return p_opr;
    }
    memset(p_opr->data,0,len);
    p_opr->timestamp1 = get_timestamp_ns(); //记录消息生成时间
    p_opr->len = len;
    p_opr->dir = dir;
    p_opr->ch_type = ch_type;
    p_opr->state = ack_stat;
    memcpy(p_opr->data,frame_buf,len);
    const char *dir_str = (p_opr->dir == ICCMSG_DIR_RX) ? "RX" : "TX";
    const char *ch_str = (p_opr->ch_type == ICCMSG_CH_IPCC) ? "IPCC" : "VIRTIO";
    ssdk_printf(SSDK_DEBUG,"iccmsg_cache_msg_new: timestamp1=%lld, len=%d, dir=%s, ch_str=%s\r\n", p_opr->timestamp1, p_opr->len, dir_str, ch_str);
    return p_opr;
}
/*
    将msg消息节点存入链表，链表条数超过ctx->max_num则先删再加
*/
int iccmsg_cache_record(iccmsg_cache_ctx_t *ctx, const iccmsg_que_msg_t *msg)
{
    iccmsg_que_msg_t *pmsg = NULL;
    if (!ctx || !msg ) {
        return -1;
    }

    if (ctx->lock) {
        osMutexAcquire(ctx->lock, osWaitForever); // 获取互斥锁
    }
    /* 链表满，删除头节点 */
    if(list_length(&ctx->list) == ctx->max_num){
        // 删除头节点
        pmsg = list_remove_head_type(&ctx->list, iccmsg_que_msg_t, node);
        ssdk_printf(SSDK_DEBUG,"[CACHE] evict: ctx=%p, pmsg=%p, pmsg->data=%p, pmsg->id=%d, pmsg->dir=%d\r\n",
            ctx, (void*)pmsg, (void*)pmsg->data, pmsg->id, pmsg->dir);
        // 内存回收
        if(pmsg) {
            if(pmsg->data) {
                ssdk_printf(SSDK_DEBUG,"[CACHE] ExtFree data=%p\r\n", (void*)pmsg->data);
                ExtFree(pmsg->data);
                pmsg->data = NULL;
            }
            ssdk_printf(SSDK_DEBUG,"[CACHE] ExtFree pmsg=%p\r\n", (void*)pmsg);
            ExtFree(pmsg);
            pmsg = NULL;
        } else {
            ssdk_printf(SSDK_ERR,"[CACHE] BUG: list_length==max_num but remove_head returned NULL!\r\n");
        }
    }
    //尾端增加msg节点
    list_add_tail(&ctx->list,&msg->node);
    ctx->last_msg = msg;
    if (ctx->lock) {
        osMutexRelease(ctx->lock);
    }
    ssdk_printf(SSDK_DEBUG,"iccmsg_cache_record msg[%d], total %d msgs, ctx=%p.\r\n",msg->id,list_length(&ctx->list),ctx);
    return 0;
}

/**
 * @brief 打印最近 N 条消息
 */
void iccmsg_trace_dump(iccmsg_cache_ctx_t *ctx, uint32_t n)
{
    if(NULL == ctx->last_msg)return ;
    iccmsg_que_msg_t *pmsg = NULL;
    uint32_t i,j;
    uint32_t num = n;

    if (!ctx || n == 0) return;

    if (ctx->lock) {
        osMutexAcquire(ctx->lock, osWaitForever); // 获取互斥锁
    }
    num = list_length(&ctx->list);
    ssdk_printf(SSDK_DEBUG,"there's %d ctx.", num);
    if(num > n) num = n;
    for (i = 0; i < num; i++) {
        if(i == 0)
            pmsg = ctx->last_msg;
        else
            pmsg = list_prev_type(&ctx->list, &pmsg->node, iccmsg_que_msg_t, node);
        if(NULL == pmsg){
            ssdk_printf(SSDK_ERR,"ctx prev get null\n");
            break;
        }else{
            const char *dir_str = (pmsg->dir == ICCMSG_DIR_RX) ? "RX" : "TX";
            const char *ch_str = (pmsg->ch_type == ICCMSG_CH_IPCC) ? "IPCC" : "VIRTIO";
            ssdk_printf(SSDK_INFO,"[%llu] %s %s len=%u: ", pmsg->timestamp1, dir_str, ch_str, pmsg->len);
            for (j = 0; j < pmsg->len; j++) {
                ssdk_printf(SSDK_INFO,"%02x ", pmsg->data[j]);
            }
            ssdk_printf(SSDK_INFO,"\n");
        }
    }
    if (ctx->lock) {
        osMutexRelease(ctx->lock);
    }
}
