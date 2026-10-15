//*****************************************************************************
//
// ulink_net.c - Driver for rpmsg vnet.
//
// Copyright (c) 2019-2029 SemiDrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************
#include "ulink.h"

#include <netdev.h>
#include "driver/kunlun/pcie/dwc_pcie.h"
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <lwip/prot/ethernet.h>
#include <lwip/opt.h>
#include <driver/dma.h>
#include <debug.h>

#include <param.h>
#include <arch/barriers.h>
#include <arch/cache.h>
#include <driver/memmap.h>

#include "regs_base.h"
#include "pcie_cfg.h"

#include <driver/gpio.h>


#define v2p(va) (paddr_t)(va)
#define p2v(pa) (vaddr_t)(pa)

#ifdef CONFIG_UNILINK

#define ULINK_NET_NS0 172
#define ULINK_NET_NS1 20
#define ULINK_NET_NS2 2
#define ULINK_IP_OFFSET 32

#ifdef CONFIG_ULINK_SIDE_B
#define ULINK_NET_NS3_BASE (ULINK_IP_OFFSET + DP_CPU_MAX)
#else
#define ULINK_NET_NS3_BASE (ULINK_IP_OFFSET)
#endif

#ifdef CONFIG_MP
#define ULINK_NET_NS3 (ULINK_NET_NS3_BASE + DP_CR5_MPC)
#else
#define ULINK_NET_NS3 (ULINK_NET_NS3_BASE + DP_CR5_SAF)
#endif

#endif

#ifdef CONFIG_DW_DMA
extern dma_dev_t* dma_dev;
#endif

#define UC_PACK_SIZE 0x100
#define ULINK_ETH_MAGIC 0x5a5a
#define ULINK_RET_MAGIC 0xa5a5
#define ULINK_IP_OFFSET 32
#define PCIE1_MAP_ADDR 0x1c002000

struct ulink_net {
    ulink_channel_t *ch;
    uint32_t rproc;
    struct net_driver_s *net;
    uint32_t frame;
    uint32_t recived;
    int q_index;
    struct pbuf *p;
    struct pbuf *q;
};

struct ulink_eth_header {
    uint16_t magic;
    uint16_t tot_len;
    uint16_t sec_id;
    uint16_t sec_len;
    uint64_t data_pa;
    uint64_t skb_va;

} __attribute__((packed));

struct net_driver_s g_net;
struct ulink_net *g_unets[ALL_DP_CPU_MAX];

#define ULINK_BLOCK_SIZE_MAX TCP_MSS+0x200
#define ULINK_CACHE_LEN CONFIG_ARCH_CACHE_LINE


#define IRAM_SHADOW (SERVER_BASE + 0x1800000)
#define iram2shadow(a) (a - IRAM1_BASE + IRAM_SHADOW)
#define shadow2iram(a) (a - IRAM_SHADOW + IRAM1_BASE)

static bool use_iram_shadow = false;


static void print_pbuf(struct pbuf *p)
{
    struct pbuf *q;
    int i;
    uint8_t *temp;

    printf("print pbuf start\n");
    for (q = p; q != NULL; q = q->next) {
        temp = q->payload;
        for (i = 0; i < q->len; i++) {
            printf("%x ", temp[i]);
        }
    }
    printf("\nprint pbuf end size %d\n", p->tot_len);
}
#define PCIE1_DBI_BASE  APB_PCIE1_1_BASE
static int memcpy_pcie(uint64_t dst, uint64_t src, int size)
{
    int ret = 0;


#ifdef CONFIG_ULINK_SIDE_A
    ret = pcie_dma_write(&g_pcie1_dev, 4, dst, src, size);
#endif

#ifdef CONFIG_ULINK_SIDE_B
    ret = pcie_dma_read(&g_pcie1_dev, 5, dst, src, size);
#endif
    return ret;
}
static void ulink_single_xfer_done(uint32_t status, uint32_t param,
                                   void *context)
{
    osEventFlagsId_t event = (osEventFlagsId_t)context;
    if (DMA_COMPLETED == status) {
        osEventFlagsSet(event, 1);
    }
}


static void memcpy_dma(uint64_t dst, uint64_t src, int size)
{
    dma_chan_t *chan = dma_req_chan(dma_dev);
    osEventFlagsId_t event = osEventFlagsNew(NULL);

    uint64_t psrc;
    uint64_t pdst;
    int dsize;
    unsigned char *dma_test_dst;

    if(size > ULINK_BLOCK_SIZE_MAX)
    {
        printf("ulink net large packet over TCP_MSS\n");
        dma_free_chan(chan);
        osEventFlagsDelete(event);
        return;
    }

    dma_test_dst = memalign(ULINK_CACHE_LEN, ULINK_BLOCK_SIZE_MAX+ULINK_CACHE_LEN);
    if (dma_test_dst == NULL) {
        printf("ulink net large packet malloc failed\n");
        dma_free_chan(chan);
        osEventFlagsDelete(event);
        return;
    }

    chan->irq_cb = ulink_single_xfer_done;
    chan->context = (void *)event;
    src = lap2p(src);
    psrc = src/ULINK_CACHE_LEN*ULINK_CACHE_LEN;
    dsize = size/ULINK_CACHE_LEN*ULINK_CACHE_LEN+ULINK_CACHE_LEN;
    pdst = v2p(dma_test_dst);
    if (use_iram_shadow)
        pdst = iram2shadow(pdst);
    dma_prep_memcpy64(chan, pdst, psrc, dsize,DMA_INTERRUPT);
    dma_flush_cache((addr_t)psrc, dsize);
    dma_invalidate_cache((addr_t)pdst, dsize);

    dma_start(chan);

    osEventFlagsWait(event, 1, osFlagsWaitAny, osWaitForever);

    dma_stop(chan);
    dma_free_chan(chan);
    if (use_iram_shadow)
        memcpy((void *)(addr_t)(dst), iram2shadow(&dma_test_dst[src-psrc]),size);
    else
        memcpy((void *)(addr_t)(dst), &dma_test_dst[src-psrc],size);

    free(dma_test_dst);
    osEventFlagsDelete(event);
}

static uint32_t get_dest_ch(void *payload, struct net_driver_s *dev)
{
    struct eth_hdr *ethhdr = (struct eth_hdr *)payload;
    uint8_t *tmp = (uint8_t *)payload;
    uint8_t addr1, addr2;

    tmp += SIZEOF_ETH_HDR;

    switch (ethhdr->type) {
    case PP_HTONS(ETHTYPE_IP):
        tmp += 16;
        break;
    case PP_HTONS(ETHTYPE_ARP):
        tmp += 24;
        break;
    default:
        return DEFAULT_RPROC;
    }
    if ((tmp[0] == ULINK_NET_NS0) && (tmp[1] == ULINK_NET_NS1) &&
        (tmp[2] == ULINK_NET_NS2)) {
        addr1 = ((ip_2_ip4(dev->d_ipaddr).addr) >> 24) & 0xff;
        addr2 = tmp[3];
        if ((addr1 < ULINK_IP_OFFSET) || (addr2 < ULINK_IP_OFFSET))
            return DEFAULT_RPROC;
        addr1 -= ULINK_IP_OFFSET;
        addr2 -= ULINK_IP_OFFSET;
        if (addr1 < DP_CPU_MAX)
            return addr2;
        else if (addr2 < DP_CPU_MAX)
            return addr2 + DP_CPU_MAX;
        else
            return addr2 - DP_CPU_MAX;
    }

    return DEFAULT_RPROC;
}
//extern gpio_dev_t g_gpio_dev;

static err_t ulink_net_tx(struct net_driver_s *dev, struct pbuf *p)
{
    struct ulink_net *unet;
    struct ulink_eth_header header;
    struct pbuf *q;
    int len;
    uint32_t dest;
    uint8_t *tmp;
    uint16_t id;
#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

    dest = get_dest_ch(p->payload, dev);
    if (unlikely(dest > ALL_DP_CPU_MAX)) {
        printf("dest is not found%d\n", dest);
        return 0;
    }

    unet = g_unets[dest];
    if (!unet) {
        printf("dest is not found%d\n", dest);
        return 0;
    }

    if ((p->tot_len > UC_PACK_SIZE) || (p->tot_len != p->len)) {
        header.magic = ULINK_ETH_MAGIC;
        header.tot_len = p->tot_len;

        pbuf_ref(p); // free async
        for (q = p, id = 0; q != NULL; q = q->next, id++) {
            header.sec_id = id;
            header.sec_len = q->len;
            if (use_iram_shadow)
                header.data_pa = p2ap(iram2shadow(v2p(q->payload)));
            else
                header.data_pa = p2ap(v2p(q->payload));
            header.skb_va = (uint64_t)((uint32_t)p);
            if (use_iram_shadow) {
                memcpy((char *)(iram2shadow(q->payload)), q->payload, q->len);
                arch_clean_cache_range((addr_t)(iram2shadow(q->payload)), q->len);
	    } else {
                arch_clean_cache_range((addr_t)(q->payload), q->len);
	    }
            ulink_channel_send(unet->ch, (uint8_t *)&header,
                               sizeof(struct ulink_eth_header));
        }
    } else {
      ulink_channel_send(unet->ch, p->payload, p->len);
    }

#if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
    return 0;
}

static void ulink_net_store_rx_data(struct ulink_net *unet, void *data, int len)
{
    int remain = len;
    int cpy_len;
    uint8_t *temp = data;

    while (remain) {
        if (unet->q_index == unet->q->len) {
            unet->q = unet->q->next;
            unet->q_index = 0;
        }
        cpy_len = MIN(remain, (unet->q->len - unet->q_index));
        memcpy(unet->q->payload + unet->q_index, temp, cpy_len);
        remain -= cpy_len;
        temp += cpy_len;
        unet->q_index += cpy_len;
    }
    unet->recived += len;
}

static void ulink_net_store_rx_data_large(struct ulink_net *unet, uint64_t pa,
                                          int len)
{
    int remain = len;
    int cpy_len;
    uint64_t offset = 0;
    uint8_t *temp;

    while (remain) {
        if (unet->q_index == unet->q->len) {
            unet->q = unet->q->next;
            unet->q_index = 0;
        }
        cpy_len = MIN(remain, (unet->q->len - unet->q_index));
        temp = unet->q->payload + unet->q_index;
        arch_invalidate_cache_range(v2p(temp), cpy_len);
       if (unet->rproc < DP_CPU_MAX)
         memcpy_dma((paddr_t)(temp), pa + offset, cpy_len);
        else
          memcpy_pcie(p2ap(v2p(temp)), pa + offset, cpy_len);
        remain -= cpy_len;
        offset += cpy_len;
        unet->q_index += cpy_len;
    }
    unet->recived += len;
}

static void ulink_net_rx(ulink_channel_t *ch, uint8_t *data, uint32_t size)
{
    struct ulink_net *unet = ch->user_data;
    struct ulink_eth_header *header = (struct ulink_eth_header *)data;
    struct ulink_eth_header ret_header;
    uint16_t magic;
    if (size == sizeof(struct ulink_eth_header)){
        magic = header->magic;
    }
    else{
        magic = 0;
    }

    switch (magic) {
    case ULINK_ETH_MAGIC:
        if (header->sec_id == 0) {
            unet->frame = header->tot_len;
            unet->recived = 0;
            unet->q_index = 0;
#if ETH_PAD_SIZE
            unet->frame += ETH_PAD_SIZE;
#endif
            unet->p = pbuf_alloc(PBUF_RAW, unet->frame, PBUF_POOL);
            if (unet->p) {
#if ETH_PAD_SIZE
                pbuf_remove_header(unet->p,
                                   ETH_PAD_SIZE); /* drop the padding word */
#endif
                unet->q = unet->p;
            }
        }
        if (!unet->q) {
            unet->recived += unet->q->len;
            if (unet->frame == unet->recived)
                unet->frame = 0;
            return;
        }

        ulink_net_store_rx_data_large(unet, header->data_pa, header->sec_len);
        if (unet->frame == unet->recived) {
            ret_header.magic = ULINK_RET_MAGIC;
            ret_header.skb_va = header->skb_va;
            ulink_channel_send(unet->ch, (uint8_t *)&ret_header,
                               sizeof(struct ulink_eth_header));
        }
        break;
    case ULINK_RET_MAGIC:
        pbuf_free((struct pbuf *)((uint32_t)(header->skb_va)));
        return;
    default:
        unet->frame = size;
        unet->recived = 0;
        unet->q_index = 0;
#if ETH_PAD_SIZE
        unet->frame += ETH_PAD_SIZE;
#endif
        unet->p = pbuf_alloc(PBUF_RAW, unet->frame, PBUF_POOL);
        if (unet->p) {
#if ETH_PAD_SIZE
            pbuf_remove_header(unet->p,
                               ETH_PAD_SIZE); /* drop the padding word */
#endif
            unet->q = unet->p;
        }
        if (!unet->q) {
            unet->recived += size;
            if (unet->frame == unet->recived)
                unet->frame = 0;
            return;
        }
        ulink_net_store_rx_data(unet, data, size);
        break;
    }

    if (unet->frame == unet->recived) {
#if ETH_PAD_SIZE
        pbuf_add_header(unet->p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
      netdev_input(unet->net, unet->p);
      unet->frame = 0;
        unet->recived = 0;
        unet->p = NULL;
        unet->q = NULL;
        unet->q_index = 0;
    }
}

static uint32_t ulink_net_getmac(struct net_driver_s *dev, void *buf,
                                 uint32_t max_len)
{
    struct ulink_net *unet;
    uint32_t ll;
    static unsigned char mac[6];

    mac[0] = 0xAB;
    mac[1] = 0xBC;
    mac[2] = 0xCD;
#ifdef CONFIG_ULINK_SIDE_B
    mac[3] = 0xB;
#else
    mac[3] = 0xA;
#endif
#ifdef CONFIG_MP
    mac[4] = 0x2;
#else
    mac[4] = 0x0;
#endif
    mac[5] = 0XE;
    ll = MIN(sizeof(mac), max_len);
    memcpy(buf, mac, ll);
    return ll;
}

static uint32_t ulink_net_getmtu(struct net_driver_s *dev) { return 65500; }

void ulink_net_probe(uint32_t rproc)
{
    struct ulink_net *unet;

    unet = calloc(1, sizeof(struct ulink_net));
    if (!unet)
        return;

    unet->net = &g_net;
    unet->ch = ulink_request_channel(rproc, 0xE, ulink_net_rx);
    if (unet->ch == NULL) {
        free(unet);
        ssdk_printf(SSDK_EMERG, "%s request channel failed for rproc: %d\n",__func__, rproc);
        return ;
    }
    unet->ch->rproc = rproc;
    unet->ch->user_data = unet;
    unet->rproc = rproc;
    g_unets[rproc] = unet;
}
int ulink_net_init(void)
{
    static bool onece = false;

    if (onece)
        return 0;
    else
        onece = true;

    if ((uint32_t)(CONFIG_MEM_BASE) < (uint32_t)(DDR_PHY_BASE))
        use_iram_shadow = true;

    IP_ADDR4(&g_net.d_gateway, ULINK_NET_NS0, ULINK_NET_NS1, ULINK_NET_NS2, DEFAULT_RPROC + ULINK_IP_OFFSET);
    IP_ADDR4(&g_net.d_ipaddr, ULINK_NET_NS0, ULINK_NET_NS1, ULINK_NET_NS2, ULINK_NET_NS3);
    IP_ADDR4(&g_net.d_netmask, 255, 255, 255, 0);
#ifdef CONFIG_MP
    ulink_net_probe(DP_CR5_SAF);
#else
    ulink_net_probe(DP_CR5_MPC);
#endif
    ulink_net_probe(DP_CA_AP1);
    ulink_net_probe(DP_CA_AP2);
#if (defined CONFIG_ULINK_SIDE_A) || (defined CONFIG_ULINK_SIDE_B)
/* for X9U  board */
    ulink_net_probe(R_DP_CR5_SAF);
    ulink_net_probe(R_DP_CR5_MPC);
    ulink_net_probe(R_DP_CA_AP1);
    ulink_net_probe(R_DP_CA_AP2);
#endif

    g_net.d_txavail = ulink_net_tx;
    g_net.d_getmac = ulink_net_getmac;
    g_net.d_getmtu = ulink_net_getmtu;
    g_net.d_private = g_unets;
    netdev_register(&g_net, NET_LL_ETHERNET);
    return 0;
}
