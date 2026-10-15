#include "imeu2_can.h"


#define GET_BIT(num, n) (((num) >> (n)) & 1)
#define SET_BIT(num, n) ((num) | (1 << (n)))

static TAG_IMEU2_PRO    sIMEU2ProData[_MAX_IMEU2_CAN_CMD];  

static USHORT CAN_StringToUShort(BYTE* strInput) //LSB
{
	USHORT usValue = ((USHORT)(strInput[0])) + (((USHORT)(strInput[1])) << 8);

	return usValue;
}

static SHORT CAN_StringToShort(BYTE* strInput) //LSB
{
	SHORT sValue = (SHORT)( ((USHORT)strInput[0]) | ((USHORT)(strInput[1] << 8)) );

	return sValue;
}


static UINT CAN_StringToUint(BYTE* strInput) //LSB
{
	UINT uValue = ((UINT)(strInput[0])) + (((UINT)(strInput[1])) << 8)
					+ (((UINT)(strInput[2])) << 16) + (((USHORT)(strInput[3])) << 24);

	return uValue;
}


void initIMEU2CAN(IMEU2_CAN_ROUGH_DATA IMEU2CANRoughData[],  int iNum)
{
	TAG_IMEU2_PRO *p = &sIMEU2ProData[0];
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG1, 0x14, 0xCB, Parse_S2C_MSG1);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG2, 0x14, 0xCC, Parse_S2C_MSG2);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG3, 0x14, 0xCD, Parse_S2C_MSG3);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG4, 0x14, 0xCE, Parse_S2C_MSG4);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG6, 0x14, 0xD0, Parse_S2C_MSG6);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG11, 0x14, 0xD5, Parse_S2C_MSG11);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG12, 0x14, 0xD6, Parse_S2C_MSG12);
	INIT_YFYBATT_CMD(p, _IMEU2_CAN_R_CMD_S2C_MSG13, 0x14, 0xD7, Parse_S2C_MSG13);

	int n = 0;
	for(n = 0; n < iNum; n++)
	{
		IMEU2CANRoughData[n].nInterrupt_Times = 0;
		IMEU2CANRoughData[n].bFoundReply = FALSE;
		IMEU2CANRoughData[n].bIsCommFail = FALSE;
	}

	return;
}


extern BOOL YFY_CompBMSGrpMapIdx(BYTE nBMSGrpAddr, int *nBMSGrpMapIdx);

BOOL judgeAndParseIMEU2MsgID(IMEU2_CAN_FRAME* pFrameReply)
{
	pFrameReply->byFuncCode = pFrameReply->byRawData[1];
	pFrameReply->bySubIndex = pFrameReply->byRawData[2];

	pFrameReply->byDestAddr = pFrameReply->byRawData[3];
	pFrameReply->bySourceAddr = pFrameReply->byRawData[4];
	
	if (!YFY_CompBMSGrpMapIdx(pFrameReply->bySourceAddr, &(pFrameReply->bySourceIdx)))
	{
		return FALSE;
	}

	int n = 0;
	for(n = 0; n < _MAX_IMEU2_CAN_CMD; n++)
	{
		if((pFrameReply->byFuncCode == sIMEU2ProData[n].nFuncCode) && (pFrameReply->bySubIndex == sIMEU2ProData[n].nSubIndex))
		{
			pFrameReply->emMsgType = sIMEU2ProData[n].emMsgType;
			memcpy(pFrameReply->byData, pFrameReply->byRawData + 5, CAN_DATA_AREA_LEN);

			return TRUE;
		}
	}
	
	return FALSE;
}

void processIMEU2CANData(IMEU2_CAN_ROUGH_DATA *pIMEU2CANRoughData, IMEU2_CAN_FRAME* pFrameReply)
{
	sIMEU2ProData[pFrameReply->emMsgType].pfnCmdHandler(pIMEU2CANRoughData, pFrameReply);
	sIMEU2ProData[pFrameReply->bySourceIdx].bFoundReply = TRUE;

	return;
}


BOOL Parse_S2C_MSG1(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG1	0x14CByyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;

	int i = 0;
	for (i = 0; i < 8; ++i)
	{
		pBlobIMEU2_V20->fPT1000Temperature[i] = (float)pFrameReply->byData[i];
	}
	
	return TRUE;
}

BOOL Parse_S2C_MSG2(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG2	0x14CCyyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;

	pBlobIMEU2_V20->fPT1000Temperature[8] = (float)pFrameReply->byData[0];
	pBlobIMEU2_V20->fPT1000Temperature[9] = (float)pFrameReply->byData[1];
	
	pBlobIMEU2_V20->uFloodFreq = CAN_StringToUShort(&pFrameReply->byData[2]);
	pBlobIMEU2_V20->uPressureSen = CAN_StringToUShort(&pFrameReply->byData[4]);
	pBlobIMEU2_V20->uVibrationSen = CAN_StringToUShort(&pFrameReply->byData[6]);
	
	return TRUE;
}

BOOL Parse_S2C_MSG3(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG3 0x14CDyyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;

	pBlobIMEU2_V20->fTemperature[0] = (float)pFrameReply->byData[0];
	pBlobIMEU2_V20->fTemperature[1] = (float)pFrameReply->byData[2];
	pBlobIMEU2_V20->fTemperature[2] = (float)pFrameReply->byData[4];

	pBlobIMEU2_V20->fHumidity[0] = (float)pFrameReply->byData[1];
	pBlobIMEU2_V20->fHumidity[1] = (float)pFrameReply->byData[3];
	pBlobIMEU2_V20->fHumidity[2] = (float)pFrameReply->byData[5];

	pBlobIMEU2_V20->fAirTemp = (float)pFrameReply->byData[6];
	pBlobIMEU2_V20->fAirHum = (float)pFrameReply->byData[7];

	return TRUE;
}

BOOL Parse_S2C_MSG4(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG4 0x14CEyyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;

	pBlobIMEU2_V20->fCombGasPotency[0] = (float)CAN_StringToUShort(&pFrameReply->byData[0]) * 0.01;
	pBlobIMEU2_V20->fCombGasPotency[1] = (float)CAN_StringToUShort(&pFrameReply->byData[2]) * 0.01;
	pBlobIMEU2_V20->fCombGasPotency[2] = (float)CAN_StringToUShort(&pFrameReply->byData[4]) * 0.01;

	pBlobIMEU2_V20->nExhaustFanSpeed = CAN_StringToUShort(&pFrameReply->byData[6]);

	return TRUE;
}



BOOL Parse_S2C_MSG6(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG6 0x14D0yyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;

	pBlobIMEU2_V20->emDI1_Fire = GET_BIT(pFrameReply->byData[0], 0);
	pBlobIMEU2_V20->emDI2_Smoke = GET_BIT(pFrameReply->byData[0], 1);
	pBlobIMEU2_V20->emDI3_DoorOpen = GET_BIT(pFrameReply->byData[0], 2);
	pBlobIMEU2_V20->emDI4_FlammableGas = GET_BIT(pFrameReply->byData[0], 3);
	pBlobIMEU2_V20->emDI5Dump = GET_BIT(pFrameReply->byData[0], 4);
	pBlobIMEU2_V20->emDI6_ShutterClosedOver = GET_BIT(pFrameReply->byData[0], 5);
	pBlobIMEU2_V20->emFlood = GET_BIT(pFrameReply->byData[0], 7);

	pBlobIMEU2_V20->emTempHumi_CommFail[0] = GET_BIT(pFrameReply->byData[1], 0);

	pBlobIMEU2_V20->emGasSensor_CommFail[0] = GET_BIT(pFrameReply->byData[2], 1);

	pBlobIMEU2_V20->emShutterOpenedOver = GET_BIT(pFrameReply->byData[4], 0);

	pBlobIMEU2_V20->byCtrlDoLighting = GET_BIT(pFrameReply->byData[5], 0);
	pBlobIMEU2_V20->byCtrlDoZHSAlarm = GET_BIT(pFrameReply->byData[5], 1);

	pBlobIMEU2_V20->bySPDAlarm = GET_BIT(pFrameReply->byData[5], 2);

	return TRUE;
	
}

BOOL Parse_S2C_MSG11(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG11	0x14D5yyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;

	pBlobIMEU2_V20->nHumiTempSensorNumb_Cfg = pFrameReply->byData[1];
	pBlobIMEU2_V20->nCombGasSensorEnable_Cfg = pFrameReply->byData[2];
	pBlobIMEU2_V20->nCombGasSensorType_Cfg = pFrameReply->byData[3];
	pBlobIMEU2_V20->nCombGasSensorNumb_Cfg = pFrameReply->byData[4];
	pBlobIMEU2_V20->nFireProtectType = pFrameReply->byData[5];
	pBlobIMEU2_V20->byHydrogenRemovalIntervalDay = pFrameReply->byData[6];
	pBlobIMEU2_V20->HydrogenRemovalIntervalMinute = pFrameReply->byData[7];

	return TRUE;
}

BOOL Parse_S2C_MSG12(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG12	0x14D6yyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;
	
	pBlobIMEU2_V20->byLouverActioTime = pFrameReply->byData[0];

	return TRUE;
}


BOOL Parse_S2C_MSG13(IMEU2_CAN_ROUGH_DATA* pIMEU2CANRoughData, IMEU2_CAN_FRAME *pFrameReply)
{
	//S2C_MSG13	0x14D7yyxx
	A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &pIMEU2CANRoughData->blobIMEU2_V20;
	
	if (0xC1 != pFrameReply->byData[0])
	{
		return FALSE;
	}
	
	BYTE byFrameSubId = pFrameReply->byData[1];
	BYTE byCRC = pFrameReply->byData[7];

	if (byFrameSubId >= 0  && byFrameSubId < MAX_MSG13_SUBID_NUM)
	{
		USHORT usHardwareVersion = (pFrameReply->byData[2] << 8 | pFrameReply->byData[3]);
		snprintf(pBlobIMEU2_V20->szHardwareVer, 16, "A%d", usHardwareVersion);

		USHORT usSoftwareVersion = (pFrameReply->byData[4] << 8 | pFrameReply->byData[5]);
		snprintf(pBlobIMEU2_V20->szSoftwareVer, 16, "V%d.%d", usSoftwareVersion / 100, usSoftwareVersion % 100);
	}

	return TRUE;
}


void IMEU2CANRawData2Rough(IMEU2_CAN_ROUGH_DATA IMEU2CANRoughData[], int iNum)
{
	int n = 0;
	VAR_VALUE varV;

	for(n = 0; n < iNum; n++)
	{
		SAMPLER* pSampler = (SAMPLER*)IMEU2CANRoughData[n].hSampler;
		varV.emValue = (IMEU2CANRoughData[n].bIsCommFail == TRUE) ? 1 : 0;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + n, 493, varV);
	}

	return;
}


//void Push_STBatt_AllData
void pushIMEU2CANData(IMEU2_CAN_ROUGH_DATA IMEU2CANRoughData[], int iNum)
{
	int i = 0, n = 0;
	int emAlmValue_DI1, emAlmValue_DI2, emAlmValue_DI3, emAlmValue_DI4, emFlood;
	VAR_VALUE varV;
	VAR_VALUE varStuff;

	for (i = 0; i < iNum; ++i)
	{
		SAMPLER* pSampler = (SAMPLER*)IMEU2CANRoughData[i].hSampler;
		DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
		
		A_BLOB_BATTCAB_IMEU2_V20* pBlobIMEU2_V20 = &IMEU2CANRoughData[i].blobIMEU2_V20;

		//	DI1-Fire Alarm
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 601 + i, 460, &varV, 4);
		emAlmValue_DI1 = (pBlobIMEU2_V20->emDI1_Fire == varV.emValue) ? 1 : 0;
		varStuff.emValue = emAlmValue_DI1;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 452, varStuff);

		//	DI2-Smoke Alarm
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 601 + i, 461, &varV, 4);
		emAlmValue_DI2 = (pBlobIMEU2_V20->emDI2_Smoke == varV.emValue) ? 1 : 0;
		varStuff.emValue = emAlmValue_DI2;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i ,453, varStuff);
		
		//	DI3-Door Open Alarm
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 601 + i, 462, &varV, 4);
		if(varV.emValue == 2)
		{
			emAlmValue_DI3 = 0;
		}
		else
		{
			emAlmValue_DI3 = (pBlobIMEU2_V20->emDI3_DoorOpen == varV.emValue) ? 1 : 0;
		}
		varStuff.emValue = emAlmValue_DI3;
		pBlobIMEU2_V20->emDI3_DoorOpen = varStuff.emValue;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 454, varStuff);


		//	DI4-Flammable Gas Alarm
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 601 + i, 463, &varV, 4);
		emAlmValue_DI4 = (pBlobIMEU2_V20->emDI4_FlammableGas == varV.emValue) ? 1 : 0;
		varStuff.emValue = emAlmValue_DI4;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 455, varStuff);

		//	DI5-Dump Alarm
		varStuff.emValue = pBlobIMEU2_V20->emDI5Dump;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 1602, varStuff);

		//Electric Shutters Completely Closed
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 601 + i, 491, &varV, 4);
		varStuff.emValue = (pBlobIMEU2_V20->emDI6_ShutterClosedOver == varV.emValue) ? 1 : 0;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 459, varStuff);

		//Electric Shutters Completely Opened
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 601 + i, 492, &varV, 4);
		varStuff.emValue = (pBlobIMEU2_V20->emShutterOpenedOver == varV.emValue) ? 1 : 0;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 460, varStuff);

		varStuff.emValue = pBlobIMEU2_V20->emFlood;
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 461, varStuff);
		
		//Temp Humi Comm Fail
		for(n = 0; n < MAX_TEMPERATURE_HUMIDITY_NUM; n++)
		{
			varStuff.emValue = pBlobIMEU2_V20->emTempHumi_CommFail[n];
			Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 437 + n, varStuff);
		}

		//Gas Sensor Comm Fail
		for(n = 0; n < MAX_GASSENSOR_NUM; n++)
		{
			varStuff.emValue = pBlobIMEU2_V20->emGasSensor_CommFail[n];
			Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 433 + n, varStuff);
		}

		Push_BlobValue(SETTYPE_RAW_SP, pSampler, 601 + i, 450, \
		(void*)pBlobIMEU2_V20, sizeof(A_BLOB_BATTCAB_IMEU2_V20), BLOB_TYPE_BATTCAB_IMEU2_V20);

	}

	return;
}


