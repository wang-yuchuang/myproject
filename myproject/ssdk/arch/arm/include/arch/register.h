/*
 * register.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM cp register interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARCH_REGISTER_H
#define INCLUDE_ARCH_REGISTER_H

#include <types.h>
#include <compiler.h>
#include <arch/barriers.h>

static inline uint32_t read_cpsr(void)
{
    uint32_t cpsr;
    __ASM volatile("mrs   %0, cpsr" : "=r" (cpsr));
    return cpsr;
}

#define ARM_CP_REG_FUNCS(cp, reg, op1, c1, c2, op2) \
static inline uint32_t arm_read_##reg(void) { \
    uint32_t val; \
    __ASM volatile("mrc " #cp ", " #op1 ", %0, " #c1 ","  #c2 "," #op2 : "=r" (val)); \
    return val; \
} \
\
static inline void arm_write_##reg(uint32_t val) { \
    __ASM volatile("mcr " #cp ", " #op1 ", %0, " #c1 ","  #c2 "," #op2 :: "r" (val)); \
    ISB; \
}

#define ARM_CP15_REG_FUNCS(reg, op1, c1, c2, op2) \
    ARM_CP_REG_FUNCS(p15, reg, op1, c1, c2, op2)

#define ARM_CP14_REG_FUNCS(reg, op1, c1, c2, op2) \
    ARM_CP_REG_FUNCS(p14, reg, op1, c1, c2, op2)

#define ARM_CP10_REG_FUNCS(reg, op1, c1, c2, op2) \
    ARM_CP_REG_FUNCS(p10, reg, op1, c1, c2, op2)

ARM_CP15_REG_FUNCS(sctlr, 0, c1, c0, 0);
ARM_CP15_REG_FUNCS(actlr, 0, c1, c0, 1);
ARM_CP15_REG_FUNCS(cpacr, 0, c1, c0, 2);

ARM_CP15_REG_FUNCS(ttbr, 0, c2, c0, 0);
ARM_CP15_REG_FUNCS(ttbr0, 0, c2, c0, 0);
ARM_CP15_REG_FUNCS(ttbr1, 0, c2, c0, 1);
ARM_CP15_REG_FUNCS(ttbcr, 0, c2, c0, 2);
ARM_CP15_REG_FUNCS(dacr, 0, c3, c0, 0);
ARM_CP15_REG_FUNCS(dfsr, 0, c5, c0, 0);
ARM_CP15_REG_FUNCS(ifsr, 0, c5, c0, 1);
ARM_CP15_REG_FUNCS(dfar, 0, c6, c0, 0);
ARM_CP15_REG_FUNCS(wfar, 0, c6, c0, 1);
ARM_CP15_REG_FUNCS(ifar, 0, c6, c0, 2);

ARM_CP15_REG_FUNCS(fcseidr, 0, c13, c0, 0);
ARM_CP15_REG_FUNCS(contextidr, 0, c13, c0, 1);
ARM_CP15_REG_FUNCS(tpidrurw, 0, c13, c0, 2);
ARM_CP15_REG_FUNCS(tpidruro, 0, c13, c0, 3);
ARM_CP15_REG_FUNCS(tpidrprw, 0, c13, c0, 4);

ARM_CP15_REG_FUNCS(midr, 0, c0, c0, 0);
ARM_CP15_REG_FUNCS(mpidr, 0, c0, c0, 5);
ARM_CP15_REG_FUNCS(vbar, 0, c12, c0, 0);
ARM_CP15_REG_FUNCS(cbar, 4, c15, c0, 0);

ARM_CP15_REG_FUNCS(ats1cpr, 0, c7, c8, 0);
ARM_CP15_REG_FUNCS(ats1cpw, 0, c7, c8, 1);
ARM_CP15_REG_FUNCS(ats1cur, 0, c7, c8, 2);
ARM_CP15_REG_FUNCS(ats1cuw, 0, c7, c8, 3);
ARM_CP15_REG_FUNCS(ats12nsopr, 0, c7, c8, 4);
ARM_CP15_REG_FUNCS(ats12nsopw, 0, c7, c8, 5);
ARM_CP15_REG_FUNCS(ats12nsour, 0, c7, c8, 6);
ARM_CP15_REG_FUNCS(ats12nsouw, 0, c7, c8, 7);
ARM_CP15_REG_FUNCS(par, 0, c7, c4, 0);

/* Branch predictor invalidate */
ARM_CP15_REG_FUNCS(bpiall, 0, c7, c5, 6);
ARM_CP15_REG_FUNCS(bpimva, 0, c7, c5, 7);
ARM_CP15_REG_FUNCS(bpiallis, 0, c7, c1, 6);

/* tlb registers */
ARM_CP15_REG_FUNCS(tlbiallis, 0, c8, c3, 0);
ARM_CP15_REG_FUNCS(tlbimvais, 0, c8, c3, 1);
ARM_CP15_REG_FUNCS(tlbiasidis, 0, c8, c3, 2);
ARM_CP15_REG_FUNCS(tlbimvaais, 0, c8, c3, 3);
ARM_CP15_REG_FUNCS(itlbiall, 0, c8, c5, 0);
ARM_CP15_REG_FUNCS(itlbimva, 0, c8, c5, 1);
ARM_CP15_REG_FUNCS(itlbiasid, 0, c8, c5, 2);
ARM_CP15_REG_FUNCS(dtlbiall, 0, c8, c6, 0);
ARM_CP15_REG_FUNCS(dtlbimva, 0, c8, c6, 1);
ARM_CP15_REG_FUNCS(dtlbiasid, 0, c8, c6, 2);
ARM_CP15_REG_FUNCS(tlbiall, 0, c8, c7, 0);
ARM_CP15_REG_FUNCS(tlbimva, 0, c8, c7, 1);
ARM_CP15_REG_FUNCS(tlbiasid, 0, c8, c7, 2);
ARM_CP15_REG_FUNCS(tlbimvaa, 0, c8, c7, 3);

ARM_CP15_REG_FUNCS(l2ctlr, 1, c9, c0, 2);
ARM_CP15_REG_FUNCS(l2ectlr, 1, c9, c0, 3);

/* mpu registers (using unified memory regions) */
ARM_CP15_REG_FUNCS(mpuir, 0, c0, c0, 4);
ARM_CP15_REG_FUNCS(rbar, 0, c6, c1, 0);
ARM_CP15_REG_FUNCS(rsr, 0, c6, c1, 2);
ARM_CP15_REG_FUNCS(racr, 0, c6, c1, 4);
ARM_CP15_REG_FUNCS(rgnr, 0, c6, c2, 0);

/* performance monitor registers */
ARM_CP15_REG_FUNCS(pmcr, 0, c9, c12, 0);
ARM_CP15_REG_FUNCS(pmcntenset, 0, c9, c12, 1);
ARM_CP15_REG_FUNCS(pmcntenclr, 0, c9, c12, 2);
ARM_CP15_REG_FUNCS(pmovsr, 0, c9, c12, 3);
ARM_CP15_REG_FUNCS(pmswinc, 0, c9, c12, 4);
ARM_CP15_REG_FUNCS(pmselr, 0, c9, c12, 5);
ARM_CP15_REG_FUNCS(pmccntr, 0, c9, c13, 0);
ARM_CP15_REG_FUNCS(pmxevtyper, 0, c9, c13, 1);
ARM_CP15_REG_FUNCS(pmxevcntr, 0, c9, c13, 2);

/* TCM registers */
ARM_CP15_REG_FUNCS(tcmtr, 0, c0, c0, 2);
ARM_CP15_REG_FUNCS(btcmrgn, 0, c9, c1, 0);
ARM_CP15_REG_FUNCS(atcmrgn, 0, c9, c1, 1);

/* debug registers */
ARM_CP14_REG_FUNCS(dbddidr, 0, c0, c0, 0);
ARM_CP14_REG_FUNCS(dbgdrar, 0, c1, c0, 0);
ARM_CP14_REG_FUNCS(dbgdsar, 0, c2, c0, 0);
ARM_CP14_REG_FUNCS(dbgdscr, 0, c0, c1, 0);
ARM_CP14_REG_FUNCS(dbgdtrtxint, 0, c0, c5, 0);
ARM_CP14_REG_FUNCS(dbgdtrrxint, 0, c0, c5, 0);
ARM_CP14_REG_FUNCS(dbgwfar, 0, c0, c6, 0);
ARM_CP14_REG_FUNCS(dbgvcr, 0, c0, c7, 0);
ARM_CP14_REG_FUNCS(dbgecr, 0, c0, c9, 0);
ARM_CP14_REG_FUNCS(dbgdsccr, 0, c0, c10, 0);
ARM_CP14_REG_FUNCS(dbgdsmcr, 0, c0, c11, 0);
ARM_CP14_REG_FUNCS(dbgdtrrxext, 0, c0, c0, 2);
ARM_CP14_REG_FUNCS(dbgdscrext, 0, c0, c2, 2);
ARM_CP14_REG_FUNCS(dbgdtrtxext, 0, c0, c3, 2);
ARM_CP14_REG_FUNCS(dbgdrcr, 0, c0, c4, 2);
ARM_CP14_REG_FUNCS(dbgvr0, 0, c0, c0, 4);
ARM_CP14_REG_FUNCS(dbgvr1, 0, c0, c1, 4);
ARM_CP14_REG_FUNCS(dbgvr2, 0, c0, c2, 4);
ARM_CP14_REG_FUNCS(dbgbcr0, 0, c0, c0, 5);
ARM_CP14_REG_FUNCS(dbgbcr1, 0, c0, c1, 5);
ARM_CP14_REG_FUNCS(dbgbcr2, 0, c0, c2, 5);
ARM_CP14_REG_FUNCS(dbgwvr0, 0, c0, c0, 6);
ARM_CP14_REG_FUNCS(dbgwvr1, 0, c0, c1, 6);
ARM_CP14_REG_FUNCS(dbgwcr0, 0, c0, c0, 7);
ARM_CP14_REG_FUNCS(dbgwcr1, 0, c0, c1, 7);
ARM_CP14_REG_FUNCS(dbgoslar, 0, c1, c0, 4);
ARM_CP14_REG_FUNCS(dbgoslsr, 0, c1, c1, 4);
ARM_CP14_REG_FUNCS(dbgossrr, 0, c1, c2, 4);
ARM_CP14_REG_FUNCS(dbgprcr, 0, c1, c4, 4);
ARM_CP14_REG_FUNCS(dbgprsr, 0, c1, c5, 4);
ARM_CP14_REG_FUNCS(dbgclaimset, 0, c7, c8, 6);
ARM_CP14_REG_FUNCS(dbgclaimclr, 0, c7, c9, 6);
ARM_CP14_REG_FUNCS(dbgauthstatus, 0, c7, c14, 6);
ARM_CP14_REG_FUNCS(dbgdevid, 0, c7, c2, 7);

/* fpu registers */
ARM_CP10_REG_FUNCS(fpexc, 7, c8, c0, 0);

#endif
