/**
 * @file acmp_drv.h
 * @brief semidrive acmp driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef SDRV_ACMP_DRV_H_
#define SDRV_ACMP_DRV_H_

#include <types.h>
#include <spinlock.h>

#define ACMP_CH_POLAR_POS        1u
#define ACMP_CH_POLAR_NEG        0u

typedef struct acmp_dev acmp_dev_t;

/* sdrv acmp mode */
typedef enum {
    ACMP_HT_MODE   = 0x0,
    ACMP_IT_MODE,
} sdrv_acmp_mode_t;

/* sdrv acmp csel num */
typedef enum {
    ACMP_CSEL_0             = 0x0,
    ACMP_CSEL_1,
    ACMP_CSEL_2,
    ACMP_CSEL_3,
} acmp_csel_num_t;

/* sdrv acmp index num */
typedef enum {
    ACMP_INDEX_0            = 0x0,
    ACMP_INDEX_1,
    ACMP_INDEX_2,
    ACMP_INDEX_3,
    ACMP_INDEX_4,
    ACMP_INDEX_5,
    ACMP_INDEX_6,
    ACMP_INDEX_7,
    ACMP_INDEX_8,
    ACMP_INDEX_9,
    ACMP_INDEX_10,
    ACMP_INDEX_11,
    ACMP_INDEX_12,
    ACMP_INDEX_13,
    ACMP_INDEX_14,
    ACMP_INDEX_15,
    ACMP_INDEX_16,
    ACMP_INDEX_17,
    ACMP_INDEX_18,
    ACMP_INDEX_19,
    ACMP_INDEX_20,
    ACMP_INDEX_21,
    ACMP_INDEX_22,
    ACMP_INDEX_23,
    ACMP_INDEX_24,
    ACMP_INDEX_25,
    ACMP_INDEX_26,
    ACMP_INDEX_27,
    ACMP_INDEX_28,
    ACMP_INDEX_29,
    ACMP_INDEX_30,
    ACMP_INDEX_31,
} acmp_index_num_t;

/* Single End Mode */
#define ACMP_SINGLE_CH_SEL(ch, polar, mux)                              \
            (((mux & 0x7u) << 6) | ((polar & 0x1u) << 3) | (ch & 0x7u))
/* Differential Mode */
#define ACMP_DIFF_CH_SEL(ch_p, ch_n, mux)                            \
            (((mux & 0x7u) << 6) | ((ch_p & 0x7u) << 3) | (ch_n & 0x7u))

/* sdrv acmp pol adjust */
typedef enum {
    ACMP_POL_ADJ_DIRECTLY   = 0x0,
    ACMP_POL_ADJ_INVERT,
} acmp_pol_adj_t;

/* sdrv acmp ref sel */
typedef enum {
    ACMP_SEL_REFP1          = 0x0,
    ACMP_SEL_REFP2,
} acmp_reg_sel_t;

/* sdrv acmp sd sel */
typedef enum {
    ACMP_SEL_SINGLE         = 0x0,
    ACMP_SEL_DIFF,
} acmp_sd_sel_t;

/* sdrv acmp hysteresis window configure */
typedef enum {
    ACMP_HCFG_NO            = 0x0,
    ACMP_HCFG_50_mV,
    ACMP_HCFG_100_mV        = 0x3,
} acmp_hcfg_t;


/* sdrv acmp csel configurations */
typedef struct sdrv_acmp_csel_cfg {
    uint8_t  csel_idx   : 2;
    /* acmp ctrl cfg */
    uint8_t  amux_setup;
    uint8_t  prop_delay : 5;
    uint8_t  rbw        : 4;
    uint8_t  fbw        : 4;
    bool     sample_en;
    bool     mask_en;
    bool     pol_adj;
    bool     sync_en;
    /* acmp analog cfg */
    uint8_t  refsel     : 1;
    uint8_t  sdsel      : 1;
    uint8_t  hcfg       : 2;
    uint8_t  dain;
} sdrv_acmp_csel_cfg_t;

/* sdrv acmp index configurations */
typedef struct sdrv_acmp_index_cfg {
    uint8_t  idx;
    uint16_t amsel      : 9;
    uint8_t  csel       : 2;
    bool     ie;
    bool     uie;
} sdrv_acmp_index_cfg_t;

/* acmp callback */
typedef int (*acmp_callback)(acmp_dev_t *dev, uint32_t state);

/* sdrv acmp configurations */
typedef struct acmp_config {
    paddr_t                base;
    int                    irq;
    uint8_t                mode;
    sdrv_acmp_csel_cfg_t   *csel_cfg;
    uint8_t                csel_num;
    sdrv_acmp_index_cfg_t  *index_cfg;
    uint8_t                index_num;
    uint16_t               req_n_dly;
    bool                   qch_dac_en;
    bool                   qch_cmp_en;

    /* IT MODE */
    bool                   tg_start;
    uint8_t                qstart        : 5;
    uint8_t                qend          : 5;
    uint8_t                cur_entry_pt  : 5;
    uint16_t               timer_compare_val;
    uint16_t               timer_terminal_val;
} acmp_config_t;

/* sdrv acmp dev */
typedef struct acmp_dev {
    paddr_t             base;
    int                 irq;
    uint8_t             acmp_idx;
    spin_lock_t         lock;
    acmp_callback       callback;
    void                *priv;
} acmp_dev_t __ALIGNED(4);

/**
 * @brief acmp init
 *
 * @param[in] dev acmp dev
 * @param[in] acmp_cfg sdrv acmp config
 * @param[in] callback acmp callback
 */
void acmp_init(acmp_dev_t *dev, acmp_config_t *acmp_cfg, acmp_callback callback);

/**
 * @brief acmp start
 *
 * @param[in] dev acmp dev
 * @param[in] start acmp start
 */
void acmp_start(acmp_dev_t *dev, bool start);

/**
 * @brief acmp IT mode trigger start
 *
 * @param[in] dev acmp dev
 * @param[in] qstart sdrv acmp trigger start id number for loop
 * @param[in] qend sdrv acmp trigger end id number for loop
 * @param[in] cur_entry_pt sdrv acmp trigger first access id number
 * @param[in] start acmp IT mode trigger start
 */
void acmp_it_trigger_start(acmp_dev_t *dev, uint8_t qstart, uint8_t qend, uint8_t cur_entry_pt,
                           bool start);

/**
 * @brief acmp power ctrl
 *
 * @param[in] dev acmp dev
 * @param[in] down 0 power on (default), 1 power down.
 */
void acmp_power_ctrl(acmp_dev_t *dev, bool down);

/**
 * @brief acmp set callback
 *
 * @param[in] dev acmp dev
 * @param[in] callback acmp callback.
 */
void acmp_set_callback(acmp_dev_t *dev, acmp_callback callback);

/**
 * @brief acmp deinit
 *
 * @param[in] dev acmp dev
 */
void acmp_deinit(acmp_dev_t *dev);

#endif /* SDRV_ACMP_DRV_H_ */
