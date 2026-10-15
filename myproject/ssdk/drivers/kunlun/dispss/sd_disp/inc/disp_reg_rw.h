/*
* disp_reg_rw.h
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 10/21/2019 BI create this file
*/
#ifndef __DISP_REG_RW__
#define __DISP_REG_RW__

#include <reg.h>

static inline unsigned int reg_value(unsigned int val,
    unsigned int src, unsigned int shift, unsigned int mask)
{
    return (src & ~mask) | ((val << shift) & mask);
}

static inline unsigned int disp_read(unsigned long base, unsigned int reg)
{
    return readl(base + reg);
}

static inline void disp_write(unsigned long base,
        unsigned int reg, unsigned int value)
{
    writel(value, base + reg);

#if 0
    DISPDBG("w(0x%lx, 0x%08x), r(0x%08x)\n", (base + reg),
            value, readl(base + reg));
#endif
}

#endif //__DISP_REG_RW__