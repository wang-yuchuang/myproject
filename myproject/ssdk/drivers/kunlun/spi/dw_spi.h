/**
 * @file dw_spi.h
 * @brief spi lld driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef _DW_SPI_H_
#define _DW_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <param.h>
#include <reg.h>
#include <types.h>

/* SPI CTRLR0 */
#define SPI_CTRLR0_BASE (0x0u)
#define SPI_CTRLR0_SSTE_FIELD_OFFSET (24u)
#define SPI_CTRLR0_SSTE_FIELD_SIZE (1u)
#define SPI_CTRLR0_DFS32_FIELD_OFFSET (16u)
#define SPI_CTRLR0_DFS32_FIELD_SIZE (5u)
#define SPI_CTRLR0_SRL_FIELD_OFFSET (11u)
#define SPI_CTRLR0_SRL_FIELD_SIZE (1u)
#define SPI_CTRLR0_SCPOL_FIELD_OFFSET (7u)
#define SPI_CTRLR0_SCPOL_FIELD_SIZE (1u)
#define SPI_CTRLR0_SCPH_FIELD_OFFSET (6u)
#define SPI_CTRLR0_SCPH_FIELD_SIZE (1u)
#define SPI_CTRLR0_FRF_FIELD_OFFSET (4u)
#define SPI_CTRLR0_FRF_FIELD_SIZE (2u)

/* SPI CTRLR1 */
#define SPI_CTRLR1_BASE (0x4u)
#define SPI_CTRLR1_ODD_FIELD_OFFSET (16u)
#define SPI_CTRLR1_ODD_FIELD_SIZE (3u)
#define SPI_CTRLR1_NDF_FIELD_OFFSET (0u)
#define SPI_CTRLR1_NDF_FIELD_SIZE (16u)

/* SPI SSIENR */
#define SPI_SSIENR_BASE (0x8u)
#define SPI_SSIENR_SSI_EN_FIELD_OFFSET (0u)
#define SPI_SSIENR_SSI_EN_FIELD_SIZE (1u)

/* SPI SER */
#define SPI_SER_BASE (0x10u)

/* SPI BAUDR */
#define SPI_BAUDR_BASE (0x14u)
#define SPI_BAUDR_SCKDV_FIELD_OFFSET (0u)
#define SPI_BAUDR_SCKDV_FIELD_SIZE (16u)

/* SPI TXFTLR */
#define SPI_TXFTLR_BASE (0x18u)
#define SPI_TXFTLR_TFT_FIELD_OFFSET (0u)
#define SPI_TXFTLR_TFT_FIELD_SIZE (8u)

/* SPI RXFTLR */
#define SPI_RXFTLR_BASE (0x1cu)
#define SPI_RXFTLR_RFT_FIELD_OFFSET (0u)
#define SPI_RXFTLR_RFT_FIELD_SIZE (8u)

/* SPI TXFLR */
#define SPI_TXFLR_BASE (0x20u)
#define SPI_TXFLR_TXTFL_FIELD_OFFSET (0u)
#define SPI_TXFLR_TXTFL_FIELD_SIZE (8u)

/* SPI RXFLR */
#define SPI_RXFLR_BASE (0x24u)
#define SPI_RXFLR_RXTFL_FIELD_OFFSET (0u)
#define SPI_RXFLR_RXTFL_FIELD_SIZE (8u)

/* SPI SR */
#define SPI_SR_BASE (0x28u)
#define SPI_SR_BUSY_FIELD_OFFSET (0u)

/* SPI IMR */
#define SPI_IMR_BASE (0x2cu)
#define SPI_IMR_MSTIM_FIELD_OFFSET (5u)
#define SPI_IMR_MSTIM_FIELD_SIZE (1u)
#define SPI_IMR_RXFIM_FIELD_OFFSET (4u)
#define SPI_IMR_RXFIM_FIELD_SIZE (1u)
#define SPI_IMR_RXOIM_FIELD_OFFSET (3u)
#define SPI_IMR_RXOIM_FIELD_SIZE (1u)
#define SPI_IMR_RXUIM_FIELD_OFFSET (2u)
#define SPI_IMR_RXUIM_FIELD_SIZE (1u)
#define SPI_IMR_TXOIM_FIELD_OFFSET (1u)
#define SPI_IMR_TXOIM_FIELD_SIZE (1u)
#define SPI_IMR_TXEIM_FIELD_OFFSET (0u)
#define SPI_IMR_TXEIM_FIELD_SIZE (1u)
#define SPI_IMR_ALL_IRQ (0x3fu)

/* SPI ISR */
#define SPI_ISR_BASE (0x30u)
#define SPI_ISR_MSTIS_FIELD_OFFSET (5u)
#define SPI_ISR_MSTIS_FIELD_SIZE (1u)
#define SPI_ISR_RXFIS_FIELD_OFFSET (4u)
#define SPI_ISR_RXFIS_FIELD_SIZE (1u)
#define SPI_ISR_RXOIS_FIELD_OFFSET (3u)
#define SPI_ISR_RXOIS_FIELD_SIZE (1u)
#define SPI_ISR_RXUIS_FIELD_OFFSET (2u)
#define SPI_ISR_RXUIS_FIELD_SIZE (1u)
#define SPI_ISR_TXOIS_FIELD_OFFSET (1u)
#define SPI_ISR_TXOIS_FIELD_SIZE (1u)
#define SPI_ISR_TXEIS_FIELD_OFFSET (0u)
#define SPI_ISR_TXEIS_FIELD_SIZE (1u)

/* SPI RISR */
#define SPI_RISR_BASE (0x34u)

/* SPI ICR */
#define SPI_ICR_BASE (0x48u)
#define SPI_ICR_ICR_FIELD_OFFSET (0u)
#define SPI_ICR_ICR_FIELD_SIZE (1u)

/* SPI DMACR */
#define SPI_DMACR_BASE (0x4cu)

/* SPI DMATDLR */
#define SPI_DMATDLR_BASE (0x50u)

/* SPI DMARDLR */
#define SPI_DMARDLR_BASE (0x54u)

/* SPI DR0 */
#define SPI_DR0_BASE (0x60u)

static inline void spi_lld_enable(paddr_t base, bool is_enable)
{
    RMWREG32(base + SPI_SSIENR_BASE, SPI_SSIENR_SSI_EN_FIELD_OFFSET,
             SPI_SSIENR_SSI_EN_FIELD_SIZE, !!is_enable);
}

static inline void spi_lld_set_frame_size(paddr_t base, uint8_t size)
{
    RMWREG32(base + SPI_CTRLR0_BASE, SPI_CTRLR0_DFS32_FIELD_OFFSET,
             SPI_CTRLR0_DFS32_FIELD_SIZE,
             (size - 1) & ((1u << SPI_CTRLR0_DFS32_FIELD_SIZE) - 1));
}

static inline void spi_lld_set_srl(paddr_t base, bool is_enable)
{
    RMWREG32(base + SPI_CTRLR0_BASE, SPI_CTRLR0_SRL_FIELD_OFFSET,
             SPI_CTRLR0_SRL_FIELD_SIZE, !!is_enable);
}

static inline void spi_lld_set_clk_pol(paddr_t base, bool is_high)
{
    RMWREG32(base + SPI_CTRLR0_BASE, SPI_CTRLR0_SCPOL_FIELD_OFFSET,
             SPI_CTRLR0_SCPOL_FIELD_SIZE, !!is_high);
}

static inline void spi_lld_set_clk_phase(paddr_t base, bool is_first_edge)
{
    RMWREG32(base + SPI_CTRLR0_BASE, SPI_CTRLR0_SCPH_FIELD_OFFSET,
             SPI_CTRLR0_SCPH_FIELD_SIZE, !!is_first_edge);
}

static inline void spi_lld_set_ctrl1_ndf(paddr_t base, uint16_t val)
{
    RMWREG32(base + SPI_CTRLR1_BASE, SPI_CTRLR1_NDF_FIELD_OFFSET,
             SPI_CTRLR1_NDF_FIELD_SIZE, (val - 1) & ((1u << SPI_CTRLR1_NDF_FIELD_SIZE) - 1));
}

static inline void spi_lld_set_ctrl1_odd(paddr_t base, uint16_t val)
{
    RMWREG32(base + SPI_CTRLR1_BASE, SPI_CTRLR1_ODD_FIELD_OFFSET,
             SPI_CTRLR1_ODD_FIELD_SIZE, val & ((1u << SPI_CTRLR1_ODD_FIELD_SIZE) - 1));
}

static inline void spi_lld_set_cs(paddr_t base, bool is_enable, uint16_t cs)
{
    if (is_enable)
        writel(1u << cs, base + SPI_SER_BASE);
    else
        writel(0, base + SPI_SER_BASE);
}

static inline void spi_lld_set_clk(paddr_t base, uint32_t sclk_hz,
                                   uint32_t freq_hz)
{
    uint32_t div = (DIV_ROUND_UP(sclk_hz, freq_hz) + 1) & 0xfffe;

    RMWREG32(base + SPI_BAUDR_BASE, SPI_BAUDR_SCKDV_FIELD_OFFSET,
             SPI_BAUDR_SCKDV_FIELD_SIZE, div & ((1u << SPI_BAUDR_SCKDV_FIELD_SIZE) - 1));
}

static inline void spi_lld_set_txftl(paddr_t base, uint16_t value)
{
    RMWREG32(base + SPI_TXFTLR_BASE, SPI_TXFTLR_TFT_FIELD_OFFSET,
             SPI_TXFTLR_TFT_FIELD_SIZE, value & ((1u << SPI_TXFTLR_TFT_FIELD_SIZE) - 1));
}

static inline void spi_lld_set_rxftl(paddr_t base, uint16_t value)
{
    RMWREG32(base + SPI_RXFTLR_BASE, SPI_RXFTLR_RFT_FIELD_OFFSET,
             SPI_RXFTLR_RFT_FIELD_SIZE, value & ((1u << SPI_RXFTLR_RFT_FIELD_SIZE) - 1));
}

static inline uint16_t spi_lld_get_txftl(paddr_t base)
{
    return readl(base + SPI_TXFTLR_BASE);
}

static inline uint16_t spi_lld_get_rxftl(paddr_t base)
{
    return readl(base + SPI_RXFTLR_BASE);
}

static inline uint16_t spi_lld_get_txfl(paddr_t base)
{
    uint32_t value = readl(base + SPI_TXFLR_BASE);
    return (value >> SPI_TXFLR_TXTFL_FIELD_OFFSET) & ((1u <<
            SPI_TXFLR_TXTFL_FIELD_SIZE) - 1);
}

static inline uint16_t spi_lld_get_rxfl(paddr_t base)
{
    uint32_t value = readl(base + SPI_RXFLR_BASE);
    return (value >> SPI_RXFLR_RXTFL_FIELD_OFFSET) & ((1u <<
            SPI_RXFLR_RXTFL_FIELD_SIZE) - 1);
}

static inline bool spi_lld_is_busy(paddr_t base)
{
    return !!(readl(base + SPI_SR_BASE) & (1u << SPI_SR_BUSY_FIELD_OFFSET));
}

static inline void spi_lld_mask_irq(paddr_t base, uint8_t mask)
{
    uint32_t value = readl(base + SPI_IMR_BASE);
    value &= ~mask;
    writel(value, base + SPI_IMR_BASE);
}

static inline void spi_lld_umask_irq(paddr_t base, uint8_t mask)
{
    uint32_t value = readl(base + SPI_IMR_BASE);
    value |= mask;
    writel(value, base + SPI_IMR_BASE);
}

static inline uint32_t spi_lld_get_isr(paddr_t base)
{
    return readl(base + SPI_ISR_BASE);
}

static inline uint32_t spi_lld_get_risr(paddr_t base)
{
    return readl(base + SPI_RISR_BASE);
}

static inline void spi_lld_clear_irq(paddr_t base)
{
    readl(base + SPI_ICR_BASE);
}

static inline void spi_lld_write_data(paddr_t base, uint32_t data)
{
    writel(data, base + SPI_DR0_BASE);
}

static inline uint32_t spi_lld_read_data(paddr_t base)
{
    return readl(base + SPI_DR0_BASE);
}

static inline void spi_lld_enable_dma(paddr_t base, bool tx_enable,
                                      bool rx_enable)
{
    writel(rx_enable | (tx_enable << 1), base + SPI_DMACR_BASE);
}

static inline void spi_lld_set_dmatdl(paddr_t base, uint32_t level)
{
    writel(level, base + SPI_DMATDLR_BASE);
}

static inline void spi_lld_set_dmardl(paddr_t base, uint32_t level)
{
    writel(level, base + SPI_DMARDLR_BASE);
}

static inline void spi_lld_set_sste(paddr_t base, bool is_sste)
{
    RMWREG32(base + SPI_CTRLR0_BASE, SPI_CTRLR0_SSTE_FIELD_OFFSET,
             SPI_CTRLR0_SSTE_FIELD_SIZE, !!is_sste);
}
#ifdef __cplusplus
}
#endif

#endif
