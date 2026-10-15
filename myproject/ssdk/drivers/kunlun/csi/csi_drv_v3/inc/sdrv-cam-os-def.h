/*
 * sdrv-cam-os-def.h
 *
 * Semidrive platform camera driver and hal adaptive to system header file
 *
 * Copyright (C) 2021, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */
#ifndef __SDRV_CAM_OS_DEF_H__
#define __SDRV_CAM_OS_DEF_H__

#include <arch.h>
#include <arch/atomic.h>
#include <arch/cache.h>
#include <arch/irq.h>
#include <arch/spinlock.h>
#include <assert.h>
#include <board.h>
#include <cmsis_os2.h>
#include <debug.h>
#include <errno.h>
#include <lib/list.h>
#include <reg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "csi_drv.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/irq.h"
#include "i2c_cfg.h"
#include "irq_num.h"
#include "regs_base.h"

#define CAM_OS_EMERG       SSDK_CRIT
#define CAM_OS_ERR         SSDK_ERR
#define CAM_OS_WARNING     SSDK_WARNING
#define CAM_OS_INFO        SSDK_INFO
#define CAM_OS_DEBUG       SSDK_DEBUG

#define csi_err_ratelimited(x...) ssdk_printf(cam_err_loglevel, x)
#define csi_err(x...) ssdk_printf(cam_err_loglevel, x)
#define csi_warn(x...) ssdk_printf(cam_warn_loglevel, x)
#define csi_info(x...) ssdk_printf(cam_info_loglevel, x)
#define csi_debug(x...) ssdk_printf(cam_debug_loglevel, x)

#define cam_err(x...) ssdk_printf(cam_err_loglevel, x)
#define cam_info(x...) ssdk_printf(cam_info_loglevel, x)
#define cam_debug(x...) ssdk_printf(cam_debug_loglevel, x)

#define sensor_err(x...) ssdk_printf(cam_err_loglevel, x)
#define sensor_info(x...) ssdk_printf(cam_info_loglevel, x)
#define sensor_debug(x...) ssdk_printf(cam_debug_loglevel, x)

#define serdes_crit(x...) ssdk_printf(SSDK_CRIT, x)
#define serdes_err(x...) ssdk_printf(cam_err_loglevel, x)
#define serdes_info(x...) ssdk_printf(cam_info_loglevel, x)
#define serdes_debug(x...) ssdk_printf(cam_debug_loglevel, x)

#define HAL_LOGE(x...) ssdk_printf(cam_err_loglevel, x)
#define HAL_LOGI(x...) ssdk_printf(cam_info_loglevel, x)
#define HAL_LOGD(x...) ssdk_printf(cam_debug_loglevel, x)

#ifndef u8
typedef uint8_t u8;
#endif
#ifndef u16
typedef uint16_t u16;
#endif
#ifndef u32
typedef uint32_t u32;
#endif

#ifndef reg_addr_t
typedef paddr_t reg_addr_t;
#endif

#ifndef dma_addr_t
typedef paddr_t dma_addr_t;
#endif

#ifndef atomic_t
typedef volatile int atomic_t;
#endif

#ifndef dprintf
#define dprintf ssdk_printf
#endif

#define CSI_COMPLETE_EVENT_FLAG 1u
typedef struct {
	osEventFlagsId_t event;
} completion_t;

typedef spin_lock_t spinlock_t;

static inline uint32_t reg_read(reg_addr_t base, uint32_t addr)
{
	return readl(base + addr);
}

static inline void reg_write(reg_addr_t base, uint32_t addr, uint32_t val)
{
	writel(val, base + addr);
}

#define usleep_range(min, max) udelay(min >> 1)
//#define mdelay(ms) usleep_range(ms * 1000, ms * 1000)

static inline int atomic_add(atomic_t *ptr, int val)
{
	return arch_atomic_add((int *)ptr, val);
}

inline static void completion_init(completion_t *com)
{
	com->event = osEventFlagsNew(NULL);
}

inline static void completion_destroy(completion_t *com)
{
	osEventFlagsDelete(com->event);
}

inline static int wait_completion_timeout(completion_t *com, uint32_t timeout)
{
	uint32_t ret;

	ret = osEventFlagsWait(com->event, CSI_COMPLETE_EVENT_FLAG,
			       osFlagsWaitAny, timeout);
	return (int)ret;
}

inline static void completion_done(completion_t *com)
{
	osEventFlagsSet(com->event, CSI_COMPLETE_EVENT_FLAG);
}

#define register_int_handler(irq, handler, data) \
		irq_attach(irq, handler, data)
#define unmask_interrupt(irq) irq_enable(irq)

#define spin_lock_irqsave(spinlock, flags)                                     \
	do {                                                                   \
		irq_state_t state = arch_irq_save();                           \
		flags = state;                                                 \
		arch_spin_lock(spinlock);                                      \
	} while (0)

#define spin_unlock_irqrestore(spinlock, flags)                                \
	do {                                                                   \
		irq_state_t state = (irq_state_t)flags;                        \
		arch_spin_unlock(spinlock);                                    \
		arch_irq_restore(state);                                       \
	} while (0)

#endif /* __SDRV_CAM_OS_DEF_H__ */
