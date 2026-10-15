#include <arch.h>
#include <board.h>
#include <clock_cfg.h>
#include <cmsis_os2.h>
#include <reg.h>
#include <stdio.h>
#include <string.h>

#include "CLI_type.h"
#include "driver/taishan/spi/sdrv_spi.h"
#include "driver/spi.h"
#include "irq_num.h"
#include "regs_base.h"
#include "spi_lld.h"
#include "clock_cfg.h"
#include "taishan_ip_spi.h"


static uint32_t s_tx_buf_u32[32] __attribute__((__aligned__(32)));
static uint32_t s_rx_buf_u32[32] __attribute__((__aligned__(32)));
static uint32_t m_rx_buf_u32[32] __attribute__((__aligned__(32)));
static uint32_t m_tx_buf_u32[32] __attribute__((__aligned__(32)));

static uint16_t s_tx_buf_u16[32] __attribute__((__aligned__(32)));
static uint16_t s_rx_buf_u16[32] __attribute__((__aligned__(32)));
static uint16_t m_rx_buf_u16[32] __attribute__((__aligned__(32)));
static uint16_t m_tx_buf_u16[32] __attribute__((__aligned__(32)));

static uint8_t s_tx_buf_u8[32] __attribute__((__aligned__(32)));
static uint8_t s_rx_buf_u8[32] __attribute__((__aligned__(32)));
static uint8_t m_rx_buf_u8[32] __attribute__((__aligned__(32)));
static uint8_t m_tx_buf_u8[32] __attribute__((__aligned__(32)));


extern dma_dev_t dma_sf0, dma_sf1;

struct spi_controller controller;
struct spi_controller controller_s;
struct spi_device device;

struct spi_config config = {
    .idle = 0x4,
    .timeout = 0x4,
    .tim_ctrl = FV_SPI_TIM_CTRL_START_DLY(0xf) | FV_SPI_TIM_CTRL_END_DLY(0xf) |
                FV_SPI_TIM_CTRL_FRM_DLY(0),
    .bus_flags = SPI_TX_IRQ_ENABLE | SPI_RX_IRQ_ENABLE,
    .slave_nss_pol = 0,
    .dev_flags = 0,
    .dmac = &dma_sf0,
};

struct spi_config config_async_dma = {
    .idle = 0x4,
    .timeout = 0x4,
    .tim_ctrl = FV_SPI_TIM_CTRL_START_DLY(0xf) | FV_SPI_TIM_CTRL_END_DLY(0xf) |
                FV_SPI_TIM_CTRL_FRM_DLY(0),
    .bus_flags = SPI_DMA_RX_ENABLE | SPI_DMA_TX_ENABLE,
    .dev_flags = 0,
    .slave_nss_pol = 0,
    .dmac = &dma_sf0,
};

struct spi_controller_config controller_conf = {
    .id = 0,
    .base = APB_SPI3_BASE,
    .irq = SPI3_SPI_INTR_NUM,
    .mode = (SPI_CPHA | SPI_CPOL),
    .slave = false,
    .clk = CLK_NODE(g_ckgen_ip_spi_sf_1_to_4),
    .polling_mode = false,
    .bits_per_word_in_slave = 8,
};
struct spi_controller_config controller_conf_u16 = {
    .id = 0,
    .base = APB_SPI3_BASE,
    .irq = SPI3_SPI_INTR_NUM,
    .mode = (SPI_CPHA | SPI_CPOL),
    .slave = false,
    .clk = CLK_NODE(g_ckgen_ip_spi_sf_1_to_4),
    .polling_mode = false,
    .bits_per_word_in_slave = 16,
};
struct spi_controller_config controller_conf_u32 = {
    .id = 0,
    .base = APB_SPI3_BASE,
    .irq = SPI3_SPI_INTR_NUM,
    .mode = (SPI_CPHA | SPI_CPOL),
    .slave = false,
    .clk = CLK_NODE(g_ckgen_ip_spi_sf_1_to_4),
    .polling_mode = false,
    .bits_per_word_in_slave = 32,
};

struct spi_controller_config controller_conf_slave = {
    .id = 0,
    .base = APB_SPI4_BASE,
    .irq = SPI4_SPI_INTR_NUM,
    .mode = (SPI_CPHA | SPI_CPOL),
    .slave = true,
    .clk = CLK_NODE(g_ckgen_ip_spi_sf_1_to_4),
    .polling_mode = false,
    .bits_per_word_in_slave = 8,
};

struct spi_controller_config controller_conf_slave_u16 = {
    .id = 0,
    .base = APB_SPI4_BASE,
    .irq = SPI4_SPI_INTR_NUM,
    .mode = (SPI_CPHA | SPI_CPOL),
    .slave = true,
    .clk = CLK_NODE(g_ckgen_ip_spi_sf_1_to_4),
    .polling_mode = false,
    .bits_per_word_in_slave = 16,
};

struct spi_controller_config controller_conf_slave_u32 = {
    .id = 0,
    .base = APB_SPI4_BASE,
    .irq = SPI4_SPI_INTR_NUM,
    .mode = (SPI_CPHA | SPI_CPOL),
    .slave = true,
    .clk = CLK_NODE(g_ckgen_ip_spi_sf_1_to_4),
    .polling_mode = false,
    .bits_per_word_in_slave = 32,
};

struct spi_device_config device_conf = {
    .mode = (SPI_CPHA | SPI_CPOL),
    .freq_hz = 1000000,
    .bits_per_word = 8,
    .chip_select = 1,
    .is_lsb = 0,
    .byte_swap = 0,
    .start_delay = 0,
    .end_delay = 0,
};
struct spi_device_config device_conf_u16 = {
    .mode = (SPI_CPHA | SPI_CPOL),
    .freq_hz = 1000000,
    .bits_per_word = 16,
    .chip_select = 1,
    .is_lsb = 0,
    .byte_swap = 0,
    .start_delay = 0,
    .end_delay = 0,
};
struct spi_device_config device_conf_u32 = {
    .mode = (SPI_CPHA | SPI_CPOL),
    .freq_hz = 1000000,
    .bits_per_word = 32,
    .chip_select = 1,
    .is_lsb = 0,
    .byte_swap = 0,
    .start_delay = 0,
    .end_delay = 0,
};


static int spi_sync_test(int argc, char *argv[])
{
    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf)) {
        printf("controller_init error\n");
        return 0;
    }
    if (spi_init(&device, &controller, &device_conf)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        m_tx_buf_u8[i] = i;
    }
    spi_transfer(&controller, &device, m_tx_buf_u8, m_rx_buf_u8, 32, 1000);
    while (lld_spi_async_polling(controller.priv))
        ;
    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", m_rx_buf_u8[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    return 0;
}

static int spi_sync_test_16bits(int argc, char *argv[])
{
    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf_u16)) {
        printf("controller_init error\n");
        return 0;
    }
    if (spi_init(&device, &controller, &device_conf_u16)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        m_tx_buf_u16[i] = i|i<<8|i<<16|i<<24;
    }
    spi_transfer(&controller, &device, m_tx_buf_u16, m_rx_buf_u16,32*2, 1000);
    while (lld_spi_async_polling(controller.priv))
        ;
    for (int i = 0; i < 32; i++) {
        printf("0x%04x ", m_rx_buf_u16[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    return 0;
}

static int spi_sync_test_32bits(int argc, char *argv[])
{
    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf_u32)) {
        printf("controller_init error\n");
        return 0;
    }
    if (spi_init(&device, &controller, &device_conf_u32)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        m_tx_buf_u32[i] = i|i<<8|i<<16|i<<24;
    }
    spi_transfer(&controller, &device, m_tx_buf_u32, m_rx_buf_u32, 32*4, 1000);
    while (lld_spi_async_polling(controller.priv))
        ;
    for (int i = 0; i < 32; i++) {
        printf("0x%08x ", m_rx_buf_u32[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    return 0;
}

struct spi_msg msg_s, msg_m;

static int spi_async_irq_test(int argc, char *argv[])
{
    msg_s.tx_buf = s_tx_buf_u8;
    msg_s.rx_buf = s_rx_buf_u8;
    msg_s.tx_remain = 32;
    msg_s.rx_remain = 32;
    msg_s.len = 32;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;

    msg_m.tx_buf = m_tx_buf_u8;
    msg_m.rx_buf = m_rx_buf_u8;
    msg_m.tx_remain = 32;
    msg_m.rx_remain = 32;
    msg_m.len = 32;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;

    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf)) {
        printf("controller_init error\n");
        return 0;
    }
    if (sdrv_spi_controller_probe(&controller_s, &config,
                                  &controller_conf_slave)) {
        printf("controller_init error slave\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        s_tx_buf_u8[i] = i;
        m_tx_buf_u8[i] = i;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);
    spi_transfer_async(&controller, &device, &msg_m);
    while (lld_spi_async_polling(controller_s.priv))
        ;
    while (lld_spi_async_polling(controller.priv))
        ;
    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", s_rx_buf_u8[i]);
    }
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", m_rx_buf_u8[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    spi_controller_deinit(&controller_s);
    return 0;
}

static int spi_async_dma_test(int argc, char *argv[])
{
    msg_s.tx_buf = s_tx_buf_u8;
    msg_s.rx_buf = s_rx_buf_u8;
    msg_s.tx_remain = 32;
    msg_s.rx_remain = 32;
    msg_s.len = 32;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;

    msg_m.tx_buf = m_tx_buf_u8;
    msg_m.rx_buf = m_rx_buf_u8;
    msg_m.tx_remain = 32;
    msg_m.rx_remain = 32;
    msg_m.len = 32;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;
    if (sdrv_spi_controller_probe(&controller, &config_async_dma,
                                  &controller_conf)) {
        printf("controller_init error\n");
        return 0;
    }
    if (sdrv_spi_controller_probe(&controller_s, &config_async_dma,
                                  &controller_conf_slave)) {
        printf("controller_init error slave\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        s_tx_buf_u8[i] = i;
        m_tx_buf_u8[i] = i;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);
    spi_transfer_async(&controller, &device, &msg_m);

    while (lld_spi_async_polling(controller_s.priv))
        ;
    while (lld_spi_async_polling(controller.priv))
        ;

    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", s_rx_buf_u8[i]);
    }
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", m_rx_buf_u8[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    spi_controller_deinit(&controller_s);
    return 0;
}

static int spi_async_irq_test_16bits(int argc, char *argv[])
{

    msg_s.tx_buf = s_tx_buf_u16;
    msg_s.rx_buf = s_rx_buf_u16;
    msg_s.tx_remain = 32*2;
    msg_s.rx_remain = 32*2;
    msg_s.len = 32*2;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;

    msg_m.tx_buf = m_tx_buf_u16;
    msg_m.rx_buf = m_rx_buf_u16;
    msg_m.tx_remain = 32*2;
    msg_m.rx_remain = 32*2;
    msg_m.len = 32*2;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;

    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf_u16)) {
        printf("controller_init error\n");
        return 0;
    }
    if (sdrv_spi_controller_probe(&controller_s, &config,
                                  &controller_conf_slave_u16)) {
        printf("controller_init error slave\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf_u16)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        s_tx_buf_u16[i] = i|i<<8|i<<16|i<<24;
        m_tx_buf_u16[i] = i|i<<8|i<<16|i<<24;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);
    spi_transfer_async(&controller, &device, &msg_m);
    while (lld_spi_async_polling(controller_s.priv))
        ;
    while (lld_spi_async_polling(controller.priv))
        ;
    for (int i = 0; i < 32; i++) {
        printf("0x%04x ", s_rx_buf_u16[i]);
    }
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("0x%04x ", m_rx_buf_u16[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    spi_controller_deinit(&controller_s);
    return 0;
}
static int spi_async_dma_test_16bits(int argc, char *argv[])
{
    msg_s.tx_buf = s_tx_buf_u16;
    msg_s.rx_buf = s_rx_buf_u16;
    msg_s.tx_remain = 32*2;
    msg_s.rx_remain =  32*2;
    msg_s.len =  32*2;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;

    msg_m.tx_buf = m_tx_buf_u16;
    msg_m.rx_buf = m_rx_buf_u16;
    msg_m.tx_remain =  32*2;
    msg_m.rx_remain =  32*2;
    msg_m.len =  32*2;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;
    if (sdrv_spi_controller_probe(&controller, &config_async_dma,
                                  &controller_conf_u16)) {
        printf("controller_init error\n");
        return 0;
    }
    if (sdrv_spi_controller_probe(&controller_s, &config_async_dma,
                                  &controller_conf_slave_u16)) {
        printf("controller_init error slave\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf_u16)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        s_tx_buf_u16[i] = i|i<<8|i<<16|i<<24;
        m_tx_buf_u16[i] = i|i<<8|i<<16|i<<24;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);
    spi_transfer_async(&controller, &device, &msg_m);

    while (lld_spi_async_polling(controller_s.priv))
        ;
    while (lld_spi_async_polling(controller.priv))
        ;

    for (int i = 0; i < 32; i++) {
        printf("0x%04x ", s_rx_buf_u16[i]);
    }
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("0x%04x ", m_rx_buf_u16[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    spi_controller_deinit(&controller_s);
    return 0;
}
static int spi_async_irq_test_32bits(int argc, char *argv[])
{

    msg_s.tx_buf = s_tx_buf_u32;
    msg_s.rx_buf = s_rx_buf_u32;
    msg_s.tx_remain =  32*4;
    msg_s.rx_remain =  32*4;
    msg_s.len =  32*4;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;

    msg_m.tx_buf = m_tx_buf_u32;
    msg_m.rx_buf = m_rx_buf_u32;
    msg_m.tx_remain =  32*4;
    msg_m.rx_remain =  32*4;
    msg_m.len =  32*4;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;

    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf_u32)) {
        printf("controller_init error\n");
        return 0;
    }
    if (sdrv_spi_controller_probe(&controller_s, &config,
                                  &controller_conf_slave_u32)) {
        printf("controller_init error slave\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf_u32)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        s_tx_buf_u32[i] = i|i<<8|i<<16|i<<24;
        m_tx_buf_u32[i] = i|i<<8|i<<16|i<<24;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);
    spi_transfer_async(&controller, &device, &msg_m);
    while (lld_spi_async_polling(controller_s.priv))
        ;
    while (lld_spi_async_polling(controller.priv))
        ;
    for (int i = 0; i < 32; i++) {
        printf("0x%08x ", s_rx_buf_u32[i]);
    }
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("0x%08x ", m_rx_buf_u32[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    spi_controller_deinit(&controller_s);
    return 0;
}
static int spi_async_dma_test_32bits(int argc, char *argv[])
{
    msg_s.tx_buf = s_tx_buf_u32;
    msg_s.rx_buf = s_rx_buf_u32;
    msg_s.tx_remain =  32*4;
    msg_s.rx_remain =  32*4;
    msg_s.len =  32*4;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;

    msg_m.tx_buf = m_tx_buf_u32;
    msg_m.rx_buf = m_rx_buf_u32;
    msg_m.tx_remain =  32*4;
    msg_m.rx_remain =  32*4;
    msg_m.len =  32*4;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;
    if (sdrv_spi_controller_probe(&controller, &config_async_dma,
                                  &controller_conf_u32)) {
        printf("controller_init error\n");
        return 0;
    }
    if (sdrv_spi_controller_probe(&controller_s, &config_async_dma,
                                  &controller_conf_slave_u32)) {
        printf("controller_init error slave\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf_u32)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        s_tx_buf_u32[i] = i|i<<8|i<<16|i<<24;
        m_tx_buf_u32[i] = i|i<<8|i<<16|i<<24;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);
    spi_transfer_async(&controller, &device, &msg_m);

    while (lld_spi_async_polling(controller_s.priv))
        ;
    while (lld_spi_async_polling(controller.priv))
        ;

    for (int i = 0; i < 32; i++) {
        printf("0x%08x ", s_rx_buf_u32[i]);
    }
    printf("\n");
    for (int i = 0; i < 32; i++) {
        printf("0x%08x ", m_rx_buf_u32[i]);
    }
    printf("\n");
    spi_deinit(&device);
    spi_controller_deinit(&controller);
    spi_controller_deinit(&controller_s);
    return 0;
}
static int spi_async_irq_master_test(int argc, char *argv[])
{
    printf("%s\n", __FUNCTION__);

    msg_m.tx_buf = m_tx_buf_u8;
    msg_m.rx_buf = m_rx_buf_u8;
    msg_m.tx_remain = 32;
    msg_m.rx_remain = 32;
    msg_m.len =32;
    msg_m.event = NULL;
    msg_m.inited = false;
    msg_m.xfer_complete = false;
    msg_m.timeout = 0;

    if (sdrv_spi_controller_probe(&controller, &config, &controller_conf)) {
        printf("controller_init error\n");
        return 0;
    }

    if (spi_init(&device, &controller, &device_conf)) {
        printf("device init error\n");
        return 0;
    }
    for (int i = 0; i < 32; i++) {
        m_tx_buf_u8[i] = i;
    }

    spi_transfer_async(&controller, &device, &msg_m);

    while (lld_spi_async_polling(controller.priv))
        ;

    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", m_rx_buf_u8[i]);
    }
    printf("\n");

    spi_deinit(&device);
    spi_controller_deinit(&controller);

    return 0;
}

static int spi_async_irq_slave_test(int argc, char *argv[])
{
    printf("%s\n", __FUNCTION__);
    msg_s.tx_buf = s_tx_buf_u8;
    msg_s.rx_buf = s_rx_buf_u8;
    msg_s.tx_remain = 32;
    msg_s.rx_remain = 32;
    msg_s.len = 32;
    msg_s.event = NULL;
    msg_s.inited = false;
    msg_s.xfer_complete = false;
    msg_s.timeout = 0;


    if (sdrv_spi_controller_probe(&controller_s, &config,
                                  &controller_conf_slave)) {
        printf("controller_init error slave\n");
        return 0;
    }

    for (int i = 0; i < 32; i++) {
        s_tx_buf_u8[i] = i ;
    }
    spi_transfer_async(&controller_s, NULL, &msg_s);

    while (lld_spi_async_polling(controller_s.priv))
        ;

    for (int i = 0; i < 32; i++) {
        printf("0x%02x ", s_rx_buf_u8[i]);
    }
    printf("\n");

    spi_controller_deinit(&controller_s);
    return 0;
}

/*
 * Register Internel CLI commands.
 */
CLI_CMD("spi_sync_test", "\r\nspi_sync_test\r\n", spi_sync_test);
CLI_CMD("spi_sync_test_16bits", "\r\nspi_sync_test_16bits\r\n", spi_sync_test_16bits);
CLI_CMD("spi_sync_test_32bits", "\r\nspi_sync_test_32bits\r\n", spi_sync_test_32bits);
CLI_CMD("spi_async_irq_test", "\r\nspi_async_irq_test\r\n",
        spi_async_irq_test);
CLI_CMD("spi_async_dma_test", "\r\nspi_async_dma_test\r\n",
        spi_async_dma_test);

CLI_CMD("spi_async_irq_test_16bits", "\r\nspi_async_irq_test_16bits\r\n",
        spi_async_irq_test_16bits);
CLI_CMD("spi_async_dma_test_16bits", "\r\nspi_async_dma_test_16bits\r\n",
        spi_async_dma_test_16bits);

CLI_CMD("spi_async_irq_test_32bits", "\r\nspi_async_irq_test_32bits\r\n",
        spi_async_irq_test_32bits);
CLI_CMD("spi_async_dma_test_32bits", "\r\nspi_async_dma_test_32bits\r\n",
        spi_async_dma_test_32bits);

CLI_CMD("spi_async_irq_master_test", "\r\nspi_async_irq_master_test\r\n",
        spi_async_irq_master_test);
CLI_CMD("spi_async_dma_slave_test", "\r\nspi_async_irq_slave_test\r\n",
        spi_async_irq_slave_test);