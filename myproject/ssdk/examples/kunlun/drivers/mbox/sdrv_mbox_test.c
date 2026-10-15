#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cmsis_os2.h>
#include <CLI_type.h>

#include <board.h>
#include <driver/kunlun/mbox/sdrv_mbox_drv.h>

static mbox_dev_t *test_mbox;
static mbox_chan_t *test_chan;
static sdrv_mbox_dev_t test_sdrv_mbox;
static osSemaphoreId_t sem;

static void mbox_test_usage(void)
{
    printf("\n");
    printf("mbox_test init");
    printf("\t\t\t:mbox init test\n");

    printf("mbox_test req_chan [rproc] [src_addr] [dest_addr]");
    printf("\t\t:mbox request channel test\n");

    printf("mbox_test send_test [rproc] [src_addr] [dest_addr]");
    printf("\t\t:mbox request loop_test test\n");
}

static mbox_config_t config = {
    .base = MB_REG_BASE,
    .irq = MU_MU_MESSAGE_READY_INTR_NUM,
};

static void mbox_init_test(void)
{
    test_mbox = sdrv_mbox_probe(&test_sdrv_mbox, &config);
    if (!test_mbox) {
        printf("%s: fail\n", __func__);
    }
    else {
        printf("%s: success\n", __func__);
    }
}

static void mbox_req_chan_test(int argc, char *argv[])
{
    sdrv_mbox_chan_req_t chan_req;

    if (argc != 3) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    if (!test_mbox) {
        test_mbox = sdrv_mbox_probe(&test_sdrv_mbox, &config);
    }

    chan_req.rproc = atoi(argv[0]);
    chan_req.src_addr = atoi(argv[1]);
    chan_req.dest_addr = atoi(argv[2]);

    test_chan = mbox_request_channel(test_mbox, &chan_req);
    if (!test_chan) {
        printf("%s: fail\n", __func__);
        return;
    }

    mbox_release_channel(test_chan);
    printf("%s: success\n", __func__);
}

static void mbox_send_test(int argc, char *argv[])
{
    sdrv_mbox_chan_req_t chan_req;
    uint8_t send_data[32];
    int ret;

    if (argc != 3) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    if (!test_mbox) {
        test_mbox = sdrv_mbox_probe(&test_sdrv_mbox, &config);
    }

    chan_req.rproc = atoi(argv[0]);
    chan_req.src_addr = atoi(argv[1]);
    chan_req.dest_addr = atoi(argv[2]);

    test_chan = mbox_request_channel(test_mbox, &chan_req);
    if (!test_chan) {
        printf("mbox_test request send channel fail\n");
        return;
    }

    memset(send_data, 0xAA, 32);

    ret = mbox_send(test_chan, send_data, 32, 1000);
    if (ret >= 0) {
        printf("%s: mbox send long msg success\n", __func__);
    }
    else {
        printf("%s: mbox send long msg fail\n", __func__);
    }

    mbox_release_channel(test_chan);
}

static void mbox_send_test_nocopy(int argc, char *argv[])
{
    sdrv_mbox_chan_req_t chan_req;
    uint8_t *tx_data = NULL;
    int ret;

    if (argc != 3) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    if (!test_mbox) {
        test_mbox = sdrv_mbox_probe(&test_sdrv_mbox, &config);
    }

    chan_req.rproc = atoi(argv[0]);
    chan_req.src_addr = atoi(argv[1]);
    chan_req.dest_addr = atoi(argv[2]);

    test_chan = mbox_request_channel(test_mbox, &chan_req);
    if (!test_chan) {
        printf("mbox_test request send channel fail\n");
        return;
    }

    tx_data = mbox_alloc_buf(test_chan, 32);
    memset(tx_data, 0xAA, 32);
    ret = mbox_send_nocopy(test_chan, tx_data, 32, 1000);

    if (ret >= 0) {
        printf("%s: mbox send long msg nocopy success\n", __func__);
    }
    else {
        printf("%s: mbox send long msg nocopy fail\n", __func__);
    }

    mbox_release_channel(test_chan);
}

static int mbox_callback_test(void *arg, uint8_t *msg, uint32_t len)
{
    printf("mbox recv data len:%d\n", len);
    for (uint32_t cnt = 0; cnt < len; cnt++) {
        printf("%x ", msg[cnt]);
    }
    printf("\n");

    osSemaphoreRelease(sem);
    return 0;
}

static void mbox_recv_test(int argc, char *argv[])
{
    sdrv_mbox_chan_req_t chan_req;

    if (argc != 3) {
        printf("%s: argc num error:%d\n", __func__, argc);
        return;
    }

    if (!test_mbox) {
        test_mbox = sdrv_mbox_probe(&test_sdrv_mbox, &config);
    }

    chan_req.rproc = atoi(argv[0]);
    chan_req.src_addr = atoi(argv[1]);
    chan_req.dest_addr = atoi(argv[2]);

    test_chan = mbox_request_channel(test_mbox, &chan_req);
    if (!test_chan) {
        printf("mbox_test request send channel fail\n");
        return;
    }

    sem = osSemaphoreNew(1, 0, NULL);

    mbox_set_callback(test_chan, mbox_callback_test, test_chan);
    if (osSemaphoreAcquire(sem, osWaitForever) == osOK) {
        printf("%s: success\n", __func__);
    }
    else {
        printf("%s: fail\n", __func__);
    }

    mbox_release_channel(test_chan);
}

static int mbox_test(int argc, char *argv[])
{
    if (argc == 0) {
        mbox_test_usage();
        return CLI_FALSE;
    }

    if (!strcmp(argv[0], "init")) {
        mbox_init_test();
    }
    else if (!strcmp(argv[0], "req_chan")) {
        mbox_req_chan_test(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "send_test")) {
        mbox_send_test(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "send_test_nocopy")) {
        mbox_send_test_nocopy(argc - 1, &argv[1]);
    }
    else if (!strcmp(argv[0], "recv_test")) {
        mbox_recv_test(argc - 1, &argv[1]);
    }
    else {
        printf("cmd error:%s\n", argv[0]);
    }

    return CLI_FALSE;
}

CLI_CMD("mbox_test", "mbox test", mbox_test);
