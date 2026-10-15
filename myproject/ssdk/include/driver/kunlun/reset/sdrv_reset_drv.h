/**
 * @file sdrv_reset_drv.h
 * @brief semidrive reset driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */
#ifndef SDRV_RESET_DRV_H_
#define SDRV_RESET_DRV_H_

#include <types.h>
#include "driver/reset.h"
#include "rstgen_id.h"

/**
 * @brief Abstract semidrive rstgen device.
*/
typedef struct sdrv_reset_ctl {
    paddr_t        base;

    uint32_t    self_rst_en : 1;
    uint32_t    sem_rst_en  : 1;
    uint32_t    dbg_rst_en  : 1;
    uint32_t    wdg1_rst_en : 1;
    uint32_t    wdg2_rst_en : 1;
    uint32_t    wdg3_rst_en : 1;
    uint32_t    wdg4_rst_en : 1;
    uint32_t    wdg5_rst_en : 1;
    uint32_t    wdg6_rst_en : 1;
    uint32_t    wdg7_rst_en : 1;
    uint32_t    wdg8_rst_en : 1;
} sdrv_reset_ctl_t;

/**
 * @brief Initialize the  SemiDrive RESET controller.
 *
 * @param[in] rst_ctl Common reset device.
 * @param[in] sdrv_rst_ctl SemiDrive reset device.
 * @return 0 if OK, or a negative error code.
 */
int32_t sdrv_reset_init(reset_ctl_t *rst_ctl, sdrv_reset_ctl_t *sdrv_rst_ctl);
#endif /* SDRV_RESET_DRV_H_ */
