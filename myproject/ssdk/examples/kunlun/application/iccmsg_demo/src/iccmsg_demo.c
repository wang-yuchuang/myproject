#include <CLI.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <driver/wav_header.h>
#include "param.h"
#include "iccmsg.h"
#include "iccmsg_demo.h"
#include "driver/watchdog.h"
#include "driver/kunlun/watchdog/sdrv_watchdog_drv.h"
#include "regs_base.h"
#include "irq_num.h"

static int wdt_overflow_isr(uint32_t irq, void *arg);
static osSemaphoreId_t g_sem;
static watchdog_device_t g_wdt;
static sdrv_watchdog_device_t g_sdrv_wdt = {
#ifdef CONFIG_BOARD_D9_AP2
    .base  =  APB_WDT6_BASE,
    .irq   =  WDT6_OVFLOW_INTR_NUM,
#else
    .base  =  APB_WDT1_BASE,
    .irq   =  WDT1_OVFLOW_INTR_NUM,
#endif
    .isr   =  wdt_overflow_isr,
    .int_rst = 1,
};

static void app_iccmsg_list(void){
    printf( "there is all %d registered iccmsgs:\n", get_iccmsg_num());
    iccmsg_list_all();
}

/*
    iccmsg_demo 循环测试：
    1. iccmsg实例 增加、删除、查询;
    2. 数据发送测试：同步发送测试，异步发送测试，异步发送ACK测试。
*/
//先发后收，对比收发是否一致


static int iccmsg_send_test(iccmsg_t *iccmsg)
{
    iccmsg_t *p_iccmsg = iccmsg;
    uint8_t buf1[256];  //单帧发送
    uint8_t buf2[512];  //多帧发送

    memset(buf1,0xA1,sizeof(buf1));
    memset(buf2,0xA1,sizeof(buf2));
    // 同步单帧发送 ipcc
    printf("single frame sync send ...\r\n");
    if(iccmsg_send(p_iccmsg, 1, 0, buf1, sizeof(buf1), ICCMSG_ACK_NO_NEED))
        printf("single frame sync send failed.\r\n");
    // 同步多帧发送 virtio
    printf("multi frame sync send ...\r\n");
    if(iccmsg_send(p_iccmsg, 1, 0, buf2, sizeof(buf2), ICCMSG_ACK_NO_NEED))
        printf("multi frame sync send failed.\r\n");
    // 异步单帧发送 ipcc 无需等待ACK
    printf("single frame async send(no ack) ...\r\n");
    if(iccmsg_send(p_iccmsg, 0, 0, buf1, sizeof(buf1), ICCMSG_ACK_NO_NEED))
        printf("single frame sync send(no ack) failed.\r\n");
    // 异步单帧发送 ipcc 需要等待ACK
    printf("single frame async send(need ack) ...\r\n");
    if(iccmsg_send(p_iccmsg, 0, 0, buf1, sizeof(buf1), ICCMSG_ACK_NEED))
        printf("single frame async send(need ack) failed.\r\n");
    // 异步多帧发送 virtio 无需等待ACK
    printf("multi frame async send(no ack) ...\r\n");
    if(iccmsg_send(p_iccmsg, 0, 0, buf2, sizeof(buf2), ICCMSG_ACK_NO_NEED))
        printf("multi frame async send(no ack) failed.\r\n");
    // 异步多帧发送 virtio 需要等待ACK
    printf("multi frame async send(need ack) ...\r\n");
    if(iccmsg_send(p_iccmsg, 0, 0, buf2, sizeof(buf2), ICCMSG_ACK_NEED))
        printf("multi frame async send(need ack) failed.\r\n");
    
    osDelay(12000);
    iccmsg_trace_dump(&p_iccmsg->tx_ctx,12);
    // iccmsg_trace_dump(&p_iccmsg->rx_ctx,6);
    return 0;
}




int iccmsg_loop_test(iccmsg_t *iccmsg)
{
    iccmsg_t *p_iccmsg = iccmsg;
    iccmsg_rx_msg_t *rx_msg = NULL;
    uint8_t buf[256];
    uint32_t i = 0;
    int ret = 0;

    for(i = 0;i < 256;i++)
    {
        buf[i] = i;
    }

    if(iccmsg_send(p_iccmsg, 0, 0, buf, 256, ICCMSG_ACK_NO_NEED)){
        printf("error: send failed!\r\n");
    }
    i = 0;
    while(i < 3){
        ret = osMessageQueueGet(p_iccmsg->rx_queue, &rx_msg, NULL, osWaitForever);
        if (ret == osOK && NULL != rx_msg)
        {
            printf("ept[%d] received msg:\r\n",p_iccmsg->ch_ept);
            for(i = 0; i < rx_msg->len; i++){
                printf("%02x ",rx_msg->data[i]);
                if(buf[i] != rx_msg->data[i]){
                    printf("error: data error!\r\n");
                    return -1;
                }
            }
            printf(".\r\n");
        }
        osDelay(1000);
        printf("waiting for echo back.\r\n");
        i ++;
    }

    return 0;
}

static int iccmsg_instance_add_remove_test(void){
    char name[32];
    uint32_t i = 0;
    frame_conf_t frm = {0};
    iccmsg_t *p_iccmsg = NULL;

    //1.1 增删改查 iccmsg 实例 增加测试
    printf("iccmsg test add 10 instance.\r\n");
    for(i = 0; i < ICCMSG_TEST_NUM; i++)
    {
        sprintf(name, "test%d",i);
        if(NULL == iccmsg_add(name,frm,NULL,NULL)){
            printf( "add iccmsg %s failed\n", name);
            break;
        }
    }
    if(get_iccmsg_num() < ICCMSG_TEST_NUM){
        printf("error: iccmsg_add num:%d,but only create %d.\r\n",ICCMSG_TEST_NUM,get_iccmsg_num());
    }
    iccmsg_list_all();
    //1.2 增删改查 iccmsg 实例 查找&删除测试
    printf("iccmsg test remove 10 instance.\r\n");
    for(i = 0; i < ICCMSG_TEST_NUM; i++)
    {
        sprintf(name, "test%d",i);
        p_iccmsg = find_iccmsg(name);
        if(NULL != p_iccmsg)
        {
           if(false == iccmsg_remove(p_iccmsg)){
                printf( "remove iccmsg %s failed\n", name);
                break;
            }
        } 
    }

    if(0 != get_iccmsg_num()){
        printf("error: iccmsg_remove num:%d,but there's still %d not removed.\r\n",ICCMSG_TEST_NUM,get_iccmsg_num());
    }
    iccmsg_list_all();

    return 0;
}

static int crc32_test(void){
    char buf[512];
    int i = 0;
    int ret;

    // crc32 测试
    for(i = 0; i < 512; i++){
        buf[i] = i;
    }
    ret = sd_crc32(0, (const uint8_t *)buf, 512);
    ssdk_printf(SSDK_INFO, "cal 0~512 crc32 value = 0x%x.\n",ret);
    return 0;
}

static int ext_heap_test(void){
    int i;

    ssdk_printf(SSDK_INFO, "ExtMalloc test start.\n");
    ssdk_printf(SSDK_INFO, "FreeHeapSize:%d\n",ExtxPortGetFreeHeapSize());
    uint32_t *heap_addr[256] = {0};
    for(i = 0; i < 255; i++){
        heap_addr[i] = ExtMalloc(1024*1024);
        if(NULL == heap_addr[i]){
            ssdk_printf(SSDK_INFO, "[%d]:ExtMalloc failed.\n",i);
            return -1;
        }
    }
    ssdk_printf(SSDK_INFO, "FreeHeapSize:%d KB\n",ExtxPortGetFreeHeapSize()/1024);
    ssdk_printf(SSDK_INFO, "ExtFree test start.\n");
    for(i = 0; i < 255; i++){
        ExtFree(heap_addr[i]);
        heap_addr[i] = NULL;
    }
    ssdk_printf(SSDK_INFO, "FreeHeapSize:%d KB\n",ExtxPortGetFreeHeapSize()/1024);
    return 0;
}

static void msgqueue_creater(void *arg) {
    iccmsg_t *p_iccmsg = (iccmsg_t *)arg;
    iccmsg_rx_msg_t *msg[256];
    uint8_t * p_u8;
    int i = 0, j = 0;

    for(i = 0; i < 256; i ++) {
        msg[i] = ExtMalloc(sizeof(iccmsg_rx_msg_t));
        if(NULL == msg[i]) {
            ssdk_printf(SSDK_ERR, "msgqueue_creater msg: ExtMalloc failed!\n");
            return ;
        }
        msg[i]->len = 100;
        msg[i]->data = ExtMalloc(100);
        if(NULL == msg[i]->data) {
            ssdk_printf(SSDK_ERR, "msgqueue_creater msg->data: ExtMalloc failed!\n");
            return ;
        }

        for(j = 0; j < 100; j++) {
            msg[i]->data[j] = j;
        }

        if(osOK != osMessageQueuePut(p_iccmsg->rx_queue, &msg[i], 0, 10)){
            ssdk_printf(SSDK_ERR, "iccmsg_async_send: cache msg new failed!\n");
            return ;
        }
        
        ssdk_printf(SSDK_DEBUG, "osMessageQueuePut: msg[%d] = %x, msg[%d]->data = %x\n", i, (uint32_t)msg[i], i, (uint32_t)msg[i]->data);
    }
    osThreadExit();
    
}

static int msgqueue_test(iccmsg_t *iccmsg) {
    iccmsg_t *p_iccmsg = iccmsg;
    iccmsg_rx_msg_t *rx_msg = NULL;
    int i = 0, ret = 0;
    uint32_t heap_rec = ExtxPortGetFreeHeapSize();
    ssdk_printf(SSDK_INFO, "msgqueue_test start FreeHeapSize:%d B\n", heap_rec);
    //thread 
    osThreadAttr_t attr = {
        .name = "msgqueue_creater",
        .stack_size = 4096,
        .priority = 24,
    };
    osThreadId_t *msgqueue_test_id = osThreadNew(msgqueue_creater, p_iccmsg, &attr);
    if (!msgqueue_test_id) {
        ssdk_printf(SSDK_ERR, "msgqueue_creater thread create err\n");
    }
    osDelay(1000);    //wait some msg create enough
    //create queue msg 
    while(i < 256){
        watchdog_ping(&g_wdt);
        ret = osMessageQueueGet(p_iccmsg->rx_queue, &rx_msg, NULL, 1000);
        if (ret == osOK && NULL != rx_msg)
        {
            ssdk_printf(SSDK_DEBUG, "osMessageQueueGet:msg[%d] = %x, msg[%d]->data = %x\n", i, (uint32_t)rx_msg, i, (uint32_t)rx_msg->data);
            for(int j = 0; j < 100; j ++){
                if(rx_msg->data[j] != j){
                    ssdk_printf(SSDK_INFO, "msg data wrong\n");
                }
            }
            ExtFree(rx_msg->data);
            ExtFree(rx_msg);
            i++;

        }
    }
    if(heap_rec != ExtxPortGetFreeHeapSize()){
        ssdk_printf(SSDK_ERR, "msgqueue_test failed. FreeHeapSize:%d B\n", ExtxPortGetFreeHeapSize());
    }
    else{
        ssdk_printf(SSDK_INFO, "msgqueue_test success.\n");
    }
    
    return 0;
}

extern void board_reset_init(void);

static int wdt_overflow_isr(uint32_t irq, void *arg)
{
    printf("wdt_overflow_isr\n");
    board_reset_init();

    return 0;
}
static void iccmsg_echo_server(void *arg)
{
    uint32_t i = 0;
    iccmsg_t *p_iccmsg = (iccmsg_t *)arg;
    frame_conf_t frm = {0};
    
    iccmsg_rx_msg_t *rx_msg = NULL;
    int ret;
    iccmsg_test_msg_t tmp;
    iccmsg_test_multi_msg_t multi_msg;

    /* stats counters */
    static uint32_t st_sync_nack = 0;
    static uint32_t st_async_ack = 0;
    static uint32_t st_multi_nack = 0;
    static uint32_t st_multi_ack = 0;
    static uint32_t st_async_nack = 0;
    static uint32_t st_multi_async_nack = 0;
    static uint32_t st_err = 0;
    static uint32_t st_total = 0;
    static uint32_t st_rpt = 1;  /* report every N cmds */


    watchdog_ping(&g_wdt);
    while(1)
    {
        ret = osMessageQueueGet(p_iccmsg->rx_queue, &rx_msg, NULL, 1000);
        watchdog_ping(&g_wdt);

        if (ret == osOK && NULL != rx_msg)
        {
            iccmsg_test_msg_t *msg = (iccmsg_test_msg_t *)rx_msg->data;
            ssdk_printf(SSDK_DEBUG, "received msg: %d bytes.\n",rx_msg->len);

            switch(msg->cmd)
            {
                case CMD_SINGLE_SYNC_ECHO_NACK:
                    st_sync_nack++;
                    memcpy(&tmp,rx_msg->data,rx_msg->len);
                    iccmsg_send(p_iccmsg, 1, 0, &tmp, rx_msg->len, ICCMSG_ACK_NO_NEED);
                    // release queue msg memory space
                    ExtFree(rx_msg->data);
                    rx_msg->data = NULL;
                    ExtFree(rx_msg);
                    rx_msg = NULL;

                break;

                case CMD_SINGLE_ASYNC_ECHO_ACK:
                    st_async_ack++;
                    memcpy(&tmp,rx_msg->data,rx_msg->len);
                    iccmsg_send(p_iccmsg, 0, 0, &tmp, rx_msg->len, ICCMSG_ACK_NEED);
                    // release queue msg memory space
                    ExtFree(rx_msg->data);
                    rx_msg->data = NULL;
                    ExtFree(rx_msg);
                    rx_msg = NULL;

                break;

                case CMD_MULTI_SYNC_ECHO_NACK:
                    st_multi_nack++;
                    for(i = 0; i < rx_msg->len; i++){
                        ssdk_printf(SSDK_DEBUG, "data[%d]:%d\n", i, rx_msg->data[i]);
                    }
                    ssdk_printf(SSDK_DEBUG, "\n");
                    // 回传
                    multi_msg.cmd = CMD_MULTI_SYNC_ECHO_NACK;
                    memcpy(&multi_msg,rx_msg->data,rx_msg->len);
                    iccmsg_send(p_iccmsg, 1, 0, &multi_msg, sizeof(iccmsg_test_multi_msg_t), ICCMSG_ACK_NO_NEED);
                    // release queue msg memory space,multi_msg is static,do not release
                    ssdk_printf(SSDK_DEBUG, "release addr :rx_msg->data = 0x%x,FreeHeapSize = %d\n",rx_msg->data,ExtxPortGetFreeHeapSize());
                    ExtFree(rx_msg->data);
                    rx_msg->data = NULL;
                    ExtFree(rx_msg);
                    rx_msg = NULL;
                break;

                case CMD_MULTI_ASYNC_ECHO_ACK:
                    st_multi_ack++;
                    for(i = 0; i < rx_msg->len; i++){
                        ssdk_printf(SSDK_DEBUG, "data[%d]:%d ", i, rx_msg->data[i]);
                    }
                    ssdk_printf(SSDK_DEBUG, "\n");
                    // 回传
                    multi_msg.cmd = CMD_MULTI_ASYNC_ECHO_ACK;
                    memcpy(&multi_msg,rx_msg->data,rx_msg->len);
                    iccmsg_send(p_iccmsg, 0, 0, &multi_msg, sizeof(iccmsg_test_multi_msg_t), ICCMSG_ACK_NEED);
                    // release queue msg memory space,multi_msg is static,do not release
                    ExtFree(rx_msg->data);
                    rx_msg->data = NULL;
                    ExtFree(rx_msg);
                    rx_msg = NULL;
                break;

                case CMD_MULTI_ASYNC_ECHO_NACK:
                    st_multi_async_nack++;
                    for(i = 0; i < rx_msg->len; i++){
                        ssdk_printf(SSDK_DEBUG, "data[%d]:%d ", i, rx_msg->data[i]);
                    }
                    ssdk_printf(SSDK_DEBUG, "\n");
                    multi_msg.cmd = CMD_MULTI_ASYNC_ECHO_NACK;
                    memcpy(&multi_msg,rx_msg->data,rx_msg->len);
                    iccmsg_send(p_iccmsg, 0, 0, &multi_msg, sizeof(iccmsg_test_multi_msg_t), ICCMSG_ACK_NO_NEED);
                    ExtFree(rx_msg->data);
                    rx_msg->data = NULL;
                    ExtFree(rx_msg);
                    rx_msg = NULL;
                break;

                case CMD_SINGLE_ASYNC_ECHO_NACK:
                    st_async_nack++;
                    memcpy(&tmp,rx_msg->data,rx_msg->len);
                    iccmsg_send(p_iccmsg, 0, 0, &tmp, rx_msg->len, ICCMSG_ACK_NO_NEED);
                    // release queue msg memory space
                    ExtFree(rx_msg->data);
                    rx_msg->data = NULL;
                    ExtFree(rx_msg);
                    rx_msg = NULL;
                break;

                default:
                    st_err++;
                    ssdk_printf(SSDK_INFO, "unrecognized cmd : msg->cmd = %d, msg->cnt = %d.\n",msg->cmd, msg->cnt);
                    iccmsg_trace_dump(&p_iccmsg->rx_ctx,4);

                break;
            }
            st_total++;
            if(st_total % st_rpt == 0) {
                ssdk_printf(SSDK_INFO, "[STAT] total=%d | SYN_NACK=%d ASY_ACK=%d ASY_NACK=%d MULTI_NACK=%d MULTI_ACK=%d MULTI_ASY_NACK=%d err=%d | heap=%dKB\n",
                    st_total, st_sync_nack, st_async_ack, st_async_nack, st_multi_nack, st_multi_ack, st_multi_async_nack, st_err,
                    ExtxPortGetFreeHeapSize()/1024);
            }
            
        }
        
    }

    osThreadExit();
}


static int iccmsg_demo_init(int argc, char *argv[])
{
    uint32_t i = 0;
    int ret;
    iccmsg_t *p_iccmsg = NULL;
    frame_conf_t frm = {0};

    printf("iccmsg_demo argc = %d.\r\n",argc);

    // iccmsg 各个功能自检
    if (sdrv_watchdog_init(&g_wdt, &g_sdrv_wdt)) {
        printf("sdrv_watchdog_init fail.\n");
        return -1;
    }
    /* set timeout 2second */
    if (watchdog_set_timeout(&g_wdt, 2)) {
        printf("watchdog_set_timeout fail.\n");
        return -1;
    }
    if (watchdog_start(&g_wdt)) {
        printf("watchdog_start fail.\n");
        return -1;
    }
    watchdog_ping(&g_wdt);
    iccmsg_init();
    watchdog_ping(&g_wdt);
    iccmsg_instance_add_remove_test();
    watchdog_ping(&g_wdt);
    frm.head_len = 2;
    frm.tail_len = 2;
    memset(frm.frame_head,ICCMSG_TEST_MSG_HEAD,frm.head_len);
    memset(frm.frame_tail,ICCMSG_TEST_MSG_TAIL,frm.tail_len);
    frm.checksum_type = CRC32;
    watchdog_ping(&g_wdt);
    p_iccmsg = find_iccmsg(ICCMSG_TEST_REMOTE_NAME);
    if(p_iccmsg == NULL) {
        p_iccmsg = iccmsg_add(ICCMSG_TEST_REMOTE_NAME,frm,NULL,NULL);
    }
    if(p_iccmsg == NULL){
        ssdk_printf(SSDK_ERR, "add iccmsg %s failed\n", ICCMSG_TEST_REMOTE_NAME);
        osThreadExit();
    }
    watchdog_ping(&g_wdt);
    crc32_test();
    watchdog_ping(&g_wdt);
    ext_heap_test();
    watchdog_ping(&g_wdt);
    msgqueue_test(p_iccmsg);
    watchdog_ping(&g_wdt);
    
    if(argc > 1){
        if(!strcmp(argv[0], "w")){
            if(strlen(argv[1])){
                for(;;){
                    watchdog_ping(&g_wdt);
                    iccmsg_send(p_iccmsg, 1, 0, argv[1], strlen(argv[1])+1, ICCMSG_ACK_NO_NEED);
                    osDelay(1000);
                    watchdog_ping(&g_wdt);
                    iccmsg_send(p_iccmsg, 0, 0, argv[1], strlen(argv[1])+1, ICCMSG_ACK_NO_NEED);
                    osDelay(1000);
                    watchdog_ping(&g_wdt);
                    iccmsg_send(p_iccmsg, 0, 0, argv[1], strlen(argv[1])+1, ICCMSG_ACK_NEED);
                }
                
            }
            else {
                ssdk_printf(SSDK_ERR, "error, unavailable args.\n");
                return -1;
            }
        }
        else if(!strcmp(argv[0], "b")){
            iccmsg_test_multi_msg_t burst_msg;
            burst_msg.cmd = 6;
            memset(burst_msg.data, 0xA5, sizeof(burst_msg.data));
            int burst_num = (argc > 2 && atoi(argv[1]) > 0) ? atoi(argv[1]) : 500;
            ssdk_printf(SSDK_INFO, "[BURST] argc=%d argv[0]=%s argv[1]=%s argv[2]=%s\n", argc, argv[0], argc>1?argv[1]:"-", argc>2?argv[2]:"-");
            int burst_mode = (argc > 2 && atoi(argv[2]) >= 2 && atoi(argv[2]) <= 6) ? atoi(argv[2]) : 6;
            uint32_t burst_cmd = 0, burst_ack = ICCMSG_ACK_NO_NEED;
            int multi_frame = (burst_mode == 4 || burst_mode == 6);
            if(burst_mode == 4) { burst_cmd = 4; burst_ack = ICCMSG_ACK_NEED; }
            else if(burst_mode == 5) { burst_cmd = 5; burst_ack = ICCMSG_ACK_NO_NEED; }
            else if(burst_mode == 2) { burst_cmd = 2; burst_ack = ICCMSG_ACK_NEED; }
            else { burst_cmd = 6; burst_ack = ICCMSG_ACK_NO_NEED; }

            iccmsg_test_multi_msg_t burst_multi;
            iccmsg_test_msg_t burst_single;
            memset(&burst_multi, 0xA5, sizeof(burst_multi));
            burst_multi.cmd = burst_cmd;
            burst_single.cmd = burst_cmd;

            ssdk_printf(SSDK_INFO, "[BURST] mode=%d %s ACK=%s x%d...\n",
                burst_mode, multi_frame ? "multi" : "single",
                burst_ack == ICCMSG_ACK_NEED ? "NEED" : "NONE", burst_num);

            watchdog_set_timeout(&g_wdt, 3600);
            for(int bi = 0; bi < burst_num; bi++){
                watchdog_ping(&g_wdt);
                if(multi_frame) {
                    burst_multi.cnt = bi;
                    iccmsg_send(p_iccmsg, 0, 0, &burst_multi, sizeof(burst_multi), burst_ack);
                } else {
                    burst_single.cnt = bi;
                    iccmsg_send(p_iccmsg, 0, 0, &burst_single, sizeof(burst_single), burst_ack);
                }
            }
            watchdog_set_timeout(&g_wdt, 2);
            ssdk_printf(SSDK_INFO, "[BURST] done.\n");
        }
    }
    else {
        watchdog_ping(&g_wdt);
        osThreadAttr_t attr = {
            .name = "iccmsg_demo",
            .stack_size = 10240,
            .priority = 24,
        };
        osThreadId_t *iccmsg_demo_task = osThreadNew(iccmsg_echo_server, (void *)p_iccmsg, &attr);
        if (!iccmsg_demo_task) {
            ssdk_printf(SSDK_CRIT, "iccmsg_demo_task thread create err\n");
        }
    }

    watchdog_set_timeout(&g_wdt, 0);  // disable WDT to prevent system reset after test

    return 0;
}

CLI_CMD("iccmsg_demo",
        "iccmsg_demo: iccmsg echo test server."
        "\r\n\t iccmsg_demo w <string>: write string to remote ",
        iccmsg_demo_init);


