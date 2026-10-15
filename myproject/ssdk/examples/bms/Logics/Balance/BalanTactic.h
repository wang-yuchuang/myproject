#ifndef __BALANTACTIC_H__
#define __BALANTACTIC_H__

#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"

extern s32 (*gGBalanSubVolt)[GROUP_MAX_CELVNUM];
extern u64 (*gGPasBalanReqState)[SLAVE_MAX_NUM];

void BalanTacticInfoInit(void);

#define BALAN_PAS_PRO_TEMP           80        /*被动均衡温度保护温度值 1℃*/

//菊花链通信从控或自研CAN通信从控
#if((0 == BMU_BSU_COM_TYPE) || (0 == BMU_BSU_CAN_TYPE))
#define BALAN_PAS_LIM_TEMP           60        /*被动均衡温度限制开启节数温度值 1℃*/
#define BALAN_PAS_LIM_CELL           8         /*被动均衡温度限制单从控开启均衡节数*/

/*关闭放电均衡低电压默认值定义*/
#if(3 == CELL_CHEM_TYPE)                    /*钛酸锂电芯默认值*/
#define BALAN_DHG_MIN_VOLT          1900    /*放电均衡最小电压 1mV*/

#elif(2 == CELL_CHEM_TYPE)                  /*磷酸铁锂电芯默认值*/
#define BALAN_DHG_MIN_VOLT          3100    /*放电均衡最小电压 1mV*/
#define BALAN_DHG_MAX_VOLT          3400    /*放电均衡最大电压1mV*/

#define Infy_eGHardPara104_BalDhgV   200   /*放电非平台期开启均衡压差 1mv*/
#define Infy_eGHardPara104_BalDhgV_2   50   /*空闲平台期且电压小于3200mv时均衡压差 1mv*/
#define Infy_Plan_eGHardPara104_BalDhgV   20   /*放电平台期开启均衡压差 1mv*/

#elif(1 == CELL_CHEM_TYPE)                  /*三元电芯默认值*/
#define BALAN_DHG_MIN_VOLT          3600    /*放电均衡最小电压 1mV*/

#else                                       /*铅酸电芯*/
#define BALAN_DHG_MIN_VOLT          2500    /*放电均衡最小电压 1mV*/

#endif

/*=================================================================================================
** @Name      : void GroupVoltBalanTacticTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电压均衡策略任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupVoltBalanTacticTask(void);

/*=================================================================================================
** @Name      : void SetGroupVoltPasBalanReqState(u8 pack, u32 cellState)
** @Input     : pack:从控设备节点号 cellState:从控电压被动均衡请求状态:按bit位表示,每bit代表从控内一个电池(0:关闭均衡 :开启被动均衡)
** @Output    : 电池组内各
** @Function  : 设置电池组内各从控电压被动均衡请求状态
** @The notes : 建议只在调试模式调用,否则会与均衡策略计算值冲突
**===============================================================================================*/
void SetGroupVoltPasBalanReqState(u8 ecu, u64 cellState);

/*=================================================================================================
** @Name      : u62 GetGroupVoltPasBalanReqState(u8 pack)
** @Input     : pack:从控设备节点号(即Pack序号)
** @Output    : 电池组内各从控电压被动均衡请求状态:按bit位表示,每bit代表从控内一个电池(0:关闭均衡 :开启被动均衡)
** @Function  : 获取电池组内各从控电压被动均衡请求状态
** @The notes : 包括均衡策略计算值和调试值
**===============================================================================================*/
u64 GetGroupVoltPasBalanReqState(u8 absSlave);

#endif
#endif