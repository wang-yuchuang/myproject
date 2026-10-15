/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author LJH936
* @brief 站级EMS
* @history
1. 2024-12-02 created
2. LJH250205 REMS 川崎项目增加上级EMS(SEMS:本地EMS REMS:上级EMS)
*******************************************************************************/

#ifndef __SEMS_SAMPLER_H
#define __SEMS_SAMPLER_H

#include "ccu_sampler_main.h"

#define MAX_LEN_SZIP            16
#define TCP_CLIENT_READ_MIN_INTERV_MSEC  50
// #define  CCU_TCP_BUFFLEN_SEMS                  4096
#define CCU_TCP_BUFFLEN_SEMS        TCP_BUFF_LEN

#define MAX_INTV_MS_CHGWORK         (0)     // 改变工作状态
#define MAX_INTV_MS_CONTINUE_WORK   (10000) // 保持充放电状态
#define MAX_INTV_MS_WAIT_PCS_WORK   (90000)
#define MIN_CURR_WORKING            (0.5f)    // 最小充放电电流，判断PCS是否正在充放电的额外条件
// #define MIN_PWR_CHRG                (2.0f)   // 最小充电功率，判断PCS是否正在充电
// #define MIN_PWR_DISCHRG             (-2.0f)   // 最小放电功率，判断PCS是否正在放电
#define MIN_DIFF_VALUE_REQ          (3.0f)
#define MIN_CURR_PCS_OUT_ABL        (2.0f)    // PCS最小输出电流能力阈值，判断PCS是否正常
#define MIN_PWR_PCS_OUT_ABL         (2.0f)    // PCS最小输出功率能力阈值，判断PCS是否正常
#define MIN_PWR_OUT_VSREQ           (20.0f)   // PCS输出与需求差值，判断是否继续下发功率

int Parse_CFGG_SEMS(ROUGH_CCU_DATA *pCCU, BYTE* pData, USHORT usDataLen, int idx);
int Parse_CFGS_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int idx);
int Parse_UCTL_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int idx);
int Parse_LSYS_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int idx);
int Parse_LDEV_SEMS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int idx);
int Parse_PCS_INFO(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int idx);

BOOL DoTCPSession_SEMS(int nWhichClient, NL_MSGTYPE emMsgTp, BYTE *pData, USHORT usDataLen, int nTimeOut_mSec);
BOOL DoUDPSession_SEMS(int nWhichClient, NL_MSGTYPE emMsgTp, BYTE *pData, USHORT usDataLen, int nTimeOut_mSec);
// BOOL DoTCP_CCU_Upgrade(int nWhichClient);
BOOL Wait_ACCU_TCPReply(int nWhichClient, NLMSG_FRAME_INFO_SEMS *pFramInfo, int nTimeOut_mSec);

BOOL Send_61CFGG(int n);
BOOL Send_6202CFGS_RTC(int n);
BOOL Send_6204CFGS_LSYSPeriod(int n);
BOOL Send_6205CFGS_StopSoc(int n);
BOOL Send_6206CFGS_SendCmdByLC1stOnly(int n);
BOOL Send_6301UCTL_AutoManuSwitchMode(int n, AutoManuSwitchMode mode);
BOOL Send_6302UCTL_OnOffGridMode(int n, OnOffGridMode state);
BOOL Send_6317UCTL_CtrlRectMode(int n, CtrlRectMode mode);
BOOL Send_6330UctlWorkPwrAuto(ROUGH_CCU_DATA* pCCU, ACDC_WORKSTATU emACDC_WorkStatuReq, int n, int msElapse);
BOOL Send_6330UctlWorkPwrAuto_UDP(ROUGH_CCU_DATA* pCCU, ACDC_WORKSTATU emACDC_WorkStatuReq, int n, int msElapse);
// LJH250930 加快自动充放响应,standbySetReq
BOOL Send_6360UCTL_Standby(int n, BOOL bStandby);
BOOL Send_6390UCTL_EnableDisableSystem(int n, CCU_STATE state);
BOOL Send_6391_SlaveUpgradeCmd(int n, S63Uctl_EmsUpgradeCmd byUpgradeCmd);
BOOL Send_65Query_PCSInfo(int n);
void Send_A6USYS(int n);

BOOL Initialize_SEMS_StateM(HANDLE hSamplerThis);
int Sample_SEMS_StateM(HANDLE hSamplerThis);

int Ems_Mgmt_Main_SEMS(ROUGH_CCU_DATA* pCCU);

#endif
