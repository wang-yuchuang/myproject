//#define _TRACE_PRINT_
#include "imeu3_can.h"


#define GET_BIT(num, n) (((num) >> (n)) & 1)
#define SET_BIT(num, n) ((num) | (1 << (n)))

static TAG_IMEU3_PRO    sIMEU3ProData[_MAX_IMEU3_CAN_CMD];
static BOOL             sIMEU3ProData_Initialized = FALSE;

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

static UINT CAN_StringToUint3Byte(BYTE* strInput) //LSB
{
	UINT usValue = (UINT)(((UINT)(strInput[0])) + (((UINT)(strInput[1])) << 8) + (((UINT)(strInput[2])) << 16));

	return usValue;
}


static UINT CAN_StringToUint(BYTE* strInput) //LSB
{
	UINT uValue = ((UINT)(strInput[0])) + (((UINT)(strInput[1])) << 8) + (((UINT)(strInput[2])) << 16) + (((UINT)(strInput[3])) << 24);

	return uValue;
}



void initIMEU3CAN(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[],  int iNum)
{
	TAG_IMEU3_PRO *p = &sIMEU3ProData[0];
	INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_TEMP_MSG1, 0x14, 0xCB, Parse_S2C_TEMP_MSG1);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_TEMP_MSG2, 0x14, 0xCC, Parse_S2C_TEMP_MSG2);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_DIDO_MSG1, 0x14, 0xCD, Parse_S2C_DIDO_MSG1);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_DIPWM_MSG1, 0x14, 0xCE, Parse_S2C_DIPWM_MSG1);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_DIPWM_MSG2, 0x14, 0xCF, Parse_S2C_DIPWM_MSG2);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_PRESEN_MSG1, 0x14, 0xD0, Parse_S2C_PRESEN_MSG1);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_PRESEN_MSG2, 0x14, 0xD1, Parse_S2C_PRESEN_MSG2);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_HEART_MSG, 0x14, 0xD2, Parse_S2C_HEART_MSG);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_TEMHUM_MSG, 0x14, 0xD3, Parse_S2C_TEMHUM_MSG);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_COMBUSTIBLE_MSG, 0x14, 0xD4, Parse_S2C_COMBUSTIBLE_MSG);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_ALARM_MSG, 0x14, 0xD5, Parse_S2C_ALARM_MSG);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_CONFIG_MSG, 0x14, 0xD6, Parse_S2C_CONFIG_MSG);
    INIT_YFYBATT_CMD(p, _IMEU3_CAN_R_CMD_S2C_DE_MSG, 0x14, 0xD7, Parse_S2C_DE_MSG);
    INIT_YFYBATT_CMD(p, _CAN_R_CMD_C2S_IMEU3UPGRADE,    0x1C, 0xF0,  Parse_C2S_IMEU3Upgrade);

    int n =0 ;
    for(n = 0; n < iNum; n++)
    {
        IMEU3CANRoughData[n].nInterrupt_Times = 0;
        IMEU3CANRoughData[n].bFoundReply = FALSE;
        IMEU3CANRoughData[n].bIsCommFail = FALSE;
    }

	return;
}


extern BOOL YFY_CompBMSGrpMapIdx(BYTE nBMSGrpAddr, int *nBMSGrpMapIdx);


BOOL judgeAndParseIMEU3MsgID(IMEU3_CAN_FRAME* pFrameReply)
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
	for(n = 0; n < _MAX_IMEU3_CAN_CMD; n++)
	{
		if((pFrameReply->byFuncCode == sIMEU3ProData[n].nFuncCode) && (pFrameReply->bySubIndex == sIMEU3ProData[n].nSubIndex))
		{
			pFrameReply->emMsgType = sIMEU3ProData[n].emMsgType;
			memcpy(pFrameReply->byData, pFrameReply->byRawData + 5, CAN_DATA_AREA_LEN);

			return TRUE;
		}
	}
	
	return FALSE;
}


extern BOOL YFY_Comp_IMEU3MapIdx(BYTE nIMEU3GrpAddr, int *nIMEU3GrpMapIdx);

BOOL judgeAndParseOtherIMEU3MsgID(IMEU3_CAN_FRAME* pFrameReply)
{
	pFrameReply->byFuncCode = pFrameReply->byRawData[1];
	pFrameReply->bySubIndex = pFrameReply->byRawData[2];

	pFrameReply->byDestAddr = pFrameReply->byRawData[3];
	pFrameReply->bySourceAddr = pFrameReply->byRawData[4];
	
	if (!YFY_Comp_IMEU3MapIdx(pFrameReply->bySourceAddr, &(pFrameReply->bySourceIdx)))
	{
		return FALSE;
	}

	int n = 0;
	for(n = 0; n < _MAX_IMEU3_CAN_CMD; n++)
	{
		if((pFrameReply->byFuncCode == sIMEU3ProData[n].nFuncCode) && (pFrameReply->bySubIndex == sIMEU3ProData[n].nSubIndex))
		{
			pFrameReply->emMsgType = sIMEU3ProData[n].emMsgType;
			memcpy(pFrameReply->byData, pFrameReply->byRawData + 5, CAN_DATA_AREA_LEN);

			return TRUE;
		}
	}
	
	return FALSE;
}

void processIMEU3CANData(IMEU3_CAN_ROUGH_DATA *pIMEU3CANRoughData, IMEU3_CAN_FRAME* pFrameReply)
{
	sIMEU3ProData[pFrameReply->emMsgType].pfnCmdHandler(pIMEU3CANRoughData, pFrameReply);
	sIMEU3ProData[pFrameReply->bySourceIdx].bFoundReply = TRUE;

	return;
}


BOOL Parse_S2C_TEMP_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_TEMP_MSG1	   0x14CByyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

	int i = 0;
	for (i = 0; i < MAX_IMEU3NTC_NUM; ++i)
	{
		pIMEU3->fNTC[i] = ((float)pFrameReply->byData[i]) - 50;
	}

    pIMEU3->fPT1000[0] = ((float)pFrameReply->byData[7]) - 50;

	return TRUE;
}

BOOL Parse_S2C_TEMP_MSG2(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_TEMP_MSG2	   0x14CCyyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    int i;
    for(i = 1; i < MAX_IMEU3PT1000_NUM; i++)
    {
        pIMEU3->fPT1000[i] = ((float)pFrameReply->byData[i - 1]) - 50;
    }

    pIMEU3->fTemperatureSen = ((float)pFrameReply->byData[4]) - 50;
    pIMEU3->fHumiditySen = ((float)pFrameReply->byData[5]) - 10;
    pIMEU3->uFloodFreq = CAN_StringToUShort(&pFrameReply->byData[6]);

    return TRUE;
}

BOOL Parse_S2C_DIDO_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_DIDO_MSG1	   0x14CDyyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->uDIStatus = CAN_StringToUint3Byte(&pFrameReply->byData[0]);
    pIMEU3->uDOStatus = CAN_StringToUint3Byte(&pFrameReply->byData[3]);
    pIMEU3->uX1Connect = (UINT)pFrameReply->byData[6];

    return TRUE;
}

BOOL Parse_S2C_DIPWM_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_DIPWM_MSG1	   0x14CEyyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    int i = 0;
    for(i = 0; i< 4; i++)
    {
        pIMEU3->uPWMFreq[i] = CAN_StringToUShort(&pFrameReply->byData[i*2]);
    }

    return TRUE;
}

BOOL Parse_S2C_DIPWM_MSG2(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_DIPWM_MSG2	   0x14CFyyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->uPWMFreq[4] = CAN_StringToUShort(&pFrameReply->byData[0]);
    pIMEU3->uPWMFreq[5] = CAN_StringToUShort(&pFrameReply->byData[2]);

    return TRUE;
}

BOOL Parse_S2C_PRESEN_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_PRESEN_MSG1	   0x14D0yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;
    int i = 0;
    for(i = 0; i < 4; i++)
    {
        pIMEU3->uPressureSen[i] = CAN_StringToUShort(&pFrameReply->byData[i*2]);
    }

    return TRUE;
}

BOOL Parse_S2C_PRESEN_MSG2(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_PRESEN_MSG2	   0x14D1yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->uPressureSen[4] = CAN_StringToUShort(&pFrameReply->byData[0]);
    pIMEU3->uPressureSen[5] = CAN_StringToUShort(&pFrameReply->byData[2]);

    return TRUE;
}

BOOL Parse_S2C_HEART_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_HEART_MSG	   0x14D2yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    // pIMEU3->uSoftwareVer = (UINT)pFrameReply->byData[0];
    pIMEU3->uSoftwareVer = (UINT)(pFrameReply->byData[0] | (pFrameReply->byData[1] << 8));//edit by kuangyue 20250326
    pIMEU3->uSystemRunTime = CAN_StringToUint(&pFrameReply->byData[2]);

    return TRUE;
}

BOOL Parse_S2C_TEMHUM_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_TEMHUM_MSG	   0x14D3yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;
    int i = 0; 
    for(i = 0; i < MAX_TemperatureHumiditySen_NUM; i++)
    {
        pIMEU3->fTemperatureValue[i] = ((float)pFrameReply->byData[i*2]) - 50;
        pIMEU3->fhumidityValue[i] = (float)pFrameReply->byData[i*2 + 1];
        
        TRACEI("fTemperatureValue[%f], fhumidityValue[%f]", pIMEU3->fTemperatureValue[i], pIMEU3->fhumidityValue[i]);
    }

    return TRUE;
}

BOOL Parse_S2C_COMBUSTIBLE_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_COMBUSTIBLE_MSG	0x14D4yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->fGasConcent1H2 = ((float)CAN_StringToUShort(&pFrameReply->byData[0]))* 0.01;
    pIMEU3->fGasConcent2C0 = ((float)CAN_StringToUShort(&pFrameReply->byData[2])) * 0.01;
    pIMEU3->fGasConcent3Other = ((float)CAN_StringToUShort(&pFrameReply->byData[4])) * 0.01;
    pIMEU3->fGasConcent4Other = ((float)CAN_StringToUShort(&pFrameReply->byData[6])) * 0.01;

    return TRUE;
}

BOOL Parse_S2C_ALARM_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_ALARM_MSG	   0x14D5yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->bySingleGasAlarm = GET_BIT(pFrameReply->byData[0], 0);
    pIMEU3->byTemperatureHumiAlarm = GET_BIT(pFrameReply->byData[0], 1);
    pIMEU3->byMultiGasAlarm = GET_BIT(pFrameReply->byData[0], 2);
    pIMEU3->byDehumidifierAlarm = GET_BIT(pFrameReply->byData[0], 3);
    pIMEU3->byImluAlarm = GET_BIT(pFrameReply->byData[0], 5);

    pIMEU3->usSenShortCircuitAlarm = CAN_StringToUShort(&pFrameReply->byData[1]);
    pIMEU3->usSenOpenCircuitAlarm = CAN_StringToUShort(&pFrameReply->byData[3]);

    TRACED("usSenShortCircuitAlarm[%d], usSenOpenCircuitAlarm[%d]", pIMEU3->usSenShortCircuitAlarm, pIMEU3->usSenOpenCircuitAlarm);

    return TRUE;
}

BOOL Parse_S2C_CONFIG_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_CONFIG_MSG	   0x14D6yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->bySystemRunTime = (SHORT)pFrameReply->byData[0];
    pIMEU3->byConfigDI = (SHORT)pFrameReply->byData[1];
    pIMEU3->byConfigPWM = (SHORT)pFrameReply->byData[2];
    pIMEU3->byConfigPressure = (SHORT)pFrameReply->byData[3];
    pIMEU3->byConfigGasSen = (SHORT)pFrameReply->byData[4];
    pIMEU3->byConfig485Sen = (SHORT)pFrameReply->byData[5];
    pIMEU3->byConfigDehumidifier = (SHORT)pFrameReply->byData[6];

    return TRUE;
}

BOOL Parse_S2C_DE_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //S2C_DE_MSG	   0x14D7yyxx
    IMEU3_CAN_DataInfo* pIMEU3 = &pIMEU3CANRoughData->stIMEU3DataInfo;

    pIMEU3->fHumidity = ((float)CAN_StringToUShort(&pFrameReply->byData[0])) * 0.1;
    pIMEU3->fDehumidifierStart = ((float)CAN_StringToUShort(&pFrameReply->byData[2])) * 0.1;
    pIMEU3->fDehumidifierStop = ((float)CAN_StringToUShort(&pFrameReply->byData[4])) * 0.1;
    pIMEU3->byTemperatureLoad = GET_BIT(pFrameReply->byData[6], 0);
    pIMEU3->byHumidityLoad = GET_BIT(pFrameReply->byData[6], 1);
    pIMEU3->byDehumidifierRunmode = GET_BIT(pFrameReply->byData[6], 3);
    pIMEU3->byDehumidifierFaultState = GET_BIT(pFrameReply->byData[6], 4);
    pIMEU3->byDehumidifierAddr = (SHORT)pFrameReply->byData[7];

    return TRUE;
}


BOOL Parse_C2S_IMEU3Upgrade(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply)
{
    //IMEU3 升级报文处理，会调用Parse_C2S_UpgradeStatu函数，直接返回即可
    return TRUE;
} 

void IMEU3ComStatus(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum)
{
	int n = 0;
    VAR_VALUE varV;

    for(n = 0; n < iNum; n++)
    {
        SAMPLER* pSampler = (SAMPLER*)IMEU3CANRoughData[n].hSampler;     
          
        varV.emValue = (IMEU3CANRoughData[n].bIsCommFail == TRUE) ? 1 : 0; //电池柜IMEU3  设备地址0x1F  通信状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + n, 1660, varV);
        
    }
	return;
}

void IMEU3PowerComStatus(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum)
{
    int n = 0;
    VAR_VALUE varV;
    SAMPLER* pSampler = (SAMPLER*)IMEU3CANRoughData[n].hSampler;

    for(n = 0; n < iNum; n++)
    {
        varV.emValue = (IMEU3CANRoughData[n].bIsCommFail == TRUE) ? 1 : 0;  //汇流柜IMEU3   设备地址0x1E  通信状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 621 + n, 2317, varV);
    }

}

//edit by Kuangyue 20251130
void pushMultifunctionCombinerCabinetIMEU3(IMEU3_CAN_ROUGH_DATA* pPowerCabinetIMEU3)
{
    SAMPLER* pSampler = (SAMPLER*)pPowerCabinetIMEU3->hSampler;
    
    IMEU3_CAN_DataInfo* pIMEU3 = &(pPowerCabinetIMEU3->stIMEU3DataInfo);
    
    VAR_VALUE varV;

    varV.fValue = pIMEU3->fNTC[0];  //箱体环境温度检测
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2300, varV);
    varV.fValue = pIMEU3->fNTC[1];  //箱体环境温度检测
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2301, varV);
    varV.fValue = pIMEU3->fNTC[2];   //母排温度检测
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2302, varV);
    varV.fValue = pIMEU3->fNTC[3];   //母排温度检测
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2303, varV);
    
    //解析22路DI口状态
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 3);    //DI4 防雷及其开关保护
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2304, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 4);    //DI5 急停开关反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2305, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 11);   //DI12 箱体门禁反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2306, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 5);    //DI6 电网侧开关合闸反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2307, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 16);   //DI17 QFR1
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2308, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 17);   //DI18 QFR2
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2309, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 18);   //DI19 QFR3 三路储能
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2310, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 19);   //DI20 QFR4
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2311, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 20);   //DI21 QFR5
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2312, varV);
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 21);   //DI22 QFR6 六路储能
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2313, varV);

    varV.ulValue = GET_BIT(pIMEU3->uDOStatus, 4);        //DO5 断路器状态反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2314, varV);

    varV.emValue = (pIMEU3->uFloodFreq > 0 && pIMEU3->uFloodFreq < 300) ? 1 : 0;//DO1 水浸传感器状态反馈 1:异常 0:正常
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2315, varV);

    varV.emValue = pIMEU3->byImluAlarm;    //IMLU2灯通信状态
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 621, 2316, varV);
    return;
}



void pushIMEU3toCloudPlatform(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], IMEU3_CAN_ROUGH_DATA PowerEU3_OtherEU3_Data[], ESS_DEVICE_TYPE emEssType, A_BLOB_BATTCAB_IMEU2_V20 blobIMEU2_V20[])
{
    int i = 0;

    for(i = 0; i < MAX_IMEU_NUM; i++)
    {
        SAMPLER* pSampler = (SAMPLER*)IMEU3CANRoughData[i].hSampler;
        IMEU3_CAN_DataInfo* pIMEU3_Bat = &IMEU3CANRoughData[0].stIMEU3DataInfo;
        IMEU3_CAN_DataInfo* pIMEU3_Power = &PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo;
        IMEU3_CAN_DataInfo* pIMEU3_STS = &PowerEU3_OtherEU3_Data[1].stIMEU3DataInfo;

        A_BLOB_BATTCAB_IMEU2_V20* pIMEU = &blobIMEU2_V20[i];
        int n;
        if(i == 0)   //电池仓EU3
        {
            pIMEU->emIMEU2_CommFail = (IMEU3CANRoughData[0].bIsCommFail == TRUE) ? 1 : 0;
            
            if(LIQUID_COOLING_BACKUP == emEssType)  //备电柜电池有4个PT1000温度值
            {
                for(n = 0; n < 4; ++n)
                {
                    pIMEU->fPT1000Temperature[n] =  pIMEU3_Bat->fPT1000[n + 1];
                }
            }
            else
            {
                pIMEU->fTemperature[0] = pIMEU3_Bat->fTemperatureValue[0];  //1#温度传感器温度值
                pIMEU->fHumidity[0] = pIMEU3_Bat->fhumidityValue[0];     //1#温度传感器湿度值
            }


            pIMEU->emDI3_DoorOpen = GET_BIT(pIMEU3_Bat->uDIStatus, 0);    //门禁

            //pIMEU->emDI4_FlammableGas = GET_BIT(pIMEU3_Bat->uDIStatus, 12);    //可燃气体检测

            pIMEU->fCombGasPotency[0] = pIMEU3_Bat->fGasConcent1H2;         //可燃气体传感器1读数  H2

            pIMEU->fCombGasPotency[1] = pIMEU3_Bat->fGasConcent2C0;         //可燃气体传感器2读数   CO 

            pIMEU->emGasSensor_CommFail[0] = pIMEU3_Bat->bySingleGasAlarm;        //可燃气体传感器通信中断

            pIMEU->emTempHumi_CommFail[0] = pIMEU3_Bat->byTemperatureHumiAlarm;    //温湿度传感器通信中断

            pIMEU->nHumiTempSensorNumber = pIMEU3_Bat->byConfig485Sen; // 温湿度传感器个数 //edit by Kuangyue 20250110

            pIMEU->nGasSensorNum = pIMEU3_Bat->byConfigGasSen; //可燃气体传感器个数 //edit by Kuangyue 20250110

        }
        else if(i == 1) //电源仓EU3
        {
            pIMEU->emIMEU2_CommFail = (PowerEU3_OtherEU3_Data[0].bIsCommFail == TRUE) ? 1 : 0;
            if(LIQUID_COOLING_BACKUP == emEssType)
            {
                for(n = 0; n < 4; ++n)
                {
                    pIMEU->fPT1000Temperature[n] =  pIMEU3_Power->fPT1000[n + 1];
                }
                pIMEU->emDI3_DoorOpen = GET_BIT(pIMEU3_Power->uDIStatus, 0);  //门禁
            }
        }
        else if(i == 2) //STS EU3
        {
            pIMEU->emIMEU2_CommFail = (PowerEU3_OtherEU3_Data[1].bIsCommFail == TRUE) ? 1 : 0;
            for(n = 0; n < 3; ++n)
            {
                pIMEU->fPT1000Temperature[n] =  pIMEU3_STS->fNTC[n];
            }
            pIMEU->emDI3_DoorOpen = GET_BIT(pIMEU3_STS->uDIStatus, 0);  //门禁
               
        }
        Push_BlobValue(SETTYPE_RAW_SP, pSampler, 601 + i, 450, \
		(void*)pIMEU, sizeof(A_BLOB_BATTCAB_IMEU2_V20), BLOB_TYPE_BATTCAB_IMEU2_V20);
    }
    return;
}