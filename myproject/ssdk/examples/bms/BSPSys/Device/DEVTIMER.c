/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DECTIMER.c
** @Founder      : ZQD808
** @Date         : 2025.12.17
** @Function     : 定时器模块
** @Instructions : 
**===========================================================================================*/

#include "timer_cfg.h"
#include "PublicInfo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>

#include <types.h>
#include <board.h>

#include <driver/arch_timer.h>

/* timer device driver headers */
#include <driver/kunlun/tmr/sdrv_tmr_drv.h>
#include "DEVTIMER.h"
#include "IccMsgInterface.h"
#include "CurrSample.h"
#include "PublicInfo.h"
#include "BSPUserAPI.h"

sdrv_tmr_dev_t sdrv_timer3;
timer_dev_t *timer3;
sdrv_tmr_dev_t sdrv_timer4;
timer_dev_t *timer4;
volatile unsigned long Timer3Cnt = 0;
volatile unsigned long Timer4Cnt = 0;
static const sdrv_tmr_cfg_t timer3_config = {
    .base = DEVICE_BASE(TIMER3),
    .irq = DEVICE_INTR(TIMER3_CHN),
    .ovf_irq = DEVICE_INTR(TIMER3_OVF),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
    .cascase_mode = false,
};

static const sdrv_tmr_cfg_t timer4_config = {
    .base = DEVICE_BASE(TIMER4),
    .irq = DEVICE_INTR(TIMER4_CHN),
    .ovf_irq = DEVICE_INTR(TIMER4_OVF),
    .clk_sel = SDRV_TMR_LF_CLK,
    .clk_frq = 24000000,
    .clk_div = 1,
    .cascase_mode = true,
};
/*=============================================================================================
** @Name      : static void Timer3Handle(void *arg)
** @Input     : void
** @Output    : void
** @Function  : 定时器3中断函数
** @The notes : 1ms
**===========================================================================================*/
static void Timer3Handle(void *arg)
{
    Timer3Cnt++;
    if(BSPGetOSMillSecTimeAPI() < 1)   /*上电等待初始化不能执行防止出错*/
    {
        return;
    }
    ICCLibTxRcvHandleTask_API();
}
/*=============================================================================================
** @Name      : static void Timer4Handle(void *arg)
** @Input     : void
** @Output    : void
** @Function  : 定时器4中断函数
** @The notes : 1ms
**===========================================================================================*/
static void Timer4Handle(void *arg)
{
    Timer4Cnt++;
    if(BSPGetOSMillSecTimeAPI() < 1)   /*上电等待初始化不能执行防止出错*/
    {
        return;
    }
    ISRCurrCapAddTask();	            /*定时中断电流积分任务*/
    SysClockCountTask();		        /*系统1ms系统时钟计时任务*/
}
void DEVTimerInit(void)
{
    /*创建2路定时器，给APP代码使用*/
    sdrv_tmr_init(&sdrv_timer3, &timer3_config);
    timer3 = sdrv_tmr_timer_probe(&sdrv_timer3, SDRV_TMR_A);
    
    timer_set_callback(timer3, Timer3Handle, NULL);
    timer_start(timer3, TMR_TYPE_PERIOD, 1);
    
    sdrv_tmr_init(&sdrv_timer4, &timer4_config);
    timer4 = sdrv_tmr_timer_probe(&sdrv_timer4, SDRV_TMR_A);
    
    timer_set_callback(timer4, Timer4Handle, NULL);
    timer_start(timer4, TMR_TYPE_PERIOD, 1);
}