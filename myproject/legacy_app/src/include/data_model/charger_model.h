#ifndef _CHARER_MODEL_2018_04_02_
#define _CHARER_MODEL_2018_04_02_

#include "base_defines/basetypes.h"
#include "data_model/dai_model.h"
#include "data_model/dataprocessor_model.h"
#include "data_model/reporter_model.h"
#include "data_model/sampler_model.h"
#include "comm_ports/comm_port.h"
#include "data_model/hisdata_model.h"

DEFINE_CLUSTER(DEVICE_MODEL_INFO);
DEFINE_CLUSTER(STATUS_PARAM_INFO);
DEFINE_CLUSTER(CONTROL_PARAM_INFO);
DEFINE_CLUSTER(SETTING_PARAM_INFO);
DEFINE_CLUSTER(ALARM_INFO);
DEFINE_CLUSTER(HISPARAM_INFO);
DEFINE_CLUSTER(RESOURCE_TEXT);

DEFINE_CLUSTER(DEVICE);
DEFINE_CLUSTER(SAMPLER);
DEFINE_CLUSTER(REPORTER);
DEFINE_CLUSTER(COMMPORT);

typedef struct tagCharger_Revision
{
	char			szSWVersion[MAX_LEN_MU2SWVER];
	//added by Jimmy 20200307 for remote update
	int				nPkgVersion;
	char			szLCCUVersion[16];
	_DATE_TIME		dtReleaseDate;
	char			szConfAuthor[32];
	char			szUpdateDetails[128];
}CHARGER_REVISION;
DEFINE_CLUSTER(CHARGER_REVISION);

typedef struct tagChargerInfo
{
	//revision list
	CLUSTER_CHARGER_REVISION		charger_revision_list;
	char							szSWVer[MAX_LEN_MU2SWVER];
	int								nPkgVer;
	//resources
	CLUSTER_RESOURCE_TEXT			resource_list;
	//config from db
	CLUSTER_DEVICE_MODEL_INFO		devModels_info;
	CLUSTER_STATUS_PARAM_INFO		statusParams_info;
	CLUSTER_CONTROL_PARAM_INFO	controlParams_info;
	CLUSTER_SETTING_PARAM_INFO	settingParms_info;
	CLUSTER_ALARM_INFO			alarms_info;
	//historical parameters who need persistence
	CLUSTER_HISPARAM_INFO			hisParams_info;

}CHARGER_INFO;

typedef struct tagModulePTR
{
	MODULE_TYPE	emModuleType; //shall put in first place
	int 			nModuleID; //shall put in second place
}MODULE_PTR;

//DEFINE_LINKTABLE(ACTIVE_EVENT);

typedef struct tagCharger
{
	CHARGER_INFO					info;

	CLUSTER_DEVICE				devices;
	CLUSTER_SAMPLER				samplers;
	CLUSTER_REPORTER				reporters;
	CLUSTER_COMMPORT				commports;
#define SYS_EXIT_NOTEXIT			0	//normal, no exit!
#define SYS_EXIT_BY_USER			3	// killed/stopped by user.
#define SYS_EXIT_BY_ERROR			127
#define SYS_EXIT_QT_NORESP			150 //qt no response
#define SYS_EXIT_BY_INIT			128	// for init error.
#define SYS_EXIT_BY_THREAD			129	// for abnormal threads.
#define SYS_EXIT_BY_THREADMANAGER	130	// for thread manager
#define SYS_EXIT_BY_FAULT			131 // for fault errors, such as segment fault.
	BOOL					bExit; //every thread will be exited while this flag is true
	int					nExitCode;//Exit Reason
	HANDLE				hRunDB; //Runtime DB
#define MAX_EVENTS_QUEUE	96
	HANDLE				hqEvent; //Event queue
	BOOL					bIsInitOK;

	DATA_PROCESSOR		dataprocessor;
	DAI					dai;


	BOOL				bResetCmdPending;
	//active event linktable
	//int					nActiveEventTotal;
	//LINKTABLE_ACTIVE_EVENT *pActEventHead;
	//HIS_ES_STATISTIC_YEAR	esStatData; //energy storage statistic
	
}CHARGER;

#define CHARGER_CONFIG_DB	"System.cfg"

#endif
