/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVWDT.c
** @Founder      : SYS
** @Date         : 2026.07.23
** @Function     : 看门狗驱动（内部Kunlun WDT + 外部GPIO WDT + CLI调试）
** @Instructions : 内狗APB_WDT1_BASE 2s超时，外狗GPIO113 WDI 1.6s超时
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "DEVWDT.h"
#include "BSPConfig.h"
#include "driver/watchdog.h"
#include "driver/kunlun/watchdog/sdrv_watchdog_drv.h"
#include "irq_num.h"
#include "regs_base.h"
#include <debug.h>
#include <stdio.h>
#include <CLI.h>
#include "CLI_type.h"
#include <string.h>
#include <stdlib.h>
#include <cmsis_os2.h>

extern volatile u_BSPFunCfg gBSPFunCfg;

#define INT_WDT_TIMEOUT_S     2

static u8 sExtFeed = 0;                     /* 外部狗喂狗开关: 1喂狗 0停止 */
static u8 sIntFeed = 0;                     /* 内部狗喂狗开关: 1喂狗 0停止 */

/*=============================================================================================
** @Name      : 内部WDT设备变量 
** @Function  : gDevWdt是watchdog_device_t通用设备句柄；gSdrvWdt是Kunlun平台私有配置
** @The notes : wdt_id=WATCHDOG1配合int_rst=true使RSTGEN将WDT1复位信号路由到AP域
**=============================================================================================*/
static watchdog_device_t gDevWdt;
static sdrv_watchdog_device_t gSdrvWdt = {
    .base    =  APB_WDT1_BASE,              /* WDT1寄存器基址 0xF01C0000 */
    .irq     =  WDT1_OVFLOW_INTR_NUM,       /* WDT1溢出中断号 */
    .isr     =  NULL,                       /* 不使用中断，直接用硬件复位 */
    .wdt_id  =  WATCHDOG1,                  /* WDT实例编号，配合int_rst配置RSTGEN */
    .int_rst =  true,                       /* 使能内部复位：sdrv_watchdog_init会配置RSTGEN通路 */
};

/* 外部看门狗部分 */

static u8 sExtWdtToggle = 0;                /* 外部狗WDI翻转计数: 每次^1实现高低交替 */

/*=============================================================================================
** @Name      : static void ExtWdtInit(void)
** @Input     : void
** @Output    : void
** @Function  : 使能外部看门狗芯片（WDT_EN=高,WDI=低），立即翻转一次WDI喂狗 
** @The notes : EN(GPIO115)拉高后外狗就开始倒计时，必须先喂一次再printf
**===========================================================================================*/
static void ExtWdtInit(void)
{
    sExtWdtToggle = 0;
    DEVGPIOSetPortState(ePOut_WDT_EN, ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_WDT_WDI, ePSTATE_OFF);
    DEVGPIOSetPortState(ePOut_WDT_WDI, ePSTATE_ON);
    DEVGPIOSetPortState(ePOut_WDT_WDI, ePSTATE_OFF);
    printf("ExtWDT init done, EN=GPIO115, WDI=GPIO113\r\n");
}

/*=============================================================================================
** @Name      : static void ExtWdtFeed(void)
** @Input     : void
** @Output    : void
** @Function  : 翻转WDI(GPIO_113)电平喂外部看门狗 
** @The notes : 每50ms调用一次，100ms完成一个完整高低翻转周期
**===========================================================================================*/
static void ExtWdtFeed(void)
{
    sExtWdtToggle = sExtWdtToggle ^ 1;
    DEVGPIOSetPortState(ePOut_WDT_WDI, sExtWdtToggle ? ePSTATE_ON : ePSTATE_OFF);
}

/* 内部看门狗部分 */

/*=============================================================================================
** @Name      : static void IntWdtInit(void)
** @Input     : void
** @Output    : void
** @Function  : 初始化内部Kunlun WDT 
** @The notes : 按官方Demo方式：sdrv_watchdog_init→set_timeout→start，DRV层自动清零寄存器+软复位
**===========================================================================================*/
static void IntWdtInit(void)
{
    /* step1: 清零WDG_CTRL消除bootloader遗留状态 */
    if (sdrv_watchdog_init(&gDevWdt, &gSdrvWdt))
    {
        printf("IntWDT init FAIL\r\n");
        return;
    }
    /* step2: 设置超时时间(秒)，函数内部×时钟频率计算WTC */
    if (watchdog_set_timeout(&gDevWdt, INT_WDT_TIMEOUT_S))
    {
        printf("IntWDT set_timeout FAIL\r\n");
        return;
    }
    /* step3: 使能WDT并软复位计数器，确保从0开始倒数 */
    if (watchdog_start(&gDevWdt))
    {
        printf("IntWDT start FAIL\r\n");
        return;
    }
    printf("IntWDT init done, base=0x%X, timeout=%ds\r\n",
           (u32)(uintptr_t)APB_WDT1_BASE, INT_WDT_TIMEOUT_S);
}

/*=============================================================================================
** @Name      : static void IntWdtFeed(void)
** @Input     : void
** @Output    : void
** @Function  : 喂内部Kunlun WDT 
** @The notes : 使用DRV层watchdog_ping
**===========================================================================================*/
static void IntWdtFeed(void)
{
    watchdog_ping(&gDevWdt);
}

/* 对外接口 */

/*=============================================================================================
** @Name      : void DEVWDTAllInit(void)
** @Input     : void
** @Output    : void
** @Function  : 初始化内外看门狗并随后立即喂一次狗 
** @The notes : 先内部WDT再外部WDT，最后置sExtFeed=1,sIntFeed=1并喂狗
**===========================================================================================*/
void DEVWDTAllInit(void)
{
    IntWdtInit();
    ExtWdtInit();
    sExtFeed = 1;
    sIntFeed = 1;
    IntWdtFeed();
    ExtWdtFeed();
    printf("WDT all init done\r\n");
}

/*=============================================================================================
** @Name      : void DEVWDTFeedTask(void)
** @Input     : void
** @Output    : void
** @Function  : 每50ms统一喂狗入口 
** @The notes : 检查WDTEn使能位；sIntFeed控制内部狗，sExtFeed控制外部狗
**===========================================================================================*/
void DEVWDTFeedTask(void)
{
    if (!gBSPFunCfg.Bits.WDTEn)
    {
        return;
    }
    /* sIntFeed管内部Kunlun WDT寄存器刷新，sExtFeed管外部GPIO WDI翻转 */
    if (sIntFeed)
    {
        IntWdtFeed();
    }
    if (sExtFeed)
    {
        ExtWdtFeed();
    }
}


#if(0)  /*看门狗调试CLI命令*/
/* 死锁任务：用于测试软件看门狗，prio=25挤死Task50ms(21) */
static void DeadlockTask(void *arg)
{
    (void)arg;
    printf(">>> DeadlockTask started, system will reset in 2s <<<\r\n");
    while (1) { }
}

/*=============================================================================================
** @Name      : static int wdt_test(int argc, char *argv[])
** @Input     : argc:参数个数 argv:参数列表
** @Output    : CLI_FALSE
** @Function  : 看门狗CLI调试命令 
** @The notes : status查看状态、hwstop停止外部喂狗(1.6s复位)、swstop停止内部喂狗(2s复位)
**===========================================================================================*/
static int wdt_test(int argc, char *argv[])
{
    if (argc == 0)
    {
        printf("=== WDT CLI  ===\r\n");
        printf("  wdt status      - show WDT state\r\n");
        printf("  wdt hwstop      - stop external WDT feeding (1.6s reset)\r\n");
        printf("  wdt swstop      - stop internal WDT feeding (2s reset)\r\n");
        printf("  wdt swtest      - create deadlock task to test SW WDT reset\r\n");
        return CLI_FALSE;
    }

    if (!strcmp(argv[0], "status"))
    {
        printf("=== WDT Status  ===\r\n");
        printf("Enable:       %s (WDT_FUN_EN=%d)\r\n",
               gBSPFunCfg.Bits.WDTEn ? "YES" : "NO", WDT_FUN_EN);
        printf("Internal WDT: base=0x%X, timeout=%ds\r\n",
               (u32)(uintptr_t)APB_WDT1_BASE, INT_WDT_TIMEOUT_S);
        printf("External WDT: timeout=1.6s, EN=GPIO115, WDI=GPIO113\r\n");
        printf("Int WDT:      %s (50ms period)\r\n", sIntFeed ? "FEEDING" : "STOPPED");
        printf("Ext WDT:      %s (50ms period)\r\n", sExtFeed ? "FEEDING" : "STOPPED");
        return 0;
    }

    if (!strcmp(argv[0], "hwstop"))
    {
        printf("!!! HW WDT TEST: stop external feeding, chip resets in 1.6s !!!\r\n");
        /* 只停外部狗喂狗(sExtFeed=0)，内部狗仍正常喂狗 */
        sExtFeed = 0;
        return 0;
    }

    if (!strcmp(argv[0], "swstop"))
    {
        printf("!!! SW WDT TEST: stop internal feeding, chip resets in 2s !!!\r\n");
        /* 只停内部狗喂狗(sIntFeed=0)，外部狗仍正常喂狗 */
        sIntFeed = 0;
        return 0;
    }

    if (!strcmp(argv[0], "swtest"))
    {
        printf("!!! SW WDT TEST: creating deadlock task, chip resets in 2s !!!\r\n");
        /* 创建prio=25的死锁任务，挤死prio=21的Task50ms，2s后内狗复位 */
        osThreadAttr_t attr = {
            .name = "SwWdtTest",
            .stack_size = 1024,
            .priority = 25,
        };
        osThreadNew(DeadlockTask, NULL, &attr);
        return 0;
    }

    printf("wdt: unknown subcommand '%s'\r\n", argv[0]);
    return CLI_FALSE;
}
CLI_CMD("wdt", "wdt status/hwstop/swstop/swtest", wdt_test);
#endif
