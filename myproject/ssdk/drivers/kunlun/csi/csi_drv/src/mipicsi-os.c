/*
 * mipicsi-os.c
 *
 * Semidrive platform mipi csi2 operation
 *
 * Copyright (C) 2022, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */

#include <driver/kunlun/csi/cam_res.h>
#include "sdrv-cam-os-def.h"
#include "mipicsi-ip.h"
#include "mipicsi-os.h"
#include "sd_errcode.h"

struct mipicsi_device {
	osMutexId_t lock;
	uint32_t inited;
	uint32_t irq_id;
	uint32_t stream_num;
	struct csi_mipi_dev core;
};

#ifdef _CONFIG_OS_SAFETY_
static enum handler_return mipicsi_int_handler(void *data)
{
	enum handler_return ret = INT_NO_RESCHEDULE;
	struct mipicsi_device *dev = (struct mipicsi_device *)data;

	mipi_csi_irq(&dev->core);
	return ret;
}

static uint32_t get_irq(uint32_t id)
{
	if (id == 0)
		return MIPI_CSI1_INTERRUPT_NUM;
	else
		return MIPI_CSI2_INTERRUPT_NUM;
}

static reg_addr_t get_reg_base(uint32_t id)
{
	uint32_t res;
	int32_t ret, res_index = 0;
	addr_t phy_addr = 0;

	if (id == 0)
		res = RES_MIPI_CSI_MIPI_CSI1;
	else
		res = RES_MIPI_CSI_MIPI_CSI2;

	ret = res_get_info_by_id(res, &phy_addr, &res_index);
	if (ret < 0) {
		cam_err("%s: get resource 0x%x fail, id=%d, ret=%d\n", __func__,
			res, id, ret);
		ERRCODE_PRINT(MOD_ID_SYS_SERVICE_CAMERA,MIPI_MOD,ERROR_MIPI_GETRES_FAIL,id);
		return NULL;
	}

	cam_info("%s: mipicsi%d, base 0x%lx\n", __func__, id,
		 (unsigned long)phy_addr);
	return (reg_addr_t)phy_addr;
}

static reg_addr_t get_dispmux_base(uint32_t id)
{
	uint32_t paddr = APB_DISP_MUX_BASE;
	reg_addr_t vaddr;
#if WITH_KERNEL_VM
	vaddr = (reg_addr_t)paddr_to_kvaddr(paddr);
#else
	vaddr = (reg_addr_t)paddr;
#endif

	return vaddr;
}
#else /* _CONFIG_OS_SSDK_ */

#define get_irq(id) \
	((id == 0) ? MIPI_CSI1_INTR_NUM : MIPI_CSI2_INTR_NUM)
#define get_reg_base(id) \
	((id == 0) ? APB_MIPI_CSI1_BASE : APB_MIPI_CSI2_BASE)
#define get_dispmux_base(id) APB_DISP_MUX_BASE

static int mipicsi_int_handler(uint32_t irq, void *data)
{
	struct mipicsi_device *dev = (struct mipicsi_device *)data;
	mipi_csi_irq(&dev->core);
	return 0;
}
#endif

spin_lock_t init_lock = SPIN_LOCK_INITIAL_VALUE;
static struct mipicsi_device s_mipi_devices[MAX_MIPICSI_INST];

int mipicsi_init(uint32_t id, void *data)
{
	struct mipicsi_device *dev;
	struct mipi_config *cfg;
	struct csi_vch_info *ipi;

	if (data == NULL) {
		cam_err("%s: fail to get cfg ptr\n", __func__);
		return -1;
	}
	if (id == 0)
		dev = &s_mipi_devices[0];
	else
		dev = &s_mipi_devices[1];

	spin_lock(&init_lock);
	if (dev->inited) {
		spin_unlock(&init_lock);
		return 0;
	}

	dev->irq_id = get_irq(id);
	register_int_handler(dev->irq_id, mipicsi_int_handler, dev);
	unmask_interrupt(dev->irq_id);

	cfg = (struct mipi_config *)data;
	dev->core.id = id;
	dev->core.base = (reg_addr_t)get_reg_base(id);
	dev->core.dispmux_base = (reg_addr_t)get_dispmux_base(id);
	dev->core.lane_num = cfg->lane_num;
	dev->core.lanerate = cfg->lanerate;

	cam_info("mipicsi%d init, lane %d, rate %d, output 0x%02x\n", id,
		 cfg->lane_num, cfg->lanerate, cfg->output_type);

	for (int i = 0; i < 4; i++) {
		ipi = &dev->core.vch_hw[i];
		ipi->output_type = cfg->output_type;
		ipi->hsa = cfg->hsa;
		ipi->hbp = cfg->hbp;
		ipi->hsd = cfg->hsd;
		ipi->virtual_ch = i;
		ipi->ipi_mode = IPI_MODE_CUT_THROUGH | IPI_MODE_ENABLE;
		ipi->adv_feature = IPI_ADV_SYNC_LEGACCY | IPI_ADV_USE_VIDEO |
				   IPI_ADV_SEL_LINE_EVENT;
	}

	dev->inited = 1;
	spin_unlock(&init_lock);
	dev->lock = osMutexNew(NULL);

	return 0;
}

int mipicsi_stream(uint32_t id, int on)
{
	struct mipicsi_device *dev;

	cam_info("mipicsi%d, stream_%s\n", id, on ? "on" : "off");
	if (id == 0)
		dev = &s_mipi_devices[0];
	else
		dev = &s_mipi_devices[1];

	osMutexAcquire(dev->lock, 0);
	if (on) {
		dev->stream_num++;
		if (dev->stream_num == 1) {
			mipi_csi2_initialization(&dev->core);
			mipi_csi2_enable(&dev->core, 1);
		}
	} else {
		dev->stream_num--;
		if (dev->stream_num == 0)
			mipi_csi2_enable(&dev->core, 0);
	}
	osMutexRelease(dev->lock);

	return 0;
}
