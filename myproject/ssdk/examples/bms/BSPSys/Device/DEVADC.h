/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : DEVADC.h
** @Founder      : ZQD808
** @Date         : 2025.11.14
** @Function     : ADC采样通道管理函数模块
** @Instructions : 根据PCB外围设备编写管理函数
**===========================================================================================*/
#ifndef __DEVADC_H__
#define __DEVADC_H__
#include "BSPTypeDef.h"
#include "DEVGPIO.h"
#include "GroupInfo.h"

#define ADC1_MUX0_R1  (2)     //ADC1复用通道0上分压电阻阻值（KΩ）
#define ADC1_MUX0_R2  (10)    //ADC1复用通道0下分压电阻（KΩ）

#define ADC1_MUX1_R1  (2)     //ADC1复用通道1上分压电阻（KΩ）
#define ADC1_MUX1_R2  (10)    //ADC1复用通道1下分压电阻（KΩ）

#define ADC1_MUX2_R1  (10)    //ADC1复用通道2上分压电阻（KΩ）
#define ADC1_MUX2_R2  (10)    //ADC1复用通道2下分压电阻（KΩ）

#define ADC1_MUX3_R1  (10)    //ADC1复用通道3上分压电阻（KΩ）
#define ADC1_MUX3_R2  (10)    //ADC1复用通道3下分压电阻（KΩ）

#define ADC1_MUX4_R1  (10)    //ADC1复用通道4上分压电阻（KΩ）
#define ADC1_MUX4_R2  (10)    //ADC1复用通道4下分压电阻（KΩ）

#define ADC1_MUX5_R1  (10)    //ADC1复用通道5上分压电阻（KΩ）
#define ADC1_MUX5_R2  (5)     //ADC1复用通道5下分压电阻（KΩ）

#define ADC1_MUX6_R1  (10)    //ADC1复用通道6上分压电阻（KΩ）
#define ADC1_MUX6_R2  (5)    //ADC1复用通道6下分压电阻（KΩ）

#define ADC1_MUX7_R1  (10)    //ADC1复用通道7上分压电阻（KΩ）
#define ADC1_MUX7_R2  (1)     //ADC1复用通道7下分压电阻（KΩ）


#if(BOARDX1_TYPE == 0)
/*输出端口序号*/
typedef enum
{
    e_MuxChannel_VCC1V2 = 0,    //复用通道选中VCC1.2V采样通道
    e_MuxChannel_VCC1V8 = 1,    //复用通道选中VCC1.8V采样通道
    e_MuxChannel_VCC2V5 = 2,    //复用通道选中VCC2.5V采样通道
    e_MuxChannel_VCC3V3 = 3,    //复用通道选中VCC3.3V采样通道
    e_MuxChannel_4G3V5  = 4,    //复用通道选中4G3.5V采样通道
    e_MuxChannel_VCC5V  = 5,    //复用通道选中VCC5V采样通道
    e_MuxChannel_VCCH5V = 6,    //复用通道选中VCCH5V采样通道
    e_MuxChannel_VCC12V = 7,    //复用通道选中VCC12V采样通道
    e_Adcchannel2 = 8,          //ADC通道2
    e_Adcchannel3 = 9,          //ADC通道3

    e_AdcchannelNum
}e_MUXCHANNEL;
#elif(BOARDX1_TYPE == 1)
/*输出端口序号*/
typedef enum
{
    e_MuxChannel_VCC1V8 = 0,    //复用通道选中VCC1.8V采样通道
    e_MuxChannel_VCC3V3 = 1,    //复用通道选中VCC3.3V采样通道
    e_MuxChannel_4G3V5  = 2,    //复用通道选中4G3.5V采样通道
    e_MuxChannel_VCC5V  = 3,    //复用通道选中VCC5V采样通道
    e_MuxChannel_VCCH5V = 4,    //复用通道选中VCCH5V采样通道
    e_MuxChannel_VCC12V = 5,    //复用通道选中VCC12V采样通道

     e_AdcchannelNum
}e_MUXCHANNEL;
#endif

/*=============================================================================================
** @Name      : u8 DEVADCInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : ADC端口初始化
** @The notes : 只初始化板级提供的资源
**===========================================================================================*/
u8 DEVADCInit(void);
/*=============================================================================================
** @Name      : void DEVADCSetMuxChannel(e_MUXCHANNEL num)
** @Input     : num :4051复用通道选择
** @Output    : void
** @Function  : 选择复用ADC采集通道
** @The notes : 
**===========================================================================================*/
void DEVADCSetMuxChannel(e_MUXCHANNEL num);
/*=============================================================================================
** @Name      : void DEVADVSetMuxFlag(u8 flag)
** @Input     : flag:4051复用通道标志
** @Output    : void
** @Function  : 设置4051复用通道标志
** @The notes : 
**===========================================================================================*/
void DEVADVSetMuxFlag(u8 flag);
/*=============================================================================================
** @Name      : vu8 DECADVGetMuxFlag(void)
** @Input     : void
** @Output    : s4051MuxFLag
** @Function  : 获取4051复用通道标志
** @The notes : 
**===========================================================================================*/
u8 DECADVGetMuxFlag(void);
/* 获取BMU本板低压ADC链路异常位：启动失败/回调超时/原始值越界，供故障ID181使用。 */
u8 DEVADCGetSampExpFlagAPI(void);
/*=============================================================================================
** @Name      : void LowInfoSampleTask(void *argument)
** @Input     : *argument:形参,任务函数参数需要
** @Output    : void
** @Function  : ADC通道电压电流采样任务
** @The notes : 通过MCU的ADC通道读取
**===========================================================================================*/
void LowInfoSampleTask(void *argument);
/*=============================================================================================
** @Name      : static u16 ChangeSampADtoData(u8 channel, u16 ADdata)
** @Input     : channel:通道号 ADdata:采样AD值
** @Output    : 采样目标值 10mv
** @Function  : 将采样AD值转换为采样目标值
** @The notes : 电压值
**===========================================================================================*/
static u16 ChangeSampADtoData(u8 channel, u16 ADdata);
#endif
