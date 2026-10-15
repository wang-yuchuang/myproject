
#ifndef _SDRV_SD_SPI_H_
#define _SDRV_SD_SPI_H_

#include <driver/spi.h>

struct spi_config {
    uint32_t idle;
    uint32_t timeout;
    uint32_t tim_ctrl;
    uint32_t bus_flags;
    uint16_t slave_nss_pol;
    uint32_t dev_flags;
    uint32_t dev_nss_idx;
    void *dmac;
};

int32_t sdrv_spi_controller_probe(struct spi_controller *controller,
                                  const struct spi_config *config,
                                  const struct spi_controller_config *cfg);

#endif