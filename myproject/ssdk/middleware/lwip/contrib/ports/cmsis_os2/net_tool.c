/*
 * Copyright (c) 2013 Corey Tabaka
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/inet.h>
#include <lwip/stats.h>
#include <apps/ping/ping.h>
#include "lwip/apps/lwiperf.h"
#include "CLI.h"

static void
lwiperf_report(void *arg, enum lwiperf_report_type report_type,
               const ip_addr_t* local_addr, u16_t local_port, const ip_addr_t* remote_addr, u16_t remote_port,
               u32_t bytes_transferred, u32_t ms_duration, u32_t bandwidth_kbitpsec)
{
    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(local_addr);
    LWIP_UNUSED_ARG(local_port);

    printf("IPERF report: type=%d, remote: %s:%d, total bytes: %"U32_F", duration in ms: %"U32_F", kbits/s: %"U32_F"\n",
        (int)report_type, ipaddr_ntoa(remote_addr), (int)remote_port, bytes_transferred, ms_duration, bandwidth_kbitpsec);
}

static int net_cmd(int argc, char *argv[])
{
    if (argc < 1) {
        printf("net commands:\n");
        printf("net ifconfig\n");
        printf("net stats\n");
        printf("net ping <ip>\n");
        printf("net iperf <server | client> [ip]\n");
        goto out;
    }

    if (!strcmp(argv[0], "ifconfig")) {
        struct netif *netif = NULL;

        if (argc == 1) {
            NETIF_FOREACH(netif) {
                printf("%s%d\n", netif->name, netif->num);
                printf("\tHwaddr:\t%02X:%02X:%02X:%02X:%02X:%02X\n",
                    netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2],
                    netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
                printf("\tIPaddr:\t%u.%u.%u.%u\n",
                    ip4_addr1_16(&netif->ip_addr),
                    ip4_addr2_16(&netif->ip_addr),
                    ip4_addr3_16(&netif->ip_addr),
                    ip4_addr4_16(&netif->ip_addr));
                printf("\tMask:\t%u.%u.%u.%u\n",
                    ip4_addr1_16(&netif->netmask),
                    ip4_addr2_16(&netif->netmask),
                    ip4_addr3_16(&netif->netmask),
                    ip4_addr4_16(&netif->netmask));
                printf("\tMTU:\t%d\n", netif->mtu);
            }
        }
        else if (argc > 2) {
            netif = netif_find(argv[1]);

            if (!netif) {
                printf("Invalid network card\n");
                goto out;
            }

            if (!strcmp(argv[2], "down")) {
                netif_set_down(netif);
            }
            else if (!strcmp(argv[2], "up")) {
                netif_set_default(netif);
                netif_set_up(netif);
            }
            else {
                ip4_addr_t ip;
                if (!inet_aton(argv[2], &ip)) {
                    printf("net ifconfig <net card> <up/down/ip>\n");
                }
                else {
                    ip4_addr_t gw;
                    IP4_ADDR(&gw, ip4_addr1_16(&ip),
                             ip4_addr2_16(&ip), ip4_addr3_16(&ip), 1);
                    netif_set_ipaddr(netif, &ip);
                    netif_set_gw(netif, &gw);
                }
                goto out;
            }
        }
    }
#if LWIP_STATS
    else if (!strcmp(argv[0], "stats")) {
        stats_display();
    }
#endif
    else if (!strcmp(argv[0], "ping")) {
        static ip_addr_t ip_addr;
        ip4_addr_t ip4_addr;
        unsigned long counter = 20;
        char *end_ptr;

        if (argc < 2) {
            printf("net ping <IPaddress> [counter]\n");
            printf("for example:\n");
            printf("net ping 192.168.10.11 10\n");
            goto out;
        }

        if (inet_aton(argv[1], &ip4_addr) == 0) {
            printf("ipaddr %s is wrong\n", argv[1]);
            goto out;
        }

        if (argc == 3) {
            counter = strtoul(argv[2], &end_ptr, 10);
        }
#if PING_USE_SOCKETS
        set_ping_counter(counter);
#endif
        ip_addr_copy_from_ip4(ip_addr, ip4_addr);
        ping_init(&ip_addr);
    }
    else if (!strcmp(argv[0], "iperf")) {
        if (argc < 2) {
            printf("for example:\n");
            printf("net iperf server\n");
            printf("net iperf client 192.168.10.20\n");
            goto out;
        }

        if (!strcmp(argv[1], "server")) {
            LOCK_TCPIP_CORE();
            lwiperf_start_tcp_server_default(lwiperf_report, NULL);
            UNLOCK_TCPIP_CORE();
        }
        else if (!strcmp(argv[1], "client")) {
            if (argc < 3) {
                printf("missing ip addr\n");
                goto out;
            }

            static ip_addr_t ip_addr;
            ip4_addr_t ip4_addr;

            if (inet_aton(argv[2], &ip4_addr) == 0) {
                printf("ipaddr %s is wrong\n", argv[2]);
                goto out;
            }

            ip_addr_copy_from_ip4(ip_addr, ip4_addr);
            LOCK_TCPIP_CORE();
            lwiperf_start_tcp_client_default(&ip_addr, lwiperf_report, NULL);
            UNLOCK_TCPIP_CORE();
        }
        else {
            printf("cmd error: net iperf %s\n", argv[1]);
        }
    }

out:
    return 0;
}

CLI_CMD("net", "net toolbox", net_cmd);
