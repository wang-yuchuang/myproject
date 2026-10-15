/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief  AEMS-According EMS,EMS协调器
* @history 2025-11-11 created
*******************************************************************************/

#ifndef __SAMPLERS_AEMS_MAIN_H__
#define __SAMPLERS_AEMS_MAIN_H__

#include "../sampler_base.h"

#define MIN_PWR_CHRG_4START           (10)  // 最小启动充电功率

typedef enum tagCtrlPVMode
{
	CtrlPVMode_Auto, // 自动
	CtrlPVMode_DAQ,  // 数采控制
    CtrlPVMode_AEMS, // AEMS控制
    CtrlPVMode_MAX
} CtrlPVMode;

typedef struct tagBESS
{
    A_BLOB_SEMS_CLIENTINFO_SUM      bessInfo;
    // BOOL                            bCtrlStandby;
    time_t                          tmCtrlStandby;
} BESS;

typedef struct tagPV
{
    A_BLOB_PV_INVERTER_DATA_TOTAL   pvInfo; // pv_inverter_sampler report
    A_BLOB_PV_CTRL_BY_AEMS          pvCtrl;
    time_t                          tmCtrlPwr_Last;
    BOOL                            bRetry;
    double                          dtmRetry_Last;
} PV;

typedef struct tagROUGH_AEMS
{
    HANDLE                          hSampler; //pointer to sampler
	HANDLE                          hPort; //opened port handle
	DATA_PROCESSOR*                 pDPR;
	int                             nSamplerId; //copy from sampler
	HANDLE                          hSamplerLock;

    BOOL                            bAntiBackflow_PV; // 1050, 440, PV是否有防逆流功能
    BOOL                            bAutoChrgEnable;

    // setParam
    CtrlPVMode                      ctrlPVMode; // 991, 990,
    // float                           fPowerRate_AEMS; // 跟随比
    float                           fProtBackflow_PV_AEMS; // 预保护值(AEMS-PV) = fPreProt4RefluxAndOverLoad
    float                           fProtBackflow_PV_DAQ; // 数采防逆流预保护值(DAQ-PV) PreProt(DAQ-PV) 991, 991,
    UINT                            uPStep_PV; // 调节功率步长 991, 992,
    UINT                            uMsStep_PV; // 调节时间步长 991, 993,
    UINT                            uMaxPBackflow; // 逆流最大功率 991, 994,
    UINT                            uMaxMsBackflow; // 逆流最长时间 991, 995,

    A_BLOB_ES_BACKEND_CFG 			blobBackendCfg;
    float                           fPBackflow; // 逆流功率
    float                           fPLoad_AC; // 负载功率
    float                           fMax_Chrg_PwrAbility_AEMS;
    float                           fPLoad_AndBess; // 负载+BESS总消纳功率
    float                           fRemainChrgAbl_Bess; // BESS剩余消纳能力
    BOOL                            bChrgAbl_Bess; // BESS是否有消纳逆流功率的能力
    BOOL                            bPwrDown_PV; // 是否需要降光伏功率
    BOOL                            bPwrUpStep_PV; // 是否需要往上步调
    UINT                            msBackflow; // 逆流的毫秒数
    double                          dtmBackflow_Start;

    float                           fLtPwr2Standby_4Chrg; // 开始待机功率(小于) 1001, 3317,
	UINT                            uDelayStandby_4Chrg; // 待机延时(充电) 1001, 3318,
    BESS                            bess; // BESS
    PV                              pv;   // PV
} ROUGH_AEMS;

#endif