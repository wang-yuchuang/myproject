/*
 * dw_i2c.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: I2C driver source file.
 *
 * Revision History:
 * --------------------------------------------------------------------
 */
#include <assert.h>
#include <types.h>
#include <board.h>
#include <debug.h>
#include <driver/kunlun/i2c/dw_i2c.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <reg.h>
#include <driver/irq.h>

#define min(a, b) ((a) < (b) ? (a) : (b))

static void dw_i2c_enable(paddr_t base)
{
    writel(1, base + DW_IC_ENABLE);
}

static void dw_i2c_disable_nowait(paddr_t base)
{
    writel(0, base + DW_IC_ENABLE);
}

static void dw_i2c_disable(i2c_adap_dev_t *adap)
{
    int timeout = 100;
    uint32_t status;

    do {
        dw_i2c_disable_nowait(adap->base);
        /*
         * The enable status register may be unimplemented, but
         * in that case this test reads zero and exits the loop.
         */
        status = readl(adap->base + DW_IC_ENABLE_STATUS);

        if ((status & 1) == 0)
            return;

        /*
         * Wait 10 times the signaling period of the highest I2C
         * transfer supported by the driver (for 400KHz this is
         * 25us) as described in the DesignWare I2C databook.
         */
        udelay(100);
    }
    while (timeout--);

    ssdk_printf(SSDK_WARNING, "i2c%d-adap, timeout disabling adapter\n", adap->id);
}

static void dw_i2c_disable_int(paddr_t base)
{
    writel(0, base + DW_IC_INTR_MASK);
}

static int dw_i2c_get_fsm_stat(paddr_t base)
{
    uint32_t stat, enabled;

    enabled = readl(base + DW_IC_ENABLE);
    stat = readl(base + DW_IC_RAW_INTR_STAT);
    ssdk_printf(SSDK_DEBUG, "enabled=%#x stat=%#x\n", enabled, stat);

    if (!enabled || !(stat & ~DW_IC_INTR_ACTIVITY))
        return 0;

    return 1;
}

static uint32_t dw_i2c_read_clear_intrbits(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    uint32_t stat;

    /*
     * The IC_INTR_STAT register just indicates "enabled" interrupts.
     * The unmasked raw version of interrupt status bits is available
     * in the IC_RAW_INTR_STAT register.
     *
     * That is,
     *   stat = readl(IC_INTR_STAT);
     * equals to,
     *   stat = readl(IC_RAW_INTR_STAT) & readl(IC_INTR_MASK);
     *
     * The raw version might be useful for debugging purposes.
     */
    stat = readl(adap->base + DW_IC_INTR_STAT);

    /*
     * Do not use the IC_CLR_INTR register to clear interrupts, or
     * you'll miss some interrupts, triggered during the period from
     * readl(IC_INTR_STAT) to readl(IC_CLR_INTR).
     *
     * Instead, use the separately-prepared IC_CLR_* registers.
     */
    if (stat & DW_IC_INTR_RX_UNDER)
        readl(adap->base + DW_IC_CLR_RX_UNDER);

    if (stat & DW_IC_INTR_RX_OVER)
        readl(adap->base + DW_IC_CLR_RX_OVER);

    if (stat & DW_IC_INTR_TX_OVER)
        readl(adap->base + DW_IC_CLR_TX_OVER);

    if (stat & DW_IC_INTR_RD_REQ)
        readl(adap->base + DW_IC_CLR_RD_REQ);

    if (stat & DW_IC_INTR_TX_ABRT) {
        /*
         * The IC_TX_ABRT_SOURCE register is cleared whenever
         * the IC_CLR_TX_ABRT is read.  Preserve it beforehand.
         */
        cxt->abort_source = readl(adap->base + DW_IC_TX_ABRT_SOURCE);
        readl(adap->base + DW_IC_CLR_TX_ABRT);
    }

    if (stat & DW_IC_INTR_RX_DONE)
        readl(adap->base + DW_IC_CLR_RX_DONE);

    if (stat & DW_IC_INTR_ACTIVITY)
        readl(adap->base + DW_IC_CLR_ACTIVITY);

    if (stat & DW_IC_INTR_STOP_DET)
        readl(adap->base + DW_IC_CLR_STOP_DET);

    if (stat & DW_IC_INTR_START_DET)
        readl(adap->base + DW_IC_CLR_START_DET);

    if (stat & DW_IC_INTR_GEN_CALL)
        readl(adap->base + DW_IC_CLR_GEN_CALL);

    if (stat & DW_IC_INTR_SCL_STUCK)
        readl(adap->base + DW_IC_CLR_SCL_STUCK_DET);

    return stat;
}

static void dw_i2c_read(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    struct i2c_msg *msgs = cxt->msgs;
    unsigned int rx_valid;

    for (; cxt->msg_read_idx < cxt->msgs_num; cxt->msg_read_idx++) {
        uint32_t len;
        uint8_t *buf;

        if (!(msgs[cxt->msg_read_idx].rw_flag & I2C_M_R))
            continue;

        if (!(cxt->status & DW_STATUS_R_IN_PROGRESS)) {
            len = msgs[cxt->msg_read_idx].len;
            buf = msgs[cxt->msg_read_idx].buf;
        }
        else {
            len = cxt->rx_buf_len;
            buf = cxt->rx_buf;
        }

        rx_valid = readl(adap->base + DW_IC_RXFLR);

        for (; len > 0 && rx_valid > 0; len--, rx_valid--) {
            *buf++ = readl(adap->base + DW_IC_DATA_CMD);
            cxt->rx_outstanding--;
        }

        if (len > 0) {
            cxt->status |= DW_STATUS_R_IN_PROGRESS;
            cxt->rx_buf_len = len;
            cxt->rx_buf = buf;
            return;
        }
        else
            cxt->status &= ~DW_STATUS_R_IN_PROGRESS;
    }
}

/*
 * Initiate (and continue) low level master read/write transaction.
 * This function is only called from i2c_dw_isr, and pumping i2c_msg
 * messages into the tx buffer.  Even if the size of i2c_msg data is
 * longer than the size of the tx buffer, it handles everything.
 */
static void dw_i2c_xfer_msg(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    struct i2c_msg *msgs = cxt->msgs;
    uint32_t intr_mask;
    int tx_limit, rx_limit;
    uint32_t addr = msgs[cxt->msg_write_idx].addr;
    uint32_t buf_len = cxt->tx_buf_len;
    uint8_t *buf = cxt->tx_buf;
    bool need_restart = false;
    unsigned int flr;

    intr_mask = DW_IC_INTR_MASTER_MASK;

    for (; cxt->msg_write_idx < cxt->msgs_num; cxt->msg_write_idx++) {
        /*
         * If target address has changed, we need to
         * reprogram the target address in the I2C
         * adapter when we are done with this transfer.
         */
        if (msgs[cxt->msg_write_idx].addr != addr) {
            ssdk_printf(SSDK_ERR, "%s: invalid target addr\n", __func__);
            cxt->msg_err = -DW_I2C_EMSG;
            break;
        }

        if (!(cxt->status & DW_STATUS_W_IN_PROGRESS)) {
            /* new i2c_msg */
            buf = msgs[cxt->msg_write_idx].buf;
            buf_len = msgs[cxt->msg_write_idx].len;

            /* If both IC_EMPTYFIFO_HOLD_MASTER_EN and
             * IC_RESTART_EN are set, we must manually
             * set restart bit between messages.
             */
            if ((cxt->master_cfg & DW_IC_CON_RESTART_EN) &&
                    (cxt->msg_write_idx > 0))
                need_restart = true;
        }

        flr = readl(adap->base + DW_IC_TXFLR);
        tx_limit = cxt->tx_fifo_depth - flr;

        flr = readl(adap->base + DW_IC_RXFLR);
        rx_limit = cxt->rx_fifo_depth - flr;

        while (buf_len > 0 && tx_limit > 0 && rx_limit > 0) {
            uint32_t cmd = 0;
            /*
             * If IC_EMPTYFIFO_HOLD_MASTER_EN is set we must
             * manually set the stop bit. However, it cannot be
             * detected from the registers so we set it always
             * when writing/reading the last byte.
             */

            /*
             * i2c-core always sets the buffer length of
             * I2C_FUNC_SMBUS_BLOCK_DATA to 1. The length will
             * be adjusted when receiving the first byte.
             * Thus we can't stop the transaction here.
             */
            if (cxt->msg_write_idx == cxt->msgs_num - 1 && buf_len == 1)
                cmd |= 0x1 << 9;

            if (need_restart) {
                cmd |= 0x1 << 10;
                need_restart = false;
            }

            if (msgs[cxt->msg_write_idx].rw_flag & I2C_M_R) {
                /* Avoid rx buffer overrun */
                if (cxt->rx_outstanding >= cxt->rx_fifo_depth)
                    break;

                writel(cmd | 0x100, adap->base + DW_IC_DATA_CMD);
                rx_limit--;
                cxt->rx_outstanding++;
            }
            else {
                writel(cmd | *buf++, adap->base + DW_IC_DATA_CMD);
            }

            tx_limit--;
            buf_len--;
        }

        cxt->tx_buf = buf;
        cxt->tx_buf_len = buf_len;

        /*
         * Because we don't know the buffer length in the
         * I2C_FUNC_SMBUS_BLOCK_DATA case, we can't stop
         * the transaction here.
         */
        if (buf_len > 0) {
            /* more bytes to be written */
            cxt->status |= DW_STATUS_W_IN_PROGRESS;
            break;
        }
        else
            cxt->status &= ~DW_STATUS_W_IN_PROGRESS;
    }

    /*
     * If i2c_msg index search is completed, we don't need TX_EMPTY
     * interrupt any more.
     */
    if (cxt->msg_write_idx == cxt->msgs_num)
        intr_mask &= ~DW_IC_INTR_TX_EMPTY;

    if (cxt->msg_err)
        intr_mask = 0;

    writel(intr_mask, adap->base + DW_IC_INTR_MASK);
}

static int dw_i2c_irq_handler(uint32_t irq, void *arg)
{
    i2c_adap_dev_t *adap = arg;
    dw_i2c_context *cxt = adap->priv;
    uint32_t stat;

    if (!dw_i2c_get_fsm_stat(adap->base))
        return 0;

    stat = dw_i2c_read_clear_intrbits(adap);

    if (stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_SCL_STUCK)) {
        if (stat & DW_IC_INTR_TX_ABRT)
            cxt->cmd_err |= DW_IC_ERR_TX_ABRT;

        if (stat & DW_IC_INTR_SCL_STUCK)
            cxt->cmd_err |= DW_IC_ERR_SCL_STUCK;

        cxt->status = DW_STATUS_IDLE;

        /*
         * Anytime TX_ABRT is set, the contents of the tx/rx
         * buffers are flushed. Make sure to skip them.
         */
        writel(0, adap->base + DW_IC_INTR_MASK);
        goto tx_aborted;
    }

    if (stat & DW_IC_INTR_RX_FULL)
        dw_i2c_read(adap);

    if (stat & DW_IC_INTR_TX_EMPTY)
        dw_i2c_xfer_msg(adap);

    /*
     * No need to modify or disable the interrupt mask here.
     * i2c_dw_xfer_msg() will take care of it according to
     * the current transmit status.
     */

tx_aborted:

    if ((stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_SCL_STUCK | DW_IC_INTR_STOP_DET))
            || cxt->msg_err)
        osEventFlagsSet(cxt->event, DW_I2C_EVENT_COMPLETE);

    return 0;
}

static int dw_i2c_poll_write(i2c_adap_dev_t *adap, uint8_t *txbuf,
                             uint16_t txlen, bool need_restart, bool need_stop)
{
    dw_i2c_context *cxt = adap->priv;
    uint32_t cmd, stat;
    int flag = 0;
    int cnt = 0;

    while (txlen) {
        stat = dw_i2c_read_clear_intrbits(adap);

        if (stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_SCL_STUCK)) {
            if (stat & DW_IC_INTR_TX_ABRT)
                cxt->cmd_err |= DW_IC_ERR_TX_ABRT;

            if (stat & DW_IC_INTR_SCL_STUCK)
                cxt->cmd_err |= DW_IC_ERR_SCL_STUCK;

            return -1;
        }

        stat = readl(adap->base + DW_IC_STATUS);

        if (stat & 0x2) {
            cnt = 0;
            cmd = 0;

            if (need_stop && txlen == 1)
                cmd |= 0x1 << 9;

            if (need_restart) {
                cmd |= 0x1 << 10;
                need_restart = false;
            }

            writel(cmd | *txbuf++, adap->base + DW_IC_DATA_CMD);
            txlen--;
        }
        else {
            udelay(5);
            if (cnt++ > 10000)
                return -DW_I2C_ETIMEOUT;
        }
    }

    cnt = 0;
    while (1) {
        stat = dw_i2c_read_clear_intrbits(adap);

        if (stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_SCL_STUCK)) {
            if (stat & DW_IC_INTR_TX_ABRT)
                cxt->cmd_err |= DW_IC_ERR_TX_ABRT;

            if (stat & DW_IC_INTR_SCL_STUCK)
                cxt->cmd_err |= DW_IC_ERR_SCL_STUCK;

            return -1;
        }
        else if (need_stop && (stat & DW_IC_INTR_STOP_DET)) {
            return 0;
        }
        else if (readl(adap->base + DW_IC_STATUS) & 0x4) {
            if (!flag) {
                flag = 1;
                continue;
            }

            return 0;
        }
        else {
            udelay(5);
            if (cnt++ > 10000)
                return -DW_I2C_ETIMEOUT;
        }
    }

    return 0;
}

static int dw_i2c_poll_read(i2c_adap_dev_t *adap, uint8_t *rxbuf,
                            uint16_t rxlen, bool need_restart, bool need_stop)
{
    dw_i2c_context *cxt = adap->priv;
    uint16_t txlen = rxlen;
    uint32_t cmd, stat, rx_valid;
    int cnt = 0;

    while (1) {
        stat = dw_i2c_read_clear_intrbits(adap);

        if (stat & (DW_IC_INTR_TX_ABRT | DW_IC_INTR_SCL_STUCK)) {
            if (stat & DW_IC_INTR_TX_ABRT)
                cxt->cmd_err |= DW_IC_ERR_TX_ABRT;

            if (stat & DW_IC_INTR_SCL_STUCK)
                cxt->cmd_err |= DW_IC_ERR_SCL_STUCK;

            return -1;
        }
        else if (!rxlen) {
            return 0;
        }

        stat = readl(adap->base + DW_IC_STATUS);

        if ((stat & 0x2) && (cxt->rx_outstanding < rxlen)
                && (cxt->rx_outstanding < cxt->rx_fifo_depth)) {
            cmd = 0;

            if (need_stop && txlen == 1)
                cmd |= 0x1 << 9;

            if (need_restart) {
                cmd |= 0x1 << 10;
                need_restart = false;
            }

            writel(cmd | 0x100, adap->base + DW_IC_DATA_CMD);
            cxt->rx_outstanding++;
            txlen--;
        }

        rx_valid = readl(adap->base + DW_IC_RXFLR);

        if (rx_valid > 0) {
            cnt = 0;
            for (; rxlen > 0 && rx_valid > 0; rxlen--, rx_valid--) {
                *rxbuf++ = readl(adap->base + DW_IC_DATA_CMD);
                cxt->rx_outstanding--;
            }
        }
        else {
            udelay(5);
            if (cnt++ > 10000)
                return -DW_I2C_ETIMEOUT;
        }
    }

    return 0;
}

static int dw_i2c_poll_handler(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    bool need_restart = false;
    bool need_stop = false;
    int ret, cnt = 100;

    while (cnt--) {
        if (dw_i2c_get_fsm_stat(adap->base))
            break;

        udelay(10);
    }

    if (cnt <= 0)
        return -1;

    for (int i = 0; i < cxt->msgs_num; i++) {
        if (i > 0) {
            need_restart = true;

            if (cxt->msgs[i - 1].addr != cxt->msgs[i].addr) {
                ssdk_printf(SSDK_ERR, "%s: invalid target addr\n", __func__);
                cxt->msg_err = -EINVAL;
                return -1;
            }
        }

        if (i == cxt->msgs_num - 1)
            need_stop = true;

        if (cxt->msgs[i].rw_flag == I2C_M_R)
            ret = dw_i2c_poll_read(adap, cxt->msgs[i].buf, cxt->msgs[i].len,
                                   need_restart, need_stop);
        else
            ret = dw_i2c_poll_write(adap, cxt->msgs[i].buf, cxt->msgs[i].len,
                                    need_restart, need_stop);

        if (ret < 0)
            break;
    }

    return ret;
}

static int dw_i2c_wait_bus_not_busy(i2c_adap_dev_t *adap)
{
    uint32_t status;
    int ret = 0, cnt = 100;

    while (cnt--) {
        status = readl(adap->base + DW_IC_STATUS);

        if (!(status & DW_IC_STATUS_ACTIVITY))
            break;

        udelay(200);
    }

    if (cnt <= 0)
        ret = -1;

    return ret;
}

static void dw_i2c_xfer_init(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    struct i2c_msg *msgs = cxt->msgs;
    uint32_t master_cfg = cxt->master_cfg;

    /* Disable the adapter */
    dw_i2c_disable(adap);

    /* If the slave address is ten bit address, enable 10BITADDR */
    if (msgs[0].addr_flag & I2C_ADDR10B)
        master_cfg |= DW_IC_CON_10BITADDR_MASTER;

    writel(master_cfg, adap->base + DW_IC_CON);

    /* Set the slave (target) address */
    writel(msgs[0].addr, adap->base + DW_IC_TAR);

    /* Enforce disabled interrupts (due to HW issues) */
    dw_i2c_disable_int(adap->base);

    /* Enable the adapter */
    dw_i2c_enable(adap->base);

    /* Clear and enable interrupts */
    readl(adap->base + DW_IC_CLR_INTR);
    writel(DW_IC_INTR_MASTER_MASK, adap->base + DW_IC_INTR_MASK);
}

static void dw_i2c_sda_stuck_recover(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    int cnt = 0;

    writel(0x9, adap->base + DW_IC_ENABLE);

    while (cnt++ < 10) {
        udelay(20);

        if (!(readl(adap->base + DW_IC_ENABLE) & 0x8))
            break;
    }

    if (readl(adap->base + DW_IC_STATUS) & 0x800)
        cxt->sda_stuck_err = 1;
}

static int dw_i2c_xfer(i2c_adap_dev_t *adap, struct i2c_msg *msgs, int num)
{
    dw_i2c_context *cxt = adap->priv;
    int ret = 0;
    uint32_t flags;

    if (!adap || !msgs || (num == 0)) {
        ssdk_printf(SSDK_ERR, "i2c%d-adap, param err %p,%p,%d\n",
                    adap->id, adap, msgs, num);
        ret = -DW_I2C_EPARAM;
        goto done;
    }

    if (cxt->scl_stuck_err) {
        ssdk_printf(SSDK_ERR, "i2c%d-adap, scl stuck\n", adap->id);
        ret = -DW_I2C_ESCLSTUCK;
        goto done;
    }

    if (cxt->sda_stuck_err) {
        ssdk_printf(SSDK_ERR, "i2c%d-adap, sda stuck\n", adap->id);
        ret = -DW_I2C_ESDASTUCK;
        goto done;
    }

    ret = dw_i2c_wait_bus_not_busy(adap);
    if (ret < 0) {
        ret = -DW_I2C_EBUSY;
        goto done;
    }

    cxt->msgs = msgs;
    cxt->msgs_num = num;
    cxt->cmd_err = 0;
    cxt->msg_write_idx = 0;
    cxt->msg_read_idx = 0;
    cxt->msg_err = 0;
    cxt->status = DW_STATUS_IDLE;
    cxt->abort_source = 0;
    cxt->rx_outstanding = 0;

    dw_i2c_xfer_init(adap);

    if (arch_in_irq_mode() || arch_irq_is_masked() || (osKernelGetState() < osKernelRunning)
            || msgs[0].polling)
        ret = dw_i2c_poll_handler(adap);
    else {
        irq_enable(adap->irq);
        flags = osEventFlagsWait(cxt->event, DW_I2C_EVENT_COMPLETE, osFlagsWaitAll,
                                 DW_I2C_EVENT_TIMEOUT);

        if (osErrorTimeout == flags)
            ret = -DW_I2C_ETIMEOUT;

        irq_disable(adap->irq);
    }

    if (ret == (-DW_I2C_ETIMEOUT)) {
        dw_i2c_disable(adap);
        goto done;
    }

    /*
     * We must disable the adapter before returning and signaling the end
     * of the current transfer. Otherwise the hardware might continue
     * generating interrupts which in turn causes a race condition with
     * the following transfer.  Needs some more investigation if the
     * additional interrupts are a hardware bug or this driver doesn't
     * handle them correctly yet.
     */
    dw_i2c_disable_nowait(adap->base);

    if (cxt->msg_err) {
        ret = cxt->msg_err;
        goto done;
    }

    /* No error */
    if (likely(!cxt->cmd_err && !cxt->status)) {
        ret = 0;
        goto done;
    }

    /* We have an error */
    if (cxt->cmd_err & DW_IC_ERR_SCL_STUCK) {
        cxt->scl_stuck_err = 1;
        ret = -DW_I2C_ESCLSTUCK;
        goto done;
    }

    if (cxt->cmd_err & DW_IC_ERR_TX_ABRT) {
        if (cxt->abort_source & DW_IC_TX_ABRT_NOACK)
            ret = -DW_I2C_ENOACK;
        else if (cxt->abort_source & DW_IC_TX_ARB_LOST)
            ret = -DW_I2C_EARBLOST;
        else if (cxt->abort_source & DW_IC_TX_ABRT_GCALL_READ)
            ret = -DW_I2C_EINVAL;
        else if (cxt->abort_source & DW_IC_ABRT_SDA_STUCK_AT_LOW) {
            ret = -DW_I2C_ESDASTUCK;
            dw_i2c_sda_stuck_recover(adap);
        }
        else
            ret = -DW_I2C_EIO;

        goto done;
    }

    if (cxt->status)
        ssdk_printf(SSDK_ERR,
                    "i2c%d-adap, trans terminated early,int latency too high?\n", adap->id);

    ret = -DW_I2C_EIO;

done:
    return ret;
}

static  uint32_t dw_i2c_scl_hcnt(uint32_t ic_clk, uint32_t tSYMBOL, uint32_t tf,
                                 int cond, int offset)
{
    /*
     * DesignWare I2C core doesn't seem to have solid strategy to meet
     * the tHD;STA timing spec.  Configuring _HCNT based on tHIGH spec
     * will result in violation of the tHD;STA spec.
     */
    if (cond)
        /*
         * Conditional expression:
         *
         *   IC_[FS]S_SCL_HCNT + (1+4+3) >= IC_CLK * tHIGH
         *
         * This is based on the DW manuals, and represents an ideal
         * configuration.  The resulting I2C bus speed will be
         * faster than any of the others.
         *
         * If your hardware is free from tHD;STA issue, try this one.
         */
        return (ic_clk * tSYMBOL + 500000) / 1000000 - 8 + offset;
    else
        /*
         * Conditional expression:
         *
         *   IC_[FS]S_SCL_HCNT + 3 >= IC_CLK * (tHD;STA + tf)
         *
         * This is just experimental rule; the tHD;STA period turned
         * out to be proportinal to (_HCNT + 3).  With this setting,
         * we could meet both tHIGH and tHD;STA timing specs.
         *
         * If unsure, you'd better to take this alternative.
         *
         * The reason why we need to take into account "tf" here,
         * is the same as described in i2c_dw_scl_lcnt().
         */
        return (ic_clk * (tSYMBOL + tf) + 500000) / 1000000 - 3 + offset;
}

static uint32_t dw_i2c_scl_lcnt(uint32_t ic_clk, uint32_t tLOW, uint32_t tf,
                                int offset)
{
    /*
     * Conditional expression:
     *
     *   IC_[FS]S_SCL_LCNT + 1 >= IC_CLK * (tLOW + tf)
     *
     * DW I2C core starts counting the SCL CNTs for the LOW period
     * of the SCL clock (tLOW) as soon as it pulls the SCL line.
     * In order to meet the tLOW timing spec, we need to take into
     * account the fall time of SCL signal (tf).  Default tf value
     * should be 0.3 us, for safety.
     */
    return ((ic_clk * (tLOW + tf) + 500000) / 1000000) - 1 + offset;
}

static int dw_i2c_set_sda_hold(i2c_adap_dev_t *adap)
{
    int tmp;
    int lcnt;
    dw_i2c_context *cxt = adap->priv;

    /* Keep previous hold time setting if no one set it */
    //cxt->sda_hold_time = readl(adap->base + DW_IC_SDA_HOLD);

    lcnt = adap->speed == I2C_SSPEED ? cxt->ss_lcnt : cxt->fs_lcnt;
    tmp = min(lcnt / 2, (lcnt - 2));
    cxt->sda_hold_time = (tmp << DW_IC_SDA_HOLD_RX_SHIFT) + tmp;

    /*
     * Workaround for avoiding TX arbitration lost in case I2C
     * slave pulls SDA down "too quickly" after falling edge of
     * SCL by enabling non-zero SDA RX hold. Specification says it
     * extends incoming SDA low to high transition while SCL is
     * high but it appears to help also above issue.
     */
    if (!(cxt->sda_hold_time & 0xff0000))
        cxt->sda_hold_time |= 1 << DW_IC_SDA_HOLD_RX_SHIFT;

    writel(cxt->sda_hold_time, adap->base + DW_IC_SDA_HOLD);

    ssdk_printf(SSDK_INFO, "i2c%d-adap, SDA Hold Time %#x\n", adap->id,
                cxt->sda_hold_time);
    return 0;
}

static int dw_i2c_set_timeout(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    cxt->scl_timeout = adap->clk * DW_I2C_STUCK_TIMEOUT;
    cxt->sda_timeout = adap->clk * DW_I2C_STUCK_TIMEOUT;
    writel(cxt->scl_timeout, adap->base + DW_IC_SCL_STUCK_TIMEOUT);
    writel(cxt->sda_timeout, adap->base + DW_IC_SDA_STUCK_TIMEOUT);

    return 0;
}

static int dw_i2c_set_timings(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    uint32_t sda_falling_time, scl_falling_time;
    uint32_t ic_clk_khz;

    /* Set standard and fast speed dividers for high/low periods */
    sda_falling_time = 300; /* ns */
    scl_falling_time = 300; /* ns */
    ic_clk_khz = adap->clk / 1000;

    if (adap->speed == I2C_SSPEED) {
        cxt->ss_hcnt = dw_i2c_scl_hcnt(ic_clk_khz,
                                       4000, /* tHD;STA = tHIGH = 4.0 us */
                                       sda_falling_time,
                                       0,    /* 0: DW default, 1: Ideal */
                                       0);   /* No offset */
        cxt->ss_lcnt = dw_i2c_scl_lcnt(ic_clk_khz,
                                       4700, /* tLOW = 4.7 us */
                                       scl_falling_time,
                                       0);   /* No offset */
        ssdk_printf(SSDK_INFO, "i2c%d-adap, Standard Mode HCNT:LCNT = %d:%d\n",
                    adap->id, cxt->ss_hcnt, cxt->ss_lcnt);
    }
    else if (adap->speed == I2C_FSPEED) {
        cxt->fs_hcnt = dw_i2c_scl_hcnt(ic_clk_khz, 600, sda_falling_time, 0, 0);
        cxt->fs_lcnt = dw_i2c_scl_lcnt(ic_clk_khz, 1300, scl_falling_time, 0);

        ssdk_printf(SSDK_INFO, "i2c%d-adap, Fast Mode HCNT:LCNT = %d:%d\n",
                    adap->id, cxt->fs_hcnt, cxt->fs_lcnt);
    }
    else if (adap->speed == I2C_PSPEED) {
        cxt->fs_hcnt = dw_i2c_scl_hcnt(ic_clk_khz, 260, sda_falling_time, 0, 0);
        cxt->fs_lcnt = dw_i2c_scl_lcnt(ic_clk_khz, 500, scl_falling_time, 0);

        ssdk_printf(SSDK_INFO, "i2c%d-adap, Fast Mode Plus HCNT:LCNT = %d:%d\n",
                    adap->id, cxt->fs_hcnt, cxt->fs_lcnt);
    }
    else {
        ssdk_printf(SSDK_ERR, "i2c%d-adap, unsupport i2c speed mode error\n",
                    adap->id);
        return -1;
    }

    if (adap->speed == I2C_SSPEED) {
        writel(cxt->ss_hcnt, adap->base + DW_IC_SS_SCL_HCNT);
        writel(cxt->ss_lcnt, adap->base + DW_IC_SS_SCL_LCNT);
    }
    else {
        writel(cxt->fs_hcnt, adap->base + DW_IC_FS_SCL_HCNT);
        writel(cxt->fs_lcnt, adap->base + DW_IC_FS_SCL_LCNT);
    }

    dw_i2c_set_sda_hold(adap);
    dw_i2c_set_timeout(adap);
    return 0;
}

static void dw_i2c_set_fifo_size(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;

    /*
     * Try to detect the FIFO depth if not set by interface driver,
     * the depth could be from 2 to 256 from HW spec.
     */
    uint32_t param = readl(adap->base + DW_IC_COMP_PARAM_1);
    cxt->tx_fifo_depth = ((param >> 16) & 0xff) + 1;
    cxt->rx_fifo_depth = ((param >> 8)  & 0xff) + 1;

    /* Configure Tx/Rx FIFO threshold levels */
    writel(cxt->tx_fifo_depth / 2, adap->base + DW_IC_TX_TL);
    writel(0, adap->base + DW_IC_RX_TL);
}

static void dw_i2c_cfg_master(i2c_adap_dev_t *adap)
{
    dw_i2c_context *cxt = adap->priv;
    cxt->master_cfg = DW_IC_CON_MASTER | DW_IC_CON_SLAVE_DISABLE |
                      DW_IC_CON_RESTART_EN | DW_IC_CON_BUS_CLEAR_FEATURE_CTRL;

    if (adap->speed == I2C_SSPEED)
        cxt->master_cfg |= DW_IC_CON_SPEED_STD;
    else
        cxt->master_cfg |= DW_IC_CON_SPEED_FAST;

    writel(cxt->master_cfg, adap->base + DW_IC_CON);
}

static int dw_i2c_master_init(i2c_adap_dev_t *adap)
{
    int ret;
    dw_i2c_context *cxt = adap->priv;

    dw_i2c_disable(adap);
    dw_i2c_disable_int(adap->base);

    ret = dw_i2c_set_timings(adap);

    if (ret)
        return ret;

    dw_i2c_set_fifo_size(adap);
    dw_i2c_cfg_master(adap);

    cxt->event = osEventFlagsNew(NULL);
    irq_attach(adap->irq, dw_i2c_irq_handler, adap);

    return 0;
}

static int dw_i2c_init(i2c_adap_dev_t *adap)
{
    int ret = 0;

    if (!adap)
        return -1;

    switch (adap->opmode) {
        case I2C_MASTER:
            ret = dw_i2c_master_init(adap);
            break;

        default:
            ret = -1;
            ssdk_printf(SSDK_ERR, "i2c%d-adap, invaild opmode\n", adap->id);
            break;
    };

    return ret;
}

static i2c_adap_ops_t dw_i2c_adap_ops = {
    .i2c_init = dw_i2c_init,
    .i2c_xfer = dw_i2c_xfer,
};

void dw_i2c_adap_probe(i2c_adap_dev_t *adap, i2c_adap_cfg_t *cfg,
                       dw_i2c_context *i2c_cxt, int id)
{
    if (!adap || !cfg || !i2c_cxt)
        return;

    adap->id = id;
    adap->ops = &dw_i2c_adap_ops;
    adap->retry = DW_I2C_RETRY;
    adap->priv = i2c_cxt;

    i2c_adap_earlyinit(adap, cfg);
}
