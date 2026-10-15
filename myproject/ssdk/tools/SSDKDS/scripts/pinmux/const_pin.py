# -*- coding: utf-8 -*-

from const import Const

const_pin = Const()

const_pin.controller = {
    "GPIO1" : "PIN_CONTROLLER1",
    "GPIO2" : "PIN_CONTROLLER2",
    "GPIO3" : "PIN_CONTROLLER3",
    "GPIO4" : "PIN_CONTROLLER4",
    "GPIO5" : "PIN_CONTROLLER5"
}

const_pin.openDrain = {
    "enable" : "PIN_OPEN_DRAIN_ENABLED",
    "disable" : "PIN_OPEN_DRAIN_DISABLED"
}

const_pin.pullSelect = {
    "pull up" : "PIN_PULL_UP_ENABLED",
    "pull down" : "PIN_PULL_DOWN_ENABLED",
    "disable" : "PIN_PULL_NOT_ENABLED"
}

const_pin.driveStrength = {
    "2mA" : "PIN_DRIVE_STRENTH_2MA",
    "4mA" : "PIN_DRIVE_STRENTH_4MA",
    "8mA" : "PIN_DRIVE_STRENTH_8MA",
    "12mA" : "PIN_DRIVE_STRENTH_12MA",
    "100 ohm" : "PIN_DRIVE_STRENTH_100_OHM",
    "66 ohm" : "PIN_DRIVE_STRENTH_66_OHM",
    "50 ohm" : "PIN_DRIVE_STRENTH_50_OHM",
    "40 ohm" : "PIN_DRIVE_STRENTH_40_OHM",
    "33 ohm" : "PIN_DRIVE_STRENTH_33_OHM"
}

const_pin.slewRate = {
    "fast"    : "PIN_FAST_SLEW_RATE",
    "slow"    : "PIN_SLOW_SLEW_RATE",
    "fastest" : "PIN_FASTEST_SLEW_RATE",
    "slowest" : "PIN_SLOWEST_SLEW_RATE"
}

const_pin.inputSelect = {
    "cmos schmitt" : "PIN_INPUT_CMOS_SCHMITT_MODE",
    "cmos" : "PIN_INPUT_CMOS_MODE",
    "receiver disable" : "PIN_INPUT_RECEIVER_DISABLE",
    "1.8v schmitt trigger" : "PIN_INPUT_1V8_SCHMITT_TRIGGER",
    "3.3v schmitt trigger" : "PIN_INPUT_3V3_SCHMITT_TRIGGER",
    "interal loopback" : "PIN_INPUT_INTERAL_LOOPBACK"
}

const_pin.direction = {
    "input" : "PIN_INPUT_DIRECTION",
    "output" : "PIN_OUTPUT_DIRECTION"
}

const_pin.interrupt = {
    "disable" : "PIN_INTERRUPT_DISABLED",
    "high" : "PIN_INTERRUPT_HIGH_LEVEL",
    "low" : "PIN_INTERRUPT_LOW_LEVEL",
    "rising" : "PIN_INTERRUPT_RISING_EDGE",
    "falling" : "PIN_INTERRUPT_FALLING_EDGE",
    "both" : "PIN_INTERRUPT_BOTH_EDGE"
}

const_pin.initialVal = {
    "high" : "PIN_LEVEL_HIGH",
    "low" : "PIN_LEVEL_LOW"
}

const_pin.mux = {
    "0" : "PIN_MUX_ALT0",
    "1" : "PIN_MUX_ALT1",
    "2" : "PIN_MUX_ALT2",
    "3" : "PIN_MUX_ALT3",
    "4" : "PIN_MUX_ALT4",
    "5" : "PIN_MUX_ALT5",
    "6" : "PIN_MUX_ALT6",
    "7" : "PIN_MUX_ALT7",
    "8" : "PIN_MUX_ALT8",
    "9" : "PIN_MUX_ALT9"
}

const_pin.forceInput = {
    "normal" : "PIN_FORCE_INPUT_NORMAL",
    "enable" : "PIN_FORCE_INPUT_ENABLE"
}

const_pin.xml_excel_controller = {
    "GPIO1" : "GPIO1",
    "GPIO2" : "GPIO2",
    "GPIO3" : "GPIO3",
    "GPIO4" : "GPIO4",
    "GPIO5" : "GPIO5"
}

const_pin.xml_excel_ode = {
    "enable" : "Open-Drain",
    "disable" : "Push-Pull"
}

const_pin.xml_excel_pull = {
    "pull up" : "Pull Up",
    "pull down" : "Pull Down",
    "disable" : "No Pull"
}

const_pin.xml_excel_ds = {
    "2mA" : "2mA",
    "4mA" : "4mA",
    "8mA" : "8mA",
    "12mA" : "12mA",
    "100 ohm" : "100 ohm",
    "66 ohm" : "66 ohm",
    "50 ohm" : "50 ohm",
    "40 ohm" : "40 ohm",
    "33 ohm" : "33 ohm"
}

const_pin.xml_excel_emmc_ds = [
    "100 ohm", "66 ohm", "50 ohm", "40 ohm", "33 ohm"
]

const_pin.xml_excel_sr = {
    "fast"    : "Fast",
    "slow"    : "Slow",
    "fastest" : "Fastest",
    "slowest" : "Slowest"
}

const_pin.xml_excel_emmc_sr = [ "Fastest", "Slowest" ]

const_pin.xml_excel_is = {
    "cmos schmitt" : "Schmitt",
    "cmos" : "CMOS",
    "receiver disable" : "Receiver Disabled",
    "1.8v schmitt trigger" : "1.8V Schmitt Trigger",
    "3.3v schmitt trigger" : "3.3V Schmitt Trigger",
    "interal loopback" : "Loop-back mode"
}

const_pin.xml_excel_emmc_is = [
    "Receiver Disabled", "1.8V Schmitt Trigger",
    "3.3V Schmitt Trigger", "Loop-back mode"
]

const_pin.xml_excel_fi = {
    "normal" : "Normal",
    "enable" : "Enable"
}

const_pin.xml_excel_dir = {
    "input" : "Input",
    "output" : "Output"
}

const_pin.xml_excel_int = {
    "disable" : "Disable",
    "high" : "High Level",
    "low" : "Low Level",
    "rising" : "Rising Edge",
    "falling" : "Falling Edge",
    "both" : "Either Edge"
}

const_pin.xml_excel_init = {
    "high" : "High",
    "low" : "Low"
}

const_pin.xml_name       = "name"
const_pin.xml_controller = "gpio_controller"
const_pin.xml_mux        = "mux_function"
const_pin.xml_ode        = "open_drain"
const_pin.xml_pull       = "pull_select"
const_pin.xml_ds         = "drive_strength"
const_pin.xml_sr         = "slew_rate"
const_pin.xml_is         = "input_select"
const_pin.xml_dir        = "direction"
const_pin.xml_int        = "interrupt"
const_pin.xml_init       = "initial_value"
const_pin.xml_fi         = "force_input"

const_pin.relation = {
    const_pin.xml_controller  : ["controller_id     ", const_pin.controller],
    const_pin.xml_ode         : ["open_drain        ", const_pin.openDrain],
    const_pin.xml_pull        : ["pull_config       ", const_pin.pullSelect],
    const_pin.xml_ds          : ["drive_strength    ", const_pin.driveStrength],
    const_pin.xml_sr          : ["slew_rate         ", const_pin.slewRate],
    const_pin.xml_is          : ["input_select      ", const_pin.inputSelect],
    const_pin.xml_dir         : ["data_direction    ", const_pin.direction],
    const_pin.xml_int         : ["interrupt_config  ", const_pin.interrupt],
    const_pin.xml_init        : ["initial_value     ", const_pin.initialVal],
    const_pin.xml_fi          : ["force_input       ", const_pin.forceInput]
}

const_pin.excel_pin        = "Pin"
const_pin.excel_ball       = "Ball"
const_pin.excel_mux        = "Mux"
const_pin.excel_controller = "Controller"
const_pin.excel_ode        = "Open-Drain Enable"
const_pin.excel_pull       = "Pull Up/Down"
const_pin.excel_ds         = "Drive Strength"
const_pin.excel_sr         = "Slew Rate"
const_pin.excel_is         = "Input Select"
const_pin.excel_fi         = "Force Input"
const_pin.excel_dir        = "Direction"
const_pin.excel_int        = "Interrupt"
const_pin.excel_init       = "Initial Value"

const_pin.excel_first_row = (
    const_pin.excel_pin, const_pin.excel_ball, const_pin.excel_mux,
    const_pin.excel_controller, const_pin.excel_ode, const_pin.excel_pull,
    const_pin.excel_ds, const_pin.excel_sr, const_pin.excel_is,
    const_pin.excel_fi, const_pin.excel_dir, const_pin.excel_int,
    const_pin.excel_init
)

const_pin.xml_excel = {
    const_pin.xml_name       : [const_pin.excel_pin, {}],
    const_pin.xml_mux        : [const_pin.excel_mux, {}],
    const_pin.xml_controller : [const_pin.excel_controller, const_pin.xml_excel_controller],
    const_pin.xml_ode        : [const_pin.excel_ode, const_pin.xml_excel_ode],
    const_pin.xml_pull       : [const_pin.excel_pull, const_pin.xml_excel_pull],
    const_pin.xml_ds         : [const_pin.excel_ds, const_pin.xml_excel_ds],
    const_pin.xml_sr         : [const_pin.excel_sr, const_pin.xml_excel_sr],
    const_pin.xml_is         : [const_pin.excel_is, const_pin.xml_excel_is],
    const_pin.xml_fi         : [const_pin.excel_fi, const_pin.xml_excel_fi],
    const_pin.xml_dir         : [const_pin.excel_dir, const_pin.xml_excel_dir],
    const_pin.xml_int         : [const_pin.excel_int, const_pin.xml_excel_int],
    const_pin.xml_init         : [const_pin.excel_init, const_pin.xml_excel_init],
}

const_pin.xml_excel_emmc = {
    const_pin.xml_ds : const_pin.xml_excel_emmc_ds,
    const_pin.xml_sr : const_pin.xml_excel_emmc_sr,
    const_pin.xml_is : const_pin.xml_excel_emmc_is
}

const_pin.default_sheet_name = "Sheet1"
const_pin.emmc_pad_start = 132

const_pin.excel_xml = {
    const_pin.excel_pin       : [const_pin.xml_name, {}],
    const_pin.excel_mux        : [const_pin.xml_mux, {}],
    const_pin.excel_controller : [const_pin.xml_controller, const_pin.xml_excel_controller],
    const_pin.excel_ode        : [const_pin.xml_ode, const_pin.xml_excel_ode],
    const_pin.excel_pull       : [const_pin.xml_pull, const_pin.xml_excel_pull],
    const_pin.excel_ds         : [const_pin.xml_ds, const_pin.xml_excel_ds],
    const_pin.excel_sr         : [const_pin.xml_sr, const_pin.xml_excel_sr],
    const_pin.excel_is         : [const_pin.xml_is, const_pin.xml_excel_is],
    const_pin.excel_fi         : [const_pin.xml_fi, const_pin.xml_excel_fi],
    const_pin.excel_dir         : [const_pin.xml_dir, const_pin.xml_excel_dir],
    const_pin.excel_int         : [const_pin.xml_int, const_pin.xml_excel_int],
    const_pin.excel_init         : [const_pin.xml_init, const_pin.xml_excel_init],
}

const_pin.prefix = """/**
 * @file {}
 * @brief {}.
 * @warning Automatically generated file, don't modify.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
"""

const_pin.head_prefix = """
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

"""

const_pin.taishan_head_prefix = """
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
} pin_settings_config_t;

"""

const_pin.head_suffix = """
#if NUM_OF_CONFIGURED_PINS > 0
extern pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS];
#endif

#if NUM_OF_SOURCE_SELECT > 0
extern pin_source_select_t   g_pin_source_select[NUM_OF_SOURCE_SELECT];
#endif

extern void board_port_init(void);

#endif

#endif /* PIN_MUX_CFG_H_ */
"""

const_pin.taishan_head_suffix = """
#if NUM_OF_CONFIGURED_PINS > 0
extern pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS];
#endif

extern void board_port_early_init(void);

extern void board_port_init(void);

#endif

#endif /* PIN_MUX_CFG_H_ */
"""

const_pin.source_prefix = """
#ifdef CONFIG_PINCTRL

#include <types.h>
#include "pinmux_cfg.h"

#if NUM_OF_CONFIGURED_PINS > 0
pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS] = {
"""

const_pin.taishan_source_prefix = """
#ifdef CONFIG_PINCTRL

#include <types.h>
#include "pinmux_cfg.h"
#include <driver/pinctrl/taishan_pinctrl.h>

#if NUM_OF_CONFIGURED_PINS > 0
pin_settings_config_t g_pin_init_config[NUM_OF_CONFIGURED_PINS] = {
"""

const_pin.source_mid = """};
#endif

#if NUM_OF_SOURCE_SELECT > 0
pin_source_select_t g_pin_source_select[NUM_OF_SOURCE_SELECT] = {
"""

const_pin.source_suffix = """};
#endif

#endif
"""
