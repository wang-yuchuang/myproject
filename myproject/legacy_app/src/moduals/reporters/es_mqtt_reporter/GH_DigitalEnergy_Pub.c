#include "../reporters_base.h"
#include "es_mqtt.h"
#include <signal.h>
#include "3rd_party/libpahomqtt/include/MQTTClient.h"


char *Pack_GH_Mqtt_CallFrame(MQTT_DATA_ROUGH* pMqtt, MQTT_CALL_MSG *pMsg)
{
    // TRACE("--- Start PackMqtt_CallFrame ---\n");
     //char szTime[32];
    memset(pMqtt->szSendBuffer, 0 , MAX_MQTT_BUFFER_LEN);
     
    snprintf(pMqtt->szSendBuffer, MAX_MQTT_BUFFER_LEN, pMsg->szJsonData);

	pMqtt->nSendBuffLen = (int)strlen(pMqtt->szSendBuffer);
/* 
	if(pMqtt->bZlibCompress == TRUE)
	{
		Zlib_CompressData(pMqtt);
	} */
 	return pMqtt->szSendBuffer;

}

void Send_GH_Call(MQTT_DATA_ROUGH* pMqtt, cJSON* pJson,
			MSG_CALL_SRC emMsgSrc, int nConnNum, BOOL bUrgent)
{
	MQTT_CALL_MSG   msg;
	ZERO_POBJS(&msg, 1);
	char *szData = 0;
	szData = cJSON_Print(pJson);
	//TRACEX("MsgCall[%d], msgSrc = %d, Json_Print = %s\n",pMqtt->stMsgInfoList[msgType].nMsgId, emMsgSrc,
	//	szData);

	cJSON_Delete(pJson);

	//make the msg
	//msg.emMsgType = msgType;
	msg.emSrc = emMsgSrc;
	msg.nBelongedConn = nConnNum;
	//memcpy(msg.szUniqMsgID, pMqtt->stSuspendedMsgList[connectorNo].nUniqMsgId, sizeof(pMqtt->stSuspendedMsgList[connectorNo].nUniqMsgId));
	//sprintf(msg.nUniqMsgId, "%lu", (unsigned int)(GetCurrentTime() * 1000));
	memset(msg.szJsonData, 0, MAX_MQTT_BUFFER_LEN);
	//memcpy(msg.szJsonData, szData, strlen(szData));
	snprintf(msg.szJsonData, MAX_MQTT_BUFFER_LEN, "%s", szData);
	//msg.nSentLength = strlen(szData);
	free(szData);
    //printf("Send_GH_Call: msg.szJsonData = %s\n", msg.szJsonData);
	//HANDLE hq = pMqtt->hqMsg2Sent;
	//Queue_Put(hq, &msg, bUrgent);

    Pack_GH_Mqtt_CallFrame(pMqtt, &msg);

	//TRACEX("Queue_Put NORMAL Call Successfully into Socket: %s \n", msg.szJsonData);
        
    return;
}

int prepare_ArrData(void* pArg, int index)
{
    //printf("prepare_ArrData\n");
   	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet, VarV, valBat, valGet2;
    //ERRCODE_DAI 		err;
    ARR_UNION Arr_union;
    ZERO_POBJS(&Arr_union, 1);

    A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
    ZERO_POBJS(&blobSiglBatt, 1);
    blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
    valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
    valGet.val.blobValue = (BYTE*)&blobSiglBatt;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 551, &valGet, sizeof(PARAM_VAL_GET));   

    A_BLOB_IFY_BMS_DATAINFO blobIfySiglBat;
	ZERO_POBJS(&blobIfySiglBat, 1);
	blobIfySiglBat.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
	valBat.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
	valBat.val.blobValue = (BYTE*)&blobIfySiglBat;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501 + index, 557, &valBat, sizeof(PARAM_VAL_GET)); 

    A_BLOB_BRANCH_GRP_RECTDATA blobRectGrp_1;
    ZERO_POBJS(&blobRectGrp_1, 1);
    blobRectGrp_1.emBlobType = BLOB_TYPE_BRANCH_GRP_RECT_DATA;
    valGet2.nGetSize = sizeof(A_BLOB_BRANCH_GRP_RECTDATA);
    valGet2.val.blobValue = (BYTE*)&blobRectGrp_1;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                211, 201, &valGet2, sizeof(PARAM_VAL_GET));

    VarV.nGetSize = 4;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 554, &VarV, sizeof(PARAM_VAL_GET)); 
    float SunChag = VarV.val.fValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 555, &VarV, sizeof(PARAM_VAL_GET));
    float SumDischg = VarV.val.fValue;

    Arr_union.arrData.arrno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", Arr_union.arrData.arrno);    
    //充放电运行状态
    switch (blobSiglBatt.emBattStatus_Run)
    {
        case STBATT_RUNSTAT_CHARGE:
            Arr_union.arrData.arrStatus = 1;
            break;
        case STBATT_RUNSTAT_DISCHARGE:
            Arr_union.arrData.arrStatus = 2;
            break;
        case STBATT_RUNSTAT_IDLE:
            Arr_union.arrData.arrStatus = 3;
            break;
        case STBATT_RUNSTAT_COMM_FAIL:
            Arr_union.arrData.arrStatus = 4;
            break;
        default:
            Arr_union.arrData.arrStatus = 3;
            break;
    }
    cJSON_AddNumberToObject(pRoot, "1", Arr_union.arrData.arrStatus);
    //BMS状态
    switch (blobSiglBatt.emBattStatus_Alm)
    {
        case STBATT_ALMSTAT_NOMAL:
            Arr_union.arrData.bmsStatus = 0;
            break;
        case STBATT_ALMSTAT_ALARMING:
            Arr_union.arrData.bmsStatus = 1;
            break;
        case STBATT_ALMSTAT_FAULT:
            Arr_union.arrData.bmsStatus = 2;
            break;
        case STBATT_ALMSTAT_PROTECT:
            Arr_union.arrData.bmsStatus = 3;
            break;
        default:
            Arr_union.arrData.bmsStatus = 0;
            break;
    }
    cJSON_AddNumberToObject(pRoot, "2", Arr_union.arrData.bmsStatus);

    Arr_union.arrData.cluSums = pMqtt->nCfgBMSNum;  //电池组数
    cJSON_AddNumberToObject(pRoot, "3", Arr_union.arrData.cluSums);

    Arr_union.arrData.cluMaxSocNum = 1; //簇SOC最大簇号
    cJSON_AddNumberToObject(pRoot, "5", Arr_union.arrData.cluMaxSocNum);

    Arr_union.arrData.cluMinSocNum = 1;//簇SOC最小簇号
    cJSON_AddNumberToObject(pRoot, "6", Arr_union.arrData.cluMinSocNum);

    Arr_union.arrData.cluMaxVolNum = 1;//簇电压最大簇号
    cJSON_AddNumberToObject(pRoot, "7", Arr_union.arrData.cluMaxVolNum);

    Arr_union.arrData.cluMinVolNum = 1;//簇电压最小簇号
    cJSON_AddNumberToObject(pRoot, "8", Arr_union.arrData.cluMinVolNum);

    Arr_union.arrData.maxCellVolCluNum = 1;  //最高单体电压电池簇号
    cJSON_AddNumberToObject(pRoot, "9", Arr_union.arrData.maxCellVolCluNum);

    Arr_union.arrData.maxCellVolCellNum = blobSiglBatt.uiBAT_Ucell_Max_CELL; //最高单体电压所在簇的簇内编号
    cJSON_AddNumberToObject(pRoot, "10", Arr_union.arrData.maxCellVolCellNum);

    Arr_union.arrData.minCellVolCluNum = 1; //最低单体电压电池簇号
    cJSON_AddNumberToObject(pRoot, "11", Arr_union.arrData.minCellVolCluNum);

    Arr_union.arrData.minCellVolCellNum = blobSiglBatt.uiBAT_Ucell_Min_CELL;  //最低单体电压所在簇的簇内编号
    cJSON_AddNumberToObject(pRoot, "12", Arr_union.arrData.minCellVolCellNum);

    Arr_union.arrData.maxCellTemCluNum = 1; //最高电池温度所在簇号
    cJSON_AddNumberToObject(pRoot, "13", Arr_union.arrData.maxCellTemCluNum);

    Arr_union.arrData.maxCellTemCellNum = blobSiglBatt.uiBAT_T_Max_CELL; //最高电池温度所在簇的簇内编号
    cJSON_AddNumberToObject(pRoot, "14", Arr_union.arrData.maxCellTemCellNum);

    Arr_union.arrData.minCellTemCluNum = 1; //最低电池温度所在簇号
    cJSON_AddNumberToObject(pRoot, "15", Arr_union.arrData.minCellTemCluNum);

    Arr_union.arrData.minCellTemCellNum = blobSiglBatt.uiBAT_T_Min_CELL; //最低电池温度所在簇的簇内编号
    cJSON_AddNumberToObject(pRoot, "16", Arr_union.arrData.minCellTemCellNum);

    Arr_union.arrData.arrVol = blobSiglBatt.fPackTotalVolt; //电池堆电压
    cJSON_AddFloatToObject(pRoot, "17", Arr_union.arrData.arrVol, 1);

    Arr_union.arrData.arrCur = blobSiglBatt.fPackTotalCurr; //电池堆电流
    cJSON_AddFloatToObject(pRoot, "18", Arr_union.arrData.arrCur, 1);

    Arr_union.arrData.arrSOC = (float)blobSiglBatt.nSOC; //电池堆SOC
    cJSON_AddFloatToObject(pRoot, "19", Arr_union.arrData.arrSOC, 1);

    Arr_union.arrData.arrSOH = (float)blobSiglBatt.nSOH; //电池堆SOH
    cJSON_AddFloatToObject(pRoot, "20", Arr_union.arrData.arrSOH, 1);

    Arr_union.arrData.arrRechaElec = blobIfySiglBat.fTotalEnergy - blobIfySiglBat.fRemainEnergy; //电池堆可充电量
    cJSON_AddFloatToObject(pRoot, "21", Arr_union.arrData.arrRechaElec, 1);


    Arr_union.arrData.arrRedischgElec = blobIfySiglBat.fRemainEnergy; //电池堆可放电量
    cJSON_AddFloatToObject(pRoot, "22", Arr_union.arrData.arrRedischgElec, 1);

    Arr_union.arrData.arrLastRechaElec = blobIfySiglBat.fSingleChgCapacity; //电池堆单次充电电量
    cJSON_AddFloatToObject(pRoot, "23", Arr_union.arrData.arrLastRechaElec, 1);

    Arr_union.arrData.arrLastRedischgElec = blobIfySiglBat.fSingleDischgCapacity; //电池堆单次放电电量
    cJSON_AddFloatToObject(pRoot, "24", Arr_union.arrData.arrLastRedischgElec, 1);

    Arr_union.arrData.arrSumsChaElec = SunChag; //电池堆充电量累加值
    cJSON_AddFloatToObject(pRoot, "25", Arr_union.arrData.arrSumsChaElec, 1);

    Arr_union.arrData.arrSumsDischgElec = SumDischg;  //电池堆放电量累加值
    cJSON_AddFloatToObject(pRoot, "26", Arr_union.arrData.arrSumsDischgElec, 1);

    Arr_union.arrData.maxCellVol = blobSiglBatt.fMax_CellVolt; //最高单体电压
    cJSON_AddFloatToObject(pRoot, "28", Arr_union.arrData.maxCellVol, 3);

    Arr_union.arrData.minCellVol = blobSiglBatt.fMin_CellVolt; //最低单体电压
    cJSON_AddFloatToObject(pRoot, "29", Arr_union.arrData.minCellVol, 3);

    Arr_union.arrData.maxCellTem = blobSiglBatt.fMax_CellTemp; //最高电池温度
    cJSON_AddFloatToObject(pRoot, "31", Arr_union.arrData.maxCellTem, 1);

    Arr_union.arrData.minCellTem = blobSiglBatt.fMin_CellTemp; //最低电池温度
    cJSON_AddFloatToObject(pRoot, "32", Arr_union.arrData.minCellTem, 1);

    Arr_union.arrData.arrMaxReChaPower = blobRectGrp_1.fACDC_Max_Output_Pwr_Ability;  //电池堆当前允许最大充电功率
    cJSON_AddFloatToObject(pRoot, "39", Arr_union.arrData.arrMaxReChaPower, 1);

    Arr_union.arrData.arrMaxReDischgPower = blobRectGrp_1.fACDC_Max_Output_Pwr_Ability; //电池堆当前允许最大放电功率
    cJSON_AddFloatToObject(pRoot, "40", Arr_union.arrData.arrMaxReDischgPower, 1);

    Arr_union.arrData.arrMaxReChaCur = blobSiglBatt.fAllow_MaxChargeCurr; //电池堆当前允许最大充电电流
    cJSON_AddFloatToObject(pRoot, "41", Arr_union.arrData.arrMaxReChaCur, 1);

    Arr_union.arrData.arrMaxRedischgCur = blobSiglBatt.fAllow_MaxDisChargeCurr; //电池堆当前允许最大放电电流
    cJSON_AddFloatToObject(pRoot, "42", Arr_union.arrData.arrMaxRedischgCur, 1);
    
    Arr_union.arrData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "47", Arr_union.arrData.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 

    return 0;
}

int prepare_CluData(void* pArg, int index)
{
    //printf("prepare_CluData\n");
   	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet, VarV, valBat;
    //ERRCODE_DAI 		err;
    CLU_UNION            clu_uion;
    ZERO_POBJS(&clu_uion, 1);
 
    A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
    ZERO_POBJS(&blobSiglBatt, 1);
    blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
    valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
    valGet.val.blobValue = (BYTE*)&blobSiglBatt;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 551, &valGet, sizeof(PARAM_VAL_GET));

    VarV.nGetSize = 4;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 554, &VarV, sizeof(PARAM_VAL_GET)); 
    float SunChag = VarV.val.fValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 555, &VarV, sizeof(PARAM_VAL_GET));
    float SumDischg = VarV.val.fValue;    

    A_BLOB_IFY_BMS_DATAINFO blobIfySiglBat;
	ZERO_POBJS(&blobIfySiglBat, 1);
	blobIfySiglBat.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
	valBat.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
	valBat.val.blobValue = (BYTE*)&blobIfySiglBat;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501 + index, 557, &valBat, sizeof(PARAM_VAL_GET));  

    clu_uion.cluData.arrno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", clu_uion.cluData.arrno);

    clu_uion.cluData.cluno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", clu_uion.cluData.cluno);

    clu_uion.cluData.cluSoc = (float)blobSiglBatt.nSOC; //SOC
    cJSON_AddFloatToObject(pRoot, "2", clu_uion.cluData.cluSoc, 1);

    clu_uion.cluData.cluSoh = (float)blobSiglBatt.nSOH; //SOH
    cJSON_AddFloatToObject(pRoot, "3", clu_uion.cluData.cluSoh, 1);

    //电池簇运行状态
    switch (blobSiglBatt.emBattStatus_Run)
    {
        case STBATT_RUNSTAT_CHARGE:
            clu_uion.cluData.cluStatus = 1;
            break;
        case STBATT_RUNSTAT_DISCHARGE:
            clu_uion.cluData.cluStatus = 2;
            break;
        case STBATT_RUNSTAT_IDLE:
            clu_uion.cluData.cluStatus = 3;
            break;
        case STBATT_RUNSTAT_COMM_FAIL:
            clu_uion.cluData.cluStatus = 4;
            break;
        default:
            clu_uion.cluData.cluStatus = 3;
            break;
    }
    cJSON_AddNumberToObject(pRoot, "4", clu_uion.cluData.cluStatus);
    //BMS状态
    switch (blobSiglBatt.emBattStatus_Alm)
    {
        case STBATT_ALMSTAT_NOMAL:
            clu_uion.cluData.bmsStatus = 1;
            break;
        case STBATT_ALMSTAT_ALARMING:
            clu_uion.cluData.bmsStatus = 2;
            break;
        case STBATT_ALMSTAT_FAULT:
            clu_uion.cluData.bmsStatus = 3;
            break;
        case STBATT_ALMSTAT_PROTECT:
            clu_uion.cluData.bmsStatus = 4;
            break;
        default:
            clu_uion.cluData.bmsStatus = 1;
            break;
    }
    cJSON_AddNumberToObject(pRoot, "5", clu_uion.cluData.bmsStatus);

    clu_uion.cluData.cluVol = blobSiglBatt.fPackTotalVolt; //总电压
    cJSON_AddFloatToObject(pRoot, "6", clu_uion.cluData.cluVol, 1);

    clu_uion.cluData.cluCur = blobSiglBatt.fPackTotalCurr; //电流
    cJSON_AddFloatToObject(pRoot, "7", clu_uion.cluData.cluCur, 1);

    clu_uion.cluData.maxCellVol = blobSiglBatt.fMax_CellVolt; //最高单体电压
    cJSON_AddFloatToObject(pRoot, "8", clu_uion.cluData.maxCellVol, 3);

    clu_uion.cluData.minCellVol = blobSiglBatt.fMin_CellVolt; //最低单体电压
    cJSON_AddFloatToObject(pRoot, "9", clu_uion.cluData.minCellVol, 3);

    clu_uion.cluData.maxCellTem = blobSiglBatt.fMax_CellTemp; //最高温度
    cJSON_AddFloatToObject(pRoot, "10", clu_uion.cluData.maxCellTem, 1);

    clu_uion.cluData.minCellTem = blobSiglBatt.fMin_CellTemp; //最低温度
    cJSON_AddFloatToObject(pRoot, "11", clu_uion.cluData.minCellTem, 1);

    clu_uion.cluData.cluPosres  = blobSiglBatt.fInsulation_P_R; //正对地绝缘电值
    cJSON_AddFloatToObject(pRoot, "16", clu_uion.cluData.cluPosres, 1);

    clu_uion.cluData.cluNegres  = blobSiglBatt.fInsulation_N_R; //负对地绝缘电阻
    cJSON_AddFloatToObject(pRoot, "17", clu_uion.cluData.cluNegres, 1);

    clu_uion.cluData.maxCellVolCellNum = blobSiglBatt.uiBAT_T_Max_CELL; //最高单体电压编号
    cJSON_AddNumberToObject(pRoot, "18", clu_uion.cluData.maxCellVolCellNum);

    clu_uion.cluData.minCellVolCellNum = blobSiglBatt.uiBAT_T_Min_CELL; //最低单体电压编号
    cJSON_AddNumberToObject(pRoot, "19", clu_uion.cluData.minCellVolCellNum);

    clu_uion.cluData.cellVolTotalSum =  blobSiglBatt.fPackTotalVolt; //单体电压总和
    cJSON_AddFloatToObject(pRoot, "22", clu_uion.cluData.cellVolTotalSum, 1);

    clu_uion.cluData.maxDischgArrCur = blobSiglBatt.fAllow_MaxDisChargeCurr; //最大放电电流
    cJSON_AddFloatToObject(pRoot, "23", clu_uion.cluData.maxDischgArrCur, 1);

    clu_uion.cluData.reqDischgArrCur = blobSiglBatt.fAllow_MaxChargeCurr; //请求充电电流
     cJSON_AddFloatToObject(pRoot, "25", clu_uion.cluData.reqDischgArrCur, 1);

    clu_uion.cluData.cluRtPower = (float)blobSiglBatt.fPackTotalVolt*blobSiglBatt.fPackTotalCurr*0.001; //实时功率
    cJSON_AddFloatToObject(pRoot, "31", clu_uion.cluData.cluRtPower, 1);

    clu_uion.cluData.cluSumsChaElec = SunChag; //累计充电电量
    cJSON_AddFloatToObject(pRoot, "32", clu_uion.cluData.cluSumsChaElec, 1);

    clu_uion.cluData.cluSumsDischgElec = SumDischg; //累计放电电量
    cJSON_AddFloatToObject(pRoot, "33", clu_uion.cluData.cluSumsDischgElec, 1);

    clu_uion.cluData.lastSumsChaElec = blobIfySiglBat.fSingleChgCapacity; //单次充电电量
    cJSON_AddFloatToObject(pRoot, "34", clu_uion.cluData.lastSumsChaElec, 1);

    clu_uion.cluData.lastSumsDischgElec = blobIfySiglBat.fSingleDischgCapacity; //单次放电电量
    cJSON_AddFloatToObject(pRoot, "35", clu_uion.cluData.lastSumsDischgElec, 1);
    
    clu_uion.cluData.cluRechaElec = blobIfySiglBat.fTotalEnergy - blobIfySiglBat.fRemainEnergy; //可充电量
    cJSON_AddFloatToObject(pRoot, "36", clu_uion.cluData.cluRechaElec, 1);

    clu_uion.cluData.cluRedischgElec = blobIfySiglBat.fRemainEnergy; //可放电量
    cJSON_AddFloatToObject(pRoot, "37", clu_uion.cluData.cluRedischgElec, 1);

    clu_uion.cluData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddFloatToObject(pRoot, "38", clu_uion.cluData.Ts, 1);
    
    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 

    return 0;          
}

int prepare_EmuData(void* pArg, int index)
{
    //printf("prepare_EmuData\n");
   	MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet, valGet1, valGet2, VarV;
    ERRCODE_DAI 		err;
    EMU_UNION           emu_union;
    ZERO_POBJS(&emu_union, 1);

    A_BLOB_ES_RUN_DATA	esRunData;
 	ZERO_POBJS(&esRunData, 1);
    esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet, sizeof(PARAM_VAL_GET));
    
    A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
    ZERO_POBJS(&blobSiglBatt, 1);
    blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
    valGet1.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
    valGet1.val.blobValue = (BYTE*)&blobSiglBatt;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 551, &valGet1, sizeof(PARAM_VAL_GET)); 

    A_BLOB_BRANCH_GRP_RECTDATA blobRectGrp_1;
    ZERO_POBJS(&blobRectGrp_1, 1);
    blobRectGrp_1.emBlobType = BLOB_TYPE_BRANCH_GRP_RECT_DATA;
    valGet2.nGetSize = sizeof(A_BLOB_BRANCH_GRP_RECTDATA);
    valGet2.val.blobValue = (BYTE*)&blobRectGrp_1;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                211, 201, &valGet2, sizeof(PARAM_VAL_GET));

    VarV.nGetSize = 4;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                611, 1673, &VarV, sizeof(PARAM_VAL_GET)); 
    float temp = VarV.val.fValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
              601, 2900, &VarV, sizeof(PARAM_VAL_GET));
    float Humidity = VarV.val.fValue;
    //充放电状态
    switch (blobSiglBatt.emBattStatus_Run)
    {
        case STBATT_RUNSTAT_CHARGE:
            emu_union.emuData.runStatus = 1;
            break;
        case STBATT_RUNSTAT_DISCHARGE:
            emu_union.emuData.runStatus = 2;
            break;
        default:
            emu_union.emuData.runStatus = 0;
            break;
    }
    cJSON_AddFloatToObject(pRoot, "0", emu_union.emuData.runStatus, 1);

    emu_union.emuData.pcsSumsActivePower = esRunData.fActvPwr_PCSGrp[index]; //PCS实际总有功功率
    cJSON_AddFloatToObject(pRoot, "1", emu_union.emuData.pcsSumsActivePower, 1);

    emu_union.emuData.pcsSumsReactivePower = esRunData.fQ_PCSGrp[index]; //PCS实际总无功功率
    cJSON_AddFloatToObject(pRoot, "2", emu_union.emuData.pcsSumsReactivePower, 1);

    emu_union.emuData.dcBusVol = blobSiglBatt.fPackTotalVolt; //直流母线总电压
    cJSON_AddFloatToObject(pRoot, "3", emu_union.emuData.dcBusVol, 1);

    emu_union.emuData.dcBusVur = blobSiglBatt.fPackTotalCurr; //直流母线总电流
    cJSON_AddFloatToObject(pRoot, "4", emu_union.emuData.dcBusVur, 1);

    emu_union.emuData.sumsSOC = (float)blobSiglBatt.nSOC; //总SOC
    cJSON_AddFloatToObject(pRoot, "5", emu_union.emuData.sumsSOC, 1);

    //电池状态
    switch (blobSiglBatt.emBattStatus_Run)
    {
        case STBATT_RUNSTAT_CHARGE:
            emu_union.emuData.batteryStatus = 3;
            break;
        case STBATT_RUNSTAT_DISCHARGE:
            emu_union.emuData.batteryStatus = 4;
            break;
        case STBATT_RUNSTAT_IDLE:
            emu_union.emuData.batteryStatus = 2;
            break;
        case STBATT_RUNSTAT_COMM_FAIL:
            emu_union.emuData.batteryStatus = 5;
            break;
        default:
            emu_union.emuData.batteryStatus = 2;
            break;
    }
    cJSON_AddNumberToObject(pRoot, "6", emu_union.emuData.batteryStatus);

    emu_union.emuData.cellSums = SYSTYPE_ESS_LIQUIDED_PACK_NUM*SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM; //电池总节数
    cJSON_AddFloatToObject(pRoot, "7", emu_union.emuData.cellSums, 1);

    emu_union.emuData.batteryAverageTem = blobSiglBatt.fAvg_CellTemp; //电池平均温度
    cJSON_AddFloatToObject(pRoot, "8", emu_union.emuData.batteryAverageTem, 1);

    emu_union.emuData.batteryMaxTemDif = blobSiglBatt.fMax_CellTemp - blobSiglBatt.fMin_CellTemp; //电池最大温差
    cJSON_AddFloatToObject(pRoot, "9", emu_union.emuData.batteryMaxTemDif, 1);

    emu_union.emuData.maxCellVol = blobSiglBatt.fMax_CellVolt; //最高单体电压值
    cJSON_AddFloatToObject(pRoot, "10", emu_union.emuData.maxCellVol, 3);

    emu_union.emuData.minCellVol = blobSiglBatt.fMin_CellVolt; //最低单体电压值
    cJSON_AddFloatToObject(pRoot, "11", emu_union.emuData.minCellVol, 3);

    emu_union.emuData.ambientTem = temp; //环境温度
    cJSON_AddFloatToObject(pRoot, "12", emu_union.emuData.ambientTem, 1);

    emu_union.emuData.ambientHumidity = Humidity; //环境湿度
    cJSON_AddFloatToObject(pRoot, "13", emu_union.emuData.ambientHumidity, 1);

    emu_union.emuData.maxAllowChargPower = blobRectGrp_1.fACDC_Max_Output_Pwr_Ability; //系统最大允许充电功率
    cJSON_AddFloatToObject(pRoot, "16", emu_union.emuData.maxAllowChargPower, 1);

    emu_union.emuData.minAllowChargPower = blobRectGrp_1.fACDC_Max_Output_Pwr_Ability; //系统最大允许放电功率
    cJSON_AddFloatToObject(pRoot, "17", emu_union.emuData.minAllowChargPower, 1);

    //控制源
    switch (pMqtt->emControlMode)
    {
        case LOCAL_CTRL_MODE:
            emu_union.emuData.ControlSrc = 0; 
            break;
        case REMOTE_CTRL_MODE:
            emu_union.emuData.ControlSrc = 1; 
            break;
        default:
            emu_union.emuData.ControlSrc = 2; 
            break;
    }
    cJSON_AddNumberToObject(pRoot, "19", emu_union.emuData.ControlSrc);

    emu_union.emuData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;  //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "21", emu_union.emuData.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 

    return 0;
}

int prepare_PCSData(void* pArg, int index)
{
    //printf("prepare_PCSData\n");
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet, valGet2;
    ERRCODE_DAI 		err;
    PCS_UNION            pcs_union;
    ZERO_POBJS(&pcs_union, 1);

	A_BLOB_ES_RUN_DATA	esRunData;
 	ZERO_POBJS(&esRunData, 1);
    esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet, sizeof(PARAM_VAL_GET));

    A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
    ZERO_POBJS(&blobSiglBatt, 1);
    blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
    valGet2.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
    valGet2.val.blobValue = (BYTE*)&blobSiglBatt;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 551, &valGet2, sizeof(PARAM_VAL_GET));

    pcs_union.pcsData.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", pcs_union.pcsData.ArrNum);

    pcs_union.pcsData.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", pcs_union.pcsData.CluNum);

    pcs_union.pcsData.Devno  = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "2", pcs_union.pcsData.Devno);

    //Run_Sta
    switch (esRunData.emACDC_WorkStatu[index])
    {
        case STATU_ACDC_RECTIFY:
            pcs_union.pcsData.Run_Sta = 3;
            break;
        case STATU_ACDC_INVERTER:
            pcs_union.pcsData.Run_Sta = 4;
            break;
        default:
            pcs_union.pcsData.Run_Sta = 0;
            break;
    }
    cJSON_AddNumberToObject(pRoot, "3", pcs_union.pcsData.Run_Sta);

    PARAM_VAL_GET   valGetSatus;
  	valGetSatus.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			200, 161, &valGetSatus, sizeof(PARAM_VAL_GET));

    pcs_union.pcsData.Offgrd_Sta = valGetSatus.val.emValue; //并离网状态0并网1离网
    cJSON_AddNumberToObject(pRoot, "4", pcs_union.pcsData.Offgrd_Sta);

    pcs_union.pcsData.Out_Uw_Vol = esRunData.fV_PCS3Phase[index][0]; //交流A相电压
    cJSON_AddFloatToObject(pRoot, "5",pcs_union.pcsData.Out_Uw_Vol, 1);

    pcs_union.pcsData.Out_Vw_Vol = esRunData.fV_PCS3Phase[index][1]; //交流B相电压
    cJSON_AddFloatToObject(pRoot, "6",pcs_union.pcsData.Out_Vw_Vol, 1);

    pcs_union.pcsData.Out_Wu_Vol = esRunData.fV_PCS3Phase[index][2]; //交流C相电压
    cJSON_AddFloatToObject(pRoot, "7",pcs_union.pcsData.Out_Wu_Vol, 1);

    pcs_union.pcsData.Out_U_Cur  = esRunData.fI_PCS3Phase[index][0]; //交流A相电流
    cJSON_AddFloatToObject(pRoot, "8",pcs_union.pcsData.Out_U_Cur, 1);

    pcs_union.pcsData.Out_V_Cur  = esRunData.fI_PCS3Phase[index][1]; //交流B相电流
    cJSON_AddFloatToObject(pRoot, "9",pcs_union.pcsData.Out_V_Cur, 1);

    pcs_union.pcsData.Out_W_Cur  = esRunData.fI_PCS3Phase[index][2]; //交流C相电流
    cJSON_AddFloatToObject(pRoot, "10",pcs_union.pcsData.Out_W_Cur, 1);

    pcs_union.pcsData.Grd_F      = esRunData.fAC_Freq; //电网频率
    cJSON_AddFloatToObject(pRoot, "11",pcs_union.pcsData.Grd_F, 1);

    pcs_union.pcsData.Ac_U_Pow_P = (float)esRunData.fV_PCS3Phase[index][0]*esRunData.fI_PCS3Phase[index][0]*0.001; //交流A相有功功率
    cJSON_AddFloatToObject(pRoot, "12",pcs_union.pcsData.Ac_U_Pow_P, 1);

    pcs_union.pcsData.Ac_V_Pow_P = (float)esRunData.fV_PCS3Phase[index][1]*esRunData.fI_PCS3Phase[index][1]*0.001; //交流B相有功功率
    cJSON_AddFloatToObject(pRoot, "13",pcs_union.pcsData.Ac_V_Pow_P, 1);

    pcs_union.pcsData.Ac_W_Pow_P = (float)esRunData.fV_PCS3Phase[index][2]*esRunData.fI_PCS3Phase[index][2]*0.001; //交流C相有功功率
    cJSON_AddFloatToObject(pRoot, "14",pcs_union.pcsData.Ac_W_Pow_P, 1);

    pcs_union.pcsData.Out_Pow_P  = esRunData.fActvPwr_PCSGrp[index];   //总输出有功功率
    cJSON_AddFloatToObject(pRoot, "15",pcs_union.pcsData.Out_Pow_P, 1);

    pcs_union.pcsData.Out_Pow_Q  = esRunData.fQ_PCSGrp[index];  //总输出无功功率
    cJSON_AddFloatToObject(pRoot, "19",pcs_union.pcsData.Out_Pow_Q, 1);

    if((ABS(esRunData.fActvPwr_PCSGrp[index]*esRunData.fActvPwr_PCSGrp[index]) + ABS(esRunData.fQ_PCSGrp[index]*esRunData.fQ_PCSGrp[index])) < 0.0001f)
    {//总输出功率因数
        pcs_union.pcsData.Out_Pow_Pf = 0.0f; // Avoid division by zero
    }
    else
    {
        pcs_union.pcsData.Out_Pow_Pf = ABS(esRunData.fActvPwr_PCSGrp[index]) / sqrt(ABS(esRunData.fActvPwr_PCSGrp[index]*esRunData.fActvPwr_PCSGrp[index]) + ABS(esRunData.fQ_PCSGrp[index]*esRunData.fQ_PCSGrp[index]));
    }
    //直流侧
    cJSON_AddFloatToObject(pRoot, "27",pcs_union.pcsData.Out_Pow_Pf, 1);

    pcs_union.pcsData.Power_Input = (float)blobSiglBatt.fPackTotalVolt*blobSiglBatt.fPackTotalCurr*0.001; //PCS输入功率
    cJSON_AddFloatToObject(pRoot, "28",pcs_union.pcsData.Power_Input, 1);

    pcs_union.pcsData.Vol_Input = blobSiglBatt.fPackTotalVolt; //PCS输入电压
    cJSON_AddFloatToObject(pRoot, "29",pcs_union.pcsData.Vol_Input, 1);

    pcs_union.pcsData.Cur_Input = blobSiglBatt.fPackTotalCurr; //PCS输入电流
    cJSON_AddFloatToObject(pRoot, "30",pcs_union.pcsData.Cur_Input, 1);

    pcs_union.pcsData.Disstemp =  esRunData.fAmbit_Temp; //散热器温度
    cJSON_AddFloatToObject(pRoot, "31",pcs_union.pcsData.Disstemp, 1);

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			407, 1491, &valGetSatus, sizeof(PARAM_VAL_GET));
    float chg_cur = valGetSatus.val.fValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
            407, 1496, &valGetSatus, sizeof(PARAM_VAL_GET));
    float dis_cur = valGetSatus.val.fValue;

    pcs_union.pcsData.Ac_Acc_Chg_Cur = chg_cur; //交流累计充电电量
    cJSON_AddFloatToObject(pRoot, "32",pcs_union.pcsData.Ac_Acc_Chg_Cur, 1);

    pcs_union.pcsData.Ac_Acc_Dis_Cur = dis_cur; //交流累计放电电量
    cJSON_AddFloatToObject(pRoot, "33",pcs_union.pcsData.Ac_Acc_Dis_Cur, 1);

    pcs_union.pcsData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "37", pcs_union.pcsData.Ts);
    
    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_FIREData(void* pArg, int index)
{
    //printf("prepare_FIREData\n");

    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;
    FIRE_UNION           fire_union;
    ZERO_POBJS(&fire_union, 1);

    valGet.nGetSize = 4;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		501 + index, 813, &valGet, sizeof(PARAM_VAL_GET));
    int fault = valGet.val.emValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		601, 1644, &valGet, sizeof(PARAM_VAL_GET));
    float Co = valGet.val.fValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		611, 1673, &valGet, sizeof(PARAM_VAL_GET));
    float Pdu_Temp = valGet.val.fValue;
    

    fire_union.fireData.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", fire_union.fireData.ArrNum);

    fire_union.fireData.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", fire_union.fireData.CluNum);

    fire_union.fireData.Devno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "2", fire_union.fireData.Devno);

    fire_union.fireData.Sysfault = (UINT)fault; //故障类型
    cJSON_AddNumberToObject(pRoot, "5", fire_union.fireData.Sysfault);

    fire_union.fireData.Co_Concentration = (UINT)Co; //co浓度
    cJSON_AddNumberToObject(pRoot, "7", fire_union.fireData.Co_Concentration);

    fire_union.fireData.Fire_Temp = (UINT)Pdu_Temp; //温度值
    cJSON_AddNumberToObject(pRoot, "8", fire_union.fireData.Fire_Temp);

    fire_union.fireData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "10", fire_union.fireData.ArrNum);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_LiqcoolData(void* pArg, int index)
{
    //printf("prepare_LiqcoolArray\n");
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;  
    LIQCOOL_UNION        liqcool_union;
    ZERO_POBJS(&liqcool_union, 1);

    liqcool_union.liqcoolData.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", liqcool_union.liqcoolData.ArrNum);

    liqcool_union.liqcoolData.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", liqcool_union.liqcoolData.CluNum);

    liqcool_union.liqcoolData.Devno = (UINT)index;	
    cJSON_AddNumberToObject(pRoot, "2", liqcool_union.liqcoolData.Devno);
			
    valGet.nGetSize = 4;
    float inletwater_temp = 0;
    float outletwater_temp = 0;
    float hyd_pressure = 0;
    float liq_pressure = 0;
    if(pMqtt->emAirType == TONGFEI_AIR)
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1779, &valGet, sizeof(PARAM_VAL_GET));
        inletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1780, &valGet, sizeof(PARAM_VAL_GET));
        outletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1789, &valGet, sizeof(PARAM_VAL_GET));
        hyd_pressure = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1790, &valGet, sizeof(PARAM_VAL_GET));
        liq_pressure = valGet.val.fValue;
    }
    else if(pMqtt->emAirType == IMEU3_AIR)
    {
        //printf("prepare_LiqcoolArray: IMEU3_AIR\n");
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3535, &valGet, sizeof(PARAM_VAL_GET));
        inletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3536, &valGet, sizeof(PARAM_VAL_GET));
        outletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3545, &valGet, sizeof(PARAM_VAL_GET));
        hyd_pressure = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3544, &valGet, sizeof(PARAM_VAL_GET));
        liq_pressure = valGet.val.fValue;
    }
    else if(pMqtt->emAirType == INFYPOWER_AIR)
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1541, &valGet, sizeof(PARAM_VAL_GET));
        inletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1542, &valGet, sizeof(PARAM_VAL_GET));
        outletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1545, &valGet, sizeof(PARAM_VAL_GET));
        hyd_pressure = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1544, &valGet, sizeof(PARAM_VAL_GET));
        liq_pressure = valGet.val.fValue;
    }
    else if(pMqtt->emAirType == XINMIAO_AIR)
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3648, &valGet, sizeof(PARAM_VAL_GET));
        inletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3647, &valGet, sizeof(PARAM_VAL_GET));
        outletwater_temp = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3649, &valGet, sizeof(PARAM_VAL_GET));
        hyd_pressure = valGet.val.fValue;
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3650, &valGet, sizeof(PARAM_VAL_GET));
        liq_pressure = valGet.val.fValue;			
    }
    liqcool_union.liqcoolData.Hot_Water_Tmp = inletwater_temp; //供液温度
    cJSON_AddFloatToObject(pRoot, "9", liqcool_union.liqcoolData.Hot_Water_Tmp, 1);

    liqcool_union.liqcoolData.Inlet_Water_Tmp = outletwater_temp; //回液温度
    cJSON_AddFloatToObject(pRoot, "10", liqcool_union.liqcoolData.Inlet_Water_Tmp, 1);

    liqcool_union.liqcoolData.Hydraulic_Pressure = hyd_pressure; //供水压力
    cJSON_AddFloatToObject(pRoot, "14", liqcool_union.liqcoolData.Hydraulic_Pressure, 1);

    liqcool_union.liqcoolData.Liquid_Pressure  =  liq_pressure; //补水压力  
    cJSON_AddFloatToObject(pRoot, "15", liqcool_union.liqcoolData.Liquid_Pressure, 1);

    liqcool_union.liqcoolData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "22", liqcool_union.liqcoolData.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;      
}

int prepare_DrierData(void* pArg, int index)
{
    //printf("prepare_DrierArray\n");
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err; 
    DRIER_UNION         drier_union;
    ZERO_POBJS(&drier_union, 1);

    int i = 0;
    valGet.nGetSize = 4;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 601 + index, 2900, &valGet, sizeof(PARAM_VAL_GET));
    float Env = valGet.val.fValue;

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 611, 1673, &valGet, sizeof(PARAM_VAL_GET)); 
    float temp = valGet.val.fValue;

    drier_union.drierData.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", drier_union.drierData.ArrNum);

    drier_union.drierData.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", drier_union.drierData.CluNum);

    drier_union.drierData.Devno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "2", drier_union.drierData.Devno);

    drier_union.drierData.Env_Tem = (UINT)temp; //装置所测温度值
    cJSON_AddNumberToObject(pRoot, "4", drier_union.drierData.Env_Tem);

    drier_union.drierData.Env_Wet = (UINT)Env; //装置所测湿度值
    cJSON_AddNumberToObject(pRoot, "5", drier_union.drierData.Env_Wet);

    drier_union.drierData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "13", drier_union.drierData.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_MeterData(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err; 
    METER_UNION          meter_union;
    ZERO_POBJS(&meter_union, 1);

    valGet.nGetSize = 4;

    meter_union.meterData.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", meter_union.meterData.ArrNum);

    meter_union.meterData.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", meter_union.meterData.CluNum);

    meter_union.meterData.Devno  = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "2", meter_union.meterData.Devno);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1491, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Add_Pos = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "3", meter_union.meterData.Meter_Add_Pos, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1492, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Ver_Pos = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "4", meter_union.meterData.Meter_Ver_Pos, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1493, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Hig_Pos = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "5", meter_union.meterData.Meter_Hig_Pos, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1494, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Mid_Pos = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "6", meter_union.meterData.Meter_Mid_Pos, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1495, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Low_Pos = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "7", meter_union.meterData.Meter_Low_Pos, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1496, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Add_Neg = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "8", meter_union.meterData.Meter_Add_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1497, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Ver_Neg = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "9", meter_union.meterData.Meter_Ver_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1498, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Hig_Neg = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "10", meter_union.meterData.Meter_Hig_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1499, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Mid_Neg = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "11", meter_union.meterData.Meter_Mid_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 1500, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Low_Neg = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "12", meter_union.meterData.Meter_Low_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 407 + index, 407, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Pt = valGet.val.fValue;  //PT
    cJSON_AddFloatToObject(pRoot, "13", meter_union.meterData.Meter_Low_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 407 + index, 402, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Ct = valGet.val.fValue;  //CT
    cJSON_AddFloatToObject(pRoot, "14", meter_union.meterData.Meter_Low_Neg, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 413, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_A_Vol = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "15", meter_union.meterData.Meter_A_Vol, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 415, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_B_Vol = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "16", meter_union.meterData.Meter_B_Vol, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 417, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_C_Vol = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "17", meter_union.meterData.Meter_C_Vol, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 414, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_A_Cur = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "18", meter_union.meterData.Meter_A_Cur, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 416, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_B_Cur = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "19", meter_union.meterData.Meter_B_Cur, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 418, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_C_Cur = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "20", meter_union.meterData.Meter_C_Cur, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 424, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_F = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "21", meter_union.meterData.Meter_F, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 400, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Tot_P = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "22", meter_union.meterData.Meter_Tot_P, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 404, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Tot_Q =valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "23", meter_union.meterData.Meter_Tot_Q, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 426, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_Tot_Pf = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "24", meter_union.meterData.Meter_Tot_Pf, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 400, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_All_Pow = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "25", meter_union.meterData.Meter_All_Pow, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 401, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_A_Pow = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "26", meter_union.meterData.Meter_A_Pow, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 402, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_B_Pow = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "27", meter_union.meterData.Meter_B_Pow, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 403, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_C_Pow = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "28", meter_union.meterData.Meter_C_Pow, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 404, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_All_Qre = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "29", meter_union.meterData.Meter_All_Qre, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 405, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_A_Qre = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "30", meter_union.meterData.Meter_A_Qre, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 406, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_B_Qre = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "31", meter_union.meterData.Meter_B_Qre, 1);

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 407 + index, 407, &valGet, sizeof(PARAM_VAL_GET));
    meter_union.meterData.Meter_C_Qre = valGet.val.fValue;
    cJSON_AddFloatToObject(pRoot, "32", meter_union.meterData.Meter_C_Qre, 1);

    meter_union.meterData.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "39", meter_union.meterData.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_CellData(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err; 

 	A_BLOB_IFY_BMS_DATAINFO blobIfySiglBat;
	ZERO_POBJS(&blobIfySiglBat, 1);
	blobIfySiglBat.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
	valGet.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
	valGet.val.blobValue = (BYTE*)&blobIfySiglBat;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			501, 557, &valGet, sizeof(PARAM_VAL_GET));

    int i = 0;
    int PackNum = 0, SingleCellNum = 0;
    PARAM_VAL_GET   valGetSatus;
  	valGetSatus.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			500, 1944, &valGetSatus, sizeof(PARAM_VAL_GET));
    PackNum = valGetSatus.val.ulValue;  

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
			500, 1945, &valGetSatus, sizeof(PARAM_VAL_GET));
    SingleCellNum = valGetSatus.val.ulValue;

    CETT_DATA        cell_data;
 
    char key[10];
    int idx = 0;
    cell_data.ArrNo = (UINT)index;
    cell_data.CluNo = (UINT)index;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, cell_data.ArrNo);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, cell_data.CluNo);
    idx++;   

    cJSON	*pvlo_Array = cJSON_CreateArray();
    cJSON	*pTmp_Array = cJSON_CreateArray();
    cJSON	*pSOC_Array = cJSON_CreateArray();
    cJSON	*pSOH_Array = cJSON_CreateArray();

    for(i = 0; i < PackNum*SingleCellNum; ++i)
    {
        if(PackNum*SingleCellNum > A_CABINET_CELL_MAX_NUM)
        {
            break; 
        }
        cell_data.CelVol = (float)blobIfySiglBat.usCellVolt[i]*0.001;  //单体电压
        // printf("Cell %d, Vol: %d\n", i, cell_data.CelVol);
        cJSON_AddItemToArray(pvlo_Array, cJSON_CreateFloat(cell_data.CelVol, 2));    
    }
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, pvlo_Array);
    idx++;  

    for(i = 0; i < PackNum*SingleCellNum; ++i)
    {
        if(PackNum*SingleCellNum > A_CABINET_CELL_MAX_NUM)
        {
            break; 
        }
        cell_data.CelTem = blobIfySiglBat.cCellTemp[i];  //单体温度
        cJSON_AddItemToArray(pTmp_Array, cJSON_CreateFloat(cell_data.CelTem, 2));
    }
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, pTmp_Array);
    idx++; 

    for(i = 0; i < PackNum*SingleCellNum; ++i)
    {
        cell_data.CelSOC = blobIfySiglBat.fSOC;  //单体SOC
        cJSON_AddItemToArray(pSOC_Array, cJSON_CreateNumber(cell_data.CelSOC));
    }

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, pSOC_Array);
    idx++; 

    for(i = 0; i < PackNum*SingleCellNum; ++i)
    {
        cell_data.CelSOH = blobIfySiglBat.fSOH;  //单体SOH
        cJSON_AddItemToArray(pSOH_Array, cJSON_CreateNumber(cell_data.CelSOH));
    }
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, pSOH_Array);
    idx++; 

    cell_data.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, cell_data.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

//只获取枚举状态参数值参数，主要是获取告警状态
int GetStatusPar(MQTT_DATA_ROUGH *pMqtt, int DEV, int Parameter)
{
    REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;

    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;
    
    valGet.nGetSize = 4;
    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		DEV, Parameter, &valGet, sizeof(PARAM_VAL_GET));

    if(err != ERRCODE_DAI_OK)
    {
        return 0; // Error in getting value
    }

    return valGet.val.emValue;
}

int prepare_ArrStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet, valGet2;
    ERRCODE_DAI 		err;

    A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
    ZERO_POBJS(&blobSiglBatt, 1);
    blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
    valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
    valGet.val.blobValue = (BYTE*)&blobSiglBatt;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 551, &valGet, sizeof(PARAM_VAL_GET));

	A_BLOB_ES_RUN_DATA	esRunData;
 	ZERO_POBJS(&esRunData, 1);
    esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet2.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet2.val.blobValue = (BYTE*)&esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet2, sizeof(PARAM_VAL_GET));

    ARR_STATUS_UNION       arrstatus_union;
    ZERO_POBJS(&arrstatus_union, 1);

    arrstatus_union.arrStatus.arrno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", arrstatus_union.arrStatus.arrno);

    arrstatus_union.arrStatus.cluVolLowWarnL1 = (blobSiglBatt.fPackTotalVolt < 715.0f) ? 1 : 0; // 电池总压低预警 堆内各簇端电压欠压预警(轻度)
    cJSON_AddNumberToObject(pRoot, "2", arrstatus_union.arrStatus.cluVolLowWarnL1);

    arrstatus_union.arrStatus.cluVolLowWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 643); // 电池总压低二级告警 堆内各簇端电压欠压告警(中度)
    cJSON_AddNumberToObject(pRoot, "3", arrstatus_union.arrStatus.cluVolLowWarnL2);

    arrstatus_union.arrStatus.cluVolLowWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 659); // 电池总压低一级告警 堆内各簇端电压欠压保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "4", arrstatus_union.arrStatus.cluVolLowWarnL3);

    arrstatus_union.arrStatus.cluVolUpWarnL1 = (blobSiglBatt.fPackTotalVolt > 840.0f) ? 1 : 0;// 电池总压高预警 堆内各簇端电压过压预警(轻度) 
    cJSON_AddNumberToObject(pRoot, "5", arrstatus_union.arrStatus.cluVolUpWarnL1);

    arrstatus_union.arrStatus.cluVolUpWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 642); //电池总压高二级告警 堆内各簇端电压过压告警(中度)
    cJSON_AddNumberToObject(pRoot, "6", arrstatus_union.arrStatus.cluVolUpWarnL2);

    arrstatus_union.arrStatus.cluVolUpWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 658); //电池总压高一级告警 堆内各簇端电压过压保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "7", arrstatus_union.arrStatus.cluVolUpWarnL3);

    arrstatus_union.arrStatus.cluCurUpWarnL1 = (blobSiglBatt.fPackTotalCurr > 190.0f) ? 1 : 0; //充电过流预警 放电过流预警 堆内各簇端过流预警(轻度)
    cJSON_AddNumberToObject(pRoot, "8", arrstatus_union.arrStatus.cluCurUpWarnL1);

    arrstatus_union.arrStatus.cluCurUpWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 648) | (UINT)GetStatusPar(pMqtt, 501, 649); //充电过流二级告警  放电过流二级告警 堆内各簇端过流告警(中度)
    cJSON_AddNumberToObject(pRoot, "9", arrstatus_union.arrStatus.cluCurUpWarnL2);

    arrstatus_union.arrStatus.cluCurUpWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 664) | (UINT)GetStatusPar(pMqtt, 501, 665); //充电过流一级告警  放电过流一级告警 堆内各簇端过流保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "10", arrstatus_union.arrStatus.cluCurUpWarnL3);

    if(ABS(blobSiglBatt.fPackTotalVolt) < 0.00001)
    {
        arrstatus_union.arrStatus.cluRegLowWarnL1 = 0;
    }
    else
    {//电池正极和负极绝缘预警  堆内各簇绝缘电阻低预警(轻度)
        arrstatus_union.arrStatus.cluRegLowWarnL1 = ((blobSiglBatt.fInsulation_P_R*1000/blobSiglBatt.fPackTotalVolt < 400.0f)? 1 : 0)|((blobSiglBatt.fInsulation_N_R*1000/blobSiglBatt.fPackTotalVolt < 400.0f)? 1 : 0); //电池绝缘预警
    }
    cJSON_AddNumberToObject(pRoot, "11", arrstatus_union.arrStatus.cluRegLowWarnL1);

    arrstatus_union.arrStatus.cluRegLowWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 651); //电池绝缘二级告警 堆内各簇绝缘电阻低告警(中度)
    cJSON_AddNumberToObject(pRoot, "12", arrstatus_union.arrStatus.cluRegLowWarnL2);

    arrstatus_union.arrStatus.cluRegLowWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 667); //电池绝缘一级告警 堆内各簇绝缘电阻低保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "13", arrstatus_union.arrStatus.cluRegLowWarnL3);

    arrstatus_union.arrStatus.devModuleTemLowWarnL1 = (esRunData.fAmbit_Temp < -10.0f) ? 1 : 0; //模块过低预警 堆内各簇模块欠温度预警(轻度)
    cJSON_AddNumberToObject(pRoot, "14", arrstatus_union.arrStatus.devModuleTemLowWarnL1);

    arrstatus_union.arrStatus.devModuleTemLowWarnL2 = (esRunData.fAmbit_Temp < -15.0f) ? 1 : 0; //模块过低告警 堆内各簇模块欠温度告警(中度)
    cJSON_AddNumberToObject(pRoot, "15", arrstatus_union.arrStatus.devModuleTemLowWarnL2);

    arrstatus_union.arrStatus.devModuleTemLowWarnL3 = (esRunData.fAmbit_Temp < -20.0f) ? 1 : 0; //模块过低严重告警 堆内各簇模块欠温度保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "16", arrstatus_union.arrStatus.devModuleTemLowWarnL3);

    arrstatus_union.arrStatus.devModuleTemUpWarnL1 = (UINT)GetStatusPar(pMqtt, 251, 1304) | (UINT)GetStatusPar(pMqtt, 252 , 1304) | (UINT)GetStatusPar(pMqtt, 253, 1304) | (UINT)GetStatusPar(pMqtt, 254, 1304); //环温过温 堆内各簇模块过温度预警(轻度)
    cJSON_AddNumberToObject(pRoot, "17", arrstatus_union.arrStatus.devModuleTemUpWarnL1);

    arrstatus_union.arrStatus.devModuleTemUpWarnL2 = (UINT)GetStatusPar(pMqtt, 251, 1303) | (UINT)GetStatusPar(pMqtt, 252 , 1303) | (UINT)GetStatusPar(pMqtt, 253, 1303) | (UINT)GetStatusPar(pMqtt, 254, 1303); //散热器过温 堆内各簇模块过温度告警(中度)
    cJSON_AddNumberToObject(pRoot, "18", arrstatus_union.arrStatus.devModuleTemUpWarnL2);

    arrstatus_union.arrStatus.devModuleTemUpWarnL3 = (UINT)GetStatusPar(pMqtt, 251, 1221) | (UINT)GetStatusPar(pMqtt, 252 , 1221) | (UINT)GetStatusPar(pMqtt, 253, 1221) | (UINT)GetStatusPar(pMqtt, 254, 1221); //模块过温告警 堆内各簇模块过温度保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "19", arrstatus_union.arrStatus.devModuleTemUpWarnL3);

    arrstatus_union.arrStatus.cellVolUpWarnL1 = (blobSiglBatt.fMax_CellVolt > 3.55f) ? 1 : 0; //单体电压高预警  堆内各单体过压预警(轻度)
    cJSON_AddNumberToObject(pRoot, "20", arrstatus_union.arrStatus.cellVolUpWarnL1);

    arrstatus_union.arrStatus.cellVolUpWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 640); //单体电压高二级告警 堆内各单体过压告警(中度)
    cJSON_AddNumberToObject(pRoot, "21", arrstatus_union.arrStatus.cellVolUpWarnL2);

    arrstatus_union.arrStatus.cellVolUpWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 656); //单体电压高一级告警 堆内各单体过压保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "22", arrstatus_union.arrStatus.cellVolUpWarnL3);

    arrstatus_union.arrStatus.cellVolLowWarnL1 = (blobSiglBatt.fMin_CellVolt < 2.85f) ? 1 : 0; //单体电压低预警 堆内各单体欠压预警(轻度)
    cJSON_AddNumberToObject(pRoot, "23", arrstatus_union.arrStatus.cellVolLowWarnL1);

    arrstatus_union.arrStatus.cellVolLowWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 641); //单体电压低二级告警 堆内各单体欠压告警(中度)
    cJSON_AddNumberToObject(pRoot, "24", arrstatus_union.arrStatus.cellVolLowWarnL2);

    arrstatus_union.arrStatus.cellVolLowWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 657); //单体电压低一级告警 堆内各单体欠压保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "25", arrstatus_union.arrStatus.cellVolLowWarnL3);

    arrstatus_union.arrStatus.cellVolDifWarnL1 = (ABS(blobSiglBatt.fMax_CellVolt - blobSiglBatt.fMin_CellVolt) > 0.15f) ? 1 : 0; //单体压差过大预警 堆内各单体压差预警(轻度)
    cJSON_AddNumberToObject(pRoot, "26", arrstatus_union.arrStatus.cellVolDifWarnL1);

    arrstatus_union.arrStatus.cellVolDifWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 574); //单体压差过大二级告警 堆内各单体压差告警(中度)
    cJSON_AddNumberToObject(pRoot, "27", arrstatus_union.arrStatus.cellVolDifWarnL2);

    arrstatus_union.arrStatus.cellVolDifWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 668); //单体压差过大一级告警 堆内各单体压差保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "28", arrstatus_union.arrStatus.cellVolDifWarnL3);

    arrstatus_union.arrStatus.cellTemLowWarnL1 = (blobSiglBatt.fMin_CellTemp < 5.0f) ? 1 : 0; // 充电温度低预警 放电温度低预警 堆内各单体欠温预警(轻度) 
    cJSON_AddNumberToObject(pRoot, "29", arrstatus_union.arrStatus.cellTemLowWarnL1);

    arrstatus_union.arrStatus.cellTemLowWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 645)|(UINT)GetStatusPar(pMqtt, 501 + index, 647); // 充电温度低二级告警 放电温度低二级告警 堆内各单体欠温告警(中度)
    cJSON_AddNumberToObject(pRoot, "30", arrstatus_union.arrStatus.cellTemLowWarnL2);

    arrstatus_union.arrStatus.cellTemLowWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 661)|(UINT)GetStatusPar(pMqtt, 501 + index, 663); //充电温度低一级告警 放电温度低一级告警  堆内各单体欠温保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "31", arrstatus_union.arrStatus.cellTemLowWarnL3);

    arrstatus_union.arrStatus.cellTemUpWarnL1 = (blobSiglBatt.fMax_CellTemp > 48.0f) ? 1 : 0; // 充电温度高预警 放电温度高预警 堆内各单体过温预警(轻度)
    cJSON_AddNumberToObject(pRoot, "32", arrstatus_union.arrStatus.cellTemUpWarnL1);

    arrstatus_union.arrStatus.cellTemUpWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 644)|(UINT)GetStatusPar(pMqtt, 501 + index, 646); //充电温度高二级告警 放电温度高二级告警 堆内各单体过温告警(中度)
    cJSON_AddNumberToObject(pRoot, "33", arrstatus_union.arrStatus.cellTemUpWarnL2);

    arrstatus_union.arrStatus.cellTemUpWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 660)|(UINT)GetStatusPar(pMqtt, 501 + index, 662); //充电温度高一级告警 放电温度高一级告警 堆内各单体过温保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "34", arrstatus_union.arrStatus.cellTemUpWarnL3);

    arrstatus_union.arrStatus.cellTemDifWarnL1 = (ABS(blobSiglBatt.fMax_CellTemp - blobSiglBatt.fMin_CellTemp) > 10.0f) ? 1 : 0; //单体温差过大预警 堆内各单体温差预警(轻度)
    cJSON_AddNumberToObject(pRoot, "35", arrstatus_union.arrStatus.cellTemDifWarnL1);

    arrstatus_union.arrStatus.cellTemDifWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 653); //单体温差过大二级告警 堆内各单体温差告警(中度)
    cJSON_AddNumberToObject(pRoot, "36", arrstatus_union.arrStatus.cellTemDifWarnL2);

    arrstatus_union.arrStatus.cellTemDifWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 669); //单体温差过大一级告警 堆内各单体温差保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "37", arrstatus_union.arrStatus.cellTemDifWarnL3);

    if(ABS(blobSiglBatt.fPackTotalVolt) < 0.00001f)
    {
        arrstatus_union.arrStatus.arrPosresWarnL1 = 0; //正母绝缘预警(轻度)
        arrstatus_union.arrStatus.arrPosresWarnL2 = 0; //正母绝缘告警(中度)
        arrstatus_union.arrStatus.arrPosresWarnL3 = 0; //正母绝缘保护动作(重度)
        arrstatus_union.arrStatus.arrNegresWarnL1 = 0; //负母绝缘预警(轻度)
        arrstatus_union.arrStatus.arrNegresWarnL2 = 0; //负母绝缘告警(中度)
        arrstatus_union.arrStatus.arrNegresWarnL3 = 0; //负母绝缘保护动作(重度)
    }
    else
    {
        arrstatus_union.arrStatus.arrPosresWarnL1 = (blobSiglBatt.fInsulation_P_R*1000/blobSiglBatt.fPackTotalVolt < 400.0f)? 1 : 0;
        arrstatus_union.arrStatus.arrPosresWarnL2 = (blobSiglBatt.fInsulation_P_R*1000/blobSiglBatt.fPackTotalVolt < 300.0f)? 1 : 0;
        arrstatus_union.arrStatus.arrPosresWarnL3 = (blobSiglBatt.fInsulation_P_R*1000/blobSiglBatt.fPackTotalVolt < 100.0f)? 1 : 0;
        arrstatus_union.arrStatus.arrNegresWarnL1 = (blobSiglBatt.fInsulation_N_R*1000/blobSiglBatt.fPackTotalVolt < 400.0f)? 1 : 0;
        arrstatus_union.arrStatus.arrNegresWarnL2 = (blobSiglBatt.fInsulation_N_R*1000/blobSiglBatt.fPackTotalVolt < 300.0f)? 1 : 0;
        arrstatus_union.arrStatus.arrNegresWarnL3 = (blobSiglBatt.fInsulation_N_R*1000/blobSiglBatt.fPackTotalVolt < 100.0f)? 1 : 0;    
    }
    cJSON_AddNumberToObject(pRoot, "44", arrstatus_union.arrStatus.arrPosresWarnL1);
    cJSON_AddNumberToObject(pRoot, "45", arrstatus_union.arrStatus.arrPosresWarnL2);
    cJSON_AddNumberToObject(pRoot, "46", arrstatus_union.arrStatus.arrPosresWarnL3);
    cJSON_AddNumberToObject(pRoot, "47", arrstatus_union.arrStatus.arrNegresWarnL1);
    cJSON_AddNumberToObject(pRoot, "48", arrstatus_union.arrStatus.arrNegresWarnL2);
    cJSON_AddNumberToObject(pRoot, "49", arrstatus_union.arrStatus.arrNegresWarnL3);
    
    if(blobSiglBatt.nSOC < 1)
    {
        arrstatus_union.arrStatus.cellSocLowWarnL1 = 1;
    }
    else
    {
         arrstatus_union.arrStatus.cellSocLowWarnL1 = 0; //堆内各单体SOC低预警(轻度)
    }
    cJSON_AddNumberToObject(pRoot, "50", arrstatus_union.arrStatus.cellSocLowWarnL1);

    arrstatus_union.arrStatus.cellSocLowWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 655); //电池SOC低二级告警 堆内各单体SOC低告警(中度)
    cJSON_AddNumberToObject(pRoot, "51", arrstatus_union.arrStatus.cellSocLowWarnL2);

    arrstatus_union.arrStatus.cellSocLowWarnL3 = (UINT)GetStatusPar(pMqtt, 501 + index, 671); //电池SOC低一级告警 堆内各单体SOC低保护动作(重度)
    cJSON_AddNumberToObject(pRoot, "52", arrstatus_union.arrStatus.cellSocLowWarnL3);

    arrstatus_union.arrStatus.cellSocUpWarnL2 = (UINT)GetStatusPar(pMqtt, 501 + index, 654);  //电池SOC高二级告警 堆内各单体SOC高预警(轻度)
    cJSON_AddNumberToObject(pRoot, "55", arrstatus_union.arrStatus.cellSocUpWarnL2);

    arrstatus_union.arrStatus.pcsComWarn = (UINT)GetStatusPar(pMqtt, 200, 369);  //所有模块组通信中断 与PCS通讯中断
    cJSON_AddNumberToObject(pRoot, "67", arrstatus_union.arrStatus.pcsComWarn);

    arrstatus_union.arrStatus.chaDisable = blobSiglBatt.bIsBatt_ChargeForbidden;  //禁止充电标志  充电禁止
    cJSON_AddNumberToObject(pRoot, "73", arrstatus_union.arrStatus.chaDisable); 

    arrstatus_union.arrStatus.dischgDisable = blobSiglBatt.bIsBatt_DisChargeForbidden; //禁止放电标志 放电禁止
    cJSON_AddNumberToObject(pRoot, "74", arrstatus_union.arrStatus.dischgDisable); 

    arrstatus_union.arrStatus.bmsWarn = (blobSiglBatt.emBattStatus_Alm == STBATT_ALMSTAT_ALARMING) ? 1 : 0; //电池告警 BMS系统故障总
    cJSON_AddNumberToObject(pRoot, "75", arrstatus_union.arrStatus.bmsWarn);

    arrstatus_union.arrStatus.bmsFault = (blobSiglBatt.emBattStatus_Alm == STBATT_ALMSTAT_FAULT) ? 1 : 0; //电池故障 BMS系统故障总
    cJSON_AddNumberToObject(pRoot, "76", arrstatus_union.arrStatus.bmsFault);

    arrstatus_union.arrStatus.eclondComFault = (blobSiglBatt.emBattStatus_Run == STBATT_RUNSTAT_COMM_FAIL) ? 1 : 0; //电池通信故障 与平台通讯故障
    cJSON_AddNumberToObject(pRoot, "77", arrstatus_union.arrStatus.eclondComFault);

    arrstatus_union.arrStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "78", arrstatus_union.arrStatus.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 

    return 0;
}

int prepare_CluStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;

    A_BLOB_SINGLE_STBATT_RUNINFO blobSiglBatt;
    ZERO_POBJS(&blobSiglBatt, 1);
    blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
    valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
    valGet.val.blobValue = (BYTE*)&blobSiglBatt;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
                501 + index, 551, &valGet, sizeof(PARAM_VAL_GET));

    CLU_STATUS_UNION  clu_stutas_array;
    ZERO_POBJS(&clu_stutas_array, 1);

    clu_stutas_array.cluStatus.arrno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", clu_stutas_array.cluStatus.arrno);

    clu_stutas_array.cluStatus.cluno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", clu_stutas_array.cluStatus.cluno);

    clu_stutas_array.cluStatus.cluVolAbn = (UINT)GetStatusPar(pMqtt, 501 + index, 658) | (UINT)GetStatusPar(pMqtt, 501 + index, 659); //电池总压高一级告警  电池总压低一级告警 总电压不正常
    cJSON_AddNumberToObject(pRoot, "5", clu_stutas_array.cluStatus.cluVolAbn);

    clu_stutas_array.cluStatus.cluCurSensorFault = (UINT)GetStatusPar(pMqtt, 501 + index, 723); //主控电流采样电路异常 霍尔电流传感器故障
    cJSON_AddNumberToObject(pRoot, "6", clu_stutas_array.cluStatus.cluCurSensorFault);

    clu_stutas_array.cluStatus.bmuComWarn = (UINT)GetStatusPar(pMqtt, 501 + index, 715);  //主控通信中断 与电池模组通信故障
    cJSON_AddNumberToObject(pRoot, "8", clu_stutas_array.cluStatus.bmuComWarn);

    clu_stutas_array.cluStatus.cluSocLowWarnLevel = (UINT)GetStatusPar(pMqtt, 501 + index, 655); // 电池SOC低二级告警 簇SOC低预警等级
    cJSON_AddNumberToObject(pRoot, "9", clu_stutas_array.cluStatus.cluSocLowWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 658))
    {
        clu_stutas_array.cluStatus.cluVolUpWarnLevel = 3; // 电池总压高一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 642))
    {
        clu_stutas_array.cluStatus.cluVolUpWarnLevel = 2; //电池总压高二级告警
    }
    else if(blobSiglBatt.fPackTotalVolt > 840.0f)
    {
        clu_stutas_array.cluStatus.cluVolUpWarnLevel = 1; //电池总压高预警
    }
    cJSON_AddNumberToObject(pRoot, "11", clu_stutas_array.cluStatus.cluVolUpWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 659))
    {
        clu_stutas_array.cluStatus.cluVolLowWarnLevel = 3; // 电池总压低一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 643))
    {
        clu_stutas_array.cluStatus.cluVolLowWarnLevel = 2; //电池总压低二级告警
    }
    else if(blobSiglBatt.fPackTotalVolt < 715.0f)
    {
        clu_stutas_array.cluStatus.cluVolLowWarnLevel = 1; //电池总压低预警
    }
    cJSON_AddNumberToObject(pRoot, "12", clu_stutas_array.cluStatus.cluVolLowWarnLevel);
    
    if((UINT)GetStatusPar(pMqtt, 501 + index, 664))
    {
        clu_stutas_array.cluStatus.cluChgCurUpWarnLevel = 3; //充电过流一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 648))
    {
        clu_stutas_array.cluStatus.cluChgCurUpWarnLevel = 2; //充电过流二级告警
    }
    else if(blobSiglBatt.fPackTotalCurr > 190.0f)
    {
        clu_stutas_array.cluStatus.cluChgCurUpWarnLevel = 1; //充电过流预警
    }
    cJSON_AddNumberToObject(pRoot, "13", clu_stutas_array.cluStatus.cluChgCurUpWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 665))
    {
        clu_stutas_array.cluStatus.cluDischgCurUpWarnLevel = 3; //放电过流一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 649))
    {
        clu_stutas_array.cluStatus.cluDischgCurUpWarnLevel = 2; //放电过流二级告警
    }
    else if(blobSiglBatt.fPackTotalCurr > 190.0f)
    {
        clu_stutas_array.cluStatus.cluDischgCurUpWarnLevel = 1; //放电过流预警
    }
    cJSON_AddNumberToObject(pRoot, "14", clu_stutas_array.cluStatus.cluDischgCurUpWarnLevel);
    
    if((UINT)GetStatusPar(pMqtt, 501 + index, 656))
    {
        clu_stutas_array.cluStatus.cellVolUpWarnLevel = 3; //单体电压高一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 640))
    {
        clu_stutas_array.cluStatus.cellVolUpWarnLevel = 2; //单体电压高二级告警
    }
    else if(blobSiglBatt.fMax_CellVolt > 3.55f)
    {
        clu_stutas_array.cluStatus.cellVolUpWarnLevel = 1; //单体电压高预警
    }
    cJSON_AddNumberToObject(pRoot, "15", clu_stutas_array.cluStatus.cellVolUpWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 657))
    {
        clu_stutas_array.cluStatus.cellVolLowWarnLevel = 3; //单体电压低一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 641))
    {
        clu_stutas_array.cluStatus.cellVolLowWarnLevel = 2; //单体电压低二级告警
    }
    else if(blobSiglBatt.fMin_CellVolt < 2.85f)
    {
        clu_stutas_array.cluStatus.cellVolLowWarnLevel = 1; //单体电压低预警
    }
    cJSON_AddNumberToObject(pRoot, "16", clu_stutas_array.cluStatus.cellVolLowWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 660))
    {
        clu_stutas_array.cluStatus.cellChgTemUpWarnLevel = 3; //充电温度高一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 645))
    {
        clu_stutas_array.cluStatus.cellChgTemUpWarnLevel = 2; //充电温度高二级告警
    }
    else if(blobSiglBatt.fMax_CellTemp > 48.0f)
    {
        clu_stutas_array.cluStatus.cellChgTemUpWarnLevel = 1; //充电温度高预警
    }
    cJSON_AddNumberToObject(pRoot, "17", clu_stutas_array.cluStatus.cellChgTemUpWarnLevel);
    
    if((UINT)GetStatusPar(pMqtt, 501 + index, 661))
    {
        clu_stutas_array.cluStatus.cellChgTemLowWarnLevel = 3; //充电温度低一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 645))
    {
        clu_stutas_array.cluStatus.cellChgTemLowWarnLevel = 2; //充电温度低二级告警
    }
    else if(blobSiglBatt.fMin_CellTemp < 5.0f)
    {
        clu_stutas_array.cluStatus.cellChgTemLowWarnLevel = 1; //充电温度低预警
    }
    cJSON_AddNumberToObject(pRoot, "18", clu_stutas_array.cluStatus.cellChgTemLowWarnLevel);
    
    if((UINT)GetStatusPar(pMqtt, 501 + index, 662))
    {
        clu_stutas_array.cluStatus.cellDischgTemUpWarnLevel = 3; //放电温度高一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 646))
    {
        clu_stutas_array.cluStatus.cellDischgTemUpWarnLevel = 2; //放电温度高二级告警
    }
    else if(blobSiglBatt.fMax_CellTemp > 48.0f)
    {
        clu_stutas_array.cluStatus.cellDischgTemUpWarnLevel = 1; //放电温度高预警
    }
    cJSON_AddNumberToObject(pRoot, "19", clu_stutas_array.cluStatus.cellDischgTemUpWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 663))
    {
        clu_stutas_array.cluStatus.cellDischgTemLowWarnLevel = 3; //放电温度低一级告警
    }
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 647))
    {
        clu_stutas_array.cluStatus.cellDischgTemLowWarnLevel = 2; //放电温度低二级告警
    }
    else if(blobSiglBatt.fMin_CellTemp < 5.0f)
    {
        clu_stutas_array.cluStatus.cellDischgTemLowWarnLevel = 1; //放电温度低预警
    }
    cJSON_AddNumberToObject(pRoot, "20", clu_stutas_array.cluStatus.cellDischgTemLowWarnLevel);

    clu_stutas_array.cluStatus.cellTemUpWarnLevel = (UINT)GetStatusPar(pMqtt, 501 + index, 562); //单体过温报警
    cJSON_AddNumberToObject(pRoot, "21", clu_stutas_array.cluStatus.cellTemUpWarnLevel);

    clu_stutas_array.cluStatus.cellTemLowWarnLevel = (UINT)GetStatusPar(pMqtt, 501 + index, 563); //单体低温报警
    cJSON_AddNumberToObject(pRoot, "22", clu_stutas_array.cluStatus.cellTemLowWarnLevel);

    clu_stutas_array.cluStatus.cellSocLowWarnLevel = (UINT)GetStatusPar(pMqtt, 501 + index, 655); //电池SOC低二级告警
    cJSON_AddNumberToObject(pRoot, "23", clu_stutas_array.cluStatus.cellSocLowWarnLevel);

    clu_stutas_array.cluStatus.cellSocUpWarnLevel = (UINT)GetStatusPar(pMqtt, 501 + index, 654); //电池SOC高二级告警
    cJSON_AddNumberToObject(pRoot, "24", clu_stutas_array.cluStatus.cellSocUpWarnLevel);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 668)) 
    {
        clu_stutas_array.cluStatus.cellVolDifWarnLevel = 3; //单体压差过大一级告警
    }  
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 574)) 
    {
        clu_stutas_array.cluStatus.cellVolDifWarnLevel = 2; //单体压差过大二级告警
    }  
    else if(ABS(blobSiglBatt.fMax_CellVolt - blobSiglBatt.fMin_CellVolt) > 0.15f)
    {
        clu_stutas_array.cluStatus.cellVolDifWarnLevel = 1; //单体压差过大预警
    }  
    cJSON_AddNumberToObject(pRoot, "26", clu_stutas_array.cluStatus.cellVolDifWarnLevel);
    
    if((UINT)GetStatusPar(pMqtt, 501 + index, 669))
    {
        clu_stutas_array.cluStatus.cellTemDifWarnLevel = 3; //单体温差过大一级告警
    }  
    else if((UINT)GetStatusPar(pMqtt, 501 + index, 653))
    {
        clu_stutas_array.cluStatus.cellTemDifWarnLevel = 2; //单体温差过大二级告警
    }  
    else if(ABS(blobSiglBatt.fMax_CellTemp - blobSiglBatt.fMin_CellTemp) > 10.0f)
    {
        clu_stutas_array.cluStatus.cellTemDifWarnLevel = 1; //单体温差过大预警
    }
    cJSON_AddNumberToObject(pRoot, "27", clu_stutas_array.cluStatus.cellTemDifWarnLevel);
    
    clu_stutas_array.cluStatus.cluPositiveRelaySt = ((UINT)GetStatusPar(pMqtt, 501 + index, 736) == 1)? 0:1; //电池组主正接触器异常 簇正接触器位置
    cJSON_AddNumberToObject(pRoot, "28", clu_stutas_array.cluStatus.cluPositiveRelaySt);

    clu_stutas_array.cluStatus.cluNegativeRelaySt = ((UINT)GetStatusPar(pMqtt, 501 + index, 738) == 1)? 0:1; //电池组主负接触器异常 簇负接触器位置
    cJSON_AddNumberToObject(pRoot, "29", clu_stutas_array.cluStatus.cluNegativeRelaySt);

    clu_stutas_array.cluStatus.cluPrePos  = ((UINT)GetStatusPar(pMqtt, 501 + index, 737) == 1)? 0:1; //电池组预充接触器异常 簇预充接触器位置
    cJSON_AddNumberToObject(pRoot, "30", clu_stutas_array.cluStatus.cluPrePos);

    clu_stutas_array.cluStatus.chaDisable = blobSiglBatt.bIsBatt_ChargeForbidden;  //充电禁止
    cJSON_AddNumberToObject(pRoot, "35", clu_stutas_array.cluStatus.chaDisable);

    clu_stutas_array.cluStatus.dischgDisable = blobSiglBatt.bIsBatt_DisChargeForbidden;  //放电禁止
    cJSON_AddNumberToObject(pRoot, "36", clu_stutas_array.cluStatus.dischgDisable);

    clu_stutas_array.cluStatus.bmsWarn = (blobSiglBatt.emBattStatus_Alm == STBATT_ALMSTAT_ALARMING) ? 1 : 0; //电池告警 BMS系统告警汇总
    cJSON_AddNumberToObject(pRoot, "37", clu_stutas_array.cluStatus.bmsWarn);

    clu_stutas_array.cluStatus.bmsFault = (blobSiglBatt.emBattStatus_Alm == STBATT_ALMSTAT_FAULT) ? 1 : 0; //电池故障 BMS系统故障汇总
    cJSON_AddNumberToObject(pRoot, "38", clu_stutas_array.cluStatus.bmsFault);

    clu_stutas_array.cluStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "39", clu_stutas_array.cluStatus.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 

    return 0;
}

int prepare_EmuStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;

    EMU_STATUS_UNION           emustatus_union;
    ZERO_POBJS(&emustatus_union, 1);

    emustatus_union.emuStatus.pcsFault = (UINT)GetStatusPar(pMqtt, 200, 368); //所有模块组失效 PCS故障
    cJSON_AddNumberToObject(pRoot, "0", emustatus_union.emuStatus.pcsFault);

    if((UINT)GetStatusPar(pMqtt, 200, 103) == 2)
    {
        emustatus_union.emuStatus.sysFault = 1;  //CCU故障 系统故障 
    }
    else
    {
        emustatus_union.emuStatus.sysFault = 0;
    }
    cJSON_AddNumberToObject(pRoot, "1", emustatus_union.emuStatus.sysFault);
    
    // 消防1级告警  消防2级告警 消防3级告警 消防反馈
    emustatus_union.emuStatus.fireFeedBack = (UINT)GetStatusPar(pMqtt, 501 + index, 814) | (UINT)GetStatusPar(pMqtt, 501 + index, 815) | (UINT)GetStatusPar(pMqtt, 501 + index, 816);
    cJSON_AddNumberToObject(pRoot, "5", emustatus_union.emuStatus.fireFeedBack);
 
    emustatus_union.emuStatus.doorOpenStatus = (UINT)GetStatusPar(pMqtt, 601, 454); //门被打开  门限
    cJSON_AddNumberToObject(pRoot, "6", emustatus_union.emuStatus.doorOpenStatus);

    emustatus_union.emuStatus.spdFault = (UINT)GetStatusPar(pMqtt, 200, 242); //AC防雷器检测反馈  防雷器
    cJSON_AddNumberToObject(pRoot, "7", emustatus_union.emuStatus.spdFault);

    emustatus_union.emuStatus.ydnwdtFault = (UINT)GetStatusPar(pMqtt, 501 + index, 706); //电池柜水浸告警  水浸传感器
    cJSON_AddNumberToObject(pRoot, "8", emustatus_union.emuStatus.ydnwdtFault);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 748) == 1)
    {
        emustatus_union.emuStatus.bmsqfFeedback = 0; //电池组断路器未合闸  BMS断路器分励反馈
    }
    else
    {
        emustatus_union.emuStatus.bmsqfFeedback = 1;  
    }
    cJSON_AddNumberToObject(pRoot, "10", emustatus_union.emuStatus.bmsqfFeedback);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 553) == 2)
    {
        emustatus_union.emuStatus.bmsqfOut = 1; //高压下电故障  BMS断路器分励输出
    }
    cJSON_AddNumberToObject(pRoot, "11", emustatus_union.emuStatus.bmsqfFeedback);

    emustatus_union.emuStatus.remoteComNoLink = (UINT)GetStatusPar(pMqtt, 1, 30); //MQTT通信中断  远程通信无连接标志
    cJSON_AddNumberToObject(pRoot, "12", emustatus_union.emuStatus.remoteComNoLink);

    emustatus_union.emuStatus.liqcoolComFault = (UINT)GetStatusPar(pMqtt, 501 + index, 804); //空调通信中断 液冷故障
    cJSON_AddNumberToObject(pRoot, "13", emustatus_union.emuStatus.liqcoolComFault);

    emustatus_union.emuStatus.drierComFault = (UINT)GetStatusPar(pMqtt, 501 + index, 806); //除湿机通信中断 除湿器故障
    cJSON_AddNumberToObject(pRoot, "14", emustatus_union.emuStatus.drierComFault);

    emustatus_union.emuStatus.pcsComFault = (UINT)GetStatusPar(pMqtt, 200, 369);  //所有模块组通信中断 PCS通讯错误状态
    cJSON_AddNumberToObject(pRoot, "15", emustatus_union.emuStatus.pcsComFault);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 550) == 1)
    {
        emustatus_union.emuStatus.bmsComFault = 1; //BMS通信中断 BMS通讯错误状态
    }
    else
    {
        emustatus_union.emuStatus.bmsComFault = 0;
    }
    cJSON_AddNumberToObject(pRoot, "16", emustatus_union.emuStatus.bmsComFault);

    emustatus_union.emuStatus.airComFault = (UINT)GetStatusPar(pMqtt, 501 + index, 804); //空调通信中断  空调通讯错误状态
    cJSON_AddNumberToObject(pRoot, "17", emustatus_union.emuStatus.airComFault);

    VAR_VALUE varV;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R, 408, 401, &varV, 4);
    if(varV.emValue != 0)
    {
        emustatus_union.emuStatus.backsetDeviceComFault = 1; //逆流电表通信中断 逆流电表通讯错误状态
    }
    else
    {
        emustatus_union.emuStatus.backsetDeviceComFault = 0;
    }
    cJSON_AddNumberToObject(pRoot, "18", emustatus_union.emuStatus.backsetDeviceComFault);

    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R, 407, 401, &varV, 4);
    if(varV.emValue != 0)
    {
        emustatus_union.emuStatus.meterDeviceComFault = 1; //电表通信中断 计量电表通讯错误状态
    }
    else
    {
        emustatus_union.emuStatus.meterDeviceComFault = 0;
    }
    cJSON_AddNumberToObject(pRoot, "19", emustatus_union.emuStatus.meterDeviceComFault);
    
    emustatus_union.emuStatus.STOPFault =  (UINT)GetStatusPar(pMqtt, 200, 241);  //电源柜急停按钮被按下 急停
    cJSON_AddNumberToObject(pRoot, "20", emustatus_union.emuStatus.STOPFault);

    emustatus_union.emuStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "21", emustatus_union.emuStatus.Ts);
    
    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

BOOL CheckAirCondiFalut(MQTT_DATA_ROUGH *pMqtt, int idx, int start, int end)
{
    REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;
    BOOL Flag = FALSE;
    valGet.nGetSize = 4;
    int index = 0;

    for(index = start; index <= end; index++)
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R,
            951 + idx, index, &valGet, sizeof(PARAM_VAL_GET));
        if(valGet.val.emValue == 1) // Check if the value indicates a fault
        {
            Flag = TRUE;
            break;
        }
    }
    return Flag;
}

int prepare_LiqcoolStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;

    LIQCOOL_STATUS_UNION liqcool_union;
    ZERO_POBJS(&liqcool_union, 1);

    liqcool_union.liqcoolStatus.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", liqcool_union.liqcoolStatus.ArrNum);

    liqcool_union.liqcoolStatus.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", liqcool_union.liqcoolStatus.CluNum);

    liqcool_union.liqcoolStatus.Devno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "2", liqcool_union.liqcoolStatus.Devno);

    valGet.nGetSize = 4;
    if(pMqtt->emAirType == TONGFEI_AIR)
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1816, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm1_2 = valGet.val.emValue; //系统低压

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1828, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_8 = valGet.val.emValue; //液温过高
        
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1819, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_10 = valGet.val.emValue; //系统高压故障

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1822, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_12 = valGet.val.emValue; //高压报警锁定

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1812, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm3_12 = valGet.val.emValue; //回水压力过低

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1813, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm3_13 = valGet.val.emValue; //出口压力过高

        if(CheckAirCondiFalut(pMqtt, index, 3000, 3094) == TRUE) // Check for any faults in the range
        {
            liqcool_union.liqcoolStatus.Alarm_Mark = 1;
            liqcool_union.liqcoolStatus.Err1 = 1;
        }
        else
        {
            liqcool_union.liqcoolStatus.Alarm_Mark = 0;
            liqcool_union.liqcoolStatus.Err1 = 0;    
        }
            

/*         pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1816, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm_Mark = valGet.val.emValue;

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1816, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Comm_Err = valGet.val.emValue;

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1816, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Err1 = valGet.val.emValue; */
    }
    else if((pMqtt->emAirType == IMEU3_AIR) || (pMqtt->emAirType == INFYPOWER_AIR))
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1592, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm1_2 = valGet.val.emValue; //系统低压

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1568, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_8 = valGet.val.emValue; //液温过高
        
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1591, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_10 = valGet.val.emValue; //系统高压故障

/*         pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1822, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_12 = valGet.val.emValue; //高压报警锁定 */

/*         pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1812, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm3_12 = valGet.val.emValue; //回水压力过低 */

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1564, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm3_13 = valGet.val.emValue; //出口压力过高    

        if(CheckAirCondiFalut(pMqtt, index, 1501, 1539) == TRUE) // Check for any faults in the range
        {
            liqcool_union.liqcoolStatus.Alarm_Mark = 1;
            liqcool_union.liqcoolStatus.Err1 = 1;
        }
        else
        {
            liqcool_union.liqcoolStatus.Alarm_Mark = 0;
            liqcool_union.liqcoolStatus.Err1 = 0;    
        }    
    }
    else if(pMqtt->emAirType == XINMIAO_AIR)
    {
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3678, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm1_2 = valGet.val.emValue; //系统低压

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3673, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_8 = valGet.val.emValue; //液温过高
        
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3677, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_10 = valGet.val.emValue; //系统高压故障

/*         pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 1822, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm2_12 = valGet.val.emValue; //高压报警锁定 */

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3671, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm3_12 = valGet.val.emValue; //回水压力过低

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 951 + index, 3682, &valGet, sizeof(PARAM_VAL_GET));
        liqcool_union.liqcoolStatus.Alarm3_13 = valGet.val.emValue; //出口压力过高

        if(CheckAirCondiFalut(pMqtt, index, 3100, 3185) == TRUE) // Check for any faults in the range
        {
            liqcool_union.liqcoolStatus.Alarm_Mark = 1;
            liqcool_union.liqcoolStatus.Err1 = 1;
        }
        else
        {
            liqcool_union.liqcoolStatus.Alarm_Mark = 0;
            liqcool_union.liqcoolStatus.Err1 = 0;    
        }
    }
    cJSON_AddNumberToObject(pRoot, "14", liqcool_union.liqcoolStatus.Alarm1_2);
    cJSON_AddNumberToObject(pRoot, "28", liqcool_union.liqcoolStatus.Alarm2_8);
    cJSON_AddNumberToObject(pRoot, "29", liqcool_union.liqcoolStatus.Alarm2_10);
    cJSON_AddNumberToObject(pRoot, "30", liqcool_union.liqcoolStatus.Alarm2_12);
    cJSON_AddNumberToObject(pRoot, "36", liqcool_union.liqcoolStatus.Alarm3_12);
    cJSON_AddNumberToObject(pRoot, "37", liqcool_union.liqcoolStatus.Alarm3_13);
    cJSON_AddNumberToObject(pRoot, "40", liqcool_union.liqcoolStatus.Alarm_Mark);

    liqcool_union.liqcoolStatus.Comm_Err = (UINT)GetStatusPar(pMqtt, 501 + index, 804); //空调通信中断
    cJSON_AddNumberToObject(pRoot, "41", liqcool_union.liqcoolStatus.Comm_Err);
    cJSON_AddNumberToObject(pRoot, "42", liqcool_union.liqcoolStatus.Err1);

    liqcool_union.liqcoolStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    cJSON_AddNumberToObject(pRoot, "45", liqcool_union.liqcoolStatus.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 

    return 0;
}

int prepare_PcsStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;

    PCS_STATUS_UNION        pcs_status_union;
    ZERO_POBJS(&pcs_status_union, 1);
    
    pcs_status_union.pcsStatus.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", pcs_status_union.pcsStatus.ArrNum);

    pcs_status_union.pcsStatus.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", pcs_status_union.pcsStatus.CluNum);

    pcs_status_union.pcsStatus.Devno = (UINT)index; 
    cJSON_AddNumberToObject(pRoot, "2", pcs_status_union.pcsStatus.Devno);

    pcs_status_union.pcsStatus.Alarm_0 = (UINT)GetStatusPar(pMqtt, 251 + index, 1268); //A相逐波限流
    cJSON_AddNumberToObject(pRoot, "3", pcs_status_union.pcsStatus.Alarm_0);

    pcs_status_union.pcsStatus.Alarm_1 = (UINT)GetStatusPar(pMqtt, 251 + index, 1269); //B相逐波限流
    cJSON_AddNumberToObject(pRoot, "4", pcs_status_union.pcsStatus.Alarm_1);

    pcs_status_union.pcsStatus.Alarm_2 = (UINT)GetStatusPar(pMqtt, 251 + index, 1270); //C相逐波限流
    cJSON_AddNumberToObject(pRoot, "5", pcs_status_union.pcsStatus.Alarm_2);

    pcs_status_union.pcsStatus.Alarm_6 =  (UINT)GetStatusPar(pMqtt, 251 + index, 1268); //A相逐波限流
    cJSON_AddNumberToObject(pRoot, "9", pcs_status_union.pcsStatus.Alarm_6);

    pcs_status_union.pcsStatus.Alarm_7 = (UINT)GetStatusPar(pMqtt, 251 + index, 1269); //B相逐波限流
    cJSON_AddNumberToObject(pRoot, "10", pcs_status_union.pcsStatus.Alarm_7);

    pcs_status_union.pcsStatus.Alarm_8 = (UINT)GetStatusPar(pMqtt, 251 + index, 1270); //C相逐波限流
    cJSON_AddNumberToObject(pRoot, "11", pcs_status_union.pcsStatus.Alarm_8);
    
    pcs_status_union.pcsStatus.Alarm_10 = (UINT)GetStatusPar(pMqtt, 251 + index, 1221); //过温故障
    cJSON_AddNumberToObject(pRoot, "13", pcs_status_union.pcsStatus.Alarm_10);

    pcs_status_union.pcsStatus.Alarm_11 = (UINT)GetStatusPar(pMqtt, 251 + index, 1268); //A相逐波限流
    cJSON_AddNumberToObject(pRoot, "14", pcs_status_union.pcsStatus.Alarm_11);

    pcs_status_union.pcsStatus.Alarm_13 = (UINT)GetStatusPar(pMqtt, 251 + index, 1269); //B相逐波限流
    cJSON_AddNumberToObject(pRoot, "16", pcs_status_union.pcsStatus.Alarm_13);

    pcs_status_union.pcsStatus.Alarm_15 = (UINT)GetStatusPar(pMqtt, 251 + index, 1270); //C相逐波限流
    cJSON_AddNumberToObject(pRoot, "18", pcs_status_union.pcsStatus.Alarm_15);

    pcs_status_union.pcsStatus.Alarm_19 = (UINT)GetStatusPar(pMqtt, 251 + index, 1233); //电网过压
    cJSON_AddNumberToObject(pRoot, "22", pcs_status_union.pcsStatus.Alarm_19);

    pcs_status_union.pcsStatus.Alarm_20 = (UINT)GetStatusPar(pMqtt, 251 + index, 1234); //电网欠压
    cJSON_AddNumberToObject(pRoot, "23", pcs_status_union.pcsStatus.Alarm_20);

    pcs_status_union.pcsStatus.Alarm_21 = (UINT)GetStatusPar(pMqtt, 251 + index, 1235); //电网过频
    cJSON_AddNumberToObject(pRoot, "24", pcs_status_union.pcsStatus.Alarm_21);

    pcs_status_union.pcsStatus.Alarm_22 = (UINT)GetStatusPar(pMqtt, 251 + index, 1236); //电网欠频
    cJSON_AddNumberToObject(pRoot, "25", pcs_status_union.pcsStatus.Alarm_22);

    pcs_status_union.pcsStatus.Alarm_24 = (UINT)GetStatusPar(pMqtt, 251 + index, 1237); //电网缺相
    cJSON_AddNumberToObject(pRoot, "27", pcs_status_union.pcsStatus.Alarm_24);

    pcs_status_union.pcsStatus.Alarm_27 = (UINT)GetStatusPar(pMqtt, 251 + index, 1257); //DC过压
    cJSON_AddNumberToObject(pRoot, "30", pcs_status_union.pcsStatus.Alarm_27);

    pcs_status_union.pcsStatus.Alarm_28 = (UINT)GetStatusPar(pMqtt, 251 + index, 1258); //DC欠压
    cJSON_AddNumberToObject(pRoot, "31", pcs_status_union.pcsStatus.Alarm_28);

    pcs_status_union.pcsStatus.Alarm_31 = (UINT)GetStatusPar(pMqtt, 251 + index, 1260); //DC过流
    cJSON_AddNumberToObject(pRoot, "34", pcs_status_union.pcsStatus.Alarm_31);

    pcs_status_union.pcsStatus.Alarm_32 = (UINT)GetStatusPar(pMqtt, 251 + index, 1260); //DC过流
    cJSON_AddNumberToObject(pRoot, "35", pcs_status_union.pcsStatus.Alarm_32);

    pcs_status_union.pcsStatus.Alarm_33 = (UINT)GetStatusPar(pMqtt, 251 + index, 1248); //孤岛保护
    cJSON_AddNumberToObject(pRoot, "36", pcs_status_union.pcsStatus.Alarm_33);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 550) == 1)
    {
        pcs_status_union.pcsStatus.Alarm_48 = 1; //BMS通信中断
    }
    else
    {
        pcs_status_union.pcsStatus.Alarm_48 = 0;
    }
    cJSON_AddNumberToObject(pRoot, "51", pcs_status_union.pcsStatus.Alarm_48);

    pcs_status_union.pcsStatus.Alarm_49 = (UINT)GetStatusPar(pMqtt, 501 + index, 684); //从控温度异常
    cJSON_AddNumberToObject(pRoot, "52", pcs_status_union.pcsStatus.Alarm_49);

    pcs_status_union.pcsStatus.Alarm_50 =  (UINT)GetStatusPar(pMqtt, 501 + index, 719); //主控内部自行保护
    cJSON_AddNumberToObject(pRoot, "53", pcs_status_union.pcsStatus.Alarm_50);

    if((UINT)GetStatusPar(pMqtt, 501 + index, 552) == 3)
    {
        pcs_status_union.pcsStatus.Alarm_51 = 1; //电池告警
    }
    cJSON_AddNumberToObject(pRoot, "54", pcs_status_union.pcsStatus.Alarm_51);

    pcs_status_union.pcsStatus.Alarm_53 = (UINT)GetStatusPar(pMqtt, 200, 369); //所有模块组通信中断
    cJSON_AddNumberToObject(pRoot, "56", pcs_status_union.pcsStatus.Alarm_53);

    pcs_status_union.pcsStatus.Alarm_54 = (UINT)GetStatusPar(pMqtt, 200, 241); //电源柜急停按钮被按下
    cJSON_AddNumberToObject(pRoot, "57", pcs_status_union.pcsStatus.Alarm_54);

    pcs_status_union.pcsStatus.Comm_Err = (UINT)GetStatusPar(pMqtt, 200, 369); //所有模块组通信中断
    cJSON_AddNumberToObject(pRoot, "58", pcs_status_union.pcsStatus.Comm_Err);

    pcs_status_union.pcsStatus.Err1 = (UINT)GetStatusPar(pMqtt, 251 + index, 1232); //系统故障(心跳CPLD继电器软起辅源)
    cJSON_AddNumberToObject(pRoot, "59", pcs_status_union.pcsStatus.Err1);

    pcs_status_union.pcsStatus.Err2 = 0;
    cJSON_AddNumberToObject(pRoot, "60", pcs_status_union.pcsStatus.Err2);

    pcs_status_union.pcsStatus.Err3 = 0;
    cJSON_AddNumberToObject(pRoot, "61", pcs_status_union.pcsStatus.Err3);

    pcs_status_union.pcsStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS; //时间戳，秒级
    cJSON_AddNumberToObject(pRoot, "62", pcs_status_union.pcsStatus.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_FireStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;
    FIRE_STATUS_UNION       fire_status_union;
    ZERO_POBJS(&fire_status_union, 1);

    fire_status_union.fireStatus.ArrNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "0", fire_status_union.fireStatus.ArrNum);

    fire_status_union.fireStatus.CluNum = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "1", fire_status_union.fireStatus.CluNum);

    fire_status_union.fireStatus.Devno = (UINT)index;
    cJSON_AddNumberToObject(pRoot, "2", fire_status_union.fireStatus.Devno);

    fire_status_union.fireStatus.Comm_Err = (UINT)GetStatusPar(pMqtt, 501 + index, 807); //可燃气体传感器通信中断
    cJSON_AddNumberToObject(pRoot, "3", fire_status_union.fireStatus.Comm_Err);

    fire_status_union.fireStatus.Err1 =  (UINT)GetStatusPar(pMqtt, 501 + index, 814) | (UINT)GetStatusPar(pMqtt, 501 + index, 815) | (UINT)GetStatusPar(pMqtt, 501 + index, 816);
    cJSON_AddNumberToObject(pRoot, "4", fire_status_union.fireStatus.Err1); //消防1级告警  消防2级告警 消防3级告警 消防反馈

    fire_status_union.fireStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    cJSON_AddNumberToObject(pRoot, "19", fire_status_union.fireStatus.Ts); //时间戳，秒级

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_DrierStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;
    DRIER_STATUS_UNION       drier_union;
    ZERO_POBJS(&drier_union, 1);

    drier_union.drierStatus.ArrNum = index;
    cJSON_AddNumberToObject(pRoot, "0", drier_union.drierStatus.ArrNum);

    drier_union.drierStatus.CluNum = index;
    cJSON_AddNumberToObject(pRoot, "1", drier_union.drierStatus.CluNum);

    drier_union.drierStatus.Devno = index;
    cJSON_AddNumberToObject(pRoot, "2", drier_union.drierStatus.Devno);

    drier_union.drierStatus.Comm_Err = (UINT)GetStatusPar(pMqtt, 501 + index, 806);
    cJSON_AddNumberToObject(pRoot, "3", drier_union.drierStatus.Comm_Err); //除湿机通信中断

    drier_union.drierStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    cJSON_AddNumberToObject(pRoot, "7", drier_union.drierStatus.Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}

int prepare_MeterStatus(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
	cJSON *pRoot = cJSON_CreateObject(); 
    
    PARAM_VAL_GET 		valGet;
    ERRCODE_DAI 		err;
    METER_STATUS_UNION       meter_union;
    ZERO_POBJS(&meter_union, 1);

    meter_union.meterStatus.ArrNum = index;
    cJSON_AddNumberToObject(pRoot, "0", meter_union.meterStatus.ArrNum);

    meter_union.meterStatus.CluNum = index;
    cJSON_AddNumberToObject(pRoot, "1", meter_union.meterStatus.CluNum);

    meter_union.meterStatus.Devno = index;
    cJSON_AddNumberToObject(pRoot, "2", meter_union.meterStatus.Devno);

    VAR_VALUE varV;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R, 407 + index, 401, &varV, 4);
    if(varV.emValue != 0)
    {
        meter_union.meterStatus.Comm_Err = 1; //电表通信中断 计量电表通讯错误状态
    }
    else
    {
        meter_union.meterStatus.Comm_Err = 0;
    }
    cJSON_AddNumberToObject(pRoot, "3", meter_union.meterStatus.Comm_Err);

    meter_union.meterStatus.Err1 = 0;
    cJSON_AddNumberToObject(pRoot, "4", meter_union.meterStatus.Err1); //电表故障

    meter_union.meterStatus.Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    cJSON_AddNumberToObject(pRoot, "7", meter_union.meterStatus.Ts); //时间戳，秒级

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE); 
    return 0;
}


//上传参数设置
int prepare_Cmd_PowerCmd(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    cJSON *pRoot = cJSON_CreateObject();

    time_t Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;

    char key[10];
    int idx = 0;
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, pMqtt->emControlMode);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE);

    return 0;
}

int date_to_HMS(time_t tm_date)
{
    int date = 0;
    struct tm gmTime_date;
    gmtime_r(&tm_date, &gmTime_date);

    int hour = gmTime_date.tm_hour;
    int min = gmTime_date.tm_min;
    int sec = gmTime_date.tm_sec;

    date = hour * 3600 + min * 60 + sec;
    
    return date;
}

int date_to_YMD(time_t tm_date)
{
    int date = 0;
    struct tm gmTime_date;
    gmtime_r(&tm_date, &gmTime_date); 

    time_t tmNow = _NOW_;
	struct tm gmTime_Now;
	gmtime_r(&tmNow, &gmTime_Now);
    gmTime_date.tm_year = gmTime_Now.tm_year;
    gmTime_date.tm_mon = gmTime_Now.tm_mon;
    gmTime_date.tm_mday = gmTime_Now.tm_mday;
    //printf("day = [%d] month = [%d] year = [%d]\n", gmTime_date.tm_mday, gmTime_date.tm_mon, gmTime_date.tm_year);
    gmTime_date.tm_hour = 0;
    gmTime_date.tm_min = 0;
    gmTime_date.tm_sec = 0;

    date = (UINT)mktime(&gmTime_date) - BEIJING_TIMEZONE_OFFSET_HOURS;
    return date;
}

BOOL checkDay(time_t tm_start, UINT tmEdn, BOOL flag)
{
    time_t tmNow = _NOW_;
    struct tm gmTime_Now;
    gmtime_r(&tmNow, &gmTime_Now);
    struct tm gmTime_start;
    gmtime_r(&tm_start, &gmTime_start);
    if(flag == FALSE)
    {//远程调度，核对管理计划开始时间和结束时间，是否在当天范围内
        gmTime_start.tm_year = gmTime_Now.tm_year;
        gmTime_start.tm_mon  = gmTime_Now.tm_mon;
        gmTime_start.tm_mday = gmTime_Now.tm_mday;

        time_t tmNewStart = mktime(&gmTime_start);	
        //printf("tmNewStart = [%d] tmEdn = [%d]\n", (int)tmNewStart, tmEdn);
        if(tmNewStart <= tmEdn)
        {
            return TRUE; // Current time is within the same day
        }
    }
    else
    {//本地调度，核对月份是否相同
        if(gmTime_start.tm_mon == tmEdn)
        {
            //printf("checkDay: tm_start = [%ld] tmEdn = [%d]\n", tm_start, tmEdn);
            return TRUE;
        }        
    }

    return FALSE;
}

BOOL CheckHourMinSec(time_t start, time_t  end)
{
    struct tm gmTime_start, gmTime_end;
    gmtime_r(&start, &gmTime_start);
    gmtime_r(&end, &gmTime_end);
    time_t tmNow = _NOW_;
    struct tm gmTime_Now;
    gmtime_r(&tmNow, &gmTime_Now);

    gmTime_start.tm_year = gmTime_Now.tm_year;
    gmTime_start.tm_mon = gmTime_Now.tm_mon;
    gmTime_start.tm_mday = gmTime_Now.tm_mday;

    gmTime_end.tm_year = gmTime_Now.tm_year;
    gmTime_end.tm_mon = gmTime_Now.tm_mon;
    gmTime_end.tm_mday = gmTime_Now.tm_mday;

    time_t startTime = mktime(&gmTime_start);
    time_t endTime = mktime(&gmTime_end);

    if((startTime >= tmNow) && (tmNow <= endTime))
    {
        return TRUE; // Current time is within the range
    }
    return FALSE; // Current time is not within the range
}

//上传功率调度，当天24小时内会运行的本地调度策略
int prepare_Cmd_DispatchPower(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    PARAM_VAL_GET valGet;
    ERRCODE_DAI err = -1;

    cJSON *pRoot = cJSON_CreateObject(); 
    A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
    ZERO_POBJS(&blobEmsCfg, 1);
    blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
    valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
    valGet.val.blobValue = (BYTE*)&blobEmsCfg;    

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
                200, 201, &valGet, sizeof(PARAM_VAL_GET));        
    int n = 0;
    UINT bMon = -1;
    UINT Gap = 999;
    if(blobEmsCfg.nEMSMgmt_Ver != 2102)
    {//本地策略需要查询，月份
        for(n = 0; n < blobEmsCfg.nTotal_EmsLmt_Sectors; ++n)
        {//搜寻所有段数，找到最近的月份，用于判断上传功率调度
            time_t tmStart = blobEmsCfg.emsLmtSectors[n].tmStart;
            struct tm gmTime_start;
            gmtime_r(&tmStart, &gmTime_start);
            time_t tmNow = _NOW_;
            struct tm gmTime_Now;
            gmtime_r(&tmNow, &gmTime_Now);
            if((gmTime_start.tm_mon <= gmTime_Now.tm_mon) && ((gmTime_Now.tm_mon - gmTime_start.tm_mon) < Gap))
            {//找小于等于当前月份的段数 
                bMon = gmTime_start.tm_mon;
                Gap = MIN(Gap, ABS(gmTime_Now.tm_mon - gmTime_start.tm_mon));
            }
        }
    }

    cJSON	*p1_Array = cJSON_CreateArray();
    cJSON	*p2_Array = cJSON_CreateArray();
    cJSON	*p3_Array = cJSON_CreateArray();
    cJSON	*p4_Array = cJSON_CreateArray();
    cJSON	*p5_Array = cJSON_CreateArray();
    cJSON	*p6_Array = cJSON_CreateArray();
    cJSON	*p7_Array = cJSON_CreateArray();

    //printf("blobEmsCfg.nEMSMgmt_Ver = [%d] nTotal_EmsLmt_Sectors = [%d]\n", blobEmsCfg.nEMSMgmt_Ver, blobEmsCfg.nTotal_EmsLmt_Sectors);

    for(n = 0; n < blobEmsCfg.nTotal_EmsLmt_Sectors; ++n)
    {//检查是否在当天的调度时间段内
        if(blobEmsCfg.nEMSMgmt_Ver == 2102)
        {//如果是远程控制模式，则检查是否在当天的调度时间段内
            if(checkDay(blobEmsCfg.emsLmtSectors[n].tmStart, blobEmsCfg.emsLmtSectors_ext[n].tmEndTime, FALSE) == FALSE)
            {
                continue;  //如果不在当天的调度时间段内，则跳过
            }
        }
        else
        {//如果是本地控制模式，核对月份，并上传
            if(checkDay(blobEmsCfg.emsLmtSectors[n].tmStart, bMon, TRUE) == FALSE)
            {
                continue;  
            }
        }

        time_t  start = blobEmsCfg.emsLmtSectors[n].tmStart;
        struct tm gmTime_start;
        gmtime_r(&start, &gmTime_start);
        //printf("nInterval = [%d]\n", blobEmsCfg.emsLmtSectors[n].nInterval);
        if(blobEmsCfg.emsLmtSectors[n].nInterval > 86400)
        {//间隔时间不会超过24小时
            blobEmsCfg.emsLmtSectors[n].nInterval = 86400 - gmTime_start.tm_hour * 3600 - gmTime_start.tm_min * 60 - gmTime_start.tm_sec;
            //printf("nInterval = [%d]\n", blobEmsCfg.emsLmtSectors[n].nInterval);
        }
        time_t  end = blobEmsCfg.emsLmtSectors[n].tmStart + blobEmsCfg.emsLmtSectors[n].nInterval - 1;

        int startDate = date_to_YMD(start);
        int endDate = date_to_YMD(end);
        int startTime = date_to_HMS(start);
        int endTime = date_to_HMS(end);
        
        float ActvPwrLmt = 0;
        ActvPwrLmt = (float)blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt;
        if(blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING)
        {
            ActvPwrLmt = -(float)blobEmsCfg.emsLmtSectors[n].nBattGrpPwrLmt;
        }
        int switch_OnOff = 1;
        if(BATT_WORKMODE_IDLE == blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode)
        {
            switch_OnOff = 0;
        }
        cJSON_AddItemToArray(p1_Array, cJSON_CreateNumber(startDate)); 
        cJSON_AddItemToArray(p2_Array, cJSON_CreateNumber(endDate)); 
        cJSON_AddItemToArray(p3_Array, cJSON_CreateNumber(startTime)); 
        cJSON_AddItemToArray(p4_Array, cJSON_CreateNumber(endTime)); 
        cJSON_AddItemToArray(p5_Array, cJSON_CreateFloat(ActvPwrLmt, 1)); 
        cJSON_AddItemToArray(p6_Array, cJSON_CreateFloat(blobEmsCfg.emsLmtSectors[n].nReActvPwrLmt, 1)); 
        cJSON_AddItemToArray(p7_Array, cJSON_CreateNumber(switch_OnOff));   
    }

    time_t Ts = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;

    char key[10];
    int idx = 0;
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p1_Array);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p2_Array);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p3_Array);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p4_Array);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p5_Array);
    idx++;
    
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p6_Array);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddItemToObject(pRoot, key, p7_Array);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, Ts);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE);

    return 0;
}

int prepare_Cmd_powerLimit(void* pArg, int index)
{
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    PARAM_VAL_GET valGet, valGet2;
    ERRCODE_DAI err = -1;

    cJSON *pRoot = cJSON_CreateObject(); 
    A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
    ZERO_POBJS(&blobEmsCfg, 1);
    blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
    valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
    valGet.val.blobValue = (BYTE*)&blobEmsCfg; 

    UINT chg_SOC = 100;
    UINT disc_SOC = 5;
    int soc_stop_enable = 1;
    valGet2.nGetSize = 4;

    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
            200, 201, &valGet, sizeof(PARAM_VAL_GET));   

    if(blobEmsCfg.nEMSMgmt_Ver == 2102)
    {
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 91, &valGet2, sizeof(PARAM_VAL_GET));
        chg_SOC = (UINT)valGet2.val.ulValue; // 充电SOC
        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 92, &valGet2, sizeof(PARAM_VAL_GET));
        disc_SOC = (UINT)valGet2.val.ulValue; // 放电SOC       
    }
    else
    {
        int n = 0;
        UINT bMon = -1;
        UINT Gap = 999;
        for(n = 0; n < blobEmsCfg.nTotal_EmsLmt_Sectors; ++n)
        {//搜寻所有段数，找到最近的月份，用于判断上传功率调度
            time_t tmStart = blobEmsCfg.emsLmtSectors[n].tmStart;
            struct tm gmTime_start;
            gmtime_r(&tmStart, &gmTime_start);
            time_t tmNow = _NOW_;
            struct tm gmTime_Now;
            gmtime_r(&tmNow, &gmTime_Now);
            if((gmTime_start.tm_mon <= gmTime_Now.tm_mon) && ((gmTime_Now.tm_mon - gmTime_start.tm_mon) < Gap))
            {//找小于等于当前月份的段数 
                bMon = gmTime_start.tm_mon;
                Gap = MIN(Gap, ABS(gmTime_Now.tm_mon - gmTime_start.tm_mon));
            }
        }
        for(n = 0; n  < blobEmsCfg.nTotal_EmsLmt_Sectors; n++)
        {
            if(checkDay(blobEmsCfg.emsLmtSectors[n].tmStart, bMon, TRUE) == TRUE)
            {
                if(blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING)
                {
                    chg_SOC = blobEmsCfg.emsLmtSectors[n].nStopSOC;
                }
                else if(blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
                {
                    disc_SOC = blobEmsCfg.emsLmtSectors[n].nStopSOC;
                }
            }
        }
    }

    time_t Ts1 = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;

    char key[10];
    int idx = 0;
    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, soc_stop_enable);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, chg_SOC);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, disc_SOC);
    idx++;

    snprintf(key, sizeof(key), "%d", idx);
    cJSON_AddNumberToObject(pRoot, key, Ts1);

    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE);

    return 0;
}

int EMU_PowerCmd(void* pArg, cJSON *json)
{

    char *szData = 0;
	szData = cJSON_Print(json);
    LOG_INFO("GH_MQTT Publish_MqttMessage szSubTopic=%s, Data='%s'",
    "PowerCmd",
    szData);
    free(szData);

    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
    REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    ERRCODE_DAI err = -1;

    cJSON *mode = cJSON_GetObjectItem(json, "0");
    cJSON *Ts = cJSON_GetObjectItem(json, "1");

    if((mode == NULL) || (Ts == NULL))
    {
        //printf("111 cmd Invalid JSON Type\n");
        return -1;
    }
    else if((mode->type != cJSON_Number) || (Ts->type != cJSON_Number))
    {
        //printf("222 cmd Invalid JSON Type\n");
        return -1;
    }
    time_t Now_time = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    //printf("Now_time = %d, Ts->valueint = %d\n", Now_time, Ts->valueint);

    if(ABS(Now_time - Ts->valueint) > 15)
    {
        pMqtt->bDisPower = TRUE;
        pMqtt->tSendDisPower = _NOW_;
        //printf("cmd Ts is not valid, please check\n");
        return -1;
    }

    cJSON *pRoot = cJSON_Duplicate(json, 1);
    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE);
    Publish_GH_MqttMessage(pMqtt, GH_CMD_TYPE_POWERCMD, pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERCMD].szPubTopic);    
    
    pMqtt->emControlMode = mode->valueint;
    VAR_VALUE SetvarV;
	SetvarV.emValue = pMqtt->emControlMode; 
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 89, &SetvarV, 4, 3000);

    return 0;
}

BOOL CheckEmsPlanTime(cJSON *startDate, cJSON *startTime,  UINT idx)
{
    BOOL Flag = TRUE;
    int j = 0;
    cJSON *startdate_val = cJSON_GetArrayItem(startDate, idx);
    cJSON *starttime_val = cJSON_GetArrayItem(startTime, idx);
    for(j = 0; j < idx; ++j)
    {
        //printf("j = [%d] idx = [%d]\n", j, idx);
        cJSON *val1 = cJSON_GetArrayItem(startDate, j);
        cJSON *val2 = cJSON_GetArrayItem(startTime, j);
        //printf("startdate_val = [%d] starttime_val = [%d] val1 = [%d] val2 = [%d]\n", \
        startdate_val->valueint, starttime_val->valueint, val1->valueint, val2->valueint);
        if((startdate_val->valueint + starttime_val->valueint) == (val1->valueint + val2->valueint))
        {
            Flag = FALSE; //重复的时间段
            break;
        }
    }
    return Flag;
}

int EMU_dispatchPower(void* pArg, cJSON *json)
{   

    char *szData = 0;
	szData = cJSON_Print(json);
    LOG_INFO("GH_MQTT Publish_MqttMessage szSubTopic=%s, Data='%s'",
    "dispatchPower",
    szData);
    free(szData);
    
    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    PARAM_VAL_GET valGet, valGet2;
    ERRCODE_DAI err = -1;

    A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
	ZERO_POBJS(&blobEmsCfg, 1);
	blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;         
    blobEmsCfg.nEMSMgmt_Ver = 2102;
    blobEmsCfg.nTotal_EmsLmt_Sectors = 0;

    cJSON *startDate = cJSON_GetObjectItem(json, "0");
    cJSON *endDate  = cJSON_GetObjectItem(json, "1");
    cJSON *startTime = cJSON_GetObjectItem(json, "2");
    cJSON *endTime = cJSON_GetObjectItem(json, "3");
    cJSON *actPower = cJSON_GetObjectItem(json, "4");
    cJSON *reaPower = cJSON_GetObjectItem(json, "5");
    cJSON *GHswitch = cJSON_GetObjectItem(json, "6");
    cJSON *Ts_val = cJSON_GetObjectItem(json, "7");

    if((startDate == NULL)  || (endDate == NULL)    || 
       (startTime == NULL) || (endTime == NULL)     || 
       (actPower == NULL) || (reaPower == NULL)     || 
       (GHswitch == NULL) || (Ts_val == NULL))
    {
        //printf("111  Invalid JSON data\n");
        return -1;
    }
    else if((startDate->type != cJSON_Array) ||(endDate->type != cJSON_Array) ||
            (startTime->type != cJSON_Array) ||(endTime->type != cJSON_Array) ||
            (actPower->type != cJSON_Array) ||(reaPower->type != cJSON_Array) ||
            (GHswitch->type != cJSON_Array) || (Ts_val->type != cJSON_Number)) 
    {
        //printf("222: Invalid JSON Type\n");
        return -1;
    } 

    //获取当前段数，并保存下一段管理计划
    time_t Now_time = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    if(ABS(Now_time - Ts_val->valueint) > 15)
    {
        pMqtt->bDisPower = TRUE;
        pMqtt->tSendDisPower = _NOW_;
        //printf("Ts is not valid, please check\n");
        return -1;
    }

    cJSON *pRoot = cJSON_Duplicate(json, 1);
    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE);
    Publish_GH_MqttMessage(pMqtt, GH_CMD_TYPE_DISPATCHPOWER, pMqtt->stGHmqttInfoList[GH_CMD_TYPE_DISPATCHPOWER].szPubTopic);

    if(pMqtt->emControlMode == LOCAL_CTRL_MODE)
    {
        return -1;
    }
    
    valGet2.nGetSize = 4;
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 91, &valGet2, sizeof(PARAM_VAL_GET));
    UINT chg_SOC = (UINT)valGet2.val.ulValue; // 充电SOC
    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 92, &valGet2, sizeof(PARAM_VAL_GET));
    UINT disc_SOC = (UINT)valGet2.val.ulValue; // 放电SOC

    int j = 0;
    //printf("cJSON_GetArraySize = [%d]\n", cJSON_GetArraySize(startDate));
    for(j = 0; j < cJSON_GetArraySize(startDate); ++j)
    {
        cJSON *startDate_val = cJSON_GetArrayItem(startDate, j);
        cJSON *endDate_val = cJSON_GetArrayItem(endDate, j);
        cJSON *startTime_val = cJSON_GetArrayItem(startTime, j);
        cJSON *endTime_val = cJSON_GetArrayItem(endTime, j);
        cJSON *actPower_val = cJSON_GetArrayItem(actPower, j);
        cJSON *reaPower_val = cJSON_GetArrayItem(reaPower, j);
        cJSON *GHswitch_val = cJSON_GetArrayItem(GHswitch, j);

        if((startDate_val == NULL) || (endDate_val == NULL) || 
        (startTime_val == NULL) || (endTime_val == NULL) || 
        (actPower_val == NULL)  || (reaPower_val == NULL) || 
        (GHswitch_val == NULL))
        {
            //printf("333 Invalid JSON data\n");
            return -1;
        }
        else if((startDate_val->type != cJSON_Number) || (endDate_val->type != cJSON_Number) ||
                (startTime_val->type != cJSON_Number) || (endTime_val->type != cJSON_Number) ||
                (GHswitch_val->type != cJSON_Number))
        {
            //printf("444: Invalid JSON Type\n");
            return -1;
        } 
    
        if((startDate_val->valueint <= 0) || (endDate_val->valueint <= 0) || \
        (startTime_val->valueint < 0) || (endTime_val->valueint < 0) || \
        (startTime_val->valueint >= 86400) || (endTime_val->valueint >= 86400) ||\
        ((startDate_val->valueint+startTime_val->valueint) > (endDate_val->valueint + endTime_val->valueint))|| \
        (CheckEmsPlanTime(startDate, startTime, j) == FALSE))
        { 
            //printf("fsdfdsfdsfdsfs\n");
            continue;
        }

        if((actPower_val->type != cJSON_Number)  || (reaPower_val->type != cJSON_Number))
        {//还要加大于额定功率
            continue;
        }

        UINT start_tm = startDate_val->valueint  + startTime_val->valueint + BEIJING_TIMEZONE_OFFSET_HOURS;

        if(GHswitch_val->valueint != 0 && GHswitch_val->valueint != 1)
        {
            GHswitch_val->valueint = 0;
        }
        
     /*    printf("startDate: %d, endDate: %d, startTime: %d, endTime: %d, actPower: %d, reaPower: %d, GHswitch: %d, Ts: %d\n",
            startDate_val->valueint, endDate_val->valueint, startTime_val->valueint,
            endTime_val->valueint, actPower_val->valueint, reaPower_val->valueint,
            GHswitch_val->valueint, Ts_val->valueint);  */

        blobEmsCfg.nTotal_EmsLmt_Sectors += 1;

        blobEmsCfg.emsLmtSectors_ext[j].tmEndTime = endDate_val->valueint + startTime_val->valueint + BEIJING_TIMEZONE_OFFSET_HOURS;
        
        blobEmsCfg.emsLmtSectors[j].tmStart = startDate_val->valueint  + startTime_val->valueint + BEIJING_TIMEZONE_OFFSET_HOURS; // Convert to seconds since epoch
        blobEmsCfg.emsLmtSectors[j].nInterval = endTime_val->valueint - startTime_val->valueint;
        //printf("22222222222222 tmStart = %d\n", blobEmsCfg.emsLmtSectors[j].tmStart);

       // printf("chg_SOC = [%d] disc_SOC = [%d]\n", chg_SOC, disc_SOC);
        if(GHswitch_val->valueint == 0)
        {
            blobEmsCfg.emsLmtSectors[j].emBatt_WorkMode = BATT_WORKMODE_IDLE;
        }
        else
        {
            blobEmsCfg.emsLmtSectors[j].emBatt_WorkMode = (actPower_val->valuedouble < 0) ? BATT_WORKMODE_FIRST_CHRGING: BATT_WORKMODE_FIRST_DISCHRG;
        }
       
        blobEmsCfg.emsLmtSectors[j].nStopSOC =  (actPower_val->valuedouble < 0) ? chg_SOC : disc_SOC;
        blobEmsCfg.emsLmtSectors[j].nActvPwrLmt = (actPower_val->valuedouble < 0) ? -(float)actPower_val->valuedouble : actPower_val->valuedouble; // 充电功率为负值
        blobEmsCfg.emsLmtSectors[j].nBattGrpPwrLmt = (actPower_val->valuedouble < 0) ? -(float)actPower_val->valuedouble : actPower_val->valuedouble; // 充电功率为负值 
        blobEmsCfg.emsLmtSectors[j].nReActvPwrLmt = reaPower_val->valuedouble;
    }

    A_BLOB_EMS_CONFIG_PLAN EmsPlan;
    ZERO_POBJS(&EmsPlan, 1);
    EmsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
    valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
    valGet.val.blobValue = (BYTE*)&EmsPlan; 
    err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
                200, 201, &valGet, sizeof(PARAM_VAL_GET)); 

    VAR_VALUE varV;
	varV.blobValue = &blobEmsCfg;           
    if(EmsPlan.nEMSMgmt_Ver == 2102)
    {
        err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
                200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);        
    }
    else
    {
        err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
                200, 224, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);        
    }

    if (err == ERRCODE_DAI_OK)
	{
		TRACEX("Info: EMS Config Plan Set Sucessfully\n");
	}
    return 0;
}

int EMU_powerLimit(void* pArg, cJSON *json)
{

    char *szData = 0;
	szData = cJSON_Print(json);
    LOG_INFO("GH_MQTT Publish_MqttMessage szSubTopic=%s, Data='%s'",
    "PowerLimit", szData);
    free(szData);

    MQTT_DATA_ROUGH *pMqtt = (MQTT_DATA_ROUGH*)pArg;
	REPORTER* pReporter = (REPORTER*)pMqtt->hReporter;
    PARAM_VAL_GET valGet;
    
    A_BLOB_EMS_CONFIG_PLAN blobEmsCfg;
	ZERO_POBJS(&blobEmsCfg, 1);
	blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	valGet.val.blobValue = (BYTE*)&blobEmsCfg;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));
    int paramter = 201;
    if(blobEmsCfg.nEMSMgmt_Ver != 2102)
    {
	    pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 224, &valGet, sizeof(PARAM_VAL_GET));
        paramter = 224;
    }

    cJSON *soc_stop_enable = cJSON_GetObjectItem(json, "0");
    cJSON *soc_stop_chg  = cJSON_GetObjectItem(json, "1");
    cJSON *soc_stop_dis = cJSON_GetObjectItem(json, "2");
    cJSON *Ts = cJSON_GetObjectItem(json, "3");

    if((soc_stop_enable == NULL) || (soc_stop_chg == NULL)|| \
        (soc_stop_dis == NULL) || (Ts == NULL))
    {
       // printf("Invalid JSON Type\n");
        return -1;
    }
    else if((soc_stop_enable->type != cJSON_Number) || (soc_stop_chg->type != cJSON_Number) || \
            (soc_stop_dis->type != cJSON_Number) || (Ts->type != cJSON_Number))
    {
        //printf("Invalid JSON Type\n");
        return -1;
    }

    time_t Now_time = _NOW_ - BEIJING_TIMEZONE_OFFSET_HOURS;
    if(ABS(Now_time - Ts->valueint) > 15)
    {
        pMqtt->bDisPower = TRUE;
        pMqtt->tSendDisPower = _NOW_;
        //printf("Ts is not valid, please check\n");
        return -1;
    }

    cJSON *pRoot = cJSON_Duplicate(json, 1);
    Send_GH_Call(pMqtt, pRoot, CALL_SRC_Normal, 0, FALSE);
    Publish_GH_MqttMessage(pMqtt, GH_CMD_TYPE_POWERLIMIT, pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERLIMIT].szPubTopic);

    if(pMqtt->emControlMode == LOCAL_CTRL_MODE)
    {
        return -1;
    }

    UINT SOC_chg = 100;
    UINT SOC_dis = 5;
    if((soc_stop_chg->valueint >= 0) && (soc_stop_chg->valueint <= 100) && 
       (soc_stop_dis->valueint >= 0) && (soc_stop_dis->valueint <= 100))   //&& (soc_stop_chg > soc_stop_dis)
    {
        SOC_chg = (UINT)(soc_stop_enable->valueint == 0) ? 100 : soc_stop_chg->valueint;
        SOC_dis = (UINT)(soc_stop_enable->valueint == 0) ? 5 :soc_stop_dis->valueint;
    }

    VAR_VALUE varV, varV1;
    varV.ulValue = (unsigned long)SOC_chg;
    pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 91, &varV, 4, 100);
    varV.ulValue = (unsigned long)SOC_dis;
    pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 92, &varV, 4, 100);

    //printf("soc_stop_chg = %d, soc_stop_dis = %d\n", SOC_chg, SOC_dis);

    int n = 0;
    for(n = 0; n  < blobEmsCfg.nTotal_EmsLmt_Sectors; n++)
    {
        if(blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_CHRGING)
        {
            blobEmsCfg.emsLmtSectors[n].nStopSOC = SOC_chg;
        }
        else if(blobEmsCfg.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
        {
            blobEmsCfg.emsLmtSectors[n].nStopSOC = SOC_dis;
        }
    }
    
	blobEmsCfg.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	varV1.blobValue = &blobEmsCfg;
	pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 200, paramter, &varV1, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);

    return 0;
}


#define INIT_GH_MQTT(_pMsg,_emMsgType, _szPubTopic, _emMsgSrc, _pfnMsgHandler) \
	do{\
	(_pMsg)->emMsgType = (_emMsgType); \
    snprintf((_pMsg)->szPubTopic, sizeof((_pMsg)->szPubTopic), "%s", (_szPubTopic)); \
	(_pMsg)->emMsgSrc = (_emMsgSrc); \
	(_pMsg)->pfnMsgHandler = (_pfnMsgHandler); \
	(_pMsg)++; \
	}while(0)

#define INIT_GH_SET(_pMsg, _emMegType, _szSubTopic, _pfnMsgHandlr)\
    do{\
    (_pMsg)->emSetCmdType = (_emMegType);\
     snprintf((_pMsg)->szSubTopic, sizeof((_pMsg)->szSubTopic), "%s", (_szSubTopic)); \
    (_pMsg)->pfnMsgHandler = (_pfnMsgHandlr);\
    (_pMsg)++; \
    }while(0)

void Init_GH_MqttMsgInfo(MQTT_DATA_ROUGH *pMqtt)
{
    GH_MQTT_INFO* p = &(pMqtt->stGHmqttInfoList[0]);
	//!!!!!!!!!Notice Macro MUST be in the right order because we use it for INDEXING!!!!!!!!!/**/
	INIT_GH_MQTT(p,	GH_MQTT_TYPE_ARR,			    "rtg/data/arr/0",			GH_MQTT_MSG_C2S,		prepare_ArrData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_CLU,			    "rtg/data/clu/0",			GH_MQTT_MSG_C2S,		prepare_CluData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_EMU,			    "rtg/data/emu",			    GH_MQTT_MSG_C2S,		prepare_EmuData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_PCS,			    "rtg/data/pcs/0",			GH_MQTT_MSG_C2S,		prepare_PCSData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_FIRE,			    "rtg/data/fire/0",			GH_MQTT_MSG_C2S,		prepare_FIREData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_LIQCOOL,			"rtg/data/liqcool/0",		GH_MQTT_MSG_C2S,		prepare_LiqcoolData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_DRIER,			    "rtg/data/drier/0",		    GH_MQTT_MSG_C2S,		prepare_DrierData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_METER,			    "rtg/data/meter",		    GH_MQTT_MSG_C2S,		prepare_MeterData);
    INIT_GH_MQTT(p,	GH_MQTT_TYPE_CELL,			    "rtg/data/cell/0",		    GH_MQTT_MSG_C2S,		prepare_CellData);

    INIT_GH_MQTT(p, GH_STATUS_TYPE_ARR,			    "rtg/status/arr/0",		    GH_MQTT_MSG_C2S,		prepare_ArrStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_CLU,			    "rtg/status/clu/0",		    GH_MQTT_MSG_C2S,		prepare_CluStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_EMU,			    "rtg/status/emu",		    GH_MQTT_MSG_C2S,		prepare_EmuStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_LIQCOOL,			"rtg/status/liqcool/0",		GH_MQTT_MSG_C2S,		prepare_LiqcoolStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_PCS,			    "rtg/status/pcs/0",		    GH_MQTT_MSG_C2S,		prepare_PcsStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_FIRE,			"rtg/status/fire/0",		GH_MQTT_MSG_C2S,		prepare_FireStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_DRIER,			"rtg/status/drier/0",		GH_MQTT_MSG_C2S,		prepare_DrierStatus);
    INIT_GH_MQTT(p, GH_STATUS_TYPE_METER,			"rtg/status/meter",		    GH_MQTT_MSG_C2S,		prepare_MeterStatus);

    INIT_GH_MQTT(p, GH_CMD_TYPE_POWERCMD,		"cmd/get/emu/powerCmd",         GH_MQTT_MSG_C2S,        prepare_Cmd_PowerCmd);
    INIT_GH_MQTT(p, GH_CMD_TYPE_DISPATCHPOWER,	"cmd/get/emu/dispatchPower",    GH_MQTT_MSG_C2S,        prepare_Cmd_DispatchPower);
    INIT_GH_MQTT(p, GH_CMD_TYPE_POWERLIMIT,		"cmd/get/emu/powerLimit",       GH_MQTT_MSG_C2S,        prepare_Cmd_powerLimit);
    
    GH_SETCMD_INFO* pd = &(pMqtt->stGHSetCmdIndoList[0]);
    INIT_GH_SET(pd, GH_SetCmdType_PowerCmd,      "cmd/set/emu/powerCmd",         EMU_PowerCmd);
    INIT_GH_SET(pd, GH_SetCmdType_DispatchPower, "cmd/set/emu/dispatchPower",    EMU_dispatchPower);
    INIT_GH_SET(pd, GH_SetCmdType_PowerLimit,    "cmd/set/emu/powerLimit",       EMU_powerLimit);


/*     snprintf(pMqtt->szGH_SubTopic[0], sizeof(pMqtt->szGH_SubTopic[0]), "%s", "cmd/set/emu/powerCmd");
    snprintf(pMqtt->szGH_SubTopic[1], sizeof(pMqtt->szGH_SubTopic[1]), "%s", "cmd/set/emu/dispatchPower");
    snprintf(pMqtt->szGH_SubTopic[2], sizeof(pMqtt->szGH_SubTopic[2]), "%s", "cmd/set/emu/powerLimit"); */
	
	return; 
}

int Publish_GH_MqttMessage(MQTT_DATA_ROUGH *pMqtt, GH_MQTT_TYPE emGHType, char *szTopic)
{       
    int rc = 0;
	//REPORTER* pReporter 			= (REPORTER*)pMqtt->hReporter;
    MQTTAsync client 				= pMqtt->client;
    MQTTAsync_responseOptions opts 	= MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg 		= MQTTAsync_message_initializer;

/*  	if(pMqtt->emMQTTVersion == VERSION_5)
	{
		opts.onSuccess5 	= GH_ExecOkCall_onSend;
    	opts.onFailure5 	= GH_ExecFailCall_onSend;
	}
	else
	{
    	opts.onSuccess 	= GH_ExecOkCall_onSend;
    	opts.onFailure 	= GH_ExecFailCall_onSend;
	}   */
    opts.context 	= client;

	if(pMqtt->bZlibCompress == TRUE)
	{
  		pubmsg.payload 		= pMqtt->szCompressSendBuff; 
		pubmsg.payloadlen 	= pMqtt->nCompressSendBuffLen;  
	}
	else
	{
		pubmsg.payload 		= pMqtt->szSendBuffer;           
   		pubmsg.payloadlen 	= pMqtt->nSendBuffLen; 
	}

    pubmsg.qos 				= QOS;
    pubmsg.retained 		= 0;
    //Callback_Deliveredtoken = 0;
	static int s_nErrCounter= 0;

    //printf("szTopic = %s\n", szTopic);

    char  szSubTopicBuffer[MQTT_TOPIC_SIZE];
	snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%s", pMqtt->szSelfTopic, szTopic);

    if ((rc = MQTTAsync_sendMessage(client, szSubTopicBuffer, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        //printf("Failed to start sendMessage, return code %d\n", rc);
        rc = -1;
        //Need ReConnect
        s_nErrCounter++;
		if(s_nErrCounter >= 5)
		{
			s_nErrCounter = 5;
			pMqtt->bIs_MqttConnOK 	= FALSE;
			pMqtt->emUserCommStatus = USER_COMM_DISCONECTED;
			pMqtt->emDisCnt_Reason	= STATCODE_COMM_CONECTFAILURE;			
		}
    }
	else
	{
		s_nErrCounter = 0;
	}

   /*  printf("mqtt MainRun=[%d] pubmsg.payload=[%d] pubmsg.payload=[%s] szPubTopic = [%s]\n", \
        rc, pubmsg.payloadlen, pubmsg.payload, pMqtt->stGHmqttInfoList[emGHType].szPubTopic); */

	if(s_nErrCounter < 5)
	{
		// DUMP_ESiOCPPMSG(pReporter, DUMP_TYPE_WRITE, pMqtt->szSendBuffer);
		LOG_INFO("GH_MQTT Publish_MqttMessage szTopic=%s, type=%d, Data='%s'",
			szTopic,
			DUMP_TYPE_WRITE,
			pMqtt->szSendBuffer);	
	}

    return rc;

}	



void Pub_GH_MsgToServer(MQTT_DATA_ROUGH *pMqtt)
{       
    
    time_t 			tmNow = time(NULL);
	char  szSubTopicBuffer[MQTT_TOPIC_SIZE];
    //printf("Pub_GH_MsgToServer\n");

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_ARR].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_ARR].pfnMsgHandler((void*)pMqtt, 0);
        Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_ARR, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_ARR].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_ARR].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CLU].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CLU].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_CLU, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CLU].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CLU].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_EMU].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_EMU].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_EMU, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_EMU].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_EMU].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_PCS].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_PCS].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_PCS,  pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_PCS].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_PCS].tm_LastSend = tmNow;	
    }
    
    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_FIRE].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_FIRE].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_FIRE, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_FIRE].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_FIRE].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_LIQCOOL].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_LIQCOOL].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_LIQCOOL, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_LIQCOOL].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_LIQCOOL].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_DRIER].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_DRIER].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_DRIER, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_DRIER].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_DRIER].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_METER].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {

        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_METER].pfnMsgHandler((void*)pMqtt, 0);
        snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%d", pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_METER].szPubTopic, 0);
        Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_METER, szSubTopicBuffer);

        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_METER].pfnMsgHandler((void*)pMqtt, 1);
        snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%d", pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_METER].szPubTopic, 1);
        Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_METER, szSubTopicBuffer);

        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_METER].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CELL].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {

        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CELL].pfnMsgHandler((void*)pMqtt, 0);
        Publish_GH_MqttMessage(pMqtt, GH_MQTT_TYPE_CELL, pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CELL].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_MQTT_TYPE_CELL].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_ARR].tm_LastSend) >= pMqtt->nSendTimeInterval) 
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_ARR].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_ARR, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_ARR].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_ARR].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_CLU].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_CLU].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_CLU, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_CLU].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_CLU].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_EMU].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_EMU].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_EMU, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_EMU].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_EMU].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_PCS].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_PCS].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_PCS, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_PCS].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_PCS].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_FIRE].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_FIRE].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_FIRE, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_FIRE].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_FIRE].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_DRIER].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_DRIER].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_DRIER, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_DRIER].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_DRIER].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_LIQCOOL].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_LIQCOOL].pfnMsgHandler((void*)pMqtt, 0);
	    Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_LIQCOOL, pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_LIQCOOL].szPubTopic);
        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_LIQCOOL].tm_LastSend = tmNow;	
    }

    if((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_METER].tm_LastSend) >= pMqtt->nSendTimeInterval)
    {

        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_METER].pfnMsgHandler((void*)pMqtt, 0);
        snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%d", pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_METER].szPubTopic, 0);
        Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_METER, szSubTopicBuffer);

        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_METER].pfnMsgHandler((void*)pMqtt, 1);
        snprintf(szSubTopicBuffer, MQTT_TOPIC_SIZE, "%s/%d", pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_METER].szPubTopic, 1);
        Publish_GH_MqttMessage(pMqtt, GH_STATUS_TYPE_METER, szSubTopicBuffer);

        pMqtt->stGHmqttInfoList[GH_STATUS_TYPE_METER].tm_LastSend = tmNow;	
    }

    if(((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_CMD_TYPE_DISPATCHPOWER].tm_LastSend) >= pMqtt->nRemoteControlTime) || ((pMqtt->bDisPower == TRUE)&&((_NOW_ - pMqtt->tSendDisPower) > 2)))
    {
        pMqtt->bDisPower = FALSE;
        if(pMqtt->stGHmqttInfoList[GH_CMD_TYPE_DISPATCHPOWER].pfnMsgHandler((void*)pMqtt, 0) == 0)
        {
            Publish_GH_MqttMessage(pMqtt, GH_CMD_TYPE_DISPATCHPOWER, pMqtt->stGHmqttInfoList[GH_CMD_TYPE_DISPATCHPOWER].szPubTopic);	
        }
        pMqtt->stGHmqttInfoList[GH_CMD_TYPE_DISPATCHPOWER].tm_LastSend = tmNow;	
    }

    if(((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERCMD].tm_LastSend) >= pMqtt->nRemoteControlTime))
    {
        pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERCMD].pfnMsgHandler((void*)pMqtt, 0);
        Publish_GH_MqttMessage(pMqtt, GH_CMD_TYPE_POWERCMD, pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERCMD].szPubTopic);	
        pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERCMD].tm_LastSend = tmNow;	
    }

    if(((UINT)(tmNow - pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERLIMIT].tm_LastSend) >= pMqtt->nRemoteControlTime))
    {
        pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERLIMIT].pfnMsgHandler((void*)pMqtt, 0);
        Publish_GH_MqttMessage(pMqtt, GH_CMD_TYPE_POWERLIMIT, pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERLIMIT].szPubTopic);	
        pMqtt->stGHmqttInfoList[GH_CMD_TYPE_POWERLIMIT].tm_LastSend = tmNow;	
    }
	return;
}