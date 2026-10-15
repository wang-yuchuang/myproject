
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_PINCTRL

#include <types.h>
#include "pinmux_cfg.h"
#include <driver/taishan/pinctrl/taishan_pinctrl.h>

#if NUM_OF_CONFIGURED_PINS > 0
pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS] = {
    /* Pin 5, GPIO_X0, Mux:XSPI1.PA_DQS */
    {
        .pin_index         =  GPIO_X0,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 6, GPIO_X1, Mux:XSPI1.PA_SCLK */
    {
        .pin_index         =  GPIO_X1,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 7, GPIO_X2, Mux:XSPI1.PA_SS0 */
    {
        .pin_index         =  GPIO_X2,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 8, GPIO_X3, Mux:XSPI1.PA_DATA0 */
    {
        .pin_index         =  GPIO_X3,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 9, GPIO_X4, Mux:XSPI1.PA_DATA1 */
    {
        .pin_index         =  GPIO_X4,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 10, GPIO_X5, Mux:XSPI1.PA_DATA2 */
    {
        .pin_index         =  GPIO_X5,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 11, GPIO_X6, Mux:XSPI1.PA_DATA3 */
    {
        .pin_index         =  GPIO_X6,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 12, GPIO_X7, Mux:XSPI1.PA_DATA4 */
    {
        .pin_index         =  GPIO_X7,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 13, GPIO_X8, Mux:XSPI1.PA_DATA5 */
    {
        .pin_index         =  GPIO_X8,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 14, GPIO_X9, Mux:XSPI1.PA_DATA6 */
    {
        .pin_index         =  GPIO_X9,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 15, GPIO_X10, Mux:XSPI1.PA_DATA7 */
    {
        .pin_index         =  GPIO_X10,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 17, GPIO_Y0, Mux:UART4.TXD */
    {
        .pin_index         =  GPIO_Y0,
        .mux               =  PIN_MUX_ALT4,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS_SCHMITT,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    },

    /* Pin 18, GPIO_Y1, Mux:UART4.RXD */
    {
        .pin_index         =  GPIO_Y1,
        .mux               =  PIN_MUX_ALT4,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_UP,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS_SCHMITT,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
        .mode_select       =  PIN_MODE_DIGITAL,
    },
};
#endif

#endif
