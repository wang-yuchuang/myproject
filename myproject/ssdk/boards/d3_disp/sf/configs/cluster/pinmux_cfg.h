
//*****************************************************************************
//
// WARNING: Automatically generated file, don't modify anymore!!!
//
// Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
// Software License Agreement
//
//*****************************************************************************

#ifndef PIN_MUX_CFG_H_
#define PIN_MUX_CFG_H_

#ifdef CONFIG_PINCTRL

typedef enum pin_mux {
    PIN_MUX_ALT0  = 0U,
    PIN_MUX_ALT1  = 1U,
    PIN_MUX_ALT2  = 2U,
    PIN_MUX_ALT3  = 3U,
    PIN_MUX_ALT4  = 4U,
    PIN_MUX_ALT5  = 5U,
    PIN_MUX_ALT6  = 6U,
    PIN_MUX_ALT7  = 7U,
    PIN_MUX_ALT8  = 8U,
    PIN_MUX_ALT9  = 9U,
} pin_mux_e;

typedef enum pin_open_drain {
    PIN_OPEN_DRAIN  = 0U,
    PIN_PUSH_PULL   = 2U,
} pin_open_drain_e;

typedef enum pin_pull_config {
    PIN_NOPULL     = 0U,
    PIN_PULL_DOWN  = 1U,
    PIN_PULL_UP    = 2U,
} pin_pull_config_e;

typedef enum pin_drive_strength {
    PIN_DS_2MA  = 0U,
    PIN_DS_8MA  = 1U,
    PIN_DS_4MA  = 2U,
    PIN_DS_12MA = 3U,
} pin_drive_strength_e;

typedef enum pin_slew_rate {
    PIN_SR_FAST  = 0U,
    PIN_SR_SLOW  = 1U,
} pin_slew_rate_e;

typedef enum pin_input_select {
    PIN_IS_CMOS          = 0U,
    PIN_IS_CMOS_SCHMITT  = 1U,
} pin_input_select_e;

typedef enum pin_data_direction {
    PIN_INPUT_DIRECTION  = 0U,
    PIN_OUTPUT_DIRECTION = 1U,
} pin_data_direction_e;

typedef enum pin_interrupt_config {
    PIN_INTERRUPT_DISABLED     = 0U,
    PIN_INTERRUPT_HIGH_LEVEL   = 1U,
    PIN_INTERRUPT_LOW_LEVEL    = 2U,
    PIN_INTERRUPT_RISING_EDGE  = 3U,
    PIN_INTERRUPT_FALLING_EDGE = 4U,
    PIN_INTERRUPT_BOTH_EDGE    = 5U,
} pin_interrupt_config_e;

typedef enum pin_level_type {
    PIN_LEVEL_LOW  = 0U,
    PIN_LEVEL_HIGH = 1U,
} pin_level_type_e;

typedef enum pin_force_input {
    PIN_FORCE_INPUT_NORMAL  = 0U,
    PIN_FORCE_INPUT_ENABLE  = 1U,
    PIN_FORCE_INPUT_HIGH    = 2U,
    PIN_FORCE_INPUT_LOW     = 3U,
} pin_force_input_e;

typedef enum pin_mode_select {
    PIN_MODE_DIGITAL            = 0U,
    PIN_MODE_SELECT_NORMAL      = 1U,
    PIN_MODE_DOWN_CONVERSION    = 2U
} pin_mode_select_e;

typedef struct pin_settings_config {
    uint32_t                  pin_index;
    pin_mux_e                 mux;
    pin_open_drain_e          open_drain;
    pin_pull_config_e         pull_config;
    pin_drive_strength_e      drive_strength;
    pin_slew_rate_e           slew_rate;
    pin_input_select_e        input_select;
    pin_data_direction_e      data_direction;
    pin_interrupt_config_e    interrupt_config;
    pin_level_type_e          initial_value;
    pin_force_input_e         force_input;
    pin_mode_select_e         mode_select;
} pin_settings_config_t;

#ifdef CONFIG_WORK_ON_Z1
#define NUM_OF_CONFIGURED_PINS  214
#else
#define NUM_OF_CONFIGURED_PINS  184
#endif

#if NUM_OF_CONFIGURED_PINS > 0
extern pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS];
#endif

extern void board_port_early_init(void);

extern void board_port_init(void);

#endif

#endif /* PIN_MUX_CFG_H_ */
