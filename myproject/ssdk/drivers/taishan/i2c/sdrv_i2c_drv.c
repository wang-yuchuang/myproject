/**
 * @file sdrv_i2c_drv.c
 * @brief sdrv i2c driver interface file.
 *
 * @Copyright (c) 2020  Semidrive Semiconductor.
 * @All rights reserved.
 **/

#include <stdio.h>
#include <types.h>
#include <errno.h>
#include <debug.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <driver/irq.h>
#include <math.h>
#include <board.h>

#include "driver/taishan/i2c/sdrv_i2c_drv.h"
#include "sdrv_i2c.h"
#include "CLI_type.h"

static void sdrv_i2c_write(i2c_adap_dev_t *adap, uint8_t *buf,
                           uint16_t wlen, uint16_t *cur_len)
{
    uint8_t *tbuf = buf + *cur_len;
    int flen = sdrv_i2c_lld_get_wspace(adap->base);
    int len = min(flen, wlen - *cur_len);
    *cur_len += len;

    while (len--)
        sdrv_i2c_lld_send_data(adap->base, *tbuf++);
}

static void sdrv_i2c_read(i2c_adap_dev_t *adap, uint8_t *buf,
                          uint16_t rlen, uint16_t *cur_len)
{
    uint8_t *rbuf = buf + *cur_len;
    int flen = sdrv_i2c_lld_get_rspace(adap->base);
    int len = min(flen, rlen - *cur_len);
    *cur_len += len;

    while (len--)
        *rbuf++ = sdrv_i2c_lld_recv_data(adap->base);
}

static int sdrv_i2c_pwrite(i2c_adap_dev_t *adap,  uint8_t *buf, uint16_t wlen)
{
    int len = 0;
    int fcnt = 1000;
    int scnt = 1000;

    while (wlen) {
        int flen = sdrv_i2c_lld_get_wspace(adap->base);
        if (!flen) {
            udelay(100);
            if (fcnt--)
                continue;
            else
                return -1;
        }

        len = min(flen, wlen);
        wlen -= len;
        while (len--)
            sdrv_i2c_lld_send_data(adap->base, *buf++);
        fcnt = 1000;
    }

    while (scnt--) {
        if (sdrv_i2c_lld_get_int0_stat(adap->base) & SDRV_I2C_INT0_APBCMDDONE) {
            udelay(20);
            break;
        }
        udelay(100);
    }

    if (scnt <= 0)
        return -2;

    return 0;
}

static int sdrv_i2c_pread(i2c_adap_dev_t *adap, uint8_t *buf, uint16_t rlen)
{
    int len = 0;
    int fcnt = 1000;
    int scnt = 1000;

    while (rlen) {
        int flen = sdrv_i2c_lld_get_rspace(adap->base);
        if (!flen) {
            udelay(100);

            if (fcnt--)
                continue;
            else
                return -1;
        }

        len = min(flen, rlen);
        rlen -= len;
        while (len--)
            *buf++ = sdrv_i2c_lld_recv_data(adap->base);
        fcnt = 1000;
    }

    while (scnt--) {
        if (sdrv_i2c_lld_get_int0_stat(adap->base) & SDRV_I2C_INT0_APBCMDDONE) {
            udelay(20);
            break;
        }
        udelay(100);
    }

    if (scnt <= 0)
        return -2;

    return 0;
}

static void sdrv_i2c_disable_int_all(i2c_adap_dev_t *adap)
{
    sdrv_i2c_lld_unmask_int0(adap->base, 0x0);
    sdrv_i2c_lld_unmask_int1(adap->base, 0x0);
    sdrv_i2c_lld_unmask_int2(adap->base, 0x0);
    sdrv_i2c_lld_unmask_int3(adap->base, 0x0);
}

#ifdef SDRV_I2C_FUSA
static int sdrv_i2c_fusa_check(i2c_adap_dev_t *adap)
{
    uint32_t int3_stat, parity_stat;

    int3_stat = sdrv_i2c_lld_get_int3_stat(adap->base);
    sdrv_i2c_lld_read_reg(adap->base, I2C_RED_PARITY_ERR_INT_STAT, &parity_stat);

    if (int3_stat & SDRV_I2C_INT3_MASK || parity_stat) {
        ssdk_printf(SSDK_ERR, "i2c-%u fusa err stat3=%#x, parity_stat=%#x\n",
                    adap->id, int3_stat, parity_stat);
        return -1;
    }

    return 0;
}
#else
static int sdrv_i2c_fusa_check(i2c_adap_dev_t *adap) {return 0;}
#endif

static int sdrv_i2c_xfer(i2c_adap_dev_t *adap, struct i2c_msg *msgs, int num)
{
    int ret = 0;

    if (num < 1) {
        ssdk_printf(SSDK_ERR, "%s-%u i2c msg num invaild\n", __func__, adap->id);
        return -1;
    }

    sdrv_i2c_dev_t *dev = adap->priv;
    sdrv_i2c_mdev_t *mdev = &dev->dev_type.mdev;
    mdev->msgs = msgs;
    mdev->num = num;
    mdev->status = 0;
    mdev->cur_msg_idx = 0;
    mdev->cur_msg_len = 0;
    mdev->cur_cmd_done = 0;
    mdev->cur_msg_stat = SDRV_I2C_MSG_IDLE;

    sdrv_i2c_disable_int_all(adap);
    sdrv_i2c_lld_clear_fifo(adap->base);
    sdrv_i2c_lld_clear_int(adap->base);
    sdrv_i2c_lld_reset(adap->base, true);

    if (mdev->msgs[0].polling) {
        for (int trans = 0; trans < mdev->num; trans++) {
            mdev->cur_msg = &mdev->msgs[trans];
            sdrv_i2c_lld_set_cmdcsr2(adap->base, mdev->cur_msg->addr
                | (mdev->cur_msg->addr_flag << 10)
                | (mdev->cur_msg->rw_flag << 11)
                | (mdev->cur_msg->len << 16));

            if (trans == mdev->num - 1)
                sdrv_i2c_lld_set_cmdcsr0(adap->base, 0xA53C | (0x1 << 30));
            else
                sdrv_i2c_lld_set_cmdcsr0(adap->base, 0xA53C | (0x0 << 30));

            if (mdev->cur_msg->rw_flag)
                ret = sdrv_i2c_pread(adap, mdev->cur_msg->buf, mdev->cur_msg->len);
            else
                ret = sdrv_i2c_pwrite(adap, mdev->cur_msg->buf, mdev->cur_msg->len);

            if (sdrv_i2c_fusa_check(adap) < 0) {
                ret = -EAGAIN;
            }

            if (ret < 0) {
                uint32_t psr0 = sdrv_i2c_lld_get_psr0_stat(adap->base);
                uint32_t int_stat = sdrv_i2c_lld_get_int0_stat(adap->base);
                ssdk_printf(SSDK_ERR, "i2c-%u perr stat=%x, psr0=%x\n", adap->id, int_stat, psr0);
                return ret;
            }
        }
    }
    else {
        uint32_t flags;
        sdrv_i2c_lld_unmask_int0(adap->base, SDRV_I2C_INT0_DEFAULT_MASK);
#ifdef SDRV_I2C_FUSA
        sdrv_i2c_lld_unmask_int3(adap->base, SDRV_I2C_INT3_MASK);
        sdrv_i2c_lld_write_reg(adap->base, I2C_RED_PARITY_ERR_INT_SIG_EN, 0x1);
#endif
        flags = osEventFlagsWait(mdev->event, SDRV_I2C_EVENT_COMPLETE, osFlagsWaitAll, mdev->timeout);

        if (osErrorTimeout == flags)
            ret = -ETIMEDOUT;
        else
            ret = mdev->status;

        if (ret < 0) {
            uint32_t psr0 = sdrv_i2c_lld_get_psr0_stat(adap->base);
            ssdk_printf(SSDK_ERR, "i2c-%u err stat=%x, psr0=%x\n", adap->id, mdev->int0_stat, psr0);
        }
    }

    return ret;
}

static int sdrv_i2c_get_timing_cfg(enum i2c_speed speed, uint32_t clk,
                                   sdrv_i2c_timing_t *timing)
{
    sdrv_i2c_timing_cfg_t cfg = {0};
    uint32_t clk_khz = clk / 1000;

    if ((timing->divs = clk_khz / 108000))
        clk_khz /= (timing->divs + 1);

    if (speed == I2C_SSPEED) {
        cfg.timscll = CAL_SCLL(5000, clk_khz);
        cfg.timtasclh = CAL_TA(150, clk_khz);
        cfg.timtascll = CAL_TA(150, clk_khz);
        cfg.timtasdal = CAL_TA(150, clk_khz);
        cfg.timdvdat = CAL_DVDAT(3400, clk_khz);
        cfg.timunit = CAL_UNIT(6);
        cfg.timmext = CAL_MEXT(0, clk_khz);
        cfg.timsext = CAL_SEXT(0, clk_khz);
        cfg.timstuck = CAL_STUCK(5000, clk_khz);
        cfg.timsdarxhld = CAL_SDARXHLD(300, clk_khz);
        cfg.timdatspl = CAL_DATSPL(2);
        cfg.timbuf = CAL_BUF(5000, clk_khz);
        cfg.timidle = CAL_IDLE(0, clk_khz);
    }
    else if (speed == I2C_FSPEED) {
        cfg.timscll = CAL_SCLL(1250, clk_khz);
        cfg.timtasclh = CAL_TA(150, clk_khz);
        cfg.timtascll = CAL_TA(150, clk_khz);
        cfg.timtasdal = CAL_TA(150, clk_khz);
        cfg.timdvdat = CAL_DVDAT(900, clk_khz);
        cfg.timunit = CAL_UNIT(6);
        cfg.timmext = CAL_MEXT(0, clk_khz);
        cfg.timsext = CAL_SEXT(0, clk_khz);
        cfg.timstuck = CAL_STUCK(5000, clk_khz);
        cfg.timsdarxhld = CAL_SDARXHLD(300, clk_khz);
        cfg.timdatspl = CAL_DATSPL(2);
        cfg.timbuf = CAL_BUF(5000, clk_khz);
        cfg.timidle = CAL_IDLE(0, clk_khz);
    }
    else if (speed == I2C_PSPEED) {
        cfg.timscll = CAL_SCLL(500, clk_khz);
        cfg.timtasclh = CAL_TA(60, clk_khz);
        cfg.timtascll = CAL_TA(60, clk_khz);
        cfg.timtasdal = CAL_TA(60, clk_khz);
        cfg.timdvdat = CAL_DVDAT(450, clk_khz);
        cfg.timunit = CAL_UNIT(6);
        cfg.timmext = CAL_MEXT(0, clk_khz);
        cfg.timsext = CAL_SEXT(0, clk_khz);
        cfg.timstuck = CAL_STUCK(5000, clk_khz);
        cfg.timsdarxhld = CAL_SDARXHLD(120, clk_khz);
        cfg.timdatspl = CAL_DATSPL(2);
        cfg.timbuf = CAL_BUF(5000, clk_khz);
        cfg.timidle = CAL_IDLE(0, clk_khz);
    }
    else if (speed == I2C_HSPEED) {
        cfg.timscll_h = CAL_SCLL(400, clk_khz);
        cfg.timtasclh_h = CAL_TA(30, clk_khz);
        cfg.timtascll_h = CAL_TA(30, clk_khz);
        cfg.timtasdal_h = CAL_TA(30, clk_khz);
        cfg.timdvdat_h = CAL_DVDAT(40, clk_khz);
        cfg.timunit = CAL_UNIT(6);
        cfg.timmext = CAL_MEXT(0, clk_khz);
        cfg.timsext = CAL_SEXT(0, clk_khz);
        cfg.timstuck = CAL_STUCK(5000, clk_khz);
        cfg.timsdarxhld = CAL_SDARXHLD(60, clk_khz);
        cfg.timdatspl = CAL_DATSPL(2);
        cfg.timbuf = CAL_BUF(5000, clk_khz);
        cfg.timidle = CAL_IDLE(0, clk_khz);
    }
    else if (speed == I2C_USPEED) {
        cfg.timscll = CAL_SCLL(125, clk_khz);
        cfg.timtasclh = CAL_TA(25, clk_khz);
        cfg.timtascll = CAL_TA(25, clk_khz);
        cfg.timtasdal = CAL_TA(25, clk_khz);
        cfg.timdvdat = CAL_DVDAT(20, clk_khz);
        cfg.timunit = CAL_UNIT(6);
        cfg.timmext = CAL_MEXT(0, clk_khz);
        cfg.timsext = CAL_SEXT(0, clk_khz);
        cfg.timstuck = CAL_STUCK(5000, clk_khz);
        cfg.timsdarxhld = CAL_SDARXHLD(20, clk_khz);
        cfg.timdatspl = CAL_DATSPL(2);
        cfg.timbuf = CAL_BUF(5000, clk_khz);
        cfg.timidle = CAL_IDLE(0, clk_khz);
    }
    else {
        return -1;
    }

    timing->pcr2 = cfg.timscll | (cfg.timtasclh << 12) | (cfg.timtascll << 16) |
                    (cfg.timtasdal << 20) | (cfg.timdvdat << 24);
    timing->pcr3 = cfg.timscll_h | (cfg.timtasclh_h << 12) | (cfg.timtascll_h << 16) |
                    (cfg.timtasdal_h << 20) | (cfg.timdvdat_h << 24);
    timing->pcr4 = cfg.timunit | (cfg.timmext << 3) | (cfg.timsext << 9) |
                    (cfg.timstuck << 16);
    timing->pcr5 = cfg.timsdarxhld | (cfg.timdatspl << 6) | (cfg.timbuf << 18) |
                    (cfg.timidle << 21);

    return 0;
}

static int sdrv_i2c_master_irq_handler(uint32_t irq, void *arg)
{
    i2c_adap_dev_t *adap = arg;
    sdrv_i2c_dev_t *dev = adap->priv;
    sdrv_i2c_mdev_t *mdev = &dev->dev_type.mdev;
    mdev->int0_stat = sdrv_i2c_lld_get_int0_stat(adap->base);
    sdrv_i2c_lld_clear_int(adap->base);

    if ((sdrv_i2c_fusa_check(adap) < 0) || (mdev->int0_stat & SDRV_I2C_INT0_ERR_STAT)) {
        mdev->status = -1;
        goto done;
    }

    if (mdev->int0_stat & SDRV_I2C_INT0_APBCMDDONE)
        mdev->cur_cmd_done = 1;
    else
        mdev->cur_cmd_done = 0;

    if (mdev->cur_msg_stat == SDRV_I2C_MSG_IDLE
            && mdev->cur_msg_idx < mdev->num) {
        mdev->cur_msg = &mdev->msgs[mdev->cur_msg_idx];
        sdrv_i2c_lld_set_cmdcsr2(adap->base, mdev->cur_msg->addr
            | (mdev->cur_msg->addr_flag << 10)
            | (mdev->cur_msg->rw_flag << 11)
            | (mdev->cur_msg->len << 16));

        if (mdev->cur_msg_idx == mdev->num - 1)
            sdrv_i2c_lld_set_cmdcsr0(adap->base, 0xA53C | (0x1 << 30));
        else
            sdrv_i2c_lld_set_cmdcsr0(adap->base, 0xA53C | (0x0 << 30));

        if (mdev->cur_msg->rw_flag)
            sdrv_i2c_lld_unmask_int0(adap->base,
                                     SDRV_I2C_INT0_DEFAULT_MASK & ~SDRV_I2C_INT0_TXFWE);
        else
            sdrv_i2c_lld_unmask_int0(adap->base,
                                     SDRV_I2C_INT0_DEFAULT_MASK & ~SDRV_I2C_INT0_RXFWF);
    }

    if (mdev->cur_msg_idx < mdev->num) {
        mdev->cur_msg_stat = SDRV_I2C_MSG_RW;
        if (mdev->cur_msg->rw_flag)
            sdrv_i2c_read(adap, mdev->cur_msg->buf, mdev->cur_msg->len, &mdev->cur_msg_len);
        else
            sdrv_i2c_write(adap, mdev->cur_msg->buf, mdev->cur_msg->len, &mdev->cur_msg_len);
    }

    if (mdev->cur_msg_len >= mdev->cur_msg->len) {
        sdrv_i2c_lld_unmask_int0(adap->base,
                                 SDRV_I2C_INT0_DEFAULT_MASK & ~(SDRV_I2C_INT0_RXFWF | SDRV_I2C_INT0_TXFWE));
        mdev->cur_msg_stat = SDRV_I2C_MSG_IDLE;
        mdev->cur_msg_len = 0;
        mdev->cur_msg_idx++;
    }

    if (mdev->cur_msg_idx >= mdev->num && mdev->cur_cmd_done)
        goto done;

    return 0;
done:
    sdrv_i2c_disable_int_all(adap);
    osEventFlagsSet(mdev->event, SDRV_I2C_EVENT_COMPLETE);
    return 0;
}

static void sdrv_i2c_slave_reset(i2c_adap_dev_t *adap, sdrv_i2c_sdev_t *sdev)
{
    sdrv_i2c_lld_unmask_int0(adap->base, SDRV_I2C_INT0_SLV_DEFAULT_MASK);
    sdrv_i2c_lld_clear_fifo(adap->base);
    sdrv_i2c_lld_clear_int(adap->base);
    sdrv_i2c_lld_set_cmdcsr1(adap->base, SDRV_I2C_CMD1_SLV_MASK);
    sdrv_i2c_lld_set_cmdcsr2(adap->base, SDRV_I2C_CMD2_SLV_MASK);
    sdrv_i2c_lld_set_cmdcsr3(adap->base, SDRV_I2C_CMD3_SLV_MASK);
    sdev->ctlbyte_det = false;
    sdev->rw_event = I2C_SLV_RW_NONE;
    sdev->first_read_req = true;
    sdev->slv_buf_pos = 0;
}

static int sdrv_i2c_slave_irq_handler(uint32_t irq, void *arg)
{
    uint32_t int0_stat, int2_stat;
    i2c_adap_dev_t *adap = arg;
    sdrv_i2c_dev_t *dev = adap->priv;
    sdrv_i2c_sdev_t *sdev = &dev->dev_type.sdev;

    int0_stat = sdrv_i2c_lld_get_int0_stat(adap->base);
    int2_stat = sdrv_i2c_lld_get_int2_stat(adap->base);
    //ssdk_printf(SSDK_ERR, "stat0=%x stat2=%x\n", int0_stat, int2_stat);

    if (sdrv_i2c_fusa_check(adap) < 0) {
        sdrv_i2c_slave_reset(adap, sdev);
        return 0;
    }

    if (int0_stat & SDRV_I2C_INT0_ERR_STAT) {
        sdrv_i2c_slave_reset(adap, sdev);
        ssdk_printf(SSDK_ERR, "i2c-%u err stat0=%x,stat2=%x\n", adap->id, int0_stat, int2_stat);
        return 0;
    }

    if (int2_stat & SDRV_I2C_INT2_STOPDET) {
        sdrv_i2c_slave_reset(adap, sdev);

    }else if (int2_stat & SDRV_I2C_INT2_SLVRDREQDET) {
        if (sdev->ctlbyte_det)
            sdev->rw_event = I2C_SLV_RD_REQUEST;
        sdrv_i2c_lld_unmask_int0(adap->base, SDRV_I2C_INT0_SLV_DEFAULT_MASK | SDRV_I2C_INT0_TXFWE);
        sdrv_i2c_lld_write_reg(adap->base, I2C_INTR2, int2_stat & SDRV_I2C_INT2_SLVRDREQDET);

    } else if (int2_stat & SDRV_I2C_INT2_SLVWRREQDET) {
        if (sdev->ctlbyte_det)
            sdev->rw_event = I2C_SLV_WR_RECEIVE;
        sdrv_i2c_lld_unmask_int0(adap->base, SDRV_I2C_INT0_SLV_DEFAULT_MASK | SDRV_I2C_INT0_RXFWF);
        sdrv_i2c_lld_write_reg(adap->base, I2C_INTR2, int2_stat & SDRV_I2C_INT2_SLVWRREQDET);

    } else if (int2_stat & SDRV_I2C_INT2_CTLBYTEDET) {
        sdev->ctlbyte_det = true;
        sdrv_i2c_lld_clear_fifo(adap->base);
        sdev->rw_req_len = sdrv_i2c_lld_get_cmdcsr1(adap->base);
        sdrv_i2c_lld_set_cmdcsr2(adap->base, (sdev->rw_req_len << 16) | SDRV_I2C_SLV_ADDR);
        sdrv_i2c_lld_set_cmdcsr3(adap->base, (sdev->rw_req_len << 16) | SDRV_I2C_SLV_ADDR);
        sdrv_i2c_lld_write_reg(adap->base, I2C_INTR2, int2_stat & SDRV_I2C_INT2_CTLBYTEDET);
    }

    if (int0_stat & SDRV_I2C_INT0_RXFWF) {
        if (sdev->rw_event == I2C_SLV_WR_RECEIVE) {
            int flen = sdrv_i2c_lld_get_rspace(adap->base);
            if (sdev->rw_req_len - sdev->slv_buf_pos >= flen) {
                while (flen--)
                    sdev->slv_buf[sdev->slv_buf_pos++] = sdrv_i2c_lld_recv_data(adap->base);
            }
            sdrv_i2c_lld_write_reg(adap->base, I2C_INTR0, int0_stat & SDRV_I2C_INT0_RXFWF);
        }
    }

    if (int0_stat & SDRV_I2C_INT0_SLVWRTRANSDONE) {
        if (sdev->rw_event == I2C_SLV_WR_RECEIVE) {
            int flen = sdrv_i2c_lld_get_rspace(adap->base);
            if (sdev->rw_req_len - sdev->slv_buf_pos >= flen) {
                while (flen--)
                    sdev->slv_buf[sdev->slv_buf_pos++] = sdrv_i2c_lld_recv_data(adap->base);
            }

            sdrv_i2c_slave_reset(adap, sdev);

            if (adap->slv_cb)
                adap->slv_cb(adap, I2C_SLV_WR_RECEIVE, sdev->slv_buf, sdev->rw_req_len);
        } else {
            sdrv_i2c_lld_write_reg(adap->base, I2C_INTR0, int0_stat & SDRV_I2C_INT0_SLVWRTRANSDONE);
        }
    }

    if (sdev->rw_event == I2C_SLV_RD_REQUEST) {
        if (int0_stat & SDRV_I2C_INT0_TXFWE) {
            if (sdev->first_read_req) {
                adap->slv_cb(adap, I2C_SLV_RD_REQUEST, sdev->slv_buf, sdev->rw_req_len);
                sdev->first_read_req = false;
            }

            if (sdev->slv_buf_pos < sdev->rw_req_len) {
                int flen = sdrv_i2c_lld_get_wspace(adap->base);
                int len = min(flen, sdev->rw_req_len - sdev->slv_buf_pos);
                while (len--)
                    sdrv_i2c_lld_send_data(adap->base, sdev->slv_buf[sdev->slv_buf_pos++]);
            } else {
                sdrv_i2c_lld_unmask_int0(adap->base, SDRV_I2C_INT0_SLV_DEFAULT_MASK);
            }

            sdrv_i2c_lld_write_reg(adap->base, I2C_INTR0, int0_stat & SDRV_I2C_INT0_TXFWE);
        }

        if (int0_stat & SDRV_I2C_INT0_SLVRDTRANSDONE) {
            sdrv_i2c_slave_reset(adap, sdev);
        }
    }

    return 0;
}

static int sdrv_i2c_do_init(i2c_adap_dev_t *adap)
{
    sdrv_i2c_timing_t timing = {0};
    sdrv_i2c_dev_t *dev = adap->priv;

    if (adap->opmode == I2C_MASTER) {
        dev->dev_type.mdev.timeout = SDRV_I2C_TIMEOUT;
        dev->dev_type.mdev.event = osEventFlagsNew(NULL);
        dev->dev_type.mdev.status = 0;
    } else {
        dev->dev_type.sdev.ctlbyte_det = false;
        dev->dev_type.sdev.rw_event = I2C_SLV_RW_NONE;
        dev->dev_type.sdev.first_read_req = true;
        dev->dev_type.sdev.rw_req_len = 0;
        dev->dev_type.sdev.slv_buf_pos = 0;
    }

    if (sdrv_i2c_get_timing_cfg(adap->speed, adap->clk, &timing)) {
        ssdk_printf(SSDK_ERR, "%s-%u speed invaild\n", __func__, adap->id);
        return -1;
    }

    sdrv_i2c_lld_reset(adap->base, true);
    sdrv_i2c_lld_set_watermark(adap->base, SDRV_I2C_TX_WML, SDRV_I2C_RX_WML);
#ifdef SDRV_I2C_FUSA
    sdrv_i2c_lld_write_reg(adap->base, I2C_RED_PARITY_ERR_INT_STAT_EN, 0x1);
#endif

    if (adap->speed == I2C_HSPEED)
        sdrv_i2c_lld_set_speed(adap->base, SDRV_I2C_HSPEED | (timing.divs << 3));
    else if (adap->speed == I2C_USPEED)
        sdrv_i2c_lld_set_speed(adap->base, SDRV_I2C_USPEED | (timing.divs << 3));
    else
        sdrv_i2c_lld_set_speed(adap->base, SDRV_I2C_NSPEED | (timing.divs << 3));

    sdrv_i2c_lld_set_timing(adap->base,
                            timing.pcr2, timing.pcr3, timing.pcr4, timing.pcr5);

    if (adap->opmode == I2C_MASTER) {
        sdrv_i2c_lld_set_opmode(adap->base, SDRV_I2C_MASTER);
        sdrv_i2c_lld_disable(adap->base);
        irq_attach(adap->irq, sdrv_i2c_master_irq_handler, adap);
    } else {
        sdrv_i2c_lld_set_opmode(adap->base, SDRV_I2C_SLAVE);
        sdrv_i2c_lld_set_slvaddr(adap->base, SDRV_I2C_SLV_ADDR);
        sdrv_i2c_lld_set_cmdcsr1(adap->base, SDRV_I2C_CMD1_SLV_MASK);
        sdrv_i2c_lld_set_cmdcsr2(adap->base, SDRV_I2C_CMD2_SLV_MASK);
        sdrv_i2c_lld_set_cmdcsr3(adap->base, SDRV_I2C_CMD3_SLV_MASK);
        sdrv_i2c_lld_unmask_int0(adap->base, SDRV_I2C_INT0_SLV_DEFAULT_MASK);
        sdrv_i2c_lld_unmask_int2(adap->base, SDRV_I2C_INT2_SLV_MASK);
#ifdef SDRV_I2C_FUSA
        sdrv_i2c_lld_unmask_int3(adap->base, SDRV_I2C_INT3_MASK);
        sdrv_i2c_lld_write_reg(adap->base, I2C_RED_PARITY_ERR_INT_SIG_EN, 0x1);
#endif
        sdrv_i2c_lld_clear_int(adap->base);
        irq_attach(adap->irq, sdrv_i2c_slave_irq_handler, adap);
    }

    irq_enable(adap->irq);
    return 0;
}

static int sdrv_i2c_init(i2c_adap_dev_t *adap)
{
    int ret;

    if (adap->opmode == I2C_MASTER || adap->opmode == I2C_SLAVE)
        ret = sdrv_i2c_do_init(adap);
    else {
        ret = -1;
        ssdk_printf(SSDK_ERR, "%s-%u invaild opmode\n", __func__, adap->id);
    }

    return ret;
}

static i2c_adap_ops_t sdrv_i2c_adap_ops = {
    .i2c_init = sdrv_i2c_init,
    .i2c_xfer = sdrv_i2c_xfer,
};

i2c_adap_dev_t *sdrv_i2c_adap_probe(sdrv_i2c_dev_t *dev, i2c_adap_cfg_t *cfg, int id)
{
    i2c_adap_dev_t *adap = &dev->adap;
    adap->id = id;
    adap->ops = &sdrv_i2c_adap_ops;
    adap->retry = SDRV_I2C_RETRY;
    adap->priv = dev;
    i2c_adap_earlyinit(adap, cfg);
    return adap;
}

