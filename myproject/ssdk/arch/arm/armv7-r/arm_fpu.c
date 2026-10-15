/*
 * arm_fpu.c
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM fpu function.
 *
 * Revision History:
 * -----------------
 */

#ifdef CONFIG_ARCH_WITH_FPU

#include <arch/register.h>
#include <arch/fpu.h>
#include <debug.h>

/* CPACR */

#define CPACR_CP10_SHIFT    (20)
#define CPACR_CP10_MASK     (3 << CPACR_CP10_SHIFT)
#define CPACR_CP11_SHIFT    (22)
#define CPACR_CP11_MASK     (3 << CPACR_CP11_SHIFT)

/* FPEXC */

#define FPEXC_EX            (31)
#define FPEXC_EN            (30)

/*
 * fpu enable.
 */
void arm_fpu_enable(void)
{
    uint32_t cpacr = arm_read_cpacr();
    cpacr |= ((3 << CPACR_CP10_SHIFT) | (3 << CPACR_CP11_SHIFT));
    arm_write_cpacr(cpacr);

    uint32_t fpexc = arm_read_fpexc();
    fpexc |= (1 << FPEXC_EN);
    arm_write_fpexc(fpexc);
}

/*
 * fpu disable.
 */
void arm_fpu_disable(void)
{
    uint32_t fpexc = arm_read_fpexc();
    fpexc &= ~(1 << FPEXC_EN);
    arm_write_fpexc(fpexc);
}

#endif