/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVGPIO.c
** @Founder      : ZQD808
** @Date         : 2025.11.10
** @Function     : 输入输出IO管理函数模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#include <arch.h>
#include <cmsis_os2.h>
#include <board.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CLI_type.h"
#include "driver/gpio.h"
#include <regs_base.h>
#include <cmsis_os2.h> 
#include "driver/pinctrl.h"
#include "driver/kunlun/pinctrl/sdrv_pinctrl.h"
#include "driver/gpio.h"
#include "driver/kunlun/gpio/sdrv_gpio.h"
#include "irq_num.h"
#include "param.h"
#include "DEVGPIO.h"
#include "SN74HC165.h"
#include "CD74HC595.h"
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include <stdio.h>
#include <debug.h>
#include "TaskMonitor.h"

extern gpio_dev_t g_gpio_ap1;
extern gpio_dev_t g_gpio_secure;
gpio_dev_t *g_bmu02_gpio_dev;

extern u16 *gDevSampInfo_14;                       /*板级采样信息*/

static u16 sSPI4DevMulPortIoCtrlState = 0;         /*按位表示复用端口1-16控制状态(0断开 1闭合)*/
#if(BOARDX1_TYPE == 0)
static u16 sSPI8DevMulPortIoCtrlState = 0;         /*按位表示复用端口1-16控制状态(0断开 1闭合)*/
static u8 sSPI3DevMulPortDiState1 = 0;             /*按位表示第一个165DI状态(0断开 1闭合)*/
static u8 sSPI3DevMulPortDiState2 = 0;             /*按位表示第二个165DI状态(0断开 1闭合)*/
#endif

static u8 sSPI6DevMulPortDiState1 = 0;             /*按位表示第一个165DI状态(0断开 1闭合)*/
static u8 sSPI6DevMulPortDiState2 = 0;             /*按位表示第二个165DI状态(0断开 1闭合)*/
static u8 sSPI6DevMulPortDiState3 = 0;             /*按位表示第三个165DI状态(0闭合 1断开)*/

/*=============================================================================================
** @Name      : void SetMuiPortDiData(e_165SPI_CHANNEL Channel , u8 Num165,u8 Data)
** @Input     : Channel:SPI通道  Num165:级联的165序号 Data：DI数据
** @Output    : void
** @Function  : 获取SN74HC165
**===========================================================================================*/
void SetMuiPortDiData(e_165SPI_CHANNEL Channel , u8 Num165, u8 Data)
{
    if(gDevSampInfo_14 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gDevSampInfo_14 null\r\n", __func__);
        return;
    }
    if(Channel > e_165SPINum)
        return;
    if((Num165 < 1) || (Num165 > 3))
        return;
    #if(BOARDX1_TYPE == 0)
    if(Channel == e_SPI3_165)
    {
        if(Num165 == 1)
            sSPI3DevMulPortDiState1 = Data;
        else if(Num165 == 2)
            sSPI3DevMulPortDiState2 = Data;
    }
    else if(Channel == e_SPI6_165)
    {
        if(Num165 == 1)
            sSPI6DevMulPortDiState1 = Data;
        else if(Num165 == 2)
            sSPI6DevMulPortDiState2 = Data;
        else if(Num165 == 3)
            sSPI6DevMulPortDiState3 = Data;
    }
    #else
    if(Channel == e_SPI6_165)
    {
        if(Num165 == 1)
            sSPI6DevMulPortDiState1 = Data;
        else if(Num165 == 2)
            sSPI6DevMulPortDiState2 = Data;
        else if(Num165 == 3)
            sSPI6DevMulPortDiState3 = Data;

        gDevSampInfo_14[eDevSamp_MuxGpioIn1] = ((sSPI6DevMulPortDiState1) | (sSPI6DevMulPortDiState2 << 8));
        gDevSampInfo_14[eDevSamp_MuxGpioIn2] = sSPI6DevMulPortDiState3;
    }
    #endif
}
/*=============================================================================================
** @Name      : void DEVGPIOInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : GPIO端口初始化
** @The notes : GPIO初始化已在board_port_init()函数初始化，如果PINMUX配置没问题的话可以无需调用此函数
**===========================================================================================*/
void DEVGPIOInit(void)
{
    g_bmu02_gpio_dev = &g_gpio_secure;

    if(gDevSampInfo_14 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gDevSampInfo_14 null\r\n", __func__);
        return;
    }
    #if(BOARDX1_TYPE == 0)

    gpio_config(g_bmu02_gpio_dev, 1, GPIO_OUT);     /*SPI4 595使能引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 1, 0);     

    gpio_config(g_bmu02_gpio_dev, 125, GPIO_OUT);    /*SPI4 595RCLK引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 125, 0);      



    gpio_config(g_bmu02_gpio_dev, 149, GPIO_OUT);    /* SPI6 165 CE引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 149, 0);     

    gpio_config(g_bmu02_gpio_dev, 148, GPIO_OUT);    /*SPI6 165 PL引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 148, 0);      


    gpio_config(g_bmu02_gpio_dev, 131, GPIO_OUT);     /*SPI8 595使能引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 131, 0);     

    gpio_config(g_bmu02_gpio_dev, 124, GPIO_OUT);    /*SPI8 595RCLK引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 124, 0);   


    gpio_config(g_bmu02_gpio_dev, 9, GPIO_OUT);    /* SPI3 165 CE引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 9, 0);     

    gpio_config(g_bmu02_gpio_dev, 8, GPIO_OUT);    /*SPI3 165 PL引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 8, 0); 



    gpio_config(g_bmu02_gpio_dev, 119, GPIO_OUT);     /*运行指示灯配置为输出模式*/    
    gpio_set(g_bmu02_gpio_dev, 119, 0); 

    gpio_config(g_bmu02_gpio_dev, 120, GPIO_OUT);     /*告警指示灯配置为输出模式*/   
    gpio_set(g_bmu02_gpio_dev, 120, 0);  



    gpio_config(g_bmu02_gpio_dev, 114, GPIO_IN);     /*POWER_DI配置为输入模式*/  

    #elif(BOARDX1_TYPE == 1)
    gpio_config(g_bmu02_gpio_dev, 1, GPIO_OUT);      /*595使能引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 1, 0);      
    gpio_config(g_bmu02_gpio_dev, 125, GPIO_OUT);    /*595RCLK引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 125, 0);      

    gpio_config(g_bmu02_gpio_dev, 149, GPIO_OUT);    /*165 CE引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 149, 0);     
    gpio_config(g_bmu02_gpio_dev, 148, GPIO_OUT);    /*165 PL引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 148, 0);      

    gpio_config(g_bmu02_gpio_dev, 115, GPIO_OUT);    /*看门狗使能引脚配置为输出模式*/
    gpio_set(g_bmu02_gpio_dev, 115, 1);
    
    gpio_config(g_bmu02_gpio_dev, 119, GPIO_OUT);     /*运行指示灯配置为输出模式*/    
    gpio_set(g_bmu02_gpio_dev, 119, 0); 

    gpio_config(g_bmu02_gpio_dev, 120, GPIO_OUT);     /*告警指示灯配置为输出模式*/   
    gpio_set(g_bmu02_gpio_dev, 120, 0);  

    gpio_config(g_bmu02_gpio_dev, 131, GPIO_OUT);     /*4051_S0配置为输出模式*/   
    gpio_config(g_bmu02_gpio_dev, 128, GPIO_OUT);     /*4051_S1配置为输出模式*/   
    gpio_config(g_bmu02_gpio_dev, 129, GPIO_OUT);     /*4051_S2配置为输出模式*/   
    gpio_config(g_bmu02_gpio_dev, 113, GPIO_OUT);     /*WDT_WDI配置为输出模式*/

    gpio_config(g_bmu02_gpio_dev, 114, GPIO_IN);     /*POWER_DI配置为输入模式*/  

    gpio_config(g_bmu02_gpio_dev, 140, GPIO_IN);     /*TYPE_C_5V_DI配置为输入模式*/   

    gpio_config(g_bmu02_gpio_dev, 6, GPIO_IN);     /*CAN_2_5V_DI配置为输入模式*/   

    gpio_config(g_bmu02_gpio_dev, 7, GPIO_IN);     /*CAN_4_5V_DI配置为输入模式*/   

    gpio_config(g_bmu02_gpio_dev, 8, GPIO_IN);     /*RS485_2_5V_DI配置为输入模式*/   

    gpio_config(g_bmu02_gpio_dev, 9, GPIO_IN);     /*RS485_4_5V_DI配置为输入模式*/   

    //gpio_config(g_bmu02_gpio_dev, 63, GPIO_IN);     /*VCCP_3V3_DI配置为输入模式*/   
    #endif


    sSPI4DevMulPortIoCtrlState = 0;
    sSPI6DevMulPortDiState1 = 0;
    sSPI6DevMulPortDiState2 = 0;
    sSPI6DevMulPortDiState3 = 0;

    gDevSampInfo_14[eDevSamp_GpioOut] = 0;
    gDevSampInfo_14[eDevSamp_MuxGpioOut] = 0;
    gDevSampInfo_14[eDevSamp_MuxGpioIn1] = 0;
    gDevSampInfo_14[eDevSamp_MuxGpioIn2] = 0;
    #if(BOARDX1_TYPE == 0)
    {
        sSPI8DevMulPortIoCtrlState = 0;        
        sSPI3DevMulPortDiState1 = 0;            
        sSPI3DevMulPortDiState2 = 0;             
    }
    #endif
}

/*=============================================================================================
** @Name      : u8 DEVGPIOGetInPortState(e_INPUTPORT port)
** @Input     : port:输入端口号
** @Output    : 端口状态[0:断开 1:闭合]
** @Function  : 获取GPIO输入端口状态
**===========================================================================================*/
u8 DEVGPIOGetInPortState(e_INPUTPORT port)
{
    u8 state = 0;
    if(gDevSampInfo_14 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gDevSampInfo_14 null\r\n", __func__);
        return (0);
    }
    #if(BOARDX1_TYPE == 0)
    switch(port)
    {
        /*电源回检DI信号*/
        case ePIn_POWER_DI:
        state = gpio_get(g_bmu02_gpio_dev, 114);
        break;
        //以下是SPI6移位寄存器IO复用
        /*第一路低边开关状态信号，0表示故障*/
        case ePIn_NK1_STATUS:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D0);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*第二路低边开关状态信号，0表示故障*/
        case ePIn_NK2_STATUS:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D1);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

         /*第三路低边开关状态信号，0表示故障*/
         case ePIn_NK3_STATUS:
         state = BitGet(sSPI6DevMulPortDiState1, ePMul_D2);
         //低电平(0)代表闭合(用户根据实际修改)
         if(0 == state)
         {
              state = ePSTATE_OFF;
         }
         else if(1 == state)
         {
             state = ePSTATE_ON;
         }
         break;
         
        /*第四路低边开关状态信号，0表示故障*/
        case ePIn_NK4_STATUS:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D3);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*DO板连接状态*/
        case ePIn_PWDO_BOARD:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D5);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*DI板连接状态*/
        case ePIn_DI_BOARD:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D6);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*DI9，无源DI*/
        case ePIn_DI9:
        state = BitGet(sSPI6DevMulPortDiState2, ePMul_D1);
        //低电平(0)代表闭合(常闭急停按钮导通，未按下)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI10，无源DI*/
        case ePIn_DI10:
        state = BitGet(sSPI6DevMulPortDiState2, ePMul_D2);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI1，有源DI*/
        case ePIn_DI1:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D0);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI2，有源DI*/
        case ePIn_DI2:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D1);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI3，有源DI*/
        case ePIn_DI3:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D2);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI4，有源DI*/
        case ePIn_DI4:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D3);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI5，有源DI*/
        case ePIn_DI5:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D4);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI6，有源DI*/
        case ePIn_DI6:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D5);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI7，有源DI*/
        case ePIn_DI7:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D6);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI8*/
        case ePIn_DI8:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D7);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        //以下是SPI3移位寄存器复用IO
        /*ePIn_485_6_5V_DI*/
        case ePIn_485_6_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D1);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_485_4_5V_DI*/
        case ePIn_485_4_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D2);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_485_2_5V_DI*/
        case ePIn_485_2_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D3);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_CAN_2_5V_DI*/
        case ePIn_CAN_2_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D4);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_CAN_4_5V_DI*/
        case ePIn_CAN_4_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D5);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_232_12_5V_DI*/
        case ePIn_232_12_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D6);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        case ePIn_232_34_5V_DI:
        state = BitGet(sSPI3DevMulPortDiState1, ePMul_D7);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_VCCP_3V3_DI*/
        case ePIn_VCCP_3V3_DI:
        state = BitGet(sSPI3DevMulPortDiState2, ePMul_D1);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_TYPE_C_5V_DI*/
        case ePIn_TYPE_C_5V_DI:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D3);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_DO_BOARD_DI*/
        case ePIn_DO_BOARD_DI:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D4);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*ePIn_LVDS_BOARD_DI*/
        case ePIn_LVDS_BOARD_DI:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D5);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        default:
        state = 0xff;
        break;
    }
    #elif(BOARDX1_TYPE == 1)
    switch(port)
    {
        /*电源回检DI信号*/
        case ePIn_POWER_DI:
        state = gpio_get(g_bmu02_gpio_dev, 114);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioIn], 1);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioIn], 1);
        }
        break;

        /*TYPE_C_5V_DI信号*/
        case ePIn_TYPE_C_5V_DI:
        state = gpio_get(g_bmu02_gpio_dev, 140);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioIn], 2);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioIn], 2);
        }
        break;

        /*CAN 2.5VDI信号*/
        case ePIn_CAN_2_5V_DI:
        state = gpio_get(g_bmu02_gpio_dev, 6);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioIn], 3);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioIn], 3);
        }
        break;

        /*CAN 4.5VDI信号*/
        case ePIn_CAN_4_5V_DI:
        state = gpio_get(g_bmu02_gpio_dev, 7);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioIn], 4);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioIn], 4);
        }
        break;

        /*RS485 2.5VDI信号*/
        case ePIn_RS485_2_5V_DI:
        state = gpio_get(g_bmu02_gpio_dev, 8);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioIn], 5);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioIn], 5);
        }
        break;

        /*RS485 4.5VDI信号*/
        case ePIn_RS485_4_5V_DI:
        state = gpio_get(g_bmu02_gpio_dev, 9);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioIn], 6);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioIn], 6);
        }
        break;

        /*VCCP 3.3VDI信号*/
        //case ePIn_VCCP_3_3V_DI:
        //state = gpio_get(g_bmu02_gpio_dev, 63);
        //break;

        /*第一路低边开关状态信号，0表示故障*/
        case ePIn_NK1_STATUS:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D0);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*第二路低边开关状态信号，0表示故障*/
        case ePIn_NK2_STATUS:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D1);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

         /*第三路低边开关状态信号，0表示故障*/
         case ePIn_NK3_STATUS:
         state = BitGet(sSPI6DevMulPortDiState1, ePMul_D2);
         //低电平(0)代表闭合(用户根据实际修改)
         if(0 == state)
         {
              state = ePSTATE_OFF;
         }
         else if(1 == state)
         {
             state = ePSTATE_ON;
         }
         break;
         
        /*第四路低边开关状态信号，0表示故障*/
        case ePIn_NK4_STATUS:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D3);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*DO板连接状态*/
        case ePIn_PWDO_BOARD:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D5);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*DI板连接状态*/
        case ePIn_DI_BOARD:
        state = BitGet(sSPI6DevMulPortDiState1, ePMul_D6);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
             state = ePSTATE_OFF;
        }
        else if(1 == state)
        {
            state = ePSTATE_ON;
        }
        break;

        /*DI9，无源DI*/
        case ePIn_DI9:
        state = BitGet(sSPI6DevMulPortDiState2, ePMul_D1);
        //低电平(0)代表闭合(常闭急停按钮导通，未按下)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI10，无源DI*/
        case ePIn_DI10:
        state = BitGet(sSPI6DevMulPortDiState2, ePMul_D2);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI1，有源DI*/
        case ePIn_DI1:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D0);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI2，有源DI*/
        case ePIn_DI2:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D1);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI3，有源DI*/
        case ePIn_DI3:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D2);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI4，有源DI*/
        case ePIn_DI4:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D3);
        //低电平(0)代表闭合(接触器反馈: 断开=高电平1)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI5，有源DI*/
        case ePIn_DI5:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D4);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI6，有源DI*/
        case ePIn_DI6:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D5);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI7，有源DI*/
        case ePIn_DI7:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D6);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        /*DI8*/
        case ePIn_DI8:
        state = BitGet(sSPI6DevMulPortDiState3, ePMul_D7);
        //低电平(0)代表闭合(用户根据实际修改)
        if(0 == state)
        {
            state = ePSTATE_ON;
        }
        else if(1 == state)
        {
            state = ePSTATE_OFF;
        }
        break;

        default:
        state = 0xff;
        break;
    }
    #endif
    return(state);
}

/*=============================================================================================
** @Name      : void DEVGPIOSetOutputState(e_OutputPort port, e_GPIOState state)
** @Input     : port:输出端口号 state:输出状态[0:断开 1:闭合]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 设置GPIO输出端口状态
**===========================================================================================*/
void DEVGPIOSetPortState(e_OutputPort port, e_GPIOState state)
{
    u8 result = TRUE;
    e_IOState ioSign = eIOLOW;

    #if(BOARDX1_TYPE == 0)
    switch(port)
    {
        /*SPI4 595 EN引脚*/
        case ePOut_SPI4595_EN:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 1, ioSign);      
        break;

        /*SPI4 595_RCLK引脚*/
        case ePOut_SPI4595_RCLK:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 125, ioSign);  
        break;

        /*SPI8 595 EN引脚*/
        case ePOut_SPI8595_EN:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 131, ioSign);      
        break;

       /*SPI8 595_RCLK引脚*/
        case ePOut_SPI8595_RCLK:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 124, ioSign);  
        break;

       /*SPI6165 CE引脚*/
        case ePOut_SPI6165_CE:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 149, ioSign);  
        break;

       /*SPI6165 PL引脚*/
        case ePOut_SPI6165_PL:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 148, ioSign);  
        break;

        /*SPI3165 CE引脚*/
        case ePOut_SPI3165_CE:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 9, ioSign);  
        break;

        /*SPI3165 PL引脚*/
        case ePOut_SPI3165_PL:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 8, ioSign);  
        break;

        /*运行指示灯引脚*/
        case ePOut_RUN_LED:
        /*低电平(0)代表闭合(用户根据实际修改)*/
        if(ePSTATE_ON == state)
        {
            ioSign = eIOHIG;
        }
        else if(ePSTATE_OFF == state)
        {
            ioSign = eIOLOW;
        }
        gpio_set(g_bmu02_gpio_dev, 119, ioSign);  
        break;

        /*告警指示灯引脚*/
        case ePOut_ALM_LED:
        /*低电平(0)代表闭合(用户根据实际修改)*/
        if(ePSTATE_ON == state)
        {
            ioSign = eIOHIG;
        }
        else if(ePSTATE_OFF == state)
        {
            ioSign = eIOLOW;
        }
        gpio_set(g_bmu02_gpio_dev, 120, ioSign);  
        break;

        /*第一路高边驱动芯片通道0输入,复用引脚*/
        case ePOut_PK1_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D0);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D0);
        }
        break;

        /*第一个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK12_DEN:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D1);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D1);
        }
        break;

        /*第一个高边驱动芯片诊断通道选择*/
        case ePOut_PK12_DSEL:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D2);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D2);
        }
        break;

        /*第一路高边驱动芯片通道1输入*/
        case ePOut_PK2_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D3);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D3);
        }
        break;

        /*第三路高边驱动芯片通道0输入*/
        case ePOut_PK3_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D4);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D4);
        }
        break;

       /*第二个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK34_DEN:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D5);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D5);
        }
        break;

        /*第二个高边驱动芯片诊断通道选择*/
        case ePOut_PK34_DSEL:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D6);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D6);
        }
        break;

        /*第四路高边驱动芯片通道1输入*/
        case ePOut_PK4_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D7);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D7);
        }
        break;

        /*第一路低边驱动芯片输入*/
        case ePOut_NK1_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D8);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D8);
        }
        break;

        /*第二路低边驱动芯片输入*/
        case ePOut_NK2_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D9);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D9);
        }
        break;

        /*第三路低边驱动芯片输入*/
        case ePOut_NK3_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D10);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D10);
        }
        break;

        /*第四路低边驱动芯片输入*/
        case ePOut_NK4_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D11);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D11);
        }
        break;

        /*DO1*/
        case ePOut_DO1:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D0);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D0);
        }
        break;

        /*DO2*/
        case ePOut_DO2:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D1);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D1);
        }
        break;

        /*DO3*/
        case ePOut_DO3:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D2);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D2);
        }
        break;

        /*DO4*/
        case ePOut_DO4:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D3);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D3);
        }
        break;

        /*DO5*/
        case ePOut_DO5:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D4);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D4);
        }
        break;

        /*DO6*/
        case ePOut_DO6:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D5);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D5);
        }
        break;

        /*DP复位*/
        case ePOut_DP_RST:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D6);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D6);
        }
        break;

        /*LVDS电源使能*/
        case ePOut_LVDS_PW_EN:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D8);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D8);
        }
        break;

        /*4G电源使能*/
        case ePOut_4G_PW_EN:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D9);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D9);
        }
        break;

        /*4G复位*/
        case ePOut_4G_RST:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D10);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D10);
        }
        break;

        /*HDMI复位*/
        case ePOut_HDMI_RST:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D11);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D11);
        }
        break;

        /*USB复位*/
        case ePOut_USB_RST:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D12);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D12);
        }
        break;

        /*4051_S0片选*/
        case ePOut_4051_S0:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D13);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D13);
        }
        break;

        /*4051_S1片选*/
        case ePOut_4051_S1:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D14);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D14);
        }
        break;

        /*4051_S2片选*/
        case ePOut_4051_S2:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI8DevMulPortIoCtrlState, ePMul_D15);
        }
        else
        {
            BitClr(sSPI8DevMulPortIoCtrlState, ePMul_D15);
        }
        break;

        default:
        break;
    }
    #elif(BOARDX1_TYPE == 1)
    switch(port)
    {
        /*595 EN引脚*/
        case ePOut_SPI4595_EN:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 1, ioSign); 
	    if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 0);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 0);
        }
        break;

        /*595_RCLK引脚*/
        case ePOut_SPI4595_RCLK:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 125, ioSign); 
	    if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 1);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 1);
        }
        break;

        /*165 CE引脚*/
        case ePOut_SPI6165_CE:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 149, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 2);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 2);
        }
        break;

        /*165 PL引脚*/
        case ePOut_SPI6165_PL:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 148, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 3);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 3);
        }
        break;

        /*看门狗使能引脚*/
        case ePOut_WDT_EN:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 115, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 4);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 4);
        }
        break;
        case ePOut_WDT_WDI:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 113, ioSign);
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 10);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 10);
        }
        break;

        /*运行指示灯引脚*/
        case ePOut_RUN_LED:
        /*低电平(0)代表闭合(用户根据实际修改)*/
        if(ePSTATE_ON == state)
        {
            ioSign = eIOHIG;
        }
        else if(ePSTATE_OFF == state)
        {
            ioSign = eIOLOW;
        }
        gpio_set(g_bmu02_gpio_dev, 119, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 5);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 5);
        }
        break;

        /*告警指示灯引脚*/
        case ePOut_ALM_LED:
        /*低电平(0)代表闭合(用户根据实际修改)*/
        if(ePSTATE_ON == state)
        {
            ioSign = eIOHIG;
        }
        else if(ePSTATE_OFF == state)
        {
            ioSign = eIOLOW;
        }
        gpio_set(g_bmu02_gpio_dev, 120, ioSign);  
	    if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 6);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 6);
        }
        break;

        /*4051 S0引脚*/
        case ePOut_4051_S0:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 131, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 7);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 7);
        }
        break;

         /*4051 S1引脚*/
        case ePOut_4051_S1:
        ioSign = (e_IOState)state;
        gpio_set(g_bmu02_gpio_dev, 128, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 8);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 8);
        }
        break;

        
         /*4051 S2引脚*/
         case ePOut_4051_S2:
         ioSign = (e_IOState)state;
         gpio_set(g_bmu02_gpio_dev, 129, ioSign);  
        if(state == 1)
        {
            BitSet16(gDevSampInfo_14[eDevSamp_GpioOut], 9);
        }
        else
        {
            BitClr16(gDevSampInfo_14[eDevSamp_GpioOut], 9);
        }
         break;

        /*第一路高边驱动芯片通道0输入,复用引脚*/
        case ePOut_PK1_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D0);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D0);
        }
        break;

        /*第一个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK12_DEN:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D1);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D1);
        }
        break;

        /*第一个高边驱动芯片诊断通道选择*/
        case ePOut_PK12_DSEL:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D2);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D2);
        }
        break;

        /*第一路高边驱动芯片通道1输入*/
        case ePOut_PK2_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D3);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D3);
        }
        break;

        /*第三路高边驱动芯片通道0输入*/
        case ePOut_PK3_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D4);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D4);
        }
        break;

       /*第二个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK34_DEN:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D5);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D5);
        }
        break;

        /*第二个高边驱动芯片诊断通道选择*/
        case ePOut_PK34_DSEL:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D6);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D6);
        }
        break;

        /*第四路高边驱动芯片通道1输入*/
        case ePOut_PK4_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D7);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D7);
        }
        break;

        /*第一路低边驱动芯片输入*/
        case ePOut_NK1_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D8);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D8);
        }
        break;

        /*第二路低边驱动芯片输入*/
        case ePOut_NK2_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D9);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D9);
        }
        break;

        /*第三路低边驱动芯片输入*/
        case ePOut_NK3_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D10);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D10);
        }
        break;

        /*第四路低边驱动芯片输入*/
        case ePOut_NK4_DRV:
        ioSign = (e_IOState)state;
        if(eIOHIG == ioSign)
        {
            BitSet(sSPI4DevMulPortIoCtrlState, ePMul_D11);
        }
        else
        {
            BitClr(sSPI4DevMulPortIoCtrlState, ePMul_D11);
        }
        break;

        default:
        break;
    }
    #endif
}

/*=============================================================================================
** @Name      : u8 DEVGPIOGetOutPortState(e_OutputPort port)
** @Input     : port:输出端口号
** @Output    : 端口状态[0:断开 1:闭合]
** @Function  : 获取GPIO输出端口状态
** @The notes : D9 设置为输出的GPIO无法获取输出电平，这里只能获取扩展IO输出状态
**===========================================================================================*/
u8 DEVGPIOGetOutPortState(e_OutputPort port)
{
    u8 state = 0;
    #if(BOARDX1_TYPE == 0)
    switch(port)
    {
        //以下为595移位寄存器复用IO
        /*第一路高边驱动芯片通道0输入*/
        case ePOut_PK1_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D0);
        break;

        /*第一个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK12_DEN:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D1);
        break;

        /*第一个高边驱动芯片诊断通道选择*/
        case ePOut_PK12_DSEL:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D2);
        break;

        /*第一路高边驱动芯片通道1输入*/
        case ePOut_PK2_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D3);
        break;

        /*第三路高边驱动芯片通道0输入*/
        case ePOut_PK3_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D4);
        break;

        /*第二个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK34_DEN:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D5);
        break;

       /*第二个高边驱动芯片诊断通道选择*/
        case ePOut_PK34_DSEL:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D6);
        break;

        /*第四路高边驱动芯片通道1输入*/
        case ePOut_PK4_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D7);
        break;

        /*第一路低边驱动芯片输入*/
        case ePOut_NK1_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D8);
        break;

        /*第二路低边驱动芯片输入*/
        case ePOut_NK2_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D9);
        break;

        /*第三路低边驱动芯片输入*/
        case ePOut_NK3_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D10);
        break;

        /*第四路低边驱动芯片输入*/
        case ePOut_NK4_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D11);
        break;


        /*DO1*/
        case ePOut_DO1:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D0);
        break;

        /*DO2*/
        case ePOut_DO2:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D1);
        break;

        /*DO3*/
        case ePOut_DO3:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D2);
        break;

        /*DO4*/
        case ePOut_DO4:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D3);
        break;

        /*DO5*/
        case ePOut_DO5:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D4);
        break;

        /*DO6*/
        case ePOut_DO6:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D5);
        break;

        /*DP复位*/
        case ePOut_DP_RST:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D6);
        break;

        /*LVDS电源使能*/
        case ePOut_LVDS_PW_EN:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D8);
        break;

        /*4G电源使能*/
        case ePOut_4G_PW_EN:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D9);
        break;

        /*4G复位*/
        case ePOut_4G_RST:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D10);
        break;

        /*HDMI复位*/
        case ePOut_HDMI_RST:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D11);
        break;

        /*USB复位*/
        case ePOut_USB_RST:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D12);
        break;

        /*4051片选S0*/
        case ePOut_4051_S0:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D13);
        break;

        /*4051片选S1*/
        case ePOut_4051_S1:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D14);
        break;

        /*4051片选S2*/
        case ePOut_4051_S2:
        state = BitGet(sSPI8DevMulPortIoCtrlState, ePMul_D15);
        break;
        default:
        state = 0xff;
        break;
    }
    #elif(BOARDX1_TYPE == 1)
    switch(port)
    {
        /*595 EN引脚*/
        case ePOut_SPI4595_EN:
        state = gpio_get(g_bmu02_gpio_dev, 1);
        break;

        /*595_RCLK引脚*/
        case ePOut_SPI4595_RCLK:
        state = gpio_get(g_bmu02_gpio_dev, 125);
        break;

        /*165 CE引脚*/
        case ePOut_SPI6165_CE:
        state = gpio_get(g_bmu02_gpio_dev, 149);
        break;

        /*165 PL引脚*/
        case ePOut_SPI6165_PL:
        state = gpio_get(g_bmu02_gpio_dev, 148); 
        break;

        /*看门狗使能引脚*/
        case ePOut_WDT_EN:
        state = gpio_get(g_bmu02_gpio_dev, 115);
        break;
        case ePOut_WDT_WDI:
        state = gpio_get(g_bmu02_gpio_dev, 113);
        break;

        /*运行指示灯引脚*/
        case ePOut_RUN_LED:
        if(0 == gpio_get(g_bmu02_gpio_dev, 119))
        {
            state = eIOLOW;
        }
        else
        {
            state = eIOHIG;
        }
        break;

        /*告警指示灯引脚*/
        case ePOut_ALM_LED:
        if(0 == gpio_get(g_bmu02_gpio_dev, 120))
        {
            state = eIOLOW;
        }
        else
        {
            state = eIOHIG;
        }
        break;

        /*4051 S0引脚*/
        case ePOut_4051_S0:
        state = gpio_get(g_bmu02_gpio_dev, 141);
        break;

         /*4051 S1引脚*/
        case ePOut_4051_S1:
        state = gpio_get(g_bmu02_gpio_dev, 138);
        break;

        
         /*4051 S2引脚*/
         case ePOut_4051_S2:
         state = gpio_get(g_bmu02_gpio_dev, 139);
         break;

        //以下为595移位寄存器复用IO
        /*第一路高边驱动芯片通道0输入*/
        case ePOut_PK1_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D0);
        break;

        /*第一个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK12_DEN:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D1);
        break;

        /*第一个高边驱动芯片诊断通道选择*/
        case ePOut_PK12_DSEL:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D2);
        break;

        /*第一路高边驱动芯片通道1输入*/
        case ePOut_PK2_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D3);
        break;

        /*第三路高边驱动芯片通道0输入*/
        case ePOut_PK3_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D4);
        break;

        /*第二个高边驱动芯片诊断功能使能引脚*/
        case ePOut_PK34_DEN:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D5);
        break;

       /*第二个高边驱动芯片诊断通道选择*/
        case ePOut_PK34_DSEL:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D6);
        break;

        /*第四路高边驱动芯片通道1输入*/
        case ePOut_PK4_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D7);
        break;

        /*第一路低边驱动芯片输入*/
        case ePOut_NK1_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D8);
        break;

        /*第二路低边驱动芯片输入*/
        case ePOut_NK2_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D9);
        break;

        /*第三路低边驱动芯片输入*/
        case ePOut_NK3_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D10);
        break;

        /*第四路低边驱动芯片输入*/
        case ePOut_NK4_DRV:
        state = BitGet(sSPI4DevMulPortIoCtrlState, ePMul_D11);
        break;

        default:
        state = 0xff;
        break;
    }
    #endif
    return (state);
}

/*=============================================================================================
** @Name      : void DEVGPIOMulPortManagTask(void *argument)
** @Input     : void
** @Output    : void
** @Function  : GPIO复用端口管控任务
**===========================================================================================*/
/*=============================================================================================
** @Name      : void DEVGPIOMulPortManagTask(void *argument)
** @Input     : void
** @Output    : void
** @Function  : GPIO复用端口管控任务
**===========================================================================================*/
void DEVGPIOMulPortManagTask(void *argument)
{
    //static u16 LsatDoCtrlFLag = 0xFFFF;
    if(gDevSampInfo_14 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gDevSampInfo_14 null\r\n", __func__);
        return;
    }
    while(1)
    {
        uint32_t start_tick = osKernelGetTickCount();
        SN74HC165GetDataAsync();
        //if(LsatDoCtrlFLag != sSPI4DevMulPortIoCtrlState)
        {
            CD74HC595SendDataAsync(e_SPI4_595,sSPI4DevMulPortIoCtrlState);
            //LsatDoCtrlFLag = sSPI4DevMulPortIoCtrlState;
        }

        gDevSampInfo_14[eDevSamp_MuxGpioOut] = sSPI4DevMulPortIoCtrlState;
        #if(BOARDX1_TYPE == 0)
        CD74HC595SendDataAsync(e_SPI8_595,sSPI8DevMulPortIoCtrlState);
        #endif
        osDelay(5);
        uint32_t execution_time = osKernelGetTickCount() - start_tick;
        MONITOR_RECORD_ITER(eTaskMon_MulPortManag, execution_time);
    }
}
#if(0)
//如下是为了验证GPIO功能，使用CLI命令验证，调试时可以打开。
static int devgpio_test(int argc, char *argv[])
{
        if (argc == 0) 
        {
            printf("devgpio test para error\n");
            return CLI_FALSE;
        }
        if(!strcmp(argv[0], "spi4")) 
        {
            sSPI4DevMulPortIoCtrlState = strtoul(argv[1], NULL, 0);
            CD74HC595SendData(e_SPI4_595,sSPI4DevMulPortIoCtrlState);
            printf("sSPI4DevMulPortIoCtrlState = %d\r\n",sSPI4DevMulPortIoCtrlState);
        }
        else if(!strcmp(argv[0], "spi6")) 
        {
            SN74HC165GetData();
            printf("sSPI6DevMulPortDiState1 = %d,sSPI6DevMulPortDiState2= %d,sSPI6DevMulPortDiState3 = %d\r\n",sSPI6DevMulPortDiState1,sSPI6DevMulPortDiState2,sSPI6DevMulPortDiState3);
        }
        return CLI_FALSE;


}
CLI_CMD("devgpio_test", "devgpio_test", devgpio_test);
#endif
/*=============================================================================================
** @Name      : u8 DEVGPIOGetSPI6DiState3(void)
** @Input     : void
** @Output    : SPI6第三片165DI状态(按位表示D16-D23)
** @Function  : 获取SPI6第三片165的数字输入状态
** @The notes : 供dbg_test stat读取DI1-4接触器反馈
**=============================================================================================*/
u8 DEVGPIOGetSPI6DiState3(void)
{
    return sSPI6DevMulPortDiState3;
}
