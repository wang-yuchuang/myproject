#ifdef CONFIG_SDRV_SMC

#ifndef SMC_CFG_H_
#define SMC_CFG_H_

#include <driver/taishan/smc/sdrv_smc_drv.h>

extern struct sdrv_smc_ckgen_config g_smc_ckgen_cfg;
extern struct sdrv_smc_rstgen_config g_smc_rstgen_cfg;
extern struct sdrv_smc_pmu_config g_smc_pmu_cfg;
extern struct sdrv_smc_config g_smc_cfg;

#endif /* SMC_CFG_H_ */

#endif /* CONFIG_SDRV_SMC */
