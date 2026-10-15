#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <rpmsg.h>
#include "CLI_type.h"
#include <rpmsg_service.h>
#include <rpmsg_channel.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include "iccmsg.h"
#include "iccmsg_endian.h"

#include <sd_errcode.h>


extern size_t ExtxPortGetFreeHeapSize( void );
extern size_t xPortGetFreeHeapSize( void );
static struct list_node iccmsg_list;
static osMutexId_t iccmsg_mutex = NULL;

/**
 * @brief iccmsg_ipcc_cb
 * 
 * ipcc 数据接收处理回调，本函数将接收协议帧存入缓冲区，再进行协议帧校验，根据frame data type 处理不同类型的协议帧
 * DATA_TYPE_ACK：处理ACK帧，需要发送ACK接收事件通知到数据发送线程，关闭重发循环。
 * DATA_TYPE_SINGLE_FRAME：处理单帧数据，深度复制app数据，通过rx_queue发送至app
 * 根据data type 内的 ackflag 判断是否需要同步发送ACK帧
 * @param data 回调传入数据指针，动态内存，生命周期未知。
 * @param len 传入数据长度
 * @param src 传入节点原地址，暂未使用
 * @param arg 传入参数，注册时已填入iccsmg实例指针
 * @return 0: 成功, -1: 失败
 * 
 * @note 函数执行过程中如果任何一步失败，会清理已创建的资源并返回-1
 */
static int iccmsg_ipcc_cb(void *data, int len, unsigned long src, void *arg)
{
    iccmsg_data_type_t data_type;
    iccmsg_t *iccmsg = (iccmsg_t *)arg;
    iccmsg_que_msg_t *rx_cache_msg;
    iccmsg_rx_msg_t *rx_msg;
    uint32_t opr_pos;
    uint8_t * p_opr = NULL;
    if (!iccmsg || !data || len <= 0 || len > ICCMSG_FRAME_MAX_LEN)
        return -1;
    p_opr = (uint8_t *)data;
    ssdk_printf(SSDK_DEBUG, "iccmsg_ipcc_cb rcv: %d.\n",len);
    //receive msg record
    rx_cache_msg = iccmsg_cache_msg_new(p_opr, len, ICCMSG_DIR_RX, ICCMSG_CH_IPCC, ICCMSG_RX_RECORD);
    if(NULL == rx_cache_msg){
        ssdk_printf(SSDK_WARNING,"iccmsg_cache_msg_new failed, skip cache.\n");
        // NOTE: continue without cache, don't crash on NULL deref below
    }
    else{
        rx_cache_msg->timestamp2 = rx_cache_msg->timestamp1;
        iccmsg_cache_record(&iccmsg->rx_ctx, rx_cache_msg);
    }

    // verify msg
    if (false == iccmsg_verify(p_opr, len, &iccmsg->frame_cfg)){
        ssdk_printf(SSDK_ERR,"iccmsg_verify failed");
        return -1;
    }
    if(rx_cache_msg) {
        rx_cache_msg->state = ICCMSG_RX_VERIFIED;
    }

    //get data type with memory copy
    opr_pos = get_frm_data_pos(&iccmsg->frame_cfg);
    ssdk_printf(SSDK_DEBUG, "opr write addr = %d,read addr = %d.\n",(uint32_t)&data_type,(uint32_t)&p_opr[opr_pos]);
    memcpy(&data_type,(iccmsg_data_type_t *)&p_opr[opr_pos],sizeof(iccmsg_data_type_t));
    if(rx_cache_msg) {
        rx_cache_msg->id = data_type.id;
    }
    //get origin data pos
    opr_pos = get_origin_data_pos(&iccmsg->frame_cfg);
    switch(data_type.type){
        case DATA_TYPE_ACK:
            ssdk_printf(SSDK_WARNING, "ipcc ACK: id=%d, waiting=%d, wait_id=%d.\n", data_type.id, iccmsg->ack_waiting, iccmsg->ack_wait_id);
            if(iccmsg->ack_waiting && data_type.id == iccmsg->ack_wait_id) {
                iccmsg->ack_waiting = false;
                osEventFlagsSet(iccmsg->event_id, ICCMSG_ACK_EVENT);
            }
            return 0;
        case DATA_TYPE_SINGLE_FRAME:
            ssdk_printf(SSDK_DEBUG, "iccmsg_ipcc_cb: DATA_TYPE_SINGLE_FRAME.\n");
            //create rx msg ,need destory by app
            rx_msg = ExtMalloc(sizeof(iccmsg_rx_msg_t));
            if(NULL == rx_msg){
                ssdk_printf(SSDK_ERR, "ExtMalloc failed.\n");
                return -1;
            }
            memset(rx_msg, 0x00, sizeof(iccmsg_rx_msg_t));
            rx_msg->len = data_type.len;
            rx_msg->data = ExtMalloc(data_type.len);
            if(NULL == rx_msg->data){
                ssdk_printf(SSDK_ERR, "ExtMalloc failed.\n");
                ExtFree(rx_msg);
                return -1;
            }
            ssdk_printf(SSDK_DEBUG, "iccmsg_ipcc_cb: memcpy.\n");
            // copy origin data to rx msg data space
            memcpy(rx_msg->data,&p_opr[opr_pos],data_type.len);

            ssdk_printf(SSDK_DEBUG, "iccmsg_ipcc_cb: msgqueue put.\n");
            // send by msg queue to app
            if(osMessageQueuePut(iccmsg->rx_queue, &rx_msg, 0, 0) == osOK) {
            iccmsg->stats.rx_cnt++;
                if(rx_cache_msg) {
                    if(data_type.ack_flag == ICCMSG_ACK_NEED)
                        rx_cache_msg->state = ICCMSG_RX_QUE_PUT_WITH_ACK;
                    else if(data_type.ack_flag == ICCMSG_ACK_NO_NEED)
                        rx_cache_msg->state = ICCMSG_RX_QUE_PUT_NO_ACK;
                }
                break;
            }
            else{
                ssdk_printf(SSDK_CRIT, "osMessageQueuePut failed, drop rx msg.\n");
                ExtFree(rx_msg->data);
                ExtFree(rx_msg);
                return -1;
            }

        break;
        default:
            ssdk_printf(SSDK_ERR, "default.\n");
        break;
    }

    if(data_type.ack_flag == ICCMSG_ACK_NEED){
        ssdk_printf(SSDK_DEBUG, "ipcc msgid[%d],send ack.\n",data_type.id);
        if(iccmsg_send(iccmsg,1,data_type.id,NULL,0,ICCMSG_ACK)){
            ssdk_printf(SSDK_ERR, "iccmsg_send ACK failed.\n");
            return -1;
        }
    }

    return 0;

}


/**
 * @brief iccmsg_virtio_cb
 * 
 * virtio 数据接收处理回调，本函数将接收协议帧存入缓冲区，再进行协议帧校验，根据frame data type 处理不同类型的协议帧
 * DATA_TYPE_ACK：处理ACK帧，需要发送ACK接收事件通知到数据发送线程，关闭重发循环。
 * DATA_TYPE_MULTI_FRAME_HEAD：保存多帧帧头信息，创建多帧动态内存,用于保存多帧数据。
 * DATA_TYPE_MULTI_FRAME: 处理多帧数据，将多帧数据存入动态内存组成完整帧，使用帧头信息进行校验后将完整帧通过rx_queue发送至app
 * 根据data type 内的 ackflag 判断是否需要同步发送ACK帧
 * @param data 回调传入数据指针，动态内存，生命周期未知。
 * @param len 传入数据长度
 * @param src 传入节点原地址，暂未使用
 * @param arg 传入参数，注册时已填入iccsmg实例指针
 * @return 0: 成功, -1: 失败
 * 
 * @note virtio 仅处理多帧帧头与多帧数据，多帧数据在此合包后发送至app
 */
static int iccmsg_virtio_cb(void *data, int len, unsigned long src, void *arg)
{
    iccmsg_t *iccmsg = (iccmsg_t *)arg;
    iccmsg_reassembly_ctx_t *ra = &iccmsg->reassembly;

    iccmsg_data_type_t data_type;
    iccmsg_que_msg_t *rx_cache_msg;
    uint32_t opr_pos;
    uint32_t checksum;
    uint8_t * p_opr = NULL;
    if (!iccmsg || !data || len <= 0 || len > ICCMSG_FRAME_MAX_LEN)
        return -1;
    p_opr = (uint8_t *)data;
    ssdk_printf(SSDK_DEBUG, "virtio rcv: %d.\n",len);
    //receive msg record
    rx_cache_msg = iccmsg_cache_msg_new(p_opr, len, ICCMSG_DIR_RX, ICCMSG_CH_VIRTIO, ICCMSG_RX_RECORD);
    if(NULL == rx_cache_msg){
        ssdk_printf(SSDK_WARNING,"iccmsg_cache_msg_new failed, skip cache.\n");
        // NOTE: continue without cache, don't crash on NULL deref below
    }
    else{
        rx_cache_msg->timestamp2 = rx_cache_msg->timestamp1;
        iccmsg_cache_record(&iccmsg->rx_ctx, rx_cache_msg);
    }

    // verify msg
    if (false == iccmsg_verify(p_opr, len, &iccmsg->frame_cfg)){
        ssdk_printf(SSDK_ERR,"iccmsg_verify failed");
        return -1;
    }
    if(rx_cache_msg) {
        rx_cache_msg->state = ICCMSG_RX_VERIFIED;
    }

    //get data type with memory copy
    opr_pos = get_frm_data_pos(&iccmsg->frame_cfg);
    ssdk_printf(SSDK_DEBUG, "opr write addr = %d,read addr = %d.\n",(uint32_t)&data_type,(uint32_t)&p_opr[opr_pos]);
    memcpy(&data_type,(iccmsg_data_type_t *)&p_opr[opr_pos],sizeof(iccmsg_data_type_t));
    if(rx_cache_msg) {
        rx_cache_msg->id = data_type.id;
    }
    //get origin data pos
    opr_pos = get_origin_data_pos(&iccmsg->frame_cfg);
    switch(data_type.type){
        case DATA_TYPE_ACK:
            if(iccmsg->ack_waiting && data_type.id == iccmsg->ack_wait_id) {
                iccmsg->ack_waiting = false;
                osEventFlagsSet(iccmsg->event_id, ICCMSG_ACK_EVENT);
            }
            return 0;

        case DATA_TYPE_MULTI_FRAME_HEAD:
            // get multi frame head data info with memory copy
            memset(&ra->info, 0x00, sizeof(iccmsg_data_info_t));
            memcpy(&ra->info, (iccmsg_data_info_t *)&p_opr[opr_pos], data_type.len);
            ssdk_printf(SSDK_DEBUG, "opr_pos =%d, data_info.len =%d,data_info.crc32 = 0x%x .\n",opr_pos, ra->info.len, ra->info.crc32);

            if(NULL == ra->msg){
                // create multi_msg 
                ssdk_printf(SSDK_DEBUG, "iccmsg_virtio_cb before requiring multi_msg,FreeHeapSize = %d.\n", ExtxPortGetFreeHeapSize());
                ra->msg = ExtMalloc(sizeof(iccmsg_rx_msg_t));
                if(NULL == ra->msg) {
                    ssdk_printf(SSDK_ERR, "error.require multi msg memory space.\n");
                    return -1;
                }
                ssdk_printf(SSDK_DEBUG, "iccmsg_virtio_cb required multi_msg's addr = 0x%x,FreeHeapSize = %d.\n", \
                                        (uint32_t)ra->msg, ExtxPortGetFreeHeapSize());
            }
            else {
                // destroy old multi_msg->data
                if(NULL != ra->msg->data){
                    ssdk_printf(SSDK_DEBUG, "release old multi msg data memory space.\n");
                    ExtFree(ra->msg->data);
                    ra->msg->data = NULL;
                }
            }
            
            memset(ra->msg,0x00,sizeof(iccmsg_rx_msg_t));
            ra->msg->len = ra->info.len;
            ssdk_printf(SSDK_DEBUG, "iccmsg_virtio_cb id before requiring multi_msg->data,FreeHeapSize = %d.\n", ExtxPortGetFreeHeapSize());
            ra->msg->data = ExtMalloc(ra->info.len);
            if(NULL == ra->msg->data){
                ssdk_printf(SSDK_ERR, "ExtMalloc failed.\n");
                return -1;
            }
            ssdk_printf(SSDK_DEBUG, "iccmsg_virtio_cb required multi_msg->data's addr = 0x%x,FreeHeapSize = %d.\n", \
                                        (uint32_t)ra->msg->data, ExtxPortGetFreeHeapSize());
            memset(ra->msg->data,0x00,ra->info.len);
            // init multi msg param
            ra->id = data_type.id;
            ra->bytes = 0;
            ra->active = true;

        break;

        case DATA_TYPE_MULTI_FRAME:
            // check whether there is multi msg
            if(0 == ra->msg->len || NULL == ra->msg->data){
                ssdk_printf(SSDK_ERR, "multi msg seems not receive frame head .\n");
                return -1;
            }

            // check msg id
            if(data_type.id == ra->id) {
                ssdk_printf(SSDK_DEBUG, "copy data: multi_cnt = 0x%d, opr_pos = %d, data_type.len = %d\n",ra->bytes,opr_pos, data_type.len);
                //copy origin data to multi_msg->data
                memcpy(&ra->msg->data[ra->bytes], &p_opr[opr_pos],data_type.len);

                //record multi frame received progress
                ra->bytes += data_type.len;
                //get all multi frame
                if(ra->bytes == ra->info.len){
                    //verify multi frame checksum
                    checksum = sd_crc32(0, ra->msg->data, ra->bytes);
                    if(ra->info.crc32 == checksum) {
                        ssdk_printf(SSDK_DEBUG, "multi frame id=%d,size=%d,verified success.\n",ra->id, ra->bytes);
                        if(osMessageQueuePut(iccmsg->rx_queue, &ra->msg, 0, 0) == osOK) {
                            //TBD: update all multi frame record state to ICCMSG_RX_QUE_PUT
                            //now only update last frame state
                            if(rx_cache_msg) {
                                if(data_type.ack_flag == ICCMSG_ACK_NEED)
                                    rx_cache_msg->state = ICCMSG_RX_QUE_PUT_WITH_ACK;
                                else if(data_type.ack_flag == ICCMSG_ACK_NO_NEED)
                                    rx_cache_msg->state = ICCMSG_RX_QUE_PUT_NO_ACK;
                            }
                            ra->msg = NULL;
                            ra->active = false;
                            break;
                        }
                        else{
                            ssdk_printf(SSDK_ERR, "multi msg send queue msg failed.\n");
                            goto multi_exit;
                        }
                    }
                    else {
                        ssdk_printf(SSDK_ERR, "multi msg checksum faild: calc val = 0x%x != rcv val = 0x%x.\n",ra->info.crc32,checksum);
                        for(int i = 0; i < ra->bytes; i ++){
                            ssdk_printf(SSDK_DEBUG, "[%d]%x ",i,ra->msg->data[i]);
                        }
                        ssdk_printf(SSDK_DEBUG, "\n");
                        ssdk_printf(SSDK_DEBUG, "cat received ctx data :\n");
                        iccmsg_trace_dump(&iccmsg->rx_ctx, ra->info.pack_num);
                        goto multi_exit;
                    }
                }
            }
            else {
                ssdk_printf(SSDK_ERR, "id wrong, multi head id = %d, frag id = %d.\n",ra->id,data_type.id);
                goto multi_exit;
            }
            
        break;
        default:
            ssdk_printf(SSDK_ERR, "default.\n");
        break;
    }

    if(data_type.ack_flag == ICCMSG_ACK_NEED){
        ssdk_printf(SSDK_DEBUG, "virtio msgid[%d],send ack.\n",data_type.id);
        //异步回复ACK
        if(iccmsg_send(iccmsg,0,data_type.id,NULL,0,ICCMSG_ACK)){
            ssdk_printf(SSDK_ERR, "iccmsg_send ACK failed.\n");
            return -1;
        }
    }
    return 0;

multi_exit:
    if(ra->msg) {
        if(ra->msg->data) { ExtFree(ra->msg->data); ra->msg->data = NULL; }
        ExtFree(ra->msg);
        ra->msg = NULL;
    }
    memset(ra, 0, sizeof(iccmsg_reassembly_ctx_t));
    iccmsg->stats.crc_fail_cnt++;
    return -1;
}



// sec域的ept使用偶数,ap域的ept使用奇数，默认绑定关系如下:
// sec ept = x;
// ap ept = x+1;
static uint16_t get_iccmsg_ept(void){
    iccmsg_t *tmp_iccmsg;
    uint16_t cnt = 0,f_found = 0;
    for(cnt = ICCMSG_EPT_BASE_NUM; cnt < ICC_EPT_NUM_MAX; cnt+=2){
        f_found = 0;
        list_for_every_entry(&iccmsg_list, tmp_iccmsg, iccmsg_t, node){
            if(cnt == tmp_iccmsg->ch_ept){
                f_found = 1;
                break;
            }    
        }
        if(!f_found)
            return cnt;
    }
    if(cnt == ICC_EPT_NUM_MAX)
        return -1;
    return 0;
}

/**
 * 检查指定名称的iccmsg是否存在
 * @param name 要检查的iccmsg名称
 * @return 存在返回true，否则返回false
 */
iccmsg_t *find_iccmsg(const char *name){
    iccmsg_t *tmp_iccmsg;
    if(NULL == name || get_iccmsg_num()<=0)
        return NULL;

    list_for_every_entry(&iccmsg_list, tmp_iccmsg, iccmsg_t, node){
        if(0 == strcmp(tmp_iccmsg->name, name))
            return tmp_iccmsg;
    }
    
    return NULL;
}

int iccmsg_init(void){
    if(iccmsg_mutex == NULL){
        osKernelLock();
        if(iccmsg_mutex == NULL){
            iccmsg_mutex = osMutexNew(NULL);
        }
        osKernelUnlock();
    }
    if(false == list_in_list(&iccmsg_list)){
        list_initialize(&iccmsg_list);
    }
    return 0;
}

static void iccmsg_tx_task(void *arg)
{
    iccmsg_t *iccmsg = (iccmsg_t *)arg;
    iccmsg_que_msg_t *msg = NULL;
    int ret,i;
    uint32_t flags;
    while (1) {
        if (osMessageQueueGet(iccmsg->tx_queue, &msg, NULL, osWaitForever) != osOK)
            continue;
        if (!msg) continue;
        ssdk_printf(SSDK_DEBUG, "[iccmsg_tx_task][1]: got msg to send\n");
        for(i = 0; i < ICCMSG_TX_REPEAT_COUNT ; i++){
            if(ICCMSG_CH_IPCC == msg->ch_type){
                ret = rpmsg_channel_send(iccmsg->ipcc_ch, iccmsg->ch_ept+1, msg->data, msg->len, 10);
            }
            else{
                ret = rpmsg_channel_send(iccmsg->virtio_ch, iccmsg->ch_ept+1, msg->data, msg->len, 10);
            }

            if(ret != RPMSG_SUCCESS){
            ssdk_printf(SSDK_CRIT, "[tx_task] ch=%s ret=%d\n",(msg->ch_type == ICCMSG_CH_IPCC) ? "IPCC" : "VIRTIO", ret);
            ssdk_printf(SSDK_CRIT, "iccmsg_tx_task: send data error, ret=%d\n",ret);
            ssdk_printf(SSDK_CRIT, "%s: alloc fail  freeheap=%u exheap=%u\n",__func__,
            (unsigned)xPortGetFreeHeapSize(),
            (unsigned)ExtxPortGetFreeHeapSize()); 
                continue;   //发送失败则不再等待ACK
            }
            ssdk_printf(SSDK_DEBUG, "[iccmsg_tx_task][2]: send data length=%d\n",msg->len);

            // 需要等待ACK
            if(msg->state == ICCMSG_ACK_WAIT){
                iccmsg->ack_waiting = true;
                iccmsg->ack_wait_id = msg->id;
                msg->state = ICCMSG_ACK_WAIT;  // already ACK_WAIT, no-op
            }
            else if(msg->state == ICCMSG_ACK_NO_NEED || msg->state == ICCMSG_ACK || msg->state == ICCMSG_ASYNC_SEND_NOACK || msg->state == ICCMSG_CACHE_CREATE){
                msg->state = ICCMSG_SEND_FINISHED;
                break;
            }

            /* 等待 ACK接收事件，超时时间 500ms*/
            flags = osEventFlagsWait(iccmsg->event_id, ICCMSG_ACK_EVENT, osFlagsWaitAll, ICCMSG_TX_TIMEOUT_MS);
            if ((flags & ICCMSG_ACK_EVENT) == ICCMSG_ACK_EVENT) {   
                msg->state = ICCMSG_ACK_GOT;  // 收到 ACK
                ssdk_printf(SSDK_DEBUG,"async send ack received.\n");
                break;
            }
            ssdk_printf(SSDK_DEBUG, "[iccmsg_tx_task][3]: waiting ack...\n");
        }
        /* 超时未收到ACK */
        if(msg->state == ICCMSG_ACK_WAIT){
            msg->state = ICCMSG_ACK_TIMEOUT;
            ssdk_printf(SSDK_WARNING,"async send wait ack timeout.\n");
            iccmsg->stats.ack_timeout_cnt++;
        }

        /* 异步消息发送结束(成功/失败/超时)后由tx_task释放。
         * 原实现: 同一msg指针既入tx_queue又进trace缓存,缓存满256条驱逐时
         * ExtFree了仍在队列中的消息;被释放块复用后msg->data/len变垃圾,
         * rpmsg_channel_send内部memcpy读非法地址引发data abort(跑死根因)。
         * 现在异步tx消息由tx_task独占所有权,不再进trace缓存。 */
        if (msg->data) {
            ExtFree(msg->data);
            msg->data = NULL;
        }
        ExtFree(msg);
        msg = NULL;

        /* 记录发送数据，并保存ACK状态
        ssdk_printf(SSDK_INFO, "[iccmsg_tx_task][4]: record tx msg.\n");
        msg->timestamp2 = get_timestamp_ns();
        if(iccmsg_cache_record(&iccmsg->tx_ctx, msg)){
            ssdk_printf(SSDK_ERR, "iccmsg_tx_task: cache record failed!\n");
        }
        */

    }
}


/**
 * @brief iccmsg_ch_init
 * 
 * 该函数用于初始化ICC（Inter-Chip Communication）通信通道，包括创建和启动
 * IPCC和VIRTIO两种类型的RPMSG通道。函数会获取对应的RPMSG实例，创建通道，
 * 并启动通道以准备通信。
 * 
 * @param iccmsg 初始化iccsmg实例
 * @return true: 成功, false: 失败
 * 
 * @note iccmsg_ch_init 前需要使用iccmsg_add()新建iccmsg实例
 */
static int iccmsg_ch_init(iccmsg_t *iccmsg)
{
    struct rpmsg_instance *rpmsg_dev;
    int ret;
    /* get a ipcc rpmsg instance */
    rpmsg_dev = rpmsg_get_instance(RPMSG_TYPE_IPCC, MBOX_RPROC_AP1);
    if (rpmsg_dev == NULL) {
        ssdk_printf(SSDK_CRIT,"%s: get %d:%d rpmsg device fail\n", __func__, RPMSG_TYPE_IPCC, MBOX_RPROC_AP1);
        return false;
    }

    /* create a ipcc rpmsg channel */
    iccmsg->ipcc_ch = rpmsg_channel_create(rpmsg_dev, iccmsg->name, iccmsg->ch_ept);

    if (iccmsg->ipcc_ch == NULL) {
        ssdk_printf(SSDK_CRIT,"%s:ipcc rpmsg_channel_create fail\n", __func__);
        return false;
    }

    /* get a virtio rpmsg instance */
    rpmsg_dev = rpmsg_get_instance(RPMSG_TYPE_VIRTIO, MBOX_RPROC_AP1);
    if (rpmsg_dev == NULL) {
        ssdk_printf(SSDK_CRIT,"%s: get %d:%d rpmsg device fail\n", __func__, RPMSG_TYPE_VIRTIO, MBOX_RPROC_AP1);
        return false;
    }

    /* create a virtio rpmsg channel */
    iccmsg->virtio_ch = rpmsg_channel_create(rpmsg_dev, iccmsg->name, iccmsg->ch_ept);

    if (iccmsg->virtio_ch == NULL) {
        ssdk_printf(SSDK_CRIT,"%s:virtio rpmsg_channel_create fail\n", __func__);
        return false;
    }

    /* start the ipcc rpmsg channel */
    ret = rpmsg_channel_start(iccmsg->ipcc_ch, iccmsg_ipcc_cb, iccmsg);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "iccmsg_ch_init: start ipcc rpmsg channel failed.\n");
        rpmsg_channel_destroy(iccmsg->ipcc_ch);
        return false;
    }

    /* start the virtio rpmsg channel */
    ret = rpmsg_channel_start(iccmsg->virtio_ch, iccmsg_virtio_cb, iccmsg);
    if (ret != 0) {
        ssdk_printf(SSDK_CRIT, "iccmsg_ch_init: start virtio rpmsg channel failed.\n");
        rpmsg_channel_destroy(iccmsg->virtio_ch);
        rpmsg_channel_destroy(iccmsg->ipcc_ch);   /* 回滚已成功的 IPCC 通道，避免泄漏 (oracle P2) */
        return false;
    }
    return true;
}

/**
 * @brief iccmsg_add
 * 
 * 传入app名与协议帧配置，创建iccmsg实例，可选配同步回调函数cb与回调函数传参ctx。
 * 创建iccmsg实例成功后，iccmsg实例会加入全局iccmsg链表，并返回iccmsg指针。
 * 
 * @param name 32字符串，用于标识iccmsg实例
 * @param conf 协议帧
 * @param cb app同步回调函数
 * @param ctx app同步回调函数传参
 * @return !NULL: 成功, NULL: 失败; 
 * 
 * @note 
 */
iccmsg_t *iccmsg_add(const char *name, frame_conf_t conf, iccmsg_rx_cb_t cb, void *ctx){
    iccmsg_t *iccmsg = NULL;
    uint16_t ept;
    bool locked = false;
    // 参数校验
    if(get_iccmsg_num() >= ICC_CHANNELS_MAX_NUM){
        ssdk_printf(SSDK_ERR, "iccmsg_add: iccmsgs pool is full.\n");
        return NULL;
    }
    if(NULL == name || 0 == strlen(name)){
        ssdk_printf(SSDK_ERR, "iccmsg_add: name is null.\n");
        return NULL;
    }
    if(find_iccmsg(name)){
        ssdk_printf(SSDK_ERR, "iccmsg_add: app %s already create iccmsg.\n",name);
        return NULL;
    }
    // 创建iccmsg 实例
    iccmsg = (iccmsg_t *)ExtMalloc(sizeof(iccmsg_t));
    if(NULL == iccmsg){
        ssdk_printf(SSDK_ERR, "iccmsg_add: iccmsg malloc error.\n");
        return NULL;
    }
    memset(iccmsg, 0x00, sizeof(iccmsg_t));
    iccmsg->tx_task_priority = osPriorityNormal; // 可配置：建议高于业务线程(25)以减少队列积压
    iccmsg->tx_task_stack_size = 4096;

    // 参数传递
    strncpy(iccmsg->name, name, sizeof(iccmsg->name) - 1);
    iccmsg->name[sizeof(iccmsg->name) - 1] = '\0';

    /* get_iccmsg_ept() 与 list_add_before() 之间必须原子, 否则并发 init
     * (sys_monitor 与 BMS) 会取到同一空闲 ept, 出现 sysmgr/cell 同占 500。 */
    if(iccmsg_mutex != NULL){
        osMutexAcquire(iccmsg_mutex, osWaitForever);
        locked = true;
    }

    // 配置通道号
    ept = get_iccmsg_ept();
    if(-1 == ept){
        ssdk_printf(SSDK_ERR, "iccmsg_add: can not get available ept.\n");
        goto exit;
    }
    iccmsg->ch_ept = ept;

    // 创建rpmsg 通道
    if(false == iccmsg_ch_init(iccmsg)){
        ssdk_printf(SSDK_ERR, "iccmsg_add: iccmsg_ch_init failed.\n");
        goto exit;
    }
    // 配置协议帧
    if(iccmsg_frm_cfg(&iccmsg->frame_cfg, 
                        conf.frame_head, 
                        conf.head_len, 
                        conf.frame_tail, 
                        conf.tail_len, 
                        conf.checksum_type))
    {
        ssdk_printf(SSDK_ERR, "iccmsg_frm_cfg failed.\n");
        goto exit;
    }

    // 消息队列初始化
    iccmsg->rx_queue = osMessageQueueNew(ICCMSG_RX_QUE_SIZE,sizeof(iccmsg_rx_msg_t *),NULL);
    if(iccmsg->rx_queue == NULL)
    {
        ssdk_printf(SSDK_ERR, "osMessageQueueNew iccmsg->rx_queue failed.\n");
        goto exit;
    }
    iccmsg->tx_queue = osMessageQueueNew(ICCMSG_TX_QUE_SIZE,sizeof(iccmsg_que_msg_t *),NULL);
    if(iccmsg->tx_queue == NULL)
    {
        ssdk_printf(SSDK_ERR, "osMessageQueueNew iccmsg->tx_queue failed.\n");
        goto exit;
    }

    // 事件初始化
    iccmsg->event_id = osEventFlagsNew(NULL);
    if(iccmsg->event_id == NULL){
        ssdk_printf(SSDK_ERR, "osEventFlagsNew iccmsg->event_id failed.\n");
        goto exit;
    }

    // 配置app同步接收回调，NULL则无同步接收功能
    iccmsg->ipcc_rx_cb = cb;
    iccmsg->user_ctx = ctx;

    // 缓存模块初始化，默认支持 ICCMSG_DEFAULT_CACHE_SIZE 条消息缓存在内存中
    iccmsg_cache_init(&iccmsg->rx_ctx,ICCMSG_DEFAULT_CACHE_SIZE);
    iccmsg_cache_init(&iccmsg->tx_ctx,ICCMSG_DEFAULT_CACHE_SIZE);

    // iccmsg 新节点添加至链表
    list_add_before(&iccmsg_list, &iccmsg->node);

    if(locked){
        osMutexRelease(iccmsg_mutex);
        locked = false;
    }

    /* 诊断: 打印本次通道实际分配到的端点号 + 当前已注册通道总数。
     * ept 由 get_iccmsg_ept() 从 ICCMSG_EPT_BASE_NUM(500) 起按首个空闲偶数分配,
     * 非固定值——可据此定位是哪些通道先占了 500~518, 导致 cell 顺延。 */
    printf("iccmsg_create: name=%-12s ept=%u total=%d\n",
           iccmsg->name, (unsigned)iccmsg->ch_ept, get_iccmsg_num());

    // 启动自动发送任务,若失败则只能同步发送
    osThreadAttr_t attr = {
        .name = "iccmsg_demo",
        .stack_size = 2048,
        .priority = iccmsg->tx_task_priority,
        /* 优先级可通过 iccmsg->tx_task_priority 配置，
           默认 osPriorityNormal(24)；高吞吐场景建议提升至 25，
           使 tx_task 抢占业务线程以快速排空队列。 */
    };
    iccmsg->tx_task = osThreadNew(iccmsg_tx_task, (void*)iccmsg, &attr);
    if (!iccmsg->tx_task) {
        ssdk_printf(SSDK_ERR, "%s: tx task init failed.\n", __func__);
    }
    return iccmsg;

exit:
    if(locked){
        osMutexRelease(iccmsg_mutex);
    }
    ExtFree(iccmsg);
    return NULL;
}



// 删除iccmsg实例，需要由app关闭收发任务后调用
int iccmsg_remove(iccmsg_t *iccmsg){
    iccmsg_t * p_opr = iccmsg;
    //检查传参
    if(!p_opr){
        ssdk_printf(SSDK_ERR, "%s: null pointer.\n", __func__);
        return -1;
    }
    // 销毁rpmsg通道
    if(rpmsg_channel_destroy(p_opr->ipcc_ch)){
        ssdk_printf(SSDK_ERR, "iccmsg_remove: rpmsg_channel_destroy ipcc_ch failed.\n");
        return false;
    }
    if(rpmsg_channel_destroy(p_opr->virtio_ch)){
        ssdk_printf(SSDK_ERR, "iccmsg_remove: rpmsg_channel_destroy virtio_ch failed.\n");
        return false;
    }
    // 删除事件句柄
    osEventFlagsDelete(p_opr->event_id);

    // 删除缓冲队列
    osMessageQueueDelete(p_opr->tx_queue);
    osMessageQueueDelete(p_opr->rx_queue);

    // 删除记录实例
    iccmsg_cache_deinit(&p_opr->rx_ctx);
    iccmsg_cache_deinit(&p_opr->tx_ctx);

    // 链表节点删除
    list_delete(&p_opr->node);

    // 释放内存
    ExtFree(p_opr);

    return true;
}



void iccmsg_list_all(void ){
    iccmsg_t *tmp_iccmsg;
    list_for_every_entry(&iccmsg_list, tmp_iccmsg, iccmsg_t, node){
        ssdk_printf(SSDK_INFO, "[%d]-iccmsg's name: %s .\n",tmp_iccmsg->ch_ept,tmp_iccmsg->name);
    }
}

int get_iccmsg_num(void){
    return list_length(&iccmsg_list);
}

static int iccmsg_get_frag_len(const frame_conf_t *fc){
    int fixed_len = 0,frag_len = 0;
    fixed_len = fc->head_len + ICCMSG_FRAME_DATA_LEN_LEN + ICCMSG_FRAME_CHECKSUM_LEN + sizeof(iccmsg_data_type_t) + fc->tail_len;
    frag_len = ICCMSG_FRAME_MAX_LEN - fixed_len;
    return frag_len;
}

int iccmsg_send(iccmsg_t *iccmsg, int sync_flag, uint8_t id, const void *data, uint32_t len,iccmsg_msg_status_t ack_stat)
{
    if(!iccmsg) return -1;

    if(ack_stat != ICCMSG_ACK){
        if (!data || !len ) return -1;
    }
    if(ack_stat > ICCMSG_ACK_NEED) return -1; //app 传入参数验证

    iccmsg_data_type_t data_type;
    iccmsg_data_info_t data_info = {0};
    rpmsg_channel_t ch = iccmsg->ipcc_ch;
    iccmsg_channel_type_t msg_ch= ICCMSG_CH_IPCC;
    iccmsg_msg_status_t async_msgstat = ack_stat;

    uint8_t *p_opr = (uint8_t *)data;
    uint8_t frame_buf[ICCMSG_FRAME_MAX_LEN],data_buf[ICCMSG_FRAME_MAX_LEN];
    uint32_t i = 0, frag_size = 0, last_size = 0, data_len = 0, frame_len = 0,orig_len = 0;
    int ret = 0, frag_flag = 0;

    memset(&data_type,0x00,sizeof(iccmsg_data_type_t));
    data_type.id = id;
    data_type.ack_flag = ack_stat;
    data_type.len = len;
    ssdk_printf(SSDK_DEBUG, " %s iccmsg_send msg[%d] %d bytes, ack_stat = %d.\n", (sync_flag ? "sync" : "async"), id, len, ack_stat);
    iccmsg->stats.tx_cnt++;
    if(ack_stat != ICCMSG_ACK){
        //需要分包,记录总包信息
        if(ICCMSG_FRM_MIN_LEN+len > ICCMSG_FRAME_MAX_LEN){
            frag_flag = 1;
            ch = iccmsg->virtio_ch;
            msg_ch= ICCMSG_CH_VIRTIO;

            data_info.crc32 = sd_crc32(0, p_opr, len);
            data_info.len = len;
            frag_size = iccmsg_get_frag_len(&iccmsg->frame_cfg);    //原始数据分包长度
            last_size = len%frag_size;
            data_info.pack_num = len/frag_size + (last_size? 1:0);
            data_type.type = DATA_TYPE_MULTI_FRAME_HEAD;
            data_type.index = 0;
            ssdk_printf(SSDK_DEBUG, "len = %d, data_info.pack_num = %d,frag_size = %d,last_size = %d\n",len, data_info.pack_num, frag_size, last_size);
        }
        if(frag_flag){
            // 第一帧数据，填充data_info
            memcpy(data_buf, (uint8_t *)&data_info, sizeof(iccmsg_data_info_t));
            data_len = sizeof(iccmsg_data_info_t);
        }
        else{
            // 单帧数据，直接填充data内容
            memcpy(data_buf,(uint8_t *)p_opr, len);
            data_len = len;
        }
    }
    else{
        data_len = 0;
        data_type.type = DATA_TYPE_ACK;
        data_type.ack_flag = ICCMSG_ACK_NO_NEED;
    }
    // 单帧或帧头打包
    frame_len = iccmsg_pack((uint8_t *)frame_buf, (const uint8_t *)&data_buf[0], data_len,
                        (const frame_conf_t *)&iccmsg->frame_cfg, data_type);
    if(frame_len == 0){
        return -1;
    }


    
    // 创建发送缓存消息日志
    iccmsg_que_msg_t *msg = iccmsg_cache_msg_new(frame_buf,frame_len,ICCMSG_DIR_TX,msg_ch,ack_stat);
    if(NULL == msg){
        ssdk_printf(SSDK_ERR, "iccmsg_sync_send: cache msg new failed!\n");
        return -1;
    }
    else{
        ssdk_printf(SSDK_DEBUG, "iccmsg_cache_msg_new msg[%d] success.\n",id);
        // 完善缓存信息
        msg->id = id;
        msg->state = ICCMSG_CACHE_CREATE;

        // 同步发送 若是ACK，则强制同步发送降低传输延迟
        if(sync_flag || ICCMSG_ACK == ack_stat){
            ssdk_printf(SSDK_DEBUG, "sync send msg[%d] start.\n",id);
            ret = rpmsg_channel_send(ch, iccmsg->ch_ept+1, frame_buf, frame_len, 10);
            if(ret != RPMSG_SUCCESS){
                ssdk_printf(SSDK_ERR, "iccmsg_sync_send: send data error, ret=%d\n",ret);
                return -1;
            }
            msg->timestamp2 = get_timestamp_ns();
            msg->state = ICCMSG_SEND_FINISHED;
            // 加入缓存（发送成功后记录）
            if(iccmsg_cache_record(&iccmsg->tx_ctx, msg)){
                ssdk_printf(SSDK_ERR, "iccmsg_sync_send: cache record failed!\n");
            }
        }
        else{
            ssdk_printf(SSDK_DEBUG, "async send msg[%d] start.\n",id);
            if(ack_stat == ICCMSG_ACK_NEED){
                async_msgstat = ICCMSG_ACK_WAIT;
            }
            else if (ack_stat == ICCMSG_ACK_NO_NEED){
                async_msgstat = ICCMSG_ASYNC_SEND_NOACK;
            }
            msg->state = async_msgstat;

            // 异步发送
            if(osOK != osMessageQueuePut(iccmsg->tx_queue, &msg, 0, 10)){
                ssdk_printf(SSDK_ERR, "iccmsg_async_send: tx queue full, put failed!\n");
                ExtFree(msg->data);
                msg->data = NULL;
                ExtFree(msg);
                msg = NULL;
                return -1;
            }
            ssdk_printf(SSDK_DEBUG,"async send %d byte.\n",msg->len);
            /* 异步消息不入trace缓存: 队列与缓存共享指针会导致缓存驱逐
             * 释放仍在tx_queue中的消息(use-after-free),所有权归tx_task */
        }

        // 多包发送
        if(frag_flag){
            orig_len = frag_size;
            data_type.type = DATA_TYPE_MULTI_FRAME;
            for(i = 0; i < data_info.pack_num; i++){
                //最后一包
                if(last_size && (i == data_info.pack_num-1)){
                    orig_len = last_size;
                }
                // 填充 data_type 至data_buf
                data_type.index = i;
                // 填充 原始数据 至data_buf
                memcpy(data_buf,(uint8_t *)&p_opr[i*frag_size], orig_len);
                data_len = orig_len;
                data_type.len = orig_len;
                // 打包成协议帧 存放至 frame_buf
                frame_len = iccmsg_pack((uint8_t *)frame_buf, (const uint8_t *)&data_buf[0], data_len,
                                    (const frame_conf_t *)&iccmsg->frame_cfg, data_type);
                if(frame_len == 0){
                    ssdk_printf(SSDK_ERR, "iccmsg_sync_send: send first frame failed.\n");
                    return -1;
                }
                // 发送记录
                iccmsg_que_msg_t *msg2 = iccmsg_cache_msg_new(frame_buf,frame_len,ICCMSG_DIR_TX,msg_ch,ack_stat);
                if(NULL == msg2){
                    ssdk_printf(SSDK_ERR, "iccmsg_sync_send: cache msg new failed!\n");
                    continue;
                }
                msg2->id = id;
                msg2->state = ICCMSG_CACHE_CREATE;
                if(sync_flag){
                    // 同步发送数据
                    ret = rpmsg_channel_send(ch, iccmsg->ch_ept+1, frame_buf, frame_len, 10);
                    if(ret != RPMSG_SUCCESS){
                        ssdk_printf(SSDK_CRIT, "iccmsg_sync_send: send data error, ret=%d\n",ret);
                        return -1;
                    }
                    msg2->timestamp2 = get_timestamp_ns();
                    msg2->state = ICCMSG_SEND_FINISHED;
                    // 加入缓存（发送成功后记录）
                    if(iccmsg_cache_record(&iccmsg->tx_ctx, msg2)){
                        ssdk_printf(SSDK_ERR, "iccmsg_sync_send: cache record failed!\n");
                    }
                }
                else{
                    // 异步发送
                    msg2->state = async_msgstat;
                    if(osOK != osMessageQueuePut(iccmsg->tx_queue, &msg2, 0, 10)){
                        ssdk_printf(SSDK_ERR, "iccmsg_async_send: tx queue full, put failed!\n");
                        ExtFree(msg2->data);
                        msg2->data = NULL;
                        ExtFree(msg2);
                        msg2 = NULL;
                        return -1;     
                    }
                    ssdk_printf(SSDK_DEBUG,"async send %d byte.\n",msg2->len);
                    /* 异步消息不入trace缓存(同上,防use-after-free) */
                }
            }
        }
    }

    return 0;
}
/*
int iccmsg_is_link_up(iccmsg_t *iccmsg){
    if(rpmsg_is_link_up(iccmsg->ipcc_ch) && rpmsg_is_link_up(iccmsg->virtio_ch))
        return 1;
    else
        return 0;
}
*/