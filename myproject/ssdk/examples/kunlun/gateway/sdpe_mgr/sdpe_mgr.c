/**
 * @file sdpe_mgr.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#include <compiler.h>
#include <types.h>
#include <sdpe_rpc_service.h>
#include <sdpe_ctrl.h>
#include "vcan.h"
#include "can_cfg.h"
#include "lin_cfg.h"
#include "vlin.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/tca9539.h"
#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"
#include "boardinfo.h"
#include "pinmux_cfg.h"

extern pinctrl_t g_pinctrl_safety;
extern gpio_dev_t g_gpio_dev;
extern gpio_dev_t g_gpio_tca9539_sf_io33;

static void i2c_port_setup(void)
{
    /* W22, Pin 28, GPIO_A4, Mux:I2C3.SCL */
    pinctrl_config(&g_pinctrl_safety, 28, PINCTRL_CONFIG_FUNCTION, PAD_MUX_FUNC_ALT2);
    /* W21, Pin 29, GPIO_A5, Mux:I2C3.SDA */
    pinctrl_config(&g_pinctrl_safety, 29, PINCTRL_CONFIG_FUNCTION, PAD_MUX_FUNC_ALT2);
}

static void sdpe_port_init(void)
{
    int board_ver = 2;  /* default A02 */
    struct sd_hwid_usr usr;

    usr = get_fullhw_id();

    if (usr.magic == HW_ID_MAGIC && usr.ver == 1) {
        board_ver = usr.v.v1.board_id_minor;
    }

    i2c_port_setup();

    if (board_ver == 3) {
        /* Set GPIO_B4 High */
        gpio_set(&g_gpio_dev, 40U, PIN_LEVEL_HIGH);

        /* LIN1_EN */
        if (lin_config.Config[LIN_IFC_SCI1].hrdChannel != LIN_INVALID_ID) {
            gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P00, GPIO_OUT);
            gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P00, 1);
        }

        /* LIN2_EN */
        if (lin_config.Config[LIN_IFC_SCI2].hrdChannel != LIN_INVALID_ID) {
            gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P05, GPIO_OUT);
            gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P05, 1);
        }
    }
    else {
        /* GPIO_B8/B9/B10/B11 connected to LIN 1/2 transceiver. */
        if (lin_config.Config[LIN_IFC_SCI1].hrdChannel != LIN_INVALID_ID || \
            lin_config.Config[LIN_IFC_SCI2].hrdChannel != LIN_INVALID_ID) {
            gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P05, GPIO_OUT);
            gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P05, 1);
        }
    }

    if (lin_config.Config[LIN_IFC_SCI4].hrdChannel != LIN_INVALID_ID) {
        gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P04, GPIO_OUT);
        gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P04, 1);
    }

    /* CAN1_EN */
    gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P11, GPIO_OUT);
    gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P11, 1);
    /* CAN1_NTSB */
    gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P12, GPIO_OUT);
    gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P12, 1);
    /* CAN1_NERR */
    gpio_config(&g_gpio_tca9539_sf_io33, TCA9539_P13, GPIO_OUT);
    gpio_set(&g_gpio_tca9539_sf_io33, TCA9539_P13, 1);
}

static void sdpe_can_init(void)
{
    for (uint8_t i = 0; i < gCan_Config.controllerCount; i++) {
        Can_ConfigType Can_Config = {
            .controllerCount = 1,
            .ctrllerCfg = &gCan_CtrllerConfig[i],
        };

        for (uint16_t j = 0; j < gCan_Config.rxCount; j++) {
            if (gCan_RxMBCfg[j].controllerId ==
                    gCan_CtrllerConfig[i].controllerId) {
                Can_Config.rxCount++;

                if (!Can_Config.rxMBCfg) {
                    Can_Config.rxMBCfg = &gCan_RxMBCfg[j];
                }
            }
        }

        for (uint16_t j = 0; j < gCan_Config.txCount; j++) {
            if (gCan_TxMBCfg[j].controllerId ==
                    gCan_CtrllerConfig[i].controllerId) {
                Can_Config.txCount++;

                if (!Can_Config.txMBCfg) {
                    Can_Config.txMBCfg = &gCan_TxMBCfg[j];
                }
            }
        }

        Can_Init(&Can_Config);
    }

    /* Send configuration paramenter ending message. */
    Can_ConfigType ending = {
        .controllerCount = 0
    };
    Can_Init(&ending);

    for (uint8_t i = 0; i < gCan_Config.controllerCount; i++) {
        Can_SetControllerMode(gCan_Config.ctrllerCfg[i].controllerId,
                              CAN_CS_STARTED);
    }
}

static void sdpe_lin_init(void)
{
    Lin_Init(&lin_config);

    for (uint32_t i = 0; i < lin_config.Count; i++) {
        if (lin_config.Config[i].hrdChannel != LIN_INVALID_ID) {
            Lin_WakeupInternal((uint8_t)i);
        }
    }
}

void sdpe_mgr_init(void)
{
    sdpe_port_init();
    sdpe_rpc_service_init();
    sdpe_can_init();
    sdpe_lin_init();
    sdpe_start_routing((uint32_t)CONFIG_ROUTE_TABLE_MEMBASE,
                       (uint32_t)CONFIG_ROUTE_TABLE_MEMSIZE);
}
