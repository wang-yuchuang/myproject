/*
 * scr_ip.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: SCR low level driver.
 *
 * Revision History:
 * -----------------
 */
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <driver/irq.h>
#include <irq_num.h>
#include <reg.h>
#include <types.h>

#include "scr_lld.h"

#ifdef CONFIG_SCR_FUSA
#include "scr_fusa_reg.h"
#endif

#define BITFIELD_POS_MSK 0x1Fu
#define UNEXPECTED_CLEAR_STATUS 0x80000000
#define SAFETY_ERR_IRQ_STAT_NUM 0x8
#define SCR_FS_LS_FIELDWIDTH 16

#ifdef CONFIG_SCR_FUSA
typedef void (*scr_callback_t)(uint32_t value, void *user_arg);
static scr_callback_t _scr_callback;
static void *_scr_callback_arg;
#endif

static inline uint32_t _scr_reg(const scr_dev_t *dev,
                                const scr_signal_t *signal)
{
    uint32_t reg_off =
        dev->reg_off[signal->type] + (signal->start_bit >> 5) * 4;
    return dev->base + (reg_off << dev->reg_shift);
}

static inline uint32_t _scr_read_reg(const scr_dev_t *dev,
                                     const scr_signal_t *signal)
{
    return readl(_scr_reg(dev, signal));
}

static inline void _scr_write_reg(const scr_dev_t *dev,
                                  const scr_signal_t *signal, uint32_t val)
{
    writel(val, _scr_reg(dev, signal));
}

uint32_t scr_lld_get(const scr_dev_t *dev, const scr_signal_t *signal)
{
    uint32_t val = _scr_read_reg(dev, signal);
    uint32_t start_bit = signal->start_bit & BITFIELD_POS_MSK;
    return (val >> start_bit) & BIT_MASK(signal->width);
}

bool scr_lld_set(const scr_dev_t *dev, const scr_signal_t *signal,
                 uint32_t value)
{
    uint32_t val = _scr_read_reg(dev, signal);
    bool ret = false;

    if (!scr_lld_is_locked(dev, signal)) {
        uint32_t start_bit = signal->start_bit & BITFIELD_POS_MSK;
        val &= ~(BIT_MASK(signal->width) << start_bit);
        val |= value << start_bit;
        _scr_write_reg(dev, signal, val);
        ret = true;
    }

    return ret;
}

bool scr_lld_lock(const scr_dev_t *dev, const scr_signal_t *signal)
{
    if (scr_is_locked((scr_dev_t *)dev, signal))
        return true;

    uint32_t val = _scr_read_reg(dev, signal);
    bool ret = false;
    uint32_t start_bit = signal->start_bit & BITFIELD_POS_MSK;

    switch (signal->type) {
    case TYPE_L16:
        if (start_bit < 16)
            val |= BIT_MASK(signal->width) << (start_bit + 16);
        else /* sticky bit */
            val |= BIT_MASK(signal->width) << start_bit;

        _scr_write_reg(dev, signal, val);
        ret = true;
        break;

    case TYPE_L31:
        val |= 1ul << 31;
        _scr_write_reg(dev, signal, val);
        ret = true;
        break;

    case TYPE_R16W16:
    case TYPE_RW:
    case TYPE_RO:
    default:
        break;
    }

    return ret;
}

bool scr_lld_is_locked(const scr_dev_t *dev, const scr_signal_t *signal)
{
    uint32_t val = _scr_read_reg(dev, signal);

    bool locked;
    uint32_t start_bit = signal->start_bit & BITFIELD_POS_MSK;

    switch (signal->type) {
    case TYPE_L16:
        if (start_bit < 16)
            locked = val & (1ul << (start_bit + 16));
        else
            locked = val & (1ul << start_bit);

        break;

    case TYPE_L31:
        locked = val & (1ul << 31);
        break;

    /* Read only bits are considered locked.*/
    case TYPE_R16W16:
        locked = start_bit >= 16;
        break;

    case TYPE_RO:
        locked = true;
        break;

    case TYPE_RW:
    default:
        locked = false;
        break;
    }

    return locked;
}
#ifdef CONFIG_SCR_FUSA

#define BITFIELD_POS_MSK 0x1Fu
#define SCR_FUSA_REG_RSSEL_OFFSET                                              \
    (SCR_RW_RS_MAPPING_SELECT_OFF(0) - SCR_RW_OFF(0))
#define SCR_FUSA_REG_EJ_OFFSET (APB_MONITOR_ERR_INJ_EN_OFF - SCR_RW_OFF(0))
#define SCR_FUSA_REG_LS_OFFSET (SCR_RW_LOCK_STEP_ERR_IRQ_OFF(0) - SCR_RW_OFF(0))
#define SCR_FUSA_REG_RW_EJ_OFFSET (SCR_RW_EJ_OFF(0) - SCR_RW_OFF(0))
static inline uint32_t _scr_fs_reg(const scr_dev_t *dev, uint32_t offset,
                                   const scr_signal_t *signal)
{
    uint32_t reg_off =
        dev->reg_off[signal->type] + (signal->start_bit >> 5) * 4 + offset;
    return dev->base + (reg_off << dev->reg_shift);
}

static inline uint32_t _scr_read_fs_reg(const scr_dev_t *dev, uint32_t offset,
                                        const scr_signal_t *signal)
{
    return readl(_scr_fs_reg(dev, offset, signal));
}

static inline void _scr_write_fs_reg(const scr_dev_t *dev, uint32_t offset,
                                     const scr_signal_t *signal, uint32_t val)
{
    writel(val, _scr_fs_reg(dev, offset, signal));
}

uint32_t scr_fusa_ej_get(const scr_dev_t *dev, const scr_signal_t *signal)
{
    uint32_t ej_off = SCR_FUSA_REG_EJ_OFFSET;
    if (dev->reg_off[signal->type] == SCR_RW_OFF(0))
        ej_off = SCR_FUSA_REG_RW_EJ_OFFSET;
    uint32_t val = _scr_read_fs_reg(dev, ej_off, signal);
    uint32_t start_bit = signal->start_bit & BITFIELD_POS_MSK;
    return (val >> start_bit) & BIT_MASK(signal->width);
}

void scr_fusa_ej_set(const scr_dev_t *dev, const scr_signal_t *signal,
                     uint32_t value)
{

    uint32_t ej_off = SCR_FUSA_REG_EJ_OFFSET;
    if (dev->reg_off[signal->type] == SCR_RW_OFF(0))
        ej_off = SCR_FUSA_REG_RW_EJ_OFFSET;

    uint32_t val = _scr_read_fs_reg(dev, ej_off, signal);

    uint32_t start_bit = signal->start_bit & BITFIELD_POS_MSK;
    val &= ~(BIT_MASK(signal->width) << start_bit);
    val |= (value << start_bit);
    _scr_write_fs_reg(dev, SCR_FUSA_REG_EJ_OFFSET, signal, val);
}

void scr_fusa_ej_enable(const scr_dev_t *dev, uint32_t value)
{
    uint32_t typeval = 0;
    uint32_t subval = 0;

    if (value)
        typeval = BM_SCR_RW_L16_L31_W16_ERR_INJ_EN_SCR_R16W16_EJ_EN |
                  BM_SCR_RW_L16_L31_W16_ERR_INJ_EN_SCR_L31_EJ_EN |
                  BM_SCR_RW_L16_L31_W16_ERR_INJ_EN_SCR_L16_EJ_EN |
                  BM_SCR_RW_L16_L31_W16_ERR_INJ_EN_SCR_RW_EJ_EN;
    writel(value, dev->base + SCR_EJ_EN_OFF);

    writel(typeval, dev->base + SCR_RW_L16_L31_W16_ERR_INJ_EN_OFF);

    // 0x3034 write 0xf,0x3040 write 0xaaaa;
    if (value) {
        typeval = BM_APB_LOCK_STEP_EJ_EN_PREADY_EJ_EN |
                  BM_APB_LOCK_STEP_EJ_EN_PWDATA_EJ_EN |
                  BM_APB_LOCK_STEP_EJ_EN_PRDATA_EJ_EN |
                  BM_APB_LOCK_STEP_EJ_EN_PSLVERR_EJ_EN;
        subval = 0xaaaa;
    } else
        typeval = 0;
    writel(typeval, dev->base + APB_LOCK_STEP_EJ_EN_OFF);
    uint32_t val = readl(dev->base + APB_LOCK_STEP_ERR_IRQ_OFF);
    writel(subval | val, dev->base + APB_LOCK_STEP_ERR_IRQ_OFF);
    // 0x3000
    typeval = (value == 1) ? BM_APB_MONITOR_ERR_INJ_EN_ERR_INJ_EN : 0;
    writel(typeval, dev->base + APB_MONITOR_ERR_INJ_EN_OFF);

    // 0x301c
    subval = (value == 1) ? (BM_IRQ_ERR_INJ_SCR_APB_PSLVERR_IRQ_ERR_INJ_EN |
                             BM_IRQ_ERR_INJ_SCR_IRQ_UNC_ERR_INJ_EN |
                             BM_IRQ_ERR_INJ_SCR_IRQ_COR_ERR_INJ_EN)
                          : 0;
    val = readl(dev->base + SCR_IRQ_ERR_INJ_OFF);
    writel(subval, dev->base + SCR_IRQ_ERR_INJ_OFF);
}

uint32_t scr_fusa_chk_lockstep(const scr_dev_t *dev, const scr_signal_t *signal)
{
    uint32_t ret = 0;
    uint32_t val = _scr_read_fs_reg(dev, SCR_FUSA_REG_LS_OFFSET, signal);

    if (val & 0x1)
        ret |= 1;
    else
        ret |= 2;

    val |= 4;
    _scr_write_fs_reg(dev, SCR_FUSA_REG_LS_OFFSET, signal, val);
    val = _scr_read_fs_reg(dev, SCR_FUSA_REG_LS_OFFSET, signal);

    if (val & 0x1)
        ret |= 8;
    else
        ret |= 4;
    return ret;
}

void scr_permission_set(const scr_dev_t *dev, uint32_t did, uint32_t secp,
                        uint32_t rwp)
{
    uint32_t offset = (did < 4) ? did * 0xc : (did & DOMAIN_ID_MSK) * 0xc + 4;
    uint32_t reg = dev->base + offset;

    uint32_t field_off = ((did & DOMAIN_ID_MSK) << 3) + secp;
    uint32_t rdata = readl(reg);
    rdata &= ~(ACCESS_NULL << field_off);
    rdata |= rwp << field_off;
    writel(rdata, dev->base + offset);
    rdata = readl(dev->base + offset);
}

void scr_rs_set(const scr_dev_t *dev, const scr_signal_t *signal, uint32_t did,
                uint32_t en)
{
    uint32_t val = _scr_read_fs_reg(dev, SCR_FUSA_REG_RSSEL_OFFSET, signal);

    val |= ((did << 1) + en);

    _scr_write_fs_reg(dev, SCR_FUSA_REG_RSSEL_OFFSET, signal, val);
}

static int _scr_handler(uint32_t source, void *arg)
{
    scr_dev_t *dev = (scr_dev_t *)arg;
    uint32_t pslverr_off = 0;
    uint32_t errid = 0;
    if (source == SCR_SF_SCR_APB_PSLVERR_INTR_NUM) {
        pslverr_off = SCR_APB_PSLVERR_IRQ_OFF;

        uint32_t status = readl(dev->base + pslverr_off);
        if ((status & BM_SCR_APB_PSLVERR_IRQ_IRQ_STAT) == 1)
            errid |= 1;
        writel(status | BM_SCR_APB_PSLVERR_IRQ_IRQ_STAT_CLR,
               dev->base + pslverr_off);
        status = readl(dev->base + pslverr_off);
        if ((status & BM_SCR_APB_PSLVERR_IRQ_IRQ_STAT) == 0)
            errid |= 0x2;
        if (_scr_callback)
            _scr_callback(errid, _scr_callback_arg);
    }

    return errid;
}

void scr_fusa_enable_it(const scr_dev_t *dev, scr_callback_t cb, void *user_arg)
{
    _scr_callback = cb;
    _scr_callback_arg = user_arg;

    irq_attach(SCR_SF_SCR_APB_PSLVERR_INTR_NUM, _scr_handler, dev);
    irq_enable(SCR_SF_SCR_APB_PSLVERR_INTR_NUM);
}

uint32_t scr_fs_clear_err_status(const scr_dev_t *dev)
{
    uint32_t isr = readl(dev->base + SAFTY_ERR_IRQ_STAT_OFF);
    uint32_t err_idx = 0;

    uint32_t clr_isr = readl(dev->base + SAFTY_ERR_IRQ_STAT_CLR_OFF);
    for (err_idx = 0; err_idx < SAFETY_ERR_IRQ_STAT_NUM; err_idx++) {
        if ((isr >> err_idx) == 1) {
            writel(clr_isr | (0x1 << err_idx),
                   dev->base + SAFTY_ERR_IRQ_STAT_CLR_OFF);
            break; // assume irq_err cannot coexist
        }
    }
    isr = readl(dev->base + SAFTY_ERR_IRQ_STAT_OFF);
    if ((isr >> err_idx) != 0)
        err_idx |= UNEXPECTED_CLEAR_STATUS;
    return err_idx;
}

uint32_t scr_fs_clear_lserr_status(const scr_dev_t *dev)
{
    uint32_t i = 0;
    uint32_t err_id = 0;

    uint32_t isr_ls = readl(dev->base + APB_LOCK_STEP_ERR_IRQ_OFF);
    uint32_t lsoff = APB_PWDATA_LOCK_STEP_EJ_OFF;
    for (i = 0; i < SCR_FS_LS_FIELDWIDTH; i += 4) {
        if (((isr_ls >> i) & 0x1) == 1) {
            writel(0, dev->base + lsoff + i);
            writel(isr_ls | (0x1 << (i + 2)),
                   dev->base + APB_LOCK_STEP_ERR_IRQ_OFF);
            break;
        }
    }
    isr_ls = readl(dev->base + APB_LOCK_STEP_ERR_IRQ_OFF);

    if (((isr_ls >> i) & 0x1) == 1)
        err_id |= UNEXPECTED_CLEAR_STATUS;
    err_id |= i >> 2;
    return err_id;
}

void scr_fs_ej(const scr_dev_t *dev, uint32_t offset, uint32_t data)
{
    uint32_t wdata = 0;
    uint32_t reg = 0;
    if (SCR_IRQ_ERR_INJ_OFF == offset) {
        reg = readl(dev->base + offset);
        reg &= ~(BM_IRQ_ERR_INJ_SCR_IRQ_COR_ERR_INJ |
                 BM_IRQ_ERR_INJ_SCR_IRQ_UNC_ERR_INJ);
        wdata = reg | data;
    } else
        wdata = data;

    writel(wdata, dev->base + offset);

    if (SCR_IRQ_ERR_INJ_OFF == offset)
        wdata = reg;
    else
        wdata = 0;
    if (offset <= APB_PWDATA_LOCK_STEP_EJ_OFF ||
        offset > APB_PREADY_LOCK_STEP_EJ_OFF)
        writel(wdata, dev->base + offset);
}

// Known issue,mask RO/R16W16_RO to avoid endless unc error if sem uncorr is
// enabeld.
void scr_fusa_ro_mask(const scr_dev_t *dev)
{
    uint32_t val = 0;
    val = readl(dev->base + SCR_RO_LOCK_STEP_ERR_IRQ_OFF(0));
    val &= ~BM_SCR_RO_LOCK_STEP_ERR_IRQ_ERR_IRQ_SIG_EN;
    writel(val, dev->base + SCR_RO_LOCK_STEP_ERR_IRQ_OFF(0));

    val = readl(dev->base + SCR_R16W16_LOCK_STEP_ERR_IRQ_OFF(0));
    val &= ~BM_SCR_R16W16_LOCK_STEP_ERR_IRQ_RO_ERR_IRQ_SIG_EN;
    writel(val, dev->base + SCR_R16W16_LOCK_STEP_ERR_IRQ_OFF(0));
}

#endif
