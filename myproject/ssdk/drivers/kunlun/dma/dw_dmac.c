/**
 * @file  dw_dmac.c
 * @brief dmac driver.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 */
#include <arch.h>
#include <reg.h>
#include <string.h>
#include <sd_errcode.h>

#include "dw_dmac.h"

#include "board.h"
#include "debug.h"
#include "driver/memmap.h"
#include "types.h"

static inline uint32_t dw_dmac_regoff(paddr_t base, uint32_t offset)
{
#if CONFIG_BOARD_X9SP
    return offset;
#else
    if (base == DMA1_BASE)
        offset <<= 1;

    return offset;
#endif
}

static inline void dw_dmac_prefetch_disable(paddr_t base)
{
#ifndef CONFIG_BOARD_X9SP
    if (base == DMA1_BASE) {
        arm_prefetch_burst_disable();
    }
#endif
}

static inline void dw_dmac_prefetch_enable(paddr_t base)
{
#ifndef CONFIG_BOARD_X9SP
    if (base == DMA1_BASE) {
        arm_prefetch_burst_enable();
    }
#endif
}

static inline void dw_dmac_writereg(paddr_t base, uint32_t reg, uint32_t val)
{
    writel(val, base + dw_dmac_regoff(base, reg));
}

static inline uint32_t dw_dmac_readreg(paddr_t base, uint32_t reg)
{
    return readl(base + dw_dmac_regoff(base, reg));
}

/* dmac int enable/disable. */
static void dw_dmac_set_int_reg(paddr_t dma_base, uint32_t enable)
{
    uint32_t val = dw_dmac_readreg(dma_base, DMAC_CFG_REG_OFFSET);

    val |= (enable << 1) | 0x01; // bit0-dmac_en bit1-int_en

    dw_dmac_writereg(dma_base, DMAC_CFG_REG_OFFSET, val);
}

static void dw_dmac_ch_set_addr_reg(paddr_t dma_base, uint32_t chan,
                                    uint64_t src_addr, uint64_t dst_addr,
                                    uint32_t size)
{
    uint32_t offset = DMAC_CH_REG_OFFSET + chan * DMAC_CH_REG_STEP;
    uint32_t val = src_addr & 0xFFFFFFFF;

    dw_dmac_writereg(dma_base, offset + DMAC_CH_REG_SAR_L_OFFSET, val);
    dw_dmac_writereg(dma_base, offset + DMAC_CH_REG_SAR_H_OFFSET, 0);

    val = dst_addr & 0xFFFFFFFF;
    dw_dmac_writereg(dma_base, offset + DMAC_CH_REG_DAR_L_OFFSET, val);
    dw_dmac_writereg(dma_base, offset + DMAC_CH_REG_DAR_H_OFFSET, 0);

    val = size - 1;
    dw_dmac_writereg(dma_base, offset + DMAC_CH_REG_BLK_TS_OFFSET, val);
}

static void dw_dmac_ch_set_ctrl_reg(paddr_t dma_base, uint32_t chan,
                                    uint64_t ctrl_val)
{
    uint32_t offset = DMAC_CH_CTRL_OFFSET + chan * DMAC_CH_REG_STEP;
    uint32_t val = ctrl_val & 0xFFFFFFFF;

    dw_dmac_writereg(dma_base, offset + DMAC_CH_CTRL_L_OFFSET, val);

    val = (ctrl_val >> 32) & 0xFFFFFFFF;
    dw_dmac_writereg(dma_base, offset + DMAC_CH_CTRL_H_OFFSET, val);
}

static void dw_dmac_ch_set_cfg_reg(paddr_t dma_base, uint32_t chan,
                                   uint64_t cfg_val)
{
    uint32_t offset = DMAC_CH_CFG_OFFSET + chan * DMAC_CH_REG_STEP;
    uint32_t val = cfg_val & 0xFFFFFFFF;

    dw_dmac_writereg(dma_base, offset + DMAC_CH_CFG_L_OFFSET, val);

    val = (cfg_val >> 32) & 0xFFFFFFFF;
    dw_dmac_writereg(dma_base, offset + DMAC_CH_CFG_H_OFFSET, val);
}

static void dw_dmac_ch_set_lli_reg(paddr_t dma_base, uint32_t chan,
                                   uint64_t llp)
{
    uint32_t offset = 0;
    uint32_t val = 0;

    if (0 != llp) {
        offset = DMAC_CH_CTRL_OFFSET + chan * DMAC_CH_REG_STEP;

        val = dw_dmac_readreg(dma_base, offset + DMAC_CH_CTRL_H_OFFSET);

        val |= DMAC_CH_CTRL_LLI_VALID_MASK(1);
        dw_dmac_writereg(dma_base, offset + DMAC_CH_CTRL_H_OFFSET, val);

        offset = DMAC_CH_LLP_OFFSET + chan * DMAC_CH_REG_STEP;

        val = llp & 0xFFFFFFFF;
        dw_dmac_writereg(dma_base, offset + DMAC_CH_LLP_L_OFFSET, val);

        val = (llp >> 32) & 0xFFFFFFFF;
        dw_dmac_writereg(dma_base, offset + DMAC_CH_LLP_H_OFFSET, val);
    }

    offset = DMAC_CH_AXI_QOS_OFFSET + chan * DMAC_CH_REG_STEP;
    dw_dmac_writereg(dma_base, offset + DMAC_CH_AXI_QOS_L_OFFSET, 0);
    dw_dmac_writereg(dma_base, offset + DMAC_CH_AXI_QOS_H_OFFSET, 0);
}

void dw_dmac_set_int_stat(dmac_coeff_t *cfg, bool en, uint8_t type)
{
    ch_int_stat_en_t mask = {0};

    if (en) {
        cfg->dmac_int_en = 1;
        if (!type)
            mask.dma_tr_done = 1;
        else
            mask.block_tr_done = 1;
        mask.vl |= DMA_ALL_ERR;
        mask.ch_src_suspended = 1;
        mask.ch_suspended = 1;
        mask.ch_disabled = 1;
        mask.ch_aborted = 1;
    } else {
        cfg->dmac_int_en = 0;
        mask.dma_tr_done = 1;
        /* handle error irq */
        mask.vl |= DMA_ALL_ERR;
    }

    cfg->dmac_int_mask = mask.vl;
}

void dw_dmac_get_ctrl_cfg(dmac_coeff_t *cfg, ch_ctl_t *ctrl_cfg)
{
    ctrl_cfg->sinc = cfg->sinc;
    ctrl_cfg->dinc = cfg->dinc;
    ctrl_cfg->src_tr_width = cfg->src_tr_width;
    ctrl_cfg->dst_tr_width = cfg->dst_tr_width;
    ctrl_cfg->src_msize = cfg->src_msize;
    ctrl_cfg->dst_msize = cfg->dst_msize;
    ctrl_cfg->arlen_en = cfg->arlen_en;
    ctrl_cfg->arlen = cfg->arlen;
    ctrl_cfg->awlen_en = cfg->awlen_en;
    ctrl_cfg->awlen = cfg->awlen;
    ctrl_cfg->ioc_blk_tr = 1; /* interrupt 0 disable */
    /* not last shadow-> */
    ctrl_cfg->shadow_lli_valid = 1;
    ctrl_cfg->shadow_lli_last = 0;
    ctrl_cfg->ar_cache = cfg->ar_cache;
    ctrl_cfg->aw_cache = cfg->aw_cache;
}

void dw_dmac_get_cfg_param(dmac_coeff_t *cfg, ch_cfg_t *cfg_val)
{
    bool src_or_dst = (cfg->dma_transfer_type == 2) ? true : false;

    cfg_val->vl = 0;
    cfg_val->vh = 0;
    cfg_val->tt_fc = cfg->dma_transfer_type;
    cfg_val->src_per = cfg->chan_id * 2;
    cfg_val->dst_per = cfg->chan_id * 2 + 1;
    cfg_val->ch_pri = cfg->channel_pri;
    cfg_val->lock_ch = 0;
    cfg_val->lock_ch_l = 2;
    cfg_val->src_osr_lmt = 15;
    cfg_val->dst_osr_lmt = 15;
    cfg_val->src_multblk_type = cfg->src_transfer_type;
    cfg_val->dst_multblk_type = cfg->dst_transfer_type;

    if (src_or_dst)
        cfg_val->hs_sel_src = cfg->hs_sel_src;
    else
        cfg_val->hs_sel_dst = cfg->hs_sel_dst;
}

static bool dw_dmac_mux_cfg(paddr_t mux_base, uint32_t chan, uint32_t port,
                            DMA_MUX_DIR direct)
{
    dmac_mux_t *dmac_mux = (dmac_mux_t *)mux_base;

    /* disable MUX at first , should it will interrupt other transfer process?*/
    dmac_mux->en_reg.en = 0;

    if (DMA_MUX_WR == direct)
        dmac_mux->ch[chan].wr_hdsk = port;
    else
        dmac_mux->ch[chan].rd_hdsk = port;

    /* Re-enable dma mux. */
    dmac_mux->en_reg.en = 1;
    /* delay two nop for read correct result*/
    __NOP;
    __NOP;

    /* Check result of MUX. */
    if (DMA_MUX_WR == direct)
        return dmac_mux->ch[chan].updated_wr_hdsk == port;
    else
        return dmac_mux->ch[chan].updated_rd_hdsk == port;
}

void dw_dmac_ch_int_cfg(paddr_t dma_base, uint32_t chan,
                               uint32_t mask_flag)
{
    uint32_t offset = DMAC_CH_STAT_EN_OFFSET + chan * DMAC_CH_REG_STEP;
    uint32_t val = 0;

    /*  enable irq. */
    dw_dmac_writereg(dma_base, offset + DMAC_CH_STAT_EN_L_OFFSET, mask_flag);

    offset = DMAC_CH_CTRL_OFFSET + chan * DMAC_CH_REG_STEP;
    val = dw_dmac_readreg(dma_base, offset + DMAC_CH_CTRL_H_OFFSET);

    if (1 == (mask_flag & DMAC_CH_STAT_EN_TR_DONE_MASK(1)))
        val |= DMAC_CH_CTRL_BLK_TR_MASK(1);
    else
        val &= ~DMAC_CH_CTRL_BLK_TR_MASK(1);

    dw_dmac_writereg(dma_base, offset + DMAC_CH_CTRL_H_OFFSET, val);
}

void dw_dmac_ch_intsignal_cfg(paddr_t dma_base, uint32_t chan,
                              uint64_t mask_flag)
{
    dw_dmac_writereg(dma_base,
                     (chan * DMAC_CH_REG_STEP) + DMAC_CH_INT_SIG_EN_OFFSET,
                     (int32_t)mask_flag);
    dw_dmac_writereg(
        dma_base, (chan * DMAC_CH_REG_STEP) + DMAC_CH_INT_SIG_EN_OFFSET + 0x4,
        (int32_t)(mask_flag >> 32));
}

static int get_dma_handshake_id(dw_dma_cfg_t *cfg, uint64_t addr,
                                DMA_MUX_DIR direct)
{
    for (int i = 0; i < cfg->mux_table_len; i++) {
        if ((addr >= p2ap(cfg->mux_table[i].start_addr)) &&
            (addr <
             (p2ap(cfg->mux_table[i].start_addr) + cfg->mux_table[i].len))) {
            if ((direct & cfg->mux_table[i].direct) > 0)
                return cfg->mux_table[i].port;
        }
    }

    return -1;
}

static void dw_dma_config_handshake(paddr_t mux_base, uint32_t chan,
                                    dw_dma_cfg_t *cfg, uint64_t sar,
                                    uint64_t dar)
{
    int rd_hs_id = get_dma_handshake_id(cfg, sar, DMA_MUX_RD);
    int wr_hs_id = get_dma_handshake_id(cfg, dar, DMA_MUX_WR);

    if (rd_hs_id >= 0)
        dw_dmac_mux_cfg(mux_base, chan, rd_hs_id, DMA_MUX_RD);
    else
        ERRCODE_PRINT(MOD_ID_DMA, mux_base, DMA_ERR_INVALID_MUX_ID, sar);

    if (wr_hs_id >= 0)
        dw_dmac_mux_cfg(mux_base, chan, wr_hs_id, DMA_MUX_WR);
    else
        ERRCODE_PRINT(MOD_ID_DMA, mux_base, DMA_ERR_INVALID_MUX_ID, dar);
}

bool dw_dmac_is_ch_enabled(paddr_t dma_base, uint32_t chan)
{
    uint32_t val = 0;

    val = dw_dmac_readreg(dma_base, DMAC_CH_EN_OFFSET);

    return BIT_SET(val, chan);
}

void dw_dmac_init_mux(paddr_t mux_base, uint32_t chan)
{
    dmac_mux_t *dmac_mux = (dmac_mux_t *)mux_base;

    dmac_mux->en_reg.en = 0;
    dmac_mux->ch[chan].rd_hdsk = 0xFF;
    dmac_mux->ch[chan].wr_hdsk = 0xFF;
    dmac_mux->en_reg.en = 1;
}

void dw_dmac_set_cfg(dmac_coeff_t cfg)
{
    paddr_t dma_base = cfg.dma_base;
    paddr_t mux_base = cfg.mux_base;
    uint32_t chan = cfg.chan_id;

    uint64_t ctrl_val = 0;
    uint64_t cfg_val = 0;
    ch_ctl_t ch_ctrl_val = {0};
    ch_cfg_t ch_cfg_val = {0};

    dw_dmac_prefetch_disable(dma_base);
    dw_dmac_get_ctrl_cfg(&cfg, &ch_ctrl_val);

    ch_ctrl_val.ioc_blk_tr = 0;
    ch_ctrl_val.shadow_lli_valid = 0;
    ch_ctrl_val.shadow_lli_last = 0;
    ctrl_val = ch_ctrl_val.vh;
    ctrl_val <<= 32;
    ctrl_val |= ch_ctrl_val.vl;

    dw_dmac_get_cfg_param(&cfg, &ch_cfg_val);

    cfg_val = ch_cfg_val.vh;
    cfg_val <<= 32;
    cfg_val |= ch_cfg_val.vl;

    dw_dmac_set_int_reg(dma_base, cfg.dmac_int_en);

    // dw_dmac_ch_set_addr_reg(dma_base, chan, cfg.src_addr, cfg.dst_addr,
    //                         cfg.block_transfer_size);

    dw_dmac_ch_set_ctrl_reg(dma_base, chan, ctrl_val);

    dw_dmac_ch_set_cfg_reg(dma_base, chan, cfg_val);

    dw_dmac_ch_set_lli_reg(dma_base, chan, cfg.llp);

    dw_dmac_ch_int_cfg(dma_base, chan, cfg.dmac_int_mask);

    if (DMA_TRANSFER_DIR_MEM2MEM_DMAC != cfg.dma_transfer_type)
        dw_dma_config_handshake(mux_base, chan, cfg.dw_dma_cfg, cfg.src_addr,
                                cfg.dst_addr);
    dw_dmac_prefetch_enable(dma_base);
}

void dw_dmac_enable_ch(paddr_t dma_base, uint32_t chan)
{
    uint32_t reg_val = ((1 << chan) + (1 << (chan + 8)));

    dw_dmac_prefetch_disable(dma_base);
    /* because there is write protect so don't care previous value. */
    dw_dmac_writereg(dma_base, DMAC_CH_EN_OFFSET, reg_val);
    dw_dmac_prefetch_enable(dma_base);
}

#if CONFIG_BOARD_X9SP
static void dw_dmac_abort_ch(paddr_t dma_base, uint32_t chan)
{
    uint32_t reg_val = ((1 << chan) + (1 << (chan + 8)));

    /* because there is write protect so don't care previous value. */
    dw_dmac_writereg(dma_base, DMAC_CH_ABORT_OFFSET, reg_val);
}
#endif

bool dw_dmac_disable_ch(dmac_coeff_t cfg)
{
    uint32_t val = 0;
    uint32_t tr_width, msize, max_exchange_count;
    uint32_t mux_number = 0;
    DMA_MUX_DIR mux_dir = 0;
    dmac_mux_t *dmac_mux = (dmac_mux_t *)cfg.mux_base;
    dw_dmac_prefetch_disable(cfg.dma_base);

    if (!dw_dmac_is_ch_enabled(cfg.dma_base, cfg.chan_id)) {
        dw_dmac_prefetch_enable(cfg.dma_base);
        ERRCODE_PRINT(MOD_ID_DMA, cfg.dma_base, DMA_ERR_INVALID_STATE,
                cfg.chan_id);
        return true;
    }

    /* because there is write protect so don't care previous value. */
    val = ((0 << (cfg.chan_id)) + (1 << (cfg.chan_id + 8)));
    dw_dmac_writereg(cfg.dma_base, DMAC_CH_EN_OFFSET, val);

    tr_width = 1 << cfg.dst_tr_width;

    if (cfg.dst_msize == 0)
        msize = 1;
    else
        msize = 2 << cfg.dst_msize;

    max_exchange_count = 128 * 4 / (tr_width * msize);

    if (cfg.dma_transfer_type == DMA_TRANSFER_DIR_MEM2PER_DMAC) {
        mux_dir = DMA_MUX_WR;
        mux_number = dmac_mux->ch[cfg.chan_id].wr_hdsk;

        while ((max_exchange_count-- > 0) &&
               dw_dmac_is_ch_enabled(cfg.dma_base, cfg.chan_id)) {
            if (DMA_TRANSFER_DIR_MEM2MEM_DMAC != cfg.dma_transfer_type) {
                dw_dmac_mux_cfg(cfg.mux_base, cfg.chan_id, 255, mux_dir);
                dw_dmac_mux_cfg(cfg.mux_base, cfg.chan_id, mux_number, mux_dir);
            }
        }
    }
#if CONFIG_BOARD_X9SP
    while (dw_dmac_is_ch_enabled(cfg.dma_base, cfg.chan_id)) {
        dw_dmac_abort_ch(cfg.dma_base, cfg.chan_id);
    }
    return true;
#else
    uint32_t count = 0;
/* delay 10ms for parse correct result*/
#define DW_DMA_CH_DISABLE_DELAY 10000 /* us */
    while (count < DW_DMA_CH_DISABLE_DELAY &&
           dw_dmac_is_ch_enabled(cfg.dma_base, cfg.chan_id)) {
        udelay(100);
        count += 100;
    }
    dw_dmac_prefetch_enable(cfg.dma_base);

    return count < DW_DMA_CH_DISABLE_DELAY;
#endif
}

/* reset dmac */
void dw_dmac_reset(paddr_t dma_base)
{
    dw_dmac_prefetch_disable(dma_base);

    dw_dmac_writereg(dma_base, DMAC_RESET_OFFSET, 1);

    dw_dmac_prefetch_enable(dma_base);
}

/* enable irq */
void dw_dmac_enable_irq(paddr_t dma_base)
{
    uint32_t val;

    dw_dmac_prefetch_disable(dma_base);
    val = dw_dmac_readreg(dma_base, DMAC_CFG_REG_OFFSET);

    val |= 0x02;
    dw_dmac_writereg(dma_base, DMAC_CFG_REG_OFFSET, val);
    dw_dmac_prefetch_enable(dma_base);
}

/* disable irq */
void dw_dmac_disable_irq(paddr_t dma_base)
{
    uint32_t val;

    dw_dmac_prefetch_disable(dma_base);
    val = dw_dmac_readreg(dma_base, DMAC_CFG_REG_OFFSET);
    val &= ~0x02;
    dw_dmac_writereg(dma_base, DMAC_CFG_REG_OFFSET, val);
    dw_dmac_prefetch_enable(dma_base);
}

uint32_t dw_dmac_get_ch_int_stat(paddr_t dma_base, uint32_t chan)
{
    uint32_t offset = DMAC_CH_INT_STAT_OFFSET + chan * DMAC_CH_REG_STEP;

    return dw_dmac_readreg(dma_base, offset);
}

bool dw_dmac_ch_has_int_status(paddr_t dma_base, uint32_t chan)
{
    bool status;

    dw_dmac_prefetch_disable(dma_base);
    status = !!(dw_dmac_readreg(dma_base, DMAC_INT_STAT_OFFSET) & (1 << chan));
    dw_dmac_prefetch_enable(dma_base);

    return status;
}

/* clear dw_dmac_int_stat here chan */
void dw_dmac_clr_ch_int_stat(paddr_t dma_base, uint32_t chan, uint32_t status)
{
    uint32_t offset = DMAC_CH_INT_CLR_OFFSET + chan * DMAC_CH_REG_STEP;

    dw_dmac_prefetch_disable(dma_base);
    dw_dmac_writereg(dma_base, offset, status);
    dw_dmac_prefetch_enable(dma_base);
}

/* wait dma channel transfer done */
bool dw_dmac_ch_tr_done(paddr_t dma_base, uint32_t chan)
{
    uint32_t val;
    bool chan_is_enabled;
    bool status;

    dw_dmac_prefetch_disable(dma_base);
    val = dw_dmac_readreg(dma_base, DMAC_CH_EN_OFFSET);
    chan_is_enabled = BIT_SET(val, chan);
    uint32_t ch_intstatus = dw_dmac_get_ch_int_stat(dma_base, chan);
    dw_dmac_clr_ch_int_stat(dma_base, chan, ch_intstatus);
    status = !(chan_is_enabled && ((ch_intstatus & 0x03) == 0));
    dw_dmac_prefetch_enable(dma_base);

    return status;
}

dma_status_e dw_dmac_irq_status_handle(uint32_t status)
{
    dma_status_e chan_status = DMA_INVALID;

    if ((status & DMAC_CH_INT_STAT_BLK_TR_DONE_MASK(1)) ||
        (status & DMAC_CH_INT_STAT_TR_DONE_MASK(1)))
        chan_status = DMA_COMPLETED;

    if ((status & DMA_ALL_ERR) || (status & DMAC_CH_INT_STAT_ABORT_MASK(1)) ||
        (status & DMAC_CH_INT_STAT_DISABLED_MASK(1)))
        chan_status = DMA_ERR;

    if ((status & DMAC_CH_INT_STAT_SUSPEND_MASK(1)) ||
        (status & DMAC_CH_INT_STAT_SRC_SUSPEND_MASK(1)))
        chan_status = DMA_PAUSED;

    return chan_status;
}
