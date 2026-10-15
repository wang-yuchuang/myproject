// #define _TRACE_PRINT_

#include <dlfcn.h>
#include "sampler_loader.h"
#include "3rd_party/curl/curl.h"

static BOOL IsTimeToSample(SAMPLER* pSampler, double tmNow)
{
	if (pSampler->bJustControlled || pSampler->bLastSampleInterrupted ||
		((tmNow - pSampler->tmLastSampled) >= ((float)pSampler->info.nSamplePeriod_mSec / 1000.0)) ||
		(tmNow < pSampler->tmLastSampled))
	{
		//TRACEX("NeedSample:[%d, %d], samplePreiod[%d].\n",pSampler->bJustControlled,pSampler->bLastSampleInterrupted,pSampler->info.iSamplerPeriod);
		pSampler->bLastSampleInterrupted = FALSE;
		pSampler->bJustControlled = FALSE;	// reset need just control flag.
		return TRUE;					// Need query data now.
	}
	return FALSE;
}


#define  WAIT_CMD_QUEUE	100  //that means min sample period should be 100 msec
static DWORD __Thread_ASampler(void* pArgs)
{
	double			tmNow;
	SAMPLER* pSampler = (SAMPLER*)pArgs;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	int nErrCode;
	int nMaxExeCmd = 0;	
	INTER_QUEUE_CMD cmd;
	//Sleep(5000);
	while(!pSampler->bExit)
	{
		RUN_THREAD_HEARTBEAT();
		if(!pSampler->bIsInitlized)
		{
			Sleep(100);
			continue;
		}
		tmNow = GetCurrentTime();	// current time.

		//Check command first
		nMaxExeCmd = 6; //at most execute 6 cmds each time/**/
		do 
		{
			if(Queue_Get(pSampler->hqControlCommand, &cmd, FALSE, WAIT_CMD_QUEUE) == ERR_QUEUE_OK)
			{
				RUN_THREAD_HEARTBEAT();
				//Now execute the callback control function
				nErrCode = pSampler->Control(
					(HANDLE)pSampler,
					(cmd.emType == SETTYPE_INTERN_CTRL) ? PARAM_TYPE_CONTROL : PARAM_TYPE_SETTING, 
					cmd.data.nDevId, 
					cmd.data.nParamId,
					&cmd.data.varValue,
					cmd.data.nValueSize,
					/*cmd.nTimeOut*/WAIT_INFINITE);

				pSampler->bJustControlled = TRUE;

				//now copy new value back to db
				if(nErrCode == ERR_CONTROL_OK)
				{
					pDPR->Set((HANDLE)pSampler, (cmd.emType == SETTYPE_INTERN_CTRL) ? SETTYPE_RAW_BACK_CTRL : SETTYPE_RAW_BACK_SETT,
						&cmd.data, sizeof(INTERSET_DATA));
				}

				if(cmd.data.nValueSize > 4)
				{
					DELETE(cmd.data.varValue.blobValue); //this was new in DPR when put into QUEUE
				}

				TRACEX("--------------Sampler executed the cmd, result = %d-------\n", nErrCode);

				//if(nErrCode != ERR_OK)
				//{
				//	pCmdOrig->bIsExecutedOK = FALSE;
				//	//AppLogOut(SYS_MAIN, APP_LOG_ERROR,
				//	//	"There is an urgent cmd that failed to execute!!! \n");
				//}
				//else
				//{
				//	pCmdOrig->bIsExecutedOK = TRUE;
				//}
				//TRACEX("RUN Here22222222\n");
			}
			else
			{
				break;
			}
			
			nMaxExeCmd--;
		} while (nMaxExeCmd >= 0);


		//if(s_pChargeStub->pJigData->bEnterTestMode)
		//{
		//	//进入工装测试程序/**/
		//	pSampler->pfnJigTest(&pSampler->info, (void*)s_pChargeStub->pJigData);
		//	continue;
		//}

		//TRACEX("RUN HereAAAAAAA\n");
		// Then sample
		if(IsTimeToSample(pSampler, tmNow))
		{
			nErrCode = pSampler->Sample((HANDLE)pSampler);
			/*if(nErrCode != ERR_OK)*/
			if((nErrCode == ERR_SAMPLE_TIMEOUT) ||
				(nErrCode == ERR_SAMPLE_NEED_RESAMP))
			{
				pSampler->bLastSampleInterrupted = TRUE;
				// if (pSampler->info.nSamplerID == 1)
				// {
				// 	TRACEW("__Thread_ASampler ERR_SAMPLE_NEED_RESAMP [%s]--Time to sample, sample period = %d, now = %0.1f,.\n",
				// 	pSampler->info.szSamplerName, pSampler->info.nSamplePeriod_mSec,
				// 	tmNow);
				// }
			}
			else
			{
				pSampler->tmLastSampled = tmNow;
				// if (pSampler->info.nSamplerID == 1)
				// {
				// 	TRACEW("__Thread_ASampler else [%s]--Time to sample, sample period = %d, now = %0.1f,.\n",
				// 	pSampler->info.szSamplerName, pSampler->info.nSamplePeriod_mSec,
				// 	tmNow);
				// }
			}
	
			//TRACEX("[%s]--nErrCode = %d, StartSamp = %0.1f, EndSamp = %0.1f.\n",
			//	pSampler->info.szSamplerName, nErrCode,
			//	pSampler->tmSampleStarted, pSampler->tmDataSampled);
		}
	}

	//close comm_port
	//pSampler->pUsedCommPort->Close(pSampler->hCommPort);

	//do clean job
	//THREAD_CANCEL_ALL means must system will reboot
	return THREAD_CANCEL_ALL;
}

static BOOL LoadSamplerLib(IN SAMPLER *pSampler)
{
	const char *pszSymName[] =
	{
		SAMPLER_INIT_PROC_NAME,		//Initialize
		SAMPLER_SAMPLE_PROC_NAME,		//Sample
		SAMPLER_CONTROL_PROC_NAME,	//Control
		SAMPLER_JIG_TEST_PROC_NAME	//Jig Test
	};

	HANDLE	*pfnProc[] =
	{
		(HANDLE* )&pSampler->Init,
		(HANDLE *)&pSampler->Sample,
		(HANDLE *)&pSampler->Control,
		(HANDLE *)&pSampler->JigTest
	};

	char szFullLibPath[MAX_FILE_PATH];
	pSampler->hSamplerLib = LoadDynamicLibrary(MakeFullFileName(szFullLibPath,
			getenv(ENV_VARIABLE_APP_PATH), "sampler", pSampler->info.szSamplerDriver),
			ITEM_OF(pszSymName), pszSymName,
			pfnProc, TRUE);

	if (pSampler->hSamplerLib == NULL)
	{
		LogOut(MAIN_MODULE_MAIN, "LoadSampLib", LOG_TYPE_ERROR,
			"Fails on loading sampler lib --> [%s], please check!",
			pSampler->info.szSamplerDriver);
		return FALSE;
	}

	return TRUE;
}

BOOL Init_Load_AllSamplers(CHARGER* pCharger)
{
	//pCharger->pJigData = NEW(JIG_DATA, 1);
	//if(pCharger->pJigData == NULL)
	//{
	//	return FALSE;
	//}
	//pCharger->pJigData->bEnterTestMode = FALSE;
	//Jimmy 20210201: put it here so that it only be called once
	curl_global_init(CURL_GLOBAL_ALL);

	int n;
	SAMPLER* pSampler = pCharger->samplers.p;
	BOOL bGot = FALSE, bSEMS = FALSE, bAEMS = FALSE;
	SYSTEM_TYPE emSystemType;
	for(n = 0; n < pCharger->samplers.nTotal; n++,pSampler++)
	{
		// LJH241101 MU3 don't Queue_Create if Sampler isn't loaded
		int nSamplerID = pSampler->info.nSamplerID;
		pSampler->bJustControlled = FALSE; //It is used control20060616
		pSampler->bLastSampleInterrupted = FALSE;
		pSampler->bIsInitlized = FALSE;
		pSampler->hDPR = (HANDLE)&pCharger->dataprocessor;
		pSampler->bExit = FALSE;
#ifdef IMMU3
		if(nSamplerID==4)
		{
			TRACEW("sampler_loader:Init_Load_AllSamplers continue IMMU3 not load nSamplerID:%d szSamplerName:%s",
				nSamplerID, pSampler->info.szSamplerName);
			continue;
		}
#endif
		VAR_VALUE varV;
		ERRCDDE_DPR err;
		DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
		if (!bGot)
		{
			bGot = TRUE;
			err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 100, &varV, 4);
			bSEMS = (((err == ERR_DPR_OK) ? varV.emValue : SYSTYPE_ESS_AIR) == SYSTYPE_EMS_STATION);

			err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 73, &varV, 4);
			bAEMS = (BOOL)((err==ERR_DPR_OK) ? varV.emValue : FALSE);
			TRACED("sampler_loader:Init_Load_AllSamplers bSEMS:%d bAEMS:%d", bSEMS, bAEMS);
		}
		// LJH251111 AEMS
		if ((bSEMS && nSamplerID>=4 && nSamplerID<=6) ||
			(!bAEMS && nSamplerID==9))
		{
			TRACEW("sampler_loader:Init_Load_AllSamplers continue not load nSamplerID:%d szSamplerName:%s",
				nSamplerID, pSampler->info.szSamplerName);
			continue;
		}
		
		// if (bSEMS)
		// {// LJH241202 SEMS 
		// 	if(nSamplerID>=4 && nSamplerID<=6)//edit by kuangyue 20251029
		// 	{
		// 		TRACE("sampler_loader:Init_Load_AllSamplers SEMS continue not load nSamplerID:%d szSamplerName:%s",
		// 			nSamplerID, pSampler->info.szSamplerName);
		// 		continue;
		// 	}
		// }

		// varV.emValue = 0;
		// pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 56, &varV, 4);
 		// if(varV.emValue == 0)
		// {
		// 	// INFY Charger Disabled
			if(nSamplerID == 2)
			{
				TRACEW("Master:nSamplerID =%d szSamplerName=%s \n", \
					nSamplerID, pSampler->info.szSamplerName);
				continue;
			}
		// }

		varV.emValue = 0;
		pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 8, &varV, 4);
 		if(varV.emValue == 0) //Not Installed.
		{
			//Not Load Udp (#6).
			if(nSamplerID == 6)
			{
				TRACEW("Master:nSamplerID =%d szSamplerName=%s \n", \
					nSamplerID, pSampler->info.szSamplerName);
				continue;
			}
		}

		//add by lpl 240530 for UartServer
		varV.emValue = 0;
		pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 901, 910, &varV, 4);
 		if(varV.emValue == 0) //Not Installed.
		{
			//Not Load UartServer (#7).
			if(nSamplerID == 7)
			{
				TRACEW("Master:nSamplerID =%d szSamplerName=%s \n", \
					nSamplerID, pSampler->info.szSamplerName);
				continue;
			}
		}
		varV.emValue = 0;
		pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1050, 458, &varV, 4);
 		if(varV.emValue == 0) //Not Installed.
		{
			//Not Load PV (#8).
			if(nSamplerID == 8)
			{
				TRACED("Master:nSamplerID =%d szSamplerName=%s \n", \
					nSamplerID, pSampler->info.szSamplerName);
				continue;
			}
		}
		
		varV.emValue = 0;
		pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 621, 409, &varV, 4);
		TRACED("varV.emValue = %d\n", varV.emValue);
 		if(varV.emValue == 0) //Not Installed.
		{
			//Not Load Multifunction Combiner Cabinet IMEU3 (#10).
			if(nSamplerID == 10)
			{
				TRACED("Master:nSamplerID =%d szSamplerName=%s \n", \
					nSamplerID, pSampler->info.szSamplerName);
				continue;
			}
		}

		pSampler->hqControlCommand = Queue_Create(MAX_CTRL_CMD_NUM,sizeof(INTER_QUEUE_CMD), 0);//0 means can not be auto increased
		pSampler->hSamplerLock = Mutex_Create(TRUE);
		if((pSampler->hqControlCommand == NULL) ||
			(pSampler->hSamplerLock == NULL))
		{
			LogOut(MAIN_MODULE_MAIN, "InitSampler", LOG_TYPE_ERROR,
				"Fails on create queue/mutex for the Sampler --> [%s] !\n",
				pSampler->info.szSamplerName);
			printf("1111111111111111\n");

			return FALSE;
		}
		//load DLLs
		if(!LoadSamplerLib(pSampler))
		{
			printf("22222222222222");
			return FALSE;
		}

		//Now create and start the sampling thread
		pSampler->hSampleThread= RunThread_Create(pSampler->info.szSamplerName,
			(RUN_THREAD_START_PROC)__Thread_ASampler,
			(void *)(pSampler),
			(DWORD*)&pSampler->bExit);

		if(pSampler->hSampleThread  == NULL)
		{
			printf("3333333333333");
			return FALSE;
		}

		//int nErrCode = 0;
		//pSampler->hCommPort = pSampler->pUsedCommPort->Open(
		//	pSampler->pUsedCommPort->info.szPortName, 
		//	pSampler->info.szSamplerSetting,
		//	COMM_CLIENT_MODE, 5000, &nErrCode);
		//if(ERR_COMM_OK != nErrCode)
		//{
		//	return FALSE;
		//}

		//Now Initialize the sampler
		if(!pSampler->Init((HANDLE)pSampler))
		{
			LogOut(MAIN_MODULE_MAIN, "InitSampler", LOG_TYPE_ERROR,
				"Fails on initializing the Sampler --> [%s], please check!",
				pSampler->info.szSamplerName);

			RunThread_Stop(pSampler->hSampleThread, 1000, TRUE);
			printf("44444444444");
			return FALSE;
		}
		pSampler->bIsInitlized = TRUE;
	}

	return TRUE;
}
