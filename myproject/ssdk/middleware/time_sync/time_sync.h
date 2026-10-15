/**
 * @file time_sync.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
#ifndef __TIME_SYNC_H__
#define __TIME_SYNC_H__
/* =========================== include file session ======================== */
#include <compiler.h>
#include "driver/rtc.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */
/**
* @defgroup XXX XXX
* @brief  XXX service
* @{
*/
/* =========================== marco define session ======================== */


/* =========================== enumeration define session ================== */


/* =========================== structure define session ==================== */
typedef struct
{
	uint32_t sec;		/* second */
	uint32_t nsec;	/* nanosecond */

}timesync_ptp_timespec_t;

/* =========================== typedef define session ====================== */



/* =========================== function declare session ==================== */
void time_sync_service_init();

/**
 * @brief timesync get utc value.
 *
 * @param[in] rtc_time struct
 * @param[in] Microsecond value less than one second
 * @return 0 utc sync    -1:utc not sync
 */
int get_timesync_utc(struct rtc_time *tm,uint32_t *us);

/**
 * @brief timesync get rtc value.
 *
 * @param[in] rtc_time struct
 * @param[in] ms 
 */
 void  get_timesync_rtc(struct rtc_time *tm,uint32_t *ms);



/**
 * @brief timesync get GMAC1588(gPTP) value.
 *
 * @param[in] timesync_ptp_timespec_t struct

 * @return 0 ptp sync    -1:ptp not sync
 */
int  get_timesync_counter(timesync_ptp_timespec_t *ptp);


/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __INPUT_SERVICE_H__ */
/* end of header file time_sync.h */
