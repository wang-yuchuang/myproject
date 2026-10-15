#ifndef __REPORTER_ES_IEC101_H
#define __REPORTER_ES_IEC101_H
 
#include "std_sys.h"
#include "app_incs.h"

#include "../../../include/3rd_party/libiec104/cs101_slave.h"
#include "../../../include/3rd_party/libiec104/hal_thread.h"
#include "../../../include/3rd_party/libiec104/hal_time.h"
#include "../../../include/3rd_party/libiec104/hal_serial.h"

//#define TRACE		printf
//// trace with function and line number
//#define TRACEX		printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

typedef struct  strIec101DataRough
{
	HANDLE						hReporter;

	//HANDLE					hqMsg2Sent; 

	HANDLE						hMutex;
	
}IEC101_DATA_ROUGH;
 
//-----------------------End----------------------//

void printCP56Time2a(CP56Time2a time);
void rawMessageHandler (void* parameter, uint8_t* msg, int msgSize, bool sent);
bool clockSyncHandler (void* parameter, IMasterConnection connection, CS101_ASDU asdu, CP56Time2a newTime);
bool interrogationHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu, uint8_t qoi);
bool asduHandler(void* parameter, IMasterConnection connection, CS101_ASDU asdu);
void resetCUHandler(void* parameter);
void linkLayerStateChanged(void* parameter, int address, LinkLayerState state);

#define DUMP_ESiOCPPMSG(pReporter, nRW, szData) do{ \
	(pReporter)->pDAI->Set((HANDLE)(pReporter), DAITYPE_COMM_DATA_DUMP, \
	DUMP_SRC_ES_iOCPP, (nRW), (szData), 0, 0);\
	}while(0)

#endif

