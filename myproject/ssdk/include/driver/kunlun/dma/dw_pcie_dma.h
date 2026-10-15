/**
 * @file dw_pcie_dma.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dwc pcie dma header file.
 *
 * Revision History:
 * -----------------
 */

#ifndef _DW_PCIE_DMA_H_

#ifndef ASSEMBLY

#include "driver/dma.h"

__BEGIN_CDECLS

void dw_pcie_dma_init(dma_dev_t *dev);

__END_CDECLS

#endif

#endif // !_DW_PCIE_DMA_H_
