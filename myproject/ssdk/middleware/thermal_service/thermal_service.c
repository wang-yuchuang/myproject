/*
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#include <arch.h>
#include <arch/register.h>
#include <thermal_service.h>
#include <stdbool.h>
#include <driver/kunlun/pvt/pvt.h>
#include <driver/kunlun/pvt/sdrv_pvt_drv.h>
#include <pvt_cfg.h>
#include <CLI.h>
#include <reg.h>
#include "driver/reset.h"
#include "CLI_type.h"
#include "rstgen_cfg.h"
#include "pmu_cfg.h"
#include "regs_base.h"

#define PVT_DEV_INS_NUM	2
#define PVT_SAFETY_CORE_TEMP 0
/* the AP of tem was set 125, SSDK should be set higher */
#define PVT_SAFETY_CRITAICAL_TEMP	130

typedef struct thermal_service_control {
	osThreadId_t *thread;
	int flag;
} thermal_service_ctl_t;

volatile static int cmd_temp;
volatile static int cmd_action;
volatile static int cmd_status = 0;
#define SYS_RESET	0
#define SYS_SHUTDOWN	1

#define PMU_EXT_CTRL	(APB_PMU_BASE + 0x00)
#define PMU_RTC_CTRL	(APB_PMU_BASE + 0x04)
#define PMU_RST_CTRL	(APB_PMU_BASE + 0x0c)
#define PMU_CTRL_0	(APB_PMU_BASE + 0x10)

static thermal_service_ctl_t thermal_service_ctl;

static thermal_service_ctl_t *thermal_service_ctl_get(void)
{
	return &thermal_service_ctl;
}

static pvt_dev_t* get_pvt_dev(int32_t devId)
{
    if (devId >= PVT_DEV_INS_NUM) {
        ssdk_printf(SSDK_EMERG, "parameter error for %s.\n", __func__);
        return NULL;
    }

    return g_pvt_dev_ptr[devId];
}

extern reset_sig_t rstsig_sw_self_rst;
static int system_action(int action)
{
	if (action) {
		ssdk_printf(SSDK_EMERG, "Shutdown, over SOC %d\n", PVT_SAFETY_CRITAICAL_TEMP);

		pmu_dev_t* dev_ptr = g_pmu_dev_ptr;

		pmu_change_state(dev_ptr, PMU_PWRDOWN);
		arch_irq_disable();

		/* disabled wakeup */
		RMWREG32(PMU_RTC_CTRL, 0, 1, 0);
		RMWREG32(PMU_RTC_CTRL, 1, 1, 0);
		RMWREG32(PMU_EXT_CTRL, 14, 1, 0);
		RMWREG32(PMU_EXT_CTRL, 21, 1, 0);

		/* shutdown pmu */
		RMWREG32(PMU_RST_CTRL, 9, 1, 1);  /* POR cell down */
		RMWREG32(PMU_CTRL_0, 8, 1, 0);    /* PMU power down */
		osDelay(2000);
		RMWREG32(PMU_CTRL_0, 8, 1, 1);

		while(1);
	} else {
		ssdk_printf(SSDK_EMERG, "Reset, over SOC %d\n", PVT_SAFETY_CRITAICAL_TEMP);

		reset_ctl_assert(&rstsig_sw_self_rst);
		reset_ctl_deassert(&rstsig_sw_self_rst);
	}

	return 0;
}

static void thermal_service_task(void *arg)
{
	thermal_service_ctl_t *ctl = (thermal_service_ctl_t *)arg;
	float out_t_data;

	if (!ctl) {
		ssdk_printf(SSDK_ALERT, "%s: ctl is NULL\n",  __func__);
		goto exit;
	}

	for (;;) {
		if (pvt_float_data_get(get_pvt_dev(PVT_SAFETY_CORE_TEMP), PVT_GET_FLOAT_CUR_TEMP, \
					-1, &out_t_data) == 0) {
			ssdk_printf(SSDK_INFO, "%s temperature value = %5.2f\n", __func__, \
					out_t_data);
		} else {
			ssdk_printf(SSDK_ALERT, "%s pvt device is invalid for get SOC temp\n", \
					__func__);
			goto exit;
		}

		if (cmd_status) {
			out_t_data = (float)cmd_temp;
			ctl->flag = cmd_action;
			ssdk_printf(SSDK_ALERT, "Simulated setting:SOC temperature: %5.2f, \
					action: %d\n", out_t_data, ctl->flag);
		}

		if ( ((int)out_t_data) > PVT_SAFETY_CRITAICAL_TEMP) {
			ssdk_printf(SSDK_ALERT, "SOC temperature value: %5.2f, \
					trigger reboot or shutdown\n", out_t_data);
			system_action(ctl->flag);
		}

		osDelay(2000);
	}

exit:
    osThreadExit();
}

void thermal_service_init(int type)
{
	const char *thermal_service_name = "thermal_service";
	thermal_service_ctl_t *ctl = thermal_service_ctl_get();
	osThreadAttr_t thermal_service_task_attr = {0};

	thermal_service_task_attr.name       = thermal_service_name;
	thermal_service_task_attr.stack_size = THERMAL_SERVICE_STACK_SIZE;
	thermal_service_task_attr.priority   = osPriorityBelowNormal;

	ctl->flag = type;
	ctl->thread = osThreadNew(thermal_service_task, (void *)ctl, &thermal_service_task_attr);
	if (ctl->thread == NULL) {
		ssdk_printf(SSDK_ALERT, "%s: create thermal service thread failed.\n", __func__);
		return;
	}

	ssdk_printf(SSDK_ALERT, "%s succeeded\n", __func__);
	return;
}

void thermal_subCmd_help_msg()
{
	printf("thermal_test [system_action] [temp] \n");
	printf("system_action: 0 or 1 \n");
	printf("0: system reset \n");
	printf("1: system shutdown \n");
	printf("for example:\n");
	printf("thermal_test 1 70\n");
	printf("When the SOC of temperature was over 70, system shutdown.\n");
	printf("thermal_test 0 80\n");
	printf("When the SOC of temperature was over 80, system reboot.\n");
}

int32_t thermal_test_main(int32_t argc, char* argv[])
{
	if (argc != 2) {
		thermal_subCmd_help_msg();
		return CLI_FALSE;
	}

	cmd_action = strtoul(argv[0], NULL, 0);
	if ( cmd_action == SYS_RESET ) {
		ssdk_printf(SSDK_ALERT, "%s: System reset: %d from user command\n", \
				__func__, cmd_action);
	} else if ( cmd_action == SYS_SHUTDOWN ) {
		ssdk_printf(SSDK_ALERT, "%s: System shutdown: %d from user command\n", \
				__func__, cmd_action);
	}else {
		ssdk_printf(SSDK_ALERT, "%s: Can't prase user command: %d\n", __func__, \
				cmd_action);
		thermal_subCmd_help_msg();
		return CLI_FALSE;
	}

	cmd_temp = strtoul(argv[1], NULL, 0);
	ssdk_printf(SSDK_ALERT, "Simulated temp: %d from user command\n", cmd_temp);

	cmd_status = 1;

	return 0;
}

CLI_CMD("thermal_test", "simulate Soc temp for test", thermal_test_main);
