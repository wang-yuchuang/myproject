
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifdef CONFIG_PINCTRL
#ifdef CONFIG_WORK_ON_Z1

#include <types.h>
#include "pinmux_cfg.h"
#include <driver/taishan/pinctrl/taishan_pinctrl.h>

#if NUM_OF_CONFIGURED_PINS > 0
pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS] = {
    /* Pin 0, JTAG_TDI, Mux:UART1.TXD */
    {
        .pin_index         =  JTAG_TDI,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 1, JTAG_TDO, Mux:UART1.RXD */
    {
        .pin_index         =  JTAG_TDO,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 2, JTAG_TRST_N, Mux:JTAG.TRST_N */
    {
        .pin_index         =  JTAG_TRST_N,
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

    /* Pin 3, JTAG_TMS, Mux:JTAG.TMS */
    {
        .pin_index         =  JTAG_TMS,
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

    /* Pin 4, JTAG_TCK, Mux:JTAG.TCK */
    {
        .pin_index         =  JTAG_TCK,
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
        .pull_config       =  PIN_PULL_UP,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
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
        .pull_config       =  PIN_NOPULL,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 16, GPIO_X11, Mux:XSPI1.PA_SS1 */
    {
        .pin_index         =  GPIO_X11,
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

    /* Pin 17, GPIO_Y0, Mux:GPIO_SF.IO17 */
    {
        .pin_index         =  GPIO_Y0,
        .mux               =  PIN_MUX_ALT0,
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

    /* Pin 18, GPIO_Y1, Mux:SPI6.SS1 */
    {
        .pin_index         =  GPIO_Y1,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 19, GPIO_Y2, Mux:SPI6.SS0 */
    {
        .pin_index         =  GPIO_Y2,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 20, GPIO_Y3, Mux:SPI6.SCLK */
    {
        .pin_index         =  GPIO_Y3,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 21, GPIO_Y4, Mux:SPI6.MISO */
    {
        .pin_index         =  GPIO_Y4,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 22, GPIO_Y5, Mux:XSPI1.PB_DATA3 */
    {
        .pin_index         =  GPIO_Y5,
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

    /* Pin 23, GPIO_Y6, Mux:XSPI1.PB_DATA2 */
    {
        .pin_index         =  GPIO_Y6,
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

    /* Pin 24, GPIO_Y7, Mux:XSPI1.PB_DATA1 */
    {
        .pin_index         =  GPIO_Y7,
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

    /* Pin 25, GPIO_Y8, Mux:XSPI1.PB_DATA0 */
    {
        .pin_index         =  GPIO_Y8,
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

    /* Pin 26, GPIO_Y9, Mux:XSPI1.PB_SS0 */
    {
        .pin_index         =  GPIO_Y9,
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

    /* Pin 27, GPIO_Y10, Mux:XSPI1.PB_SCLK */
    {
        .pin_index         =  GPIO_Y10,
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

    /* Pin 28, GPIO_Y11, Mux:XSPI1.PB_DQS */
    {
        .pin_index         =  GPIO_Y11,
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

    /* Pin 29, GPIO_A0, Mux:SPI1.SS0 */
    {
        .pin_index         =  GPIO_A0,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 30, GPIO_A1, Mux:SPI1.SCLK */
    {
        .pin_index         =  GPIO_A1,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 31, GPIO_A2, Mux:SPI1.MISO */
    {
        .pin_index         =  GPIO_A2,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 32, GPIO_A3, Mux:SPI1.MOSI */
    {
        .pin_index         =  GPIO_A3,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 33, GPIO_A4, Mux:XSPI2.PA_SCLK */
    {
        .pin_index         =  GPIO_A4,
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

    /* Pin 34, GPIO_A5, Mux:XSPI2.PA_SS0 */
    {
        .pin_index         =  GPIO_A5,
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

    /* Pin 35, GPIO_A6, Mux:XSPI2.PA_DATA0 */
    {
        .pin_index         =  GPIO_A6,
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

    /* Pin 36, GPIO_A7, Mux:XSPI2.PA_DATA1 */
    {
        .pin_index         =  GPIO_A7,
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

    /* Pin 37, GPIO_A8, Mux:XSPI2.PA_DATA2 */
    {
        .pin_index         =  GPIO_A8,
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

    /* Pin 38, GPIO_A9, Mux:XSPI2.PA_DATA3 */
    {
        .pin_index         =  GPIO_A9,
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

    /* Pin 39, GPIO_A10, Mux:XSPI2.PA_DATA4 */
    {
        .pin_index         =  GPIO_A10,
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

    /* Pin 40, GPIO_A11, Mux:XSPI2.PA_DATA5 */
    {
        .pin_index         =  GPIO_A11,
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

    /* Pin 41, GPIO_A12, Mux:XSPI2.PA_DATA6 */
    {
        .pin_index         =  GPIO_A12,
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

    /* Pin 42, GPIO_A13, Mux:XSPI2.PA_DATA7 */
    {
        .pin_index         =  GPIO_A13,
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

    /* Pin 43, GPIO_A14, Mux:XSPI2.PA_DQS */
    {
        .pin_index         =  GPIO_A14,
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

    /* Pin 44, GPIO_A15, Mux:XSPI2.PA_SS1 */
    {
        .pin_index         =  GPIO_A15,
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

    /* Pin 45, GPIO_B0, Mux:SPI3.SS0 */
    {
        .pin_index         =  GPIO_B0,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 46, GPIO_B1, Mux:SPI3.SCLK */
    {
        .pin_index         =  GPIO_B1,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 47, GPIO_B2, Mux:SPI3.MISO */
    {
        .pin_index         =  GPIO_B2,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 48, GPIO_B3, Mux:SPI3.MOSI */
    {
        .pin_index         =  GPIO_B3,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 49, GPIO_B4, Mux:xTRG.IO20 */
    {
        .pin_index         =  GPIO_B4,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 50, GPIO_B5, Mux:xTRG.IO21 */
    {
        .pin_index         =  GPIO_B5,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 51, GPIO_B6, Mux:xTRG.IO22 */
    {
        .pin_index         =  GPIO_B6,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 52, GPIO_B7, Mux:xTRG.IO23 */
    {
        .pin_index         =  GPIO_B7,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 53, GPIO_B8, Mux:xTRG.IO24 */
    {
        .pin_index         =  GPIO_B8,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 54, GPIO_B9, Mux:xTRG.IO25 */
    {
        .pin_index         =  GPIO_B9,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 55, GPIO_B10, Mux:xTRG.IO26 */
    {
        .pin_index         =  GPIO_B10,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 56, GPIO_B11, Mux:xTRG.IO27 */
    {
        .pin_index         =  GPIO_B11,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 57, GPIO_B12, Mux:SPI5.MISO */
    {
        .pin_index         =  GPIO_B12,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 58, GPIO_B13, Mux:SPI5.MOSI */
    {
        .pin_index         =  GPIO_B13,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 59, GPIO_B14, Mux:SPI5.SS0 */
    {
        .pin_index         =  GPIO_B14,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 60, GPIO_B15, Mux:SPI5.SCLK */
    {
        .pin_index         =  GPIO_B15,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 61, GPIO_C0, Mux:xTRG.IO54 */
    {
        .pin_index         =  GPIO_C0,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 62, GPIO_C1, Mux:xTRG.IO55 */
    {
        .pin_index         =  GPIO_C1,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 63, GPIO_C2, Mux:SPI8.SS0 */
    {
        .pin_index         =  GPIO_C2,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 64, GPIO_C3, Mux:SPI8.SCLK */
    {
        .pin_index         =  GPIO_C3,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 65, GPIO_C4, Mux:SPI8.MISO */
    {
        .pin_index         =  GPIO_C4,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 66, GPIO_C5, Mux:SPI8.MOSI */
    {
        .pin_index         =  GPIO_C5,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 67, GPIO_C6, Mux:SPI4.SS3 */
    {
        .pin_index         =  GPIO_C6,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 68, GPIO_C7, Mux:SPI4.SS2 */
    {
        .pin_index         =  GPIO_C7,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 69, GPIO_C8, Mux:xTRG.IO62 */
    {
        .pin_index         =  GPIO_C8,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 70, GPIO_C9, Mux:SPI4.SS0 */
    {
        .pin_index         =  GPIO_C9,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 71, GPIO_C10, Mux:SPI4.SCLK */
    {
        .pin_index         =  GPIO_C10,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 72, GPIO_C11, Mux:SPI4.MISO */
    {
        .pin_index         =  GPIO_C11,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 73, GPIO_C12, Mux:SPI4.MOSI */
    {
        .pin_index         =  GPIO_C12,
        .mux               =  PIN_MUX_ALT3,
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

    /* Pin 74, GPIO_C13, Mux:xTRG.IO3 */
    {
        .pin_index         =  GPIO_C13,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 75, GPIO_C14, Mux:xTRG.IO4 */
    {
        .pin_index         =  GPIO_C14,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 76, GPIO_C15, Mux:xTRG.IO5 */
    {
        .pin_index         =  GPIO_C15,
        .mux               =  PIN_MUX_ALT5,
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

    /* Pin 77, GPIO_G0, Mux:UART7.TXD */
    {
        .pin_index         =  GPIO_G0,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 78, GPIO_G1, Mux:UART7.RXD */
    {
        .pin_index         =  GPIO_G1,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 79, GPIO_G2, Mux:UART6.TXD */
    {
        .pin_index         =  GPIO_G2,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 80, GPIO_G3, Mux:UART6.RXD */
    {
        .pin_index         =  GPIO_G3,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 81, GPIO_G4, Mux:ENET1.TXD3 */
    {
        .pin_index         =  GPIO_G4,
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

    /* Pin 82, GPIO_G5, Mux:ENET1.TXEN */
    {
        .pin_index         =  GPIO_G5,
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

    /* Pin 83, GPIO_G6, Mux:UART4.TXD */
    {
        .pin_index         =  GPIO_G6,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 84, GPIO_G7, Mux:UART4.RXD */
    {
        .pin_index         =  GPIO_G7,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 85, GPIO_G8, Mux:UART3.TXD */
    {
        .pin_index         =  GPIO_G8,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 86, GPIO_G9, Mux:UART3.RXD */
    {
        .pin_index         =  GPIO_G9,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 87, GPIO_G10, Mux:UART2.TXD */
    {
        .pin_index         =  GPIO_G10,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 88, GPIO_G11, Mux:UART2.RXD */
    {
        .pin_index         =  GPIO_G11,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 89, GPIO_M0, Mux:UART16.TXD */
    {
        .pin_index         =  GPIO_M0,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 90, GPIO_M1, Mux:UART16.RXD */
    {
        .pin_index         =  GPIO_M1,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 91, GPIO_M2, Mux:UART15.TXD */
    {
        .pin_index         =  GPIO_M2,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 92, GPIO_M3, Mux:UART15.RXD */
    {
        .pin_index         =  GPIO_M3,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 93, GPIO_M4, Mux:UART14.TXD */
    {
        .pin_index         =  GPIO_M4,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 94, GPIO_M5, Mux:UART14.RXD */
    {
        .pin_index         =  GPIO_M5,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 95, GPIO_M6, Mux:UART13.TXD */
    {
        .pin_index         =  GPIO_M6,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 96, GPIO_M7, Mux:UART13.RXD */
    {
        .pin_index         =  GPIO_M7,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 97, GPIO_M8, Mux:UART13.CTSN */
    {
        .pin_index         =  GPIO_M8,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 98, GPIO_M9, Mux:UART13.RTSN */
    {
        .pin_index         =  GPIO_M9,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 99, GPIO_M10, Mux:UART12.TXD */
    {
        .pin_index         =  GPIO_M10,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 100, GPIO_M11, Mux:UART12.RXD */
    {
        .pin_index         =  GPIO_M11,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 101, GPIO_S0, Mux:SACI1.TXSCK */
    {
        .pin_index         =  GPIO_S0,
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

    /* Pin 102, GPIO_S1, Mux:SACI1.TXWS */
    {
        .pin_index         =  GPIO_S1,
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

    /* Pin 103, GPIO_S2, Mux:SACI1.TXRXD0 */
    {
        .pin_index         =  GPIO_S2,
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

    /* Pin 104, GPIO_S3, Mux:SACI1.TXRXD1 */
    {
        .pin_index         =  GPIO_S3,
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

    /* Pin 105, GPIO_S4, Mux:SACI1.TXRXD2 */
    {
        .pin_index         =  GPIO_S4,
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

    /* Pin 106, GPIO_S5, Mux:SACI1.TXRXD3 */
    {
        .pin_index         =  GPIO_S5,
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

    /* Pin 107, GPIO_S6, Mux:SACI1.TXRXD4 */
    {
        .pin_index         =  GPIO_S6,
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

    /* Pin 108, GPIO_S7, Mux:SACI1.TXRXD5 */
    {
        .pin_index         =  GPIO_S7,
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

    /* Pin 109, GPIO_S8, Mux:SACI1.TXRXD6 */
    {
        .pin_index         =  GPIO_S8,
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

    /* Pin 110, GPIO_S9, Mux:SACI1.TXRXD7 */
    {
        .pin_index         =  GPIO_S9,
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

    /* Pin 111, GPIO_S10, Mux:SACI1.RXSCK */
    {
        .pin_index         =  GPIO_S10,
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

    /* Pin 112, GPIO_S11, Mux:SACI1.RXWS */
    {
        .pin_index         =  GPIO_S11,
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

    /* Pin 113, GPIO_S12, Mux:SACI1.PDM_CLK */
    {
        .pin_index         =  GPIO_S12,
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

    /* Pin 114, GPIO_S13, Mux:SACI1.PDM_DATA */
    {
        .pin_index         =  GPIO_S13,
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

    /* Pin 115, GPIO_H0, Mux:GPIO_SF.IO115 */
    {
        .pin_index         =  GPIO_H0,
        .mux               =  PIN_MUX_ALT0,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS_SCHMITT,
        .data_direction    =  PIN_INPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 116, GPIO_H1, Mux:GPIO_SF.IO116 */
    {
        .pin_index         =  GPIO_H1,
        .mux               =  PIN_MUX_ALT0,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_HIGH,
        .force_input       =  PIN_FORCE_INPUT_NORMAL,
    },

    /* Pin 117, GPIO_H2, Mux:SEHC1.EMMC_DAT7 */
    {
        .pin_index         =  GPIO_H2,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 118, GPIO_H3, Mux:SEHC1.EMMC_DAT6 */
    {
        .pin_index         =  GPIO_H3,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 119, GPIO_H4, Mux:SEHC1.EMMC_DAT5 */
    {
        .pin_index         =  GPIO_H4,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 120, GPIO_H5, Mux:SEHC1.EMMC_DAT4 */
    {
        .pin_index         =  GPIO_H5,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 121, GPIO_H6, Mux:SEHC1.EMMC_DAT3 */
    {
        .pin_index         =  GPIO_H6,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 122, GPIO_H7, Mux:SEHC1.EMMC_DAT2 */
    {
        .pin_index         =  GPIO_H7,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 123, GPIO_H8, Mux:SEHC1.EMMC_DAT1 */
    {
        .pin_index         =  GPIO_H8,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 124, GPIO_H9, Mux:SEHC1.EMMC_DAT0 */
    {
        .pin_index         =  GPIO_H9,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 125, GPIO_H10, Mux:SEHC1.EMMC_CLK */
    {
        .pin_index         =  GPIO_H10,
        .mux               =  PIN_MUX_ALT4,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_ENABLE,
    },

    /* Pin 126, GPIO_H11, Mux:SEHC1.EMMC_CMD */
    {
        .pin_index         =  GPIO_H11,
        .mux               =  PIN_MUX_ALT4,
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

    /* Pin 127, GPIO_H12, Mux:GPIO_SF.IO127 */
    {
        .pin_index         =  GPIO_H12,
        .mux               =  PIN_MUX_ALT0,
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

    /* Pin 128, GPIO_H13, Mux:GPIO_SF.IO128 */
    {
        .pin_index         =  GPIO_H13,
        .mux               =  PIN_MUX_ALT0,
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

    /* Pin 129, GPIO_F0, Mux:SEHC2.EMMC_DAT3 */
    {
        .pin_index         =  GPIO_F0,
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

    /* Pin 130, GPIO_F1, Mux:SEHC2.EMMC_DAT2 */
    {
        .pin_index         =  GPIO_F1,
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

    /* Pin 131, GPIO_F2, Mux:SEHC2.EMMC_DAT1 */
    {
        .pin_index         =  GPIO_F2,
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

    /* Pin 132, GPIO_F3, Mux:SEHC2.EMMC_DAT0 */
    {
        .pin_index         =  GPIO_F3,
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

    /* Pin 133, GPIO_F4, Mux:SEHC2.EMMC_CLK */
    {
        .pin_index         =  GPIO_F4,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_ENABLE,
    },

    /* Pin 134, GPIO_F5, Mux:SEHC2.EMMC_CMD */
    {
        .pin_index         =  GPIO_F5,
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

    /* Pin 135, GPIO_L0, Mux:ENET2.MDIO */
    {
        .pin_index         =  GPIO_L0,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 136, GPIO_L1, Mux:ENET2.MDC */
    {
        .pin_index         =  GPIO_L1,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 137, GPIO_L2, Mux:ENET2.RMII_REF_CLK */
    {
        .pin_index         =  GPIO_L2,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 138, GPIO_L3, Mux:ENET2.TXD0 */
    {
        .pin_index         =  GPIO_L3,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 139, GPIO_L4, Mux:ENET2.TXD1 */
    {
        .pin_index         =  GPIO_L4,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 140, GPIO_L5, Mux:ENET2.TXEN */
    {
        .pin_index         =  GPIO_L5,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 141, GPIO_L6, Mux:ENET2.RX_ER */
    {
        .pin_index         =  GPIO_L6,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 142, GPIO_L7, Mux:ENET2.RXD0 */
    {
        .pin_index         =  GPIO_L7,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 143, GPIO_L8, Mux:ENET2.RXD1 */
    {
        .pin_index         =  GPIO_L8,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 144, GPIO_L9, Mux:ENET2.RXDV */
    {
        .pin_index         =  GPIO_L9,
        .mux               =  PIN_MUX_ALT2,
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

    /* Pin 145, GPIO_D0, Mux:SDRAMC.ADDR0 */
    {
        .pin_index         =  GPIO_D0,
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

    /* Pin 146, GPIO_D1, Mux:SDRAMC.ADDR1 */
    {
        .pin_index         =  GPIO_D1,
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

    /* Pin 147, GPIO_D2, Mux:SDRAMC.ADDR2 */
    {
        .pin_index         =  GPIO_D2,
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

    /* Pin 148, GPIO_D3, Mux:SDRAMC.ADDR3 */
    {
        .pin_index         =  GPIO_D3,
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

    /* Pin 149, GPIO_D4, Mux:SDRAMC.ADDR4 */
    {
        .pin_index         =  GPIO_D4,
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

    /* Pin 150, GPIO_D5, Mux:SDRAMC.ADDR5 */
    {
        .pin_index         =  GPIO_D5,
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

    /* Pin 151, GPIO_D6, Mux:SDRAMC.ADDR6 */
    {
        .pin_index         =  GPIO_D6,
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

    /* Pin 152, GPIO_D7, Mux:SDRAMC.ADDR7 */
    {
        .pin_index         =  GPIO_D7,
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

    /* Pin 153, GPIO_D8, Mux:SDRAMC.ADDR8 */
    {
        .pin_index         =  GPIO_D8,
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

    /* Pin 154, GPIO_D9, Mux:SDRAMC.ADDR9 */
    {
        .pin_index         =  GPIO_D9,
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

    /* Pin 155, GPIO_D10, Mux:SDRAMC.ADDR10 */
    {
        .pin_index         =  GPIO_D10,
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

    /* Pin 156, GPIO_D11, Mux:SDRAMC.ADDR11 */
    {
        .pin_index         =  GPIO_D11,
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

    /* Pin 157, GPIO_D12, Mux:SDRAMC.ADDR12 */
    {
        .pin_index         =  GPIO_D12,
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

    /* Pin 158, GPIO_D13, Mux:SDRAMC.BA0 */
    {
        .pin_index         =  GPIO_D13,
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

    /* Pin 159, GPIO_D14, Mux:SDRAMC.BA1 */
    {
        .pin_index         =  GPIO_D14,
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

    /* Pin 160, GPIO_D15, Mux:SDRAMC.RAS */
    {
        .pin_index         =  GPIO_D15,
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

    /* Pin 161, GPIO_D16, Mux:SDRAMC.CAS */
    {
        .pin_index         =  GPIO_D16,
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

    /* Pin 162, GPIO_D17, Mux:SDRAMC.WE */
    {
        .pin_index         =  GPIO_D17,
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

    /* Pin 163, GPIO_D18, Mux:SDRAMC.CKE0 */
    {
        .pin_index         =  GPIO_D18,
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

    /* Pin 164, GPIO_D19, Mux:SDRAMC.CLK_P */
    {
        .pin_index         =  GPIO_D19,
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

    /* Pin 165, GPIO_D20, Mux:SDRAMC.CLK_N */
    {
        .pin_index         =  GPIO_D20,
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

    /* Pin 166, GPIO_D21, Mux:SDRAMC.CS0 */
    {
        .pin_index         =  GPIO_D21,
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

    /* Pin 167, GPIO_D22, Mux:SDRAMC.DATA0 */
    {
        .pin_index         =  GPIO_D22,
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

    /* Pin 168, GPIO_D23, Mux:SDRAMC.DATA1 */
    {
        .pin_index         =  GPIO_D23,
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

    /* Pin 169, GPIO_D24, Mux:SDRAMC.DATA2 */
    {
        .pin_index         =  GPIO_D24,
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

    /* Pin 170, GPIO_D25, Mux:SDRAMC.DATA3 */
    {
        .pin_index         =  GPIO_D25,
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

    /* Pin 171, GPIO_D26, Mux:SDRAMC.DQS0 */
    {
        .pin_index         =  GPIO_D26,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_ENABLE,
    },

    /* Pin 172, GPIO_D27, Mux:SDRAMC.DATA4 */
    {
        .pin_index         =  GPIO_D27,
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

    /* Pin 173, GPIO_D28, Mux:SDRAMC.DATA5 */
    {
        .pin_index         =  GPIO_D28,
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

    /* Pin 174, GPIO_D29, Mux:SDRAMC.DATA6 */
    {
        .pin_index         =  GPIO_D29,
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

    /* Pin 175, GPIO_D30, Mux:SDRAMC.DATA7 */
    {
        .pin_index         =  GPIO_D30,
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

    /* Pin 176, GPIO_D31, Mux:SDRAMC.DM0 */
    {
        .pin_index         =  GPIO_D31,
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

    /* Pin 177, GPIO_D32, Mux:SDRAMC.DATA8 */
    {
        .pin_index         =  GPIO_D32,
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

    /* Pin 178, GPIO_D33, Mux:SDRAMC.DATA9 */
    {
        .pin_index         =  GPIO_D33,
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

    /* Pin 179, GPIO_D34, Mux:SDRAMC.DATA10 */
    {
        .pin_index         =  GPIO_D34,
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

    /* Pin 180, GPIO_D35, Mux:SDRAMC.DATA11 */
    {
        .pin_index         =  GPIO_D35,
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

    /* Pin 181, GPIO_D36, Mux:SDRAMC.DQS1 */
    {
        .pin_index         =  GPIO_D36,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_ENABLE,
    },

    /* Pin 182, GPIO_D37, Mux:SDRAMC.DATA12 */
    {
        .pin_index         =  GPIO_D37,
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

    /* Pin 183, GPIO_D38, Mux:SDRAMC.DATA13 */
    {
        .pin_index         =  GPIO_D38,
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

    /* Pin 184, GPIO_D39, Mux:SDRAMC.DATA14 */
    {
        .pin_index         =  GPIO_D39,
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

    /* Pin 185, GPIO_D40, Mux:SDRAMC.DATA15 */
    {
        .pin_index         =  GPIO_D40,
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

    /* Pin 186, GPIO_D41, Mux:SDRAMC.DM1 */
    {
        .pin_index         =  GPIO_D41,
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

    /* Pin 187, GPIO_E0, Mux:SDRAMC.ADDR13 */
    {
        .pin_index         =  GPIO_E0,
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

    /* Pin 188, GPIO_E1, Mux:SDRAMC.DATA16 */
    {
        .pin_index         =  GPIO_E1,
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

    /* Pin 189, GPIO_E2, Mux:SDRAMC.DATA17 */
    {
        .pin_index         =  GPIO_E2,
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

    /* Pin 190, GPIO_E3, Mux:SDRAMC.DATA18 */
    {
        .pin_index         =  GPIO_E3,
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

    /* Pin 191, GPIO_E4, Mux:SDRAMC.DATA19 */
    {
        .pin_index         =  GPIO_E4,
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

    /* Pin 192, GPIO_E5, Mux:SDRAMC.DQS2 */
    {
        .pin_index         =  GPIO_E5,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_ENABLE,
    },

    /* Pin 193, GPIO_E6, Mux:SDRAMC.DATA20 */
    {
        .pin_index         =  GPIO_E6,
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

    /* Pin 194, GPIO_E7, Mux:SDRAMC.DATA21 */
    {
        .pin_index         =  GPIO_E7,
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

    /* Pin 195, GPIO_E8, Mux:SDRAMC.DATA22 */
    {
        .pin_index         =  GPIO_E8,
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

    /* Pin 196, GPIO_E9, Mux:SDRAMC.DATA23 */
    {
        .pin_index         =  GPIO_E9,
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

    /* Pin 197, GPIO_E10, Mux:SDRAMC.DM2 */
    {
        .pin_index         =  GPIO_E10,
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

    /* Pin 198, GPIO_E11, Mux:SDRAMC.DATA24 */
    {
        .pin_index         =  GPIO_E11,
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

    /* Pin 199, GPIO_E12, Mux:SDRAMC.DATA25 */
    {
        .pin_index         =  GPIO_E12,
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

    /* Pin 200, GPIO_E13, Mux:SDRAMC.DATA26 */
    {
        .pin_index         =  GPIO_E13,
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

    /* Pin 201, GPIO_E14, Mux:SDRAMC.DATA27 */
    {
        .pin_index         =  GPIO_E14,
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

    /* Pin 202, GPIO_E15, Mux:SDRAMC.DQS3 */
    {
        .pin_index         =  GPIO_E15,
        .mux               =  PIN_MUX_ALT1,
        .open_drain        =  PIN_PUSH_PULL,
        .pull_config       =  PIN_PULL_DOWN,
        .drive_strength    =  PIN_DS_8MA,
        .slew_rate         =  PIN_SR_FAST,
        .input_select      =  PIN_IS_CMOS,
        .data_direction    =  PIN_OUTPUT_DIRECTION,
        .interrupt_config  =  PIN_INTERRUPT_DISABLED,
        .initial_value     =  PIN_LEVEL_LOW,
        .force_input       =  PIN_FORCE_INPUT_ENABLE,
    },

    /* Pin 203, GPIO_E16, Mux:SDRAMC.DATA28 */
    {
        .pin_index         =  GPIO_E16,
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

    /* Pin 204, GPIO_E17, Mux:SDRAMC.DATA29 */
    {
        .pin_index         =  GPIO_E17,
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

    /* Pin 205, GPIO_E18, Mux:SDRAMC.DATA30 */
    {
        .pin_index         =  GPIO_E18,
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

    /* Pin 206, GPIO_E19, Mux:SDRAMC.DATA31 */
    {
        .pin_index         =  GPIO_E19,
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

    /* Pin 207, GPIO_E20, Mux:SDRAMC.DM3 */
    {
        .pin_index         =  GPIO_E20,
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

    /* Pin 208, GPIO_E21, Mux:SDRAMC.CKE1 */
    {
        .pin_index         =  GPIO_E21,
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

    /* Pin 209, GPIO_E22, Mux:SDRAMC.CKE2 */
    {
        .pin_index         =  GPIO_E22,
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

    /* Pin 210, GPIO_E23, Mux:SDRAMC.CKE3 */
    {
        .pin_index         =  GPIO_E23,
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

    /* Pin 211, GPIO_E24, Mux:SDRAMC.CS1 */
    {
        .pin_index         =  GPIO_E24,
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

    /* Pin 212, GPIO_E25, Mux:SDRAMC.CS2 */
    {
        .pin_index         =  GPIO_E25,
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

    /* Pin 213, GPIO_E26, Mux:SDRAMC.CS3 */
    {
        .pin_index         =  GPIO_E26,
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
};
#endif

#endif
#endif
