/**
 * @file serdes_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes config implement file
 *
 * Revision History:
 * -----------------
*/
#ifdef CONFIG_SERDES
#include <types.h>
#include <debug.h>
#include <driver/kunlun/serdes/serdes.h>
#include <driver/kunlun/serdes/serdes_vendor.h>

#include "i2c_cfg.h"
#include "serdes_cfg.h"

#define SERDES_CONFIG_REDUNDANCY_MAX    5

serdes_dev_t* g_serdes_dev[SERDES_DEV_MAX];

static sdrv_serdes_handle s_serdes_drv[SERDES_DEV_MAX];

serdes_config_t s_dsi1_cfg = {
    .name = "dsi1",
    .ser_type = TI941_DUAL,
    .des_type = TI948,
    .ser_addr = 0x0c,
    .des_addr = 0x2c,
    .i2c_dev = &g_i2c16_adap_dev,
    .serdes_probe = ti9xx_serdes_probe,
};

serdes_config_t s_dsi2_a01_cfg = {
    .name = "dsi2_a01",
    .ser_type = TI941_DUAL,
    .des_type = TI948,
    .ser_addr = 0x0d,
    .des_addr = 0x3c,
    .i2c_dev = &g_i2c16_adap_dev,
    .serdes_probe = ti9xx_serdes_probe,
};

serdes_config_t s_dsi2_a02_a03_cfg = {
    .name = "dsi2_a02/03",
    .ser_type = TI941_DUAL,
    .des_type = TI948,
    .ser_addr = 0x1a,
    .des_addr = 0x3c,
    .i2c_dev = &g_i2c16_adap_dev,
    .serdes_probe = ti9xx_serdes_probe,
};

serdes_config_t s_lvds4_cfg = {
    .name = "lvds4",
    .ser_type = TI947_SINGLE,
    .des_type = TI948,
    .ser_addr = 0x1a,
    .des_addr = 0x2c,
    .i2c_dev = &g_i2c15_adap_dev,
    .serdes_probe = ti9xx_serdes_probe,
};

static serdes_config_t* s_serdes_cfg_tbl[SERDES_DEV_MAX][SERDES_CONFIG_REDUNDANCY_MAX] = {
    [SERDES_DEV_DSI1] = {
        [0] = &s_dsi1_cfg,
        [1] = NULL
    },
    [SERDES_DEV_DSI2] = {
        [0] = &s_dsi2_a02_a03_cfg,
        [1] = &s_dsi2_a01_cfg,
        [2] = NULL
    },
    [SERDES_DEV_LVDS4] = {
        [0] = &s_lvds4_cfg,
        [1] = NULL
    }
};


int32_t board_serdes_init()
{
    serdes_dev_t* dev = NULL;
    for (int32_t serdes_cnt = 0; serdes_cnt < SERDES_DEV_MAX; serdes_cnt++) {

        uint8_t cfg_cnt = 0;
        dev = NULL;

        while (cfg_cnt < SERDES_CONFIG_REDUNDANCY_MAX &&
                s_serdes_cfg_tbl[serdes_cnt][cfg_cnt] != NULL) {
            if (s_serdes_cfg_tbl[serdes_cnt][cfg_cnt]->serdes_probe != NULL) {
                dev = s_serdes_cfg_tbl[serdes_cnt][cfg_cnt]->serdes_probe(
                                &s_serdes_drv[serdes_cnt]
                                , s_serdes_cfg_tbl[serdes_cnt][cfg_cnt]);
                ssdk_printf(SSDK_EMERG, "probe dev for %s \n", s_serdes_cfg_tbl[serdes_cnt][cfg_cnt]->name);
                if (dev != NULL) {
                    break;
                }
            }
            cfg_cnt++;
        }
        g_serdes_dev[serdes_cnt] = dev;
    }

    return 0;
}

#endif /* end CONFIG_SERDES */
/* end of file serdes_cfg.c */
