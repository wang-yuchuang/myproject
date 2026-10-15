/*
 * sdrv_eic_drv.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv eic driver.
 *
 * Revision History:
 * -----------------
 */
#include <compiler.h>
#include <driver/kunlun/eic/sdrv_eic_drv.h>
#include <spinlock.h>
#include <string.h>
#include <types.h>

#include "debug.h"
#include "kunlun_eic.h"
#include "regs_base.h"

#define EXTRA_FMT "sdrv_eic:"
#define SDRV_EIC_ERR(format, ...)                                              \
    ssdk_printf(SSDK_ERR, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_EIC_WARN(format, ...)                                             \
    ssdk_printf(SSDK_WARNING, EXTRA_FMT format, ##__VA_ARGS__)
#define SDRV_EIC_INFO(format, ...)                                             \
    ssdk_printf(SSDK_INFO, EXTRA_FMT format, ##__VA_ARGS__)

/**
 * @brief EIC error injection for others IP.
 *
 * @param[in] dev      EIC device.
 * @param[in] cfg      EIC error injection signal configuration.
 * @param[in] en       Enable or disable error injection.
 */
uint32_t sdrv_eic_err_injection(eic_dev_t *dev,
                                const eic_err_inj_sig_config_t *cfg, bool en)
{
    uint8_t eic_enum = 0;

    switch (dev->base) {
    case APB_EIC_SAF_BASE:
        eic_enum = 0;
        break;
    case APB_EIC_SEC_BASE:
        eic_enum = 1;
        break;
    case APB_EIC_HPI_BASE:
        eic_enum = 2;
        break;
#if CONFIG_BOARD_X9SP
    case EIC_VSN_BASE:
#else
    case APB_EIC_VSN_BASE:
#endif
        eic_enum = 3;
        break;
    }

    /* enable error injection */
    eic_enable_injectpoint(eic_enum, *cfg->err_inj_sig, en);

    /* toggle 1 bit to inject error */
    eic_inject(*cfg->err_inj_sig, cfg->err_inj_bit);

    return 0;
}
