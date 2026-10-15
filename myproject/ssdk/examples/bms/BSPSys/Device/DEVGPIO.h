/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVGPIO.h
** @Founder      : ZQD808
** @Date         : 2025.11.10
** @Function     : 输入输出IO管理函数模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#ifndef __DEVGPIO_H__
#define __DEVGPIO_H__
#include "BSPTypeDef.h"
#include "GroupInfo.h"


#define SPI4_595_NUM   (2)            //SPI4 595级联数量
#define SPI8_595_NUM   (2)            //SPI8 595级联数量

#define SPI6_165_NUM   (3)            //SPI6 165级联数量
#define SPI3_595_NUM   (2)            //SPI3 165级联数量
/*输入端口序号*/
#if(BOARDX1_TYPE == 0)
typedef enum
{
    ePIn_POWER_DI         = 0,      	/*电源回检DI信号*/
    //以下为SPI6移位寄存器复用IO
    ePIn_NK1_STATUS       = 1,      	/*第一路低边开关状态信号，0表示故障*/
    ePIn_NK2_STATUS       = 2,      	/*第二路低边开关状态信号，0表示故障*/
    ePIn_NK3_STATUS       = 3,      	/*第三路低边开关状态信号，0表示故障*/
    ePIn_NK4_STATUS       = 4,      	/*第四路低边开关状态信号，0表示故障*/
    ePIn_PWDO_BOARD       = 5,      	/*DO板连接状态*/
    ePIn_DI_BOARD         = 6,      	/*DI板连接状态*/
    ePIn_DI9              = 7,      	/*DI9*/
    ePIn_DI10             = 8,      	/*DI10*/
    ePIn_DI1              = 9,      	/*DI1*/
    ePIn_DI2              = 10,      	/*DI2*/
    ePIn_DI3              = 11,      	/*DI3*/
    ePIn_DI4              = 12,      	/*DI4*/
    ePIn_DI5              = 13,      	/*DI5*/
    ePIn_DI6              = 14,      	/*DI6*/
    ePIn_DI7              = 15,      	/*DI7*/
    ePIn_DI8              = 16,      	/*DI8*/
    //以下为SPI3移位寄存器复用IO
    ePIn_485_6_5V_DI      = 17,      	/*485_6_5V_DI*/
    ePIn_485_4_5V_DI      = 18,      	/*485_4_5V_DI*/
    ePIn_485_2_5V_DI      = 19,      	/*485_2_5V_DI*/
    ePIn_CAN_2_5V_DI      = 20,      	/*CAN_2_5V_DI*/
    ePIn_CAN_4_5V_DI      = 21,      	/*CAN_4_5V_DI*/
    ePIn_232_12_5V_DI     = 22,      	/*232_12_5V_DI*/
    ePIn_232_34_5V_DI     = 23,      	/*232_34_5V_DI*/
    ePIn_VCCP_3V3_DI      = 24,      	/*VCCP_3V3_DI*/
    ePIn_TYPE_C_5V_DI     = 25,      	/*TYPE_C_5V_DI*/
    ePIn_DO_BOARD_DI      = 26,      	/*DO_BOARD_DI*/
    ePIn_LVDS_BOARD_DI    = 27,      	/*LVDS_BOARD_DI*/





}e_INPUTPORT;

/*输出端口序号*/
typedef enum
{
    ePOut_SPI4595_EN      	= 0, 		/*SPI4595 EN引脚*/
    ePOut_SPI4595_RCLK      = 1, 		/*SPI4595_RCLK引脚*/
    ePOut_SPI8595_EN      	= 2, 		/*SPI4595 EN引脚*/
    ePOut_SPI8595_RCLK      = 3, 		/*SPI4595_RCLK引脚*/

    ePOut_SPI6165_CE      	= 4, 		/*SPI6165 CE引脚*/
    ePOut_SPI6165_PL      	= 5, 		/*SPI6165 PL引脚*/
    ePOut_SPI3165_CE      	= 6, 		/*SPI3165 CE引脚*/
    ePOut_SPI3165_PL      	= 7, 		/*SPI3165 PL引脚*/


    ePOut_RUN_LED      	= 8, 		/*运行指示灯引脚*/
    ePOut_ALM_LED      	= 9, 		/*告警指示灯引脚*/
    //以下为SPI4移位寄存器复用IO
    ePOut_PK1_DRV      	= 10, 		/*第一路高边驱动芯片通道0输入*/
    ePOut_PK12_DEN      = 11, 		/*第一个高边驱动芯片诊断功能使能引脚*/
    ePOut_PK12_DSEL     = 12, 		/*第一个高边驱动芯片诊断通道选择*/
    ePOut_PK2_DRV      	= 13, 		/*第一路高边驱动芯片通道1输入*/
    ePOut_PK3_DRV      	= 14, 		/*第三路高边驱动芯片通道0输入*/
    ePOut_PK34_DEN      = 15, 		/*第二个高边驱动芯片诊断功能使能引脚*/
    ePOut_PK34_DSEL     = 16, 		/*第二个高边驱动芯片诊断通道选择*/
    ePOut_PK4_DRV      	= 17, 		/*第四路高边驱动芯片通道1输入*/
    ePOut_NK1_DRV      	= 18, 		/*第一路低边驱动芯片输入*/
    ePOut_NK2_DRV      	= 19, 		/*第二路低边驱动芯片输入*/
    ePOut_NK3_DRV      	= 20, 		/*第三路低边驱动芯片输入*/
    ePOut_NK4_DRV      	= 21, 		/*第四路低边驱动芯片输入*/
    //以下为SPI8移位寄存器复用IO
    ePOut_DO1      	    = 22, 		/*DO1*/
    ePOut_DO2           = 23, 		/*DO2*/
    ePOut_DO3           = 24, 		/*DO3*/
    ePOut_DO4      	    = 25, 		/*DO4*/
    ePOut_DO5      	    = 26, 		/*DO5*/
    ePOut_DO6           = 27, 		/*DO6*/
    ePOut_DP_RST         = 28, 		/*DP复位*/
    ePOut_LVDS_PW_EN    = 29, 		/*LVDS电源使能*/
    ePOut_4G_PW_EN      = 30, 		/*4G电源使能*/
    ePOut_4G_RST      	= 31, 		/*4G复位*/
    ePOut_HDMI_RST      = 32, 		/*HDMI复位*/
    ePOut_USB_RST      	= 33, 		/*USB复位*/
    ePOut_4051_S0      	= 34, 		/*4051_S0片选*/
    ePOut_4051_S1      	= 35, 		/*4051_S1片选*/
    ePOut_4051_S2      	= 36, 		/*4051_S2片选*/

}e_OutputPort;


#elif(BOARDX1_TYPE == 1)
typedef enum
{
    ePIn_RESERVED         = 0,      	/*预留*/
    ePIn_POWER_DI         = 1,      	/*电源回检DI信号*/
    ePIn_TYPE_C_5V_DI     = 2,      	/*TYPE_C_5V_DI信号*/
    ePIn_CAN_2_5V_DI      = 3,      	/*CAN 2.5VDI信号*/
    ePIn_CAN_4_5V_DI      = 4,      	/*CAN 4.5VDI信号*/
    ePIn_RS485_2_5V_DI    = 5,      	/*RS485 2.5VDI信号*/
    ePIn_RS485_4_5V_DI    = 6,      	/*RS485 4.5VDI信号*/
    ePIn_VCCP_3_3V_DI     = 7,      	/*VCCP 3.3VDI信号*/
    //以下为移位寄存器复用IO
    ePIn_NK1_STATUS       = 8,      	/*第一路低边开关状态信号，0表示故障*/
    ePIn_NK2_STATUS       = 9,      	/*第二路低边开关状态信号，0表示故障*/
    ePIn_NK3_STATUS       = 10,      	/*第三路低边开关状态信号，0表示故障*/
    ePIn_NK4_STATUS       = 11,      	/*第四路低边开关状态信号，0表示故障*/
    ePIn_PWDO_BOARD       = 12,      	/*DO板连接状态*/
    ePIn_DI_BOARD         = 13,      	/*DI板连接状态*/
    ePIn_DI9              = 14,      	/*DI9*/
    ePIn_DI10             = 15,      	/*DI10*/
    ePIn_DI1              = 16,      	/*DI1*/
    ePIn_DI2              = 17,      	/*DI2*/
    ePIn_DI3              = 18,      	/*DI3*/
    ePIn_DI4              = 19,      	/*DI4*/
    ePIn_DI5              = 20,      	/*DI5*/
    ePIn_DI6              = 21,      	/*DI6*/
    ePIn_DI7              = 22,      	/*DI7*/
    ePIn_DI8              = 23,      	/*DI8*/


}e_INPUTPORT;

/*输出端口序号*/
typedef enum
{
    ePOut_SPI4595_EN    = 0, 		/*595 EN引脚*/
    ePOut_SPI4595_RCLK  = 1, 		/*595_RCLK引脚*/
    ePOut_SPI6165_CE    = 2, 		/*165 CE引脚*/
    ePOut_SPI6165_PL    = 3, 		/*165 PL引脚*/
    ePOut_WDT_EN      	= 4, 		/*看门狗使能引脚*/
    /* 看门狗WDI喂狗引脚 */
    ePOut_WDT_WDI       = 22, 		/*看门狗WDI喂狗引脚*/
    ePOut_RUN_LED      	= 5, 		/*运行指示灯引脚*/
    ePOut_ALM_LED      	= 6, 		/*告警指示灯引脚*/
    ePOut_4051_S0      	= 7, 		/*4051 S0引脚*/
    ePOut_4051_S1      	= 8, 		/*4051 S1引脚*/
    ePOut_4051_S2      	= 9, 		/*4051 S2引脚*/
    //以下为移位寄存器复用IO
    ePOut_PK1_DRV      	= 10, 		/*第一路高边驱动芯片通道0输入*/
    ePOut_PK12_DEN      = 11, 		/*第一个高边驱动芯片诊断功能使能引脚*/
    ePOut_PK12_DSEL     = 12, 		/*第一个高边驱动芯片诊断通道选择*/
    ePOut_PK2_DRV      	= 13, 		/*第一路高边驱动芯片通道1输入*/
    ePOut_PK3_DRV      	= 14, 		/*第三路高边驱动芯片通道0输入*/
    ePOut_PK34_DEN      = 15, 		/*第二个高边驱动芯片诊断功能使能引脚*/
    ePOut_PK34_DSEL     = 16, 		/*第二个高边驱动芯片诊断通道选择*/
    ePOut_PK4_DRV      	= 17, 		/*第四路高边驱动芯片通道1输入*/
    ePOut_NK1_DRV      	= 18, 		/*第一路低边驱动芯片输入*/
    ePOut_NK2_DRV      	= 19, 		/*第二路低边驱动芯片输入*/
    ePOut_NK3_DRV      	= 20, 		/*第三路低边驱动芯片输入*/
    ePOut_NK4_DRV      	= 21, 		/*第四路低边驱动芯片输入*/
}e_OutputPort;
#endif
/*输入输出端口状态(闭合/断开)*/
typedef enum
{
    ePSTATE_OFF  = 0,
    ePSTATE_ON   = 1,
    ePSTATE_NUM
    
}e_GPIOState;

/*I/O状态*/
typedef enum
{
    eIOLOW = 0,             //低电平
    eIOHIG = 1,             //高电平
    
    eIOSTATE                //IO状态数目
}e_IOState;

//复用端口序号
typedef enum
{
    ePMul_D0 = 0,       		/*复用端口D0*/
    ePMul_D1 = 1,       		/*复用端口D1*/
    ePMul_D2 = 2,       		/*复用端口D2*/
    ePMul_D3 = 3,       		/*复用端口D3*/
    ePMul_D4 = 4,       		/*复用端口D4*/
    ePMul_D5 = 5,       		/*复用端口D5*/
    ePMul_D6 = 6,       		/*复用端口D6*/
    ePMul_D7 = 7,       		/*复用端口D7*/
    ePMul_D8 = 8,       		/*复用端口D8*/
    ePMul_D9 = 9,       		/*复用端口D9*/
    ePMul_D10 = 10,       		/*复用端口D10*/
    ePMul_D11 = 11,       		/*复用端口D11*/
    ePMul_D12 = 12,       		/*复用端口D12*/
    ePMul_D13 = 13,       		/*复用端口D13*/
    ePMul_D14 = 14,       		/*复用端口D14*/
    ePMul_D15 = 15,       		/*复用端口D15*/

}e_MULPORT;


/*165 SPI通道*/
typedef enum
{
    e_SPI3_165 = 0,             //SPI3_165
    e_SPI6_165 = 1,             //SPI6_165
    
    e_165SPINum                 //165 SPI序号          
}e_165SPI_CHANNEL;

/*595 SPI通道*/
typedef enum
{
    e_SPI4_595 = 0,             //SPI4_595
    e_SPI8_595 = 1,             //SPI8_595
    
    e_595SPINum                 //165 SPI序号          
}e_595SPI_CHANNEL;
/*=============================================================================================
** @Name      : void SetMuiPortDiData(e_165SPI_CHANNEL Channel , u8 Num165,u8 Data)
** @Input     : Channel:SPI通道  Num165:级联的165序号 Data：DI数据
** @Output    : void
** @Function  : 获取SN74HC165
**===========================================================================================*/
void SetMuiPortDiData(e_165SPI_CHANNEL Channel , u8 Num165,u8 Data);
/*=============================================================================================
** @Name      : void DEVGPIOInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : GPIO端口初始化
** @The notes : GPIO初始化已在board_port_init()函数初始化，如果PINMUX配置没问题的话可以无需调用此函数
**===========================================================================================*/
void DEVGPIOInit(void);
/*=============================================================================================
** @Name      : u8 DEVGPIOGetInPortState(e_INPUTPORT port)
** @Input     : port:输入端口号
** @Output    : 端口状态[0:断开 1:闭合]
** @Function  : 获取GPIO输入端口状态
**===========================================================================================*/
u8 DEVGPIOGetInPortState(e_INPUTPORT port);
/*=============================================================================================
** @Name      : void DEVGPIOSetOutputState(e_OutputPort port, e_GPIOState state)
** @Input     : port:输出端口号 state:输出状态[0:断开 1:闭合]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 设置GPIO输出端口状态
**===========================================================================================*/
void DEVGPIOSetPortState(e_OutputPort port, e_GPIOState state);
/*=============================================================================================
** @Name      : u8 DEVGPIOGetOutPortState(e_OutputPort port)
** @Input     : port:输出端口号
** @Output    : 端口状态[0:断开 1:闭合]
** @Function  : 获取GPIO输出端口状态
**===========================================================================================*/
u8 DEVGPIOGetOutPortState(e_OutputPort port);
/*=============================================================================================
** @Name      : void DEVGPIOMulPortManagTask(void *argument)
** @Input     : void
** @Output    : void
** @Function  : GPIO复用端口管控任务
**===========================================================================================*/
void DEVGPIOMulPortManagTask(void *argument);
/*=============================================================================================
** @Name      : u8 DEVGPIOGetSPI6DiState3(void)
** @Input     : void
** @Output    : SPI6第三片165DI状态(按位表示D16-D23)
** @Function  : 获取SPI6第三片165的数字输入状态
** @The notes : 供dbg_test stat读取DI1-4接触器反馈; bit0=DI1(主正1) bit1=DI2(主负1) bit2=DI3(主正2) bit3=DI4(主负2)
**=============================================================================================*/
u8 DEVGPIOGetSPI6DiState3(void);
#endif