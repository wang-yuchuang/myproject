#include <stdio.h>
#include <stdlib.h>
#include <types.h>
#include <param.h>
#include <string.h>
#include "CLI_type.h"
#include <regs_base.h>
#include <reg.h>
#include "board.h"
#include <scr_hw.h>
#include <scr_cfg.h>
#include <driver/gpio.h>
#include <driver/taishan/pinctrl/taishan_pinctrl.h>
#include <driver/taishan/sdrv_acmp_drv.h>
#include <driver/taishan/xtrg/sdrv_xtrg_drv.h>
#include <sdrv_xtrg.h>

/* ACMP_NUM: 1 - 4 */
#define ACMP_NUM(n) n

static acmp_dev_t g_acmp1_dev;
extern pinctrl_t g_pinctrl_safety;
extern gpio_dev_t g_gpio_safety;

static int acmp_callback_test(acmp_dev_t *dev, uint32_t state)
{
    printf("acmp_callback dev->base 0x%x, state 0x%x\n", dev->base, state);

    return 0;
}

/* Totally 4 csel, csel_idx: 0 - 3 */
static sdrv_acmp_csel_cfg_t csel_cfg[] = {
    {
        .csel_idx = ACMP_CSEL_0,
        .amux_setup = 2,
        .prop_delay = 2,
        .rbw = 8,
        .fbw = 8,
        .sample_en = false,
        .mask_en = false,
        .pol_adj = ACMP_POL_ADJ_DIRECTLY,
        .sync_en = false,

        .refsel = ACMP_SEL_REFP1,
        .sdsel = ACMP_SEL_SINGLE,
        .hcfg = ACMP_HCFG_50_mV,
        .dain = 128,
    },

    {
        .csel_idx = ACMP_CSEL_1,
        .amux_setup = 2,
        .prop_delay = 2,
        .rbw = 0,
        .fbw = 0,
        .sample_en = false,
        .mask_en = false,
        .pol_adj = ACMP_POL_ADJ_DIRECTLY,
        .sync_en = false,

        .refsel = ACMP_SEL_REFP2,//refsel no use in diff mode
        .sdsel = ACMP_SEL_DIFF,
        .hcfg = ACMP_HCFG_NO,
        .dain = 30,
    },
};

/* Totally 32 index, idx: 0 - 31*/
static sdrv_acmp_index_cfg_t index_cfg[] = {
    {
        .idx   = ACMP_INDEX_0,
        .amsel = ACMP_SINGLE_CH_SEL(5, ACMP_CH_POLAR_NEG, 2),//single mode, GPIO_A4
        .csel  = ACMP_CSEL_0,
        .ie    = true,
        .uie   = true,
    },

    {
        .idx   = ACMP_INDEX_10,
        .amsel = ACMP_DIFF_CH_SEL(5, 5, 3),//diff mode, ch_p: GPIO_A7, ch_n: GPIO_A6
        .csel  = ACMP_CSEL_0,
        .ie    = true,
        .uie   = true,
    },
};

static acmp_config_t acmp1_config = {
    .base = APB_ACMP1_BASE,
    .irq = ACMP1_O_ACMP_INTR_NUM,
    .mode = ACMP_HT_MODE,
    .csel_cfg = csel_cfg,
    .csel_num = sizeof(csel_cfg) / sizeof(sdrv_acmp_csel_cfg_t),
    .index_cfg = index_cfg,
    .index_num = sizeof(index_cfg) / sizeof(sdrv_acmp_index_cfg_t),

    /* IT mode params */
    .tg_start = 1,
    .qstart = 0,
    .qend = 10,
    .cur_entry_pt = 0,
    .timer_compare_val = 50,
    .timer_terminal_val = 60,
};

static const scr_signal_t apda_scr[] = {
    SCR_SF_ANA_SF_CFG_APD_A_A4_CTRL_3_0,
    SCR_SF_ANA_SF_CFG_APD_A_A6_CTRL_3_0,
    SCR_SF_ANA_SF_CFG_APD_A_A7_CTRL_3_0,
};

static void xtrg_trg_acmp(int index, int acmp_num)
{
    xtrg_dev_t xtrg_dev = {.base = APB_XTRG_BASE};

    //1: config the ssig to select the swt
    xtrg_sse5_config_t xtrg_sse5_config;
    memset(&xtrg_sse5_config, 0, sizeof(xtrg_sse5_config));
    xtrg_sse5_config.in[0].sig = XTRG_SWT(1);
    xtrg_sse5_config.in[0].edge_detect = 0;
    xtrg_sse5_config.in[0].edge = XTRG_POSEDGE;
    xtrg_sse5_config.ssm_sel = XTRG_SSM0;
    xtrg_sse5_config.synthesis_val = 2;
    sdrv_xtrg_sse5_enable(&xtrg_dev, &xtrg_sse5_config, true);

    //2: config the xtrg register
    xtrg_tmux_indrt_config_t xtrg_tmux_indrt_config;
    xtrg_tmux_indrt_config.trigger_sig = XTRG_SWT(0);
    xtrg_tmux_indrt_config.tmux0_indrt_sel = 0;
    xtrg_tmux_indrt_config.term_tid = 0x00;

    for (int i = 0; i < 8; i++) {
        xtrg_tmux_indrt_config.target_lut[0] = 1 << (2 + acmp_num);
    }

    xtrg_tmux_indrt_config.term_val = 0x7;
    xtrg_tmux_indrt_config.tid_pool[0] = index;
    xtrg_tmux_indrt_config.tid_pool[1] = index;
    sdrv_xtrg_tmux0_indrt_config(&xtrg_dev, &xtrg_tmux_indrt_config);

    //3: start the trigger
    xtrg_swt_config_t xtrg_swt_config;
    xtrg_swt_config.swt = XTRG_SWT1;
    xtrg_swt_config.event = XTRG_SWT_EVENT_PULSE;
    xtrg_swt_config.width = 2;
    sdrv_xtrg_swt_config(&xtrg_dev, &xtrg_swt_config);
}

static int acmp_test(int argc, char *argv[])
{
    int mode = ACMP_HT_MODE;

    if (argc > 0) {
        mode = atoi(argv[0]);
    }

    if (mode) {
        printf("IT mode\n");
    }
    else {
        printf("HT mode\n");
    }

    // config IO
    for (int i = 0; i < ARRAY_SIZE(apda_scr); i++) {
        /* APD_A ctrl */
        scr_set(&g_scr_sf, &apda_scr[i], 1);
    }

    pinctrl_config(&g_pinctrl_safety, GPIO_A4, PINCTRL_CONFIG_MODE_SELECT, PINCTRL_MODE_SELECT_NORMAL);
    pinctrl_config(&g_pinctrl_safety, GPIO_A6, PINCTRL_CONFIG_MODE_SELECT, PINCTRL_MODE_SELECT_NORMAL);
    pinctrl_config(&g_pinctrl_safety, GPIO_A7, PINCTRL_CONFIG_MODE_SELECT, PINCTRL_MODE_SELECT_NORMAL);

    //init acmp
    acmp1_config.mode = mode;
    acmp_init(&g_acmp1_dev, &acmp1_config, acmp_callback_test);

    // start acmp
    acmp_start(&g_acmp1_dev, true);

    // ht mode trigger
    if (mode == ACMP_HT_MODE) {
        printf("xtrg trigger acmp\n");
        xtrg_trg_acmp(ACMP_INDEX_0, ACMP_NUM(1));
        xtrg_trg_acmp(ACMP_INDEX_10, ACMP_NUM(1));
    }

    return CLI_FALSE;
}

CLI_CMD("acmp_test", "\r\nacmp_test <mode>\r\n", acmp_test);
