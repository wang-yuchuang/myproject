#include <time.h>
#include "iccmsg_common.h"
#include "iccmsg_dpr.h"
#include "iccmsg_ll.h"
#include "iccmsg_hal.h"


static void iccmsg_rx_thread(iccmsg_hal_t* iccmsg){
    message_t msg;
    iccmsg_hal_t * p_iccmsg = iccmsg;
    uint8_t buf[ICCMSG_FRAME_MAX_LEN];
    while (1) {
        // epoll_wait 阻塞等待事件（timeout=-1 永久阻塞）
        int nfds = epoll_wait(p_iccmsg->epoll_fd, p_iccmsg->events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait failed");
            break;
        }

        // 遍历所有就绪事件
        for (int i = 0; i < nfds; i++) {
            int fd = p_iccmsg->events[i].data.fd;
            uint32_t revents = p_iccmsg->events[i].events;

            // 处理错误/断开事件
            if (revents & (EPOLLERR | EPOLLHUP)) {
                LOG_PRINT(LOG_ERR, "FD %d disconnect!\n", fd);
                del_epoll_fd(p_iccmsg->epoll_fd, fd);
                continue;
            }
            memset(buf, 0, sizeof(buf));
            // 处理读事件
            if (revents & EPOLLIN) {
                if (fd == p_iccmsg->ll.fd_ipcc) {
                    p_iccmsg->ipcc_rx_cnt++;
                    uint32_t len = iccmsg_read(p_iccmsg, ICCMSG_CH_IPCC, &p_iccmsg->dpr.fc, buf);
                    
                }
                else if(fd == p_iccmsg->ll.fd_virtio){
                    p_iccmsg->virtio_rx_cnt++;
                    uint32_t len = iccmsg_read(p_iccmsg, ICCMSG_CH_VIRTIO, &p_iccmsg->dpr.fc, buf);
                    
                }
            }
        }
    }

}

static int iccmsg_wait_ack(sem_t *ack_sem, int timeout_ms){
    struct timespec abs_timeout;
    int ret;
    if (clock_gettime(CLOCK_REALTIME, &abs_timeout) == -1)
    {
        perror("clock_gettime failed");
        pthread_exit(NULL);
    }
    abs_timeout.tv_nsec += timeout_ms * 1000000;
    if (abs_timeout.tv_nsec >= 1000000000)
    {
        abs_timeout.tv_sec += 1;  // 纳秒满10^9，进位1秒
        abs_timeout.tv_nsec -= 1000000000;  // 剩余纳秒数
    }

    ret = sem_timedwait(ack_sem, &abs_timeout);
    // 接收到信号
    if (ret == 0)
    {
        return 0;
    }
    else
    {
        // 接收超时
        if (errno == ETIMEDOUT)
        {
            return 1;
        }
        else // 接收失败
        {
            perror("sem_timedwait failed");
            return -2;
        }
    }

}


static void iccmsg_tx_thread(iccmsg_hal_t* iccmsg){
    thread_safe_queue_t *tsq = &iccmsg->tx_que;
    message_t msg;
    int ret = 0;
    int i = 0;

    while (1) {
        if (receive_message_timeout(tsq, &msg, QUEUE_TIME_OUT_MS) != 0)
            continue;
        LOG_PRINT(LOG_DEBUG, "[iccmsg_tx_task][1]: got msg to send\n");
        for(i = 0; i < msg.data_len; i ++){
            LOG_PRINT(LOG_DEBUG, "[%d]0x%x,",i,msg.data[i]);
        }
        LOG_PRINT(LOG_DEBUG, "\n");
        for(i = 0; i < ICCMSG_TX_REPEAT_COUNT ; i++){
            if(ICCMSG_CH_IPCC == msg.ch_type){
                ret = write(iccmsg->ll.fd_ipcc, msg.data, msg.data_len);
                if(ret <= 0){
                    LOG_PRINT(LOG_ERR,"write err: %d.\n", ret);
                    usleep(50);
                    continue;
                }
            }
            else{
                ret = write(iccmsg->ll.fd_virtio, msg.data, msg.data_len);
                if(ret <= 0){
                    LOG_PRINT(LOG_ERR,"write err: %d.\n", ret);
                    usleep(50);
                    continue;
                }
            }

            LOG_PRINT(LOG_DEBUG, "send data length=%d\n",msg.data_len);
            // 无需等待ACK,直接跳出重发循环
            if(msg.state == ICCMSG_ACK_NO_NEED || msg.state == ICCMSG_ACK)
                break;
            // 需要等待ACK
            if(msg.state == ICCMSG_ACK_NEED){
                //切换状态
                msg.state = ICCMSG_ACK_WAIT;
            }

            ret = iccmsg_wait_ack(&iccmsg->ack_sem,500);
            if(ret == 0){
                msg.state = ICCMSG_ACK_GOT;
                LOG_PRINT(LOG_INFO, "msg.id[%d]:got ack.\n",msg.id);
                break;
            }   
            else if(ret == 1){
                msg.state = ICCMSG_ACK_TIMEOUT;
                LOG_PRINT(LOG_WARNING, "msg.id[%d]:wait ack timeout.\n",msg.id);
            }    
        }
    }

}



/**
 *  依据所用远程ept号与协议帧配置，创建iccmsg hal 实例
 * 
*/
iccmsg_hal_t *iccmsg_hal_init(int remote_ept, frame_conf_t fc, char *app_name)
{
    //struct epoll_event events[MAX_EVENTS];
    int epoll_fd;
    int ret = 0;
    iccmsg_hal_t *hal = NULL;
    hal = malloc(sizeof(iccmsg_hal_t));
    if(hal == NULL){
        LOG_PRINT(LOG_ERR, "malloc failed\n");
        return NULL;
    }
    //TBD: 避免重复初始化remote_ept
    if(iccmsg_ll_init(&hal->ll, remote_ept, app_name)){
        LOG_PRINT(LOG_ERR, "iccmsg_ll_init failed\n");
        goto exit;
    }
    memcpy(&hal->dpr.fc,&fc,sizeof(frame_conf_t));

    // 创建epoll句柄
    epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (epoll_fd == -1) {
        LOG_PRINT(LOG_ERR, "epoll_create1 failed\n");
        iccmsg_ll_deinit(&hal->ll);
        goto exit;
    }
    
    // epoll 监听 ipcc文件描述符
    if (add_epoll_fd(epoll_fd, hal->ll.fd_ipcc, EPOLLIN | EPOLLERR | EPOLLHUP) == -1) {
        LOG_PRINT(LOG_ERR, "add ipcc fd to epoll failed");
        iccmsg_ll_deinit(&hal->ll);
        goto exit;
    }
    // epoll 监听 virtio文件描述符
    if (add_epoll_fd(epoll_fd, hal->ll.fd_virtio, EPOLLIN | EPOLLERR | EPOLLHUP) == -1) {
        LOG_PRINT(LOG_ERR, "add virtio fd to epoll failed");
        iccmsg_ll_deinit(&hal->ll);
        goto exit;
    }
    hal->epoll_fd = epoll_fd;

    // 创建收发数据处理线程
    if(pthread_create(&hal->rx_thread, NULL, iccmsg_rx_thread, hal))
    {
        LOG_PRINT(LOG_ERR, "pthread_create rx_thread failed.\n");
        iccmsg_ll_deinit(&hal->ll);
        goto exit;
    }

    if(pthread_create(&hal->tx_thread, NULL, iccmsg_tx_thread, hal))
    {
        LOG_PRINT(LOG_ERR, "pthread_create rx_thread failed.\n");
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_ipcc);
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_virtio);
        iccmsg_ll_deinit(&hal->ll);
        goto exit;
    }

    if (init_queue(&hal->rx_que, DEFAULT_QUEUE_SIZE) != 0) {
        LOG_PRINT(LOG_ERR, "Failed to initialize queue\n");
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_ipcc);
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_virtio);
        iccmsg_ll_deinit(&hal->ll);
        goto exit;
    }

    if (init_queue(&hal->tx_que, DEFAULT_QUEUE_SIZE) != 0) {
        LOG_PRINT(LOG_ERR, "Failed to initialize queue\n");
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_ipcc);
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_virtio);
        iccmsg_ll_deinit(&hal->ll);
        destroy_queue(&hal->rx_que);
        goto exit;
    }

    if (sem_init(&hal->ack_sem, 0, 1) != 0) {
        LOG_PRINT(LOG_ERR, "ack sem init failed.\n");
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_ipcc);
        del_epoll_fd(hal->epoll_fd, hal->ll.fd_virtio);
        iccmsg_ll_deinit(&hal->ll);
        destroy_queue(&hal->rx_que);
        destroy_queue(&hal->tx_que);
        goto exit;
    }

    return hal;

exit:
    if(epoll_fd > 0)
        close(epoll_fd);
    if(NULL != hal)
        free(hal);

    return NULL;
}


// 将 FD 添加到 epoll 实例，监听指定事件
int add_epoll_fd(int epoll_fd, int fd, uint32_t events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOG_PRINT(LOG_ERR, "epoll_ctl ADD failed.\n");
        return -1;
    }
    LOG_PRINT(LOG_DEBUG, "add_epoll_fd: add FD %d to epoll,watching: 0x%x\n", fd, events);
    return 0;
}

// 从 epoll 实例中删除 FD
int del_epoll_fd(int epoll_fd, int fd) {
    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL) == -1) {
        LOG_PRINT(LOG_ERR, "epoll_ctl DEL failed.\n");
        return -1;
    }
    close(fd);
    return 0;
}

int send_ack(int dev_fd,const frame_conf_t *fc){
    uint8_t buf[512];
    
    if(dev_fd < 0 || NULL == fc)
        return NULL;
    iccmsg_data_type_t data_type;
    data_type.type = DATA_TYPE_ACK;
    data_type.ack_flag = ICCMSG_ACK_NO_NEED;
    uint32_t len = iccmsg_pack(buf, NULL, 0, fc, data_type);
    int ret = write(dev_fd,buf,len);
    return ret;
}

// 处理字符设备的读事件
ssize_t iccmsg_read(iccmsg_hal_t *hal, iccmsg_channel_type_t ch, const frame_conf_t *fc, uint8_t *buf) {
    
    uint8_t tmp_buf[512];
    uint32_t pos;
    int opr_fd;
    ssize_t n;
    message_t msg;
    static uint8_t id;
    static uint32_t data_pos;
    static message_t multi_msg;
    static iccmsg_data_info_t data_info;

    if(NULL == hal || ch > ICCMSG_CH_VIRTIO || ch < ICCMSG_CH_IPCC || NULL == fc || NULL == buf)
        return -1;

    if(ch == ICCMSG_CH_IPCC)
        opr_fd = hal->ll.fd_ipcc;
    else
        opr_fd = hal->ll.fd_virtio;
    
    n = read(opr_fd, tmp_buf, sizeof(tmp_buf));
    if (n == -1) {
        LOG_PRINT(LOG_ERR, " read dev_fd failed.\n");
        return -1;
    } else if (n == 0) {
        // 字符设备无数据（部分设备如 /dev/zero 不会返回 0）
        LOG_PRINT(LOG_WARNING, " read 0 byte.\n");
        return -1;
    }
    LOG_PRINT(LOG_DEBUG, "read %d bytes:\n", n);
    for(int i = 0 ; i < n ; i++){
        LOG_PRINT(LOG_DEBUG,"%02x ", tmp_buf[i]);
    }
    LOG_PRINT(LOG_DEBUG,"\n");
    if(false == iccmsg_verify(tmp_buf, n, fc)){
        LOG_PRINT(LOG_ERR,"iccmsg verify failed.\n");
        return -1;
    }
    pos = get_frm_data_pos(fc);
    iccmsg_data_type_t *msg_dt = (iccmsg_data_type_t*)(tmp_buf+pos);
    LOG_PRINT(LOG_DEBUG,"pos = 0x%x, msg_dt.id = 0x%x, msg_dt.type = 0x%x, msg_dt.ack_flag = 0x%x, msg_dt.len = 0x%x, msg_dt.index = 0x%x, \n", \
                pos, msg_dt->id, msg_dt->type, msg_dt->ack_flag, msg_dt->len, msg_dt->index);

    pos = get_origin_data_pos(fc);

    
    msg.ch_type = ch;
    msg.state = (msg_dt->ack_flag > ICCMSG_ACK_NO_NEED)? ICCMSG_ACK_NEED:ICCMSG_ACK_NO_NEED;

    switch(msg_dt->type){
        case DATA_TYPE_ACK:
            LOG_PRINT(LOG_DEBUG,"received ack frame.\n");
            sem_post(&hal->ack_sem);
            return -1;
        break;
        case DATA_TYPE_SINGLE_FRAME:
            LOG_PRINT(LOG_DEBUG,"received single frame.\n");
            msg.data = malloc(msg_dt->len);
            if(NULL == msg.data){
                LOG_PRINT(LOG_ERR,"malloc failed.\n");
            }
            memcpy(msg.data, &tmp_buf[pos], msg_dt->len);
            gettimeofday(&msg.timestamp, NULL);
            msg.data_len = msg_dt->len;
            msg.id = msg_dt->id;
            
            if(send_message(&hal->rx_que, &msg)){
                LOG_PRINT(LOG_ERR,"iccmsg_read error.\n");
            }
            free(msg.data);

        break;

        case DATA_TYPE_MULTI_FRAME_HEAD:
            
            id = msg_dt->id;
            data_pos = 0;
            memset(&data_info,0x00,sizeof(iccmsg_data_info_t));
            iccmsg_data_info_t *di = (iccmsg_data_type_t*)(tmp_buf+pos);
            memcpy(&data_info, di, sizeof(iccmsg_data_info_t));
            LOG_PRINT(LOG_DEBUG,"iccmsg_read data info: len = %d, pack_num = %d, crc32 = 0x%x.\n", \
                                data_info.len, data_info.pack_num, data_info.crc32);
            multi_msg.data_len = di->len;
            multi_msg.id = msg_dt->id;
            multi_msg.data = malloc(di->len);
            if(NULL == multi_msg.data){
                LOG_PRINT(LOG_ERR,"iccmsg_read malloc error.\n");
                return -1;
            }

        break;

        case DATA_TYPE_MULTI_FRAME:
            if(msg_dt->id != id) {
                LOG_PRINT(LOG_ERR,"iccmsg_read multi frame id wrong.\n");
                break;
            }
            else {
                
                if(data_info.len == 0 || data_info.crc32 == 0 || data_info.pack_num == 0){
                    LOG_PRINT(LOG_ERR,"error:get multi frame before multi frame head,refused!\n");
                    return -1;
                }
                if(NULL == multi_msg.data){
                    LOG_PRINT(LOG_ERR,"error: get available multi frame head,but multi_msg.data malloc failed.\n");
                    return -1;
                }
                memcpy(&multi_msg.data[data_pos], &tmp_buf[pos], msg_dt->len);
                data_pos += msg_dt->len;
                multi_msg.data_len = data_info.len;
                LOG_PRINT(LOG_DEBUG,"iccmsg_read multi data index: %d.\n", msg_dt->index);
                
                if(data_info.len == data_pos) {
                    uint32_t calc_crc = sd_crc32(0, multi_msg.data, multi_msg.data_len);
                    if(data_info.crc32 == calc_crc) {
                        // LOG_PRINT(LOG_INFO,"iccmsg_read multi data receive finished.\n");
                        if(send_message(&hal->rx_que, &multi_msg)){
                            LOG_PRINT(LOG_ERR,"iccmsg_read error.\n");
                        }
                    }
                    else {
                        LOG_PRINT(LOG_ERR,"iccmsg_read verify error, calc_crc = 0x%x, data_info.crc32 = 0x%x\n", calc_crc, data_info.crc32);
                    }
                }

            }

        break;
        default:
            
        break;
    }

    if(msg_dt->ack_flag == ICCMSG_ACK_NEED){
        LOG_PRINT(LOG_DEBUG,"send ack.\n");
        send_ack(opr_fd,fc);
    }
    

    return msg_dt->len;
}

int iccmsg_write(iccmsg_hal_t *hal, int sync_flag, uint8_t id, const void *data, uint32_t len, iccmsg_msg_status_t ack_stat){
    
    uint32_t frm_len, frag_len, frag_num, last_len;
    uint8_t tmp_buf[512];
    int ret = 0, frag_flag = 0, i = 0;
    int opr_fd = 0;

    iccmsg_data_type_t data_type;
    iccmsg_data_info_t data_info;
    message_t msg;
    iccmsg_msg_status_t asyncmsg_stat = ack_stat;
    
    if(NULL == hal || NULL == data ){
        LOG_PRINT(LOG_ERR,"iccmsg_write: param error.\n");
        return -1;
    }
    memset(&data_type,0x00,sizeof(iccmsg_data_type_t));
    // 发送ACK
    if(ack_stat == ICCMSG_ACK){
        data_type.id = id;
        data_type.ack_flag = ICCMSG_ACK_NO_NEED;
        data_type.type = DATA_TYPE_ACK;
        frm_len = iccmsg_pack(tmp_buf, NULL, 0, &hal->dpr.fc, data_type);
    }
    else{
        //计算分包数据
        frag_len = get_frame_data_max_len(&hal->dpr.fc);
        last_len = len%frag_len;
        frag_num = len/frag_len +(last_len? 1:0);
        if(frag_num > 2)
            frag_flag = 1;

        LOG_PRINT(LOG_DEBUG,"iccmsg_write: frag_len = %d, last_len = %d, frag_num = %d.\n", frag_len, last_len, frag_num);
        data_type.id = id;
        data_type.type = DATA_TYPE_SINGLE_FRAME;
        data_type.ack_flag = ack_stat;
        //单帧数据
        if(!frag_flag){
            opr_fd = hal->ll.fd_ipcc;
            data_type.type = DATA_TYPE_SINGLE_FRAME;
            data_type.len = len;
            data_type.index = 0;
            msg.ch_type = ICCMSG_CH_IPCC;
            frm_len = iccmsg_pack(tmp_buf, (const uint8_t *)data, len, &hal->dpr.fc, data_type);
        }
        else{   // 多帧数据,打包帧头
            opr_fd = hal->ll.fd_virtio;
            data_info.len = len;
            data_info.pack_num = frag_num;
            data_info.crc32 = sd_crc32(0, data, len);
            data_type.type = DATA_TYPE_MULTI_FRAME_HEAD;
            data_type.len = sizeof(iccmsg_data_info_t);
            data_type.index = 0;
            msg.ch_type = ICCMSG_CH_VIRTIO;
            frm_len = iccmsg_pack(tmp_buf, (const uint8_t *)&data_info, sizeof(iccmsg_data_info_t), &hal->dpr.fc, data_type);
        }
    }

    if(sync_flag || ack_stat == ICCMSG_ACK){
        ret = write(opr_fd, tmp_buf, frm_len);
        if(ret <= 0){
            LOG_PRINT(LOG_ERR,"write err: %d.\n", ret);
            return ret;
        }
    }
    else{
        msg.id = id;
        msg.data_len = frm_len;
        msg.data = malloc(frm_len);
        if(NULL == msg.data){
            LOG_PRINT(LOG_ERR,"malloc failed.\n");
            return -1;
        }
        memcpy(msg.data, tmp_buf, frm_len);
        msg.state = asyncmsg_stat;
        //send_message 会深度复制消息内容,发送完成即可直接释放堆内存
        if(send_message(&hal->tx_que, &msg)) {
            LOG_PRINT(LOG_ERR,"send message failed.\n");
            free(msg.data);
            return -1;
        }   
        free(msg.data);
    }
    
    // 发送多帧数据
    if(frag_flag)
    {
        LOG_PRINT(LOG_DEBUG,"iccmsg_write: start %s sending %d frames.\n",(sync_flag)?"sync":"async", frag_num);
        data_type.type = DATA_TYPE_MULTI_FRAME;
        data_type.len = frag_len;
        
        for(i = 0; i < frag_num; i++){
            data_type.index = i;
            if(last_len && (i == frag_num-1)){
                data_type.len = last_len;
            }
            memset(tmp_buf, 0, sizeof(tmp_buf));
            frm_len = iccmsg_pack(tmp_buf, &data[i*frag_len], data_type.len, &hal->dpr.fc, data_type);

            LOG_PRINT(LOG_DEBUG,"iccmsg_write: frame index = %d, data size = %d,frame size = %d\n", \
                        data_type.index, data_type.len, frm_len);

            if(sync_flag){
                ret = write(opr_fd, tmp_buf, frm_len);
                if(ret <= 0){
                    LOG_PRINT(LOG_ERR,"write err: %d.\n", ret);
                    return ret;
                }
            }
            else{
                msg.data_len = frm_len;
                msg.data = malloc(frm_len);
                if(NULL == msg.data){
                    LOG_PRINT(LOG_ERR,"malloc failed.\n");
                    return -1;
                }
                memcpy(msg.data, tmp_buf, frm_len);
                msg.state = asyncmsg_stat;
                //send_message 会深度复制消息内容,发送完成即可直接释放堆内存
                if(send_message(&hal->tx_que, &msg)) {
                    LOG_PRINT(LOG_ERR,"send message failed.\n");
                    free(msg.data);
                    return -1;
                }
                free(msg.data);
            }

        }
    }
    return ret; //返回最后写入一帧长度
}







//此测试依赖R核作为测试服务器接收测试命令配合测试
#define ICCMSG_TEST_REMOTE_EPT      500
#define ICCMSG_TEST_REMOTE_NAME     "iccmsg_test"
#define ICCMSG_TEST_LOCAL_EPT       501
#define ICCMSG_TEST_LOCAL_NAME      "iccmsg_test_ap"
#define ICCMSG_TEST_MULTI_MSG_SIZE  (1024)
#define BUF_SIZE                    1024
#define MAX_EVENTS                  256

#define ICCMSG_TEST_NUM             10
#define ICCMSG_TEST_MSG_HEAD        0xA0
#define iccmsg_TEST_MSG_TAIL        0xA5
#define ICCMSG_RX_QUEUE_SIZE        32

#define CMD_PRINT_RECEIVE_MSG                   0   // 接收消息打印输出 server: ap  client: sec
#define CMD_SINGLE_SYNC_ECHO_NACK               1   // 单帧同步 NACK 回传测试 server: sec  client: ap
#define CMD_SINGLE_ASYNC_ECHO_ACK               2   // 单帧异步 ACK 回传测试 server: sec  client: ap
#define CMD_MULTI_SYNC_ECHO_NACK                3
#define CMD_MULTI_ASYNC_ECHO_ACK                4   // 多帧异步 ACK 回传测试 server: sec  client: ap
#define CMD_SINGLE_ASYNC_ECHO_NACK              5   // 单帧异步 NACK 回传测试 server: sec  client: ap

typedef struct {
    uint32_t cmd;       // 消息命令
    uint32_t cnt;       // 测试记录
}__attribute__((packed)) iccmsg_test_single_msg_t;

typedef struct {
    uint32_t cmd;       // 消息命令
    uint32_t cnt;       // 测试记录
    uint8_t data[ICCMSG_TEST_MULTI_MSG_SIZE];
}__attribute__((packed)) iccmsg_test_multi_msg_t;


void iccmsg_hal_test(int choice, int times)
{
    uint8_t buf[512];
    int ret;
    int len;
    struct timeval tv;
    uint64_t usec_rec = 0,max_latancy = 0,min_latancy = 0;
    __uint128_t sum_latancy = 0;
    iccmsg_test_single_msg_t msg;
    iccmsg_hal_t *hal = NULL;
    frame_conf_t fc = {
        .head_len =2,
        .tail_len =2,
        .frame_head = {ICCMSG_TEST_MSG_HEAD,ICCMSG_TEST_MSG_HEAD},
        .frame_tail = {iccmsg_TEST_MSG_TAIL,iccmsg_TEST_MSG_TAIL},
        .checksum_type = CRC32,
    };
    // 等待远端iccmsg 通道创建
    while(iccmsg_find_name(ICCMSG_TEST_REMOTE_NAME)) {
        usleep(1000000);
        LOG_PRINT(LOG_INFO,"waiting remote ept :%s.\n",ICCMSG_TEST_REMOTE_NAME);
    }
    hal = iccmsg_hal_init(ICCMSG_TEST_REMOTE_EPT, fc, ICCMSG_TEST_LOCAL_NAME);
    if(hal == NULL){
        LOG_PRINT(LOG_ERR,"iccmsg_hal_init failed.\n");
    }
    LOG_PRINT(LOG_INFO,"iccmsg_hal_init success.\n");
    // crc32 测试
    for(int i = 0; i < 512; i++){
        buf[i] = i;
    }
    LOG_PRINT(LOG_DEBUG,"cal 0~512 crc32 value = 0x%x.\n",sd_crc32(0, buf, 512));
    memset(&msg,0x00,sizeof(message_t));
    memset(&min_latancy,0xFF,sizeof(uint64_t));

    for(int i = 0; i < times; i ++){
        switch(choice){
            case CMD_PRINT_RECEIVE_MSG:
            {
                time_t currentTime;
                message_t rx_msg;
                LOG_PRINT(LOG_INFO,"iccmsg_hal_test: CMD_PRINT_RECEIVE_MSG start:\n");
                for(;;){
                    
                    if(receive_message_timeout(&hal->rx_que, &rx_msg, -1)) {
                        LOG_PRINT(LOG_ERR,"receive_message_timeout error.\n");
                        return -1;
                    }
                    time(&currentTime);
                    struct tm *localTime = localtime(&currentTime);
                    char timeString[100];
                    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localTime);
                    
                    LOG_PRINT(LOG_INFO,"[%s][%s]received %d bytes:\n",(rx_msg.ch_type==ICCMSG_CH_IPCC)?"ipcc":"virtio",\
                    (rx_msg.state == ICCMSG_ACK_NO_NEED)? "NACK":"ACK" ,rx_msg.data_len );
                    if(strlen(rx_msg.data)){
                        LOG_PRINT(LOG_INFO,"%s\n",rx_msg.data);
                    }
                    else{
                        for(int j = 0; j < rx_msg.data_len; j++){
                            LOG_PRINT(LOG_INFO,"0x%x ", rx_msg.data[j]);
                        }
                    }
                    LOG_PRINT(LOG_INFO,"\n");
                }
                
            }
            
            break;

            case CMD_SINGLE_SYNC_ECHO_NACK:
            {
                LOG_PRINT(LOG_INFO,"iccmsg_hal_test: CMD_SINGLE_SYNC_ECHO_NACK start.\n");
                msg.cmd = CMD_SINGLE_SYNC_ECHO_NACK;
                msg.cnt = i;
                ret = iccmsg_write(hal, 1, i, &msg, sizeof(iccmsg_test_single_msg_t), ICCMSG_ACK_NO_NEED);
                if(ret < 0){
                    LOG_PRINT(LOG_ERR,"write error.\n");
                }
                gettimeofday(&tv, NULL);
                LOG_PRINT(LOG_INFO,"(>%d)write %d bytes.\n",tv.tv_usec, sizeof(iccmsg_test_single_msg_t));
                uint64_t tick = tv.tv_usec;
                // waitting for test frame 
                message_t rx_msg;
                if(receive_message_timeout(&hal->rx_que, &rx_msg, -1)) {
                    LOG_PRINT(LOG_ERR,"receive_message_timeout error.\n");
                    return -1;
                }
                gettimeofday(&tv, NULL);
                if(tv.tv_usec < tick) {
                    usec_rec = (1000000 - tick) + tv.tv_usec;
                } else {
                    usec_rec = tv.tv_usec - tick;
                }
                if(max_latancy < usec_rec)
                    max_latancy = usec_rec;
                if(min_latancy > usec_rec)
                    min_latancy = usec_rec;
                sum_latancy += usec_rec;
                LOG_PRINT(LOG_INFO,"(>%d)read %d bytes, go-back latancy:%d us\n",tv.tv_usec, rx_msg.data_len, usec_rec);
                iccmsg_test_single_msg_t * test_msg = (iccmsg_test_single_msg_t *)rx_msg.data;
                if(test_msg->cmd != CMD_SINGLE_SYNC_ECHO_NACK || test_msg->cnt != msg.cnt) {
                    LOG_PRINT(LOG_ERR,"error.\n");
                }else {
                    LOG_PRINT(LOG_INFO,"test CMD_SINGLE_SYNC_ECHO_NACK %d times success.\n",i+1);
                }
            }
            
            break;

            case CMD_SINGLE_ASYNC_ECHO_ACK:
            {
                LOG_PRINT(LOG_INFO,"iccmsg_hal_test: CMD_SINGLE_ASYNC_ECHO_ACK start.\n");
                msg.cmd = CMD_SINGLE_ASYNC_ECHO_ACK;
                msg.cnt = i;
                ret = iccmsg_write(hal, 0, i, &msg, sizeof(iccmsg_test_single_msg_t), ICCMSG_ACK_NEED);
                if(ret < 0){
                    LOG_PRINT(LOG_ERR,"write error.\n");
                }
                gettimeofday(&tv, NULL);
                LOG_PRINT(LOG_INFO,"(>%d)write %d bytes:\n",tv.tv_usec, sizeof(iccmsg_test_single_msg_t));
                LOG_PRINT(LOG_DEBUG," write msg.cmd = %d, msg.cnt = %d\n",msg.cmd, msg.cnt);
                uint64_t tick = tv.tv_usec;
                // waitting for test frame 
                message_t rx_msg;
                if(receive_message_timeout(&hal->rx_que, &rx_msg, -1)) {
                    LOG_PRINT(LOG_ERR,"receive_message_timeout error.\n");
                    return -1;
                }
                gettimeofday(&tv, NULL);
                if(tv.tv_usec < tick) {
                    usec_rec = (1000000 - tick) + tv.tv_usec;
                } else {
                    usec_rec = tv.tv_usec - tick;
                }
                if(max_latancy < usec_rec)
                    max_latancy = usec_rec;
                if(min_latancy > usec_rec)
                    min_latancy = usec_rec;
                sum_latancy += usec_rec;
                LOG_PRINT(LOG_INFO,"(>%d)read %d bytes, go-back latancy:%d us\n",tv.tv_usec, rx_msg.data_len, usec_rec);
                
                iccmsg_test_single_msg_t * test_msg = (iccmsg_test_single_msg_t *)rx_msg.data;
                LOG_PRINT(LOG_INFO," read msg.cmd = %d, msg.cnt = %d\n",test_msg->cmd, test_msg->cnt);
                if(test_msg->cmd != CMD_SINGLE_ASYNC_ECHO_ACK || test_msg->cnt != msg.cnt) {
                    LOG_PRINT(LOG_ERR,"error.test_msg->cmd = %d,rx_msg->cnt = %d,tx_msg.cnt = %d\n",test_msg->cmd,msg.cnt);
                }else {
                    LOG_PRINT(LOG_INFO,"test CMD_SINGLE_ASYNC_ECHO_ACK %d times success.\n",i+1);
                }

            }
            
            break;

            case CMD_MULTI_SYNC_ECHO_NACK:
            {
                LOG_PRINT(LOG_INFO,"iccmsg_hal_test: CMD_MULTI_SYNC_ECHO_NACK start.\n");
                iccmsg_test_multi_msg_t multi_msg = {
                    .cmd = CMD_MULTI_SYNC_ECHO_NACK,
                    .cnt = i,
                };
                memset(multi_msg.data,0x00,sizeof(multi_msg.data));
                // 数据填充
                for (int i = 0; i < ICCMSG_TEST_MULTI_MSG_SIZE; i++)
                {
                    multi_msg.data[i] = i%0xFF;
                }
                ret = iccmsg_write(hal, 1, i, &multi_msg, sizeof(iccmsg_test_multi_msg_t), ICCMSG_ACK_NO_NEED);
                if(ret < 0){
                    LOG_PRINT(LOG_ERR,"write error.\n");
                }
                gettimeofday(&tv, NULL);
                LOG_PRINT(LOG_INFO,"(>%d)write %d bytes.\n",tv.tv_usec, sizeof(iccmsg_test_single_msg_t));
                uint64_t tick = tv.tv_usec;
                // waitting for test frame 
                message_t rx_msg;
                if(receive_message_timeout(&hal->rx_que, &rx_msg, -1)) {
                    LOG_PRINT(LOG_ERR,"receive_message_timeout error.\n");
                    return -1;
                }
                gettimeofday(&tv, NULL);
                if(tv.tv_usec < tick) {
                    usec_rec = (1000000 - tick) + tv.tv_usec;
                } else {
                    usec_rec = tv.tv_usec - tick;
                }
                if(max_latancy < usec_rec)
                    max_latancy = usec_rec;
                if(min_latancy > usec_rec)
                    min_latancy = usec_rec;
                sum_latancy += usec_rec;
                LOG_PRINT(LOG_INFO,"(>%d)read %d bytes, go-back latancy:%d us\n",tv.tv_usec, rx_msg.data_len, usec_rec);

                if(rx_msg.data_len != sizeof(iccmsg_test_multi_msg_t)) {
                    LOG_PRINT(LOG_ERR,"rx_msg.data_len != sizeof(iccmsg_test_multi_msg_t).\n");
                    return -1;
                }

                iccmsg_test_multi_msg_t verify_msg;
                memcpy(&verify_msg, rx_msg.data, sizeof(iccmsg_test_multi_msg_t));
                if(verify_msg.cmd != multi_msg.cmd || verify_msg.cnt != multi_msg.cnt) {
                    LOG_PRINT(LOG_ERR,"error:verify_msg.cmd = %d, verify_msg.cnt = %d, multi_msg.cmd = %d, multi_msg.cnt = %d\n", \
                                        verify_msg.cmd, verify_msg.cnt, multi_msg.cmd, multi_msg.cnt);
                }else {
                    for(int i = 0; i < ICCMSG_TEST_MULTI_MSG_SIZE; i++) {
                        if(verify_msg.data[i] != multi_msg.data[i]){
                            LOG_PRINT(LOG_ERR,"test CMD_MULTI_SYNC_ECHO_NACK failed. data wrong.\n");
                            return -1;
                        }
                    }
                    LOG_PRINT(LOG_INFO,"test CMD_MULTI_SYNC_ECHO_NACK %d times success.\n",i+1);
                }
            }
            break;

            case CMD_MULTI_ASYNC_ECHO_ACK:
            {
                LOG_PRINT(LOG_INFO,"iccmsg_hal_test: CMD_MULTI_ASYNC_ECHO_ACK start.\n");
                iccmsg_test_multi_msg_t multi_msg = {
                    .cmd = CMD_MULTI_ASYNC_ECHO_ACK,
                    .cnt = i,
                };
                memset(multi_msg.data,0x00,sizeof(multi_msg.data));
                // 数据填充
                for (int i = 0; i < ICCMSG_TEST_MULTI_MSG_SIZE; i++)
                {
                    multi_msg.data[i] = i%0xFF;
                }
                ret = iccmsg_write(hal, 0, i, &multi_msg, sizeof(iccmsg_test_multi_msg_t), ICCMSG_ACK_NEED);
                if(ret < 0){
                    LOG_PRINT(LOG_ERR,"write error.\n");
                }
                gettimeofday(&tv, NULL);
                LOG_PRINT(LOG_INFO,"(>%d)write %d bytes.\n",tv.tv_usec, sizeof(iccmsg_test_single_msg_t));
                uint64_t tick = tv.tv_usec;
                // waitting for test frame 
                message_t rx_msg;
                if(receive_message_timeout(&hal->rx_que, &rx_msg, -1)) {
                    LOG_PRINT(LOG_ERR,"receive_message_timeout error.\n");
                    return -1;
                }
                gettimeofday(&tv, NULL);
                if(tv.tv_usec < tick) {
                    usec_rec = (1000000 - tick) + tv.tv_usec;
                } else {
                    usec_rec = tv.tv_usec - tick;
                }
                if(max_latancy < usec_rec)
                    max_latancy = usec_rec;
                if(min_latancy > usec_rec)
                    min_latancy = usec_rec;
                sum_latancy += usec_rec;
                LOG_PRINT(LOG_INFO,"(>%d)read %d bytes, go-back latancy:%d us\n",tv.tv_usec, rx_msg.data_len, usec_rec);

                if(rx_msg.data_len != sizeof(iccmsg_test_multi_msg_t)) {
                    LOG_PRINT(LOG_ERR,"rx_msg.data_len != sizeof(iccmsg_test_multi_msg_t).\n");
                    return -1;
                }

                iccmsg_test_multi_msg_t verify_msg;
                memcpy(&verify_msg, rx_msg.data, sizeof(iccmsg_test_multi_msg_t));
                if(verify_msg.cmd != multi_msg.cmd || verify_msg.cnt != multi_msg.cnt) {
                    LOG_PRINT(LOG_ERR,"error:verify_msg.cmd = %d, verify_msg.cnt = %d, multi_msg.cmd = %d, multi_msg.cnt = %d\n", \
                                        verify_msg.cmd, verify_msg.cnt, multi_msg.cmd, multi_msg.cnt);
                }else {
                    for(int i = 0; i < ICCMSG_TEST_MULTI_MSG_SIZE; i++) {
                        if(verify_msg.data[i] != multi_msg.data[i]){
                            LOG_PRINT(LOG_ERR,"test CMD_MULTI_ASYNC_ECHO_ACK failed. data wrong.\n");
                            return -1;
                        }
                    }
                    LOG_PRINT(LOG_INFO,"test CMD_MULTI_ASYNC_ECHO_ACK %d times success.\n",i+1);
                }
            }
            break;

            case CMD_SINGLE_ASYNC_ECHO_NACK:
            {
                LOG_PRINT(LOG_INFO,"iccmsg_hal_test: CMD_SINGLE_ASYNC_ECHO_NACK start.\n");
                msg.cmd = CMD_SINGLE_ASYNC_ECHO_NACK;
                msg.cnt = i;
                ret = iccmsg_write(hal, 0, i, &msg, sizeof(iccmsg_test_single_msg_t), ICCMSG_ACK_NO_NEED);
                if(ret < 0){
                    LOG_PRINT(LOG_ERR,"write error.\n");
                }
                gettimeofday(&tv, NULL);
                LOG_PRINT(LOG_INFO,"(>%d)write %d bytes:\n",tv.tv_usec, sizeof(iccmsg_test_single_msg_t));
                LOG_PRINT(LOG_DEBUG," write msg.cmd = %d, msg.cnt = %d\n",msg.cmd, msg.cnt);
                uint64_t tick = tv.tv_usec;
                // waitting for test frame 
                message_t rx_msg;
                if(receive_message_timeout(&hal->rx_que, &rx_msg, -1)) {
                    LOG_PRINT(LOG_ERR,"receive_message_timeout error.\n");
                    return -1;
                }
                gettimeofday(&tv, NULL);
                if(tv.tv_usec < tick) {
                    usec_rec = (1000000 - tick) + tv.tv_usec;
                } else {
                    usec_rec = tv.tv_usec - tick;
                }
                if(max_latancy < usec_rec)
                    max_latancy = usec_rec;
                if(min_latancy > usec_rec)
                    min_latancy = usec_rec;
                sum_latancy += usec_rec;
                LOG_PRINT(LOG_INFO,"(>%d)read %d bytes, go-back latancy:%d us\n",tv.tv_usec, rx_msg.data_len, usec_rec);
                
                iccmsg_test_single_msg_t * test_msg = (iccmsg_test_single_msg_t *)rx_msg.data;
                LOG_PRINT(LOG_INFO," read msg.cmd = %d, msg.cnt = %d\n",test_msg->cmd, test_msg->cnt);
                if(test_msg->cmd != CMD_SINGLE_ASYNC_ECHO_NACK || test_msg->cnt != msg.cnt) {
                    LOG_PRINT(LOG_INFO,"error.test_msg->cmd = %d,rx_msg->cnt = %d,tx_msg.cnt = %d\n",test_msg->cmd,msg.cnt);
                }else {
                    LOG_PRINT(LOG_INFO,"test CMD_SINGLE_ASYNC_ECHO_ACK %d times success.\n",i+1);
                }

            }
            
            break;

            default:

            break;
        }
    }
    LOG_PRINT(LOG_INFO,"%d times test end. latancy info: min = %d us,max = %d us,avg = %d us.\n",times, min_latancy, max_latancy, sum_latancy/times);
}

