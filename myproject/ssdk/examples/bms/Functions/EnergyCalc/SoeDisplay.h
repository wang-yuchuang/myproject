/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SoeDisplay.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : soe平滑显示模块
** @Instructions :
**===========================================================================================*/
#ifndef _SOEDISPLAY_H_
#define _SOEDISPLAY_H_

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "PublicInfo.h"
#include "BSPTypeDef.h"
/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define SOE_SLOW_SPEED                  5               /*SOE平滑速度(原变化速度减缓倍率)*/

#define SOE_SLOW_VHER_SOE               10000           /*SOE平滑显示高压保护时目标SOE(万分之一)*/
#define SOE_SLOW_VH_SOE                 9980            /*SOE平滑显示高压告警时目标SOE(万分之一)*/
#define SOE_SLOW_VLER_SOE               0               /*SOF平滑显示低压保护时目标SOE(万分之一)*/
#define SOE_SLOW_VL_SOE                 80              /*SOE平滑显示低压告警时目标SOE(万分之一)*/

#define SOE_SLOW_CORR_IDLE_T            6000            /*SOE平滑显示可信度修正静置时间(0.1S)*/

/*不同电芯电压区间和对应SOE取值*/
#if(3 == CELL_CHEM_TYPE)                                /*钛酸锂电芯默认值*/
#define SOE_V_SEC_DN_MIN_V              2050            /*电压平台区间电压下限最小电压(1mV)*/
#define SOE_V_SEC_DN_MAX_V              2050            /*电压平台区间电压下限最大电压(1mV)*/
#define SOE_V_SEC_UP_MIN_V              2500            /*电压平台区间电压上限最小电压(1mV)*/
#define SOE_V_SEC_UP_MAX_V              2500            /*电压平台区间电压上限最大电压(1mV)*/
#define SOE_V_SEC_DN_LES_SOE            1000            /*电压高于下区间至小SOE(万分之一)*/
#define SOE_V_SEC_DN_MOS_SOE            4000            /*电压处于下区间至大SOE(万分之一)*/
#define SOE_V_SEC_UP_LES_SOE            8000            /*电压处于上区间至小SOE(万分之一)*/
#define SOE_V_SEC_UP_MOS_SOE            10000           /*电压低于上区间至大SOE(万分之一)*/

#elif(2 == CELL_CHEM_TYPE)                              /*磷酸铁锂电芯默认值*/
#define SOE_V_SEC_DN_MIN_V              3300            /*电压平台区间电压下限最小电压(1mV)*/
#define SOE_V_SEC_DN_MAX_V              3250            /*电压平台区间电压下限最大电压(1mV)*/
#define SOE_V_SEC_UP_MIN_V              3350            /*电压平台区间电压上限最小电压(1mV)*/
#define SOE_V_SEC_UP_MAX_V              3450            /*电压平台区间电压上限最大电压(1mV)*/
#define SOE_V_SEC_DN_LES_SOE            1000            /*电压高于下区间至小SOE(万分之一)*/
#define SOE_V_SEC_DN_MOS_SOE            3000            /*电压处于下区间至大SOE(万分之一)*/
#define SOE_V_SEC_UP_LES_SOE            6000            /*电压处于上区间至小SOE(万分之一)*/
#define SOE_V_SEC_UP_MOS_SOE            10000           /*电压低于上区间至大SOE(万分之一)*/

#if((3 == PRJ_PARA_NUM_INFO) || (4 == PRJ_PARA_NUM_INFO))/*自研PACK电芯*/
#define SOE_V_CHG_DN_MAX_V              3497            /*充电末端电压区间起始电压最大值(1mV)*/
#define SOE_V_CHG_UP_MAX_V              3535            /*充电末端电压区间结束电压最大值(1mV)*/
#define SOE_V_CHG_DN_LES_SOE            8550            /*充电末端电压区间起始电压至小SOC(万分之一)*/
#define SOE_V_CHG_UP_LES_SOE            9650            /*充电末端电压区间结束电压至小SOC(万分之一)*/

#else                                                   /*CATL-PACK电芯*/
#define SOE_V_CHG_DN_MAX_V              3455            /*充电末端电压区间起始电压最大值(1mV)*/
#define SOE_V_CHG_UP_MAX_V              3495            /*充电末端电压区间结束电压最大值(1mV)*/
#define SOE_V_CHG_DN_LES_SOE            9050            /*充电末端电压区间起始电压至小SOE(万分之一)*/
#define SOE_V_CHG_UP_LES_SOE            9650            /*充电末端电压区间结束电压至小SOE(万分之一)*/
#endif

#define SOE_V_DHG_UP_MAX_V              3070            /*放电末端电压区间起始电压最大值(1mV)*/
#define SOE_V_DHG_DN_MAX_V              2960            /*放电末端电压区间结束电压最大值(1mV)*/
#define SOE_V_DHG_UP_MOS_SOE            1400            /*放电末端电压区间起始电压至大SOE(万分之一)*/
#define SOE_V_DHG_DN_MOS_SOE            600             /*放电末端电压区间结束电压至大SOE(万分之一)*/

#elif(1 == CELL_CHEM_TYPE)                              /*三元电芯默认值*/
#define SOE_V_SEC_DN_MIN_V              3500            /*电压平台区间电压下限最小电压(1mV)*/
#define SOE_V_SEC_DN_MAX_V              3500            /*电压平台区间电压下限最大电压(1mV)*/
#define SOE_V_SEC_UP_MIN_V              4000            /*电压平台区间电压上限最小电压(1mV)*/
#define SOE_V_SEC_UP_MAX_V              4000            /*电压平台区间电压上限最大电压(1mV)*/
#define SOE_V_SEC_DN_LES_SOE            1000            /*电压高于下区间至小SOE(万分之一)*/
#define SOE_V_SEC_DN_MOS_SOE            3000            /*电压处于下区间至大SOE(万分之一)*/
#define SOE_V_SEC_UP_LES_SOE            8000            /*电压处于上区间至小SOE(万分之一)*/
#define SOE_V_SEC_UP_MOS_SOE            10000           /*电压低于上区间至大SOE(万分之一)*/

#else                                                   /*铅酸电芯*/
#define SOE_V_SEC_DN_MIN_V              2500            /*电压平台区间电压下限最小电压(1mV)*/
#define SOE_V_SEC_DN_MAX_V              2500            /*电压平台区间电压下限最大电压(1mV)*/
#define SOE_V_SEC_UP_MIN_V              3000            /*电压平台区间电压上限最小电压(1mV)*/
#define SOE_V_SEC_UP_MAX_V              3000            /*电压平台区间电压上限最大电压(1mV)*/
#define SOE_V_SEC_DN_LES_SOE            2000            /*电压高于下区间至小SOE(万分之一)*/
#define SOE_V_SEC_DN_MOS_SOE            4000            /*电压处于下区间至大SOE(万分之一)*/
#define SOE_V_SEC_UP_LES_SOE            9500            /*电压处于上区间至小SOE(万分之一)*/
#define SOE_V_SEC_UP_MOS_SOE            10000           /*电压低于上区间至大SOE(万分之一)*/
#endif


/*=============================================================================================
** @Name      : void GroupSoeDisplayInit(void)
** @Input     : void
** @Output    : void
** @Function  : SOE平滑显示初始化
** @The notes : 用户初始化调用
**===========================================================================================*/
void GroupSoeDisplayInit(void);
/*=============================================================================================
** @Name      : void GroupSoeDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : SOE平滑显示任务
** @The notes : 100ms
**===========================================================================================*/
void GroupSoeDisplayTask(void);
/*=============================================================================================
** @Name      : void CorrGDisplaySoeByUser(u8 clunum, u16 soe)
** @Input     : clunum:簇编号 soe:目标SOE(0.01%)
** @Output    : void
** @Function  : 用户修正当前显示SOE
** @The notes :
**===========================================================================================*/
void CorrGDisplaySoeByUser(u8 clunum, u16 soe);
/*=============================================================================================
** @Name      : static u16 CalcSoeChgDhgEndCorr(u8 clunum, u16 nowSoe)
** @Input     : clunum:簇编号 aimSoe:当前SOE(万分之一)
** @Output    : 修正后的SOE值(万分之一)
** @Function  : 计算SOE充放电末端修正值
** @The notes :
**===========================================================================================*/
static u16 CalcSoeChgDhgEndCorr(u8 clunum, u16 nowSoe);
/*=============================================================================================
** @Name      : static u16 CalcSoeSlowChang(u8 clunum, u16 aimSoe)
** @Input     : clunum:簇编号 aimSoe:目标SOE(万分之一)
** @Output    : 平滑后的SOE值(万分之一)
** @Function  : 计算SOE平滑值
** @The notes :
**===========================================================================================*/
static u16 CalcSoeSlowChang(u8 clunum, u16 aimSoe);
/*=============================================================================================
** @Name      : static void UpdateSoeSlowDisplay(u8 clunum, u16 slowSoe)
** @Input     : clunum:簇编号 slowSoe:平滑后待显示的值
** @Output    : void
** @Function  : 将平滑值更新到显示值
** @The notes :
**===========================================================================================*/
static void UpdateSoeSlowDisplay(u8 clunum, u16 slowSoe);
#endif