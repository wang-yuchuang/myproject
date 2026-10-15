#ifndef _EVENT_MODEL_2018_03_27_
#define _EVENT_MODEL_2018_03_27_

#include "base_defines/basetypes.h"
#include "data_model/device_model.h"
#include "data_model/blob_model.h"
#include "data_model/hisdata_model.h"

#define MAX_EVENT_RECORDS		10000
#define MAX_ALARM_RECORDS		5000
#define MAX_CHARGE_RECORDS		3000
#define MAX_ES_STATDAILY_DATA	1 
#define MAX_BMS_RECORDS_NUM		10000
#define MAX_HEALTH_RECORDS_DAY  365*5
#define MAX_METER_DATA_RECORDS_DAY  365*2   //two meters 0ne year

typedef enum tagEvent_Type
{
	//alarms
	EVENT_TYPE_ALARM,
	//sys config changed
	EVENT_TYPE_CFG_CHANGED,
	//charge
	EVENT_TYPE_CHARGING,
	//User authorize
	EVENT_TYPE_USER_AUTHORIZE,
	//system exit
	EVENT_TYPE_SYSTEM_EXIT,
	//comm status
	EVENT_TYPE_COMM_FAIL, //for dpr only
	//Charge record update
	EVENT_TYPE_CHARGEREC_UPDATE, //for dpr only
	//firmware update
	EVENT_TYPE_FIRMWARE_UPDATE,
	//diagnostic uploading
	EVENT_TYPE_DIAGNOSTIC_UPLOAD,
	//in maintenance mode
	EVENT_TYPE_IN_MAINTAINING,
	//gun/system ability change
	EVENT_TYPE_ABILITY_CHANGE,
	//meter values periodically update (int charging)
	EVENT_TYPE_METERVAL_UPDATE_IN_CHARGING,
	//UI general notice
	EVENT_TYPE_UI_NOTICE,
	//Reserve should be restored after a reboot
	EVENT_TYPE_GUN_RESERVE,
	//Save ES statistic data
	EVENT_TYPE_SAVE_ES_STATDATA,

	EVENT_TYPE_CHRG_DI_CHANGED,
	//added by John 20210930 for BMS Diagnctic
	EVENT_TYPE_BMSRUNDATA_UPDATE,

	//added by John 20220513 for BMS Health Static
	EVENT_TYPE_HEALTHDATA_UPDATE,

	//Save Accumulated Data
	EVENT_TYPE_SAVE_ACCUMULATED_DATA,

	//added by LPL 20240730 for saving current SPFW kwh
	EVENT_TYPE_KWHMETER_CURRENT_POS_NEG_SPFW,

	EVENT_TYPE_REFRESH_ALARM,
	
	EVENT_TYPE_BMS_HISTFAULT,
	EVENT_TYPE_BMS_HISTEVENT,
	//added by Jimmy 20200526, just special for identifying that the DB has 
	//been compressed successfully
	EVENT_TYPE_DB_COMPRESSED = 255,

}_EVENT_TYPE;

typedef enum tagEventMsg
{
	EVENT_MSG_BEGIN				=0x01,
	EVENT_MSG_STATUS_CHANGED		=0x02, //for some event, during process, status may change, e,g OCPP shall issue this msg after Authorization done!
	EVENT_MSG_END				=0x04,
	EVENT_MSG_RESTORE			=0x08, //restore when power reset
	EVENT_MSG_NA					=0xFF  //means no type, e,g cfg change
}_EVENT_MSG;

typedef enum tagEventObjectType
{
	EVENT_OBJ_NOT_SPECIFIED = 0, //when set this flag, means the receiver can be any module that is interested in the event
	EVENT_OBJ_SAMPLER,
	EVENT_OBJ_REPORTER,
	EVENT_OBJ_DATAPROCESSOR,
}_EVENT_OBJ_TYPE;

#define EVENT_STATUS_OCCUR			1
#define EVENT_STATUS_NOT_OCCUR		0

//typedef struct tagEventObjectPTR
//{
//	_EVENT_OBJ_TYPE	emObjType;
//	int				nObjSubId; //-1 means not specified
//}EVENT_OBJ_PTR;

typedef void (*EVENT_RECEIPT)(HANDLE hSender, void* pEventCallBackArg);

typedef struct tagEventArg
{
	//EVENT_OBJ_PTR	ptrReciver;
	_EVENT_TYPE		emEvType;
	_EVENT_MSG		emMsgType;
	UINT				nEvDataSize;
	void*				pEvData;
	BOOL				bHighPriority; //high priority msg will be put head of the queue
	//if need return after the event to be handled. then the event shall be send to receiver and get a reply.
	BOOL				bBlockMode; //block mode will block the sender process
	int				nMaxBlockMSec;
	//the following for sender calling back
	HANDLE			hSender;
	void*				pSenderCallBackArg; 
	EVENT_RECEIPT		pfnReceipt; //when handled, the receiver can callback to notice the result directly.
}EVENT_ARG;

#define EVENT_HANDED_OK		0
#define EVENT_HANDED_FAILED	1
#define EVENT_HANDED_TIMEOUT	2

typedef int (*EVENT_HANDLER)(HANDLE		hModuleSeif,
						_EVENT_TYPE	emEvType,
						_EVENT_MSG	emMsgType,
						UINT			nEvDataSize,
						void*			pEvData,
						BOOL			bBlockMode,
						int			nMaxBlockMSec);

typedef struct tagEventHandlerRegister
{
	char				szName[32];	//just Display string for debug
	HANDLE			hModuleSelf;	//for sender to use when call back
	EVENT_HANDLER	pfnEvHandler;
	UINT				nEvTypeFlag;	//what types of event is the module interested?
	UINT				nMsgTypeFlag;	//what types of event msg is the module interested?
}EVENT_HANDLER_REGISTER;

//typedef struct tagActiveEvent
//{
//	HANDLE			hSender;
//	_EVENT_TYPE		emEvType;
//	UINT				nEvDataSize;
//	void*				pEvData;
//	double			tmF_Start;
//}ACTIVE_EVENT;

typedef struct tagEventCharging
{
	//following information just for identify which charge session
#define EVENT_FOR_CONN_0	0
	BYTE			emGunNum; //1--A, 2--B ......
#define CHG_TYPE_GUN_PLUG		55
#define CHG_TYPE_GUN_AVAILB	1
#define CHG_TYPE_GUN_INCHARGE	2
#define CHG_TYPE_SYS_STATE		3
#define CHG_TYPE_GUN_PREPARE_STAT	4
#define CHG_TYPE_SYS_INEXIT		5 //notice reporter that system is now exiting...
	BYTE			emChangeType;

	//following are foe event restoring
	time_t		tmChargeStart;
	int			nSessionID;

	void*			pRestoreChgData; //this is a pointer for transmiting the restored chargeData
}EVENT_CHARGING;

typedef struct tagBmsHistFault
{
	int		nBattGrp;		// 电池组号 0=组1, 1=组2
	int		nFaultType;		// 0:无效 1:告警产生 2:告警消除 3:故障产生 4:故障消除
	int		nFaultCode;		// 故障代码 (e_ERR_ID)
	time_t	tmFault;		// 故障时间（Unix时间戳）
	int		nProtectState;	// 系统保护状态 [0,5]
	int		nWorkState;		// 系统工作状态 [0,6]
	int		nCurrent;		// 系统输出电流[0.1A] (附件信息2)
	int		nCustom3;		// 用户自定义 (附件信息3)
	int		nCustom4;		// 用户自定义 (附件信息4)
	int		nCustom5;		// 用户自定义 (附件信息5)
	int		nStatus;		// 0=已恢复(消除) 1=未恢复(产生)
}EVENT_BMS_HISTFAULT;

typedef struct tagBmsHistEvent
{
	int		nBattGrp;		// 电池组号 0=组1, 1=组2
	int		nEventType;		// 事件类型 (5~14 用户自定义=100, 见 eEvent_RcdSort)
	int		nEventCode;		// 事件代码 (用户自定义时见 eUserEvn_RcdId)
	time_t	tmEvent;		// 事件时间（Unix时间戳）
	int		nProtectState;	// 系统保护状态 [0,5]
	int		nWorkState;		// 系统工作状态 [0,6]
	int		nCurrent;		// 系统输出电流[0.1A] (附件信息2)
	int		nCustom3;		// 用户自定义 (附件信息3)
	int		nCustom4;		// 用户自定义 (附件信息4)
	int		nCustom5;		// 用户自定义 (附件信息5)
}EVENT_BMS_HISTEVENT;

/*
typedef struct tagHisAlarm
{
	int		nDevId;
	int		nAlarmId;
	ALARM_STATUS	status;
	float		fExParamVal;
	time_t	tmStart;
	time_t	tmEnd;
}EVENT_ALARM;
*/
typedef struct tagHisParam
{
	HANDLE			hSender;
	SETTPARAM_PTR	parmPTR;
}EVENT_PARAM_CHG;

typedef enum tagAuthResult  //1
{
	ACCEPTED = 0,
	BLOCKED,
}AUTHRESULT;

typedef struct tagAuthorize
{
	//int		nConnectorId; //should not be required
 	char		szIdTocken[64];
    char     szIdParent[64];
	float		fCardLeftMoney;
#define AUTH_SRC_LOCAL				0
#define AUTH_SRC_LCDPASSWD		1
#define AUTH_SRC_SERVER			2
	int		nWhereFrom; //0--Local user card, 1--Local LCD passwd, 2--From center system
#define AUTH_DO_LOCALLIST			0
#define AUTH_DO_AUTHCACHE			1
#define AUTH_DO_OCPP				2
#define AUTH_DO_TX_UNKONW			3
#define AUTH_DO_STOPTRANS_SAMEID	4
	int		nWhereDoAuthorizing; //0--Local list, 1--Authorization cache, 3--Center system
	UINT		uFlagUsed4StopConn; //only valid when AUTH_DO_STOPTRANS_SAMEID, point to which connector is valid
							//Conn1-bit0 = 1, Conn2-bit1 = 1, Conn3-bit2 = 1...
	AUTHRESULT		emResult;  //Authorization result, true = Accepted
}EVENT_AUTHORIZE;

//typedef struct tagEventCommFail
//{
//#define COMMFAIL_TYPE_DEVICE	0
//#define COMMFAIL_TYPE_SAMPLER	1
//#define COMMFAIL_TYPE_REPORTER	2
//	int		nFailType; //0--device comm fail, 1--sampler comm fail, 2--reporter comm fail
//	int		nFailObjId;
//}EVENT_COMMFAIL;

typedef struct tagEventAbilityChange
{
#define ABILITY_SET_ALL_GUN	0
	int		nWhichGun;	//0-means all
	//int		nWhoLocked;//0--Local, 1--Remote Center System
	//int		nWhoUnLocked;//0--Local, 1--Remote Center System
}EVENT_ABILITY_CHG;


//Bolow Status is reported by BMS
typedef enum tagBMSReportSTATE
{
	Stat_Downloaded = 0,
	Stat_Download_Fail,
	Stat_Downloading,
	Stat_Idle,
	Stat_Install_Fail,
	Stat_Installing,
	Stat_Installed,
	Stat_Other
 }BMS_REPORT_STATE;

typedef enum tagSWUpgradeStatus
{
	UPGRADE_IDLE = 0,
	UPGRADE_READBINFILE,
	UPGRADE_HANDSHAKE,
	UPGRADE_UNLOCK,
	UPGRADE_EXCHANGE_KEY,
	UPGRADE_WRITE_REQUEST,
	UPGRADE_WRITE_REQUEST_2,
	UPGRADE_ROUTINE_CONTROL,
	UPGRADE_DOWNLOAD_REQUSET,
	UPGRADE_DOWNLOAD_FILE,
	UPGRADE_DOWNLOAD_EXIT,
	UPGRADE_ROUTINE_EXIT,
	UPGRADE_CHECK_FILE,
	UPGRADE_CHECK_COMPATIBILITY,
	UPGRADE_RESET_ECU,
	UPGRADE_WAIT_BEGIN,
	UPGRADE_PENDING,
	UPGRADE_FINISHED

}SW_UPGRADE_STATUS;



typedef enum tagHttpFtpUploadDownloadStatus
{
	UPDOWN_IDLE,
	UPDOWN_WAIT_BEGIN,
	UPDOWN_PENDING,
	UPDOWN_FINISHED,
	UPDOWN_WAIT_REBOOT,
	UPDOWN_WAIT_REBOOT_AND_CCU //added by Jimmy 20210401 for CCU update
}HF_UPDOWN_STATUS;

#define LOCAL_UPGRADE_IMSU_NAME	"/opt/appdata/data/IMSU_APP.bin"
#define LOCAL_UPGRADE_FIRMWARE_NAME	"/opt/appdata/data/nccu_app.rpkg"
#define MAX_UPDOWN_FILESIZE	20*1024*1024  //MAX 20M
typedef struct stUpdownFileInfo
{
	BYTE        	szFileName[30];
    FILE            *fp;
	ULONG			nTotalFileSize;
	int 			nDataOffset;
	int				nFileHandle;
	BYTE			*pbyReadBuf;

}UPDOWN_FILE_INFO;
 
typedef enum tagFirmwareUpdateStatus
{
	FIRMUPDATE_Downloaded,
	FIRMUPDATE_DownloadFailed,
	FIRMUPDATE_Downloading,
	FIRMUPDATE_Idle, //can only be replied when CS Triggled firwareupdatestatus.req
	FIRMUPDATE_InstallFailed,
	FIRMUPDATE_Installing,
	FIRMUPDATE_Installed
}FIRM_UPDATESTT;

typedef struct tagMasterUpgradeParam
{
	SW_UPGRADE_STATUS		emUpgradeStatus;	
	BOOL 					bInterSlvReady4Upgrade;
	int						nMsgId;
	int						nRecvMsgId;
	FIRM_UPDATESTT			emSlvUpdateStatus;
	time_t 					tm_StartUpgrade;

}MASTER_UPGRADE_PARAM;

typedef struct tagEventFirmwareUpdateStatus
{
	FIRM_UPDATESTT				emUpdateStatus;
	char							szOldFirmwareVer[MAX_LEN_MU2SWVER]; //just for reboot comparing
	char							szNewFirmwareVer[MAX_LEN_MU2SWVER];
	A_BLOB_UPDATEFIRMWARE_CMD	stUpdateCmd; //just stored in case crash when downloading

}EVENT_FIRMWAREUPDATE_STATUS;

typedef enum tagDiagnosticUploadStatus
{
	DIAGNOUPLOAD_Idle, //can only be replied when CS Triggled firwareupdatestatus.req
	DIAGNOUPLOAD_Uploaded,
	DIAGNOUPLOAD_UploadFailed,
	DIAGNOUPLOAD_Uploading
}DIAGNOS_UPLOAD_STATUS;

typedef struct tagEventDiagnoticUploadStatus
{
	DIAGNOS_UPLOAD_STATUS		emUploadStatus;
}EVENT_DIAGNOUPLOAD_STATUS;

 
typedef struct tagEventMeterValusUpdate
{
#define METER_VAL_UPDATETYPE_BEGIN		1
#define METER_VAL_UPDATETYPE_PENDING	2
#define METER_VAL_UPDATETYPE_END		3
	BYTE						emMeteVal_UpdateType;
	int						nUpdateIdx;
	HIS_METER_VALUE_REC		*pMeterRecord; //note this pointer should never be deleted!!!
}EVENT_METERVALUE_UPDATE;

typedef struct tagEventChgRunDataUpdate
{
#define BMSRUNDATA_UPDATETYPE_BEGIN		1
#define BMSRUNDATA_UPDATETYPE_PENDING	2
#define BMSRUNDATA_UPDATETYPE_END		3
	BYTE							emBmsRunData_UpdateType;
	HIS_BMS_RUNTIMEDATA_REC		*pBmsRunDataRec; //note this pointer should never be deleted!!!
}EVENT_BMSRUNDATA_UPDATE;


typedef struct tagEventHealthDataUpdate
{
	#define BMSHEALTH_UPDATETYPE_BEGIN		1
	#define BMSHEALTH_UPDATETYPE_PENDING	2
	#define BMSHEALTH_UPDATETYPE_END		3
	BYTE							emHealth_UpdtTP;
	HIS_HEALTH_STATIC_DAILY			*pHisBmsHealthRec; //note this pointer should never be deleted!!!

}EVENT_BMSHEALTH_UPDATE;


typedef struct tagEventSysExit
{
#define SYS_EXIT_HARD_REBOOT	0
#define SYS_EXIT_SOFT_REBOOT	1
#define SYS_EXIT_FIRMWAREUPDT	2
#define SYS_INTERNAL_ERROR		3
	int		nExitType;
}EVENT_SYSTEM_EXIT;

typedef struct tagEventUiNotice
{
#define UI_NOTICETYPE_FIRMWARE_UPDATE	1
#define UI_NOTICETYPE_ADVERTISEMENT		2
#define UI_NOTICETYPE_AUTHRIZE_TIMEOUT	3
#define UI_NOTICETYPE_IMSU_UPDATE	4
#define UI_NOTICETYPE_BMS_UPDT    5 //added by John 20210702 for BMS online update
#define UI_NOTICETYPE_SYSCFG_CHANGED    6 //added by Jimmy 20200702 for system-cfg changed
#define UI_NOTICETYPE_ONECLICK_SETTINGS	7
	int	emNotoceType;
	BYTE	byData[256];
}EVENT_UI_NOTICE;

typedef struct tagEventBMSMgmt
{
	int 				nBattIdx;

	STAT_BMSMGMT_STATUS emLast_MgmtStatus;
	STAT_BMSMGMT_STATUS emCurrtMgmtStatus;
	
	CMD_BMSMGMT_PCSCTRL	emMgmtPcsCtrlCmd;
	REASON_BMSMGMT_EXEC	emMgmtExecReason;
	//BYTE 				byData[32];

}EVENT_BMSMGMT_STATCHANGE; 

typedef struct tagChrgDIStat
{
	int		emDIType;
	int		nGunIdx; //0--A,1--B...
	BYTE	byData[256];
}EVENT_CHRGDI_STAT;

typedef struct tagESStatDataSaveEv
{
	HIS_ES_STATISTIC_DAILY* pDailyData;
}EVENT_ES_SAVE_STATDATA;

typedef struct tagAccumulatedData
{
	EFFICIENCY_DATA_DAILY*	pEfficiency_data;
}EVENT_ACCUMULATED_DATA;

typedef enum tagAccumulatedDataType
{
	TYPE_INPUT_KWH_D,
	TYPE_OUTPUT_KWH_D,
	TYPE_INPUT_KWH_M,
	TYPE_OUTPUT_KWH_M,
	TYPE_INPUT_KWH_Y,
	TYPE_OUTPUT_KWH_Y,

	TYPE_MAX
}ACCUMULATED_DATA_TYPE;

typedef struct tagKWHmeterDataSave
{
	int         nYear;
	int   		nMonth;
	int 		nDay;
	int 		ndevid;
	VAR_VALUE	varEventWHMeterRough[_MAX_WH_ROUGH];

}EVENT_METER_SAVE_DATA;

#endif
