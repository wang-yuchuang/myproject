// #define _TRACE_PRINT_
#include "iec_104.h"

void printCP56Time2a(CP56Time2a time)
{
    TRACE("%02i:%02i:%02i %02i/%02i/%04i", CP56Time2a_getHour(time),
                             CP56Time2a_getMinute(time),
                             CP56Time2a_getSecond(time),
                             CP56Time2a_getDayOfMonth(time),
                             CP56Time2a_getMonth(time),
                             CP56Time2a_getYear(time) + 2000);
}

/*
系统对时
RCVD: 68 14 06 00 70 00 67 01 06 03 01 00 00 00 00 ff 25 04 0c 0d 01 17 
Process time sync command with time 12:04:09 13/01/2023
SEND: 68 14 74 00 08 00 67 01 07 03 01 00 00 00 00 00 26 04 0c 0d 01 17

主站发送
68 14 00 00 18 00 67 01 06 00 01 00 00 00 00 70 d0 1f 10 04 06 17 
起始字节=68 数据单元长度(APDU)=20 I格式帧 发送序号(NS)=0 接收序号(NR)=12 TI= 103 VSQ=01 SQ=0 INFONUM=1 COT= 06 T=0 PN=0 CAUSE =6 COA =1 C_CS_NA_1 
时钟同步命令      肯定认可    激活 
点号=0 360 毫秒 53 秒 31分 TIV=0时间有效 16小时 4日 6月 23年
从站发送
68 14 18 00 02 00 67 01 07 00 01 00 00 00 00 33 81 1f 10 04 06 17 
起始字节=68 数据单元长度(APDU)=20 I格式帧 发送序号(NS)=12 接收序号(NR)=1 TI= 103 VSQ=01 SQ=0 INFONUM=1 COT= 07 T=0 PN=0 CAUSE =7 COA =1 C_CS_NA_1 
时钟同步命令      肯定认可    激活确认 
点号=0 75 毫秒 33 秒 31分 TIV=0时间有效 16小时 4日 6月 23年

*/

time_t CP56Time2a_toSecTimestamp(CP56Time2a time)
{
    struct tm tmTime;

    tmTime.tm_year = CP56Time2a_getYear(time) + 100;
    tmTime.tm_mon = CP56Time2a_getMonth(time) - 1;
    tmTime.tm_mday = CP56Time2a_getDayOfMonth(time);

    tmTime.tm_hour = CP56Time2a_getHour(time);
    tmTime.tm_min = CP56Time2a_getMinute(time);
    tmTime.tm_sec = CP56Time2a_getSecond(time);

    TRACE("%d-%d-%d %d:%d:%d",\
         tmTime.tm_year,tmTime.tm_mon,tmTime.tm_mday,tmTime.tm_hour,tmTime.tm_min,tmTime.tm_sec);

    time_t timestamp = mktime(&tmTime);

    TRACE("timestamp=[%ld]\n", timestamp);

    return timestamp;
}


bool clockSyncHandler (void* parameter, IMasterConnection connection, CS101_ASDU asdu, CP56Time2a newTime)
{
    IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)parameter;
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

    TRACE("Process time sync command with time "); printCP56Time2a(newTime); printf("\n");

    time_t newSystemTimeInSec = CP56Time2a_toSecTimestamp(newTime);

    /* Set time for ACT_CON message */
    CP56Time2a_setFromMsTimestamp(newTime, Hal_getTimeInMs());

    /* update system time here */
    time_t tm_Now = _NOW_;

    TRACE("newSystemTimeInSec=[%ld] tm_Now=[%ld]\n", newSystemTimeInSec, tm_Now);

    if (pIEC104->lcCfg.lcType == LC_TYPE_1st)
    {
        if (ABS(newSystemTimeInSec - _NOW_) > 5) //sync RTC
        {
            VAR_VALUE varV;
            varV.dtValue = newSystemTimeInSec;
            pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 1, &varV, 4, 0);
        }
    }

    return true;
}

/*
服务器端使用InterrogationHandler回调函数来处理Interrogation请求。
根据QOI（询问限定符）的值，返回不同的信息对象。对于一个简单的系统，
仅处理 总查询 请求就足够了（QOI = 20）。
QOI值21-36用于询问组（1-16）。由从属实施者将信息对象分配给询问组。
************************************************************
服务器端处理研制的集群广域控制器的总召数据，采用M_ME_NC_1，短浮点遥测值，不带时标，
带品质描述，调用MeasuredValueShort_create创建io。
CS101_ASDU CS101_ASDU_create(
    CS101_AppLayerParameters parameters,（应用层参数）
    bool isSequence,（序列）
    CS101_CauseOfTransmission cot,（传送原因）
    int oa,（消息对象地址）
    int ca(应用服务数据单元公共地址),
    bool isTest, （是否测试帧）
    bool isNegative);
    //以下用M_ME_NC_1（MeasuredValueShort_create），创建4个数据 
    newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 0, 1, false, false);
    CS101_ASDU_addInformationObject(newAsdu, (InformationObject)MeasuredValueShort_create((MeasuredValueShort) io, 6000, 6010.1, IEC60870_QUALITY_GOOD));
    CS101_ASDU_addInformationObject(newAsdu, (InformationObject)MeasuredValueShort_create(( MeasuredValueShort) io, 6001, 6011.1, IEC60870_QUALITY_GOOD));
    CS101_ASDU_addInformationObject(newAsdu, (InformationObject)MeasuredValueShort_create(( MeasuredValueShort) io, 6001, 6012.1, IEC60870_QUALITY_GOOD));
    CS101_ASDU_addInformationObject(newAsdu, (InformationObject)MeasuredValueShort_create(( MeasuredValueShort) io, 6003, 6013.1, IEC60870_QUALITY_GOOD));
    
    //-发送8个数据-//
    InformationObject_destroy(io);
    IMasterConnection_sendASDU(connection, newAsdu);
    CS101_ASDU_destroy(newAsdu);

    //总召唤下发流程
    Step1.Client下发Type=100，COT=6，QOI=20；总召唤激活；
    Step2.Server答复Type=100，COT=7，QOI=20；响应激活
    Step3.Server答复Type=遥测、遥信，COT=5； 上送遥测、遥信
    Step4.Server答复Type=100，COT=7；   激活停止

*/
/*
SEND: 68 16 80 00 00 00 0b 83 01 00 01 00 6e 00 00 e0 0a 00 e1 0a 00 e2 0a 00 
SEND: 68 1c 82 00 00 00 07 83 01 00 01 00 65 00 00 aa aa aa aa 80 00 00 00 00 00 ff ff ff ff 00 
RCVD: 68 0e 00 00 84 00 64 01 06 03 01 00 00 00 00 14 
Received interrogation for group 20
SEND: 68 0e 84 00 02 00 64 01 07 03 01 00 00 00 00 14 
SEND: 68 1c 86 00 02 00 0b 03 14 00 01 00 64 00 00 ff ff 00 65 00 00 17 00 00 66 00 00 fc 08 00 
SEND: 68 12 88 00 02 00 01 02 14 00 01 00 68 00 00 01 69 00 00 00 
SEND: 68 15 8a 00 02 00 01 88 14 00 01 00 2c 01 00 01 00 01 00 01 00 01 00 
SEND: 68 12 8c 00 02 00 07 01 14 00 01 00 f4 01 00 aa aa 00 00 00 
SEND: 68 0e 8e 00 02 00 64 01 0a 03 01 00 00 00 00 14 
SEND: 68 16 aa 00 04 00 0b 83 01 00 01 00 6e 00 00 f5 0a 00 f6 0a 00 f7 0a 00 
SEND: 68 1c ac 00 04 00 07 83 01 00 01 00 65 00 00 aa aa aa aa 80 00 00 00 00 00 ff ff ff ff 00 
RCVD: 68 04 01 00 ae 00 
Connection closed (0xb2a25010)
*/
/*
bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi)
{
    printf("Received interrogation for group %i\n", qoi);

    if (qoi == 20) { // only handle station interrogation 总召唤限定词=20

        // 1、响应client的 CS101_COT_ACTIVATION 请求 
        CS101_AppLayerParameters alParams = IMasterConnection_getApplicationLayerParameters(connection);

        IMasterConnection_sendACT_CON(connection, asdu, false);

        // The CS101 specification only allows information objects without timestamp in GI responses 
        //! 2、发送ASDU, 其中COT设置为CS101_COT_INTERROGATED_BY_STATION，从站总召 
        //!  以下用M_ME_NB_1（MeasuredValueScaled_create），创建三个点对数据，分别为（100，-1）,(101,23),(102,2300) 
        //02版本后无组召唤概念，只有总召唤；CS101_COT_INTERROGATED_BY_STATION=20响应站总召唤
        CS101_ASDU newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                0, 1, false, false);

        InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, 100, -1, IEC60870_QUALITY_GOOD);

        CS101_ASDU_addInformationObject(newAsdu, io);

        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
            MeasuredValueScaled_create((MeasuredValueScaled) io, 101, 23, IEC60870_QUALITY_GOOD));

        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
            MeasuredValueScaled_create((MeasuredValueScaled) io, 102, 2300, IEC60870_QUALITY_GOOD));

        //! 发送 三个M_ME_NB_1数据 
        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        //!  以下用M_SP_NA_1（SinglePointInformation_create），创建2个数据，分别为（104，true）,(104,false) 
        newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                    0, 1, false, false);

        io = (InformationObject) SinglePointInformation_create(NULL, 104, true, IEC60870_QUALITY_GOOD);

        CS101_ASDU_addInformationObject(newAsdu, io);

        CS101_ASDU_addInformationObject(newAsdu, (InformationObject)
            SinglePointInformation_create((SinglePointInformation) io, 105, false, IEC60870_QUALITY_GOOD));

        //! 发送两个M_SP_NA_1数据 
        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        //!  以下用M_SP_NA_1（SinglePointInformation_create），创建8个数据 
        newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
                0, 1, false, false);

        CS101_ASDU_addInformationObject(newAsdu, io = (InformationObject) SinglePointInformation_create(NULL, 300, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 301, false, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 302, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 303, false, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 304, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 305, false, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 306, true, IEC60870_QUALITY_GOOD));
        CS101_ASDU_addInformationObject(newAsdu, (InformationObject) SinglePointInformation_create((SinglePointInformation) io, 307, false, IEC60870_QUALITY_GOOD));

        //!  发送8个数据 
        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        //! 增加其他响应数据
        newAsdu = CS101_ASDU_create(alParams, false, CS101_COT_INTERROGATED_BY_STATION,
                        0, 1, false, false);

        io = (InformationObject) BitString32_create(NULL, 500, 0xaaaa);

        CS101_ASDU_addInformationObject(newAsdu, io);

        InformationObject_destroy(io);

        IMasterConnection_sendASDU(connection, newAsdu);

        CS101_ASDU_destroy(newAsdu);

        //! 结束响应必须发送 CS101_COT_ACTIVATION_TERMINATION 
        IMasterConnection_sendACT_TERM(connection, asdu);
    }
    else {
        IMasterConnection_sendACT_CON(connection, asdu, true);
    }

    return true;
}*/ 

/*********************************************************************************
*  
*  FUNCTION NAME :  OnHandle_YX_Station() 总召遥信处理
*  CREATOR       : John
*  CREATED DATE  : 2023-6-2 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void OnHandle_YX_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{   
    int n = 0;

    CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
        0x01, 1, false, false);

    IEC104_CONFIG_SIGMAP *pYXItem;

	for (n = 0; n < pIEC104->nYXNum; n++)
	{
		pYXItem = pIEC104->pYXEle + n;
		//printf("n=[%d] pIEC104->nYXNum=[%d]\n", n, pIEC104->nYXNum);
		if(pYXItem == NULL)
		{
			printf("pYXItem == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYXItem->ptrValue);
 		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYXItem->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYXItem->nIOA, tempVal, pYXItem->emQuality);
		
		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

    CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YX_Station---\n");

    return;
}

static void OnHandle_BmsYX_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{   
    int n = 0;
	int nBattIdx = 0;
	// BmsPart1
	for(nBattIdx=0; nBattIdx<MAX_SINGLE_STBATT_NUM; ++nBattIdx)
	{
        if(pIEC104->strYC_BAT[nBattIdx].fRunState < STBATT_RUNSTAT_COMM_FAIL)
		{
			break;
		}

		/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
		CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
            0x1001 + nBattIdx*0x100, 1, false, false);
		
		IEC104_CONFIG_SIGMAP *pYX_BmsPart1Item;
	
		for (n = 0; n < pIEC104->nYX_BmsPart1Num; n++)
		{
			pYX_BmsPart1Item = pIEC104->pYX_BmsPart1[nBattIdx] + n;
			//printf("n=[%d] pIEC104->nYX_BmsPart2Num=[%d]\n", n, pIEC104->nYX_BmsPart2Num);
			if(pYX_BmsPart1Item == NULL)
			{
				TRACE("BattIdx:[%d]:pYX_BmsPart1Item == NUL\n", nBattIdx);
				break;
			}
	
			int tempVal =  *((int*)pYX_BmsPart1Item->ptrValue);
			 
			//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYX_BmsPart1Item->nIOA, tempVal);
			
			 InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYX_BmsPart1Item->nIOA, tempVal, pYX_BmsPart1Item->emQuality);
			
			CS101_ASDU_addInformationObject(newAsdu, io);
	
			InformationObject_destroy(io);
		}
	
        IMasterConnection_sendASDU(connection, newAsdu);
	
		CS101_ASDU_destroy(newAsdu);

	}

	// BmsPart2
	for(nBattIdx=0; nBattIdx<MAX_SINGLE_STBATT_NUM; ++nBattIdx)
	{
		if(pIEC104->strYC_BAT[nBattIdx].fRunState < STBATT_RUNSTAT_COMM_FAIL)
		{
			break;
		}

        /*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
		CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
            0x1081 + nBattIdx*0x100, 1, false, false);
		
		IEC104_CONFIG_SIGMAP *pYX_BmsPart2Item;
	
		for (n = 0; n < pIEC104->nYX_BmsPart2Num; n++)
		{
			pYX_BmsPart2Item = pIEC104->pYX_BmsPart2[nBattIdx] + n;
			//printf("n=[%d] pIEC104->nYX_BmsPart2Num=[%d]\n", n, pIEC104->nYX_BmsPart2Num);
			if(pYX_BmsPart2Item == NULL)
			{
				TRACE("BattIdx:[%d]:pYX_BmsPart1Item == NUL\n", nBattIdx);
				break;
			}
	
			int tempVal =  *((int*)pYX_BmsPart2Item->ptrValue);
			 
			//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYX_BmsPart1Item->nIOA, tempVal);
			
			 InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYX_BmsPart2Item->nIOA, tempVal, pYX_BmsPart2Item->emQuality);
			
			CS101_ASDU_addInformationObject(newAsdu, io);
	
			InformationObject_destroy(io);
		}
	
        IMasterConnection_sendASDU(connection, newAsdu);
	
		CS101_ASDU_destroy(newAsdu);

	}
	return;
}

static void OnHandle_PcsTotalYX_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{   
	int n = 0;
	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x2001, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYXPcsTotalItem;

	for (n = 0; n < pIEC104->nYX_PcsTotalNum; n++)
	{
		pYXPcsTotalItem = pIEC104->pYX_PcsTotal + n;
		//printf("n=[%d] pIEC104->nYX_PcsTotalNum=[%d]\n", n, pIEC104->nYX_PcsTotalNum);
		if(pYXPcsTotalItem == NULL)
		{
			TRACE("pYXPcsTotalItem == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYXPcsTotalItem->ptrValue);
 		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYXPcsTotalItem->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYXPcsTotalItem->nIOA, tempVal, pYXPcsTotalItem->emQuality);
		
		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YX_Station---\n");
	return;
}

static void OnHandle_PcsGrpYX_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{   
	int n = 0;
	int nBattIdx = 0;
	for(nBattIdx=0; nBattIdx < pIEC104->nCfgBMSNum; ++nBattIdx)
	{
		/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
		CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
            0x2101 + nBattIdx*0x100, 1, false, false);
		
		IEC104_CONFIG_SIGMAP *pYX_PcsGrpItem;
	
		for (n = 0; n < pIEC104->nYX_PcsGrpNum; n++)
		{
			pYX_PcsGrpItem = pIEC104->pYX_PcsGrp[nBattIdx] + n;
			//printf("n=[%d] pIEC104->nYX_PcsGrpNum=[%d]\n", n, pIEC104->nYX_PcsGrpNum);
			if(pYX_PcsGrpItem == NULL)
			{
				TRACE("BattIdx:[%d]:pYX_PcsGrpItem == NUL\n", nBattIdx);
				break;
			}
	
			int tempVal =  *((int*)pYX_PcsGrpItem->ptrValue);
			 
			//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYX_PcsGrpItem->nIOA, tempVal);
			
			 InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYX_PcsGrpItem->nIOA, tempVal, pYX_PcsGrpItem->emQuality);
			
			CS101_ASDU_addInformationObject(newAsdu, io);
	
			InformationObject_destroy(io);
		}
	
        IMasterConnection_sendASDU(connection, newAsdu);
	
		CS101_ASDU_destroy(newAsdu);

	}

    //printf("---OnHandle_YX_Station---\n");
	return;
}

static void OnHandle_STSYX_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{   
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
    PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue != LIQUID_COOLING_ADD_STS)
	{
		return;
	}

	int n = 0;
	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x2501, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYXSTSItem;

	for (n = 0; n < pIEC104->nYX_STSNum; n++)
	{
		pYXSTSItem = pIEC104->pYX_STS + n;
		//printf("n=[%d] pIEC104->nYX_STSNum=[%d]\n", n, pIEC104->nYX_STSNum);
		if(pYXSTSItem == NULL)
		{
			TRACE("pYXSTSItem == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYXSTSItem->ptrValue);
 		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYXSTSItem->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYXSTSItem->nIOA, tempVal, pYXSTSItem->emQuality);
		
		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YX_Station---\n");
	return;
}

static void OnHandle_FireYX_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{   
	int n = 0;
	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x2601, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYXFireItem;

	for (n = 0; n < pIEC104->nYX_FireNum; n++)
	{
		pYXFireItem = pIEC104->pYX_Fire + n;
		//printf("n=[%d] pIEC104->nYX_FireNum=[%d]\n", n, pIEC104->nYX_FireNum);
		if(pYXFireItem == NULL)
		{
			TRACE("pYXFireItem == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYXFireItem->ptrValue);
 		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYXFireItem->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYXFireItem->nIOA, tempVal, pYXFireItem->emQuality);
		
		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YX_Station---\n");
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  OnHandle_YC_EMS_Station() 总召遥测处理
*  CREATOR       : John
*  CREATED DATE  : 2023-6-2 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void OnHandle_YC_EMS_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

    CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
        0x4001, 1, false, false);
    
    IEC104_CONFIG_SIGMAP *pYCItem_Ems;

    int nRealEmsSectorNum = MIN((pIEC104->strYC_Ems.nEmsSectorNum*5+1), pIEC104->nYCNum_Ems);
    
    TRACE("nRealEmsSectorNum=[%d] nYCNum_Ems=[%d]\n", nRealEmsSectorNum, pIEC104->nYCNum_Ems);

    for (n = 0; n < nRealEmsSectorNum; n++)
	{
		pYCItem_Ems = pIEC104->pYCEle_Ems + n;
 		if(pYCItem_Ems == NULL)
		{
			printf("Error: Station pYCItem_Ems == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYCItem_Ems->ptrValue);
		tempVal = tempVal*((int)pYCItem_Ems->nScale);

        InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_Ems->nIOA, tempVal, pYCItem_Ems->emQuality);

        CS101_ASDU_addInformationObject(newAsdu, io);
        
        InformationObject_destroy(io);
    }

    IMasterConnection_sendASDU(connection, newAsdu);

    CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YC_EMS_Station---\n");

    return;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  OnHandle_YC_Sys_Station() 总召遥测处理
*  CREATOR       : John
*  CREATED DATE  : 2023-6-2 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void OnHandle_YC_Sys_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

    CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
        0x4051, 1, false, false);
    
	IEC104_CONFIG_SIGMAP *pYCItem_Sys;

    for (n = 0; n < pIEC104->nYCNum_Sys; n++)
	{
		pYCItem_Sys = pIEC104->pYCEle_Sys + n;
 		if(pYCItem_Sys == NULL)
		{
			printf("Error: Station pYCItem_Sys == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYCItem_Sys->ptrValue);
		tempVal = tempVal*((int)pYCItem_Sys->nScale);

        InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_Sys->nIOA, tempVal, pYCItem_Sys->emQuality);

        CS101_ASDU_addInformationObject(newAsdu, io);
        
        InformationObject_destroy(io);
    }

    IMasterConnection_sendASDU(connection, newAsdu);

    CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YC_Sys_Station---\n");

    return;
}

static void OnHandle_YCCabMeter_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    if(pIEC104->strYC_SysInfo.emCabMeter_CommStatu == 1) //2023-07-17 langxin Request CommFail Not Send Value.
	{
		return;
	}
    int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4101, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_CabMeter;

	for (n = 0; n < pIEC104->nYCNum_CabMeter; n++)
	{
		pYCItem_CabMeter = pIEC104->pYCEle_CabMeter + n;
		if(pYCItem_CabMeter == NULL)
		{
			printf("pYCItem_CabMeter == NUL\n");
			break;
		}

		float tempVal =  *((float*)pYCItem_CabMeter->ptrValue);
		tempVal = tempVal*((float)pYCItem_CabMeter->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CabMeter->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_CabMeter->nIOA, (int)tempVal, pYCItem_CabMeter->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CabMeter->nIOA, tempVal, pYCItem_CabMeter->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

    CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YCCabMeter_Station---\n");
    return;
}

static void OnHandle_YCGridMeter_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    if(pIEC104->strYC_SysInfo.emGridMeter_CommStatu == 1) //2023-07-17 langxin Request CommFail Not Send Value.
	{
		return;
	}

    int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4201, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_GridMeter;

	for (n = 0; n < pIEC104->nYCNum_GridMeter; n++)
	{
		pYCItem_GridMeter = pIEC104->pYCEle_GridMeter + n;
		if(pYCItem_GridMeter == NULL)
		{
			printf("pYCItem_GridMeter == NUL\n");
			break;
		}

		float tempVal =  *((float*)pYCItem_GridMeter->ptrValue);
		tempVal = tempVal*((float)pYCItem_GridMeter->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_GridMeter->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_CabMeter->nIOA, (int)tempVal, pYCItem_CabMeter->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_GridMeter->nIOA, tempVal, pYCItem_GridMeter->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCGridMeter_Station---\n");

	return;
}



static void OnHandle_YCAuxMeter_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    if(pIEC104->strYC_SysInfo.emAuxMeter_CommStatu == 1) //2023-08-08  Upload Aux ACMeter.
	{
		return;
	}

    int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4251, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_AuxMeter;

	for (n = 0; n < pIEC104->nYCNum_AuxMeter; n++)
	{
		pYCItem_AuxMeter = pIEC104->pYCEle_AuxMeter + n;
		if(pYCItem_AuxMeter == NULL)
		{
			printf("pYCItem_AuxMeter == NUL\n");
			break;
		}

		float tempVal =  *((float*)pYCItem_AuxMeter->ptrValue);
		tempVal = tempVal*((float)pYCItem_AuxMeter->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_AuxMeter->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_AuxMeter->nIOA, tempVal, pYCItem_AuxMeter->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCAuxMeter_Station---\n");

	return;
}

static void OnHandle_YCAux3Meter_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    if(pIEC104->strYC_SysInfo.emAux3Meter_CommStatu >= 1) //2024-04-12  Upload Aux3 ACMeter.
	{
		return;
	}

    int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x42A1, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_Aux3Meter;

	for (n = 0; n < pIEC104->nYCNum_Aux3Meter; n++)
	{
		pYCItem_Aux3Meter = pIEC104->pYCEle_Aux3Meter + n;
		if(pYCItem_Aux3Meter == NULL)
		{
			printf("pYCItem_Aux3Meter == NUL\n");
			break;
		}

		float tempVal =  *((float*)pYCItem_Aux3Meter->ptrValue);
		tempVal = tempVal*((float)pYCItem_Aux3Meter->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_AuxMeter->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_Aux3Meter->nIOA, tempVal, pYCItem_Aux3Meter->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCAux3Meter_Station---\n");

	return;
}

static void OnHandle_YCBatG1_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

	if(pIEC104->strYC_BAT[0].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
     0x4301, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_BatG1;

	for (n = 0; n < pIEC104->nYCNum_BatG1; n++)
	{
		pYCItem_BatG1 = pIEC104->pYCEle_BatG1 + n;
		if(pYCItem_BatG1 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_BatG1->ptrValue);
		tempVal = tempVal*((float)pYCItem_BatG1->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_BatG1->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_BatG1->nIOA, (int)tempVal, pYCItem_BatG1->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_BatG1->nIOA, tempVal, pYCItem_BatG1->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YCBatG1_Station---\n");

    return;
}

static void OnHandle_YCBatG2_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

	if(pIEC104->strYC_BAT[1].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4321, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_BatG2;

	for (n = 0; n < pIEC104->nYCNum_BatG2; n++)
	{
		pYCItem_BatG2 = pIEC104->pYCEle_BatG2 + n;
		if(pYCItem_BatG2 == NULL)
		{
			return;
		}

		float tempVal =  *((float*)pYCItem_BatG2->ptrValue);
		tempVal = tempVal*((float)pYCItem_BatG2->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_BatG2->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_BatG2->nIOA, tempVal, pYCItem_BatG2->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCBatG2_Station---\n");

    return;
}

static void OnHandle_YCBatG3_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;
	if(pIEC104->strYC_BAT[2].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4341, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_BatG3;

	for (n = 0; n < pIEC104->nYCNum_BatG3; n++)
	{
		pYCItem_BatG3 = pIEC104->pYCEle_BatG3 + n;
		if(pYCItem_BatG3 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_BatG3->ptrValue);
		tempVal = tempVal*((float)pYCItem_BatG3->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_BatG3->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_BatG3->nIOA, tempVal, pYCItem_BatG3->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCBatG3_Station---\n");

    return;
}

static void OnHandle_YCBatG4_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

	if(pIEC104->strYC_BAT[3].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4361, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_BatG4;

	for (n = 0; n < pIEC104->nYCNum_BatG4; n++)
	{
		pYCItem_BatG4 = pIEC104->pYCEle_BatG4 + n;
		if(pYCItem_BatG4 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_BatG4->ptrValue);
		tempVal = tempVal*((float)pYCItem_BatG4->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_BatG4->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_BatG4->nIOA, tempVal, pYCItem_BatG4->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCBatG4_Station---\n");

	return;
}

static void OnHandle_YCPCS_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4501, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_PCS;

	for (n = 0; n < pIEC104->nYCNum_PCS; n++)
	{
		pYCItem_PCS = pIEC104->pYCEle_PCS + n;
		if(pYCItem_PCS == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_PCS->ptrValue);
		tempVal = tempVal*((float)pYCItem_PCS->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_PCS->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_PCS->nIOA, tempVal, pYCItem_PCS->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCPCS_Station---\n");

    return;
}


static void OnHandle_YCSiglPCS_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4601, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_SiglPCS;

	for (n = 0; n < pIEC104->nYCNum_SiglPCS; n++)
	{
		pYCItem_SiglPCS = pIEC104->pYCEle_SiglPCS + n;
		if(pYCItem_SiglPCS == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_SiglPCS->ptrValue);
		tempVal = tempVal*((float)pYCItem_SiglPCS->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_SiglPCS->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_SiglPCS->nIOA, tempVal, pYCItem_SiglPCS->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCSiglPCS_Station---\n");

    return;
}


static void OnHandle_YCMpptGrp_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;
    if(pIEC104->bNeedUploadMppt == FALSE)
    {
        return;
    }

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x4701, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_MpptGrp;

	for (n = 0; n < pIEC104->nYCNum_MpptGrp; n++)
	{
		pYCItem_MpptGrp = pIEC104->pYCEle_MpptGrp + n;
		if(pYCItem_MpptGrp == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_MpptGrp->ptrValue);
		tempVal = tempVal*((float)pYCItem_MpptGrp->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_MpptGrp->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_MpptGrp->nIOA, tempVal, pYCItem_MpptGrp->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCMpptGrp_Station---\n");

    return;
}

//YNT
static void OnHandle_YCCellG1_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection, int id)
{
    int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[0].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
     0xA001 + offset, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_CellG1;

	for (n = 0; (((n + offset) < pIEC104->nYCNum_CellG1) && (n < 48)); n++)
	{
		pYCItem_CellG1 = pIEC104->pYCEle_CellG1 + offset + n;
		if(pYCItem_CellG1 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_CellG1->ptrValue);
		tempVal = tempVal*((float)pYCItem_CellG1->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG1->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_CellG1->nIOA, (int)tempVal, pYCItem_CellG1->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG1->nIOA, tempVal, pYCItem_CellG1->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);


    return;
}

static void OnHandle_YCCellG2_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection, int id)
{
    int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[1].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0xA1E1 + offset, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_CellG2;

	for (n = 0; (((n + offset) < pIEC104->nYCNum_CellG2) && (n < 48)); n++)
	{
		pYCItem_CellG2 = pIEC104->pYCEle_CellG2 + offset + n;
		if(pYCItem_CellG2 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_CellG2->ptrValue);
		tempVal = tempVal*((float)pYCItem_CellG2->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG1->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_CellG1->nIOA, (int)tempVal, pYCItem_CellG1->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG2->nIOA, tempVal, pYCItem_CellG2->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);


    return;
}

static void OnHandle_YCCellG3_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection, int id)
{
    int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[2].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
        0xA3C1 + offset, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_CellG3;

	for (n = 0; (((n + offset) < pIEC104->nYCNum_CellG3) && (n < 48)); n++)
	{
		pYCItem_CellG3 = pIEC104->pYCEle_CellG3 + offset + n;
		if(pYCItem_CellG3 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_CellG3->ptrValue);
		tempVal = tempVal*((float)pYCItem_CellG3->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG1->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_CellG1->nIOA, (int)tempVal, pYCItem_CellG1->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG3->nIOA, tempVal, pYCItem_CellG3->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);


    return;
}

static void OnHandle_YCCellG4_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection, int id)
{
    int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[3].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
        0xA5A1 + offset, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_CellG4;

	for (n = 0; (((n + offset) < pIEC104->nYCNum_CellG4) && (n < 48)); n++)
	{
		pYCItem_CellG4 = pIEC104->pYCEle_CellG4 + offset + n;
		if(pYCItem_CellG4 == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_CellG4->ptrValue);
		tempVal = tempVal*((float)pYCItem_CellG4->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG1->nIOA, tempVal);
		
		//InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_CellG1->nIOA, (int)tempVal, pYCItem_CellG1->emQuality);
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG4->nIOA, tempVal, pYCItem_CellG4->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);


    return;
}

static void OnHandle_YCCellGrp_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
	int i=0;
	for(i=0; i<10; i++)
	{
        OnHandle_YCCellG1_Station(pIEC104, alParams, connection, i);
        OnHandle_YCCellG2_Station(pIEC104, alParams, connection, i);
        OnHandle_YCCellG3_Station(pIEC104, alParams, connection, i);
        OnHandle_YCCellG4_Station(pIEC104, alParams, connection, i);
    
    }

	return;
}


static void OnHandle_YCKWh_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

	Refresh_YC_KWh_DataMap(pIEC104);

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION, 
        0x6401, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_KWh;

	for (n = 0; n < pIEC104->nYCNum_KWh; n++)
	{
		pYCItem_KWh = pIEC104->pYCEle_KWh + n;
		if(pYCItem_KWh == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_KWh->ptrValue);
		tempVal = tempVal*((float)pYCItem_KWh->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_KWh->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_KWh->nIOA, tempVal, pYCItem_KWh->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

    IMasterConnection_sendASDU(connection, newAsdu);

	CS101_ASDU_destroy(newAsdu);
    //printf("---OnHandle_YCKWh_Station---\n");

    return;
}

static void OnHandle_YC_Air_Station(IEC104_DATA_ROUGH* pIEC104, CS101_AppLayerParameters alParams, IMasterConnection connection)
{
    int n = 0;

    CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_INTERROGATED_BY_STATION,
        0x5051, 1, false, false);
    
	IEC104_CONFIG_SIGMAP *pYCItem_Air;

    for (n = 0; n < pIEC104->nYCNum_AirInfo; n++)
	{
		pYCItem_Air = pIEC104->pYCEle_AirInfo + n;
 		if(pYCItem_Air == NULL)
		{
			printf("Error: Station pYCItem_Air == NUL\n");
			break;
		}

		float tempVal =  *((float*)pYCItem_Air->ptrValue);
		tempVal = tempVal*((float)pYCItem_Air->nScale);

        InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_Air->nIOA, tempVal, pYCItem_Air->emQuality);

        CS101_ASDU_addInformationObject(newAsdu, io);
        
        InformationObject_destroy(io);
    }

    IMasterConnection_sendASDU(connection, newAsdu);

    CS101_ASDU_destroy(newAsdu);

    //printf("---OnHandle_YC_Sys_Station---\n");

    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  interrogationHandler() 总召处理
*  CREATOR       : John
*  CREATED DATE  : 2023-6-2 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi)
{    
    IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)parameter;
    
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
    PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;

    TRACE("Received interrogation for group %i\n", qoi);

    if (qoi == 20) // only handle station interrogation 总召唤限定词=20
    { 
        pIEC104->bOn_Interrogating = TRUE;
        //------------ 1、响应client的 CS101_COT_ACTIVATION 请求 -----------//
        CS101_AppLayerParameters alParams = IMasterConnection_getApplicationLayerParameters(connection);

        IMasterConnection_sendACT_CON(connection, asdu, false);

        // The CS101 specification only allows information objects without timestamp in GI responses 
        //! 2、发送ASDU, 其中COT设置为CS101_COT_INTERROGATED_BY_STATION，从站总召 

        OnHandle_YC_EMS_Station(pIEC104, alParams, connection);
        
        OnHandle_YC_Sys_Station(pIEC104, alParams, connection);

        OnHandle_BmsYX_Station(pIEC104, alParams, connection);

        OnHandle_PcsTotalYX_Station(pIEC104, alParams, connection);

        OnHandle_PcsGrpYX_Station(pIEC104, alParams, connection);

        pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
        if(valGet.val.emValue == LIQUID_COOLING_ADD_STS)
        {
            OnHandle_STSYX_Station(pIEC104, alParams, connection);
        }

        OnHandle_FireYX_Station(pIEC104, alParams, connection);

        OnHandle_YX_Station(pIEC104, alParams, connection);

        //---Added By John. LangXin Need Station Triiger All Msg.---//

        OnHandle_YCCabMeter_Station(pIEC104, alParams, connection);
	
		OnHandle_YCGridMeter_Station(pIEC104, alParams, connection);

        OnHandle_YCAuxMeter_Station(pIEC104, alParams, connection);

        OnHandle_YCAux3Meter_Station(pIEC104, alParams, connection);

        OnHandle_YCBatG1_Station(pIEC104, alParams, connection);

		OnHandle_YCBatG2_Station(pIEC104, alParams, connection);

		OnHandle_YCBatG3_Station(pIEC104, alParams, connection);

		OnHandle_YCBatG4_Station(pIEC104, alParams, connection);

        OnHandle_YCPCS_Station(pIEC104, alParams, connection);

        OnHandle_YCSiglPCS_Station(pIEC104, alParams, connection);

        OnHandle_YCMpptGrp_Station(pIEC104, alParams, connection);

        // YNT
        if(pIEC104->bNeedUploadCellInfo)
		{
            OnHandle_YCCellGrp_Station(pIEC104, alParams, connection);
		}
        //YNT over

		OnHandle_YCKWh_Station(pIEC104, alParams, connection);

        OnHandle_YC_Air_Station(pIEC104, alParams, connection);
        //---------------------------  End ---------------------------------//
 
        //--- 结束响应必须发送 CS101_COT_ACTIVATION_TERMINATION ---//
        IMasterConnection_sendACT_TERM(connection, asdu);
    }
    else 
    {
        IMasterConnection_sendACT_CON(connection, asdu, true);
    }
    pIEC104->bOn_Interrogating = FALSE;

    TRACE("End interrogation for group %i\n", qoi);

    return true;
}

BOOL On_Hander_Reset(IEC104_DATA_ROUGH* pIEC104 , BOOL bCtrlState)
{
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

	BOOL bAccepted = FALSE;
	VAR_VALUE varV;

	varV.emValue = bCtrlState ? 1: 0;
	ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
			1, 2, &varV, 4, 3000);
		bAccepted = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
	
    TRACE("On_Hander_Reset:bCtrlState=[%d] err=[%d]\n", bCtrlState, err);

	return bAccepted;
}

BOOL On_Hander_Lock(IEC104_DATA_ROUGH* pIEC104 , BOOL bCtrlState)
{
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

	BOOL bAccepted = FALSE;
	VAR_VALUE varV;

	varV.emValue = bCtrlState ? 1: 0;
	ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			200, 198, &varV, 4, 3000);
		bAccepted = (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
	
    TRACE("On_Hander_Lock:bCtrlState=[%d] err=[%d]\n", bCtrlState, err);

	return bAccepted;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Handle_ParamStat()
*  CREATOR       : George
*  CREATED DATE  : 2025-1-23 11:45:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
1.241201 0x6191:+充-放      0x6192:EMSCtrlMode                            川崎、江苏扬州-邦道、金华-中能电科-top#6(祥宏&新电途)
2.250608 0x6391:+放-充                                                    深瑞同华-广储(百济神州)
3.251030 0x6392:EMSCtrlMode 0x6393:ACDC_WorkStatu 0x6394:ActvPwrReq(ABS)  新协议
***********************************************************************************/
void Handle_ParamStat(IEC104_DATA_ROUGH* pIEC104, int nIOA, float fSetValue)
{
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

    VAR_VALUE varV;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;
    int nStatusParamID = -1;
    int nDevId = -1;

    switch (nIOA)
    {
        case 0x6391: // +放-充 深瑞同华只发0x6391
        {
            // 1. 1001, 3902, emEMSCtrlMode_REMS
            varV.emValue = 1;
            err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
                1001, 3902, &varV, 4, 100);
                
            pIEC104->emEMSCtrlMode_REMS = EMSCtrlMode_Remote;

            // 2. 1001, 3907, emACDC_WorkStatu_REMS
            ACDC_WORKSTATU emACDC_WorkStatu = STATU_ACDC_IDLE;
            if (fSetValue < -1)
            {
                emACDC_WorkStatu = STATU_ACDC_RECTIFY;
            }
            else if (fSetValue > 1)
            {
                emACDC_WorkStatu = STATU_ACDC_INVERTER;
            }
            varV.emValue = (_ENUM)emACDC_WorkStatu;
            err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
                1001, 3907, &varV, 4, 100);

            // 3. 1001, 3901, fActvPwrReq_REMS
            nDevId = 1001; nStatusParamID = 3901;
            varV.fValue = ABS(fSetValue);
            TRACED("104:Handle_ParamStat 0x6391 emACDC_WorkStatu:%d fSetValue:%.1f", emACDC_WorkStatu, fSetValue);
        }
            break;

        case 0x6191: // +充-放 川崎、朗新
        {
            // 2. 1001, 3907, emACDC_WorkStatu_REMS
            ACDC_WORKSTATU emACDC_WorkStatu = STATU_ACDC_IDLE;
            if (fSetValue > 1)
            {
                emACDC_WorkStatu = STATU_ACDC_RECTIFY;
            }
            else if (fSetValue < -1)
            {
                emACDC_WorkStatu = STATU_ACDC_INVERTER;
            }
            varV.emValue = (_ENUM)emACDC_WorkStatu;
            err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
                1001, 3907, &varV, 4, 100);

            // 3. 1001, 3901, fActvPwrReq_REMS
            nDevId = 1001; nStatusParamID = 3901; // 1001, 3901, fActvPwrReq_REMS
            varV.fValue = ABS(fSetValue);
            TRACED("104:Handle_ParamStat 0x6191 emACDC_WorkStatu:%d fSetValue:%.1f err:%d", emACDC_WorkStatu, fSetValue, err);
        }
            break;

        case 0x6392: // 新协议
        case 0x6192:    nDevId = 1001; nStatusParamID = 3902; // 1001, 3902, emEMSCtrlMode_REMS
            pIEC104->emEMSCtrlMode_REMS = fSetValue;
            varV.emValue = fSetValue;
            break;
        case 0x6393:    nDevId = 1001; nStatusParamID = 3907; // 1001, 3907, emACDC_WorkStatu_REMS 0-Idle 1-rectify 2-inverter 3-standby
            varV.emValue = fSetValue;
            break;
        case 0x6394:    nDevId = 1001; nStatusParamID = 3901; // 1001, 3901, fActvPwrReq_REMS
            varV.fValue = fSetValue;
            break;

        default:
            return;
    }

    err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
            nDevId, nStatusParamID, &varV, 4, 100);
    if(err != ERRCODE_DAI_OK)
	{
		TRACE("Get A_BLOB_GROUP_STBATT_RUNINFO Error!!!\n");
		return;
	}
    
    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Handle_YT_ParamSett()
*  CREATOR       : John
*  CREATED DATE  : 2023-6-2 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Handle_YT_ParamSett(IEC104_DATA_ROUGH* pIEC104, int nIOA, int nSetValue)
{  
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;

    VAR_VALUE varV;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;

    switch (nIOA)
    {
        case 0x6101:
            pIEC104->blobBackendSet.fSysACInput_PwrLmt = (float)nSetValue;
            LogOut4c(MAIN_MODULE_REPORTER, "IEC-104", LOG_TYPE_INFO, "fSysACInput_PwrLmt:%d", nSetValue);
            break;
        case 0x6102:
            pIEC104->blobBackendSet.nCmd_107_Interval = (UINT)nSetValue;
            break;
        case 0x6103:
            pIEC104->blobBackendSet.nCmd_103_Interval = (UINT)nSetValue;
            break;
        case 0x6104:
            pIEC104->blobBackendSet.nCmd_105_Interval = (UINT)nSetValue;
            break;
        case 0x6105:
            pIEC104->blobBackendSet.nCmd_204_Interval = (UINT)nSetValue;
            break;
        default:
            break;
    }

	pIEC104->blobBackendSet.emBlobType = BLOB_TYPE_ES_BACKEND_CFG;
	varV.blobValue = &(pIEC104->blobBackendSet);
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG), 1000);
	
   	if (err != ERRCODE_DAI_OK)
	{
		valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
		valGet.val.blobValue = (BYTE*)&pIEC104->blobBackendSet;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 26, &valGet, sizeof(PARAM_VAL_GET)); 
	}

    return;
}


static BOOL Handle_C2S_EMS_Param(IEC104_DATA_ROUGH* pIEC104, int nIOA, int nSetValue)
{
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
    BOOL    bRet = FALSE;
    int     n = 0;
   
	IEC104_CONFIG_SIGMAP *pYTItem_C2S;

    for(n = 0; n < pIEC104->nC2S_EmsNum; n++)
    {
        pYTItem_C2S = pIEC104->pYTEle_C2SEms + n;
        if(nIOA == pYTItem_C2S->nIOA)
        {
            *((int*)pYTItem_C2S->ptrValue) = nSetValue;
            
            if (nIOA == 0x6201)
            {
                pIEC104->strYC_Ems.nEmsSectorNum = nSetValue;
                pIEC104->emsPlan.nTotal_EmsLmt_Sectors = pIEC104->strYC_Ems.nEmsSectorNum;
            }
            else if (nIOA >= 0x6202 && nIOA <= 0x623D)
            {
                int m;
                time_t tm_Now = _NOW_;
                struct tm gmTime_Sec_Start;
 
                gmtime_r(&tm_Now, &gmTime_Sec_Start);
                gmTime_Sec_Start.tm_sec = 0;
                gmTime_Sec_Start.tm_min = 0;
                gmTime_Sec_Start.tm_hour = 0;
                
                TRACE("tm_year=[%d] tm_mon=[%d] tm_mday=[%d]\n",\
                    gmTime_Sec_Start.tm_year ,
                    gmTime_Sec_Start.tm_mon ,
                    gmTime_Sec_Start.tm_mday);

                time_t tmNewStart = mktime(&gmTime_Sec_Start);		

                TRACE("tm_Now =[%ld] tmNewStart=[%ld]\n", tm_Now, tmNewStart);
                for (m = 0; m < pIEC104->nC2S_EmsNum; m++)
                {
                    if (nIOA == pIEC104->pYTEle_C2SEms[m].nIOA)
                    {
                        int index = (nIOA - 0x6202) / 5;
                        int offset = (nIOA - 0x6202) % 5;
                        int nInterval_Sec = 0;
                        switch (offset)
                        {
                            case 0:
                                pIEC104->strYC_Ems.arryEmsParam[index].tmStart = pIEC104->strC2S_Ems.arryEmsParam[index].tmStart;
                                nInterval_Sec = pIEC104->strC2S_Ems.arryEmsParam[index].tmStart*60;
		                        pIEC104->emsPlan.emsLmtSectors[index].tmStart = tmNewStart + nInterval_Sec;
                                break;
                            case 1:
                                pIEC104->strYC_Ems.arryEmsParam[index].nStopSOC = pIEC104->strC2S_Ems.arryEmsParam[index].nStopSOC;
                                break;
                            case 2:
                                pIEC104->strYC_Ems.arryEmsParam[index].nBattGrpPwrLmt = pIEC104->strC2S_Ems.arryEmsParam[index].nBattGrpPwrLmt;
                                break;
                            case 3:
                                pIEC104->strYC_Ems.arryEmsParam[index].nActvPwrLmt = pIEC104->strC2S_Ems.arryEmsParam[index].nActvPwrLmt;
                                break;
                            case 4:
                                pIEC104->strYC_Ems.arryEmsParam[index].nReActvPwrLmt = pIEC104->strC2S_Ems.arryEmsParam[index].nReActvPwrLmt;
                                break;
                            default:
                                // printf("can't get any EMS plan info?????\n", offset);
                                break;
                        }
                        break;
                    }
                }
            }

            ERRCODE_DAI err = -1;
            PARAM_VAL_GET valGet;
            int l = 0;
            int nInterval_Sec = 0;
            time_t tm_Now = _NOW_;
            static time_t tm_LastTM ;

            // if((tm_Now - tm_LastTM) < 5)
            // {
            //     return;
            // }
            // tm_LastTM = tm_Now;


            struct tm gmTime_Sec_Start;
        
            gmtime_r(&tm_Now, &gmTime_Sec_Start);
            gmTime_Sec_Start.tm_sec = 0;
            gmTime_Sec_Start.tm_min = 0;
            gmTime_Sec_Start.tm_hour = 0;
            
            TRACE("tm_year=[%d] tm_mon=[%d] tm_mday=[%d]\n",\
                gmTime_Sec_Start.tm_year ,
                gmTime_Sec_Start.tm_mon ,
                gmTime_Sec_Start.tm_mday);

            time_t tmNewStart = mktime(&gmTime_Sec_Start);		

            TRACE("tm_Now =[%ld] tmNewStart=[%ld]\n", tm_Now, tmNewStart);

        
            int nSectorNum = MIN(MAX_EMS_SECTOR,pIEC104->strYC_Ems.nEmsSectorNum);
            pIEC104->emsPlan.nTotal_EmsLmt_Sectors = nSectorNum; //Total Sector Number
            for(l = 0; l < nSectorNum; l++)
            {	
                nInterval_Sec = pIEC104->strYC_Ems.arryEmsParam[l].tmStart*60;

                pIEC104->emsPlan.emsLmtSectors[l].tmStart = tmNewStart + nInterval_Sec;

                if(nSectorNum <= 1)
                {
                    pIEC104->emsPlan.emsLmtSectors[l].nInterval = 86400;
                }
                else
                {
                    if((l+1) < nSectorNum)
                    {
                        pIEC104->emsPlan.emsLmtSectors[l].nInterval = 
                            (pIEC104->strYC_Ems.arryEmsParam[l+1].tmStart - pIEC104->strYC_Ems.arryEmsParam[l].tmStart)*60;
                    }
                    else
                    {
                        pIEC104->emsPlan.emsLmtSectors[l].nInterval = 86400 - (pIEC104->strYC_Ems.arryEmsParam[l].tmStart*60);
                    }
                }

                pIEC104->emsPlan.emsLmtSectors[l].nStopSOC = pIEC104->strYC_Ems.arryEmsParam[l].nStopSOC;

                if(pIEC104->strYC_Ems.arryEmsParam[l].nActvPwrLmt < 0)
                {
                    pIEC104->emsPlan.emsLmtSectors[l].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;

                }
                else
                {
                    pIEC104->emsPlan.emsLmtSectors[l].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING; 
                }


                pIEC104->emsPlan.emsLmtSectors[l].nBattGrpPwrLmt =  pIEC104->strYC_Ems.arryEmsParam[l].nBattGrpPwrLmt;

                pIEC104->emsPlan.emsLmtSectors[l].nActvPwrLmt =  ABS(pIEC104->strYC_Ems.arryEmsParam[l].nActvPwrLmt); 

                pIEC104->emsPlan.emsLmtSectors[l].nReActvPwrLmt = pIEC104->strYC_Ems.arryEmsParam[l].nReActvPwrLmt;  
            }

            //Update EMS Config Plan into DB
            VAR_VALUE varV;
            pIEC104->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
            varV.blobValue = &pIEC104->emsPlan;
            err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
                    200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);
            if (err == ERRCODE_DAI_OK)
            {
                TRACE("Info: EMS Config Plan Set Sucessfully\n");
            }
            else
            {
                TRACEX("Error: EMS Config Plan Update Failure!\n");
                //Revert EMS Config Settings
                valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
                valGet.val.blobValue = (BYTE*)&pIEC104->emsPlan;
                err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
                        200, 201, &valGet, sizeof(PARAM_VAL_GET));
            }
            
            bRet = TRUE;
            pIEC104->bTrigger = TRUE;
            pIEC104->bTriggerEmsYC = TRUE;
            break;
       }
    }

    return bRet;
}


static BOOL Compare_IfNeedUpdate_EMSParam(IEC104_DATA_ROUGH* pIEC104)
{
    BOOL bRet = FALSE;
    int n = 0;

    pIEC104->strC2S_Ems.nEmsSectorNum = MIN(MAX_EMS_SECTOR, pIEC104->strC2S_Ems.nEmsSectorNum);
    if(pIEC104->strYC_Ems.nEmsSectorNum != pIEC104->strC2S_Ems.nEmsSectorNum)
    {
        TRACE("strYC_Ems.nEmsSectorNum=[%d] strC2S_Ems.nEmsSectorNum=[%d]\n", pIEC104->strYC_Ems.nEmsSectorNum , pIEC104->strC2S_Ems.nEmsSectorNum);
        return TRUE;
    }

    for(n = 0; n < pIEC104->strC2S_Ems.nEmsSectorNum; n++)
    {
        if(pIEC104->strC2S_Ems.arryEmsParam[n].tmStart != pIEC104->strYC_Ems.arryEmsParam[n].tmStart)
        {
            TRACE("strYC_Ems.tmStart=[%ld] strC2S_Ems.tmStart=[%ld]\n", \
                pIEC104->strC2S_Ems.arryEmsParam[n].tmStart , pIEC104->strYC_Ems.arryEmsParam[n].tmStart);

            bRet = TRUE;
            break;
        }
 
        if(pIEC104->strC2S_Ems.arryEmsParam[n].nStopSOC != pIEC104->strYC_Ems.arryEmsParam[n].nStopSOC)
        {
            TRACE("strYC_Ems.nStopSOC=[%d] strC2S_Ems.nStopSOC=[%d]\n", \
                pIEC104->strC2S_Ems.arryEmsParam[n].nStopSOC , pIEC104->strYC_Ems.arryEmsParam[n].nStopSOC);

            bRet = TRUE;
            break;
        }

        if(pIEC104->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt != pIEC104->strYC_Ems.arryEmsParam[n].nBattGrpPwrLmt)
        {
            TRACE("strYC_Ems.nBattGrpPwrLmt=[%d] strC2S_Ems.nBattGrpPwrLmt=[%d]\n", \
                pIEC104->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt , pIEC104->strYC_Ems.arryEmsParam[n].nBattGrpPwrLmt);

            bRet = TRUE;
            break;
        }

        if(pIEC104->strC2S_Ems.arryEmsParam[n].nActvPwrLmt != pIEC104->strYC_Ems.arryEmsParam[n].nActvPwrLmt)
        {
            bRet = TRUE;
            break;
        }

        if(pIEC104->strC2S_Ems.arryEmsParam[n].nReActvPwrLmt != pIEC104->strYC_Ems.arryEmsParam[n].nReActvPwrLmt)
        {
            bRet = TRUE;
            break;
        } 

    }

    return bRet;
}

static void Update_EMSParam2Local(IEC104_DATA_ROUGH* pIEC104)
{
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	ERRCODE_DAI err = -1;
	PARAM_VAL_GET valGet;
    int n = 0;
    int nInterval_Sec = 0;
    time_t tm_Now = _NOW_;
    static time_t tm_LastTM ;

    if((tm_Now - tm_LastTM) < 10)
    {
        return;
    }
    tm_LastTM = tm_Now;


	struct tm gmTime_Sec_Start;
 
	gmtime_r(&tm_Now, &gmTime_Sec_Start);
	gmTime_Sec_Start.tm_sec = 0;
	gmTime_Sec_Start.tm_min = 0;
	gmTime_Sec_Start.tm_hour = 0;
	
	TRACE("tm_year=[%d] tm_mon=[%d] tm_mday=[%d]\n",\
		gmTime_Sec_Start.tm_year ,
		gmTime_Sec_Start.tm_mon ,
		gmTime_Sec_Start.tm_mday);

	time_t tmNewStart = mktime(&gmTime_Sec_Start);		

	TRACE("tm_Now =[%ld] tmNewStart=[%ld]\n", tm_Now, tmNewStart);

 
    int nSectorNum = MIN(MAX_EMS_SECTOR,pIEC104->strC2S_Ems.nEmsSectorNum);
	for(n = 0; n < nSectorNum; n++)
	{	
		pIEC104->emsPlan.nTotal_EmsLmt_Sectors = nSectorNum; //Total Sector Number

		nInterval_Sec = pIEC104->strC2S_Ems.arryEmsParam[n].tmStart*60;

		pIEC104->emsPlan.emsLmtSectors[n].tmStart = tmNewStart + nInterval_Sec;

		if(nSectorNum <= 1)
		{
			pIEC104->emsPlan.emsLmtSectors[n].nInterval = 86400;
		}
		else
		{
			if((n+1) < nSectorNum)
			{
				pIEC104->emsPlan.emsLmtSectors[n].nInterval = 
					(pIEC104->strC2S_Ems.arryEmsParam[n+1].tmStart - pIEC104->strC2S_Ems.arryEmsParam[n].tmStart)*60;
			}
			else
			{
				pIEC104->emsPlan.emsLmtSectors[n].nInterval = 86400 - (pIEC104->strC2S_Ems.arryEmsParam[n].tmStart*60);
			}
		}

        pIEC104->emsPlan.emsLmtSectors[n].nStopSOC = pIEC104->strC2S_Ems.arryEmsParam[n].nStopSOC;

        if(pIEC104->strC2S_Ems.arryEmsParam[n].nActvPwrLmt < 0)
        {
		    pIEC104->emsPlan.emsLmtSectors[n].emBatt_WorkMode = BATT_WORKMODE_FIRST_DISCHRG;

        } else
        {
            pIEC104->emsPlan.emsLmtSectors[n].emBatt_WorkMode = BATT_WORKMODE_FIRST_CHRGING; 
        }

		pIEC104->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt =  pIEC104->strC2S_Ems.arryEmsParam[n].nBattGrpPwrLmt;

		pIEC104->emsPlan.emsLmtSectors[n].nActvPwrLmt =  ABS(pIEC104->strC2S_Ems.arryEmsParam[n].nActvPwrLmt); 

		pIEC104->emsPlan.emsLmtSectors[n].nReActvPwrLmt = pIEC104->strC2S_Ems.arryEmsParam[n].nReActvPwrLmt;  
    }

    VAR_VALUE varV;
	pIEC104->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	varV.blobValue = &pIEC104->emsPlan;
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
			200, 201, &varV, sizeof(A_BLOB_EMS_CONFIG_PLAN), 1000);
	if (err == ERRCODE_DAI_OK)
	{
		TRACE("Info: EMS Config Plan Set Sucessfully\n");
        memcpy(&pIEC104->strYC_Ems, &pIEC104->strC2S_Ems, sizeof(STR_YC_PARAM_EMS));
	}
	else
	{
		TRACEX("Error: EMS Config Plan Update Failure!\n");
		//Revert EMS Config Settings
		valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
		valGet.val.blobValue = (BYTE*)&pIEC104->emsPlan;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				200, 201, &valGet, sizeof(PARAM_VAL_GET));
	}

    return;
}

/*
*ASDU报文处理
*RCVD: 68 0e 04 00 68 07 2d 01 06 00 01 00 00 00 00 00 
received single command
SEND: 68 0e 68 07 06 00 2d 01 2f 00 01 00 00 00 00 00 
SEND: 68 10 6a 07 06 00 0b 01 01 00 01 00 6e 00 00 a2 84 00 
*/
/*
    SEND: 68 16 6c 00 02 00 0b 83 01 00 01 00 6e 00 00 8b 02 00 8c 02 00 8d 02 00 
    SEND: 68 1c 6e 00 02 00 07 83 01 00 01 00 65 00 00 aa aa aa aa 80 00 00 00 00 00 ff ff ff ff 00 
    RCVD: 68 04 01 00 70 00 
    RCVD: 68 16 02 00 70 00 6b 01 06 03 01 00 00 00 00 38 49 fe 25 04 0c 0d 01 17 
    SEND: 68 16 70 00 04 00 6b 01 07 03 01 00 00 00 00 38 49 fe 25 04 0c 0d 01 17 
    RCVD: 68 0e 04 00 70 00 2d 01 06 03 01 00 88 13 00 01 
    asduHandler:RECVD ASDU type: C_SC_NA_1(45) elements: 1 
    received single command
    IOA: 5000 switch to 1
    SEND: 68 0e 72 00 06 00 2d 01 07 03 01 00 88 13 00 01 
    RCVD: 68 14 06 00 70 00 67 01 06 03 01 00 00 00 00 ff 25 04 0c 0d 01 17 
    Process time sync command with time 12:04:09 13/01/2023
    SEND: 68 14 74 00 08 00 67 01 07 03 01 00 00 00 00 00 26 04 0c 0d 01 17 

    RCVD: 68 0e 04 00 88 02 2d 01 06 00 63 00 88 13 00 01 
    asduHandler:RECVD ASDU type: C_SC_NA_1(45) elements: 1 
    received single command
    111 IOA: 5000 value: 12545
    222 IOA: 5000 value: 12545
    IOA: 5000 switch to 1
    SEND: 68 0e 88 02 06 00 2d 01 07 00 63 00 88 13 00 01 

    RCVD: 68 10 06 00 38 01 31 01 06 00 63 00 88 13 00 ff 7f 00 
    asduHandler:RECVD ASDU type: C_SE_NB_1(49) elements: 1 
    received single command : C_SE_NB_1
    555 IOA: 5000 value: 32767
    666 IOA: 5000 value: 32767
    777 IOA: 5000 switch to 1

    TI：
    Current Only Support
    C_SC_NA_1 = 45,
    C_SE_NB_1 = 49,


    COT:
    CS101_COT_ACTIVATION = 6,
    CS101_COT_ACTIVATION_CON = 7,
    CS101_COT_DEACTIVATION = 8,
    CS101_COT_DEACTIVATION_CON = 9,
    CS101_COT_ACTIVATION_TERMINATION = 10,
    CS101_COT_RETURN_INFO_REMOTE = 11,
    CS101_COT_RETURN_INFO_LOCAL = 12,
    CS101_COT_FILE_TRANSFER = 13,

    CS101_COT_INTERROGATED_BY_STATION = 20;总召

    QOS:
    设定命令限定词：S/E：0设定执行，1设定选择
*/
bool asduHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu)
{
    IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)parameter;

    TRACE("asduHandler:RECVD ASDU type: %s(%i) elements: %i \n",
            TypeID_toString(CS101_ASDU_getTypeID(asdu)),
            CS101_ASDU_getTypeID(asdu),
            CS101_ASDU_getNumberOfElements(asdu));

    if (CS101_ASDU_getTypeID(asdu) == C_SC_NA_1) //单点遥控
    { 
        TRACE("received single command : C_SC_NA_1\n");

        if (CS101_ASDU_getCOT(asdu) == CS101_COT_ACTIVATION) 
        {
            InformationObject io = CS101_ASDU_getElement(asdu, 0);
            
            TRACE("111 IOA: %i value: %i\n", InformationObject_getObjectAddress(io),  MeasuredValueScaled_getValue(io));

            if(io) 
            {
                TRACE("222 IOA: %i value: %i\n", InformationObject_getObjectAddress(io), 
                    MeasuredValueScaled_getValue(io));
                
                SingleCommand sc = (SingleCommand) io;
                BOOL bCtrlState = SingleCommand_getState(sc);
                int nIOA = InformationObject_getObjectAddress(io);

                TRACE("IOA: %i switch to %i\n", nIOA, bCtrlState);

                if (nIOA == 0x6001) 
                {
                    CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
                    On_Hander_Lock(pIEC104, bCtrlState);
                }
                else if(nIOA == 0x6002)
                {
                    CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
                    On_Hander_Reset(pIEC104, bCtrlState);
                }
                else
                {
                    CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_IOA);
                }
                InformationObject_destroy(io);
            }
            else 
            {
                TRACE("ERROR: message has no valid information object\n");
                return true;
            }
        }
        else
        {
            TRACE("--- CS101_COT_UNKNOWN_COT ---\n");
            CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_COT);
        }
        IMasterConnection_sendASDU(connection, asdu);

        return true;
    }
    else if(CS101_ASDU_getTypeID(asdu) == C_SE_NB_1)//设定比例系数值命令
    {
         TRACE("received single command : C_SE_NB_1\n");

        if (CS101_ASDU_getCOT(asdu) == CS101_COT_ACTIVATION) 
        {
            InformationObject io = CS101_ASDU_getElement(asdu, 0);
            
            TRACE("555 IOA: %i value: %i\n",InformationObject_getObjectAddress(io), 
                MeasuredValueScaled_getValue(io));

            if(io) 
            {
                int nIOA = InformationObject_getObjectAddress(io);
                int nSetValue = MeasuredValueScaled_getValue(io);

                TRACE("666 IOA: %i value: %i\n", InformationObject_getObjectAddress(io),
                    MeasuredValueScaled_getValue(io));

                TRACE("emEMSCtrlMode_REMS = [%d]\n", pIEC104->emEMSCtrlMode_REMS);
                if ((nIOA >= 0x6101) && (nIOA <= 0x6105)) 
                {
                    Handle_YT_ParamSett(pIEC104, nIOA, nSetValue);
                    CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
                }
                else if(nIOA >= 0x6201 && nIOA <= 0x623D)
                {
                    // 修改 Handle_C2S_EMS_Param 以适应能量管理计划的单点遥信（added by George Chan）
                    Handle_C2S_EMS_Param(pIEC104, nIOA, nSetValue);
                    CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
                }
                else
                {
                    CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_IOA);
                }
                InformationObject_destroy(io);
            }
            else 
            {
                TRACE("ERROR: message has no valid information object\n");
                return true;
            }
        }
        else
        {
            TRACE("--- CS101_COT_UNKNOWN_COT ---\n");
            CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_COT);
        }

        IMasterConnection_sendASDU(connection, asdu);

        return true;
    }
    else if(CS101_ASDU_getTypeID(asdu) == C_SE_NC_1)//设定短浮点值命令
    {
         TRACE("received single command : C_SE_NB_1\n");

        if (CS101_ASDU_getCOT(asdu) == CS101_COT_ACTIVATION) 
        {
            InformationObject io = CS101_ASDU_getElement(asdu, 0);
            
            TRACE("555 IOA: %i value: %i\n",InformationObject_getObjectAddress(io), 
                MeasuredValueShort_getValue(io));

            if(io) 
            {
                int nIOA = InformationObject_getObjectAddress(io);
                float fSetValue = MeasuredValueShort_getValue(io);

                TRACE("666 IOA: %i value: %i\n", InformationObject_getObjectAddress(io),
                    MeasuredValueShort_getValue(io));

                TRACE("emEMSCtrlMode_REMS = [%d]\n", pIEC104->emEMSCtrlMode_REMS);
                if(pIEC104->enableREMSCtrl && ((nIOA >= 0x6191 && nIOA <= 0x6192) || (nIOA >= 0x6391 && nIOA <= 0x6394)))
                {
                    Handle_ParamStat(pIEC104, nIOA, fSetValue);
                    CS101_ASDU_setCOT(asdu, CS101_COT_ACTIVATION_CON);
                }
                else
                {
                    CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_IOA);
                }
                InformationObject_destroy(io);
            }
            else 
            {
                TRACE("ERROR: message has no valid information object\n");
                return true;
            }
        }
        else
        {
            TRACE("--- CS101_COT_UNKNOWN_COT ---\n");
            CS101_ASDU_setCOT(asdu, CS101_COT_UNKNOWN_COT);
        }

        IMasterConnection_sendASDU(connection, asdu);

        return true;
    }
    else if(CS101_ASDU_getTypeID(asdu) == M_ME_NB_1)
    {
        TRACE("Server: received single command : M_ME_NB_1\n");

        // 能量管理计划下发错误的处理位置，将在一个月后删除相关代码（Marked by George Chan）
        int i;

        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) 
        {
            SinglePointInformation io =
                    (SinglePointInformation) CS101_ASDU_getElement(asdu, i);

            int nIOA = InformationObject_getObjectAddress(io);
            int nSetValue = MeasuredValueScaled_getValue(io);

            Handle_C2S_EMS_Param(pIEC104, nIOA, nSetValue);

            TRACE("Server Get:  IOA: %i value: %i\n", nIOA, nSetValue);

            SinglePointInformation_destroy(io);
        }

        for(i = 0; i < pIEC104->strC2S_Ems.nEmsSectorNum; i++)
        {
        	TRACE("Idx=[%d] tmStart=[%d] nStopSOC=[%d] nBattGrpPwrLmt=[%d] nActvPwrLmt=[%d] nReActvPwrLmt=[%d]\n", i, \
                pIEC104->strC2S_Ems.arryEmsParam[i].tmStart, \
                pIEC104->strC2S_Ems.arryEmsParam[i].nStopSOC,\
                pIEC104->strC2S_Ems.arryEmsParam[i].nBattGrpPwrLmt,\
                pIEC104->strC2S_Ems.arryEmsParam[i].nActvPwrLmt,\
                pIEC104->strC2S_Ems.arryEmsParam[i].nReActvPwrLmt);
        }
        BOOL bNeedUpdate = Compare_IfNeedUpdate_EMSParam(pIEC104);
        
        if(bNeedUpdate == TRUE)
        {
            pIEC104->bTrigger = TRUE;
            Update_EMSParam2Local(pIEC104);
        }
        else
        {
            TRACE("!!! Warning:Same EMS Plan, NoNeed Update!!!\n");
        }

    }
    return false;
}

bool connectionRequestHandler(void* parameter, const char* ipAddress)
{
    IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)parameter;

    TRACE("New connection request from %s\n", ipAddress);

    LogOut(MAIN_MODULE_REPORTER, "IEC-104", LOG_TYPE_INFO, "ConctReq:ip[%s]Statu=[%d][Open|Close|Act|DeAct]", \
		ipAddress, pIEC104->em104ConnStatus);

#if 0
    if (strcmp(ipAddress, "127.0.0.1") == 0) {
        printf("Accept connection\n");
        return true;
    }
    else {
        printf("Deny connection\n");
        return false;
    }
#else
    return true;
#endif
}




void connectionEventHandler(void* parameter, IMasterConnection con, CS104_PeerConnectionEvent event)
{
    IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH *)parameter;

    if (event == CS104_CON_EVENT_CONNECTION_OPENED) 
    {
        pIEC104->em104ConnStatus = STAT_Connect_Opened;
        TRACE("Connection opened (%p)\n", con);
    }
    else if (event == CS104_CON_EVENT_CONNECTION_CLOSED) 
    {
        pIEC104->em104ConnStatus = STAT_Connect_Closed;
        TRACE("Connection closed (%p)\n", con);
    }
    else if (event == CS104_CON_EVENT_ACTIVATED) 
    {
        pIEC104->em104ConnStatus = STAT_Activated;
        pIEC104->bTrigger = TRUE;
        TRACE("Connection activated (%p)\n", con);
    }
    else if (event == CS104_CON_EVENT_DEACTIVATED) 
    {
        pIEC104->em104ConnStatus = STAT_DeActivated;
        TRACE("Connection deactivated (%p)\n", con);
    }

    LogOut(MAIN_MODULE_REPORTER, "IEC-104", LOG_TYPE_INFO, "ConctEvtHand:event[%d] Statu=[%d]", \
		event, pIEC104->em104ConnStatus);
    return;
}
 
 

