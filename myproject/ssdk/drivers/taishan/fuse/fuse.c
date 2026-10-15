/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#include <compiler.h>
#include <debug.h>
#include <param.h>
#include <reg.h>
#include <string.h>

#include "driver/taishan/fuse/fuse.h"

#include "fuse_ctrl_reg.h"
#include "regs_base.h"

#if !defined(FUSE0_OFFSET)
#define FUSE0_OFFSET 0x1000
#endif

#define FUSE_PROG_KEY 0x9458u

#define IS_RED_FUSE(i) (((i) < 4u) || (((i) >= 176u) && ((i) < 200u)))
#define FUSE_INDEX_MAX 199u
#define FUSE_INDEX_RAW_MAX 255

#define FUSE_TIMING_CAL(fld, freq) (((fld) * (freq_mhz)) / 150 + 1u)

uint32_t fuse_read(uint32_t index)
{
    uint32_t base = APB_SCR_SF_BASE;

    return readl(base + FUSE0_OFFSET + index * 4);
}

void fuse_set_sticky_bit(fuse_acc_domain_e dom, uint32_t id)
{
    uint32_t b = APB_EFUSEC_BASE;

    uint32_t sticky_off = SW_STICKY_OFF + (id / 32) * 4;

    uint32_t v = readl(b + sticky_off);
    v |= (0x01u << (id % 32));
    writel(v, b + sticky_off);
}

uint32_t fuse_get_bank_lock(fuse_acc_domain_e dom, uint32_t bank)
{
    uint32_t b = APB_EFUSEC_BASE;

    uint32_t lock_off = BANK_LOCK0_OFF + (bank / 8) * 4;

    uint32_t v = readl(b + lock_off);
    v &= (0xFu << ((bank % 8) * 4));
    v = v >> ((bank % 8) * 4);

    uint32_t lock_off_fuse = FUSE0_OFFSET + (bank / 8) * 4;

    uint32_t vfuse = readl(b + lock_off_fuse);
    vfuse &= (0xFu << ((bank % 8) * 4));
    vfuse = vfuse >> ((bank % 8) * 4);

    return (v | vfuse);
}

void fuse_lock_bank(fuse_acc_domain_e dom, uint32_t bank, uint32_t lock_bits)
{
    uint32_t b = APB_EFUSEC_BASE;
    uint32_t lock_off = BANK_LOCK0_OFF + (bank / 8) * 4;

    uint32_t v = readl(b + lock_off);
    v &= ~(0xFu << ((bank % 8) * 4));
    v |= (lock_bits << ((bank % 8) * 4));
    writel(v, b + lock_off);
}

void fuse_ctl_cfg_timing(uint32_t freq_mhz)
{
    /* the default timing settings are based on 150MHz. */

    uint32_t b = APB_EFUSEC_BASE;

    uint32_t v = readl(b + FUSE_PWR_UP_TIMING_OFF);
    uint32_t fld =
        FUSE_TIMING_CAL(GFV_FUSE_PWR_UP_TIMING_TPENS_CNT(v), freq_mhz);
    v &= ~FM_FUSE_PWR_UP_TIMING_TPENS_CNT;
    v |= FV_FUSE_PWR_UP_TIMING_TPENS_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_PWR_UP_TIMING_TSAS_CNT(v), freq_mhz);
    v &= ~FM_FUSE_PWR_UP_TIMING_TSAS_CNT;
    v |= FV_FUSE_PWR_UP_TIMING_TSAS_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_PWR_UP_TIMING_TPLS_CNT(v), freq_mhz);
    v &= ~FM_FUSE_PWR_UP_TIMING_TPLS_CNT;
    v |= FV_FUSE_PWR_UP_TIMING_TPLS_CNT(fld);
    writel(v, b + FUSE_PWR_UP_TIMING_OFF);

    v = readl(b + FUSE_STB_TIMING_OFF);
    fld = FUSE_TIMING_CAL(GFV_FUSE_STB_TIMING_TSTB_CNT(v), freq_mhz);
    writel(fld, b + FUSE_STB_TIMING_OFF);

    v = readl(b + FUSE_IPEN_RD_TIMING_OFF);
    fld = FUSE_TIMING_CAL(GFV_FUSE_IPEN_RD_TIMING_TIPEN_CNT(v), freq_mhz);
    v &= ~FM_FUSE_IPEN_RD_TIMING_TIPEN_CNT;
    v |= FV_FUSE_IPEN_RD_TIMING_TIPEN_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_IPEN_RD_TIMING_TASH_CNT(v), freq_mhz);
    v &= ~FM_FUSE_IPEN_RD_TIMING_TASH_CNT;
    v |= FV_FUSE_IPEN_RD_TIMING_TASH_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_IPEN_RD_TIMING_TASP_CNT(v), freq_mhz);
    v &= ~FM_FUSE_IPEN_RD_TIMING_TASP_CNT;
    v |= FV_FUSE_IPEN_RD_TIMING_TASP_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_IPEN_RD_TIMING_TAS_CNT(v), freq_mhz);
    v &= ~FM_FUSE_IPEN_RD_TIMING_TAS_CNT;
    v |= FV_FUSE_IPEN_RD_TIMING_TAS_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_IPEN_RD_TIMING_TCS_CNT(v), freq_mhz);
    v &= ~FM_FUSE_IPEN_RD_TIMING_TCS_CNT;
    v |= FV_FUSE_IPEN_RD_TIMING_TCS_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_IPEN_RD_TIMING_TRD_CNT(v), freq_mhz);
    v &= ~FM_FUSE_IPEN_RD_TIMING_TRD_CNT;
    v |= FV_FUSE_IPEN_RD_TIMING_TRD_CNT(fld);
    writel(v, b + FUSE_IPEN_RD_TIMING_OFF);

    v = readl(b + FUSE_PG_TIMING_OFF);
    fld = FUSE_TIMING_CAL(GFV_FUSE_PG_TIMING_TPWI_CNT(v), freq_mhz);
    v &= ~FM_FUSE_PG_TIMING_TPWI_CNT;
    v |= FV_FUSE_PG_TIMING_TPWI_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_PG_TIMING_TPM_CNT(v), freq_mhz);
    v &= ~FM_FUSE_PG_TIMING_TPM_CNT;
    v |= FV_FUSE_PG_TIMING_TPM_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_PG_TIMING_TPW_CNT(v), freq_mhz);
    v &= ~FM_FUSE_PG_TIMING_TPW_CNT;
    v |= FV_FUSE_PG_TIMING_TPW_CNT(fld);
    writel(v, b + FUSE_PG_TIMING_OFF);

    v = readl(b + FUSE_REPAIR_TIMING_OFF);
    fld = FUSE_TIMING_CAL(GFV_FUSE_REPAIR_TIMING_TTAH_CNT(v), freq_mhz);
    v &= ~FM_FUSE_REPAIR_TIMING_TTAH_CNT;
    v |= FV_FUSE_REPAIR_TIMING_TTAH_CNT(fld);

    fld = FUSE_TIMING_CAL(GFV_FUSE_REPAIR_TIMING_TTAS_CNT(v), freq_mhz);
    v &= ~FM_FUSE_REPAIR_TIMING_TTAS_CNT;
    v |= FV_FUSE_REPAIR_TIMING_TTAS_CNT(fld);
    writel(v, b + FUSE_REPAIR_TIMING_OFF);
}

/* vio0 to seip and vio1 to xspi, to clear keys there */
void fuse_ctl_trigger_sw_vio(uint32_t msk)
{
    uint32_t b = APB_EFUSEC_BASE;
    uint32_t v = readl(b + VIO0_CFG_OFF);
    v |= 0x01u << 15;
    writel(v, b + VIO0_CFG_OFF);
    v = readl(b + VIO1_CFG_OFF);
    v |= 0x01u << 15;
    writel(v, b + VIO1_CFG_OFF);

    v = readl(b + SW_TRIG_VIO_OFF);
    v |= (msk & FM_SW_TRIG_VIO_SW_TRIG);
    writel(v, b + SW_TRIG_VIO_OFF);
}

uint32_t fuse_sense(uint32_t index, uint32_t *data)
{
    uint32_t b = APB_EFUSEC_BASE;
    uint32_t res = -1;

    do {
        if ((index > FUSE_INDEX_MAX) || (NULL == data)) {
            break;
        }

        uint32_t v = readl(b + FUSE_CTRL_OFF);

        if (v & (BM_FUSE_CTRL_ERROR | BM_FUSE_CTRL_BUSY)) {
            res = -2;
            break;
        }

        v &= ~FM_FUSE_CTRL_ADDR;
        v |= FV_FUSE_CTRL_ADDR(index);

        if (IS_RED_FUSE(index)) {
            v |= BM_FUSE_CTRL_PECCB;
        } else {
            v &= ~BM_FUSE_CTRL_PECCB;
        }

        writel(v, b + FUSE_CTRL_OFF);

        v = readl(b + FUSE_TEST_CFG_OFF);
        v &= ~FM_FUSE_TEST_CFG_PTM;
        writel(v, b + FUSE_TEST_CFG_OFF);

        v = BM_FUSE_TRIG_READ;
        writel(v, b + FUSE_TRIG_OFF);

        while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY)
            ;

        if (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) {
            res = -3;
            break;
        }

        *data = readl(b + READ_FUSE_DATA_OFF);
        res = 0;
    } while (0);

    return res;
}

uint32_t fuse_reload(void)
{
    uint32_t b = APB_EFUSEC_BASE;
    uint32_t res = -1;

    do {
        uint32_t v = readl(b + FUSE_CTRL_OFF);

        if (v & (BM_FUSE_CTRL_ERROR | BM_FUSE_CTRL_BUSY)) {
            break;
        }

        v = BM_FUSE_TRIG_LOAD;
        writel(v, b + FUSE_TRIG_OFF);

        while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY)
            ;

        if (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_ERROR) {
            res = -2;
            break;
        }

        res = 0;
    } while (0);

    return res;
}

uint32_t fuse_program_raw(uint32_t index, uint32_t val, bool ecc)
{
    uint32_t b = APB_EFUSEC_BASE;
    uint32_t res = -1;

    do {
        if (index > FUSE_INDEX_RAW_MAX) {
            break;
        }

        uint32_t v = readl(b + FUSE_CTRL_OFF);

        if (v & (BM_FUSE_CTRL_ERROR | BM_FUSE_CTRL_BUSY)) {
            res = -2;
            break;
        }

        v &= ~(FM_FUSE_CTRL_ADDR | FM_FUSE_CTRL_PROG_KEY);
        v |= FV_FUSE_CTRL_ADDR(index);
        v |= FV_FUSE_CTRL_PROG_KEY(FUSE_PROG_KEY);

        if (ecc) {
            v &= ~BM_FUSE_CTRL_PECCB;
        } else {
            v |= BM_FUSE_CTRL_PECCB;
        }

        writel(v, b + FUSE_CTRL_OFF);

        writel(val, b + PROG_DATA_OFF);

        v = readl(b + FUSE_TEST_CFG_OFF);
        v &= ~FM_FUSE_TEST_CFG_PTM;
        v |= FV_FUSE_TEST_CFG_PTM(2u);
        writel(v, b + FUSE_TEST_CFG_OFF);

        v = BM_FUSE_TRIG_PROG;
        writel(v, b + FUSE_TRIG_OFF);

        while (readl(b + FUSE_CTRL_OFF) & BM_FUSE_CTRL_BUSY)
            ;

        v = readl(b + FUSE_CTRL_OFF);
        v &= ~FM_FUSE_CTRL_PROG_KEY;
        writel(v, b + FUSE_CTRL_OFF);

        /* error bit is 'write 1 to clear' bit */
        if (v & BM_FUSE_CTRL_ERROR) {
            res = -3;
            break;
        }

        res = 0;
    } while (0);

    return res;
}

uint32_t fuse_program(uint32_t index, uint32_t v)
{
    uint32_t res = -1u;

    do {
        if (index > FUSE_INDEX_MAX) {
            break;
        }

        if (IS_RED_FUSE(index)) {
            uint32_t red_pos = 0;

            if (index < 4u) {
                red_pos = 200u + index * 2u;
            } else {
                red_pos = 208u + (index - 176u) * 2u;
            }

            /* shall blow redundancy fuse bits then the main bit */
            if ((0 != fuse_program_raw(red_pos, v, false)) ||
                (0 != fuse_program_raw(red_pos + 1, v, false)) ||
                (0 != fuse_program_raw(index, v, false))) {
                res = -2;
                break;
            }
        } else {
            if (0 != fuse_program_raw(index, v, true)) {
                res = -3;
                break;
            }
        }

        res = 0u;
    } while (0);

    return res;
}

void fuse_part_load(void){
    
    uint32_t part_info=0;
    uint16_t part = 0;
    
    if(0 == fuse_sense(PART_FUSE_ADDRESS,&part_info)){
        part = (uint16_t)(part_info & PART_FUSE_MASK);

        if(part == CONFIG_PART_ID){
            printf("Chip ID is 0x%x\r\n",part);
        }else{
            printf("Chip ID is 0x%x,But SSDK Select ID is 0x%x\r\n",part,CONFIG_PART_ID);
        }
    }
}