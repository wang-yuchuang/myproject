#ifndef _REG_PFM_H
#define _REG_PFM_H

#include <bits.h>
#include <reg.h>
#include <stdio.h>
#include <regs_base.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

#if WITH_KERNEL_VM
# define v2p(va)        (paddr_t)(vaddr_to_paddr(va))
# define p2v(pa)        (vaddr_t)(paddr_to_kvaddr(pa))
#else
# define v2p(va)        (paddr_t)(va)
# define p2v(pa)        (vaddr_t)(pa)
#endif

#define _vaddr(pa)      p2v(pa)
#define _paddr(va)      v2p(va)
#define _ioaddr(pa)     _vaddr(pa)

#define PERF_MON_BASE (vaddr_t)_ioaddr(APB_DDR_PERF_MON_BASE)

#define PFM_MON_BASE_CNT_CTL       PERF_MON_BASE
#define PFM_MON_BASE_CNT_CMP       PERF_MON_BASE + 4

#define PFM_MON_MISC_WDG_CTL       PERF_MON_BASE + 0x70

#define PFM_MON_MISC_ST            PERF_MON_BASE + 0xA0
#define PFM_MON_MISC_INT_CTL       PERF_MON_BASE + 0xA4
#define PFM_MON_AXI_CNT_CTL0       PERF_MON_BASE + 0x100
#define PFM_MON_AXI_CNT_CTL1       PERF_MON_BASE + 0x104

#define PFM_MON_AXI_RD_THR(i)      PERF_MON_BASE + 0x110 + i*16
#define PFM_MON_AXI_WR_THR(i)      PERF_MON_BASE + 0x114 + i*16
#define PFM_MON_AXI_RD_BCNT_THR(i) PERF_MON_BASE + 0x118 + i*16
#define PFM_MON_AXI_WR_BCNT_THR(i) PERF_MON_BASE + 0x11C + i*16

#define PFM_MON_AXI_ID0(i)         PERF_MON_BASE + 0x200 + i*16
#define PFM_MON_AXI_ID0_MSK(i)     PERF_MON_BASE + 0x204 + i*16
#define PFM_MON_AXI_ID1(i)         PERF_MON_BASE + 0x208 + i*16
#define PFM_MON_AXI_ID1_MSK(i)     PERF_MON_BASE + 0x20C + i*16

#define PFM_MON_AXI_RND_CNT        PERF_MON_BASE + 0x1000
#define PFM_MON_AXI_CNT_ST         PERF_MON_BASE + 0x1004

#define PFM_MON_AXI_RD_CNT(i)      PERF_MON_BASE + 0x1010 + i*24
#define PFM_MON_AXI_WR_CNT(i)      PERF_MON_BASE + 0x1014 + i*24
#define PFM_MON_AXI_RD_BCNTL(i)    PERF_MON_BASE + 0x1018 + i*24
#define PFM_MON_AXI_RD_BCNTH(i)    PERF_MON_BASE + 0x101C + i*24
#define PFM_MON_AXI_WR_BCNTL(i)    PERF_MON_BASE + 0x1020 + i*24
#define PFM_MON_AXI_WR_BCNTH(i)    PERF_MON_BASE + 0x1024 + i*24

#define PFM_MON_GROUP                    PFM_MON_AXI_CNT_CTL0


#define PFM_MON_ENABLE_SHIFT             (0U)
#define PFM_MON_ENABLE_MASK              (uint32_t)(1 << PFM_MON_ENABLE_SHIFT)

#define PFM_MON_CLEAR_REG_SHIFT          (1U)
#define PFM_MON_CLEAR_REG_MASK           (uint32_t)(1 << PFM_MON_CLEAR_REG_SHIFT)

#define PFM_MON_BASE_CNT_TIMEOUT_SHIFT   (4U)
#define PFM_MON_BASE_CNT_TIMEOUT_MASK    (uint32_t)(1 << PFM_MON_BASE_CNT_TIMEOUT_SHIFT)

#define PFM_MON_GROUP_SHIFT              (16U)
#define PFM_MON_GROUP_MSK                (uint32_t)(1 << PFM_MON_GROUP_SHIFT - 1)

#define PFM_MON_REACH_THR_SHIFT          (8U)
#define PFM_MON_REACH_THR_MSK            (uint32_t)(1 << PFM_MON_REACH_THR_SHIFT - 1)

#define PFM_MON_BCNTH_SHIFT              (8u)
#define PFM_MON_BCNTH_MSK                (uint32_t)(1 << PFM_MON_BCNTH_SHIFT - 1)

#endif
