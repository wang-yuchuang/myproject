#ifndef __REPORTER_ES_IEC61850_H
#define __REPORTER_ES_IEC61850_H
 
#include "std_sys.h"
#include "app_incs.h"

#include "../../../include/3rd_party/iec61850/gx61850v4/iedinf.h"

//#define TRACE		printf
//// trace with function and line number
//#define TRACEX		printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

#define MAX_DI_NUM	6
#define MAX_DO_NUM	6

typedef struct  strIec61850DataRough
{
	HANDLE						hReporter;

	//HANDLE					hqMsg2Sent; 

	HANDLE						hMutex;
	
	A_BLOB_ES_BACKEND_CFG		blobBackendSet;
	
	unsigned short 				DI[MAX_DI_NUM];
	unsigned short 				s_LastDI[MAX_DI_NUM];

    unsigned short 				DO[MAX_DO_NUM];
	unsigned short 				s_LastDO[MAX_DO_NUM];


}IEC61850_DATA_ROUGH;
 
//-----------------------End----------------------//

#define DUMP_ESiOCPPMSG(pReporter, nRW, szData) do{ \
	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
	DUMP_SRC_ES_iOCPP, (nRW), (szData), 0, 0);\
	}while(0)

#endif

