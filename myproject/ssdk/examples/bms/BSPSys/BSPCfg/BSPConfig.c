/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPConfig.c
** @Founder      : ZQD808
** @Date         : 2025.12.05
** @Function     : 各驱动模块初始化接口
**===========================================================================================*/
#include "BSPConfig.h"
#include "InsuCheck.h"
#include "can_uds_interface.h"
#include "DEVWDT.h"
#include "GD25Q32.h"
volatile u_BSPFunCfg gBSPFunCfg;
static s_BSPParaCfg gBSPParaCfg;
/*=============================================================================================
** @Name      : void BSPUserCfgInit(void)
** @Input     : void
** @Output    : void
** @Function  : BSP驱动功能用户配置模块参数初始化
** @The notes : Main()函数调用，上电初始化
**===========================================================================================*/
void BSPUserCfgInit(void)
{
    /*读取驱动模块使能配置*/
	gBSPFunCfg.Bits.CAN0En = CAN0_FUN_EN;		/*CAN0使能配置*/
	gBSPFunCfg.Bits.CAN1En = CAN1_FUN_EN;		/*CAN1使能配置*/
	gBSPFunCfg.Bits.CAN2En = CAN2_FUN_EN;		/*CAN2使能配置*/
    gBSPFunCfg.Bits.CAN3En = CAN3_FUN_EN;		/*CAN3使能配置*/

    gBSPFunCfg.Bits.UART5En = UART5_FUN_EN;		/*UART5使能配置*/
	gBSPFunCfg.Bits.UART9En = UART9_FUN_EN;		/*UART9使能配置*/

	gBSPFunCfg.Bits.SPI3En = SPI3_FUN_EN;		/*SPI3使能配置*/
    gBSPFunCfg.Bits.SPI4En = SPI4_FUN_EN;		/*SPI4使能配置*/
    gBSPFunCfg.Bits.SPI6En = SPI6_FUN_EN;		/*SPI6使能配置*/
    gBSPFunCfg.Bits.SPI8En = SPI8_FUN_EN;		/*SPI8使能配置*/


	gBSPFunCfg.Bits.GPIOEn = GPIO_FUN_EN;       /*GPIO使能配置*/
	gBSPFunCfg.Bits.ADCEn  = ADC_FUN_EN;        /*ADC使能配置*/
	gBSPFunCfg.Bits.EEPEn  = EEP_FUN_EN;        /*EEP使能配置*/
	gBSPFunCfg.Bits.FLASHEn= FLASH_FUN_EN;      /*FLASH使能配置*/

    gBSPFunCfg.Bits.TIMEREn= TIMER_FUN_EN;      /*定时器使能配置*/
    gBSPFunCfg.Bits.WDTEn  = WDT_FUN_EN;        /*看门狗使能配置*/

	/*读取驱动模块功能参数配置信息*/
	gBSPParaCfg.Can0Rate   = CAN0_BPS;         /*CAN0通信波特率(500kbps)*/
	gBSPParaCfg.Can1Rate   = CAN1_BPS;         /*CAN1通信波特率(5000kbps)*/
	gBSPParaCfg.Can2Rate   = CAN2_BPS;         /*CAN2通信波特率(5000kbps)*/

    gBSPParaCfg.Uart5Rate   = UART5_BPS;       /*串口5波特率(9600kbps)*/
	gBSPParaCfg.Uart9Rate   = UART9_BPS;       /*串口9波特率(9600kbps)*/

	gBSPParaCfg.Spi3Rate   = SPI3_BPS;         /*SPI0通信波特率(1Mbps)*/
	gBSPParaCfg.Spi4Rate   = SPI4_BPS;         /*SPI1通信波特率(1Mbps)*/
	gBSPParaCfg.Spi6Rate   = SPI6_BPS;         /*SPI2通信波特率(1Mbps)*/
	gBSPParaCfg.Spi8Rate   = SPI8_BPS;         /*SPI3通信波特率(1Mbps)*/
}
/*=============================================================================================
** @Name      : void BSPDriverInit(void)
** @Input     : void
** @Output    : void
** @Function  : 所有底层驱动模块进行初始化
** @The notes : Main()函数调用，上电初始化
**===========================================================================================*/
void BSPDriverInit(void)
{
    if(1 == gBSPFunCfg.Bits.TIMEREn)     /*定时器使能*/
    {
        DEVTimerInit();
    }

    if(1 == gBSPFunCfg.Bits.UART5En)     /*UART5使能*/
    {
        //DEVUARTResetInit(eUART5, gBSPParaCfg.Uart5Rate);
    }
    if(1 == gBSPFunCfg.Bits.UART9En)     /*UART9使能*/
    {
        //DEVUARTResetInit(eUART9, gBSPParaCfg.Uart9Rate);
    }
    if(1 == gBSPFunCfg.Bits.CAN0En)     /*CAN0使能*/
    {
        //DEVCANResetInit(eCAN0, FALSE);
    }
    if(1 == gBSPFunCfg.Bits.CAN1En)     /*CAN1使能*/
    {
        DEVCANResetInit(eCAN1, FALSE);
    }
    if(1 == gBSPFunCfg.Bits.CAN2En)     /*CAN2使能*/
    {
        DEVCANResetInit(eCAN2, FALSE);
    }
    if(1 == gBSPFunCfg.Bits.CAN3En)     /*CAN3使能*/
    {
        //DEVCANResetInit(eCAN3, FALSE);
    }
    DEVDriverInit(); /*初始化外围硬件*/
    /* 看门狗初始化：WDTEn使能则调用DEVWDTAllInit */
    if(1 == gBSPFunCfg.Bits.WDTEn)
    {
        DEVWDTAllInit();
    }
}
/*=============================================================================================
** @Name      : void BSPUserCfgInit(void)
** @Input     : void
** @Output    : void
** @Function  : 所有外围设备驱动模块进行初始化
** @The notes : Main()函数调用，上电初始化。必须在底层驱动初始化完成后调用
**===========================================================================================*/
void DEVDriverInit(void)
{
    //DEVUARTSendBufInit();
    DEVGPIOInit();
    SN74HC165Init();
    CD74HC595Init();
    DEVADCInit();
    EXTEep24M512Init();
    InsuCheckInit();
    init_uds();
#if !defined(CONFIG_XIP_MODE)
    NORFlashGD25Init();
#endif
}

