#ifndef __WEB_DATA_MODEL_H
#define __WEB_DATA_MODEL_H
//re-use the main-app definitions
#include "std_sys.h"
#include "app_incs.h"


#define DEFINE_DEV_MODEL(type, value) DEV_MODEL_##type = value,
enum DEV_MODEL
{
    DEFINE_DEV_MODEL(ESS, 100)
    DEFINE_DEV_MODEL(CCU, 201)
    DEFINE_DEV_MODEL(GPRDATA, 210)
    DEFINE_DEV_MODEL(SIGDATA, 250)
    DEFINE_DEV_MODEL(METER, 400)
    //DEFINE_DEV_MODEL(RS485_2, 450)
    DEFINE_DEV_MODEL(BAT_GRP, 500)
    DEFINE_DEV_MODEL(BAT, 501)
    DEFINE_DEV_MODEL(BAT_EU3, 600)
    DEFINE_DEV_MODEL(POWER_EU3, 610)
    DEFINE_DEV_MODEL(MULCUBE_EU3, 620)//edit by Kuangyue 20251130
    DEFINE_DEV_MODEL(PCS, 700)
    DEFINE_DEV_MODEL(METER_VAL, 800)
    DEFINE_DEV_MODEL(UART_SERVER, 900)
    DEFINE_DEV_MODEL(TEMP_CONT, 910)
    DEFINE_DEV_MODEL(AIR_CONDI, 950)
    DEFINE_DEV_MODEL(WATER_CHILLER, 960)   // NJX v5.3: chiller (was missing — caused Find_BelongDev to return -1 → Step7 fail → web_main.api exit loop)
    DEFINE_DEV_MODEL(PACK, 980)
    DEFINE_DEV_MODEL(AEMS, 990)
    DEFINE_DEV_MODEL(SEMS, 1000)
    DEFINE_DEV_MODEL(PV_INVT, 1050)
};
#undef DEFINE_DEV_MODEL


#define WEB_DISP_INFO_MAIN_IDX   10
#define WEB_DISP_INFO_SUB_IDX    40

#define MAX_MANIDX_NUM         100
#define MAX_SUBIDX_NUM         300 
typedef struct tagWebMainIdx
{
	int 		nParaId;
	int 	    nSetOrStatus; //0--setting, 1--status
	int 		nResId;
}WEBMAINIDX;

typedef struct tagWebSubIdx
{
	int 		nParaID;
	int 		nBellongMainID;
	int 	    nSetOrStatus; //0--setting, 1--status
	int 		nResId;
}WEBSUBIDX;


typedef struct tagWeb_DispInfo
{
    int     Value;
    int     Sub_id;
    BOOL    bIsDisp; 
}WEB_DISP_INFO;
typedef struct tagWebMainData
{
    UINT    main_id;
    UINT    nRes_Id;
    UINT    nSubTotal;
    WEB_DISP_INFO SubDispList[WEB_DISP_INFO_SUB_IDX];

}WEB_MAIN_DATA;    
typedef enum tagUserLevel
{
    USER_LEVEL_ENGINEER = 0,
    USER_LEVEL_ADMIN,
    USER_LEVEL_ROOT
}USER_LOGIN_LEVEL;

typedef enum tagDevClass
{
    DEV_CLASS_CHARGER_SYSTEM = 0,
    DEV_CLASS_CCU,
    //DEV_CLASS_CALC_NODEPOWER,
    DEV_CLASS_METERS,
    DEV_CLASS_BATTERY,
    DEV_CLASS_BATTCAB_IMEU2,

    DEV_CLASS_POWER_IMEU,
    DEV_CLASS_SINGLE_PM,
    DEV_CLASS_PCS_GROUP,
    DEV_CLASS_UART_SERVER,
    DEV_CLASS_TEMP_CONTROL,

    DEV_CLASS_AIR_COND,
    DEV_CLASS_LIQUIDCOOLING_PACK,
    DEV_CLASS_PV_INVERTER,
    DEV_CLASS_SEMS,
    DEV_CLASS_CABINET,
    DEV_CLASS_OTHER,
    DEV_CLASS_MAX = DEV_CLASS_OTHER
}WEB_DEV_CLASS;

typedef struct tagWebDevice
{
    int             nDevModelId;
    int             nDevId;
    WEB_DEV_CLASS   emDevClass;
    int             nDevName_ResId;
}WEB_DEVICE_INFO;

typedef struct tagWebEnumItem
{
    BYTE       emVal;
    USHORT     nEnumName_ResId;
}WEB_ENUM_ITEM;

typedef enum tagWebValueType
{
    WEB_VALTYPE_BOOL = 0,
    WEB_VALTYPE_INT,
    WEB_VALTYPE_DATETIME,
    WEB_VALTYPE_DATE,
    WEB_VALTYPE_TIME,
    WEB_VALTYPE_IP,
    WEB_VALTYPE_ENUM,
    WEB_VALTYPE_FLOAT_0,
    WEB_VALTYPE_FLOAT_1,
    WEB_VALTYPE_FLOAT_2,
    WEB_VALTYPE_FLOAT_3,
    WEB_VALTYPE_FLOAT_4,
    //string and object, need handle specially
    WEB_VALTYPE_OBJECT = 20,
    WEB_VALTYPE_STRING,
    WEB_VALTYPE_PASSWORD
    
}WEB_VALUE_TYPE;

typedef struct tagWEB_SC6LDEV_CMD155_ESCData
{
	float                   batt_curr;
	float                   batt_volt;
	UINT                    batt_soc;
	UINT                    batt_workstat;

	float                   acdc_ext_volt;
	float                   acdc_curr;
	float                   acdc_volt;
	UINT                    acdc_workstat;

	float                   kw_meter_cab;
	float                   ac_currs[3];
	float                   ac_volts[3]; // 0-C 1-B 2-A
	UINT                    ac_workstat;

	long                    tmStamp;
	UINT                    dev_id; // idxClient
} WEB_SC6LDEV_CMD155_ESCData;

typedef struct tagWebBlobCMD155
{// LJH250428 cmd155 BLOB_TYPE_CMD155
	//must always put first!!!
	BLOB_TYPE				          emBlobType;
	SC6LDEV_CMDHead                   cmdHead;
	SC6LDEV_CMD155_BranchData         branchDatas[MAX_NUM_BRANCH];
	WEB_SC6LDEV_CMD155_ESCData        escDatas[MAX_NUM_ESC];
} WEB_BLOB_CMD155;

typedef struct __attribute__((aligned(4))) tagWebESCInfo
{
	int     			nParamNameResId;
    int     			nParamUnitResId;
    WEB_VALUE_TYPE      emValType;
    void				*ptrValue;

}WEB_ESC_INFO;

typedef struct tagWebParameterInfo
{
    int                 nDevModelId;
    int                 nParamId;
    PARAM_TYPE		    emParamType;
    int                 nParamName_ResId;
    int                 nParamUnit_ResId;
    //_VALUE_TYPE		emValueType;
    WEB_VALUE_TYPE      emValType; //need reporter to transform it to Web Value Type
	UINT				nValueSize; //for var, set 4, for sz/blob, set sz-max bytes
	//int				nPrecision; //for float only, default 0
    int                 nBlobId; //for BlobParam only
    VAR_VALUE			varValDefault;
	float				fMaxLimit;
	float				fMinLimit;
#define MAX_SUPPORTED_WEB_ENUMITEM  200
	int					nEnumTotal;
	WEB_ENUM_ITEM		enumList[MAX_SUPPORTED_WEB_ENUMITEM];
    int					nDisplayPriority; //0--default, and the greater, the lower prio
    UINT				uMask_AuthorityLevel; //get from user level
    UINT                Web_1;
	UINT				Web_2;
}WEB_PARAM_INFO;

typedef struct tagWebParamVal
{
    WEB_PARAM_INFO*     pParamInfo;
    //value and attribute
    int                 nDevid;
   	VAR_VALUE		    currValue;
    int                 nParamId;
    VAR_VALUE           RereadValue;
    VAR_VALUE		    ReadValue;
    int                 nBlobSize; //just for value size matching
#define WEB_PARAM_MASK_VALID          0x01
#define WEB_PARAM_MASK_DISP           0x02
#define WEB_PARAM_MASK_CTRLSET        0x04
    UINT                m_emValueAttribute;
}WEB_PARAM_VALUE;

typedef struct tagRunDev
{
    WEB_DEVICE_INFO*    pDevInfo;
    //status params
    int                 nStatusParamTotal;
    WEB_PARAM_VALUE*    pStatusParamList;
    //setting params
    int                 nSettParamTotal;
    WEB_PARAM_VALUE*    pSettParamList;
    //ctrl params
    int                 nCtrlParamTotal;
    WEB_PARAM_VALUE*    pCtrlParamList;
}WEB_RUN_DEV;

// PCS Param_Id映射表
static const int g_PCS_ParamId_List[] = {
    252, 900, 916, 996, 997, 998, 999,

    1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009, 1010,
    1101, 1102, 1110, 1127, 1128, 1133,
    2003,

    2101, 2102, 2103, 2105, 2106, 2107, 2110, 2111,
    2114, 2115, 2117, 2118, 2120, 2121, 2122, 2123,
    2124, 2125, 2126, 2127, 2128, 2129, 2130, 2140,
    2141, 2142, 2143, 2144, 2145, 2146,

    3111, 3112, 3113, 3114, 3115, 3116, 3117, 3118,
    3119, 3121, 3122, 3123, 3124, 3125, 3126, 3127, 3128, 3129,
    3131, 3132, 3133, 3135, 3136, 3138, 3150, 3151,
    3152, 3153, 3154, 3155, 3156, 3157, 3160, 3161,
    3163, 3164, 3165, 3166, 3167, 3168, 3169, 3173,
    3174, 3175, 3176, 3177, 3178, 3179, 3180, 3181,
    3182, 3183, 3184, 3185, 3186, 3187, 3188, 3189,
    3190, 3191, 3192, 3193, 3194, 3195, 3196, 3197,
    3198, 3199, 3200, 3201, 3202, 3203, 3204, 3205,
    3206, 3207,

    5101, 5102, 5103, 5104, 5105,

    7140, 7141,

    21210, 21211, 21212, 21213, 21214, 21215,

    31110, 31111, 31113, 31121, 31130, 31131,
    31132, 31133, 31136, 31138, 31139, 31156,
    31157, 31158, 31159, 31181, 31191, 31211,
    31243, 31250, 31251, 31281, 31310, 31315,
    31610, 31611, 31612, 31613, 31614, 31615,
    31710, 31711, 31712, 31713, 31714, 31741,
    31751, 31781, 31810, 31811, 31812, 31813,
    31814, 31910, 31911, 31912, 31914, 31915,
    32000,

    71411, 71412,

    311011, 311101, 311111, 311313, 311315,
    311512, 311561, 313151, 316121, 316132,
    317141, 319151, 3111111, 91011
};

typedef struct {
    UINT nParam;       
    char* szCmd;    
} ParamIdToCmdMap;

// ParamId-命令号映射表
static ParamIdToCmdMap ParamToCmdMap[] = {
    {1101,   "1001"},
    {1102,   "1002"},
    {1110,   "1110"},
    {1127,   "1127"},
    {1128,   "1128"},
    {1133,   "1133"},
    {2003,   "2003"},
    {2101,   "2101"},
    {2102,   "2102"},
    {2103,   "2103"},
    {2105,   "2105"},
    {2106,   "2106"},
    {2107,   "2107"},
    {2110,   "2110"},
    {2111,   "2111"},
    {2114,   "2114"},
    {2115,   "2115"},
    {2117,   "2117"},
    {2118,   "2118"},
    {2120,   "2120"},
    {2121,   "2121"},
    {2122,   "2122"},
    {2123,   "2123"},
    {2124,   "2124"},
    {2125,   "2125"},
    {2126,   "2126"},
    {2127,   "2127"},
    {2128,   "2128"},
    {2129,   "2129"},
    {21210,  "212A"},
    {21211,  "212B"},
    {21212,  "212C"},
    {21213,  "212D"},
    {21214,  "212E"},
    {21215,  "212F"},
    {2130,   "2130"},
    {2140,   "2140"},
    {2141,   "2141"},
    {2142,   "2142"},
    {2143,   "2143"},
    {2144,   "2144"},
    {2145,   "2145"},
    {2146,   "2146"},
    {3111,   "3111"},
    {3112,   "3112"},
    {3113,   "3113"},
    {3114,   "3114"},
    {3115,   "3115"},
    {3116,   "3116"},
    {3117,   "3117"},
    {3118,   "3118"},
    {3119,   "3119"},
    {31181,  "3118"},
    {31191,  "3119"},
    {3121,   "3121"},
    {3122,   "3122"},
    {3123,   "3123"},
    {3124,   "3124"},
    {3125,   "3125"},
    {3126,   "3126"},
    {3127,   "3127"},
    {3128,   "3128"},
    {31281,  "312A"},
    {3129,   "3129"},
    {31211,  "312B"},
    {3131,   "3131"},
    {3132,   "3132"},
    {3133,   "3133"},
    {3135,   "3135"},
    {3136,   "3136"},
    {3138,   "3138"},
    {3150,   "3150"},
    {3151,   "3151"},
    {3152,   "3152"},
    {3153,   "3153"},
    {3154,   "3154"},
    {3155,   "3155"},
    {3156,   "3156"},
    {3157,   "3157"},
    {3160,   "3160"},
    {3161,   "3161"},
    {3200,   "3161"},
    {3163,   "3163"},
    {3164,   "3164"},
    {3165,   "3165"},
    {3166,   "3166"},
    {3167,   "3167"},
    {3168,   "3168"},
    {3169,   "3169"},
    {31610,  "316A"},
    {31611,  "316B"},
    {31612,  "316C"},
    {31613,  "316D"},
    {31614,  "316E"},
    {31615,  "316F"},
    {3173,   "3173"},
    {3175,   "3175"},
    {31751,  "3175"},
    {3174,   "3174"},
    {31741,  "3174"},
    {3176,   "3176"},
    {3177,   "3177"},
    {3178,   "3178"},
    {31781,  "3178"},
    {3179,   "3179"},
    {31710,  "317A"},
    {31711,  "317B"},
    {31712,  "317C"},
    {31713,  "317D"},
    {31714,  "317E"},
    {317141, "317E"},
    {31110,  "311A"},
    {31111,  "311B"},
    {311101, "311A"},
    {311111, "311B"},
    {31113,  "311D"},
    {31121,  "31C1"},
    {31130,  "31D0"},
    {31131,  "31D1"},
    {31132,  "31D2"},
    {31133,  "31D3"},
    {31136,  "31D6"},
    {31138,  "31D8"},
    {31139,  "31D9"},
    {31156,  "312E"},
    {311561, "31F6"},
    {31157,  "31F7"},
    {31159,  "31F9"},
    {311512, "31FC"},
    {31310,  "313A"},
    {31315,  "312F"},
    {313151, "313F"},
    {31914,  "319E"},
    {31915,  "319F"},
    {3111111,"31BB"},
    {31243,  "31F3"},
    {31250,  "31FA"},
    {31251,  "31FB"},
    {311313, "31DD"},
    {311315, "31DF"},
    {5101,   "5101"},
    {5102,   "5102"},
    {5103,   "5103"},
    {5104,   "5104"},
    {5105,   "5105"},
    {7140,   "7140"},
    {71411,  "714B"},
    {71412,  "714C"},
    {3180,   "3180"},
    {3181,   "3181"},
    {3182,   "3182"},
    {3183,   "3183"},
    {3184,   "3184"},
    {3185,   "3185"},
    {3186,   "3186"},
    {3187,   "3187"},
    {3188,   "3188"},
    {3189,   "3189"},
    {31810,  "318A"},
    {31811,  "318B"},
    {31812,  "318C"},
    {31813,  "318D"},
    {31814,  "318E"},
    {3190,   "3190"},
    {3191,   "3191"},
    {3192,   "3192"},
    {3193,   "3193"},
    {3194,   "3194"},
    {3195,   "3195"},
    {3196,   "3196"},
    {3197,   "3197"},
    {3198,   "3198"},
    {3199,   "3199"},
    {31910,  "319A"},
    {31911,  "319B"},
    {31912,  "319C"},
    {311011, "31AB"},
    {32000,  "3200"},
    {3201,   "3201"},
    {3202,   "3202"},
    {3203,   "3203"},
    {3204,   "3204"},
    {3205,   "3205"},
    {3206,   "3206"},
    {3207,   "3207"},
    {91011,  "910B"},
    {31158,  "31F8"},
    {4000,   "4000"},
    {4001,   "4001"},
    {4002,   "4002"},
    {4003,   "4003"},
    {4004,   "4004"},
    {4005,   "4005"},
    {4006,   "4006"},
    {4007,   "4007"},
    {4008,   "4008"},
    {4009,   "4009"},
    {4010,   "4010"},
    {4011,   "4011"},
    {4012,   "4012"},
    {4013,   "4013"},
};

// 获取映射表长度
#define MAP_LENGTH (sizeof(ParamToCmdMap) / sizeof(ParamIdToCmdMap))

#define PARAM_ID_CNT (sizeof(g_PCS_ParamId_List)/sizeof(int))

typedef struct tagWebAlarmInfo
{
	int					nAlarmId;
	int         		nAlmName_ResId;
	ALARM_LEVEL			emLevel;
}WEB_ALARM_INFO;

typedef struct tagAWebAlarmValue
{
   	WEB_ALARM_INFO*		pAlarmInfo;
	WEB_DEVICE_INFO*    pBelongDev; //happened on which device
	time_t			    tmLastOccured;
	time_t			    tmLastCeased; //time == 0 means active alarm
    int                 nAlarmStatus;
}WEB_ALARM_VALUE;

typedef struct tagWebSyslog
{
    int         nLogId;
    LOG_TYPE    emLogType;
    double      fLogTime;
    char*       szLog; //use 'NEW' to create space 
}WEB_SYSLOG;

typedef struct tagWebOCPPlog
{
    int         nLogId;
    DUMP_TYPE   emDumpType;
    double      fLogTime;
    char*       szMsg; //use 'NEW' to create space 
}WEB_OCPPLOG;

typedef struct tagWebVersionInfo
{
    //IMMU2 Version info
	char			szIMMU2_SWVer[MAX_LEN_MU2SWVER];
    char            szIMMU2_HWVer[8];
    //IMSU-D version info
    char            szIMSU_D_Vers[32];
     //Rectifier version info(DC, PFC)
    char            szRect_Ver_DC[32];
    char            szRect_Ver_PFC[32];

    //STbattery BMS Verson 
    char            szSTBatt_Ver[32];
    //IMEU2 Version
    char            szIMEU2_Ver[32];

    //DevID
    char            szDevID[64];

}WEB_VERSION_INFO;

typedef struct tagIMMU2_HisVerItem
{
    time_t   tmReleaseDate;
    char     szCurrVer[32];
    char     szReleaseNotes[128];   
}IMMU2_HISVERITEM;

typedef struct tagWebHisEvent
{
    _EVENT_TYPE		emEvType;
	UINT			nEvDataSize;
	void*			pEvData;
    time_t          tmBegin;
    time_t          tmEnd; //0 means event in process
}WEB_HIS_EVENT;

typedef struct tagWebHisData
{
    int                 nDevId;
    WEB_PARAM_INFO*     pParamInfo;   
   	float		        fParamVal;
    double              tmD_RecordTime; //'D' means accurate double time
}WEB_HIS_DATA;

typedef struct tagWebBmsRecData
{
    UINT    nIdx;
    UINT    nDay;
    time_t  tmLogTime;

}WEB_BMSREC_DATA;


typedef struct tagChargerRunData
{
    //static config data
    WEB_VERSION_INFO    versionInfos;
    int                 nDevInfoTotal;
    WEB_DEVICE_INFO     *pAllDevInfo;
    int                 nParamInfosTotal;
    WEB_PARAM_INFO      *pAllParamInfo;
    int                 nAlarmInfosTotal;
    WEB_ALARM_INFO      *pAllAlarmInfo;
    //static version history
#define WEB_MAX_SUPPORT_IMMUVERHIST       200
    int                 nIMMU2HisVerTotal;
    IMMU2_HISVERITEM    hisIMMU2VerList[WEB_MAX_SUPPORT_IMMUVERHIST];

    //--------the following run data are from IPC--------
    //run data for all devices
    WEB_RUN_DEV*        pAllRunDevs;

    int                   nParamTotal[WEB_DISP_INFO_MAIN_IDX][WEB_DISP_INFO_SUB_IDX];
    WEB_PARAM_VALUE***    pSettParList;

    int                   nTotal[WEB_DISP_INFO_MAIN_IDX][WEB_DISP_INFO_SUB_IDX];
    WEB_PARAM_VALUE***    pstatusParList;

    #define WEB_MAX_PCSGROUPNUM                 4
    #define WEB_MAX_SUPPORT_PCSRECTNUM          8
    #define WEB_MAX_PCSSETNUM                   200
    WEB_PARAM_VALUE     pRereadPCSParam[WEB_MAX_PCSGROUPNUM][WEB_MAX_PCSSETNUM][WEB_MAX_SUPPORT_PCSRECTNUM];  
    int                 nPCSGroupIdx;
    int                 nSingleRectIdx;
    int                 nRereadParamId;
    WEB_MAIN_DATA         ParaSetDispList1[WEB_DISP_INFO_MAIN_IDX];
    UINT                  Setnum;
    WEB_MAIN_DATA         ParaStatusDispList1[WEB_DISP_INFO_MAIN_IDX];
    UINT                  Stanum;

    //active alarms
#define WEB_MAX_SUPPORT_ACTALARM        200
    int                 nActiveAlmTotal;
    WEB_ALARM_VALUE     activeAlms[WEB_MAX_SUPPORT_ACTALARM];

    //---------the following data are from SQL db--------
    //history alarms
#define WEB_MAX_SUPPORT_HISTORYALARM    MAX_ALARM_RECORDS    
    int                 nHistAlmTotal;
    WEB_ALARM_VALUE     histAlms[WEB_MAX_SUPPORT_HISTORYALARM];
   
    //sys log
#define WEB_SUPPORT_MAX_SYSLOG  APP_LOG_SIZE
    int                 nSysLogTotal;
    WEB_SYSLOG          sysLogs[WEB_SUPPORT_MAX_SYSLOG];
    BOOL                bIsNoticeAPI_ForceRequery_SysLog; //added by Jimmy 20240112
    //ocpp log
#define WEB_SUPPORT_MAX_OCPPLOG  10000
    int                 nOCPPLogTotal;
    WEB_OCPPLOG         ocppLogs[WEB_SUPPORT_MAX_OCPPLOG];
    BOOL                bIsNoticeAPI_ForceRequery_OCPPLog; //added by Jimmy 20240112
    //his event
#define WEB_SUPPORT_MAX_HISEVENT 3000
    int                 nHisEventTotal;
    WEB_HIS_EVENT       hisEvents[WEB_SUPPORT_MAX_HISEVENT];
    BOOL                bIsNoticeAPI_ForceRequery_HisEvent; //added by Jimmy 20240112
    //his data
#define WEB_SUPPORT_MAX_HISDATA 10000 //MAX_HISDATA_RECORDS
    int                 nHisDataTotal;
    WEB_HIS_DATA        hisData[WEB_SUPPORT_MAX_HISDATA];
   BOOL                bIsNoticeAPI_ForceRequery_HisData; //added by Jimmy 20240112
   #define WEB_SUPPORT_MAX_BMSREC 50  
    int                 nBmsRecTotal;
    WEB_BMSREC_DATA     bmsRecData[WEB_SUPPORT_MAX_BMSREC];
    UINT   tTimeSum[WEB_SUPPORT_MAX_BMSREC];

   HIS_BMS_RUNTIMEDATA_REC bmsRunData_Rec;
    // BMS historical fault records
#define WEB_MAX_SUPPORT_BMSHISTFAULT  5000
    int                 nBmsHistFaultTotal;
    EVENT_BMS_HISTFAULT bmsHistFaults[WEB_MAX_SUPPORT_BMSHISTFAULT];
    // BMS historical event records
#define WEB_MAX_SUPPORT_BMSHISTEVENT  5000
    int                 nBmsHistEventTotal;
    EVENT_BMS_HISTEVENT bmsHistEvents[WEB_MAX_SUPPORT_BMSHISTEVENT];

    #define WEB_MAX_SUPPORT_HEALTH  MAX_HEALTH_RECORDS_DAY 
    int                     nHealthRecTotal;
   HIS_HEALTH_STATIC_DAILY  healthRec[WEB_MAX_SUPPORT_HEALTH];

}WEB_CHARGER_RUNDATA;

#define WEB_IPC_KEY	"ipc:///opt/appdata/data/nccu_web.ipc"

typedef enum tagWebReqType
{
	//Get static
    WEB_REQTYPE_GET_VERSION_IINFO = 32,
	WEB_REQTYPE_GET_ALL_DEVINFO,
	WEB_REQTYPE_GET_ALL_PARAMINFO,
    WEB_REQTYPE_GET_ALL_ALARMINFO,
    //Get class run param vals
	WEB_REQTYPE_GET_ALL_PARAMVALS,
    WEB_REQTYPE_GET_READBACK_VALS,
    //get locallist & localcache
    WEB_REQTYPE_GET_LOCALLIST,
    WEB_REQTYPE_GET_LOCALCACHE,
    //Get Active Alarm List
    //WEB_REQTYPE_GET_ACTIVE_ALARMS, //will get it thru sqlDB directly instead
	//Set/Ctrl Param
    WEB_REQTYPE_SET_PARAM_SERETCTRL,
	//Firware updt
    //WEB_REQTYPE_QUERY_FIRMWARE_UPDT,
    //Get ChgProfile
    WEB_REQTYPE_GET_CHGPROFILES,
    //LocalList updt
    //WEB_REQTYPE_OTH_LOCALLIST_UPDT,
    WEB_REQTYPE_HANDLE_HEARTBEAT,

    WEB_REQTYPE_GET_ALL_PARAMINFO_1,
    WEB_REQTYPE_GET_ALL_PARAMINFO_2,

    //just for checking purpose
    WEB_REQTYPE_MAX
}WEB_REQTYPE;

#define MAX_WEB_MSG_SIZE	4096
typedef struct tagAPP_WebMsg
{
    BYTE    bySOI; //for SOI
    UINT    ulLength;
    UINT    ulChkSum;
	BYTE    byEvType;
    BYTE    byMsgType;
    BYTE    byEvData[MAX_WEB_MSG_SIZE];
}WEB_FROMAAPP_MSG;

typedef enum tagWebErrCode
{
	WEB_ERRCODE_OK,
	WEB_ERRCODE_WRONG_LEN,
    WEB_ERRCODE_OUT_OF_RANGE,
	WEB_ERRCODE_WRONG_FORMAT,
	WEB_ERRCODE_EXECUTE_FAIL,
    WEB_ERRCODE_WRONG_AUTHLEVEL, //authrize level is not engough to execute the set/ctrl
	WEB_ERRCODE_TIMEOUT
}WEB_ERRCODE;

// LJH240726 DetailedInfo Meter is longer!!!
#define MAX_WEB_REQDATA_SIZE	(1024*60*2)
#define MAX_WEB_REPDATA_SIZE	(4*1024*1024) //nn_msg default send/receive buffer size
#define MAX_WEB_CAP_MSGQUEUE	16
#define WEB_IPC_HEADER_LEN      16

#define ESS_TCP_MAINTEN_FIX_LEN      11
typedef struct tagKeyVal
{
    int key;
    int val;
}INT_KEYVAL;
typedef struct tagStrVal
{
    char value[32];
    int  label;
}STR_KEYVAL;

#define MAX_WAIT_TIMEOUT_SEC_4SETCTRL   30000

#define MAX_WAIT_TIMEOUT_SEC_4SETCTRL_SPECIAL   90000

//for web-msg
#define WEB_IPC_MSG_SOI 0xAA

typedef enum tagDatabaseType
{
    WEB_DB_TYPE_SYSTEM = 0,
    WEB_DB_TYPE_NCCU_DATA,
    WEB_DB_TYPE_BMS_DATA,
    WEB_DB_TYPE_APP_LOG,

    WEB_DB_TYPE_MAX
}WebDatabaseType;

#endif