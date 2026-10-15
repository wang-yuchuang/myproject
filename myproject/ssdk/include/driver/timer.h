/**
 * @file timer.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: timer interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_DRV_TIMER_H
#define INCLUDE_DRV_TIMER_H

#include <types.h>
#include <compiler.h>
#include <spinlock.h>
#include <cmsis_os2.h>

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* timer status */
typedef enum {
    TMR_STA_NULL = 0,
    TMR_STA_STOP,
    TMR_STA_START,
    TMR_STA_ERR
} timer_status_e;

/* timer type */
typedef enum {
    TMR_TYPE_NORMAL = 0,
    TMR_TYPE_PERIOD = 1,
    TMR_TYPE_ERR
} timer_type_e;

/* lower half timer type */
typedef enum {
    TMR_DRV_ONESHOT = 0,
    TMR_DRV_CYCLE = 1,
    TMR_DRV_ERR
} timer_drv_type_e;

/* lower half timer type */
typedef enum {
    TMR_COUNT_MS = 0,
    TMR_COUNT_NS = 1
} timer_count_type;

typedef struct timer_ptp_timespec
{
	uint32_t sec;		/* second */
	uint32_t nsec;	/* nanosecond */

	uint32_t upd_sec;		/* second */
	uint32_t upd_nsec;	/* nanosecond */
}timer_ptp_timespec_t;


/* timer callback */
typedef void (*timer_callback)(void *arg);

typedef struct timer_dev timer_dev_t;

/* timer operation */
typedef struct timer_ops {
    void (*init)(timer_dev_t *dev);
    void (*deinit)(timer_dev_t *dev);
    void (*start)(timer_dev_t *dev, uint32_t time_out);
    void (*start_ns)(timer_dev_t *dev, uint32_t time_out);
    void (*stop)(timer_dev_t *dev);
    uint32_t (*current)(timer_dev_t *dev);
    /* option, only valid for kernel tick time */
    uint64_t (*global)(timer_dev_t *dev);
    void (*ptp)(timer_dev_t *dev,timer_ptp_timespec_t * ptp_time);
#ifdef CONFIG_USE_TICKLESS_IDLE
    void (*_frozen)(timer_dev_t *dev);
    void (*_continue)(timer_dev_t *dev);
    uint32_t (*_get_counter)(timer_dev_t *dev);
    void (*_set_overflow)(timer_dev_t *dev, uint32_t val);
    void (*_force_reload)(timer_dev_t *dev);
    bool (*_overflowed)(timer_dev_t *dev);
    void (*_clear_int)(timer_dev_t *dev);
    uint32_t (*_count_to_us)(timer_dev_t *dev, uint32_t cnt);
#endif /* CONFIG_USE_TICKLESS_IDLE */
} timer_ops_t;

/* timer driver */
typedef struct timer_dev {
    timer_status_e          status;
    timer_type_e            type;
    timer_drv_type_e        drv_type;
    timer_count_type        count_type;
    uint32_t                time_out;
    timer_callback          callback;
    spin_lock_t             lock;
    void                    *arg;
    const timer_ops_t       *ops;
    void                    *priv;
} timer_dev_t;

/**
 * @brief timer set callback.
 *
 * @param[in] dev timer dev
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void timer_init(timer_dev_t *dev);

/**
 * @brief timer set callback.
 *
 * @param[in] dev timer dev
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void timer_deinit(timer_dev_t *dev);

/**
 * @brief timer set callback.
 *
 * @param[in] dev timer dev
 * @param[in] callback callback function
 * @param[in] arg callback arg
 */
void timer_set_callback(timer_dev_t *dev, timer_callback callback, void *arg);

/**
 * @brief timer start.
 *
 * @param[in] dev timer dev
 * @param[in] type timer type
 * @param[in] time_out time out value(ms)
 */
void timer_start(timer_dev_t *dev, timer_type_e type, uint32_t time_out);

/**
 * @brief timer start ns.
 *
 * @param[in] dev timer dev
 * @param[in] type timer type
 * @param[in] time_out time out value(ns)
 */
void timer_start_ns(timer_dev_t *dev, timer_type_e type, uint32_t time_out);

/**
 * @brief timer stop.
 *
 * @param[in] dev timer dev
 */
void timer_stop(timer_dev_t *dev);

/**
 * @brief timer expiry, used by timer driver.
 *
 * @param[in] dev timer dev
 */
void timer_expiry(timer_dev_t *dev);

/**
 * @brief timer get current cnt value.
 *
 * @param[in] dev timer dev
 * @return the value of spec tmr_id(us)
 */
uint32_t timer_get_current_time(timer_dev_t *dev);

/**
 * @brief timer get global cnt value.
 *
 * @param[in] dev timer dev
 * @return the value of global tmr_id(us)
 */
uint64_t timer_get_global_time(timer_dev_t *dev);

/**
 * @brief timer get ptp time(counter).
 *
 * @param[in] dev timer dev
 *  @param[in] ptp_time struct
 */
void timer_get_gptp_time(timer_dev_t *dev,timer_ptp_timespec_t *ptp_time);

#ifdef CONFIG_USE_TICKLESS_IDLE

void timer_frozen(timer_dev_t *dev);

void timer_continue(timer_dev_t *dev);

uint32_t timer_get_counter_register(timer_dev_t *dev);

void timer_set_terminal_register(timer_dev_t *dev, uint32_t val);

void timer_force_reload(timer_dev_t *dev);

bool timer_overflow_int_occur(timer_dev_t *dev);

void timer_clear_overflow_int(timer_dev_t *dev);

uint32_t timer_count_to_us(timer_dev_t *dev, uint32_t val);

#endif /* CONFIG_USE_TICKLESS_IDLE */

__END_CDECLS

#endif

#endif
