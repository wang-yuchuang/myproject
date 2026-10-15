#pragma once
#include <stdint.h>
#include "driver/kunlun/mmc/mmc_sdhci.h"

/* mmc frontend platform init*/
int vmmc_platform_init(struct mmc_device *dev, void *priv_data);
