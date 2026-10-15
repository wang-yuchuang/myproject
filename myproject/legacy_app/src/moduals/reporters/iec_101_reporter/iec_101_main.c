#include "../reporters_base.h"
#include "iec_101.h"
 
static bool running = true;


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
	REPORTER* pReporter = (REPORTER*)hModuleSeif;
	IEC101_DATA_ROUGH* pIEC101 = (IEC101_DATA_ROUGH*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n = 0;

	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response

	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			TRACE("Info:Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_ALARM * pAlmEv = (EVENT_ALARM*)pEvData;
			if (nEvDataSize != sizeof(EVENT_ALARM))
			{
				return EVENT_HANDED_FAILED;
			}
 		}
		break;
		case EVENT_TYPE_FIRMWARE_UPDATE:
		{
			TRACEX("Mqtt_Event_Handler  _EVENT_TYPE = EVENT_TYPE_FIRMUPDATE \t emMsgType = %d, size[%d,%d] \n", 
				emMsgType, nEvDataSize, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			EVENT_FIRMWAREUPDATE_STATUS * pFirmUpSta = (EVENT_FIRMWAREUPDATE_STATUS*)pEvData;
			if (nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
		
		case EVENT_TYPE_DIAGNOSTIC_UPLOAD:
		{
			TRACEX("Mqtt_Event_Handler  _EVENT_TYPE = EVENT_TYPE_DIAGNOSUPLOAD \t emMsgType = %d \n", emMsgType);
			EVENT_DIAGNOUPLOAD_STATUS * pDiagUpSta = (EVENT_DIAGNOUPLOAD_STATUS*)pEvData;
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
			TRACEX("Mqtt_Event_Handler  _EVENT_TYPE = EVENT_TYPE_EXIT, EXIT_TYPE = %d \n", pEvReboot->nExitType);
 
		}
		break;
		
		case EVENT_TYPE_CFG_CHANGED:
		{
			TRACEX("101_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}
		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  _ThreadEntry_IEC101Comm
*  CREATOR       :  
*  CREATED DATE  : 2023-01-10 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static DWORD _ThreadEntry_IEC101Comm(void* pArg)
{
	TRACE("-----------_ThreadEntry_IEC101Comm: Start Enter------- \n");
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
 	IEC101_DATA_ROUGH* pIEC101 = (IEC101_DATA_ROUGH*)pReporter->pDataHeap;

	SerialPort port = SerialPort_create(TTYS_IEC101, 9600, 8, 'E', 1);

	  /* create a new slave/server instance with default link layer and application layer parameters */
    // CS101_Slave slave = CS101_Slave_create(port, NULL, NULL, IEC60870_LINK_LAYER_BALANCED);
    CS101_Slave slave = CS101_Slave_create(port, NULL, NULL, IEC60870_LINK_LAYER_UNBALANCED);

    CS101_Slave_setLinkLayerAddress(slave, 1);
    CS101_Slave_setLinkLayerAddressOtherStation(slave, 1);

    /* get the application layer parameters - we need them to create correct ASDUs */
    CS101_AppLayerParameters alParameters = CS101_Slave_getAppLayerParameters(slave);

    /* change default application layer parameters (optional) */
   
    alParameters->sizeOfCA = 1;//2;  /* 公共地址的长度 size of common address (CA) of ASDU (1/2 - default = 2) */
    alParameters->sizeOfIOA = 2;//3; /* 信息体地址长度size of information object address (IOA) (1/2/3 - default = 3) */
    alParameters->sizeOfCOT = 1;//2; //传送原因的长度 /* size of COT (1/2 - default = 2 -> COT includes OA) */

    LinkLayerParameters llParameters = CS101_Slave_getLinkLayerParameters(slave);
    llParameters->timeoutForAck = 500;

    /* set the callback handler for the clock synchronization command */
    CS101_Slave_setClockSyncHandler(slave, clockSyncHandler, NULL);

    /* set the callback handler for the interrogation command */
    CS101_Slave_setInterrogationHandler(slave, interrogationHandler, NULL);

    /* set handler for other message types */
    CS101_Slave_setASDUHandler(slave, asduHandler, NULL);

    /* set handler for reset CU (reset communication unit) message */
    CS101_Slave_setResetCUHandler(slave, resetCUHandler, (void*) slave);

    /* set timeout for detecting connection loss */
    CS101_Slave_setIdleTimeout(slave, 1500);

    /* set handler for link layer state changes */
    CS101_Slave_setLinkLayerStateChanged(slave, linkLayerStateChanged, NULL);

    /* uncomment to log messages */
    CS101_Slave_setRawMessageHandler(slave, rawMessageHandler, NULL);

    int16_t scaledValue = 0;

    uint64_t lastMessageSent = 0;

    SerialPort_open(port);


    TRACEX("!!! CS101_Slave_start scaledValue=[%d]---\n", scaledValue);
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();
        TRACEX("111 scaledValue=[%d] ---\n", scaledValue);

		 /* has to be called periodically */
        CS101_Slave_run(slave);

        /* Enqueue a measurement every second */
        if (Hal_getTimeInMs() > (lastMessageSent + 1000)) {

            CS101_ASDU newAsdu = CS101_ASDU_create(alParameters, false, CS101_COT_PERIODIC, 0, 1, false, false);

            InformationObject io = (InformationObject) MeasuredValueScaled_create(NULL, 110, scaledValue, IEC60870_QUALITY_GOOD);

            scaledValue++;

            CS101_ASDU_addInformationObject(newAsdu, io);

            InformationObject_destroy(io);

            CS101_Slave_enqueueUserDataClass2(slave, newAsdu);

            CS101_ASDU_destroy(newAsdu);

            lastMessageSent = Hal_getTimeInMs();
        }


		TRACEX("333 -----------IEC101 : Pub_MsgToServer rc=[%d]--- \n");
		Sleep(1000);
	}

    CS101_Slave_destroy(slave);
	SerialPort_close(port);
    SerialPort_destroy(port);

	SAFELY_DELETE(pIEC101);  

	TRACEX("-----------_ThreadEntry_IEC101Comm: Exit--------------\n");
	return THREAD_CANCEL_ALL;
}


typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("---------------Initialize 101 Start !!!--------------- \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;

	//Create data
	pReporter->pDataHeap = NEW(IEC101_DATA_ROUGH, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for 101 dataStructs!\n");
		return FALSE;
	}

	IEC101_DATA_ROUGH* pIEC101 = (IEC101_DATA_ROUGH*)pReporter->pDataHeap;
	ZERO_POBJS(pIEC101, 1);

	pIEC101->hReporter = pReporterThis;

	pIEC101->hMutex = Mutex_Create(TRUE);
  	if(pIEC101->hMutex == NULL)
	{
		TRACE("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("IEC-101Comm",
		(RUN_THREAD_START_PROC)_ThreadEntry_IEC101Comm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create IEC101 thread Failed \n");
		return FALSE;
	}
	
 	EVHAND_TAGS tags[] =
	{
			{ EVENT_TYPE_ALARM,				EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_RESTORE},
			{ EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN },
 			{ EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_DIAGNOSTIC_UPLOAD,EVENT_MSG_BEGIN | EVENT_MSG_END | EVENT_MSG_STATUS_CHANGED },
			{ EVENT_TYPE_ABILITY_CHANGE,	EVENT_MSG_BEGIN | EVENT_MSG_END }, //do not treat this, use status change event instead
			{ EVENT_TYPE_SYSTEM_EXIT,		EVENT_MSG_END},
 	};

	int n = 0;
	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("101_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}	
	return TRUE;
}


