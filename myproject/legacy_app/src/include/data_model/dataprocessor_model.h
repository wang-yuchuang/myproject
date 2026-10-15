#ifndef _DATAPROCESSOR_2018_04_04_
#define _DATAPROCESSOR_2018_04_04_

#include "base_defines/basetypes.h"
#include "data_model/event_model.h"
#include "data_model/device_model.h"

typedef enum tagErrDPR
{
	ERR_DPR_OK = 0,
	ERR_DPR_INVALID_ARG,
	ERR_DPR_NO_MEMORY,
	ERR_DPR_WRONG_SIZE,
	ERR_DPR_RAW_INVALID,
	ERR_DPR_TIMEOUT,
	ERR_DPR_SAMPLER_COMMFAIL,
	ERR_DPR_MUTEX_ERR,
	ERR_DPR_REPEATED_EVHAND,
	ERR_DPR_TOOMANY_EVHAND,
	ERR_DPR_QUEUE_FULL,
	ERR_DPR_DB_ERR,
	ERR_DPR_NO_DATA,
	ERR_DPR_NOTSUPPORTED, // added by wangdong 20230625 for return ctrl unlock status
}ERRCDDE_DPR;

typedef BOOL (*LOADCFG_PROC)(HANDLE pDataDB);
typedef int (*PROCESSEVENT_PROC)(HANDLE hEvent);
typedef BOOL (*STARTSTOPDATA_PROC)(BOOL bStart);  //start = TRUE means begin, or else means end
typedef int (*REGISTER_EVHAND)(char* pszFuncName, 
				HANDLE hModule, 
				EVENT_HANDLER pfnHandler, 
				UINT nEventFlag, 
				UINT nMsgFlag, 
				BOOL bIsUnregister);

typedef enum tagInternalGetType
{
	GETTYPE_SP_RAW_VAL, //status parameter raw float value
	GETTYPE_SP_VAR, //status parameter varvalue
	GETTYPE_ST_VAR , //setting parameter varvalue
	GETTYPE_CL_VAR , //control parameter varvalue
	GETTYPE_READBACK_VAR,
	GETTYPE_CHARGE_REC, //get charge record from DB
	GETTYPE_METERVALUE_REC, //get meter values record from DB
	GETTYPE_ALMSTATE,
	GETTYPE_APPVER,
	GETTYPE_COMMPORT
}INTERGET_TYPE;

typedef ERRCDDE_DPR (*INTER_GET)(IN HANDLE hSender,		//which module send this
					IN INTERGET_TYPE emGetTp, 
					IN int nDevId, 
					IN int nParamId,
					IN OUT VAR_VALUE* pBuff,
					IN int nBuffSize);

typedef struct tagInterSetData
{
	int	nWhichSingleModule;//only for PCS Reread to find which single module param
	int nDevId;
	int nParamId;
	VAR_VALUE varValue;
	BOOL	 bRawInvalid; //only for raw, if raw is invalid ,set this flag
	int nValueSize;
	int nTimeOut; //in mSec, 0 means not async
}INTERSET_DATA;

typedef enum tagInternalSetType
{
	SETTYPE_RAW_SP,	//push to raw status param
	SETTYPE_LOGIC_SP, //set LS type status parameter
	SETTYPE_INTERN_SETT, //internal sett parameter, to other sampler or logic setting
	SETTYPE_RAW_ST, //just refresh setting params and save but not sync
	SETTYPE_INTERN_CTRL, //internal ctrl parameter, to other sampler
	SETTYPE_SENDMSG, //send out msg to reporter / dpr
	SETTYPE_RAW_BACK_CTRL, //copy back value for CTRL
	SETTYPE_RAW_BACK_SETT, //copy back value for SETT
	//restore settings from SD card
	SETTYPE_RESTORE_SETTINGS_FROMDB,
	//clear factory debug data
	SETTYPE_CLEAR_FACTOY_DEBUG_DATA,
	//added by Jimmy for data clone
	SETTYPE_OVERWRITE_SET_BEFORE_CLONE,
	SETTYPE_READ_VALUE,
	SETTYPE_Reread_PCS_Param //push Reread PCS Param data to set param
}INTERSET_TYPE;

typedef struct tagQInterCtrlCmd
{
	INTERSET_TYPE		emType;
	INTERSET_DATA	data;
}INTER_QUEUE_CMD;

typedef ERRCDDE_DPR (*INTER_SET)(IN HANDLE hSender,		//which module send this
					IN INTERSET_TYPE emSetTp, 
					IN OUT void* pBuff,
					IN int nBuffSize);

typedef ERRCDDE_DPR (*CALCULATE_EXPRESSION)(IN PARAMETER* pParam,
					IN char* szExpression, 
					CLUSTER_EXP_ELEMENT *pExpSet, 
					BOOL bParseExpSet, //first time parse, then no need
					OUT void* pResult);

typedef struct tagDataProcInfo
{
	MODULE_TYPE	emModuleType; //shall put in first place
	int 			nDataProcID; //always zero shall put in second place
	char			szDataProcName[32];	

}DATAPROC_INFO;

typedef struct tagDataProc
{
	DATAPROC_INFO		info;
	HANDLE				hThread;
	BOOL					bExit;
	//following interfaces are for main to call
	LOADCFG_PROC			LoadConfigAndData;		//Load configuration and data(his setting/his event /his data)
	STARTSTOPDATA_PROC	DoDataProcessing;		//create and start a single thread to process all the data
	PROCESSEVENT_PROC	DoEvent;				//start to handle an Event from a queue
	//following interfaces are for samplers to call
	INTER_GET			Get; 
	INTER_SET			Set;
	CALCULATE_EXPRESSION	CalculateExp;
	REGISTER_EVHAND		Register_EvHandler;
}DATA_PROCESSOR;

#endif
