/**
 * @file taihang_xspi.c
 * @brief TaiShan xspi host low level driver C code.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <param.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <arch/barriers.h>
#include <arch/cache.h>
#include <driver/irq.h>
#include <param.h>
#include <reg.h>
#include <driver/taihang/spi_nor/taihang_xspi_drv.h>

#include "taihang_xspi.h"


/* default use direct read mode */
#ifndef XSPI_USE_DIRECT_MODE
#define XSPI_USE_DIRECT_MODE (1)
#endif


#define XSPI_POLLING_INTERVAL_FACTOR 80
#define SPI_NOR_XFER_COMPLETE (0x1u)

volatile int g_xspi_debug = 1;

static uint32_t s_lockstep_err = 0;

extern void xspi_rx_config(struct xspi_pdata *xspi, uint32_t mode);

static uint32_t g_enable_event;

static void xspi_cache_flush(struct spi_nor *nor, flash_addr_t addr, flash_size_t length);

static inline uint8_t _fls(unsigned int x)
{
    uint8_t left = 0, right = 31, mid = 0;

    while (left < right) {
        mid = (left + right) >> 1;

        if ((0xffffffff - ((1u << (mid + 1)) - 1)) & x) {
            left = mid + 1;
        }
        else if (x & (1u << mid)) {
            return mid;
        }
        else {
            right = mid - 1;
        }
    }

    return left;
}

static inline uint8_t my_log2_uint(uint64_t val)
{
    uint8_t ret = 0;

    if (val & (val - 1)) {
        ssdk_printf(SSDK_ERR, "val is not 2^n\r\n");
    }

    while (val > 1ull) {
        val >>= 1ull;
        ret++;
    }

    return ret;
}

static int xspi_wait_for_bit_times(addr_t reg, const uint32_t mask,
                                        bool clear,
                                        uint32_t times)
{
    uint32_t val;
    uint32_t count = 0;

    while (count < times) {
        val = readl(reg);

        if (clear) {
            val = ~val;
        }

        val &= mask;

        if (val == mask) {
            return 0;
        }

        count++;
    }

    return -1;
}

static inline uint32_t xspi_get_rd_buf_level(struct xspi_pdata *xspi)
{
    uint32_t reg = xspi_readl(xspi->apb_base + XSPI_INDIRECT_RX_BUF);
    reg >>= XSPI_INDIRECT_RX_BUF_ENTRY_LSB;
    return reg & XSPI_INDIRECT_RX_BUF_ENTRY_MASK;
}

static inline uint32_t xspi_get_wr_buf_level(struct xspi_pdata *xspi)
{
    uint32_t reg = xspi_readl(xspi->apb_base + XSPI_INDIRECT_TX_BUF);
    reg >>= XSPI_INDIRECT_TX_BUF_ENTRY_LSB;
    return reg & XSPI_INDIRECT_TX_BUF_ENTRY_MASK;
}

void xspi_reset_flash(struct xspi_pdata *xspi)
{
    /* rst enable */
    RMWREG32(xspi->apb_base + XSPI_IO_CTRL, 0, 1, 1);

    /* rst start */
    RMWREG32(xspi->apb_base + XSPI_IO_CTRL, 1, 1, 0);
    udelay(1000u);

    /* rst end */
    RMWREG32(xspi->apb_base + XSPI_IO_CTRL, 1, 1, 1);
    udelay(1000u);

    /* rst disable */
    RMWREG32(xspi->apb_base + XSPI_IO_CTRL, 0, 1, 0);
    xspi_readl(xspi->apb_base + XSPI_IO_CTRL);
}

static void xspi_delay_set(struct spi_nor *nor)
{
    uint32_t reg, tslch, tshsl, tchsh;
    struct xspi_pdata *xspi = nor->host->priv_data;

    tslch = XSPI_NS_2_TICKS(xspi->ref_clk_hz, 10);
    tshsl = XSPI_NS_2_TICKS(xspi->ref_clk_hz, 30);
    tchsh = XSPI_NS_2_TICKS(xspi->ref_clk_hz, 10);
    tchsh = tchsh >= 3 ? tchsh : 3;

    reg = xspi_readl(xspi->apb_base + XSPI_CS_CTRL);
    reg &= ~(XSPI_CS_CTRL_TIME_DELAY_MASK << XSPI_CS_CTRL_TSLCH_LSB);
    reg |= (tslch & XSPI_CS_CTRL_TIME_DELAY_MASK) << XSPI_CS_CTRL_TSLCH_LSB;

    reg &= ~(XSPI_CS_CTRL_TIME_SHSL_DELAY_MASK << XSPI_CS_CTRL_TSHSL_LSB);
    reg |= (tshsl & XSPI_CS_CTRL_TIME_SHSL_DELAY_MASK) << XSPI_CS_CTRL_TSHSL_LSB;

    reg &= ~(XSPI_CS_CTRL_TIME_DELAY_MASK << XSPI_CS_CTRL_TCHSH_LSB);
    reg |= (tchsh & XSPI_CS_CTRL_TIME_DELAY_MASK) << XSPI_CS_CTRL_TCHSH_LSB;
    xspi_writel(reg, xspi->apb_base + XSPI_CS_CTRL);
}

__UNUSED static void xspi_timeout_set(struct xspi_pdata *xspi)
{
    uint32_t timeout = XSPI_MS_2_TICKS(xspi->ref_clk_hz, 10);
    RMWREG32(xspi->apb_base + XSPI_AXI_CTRL, 0, 24, timeout);
}

static uint32_t xspi_idle_status(struct xspi_pdata *xspi)
{
    uint32_t reg;
    reg = readl(xspi->apb_base + XSPI_MODE_CTRL);
    return reg & XSPI_MODE_CTRL_IDLE;
}

static int xspi_wait_idle(struct xspi_pdata *xspi)
{
    uint32_t idle_latency_cycles = 4;
    uint32_t idle_count = 0;
    uint32_t polling_cycles = 0;

    while (1) {
        if (xspi_idle_status(xspi) != 0u) {
            idle_count++;
        }
        else {
            idle_count = 0;
        }

        /* When IDLE bit asserted, need wait 4 cycyles of ref_clk. */
        if (idle_count >= idle_latency_cycles) {
            return 0;
        }
        else if (polling_cycles > XSPI_IDLE_TIMEOUT_CYCLES) {
            return -1;
        }

        polling_cycles++;
    }
}

void xspi_xip_enable(struct xspi_pdata *xspi)
{
    uint32_t reg;
    uint32_t read_dummy;

    /* Enable xip mode */
    reg = xspi_readl(xspi->apb_base + XSPI_MODE_CTRL);
    reg |= XSPI_MODE_CTRL_XIP;
    xspi_writel(reg, xspi->apb_base + XSPI_MODE_CTRL);

    xspi_writel(0u, xspi->apb_base + XSPI_DIRECT_M_CODE);

    reg = xspi_readl(xspi->apb_base + XSPI_DIRECT_RD_PH_CTRL);
    /* Set read mode phase 1 byte */
    reg &= ~(XSPI_DIRECT_MODE_SIZE_MASK << XSPI_DIRECT_MODE_SIZE_LSB);
    /* Xip mode bit sent by 8 lines */
    reg |= 0x3u << XSPI_DIRECT_MODE_LINE_LSB;
    /* Enable read mode phase */
    reg |= XSPI_DIRECT_MODE_EN;
    xspi_writel(reg, xspi->apb_base + XSPI_DIRECT_RD_PH_CTRL);

    /* Read dummy cycle - 1 */
    reg = xspi_readl(xspi->apb_base + XSPI_DIRECT_D_CYC);
    read_dummy = reg & XSPI_DIRECT_D_CYC_RD_MASK;
    reg &= ~(XSPI_DIRECT_D_CYC_RD_MASK);
    reg |= read_dummy - 1;
    xspi_writel(reg, xspi->apb_base + XSPI_DIRECT_D_CYC);
}

void xspi_xip_enter(struct xspi_pdata *xspi)
{
    uint32_t reg;

    /* Disable direct read cmd phase */
    reg = xspi_readl(xspi->apb_base + XSPI_DIRECT_RD_PH_CTRL);
    reg &= ~XSPI_DIRECT_CMD_EN;
    xspi_writel(reg, xspi->apb_base + XSPI_DIRECT_RD_PH_CTRL);
}

static int xspi_enter_xip_mode(struct spi_nor *nor) {
    struct xspi_pdata *xspi = nor->host->priv_data;

    xspi_xip_enable(xspi);
    xspi_cache_flush(nor, 0, nor->info.size);
    ssdk_printf(SSDK_EMERG, "read to confirm XIP %X\r\n", *(uint32_t *)xspi->direct_base);
    xspi_cache_flush(nor, 0, nor->info.size);
    xspi_xip_enter(xspi);
    return 0;
}

void xspi_dma_enable(struct xspi_pdata *xspi)
{
    uint32_t reg;

    reg = xspi_readl(xspi->apb_base + XSPI_INDIRECT_WR_DMA);
    reg |= XSPI_INDIRECT_DMA_SINGLE_EN | XSPI_INDIRECT_DMA_REQ_EN;
    xspi_writel(reg, xspi->apb_base + XSPI_INDIRECT_WR_DMA);

    reg = xspi_readl(xspi->apb_base + XSPI_INDIRECT_RD_DMA);
    reg |= XSPI_INDIRECT_DMA_SINGLE_EN | XSPI_INDIRECT_DMA_REQ_EN;
    xspi_writel(reg, xspi->apb_base + XSPI_INDIRECT_RD_DMA);
}

void xspi_dma_disable(struct xspi_pdata *xspi)
{
    xspi_writel(0u, xspi->apb_base + XSPI_INDIRECT_WR_DMA);
    xspi_writel(0u, xspi->apb_base + XSPI_INDIRECT_RD_DMA);
}

void xspi_rx_config(struct xspi_pdata *xspi, uint32_t mode)
{
    uint32_t reg;

    reg = xspi_readl(xspi->apb_base + XSPI_SCLK_CTRL);
    reg &= ~(XSPI_SCLK_CTRL_RX_SEL_MASK << XSPI_SCLK_CTRL_RX_SEL_LSB);
    reg |= mode << XSPI_SCLK_CTRL_RX_SEL_LSB;
    xspi_writel(reg, xspi->apb_base + XSPI_SCLK_CTRL);
}

#ifdef XSPI_DLL_USE_OVERRIDE_MODE
void xspi_dll_enable(struct xspi_pdata *xspi, int dll_num, int value,
                          bool enable)
{
    uint32_t reg = 0u;
    uint32_t addr;

    addr = dll_num ? XSPI_DLL1_CTRL : XSPI_DLL_CTRL;

    if (enable) {
        xspi_writel(reg, xspi->apb_base + addr);
        reg |= value << 9;
        xspi_writel(reg, xspi->apb_base + addr);
        reg |= BIT(8);
        xspi_writel(reg, xspi->apb_base + addr);
    }

    RMWREG32(xspi->apb_base + XSPI_MODE_CTRL, 31, 1, 1);
    udelay(1);
    RMWREG32(xspi->apb_base + XSPI_MODE_CTRL, 31, 1, 0);
    xspi_wait_idle(xspi);

    /* set dll_en */
    RMWREG32(xspi->apb_base + XSPI_SCLK_CTRL, 1, 1, enable ? 1 : 0);
    xspi_readl(xspi->apb_base + XSPI_SCLK_CTRL);
}
#else
void xspi_dll_enable(struct xspi_pdata *xspi, int dll_num, int value,
                          bool enable)
{
    uint32_t reg = 0u;
    int ret = 0;
    uint32_t addr;

    addr = dll_num ? XSPI_DLL1_CTRL : XSPI_DLL_CTRL;

    if (enable) {
        xspi_writel(reg, xspi->apb_base + addr);
        reg |= value << 1;
        xspi_writel(reg, xspi->apb_base + addr);
        reg |= BIT(0);
        xspi_writel(reg, xspi->apb_base + addr);

        ret = xspi_wait_for_bit_times(xspi->apb_base + addr + 4, BIT(16), false,
                                           100000);

        if (ret) {
            ssdk_printf(SSDK_ERR, "dll enable err\r\n");
        }

#if CONFIG_E3L
        if (IS_P0)
#endif
        {
            /* workround: XSPI DLL slave chain update should be gated during flash read operation.
                otherwise, there will be DQS (read strobe) glitch and read data will be wrong. */
            RMWREG32(xspi->apb_base + addr, 31, 1, 1);
        }
    }

    RMWREG32(xspi->apb_base + XSPI_MODE_CTRL, 31, 1, 1);
    udelay(1);
    RMWREG32(xspi->apb_base + XSPI_MODE_CTRL, 31, 1, 0);
    xspi_wait_idle(xspi);

    /* set dll_en */
    RMWREG32(xspi->apb_base + XSPI_SCLK_CTRL, 1, 1, enable ? 1 : 0);
    reg = xspi_readl(xspi->apb_base + addr);
}
#endif

static void xspi_prefetch_config(struct xspi_pdata *xspi, uint32_t channel,
                                      uint16_t gid_mask, uint16_t gid_match,
                                      uint8_t block_num, uint8_t group_num)
{
    uint32_t reg;

    /* Entry size is 8 blocks */
    reg = block_num;
    reg |= group_num << 8;
    /* Disable cache for hyperram */
    reg |= 1u << 16;
    xspi_writel(reg, xspi->apb_base + XSPI_RBUF_CTRL + 0x10 * channel);
    xspi_writel(gid_mask, xspi->apb_base + XSPI_GID_MASK + 0x10 * channel);
    xspi_writel(gid_match, xspi->apb_base + XSPI_GID_MATCH + 0x10 * channel);
}

void xspi_prefetch_flush(struct xspi_pdata *xspi, uint32_t channel)
{
    RMWREG32(xspi->apb_base + XSPI_RBUF_CTRL + 0x10 * channel , 17, 1, 1);
    RMWREG32(xspi->apb_base + XSPI_RBUF_CTRL + 0x10 * channel , 17, 1, 0);
}

static void xspi_prefetch_clear(struct xspi_pdata *xspi, uint32_t channel)
{
    xspi_writel(0u, xspi->apb_base + XSPI_RBUF_CTRL + 0x10 * channel);
    xspi_writel(0u, xspi->apb_base + XSPI_GID_MASK + 0x10 * channel);
    xspi_writel(0u, xspi->apb_base + XSPI_GID_MATCH + 0x10 * channel);
}

static void xspi_prefetch_disable(struct xspi_pdata *xspi)
{
    xspi_prefetch_clear(xspi, 0u);
    xspi_prefetch_clear(xspi, 1u);
    xspi_prefetch_clear(xspi, 2u);
    xspi_prefetch_clear(xspi, 3u);
    xspi_prefetch_clear(xspi, 4u);
    xspi_prefetch_clear(xspi, 5u);
    xspi_prefetch_clear(xspi, 6u);
    xspi_prefetch_clear(xspi, 7u);
    xspi_writel(0, xspi->apb_base + XSPI_GROUP_MERGE);
}

static void xspi_prefetch_enable(struct xspi_pdata *xspi)
{
    RMWREG32(xspi->apb_base + XSPI_AXI_CTRL, 25, 1, 1);

    xspi_prefetch_config(xspi, 0u, 0x0000u, 0x000u, 2u, 4u);
    xspi_prefetch_config(xspi, 1u, 0xff00u, 0x000u, 2u, 4u);
    xspi_prefetch_config(xspi, 2u, 0xff00u, 0x100u, 2u, 4u);
    xspi_prefetch_config(xspi, 3u, 0xff00u, 0x200u, 2u, 4u);
    xspi_prefetch_config(xspi, 4u, 0xff00u, 0x300u, 2u, 4u);
    xspi_prefetch_config(xspi, 5u, 0xff00u, 0x400u, 2u, 4u);
    xspi_prefetch_config(xspi, 6u, 0xff00u, 0x500u, 8u, 1u);
    xspi_prefetch_config(xspi, 7u, 0xff00u, 0x800u, 8u, 1u);
    /* merge group 0-1 for sf core */
    xspi_writel(1u, xspi->apb_base + XSPI_GROUP_MERGE);
}

void *xspi_init(struct xspi_pdata *xspi)
{
    if (xspi_wait_idle(xspi)) {
        return NULL;
    }

    /* Use rx mode by pop data by deafult */
    xspi_writel(XSPI_MODE_CTRL_RX_MODE, xspi->apb_base + XSPI_MODE_CTRL);

    /* Enable|Disable prefetch */
    if (xspi->no_cache) {
        xspi_prefetch_disable(xspi);
    } else {
        xspi_prefetch_enable(xspi);
    }

    /* Set AXI access timeout threshold */
    xspi_timeout_set(xspi);

#ifdef XSPI_INDIRECT_AXI_EN
    /* indirect wr/rd enable */
    RMWREG32(xspi->apb_base + XSPI_MISC, 10, 2, 0x3);
    xspi_readl(xspi->apb_base + XSPI_MISC);
#endif

    /* Set capture enable select to 7 clk period */
    RMWREG32(xspi->apb_base + XSPI_MISC, 12, 3, 0x5);

    xspi_reset_flash(xspi);

    /* rx_mode use internal sclk by default */
    xspi_rx_config(xspi, 0x0);

    xspi_dll_enable(xspi, 0, 7u, true);
    xspi_dll_enable(xspi, 1, 4u, true);

    return (void *)xspi;
}

static inline void fifo_readsl(uint32_t *addr, uint32_t *data,
                                    uint32_t len)
{
    while (len-- != 0u) {
        *data++ = readl(addr);
    }
}

static inline void fifo_writesl(uint32_t *addr, uint32_t const *data,
                                     uint32_t len)
{
    while (len-- != 0u) {
        writel(*data, (uint32_t)addr);
        data++;
    }
}

static void xspi_size_set(struct spi_nor *nor)
{
    uint32_t size = 0;
    struct xspi_pdata *xspi = nor->host->priv_data;

    uint32_t reg = xspi_readl(xspi->apb_base + XSPI_DEV_SIZE);

    size = my_log2_uint(nor->info.size) & 0x1Fu;

    if ((size != 0) && (size != (reg & 0xff))) {
        RMWREG32(xspi->apb_base + XSPI_DEV_SIZE, nor->cs * 8, 5, size);
        reg = xspi_readl(xspi->apb_base + XSPI_DEV_SIZE);
        ssdk_printf(SSDK_CRIT, "xspi dev size reg = 0x%x\r\n", reg);
    }
}

void xspi_proto_setup(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                           enum xspi_access_mode access_mode,
                           enum spi_nor_ops ops)
{
    struct xspi_pdata *xspi = nor->host->priv_data;
    uint32_t ph_ctrl = 0;
    uint16_t cmd_code = 0;
    uint8_t dummy = 0;

    if (!nor->hyperbus_mode) {
        ph_ctrl |= XSPI_C_EN | XSPI_C_SIZE(0u);
    }

    if (nor->octal_dtr_en || nor->hyperbus_mode) {
        ph_ctrl |= XSPI_C_RATE(1u) | XSPI_A_RATE(1u) | XSPI_D_RATE(1u) |
                   XSPI_C_SIZE(1u) | XSPI_A_SIZE(3u);
    }

    if (nor->xfer_info_bottom.size > 0) {
        ph_ctrl |= XSPI_D_EN;
    }

    if (cmd) {
        dummy = cmd->dummy;
        ph_ctrl |= XSPI_C_LINE(cmd->inst_type) | XSPI_A_LINE(cmd->inst_type) |
                   XSPI_D_LINE(cmd->inst_type);
        cmd_code = cmd->opcode;

        if (cmd->addr_bytes) {
            ph_ctrl |= XSPI_A_EN | XSPI_A_SIZE(cmd->addr_bytes - 1);
        }
    }
    else {
        if (nor->addr_width) {
            ph_ctrl |= XSPI_A_SIZE(nor->addr_width - 1);
        }

        if (SPI_NOR_OPS_WRITE == ops) {
            dummy = 0;
            ph_ctrl |= XSPI_C_LINE(SNOR_INST_LANS(nor->info.write_proto)) |
                       XSPI_A_LINE(SNOR_ADDR_LANS(nor->info.write_proto)) |
                       XSPI_D_LINE(SNOR_DATA_LANS(nor->info.write_proto));
            cmd_code = (nor->info.write_proto >> SNOR_OPCODE_PROTO_LSB) & 0xff;
        }
        else if (SPI_NOR_OPS_READ == ops) {
            dummy = nor->info.read_dummy;
            ph_ctrl |= XSPI_C_LINE(SNOR_INST_LANS(nor->info.read_proto)) |
                       XSPI_A_LINE(SNOR_ADDR_LANS(nor->info.read_proto)) |
                       XSPI_D_LINE(SNOR_DATA_LANS(nor->info.read_proto));
            cmd_code = (nor->info.read_proto >> SNOR_OPCODE_PROTO_LSB) & 0xff;
            //  Add for 1s-Xd-Xd, X=2,4,8
            if (!nor->octal_dtr_en && (nor->info.read_proto & SNOR_DTR_PROTO)) {
                ph_ctrl |= XSPI_A_RATE(1u) | XSPI_D_RATE(1u);
            }
        }
    }

    /* For hyperbus mode, taihang xspi's dummy is device spec's dummy - 1 */
    if (nor->hyperbus_mode && dummy > 0u) {
        dummy = dummy - 1u;
    }

    if (nor->octal_dtr_en || nor->hyperbus_mode) {
        cmd_code |= cmd_code << 8u;
    }

    if (0u == dummy) {
        RMWREG32(xspi->apb_base + XSPI_MISC, 8u, 1u, 1);
    }
    else {
        RMWREG32(xspi->apb_base + XSPI_MISC, 8u, 1u, 0);
    }

    if (XSPI_DIRECT_ACCESS_MODE == access_mode) {
        if (SPI_NOR_OPS_WRITE == ops) {
            xspi_writel(ph_ctrl, xspi->apb_base + XSPI_DIRECT_WR_PH_CTRL);
            RMWREG32(xspi->apb_base + XSPI_DIRECT_C_CODE, 16, 16, cmd_code);
            xspi_readl(xspi->apb_base + XSPI_DIRECT_C_CODE);
            RMWREG32(xspi->apb_base + XSPI_DIRECT_D_CYC, 8, 5, dummy);
            xspi_readl(xspi->apb_base + XSPI_DIRECT_D_CYC);

            /* Hyperbus mode not need write enable */
            if (!nor->hyperbus_mode) {
                xspi_writel(0x06u << 16 | 1, xspi->apb_base + XSPI_DIRECT_WREN);
            }
        }
        else if (SPI_NOR_OPS_READ == ops) {
            xspi_writel(ph_ctrl, xspi->apb_base + XSPI_DIRECT_RD_PH_CTRL);
            RMWREG32(xspi->apb_base + XSPI_DIRECT_C_CODE, 0, 16, cmd_code);
            xspi_readl(xspi->apb_base + XSPI_DIRECT_C_CODE);
            RMWREG32(xspi->apb_base + XSPI_DIRECT_D_CYC, 0, 5, dummy);
            xspi_readl(xspi->apb_base + XSPI_DIRECT_D_CYC);
        }
        else {
            ssdk_printf(SSDK_ERR, "%s: set direct mode err. %d\r\n", __FUNCTION__,
                        ops);
        }
    }
    else {
        if (!cmd) {
            ph_ctrl |= (nor->addr_width > 0 ? XSPI_A_EN : 0);
        }

        if (SPI_NOR_OPS_WRITE == ops) {
            xspi_writel(ph_ctrl, xspi->apb_base + XSPI_INDIRECT_WR_PH_CTRL);
            xspi_writel(cmd_code, xspi->apb_base + XSPI_INDIRECT_WR_CMD_CODE);
            xspi_writel(dummy, xspi->apb_base + XSPI_INDIRECT_WR_CYC);
        }
        else if (SPI_NOR_OPS_READ == ops) {
            xspi_writel(ph_ctrl, xspi->apb_base + XSPI_INDIRECT_RD_PH_CTRL);
            xspi_writel(cmd_code, xspi->apb_base + XSPI_INDIRECT_RD_CMD_CODE);
            xspi_writel(dummy, xspi->apb_base + XSPI_INDIRECT_RD_CYC);
        }
        else {
            ssdk_printf(SSDK_ERR, "%s: set indirect mode err. %d\r\n",
                        __FUNCTION__, ops);
        }
    }
}

__UNUSED static void xspi_cs_set(struct spi_nor *nor)
{
    uint32_t reg;
    uint8_t cs;
    struct xspi_pdata *xspi = nor->host->priv_data;

    if (nor->cs == 0u) {
        cs = XSPI_CS_CTRL_HW_SEL_DEV1;
    }
    else {
        cs = XSPI_CS_CTRL_HW_SEL_DEV2;
    }

    reg = xspi_readl(xspi->apb_base + XSPI_CS_CTRL);
    reg &= ~(XSPI_CS_CTRL_HW_SEL_MASK << XSPI_CS_CTRL_HW_SEL_LSB);

    reg |= cs << XSPI_CS_CTRL_HW_SEL_LSB;

    xspi_writel(reg, xspi->apb_base + XSPI_CS_CTRL);
}

static void xspi_polling_set(struct spi_nor *nor)
{
    struct xspi_pdata *xspi = nor->host->priv_data;

    xspi_writel(1u, xspi->apb_base + XSPI_INDIRECT_P_VALUE);
    xspi_writel(0xfffffffeu, xspi->apb_base + XSPI_INDIRECT_P_MASK);
    xspi_writel(XSPI_US_2_TICKS((uint32_t)xspi->ref_clk_hz / 80, 1),
                     xspi->apb_base + XSPI_INDIRECT_P_TIME);
}

void xspi_dqs_enable(struct xspi_pdata *xspi, bool enable)
{
    uint32_t reg;

    /* For aviod dqs bug */
    RMWREG32(xspi->apb_base + XSPI_MISC, 9u, 1u, 1);

    reg = xspi_readl(xspi->apb_base + XSPI_SCLK_CTRL);
    reg &= ~(XSPI_SCLK_CTRL_RX_SEL_MASK << XSPI_SCLK_CTRL_RX_SEL_LSB);

    if (enable) {
        /* dqs mode */
        reg |= 2u << XSPI_SCLK_CTRL_RX_SEL_LSB;
    }
    else {
        reg |= 0u << XSPI_SCLK_CTRL_RX_SEL_LSB;
    }

    xspi_writel(reg, xspi->apb_base + XSPI_SCLK_CTRL);
}

static inline void xspi_hyperbus_en(struct spi_nor *nor)
{
    struct xspi_pdata *xspi = nor->host->priv_data;

    /* Set rx enable mode to half-dummy or rdata phase 0 */
    RMWREG32(xspi->apb_base + XSPI_MISC, 9u, 1u, 1);
    xspi_readl(xspi->apb_base + XSPI_MISC);

    /* Set bytes swap */
    RMWREG32(xspi->apb_base + XSPI_MODE_CTRL, 12u, 1u, 1);

    /* Set profile2.0 enable */
    RMWREG32(xspi->apb_base + XSPI_HYPERBUS_CTRL, 3u, 1u, 1);

    /* Set address type divided to 2 */
    RMWREG32(xspi->apb_base + XSPI_HYPERBUS_CTRL, 2u, 1u, 0);

    /*Set address space type memory */
    RMWREG32(xspi->apb_base + XSPI_HYPERBUS_CTRL, 1u, 1u, 0);

    RMWREG32(xspi->apb_base + XSPI_HYPERBUS_CTRL, 0u, 1u, nor->hyperbus_mode);
}

static void xspi_hyperbus_direct_setup(struct spi_nor *nor)
{
    struct xspi_pdata *xspi = nor->host->priv_data;

    /* Config direct read pre-transaction to 0 */
    xspi_writel(0, xspi->apb_base + XSPI_DIRECT_RD_PTC);

    /* Config direct write pre-transaction to 0 */
    xspi_writel(0, xspi->apb_base + XSPI_DIRECT_WR_PTC);

    xspi_proto_setup(nor, NULL, XSPI_DIRECT_ACCESS_MODE, SPI_NOR_OPS_READ);
    xspi_proto_setup(nor, NULL, XSPI_DIRECT_ACCESS_MODE, SPI_NOR_OPS_WRITE);
}

void xspi_hyperram_enable(struct spi_nor *nor, uint8_t cs,
                               uint8_t read_dummy, uint8_t write_dummy)
{
    uint32_t dummy;
    struct xspi_pdata *xspi = nor->host->priv_data;

    RMWREG32(xspi->apb_base + XSPI_DEV_SIZE, 16u + cs, 1u, 0x1u);

    /* Config direct read pre-transaction to 0 */
    xspi_writel(0, xspi->apb_base + XSPI_DIRECT_RD_PTC);

    /* Config direct write pre-transaction to 0 */
    xspi_writel(0, xspi->apb_base + XSPI_DIRECT_WR_PTC);

    dummy = ((read_dummy + 1) & XSPI_DIRECT_D_CYC_RD_MASK) <<
            XSPI_DIRECT_D_CYC_RD_LSB;
    dummy |= ((write_dummy + 1) & XSPI_DIRECT_D_CYC_WR_MASK) <<
             XSPI_DIRECT_D_CYC_WR_LSB;
    xspi_writel(dummy, xspi->apb_base + XSPI_DIRECT_D_CYC);

    xspi_prefetch_disable(xspi);
}

void xspi_nor_setup(struct spi_nor *nor, enum spi_nor_ops ops)
{
    struct xspi_pdata *xspi = nor->host->priv_data;

    if (xspi_wait_idle(xspi)) {
        ssdk_printf(SSDK_ERR, "%s: wait idle err.", __FUNCTION__);
        return;
    }

    if (nor->cs != xspi->current_cs && nor->info.name != 0) {
        /* cs select by hardware, based on access address */
        // xspi_cs_set(nor);
        xspi_delay_set(nor);

        if (nor->hyperbus_mode) {
            xspi_hyperbus_en(nor);
            xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                                  SPI_NOR_OPS_READ);
            xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                                  SPI_NOR_OPS_WRITE);
            xspi_hyperbus_direct_setup(nor);
        }
        else {
            xspi_polling_set(nor);
        }

        xspi->current_cs = nor->cs;
    }

    if (nor->info.name != 0) {
        xspi_size_set(nor);
    }

    if (nor->dqs_en != xspi->dqs_en) {
        xspi_dll_enable(xspi, 0, 7u, nor->dqs_en);
        xspi_dll_enable(xspi, 1, 7u, nor->dqs_en);
        xspi_dqs_enable(xspi, nor->dqs_en);
        xspi->dqs_en = nor->dqs_en;
    }

    if (!nor->hyperbus_mode) {
        xspi_proto_setup(nor, NULL, XSPI_DIRECT_ACCESS_MODE, SPI_NOR_OPS_READ);
        xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,SPI_NOR_OPS_WRITE);
    }

    return;
}

void xspi_setup_xfer(struct spi_nor *nor, enum flash_opt opt,
                          flash_addr_t addr, uint8_t *buf, flash_size_t size)

{
    nor->xfer_info_bottom.opt_type = opt;
    nor->xfer_info_bottom.addr = addr;
    nor->xfer_info_bottom.buf = buf;
    nor->xfer_info_bottom.opt_result = FLASH_OPT_PENDING;
    nor->xfer_info_bottom.size = size;
}

uint32_t xspi_read_fifo(struct xspi_pdata *xspi, uint8_t *buf,
                             flash_size_t size)
{
    uint32_t last_word;
    uint32_t aligned_size;
    uint32_t bytes_need_read = 0;

#ifdef XSPI_INDIRECT_AXI_EN
    uint32_t addr = xspi_readl(xspi->apb_base + XSPI_INDIRECT_RD_ADDR);
    addr_t fifo_address = xspi->direct_base + ROUNDUP(addr, 4u);
#else
    addr_t fifo_address = xspi->apb_base + XSPI_INDIRECT_RDATA;
#endif

    bytes_need_read =
        xspi_get_rd_buf_level(xspi) * (uint32_t)xspi->fifo_width;

    bytes_need_read = MIN(bytes_need_read, size);

    if (bytes_need_read) {
        aligned_size = bytes_need_read / 4u * 4u;

        if (aligned_size == 16) {
            ssdk_printf(SSDK_INFO, "DBG: aligned size %d\r\n", aligned_size);
        }

        fifo_readsl((uint32_t *)fifo_address, (void *)buf, aligned_size / 4u);

        if (bytes_need_read % 4) {
            last_word = readl(fifo_address);
            memcpy(buf + aligned_size, &last_word, bytes_need_read % 4);
        }

        return bytes_need_read;
    }

    return 0;
}

uint32_t xspi_write_fifo(struct xspi_pdata *xspi, const uint8_t *buf,
                              flash_size_t size)
{
    uint32_t sram_level;
    uint32_t write_bytes = 0;
    uint32_t sram_size = (uint32_t)xspi->fifo_depth * xspi->fifo_width;

#ifdef XSPI_INDIRECT_AXI_EN
    uint32_t addr = xspi_readl(xspi->apb_base + XSPI_INDIRECT_WR_ADDR);
    addr_t fifo_address = xspi->direct_base + ROUNDUP(addr, 4u);
#else
    addr_t fifo_address = xspi->apb_base + XSPI_INDIRECT_WDATA;
#endif

    write_bytes = MIN(size, sram_size);

    sram_level = xspi_get_wr_buf_level(xspi);

    if (0u == sram_level) {
        fifo_writesl((uint32_t *)fifo_address, (void *)buf,
                          DIV_ROUND_UP(write_bytes, 4u));
        return write_bytes;
    }

    return 0;
}

void xspi_indirect_trigger(const struct xspi_pdata *xspi, uint32_t addr,
                                uint32_t size, int flag)
{
    if (flag == INDIRECT_READ_FLAG) {
        /* Set indirect read start address */
        xspi_writel(addr, xspi->apb_base + XSPI_INDIRECT_RD_ADDR);
        /* Set indirect read bytes number */
        xspi_writel(size, xspi->apb_base + XSPI_INDIRECT_RD_SIZE);
        /* Start the indirect read */
        writel(1u, xspi->apb_base + XSPI_INDIRECT_RD_CTRL);
    }
    else {
        /* Set indirect write start address */
        xspi_writel(addr, xspi->apb_base + XSPI_INDIRECT_WR_ADDR);
        /* Set indirect write bytes number */
        xspi_writel(size, xspi->apb_base + XSPI_INDIRECT_WR_SIZE);
        /* Start the indirect write */
        writel(1u, xspi->apb_base + XSPI_INDIRECT_WR_CTRL);
    }
}

int xspi_tx_complete(struct xspi_pdata *xspi)
{
    int ret = 0;
    uint32_t reg;

    ret =
        xspi_wait_for_bit_times(xspi->apb_base + XSPI_INT_ST_FUC,
                                     XSPI_INT_ST_FUC_INDIRECT_WR_DONE, false, 100u);

    if (ret != 0) {
        ssdk_printf(SSDK_ERR, "Indirect write not complete, ret %d.\r\n", ret);
    }
    else {
        reg = xspi_readl(xspi->apb_base + XSPI_INT_ST_FUC);
        xspi_writel(reg, xspi->apb_base + XSPI_INT_ST_FUC);
    }

    return ret;
}

int xspi_rx_complete(struct xspi_pdata *xspi)
{
    int ret = 0;
    uint32_t reg;

    ret =
        xspi_wait_for_bit_times(xspi->apb_base + XSPI_INT_ST_FUC,
                                     XSPI_INT_ST_FUC_INDIRECT_RD_DONE, false, 10000u);

    if (ret != 0) {
        ssdk_printf(SSDK_ERR, "Indirect read not complete, ret %d.\r\n", ret);
    }
    else {
        reg = xspi_readl(xspi->apb_base + XSPI_INT_ST_FUC);
        xspi_writel(reg, xspi->apb_base + XSPI_INT_ST_FUC);
    }

    return ret;
}

int xspi_rx_polling(struct spi_nor *nor)
{
    int ret = 0;
    uint32_t rx_size;
    struct xspi_pdata *xspi = nor->host->priv_data;

    if ((nor->xfer_info_bottom.buf != NULL) &&
            (nor->xfer_info_bottom.size > 0u)) {
        rx_size = xspi_read_fifo(xspi, nor->xfer_info_bottom.buf,
                                      nor->xfer_info_bottom.size);
        nor->xfer_info_bottom.buf += rx_size;
        nor->xfer_info_bottom.size -= rx_size;

        if (nor->xfer_mode == SPI_NOR_XFER_POLLING_MODE) {
            if (nor->xfer_info_bottom.size == 0u) {
                ret = xspi_rx_complete(xspi);
            }
        }
    }
    else {
        ret = xspi_rx_complete(xspi);
    }

    return ret;
}

int xspi_tx_polling(struct spi_nor *nor)
{
    int ret = 0;
    uint32_t tx_size;
    struct xspi_pdata *xspi = nor->host->priv_data;

    if ((nor->xfer_info_bottom.buf != NULL) &&
            (nor->xfer_info_bottom.size > 0u)) {
        tx_size = xspi_write_fifo(xspi, nor->xfer_info_bottom.buf,
                                       nor->xfer_info_bottom.size);
        nor->xfer_info_bottom.buf += tx_size;
        nor->xfer_info_bottom.size -= tx_size;

        if (nor->xfer_mode == SPI_NOR_XFER_POLLING_MODE) {
            if (nor->xfer_info_bottom.size == 0u) {
                ret = xspi_tx_complete(xspi);
            }
        }
    }
    else {
        ret = xspi_tx_complete(xspi);
    }

    return ret;
}

void xspi_int_unmask(struct xspi_pdata *xspi, addr_t addr, uint32_t mask)
{
    uint32_t reg;
    reg = xspi_readl(xspi->apb_base + addr);
    reg |= mask;
    xspi_writel(reg, xspi->apb_base + addr);
}

int xspi_rfd_en(struct spi_nor *nor, uint8_t mask)
{
    uint32_t reg;
    struct xspi_pdata *xspi = nor->host->priv_data;
    uint8_t region_mask = mask & 0xF;

    nor->xfer_info_bottom.size = 4;  // enable the data phase
    xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                        SPI_NOR_OPS_READ);

    for (uint8_t i = 0; i < 4; i++) {
        if (region_mask & (1u << i)) {
            /* Key derivation trigger */
            RMWREG32(xspi->apb_base + XSPI_IMG_RFD, 8 + i, 1, 0x01);

            /* Polling key ready */
            while (!(readl(xspi->apb_base + XSPI_IMG_RFD) & BIT(16 + i)))
                ;
        }
    }

    /* Enable decryption */
    RMWREG32(xspi->apb_base + XSPI_IMG_RFD, 28, 4, region_mask);
    nor->xfer_info_bottom.size = 0;

    /* Flush prefetch */
    reg = xspi_readl(xspi->apb_base + XSPI_GROUP_MERGE);
    if (reg) {
        for (uint8_t channel = 0; channel < 8; channel++) {
            xspi_prefetch_flush(xspi, channel);
        }
    }

    return 0;
}

static flash_size_t xspi_read_dev1_size(struct xspi_pdata *xspi)
{
    uint32_t reg = xspi_readl(xspi->apb_base + XSPI_DEV_SIZE);
    return 0x1u << (reg & XSPI_DEV_SIZE_SIZE_MASK);
}

static bool xspi_read_dqs_en(struct xspi_pdata *xspi)
{
    uint32_t reg = xspi_readl(xspi->apb_base + XSPI_SCLK_CTRL);
    if ((reg >> XSPI_SCLK_CTRL_RX_SEL_LSB) == 2) {
        return true;
    } else {
        return false;
    }
}

static uint32_t xspi_read_direct_dummy_cycle(struct xspi_pdata *xspi)
{
    uint32_t reg = xspi_readl(xspi->apb_base + XSPI_DIRECT_D_CYC);
    return reg & XSPI_DIRECT_D_CYC_RD_MASK;
}

static uint32_t spi_nor_get_proto(struct spi_nor *nor, enum spi_nor_rw_para ops) {
    struct xspi_pdata *xspi = nor->host->priv_data;
    ph_ctrl_t ph_data = { 0 };
    uint32_t opcode = 0;
    uint32_t res = 0;

    switch (ops) {
    case SPI_NOR_DIRECT_READ:
        ph_data.data = xspi_readl(xspi->apb_base + XSPI_DIRECT_RD_PH_CTRL);
        opcode = (xspi_readl(xspi->apb_base + XSPI_DIRECT_C_CODE)) & XSPI_DIRECT_C_CODE_MASK;
        break;
    case SPI_NOR_INDIRECT_READ:
        ph_data.data = xspi_readl(xspi->apb_base + XSPI_INDIRECT_RD_PH_CTRL);
        opcode = (xspi_readl(xspi->apb_base + XSPI_INDIRECT_RD_CMD_CODE)) & XSPI_INDIRECT_RD_CMD_CODE_MASK;
        break;
    case SPI_NOR_DIRECT_WRITE:
        ph_data.data = xspi_readl(xspi->apb_base + XSPI_DIRECT_WR_PH_CTRL);
        opcode = (xspi_readl(xspi->apb_base + XSPI_DIRECT_C_CODE)) & XSPI_DIRECT_C_CODE_MASK;
        break;
    case SPI_NOR_INDIRECT_WRITE:
        ph_data.data = xspi_readl(xspi->apb_base + XSPI_INDIRECT_WR_PH_CTRL);
        opcode = (xspi_readl(xspi->apb_base + XSPI_INDIRECT_WR_CMD_CODE)) & XSPI_INDIRECT_WR_CMD_CODE_MASK;
        break;
    default:
        ssdk_printf(SSDK_CRIT, "This ops %x not support\r\n", ops);
        break;
    }
    ssdk_printf(SSDK_CRIT, "opcode: 0x%x, %d, %d, %d \r\n", opcode,
        ph_data.ph_ctrl.c_line, ph_data.ph_ctrl.a_line, ph_data.ph_ctrl.d_line);
    res |= opcode << SNOR_OPCODE_PROTO_LSB;
    res |= ph_data.ph_ctrl.c_line << SNOR_INST_LANS_PROTO_LSB;
    res |= ph_data.ph_ctrl.a_line << SNOR_ADDR_LANS_PROTO_LSB;
    res |= ph_data.ph_ctrl.d_line;

    if (ph_data.ph_ctrl.c_rate && ph_data.ph_ctrl.a_rate && ph_data.ph_ctrl.d_rate) {
        res |= SNOR_DTR_PROTO;
    }
    ssdk_printf(SSDK_CRIT, "Proto Res = %x\r\n", res);
    return res;
}

static uint32_t spi_nor_read_dummy_para(struct spi_nor *nor) {
    uint32_t read_dummy;
    struct xspi_pdata *xspi = nor->host->priv_data;

    read_dummy = xspi_read_direct_dummy_cycle(xspi);
    ssdk_printf(SSDK_CRIT, "read_dummy = 0x%x \r\n", read_dummy);

    return read_dummy;
}

static flash_size_t spi_nor_read_dev_size_para(struct spi_nor *nor) {
    flash_size_t flash_size;
    struct xspi_pdata *xspi = nor->host->priv_data;

    flash_size = xspi_read_dev1_size(xspi);
    ssdk_printf(SSDK_CRIT, "flash_size = 0x%llx \r\n", flash_size);

    return flash_size;
}

static void xspi_xip_proto_setup(struct spi_nor *nor) {
    nor->reg_proto = spi_nor_get_proto(nor, SPI_NOR_INDIRECT_READ);
    nor->info.write_proto = spi_nor_get_proto(nor, SPI_NOR_INDIRECT_WRITE);
    nor->info.read_proto = spi_nor_get_proto(nor, SPI_NOR_DIRECT_READ);
    nor->info.read_dummy = spi_nor_read_dummy_para(nor);
    nor->info.size = spi_nor_read_dev_size_para(nor);
    nor->dqs_en = xspi_read_dqs_en(nor->host->priv_data);
    if (SNOR_PROTO_DTR(nor->info.read_proto) == SNOR_PROTO_8_8_8_DTR &&
            SNOR_PROTO_DTR(nor->info.write_proto) == SNOR_PROTO_8_8_8_DTR) {
        nor->octal_dtr_en = true;
    }
    if (nor->info.size > 0x1000000) {
        nor->addr_width = 4;
    }
}

/**
 * @brief xspi polling mode function
 * @param[in] arg spi_nor context
 */
static void xspi_polling_handler(void *arg)
{
    int ret = 0;
    struct spi_nor *nor = arg;
    struct xspi_pdata *xspi = nor->host->priv_data;

    if (nor->xfer_info_bottom.opt_result == FLASH_OPT_PENDING) {
        if (nor->xfer_info_bottom.opt_type == FLASH_OPT_READ) {
            ret = xspi_rx_polling(nor);
        }
        else if (nor->xfer_info_bottom.opt_type == FLASH_OPT_WRITE) {
            ret = xspi_tx_polling(nor);
        }
    }

    nor->xfer_info_bottom.opt_result = ret ? FLASH_OPT_FAILED : FLASH_OPT_PENDING;
    ssdk_printf(SSDK_DEBUG, "xfer_type = %d, xfer size = %lld, xfer result = %d\r\n",
                nor->xfer_info_bottom.opt_type == FLASH_OPT_READ, nor->xfer_info_bottom.size,
                nor->xfer_info_bottom.opt_result);

    if ((nor->xfer_info_bottom.size == 0 || nor->xfer_info_bottom.buf == NULL) &&
            nor->xfer_info_bottom.opt_result == FLASH_OPT_PENDING) {
        nor->xfer_info_bottom.opt_result = FLASH_OPT_COMPLETE;

        if ((nor->xfer_mode == SPI_NOR_XFER_POLLING_MODE)
                || (nor->xfer_info_bottom.opt_type == FLASH_OPT_READ)) {
            xspi_writel(0u, xspi->apb_base + XSPI_INT_EN_FUC);

            if (nor->async_mode) {
                if (nor->event_handler)
                    nor->event_handler(nor->xfer_info_bottom.opt_type,
                                       nor->xfer_info_bottom.opt_result);
            }
            else {
                /* Set xfer complete flag when read opt */
                g_enable_event = 1;
            }
        }
    }
}

/**
 * @brief xspi wait mode function, xfer polling mode or xfer irq mode
 * @param[in] nor spi_nor ptr
 */
static void xspi_wait_xfer_done(struct spi_nor *nor)
{
    struct xspi_pdata *xspi = nor->host->priv_data;
    uint32_t try_count = 0;

    if (nor->xfer_mode != SPI_NOR_XFER_POLLING_MODE) {

        while ((!g_enable_event) && (try_count++ < 1000000U)) {
            udelay(1u); // wait 1s for indirect write(<=256B) or read(xxB)
        }
        if (try_count >= 1000000U) {
            ssdk_printf(SSDK_NOTICE, "xspi wait %d us, exit\r\n", try_count);
            nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
        }
        g_enable_event = 0;
        s_lockstep_err = xspi_readl(xspi->apb_base + XSPI_LSP_ERR_ST);

        if (s_lockstep_err != 0) {
            writel(s_lockstep_err, xspi->apb_base + XSPI_LSP_ERR_ST);
            ssdk_printf(SSDK_NOTICE, "xspi lsp error status = 0x%x\r\n", s_lockstep_err);
        }
    }
    else {
        while (1) {
            xspi_polling_handler(nor);

            if (g_enable_event == 1) {
                g_enable_event = 0;
                s_lockstep_err = xspi_readl(xspi->apb_base + XSPI_LSP_ERR_ST);

                if (s_lockstep_err != 0) {
                    writel(s_lockstep_err, xspi->apb_base + XSPI_LSP_ERR_ST);
                    ssdk_printf(SSDK_NOTICE, "xspi lsp error status = 0x%x\r\n", s_lockstep_err);
                }

                break;
            } else if (try_count++ >= 10000000U) {
                nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
                ssdk_printf(SSDK_NOTICE, "xspi polling %d times, exit\r\n", try_count);
                break;
            }
        }
    }
}

/**
 * @brief xspi nor parallel mode data prepare
 * @param[in] raw raw data
 * @param[in] dst switch dst data form raw
 * @param[in] len data length
 */
static inline void para_data_prep(const uint8_t *raw, uint8_t *dst,
                                       uint32_t len)
{
    for (uint32_t i = 0; i < len / 2; i++) {
        *(dst + 2 * i) = *(raw + i);
        *(dst + 2 * i + 1) = *(raw + i);
    }
}

/**
 * @brief xspi nor parallel mode check command read data
 * @param[in] raw raw data
 * @param[in] dst switch dst data form raw
 * @param[in] len data length
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static inline int para_data_parse(const uint8_t *raw, uint8_t *dst,
                                       uint32_t len)
{
    for (uint32_t i = 0; i < len / 2; i++) {
        if (*(raw + 2 * i) != *(raw + 2 * i + 1)) {
            ssdk_printf(SSDK_ERR, "xspi parallel read register data error!\r\n");
            return -1;
        }

        *(dst + i) = *(raw + 2 * i);
    }

    return 0;
}

/**
 * @brief xspi nor read register command
 * @param[in] nor spi nor ptr
 * @param[in] cmd spi nor cmd
 * @param[in] addr flash read address
 * @param[out] buf read data buf
 * @param[in] len read data length
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_command_read(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                                  flash_addr_t addr, uint8_t *buf, flash_size_t len)
{
    uint32_t irq_trig_level;
    struct xspi_pdata *xspi = nor->host->priv_data;
    uint8_t *ptr = buf;
    uint8_t local_buf[32] = {0};
    flash_size_t read_length = len;
    flash_addr_t read_addr = addr + nor->offset_address;

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        ptr = local_buf;
        read_length = MAX(4u, len * 2);
        read_addr = 2u * addr;
    }

    xspi_setup_xfer(nor, FLASH_OPT_READ, read_addr, ptr, read_length);

    xspi_proto_setup(nor, cmd, XSPI_INDIRECT_ACCESS_MODE,
                          SPI_NOR_OPS_READ);

    irq_trig_level = MIN(xspi->fifo_depth / 2, DIV_ROUND_UP(read_length, 4u));

    RMWREG32(xspi->apb_base + XSPI_INDIRECT_RD_DMA,
             XSPI_INDIRECT_DMA_TRIG_WML_LSB, 8u, irq_trig_level);

    xspi_int_unmask(xspi, XSPI_INT_EN_FUC, XSPI_INT_ST_FUC_RX_FRE_FULL |
                         XSPI_INT_ST_FUC_INDIRECT_RD_DONE);

    xspi_indirect_trigger(xspi, read_addr, read_length, INDIRECT_READ_FLAG);

    xspi_wait_xfer_done(nor);
    if (FLASH_OPT_FAILED == nor->xfer_info_bottom.opt_result) {
        return -1;
    }

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        return para_data_parse(local_buf, buf, read_length);
    }

    return 0;
}

/**
 * @brief xspi nor write register command
 * @param[in] nor spi nor ptr
 * @param[in] cmd spi nor cmd
 * @param[in] addr flash write address
 * @param[in] buf write data buf
 * @param[in] len write data length
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_command_write(struct spi_nor *nor, struct spi_nor_cmd *cmd,
                                   flash_addr_t addr, const uint8_t *buf,
                                   flash_size_t len)
{
    uint32_t tx_size;
    uint32_t irq_trig_level;
    struct xspi_pdata *xspi = nor->host->priv_data;
    uint8_t *ptr = (uint8_t *)buf;
    uint8_t local_buf[32] = {0};
    flash_size_t write_length = len;
    flash_addr_t write_addr = addr + nor->offset_address;

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        if (len != 0u) {
            write_length = len * 2u;
            para_data_prep(buf, local_buf, write_length);
            ptr = local_buf;
        }

        write_addr = addr * 2u;
    }

    xspi_setup_xfer(nor, FLASH_OPT_WRITE, write_addr, ptr, write_length);

    xspi_proto_setup(nor, cmd, XSPI_INDIRECT_ACCESS_MODE, SPI_NOR_OPS_WRITE);

    xspi_indirect_trigger(xspi, write_addr, write_length, INDIRECT_WRITE_FLAG);

    xspi_int_unmask(xspi, XSPI_INT_EN_FUC, XSPI_INT_ST_FUC_INDIRECT_WR_DONE);

    if (buf != NULL) {
        irq_trig_level = MIN(xspi->fifo_depth, DIV_ROUND_UP(write_length, 4u));
        RMWREG32(xspi->apb_base + XSPI_INDIRECT_TX_BUF, 0, 1u, 1);
        RMWREG32(xspi->apb_base + XSPI_INDIRECT_WR_DMA,
                 XSPI_INDIRECT_DMA_TRIG_WML_LSB, 8u, irq_trig_level);
    }

    if ((nor->xfer_info_bottom.buf != NULL) &&
            (nor->xfer_info_bottom.size > 0u)) {
        tx_size = xspi_write_fifo(xspi, nor->xfer_info_bottom.buf,
                                       nor->xfer_info_bottom.size);
        nor->xfer_info_bottom.buf += tx_size;
        nor->xfer_info_bottom.size -= tx_size;
    }

    xspi_wait_xfer_done(nor);
    if (FLASH_OPT_FAILED == nor->xfer_info_bottom.opt_result) {
        return -1;
    }

    return 0;
}

/**
 * @brief xspi irq handler function
 * @param[in] irq irq num
 * @param[in] arg spi nor host context
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
__UNUSED static int xspi_irq_handler(uint32_t irq, void *arg)
{
    struct spi_nor_host *host = arg;
    struct xspi_pdata *xspi = host->priv_data;
    struct spi_nor *nor = host->dev;
    uint32_t status;

    status = xspi_readl(xspi->apb_base + XSPI_INT_ST_ERR);
    writel(status, xspi->apb_base + XSPI_INT_ST_ERR);

    status = xspi_readl(xspi->apb_base + XSPI_INT_ST_FUC);
    ssdk_printf(SSDK_DEBUG, "xspi irq status = 0x%x\r\n", status);

    if ((status & (XSPI_INT_ST_FUC_IND_COMP)) != 0u) {
        nor->xfer_info_bottom.opt_result = FLASH_OPT_COMPLETE;

        if (nor->async_mode) {
            if (nor->event_handler)
                nor->event_handler(nor->xfer_info_bottom.opt_type,
                                   nor->xfer_info_bottom.opt_result);
        }
        else {
            g_enable_event = 1;
        }

        xspi_writel(0u, xspi->apb_base + XSPI_INT_EN_FUC);
    }
    else if (status &
             (XSPI_INT_ST_FUC_FIFO_LEVEL_TRIG | XSPI_INT_ST_FUC_INDIRECT_RD_DONE)) {
        xspi_polling_handler((void *)nor);
    }

    writel(status, xspi->apb_base + XSPI_INT_ST_FUC);
    return 0;
}

/**
 * @brief xspi r/w prepare setup
 * @param[in] nor spi_nor ptr
 * @param[in] ops r/w ops flags
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_lock(struct spi_nor *nor, enum spi_nor_ops ops)
{
    __UNUSED struct xspi_pdata *xspi = nor->host->priv_data;
    xspi_indr_pre_trans_clean(nor);

    if (NULL == nor->host->dev) {
        nor->host->dev = nor;
        xspi_nor_setup(nor, ops);

        if (nor->xfer_mode != SPI_NOR_XFER_POLLING_MODE) {
            ssdk_printf(SSDK_INFO, "irq enable!\r\n");
            irq_enable(xspi->irq);
        }

        return 0;
    }
    else {
        return -1;
    }
}

/**
 * @brief xspi r/w unprepare setup
 * @param[in] nor spi_nor ptr
 * @param[in] ops r/w ops flags
 */
static void xspi_unlock(struct spi_nor *nor, enum spi_nor_ops ops)
{
    __UNUSED struct xspi_pdata *xspi = nor->host->priv_data;

    if (nor->xfer_mode != SPI_NOR_XFER_POLLING_MODE) {
        ssdk_printf(SSDK_INFO, "irq disable!\r\n");
        irq_disable(xspi->irq);
    }

    nor->host->dev = NULL;
}

static inline void readsl(uint32_t *addr, uint32_t *data, uint32_t len)
{
    while (len-- != 0u) {
        *data++ = readl(addr++);
    }
}

static void xspi_cache_flush(struct spi_nor *nor, flash_addr_t addr,
                             flash_size_t length)
{
    struct xspi_pdata *xspi = nor->host->priv_data;
    if (nor->host->xip_mode) {
        return;  // xip mode can not flush cache
    }

    arch_invalidate_cache_range((addr_t)xspi->direct_base + addr + nor->offset_address,
                                ROUNDUP(length, CONFIG_ARCH_CACHE_LINE));
    xspi_prefetch_flush(xspi, 0);
    xspi_prefetch_flush(xspi, 1);
}

/**
 * @brief xspi read flash data
 * @param[in] nor spi_nor ptr
 * @param[in] addr flash address
 * @param[out] buf read flash data to dest address
 * @param[in] size read flash size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_read(struct spi_nor *nor, flash_addr_t addr, uint8_t *buf,
                          flash_size_t size)
{
    uint32_t irq_trig_level;
    uint32_t mask = 0u;
    struct xspi_pdata *xspi = nor->host->priv_data;
    flash_size_t limit_size = size;
    flash_size_t read_size = 0;
    flash_addr_t current_addr = addr + nor->offset_address;
    flash_size_t size_done = 0;
    uint32_t rel_data;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buf, 4)) {
        return SPI_NOR_ADDRESS_INVALID;
    }

    if (addr < xspi->direct_range_size && XSPI_USE_DIRECT_MODE
            && !nor->async_mode) {
        /* direct access */
        arch_invalidate_cache_range((addr_t)xspi->direct_base + current_addr, ROUNDUP(size,
                                    32));
        // TODO: for hyperflash not support 1byte access, need a new memcpy
        // memcpy(buf, (uint8_t *)xspi->direct_base + addr, size);
        readsl((uint32_t *)(xspi->direct_base + (uint32_t)current_addr), (uint32_t *)buf,
                    size / 4);

        if (0 != (size % 4)) {
            readsl((uint32_t *)(xspi->direct_base + (uint32_t)current_addr + (uint32_t)size / 4 * 4), &rel_data, 1);
            memcpy(buf + (uint32_t)size / 4 * 4, &rel_data, size % 4);
        }

        return 0;
    }
    else {
        if (!IS_ALIGNED(size, 4)) {
            return SPI_NOR_LENGTH_INVALID;
        }

#if CONFIG_E3L
        if (IS_P0)
#endif
        {
            if ((nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE)) {
                limit_size = xspi->fifo_depth * xspi->fifo_width;
            }
        }

        while (size_done < size) {
            if ((size - size_done) > limit_size) {
                read_size = limit_size;
            } else {
                read_size = size - size_done;
            }
            xspi_setup_xfer(nor, FLASH_OPT_READ, current_addr, buf + size_done, read_size);
            xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                                SPI_NOR_OPS_READ);

            if (xspi->fifo_depth & (xspi->fifo_depth - 1)) {
                irq_trig_level = MIN(xspi->fifo_depth, (size / 4u));
             } else {
                irq_trig_level = MIN(xspi->fifo_depth / 2u, (size / 4u));
             }
            irq_trig_level = 1u << _fls(irq_trig_level);

            RMWREG32(xspi->apb_base + XSPI_INDIRECT_RD_DMA,
                    XSPI_INDIRECT_DMA_TRIG_WML_LSB, 8u, irq_trig_level);
            if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
                arch_invalidate_cache_range((addr_t)buf + size_done, ROUNDUP(read_size, 32));
                xspi_dma_enable(xspi);
                ssdk_printf(SSDK_DEBUG, "indirect read length %lld, dma trig level %d\r\n", read_size,
                            irq_trig_level);
                if(nor->dma_xfer_config){
                    nor->dma_xfer_config(nor, current_addr, buf + size_done, true,
                                                read_size, 4u, irq_trig_level);
                }
                mask |= XSPI_INT_ST_FUC_INDIRECT_DMA_RD_DONE;
            }
            else {
                mask |= XSPI_INT_ST_FUC_RX_FRE_FULL | XSPI_INT_ST_FUC_INDIRECT_RD_DONE;
            }

            xspi_int_unmask(xspi, XSPI_INT_EN_FUC, mask);
            xspi_indirect_trigger(xspi, current_addr, read_size, INDIRECT_READ_FLAG);

            if (!nor->async_mode) {
                xspi_wait_xfer_done(nor);
                if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
                    xspi_dma_disable(xspi);
                    /* stop channel transfer */
                    if (nor->dma_xfer_stop) {
                        nor->dma_xfer_stop();
                    } else if (nor->dma_stop) {
                        nor->dma_stop(nor);
                    }
                }
            }
            if (FLASH_OPT_FAILED == nor->xfer_info_bottom.opt_result) {
                break;
            } else {
                current_addr += read_size;
                size_done += read_size;
            }
        }
        return nor->xfer_info_bottom.opt_result;
    }
}

/**
 * @brief xspi write flash data
 * @param[in] nor spi_nor ptr
 * @param[in] addr flash address
 * @param[in] buf write flash data to dest address
 * @param[in] size write flash size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_write(struct spi_nor *nor, flash_addr_t addr,
                           const uint8_t *buf, flash_size_t size)
{
    int ret;
    uint32_t irq_trig_level;
    uint32_t mask = 0u;
    uint32_t addr_tmp = addr + nor->offset_address;
    uint32_t buf_index = 0;
    uint32_t size_tmp = size;
    uint32_t write_size = 0;
    bool hyperram_mode = false;

    hyperram_mode = nor->mem_type == SPI_HYPERRAM ? true : false;

    struct xspi_pdata *xspi = nor->host->priv_data;

    if (!IS_ALIGNED(addr, 4) || !IS_ALIGNED(buf, 4)) {
        return SPI_NOR_ADDRESS_INVALID;
    } else if (!IS_ALIGNED(size, 4)) {
        return SPI_NOR_LENGTH_INVALID;
    }

    if (addr < xspi->direct_range_size && XSPI_USE_DIRECT_MODE && hyperram_mode
            && !nor->async_mode) {
        /* direct access */
        memcpy((uint8_t *)xspi->direct_base + addr_tmp, buf, size);
        arch_clean_invalidate_cache_range((addr_t)xspi->direct_base + addr_tmp,
                                          ROUNDUP(size, 32));
        nor->xfer_info_bottom.opt_result = FLASH_OPT_COMPLETE;
    }
    else {
        xspi_cache_flush(nor, addr, size);
        if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
            arch_clean_cache_range((addr_t)buf, ROUNDUP(size, 32));
        }

        while (size_tmp > 0) {
            write_size = MIN((nor->info.page_size - addr_tmp % nor->info.page_size),
                             size_tmp);
            spi_nor_write_enable(nor, true);

            xspi_setup_xfer(nor, FLASH_OPT_WRITE, addr_tmp, (uint8_t *)buf + buf_index,
                                 write_size);
            xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                                  SPI_NOR_OPS_WRITE);

            if (xspi->fifo_depth & (xspi->fifo_depth - 1)) {
                   irq_trig_level = MIN(xspi->fifo_depth, (size / 4u));
             } else {
                   irq_trig_level = MIN(xspi->fifo_depth / 2u, (size / 4u));
             }
            irq_trig_level =  1u << _fls(irq_trig_level);

            RMWREG32(xspi->apb_base + XSPI_INDIRECT_WR_DMA,
                     XSPI_INDIRECT_DMA_TRIG_WML_LSB, 8u, irq_trig_level);
            if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {
                xspi_dma_enable(xspi);
                if(nor->dma_xfer_config){
                  nor->dma_xfer_config(nor, addr_tmp, (uint8_t *)buf + buf_index,
                                        false, write_size, 4u, irq_trig_level);
                }

                mask |= XSPI_INT_ST_FUC_INDIRECT_WR_DONE;
            }
            else {
                mask |= XSPI_INT_ST_FUC_TX_FRE_EMPTY | XSPI_INT_ST_FUC_INDIRECT_WR_DONE;
            }
            xspi_indirect_trigger(xspi, addr_tmp, write_size, INDIRECT_WRITE_FLAG);
            xspi_int_unmask(xspi, XSPI_INT_EN_FUC, mask);

            if (!nor->async_mode) {
                xspi_wait_xfer_done(nor);
                if (nor->xfer_mode == SPI_NOR_XFER_DMA_MODE) {

                    xspi_dma_disable(xspi);
                    /* stop channel transfer */
                    if(nor->dma_xfer_stop){
                        nor->dma_xfer_stop();
                    } else if (nor->dma_stop) {
                        nor->dma_stop(nor);
                    }
                }
                /* wait for flash idle */
                do {
                    ret = spi_nor_wait_idle(nor);

                    if (ret) {
                        nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
                    }
                }
                while (s_lockstep_err != 0u);
            }

            if (FLASH_OPT_FAILED == nor->xfer_info_bottom.opt_result) {
                break;
            }
            addr_tmp += write_size;
            size_tmp -= write_size;
            buf_index += write_size;
        }

        return nor->xfer_info_bottom.opt_result;
    }

    return 0;
}

/**
 * @brief xspi erase flash data
 * @param[in] nor spi_nor ptr
 * @param[in] offset erase flash address
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int xspi_erase(struct spi_nor *nor, flash_addr_t offset)
{
    int ret;
    flash_addr_t erase_addr = offset;

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        erase_addr = offset / 2u;
    }

    xspi_cache_flush(nor, erase_addr, nor->info.sector_size);

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = 0x6u,
        .addr_bytes = 0,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = xspi_command_write(nor, &write_enable_cmd, 0, NULL, 0);

    if (ret != 0)
        return ret;

    struct spi_nor_cmd erase_cmd = {
        .opcode = (uint8_t)(nor->info.erase_proto >> SNOR_OPCODE_PROTO_LSB),
        .addr_bytes = (uint8_t)nor->addr_width,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = xspi_command_write(nor, &erase_cmd, erase_addr, NULL, 0);

    if (ret != 0)
        return ret;

    /* wait for flash idle */
    do {
        ret = spi_nor_wait_idle(nor);

        if (ret) {
            nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
        }
    }
    while (s_lockstep_err != 0u);

    return ret;
}

int xspi_complex_erase(struct spi_nor *nor, struct spi_nor_erase_cmd *cmd)
{
    int ret;
    flash_addr_t erase_addr = cmd->addr;

    if (nor->dev_mode == SPI_NOR_DEV_PARALLEL_MODE) {
        erase_addr = cmd->addr / 2u;
    }

    xspi_cache_flush(nor, erase_addr, cmd->map->erase_size);

    struct spi_nor_cmd write_enable_cmd = {
        .opcode = 0x6u,
        .addr_bytes = 0,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = xspi_command_write(nor, &write_enable_cmd, 0, NULL, 0);

    if (ret != 0)
        return ret;

    struct spi_nor_cmd erase_cmd = {
        .opcode = (uint8_t)(cmd->map->erase_proto >> SNOR_OPCODE_PROTO_LSB),
        .addr_bytes = (uint8_t)nor->addr_width,
        /* the write enable cmd inst_width need aline with erase cmd */
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = xspi_command_write(nor, &erase_cmd, erase_addr, NULL, 0);

    if (ret != 0)
        return ret;

    /* wait for flash idle */
    do {
        ret = spi_nor_wait_idle(nor);

        if (ret) {
            nor->xfer_info_bottom.opt_result = FLASH_OPT_FAILED;
        }
    }
    while (s_lockstep_err != 0u);

    return ret;
}

/**
 * @brief xspi nor training flash and verify the correctness of the data
 * @param[in] nor spi_nor ptr
 * @param[in] addr training flash address
 * @param[in] buf read buf address
 * @param[in] pattern training pattern data
 * @param[in] size training data size
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_do_training(struct spi_nor *nor, flash_addr_t addr,
                                 uint8_t *buf, const uint8_t *pattern,
                                 flash_size_t size)
{
    int ret = 0;
    uint32_t rx_delay;
    uint32_t rx_delay_start = 0;
    uint32_t training_steps;
    bool train_ok = false;
    struct xspi_pdata *xspi = nor->host->priv_data;

#ifdef XSPI_DLL_USE_OVERRIDE_MODE
    training_steps = 0x7fu;
#else
    training_steps = 0x1fu;
#endif

    for (rx_delay = 0u; rx_delay <= training_steps; rx_delay++) {
        xspi_dll_enable(xspi, 0u, rx_delay, true);
        xspi_dll_enable(xspi, 1u, rx_delay, true);
        spi_nor_read(nor, addr, buf, size);

        s_lockstep_err = xspi_readl(xspi->apb_base + XSPI_LSP_ERR_ST);

        if (s_lockstep_err != 0) {
            writel(s_lockstep_err, xspi->apb_base + XSPI_LSP_ERR_ST);
            ssdk_printf(SSDK_NOTICE, "xspi lsp error status = 0x%x\r\n", s_lockstep_err);
        }

        if (memcmp(buf, pattern, (size_t)size) == 0) {
            if (train_ok == false) {
                train_ok = true;
                rx_delay_start = rx_delay;
            }

            ssdk_printf(SSDK_INFO, "xspi phy training succeful, rx_delay = %d \r\n",
                        rx_delay);
        }
        else {
            if (train_ok && (rx_delay > (rx_delay_start + 10u)))
                break;
            else
                train_ok = false;
        }
    }

    if (train_ok) {
#if (CONFIG_E3 || CONFIG_D3)
        if (IS_1P0) {
            /* For xspi dll first delay sell bigger than others, not use the training window's centre */
            rx_delay = (rx_delay_start + rx_delay) / 2u - 4u;  // for 1.0
        } else
#endif  /* CONFIG_E3 || CONFIG_D3 */
        {
            rx_delay = (rx_delay_start + rx_delay) / 2u;
        }
        xspi_dll_enable(xspi, 0u, rx_delay, true);
        xspi_dll_enable(xspi, 1u, rx_delay, true);
        ssdk_printf(SSDK_INFO, "xspi phy training pass!\r\n");
        ret = 0;
    }
    else {
        xspi_dll_enable(xspi, 0u, 0u, false);
        xspi_dll_enable(xspi, 1u, 0u, false);
        ssdk_printf(SSDK_ERR, "xspi phy training failed, disable dll!\r\n");
        ret = -1;
    }

    xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                            SPI_NOR_OPS_READ);
    xspi_proto_setup(nor, NULL, XSPI_INDIRECT_ACCESS_MODE,
                            SPI_NOR_OPS_WRITE);

    return ret;
}

static void spi_nor_drv_main_function(struct spi_nor *nor)
{
    if (nor->xfer_mode == SPI_NOR_XFER_POLLING_MODE) {
        xspi_polling_handler(nor);
    }
}

/**
 * @brief xspi nor host provide callback function
 */
struct spi_nor_host_ops xspi_host_ops = {
    .reg_read = xspi_command_read,
    .reg_write = xspi_command_write,
    .read = xspi_read,
    .write = xspi_write,
    .erase = xspi_erase,
    .complex_erase = xspi_complex_erase,
    .training = xspi_do_training,
    .prepare = xspi_lock,
    .unprepare = xspi_unlock,
    .drv_main_function = spi_nor_drv_main_function,
    .cache_flush = xspi_cache_flush,
    .enable_rfd = xspi_rfd_en,
    .enter_xip_mode = xspi_enter_xip_mode,
    .xip_proto_setup = xspi_xip_proto_setup,
};

/**
 * @brief xspi hyper command read
 * @param[in] nor spi_nor ptr
 * @param[in] addr flash register addr
 * @param[out] buf  2byte data
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_hyper_read16(struct spi_nor *nor, flash_addr_t addr,
                                  uint16_t *buf)
{
    int ret = 0;

    struct spi_nor_cmd read_cmd = {
        .opcode = 0,
        .dummy = nor->info.read_dummy,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = xspi_command_read(nor, &read_cmd, addr * 2u, (uint8_t *)buf, 2u);

    if (ret)
        return -1;

    return 0;
}

/**
 * @brief xspi hyper command write
 * @param[in] nor spi_nor ptr
 * @param[in] addr flash register addr
 * @param[in] buf  2byte data
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
static int xspi_hyper_write16(struct spi_nor *nor, flash_addr_t addr,
                                   uint16_t *buf)
{
    int ret = 0;

    struct spi_nor_cmd write_cmd = {
        .opcode = 0,
        .dummy = 0,
        .addr_bytes = nor->addr_width,
        .inst_type = SNOR_INST_LANS(nor->reg_proto),
    };

    ret = xspi_command_write(nor, &write_cmd, addr * 2u, (uint8_t *)buf, 2u);

    if (ret)
        return -1;

    return 0;
}

/**
 * @brief xspi hyper setup before transaction
 * @param[in] nor spi_nor ptr
 * @param[in] is_read read or write mode
 * @param[in] is_direct direct or indirect mode
 * @param[in] num hyperbus_ca trans number
 * @param[in] ca_array  hyperbus_ca ptr
 */
static void xspi_hyper_set_pre_transaction(struct spi_nor *nor,
        bool is_read,
        bool is_direct, uint32_t num, struct hyperbus_ca **ca_array)
{
    uint32_t addr;
    uint16_t data;
    addr_t ptc_addr;
    addr_t pta_base_addr;
    addr_t ptd_base_addr;
    struct hyperbus_ca *ca_ptr = *ca_array;
    struct xspi_pdata *xspi = nor->host->priv_data;

    if (nor->mem_type == SPI_HYPERBUS_MCP) {
        for (uint32_t i = 0u; i < num; i++) {
            addr = ca_ptr->addr;
            data = ca_ptr->data;
            xspi_hyper_write16(nor, addr, &data);
            ca_ptr += 1;
        }
    }
    else {
        if (is_read) {
            if (is_direct) {
                ptc_addr = xspi->apb_base + XSPI_DIRECT_RD_PTC;
                pta_base_addr = xspi->apb_base + XSPI_DIRECT_RD_PTA;
                ptd_base_addr = xspi->apb_base + XSPI_DIRECT_RD_PTD;
            }
            else {
                ptc_addr = xspi->apb_base + XSPI_INDR_RD_PTC;
                pta_base_addr = xspi->apb_base + XSPI_INDR_RD_PTA;
                ptd_base_addr = xspi->apb_base + XSPI_INDR_RD_PTD;
            }
        }
        else {
            if (is_direct) {
                ptc_addr = xspi->apb_base + XSPI_DIRECT_WR_PTC;
                pta_base_addr = xspi->apb_base + XSPI_DIRECT_WR_PTA;
                ptd_base_addr = xspi->apb_base + XSPI_DIRECT_WR_PTD;
            }
            else {
                ptc_addr = xspi->apb_base + XSPI_INDR_WR_PTC;
                pta_base_addr = xspi->apb_base + XSPI_INDR_WR_PTA;
                ptd_base_addr = xspi->apb_base + XSPI_INDR_WR_PTD;
            }
        }

        for (uint32_t i = 0u; i < num; i++) {
            addr = ca_ptr->addr;
            data = ca_ptr->data;
            xspi_writel(addr, pta_base_addr);
            xspi_writel(data, ptd_base_addr);

            pta_base_addr += 0x20;
            ptd_base_addr += 0x20;
            ca_ptr += 1;
        }

        /* set pre-tansaction num */
        xspi_writel(num << 8u, ptc_addr);
    }
}

/**
 * @brief xspi hyperbus ops provide callback function
 */
struct hyperbus_host_ops xspi_hyperbus_ops = {
    .read16 = xspi_hyper_read16,
    .write16 = xspi_hyper_write16,
    .read = xspi_read,
    .write = xspi_write,
    .hyperram_en = xspi_hyperram_enable,
    .set_pre_transaction = xspi_hyper_set_pre_transaction,
    .cache_flush = xspi_cache_flush,
};

/**
 * @brief xspi host init
 *
 * @param[in] host spi nor host
 * @param[in] xspi xspi private info
 * @param[in] config xspi config info
 * @return int
 * @retval 0: success
 * @retval other: failed
 */
int xspi_host_init(struct spi_nor_host *host, struct xspi_pdata *xspi,
                        struct xspi_config *config)
{
    if (!host || !xspi || !config) {
        ssdk_printf(SSDK_EMERG, "%s: point is null\r\n", __FUNCTION__);
        return -1;
    }

    memset(host, 0, sizeof(struct spi_nor_host));
    memset(xspi, 0, sizeof(struct xspi_pdata));

    xspi->id = config->id;
    xspi->apb_base = config->apb_base;
    xspi->direct_range_size = 0x8000000;
    xspi->direct_base = config->direct_base;
    xspi->irq = config->irq;
    xspi->ref_clk_hz = config->ref_clk;
    xspi->crc_en = config->crc_en;
    xspi->no_cache = config->no_cache;
    /* initialize the current cs for no one */
    xspi->current_cs = 0xff;
#if CONFIG_E3L
    xspi->fifo_depth = 48;
#else
    xspi->fifo_depth = 192 / 4;
#endif
    xspi->fifo_width = 4u;

    g_enable_event = 0;
    host->ref_clk_hz = config->ref_clk;
    host->priv_data = xspi;
    host->ops = &xspi_host_ops;
    host->hyper_ops = &xspi_hyperbus_ops;
    host->xip_mode = config->xip_mode;

    if (xspi->irq) {
        ssdk_printf(SSDK_INFO, "xspi irq num = %d\r\n", xspi->irq);
        irq_attach(xspi->irq, xspi_irq_handler, (void *)host);
    }

    if (!host->xip_mode) {  // xip mode no need to do initialization
        xspi_init(xspi);
    }

    return 0;
}

void xspi_host_deinit(struct spi_nor *nor)
{
#ifdef SPI_NOR_DMA_ENABLE
    // TODO
#endif
    struct xspi_pdata *xspi = nor->host->priv_data;
    xspi_reset_flash(xspi);
    return;
}

int xspi_protect_clear(struct xspi_pdata *xspi) {
    int ret = 0;
    uint32_t reg = 0;

    for (uint8_t i = 0; i < 4; i++) {
        reg = xspi_readl(xspi->apb_base + XSPI_CMD_MON_CTRL + i * 0x10);
        if (reg & (BIT(29) | BIT(30) | BIT(31))) {
            ssdk_printf(SSDK_EMERG, "xspi protect config(%d) locked(%x),"
            " please reset xspi\r\n", i, reg);
            return -1;
        }

        xspi_writel(0u, xspi->apb_base + XSPI_MON_REGION_START_ADDR + i * 0x10);
        xspi_writel(0u, xspi->apb_base + XSPI_MON_REGION_END_ADDR + i * 0x10);
        xspi_writel(0u, xspi->apb_base + XSPI_MON_CMD_VALUE + i * 0x10);
        xspi_writel(0u, xspi->apb_base + XSPI_CMD_MON_CTRL + i * 0x10);
    }
    return ret;
}

int xspi_protect_setup(struct xspi_pdata *xspi, protect_config_t *config,
    uint8_t num) {
    int ret = 0;

    if (num > 4) {
        ssdk_printf(SSDK_EMERG, "xspi protect config num %d > max(4)\r\n", num);
        return -1;
    }
    // TODO: check address param
    // ...

    for (uint8_t i = 0; i < num; i++) {
        protect_config_t *p_cfg = &config[i];
        uint32_t monitor_ctrl = 0;
        uint32_t temp_cmd = 0;

        for (uint8_t j = 0; j < 4; j++) {
            if (p_cfg->cmd[j]) {
                monitor_ctrl |= BIT(0 + j);
                monitor_ctrl |= BIT(4 + j);
                temp_cmd |= p_cfg->cmd[j] << (8 * j);
            }
        }

        if (temp_cmd) {
            ssdk_printf(SSDK_INFO, "Setup protect command: cmd %x\r\n", temp_cmd);
            xspi_writel(temp_cmd, xspi->apb_base + XSPI_MON_CMD_VALUE + i * 0x10);
            if (p_cfg->lock_en) {
                monitor_ctrl |= BIT(31);
            }
        }

        if (p_cfg->size > 0) {
            ssdk_printf(SSDK_INFO, "Setup protect area: start %llx, end %llx\r\n",
                p_cfg->start_addr, p_cfg->start_addr + p_cfg->size);
            xspi_writel(p_cfg->start_addr, xspi->apb_base +
                XSPI_MON_REGION_START_ADDR + i * 0x10);
            xspi_writel(p_cfg->start_addr + p_cfg->size, xspi->apb_base +
                XSPI_MON_REGION_END_ADDR + i * 0x10);
            monitor_ctrl |= BIT(9);
            monitor_ctrl |= BIT(8);

            if (p_cfg->lock_en) {
                monitor_ctrl |= BIT(29);
                monitor_ctrl |= BIT(30);
            }
        }

        xspi_writel(monitor_ctrl, xspi->apb_base + XSPI_CMD_MON_CTRL + i * 0x10);
    }
    return ret;
}
