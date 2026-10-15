#include <driver/uart.h>
#include <errno.h>
#include <reg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../../../drivers/taishan/dma/sdrv_dmac.h"

#include "CLI.h"
#include "board.h"
#include "driver/dma.h"
#include "driver/taishan/pinctrl/taishan_pinctrl.h"
#include "regs_base.h"
#include "uart_cfg.h"

#ifdef CONFIG_SDRV_UART
#define UART_TX UART5
#define UART_RX UART5
#define UART_TX_FIFO_OFFSET 0x200
#define UART_RX_FIFO_OFFSET 0x300

#else
#define UART_TX_FIFO_OFFSET 0x0
#define UART_TX UART3
#define UART_RX_FIFO_OFFSET 0x0
#define UART_RX UART3
#endif
#define INFO(fmt, ...)                                                         \
    printf(" %04d %s: " fmt, __LINE__, __func__, ##__VA_ARGS__)
#define ERROR(fmt, ...)                                                         \
    printf(" %04d ### ERR ### %s: " fmt, __LINE__, __func__, ##__VA_ARGS__)

#define FUNC_IN INFO("in...");

enum dma_xfer_t { MEMCPY, MEMSET, MEM2DEV, DEV2MEM };
#ifdef CONFIG_SDRV_DMA
extern dma_dev_t dma_sf0;
#endif
#define MEMCPY_SZ (8 * 1024)
static unsigned char dma_test_src[MEMCPY_SZ] __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};
static unsigned char dma_test_dst[MEMCPY_SZ] __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};
static uint8_t dev_src[1024 * 2] __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};
static uint8_t dev_dst[1024 * 2] __attribute__((__aligned__(CONFIG_ARCH_CACHE_LINE))) = {0};

struct app_priv {
    int xfer_count;
    osEventFlagsId_t event;
    dma_chan_t *chan;
};

/*irq callback
 * status  - err or success
 * param   - err code
 * context - private data
 */
static void app_single_xfer_done(uint32_t status, uint32_t param, void *context)
{
    struct app_priv *priv = (struct app_priv *)context;
    if (DMA_COMPLETED == status)
        osEventFlagsSet(priv->event, 1);
}

static void app_cyclic_item_done(uint32_t status, uint32_t param, void *context)
{
    struct app_priv *priv = (struct app_priv *)context;
    if (DMA_COMPLETED == status) {
        priv->xfer_count++;
    }
    if (5 == priv->xfer_count) // Customized according to actual application
                               // scenarios
    {
        osEventFlagsSet(priv->event, 1);
    }
}

static void printf_buf(uint8_t *buf, int len)
{
    printf("\n###### printf %p ##### \n", buf);
    for (int i = 0; i < len; i++) {
        printf("%#x ", buf[i]);
        if (i % 15 == 0)
            printf("\n");
    }
    printf("\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
}

static int app_dma_mem2mem(enum dma_xfer_t type)
{
    FUNC_IN
    int ret = 0;
    struct app_priv priv = {
        .xfer_count = 0,
        .event = osEventFlagsNew(NULL),
    };

    /* prepare data */
    memset(dma_test_src, 0, MEMCPY_SZ);
    memset(dma_test_dst, 0, MEMCPY_SZ);
    for (int index = 0; index < MEMCPY_SZ; index++) {
        dma_test_src[index] = '1' + (index % 8);
    }

    /* require dma channel */
    dma_chan_t *chan = dma_req_chan(&dma_sf0);
    if (!chan) {
        ERROR("req chan failed \n");
        ret = -1;
        goto err;
    }

    /* fill dma xfer done callback */
    chan->irq_cb = app_single_xfer_done;
    priv.chan = chan;
    chan->context = (void *)&priv;

    if (type == MEMCPY) {
        ret = dma_prep_memcpy(chan, dma_test_dst, dma_test_src, MEMCPY_SZ,
                              DMA_INTERRUPT);
    } else if (type == MEMSET) {
        ret =
            dma_prep_memset(chan, 0x5A, dma_test_dst, MEMCPY_SZ, DMA_INTERRUPT);
    }
    if (!ret) {
        ERROR("prep chan failed \n");
        ret = -1;
        goto err;
    }
    /* clean cache (or alloc no-cache buffer) */
    dma_flush_cache((addr_t)dma_test_src, MEMCPY_SZ);
    dma_invalidate_cache((addr_t)dma_test_dst, MEMCPY_SZ);

    /* trigger dma */
    dma_start(chan);

    osEventFlagsWait(priv.event, 1, osFlagsWaitAny, osWaitForever);

    /* xfer dont stop dma */
    dma_stop(chan);
    /* free dma chan
     * rejoin the pool for use by other modules
     */
    dma_free_chan(chan);

    INFO("memcpy/memset usecase done! \n\n");
    if (type == MEMCPY) {
        for (int i = 0; i < MEMCPY_SZ; i++) {
            if (dma_test_dst[i] != dma_test_src[i])
                printf("idex: %d , src: %#02x , dst: %#02x \n", i,
                       dma_test_src[i], dma_test_dst[i]);
        }
    } else if (type == MEMSET) {
        for (int i = 0; i < MEMCPY_SZ; i++) {
            if (dma_test_dst[i] != 0x5a)
                printf("idex: %d , dst: %#02x \n", i, dma_test_dst[i]);
        }
    }
    // printf_buf(dma_test_dst, 1024);

err:
    osEventFlagsDelete(priv.event);
    return ret;
}

static int app_dma_dev_single(enum dma_xfer_t type)
{
    FUNC_IN
    int ret = 0;
    char *str = "This is an Semidrive DMA application test message";
    int len = strlen(str);
    struct dma_dev_cfg dma_cfg = {0};
    struct app_priv priv = {
        .xfer_count = 0,
        .event = osEventFlagsNew(NULL),
    };
    /* prepare data */
    memset(dev_src, 0, sizeof(dev_src));
    dma_flush_cache((addr_t)dev_src, sizeof(dev_src));

    memset(dev_dst, 0, sizeof(dev_dst));
    dma_flush_cache((addr_t)dev_dst, sizeof(dev_dst));

    for (int index = 0; index < sizeof(dev_src); index++) {
        dev_src[index] = '1' + (index % 8);
    }
    strncpy((char *)dev_src, str,
            len > sizeof(dev_src) ? sizeof(dev_src) : len);

    /* require dma channel */
    dma_chan_t *chan = dma_req_chan(&dma_sf0);
    if (!chan) {
        ERROR("req chan failed \n");
        ret = -1;
        goto err;
    }

    /* fill dma xfer done callback */
    chan->irq_cb = app_single_xfer_done;
    priv.chan = chan;
    chan->context = (void *)&priv;

    /* set transfer config (only for mem2dev/dev2mem) */
    if (type == MEM2DEV) {
        dma_cfg.direction = DMA_MEM2DEV;
        dma_cfg.src_addr = (addr_t)dev_src;
        #ifdef CONFIG_DMA_UART_PREPARED
        dma_cfg.dst_addr =
            g_dma_uart->base + UART_TX_FIFO_OFFSET; // uart tx fifo address
        #else
        dma_cfg.dst_addr =
            DEVICE_BASE(UART_TX) + UART_TX_FIFO_OFFSET; // uart tx fifo address
        #endif
        dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.src_maxburst = 16;
        dma_cfg.dst_maxburst = 16; // tx water level 0xf
        dma_config_chan(chan, &dma_cfg);
        ret = dma_prep_dev(chan, dev_src, 64, DMA_INTERRUPT);
    } else if (type == DEV2MEM) {
        dma_cfg.direction = DMA_DEV2MEM;
        #ifdef CONFIG_DMA_UART_PREPARED
        dma_cfg.src_addr =
            g_dma_uart->base + UART_RX_FIFO_OFFSET; // uart rx fifo address
        #else
        dma_cfg.src_addr =
            DEVICE_BASE(UART_RX) + UART_RX_FIFO_OFFSET; // uart rx fifo address
        #endif
        dma_cfg.dst_addr = (addr_t)dev_dst;
        /* sdrv uart wml must set to 0x0->1bytes */
        dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.src_maxburst = 1; // rx water level 0x0
        dma_cfg.dst_maxburst = 1;

        dma_config_chan(chan, &dma_cfg);
        ret = dma_prep_dev(chan, dev_dst, 64, DMA_INTERRUPT);
        printf("\n\n Fill in 64 characters from uart: \n");
    }
    if (!ret) {
        ERROR("prep chan failed \n");
        ret = -1;
        goto err;
    }

    /* clean cache (or alloc no-cache buffer) */
    dma_flush_cache((addr_t)dev_src, sizeof(dev_src));

    /* trigger dma */
    dma_start(chan);

    osEventFlagsWait(priv.event, 1, osFlagsWaitAny, osWaitForever);
    dma_stop(chan);
    dma_free_chan(chan);
    INFO("dma xfer done\n");
    dma_invalidate_cache((addr_t)dev_dst, sizeof(dev_dst));
    char out[2048];
    if (type == DEV2MEM) {
        sprintf(out, "%s", dev_dst);
        printf("%s \n", out);
    }

    osEventFlagsDelete(priv.event);
    return ret;

err:
    osEventFlagsDelete(priv.event);
    if (chan)
        dma_free_chan(chan);
    return ret;
}

static int app_dma_dev_cyclic(enum dma_xfer_t type)
{
    FUNC_IN
    int ret = 0;
    char *str = "This is an Semidrive DMA application test message";
    int len = strlen(str);
    struct dma_dev_cfg dma_cfg = {0};
    struct app_priv priv = {
        .xfer_count = 0,
        .event = osEventFlagsNew(NULL),
    };
    /* prepare data */
    memset(dev_src, 0, sizeof(dev_src));
    dma_flush_cache((addr_t)dev_src, sizeof(dev_src));

    memset(dev_dst, 0, sizeof(dev_dst));
    dma_flush_cache((addr_t)dev_dst, sizeof(dev_dst));

    for (int index = 0; index < sizeof(dev_src); index++) {
        dev_src[index] = '1' + (index % 8);
    }
    strncpy((char *)dev_src, str,
            len > sizeof(dev_src) ? sizeof(dev_src) : len);

    /* require dma channel */
    dma_chan_t *chan = dma_req_chan(&dma_sf0);
    if (!chan) {
        ERROR("req chan failed \n");
        ret = -1;
        goto err;
    }

    /* fill dma xfer done callback */
    chan->irq_cb = app_cyclic_item_done;
    priv.chan = chan;
    chan->context = (void *)&priv;

    /* set transfer config (only for mem2dev/dev2mem) */
    if (type == MEM2DEV) {
        dma_cfg.direction = DMA_MEM2DEV;
        dma_cfg.src_addr = (addr_t)dev_src;
        #ifdef CONFIG_DMA_UART_PREPARED
        dma_cfg.dst_addr =
            g_dma_uart->base + UART_TX_FIFO_OFFSET; // uart tx fifo address
        #else
        dma_cfg.dst_addr =
            DEVICE_BASE(UART_TX) + UART_TX_FIFO_OFFSET; // uart tx fifo address
        #endif
        dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.src_maxburst = 16; // tx water level 0xf
        dma_cfg.dst_maxburst = 16;
        dma_config_chan(chan, &dma_cfg);
        ret = dma_prep_cyclic(chan, dev_src, 64, 32, DMA_INTERRUPT);
    } else if (type == DEV2MEM) {
        dma_cfg.direction = DMA_DEV2MEM;
        #ifdef CONFIG_DMA_UART_PREPARED
        dma_cfg.src_addr =
            g_dma_uart->base + UART_RX_FIFO_OFFSET; // uart rx fifo address
        #else
        dma_cfg.src_addr =
            DEVICE_BASE(UART_RX) + UART_RX_FIFO_OFFSET; // uart rx fifo address
        #endif
        dma_cfg.dst_addr = (addr_t)dev_dst;
        dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.src_maxburst = 1; // rx water level 0x0
        dma_cfg.dst_maxburst = 1;

        dma_config_chan(chan, &dma_cfg);
        ret = dma_prep_cyclic(chan, dev_dst, 64, 16, DMA_INTERRUPT);
        printf("\n\n Fill in 16 characters from uart: \n");
    }
    if (!ret) {
        ERROR("prep chan failed \n");
        ret = -1;
        goto err;
    }
    /* clean cache (or alloc no-cache buffer) */
    dma_flush_cache((addr_t)dev_src, sizeof(dev_src));

    /* trigger dma */
    dma_start(chan);

    osEventFlagsWait(priv.event, 1, osFlagsWaitAny, osWaitForever);

    // Limited by the scheduling speed of the CPU, the DMA may still continue to
    // transfer some data.
    // You need to handle this piece of data in your application.
    dma_stop(chan);
    dma_free_chan(chan);

    dma_invalidate_cache((addr_t)dev_dst, sizeof(dev_dst));
    INFO("dma xfer done\n");
    char out[2048];
    if (type == DEV2MEM) {
        sprintf(out, "%s", dev_dst);
        printf("%s \n", out);
    }

err:
    osEventFlagsDelete(priv.event);
    return ret;
}

static void usage(const char *cmd)
{
    printf("This is DMA app demo\n");
    printf("command options:\n");
    printf("%s -h\t\t\t\t:Show this help info\n", cmd);
    printf("%s -r memcpy\t\t\t:DMA memcpy app\n", cmd);
    printf("%s -r memset\t\t\t:DMA memset app\n", cmd);
    printf("%s -r mem2dev\t\t\t:DMA mem2dev app, will send 64 character to uart\n", cmd);
    printf("%s -r mem2dev cyclic\t\t:DMA mem2dev cyclic app, will send 32 character to uart\n", cmd);
    printf("%s -r dev2mem\t\t\t:DMA dev2mem app, will accept 64 characters\n", cmd);
    printf("%s -r dev2mem cyclic\t\t:DMA dev2mem cyclic app, will accept 16 characters\n", cmd);
}

static int app_dma_cmd(int argc, char *argv[])
{

    if (!strcmp("-r", argv[0])) {
        if (!strcmp("memcpy", argv[1])) {
            INFO("run usecase %s \n", argv[1]);
            app_dma_mem2mem(MEMCPY);
            return 0;
        } else if (!strcmp("memset", argv[1])) {
            INFO("run usecase %s \n", argv[1]);
            app_dma_mem2mem(MEMSET);
            return 0;
        } else if (!strcmp("mem2dev", argv[1])) {
            INFO("run usecase %s argc: %d\n", argv[1], argc);
            if (argc == 3 &&
                !strcmp("cyclic", argv[2])) /* app_dma -r mem2dev cyclic*/
            {
                app_dma_dev_cyclic(MEM2DEV);
                return 0;

            } else {
                app_dma_dev_single(MEM2DEV);
                return 0;
            }
        } else if (!strcmp("dev2mem", argv[1])) {
            INFO("run usecase %s \n", argv[1]);
            if (argc == 3 &&
                !strcmp("cyclic", argv[2])) /* app_dma -r mem2dev cyclic*/
            {
                app_dma_dev_cyclic(DEV2MEM);
                return 0;

            } else {
                app_dma_dev_single(DEV2MEM);
                return 0;
            }
            return 0;
        }
    }
    usage("app_dma");
    return 0;
}
CLI_CMD("app_dma", "\r\napp_dma:\r\n Example app implement with dma\r\n",
        app_dma_cmd);
