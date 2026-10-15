
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include<time.h>
#include<string.h>

//gcc -o multi_Power_Share multi_Power_Share.c  -l pthread 

#define TRUE            	1
#define FALSE           	0

#define                 	IN 
#define                 	OUT

typedef unsigned int    	BOOL;
typedef unsigned int    	UINT;
typedef unsigned char   	BYTE;
typedef unsigned int    	ULONG;
typedef	 unsigned short		USHORT;
typedef	 short				SHORT;

#define TRACEX   			printf

#define isTimeOut2Send(tmNow,tmLast,TimeOut)   (abs(tmNow - tmLast) > TimeOut) ? TRUE : FALSE
#define ZERO_POBJS(p, n)	memset((p), 0, (n)*sizeof(*(p)))


#define  INIT_CANMSG_CMD(p, _emMsgType, _emFuncCode, _nSubIdx, _pfnHandler, _bCrcCheck) \
	((p)->emMsgType = (_emMsgType), \
	(p)->byCmdType = (_emFuncCode), \
	(p)->byFuncID = (_nSubIdx), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)->bCrcCheck = (_bCrcCheck), \
	(p)++);

 

void* _Thread_Master_Addr1(void* arg);   
void* _Thread_Slave_Addr2(void* arg);   
 

//----------------- Below is Defined For Slave MsgType Start---------------//
#define CANMSG_FRAME_LEN		  		12
#define CANMSG_DATA_AREA_LEN			8

typedef enum emSlvCanMsgCmd
{
	_CAN_R_CMD_M2S_STATUS = 0,
	_CAN_R_CMD_M2S_GRIDMETER,

	_CAN_R_CMD_SLV_MAX

}SLV_CMD_MSGTYPE;

typedef enum emMstCanMsgCmd
{
	_CAN_W_CMD_M2S_STATUS = 0,
	_CAN_W_CMD_M2S_GRIDWH_POWER,
	_CAN_W_CMD_M2S_GRIDWH_VOLT,
	_CAN_W_CMD_M2S_GRIDWH_CURT,

	_CAN_W_CMD_MST_MAX

}MST_CMD_MSGTYPE;
 
typedef union SystemStatus
{
	BYTE		byData[2];
	struct
	{
		BYTE Avaliable:1;
		BYTE AutoOrManual:1;
		BYTE OnOrOffGrid:1;
		 
	}Val; 

}UNION_SYS_STATUS;

typedef union tagCanDataArea
{
	BYTE					byData[CANMSG_DATA_AREA_LEN];
	struct
	{
		float 				fPgrid;
		UNION_SYS_STATUS 	unionSysStatu;
		BYTE 				byGridWhComm;
		BYTE 				byCRC;
	}M2S_Pgrid_Sum;

	struct
	{
		float 				fPaux1;
		BYTE 				Resv;
		BYTE 				byAux1WhComm;
		BYTE 				byCRC;
	}M2S_Paux1_Sum;	 

	struct
	{
		float 				fPaux3;
		BYTE 				byResv;
		BYTE   				byAux3WhComm;
		BYTE 				byCRC;
	}M2S_Paux3_Sum;	 

	struct
	{
		SHORT 				sQgrid_Sum;
		SHORT 				sQgrid_PhA;
		SHORT 				sQgrid_PhB;
		SHORT 				sQgrid_PhC;
	}M2S_Qgrid_Total;

	struct
	{
		SHORT				sQaux1_Sum;
		SHORT				sQaux1_PhA;
		SHORT				sQaux1_PhB;
		SHORT				sQaux1_PhC;
	}M2S_Qaux1_Total;	 

	struct
	{
		SHORT 				sQaux3_Sum;
		SHORT 				sQaux3_PhA;
		SHORT 				sQaux3_PhB;
		SHORT 				sQaux3_PhC;
	}M2S_Qaux3_Total;	

	struct
	{
		SHORT 		 		sPgrid_PhA;
		SHORT 				sPgrid_PhB;
		SHORT 				sPgrid_PhC;
		SHORT 				sRESV;
	}M2S_Pgrid_3Phase;
	
	struct
	{
		SHORT 				sPaux1_PhA;
		SHORT 				sPaux1_PhB;
		SHORT 				sPaux1_PhC;
		SHORT 				sRESV;
	}M2S_Paux1_3Phase;

	struct
	{
		SHORT 				sPaux3_PhA;
		SHORT 				sPaux3_PhB;
		SHORT 				sPaux3_PhC;
		SHORT 				sRESV;
	}M2S_Paux3_3Phase;

	struct
	{
		SHORT 				sPFgrid_Sum;
		SHORT 				sPFgrid_PhA;
		SHORT 				sPFgrid_PhB;
		SHORT 				sPFgrid_PhC;
	}M2S_PFgrid_Total;

	struct
	{
		USHORT 				usVphA_Grid;
		USHORT 				usVphB_Grid;
		USHORT 				usVphC_Grid;
		USHORT 				usResv;
	}M2S_Vgrid_3Phase;

	struct
	{
		SHORT 				sIphA_Grid;
		SHORT 				sIphB_Grid;
		SHORT 				sIphC_Grid;
		SHORT 				usResv;
	}M2S_Igrid_3Phase;

	struct
	{
		float 				Kwh_Pos;
		float 				Kwh_Neg;
	}M2S_Kwhgrid_Total;

	struct
	{	
		float 				fPcab;
		BYTE				byResv;
		BYTE				byCabWhComm;
		BYTE 				byCRC;
	}S2M_Pcab_WH;

	struct
	{		
		USHORT 				usVphA_Cab;
		USHORT 				usVphB_Cab;
		USHORT 				usVphC_Cab;
		USHORT 				usResv;
	}S2M_Vcab_WH;

	struct
	{
		SHORT 				sIphA_Cab;
		SHORT 				sIphB_Cab;
		SHORT 				sIphC_Cab;
		SHORT 				sResv;
	}S2M_Icab_WH;

	struct
	{ 	
		float 				fKwh_Pos;
		float 				fKwh_Neg;
	}S2M_Kwhcab_WH;	

}CAN_DATA_AREA;

typedef struct tagMeterValueTable
{
	//must always put first!!!
	//BLOB_TYPE		emBlobType;

	float 	fRealTime_ActivePower;
	float 	fPhA_ActivePower;
	float 	fPhB_ActivePower;
	float 	fPhC_ActivePower;

	float 	fReactivePower;
	float 	fPhA_ReactivePower;
	float 	fPhB_ReactivePower;
	float 	fPhC_ReactivePower;

	float 	fRealTime_kWh;
	float 	fRealTime_kVarh1;

	float 	fPhA_Volt;
	float 	fPhA_Curr;

	float 	fPhB_Volt;
	float 	fPhB_Curr;

	float 	fPhC_Volt;
	float 	fPhC_Curr;

	float 	fRealTime_kVarh2;

 	float 	fReverse_kWh;

 	float 	fAC_frequency;

	float 	fPmaxDemand;

	float 	fPower_Factor;
	float 	fPhA_PowerFactor;
	float 	fPhB_PowerFactor;
	float 	fPhC_PowerFactor;
	 
}A_BLOB_METER_VALUES;


typedef struct tagA_CanMsgFrame
{
	BYTE 			byRawData[CANMSG_FRAME_LEN];
	
	MST_CMD_MSGTYPE emMsgType;

 	BYTE  			byCmdType;

	BYTE  			byFuncID;

    BYTE    		byDestAddr;    

	BYTE 			bySrcAddr;
 
	//UINT 			uiFrmID; 								//Send CAN ID

	//UINT 			uiDataLen; 								//Send DLC

	CAN_DATA_AREA   unionDataArea;

	BOOL    		bIsCtrlCmd;     						// if true, need send urgently 

}A_CAN_MSG_FRAME;

typedef BOOL (*SLV_CAN_SAMPLE_PROC)(IN void *argv, A_CAN_MSG_FRAME *pFrmReply);
typedef BOOL (*MST_CAN_SAMPLE_PROC)(IN void *argv, A_CAN_MSG_FRAME *pFrmSend);

typedef struct tagCanMsgPro
{
	SLV_CMD_MSGTYPE 			emMsgType;
	int  						byCmdType;
	int  						byFuncID;

	SLV_CAN_SAMPLE_PROC			pfnCmdHandler;

	BOOL						bCrcCheck;

}SLAVE_CANMSG_PRO;


typedef struct tagMstCanPro
{
	MST_CMD_MSGTYPE 			emMsgType;
	int  						byCmdType;
	int  						byFuncID;

	MST_CAN_SAMPLE_PROC			pfnCmdHandler;

	BOOL 						bCrcCheck;

}MASTER_CANMSG_PRO;
//----------------------------------- End ---------------------------------//

#define MAX_RING_BUFF		10
#define MAX_CAN_MSG_LENTH	12
typedef struct 
{
	A_CAN_MSG_FRAME 		stCanFrame;

	BOOL 					bRead;
}A_CAN_RINGBUFFER;

typedef struct stPowerShareRoughData
{
	//int                 	nIdx;

    BOOL                	bNeedExitTrdAddr1;

    BOOL                	bNeedExitTrdAddr2;

	SLAVE_CANMSG_PRO 		stSlaveCanMsg[_CAN_R_CMD_SLV_MAX];

	MASTER_CANMSG_PRO 		stMasterCanMsg[_CAN_W_CMD_MST_MAX];

	time_t 					tm_LastSend[_CAN_W_CMD_MST_MAX];

	A_BLOB_METER_VALUES		blobGridWh;

	A_BLOB_METER_VALUES		blobCabWh;

	A_CAN_RINGBUFFER		stCanMsgBuff[MAX_RING_BUFF];

	//CAN_DATA_AREA			stCanDataArea;

}PWRSHARE_SAMP_ROUGH;


//--------------------------- Public ------------------------------//
///Crc_ComputeCRC8- According SAE J1850  Calculate CRC Value
static BYTE Crc_ComputeCRC8(const USHORT Msg_Id, const BYTE* Crc_DataPtr, USHORT Crc_Length)
{
	// SAE J1850 crc lookup table 
	static const BYTE Crc8Table[256] =
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
	USHORT count; // loop counter for CRC calculation 
	BYTE crc_value; // CRC initial value 
	BYTE XOR_out; // Value to XOR output 
	crc_value = 0xffu;
	XOR_out = 0xffu;
	crc_value = Crc8Table[crc_value ^ (BYTE)(Msg_Id & 0x00ffu)]; // Calculate CRC of low byte of ApplicationId 
	crc_value = Crc8Table[crc_value ^ (BYTE)(Msg_Id >> 8)]; // Calculate CRC of high byte of ApplicationId 
	for (count = 0; count < (USHORT)Crc_Length; count++) // Calculate CRC of data block 
	{
		crc_value = Crc8Table[crc_value ^ Crc_DataPtr[count]];
	}
	crc_value = crc_value ^ XOR_out;
	// Calculate the official checksum.
	return crc_value;
}


//----------------------- Slave Func Start ------------------//

static BOOL Parse_M2S_Status(IN void *argv, A_CAN_MSG_FRAME *pFrmReply)
{
	printf("[--- Parse_M2S_Status ---]\n");

	return TRUE;
}

static BOOL Parse_M2S_GridMeter(IN void *argv, A_CAN_MSG_FRAME *pFrmReply)
{
	printf("[--- Parse_M2S_GridMeter ---]\n");

	return TRUE;
}
//----------------------- SLV End -----------------------------//

//----------------------- Master Start ------------------------//
static BOOL Send_Sys_Status(IN void *argv, A_CAN_MSG_FRAME *pFrmSend)
{
	printf("[--- Send_Sys_Status ---]\n");

	return TRUE;
}


static UINT Find_RingBuffPosition(IN PWRSHARE_SAMP_ROUGH *pWhRough)
{
	UINT n = 0;
	for(n = 0; n < MAX_RING_BUFF; n++)
	{
		if(pWhRough->stCanMsgBuff[n].bRead == TRUE)
		{
			break;
		}
	}
	return n;
}

static void CAN_StuffMsgID(BYTE byCmdType, BYTE bySubIdx, BYTE byDestAddr, BYTE bySrcAddr, UINT uiDataLen, BYTE *pbyFrame)
{
	//*pbyFrame = (BYTE)uiDataLen;
	*(pbyFrame + 0) = byCmdType;
	*(pbyFrame + 1) = bySubIdx;
	*(pbyFrame + 2) = byDestAddr;
	*(pbyFrame + 3) = bySrcAddr;

	//TRACE("Info:CAN_StuffMsgID--- %02x %02x %02x %02x ---\n",pbyFrame[1],pbyFrame[2],pbyFrame[3],pbyFrame[4]);
	return ;
}

static BOOL Send_GridWh_Power(IN void *argv, A_CAN_MSG_FRAME *pFrmSend)
{	
	PWRSHARE_SAMP_ROUGH *pWhRough = (PWRSHARE_SAMP_ROUGH*)argv;

	printf("[Send_GridWh_Power]:TM=[%ld]\n", pWhRough->tm_LastSend[_CAN_W_CMD_M2S_GRIDWH_POWER]);

	UINT n = Find_RingBuffPosition(pWhRough);

	if(n >= MAX_RING_BUFF)
	{
		return FALSE;
	}


 	BYTE byCRC;
	BYTE *pbySendBuf;
	pbySendBuf = pWhRough->stCanMsgBuff[n].stCanFrame.byRawData;

 	CAN_StuffMsgID(pFrmSend->byCmdType, pFrmSend->byFuncID, pFrmSend->byDestAddr, pFrmSend->bySrcAddr, CANMSG_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[0]<<24) + (pbySendBuf[1]<<16) + (pbySendBuf[2]<<8) + pbySendBuf[3];


	pWhRough->stCanMsgBuff[n].bRead = FALSE;
	
	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.fPgrid =
		pWhRough->blobGridWh.fRealTime_ActivePower;

	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.unionSysStatu.Val.Avaliable = 1;
	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.unionSysStatu.Val.AutoOrManual = 0;
	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.unionSysStatu.Val.OnOrOffGrid = 1;

	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.byGridWhComm = 0x05;

	memcpy(&pFrmSend->unionDataArea.byData[0], pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.byData, 7);

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &pFrmSend->unionDataArea.byData[0], 7);

	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.byCRC = byCRC;

	memcpy(pbySendBuf + 4, pFrmSend->unionDataArea.byData, CANMSG_DATA_AREA_LEN);//

	printf("nMsgID=[%d] byCRC=[%02X]\n", nMsgID, byCRC); 

	for(n = 0; n < 12; n++)
	{
		printf("%02X-",pWhRough->stCanMsgBuff[n].stCanFrame.byRawData[n]);
	}

	printf("\n\n");
 
	return TRUE;
}

static BOOL Send_GridWh_Volt(IN void *argv, A_CAN_MSG_FRAME *pFrmSend)
{
	printf("[--- Send_GridWh_Volt ---]\n");

	return TRUE;
}

static BOOL Send_GridWh_Curt(IN void *argv, A_CAN_MSG_FRAME *pFrmSend)
{
	printf("[--- Send_GridWh_Curt ---]\n");
	PWRSHARE_SAMP_ROUGH *pWhRough = (PWRSHARE_SAMP_ROUGH*)argv;

	printf("[Send_GridWh_Power]:TM=[%ld]\n", pWhRough->tm_LastSend[_CAN_W_CMD_M2S_GRIDWH_POWER]);

	UINT n = Find_RingBuffPosition(pWhRough);

	if(n >= MAX_RING_BUFF)
	{
		return FALSE;
	}


 	BYTE byCRC;
	BYTE *pbySendBuf;
	pbySendBuf = pWhRough->stCanMsgBuff[n].stCanFrame.byRawData;

 	CAN_StuffMsgID(pFrmSend->byCmdType, pFrmSend->byFuncID, pFrmSend->byDestAddr, pFrmSend->bySrcAddr, CANMSG_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[0]<<24) + (pbySendBuf[1]<<16) + (pbySendBuf[2]<<8) + pbySendBuf[3];

	pWhRough->stCanMsgBuff[n].bRead = FALSE;
	
	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.sIphA_Grid = -111;
 	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.sIphB_Grid =  112;
	pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.sIphC_Grid = -113;

	memcpy(pbySendBuf + 4, pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.byData, CANMSG_DATA_AREA_LEN);

	for(n = 0; n < 12; n++)
	{
		printf("%02X-",pWhRough->stCanMsgBuff[n].stCanFrame.byRawData[n]);
	}

	printf("\n\n");

	return TRUE;
}
//------------------------------- Master End -----------------------------//


 
static void Master_RefreshGridMeterValue(IN PWRSHARE_SAMP_ROUGH *pWhRough)
{
 	pWhRough->blobGridWh.fRealTime_ActivePower 	= 100 + rand()%100;
	pWhRough->blobGridWh.fPhA_ActivePower		= 31;
	pWhRough->blobGridWh.fPhB_ActivePower		= 32;
	pWhRough->blobGridWh.fPhC_ActivePower		= 33;

	pWhRough->blobGridWh.fReactivePower			= -40 + rand()%100;
	pWhRough->blobGridWh.fPhA_ReactivePower		= -11;
	pWhRough->blobGridWh.fPhB_ReactivePower		= -12;
	pWhRough->blobGridWh.fPhC_ReactivePower		= -13;

	pWhRough->blobGridWh.fRealTime_kWh			= 9999;
	pWhRough->blobGridWh.fRealTime_kVarh1		= -8888;

	pWhRough->blobGridWh.fPhA_Volt				= 221;
	pWhRough->blobGridWh.fPhA_Curr				= 111;

	pWhRough->blobGridWh.fPhB_Volt				= 222;
	pWhRough->blobGridWh.fPhB_Curr				= 112;

	pWhRough->blobGridWh.fPhC_Volt				= 223;
	pWhRough->blobGridWh.fPhC_Curr				= 113;

	pWhRough->blobGridWh.fRealTime_kVarh2		= 6666;
 	pWhRough->blobGridWh.fReverse_kWh			= -7777;

 	pWhRough->blobGridWh.fAC_frequency			= 50.1;
	pWhRough->blobGridWh.fPmaxDemand			= 5555;

	pWhRough->blobGridWh.fPower_Factor			= 0.98;
	pWhRough->blobGridWh.fPhA_PowerFactor		= 1.0;
	pWhRough->blobGridWh.fPhB_PowerFactor		= 0.98;	
	pWhRough->blobGridWh.fPhC_PowerFactor		= -0.97;

	return;
}

static void init_FrmSend(MASTER_CANMSG_PRO *pstMasterCanMsg, A_CAN_MSG_FRAME *pFrmSend)
{
	if(pFrmSend == NULL)
	{
		printf("Error: (pFrmSend == NULL\n");
		return;
	}
	memset(pFrmSend, 0 , sizeof(A_CAN_MSG_FRAME));
	pFrmSend->byCmdType = pstMasterCanMsg->byCmdType;
	pFrmSend->byFuncID = pstMasterCanMsg->byFuncID;
    pFrmSend->byDestAddr = 0xFF;    
	pFrmSend->bySrcAddr  = 0x01;

	return;
}

void* _Thread_Master_Addr1(void* arg)
{
    PWRSHARE_SAMP_ROUGH *pWhRough = (PWRSHARE_SAMP_ROUGH*)arg;	
	A_CAN_MSG_FRAME frmSend;
	time_t tmNow;

    printf("--- Enter _Thread_Master_Addr1 ---\n");
    while(!pWhRough->bNeedExitTrdAddr1) 
    {
		sleep(2);

        printf("[---------- _Thread_Master_Addr1 Running ----------]\n");

		Master_RefreshGridMeterValue(pWhRough);
 
		tmNow = time(NULL);
 
		/*if(isTimeOut2Send(tmNow, pWhRough->tm_LastSend[_CAN_W_CMD_M2S_STATUS], 2) == TRUE)
		{
			printf("tmNow=[%ld] pWhRough->tm_LastSend[_CAN_W_CMD_M2S_STATUS]=[%ld]\n",\
				tmNow, pWhRough->tm_LastSend[_CAN_W_CMD_M2S_STATUS]);

			pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_STATUS].pfnCmdHandler((void*)pWhRough, &frmSend);
			pWhRough->tm_LastSend[_CAN_W_CMD_M2S_STATUS] = tmNow;
		}*/
 
		if(isTimeOut2Send(tmNow, pWhRough->tm_LastSend[_CAN_W_CMD_M2S_GRIDWH_POWER], 1) == TRUE)
		{
			init_FrmSend(&pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_GRIDWH_POWER], &frmSend);
			pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_GRIDWH_POWER].pfnCmdHandler((void*)pWhRough, &frmSend);
			pWhRough->tm_LastSend[_CAN_W_CMD_M2S_GRIDWH_POWER] = tmNow;
		}


		if(isTimeOut2Send(tmNow, pWhRough->tm_LastSend[_CAN_W_CMD_M2S_GRIDWH_CURT], 2) == TRUE)
		{
			init_FrmSend(&pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_GRIDWH_CURT], &frmSend);
			pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_GRIDWH_CURT].pfnCmdHandler((void*)pWhRough, &frmSend);
			pWhRough->tm_LastSend[_CAN_W_CMD_M2S_GRIDWH_CURT] = tmNow;
		}
	

		//pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_GRIDWH_VOLT].pfnCmdHandler((void*)pWhRough, &frmSend);

		//pWhRough->stMasterCanMsg[_CAN_W_CMD_M2S_GRIDWH_CURT].pfnCmdHandler((void*)pWhRough, &frmSend);

    }
    
    return NULL;
}
 
 #define MASTER_ADDR 		0x01
 #define BROADCAST_ADDR 	0xFF

 static BOOL Judge_SrcAddrIsMaster(IN PWRSHARE_SAMP_ROUGH *pWhRough, IN BYTE bySrcAddr)
 {
	//if(bySrcAddr == bySelfAddr) //Recv Data From SelfAddr
	//{
	//	printf("Error: Addr Conflict !!!\n");
	//	return FALSE;
	//}

	return (bySrcAddr == MASTER_ADDR) ? TRUE : FALSE; 
 }

static BOOL Find_CmdType(IN PWRSHARE_SAMP_ROUGH *pWhRough, IN BYTE byCmdType, IN BYTE byFuncID, OUT MST_CMD_MSGTYPE *emMsgType)
{
	UINT n = 0;
	BOOL bRet = FALSE;

	printf("!!! Find_CmdType --- \n");
	for(n = 0; n < _CAN_W_CMD_MST_MAX; n++)
	{
		if((byCmdType == pWhRough->stMasterCanMsg[n].byCmdType) 
			&& (byFuncID == pWhRough->stMasterCanMsg[n].byFuncID))
		{
			printf("n = %d\n", n);
			bRet = TRUE;
			*emMsgType = (MST_CMD_MSGTYPE)n;
			break;
		}
	}
	printf("!!! return Find_CmdType --- \n");

	return bRet;
}

static BOOL Parse_RecvData(IN PWRSHARE_SAMP_ROUGH *pWhRough, A_CAN_MSG_FRAME *pstCanFrame)
{
	UINT uiRawHead = 0;

	pstCanFrame->byCmdType 		= pstCanFrame->byRawData[0];
	pstCanFrame->byFuncID 		= pstCanFrame->byRawData[1];
    pstCanFrame->byDestAddr	 	= pstCanFrame->byRawData[2];    
	pstCanFrame->bySrcAddr 		= pstCanFrame->byRawData[3];

	printf("byCmdType=%02X, byFuncID=%02X, byDestAddr=%02X, bySrcAddr=%02X\n", \
		pstCanFrame->byCmdType, pstCanFrame->byFuncID, pstCanFrame->byDestAddr, pstCanFrame->bySrcAddr);

	if(!Judge_SrcAddrIsMaster(pWhRough, pstCanFrame->bySrcAddr))
	{
		printf("Error: Not Master Src Addr bySrcAddr=[%02X] !!!\n", pstCanFrame->bySrcAddr);
		return FALSE;
	}

	if(!Find_CmdType(pWhRough, pstCanFrame->byCmdType, pstCanFrame->byFuncID, &(pstCanFrame->emMsgType)))
	{
		printf("Error: Not Master Src Addr bySrcAddr=[%02X] !!!\n", pstCanFrame->bySrcAddr);
		return FALSE;
	}
 	//pstCanFrame->byCmdType
	/*uiRawHead = ((UINT)pFrame->byRawData[1]) << 24;
	uiRawHead += ((UINT)pFrame->byRawData[2]) << 16;
	uiRawHead += ((UINT)pFrame->byRawData[3]) << 8;
	uiRawHead += ((UINT)pFrame->byRawData[4]);

	printf("---- The Parsed Reply Frame Head: [%08X]----------------\n",uiRawHead);

	pFrame->byBMSGrpAddr = (BYTE)uiRawHead;
	if(YFY_CompBMSGrpMapIdx(pFrame->byBMSGrpAddr, &pFrame->nBMSGrpMapIdx) == FALSE)
	{
		//TRACE("Wrong nBMSGrpAddr =%02X\n", pFrame->byBMSGrpAddr);
		return FALSE;
	}
	
	pFrame->byFuncID = (BYTE)(uiRawHead >> 16);
	pFrame->byCmdType = (BYTE)(uiRawHead >> 24);
	if(YFY_CompBattProCmd(pFrame->byCmdType, pFrame->byFuncID, &pFrame->emMsgType) == FALSE)
	{
		//printf("Wrong byCmdType=%02X, byFuncID=%02X\n",pFrame->byCmdType, pFrame->byFuncID);
		return FALSE;
	} 

	memcpy(pFrame->byData, pFrame->byRawData + 5, CAN_DATA_AREA_LEN);*/

	return TRUE;
}

static void ReadAndParseRecvData(IN PWRSHARE_SAMP_ROUGH *pWhRough)
{
	BOOL bFoundReply = FALSE;
	int n = 0;
	for(n = 0; n < MAX_RING_BUFF; n++)
	{
		if(pWhRough->stCanMsgBuff[n].bRead == FALSE)
		{
			bFoundReply = TRUE;
			break;
		}
	}

	if(bFoundReply == FALSE)
	{
		return;
	}

	Parse_RecvData(pWhRough, &pWhRough->stCanMsgBuff[n].stCanFrame);
	
	if(pWhRough->stCanMsgBuff[n].stCanFrame.emMsgType == _CAN_W_CMD_M2S_GRIDWH_POWER)
	{
		printf("[_CAN_W_CMD_M2S_GRIDWH_POWER]:%02X %02X %.1fkW %02X %02X CRC=%02X\n",\
			pWhRough->stCanMsgBuff[n].stCanFrame.byCmdType,\
			pWhRough->stCanMsgBuff[n].stCanFrame.byFuncID,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.fPgrid,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.unionSysStatu.Val.AutoOrManual,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.byGridWhComm,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Pgrid_Sum.byCRC);
	}
	else if(pWhRough->stCanMsgBuff[n].stCanFrame.emMsgType == _CAN_W_CMD_M2S_GRIDWH_CURT)
	{
		printf("[_CAN_W_CMD_M2S_GRIDWH_CURT]:%02X %02X %dA %dA %dA %d\n",\
			pWhRough->stCanMsgBuff[n].stCanFrame.byCmdType,\
			pWhRough->stCanMsgBuff[n].stCanFrame.byFuncID,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.sIphA_Grid,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.sIphB_Grid,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.sIphC_Grid,\
			pWhRough->stCanMsgBuff[n].stCanFrame.unionDataArea.M2S_Igrid_3Phase.usResv);
	}

	pWhRough->stCanMsgBuff[n].bRead = TRUE;
	memset(&pWhRough->stCanMsgBuff[n].stCanFrame, 0 , sizeof(A_CAN_MSG_FRAME));

	return;
}


static void Slave_StateCmd_Init(PWRSHARE_SAMP_ROUGH *pWhRough)
{
	SLAVE_CANMSG_PRO *p = &pWhRough->stSlaveCanMsg[0];

	INIT_CANMSG_CMD(p,		_CAN_R_CMD_M2S_STATUS,			0x14, 		0x32,			Parse_M2S_Status,	 		FALSE);
	INIT_CANMSG_CMD(p,		_CAN_R_CMD_M2S_GRIDMETER, 		0x14, 		0x33,			Parse_M2S_GridMeter,		TRUE);	
	 
	return;
}

void* _Thread_Slave_Addr2(void* arg)
{
    PWRSHARE_SAMP_ROUGH *pWhRough = (PWRSHARE_SAMP_ROUGH*)arg;

	A_CAN_MSG_FRAME frmReply;

    printf("--------- Enter _Thread_Slave_Addr2 ---------\n");
    while(!pWhRough->bNeedExitTrdAddr2) 
    {
		ReadAndParseRecvData(pWhRough);

		//pWhRough->stSlaveCanMsg[_CAN_R_CMD_M2S_STATUS].pfnCmdHandler((void*)pWhRough, &frmReply);

		//pWhRough->stSlaveCanMsg[_CAN_R_CMD_M2S_GRIDMETER].pfnCmdHandler((void*)pWhRough, &frmReply);
        printf("[---------- _Thread_Slave_Addr2 Running ----------]\n");
        sleep(1);
    }
 
    return NULL;
}
 

void Master_StateCmd_Init(PWRSHARE_SAMP_ROUGH *pWhRough)
{
	MASTER_CANMSG_PRO* p = &pWhRough->stMasterCanMsg[0];

	INIT_CANMSG_CMD(p,		_CAN_W_CMD_M2S_STATUS,			0x03, 		0x01,	Send_Sys_Status,	TRUE);
	INIT_CANMSG_CMD(p,		_CAN_W_CMD_M2S_GRIDWH_POWER,	0x03, 		0x02,	Send_GridWh_Power,	TRUE);
	INIT_CANMSG_CMD(p,		_CAN_W_CMD_M2S_GRIDWH_VOLT,		0x03, 		0x03,	Send_GridWh_Volt,	TRUE);
	INIT_CANMSG_CMD(p,		_CAN_W_CMD_M2S_GRIDWH_CURT,		0x03, 		0x04,	Send_GridWh_Curt,	TRUE);

	int n = 0;
	for(n = 0; n < _CAN_W_CMD_MST_MAX; n++)
	{
		pWhRough->tm_LastSend[n] = time(NULL);
	}

	return;
}

static void Init_CanRingBuff(PWRSHARE_SAMP_ROUGH *pWhRough)
{
	int n = 0;
	for(n = 0; n < MAX_RING_BUFF; n++)
	{
		pWhRough->stCanMsgBuff[n].bRead = TRUE;
		memset(&pWhRough->stCanMsgBuff[n].stCanFrame, 0 , sizeof(A_CAN_MSG_FRAME));
	}

	return;
}

int main(int argc, char *argv[])
{
	PWRSHARE_SAMP_ROUGH stPwrShare = 
	{
		.bNeedExitTrdAddr1 = FALSE,
		.bNeedExitTrdAddr2 = FALSE
	};

	printf("[%d| %d]\n", stPwrShare.bNeedExitTrdAddr1, stPwrShare.bNeedExitTrdAddr2);

    PWRSHARE_SAMP_ROUGH *pWhRough = &stPwrShare;

	Slave_StateCmd_Init(pWhRough);
	Master_StateCmd_Init(pWhRough);

	Init_CanRingBuff(pWhRough);
 

    pthread_t p1;
    pthread_t p2;
     
    pthread_create(&p1, NULL, _Thread_Master_Addr1,    (void*)pWhRough);   
    pthread_create(&p2, NULL, _Thread_Slave_Addr2,     (void*)pWhRough);  
 
    pthread_join(p1, NULL);     //阻塞回收子线程
    pthread_join(p2, NULL);     //阻塞回收子线程

    return 0;
}

/*

	//pWhRough->stCanDataArea.M2S_Pgrid_Sum.fActvPower = -110.1;
	//pWhRough->stCanDataArea.M2S_Pgrid_Sum.fReActvPower = 200;

	int n = 0;

	VAR_VALUE varV;
	varV.fValue = -210.1;
	for(n = 0; n < 4; n++)
	{
		printf("%02X", varV.abyValue[n]);
	}

	pWhRough->stCanDataArea.byData[0] = 0x33;
	pWhRough->stCanDataArea.byData[1] = 0x33;
	pWhRough->stCanDataArea.byData[2] = 0xDC;
	pWhRough->stCanDataArea.byData[3] = 0xC2;
	pWhRough->stCanDataArea.byData[4] = 0x9A;
	pWhRough->stCanDataArea.byData[5] = 0x19;
	pWhRough->stCanDataArea.byData[6] = 0x52;
	pWhRough->stCanDataArea.byData[7] = 0xC3;
	float fValue = Bytes2Float(pWhRough->stCanDataArea.byData);

	printf("fValue = %.1f\n", fValue);

	for(n = 0; n < 8; n++)
	{
		printf("%02X", pWhRough->stCanDataArea.byData[n]);
	}
	printf("\n");

	printf("%f %f\n",\
	pWhRough->stCanDataArea.M2S_Pgrid_Sum.fActvPower ,
	pWhRough->stCanDataArea.M2S_Pgrid_Sum.fReActvPower );
*/