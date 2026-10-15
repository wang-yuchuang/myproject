// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2022
 * Huafeng.huang <huafeng.huang@semidrive.com>
 */


#ifndef _INCLUDE_SMP_H
#define _INCLUDE_SMP_H

#ifdef CONFIG_ARCH_WITH_SMP

#define GIC_IPI_BASE (0)

/* SGI irq num, max 15 */
enum ipi_msg_type {
	IPI_0,
	IPI_1,
	IPI_2,
	IPI_3,
	IPI_4,
	NR_IPI=16
};

static inline unsigned int arch_curr_cpu_num(void)
{
	uint64_t mpidr;

	__ASM volatile("mrs %0, mpidr_el1" : "=r" (mpidr) : : "cc");
	return (((mpidr & ((1U << CONFIG_SMP_CPU_ID_BITS) - 1)) >> 16) \
		<< CONFIG_SMP_CPU_CLUSTER_SHIFT) | ((mpidr & 0xff00) >> 8);
}

void boot_other_cpu(int corenum, uint64_t addr);
void arm64_secondary_entry(uint32_t cpu);
void smp_bringup_initialize(void);
void kick_cpucore(int corenum, uint64_t addr);
void second_cpu_up(uint32_t cpu);
#else
static inline unsigned int arch_curr_cpu_num(void)
{
	return 0;
}
void smp_bringup_initialize(void) {}
void arm64_secondary_entry(uint32_t cpu) {}
void boot_other_cpu(int corenum, uint64_t addr) {}
void kick_cpucore(int corenum, uint64_t addr) {}
void second_cpu_up(uint32_t cpu) {}
#endif
#endif

