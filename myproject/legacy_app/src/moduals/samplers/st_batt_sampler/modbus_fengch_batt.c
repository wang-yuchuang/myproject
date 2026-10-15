#include "st_batt_sampler.h"

//#define TRACE	1 ? (void)0 : (void)printf //TRACE("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

static COMMPORT  s_ModbusComm;
static COMMPORT* s_PMdComm = NULL;
static HANDLE 	 s_hRS485Port = NULL;

#define	MDB_SEND_BUF_LEN				16
#define	MDB_RECV_BUF_LEN				256

#define MDB_FUN_CODE_GET_MANY_DATA		0x03
#define MDB_FUN_CODE_SET_ONE_DATA		0x06

#define MDB_MATER_ADDR					0x01

#define START_REG_ADDR_03				0x00
#define SET_REG_ADDR_06					0x05


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

typedef union unStrRUN_State
{
	USHORT usRUN_State;
	struct 
	{
		USHORT Full_Charge		:1;
		USHORT DisCharge_Empty	:1;
		USHORT Contact_Statu	:1;
		USHORT Abnormal_Curr	:1;
		USHORT Sigl_OverVolt	:1;
		USHORT Sigl_LowVolt		:1;
		USHORT Sigl_OverTemp	:1;
		USHORT Sigl_LowTemp		:1;

	}STAT;

}RUN_STATE_BITMAP;


typedef union unStrCtrl_Cmd
{
	USHORT usCtrlCmd;
	struct 
	{
		USHORT OnOrOff				:1;
		USHORT EnbOrDisabChrg		:1;
		USHORT EnbOrDisabDisChrg	:1;

	}STAT;

}CTRL_CMD_BITMAP;

typedef struct tagFengCHBattRunData
{
	float               fPackTotalVolt; //00H
    float               fPackTotalCurr; //01H
    float               fSOC;			//02H
  	USHORT              usSOH;			//03H

	RUN_STATE_BITMAP	mapRUN_State;  //04H
	CTRL_CMD_BITMAP		mapCtrlCmd;  	//05H

	float               fMax_CellVolt;  //06H
    float               fMin_CellVolt;	//07H
	float               fMax_CellTemp;	//08H
    float               fMin_CellTemp;	//09H


	USHORT              usBattRunStatus; //0AH
	USHORT				usOA_Alarm;		 //0BH
	USHORT				usMA_Alarm;		 //0CH
	USHORT				usCA_Alarm;		 //0DH

	float               fAllow_MaxChargeCurr;		//0EH
	float               fAllow_MaxDisChargeCurr;	//0FH
 	float               fAllow_MaxChargePower;  	//10H
	float               fAllow_MaxDisChargePower;	//11H
	
	float 				fAllow_LmtChargeVolt;
	float 				fAllow_LmtDisChargeVolt;

	//S2A_ALARM 
	BYTE				 szS2A_AlarmInfo[36];
	STBATT_SYS_FAULT_MAP mapSysFault;

	
	BYTE				sz_ByMdbSendBuffer[MDB_SEND_BUF_LEN];
	BYTE				sz_ByMdbRecvBuffer[MDB_RECV_BUF_LEN];

    int					bFoundReply;
	int					nInterrupt_Times;
	BOOL				bIsCommFail;
    BOOL				bBattInfo_Regisited; //if the battGrp info NOT registered
	
}FENGCH_BATT_RUN_DATA;

static FENGCH_BATT_RUN_DATA s_FC_BattRunData;


/*==========================================================================*
* FUNCTION : MDB_CRC
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : 
* CREATOR  : 
*==========================================================================*/		 
static BYTE gabyCRCHi[] =
{
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
	0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
	0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x01,0xc0,0x80,0x41,0x00,0xc1,0x81,0x40,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40,0x00,0xc1,0x81,0x40,
	0x01,0xc0,0x80,0x41,0x01,0xc0,0x80,0x41,0x00,0xc1,
	0x81,0x40,0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,
	0x00,0xc1,0x81,0x40,0x01,0xc0,0x80,0x41,0x01,0xc0,
	0x80,0x41,0x00,0xc1,0x81,0x40
};

static BYTE gabyCRCLo[] =
{
	0x00,0xc0,0xc1,0x01,0xc3,0x03,0x02,0xc2,0xc6,0x06,
	0x07,0xc7,0x05,0xc5,0xc4,0x04,0xcc,0x0c,0x0d,0xcd,
	0x0f,0xcf,0xce,0x0e,0x0a,0xca,0xcb,0x0b,0xc9,0x09,
	0x08,0xc8,0xd8,0x18,0x19,0xd9,0x1b,0xdb,0xda,0x1a,
	0x1e,0xde,0xdf,0x1f,0xdd,0x1d,0x1c,0xdc,0x14,0xd4,
	0xd5,0x15,0xd7,0x17,0x16,0xd6,0xd2,0x12,0x13,0xd3,
	0x11,0xd1,0xd0,0x10,0xf0,0x30,0x31,0xf1,0x33,0xf3,
	0xf2,0x32,0x36,0xf6,0xf7,0x37,0xf5,0x35,0x34,0xf4,
	0x3c,0xfc,0xfd,0x3d,0xff,0x3f,0x3e,0xfe,0xfa,0x3a,
	0x3b,0xfb,0x39,0xf9,0xf8,0x38,0x28,0xe8,0xe9,0x29,
	0xeb,0x2b,0x2a,0xea,0xee,0x2e,0x2f,0xef,0x2d,0xed,
	0xec,0x2c,0xe4,0x24,0x25,0xe5,0x27,0xe7,0xe6,0x26,
	0x22,0xe2,0xe3,0x23,0xe1,0x21,0x20,0xe0,0xa0,0x60,
	0x61,0xa1,0x63,0xa3,0xa2,0x62,0x66,0xa6,0xa7,0x67,
	0xa5,0x65,0x64,0xa4,0x6c,0xac,0xad,0x6d,0xaf,0x6f,
	0x6e,0xae,0xaa,0x6a,0x6b,0xab,0x69,0xa9,0xa8,0x68,
	0x78,0xb8,0xb9,0x79,0xbb,0x7b,0x7a,0xba,0xbe,0x7e,
	0x7f,0xbf,0x7d,0xbd,0xbc,0x7c,0xb4,0x74,0x75,0xb5,
	0x77,0xb7,0xb6,0x76,0x72,0xb2,0xb3,0x73,0xb1,0x71,
	0x70,0xb0,0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,
	0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,0x9c,0x5c,
	0x5d,0x9d,0x5f,0x9f,0x9e,0x5e,0x5a,0x9a,0x9b,0x5b,
	0x99,0x59,0x58,0x98,0x88,0x48,0x49,0x89,0x4b,0x8b,
	0x8a,0x4a,0x4e,0x8e,0x8f,0x4f,0x8d,0x4d,0x4c,0x8c,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,
	0x43,0x83,0x41,0x81,0x80,0x40
};

static WORD	MDB_CRC(BYTE *pData, int len)
{
	BYTE i = (BYTE)len;
	BYTE  byCRCHi = 0xff;
	BYTE  byCRCLo = 0xff;
	BYTE  byIdx;
	WORD  crc;

	while(i--)
	{
		byIdx = byCRCHi ^ *pData++;
		byCRCHi = byCRCLo ^ gabyCRCHi[byIdx];
		byCRCLo = gabyCRCLo[byIdx];
	}

	crc = byCRCHi;
	crc <<= 8;
	crc += byCRCLo;

	return crc;
}


static void Close_RS485Port(void)
{
	s_PMdComm->Close(s_hRS485Port);
	s_hRS485Port = NULL;
}

static BOOL ReOpen_RS485Port(void)
{
	//SAMPLER *pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;

	if(s_hRS485Port != NULL)
	{
		Close_RS485Port();
	}

		//Open comm port
	int nErrCode = 0;
	s_hRS485Port = s_PMdComm->Open(
		TTYS_RS485_1,
		"9600,N,8,1",
		COMM_CLIENT_MODE, 1000, &nErrCode);
	if(ERR_COMM_OK != nErrCode)
	{
		TRACE("Error: OPen ttyS3 Failed! nErrCode = [%d]\n", nErrCode);
		return FALSE;
	}

	//Clear buffer
	s_PMdComm->Control(s_hRS485Port,
		COMM_PURGE_RXCLEAR, NULL, 0);

	TRACE("\n!!!-------Open RS485_1::ttyS3 Success-szPortName=[%s]-szPortAttri=[%s]-szPortDriver=[%s]-!!! \n",\
		s_PMdComm->info.szPortName,\
		s_PMdComm->info.szPortAttri,\
		s_PMdComm->info.szPortDriver);
	
	return TRUE;
}


/*==========================================================================*
* FUNCTION : Init_FengCHBatt_RoughData
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS :   
* CREATOR  : 
*==========================================================================*/
void Init_FengCHBatt_RoughData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[0]; //Only One Battery Group
 
 	s_FC_BattRunData.fPackTotalVolt = 0.0; //00H
    s_FC_BattRunData.fPackTotalCurr = 0.0; //01H
    s_FC_BattRunData.fSOC = 0.0;			//02H
  	s_FC_BattRunData.usSOH = 0.0;			//03H

	s_FC_BattRunData.mapRUN_State.usRUN_State = 0.0;  //04H
	s_FC_BattRunData.mapCtrlCmd.usCtrlCmd = 0.0;  	//05H

	s_FC_BattRunData.fMax_CellVolt = 0.0;  //06H
    s_FC_BattRunData.fMin_CellVolt = 0.0;	//07H
	s_FC_BattRunData.fMax_CellTemp = 0.0;	//08H
    s_FC_BattRunData.fMin_CellTemp = 0.0;	//09H

	s_FC_BattRunData.usBattRunStatus = 0.0; //0AH
	s_FC_BattRunData.usOA_Alarm = 0.0;		 //0BH
	s_FC_BattRunData.usMA_Alarm = 0.0;		 //0CH
	s_FC_BattRunData.usCA_Alarm = 0.0;		 //0DH

	s_FC_BattRunData.fAllow_MaxChargeCurr = 0.0;		//0EH
	s_FC_BattRunData.fAllow_MaxDisChargeCurr = 0.0;	//0FH
 	s_FC_BattRunData.fAllow_MaxChargePower = 0.0;  	//10H
	s_FC_BattRunData.fAllow_MaxDisChargePower = 0.0;	//11H
 	s_FC_BattRunData.fAllow_LmtChargeVolt = 0.0;	//Need Calc
    s_FC_BattRunData.fAllow_LmtDisChargeVolt = 0.0;	//Need Calc

	//--- Init Single Battery DataInfo ---//
	pSingleRunInfo->fPackTotalVolt = 0;
	pSingleRunInfo->fPackTotalCurr = 0;
	pSingleRunInfo->nSOC = 0;
	pSingleRunInfo->nSOH = 0;
	
	pSingleRunInfo->emPowerONStatu = 0;
	
	pSingleRunInfo->fMax_CellVolt = s_FC_BattRunData.fMax_CellVolt;  //06H
    pSingleRunInfo->fMin_CellVolt = s_FC_BattRunData.fMin_CellVolt;	//07H
	pSingleRunInfo->fMax_CellTemp = s_FC_BattRunData.fMax_CellTemp;	//08H
    pSingleRunInfo->fMin_CellTemp = s_FC_BattRunData.fMin_CellTemp;	//09H


	pSingleRunInfo->fAllow_LmtChargeVolt = 0;
	pSingleRunInfo->fAllow_LmtDisChargeVolt = 0;
	pSingleRunInfo->fAllow_MaxChargeCurr = 0;
	pSingleRunInfo->fAllow_MaxDisChargeCurr = 0;


	pSingleRunInfo->uiBattCabDevAlm	= 0;

	pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;
	pSingleRunInfo->emLast_BattStatus_Run = STBATT_RUNSTAT_SLEEP;


	return;
}




/*==========================================================================*
* FUNCTION : Modbus_SendCmd
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
*
* Rx:01 03 00 00 00 12 C5 C7
* Tx:01 03 24 00 01 00 02 00 03 00 04 00 05 00 07 00 07 00 08 00 09 00 0A 00 0B 00 0C 00 0D 00 0E 00 0F 00 10 06 A4 07 08 D3 F8

Tx:007220-01 03 00 00 00 0A C5 CD
Rx:007221-01 03 14 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 A3 67

* COMMENTS :  1 Reister == 2 Bytes
    ---------- ------ --------------- ---------
    | header | | DevAddr:01 | Cmd: 03 | | Start Register H8|L8 Byte:00 00 Register Num:00 01 | | CRC H8|L8 |
    ---------- ------ --------------- ---------
	|Response| |DevAddr: 01 | Cmd : 03| | Reply Bytes Num | Data1 | Data2 | DataN| CRC H8|L8 |
* CREATOR  : 
*==========================================================================*/
static BOOL Modbus_SendCmd( BYTE *pBuf, int nDataLen)
{
	//printf("MDB SEND:");
	//int i;
	//for(i = 0; i < nDataLen; i++)
	//{
	//	printf(" %02X", *(pBuf + i));
	//}
	//printf("\n");


	int nWriteBytes = s_PMdComm->Write(s_hRS485Port, pBuf, nDataLen);


	TRACE("INFO:nWriteBytes=[%d]\n", nWriteBytes);
 	//Sleep(200);

	return nWriteBytes == nDataLen;
}

/*==========================================================================*
* FUNCTION : MDB_GetResponse
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : 
* CREATOR  : 
*==========================================================================*/
static int MDB_GetResponse(BYTE *pBuf, int iToRead)
{
	int nReadBytes = s_PMdComm->Read(s_hRS485Port, pBuf,  iToRead);

	TRACE("MDB RECV:");
	int i;
	for(i = 0; i < nReadBytes; i++)
	{
		printf(" %02X", *(pBuf + i));
	}
	TRACE("\n\n");
	TRACE("Rec Data Length = %d\n\n",nReadBytes);

	return nReadBytes;

}

/*==========================================================================*
* FUNCTION : Modbus_SetOneData
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : 
* CREATOR  : 
*==========================================================================*/
int Modbus_SetOneData(IN BYTE nAddr, IN WORD wRegAddr, IN WORD wRegValue)
{
	memset(s_FC_BattRunData.sz_ByMdbSendBuffer, 0, MDB_SEND_BUF_LEN);
    BYTE *pBuf = s_FC_BattRunData.sz_ByMdbSendBuffer;

	BYTE *pSendBuf = pBuf;
	
	*pSendBuf++ = nAddr;
	*pSendBuf++ = MDB_FUN_CODE_SET_ONE_DATA;

	*pSendBuf++ = HIBYTE(wRegAddr);
	*pSendBuf++ = LOBYTE(wRegAddr);

	*pSendBuf++ = HIBYTE(wRegValue);
	*pSendBuf++ = LOBYTE(wRegValue);

	WORD wCRC = MDB_CRC(pBuf, 6);
	*pSendBuf++ = HIBYTE(wCRC);
	*pSendBuf++ = LOBYTE(wCRC);

	Modbus_SendCmd( pBuf, 8);
	Sleep(500);


	int nRecvBytes = 0;

  	memset(s_FC_BattRunData.sz_ByMdbRecvBuffer, 0 , MDB_RECV_BUF_LEN);
    BYTE *pRecvBuf = s_FC_BattRunData.sz_ByMdbRecvBuffer;
	
	MDB_GetResponse(pRecvBuf, MDB_RECV_BUF_LEN);

	// response of set command should be same with send command
	return (strncmp(pBuf, pRecvBuf, 8) == 0) ? MDB_ERROR_OK : MDB_ERROR_FAIL;

}



/*==========================================================================*
* FUNCTION : Parse_Recv03CmdData
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : High Byte First, Low Byte Last
* CREATOR  : #define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))

*==========================================================================*/
static void Parse_Recv03CmdData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE *pData)
{
	s_FC_BattRunData.fPackTotalVolt = (float)MAKEWORD(*(pData + 1), *pData) * 0.1 ; //00H
	pData = pData + 2;

	s_FC_BattRunData.fPackTotalCurr = (float)MAKEWORD(*(pData + 1), *pData) * 0.1 - 1600; //01H
	pData = pData + 2;

	s_FC_BattRunData.fSOC = (float)MAKEWORD(*(pData + 1), *pData) * 0.1;			//02H
	pData = pData + 2;

	s_FC_BattRunData.usSOH = (USHORT)MAKEWORD(*(pData + 1), *pData);			//03H
	pData = pData + 2;

	s_FC_BattRunData.mapRUN_State.usRUN_State = (USHORT)MAKEWORD(*(pData + 1), *pData);  //04H
	pData = pData + 2;

	s_FC_BattRunData.mapCtrlCmd.usCtrlCmd = (USHORT)MAKEWORD(*(pData + 1), *pData);  	//05H
	pData = pData + 2;

	s_FC_BattRunData.fMax_CellVolt = (float)MAKEWORD(*(pData + 1), *pData) * 0.001;  //06H
	pData = pData + 2;

	s_FC_BattRunData.fMin_CellVolt = (float)MAKEWORD(*(pData + 1), *pData) * 0.001;	//07H
	pData = pData + 2;

	s_FC_BattRunData.fMax_CellTemp = (float)MAKEWORD(*(pData + 1), *pData) - 40;	//08H
	pData = pData + 2;

	s_FC_BattRunData.fMin_CellTemp = (float)MAKEWORD(*(pData + 1), *pData) - 40;	//09H
	pData = pData + 2;

	s_FC_BattRunData.usBattRunStatus = (USHORT)MAKEWORD(*(pData + 1), *pData); //0AH
	pData = pData + 2;

	s_FC_BattRunData.usOA_Alarm = (USHORT)MAKEWORD(*(pData + 1), *pData);		 //0BH
	pData = pData + 2;

	s_FC_BattRunData.usMA_Alarm = (USHORT)MAKEWORD(*(pData + 1), *pData);		 //0CH
	pData = pData + 2;

	s_FC_BattRunData.usCA_Alarm = (USHORT)MAKEWORD(*(pData + 1), *pData);		 //0DH
	pData = pData + 2;

	s_FC_BattRunData.fAllow_MaxChargeCurr = (float)MAKEWORD(*(pData + 1), *pData);		//0EH
	pData = pData + 2;

	s_FC_BattRunData.fAllow_MaxDisChargeCurr = (float)MAKEWORD(*(pData + 1), *pData);	//0FH
	pData = pData + 2;

	s_FC_BattRunData.fAllow_MaxChargePower = (float)MAKEWORD(*(pData + 1), *pData);  	//10H
	pData = pData + 2;

	s_FC_BattRunData.fAllow_MaxDisChargePower = (float)MAKEWORD(*(pData + 1), *pData);	//11H
	//pData = pData + 2;

	s_FC_BattRunData.fAllow_LmtChargeVolt =	s_FC_BattRunData.fAllow_MaxChargePower/(s_FC_BattRunData.fAllow_MaxChargeCurr+1); //Need Calc
	s_FC_BattRunData.fAllow_LmtDisChargeVolt = s_FC_BattRunData.fAllow_MaxDisChargePower/(s_FC_BattRunData.fAllow_MaxDisChargeCurr+1);	//Need Calc

	sprintf(s_FC_BattRunData.szS2A_AlarmInfo, "%04X|%04X|%04X",s_FC_BattRunData.usOA_Alarm, s_FC_BattRunData.usMA_Alarm, s_FC_BattRunData.usCA_Alarm);

	TRACE("fPackTotalVolt=[%.1f] fPackTotalCurr=[%.1f] fSOC=[%.1f] usSOH=[%d] usRUN_State=[%d] usCtrlCmd=[%d]\
	fMax_CellVolt=[%.1f] fMin_CellVolt=[%.1f] fMax_CellTemp=[%.1f] fMin_CellTemp=[%.1f]\
	usBattRunStatus=[%d] usOA_Alarm=[%d]  usMA_Alarm=[%d]  usCA_Alarm=[%d] \
	fAllow_MaxChargeCurr=[%.1f] fAllow_MaxDisChargeCurr=[%.1f] fAllow_MaxChargePower=[%.1f] fAllow_MaxDisChargePower=[%.1f] \n",\
	s_FC_BattRunData.fPackTotalVolt,
    s_FC_BattRunData.fPackTotalCurr,
    s_FC_BattRunData.fSOC,
  	s_FC_BattRunData.usSOH,
	s_FC_BattRunData.mapRUN_State.usRUN_State,
	s_FC_BattRunData.mapCtrlCmd.usCtrlCmd,
	s_FC_BattRunData.fMax_CellVolt,
    s_FC_BattRunData.fMin_CellVolt,
	s_FC_BattRunData.fMax_CellTemp,
    s_FC_BattRunData.fMin_CellTemp,
	s_FC_BattRunData.usBattRunStatus,
	s_FC_BattRunData.usOA_Alarm,
	s_FC_BattRunData.usMA_Alarm,
	s_FC_BattRunData.usCA_Alarm,
	s_FC_BattRunData.fAllow_MaxChargeCurr,
	s_FC_BattRunData.fAllow_MaxDisChargeCurr,
 	s_FC_BattRunData.fAllow_MaxChargePower,
	s_FC_BattRunData.fAllow_MaxDisChargePower);

	return;
}

/*==========================================================================*
* FUNCTION : Modbus_GetManyData_DWORD
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : 
* CREATOR  : 
*==========================================================================*/
int	Modbus_GetManyData_DWORD(IN STBATT_SAMPLER_ROUGH *pSTBatt_Rough,
			IN BYTE nAddr,
			IN WORD wRegStart, 
			IN WORD wDataSize)
{
	memset(s_FC_BattRunData.sz_ByMdbSendBuffer, 0, MDB_SEND_BUF_LEN);

    BYTE *pBuf = s_FC_BattRunData.sz_ByMdbSendBuffer;
    BYTE *pSendBuf = pBuf;

    *pSendBuf++ = nAddr;

    *pSendBuf++ = MDB_FUN_CODE_GET_MANY_DATA;

    *pSendBuf++ = HIBYTE(wRegStart);
    *pSendBuf++ = LOBYTE(wRegStart);

    *pSendBuf++ = HIBYTE(wDataSize);// HIBYTE(wDataSize*2);
    *pSendBuf++ = LOBYTE(wDataSize);// LOBYTE(wDataSize*2);
    WORD wCRC = MDB_CRC(pBuf, 6);
    *pSendBuf++ = HIBYTE(wCRC);
    *pSendBuf++ = LOBYTE(wCRC);

    
    Modbus_SendCmd(pBuf, 8);

    // receive

  	memset(s_FC_BattRunData.sz_ByMdbRecvBuffer, 0 , MDB_RECV_BUF_LEN);

    BYTE *pRecvBuf = s_FC_BattRunData.sz_ByMdbRecvBuffer;
    int  nRecvBytes = MDB_GetResponse(pRecvBuf, MDB_RECV_BUF_LEN);

	TRACE("\nReceived Bytes = %d: ",nRecvBytes );
	int j = 0;
	for(j = 0; j < nRecvBytes; j++)
	{
		printf("%02X ",*(pRecvBuf+j));
	}

    if(nRecvBytes < 7)
    {
		TRACE("Error: nRecvBytes = [%d]  < 7\n", nRecvBytes);
	    return MDB_ERROR_RESPOND_DATA_LEN;
    }

    // Check Address
    if(*pRecvBuf != nAddr)
    {
		TRACE("Error: Wrong RecvAddr=[%d] Send_Addr=[%d]\n", *pRecvBuf, nAddr);
	    return MDB_ERROR_ADDR;
    }


    // Check Function Code
    if(*(pRecvBuf + 1) != MDB_FUN_CODE_GET_MANY_DATA)
    {
		TRACE("Error: Wrong Check Function Code=[%d]. \n" , *(pRecvBuf + 1));
	    return MDB_ERROR_FUN_CODE;
    }

    // Check Data Length
    BYTE byDataLen = *(pRecvBuf + 2);

    if (nRecvBytes < byDataLen + 5)
    {
		TRACE("Error:nRecvBytes=[%d] byDataLen+5=[%d]\n", nRecvBytes, byDataLen+5);
	    return MDB_ERROR_RESPOND_DATA_LEN;
    }

    // Check CRC
    int iCrcStart = 3 + (int)byDataLen;
    WORD wReadCRC = MAKEWORD(*(pRecvBuf + iCrcStart + 1), *(pRecvBuf + iCrcStart));
    WORD wCalcCRC = MDB_CRC(pRecvBuf, byDataLen + 3);

    if(wReadCRC != wCalcCRC)
    {
	    TRACE("Error: CRC Error! Calc CRC = %04X\n", wCalcCRC);
	    return MDB_ERROR_CRC;
    }

    if(byDataLen !=(wDataSize * 2))
    {
		TRACE("Error: byDataLen=[%d] wDataSize * 2=[%d]\n", byDataLen, wDataSize * 2);
	    return MDB_ERROR_RESPOND_DATA_LEN;
    }

   int i;
   BYTE *pData = pRecvBuf + 3;

   Parse_Recv03CmdData(pSTBatt_Rough, pData);


    return MDB_ERROR_OK;
}

/*==========================================================================*
* FUNCTION : Map_OrgBattData2Blob
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : 
* CREATOR  : 
*==========================================================================*/
static void Map_OrgBattData2Blob(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[0];


	//--- Init Single Battery DataInfo ---//
	pSingleRunInfo->fPackTotalVolt = s_FC_BattRunData.fPackTotalVolt;
	pSingleRunInfo->fPackTotalCurr = s_FC_BattRunData.fPackTotalCurr;
	pSingleRunInfo->nSOC = (UINT)s_FC_BattRunData.fSOC;
	pSingleRunInfo->nSOH = (UINT)s_FC_BattRunData.usSOH;
	
	pSingleRunInfo->emPowerONStatu = (s_FC_BattRunData.mapRUN_State.STAT.Contact_Statu);
	
	pSingleRunInfo->fMax_CellVolt = s_FC_BattRunData.fMax_CellVolt; 
    pSingleRunInfo->fMin_CellVolt = s_FC_BattRunData.fMin_CellVolt;	
	pSingleRunInfo->fMax_CellTemp = s_FC_BattRunData.fMax_CellTemp;	
    pSingleRunInfo->fMin_CellTemp = s_FC_BattRunData.fMin_CellTemp;	


	pSingleRunInfo->fAllow_LmtChargeVolt =		s_FC_BattRunData.fAllow_LmtChargeVolt;
	pSingleRunInfo->fAllow_LmtDisChargeVolt = 	s_FC_BattRunData.fAllow_LmtDisChargeVolt;
	pSingleRunInfo->fAllow_MaxChargeCurr = 		s_FC_BattRunData.fAllow_MaxChargeCurr;
	pSingleRunInfo->fAllow_MaxDisChargeCurr = 	s_FC_BattRunData.fAllow_MaxDisChargeCurr;

	pSingleRunInfo->uiBattCabDevAlm = s_FC_BattRunData.mapSysFault.uiFlag; 
	memcpy(pSingleRunInfo->szS2A_AlarmInfo, s_FC_BattRunData.szS2A_AlarmInfo, sizeof(pSingleRunInfo->szS2A_AlarmInfo));


	return;
}

/*==========================================================================*
* FUNCTION : FengCH_Map_BattRawData_2Rough
* PURPOSE  : 
* CALLS    : 
* CALLED BY: 
* ARGUMENTS: 
* RETURN   : 
* COMMENTS : 
* CREATOR  : 
*==========================================================================*/
static void FengCH_Map_BattRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	A_BLOB_GROUP_STBATT_RUNINFO* pGrpRunInfo = &pSTBatt_Rough->blobGroupBattInfo;
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[0];

	int nOnlineTotal = 0;
	
	if(pSTBatt_Rough->nSystemCfgBattNum == 0)
	{
		ZERO_POBJS(pSingleRunInfo, 1);
		pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_NOT_INSTALLED;
		TRACEX("Skip Single Battery, No Battery Configed !!\n");
		return;
	}

	if((s_FC_BattRunData.bIsCommFail) ||
		(!s_FC_BattRunData.bBattInfo_Regisited))
	{
		TRACEX("BattIdx=[%d] bIsCommFail = %d, bBattInfo_Regisited = %d\n", n, s_FC_BattRunData.bIsCommFail, s_FC_BattRunData.bBattInfo_Regisited);
		
		Init_FengCHBatt_RoughData(pSTBatt_Rough);
 		
		pGrpRunInfo->nTotalOnlineBatt = 0;
		pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;

		return;
	}

	nOnlineTotal++;
	Map_OrgBattData2Blob(pSTBatt_Rough);

	//Now handle status
	switch (s_FC_BattRunData.usBattRunStatus)//0-Idle, 1-Charge, 2-Discharging
	{
	case 0: //Normal Statu
		{
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
			if(s_FC_BattRunData.mapRUN_State.STAT.Full_Charge == 1)
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_FULL_CHARGE;
				pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			}
			else if(s_FC_BattRunData.mapRUN_State.STAT.DisCharge_Empty == 1)
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_FULL_DISCHARGE;
				pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			}
			else if(s_FC_BattRunData.mapRUN_State.STAT.Contact_Statu == 0)
			{
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
				pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
				pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			}
			else if((s_FC_BattRunData.mapRUN_State.STAT.Abnormal_Curr == 1) ||
				(s_FC_BattRunData.mapRUN_State.STAT.Sigl_OverVolt == 1) ||
				(s_FC_BattRunData.mapRUN_State.STAT.Sigl_LowVolt == 1) ||
				(s_FC_BattRunData.mapRUN_State.STAT.Sigl_OverTemp == 1)||
				(s_FC_BattRunData.mapRUN_State.STAT.Sigl_LowTemp == 1))
			{
				pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_FAULT;
 				pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
				pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
				pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			}
		}
		break;
	case 1:
		pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_CHARGE;
  		break;
	case 2:
		pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_DISCHARGE;
 		break;

	default:
		{
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
		}
		break;
	}
	//alarm status

	if((s_FC_BattRunData.usCA_Alarm > 0) || 
	   (s_FC_BattRunData.mapSysFault.uiFlag > 0))
	{
		pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_FAULT;
		//must set status into sleep in case BMS not set
		pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
		pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
		pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
		TRACE("Alarm:mapNDFault=[%d] mapSysFault=[%d]\n",\
			s_FC_BattRunData.usCA_Alarm, s_FC_BattRunData.mapSysFault.uiFlag );
	}
	else if(s_FC_BattRunData.usMA_Alarm > 0)
	{
		if( (fabs(pSingleRunInfo->fAllow_MaxDisChargeCurr) < 5.0) &&
            (fabs(pSingleRunInfo->fAllow_MaxChargeCurr) < 5.0) )   
        {
            pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_ALARMING;
            pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
            pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
            //must set status into sleep in case BMS not set
            pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
        }
        else
        {
            pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
        }
		TRACE("pSingleRunInfo->emBattStatus_Alm  =%d\n", pSingleRunInfo->emBattStatus_Alm);
	}
	else if(pSingleRunInfo->nSOC <= (pSTBatt_Rough->nProtectSOC + pSTBatt_Rough->nPVSysBattRetSOC))
	{
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

	TRACEX("nBattIdx=[%d] usBattRunStatus=[%02X] \
		->emBattStatus_Alm =%d emBattStatus_Run=%d \
		mapSysFault.uiFlag=[%d]\n", \
 		n, s_FC_BattRunData.usBattRunStatus, \
		pSingleRunInfo->emBattStatus_Alm, \
		pSingleRunInfo->emBattStatus_Run, \
		s_FC_BattRunData.mapSysFault.uiFlag);

	pSingleRunInfo->emLast_BattStatus_Run = pSingleRunInfo->emBattStatus_Run;

	pGrpRunInfo->nTotalOnlineBatt = nOnlineTotal;

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  FengCH_Batt_Sample
*  CREATOR       :  
*  CREATED DATE  : 2019-12-12 19:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void FengCH_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int nResult = MDB_ERROR_OTHER;

	nResult = Modbus_GetManyData_DWORD(pSTBatt_Rough, MDB_MATER_ADDR, START_REG_ADDR_03, 18);
	if(nResult != MDB_ERROR_OK)
	{
	    TRACEX("MDB Sample Result =[%d]", nResult);
		s_FC_BattRunData.nInterrupt_Times++;
		if(s_FC_BattRunData.nInterrupt_Times >= 10)
		{
			s_FC_BattRunData.nInterrupt_Times = 0;
			s_FC_BattRunData.bIsCommFail = TRUE;
			s_FC_BattRunData.bBattInfo_Regisited = FALSE; 

			Init_FengCHBatt_RoughData(pSTBatt_Rough);
			
			ReOpen_RS485Port();
		}
		// return ;
	}
	else
	{
		s_FC_BattRunData.bFoundReply = TRUE;
		s_FC_BattRunData.nInterrupt_Times = 0;
		s_FC_BattRunData.bIsCommFail = FALSE;
		s_FC_BattRunData.bBattInfo_Regisited = TRUE; 
	}

	Sleep(200);

	//John Don't Known The Reality Logic From BMS Protocal. Temperaly Process as Below.
	if(s_FC_BattRunData.mapRUN_State.STAT.Contact_Statu == 1) // DC Contactor Connected
	{
		if((s_FC_BattRunData.mapCtrlCmd.STAT.OnOrOff == 0) || 
			(s_FC_BattRunData.mapCtrlCmd.STAT.EnbOrDisabChrg == 0) || 
			(s_FC_BattRunData.mapCtrlCmd.STAT.EnbOrDisabDisChrg == 0)) 
		{
			Modbus_SetOneData(MDB_MATER_ADDR, SET_REG_ADDR_06, 0x07);
		}
	}
	
	FengCH_Map_BattRawData_2Rough(pSTBatt_Rough);

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
void FengCH_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->nCtrlTimes[n] = 0;
 		pSTBatt_Rough->nPowerONTimes[n] = 0;
	}

 	GetCommPort_ById(pSTBatt_Rough->hSampler, 3, &s_ModbusComm);
	
	s_PMdComm = &s_ModbusComm;

	TRACE("\n!!!---444---szPortName=[%s]-szPortAttri=[%s]-szPortDriver=[%s]-!!! \n",\
		s_PMdComm->info.szPortName,\
		s_PMdComm->info.szPortAttri,\
		s_PMdComm->info.szPortDriver);


	ReOpen_RS485Port();

	//--------- Init RoughData-------------//
	Init_FengCHBatt_RoughData(pSTBatt_Rough);
	s_FC_BattRunData.bFoundReply = FALSE;
	s_FC_BattRunData.nInterrupt_Times = 0;
	s_FC_BattRunData.bIsCommFail = TRUE;
    s_FC_BattRunData.bBattInfo_Regisited = FALSE; 

	TRACE("--- FengCH_Init Success !!! --- \n");

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  FengCH_Batt_Control
*  CREATOR       :  
*  CREATED DATE  : 2019-12-12 19:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL FengCH_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt)
{
	BOOL bRtn = FALSE;
	//TRACE("-----Now received cmd[%d] for ND Batt[%d], bIscommFail = %d--------\n", emCmdTp, nWhichBatt, s_FC_BattRunData[nWhichBatt].bIsCommFail);
	
	return bRtn;
}
