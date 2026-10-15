#ifndef _REPOTER_2018_04_04_
#define _REPOTER_2018_04_04_

#include "base_defines/basetypes.h"
#include "event_model.h"

typedef enum tagDAIType
{
	//Structure(get by Idx)
	DAITYPE_DEV_TOTAL_R,
	DAITYPE_DEV_INFO_R,
	DAITYPE_PARAM_TOTAL_R,
	DAITYPE_PARAM_INFO_R,
	DAITYPE_ALARM_TOTAL_R,
	DAITYPE_ALARM_INFO_R,
	//App version
	DAITYPE_APP_VRESION_R,
	DAITYPE_PKG_VRESION_R, //added by Jimmy 20200307
	//RT Data
	DAITYPE_STAT_PARAMVALUE_R, //Get status parameter value
	DAITYPE_SETT_PARAMVALUE_RW, //Get/Set setting parameter value
	DAITYPE_CTRL_PARAMVALUE_RW, //Set control parameter value
	DAITYPE_READBACK_VAL,
	DAITYPE_ALARMSTAT_R, //Get an alarm status
	//Local List(for OCPP)
	DAITYPE_GETLOCALLIST_VERSION,
	DAITYPE_GETLOCALLIST_TOTAL,
	DAITYPE_GETLOCALLIST_ITEMS,
	//Comm port for use
	DAITYPE_COMMPORT_R,
	//Reporter set Virtual statParam
	DAITYPE_VIRTUAL_PARAMVALUE_W,
	//Comm(OCPP) data dump
	DAITYPE_COMM_DATA_DUMP,
	//added by Jimmy 20210426 for MaintainBE reporter
	DAITYPE_GETDEVNAME_BYID_R,
	DAITYPE_GETAALARMNAME_BYID_R,
	DAITYPE_GETAALARM_LEVEL_BYID_R,
	DAITYPE_PARAM_INFO_BY_PARAMID_R //added by Jimmy 2022.325
}DAI_TYPE;

typedef enum tagQueryType
{
	QUERYTYPE_CHARGEREC,
	QUERYTYPE_ALARM,
	QUERYTYPE_HISEVENT,
	QUERYTYPE_HISDATA,
	QUERYTYPE_METERVALUS,
	QUERYTYPE_ACTV_ALARM

}QUERY_TYPE;

typedef struct tagChargeRecQueryCond
{
#define QUERY_COND_CHARGE_STARTTIME		0x01
#define QUERY_COND_CHARGE_USER				0x02
#define QUERY_COND_CHARGE_GUN				0x04
#define QUERY_COND_CHARGE_CARVIN			0x08
#define QUERY_COND_CHARGE_STARTMETHOD		0x10
#define QUERY_COND_CHARGE_TRANSID			0x20 //Transaction ID
#define QUERY_COND_REPORTED_FLAG			0x40 //if reported OK?
	int		nTransID;
	int		nWhichGun;
	time_t	tmChargeTime_Begin;
	time_t	tmChargeTime_End;
	char		szUserId[64];
	char		szCarVIN[64];
	int		nChargeStartMethod;
	UINT		nReportFlag;

}QUERYCOND_CHARGERECORD;

typedef struct tagAlarmQueryCond
{
#define QUERY_COND_ALARM_STATUS		0x01
#define QUERY_COND_ALARM_PTR			0x02
#define QUERY_COND_ALARM_STARTTIME		0x04
	ALARM_PTR	alarmPTR;
	ALARM_STATUS	almStatus;
	time_t		tmAlarmStartTime_Begin;
	time_t		tmAlarmStartTime_End;
}QUERYCOND_ALARM;

typedef struct tagEventQueryCond
{
#define QUERY_COND_EVENT_TYPE		0x01
#define QUERY_COND_EVENT_STATUS	0x02
#define QUERY_COND_EVENT_STARTTIME	0x04
	int		nEventType;
	int		nEventStatus;
	time_t	tmEventStartTime_Begin;
	time_t	tmEventStartTime_End;
}QUERYCOND_EVENT;

typedef struct tagHisDataQueryCond
{
#define QUERY_COND_HISDATA_PTR		0x01
#define QUERY_COND_HISDATA_RECTIME	0x02
	STATPARAM_PTR	parmPTR;
	time_t			tmRecTime_Begin;
	time_t			tmRecTime_End;
}QUERYCOND_HISDATA;

typedef struct tagHisMeterValQueryCond
{
	int		nGunIdx;
	time_t	tmChargeStart;
}QUERYCOND_HISMETERVAL;

typedef struct tagQueryCondition
{
	int		nQueryCondType; //flag can be AND/OR different cond, 0 means no cond
	//int		nOrderType; //order by xxx, default by time DESC
	int		nArgSize;
	void*		pCondArg;
}QUERY_COND;

typedef enum tagDAIErrcode
{
	ERRCODE_DAI_OK = 0,
	ERRCODE_DAI_INVALID_ARG,
	ERRCODE_DAI_NOTSETTABLE,
	ERRCODE_DAI_NOTCONTROABLE,
	ERRCODE_DAI_ARG_MEMNOTENOUGH,
	ERRCODE_DAI_EXECUTE_ERR,
	ERRCODE_DAI_TIMEOUT,
	ERRCODE_DAI_NOTSUPPORTED, // added by wangdong 20230625 for return ctrl unlock status
}ERRCODE_DAI;

typedef struct tagGetValue
{
	int		nGetSize;
	VAR_VALUE val;//if value is invalid ,then this is the last valide value (and time)
#define VALUE_ATTRIBUTE_VALID		0x01
#define VALUE_ATTRIBUTE_DISPLAY		0x02
#define VALUE_ATTRIBUTE_CTRLSETABLE	0x04
	UINT		nValueAttribute;
	time_t	tmRefresh;
}PARAM_VAL_GET;

typedef ERRCODE_DAI (*DAI_GET)(IN HANDLE hSender,
			IN DAI_TYPE emInterfaceType, IN int nArg1, IN int nArg2,
			IN OUT void* pBuffer, IN UINT nBuffSize);
#define  DAI_NONEBLOCK_MODE	(-1)
typedef ERRCODE_DAI (*DAI_SET)(IN HANDLE hSender,
	IN DAI_TYPE emInterfaceType, IN int nArg1, IN int nArg2,
	IN OUT void* pBuffer, IN UINT nBuffSize, /*BOOL bBlockMode, */IN int nTimeout); //nTimeout = -1 means Non-Block Mode

typedef ERRCODE_DAI (*DAI_QUERY)(IN HANDLE hSender,
	IN QUERY_TYPE emQueryType, IN QUERY_COND cond,
		IN OUT void* pBuffer, IN OUT UINT *pnBuffSize);

typedef ERRCODE_DAI (*DAI_SIGNAL)(IN HANDLE hSender, IN EVENT_ARG* pArg);

typedef ERRCODE_DAI (*DAI_REGISTER_EVHAND)(char* pszFuncName, 
					HANDLE hModule, 
					EVENT_HANDLER pfnHandler, 
					UINT nEventFlag, 
					UINT nMsgFlag, 
					BOOL bIsUnregister);

//typedef ERRCODE_DAI (*DAI_CALC_EXPRESSION)(IN HANDLE hSender,
//					IN PARAMETER* pParam,
//					IN char* szExpression, 
//					CLUSTER_EXP_ELEMENT *pExpSet, 
//					BOOL bParseExpSet, //first time parse, then no need
//					OUT void* pResult);
//typedef void (*DAI_FREE_EXPRESSION)(IN CLUSTER_EXP_ELEMENT *pExpSet);

typedef struct tagDAI
{
	DAI_GET				Get;
	DAI_SET				Set;
	DAI_QUERY			Query;
	DAI_SIGNAL			Signal;
	DAI_REGISTER_EVHAND	Regsiter_EvHandler;
	//DAI_CALC_EXPRESSION	CalculateExp; //do calculating
	//DAI_FREE_EXPRESSION	FreeExp; //shall be called when expression not in use any more
}DAI;

#endif
