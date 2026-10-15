#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "CLI.h"
#include "board/board.h"
#include "cmsis_os2.h"
#include "driver/taishan/eth/dwc_eth.h"
#include "eth_cfg.h"
#include "lwip/sockets.h"

static int netdemo_connect_server(ip4_addr_t *ip, int port)
{
    int s;
    struct sockaddr_in addr;

    memset(&addr, 0, sizeof(addr));
    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = PP_HTONS(port);
    inet_addr_from_ip4addr(&addr.sin_addr, ip);

    s = lwip_socket(AF_INET, SOCK_STREAM, 0);
    if (s >= 0) {
        int ret = lwip_connect(s, (struct sockaddr*)&addr, sizeof(addr));
        if (ret == 0) {
            return s;
        }
        else {
            lwip_close(s);
            printf("connect to server failed\n");
        }
    }
    else {
        printf("create socket failed\n");
    }

    return -1;
}

static void eth_send_test(ip4_addr_t *ip, int port)
{
    #define WR_CNT 3
    int s = netdemo_connect_server(ip, port);
    static char tx_buf[2000];
    int cnt = WR_CNT;
    uint64_t start, end, v, sum = 0, min = ~0, max = 0;

    if (s >= 0) {
        ssize_t ret;
        unsigned char ch = 0;
        while (cnt--) {
            memset(tx_buf, ++ch, sizeof(tx_buf));
            start = current_time_hires();
            ret = lwip_write(s, tx_buf, sizeof(tx_buf));
            end = current_time_hires();
            v = end - start;
            if (min > v)
                min = v;
            if (max < v)
                max = v;
            sum += v;
            if (ret == sizeof(tx_buf)) {
                osDelay(1000);
            }
            else {
                printf("send failed, errno %d\n", errno);
                break;
            }
        }
        lwip_close(s);

        double min_bw = 2000 / max;
        double max_bw = 2000 / min;
        double bw = 2000 * WR_CNT / sum;
        printf("tcp tx perf: min %fMB/s, max %fMB/s, aver %fMB/s\n",
               min_bw, max_bw, bw);
    }
}

static void netdemo_hexdump(char *d, int l)
{
    for (int i = 0; i < l; i++) {
        if ((i & 0x7) == 0x0) printf("\n");
        printf("0x%x ", d[i]);
    }
    printf("\n");
}

static void eth_read_test(ip4_addr_t *ip, int port)
{
    int s;
    char rx_buf[16];
    int cnt = 3;

    s = netdemo_connect_server(ip, port);
    if ( s >= 0) {
        ssize_t ret;
        while (cnt--) {
            ret = lwip_read(s, rx_buf, sizeof(rx_buf));
            if (ret > 0) {
                printf("receive %d bytes:\n", ret);
                netdemo_hexdump(rx_buf, ret);
                osDelay(1000);
            }
            else {
                printf("receive failed, errno %d\n", errno);
                break;
            }
        }
        lwip_close(s);
    }
}

static void netdemo_setip(char *nic, ip4_addr_t *ip)
{
    struct netif *netif;
    ip4_addr_t gw;

    IP4_ADDR(&gw, ip4_addr1_16(ip),
        ip4_addr2_16(ip), ip4_addr3_16(ip), 1);

    LOCK_TCPIP_CORE();
    netif = netif_find(nic);
    UNLOCK_TCPIP_CORE();
    if (netif) {
        netif_set_ipaddr(netif, ip);
        netif_set_gw(netif, &gw);
    }
    else {
        printf("can't find net device\n");
    }
}

static int eth_test(int argc, char *argv[])
{
    if (argc < 3) {
        printf("Usage:\n");
        printf("eth_demo send/read <ip> <port>\n");
        printf("eth_demo setip <NIC> <ip>\n");
        return 0;
    }

    ip4_addr_t ip;
    int port;

    if (strcmp(argv[0], "setip")) {
        if (!inet_aton(argv[1], &ip)) {
            printf("ipaddr is wrong\n");
            return 0;
        }
        port = strtoul(argv[2], NULL, 0);

        if (0 == strcmp(argv[0], "send"))
            eth_send_test(&ip, port);
        else if (0 == strcmp(argv[0], "read"))
            eth_read_test(&ip, port);
        else
            printf("Invalid command\n");
    }
    else {
        if (!inet_aton(argv[2], &ip)) {
            printf("ipaddr is wrong\n");
            return 0;
        }

        netdemo_setip(argv[1], &ip);
    }

    return 0;
}

CLI_CMD("eth_demo", "\r\neth_demo:\r\n eth demo\r\n", eth_test);
