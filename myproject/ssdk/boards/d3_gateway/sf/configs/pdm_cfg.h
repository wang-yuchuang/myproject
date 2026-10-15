/**
 * @file pdm_cfg.h
 * @brief
 * @version 0.1
 * @date 2021-12-04
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 *
 */

#ifndef BOARDS_SACI_PDM_H
#define BOARDS_SACI_PDM_H

#ifdef CONFIG_SACI_PDM
extern snd_dev_t *pdm1_dev;
extern snd_dev_t *pdm2_dev;
extern void board_pdm_init(void);

#endif

#endif /* BOARDS_I2S_CFG_H */
