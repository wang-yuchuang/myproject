/**
 * @file board.c
 * @brief board config source file.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <arch.h>
#include <cmsis_os2.h>
#include <driver/uart.h>
#include <driver/arch_timer.h>
#include <driver/taishan/fuse/fuse.h>
#include <board.h>
#include <debug.h>

#ifdef CONFIG_ARCH_WITH_SMP
#include <driver/clk.h>
#include <driver/scr.h>
#include <driver/reset.h>
#include <scr_hw.h>
#include <lib/tiny-printf.h>
#include <bits.h>
#endif

#ifdef __GNUC__
extern const char build_msg[];
#endif

static uart_dev_t *board_console = NULL;

extern int main(void);

__WEAK void board_early_init(void)
{
}

__WEAK void board_init(void)
{
}

__WEAK void board_set_console(void *dev)
{
    board_console = (uart_dev_t *)dev;
}

__WEAK void * board_get_console(void)
{
    return board_console;
}

__WEAK char board_getc(void)
{
#ifdef CONFIG_UART
    if (board_console)
        return uart_getc(board_console);
    else
#endif
        return '\0';
}

__WEAK void board_putc(char c)
{
    if (board_console) {
#ifdef CONFIG_UART
        if (c == '\n')
            uart_putc(board_console, '\r');
        uart_putc(board_console, c);
#endif
    }
}

__WEAK void board_raw_putc(char c)
{
    if (board_console) {
#ifdef CONFIG_UART
        if (c == '\n')
            uart_raw_putc(board_console, '\r');
        uart_raw_putc(board_console, c);
#endif
    }
}

__WEAK void udelay(uint32_t usecs)
{
#ifdef CONFIG_TIMER
    arch_udelay(usecs);
#else
    volatile uint64_t count;
    count = CONFIG_BOARD_LOOPSPERUSEC * (uint64_t)usecs;
    while (count > 0)
        count--;
#endif
}

__WEAK void mdelay(uint32_t msecs)
{
    if (arch_irq_is_masked() ||
        arch_in_irq_mode() ||
        osKernelGetState() != osKernelRunning) {
        udelay(msecs * 1000);
    }
    else {
        // os is running and in task context
        osDelay(msecs);
    }
}

__WEAK uint32_t current_time(void)
{
#ifdef CONFIG_TIMER
    return arch_timer_get_current() / 1000;
#else
    return 0;
#endif
}

__WEAK uint64_t current_time_hires(void)
{
#ifdef CONFIG_TIMER
    return arch_timer_get_current();
#else
    return 0;
#endif
}

__WEAK void board_halt(board_halt_action action,
                       board_halt_reason reason)
{
    arch_irq_disable();
    for (;;);
}

static void _main_entry(void *argument)
{
    main();

    ssdk_printf(SSDK_EMERG, "SSDK Main Startup Success!\n");

#ifndef CONFIG_OS_POLL
    osThreadExit();
#else
    osThreadTerminate(osThreadGetId());
#endif
}

static void call_constructors(void)
{
#if __GNUC__
    extern void (*__ctor_list[])(void);
    extern void (*__ctor_end[])(void);
    void (**ctor)(void);

    for (ctor = __ctor_list; ctor != __ctor_end; ctor++)
        (*ctor)();
#elif __ICCARM__
    extern void *__iar_cstart_call_ctors(void *);
    __iar_cstart_call_ctors(NULL);
#endif
}

#ifdef CONFIG_ARCH_WITH_SMP

static scr_signal_t g_smp_core_scr[] = {
	SCR_SEC__cpu1_rvbaraddr0_39_30,
	SCR_SEC__cpu1_rvbaraddr0_29_2,
	SCR_SEC__cpu1_rvbaraddr1_39_30,
	SCR_SEC__cpu1_rvbaraddr1_29_2,
	SCR_SEC__cpu1_rvbaraddr2_39_30,
	SCR_SEC__cpu1_rvbaraddr2_29_2,
	SCR_SEC__cpu1_rvbaraddr3_39_30,
	SCR_SEC__cpu1_rvbaraddr3_29_2,
};

extern scr_dev_t g_scr_sec;
extern clk_node_t g_ckgen_gating_cpu1_clk0;
extern clk_node_t g_ckgen_gating_cpu1_clk1;
extern clk_node_t g_ckgen_gating_cpu1_clk2;
extern clk_node_t g_ckgen_gating_cpu1_clk3;
extern reset_sig_t rstsig_cpu1_core0_warm;
extern reset_sig_t rstsig_cpu1_core1_warm;
extern reset_sig_t rstsig_cpu1_core2_warm;
extern reset_sig_t rstsig_cpu1_core3_warm;

void kick_cpucore(int corenum, uint64_t addr)
{
	scr_signal_t *hi_res_id, *lo_res_id;
	clk_node_t *clk;
	reset_sig_t *reset;

	if (corenum > CONFIG_SMP_MAX_CPUS -1 ) {
		early_printf("cpu core overflow, max cpu count:%d, corenum:%d\n", CONFIG_SMP_MAX_CPUS, corenum);
		return;
	}

	hi_res_id = &g_smp_core_scr[corenum * 2];
	lo_res_id = &g_smp_core_scr[(corenum * 2) + 1];

	switch (corenum) {
		case 0:
			clk = &g_ckgen_gating_cpu1_clk0;
			reset = &rstsig_cpu1_core0_warm;
			break;
		case 1:
			clk = &g_ckgen_gating_cpu1_clk1;
			reset = &rstsig_cpu1_core1_warm;
			break;
		case 2:
			clk = &g_ckgen_gating_cpu1_clk2;
			reset = &rstsig_cpu1_core2_warm;
			break;
		case 3:
			clk = &g_ckgen_gating_cpu1_clk3;
			reset = &rstsig_cpu1_core3_warm;
			break;
		default:
			early_printf("Don't Support this core %d\n", corenum);
			return;
	}

	scr_set(&g_scr_sec, hi_res_id, (uint32_t)(addr >> 30));
	scr_set(&g_scr_sec, lo_res_id, ((uint32_t)addr & BIT_MASK(30)) >> 2);

	/* Enable cpu core clk gating */
	clk_enable(clk);

	/* reset */
	reset_ctl_reset(reset);
}

void second_cpu_up(uint32_t cpu)
{
	switch (cpu) {
	case 0:
		while(1)
			arch_idle();
		break;
	case 1:
		while(1)
			arch_idle();
		break;
	case 2:
		while(1)
			arch_idle();
		break;
	case 3:
		while(1)
			arch_idle();
		break;
	default:
		board_halt(HALT_ACTION_HALT, HALT_REASON_SW_PANIC);
		break;
	}
	/* Nerver reach */
	board_halt(HALT_ACTION_HALT, HALT_REASON_SW_PANIC);

}
#endif
#ifdef CONFIG_FuncSafetyPkg
#ifdef CONFIG_BOARD_X9SP
extern void FuncSafetyPkg_Init(void);
extern void FuncSafetyPkg_Run_PowerOn_Func(void);
#endif
#endif
void board_startup(void)
{
    arch_early_init();
    board_early_init();

    call_constructors();
    osKernelInitialize();

    arch_init();
    board_init();

#ifdef CONFIG_CHIP_ID_DISPLAY
    fuse_part_load();
#endif
    ssdk_printf(SSDK_EMERG, "SSDK Board Startup Success!\n");
#ifdef __GNUC__
    ssdk_printf(SSDK_EMERG, "%s\n", build_msg);
#endif

    osThreadAttr_t attr = {
        .name = "main",
        .stack_size = CONFIG_MAIN_THREAD_STACK_SIZE,
        .priority = CONFIG_MAIN_THREAD_PRIORITY,
    };
#ifdef CONFIG_FuncSafetyPkg
#ifdef CONFIG_BOARD_X9SP
    FuncSafetyPkg_Init();
    FuncSafetyPkg_Run_PowerOn_Func();
#endif
#endif

    osThreadNew(_main_entry, NULL, &attr);

    if (osKernelGetState() == osKernelReady) {
        osKernelStart();
    }

    for (;;) {
    }
}
