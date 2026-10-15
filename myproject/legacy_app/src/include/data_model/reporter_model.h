#ifndef _REPOTER_2018_04_04_H_
#define _REPOTER_2018_04_04_H_

#include "base_defines/basetypes.h"
#include "dai_model.h"

#define REPORTER_INIT			"Initialize"
typedef BOOL (*INIT_PROC)(HANDLE pReporterThis);

typedef struct tagReporterInfo
{
	MODULE_TYPE	emModuleType; //shall put in first place
	int 			nReporterID; //shall put in second place
	char			szReporterName[32];
	char			szReporterDriver[32];
	char			szReporterSetting[64];

}REPORTER_INFO;

typedef struct tagReporter
{
	REPORTER_INFO		info;

	void*					pDataHeap; //for user-defined data

	HANDLE				hSamplerLib;

	COMMPORT*			pUsedCommPort;

	HANDLE				hCommPort;

	HANDLE				hThread;
	BOOL					bExit;

	HANDLE				hqEvent; //event queue for handling

	DAI*					pDAI; //data access interfaces

	BOOL					bCommStatus;
	int					nCommErrors;	// the comm read failure count
	
	INIT_PROC			Init;	//start a single thread
}REPORTER;

#endif
