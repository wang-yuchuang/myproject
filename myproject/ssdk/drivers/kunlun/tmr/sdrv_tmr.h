/**
 * @file sdrv_tmr.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: semidriver tmr driver.
 *
 * Revision History:
 * -----------------
 */

#ifndef DRV_LLD_SDRV_TMR_H
#define DRV_LLD_SDRV_TMR_H

#include <types.h>
#include <compiler.h>

#define SDRV_TMR_STA                            0x0
#define SDRV_TMR_STA_EN                         0x4
#define SDRV_TMR_SIG_EN                         0x8
#define SDRV_TMR_STA_SHIFT(int_id)              (int_id)
#define SDRV_TMR_STA_MASK(int_id)               (0x1 << SDRV_TMR_STA_SHIFT(int_id))
#define SDRV_TMR_STA_FIFO_OVERRUN_SHIFT(id)     (20 + (id))
#define SDRV_TMR_STA_FIFO_OVERRUN_MASK(id)      (0x1 << SDRV_TMR_STA_FIFO_OVERRUN_SHIFT(id))
#define SDRV_TMR_STA_FIFO_UNDERRUN_SHIFT(id)    (16 + (id))
#define SDRV_TMR_STA_FIFO_UNDERRUN_MASK(id)     (0x1 << SDRV_TMR_STA_FIFO_UNDERRUN_SHIFT(id))
#define SDRV_TMR_STA_CNT_OVF_SHIFT(id)          (8 + (id))
#define SDRV_TMR_STA_CNT_OVF_MASK(id)           (0x1 << SDRV_TMR_STA_CNT_OVF_SHIFT(id))
#define SDRV_TMR_STA_CMP_SHIFT(id)              (4 + (id))
#define SDRV_TMR_STA_CMP_MASK(id)               (0x1 << SDRV_TMR_STA_CMP_SHIFT(id))
#define SDRV_TMR_STA_CPT_SHIFT(id)              (id)
#define SDRV_TMR_STA_CPT_MASK(id)               (0x1 << SDRV_TMR_STA_CPT_SHIFT(id))

#define SDRV_TMR_DMA_CTRL                       0x10
#define SDRV_TMR_DMA_CTRL_WML_SHIFT(id)         (16 + (id) * 4)
#define SDRV_TMR_DMA_CTRL_WML_MASK(id)          (0xf << SDRV_TMR_DMA_CTRL_WML_SHIFT(id))
#define SDRV_TMR_DMA_CTRL_OVF_SEL_SHIFT         13
#define SDRV_TMR_DMA_CTRL_OVF_SEL_MASK          (0x1 << SDRV_TMR_DMA_CTRL_OVF_SEL_SHIFT)
#define SDRV_TMR_DMA_CTRL_CHN_SEL_SHIFT(id)     (5 + (id) * 2)
#define SDRV_TMR_DMA_CTRL_CHN_SEL_MASK(id)      (0x3 << SDRV_TMR_DMA_CTRL_CHN_SEL_SHIFT(id))
#define SDRV_TMR_DMA_CTRL_CHN_SEL_CMP           0
#define SDRV_TMR_DMA_CTRL_CHN_SEL_CPT           1
#define SDRV_TMR_DMA_CTRL_CHN_SEL_OVF           2
#define SDRV_TMR_DMA_CTRL_OVF_EN_SHIFT          4
#define SDRV_TMR_DMA_CTRL_OVF_EN_MASK           (0x1 << SDRV_TMR_DMA_CTRL_OVF_EN_SHIFT)
#define SDRV_TMR_DMA_CTRL_CHN_EN_SHIFT(id)      (id)
#define SDRV_TMR_DMA_CTRL_CHN_EN_MASK(id)       (0x1 << SDRV_TMR_DMA_CTRL_CHN_EN_SHIFT(id))

#define SDRV_TMR_SOFT_RESET                     0x18
#define SDRV_TMR_SOFT_RESET_CHN_SHIFT(id)       (16 + (id))
#define SDRV_TMR_SOFT_RESET_CHN_MASK(id)        (0x1 << SDRV_TMR_SOFT_RESET_CHN_SHIFT(id))

#define SDRV_TMR_FIFO_STA                       0x1c
#define SDRV_TMR_FIFO_STA_FULL_SHIFT(id)        ((id) * 8)
#define SDRV_TMR_FIFO_STA_FULL_MASK(id)         (0x1 << SDRV_TMR_FIFO_STA_FULL_SHIFT(id))
#define SDRV_TMR_FIFO_STA_EMPTY_SHIFT(id)       (1 + (id) * 8)
#define SDRV_TMR_FIFO_STA_EMPTY_MASK(id)        (0x1 << SDRV_TMR_FIFO_STA_EMPTY_SHIFT(id))
#define SDRV_TMR_FIFO_STA_ENTRIES_SHIFT(id)     (0x2 + (id) * 8)
#define SDRV_TMR_FIFO_STA_ENTRIES_MASK(id)      (0x1f << SDRV_TMR_FIFO_STA_ENTRIES_SHIFT(id))
#define SDRV_TMR_FIFO_STA_ERR_SHIFT(id)         (7 + (id) * 8)
#define SDRV_TMR_FIFO_STA_ERR_MASK(id)          (0x1 << SDRV_TMR_FIFO_STA_ERR_SHIFT(id))

#define SDRV_TMR_CLK_CFG                        0x20
#define SDRV_TMR_CLK_CFG_DIV_NUM_SHIFT          0
#define SDRV_TMR_CLK_CFG_DIV_NUM_MASK           (0xffff << SDRV_TMR_CLK_CFG_DIV_NUM_SHIFT)
#define SDRV_TMR_CLK_CFG_SRC_CLK_SEL_SHIFT      16
#define SDRV_TMR_CLK_CFG_SRC_CLK_SEL_MASK       (0x3 << SDRV_TMR_CLK_CFG_SRC_CLK_SEL_SHIFT)

#define SDRV_TMR_CNT_CFG                        0x24
#define SDRV_TMR_CNT_CFG_CASCADE_MODE_SHIFT     6
#define SDRV_TMR_CNT_CFG_CASCADE_MODE_MASK      (0x1 << SDRV_TMR_CNT_CFG_CASCADE_MODE_SHIFT)
#define SDRV_TMR_CNT_CFG_RLD_SHIFT(id)          (id)
#define SDRV_TMR_CNT_CFG_RLD_MASK(id)           (0x1 << SDRV_TMR_CNT_CFG_RLD_SHIFT(id))
#define SDRV_TMR_CNT_CFG_RST_EN_SHIFT(id)       (8 + (id))
#define SDRV_TMR_CNT_CFG_RST_EN_MASK(id)        (0x1 << SDRV_TMR_CNT_CFG_RST_EN_SHIFT(id))

#define SDRV_TMR_CNT_OVF(id)                    (0x28 + (id) * 4)
#define SDRV_TMR_CNT_VAL(id)                    (0x40 + (id) * 4)
#define SDRV_TMR_CNT_INIT(id)                   (0xb0 + (id) * 4)

#define SDRV_TMR_CMP_UPT(id)                    (0x60 + (id) * 16)
#define SDRV_TMR_CMP0_VAL(id)                   (0x64 + (id) * 16)
#define SDRV_TMR_CMP1_VAL(id)                   (0x68 + (id) * 16)

#define SDRV_TMR_FIFO_VAL(id)                   (0xa0 + (id) * 4)

#define SDRV_TMR_CPT_FLT                        0xc8
#define SDRV_TMR_CPT_FLT_WID_SHIFT(id)          (4 + (id) * 8)
#define SDRV_TMR_CPT_FLT_WID_MASK(id)           (0xf << SDRV_TMR_CPT_FLT_WID_SHIFT(id))
#define SDRV_TMR_CPT_FLT_DIS_SHIFT(id)          ((id) * 8)
#define SDRV_TMR_CPT_FLT_DIS_MASK(id)           (0x1 << SDRV_TMR_CPT_FLT_DIS_SHIFT(id))

#define SDRV_TMR_SSE_CTRL                       0xcc
#define SDRV_TMR_SSE_CTRL_CMP_SEL_SHIFT(id)     (8 + (id) * 2)
#define SDRV_TMR_SSE_CTRL_CMP_SEL_MASK(id)      (0x3 << SDRV_TMR_SSE_CTRL_CMP_SEL_SHIFT(id))
#define SDRV_TMR_SSE_CTRL_CMP_EN_SHIFT(id)      (4 + (id))
#define SDRV_TMR_SSE_CTRL_CMP_EN_MASK(id)       (0x1 << SDRV_TMR_SSE_CTRL_CMP_EN_SHIFT(id))
#define SDRV_TMR_SSE_CTRL_CPT_EN_SHIFT(id)      (id)
#define SDRV_TMR_SSE_CTRL_CPT_EN_MASK(id)       (0x1 << SDRV_TMR_SSE_CTRL_CPT_EN_SHIFT(id))

#define SDRV_TMR_CPT_CFG(id)                    (0x100 + (id) * 4)
#define SDRV_TMR_CPT_CFG_TRIG_MODE_SHIFT        5
#define SDRV_TMR_CPT_CFG_TRIG_MODE_MASK         (0x3 << SDRV_TMR_CPT_CFG_TRIG_MODE_SHIFT)
#define SDRV_TMR_CPT_CFG_CNT_SEL_SHIFT          3
#define SDRV_TMR_CPT_CFG_CNT_SEL_MASK           (0x3 << SDRV_TMR_CPT_CFG_CNT_SEL_SHIFT)
#define SDRV_TMR_CPT_CFG_DUAL_MODE_SHIFT        2
#define SDRV_TMR_CPT_CFG_DUAL_MODE_MASK         (0x1 << SDRV_TMR_CPT_CFG_DUAL_MODE_SHIFT)
#define SDRV_TMR_CPT_CFG_SINGLE_MODE_SHIFT      1
#define SDRV_TMR_CPT_CFG_SINGLE_MODE_MASK       (0x1 << SDRV_TMR_CPT_CFG_SINGLE_MODE_SHIFT)
#define SDRV_TMR_CPT_CFG_EN_SHIFT               0
#define SDRV_TMR_CPT_CFG_EN_MASK                (0x1 << SDRV_TMR_CPT_CFG_EN_SHIFT)

#define SDRV_TMR_CMP_CFG(id)                    (0x110 + (id) * 4)
#define SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_SHIFT   24
#define SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_MASK    (0xff << SDRV_TMR_CMP_CFG_CMP1_PULSE_WID_SHIFT)
#define SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_SHIFT   16
#define SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_MASK    (0xff << SDRV_TMR_CMP_CFG_CMP0_PULSE_WID_SHIFT)
#define SDRV_TMR_CMP_CFG_FRC_LOW_SHIFT          15
#define SDRV_TMR_CMP_CFG_FRC_HIGH_SHIFT         14
#define SDRV_TMR_CMP_CFG_FRC_MASK               (0x3 << SDRV_TMR_CMP_CFG_FRC_HIGH_SHIFT)
#define SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_SHIFT    7
#define SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_MASK     (0x7 << SDRV_TMR_CMP_CFG_CMP1_OUT_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_SHIFT    4
#define SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_MASK     (0x7 << SDRV_TMR_CMP_CFG_CMP0_OUT_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_CNT_SEL_SHIFT          3
#define SDRV_TMR_CMP_CFG_CNT_SEL_MASK           (0x1 << SDRV_TMR_CMP_CFG_CNT_SEL_SHIFT)
#define SDRV_TMR_CMP_CFG_DUAL_MODE_SHIFT        2
#define SDRV_TMR_CMP_CFG_DUAL_MODE_MASK         (0x1 << SDRV_TMR_CMP_CFG_DUAL_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_SINGLE_MODE_SHIFT      1
#define SDRV_TMR_CMP_CFG_SINGLE_MODE_MASK       (0x1 << SDRV_TMR_CMP_CFG_SINGLE_MODE_SHIFT)
#define SDRV_TMR_CMP_CFG_EN_SHIFT               0
#define SDRV_TMR_CMP_CFG_EN_MASK                (0x1 << SDRV_TMR_CMP_CFG_EN_SHIFT)

#define SDRV_TMR_MAX_OVF_VAL                    0xffffffff

/* sdrv tmr id */
#define SDRV_TMR_G0                             0
#define SDRV_TMR_G1                             1
#define SDRV_TMR_CHN_A                          2
#define SDRV_TMR_CHN_B                          3
#define SDRV_TMR_CHN_C                          4
#define SDRV_TMR_CHN_D                          5

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* DMA cfg */
typedef struct sdrv_tmr_lld_dma_cfg {
    uint8_t                 dma_enable;
    uint8_t                 fifo_wml;
} sdrv_tmr_lld_dma_cfg_t;

/* sdrv cmp cfg */
typedef struct sdrv_tmr_cmp_cfg {
    uint8_t                 cmp_cnt_sel;
    uint8_t                 cmp0_out_mode;
    uint8_t                 cmp1_out_mode;
    uint8_t                 cmp0_pulse_width;
    uint8_t                 cmp1_pulse_width;
    uint8_t                 dual_cmp_mode;
    uint8_t                 oneshot_mode;
    uint8_t                 cmp_rst_en;
    uint8_t                 sse_enable;
    uint8_t                 force_out;
    uint8_t                 force_level;
    sdrv_tmr_lld_dma_cfg_t  dma_cfg;
    uint8_t                 cmp_int_en;
    uint8_t                 underrun_int_en;
} sdrv_tmr_lld_cmp_cfg_t;

/* sdrv cmp cfg */
typedef struct sdrv_tmr_lld_cpt_cfg {
    uint8_t                 cpt_cnt_sel;
    uint8_t                 trig_mode;
    uint8_t                 dual_cpt_mode;
    uint8_t                 oneshot_mode;
    uint8_t                 filter_dis;
    uint8_t                 filter_width;
    uint32_t                first_cpt_rst_en;
    uint8_t                 sse_enable;
    uint8_t                 sse_cmp_chn;
    sdrv_tmr_lld_dma_cfg_t  dma_cfg;
    uint8_t                 cpt_int_en;
    uint8_t                 overrun_int_en;
} sdrv_tmr_lld_cpt_cfg_t;

/**
 * @brief sdrv tmr init.
 *
 * @param[in] base sdrv tmr address
 * @param[in] clk clk src
 * @param[in] div clk div
 */
void sdrv_tmr_lld_init(uint32_t base, uint32_t clk, uint32_t div);

/**
 * @brief sdrv tmr set cascase mode.
 *
 * @param[in] base sdrv tmr address
 * @param[in] cascase_mode cascase mode
 */
void sdrv_tmr_lld_set_cascase_mode(uint32_t base, bool cascase_mode);

/**
 * @brief sdrv tmr set timer start.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 * @param[in] time_out time_out time out value(count)
 */
void sdrv_tmr_lld_timer_start(uint32_t base, uint8_t id, uint32_t count);

/**
 * @brief sdrv tmr stop.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 */
void sdrv_tmr_lld_timer_stop(uint32_t base, uint8_t id);

/**
 * @brief sdrv tmr get current cnt value.
 *
 * @param[in] base sdrv tmr address
 * @param[in] id tmr id
 * @return current time(count)
 */
uint32_t sdrv_tmr_lld_get_cur_cnt(uint32_t base, uint8_t id);

/**
 * @brief sdrv tmr get glb cnt value.
 *
 * @param[in] dev tmr dev
 * @return glb cnt value(count)
 */
uint64_t sdrv_tmr_lld_get_glb_cnt(uint32_t base);

/**
 * @brief sdrv tmr get ovf status and clear.
 *
 * @param[in] tmr dev base
 * @return ovf status bitmap
 */
uint32_t sdrv_tmr_lld_get_int_status(uint32_t base);

/**
 * @brief sdrv tmr func cmp config.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] cmp_cfg func cmp config
 */
void sdrv_tmr_lld_cmp_init(uint32_t base, uint8_t chn_id,
                           const sdrv_tmr_lld_cmp_cfg_t *cmp_cfg);

/**
 * @brief sdrv tmr func compare set val.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] cmp_val0 func cmp val0
 * @param[in] cmp_val1 func cmp val1
 */
void sdrv_tmr_lld_set_cmp_val(uint32_t base, uint8_t chn_id,
                              uint32_t cmp_val0, uint32_t cmp_val1);

/**
 * @brief sdrv tmr func cmp start.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] start_fg start or stop cmp(0-stop, 1-start)
 */
void sdrv_tmr_lld_cmp_ctrl(uint32_t base, uint8_t chn_id,
                           bool start_fg);

/**
 * @brief sdrv tmr func cmp force out.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] enable enable force out(0-disable, 1-enable)
 * @param[in] level out level(0-low, 1-high)
 */
void sdrv_tmr_lld_cmp_forceout(uint32_t base, uint8_t chn_id,
                               bool enable, bool level);

/**
 * @brief sdrv tmr func cpt config.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] cpt_cfg func cpt config
 */
void sdrv_tmr_lld_cpt_init(uint32_t base, uint8_t chn_id,
                           const sdrv_tmr_lld_cpt_cfg_t *cpt_cfg);

/**
 * @brief sdrv tmr func cpt start.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] start_fg start or stop cpt(0-stop, 1-start)
 */
void sdrv_tmr_lld_cpt_ctrl(uint32_t base, uint8_t chn_id,
                           bool start_fg);

/**
 * @brief sdrv tmr cpt filter init.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] flt_enable flt enalbe(1-enable, 0-disable)
 * @param[in] flt_width flt width
 */
void sdrv_tmr_lld_cpt_filter_init(uint32_t base, uint8_t chn_id,
                                  uint8_t flt_dis, uint8_t flt_width);

/**
 * @brief sdrv tmr get fifo value.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @return fifo val
 */
uint32_t sdrv_tmr_lld_get_fifo_val(uint32_t base, uint8_t chn_id);

/**
 * @brief sdrv tmr set fifo value.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] val set value
 */
void sdrv_tmr_lld_set_fifo_val(uint32_t base, uint8_t chn_id, uint32_t val);

/**
 * @brief sdrv tmr sse cmp init.
 *
 * @param[in] base dev base
 * @param[in] chn_id cmp channel id
 * @param[in] cmp_enable enable cmp sse(0-disable, 1-enable)
 */
void sdrv_tmr_lld_sse_cmp_init(uint32_t base, uint8_t chn_id,
                               bool cmp_enable);

/**
 * @brief sdrv tmr sse cpt init.
 *
 * @param[in] base dev base
 * @param[in] chn_id cpt channel id
 * @param[in] cpt_enable enable cpt sse(0-disable, 1-enable)
 * @param[in] cmp_chn cmp channel id
 */
void sdrv_tmr_lld_sse_cpt_init(uint32_t base, uint8_t chn_id,
                               bool cpt_enable, uint8_t cmp_chn);

/**
 * @brief sdrv tmr ovf dma init.
 *
 * @param[in] base dev base
 * @param[in] ovf_sel ovf sel
 */
void sdrv_tmr_lld_glb_dma_init(uint32_t base, uint8_t ovf_sel);

/**
 * @brief sdrv tmr func dma init.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @param[in] sel func sel
 * @param[in] fifo_wml fifo wml
 */
void sdrv_tmr_lld_func_dma_init(uint32_t base, uint8_t chn_id,
                                uint8_t sel, uint8_t fifo_wml);

/**
 * @brief sdrv tmr get fifo num.
 *
 * @param[in] base dev base
 * @param[in] chn_id func channel id
 * @return fifo num
 */
uint8_t sdrv_tmr_lld_get_fifo_num(uint32_t base, uint8_t chn_id);

/**
 * @brief sdrv tmr func soft reset.
 *
 * @param[in] base dev base
 * @param[in] chn_id channel id
 */
void sdrv_tmr_lld_func_reset(uint32_t base, uint8_t chn_id);

/**
 * @brief sdrv tmr get ptp time.
 *
 * @param[in] base ptp tmr address
 * @param[in] ptp reg offset
 * @return ptp time
 */
uint32_t sdrv_tmr_lld_get_ptp_time(uint32_t base, uint8_t offset);


/**
 * @brief sdrv tmr get ptp  sta.
 *
 * @param[in] base ptp tmr address
 * @param[in] ptp reg offset
 * @return 1:ptp enable  0:ptp disable
 */
uint8_t sdrv_tmr_lld_get_ptp_enable(uint32_t base, uint8_t offset);

__END_CDECLS

#endif

#endif
