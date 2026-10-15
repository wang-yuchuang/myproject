/**
 * @file spinlock_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: spinlock interface Tester.
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cmsis_os2.h>
#include <CLI_type.h>

#include <board.h>
#include <driver/irq.h>
#include <arch/smp.h>
#include <spinlock.h>

spin_lock_t g_spinlock;
volatile int flag = 0;

int spinlock_irq_handler(uint32_t irq, void *arg)
{
	printf("ipi irq test start, core id:%d, irq:%u\n", arch_curr_cpu_num(), irq);

	flag = 1;
	/* get spinlock cpu1 would spin here until unlock */
	spin_lock(&g_spinlock);

	printf("ipi irq test end, core id:%d, irq:%u\n", arch_curr_cpu_num(), irq);
	spin_unlock(&g_spinlock);
	return 0;
}

static int spinlock_test(int argc, char *argv[])
{
	int i;

	/* register all ipi interrupt to IPI handler */
	for(i = 0; i < NR_IPI; i++)
		irq_attach(i, spinlock_irq_handler, NULL);

	spin_lock_init(&g_spinlock);

	/* get spinlock */
	spin_lock(&g_spinlock);

	/* send ipi1 to cpu1 */
	smp_cross_call(0x2, IPI_1);

	/* wait for interrutp */
	while (1) {
		udelay(10000UL);
		if (flag)
			break;
	}
	printf("unlock, %s\n", __func__);
	spin_unlock(&g_spinlock);

	return 0;
}

CLI_CMD("spinlock_test", "spinlock test", spinlock_test);
