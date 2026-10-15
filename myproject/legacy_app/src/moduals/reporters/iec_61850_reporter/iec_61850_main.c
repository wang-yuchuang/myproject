#include "../reporters_base.h"
#include "iec_61850.h"
 
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
	IEC61850_DATA_ROUGH* pIEC61850 = (IEC61850_DATA_ROUGH*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;
	ERRCODE_DAI err;
	int n = 0;

	RunThread_Heartbeat(pReporter->hThread); //in case the thread no response

	switch (emEvType)
	{
		case EVENT_TYPE_ALARM:
		{
			//TRACE("Info:Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
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
			TRACEX("MQTT_Event_Handler  _EVENT_TYPE = EVENT_TYPE_ALARM, emMsgType = %d \n", emMsgType);
			EVENT_PARAM_CHG * pParamChg = (EVENT_PARAM_CHG*)pEvData;
			if (nEvDataSize != sizeof(EVENT_PARAM_CHG))
			{
				return EVENT_HANDED_FAILED;
			}

			if ((pParamChg->parmPTR.nSubId == 26))
			{
				// Read ESiOCPP Config 
				TRACEX("Config Changed Evevt \n");
				PARAM_VAL_GET valGet;
				valGet.nGetSize = sizeof(pIEC61850->blobBackendSet.arrySGvalue);
				valGet.val.blobValue = (BYTE*)&pIEC61850->blobBackendSet;
				err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
					1, 26, &valGet, sizeof(PARAM_VAL_GET));
				if (err != ERRCODE_DAI_OK)
				{
					TRACEX("ERROR: OCPP settings can not be retrieved\n");
					return FALSE;
				}
			}

		}
		break;
	default:
		break;
	}
	return EVENT_HANDED_OK;
}

static void IEC61850_AI_DI_Serv(IEC61850_DATA_ROUGH* pIEC61850)
{
	REPORTER* pReporter = (REPORTER*)pIEC61850->hReporter;

	SCL_AI ai;
	SCL_DI di;

	int n = 0;
	//printf("IEC61850 : putAi putDi Service!!!\n");

	//putAi 和 putDi 两个函数调用的内存块是分开的两个

	//模拟量 --电压 -- 电流

	/*
	ai.value = 10 + (float)(rand()%5);   //总有功功率 kw //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,0,&ai);			//将 ai的值放入 组号为0 ，序号为0的内存中

	ai.value = 20 + (float)(rand()%5);   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,1,&ai);			//将 ai的值放入 组号为1 ，序号为0的内存中

	ai.value = 30 + (float)(rand()%5);   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,2,&ai);			//将 ai的值放入 组号为2 ，序号为0的内存中

	ai.value = 0.950 + (float)(rand()%5) * 0.01;   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,3,&ai);			//将 ai的值放入 组号为3 ，序号为0的内存中

	ai.value =  49.5 + (float)(rand()%5)*0.01;   //Hz Frequncy //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,4,&ai);			//将 ai的值放入 组号为4 ，序号为0的内存中

	ai.value = 220 + (float)(rand()%5);   	//实际采集的值，强制转换成float型
	ai.quality = 0 ;				//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,5,&ai);				//将 ai的值放入 组号为5 ，序号为0的内存中

	ai.value = 221 + (float)(rand()%5);   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,6,&ai);			//将 ai的值放入 组号为6 ，序号为0的内存中

   
    ai.value = 222.0 + (float)(rand()%5);   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,7,&ai);			//将 ai的值放入 组号为7 ，序号为0的内存中

    
 	ai.value = 30.0 + (float)(rand()%5);   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,8,&ai);		//将 ai的值放入 组号为8 ，序号为0的内存中

	ai.value = 30.1 + (float)(rand()%5);   		//实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,9,&ai);			//将 ai的值放入 组号为9 ，序号为0的内存中


    ai.value = 30.2 + (float)(rand()%5);   //实际采集的值，强制转换成float型
	ai.quality = 0 ;		//0表示该数据有效， 1表示该数据无效
	ai.updtime.secs = time(NULL);  //数据采集的时间，距1970-01-01 00:00:00 的秒数
	putAi(0,10,&ai);			//将 ai的值放入 组号为10 ，序号为0的内存中
	*/

 
	//开关量
	PARAM_VAL_GET valGet;
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 17, &valGet, sizeof(PARAM_VAL_GET));
  	pIEC61850->DI[0] = valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 21, &valGet, sizeof(PARAM_VAL_GET));
  	pIEC61850->DI[1] = valGet.val.emValue;

	for(n = 0; n < MAX_DI_NUM ; n++)
	{
		if(pIEC61850->DI[n] != pIEC61850->s_LastDI[n])
		{
			pIEC61850->s_LastDI[n] = pIEC61850->DI[n];
			di.value = pIEC61850->DI[n];	//bool型数据
			di.quality = 0;					//同ai
			di.updtime.secs = time(NULL) ; 	//同ai
			putDi(0,n,&di); 
		}
	}

	int nDO_Offset = 6;
	for(n = 0; n < MAX_DO_NUM ; n++)
	{
		if(pIEC61850->DO[n] != pIEC61850->s_LastDO[n])
		{
			pIEC61850->s_LastDO[n] = pIEC61850->DO[n];
			di.value = pIEC61850->DO[n];	//bool型数据
			di.quality = 0;					//同ai
			di.updtime.secs = time(NULL) ; 	//同ai
			putDi(0,nDO_Offset + n,&di); 
		}
	}

	/*
	//DI量
	di.value = pIEC61850->DI[0];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,0,&di); 

	di.value = pIEC61850->DI[1];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,1,&di);

	di.value = pIEC61850->DI[2];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,2,&di);

	di.value = pIEC61850->DI[3];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,3,&di);
	
	di.value = pIEC61850->DI[4];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,4,&di);

	di.value = pIEC61850->DI[5];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,5,&di);
	*/

	/*
	//DO量
	di.value = pIEC61850->DO[0];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ; 	//同ai
	putDi(0,6,&di); 
	
	di.value = pIEC61850->DO[1];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ;  //同ai
	putDi(0,7,&di);
	
	di.value = pIEC61850->DO[2];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ;  //同ai
	putDi(0,8,&di);
	
	di.value = pIEC61850->DO[3];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ;  //同ai
	putDi(0,9,&di);
	
	di.value = pIEC61850->DO[4];				//bool型数据
	di.quality = 0;					//同ai
	di.updtime.secs = time(NULL) ;  //同ai
	putDi(0,10,&di);
	*/
	//TRACE("!!! PutDi - putDO Success !!!\n");

	return;
}
 

static BOOL Update_Local_SEArray_Value(IEC61850_DATA_ROUGH* pIEC61850)
{
	REPORTER* pReporter = (REPORTER*)pIEC61850->hReporter;
	VAR_VALUE varV;
	ERRCODE_DAI err = -1;
	pIEC61850->blobBackendSet.emBlobType = BLOB_TYPE_ES_BACKEND_CFG;
	varV.blobValue = &(pIEC61850->blobBackendSet);
  
	err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 
		1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG), 1000);
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: SEt A_BLOB_ES_BACKEND_CFG FAiled!\n");
		return FALSE;
	}
	return TRUE;
}


	 
static BOOL Excute_YK_Action(IEC61850_DATA_ROUGH* pIEC61850, SCL_YK* pYk)
{
	REPORTER* pReporter = (REPORTER*)pIEC61850->hReporter;
	VAR_VALUE varV;
	UINT nSigID;

	switch (pYk->no)
	{
		case 100: nSigID = 31; break;
		case 101: nSigID = 32; break;
		case 102: nSigID = 33; break;
		case 103: nSigID = 34; break;
		case 104: nSigID = 35; break;
		case 105: nSigID = 36; break;

		default:
			break;
	}

	UINT nDOIdx = pYk->no - 100;
	if((nDOIdx >= 0) && (nDOIdx < 6))
	{
		pIEC61850->DO[nDOIdx] = pYk->ctrlIntValue;
	}
	varV.emValue = pYk->ctrlIntValue;
	ERRCODE_DAI err = pReporter->pDAI->Set((HANDLE)pReporter, DAITYPE_CTRL_PARAMVALUE_RW,
		1, nSigID, &varV, 4, 1000);

	return (err == ERRCODE_DAI_OK) ? TRUE : FALSE;
}
//模拟定值组参数, 总共 4个参数, 3组
//float pIEC61850->blobBackendSet.arrySGvalue[3][4] = {{1,2,3,4}, {11,22,33,44}, {111,222,333,444}};


static DWORD _ThreadEntry_IEC61850Comm(void* pArg)
{
	TRACE("-----------_ThreadEntry_IEC61850Comm: Start Enter------- \n");
	REPORTER* pReporter = (REPORTER*)pArg;
	HANDLE hPort = pReporter->hCommPort;
 	IEC61850_DATA_ROUGH* pIEC61850 = (IEC61850_DATA_ROUGH*)pReporter->pDataHeap;
	int rc = -1;
	static int nInitTimes = 0;
	/******61850 - - Init Public Memory ***/
	while(!initScl())
	{
		RUN_THREAD_HEARTBEAT();
		printf("initScl()...faild!!!\n");
		Sleep(1000);
		initScl();
		nInitTimes++;
		if(nInitTimes > 5)
		{
			SAFELY_DELETE(pIEC61850);  
			return THREAD_CANCEL_ALL;
		}
	}
	printf("initScl()...ok\n"); /******初始化内存***/

	
	//初始化遥控参数
	unsigned int m_ykReadPos;
	m_ykReadPos = getYkWritePos();


	//初始化定值组控制块
	unsigned int mEditSG = 0;
	unsigned int mActSG = 1;
	SCL_AI ActSG;
	ActSG.value = mActSG;
	putAi(1, 1, &ActSG);

	//初始化定值组参数,默认激活第1组
	SCL_AI mSG;
	int i = 0;
	for(i = 0; i < MAX_SG_SIGLGRP_ELE; i++)
	{
		mSG.value = pIEC61850->blobBackendSet.arrySGvalue[0][i];
		putAi(1, 100 + i, &mSG);
	}
	

    //第三步：将默认组号里面的参数下设到装置
 
	while (!pReporter->bExit)
	{
		RUN_THREAD_HEARTBEAT();

 		/********实时刷新遥测遥信********/
		IEC61850_AI_DI_Serv(pIEC61850);
		

		//IEC61850_SG_ACTSG_ESG_Serv(pIEC61850);

		if ( hasYk( m_ykReadPos ) )
		{
			SCL_YK* pYk = getYk(&m_ykReadPos);
			if ( pYk )
			{
				switch ( pYk->ctrlAction )
				{
					case SCL_YK_ACTION_SELECT_V:			//控制选择
						break;
					case SCL_YK_ACTION_EXECUTE:				//控制执行
					{
						// 1、接收到遥控指令后操作设备-begin
						//***控制值为 pYk->ctrlIntValue
						Excute_YK_Action(pIEC61850, pYk);

						// 2、遥控反馈
						SCL_YK_ACK ack;  //遥控反馈
						ack.type = pYk->type;
						ack.ctrlAction = pYk->ctrlAction;
						ack.grpno = pYk->grpno;
						ack.no = pYk->no;
						ack.newIntValue = pYk->ctrlIntValue;
						ack.newFloatValue = pYk->ctrlFloatValue;
						ack.type = pYk->type;
						ack.result = YK_SUCCESS;
						printf("Remore Control Excute:group = %d, no = %d, value = %f \n", pYk->grpno, pYk->no, pYk->ctrlIntValue);
						putYkAck(&ack);
						break;
					}
					case SCL_YK_ACTION_CANCEL:		//控制撤销
						break;
					case SCL_YK_ACTION_SET_ACTSG:
					{
						// 1、将参数设置到装置


						// 2、将参数显示给客户端
						// 2.1 显示激活组
						mActSG = pYk->ctrlIntValue;
						ActSG.value = mActSG;
						putAi(1, 1, &ActSG);
						// 2.2 显示具体定值SG
						for(i = 0; i < MAX_SG_SIGLGRP_ELE; i++)
						{
							if(mActSG <= 0)
							{
								break;
							}
							mSG.value = pIEC61850->blobBackendSet.arrySGvalue[mActSG-1][i];
							putAi(1, 100+i, &mSG);
						}

						// 3、反馈失败还是成功
						SCL_YK_ACK ack;
						ack.ctrlAction = pYk->ctrlAction;
						ack.grpno = pYk->grpno;
						ack.no = pYk->no;
						ack.newIntValue = pYk->ctrlIntValue;
						ack.type = pYk->type;
						ack.result = YK_SUCCESS;
						printf("SCL_YK_ACTION_SET_ACTSG:Success\n");
						putYkAck(&ack);
						break;
					}
					case SCL_YK_ACTION_SET_EDITSG:
					{
						// 1、记录编辑组
						mEditSG = pYk->ctrlIntValue;

						// 2、将对应的SE 内容展示给客户端
						SCL_AI_SE aise;
						for(i=0; i < MAX_SG_SIGLGRP_ELE; i++)
						{
							if(mEditSG <= 0)
							{
								break;
							}
							aise.value = pIEC61850->blobBackendSet.arrySGvalue[mEditSG-1][i];
							putAiSE(1, 104 + i, &aise);
						}

						// 3、反馈失败还是成功
						SCL_YK_ACK ack;
						ack.ctrlAction = pYk->ctrlAction;
						ack.grpno = pYk->grpno;
						ack.no = pYk->no;
						ack.newIntValue = pYk->ctrlIntValue;
						ack.type = pYk->type;
						ack.result = YK_SUCCESS;
						printf("SCL_YK_ACTION_SET_EDITSG: Success!\n");
						putYkAck(&ack);
						break;
					}
					case SCL_YK_ACTION_SET_CNFEDIT:
					{
						// 1、获取编辑后的数据，并写入对应的组
						const SCL_AI_SE* aise;
						for( i=0; i < MAX_SG_SIGLGRP_ELE; i++)
						{
							if(mEditSG <= 0)
							{
								break;
							}
							aise = getAiSE(1, 104+i);
							pIEC61850->blobBackendSet.arrySGvalue[mEditSG-1][i] = aise->value;
						}

						// 2、反馈失败还是成功
						SCL_YK_ACK ack;
						ack.ctrlAction = pYk->ctrlAction;
						ack.grpno = pYk->grpno;
						ack.no = pYk->no;
						ack.newIntValue = pYk->ctrlIntValue;
						ack.type = pYk->type;
						ack.result = YK_SUCCESS;
						printf("SCL_YK_ACTION_SET_CNFEDIT: Success\n");
						putYkAck(&ack);
						
						//3、Update LD SE Array Value
						Update_Local_SEArray_Value(pIEC61850);

						break;
					}
				}
			}
		}
		//TRACEX("-----------Pub_MsgToServer rc=[%d]--- \n", rc);
		Sleep(500);
	}

	finitScl();

	SAFELY_DELETE(pIEC61850);  

	TRACEX("-----------_ThreadEntry_IEC61850Comm: Exit--------------\n");
	return THREAD_CANCEL_ALL;
}


typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;

}EVHAND_TAGS;

DLLExport BOOL Initialize(HANDLE pReporterThis)
{
	TRACEX("---------------Initialize MQTT Start !!!--------------- \n");
	REPORTER* pReporter = (REPORTER*)pReporterThis;

	//Create data
	pReporter->pDataHeap = NEW(IEC61850_DATA_ROUGH, 1);
	if(pReporter->pDataHeap == NULL)
	{
		TRACEX("Memory full for MQTT dataStructs!\n");
		return FALSE;
	}

	IEC61850_DATA_ROUGH* pIEC61850 = (IEC61850_DATA_ROUGH*)pReporter->pDataHeap;
	ZERO_POBJS(pIEC61850, 1);

	pIEC61850->hReporter = pReporterThis;

	pIEC61850->hMutex = Mutex_Create(TRUE);
  	if(pIEC61850->hMutex == NULL)
	{
		TRACE("ERROR : Fail to Create Mutex and Queue \n");
		return FALSE;
	}

	//-------------Get Backend Config-------------// 
	PARAM_VAL_GET valGet;
	valGet.nGetSize = sizeof(A_BLOB_ES_BACKEND_CFG);
	valGet.val.blobValue = (BYTE*)&pIEC61850->blobBackendSet;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		1, 26, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: IEC61850 Backend settings Get Failed!\n");
		return FALSE;
	}

	int n = 0;
	for(n = 0; n < MAX_DI_NUM ; n++)
	{
		pIEC61850->DI[n] = 0; 
		pIEC61850->s_LastDI[n] = 0;
	}

	for(n = 0; n < MAX_DO_NUM ; n++)
	{
		pIEC61850->DO[n] = 0; 
		pIEC61850->s_LastDO[n] = 0;
	}

	pReporter->bExit = FALSE;
	pReporter->hThread = RunThread_Create("IEC-61850Comm",
		(RUN_THREAD_START_PROC)_ThreadEntry_IEC61850Comm,
		(void *)(pReporter),
		(DWORD*)&pReporter->bExit);
		
	if(pReporter->hThread == NULL)
	{
		TRACE("ERROR : Create MqttComm thread Failed \n");
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

 	for (n = 0; n < ITEM_OF(tags); n++)
	{
		pReporter->pDAI->Regsiter_EvHandler("61850_CommEventHand",
			pReporterThis,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}	
	return TRUE;
}


