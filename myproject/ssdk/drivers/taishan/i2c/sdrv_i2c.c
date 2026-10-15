/**
 * @file sdrv_i2c.c
 * @low level sdrv i2c driver file.
 *
 * @Copyright (c) 2020  Semidrive Semiconductor.
 * @All rights reserved.
 *
 **/

#include <types.h>
#include <stdio.h>
#include <reg.h>
#include <board.h>

#include "sdrv_i2c.h"

uint8_t sdrv_i2c_lld_recv_data(paddr_t base)
{
    return readl(base + I2C_RXFIFO);
}

void sdrv_i2c_lld_send_data(paddr_t base, uint8_t data)
{
    writel(data, base + I2C_TXFIFO);
}

uint32_t sdrv_i2c_lld_get_psr0_stat(paddr_t base)
{
    return readl(base + I2C_PSR0);
}

uint32_t sdrv_i2c_lld_get_wspace(paddr_t base)
{
    return (readl(base + I2C_FSR) >> 8) & 0xff;
}

uint32_t sdrv_i2c_lld_get_rspace(paddr_t base)
{
    return readl(base + I2C_FSR) & 0xff;
}

void sdrv_i2c_lld_set_watermark(paddr_t base, uint32_t tx_lvl,
                                uint32_t rx_lvl)
{
    writel(tx_lvl | rx_lvl << 8, base + I2C_FCR);
}

void sdrv_i2c_lld_clear_fifo(paddr_t base)
{
    uint32_t val;
    sdrv_i2c_lld_read_reg(base, I2C_FCR, &val);
    sdrv_i2c_lld_write_reg(base, I2C_FCR, val | 0x1 << 16);
}

void sdrv_i2c_lld_set_cmdcsr0(paddr_t base, uint32_t cmdcsr0)
{
    writel(cmdcsr0, base + I2C_CMDCSR0);
}

void sdrv_i2c_lld_set_cmdcsr1(paddr_t base, uint32_t cmdcsr1)
{
    writel(cmdcsr1, base + I2C_CMDCSR1);
}

void sdrv_i2c_lld_set_cmdcsr2(paddr_t base, uint32_t cmdcsr2)
{
    writel(cmdcsr2, base + I2C_CMDCSR2);
}

void sdrv_i2c_lld_set_cmdcsr3(paddr_t base, uint32_t cmdcsr3)
{
    writel(cmdcsr3, base + I2C_CMDCSR3);
}

uint32_t sdrv_i2c_lld_get_cmdcsr0(paddr_t base)
{
    return readl(base + I2C_CMDCSR0);
}

uint32_t sdrv_i2c_lld_get_cmdcsr1(paddr_t base)
{
    return readl(base + I2C_CMDCSR1);
}

uint32_t sdrv_i2c_lld_get_cmdcsr2(paddr_t base)
{
    return readl(base + I2C_CMDCSR2);
}

uint32_t sdrv_i2c_lld_get_cmdcsr3(paddr_t base)
{
    return readl(base + I2C_CMDCSR3);
}

void sdrv_i2c_lld_clear_int(paddr_t base)
{
    writel(~0x0, base + I2C_INTR0);
    writel(~0x0, base + I2C_INTR1);
    writel(~0x0, base + I2C_INTR2);
    writel(~0x0, base + I2C_INTR3);
}

uint32_t sdrv_i2c_lld_get_int0_stat(paddr_t base)
{
    return readl(base + I2C_INTR0);
}

uint32_t sdrv_i2c_lld_get_int1_stat(paddr_t base)
{
    return readl(base + I2C_INTR1);
}

uint32_t sdrv_i2c_lld_get_int2_stat(paddr_t base)
{
    return readl(base + I2C_INTR2);
}

uint32_t sdrv_i2c_lld_get_int3_stat(paddr_t base)
{
    return readl(base + I2C_INTR3);
}

void sdrv_i2c_lld_unmask_int0(paddr_t base, uint32_t int_flag)
{
    writel(int_flag, base + I2C_INTEN0);
}

void sdrv_i2c_lld_unmask_int1(paddr_t base, uint32_t int_flag)
{
    writel(int_flag, base + I2C_INTEN1);
}

void sdrv_i2c_lld_unmask_int2(paddr_t base, uint32_t int_flag)
{
    writel(int_flag, base + I2C_INTEN2);
}

void sdrv_i2c_lld_unmask_int3(paddr_t base, uint32_t int_flag)
{
    writel(int_flag, base + I2C_INTEN3);
}

void sdrv_i2c_lld_set_IO0(paddr_t base, uint32_t val)
{
    writel(val, base + I2C_PCR8);
}

void sdrv_i2c_lld_set_IO1(paddr_t base, uint32_t val)
{
    writel(val, base + I2C_PCR9);
}

void sdrv_i2c_lld_set_IO2(paddr_t base, uint32_t val)
{
    writel(val, base + I2C_PCR10);
}

void sdrv_i2c_lld_set_IO3(paddr_t base, uint32_t val)
{
    writel(val, base + I2C_PCR11);
}

void sdrv_i2c_lld_set_speed(paddr_t base, uint32_t speed_mode)
{
    uint32_t val;
    sdrv_i2c_lld_read_reg(base, I2C_PCR0, &val);
    sdrv_i2c_lld_write_reg(base, I2C_PCR0, (val & ~0xff) | speed_mode);
}

void sdrv_i2c_lld_set_slvaddr(paddr_t base, uint8_t addr)
{
    writel(addr, base + I2C_PCR1);
}

void sdrv_i2c_lld_set_timing(paddr_t base,
                             uint32_t pcr2, uint32_t pcr3, uint32_t pcr4, uint32_t pcr5)
{
    writel(pcr2, base + I2C_PCR2);
    writel(pcr3, base + I2C_PCR3);
    writel(pcr4, base + I2C_PCR4);
    writel(pcr5, base + I2C_PCR5);
}

void sdrv_i2c_lld_set_opmode(paddr_t base, uint32_t opmode)
{
    uint32_t val;
    sdrv_i2c_lld_read_reg(base, I2C_MCR0, &val);
    sdrv_i2c_lld_write_reg(base, I2C_MCR0, (val & ~0xf0) | (opmode << 4));
}

void sdrv_i2c_lld_disable(paddr_t base)
{
    uint32_t val;
    sdrv_i2c_lld_read_reg(base, I2C_MCR0, &val);
    sdrv_i2c_lld_write_reg(base, I2C_MCR0, val & ~0x1);
}

void sdrv_i2c_lld_enable(paddr_t base)
{
    uint32_t val;
    sdrv_i2c_lld_read_reg(base, I2C_MCR0, &val);
    sdrv_i2c_lld_write_reg(base, I2C_MCR0, val | 0x1);
}

int sdrv_i2c_lld_reset(paddr_t base, bool init)
{
    uint32_t timeout = 100;
    uint32_t val;

    sdrv_i2c_lld_disable(base);
    sdrv_i2c_lld_read_reg(base, I2C_MCR0, &val);
    sdrv_i2c_lld_write_reg(base, I2C_MCR0, val | 0x2);

    do {
        sdrv_i2c_lld_read_reg(base, I2C_MCR0, &val);
        if (!(val & 0x2)){
            sdrv_i2c_lld_enable(base);
            return 0;
        }
        udelay(2);
    } while (timeout--);

    return -1;
}

void sdrv_i2c_lld_dump_reg(paddr_t base, uint32_t *reg_val)
{
    reg_val[0] = readl(base + I2C_MCR0);
    reg_val[1] = readl(base + I2C_INTR0);
    reg_val[2] = readl(base + I2C_INTR1);
    reg_val[3] = readl(base + I2C_INTR2);
    reg_val[4] = readl(base + I2C_INTR3);
    reg_val[5] = readl(base + I2C_INTEN0);
    reg_val[6] = readl(base + I2C_INTEN1);
    reg_val[7] = readl(base + I2C_INTEN2);
    reg_val[8] = readl(base + I2C_INTEN3);
    reg_val[9] = readl(base + I2C_CMDCSR0);
    reg_val[10] = readl(base + I2C_CMDCSR1);
    reg_val[11] = readl(base + I2C_CMDCSR2);
    reg_val[12] = readl(base + I2C_CMDCSR3);
    reg_val[13] = readl(base + I2C_FCR);
    reg_val[14] = readl(base + I2C_FSR);
    reg_val[15] = readl(base + I2C_DMACR);
    reg_val[16] = readl(base + I2C_DMASR);
    reg_val[17] = readl(base + I2C_PCR0);
    reg_val[18] = readl(base + I2C_PCR1);
    reg_val[19] = readl(base + I2C_PCR2);
    reg_val[20] = readl(base + I2C_PCR3);
    reg_val[21] = readl(base + I2C_PCR4);
    reg_val[22] = readl(base + I2C_PCR5);
    reg_val[23] = readl(base + I2C_PCR6);
    reg_val[24] = readl(base + I2C_PCR7);
    reg_val[25] = readl(base + I2C_PCR8);
    reg_val[26] = readl(base + I2C_PCR9);
    reg_val[27] = readl(base + I2C_PCR10);
    reg_val[28] = readl(base + I2C_PCR11);
    reg_val[29] = readl(base + I2C_PSR0);
}

void sdrv_i2c_lld_write_reg(paddr_t base, uint32_t reg, uint32_t val)
{
    writel(val, base + reg);
}

void sdrv_i2c_lld_read_reg(paddr_t base, uint32_t reg, uint32_t *val)
{
    *val = readl(base + reg);
}

