/**
 * @file pcie_test.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: kunlun pcie test.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <reg.h>
#include <arch/cache.h>
#include "CLI_type.h"
#include "driver/irq.h"
#include "pcie_cfg.h"
#include "dma_cfg.h"

int pcie_init(int argc, char *argv[])
{
    if (g_pcie1_dev.mode == PCIE_RC_MODE) {
        if (pcie_establish_link(&g_pcie1_dev)) {
            printf("PCIe link fails\n");
            return 0;
        }
        else {
            printf("PCIe link succeeds\n");
        }

        pcie_scan_bus(&g_pcie1_dev);
        /* Configure RC own BAR0.
        * For EP access to RC, if not needed, can be ommitted.
        */
        pcie_config_bar(&g_pcie1_dev, 0, 0, 0, 0, 0x10000000);
        /* Configure EP BAR.
        * EP always locates at bus start frome bus 1.
        */
        pcie_config_bar(&g_pcie1_dev, 1, 0, 0, 2, 0x10000000);
        pcie_config_bar(&g_pcie1_dev, 1, 0, 0, 3, 0x60000000);
        pcie_config_bar(&g_pcie1_dev, 1, 0, 0, 4, 0x70000000);
    }
    else
        pcie_establish_link(&g_pcie1_dev);

    return 0;
}

static int pcie_write(int argc, char *argv[])
{
    uint32_t addr, val;
    char *p;

    if (argc == 0) {
        addr = 0x10000000;
        val = 0x12345678;
    }
    else {
        addr = strtoul(argv[0], &p, 16);
        if (argc > 1) {
            val = strtoul(argv[1], &p, 16);
        }
        else {
            val = 0x12345678;
        }
    }

    printf("Write address 0x%x, value 0x%x", addr, val);

    writel(val, addr);
    arch_clean_invalidate_cache_range(addr, 4);

    return 0;
}

static int pcie_read(int argc, char *argv[])
{
    uint32_t addr;
    char *p;

    if (argc == 0) {
        addr = 0x10000000;
    }
    else {
        addr = strtoul(argv[0], &p, 16);
    }

    arch_invalidate_cache_range(addr, 4);
    printf("0x%x: 0x%x\n", addr, readl(addr));

    return 0;
}

static int pcie_own_cfg_header_dump(int argc, char *argv[])
{
    if (argc != 1) {
        printf("Need to input reg number");
        return 0;
    }

    uint32_t reg = atoi(argv[0]);
    uint32_t val;

    reg <<= 2;

    /* Read own configuration header register. */
    g_pcie1_dev.ops->read_config(&g_pcie1_dev, 0, reg, &val);

    printf("Reg %d Val 0x%x\n", reg >> 2, val);

    return 0;
}

static void pcie1_dma_cb(uint32_t status, uint32_t param, void *context)
{
    dma_chan_t *chan = context;
    uint32_t int_status = param;
    uint32_t dma_chn_nr = chan->device->channel_nr;
    uint32_t chn = (chan->chan_id < dma_chn_nr / 2) ? chan->chan_id :
                                        (chan->chan_id - dma_chn_nr / 2);

    printf("%s: channenl %d status %d int status 0x%x\n", __func__,
           chan->chan_id, status, int_status);

    if (status == DMA_COMPLETED) {
        if (int_status & (1 << chn))
            printf("Channel %d transaction done\n", chan->chan_id);
        if (int_status & (1 << (16 + chn)))
            printf("Channel %d aborted\n", chan->chan_id);
    }

    dma_free_chan(chan);
}

static int pcie_dma_test(int argc, char *argv[])
{
    uint32_t chn = 0;
    uint32_t src = 0x70000000, dst = 0x60000000;
    uint32_t len = 0x10000000;

    if (argc >= 1) {
        chn = atoi(argv[0]);
        chn = (chn >= 16) ? 15 : chn;
    }
    if (argc >= 2)
        src = strtoul(argv[1], NULL, 16);
    if (argc >= 3)
        dst = strtoul(argv[2], NULL, 16);
    if (argc >= 4)
        len = strtoul(argv[3], NULL, 16);

    if (chn < 8) {
        dma_invalidate_cache(dst, len);
        printf("PCIe1 DMA read test\n");
    }
    else {
        dma_flush_cache(src, len);
        printf("PCIe1 DMA write test\n");
    }
    printf("\tchannel %d\n", chn);
    printf("\tsrc 0x%x\n", src);
    printf("\tdst 0x%x\n", dst);
    printf("\tlen 0x%x\n", len);

    dma_chan_t *chan = dma_req_chan_with_id(&g_pcie1_dma, chn);
    if (!chan) {
        printf("Request channel %d failed\n", chn);
        return 0;
    }
    chan->context = chan;
    chan->irq_cb = pcie1_dma_cb;
    dma_prep_memcpy(chan, (void *)dst, (void *)src, len, DMA_INTERRUPT);
    dma_start(chan);

    return 0;
}

CLI_CMD("pcie_init", "Init pcie1", pcie_init);
CLI_CMD("pcie_write", "Write pcie1 memory space", pcie_write);
CLI_CMD("pcie_read", "Read pcie1 memory space", pcie_read);
CLI_CMD("pcie_dump", "Dump pcie1 configuration header register", pcie_own_cfg_header_dump);
CLI_CMD("pcie_dma", "PCIe1 DMA test", pcie_dma_test);
