/**
 * @file mmc_dwcmshc.h
 * @brief dwc mshc sdhci host driver header.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef MMC_DWCMSHC_H_
#define MMC_DWCMSHC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <driver/scr.h>

struct mmc_dwcmshc {
    scr_dev_t *scr_base;
    scr_signal_t  ddr_mode_scr; /* scr data */
};


#ifdef __cplusplus
}
#endif
#endif /* MMC_DWCMSHC_H_ */
