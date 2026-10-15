/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : InsuCheck.h
** @Founder      : 
** @Date         : 2026.06.12
** @Function     : 
** @Instructions : 
**===========================================================================================*/
#ifndef FUNCTIONS_MEASUREMENT_INSUCHECK_H_
#define FUNCTIONS_MEASUREMENT_INSUCHECK_H_

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "IMIU4Protocol.h"
#include "IMIU4Interface.h"

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
/* 绝缘检测执行周期（毫秒）：参考2415BMU的ISO_ERR_DIAGTIME=60s */
#define INSU_CHECK_INTERVAL_MS  (60000)

/* 绝缘检测超时告警周期（毫秒）：参考 2415BMU ISO_ERR_DIAGTIME=120s */
#define ISO_ERR_DIAGTIME_MS     (120000)

/* 绝缘检测最低电池电压（V）：参考2415BMU的ISO_MIN_BAT_V=36.0V */
#define INSU_MIN_BAT_V          (36.0f)

/* 绝缘检测超时时间（毫秒）：IMIU4检测超时保护，60s */
#define INSU_CHECK_TIMEOUT_MS   (60000)

/* 绝缘检测启动命令下发后等待稳定时间（毫秒）：3s */
#define INSU_WAIT_STABLE_MS     (3000)

/* 外部请求检测超时时间（毫秒）：65s */
#define INSU_REQ_TIMEOUT_MS     (65000)

/* 上电强制首次检测窗口（毫秒）：5s */
#define INSU_BOOT_FORCE_MS      (5000)

/* 绝缘值无效默认值（KΩ）：参考2415BMU的ISORp_Inval_Value=3000 */
#define INSU_INVAL_VALUE        (3000)

#define INSU_CLUNUM              (2)  //绝缘检测簇数目

/* IMIU4 高压采样异常标志位掩码（告警信息1） */
#define IMIU4_ALM_HIGSAMP_INIT_BIT   (0x01)  /* bit0: 高压采集芯片初始化异常 */
#define IMIU4_ALM_HIGSAMP_COMM_BIT   (0x02)  /* bit1: 高压采集芯片通信异常 */

/*=============================================================================================
** @Data type definition
**=============================================================================================*/
/* 绝缘检测状态机（完整 4 态） */
typedef enum
{
    eInsuState_Idle  = 0,   /* 空闲，调用 SysNeedInsuCheckState 判断 */
    eInsuState_Start = 1,   /* 启动检测，下发 IMIU4 开始命令 */
    eInsuState_Wait  = 2,   /* 等待 IMIU4 检测完成 */
    eInsuState_Read  = 3,   /* 读取绝缘值，判断告警 */
} e_InsuState;

/* 绝缘故障状态（参考2415BMU的sInsuErrStateFlag） */
typedef enum
{
    eInsuErr_Norm  = 0,     /* 正常 */
    eInsuErr_Lev1  = 1,     /* 绝缘1级告警（保护） */
    eInsuErr_Lev2  = 2,     /* 绝缘2级告警（限制） */
    eInsuErr_Exp   = 3,     /* 绝缘检测异常（失效） */
} e_InsuErrState;

/*=============================================================================================
** @Function declaration
**=============================================================================================*/
/* 绝缘检测初始化 */
void InsuCheckInit(void);

/* 绝缘检测任务（100ms周期调用，参考2415BMU的InsuAdhesMeasurTask） */
void InsuCheckTask(void);

/* 获取绝缘故障状态（供DiagMaker调用，参考2415BMU的DiagGetDcSideLeakFlagAPI） */
u8 InsuGetErrState(u8 clunum);

/* 获取正极绝缘阻值（KΩ） */
u16 InsuGetPasValue(u8 clunum);

/* 获取负极绝缘阻值（KΩ） */
u16 InsuGetNegValue(u8 clunum);

/* 获取绝缘检测完成标志（供外部判断当前是否在检测中） */
u8 InsuGetCheckFinFlag(u8 clunum);

/* 获取绝缘检测计数器（供外部判断重试次数） */
u8 InsuGetDiagCount(u8 clunum);

/* 用户请求开始绝缘检测（2415BMU: UserReqInsuCheckAPI, line 917-921）
   重置完成标志，强制 SysNeedInsuCheckState 下一轮返回 TRUE */
void UserReqInsuCheckAPI(u8 clunum);

/* 清除绝缘故障标志（2415BMU: ClrInsuAdhesErrFlagAPI, line 980-984）
   BMU05 无粘连检测，仅清除绝缘故障状态 */
void ClrInsuAdhesErrFlagAPI(u8 clunum);

#endif /* FUNCTIONS_MEASUREMENT_INSUCHECK_H_ */