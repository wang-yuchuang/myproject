/**
 * @file pcie.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pcie driver.
 *
 * Revision History:
 * -----------------
 */

#include <debug.h>
#include "driver/kunlun/pcie/pcie.h"

int pcie_establish_link(pcie_dev_t *dev)
{
    if (dev && dev->ops && dev->ops->establish_link)
        return dev->ops->establish_link(dev);
    else
        return -1;
}

int pcie_ep_set_class(pcie_dev_t *dev, uint8_t bus, uint8_t device,
                      uint8_t function, uint32_t class_code)
{
    if (dev && dev->ops && dev->ops->read_config && dev->ops->write_config) {
        int bdf = MAKE_BDF(bus, device, function);
        uint32_t offset = REG_OFFSET(CLASS_CODE_REG_NUM);
        uint32_t val;

        osMutexAcquire(dev->lock, osWaitForever);
        if (dev->ops->read_config(dev, bdf, offset, &val))
            return -1;

        val &= ~0xFFFFFF00;
        val |= class_code << 8;
        return dev->ops->write_config(dev, bdf, offset, val);
        osMutexRelease(dev->lock);
    }
    else
        return -1;
}

int pcie_config_bar(pcie_dev_t *dev, uint8_t bus, uint8_t device,
                     uint8_t function, uint32_t index, uint32_t base)
{
    if (dev && dev->ops && dev->ops->write_config) {
        int bdf = MAKE_BDF(bus, device, function);
        uint32_t offset = REG_OFFSET(BAR_REG_NUM(index));

        osMutexAcquire(dev->lock, osWaitForever);
        int ret = dev->ops->write_config(dev, bdf, offset, base);
        osMutexRelease(dev->lock);

        return ret;
    }
    else
        return -1;
}

int pcie_scan_bus(pcie_dev_t *dev)
{
    /*
     * TODO: not support enumeration,
     * should be fixed.
     */
    if (dev && dev->ops && dev->ops->read_config) {
        int bdf = MAKE_BDF(1, 0, 0);
        uint32_t offset = REG_OFFSET(VENDOR_REG_NUM);
        uint32_t val;

        osMutexAcquire(dev->lock, osWaitForever);
        int ret = dev->ops->read_config(dev, bdf, offset, &val);
        if (!ret && ((val >> 16) != 0xFFFF)) {
            ssdk_printf(SSDK_NOTICE, "EP vendor ID 0x%x, device ID 0x%x\n",
                        val & 0xFFFF, val >> 16);
            offset = REG_OFFSET(COMMAND_REG_NUM);
            ret = dev->ops->read_config(dev, bdf, offset, &val);
            if (!ret) {
                /* Enable bus master, memory response and IO response. */
                val |= 7;
                ret = dev->ops->write_config(dev, bdf, offset, val);
            }
        }
        osMutexRelease(dev->lock);

        return ret;
    }
    else
        return -1;
}


int pcie_dma_read(pcie_dev_t *dev, int dma_channel, uint64_t dst,
                            uint64_t src, uint32_t len)
{
	if (dev && dev->ops && dev->ops->dma_read)
	{
		return dev->ops->dma_read(dev, dma_channel, dst, src, len);
	}
	else
		return -1;
}

int pcie_dma_write(pcie_dev_t *dev, int dma_channel, uint64_t dst,
                            uint64_t src, uint32_t len)
{
	if (dev && dev->ops && dev->ops->dma_write)
	{
		return dev->ops->dma_write(dev, dma_channel, dst, src, len);
	}
	else
		return -1;
}
