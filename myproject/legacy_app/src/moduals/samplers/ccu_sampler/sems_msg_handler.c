// #define _TRACE_PRINT_ // SEMS
#include "sems_sampler_main.h"

#define MAX_MS_DoTCPSession_SEMS_NoWaitREP (-1)
#define MAX_MS_DoTCPSession_SEMS_CFGS      (1000)
#define MAX_MS_DoTCPSession_SEMS_UCTL      (1000)

// #define IDX_CLIENT  ((BYTE)(n + 1));
#define IDX_CLIENT  (s_MMClients[n].idxClientRecv);

static BOOL ReverseS62CFGS_SingleAndSend(int n, S62CFGS_Single* pData, int sizeOfVal, int nTimeOut_mSec)
{
	reverseBytes(pData->bytesVal, sizeOfVal);
	return DoTCPSession_SEMS(n, NL_MSG_CFGS, (BYTE*)pData,
							(USHORT)(offsetof(S62CFGS_Single, bytesVal) + (USHORT)sizeOfVal),
							nTimeOut_mSec);
}

static BOOL ReverseS63UCTL_SingleAndSend(int n, S63UCTL_Single* pData, int sizeOfVal, int nTimeOut_mSec)
{
	pData->idxClient = IDX_CLIENT;
	// pData->tmStamp =   GetCurrentBcdTimeStmp();
	// reverseBytes((BYTE*)&pData->tmStamp, sizeof(pData->tmStamp));
	reverseBytes((BYTE*)&pData->idxClient, sizeof(pData->idxClient));
	reverseBytes(pData->bytesVal, sizeOfVal);
	return DoTCPSession_SEMS(n, NL_MSG_UCTL, (BYTE*)pData,
							(USHORT)(offsetof(S63UCTL_Single, bytesVal) + (USHORT)sizeOfVal),
							nTimeOut_mSec);
}

// 61站级EMS获取从机配置参数（SEMS_CFGG）
BOOL Send_61CFGG(int n)
{
	// BYTE bysVerSEMSProt[1]; // LJH250630 SEMSProtVer
	// bysVerSEMSProt[0] = SEMS_PROTOCAL_VER;
	ROUGH_CCU_DATA *pCCU = s_MMClients[n].pCCU;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	SEMS_61_CFGG_DATA_INFO sendData;
	ZERO_POBJS(&sendData, 1);
	//DataLen
	USHORT usDataLen = sizeof(SEMS_61_CFGG_DATA_INFO);
	sendData.byVerSEMSProt = SEMS_PROTOCAL_VER; // LJH250630 SEMSProtVer
	sendData.nSlaveIP =  0;// (UINT)pSlave->blobNetWorkInfo.netInfo_eht0.ulIpV4_IP;
	sendData.idxClient = pCCU->lcCfg.idxClientLC;
	// memcpy(sendData.szIMMU2_SWVer, pSlave->szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	sendData.tmStamp = GetCurrentBcdTimeStmp();
	
	// sendData.SEMS_LSYS_Period = pSlave->Sent_SEMS_LSYS_Period;
	
	sendData.nStopSOC_ChrgSet = pEMS->nStopSOC_ChrgSet_SEMS;
	sendData.nStopSOC_DiscSet = pEMS->nStopSOC_DiscSet_SEMS;
	sendData.bSendCmdByLC1stOnly = pCCU->lcCfg.bSendCmdByLC1stOnly;
	TRACEI("Send_61CFGG usDataLen = [%d] nSlaveIP:%08X idxClient:%u szIMMU2_SWVer:%s "
		"SEMS_LSYS_Period = [%d] StopSOC:%d-%d bSendCmdByLC1stOnly:%d",
		usDataLen, sendData.nSlaveIP, sendData.idxClient, sendData.szIMMU2_SWVer,
		sendData.SEMS_LSYS_Period, sendData.nStopSOC_ChrgSet, sendData.nStopSOC_DiscSet, sendData.bSendCmdByLC1stOnly);

	reverseBytes((BYTE *)sendData.szIMMU2_SWVer, MAX_LEN_MU2SWVER);
	reverseBytes((BYTE *)&sendData, usDataLen);
	BOOL ret = DoTCPSession_SEMS(n, NL_MSG_CFGG,  (BYTE*)&sendData, usDataLen, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_61CFGG n:%d ret:%d", n, ret);
	return ret;
}

// 62-02 RTC Time
BOOL Send_6202CFGS_RTC(int n)
{
	S62CFGS_Single sendData;
	sendData.subCmd = 0x02;
	sendData.sub02BcdTimeStmp = GetCurrentBcdTimeStmp();
	BOOL ret = ReverseS62CFGS_SingleAndSend(n, &sendData, sizeof(sendData.sub02BcdTimeStmp), MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6202CFGS_RTC n:%d ret:%d val<%s>", n, ret, printableBcdTimeStmp((BcdTimeStmp*)(reverseBytes((BYTE*)&sendData.sub02BcdTimeStmp, sizeof(BcdTimeStmp)))));
	return ret;
}

// 62-04 上报SEMS_LSYS周期
BOOL Send_6204CFGS_LSYSPeriod(int n)
{
	ROUGH_CCU_DATA *pCCU = s_MMClients[n].pCCU;
	S62CFGS_Single sendData;
	sendData.subCmd = 0x04;
	sendData.sub04Period = 1000; // pCCU->msPeriodLSYS;
	BOOL ret = ReverseS62CFGS_SingleAndSend(n, &sendData, sizeof(sendData.sub04Period), MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6204CFGS_LSYSPeriod n:%d ret:%d val<%u> msPeriodLSYS:%u", n, ret, sendData.sub04Period, pCCU->msPeriodLSYS);
	return ret;
}

// LJH250930 加快自动充放响应,StopSOC_SetSEMS
BOOL Send_6205CFGS_StopSoc(int n)
{
	ROUGH_CCU_DATA *pCCU = s_MMClients[n].pCCU;
	S62CFGS_Single sendData;
	sendData.subCmd = 0x05;
	sendData.bytesVal[0] = pCCU->emsMgmtData.nStopSOC_ChrgSet_SEMS;
	sendData.bytesVal[1] = pCCU->emsMgmtData.nStopSOC_DiscSet_SEMS;
	BOOL ret = DoTCPSession_SEMS(n, NL_MSG_CFGS, (BYTE*)&sendData,
							(USHORT)(offsetof(S62CFGS_Single, bytesVal) + 2),
							MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6205CFGS_StopSoc n:%d ret:%d stopsoc<%d %d>", n, ret, sendData.bytesVal[0], sendData.bytesVal[1]);
	return ret;
}

// LJH250930 加快自动充放响应,StopSOC_SetSEMS
BOOL Send_6206CFGS_SendCmdByLC1stOnly(int n)
{
	ROUGH_CCU_DATA *pCCU = s_MMClients[n].pCCU;
	S62CFGS_Single sendData;
	sendData.subCmd = 0x06;
	sendData.bytesVal[0] = pCCU->lcCfg.bSendCmdByLC1stOnly;
	BOOL ret = DoTCPSession_SEMS(n, NL_MSG_CFGS, (BYTE*)&sendData,
							(USHORT)(offsetof(S62CFGS_Single, bytesVal) + 1),
							MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6206CFGS_SendCmdByLC1stOnly n:%d ret:%d bSendCmdByLC1stOnly<%d>", n, ret, sendData.bytesVal[0]);
	return ret;
}

// 63-01并离网自动/手动切换模式
BOOL Send_6301UCTL_AutoManuSwitchMode(int n, AutoManuSwitchMode mode)
{
	S63UCTL_Single sendData;
	sendData.subCmd = 0x01;
	sendData.byteVal = mode;
	BOOL ret = ReverseS63UCTL_SingleAndSend(n, &sendData, 1, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6301UCTL_AutoManuSwitchMode n:%d ret:%d val<%x>", n, ret, sendData.byteVal);
	return ret;
}

// 63-02手动并/离网模式
BOOL Send_6302UCTL_OnOffGridMode(int n, OnOffGridMode state)
{
	S63UCTL_Single sendData;
	sendData.subCmd = 0x02;
	sendData.byteVal = state;
	BOOL ret = ReverseS63UCTL_SingleAndSend(n, &sendData, 1, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6302UCTL_OnOffGridMode n:%d ret:%d val<%x>", n, ret, sendData.byteVal);
	return ret;
}

// 63-14手动控制充放电功率

// 63-17设置自动、手动控制PCS模式
BOOL Send_6317UCTL_CtrlRectMode(int n, CtrlRectMode mode)
{
	S63UCTL_Single sendData;
	sendData.subCmd = 0x17;
	sendData.byteVal = mode;
	BOOL ret = ReverseS63UCTL_SingleAndSend(n, &sendData, 1, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6317UCTL_CtrlRectMode n:%d ret:%d val<%x>", n, ret, sendData.byteVal);
	return ret;
}

// 6330自动控制充放电功率_UDP
BOOL Send_6330UctlWorkPwrAuto_UDP(ROUGH_CCU_DATA* pCCU, ACDC_WORKSTATU emACDC_WorkStatuReq, int n, int msElapse)
{
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node =   &pEMS->calc_Node[n];
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[n];
	PcsSMInfo* pSMInfo =                         &pCCU->semsSMInfos[n];
	SEMS_B6_LSYS_DATA_INFO* pB6LSYS =            &pBranchGrpData->b6LSYSLast_SEMS;
	double tmNow = GetCurrentTime();
	S63Uctl_30WorkPwr* pCtrlSend =               &pSMInfo->sems6330ctrlPwrReq;
	pCtrlSend->subCmd =     0x30;
	pCtrlSend->idxClient =  IDX_CLIENT;
	pCtrlSend->emACDC_WorkStatuCtrl = emACDC_WorkStatuReq;
	pCtrlSend->byWorkAsap = pEMS->byWorkAsap;
	// pCtrlSend->nStopSoc = (BYTE)pEMS->planSectorNow.nStopSOC;
	// pCtrlSend->nBattGrpPwrLmtPlan = pEMS->planSectorNow.nBattGrpPwrLmt;
	switch (emACDC_WorkStatuReq)
	{
		case STATU_ACDC_RECTIFY:
			pCtrlSend->fActv_PwrCtrl = MIN(pCalc_Node->fACDC_Distri_4_BattChrg_Pwr, pB6LSYS->fMax_Chrg_PwrAbilitySum);
			break;
		case STATU_ACDC_INVERTER:
			// printf("pB6LSYS->fMax_DisChrg_PwrAbilitySum = [%0.3f]\n",pB6LSYS->fMax_DisChrg_PwrAbilitySum);
			pCtrlSend->fActv_PwrCtrl = MIN(pCalc_Node->fBatt_Distri_4_ACDC_Pwr, pB6LSYS->fMax_DisChrg_PwrAbilitySum);
			break;

		default:
			pCtrlSend->fActv_PwrCtrl = 0;
			pSMInfo->tm_LastStopWork = tmNow;
			break;
	}
	
	float fActv_PwrReqDiff = (float)fabs(pSMInfo->fActv_PwrCtrl_Last - pCtrlSend->fActv_PwrCtrl);
	float fActv_PwrOutDiff = (float)fabs(fabs(pB6LSYS->fActvPwr_PCSGrpSum) - pCtrlSend->fActv_PwrCtrl);
	// printf("Send_6330UctlWorkPwrAuto_UDP n:%d fActv_PwrReqDiff:%.1f fActv_PwrOutDiff:%.1f fActv_Pwr:%.1f "
	// 	"fActv_PwrCtrl_Last:%.1f fActvPwr_PCSGrpSum:%.1f\n",
	// 	n, fActv_PwrReqDiff, fActv_PwrOutDiff, pCtrlSend->fActv_PwrCtrl,
	// 	pSMInfo->fActv_PwrCtrl_Last, pB6LSYS->fActvPwr_PCSGrpSum);
	// if (fActv_PwrReqDiff>MIN_DIFF_VALUE_REQ || pCtrlSend->emACDC_WorkStatuCtrl!=pSMInfo->emACDC_WorkStatuCtrl_Last ||
	// 	fActv_PwrOutDiff>MIN_PWR_OUT_VSREQ ||
	// 	(ElapseMS(pSMInfo->tm_LastSendCmd_2Pcs) >= (UINT)msElapse))
	{
		USHORT usDataLen = (USHORT)sizeof(S63Uctl_30WorkPwr);
		TRACED("Send_6330UctlWorkPwrAuto_UDP n:%d emACDC_WorkStatuCtrl:%d fActv_Pwr:%.1f fActv_PwrReqDiff:%.1f fActv_PwrOutDiff:%.1f "
			"fActv_PwrCtrl_Last:%.1f fActvPwr_PCSGrpSum:%.1f ElapseMS:%u %d <%s>\n",
			n, pCtrlSend->emACDC_WorkStatuCtrl, pCtrlSend->fActv_PwrCtrl, fActv_PwrReqDiff, fActv_PwrOutDiff,
			pSMInfo->fActv_PwrCtrl_Last, pB6LSYS->fActvPwr_PCSGrpSum, ElapseMS(pSMInfo->tm_LastSendCmd_2Pcs), msElapse,
			printableBytes((BYTE*)pCtrlSend, usDataLen));
		// LogOut(MAIN_MODULE_MAIN, "QuickChannel", LOG_TYPE_WARING,
		// 	"Send_6330UctlWorkPwrAuto_UDP Sent fActv_PwrCtrl[%.1f]",pCtrlSend->fActv_PwrCtrl);
		
		pSMInfo->emACDC_WorkStatuCtrl_Last = pCtrlSend->emACDC_WorkStatuCtrl;
		pSMInfo->fActv_PwrCtrl_Last =        pCtrlSend->fActv_PwrCtrl;
		pSMInfo->byWorkASAP_Last =           pCtrlSend->byWorkAsap;
		pSMInfo->tm_LastSendCmd_2Pcs = tmNow;
		reverseBytes((BYTE*)pCtrlSend, usDataLen);
		DoUDPSession_SEMS(n, NL_MSG_UCTL,  (BYTE*)pCtrlSend, usDataLen, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	}

	return TRUE;
}
// 6330自动控制充放电功率
BOOL Send_6330UctlWorkPwrAuto(ROUGH_CCU_DATA* pCCU, ACDC_WORKSTATU emACDC_WorkStatuReq, int n, int msElapse)
{
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node =   &pEMS->calc_Node[n];
	A_BLOB_BRANCH_GRP_RECTDATA* pBranchGrpData = &pCCU->strLSYSInfo.branchNodeData[n];
	PcsSMInfo* pSMInfo =                         &pCCU->semsSMInfos[n];
	SEMS_B6_LSYS_DATA_INFO* pB6LSYS =            &pBranchGrpData->b6LSYSLast_SEMS;
	double tmNow = GetCurrentTime();
	S63Uctl_30WorkPwr* pCtrlSend =               &pSMInfo->sems6330ctrlPwrReq;
	pCtrlSend->subCmd =     0x30;
	pCtrlSend->idxClient =  IDX_CLIENT;
	pCtrlSend->emACDC_WorkStatuCtrl = emACDC_WorkStatuReq;
	pCtrlSend->byWorkAsap = pEMS->byWorkAsap;
	// pCtrlSend->nStopSoc = (BYTE)pEMS->planSectorNow.nStopSOC;
	// pCtrlSend->nBattGrpPwrLmtPlan = pEMS->planSectorNow.nBattGrpPwrLmt;
	switch (emACDC_WorkStatuReq)
	{
		case STATU_ACDC_RECTIFY:
			pCtrlSend->fActv_PwrCtrl = MIN(pCalc_Node->fACDC_Distri_4_BattChrg_Pwr, pB6LSYS->fMax_Chrg_PwrAbilitySum);
			break;
		case STATU_ACDC_INVERTER:
			pCtrlSend->fActv_PwrCtrl = MIN(pCalc_Node->fBatt_Distri_4_ACDC_Pwr, pB6LSYS->fMax_DisChrg_PwrAbilitySum);
			break;

		default:
			pCtrlSend->fActv_PwrCtrl = 0;
			pSMInfo->tm_LastStopWork = tmNow;
			break;
	}
	
	float fActv_PwrReqDiff = (float)fabs(pSMInfo->fActv_PwrCtrl_Last - pCtrlSend->fActv_PwrCtrl);
	float fActv_PwrOutDiff = (float)fabs(fabs(pB6LSYS->fActvPwr_PCSGrpSum) - pCtrlSend->fActv_PwrCtrl);
	TRACED("Send_6330UctlWorkPwrAuto n:%d emACDC_WorkStatuCtrl:%d fActv_PwrCtrl:%.1f "
		"fActv_PwrReqDiff:%.1f fActv_PwrOutDiff:%.1f fActv_PwrCtrl_Last:%.1f fActvPwr_PCSGrpSum:%.1f byWorkAsap:%d (%d-%d)",
		n, pCtrlSend->emACDC_WorkStatuCtrl, pCtrlSend->fActv_PwrCtrl,
		fActv_PwrReqDiff, fActv_PwrOutDiff, pSMInfo->fActv_PwrCtrl_Last, pB6LSYS->fActvPwr_PCSGrpSum,
		pCtrlSend->byWorkAsap, ((U6330WorkAsap*)&pCtrlSend->byWorkAsap)->bChrgAsap, ((U6330WorkAsap*)&pCtrlSend->byWorkAsap)->bDiscAsap);
	if (pEMS->emHighSensitivityEnable || fActv_PwrReqDiff>MIN_DIFF_VALUE_REQ || pCtrlSend->emACDC_WorkStatuCtrl!=pSMInfo->emACDC_WorkStatuCtrl_Last ||
		fActv_PwrOutDiff>MIN_PWR_OUT_VSREQ || pCtrlSend->byWorkAsap!=pSMInfo->byWorkASAP_Last ||
		(ElapseMS(pSMInfo->tm_LastSendCmd_2Pcs) >= (UINT)msElapse))
	{
		// printf("Send_6330UctlWorkPwrAuto: %d\n", pEMS->emHighSensitivityEnable);
		USHORT usDataLen = (USHORT)sizeof(S63Uctl_30WorkPwr);
		TRACEI("Send_6330UctlWorkPwrAuto n:%d emACDC_WorkStatuCtrl:%d fActv_Pwr:%.1f fActv_PwrReqDiff:%.1f fActv_PwrOutDiff:%.1f "
			"fActv_PwrCtrl_Last:%.1f fActvPwr_PCSGrpSum:%.1f byWorkAsap:%d"
			"ElapseMS:%u %d <%s>",
			n, pCtrlSend->emACDC_WorkStatuCtrl, pCtrlSend->fActv_PwrCtrl, fActv_PwrReqDiff, fActv_PwrOutDiff,
			pSMInfo->fActv_PwrCtrl_Last, pB6LSYS->fActvPwr_PCSGrpSum,
			pCtrlSend->byWorkAsap,
			ElapseMS(pSMInfo->tm_LastSendCmd_2Pcs), msElapse, printableBytes((BYTE*)pCtrlSend, usDataLen));
		
		pSMInfo->emACDC_WorkStatuCtrl_Last = pCtrlSend->emACDC_WorkStatuCtrl;
		pSMInfo->fActv_PwrCtrl_Last =        pCtrlSend->fActv_PwrCtrl;
		pSMInfo->byWorkASAP_Last =           pCtrlSend->byWorkAsap;
		pSMInfo->tm_LastSendCmd_2Pcs = tmNow;
		reverseBytes((BYTE*)pCtrlSend, usDataLen);
		DoTCPSession_SEMS(n, NL_MSG_UCTL,  (BYTE*)pCtrlSend, usDataLen, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	}

	return TRUE;
}

// LJH250930 加快自动充放响应,standbySetReq
// 63-60热待机模式
BOOL Send_6360UCTL_Standby(int n, BOOL bStandby)
{
	S63UCTL_Single sendData;
	sendData.subCmd =    0x60;
	sendData.byteVal =   (BYTE)bStandby;
	BOOL ret = ReverseS63UCTL_SingleAndSend(n, &sendData, 1, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6360UCTL_Standby n:%d ret:%d val<%x>", n, ret, sendData.byteVal);
	return ret;
}

// 63-90启用/禁用系统
BOOL Send_6390UCTL_EnableDisableSystem(int n, CCU_STATE state)
{
	S63UCTL_Single sendData;
	sendData.subCmd = 0x90;
	sendData.byteVal = state;
	BOOL ret = ReverseS63UCTL_SingleAndSend(n, &sendData, 1, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	TRACEI("Send_6390UCTL_EnableDisableSystem n:%d ret:%d val<%x>", n, ret, sendData.byteVal);
	return ret;
}

BOOL Send_6391_SlaveUpgradeCmd(int n, S63Uctl_EmsUpgradeCmd byUpgradeCmd)
{
	byUpgradeCmd.subCmd = 0x91;
	byUpgradeCmd.idxClient = n;
	USHORT usDataLen = (USHORT)sizeof(S63Uctl_EmsUpgradeCmd);
	
	reverseBytes((BYTE*) &byUpgradeCmd, usDataLen);
	BOOL ret = DoTCPSession_SEMS(n - 1, NL_MSG_UCTL, (BYTE*) &byUpgradeCmd, usDataLen, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	return ret;
}

BOOL Send_65Query_PCSInfo(int n)
{
	BYTE Flag = 1;
	USHORT usDataLen = (USHORT)sizeof(BYTE);
	
	BOOL ret = DoTCPSession_SEMS(n, NL_MSG_PCSINFO, (BYTE*) &Flag, usDataLen, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
	return ret;
}


void Send_A6USYS(int n)
{
	if (s_MMClients[n].emClient_RegisteredStatus != CCU_REGISTERED_OK)
	{
		TRACEW("Send_A6USYS emClient_RegisteredStatus != CCU_REGISTERED_OK");
		return;
	}
	// //send it every 1sec
	// if (ElapseMS(s_MMClients[n].tmLastSentUSYS) < 1.0f)
	// {
	// 	TRACEW("Send_A6USYS tmLastSentUSYS < 1.0");
	// 	return;
	// }
	// else
	if(ElapseMS(s_MMClients[n].tmLastSentCmd) < 0.3f)
	{
		TRACEW("Send_A6USYS tmLastSentCmd < 0.3f");
		return;
	}
	double fTimeNow = GetCurrentTime();
	s_MMClients[n].tmLastSentUSYS = fTimeNow;
	
	ROUGH_CCU_DATA *pCCU = s_MMClients[n].pCCU;
	EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;
	SEMS_A6_USYS_DATA_INFO sendData;
	sendData.idxClient = IDX_CLIENT;
	sendData.tmStamp = GetCurrentBcdTimeStmp();
	sendData.emUSYSStationEMSAvaiStatus = pCCU->emCCUState;
	sendData.emCtrlRectMode =       (pCCU->emPMCtrlMode == AUTO_CTRL_PM_MODE ? CtrlRectMode_Auto : CtrlRectMode_Manu);
	sendData.emAutoManuSwitchMode = (pCCU->emSTSAutoManual == AUTO_ONOFFGRID_MODE ? AutoManuSwitchMode_Auto : AutoManuSwitchMode_Manu);
	sendData.emOnOffGridMode =      (pCCU->emSTSOnOffGridState == STATE_STS_ONGRID ? OnOffGridMode_On_Invert : OnOffGridMode_Off);
	// LJH250612 planSector USYS发送的是主控的PlanSectorOrg,单组电池组功率限制设置的是ESC
	if (pCCU->lcCfg.lcType == LC_TYPE_1st)
	{
		EMS_LIMIT_SECTOR* pPlanSector = &pEMS->planSectorOrg;
		if (pEMS->bCtrlByREMS)
		{
			// LJH250902 top#6+REMS
			pPlanSector = &pEMS->planSectorNow;
		}
		if ((NEED_AUTOCHRG() &&
				pEMS->bEnter_AutoChrg_SEMS) || // AutoCharging_IfReflux
			(pEMS->bAutoDischrgEnable &&
				pEMS->bEnter_AutoDisc_SEMS)) // AutoDischarging_IfOverload
		{
			// LJH250910 fix：美华智充-光伏逆流不充/过载不放
			pPlanSector = &pEMS->planSector4USYSIfAutoChDis;
		}
		sendData.emBatt_WorkModePlan = (BYTE)pPlanSector->emBatt_WorkMode;
		sendData.nStopSOCPlan =        (BYTE)pPlanSector->nStopSOC;
		sendData.nActvPwrLmtPlan =           pPlanSector->nActvPwrLmt;
		sendData.nReActvPwrLmtPlan =         pPlanSector->nReActvPwrLmt;
		sendData.nBattGrpPwrLmtPlan =        pPlanSector->nBattGrpPwrLmt;
	}
	else
	{
		SEMS_A6_USYS_DATA_INFO* pA6_USYS = &pEMS->blobA6_USYS.A6_USYS;
		sendData.emBatt_WorkModePlan = pA6_USYS->emBatt_WorkModePlan;
		sendData.nStopSOCPlan =        pA6_USYS->nStopSOCPlan;
		sendData.nActvPwrLmtPlan =     pA6_USYS->nActvPwrLmtPlan;
		sendData.nReActvPwrLmtPlan =   pA6_USYS->nReActvPwrLmtPlan;
		sendData.nBattGrpPwrLmtPlan =  pA6_USYS->nBattGrpPwrLmtPlan;
	}
	
	USHORT usDataLen = (USHORT)sizeof(sendData);
#ifdef LOG_FILEDEBUG
	static BYTE emBatt_WorkModePlan = BATT_WORKMODE_IDLE;
	static time_t stmLogFile = 0;
	if (ElapseS(stmLogFile) > INTV_S_LOG_DEBUGFILE || emBatt_WorkModePlan != sendData.emBatt_WorkModePlan)
	{
		stmLogFile = _NOW_;
		emBatt_WorkModePlan = sendData.emBatt_WorkModePlan;
		LOGFD("Send_A6USYS n:%d bEnter_AutoChrg_SEMS:%d bEnter_AutoDisc_SEMS:%d emEMSCtrlMode_REMS:%d "
			"emCCUState:%d emCtrlRectMode:%X A0:Auto emBatt_WorkModePlan:%d 5:idle nStopSOCPlan:%d nActvPwrLmtPlan:%u nBattGrpPwrLmtPlan:%u",
			n, pEMS->bEnter_AutoChrg_SEMS, pEMS->bEnter_AutoDisc_SEMS, pEMS->clientInfoSum.emEMSCtrlMode_REMS, 
			sendData.emUSYSStationEMSAvaiStatus, sendData.emCtrlRectMode, 
			sendData.emBatt_WorkModePlan, sendData.nStopSOCPlan, sendData.nActvPwrLmtPlan, sendData.nBattGrpPwrLmtPlan);
	}
#endif

	DoTCPSession_SEMS(n, NL_MSG_USYS, reverseBytes((BYTE*)&sendData, usDataLen), usDataLen, MAX_MS_DoTCPSession_SEMS_NoWaitREP);
}

static USHORT MakeNLChkSum(BYTE byCmd, USHORT usDataLen, BYTE* byData)
{
	UINT ulCalcSum = (UINT)byCmd;
	ulCalcSum += (BYTE)(usDataLen >> 8);
	ulCalcSum += (BYTE)(usDataLen);
	int n;
	for(n = 0; n < usDataLen; n++)
	{
		ulCalcSum += byData[n];
	}

	ulCalcSum &= 0xFFFF;
	return (USHORT)ulCalcSum;
}

static NLMSG_FRAME_INFO_SEMS* Find_NLMsgFrame_SEMS(IN ROUGH_CCU_DATA *pCCU, IN BYTE* pRaw, IN int nReadLen, OUT BYTE** ppDataPos, OUT USHORT* pusDataLen)
{
	UNUSED(pCCU);
	int n, m;
	BYTE* byPos = pRaw;
	// int nSOIPos = -1;
	// int nFindLen = 0;
	USHORT nParsedDataLen = 0;
	NLMSG_FRAME_INFO_SEMS *pFoundMsg = NULL;
	//Notice: only find the first frame, if have multi frames, ignore the followed others
	TRACEX("--------Now begin parsing frame, nReadLen = %d, StartBytes = [%s]--------\n", nReadLen, printableBytes(byPos, 3));
	for(n = 0; n < nReadLen - NLMSG_MIN_LEN + 1; n++)
	{
		if((byPos[n] == (BYTE)(NLMSG_SOI >> 16)) && 
			(byPos[n + 1] == (BYTE)(NLMSG_SOI >> 8)) &&
			(byPos[n + 2] == (BYTE)(NLMSG_SOI & 0xFF)))
		{
			//CMD;
			pFoundMsg = NULL;
			for(m = 0; m < NL_MSG_Max; m++)
			{
				if(s_SemsInfo.nlFrameInfoSEMS[m].byCmd == byPos[n + 3])
				{
					pFoundMsg = &s_SemsInfo.nlFrameInfoSEMS[m];
					TRACE("Had Found nlFrameInfo[%d].byCmd=[%02X]", n, s_SemsInfo.nlFrameInfoSEMS[m].byCmd);
					break;
				}
			}
			if(pFoundMsg == NULL)
			{
				TRACEX("-----Can NOT find such NLMSG=>%d----\n", byPos[n + 4]);
				continue;
			}
			//now check data length;
			nParsedDataLen = (USHORT)((byPos[n + 4] << 8) + byPos[n + 5]);
			if((nParsedDataLen > CCU_TCP_BUFFLEN_SEMS) || (nParsedDataLen < pFoundMsg->usMin_ExpectDataLen) ||
				(nParsedDataLen > nReadLen - NLMSG_MIN_LEN))
			{
				TRACEX("Found Msg[%02X] but the datalen[%d, %d] is INVALID\n", byPos[n + 3], 
					nParsedDataLen, pFoundMsg->usMin_ExpectDataLen);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}

			USHORT ulCalcSum = MakeNLChkSum(pFoundMsg->byCmd, nParsedDataLen, byPos + n + 6);
			USHORT ulRealSum = (USHORT)((USHORT)(byPos[n + 6 + nParsedDataLen] << 8) + (USHORT)byPos[n + 7 + nParsedDataLen]);
			TRACEX("----[%d]: The nParsedDataLen=>%d, usMin_ExpectDataLen=>%d, chkSum=>[%d, %d]----\n", 
				n, nParsedDataLen, pFoundMsg->usMin_ExpectDataLen, ulCalcSum, ulRealSum);

			//now check chksum
			if(ulCalcSum != ulRealSum)
			{
				TRACEX("Found Msg[%02X] but the chkSum[%d, %d] is DIFFERENT\n", byPos[n + 3], 
					ulCalcSum, ulRealSum);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}
			//EOI
			if(byPos[n + 8 + nParsedDataLen] != NLMSG_EOI)
			{
				TRACEX("Found Msg[%02X] but the EOI[%02X] is INVALID\n", byPos[n + 3], 
					byPos[n + 8 + nParsedDataLen]);
				//return NULL;
				pFoundMsg = NULL;
				continue;
			}

			TRACEX("-----Now we parsed a NLMsg=>%02X successfully!-----\n", pFoundMsg->byCmd);
			//now succeccfully get the frame
			*ppDataPos = byPos + n + 6;
			*pusDataLen = nParsedDataLen;
			break;
		}
	}

	if(pFoundMsg != NULL)
	{
		TRACEX("-----WWWWWWWWWWow, now find a new frame => %d, dataLen => %d-----\n", pFoundMsg->byCmd, nParsedDataLen);
	}		

	return pFoundMsg;
}

BOOL Wait_ACCU_TCPReply(int nWhichClient, NLMSG_FRAME_INFO_SEMS *pFramInfo, int nTimeOut_mSec)
{
	ROUGH_CCU_DATA *pCCU = s_MMClients[nWhichClient].pCCU;
	SAMPLER *pSampler = (SAMPLER *)pCCU->hSampler;

	BYTE byReceiveSendBuff[CCU_TCP_BUFFLEN_SEMS] = {0};
	int nReadBytes = 0;
	int nTotalReadBytes = 0;
	BYTE* pReadPos = byReceiveSendBuff;
	BYTE* pParsePos = pReadPos;
	NLMSG_FRAME_INFO_SEMS* pFrm = NULL;
	BYTE* pFrmData = NULL;
	USHORT usFrmDataLen = 0;
	// BOOL bHasFoundFrm = FALSE;
	UINT nWaitmSec = 0;
	int nLastErrorCode = 0;
	int nParseLen = 0;
	BOOL bHasMoreFrm = FALSE;
	BOOL bSuccess = FALSE;
	int nReadChunkDataCount = 0;
	BOOL bFoundResult = FALSE;
	do
	{
		nReadBytes = pSampler->pUsedCommPort->Read(s_MMClients[nWhichClient].hClientPort, (char*)pReadPos, CCU_TCP_BUFFLEN_SEMS - nTotalReadBytes);
		nTotalReadBytes += nReadBytes;
		nParseLen += nReadBytes;
		//in case the client is disconnected
		nLastErrorCode = 0;
		pSampler->pUsedCommPort->Control(s_MMClients[nWhichClient].hClientPort, 
				COMM_GET_LAST_ERROR, &nLastErrorCode, sizeof(int));
		if(nLastErrorCode == ERR_COMM_CONNECTION_BROKEN)
		{
			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, "Detected tcp client[%d] DISCONNECTED, so we need re-accept it!!!", nWhichClient);
			s_MMClients[nWhichClient].bDetected_ClientDisconnected = TRUE;
			return FALSE;
		}
		TRACEX("Wait_ACCU_TCPReply n:%d len:%d [%d]: ReadBytes=>[%d, %d], nLastErrorCode=>%d rdBuff<%s>",
			nWhichClient, nReadBytes, nTimeOut_mSec, nReadBytes, nTotalReadBytes, nLastErrorCode, printableBytes(pReadPos, nReadBytes));
		while(nParseLen >= NLMSG_MIN_LEN)
		{
			s_SemsInfo.nInHandlingClient = (UINT)nWhichClient;
			pFrm = Find_NLMsgFrame_SEMS(pCCU, pParsePos, nParseLen, &pFrmData, &usFrmDataLen);
			// printf("-----------ReadBytes=>[%d, %d], nParseLen=>%d, usFrmDataLen ->%d, FoundFrm=>%s-----------\n", 
			// 	nReadBytes, nTotalReadBytes, nParseLen, usFrmDataLen, (pFrm == NULL) ? "NO" : "YES");			
			if(pFrm != NULL)
			{
				TRACE("Wait_ACCU_TCPReply Find_NLMsgFrame_SEMS byCmd<%02X> n:%d", pFrm->byCmd, nWhichClient);
#ifdef _TRACE_PRINT_
				switch (pFrm->msgTp)
				{
				case NL_MSG_CFGG:
					TRACEI("Wait_ACCU_TCPReply CFGG n:%d len:%d data<%s>", nWhichClient, usFrmDataLen+NLMSG_MIN_LEN, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				case NL_MSG_UCTL:
					TRACEW("Wait_ACCU_TCPReply UCTL n:%d len:%d data<%s>", nWhichClient, usFrmDataLen+NLMSG_MIN_LEN, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				case NL_MSG_LSYS:
					TRACED("Wait_ACCU_TCPReply LSYS n:%d len:%d data<%s>", nWhichClient, usFrmDataLen+NLMSG_MIN_LEN, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				// case NL_MSG_LDEV:
				// 	TRACEI("Wait_ACCU_TCPReply LDEV n:%d len:%d data<%s>", nWhichClient, usFrmDataLen+NLMSG_MIN_LEN, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
				// 	break;

				default:
					// TRACED("Wait_ACCU_TCPReply n:%d len:%d data<%s>", nWhichClient, usFrmDataLen+NLMSG_MIN_LEN, printableBytes(pParsePos, usFrmDataLen+NLMSG_MIN_LEN));
					break;
				}
#endif

				// bHasFoundFrm = TRUE;	
				if (pFrm->msgTp == NL_MSG_LSYS ||
					pFrm->msgTp == NL_MSG_LDEV )
				{
					pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen, nWhichClient);
				}
				else if((pFrm == pFramInfo) && (pFramInfo != NULL))
				{
					bSuccess = (pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen, nWhichClient) == _RTN_OK) ? TRUE : FALSE;
					bFoundResult = TRUE;
				}
				else if(pFrm->msgTp == NL_MSG_CFGG)
				{
					pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen, nWhichClient);
				}
				else if(pFrm->msgTp == NL_MSG_PCSINFO)
				{
					pFrm->pfnParser(pCCU, pFrmData, usFrmDataLen, nWhichClient);
				}
				//find pos move
				pParsePos = pFrmData + usFrmDataLen + 3; //3 = 2byte-CheckSum + 1byte-EOI
				nParseLen -= (usFrmDataLen + NLMSG_MIN_LEN);
			}
			else
			{
				//printf("----[%.3f]: FOUND chunk data!----\n", GetCurrentTime());
				break;
			}
		}

		if(nParseLen > 0) //added by Jimmy 20220110 as to check chunk data	
		{
			bHasMoreFrm = TRUE;
		}
		else if(bFoundResult) //if in setting, we should skip if no more data
		{
			break;
		}

		pReadPos += nReadBytes;		
		if(nTotalReadBytes >= CCU_TCP_BUFFLEN_SEMS)
		{
			break;
		}

		Sleep_Ex(TCP_CLIENT_READ_MIN_INTERV_MSEC);
		nTimeOut_mSec -= TCP_CLIENT_READ_MIN_INTERV_MSEC;
		nWaitmSec += TCP_CLIENT_READ_MIN_INTERV_MSEC;
		if((nWaitmSec > 2000)/* && (!bHasFoundFrm)*/)
		{
			//in case it waits the IMSU-D will consider comm-fail
			TRACEX("----NOW neeed ReCHECK the USYS----\n");
			nWaitmSec = 0;
			if (ElapseMS(s_MMClients[nWhichClient].tmLastSentUSYS) >= 1000.0f)
			{
				Send_A6USYS(nWhichClient);
				s_MMClients[nWhichClient].tmLastSentUSYS = GetCurrentTime();
			}
		}

		if(bHasMoreFrm && (nTimeOut_mSec <= 0))
		{
			nReadChunkDataCount++;
			// printf("----[%.3f]: Read out ChUNNNNNK data for frm->[%d], nReadChunkDataCount = %d, nParseLen = %d, nTotalReadBytes = %d, nTimeOut_mSec = %d------\n",
			// 	GetCurrentTime(), (pFramInfo != NULL) ? pFramInfo->byCmd : -9999,
			// 	nReadChunkDataCount, nParseLen, nTotalReadBytes, nTimeOut_mSec);
			if(nReadChunkDataCount < 20) //read 1 sec more
			{
				nTimeOut_mSec = 1; //give it one more chance if read chunk data
			}
			else
			{
				break;
			}			
		}
	} while (nTimeOut_mSec > 0);
	TRACE("Wait_ACCU_TCPReply return:%d 1:TRUE", (pFramInfo != NULL) ? bSuccess : TRUE);
	return (pFramInfo != NULL) ? bSuccess : TRUE;
}

BOOL DoUDPSession_SEMS(int nWhichClient, NL_MSGTYPE emMsgTp, BYTE *pData, USHORT usDataLen, int nTimeOut_mSec)
{
	// printf("enter DoUDPSession_SEMS \n");
	TRACE("DoUDPSession_SEMS n:%d emMsgTp:%d byCmd:%02X usDataLen:%u nTimeOut_mSec:%d %s",
		nWhichClient, emMsgTp, s_SemsInfo.nlFrameInfoSEMS[emMsgTp].byCmd, usDataLen, nTimeOut_mSec, printableBytes(pData, usDataLen));
	ROUGH_CCU_DATA *pCCU = s_MMClients[nWhichClient].pCCU;
	if((s_MMClients[nWhichClient].emRunStatus == MMCLITNT_COMM_STAT_FAILED) ||
		(s_MMClients[nWhichClient].hClientPort == NULL))
	{
		TRACEE("DoUDPSession_SEMS return FALSE nWhichClient:%d hClientPort == NULL emRunStatus:%d hClientPort:%X\n",
			nWhichClient, s_MMClients[nWhichClient].emRunStatus, s_MMClients[nWhichClient].hClientPort);
		return FALSE;
	}
	if((nWhichClient < 0) || (nWhichClient > pCCU->emsMgmtData.nNodeNum))
	{
		TRACEE("DoUDPSession_SEMS return FALSE nWhichClient:%d nNodeNum:%d\n", nWhichClient, pCCU->emsMgmtData.nNodeNum);
		return FALSE;
	}
	// printf("DoUDPSession_SEMS 1\n");
	// SAMPLER *pSampler = (SAMPLER *)pCCU->hSampler;
    BYTE bySendBuff[CCU_TCP_BUFFLEN_SEMS];
	memset(bySendBuff, 0, CCU_TCP_BUFFLEN_SEMS);
	//int nSentLen = MakeNLMSG_SendFrame_SEMS(&s_SemsInfo.nlFrameInfoSEMS[emMsgTp], usDataLen, pData, bySendBuff);
	int nSentLen = MakeNLMSG_SendFrame(s_SemsInfo.nlFrameInfoSEMS[emMsgTp].byCmd, usDataLen, pData, bySendBuff);

	if (nTimeOut_mSec > MAX_MS_DoTCPSession_SEMS_NoWaitREP)
	{
		Mutex_Lock(s_MMClients[nWhichClient].hLock, WAIT_INFINITE);
		TRACED(">>>DoUDPSession_SEMS lock Mutex_Lock WAIT_INFINITE");
	}
	UDP_PORT_DRV tmp;
	memcpy(&tmp, s_SemsInfo.ClientsUdpInfo.hClientUDPPort, sizeof(UDP_PORT_DRV));

	struct sockaddr_in send_addr;
	memset(&send_addr, 0, sizeof(send_addr));
	send_addr.sin_family      = AF_INET ;
	send_addr.sin_port        = htons(10005);//htons((u_short)s_MMClients[nWhichClient].stClientInfo.nPeerPort);
	send_addr.sin_addr.s_addr = (*(ULONG *)s_MMClients[nWhichClient].stClientInfo.szAddr);
	tmp.send_addr = send_addr;

	memcpy(s_SemsInfo.ClientsUdpInfo.hClientUDPPort, &tmp, sizeof(UDP_PORT_DRV));
	int nWriteBytes = s_pUDPComm->Write(s_SemsInfo.ClientsUdpInfo.hClientUDPPort, (char*)bySendBuff, nSentLen);

	TRACED("DoUDPSession_SEMS 2 nWriteBytes[%d] sin_port:%x ip:%x s_addr:%s\n",
		nWriteBytes, send_addr.sin_port, *(ULONG *)s_MMClients[nWhichClient].stClientInfo.szAddr, 
		inet_ntoa(*(struct in_addr *)s_MMClients[nWhichClient].stClientInfo.szAddr));
//#ifdef _TRACE_PRINT_
	// UCTL
	BYTE byCmd = bySendBuff[3];
	//if (byCmd==0x03 && szBuff[6]>=0x31 && szBuff[6]<=0x34)
	//if (byCmd==0x03)
	if (byCmd==0x03 && bySendBuff[6]==0x31)
	{
		TRACED("-->DoUDPSession_SEMS byCmd<%02X> UCTL n:%d nSentLen<%d %d> byBuff<%s>\n",
			byCmd, nWhichClient, nWriteBytes, nSentLen, printableBytes(bySendBuff, nSentLen));
	}
	TRACED("-->DoUDPSession_SEMS byCmd<%02X> UCTL n:%d nSentLen<%d %d> byBuff<%s>\n",
		byCmd, nWhichClient, nWriteBytes, nSentLen, printableBytes(bySendBuff, nSentLen));
	// else
	// {
	// 	TRACED("-->DoUDPSession_SEMS byCmd<%02X> n:%d nSentLen<%d %d> bySendBuff<%s>",
	// 		byCmd, nWhichClient, nWriteBytes, nSentLen, printableBytes(bySendBuff, nSentLen));
	// }
//#endif
	s_MMClients[nWhichClient].tmLastSentCmd = GetCurrentTime();//udp通信是否需要更新发送时间？
	BOOL bRet = FALSE;
	if (nTimeOut_mSec > MAX_MS_DoTCPSession_SEMS_NoWaitREP)//6330
	{
		bRet = Wait_ACCU_TCPReply(nWhichClient, &s_SemsInfo.nlFrameInfoSEMS[emMsgTp], nTimeOut_mSec);
		Mutex_Unlock(s_MMClients[nWhichClient].hLock);
		TRACED("DoUDPSession_SEMS lock Mutex_Unlock >>>");
		// printf("DoUDPSession_SEMS 3\n");
	}
	else
	{
		TRACED("DoUDPSession_SEMS NL_MSG_USYS return TRUE Don't Wait_ACCU_TCPReply");
		bRet = TRUE;
		TRACED("DoUDPSession_SEMS 4\n");
	}
	return bRet;
}
BOOL DoTCPSession_SEMS(int nWhichClient, NL_MSGTYPE emMsgTp, BYTE *pData, USHORT usDataLen, int nTimeOut_mSec)
{
	TRACE("DoTCPSession_SEMS n:%d emMsgTp:%d byCmd:%02X usDataLen:%u nTimeOut_mSec:%d %s",
		nWhichClient, emMsgTp, s_SemsInfo.nlFrameInfoSEMS[emMsgTp].byCmd, usDataLen, nTimeOut_mSec, printableBytes(pData, usDataLen));
	ROUGH_CCU_DATA *pCCU = s_MMClients[nWhichClient].pCCU;
	if((s_MMClients[nWhichClient].emRunStatus == MMCLITNT_COMM_STAT_FAILED) ||
		(s_MMClients[nWhichClient].hClientPort == NULL))
	{
		TRACEE("DoTCPSession_SEMS return FALSE nWhichClient:%d hClientPort == NULL emRunStatus:%d hClientPort:%X",
			nWhichClient, s_MMClients[nWhichClient].emRunStatus, s_MMClients[nWhichClient].hClientPort);
		return FALSE;
	}
	if((nWhichClient < 0) || (nWhichClient > pCCU->emsMgmtData.nNodeNum))
	{
		TRACEE("DoTCPSession_SEMS return FALSE nWhichClient:%d nNodeNum:%d", nWhichClient, pCCU->emsMgmtData.nNodeNum);
		return FALSE;
	}

	SAMPLER *pSampler = (SAMPLER *)pCCU->hSampler;
    BYTE bySendBuff[CCU_TCP_BUFFLEN_SEMS];
	memset(bySendBuff, 0, CCU_TCP_BUFFLEN_SEMS);
	//int nSentLen = MakeNLMSG_SendFrame_SEMS(&s_SemsInfo.nlFrameInfoSEMS[emMsgTp], usDataLen, pData, bySendBuff);
	int nSentLen = MakeNLMSG_SendFrame(s_SemsInfo.nlFrameInfoSEMS[emMsgTp].byCmd, usDataLen, pData, bySendBuff);

	if (nTimeOut_mSec > MAX_MS_DoTCPSession_SEMS_NoWaitREP)
	{
		Mutex_Lock(s_MMClients[nWhichClient].hLock, WAIT_INFINITE);
		TRACED(">>>DoTCPSession_SEMS lock Mutex_Lock WAIT_INFINITE");
	}
	int nWriteBytes = pSampler->pUsedCommPort->Write(s_MMClients[nWhichClient].hClientPort, (char*)bySendBuff, nSentLen);
#ifdef _TRACE_PRINT_
	// UCTL
	BYTE byCmd = bySendBuff[3];
	//if (byCmd==0x03 && szBuff[6]>=0x31 && szBuff[6]<=0x34)
	if (byCmd==0x63)
	{
		TRACEI("-->DoTCPSession_SEMS Write byCmd<%02X> UCTL n:%d nSentLen<%d %d> byBuff<%s>",
			byCmd, nWhichClient, nWriteBytes, nSentLen, printableBytes(bySendBuff, nSentLen));
	}
	else
	{
		TRACED("-->DoTCPSession_SEMS Write byCmd<%02X> n:%d nSentLen<%d %d> bySendBuff<%s>",
			byCmd, nWhichClient, nWriteBytes, nSentLen, printableBytes(bySendBuff, nSentLen));
	}
#endif
	s_MMClients[nWhichClient].tmLastSentCmd = GetCurrentTime();
	BOOL bRet = FALSE;
	if (nTimeOut_mSec > MAX_MS_DoTCPSession_SEMS_NoWaitREP)
	{
		bRet = Wait_ACCU_TCPReply(nWhichClient, &s_SemsInfo.nlFrameInfoSEMS[emMsgTp], nTimeOut_mSec);
		Mutex_Unlock(s_MMClients[nWhichClient].hLock);
		TRACED("DoTCPSession_SEMS lock Mutex_Unlock >>>");
	}
	else
	{
		TRACED("DoTCPSession_SEMS NL_MSG_USYS return TRUE Don't Wait_ACCU_TCPReply");
		bRet = TRUE;
	}
	return bRet;
}

// BOOL DoTCP_S_UDIAG(BYTE* pData, int nDataLen)
// {
// 	//will implement it later
// 	return TRUE;
// }

// BOOL DoTCP_S_UDEBUG(BYTE* pData, int nDataLen)
// {
// 	//will implement it later
// 	return TRUE;
// }

// static void Update_CCUFirmwareUpdate_Status(int nWhichClient, int nState)
// {
// 	s_MMClients[nWhichClient].emCCUTCP_UpgradeStatus = nState;
// 	VAR_VALUE varV;
// 	varV.emValue = nState;
// 	Push_VarValue(SETTYPE_RAW_SP, s_MMClients[nWhichClient].pCCU->hSampler, 181 + nWhichClient, 130, varV);	
// }

// BOOL Notify_CCU_EnterUpgrade(int nWhichClient, char *szCCUBinfileName)
// {
// 	ROUGH_CCU_DATA *pCCU = s_MMClients[nWhichClient].pCCU;

// 	char szFile[255];
// 	snprintf(s_MMClients[nWhichClient].szCCUBinFileName, 64, "%s",  szCCUBinfileName);
// 	snprintf(szFile, 255, "%s%s", CCU_FIRMWARE_UPGRADE_PATH, s_MMClients[nWhichClient].szCCUBinFileName);	
// 	//added by Jimmy 20221129
// 	if(strstr(szCCUBinfileName, LOCAL_FIRMWARE_NAME_CCU_ONLY) != NULL)
// 	{
// 		snprintf(szFile, 255, "%s", LOCAL_FIRMWARE_NAME_CCU_ONLY);	
// 	}
	
// 	//another update is in process, so we need quit
// 	if((s_MMClients[nWhichClient].emCCUTCP_UpgradeStatus != CCU_TCP_UPGRADESTATUS_IDLE) &&
// 		(s_MMClients[nWhichClient].emCCUTCP_UpgradeStatus != CCU_TCP_UPGRADESTATUS_FINISHED))
// 	{
// 		printf("-------SSSSSSSSSStatus ERRRR11111--------\n");
// 		// unlink(szFile);
// 		return FALSE;
// 	}

// 	Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
	
// 	s_MMClients[nWhichClient].nBinFile_TotalBytes = 0;
// 	SAFELY_DELETE(s_MMClients[nWhichClient].pbyBinBuffer);

// 	//now read-out the bin file into memory	
// 	FILE*fp = fopen(szFile,"rb");
// 	if(fp == NULL)
// 	{
// 		printf("Can NOT open the file[%s]\n", szFile);
// 		// unlink(szFile);
// 		return FALSE;
// 	}

// 	fseek(fp, 0L, SEEK_END);
// 	size_t nFilelen = ftell(fp);
// 	if((nFilelen < IMSU_MIN_BIN_SIZE) || (nFilelen > IMSU_MAX_BIN_SIZE))
// 	{
// 		printf("FileLen[%d] is incorrect\n", nFilelen);
// 		fclose(fp);
// 		// unlink(szFile);
// 		return FALSE;
// 	}
	
// 	//now allocate memory for the file 
// 	BYTE *pBuff = NEW(BYTE, nFilelen);
// 	if(pBuff == NULL)
// 	{
// 		printf("CAN NOT allocate memory for FileLen[%d]\n", nFilelen);
// 		fclose(fp);
// 		// unlink(szFile);
// 		return FALSE;
// 	}

// 	printf("--------now readbuff from bin, size=>%d----------\n", nFilelen);
// 	//DO NOT forget to go back to file head!!!
// 	fseek(fp, 0L, SEEK_SET);
// 	//now read all data into memory
// 	fread(pBuff, nFilelen, 1, fp); 
// 	fclose(fp);
// 	s_MMClients[nWhichClient].pbyBinBuffer = pBuff;
// 	s_MMClients[nWhichClient].nBinFile_TotalBytes = nFilelen;
// 	s_MMClients[nWhichClient].nTransferred_Pkg = 0;	
// 	s_MMClients[nWhichClient].nUsed_SingleFrameSize = DEFAULT_FRMAESIZE_FOR_CCUUPDATE;
// 	UINT nTotal_Pkg = (nFilelen / s_MMClients[nWhichClient].nUsed_SingleFrameSize);
// 	if((nFilelen % s_MMClients[nWhichClient].nUsed_SingleFrameSize) > 0)
// 	{
// 		nTotal_Pkg += 1;
// 	}
// 	s_MMClients[nWhichClient].nTotal_Pkg2Transfer = nTotal_Pkg;
// 	s_MMClients[nWhichClient].byLastUpdate_Percent = 255;
// 	Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_WAIT_FILETRANS);
	
// 	printf("----Now prepare CCU[%d]-bin OK, bytes=>%d, totalPkg=>%d----\n",
// 		nWhichClient, nFilelen, nTotal_Pkg);

// 	// unlink(szFile);
// 	return TRUE;
// }

// static void Notify_UI_IMSUUpgrage_Percent(int nWhichClient, int nPercent)
// {
// 	float fPercent = (float)nPercent;
// 	if(nPercent == -1)
// 	{
// 		fPercent = (float)100 * s_MMClients[nWhichClient].nTransferred_Pkg / s_MMClients[nWhichClient].nTotal_Pkg2Transfer;
// 		nPercent = (int)fPercent;
// 	}
	
// 	//do not send when no update
// 	if(s_MMClients[nWhichClient].byLastUpdate_Percent == (BYTE)nPercent)
// 	{
// 		//printf("------EEEEEEEEEEEEE, %.2f, Trans[%d, %d]----\n", GetCurrentTime(), 
// 		//	s_stCCUTCP_RunRough.nTransferred_Pkg, s_stCCUTCP_RunRough.nTotal_Pkg2Transfer);
// 		return;
// 	}

// 	//printf("------%.2f, Trans[%d, %d], nPercent=>%d----\n", GetCurrentTime(), 
// 	//		s_stCCUTCP_RunRough.nTransferred_Pkg, s_stCCUTCP_RunRough.nTotal_Pkg2Transfer, nPercent);

// 	s_MMClients[nWhichClient].byLastUpdate_Percent = nPercent;
// 	EVENT_UI_NOTICE evNote;
// 	evNote.emNotoceType = UI_NOTICETYPE_IMSU_UPDATE;
// 	evNote.byData[0] = nPercent; //0~100 means percent, 241 means fail, 242 means success
// 	evNote.byData[1] = nWhichClient + 1;
// 	SendMsg((SAMPLER*)s_MMClients[nWhichClient].pCCU->hSampler, EVENT_TYPE_UI_NOTICE, 
// 		EVENT_MSG_BEGIN, &evNote, sizeof(EVENT_UI_NOTICE));
	
// 	//now judge if need log
// 	static time_t s_tmLastRecord_Percent[MAX_SEMS_CLIENTS_NUM] = {0};
// 	time_t tmNow = _NOW_;
// 	if((ABS(tmNow - s_tmLastRecord_Percent[nWhichClient]) >= 30) || (fPercent > 99.999))
// 	{
// 		s_tmLastRecord_Percent[nWhichClient] = tmNow;
// 		if(nPercent <= 100)
// 		{
// 			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_INFO, 
// 				"CCU[%d] firmware upgrade pending, percent => %d...", nWhichClient, nPercent);				
// 		}
// 		else 
// 		{
// 			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_INFO, 
// 				"CCU[%d] firmware upgrade done, result => [%s]!!!", nWhichClient, 
// 				(nPercent == 242) ? "SUCCESSS" : "FAILED");				
// 		}
// 	}
// }

// BOOL DoTCP_CCU_Upgrade(int nWhichClient)
// {
// 	//printf("----now enter the CCU upgrade process----\n");
// 	ROUGH_CCU_DATA *pCCU = s_MMClients[nWhichClient].pCCU;
// 	//judge if bin file prepared
// 	if(s_MMClients[nWhichClient].nBinFile_TotalBytes < 512)
// 	{
// 		Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
// 		SAFELY_DELETE(s_MMClients[nWhichClient].pbyBinBuffer);
// 		LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 				"Can NOT start CCU[%d] upgrade as NO bin file found!!!", nWhichClient);	
// 		return FALSE;
// 	}

// 	//added by Jimmy for set the CCU=>Unavailble so that not interrupted by other
// 	VAR_VALUE varV;
// 	varV.emValue = CCU_STATE_LOCKED; 
// 	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 4, varV);
// 	varV.emValue = SYSFAIL_REASON_OTHER;
// 	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 6, varV);

// 	int nn;
// 	BOOL bIsCommOK = FALSE;
// 	Notify_UI_IMSUUpgrage_Percent(nWhichClient, 0);
// 	//wait 3.3 * 9 = 30s for connection
// 	double tmf_Now = GetCurrentTime();
// 	for(nn = 0; nn < 9; nn++)
// 	{
// 		Notify_UI_IMSUUpgrage_Percent(nWhichClient, 0);
// 		if(!s_MMClients[nWhichClient].bCommFail)
// 		{
// 			bIsCommOK = TRUE;
// 			break;
// 		}
// 	}
	
// 	if(!bIsCommOK)
// 	{
// 		//printf("----CCU discconnected so QUIT the upgrade------\n");
// 		Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
// 		SAFELY_DELETE(s_MMClients[nWhichClient].pbyBinBuffer);
// 		LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 				"CCU[%d] comm fail so QUIT the upgrade!!!", nWhichClient);	
// 		Notify_UI_IMSUUpgrage_Percent(nWhichClient, 241);	
// 		return FALSE;
// 	}

// 	//first notice LCCU to enter upgrade mode
// 	BYTE byData[512] = {0};
// 	BYTE* pbyPos = byData;
// 	USHORT us2BeSentDataLen = 0;
// 	//------total bytes
// 	varV.ulValue = s_MMClients[nWhichClient].nBinFile_TotalBytes;
// 	//memcpy(pbyPos, varV.abyValue, 4);
// 	pbyPos[0] = (BYTE)(s_MMClients[nWhichClient].nBinFile_TotalBytes >> 24);
// 	pbyPos[1] = (BYTE)(s_MMClients[nWhichClient].nBinFile_TotalBytes >> 16);
// 	pbyPos[2] = (BYTE)(s_MMClients[nWhichClient].nBinFile_TotalBytes >> 8);
// 	pbyPos[3] = (BYTE)s_MMClients[nWhichClient].nBinFile_TotalBytes;
// 	us2BeSentDataLen += 4;
// 	pbyPos += 4;
// 	//-------MD5 0x00* 128
// 	memset(pbyPos, 0, 128);
// 	us2BeSentDataLen += 128;
// 	pbyPos += 128;
// 	//-------Max allow frm size
// 	pbyPos[0] = (BYTE)(s_MMClients[nWhichClient].nUsed_SingleFrameSize >> 8);
// 	pbyPos[1] = (BYTE)s_MMClients[nWhichClient].nUsed_SingleFrameSize;
// 	us2BeSentDataLen += 2;
// 	pbyPos += 2;
// 	//------Package total
// 	pbyPos[0] = (BYTE)(s_MMClients[nWhichClient].nTotal_Pkg2Transfer >> 8);
// 	pbyPos[1] = (BYTE)s_MMClients[nWhichClient].nTotal_Pkg2Transfer;
// 	us2BeSentDataLen += 2;
// 	pbyPos += 2;

// 	//now send enter-upgrade cmd => CCU 5 times (2* 5s)
// 	int n;
// 	int nTimeOut_mSec = 2000;
// 	BOOL bStartedOK = FALSE;
// 	for(n = 0; n < 5; n++)
// 	{
// 		//printf("---[%.2f]: Now sendout the UFRM msg => CCU---\n", GetCurrentTime());
// 		if(DoTCPSession_SEMS(nWhichClient, NL_MSG_UFRM, byData, us2BeSentDataLen, nTimeOut_mSec))
// 		{
// 			bStartedOK = TRUE;
// 			break;
// 		}		
// 	}
// 	if(!bStartedOK)
// 	{
// 		//printf("----Failed to start CCU upgrade------\n");
// 		Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
// 		SAFELY_DELETE(s_MMClients[nWhichClient].pbyBinBuffer);
// 		LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 				"CCU[%d] firmware upgrade start FAILED!!!", nWhichClient);	
// 		Notify_UI_IMSUUpgrage_Percent(nWhichClient, 241);		
// 		return FALSE;
// 	}

// 	LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 				"CCU[%d] firmware upgrade started OK!!!", nWhichClient);	
	
// 	//now wait and hanle file sending
// 	Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_PENDING_FILETRANS);	
// 	//Notify_UI_IMSUUpgrage_Percent(0);
// 	int nWaitMax_mSec = 1200*1000; //max wait 20min
// 	int nNoResponseCouter = 0;
// 	while (nWaitMax_mSec > 0)
// 	{
// 		RUN_THREAD_HEARTBEAT();
// 		Notify_UI_IMSUUpgrage_Percent(nWhichClient, -1);
// 		Wait_ACCU_TCPReply(nWhichClient, NULL, 0);
// 		BOOL bIsNeedReconect = FALSE;
// 		if(ABS(_NOW_ - s_MMClients[nWhichClient].tmLastReceiveCCUUpgdadeCmd) > 10)
// 		{
// 			nNoResponseCouter++;
// 			if(nNoResponseCouter > 100)
// 			{
// 				nNoResponseCouter = 0;
// 				bIsNeedReconect = TRUE;
// 			}
// 		}
// 		else
// 		{
// 			nNoResponseCouter = 0;
// 		}

// 		if(bIsNeedReconect)
// 		{
// 			Sleep(3000);
			
// 			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 				"OOPs! TCP[%d] DISCONNETED while transferring, now quit the process => %d!!!",
// 				nWhichClient, nNoResponseCouter);	
// 			SAFELY_DELETE(s_MMClients[nWhichClient].pbyBinBuffer);
// 			Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
// 			return FALSE;							
// 		}

// 		if(s_MMClients[nWhichClient].emCCUTCP_UpgradeStatus == CCU_TCP_UPGRADESTATUS_FAILED_FILETRANS)
// 		{
// 			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 				"CCU[%d] TCP file transfer FALIED!!!", nWhichClient);	
// 			Notify_UI_IMSUUpgrage_Percent(nWhichClient, 241);
// 			break;
// 		}
// 		else if(s_MMClients[nWhichClient].emCCUTCP_UpgradeStatus == CCU_TCP_UPGRADESTATUS_SUCCESS_FILETRANS)
// 		{
// 			//now wait CCU to be reset and wait CFGG
// 			LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_INFO, 
// 				"CCU[%d] TCP file transfer SUCCESSFULLY!!!", nWhichClient);	
// 			Notify_UI_IMSUUpgrage_Percent(nWhichClient, 242);
// 			break;
// 		}
// 		nWaitMax_mSec -= TCP_CLIENT_READ_MIN_INTERV_MSEC;
// 	}	
	
// 	//now get CFGG so that to consider the upgrade done OK
// 	SAFELY_DELETE(s_MMClients[nWhichClient].pbyBinBuffer);
// 	Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_FINISHED);

// 	Sleep(10000);
// 	//now wait CFGG
// 	nWaitMax_mSec = 45*1000; //max wait 45s
// 	do
// 	{
// 		RUN_THREAD_HEARTBEAT();
// 		Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
// 		// LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_INFO, 
// 		// 	"***CCU[%d] TCP firmware upgraded SUCCESSFULLY to version=>[%s]!!!", 
// 		// 	nWhichClient, pCCU->blobChargerSpecificSett[nWhichClient].ccuSettings.szSWVer);	
// 		return TRUE;
		
// 		nWaitMax_mSec -= 2000;
// 	} while (nWaitMax_mSec > 0);
	
// 	Update_CCUFirmwareUpdate_Status(nWhichClient, CCU_TCP_UPGRADESTATUS_IDLE);
	
// 	LogOut(MAIN_MODULE_SAMPLER, "CCU_TCPSamp", LOG_TYPE_ERROR, 
// 		"CCU[%d] TCP firmware upgraded FAILED!!!", nWhichClient);	

// 	return FALSE;
// }

