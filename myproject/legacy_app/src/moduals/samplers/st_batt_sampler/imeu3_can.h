#ifndef __IMEU3_CAN_H__
#define __IMEU3_CAN_H__

#include "../sampler_base.h"


#define MAX_IMEU3NTC_NUM 7
#define MAX_IMEU3PT1000_NUM 5
#define MAX_PWMChannel_NUM 6
#define MAX_PressureSen_NUM 6
#define MAX_TemperatureHumiditySen_NUM 4
#define MAX_IMEU_NUM 4

typedef enum tagIMEU3_CAN_CMD_ID
{ 
	//S2C: S,Server-IMEU3  C:Client-Monitor
	_IMEU3_CAN_R_CMD_S2C_TEMP_MSG1 = 0,
    _IMEU3_CAN_R_CMD_S2C_TEMP_MSG2,
    _IMEU3_CAN_R_CMD_S2C_DIDO_MSG1,
    _IMEU3_CAN_R_CMD_S2C_DIPWM_MSG1,
    _IMEU3_CAN_R_CMD_S2C_DIPWM_MSG2,
    _IMEU3_CAN_R_CMD_S2C_PRESEN_MSG1,
    _IMEU3_CAN_R_CMD_S2C_PRESEN_MSG2,
    _IMEU3_CAN_R_CMD_S2C_HEART_MSG,
    _IMEU3_CAN_R_CMD_S2C_TEMHUM_MSG,
    _IMEU3_CAN_R_CMD_S2C_COMBUSTIBLE_MSG,
    _IMEU3_CAN_R_CMD_S2C_ALARM_MSG,
    _IMEU3_CAN_R_CMD_S2C_CONFIG_MSG,
    _IMEU3_CAN_R_CMD_S2C_DE_MSG,
	_CAN_R_CMD_C2S_IMEU3UPGRADE,    //电源柜 STS柜升级CAN报文处理

	_MAX_IMEU3_CAN_CMD

}IMEU3_CAN_CMD_ID;

typedef struct tagIMEU3RunData
{
	BOOL						bFoundReply;

}IMEU3RunData;


typedef struct tagIMEU3_CAN_FRAME
{
	BYTE 			byRawData[CAN_FRAME_LEN];
	
	IMEU3_CAN_CMD_ID    emMsgType;
	UINT 			    uiFrmID; 	//Send CAN ID
	UINT 			    uiDataLen; 	//Send DLC
 	BYTE  			    byFuncCode;
	BYTE  			    bySubIndex;
	BYTE 			    byDestAddr;
    BYTE    		    bySourceAddr; 
	BYTE				bySourceIdx;
	BYTE   	 		    byData[CAN_DATA_AREA_LEN+1];      /* Data Area */
					

	BOOL    		    bIsCtrlCmd;     /* if true, need send urgently */

}IMEU3_CAN_FRAME;

typedef struct tagIMEU3CANDataInfo
{
	//must always put first!!!
	BLOB_TYPE	emBlobType;

	//S2C_TEMP_MSG1	   0x14CByyxx
	float	fNTC[MAX_IMEU3NTC_NUM];   //NTC or PT1000 Temperature

	//S2C_TEMP_MSG2	   0x14CCyyxx
    float	fPT1000[MAX_IMEU3PT1000_NUM];
	float	fTemperatureSen;		  // Temperature sensor temperature value
	float	fHumiditySen;		     // Humidity sensor humidity level
	UINT	uFloodFreq;		         // Frequency value of water immersion sensor signal
	 
	//S2C_MSG3       0x14CDyyxx
	UINT    uDIStatus;            //22 channel DI status
	UINT    uDOStatus;            //24 channel DO status
    UINT    uX1Connect;           //Connection status of 4-channel X1 board

	//S2C_DIPWM_MSG1  0x14CEyyxx  || S2C_DIPWM_MSG2  0x14CFyyxx
	UINT    uPWMFreq[MAX_PWMChannel_NUM];        //Channel PWM frequency
	
	//S2C_PRESEN_MSG1  0x14D0yyxx  || S2C_PRESEN_MSG2  0x14D1yyxx
	UINT    uPressureSen[MAX_PressureSen_NUM];                //Channel pressure sensor voltage

	//S2C_HEART_MSG  0x14D2yyxx
	UINT    uSoftwareVer;                   //Software version
	UINT    uSystemRunTime;                 //System running time

	//S2C_TEMHUM_MSG   0x14D3yyxx
	float  fTemperatureValue[MAX_TemperatureHumiditySen_NUM];         //Temperature and humidity sensor temperature value
	float  fhumidityValue[MAX_TemperatureHumiditySen_NUM];             //Temperature and humidity sensor humidity value   

	//S2C_COMBUSTIBLE_MSG   0x14D4yyxx
	float	fGasConcent1H2;			//H2 gas
	float	fGasConcent2C0;			//CO gas
	float	fGasConcent3Other;		//other gas
	float	fGasConcent4Other;		//other gas

	//S2C_ALARM_MSG   0x14D5yyxx
	BYTE    bySingleGasAlarm;                  //Communication interruption of combustible gas sensor
    BYTE    byTemperatureHumiAlarm;         //Communication interruption of temperature and humidity sensor
	BYTE 	byMultiGasAlarm;               //Multi in one gas sensor communication interruption
	BYTE 	byDehumidifierAlarm;         //Communication interruption of dehumidifier
	UINT	usSenShortCircuitAlarm;	     //Short circuit fault of 1-12 temperature and humidity sensors
	UINT	usSenOpenCircuitAlarm;       //1-12 temperature and humidity sensor open circuit fault

    //S2C_CONFIG_MSG  0x14D6yyxx
    BYTE  bySystemRunTime;                //Temperature parameter configuration
    BYTE  byConfigDI;                  //DI channel parameter configuration
    BYTE  byConfigPWM;                 //PWM parameter configuration
    BYTE  byConfigPressure;                  //Pressure sensor configuration
    BYTE  byConfigGasSen;                   //Combustible gas sensor configuration
    BYTE  byConfig485Sen;                 //Configuration of 485 temperature and humidity sensors
    BYTE  byConfigDehumidifier;                       //Dehumidifier configuration

    //S2C_DE_MSG  0x14D7yyxx
    float fHumidity;
    float fDehumidifierStart;            //Dehumidifier start value
    float fDehumidifierStop;            //Dehumidifier stop value
    BYTE  byTemperatureLoad;	// 0 stop, 1 execute
    BYTE  byHumidityLoad;		// 0 stop, 1 execute
    BYTE  byDehumidifierRunmode;		// 0 manual, 1 auto
    BYTE  byDehumidifierFaultState;		// 0 normal, 1 fault
    BYTE  byDehumidifierAddr;

	BYTE 	byIMEU3CommFail;

}IMEU3_CAN_DataInfo;


typedef struct tagIMEU3CANRoughData
{
	HANDLE						hSampler; //pointer to sampler
	int							nSamplerId; //copy from sampler
	HANDLE 						hSamplerLock;

	int                         nInterrupt_Times;
	BOOL                        bFoundReply;
	BOOL						bIsCommFail;
	
	IMEU3_CAN_DataInfo	        stIMEU3DataInfo;

	UINT						nPT1000Num;
	
	//ESS_DEVICE_TYPE 				emDeviceType; // 区分不同设备的EU3（液冷一体，液冷一体+STS，液冷备电柜等） 

}IMEU3_CAN_ROUGH_DATA;


typedef BOOL (*CAN_IMEU3_SAMPLE_PROC)(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrmReply);

typedef struct tagIMEU3Pro
{
	IMEU3_CAN_CMD_ID emMsgType;
	int  			nFuncCode;
	int  			nSubIndex;
	BOOL			bFoundReply;
	CAN_IMEU3_SAMPLE_PROC pfnCmdHandler;

}TAG_IMEU3_PRO;



void initIMEU3CAN(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum);
BOOL judgeAndParseIMEU3MsgID(IMEU3_CAN_FRAME* pFrameReply);
BOOL judgeAndParseOtherIMEU3MsgID(IMEU3_CAN_FRAME* pFrameReply); //解析电源仓can数据报文
void processIMEU3CANData(IMEU3_CAN_ROUGH_DATA *pIMEU3CANRoughData, IMEU3_CAN_FRAME* pFrameReply);

void IMEU3ComStatus(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum); //电池仓 通信状态
void IMEU3PowerComStatus(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum);  //电源仓通信状态



BOOL Parse_S2C_TEMP_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_TEMP_MSG2(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_DIDO_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_DIPWM_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_DIPWM_MSG2(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_PRESEN_MSG1(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_PRESEN_MSG2(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_HEART_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_TEMHUM_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_COMBUSTIBLE_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_ALARM_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_CONFIG_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_S2C_DE_MSG(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply);
BOOL Parse_C2S_IMEU3Upgrade(IMEU3_CAN_ROUGH_DATA* pIMEU3CANRoughData, IMEU3_CAN_FRAME *pFrameReply); 


void pushBattCabinetIMEU3CANData(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum);
void pushBackupBattIMEU3Data(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum);


void pushLiquidCoolBackupPowerCabinetIMEU3(IMEU3_CAN_ROUGH_DATA* pPowerCabinetIMEU3);
void pushLiquidCoolPowerCabinetIMEU3(IMEU3_CAN_ROUGH_DATA* pPowerCabinetIMEU3);
void push2524IMEU3CanData(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], int iNum);
void pushSTSCabinetIMEU3CanData(IMEU3_CAN_ROUGH_DATA* pSTSCabinetIMEU3);

void pushIMEU3toCloudPlatform(IMEU3_CAN_ROUGH_DATA IMEU3CANRoughData[], IMEU3_CAN_ROUGH_DATA PowerEU3_OtherEU3_Data[], ESS_DEVICE_TYPE emEssType, A_BLOB_BATTCAB_IMEU2_V20 blobIMEU2_V20[]);











#endif // !__IMEU3_CAN_H__