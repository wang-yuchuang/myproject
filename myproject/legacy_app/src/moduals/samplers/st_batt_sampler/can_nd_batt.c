/*#include "st_batt_sampler.h"

#define TRACE	1 ? (void)0 : (void)printf  //printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf


//Crc_ComputeCRC8- 按SAE J1850 方式计算CRC值
uint8_t Crc_ComputeCRC8(const uint16_t Msg_Id, const uint8_t* Crc_DataPtr, uint16_t Crc_Length)
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


static BYTE szBMSGrpMapIdx[]={0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A};
static BYTE szPCSGrpMapIdx[]={0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2};

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

typedef union tagNDBattFaultByte
{
	BYTE	byFlag;
	struct
	{
		BYTE	IMEU_CommFail : 1;
		BYTE	Flood_Alarm : 1;
		BYTE	DoorOpen_Alarm : 1;
		BYTE	Smoke_Alarm : 1;
        BYTE	Fire_Alarm : 1;
		BYTE	PowerON_Fail : 1;
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
  
	ND_BATT_ALARM_MAP	mapNDAlarm;
	
	ND_BATT_FAULT_MAP	mapNDFault;

	ND_BATT_PROTECT_MAP mapNDProtect;

	BOOL                bIsBatt_ChargeForbidden;
    BOOL                bIsBatt_DisChargeForbidden;

    USHORT              usBattModuleQty;
	
	BYTE				byPCS_Cmd;

	time_t 				t_LastSendC2BStatus;
    int                 bFoundReply;
	int			        nInterrupt_Times;
	BOOL		        bIsCommFail;
    BOOL                bBattInfo_Regisited; //if the battGrp info NOT registered
	
}ND_BATT_RUN_DATA;

#define MKAE_ND_BATT_FRAME(pFrame, _emFuncCmd, _bySubCmd, _nBMSGrpMapIdx, _bIsCtrlCmd) \
		((pFrame)->emFuncCmd = (_emFuncCmd), \
		(pFrame)->bySubCmd = (_bySubCmd), \
		(pFrame)->nBMSGrpMapIdx = (_nBMSGrpMapIdx), \
		(pFrame)->bIsCtrlCmd = (_bIsCtrlCmd))

typedef BOOL (*CAN_ND_BATT_SAMPLE_PROC)(A_CAN_ND_BATT_FRAME *pFrmReply);
typedef struct tagNDBattPro
{
	ID_CMD_ND_BATT emFuncCmd;
	BYTE 			bySubCmd;

	CAN_ND_BATT_SAMPLE_PROC pfnCmdHandler;

}TAG_ND_BATT_PRO;

#define  INIT_NDBATT_CMD(p, _emFuncCmd, _bySubCmd, _pfnHandler) \
	((p)->emFuncCmd = (_emFuncCmd), \
	(p)->bySubCmd = (_bySubCmd), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)++);

static ND_BATT_RUN_DATA     s_nd_BattRunData[MAX_SINGLE_STBATT_NUM]; //max 6 grp of batt
static TAG_ND_BATT_PRO		s_nd_BattPro[_MAX_ND_CAN_CMD];

static void CtrlBattCabinetContactor(BOOL bClose)
{
	TRACE("CtrlBattCabinetContactor: bClose=%d\n",bClose);
	Set_IMMU_DO(IOC_SET_DO5, bClose);
	return;
} 

static USHORT CAN_StringToUShort(BYTE* strInput) //LSB
{
	USHORT usValue = ((USHORT)(strInput[0]))
		+ (((USHORT)(strInput[1])) << 8);

	return usValue;
}

BOOL Parse_B2C_Status(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_nd_BattRunData[nBMSGrpMapIdx].bFoundReply = TRUE;
    s_nd_BattRunData[nBMSGrpMapIdx].bBattInfo_Regisited = TRUE;  

	//V3.0
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.Alarm.Module_HighVolt = (*(pFrmReply->byData + 2)) & 0x01;
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.Alarm.Module_LowVolt = ((*(pFrmReply->byData + 2)) >> 1) & 0x01; 
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.Alarm.Module_HighTemp = ((*(pFrmReply->byData + 2)) >> 2) & 0x01; 
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.Alarm.Module_LowTemp = ((*(pFrmReply->byData + 2)) >> 3) & 0x01; 
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.Alarm.Low_Insulation = ((*(pFrmReply->byData + 2)) >> 4) & 0x01; 
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.Alarm.Inner_CommFail = ((*(pFrmReply->byData + 2)) >> 5) & 0x01; 

	s_nd_BattRunData[nBMSGrpMapIdx].byBattPowerON = ((*(pFrmReply->byData + 4)) >> 1) & 0x01; 
	s_nd_BattRunData[nBMSGrpMapIdx].mapNDProtect.Protect.usNotCompletePowerON = (s_nd_BattRunData[nBMSGrpMapIdx].byBattPowerON ==0) ? TRUE : FALSE;
	s_nd_BattRunData[nBMSGrpMapIdx].byBattRunStatus = ((*(pFrmReply->byData + 4)) >> 2) & 0x07; 
 	s_nd_BattRunData[nBMSGrpMapIdx].fSOC = ((float)CAN_StringToUShort(pFrmReply->byData + 6))*0.1;
	
	TRACE("nBMSGrpMapIdx=[%d] byBattPowerON=%d byBattRunStatus=%d fSOC=%f usFlag=%d usNotCompletePowerON=%d\n",\
		nBMSGrpMapIdx,
		s_nd_BattRunData[nBMSGrpMapIdx].byBattPowerON,\
		s_nd_BattRunData[nBMSGrpMapIdx].byBattRunStatus,\
		s_nd_BattRunData[nBMSGrpMapIdx].fSOC,\
		s_nd_BattRunData[nBMSGrpMapIdx].mapNDAlarm.byFlag,\
		s_nd_BattRunData[nBMSGrpMapIdx].mapNDProtect.Protect.usNotCompletePowerON);
	
	/*int i;
	for(i =0;i < CAN_DATA_AREA_LEN; i++)
	{
		printf("%02X",pFrmReply->byData[i]);
	}
	printf("\n");
	*/
	return TRUE;
}

BOOL Parse_B2C_SumData1(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalVolt = ((float)CAN_StringToUShort(pFrmReply->byData +2))*0.1;
	s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalCurr = -(((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.1 -2000);

	//ND Batt: Charge Current is Negative, Discharge Current is Positive, so Reverse it;

	TRACE("nBMSGrpMapIdx=[%d] fPackTotalVolt=%f fPackTotalCurr=%f\n", \
		nBMSGrpMapIdx,s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalVolt,s_nd_BattRunData[nBMSGrpMapIdx].fPackTotalCurr);
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

BOOL Parse_B2C_SumData2(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	s_nd_BattRunData[nBMSGrpMapIdx].fRemainCharge_Energy = ((float)CAN_StringToUShort(pFrmReply->byData +2))*0.1;
    s_nd_BattRunData[nBMSGrpMapIdx].fRemainDisCharge_Energy = ((float)CAN_StringToUShort(pFrmReply->byData + 4))*0.1;
	s_nd_BattRunData[nBMSGrpMapIdx].bySOH = *(pFrmReply->byData + 6);

	TRACE("nBMSGrpMapIdx=[%d] fRemainCharge_Energy=%f fRemainDisCharge_Energy=%f bySOH=%d\n",\
		nBMSGrpMapIdx, s_nd_BattRunData[nBMSGrpMapIdx].fRemainCharge_Energy, s_nd_BattRunData[nBMSGrpMapIdx].fRemainDisCharge_Energy,\
		s_nd_BattRunData[nBMSGrpMapIdx].bySOH);

	return TRUE;
}

BOOL Parse_B2C_SumData3(A_CAN_ND_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellVolt = ((float)CAN_StringToUShort(pFrmReply->byData +1))*0.001;
    s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellVolt = ((float)CAN_StringToUShort(pFrmReply->byData +3))*0.001;
	s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellTemp = ((float)(*(pFrmReply->byData + 5))) - 50;
    s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellTemp = ((float)(*(pFrmReply->byData + 6))) - 50;
	//s_nd_BattRunData[nBMSGrpMapIdx].fVersion = ((float)(*(pFrmReply->byData + 7)))*0.1;

	TRACE("nBMSGrpMapIdx[=%d] fMax_CellVolt=%f fMin_CellVolt=%f fMax_CellTemp=%f fMin_CellTemp=%f fVersion=%f \n",\
		nBMSGrpMapIdx, s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellVolt, s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellVolt,\
		s_nd_BattRunData[nBMSGrpMapIdx].fMax_CellTemp, s_nd_BattRunData[nBMSGrpMapIdx].fMin_CellTemp, s_nd_BattRunData[nBMSGrpMapIdx].fVersion);

	return TRUE;
}

BOOL Parse_B2C_Limit(A_CAN_ND_BATT_FRAME *pFrmReply)
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

	TRACE("nBMSGrpMapIdx[=%d] fAllow_MaxDisChargeCurr=%f fAllow_MaxChargeCurr =%f fCharge_Cutoff_Volt=%f fDisCharge_Cutoff_Volt=%f\n", \
		nBMSGrpMapIdx, s_nd_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr, s_nd_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr,\
		s_nd_BattRunData[nBMSGrpMapIdx].fCharge_Cutoff_Volt,s_nd_BattRunData[nBMSGrpMapIdx].fDisCharge_Cutoff_Volt );


	return TRUE;
}

BYTE GetPCSAddr(int nPCSIdx)
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

void PackSendFrameData(STBATT_SAMPLER_ROUGH  *pSTBatt_Rough, int nPCSIdx, BYTE bySubCmd,  BYTE *pbySendBuf)
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

void Send_C2B_Status(STBATT_SAMPLER_ROUGH  *pSTBatt_Rough, int nPCSIdx)
{
	int n;

	BYTE byRawData[CAN_FRAME_LEN] = {0};
	BYTE* pbySendBuf = byRawData;
	BYTE bySubCmd = 0x8F;

	TRACE("***Send_C2B_Status : nPCSIdx=[%d] \n", nPCSIdx);
 
	PackSendFrameData(pSTBatt_Rough, nPCSIdx, bySubCmd, pbySendBuf);

 	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}

	return TRUE;
}

BOOL CompBMSGrpMapIdx(BYTE nBMSGrpAddr, int *nBMSGrpMapIdx)
{
	//TRACE("nBMSGrpAddr = %d sizeof(szBMSGrpMapIdx)/sizeof(szBMSGrpMapIdx[0])=%d \n", nBMSGrpAddr, sizeof(szBMSGrpMapIdx)/sizeof(szBMSGrpMapIdx[0]));
	int n = 0;
	for(n=0; n < sizeof(szBMSGrpMapIdx)/sizeof(szBMSGrpMapIdx[0]); n++)
	{
		if(nBMSGrpAddr == szBMSGrpMapIdx[n])
		{
			*nBMSGrpMapIdx = n;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CompBattProCmd(BYTE bySubCmd, ID_CMD_ND_BATT *emFuncCmd)
{
	int n =0;
	for(n =0; n < _MAX_ND_CAN_CMD; n++)
	{
		if(bySubCmd == s_nd_BattPro[n].bySubCmd)
		{
			*emFuncCmd = s_nd_BattPro[n].emFuncCmd;
			return TRUE;
		}
	}

	return FALSE;
}

//0x040E8F (0x91-0x9A)
BOOL JudgeAndParseMsgID(A_CAN_ND_BATT_FRAME* pFrame)
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
	
	pFrame->bySubCmd = (BYTE)(uiRawHead >> 8);
	if(CompBattProCmd(pFrame->bySubCmd, &pFrame->emFuncCmd) == FALSE)
	{
		TRACE("Wrong SubCmd, bySubCmd =%02X\n",pFrame->bySubCmd);
		return FALSE;
	} 

	pFrame->byCmdType = (BYTE)(uiRawHead >> 16);
	pFrame->byResv1 = (BYTE)(uiRawHead >> 24);

 	//cp[0] = (uint8_t)(Crc_ComputeCRC8((uint16_t)(id&0xffff),&cp[1],7));
	BYTE crcCheck = (BYTE)Crc_ComputeCRC8((uiRawHead & 0xffff), &pFrame->byRawData[6], 7);
	if(crcCheck != pFrame->byRawData[5])
	{
		//TRACE("Wrong crcCheck=%02X pFrame->byRawData[5]=%02X\n", crcCheck,  pFrame->byRawData[5]);
		//return FALSE;
	}
	
	TRACE("byResv1=%02X byCmdType=%02X bySubCmd=%02X byBMSGrpAddr=%02X nBMSGrpMapIdx=%d crcCheck=%02X byRawData[5]=%02X \n",\
		pFrame->byResv1,pFrame->byCmdType,pFrame->bySubCmd,pFrame->byBMSGrpAddr,pFrame->nBMSGrpMapIdx ,crcCheck, pFrame->byRawData[5]);

	memcpy(pFrame->byData, pFrame->byRawData + 5, CAN_DATA_AREA_LEN);

	if((pFrame->byResv1 == 0x04) && (pFrame->byCmdType == 0x0E))
	{
		return TRUE;
	}

	return FALSE;
}

static void Pro_ND_Batt_CommStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
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
			
		TRACE("*** ->s_nd_BattRunData[%d].bIsCommFail = %d ->s_nd_BattRunData[n].nInterrupt_Times=%d bFoundReply=%d\n",\
		 	n, s_nd_BattRunData[n].bIsCommFail,s_nd_BattRunData[n].nInterrupt_Times,s_nd_BattRunData[n].bFoundReply);

		s_nd_BattRunData[n].bIsCommFail = (s_nd_BattRunData[n].nInterrupt_Times > BATT_COMM_FAIL_TIMES) ? TRUE : FALSE;

		if(s_nd_BattRunData[n].bIsCommFail)
		{
			s_nd_BattRunData[n].bBattInfo_Regisited = FALSE;
		}
		else
		{
			
		}
		
	}

	return ;
}


void ReadAndParseCanData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0, nReadBytes = 0, nReadLen = 0;
	BOOL bFoundReply = FALSE;
	A_CAN_ND_BATT_FRAME frmReply;
	ZERO_POBJS(&frmReply, 1);
	BYTE *byRecvBuff = pSTBatt_Rough->abyRcvBuf;
	ZERO_POBJS(byRecvBuff, MAX_READ_BUFF_SIZE * CAN_FRAME_LEN);

	nReadLen = pSTBatt_Rough->pComm->Read(pSTBatt_Rough->hCanDevHanle, (void*)byRecvBuff, (ssize_t)(CAN_FRAME_LEN * MAX_READ_BUFF_SIZE));
	
	/*
	for(n =0 ; n < nReadLen; n++)
	{
		printf("%02X", byRecvBuff[n]);
	}
	printf("\n");
	*/

	if(nReadLen >= CAN_FRAME_LEN)
	{
		for(n =0; n < nReadLen / CAN_FRAME_LEN; n++)
		{
			memcpy(frmReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
			if(JudgeAndParseMsgID(&frmReply))
			{
				bFoundReply = TRUE;
				s_nd_BattPro[frmReply.emFuncCmd].pfnCmdHandler(&frmReply);
			}	
		}
	}

	if(!bFoundReply)
	{
		TRACE("Not Found Any Data!!!\n");
		for(n = 0; n < MAX_SINGLE_STBATT_NUM ; n++)
		{
			s_nd_BattRunData[n].bFoundReply = FALSE;

		}
	}
	
	if(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt)
	{
		pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt = 0;
		for(n = 0 ;n < MAX_SINGLE_STBATT_NUM; n++)
		{
			if(s_nd_BattRunData[n].bBattInfo_Regisited == TRUE)
			{
				pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt++;
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
			TRACE("n=%d bIsCommFail=%d nSystemCfgBattNum=%d nTotalOnlineBatt=%d bBattInfo_Regisited=%d\n",\
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
	int n;
	static BYTE byLastAlarm = 0;
	VAR_VALUE varV;
	A_BLOB_GROUP_STBATT_RUNINFO* pGrpRunInfo = &pSTBatt_Rough->blobGroupBattInfo;
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	int nOnlineTotal = 0;
	int nWorkTotal = 0;
	BOOL bGrpNeedForbid_Charge = FALSE;
	BOOL bGrpNeedForbid_DisCharge = FALSE;

	for(n =0 ;n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
		//skip those not configed
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			ZERO_POBJS(pSingleRunInfo, 1);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_NOT_INSTALLED;
			continue;
		}
		else if((s_nd_BattRunData[n].bIsCommFail) ||
			(!s_nd_BattRunData[n].bBattInfo_Regisited))
		{
			TRACE("bIsCommFail = %d, bBattInfo_Regisited = %d\n", s_nd_BattRunData[n].bIsCommFail,
				s_nd_BattRunData[n].bBattInfo_Regisited);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;
			nOnlineTotal++;
			continue;
		}

		nOnlineTotal++;
	
		pSingleRunInfo->fPackTotalVolt = s_nd_BattRunData[n].fPackTotalVolt;
		pSingleRunInfo->fPackTotalCurr = s_nd_BattRunData[n].fPackTotalCurr;
		pSingleRunInfo->nSOC = (int)s_nd_BattRunData[n].fSOC;
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

		bGrpNeedForbid_Charge = s_nd_BattRunData[n].bIsBatt_ChargeForbidden ? TRUE : FALSE; //if all is forbidden, then forbidden
		pSingleRunInfo->bIsBatt_ChargeForbidden = s_nd_BattRunData[n].bIsBatt_ChargeForbidden;

		if(s_nd_BattRunData[n].bIsBatt_DisChargeForbidden)
		{
			bGrpNeedForbid_DisCharge = TRUE;
		}

		pSingleRunInfo->bIsBatt_DisChargeForbidden = s_nd_BattRunData[n].bIsBatt_DisChargeForbidden;

		//Now handle status
		switch (s_nd_BattRunData[n].byBattRunStatus)
		{
		case 1:
			{
				bGrpNeedForbid_Charge = FALSE;
				bGrpNeedForbid_DisCharge = FALSE;
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
				nWorkTotal++;
			}
			break;
		case 2:
			bGrpNeedForbid_Charge = TRUE;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			break;
		case 3:
			bGrpNeedForbid_DisCharge = TRUE;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			break;
		
		case 4: //0x04 System has Alarm, Power-Limited Mode
			{
				bGrpNeedForbid_Charge = FALSE;
				bGrpNeedForbid_DisCharge = FALSE;
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
				nWorkTotal++;
			}
			break;
		case 5:
		default:
			{
				bGrpNeedForbid_Charge = TRUE;
				bGrpNeedForbid_DisCharge = TRUE;
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			}
			break;
		}
		//alarm status
		
		if(s_nd_BattRunData[n].mapNDFault.byFlag > 0)
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_FAULT;
			//must set status into sleep in case BMS not set
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
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
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_ALARMING;
			//must set status into sleep in case BMS not set
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
		}
		else
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
		}
		
		if( byLastAlarm != s_nd_BattRunData[n].mapNDAlarm.byFlag)			
		{
			byLastAlarm = s_nd_BattRunData[n].mapNDAlarm.byFlag;
			if(byLastAlarm > 0)
			{
				LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_ERROR,
			"STBatt Group[%d][%02X] Alarm!", n, s_nd_BattRunData[n].mapNDAlarm.byFlag);
			}
		}
	}



	pGrpRunInfo->nTotalOnlineBatt = nOnlineTotal;
	pGrpRunInfo->nTotalInUseBatt = nWorkTotal;

	pGrpRunInfo->bIsBatt_ChargeForbidden = bGrpNeedForbid_Charge;
	pGrpRunInfo->bIsBatt_DisChargeForbidden = bGrpNeedForbid_DisCharge;
}
static void NormalRunJudgeBattCabinetStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n=0;
 	static int nPowerONTimes = 0;
	for(n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
	{
		if(s_nd_BattRunData[n].byPCS_Cmd == 0x01)
	 	{
			if(s_nd_BattRunData[n].mapNDProtect.Protect.usNotCompletePowerON == TRUE)
			{
				nPowerONTimes++;
				TRACE("nPowerONTimes=%d s_nd_BattRunData[%d].byPCS_Cmd=%d s_nd_BattRunData[%d].mapNDProtect.Protect.usNotCompletePowerON=%d\n",\
				nPowerONTimes,n,s_nd_BattRunData[n].byPCS_Cmd,n,s_nd_BattRunData[n].mapNDProtect.Protect.usNotCompletePowerON);
				if(nPowerONTimes > 3) //continues POWER ON Over 10 times,Power OFF
				{
					//Fault Alarm,need system power off or ctrl wake up battery
					nPowerONTimes = 0;
					s_nd_BattRunData[n].mapNDFault.Fault.PowerON_Fail = TRUE;
					if(pSTBatt_Rough->bCtrlDO5Close)
					{
						pSTBatt_Rough->bCtrlDO5Close = FALSE;
						CtrlBattCabinetContactor(pSTBatt_Rough->bCtrlDO5Close);
						LogOut(MAIN_MODULE_SAMPLER, "ND Batt Sampler", LOG_TYPE_ERROR, "ND Batt V3.0 Power ON Failed!");
					
					}
					TRACE("POWER ON FAILED: ND Batt V3.0 Power ON Failed!\n");
					
				}
			}
			else  
			{
				//Power ON Success !!!
				//printf("POWER ON Success: s_nd_BattRunData[n].mapNDFault.Fault.PowerON_Fail = FALSE\n");
				s_nd_BattRunData[n].mapNDFault.Fault.PowerON_Fail = FALSE;
				if(pSTBatt_Rough->bCtrlDO5Close == FALSE)
				{
					pSTBatt_Rough->bCtrlDO5Close = TRUE;
					CtrlBattCabinetContactor(pSTBatt_Rough->bCtrlDO5Close);

					TRACE("POWER ON Again: Close Cabinet Contactor\n");
				}
			}
		}
	}
	return;
}


static void RefreshBattCabinetAlarm(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n;

	VAR_VALUE varV;
	for(n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
	{
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601, 456, &varV, 4);
		s_nd_BattRunData[n].mapNDFault.Fault.IMEU_CommFail = varV.emValue;
		//TRACE("IMEU CommFail:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601, 451, &varV, 4);
		s_nd_BattRunData[n].mapNDFault.Fault.Flood_Alarm = varV.emValue;	
		//TRACEX("Flood Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601, 452, &varV, 4);
		s_nd_BattRunData[n].mapNDFault.Fault.DoorOpen_Alarm = varV.emValue;	
		//TRACEX("DoorOpen Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601, 453, &varV, 4);
		s_nd_BattRunData[n].mapNDFault.Fault.Smoke_Alarm = varV.emValue;	
		//TRACEX("Smoke Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601, 454, &varV, 4);
		s_nd_BattRunData[n].mapNDFault.Fault.Fire_Alarm = varV.emValue;	
		//TRACEX("Fire Alm:varV.emValue=%d\n",varV.emValue);
	}

	return;
}

void ND_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//ERR_CODE_CAN_R errCode;
	double tmNow = GetCurrentTime();
	int n = 0;
	static bFirstRun = TRUE;

	TRACE("--- ND_Batt_Sample Running --- \n");
	if(JudgeIfNeedReconfig(pSTBatt_Rough))
	{
		ReadAndParseCanData(pSTBatt_Rough);
		TRACE("Here Waitting Batt Reconfig\n");
		return;
	}

	for(n= 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
	{
		if(ABS(tmNow - s_nd_BattRunData[n].t_LastSendC2BStatus) >= 0.5)
		{
			if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
			{
				TRACE("Send_C2B_Status: nPCSIdx =%d\n",n);
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

	//temporary solution
	if(bFirstRun)
	{
		if(ABS(tmNow - pSTBatt_Rough->t_FirstBattBoot) > 20)//ND Batt should wait 20S,then Send Power on Cmd
		{
			for(n= 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
			{
				bFirstRun = FALSE;
				s_nd_BattRunData[n].byPCS_Cmd = 0x01;
				pSTBatt_Rough->bCtrlDO5Close = TRUE;
				CtrlBattCabinetContactor(pSTBatt_Rough->bCtrlDO5Close);
				TRACE("Waiting 20S ,then POWER ON !!!\n");
			}
		}
		return;
	}

	NormalRunJudgeBattCabinetStatu(pSTBatt_Rough);
	return;
}


void ND_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//Ctrl DO5, Break Batt Contactor
	pSTBatt_Rough->bCtrlDO5Close = FALSE;
	CtrlBattCabinetContactor(pSTBatt_Rough->bCtrlDO5Close);
	//---------Init cmd-------------
	TRACE("--- ND_Batt_Init Enter --- \n");
	TAG_ND_BATT_PRO *p = &s_nd_BattPro[0];
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_STATU,	 0x8F,	Parse_B2C_Status 	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_SUMDATA1, 0x90,	Parse_B2C_SumData1	);	
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_SUMDATA2, 0x91,	Parse_B2C_SumData2	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_SUMDATA3, 0x93,	Parse_B2C_SumData3	);
	INIT_NDBATT_CMD(p,	_CAN_R_CMD_B2C_LIMIT,	 0x92,	Parse_B2C_Limit		);

 	//INIT_NDBATT_CMD(p,	_CAN_S_CMD_C2B_STATU,		Send_C2BStatus);

	//---------Init rough-------------
	int i;
	for(i = 0; i < MAX_SINGLE_STBATT_NUM; i++)
	{
		memset(&s_nd_BattRunData[i], 0 ,sizeof(ND_BATT_RUN_DATA));
		
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
	
		s_nd_BattRunData[i].bIsBatt_ChargeForbidden = FALSE;
    	s_nd_BattRunData[i].bIsBatt_DisChargeForbidden = FALSE;

		s_nd_BattRunData[i].mapNDAlarm.byFlag = 0;
		s_nd_BattRunData[i].mapNDFault.byFlag = 0;
		s_nd_BattRunData[i].mapNDProtect.usFlag = 0;
		
    	s_nd_BattRunData[i].usBattModuleQty = 0;
		s_nd_BattRunData[i].byPCS_Cmd = 0x01;

		s_nd_BattRunData[i].t_LastSendC2BStatus = 0;
    	s_nd_BattRunData[i].bFoundReply = FALSE;
		s_nd_BattRunData[i].nInterrupt_Times = 0;
		s_nd_BattRunData[i].bIsCommFail = FALSE;
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
BOOL ND_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt)
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
		s_nd_BattRunData[nWhichBatt].byPCS_Cmd = 0x01;
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_GO_SLEEP)
	{
		TRACE("ND BATT[%d]_GOTO_SLEEP Power OFF\n", nWhichBatt);
		s_nd_BattRunData[nWhichBatt].byPCS_Cmd = 0x02;
		bRtn = TRUE;
	}
	 
	return bRtn;

}
