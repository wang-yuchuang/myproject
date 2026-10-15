/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef PIN_MUX_CFG_H_
#define PIN_MUX_CFG_H_

#ifdef CONFIG_PINCTRL

typedef enum pin_controller {
    PIN_CONTROLLER1  =  1U,
    PIN_CONTROLLER2  =  2U,
    PIN_CONTROLLER3  =  3U,
    PIN_CONTROLLER4  =  4U,
    PIN_CONTROLLER5  =  5U,
} pin_controller_e;

typedef enum pin_mux {
    PIN_MUX_ALT0  = 0U,
    PIN_MUX_ALT1  = 1U,
    PIN_MUX_ALT2  = 2U,
    PIN_MUX_ALT3  = 3U,
    PIN_MUX_ALT4  = 4U,
    PIN_MUX_ALT5  = 5U,
    PIN_MUX_ALT6  = 6U,
    PIN_MUX_ALT7  = 7U,
} pin_mux_e;

typedef enum pin_open_drain {
    PIN_OPEN_DRAIN_DISABLED  = 0U,
    PIN_OPEN_DRAIN_ENABLED   = 1U,
} pin_open_drain_e;

typedef enum pin_pull_config {
    PIN_PULL_NOT_ENABLED  = 0U,
    PIN_PULL_DOWN_ENABLED = 1U,
    PIN_PULL_UP_ENABLED   = 2U,
} pin_pull_config_e;

typedef enum pin_drive_strength {
    PIN_DRIVE_STRENTH_2MA   = 0U,
    PIN_DRIVE_STRENTH_8MA   = 1U,
    PIN_DRIVE_STRENTH_4MA   = 2U,
    PIN_DRIVE_STRENTH_12MA  = 3U,

    /* Drive Strength for SD/eMMC PAD */
    PIN_DRIVE_STRENTH_100_OHM  = 0U,
    PIN_DRIVE_STRENTH_66_OHM   = 4U,
    PIN_DRIVE_STRENTH_50_OHM   = 8U,
    PIN_DRIVE_STRENTH_40_OHM   = 12U,
    PIN_DRIVE_STRENTH_33_OHM   = 14U,
} pin_drive_strength_e;

typedef enum pin_slew_rate {
    PIN_FAST_SLEW_RATE  = 0U,
    PIN_SLOW_SLEW_RATE  = 1U,

    /* Slew rate for SD/eMMC PAD */
    PIN_SLOWEST_SLEW_RATE  = 0U,
    PIN_FASTEST_SLEW_RATE  = 3U,
} pin_slew_rate_e;

typedef enum pin_input_select {
    PIN_INPUT_CMOS_SCHMITT_MODE   = 0U,
    PIN_INPUT_CMOS_MODE           = 1U,

    /* Input Select for SD/eMMC PAD */
    PIN_INPUT_RECEIVER_DISABLE    = 0U,
    PIN_INPUT_1V8_SCHMITT_TRIGGER = 1U,
    PIN_INPUT_3V3_SCHMITT_TRIGGER = 2U,
    PIN_INPUT_INTERAL_LOOPBACK    = 7U,
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
} pin_force_input_e;

typedef struct pin_settings_config {
    uint32_t                  pin_index;
    pin_controller_e          controller_id;
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
} pin_settings_config_t;

typedef struct pin_source_select {
    uint32_t  pin_index;
    uint32_t  mux_id;
    uint32_t  offset;
    uint32_t  value;
} pin_source_select_t;

#define NUM_OF_CONFIGURED_PINS  156
#define NUM_OF_SOURCE_SELECT    364

#if NUM_OF_CONFIGURED_PINS > 0
extern pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS];
#endif

#if NUM_OF_SOURCE_SELECT > 0
extern pin_source_select_t   g_pin_source_select[NUM_OF_SOURCE_SELECT];
#endif

extern void board_port_init(void);

#endif

#endif /* PIN_MUX_CFG_H_ */
