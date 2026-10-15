/**
 * @file time_sync.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <types.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <cmsis_os2.h>
#include <debug.h>
#include <CLI.h>
#include <mbox_cfg.h>
#include "timer_cfg.h"
#include "rtc_cfg.h"
#include "driver/rtc.h"
#include "driver/timer.h"

#include "time_sync.h"


/* =========================== marco define session ======================== */

/* =========================== enumeration define session ================== */


/* =========================== structure define session ==================== */


/* =========================== function declare session ==================== */
static int time_sync_recive_mbox(void *arg, uint8_t *mssg, uint32_t len);

/* =========================== global value define session ================= */

/* =========================== static value define session ================= */
static struct mbox_chan *chan;
static uint64_t time_real = 0;
static uint64_t time_counter = 0;
static timer_ptp_timespec_t  time_ptp = {0};
#if CONFIG_SAFETY
static uint64_t rtc_counter = 0;
#endif


static int time_sync_recive_mbox(void *arg, uint8_t *mssg, uint32_t len)
{
	uint32_t rtc_epoch;  /* s */
	uint64_t realtime;  /* us */
	uint64_t syscnt_AP,syscnt_R;
	uint32_t magic;
	int Pos = 0;

	memcpy(&magic,mssg,sizeof(magic));
	Pos += sizeof(magic);
	if(magic == 0x01)
	{	/* RTC */
		memcpy(&rtc_epoch,mssg+Pos,sizeof(rtc_epoch));
		Pos += sizeof(rtc_epoch);
		memcpy(&syscnt_AP,mssg+Pos,sizeof(syscnt_AP));
		syscnt_R = timer_get_global_time(g_syscnt_timer);
		if(syscnt_R >=syscnt_AP)
		{
			struct rtc_time tm = {0};

			rtc_epoch += (syscnt_R - syscnt_AP)/1000000;
			epoch_to_rtc(&tm,rtc_epoch);
			rtc_set_time(&g_rtc1_dev,&tm);
			#if CONFIG_SAFETY
			if(rtc_counter != 0)
			{
				struct rtc_time tm_rtc1 = {0};
				rtc_get_time(&g_rtc1_dev,&tm_rtc1);
				uint32_t rtc_epoch_rtc1,rtc_epoch_rtc2;
				rtc_epoch_rtc2 = rtc_epoch;
				rtc_epoch_rtc1 = (uint32_t)rtc_to_epoch(&tm_rtc1);
				printf(" time_sync  rtc pathdelay:<%lld> us  rtc1_epoch:<%d> s  rtc2_epoch:<%d> s\n ",
					syscnt_R - syscnt_AP,rtc_epoch_rtc1,rtc_epoch_rtc2);
			}
			rtc_counter = syscnt_AP;
			#endif
		}
		else
		{
			ssdk_printf(SSDK_ERR,"time_sync rtc  time sync  error !\n");
		}

	}
	else if(magic == 0x02) {	/* realtime  us */
		memcpy(&realtime,mssg+Pos,sizeof(realtime));
		Pos += sizeof(realtime);
		memcpy(&syscnt_AP,mssg+Pos,sizeof(syscnt_AP));
		syscnt_R = timer_get_global_time(g_syscnt_timer);
		if(syscnt_R >=syscnt_AP)
		{
			realtime += (syscnt_R - syscnt_AP);
			time_real = realtime;
			time_counter = syscnt_R;
			/* set realtime to RTC */
			realtime = realtime / 1000; /* ms */
			rtc_set_time_ms(&g_rtc1_dev,realtime);
		}
		else
		{
			ssdk_printf(SSDK_ERR,"time_sync realtime  time sync  error !\n");
		}

	}
	else
	{
		ssdk_printf(SSDK_ERR,"time_sync  wrong magic! %x\n",magic);
	}

	return 0;
}

static void time_sync_wait_ptp_enable(void *arg)
{

	timer_ptp_timespec_t  ptp = {0};
	while((ptp.upd_sec == 0)  && (ptp.upd_nsec == 0))
	{
		timer_get_gptp_time(g_kernel_timer,&ptp);
		osDelay(50);
	}

	time_ptp.upd_sec = ptp.upd_sec;
	time_ptp.upd_nsec = ptp.upd_nsec;
	printf(" time_sync  ptp enable and sync at %d s ! \n ",time_ptp.upd_sec);
	osThreadExit();
}

void time_sync_service_init()
{
    struct sdrv_mbox_chan_req chan_req;
    chan_req.rproc = 3;  /* AP1 */
    chan_req.src_addr = 0xbd;
    chan_req.dest_addr = 0xbd;
    chan = mbox_request_channel(g_mbox_dev, &chan_req);
    mbox_set_callback(chan, time_sync_recive_mbox, NULL);

    osThreadNew(time_sync_wait_ptp_enable, NULL, NULL);


}


int  get_timesync_utc(struct rtc_time *tm,uint32_t *us)
{
	uint64_t syscnt_R;
	uint64_t timer_us;

	if(time_counter ==0){
		return -1;
		}
	if(!tm && !us){
		return -1;
		}
	syscnt_R = timer_get_global_time(g_syscnt_timer);
	timer_us =  syscnt_R - time_counter + time_real;
	epoch_to_rtc(tm,timer_us/1000000);
	*us = timer_us % 1000000;
	return 0;
}


 void  get_timesync_rtc(struct rtc_time *tm,uint32_t *ms) {
 	      uint64_t  us = 0;

		if(tm != NULL && ms != NULL)
		{
			rtc_get_time(&g_rtc1_dev,tm);
			rtc_get_time_us(&g_rtc1_dev,&us);
			*ms = (us /1000) % 1000;
		}
}



int  get_timesync_counter(timesync_ptp_timespec_t *ptp)
{
#if 0
	timer_ptp_timespec_t  timer_ptp = {0};

	if(time_ptp.upd_sec ==0){
		return -1;
		}
	if(!ptp){
		return -1;
		}

	timer_get_gptp_time(g_kernel_timer,&timer_ptp);


	ptp->nsec = timer_ptp.nsec;
	ptp->sec = timer_ptp.sec;

#endif

	timer_ptp_timespec_t  timer_ptp = {0};
	timer_get_gptp_time(g_kernel_timer,&timer_ptp);
	if((timer_ptp.upd_sec == 0)  &&  (timer_ptp.upd_nsec == 0))
	{
		return -1;
	}
	if(!ptp){
		return -1;
		}
	ptp->nsec = timer_ptp.nsec;
	ptp->sec = timer_ptp.sec;
	return 0;
}


static int time_sync_service_test(int argc, char* argv[])
{

	struct rtc_time tm = {0};
	timesync_ptp_timespec_t  ptp = {0};
	int repeat = 10;
	uint32_t rtc_ms = 0;
	while(repeat-- > 0)
	{
	#if 0
		ret = get_timesync_utc(&tm,&us);
		if(ret == 0)
		{
			printf("time_sync  utc time  %04d-%02d-%02d   %02d:%02d:%02d   <%d>us!\n",tm.tm_year+1900,tm.tm_mon+1
				,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,us);
		}
		else
		{
			printf(" time_sync  utc time  has not been sync ! \n ");
		}
	#endif
		#if CONFIG_SAFETY
		if(rtc_counter == 0)
		{
			printf("  time_sync rtc  time  has not been sync ! \n");
		}
		else
		{
			get_timesync_rtc(&tm,&rtc_ms);
			printf("time_sync  rtc time  %04d-%02d-%02d   %02d:%02d:%02d:%03d    \n",tm.tm_year+1900,tm.tm_mon+1
				,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,rtc_ms);
		}
		#else
			get_timesync_rtc(&tm,&rtc_ms);
			printf("time_sync  rtc time  %04d-%02d-%02d   %02d:%02d:%02d:%03d    \n",tm.tm_year+1900,tm.tm_mon+1
				,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,rtc_ms);
		#endif


		ptp.sec = ptp.nsec = 0;
		get_timesync_counter(&ptp);
		if(ptp.sec == 0 && ptp.nsec == 0)
		{
			printf(" time_sync  gptp time  has not been sync ! \n ");
		}
		else
		{
			epoch_to_rtc(&tm,ptp.sec);
			printf("time_sync  ptp time  %04d-%02d-%02d   %02d:%02d:%02d:%03d:%03d \n",tm.tm_year+1900,tm.tm_mon+1
				,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec,ptp.nsec/1000000,ptp.nsec/1000%1000);

		}
	osDelay(2000);
	}

    return CLI_FALSE;

}


#ifdef CONFIG_CLI
CLI_CMD("time_sync", "display realtime and gptp time", time_sync_service_test);
#endif /* end  CONFIG_CLI*/

/* end of file time_sync.c */
