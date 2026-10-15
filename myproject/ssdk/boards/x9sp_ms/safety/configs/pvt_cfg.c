/**
 * @file pvt_cfg.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt config implement file
 *
 * Revision History:
 * -----------------
*/
#ifdef CONFIG_PVT

#include <debug.h>
#include <irq_num.h>
#include <arch/register.h>
#include <regs_base.h>
#include <types.h>

#include "pvt_cfg.h"

/* global pvt device pointer  */
pvt_dev_t* g_pvt_dev_ptr[PVT_DEV_INS_NUM];

/* static sdrv pvt device and config */
static sdrv_pvt_dev_t s_sdrv_pvt_dev[PVT_DEV_INS_NUM];



static sdrv_pvt_config_t s_pvt_cfg[PVT_DEV_INS_NUM] = {
    [PVT_DEV_SAFETY_INS] = {
        .base = APB_PVT_SNS_SAF_BASE,
        .irq_num = {
            [PVT_SENSOR_ALARM_CHAN0] = PVT_SNS_SAF_PVT_INT_0_NUM,
            [PVT_SENSOR_ALARM_CHAN1] = PVT_SNS_SAF_PVT_INT_1_NUM
        },
        .sensor_type = PVT_SENSOR_TYPE_LVT,
        .sensor_mode = PVT_SENSOR_SAMPLE_MODE_TEMP,
        .sensor_clk = 1200000,
        .conversion_time = 500,     /**< 500 us */
        .process_cfg = {
            [PVT_SENSOR_PN_TYPE_LVT] = {
                .ss_process = 116,
                .ff_process = 280,
            },
            [PVT_SENSOR_PN_TYPE_ULVT] = {
                .ss_process = 175,
                .ff_process = 361,
            },
            [PVT_SENSOR_PN_TYPE_SVT] = {
                .ss_process = 69,
                .ff_process = 227,
            }
        },
        .conv_temp2code = {
            .par_num = 5,
            .para_arry = {
                [0] = 2.0705E+02,
                [1] = 3.6384E+00,
                [2] = 6.6309E-03,
                [3] = 2.4759E-05,
                [4] = 1.2569E-08
            }
        },
        .conv_code2temp = {
            .par_num = 5,
            .para_arry = {
                [0] = -6.2861E+01,
                [1] = 3.3408E-01,
                [2] = -1.5089E-04,
                [3] = 1.5608E-08,
                [4] = 1.6034E-11
            }
        },
        .conv_vol2code = {
            .par_num = 2,
            .para_arry = {
                [0] = -856.39,
                [1] = 1675.7,
            }
        },
        .conv_code2vol = {
            .par_num = 2,
            .para_arry = {
                [0] = 5.1106E-01,
                [1] = 5.9677E-04,
            }
        }
    },

    [PVT_DEV_AP_INS] = {
        .base = APB_PVT_SNS_SEC_BASE,
        .irq_num = {
            [PVT_SENSOR_ALARM_CHAN0] = PVT_SNS_SEC_PVT_INT_0_NUM,
            [PVT_SENSOR_ALARM_CHAN1] = PVT_SNS_SEC_PVT_INT_1_NUM
        },
        .sensor_type = PVT_SENSOR_TYPE_LVT,
        .sensor_mode = PVT_SENSOR_SAMPLE_MODE_TEMP,
        .sensor_clk = 1200000,
        .conversion_time = 500,     /**< 500 us */
        .process_cfg = {
            [PVT_SENSOR_PN_TYPE_LVT] = {
                .ss_process = 116,
                .ff_process = 280,
            },
            [PVT_SENSOR_PN_TYPE_ULVT] = {
                .ss_process = 175,
                .ff_process = 361,
            },
            [PVT_SENSOR_PN_TYPE_SVT] = {
                .ss_process = 69,
                .ff_process = 227,
            }
        },
        .conv_temp2code = {
            .par_num = 5,
            .para_arry = {
                [0] = 2.0705E+02,
                [1] = 3.6384E+00,
                [2] = 6.6309E-03,
                [3] = 2.4759E-05,
                [4] = 1.2569E-08
            }
        },
        .conv_code2temp = {
            .par_num = 5,
            .para_arry = {
                [0] = -6.2861E+01,
                [1] = 3.3408E-01,
                [2] = -1.5089E-04,
                [3] = 1.5608E-08,
                [4] = 1.6034E-11
            }
        },
        .conv_vol2code = {
            .par_num = 2,
            .para_arry = {
                [0] = -856.39,
                [1] = 1675.7,
            }
        },
        .conv_code2vol = {
            .par_num = 2,
            .para_arry = {
                [0] = 5.1106E-01,
                [1] = 5.9677E-04,
            }
        }
    }
};

int32_t board_pvt_init()
{
    g_pvt_dev_ptr[PVT_DEV_SAFETY_INS] = sdrv_pvt_probe(&s_sdrv_pvt_dev[PVT_DEV_SAFETY_INS], &s_pvt_cfg[PVT_DEV_SAFETY_INS]);
    g_pvt_dev_ptr[PVT_DEV_AP_INS] = NULL;
    return 0;
}

#endif /* end CONFIG_PVT */

/* end of file pvt_cfg.c */