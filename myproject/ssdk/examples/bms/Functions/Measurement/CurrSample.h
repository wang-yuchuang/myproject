/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrSample.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 电流采样模块
** @Instructions : 同时使能霍尔和分流器时以分流器为主,霍尔作为参考
**===========================================================================================*/
#ifndef _CURRSAMPLE_H_
#define _CURRSAMPLE_H_

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
//#include "HigAFESamp.h"

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
//采样参数配置
#define CURR_SAMP_MAIN_EN   0                                           /*使用主板BMU电流采样使能 0:高压模块HVCU 1:主板BMU*/
#define CURR_AUT_ZERO_EN    1                                           /*自动校零使能 0:禁止 1:使能*/
#define CURR_INT_ADD_EN     1                                           /*中断电流积分使能 0:定时任务积分 1:定时中断积分*/


#define CURR_HALL5V_MINV    (450)                                       /*霍尔5V基准采样允许最低值 0.01V*/
#define CURR_HALL_MIN_AD    (249)                                       /*使用霍尔采时最小AD值(0.3V)*/
#define CURR_HALL_MAX_AD    (3890)                                      /*使用霍尔采时最大AD值(4.7V)*/
#define CURR_SHUNT_MIN_V    (-9000)                                     /*使用分流器采时最小AD值(-90mV) 0.01mV*/
#define CURR_SHUNT_MAX_V    (9000)                                      /*使用分流器采时最大AD值(90mV) 0.01mV*/

/*电流采样通道*/
typedef enum
{
    eCADC_Hall = 0,                 /*内部ADC通道霍尔电流采样*/
    eCAFE_Shunt = 1,                /*外部AFE通道分流器电流采样*/

    eCCHAN_Num                      /*电流采样通道个数*/
}e_CSampChan;

/*电流采样异常ID*/
typedef enum
{
    eCExp_HalErr = 0,               /*电流采样霍尔过载回检异常*/
    eCExp_HalOff = 1,               /*电流采样霍尔供电异常*/
    eCExp_HADErr = 2,               /*电流采样霍尔AD值异常*/
    eCExp_SADErr = 3,               /*电流采样分流器AD值异常*/
    eCExp_HSDiff = 4,               /*电流采样霍尔分流器采样值不一致(均使能时)*/
    eCExp_AfeErr = 5,               /*电流采样分流器模拟前端异常*/
    eCExp_ZroErr = 6,               /*电流采样校零异常*/
    eCExp_SefErr = 7,               /*电流采样自检异常*/


    eCExp_IDLen                     /*电流采样异常ID长度*/
}e_CSampExp_ID;
/*=============================================================================================
** @Name      : void CurrSampleInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电流信息采样初始化
** @The notes :
**===========================================================================================*/
void CurrSampleInit(void);
/*=============================================================================================
** @Name      : void ISRCurrCapAddTask(void)
** @Input     : void
** @Output    : void
** @Function  : 定时中断电流积分任务
** @The notes : 必须1ms定时中断调用
**===========================================================================================*/
void ISRCurrCapAddTask(void);
/*=============================================================================================
** @Name      : void ClrCurrZeroFinFlagAPI(u8 clunum)
** @Input     : clunum: 簇编号
** @Output    : void
** @Function  : 清除电流零点校零标志
** @The notes : 重新校零
**===========================================================================================*/
void ClrCurrZeroFinFlagAPI(u8 clunum);
/*=============================================================================================
** @Name      : void CalcSampleADToCurr(u8 clunum, s32 currAD)
** @Input     :clunum：簇编号  currAD:电流AD
** @Output    : void
** @Function  : 将采样AD值转化为电流值
** @The notes : 高压模块采电流时输入为电流值0.01A
**===========================================================================================*/
void CalcSampleADToCurr(u8 clunum, s32 currAD);

/*=============================================================================================
** @Name      : void SetCurrSampExpFlagAPI(u8 clunum, u8 id, u8 state, s16 currAD)
** @Input     : clunum: 簇编号 id:电流采样异常ID(e_CSampExp_ID: 0:霍尔过载回检 1:霍尔供电 2:霍尔AD值 3:分流器AD值 4:霍尔/分流器不一致 5:分流器模拟前端 6:校零 7:自检) state:设置状态[0:正常 1:异常] currAD:读取到的AD值
** @Output    : void
** @Function  : 设置电流采样异常标志
** @The notes :
**===========================================================================================*/
void SetCurrSampExpFlagAPI(u8 clunum, u8 id, u8 state, s16 currAD);
/* 清除由IMIU4告警报文驱动的电流采样异常位；本板AFE/校零异常位保持不变。 */
void ClrRemoteCurrSampExpFlagAPI(u8 clunum);
/*=============================================================================================
** @Name      : u8 SampGetCurrZeroFinFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电流采样零飘校准完成标志[0未校准 1校准成功 2校准失败]
** @Function  : 获取电流采样零飘校准完成标志
** @The notes :
**===========================================================================================*/
u8 SampGetCurrZeroFinFlagAPI(u8 clunum);
/*=============================================================================================
** @Name      : u8 SampGetCurrSampExpFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 按位表示采样异常标志(同e_CSampExp_ID: bit0:霍尔过载回检 bit1:霍尔供电 bit2:霍尔AD值 bit3:分流器AD值 bit4:霍尔/分流器不一致 bit5:分流器模拟前端 bit6:校零 bit7:自检)
** @Function  : 获取指定采样通道电流采样异常标志
** @The notes :
**===========================================================================================*/
u8 SampGetCurrSampExpFlagAPI(u8 clunum);
/*=============================================================================================
** @Name      : u8 SampGetLowVSampExpFlagAPI(void)
** @Input     : void
** @Output    : 低压采样异常标志[0:正常 1:异常]
** @Function  : 获取硬件板低压侧采样异常标志
** @The notes : 836由BMU本板DEVADC提供ADC启动/原始值有效性；电源轨容差门限待硬件确认。
**===========================================================================================*/
u8 SampGetLowVSampExpFlagAPI(void);
#endif
