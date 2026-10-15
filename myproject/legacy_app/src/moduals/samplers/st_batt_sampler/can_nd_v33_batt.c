#include "st_batt_sampler.h"

//#define TRACE	1 ? (void)0 : (void)printf //printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

static BYTE AsciiToHex(BYTE Input)
{
    if(Input>='0'&& Input<='9')
    {
         Input -= '0';
    }
    else if(Input>='A'&&Input<='F')
    {
         Input -= 'A';
         Input += 10;
    }
    else if(Input>='a'&& Input<='f')
    {
         Input -= 'a';
         Input += 10;
    } 
    return Input;
}
 
static BYTE	Ascii2ToHex(BYTE *pInput)
{
	//TRACE("pInput[0]=[%02X]pInput[1]=%02X\n", pInput[0], pInput[1]);

	return	((AsciiToHex(pInput[0])) << 4) + AsciiToHex(pInput[1]);
}

//Crc_ComputeCRC8- 按SAE J1850 方式计算CRC值
static uint8_t Crc_ComputeCRC8(const uint16_t Msg_Id, const uint8_t* Crc_DataPtr, uint16_t Crc_Length)
{
// SAE J1850 crc lookup table 
static const uint8_t Crc8Table[256] =
{
0x00u, 0x1du, 0x3au, 0x27u, 0x74u, 0x69u, 0x4eu, 0x53u, 0xe8u, 0xf5u, 0xd2u, 0xcfu, 0x9cu, 0x81u, 0xa6u, 0xbbu,
0xcdu, 0xd0u, 0xf7u, 0xeau, 0xb9u, 0xa4u, 0x83u, 0x9eu, 0x25u, 0x38u, 0x1fu, 0x02u, 0x51u, 0x4cu, 0x6bu, 0x76u,
0x87u, 0x9au, 0xbdu, 0xa0u, 0xf3u, 0xeeu, 0xc9u, 0xd4u, 0x6fu, 0x72u, 0x55u, 0x48u, 0x1bu, 0x06u, 0x21u, 0x3cu,
0x4au, 0x57u, 0x70u, 0x6du, 0x3eu, 0x23u, 0x04u, 0x19u, 0xa2u, 0xbfu, 0x98u, 0x85u, 0xd6u, 0xcbu, 0xecu, 0xf1u,
0x13u, 0x0eu, 0x29u, 0x34u, 0x67u, 0x7au, 0x5du, 0x40u, 0xfbu, 0xe6u, 0xc1u, 0xdcu, 0x8fu, 0x92u, 0xb5u, 0xa8u,
0xdeu, 0xc3u, 0xe4u, 0xf9u, 0xaau, 0xb7u, 0x90u, 0x8du, 0x36u, 0x2bu, 0x0cu, 0x11u, 0x42u, 0x5fu, 0x78u, 0x65u,
0x94u, 0x89u, 0xaeu, 0xb3u, 0xe0u, 0xfdu, 0xdau, 0xc7u, 0x7cu, 0x61u, 0x46u, 0x5bu, 0x08u, 0x15u, 0x32u, 0x2fu,
0x59u, 0x44u, 0x63u, 0x7eu, 0x2du, 0x30u, 0x17u, 0x0au, 0xb1u, 0xacu, 0x8bu, 0x96u, 0xc5u, 0xd8u, 0xffu, 0xe2u,
0x26u, 0x3bu, 0x1cu, 0x01u, 0x52u, 0x4fu, 0x68u, 0x75u, 0xceu, 0xd3u, 0xf4u, 0xe9u, 0xbau, 0xa7u, 0x80u, 0x9du,
0xebu, 0xf6u, 0xd1u, 0xccu, 0x9fu, 0x82u, 0xa5u, 0xb8u, 0x03u, 0x1eu, 0x39u, 0x24u, 0x77u, 0x6au, 0x4du, 0x50u,
0xa1u, 0xbcu, 0x9bu, 0x86u, 0xd5u, 0xc8u, 0xefu, 0xf2u, 0x49u, 0x54u, 0x73u, 0x6eu, 0x3du, 0x20u, 0x07u, 0x1au,
0x6cu, 0x71u, 0x56u, 0x4bu, 0x18u, 0x05u, 0x22u, 0x3fu, 0x84u, 0x99u, 0xbeu, 0xa3u, 0xf0u, 0xedu, 0xcau, 0xd7u,
0x35u, 0x28u, 0x0fu, 0x12u, 0x41u, 0x5cu, 0x7bu, 0x66u, 0xddu, 0xc0u, 0xe7u, 0xfau, 0xa9u, 0xb4u, 0x93u, 0x8eu,
0xf8u, 0xe5u, 0xc2u, 0xdfu, 0x8cu, 0x91u, 0xb6u, 0xabu, 0x10u, 0x0du, 0x2au, 0x37u, 0x64u, 0x79u, 0x5eu, 0x43u,
0xb2u, 0xafu, 0x88u, 0x95u, 0xc6u, 0xdbu, 0xfcu, 0xe1u, 0x5au, 0x47u, 0x60u, 0x7du, 0x2eu, 0x33u, 0x14u, 0x09u,
0x7fu, 0x62u, 0x45u, 0x58u, 0x0bu, 0x16u, 0x31u, 0x2cu, 0x97u, 0x8au, 0xadu, 0xb0u, 0xe3u, 0xfeu, 0xd9u, 0xc4u
};
uint16_t count; // loop counter for CRC calculation 
uint8_t crc_value; // CRC initial value 
uint8_t XOR_out; // Value to XOR output 
crc_value = 0xffu;
XOR_out = 0xffu;
crc_value = Crc8Table[crc_value ^ (uint8_t)(Msg_Id & 0x00ffu)]; // Calculate CRC of low byte of ApplicationId 
crc_value = Crc8Table[crc_value ^ (uint8_t)(Msg_Id >> 8)]; // Calculate CRC of high byte of ApplicationId 
for (count = 0; count < (uint16_t)Crc_Length; count++) // Calculate CRC of data block 
{
crc_value = Crc8Table[crc_value ^ Crc_DataPtr[count]];
}
crc_value = crc_value ^ XOR_out;
// Calculate the official checksum.
return crc_value;
}

//举例： cp[0] = (uint8_t)(Crc_ComputeCRC8((uint16_t)(id&0xffff),&cp[1],7));


//static BYTE szBMSGrpMapIdx[]={0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A};
//static BYTE szPCSGrpMapIdx[]={0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2};

//2021-11-16 MAx Support 4 Group Battery,if IMTCU Addr is Over 4,will Cause SW Reboot.
static BYTE szBMSGrpMapIdx[]={0x91, 0x92, 0x93, 0x94};
static BYTE szPCSGrpMapIdx[]={0xB9, 0xBA, 0xBB, 0xBC};

static BYTE szLocalBMSGrpMapIdx[]={0x32, 0x33, 0x34, 0x35};

typedef union tagNDBattAlarmByte
{
	BYTE	byFlag;
	struct
	{
		BYTE	Inner_CommFail : 1;
		BYTE	Low_Insulation : 1;
		BYTE	Module_LowTemp : 1;
		BYTE	Module_HighTemp : 1;
        BYTE	Module_LowVolt : 1;
        BYTE	Module_HighVolt : 1;
	}Alarm;

}ND_BATT_ALARM_MAP;


typedef union tagBattFaultByte
{
	USHORT	usFlag;
	struct
	{
 
 	}Fault;

}ND_BATT_FAULT_MAP;

typedef union tagNDBattProtectUShort
{
	USHORT	usFlag;
	struct
	{
		USHORT	usNotCompletePowerON : 1;
	}Protect;
}ND_BATT_PROTECT_MAP;

typedef struct tagNDBattRunData
{
	/*
	0 "High Volt Not Power ON"
	1 "High Volt Power ON"
	2 "High Volt Power ON FALUT"
	3 "High Volt POWER OFF FAULT"
	*/
	/*
	0 "Initial Statu"
	1 "Normal Statu"
	2 "Full Charge Statu"
	3 "Full DisCharge Statu"
	4 "Alarm Statu"
	5 "Fault Statu"
	*/
	BOOL				byBattPowerON; 

	BYTE                byBattRunStatus; 
	float               fPackTotalVolt;
    float               fPackTotalCurr;

    float               fMainControllerTemperature;
    float               fSOC;
  
    //SUMDATA1
	float 				fRemainCharge_Energy;
	float				fRemainDisCharge_Energy;
	BYTE                bySOH;
	
	//SUMDATA2
	float               fMax_CellVolt;
    float               fMin_CellVolt;
	float               fMax_CellTemp;
    float               fMin_CellTemp;
	float 				fVersion;

	//SUMDATA3
	float               fAllow_MaxDisChargeCurr;
	float               fAllow_MaxChargeCurr;
	float               fCharge_Cutoff_Volt;
    float               fDisCharge_Cutoff_Volt;
	
	//S2A_ALARM 
	BYTE				byS2A_Alarm[9];
	BYTE				szS2A_AlarmInfo[36];

 	//S2A_SUMDATA3 
	UINT 				uiBAT_Ucell_Max_CSC;
	UINT				uiBAT_Ucell_Max_CELL;
	UINT				uiBAT_Ucell_Min_CSC;
	UINT				uiBAT_Ucell_Min_CELL;

	float				fBAT_Ucell_Max;
	float				fBAT_Ucell_Min;

	//S2A_SUMDATA4
	float				fBAT_Ucell_Avg;
	int 				nBAT_T_Max;
	int					nBAT_T_Min;
	int 				nBAT_T_Avg;
	int 				nBAT_T_Max_CSC;
	int 				nBAT_T_Min_CSC;

	//---INFY BMS Alarm | Fault | GPIO---//
	BYTE				byInfy_Alarm[9];
	BYTE				szInfy_AlarmInfo[36];

	BYTE				byInfy_Fault[9];
	BYTE				szInfy_FaultInfo[36];

	BYTE				byInfy_GPIO[9];
	BYTE				szInfy_GPIOInfo[36];

	ND_BATT_ALARM_MAP	mapNDAlarm;
	
	ND_BATT_FAULT_MAP	mapNDFault;

	ND_BATT_PROTECT_MAP mapNDProtect;

	STBATT_SYS_FAULT_MAP mapSysFault;

	BOOL                bIsBatt_ChargeForbidden;
    BOOL                bIsBatt_DisChargeForbidden;

	int 				nRemtDownIndex;
	BMS_REPORT_STATE	emRemtReptStat;

 	int					nRemtRunModel;
	int					nRemtRunState;
	int					nRemtVersion; 

    USHORT              usBattModuleQty;
	
	BYTE				byPCS_Cmd;

	BYTE 				byBattAddr;

	double 				t_LastSendC2BStatus;
	
	time_t 				t_LastRecvAbnormalData;


    int                 bFoundReply;
	int			        nInterrupt_Times;
	BOOL		        bIsCommFail;
    BOOL                bBattInfo_Regisited; //if the battGrp info NOT registered
	
}ND_V33_BATT_RUN_DATA;
 
#define MKAE_ND_BATT_FRAME(pFrame, _emFuncCmd, _bySubCmd, _nBMSGrpMapIdx, _bIsCtrlCmd) \
		((pFrame)->emFuncCmd = (_emFuncCmd), \
		(pFrame)->bySubCmd = (_bySubCmd), \
		(pFrame)->nBMSGrpMapIdx = (_nBMSGrpMapIdx), \
		(pFrame)->bIsCtrlCmd = (_bIsCtrlCmd))

typedef BOOL (*CAN_ND_BATT_SAMPLE_PROC)(A_CAN_ND_BATT_FRAME *pFrmReply);
typedef struct tagNDBattPro
{
	ID_CMD_ND_BATT emFuncCmd;
	int  		nSubCmd;

	CAN_ND_BATT_SAMPLE_PROC pfnCmdHandler;

}TAG_ND_BATT_PRO;

#define  INIT_NDBATT_CMD(p, _emFuncCmd, _nSubCmd, _pfnHandler) \
	((p)->emFuncCmd = (_emFuncCmd), \
	(p)->nSubCmd = (_nSubCmd), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)++);

static ND_V33_BATT_RUN_DATA     s_nd_BattRunData[MAX_SINGLE_STBATT_NUM]; //max 6 grp of batt
static TAG_ND_BATT_PRO			s_nd_BattPro[_MAX_ND_CAN_CMD];

static USHORT CAN_StringToUShort(BYTE* strInput) //LSB
{
	USHORT usValue = ((USHORT)(strInput[0]))
		+ (((USHORT)(strInput[1])) << 8);

	return usValue;
}

static BOOL Parse_B2C_Status(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_nd_BattRunData[nBMSGrpMapIdx].bFoundReply = TRUE;
    s_nd_BattRunData[nBMSGrpMapIdx].bBattInfo_Regisited = TRUE;  

	//V3.3
	s_nd_BattRunData[nBMSGrpMapIdx].byBattPowerON = ((*(pFrmReply->byData + 2))) & 0x03; 
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDProtect.Protect.usNotCompletePowerON = (s_nd_BattRunData[nBMSGrpMapIdx].byBattPowerON !=1) ? TRUE : FALSE;
	s_nd_BattRunData[nBMSGrpMapIdx].byBattRunStatus = ((*(pFrmReply->byData + 2)) >> 2) & 0x07; 
 	s_nd_BattRunData[nBMSGrpMapIdx].fSOC = ((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.1;
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.byFlag = (s_nd_BattRunData[nBMSGrpMapIdx].byBattRunStatus == 4) ? 1 : 0;
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDFault.usFlag = (s_nd_BattRunData[nBMSGrpMapIdx].byBattRunStatus == 5) ? 1 : 0;
	//TRACE("nBMSGrpMapIdx=[%d] byBattPowerON=%d byBattRunStatus=%d fSOC=%f usAlarmFlag=%d usFault=%d usNotCompletePowerON=%d\n",\
	//	nBMSGrpMapIdx,
	//	s_nd_BattRunData[nBMSGrpMapIdx].byBattPowerON,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].byBattRunStatus,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].fSOC,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.byFlag,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].mapNDFault.usFlag,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].mapNDProtect.Protect.usNotCompletePowerON);
	
	/*int i;
	for(i =0;i < CAN_DATA_AREA_LEN; i++)
	{
		printf("%02X",pFrmReply->byData[i]);
	}
	printf("\n");
	*/
	return TRUE;
}

static BOOL Parse_B2C_SumData1(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	USHORT usVal;
	usVal = CAN_StringToUShort(pFrmReply->byData +2);
	if(usVal == 0xFFFF)
	{
		s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalVolt = 0;
		s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalCurr = 0;
		return TRUE;
	}
	s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalVolt = ((float)usVal)*0.1;
	s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalCurr = -(((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.1 -2000);

	//ND Batt: Charge Current is Negative, Discharge Current is Positive, so Reverse it;

	//TRACE("nBMSGrpMapIdx=[%d] fPackTotalVolt=%f fPackTotalCurr=%f\n", \
	//	nBMSGrpMapIdx,s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalVolt,s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalCurr);
	/*
	int i;
	for(i =0;i < CAN_DATA_AREA_LEN; i++)
	{
		printf("%02X",pFrmReply->byData[i]);
	}
	printf("\n");
	*/
	return TRUE;
}

static BOOL Parse_B2C_SumData2(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	s_nd_BattRunData[nBMSGrpMapIdx].fRemainCharge_Energy = ((float)CAN_StringToUShort(pFrmReply->byData +2))*0.1;
    s_nd_BattRunData[nBMSGrpMapIdx].fRemainDisCharge_Energy = ((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.1;
	s_nd_BattRunData[nBMSGrpMapIdx].bySOH = *(pFrmReply->byData + 6);

	//TRACEX("nBMSGrpMapIdx=[%d] fRemainCharge_Energy=%f fRemainDisCharge_Energy=%f bySOH=%d\n",\
	//	nBMSGrpMapIdx, s_nd_BattRunData[nBMSGrpMapIdx].fRemainCharge_Energy, s_nd_BattRunData[nBMSGrpMapIdx].fRemainDisCharge_Energy,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].bySOH);
	/*
	int i;
	for(i =0;i < CAN_DATA_AREA_LEN; i++)
	{
		printf("%02X",pFrmReply->byData[i]);
	}
	printf("\n");
	*/
	return TRUE;
}

static BOOL Parse_B2C_SumData3(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	float fMax_CellVolt = ((float)CAN_StringToUShort(pFrmReply->byData +1))*0.001;
    float fMin_CellVolt = ((float)CAN_StringToUShort(pFrmReply->byData +3))*0.001;
	float fMax_CellTemp = ((float)(*(pFrmReply->byData + 5))) - 50;
    float fMin_CellTemp = ((float)(*(pFrmReply->byData + 6))) - 50;
	float fVersion = ((float)(*(pFrmReply->byData + 7)))*0.1;

	if((fMax_CellTemp > 127) ||
		(fMin_CellTemp > 127) ||
		(fMax_CellVolt > 10.0) ||
		(fMin_CellVolt > 10.0))
	{
		if(ABS(_NOW_ - s_nd_BattRunData[nBMSGrpMapIdx].t_LastRecvAbnormalData) > 60)
		{
			s_nd_BattRunData[nBMSGrpMapIdx].t_LastRecvAbnormalData = _NOW_;
			LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_ERROR,
				"B2CSD3#Id[%d]%02X%02X%02X%02X%02X|%02X%02X%02X%02X%02X%02X%02X%02X|%02X%02X%02X\n", 
				nBMSGrpMapIdx, 
				pFrmReply->byRawData[0],pFrmReply->byRawData[1],
				pFrmReply->byRawData[2],pFrmReply->byRawData[3],
				pFrmReply->byRawData[4],pFrmReply->byRawData[5],
				pFrmReply->byRawData[6],pFrmReply->byRawData[7],
				pFrmReply->byRawData[8],pFrmReply->byRawData[9],
				pFrmReply->byRawData[10],pFrmReply->byRawData[11],
				pFrmReply->byRawData[12],pFrmReply->byRawData[13],
				pFrmReply->byRawData[14],pFrmReply->byRawData[15]);

			return FALSE;
		}
	}

	s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellVolt = ((float)CAN_StringToUShort(pFrmReply->byData +1))*0.001;
    s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellVolt = ((float)CAN_StringToUShort(pFrmReply->byData +3))*0.001;
	s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellTemp = ((float)(*(pFrmReply->byData + 5))) - 50;
    s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellTemp = ((float)(*(pFrmReply->byData + 6))) - 50;
	s_nd_BattRunData[nBMSGrpMapIdx].fVersion = ((float)(*(pFrmReply->byData + 7)))*0.1;

	//TRACE("V33 nBMSGrpMapIdx[=%d] fMax_CellVolt=%f fMin_CellVolt=%f fMax_CellTemp=%f fMin_CellTemp=%f fVersion=%f \n",\
	//	nBMSGrpMapIdx, s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellVolt, s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellVolt,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellTemp, s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellTemp, s_nd_BattRunData[nBMSGrpMapIdx].fVersion);
	/*
	int i;
	for(i =0;i < CAN_DATA_AREA_LEN; i++)
	{
		printf("%02X",pFrmReply->byData[i]);
	}
	printf("\n");
	*/
	return TRUE;
}

static BOOL Parse_B2C_Limit(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	s_nd_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr = ((float)CAN_StringToUShort(pFrmReply->byData))*0.1;
	s_nd_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr = ((float)CAN_StringToUShort(pFrmReply->byData + 2))*0.1; 
	s_nd_BattRunData[nBMSGrpMapIdx].fCharge_Cutoff_Volt = ((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.1;
    s_nd_BattRunData[nBMSGrpMapIdx].fDisCharge_Cutoff_Volt = ((float)CAN_StringToUShort(pFrmReply->byData + 6))*0.1;

	//TRACEX("nBMSGrpMapIdx[=%d] fAllow_MaxDisChargeCurr=%f fAllow_MaxChargeCurr =%f fCharge_Cutoff_Volt=%f fDisCharge_Cutoff_Volt=%f\n", \
	//	nBMSGrpMapIdx, s_nd_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr, s_nd_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr,\
	//	s_nd_BattRunData[nBMSGrpMapIdx].fCharge_Cutoff_Volt,s_nd_BattRunData[nBMSGrpMapIdx].fDisCharge_Cutoff_Volt );

	/*
	int i;
	for(i =0;i < CAN_DATA_AREA_LEN; i++)
	{
		printf("%02X",pFrmReply->byData[i]);
	}
	printf("\n");
	*/
	return TRUE;
}

static BOOL Parse_S2A_Alarm(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	memcpy(s_nd_BattRunData[nBMSGrpMapIdx].byS2A_Alarm, pFrmReply->byData, CAN_DATA_AREA_LEN);
	
	int n = 0, nOffset = 0;
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
    {
        nOffset  += sprintf(s_nd_BattRunData[nBMSGrpMapIdx].szS2A_AlarmInfo + nOffset, "%02X|",pFrmReply->byData[n]);
    }
    s_nd_BattRunData[nBMSGrpMapIdx].szS2A_AlarmInfo[nOffset] ='\0';

	return TRUE;
}

static BOOL Parse_S2A_SumData3(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	//S2A_SUMDATA3
	UINT  uiRawData =  (pFrmReply->byData[1]) + (pFrmReply->byData[2] << 8) + (pFrmReply->byData[3]<<16); 

	UINT uiBAT_Ucell_Max_CSC =  (uiRawData) & 0x3F;
	UINT uiBAT_Ucell_Max_CELL = (uiRawData >> 6) & 0x3F; 
	UINT uiBAT_Ucell_Min_CSC =  (uiRawData >> 12) & 0x3F; 
	UINT uiBAT_Ucell_Min_CELL = (uiRawData >> 18) & 0x3F; 

	float fBAT_Ucell_Max = ((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.001;
	float fBAT_Ucell_Min = ((float)CAN_StringToUShort(pFrmReply->byData + 6))*0.001;

	if((uiBAT_Ucell_Max_CSC > 20) ||
		(uiBAT_Ucell_Max_CELL > 20) ||
		(uiBAT_Ucell_Min_CSC > 20) ||
		(uiBAT_Ucell_Min_CELL > 20) ||
		(fBAT_Ucell_Max > 10.0) ||
		(fBAT_Ucell_Min > 10.0))
	{
		if(ABS(_NOW_ - s_nd_BattRunData[nBMSGrpMapIdx].t_LastRecvAbnormalData) > 60)
		{
			s_nd_BattRunData[nBMSGrpMapIdx].t_LastRecvAbnormalData = _NOW_;
			LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_ERROR,
				"SD3#Id[%d]%02X%02X%02X%02X%02X|%02X%02X%02X%02X%02X%02X%02X%02X|%02X%02X%02X\n", 
				nBMSGrpMapIdx, 
				pFrmReply->byRawData[0],pFrmReply->byRawData[1],
				pFrmReply->byRawData[2],pFrmReply->byRawData[3],
				pFrmReply->byRawData[4],pFrmReply->byRawData[5],
				pFrmReply->byRawData[6],pFrmReply->byRawData[7],
				pFrmReply->byRawData[8],pFrmReply->byRawData[9],
				pFrmReply->byRawData[10],pFrmReply->byRawData[11],
				pFrmReply->byRawData[12],pFrmReply->byRawData[13],
				pFrmReply->byRawData[14],pFrmReply->byRawData[15]);

			return FALSE;
		}
	}



	s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Max_CSC =  (uiRawData) & 0x3F;
	s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Max_CELL = (uiRawData >> 6) & 0x3F; 
	s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Min_CSC =  (uiRawData >> 12) & 0x3F; 
	s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Min_CELL = (uiRawData >> 18) & 0x3F; 

	s_nd_BattRunData[nBMSGrpMapIdx].fBAT_Ucell_Max = ((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.001;
	s_nd_BattRunData[nBMSGrpMapIdx].fBAT_Ucell_Min = ((float)CAN_StringToUShort(pFrmReply->byData + 6))*0.001;
	
	//int n;
	//for(n=0;n < 8; n++)
	//{
	//	printf("%02X|", pFrmReply->byData[n]);
	//}
	//printf(" uiRawData=%d\n", uiRawData);
	
	//TRACEX("nGrp[%d] uiBAT_Ucell_Max_CSC=%d uiBAT_Ucell_Max_CELL=%d \
	//	uiBAT_Ucell_Min_CSC=%d uiBAT_Ucell_Min_CELL=%d \
	//	fBAT_Ucell_Max=%f fBAT_Ucell_Min=%f\n", \
	//nBMSGrpMapIdx, \
	//s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Max_CSC, \
	//s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Max_CELL,\
	//s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Min_CSC ,\
	//s_nd_BattRunData[nBMSGrpMapIdx].uiBAT_Ucell_Min_CELL,\
	//s_nd_BattRunData[nBMSGrpMapIdx].fBAT_Ucell_Max,\
	//s_nd_BattRunData[nBMSGrpMapIdx].fBAT_Ucell_Min);

	return TRUE;
}

static BOOL Parse_S2A_SumData4(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	//S2A_SUMDATA4

	float fBAT_Ucell_Avg = ((float)CAN_StringToUShort(pFrmReply->byData + 1))*0.001;
	int nBAT_T_Max = *(pFrmReply->byData + 3) - 50;
	int nBAT_T_Min = *(pFrmReply->byData + 4) - 50;
	int nBAT_T_Avg = *(pFrmReply->byData + 5) - 50;
	UINT nBAT_T_Max_CSC = *(pFrmReply->byData + 6);
	UINT nBAT_T_Min_CSC = *(pFrmReply->byData + 7);
	
	if((fBAT_Ucell_Avg < -10.0) ||
		(nBAT_T_Max < -10.0) ||
		(nBAT_T_Min < -10.0) ||
		(nBAT_T_Avg < -10.0) ||
		(nBAT_T_Max_CSC > 20) ||
		(nBAT_T_Min_CSC > 20))
	{
		if(ABS(_NOW_ - s_nd_BattRunData[nBMSGrpMapIdx].t_LastRecvAbnormalData) > 60)
		{
			s_nd_BattRunData[nBMSGrpMapIdx].t_LastRecvAbnormalData = _NOW_;
			LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_ERROR,
				"SD4#Id[%d]%02X%02X%02X%02X%02X|%02X%02X%02X%02X%02X%02X%02X%02X|%02X%02X%02X\n", 
				nBMSGrpMapIdx, 
				pFrmReply->byRawData[0],pFrmReply->byRawData[1],
				pFrmReply->byRawData[2],pFrmReply->byRawData[3],
				pFrmReply->byRawData[4],pFrmReply->byRawData[5],
				pFrmReply->byRawData[6],pFrmReply->byRawData[7],
				pFrmReply->byRawData[8],pFrmReply->byRawData[9],
				pFrmReply->byRawData[10],pFrmReply->byRawData[11],
				pFrmReply->byRawData[12],pFrmReply->byRawData[13],
				pFrmReply->byRawData[14],pFrmReply->byRawData[15]);

			return FALSE;
		}
	}

	s_nd_BattRunData[nBMSGrpMapIdx].fBAT_Ucell_Avg = ((float)CAN_StringToUShort(pFrmReply->byData + 1))*0.001;
	s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Max = (*(pFrmReply->byData + 3)) - 50;
	s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Min = (*(pFrmReply->byData + 4)) - 50;
	s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Avg = *(pFrmReply->byData + 5) - 50;
	s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Max_CSC = *(pFrmReply->byData + 6);
	s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Min_CSC = *(pFrmReply->byData + 7);

	//TRACE("nGrp[%d] fBAT_Ucell_Avg=%f nBAT_T_Max=%d nBAT_T_Min=%d \
	//nBAT_T_Avg =%d nBAT_T_Max_CSC=%d nBAT_T_Min_CSC=%d\n", \
	//nBMSGrpMapIdx,\
	//s_nd_BattRunData[nBMSGrpMapIdx].fBAT_Ucell_Avg,\
	//s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Max ,\
	//s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Min ,\
	//s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Avg ,\
	//s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Max_CSC,\
	//s_nd_BattRunData[nBMSGrpMapIdx].nBAT_T_Min_CSC);
	return TRUE;
}

static BOOL Parse_B2C_UpgradeStatu(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	//B2C BMS Upgrade Status 
	s_nd_BattRunData[nBMSGrpMapIdx].nRemtDownIndex = ((int)pFrmReply->byData[0] & 0x0ff) | (((int)pFrmReply->byData[1] & 0x000f) << 8);
	s_nd_BattRunData[nBMSGrpMapIdx].emRemtReptStat = (BMS_REPORT_STATE)(pFrmReply->byData[1] >> 4);
	s_nd_BattRunData[nBMSGrpMapIdx].nRemtRunModel = pFrmReply->byData[2];
	s_nd_BattRunData[nBMSGrpMapIdx].nRemtRunState = pFrmReply->byData[3];
	s_nd_BattRunData[nBMSGrpMapIdx].nRemtVersion = ((int)pFrmReply->byData[4] & 0xff) | (((int)pFrmReply->byData[5] & 0x00ff) << 8) | (((int)pFrmReply->byData[6] & 0x00ff) << 16) | (((int)pFrmReply->byData[7] & 0x00ff) << 24);

	TRACE("nRemtDownIndex=[%d] emRemtReptStat=[%d] nRemtRunModel=[%d] nRemtRunState=[%d] nRemVersion=[%d]\n",\
		s_nd_BattRunData[nBMSGrpMapIdx].nRemtDownIndex, \
		s_nd_BattRunData[nBMSGrpMapIdx].emRemtReptStat, \
		s_nd_BattRunData[nBMSGrpMapIdx].nRemtRunModel, \ 
		s_nd_BattRunData[nBMSGrpMapIdx].nRemtRunState, \ 
		s_nd_BattRunData[nBMSGrpMapIdx].nRemtVersion);

	return TRUE;
}

static BOOL Parse_B2C_InfyAlarm(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	memcpy(s_nd_BattRunData[nBMSGrpMapIdx].byInfy_Alarm, pFrmReply->byData, CAN_DATA_AREA_LEN);
	
	int n = 0, nOffset = 0;
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
    {
        nOffset  += sprintf(s_nd_BattRunData[nBMSGrpMapIdx].szInfy_AlarmInfo + nOffset, "%02X|",pFrmReply->byData[n]);
		//printf("%02X|", pFrmReply->byData[n]);
	}
    s_nd_BattRunData[nBMSGrpMapIdx].szInfy_AlarmInfo[nOffset] ='\0';

	TRACE("INFY Alarm: nBMSGrpMapIdx=[%d]nOffset=[%d] szS2A_AlarmInfo=[%s]\n", \
		nBMSGrpMapIdx, nOffset, s_nd_BattRunData[nBMSGrpMapIdx].szS2A_AlarmInfo);

	return TRUE;
}	

static BOOL Parse_B2C_InfyFault(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	memcpy(s_nd_BattRunData[nBMSGrpMapIdx].byInfy_Fault, pFrmReply->byData, CAN_DATA_AREA_LEN);
	
	int n = 0, nOffset = 0;
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
    {
        nOffset  += sprintf(s_nd_BattRunData[nBMSGrpMapIdx].szInfy_FaultInfo + nOffset, "%02X|",pFrmReply->byData[n]);
		TRACE("%02X|", pFrmReply->byData[n]);
    }
    s_nd_BattRunData[nBMSGrpMapIdx].szInfy_FaultInfo[nOffset] ='\0';

	TRACE("INFY Fault: nBMSGrpMapIdx=[%d]nOffset=[%d] szInfy_FaultInfo=[%s]\n", \
		nBMSGrpMapIdx, nOffset, s_nd_BattRunData[nBMSGrpMapIdx].szInfy_FaultInfo);

	return TRUE;
}

static BOOL Parse_B2C_InfyGPIO(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	memcpy(s_nd_BattRunData[nBMSGrpMapIdx].byInfy_GPIO, pFrmReply->byData, CAN_DATA_AREA_LEN);
	
	int n = 0, nOffset = 0;
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
    {
        nOffset  += sprintf(s_nd_BattRunData[nBMSGrpMapIdx].szInfy_GPIOInfo + nOffset, "%02X|",pFrmReply->byData[n]);
		TRACE("%02X|", pFrmReply->byData[n]);
    }
    s_nd_BattRunData[nBMSGrpMapIdx].szInfy_GPIOInfo[nOffset] ='\0';

	TRACE("INFY GPIO: nBMSGrpMapIdx=[%d]nOffset=[%d] szInfy_GPIOInfo=[%s]\n", \
		nBMSGrpMapIdx, nOffset, s_nd_BattRunData[nBMSGrpMapIdx].szInfy_GPIOInfo);

	return TRUE;
}

static BYTE GetPCSAddr(int nPCSIdx)
{
	int n;
	for(n =0; n < sizeof(szPCSGrpMapIdx)/sizeof(szPCSGrpMapIdx[0]); n++)
	{
		if(n == nPCSIdx)
		{
			return szPCSGrpMapIdx[n];
		}
	}
	return szPCSGrpMapIdx[0];
}

static void CAN_StuffMsgID(BYTE bySubCmd, BYTE byPCSAddr, UINT uiDataLen, BYTE *pbyFrame)
{
	*pbyFrame = (BYTE)uiDataLen;
	*(pbyFrame + 1) = 0x04;
	*(pbyFrame + 2) = 0x0C;
	*(pbyFrame + 3) = bySubCmd;
	*(pbyFrame + 4) = byPCSAddr;

	TRACE("111--- %02x %02x %02x %02x ---\n",pbyFrame[1],pbyFrame[2],pbyFrame[3],pbyFrame[4]);
	return ;
}

static void PackSendFrameData(STBATT_SAMPLER_ROUGH  *pSTBatt_Rough, int nPCSIdx, BYTE bySubCmd,  BYTE *pbySendBuf)
{
	#define MAX_PCS_LIFE_SIGNAL	15
	//Stuff first 29 bits in the send frame
	BYTE byCRC;
	static int nPCSLifeSignal[MAX_SINGLE_STBATT_NUM] = {0};
	A_CAN_ND_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byPCSAddr  = GetPCSAddr(nPCSIdx);

	CAN_StuffMsgID(bySubCmd, frm.byPCSAddr , CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
		
	frm.byData[1] = (BYTE)nPCSLifeSignal[nPCSIdx];  
	nPCSLifeSignal[nPCSIdx]++;
	if(nPCSLifeSignal[nPCSIdx] > 15)
	{
		nPCSLifeSignal[nPCSIdx] = 0;
	}

	//s_nd_BattRunData[nPCSIdx].byPCS_Cmd = 0x01; //For Test
 
	frm.byData[2] = 0x0C | s_nd_BattRunData[nPCSIdx].byPCS_Cmd;
	
	frm.byData[3] = 0x01;
	frm.byData[4] = 0x01;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[1], 7);
	frm.byData[0] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
}


void Make_SendCmd_4CANFrame(IN A_CAN_ND_BATT_FRAME *pFrm)
{
	UINT n = 0;
	UINT uiFrmID = pFrm->uiFrmID;
	BYTE *pbyFrame = pFrm->byRawData;
	UINT uiDataLen = MIN(pFrm->uiDataLen, CAN_DATA_AREA_LEN);

 	*(pbyFrame) = 	(BYTE)uiDataLen;
	*(pbyFrame + 1) = 	(BYTE)(uiFrmID >> 24);
	*(pbyFrame + 2) = 	(BYTE)(uiFrmID >> 16);
	*(pbyFrame + 3) = 	(BYTE)(uiFrmID >> 8);
	*(pbyFrame + 4) = 	(BYTE)(uiFrmID);

	for(n = 0; n < uiDataLen; n++)
	{
		*(pbyFrame + 5 + n) = pFrm->byData[n];
	}

	//TRACE("---------CAN-Data %02x %02x %02x %02x %02x %02x %02x %02x\n",\
	//	pbyFrame[5],pbyFrame[6],pbyFrame[7],pbyFrame[8], \
	//	pbyFrame[9],pbyFrame[10],pbyFrame[11],pbyFrame[12]);

	return;
}

static void Send_C2B_Status(STBATT_SAMPLER_ROUGH  *pSTBatt_Rough, int nPCSIdx)
{
	int n = 0;

	BYTE byRawData[CAN_FRAME_LEN] = {0};
	BYTE* pbySendBuf = byRawData;
	BYTE bySubCmd = 0x8F;

	//TRACE("***Send_C2B_Status : nPCSIdx=[%d] \n", nPCSIdx);
 
	PackSendFrameData(pSTBatt_Rough, nPCSIdx, bySubCmd, pbySendBuf);

 	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[nPCSIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nPCSIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nPCSIdx, pbySendBuf, 1);
		}
	}
	/*
	for(n = 0; n <nWriteSize; n++)
	{
		printf("%02X",pbySendBuf[n]);
	}
	printf("\n");
	*/
	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : CompBMSGrpMapIdx
*  CREATOR       :  
*  CREATED DATE  : 2020-11-14 11:13:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL CompBMSGrpMapIdx(BYTE nBMSGrpAddr, int *nBMSGrpMapIdx)
{
	//TRACE("nBMSGrpAddr = %d sizeof(szBMSGrpMapIdx)/sizeof(szBMSGrpMapIdx[0])=%d \n", nBMSGrpAddr, sizeof(szBMSGrpMapIdx)/sizeof(szBMSGrpMapIdx[0]));
	int n = 0;
	//CATL Protocal Cmd Addr
	for(n = 0; n < sizeof(szBMSGrpMapIdx)/sizeof(szBMSGrpMapIdx[0]); n++)
	{
		if(nBMSGrpAddr == szBMSGrpMapIdx[n])
		{
			*nBMSGrpMapIdx = n;
			return TRUE;
		}
	}

	//Infy Protocal Upgrade Protocal BMS Addr:0x32~0x37
	for(n = 0; n < sizeof(szLocalBMSGrpMapIdx)/sizeof(szLocalBMSGrpMapIdx[0]); n++)
	{
		if(nBMSGrpAddr == szLocalBMSGrpMapIdx[n])
		{
			*nBMSGrpMapIdx = n;
			return TRUE;
		}
	}
	return FALSE;
}

static BOOL CompBattProCmd(int nSubCmd, ID_CMD_ND_BATT *emFuncCmd)
{
	int n =0;
	for(n =0; n < _MAX_ND_CAN_CMD; n++)
	{
		if(nSubCmd == s_nd_BattPro[n].nSubCmd)
		{
			*emFuncCmd = s_nd_BattPro[n].emFuncCmd;
			return TRUE;
		}
	}

	return FALSE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : JudgeAndParseMsgID
*  CREATOR       :  
*  CREATED DATE  : 2020-11-14 11:13:53
*  DESCRIPTION   : Judge Recv BMS Msg Addr:0x040E8F (0x91-0x9A) 
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL JudgeAndParseMsgID(A_CAN_ND_BATT_FRAME* pFrame)
{
	UINT uiRawHead = 0;
	int n;

	uiRawHead = ((UINT)pFrame->byRawData[1]) << 24;
	uiRawHead += ((UINT)pFrame->byRawData[2]) << 16;
	uiRawHead += ((UINT)pFrame->byRawData[3]) << 8;
	uiRawHead += ((UINT)pFrame->byRawData[4]);
	//uiRawHead = (uiRawHead >> 3);

	//TRACE("---- The Parsed Reply Frame Head: [%08X]----------------\n",uiRawHead);

	pFrame->byBMSGrpAddr = (BYTE)uiRawHead;
	if(CompBMSGrpMapIdx(pFrame->byBMSGrpAddr, &pFrame->nBMSGrpMapIdx) == FALSE)
	{
		TRACE("Wrong nBMSGrpAddr =%02X\n", pFrame->byBMSGrpAddr);
		return FALSE;
	}
	
	pFrame->nSubCmd = (uiRawHead >> 8);
	if(CompBattProCmd(pFrame->nSubCmd, &pFrame->emFuncCmd) == FALSE)
	{
		//TRACE("Wrong SubCmd, nSubCmd =%02X pFrame->emFuncCmd)=%02x\n",\
		//	pFrame->nSubCmd, pFrame->emFuncCmd);
		return FALSE;
	} 

	pFrame->byCmdType = (BYTE)(uiRawHead >> 16);
	pFrame->byResv1 = (BYTE)(uiRawHead >> 24);

 	//cp[0] = (uint8_t)(Crc_ComputeCRC8((uint16_t)(id&0xffff),&cp[1],7));
	BYTE crcCheck = (BYTE)Crc_ComputeCRC8((uiRawHead & 0xffff), &pFrame->byRawData[6], 7);
	
	//John 20220627 Added. Sometimes Recv Abnormal Msg,So Add CRC.
	//if((pFrame->emFuncCmd == _CAN_R_CMD_B2C_STATU) ||
	//	(pFrame->emFuncCmd == _CAN_R_CMD_B2C_SUMDATA1) ||
	//	(pFrame->emFuncCmd == _CAN_R_CMD_B2C_SUMDATA2) ||
	//	(pFrame->emFuncCmd == _CAN_R_CMD_B2C_SUMDATA3))
	//{
	//	if(crcCheck != pFrame->byRawData[5])
	//	{
	//		TRACE("emFuncCmd =[%d] Wrong crcCheck=%02X pFrame->byRawData[5]=%02X\n", pFrame->emFuncCmd, crcCheck,  pFrame->byRawData[5]);
	//		return FALSE;
	//	}
	//}	
	//TRACE("byResv1=%02X byCmdType=%02X nSubCmd=%d byBMSGrpAddr=%02X nBMSGrpMapIdx=%d crcCheck=%02X byRawData[5]=%02X \n",\
	//	pFrame->byResv1,pFrame->byCmdType,pFrame->nSubCmd,pFrame->byBMSGrpAddr,pFrame->nBMSGrpMapIdx ,crcCheck, pFrame->byRawData[5]);

	memcpy(pFrame->byData, pFrame->byRawData + 5, CAN_DATA_AREA_LEN);

	/*if((pFrame->byResv1 == 0x04) && (pFrame->byCmdType == 0x0E))
	{
		return TRUE;
	}*/

	return TRUE;
}

static void Pro_ND_Batt_CommStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	VAR_VALUE varV;
	int n = 0 ;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
 		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
        {
			// s_nd_BattRunData[n].bIsCommFail = FALSE; // Only one Batt,so Batt 2 default ok 
            // TRACE("s_nd_BattRunData[%d].bIsCommFail =%d\n",\
			// 	n, s_nd_BattRunData[n].bIsCommFail);
			continue;
        }

		if(s_nd_BattRunData[n].bFoundReply == FALSE)
		{
			if(s_nd_BattRunData[n].nInterrupt_Times <= BATT_COMM_FAIL_TIMES)
			{
				s_nd_BattRunData[n].nInterrupt_Times++;
			}
		}
		else
		{
			s_nd_BattRunData[n].nInterrupt_Times = 0;
		}
			
		//TRACEX("*** ->s_nd_BattRunData[%d].bIsCommFail = %d ->s_nd_BattRunData[n].nInterrupt_Times=%d bFoundReply=%d\n",\
		// 	n, s_nd_BattRunData[n].bIsCommFail,s_nd_BattRunData[n].nInterrupt_Times,s_nd_BattRunData[n].bFoundReply);

		s_nd_BattRunData[n].bIsCommFail = (s_nd_BattRunData[n].nInterrupt_Times > BATT_COMM_FAIL_TIMES) ? TRUE : FALSE;

		if(s_nd_BattRunData[n].bIsCommFail)
		{
			s_nd_BattRunData[n].bBattInfo_Regisited = FALSE;
			TRACE("pSTBatt_Rough->bCtrlOutRelayClose[%d]=%d\n", n, pSTBatt_Rough->bCtrlOutRelayClose[n]);
			if(s_nd_BattRunData[n].fPackTotalCurr < 1.0)
			{
				if(pSTBatt_Rough->bCtrlOutRelayClose[n] == STAT_RELAY_ON)
				{
 					CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
					//LogOut(MAIN_MODULE_SAMPLER, "ND Batt Sampler", LOG_TYPE_ERROR, 
					//"ND Batt V3.3 Grp=[%d] CommFail,Volt=[%.1f] Curr[%.1f]",n,s_nd_BattRunData[n].fPackTotalVolt, s_nd_BattRunData[n].fPackTotalCurr);
				}
			}	
		}	
	}

	//Judge Online Number to Cfg Number
	static int s_LastNumbStatu = 0;
	int nCurrNumbStatu;
	if(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt)
	{	
		nCurrNumbStatu = 1;
		if(nCurrNumbStatu != s_LastNumbStatu)
		{
			s_LastNumbStatu = nCurrNumbStatu;
			varV.emValue = 1; //Alarm
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 504, varV);
			TRACE("--Cfg Number Alarm--\n");
		}
	}
	else
	{
		nCurrNumbStatu = 0;
		if(nCurrNumbStatu != s_LastNumbStatu)
		{
			s_LastNumbStatu = nCurrNumbStatu;
			varV.emValue = 0; //Normal 
			Push_VarValue(SETTYPE_RAW_SP, pSTBatt_Rough->hSampler, 500, 504, varV);
			TRACE("--Cfg Number OK--\n");
		}
	}
	
	return ;
}

static void ReadAndParseCanData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0, nReadBytes = 0, nReadLen = 0;
 	A_CAN_ND_BATT_FRAME frmReply;
	ZERO_POBJS(&frmReply, 1);
	BYTE *byRecvBuff = pSTBatt_Rough->abyRcvBuf;
	memset(byRecvBuff, 0, MAX_READ_BUFF_SIZE * CAN_FRAME_LEN);

	nReadLen = pSTBatt_Rough->pComm->Read(pSTBatt_Rough->hCanDevHanle, (void*)byRecvBuff, (ssize_t)(CAN_FRAME_LEN * MAX_READ_BUFF_SIZE));
	
	//for(n =0 ; n < nReadLen; n++)
	//{
	//	printf("%02X", byRecvBuff[n]);
	//}
	//printf("\n");

	TRACE("Not Found Any Data!!!\n");
	for(n = 0; n < MAX_SINGLE_STBATT_NUM ; n++)
	{
		s_nd_BattRunData[n].bFoundReply = FALSE;
	}

	if(nReadLen >= CAN_FRAME_LEN)
	{
		for(n = 0; n < (nReadLen / CAN_FRAME_LEN); n++)
		{
			memcpy(frmReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
			if(JudgeAndParseMsgID(&frmReply))
			{
				s_nd_BattRunData[frmReply.nBMSGrpMapIdx].bFoundReply = TRUE;
				
				if((pSTBatt_Rough->stuCanSave.stuStatMach[frmReply.nBMSGrpMapIdx].emSaveState == STAT_ON_PENDING) &&
					(pSTBatt_Rough->stuCanSave.emTrigMsgTp[frmReply.nBMSGrpMapIdx] == TP_TRIG_MANUAL))
				{
					Pending_WriteMsg2USB(pSTBatt_Rough, frmReply.nBMSGrpMapIdx, frmReply.byRawData, 1);
				}

				//TRACE("frmReply.nBMSGrpMapIdx=%d\n", frmReply.nBMSGrpMapIdx);
				
				s_nd_BattPro[frmReply.emFuncCmd].pfnCmdHandler(&frmReply);
			}	
		}
	}

	if(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt)
	{
		//pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt = 0;
		for(n = 0 ;n < MAX_SINGLE_STBATT_NUM; n++)
		{
			if(s_nd_BattRunData[n].bBattInfo_Regisited == TRUE)
			{
				s_nd_BattRunData[n].byBattAddr = szBMSGrpMapIdx[n];
				//pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt++;
				TRACE("n=%d nTotalOnlineBatt=%d bBattInfo_Regisited=%d\n", \
					n, pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt, s_nd_BattRunData[n].bBattInfo_Regisited);
			}
		}
	}

	Pro_ND_Batt_CommStatu(pSTBatt_Rough);

	return;
}

static BOOL JudgeIfNeedReconfig(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n;
	BOOL bNeed = FALSE;

	for(n =0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			break;
		}
		if((!s_nd_BattRunData[n].bBattInfo_Regisited) ||
        	s_nd_BattRunData[n].bIsCommFail || 
			(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt))
        {
			TRACEX("n=%d bIsCommFail=%d nSystemCfgBattNum=%d nTotalOnlineBatt=%d bBattInfo_Regisited=%d\n",\
				n, s_nd_BattRunData[n].bIsCommFail, pSTBatt_Rough->nSystemCfgBattNum,\
				pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt,s_nd_BattRunData[n].bBattInfo_Regisited);

            bNeed = TRUE;
            break;
        }
	}

	return bNeed;
}


static void ND_Map_BattRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	static BYTE byLastAlarm[MAX_SINGLE_STBATT_NUM] = {0};
	static BYTE byLastFault[MAX_SINGLE_STBATT_NUM] = {0};

	A_BLOB_GROUP_STBATT_RUNINFO* pGrpRunInfo = &pSTBatt_Rough->blobGroupBattInfo;
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	int nOnlineTotal = 0;
	int nWorkTotal = 0;

	for(n =0 ;n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
		//skip those not configed
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			ZERO_POBJS(pSingleRunInfo, 1);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_NOT_INSTALLED;
			TRACEX("Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		else if((s_nd_BattRunData[n].bIsCommFail) ||
			(!s_nd_BattRunData[n].bBattInfo_Regisited))
		{
			TRACEX("BattIdx=[%d] bIsCommFail = %d, bBattInfo_Regisited = %d\n", n, s_nd_BattRunData[n].bIsCommFail, s_nd_BattRunData[n].bBattInfo_Regisited);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;
			ND_V33_Init_RoughData(pSTBatt_Rough, n);
 			//nOnlineTotal++;
			continue;
		}

		nOnlineTotal++;
		pSingleRunInfo->emPowerONStatu = s_nd_BattRunData[n].byBattPowerON;//s_nd_BattRunData[n].mapNDProtect.Protect.usNotCompletePowerON;
		pSingleRunInfo->fPackTotalVolt = s_nd_BattRunData[n].fPackTotalVolt;
		pSingleRunInfo->fPackTotalCurr = s_nd_BattRunData[n].fPackTotalCurr;
		pSingleRunInfo->nSOC = (UINT)s_nd_BattRunData[n].fSOC;
		pSingleRunInfo->nSOH = s_nd_BattRunData[n].bySOH;
		pSingleRunInfo->fAllow_LmtChargeVolt = s_nd_BattRunData[n].fCharge_Cutoff_Volt;
		pSingleRunInfo->fAllow_LmtDisChargeVolt = s_nd_BattRunData[n].fDisCharge_Cutoff_Volt;
		pSingleRunInfo->fAllow_MaxChargeCurr = s_nd_BattRunData[n].fAllow_MaxChargeCurr;
		pSingleRunInfo->fAllow_MaxDisChargeCurr = s_nd_BattRunData[n].fAllow_MaxDisChargeCurr;

    	pSingleRunInfo->fMax_CellVolt = s_nd_BattRunData[n].fMax_CellVolt;
    	pSingleRunInfo->fMin_CellVolt = s_nd_BattRunData[n].fMin_CellVolt;  

    	pSingleRunInfo->fMax_CellTemp = s_nd_BattRunData[n].fMax_CellTemp;
    	pSingleRunInfo->fMin_CellTemp = s_nd_BattRunData[n].fMin_CellTemp;

    	pSingleRunInfo->nLifeCycleTimes = 0; //s_nd_BattRunData[n].usCyclePeriod;
		pSingleRunInfo->fVersion = s_nd_BattRunData[n].fVersion;

		pSingleRunInfo->uiBattCabDevAlm = s_nd_BattRunData[n].mapSysFault.uiFlag;

		memcpy(pSingleRunInfo->szS2A_AlarmInfo, s_nd_BattRunData[n].szS2A_AlarmInfo, sizeof(pSingleRunInfo->szS2A_AlarmInfo));
		memcpy(pSingleRunInfo->szInfy_AlarmInfo, s_nd_BattRunData[n].szInfy_AlarmInfo, sizeof(pSingleRunInfo->szInfy_AlarmInfo));
		memcpy(pSingleRunInfo->szInfy_FaultInfo, s_nd_BattRunData[n].szInfy_FaultInfo, sizeof(pSingleRunInfo->szInfy_FaultInfo));
		memcpy(pSingleRunInfo->szInfy_GPIOInfo,  s_nd_BattRunData[n].szInfy_GPIOInfo, sizeof(pSingleRunInfo->szInfy_GPIOInfo));

		pSingleRunInfo->uiBAT_Ucell_Max_CSC = s_nd_BattRunData[n].uiBAT_Ucell_Max_CSC;
		pSingleRunInfo->uiBAT_Ucell_Max_CELL = s_nd_BattRunData[n].uiBAT_Ucell_Max_CELL;
		pSingleRunInfo->uiBAT_Ucell_Min_CSC = s_nd_BattRunData[n].uiBAT_Ucell_Min_CSC;
		pSingleRunInfo->uiBAT_Ucell_Min_CELL = s_nd_BattRunData[n].uiBAT_Ucell_Min_CELL;

		//pSingleRunInfo->fBAT_Ucell_Max = s_nd_BattRunData[n].fBAT_Ucell_Max;
		//pSingleRunInfo->fBAT_Ucell_Min = s_nd_BattRunData[n].fBAT_Ucell_Min;

		pSingleRunInfo->fChrg_Energy 		= s_nd_BattRunData[n].fRemainCharge_Energy;
		pSingleRunInfo->fDisChrging_Energy 	= s_nd_BattRunData[n].fRemainDisCharge_Energy;

		//S2A_SUMDATA4
		pSingleRunInfo->fAvg_CellVolt = s_nd_BattRunData[n].fBAT_Ucell_Avg;
		//pSingleRunInfo->nBAT_T_Max = s_nd_BattRunData[n].nBAT_T_Max; 
		//pSingleRunInfo->nBAT_T_Min = s_nd_BattRunData[n].nBAT_T_Min;
		pSingleRunInfo->fAvg_CellTemp = (float)s_nd_BattRunData[n].nBAT_T_Avg;

		pSingleRunInfo->nBAT_T_Max_CSC = s_nd_BattRunData[n].nBAT_T_Max_CSC;
		pSingleRunInfo->nBAT_T_Min_CSC = s_nd_BattRunData[n].nBAT_T_Min_CSC;

		//TRACE("fBAT_Ucell_Avg=[%.1f] nBAT_T_Max=[%d] nBAT_T_Min=[%d] nBAT_T_Avg=[%d] nBAT_T_Max_CSC=[%d] nBAT_T_Min_CSC=[%d]\n",\
		//		pSingleRunInfo->fBAT_Ucell_Avg,
		//		pSingleRunInfo->nBAT_T_Max,
		//		pSingleRunInfo->nBAT_T_Min,
		//		pSingleRunInfo->nBAT_T_Avg,
		//		pSingleRunInfo->nBAT_T_Max_CSC,
		//		pSingleRunInfo->nBAT_T_Min_CSC);

		pSingleRunInfo->byBattAddr = s_nd_BattRunData[n].byBattAddr;

		memcpy(pSingleRunInfo->unioBattGrpAlm.szByAlarm, s_nd_BattRunData[n].byS2A_Alarm, CAN_DATA_AREA_LEN);

		pSingleRunInfo->bIsBatt_ChargeForbidden = s_nd_BattRunData[n].bIsBatt_ChargeForbidden;
		pSingleRunInfo->bIsBatt_DisChargeForbidden = s_nd_BattRunData[n].bIsBatt_DisChargeForbidden;

		//Now handle status
		switch (s_nd_BattRunData[n].byBattRunStatus)
		{
		case 1: //Normal Statu
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
				if(pSingleRunInfo->fPackTotalCurr > 2.0)
				{
					pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_CHARGE;
				}
				else if(pSingleRunInfo->fPackTotalCurr < -2.0)
				{
					pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_DISCHARGE;
				}
				nWorkTotal++;
			}
			break;
		case 2:
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_FULL_CHARGE;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			nWorkTotal++;
			break;
		case 3:
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_FULL_DISCHARGE;
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			nWorkTotal++;
			break;
		
		case 4: //0x04 System has Alarm 
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			}
			break;
		case 5: //Fault
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			}
			break;
		default:
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			}
			break;
		}
		//alarm status

		if((s_nd_BattRunData[n].mapNDFault.usFlag > 0) || (s_nd_BattRunData[n].mapSysFault.uiFlag > 0))
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_FAULT;
			//must set status into sleep in case BMS not set
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			TRACE("BattIdx=[%d] mapNDFault=[%d] mapSysFault=[%d]\n",\
				n,s_nd_BattRunData[n].mapNDFault.usFlag, s_nd_BattRunData[n].mapSysFault.uiFlag );
		}
		else if(s_nd_BattRunData[n].mapNDProtect.usFlag > 0)
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_PROTECT;
			//must set status into sleep in case BMS not set
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
		}
		else if(s_nd_BattRunData[n].mapNDAlarm.byFlag > 0)
		{
			//John 2021-12-15,If BMS Alarm,but Allow Charge Or DisCharge,so Battery is Normal Status 
			//if(((!s_nd_BattRunData[n].bIsBatt_ChargeForbidden) && (s_nd_BattRunData[n].fAllow_MaxChargeCurr > 1.0)) ||
			//	((!s_nd_BattRunData[n].bIsBatt_DisChargeForbidden) && (s_nd_BattRunData[n].fAllow_MaxDisChargeCurr > 1.0)))
			//{
			//	pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
			//	pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
			//	pSingleRunInfo->fAllow_MaxDisChargeCurr = MIN(s_nd_BattRunData[n].fAllow_MaxChargeCurr, 20); //Limit Current 20A
			//	pSingleRunInfo->fAllow_MaxChargeCurr = MIN(s_nd_BattRunData[n].fAllow_MaxDisChargeCurr, 20);
			//}
			//else
			//{
			//	pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_ALARMING;
				//must set status into sleep in case BMS not set
			//	pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			//	pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			//	pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			//}
			if((s_nd_BattRunData[n].fAllow_MaxChargeCurr < 1.0) &&
				(s_nd_BattRunData[n].fAllow_MaxDisChargeCurr < 1.0))
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
				pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
				pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			}
			else
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
			}

			TRACE("pSingleRunInfo->emBattStatus_Alm  =%d\n", pSingleRunInfo->emBattStatus_Alm);
		}
		else if(pSingleRunInfo->nSOC <= (pSTBatt_Rough->nProtectSOC + pSTBatt_Rough->nPVSysBattRetSOC))
		{
			//Should Be Alarm
			//Mppt Chrging to Battery 2A,cause Battery SOC=30->31, then cause ACDC Running ON->OFF->ON
			TRACE("BattIdx=[%d]SOC=%d nProtectSOC=%d emBattStatus_Run=%d emLast_BattStatus_Run=%d\n",\
			n,pSingleRunInfo->nSOC , pSTBatt_Rough->nProtectSOC, pSingleRunInfo->emBattStatus_Run,\
			 pSingleRunInfo->emLast_BattStatus_Run);
			if((pSingleRunInfo->nSOC <= pSTBatt_Rough->nProtectSOC) || (pSingleRunInfo->emLast_BattStatus_Run == STBATT_RUNSTAT_SLEEP))
			{
				pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
				pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			}
			else
			{
				pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
			}
		}
		else
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
		}

		TRACEX("nBattIdx=[%d] byBattRunStatus=[%02X] ->emBattStatus_Alm =%d emBattStatus_Run=%d mapSysFault.uiFlag=[%d]\n", \
 			n, s_nd_BattRunData[n].byBattRunStatus, \
			pSingleRunInfo->emBattStatus_Alm, pSingleRunInfo->emBattStatus_Run, s_nd_BattRunData[n].mapSysFault.uiFlag);

		if( (byLastAlarm[n] != s_nd_BattRunData[n].mapNDAlarm.byFlag) ||
			(byLastFault[n] != s_nd_BattRunData[n].mapNDFault.usFlag))			
		{
			byLastAlarm[n] = s_nd_BattRunData[n].mapNDAlarm.byFlag;
			byLastFault[n] = s_nd_BattRunData[n].mapNDFault.usFlag;
			if((byLastAlarm[n] > 0) || (byLastFault[n] > 0))
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_ERROR,
			"Grp[%d]Alm:[%s]", n, s_nd_BattRunData[n].szS2A_AlarmInfo);
			}
		}

		pSingleRunInfo->emLast_BattStatus_Run = pSingleRunInfo->emBattStatus_Run;

	}//End For Cycle

	pGrpRunInfo->nTotalOnlineBatt = nOnlineTotal;
	//pGrpRunInfo->nTotalInUseBatt = nWorkTotal;

	//pGrpRunInfo->bIsBatt_ChargeForbidden = bGrpNeedForbid_Charge;
	//pGrpRunInfo->bIsBatt_DisChargeForbidden = bGrpNeedForbid_DisCharge;
}



static void NormalRunJudgeBattCabinetStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
 	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		if(s_nd_BattRunData[n].bIsCommFail == TRUE)
		{
			continue;
		}
		
		if(s_nd_BattRunData[n].byPCS_Cmd == STBATT_V33_POWERON_CMD)
	 	{
			UINT nWaitBMSPowerTimes = (s_nd_BattRunData[n].byBattPowerON > 1) ? 10 : 100;//Added By John2022-03-29,SBK Advs Need Wait 10S
			if((s_nd_BattRunData[n].mapNDProtect.Protect.usNotCompletePowerON == TRUE)||isBMSRunAbnormal(pSTBatt_Rough, n, s_nd_BattRunData[n].fPackTotalCurr))//CCU CommFail Alarm
			{
				pSTBatt_Rough->nPowerONTimes[n]++;
				TRACE("nPowerONTimes=%d s_nd_BattRunData[%d].byPCS_Cmd=%d s_nd_BattRunData[%d].mapNDProtect.Protect.usNotCompletePowerON=%d\n",\
				pSTBatt_Rough->nPowerONTimes[n],n,s_nd_BattRunData[n].byPCS_Cmd,n,s_nd_BattRunData[n].mapNDProtect.Protect.usNotCompletePowerON);
				if(pSTBatt_Rough->nPowerONTimes[n] > nWaitBMSPowerTimes) //continues POWER ON Over 50 times,Power OFF 
				{
					//Fault Alarm,need system power off or ctrl wake up battery
					pSTBatt_Rough->nPowerONTimes[n] = 0;
					s_nd_BattRunData[n].mapSysFault.Fault.PowerON_Fail = TRUE;
					if(pSTBatt_Rough->bCtrlOutRelayClose[n] == STAT_RELAY_ON)
					{
						//pSTBatt_Rough->bCtrlOutRelayClose[n] = FALSE;
						CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
						//LogOut(MAIN_MODULE_SAMPLER, "ND Batt Sampler", LOG_TYPE_ERROR, "ND Batt Idx=[%d] V3.3 Power ON Failed!",n);
					
					}
					TRACE("POWER ON FAILED: ND Batt Idx=[%d] V3.3 Power ON Failed!\n", n);
					
				}
			}
			else  
			{
				//Power ON Success !!!
				//printf("POWER ON Success: s_nd_BattRunData[n].mapSysFault.Fault.PowerON_Fail = FALSE\n");
				s_nd_BattRunData[n].mapSysFault.Fault.PowerON_Fail = FALSE;
				if(pSTBatt_Rough->bCtrlOutRelayClose[n] == STAT_RELAY_OFF)
				{
					CtrlBattCabinetContactor(pSTBatt_Rough, n, TRUE);

					TRACE("POWER ON Again: Battery IDx=[%d] Close Cabinet Contactor\n", n);
				}
			}
		}
	}
	return;
}


static void RefreshBattCabinetAlarm(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	VAR_VALUE varV;
	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 456, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.IMEU_CommFail = varV.emValue;
		//TRACE("IMEU CommFail:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 451, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.Fire_Alarm = varV.emValue;	
		//TRACEX("Flood Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 452, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.Smoke_Alarm = varV.emValue;	
		//TRACEX("DoorOpen Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 453, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.DoorOpen_Alarm = varV.emValue;	
		//TRACEX("Smoke Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 454, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.Flammable_Gas = varV.emValue;	
		//TRACEX("Fire Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 462, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.CabSafe_AbnormalState = varV.emValue;	

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 457, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.AirCond_CommFail = varV.emValue;
		
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 450, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.AirCond_Alarm = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 461, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.Flood_Alarm = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			200, 304 + n, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.BattGrp_ForbidRun = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			200, 308 + n, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.BattGrp_InputContactor = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			200, 312 + n, &varV, 4);
		s_nd_BattRunData[n].mapSysFault.Fault.BattGrp_IsolationSwitch = varV.emValue;

	}

	return;
}

static void JudgeBMSAvaliableStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{	
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	int n=0;
	VAR_VALUE 	varV;

 	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 200, 308 + n, &varV, 4);
		pSTBatt_Rough->bCtrlOutRelayClose[n] = (varV.emValue == 0) ? STAT_RELAY_ON : STAT_RELAY_OFF;

		if(s_nd_BattRunData[n].bIsCommFail == TRUE)
		{
			continue;
		}
		
		s_nd_BattRunData[n].byPCS_Cmd = STBATT_V33_POWERON_CMD;

	}
	return;
}

void ND_V33_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//ERR_CODE_CAN_R errCode;
	double tmNow = GetCurrentTime();
	int n = 0;
	static BOOL bFirstRun = TRUE;
	VAR_VALUE varV;
 
	//TRACE("--- ND_V33_Batt_Sample Running --- \n");
	if(JudgeIfNeedReconfig(pSTBatt_Rough))
	{
		ReadAndParseCanData(pSTBatt_Rough);
		
		//TRACE("Here Waitting Batt Reconfig\n");

		//ND_Map_BattRawData_2Rough(pSTBatt_Rough);
		//return;
	}


	for(n= 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(s_nd_BattRunData[n].bIsCommFail == TRUE)
		{
			continue;
		}
		if(fabs(tmNow - s_nd_BattRunData[n].t_LastSendC2BStatus) >= 0.5)
		{
			if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
			{
				TRACE("V33 Send_C2B_Status: nPCSIdx =%d\n",n);
				Send_C2B_Status(pSTBatt_Rough, n); //Alone With Ctrl Cmd (B9~C2)
				s_nd_BattRunData[n].t_LastSendC2BStatus = tmNow;
				Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
			}
		}
	}

	if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
	{	
		ReadAndParseCanData(pSTBatt_Rough);
		Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
	}

	RefreshBattCabinetAlarm(pSTBatt_Rough);

	ND_Map_BattRawData_2Rough(pSTBatt_Rough);

		if(pSTBatt_Rough->stUpgradeParam.emUpgradeStatus != UPGRADE_IDLE)
	{
		if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			HandleBatteryUpgrading(pSTBatt_Rough);			
		}
		Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
		
	}
	//temporary solution
	if(bFirstRun)
	{
		if(ABS(tmNow - pSTBatt_Rough->t_FirstBattBoot) > 30)//ND Batt should wait 20S,then Send Power on Cmd
		{
			bFirstRun = FALSE;
			for(n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
			{
				//s_nd_BattRunData[n].byPCS_Cmd = STBATT_V33_POWERON_CMD;
				//2020-12-29 Not Ctrl Relay Here, NormalRunJudgeBattCabinetStatu will Do this
				//pSTBatt_Rough->bCtrlOutRelayClose[n] = TRUE;
				//CtrlBattCabinetContactor(pSTBatt_Rough, n);
				TRACE("Waiting 20S ,then POWER ON !!!\n");
			}
		}
		return;
	}

	JudgeBMSAvaliableStatu(pSTBatt_Rough);

	NormalRunJudgeBattCabinetStatu(pSTBatt_Rough);
	return;
}

void ND_V33_Init_RoughData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n)
{
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
 
	s_nd_BattRunData[n].fPackTotalVolt = 0;
	s_nd_BattRunData[n].fPackTotalCurr = 0;
	s_nd_BattRunData[n].fSOC = 0;
	s_nd_BattRunData[n].bySOH = 0;
	s_nd_BattRunData[n].fCharge_Cutoff_Volt = 0;
	s_nd_BattRunData[n].fDisCharge_Cutoff_Volt = 0;
	s_nd_BattRunData[n].fAllow_MaxChargeCurr = 0;
	s_nd_BattRunData[n].fAllow_MaxDisChargeCurr = 0;

    s_nd_BattRunData[n].fMax_CellVolt = 0;
    s_nd_BattRunData[n].fMin_CellVolt = 0;  
    s_nd_BattRunData[n].fMax_CellTemp = 0;
    s_nd_BattRunData[n].fMin_CellTemp = 0;
 
	s_nd_BattRunData[n].uiBAT_Ucell_Max_CSC = 0;
	s_nd_BattRunData[n].uiBAT_Ucell_Max_CELL = 0;
	s_nd_BattRunData[n].uiBAT_Ucell_Min_CSC = 0;
	s_nd_BattRunData[n].uiBAT_Ucell_Min_CELL = 0;

	s_nd_BattRunData[n].fBAT_Ucell_Max = 0;
	s_nd_BattRunData[n].fBAT_Ucell_Min = 0;
	
	s_nd_BattRunData[n].fBAT_Ucell_Avg = 0;
	s_nd_BattRunData[n].nBAT_T_Max = 0; 
	s_nd_BattRunData[n].nBAT_T_Min = 0;
	s_nd_BattRunData[n].nBAT_T_Avg = 0;
	s_nd_BattRunData[n].nBAT_T_Max_CSC = 0;
	s_nd_BattRunData[n].nBAT_T_Min_CSC = 0;

	s_nd_BattRunData[n].bIsBatt_ChargeForbidden = FALSE;
    s_nd_BattRunData[n].bIsBatt_DisChargeForbidden = FALSE;

	s_nd_BattRunData[n].mapNDAlarm.byFlag = 0;
	s_nd_BattRunData[n].mapNDFault.usFlag = 0;
	s_nd_BattRunData[n].mapNDProtect.usFlag = 0;

	s_nd_BattRunData[n].mapSysFault.uiFlag = 0;
	
    s_nd_BattRunData[n].usBattModuleQty = 0;
	s_nd_BattRunData[n].byPCS_Cmd = STBATT_V33_HOLDSTAT_CMD;

	pSTBatt_Rough->blobSingleBattInfo[n].emPowerONStatu = s_nd_BattRunData[n].byBattPowerON;
	pSingleRunInfo->fPackTotalVolt = s_nd_BattRunData[n].fPackTotalVolt;
	pSingleRunInfo->fPackTotalCurr = s_nd_BattRunData[n].fPackTotalCurr;
	pSingleRunInfo->nSOC = (UINT)s_nd_BattRunData[n].fSOC;
	pSingleRunInfo->nSOH = s_nd_BattRunData[n].bySOH;
	pSingleRunInfo->fAllow_LmtChargeVolt = s_nd_BattRunData[n].fCharge_Cutoff_Volt;
	pSingleRunInfo->fAllow_LmtDisChargeVolt = s_nd_BattRunData[n].fDisCharge_Cutoff_Volt;
	pSingleRunInfo->fAllow_MaxChargeCurr = s_nd_BattRunData[n].fAllow_MaxChargeCurr;
	pSingleRunInfo->fAllow_MaxDisChargeCurr = s_nd_BattRunData[n].fAllow_MaxDisChargeCurr;

	pSingleRunInfo->fMax_CellVolt = s_nd_BattRunData[n].fMax_CellVolt;
	pSingleRunInfo->fMin_CellVolt = s_nd_BattRunData[n].fMin_CellVolt;
	pSingleRunInfo->fAvg_CellVolt = s_nd_BattRunData[n].fBAT_Ucell_Avg;

	pSingleRunInfo->fMax_CellTemp = s_nd_BattRunData[n].fMax_CellTemp;
	pSingleRunInfo->fMin_CellTemp = s_nd_BattRunData[n].fMin_CellTemp;
	pSingleRunInfo->fAvg_CellTemp = (float)s_nd_BattRunData[n].nBAT_T_Avg;

	pSingleRunInfo->nLifeCycleTimes = 0; //s_nd_BattRunData[n].usCyclePeriod;

	pSingleRunInfo->uiBAT_Ucell_Max_CSC = s_nd_BattRunData[n].uiBAT_Ucell_Max_CSC;
	pSingleRunInfo->uiBAT_Ucell_Max_CELL = s_nd_BattRunData[n].uiBAT_Ucell_Max_CELL;
	pSingleRunInfo->uiBAT_Ucell_Min_CSC = s_nd_BattRunData[n].uiBAT_Ucell_Min_CSC;
	pSingleRunInfo->uiBAT_Ucell_Min_CELL = s_nd_BattRunData[n].uiBAT_Ucell_Min_CELL;

	//pSingleRunInfo->fBAT_Ucell_Max = s_nd_BattRunData[n].fBAT_Ucell_Max;
	//pSingleRunInfo->fBAT_Ucell_Min = s_nd_BattRunData[n].fBAT_Ucell_Min;

	//S2A_SUMDATA4
	//pSingleRunInfo->fBAT_Ucell_Avg = s_nd_BattRunData[n].fBAT_Ucell_Avg;
	//pSingleRunInfo->nBAT_T_Max = s_nd_BattRunData[n].nBAT_T_Max; 
	//pSingleRunInfo->nBAT_T_Min = s_nd_BattRunData[n].nBAT_T_Min;
	//pSingleRunInfo->nBAT_T_Avg = s_nd_BattRunData[n].nBAT_T_Avg;

	pSingleRunInfo->nBAT_T_Max_CSC = s_nd_BattRunData[n].nBAT_T_Max_CSC;
	pSingleRunInfo->nBAT_T_Min_CSC = s_nd_BattRunData[n].nBAT_T_Min_CSC;
	pSingleRunInfo->uiBattCabDevAlm	= (UINT)s_nd_BattRunData[n].mapSysFault.uiFlag;

	pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;
	pSingleRunInfo->emLast_BattStatus_Run = STBATT_RUNSTAT_SLEEP;

	//Added For Health Statistic . By John 20220516
	pSingleRunInfo->fChg_Start_MaxCellTemp = 0.0;
	pSingleRunInfo->fChg_Start_MinCellTemp = 0.0;
	pSingleRunInfo->fDischg_Start_MaxCellTemp = 0.0;
	pSingleRunInfo->fDischg_Start_MinCellTemp = 0.0;

	return;
}
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2019-12-12 19:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ND_V33_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->nCtrlTimes[n] = 0;
    	pSTBatt_Rough->nPowerONTimes[n] = 0;
		CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
	}
	
	//2021-11-2 A2105 Not Used this Yet
	//Set_IMMU_DO(IOC_SET_DO5, FALSE);
   	//Set_IMMU_DO(IOC_SET_DO6, FALSE);

	//---------Init cmd-------------
	TRACE("--- ND_Batt_Init Enter --- \n");
	TAG_ND_BATT_PRO *p = &s_nd_BattPro[0];
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_STATU,	 0x040E8F,	Parse_B2C_Status 	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_SUMDATA1, 0x040E90,	Parse_B2C_SumData1	);	
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_SUMDATA2, 0x040E91,	Parse_B2C_SumData2	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_SUMDATA3, 0x040E93,	Parse_B2C_SumData3	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_LIMIT,	 0x040E92,	Parse_B2C_Limit		);

	INIT_NDBATT_CMD(p,	_CAN_R_CMD_S2A_ALARM, 	  0x0701FF,	Parse_S2A_Alarm		);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_S2A_SUMDATA3,  0x0B03FF,	Parse_S2A_SumData3	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_S2A_SUMDATA4,  0x0B04FF,	Parse_S2A_SumData4	);

	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_INFYALARM,  		0x1435FF,	Parse_B2C_InfyAlarm );  
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_INFYFAULT,  		0x1436FF,	Parse_B2C_InfyFault ); 
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_INFYGPIO,  		0x1438FF,	Parse_B2C_InfyGPIO ); 

	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_UPGRADESTATU,  	0x1CFA02,	Parse_B2C_UpgradeStatu ); //0xFF-Broadcast cmd,BMS Addr:0x32~0x37.

 	//INIT_NDBATT_CMD(p,	_CAN_S_CMD_C2B_STATU,		Send_C2BStatus);

	//---------Init rough-------------
	pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt = 0;

	pSTBatt_Rough->stUpgradeParam.emUpgradeStatus = UPGRADE_IDLE;
	pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.pbyReadBuf = NULL;
	
	int i;
	for(i = 0; i < MAX_SINGLE_STBATT_NUM; i++)
	{
		memset(&s_nd_BattRunData[i], 0 ,sizeof(ND_V33_BATT_RUN_DATA));
		
		s_nd_BattRunData[i].byBattPowerON = 0; 
		s_nd_BattRunData[i].byBattRunStatus = 0; 
		s_nd_BattRunData[i].fPackTotalVolt = 0; 
    	s_nd_BattRunData[i].fPackTotalCurr = 0; 
    	s_nd_BattRunData[i].fMainControllerTemperature = 0; 
    	s_nd_BattRunData[i].fSOC = 0; 
  
    	//SUMDATA1
		s_nd_BattRunData[i].fRemainCharge_Energy = 0;
		s_nd_BattRunData[i].fRemainDisCharge_Energy = 0;
		s_nd_BattRunData[i].bySOH = 0;

		//SUMDATA2
		s_nd_BattRunData[i].fMax_CellVolt = 0;
    	s_nd_BattRunData[i].fMin_CellVolt = 0;
		s_nd_BattRunData[i].fMax_CellTemp = 0;
    	s_nd_BattRunData[i].fMin_CellTemp = 0;
		s_nd_BattRunData[i].fVersion = 0;

		//SUMDATA3
		s_nd_BattRunData[i].fAllow_MaxDisChargeCurr = 0;
		s_nd_BattRunData[i].fAllow_MaxChargeCurr = 0;
		s_nd_BattRunData[i].fCharge_Cutoff_Volt = 0;
    	s_nd_BattRunData[i].fDisCharge_Cutoff_Volt = 0;

		//S2A_SUMDATA3 
		s_nd_BattRunData[i].uiBAT_Ucell_Max_CSC = 0;
		s_nd_BattRunData[i].uiBAT_Ucell_Max_CELL = 0;
		s_nd_BattRunData[i].uiBAT_Ucell_Min_CSC = 0;
		s_nd_BattRunData[i].uiBAT_Ucell_Min_CELL = 0;

		s_nd_BattRunData[i].fBAT_Ucell_Max = 0;
		s_nd_BattRunData[i].fBAT_Ucell_Min = 0;

		//S2A_SUMDATA4
		s_nd_BattRunData[i].fBAT_Ucell_Avg = 0;
		s_nd_BattRunData[i].nBAT_T_Max = 0;
		s_nd_BattRunData[i].nBAT_T_Min = 0;
		s_nd_BattRunData[i].nBAT_T_Avg = 0;
		s_nd_BattRunData[i].nBAT_T_Max_CSC = 0;
		s_nd_BattRunData[i].nBAT_T_Min_CSC = 0;

		s_nd_BattRunData[i].bIsBatt_ChargeForbidden = FALSE;
    	s_nd_BattRunData[i].bIsBatt_DisChargeForbidden = FALSE;

		s_nd_BattRunData[i].mapNDAlarm.byFlag = 0;
		s_nd_BattRunData[i].mapNDFault.usFlag = 0;
		s_nd_BattRunData[i].mapNDProtect.usFlag = 0;

		s_nd_BattRunData[i].mapSysFault.uiFlag = 0;
		
    	s_nd_BattRunData[i].usBattModuleQty = 0;
		s_nd_BattRunData[i].byPCS_Cmd = STBATT_V33_HOLDSTAT_CMD;

		s_nd_BattRunData[i].t_LastSendC2BStatus = 0;
    	s_nd_BattRunData[i].bFoundReply = FALSE;
		s_nd_BattRunData[i].nInterrupt_Times = 0;
		s_nd_BattRunData[i].bIsCommFail = TRUE;
    	s_nd_BattRunData[i].bBattInfo_Regisited = FALSE; //if the battGrp info NOT registered
	}

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  ND_Batt_Control
*  CREATOR       :  
*  CREATED DATE  : 2019-12-12 19:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL ND_V33_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt)
{
	BOOL bRtn = FALSE;
	TRACE("-----Now received cmd[%d] for ND Batt[%d], bIscommFail = %d--------\n", emCmdTp, nWhichBatt, 
		s_nd_BattRunData[nWhichBatt].bIsCommFail);
		
	if(s_nd_BattRunData[nWhichBatt].bIsCommFail)
	{
		return FALSE;
	}
  
	if(emCmdTp == CTRL_STBATT_WAKE_UP)
	{
		TRACE("ND BATT[%d]_WAKE_UP -Power ON\n", nWhichBatt);
		s_nd_BattRunData[nWhichBatt].byPCS_Cmd = STBATT_V33_POWERON_CMD;
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_GO_SLEEP)
	{
		TRACE("ND BATT[%d]_GOTO_SLEEP Power OFF\n", nWhichBatt);
		s_nd_BattRunData[nWhichBatt].byPCS_Cmd = STBATT_V33_POWEROFF_CMD;
		bRtn = TRUE;
	}
	 
	return bRtn;

}
