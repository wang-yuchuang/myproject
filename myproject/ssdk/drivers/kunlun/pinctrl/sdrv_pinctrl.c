/**
 * @file sdrv_pinctrl.c
 * @brief pinctrl driver
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <types.h>
#include <debug.h>
#include <debug.h>
#include <param.h>

#include "scr_hw.h"
#include "driver/scr.h"
#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"

#include "sdrv_pinctrl_lld.h"

#include "regs_base.h"
#include "pinmux_cfg.h"
#include "scr_cfg.h"

static inline bool is_emmc_pin(pinctrl_t *pinctrl, unsigned int pin)
{
    unsigned int soc_pin = to_soc_pin(pinctrl, pin);
    sdrv_pinctrl_t *ctrl = pinctrl->dev;

    return soc_pin >= ctrl->emmc_pad_start &&
           soc_pin < ctrl->emmc_pad_start + ctrl->emmc_pad_nr;
}

/*
 * Some SOC internal module signals have multiple pad connections, which
 * could be selected by source MUX. This function selects the source pad
 * connected to the signal.
 */
static void set_source_pad(paddr_t base, unsigned int pin, uint32_t function)
{
    for (int i = 0; i < NUM_OF_SOURCE_SELECT; i++) {
        pin_source_select_t *select = &g_pin_source_select[i];
        if (select->pin_index == pin && select->mux_id == function) {
            writel(select->value, base + (select->offset << 10));
            return;
        }
    }
}

/**
 * @brief Configure the pinctrl device.
 *
 * @param[in] pinctrl pinctrl device to initialize.
 * @param[in] pin Controller local pin index.
 * @param[in] config Configuration type.
 * @param[in] value Configuration value, depending on config type.
 */
static bool _sdrv_pinctrl_config(pinctrl_t *pinctrl, unsigned int pin,
                                 pinctrl_config_e config, unsigned int value)
{
    sdrv_pinctrl_t *ctrl = pinctrl->dev;
    bool ret = true;

    ASSERT(ctrl);

    switch (config) {
        case PINCTRL_CONFIG_FUNCTION:
            /* The controller must have MUX function. */
            ASSERT(ctrl->mux_reg_offset);
#ifdef CONFIG_BOARD_X9SP
            ASSERT(value >= PAD_MUX_FUNC_GPIO && value <= PAD_MUX_FUNC_ALT8);
#else
            ASSERT(value >= PAD_MUX_FUNC_GPIO && value <= PAD_MUX_FUNC_ALT7);
#endif
#ifdef SOURCE_SELECT_BASE
            set_source_pad(SOURCE_SELECT_BASE, to_soc_pin(pinctrl, pin), value);
#else
            set_source_pad(ctrl->base, to_soc_pin(pinctrl, pin), value);
#endif
            sdrv_pinctrl_set_function(ctrl->base, ctrl->mux_reg_offset,
                                      pin, value);
            break;

        case PINCTRL_CONFIG_DRIVE:
            /* The controller must have MUX function. */
            ASSERT(ctrl->mux_reg_offset);
            ASSERT(value == PINCTRL_DRIVE_OPEN_DRAIN ||
                   value == PINCTRL_DRIVE_PUSH_PULL);

            if (value == PINCTRL_DRIVE_OPEN_DRAIN)
                value = PAD_OPENDRAIN;
            else
                value = PAD_PUSHPULL;
            sdrv_pinctrl_set_drive(ctrl->base, ctrl->mux_reg_offset,
                                   pin, value);
            break;

        case PINCTRL_CONFIG_PULL:
            ASSERT(value >= PINCTRL_PULL_NOPULL && value <= PINCTRL_PULL_UP);

            if (is_emmc_pin(pinctrl, pin)) {
                if (value == PINCTRL_PULL_NOPULL)
                    value = EMMC_PAD_NOPULL;
                else if (value == PINCTRL_PULL_UP)
                    value = EMMC_PAD_PULLUP;
                else
                    value = EMMC_PAD_PULLDOWN;
                sdrv_pinctrl_set_pull_emmc(ctrl->base, ctrl->pad_reg_offset,
                                           pin, value);
            }
            else {
                if (value == PINCTRL_PULL_NOPULL)
                    value = PAD_NOPULL;
                else if (value == PINCTRL_PULL_UP)
                    value = PAD_PULLUP;
                else
                    value = PAD_PULLDOWN;
                sdrv_pinctrl_set_pull_gpio(ctrl->base, ctrl->pad_reg_offset,
                                           pin, value);
            }
            break;

        case PINCTRL_CONFIG_DRIVE_STRENGTH:
            if (is_emmc_pin(pinctrl, pin)) {
                ASSERT(value >= EMMC_PAD_DRIVE_STRENGTH_100ohm &&
                       value <= EMMC_PAD_DRIVE_STRENGTH_33ohm);
                sdrv_pinctrl_set_drive_strength_emmc(ctrl->base,
                                                     ctrl->pad_reg_offset,
                                                     pin, value);
            }
            else {
                ASSERT(value >= PAD_DRIVE_STRENGTH_2mA &&
                       value <= PAD_DRIVE_STRENGTH_12mA);
                sdrv_pinctrl_set_drive_strength_gpio(ctrl->base,
                                                     ctrl->pad_reg_offset,
                                                     pin, value);
            }
            break;

        case PINCTRL_CONFIG_INPUT_SCHMITT:
            if (is_emmc_pin(pinctrl, pin)) {
                /*
                 * FIXME - EMMC pads have 1.8V and 3.3V Schmitt modes, so
                 * caller shall use EMMC specific Schmitt values...
                 */
                ASSERT(value >= EMMC_PAD_RXSEL_RECEIVER_DISABLED &&
                       value <= EMMC_PAD_RXSEL_INTERNAL_LOOPBACK);
                sdrv_pinctrl_set_input_schmitt_emmc(ctrl->base,
                                                    ctrl->pad_reg_offset,
                                                    pin, value);
            }
            else {
                if (value == PINCTRL_INPUT_SCHMITT_DISABLE)
                    value = PAD_INPUT_NORMAL;
                else if (value == PINCTRL_INPUT_SCHMITT_ENABLE)
                    value = PAD_INPUT_SCHMITT;
                else
                    ASSERT(false);
                sdrv_pinctrl_set_input_schmitt_gpio(ctrl->base,
                                                    ctrl->pad_reg_offset,
                                                    pin, value);
            }
            break;

        case PINCTRL_CONFIG_SLEW_RATE:
            if (is_emmc_pin(pinctrl, pin)) {
                ASSERT(value >= EMMC_PAD_SLEW_RATE_MIN &&
                       value <= EMMC_PAD_SLEW_RATE_MAX);
                sdrv_pinctrl_set_slew_rate_emmc(ctrl->base,
                                                ctrl->pad_reg_offset,
                                                pin, value);
            }
            else {
                ASSERT(value == PAD_SLEW_RATE_FAST ||
                       value == PAD_SLEW_RATE_SLOW);
                sdrv_pinctrl_set_slew_rate_gpio(ctrl->base,
                                                ctrl->pad_reg_offset,
                                                pin, value);
            }
            break;

        case PINCTRL_CONFIG_FORCE_INPUT:
            ASSERT(ctrl->mux_reg_offset);
            ASSERT(value >= PINCTRL_FORCE_INPUT_NORMAL && value <= PINCTRL_FORCE_INPUT_LOW);

            unsigned int fin = PAD_INPUT_FORCE_DISABLE;
            unsigned int fv = PAD_FORCE_INPUT_VALUE_0;

            if (value <= PINCTRL_FORCE_INPUT_ENABLE) {
                fin = value;
            }
            else if (value == PINCTRL_FORCE_INPUT_HIGH) {
                fv = PAD_FORCE_INPUT_VALUE_1;
            }

            sdrv_pinctrl_set_force_input(ctrl->base, ctrl->mux_reg_offset,
                                         pin, fin, fv);
            break;

        default:
            ret = false;
            break;
    }

    return ret;
}

/**
 * @brief Initialize the pinctrl module.
 *
 * @param[in] pinctrl pinctrl device to initialize.
 */
static void _sdrv_pinctrl_init(pinctrl_t *pinctrl)
{
    sdrv_pinctrl_t *ctrl = pinctrl->dev;
    ASSERT(ctrl);

    if (ctrl->emmc_pad_nr > 0) {
        /*
         * When one of the eMMC pads (eMMC1, eMMC2 ) is configured as
         * non-eMMC function, we need to enable the pads manually, by
         * setting mshcX_phy_resetb signals to 1.
         *
         * In eMMC mode the pad reset signals are MUX'ed to eMMC PHY, so
         * we can always enable the mshcX_phy_resetb signals.
         */
        static const scr_signal_t mshc_phy_reset_b_scr[] = {
            SCR_SEC__mshc1_phy_resetb_scr,  /* mshc1 [132:143] */
            SCR_SEC__mshc2_phy_resetb_scr,  /* mshc2 [144:155] */
        };

        for (size_t i = 0; i < ARRAY_SIZE(mshc_phy_reset_b_scr); i++) {
            scr_set(&g_scr_sec, &mshc_phy_reset_b_scr[i], 1);
        }
    }
};

static pinctrl_ops_t sdrv_pinctrl_ops = {
    .init = _sdrv_pinctrl_init,
    .config = _sdrv_pinctrl_config,
};

/**
 * @brief Initialize sdrv pinctrl device.
 *
 * @param[in] pinctrl Common pinctrl device structure.
 * @param[in] dev sdrv pinctrl device data.
 */
void sdrv_pinctrl_init(pinctrl_t *pinctrl, sdrv_pinctrl_t *dev)
{
    pinctrl->dev = dev;
    pinctrl->ops = &sdrv_pinctrl_ops;
    pinctrl_init(pinctrl);
}

/**
 * @brief Enable CANFD pinmux.
 *
 * MUX5 of pin [108:131] could be RX/TX of CANFD 9~20, or DISP DATA. Sideband
 * SCR signals are used to select CANFD or DISP functions.
 *
 * This is function is not standard pinctrl API, but implemented in the
 * pinctrl driver for convenience.
 *
 * @param[in] can_ctrl CANFD controller ID, from 0 (CAN1) to 19 (CAN20).
 * @param[in] enable True to enable CANFD (default function is DISP).
 */
void sdrv_pinctrl_enable_canfd(int can_ctrl, bool enable)
{
    /* Set following signals to 0 for DISP DATA, 1 for CANFD RX/TX. */
 #ifdef CONFIG_BOARD_X9SP
    return;
 #else  /* !CONFIG_BOARD_X9SP */
    static const scr_signal_t disp_canfd_mux_scr[] = {
        /* DISP DATA 0/1, or CANFD9 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data1__oe,
        /* DISP DATA 2/3, or CANFD10 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data3__oe,
        /* DISP DATA 4/5, or CANFD11 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data5__oe,
        /* DISP DATA 6/7, or CANFD12 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data7__oe,
        /* DISP DATA 8/9, or CANFD13 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data9__oe,
        /* DISP DATA 10/11, or CANFD14 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data11__oe,
        /* DISP DATA 12/13, or CANFD15 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data13__oe,
        /* DISP DATA 14/15, or CANFD16 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data15__oe,
        /* DISP DATA 16/17, or CANFD17 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data17__oe,
        /* DISP DATA 18/19, or CANFD18 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data19__oe,
        /* DISP DATA 20/21, or CANFD19 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data21__oe,
        /* DISP DATA 22/23, or CANFD20 RX/TX. */
        SCR_SEC__iomux_wrap_ap_pio_disp_mux__data23__oe,
    };

    if (can_ctrl >= 8 && can_ctrl < 20) {
        /* CANFD9 ~ CANFD20 */
        scr_set(&g_scr_sec, &disp_canfd_mux_scr[can_ctrl - 8],
                (uint32_t)enable);
    }
 #endif /* end CONFIG_BOARD_X9SP */
}
