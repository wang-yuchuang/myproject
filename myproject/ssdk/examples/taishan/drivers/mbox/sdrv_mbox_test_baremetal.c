#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#ifdef CONFIG_ARCH_CHIP_E3_SF
#include <CLI_type.h>
#endif /* CONFIG_ARCH_CHIP_E3_SF */

#include <board.h>

#include <mbox_cfg.h>
#include <driver/taishan/mbox/sdrv_taishan_mbox_drv.h>

static mbox_chan_t *send_chan;
static mbox_chan_t *recv_chan;
static osSemaphoreId_t sem;

void mbox_recv_internal_test(void);

static void mbox_test_usage(void)
{
    printf("mbox_test send_recv [rproc]\n");
    printf("\t\t:mbox send test, rproc is 1 ~ 4 .\n");
}

static int mbox_sf_callback_test(void *arg, uint8_t *msg, uint32_t len)
{
    printf("SF mbox recv data len:%d\n", len);
    for (uint32_t cnt = 0; cnt < len; cnt++) {
        printf("%x ", msg[cnt]);
    }
    printf("\n");

    osSemaphoreRelease(sem);
    return 0;
}

static void mbox_send_recv_test(int argc, char *argv[])
{
    sdrv_mbox_chan_req_t chan_req;
    uint8_t send_data[32];
    int ret;

    sem = osSemaphoreNew(1, 0, NULL);

    if (argc != 1) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    chan_req.rproc = atoi(argv[0]);
    chan_req.src_addr = MBOX_ADDR_RAW;
    chan_req.dest_addr = MBOX_ADDR_RAW;

    send_chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!send_chan) {
        printf("mbox_test request send channel fail\n");
        return;
    }
    recv_chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!recv_chan) {
        printf("mbox_test request send channel fail\n");
        return;
    }
    mbox_set_callback(recv_chan, mbox_sf_callback_test, recv_chan);
    /* send long msg */
    memset(send_data, 0xAA, 32);
    printf("SF mbox send data len: 32 \n");
    for(uint8_t i = 0; i < 32; ++i)
    {
        printf("%x ", send_data[i]);
    }
    printf("\n");

    ret = mbox_send(send_chan, send_data, 32, 1000);
    if (ret >= 0) {
        printf("%s: send long msg success\n", __func__);
    }
    else {
        printf("%s: send long msg fail\n", __func__);
    }

    if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
        printf("%s: recv long msg success\n", __func__);
    }
    else {
        printf("%s: recv long msg fail\n", __func__);
    }
    mbox_release_channel(send_chan);
    mbox_release_channel(recv_chan);
}

static int mbox_callback_test(void *arg, uint8_t *msg, uint32_t len)
{
    mbox_send_isr(send_chan, msg, len, 1000);
    return 0;
}

void mbox_recv_internal_test(void)
{
    sdrv_mbox_chan_req_t chan_req;

    chan_req.rproc = 0;
    chan_req.src_addr = MBOX_ADDR_RAW;
    chan_req.dest_addr = MBOX_ADDR_RAW;

    recv_chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!recv_chan) {
        return;
    }
    send_chan = mbox_request_channel(g_mbox_dev, &chan_req);
    if (!send_chan) {
        return;
    }

    mbox_set_callback(recv_chan, mbox_callback_test, recv_chan);
}


static int mbox_demo(int argc, char *argv[])
{
    if (argc == 0) {
        mbox_test_usage();
        return 0;
    }
    else if (!strcmp(argv[0], "send_recv")) {
        mbox_send_recv_test(argc - 1, &argv[1]);
    }
    else {
        printf("cmd error:%s\n", argv[0]);
    }

    return 0;
}

#ifdef CONFIG_ARCH_CHIP_E3_SF
CLI_CMD("mbox_demo", "\r\nmbox_demo:\r\n mbox demo\r\n", mbox_demo);
#endif /* CONFIG_ARCH_CHIP_E3_SF */