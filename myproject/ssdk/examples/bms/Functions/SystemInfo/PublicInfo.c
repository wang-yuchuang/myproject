/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : PublicInfo.c
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     :
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicPara.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "ParaWrite.h"
#include "ParaRead.h"
#include <cmsis_os2.h>
#include <stdio.h>
#include <debug.h>

u32 gSysSecondTime = 0;                 /*系统运行计时(1s)*/
t_SysClock gSysClock = {0};             /*系统时钟(北京时间)*/

extern u16 *gVerTime_0;             /*校时信息*/
extern  u16 *gGStatistTime_59;     /*时间统计信息*/

static void UpdateMonAndYearAccdDay(t_SysClock *ptm);
/*=================================================================================================
** @Name      : void SysPublicInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 系统系统通用信息初始化
** @The notes :
**===============================================================================================*/
void SysPublicInfoInit(void)
{
    u8 i = 0;
	t_SysClock rtcClock = {0};
    u16 clockTime[GINDEX0_LEN + 1] = {0};
    /*读系统时间记录(标志/年/月日/时分/毫秒)*/
	if(TRUE == ParaReadStoreSysClock(clockTime, GINDEX0_LEN + 1))
	{
		for(i = 0; i < GINDEX0_LEN; i++)
		{
            if(gVerTime_0 != NULL)
            {
                gVerTime_0[i] = clockTime[1 + i];
            }
		}
	}

    /*将校时索引信息更新到系统时钟*/
	SysClockUpdateAPI();

    //读RTC信息，此处屏蔽，由A核使用

    /*读系统运行时间,单位:1h*/
	if(TRUE == ParaReadStoreSysRunTime(clockTime, 2))
	{
        if(gGStatistTime_59 != NULL)
        {
            /*记录系统运行时间1H*/
            gGStatistTime_59[eTime59_1SysRun] = clockTime[1];
            gGStatistTime_59[eTime59_2SysRun] = clockTime[1];
        }
		/*系统运行时间由1h记录转化为1s*/
		gSysSecondTime = (u32)clockTime[1] * 3600;
	}
}
/*=================================================================================================
** @Name      : void SysClockCountTask(void)
** @Input     : void
** @Output    : void
** @Function  : 系统时钟(北京时间)计数任务,ms计时
** @The notes : 1ms定时中断调用
**===============================================================================================*/
void SysClockCountTask(void)
{
    static u16 sSysMilSecTime = 0;
    /*上电1ms计时*/
	sSysMilSecTime++;
	if(sSysMilSecTime >= 1000)
	{
		gSysSecondTime++;
		sSysMilSecTime %= 1000;
	}
    	/*时钟1ms计时*/
	gSysClock.msecond++;
	if(gSysClock.msecond >= 1000)
	{
		gSysClock.msecond = 0;
		gSysClock.second++;
		if(gSysClock.second >= 60)
		{
			gSysClock.second = 0;
			gSysClock.minute++;
			if(gSysClock.minute >= 60)
			{
				gSysClock.minute = 0;
				gSysClock.hour++;
				if(gSysClock.hour >= 24)
				{
					gSysClock.hour = 0;
					gSysClock.day++;
					UpdateMonAndYearAccdDay(&gSysClock);
				}
			}
		}
	}
}
/*=================================================================================================
** @Name      : void SysClockRecordTask(void)
** @Input     : void
** @Output    : void
** @Function  : 系统时钟时间记录任务(包括记录运行时间)
** @The notes : 2s周期调用
**===============================================================================================*/
void SysClockRecordTask(void)
{
	u8 i = 0;
	u16 time = 0;
	static u8 RcdFlag = 0;
	static u32 sHisTime = 0;
	static u32 sHisWState = 0;
	static u32 sHisCState = 0;
	static u16 sHisClock[GINDEX0_LEN] = {0};
	if(gVerTime_0 == NULL)
	{
		ssdk_printf(SSDK_WARNING, "%s: gVerTime_0 fail\r\n", __func__);
		return;
	}
	if(gGStatistTime_59 == NULL)
	{
		ssdk_printf(SSDK_WARNING, "%s: gGStatistTime_59 fail\r\n", __func__);
		return;
	}
	/*状态变化且记录间隔10s*/
	if(((GetGWorkStateAPI(0) != sHisWState)
		|| (GetGChgDhgStateAPI(0) != sHisCState))
		&& (ABS(GetSysSecondTimeAPI(), sHisTime) >= 10))
	{
		/*更新校时字段*/
		SysClockDisplayAPI();

		/*记录校时字段(不记录ms)*/
		for(i = 0; i < (GINDEX0_LEN - 1); i++)
		{
			/*校时字段有变化才重写*/
			if(gVerTime_0[i] != sHisClock[i])
			{
				ParaWriteStoreSysClock(1 + i, &gVerTime_0[i], 1);
				sHisClock[i] = gVerTime_0[i];
			}
		}

		sHisTime = GetSysSecondTimeAPI();
		sHisWState = GetGWorkStateAPI(0);
		sHisCState = GetGChgDhgStateAPI(0);

		/*运行时间1h计时超过半小时需要四舍五入记录*/
		if(((GetSysSecondTimeAPI() % 3600) >= 1800) && (0 == RcdFlag))
		{
			/*记录系统运行时间四舍五入(确保重新上电计时丢失小于半小时)*/
			time = gGStatistTime_59[eTime59_1SysRun] + 1;
			ParaWriteStoreSysRunTime(1, &time, 1);
			RcdFlag = 1;
		}
	}
	/*状态未变化*/
	else
	{
		/*间隔10min或校时改变年月日则记录系统时钟*/
		if((ABS(GetSysSecondTimeAPI(), sHisTime) >= 600)
			|| (gVerTime_0[0] != sHisClock[0])
			|| (gVerTime_0[1] != sHisClock[1]))
		{
			/*更新校时字段*/
			SysClockDisplayAPI();

			/*记录校时字段*/
			for(i = 0; i < GINDEX0_LEN; i++)
			{
				/*校时字段有变化才重写*/
				if(gVerTime_0[i] != sHisClock[i])
				{
					ParaWriteStoreSysClock(1 + i, &gVerTime_0[i], 1);
					sHisClock[i] = gVerTime_0[i];
				}
			}

			sHisTime = GetSysSecondTimeAPI();
		}
	}

	/*运行时间1h计时变化则记录*/
	if(gGStatistTime_59[eTime59_1SysRun] != (GetSysSecondTimeAPI() / 3600))
	{
		/*单位由1s转化为1h显示*/
		gGStatistTime_59[eTime59_1SysRun] = GetSysSecondTimeAPI() / 3600;
		gGStatistTime_59[eTime59_2SysRun] = GetSysSecondTimeAPI() / 3600;

		/*记录系统运行时间*/
		ParaWriteStoreSysRunTime(1, &gGStatistTime_59[eTime59_1SysRun], 1);
		RcdFlag = 0;
	}
}
/*=================================================================================================
** @Name      : void SysClockExtVerifyAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 系统时钟外部校时处理
** @The notes : 接收校时报文时调用
**===============================================================================================*/
void SysClockExtVerifyAPI(void)
{
	if(gVerTime_0 == NULL)
	{
		ssdk_printf(SSDK_WARNING, "%s: gVerTime_0 null\r\n", __func__);
		return;
	}
	//校验下发的时间有效
	if((gVerTime_0[eICCTim0_Y] >= 2000) && (gVerTime_0[eICCTim0_Y] <= 2099)
		&& (gVerTime_0[eICCTim0_MD] >= 0x0101) && (gVerTime_0[eICCTim0_MD] <= 0x1f1f)
		&& (gVerTime_0[eICCTim0_HM] <= 0x3c3c) && (gVerTime_0[eICCTim0_MS] <= 0xea5f))
	{
		/*从外部校时信息更新系统时钟*/
		SysClockUpdateAPI();

		/*更新RTC时钟*/
		//(void)DEVEXTRTCSetNowClockAPI(&gSysClock);
	}
	//校验下发的时间无效
	else
	{
		/*重新更正校时字段*/
		SysClockDisplayAPI();
	}
}
/*=================================================================================================
** @Name      : void SysClockUpdateAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 将显示区校时信息更新到系统时钟信息
** @The notes : 校时信息变化时调用
**===============================================================================================*/
void SysClockUpdateAPI(void)
{
    if(gVerTime_0 == NULL)
	{
		ssdk_printf(SSDK_WARNING, "%s: gVerTime_0 null\r\n", __func__);
        return;
	}
	/*从外部校时信息更新系统时钟*/
	gSysClock.year = gVerTime_0[eICCTim0_Y];
	gSysClock.month = (gVerTime_0[eICCTim0_MD] >> 8) & 0xff;
	gSysClock.day = gVerTime_0[eICCTim0_MD] & 0xff;
	gSysClock.hour = (gVerTime_0[eICCTim0_HM] >> 8) & 0xff;
	gSysClock.minute = gVerTime_0[eICCTim0_HM] & 0xff;
	gSysClock.second = gVerTime_0[eICCTim0_MS] / 1000;
	gSysClock.msecond = gVerTime_0[eICCTim0_MS] % 1000;
}
/*=================================================================================================
** @Name      : void SysClockDisplayAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 更新系统时钟信息到显示区
** @The notes : 查询系统时间时调用
**===============================================================================================*/
void SysClockDisplayAPI(void)
{
    if(gVerTime_0 == NULL)
	{
		ssdk_printf(SSDK_WARNING, "%s: gVerTime_0 null\r\n", __func__);
        return;
	}
	gVerTime_0[eICCTim0_Y] = gSysClock.year;
	gVerTime_0[eICCTim0_MD] = ((u16)gSysClock.month << 8) | (u16)gSysClock.day;
	gVerTime_0[eICCTim0_HM] = ((u16)gSysClock.hour << 8) | (u16)gSysClock.minute;
	gVerTime_0[eICCTim0_MS] = (u16)gSysClock.second * 1000 + (u16)gSysClock.msecond;
}
/*=================================================================================================
** @Name      : void ClrSysSecondTimeAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 清除电池组系统运行计时
** @The notes :
**===============================================================================================*/
void ClrSysSecondTimeAPI(void)
{
	gSysSecondTime = 0;
}
/*=================================================================================================
** @Name      : u32 GetSysSecondTimeAPI(void)
** @Input     : void
** @Output    : 系统计时 1s
** @Function  : 获取电池组系统运行计时
** @The notes : 上电开始计时,考虑可能会外部清零复位
**===============================================================================================*/
u32 GetSysSecondTimeAPI(void)
{
	return(gSysSecondTime);
}
/*=================================================================================================
** @Name      : t_SysClock GetSysDateClockAPI(void)
** @Input     : void
** @Output    : 系统时钟 年/月/日/时/分/秒/毫秒
** @Function  : 获取电池组系统时钟
** @The notes : 北京时间
**===============================================================================================*/
t_SysClock GetSysDateClockAPI(void)
{
	return(gSysClock);
}
/*=================================================================================================
** @Name      : static void UpdateMonAndYearAccdDay(t_SysClock *ptm)
** @Input     : ptm :系统时间
** @Output    : void
** @Function  : 根据天数更新月和年
** @The notes :
**===============================================================================================*/
static void UpdateMonAndYearAccdDay(t_SysClock *ptm)
{
    switch(ptm->month)
    {
        /*如果当前月份是31天*/
        case 1:
        case 3:
        case 5:
        case 7:
        case 8:
        case 10:
        case 12:
            if(ptm->day >= 32)
            {
                ptm->day = 1;
                ptm->month++;

                /*更新年份*/
                if(ptm->month >= 13)
                {
                    ptm->month = 1;
                    ptm->year++;
                }
            }
            break;

        case 2:
            /*判断当前是否为闰年*/
            if(((ptm->year % 4 == 0) && (ptm->year % 100 != 0)) || (ptm->year % 400 == 0))
            {
                /*为闰年有29天*/
                if(ptm->day >= 30)
                {
                    ptm->day = 1;
                    ptm->month++;
                }
            }
            else
            {
                if(ptm->day >= 29)
                {
                    ptm->day = 1;
                    ptm->month++;
                }
            }
            break;

        default:
            /*其他月份只有30天*/
            if(ptm->day >= 31)
            {
                ptm->day = 1;
                ptm->month++;
            }
            break;
    }
}
