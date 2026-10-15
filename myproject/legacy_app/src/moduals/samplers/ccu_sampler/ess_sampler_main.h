/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief 储能EMS
* @history 2024-12-02 created
*******************************************************************************/

#ifndef __ESS_SAMPLER_H
#define __ESS_SAMPLER_H

#include "ccu_sampler_main.h"

// #define MAX_INTV_MS_CHGWORK         (0)     // 改变工作状态
// #define MAX_INTV_MS_CONTINUE_WORK   (10000) // 保持充放电状态
// #define MAX_INTV_MS_WAIT_PCS_WORK   (90000)
#define MIN_CURR_WORKING_ESS            (0.5f)    // 最小充放电电流，判断PCS是否正在充放电的额外条件

int Ems_Mgmt_Main_ESS(ROUGH_CCU_DATA* pCCU);

#endif
