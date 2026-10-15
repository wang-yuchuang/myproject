/********************************************************************************
* Copyright(c) Shenzhen Infypower Co., Ltd.
* @author
* @brief
* @history
1.2024-12-02 增加5001H,Handle_ParamStat
*******************************************************************************/

#include "../reporters_base.h"
#include "iec_104.h"

/*********************************************************************************
*  
*  FUNCTION NAME :  Event_Handler()
*  CREATOR       : John
*  CREATED DATE  : 2022-12-30 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	    emEvType,
	_EVENT_MSG	    emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			    nMaxBlockMSec)
{
	UNUSED(bBlockMode);
	UNUSED(nMaxBlockMSec);
	
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	// int n = 0;

	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response

	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			//TRACE("Info:Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			// EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
 		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("IEC104_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			// EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
		
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("IEC104_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
			// EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_DIAGNOUPLOAD_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
		case EVENT_TYPE_SYSTEM_EXIT:
		{
			EVENT_SYSTEM_EXIT* pEvReboot = (EVENT_SYSTEM_EXIT*)pEvData;
			if(nEvDataSize != sizeof(EVENT_SYSTEM_EXIT))
			{
				return EVENT_HANDED_FAILED;
			}
			TRACEX("IEC104_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;
		
		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACEX("104_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			if ((pParamChg->parmPTR.nId == 1) && (pParamChg->parmPTR.nSubId == 26))
			{
				TRACEX("Config Changed Evevt \n");
				valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
				valGet.val.blobValue = (BYTE*)&pIEC104->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: OCPP settings can not be retrieved\n");
					return EVENT_HANDED_FAILED;
				}
				pIEC104->bTrigger = TRUE;
			}
			else if((pParamChg->parmPTR.nId == 500) && (pParamChg->parmPTR.nSubId == 502))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					500, 502, &valGet, sizeof(PARAM_VAL_GET));
				pIEC104->nCfgBMSNum = valGet.val.ulValue;
				if(pIEC104->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
				{
					pIEC104->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
				}
				TRACEX("nId=[%d] nSubId=[%d] nCfgBMSNum=[%d]\n", pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, pIEC104->nCfgBMSNum);
			}
			else if((pParamChg->parmPTR.nId == 200) && (pParamChg->parmPTR.nSubId == 201))
			{
				valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
				pIEC104->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
				valGet.val.blobValue = (BYTE*)&pIEC104->emsPlan;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					200, 201, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: EMS Mgmt Plan Get Failed!\n");
					return EVENT_HANDED_FAILED;
				}
				pIEC104->bTrigger = TRUE;
			}
			else if ((pParamChg->parmPTR.nId == 1001) &&
				(pParamChg->parmPTR.nSubId == 3300 || pParamChg->parmPTR.nSubId == 3303 || pParamChg->parmPTR.nSubId == 3310))
			{
				valGet.nGetSize = 4;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
					pParamChg->parmPTR.nId, pParamChg->parmPTR.nSubId, &valGet, sizeof(PARAM_VAL_GET));
				switch (pParamChg->parmPTR.nSubId)
				{
				case 3300: // 1001, 3300,
					pIEC104->nTDisconDelayIOA6191 = valGet.val.ulValue;
					break;
				case 3303: // 1001, 3303,
					pIEC104->enableREMSCtrl = (BOOL)((err==ERRCODE_DAI_OK) ? valGet.val.emValue : FALSE);
					break;
				case 3310: // 1001, 3310,
					pIEC104->nLocalEMSInfoReportingCycle = valGet.val.ulValue;
					break;
				default:
					break;
				}
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}



/* Callback handler to log sent or received messages (optional) */
/*
*DATE:2023-01-08
*Creator:John
*Func:
*Discription: RCVD: 68 04 01 00 d8 03 
            SEND: 68 10 d8 03 00 00 0b 01 01 00 01 00 6e 00 00 3f 7c 00 

            RCVD: 68 04 01 00 20 0a 
            SEND: 68 10 20 0a 0e 00 0b 01 01 00 01 00 6e 00 00 b0 8a 00 

*/
static void rawMessageHandler(void* parameter, IMasterConnection conneciton, uint8_t* msg, int msgSize, bool sent)
{
	UNUSED(conneciton);
    IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH *)parameter;
    REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
    char szDump[512]={0};
 
    UINT nOffset = 0;
    
    /*if (sent)
    {
        printf("Server SEND: ");
    }
    else
    {
        printf("Server RCVD: ");
    }*/

    int i;
    for (i = 0; i < msgSize; i++) 
    {
		if(msgSize > 256)
		{
			break;
		}
        //printf("%02x ", msg[i]);
        nOffset  += sprintf(szDump + nOffset, "%02X",msg[i]);
    }
    //printf("\n");
    szDump[nOffset] ='\0';

	//printf("Current: %s, Last: %s, Len:%d\n",&szDump[12], &pIEC104->szLastRecvData[12], (nOffset-12));

	if(strncmp(&szDump[12], &pIEC104->szLastRecvData[12], (nOffset-12)) == 0)//Recv Some Str,Do not Save.
	{
		//printf("Warning: Some String szDump=[%s], szLastRecvData=[%s]\n", szDump, pIEC104->szLastRecvData);
		return;
	}

	memcpy(pIEC104->szLastRecvData, szDump, nOffset);

	// DUMP_ESiOCPPMSG(pReporter, (sent == TRUE) ? DUMP_TYPE_WRITE : DUMP_TYPE_READ, szDump);//edit by kuangyue 20250313
	LOG_INFO("IEC_104 pReporter=[%d], type=[%s], Data=\n[%s]",
		pReporter->info.nReporterID,
		(sent == TRUE) ? "WRITE" : "READ",
		szDump);

    return;
}

static BOOL Wait_IEC104_CommReady(REPORTER* pReporter)
{
 	IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)pReporter->pDataHeap;

	if(pIEC104->em104ConnStatus == STAT_Activated)
	{	
		pIEC104->bDisconTrigger = FALSE;
		// Sleep(1000);
		return TRUE;
	}
	else 
	{
		return FALSE;
	}

	return FALSE;
}

static void Handle_IEC104CommStatus(IEC104_DATA_ROUGH* pIEC104)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	VAR_VALUE varV;
	static IEC104_CONN_STATUS emLastCommStatu = STAT_DisConnected;

	if(pIEC104->em104ConnStatus != emLastCommStatu)
	{
		TRACEX("em104ConnStatus=[%d] emLastCommStatu=[%d]\n", pIEC104->em104ConnStatus, emLastCommStatu);
		
		if (emLastCommStatu == STAT_Activated && pIEC104->em104ConnStatus != STAT_Activated)
		{
			pIEC104->tmLastDisconnect = _NOW_;
			pIEC104->bDisconTrigger = TRUE;
		}

		emLastCommStatu = pIEC104->em104ConnStatus;

		varV.emValue = (pIEC104->em104ConnStatus == STAT_Activated) ? 0 : 1;
		TRACEI("104-server_commom_status [%d]",varV.emValue);
		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 5, &varV, 4, 100);

		pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1, 31, &varV, 4, 100);
	}
	
	return;
}

/*
从站发送
68 16 92 01 00 00 0b 83 01 01 01 00 01 40 00 73 00 00 7e 00 00 89 00 80 
起始字节=68 数据单元长度(APDU)=22 I格式帧 发送序号(NS)=201 接收序号(NR)=0 TI= 11 VSQ=83 SQ=1 INFONUM=3 COT= 101 T=0 PN=0 CAUSE =1 COA =1 M_ME_NB_1
带品质描述的测量值, 标度化值        肯定认可    自定义原因     
 变化有效 IV=0  当前值 NT=-24117248  未被取代 SB=0  未被封锁 BL=0 点号=16385 OV=0 未溢出  值=115
 变化有效 IV=0  当前值 NT=-24117248  未被取代 SB=0  未被封锁 BL=0 点号=16386 OV=0 未溢出  值=126
 变化无效 IV=1  当前值 NT=-24117120  未被取代 SB=0  未被封锁 BL=0 点号=16387 OV=0 未溢出  值=137
*/
/*********************************************************************************
*  
*  FUNCTION NAME :  Create_YCSysInfo_EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-31 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Create_YCSysInfo_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	//创建一个新的CS101_ASDU实例（使用CS101_COT_PERIODIC定期数据和CS101_COT_SPONTANEOUS自发数据）
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4051, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_Sys;

	for (n = 0; n < pIEC104->nYCNum_Sys; n++)
	{
		pYCItem_Sys = pIEC104->pYCEle_Sys + n;
 		if(pYCItem_Sys == NULL)
		{
			//printf("pYCItem_Sys == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYCItem_Sys->ptrValue);
		tempVal = tempVal*((int)pYCItem_Sys->nScale);
		
		//TRACEX("Create_YCSysInfo_EnqueueASDU Addr[%d] tempVal=%f\n ", pYCItem_Sys->nIOA, tempVal);
		
		//创建一个包含要发送数据的新信息对象实例
		InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_Sys->nIOA, tempVal, pYCItem_Sys->emQuality);
		
		//将新的信息对象添加到ASDU
		CS101_ASDU_addInformationObject(newAsdu, io);

		//释放信息对象内存
		InformationObject_destroy(io);
	}

	//将ASDU放入数据队列中进行传输:Add ASDU to slave event queue
	CS104_Slave_enqueueASDU(slave, newAsdu);

	//释放ASDU内存
	CS101_ASDU_destroy(newAsdu);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Create_YCEms_EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-31 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Create_YCEms_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	//创建一个新的CS101_ASDU实例（使用CS101_COT_PERIODIC定期数据和CS101_COT_SPONTANEOUS自发数据）
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4001, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_Ems;

    int nRealEmsSectorNum = MIN((pIEC104->strYC_Ems.nEmsSectorNum*5+1), pIEC104->nYCNum_Ems);
    
    TRACE("nRealEmsSectorNum=[%d] nYCNum_Ems=[%d]\n", nRealEmsSectorNum, pIEC104->nYCNum_Ems);

	for (n = 0; n < nRealEmsSectorNum; n++)
	{
		pYCItem_Ems = pIEC104->pYCEle_Ems + n;
 		if(pYCItem_Ems == NULL)
		{
			//printf("pYCItem_Ems == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYCItem_Ems->ptrValue);
		tempVal = tempVal*((int)pYCItem_Ems->nScale);
		
		//TRACE("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_Ems->nIOA, tempVal);
		
		//创建一个包含要发送数据的新信息对象实例
		InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, pYCItem_Ems->nIOA, tempVal, pYCItem_Ems->emQuality);
		
		//InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_Ems->nIOA, tempVal, pYCItem_Ems->emQuality);

		//将新的信息对象添加到ASDU
		CS101_ASDU_addInformationObject(newAsdu, io);

		//释放信息对象内存
		InformationObject_destroy(io);
	}

	//将ASDU放入数据队列中进行传输:Add ASDU to slave event queue
	CS104_Slave_enqueueASDU(slave, newAsdu);

	//释放ASDU内存
	CS101_ASDU_destroy(newAsdu);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Create_YCCabMeter_EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-1-14 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCCabMeter_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	if(pIEC104->strYC_SysInfo.emCabMeter_CommStatu == 1) //2023-07-17 langxin Request CommFail Not Send Value.
	{
		return;
	}

	int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4101, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_CabMeter;

	for (n = 0; n < pIEC104->nYCNum_CabMeter; n++)
	{
		pYCItem_CabMeter = pIEC104->pYCEle_CabMeter + n;
		if(pYCItem_CabMeter == NULL)
		{
			TRACE("pYCItem_CabMeter == NUL\n");
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	

/*********************************************************************************
*  
*  FUNCTION NAME : Create_YCGridMeter_EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCGridMeter_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	if(pIEC104->strYC_SysInfo.emGridMeter_CommStatu == 1) //2023-07-17 langxin Request CommFail Not Send Value.
	{
		return;
	}
	
	int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4201, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_GridMeter;

	for (n = 0; n < pIEC104->nYCNum_GridMeter; n++)
	{
		pYCItem_GridMeter = pIEC104->pYCEle_GridMeter + n;
		if(pYCItem_GridMeter == NULL)
		{
			TRACE("pYCItem_GridMeter == NUL\n");
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Battery Group1 EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-1-14 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCBatG1_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	if(pIEC104->strYC_BAT[0].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4301, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Battery Group2 EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCBatG2_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	if(pIEC104->strYC_BAT[1].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4321, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	


/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Battery Group2 EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCBatG3_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;
	if(pIEC104->strYC_BAT[2].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4341, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	


/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Battery Group2 EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCBatG4_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	if(pIEC104->strYC_BAT[3].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4361, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC PCS EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCPCS_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x4501, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Cell Group1 EnqueueASDU()
*  CREATOR       : XSY
*  CREATED DATE  : 2025-3-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCCellG1_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams, int id)
{
	int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[0].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0xA001 + offset, 1, false, false);
	
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
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG1->nIOA, tempVal, pYCItem_CellG1->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Cell Group2 EnqueueASDU()
*  CREATOR       : XSY
*  CREATED DATE  : 2025-3-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCCellG2_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams, int id)
{
	int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[1].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0xA1E1 + offset, 1, false, false);
	
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
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG2->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG2->nIOA, tempVal, pYCItem_CellG2->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Cell Group3 EnqueueASDU()
*  CREATOR       : XSY
*  CREATED DATE  : 2025-3-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCCellG3_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams, int id)
{
	int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[2].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0xA3C1 + offset, 1, false, false);
	
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
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG3->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG3->nIOA, tempVal, pYCItem_CellG3->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Cell Group4 EnqueueASDU()
*  CREATOR       : XSY
*  CREATED DATE  : 2025-3-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCCellG4_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams, int id)
{
	int n = 0;
	int offset = 0;
	offset = id * 48;

	if(pIEC104->strYC_BAT[3].fRunState < STBATT_RUNSTAT_COMM_FAIL)
	{
		return;
	}

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0xA5A1 + offset, 1, false, false);
	
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
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_CellG4->nIOA, tempVal);
		
		InformationObject io = (InformationObject) MeasuredValueShort_create(NULL, pYCItem_CellG4->nIOA, tempVal, pYCItem_CellG4->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}
/*********************************************************************************
*  
*  FUNCTION NAME : Create YC Cell Group EnqueueASDU()
*  CREATOR       : XSY
*  CREATED DATE  : 2025-3-24 10:13:53
*  DESCRIPTION   :  240个电芯的单体信息：温度+电压 =480个，float型占4字节，信息体243/(4+1)=48，需10个APDU传送,需要循环创建
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCCellGrp_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int i=0;
	for(i=0; i<10; i++)
	{
		Create_YCCellG1_EnqueueASDU(pIEC104, slave, alParams, i);
		Create_YCCellG2_EnqueueASDU(pIEC104, slave, alParams, i);
		Create_YCCellG3_EnqueueASDU(pIEC104, slave, alParams, i);
		Create_YCCellG4_EnqueueASDU(pIEC104, slave, alParams, i);
	}

	return;
}



/*********************************************************************************
*  
*  FUNCTION NAME : Create YCKWh EnqueueASDU()
*  CREATOR       : cjj
*  CREATED DATE  : 2025-2-11 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCLocalSys_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x5001, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_LocalSys;

	for (n = 0; n < pIEC104->nYCNum_LocalSystem; n++)
	{
		pYCItem_LocalSys = pIEC104->pYCEle_LocalSystem + n;
		if(pYCItem_LocalSys == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_LocalSys->ptrValue);
		tempVal = tempVal*((float)pYCItem_LocalSys->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_LocalSys->nIOA, tempVal);
		
		InformationObject io = (InformationObject)MeasuredValueShort_create(NULL, pYCItem_LocalSys->nIOA, tempVal, pYCItem_LocalSys->emQuality);
		// printf("ioa = [%d], value = [%f], quality = [%d]\n", pYCItem_LocalSys->nIOA, tempVal, pYCItem_LocalSys->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Create YCKWh EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-5-24 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Create_YCKWh_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	Refresh_YC_KWh_DataMap(pIEC104);

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x6401, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}	


/*********************************************************************************
*  
*  FUNCTION NAME :  Create_YX_EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-1-14 10:13:53
*  DESCRIPTION   :
*  从站发送
*  68 10 2c 03 00 00 01 83 01 00 01 00 00 00 00 00 01 80 
*  起始字节=68 数据单元长度(APDU)=16 I格式帧 发送序号(NS)=406 接收序号(NR)=0 TI= 1 VSQ=83 SQ=1 INFONUM=3 COT= 0001 T=0 PN=0 CAUSE =1 COA =1 M_SP_NA_1
*  单点遥信变位          肯定认可    周期、循环 
*   变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=0 分
*   变位有效 IV=0  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=1 合
*   变位无效 IV=1  当前值 NT=0  未被取代 SB=0  未被封锁 BL=0 点号=2 分
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Create_YX_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;
	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x01, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYXItem;

	for (n = 0; n < pIEC104->nYXNum; n++)
	{
		pYXItem = pIEC104->pYXEle + n;
		//printf("n=[%d] pIEC104->nYXNum=[%d]\n", n, pIEC104->nYXNum);
		if(pYXItem == NULL)
		{
			TRACE("pYXItem == NUL\n");
			break;
		}

		int tempVal =  *((int*)pYXItem->ptrValue);
 		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYXItem->nIOA, tempVal);
		
 		InformationObject io = (InformationObject) SinglePointInformation_create(NULL, pYXItem->nIOA, tempVal, pYXItem->emQuality);
		
		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

static void Create_YXBms_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;
	int nBattIdx = 0;
	// BmsPart1
	// printf("BMSNUM == %d\n", pIEC104->nCfgBMSNum);
	for(nBattIdx=0; nBattIdx < pIEC104->nCfgBMSNum; ++nBattIdx)
	{
		// if(pIEC104->strYC_BAT[nBattIdx].fRunState < STBATT_RUNSTAT_COMM_FAIL)
		// {
		// 	break;
		// }

		/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
		CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x1001 + nBattIdx*0x100, 1, false, false);
		
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
	
		CS104_Slave_enqueueASDU(slave, newAsdu);
	
		CS101_ASDU_destroy(newAsdu);

	}

	// BmsPart2
	for(nBattIdx=0; nBattIdx < pIEC104->nCfgBMSNum; ++nBattIdx)
	{
		// if(pIEC104->strYC_BAT[nBattIdx].fRunState < STBATT_RUNSTAT_COMM_FAIL)
		// {
		// 	break;
		// }

		/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
		CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x1081 + nBattIdx*0x100, 1, false, false);
		
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
	
		CS104_Slave_enqueueASDU(slave, newAsdu);
	
		CS101_ASDU_destroy(newAsdu);

	}

	return;
}

static void Create_YXPcsTotal_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;
	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x2001, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

static void Create_YXPcsGrp_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;
	int nBattIdx = 0;
	// printf("pIEC104->nCfgBMSNum = %d\n", pIEC104->nCfgBMSNum);
	for(nBattIdx=0; nBattIdx < pIEC104->nCfgBMSNum; ++nBattIdx)
	{
		/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
		CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x2101 + nBattIdx*0x100, 1, false, false);
		
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
	
		CS104_Slave_enqueueASDU(slave, newAsdu);
	
		CS101_ASDU_destroy(newAsdu);

	}

	return;
}

static void Create_YXSTS_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	int n = 0;

	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue != LIQUID_COOLING_ADD_STS)
	{
		return;
	}

	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x2501, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

static void Create_YXFire_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;
	/*!  以下用M_SP_NA_1（SinglePointInformation_create），创建连续数据 */
	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x2601, 1, false, false);
	
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

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

// YC Air
static void Create_YCAir_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	int n = 0;

	CS101_ASDU newAsdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0x5051, 1, false, false);
	
	IEC104_CONFIG_SIGMAP *pYCItem_Air;

	for (n = 0; n < pIEC104->nYCNum_AirInfo; n++)
	{
		pYCItem_Air = pIEC104->pYCEle_AirInfo + n;
		if(pYCItem_Air == NULL)
		{
			break;
		}

		float tempVal =  *((float*)pYCItem_Air->ptrValue);
		tempVal = tempVal*((float)pYCItem_Air->nScale);
		
		//TRACEX("Get Config Obj Addr[%d] tempVal=%f\n ", pYCItem_Air->nIOA, tempVal);
		
		InformationObject io = (InformationObject)MeasuredValueShort_create(NULL, pYCItem_Air->nIOA, tempVal, pYCItem_Air->emQuality);
		// printf("ioa = [%d], value = [%f], quality = [%d]\n", pYCItem_Air->nIOA, tempVal, pYCItem_Air->emQuality);

		CS101_ASDU_addInformationObject(newAsdu, io);

		InformationObject_destroy(io);
	}

	CS104_Slave_enqueueASDU(slave, newAsdu);

	CS101_ASDU_destroy(newAsdu);

	return;
}

//-- 2023-01-12 John Stuff Self Define DataObject Buffer Start --//
// static void Create_SelfBuffer_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
// {
// 	//char buffer[256]="\0";
// 	//CS101_ASDU newAsdu = CS101_ASDU_createFromBuffer(alParams, buffer, strlen(buffer));
 
// 	//CS104_Slave_enqueueASDU(slave, newAsdu);

// 	//CS101_ASDU_destroy(newAsdu);

// 	return;
// }

//SEND isSequence=false: 68 22 12 00 00 00 07 03 01 00 01 00 65 00 00 aa aa aa aa 80 66 00 00 00 00 00 00 00 67 00 00 ff ff ff ff 00 
//起始字节=68 数据单元长度(APDU)=34 I格式帧 发送序号(NS)=9 接收序号(NR)=0 
//TI= 7 VSQ=03 SQ=0 INFONUM=3 COT= 01 T=0 PN=0 CAUSE =1 COA =1 M_BO_NA_1
//SEND isSequence=true: 68 1c 78 00 00 00 07 83 01 00 01 00 65 00 00 aa aa aa aa 80 00 00 00 00 00 ff ff ff ff 00 
//----------- 2023-01-12 John Test Multi Squence Data Start------------//
// static void Create_BS32Str_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
// {
// 	uint8_t buffer[256];
// 	struct sBufferFrame bf;

// 	Frame f = BufferFrame_initialize(&bf, buffer, 0);

// 	CS101_ASDU asdu = CS101_ASDU_create(alParams, true, CS101_COT_PERIODIC, 0, 1, false, false);
// 	BitString32 bs32_1 = BitString32_createEx(NULL, 101, 	(uint32_t) 0xaaaaaaaa, IEC60870_QUALITY_INVALID);
// 	BitString32 bs32_2 = BitString32_create(NULL, 	102,   	(uint32_t) 0x00000000);
// 	BitString32 bs32_3 = BitString32_create(NULL, 	103,   	(uint32_t) 0xffffffff);

// 	CS101_ASDU_addInformationObject(asdu, (InformationObject) bs32_1);
// 	CS101_ASDU_addInformationObject(asdu, (InformationObject) bs32_2);
// 	CS101_ASDU_addInformationObject(asdu, (InformationObject) bs32_3);

// 	CS101_ASDU_encode(asdu, f);

// 	CS104_Slave_enqueueASDU(slave, asdu);

// 	InformationObject_destroy((InformationObject) bs32_1);
// 	InformationObject_destroy((InformationObject) bs32_2);
// 	InformationObject_destroy((InformationObject) bs32_3);

// 	CS101_ASDU_destroy(asdu);

// 	return;
// }
 
/*********************************************************************************
*  
*  FUNCTION NAME : Handle_YC_YX_EnqueueASDU()
*  CREATOR       : John
*  CREATED DATE  : 2023-6-1 10:13:53
*  DESCRIPTION   :   
*  
*  CHANGE HISTORY:
*  
***********************************************************************************/
static void Handle_YC_YX_EnqueueASDU(IEC104_DATA_ROUGH* pIEC104, CS104_Slave slave, CS101_AppLayerParameters alParams)
{
	REPORTER* pReporter = (REPORTER*)pIEC104->hReporter;
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;

	time_t tmNow = _NOW_;

	//Added By John 20231009.if On_Interrogating, Do not Send Cycle Msg.
	if(pIEC104->bOn_Interrogating == TRUE)
	{
		TRACE("Warning:--- On_Interrogating ---\n");
		return;
	}

	if(pIEC104->bTrigger == TRUE)
	{
		pIEC104->bTrigger = FALSE;

		if (pIEC104->bTriggerEmsYC == TRUE)
		{
			pIEC104->bTriggerEmsYC = FALSE;
		}
		else
		{
			Create_YX_EnqueueASDU(pIEC104, slave, alParams);
			Create_YXBms_EnqueueASDU(pIEC104, slave, alParams);
			Create_YXPcsTotal_EnqueueASDU(pIEC104, slave, alParams);
			Create_YXPcsGrp_EnqueueASDU(pIEC104, slave, alParams);
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 1, 101, &valGet, sizeof(PARAM_VAL_GET));
			if(valGet.val.emValue == LIQUID_COOLING_ADD_STS)
			{
				Create_YXSTS_EnqueueASDU(pIEC104, slave, alParams);
			}
			Create_YXFire_EnqueueASDU(pIEC104, slave, alParams);
		}

		Create_YCEms_EnqueueASDU(pIEC104, slave, alParams);

		TRACE("----------bTrigger == TRUE -------------\n");
	}

	//if((ABS(tmNow - pIEC104->tmLastSent_Sys) >= 1))
	//{
	//	pIEC104->tmLastSent_Sys = tmNow;
	//	
	//}

	if ((ABS(tmNow - pIEC104->tmLastSent_Sys) >= 1))
	{
		TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_Sys=[%ld]\n", (tmNow - pIEC104->tmLastSent_Sys), tmNow, pIEC104->tmLastSent_Sys);

		pIEC104->tmLastSent_Sys = tmNow;

		Create_YCSysInfo_EnqueueASDU(pIEC104, slave, alParams);
		Create_YCAir_EnqueueASDU(pIEC104, slave, alParams);
	}
	

	if((ABS(tmNow - pIEC104->tmLastSent_Meter) >= MAX(2, pIEC104->blobBackendSet.nCmd_107_Interval)))
	{
		TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_Meter=[%ld]\n", (tmNow - pIEC104->tmLastSent_Meter), tmNow, pIEC104->tmLastSent_Meter);

		pIEC104->tmLastSent_Meter = tmNow;
		
		Create_YCCabMeter_EnqueueASDU(pIEC104, slave, alParams);
	
		Create_YCGridMeter_EnqueueASDU(pIEC104, slave, alParams);
	}

	if((ABS(tmNow - pIEC104->tmLastSent_BMS) >= MAX(2, pIEC104->blobBackendSet.nCmd_103_Interval)))
	{
		TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_BMS=[%ld]\n", (tmNow - pIEC104->tmLastSent_BMS), tmNow, pIEC104->tmLastSent_BMS);

		pIEC104->tmLastSent_BMS = tmNow;
		
		Create_YCBatG1_EnqueueASDU(pIEC104, slave, alParams);

		Create_YCBatG2_EnqueueASDU(pIEC104, slave, alParams);

		Create_YCBatG3_EnqueueASDU(pIEC104, slave, alParams);

		Create_YCBatG4_EnqueueASDU(pIEC104, slave, alParams);
	}

	if((ABS(tmNow - pIEC104->tmLastSent_PCS) >= MAX(2, pIEC104->blobBackendSet.nCmd_105_Interval)))
	{
		TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_PCS=[%ld]\n", (tmNow - pIEC104->tmLastSent_PCS), tmNow, pIEC104->tmLastSent_PCS);

		pIEC104->tmLastSent_PCS = tmNow;
		
		Create_YCPCS_EnqueueASDU(pIEC104, slave, alParams);
	}

	if((ABS(tmNow - pIEC104->tmLastSent_KWh) >= MAX(10, pIEC104->blobBackendSet.nCmd_204_Interval)))
	{
		TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_KWh=[%ld]\n", (tmNow - pIEC104->tmLastSent_KWh), tmNow, pIEC104->tmLastSent_KWh);

		pIEC104->tmLastSent_KWh = tmNow;
		
		Create_YCKWh_EnqueueASDU(pIEC104, slave, alParams);
	}
	
	if(pIEC104->enableREMSCtrl && ABS(tmNow - pIEC104->tmLastSent_LocalSystem) >= pIEC104->nLocalEMSInfoReportingCycle)
	{
		TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_LocalSystem=[%ld]\n", (tmNow - pIEC104->tmLastSent_LocalSystem), tmNow, pIEC104->tmLastSent_LocalSystem);

		pIEC104->tmLastSent_LocalSystem = tmNow;
		
		Create_YCLocalSys_EnqueueASDU(pIEC104, slave, alParams);
	}

	// YNT need cell volt and temp
	if(pIEC104->bNeedUploadCellInfo)
	{
		if ((ABS(tmNow - pIEC104->tmLastSent_Cell) >= 20))
		{
			TRACE("OffSet=[%d] tmNow=[%ld] tmLastSent_Cell=[%ld]\n", (tmNow - pIEC104->tmLastSent_Cell), tmNow, pIEC104->tmLastSent_Cell);
	
			pIEC104->tmLastSent_Cell = tmNow;
	
			Create_YCCellGrp_EnqueueASDU(pIEC104, slave, alParams);
		}
	}
	// YNT over

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  _ThreadEntry_IEC104Comm
*  CREATOR       :  
*  CREATED DATE  : 2023-01-11 11:13:53
*  DESCRIPTION   :  
*  CHANGE HISTORY: 
*  
*  默认模式（CS104_MODE_SINGLE_REDUNDANCY_GROUP）仅允许单个活动客户端连接。
*  活动客户端连接是发送ASDU（应用程序数据单元）的连接。所有其他连接只是不发送应用程序层数据的备用连接。
*  事件只有一个队列。当没有客户端连接或没有连接处于活动状态时，也会存储事件。
  
*  第二种模式（CS104_MODE_CONNECTION_IS_REDUNDANCY_GROUP）允许多个活动客户端连接。
*  每个连接都有其自己的事件队列。关闭客户端连接后，事件队列将被删除。
*  当多个客户端必须访问应用程序数据时，可以使用此模式。此模式易于使用。
*  但是此模式的缺点是，当没有客户端连接时事件会丢失。
  
*  第三种模式（CS104_MODE_MULTIPLE_REDUNDANCY_GROUPS）允许多个活动客户端连接，
*  同时保留未连接任何客户端时的事件。在这种模式下，可以将客户端分配给特定的冗余组。
*  分配基于客户端的IP地址。冗余组可以有多个同时连接，
*  但是这些连接中只有一个可以处于活动状态。激活的连接数受冗余组数的限制。
*  每个冗余组都有一个专用的事件队列。
    
*  在CS 104从站中，队列大小由CS104_Slave_create函数的maxLowPrioQueueSize参数确定。
*  如果将maxLowPrioQueueSize参数设置为零，则队列将始终具有CONFIG_SLAVE_MESSAGE_QUEUE_SIZE定义的大小。
*  第二个参数maxHighPrioQueueSize确定高优先级数据队列的大小。放入此队列的消息会绕过低优先级队列的消息。
*  高优先级队列用于库回调处理程序中的请求响应。

*  InterrogationHandler: 	询问请求
*  clockSyncHandler：		收到时钟同步消息
*  ASDUHandler：			ASDU已收到但未由其他回调处理程序之一处理
*  ConnectionRequestHandler：一个新的TCP / IP客户端尝试连接
***********************************************************************************/
static DWORD _ThreadEntry_IEC104Comm(void* pArg)
{
	TRACE("-----------_ThreadEntry_IEC104Comm: Start Enter------- \n");
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
 	IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)pReporter->pDataHeap;

	CS104_Slave slave = CS104_Slave_create(50, 50);

	CS104_Slave_setLocalAddress(slave, "0.0.0.0");

	 /* Set mode to a single redundancy group
     * NOTE: library has to be compiled with CONFIG_CS104_SUPPORT_SERVER_MODE_SINGLE_REDUNDANCY_GROUP enabled (=1)
     */
    CS104_Slave_setServerMode(slave, CS104_MODE_SINGLE_REDUNDANCY_GROUP);

    /* get the connection parameters - we need them to create correct ASDUs -
     * you can also modify the parameters here when default parameters are not to be used */
    CS101_AppLayerParameters alParams = CS104_Slave_getAppLayerParameters(slave);

    /* when you have to tweak the APCI parameters (t0-t3, k, w) you can access them here */
    CS104_APCIParameters apciParams = CS104_Slave_getConnectionParameters(slave);

    //printf("APCI parameters:\n");
    //printf("  t0: %i\n", apciParams->t0);	//t0: 10
    //printf("  t1: %i\n", apciParams->t1);	//t1: 15
    //printf("  t2: %i\n", apciParams->t2);	//t2: 10
    //printf("  t3: %i\n", apciParams->t3);	//t3: 20
    //printf("  k: %i\n", apciParams->k);	//k: 18 John 20240411 Midify K to 18.
    //printf("  w: %i\n", apciParams->w);	//w: 12


    /* set the callback handler for the clock synchronization command */
    CS104_Slave_setClockSyncHandler(slave, clockSyncHandler, pIEC104);

    /* set the callback handler for the interrogation command */
    CS104_Slave_setInterrogationHandler(slave, interrogationHandler, pIEC104);

    /* set handler for other message types */
    CS104_Slave_setASDUHandler(slave, asduHandler, pIEC104);

    /* set handler to handle connection requests (optional) */
    CS104_Slave_setConnectionRequestHandler(slave, connectionRequestHandler, pIEC104);

    /* set handler to track connection events (optional) */
    CS104_Slave_setConnectionEventHandler(slave, connectionEventHandler, pIEC104);

    /* uncomment to log messages */
    CS104_Slave_setRawMessageHandler(slave, rawMessageHandler, pIEC104);

    //CS104启动/停止服务器,配置服务器后，可以使用CS104_Slave_start函数启动它。
    //此函数启动一个新的后台线程，该线程正在侦听传入的客户端连接。
    CS104_Slave_start(slave);

	if (CS104_Slave_isRunning(slave) == false) 
	{
		TRACEX("Starting server failed!\n");
		pReporter->bExit = TRUE;
	}
	
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		
		TRACEX("------ OnWaiting_IEC104_CommReady ------\n");

		Handle_IEC104CommStatus(pIEC104);

		if(pIEC104->enableREMSCtrl &&
			pIEC104->bDisconTrigger &&	// 检测到断开过连接
			// pIEC104->emEMSCtrlMode_REMS == EMSCtrlMode_Remote &&
			pIEC104->em104ConnStatus != STAT_Activated)	// 当前也还是断开连接状态
		{
			
			if (abs(_NOW_ - pIEC104->tmLastDisconnect) > pIEC104->nTDisconDelayIOA6191)	// 掉线 nTDisconDelayIOA6191 s ，将 IOA 为 0x6191 的参数设置状态清零)
			{
				Handle_ParamStat(pIEC104, 0x6191, 0);
				VAR_VALUE varV;
				
				if(pIEC104->emEMSCtrlMode_REMS == EMSCtrlMode_Remote)
				{
					varV.emValue = EMSCtrlMode_Local;
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						1001, 3902, &varV, 4, 100);
					pIEC104->emEMSCtrlMode_REMS = EMSCtrlMode_Local;

					varV.emValue = 1; // abnormal 
					pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
						1001, 3908, &varV, 4, 100);
				}
				pIEC104->bDisconTrigger = FALSE;
				pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
					1001, 3902, &varV, 4, 100);
			}
		}
		else if(pIEC104->enableREMSCtrl &&
			// pIEC104->emEMSCtrlMode_REMS == EMSCtrlMode_Remote &&
			pIEC104->em104ConnStatus == STAT_Activated)
		{
			VAR_VALUE varV;
			varV.emValue = 0; // normal 
			pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_VIRTUAL_PARAMVALUE_W, 
				1001, 3908, &varV, 4, 100);
		}

		if(!Wait_IEC104_CommReady(pReporter))
		{
			Sleep(1000);
			continue;
		}

		TRACEX("Comm OK 222 scaledValue ---\n");
	

		Refresh_YC_YX_SigMapValue(pIEC104);

		Handle_YC_YX_EnqueueASDU(pIEC104, slave, alParams);

		TRACE("SendData OK ---IEC104 : Pub_MsgToServer ---\n");

		Sleep(2000);
	}

    //要停用IEC 60870-5-104服务，可以使用CS104_Slave_stop功能停止服务器。
	CS104_Slave_stop(slave);
	
	CS104_Slave_destroy(slave);

	SAFELY_DELETE(pIEC104);  

	TRACEX("-----------_ThreadEntry_IEC104Comm: Exit--------------\n");

	return THREAD_CANCEL_ALL;
}



typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("---------------Initialize 104 Start !!!--------------- \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;
	PARAM_VAL_GET valGet;

	//Create data
	pReporter->pDataHeap = NEW(IEC104_DATA_ROUGH, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for 104 dataStructs!\n");
		return FALSE;
	}

	IEC104_DATA_ROUGH* pIEC104 = (IEC104_DATA_ROUGH*)pReporter->pDataHeap;
	ZERO_POBJS(pIEC104, 1);

	pIEC104->hReporter = pReporterThis;

	pIEC104->hMutex = Mutex_Create(TRUE);
  	if(pIEC104->hMutex == NULL)
	{
		TRACE("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	ERRCODE_DAI err;
	valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 45, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->bNeedUploadCellInfo = (err==ERRCODE_DAI_OK ? valGet.val.emValue : 0);

	valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1001, 3303, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->enableREMSCtrl = (BOOL)(err==ERRCODE_DAI_OK ? valGet.val.emValue : FALSE);

    valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
        1001, 3300, &valGet, sizeof(PARAM_VAL_GET));
    pIEC104->nTDisconDelayIOA6191 = (err==ERRCODE_DAI_OK ? valGet.val.ulValue : 10);

	valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
        1001, 3310, &valGet, sizeof(PARAM_VAL_GET));
	pIEC104->nLocalEMSInfoReportingCycle = (err==ERRCODE_DAI_OK ? valGet.val.ulValue : 2);

    valGet.nGetSize = 4;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
        1001, 3902, &valGet, sizeof(PARAM_VAL_GET));
    pIEC104->emEMSCtrlMode_REMS = (EMSCtrlMode)(err==ERRCODE_DAI_OK ? valGet.val.ulValue : EMSCtrlMode_Local);
	pIEC104->lcCfg = GetLCCfg_Reporter(pReporter); // LJH250428 cmd
	//--------------------------------End-------------------------//
 
	//----- John 2023-06-04 Added IEC104 Client Used For Test-----//
	
	//pIEC104->bNeedExitThread = FALSE;
	//if(pthread_create(&pIEC104->hThread, 
	//	NULL, _Thread_IEC104_SimpleClient,
	//	(void *)pIEC104) != 0) //0-Success
	//{
	//	TRACE("----- Create _Thread_IEC104_SimpleClient Failed -----\n");
	//	return FALSE;  
	//}
	//pthread_detach(pIEC104->hThread);

	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("IEC-104Comm",
		(RUN_THREAD_START_PROC)_ThreadEntry_IEC104Comm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create IEC104 thread Failed \n");
		return FALSE;
	}

	if(!Init_IEC104_SigMap(pIEC104))
	{
		TRACE("ERROR : Init_IEC104_SigMap Failed \n");
		return FALSE;
	}

	if(!Init_SysParamConfig(pIEC104))
	{
		TRACE("ERROR : Init_SysParamConfig Failed \n");
		return FALSE;
	}

 	EVHAND_TAGS tags[] =
	{
			{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END },
			{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
 			{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
 	};

	int n = 0;
	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("104_CommEventHand",
			pReporterThis,
			Event_Handler,
			(UINT)tags[n].nEvTag,
			(UINT)tags[n].nMsgTag,
			FALSE);
	}	
	return TRUE;
}


