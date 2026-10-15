/**
 * @file pcie.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pcie header file.
 *
 * Revision History:
 * -----------------
 */

#ifndef PCIE_H_
#define PCIE_H_

#define PCIE_BUS(bdf)   (((bdf) >> 8) & 0xFF)
#define PCIE_DEV(bdf)   (((bdf) >> 3) & 0x1F)
#define PCIE_FUNC(bdf)  ((bdf) & 7)

#define MAKE_BDF(bus, dev, func)    ((((bus) & 0xFF) << 8) | \
                                     (((dev) & 0x1F) << 3) | \
                                     ((func) & 7))

#define VENDOR_REG_NUM      0
#define COMMAND_REG_NUM     1
#define CLASS_CODE_REG_NUM  2
#define BAR_REG_NUM(n)      (4 + (n))

#define REG_OFFSET(n)   ((n) << 2)

#ifndef ASSEMBLY

#include <stdint.h>
#include <stdbool.h>
#include <compiler.h>
#include "cmsis_os2.h"

typedef enum pcie_mode {
    PCIE_RC_MODE,
    PCIE_EP_MODE
} pcie_mode_t;

typedef enum pcie_link_width {
    LINK_WIDTH_DEFAULT, /* 1 lane */
    LINK_WIDTH_1_LANE = 1,
    LINK_WIDTH_2_LANES = 2,
    LINK_WIDTH_4_LANES = 4,
    LINK_WIDTH_8_LANES = 8
} pcie_link_width_t;

typedef enum pcie_link_speed {
    LINK_SPEED_DEFAULT, /* Gen 1 */
    LINK_SPEED_GEN1,
    LINK_SPEED_GEN2,
    LINK_SPEED_GEN3
} pcie_link_speed_t;

typedef enum pcie_bar_addr_type {
    BAR_32BIT,
    BAR_64BIT
} pcie_bar_addr_type_t;

typedef struct pcie_dev pcie_dev_t;

typedef struct pcie_ops {
    /*
     * Enable PCIe LTSSM to establish link.
     */
    int (*establish_link)(pcie_dev_t *dev);

    /*
     * Read PCIe configuration header register
     * @bdf bus, device and function
     * @offset register offset
     * @val variable to store the reading value
     */
    int (*read_config)(pcie_dev_t *dev, int bdf, uint32_t offset,
                       uint32_t *val);

    /*
     * Write PCIe configuration header register
     * @bdf bus, device and function. RC locates at bus 0,
     * EP locates at the subsequent buses
     * @offset register offset
     * @val writing value
     */
    int (*write_config)(pcie_dev_t *dev, int bdf, uint32_t offset,
                        uint32_t val);


    int (*set_bar)(pcie_dev_t *dev, uint32_t bar_index, uint64_t bar_base, 
                            uint32_t is_bar64);
    
    int (*enable_all_bar)(pcie_dev_t *dev);
   
    void (*rc_set_dma_addr)(pcie_dev_t *dev, unsigned long dma_addr);
    
    int (*dma_read)(pcie_dev_t *dev, int dma_channel, uint64_t dst, 
                            uint64_t src, uint32_t len);
    
    int (*dma_write)(pcie_dev_t *dev, int dma_channel, uint64_t dst, 
                            uint64_t src, uint32_t len);

} pcie_ops_t;

struct pcie_dev {
    int controller;
    int irq;
    pcie_mode_t mode;
    osMutexId_t lock;
    const pcie_ops_t *ops;
    void *priv;
};

__BEGIN_CDECLS

int pcie_establish_link(pcie_dev_t *dev);
int pcie_ep_set_class(pcie_dev_t *dev, uint8_t bus, uint8_t device,
                      uint8_t function, uint32_t class_code);
int pcie_config_bar(pcie_dev_t *dev, uint8_t bus, uint8_t device,
                    uint8_t function, uint32_t index, uint32_t base);
int pcie_scan_bus(pcie_dev_t *dev);


int pcie_set_bar(pcie_dev_t *dev, uint32_t bar_index, uint64_t bar_base, uint32_t is_bar64);
int pcie_enable_all_bar(pcie_dev_t *dev);
int pcie_rc_set_dma_addr(pcie_dev_t *dev, unsigned long dma_addr);
int pcie_dma_read(pcie_dev_t *dev, int dma_channel, uint64_t dst, 
                            uint64_t src, uint32_t len);
int pcie_dma_write(pcie_dev_t *dev, int dma_channel, uint64_t dst, 
                            uint64_t src, uint32_t len);
__END_CDECLS

#endif // !ASSEMBLY

#endif // !PCIE_H_
