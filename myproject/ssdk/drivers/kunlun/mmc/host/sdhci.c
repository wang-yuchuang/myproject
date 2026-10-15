/* Copyright (c) 2013-2015, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <string.h>
#include <arch.h>
#include <debug.h>
#include <errno.h>
#include <compiler.h>
#include <param.h>
#include <arch/cache.h>
#include <board.h>
#include <driver/irq.h>
#include <sd_errcode.h>

#include <driver/kunlun/mmc/mmc_sdhci.h>
#include <driver/kunlun/mmc/sdhci.h>
#include <driver/memmap.h>

#define SDHCI_TOUT_TIMER_CLK (1000000)
#define MAX_TUNING_LOOP 140
#define lower_32_bits(n) ((uint32_t)(n))
#define upper_32_bits(n) ((uint32_t)(((n) >> 16) >> 16))
#define IS_ARCH_CACHE_LINE_ALIGNED(addr) !((addr_t)(addr) & (CONFIG_ARCH_CACHE_LINE - 1))

#define SDHCI_TRANS_SUPPORT_INTERRUPT 1
#if ((defined SDHCI_TRANS_SUPPORT_INTERRUPT) && !(defined CONFIG_OS_POLL))
#define SDHCI_SUPPORT_INTERRUPT 1
#endif
#define SDHCI_TRANS_USE_ADMA 1
#define INT_STACK_SZ 1024

#define ADMA_BOUNDARY_SIZE (0x8000000)
#define CALC_BOUNDARY(addr, boundary) ((addr + boundary) & ~(boundary - 1))

static struct desc_entry g_dma_sg_list[ADMA_DESC_LIST_SIZE] __ALIGNED(
    CONFIG_ARCH_CACHE_LINE);

static void sdhci_dumpregs(struct sdhci_host *host)
{
    ssdk_printf(SSDK_CRIT,
                "*************** SDHC REG DUMP START *****************\n");
    ssdk_printf(SSDK_CRIT, "Version:      0x%08x\n",
                REG_READ16(host, SDHCI_SPEC_VERSION_REG));
    ssdk_printf(SSDK_CRIT, "Arg2:         0x%08x\t Blk Cnt:      0x%08x\n",
                REG_READ32(host, SDHCI_SDMASA_BLKCNT_REG),
                REG_READ16(host, SDHCI_BLK_CNT_REG));
    ssdk_printf(SSDK_CRIT, "Arg1:         0x%08x\t Blk Sz :      0x%08x\n",
                REG_READ32(host, SDHCI_ARGUMENT_REG),
                REG_READ16(host, SDHCI_BLKSZ_REG));
    ssdk_printf(SSDK_CRIT, "Command:      0x%08x\t Trans mode:   0x%08x\n",
                REG_READ16(host, SDHCI_CMD_REG),
                REG_READ16(host, SDHCI_TRANS_MODE_REG));
    ssdk_printf(SSDK_CRIT, "Resp0:        0x%08x\t Resp1:        0x%08x\n",
                REG_READ32(host, SDHCI_RESP_REG),
                REG_READ32(host, SDHCI_RESP_REG + 0x4));
    ssdk_printf(SSDK_CRIT, "Resp2:        0x%08x\t Resp3:        0x%08x\n",
                REG_READ32(host, SDHCI_RESP_REG + 0x8),
                REG_READ32(host, SDHCI_RESP_REG + 0xC));
    ssdk_printf(SSDK_CRIT, "Prsnt State:  0x%08x\t Host Ctrl1:   0x%08x\n",
                REG_READ32(host, SDHCI_PRESENT_STATE_REG),
                REG_READ8(host, SDHCI_HOST_CTRL1_REG));
    ssdk_printf(SSDK_CRIT, "Timeout ctrl: 0x%08x\t Power Ctrl:   0x%08x\n",
                REG_READ8(host, SDHCI_TIMEOUT_REG),
                REG_READ8(host, SDHCI_PWR_CTRL_REG));
    ssdk_printf(SSDK_CRIT, "Error stat:   0x%08x\t Int Status:   0x%08x\n",
                REG_READ16(host, SDHCI_ERR_INT_STS_REG),
                REG_READ16(host, SDHCI_NRML_INT_STS_REG));
    ssdk_printf(SSDK_CRIT, "Host Ctrl2:   0x%08x\t Clock ctrl:   0x%08x\n",
                REG_READ16(host, SDHCI_HOST_CTRL2_REG),
                REG_READ16(host, SDHCI_CLK_CTRL_REG));
    ssdk_printf(SSDK_CRIT, "Caps1:        0x%08x\t Caps2:        0x%08x\n",
                REG_READ32(host, SDHCI_CAPS_REG1),
                REG_READ32(host, SDHCI_CAPS_REG2));
    ssdk_printf(SSDK_CRIT, "Adma Err:     0x%08x\t Auto Cmd err: 0x%08x\n",
                REG_READ8(host, SDHCI_ADM_ERR_REG),
                REG_READ16(host, SDHCI_AUTO_CMD_ERR));
    ssdk_printf(SSDK_CRIT, "Adma addr1:   0x%08x\t Adma addr2:   0x%08x\n",
                REG_READ32(host, SDHCI_ADM_ADDR_REG),
                REG_READ32(host, SDHCI_ADM_ADDR_REG + 0x4));
}

/*
 * Function: sdhci reset
 * Arg     : Host structure & mask to write to reset register
 * Return  : None
 * Flow:   : Reset the host controller
 */
void sdhci_reset(struct sdhci_host *host, uint8_t mask)
{
    uint32_t reg;
    uint32_t timeout = SDHCI_RESET_MAX_TIMEOUT;
    REG_WRITE8(host, mask, SDHCI_RESET_REG);

    /* Wait for the reset to complete */
    do {
        reg = REG_READ8(host, SDHCI_RESET_REG);
        reg &= mask;

        if (!reg) {
            break;
        }

        if (!timeout) {
            ssdk_printf(SSDK_CRIT, "Error: sdhci reset failed for: %x\n", mask);
            break;
        }

        timeout--;
        udelay(1);
    }
    while (1);
}

/*
 * Function: sdhci error status enable
 * Arg     : Host structure
 * Return  : None
 * Flow:   : Enable command error status
 */
static void sdhci_error_status_enable(struct sdhci_host *host)
{
    /* Enable all interrupt status */
    REG_WRITE16(host, SDHCI_NRML_INT_STS_EN, SDHCI_NRML_INT_STS_EN_REG);
    REG_WRITE16(host, SDHCI_ERR_INT_STS_EN, SDHCI_ERR_INT_STS_EN_REG);
    /* Enable all interrupt signal */
    REG_WRITE16(host, SDHCI_NRML_INT_SIG_EN, SDHCI_NRML_INT_SIG_EN_REG);
    REG_WRITE16(host, SDHCI_ERR_INT_SIG_EN, SDHCI_ERR_INT_SIG_EN_REG);
}

/*
 * Function: sdhci set bus power
 * Arg     : Host structure
 * Return  : None
 * Flow:   : 1. Set the voltage
 *           2. Set the sd power control register
 */
static void sdhci_set_bus_power_on(struct sdhci_host *host)
{
    uint16_t ctrl;
    uint8_t voltage;
    voltage = host->caps.voltage;
    voltage <<= SDHCI_BUS_VOL_SEL;
    REG_WRITE8(host, voltage, SDHCI_PWR_CTRL_REG);
    voltage |= SDHCI_BUS_PWR_EN;
    REG_WRITE8(host, voltage, SDHCI_PWR_CTRL_REG);
    /* switch signal voltage by host ctrl2 register */
    ctrl = REG_READ16(host, SDHCI_HOST_CTRL2_REG);

    if (host->caps.voltage < SDHCI_VOL_3_3) {
        ctrl |= SDHCI_1_8_VOL_SET;
    }
    else {
        ctrl &= ~SDHCI_1_8_VOL_SET;
    }

    REG_WRITE16(host, ctrl, SDHCI_HOST_CTRL2_REG);
    ssdk_printf(SSDK_DEBUG, "\n %s: voltage: 0x%02x; host ctrl2: 0x%04x\n",
                __func__, voltage,
                ctrl);
}

/*
 * Function: sdhci set uhs mode, needs to be used together with set clock api
 * Arg     : Host structure, UHS mode
 * Return  : None
 * Flow:   : 1. Disable the clock
 *           2. Enable UHS mode
 *           3. Enable the clock
 * Details :
 */
void sdhci_set_uhs_mode(struct sdhci_host *host, uint32_t mode)
{
    uint8_t ctrl = 0;
    uint16_t ctrl2 = 0;
    uint16_t clk = 0;

    /* Disable the card clock */
    clk = REG_READ16(host, SDHCI_CLK_CTRL_REG);
    clk &= ~SDHCI_CLK_EN;
    REG_WRITE16(host, clk, SDHCI_CLK_CTRL_REG);

    ctrl = REG_READ8(host, SDHCI_HOST_CTRL1_REG);

    if (mode > SDHCI_UHS_SDR12_MODE)
        ctrl |= SDHCI_HIGH_SPEED_EN;
    else
        ctrl &= ~SDHCI_HIGH_SPEED_EN;

    REG_WRITE8(host, ctrl, SDHCI_HOST_CTRL1_REG);

    /* sets the uhs mode */
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 &= ~SDHCI_UHS_MODE_MASK;
    ctrl2 |= mode & SDHCI_UHS_MODE_MASK;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
}

/*
 * Function: sdhci set adma mode
 * Arg     : Host structure
 * Return  : None
 * Flow:   : Set adma mode
 */
static void sdhci_set_adma_mode(struct sdhci_host *host)
{
    uint8_t ctrl1;
    uint16_t ctrl2;
    ctrl1 = REG_READ8(host, SDHCI_HOST_CTRL1_REG);
    ctrl1 &= ~SDHCI_ADMA_SEL_MASK;

    if (host->caps.spec_version >= SDHCI_SPEC_VER4_NUM) {
        ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
        ctrl2 |= SDHCI_VER4_ENABLE;

        ctrl2 &= ~SDHCI_ADMA_64BIT_V4;
#ifdef SDHCI_DESC_ADDR_64BITS

        if (host->caps.addr_64bit_v4) {
            ctrl2 |= SDHCI_ADMA_64BIT_V4;
        }

#endif

        ctrl2 |= SDHCI_ADMA2_26BIT_LEN_MODE;
        REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
        /* Select ADMA2 type */
        ctrl1 |= SDHCI_ADMA2_SEL;
    }
    else {
#ifdef SDHCI_DESC_ADDR_64BITS

        if (host->caps.addr_64bit_v3)
            /* Select ADMA2 type, 64bit address */
            ctrl1 |= SDHCI_ADMA_64BIT;
        else
            ctrl1 |= SDHCI_ADMA_32BIT;
#else
        ctrl1 |= SDHCI_ADMA_32BIT;

#endif

        }

    REG_WRITE8(host, ctrl1, SDHCI_HOST_CTRL1_REG);
}

/*
 * Function: sdhci set bus width
 * Arg     : Host & width
 * Return  : 0 on Sucess, 1 on Failure
 * Flow:   : Set the bus width for controller
 */
uint8_t sdhci_set_bus_width(struct sdhci_host *host, uint16_t width)
{
    uint16_t reg = 0;
    reg = REG_READ8(host, SDHCI_HOST_CTRL1_REG);

    switch (width) {
        case MMC_DATA_BUS_WIDTH_8BIT:
            width = SDHCI_BUS_WITDH_8BIT;
            break;

        case MMC_DATA_BUS_WIDTH_4BIT:
            width = SDHCI_BUS_WITDH_4BIT;
            break;

        case MMC_DATA_BUS_WIDTH_1BIT:
            width = SDHCI_BUS_WITDH_1BIT;
            break;

        default:
            ssdk_printf(SSDK_CRIT, "Bus width is invalid: %u\n", width);
            return 1;
    }

    ssdk_printf(SSDK_INFO, "\n %s: bus width:0x%04x\n", __func__, width);
    REG_WRITE8(host, (reg | width), SDHCI_HOST_CTRL1_REG);
    return 0;
}

/*
 * Function: sdhci command err status
 * Arg     : Host structure
 * Return  : 0 on Sucess, 1 on Failure
 * Flow:   : Look for error status
 */
static uint8_t sdhci_cmd_err_status(struct sdhci_host *host)
{
    uint32_t err;
    err = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

    if (err & SDHCI_CMD_TIMEOUT_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: Command timeout error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_CMD, 0);
        return 1;
    }
    else if (err & SDHCI_CMD_CRC_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: Command CRC error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_CMD, 0);
        return 1;
    }
    else if (err & SDHCI_CMD_END_BIT_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: CMD end bit error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_CMD, 0);
        return 1;
    }
    else if (err & SDHCI_CMD_IDX_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: Command Index error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_CMD, 0);
        return 1;
    }
    else if (err & SDHCI_DAT_TIMEOUT_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: DATA time out error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_DAT, 0);
        return 1;
    }
    else if (err & SDHCI_DAT_CRC_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: DATA CRC error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_DAT, 0);
        return 1;
    }
    else if (err & SDHCI_DAT_END_BIT_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: DATA end bit error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_DAT, 0);
        return 1;
    }
    else if (err & SDHCI_CUR_LIM_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: Current limit error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_MISC, 0);
        return 1;
    }
    else if (err & SDHCI_AUTO_CMD12_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: Auto CMD12 error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_MISC, 0);
        return 1;
    }
    else if (err & SDHCI_ADMA_MASK) {
        ssdk_printf(SSDK_CRIT, "Error: ADMA error\n");
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_MISC, 0);
        return 1;
    }

    return 0;
}

/*
 * Function: sdhci cmd abort for error recovery
 * Arg     : Host & command structure
 * Return  : None
 * Flow:   : 1. Send abort command, the cmd type must be abort type
 */
static void sdhci_cmd_abort(struct sdhci_host *host)
{
    struct mmc_command cmd;
    memset(&cmd, 0, sizeof(struct mmc_command));
    /* Send CMD12 for abort transfer*/
    cmd.cmd_index = CMD12_STOP_TRANSMISSION;
    cmd.argument = SD_CARD_RCA << 16;
    cmd.cmd_type = SDHCI_CMD_TYPE_ABORT;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    sdhci_send_command(host, &cmd);
    return;
}

static void sdhci_read_block_pio(struct sdhci_host *host, uint32_t num_blocks)
{
    size_t blksize, len, chunk;
    uint32_t scratch;
    uint8_t *buf;
    struct mmc_command *cmd = &host->data_cmd;

    ssdk_printf(SSDK_DEBUG, "PIO reading\n");

    if (cmd->data.blk_sz) {
        blksize = cmd->data.blk_sz;
    }
    else {
        blksize = SDHCI_MMC_BLK_SZ;
    }

    /* address offset */
    buf = (uint8_t *)cmd->data.data_ptr + (num_blocks * blksize);
    len = blksize;
    chunk = 0;
    ssdk_printf(SSDK_DEBUG, "PIO reading len = %d ptr = %p\n", len, buf);

    while (len) {
        if (chunk == 0) {
            scratch = REG_READ32(host, SDHCI_BUF_DATA);
            chunk = 4;
        }

        *buf = scratch & 0xFF;

        buf++;
        scratch >>= 8;
        chunk--;
        len--;
    }
}

static void sdhci_write_block_pio(struct sdhci_host *host, uint32_t num_blocks)
{
    size_t blksize, len, chunk;
    uint32_t scratch;
    uint8_t *buf;
    struct mmc_command *cmd = &host->data_cmd;

    ssdk_printf(SSDK_DEBUG, "PIO writing\n");

    if (cmd->data.blk_sz) {
        blksize = cmd->data.blk_sz;
    }
    else {
        blksize = SDHCI_MMC_BLK_SZ;
    }

    /* address offset */
    buf = (uint8_t *)cmd->data.data_ptr + (num_blocks * blksize);
    len = blksize;
    chunk = 0;
    scratch = 0;
    ssdk_printf(SSDK_DEBUG, "PIO writing len = %d ptr = %p\n", len, buf);

    while (len) {
        scratch |= (uint32_t) * buf << (chunk * 8);

        buf++;
        chunk++;
        len--;

        if ((chunk == 4) || ((len == 0) && (blksize == 0))) {
            REG_WRITE32(host, scratch, SDHCI_BUF_DATA);
            chunk = 0;
            scratch = 0;
        }
    }

}

static void sdhci_transfer_pio(struct sdhci_host *host)
{
    uint32_t mask;
    uint32_t num_blocks = 0;

    if (host->data_cmd.data.num_blocks == 0)
        return;

    if (host->data_cmd.trans_mode == SDHCI_MMC_READ)
        mask = 0x800;
    else
        mask = 0x400;

    while (host->data_cmd.data.num_blocks) {
        if (REG_READ32(host, SDHCI_PRESENT_STATE_REG) & mask) {
            if (host->data_cmd.trans_mode == SDHCI_MMC_READ)
                sdhci_read_block_pio(host, num_blocks);
            else
                sdhci_write_block_pio(host, num_blocks);

            host->data_cmd.data.num_blocks--;

            num_blocks++;
        }
    }

    ssdk_printf(SSDK_DEBUG, "PIO transfer complete.\n");
}

/*
 * Function: sdhci command complete
 * Arg     : Host & command structure
 * Return  : 0 on Sucess, 1 on Failure
 * Flow:   : 1. Check for command complete
 *           2. Check for transfer complete
 *           3. Get the command response
 *           4. Check for errors & error recovery
 */
static uint8_t sdhci_cmd_complete(struct sdhci_host *host,
                                  struct mmc_command *cmd)
{
    uint8_t i;
    uint8_t ret = 0;
    uint8_t need_reset = 0;
    uint64_t retry = 0;
    uint32_t int_status;
    uint32_t trans_complete = 0;
    uint32_t err_status;
    uint64_t max_trans_retry =
        (cmd->cmd_timeout ? cmd->cmd_timeout : SDHCI_MAX_TRANS_RETRY);

    do {
        int_status = REG_READ16(host, SDHCI_NRML_INT_STS_REG);

        if (int_status & SDHCI_INT_STS_CMD_COMPLETE) {
            break;
        }
        else if (int_status & SDHCI_ERR_INT_STAT_MASK &&
                 !host->tuning_in_progress) {
            goto err;
        }

        /*
         * If Tuning is in progress ignore cmd crc, cmd timeout & cmd end bit
         * errors
         */
        if (host->tuning_in_progress) {
            err_status = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

            if ((err_status & SDHCI_CMD_CRC_MASK) ||
                    (err_status & SDHCI_CMD_END_BIT_MASK) ||
                    err_status & SDHCI_CMD_TIMEOUT_MASK) {
                sdhci_reset(host, (SOFT_RESET_CMD | SOFT_RESET_DATA));
                return 0;
            }

            /*
             * According sdhci v4.2 spec,
             * the tuning cmd is not need wait for cmd complete.
             */
            return 0;
        }

        retry++;
        udelay(1);

        if (retry == SDHCI_MAX_CMD_RETRY) {
            ssdk_printf(SSDK_DEBUG, "Error: Command never completed, reg = 0x%08x\n",
                        int_status);
            sdhci_dumpregs(host);
            ret = 1;
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_TIMEOUT, 0);
            goto err;
        }
    }
    while (1);

    /* Command is complete, clear the interrupt bit */
    REG_WRITE16(host, SDHCI_INT_STS_CMD_COMPLETE, SDHCI_NRML_INT_STS_REG);

    /* Copy the command response,
     * The valid bits for R2 response are 0-119, & but the actual response
     * is stored in bits 8-128. We need to move 8 bits of MSB of each
     * response to register 8 bits of LSB of next response register.
     * As:
     * MSB 8 bits of RESP0 --> LSB 8 bits of RESP1
     * MSB 8 bits of RESP1 --> LSB 8 bits of RESP2
     * MSB 8 bits of RESP2 --> LSB 8 bits of RESP3
     */
    if (cmd->resp_type == SDHCI_CMD_RESP_R2) {
        for (i = 0; i < 4; i++) {
            cmd->resp[i] = REG_READ32(host, SDHCI_RESP_REG + (i * 4));
            cmd->resp[i] <<= SDHCI_RESP_LSHIFT;

            if (i != 0)
                cmd->resp[i] |=
                    (REG_READ32(host, SDHCI_RESP_REG + ((i - 1) * 4)) >>
                     SDHCI_RESP_RSHIFT);
        }
    }
    else {
        cmd->resp[0] = REG_READ32(host, SDHCI_RESP_REG);
    }

    retry = 0;

    /*
     * Clear the transfer complete interrupt
     */
    if (cmd->data_present || cmd->resp_type == SDHCI_CMD_RESP_R1B) {
        do {
            int_status = REG_READ16(host, SDHCI_NRML_INT_STS_REG);

#ifndef SDHCI_TRANS_USE_ADMA

            if (host->cmd->data_present) {
                if (int_status & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL)) {
                    sdhci_transfer_pio(host);
                    REG_WRITE16(host, int_status & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL),
                                SDHCI_NRML_INT_STS_REG);
                }
            }

#endif

            if (int_status & SDHCI_INT_STS_TRANS_COMPLETE) {
                /* here will failed*/
                trans_complete = 1;
                break;
            }
            /*
             * Some controllers set the data timout first on issuing an erase &
             * take time to set data complete interrupt. We need to wait hoping
             * the controller would set data complete
             */
            else if (int_status & SDHCI_ERR_INT_STAT_MASK &&
                     !host->tuning_in_progress &&
                     !((REG_READ16(host, SDHCI_ERR_INT_STS_REG) &
                        SDHCI_DAT_TIMEOUT_MASK))) {
                goto err;
            }

            /*
             * If we are in tuning then we need to wait until Data timeout ,
             * Data end or Data CRC error
             */
            if (host->tuning_in_progress) {
                err_status = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

                if ((err_status & SDHCI_DAT_TIMEOUT_MASK) ||
                        (err_status & SDHCI_DAT_CRC_MASK)) {
                    sdhci_reset(host, (SOFT_RESET_CMD | SOFT_RESET_DATA));
                    return 0;
                }
            }

            retry++;
            udelay(1);

            if (retry == max_trans_retry) {
                ssdk_printf(SSDK_DEBUG, "Error: Transfer never completed\n");
                sdhci_dumpregs(host);
                ret = 1;
                ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_TIMEOUT, 0);
                goto err;
            }
        }
        while (1);

        /* Transfer is complete, clear the interrupt bit */
        REG_WRITE16(host, SDHCI_INT_STS_TRANS_COMPLETE, SDHCI_NRML_INT_STS_REG);
    }

err:
    /* Look for errors */
    int_status = REG_READ16(host, SDHCI_NRML_INT_STS_REG);
    err_status = REG_READ16(host, SDHCI_ERR_INT_STS_REG);

    if (int_status & SDHCI_ERR_INT_STAT_MASK) {
        /* Reset Command & Dat lines on error */
        need_reset = 1;

        /*
         * As per SDHC spec transfer complete has higher priority than data
         * timeout If both transfer complete & data timeout are set then we
         * should ignore data timeout error.
         * --------------------------------------------------------------------
         * | Transfer complete | Data timeout | Meaning of the Status          |
         * |-------------------------------------------------------------------|
         * |      0            |       0      | Interrupted by another factor  |
         * |-------------------------------------------------------------------|
         * |      0            |       1      | Timeout occured during transfer|
         * |-------------------------------------------------------------------|
         * |      1            |  Don't Care  | Command execution complete     |
         *  -------------------------------------------------------------------
         */
        if ((err_status & SDHCI_DAT_TIMEOUT_MASK) && trans_complete) {
            need_reset = 0;
            ret = 0;
            REG_WRITE16(host, err_status, SDHCI_ERR_INT_STS_REG);
        }
        else if (cmd->cmd_type == SDHCI_CMD_TYPE_ABORT) {
            need_reset = 0;
            REG_WRITE16(host, err_status, SDHCI_ERR_INT_STS_REG);
        }
        else if (sdhci_cmd_err_status(host)) {
            ret = 1;
            /* Dump sdhc registers on error */
            sdhci_dumpregs(host);
        }
    }

    /* Reset data & command line */
    if (need_reset) {
        /*
         * Reference synopsys's dwcmsch user guide doc 1.70a:
         * need issue DAT line reset when issuing CMD line reset.
         */
        sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
        REG_WRITE16(host, err_status, SDHCI_ERR_INT_STS_REG);

        /* Send abort cmd only when data present */
        if (cmd->data_present) {
            sdhci_cmd_abort(host);
        }
    }

    return ret;
}

static void _sdhci_adma_write_desc(struct sdhci_host *host, void **desc,
                                   addr_t addr, int len, unsigned int attr)
{
    struct desc_entry *dma_desc = *desc;
#ifdef SDHCI_DESC_ADDR_64BITS
    dma_desc->addr_l = lower_32_bits((addr_t)addr);
    dma_desc->addr_h = upper_32_bits((addr_t)addr);
    dma_desc->tran_att = attr | FV_ADMA2_ATTR_LEN(len);
#else
    dma_desc->addr = lower_32_bits((addr_t)addr);
    dma_desc->tran_att = attr | FV_ADMA2_ATTR_LEN(len);
#endif

    *(struct desc_entry **)desc += 1u;
}

static void sdhci_adma_write_desc(struct sdhci_host *host, void **desc,
                                  addr_t addr, int len, unsigned int attr)
{
    int offset;
    addr_t boundary_addr;

    boundary_addr = CALC_BOUNDARY(addr, ADMA_BOUNDARY_SIZE);

    if ((addr + len) > boundary_addr) {
        offset = boundary_addr - addr;
        _sdhci_adma_write_desc(host, desc, addr, offset, attr);
        addr += offset;
        len -= offset;
    }

    _sdhci_adma_write_desc(host, desc, addr, len, attr);
}

/*
 * Function: sdhci prep desc table
 * Arg     : Pointer data & length
 * Return  : Pointer to desc table
 * Flow:   : Prepare the adma table as per the sd spec v 4.0
 */
static struct desc_entry *sdhci_prep_desc_table(struct sdhci_host *host,
        paddr_t data, uint32_t len)
{
    uint32_t sg_len = 0;
    uint32_t remain = 0;
    uint32_t i;
    uint32_t table_len = 0;
    uint32_t desc_len_max = 0;
    uint32_t tran_len = 0;
    uint32_t adma_attr = 0;
    struct desc_entry *sg_list = NULL;
    struct desc_entry *desc = NULL;

    if (host->caps.addr_64bit_v4) {
        desc_len_max = SDHCI_ADMA2_DESC_LINE_SZ_V4;
    }
    else {
        desc_len_max = SDHCI_ADMA_DESC_LINE_SZ;
    }

    /* Calculate the number of entries in desc table */
    sg_len = len / desc_len_max;
    remain = len - (sg_len * desc_len_max);

    /* Allocate sg_len + 1 entries if there are remaining bytes at the end
        */
    if (remain) {
        sg_len++;
    }

    if (sg_len > (ADMA_DESC_LIST_SIZE - 1)) {
        ssdk_printf(SSDK_CRIT, "Out of ADMA DESC address range\n");
        return NULL;
    }

    /* For workaround the snps adma boundary, alloc one more desc entry */
    table_len = ((sg_len + 1) * sizeof(struct desc_entry));
    sg_list = g_dma_sg_list;

    if (!sg_list) {
        ssdk_printf(SSDK_CRIT, "Error allocating memory\n");
        return NULL;
    }

    memset(sg_list, 0, table_len);
    desc = sg_list;

    for (i = 0; i < sg_len; i++) {
        tran_len = MIN(len, desc_len_max);
        adma_attr = SDHCI_ADMA_DATA_VALID;
        sdhci_adma_write_desc(host, (void **)&desc, data, tran_len, adma_attr);
        data += tran_len;
        len -= tran_len;
    }

    /* Add the last entry - nop, end, valid */
    adma_attr = SDHCI_ADMA_NOP_END_VALID;
    sdhci_adma_write_desc(host, (void **)&desc, 0, 0, adma_attr);

    arch_clean_invalidate_cache_range((addr_t)sg_list,
                                      ROUNDUP(table_len, CONFIG_ARCH_CACHE_LINE));

    for (i = 0; i < sg_len + 1; i++) {
#ifdef SDHCI_DESC_ADDR_64BITS
        ssdk_printf(SSDK_DEBUG,
                    "\n %s: sg_list %p: addr_l: 0x%x addr_h: 0x%x attr: 0x%x\n", __func__,
                    &sg_list[i],
                    sg_list[i].addr_l, sg_list[i].addr_h, sg_list[i].tran_att);
#else
        ssdk_printf(SSDK_DEBUG, "\n %s: sg_list %p: addr: 0x%x attr: 0x%x\n", __func__,
                    &sg_list[i],
                    sg_list[i].addr, sg_list[i].tran_att);
#endif
    }

    return sg_list;
}

/*
 * Function: sdhci adma transfer
 * Arg     : Host structure & command stucture
 * Return  : Pointer to desc table
 * Flow    : 1. Prepare descriptor table
 *           2. Write adma register
 *           3. Write block size & block count register
 */
static struct desc_entry *sdhci_adma_transfer(struct sdhci_host *host,
        struct mmc_command *cmd)
{
    uint32_t num_blks = 0;
    uint32_t sz;
    paddr_t data;
    struct desc_entry *adma_desc;
    paddr_t adma_desc_paddr;
    num_blks = cmd->data.num_blocks;
    data = p2ap((paddr_t)cmd->data.data_ptr);

    /*
     * Some commands send data on DAT lines which is less
     * than SDHCI_MMC_BLK_SZ, in that case trying to read
     * more than the data sent by the card results in data
     * CRC errors. To avoid such errors allow data to pass
     * the required block size, if the block size is not
     * passed use the default value
     */
    if (cmd->data.blk_sz) {
        sz = num_blks * cmd->data.blk_sz;
    }
    else {
        sz = num_blks * SDHCI_MMC_BLK_SZ;
    }

    /* Prepare adma descriptor table */
    adma_desc = sdhci_prep_desc_table(host, data, sz);
    if(NULL == adma_desc) {
        ssdk_printf(SSDK_CRIT, "sdhci_prep_desc_table desc error \n");
        return NULL;
    }
    adma_desc_paddr = p2ap((paddr_t)adma_desc);
    //adma_desc_paddr = 0x10000000 + (paddr_t)adma_desc;
    /* Write adma address to adma register */
    REG_WRITE32(host, (uint32_t)adma_desc_paddr,  SDHCI_ADM_ADDR_REG);
    REG_WRITE32(host, upper_32_bits(adma_desc_paddr), SDHCI_ADM_ADDR_REG + 0x4);
    /* Write the block size */
    if (cmd->data.blk_sz) {
        REG_WRITE16(host, cmd->data.blk_sz, SDHCI_BLKSZ_REG);
    }
    else {
        REG_WRITE16(host, SDHCI_MMC_BLK_SZ, SDHCI_BLKSZ_REG);
    }

    return adma_desc;
}

static void sdhci_pre_xfer(struct sdhci_host *host)
{
    REG_WRITE16(host, 0xFFFF, SDHCI_NRML_INT_STS_REG);
    REG_WRITE16(host, 0xFFFF, SDHCI_ERR_INT_STS_REG);
}

static inline bool sdhci_data_line_cmd(struct mmc_command *cmd)
{
    return cmd->data_present || cmd->resp_type & SDHCI_CMD_RESP_R1B;
}

static inline uint16_t sdhci_cale_timeout(uint64_t us, uint32_t freq)
{
    uint16_t i = 0;
    uint64_t t;

    for (i = 0; i < 0xF; i++) {
        t = 1 << (13 + i);

        if (t / freq * 1000 * 1000 > us) {
            break;
        }
    }

    return MIN(i, 0xE);
}

/*
 * Function: sdhci send command
 * Arg     : Host structure & command stucture
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. Prepare the command register
 *           2. If data is present, prepare adma table
 *           3. Run the command
 *           4. Check for command results & take action
 */
uint32_t sdhci_send_command(struct sdhci_host *host, struct mmc_command *cmd)
{
    uint32_t ret = 0;
    uint8_t retry = 0;
    uint32_t resp_type = 0;
    uint16_t trans_mode = 0;
    uint16_t present_state;
    uint32_t flags;
    uint16_t mask;
    uint8_t timeout;
    ssdk_printf(SSDK_DEBUG, "\n %s: START: cmd:%04d, arg:0x%08x, resp_type:0x%04x, "
                "data_present:%d\n",
                __func__, cmd->cmd_index, cmd->argument, cmd->resp_type,
                cmd->data_present);
    cmd->error = 0;

    if (cmd->data_present) {
        if (!cmd->data.data_ptr)
            return 1;

        if (!IS_ARCH_CACHE_LINE_ALIGNED(cmd->data.data_ptr))
            return 1;
    }

    do {
        mask = SDHCI_STATE_CMD_MASK;

        if (sdhci_data_line_cmd(cmd)) {
            mask |= SDHCI_STATE_DAT_MASK;
        }

        present_state = REG_READ32(host, SDHCI_PRESENT_STATE_REG);
        /* check if CMD & DAT lines are free */
        present_state &= mask;

        if (!present_state) {
            break;
        }

        udelay(1);
        retry++;

        if (retry == 1000) {
            ssdk_printf(SSDK_CRIT, "Error: CMD or DAT lines were never freed\n");
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_STATUS_BUSY, 0);
            return 1;
        }
    }
    while (1);

    host->cmd = cmd;

    sdhci_pre_xfer(host);

    switch (cmd->resp_type) {
        case SDHCI_CMD_RESP_R1:
        case SDHCI_CMD_RESP_R3:
        case SDHCI_CMD_RESP_R6:
        case SDHCI_CMD_RESP_R7:
            /* Response of length 48 have 32 bits
             * of response data stored in RESP0[0:31]
             */
            resp_type = SDHCI_CMD_RESP_48;
            break;

        case SDHCI_CMD_RESP_R2:
            /* Response of length 136 have 120 bits
             * of response data stored in RESP0[0:119]
             */
            resp_type = SDHCI_CMD_RESP_136;
            break;

        case SDHCI_CMD_RESP_R1B:
            /* Response of length 48 have 32 bits
             * of response data stored in RESP0[0:31]
             * & set CARD_BUSY status if card is busy
             */
            resp_type = SDHCI_CMD_RESP_48_BUSY;
            break;

        case SDHCI_CMD_RESP_NONE:
            resp_type = SDHCI_CMD_RESP_NONE;
            break;

        default:
            ssdk_printf(SSDK_CRIT, "Invalid response type for the command\n");
            return 1;
    };

    flags = (resp_type << SDHCI_CMD_RESP_TYPE_SEL_BIT);

    if (cmd->data_present || cmd->cmd_index == CMD21_SEND_TUNING_BLOCK)
        flags |= (1 << SDHCI_CMD_DATA_PRESENT_BIT);

    flags |= (cmd->cmd_type << SDHCI_CMD_CMD_TYPE_BIT);

    /* Enable Command CRC & Index check for commands with response
     * R1, R6, R7 & R1B. Also only CRC check for R2 response
     */
    switch (cmd->resp_type) {
        case SDHCI_CMD_RESP_R1:
        case SDHCI_CMD_RESP_R6:
        case SDHCI_CMD_RESP_R7:
        case SDHCI_CMD_RESP_R1B:
            flags |=
                (1 << SDHCI_CMD_CRC_CHECK_BIT) | (1 << SDHCI_CMD_IDX_CHECK_BIT);
            break;

        case SDHCI_CMD_RESP_R2:
            flags |= (1 << SDHCI_CMD_CRC_CHECK_BIT);
            break;

        default:
            break;
    };

    /* Set the timeout value */
#ifdef SDHCI_SUPPORT_INTERRUPT
    if (cmd->cmd_timeout) {
        timeout = sdhci_cale_timeout(cmd->cmd_timeout, SDHCI_TOUT_TIMER_CLK);
    }
    else {
        timeout = SDHCI_CMD_TIMEOUT;
    }

#else
    timeout = 0xE;
#endif
    REG_WRITE8(host, timeout, SDHCI_TIMEOUT_REG);

    /* Check if data needs to be processed */
    if (cmd->data_present) {
#ifdef SDHCI_TRANS_USE_ADMA
        /*
         * clean any stale cache lines ensure cpu data buffer flush in memory,
         * and forbid cpu write back cache on unpredictable time.
         */
        arch_clean_invalidate_cache_range(
            (addr_t)cmd->data.data_ptr,
            (cmd->data.blk_sz) ? (cmd->data.num_blocks * cmd->data.blk_sz)
            : (cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
        cmd->data.sg_list = sdhci_adma_transfer(host, cmd);
        if(NULL == cmd->data.sg_list)
            return -1;
#else

        /* Write the block size */
        if (cmd->data.blk_sz) {
            REG_WRITE16(host, cmd->data.blk_sz, SDHCI_BLKSZ_REG);
        }
        else {
            REG_WRITE16(host, SDHCI_MMC_BLK_SZ, SDHCI_BLKSZ_REG);
        }

#endif
        memcpy(&host->data_cmd, cmd, sizeof(struct mmc_command));
    }

    /* Write the argument 1 */
    REG_WRITE32(host, cmd->argument, SDHCI_ARGUMENT_REG);

    if (cmd->trans_mode == SDHCI_MMC_READ)
        trans_mode |= SDHCI_READ_MODE;

    /* Set the Transfer mode */
    if (cmd->data_present) {
#ifdef SDHCI_TRANS_USE_ADMA
        /* Enable DMA */
        trans_mode |= SDHCI_DMA_EN;
#endif

        /*
         * Enable auto cmd23 or cmd12 for multi block transfer
         * based on what command card supports
         */
        if (cmd->data.num_blocks > 1) {
            if (cmd->cmd23_support) {
                trans_mode |=
                    SDHCI_TRANS_MULTI | SDHCI_AUTO_CMD23_EN | SDHCI_BLK_CNT_EN;
                REG_WRITE32(host, cmd->data.num_blocks,
                            SDHCI_SDMASA_BLKCNT_REG);
            }
            else
                trans_mode |=
                    SDHCI_TRANS_MULTI | SDHCI_AUTO_CMD12_EN | SDHCI_BLK_CNT_EN;
        }
    }

    /* Write to transfer mode register */
    REG_WRITE16(host, trans_mode, SDHCI_TRANS_MODE_REG);
    /* Write the command register */
    REG_WRITE16(host, SDHCI_PREP_CMD(cmd->cmd_index, flags), SDHCI_CMD_REG);
#ifdef SDHCI_SUPPORT_INTERRUPT

    if (!host->tuning_in_progress)
        osSemaphoreAcquire(host->cmd_sema, osWaitForever);

    ret = host->cmd->error;

    if (ret) {
        sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
        udelay(20);
        ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_CMD, 0);

        if (host->sw_tuning_in_progress)
            goto cmd_out;

        if (cmd->resp_type == SDHCI_CMD_RESP_R1B) {
            goto cmd_out;
        }
    }

    /*
     * If has data and sync mode, or cmd has busy status bit,
     * need wait for data complete signal.
     */
    if ((host->data_cmd.data_present && !host->async_mode) ||
            cmd->resp_type == SDHCI_CMD_RESP_R1B) {

        if (!host->tuning_in_progress){
            osSemaphoreAcquire(host->data_complete_sema, osWaitForever);
        }

        if (host->data_cmd.data_present) {
            ret = host->data_cmd.error;
            host->data_cmd.data_present = 0;
        }
        else {
            ret = host->cmd->error;
        }
    }

#else

    /* Command complete sequence */
    if (sdhci_cmd_complete(host, cmd)) {
        ret = 1;
        goto cmd_out;
    }

#ifdef SDHCI_TRANS_USE_ADMA

    /* Invalidate the cache only for read operations */
    if (cmd->trans_mode == SDHCI_MMC_READ) {
        /* Read can be performed on block size < SDHCI_MMC_BLK_SZ, make sure to
         * flush the data only for the read size instead
         */
        arch_invalidate_cache_range(
            (addr_t)cmd->data.data_ptr,
            (cmd->data.blk_sz) ? (cmd->data.num_blocks * cmd->data.blk_sz)
            : (cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
    }

#endif
#endif
cmd_out:
    host->cmd = NULL;
    ssdk_printf(SSDK_DEBUG,
                "\n %s: END: host:0x%lx cmd:%04d, arg:0x%08x, resp:0x%08x 0x%08x "
                "0x%08x 0x%08x\n",
                __func__, (unsigned long)host->base, cmd->cmd_index, cmd->argument,
                cmd->resp[0],
                cmd->resp[1], cmd->resp[2], cmd->resp[3]);
    return ret;
}

void sdhci_start_tuning(struct sdhci_host *host)
{
    uint16_t ctrl2;
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 |= SDHCI_EXEC_TUNING;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
}

void sdhci_reset_tuning(struct sdhci_host *host)
{
    uint16_t ctrl2;
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 &= ~SDHCI_EXEC_TUNING;
    ctrl2 &= ~SDHCI_SAMPLE_CLK_SEL;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
}

void sdhci_abort_tuning(struct sdhci_host *host)
{
    uint16_t ctrl2;
    ctrl2 = REG_READ16(host, SDHCI_HOST_CTRL2_REG);
    ctrl2 &= ~SDHCI_EXEC_TUNING;
    ctrl2 &= ~SDHCI_SAMPLE_CLK_SEL;
    REG_WRITE16(host, ctrl2, SDHCI_HOST_CTRL2_REG);
    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
}

uint32_t sdhci_tuning_sequence(struct sdhci_host *host, uint32_t opcode,
                               uint32_t bus_width)
{
    uint32_t ret;
    uint32_t reg;
    struct mmc_command cmd;
    memset(&cmd, 0, sizeof(struct mmc_command));
    cmd.cmd_index = opcode;
    cmd.argument = 0x0;
    cmd.cmd_type = SDHCI_CMD_TYPE_NORMAL;
    cmd.resp_type = SDHCI_CMD_RESP_R1;
    cmd.trans_mode = SDHCI_MMC_READ;
    cmd.data_present = 0x0;
    cmd.data.data_ptr = NULL;
    cmd.data.num_blocks = 0x1;

    if (cmd.cmd_index == CMD21_SEND_TUNING_BLOCK &&
            bus_width == MMC_DATA_BUS_WIDTH_8BIT)
        cmd.data.blk_sz = 128;
    else
        cmd.data.blk_sz = 64;

    REG_WRITE16(host, cmd.data.blk_sz, SDHCI_BLKSZ_REG);

    if (host->caps.spec_version >= SDHCI_SPEC_VER4_NUM)
        REG_WRITE32(host, cmd.data.num_blocks, SDHCI_SDMASA_BLKCNT_REG);
    else
        REG_WRITE16(host, cmd.data.num_blocks, SDHCI_BLK_CNT_REG);

    /* Write to transfer mode register */
    REG_WRITE16(host, SDHCI_READ_MODE, SDHCI_TRANS_MODE_REG);

    for (int i = 0; i < MAX_TUNING_LOOP; i++) {
        sdhci_send_command(host, &cmd);
        /* wait for read buf ready, timeout 50ms */
        ret = sdhci_wait_for_bit(host, SDHCI_NRML_INT_STS_REG,
                                 SDHCI_INT_DATA_AVAIL, 0, 50);

        if (ret) {
            ssdk_printf(SSDK_CRIT, "sdhci tuning timeout!\n");
            sdhci_dumpregs(host);
            ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_TUNING, 0);
            return ret;
        }

        reg = REG_READ16(host, SDHCI_HOST_CTRL2_REG);

        if (!(reg & SDHCI_EXEC_TUNING)) {
            if (reg & SDHCI_SAMPLE_CLK_SEL) {
                ssdk_printf(SSDK_INFO, "sdhci: tuning cycle count = %d\n", i);

                if (i < 40)
                    ssdk_printf(SSDK_CRIT, "SDHC: tuning cycle abnormal, num is %d!\n", i);

                return 0;
            }

            break;
        }
    }

    ERRCODE_PRINT(MOD_ID_SYS_SERVICE_STR_MMC, host->slot, MMC_ERR_TUNING, 0);
    return 1;
}

/*
 * Function: sdhci execute tuning
 * Arg     : Host structure & tuning opcode & bus width
 * Return  : 0 on Success, 1 on Failure
 * Flow:   : 1. IF has platform tuning api, execute it
 *           2. start sdhci tuning
 *           3. execute sdhci tuning sequence
 */
uint32_t sdhci_execute_tuning(struct sdhci_host *host, uint32_t opcode,
                              uint32_t bus_width)
{
    uint32_t ret = 0;

    if (host->ops->platform_execute_tuning) {
        return host->ops->platform_execute_tuning(host, opcode, bus_width);
    }

    /* reset the tuning circuit */
    sdhci_reset_tuning(host);

    /* start the tuning circuit */
    sdhci_start_tuning(host);
    host->tuning_in_progress = 1;
    udelay(1);

    if (sdhci_tuning_sequence(host, opcode, bus_width)) {
        ssdk_printf(SSDK_CRIT, "sdhci tuning failed, abort tuning state!\n");
        sdhci_abort_tuning(host);
        ret = 1;
    }

    host->tuning_in_progress = 0;
    return ret;
}

#ifdef SDHCI_SUPPORT_INTERRUPT
static int sdhci_irq_handler(uint32_t irq, void *arg)
{
    uint32_t int_status;
    uint32_t mask;
    struct sdhci_host *host = arg;
    int max_loops = 16;
    struct mmc_command *cmd = host->cmd;
    struct mmc_command *data_cmd = &host->data_cmd;
    spin_lock(&host->spin_lock);
    int_status = REG_READ32(host, SDHCI_NRML_INT_STS_REG);

    if (!int_status) {
        spin_unlock(&host->spin_lock);
        return 0;
    }

    do {
        ssdk_printf(SSDK_DEBUG, "sdhci: irq status 0x%08x\n", int_status);
        /* clear the interrupts */
        mask = int_status &
               (SDHCI_INT_CMD_MASK | SDHCI_INT_DATA_MASK | SDHCI_INT_BUS_POWER);
        REG_WRITE32(host, mask, SDHCI_NRML_INT_STS_REG);

        if (int_status & SDHCI_INT_CMD_MASK) {
            if (int_status &
                    (SDHCI_INT_TIMEOUT | SDHCI_INT_CRC | SDHCI_INT_END_BIT |
                     SDHCI_INT_INDEX | SDHCI_INT_ACMD12ERR)) {
                if (host->sw_tuning_in_progress) {
                    cmd->error = (int_status & SDHCI_INT_CMD_MASK) >> 16;
                }
                else {
                    cmd->error = (int_status & SDHCI_INT_CMD_MASK) >> 16;
                    ssdk_printf(SSDK_CRIT, "sdhci cmd error! reg = 0x%08x\n",
                                int_status);
                    sdhci_dumpregs(host);
                }
            }

            if (cmd->resp_type == SDHCI_CMD_RESP_R2) {
                for (int i = 0; i < 4; i++) {
                    cmd->resp[i] = REG_READ32(host, SDHCI_RESP_REG + (i * 4));
                    cmd->resp[i] <<= SDHCI_RESP_LSHIFT;

                    if (i != 0)
                        cmd->resp[i] |=
                            (REG_READ32(host, SDHCI_RESP_REG + ((i - 1) * 4)) >>
                             SDHCI_RESP_RSHIFT);
                }
            }
            else {
                cmd->resp[0] = REG_READ32(host, SDHCI_RESP_REG);
            }

            osSemaphoreRelease(host->cmd_sema);
        }

        /* clear the data complete flag */
        if (int_status & SDHCI_INT_DATA_MASK) {
            if (data_cmd->data_present) {
                data_cmd->error = (int_status & SDHCI_INT_DATA_MASK) >> 16;

                if (int_status & SDHCI_INT_DATA_END || data_cmd->error) {
                    osSemaphoreRelease(host->data_sema);
                }

                if (int_status & (SDHCI_INT_DATA_AVAIL | SDHCI_INT_SPACE_AVAIL)) {
                    sdhci_transfer_pio(host);
                }
            }
            else {
                cmd->error |= (int_status & SDHCI_INT_DATA_MASK) >> 16;

                if (int_status & SDHCI_INT_DATA_END || cmd->error) {
                    osSemaphoreRelease(host->data_sema);
                }
            }
        }

        int_status = REG_READ32(host, SDHCI_NRML_INT_STS_REG);
    }
    while (int_status & --max_loops);

    spin_unlock(&host->spin_lock);
    return 0;
}

static void sdhci_data_thread(void *arg)
{
    struct sdhci_host *host = arg;
    struct mmc_command *data_cmd;
    struct mmc_device *dev = NULL;

    for (;;) {
        osSemaphoreAcquire(host->data_sema, osWaitForever);
        ssdk_printf(SSDK_DEBUG, "\n %s:sdhci data thread in!\n", __func__);

        if (host->parent) {
            dev = host->parent;
        }

        data_cmd = &host->data_cmd;

        if (!host->data_cmd.data_present) {
            /* If no data cmd, may be RESP_R1B type cmd. */
            if (host->cmd->error) {
                if (host->sw_tuning_in_progress) {
                    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
                }
                else {
                    ssdk_printf(SSDK_CRIT, "sdhci busy error! 0x%08x\n", host->cmd->error);
                    sdhci_dumpregs(host);
                    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
                }
            }

            osSemaphoreRelease(host->data_complete_sema);
            continue;
        }
        else {
            if (data_cmd->error) {
                if (host->sw_tuning_in_progress) {
                    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
                }
                else {
                    ssdk_printf(SSDK_CRIT, "sdhci data error! 0x%08x\n", data_cmd->error);
                    sdhci_dumpregs(host);
                    sdhci_reset(host, SOFT_RESET_CMD | SOFT_RESET_DATA);
                    /* Send abort cmd only when data present */
                    sdhci_cmd_abort(host);
                }

                osSemaphoreRelease(host->data_complete_sema);
                continue;
            }
        }

#ifdef SDHCI_TRANS_USE_ADMA

        if (data_cmd->trans_mode == SDHCI_MMC_READ) {
            /*
             * Read can be performed on block size < SDHCI_MMC_BLK_SZ,
             * make sure to flush the data only for the read size instead.
             */
            arch_invalidate_cache_range(
                (addr_t)data_cmd->data.data_ptr,
                (data_cmd->data.blk_sz)
                ? (data_cmd->data.num_blocks * data_cmd->data.blk_sz)
                : (data_cmd->data.num_blocks * SDHCI_MMC_BLK_SZ));
        }

#endif

        /* Free the scatter/gather list */
        if (data_cmd->data.sg_list) {
            data_cmd->data.sg_list = NULL;
        }

        if (dev && dev->event_handle) {
            data_cmd->data_present = 0;
            dev->opt_result = data_cmd->error;
            dev->event_handle(dev->opt_type, dev->opt_result);
        }
        else {
            ssdk_printf(SSDK_DEBUG, "\n %s:signal complete event!\n", __func__);
            osSemaphoreRelease(host->data_complete_sema);
        }
    }

    return;
}
#endif
/*
 * Function: sdhci init
 * Arg     : Host structure
 * Return  : None
 * Flow:   : 1. Reset the controller
 *           2. Read the capabilities register & populate the host
 *           controller capabilities for use by other functions
 *           3. Enable the power control
 *           4. Set initial bus width
 *           5. Set Adma mode
 *           6. Enable the error status
 */
void sdhci_init(struct sdhci_host *host)
{
    uint32_t caps[2];
    uint16_t spec_version;

    sdhci_reset(host, SDHCI_SOFT_RESET);

    spec_version = REG_READ16(host, SDHCI_SPEC_VERSION_REG);
    host->caps.spec_version = (uint8_t)(spec_version & 0xff);
    /* Read the capabilities register & store the info */
    caps[0] = REG_READ32(host, SDHCI_CAPS_REG1);
    caps[1] = REG_READ32(host, SDHCI_CAPS_REG2);
    ssdk_printf(SSDK_DEBUG,
                "\n %s: Host version: %d capability: cap1:0x%08x, cap2: 0x%08x\n",
                __func__, host->caps.spec_version, caps[0], caps[1]);

    host->caps.base_clk_rate =
        (caps[0] & SDHCI_CLK_RATE_MASK) >> SDHCI_CLK_RATE_BIT;
    host->caps.base_clk_rate *= 1000000;
    /* Get the max block length for mmc */
    host->caps.max_blk_len =
        (caps[0] & SDHCI_BLK_LEN_MASK) >> SDHCI_BLK_LEN_BIT;

    /* 8 bit Bus width */
    if (caps[0] & SDHCI_8BIT_WIDTH_MASK) {
        host->caps.bus_width_8bit = 1;
    }

    /* Adma2 support */
    if (caps[0] & SDHCI_BLK_ADMA_MASK) {
        host->caps.adma2_support = 1;
    }

    /* V4 64bit address support */
    if (caps[0] & SDHCI_CAP_ADDR_64BIT_V4) {
        host->caps.addr_64bit_v4 = 1;
    }

    /* V3 64bit address support */
    if (caps[0] & SDHCI_CAP_ADDR_64BIT_V3) {
        host->caps.addr_64bit_v3 = 1;
    }

    /* Supported voltage, do nothing when it has been set */
    if (host->caps.voltage) {
        ;
    }
    else if (caps[0] & SDHCI_3_3_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_3;
    }
    else if (caps[0] & SDHCI_3_0_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_3_0;
    }
    else if (caps[0] & SDHCI_1_8_VOL_MASK) {
        host->caps.voltage = SDHCI_VOL_1_8;
    }

    /* DDR mode support */
    host->caps.ddr_support = (caps[1] & SDHCI_DDR50_MODE_MASK) ? 1 : 0;
    /* SDR50 mode support */
    host->caps.sdr50_support = (caps[1] & SDHCI_SDR50_MODE_MASK) ? 1 : 0;
    /* SDR104 mode support */
    host->caps.sdr104_support = (caps[1] & SDHCI_SDR104_MODE_MASK) ? 1 : 0;
    sdhci_reset(host, SDHCI_SOFT_RESET);

    /* Set bus power on */
    if (!host->ops->set_power) {
        host->ops->set_power = sdhci_set_bus_power_on;
    }

    host->ops->set_power(host);
    /* Set bus width */
    sdhci_set_bus_width(host, SDHCI_BUS_WITDH_1BIT);
    /* Set Adma mode */
    sdhci_set_adma_mode(host);

    /* Enable error status */
    sdhci_error_status_enable(host);

#ifdef SDHCI_SUPPORT_INTERRUPT
    host->cmd_sema = osSemaphoreNew(1, 0, NULL);
    host->data_sema = osSemaphoreNew(1, 0, NULL);
    host->data_complete_sema = osSemaphoreNew(1, 0, NULL);

    osThreadAttr_t sdhci_data_attr = {0};
    char sdhci_name[128] = {0};
    sprintf(sdhci_name, "sdhci_data%d", host->slot);
    sdhci_data_attr.name = sdhci_name;
    osThreadNew(sdhci_data_thread, host, &sdhci_data_attr);

    ssdk_printf(SSDK_DEBUG, "\n Host irq: %d \n", host->irq);

    if (host->irq) {
        irq_attach(host->irq, sdhci_irq_handler, (void *)host);
        irq_enable(host->irq);
    }

#endif
}
