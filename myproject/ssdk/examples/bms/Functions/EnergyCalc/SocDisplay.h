/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocDisplay.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : SOC平滑显示模块
** @Instructions :
**===========================================================================================*/
#ifndef _SOCDISPALY_H
#define _SOCDISPALY_H

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "PublicInfo.h"
#include "BSPTypeDef.h"

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define SOC_SLOW_SPEED                  5               /*SOC平滑速度(原变化速度减缓倍率)*/

#define SOC_SLOW_VHER_SOC               10000           /*SOC平滑显示高压保护时目标SOC(万分之一)*/
#define SOC_SLOW_VH_SOC                 10000           /*SOC平滑显示高压告警时目标SOC(万分之一)*/
#define SOC_SLOW_VLER_SOC               0               /*SOC平滑显示低压保护时目标SOC(万分之一)*/
#define SOC_SLOW_VL_SOC                 30              /*SOC平滑显示低压告警时目标SOC(万分之一)*/

#define SOC_SLOW_CORR_IDLE_T            6000            /*SOC平滑显示可信度修正静置时间(0.1S)*/

/*不同电芯电压区间和对应SOC取值*/
#if(3 == CELL_CHEM_TYPE)                                /*钛酸锂电芯默认值*/
#define SOC_V_SEC_DN_MIN_V              2050            /*电压平台区间电压下限最小电压(1mV)*/
#define SOC_V_SEC_DN_MAX_V              2050            /*电压平台区间电压下限最大电压(1mV)*/
#define SOC_V_SEC_UP_MIN_V              2500            /*电压平台区间电压上限最小电压(1mV)*/
#define SOC_V_SEC_UP_MAX_V              2500            /*电压平台区间电压上限最大电压(1mV)*/
#define SOC_V_SEC_DN_LES_SOC            1000            /*电压高于下区间至小SOC(万分之一)*/
#define SOC_V_SEC_DN_MOS_SOC            4000            /*电压处于下区间至大SOC(万分之一)*/
#define SOC_V_SEC_UP_LES_SOC            8000            /*电压处于上区间至小SOC(万分之一)*/
#define SOC_V_SEC_UP_MOS_SOC            10000           /*电压低于上区间至大SOC(万分之一)*/

#elif(2 == CELL_CHEM_TYPE)                              /*磷酸铁锂电芯默认值*/
#define SOC_V_SEC_DN_MIN_V              3300            /*电压平台区间电压下限最小电压(1mV)*/
#define SOC_V_SEC_DN_MAX_V              3250            /*电压平台区间电压下限最大电压(1mV)*/
#define SOC_V_SEC_UP_MIN_V              3350            /*电压平台区间电压上限最小电压(1mV)*/
#define SOC_V_SEC_UP_MAX_V              3450            /*电压平台区间电压上限最大电压(1mV)*/
#define SOC_V_SEC_DN_LES_SOC            1000            /*电压高于下区间至小SOC(万分之一)*/
#define SOC_V_SEC_DN_MOS_SOC            3500            /*电压处于下区间至大SOC(万分之一)*/
#define SOC_V_SEC_UP_LES_SOC            6000            /*电压处于上区间至小SOC(万分之一)*/
#define SOC_V_SEC_UP_MOS_SOC            10000           /*电压低于上区间至大SOC(万分之一)*/

#if((3 == PRJ_PARA_NUM_INFO) || (4 == PRJ_PARA_NUM_INFO))/*自研PACK电芯*/
#define SOC_V_CHG_DN_MAX_V              3497            /*充电末端电压区间起始电压最大值(1mV)*/
#define SOC_V_CHG_UP_MAX_V              3535            /*充电末端电压区间结束电压最大值(1mV)*/
#define SOC_V_CHG_DN_LES_SOC            8600            /*充电末端电压区间起始电压至小SOC(万分之一)*/
#define SOC_V_CHG_UP_LES_SOC            9600            /*充电末端电压区间结束电压至小SOC(万分之一)*/

#else                                                   /*CATL-PACK电芯*/
#define SOC_V_CHG_DN_MAX_V              3457            /*充电末端电压区间起始电压最大值(1mV)*/
#define SOC_V_CHG_UP_MAX_V              3495            /*充电末端电压区间结束电压最大值(1mV)*/
#define SOC_V_CHG_DN_LES_SOC            9100            /*充电末端电压区间起始电压至小SOC(万分之一)*/
#define SOC_V_CHG_UP_LES_SOC            9600            /*充电末端电压区间结束电压至小SOC(万分之一)*/
#endif

#define SOC_V_DHG_UP_MAX_V              3070            /*放电末端电压区间起始电压最大值(1mV)*/
#define SOC_V_DHG_DN_MAX_V              2960            /*放电末端电压区间结束电压最大值(1mV)*/
#define SOC_V_DHG_UP_MOS_SOC            1500            /*放电末端电压区间起始电压至大SOC(万分之一)*/
#define SOC_V_DHG_DN_MOS_SOC            700             /*放电末端电压区间结束电压至大SOC(万分之一)*/

#elif(1 == CELL_CHEM_TYPE)                              /*三元电芯默认值*/
#define SOC_V_SEC_DN_MIN_V              3500            /*电压平台区间电压下限最小电压(1mV)*/
#define SOC_V_SEC_DN_MAX_V              3500            /*电压平台区间电压下限最大电压(1mV)*/
#define SOC_V_SEC_UP_MIN_V              4000            /*电压平台区间电压上限最小电压(1mV)*/
#define SOC_V_SEC_UP_MAX_V              4000            /*电压平台区间电压上限最大电压(1mV)*/
#define SOC_V_SEC_DN_LES_SOC            1000            /*电压高于下区间至小SOC(万分之一)*/
#define SOC_V_SEC_DN_MOS_SOC            3000            /*电压处于下区间至大SOC(万分之一)*/
#define SOC_V_SEC_UP_LES_SOC            8000            /*电压处于上区间至小SOC(万分之一)*/
#define SOC_V_SEC_UP_MOS_SOC            10000           /*电压低于上区间至大SOC(万分之一)*/

#else                                                   /*铅酸电芯*/
#define SOC_V_SEC_DN_MIN_V              2500            /*电压平台区间电压下限最小电压(1mV)*/
#define SOC_V_SEC_DN_MAX_V              2500            /*电压平台区间电压下限最大电压(1mV)*/
#define SOC_V_SEC_UP_MIN_V              3000            /*电压平台区间电压上限最小电压(1mV)*/
#define SOC_V_SEC_UP_MAX_V              3000            /*电压平台区间电压上限最大电压(1mV)*/
#define SOC_V_SEC_DN_LES_SOC            2000            /*电压高于下区间至小SOC(万分之一)*/
#define SOC_V_SEC_DN_MOS_SOC            4000            /*电压处于下区间至大SOC(万分之一)*/
#define SOC_V_SEC_UP_LES_SOC            9500            /*电压处于上区间至小SOC(万分之一)*/
#define SOC_V_SEC_UP_MOS_SOC            10000           /*电压低于上区间至大SOC(万分之一)*/
#endif
/*=============================================================================================
** @Name      : void GroupSocDisplayInit(void)
** @Input     : void
** @Output    : void
** @Function  : SOC平滑显示初始化
** @The notes : 用户初始化调用
**===========================================================================================*/
void GroupSocDisplayInit(void);
/*=============================================================================================
** @Name      : void GroupSocDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : SOC平滑显示任务
** @The notes : 100ms
**===========================================================================================*/
void GroupSocDisplayTask(void);
/*=============================================================================================
** @Name      : void CorrGDisplaySocByUser(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:目标SOC(0.01%)
** @Output    : void
** @Function  : 用户修正当前显示SOC
** @The notes :
**===========================================================================================*/
void CorrGDisplaySocByUser(u8 clunum, u16 soc);
/*=============================================================================================
** @Name      : static u16 CalcSocChgDhgEndCorr(u8 clunum, u16 nowSoc)
** @Input     : clunum:簇编号 aimSoc:当前SOC(万分之一)
** @Output    : 修正后的SOC值(万分之一)
** @Function  : 计算SOC充放电末端修正值
** @The notes :
**===========================================================================================*/
static u16 CalcSocChgDhgEndCorr(u8 clunum, u16 nowSoc);
/*=============================================================================================
** @Name      : static u16 CalcSocSlowChang(u8 clunum, u16 aimSoc)
** @Input     : clunum:簇编号 aimSoc:目标SOC(万分之一)
** @Output    : 平滑后的SOC值(万分之一)
** @Function  : 计算SOC平滑值
** @The notes :
**===========================================================================================*/
static u16 CalcSocSlowChang(u8 clunum, u16 aimSoc);
/*=============================================================================================
** @Name      : static void UpdateSocSlowDisplay(u8 clunum, u16 slowSoc)
** @Input     : slowSoc:平滑后待显示的值
** @Output    : void
** @Function  : 将平滑值更新到显示值
** @The notes :
**===========================================================================================*/
static void UpdateSocSlowDisplay(u8 clunum, u16 slowSoc);
#endif