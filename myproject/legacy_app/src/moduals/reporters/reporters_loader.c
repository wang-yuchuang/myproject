// #define _TRACE_PRINT_ // cmd
#include <dlfcn.h>
#include "reporters_loader.h"
#include "reporters_base.h"

static BOOL LoadReporterLib(IN REPORTER *pReporter)
{
	const char *pszSymName[] =
	{
		REPORTER_INIT,//Initalize
	};
	TRACEI("loaded reporter id[%d]",pReporter->info.nReporterID);
	HANDLE	*pfnProc[] =
	{
		(HANDLE* )&pReporter->Init
	};

	char szFullLibPath[MAX_FILE_PATH];
	pReporter->hSamplerLib = LoadDynamicLibrary(MakeFullFileName(szFullLibPath,
		getenv(ENV_VARIABLE_APP_PATH), "reporter", pReporter->info.szReporterDriver),
		ITEM_OF(pszSymName), pszSymName,
		pfnProc, TRUE);

	if (pReporter->hSamplerLib == NULL)
	{
		TRACE("Fails on loading reporter lib =[%s]\n", pReporter->info.szReporterDriver);
		LogOut(MAIN_MODULE_MAIN, "LoadReporterLib", LOG_TYPE_ERROR,
			"Fails on loading reporter lib --> [%s], please check!",
			pReporter->info.szReporterDriver);
		return FALSE;
	}

	return TRUE;
}

BOOL Init_Load_AllReporters(CHARGER* pCharger)
{
	int n;
	REPORTER* pReporter = pCharger->reporters.p;
	
	/********just for test purpose ********/
	//pCharger->reporters.nTotal = 1;
	/*******just for test purpose **********/
	int nPlatformType = -1;
	for(n = 0; n < pCharger->reporters.nTotal; n++, pReporter++)
	{
		int nReporterID = pReporter->info.nReporterID;
		pReporter->pDAI = &pCharger->dai;
		// LJH241101 MU3
// #ifdef IMMU3
		if(nReporterID == 1) // Don't Load Qt GUI reporter_gui
		{
			TRACEI("reporters_loader:Init_Load_AllReporters continue not load nReporterID:%d szReporterName:%s",
				nReporterID, pReporter->info.szReporterName);
			continue;
		}
// #endif
		
		PARAM_VAL_GET valGet;
		//valGet.nGetSize = 4;
		//pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		//	1, 8, &valGet, sizeof(PARAM_VAL_GET));
		//if(valGet.val.emValue != 0)
		//{
		//	if((nReporterID == 2) || (nReporterID == 3))
		//	{
		//		TRACE("REPORTER: System Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
		//		continue;
		//	}
		//}
		//else
		{
			//---------Get Cloud Server Reporter Config-------//
			//Reporter#2-Websocket Setting Enum = 0 
			//Reporter#6-Mqtt Setting Enum = 1/2 
			//Reporter#7-IEC104 Setting Enum = 3 
			//Reporter#8-IEC61850 Setting Enum = 4 
			//Reporter#9-IEC101 Setting Enum = 5 

			valGet.nGetSize = 4;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
				1, 10, &valGet, sizeof(PARAM_VAL_GET));
			nPlatformType = valGet.val.emValue;
			if(valGet.val.emValue == 0) //Std Reporter
			{
				if((nReporterID == 6)||
					(nReporterID == 7)||
					(nReporterID == 8)||
					(nReporterID == 9))
				{
					TRACE("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				} 
			}
			else if((valGet.val.emValue == 1)||(valGet.val.emValue == 2))//Mqtt Reporter
			{
				if(
					(nReporterID == 7)||
					(nReporterID == 8)||
					(nReporterID == 9))
				{
					TRACE("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				} 
			}
			else if(valGet.val.emValue == 3)//IEC104 
			{
				if(
					(nReporterID == 6)||
					(nReporterID == 8)||
					(nReporterID == 9))
				{
					TRACE("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				} 
			}
			else if(valGet.val.emValue == 4)//IEC61850
			{
				if(
					(nReporterID == 6)||
					(nReporterID == 7)||
					(nReporterID == 9))
				{
					TRACE("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				} 
			}
			else if(valGet.val.emValue == 5)//IEC101
			{
				if(
					(nReporterID == 6)||
					(nReporterID == 7)||
					(nReporterID == 8))
				{
					TRACE("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				} 
			}
			// else if(valGet.val.emValue == 6)//ModbusTcp
 			// {
 			// 	if((nReporterID == 2)||
 			// 		(nReporterID == 6)||
 			// 		(nReporterID == 7)||
 			// 		(nReporterID == 8)||
 			// 		(nReporterID == 9))
 			// 	{
 			// 		TRACEX("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
 			// 		continue;
 			// 	} 
 			// }
			else if(valGet.val.emValue == 6)//not install
			{
				if(
					(nReporterID == 6)||
					(nReporterID == 7)||
					(nReporterID == 8)||
					(nReporterID == 9))
				{
					TRACE("REPORTER: Reporter Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				} 
			}
		}
		
		//Added By John. For Ify Cloud Connection.
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 16, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Disable-Enable
		{
			if(nReporterID == 10)
			{
				continue;
			}
		}

		// add by chenjijun 20251014 for support Separate STD reporter
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 47, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Disable-Enable
		{
			if(nReporterID == 2)
			{
				if (nPlatformType != 0) //Std Reporter
				{
					TRACE("REPORTER: System Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);
					continue;
				}
			}
		}

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			1, 50, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Disable-Enable
		{
			if(nReporterID == 11)
			{
				continue;
			}
		}

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,//new maintain backend
			1, 51, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Disable-Enable
		{
			if(nReporterID == 12)
			{
				continue;
			}
		}

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,//slave ems
			1, 42, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//Disable-Enable
		{
			if(nReporterID == 13)
			{
				continue;
			}
		}

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,//slave ems
			1, 57, &valGet, sizeof(PARAM_VAL_GET));
		if (valGet.val.ulValue == 0)
		{
			if (nReporterID == 14)
			{
				continue;
			}
		}

		TRACE("REPORTER: System Type =%d Reporter Name =%s\n",valGet.val.emValue, pReporter->info.szReporterName);

		if(!LoadReporterLib(pReporter))
		{
			TRACE("Load szReporterName=%s Failed\n", pReporter->info.szReporterName);

			return FALSE;
		}

		TRACE("Init szReporterName=%s\n", pReporter->info.szReporterName);
		if(!pReporter->Init((HANDLE)pReporter))
		{
			LogOut(MAIN_MODULE_MAIN, "InitReporter", LOG_TYPE_ERROR,
				"Fails on initializing the Reporter --> [%s], please check!",
				pReporter->info.szReporterName);
			return FALSE;
		}
	}
	return TRUE;
}
