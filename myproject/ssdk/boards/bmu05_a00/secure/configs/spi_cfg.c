/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_SPI

#include <types.h>
#include <board.h>
#include <driver/spi.h>
#include <driver/kunlun/spi/dw_spi_drv.h>
#include <clock_cfg.h>
#include <driver/scr.h>
#include "scr_hw.h"
#include "scr_cfg.h"
#include "spi_cfg.h"

extern dma_dev_t *dma_dev;
extern gpio_dev_t g_gpio_safety;

/* spi device cfg info */
#ifdef SPI1_CFG_ENABLE
static struct spi_controller spi1_ctrl;
static struct spi_controller_config spi1_cfg = {
    .id = 1,
    .base = DEVICE_BASE(SPI1),
    .irq = DEVICE_INTR(SPI1_O_SSI),
    .mode = (SPI_CPHA | SPI_CPOL | SPI_LOOP),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi1_ctrl = {
    .scr_sig = SCR_SAFETY__spi1_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI2_CFG_ENABLE
static struct spi_controller spi2_ctrl;
static struct spi_controller_config spi2_cfg = {
    .id = 2,
    .base = DEVICE_BASE(SPI2),
    .irq = DEVICE_INTR(SPI2_O_SSI),
    .mode = 1,
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi2_ctrl = {
    .scr_sig = SCR_SAFETY__spi2_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI3_CFG_ENABLE
static struct spi_controller spi3_ctrl;
static struct spi_controller_config spi3_cfg = {
    .id = 3,
    .base = DEVICE_BASE(SPI3),
    .irq = DEVICE_INTR(SPI3_O_SSI),
    .mode = (SPI_MODE_0),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi3_ctrl = {
    .scr_sig = SCR_SAFETY__spi3_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI4_CFG_ENABLE
static struct spi_controller spi4_ctrl;
static struct spi_controller_config spi4_cfg = {
    .id = 4,
    .base = DEVICE_BASE(SPI4),
    .irq = DEVICE_INTR(SPI4_O_SSI),
    .mode = (SPI_MODE_0),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi4_ctrl = {
    .scr_sig = SCR_SAFETY__spi4_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI5_CFG_ENABLE
static struct spi_controller spi5_ctrl;
static struct spi_controller_config spi5_cfg = {
    .id = 5,
    .base = DEVICE_BASE(SPI5),
    .irq = DEVICE_INTR(SPI5_O_SSI),
    .mode = (SPI_CPHA | SPI_CPOL | SPI_LOOP),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi5_ctrl = {
    .scr_sig = SCR_SEC__spi5_i_opmode,
    .scr_dev = &g_scr_sec,
    .clk = &g_ckgen_ip_spi_sec0,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI6_CFG_ENABLE
static struct spi_controller spi6_ctrl;
static struct spi_controller_config spi6_cfg = {
    .id = 6,
    .base = DEVICE_BASE(SPI6),
    .irq = DEVICE_INTR(SPI6_O_SSI),
    .mode = (SPI_MODE_0),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi6_ctrl = {
    .scr_sig = SCR_SEC__spi6_i_opmode,
    .scr_dev = &g_scr_sec,
    .clk = &g_ckgen_ip_spi_sec1,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI7_CFG_ENABLE
static struct spi_controller spi7_ctrl;
static struct spi_controller_config spi7_cfg = {
    .id = 7,
    .base = DEVICE_BASE(SPI7),
    .irq = DEVICE_INTR(SPI7_O_SSI),
    .mode = (SPI_MODE_0),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi7_ctrl = {
    .scr_sig = SCR_SEC__spi7_i_opmode,
    .scr_dev = &g_scr_sec,
    .clk = &g_ckgen_ip_spi_sec0,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI8_CFG_ENABLE
static struct spi_controller spi8_ctrl;
static struct spi_controller_config spi8_cfg = {
    .id = 8,
    .base = DEVICE_BASE(SPI8),
    .irq = DEVICE_INTR(SPI8_O_SSI),
    .mode = (SPI_MODE_0),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi8_ctrl = {
    .scr_sig = SCR_SEC__spi8_i_opmode,
    .scr_dev = &g_scr_sec,
    .clk = &g_ckgen_ip_spi_sec1,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI9_CFG_ENABLE
static struct spi_controller spi9_ctrl;
static struct spi_controller_config spi9_cfg = {
    .id = 9,
    .base = DEVICE_BASE(SPI9),
    .irq = DEVICE_INTR(SPI9_O_SSI),
    .mode = (SPI_CPHA | SPI_CPOL | SPI_LOOP),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi9_ctrl = {
    .scr_sig = SCR_SAFETY__spi9_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI10_CFG_ENABLE
static struct spi_controller spi10_ctrl;
static struct spi_controller_config spi10_cfg = {
    .id = 10,
    .base = DEVICE_BASE(SPI10),
    .irq = DEVICE_INTR(SPI10_O_SSI),
    .mode = (SPI_CPHA | SPI_CPOL | SPI_LOOP),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi10_ctrl = {
    .scr_sig = SCR_SAFETY__spi10_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI11_CFG_ENABLE
static struct spi_controller spi11_ctrl;
static struct spi_controller_config spi11_cfg = {
    .id = 11,
    .base = DEVICE_BASE(SPI11),
    .irq = DEVICE_INTR(SPI11_O_SSI),
    .mode = (SPI_CPHA | SPI_CPOL | SPI_LOOP),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi11_ctrl = {
    .scr_sig = SCR_SAFETY__spi11_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

#ifdef SPI12_CFG_ENABLE
static struct spi_controller spi12_ctrl;
static struct spi_controller_config spi12_cfg = {
    .id = 12,
    .base = DEVICE_BASE(SPI12),
    .irq = DEVICE_INTR(SPI12_O_SSI),
    .mode = (SPI_CPHA | SPI_CPOL | SPI_LOOP),
    .opmode = SPI_INTP,
    .num_cs = 8,
    .slave = false,
    .slave_bits_per_word = 8,
    .slave_buf_len = 32,
};
static struct dw_spi_controller dw_spi12_ctrl = {
    .scr_sig = SCR_SAFETY__spi12_i_opmode,
    .scr_dev = &g_scr_safety,
    .clk = &g_ckgen_ip_spi_saf,
    .dma_dev = &dma_dev,
    .gpio_dev = &g_gpio_safety,
};
#endif

/* spi initialization */
void board_spi_init(void)
{
#ifdef SPI1_CFG_ENABLE
    dw_spi_controller_probe(&spi1_ctrl, &dw_spi1_ctrl, &spi1_cfg);
#endif

#ifdef SPI2_CFG_ENABLE
    dw_spi_controller_probe(&spi2_ctrl, &dw_spi2_ctrl, &spi2_cfg);
#endif

#ifdef SPI3_CFG_ENABLE
    dw_spi_controller_probe(&spi3_ctrl, &dw_spi3_ctrl, &spi3_cfg);
#endif

#ifdef SPI4_CFG_ENABLE
    dw_spi_controller_probe(&spi4_ctrl, &dw_spi4_ctrl, &spi4_cfg);
#endif

#ifdef SPI5_CFG_ENABLE
    dw_spi_controller_probe(&spi5_ctrl, &dw_spi5_ctrl, &spi5_cfg);
#endif

#ifdef SPI6_CFG_ENABLE
    dw_spi_controller_probe(&spi6_ctrl, &dw_spi6_ctrl, &spi6_cfg);
#endif

#ifdef SPI7_CFG_ENABLE
    dw_spi_controller_probe(&spi7_ctrl, &dw_spi7_ctrl, &spi7_cfg);
#endif

#ifdef SPI8_CFG_ENABLE
    dw_spi_controller_probe(&spi8_ctrl, &dw_spi8_ctrl, &spi8_cfg);
#endif

#ifdef SPI9_CFG_ENABLE
    dw_spi_controller_probe(&spi9_ctrl, &dw_spi9_ctrl, &spi9_cfg);
#endif

#ifdef SPI10_CFG_ENABLE
    dw_spi_controller_probe(&spi10_ctrl, &dw_spi10_ctrl, &spi10_cfg);
#endif

#ifdef SPI11_CFG_ENABLE
    dw_spi_controller_probe(&spi11_ctrl, &dw_spi11_ctrl, &spi11_cfg);
#endif

#ifdef SPI12_CFG_ENABLE
    dw_spi_controller_probe(&spi12_ctrl, &dw_spi12_ctrl, &spi12_cfg);
#endif
}
#endif

