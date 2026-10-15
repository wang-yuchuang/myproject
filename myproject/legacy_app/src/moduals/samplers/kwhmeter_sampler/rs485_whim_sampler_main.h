#ifndef __RS485_2_SAMPLER_MAIN_H
#define __RS485_2_SAMPLER_MAIN_H

#include "../sampler_base.h"

#define	RS485_COMM_OK				0
#define	RS485_WRITE_ERR				1
#define	RS485_READ_ERR				2

// //#pragma pack(1)
// typedef enum whMeter_Roughs
// {
// 	//AC
// 	WHMETER_ROUGH_RTkWh_AC = 0,
// 	WHMETER_ROUGH_RTkvar,
// 	WHMETER_ROUGH_RTkvar2,
	
// 	WHMETER_ROUGH_RT_VOLT_A,
// 	WHMETER_ROUGH_RT_VOLT_B,
// 	WHMETER_ROUGH_RT_VOLT_C,
// 	WHMETER_ROUGH_RT_CURR_A,
// 	WHMETER_ROUGH_RT_CURR_B,
// 	WHMETER_ROUGH_RT_CURR_C,

// 	WHMETER_ROUGH_RTACTPower,
// 	WHMETER_ROUGH_ACTPower_PhA,
// 	WHMETER_ROUGH_ACTPower_PhB,
// 	WHMETER_ROUGH_ACTPower_PhC,

// 	WHMETER_ROUGH_RTReActPower,
// 	WHMETER_ROUGH_ReActPower_PhA,
// 	WHMETER_ROUGH_ReActPower_PhB,
// 	WHMETER_ROUGH_ReActPower_PhC,

// 	WHMETER_ROUGH_RT_RVSkWh_AC,
// 	WHMETER_ROUGH_RT_PowerFactor,
// 	WHMETER_ROUGH_PowerFactor_PhA,
// 	WHMETER_ROUGH_PowerFactor_PhB,
// 	WHMETER_ROUGH_PowerFactor_PhC,


// 	WHMETER_ROUGH_RT_Freq,
// 	WHMETER_ROUGH_PMAXDEMAND,

// 	WHMETER_ROUGH_LAST1_POS_SUM,
// 	WHMETER_ROUGH_LAST1_POS_SHARP,
// 	WHMETER_ROUGH_LAST1_POS_PEAK,
// 	WHMETER_ROUGH_LAST1_POS_FLAT,
// 	WHMETER_ROUGH_LAST1_POS_WAVE,

// 	WHMETER_ROUGH_LAST1_NEG_SUM,
// 	WHMETER_ROUGH_LAST1_NEG_SHARP,
// 	WHMETER_ROUGH_LAST1_NEG_PEAK,
// 	WHMETER_ROUGH_LAST1_NEG_FLAT,
// 	WHMETER_ROUGH_LAST1_NEG_WAVE,

// 	WHMETER_ROUGH_LAST2_POS_SUM,
// 	WHMETER_ROUGH_LAST2_POS_SHARP,
// 	WHMETER_ROUGH_LAST2_POS_PEAK,
// 	WHMETER_ROUGH_LAST2_POS_FLAT,
// 	WHMETER_ROUGH_LAST2_POS_WAVE,

// 	WHMETER_ROUGH_LAST2_NEG_SUM,
// 	WHMETER_ROUGH_LAST2_NEG_SHARP,
// 	WHMETER_ROUGH_LAST2_NEG_PEAK,
// 	WHMETER_ROUGH_LAST2_NEG_FLAT,
// 	WHMETER_ROUGH_LAST2_NEG_WAVE,

// 	WHMETER_ROUGH_SUM_RTkWh_AC ,
// 	WHMETER_ROUGH_SHARP_RTkWh_AC,
// 	WHMETER_ROUGH_PEAK_RTkWh_AC,
// 	WHMETER_ROUGH_FLAT_RTkWh_AC,
// 	WHMETER_ROUGH_WAVE_RTkWh_AC,

// 	WHMETER_ROUGH_RT_SUM_RVSkWh_AC,
// 	WHMETER_ROUGH_RT_SHARP_RVSkWh_AC,
// 	WHMETER_ROUGH_RT_PEAK_RVSkWh_AC,
// 	WHMETER_ROUGH_RT_FLAT_RVSkWh_AC,
// 	WHMETER_ROUGH_RT_WAVE_RVSkWh_AC,
	
// 	WHMETER_ROUGH_CURRENTPOSDEMAND,  

// 	WHMETER_ERR_WORD, //Received Msg that is not normal
// 	_MAX_WH_ROUGH
// }WH_ROUGH_SIG;

// 406 + n 406:Cabinet AC 408:Grid 409:Aux1 410:Aux3 411:OffGridLoad
#define  MAX_WH_NUM	 (METER_TYPES_MAX * METER_TYPE_NUM)	//(6)
// 	

typedef enum tagAcrel_MD_TYPE   //Acrel ADL400
{
	ACREL_GET_TYPE_VOLT = 0,
	ACREL_GET_TYPE_CURR,
	ACREL_GET_TYPE_ACTPOWER_AC,		
	ACREL_GET_TYPE_REACTPOWER,  	
	ACREL_GET_TYPE_FACTOR,
	ACREL_GET_TYPE_FREQ,
	ACREL_GET_TYPE_SUM_RTkWh_AC,	//正向有功电能
	ACREL_GET_TYPE_SUM_RVSkWh_AC,	//反向电能
	ACREL_GET_TYPE_TOTALKVAR,		//无功电量  组合无功总电能
	ACREL_GET_TYPE_MAX_POS_DEMAND,	//当前正向有功最大需量
	ACREL_GET_TYPE_CURRENTPOSDEMAND,//当前正向有功需量 	 	
	ACREL_GET_TYPE_SUM_RTkWh_RVSkWh_AC,

	_MAX_ACREL_GET_TYPE
}ACREL_MD_GET_TYPE;

typedef enum tagDtsd_MD_TYPE   //Dtsd1352
{
	// DTSD1352
	DTSD_GET_TYPE_VOLT = 0,
	DTSD_GET_TYPE_CURR,
	DTSD_GET_TYPE_ACTPOWER_AC,		
	DTSD_GET_TYPE_REACTPOWER,  	
	DTSD_GET_TYPE_FACTOR,
	DTSD_GET_TYPE_FREQ,
	DTSD_GET_TYPE_SUM_RTkWh_AC,
	DTSD_GET_TYPE_TOTALKVAR,		//无功电量  组合无功总电能
	DTSD_GET_TYPE_MAX_POS_DEMAND,	//当前正向有功最大需量
	DTSD_GET_TYPE_CURRENTPOSDEMAND,//当前正向有功需量 	 	
	DTSD_GET_TYPE_SUM_RTkWh_RVSkWh_AC,
	DTSD_GET_TYPE_SUM_RVSkWh_AC,	// 当前反向有功总电能

	_MAX_DTSD_GET_TYPE
}DTSD_MD_GET_TYPE;


#pragma pack(1) 
typedef struct tagACRELwhMeterFrame
{
	BYTE		byModbusAddr;
	BYTE		byFunCode;
	BYTE		byStartAddr[2];
	BYTE		byRegisterNum[2];
	USHORT		usChkSum[2];
}ACREL_WH_A_FRAME;
#pragma pack()

typedef struct tagACREL_MD_FrameInfo
{
	ACREL_MD_GET_TYPE	frameType;
	ACREL_WH_A_FRAME	sendFrame;
	UINT				nVarVal_Idx;
	UINT				nSignalNum;
	UINT				nWordLen;
	UINT				nVal_OffSet;
	UINT				nExpectAnswerLen;
}ACREL_MD_FRAME_INFO;

typedef struct tagDTSD_MD_FrameInfo
{
	DTSD_MD_GET_TYPE	frameType;
	ACREL_WH_A_FRAME	sendFrame;
	UINT				nVarVal_Idx;
	UINT				nSignalNum;
	UINT				nWordLen;// not use
	UINT				nVal_OffSet;
	UINT				nExpectAnswerLen;
}DTSD_MD_FRAME_INFO;

typedef enum tagYD_MD_TYPE   //YD60-80
{
	YD_GET_TYPE_QUICKLY_ACTPOWER_AC = 0,
	YD_GET_TYPE_CURR,
	YD_GET_TYPE_VOLT,		
	YD_GET_TYPE_QUICKLY_REACTPOWER,  	
	YD_GET_TYPE_QUICKLY_FACTOR,
	YD_GET_TYPE_FREQ,			
	YD_GET_TYPE_TOTALWH,	
	YD_GET_TYPE_RVSTATALWH,
	_MAX_YD_GET_TYPE
}YD_MD_GET_TYPE;


#pragma pack(1) 
typedef struct tagYDwhMeterFrame
{
	BYTE		byModbusAddr;
	BYTE		byFunCode;
	BYTE		byStartAddr[2];
	BYTE		byRegisterNum[2];
	USHORT		usChkSum[2];
}YD_WH_A_FRAME;
#pragma pack()

typedef struct tagYD_MD_FrameInfo
{
	YD_MD_GET_TYPE		frameType;
	YD_WH_A_FRAME		sendFrame;
	UINT				nVarVal_Idx;
	UINT				nSignalNum;
	UINT				nWordLen;   //the numbers of bytes
	UINT				nVal_OffSet;
	UINT				nExpectAnswerLen;
}YD_MD_FRAME_INFO;
#pragma pack(1) 
typedef struct tagwhMeterFrame
{
	BYTE		byFrameSOI;
	BYTE		byAddrA0_A5[6];
	BYTE		byFrameSOI_2;
	BYTE		byCtrlCode;
	BYTE		byDataLen;
#define _MAX_WH_SEND_DATA	50
#define _MAX_WH_RECEIVE_DATA	200
	//BYTE		byDI0_3[4];//闁跨喐鏋婚幏鐑芥晸閺傘倖瀚归柨鐔告灮閹风兘鈧岸鏁撻弬銈嗗data闁跨喐鏋婚幏锟�/**/
	BYTE		*pbyData;//[_MAX_WH_SEND_DATA - 4];
	BYTE		byChkSum;
	BYTE		byEOI;
}WH_A_FRAME;
#pragma pack()

typedef enum tagWH_FRAME_TYPE
{
	//WH_FRAME_TYPE_SET_ADDR = 0,
	WH_FRAME_TYPE_GET_ADDR = 0,
	WH_FRAME_TYPE_GET_TOTALWH,
	WH_FRAME_TYPE_GET_RVSTATALWH,
	WH_FRAME_TYPE_GET_TOTALWH_AC,
	// 
	//Jimmy: DC reused A phase volt and curr
	WH_FRAME_TYPE_GET_VOLT_PHASE_A,
	WH_FRAME_TYPE_GET_VOLT_PHASE_B,
	WH_FRAME_TYPE_GET_VOLT_PHASE_C,
	WH_FRAME_TYPE_GET_CURR_PHASE_A,
	WH_FRAME_TYPE_GET_CURR_PHASE_B,
	WH_FRAME_TYPE_GET_CURR_PHASE_C,

	WH_FRAME_TYPE_GET_TOTALKVAR_AC, //ReactivePower#1
	WH_FRAME_TYPE_GET_TOTALKVAR2_AC, //ReactivePower#2

	WH_FRAME_TYPE_GET_ACTPOWER_AC,  
	WH_FRAME_TYPE_GET_REACTPOWER_AC,
	WH_FRAME_TYPE_GET_POWERFACTOR,
	WH_FRAME_TYPE_GET_FREQ,
	WH_FRAME_TYPE_GET_PMAXDEMAND,

	//---John Added 20221129 For Remote Get SPFW kWh---//
	WH_FRAME_TYPE_GET_MULTI_LAST1_POS_KWH,
	WH_FRAME_TYPE_GET_MULTI_LAST1_NEG_KWH,
	WH_FRAME_TYPE_GET_MULTI_LAST2_POS_KWH,
	WH_FRAME_TYPE_GET_MULTI_LAST2_NEG_KWH,
	WH_FRAME_TYPE_GET_MULTI_TOTALWH,
	WH_FRAME_TYPE_GET_MULTI_RVSTATALWH,

	WH_FRAME_TYPE_GET_CURRENTPOSDEMAND,

	_MAX_WH_FRAME_TYPE
}WH_FARAME_TYPE;

#define MAX_MULTI_FRAMEDATA	5
typedef struct tagAWHFrameInfo
{
	WH_FARAME_TYPE	frameType;
	WH_A_FRAME		sendFrame;
	UINT			nFrameLen; //this is the real len used by app
	int				nExpectAnswerLen;
	int				nDataLen;
	int				nPointNum;
	float			fGetData;  
	float			fGet_MultiData[MAX_MULTI_FRAMEDATA+1];  
}WH_FRAME_INFO;


enum tagRS4852_CtrlCMD
{
	//Set
	RS4852_CMD_SET_SHUNT_A = 0,
	RS4852_CMD_SET_CURR_ADJ_a,
	RS4852_CMD_SET_CURR_ADJ_b,
	RS4852_CMD_SET_VOLT_ADJ_a,
	RS4852_CMD_SET_VOLT_ADJ_b,
	RS4852_CMD_SET_INSUL_DETECT_Ohm,
	RS4852_CMD_SET_INSUL_MODE,
	//RS4852_CMD_SET_BATT_REVERSE_ENABLE,
	//Ctrl
	RS4852_CMD_CTRL_INSUL_DETECT_ON_OFF,
	//IMEU set
	RS4852_CMD_SET_IMEU_OVERTEMPPOINT = 441,
	RS4852_CMD_CTRL_IMEU_FAN_ONOFF = 442,
	//set and ctrl extended DO
	RS4852_CMD_SET_CTRL_EXT_DO
};

typedef enum tagACMeterProtocolType
{
	ACMETER_PROTOCOL_GBDLT645 = 0,
	ACMETER_PROTOCOL_MODBUS_EM737,
	ACMETER_PROTOCOL_MODBUS_RM_PRIVATE,
	ACMETER_PROTOCOL_MODBUS_RTU_YD,//YDS60-80
	ACMETER_PROTOCOL_MODBUS_RTU_ACREL_ADL400,
	ACMETER_PROTOCOL_MODBUS_RTU_DTSD1352,

}ACMETER_PROTOTP;



typedef enum tagMeterValueSelfCheck
{
	METERVALUE_OK = 0,
	METERVALUE_VOLT_UNBANLANCE,  	//单相电压低于50V
	METERVALUE_ACTPower_UNBANLANCE,	//三相有功功率不平衡
 	METERVALUE_PWR_ABNML,			//reserve

}METERVALUE_CHECKCODE;

#define EM737_GET_INSTANT_VALUES	1
#define EM737_GET_ENERGY_VALUES	2
#define RM_PRIVATE_GET_INSTANT_VALUES	3

typedef struct tagMeterData
{
	METER_TYPE					emMeterType; //get from setting
	BYTE						byReadMeterA0A6[6]; //address from setting
	float						fACMeter_PowerFactor; //from setting
	ACMETER_PROTOTP				emACMeter_ProtocolType;
	char						szBaudrateSett[32];
	BYTE						byModbusAddress; //modbus address
	PHY_INTER_TYPE				bPhysicalInterType;

	VAR_VALUE					varWHMeterRough[_MAX_WH_ROUGH];
	UINT						nInterrupt_Times;
	UINT						nSampleTimes;
	BOOL						bIsCommFail;
	//UINT 						nValueAbnmlTimes;
	UINT 						nUnderVoltTimes;
	UINT 						nPhaseUnbalanceTimes;
	METERVALUE_CHECKCODE		emSelfCheckCode;
	float						fACMeter_VoltPTFactor; //from setting
	int 						nLastYearToSaveDB;   //add by LPL 240802
	int							nLastMonthToSaveDB;
	int 						nLastDayToSaveDB;
}METER_DATA;

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

typedef enum tagIMIU_FRAME_TYPE
{
	IMEU_FRAME_TYPE_GET_7041 = 0,
	IMEU_FRAME_TYPE_SET_7048,
	_MAX_IMEU_FRAME_TYPE
}IMEU_FARAME_TYPE;

enum IMEU_Roughs
{
 	IMEU_ROUGH_TEMPARATURE = 0,
	IMIU_ROUGH_OVERTEMP_POINT,
	IMIU_ROUGH_DI1,
	IMIU_ROUGH_DI2,
	IMIU_ROUGH_DI3,
	IMIU_ROUGH_DI4,
	IMIU_ROUGH_DI5,
	IMIU_ROUGH_DI6,
	_MAX_IMEU_ROUGH
};

typedef struct tagAIMEUFrameInfo
{
	IMEU_FARAME_TYPE	frameType;
	YDN_FRAME			sendFrame;
	UINT				nDataLen;
	UINT				nFrameLen; //this is the real len used by app
	int					nExpectAnswerLen;
//	int				nExpectValNum; 
//#define MAX_IMIU_FRAME_VAL	32
//	VAR_VALUE		varVal[MAX_IMIU_FRAME_VAL];
}IMEU_FRAME_INFO;

typedef enum tagLEDSTATE
{
	START1_STATE = -1,
	GREEN = 0,
	RED,
	BLUE,
	YELLOW,
	WHITE
}LED_STATE;

// 用于电表故障报文
typedef struct {
    BYTE buffer[200][50];
	UINT dataLen[200];
    UINT head;
    UINT tail;
    UINT count;
} MeterRecCirBuffer;
typedef struct {
    BYTE buffer[200][50];
	UINT dataLen[200];
    UINT count;
    BOOL isActive;
    BOOL bTrigger;
} MeterRecBuffer;


struct stRS485_ROUGH
{	
	HANDLE			hSampler; //pointer to sampler
	HANDLE			hPort; //opened port handle

	int				nSamplerId; //copy from sampler

	BYTE			abySendBuf[_MAX_PHY_INTER_TYPE][RS485_SEND_BUF_LEN];
	BYTE			abyRcvBuf[_MAX_PHY_INTER_TYPE][RS485_RECV_BUF_LEN];
	int				nReadBuffBytes[_MAX_PHY_INTER_TYPE]; //Bytes that have been readout	
	
	//BOOL			bNeedReconfig_whMeter;
	//BOOL			bNeedReconfig_ExtraACMeter;

	METER_DATA		meterData[MAX_WH_NUM];

	VAR_VALUE		varIMEURough[_MAX_IMEU_ROUGH];

	IMEU_FRAME_INFO	frm_imeuFrames[_MAX_IMEU_FRAME_TYPE];
	WH_FRAME_INFO	frm_whFrames[_MAX_WH_FRAME_TYPE];

	int				nSampleCount;
	BOOL			bHasSampledWhOK; //has already sampled wh
	BOOL			bNeedSampleIMEU;// if need sample IMEU

	int				nAllCommFailCounter;
	BOOL 			bLast_AllMeterCommOK;
	// LJH241007 AntiPhase
	BOOL            needAntiReflux_Phase;   // 单相防逆流使能
	BOOL            needAntiOverLoad_Phase; // 单相防过载使能
	
	A_BLOB_METER_VALUES	blobMeterVal[MAX_WH_NUM];

	MeterRecCirBuffer	meterRecCirBuffer;
	MeterRecBuffer		meterFaultRecBuffer;
    double              tm_fLastRecMeterFaultLog;

	YD_MD_FRAME_INFO	frm_YD_MD_whFrames[_MAX_YD_GET_TYPE];//YDS60-80
	ACREL_MD_FRAME_INFO	frm_ACREL_MD_whFrames[_MAX_PHY_INTER_TYPE][_MAX_ACREL_GET_TYPE];//ACREL ADL400
	DTSD_MD_FRAME_INFO	frm_DTSD_MD_whFrames[_MAX_PHY_INTER_TYPE][_MAX_DTSD_GET_TYPE];//DTSD1352
	int             nSemCCUWith485Meter;
	BOOL 			bEnableQuickResChannel;
	BOOL			bUartServerCommOk;
	UINT 			nMeterInterrAlarmNums;

	GRID_APPLY_SCENERY					tagGridApplyScenery;

};	
typedef struct stRS485_ROUGH		RS485_SAMPLER_DATA;

//extern RS485_SAMPLER_DATA	g_RS485_1SamplerData;
extern COMMPORT * 	s_pRS485_2_Comm;
extern HANDLE      	hRS485_2_Port;
extern COMMPORT * 	s_pUartServer_Comm;
extern HANDLE      hUartServer_Port;	

#define  WH_WAKEUP_FRAME	(0xFEFEFEFE)

int	whMeter_Sample(RS485_SAMPLER_DATA *pWhim_Rough, IN int nWhichWh);
void whMeter_InitRoughData(RS485_SAMPLER_DATA *pWhim_Rough);
BOOL JudgeAndSaveToDB(RS485_SAMPLER_DATA *pWhim_Rough,int nWhichWh);

void IMEU_InitRoughData(RS485_SAMPLER_DATA *pWhim_Rough);
int IMEU_Sample(IN RS485_SAMPLER_DATA* pWhim_Rough);
int IMEU_SendCtrl(IN RS485_SAMPLER_DATA* pWhim_Rough, int nParamId, VAR_VALUE varV);


void GetMeterSN(RS485_SAMPLER_DATA *pWhim_Rough, IN int nWhichWh, char* szSrc);

#define MAX_RS485_2_LOCK_WAIT	100

//#pragma pack()

#endif


