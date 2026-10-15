// #define _TRACE_PRINT_
#include "rs485_whim_sampler_main.h"

//static WH_FRAME_INFO			s_whFrames[_MAX_WH_FRAME_TYPE];
//static BOOL					s_bNeedReconfig; //first time do it

/*--------------------------------------------*/
/*  ChgAToHex: Convert array to hexa value. */
/* example: array="12345678af",num=5
after calling  a_to_hexa(array,hexa,num)
hexa={0x12 + 0x33,0x34 + 0x33,0x56 + 0x33,0x78 + 0x33,0xaf + 0x33}
*/
/*--------------------------------------------*/
static BYTE* ChgASCToWhHex(char*array, BYTE *hexa, int num)
{
	int  i;
	BYTE byTemp;

	BYTE* pOrig = hexa;

	for(i=0; i< (num*2); i++) 
	{
		if( *(array + i) > '9' )
			byTemp = *(array + i) - 'A' + 10;
		else
			byTemp = *(array + i) - '0';
		if (i % 2)
			hexa[i/2] |= byTemp;
		else
			hexa[i/2] = byTemp << 4;
	}
	for(i = 0; i < num; i++)
	{
		hexa[i] += 0x33;
	}
	return pOrig;
}

#define  INIT_AWH_FRAME(pFrame, _type, _ctrlCode, _sendDataLen, _headerLen, _pHeader, _DataLen, _nPointNum) \
		do{ \
			(pFrame)->frameType = (_type); \
			(pFrame)->sendFrame.byCtrlCode = (_ctrlCode); \
			(pFrame)->sendFrame.byDataLen = (_sendDataLen); \
			(pFrame)->nFrameLen = (_sendDataLen + 12); \
			(pFrame)->sendFrame.pbyData = NEW(BYTE, _sendDataLen); \
			ChgASCToWhHex((_pHeader), (pFrame)->sendFrame.pbyData, (_headerLen)); \
			(pFrame)->nDataLen = (_DataLen); \
			(pFrame)->nPointNum = (_nPointNum); \
			(pFrame)->nExpectAnswerLen = (_DataLen) + 16; \
			(pFrame)->sendFrame.byFrameSOI = 0x68; \
			(pFrame)->sendFrame.byFrameSOI_2 = 0x68; \
			(pFrame)->sendFrame.byEOI = 0x16; \
			memset((pFrame)->sendFrame.byAddrA0_A5, 0xAA, 6); \
			(pFrame)++; \
		}while(0)

//_WordLen 寄存器个数
#define  INIT_MD_FRAME(pFrame, _type, _VarVal_Idx, _ctrlCode, _Register_StartAddr, _SignalNum, _WordLen , _Val_OffSet) \
		do{ \
			(pFrame)->frameType = (_type); \
			(pFrame)->nVarVal_Idx = (_VarVal_Idx); \
			(pFrame)->nSignalNum = (_SignalNum); \
			(pFrame)->nWordLen = (_WordLen); \
			(pFrame)->nVal_OffSet = (_Val_OffSet); \
			(pFrame)->sendFrame.byFunCode = (_ctrlCode); \
			(pFrame)->sendFrame.byStartAddr[0] = (BYTE)(_Register_StartAddr >> 8); \
			(pFrame)->sendFrame.byStartAddr[1] = (BYTE)(_Register_StartAddr & 0xFF); \
			(pFrame)->sendFrame.byRegisterNum[0] = (BYTE)((_SignalNum * _WordLen)>> 8); \
			(pFrame)->sendFrame.byRegisterNum[1] = (BYTE)((_SignalNum * _WordLen) & 0xFF); \
			memset((pFrame)->sendFrame.usChkSum, 0x0, 2); \
			(pFrame)->nExpectAnswerLen = (_SignalNum * _WordLen * 2); \
			(pFrame)++; \
		}while(0)
static void Make_CS(WH_FRAME_INFO* pFrame)
{
	ULONG ulSum = 0;
	BYTE* pByArr = (BYTE*)&pFrame->sendFrame; //Note: this needs #pragma pack(1), or else will be problems!!!
	BYTE byTobeCalcBytes[4096];
	memcpy(byTobeCalcBytes, pByArr, 10);
	int nDataLen = pFrame->nFrameLen - 12;
	memcpy(byTobeCalcBytes + 10, pFrame->sendFrame.pbyData, nDataLen);

	int nCalcLen = pFrame->nFrameLen - 2; 
	int n;
	for(n = 0; n < nCalcLen; n++)
	{
		ulSum+= byTobeCalcBytes[n];
	}
	//return (BYTE)ulSum;
	pFrame->sendFrame.byChkSum = (BYTE)(ulSum & 0xFF);
}

void whMeter_InitRoughData(RS485_SAMPLER_DATA *pWhim_Rough)
{
	//Now init the frames
	WH_FRAME_INFO *pFrm = pWhim_Rough->frm_whFrames;
	//INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_SET_ADDR,			0x15,	0x06,	6,	"01AAAAAAAAAA",	0);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_ADDR,			0x11,	0x04,	4,	"01040004",		5,	0);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALWH,		0x11,	0x04,	4,	"00000100",		4,	2);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_RVSTATALWH,	0x11,	0x04,	4,	"00000200",		4,	2);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALWH_AC,		0x11,	0x04,	4,	"00000100",		4,	2);
	
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_VOLT_PHASE_A,	0x11,	0x04,	4,	"00FF0102",		2,	1);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_VOLT_PHASE_B,	0x11,	0x04,	4,	"00020102",		2,	1);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_VOLT_PHASE_C,	0x11,	0x04,	4,	"00030102",		2,	1);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURR_PHASE_A,	0x11,	0x04,	4,	"00FF0202",		3,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURR_PHASE_B,	0x11,	0x04,	4,	"00020202",		3,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURR_PHASE_C,	0x11,	0x04,	4,	"00030202",		3,	3);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALKVAR_AC,	0x11,	0x04,	4,	"00000300",		4,	2);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_TOTALKVAR2_AC,0x11,	0x04,	4,	"00000400",		4,	2);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_ACTPOWER_AC,	0x11,	0x04,	4,	"00FF0302",		3,	4);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_REACTPOWER_AC,0x11,	0x04,	4,	"00FF0402",		3,	4);
  	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_POWERFACTOR,	0x11,	0x04,	4,	"00FF0602",		2,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_FREQ,			0x11,	0x04,	4,	"02008002",		2,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_PMAXDEMAND,	0x11,	0x04,	4,	"00000101",		3,	4);

	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH,	0x11,	0x04,	4,	"01FF0100",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH,	0x11,	0x04,	4,	"01FF0200",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH,	0x11,	0x04,	4,	"02FF0100",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH,	0x11,	0x04,	4,	"02FF0200",		4,	3);
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_TOTALWH,		0x11,	0x04,	4,	"00FF0100",		4,	2);//add by lpl 240725 for getting current SPFW kwh
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_MULTI_RVSTATALWH,		0x11,	0x04,	4,	"00FF0200",		4,	2);//add by lpl 240725 for getting current SPFW kwh
	INIT_AWH_FRAME(pFrm,	WH_FRAME_TYPE_GET_CURRENTPOSDEMAND,		0x11,	0x04,	4,	"04008002",		3,	4);//当前有功需量
 
 	YD_MD_FRAME_INFO *pYD_MD_Frm = pWhim_Rough->frm_YD_MD_whFrames;//YDS60-80
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_QUICKLY_ACTPOWER_AC,	 	WHMETER_ROUGH_RTACTPower, 		0x03, 	2264,	4,	2,	1000);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_CURR,	 					WHMETER_ROUGH_RT_CURR_A, 		0x03, 	2102,	3,	2,	1);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_VOLT,	 					WHMETER_ROUGH_RT_VOLT_A, 		0x03, 	2110,	3,	2,	1);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_QUICKLY_REACTPOWER,			WHMETER_ROUGH_RTReActPower, 	0x03, 	2272,	4,	2,	1000);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_QUICKLY_FACTOR,				WHMETER_ROUGH_RT_PowerFactor, 	0x03, 	2280,	4,	2,	1);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_FREQ,	 					WHMETER_ROUGH_RT_Freq, 			0x03, 	2124,	1,	2,	1);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_TOTALWH,	 				WHMETER_ROUGH_RTkWh_AC, 		0x03, 	2166,	1,	2,	1);
	INIT_MD_FRAME(pYD_MD_Frm, 	YD_GET_TYPE_RVSTATALWH,	 				WHMETER_ROUGH_RT_RVSkWh_AC, 	0x03, 	2174,	1,	2,	1);

	PHY_INTER_TYPE phyIf = RS485_1;
	for (phyIf = RS485_1; phyIf < _MAX_PHY_INTER_TYPE; phyIf++)
	{
		ACREL_MD_FRAME_INFO *pACREL_MD_Frm = &pWhim_Rough->frm_ACREL_MD_whFrames[phyIf][ACREL_GET_TYPE_VOLT];//ACREL-ADL400
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_VOLT,	 			WHMETER_ROUGH_RT_VOLT_A, 			0x03, 	8448,	3,	2,	1);//2100H 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_CURR,	 			WHMETER_ROUGH_RT_CURR_A, 			0x03, 	8460,	3,	2,	1);//210CH 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_ACTPOWER_AC,	 		WHMETER_ROUGH_ACTPower_PhA, 		0x03, 	8468,	4,	2,	1);//2114H 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_REACTPOWER,			WHMETER_ROUGH_ReActPower_PhA, 		0x03, 	8476,	4,	2,	1);//211CH 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_FACTOR,				WHMETER_ROUGH_PowerFactor_PhA, 		0x03, 	8492,	4,	2,	1);//212CH 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_FREQ,	 			WHMETER_ROUGH_RT_Freq, 				0x03, 	8500,	1,	2,	1);//2134H 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_SUM_RTkWh_AC,		WHMETER_ROUGH_RTkWh_AC, 			0x03, 	12292,	1,	4,	1);//正向有功电能 3004H 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_SUM_RVSkWh_AC,		WHMETER_ROUGH_RT_RVSkWh_AC, 		0x03, 	12296,	1,	4,	1);//反向电能 3008H 快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_TOTALKVAR,	 		WHMETER_ROUGH_RTkvar, 				0x03, 	12300,	1,	4,	1);////无功电能 300CH  快速
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_MAX_POS_DEMAND,		WHMETER_ROUGH_PMAXDEMAND, 			0x03, 	2106,	1,	2,	1);//083AH
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_CURRENTPOSDEMAND,	WHMETER_ROUGH_CURRENTPOSDEMAND, 	0x03, 	408,	1,	1,	1000);//0198H
		INIT_MD_FRAME(pACREL_MD_Frm, 	ACREL_GET_TYPE_SUM_RTkWh_RVSkWh_AC,	WHMETER_ROUGH_SUM_RTkWh_AC, 		0x03, 	2124,	10,	2,	10);//084CH
	}

	for (phyIf = RS485_1; phyIf < _MAX_PHY_INTER_TYPE; phyIf++)
	{
		ACREL_MD_FRAME_INFO *pDTSD_MD_Frm = &pWhim_Rough->frm_DTSD_MD_whFrames[phyIf][DTSD_GET_TYPE_VOLT];//DTSD1352
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_VOLT,	 				WHMETER_ROUGH_RT_VOLT_A, 			0x03, 	0x0061,	3,	1,	10);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_CURR,	 				WHMETER_ROUGH_RT_CURR_A, 			0x03, 	0x0064,	3,	1,	100);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_ACTPOWER_AC,	 		WHMETER_ROUGH_ACTPower_PhA, 		0x03, 	0x0164,	4,	2,	1000);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_REACTPOWER,			WHMETER_ROUGH_ReActPower_PhA, 		0x03, 	0x016c,	4,	2,	1000);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_FACTOR,				WHMETER_ROUGH_PowerFactor_PhA, 		0x03, 	0x017c,	4,	1,	1000);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_FREQ,	 				WHMETER_ROUGH_RT_Freq, 				0x03, 	0x0077,	1,	1,	100);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_SUM_RTkWh_AC,			WHMETER_ROUGH_RTkWh_AC, 			0x03, 	0x000A,	1,	2,	100);//正向有功电能
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_TOTALKVAR,	 		WHMETER_ROUGH_RTkvar, 				0x03, 	0X0028,	1,	2,	100);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_MAX_POS_DEMAND,		WHMETER_ROUGH_PMAXDEMAND, 			0x03, 	0x007B,	1,	1,	1000);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_CURRENTPOSDEMAND,		WHMETER_ROUGH_CURRENTPOSDEMAND, 	0x03, 	0x0198,	1,	1,	1000);//
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_SUM_RTkWh_RVSkWh_AC,	WHMETER_ROUGH_SUM_RTkWh_AC, 		0x03, 	0x000A,	10,	2,	100);
		INIT_MD_FRAME(pDTSD_MD_Frm, 	DTSD_GET_TYPE_SUM_RVSkWh_AC,		WHMETER_ROUGH_RT_RVSkWh_AC, 		0x03, 	0x0014,	1,	2,	100);//反向电能
	}

	int n;
	int kkk;
	SAMPLER* pSampler = (SAMPLER*)pWhim_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	UINT nDeviceID = 0;
	for( kkk = 0; kkk < MAX_WH_NUM; kkk++)
	{
		nDeviceID = All_TypesMetersID[kkk / METER_TYPE_NUM][kkk % METER_TYPE_NUM];
		if (nDeviceID == 0)
		{
			continue;
		}
		for(n = 0; n < _MAX_WH_ROUGH; n++)
		{
			pWhim_Rough->meterData[kkk].varWHMeterRough[n].lValue = SAMP_INVALID_VALUE;
		}
		//only get 410 power for Sampler B
		// if((pWhim_Rough->nSamplerId != 3) && (kkk > 0))
		// {
		// 	continue;
		// }
		pWhim_Rough->meterData[kkk].nSampleTimes = 0;		
		//Meter SN
		GetMeterSN(pWhim_Rough, kkk, NULL);
		VAR_VALUE varV;
		//Meter install type
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ nDeviceID, 401, &varV, 4);
		pWhim_Rough->meterData[kkk].emMeterType = (METER_TYPE)varV.emValue;

		//AC power factor
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ nDeviceID, 402, &varV, 4);
		pWhim_Rough->meterData[kkk].fACMeter_PowerFactor = varV.fValue;
		//AC meter protocol type
		varV.emValue = ACMETER_PROTOCOL_GBDLT645; //default is this setting
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ nDeviceID, 404, &varV, 4);
		pWhim_Rough->meterData[kkk].emACMeter_ProtocolType = varV.emValue;
		//Baudrate of the kwhMeter, newly added 20190226
		varV.emValue = 0;
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ nDeviceID, 405, &varV, 4);
		if(varV.emValue == 0) //auto set by DC/AC, to be compatible with marchet exsiting kwhMeter
		{
			if((pWhim_Rough->meterData[kkk].emMeterType == METER_TYPR_ac) &&
				(pWhim_Rough->meterData[kkk].emACMeter_ProtocolType  == ACMETER_PROTOCOL_MODBUS_EM737))
			{
				snprintf(pWhim_Rough->meterData[kkk].szBaudrateSett, 32, "2400,E,8,1");
			}
			else if((pWhim_Rough->meterData[kkk].emMeterType == METER_TYPR_ac) &&
				(pWhim_Rough->meterData[kkk].emACMeter_ProtocolType  == ACMETER_PROTOCOL_MODBUS_RTU_YD))
			{
				snprintf(pWhim_Rough->meterData[kkk].szBaudrateSett, 32, "9600,N,8,1");
				//TRACE("Init kkk=[%d] szBaudrateSett=[%s]\n", kkk, pWhim_Rough->meterData[kkk].szBaudrateSett);
			}
			else
			{
				snprintf(pWhim_Rough->meterData[kkk].szBaudrateSett, 32, "9600,E,8,1");
			}
		}
		else
		{
			if(varV.emValue == 3)
			{
				snprintf(pWhim_Rough->meterData[kkk].szBaudrateSett, 32, "9600,N,8,1");
				//TRACE("kkk=[%d] szBaudrateSett=[%s]\n", kkk, pWhim_Rough->meterData[kkk].szBaudrateSett);
			}
			else if(varV.emValue == 1)
			{
				snprintf(pWhim_Rough->meterData[kkk].szBaudrateSett, 32, "9600,E,8,1");
			}
			else
			{
				snprintf(pWhim_Rough->meterData[kkk].szBaudrateSett, 32, "2400,E,8,1");
			}
		}
		//AC meter Volt PT add by lpl 240627
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ nDeviceID, 407, &varV, 4);
		pWhim_Rough->meterData[kkk].fACMeter_VoltPTFactor = varV.fValue;
		pDPR->Get(pWhim_Rough->hSampler, GETTYPE_ST_VAR,
			/*((pWhim_Rough->nSamplerId == 3) ? 408 : 410)*/ nDeviceID, 408, &varV, 4);
		pWhim_Rough->meterData[kkk].bPhysicalInterType = (PHY_INTER_TYPE)varV.emValue;
	}

	//John 2023-6-27 added for AllMeter CommFail, Reboot.
	pWhim_Rough->nAllCommFailCounter = 0;
	pWhim_Rough->bLast_AllMeterCommOK = FALSE;

	return;
}

static float ConvertBCD_2WhVal(BYTE* pBCD, WH_FRAME_INFO *pFrameInfo)
{
	float fOut = 0.0;

	//--- kWh had no sign ---/
	if((pFrameInfo->frameType == WH_FRAME_TYPE_GET_TOTALWH) 
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_RVSTATALWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
	|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH)) 
	{
		fOut = (float)((pBCD[3] >> 4)      *100000);
		//fOut = (float)(((pBCD[3] >> 4) & 0x07)      *100000);
		fOut += (float)((pBCD[3] & 0x0F) *10000);
		fOut += (float)((pBCD[2] >> 4) *1000);
		fOut += (float)((pBCD[2] & 0x0F) *100);
		fOut += (float)((pBCD[1] >> 4) *10);
		fOut += (float)((pBCD[1] & 0x0F) *1);
		fOut += (float)((pBCD[0] >> 4) *0.1);
		fOut += (float)((pBCD[0] & 0x0F) *0.01);
	}
	else
	{
		int nLen = pFrameInfo->nDataLen;
		int nPoint = pFrameInfo->nPointNum;
		int n;
		float fSCal = 1.0;
		for(n = 0; n < nPoint; n++)
		{
			fSCal /= 10.0;
		}

		//TRACE("-------The nLen = %d, nPoint = %d, fScale = %f, Bytes[%02X%02X]-----\n",
		//	nLen, nPoint, fSCal, pBCD[0], pBCD[1]);

		if(nLen < 1)
		{
			return 0.0;
		}

		BOOL bSigned = FALSE;
		
		if(((pBCD[nLen - 1] >> 7) & 0x01) == 0x01)
		{
			bSigned = TRUE;
		}

		//fOut += (float)((pBCD[0] & 0x0F) *fSCal);
		//fOut += (float)((pBCD[0] >> 4) *fSCal * 10);

		//if(nLen >= 2)
		//{
		//	fOut += (float)((pBCD[1] >> 4) *fSCal * 1000);
		//	fOut += (float)((pBCD[1] & 0x0F) *fSCal * 100);
		//}

		//if(nLen >= 3)
		//{
		//	fOut += (float)((pBCD[2] >> 4) *fSCal * 100000);
		//	fOut += (float)((pBCD[2] & 0x0F) *fSCal * 10000);
		//}

		//if(nLen >= 4)
		//{
		//	fOut += (float)(((pBCD[3] >> 4) & 0x07) *fSCal * 1000000);
		//	fOut += (float)((pBCD[3] & 0x0F) *fSCal * 100000);
		//}
		int k = 0;
		float fScale_Ex = 1.0;
		for(n = 0; n < nLen; n++)
		{
			if(n == (nLen - 1))
			{
				fOut += (float)(((pBCD[n] >> 4) & 0x07) *fSCal * fScale_Ex * 10.0);
			}
			else
			{
				fOut += (float)(((pBCD[n] >> 4)) *fSCal * fScale_Ex * 10.0);
			}

			fOut += (float)((pBCD[n] & 0x0F) *fSCal * fScale_Ex);

			fScale_Ex *= 100.0;
		}

		fOut *= (bSigned) ? -1 : 1;
	}
	return fOut;
}

// 持续保存最新200条电表报文，当告警发生时（通信中断），继续保存告警发生后的200条报文
void RecMeterFaultLog(RS485_SAMPLER_DATA* pWhim_Rough, BYTE* pbyBuff, ssize_t uFrameSize)
{
	// Auto Rec bms canlog, when alm trigger rec to file
	MeterRecCirBuffer* meterRecBuff = &pWhim_Rough->meterRecCirBuffer;
	MeterRecBuffer* meterFaultRecBuff = &pWhim_Rough->meterFaultRecBuffer;

	if(meterFaultRecBuff->isActive == FALSE)
	{
		memcpy(meterRecBuff->buffer[meterRecBuff->head], pbyBuff, uFrameSize);
		meterRecBuff->dataLen[meterRecBuff->head] = uFrameSize;
		meterRecBuff->head = (meterRecBuff->head + 1) % 200;
		if(meterRecBuff->count < 200)
		{
			meterRecBuff->count++;
		}
		else
		{
			meterRecBuff->tail = (meterRecBuff->tail + 1) % 200;
		}
	}
	else if(ABS(pWhim_Rough->tm_fLastRecMeterFaultLog - _NOW_) > 5*60 && meterFaultRecBuff->isActive == TRUE) // 最多5min记录一次
	{
		char log[1024];
		char tmp[64];
		memset(log, 0, sizeof(log));
		memset(tmp, 0, sizeof(tmp));
		
		int m, n;
		if(meterFaultRecBuff->bTrigger == FALSE)
		{
			LOG_METER_FAULT_INFO("----------follow before Alarm occur : -----------");
			while(meterRecBuff->count-- > 0)
			{
				for(n = 0; n < meterRecBuff->dataLen[meterRecBuff->tail]; ++n)
				{
					snprintf(tmp, 32, " %02X", meterRecBuff->buffer[meterRecBuff->tail][n]);
					strcat(log, tmp);
				}
				LOG_METER_FAULT_INFO("%s", log);
				memset(log, 0, sizeof(log));
				meterRecBuff->tail = (meterRecBuff->tail + 1) % 200;
			}
			meterFaultRecBuff->bTrigger = TRUE;
		}

		if(meterFaultRecBuff->count < 200)
		{
			memcpy(meterFaultRecBuff->buffer[meterFaultRecBuff->count], pbyBuff, uFrameSize);
			meterFaultRecBuff->dataLen[meterFaultRecBuff->count] = uFrameSize;
			meterFaultRecBuff->count++;
		}
		else
		{
			// TRACE("REC BMSALM\n");			
			LOG_METER_FAULT_INFO("----------follow after Alarm occur : -----------");
			for(m=0; m < meterFaultRecBuff->count; ++m)
			{
				for(n = 0; n < meterFaultRecBuff->dataLen[m]; ++n)
				{
					snprintf(tmp, 32, " %02X", meterFaultRecBuff->buffer[m][n]);
					strcat(log, tmp);
				}
				LOG_METER_FAULT_INFO("%s", log);
				memset(log, 0, sizeof(log));
			}
			meterRecBuff->count = 0;
			meterRecBuff->head = 0;
			meterRecBuff->tail = 0;

			meterFaultRecBuff->count = 0;
			meterFaultRecBuff->isActive = FALSE;
			meterFaultRecBuff->bTrigger = TRUE;
			pWhim_Rough->tm_fLastRecMeterFaultLog = _NOW_;
		}
	}
}

static BYTE* MakewhMeterSendBuff(RS485_SAMPLER_DATA* pWhim_Rough, WH_FRAME_INFO *pFrameInfo, int nWhichWh)
{
	BYTE* pbyBuff = pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	BYTE* pbyBuffOrig = pbyBuff;
	memcpy(pbyBuff, &pFrameInfo->sendFrame, 10);
	pbyBuff += 10;
	int nDataLen = pFrameInfo->nFrameLen - 12;
	//if(nDataLen > 0)
	{
		memcpy(pbyBuff, pFrameInfo->sendFrame.pbyData, nDataLen);
	}
	pbyBuff += nDataLen;
	*pbyBuff = pFrameInfo->sendFrame.byChkSum;
	pbyBuff += 1;
	*pbyBuff = pFrameInfo->sendFrame.byEOI;

	// int i;
	// printf("KWHMETER---645 send frame :  ");
	// for (i=0;i<17;i++)
	// {
	// 	printf("%02x ",pbyBuffOrig[i]);
	// }
	// printf("\n");

	// RecMeterFaultLog(pWhim_Rough, pbyBuffOrig, 17);

	return pbyBuffOrig;
}
#define MIN_WH_FRAME_LEN	(12 + 4) //min len = 16, include FE FE FE FE,
static int Pack_whMeterCmd_AndGetReply(IN RS485_SAMPLER_DATA* pWhim_Rough, IN WH_FRAME_INFO* pwhFrame, int nWhichWh)
{
	int nWriteBytes;
	int nReadBytes;
	//if (Mutex_Lock(pWhim_Rough->hSamplerLock, MAX_RS485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	//{
	//	return ERR_USR_MUTEX_WAIT_TIMEOUT;
	//}

	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	
	//Clear buffer
	pComm->Control(pWhim_Rough->hPort,
		COMM_PURGE_RXCLEAR, NULL, 0);

	//TRACEX("Now begin to write whMeter cmd!\n");
	//First Send wake up frame
	VAR_VALUE varWakeUp;
	varWakeUp.ulValue = WH_WAKEUP_FRAME;
	nWriteBytes = pComm->Write(pWhim_Rough->hPort, varWakeUp.abyValue, 4);
	TRACE("whMeter cmd Write wakeup OK [%d] !\n", nWriteBytes);
	Sleep(20);

	//Now beging send frame
	nWriteBytes = pComm->Write(pWhim_Rough->hPort, 
		MakewhMeterSendBuff(pWhim_Rough, pwhFrame, nWhichWh), pwhFrame->nFrameLen);

	TRACE("whMeter cmd Write cccc OK [%d], expect len = %d!\n", nWriteBytes, pwhFrame->nFrameLen);
	if(nWriteBytes < pwhFrame->nFrameLen)
	{
		//Mutex_Unlock(pWhim_Rough->hSamplerLock);
		//TRACE("Err: nWriteBytes=[%d] nFrameLen=[%d]\n", nWriteBytes,pwhFrame->nFrameLen);
		return RS485_WRITE_ERR;
	}

	Sleep(50);

	//TRACEX("Now begin to read whMeter cmd reply!\n");
	//if(bNeedGetReply)
	{
		nReadBytes = pComm->Read(pWhim_Rough->hPort,
			pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], /*pwhFrame->nExpectAnswerLen + 4*/ RS485_RECV_BUF_LEN);

		// int i;
		// printf("KWHMETER---645 recv frame :  ");
		// for (i=0;i<30;i++)
		// {
		// 	printf("%02x ",pWhim_Rough->abyRcvBuf[i]);
		// }
		// printf("\n");

		// RecMeterFaultLog(pWhim_Rough, pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 30);

		TRACEX("Read whMeter cmd OK! ExpectLen = %d,  readLen = %d, read[%s]!\n", pwhFrame->nExpectAnswerLen + 4, nReadBytes,
			pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType]);

		if(nReadBytes <= 0)
		{
			//Mutex_Unlock(pWhim_Rough->hSamplerLock);
			return RS485_READ_ERR;
		}
		//if((nReadBytes < pwhFrame->nExpectAnswerLen) && (nReadBytes != MIN_WH_FRAME_LEN))
		//{
		//	Sleep(2000);
		//	nReadBytes += RS485_Read(g_RS485_1SamplerData.hPort, pwhFrame->nExpectAnswerLen - nReadBytes,
		//		g_RS485_1SamplerData.abyRcvBuf + nReadBytes);
		//	TRACEX("Read whMeter cmd AGAINNNNNNNN OK! readLen = %d!\n", nReadBytes);
		//	//Mutex_Unlock(g_RS485_1SamplerData.hSamplerLock);
		//	//return RS485_READ_ERR;
		//}
	}
	pWhim_Rough->nReadBuffBytes[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType] = nReadBytes;
	//Mutex_Unlock(pWhim_Rough->hSamplerLock);
	return RS485_COMM_OK;
}
#define  ERR_WRONG_FRAME	0xFF
static int ExtractWhFrame(RS485_SAMPLER_DATA* pWhim_Rough, WH_FRAME_INFO *pFrameInfo, int nWhichWh)
{
	BYTE* pBuff = pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	int n = 0;
	int m = 0;
	int nSub_Idx = 0;
	int nStart = -1;
	for(n = 0; n < pWhim_Rough->nReadBuffBytes[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType]; n++)
	{
		//Jimmy note 20191207: do not need judge FEFEFEFE, another manufactor does NOT replay this
		//if((pBuff[n] == 0xFE) && (pBuff[n + 1] == 0x68))
		if(pBuff[n] == 0x68)
		{
			//m = n+ 8;
			m = n + 7;
			if((m < pWhim_Rough->nReadBuffBytes[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType]) &&
				(pBuff[m] == 0x68))
			{
				//nStart = n + 1;
				nStart = n;
				break;
			}
		}
	}
	if(nStart == -1)
	{
		return ERR_WRONG_FRAME;
	}
	
	////---------------Test------------------------------------
	//TRACE("whMeter[%d, %d]----nStart = %d, The Buff is:[", pWhim_Rough->nSamplerId, nWhichWh, nStart);
	//for(n = nStart; n < pWhim_Rough->nReadBuffBytes; n++)
	//{
	//	printf("%02X", pBuff[n]);
	//}
	//TRACE("]\n");
	//-------------------------------------------------------
	
	BYTE nErrCode = ERR_SAMPLE_OK;
	WH_A_FRAME *pRecFrame = (WH_A_FRAME*)(pBuff + nStart);

	int nPasedFrmLen = pWhim_Rough->nReadBuffBytes[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType] - nStart;

	if(nPasedFrmLen < pFrameInfo->nExpectAnswerLen)
	{
		TRACEX("------The frmeExpectLe = %d, but RealLen = %d, so discard this frme!-------------\n", pFrameInfo->nExpectAnswerLen, nPasedFrmLen);
		return ERR_WRONG_FRAME;
	}

	//if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_ADDR)
	//{
	//	if(nWhichWh == 1)
	//	{
	//		memcpy(pWhim_Rough->byExtraAC_ReadMeterA0A6, pRecFrame->byAddrA0_A5, 6);
	//		TRACE("----------The whMeterAddr for ExtraAC = [%02X%02X%02X%02X%02X%02X]",
	//			pWhim_Rough->byExtraAC_ReadMeterA0A6[0], pWhim_Rough->byExtraAC_ReadMeterA0A6[1],
	//			pWhim_Rough->byExtraAC_ReadMeterA0A6[2], pWhim_Rough->byExtraAC_ReadMeterA0A6[3],
	//			pWhim_Rough->byExtraAC_ReadMeterA0A6[4], pWhim_Rough->byExtraAC_ReadMeterA0A6[5]);
	//	}
	//	else
	//	{
	//		memcpy(pWhim_Rough->byReadMeterA0A6, pRecFrame->byAddrA0_A5, 6);
	//		TRACE("----------The whMeterAddr for DC = [%02X%02X%02X%02X%02X%02X]",
	//			pWhim_Rough->byReadMeterA0A6[0], pWhim_Rough->byReadMeterA0A6[1],
	//			pWhim_Rough->byReadMeterA0A6[2], pWhim_Rough->byReadMeterA0A6[3],
	//			pWhim_Rough->byReadMeterA0A6[4], pWhim_Rough->byReadMeterA0A6[5]);
	//	}
	//	return ERR_SAMPLE_OK;
	//}

	BYTE* pbyValStart = pBuff + nStart + 10;

	int i;
	for (i=0;i<4;i++)
	{
		if (pFrameInfo->sendFrame.pbyData[i] != pbyValStart[i])
		{
			break;
		}
	}
	if (i !=4 )
	{
		TRACEE("KWHMETER------The frame Data identification bit NO match send [%02x] [%02x] [%02x] [%02x]  recv [%02x] [%02x] [%02x] [%02x]",
			pFrameInfo->sendFrame.pbyData[0],pFrameInfo->sendFrame.pbyData[1],pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],
			pbyValStart[0],pbyValStart[1],pbyValStart[2],pbyValStart[3]);

		// LogOut(MAIN_MODULE_REPORTER, "whMeterSampler", LOG_TYPE_ERROR, 
		// "KWHMETER------The frame Data identification bit NO match send [%02x] [%02x] [%02x] [%02x] recv [%02x] [%02x] [%02x] [%02x]", 
		// pFrameInfo->sendFrame.pbyData[0],pFrameInfo->sendFrame.pbyData[1],pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],
		// pbyValStart[0],pbyValStart[1],pbyValStart[2],pbyValStart[3]);
		return ERR_WRONG_FRAME;
	}
	else 
	{
		TRACEI("KWHMETER------The frame Data identification bit match send [%02x] [%02x] [%02x] [%02x]  recv [%02x] [%02x] [%02x] [%02x]",
			pFrameInfo->sendFrame.pbyData[0],pFrameInfo->sendFrame.pbyData[1],pFrameInfo->sendFrame.pbyData[2],pFrameInfo->sendFrame.pbyData[3],
			pbyValStart[0],pbyValStart[1],pbyValStart[2],pbyValStart[3]);
	}

	switch(pFrameInfo->sendFrame.byCtrlCode)
	{
	case 0x11: //read data
		if((pRecFrame->byCtrlCode != 0x91) ||
			(pRecFrame->byDataLen == 0x01))
		{
			//TRACEX("whEEEEEEEEEEEEEEEEE= %d, Err+1 = %d\n",*(BYTE*)pbyValStart, *(BYTE*)(pbyValStart + 1));
			nErrCode = (*(BYTE*)pbyValStart) - 0x33;
		}
		else
		{
			//---Added By John 2022-11-30,added For Remote kWh Reading---//
			if((pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
			|| (pFrameInfo->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH))
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < MAX_MULTI_FRAMEDATA; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+4*nSub_Idx, pFrameInfo);
					//TRACEX("----The fGet_MultiData[%d]=[%.2f]----\n", nSub_Idx, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_VOLT_PHASE_A)
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+2*nSub_Idx, pFrameInfo);
					//TRACEX("----VoltA The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_CURR_PHASE_A)
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+3*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}

			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_ACTPOWER_AC)
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+3*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_REACTPOWER_AC)
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+3*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			
			else if(pFrameInfo->frameType == WH_FRAME_TYPE_GET_POWERFACTOR)
			{
				for(n = 0; n < nPasedFrmLen; n++)
				{
					pbyValStart[n + 4] -= 0x33;
				}

				for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
				{
					pFrameInfo->fGet_MultiData[nSub_Idx] = ConvertBCD_2WhVal(pbyValStart+4+2*nSub_Idx, pFrameInfo);
					//TRACEX("----Cuurt A The fGet_MultiData[%d]nPasedFrmLen=[%d] Value[%.2f]----\n", \
					//	nSub_Idx, nPasedFrmLen, pFrameInfo->fGet_MultiData[nSub_Idx]);
				}
			}
			else
			{
				for(n = 0; n < 4; n++)
				{
					pbyValStart[n + 4] -= 0x33;
					//printf("%02X|", pbyValStart[n+4]);
				}
				pFrameInfo->fGetData = ConvertBCD_2WhVal(pbyValStart + 4, pFrameInfo);
				//TRACE("\n----The fGetData is [%.2f]----\n", pFrameInfo->fGetData);
			}
		}
		break;
	case 0x15: //set comm addr, will not relply if error
		//if((pRecFrame->byAddrA0_A5[0] != 0x01) ||
		//	(pRecFrame->byCtrlCode != 0x95))
		//{
		//	nErrCode = *pbyValStart;
		//}
		break;
	}

	return nErrCode;
}

static int whMeter_SigSamp(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh, WH_FARAME_TYPE emCmdType, WH_ROUGH_SIG emSigType)
{
	int nSub_Idx = 0;
	HANDLE hSamplerLock = ((SAMPLER*)pWhim_Rough->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, MAX_RS485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}
	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	//if(pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_ac)
	//{
	//	char szSett[] = "2400, E, 8, 1";
	//	pComm->Control(pWhim_Rough->hPort,
	//		COMM_SET_SERIAL_ATTRIB, (void*)szSett, strlen(szSett));
	//}
	//else
	//{
	//	char szSett[]  = "9600, E, 8, 1";
	//	pComm->Control(pWhim_Rough->hPort,
	//		COMM_SET_SERIAL_ATTRIB, (void*)szSett, strlen(szSett));
	//}

	pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
		strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

	WH_FRAME_INFO* pFrame = &pWhim_Rough->frm_whFrames[emCmdType];

	memcpy(pFrame->sendFrame.byAddrA0_A5, pWhim_Rough->meterData[nWhichWh].byReadMeterA0A6, 6);
	
	Make_CS(pFrame);
	if (RS485_COMM_OK != Pack_whMeterCmd_AndGetReply(pWhim_Rough, pFrame, nWhichWh))
	{
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_TIMEOUT;
	}
	int nErrCode = ExtractWhFrame(pWhim_Rough, pFrame, nWhichWh);
	if(ERR_SAMPLE_OK != nErrCode)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].lValue = nErrCode;
		//pWhim_Rough->varWHMeterRough[nWhichWh][emSigType].lValue = -9999;
		TRACEX("whMeter[%d] Has Faillllllllllllllllllllllllllllllllllllllllll for type[%d], ErrCode = %d.\n", nWhichWh, emCmdType, nErrCode);
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	if((pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_TOTALWH)
	|| (pFrame->frameType == WH_FRAME_TYPE_GET_MULTI_RVSTATALWH))
	{
		for(nSub_Idx = 0; nSub_Idx < MAX_MULTI_FRAMEDATA; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
		}
	}
	else if(emSigType == WHMETER_ROUGH_RT_VOLT_A)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
		}
	}
	else if(emSigType == WHMETER_ROUGH_RT_CURR_A)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 3; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
		}
	}
	else if(emSigType == WHMETER_ROUGH_RTACTPower)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
		}

	}
	else if(emSigType == WHMETER_ROUGH_RTReActPower)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
		}
	}
	else if(emSigType == WHMETER_ROUGH_RT_PowerFactor)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < 4; nSub_Idx++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = pFrame->fGet_MultiData[nSub_Idx];
		}
	}
	else   
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue = pFrame->fGetData;
		if((pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_ac) && 
			((emSigType == WHMETER_ROUGH_RTkWh_AC) || 
			(emSigType == WHMETER_ROUGH_RTkvar) ||
			(emSigType == WHMETER_ROUGH_RTkvar2) ||  
			//(emSigType == WHMETER_ROUGH_RT_CURR_A) || 
			//(emSigType == WHMETER_ROUGH_RT_CURR_B) || 
			//(emSigType == WHMETER_ROUGH_RT_CURR_C) ||
			//(emSigType == WHMETER_ROUGH_RTACTPower) ||
			//(emSigType == WHMETER_ROUGH_RTReActPower) ||
			(emSigType == WHMETER_ROUGH_PMAXDEMAND) ||
			(emSigType == WHMETER_ROUGH_CURRENTPOSDEMAND) ||
			(emSigType == WHMETER_ROUGH_RT_RVSkWh_AC)))
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
		}
		else if(emSigType == WHMETER_ROUGH_RT_Freq)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType].fValue *= 10; //AC Freq Need * 10
		}
	}

	Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;
}



static USHORT ModbusCRC(BYTE *pData, int len)
{
	static BYTE s_byCRCHi[] =
	{
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
		0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
		0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
		0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
		0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
		0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
		0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
		0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
		0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
		0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
		0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
		0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
		0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
		0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
		0x80,0x41,0x00,0xc1,0x81,0x40
	};

	static BYTE s_byCRCLo[] =
	{
		0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,
		0x07,0xc7,0x05,0xc5,0xc4,0x04,0xcc,0x0c,0x0d,0xcd,
		0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,
		0x08,0xc8,0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,
		0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,0x14,0xd4,
		0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,
		0x11,0xd1,0xd0,0x10,0xf0,0x30,0x31,0xf1,0x33,0xf3,
		0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
		0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,
		0x3b,0xfb,0x39,0xf9,0xf8,0x38,0x28,0xe8,0xe9,0x29,
		0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,
		0xec,0x2c,0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,
		0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,0xa0,0x60,
		0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,
		0xa5,0x65,0x64,0xa4,0x6c,0xac,0xad,0x6d,0xaf,0x6f,
		0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
		0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,
		0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,0xb4,0x74,0x75,0xb5,
		0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,
		0x70,0xb0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
		0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9c,0x5c,
		0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,
		0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4b,0x8b,
		0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
		0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
		0x43,0x83,0x41,0x81,0x80,0x40
	};
	BYTE i = (BYTE)len;
	BYTE  byCRCHi = 0xff;
	BYTE  byCRCLo = 0xff;
	BYTE  byIdx;
	USHORT crc;

	while(i--)
	{
		byIdx = byCRCHi ^ *pData++;
		byCRCHi = byCRCLo ^ s_byCRCHi[byIdx];
		byCRCLo = s_byCRCLo[byIdx];
	}

	crc = byCRCHi;
	crc <<= 8;
	crc += byCRCLo;

	return crc;
}

static BOOL Get_RM_Private_Data(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh, int nGetType)
{
	TRACEX("Get_RM_Private_Data:nWhichWh=[%d] nGetType=%d\n", nWhichWh, nGetType);
	int nExpectDataLen = 80;
	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	//Clear buffer
	//pComm->Control(pWhim_Rough->hPort, COMM_PURGE_RXCLEAR, NULL, 0);

	Sleep(100);
	
	BYTE *pRecvBuf = pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
	int nReadBytes = pComm->Read(pWhim_Rough->hPort, pRecvBuf, RS485_RECV_BUF_LEN);
	//check length
	if(nReadBytes < 7)
	{
		TRACE("ReadFailed : Remote Private MOdbus read bytes[%d]\n", nReadBytes);
		return FALSE;
	}
	else
	{
		
		//TRACE("Read Success: Remote AC Meter Bytes = %d\n", nReadBytes);
		//int n = 0;
		//for(n = 0; n < nReadBytes; n++)
		//{
		//	printf("%02X-",pRecvBuf[n]);
		//}
		//printf("\n");
		
	}
	

	// Check Address
	if(*pRecvBuf != pWhim_Rough->meterData[nWhichWh].byModbusAddress)
	{
		TRACEX("RM Private: Err111");
		return FALSE;
	}
	// Check Function Code
	if(*(pRecvBuf + 1) != 0x03)
	{
		TRACEX("RM Private: Err222");
		return FALSE;
	}

	// Check Data Length
	int byDataLen = (int)((BYTE)(*(pRecvBuf + 2)));

	if(byDataLen != nExpectDataLen)
	{
		TRACEX("RM Private:Err333 byDataLen=%d nExpectDataLen=%d\n", \
			byDataLen, nExpectDataLen);
		return FALSE;
	}

	// Check CRC
	int nCRC_StartCal = 3 + (int)byDataLen;
	int nCRC_2CalcLen = nCRC_StartCal;
	USHORT usReadCRC = (((USHORT)(*(pRecvBuf + nCRC_StartCal))) << 8) + (USHORT)(*(pRecvBuf + nCRC_StartCal + 1));
	USHORT usCalcCRC = ModbusCRC(pRecvBuf, nCRC_2CalcLen);
	if(usReadCRC != usCalcCRC)
	{
		TRACE("RM Private:CRC Error! Calc CRC = %04X usReadCRC=%04X\n", usCalcCRC, usReadCRC);
		return FALSE;
	}

	//now parse data
	BYTE *pbyDataPos = pRecvBuf + 3;
	int n;
	if(nGetType == RM_PRIVATE_GET_INSTANT_VALUES)
	{
		TRACE("Start Parse Remote Private Instants Values!\n");
		OBJ_PTR modSigMap[] = 
		{
			{WHMETER_ROUGH_RTkWh_AC, 	1},
			{WHMETER_ROUGH_RT_RVSkWh_AC,6},
			{WHMETER_ROUGH_RT_VOLT_A, 	11},
			{WHMETER_ROUGH_RT_VOLT_B, 	12},
			{WHMETER_ROUGH_RT_VOLT_C, 	13},
			{WHMETER_ROUGH_RT_CURR_A, 	14},
			{WHMETER_ROUGH_RT_CURR_B, 	15},
			{WHMETER_ROUGH_RT_CURR_C, 	16},
			{WHMETER_ROUGH_RTACTPower, 	17},
		};

		for(n = 0; n < ITEM_OF(modSigMap); n++)
		{
			//Float Data Type ,Little Endian
		  	pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[3] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 3);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[2] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 2);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[1] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 1);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[0] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 0);
			
			//TRACE("nWhichWh=[%d] ulValue=[%ld] %02X|%02X|%02X|%02X \n",\
			//	nWhichWh,\
			//	pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].ulValue,\
			//	pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[3],\
			//	pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[2],\
			//	pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[1],\
			//	pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[0]);
			
		}
	}
	return TRUE;
}

static BOOL Get_EM737_Data(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh, int nGetType)
{
	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	//Clear buffer
	pComm->Control(pWhim_Rough->hPort, COMM_PURGE_RXCLEAR, NULL, 0);

	//Now prepare modbus send buffer
	BYTE* pSendBuf = pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	*pSendBuf++ = pWhim_Rough->meterData[nWhichWh].byModbusAddress;
	*pSendBuf++ = 0x04; //function code
	int nExpectDataLen;
	if(nGetType == EM737_GET_INSTANT_VALUES)
	{
		*pSendBuf++ = 0x00; //start-hibyte
		*pSendBuf++ = 0x10; //start-lowbyte
		*pSendBuf++ = 0x00; //lenth-hibyte
		*pSendBuf++ = 16 * 2; //length-lowbyte (16*4), every reister has 2 bytes
		nExpectDataLen = 64;
	}
	else// if(nGetType == EM737_GET_ENERGY_VALUES)
	{
		*pSendBuf++ = 0x01; //start-hibyte
		*pSendBuf++ = 0x60; //start-lowbyte
		*pSendBuf++ = 0x00; //lenth-hibyte
		*pSendBuf++ = 2 * 2; //length-lowbyte (4*2)
		nExpectDataLen = 8;
	}
	
	USHORT usCRC = ModbusCRC(pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 6);
	*pSendBuf++ = (BYTE)(usCRC >> 8);
	*pSendBuf++ = (BYTE)(usCRC & 0xFF);
	//now send out the data to meter
	int nWriteBytes = pComm->Write(pWhim_Rough->hPort, pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 8);
	if(nWriteBytes < 8)
	{
		return FALSE;
	}
	//else
	//{
	//	int k;
	//	TRACEX("EM737 write:");
	//	for(k = 0; k < 8; k++)
	//	{
	//		TRACE("%02X-", pWhim_Rough->abySendBuf[k]);
	//	}
	//	TRACE("\n");
	//}

	Sleep(500); //Jimmy note 20190123, do not know why need sleep for energy
	//now read response frame from meter
	BYTE *pRecvBuf = pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
	int nReadBytes = pComm->Read(pWhim_Rough->hPort, pRecvBuf, RS485_RECV_BUF_LEN);
	//check length
	if(nReadBytes < 7)
	{
		TRACEX("MOdbus read bytes[%d]\n", nReadBytes);
		return FALSE;
	}
	//else
	//{
	//	TRACEX("EM737 read%d:", nReadBytes);
	//	int k;
	//	for(k = 0; k < nReadBytes; k++)
	//	{
	//		TRACE("%02X-", pRecvBuf[k]);
	//	}
	//	TRACE("\n");
	//}

	// Check Address
	if(*pRecvBuf != pWhim_Rough->meterData[nWhichWh].byModbusAddress)
	{
		TRACEX("EM737Err111");
		return FALSE;
	}
	// Check Function Code
	if(*(pRecvBuf + 1) != 0x04)
	{
		TRACEX("EM737Err222");
		return FALSE;
	}

	// Check Data Length
	int byDataLen = (int)((BYTE)(*(pRecvBuf + 2)));

	if(byDataLen != nExpectDataLen)
	{
		TRACEX("EM737Err333");
		return FALSE;
	}

	// Check CRC
	int nCRC_StartCal = 3 + (int)byDataLen;
	int nCRC_2CalcLen = nCRC_StartCal;
	USHORT usReadCRC = (((USHORT)(*(pRecvBuf + nCRC_StartCal))) << 8) + (USHORT)(*(pRecvBuf + nCRC_StartCal + 1));
	USHORT usCalcCRC = ModbusCRC(pRecvBuf, nCRC_2CalcLen);
	if(usReadCRC != usCalcCRC)
	{
		TRACE("CRC Error! Calc CRC = %04X\n", usCalcCRC);
		//return FALSE;
	}

	//now parse data
	BYTE *pbyDataPos = pRecvBuf + 3;
	int n;
	if(nGetType == EM737_GET_INSTANT_VALUES)
	{
		OBJ_PTR modSigMap[] = 
		{
			{WHMETER_ROUGH_RT_VOLT_A, 1},
			{WHMETER_ROUGH_RT_VOLT_B, 2},
			{WHMETER_ROUGH_RT_VOLT_C, 3},
			{WHMETER_ROUGH_RT_CURR_A, 4},
			{WHMETER_ROUGH_RT_CURR_B, 5},
			{WHMETER_ROUGH_RT_CURR_C, 6},
			{WHMETER_ROUGH_RTACTPower, 12},
			{WHMETER_ROUGH_RTReActPower, 16},
		};

		for(n = 0; n < ITEM_OF(modSigMap); n++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[3] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 0);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[2] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 1);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[1] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 2);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[0] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 3);
		}
	}
	else// if(nGetType == EM737_GET_ENERGY_VALUES)
	{
		OBJ_PTR modSigMap[] = 
		{
			{WHMETER_ROUGH_RTkWh_AC, 1},
			{WHMETER_ROUGH_RTkvar, 2},
		};

		for(n = 0; n < ITEM_OF(modSigMap); n++)
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[3] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 0);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[2] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 1);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[1] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 2);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[modSigMap[n].nId].abyValue[0] = *(pbyDataPos + 4*(modSigMap[n].nSubId - 1) + 3);
		}
	}

	return TRUE;
}

int whMeter_EM737_Sample(IN RS485_SAMPLER_DATA* pWhim_Rough, IN int nWhichWh)
{
	HANDLE hSamplerLock = ((SAMPLER*)pWhim_Rough->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, MAX_RS485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	//change to meter default comm setting
	//char szSett[]  = "2400, E, 8, 1";
	//pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)szSett, strlen(szSett));
	pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
		strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

	if(!Get_EM737_Data(pWhim_Rough, nWhichWh, EM737_GET_INSTANT_VALUES))
	{
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}

	if(!Get_EM737_Data(pWhim_Rough, nWhichWh, EM737_GET_ENERGY_VALUES))
	{
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}

	Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;
}

int whMeter_RM_Private_Sample(IN RS485_SAMPLER_DATA* pWhim_Rough, IN int nWhichWh)
{
	HANDLE hSamplerLock = ((SAMPLER*)pWhim_Rough->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, MAX_RS485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	//change to meter default comm setting
	//char szSett[]  = "2400, E, 8, 1";
	//pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)szSett, strlen(szSett));
	//pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
	//	strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

	if(!Get_RM_Private_Data(pWhim_Rough, nWhichWh, RM_PRIVATE_GET_INSTANT_VALUES))
	{
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}

 
	Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;
}


//------------------ Modbus RTU YD AC Meter ----------------------//
/*
 *John 2022-09-02 Debug Data OK,But Not Parse Data!!!
Tx:006289-01 03 08 3E 00 28 26 78
Rx:006290-01 03 50 43 60 5E 62 43 61 99 36 43 62 4E EB 43 C3 2A 
53 43 C2 13 3F 43 C3 3F A4 43 C4 2C 12 42 47 E8 93 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 27 39
 *
*/
// typedef struct 
// {
// 	int		nVarVal_Idx;
// 	int		nRegister_Idx;
// 	int		nWordLen;
// 	int		nVal_OffSet;
// }YD_REGISTER_ITEM;


float FourBytesToFloat(const BYTE *pBuf)
{
    union {
        uint32_t u32;
        float f;
    } u;

    u.u32 = ((uint32_t)pBuf[0] << 24) |
            ((uint32_t)pBuf[1] << 16) |
            ((uint32_t)pBuf[2] << 8)  |
            ((uint32_t)pBuf[3]);

    return u.f;
}

static int YD_ModBus_whMeter_SigSamp(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh, WH_FARAME_TYPE emCmdType, WH_ROUGH_SIG emSigType)
{
	int nSub_Idx = 0;
	HANDLE hSamplerLock = ((SAMPLER*)pWhim_Rough->hSampler)->hSamplerLock;
	if(Mutex_Lock(hSamplerLock, MAX_RS485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	COMMPORT* pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
	//Clear buffer
	pComm->Control(pWhim_Rough->hPort, COMM_PURGE_RXCLEAR, NULL, 0);
	pComm->Control(pWhim_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
		strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

	Sleep(20);

	YD_MD_FRAME_INFO* pFrame = &pWhim_Rough->frm_YD_MD_whFrames[emCmdType];
	pFrame->sendFrame.byModbusAddr = pWhim_Rough->meterData[nWhichWh].byModbusAddress;

	//Now prepare modbus send buffer
	BYTE* pByArr = (BYTE*)&pFrame->sendFrame;//Note: this needs #pragma pack(1), or else will be problems!!!
	BYTE* pSendBuf = pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	memcpy(pSendBuf, pByArr, 8);
	// *pSendBuf++ = pWhim_Rough->meterData[nWhichWh].byModbusAddress;
	// *pSendBuf++ = 0x03; //function code
	// int nExpectDataLen;
	// if(nGetType == MD_GET_PHASE_VALUES) //MD_GET_PHASE_VALUES
	// {
	// 	*pSendBuf++ = 0x08; //start-hibyte
	// 	*pSendBuf++ = 0x3E; //start-lowbyte
	// 	*pSendBuf++ = 0x00; //lenth-hibyte
	// 	*pSendBuf++ = 20*2; //length-lowbyte (16*4), every reister has 2 bytes
	// 	nExpectDataLen = 40;
	// }
	USHORT usCRC = ModbusCRC(pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 6);
	pSendBuf += 6;
	pSendBuf[0] = (BYTE)(usCRC >> 8); 
	pSendBuf[1] = (BYTE)(usCRC & 0xFF);	
	//now send out the data to meter
	int nWriteBytes = pComm->Write(pWhim_Rough->hPort, pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 8);
	if(nWriteBytes < 8)
	{
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}
	// else
	// {
	// 	int k;
	// 	TRACEX("Modbus-RTU YD write:");
	// 	for(k = 0; k < 8; k++)
	// 	{
	// 		printf("%02X-", pWhim_Rough->abySendBuf[k]);
	// 	}
	// 	TRACE("\n");
	// }

	Sleep(50); 
	//now read response frame from meter
	BYTE *pRecvBuf = pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
	int n;
	int nReadBytes;
	//read back for 3 times
	for(n = 0; n < 3; n++)
	{
		nReadBytes = pComm->Read(pWhim_Rough->hPort, pRecvBuf, RS485_RECV_BUF_LEN);
		if(nReadBytes >= 7)
		{
			break;
		}
	}
	
	//check length
	if(nReadBytes < 7)
	{
		TRACEX("MOdbus read bytes[%d]\n", nReadBytes);
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}
	// else
	// {
	// 	printf("Modbus-RTU YD read%d:", nReadBytes);
	// 	int k;
	// 	for(k = 0; k < nReadBytes; k++)
	// 	{
	// 		printf("%02X-", pRecvBuf[k]);
	// 	}
	// 	printf("\n");
	// }


	// Check Address
	if(*pRecvBuf != pFrame->sendFrame.byModbusAddr)
	{
		TRACEX("Modbus-RTU YD ErrAddress\n");
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check Function Code
	if(*(pRecvBuf + 1) != pFrame->sendFrame.byFunCode)
	{
		TRACEX("Modbus-RTU YD ErrFunCode\n");
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check Data Length
	int byDataLen = (int)((BYTE)(*(pRecvBuf + 2)));

	if(byDataLen != pFrame->nExpectAnswerLen)
	{
		TRACEX("Modbus-RTU YD ErrnExpectAnswerLen\n");
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check CRC
	int nCRC_StartCal = 3 + (int)byDataLen;
	int nCRC_2CalcLen = nCRC_StartCal;
	USHORT usReadCRC = (((USHORT)(*(pRecvBuf + nCRC_StartCal))) << 8) + (USHORT)(*(pRecvBuf + nCRC_StartCal + 1));
	USHORT usCalcCRC = ModbusCRC(pRecvBuf, nCRC_2CalcLen);
	if(usReadCRC != usCalcCRC)
	{
		TRACEX("CRC Error! Calc CRC = %04X\n", usCalcCRC);
		Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	//now parse data
	BYTE *pbyDataPos = pRecvBuf + 3;
	VAR_VALUE varV; 
	int m;
	int nPos = 0;
	UINT uVal;
	//double			tmNow;
	// 	YD_REGISTER_ITEM* pItem = s_modSigMap_1;
	// 	int nItemTotal = ITEM_OF(s_modSigMap_1);
	// 	if(nGetType == MD_GET_ENERGY_VALUES)
	// 	{
	// 		pItem = s_modSigMap_2;
	// 		nItemTotal = ITEM_OF(s_modSigMap_2);
	// 	}
	// 	for(n = 0; n < nItemTotal; n++, pItem++)
	// 	{
	// 		nPos = pItem->nRegister_Idx * 2;
	// 		varV.ulValue = 0;
	// 		for(m = 0; m < pItem->nWordLen; m++)
	// 		{
	// 			uVal = ((UINT)(*(pbyDataPos + nPos)) << 8) + (UINT)(*(pbyDataPos + nPos + 1));
	// 			varV.ulValue += (uVal << (16 * m));
	// 			nPos += 2;
	// 		}
	// 		if((pItem->nVarVal_Idx == WHMETER_ROUGH_RTkWh_AC) || (pItem->nVarVal_Idx == WHMETER_ROUGH_RTkWh_AC))
	// 		{
	// 			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[pItem->nVarVal_Idx].fValue = (float)varV.ulValue / (pItem->nVal_OffSet);
	// 		}
	// 		else
	// 		{
	// 			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[pItem->nVarVal_Idx].fValue = (float)varV.lValue / (pItem->nVal_OffSet);
	// 		}
	// 	}
	if (pFrame->frameType == YD_GET_TYPE_QUICKLY_ACTPOWER_AC
		|| pFrame->frameType == YD_GET_TYPE_QUICKLY_REACTPOWER
		|| pFrame->frameType == YD_GET_TYPE_TOTALWH
		|| pFrame->frameType == YD_GET_TYPE_RVSTATALWH)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos); //*(float * )reverseBytes(pbyDataPos,4); 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 4;
			//tmNow = GetCurrentTime();
			//printf("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == YD_GET_TYPE_QUICKLY_FACTOR
				|| pFrame->frameType == YD_GET_TYPE_FREQ)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)   
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			pbyDataPos += 4;
			//printf("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == YD_GET_TYPE_CURR)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)   
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
			pbyDataPos += 4;
			//printf("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
			
		}
	}
	else if (pFrame->frameType == YD_GET_TYPE_VOLT)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)   
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 4;
			//printf("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
			
		}
	}
	
	Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;
	//int n;
	// if(nGetType == MD_GET_PHASE_VALUES)
	// {
		
	// 	static YD_REGISTER_ITEM s_modSigMap_1[] = 
	// 	{
	// 		{WHMETER_ROUGH_RT_VOLT_A, 	0,	2,	10},
	// 		{WHMETER_ROUGH_RT_VOLT_B,	2,	2,	10},
	// 		{WHMETER_ROUGH_RT_VOLT_C,	4,	2,	10},
	// 		{WHMETER_ROUGH_RT_CURR_A, 	12,	2,	1000},
	// 		{WHMETER_ROUGH_RT_CURR_B, 	14,	2,	1000},
	// 		{WHMETER_ROUGH_RT_CURR_C,	16,	2,	1000},
	// 		{WHMETER_ROUGH_RTACTPower,	40,	2,	10000},
	// 		{WHMETER_ROUGH_RTReActPower, 44, 2, 10000},
	// 	};
	// 	static YD_REGISTER_ITEM s_modSigMap_2[] = 
	// 	{
	// 		{WHMETER_ROUGH_RTkWh_AC, 0,	2,	10},
	// 		{WHMETER_ROUGH_RTkvar,	 2, 2,	10},
	// 	};

	// 	VAR_VALUE varV; 
	// 	int m;
	// 	int nPos = 0;
	// 	UINT uVal;
	// 	YD_REGISTER_ITEM* pItem = s_modSigMap_1;
	// 	int nItemTotal = ITEM_OF(s_modSigMap_1);
	// 	if(nGetType == MD_GET_ENERGY_VALUES)
	// 	{
	// 		pItem = s_modSigMap_2;
	// 		nItemTotal = ITEM_OF(s_modSigMap_2);
	// 	}
	// 	for(n = 0; n < nItemTotal; n++, pItem++)
	// 	{
	// 		nPos = pItem->nRegister_Idx * 2;
	// 		varV.ulValue = 0;
	// 		for(m = 0; m < pItem->nWordLen; m++)
	// 		{
	// 			uVal = ((UINT)(*(pbyDataPos + nPos)) << 8) + (UINT)(*(pbyDataPos + nPos + 1));
	// 			varV.ulValue += (uVal << (16 * m));
	// 			nPos += 2;
	// 		}
	// 		if((pItem->nVarVal_Idx == WHMETER_ROUGH_RTkWh_AC) || (pItem->nVarVal_Idx == WHMETER_ROUGH_RTkWh_AC))
	// 		{
	// 			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[pItem->nVarVal_Idx].fValue = (float)varV.ulValue / (pItem->nVal_OffSet);
	// 		}
	// 		else
	// 		{
	// 			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[pItem->nVarVal_Idx].fValue = (float)varV.lValue / (pItem->nVal_OffSet);
	// 		}
	// 	}
	// }
	

}

static BOOL whMeter_Modbus_YD_Sample(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh)
{
	//printf("whMeter_Modbus_YD_Sample nSampleTimes = [%d]\n",pWhim_Rough->meterData[nWhichWh].nSampleTimes);
	int nSampRTN = ERR_SAMPLE_OK;

	nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_QUICKLY_ACTPOWER_AC, WHMETER_ROUGH_RTACTPower);
	if(nSampRTN != ERR_SAMPLE_OK)
	{
		return nSampRTN;
	}

	if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (4 + nWhichWh))
	{
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_CURR, WHMETER_ROUGH_RT_CURR_A);
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_VOLT, WHMETER_ROUGH_RT_VOLT_A);
	}

	if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (5 + nWhichWh))
	{
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_QUICKLY_REACTPOWER, WHMETER_ROUGH_RTReActPower);
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_QUICKLY_FACTOR, WHMETER_ROUGH_RT_PowerFactor);
	}

	if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (6 + nWhichWh))
	{
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_FREQ, WHMETER_ROUGH_RT_Freq);
	}

	if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (7 + nWhichWh))
	{
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_TOTALWH, WHMETER_ROUGH_RTkWh_AC);
		nSampRTN = YD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, YD_GET_TYPE_RVSTATALWH, WHMETER_ROUGH_RT_RVSkWh_AC);
	}

	if(pWhim_Rough->meterData[nWhichWh].nSampleTimes <= (8 + nWhichWh))
	{
		pWhim_Rough->meterData[nWhichWh].nSampleTimes++;
		return nSampRTN;
	}

	pWhim_Rough->meterData[nWhichWh].nSampleTimes = 0;
	
	return nSampRTN;
}


static int ACREL_ModBus_whMeter_SigSamp(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh, WH_FARAME_TYPE emCmdType, WH_ROUGH_SIG emSigType)
{
	int nSub_Idx = 0;
	int nReadTimes = 0;
	double tmNow;
	COMMPORT* pComm = NULL;
	HANDLE 	hPort = NULL;
	if(pWhim_Rough->bEnableQuickResChannel)
	{
		nReadTimes = 1;
	}
	else
	{
		nReadTimes = 3;
	}

	// HANDLE hSamplerLock = ((SAMPLER*)pWhim_Rough->hSampler)->hSamplerLock;
	// if(Mutex_Lock(hSamplerLock, MAX_RS485_2_LOCK_WAIT) != ERR_MUTEX_OK)
	// {
	// 	return ERR_SAMPLE_NEED_RESAMP;
	// }
	if (pWhim_Rough->meterData[nWhichWh].bPhysicalInterType == RS485_1)
	{
		pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
		hPort = pWhim_Rough->hPort;
	}
	else if (pWhim_Rough->meterData[nWhichWh].bPhysicalInterType == RS485_2)
	{
		pComm = s_pRS485_2_Comm;
		hPort = hRS485_2_Port;
	}
	else
	{
		pComm = s_pUartServer_Comm;
		hPort = hUartServer_Port;
	}

	//Clear buffer
	if (pWhim_Rough->meterData[nWhichWh].bPhysicalInterType != UART_SERVER)
	{
		pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);
		pComm->Control(hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
			strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

		Sleep(10);
	}

	ACREL_MD_FRAME_INFO* pFrame = &pWhim_Rough->frm_ACREL_MD_whFrames[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType][emCmdType];
	pFrame->sendFrame.byModbusAddr = pWhim_Rough->meterData[nWhichWh].byModbusAddress;

	//Now prepare modbus send buffer
	BYTE* pByArr = (BYTE*)&pFrame->sendFrame;//Note: this needs #pragma pack(1), or else will be problems!!!
	BYTE* pSendBuf = pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	memcpy(pSendBuf, pByArr, 8);
	// *pSendBuf++ = pWhim_Rough->meterData[nWhichWh].byModbusAddress;
	// *pSendBuf++ = 0x03; //function code
	// int nExpectDataLen;
	// if(nGetType == MD_GET_PHASE_VALUES) //MD_GET_PHASE_VALUES
	// {
	// 	*pSendBuf++ = 0x08; //start-hibyte
	// 	*pSendBuf++ = 0x3E; //start-lowbyte
	// 	*pSendBuf++ = 0x00; //lenth-hibyte
	// 	*pSendBuf++ = 20*2; //length-lowbyte (16*4), every reister has 2 bytes
	// 	nExpectDataLen = 40;
	// }
	USHORT usCRC = ModbusCRC(pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 6);
	pSendBuf += 6;
	pSendBuf[0] = (BYTE)(usCRC >> 8); 
	pSendBuf[1] = (BYTE)(usCRC & 0xFF);	
	//now send out the data to meter
	int nWriteBytes = pComm->Write(hPort, pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 8);
	TRACEX("nWriteBytes = [%d]\n",nWriteBytes);
	if(nWriteBytes < 8)
	{
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}
	else
	{
		// if (pFrame->frameType == ACREL_GET_TYPE_SUM_RTkWh_AC
		// 	|| pFrame->frameType == ACREL_GET_TYPE_SUM_RVSkWh_AC
		// 	|| pFrame->frameType == ACREL_GET_TYPE_TOTALKVAR
		// 	|| pFrame->frameType == ACREL_GET_TYPE_SUM_RTkWh_RVSkWh_AC)
		// {
			// int k;
			// printf("Modbus-RTU YD write:");
			// for(k = 0; k < 8; k++)
			// {
			// 	printf("%02X-", pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType][k]);
			// }
			// printf("\n");
		// }
	}

	Sleep(50); 
	//now read response frame from meter
	BYTE *pRecvBuf = pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
	int n;
	int nReadBytes;
	//read back for 3 times
	for(n = 0; n < nReadTimes; n++)
	{
		nReadBytes = pComm->Read(hPort, pRecvBuf, RS485_RECV_BUF_LEN);
		// TRACED("nReadBytes = [%d]\n",nReadBytes);
		if(nReadBytes >= 7)
		{
			break;
		}
	}
	
	//check length
	if(nReadBytes < 7)
	{
		TRACEE("MOdbus read bytes[%d]\n", nReadBytes);
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}
	// else
	// {
		// if (pFrame->frameType == ACREL_GET_TYPE_SUM_RTkWh_AC
		// 	|| pFrame->frameType == ACREL_GET_TYPE_SUM_RVSkWh_AC
		// 	|| pFrame->frameType == ACREL_GET_TYPE_TOTALKVAR
		// 	|| pFrame->frameType == ACREL_GET_TYPE_SUM_RTkWh_RVSkWh_AC)
		// {
			// printf("Modbus-RTU YD read %d:", nReadBytes);
			// int k;
			// for(k = 0; k < nReadBytes; k++)
			// {
			// 	printf("%02X-", pRecvBuf[k]);
			// }
			// printf("\n");
		// }
	// }


	// Check Address
	if(*pRecvBuf != pFrame->sendFrame.byModbusAddr)
	{
		TRACEE("Modbus-RTU YD ErrAddress\n");
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check Function Code
	if(*(pRecvBuf + 1) != pFrame->sendFrame.byFunCode)
	{
		TRACEE("Modbus-RTU YD ErrFunCode\n");
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check Data Length
	int byDataLen = (int)((BYTE)(*(pRecvBuf + 2)));

	if(byDataLen != pFrame->nExpectAnswerLen)
	{
		TRACEE("Modbus-RTU YD ErrnExpectAnswerLen\n");
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check CRC
	int nCRC_StartCal = 3 + (int)byDataLen;
	int nCRC_2CalcLen = nCRC_StartCal;
	USHORT usReadCRC = (((USHORT)(*(pRecvBuf + nCRC_StartCal))) << 8) + (USHORT)(*(pRecvBuf + nCRC_StartCal + 1));
	USHORT usCalcCRC = ModbusCRC(pRecvBuf, nCRC_2CalcLen);
	if(usReadCRC != usCalcCRC)
	{
		TRACEE("CRC Error! Calc CRC = %04X\n", usCalcCRC);
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	//now parse data
	BYTE *pbyDataPos = pRecvBuf + 3;
	VAR_VALUE varV; 
	int m;
	int nPos = 0;
	UINT uVal;
	SHORT sTemp;
	float fTemp;
	double dTemp;

	if (pFrame->frameType == ACREL_GET_TYPE_SUM_RTkWh_AC
		|| pFrame->frameType == ACREL_GET_TYPE_SUM_RVSkWh_AC
		|| pFrame->frameType == ACREL_GET_TYPE_TOTALKVAR)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			memcpy(&dTemp, reverseBytes(pbyDataPos, sizeof(double)), sizeof(double));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)dTemp;
			// printf("fValue = [%f]\n",pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue); 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			//pbyDataPos += 8;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == ACREL_GET_TYPE_CURRENTPOSDEMAND)   
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			memcpy(&sTemp, reverseBytes(pbyDataPos, sizeof(SHORT)), sizeof(SHORT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = sTemp / pFrame->nVal_OffSet; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == ACREL_GET_TYPE_MAX_POS_DEMAND
				||pFrame->frameType == ACREL_GET_TYPE_SUM_RTkWh_RVSkWh_AC)   
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			memcpy(&fTemp, reverseBytes(pbyDataPos, sizeof(float)), sizeof(float));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = fTemp; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 4;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == ACREL_GET_TYPE_ACTPOWER_AC
			|| pFrame->frameType == ACREL_GET_TYPE_REACTPOWER
			|| pFrame->frameType == ACREL_GET_TYPE_FACTOR)	//映射不对应  A/B/C/总
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			// tmNow = GetCurrentTime();
			if(nSub_Idx == (pFrame->nSignalNum - 1))//总
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue = FourBytesToFloat(pbyDataPos); //*(float * )reverseBytes(pbyDataPos,4); 
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue /= pFrame->nVal_OffSet;
				if (pFrame->frameType != ACREL_GET_TYPE_FACTOR)
				{
					pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
				}
				// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue);
			}
			else
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos); //*(float * )reverseBytes(pbyDataPos,4); 
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
				if (pFrame->frameType != ACREL_GET_TYPE_FACTOR)
				{
					pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
				}
				// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
			}
			pbyDataPos += 4;
		}
	}
	else if (pFrame->frameType == ACREL_GET_TYPE_FREQ)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)   
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			pbyDataPos += 4;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == ACREL_GET_TYPE_VOLT
				|| pFrame->frameType == ACREL_GET_TYPE_CURR)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)   
		{
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			if (pFrame->frameType == ACREL_GET_TYPE_CURR)
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
			}
			else
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			}
			pbyDataPos += 4;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	
	// Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;

}

static BOOL whMeter_Modbus_ACREL_Sample(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh)
{
        //printf("whMeter_Modbus_YD_Sample nSampleTimes = [%d]\n",pWhim_Rough->meterData[nWhichWh].nSampleTimes);
        int nSampRTN = ERR_SAMPLE_OK;
        // TRACEI("有功功率  time[%0.3f]...\n",GetCurrentTime());
        nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_ACTPOWER_AC, WHMETER_ROUGH_ACTPower_PhA);
        if(nSampRTN != ERR_SAMPLE_OK)
        {
                return nSampRTN;
        }

        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (4 + nWhichWh))
        {
                // TRACEI("电流...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_CURR, WHMETER_ROUGH_RT_CURR_A);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (5 + nWhichWh))
        {
                // TRACEI("电压...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_VOLT, WHMETER_ROUGH_RT_VOLT_A);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (6 + nWhichWh))
        {
                // TRACEI("无功功率...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_REACTPOWER, WHMETER_ROUGH_ReActPower_PhA);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (7 + nWhichWh))
        {
                // TRACEI("频率...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_FREQ, WHMETER_ROUGH_RT_Freq);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (8 + nWhichWh))
        {
                // TRACEI("功率因数...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_FACTOR, WHMETER_ROUGH_PowerFactor_PhA);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (9 + nWhichWh))
        {
                // TRACEI("正向有功最大需量...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_MAX_POS_DEMAND, WHMETER_ROUGH_PMAXDEMAND);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (10 + nWhichWh))
        {
                // TRACEI("当前需量...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_CURRENTPOSDEMAND, WHMETER_ROUGH_CURRENTPOSDEMAND);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (15 + nWhichWh))
        {
                // TRACEI("正向有功电能...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_SUM_RTkWh_AC, WHMETER_ROUGH_RTkWh_AC);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (16 + nWhichWh))
        {
                // TRACEI("反向电能...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_SUM_RVSkWh_AC, WHMETER_ROUGH_RT_RVSkWh_AC);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (17 + nWhichWh))
        {
                // TRACEI("总无功电能...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_TOTALKVAR, WHMETER_ROUGH_RTkvar);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (18 + nWhichWh))
        {
                // TRACEI("尖峰平谷电能...\n");
                nSampRTN = ACREL_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, ACREL_GET_TYPE_SUM_RTkWh_RVSkWh_AC, WHMETER_ROUGH_SUM_RTkWh_AC);
        }

        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes <= (19 + nWhichWh))
        {
                pWhim_Rough->meterData[nWhichWh].nSampleTimes++;
                return ERR_SAMPLE_OK; // old return nSampRTN; 251110改为通信失败只判断功率数据帧
        }

        pWhim_Rough->meterData[nWhichWh].nSampleTimes = 0;
        
        return ERR_SAMPLE_OK; // old return nSampRTN; 251110改为通信失败只判断功率数据帧
}

static int DTSD_ModBus_whMeter_SigSamp(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh, WH_FARAME_TYPE emCmdType, WH_ROUGH_SIG emSigType)
{
	int nSub_Idx = 0;
	int nReadTimes = 0;
	double tmNow;
	COMMPORT* pComm = NULL;
	HANDLE 	hPort = NULL;
	if(pWhim_Rough->bEnableQuickResChannel)
	{
		nReadTimes = 1;
	}
	else
	{
		nReadTimes = 3;
	}

	if (pWhim_Rough->meterData[nWhichWh].bPhysicalInterType == RS485_1)
	{
		pComm = ((SAMPLER*)pWhim_Rough->hSampler)->pUsedCommPort;
		hPort = pWhim_Rough->hPort;
	}
	else if(pWhim_Rough->meterData[nWhichWh].bPhysicalInterType == RS485_2)
	{
		pComm = s_pRS485_2_Comm;
		hPort = hRS485_2_Port;
	}
	//Clear buffer
	pComm->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);
	pComm->Control(hPort, COMM_SET_SERIAL_ATTRIB, (void*)pWhim_Rough->meterData[nWhichWh].szBaudrateSett,
		strlen(pWhim_Rough->meterData[nWhichWh].szBaudrateSett));

	Sleep(10);

	DTSD_MD_FRAME_INFO* pFrame = &pWhim_Rough->frm_DTSD_MD_whFrames[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType][emCmdType];

	pFrame->sendFrame.byModbusAddr = pWhim_Rough->meterData[nWhichWh].byModbusAddress;

	//Now prepare modbus send buffer
	BYTE* pByArr = (BYTE*)&pFrame->sendFrame;//Note: this needs #pragma pack(1), or else will be problems!!!
	BYTE* pSendBuf = pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	memcpy(pSendBuf, pByArr, 8);

	USHORT usCRC = ModbusCRC(pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 6);
	pSendBuf += 6;
	pSendBuf[0] = (BYTE)(usCRC >> 8); 
	pSendBuf[1] = (BYTE)(usCRC & 0xFF);	
	//now send out the data to meter
	int nWriteBytes = pComm->Write(hPort, pWhim_Rough->abySendBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType], 8);
	TRACEX("nWriteBytes = [%d]\n",nWriteBytes);
	if(nWriteBytes < 8)
	{
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}


	Sleep(50); 
	//now read response frame from meter
	BYTE *pRecvBuf = pWhim_Rough->abyRcvBuf[pWhim_Rough->meterData[nWhichWh].bPhysicalInterType];
	//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
	int n;
	int nReadBytes;

	//read back for 3 times
	for(n = 0; n < nReadTimes; n++)
	{
		nReadBytes = pComm->Read(hPort, pRecvBuf, RS485_RECV_BUF_LEN);
		// TRACED("nReadBytes = [%d]\n",nReadBytes);
		if(nReadBytes >= 7)
		{
			break;
		}
	}
	

	//check length
	if(nReadBytes < 7)
	{
		TRACEE("MOdbus read bytes[%d]\n", nReadBytes);
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}

	// Check Address
	if(*pRecvBuf != pFrame->sendFrame.byModbusAddr)
	{
		TRACEE("Modbus-RTU YD ErrAddress\n");
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check Function Code
	if(*(pRecvBuf + 1) != pFrame->sendFrame.byFunCode)
	{
		TRACEE("Modbus-RTU YD ErrFunCode\n");
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check Data Length
	int byDataLen = (int)((BYTE)(*(pRecvBuf + 2)));

	if(byDataLen != pFrame->nExpectAnswerLen)
	{
		TRACEE("Modbus-RTU YD ErrnExpectAnswerLen\n");
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// Check CRC
	int nCRC_StartCal = 3 + (int)byDataLen;
	int nCRC_2CalcLen = nCRC_StartCal;
	USHORT usReadCRC = (((USHORT)(*(pRecvBuf + nCRC_StartCal))) << 8) + (USHORT)(*(pRecvBuf + nCRC_StartCal + 1));
	USHORT usCalcCRC = ModbusCRC(pRecvBuf, nCRC_2CalcLen);
	if(usReadCRC != usCalcCRC)
	{
		TRACEE("CRC Error! Calc CRC = %04X\n", usCalcCRC);
		// Mutex_Unlock(hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	//now parse data
	BYTE *pbyDataPos = pRecvBuf + 3;
	int nTemp;
	UINT uTemp;
	SHORT sTemp;
	USHORT usTemp;
	float fTemp;
	double dTemp;

	if(pFrame->frameType == DTSD_GET_TYPE_VOLT
		||pFrame->frameType == DTSD_GET_TYPE_CURR)   
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++) 
		{
			memcpy(&sTemp, reverseBytes(pbyDataPos, sizeof(SHORT)), sizeof(SHORT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)sTemp;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			if(pFrame->frameType == DTSD_GET_TYPE_VOLT)
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			}
			else if(pFrame->frameType == DTSD_GET_TYPE_CURR)   
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor;
			}
			pbyDataPos += 2;
		}
	}
	else if(pFrame->frameType == DTSD_GET_TYPE_ACTPOWER_AC
		||pFrame->frameType == DTSD_GET_TYPE_REACTPOWER)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++) 
		{
			memcpy(&nTemp, reverseBytes(pbyDataPos, sizeof(int)), sizeof(int));
			if(nSub_Idx == (pFrame->nSignalNum - 1))//总
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue = (float)nTemp;
				// pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue = FourBytesToFloat(pbyDataPos);
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue /= pFrame->nVal_OffSet;
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
				// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue);
			}
			else
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)nTemp;
				// pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			}
			pbyDataPos += 4;
		}
	}   
	else if(pFrame->frameType == DTSD_GET_TYPE_FACTOR)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++) 
		{
			memcpy(&sTemp, reverseBytes(pbyDataPos, sizeof(SHORT)), sizeof(SHORT));
			if(nSub_Idx == (pFrame->nSignalNum - 1))//总
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue = (float)sTemp;
				// pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue = FourBytesToFloat(pbyDataPos);
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx-pFrame->nSignalNum].fValue /= pFrame->nVal_OffSet;
			}
			else
			{
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)sTemp;
				// pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
				pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			}
			pbyDataPos += 2;
		}
	}
	else if(pFrame->frameType == DTSD_GET_TYPE_FREQ)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++) 
		{
			memcpy(&usTemp, reverseBytes(pbyDataPos, sizeof(USHORT)), sizeof(USHORT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)usTemp;
			// pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = FourBytesToFloat(pbyDataPos);
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet;
			pbyDataPos += 2;
		}
	}
	else if (pFrame->frameType == DTSD_GET_TYPE_TOTALKVAR)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			memcpy(&uTemp, reverseBytes(pbyDataPos, sizeof(UINT)), sizeof(UINT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)uTemp;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 4;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	else if (pFrame->frameType == DTSD_GET_TYPE_MAX_POS_DEMAND)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			memcpy(&usTemp, reverseBytes(pbyDataPos, sizeof(USHORT)), sizeof(USHORT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)usTemp;
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
            pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 2;
		}
	}
	else if (pFrame->frameType == DTSD_GET_TYPE_CURRENTPOSDEMAND)   
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			memcpy(&usTemp, reverseBytes(pbyDataPos, sizeof(USHORT)), sizeof(USHORT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)usTemp; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
            pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 2;
		}
	}
	else if (pFrame->frameType == DTSD_GET_TYPE_SUM_RTkWh_RVSkWh_AC
			|| pFrame->frameType == DTSD_GET_TYPE_SUM_RTkWh_AC
			|| pFrame->frameType == DTSD_GET_TYPE_SUM_RVSkWh_AC)
	{
		pWhim_Rough->meterData[nWhichWh].varWHMeterRough[WHMETER_ERR_WORD].ulValue = 0;
		for(nSub_Idx = 0; nSub_Idx < pFrame->nSignalNum; nSub_Idx++)  
		{
			// memcpy(&fTemp, reverseBytes(pbyDataPos, sizeof(float)), sizeof(float));
			memcpy(&uTemp, reverseBytes(pbyDataPos, sizeof(UINT)), sizeof(UINT));
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)uTemp; 
			// pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue = (float)fTemp; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue /= pFrame->nVal_OffSet; 
			pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue *= pWhim_Rough->meterData[nWhichWh].fACMeter_PowerFactor * pWhim_Rough->meterData[nWhichWh].fACMeter_VoltPTFactor;
			pbyDataPos += 4;
			// tmNow = GetCurrentTime();
			// TRACEW("time[%0.3f] emSigType[%d] nSub_Idx[%d] fValue=[%0.3f]\n",tmNow,emSigType,nSub_Idx,pWhim_Rough->meterData[nWhichWh].varWHMeterRough[emSigType+nSub_Idx].fValue);
		}
	}
	

	// Mutex_Unlock(hSamplerLock);
	return ERR_SAMPLE_OK;

}


static BOOL whMeter_Modbus_DTSD_Sample(RS485_SAMPLER_DATA* pWhim_Rough, int nWhichWh)
{
        //printf("whMeter_Modbus_YD_Sample nSampleTimes = [%d]\n",pWhim_Rough->meterData[nWhichWh].nSampleTimes);
        int nSampRTN = ERR_SAMPLE_OK;
        // TRACEI("有功功率  time[%0.3f]...\n",GetCurrentTime());
        nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_ACTPOWER_AC, WHMETER_ROUGH_ACTPower_PhA);
        if(nSampRTN != ERR_SAMPLE_OK)
        {
            return nSampRTN;
        }

        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (4 + nWhichWh))
        {
                // TRACEI("电流...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_CURR, WHMETER_ROUGH_RT_CURR_A);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (5 + nWhichWh))
        {
                // TRACEI("电压...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_VOLT, WHMETER_ROUGH_RT_VOLT_A);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (6 + nWhichWh))
        {
                // TRACEI("无功功率...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_REACTPOWER, WHMETER_ROUGH_ReActPower_PhA);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (7 + nWhichWh))
        {
                // TRACEI("频率...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_FREQ, WHMETER_ROUGH_RT_Freq);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (8 + nWhichWh))
        {
                // TRACEI("功率因数...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_FACTOR, WHMETER_ROUGH_PowerFactor_PhA);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (9 + nWhichWh))
        {
                // TRACEI("正向有功最大需量...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_MAX_POS_DEMAND, WHMETER_ROUGH_PMAXDEMAND);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (10 + nWhichWh))
        {
                // TRACEI("当前需量...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_CURRENTPOSDEMAND, WHMETER_ROUGH_CURRENTPOSDEMAND);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (15 + nWhichWh))
        {
                
                // TRACEI("总无功电能...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_TOTALKVAR, WHMETER_ROUGH_RTkvar);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (16 + nWhichWh))
        {
                // TRACEI("尖峰平谷电能...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_SUM_RTkWh_RVSkWh_AC, WHMETER_ROUGH_SUM_RTkWh_AC);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (17 + nWhichWh))
        {
                // TRACEI("正向有功电能...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_SUM_RTkWh_AC, WHMETER_ROUGH_RTkWh_AC);
        }
        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (18 + nWhichWh))
        {
                // TRACEI("反向有功电能...\n");
                nSampRTN = DTSD_ModBus_whMeter_SigSamp(pWhim_Rough, nWhichWh, DTSD_GET_TYPE_SUM_RVSkWh_AC, WHMETER_ROUGH_RT_RVSkWh_AC);
        }

        if(pWhim_Rough->meterData[nWhichWh].nSampleTimes <= (19 + nWhichWh))
        {
                pWhim_Rough->meterData[nWhichWh].nSampleTimes++;
                return ERR_SAMPLE_OK; // old return nSampRTN; 251110改为通信失败只判断功率数据帧
        }

        pWhim_Rough->meterData[nWhichWh].nSampleTimes = 0;
        
        return ERR_SAMPLE_OK; // old return nSampRTN; 251110改为通信失败只判断功率数据帧
}


/*********************************************************************************
*  
*  FUNCTION NAME :  Judge_IfNeedGetAccountKwh
*  CREATOR       :  
*  CREATED DATE  : 2022-09-1 19:13:53
*  DESCRIPTION   :  One Hour Period Get Account Kwh Data.
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Judge_IfNeedGetAccountKwh(IN RS485_SAMPLER_DATA* pWhim_Rough, IN int nWhichWh)
{
	if(nWhichWh != CAB_FIRST) //Note:Only Cabinet Meter Read Account Kwh.
	{
		return ;
	}

	int nSampRTN = ERR_SAMPLE_OK;
	time_t tmNow = _NOW_;
    struct tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    static int nLastGet_AccountHour = 0;
    int nCurr_Hour = gmTime.tm_hour;
	if(nCurr_Hour == nLastGet_AccountHour)
	{
		return;
	}

	nLastGet_AccountHour = nCurr_Hour;
	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH, WHMETER_ROUGH_LAST1_POS_SUM);
	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH, WHMETER_ROUGH_LAST1_NEG_SUM);
	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH, WHMETER_ROUGH_LAST2_POS_SUM);
	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH, WHMETER_ROUGH_LAST2_NEG_SUM);
	
	TRACE("nWhichWh=[%d] nLastGet_AccountHour=[%d]\n", nWhichWh, nLastGet_AccountHour);
	return;
}

static void Judge_IfNeedGetRelevantValue(IN RS485_SAMPLER_DATA* pWhim_Rough, IN int nWhichWh)
{	
	int nSampRTN = ERR_SAMPLE_OK;
	float fPcab = (fabs)(pWhim_Rough->meterData[CAB_FIRST].varWHMeterRough[WHMETER_ROUGH_RTACTPower].fValue);
	//Only Cab Meter And System On Idle Status
	if((nWhichWh != CAB_FIRST) && (fPcab > 10.0)) 
	{
		return;
	}
	

	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_TOTALKVAR_AC, WHMETER_ROUGH_RTkvar);//KvarH
	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_TOTALKVAR2_AC, WHMETER_ROUGH_RTkvar2);//KvarH

	nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_FREQ, WHMETER_ROUGH_RT_Freq);
 	
	//TRACE("Relevant nWhichWh=[%d]  RTACTPower=[%f]\n",  nWhichWh, fPcab);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  whMeter_Sample
*  CREATOR       :  
*  CREATED DATE  : 2022-09-1 19:13:53
*  DESCRIPTION   :  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int whMeter_Sample(IN RS485_SAMPLER_DATA* pWhim_Rough, IN int nWhichWh)
{
	int nSampRTN = ERR_SAMPLE_OK;
	BOOL	 bNeedReSamp = FALSE;
	if(pWhim_Rough->meterData[nWhichWh].emMeterType == METER_TYPR_ac)
	{
		if(pWhim_Rough->meterData[nWhichWh].emACMeter_ProtocolType == ACMETER_PROTOCOL_MODBUS_EM737)
		{
			return whMeter_EM737_Sample(pWhim_Rough, nWhichWh);
		}
		else if(pWhim_Rough->meterData[nWhichWh].emACMeter_ProtocolType == ACMETER_PROTOCOL_MODBUS_RM_PRIVATE)
		{
			//Cycle Read Remote AC Meter Should less then 2 Seconds,ohterwise will read error data
			return whMeter_RM_Private_Sample(pWhim_Rough, nWhichWh);
		}
		else if(pWhim_Rough->meterData[nWhichWh].emACMeter_ProtocolType == ACMETER_PROTOCOL_MODBUS_RTU_YD)
		{
			return whMeter_Modbus_YD_Sample(pWhim_Rough, nWhichWh);
		}
		else if(pWhim_Rough->meterData[nWhichWh].emACMeter_ProtocolType == ACMETER_PROTOCOL_MODBUS_RTU_ACREL_ADL400)
		{
			return whMeter_Modbus_ACREL_Sample(pWhim_Rough, nWhichWh);
		}
		else if(pWhim_Rough->meterData[nWhichWh].emACMeter_ProtocolType == ACMETER_PROTOCOL_MODBUS_RTU_DTSD1352)
		{
			return whMeter_Modbus_DTSD_Sample(pWhim_Rough, nWhichWh);
		}

		nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_ACTPOWER_AC, WHMETER_ROUGH_RTACTPower);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		if(nSampRTN != ERR_SAMPLE_OK)
		{
			//pWhim_Rough->bNeedReconfig_whMeter = TRUE;
			return nSampRTN;
		}
		nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_REACTPOWER_AC, WHMETER_ROUGH_RTReActPower);
		// LJH241007 AntiPhase
		if (pWhim_Rough->needAntiOverLoad_Phase)
		{
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_CURR_PHASE_A, WHMETER_ROUGH_RT_CURR_A);
		}
		
		if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (5 + nWhichWh))
		{
			//TRACE("Crrent nWhichWh=[%d] nSampleTimes=[%d]\n", nWhichWh, pWhim_Rough->meterData[nWhichWh].nSampleTimes);
			// LJH241007 AntiPhase
			if (!pWhim_Rough->needAntiOverLoad_Phase)
			{
				nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_CURR_PHASE_A, WHMETER_ROUGH_RT_CURR_A);
			}
			//move by lpl 240704 for zhongshu grid meter need to get Pmaxdemand
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_PMAXDEMAND, WHMETER_ROUGH_PMAXDEMAND);
			//added by lpl 240823 for zhongshu grid meter need to get current pos demand
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_CURRENTPOSDEMAND, WHMETER_ROUGH_CURRENTPOSDEMAND);
			//nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_CURR_PHASE_B, WHMETER_ROUGH_RT_CURR_B);
			//nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_CURR_PHASE_C, WHMETER_ROUGH_RT_CURR_C);
		}

		if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (6 + nWhichWh))
		{
			//TRACE("Kwh nWhichWh=[%d] nSampleTimes=[%d]\n", nWhichWh, pWhim_Rough->meterData[nWhichWh].nSampleTimes);
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_TOTALWH_AC, WHMETER_ROUGH_RTkWh_AC);
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_RVSTATALWH, WHMETER_ROUGH_RT_RVSkWh_AC);
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_MULTI_TOTALWH, WHMETER_ROUGH_SUM_RTkWh_AC);
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_MULTI_RVSTATALWH, WHMETER_ROUGH_RT_SUM_RVSkWh_AC);

			//add by lpl 240802
			JudgeAndSaveToDB(pWhim_Rough,nWhichWh);

			//---John Added 2022-11-30.For Account Kwh.---//
			Judge_IfNeedGetAccountKwh(pWhim_Rough, nWhichWh);
		}

		if(pWhim_Rough->meterData[nWhichWh].nSampleTimes == (7 + nWhichWh))
		{
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_VOLT_PHASE_A, WHMETER_ROUGH_RT_VOLT_A);
			nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_POWERFACTOR, WHMETER_ROUGH_RT_PowerFactor);
			//nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_VOLT_PHASE_B, WHMETER_ROUGH_RT_VOLT_B);
			//nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_VOLT_PHASE_C, WHMETER_ROUGH_RT_VOLT_C);
		}

		//---2022-02-24 Due to Improve Meter Sample Speed
		if(pWhim_Rough->meterData[nWhichWh].nSampleTimes <= (8 + nWhichWh))
		{
			pWhim_Rough->meterData[nWhichWh].nSampleTimes++;
			return nSampRTN;
		}
		//TRACE("Volt nWhichWh=[%d] nSampleTimes=[%d]\n", nWhichWh, pWhim_Rough->meterData[nWhichWh].nSampleTimes);
		pWhim_Rough->meterData[nWhichWh].nSampleTimes = 0;

		Judge_IfNeedGetRelevantValue(pWhim_Rough, nWhichWh);
	}
	else
	{
		TRACEX("----Sampler[%d] -- Now begin sampling DC meter[%d]----\n", pWhim_Rough->nSamplerId, nWhichWh);
		//John Modify 20230810 remove DC Sample.
		/*nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_TOTALWH, WHMETER_ROUGH_RTkWh);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		if(nSampRTN != ERR_SAMPLE_OK)
		{
			//pWhim_Rough->bNeedReconfig_whMeter = TRUE;
			return nSampRTN;
		}

		nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_RVSTATALWH, WHMETER_ROUGH_RVSkWh);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;

		nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_VOLT_PHASE_A, WHMETER_ROUGH_RT_VOLT_DC);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		nSampRTN = whMeter_SigSamp(pWhim_Rough, nWhichWh, WH_FRAME_TYPE_GET_CURR_PHASE_A, WHMETER_ROUGH_RT_CURR_DC);
		bNeedReSamp = (nSampRTN == ERR_SAMPLE_NEED_RESAMP) ? TRUE : bNeedReSamp;
		if((!pWhim_Rough->bHasSampledWhOK) && (!bNeedReSamp))
		{
			pWhim_Rough->nSampleCount++;
			if(pWhim_Rough->nSampleCount >= 2)
			{
				pWhim_Rough->nSampleCount = 0;
				pWhim_Rough->bHasSampledWhOK = TRUE;
				//set a cmd to CCU to notice the kwh data ready
			}
		}*/
	}

	if(bNeedReSamp)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}
	
	return ERR_SAMPLE_OK;
}


