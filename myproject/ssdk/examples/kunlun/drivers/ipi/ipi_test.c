/**
 * @file irq_test.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Interrupt Nesting Tester.
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

int ipi_irq_handler(uint32_t irq, void *arg)
{
	printf("ipi irq test, core id:%d, irq:%u\n", arch_curr_cpu_num(), irq);
	return 0;
}

static int ipi_test(int argc, char *argv[])
{
	int i;

	/* register all ipi interrupt to IPI handler */
	for(i = 0; i < NR_IPI; i++)
		irq_attach(i, ipi_irq_handler, NULL);

	/* send ipi0 to self */
	smp_cross_call(0x1, IPI_0);

	/* send ipi1 to cpu1 */
	smp_cross_call(0x2, IPI_1);

	/* send ipi2 to cpu2 */
	smp_cross_call(0x4, IPI_2);

	/* send ipi3 to cpu3 */
	smp_cross_call(0x8, IPI_3);

	/* send ipi4 to all cpu */
	smp_cross_call(0xf, IPI_1);

	/*
	for(i = 0; i < NR_IPI; i++)
		irq_detach(i);
	*/

	return 0;
}

CLI_CMD("ipi_test", "ipi interrutp test", ipi_test);
