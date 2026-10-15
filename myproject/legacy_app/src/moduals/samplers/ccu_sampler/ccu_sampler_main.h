
#ifndef __CCU_SAMPLER_MAIN_H
#define __CCU_SAMPLER_MAIN_H

#include "../sampler_base.h"
typedef struct stRoughCCUData ROUGH_CCU_DATA;
#include "power_device_unit/cansend.h"
#include "power_device_unit/N2402.h"


#define CCU_HEART_PERIOD	1
#define LSYS_BROADCAST_PERIOD	5
#define MAX_ALARMBIT_MAP		32
#define MAX_SYSOVERTEMP_TIMES	3
#define MAX_RETDIFF_PERIOD 		15  //S
#define MAX_ABNM_TIMES			30
#define MIN_PWR_PCS_OUT_REQ     (1.0f) // PCS最小输出功率请求，小于则不充放电
#define MIN_PWR_KEEPWORKING     (2.0f) // 保持工作状态最小功率
#define MAX_PERIOD_ENTER_WORKING  (3)
#define MAX_PERIOD_DELAY_WORKING  (10) // 延迟小功率工况时间，避免频繁模块开关机
#define MIN_STOPSOC_DISC        (0)
#define MAX_STOPSOC_CHRG        (100)

#define STS_ADDR				0xFA

#define MAX_SENDCMD2CCU_TIMES	2
#define MAX_QUERYVERSION_TIMES   5
#define MAX_READ_BUFF_SIZE	            200

#define CANPORT       "can1"

// LJH241202 SEMS
#define INIT_NL_MSGINFO(p, _msgType, _pfnParser, _byCmd, _usMinDataLen) ( \
					(p+_msgType)->msgTp = (_msgType), \
					(p+_msgType)->byCmd = (_byCmd), \
					(p+_msgType)->pfnParser = (_pfnParser), \
					(p+_msgType)->usMin_ExpectDataLen = (_usMinDataLen))

#define INIT_RP_MSGINFO(p, _msgType, _pfnParser, _byCmd, _usMaxSubIdxNum) ( \
					(p+_msgType)->msgTp = (_msgType), \
					(p+_msgType)->byCmd = (_byCmd), \
					(p+_msgType)->pfnParser = (_pfnParser), \
					(p+_msgType)->usMaxSubIdxNum = (_usMaxSubIdxNum))

// LJH250930 加快自动充放响应,ChgDisOnly
#define NEED_AUTOCHRG() \
	(pEMS->bAutoChrgEnable && pEMS->planSectorOrg.emBatt_WorkMode<BATT_WORKMODE_DISCHRG_ONLY)
#define NEED_AUTODISC() \
	(pEMS->bAutoDischrgEnable && pEMS->planSectorOrg.emBatt_WorkMode<BATT_WORKMODE_DISCHRG_ONLY)



#define ID_LENGTH         8

// ---------HMI On2Off-Grid Event----------
typedef enum tagHMITrig
{
	STAT_TRIG_IDLE = 0,
	STAT_TRIGABU_ON2OFF,
	STAT_TRIG_PCSON,
	STAT_TRIG_PCSOFF,
	STAT_TRIGABU_OFF2ON,
	STAT_TRIG_MAX
}STAT_HMITRIG_ON2OFFGRID;

typedef struct tagHMIOn2OffGridParam
{
	time_t						tmLastHMITrig;

	STAT_HMITRIG_ON2OFFGRID		emHMITrig; 			

}HMI_ON2OFFGRID_PARAM;

typedef struct {
    uint32_t id;        // CAN ID (此处为标准的29位扩展帧ID)
    uint8_t  len;       // 数据长度 (CAN 2.0 范围为 0~8)
    uint8_t  data[8];   // 数据负载 (最多8字节)
} can_frame_t;

typedef struct tagA_YFY_BattFrame
{
	BYTE 			byRawData[CAN_FRAME_LEN];
	
	/*CAN Protocal Head Info,Total 29 bits,extend frame*/ 
	//BYTE			byErrCode;
 	//BYTE			byResv1;
	//BYTE			byCmdType;

	// ID_CMD_YFY_BATT emMsgType;
 	BYTE  			byFuncCode;
	BYTE  			bySubIndex;
    BYTE    		byBMSGrpAddr;   //0x32~
	UINT    		nBMSGrpMapIdx;  //0x0~
	BYTE 			byEMSAddr;

	UINT 			uiFrmID; 	//Send CAN ID
	UINT 			uiDataLen; 	//Send DLC
	BYTE   	 		byData[CAN_DATA_AREA_LEN+1];      /* Data Area */

	BOOL    		bIsCtrlCmd;     /* if true, need send urgently */
	// LJH240715 aircond to push data
	// STBATT_SAMPLER_ROUGH *pSTBatt_Rough;
}A_CAN_YFY_BATT_FRAME;

//---------Public Data Definitions----------
typedef enum tagLSYSAvailStatus
{
	SYS_STATUS_Availible = 0,
	SYS_STATUS_Alarm,
	SYS_STATUS_Faulted,
	SYS_STATUS_SWUpdate

 }LSYS_AVAILBLESTATE;

typedef enum tagTcpCCUCommState
{
	CCU_COMMSTAT_ACCEPTED = 0,
	CCU_COMMSTAT_WAITSYNC,
	CCU_COMMSTAT_NORMAL,
	CCU_COMMSTAT_FAILED,

}TCP_CCU_COMMSTATU;

typedef enum tagNLMsgType
{
	NL_MSG_CFGG = 0,
	NL_MSG_CFGS,
	NL_MSG_UCTL,
	NL_MSG_RTIF,
	NL_MSG_GCAN,
	NL_MSG_PCSINFO,

	NL_MSG_PCSGET,
	NL_MSG_PCSSET,
	NL_MSG_READ_PCSSET,

 	NL_MSG_USIZ,
	NL_MSG_UDTA,

	NL_MSG_USYS,
	NL_MSG_LSYS,
	NL_MSG_LDEV,
	NL_MSG_HEART,//2524

	NL_MSG_Max
}NL_MSGTYPE;

typedef enum tagNLMSG_RTN
{
	_RTN_OK = 0,
	_RTN_EXECUTE_ERR,
	_RTN_INVALID_CMD,
	_RTN_WRONG_FORMAT,
	_RTN_WRONG_DATA
}NLMSG_RTN;

typedef enum tagSlaveCommStatu
{
	SLV_NOT_INSTALLED = 0,
	SLV_COMM_FAIL,
	SLV_COMM_NORMAL,

}SLV_COMM_STATUS;

typedef enum tagMgmtCfg_ErrCode
{
	ERR_MGMT_NO_ERR	= 0x00,
    ERR_MGMT_SECTORNUM = 0x01,	
    ERR_MGMT_SOC_ABNORMAL = 0x02,
	ERR_MGMT_SUMINTERVAL = 0x03,
    ERR_MGMT_STATTM_OVERLAP = 0x04,
    
}ERR_CODE_MGMTCFG;


typedef enum emTriggerPM2CCU
{
    PM2CCU_TRIG_OFF = 0,
    PM2CCU_TRIG_PMGRP1,
    PM2CCU_TRIG_PMGRP2,
    PM2CCU_TRIG_PMGRP3, 
    PM2CCU_TRIG_PMGRP4,  
    PM2CCU_TRIG_ABU,
	PM2CCU_TRIG_ALL,

	PM2CCU_TRIG_MAX
}PM2CCU_TRIGGER_MSG;

typedef struct tagCanDataSave
{
    FILE						*hFile;

	char						szWriteBuff;

    UINT                        nTotalMsgNum;

	char						szFileName[64];

	time_t						tm_LastStatTrig;

    PM2CCU_TRIGGER_MSG		    emTrigMsgTp;

}PM2CCU_CANDATA_SAVE;

typedef enum emEmsUpdatePcsStep
{
	DOWNLOAD_IDLE = 0,
	DOWNLOAD_SLIENCE,
	DOWNLOAD_TRIGGER,			// 进入此状态 下发	 027F 00 F0 | 00 00 00 00 00 00 00 01/02, 进入 DOWNLOAD_WAIT_TRIGGER_BACK
	// DOWNLOAD_WAIT_TRIGGER_BACK, // 等待模块回复		 027F F0 00 | 00 00 00 00 00 00 00 01/02, 若有回复，进入 DOWNLOAD_SEND
	DOWNLOAD_SEND,
	// DOWNLOAD_SEND1, 			// 等待模块回复  14 09/49 F0 00 | (addr) (num) 00 01(01确认step), 下发1F 08/48 00 F0 | num字节数据 + 校验码， 进入DOWNLOAD_SEND2
	// DOWNLOAD_SEND2, 			// 等待模块回复  14 09/49 F0 00 | (addr+num) (num2) 00 02(02确认step), 下发1F 08/48 00 F0 | num2字节数据, 一次只能传8字节，分多次传送，进入进入DOWNLOAD_SEND1
	DOWNLOAD_WAIT,
	DOWNLOAD_CHECK_UP_NUM, 		// 检查已升级模块数量，是否所有模块都升级完成
	
	DOWNLOAD_FAILED,
	DOWNLOAD_SUCCESS,

	DOWNLOAD_FINISH
}EMS_UP_PCS_STEP;

typedef struct tagEmsUpdatePcs
{
	EMS_UP_PCS_STEP		emUpdateStep;

	UINT				uSlience; // 0-close, 1-sendCmd, 2-open
	// UINT				uTrigger1; // 0-idle, 1-get 00000000 00000001/2
	UINT				uTrigger2; // 0-idle, 1-get (addr)(readLen)0001, 2-get (addr+ReadLen)(num2)0002, 3-Err, 4-wait

	// UINT				uAddr06;	// 响应 06 字节命令
	// USHORT				usReadLen06;// 响应 06 字节命令

	UINT				uAddr;		// 响应 发送数据包命令
	UINT				uAddrCalc;	// 用于计算， uAddr 需保留数据
	USHORT				usReadLen;	// 响应 发送数据包命令
	USHORT				usReadLenCalc;	// 用于计算， usReadLen 需保留数据

	UINT				uAddr03;		// 响应 03Err 命令
	USHORT				usReadLen03;	// 响应 03Err 命令
	
	BYTE				byPCScmd;
	BYTE				bySendStartAddr; // 长数据包发送起始地址

	BYTE				byCurrentModuleId; //当前在升级的模块id
	ULONG				nHasSendLen;
	BYTE				byHasUpModNum;	// 已升级模块数量
	
}EMS_UPDATE_PCS;

typedef struct tagCCUUpgradeParam
{
	double						tmLastEnterUpgrade;

	MASTER_UPGRADE_PARAM		tagMstUpgradeParam;

	UPDOWN_FILE_INFO 			strUpdownFileInfo;

	ULONG						ulSingleMsgLen; 			


	EMS_UPDATE_PCS				strEmsUpCcuInfo; // 仅bmu03使用

}CCU_UPGRADE_PARAM;


//-----------PCS RElevant Defination----------//
typedef enum emPCSSettingCmdId
{
	GET_PCS_ID_1001 = 0,
	GET_PCS_ID_1002,
	GET_PCS_ID_1003,
	GET_PCS_ID_1004,
	GET_PCS_ID_1005,
	GET_PCS_ID_1006,
	GET_PCS_ID_1007,
	GET_PCS_ID_1008,
	GET_PCS_ID_1009,
	GET_PCS_ID_1010,

	SETT_PCS_ID_1101,
	SETT_PCS_ID_1102,
	SETT_PCS_ID_1110,
	SETT_PCS_ID_1127,
	SETT_PCS_ID_1128,
	SETT_PCS_ID_1133,
	SETT_PCS_ID_2003,
	SETT_PCS_ID_2101,
	SETT_PCS_ID_2102,
	SETT_PCS_ID_2103,
	SETT_PCS_ID_2105,
	SETT_PCS_ID_2106,
	SETT_PCS_ID_2107,
	SETT_PCS_ID_2110,
	SETT_PCS_ID_2111,
	SETT_PCS_ID_2114,
	SETT_PCS_ID_2115,
	SETT_PCS_ID_2117,
	SETT_PCS_ID_2118,
	SETT_PCS_ID_2120,
	SETT_PCS_ID_2121,
	SETT_PCS_ID_2122,
	SETT_PCS_ID_2123,
	SETT_PCS_ID_2124,
	SETT_PCS_ID_2125,
	SETT_PCS_ID_2126,
	SETT_PCS_ID_2127,
	SETT_PCS_ID_2128,
	SETT_PCS_ID_2129,
	SETT_PCS_ID_21210,
	SETT_PCS_ID_21211,
	SETT_PCS_ID_21212,
	SETT_PCS_ID_21213,
	SETT_PCS_ID_21214,
	SETT_PCS_ID_21215,
	SETT_PCS_ID_2130,
	SETT_PCS_ID_2140,
	SETT_PCS_ID_2141,
	SETT_PCS_ID_2142,

	SETT_PCS_ID_2143,
	SETT_PCS_ID_2144,
	SETT_PCS_ID_2145,
	SETT_PCS_ID_2146,

	SETT_PCS_ID_3111,
	SETT_PCS_ID_3112,
	SETT_PCS_ID_3113,
	SETT_PCS_ID_3114,
	SETT_PCS_ID_3115,
	SETT_PCS_ID_3116,
	SETT_PCS_ID_3117,
	SETT_PCS_ID_3118,
	SETT_PCS_ID_3119,
	SETT_PCS_ID_31181,
	SETT_PCS_ID_31191,
	SETT_PCS_ID_3121,
	SETT_PCS_ID_3122,
	SETT_PCS_ID_3123,
	SETT_PCS_ID_3124,
	SETT_PCS_ID_3125,
	SETT_PCS_ID_3126,
	SETT_PCS_ID_3127,
	SETT_PCS_ID_3128,
	SETT_PCS_ID_31281,
	SETT_PCS_ID_3129,
	SETT_PCS_ID_31211,
	SETT_PCS_ID_3131,
	SETT_PCS_ID_3132,
	SETT_PCS_ID_3133,
	SETT_PCS_ID_3135,
	SETT_PCS_ID_3136,
	SETT_PCS_ID_3138,

	SETT_PCS_ID_3150,
	SETT_PCS_ID_3151,
	SETT_PCS_ID_3152,
	SETT_PCS_ID_3153,
	SETT_PCS_ID_3154,
	SETT_PCS_ID_3155,
	SETT_PCS_ID_3156,
	SETT_PCS_ID_3157,

	SETT_PCS_ID_3160,
	SETT_PCS_ID_3161,
	SETT_PCS_ID_3163,
	SETT_PCS_ID_3164,
	SETT_PCS_ID_3165,
	SETT_PCS_ID_3166,
	SETT_PCS_ID_3167,
	SETT_PCS_ID_3168,
	SETT_PCS_ID_3169,
	SETT_PCS_ID_31610,
	SETT_PCS_ID_31611,
	SETT_PCS_ID_31612,
	SETT_PCS_ID_31613,

	SETT_PCS_ID_3173,
	SETT_PCS_ID_3175,
	SETT_PCS_ID_31751,
	SETT_PCS_ID_3174,
	SETT_PCS_ID_31741,
	SETT_PCS_ID_3176,
	SETT_PCS_ID_3177,

	SETT_PCS_ID_3178,
	SETT_PCS_ID_31781,
	SETT_PCS_ID_3179,
	SETT_PCS_ID_31710,
	SETT_PCS_ID_31711,
	SETT_PCS_ID_31712,
	SETT_PCS_ID_31713,
	SETT_PCS_ID_31714,
	SETT_PCS_ID_317141,

	SETT_PCS_ID_3200,

	SETT_PCS_ID_31614,
	SETT_PCS_ID_31615,

	SETT_PCS_ID_31110,
	SETT_PCS_ID_31111,
	SETT_PCS_ID_311101,
	SETT_PCS_ID_311111,
	SETT_PCS_ID_31113,

	SETT_PCS_ID_31121,
	SETT_PCS_ID_31130,

	SETT_PCS_ID_31131,
	SETT_PCS_ID_31132,
	SETT_PCS_ID_31133,
	SETT_PCS_ID_31136,
	SETT_PCS_ID_31138,

	SETT_PCS_ID_31139,
	SETT_PCS_ID_31156,
	SETT_PCS_ID_311561,
	SETT_PCS_ID_31157,
	SETT_PCS_ID_31158,
	SETT_PCS_ID_31159,
	SETT_PCS_ID_311512,
	SETT_PCS_ID_31310,
	SETT_PCS_ID_31315,
	SETT_PCS_ID_313151,

	SETT_PCS_ID_31914,
	SETT_PCS_ID_31915,
	SETT_PCS_ID_3111111,
	SETT_PCS_ID_311112,
	SETT_PCS_ID_31234,
	SETT_PCS_ID_31250,
	SETT_PCS_ID_31251,

	SETT_PCS_ID_311313,
	SETT_PCS_ID_311315,

	SETT_PCS_ID_5101,
	SETT_PCS_ID_5102,
	SETT_PCS_ID_5103,
	SETT_PCS_ID_5104,
	SETT_PCS_ID_5105,
	
	SETT_PCS_ID_7140,

	SETT_PCS_ID_71411,
	SETT_PCS_ID_71412,
	SETT_PCS_ID_3180,
	SETT_PCS_ID_3181,
	SETT_PCS_ID_3182,
	SETT_PCS_ID_3183,
	SETT_PCS_ID_3184,
	SETT_PCS_ID_3185,
	SETT_PCS_ID_3186,
	SETT_PCS_ID_3187,
	SETT_PCS_ID_3188,
	SETT_PCS_ID_3189,
	SETT_PCS_ID_31810,
	SETT_PCS_ID_31811,
	SETT_PCS_ID_31812,
	SETT_PCS_ID_31813,
	SETT_PCS_ID_31814,
	SETT_PCS_ID_3190,
	SETT_PCS_ID_3191,
	SETT_PCS_ID_3192,
	SETT_PCS_ID_3193,
	SETT_PCS_ID_3194,
	SETT_PCS_ID_3195,
	SETT_PCS_ID_3196,
	SETT_PCS_ID_3197,
	SETT_PCS_ID_3198,
	SETT_PCS_ID_3199,
	SETT_PCS_ID_31910,
	SETT_PCS_ID_311011,
	
	SETT_PCS_ID_32001,
	SETT_PCS_ID_3201,
	SETT_PCS_ID_3202,
	SETT_PCS_ID_3203,
	SETT_PCS_ID_3204,
	SETT_PCS_ID_3205,
	SETT_PCS_ID_3206,
	SETT_PCS_ID_3207,

	SETT_PCS_ID_31911,
	SETT_PCS_ID_31912,
	SETT_PCS_ID_91011,

	// RPMSG 模块配置信息(子索引76) 设置参数
	SETT_PCS_ID_4000,
	SETT_PCS_ID_4001,
	SETT_PCS_ID_4002,
	SETT_PCS_ID_4003,
	SETT_PCS_ID_4004,
	SETT_PCS_ID_4005,
	SETT_PCS_ID_4006,
	SETT_PCS_ID_4007,
	SETT_PCS_ID_4008,
	SETT_PCS_ID_4009,
	SETT_PCS_ID_4010,
	SETT_PCS_ID_4011,
	SETT_PCS_ID_4012,
	SETT_PCS_ID_4013,

	SETT_PCS_ID_MAX,

}SETT_PCS_IDXLIST;

#define		R_PROTO_SELF_ADDR			0xF0 
#define		DEV_NO_SINGLE 				0x0A
#define		DEV_NO 						0x0B
#define		SETT_CMD_NO 				0x24 
#define 	GET_CMD_NO  				0x23
#define		SRC_ADDR					0xF0

typedef enum emPCSValueType
{
	PCS_TYPE_INT,
	PCS_TYPE_UINT,
	PCS_TYPE_ENUM,
	PCS_TYPE_FLOAT, 

}SEET_PCS_VALUE_TYPE;

typedef struct tagPCSSettingsParam
{
	SETT_PCS_IDXLIST		pcsIdx;
	int 					nParamId;
	int						nScale;
	SEET_PCS_VALUE_TYPE		ValueType;
	//void					*ptrValue;
	BYTE 					bySubCmd;
	BYTE 					byCmdIdx;
	BYTE 					byByte2;//0x24 ReadCmdFlag 0x01-0x0F Read MultiCmd

}SETT_PCS_PARAM;

#define CAN_FRAME_LEN 		16
#define CAN_FRAME_DATALEN  	12

typedef struct tagARectFrame
{
	BYTE		byRawData[CAN_FRAME_LEN];

	BYTE		byErrCode;
	BYTE		byDevNo;
	BYTE		byCmdNo;
	BYTE		byDestAddr;
	BYTE		bySrcAddr;

	BYTE		byData[8];

	BOOL		bIsCtrlCmd; //if true, need send urgently
}A_CAN_RECT_FRAME;
//------------------End------------------//

#define NLMSG_FRME_MIN_LEN 18

#define  NLMSG_SOI		0xAA7EAA
#define  NLMSG_EOI		0x0D
#define  NLMSG_MIN_LEN	9
#define  IMMU2_ADDR		0x01 //We are now only support this/**/
#define  NLMSG_USYS_CMD	0xA0

typedef int (*NLMSG_FRAME_PARSER)(void* pArg, BYTE* pData, USHORT usLenID);
typedef struct tagNLMSGFrame
{
	NL_MSGTYPE	msgTp;
	//Frame Info
	BYTE				byCmd;
 
	NLMSG_FRAME_PARSER	pfnParser;
	USHORT				usMin_ExpectDataLen; //minimal expected data length
}NLMSG_FRAME_INFO;

typedef union tagSysAlarmMap1
{
	UINT		uiFlag;
	struct
	{
		UINT	DoorOpen 			: 1;
		UINT	EPO 				: 1;
		UINT	SPD 				: 1;
		UINT	Resved3 			: 1;
		UINT	SysOverTemp 		: 1;
		UINT	SysFan1Fault		: 1;
		UINT	SysFan2Fault		: 1;
		UINT	SysFan3Fault		: 1;
		UINT	Resved8 			: 1;
		UINT	GroundFault 		: 1;
		UINT	ACOverVolt 			: 1;
		UINT	ACLowerVolt 		: 1;
		UINT	ACGridInputMCBQF1 	: 1;  //AC Input Breaker
		UINT	ACLoadOutputMCBQF2 	: 1;
		UINT	MainAC_Contactor 	: 1;
		UINT	IMEU2_Grp1_CommFail : 1;
		UINT	IMEU2_Grp2_CommFail : 1;
		UINT	STS1_CommFail 		: 1;
		UINT	STS2_CommFail 		: 1;
		UINT	STS3_CommFail 		: 1;
		UINT	STS4_CommFail 		: 1;
		UINT	SmokeAlarm 			: 1;
		UINT	FloodingAlarm 		: 1;
		UINT	ToppleAlarm 		: 1;
		UINT	LowVoltMCBBreaker 	: 1;
		UINT	N_LineContactor 	: 1;

	}Alarm;

}LSYS_ALARM_MAP1;

typedef union tagSysAlarmMap2
{
	UINT		uiFlag;
	struct
	{
		UINT	Insulation1_CommFail : 1;
		UINT	Insulation1_detectAlm : 1;
		UINT	Insulation2_CommFail : 1;
		UINT	Insulation2_detectAlm : 1;
		UINT	Insulation3_CommFail : 1;
		UINT	Insulation3_detectAlm : 1;
		UINT	Insulation4_CommFail : 1;
		UINT	Insulation4_detectAlm : 1;
		UINT	Insulation5_CommFail : 1;
		UINT	Insulation5_detectAlm : 1;
		UINT	Insulation6_CommFail : 1;
		UINT	Insulation6_detectAlm : 1;
		UINT	Insulation7_CommFail : 1;
		UINT	Insulation7_detectAlm : 1;
		UINT	Insulation8_CommFail : 1;
		UINT	Insulation8_detectAlm : 1;
		UINT	Insulation9_CommFail : 1;
		UINT	Insulation9_detectAlm : 1;
		UINT	Insulation10_CommFail : 1;
		UINT	Insulation10_detectAlm : 1;
		UINT	ResV : 8;
		UINT	N_wire1 : 1;
	}Alarm;

}LSYS_ALARM_MAP2;

typedef union tagSysAlarmMap3
{
	UINT		uiFlag;
	struct
	{
		UINT	BattGrp1_ForbidRun : 1;
		UINT	BattGrp2_ForbidRun : 1;
		UINT	BattGrp3_ForbidRun : 1;
		UINT	BattGrp4_ForbidRun : 1;
		UINT	BattGrp1_InputContactor : 1;
		UINT	BattGrp2_InputContactor : 1;
		UINT	BattGrp3_InputContactor : 1;
		UINT	BattGrp4_InputContactor : 1;
		UINT	BattGrp1_IsolationSwitch : 1;
		UINT	BattGrp2_IsolationSwitch : 1;
		UINT	BattGrp3_IsolationSwitch : 1;
		UINT	BattGrp4_IsolationSwitch : 1;
		UINT	BmsGrp1_CANCommFail : 1;
		UINT	BmsGrp2_CANCommFail : 1;
		UINT	BmsGrp3_CANCommFail : 1;
		UINT	BmsGrp4_CANCommFail : 1;
	}Alarm;
}LSYS_ALARM_MAP3;

typedef union tagSysAlarmMap4
{
	UINT		uiFlag;
	struct
	{
		UINT	DCLoadGrp1_SPD : 1;
		UINT	DCLoadGrp2_SPD : 1;
		UINT	DCLoadGrp3_SPD : 1;
		UINT	DCLoadGrp4_SPD : 1;
		UINT	DCLoadGrp1_IsolationSwitch : 1;
		UINT	DCLoadGrp2_IsolationSwitch : 1;
		UINT	DCLoadGrp3_IsolationSwitch : 1;
		UINT	DCLoadGrp4_IsolationSwitch : 1;
	}Alarm;
}LSYS_ALARM_MAP4;

typedef union tagGrpACDCRectAlarm
{
	UINT		uiFlag;
	struct
	{
		UINT	AllRectFail : 1;
		UINT	AllRectCommFail : 1;
		UINT	Grp_1_Fail : 1;
		UINT	Grp_1_CommFail : 1;
		UINT	Grp_2_Fail : 1;
		UINT	Grp_2_CommFail : 1;
	 	UINT	Grp_3_Fail : 1;
		UINT	Grp_3_CommFail : 1;
		UINT	Grp_4_Fail : 1;
		UINT	Grp_4_CommFail : 1;
		UINT	Grp_5_Fail : 1;
		UINT	Grp_5_CommFail : 1;
		UINT	Grp_6_Fail : 1;
		UINT	Grp_6_CommFail : 1;
	 	UINT	Grp_7_Fail : 1;
		UINT	Grp_7_CommFail : 1;
		UINT	Grp_8_Fail : 1;
		UINT	Grp_8_CommFail : 1;
		UINT	Grp_9_Fail : 1;
		UINT	Grp_9_CommFail : 1;
		UINT	Grp_10_Fail : 1;
		UINT	Grp_10_CommFail : 1;
	 	UINT	Grp_11_Fail : 1;
		UINT	Grp_11_CommFail : 1;
		UINT	Grp_12_Fail : 1;
		UINT	Grp_12_CommFail : 1;
		UINT	PCS_Grp1_Fault : 1;
		UINT	PCS_Grp2_Fault : 1;
		UINT	PCS_Grp3_Fault : 1;
		UINT	PCS_Grp4_Fault : 1;

	}Alarm;

}PM_GRP_ALARM_MAP;

 
typedef union tagUSysAlarmMap1
{
	USHORT	usFlag;
	struct
	{
		USHORT	PV_SDP_Alm: 1;
		USHORT	DoorOpen : 1;

	}Alarm;
}USYS_ALARM_MAP1;

typedef union tagGrpBatteryStatus
{
	UINT					uiFlag;
	struct
	{
		UINT				GrpA_Batt_Status :4;	//0x00-Idle, 0x01-Charging, 0x02-Discharging, 0x03-Error
		UINT				GrpB_Batt_Status :4;				 
 		UINT				GrpC_Batt_Status :4;				 
		UINT				GrpD_Batt_Status :4;				 

	 }Info;

}GRP_BATT_RUNSTATUS;

#define MAX_SUPPORT_IMIU_NUM	10
#define MAX_RECT_GRP_NUM 		2
#define MAX_AMBINENT_TEMP		4
#define BYTE_NUM 				8

typedef struct tagLSYSDataInfo
{
	LSYS_AVAILBLESTATE			emLSysAvaiStatus;	

	UINT 						emRunningMode;
	
	float						fLeakageCurr;

	float						fAmbientTemp[MAX_AMBINENT_TEMP];

	LSYS_ALARM_MAP1				sysAlarmMap1;
	LSYS_ALARM_MAP2				sysAlarmMap2;
	LSYS_ALARM_MAP3				sysAlarmMap3;
	LSYS_ALARM_MAP4				sysAlarmMap4;

	PM_GRP_ALARM_MAP			pmGrpAlmMap;
 
	float 						fIMIU_P_IsoVal[MAX_SUPPORT_IMIU_NUM];
	float						fIMIU_N_IsoVal[MAX_SUPPORT_IMIU_NUM];

	A_BLOB_BRANCH_GRP_RECTDATA	branchNodeData[MAX_SEMS_CLIENTS_NUM]; // LSYS

	double 						tm_fLastCalcMPPT;
	float						fMPPTGenerateKWH[MAX_SINGLE_STBATT_NUM];
	float						fLast_MPPTGenerateKWH[MAX_SINGLE_STBATT_NUM];

	double 						tm_fLastCalcDCLoad;
	float						fDCLoad_CostKWH;
	float						fLast_DCLoad_CostKWH;

	A_BLOB_RECT_GRP_DATA		blobGrpRectData; // RTIF

 	A_BLOB_SINGLE_RECT_INFO		singleRectData; // RTIF
 
	time_t 						tmRTC;

	PCS_SYS_STATUCODE			mapPCSSysStatus[MAX_SINGLE_STBATT_NUM];
 	PCS_SYS_STATUCODE			mapPCSSysStatusBelong2[MAX_SINGLE_STBATT_NUM];

	STS_RUN_STATUS 				mapSTSCode; 
	ABU_RUN_STATUS1				mapAbuStatu1;

	UINT 						uiCCUFaultCode;
	UINT 						uiPCS1CtrlFailCode;
	UINT 						uiPCS2CtrlFailCode;
	UINT 						uiPCS3CtrlFailCode;
	UINT 						uiPCS4CtrlFailCode;

	char						szPowerCube_IMEU2[9];

	UINT 						emAPPUpgradingType;
	UINT 						byUpgradingSubID;
	UINT 						emUpgradingStatus;
	UINT 						byUpgradingSubIDSchedule;
	UINT 						emUpgradingTotSchedule;
	UINT 						byUpgradingFailReason;

	time_t						tmLastRecv_LSYS; //last time received LSYS msg
	time_t						tmLast_Sent_USYS;
	time_t						tmLastRecv_RTIF; //last time received RTIF msg
	time_t						tmLastSend_PwrOnSTS;
	time_t						tmLastSend_CtrlVoiceAndLight;
	time_t						tmLastSend_PwrCabLight;



}LSYS_DATA_INFO;

typedef struct tagHeartDataInfo
{
	BYTE 						bySWRunLocation; // 软件运行位置
	USHORT 						usSWVer; 		// 软件版本号

	time_t						tmLastRecv_HEART;

}HEART_DATA_INFO;

typedef enum emPwrDispMode
{
   MODE_PWRDISP_ONEBYONE = 0,
   MODE_PWRDISP_AVG,

}MODE_PWR_DISPATCH;

typedef enum emPhaseType
{
	PHASE_TYPE_ALL,
	PHASE_TYPE_POWER, // 单相防逆流:功率
	PHASE_TYPE_VA     // 单相防过载:电流
}PHASE_TYPE;

typedef struct tagEMSOrgCalcData
{
	float 							fMaster_LocalAC_RTKWH_Reading;  	//主_柜内表实时电量
	float 							fMaster_LocalAC_RevsKWH;			//主_柜内表反向有功总电能

	float 							fLast_Master_LocalAC_RTKWH_Reading;	//上一次_主_柜内表实时电量
	float 							fLast_Master_LocalAC_RevsKWH;		//上一次_主_柜内表反向有功总电能

 	float 							fSlv1_LocalAC_RTKWH_Reading;		//从_柜内表实时电量
	float 							fSlv1_LocalAC_RevsKWH;				//从_柜内表反向有功总电能

	float 							fLast_Slv1_LocalAC_RTKWH_Reading;	//上一次_从_柜内表实时电量
	float 							fLast_Slv1_LocalAC_RevsKWH;			//上一次_从_柜内表反向有功总电能

	float                           fRmtAC_RealTime_ReadKWH;			//电表H 实时电量
    float                           fRmtAC_LastkWhReading;				//上一次_电表H 实时电量

	float                           fRmtAC_Revs_ReadKWH;				//电表H 反向有功总电能
    float                           fRmtAC_Last_Revs_ReadKWH;			//上一次_电表H 反向有功总电能
 
    float                           fMPPT_RealTime_ReadKWH[MAX_SINGLE_STBATT_NUM];//MPPT组#1 总发电量
    float                           fMPPT_LastkWhReading[MAX_SINGLE_STBATT_NUM];//上一次_MPPT组#1 总发电量
	
	float 							fDCLoad_TotalReadKWH;		//MPPT组#3 总发电量
	float 							fLast_DCLoad_TotalReadKWH;//上一次_MPPT组#3 总发电量

    float                           fBattChrg_RealTime_KWH[MAX_SINGLE_STBATT_NUM];//电池组充电电能
    float                           fBattChrg_LastkWhReading[MAX_SINGLE_STBATT_NUM];//上一次_电池组充电电能
 
    float                           fBattDisChrg_RealTime_KWH[MAX_SINGLE_STBATT_NUM];//电池组放电电能
    float                           fBattDisChrg_LastkWhReading[MAX_SINGLE_STBATT_NUM];//上一次_电池组放电电能
   
    float                           fBatt_Rate_KWH[MAX_SINGLE_STBATT_NUM];//额定电量

	float 							fSTBatt_LastSOC[MAX_SINGLE_STBATT_NUM];

	float                           fRmtLoad_RealTime_ReadKWH;    	//离网负载电表G  实时电量
	float                           fRmtLoad_LastkWhReading;   		//上一次_电表G  实时电量

	float                           fRmtLoad_Revs_ReadKWH;				//电表G 反向有功总电能
    float                           fRmtLoad_Last_Revs_ReadKWH;			//上一次_电表G 反向有功总电能

}EMS_ORG_CALC_DATA;

// LJH241111 PcsSCADA
typedef enum tagPcsStateMachineState
{
    PCS_STATE_IDLE = 0,
    PCS_STATE_WAIT_CHARGE,
    PCS_STATE_IN_CHARGING,
    PCS_STATE_WAIT_DISCHARGE,
    PCS_STATE_IN_DISCHARGING,
    PCS_STATE_FAULTED,
	PCS_STATE_OFFGRID,
    PCS_STATE_MAX,
} PcsSMState;

typedef PcsSMState (*PCS_ON_STATE_PROC)(void* pArg, int idx);

typedef struct tagPcsStateMachineItem
{
	char               szStateNamePcs[16];
	PcsSMState         emStatePcs;
	PCS_ON_STATE_PROC  pfnActionPcs;
	double             tm_EnterPcs;
} PcsSMItem;

typedef struct tagPcsSMInfo
{
	PcsSMItem            *pLastSMItem;
    PcsSMItem            *pCurrSMItem;

	ACDC_WORKSTATU        emACDC_WorkStatuReq;
	ACDC_WORKSTATU        emACDC_WorkStatuNow;
	A_BLOB_GRP_RECT_AUTOCTRL_CMD  ctrlRectReq;
	A_BLOB_GRP_RECT_AUTOCTRL_CMD  ctrlRectSend;
	A_BLOB_CTRL_PCS_ONGRID_CMD   ctrlPcsOnGridReq;
	A_BLOB_CTRL_PCS_ONGRID_CMD   ctrlPcsOnGridSend;
	A_BLOB_CTRL_PCS_OFFGRID_CMD  ctrlPcsOffGridReq;
	A_BLOB_CTRL_PCS_OFFGRID_CMD  ctrlPcsOffGridSend;
	A_BLOB_CTRL_PCS_ONGRID_CMD  ctrlPcsOnGridReq_Last;
	A_BLOB_CTRL_PCS_OFFGRID_CMD ctrlPcsOffGridReq_Last;
	float                 fLastSendReqVolt_2Pcs;
    float                 fLastSendReqCurr_2Pcs;
	// GRP_RECT_CTRL_CMDTYPE emLastCtrlCmdPcs;
	double                tm_LastStartWork;
	double                tm_LastStopWork;
	double                tm_LastSendCmd_2Pcs;

	// Manual Ctrl
	A_BLOB_MANUALCTRL_RECT_CMD ctrlRectReq_Manual;

	// LJH251106 同步主从参数
	HasParamSetCFGS        hasParamSetCFGS;
	// LJH241202 SEMS
	S63Uctl_30WorkPwr      sems6330ctrlPwrReq;
	float                  fActv_PwrCtrl_Last;
	BYTE                   emACDC_WorkStatuCtrl_Last;
	BYTE                   byWorkASAP_Last;
} PcsSMInfo;

typedef struct stRunDataExt_SEMS
{
	//Grid ACMeter 
	float               fGridMeter_Volt[3]; //either 3-phase AC meter 
	float               fGridMeter_Curr[3]; //either 3-phase AC meter 
	float               fGridMeter_Power[3]; //0:GridMeter_kWh(In)|1:GridMeter_kW|2:GridMeter_kWh(Out)

	float               f_QgridMeter;
	float               f_QcabMeter;

	//Cabinet ACMeter
	float               fCabMeter_Volt[3]; //either 3-phase AC meter 
	float               fCabMeter_Curr[3]; //either 3-phase AC meter 
	float               fCabMeter_Power[3]; // CabMeter_kWh(In),CabMeter_kW,CabMeter_kWh(Out),

	// float               fPowerCubeTemp[3];
} RunDataExt_SEMS;

/********************************************************* D9 Platform *****************************************************************/

typedef enum tagRPMsgType
{
	RP_MSG_ALL_MODULE_INFO = 0,
	RP_MSG_ACDC_GRP_1_INFO,
	RP_MSG_ACDC_GRP_2_INFO,
	RP_MSG_DCDC_GRP_1_INFO,
	RP_MSG_DCDC_GRP_2_INFO,
	RP_MSG_ACDC_SIG_1_INFO,
	RP_MSG_ACDC_SIG_2_INFO,
	RP_MSG_DCDC_SIG_1_INFO,
	RP_MSG_DCDC_SIG_2_INFO,
	// RP_MSG_DCDC_GRP_INFO,
	// RP_MSG_MPPT_GRP_INFO,
	// RP_MSG_ACDC_SIG_INFO,
	// RP_MSG_DCDC_SIG_INFO,
	// RP_MSG_MPPT_SIG_INFO,

	// Write/Read
	RP_MSG_MODULE_TYPE_CFG,
	RP_MSG_MODULE_NUMBER_CFG,
	RP_MSG_ACDC_CHRG_DISCHRG_CTRL,
	RP_MSG_DCDC_CHRG_DISCHRG_CTRL,
	RP_MSG_MAUNL_AUTO_MODE,
	RP_MSG_ON_OFF_GRID_CTRL,
	RP_MSG_ACDC_PARAMS_SET,

	RP_MSG_MAX
}RP_MSGTYPE;



#define  RPMSG_SOI		0xABCD
#define  RPMSG_EOI		0xEF
#define  RPMSG_MIN_LEN	5

typedef int (*RPMSG_FRAME_PARSER)(void* pArg, AR_FRAME* pData);
typedef struct tagRPMSGFrame
{
	RP_MSGTYPE msgTp;
	// Frame Info
	BYTE                    byCmd;
	RPMSG_FRAME_PARSER      pfnParser; 
	USHORT                  usMaxSubIdxNum;    // 最大子索引数目
}RPMSG_FRAME_INFO;

typedef struct tagAllModulesInfo
{
	USHORT                      usModuRunningMode;
	USHORT                      usCtrlMode;
	USHORT                      usChgDischgFailReason;
	USHORT                      usModuGrp_CfgInfo;
	USHORT                      usModuGrp_OnlineInfo;
} ALL_MODULES_INFO;

typedef struct tagACDCGrpInfo
{
	UINT                        uiOnlineModuNum;
	UINT						uiRatedPower;
	UINT                        uiRatedCurr;
	UINT                        uiMaxCurr;
	UINT                        uiCtrlState;
	UINT                        uiCtrlDcVolt;
	UINT                        uiCtrlDcCurr;
	UINT                        uiCtrlPower;
	UINT                        uiRunningState;
	UINT                        uiCurDcVolt;
	UINT                        uiCurDcTotalCurr;
	UINT                        uiCurSysPower;
} ACDC_GRP_INFO;

typedef struct tagDCDCGrpInfo
{
	UINT                        uiOnlineModuNum;
	UINT                        uiRatedPower;
	UINT                        uiRatedCurr;
	UINT                        uiMaxCurr;
	UINT                        uiCtrlState;
	UINT                        uiCtrlVolt;
	UINT                        uiCtrlBattDischgCutoffVolt;
	UINT                        uiCtrlBattCurrLmt;
	UINT                        uiCtrlBusCurrLmt;
	UINT                        uiRunningState;
	UINT                        uiCurBattVolt;
	UINT                        uiCurBattTotalCurr;
	UINT                        uiCurBusVolt;
} DCDC_GRP_INFO;

typedef struct tagMPPTGrpInfo
{
	UINT                        uiOnlineModuNum;
	UINT                        uiRatedPower;
	UINT                        uiRatedCurr;
	UINT                        uiMaxCurr;
	UINT                        uiRunningState;
	UINT                        uiSysVolt;
	UINT                        uiSysTotalCurr;
} MPPT_GRP_INFO;

typedef struct tagACDCSigInfo
{
	UINT                      uiBelongGrpID;
	UINT                      uiModuID;
	UINT                      uiRamStateInfo;
	UINT                      uiInvState_1_1;
	UINT                      uiInvState_1_2;
	UINT                      uiInvState_2_1;
	UINT                      uiInvState_2_2;
	UINT                      uiInvState_3_1;
	UINT                      uiInvState_3_2;
	UINT                      uiInvState_4_1;
	UINT                      uiInvState_4_2;
	UINT                      uiDcSoftVer;
	UINT                      uiPfcSoftVer;
	UINT                      uiBarcode_1;
	UINT                      uiBarcode_2;
	UINT                      uiBarcode_3;
	UINT                      uiBarcode_4;
	UINT                      uiModuDcVolt;
	UINT                      uiModuDcCurr;
	UINT                      uiModuEnvTemp;
	UINT                      uiAcLineVolt_AB;
	UINT                      uiAcLineVolt_BC;
	UINT                      uiAcLineVolt_CA;
	UINT                      uiAcPhaseVolt_A;
	UINT                      uiAcPhaseVolt_B;
	UINT                      uiAcPhaseVolt_C;
	UINT                      uiAcPhaseCurr_A;
	UINT                      uiAcPhaseCurr_B;
	UINT                      uiAcPhaseCurr_C;
	UINT                      uiAcFreq;
	UINT                      uiTotalActivePower;
	UINT                      uiActivePower_A;
	UINT                      uiActivePower_B;
	UINT                      uiActivePower_C;
	UINT                      uiTotalReactivePower;
	UINT                      uiReactivePower_A;
	UINT                      uiReactivePower_B;
	UINT                      uiReactivePower_C;
	UINT                      uiTotalApparentPower;
	UINT                      uiApparentPower_A;
	UINT                      uiApparentPower_B;
	UINT                      uiApparentPower_C;
	UINT                      uiTotalPowerFactor;
	UINT                      uiPowerFactor_A;
	UINT                      uiPowerFactor_B;
	UINT                      uiPowerFactor_C;
} ACDC_SIG_INFO;

typedef struct tagDCDCSigInfo
{
	UINT                      uiBelongGrpID;
	UINT                      uiModuID;
	UINT                      uiInvState_1_1;
	UINT                      uiInvState_1_2;
	UINT                      uiInvState_2_1;
	UINT                      uiInvState_2_2;
	UINT                      uiInvState_3_1;
	UINT                      uiInvState_3_2;
	UINT                      uiInvState_4_1;
	UINT                      uiInvState_4_2;
	UINT                      uiDcSoftVer;
	UINT                      uiPfcSoftVer;
	UINT                      uiBarcode_1;
	UINT                      uiBarcode_2;
	UINT                      uiBarcode_3;
	UINT                      uiBarcode_4;
	UINT                      uiModuBattVolt;
	UINT                      uiModuBattCurr;
	UINT                      uiModuBusVolt;
	UINT                      uiModuBusCurr;
	UINT                      uiModuEnvTemp;
} DCDC_SIG_INFO;

typedef struct tagMPPTSigInfo
{
	UINT                      uiBelongGrpID;
	UINT                      uiModuID;
	UINT                      uiInvState_1_1;
	UINT                      uiInvState_1_2;
	UINT                      uiInvState_2_1;
	UINT                      uiInvState_2_2;
	UINT                      uiDcSoftVer;
	UINT                      uiPfcSoftVer;
	UINT                      uiBarcode_1;
	UINT                      uiBarcode_2;
	UINT                      uiBarcode_3;
	UINT                      uiBarcode_4;
	UINT                      uiModuVolt;
	UINT                      uiModuCurr;
	UINT                      uiModuEnvTemp;
} MPPT_SIG_INFO;

typedef struct tagModuleTypeConfig
{
	BYTE                      	byACDC_Module;
	BYTE                      	byDCDC_Module;
	BYTE                      	byMPPT_Module;            
} MODULE_TYPE_CFG;

typedef struct tagModuleNumConfig
{
	BYTE                      	byACDC_GRP1_SigNum;
	BYTE                      	byACDC_GRP2_SigNum;
	BYTE                      	byDCDC_GRP1_SigNum;
	BYTE                      	byDCDC_GRP2_SigNum;
	BYTE                      	byMPPT_GRP1_SigNum;
	BYTE                      	byMPPT_GRP2_SigNum;   
} MODULE_NUM_CFG;

typedef struct tagACDC_Chrg_DisChrg_Ctrl
{
	USHORT                      usChrg_DisChrg_Mode;
	USHORT                      usCtrl_Module_GrpID;
	USHORT                      usDC_Side_Ctrl_Vlot;
	USHORT                      usDC_Side_Ctrl_Curr;
	USHORT                      usAC_Side_Ctrl_Power;
	USHORT                      usScheduling_Mode;
	USHORT                      usReactive_Power;
} ACDC_CHRG_DISCHRG_CTRL;

typedef struct tagDCDC_Chrg_DisChrg_Ctrl
{
	USHORT                      usChrg_DisChrg_Mode;
	USHORT                      usCtrl_Module_GrpID;
	USHORT                      usOutPut_Vlot;
	USHORT                      usBattDisChrg_CutoffVolt;
	USHORT                      usBattSide_CurrLmt;
	USHORT                      usBusSide_CurrLmt;
} DCDC_CHRG_DISCHRG_CTRL;

typedef struct tagManulAutoMode
{
	BYTE                    	byManulAutoMode;
} MANUL_AUTO_MODE;

typedef struct tagN2002_OnOffGrid_Ctrl
{
	BYTE                    	byABU_Enable;
	BYTE                    	byOnOffGridAutoSwitchMode;
	BYTE                    	byOnOffGridMode;
	BYTE                    	byABU_OnOff;
} N2002_ON_OFF_GRID_CTRL;

typedef struct tagACDC_Params_Set
{
	USHORT						usDispatchMode;			// 调度模式: 0:直流调度 1:交流调度
	USHORT						usStartMethod;			// 启机方式: 0:直流启机 1:交流启机
	USHORT						usOnOffGridMode;		// 并离网模式: 0:并网 1:离网
	USHORT						usStorageMode;			// 储能模式: 0:不使能 1:使能 2:??
	USHORT						usBattChgEndVolt;		// 电池充电结束电压 (V)
	USHORT						usBattChgOverCurr;		// 电池充电过流点 (A)
	USHORT						usBattDischgEndVolt;	// 电池放电结束电压 (V)
	USHORT						usBattDischgOverCurr;	// 电池放电过流点 (A)
	USHORT						usBattDODVolt;			// 电池DOD电压 (V)
} ACDC_PARAMS_SET;

typedef enum tagMsgDataType
{
	MDT_BYTE = 0,
	MDT_USHORT,
	MDT_INT8,
	MDT_UINT8,
	MDT_INT16,
	MDT_UINT16,
	MDT_UINT
} MSG_DATA_TYPE;

typedef struct tagRpmsgParserRULE
{
	int      		 nIndex;
	size_t   		 nOffset;
	int       		 nLen;
	MSG_DATA_TYPE    emDataType;
}RPMSG_PARSER_RULE;

#define MAP_ITEM(idx, parent, sub, field, type) \
		{  \
			.nIndex		= (idx),     \
			.nOffset    = offsetof(parent, sub.field), \
			.nLen   	= sizeof(((parent*)0)->sub.field),    \
			.emDataType = (type)  \
		}

#define MAP_BYTE(idx, parent, sub, field)		MAP_ITEM(idx, parent, sub, field, MDT_BYTE)
#define MAP_USHORT(idx, parent, sub, field)     MAP_ITEM(idx, parent, sub, field, MDT_USHORT)
#define MAP_INT8(idx, parent, sub, field)       MAP_ITEM(idx, parent, sub, field, MDT_INT8)
#define MAP_UINT8(idx, parent, sub, field)      MAP_ITEM(idx, parent, sub, field, MDT_UINT8)
#define MAP_INT16(idx, parent, sub, field)      MAP_ITEM(idx, parent, sub, field, MDT_INT16)
#define MAP_UINT16(idx, parent, sub, field)     MAP_ITEM(idx, parent, sub, field, MDT_UINT16)
#define MAP_UINT(idx, parent, sub, field)       MAP_ITEM(idx, parent, sub, field, MDT_UINT)

/********************************************************* end ***************************************************************/

typedef struct stPCSReread_Param
{
		float fParam1001;          // 0x10 0x01
        float fParam1002;    	   // 0x10 0x02
        int   nParam1110;          // 0x11 0x10
        int   nParam1127;   	   // 0x11 0x27
		int   nParam1128;   	   // 0x11 0x28
        int   fParam1133;     	   // 0x11 0x33
        float fParam2003;          // 0x20 0x03
        float fParam2101;          // 0x21 0x01
        float fParam2102;          // 0x21 0x02
        float fParam2103;          // 0x21 0x03
        float fParam2105;          // 0x21 0x05
        UINT  uParam2106;          // 0x21 0x06
        UINT  uParam2107;          // 0x21 0x07
        int   nParam2110;          // 0x21 0x10
        int   nParam2111;          // 0x21 0x11
        int   nParam2114;          // 0x21 0x14
        int   nParam2115;          // 0x21 0x15
        int   nParam2117;          // 0x21 0x17
        int   nParam2118;          // 0x21 0x18
        float fParam2120;          // 0x21 0x20
        float fParam2121;          // 0x21 0x21
        float fParam2122;          // 0x21 0x22
        float fParam2123;          // 0x21 0x23
        float fParam2124;          // 0x21 0x24
        float fParam2125;          // 0x21 0x25
        float fParam2126;          // 0x21 0x26
        float fParam2127;          // 0x21 0x27
        float fParam2128;          // 0x21 0x28
        float fParam2129;          // 0x21 0x29
        float fParam21210;         // 0x21 0x2A
        float fParam21211;         // 0x21 0x2B
        float fParam21212;         // 0x21 0x2C
        float fParam21213;         // 0x21 0x2D
        float fParam21214;         // 0x21 0x2E
        float fParam21215;         // 0x21 0x2F
        float fParam2130;          // 0x21 0x30
        int   bParam2140;          // 0x21 0x40
        int   bParam2141;          // 0x21 0x41
        int   bParam2142;          // 0x21 0x42
        int   bParam2143;          // 0x21 0x43
        int   bParam2144;          // 0x21 0x44
        int   nParam2145;          // 0x21 0x45
        int   nParam2146;          // 0x21 0x46

        int   bParam3111;          // 0x31 0x11
        int   bParam3112;          // 0x31 0x12
        float fParam3113;          // 0x31 0x13
        UINT  uParam3114;          // 0x31 0x14
        float fParam3115;          // 0x31 0x15
        UINT  uParam3116;          // 0x31 0x16
        UINT  uParam3117;          // 0x31 0x17
        float fParam3118;          // 0x31 0x18
        float fParam3119;          // 0x31 0x19
        float fParam31181;         // 0x31 0x18
        float fParam31191;         // 0x31 0x19
		float fParam3121;          // 0x31 0x21
        float fParam3122;          // 0x31 0x22
        float fParam3123;          // 0x31 0x23
        float fParam3124;          // 0x31 0x24
        float fParam3125;          // 0x31 0x25
        float fParam3126;          // 0x31 0x26
        float fParam3127;          // 0x31 0x27
        float fParam3128;          // 0x31 0x28
        float fParam31281;         // 0x31 0x2A
        float fParam3129;          // 0x31 0x29
        float fParam31211;         // 0x31 0x2B
        float fParam3131;          // 0x31 0x31
        float fParam3132;          // 0x31 0x32
        float fParam3133;          // 0x31 0x33
        float fParam3135;          // 0x31 0x35
        float fParam3136;          // 0x31 0x36
        float fParam3138;          // 0x31 0x38

        float fParam3150;          // 0x31 0x50
        float fParam3151;          // 0x31 0x51
        float fParam3152;          // 0x31 0x52
        float fParam3153;          // 0x31 0x53
        float fParam3154;          // 0x31 0x54
        float fParam3155;          // 0x31 0x55
        float fParam3156;          // 0x31 0x56
        float fParam3157;          // 0x31 0x57

        float fParam3160;          // 0x31 0x60
        int   nParam3161;          // 0x31 0x61
        int   nParam3200;          // 0x31 0x61
        float fParam3163;          // 0x31 0x63
        float fParam3164;          // 0x31 0x64
        float fParam3165;          // 0x31 0x65
        float fParam3166;          // 0x31 0x66
        float fParam3167;          // 0x31 0x67
        float fParam3168;          // 0x31 0x68
        float fParam3169;          // 0x31 0x69

        float fParam31610;         // 0x31 0x6A
        float fParam31611;         // 0x31 0x6B
        float fParam31612;         // 0x31 0x6C
        float fParam31613;         // 0x31 0x6D

        float fParam31614;         // 0x31 0x6E
        float fParam31615;         // 0x31 0x6F

        float fParam3173;          // 0x31 0x73
        float fParam3175;          // 0x31 0x75
        float fParam31751;         // 0x31 0x75
        float fParam3174;          // 0x31 0x74
        float fParam31741;         // 0x31 0x74
        float fParam3176;          // 0x31 0x76
        float fParam3177;          // 0x31 0x77
        float fParam3178;          // 0x31 0x78
        float fParam31781;         // 0x31 0x78
        
        float fParam3179;          // 0x31 0x79
        float fParam31710;         // 0x31 0x7A
        float fParam31711;         // 0x31 0x7B
        float fParam31712;         // 0x31 0x7C
        float fParam31713;         // 0x31 0x7D
        float fParam31714;         // 0x31 0x7E
        float fParam317141;        // 0x31 0x7E

        float fParam31110;         // 0x31 0x1A
        float fParam31111;         // 0x31 0x1B
        float fParam311101;        // 0x31 0x1A
        float fParam311111;        // 0x31 0x1B
        int   bParam31113;         // 0x31 0x1D

        UINT  uParam31121;         // 0x31 0xC1
        int   bParam31130;         // 0x31 0xD0
        int   bParam31131;         // 0x31 0xD1
        int   bParam31132;         // 0x31 0xD2
        int   bParam31133;         // 0x31 0xD3
        int   bParam31136;         // 0x31 0xD6
        int   bParam31138;         // 0x31 0xD8
        int   bParam31139;         // 0x31 0xD9
        int   bParam31156;         // 0x31 0x2E
        int   bParam311561;        // 0x31 0xF6
        int   bParam31157;         // 0x31 0xF7
        int   bParam31158;         // 0x31 0xF8
        float fParam31159;         // 0x31 0xF9
        float fParam311512;        // 0x31 0xFC
        float fParam31310;         // 0x31 0x3A
        float fParam31315;         // 0x31 0x2F
        float fParam313151;        // 0x31 0x3F

        float fParam31914;         // 0x31 0x9E
        float fParam31915;         // 0x31 0x9F
        float fParam3111111;       // 0x31 0xBB
        float fParam31234;         // 0x31 0xF3
        float fParam31250;         // 0x31 0xFA
        float fParam31251;         // 0x31 0xFB

        int   bParam311313;        // 0x31 0xDD
        int   bParam311315;        // 0x31 0xDF

        float fParam5101;          // 0x51 0x01
        float fParam5102;          // 0x51 0x02
        float fParam5103;          // 0x51 0x03
        float fParam5104;          // 0x51 0x04
        float fParam5105;          // 0x51 0x05

        int   bParam7140;          // 0x71 0x40
        int   bParam71411;         // 0x71 0x4B
        int   bParam71412;         // 0x71 0x4C

        float fParam3180;          // 0x31 0x80
        float fParam3181;          // 0x31 0x81
        float fParam3182;          // 0x31 0x82
        float fParam3183;          // 0x31 0x83
        float fParam3184;          // 0x31 0x84
        float fParam3185;          // 0x31 0x85
        float fParam3186;          // 0x31 0x86
        float fParam3187;          // 0x31 0x87
        float fParam3188;          // 0x31 0x88
        float fParam3189;          // 0x31 0x89
        float fParam31810;         // 0x31 0x8A
        int   nParam31811;         // 0x31 0x8B
        int   nParam31812;         // 0x31 0x8C
        float fParam31813;         // 0x31 0x8D
        float fParam31814;         // 0x31 0x8E
        float fParam3190;          // 0x31 0x90
        float fParam3191;          // 0x31 0x91
        float fParam3192;          // 0x31 0x92
        float fParam3193;          // 0x31 0x93
        float fParam3194;          // 0x31 0x94
        float fParam3195;          // 0x31 0x95
        float fParam3196;          // 0x31 0x96
        float fParam3197;          // 0x31 0x97
        float fParam3198;          // 0x31 0x98
        float fParam3199;          // 0x31 0x99
        float fParam31910;         // 0x31 0x9A
        float fParam31911;         // 0x31 0x9B
        float fParam31912;         // 0x31 0x9C

        int   bParam311011;        // 0x31 0xAB

        float fParam32001;         // 0x32 0x00
        float fParam3201;          // 0x32 0x01
        float fParam3202;          // 0x32 0x02
        float fParam3203;          // 0x32 0x03
        float fParam3204;          // 0x32 0x04
        float fParam3205;          // 0x32 0x05
        float fParam3206;          // 0x32 0x06
        float fParam3207;          // 0x32 0x07
		float fParam996;		   //认证参数
		float fParam997;		   //认证参数
		int   nParam998;		   //认证参数
		float fParam999;		   //认证参数
		int   nParam91011;         // 0x91 0x0B
} PCS_Reread_Param;

typedef struct stRoughCCUData
{
	HANDLE							hSampler;
	HANDLE							hSamplerLock; //From sampler info
	NLMSG_FRAME_INFO				nlFrameInfo[NL_MSG_Max];
	RPMSG_FRAME_INFO                rpFrameInfo[RP_MSG_MAX];
	DATA_PROCESSOR*					pDPR;
#ifdef IMMU2
	HANDLE							hDataComm; // tcp client
#endif

	HEART_DATA_INFO					strHEARTInfo;// 2524

	LSYS_DATA_INFO					strLSYSInfo;

	// LZY20260313
	ALL_MODULES_INFO                strAllModuInfo;
	ACDC_GRP_INFO                   strACDCGrpInfo;
	DCDC_GRP_INFO                   strDCDCGrpInfo;
	MPPT_GRP_INFO                   strMPPTGrpInfo;
	ACDC_SIG_INFO                   strACDCSigInfo;
	DCDC_SIG_INFO                   strDCDCSigInfo;
	MPPT_SIG_INFO                   strMPPTSigInfo;

	// LZY20260512 Module Config
	MODULE_TYPE_CFG					strModuleTypeCfg;
	MODULE_NUM_CFG					strModuleNumCfg;
	ACDC_CHRG_DISCHRG_CTRL			strACDC_ChrgDisChrgCtrl;
	DCDC_CHRG_DISCHRG_CTRL			strDCDC_ChrgDisChrgCtrl;
	MANUL_AUTO_MODE					strManulAutoCtrl;
	N2002_ON_OFF_GRID_CTRL			strN2002_OnOffGridCtrl;
	ACDC_PARAMS_SET					strACDCParamsSet;

	UINT 							nStuffWaitTimes;

	ESSYS_TYPE 						emESSystemType;

	//SYS_ALARM_MAP					sysAlarmMap;
	USYS_ALARM_MAP1					usysAlmMap1;

	SYSTEM_FAILREASON				emSystemFailReason;
	
    TCP_CCU_COMMSTATU   			emCommStatus;
 	 
 	CCU_STATE						emCCUState;
	
	UINT 							nAbnormalCurr_Count;

	A_BLOB_CHARGERSPECIFIC_SETTINGS pOldSet;
	A_BLOB_CHARGERSPECIFIC_SETTINGS pNewSet;

	BOOL 							bNeedSync;
	BOOL 							bNeedSyncTime2IMSU;
	time_t							tmLastSyncTime2IMSU;
	
	int								nCommFailTimes;
	BOOL							bCommFail;
	time_t							tmSysBoot; //system boot time

	time_t							tmLastCalcESStoreData; //calculate the data
	time_t							tmLastSaveESData2DB; //save data into DB
	
	time_t							tmLastCtrlACLoadMCB;  

	HIS_ES_STATISTIC_DAILY			hisESData; //daily statistic data for ES
	
	A_BLOB_SINGLE_STBATT_RUNINFO 	blobSingleBattData[MAX_SINGLE_STBATT_NUM];

	A_BLOB_GRP_RECT_AUTOCTRL_CMD	blobGrpRectCtrl;

	EMS_MGMT_STU 					emsMgmtData;

	EMS_ORG_CALC_DATA				emsOrgCalcData;

 	UINT 							emPmaxDemandMode;

 	float 							fBAT_Max_T_PwrLmtRate_Set;
	UINT 							nSysOverTempTimes;
    UINT                            emDisChgByStopSOC; // 管理计划放电关联截止SOC:(否,朗新 放电的soc填100%)

	A_BLOB_EMS_CONFIG_PLAN 			blobEmsCfgPlan; 

	ERR_CODE_MGMTCFG				emErrCode_MgmtCfg;

 	A_BLOB_ES_BACKEND_CFG 			blobBackendCfg;

	A_BLOB_CHARGE_FEESET			blobFeeRate;
	//A_BLOB_BATT_MGMT_PLAN 		battPlan;

	A_BLOB_ES_RUN_DATA				esRunData;

	double							tm_LastCtrlLED;

	STAT_PM_CTRL_MODE               emPMCtrlMode;
	STS_AUTOMANUAL_ONOFFGRID		emSTSAutoManual;
	STS_ONOFFGRID_STATE             emSTSOnOffGridState;
	//--- Modify When Reboot, need Get Last Hour From DB Blob ---//
    UINT							nLast_Hour;
    UINT							nLast_MDay;
    UINT							nLast_tmYear;
    FEERATE_ATTRI        			emLast_FeeRate;

	//----------- PCS Cmd Setting Params -----------//
	//A_BLOB_PCS_SETTINGPARAM			blobPcsSett[MAX_SINGLE_STBATT_NUM];
	char 							szSettCmdReply[64];
	A_CAN_RECT_FRAME				frmReply;
	A_CAN_RECT_FRAME				frmSent;

	//---------- Upgrade IMEU-D Bin --------------//
	CCU_UPGRADE_PARAM						tagCCUUpgradeParam;
	A_BLOB_UPDT_INFO_4CCU					blobUpdCmd;

	//------------Aux AC Meter -------------------//
	POS_AUXMETER							emAuxMeterPos;
	
	//------2023-12-24 Added For HT On/Off Grid -----//
	HMI_ON2OFFGRID_PARAM				tagOn2OffParam;

	//------ 2024-01-02 Added For GCAN For CCU-PM ------//
	PM2CCU_CANDATA_SAVE					tagPM2CCU_GCan;
	A_BLOB_CCU2PCSCAN_MSG				blobCanMsg;

	//------ 2024-06-15 Added For batt grp input contactor & pv contactor sticking error ------//
	UINT								emGrpContactor1_CtrlVal;  //0-Connected, 1-Disconnected, 2-Not Set
	UINT								emGrpContactor2_CtrlVal;
	UINT								emGrpContactor3_CtrlVal;
	UINT								emGrpContactor4_CtrlVal;
	int									nGrp1_StickingAlarmTimes;  //If more than 3 times, start alarm
	int									nGrp2_StickingAlarmTimes;
	int									nGrp3_StickingAlarmTimes;
	int									nGrp4_StickingAlarmTimes;
	UINT								emIsGrpContactor1_DisconnNeeded;  //0-no need to disconnect, 1-need to disconnect, 2-already disconnected
	UINT								emIsGrpContactor2_DisconnNeeded;
	UINT								emIsGrpContactor3_DisconnNeeded;
	UINT								emIsGrpContactor4_DisconnNeeded;
	
	UINT								emPVContactor1_CtrlVal;  //0-Connected, 1-Disconnected, 2-Not Set
	UINT								emPVContactor2_CtrlVal;
	UINT								emPVContactor3_CtrlVal;
	UINT								emPVContactor4_CtrlVal;
	int									nPV1_StickingAlarmTimes;  //If more than 3 times, start alarm
	int									nPV2_StickingAlarmTimes;
	int									nPV3_StickingAlarmTimes;
	int									nPV4_StickingAlarmTimes;
	UINT								emIsPVContactor1_DisconnNeeded;  //0-no need to disconnect, 1-need to disconnect, 2-already disconnected
	UINT								emIsPVContactor2_DisconnNeeded;
	UINT								emIsPVContactor3_DisconnNeeded;
	UINT								emIsPVContactor4_DisconnNeeded;
	// LJH241007 AntiPhase
	BOOL                                needAntiReflux_Phase;   // 单相防逆流使能
	BOOL                                needAntiOverLoad_Phase; // 单相防过载使能
	ESS_DEVICE_TYPE                     emESSDevType;
#define CCU_232_READ_TIMEOUT_SETT	300
	BOOL					            bIsReceivedCFGG_FromLCCU; // 是否已经收到过数据
	time_t					            tmLastRecvedData; // 最近一次收到数据时间戳
	BOOL							bEverRecvHeart; // keep-app-alive(2026-09): 是否曾成功收到 HEART 心跳帧；只置位不复位，用于门控通信中断重启。MUST NOT 在 InitCCUStuffData 中清零
	// LJH241111 PcsSCADA
	PcsSMItem                           pcsSMItems[PCS_STATE_MAX];
	PcsSMInfo                           pcsSMInfos[MAX_SCADA_PCS_NUM];
	// LJH241202 SEMS
	PcsSMItem                           semsSMItems[PCS_STATE_MAX];
	PcsSMInfo                           semsSMInfos[MAX_SEMS_CLIENTS_NUM];
	SAMPLER_INIT_PROC                   pfnInitialize[SYSTYPE_MAX];
	SAMPLER_SAMPLE_PROC                 pfnSamples[SYSTYPE_MAX];
	SAMPLER_SAMPLE_PROC                 pfn_can_Samples;

	UINT                                msPeriodLSYS; // 从机上报站级SEMS_LSYS周期
	// LJH241202 SEMS rundata
	RunDataExt_SEMS                     runDataExt_SEMS;
	HIS_BMS_RUNTIMEDATA_REC             hisBmsRunDataRec;

	UINT 								nCntSendOnOffGridCmd2CCU;
	double 								tmLastSendCmd2CCU;
	int 								nUpgradeProcess;		// for IMMU3 upgrade process

	float 								fTotalPowerLimit;        //A2419 35KW  Protect cabinet
	time_t								tLoadLimitTime_over110;
	time_t								tLoadLimitTime_over120;

	float								fcabPowerConsump;
	time_t								tPowerUpdataTime;

	UINT								emCurrDiffAbnormal;	//0 normal, 1 abnormal
	LCCfg                               lcCfg; // LJH250508 LCCfg
	int 								nEssSemCCUWith485Meter;
	int 								nSemsSemCCUWith485Meter;
	time_t								tSentCCUDisChargeCmdTime;
	BYTE                                byVerSEMSProt_Client; // LJH250630 SEMSProtVer
	UINT								emOnOffGridCtlNLineGround;	//切换并离网EMS是否控制N线接地控制器: 0 Yes, 1 No
	BOOL 								bQuickChannle;
	UINT								nPCSNum;
	time_t							    tmLastSend_Cmd;
	UINT								nDisChrgFilterPoint;

	GRID_APPLY_SCENERY					tagGridApplyScenery;
	UINT								nSetPriGridMeter; // 主备模式下主路电表选择：0-auto 1-H 2-H1 3-H2 4-H3
	UINT								nCurrPriGridMeterId; // 主备模式下 当前主路电表id
	//------ 2025-01-24 Added For PCS module param reread ------//
	PCS_Reread_Param					PCSRereadParam[4 * MAX_SINGLEPCS_RECT_NUM];//PCS Reread param

	// RPMSG 核间通信端口（与 CAN 并存）
    COMMPORT                            pRpmsgComm;        // RPMSG 端口
    BOOL                                bRpmsgPortReady;   // RPMSG 端口准备就绪标志

	HANDLE                              hRpmsgPCS_InfoDev;          // RPMSG PCS信息 句柄
	BOOL                                bRpmsgPCS_InfoReady;   		// RPMSG PCS信息 句柄准备就绪标志
	HANDLE                              hRpmsgPCS_CfgDev;			// RPMSG PCS配置 句柄
	BOOL                                bRpmsgPCS_CfgReady;			// RPMSG PCS配置 句柄准备就绪标志

	HANDLE                              hRpmsgCycleDev;

    HANDLE				        hCanDevHanle;   
    COMMPORT			        pComm;
    BYTE				        abyRcvBuf[MAX_READ_BUFF_SIZE * CAN_FRAME_LEN];
	ACDC_DATA_INFO			 	acdc_data;
	BYTE						bAllConfirmed;

}ROUGH_CCU_DATA;

// LJH241202 SEMS rundata
typedef int (*NLMSG_FRAME_PARSER_SEMS)(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usDataLen, int n);
typedef struct
{
	NL_MSGTYPE	            msgTp;
	BYTE		            byCmd;
	NLMSG_FRAME_PARSER_SEMS	pfnParser;
	USHORT		            usMin_ExpectDataLen; //minimal expected data length
}NLMSG_FRAME_INFO_SEMS;
typedef struct
{
	HANDLE              hClientUDPPort;
	BOOL				bSocketCreatFail;

}CLINET_UDP_INFO;

typedef struct tagSemsInfo
{
    ROUGH_CCU_DATA          *pCCU;
	//msg list
	NLMSG_FRAME_INFO_SEMS    nlFrameInfoSEMS[NL_MSG_Max];

	// client
	A_BLOB_SEMS_CLIENTINFO   MMClients[MAX_SEMS_CLIENTS_NUM];
	ULONG                    ulIP_FirstClient;
	UINT                     nInHandlingClient; //which client is in handling

#define CCU_TCP_UPGRADESTATUS_IDLE 					0
#define CCU_TCP_UPGRADESTATUS_WAIT_FILETRANS 		1
#define CCU_TCP_UPGRADESTATUS_PENDING_FILETRANS 	2
#define CCU_TCP_UPGRADESTATUS_SUCCESS_FILETRANS 	3
#define CCU_TCP_UPGRADESTATUS_FAILED_FILETRANS 		4
#define CCU_TCP_UPGRADESTATUS_FINISHED		 		5
   //add by wwa904 for LFRMDTA command
	BYTE					emCCUTCP_UpgradeStatus;	
	time_t					tmLastReceivedLSYS;
	UINT					nBinFile_TotalBytes;
	UINT					nUsed_SingleFrameSize; //frame size for each frame
	BYTE*					pbyBinBuffer; //bin buffer from reading		
	BYTE					byLastUpdate_Percent;	
	UINT 					nTotal_Pkg2Transfer; //total pkgs that need transferred 
	UINT					nTransferred_Pkg; //already transferred pkgs
	//last time get UpdateCmd from CCU
	time_t					tmLastReceiveCCUUpgdadeCmd;
	//client UDP INFO
	CLINET_UDP_INFO   ClientsUdpInfo;
} SemsInfo;

typedef struct tagPlanSector
{
	UINT                    nBattGrpPwrLmtPlan; // 单组电池组功率限制
	int                     nReActvPwrLmtPlan;
	UINT                    nActvPwrLmtPlan; // 总功率限制
	BYTE                    nStopSOCPlan;
	BYTE                    emBatt_WorkModePlan; // STBATT_PRI_WORKMODE
} PlanSector; // EMS_LIMIT_SECTOR

extern SemsInfo s_SemsInfo;
extern A_BLOB_SEMS_CLIENTINFO* s_MMClients; // s_SemsInfo.MMClients -> A_BLOB_SEMS_CLIENTINFO   MMClients[MAX_SEMS_CLIENTS_NUM];
extern COMMPORT * 	s_pUDPComm;

//----------LCCU protocol parser-------
int Parse_CFGG(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_CFGS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_UCTL(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_RTIF(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_GCAN(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);

int Parse_PCSGet(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_PCSSet(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_Read_PCSSett(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);

int Parse_USIZ(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_UDTA(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);

int Parse_LSYS(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_LDEV(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);
int Parse_HEART(ROUGH_CCU_DATA* pCCU, BYTE* pData, USHORT usLenID);

BOOL On_Handle_PCSSettingCmd(ROUGH_CCU_DATA *pCCU, int nDevId, int nParamId, VAR_VALUE *pVarVal, BOOL bSettCmd, BOOL bIsGroupCmd, int nWhichSingleModule);
BOOL On_Handle_RPMsgSettingCmd(ROUGH_CCU_DATA *pCCU, int nDevId, int nParamId, VAR_VALUE *pVarVal, BOOL bSettCmd);
BOOL On_Handle_PCSGettCmd(ROUGH_CCU_DATA *pCCU, int nDevId, int nParamId, VAR_VALUE *pVarVal);
void OnHandle_RereadPCS_Param(ROUGH_CCU_DATA* pCCU, UINT nDevId, UINT nParamId, VAR_VALUE* pVarVal, int nWhichSingleModule);
BOOL Refresh_PCSVersion(ROUGH_CCU_DATA *pCCU);

void Judge_IfNeedQueryPCS_Settings(ROUGH_CCU_DATA *pCCU);

//---------Pub functions-----------
void InitCCUStuffData(ROUGH_CCU_DATA* pCCU);
void PushAllData(ROUGH_CCU_DATA* pCCU);
void PushAllRereadPCSData(ROUGH_CCU_DATA* pCCU, UINT nDevId, UINT nParamId, VAR_VALUE* pVarVal, BOOL IsSetCmd);;//only for PCS Reread Param Data

void Var_To4Bytes(IN VAR_VALUE varV, BYTE* pbyOut);
int Event_Handler(HANDLE		hModuleSeif,
					_EVENT_TYPE	emEvType,
					_EVENT_MSG	emMsgType,
					UINT		nEvDataSize,
					void*		pEvData,
					BOOL		bBlockMode,
					int			nMaxBlockMSec);

//the following are for NL_Msg
int MakeNLMSG_SendFrame(BYTE byCmd, IN USHORT usSendDataLen, IN BYTE* pData, OUT BYTE *sendBuff);
NLMSG_FRAME_INFO* Find_NLMsgFrame(IN ROUGH_CCU_DATA* pCCU, IN BYTE* pRaw, IN int nReadLen, 
								OUT BYTE** ppDataPos, OUT USHORT* pusDataLen);
void Manage_ESStoreData(ROUGH_CCU_DATA* pCCURough);
void Send_USYS(ROUGH_CCU_DATA* pCCU);
void Prepare_ESRunData(ROUGH_CCU_DATA* pCCU);
BOOL JudgeIsoPendingStatus(ROUGH_CCU_DATA* pCCU, TYPE_SIGNLE_RECT emGrouId);
BOOL JudgeisNeedSkipSinglePCSAlarm(ROUGH_CCU_DATA* pCCU);
void Init_CurrDayKWH(ROUGH_CCU_DATA *pCCU, int nDayIdx);
//-------------2023-01-17-------------//
void HandleIMSUUpgrading(IN ROUGH_CCU_DATA* pCCU);
void Send_DebugMsg_2UI(IN ROUGH_CCU_DATA* pCCU, BOOL bFinish, 
							BOOL bError, BOOL bLog, char *pszFormat, ...);
BOOL Prepare_CCUBin_Updte(IN ROUGH_CCU_DATA* pCCU, int nTimeout_mSec);
int OnHandle_CtrlCmd4CANMsgTrig(IN ROUGH_CCU_DATA *pCCU, IN _ENUM emValue);
BOOL Init_blobEmsCfgPlanParam(ROUGH_CCU_DATA *pCCU);
void Init_EmsMgmtData(ROUGH_CCU_DATA* pCCU);
void Refresh_SysAvailState(ROUGH_CCU_DATA* pCCU);
void Update_SysDataInfo(IN ROUGH_CCU_DATA* pCCU);
// LJH250808 从控只取管理计划第一段
BOOL GetCurrentTimeSector(ROUGH_CCU_DATA* pCCU, BOOL bFromOrg);
ERR_CODE_MGMTCFG Check_MgmtPlanCfg(IN ROUGH_CCU_DATA* pCCU);
void MangeLEDStatus(ROUGH_CCU_DATA* pCCU);
void CalcMPPTGenerateKWH(ROUGH_CCU_DATA* pCCU);
void CalcDCLoadCostKWH(ROUGH_CCU_DATA* pCCU);
// LJH241101 MU3
BOOL CCU_Check_And_ReOpen_CommPort(SAMPLER* pSampler, BOOL bModuleUpdate);
void CCU_Close_CommPort(SAMPLER* pSampler);
// void CCU_Handle_CommFail(ROUGH_CCU_DATA* pCCU, BOOL bCommFailed);
int CCU_WriteData(ROUGH_CCU_DATA* pCCU, BYTE *szBuff, int nSendLen);
int CCU_ReadData(ROUGH_CCU_DATA* pCCU, BYTE *rdBuff, int nReadLen);
BOOL DoTCPSession(ROUGH_CCU_DATA* pCCU, NL_MSGTYPE emMsgTp, BYTE* pData, int nDataLen, int nTimeout_ms);
#ifdef IMMU3
	BOOL CCU_Wait_ACCU_Reply_RS232(ROUGH_CCU_DATA* pCCU, NLMSG_FRAME_INFO* pFramInfo, int nTimeOut_mSec,
								BYTE* pDataSend, int nDataLenSend);
#else
	void Recv_Tcp_CCU_Msg(ROUGH_CCU_DATA* pCCU);
	void CCU_TCP_Close_Client(ROUGH_CCU_DATA* pCCU);
#endif

// LJH241202 SEMS
BOOL Initialize_ESS(HANDLE hSamplerSelf);
BOOL Initialize_EMS_STATION(HANDLE hSamplerSelf);
// BOOL Initialize_SCADA_LOCALCTR(HANDLE hSamplerSelf);
// BOOL Initialize_SCADA_INPCS(HANDLE hSamplerSelf);

int Sample_ESS(HANDLE hSamplerSelf);
int Sample_EMS_STATION(HANDLE hSamplerSelf);
// int Sample_SCADA_LOCALCTR(HANDLE hSamplerSelf);
// int Sample_SCADA_INPCS(HANDLE hSamplerSelf);

void SetPlanSector(PlanSector* pPlanSector,
	BYTE emBatt_WorkMode, BYTE nStopSOC, UINT nActvPwrLmt, int nReActvPwrLmt, UINT nBattGrpPwrLmt);
void ModifyBlobEmsCfgPlan(ROUGH_CCU_DATA* pCCU, BOOL bRestored, PlanSector* pPlanNow, PlanSector* pPlanOrg);
void GetCtrlParam_REMS(ROUGH_CCU_DATA* pCCU); // LJH250902 top#6+REMS
void Ems_init(ROUGH_CCU_DATA *pCCU);
void Ems_Reset_Calc_Node(A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node);
// void Ems_Mgmt_Total_Pess(ROUGH_CCU_DATA* pCCU);
// void Ems_Mgmt_Total_Pess_REMS(ROUGH_CCU_DATA* pCCU); // 上级EMS
// void Ems_Mgmt_Total_Pess_EMSClient(ROUGH_CCU_DATA* pCCU); // 从机Pess
void Ems_Mgmt_Total_Pess_All(ROUGH_CCU_DATA* pCCU);
void Refresh_ESSRemainPower(ROUGH_CCU_DATA* pCCU);
void Sort_NodeOrderByChrgedEnergy(ROUGH_CCU_DATA *pCCU);
void Sort_NodeOrderBySoc(ROUGH_CCU_DATA *pCCU, WORKMODE_REQ workmodeReq);
void MPET_Calc4DispatchChrgMode(ROUGH_CCU_DATA *pCCU);
void Dispatch_ESSLmtBattChrg_Power(ROUGH_CCU_DATA* pCCU);
void MPET_Calc4DispatchDisChrgMode(ROUGH_CCU_DATA *pCCU);
void DisPatch_ACLoadReq_DisChrg_Power(ROUGH_CCU_DATA* pCCU);
void DisPatch_ReactivePower_Output(ROUGH_CCU_DATA* pCCU);
void Sort_NodeOrderByNodeIdx(ROUGH_CCU_DATA *pCCU);
void Update_RuntimeData_2DB_SEMS(ROUGH_CCU_DATA* pCCU, int n);
U6330WorkAsap Make_WorkAsap(BOOL bChrgAsap, BOOL bDiscAsap);
STBATT_PRI_WORKMODE MapACDC_WorkStatu2BattWorkMode(ACDC_WORKSTATU emACDC_WorkStatu);
ACDC_WORKSTATU MapBattWorkMode2ACDC_WorkStatu(STBATT_PRI_WORKMODE emBatt_WorkMode);
GRP_RECT_CTRL_CMDTYPE MapBattWorkMode2GrpRectCtrlCmdType(STBATT_PRI_WORKMODE emBatt_WorkMode);

void AutoCharging_IfReflux(ROUGH_CCU_DATA* pCCU);
void AutoDischarging_IfOverload(ROUGH_CCU_DATA* pCCU);

// LZY01508 251226 PID Control Algorithm
void PID_Init(ROUGH_CCU_DATA *pCCU);
void Power_Control_Loop(ROUGH_CCU_DATA *pCCU);

// void AutoCharging_IfRefluxEss(ROUGH_CCU_DATA* pCCU);
BOOL Send_CCU_Ctrl(ROUGH_CCU_DATA* pCCU, A_BLOB_GRP_RECT_AUTOCTRL_CMD CCUCtrlCmd);
void Save_EmsMgmtPlan(A_BLOB_EMS_CONFIG_PLAN *pEmsMgmtPlan);


// D9 platform
// Function Calare
RPMSG_FRAME_INFO* Find_RPMsgFrame(IN ROUGH_CCU_DATA* pCCU, IN BYTE* pRaw, IN int nReadLen, 
								OUT AR_FRAME* pMsgFrame);
								
int Parse_AllModulesInfo(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_ACDC_Grp_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_DCDC_Grp_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_MPPT_Grp_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_ACDC_Sig_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_DCDC_Sig_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_MPPT_Sig_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);

int Parse_Module_Type_Cfg(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_Module_Num_Cfg(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_ACDC_ChrgDischarge_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_DCDC_ChrgDischarge_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_Manul_Auto_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_N2002_OnOffGrid_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);
int Parse_ACDC_Params_Set(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame);

void Recv_RpMsg_CCU_Msg(ROUGH_CCU_DATA* pCCU);
void Recv_RpMsg_CCU_Reply_Msg(ROUGH_CCU_DATA* pCCU);
void SendRpMsg_CCU(ROUGH_CCU_DATA* pCCU, FUNCTION_CODE emFunCode, 
    RP_MSGTYPE emMsgTp, int nDataLen, BYTE* pData);
void BuildRpmsgCommChannel(ROUGH_CCU_DATA* pCCU);
void TASK_N2402_Proc(void *argument);
int CAN_Samples(HANDLE hSamplerThis);
void PushPCSData(ROUGH_CCU_DATA* pCCU);

#endif
