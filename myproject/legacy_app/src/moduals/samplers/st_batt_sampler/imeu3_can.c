//#define _TRACE_PRINT_
#include "imeu3_can.h"


#define GET_BIT(num, n) (((num) >> (n)) & 1)
#define SET_BIT(num, n) ((num) | (1 << (n)))

static TAG_IMEU3_PRO    sIMEU3ProData[_MAX_IMEU3_CAN_CMD];  

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
        varV.emValue = (IMEU3CANRoughData[n].bIsCommFail == TRUE) ? 1 : 0;  //电源柜IMEU3   设备地址0x1E  通信状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 611 + n, 1668, varV);
    }

}

//上传电池仓数据
void pushBattCabinetIMEU3CANData(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum)  
{
    int i = 0;
    VAR_VALUE varV;
    SAMPLER* pSampler = (SAMPLER*)IMEU3CANRoughData[i].hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    for(i = 0; i < iNum; i++)
    {
        IMEU3_CAN_DataInfo* pIMEU3 = &IMEU3CANRoughData[i].stIMEU3DataInfo;

        char buffer[64];
        
        int n;
        for(n = 0; n < MAX_IMEU3NTC_NUM; n++)  //7路NTC温度传感器值
        {
             varV.fValue = pIMEU3->fNTC[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1603 + n, varV);
        }
/* 
        for(n = 0; n < MAX_IMEU3PT1000_NUM; n++)  //5路PT1000或NTC温度传感器值
        {
            varV.fValue = pIMEU3->fPT1000[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1610 + n, varV);
        } */

        varV.fValue = pIMEU3->fTemperatureSen;  //1#温度传感器温度值
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1615, varV);

        varV.fValue = pIMEU3->fHumiditySen;     //1#温度传感器湿度值
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1616, varV);

        varV.ulValue = pIMEU3->uFloodFreq;      //水浸传感器信号频率值
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1617, varV);

    /*       varV.ulValue = pIMEU3->uDIStatus;     //22路DI状态
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i  + i, 1618, varV); */
            // 解析22路DI状态
        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 0);    //门禁
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1661, varV);

        VAR_VALUE varVSett;
        A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
        ZERO_POBJS(&blobCCUSett, 1);

        varVSett.blobValue = (BYTE*)&blobCCUSett;
        pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 200, &varVSett, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
        BOOL bEnable = blobCCUSett.ccuSettings.sysAtributesMap1.Set.DoorOpenEnable;
		if(!bEnable)
		{
            varV.emValue = 0;
		}
		Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i, 454, varV);

/*         varV.emValue = GET_BIT(pIMEU3->uDIStatus, 12);    //可燃气体检测
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1662, varV); */

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 16);    //泄爆阀2状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1664, varV);

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 17);    //泄爆阀1状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1665, varV);

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 18);    //排气扇状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1666, varV);

/*         varV.ulValue = pIMEU3->uDOStatus;         //24路DO状态
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1619, varV);

        varV.ulValue = pIMEU3->uX1Connect;       //4路X1板连接情况
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1620, varV); */

        for(n = 0; n < MAX_PWMChannel_NUM; n++)   //6个PWM频率值
        {
            varV.ulValue = pIMEU3->uPWMFreq[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1621 + n, varV);
        }

        for(n = 0; n < MAX_PressureSen_NUM; n++)    //6个压力传感器电压值
        {
            varV.ulValue = pIMEU3->uPressureSen[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1627 + n, varV);
        }

            //varV.fValue = pIMEU3->fSoftwareVer;            //软件版本
        // snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer)>>6, ((pIMEU3->uSoftwareVer)>>4)&0x03);//edit by kuangyue 20250313
        snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer) >> 8, (pIMEU3->uSoftwareVer) & 0xFF );//edit by kuangyue 20250326
            //Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1633, varV);
        Push_TextValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1633, buffer, 64);

        varV.ulValue = pIMEU3->uSystemRunTime;          //系统运行时间
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1634, varV);

        for(n = 0; n < MAX_TemperatureHumiditySen_NUM; n++)   //4路温湿度传感器温度值
        {
            varV.fValue = pIMEU3->fTemperatureValue[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1635 + n, varV);  
        }

        for(n = 0; n < MAX_TemperatureHumiditySen_NUM; n++)   //4路温湿度传感器湿度值
        {
            varV.fValue = pIMEU3->fhumidityValue[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1639 + n, varV);
        }

        varV.fValue = pIMEU3->fGasConcent1H2;         //可燃气体传感器1读数  H2
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1643, varV);
            
        varV.fValue = pIMEU3->fGasConcent2C0;         //可燃气体传感器2读数   CO 
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1644, varV);

        varV.fValue = pIMEU3->fGasConcent3Other;       //可燃气体传感器3读数   预留  
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1645, varV);
    
        varV.fValue = pIMEU3->fGasConcent4Other;       //可燃气体传感器4读数   预留  
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1646, varV);

        varV.emValue = pIMEU3->bySingleGasAlarm;        //可燃气体传感器通信中断
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1647, varV);

        varV.emValue = pIMEU3->byTemperatureHumiAlarm;    //温湿度传感器通信中断
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1648, varV);
    
        varV.emValue = pIMEU3->byMultiGasAlarm;           //多合一气体传感器通信中断
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1649, varV);

        varV.emValue = pIMEU3->byDehumidifierAlarm;       //除湿器通信中断
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1650, varV);

/*         varV.ulValue = pIMEU3->usSenShortCircuitAlarm;    //按BIT表示1-12路温度传感器、1路温度传感器、1路湿度传感器短路故障
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1651, varV);

        varV.ulValue = pIMEU3->usSenOpenCircuitAlarm;     //按BIT表示1-12路温度传感器、1路温度传感器、1路湿度传感器开路故障
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1652, varV); */
        
            //温度参数配置
        varV.emValue = pIMEU3->bySystemRunTime;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1653, varV);

            //DI通道参数配置
        varV.emValue = pIMEU3->byConfigDI;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1654, varV);

            //PWM参数配置
        varV.emValue = pIMEU3->byConfigPWM;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1655, varV);

            //压力传感器参数配置
        varV.emValue = pIMEU3->byConfigPressure;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1656, varV);

            //可燃气体传感器参数配置
        varV.emValue = pIMEU3->byConfigGasSen;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1657, varV);

            //485传感器参数配置
        varV.ulValue = pIMEU3->byConfig485Sen;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1658, varV);

            //除湿器参数配置
        varV.emValue = pIMEU3->byConfigDehumidifier;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1659, varV);

            // 除湿机湿度值
        varV.fValue = pIMEU3->fHumidity;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2900, varV);

            // 除湿机启动湿度
        varV.fValue = pIMEU3->fDehumidifierStart;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2901, varV);

            // 除湿机停止湿度
        varV.fValue = pIMEU3->fDehumidifierStop;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2902, varV);

            // 除湿机温度负载
        varV.emValue = pIMEU3->byTemperatureLoad;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2903, varV);

            // 除湿机湿度负载
        varV.emValue = pIMEU3->byHumidityLoad;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2904, varV);

            // 除湿机运行模式
        varV.emValue = pIMEU3->byDehumidifierRunmode;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2905, varV);

            // 除湿机故障状态
        varV.emValue = pIMEU3->byDehumidifierFaultState;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2906, varV);

            // 除湿机通信地址
        varV.ulValue = pIMEU3->byDehumidifierAddr;
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 2907, varV);
    }

    return;
}

void pushBackupBattIMEU3Data(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum)
{
    int i = 0;
    VAR_VALUE varV;
    SAMPLER* pSampler = (SAMPLER*)IMEU3CANRoughData[i].hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    for(i = 0; i < iNum; i++)
    {
        IMEU3_CAN_DataInfo* pIMEU3 = &IMEU3CANRoughData[i].stIMEU3DataInfo;
        
        int n = 0;
        for(n = 0; n < 4; n++)  //4路电池柜NTC温度值
        {
            varV.fValue = pIMEU3->fNTC[n];
            Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1603 + n, varV);
        }

        VAR_VALUE varVSett;
        A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
        ZERO_POBJS(&blobCCUSett, 1);

        varVSett.blobValue = (BYTE*)&blobCCUSett;
        pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 200, &varVSett, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
        BOOL bEnable = blobCCUSett.ccuSettings.sysAtributesMap1.Set.DoorOpenEnable;

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 0);    //电池舱前门
        if(!bEnable)
		{
            varV.emValue = 0;
		}
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1618, varV);

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 1);    //电池舱后门
        if(!bEnable)
		{
            varV.emValue = 0;
		}
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1619, varV);

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 2);    //国外才有可燃气体检测
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1662, varV);
        
    }

    return;
}

void pushLiquidCoolBackupPowerCabinetIMEU3(IMEU3_CAN_ROUGH_DATA* pPowerCabinetIMEU3)
{
	SAMPLER* pSampler = (SAMPLER*)pPowerCabinetIMEU3->hSampler;
	
	IMEU3_CAN_DataInfo* pIMEU3 = &(pPowerCabinetIMEU3->stIMEU3DataInfo);
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
    VAR_VALUE varV;

    char buffer[64];

    // snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer)>>6, ((pIMEU3->uSoftwareVer)>>4)&0x03);//edit by kuangyue 20250313
    snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer) >> 8, (pIMEU3->uSoftwareVer) & 0xFF );//edit by kuangyue 20250326
    Push_TextValue(SETTYPE_RAW_SP, pSampler, 611, 1669, buffer, 64);

    varV.ulValue = pIMEU3->uSystemRunTime;        //系统运行时间
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1670, varV);

    varV.ulValue = pIMEU3->uFloodFreq;          //水浸传感器信号频率值
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1692, varV);

    varV.emValue = (pIMEU3->uFloodFreq < 300) ? 1 : 0;    //DO1 水泵运行状态反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 200, 1693, varV);

	varV.fValue = pIMEU3->fNTC[0];  //配电室环境温度检测
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611 , 1986, varV);
	varV.fValue = pIMEU3->fNTC[1];  //配电室线缆温度检测
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1987, varV);
	varV.fValue = pIMEU3->fNTC[2];   //开关电源PDU内温度检测
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1988, varV);
	varV.fValue = pIMEU3->fNTC[3];   //PCS室环温
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1989, varV);
	
	//解析22路DI口状态
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 0);    //DI1 交流输入断路器QF1状态反馈（断路器闭合，常闭，断路器断开，常开）
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1990, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 3);    //DI4 N线接地接触器KM4反馈
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1993, varV);

	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 4);    //DI5 直流防雷器SPD反馈（正常，常闭，故障，常开）(光伏用）
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1994, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 5);    //DI6 交流防雷器SPD反馈（正常，常闭，故障，常开）
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1995, varV);

    
	VAR_VALUE varVSett;
    A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
    ZERO_POBJS(&blobCCUSett, 1);
    varVSett.blobValue = (BYTE*)&blobCCUSett;
    pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 200, &varVSett, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
    BOOL bEnable = blobCCUSett.ccuSettings.sysAtributesMap1.Set.DoorOpenEnable;

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 6);    //DI7 配电室门禁Door反馈（前后门常闭串联）
	if(!bEnable)
	{
        varV.emValue = 0;
	} 
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 2003, varV);
    
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 7);    //DI8 烟感SS反馈 0是表示异常，
    varV.emValue = !varV.emValue;
    BOOL bSmoke = blobCCUSett.ccuSettings.sysAtributesMap1.Set.Smoke_Sensor;
    if(!bSmoke)
    {
        varV.emValue = 0;
    }
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1996, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 8);    //DI9 倾倒传感器反馈(常开信号，故障时闭合)
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1997, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 10);    //DI11 储能主交流接触器KM1反馈（接触器闭合，常闭，接触器断开，常开）
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1998, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 12);    //DI13 负载断路器QF2状态（断路器闭合，常闭，断路器断开，常开）负载多个时串接
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1999, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 13);    //DI14 维修开关/旁路断路器QF3开关状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 2000, varV);
	
    pDPR->Get(pSampler, GETTYPE_ST_VAR, 611, 452, &varV, 4);
    if(varV.emValue == 1)
    {//使能开关
        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 17);    //DI18 维修开关/旁路断路器QF3面板接近开关反馈（接无源点）
       
    }
    else
    {
        varV.emValue = 0;
    }
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 2001, varV); 
    return;
}

void pushLiquidCoolPowerCabinetIMEU3(IMEU3_CAN_ROUGH_DATA* pPowerCabinetIMEU3)
{
    SAMPLER* pSampler = (SAMPLER*)pPowerCabinetIMEU3->hSampler;
	
	IMEU3_CAN_DataInfo* pIMEU3 = &(pPowerCabinetIMEU3->stIMEU3DataInfo);
	
	VAR_VALUE varV;
	
    char buffer[64];
    // snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer)>>6, ((pIMEU3->uSoftwareVer)>>4)&0x03);//edit by kuangyue 20250313
    snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer) >> 8, (pIMEU3->uSoftwareVer) & 0xFF );//edit by kuangyue 20250326
    Push_TextValue(SETTYPE_RAW_SP, pSampler, 611, 1669, buffer, 64);

    varV.ulValue = pIMEU3->uSystemRunTime;          //系统运行时间
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1670, varV);

	varV.fValue = pIMEU3->fNTC[0];  //模块出风口温度检测
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1671, varV);
	varV.fValue = pIMEU3->fNTC[1];  //PDU内部温度
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1672, varV);
	varV.fValue = pIMEU3->fNTC[2];   //PDU内模块进线线缆温度
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1673, varV);
	
	//解析22路DI口状态
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 0);    //DI1 QF1开关状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1674, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 1);    //DI2 交流接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1675, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 2);    //DI3 N线接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1676, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 3);    //DI4 N线接地接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1677, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 4);    //DI5 MPFT正负极接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1678, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 5);    //DI6 倾倒
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1679, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 6);    //DI7 光伏防雷状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1680, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 7);    //DI8 正极接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1681, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 8);    //DI9 负极接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1682, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 9);    //DI10 预充接触器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1683, varV);

/*	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 10);    //DI11 交流辅源状态反馈 （预留）
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1684, varV); */
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 15);    //DI16 交流防雷器开关及防雷器状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1685, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 16);    //DI17散热风机RDB状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1686, varV);
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 17);    //DI18 PDU内扰流风机状态
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1687, varV);

/*	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 19);    //   DI预留1
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1688, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 20);    //   DI预留2
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1689, varV);
	
	varV.emValue = GET_BIT(pIMEU3->uDIStatus, 21);    //   DI预留3
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 611, 1690, varV);    */
	return;
}

// 2524 EU3
void push2524IMEU3CanData(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum)
{
    int i = 0;
    VAR_VALUE varV;
    SAMPLER* pSampler = (SAMPLER*)IMEU3CANRoughData[i].hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

    VAR_VALUE varVSett;
    A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
    ZERO_POBJS(&blobCCUSett, 1);
    varVSett.blobValue = (BYTE*)&blobCCUSett;
    pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 200, &varVSett, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
    BOOL bEnable = blobCCUSett.ccuSettings.sysAtributesMap1.Set.DoorOpenEnable;

    char strSWer[64];
	memset(strSWer, 0, 64);

    for(i = 0; i < iNum; i++)
    {
        IMEU3_CAN_DataInfo* pIMEU3 = &IMEU3CANRoughData[i].stIMEU3DataInfo;

        varV.fValue = pIMEU3->fNTC[0];  // 电池仓右侧温度
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1915, varV);
    
        varV.fValue = pIMEU3->fNTC[1];  // 电池仓左侧温度
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1916, varV);
    
        varV.fValue = pIMEU3->fNTC[2];  // 功率模块出风口温度
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1917, varV);
    
        varV.fValue = pIMEU3->fNTC[3];  // PDU内模块出线线缆温度
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1918, varV);
    

        varV.fValue = pIMEU3->fNTC[4];   // PDU内模块进线线缆温度
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1919, varV);
    
        varV.fValue = pIMEU3->fNTC[5];   // PDU出风口温度
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1920, varV);
    
    
        //解析DI口状态 0:正常 1:异常
        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 0);    //DI1 门禁
    	if(!bEnable)
	    {
            varV.emValue = 0;
	    }
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 454, varV); // 不显示
    
        //DI2-烟感 bmu上传 DI3-交流防雷器状态 mcu上传 DI4-空调预留

        varV.emValue = GET_BIT(pIMEU3->uDIStatus, 4);    //DI5 消防报警信号输入-灭火装置
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601+i, 1922, varV);
    
        //DI17-泄爆阀1状态 DI18-排气扇状态 DI19-散热风机状态反馈 均由bmu上传
        
        varV.ulValue = pIMEU3->usSenShortCircuitAlarm & 0x3F;    // 只需要bit0~bit5
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1651, varV);

        varV.ulValue = pIMEU3->usSenOpenCircuitAlarm & 0x3F;     // 只需要bit0~bit5
        Push_VarValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1652, varV);        

        snprintf(strSWer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer) >> 8, (pIMEU3->uSoftwareVer) & 0xFF );
        Push_TextValue(SETTYPE_RAW_SP, pSampler, 601 + i , 1633, strSWer, 64);

    }

    return;
}

void pushSTSCabinetIMEU3CanData(IMEU3_CAN_ROUGH_DATA* pSTSCabinetIMEU3)
{
    VAR_VALUE varV;

    SAMPLER* pSampler = (SAMPLER*)pSTSCabinetIMEU3->hSampler;

    IMEU3_CAN_DataInfo* pIMEU3 = &(pSTSCabinetIMEU3->stIMEU3DataInfo);

    char buffer[64] = {0};
    // snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer)>>6, ((pIMEU3->uSoftwareVer) >> 4) & 0x03);//edit by kuangyue 20250313
    snprintf(buffer, 64, "V%d.%02d", (pIMEU3->uSoftwareVer) >> 8, (pIMEU3->uSoftwareVer) & 0xFF );//edit by kuangyue 20250326
    Push_TextValue(SETTYPE_RAW_SP, pSampler, 612, 1982, buffer, 64);

    varV.fValue = pIMEU3->fNTC[0];  //柜内温度
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1983, varV);

    varV.fValue = pIMEU3->fNTC[1];  //母线铜排温度
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1984, varV);

    varV.fValue = pIMEU3->fNTC[2];   //STS表面温度
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1985, varV);

    //解析DI口状态
    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 0);    //DI1 总进线开关QF反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1966, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 1);    //DI2 负载开关QF反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1967, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 2);    //DI3 维修开关QF反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1968, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 3);    //DI4 防雷器反馈（含防雷器开关）
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1969, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 4);    //DI5 N线接触器控KM反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1970, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 5);    //DI6 N接地接触器KM反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1971, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 6);    //DI7 逆变侧主接触器2#KM反馈
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1972, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 7);    //DI8  门禁
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1973, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 8);    //DI9 维修开关机械锁
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1974, varV);

    varV.emValue = GET_BIT(pIMEU3->uDIStatus, 9);    //DI10 急停
    Push_VarValue(SETTYPE_RAW_SP, pSampler, 612, 1975, varV);

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