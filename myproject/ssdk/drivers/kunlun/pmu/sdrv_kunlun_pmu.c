/*
 * Copyright (c) 2018 Semidrive Semiconductor.
 * All rights reserved.
 */

#include <sys/types.h>
#include <reg.h>
#include <assert.h>
#include "sdrv_kunlun_pmu.h"

#define KUNLUN_PMU_BASE 0xF1850000

// int32_t sdrv_pmu_lld_init(addr_t base)
// {
// 	if (base->name == NULL || base->base_paddr == NULL
// 	        || base == NULL) {
// 		sdrv_pmu_lld_err("init failure\n");
// 		return -1;
// 	}

// 	base->init_flag = 1;
// 	sdrv_pmu_lld_dbg("%s init okay\n", base->name);
// 	return 0;
// }

// int32_t sdrv_pmu_lld_exit(addr_t base)
// {
// 	if (base->init_flag != 1) {
// 		sdrv_pmu_lld_err("exit failure\n");
// 		return -1;
// 	}

// 	base->init_flag = 0;
// 	sdrv_pmu_lld_dbg("%s exit okay\n", base->name);
// 	return 0;
// }

static int32_t sdrv_pmu_lld_check(addr_t base)
{
	if (base != KUNLUN_PMU_BASE)
		return 1;
	return 0;
}

static inline void sdrv_pmu_lld_modify_reg32(volatile addr_t addr, int startbit, int width, uint32_t val)
{
	RMWREG32(addr, startbit, width, val);
}

/**************************************************
 * sdrv_pmu_lld_get_status
 *
 * status value 0x01 - INIT
 *              0x02 - PWRUP
 *              0x04 - ON
 *              0x08 - PWRDOWN
 *              0x10 - OFF
 *              < 0  - ERROR
 ************************************************/
int32_t sdrv_pmu_lld_get_status(addr_t base, int *status)
{
	uint32_t v = 0;

	if (sdrv_pmu_lld_check(base) || status == NULL) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	v = readl(base + PMU_RST_CTRL);
	v = (v & (0x1f << 24)) >> 24;
	if (v != 0x01 && v != 0x02 && v != 0x04 && v != 0x08 && v != 0x10) {
		sdrv_pmu_lld_err("get status fail v = %02x\n", v);
		return -1;
	}
	*status = v;
	return 0;
}

int32_t sdrv_pmu_lld_get_event_source(addr_t base, uint32_t *event)
{
	uint32_t v;

	if (sdrv_pmu_lld_check(base) || event == NULL) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	v = readl(base + PMU_INTR);
	*event = v & 0xff;
	return 0;
}

int32_t sdrv_pmu_lld_clean_single_event_source(addr_t base, int event_id)
{
	if (sdrv_pmu_lld_check(base) || event_id < 0 || event_id > 7) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	writel(1 << event_id, base + PMU_INTR);
	return 0;
}

int32_t sdrv_pmu_lld_clean_all_event_source(addr_t base)
{
	if (sdrv_pmu_lld_check(base)) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	writel(0xff, base + PMU_INTR);
	return 0;
}

int32_t sdrv_pmu_lld_powerdown(addr_t base, int32_t sta)
{
	if (sdrv_pmu_lld_check(base)) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}
	sdrv_pmu_lld_modify_reg32(base + PMU_CTRL_0, 8, 1, sta);
	return 0;
}

/******************************************************************************
 * sdrv_pmu_lld_set_powerdown_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerdown_delay(addr_t base, int id, int delay)
{
	if (sdrv_pmu_lld_check(base) || delay < 0x0 || delay > 0xf) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_CTRL_0, 4, 4, delay);
	return 0;
}

/******************************************************************************
 * sdrv_pmu_lld_set_powerup_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerup_delay(addr_t base, int id, int delay)
{
	if (sdrv_pmu_lld_check(base) || delay < 0x0 || delay > 0xf) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_CTRL_0, 0, 4, delay);
	return 0;
}

/******************************************************************************
 * sdrv_pmu_lld_set_powerctrl_powerdown_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerctrl_powerdown_delay(addr_t base, int ctrl_id, int delay)
{
	int reg_offset, reg_start_bit;

	if (sdrv_pmu_lld_check(base) || delay < 0x0 || delay > 0xf || ctrl_id < 0 || ctrl_id > 3) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 4;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 15;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 4;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 15;
			break;
	}

	sdrv_pmu_lld_modify_reg32(base + reg_offset, reg_start_bit, 4, delay);
	return 0;
}

/******************************************************************************
 * sdrv_pmu_lld_set_powerctrl_powerup_delay
 *
 * input delay:  0x0 - 0us
 *               0x1 - 30us
 *               0x2 - 60us
 *               0x3 - 120us
 *               0x4 - 0.25ms
 *               0x5 - 0.5ms
 *               0x6 - 0.75ms
 *               0x7 - 1ms
 *               0x8 - 2ms
 *               0x9 - 3ms
 *               0xa - 4ms
 *               0xb - 5ms
 *               0xc - 6ms
 *               0xd - 7ms
 *               0xe - 8ms
 *               0xf - 16ms
 *
 *****************************************************************************/
int32_t sdrv_pmu_lld_set_powerctrl_powerup_delay(addr_t base, int ctrl_id, int delay)
{
	int reg_offset, reg_start_bit;

	if (sdrv_pmu_lld_check(base) || delay < 0x0 || delay > 0xf || ctrl_id < 0 || ctrl_id > 3) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 0;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 11;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 0;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 11;
			break;
	}

	sdrv_pmu_lld_modify_reg32(base + reg_offset, reg_start_bit, 4, delay);
	return 0;
}

int32_t sdrv_pmu_lld_set_powerctrl_io_mode(addr_t base, int ctrl_id, int mode)
{
	int reg_offset, reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (mode != PWR_CTRL_INPUT && mode != PWR_CTRL_OUTPUT )
	        || ctrl_id < 0 || ctrl_id > 3) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 8;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 19;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 8;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 19;
			break;
	}

	sdrv_pmu_lld_modify_reg32(base + reg_offset, reg_start_bit, 1, mode);
	return 0;
}

int32_t sdrv_pmu_lld_set_powerctrl_out_mode(addr_t base, int ctrl_id, int mode)
{
	int reg_offset, reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (mode != PWR_CTRL_OUTPUT_MANUAL && mode != PWR_CTRL_OUTPUT_AUTO )
	        || ctrl_id < 0 || ctrl_id > 3) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 9;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 20;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 9;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 20;
			break;
	}

	sdrv_pmu_lld_modify_reg32(base + reg_offset, reg_start_bit, 1, mode);
	return 0;
}

int32_t sdrv_pmu_lld_set_powerctrl_out_ctrl(addr_t base, int ctrl_id, int out)
{
	int reg_offset, reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (out != PWR_CTRL_OUTPUT_LOW && out != PWR_CTRL_OUTPUT_HIGH)
	        || ctrl_id < 0 || ctrl_id > 3) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 10;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 21;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 10;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 21;
			break;
	}

	sdrv_pmu_lld_modify_reg32(base + reg_offset, reg_start_bit, 1, out);
	return 0;
}

int32_t sdrv_pmu_lld_get_powerctrl_input_status(addr_t base, int ctrl_id, int *status)
{
	int reg_offset, reg_start_bit;
	uint32_t v;

	if (sdrv_pmu_lld_check(base) || status == NULL || ctrl_id < 0 || ctrl_id > 3) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	switch (ctrl_id) {
		case PWR_CTRL_0:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 30;
			break;
		case PWR_CTRL_1:
			reg_offset = PMU_CTRL_1;
			reg_start_bit = 31;
			break;
		case PWR_CTRL_2:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 30;
			break;
		case PWR_CTRL_3:
			reg_offset = PMU_CTRL_2;
			reg_start_bit = 31;
			break;
	}

	v = readl(base + reg_offset);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t sdrv_pmu_lld_set_internal_powerdown_enable(addr_t base, int id, int enable)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 0;
	else
		reg_start_bit = 2;

	sdrv_pmu_lld_modify_reg32(base + PMU_CTRL_3, reg_start_bit, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_internal_powerdown_polarity(addr_t base, int id, int polarity)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (polarity != HIGH_LEVEL && polarity != LOW_LEVEL)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 1;
	else
		reg_start_bit = 3;

	sdrv_pmu_lld_modify_reg32(base + PMU_CTRL_3, reg_start_bit, 1, polarity);
	return 0;
}

int32_t sdrv_pmu_lld_get_internal_powerdown_status(addr_t base, int id, int *status)
{
	int reg_start_bit;
	uint32_t v;

	if (sdrv_pmu_lld_check(base) || status == NULL || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 8;
	else
		reg_start_bit = 9;

	v = readl(base + PMU_CTRL_3);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t sdrv_pmu_lld_set_external_reset_enable(addr_t base, int id, int enable)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 0;
	else
		reg_start_bit = 7;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_reset_polarity(addr_t base, int id, int polarity)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (polarity != HIGH_LEVEL && polarity != LOW_LEVEL)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 1;
	else
		reg_start_bit = 8;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 1, polarity);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_reset_debounce_enable(addr_t base, int id, int enable)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 2;
	else
		reg_start_bit = 9;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_reset_debounce_delay(addr_t base, int id, int delay)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || delay < 0 || delay > 0xf || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 3;
	else
		reg_start_bit = 10;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 4, delay);
	return 0;
}

int32_t sdrv_pmu_lld_get_external_reset_status(addr_t base, int id, int *status)
{
	int reg_start_bit;
	uint32_t v;

	if (sdrv_pmu_lld_check(base) || status == NULL || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 28;
	else
		reg_start_bit = 29;

	v = readl(base + PMU_EXT_CRTL);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t sdrv_pmu_lld_set_internal_wakeup_enable(addr_t base, int id, int enable)
{
	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_RTC_CTRL, id, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_wakeup_enable(addr_t base, int id, int enable)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 14;
	else
		reg_start_bit = 21;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_wakeup_polarity(addr_t base, int id, int polarity)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (polarity != HIGH_LEVEL && polarity != LOW_LEVEL)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 15;
	else
		reg_start_bit = 22;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 1, polarity);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_wakeup_debounce_enable(addr_t base, int id, int enable)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)
	        || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 16;
	else
		reg_start_bit = 23;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_external_wakeup_debounce_delay(addr_t base, int id, int delay)
{
	int reg_start_bit;

	if (sdrv_pmu_lld_check(base) || delay < 0 || delay > 0xf || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 17;
	else
		reg_start_bit = 24;

	sdrv_pmu_lld_modify_reg32(base + PMU_EXT_CRTL, reg_start_bit, 4, delay);
	return 0;
}

int32_t sdrv_pmu_lld_get_external_wakeup_status(addr_t base, int id, int *status)
{
	int reg_start_bit;
	uint32_t v;

	if (sdrv_pmu_lld_check(base) || status == NULL || id < 0 || id > 1) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	if (id == 0)
		reg_start_bit = 30;
	else
		reg_start_bit = 31;

	v = readl(base + PMU_EXT_CRTL);
	*status = ((v & (1 << reg_start_bit)) >> reg_start_bit) & 0x1;
	return 0;
}

int32_t sdrv_pmu_lld_set_glitch_filter_delay(addr_t base, int delay)
{
	if (sdrv_pmu_lld_check(base) || delay < 0 || delay > 0x7) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_GF_CTRL, 11, 3, delay);
	return 0;
}

int32_t sdrv_pmu_lld_set_glitch_filter_enable(addr_t base, int multiEnable)
{
	if (sdrv_pmu_lld_check(base)) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_GF_CTRL, 0, 10, multiEnable);
	return 0;
}

int32_t sdrv_pmu_lld_set_por_powerdown_delay(addr_t base, int delay)
{
	if (sdrv_pmu_lld_check(base) || delay < 0 || delay > 0x7) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_RST_CTRL, 11, 3, delay);
	return 0;
}

int32_t sdrv_pmu_lld_set_por_auto_powerdown(addr_t base, int enable)
{
	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_RST_CTRL, 10, 1, enable);
	return 0;
}

int32_t sdrv_pmu_lld_set_por_force_powerdown(addr_t base, int enable)
{
	if (sdrv_pmu_lld_check(base) || (enable != ENABLE && enable != DISABLE)) {
		sdrv_pmu_lld_err("check failure\n");
		return -1;
	}

	sdrv_pmu_lld_modify_reg32(base + PMU_RST_CTRL, 9, 1, enable);
	return 0;
}
