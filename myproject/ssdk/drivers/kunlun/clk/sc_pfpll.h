/**
 * @file sc_pfpll.h
 * @brief
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef SC_PFPLL_H_
#define SC_PFPLL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PLL_OUT_DIV_A (0u)
#define PLL_OUT_DIV_B (1u)
#define PLL_OUT_DIV_C (2u)
#define PLL_OUT_DIV_D (3u)

/* PLL index */
#define SC_PFPLL_ID_NUM_LSB  (0u)
#define SC_PFPLL_ID_NUM_VCO  (0u)
#define SC_PFPLL_ID_NUM_ROOT (1u)
#define SC_PFPLL_ID_NUM_DIVA (2u)
#define SC_PFPLL_ID_NUM_DIVB (3u)
#define SC_PFPLL_ID_NUM_DIVC (4u)
#define SC_PFPLL_ID_NUM_DIVD (5u)
#define SC_PFPLL_ID_NUM_MASK (0xFF)

/* Control register */
#define PLL_CTRL                (0x0u)
#define PLL_CTRL_PLLEN          (0u)
#define PLL_CTRL_INT_MODE       (1u)
#define PLL_CTRL_DACEN          (2u)
#define PLL_CTRL_FOUT4PHASEEN   (3u)
#define PLL_CTRL_FOUTPOSTDIVEN  (4u)
#define PLL_CTRL_PLLPOSTCG_EN   (16u)
#define PLL_CTRL_BAPASS         (17u)
#define PLL_CTRL_PLL_DIVA_CG_EN (18u)
#define PLL_CTRL_PLL_DIVB_CG_EN (19u)
#define PLL_CTRL_PLL_DIVC_CG_EN (20u)
#define PLL_CTRL_PLL_DIVD_CG_EN (21u)
#define PLL_CTRL_LOCK           (31u)

/* Divide number register */
#define PLL_DIV               (0x4u)
#define PLL_DIV_REFDIV_LSB    (0u)
#define PLL_REFDIV_MIN        (1u)
#define PLL_REFDIV_MAX        (63u)
#define PLL_DIV_REFDIV_MASK   (0x3Fu)
#define PLL_DIV_POSTDIV1_LSB  (6u)
#define PLL_DIV_POSTDIV1_MASK (0x7u)
#define PLL_DIV_POSTDIV2_LSB  (9u)
#define PLL_DIV_POSTDIV2_MASK (0x7u)
#define PLL_DIV_REFDIV        (0x0u)
#define PLL_DIV_POSTDIV1      (0x6u)
#define PLL_DIV_POSTDIV2      (0x9u)

/* Feedback divider register. Integer part of feedback division. */
#define PLL_FBDIV           (0x8u)
#define PLL_FBDIV_MINT_MIN  (16u)
#define PLL_FBDIV_MINT_MAX  (640u)
#define PLL_FBDIV_MFRAC_MIN (20u)
#define PLL_FBDIV_MFRAC_MAX (320u)
#define PLL_FBDIV_LSB       (0u)
#define PLL_FBDIV_MASK      (0xFFFu)

/* Fractional portion setting register. Fractional part of feedback division. */
#define PLL_FRAC      (0xCu)
#define PLL_FRAC_LSB  (0u)
#define PLL_FRAC_MASK (0xFFFFFFu)

/* Spread spectrum mode setting register */
#define PLL_SSMOD               (0x18u)
#define PLL_SSMOD_MOD_RESET     (0u)
#define PLL_SSMOD_DISABLE_SSCG  (1u)
#define PLL_SSMOD_RESETPTR      (2u)
#define PLL_SSMOD_DOWNSPREAD    (3u)
#define PLL_SSMOD_SPREAD        (4u)    /*4 ~ 8*/
#define PLL_SSMOD_DIVVAL        (9u)    /*9 ~ 14*/

#define PLL_OUT_DIV_MIN      (0x0u)
#define PLL_OUT_DIV_MAX      (0xfu)
#define PLL_OUT_DIV_NUM_LSB  (0u)
#define PLL_OUT_DIV_NUM_MASK (0xFu)

#define PLL_OUT_DIV_BUSY (15u)
/* Outside divider register 1 */
#define PLL_OUT_DIV_1               (0x20u)
#define PLL_OUT_DIV_1_DIV_NUM_A     0
#define PLL_OUT_DIV_1_DIV_BUSY_A    15
#define PLL_OUT_DIV_1_DIV_NUM_B     16
#define PLL_OUT_DIV_1_DIV_BUSY_B    31

/* Outside divider register 2 */
#define PLL_OUT_DIV_2               (0x24u)
#define PLL_OUT_DIV_2_DIV_NUM_C     0
#define PLL_OUT_DIV_2_DIV_BUSY_C    15
#define PLL_OUT_DIV_2_DIV_NUM_D     16
#define PLL_OUT_DIV_2_DIV_BUSY_D    31

typedef uint32_t pll_rate_t;

int pll_enable(paddr_t pll_base, uint32_t id);

int pll_disable(paddr_t pll_base, uint32_t id);

int pll_prepare(paddr_t base, uint32_t id,
                pll_rate_t src_rate,
                pll_rate_t rate);

pll_rate_t pll_get_rate(paddr_t pll_base, uint32_t id,
                        pll_rate_t src_rate);

int pll_set_rate(paddr_t pll_base, uint32_t id,
                 pll_rate_t src_rate, pll_rate_t rate);

#ifdef __cplusplus
}
#endif

#endif /* SC_PFPLL_H_ */
