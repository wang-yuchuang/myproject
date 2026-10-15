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
#if defined(WITH_LIB_CONSOLE)
#include <lib/console.h>
#include <stdio.h>
#include <string.h>
#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/inet.h>
#include <lwip/stats.h>
#include <apps/ping/ping.h>
#include "lwip/apps/lwiperf.h"

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

static int net_cmd(int argc, const cmd_args *argv)
{
    if (argc < 2) {
        printf("%s commands:\n", argv[0].str);
        printf("%s ifconfig\n", argv[0].str);
        printf("%s stats\n", argv[0].str);
        printf("%s ping <ip>\n", argv[0].str);
        printf("%s iperf <server | client> [ip]\n", argv[0].str);
        goto out;
    }

    if (!strcmp(argv[1].str, "ifconfig")) {
        struct netif *netif = NULL;

        NETIF_FOREACH(netif) {
            printf("%s\n", netif->name);
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
#if LWIP_STATS
    else if (!strcmp(argv[1].str, "stats")) {
        stats_display();
    }
#endif
    else if (!strcmp(argv[1].str, "ping")) {
        static ip_addr_t ip_addr;
        ip4_addr_t ip4_addr;

        if (argc < 3) {
            printf("net ping <IPaddress>\n");
            printf("for example:\n");
            printf("net ping 192.168.10.11\n");
            goto out;
        }

        if (inet_aton(argv[2].str, &ip4_addr) == 0) {
            printf("ipaddr %s is wrong\n", argv[2].str);
            goto out;
        }

        ip_addr_copy_from_ip4(ip_addr, ip4_addr);
        ping_init(&ip_addr);
    }
    else if (!strcmp(argv[1].str, "iperf")) {
        if (argc < 3) {
            printf("for example:\n");
            printf("net iperf server\n");
            printf("net iperf client 192.168.10.20\n");
            goto out;
        }

        if (!strcmp(argv[2].str, "server")) {
            lwiperf_start_tcp_server_default(lwiperf_report, NULL);
        }
        else if (!strcmp(argv[2].str, "client")) {
            if (argc < 4) {
                printf("missing ip addr\n");
                goto out;
            }

            static ip_addr_t ip_addr;
            ip4_addr_t ip4_addr;

            if (inet_aton(argv[3].str, &ip4_addr) == 0) {
                printf("ipaddr %s is wrong\n", argv[3].str);
                goto out;
            }

            ip_addr_copy_from_ip4(ip_addr, ip4_addr);
            lwiperf_start_tcp_client_default(&ip_addr, lwiperf_report, NULL);
        }
        else {
            printf("cmd error: %s iperf %s\n", argv[0].str, argv[2].str);
        }
    }

out:
    return 0;
}

STATIC_COMMAND_START
STATIC_COMMAND("net", "net toolbox", &net_cmd)
STATIC_COMMAND_END(net);

#endif
