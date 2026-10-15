#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <board.h>
#include <cmsis_os2.h>
#include <CLI_type.h>
#include <debug.h>
#include <rpmsg_channel.h>
#include <driver/timer.h>
#include <driver/kunlun/syscnt/sdrv_syscnt_drv.h>
#include <rpmsg_service.h>
#include <rpmsg_echo.h>

#ifndef RPMSG_TACK_LEN
#define RPMSG_TACK_LEN 4
#endif

#ifndef RPMSG_TACK_CODE
#define RPMSG_TACK_CODE "ACK"
#endif

#define RPMSG_TEST_EPT   18
#define RPMSG_TEST_NAME  "rpmsg-test"

extern sdrv_syscnt_dev_t g_sdrv_syscnt_dev;
extern timer_dev_t *g_syscnt_timer;

static char *type_name[RPMSG_TYPE_NUM] = {
    "rpmsg.virtio", "rpmsg.ipcc"
};

static char *remote_proc[CONFIG_RPMSG_RPROC_NUM] = {
    "safety", "secure", "mp", "ap1", "ap2",
};

static void rpmsg_test_show(void)
{
    printf("\nList rpmsg communicate with remote:\n");
    printf("\tType\t\t\tRemote-Proc\n");

    for (int i = 0; i < RPMSG_TYPE_NUM; i++) {
        for (int j = 0; j < CONFIG_RPMSG_RPROC_NUM; j++) {
            struct rpmsg_instance *rpmsg_ins;
            if (i == 0 && j == 1) {
                continue;
            }
            rpmsg_ins = rpmsg_get_instance(i, j);
            if (rpmsg_ins != NULL) {
                printf("\t%d:%-12s \t\t%d:%s\n", i, type_name[i], j, remote_proc[j]);
            }
        }
    }

    printf("\nTest command:\n");
    printf("\trpmsg_test <type> <remote-proc>\n");
    printf("\trpmsg_test ping <type> <remote-proc> <times>\n");
    printf("\trpmsg_test iperf <type> <remote-proc> <times>\n");
    printf("\trpmsg_test perf <type> <remote-proc> <times>\n");

    printf("\ne.g: test ping rpmsg.virtio 10 times with secure, use command:\n");
    printf("\trpmsg_test ping 0 1 10\n");
}

static void do_rpmsg_echo_test(rpmsg_channel_t chan)
{
    int max_payload;
    struct dcf_ccm_hdr *snd_pkg;
    struct dcf_ccm_hdr *rcv_pkg;
    int i = 0;
    int rcv_len;
    status_t ret;

    max_payload = rpmsg_channel_max_payload(chan);
    snd_pkg = osAlloc(max_payload);
    rcv_pkg = osAlloc(max_payload);
    ASSERT(snd_pkg && rcv_pkg);

    memset(snd_pkg, 0x0, sizeof(struct dcf_ccm_hdr));
    memset(snd_pkg->data, COMM_MSG_CCM_ECHO, max_payload - sizeof(struct dcf_ccm_hdr));
    snd_pkg->dmsg.msg_type = COMM_MSG_CCM_ECHO;
    snd_pkg->dmsg.opflags |= DCF_MSGF_TMS;
    snd_pkg->dmsg.msg_len = sizeof(struct dcf_ccm_hdr) - sizeof(struct dcf_message);

    while (++i <= (max_payload - sizeof(struct dcf_ccm_hdr))) {
        snd_pkg->dmsg.msg_len++;
        snd_pkg->seq = i;
        snd_pkg->time[0] = timer_get_current_time(g_syscnt_timer) * g_sdrv_syscnt_dev.cnt_per_us;
        ret = rpmsg_channel_send(chan, RPMSG_ECHO_EPT_ADDR, snd_pkg, i + sizeof(struct dcf_ccm_hdr), 3000);
        if (ret != RPMSG_SUCCESS) {
            printf("rpmsg_channel_send fail at %d times\n", i);
            goto exit;
        }

        memset(rcv_pkg, 0x0, max_payload);

        ret = rpmsg_channel_recv(chan, rcv_pkg, max_payload, &rcv_len, 3000);
        if (ret != RPMSG_SUCCESS) {
            printf("rpmsg_channel_recv fail at %d times\n", i);
            goto exit;
        }

        if (rcv_len != (i + sizeof(struct dcf_ccm_hdr))) {
            printf("rpmsg_channel_recv %d bytes at %d times\n", rcv_len, i);
            goto exit;
        }

        if (rcv_pkg->seq != i) {
            printf("rpmsg_channel_recv seq error, %d, %d\n", i, rcv_pkg->seq);
            goto exit;
        }

        for (int k = 0; k < i; k++) {
            if (rcv_pkg->data[k] != COMM_MSG_CCM_ECHO) {
                printf("recv error byte 0x%x at times %d:%d\n", rcv_pkg->data[k], i, k);
                goto exit;
            }
        }
    }

    printf("max_payload is %d, success\n", max_payload);

exit:
    osFree(snd_pkg);
    osFree(rcv_pkg);
}

static void do_rpmsg_ping_test(rpmsg_channel_t chan, uint32_t times)
{
    int max_payload;
    struct dcf_ccm_hdr *snd_pkg;
    struct dcf_ccm_hdr *rcv_pkg;
    int tx_cnt = 0;
    int rx_cnt = 0;
    uint64_t ttl_max = 0;
    uint64_t ttl_min = 3000000;
    uint64_t ttl_total = 0;

    max_payload = rpmsg_channel_max_payload(chan);
    snd_pkg = osAlloc(max_payload);
    rcv_pkg = osAlloc(max_payload);
    ASSERT(snd_pkg && rcv_pkg);

    memset(snd_pkg, 0x0, sizeof(struct dcf_ccm_hdr));
    memset(snd_pkg->data, COMM_MSG_CCM_ECHO, max_payload - sizeof(struct dcf_ccm_hdr));
    snd_pkg->dmsg.msg_type = COMM_MSG_CCM_ECHO;
    snd_pkg->dmsg.opflags |= DCF_MSGF_TMS;
    snd_pkg->dmsg.msg_len = max_payload - sizeof(struct dcf_message);

    while (++tx_cnt <= times) {
        status_t ret;
        uint64_t ttl_start;
        uint64_t ttl_txdone;
        uint64_t ttl_end;
        uint64_t ttl_once;
        int rcv_len;

        memset(rcv_pkg, 0x0, max_payload);

        snd_pkg->seq = tx_cnt;
        snd_pkg->time[0] = timer_get_current_time(g_syscnt_timer) * g_sdrv_syscnt_dev.cnt_per_us;
        ttl_start = current_time_hires();
        ret = rpmsg_channel_send(chan, RPMSG_ECHO_EPT_ADDR, snd_pkg, max_payload, 3000);
        if (ret != RPMSG_SUCCESS) {
            printf("Packet %d send fail\n", tx_cnt);
            goto next;
        }

        ttl_txdone = current_time_hires();

        ret = rpmsg_channel_recv(chan, rcv_pkg, max_payload, &rcv_len, 3000);
        if (ret != RPMSG_SUCCESS) {
            printf("Packet %d recv fail\n", tx_cnt);
            goto next;
        }

        ttl_end = current_time_hires();

        if (rcv_len != max_payload) {
            printf("Packet %d total len wrong, drop\n", tx_cnt);
            goto next;
        }

        if (rcv_pkg->seq != tx_cnt) {
            printf("Packet %d seq wrong, drop\n", tx_cnt);
            goto next;
        }

        if (snd_pkg->dmsg.msg_len != rcv_pkg->dmsg.msg_len) {
            printf("Packet %d len wrong, drop\n", tx_cnt);
            goto next;
        }

        for (int k = 0; k < (max_payload - sizeof(struct dcf_ccm_hdr)); k++) {
            if (rcv_pkg->data[k] != COMM_MSG_CCM_ECHO) {
                printf("Packet %d integrate wrong, drop\n", tx_cnt);
                goto next;
            }
        }

        rx_cnt++;
        ttl_once = ttl_end - ttl_start;
        ttl_max = MAX(ttl_max, ttl_once);
        ttl_min = MIN(ttl_min, ttl_once);
        ttl_total += ttl_once;

        printf("%d bytes from remote: seq=%d time=%llu/%u/%llu us\n",
               rcv_len, tx_cnt, ttl_txdone - ttl_start,
               (rcv_pkg->time[2] ? ((rcv_pkg->time[2] - rcv_pkg->time[0]) / g_sdrv_syscnt_dev.cnt_per_us) : 0),
               ttl_once);

next:
        osDelay(osKernelGetTickFreq());
    }

    printf("\n%d packets send, %d packets received, %.02f%% loss, time %llu us\n",
           times, rx_cnt, 100 - 100 * (float)(rx_cnt / times), ttl_total);
    printf("rtt min/avg/max = %llu/%llu/%llu us\n", ttl_min, ttl_total / rx_cnt, ttl_max);

    osFree(snd_pkg);
    osFree(rcv_pkg);
}

static void do_rpmsg_iperf_test(rpmsg_channel_t chan, uint32_t times)
{
    status_t ret;
    int max_payload;
    struct dcf_ccm_hdr *snd_pkg;
    int tx_pkg = 0;
    int err_pkg = 0;
    uint64_t ttl_start;
    uint64_t ttl_end;
    uint64_t ttl_total;
    unsigned int total_bytes;

    max_payload = rpmsg_channel_max_payload(chan);
    snd_pkg = osAlloc(max_payload);
    ASSERT(snd_pkg);

    memset(snd_pkg, COMM_MSG_CCM_DROP, max_payload);

    ttl_start = current_time_hires();

    for (tx_pkg = 0; tx_pkg < times; tx_pkg++) {
        ret = rpmsg_channel_send(chan, RPMSG_ECHO_EPT_ADDR, snd_pkg, max_payload, 10);
        if (ret != RPMSG_SUCCESS) {
            err_pkg++;
        }
    }

    ttl_end = current_time_hires();
    ttl_total = ttl_end - ttl_start;
    total_bytes = (times - err_pkg) * max_payload;

    printf("%d packets, loss %d packets, %d packets transmitted, %u bytes\n",
           times, err_pkg, times - err_pkg, total_bytes);
    printf("throughput avg. = %.2f MB/s\n", (float)total_bytes / ttl_total);
    printf("tx latency avg. = %llu us\n", ttl_total / (times - err_pkg));

    osFree(snd_pkg);
}

static void do_rpmsg_perf_test(rpmsg_channel_t chan, uint32_t times)
{
    status_t ret;
    int max_payload;
    char *snd_pkg;
    char rcv_pkg[RPMSG_TACK_LEN];
    int tx_cnt = 0;
    int rx_cnt = 0;
    uint64_t ttl_max = 0;
    uint64_t ttl_min = 3000000;
    uint64_t ttl_total = 0;
    uint64_t ttl_once = 0;
    uint64_t ttl_start = 0;
    uint64_t ttl_end = 0;
    int rcv_len;

    max_payload = rpmsg_channel_max_payload(chan);
    snd_pkg = osAlloc(max_payload);
    ASSERT(snd_pkg);

    memset(snd_pkg, COMM_MSG_CCM_ACK, max_payload);

    struct dcf_ccm_hdr *__hdr = (struct dcf_ccm_hdr *)snd_pkg;
    __hdr->dmsg.msg_type = 0xA1;
    rpmsg_channel_send(chan, RPMSG_ECHO_EPT_ADDR, snd_pkg, max_payload, 10);
    __hdr->dmsg.msg_type = COMM_MSG_CCM_ACK;

    do {
        memset(rcv_pkg, 0x0, RPMSG_TACK_LEN);
        rcv_len = 0;

        ttl_start = current_time_hires();

        ret = rpmsg_channel_send(chan, RPMSG_ECHO_EPT_ADDR, snd_pkg, max_payload, 10);
        if (ret != RPMSG_SUCCESS) {
            printf("rpmsg perf test send failed (%d) \n", ret);
            break;
        }

        tx_cnt++;

        ret = rpmsg_channel_recv(chan, rcv_pkg, RPMSG_TACK_LEN, &rcv_len, 3000);
        if (ret != RPMSG_SUCCESS) {
            printf("rpmsg perf test receive ack failed (%d) \n", ret);
            break;
        }

        ttl_end = current_time_hires();

        if ((rcv_len != RPMSG_TACK_LEN) || (strncmp(rcv_pkg, RPMSG_TACK_CODE, RPMSG_TACK_LEN))) {
            printf("rpmsg perf test receive illeage ack \n");
            break;
        }

        rx_cnt++;

        ttl_once = ttl_end - ttl_start;
        ttl_max = MAX(ttl_max, ttl_once);
        ttl_min = MIN(ttl_min, ttl_once);
        ttl_total += ttl_once;
    } while (tx_cnt < times);

    __hdr->dmsg.msg_type = 0xA2;
    rpmsg_channel_send(chan, RPMSG_ECHO_EPT_ADDR, snd_pkg, max_payload, 10);

    printf("\n%d packets transmitted, %d acked, %.02f%% loss, time %lld us\n",
           tx_cnt, rx_cnt, 100 - 100 * (float)(rx_cnt / times), ttl_total);
    printf("rtt min/avg/max = %llu/%llu/%llu us\n", ttl_min, ttl_total / rx_cnt, ttl_max);
    printf("throughput avg. = %.2f MB/s\n", (float)(rx_cnt * max_payload) / ttl_total);

    osFree(snd_pkg);
}

static int rpmsg_test(int argc, char *argv[])
{
    int test_case = -1;
    int type, rproc, times;
    struct rpmsg_instance *rpmsg_dev;
    rpmsg_channel_t channel;

    if (argc == 0) {
        rpmsg_test_show();
        goto exit;
    }

    if (argc == 2) {
        test_case = 0;
        type = atoi(argv[0]);
        rproc = atoi(argv[1]);
    }
    else if (!strcmp(argv[0], "ping")) {
        test_case = 1;
        type = atoi(argv[1]);
        rproc = atoi(argv[2]);
        times = atoi(argv[3]);
    }
    else if (!strcmp(argv[0], "iperf")) {
        test_case = 2;
        type = atoi(argv[1]);
        rproc = atoi(argv[2]);
        times = atoi(argv[3]);
    }
    else if (!strcmp(argv[0], "perf")) {
        test_case = 3;
        type = atoi(argv[1]);
        rproc = atoi(argv[2]);
        times = atoi(argv[3]);
    }
    else {
        printf("Unknown cmd %s\n", argv[0]);
        goto exit;
    }

    rpmsg_dev = rpmsg_get_instance(type, rproc);
    if (rpmsg_dev == NULL) {
        printf("get %d:%d rpmsg device fail\n", type, rproc);
        goto exit;
    }

    channel = rpmsg_channel_create(rpmsg_dev, RPMSG_TEST_NAME, RPMSG_TEST_EPT);
    if (channel == NULL) {
        printf("rpmsg_channel_create fail\n");
        goto exit;
    }

    rpmsg_channel_start(channel, NULL, NULL);
    rpmsg_channel_set_announced(channel, true);

    switch (test_case) {
        case 0:
            do_rpmsg_echo_test(channel);
        break;

        case 1:
            do_rpmsg_ping_test(channel, times);
        break;

        case 2:
            do_rpmsg_iperf_test(channel, times);
        break;

        case 3:
            do_rpmsg_perf_test(channel, times);
        break;

        default:
            printf("Unknown case %d\n", test_case);
        break;
    }

    rpmsg_channel_destroy(channel);
exit:
    return CLI_FALSE;
}

CLI_CMD("rpmsg_test", "rpmsg test", rpmsg_test);
