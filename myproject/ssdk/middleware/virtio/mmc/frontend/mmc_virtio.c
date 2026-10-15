#include <debug.h>
#include <driver/kunlun/mmc/mmc_sdhci.h>

extern uint32_t vsdhci_send_command(struct sdhci_host *host, struct mmc_command *cmd);
extern void vsdhci_init(struct sdhci_host *host);
extern void vsdhci_reinit(struct sdhci_host *host, uint8_t voltage);

static uint32_t vmmc_set_clk(struct sdhci_host *host, unsigned int clock)
{
    return 0;
}

static void vmmc_init(struct sdhci_host *host)
{
    return;
}

static void vmmc_hw_reset(struct sdhci_host *host)
{
    return;
}

static void vmmc_set_uhs_mode(struct sdhci_host *host, uint32_t mode)
{
    return;
}

static void vmmc_hs400_enhanced_strobe(struct sdhci_host *host, uint32_t enable)
{
    return;
}

static uint8_t vmmc_set_bus_width(struct sdhci_host *host, uint16_t width)
{
    return 0;
}

struct sdhci_ops mmc_virtio_ops = {
    .host_init = vsdhci_init,
    .host_reinit = vsdhci_reinit,
    .priv_init = vmmc_init,
    .hw_reset = vmmc_hw_reset,
    .set_clock = vmmc_set_clk,
    .set_uhs_mode = vmmc_set_uhs_mode,
    .hs400_enhanced_strobe = vmmc_hs400_enhanced_strobe,
    .set_bus_width = vmmc_set_bus_width,
    .send_command = vsdhci_send_command,
};

int vmmc_platform_init(struct mmc_device *dev, void *priv_data)
{
    struct sdhci_host *host;

    host = &dev->host;
    host->ops = &mmc_virtio_ops;
    host->parent = dev;

    if (mmc_sdhci_init(dev)) {
        ssdk_printf(SSDK_CRIT, "mmc init failed!\n");
        return -1;
    }

    return 0;
}
