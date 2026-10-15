/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : BSPConfig.h
** @Founder      : ZQD808
** @Date         : 2025.12.05
** @Function     : 各驱动模块初始化接口
** @Instructions :
**===========================================================================================*/
#ifndef __BSPCONFIG_H__
#define __BSPCONFIG_H__
#include "BSPTypeDef.h"
#include "DEVADC.h"
#include "DEVCAN.h"
#include "DEVGPIO.h"
#include "DEVUART.h"
#include "CD74HC595.h"
#include "SN74HC165.h"
#include "M2514.h"
#include "GD25Q32.h"
#include "DEVTIMER.h"

/*=======================================各个模块使能配置=====================================*/
#define	CAN0_FUN_EN     0           /*CAN0模块使能                     1使能该模块  0禁止该模块*/
#define	CAN1_FUN_EN	    1           /*CAN1模块使能                     1使能该模块  0禁止该模块*/
#define CAN2_FUN_EN     1           /*CAN2模块使能                     1使能该模块  0禁止该模块*/
#define CAN3_FUN_EN     0           /*CAN3模块使能                     1使能该模块  0禁止该模块*/
#define UART5_FUN_EN    0           /*UART5使能                        1使能该模块  0禁止该模块*/
#define UART9_FUN_EN    0           /*UART9使能                        1使能该模块  0禁止该模块*/
#define EEP_FUN_EN      1           /*EEPROM模块使能                    1使能该模块  0禁止该模块*/
#define FLASH_FUN_EN    0           /*FALSH模块使能                     1使能该模块  0禁止该模块*/
#define GPIO_FUN_EN     1           /*IO口模块使能                      1使能该模块  0禁止该模块*/
#define SPI3_FUN_EN     0           /*SPI_3使能                         1使能该模块  0禁止该模块*/
#define SPI4_FUN_EN     1           /*SPI_4使能                         1使能该模块  0禁止该模块*/
#define SPI6_FUN_EN     1           /*SPI_6使能                         1使能该模块  0禁止该模块*/
#define SPI8_FUN_EN     0           /*SPI_8使能                         1使能该模块  0禁止该模块*/
#define ADC_FUN_EN      1           /*ADC采样模块使能                    1使能该模块  0禁止该模块*/
#define TIMER_FUN_EN    1           /*定时器模块使能                     1使能该模块  0禁止该模块*/
#define WDT_FUN_EN      0           /*看门狗模块使能                     1使能该模块  0禁止该模块*/


#define	CAN0_BPS        (500)         /*只能为 125  500 5000(单位为K)*/
#define	CAN1_BPS        (5000)        /*只能为 125  500 5000(单位为K)*/
#define	CAN2_BPS        (5000)        /*只能为 125  500 5000(单位为K)*/
#define	CAN3_BPS        (5000)        /*只能为 125  500 5000(单位为K)*/

#define SPI3_BPS        (1)         /*SPI3通讯波特率（单位为M）*/
#define SPI4_BPS        (1)         /*SPI4通讯波特率（单位为M）*/
#define SPI6_BPS        (1)         /*SPI6通讯波特率（单位为M）*/
#define SPI8_BPS        (1)         /*SPI8通讯波特率（单位为M）*/

#define UART5_BPS        (9600)     /*只能是4800 9600 19200 38400*/
#define UART9_BPS        (9600)     /*只能是4800 9600 19200 38400*/

/*各功能模块使能配置转化为参数结构体*/
typedef union _FunConfig_
{
    u32 Word;
    struct
    {
        u16 CAN0En      :1;         /*CAN功能使能*/
        u16 CAN1En      :1;
        u16 CAN2En      :1;
        u16 CAN3En      :1;
        u16 UART5En     :1;         /*UART功能使能位*/
        u16 UART9En     :1;
        u16 SPI3En      :1;         /*SPI功能使能位*/
        u16 SPI4En      :1;
        u16 SPI6En      :1;
        u16 SPI8En      :1;
        u16 GPIOEn      :1;         /*GPIO功能使能位*/
        u16 ADCEn       :1;         /*AD采样功能使能位*/
        u16 EEPEn       :1;         /*EEP功能使能位*/
        u16 FLASHEn     :1;         /*FLASH功能使能*/
        u16 TIMEREn     :1;         /*FLASH功能使能*/
        u16 WDTEn       :1;         /*看门狗功能使能*/
    }Bits;
}u_BSPFunCfg;

/*各功能模块参数*/
typedef struct
{

    u16 Can0Rate;                   /*CAN0通信波特率(1kbps)*/
    u16 Can1Rate;                   /*CAN1通信波特率(1kbps)*/
    u16 Can2Rate;                   /*CAN2通信波特率(1kbps)*/
    u16 Uart5Rate;                 /*SCI0通信波特率(1bps)*/
    u16 Uart9Rate;                   /*SCI1通信波特率(1bps)*/
    u16 Spi3Rate;                   /*SPI1通信波特率(1Mbps)*/
    u16 Spi4Rate;                   /*SPI2通信波特率(1Mbps)*/
    u16 Spi6Rate;                   /*SPI3通信波特率(1Mbps)*/
    u16 Spi8Rate;                   /*SPI3通信波特率(1Mbps)*/

} s_BSPParaCfg;

/*=============================================================================================
** @Name      : void BSPUserCfgInit(void)
** @Input     : void
** @Output    : void
** @Function  : BSP驱动功能用户配置模块参数初始化
** @The notes : Main()函数调用，上电初始化
**===========================================================================================*/
void BSPUserCfgInit(void);
/*=============================================================================================
** @Name      : void BSPUserCfgInit(void)
** @Input     : void
** @Output    : void
** @Function  : 所有外围设备驱动模块进行初始化
** @The notes : Main()函数调用，上电初始化。必须在底层驱动初始化完成后调用
**===========================================================================================*/
void DEVDriverInit(void);
/*=============================================================================================
** @Name      : void BSPDriverInit(void)
** @Input     : void
** @Output    : void
** @Function  : 所有底层驱动模块进行初始化
** @The notes : Main()函数调用，上电初始化
**===========================================================================================*/
void BSPDriverInit(void);
#endif