#ifndef __DEV_485_2_SAMPLER_MAIN_H
#define __DEV_485_2_SAMPLER_MAIN_H

#include "../sampler_base.h"

#define STAT_DEV_ON  			1
#define STAT_DEV_OFF 			0

//Air Condition Work Status
#define STAT_AIRCOND_STANDBY 		1
#define STAT_AIRCOND_WORKING     	0

#define STAT_Shutter_CloseOK				0
#define STAT_Shutter_NotCloseTotally		1		

#define STAT_Shutter_OpenOK					0
#define STAT_Shutter_NotOpenTotally			1


#define MAX_IMEU_NUMBER				4

#define MAX_DEV_485_2_LOCK_WAIT		100

#define MAX_AIR_COND_CMD_INTERVAL	1000
#define MAX_AIR_COOLONOFF_INTERVAL	500

#define MAX_ENTERECO_SEC			1500


#define	RS485_COMM_OK				0
#define	RS485_WRITE_ERR				1
#define	RS485_READ_ERR				2

#define MAX_RS4852_TRY_TIMES		10

#define MAX_SHUTTER_CTRL_TRY_TIMES  3

#define YDN_FRAME_SOI				0x7E		
#define YDN_IMEU_FRAME_VER			0x22
#define YDN_FRAME_VER				0x21
#define YDN_FRAME_ADR				184
#define YDN_FRAME_CID1				0x71
#define YDN_FRAME_EOI				0x0D


#define MIN_SAMP_IMEU2_PERIOD		4

#define MAX_ACWH_NUM				6

//-------- ECO Mode --------//
#define STAT_NORMAL_RUN 	0
#define STAT_ECO_MODE 		1

#define SETT_NORMAL_RUN		0

#define ECO_CHANGE_TIMEOUT	600
#define ECO_OFFSET_TEMP		5

#define AIRCOND_STOP_COOLPOINT	27
#define AIRCOND_ON_COOLPOINT	18
#define AIRCOND_ON_COOLOFFSET	9


typedef enum tagCode4ECOMode
{
	CODE_NONEED_EXIT = 0,
	CODE_DISABLE_ECO,
	CODE_ENTERECO_BELOW5MIN,
	CODE_MANUALCTRL_MODE,
	CODE_URGTDISCHRG_MODE,
	CODE_WRONG_EMSPLAN,
	CODE_DISCHRG_MODE,
	CODE_NEED_CHRG,
	CODE_BATT_RUNNING,
	CODE_EMS_INTRVL_SHORT,
	CODE_ENTER_ECO,

 }STATU_ECO_CODE;


#pragma pack(1) 
typedef struct tagYDNFrame
{
	BYTE		byFrameSOI;
	BYTE		byProtVer[2];
	BYTE		byDestAddr[2];
	BYTE		byCID1[2];
	BYTE		byCID2[2];
	USHORT		usLen[2];
	BYTE		*pbyData;
	USHORT		usChkSum[2];
	BYTE		byEOI;
}YDN_FRAME;
 #pragma pack()

typedef enum tagIMEU_FRAME_TYPE
{
	TP_FRM_GET_7141_IMEU = 0,	
	TP_FRM_GET_7142_ARICDT,	
	TP_FRM_GET_7143_TEMPHUMI,
	TP_FRM_GET_7151_VER,	
	TP_FRM_SET_7144_SETPARAM,

	_MAX_IMEU_FRAME_TYPE
}IMEU_FARAME_TYPE;

typedef enum tagIMEU2_V20FRM_TYPE
{
	TP_FRM_GET_7141_IMEU_V20 = 0,	
	TP_FRM_GET_7142_ARICDT_V20,	 //Get QiLi Air Condition
	TP_FRM_GET_7143_TEMPHUMI_V20,
	TP_FRM_GET_7151_VER_V20,
	TP_FRM_GET_7152_HD_AIRCDT_V20,
	TP_FRM_GET_7153_GAS_SENSOR_V20,

	TP_FRM_SET_7144_SETPARAM_V20,
	TP_FRM_SET_7145_CTRL_V20,

	_MAX_IMEU2_FRM_TYPE_V20
}IMEU2_FRM_TYPE_V20;

typedef enum tagIMEU_CabSafe_Manage
{
	CAB_SAFE_STATE_IDLE = 0,
	//CAB_SAFE_STATE_FIRST,
	CAB_SAFE_STATE_SECOND,
	CAB_SAFE_STATE_THIRD,
	//CAB_SAFE_STATE_FOURTH,

	CAB_SAFE_STATE_MAX
	
}CAB_SAFE_MANAGE_STATE;

typedef enum tagIMEU_SEND_CMD_TYPE
{
	CMD_CTRL_SHUTTER = 0,
	CMD_CTRL_SUB_SHUTTER,
	CMD_CTRL_AIRCOND_STANDBY,
	CMD_CTRL_AIRCOND_SUPPLY_AIR,
	CMD_SET_BATT_TYPE,
	CMD_SET_AIRCOND_CRYOGEN_POINT,
	CMD_SET_AIRCOND_COOLRET_POINT,
	CMD_SET_AIRCOND_HEATING_POINT,
	CMD_SET_AIRCOND_HEAT_DEVIATION,
	CMD_SET_AIRCOND_DEHUMI_STARTTEMP,
	CMD_SET_AIRCOND_DEHUMI_STOPTEMP,
	CMD_SET_AIRCOND_DEHUMI_STARTHUMIDITY,
	CMD_SET_AIRCOND_DEHUMI_STOPHUMIDITY,
	CMD_CTRL_DO2,
	CMD_CTRL_DO3,
	CMD_CTRL_DO4,
	CMD_CTRL_DO5,
	CMD_CTRL_AIRCOND_ONOFF,
	CMD_CTRL_AIRCOND_COOL_MODE,
	CMD_CTRL_AIRCOND_HEAT_MODE,

	SEND_CMD_MAX
	
}IMEU_SEND_CMD_TYPE;
 
typedef struct tagModbusFrame
{
	BYTE        byAddr;
	BYTE		byFuncID;
	USHORT		nRegAddr;
	USHORT		nRegQuantity;
	USHORT		nExpectAnswerLen;

	int 		nSetRegAddr;
	int			nSetRegValue;
	//BYTE		byCRC[2];
}MODBUS_SEND_FRAME;


typedef union tagIMEU2DOCtrlValue
{
	USHORT	usFlag;
	struct
	{
		USHORT	DO1 : 1;
		USHORT	DO2 : 1;
		USHORT	DO3 : 1;
		USHORT	DO4 : 1;
		USHORT	DO5 : 1;
		USHORT	DO6 : 1;
		USHORT	DO7 : 1;
		USHORT	DO8 : 1;
	}Ctrl;
}IMEU2_DO_VALUE;

typedef struct tagAIMEUFrameInfo
{
	IMEU_FARAME_TYPE		frameType;
	YDN_FRAME				sendFrame;
	UINT					nDataLen;
	UINT					nFrameLen; //this is the real len used by app
	int						nExpectAnswerLen;
}IMEU_FRAME_INFO;


typedef struct tagIMEU2FrmInfoV20
{
	IMEU2_FRM_TYPE_V20		frmType_V20;
	YDN_FRAME				sendFrame;
	UINT					nDataLen;
	UINT					nFrameLen; //this is the real len used by app
	int						nExpectAnswerLen;
}IMEU2_FRM_INFO_V20;

/*
#define KWH_WAKEUP_FRAME	(0xFEFEFEFE)
typedef enum tagKWHACMeterProtcType
{
	KWHACMETER_PROTOCOL_GBDLT645,
	KWHACMETER_PROTOCOL_MODBUS_EM737,
	KWHACMETER_PROTOCOL_MODBUS_RM_PRIVATE
 
}KWHACMETER_PROTOTP;

typedef enum kwhMeter_Roughs
{
	//AC
	KWHMETER_ROUGH_RTkWh_AC,
	KWHMETER_ROUGH_RTkvar,
	KWHMETER_ROUGH_RT_VOLT_A,
	KWHMETER_ROUGH_RT_VOLT_B,
	KWHMETER_ROUGH_RT_VOLT_C,
	KWHMETER_ROUGH_RT_CURR_A,
	KWHMETER_ROUGH_RT_CURR_B,
	KWHMETER_ROUGH_RT_CURR_C,

	KWHMETER_ROUGH_RTACTPower,
	KWHMETER_ROUGH_RTReActPower,
	KWHMETER_ROUGH_RT_RVSkWh_AC,
	KWHMETER_ROUGH_RT_PowerFactor,
	KWHMETER_ROUGH_RT_Freq,
	KWHMETER_ERR_WORD, //Received Msg that is not normal
	_MAX_KWHAC_ROUGH
}KWH_ROUGH_SIG;

#pragma pack(1) 
typedef struct tagkwhMeterFrame
{
	BYTE		byFrameSOI;
	BYTE		byAddrA0_A5[6];
	BYTE		byFrameSOI_2;
	BYTE		byCtrlCode;
	BYTE		byDataLen;
 
 	BYTE		*pbyData; 
	BYTE		byChkSum;
	BYTE		byEOI;
}KWH_A_FRAME;
#pragma pack()


typedef enum tagKWH_FRAME_TYPE
{
	KWH_FRAME_TYPE_GET_TOTALWH_AC = 0,

	KWH_FRAME_TYPE_GET_VOLT_PHASE_A,
	KWH_FRAME_TYPE_GET_VOLT_PHASE_B,
	KWH_FRAME_TYPE_GET_VOLT_PHASE_C,
	KWH_FRAME_TYPE_GET_CURR_PHASE_A,
	KWH_FRAME_TYPE_GET_CURR_PHASE_B,
	KWH_FRAME_TYPE_GET_CURR_PHASE_C,

	KWH_FRAME_TYPE_GET_TOTALKVAR_AC, //wugong gonglv

	KWH_FRAME_TYPE_GET_ACTPOWER_AC,  
	KWH_FRAME_TYPE_GET_REACTPOWER_AC,
	KWH_FRAME_TYPE_GET_RVSKWH_AC,
	KWH_FRAME_TYPE_GET_POWERFACTOR,	
	KWH_FRAME_TYPE_GET_FREQ,		

	_MAX_KWH_FRAME_TYPE
}KWH_FARAME_TYPE;
*/
typedef CAB_SAFE_MANAGE_STATE (*CAB_SAFE_STATE_PROC)(void* pArg, int nWhichIMEU);
typedef struct tagCabSafeStateMachineItem
{
	//char					    szStateName[16];
	CAB_SAFE_MANAGE_STATE		emState;
	CAB_SAFE_STATE_PROC			pfnAction;
	time_t				    	tm_Enter;  // the first time that enter this state

}CAB_SAFE_STATE_MACHINE_ITEM;

/*
typedef struct tagAWHFrameInfo
{
	KWH_FARAME_TYPE		frameType;
	KWH_A_FRAME			sendFrame;
	UINT				nFrameLen; //this is the real len used by app
	int					nExpectAnswerLen;
	int					nDataLen;
	int					nPointNum;
	float				fGetData;  
}KWH_FRAME_INFO;

typedef struct tagKWHMeterData
{
	METER_TYPE			emMeterType; //get from setting
	BYTE				byReadMeterA0A6[6]; //address from setting
	float				fACMeter_PowerFactor; //from setting
	KWHACMETER_PROTOTP	emACMeter_ProtocolType;
	char				szBaudrateSett[32];
	BYTE				byModbusAddress; //modbus address

	VAR_VALUE			varWHMeterRough[_MAX_KWHAC_ROUGH];
	int					nInterrupt_Times;
	BOOL				bIsCommFail;
}KWHMETER_DATA;
*/

typedef struct tagIMEU_CtrlData
{
	IMEU2_DO_VALUE			doValue;
	BOOL                    bShutterCtrl;
	BOOL                    bAirCondOnOffCtrl;
	BOOL                    bAirCondCoolCtrl;
	BOOL                    bAirCondSupplyAirCtrl;
	BOOL                    bAirCondStandbyCtrl;
	BOOL                    bAirCondHeatCtrl;

}IMEU2_CTRL_DATA;

typedef enum tagManage_Ctrl_Mode
{
	MANAGE_CTRL_MODE_AUTO = 0,
	MANAGE_CTRL_MODE_MANUAL

}MANAGE_CTRL_MODE;

typedef enum tagCtrl_Mode_Action
{
	ACTION_NONE = 0,
	ACTION_ENTER_MANUAL,
	ACTION_QUIT_MANUAL,

}CTRL_MODE_ACTION;


typedef struct tagIMEU2_RoughData
{
	IMEU_INSTALLED_TYPE			emIMEU2Type;

	IMEU_FRAME_INFO				frm_imeuFrames[_MAX_IMEU_FRAME_TYPE];

	//A_BLOB_IMEU2_DATAINFO		blobIMEU2Data;

	//2021-09-15 IMEU2 V20 Protocal Format
	IMEU2_FRM_INFO_V20			frmIMEU2_V20[_MAX_IMEU2_FRM_TYPE_V20];
	A_BLOB_BATTCAB_IMEU2_V20	blobIMEU2_V20;

	IMEU2_DO_VALUE				doValue;
	
	UINT                        nCombGasAlarmPoint;
	UINT                        nCombGasStopRetDiff;

	//IMEU2_CTRL_DATA      		blobEnterManualCtrlData;
	//IMEU2_CTRL_DATA      		blobQuitManualCtrlData;
	
	BOOL						bNeedReconfig;
	BOOL                        bNeedSyncSetParam;

	int							nInterrupt_Times;

	BOOL						bIsCommFail;
	MANAGE_CTRL_MODE            emManageCtrlMode;
	
	//CTRL_MODE_ACTION            emCtrlModeAction;

	BOOL                    	bMonitorShutter;
	time_t                  	tmShutterFanCtrl;

	int                         nShutterOpenResult;
	int                         nShutterClosedResult;

	float                       fCoolPoint_Sync;
	float                       fCoolRet_Sync;
	float						fHeatPoint_Sync;

	float						fDehumiStartTemp_Sync;
	float						fDehumiStopTemp_Sync;
	float 						fHumidityStart_Sync;
	float 						fHumidityStop_Sync;

	float						fAmbit_TcoolonSett;
	float						fBatt_Tmax_coolonSett;
	float						fAmbit_TcooloffSett;
	float						fBatt_Tmax_cooloffSett;
	float						fAirEnTran_TlmtSett;

	float						fAmbit_TheatonSett;
	float 						fHumidity_StartSett;

	float						fECOModeTempOffset;

	float 						fBattTmax_Samp;
	float 						fBattTmin_Samp;

	//float						fLast_SetCoolPoint2AirCond;
	//float						fLast_SetCoolRet2Air;

	int                         nShutterCtrlCmd;
	int                         nShutterCtrlTimes;

	UINT 						nPT1000Num;
	UINT                        emTempImbalance;

	CAB_SAFE_STATE_MACHINE_ITEM *pCurr_StateMachineItem;
	
	BOOL                        bCabSafeStateChanged;
	CAB_SAFE_MANAGE_STATE		emCabSafeState;
	CAB_SAFE_MANAGE_STATE		emLast_CabSafeState;

	time_t 						tmLastCtrlAirCond;
	time_t 						tmLastStartCool_AirCond;

	IMEU_SEND_CMD_TYPE 			emLastAirCondCmd;
	UINT						nLastCmdVal;

	//-----	 		For ECO 		-----//
	UINT 							emECORunStatu;
 	STATU_ECO_CODE					emStatCode;
	A_BLOB_SINGLE_STBATT_RUNINFO 	blobSingleBattData;

	time_t							tm_LastChangeECOMode;
	
	UINT							nAirCondAlmTimes;


}IMEU2_ROUGH_DATA;

typedef enum tagLEDSTATE
{
	START1_STATE = -1,
	GREEN = 0,
	RED,
	BLUE,
	YELLOW,
	WHITE
}LED_STATE;


struct stDev485_2_ROUGH
{	
	HANDLE							hSampler; //pointer to sampler
	HANDLE							hPort; //opened port handle
	DATA_PROCESSOR*					pDPR;

	int								nSamplerId; //copy from sampler
	
	HANDLE 							hSamplerLock;

	BYTE							abySendBuf[RS485_SEND_BUF_LEN];
	BYTE							abyRcvBuf[RS485_RECV_BUF_LEN];
	int								nReadBuffBytes; //Bytes that have been readout	
	
 	IMEU2_ROUGH_DATA				imeu2_RoughData[MAX_IMEU_NUMBER];

	//UINT							nSampACMeterCycle;
	//KWHMETER_DATA					meterData[MAX_ACWH_NUM];
	//KWH_FRAME_INFO					frm_whFrames[_MAX_KWH_FRAME_TYPE];

	CAB_SAFE_STATE_MACHINE_ITEM		StateMachineLoop[CAB_SAFE_STATE_MAX];
	int								nSampleCount;
	BOOL							bNeedSampleIMEU;// if need sample IMEU


	UINT							emECOModeSetting;
	UINT							nProtectSOC;
	A_BLOB_EMS_CONFIG_PLAN 			blobEmsCfgPlan; 

	//WLD01206  control led date
	ESS_DEVICE_TYPE                 emESSDevType;
	STBATT_ALARM_STATU 				emBattFaultStatu;
	A_BLOB_ES_RUN_DATA				esRunData;
	CCU_STATE						emCCUState;
	BYTE							bySTSOffGrid;
	LED_STATE						emLED1LastState;
	LED_STATE						emLED2LastState;
	time_t							t_Interval_time

};	
typedef struct stDev485_2_ROUGH		RS485_2_SAMPLER_DATA;
 
void 	Dev_ShutterFan_Mgmt(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU);
ERRCODE_SAMPLER SendAirCondCmd2IMEU(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU, IN IMEU_SEND_CMD_TYPE emCurrAirCondCmd, IN int nCurrCmdVal, BOOL bNeedWait);
ERRCODE_SAMPLER SendCtrlSetCmd2IMEU(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU, IN IMEU_SEND_CMD_TYPE cmdType, IN int nCmdVal);
void 	Sync_CoolPoint2IMEU2(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, UINT nWhichIMEU);

void 	BattCab_Safety_Management(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, IN UINT nWhichIMEU);
void 	CabSafe_StateMachine(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU);

CAB_SAFE_MANAGE_STATE   OnIdle_CabSafeStage(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU);
CAB_SAFE_MANAGE_STATE   OnSecond_CabSafeStage(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU);
CAB_SAFE_MANAGE_STATE   OnThird_CabSafeStage(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichIMEU);

void 	IMEU_InitRoughData_V20(RS485_2_SAMPLER_DATA *pDev_485_2_Rough);
int 	IMEU2_Sample_V20(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough);
int 	IMEU_SendCtrl_V20(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, 
			PARAM_TYPE	emParaType,
			int 		nDevId, 
			int 		nParamId, 
			VAR_VALUE 	*pVarVal, 
			int 		nValSize,
			int			nTimeOut);
int Pack_IMEUCmd_AndGetReply_V20(IN RS485_2_SAMPLER_DATA *pDev_485_2_Rough, IN IMEU2_FRM_INFO_V20* pimeuFrame, BOOL bCtrl, UINT nWhichIMEU);

 
void		kwhMeter_InitRoughData(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough);	
int 		ACMeter_Sample(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough);
int 		kwhMeter_SendCtrl(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, 
			PARAM_TYPE	emParaType, 
			int 		nDevId, 
			int 		nParamId, 
			VAR_VALUE 	*pVarVal, 
			int 		nValSize,
			int			nTimeOut);
void 		GetKWHMeterSN(RS485_2_SAMPLER_DATA *pDev_485_2_Rough, UINT nWhichWh, char* szSrc);

void 		Make_YDN_Lengh(IN UINT nLen, OUT BYTE* pbyOut);
USHORT 		CalcSendCheckSum(BYTE* pSendBuf, USHORT dataLen);
BYTE 		LenCheckSum(UINT wLen);
void		FloatTo8AsciiData(DWORD Input, BYTE *szOutData);
void		HexToFourAsciiData(DWORD Input, BYTE *szOutData);
void		HexToTwoAsciiData(DWORD Input, BYTE *szOutData);
BYTE		Ascii2ToHex(BYTE *pInput);
BYTE		HexToAscii(BYTE Input);
BYTE		AsciiToHex(BYTE Input);
VAR_VALUE 	Ascii8ToVarV(BYTE *byInData);
short 		Ascii4ToShort(BYTE *pInput);

void RefreshSystemRunState(RS485_2_SAMPLER_DATA *pDev_485_2_Rough);
void LED_RS485_Ctl_State(RS485_2_SAMPLER_DATA *pDev_485_2_Rough);
typedef union tagShortVVV
{
	short 	sVal;
	BYTE 	byVal[2];
}SHORT_VAR;

#endif


