#include <dlfcn.h>		// for dl*()
#include "comm_loader.h"

#ifdef _DEBUG
//#define _DEBUG_HAL_COMM		1
#endif

static BOOL Load_ACommPort(COMMPORT* pCommPort)
{
	static const char *pszSymName[] =
	{
		SYM_HAL_COMMOPEN,		//"HAL_CommOpen"
		SYM_HAL_COMMACCEPT,		//"HAL_CommAccept"
		SYM_HAL_COMMREAD,		//"HAL_CommRead"
		SYM_HAL_COMMWRITE,		//"HAL_CommWrite"
		SYM_HAL_COMMCONTROL,		//"HAL_CommControl"
		SYM_HAL_COMMCLOSE		//"HAL_CommClose"
	};

	HANDLE *pfnProc[] =
	{
		(HANDLE *)&pCommPort->Open,
		(HANDLE *)&pCommPort->Accept,
		(HANDLE *)&pCommPort->Read,
		(HANDLE *)&pCommPort->Write,
		(HANDLE *)&pCommPort->Control,
		(HANDLE *)&pCommPort->Close
	};

	char szFullLibPath[MAX_FILE_PATH];
	//TRACE("Now begin load[%s].\n", pCommPort->info.szPortDriver);
	pCommPort->hLib = LoadDynamicLibrary(MakeFullFileName(szFullLibPath,
		getenv(ENV_VARIABLE_APP_PATH), "comm_port", pCommPort->info.szPortDriver),
		ITEM_OF(pszSymName),
		pszSymName, pfnProc,	// symbol names and addresses
		TRUE);					// all symbols shall be found.

	return (pCommPort->hLib != NULL) ? TRUE : FALSE;
}

BOOL Init_Load_AllCommPorts(CHARGER* pCharger)
{
	int n;
	COMMPORT* pCommPort = pCharger->commports.p;
	printf("[BOOT] CommPorts: total=%d\n", pCharger->commports.nTotal); fflush(stdout);
	for(n = 0; n < pCharger->commports.nTotal; n++, pCommPort++)
	{
		if(pCommPort->info.nCommPort == 0) //means dummy port!
		{
			printf("[BOOT] CommPorts: [%d/%d] %s - dummy (skip)\n", n+1, pCharger->commports.nTotal, pCommPort->info.szPortName); fflush(stdout);
			continue;
		}
		printf("[BOOT] CommPorts: [%d/%d] loading %s (%s)...\n", n+1, pCharger->commports.nTotal, pCommPort->info.szPortName, pCommPort->info.szPortDriver); fflush(stdout);
	if(!Load_ACommPort(pCommPort))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadCommPorts", LOG_TYPE_ERROR,
			"Can NOT load the comm port [%s], please check!", pCommPort->info.szPortName);
		return FALSE;
	}
	printf("[BOOT] CommPorts: [%d/%d] %s loaded OK\n", n+1, pCharger->commports.nTotal, pCommPort->info.szPortName); fflush(stdout);
	}
printf("[BOOT] CommPorts: all loaded\n"); fflush(stdout);
return TRUE;
}
