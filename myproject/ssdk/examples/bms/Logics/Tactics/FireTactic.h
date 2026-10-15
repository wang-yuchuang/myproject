#ifndef __FIRETACTIC_H_
#define __FIRETACTIC_H_

/*=================================================================================================
**                                  Shenzhen Infypower Co., Ltd.
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : FireTactic.h
** @Founder      : NJX
** @Date         : 2026.08.18
** @Function     : 消防4级告警策略接口定义(方案C移植自100.37平台AuxDevTactic.c TacFireAlarmCtrlTask)
** @Instructions : 500ms任务节拍驱动;L1/L2/L3阈值按BMU05消防策略技术方案V1.2附录校准(L3气体H2/CO分列,
**                 IMEU3原码即ppm读数),L4气体阈值保持100.37老平台值2500(终判从严,方案无此级),
**                 阶段2迁入gGBmuHigLevPara_103参数表
**===============================================================================================*/

#include "BSPTypeDef.h"

/*=================================================================================================
** 消防DO通道映射(IMEU3 PDUIN 地址30,A2509单IMEU3;依据836原理图2026.7.16版IMEU3U1引脚:
** DO1主动消防(预留,FFD1/FFD2气溶胶灭火)/DO2声光ALM1/DO3烟感电源
** DO4 QFP5直流辅源开关(预留方案2:DC1±→QFP5→AP3 350W→DC24V网络;常开主触点,DO4=1合闸供电/DO4=0分闸断电,失效安全)
** DO5 远程急停(EPO失电安全链:DO5=1合闸正常/DO5=0分闸急停→KA1/KA3失电断主接触器+广播急停)
**===============================================================================================*/
#define FIRE_DO_AEROSOL          (1)      /*气溶胶灭火装置触发FFD1/FFD2(836图DO1主动消防预留位;受FIRE_PERFLU_EN门控,硬件确认实装前不点火)*/
#define FIRE_DO_ZHS              (2)      /*声光报警器ALM1(老平台:PDUOUT DO4/PDUIN DO1按项目码分支)*/
#define FIRE_DO_SMOK_PWR         (3)      /*烟感电源(老平台同DO3)*/
#define FIRE_DO_QFP5             (4)      /*QFP5直流辅源开关(预留方案2:DO4=1合闸供电/DO4=0分闸断电,常开主触点,失效安全)*/
#define FIRE_DO_EPO              (5)      /*远程急停(EPO失电安全链:DO5=1合闸正常/DO5=0分闸急停)*/

/*=================================================================================================
** 使能与阈值占位(阶段2迁入gGBmuHigLevPara_103参数表,单位注释如实标注tick换算,修复老平台缺陷②)
**===============================================================================================*/
#define FIRE_TACTIC_EN           (1)      /*消防策略编译使能*/
#define FIRE_PERFLU_EN           (1)      /*灭火触发使能(老平台PerfluorEn参数)*/
#define FIRE_DEBUG_LOG_EN        (1)      /*调试打印使能(1开0关,测试期printf到串口,量产前改回0)*/
#define FIRE_DEBUG_PROFILE       (0)      /*调试阈值档使能(1一键降低阈值便于台架测试,量产前改回0)*/
#define FIRE_BENCH_INJECT_EN     (0)      /*台架数据注入使能(1开启fire_inject CLI模拟传感器数据,量产前必剿0)*/

#define FIRE_L1_TEMP_TH          (800)    /*L1Pack最高温阈值 0.1℃(=80.0℃)*/
#define FIRE_L1_RISE_TH          (30)     /*L1温升阈值 0.1℃/5s窗(=3.0℃/5s,老平台哨兵200重置)*/
#define FIRE_L1_TIME_TH          (10)     /*L1持续判定 500ms/tick(10tick=5s,老注释"持续5s"与此一致)*/
#define FIRE_RISE_WIN_T          (10)     /*温升窗口宽度 500ms/tick(10tick=5s)*/

#define FIRE_L2A_TEMP_TH         (900)    /*L2a同芯阈值 0.1℃(=90.0℃)*/
#define FIRE_L2A_RISE_CNT        (5)      /*L2a连续温升达标窗口数(5窗=25s,老平台sRiseTime5s>=5)*/
#define FIRE_L2A_TIME_TH         (10)     /*L2a同芯持续判定 500ms/tick(10tick=5s)*/
#define FIRE_L2B_AERO_TH         (5000)   /*L2b气溶胶浓度阈值 原码(=5000ppm,方案V1.2附录;BSU02U1_AERO_ALARM同体系,见GroupInfo.h)*/
#define FIRE_L2B_NB_TEMP_TH      (800)    /*L2b邻芯温度阈值 0.1℃(=80.0℃)*/
#define FIRE_L2B_TIME_TH         (10)     /*L2b持续判定 500ms/tick(10tick=5s)*/
#define FIRE_L2C_RISE_TH         (30)     /*L2c邻芯温升阈值 0.1℃/10s窗(=3.0℃/10s)*/
#define FIRE_L2C_TIME_TH         (20)     /*L2c持续判定 500ms/tick(20tick=10s,老注释"持续5s"实为10s,已如实修正)*/

#define FIRE_L3_H2_TH            (2500)   /*L3 H2阈值 原码ppm(2026-09-08口径统一2500ppm,与L4及告警CGAS_EXP同口径;原方案V1.2附录800ppm)*/
#define FIRE_L3_CO_TH            (2500)   /*L3 CO阈值 原码ppm(2026-09-08口径统一2500ppm,与L4及告警CGAS_EXP同口径;原方案V1.2附录600ppm)*/
#define FIRE_L3_TIME_TH          (10)     /*L3持续判定 500ms/tick(10tick=5s)*/

#define FIRE_L4_H2_TH            (2500)   /*L4 H2阈值 原码(100.37老平台三级/四级统一2500,AuxDevTactic.c:2656-2744;保持老平台值终判从严,2026-08-28定;精确单位待H2确认)*/
#define FIRE_L4_CO_TH            (2500)   /*L4 CO阈值 原码(100.37老平台三级/四级统一2500;保持老平台值终判从严,2026-08-28定;精确单位待H2确认)*/
#define FIRE_L4_CABTEMP_TH       (70)     /*L4柜温阈值 ℃(老平台70℃;51表全域+50偏移已确认,改为使用处解码,阈值保持℃域)*/
#define FIRE_L4_TIME_TH          (20)     /*L4持续判定 500ms/tick(20tick=10s)*/
#define FIRE_SMOK_FAULT_EN       (0)      /*L4条件①烟感故障自检使能(待H4确认gImeu3AlmInfo bit映射,默认禁用)*/

#define FIRE_SMOK_VRY_OFF_T      (4)      /*烟感去误报断电时间 500ms/tick(4tick=2s)*/
#define FIRE_SMOK_VRY_FAST_T     (10)     /*烟感二次上电快速确认窗口 500ms/tick(10tick=5s)*/
#define FIRE_SMOK_VRY_WAIT_T     (30)     /*烟感二次上电确认总窗 500ms/tick(30tick=15s,老平台TAC_SMG_ONCON_T=30@1s任务)*/

#define FIRE_TEMP_INVALID        (-410)   /*单体温度无效哨兵(GetGCellTempAPI断线/越界返回值)*/

#if(FIRE_DEBUG_PROFILE)
/*台架测试低阈值档:大幅降低判定阈值以模拟各级触发,时间阈值保持不变(仍验证持续判定)*/
#undef  FIRE_L1_TEMP_TH
#define FIRE_L1_TEMP_TH          (250)    /*25.0℃*/
#undef  FIRE_L1_RISE_TH
#define FIRE_L1_RISE_TH          (2)      /*0.2℃/5s*/
#undef  FIRE_L2A_TEMP_TH
#define FIRE_L2A_TEMP_TH         (260)    /*26.0℃*/
#undef  FIRE_L2A_RISE_CNT
#define FIRE_L2A_RISE_CNT        (1)      /*1个温升窗即可*/
#undef  FIRE_L2B_AERO_TH
#define FIRE_L2B_AERO_TH         (1)      /*气溶胶最小原码*/
#undef  FIRE_L2B_NB_TEMP_TH
#define FIRE_L2B_NB_TEMP_TH      (250)    /*25.0℃*/
#undef  FIRE_L2C_RISE_TH
#define FIRE_L2C_RISE_TH         (2)      /*0.2℃/10s*/
#undef  FIRE_L3_H2_TH
#define FIRE_L3_H2_TH            (1)      /*H2最小原码*/
#undef  FIRE_L3_CO_TH
#define FIRE_L3_CO_TH            (1)      /*CO最小原码*/
#undef  FIRE_L4_H2_TH
#define FIRE_L4_H2_TH            (1)      /*H2最小原码*/
#undef  FIRE_L4_CO_TH
#define FIRE_L4_CO_TH            (1)      /*CO最小原码*/
#undef  FIRE_L4_CABTEMP_TH
#define FIRE_L4_CABTEMP_TH       (0)      /*柜温恒成立*/
#endif

/*=================================================================================================
** 数据类型定义
**===============================================================================================*/
/*消防告警等级*/
typedef enum
{
    eFireLevel_None = 0,                            /*无告警*/
    eFireLevel_1,                                   /*一级:Pack内单体高温+温升*/
    eFireLevel_2,                                   /*二级:同芯持续高温/气溶胶+邻芯高温/邻芯温升*/
    eFireLevel_3,                                   /*三级:可燃气体+二级*/
    eFireLevel_4                                    /*四级:灭火剂喷放后复合终判(接触器下电)*/
}e_FireLevel;

/*烟感去误报核实状态*/
typedef enum
{
    eSmogVry_None = 0,                              /*无告警*/
    eSmogVry_Checking = 1,                          /*正在核实(已断电重启)*/
    eSmogVry_Confirmed = 2                          /*确认告警*/
}e_SmogVryState;

/*=================================================================================================
** 函数声明
**===============================================================================================*/
void TacFireAlarmCtrlTask(void);
u8 TacGetFireLevelAPI(u8 clunum);
u8 TacGetFireLevelPosAPI(u8 clunum);
u8 TacGetFireSmogVryStateAPI(void);

#endif

