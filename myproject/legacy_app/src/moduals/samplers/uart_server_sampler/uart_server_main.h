#ifndef __UART_SERVER_MAIN_H
#define __UART_SERVER_MAIN_H

#include "../sampler_base.h"

#define	MDB_SEND_BUF_LEN				16
#define	MDB_RECV_BUF_LEN				256
#define MDBRTU_SEND_BUF_LEN				8

#define MDB_ADDR_TEMPHUMI               0x01
#define MDB_ADDR_TEMPHUMI_HY            0x02
#define MDB_ADDR_KWMETER_BROADCAST      0x00
#define MDB_ADDR_KWMETER                0x01

#define MDB_FUN_CODE_03		            0x03
#define MDB_FUN_CODE_06		            0x06

#define  MAX_WH_NUM	    2   		//645:KWH_METER 、MutilFuction Meter

#define MAX_UART_SERVER_TCP_LOCK_WAIT	100

#define	TCP_COMM_OK					0
#define	TCP_WRITE_ERR				1
#define	TCP_READ_ERR				2

#define WAIT_INFINITE	((unsigned long)-1)

enum	tagMETER_645_TYPE
{
	METER_645_TYPE_DSZ71 = 0,
	METER_645_TYPE_AMC96,

	METER_645_TYPE_MAX,
};
enum	tagTEMPHUMIL_TYPE
{
	TEMPHUMIL_TYPE_NOTINSTALL = 0,
	TEMPHUMIL_TYPE_OMRON_E5CC,
	TEMPHUMIL_TYPE_HY_BWD3K130C,

	TEMPHUMIL_TYPE_MAX,
};

enum	tagSERIALSERVER_TYPE
{
	SERIALSERVER_TYPE_NOTINSTALL = 0,
	SERIALSERVER_TYPE_ALARMLIGHT,
	SERIALSERVER_TYPE_TEMPHUMIL,
	SERIALSERVER_TYPE_METER_DSZ71,
	SERIALSERVER_TYPE_METER_AMC96,

	SERIALSERVER_TYPE_MAX,
};//edit by kuangyue 20251029
#define  WH_WAKEUP_FRAME	(0xFEFEFEFE)

#define  MAX_UART_SERVER_TCP_TRY_TIMES	10

enum	tagMDB_ERROR_CODE
{
	MDB_ERROR_OK = 0,
	MDB_ERROR_ADDR,
	MDB_ERROR_FUN_CODE,
	MDB_ERROR_RESPOND_REG_ADDR,
	MDB_ERROR_RESPOND_DATA_LEN,
	MDB_ERROR_RESPOND_DATA_VALUE,
	MDB_ERROR_CRC,

	MDB_ERROR_FAIL,
	MDB_ERROR_NO_MEMORY,
	MDB_ERROR_OTHER,

	MDB_ERROR_MAX,
};

typedef struct tagTempAndHumi
{
	BOOL                    bInstallType;
    float                   fTemperature;
	int 					CommFailCounter;
	BOOL 					CommFailflag;
}TempAndHumiRead;

#define TCP_CLIENT_NUM_MAX       5   //For the TCP server reconnection mechanism, it is 1 more than the actual number

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

typedef struct tagwhMeterFrame
{
	BYTE		byFrameSOI;
	BYTE		byAddrA0_A5[6];
	BYTE		byFrameSOI_2;
	BYTE		byCtrlCode;
	BYTE		byDataLen;
#define _MAX_WH_SEND_DATA	50
#define _MAX_WH_RECEIVE_DATA	200
	BYTE		*pbyData;//[_MAX_WH_SEND_DATA - 4];
	BYTE		byChkSum;
	BYTE		byEOI;
}WH_A_FRAME;

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

typedef enum tagACMeterProtocolType
{
	ACMETER_PROTOCOL_GBDLT645 = 0,
	ACMETER_PROTOCOL_MODBUS_EM737,
	ACMETER_PROTOCOL_MODBUS_RM_PRIVATE,
	ACMETER_PROTOCOL_MODBUS_RTU_YD,

}ACMETER_PROTOTP;

typedef enum tagMeterValueSelfCheck
{
	METERVALUE_OK = 0,
	METERVALUE_VOLT_UNBANLANCE,  	//单相电压低于50V
	METERVALUE_ACTPower_UNBANLANCE,	//三相有功功率不平衡
 	METERVALUE_PWR_ABNML,

}METERVALUE_CHECKCODE;

typedef struct tagMeterData
{
	METER_TYPE					emMeterType; //get from setting
	BYTE						byReadMeterA0A6[6]; //address from setting
	float						fACMeter_PowerFactor; //from setting
	ACMETER_PROTOTP				emACMeter_ProtocolType;
	//char							szBaudrateSett[32];
	//BYTE							byModbusAddress; //modbus address

	VAR_VALUE					varWHMeterRough[_MAX_WH_ROUGH];
	UINT						nInterrupt_Times;
	UINT						nSampleTimes;
	BOOL						bIsCommFail;
	//UINT 						nValueAbnmlTimes;
	UINT 						nUnderVoltTimes;
	UINT 						nPhaseUnbalanceTimes;
	METERVALUE_CHECKCODE		emSelfCheckCode;
	int 						nLastYearToSaveDB;   //add by LPL 240802
	int							nLastMonthToSaveDB;
	int 						nLastDayToSaveDB;

}METER_DATA;


typedef struct stUartServerRoughData
{
	HANDLE		hSampler; //pointer to sampler
	HANDLE	    hSamplerLock;  
    HANDLE		hDataComm;
	
    DATA_PROCESSOR*					pDPR;
	time_t					tmSysBoot; //system boot time

	BYTE				    sz_ByMdbSendBuffer[MDB_SEND_BUF_LEN];
	BYTE				    sz_ByMdbRecvBuffer[MDB_RECV_BUF_LEN];
	TempAndHumiRead         stTempAndHumiRead; 

	BYTE					aby645SendBuf[TCP_BUFF_LEN];
	BYTE					aby645RcvBuf[TCP_BUFF_LEN];  
	int						nReadBuffBytes; //Bytes that have been readout	
    WH_FRAME_INFO	        frm_whFrames[_MAX_WH_FRAME_TYPE];//645 Frames
    METER_DATA				meterData[MAX_WH_NUM];   //0:KWH_Metrt 1:MutilFuction Meter）

    BOOL                    bUartServerInstall;

	int						nAllCommFailCounter;
	//int  					bMeterCommOK[MAX_WH_NUM];

}ST_UART_SERVER_ROUGHDATA;

typedef struct tagTcpClientInfo
{
	HANDLE                  hThread;
	HANDLE                  hClient;
	HANDLE                  hMutexLock; 
	BOOL                    bIsRegistered; // true : has been create pthread, false : not
	UINT                    nIndex;  // This value identifies the location stored in the client array
	
#define CLITNT_COMM_STAT_WAITCONNECT      0
#define CLITNT_COMM_STAT_NORMAL           1
#define CLITNT_COMM_STAT_FAILED           2
    int                     emRunCommStatus;
	UINT                    nCommFailTimes;
	
	double                  tmLastReceivedLSYS; // used for judge if comm-fail

	BOOL                    bNeedExitThread;
}UART_SERVER_TCP_CLIENT_INFO;

typedef struct
{
    //pointer to uart-server-Rough
    ST_UART_SERVER_ROUGHDATA		*pUART;

	// client
	UINT					        nClientConnTotal;
	UART_SERVER_TCP_CLIENT_INFO		stTcpClientInfo[TCP_CLIENT_NUM_MAX];
    time_t					        tmLastCheckCliConnectOrReopenPort;
	char                    		szClientIP[4][20];    //Tcp client List 

	BOOL	                		bUartServerRunning_A;
	BOOL	                		bUartServerRunning_B;
	BOOL	                		bUartServerRunning_C;
	BOOL	                		bUartServerRunning_D;
}UART_SERVER_TCP_RUNROUGH;//edit by kuangyue 20251029
//public

typedef struct tagSampValMapper
{
	int		iRawValIdx; //Sample raw float data, if not a float , think it as float
	int		iSigID; //only sample signal, so no need to assign sig type
}SAMPVAL_MAP;


//modbus
int Init_TempHumiData(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough);
BOOL GetFireAlarmStatus(ST_UART_SERVER_ROUGHDATA* pSTServer_Rough);
int AlarmLightMain(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,UART_SERVER_TCP_CLIENT_INFO *pClientInfo);
int ModbusSampMain(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,UART_SERVER_TCP_CLIENT_INFO *pClientInfo);
void PushModbusValue(IN ST_UART_SERVER_ROUGHDATA *pSTServer_Rough);

//645
int whMeter_InitRoughData(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough);
void UartServerGetMeterSN(ST_UART_SERVER_ROUGHDATA *pWhim_Rough, IN int nWhichWh, char* szSrc);
int whMeter645_Sample(ST_UART_SERVER_ROUGHDATA *pUart,UART_SERVER_TCP_CLIENT_INFO *pClientInfo,int TypeNum);
void RS485_StuffChanl(ST_UART_SERVER_ROUGHDATA *pWhim_Rough,int nWhichWh);
BOOL UartJudgeAndSaveToDB(ST_UART_SERVER_ROUGHDATA *pWhim_Rough,int nWhichWh);

#endif