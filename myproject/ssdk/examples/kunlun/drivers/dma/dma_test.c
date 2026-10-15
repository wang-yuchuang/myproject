#include <driver/kunlun/uart/dw_uart_drv.h>
#include <driver/uart.h>
#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CLI.h"
#include "board.h"
#include "driver/dma.h"
#include "regs_base.h"

#define ENABLE 1
#define DISABLE 0

#ifdef CONFIG_DW_DMA
#define UART_TX_FIFO_OFFSET 0
// #define UART_TX UART3
#define UART_RX_FIFO_OFFSET 0
// #define UART_RX UART3
extern uart_dev_t *g_uart_13;
#define UART_TX UART13
#define UART_RX UART13
#endif

#define MEMCPY_SZ (1024 * 1024)

extern dma_dev_t *dma_dev;

enum test_type {
    TEST_MEM2MEM_MEMSET,
    TEST_MEM2MEM_MEMCPY,
    TEST_MEM2DEV_SINGLE,
    TEST_MEM2DEV_CYCLIC,
    TEST_DEV2MEM_SINGLE,
    TEST_DEV2MEM_CYCLIC,
} test_type_t;

static uint8_t uart_test_src[1024 * 1] __attribute__((__aligned__(32))) = {0};
static uint8_t uart_test_dst[1024 * 1] __attribute__((__aligned__(32))) = {0};

// #pragma section("L1_data")
// static uint8_t gUartTransmit __attribute__((__aligned__(32))); //
// Uart发送数据区

// #pragma section("L1_data")
// static uint8_t gUartReceive __attribute__((__aligned__(32))); //
// Uart接收数据区

static volatile uint8_t dma_tr_done_flag = 0;
int32_t cyclic_every_xfer_done_count = 0;

#ifndef udelay
#define udelay(x)                                                              \
    {                                                                          \
        volatile uint64_t count = 20ull * (uint64_t)x;                         \
        while (count != 0ull)                                                  \
            count--;                                                           \
    }
#define DELAY(n) udelay(n)
#else
#define DELAY(n) osDelay(n)
#endif

/*
 * Uart Recv Callback function.
 */
static void UartReadCallback(void *arg) { printf("UartReadCallback...\n"); }

/*irq callback
 * status  - err or success
 * param   - err code
 * context - private data
 */
static void dma_transfer_done(uint32_t status, uint32_t param, void *context)
{
    if (DMA_COMPLETED == status)
        dma_tr_done_flag = 1;

    printf("\r\ndma transfer irq_callback:%x,%x,%x\r\n", status, param,
           (uint32_t)context);
}

/*irq callback
 * status  - err or success
 * param   - err code
 * context - private data
 */
static void dma_transfer_every_mad_done(uint32_t status, uint32_t param,
                                        void *context)
{
    if (DMA_COMPLETED == status) {
        cyclic_every_xfer_done_count++;
    }
    if (cyclic_every_xfer_done_count == 3) {
        dma_tr_done_flag = 1;
    }
    // printf("mad: %d, tr: %d\n", cyclic_every_xfer_done_count,
    // dma_tr_done_flag);
}

/*test cmd : uart_test mem2mem memcpy  (memcpy test)
  uart_test mem2mem memset  (memset test)
  uart_test mem2dev         (uart write)
  uart_test mem2dev cyclic  (uart write loop)
  uart_test dev2mem         (uart read)
  uart_test dev2mem cyclic  (uart read loop)
 */

void *aligned_malloc(size_t bytes, size_t alignedment)
{
    void *malloc_ptr;
    void *aligned_ptr;
    if (alignedment == 0 || alignedment == 1)
        return malloc(bytes);
    else if ((alignedment & (alignedment - 1)) != 0) {
        return NULL;
    }
    malloc_ptr = malloc(bytes + alignedment);
    if (!malloc_ptr)
        return NULL;
    aligned_ptr = (void *)(((size_t)malloc_ptr + alignedment) &
                           ~((size_t)alignedment - 1));

    ((void **)aligned_ptr)[-1] = malloc_ptr;
    return aligned_ptr;
}

static int dma_test_main(int argc, char *argv[])
{
    dma_chan_t *chan = NULL;
    uint8_t tr_flag = 0;
    uint8_t tr_ret = 0;
    uint32_t src_len = sizeof(uart_test_src), dst_len = sizeof(uart_test_dst);
    struct dma_dev_cfg dma_cfg = {0};
    // TODO: use the scr driver api

    printf("uart_test_src len=%d, uart_test_dst len=%d\n", src_len, dst_len);

    memset(uart_test_src, 0, src_len);
    memset(uart_test_dst, 0x5A, dst_len);

    for (int index = 0; index < src_len; index++) {
        uart_test_src[index] = (index % 128) + 1;
    }

    /* 1.request channel (Dynamic request or specific dma channel) */
    if (!strcmp(argv[0], "mem2dev") || !strcmp(argv[0], "dev2mem")) {
        chan = dma_req_chan(dma_dev);

        if (NULL == chan) {
            printf("alloc channel failed.\r\n");
            return -1;
        }
    } else {
        chan = dma_req_chan_with_id(dma_dev, 1);

        if (NULL == chan) {
            printf("alloc channel failed.\r\n");
            return -1;
        }
    }

    /* 2.set transfer config */
    chan->context = (void *)0x5AA5;
    chan->irq_cb = dma_transfer_done;
    if (!strcmp(argv[0], "mem2mem")) {
        if (!strcmp(argv[1], "memcpy")) {
            tr_ret = dma_prep_memcpy(chan, uart_test_dst, uart_test_src,
                                     src_len, DMA_INTERRUPT);
            tr_flag = TEST_MEM2MEM_MEMCPY;
        } else if (!strcmp(argv[1], "memset")) {
            tr_ret = dma_prep_memset(chan, 0x5A, uart_test_dst, dst_len,
                                     DMA_INTERRUPT);
            tr_flag = TEST_MEM2MEM_MEMSET;
        }

        if (0 == tr_ret) {
            printf("set transfer config err.\r\n");
            return -1;
        }

    } else if (!strcmp(argv[0], "mem2dev")) {
        /* 3.set transfer config (only for mem2dev/dev2mem) */
        dma_cfg.direction = DMA_MEM2DEV;
        dma_cfg.src_addr = (addr_t)uart_test_src;
        dma_cfg.dst_addr =
            DEVICE_BASE(UART_TX) + UART_TX_FIFO_OFFSET; // uart tx fifo address
        dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.src_maxburst = 16;
        dma_cfg.dst_maxburst = 16;

        dma_config_chan(chan, &dma_cfg);
        if (2 == argc && !strcmp(argv[1], "cyclic")) {
            chan->context = NULL;
            chan->irq_cb = dma_transfer_every_mad_done;
            tr_ret =
                dma_prep_cyclic(chan, uart_test_src, 64, 16, DMA_INTERRUPT);
            tr_flag = TEST_MEM2DEV_CYCLIC;
        } else {
            tr_ret = dma_prep_dev(chan, uart_test_src, 64, DMA_INTERRUPT);
            tr_flag = TEST_MEM2DEV_SINGLE;
        }

        if (0 == tr_ret) {
            printf("set transfer config err.\r\n");
            return -1;
        }

    } else if (!strcmp(argv[0], "dev2mem")) {
        /* 3.set transfer config (only for mem2dev/dev2mem) */
        dma_cfg.direction = DMA_DEV2MEM;
        dma_cfg.src_addr =
            DEVICE_BASE(UART_RX) + UART_RX_FIFO_OFFSET; // uart3-fifo address
        dma_cfg.dst_addr = (addr_t)uart_test_dst;
        /* sdrv uart wml must set to 0x0->1bytes */
        dma_cfg.src_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.dst_addr_width = DMA_DEV_BUSWIDTH_1_BYTE;
        dma_cfg.src_maxburst = 1; // rx water level 0x0
        dma_cfg.dst_maxburst = 1;
        cyclic_every_xfer_done_count = 0;
        printf("cyclic_every_xfer_done_count: %d\n",
               cyclic_every_xfer_done_count);
        dma_config_chan(chan, &dma_cfg);

        if (2 == argc && !strcmp(argv[1], "cyclic")) {
            chan->context = NULL;
            chan->irq_cb = dma_transfer_every_mad_done;
            tr_flag = TEST_DEV2MEM_CYCLIC;
            tr_ret =
                dma_prep_cyclic(chan, uart_test_dst, 64, 16, DMA_INTERRUPT);
        } else {
            tr_flag = TEST_DEV2MEM_SINGLE;
            tr_ret = dma_prep_dev(chan, uart_test_dst, 64, DMA_INTERRUPT);
        }

        if (0 == tr_ret) {
            printf("set transfer config err.\r\n");
            return -1;
        }
    } else {
        if (!strcmp(argv[1], "memcpy")) {
            tr_ret = dma_prep_memcpy(chan, uart_test_dst, uart_test_src,
                                     src_len, DMA_INTERRUPT);
            tr_flag = TEST_MEM2MEM_MEMCPY;
        } else if (!strcmp(argv[1], "memset")) {
            tr_ret = dma_prep_memset(chan, 0x5A, uart_test_dst, dst_len,
                                     DMA_INTERRUPT);
            tr_flag = TEST_MEM2MEM_MEMSET;
        }

        if (0 == tr_ret) {
            printf("set transfer config err.\r\n");
            return -1;
        }
    }

    /* 4.clean cache (or alloc no-cache buffer) */
    dma_flush_cache((addr_t)uart_test_src, src_len);
    dma_invalidate_cache((addr_t)uart_test_dst, dst_len);

    /* 5.start transfer */
    dma_tr_done_flag = 0;
    dma_start(chan);
    uint32_t time_out = 0;

    /* 6.wait transfer over */
    printf("waiting for dma xfer compl...\n");
#if 1
    if (chan->flags == DMA_INTERRUPT) {
        while (dma_tr_done_flag == 0) {
            if (tr_flag != TEST_DEV2MEM_SINGLE &&
                tr_flag != TEST_DEV2MEM_CYCLIC) {
                if (time_out < 400) {
                    DELAY(10);
                    time_out++;
                    printf("tr_flag: %d, time_out: %d, dma_tr_done_flag:%d\n",
                           tr_flag, time_out, dma_tr_done_flag);
                } else {
                    printf("Int time out\n");
                    break;
                }
            }
        }
        dma_stop(chan);
    } else if (chan->flags == DMA_PENDING_TIMEOUT) {
        dma_status_e sta = dma_sync_wait(chan, 0xffff);
        if (DMA_COMPLETED == sta)
            dma_tr_done_flag = 1;
        else
            dma_tr_done_flag = 0;
    }
#endif
    printf("xfer done\n");
    if (dma_tr_done_flag) {
        if (tr_flag == TEST_MEM2MEM_MEMCPY) {
            if (!memcmp(uart_test_src, uart_test_dst, src_len)) {
                printf("dma memcpy success.\r\n");
            } else {
                printf("dma memcpy failed.\r\n");
                for (int i = 0; i < src_len; i++) {
                    if (uart_test_dst[i] != uart_test_src[i]) {
                        printf("src: %#02x , dst: %#02x , i= %d\n",
                               uart_test_src[i], uart_test_dst[i], i);
                        break;
                    }
                }
            }

        } else if (tr_flag == TEST_MEM2MEM_MEMSET) {
            int i = 0;
            for (i = 0; i < dst_len; i++) {
                if (uart_test_dst[i] != 0x5A) {
                    printf("\ndma memset data err.\r\n");
                    printf("err index: %d. val: %#x ", i, uart_test_dst[i]);
                    break;
                }
            }
            if (i == dst_len)
                printf("dma memset success.\r\n");
        } else if (tr_flag == TEST_DEV2MEM_CYCLIC ||
                   tr_flag == TEST_DEV2MEM_SINGLE) {
            printf("dma dev2mem success.\r\n");
            int i = 0;
            for (i = 0; i < 64; i++) {
                printf("%02x ", uart_test_dst[i]);
            }
            printf("\r\n");
        }
    } else {
        printf("dma transfer failed.\r\n");
    }

    if (tr_flag != TEST_MEM2DEV_CYCLIC) {
        printf("Free DMA chan for tr_flag: %d\n", tr_flag);
        dma_free_chan(chan);
    }
    return 0;
}

CLI_CMD("dma_test",
        "dma test"
        "\r\n\tdma_test mem2mem memcpy  (memcpy test)"
        "\r\n\tdma_test mem2mem memset  (memset test)"
        "\r\n\tdma_test mem2dev         (uart write)"
        "\r\n\tdma_test mem2dev cyclic  (uart write loop)"
        "\r\n\tdma_test dev2mem         (uart read)"
        "\r\n\tdma_test dev2mem cyclic  (uart read loop)",
        dma_test_main);
