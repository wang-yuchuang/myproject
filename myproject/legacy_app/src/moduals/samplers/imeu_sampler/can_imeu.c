//#define _TRACE_PRINT_
#include "imeu_sampler_main.h"
#include "can_imeu.h"
#include  "imeu2_can.h"
#include  "imeu3_can.h"

// #define BATT_CAN_INFO_DEBUG
//2021-11-16 Max Support 4 Group Battery,if IMTCU Addr is Over 4,will Cause SW Reboot.
static BYTE szPCSGrpMapIdx[]={0x5A, 0x5B, 0x5C, 0x5D};

//BMU:		0x32-0x35
//BSU:		0x00-0x03
//IMEU2:	0x28-2B
//BMU-SLAVE:0xC8-0xCB
//IMEU3     0x1E-0x21
static BYTE szLocalBMSGrpMapIdx[]={0x32, 0x33, 0x34, 0x35, 
									0x00, 0x01, 0x02, 0x03,
									0x04, 0x05, 0x06, 0x07,
									0x08, 0x09, 0x0A, 0x0B,
									0x28, 0x29, 0x2A, 0X2B,
									0xC8, 0xC9, 0XCA, 0XCB,
									0x1F, 0x20, 0x21, 0x22};

//0x1E 电源仓IMEU3,  0x2D STS柜IMEU3
static BYTE szIMEU3MapIdx[]={0x1E, 0x2D, 0x2B, 0x2C};   //电源仓 STS柜EU3 空调   

static TAG_YFY_BATT_PRO		s_infy_BattPro[_MAX_YFY_CAN_CMD];

//Crc_ComputeCRC8- According SAE J1850  Calculate CRC Value
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



const uint32_t Cal_Crc32Tab[256] = {
    0x00000000uL, 0x77073096uL, 0xee0e612cuL, 0x990951bauL,
    0x076dc419uL, 0x706af48fuL, 0xe963a535uL, 0x9e6495a3uL,
    0x0edb8832uL, 0x79dcb8a4uL, 0xe0d5e91euL, 0x97d2d988uL,
    0x09b64c2buL, 0x7eb17cbduL, 0xe7b82d07uL, 0x90bf1d91uL,
    0x1db71064uL, 0x6ab020f2uL, 0xf3b97148uL, 0x84be41deuL,
    0x1adad47duL, 0x6ddde4ebuL, 0xf4d4b551uL, 0x83d385c7uL,
    0x136c9856uL, 0x646ba8c0uL, 0xfd62f97auL, 0x8a65c9ecuL,
    0x14015c4fuL, 0x63066cd9uL, 0xfa0f3d63uL, 0x8d080df5uL,
    0x3b6e20c8uL, 0x4c69105euL, 0xd56041e4uL, 0xa2677172uL,
    0x3c03e4d1uL, 0x4b04d447uL, 0xd20d85fduL, 0xa50ab56buL,
    0x35b5a8fauL, 0x42b2986cuL, 0xdbbbc9d6uL, 0xacbcf940uL,
    0x32d86ce3uL, 0x45df5c75uL, 0xdcd60dcfuL, 0xabd13d59uL,
    0x26d930acuL, 0x51de003auL, 0xc8d75180uL, 0xbfd06116uL,
    0x21b4f4b5uL, 0x56b3c423uL, 0xcfba9599uL, 0xb8bda50fuL,
    0x2802b89euL, 0x5f058808uL, 0xc60cd9b2uL, 0xb10be924uL,
    0x2f6f7c87uL, 0x58684c11uL, 0xc1611dabuL, 0xb6662d3duL,
    0x76dc4190uL, 0x01db7106uL, 0x98d220bcuL, 0xefd5102auL,
    0x71b18589uL, 0x06b6b51fuL, 0x9fbfe4a5uL, 0xe8b8d433uL,
    0x7807c9a2uL, 0x0f00f934uL, 0x9609a88euL, 0xe10e9818uL,
    0x7f6a0dbbuL, 0x086d3d2duL, 0x91646c97uL, 0xe6635c01uL,
    0x6b6b51f4uL, 0x1c6c6162uL, 0x856530d8uL, 0xf262004euL,
    0x6c0695eduL, 0x1b01a57buL, 0x8208f4c1uL, 0xf50fc457uL,
    0x65b0d9c6uL, 0x12b7e950uL, 0x8bbeb8eauL, 0xfcb9887cuL,
    0x62dd1ddfuL, 0x15da2d49uL, 0x8cd37cf3uL, 0xfbd44c65uL,
    0x4db26158uL, 0x3ab551ceuL, 0xa3bc0074uL, 0xd4bb30e2uL,
    0x4adfa541uL, 0x3dd895d7uL, 0xa4d1c46duL, 0xd3d6f4fbuL,
    0x4369e96auL, 0x346ed9fcuL, 0xad678846uL, 0xda60b8d0uL,
    0x44042d73uL, 0x33031de5uL, 0xaa0a4c5fuL, 0xdd0d7cc9uL,
    0x5005713cuL, 0x270241aauL, 0xbe0b1010uL, 0xc90c2086uL,
    0x5768b525uL, 0x206f85b3uL, 0xb966d409uL, 0xce61e49fuL,
    0x5edef90euL, 0x29d9c998uL, 0xb0d09822uL, 0xc7d7a8b4uL,
    0x59b33d17uL, 0x2eb40d81uL, 0xb7bd5c3buL, 0xc0ba6caduL,
    0xedb88320uL, 0x9abfb3b6uL, 0x03b6e20cuL, 0x74b1d29auL,
    0xead54739uL, 0x9dd277afuL, 0x04db2615uL, 0x73dc1683uL,
    0xe3630b12uL, 0x94643b84uL, 0x0d6d6a3euL, 0x7a6a5aa8uL,
    0xe40ecf0buL, 0x9309ff9duL, 0x0a00ae27uL, 0x7d079eb1uL,
    0xf00f9344uL, 0x8708a3d2uL, 0x1e01f268uL, 0x6906c2feuL,
    0xf762575duL, 0x806567cbuL, 0x196c3671uL, 0x6e6b06e7uL,
    0xfed41b76uL, 0x89d32be0uL, 0x10da7a5auL, 0x67dd4accuL,
    0xf9b9df6fuL, 0x8ebeeff9uL, 0x17b7be43uL, 0x60b08ed5uL,
    0xd6d6a3e8uL, 0xa1d1937euL, 0x38d8c2c4uL, 0x4fdff252uL,
    0xd1bb67f1uL, 0xa6bc5767uL, 0x3fb506dduL, 0x48b2364buL,
    0xd80d2bdauL, 0xaf0a1b4cuL, 0x36034af6uL, 0x41047a60uL,
    0xdf60efc3uL, 0xa867df55uL, 0x316e8eefuL, 0x4669be79uL,
    0xcb61b38cuL, 0xbc66831auL, 0x256fd2a0uL, 0x5268e236uL,
    0xcc0c7795uL, 0xbb0b4703uL, 0x220216b9uL, 0x5505262fuL,
    0xc5ba3bbeuL, 0xb2bd0b28uL, 0x2bb45a92uL, 0x5cb36a04uL,
    0xc2d7ffa7uL, 0xb5d0cf31uL, 0x2cd99e8buL, 0x5bdeae1duL,
    0x9b64c2b0uL, 0xec63f226uL, 0x756aa39cuL, 0x026d930auL,
    0x9c0906a9uL, 0xeb0e363fuL, 0x72076785uL, 0x05005713uL,
    0x95bf4a82uL, 0xe2b87a14uL, 0x7bb12baeuL, 0x0cb61b38uL,
    0x92d28e9buL, 0xe5d5be0duL, 0x7cdcefb7uL, 0x0bdbdf21uL,
    0x86d3d2d4uL, 0xf1d4e242uL, 0x68ddb3f8uL, 0x1fda836euL,
    0x81be16cduL, 0xf6b9265buL, 0x6fb077e1uL, 0x18b74777uL,
    0x88085ae6uL, 0xff0f6a70uL, 0x66063bcauL, 0x11010b5cuL,
    0x8f659effuL, 0xf862ae69uL, 0x616bffd3uL, 0x166ccf45uL,
    0xa00ae278uL, 0xd70dd2eeuL, 0x4e048354uL, 0x3903b3c2uL,
    0xa7672661uL, 0xd06016f7uL, 0x4969474duL, 0x3e6e77dbuL,
    0xaed16a4auL, 0xd9d65adcuL, 0x40df0b66uL, 0x37d83bf0uL,
    0xa9bcae53uL, 0xdebb9ec5uL, 0x47b2cf7fuL, 0x30b5ffe9uL,
    0xbdbdf21cuL, 0xcabac28auL, 0x53b39330uL, 0x24b4a3a6uL,
    0xbad03605uL, 0xcdd70693uL, 0x54de5729uL, 0x23d967bfuL,
    0xb3667a2euL, 0xc4614ab8uL, 0x5d681b02uL, 0x2a6f2b94uL,
    0xb40bbe37uL, 0xc30c8ea1uL, 0x5a05df1buL, 0x2d02ef8duL 
};

void Cal_Crc32Init(uint32_t *curCrc)
{
    *curCrc = 0xFFFFFFFFuL;
    
    return;
}

void Cal_ComputeCrc32(uint32_t *curCrc, const uint8_t *buf, const uint32_t size)
{
    uint32_t  i;
    
    for (i = 0; i < size; i ++)
    {
        *curCrc = Cal_Crc32Tab[(*curCrc ^(uint32_t)buf[i]) & 0xffu] ^ (*curCrc >> 8);
    }
    
    return;
}

void Cal_CrcFinalize(uint32_t *curCrc)
{
    *curCrc ^= 0xFFFFFFFFuL;
    
    return;
}

static USHORT CAN_StringToUShort(BYTE* strInput) //LSB
{
	USHORT usValue = ((USHORT)(strInput[0])) + (((USHORT)(strInput[1])) << 8);

	return usValue;
}

static SHORT CAN_StringToShort(BYTE* strInput) //LSB
{
	SHORT sValue = (SHORT)( ((USHORT)strInput[0]) | ((USHORT)(strInput[1] << 8)) );

	return sValue;
}


static UINT CAN_StringToUint(BYTE* strInput) //LSB
{
	UINT uValue = ((UINT)(strInput[0])) + (((UINT)(strInput[1])) << 8)
					+ (((UINT)(strInput[2])) << 16) + (((USHORT)(strInput[3])) << 24);

	return uValue;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BYTE GetEMSAddr(int nBattIdx)
{
	int n = 0;

	for(n = 0; n < sizeof(szPCSGrpMapIdx)/sizeof(szPCSGrpMapIdx[0]); n++)
	{
		if(n == nBattIdx)
		{
			return szPCSGrpMapIdx[n];
		}
	}
	
	return szPCSGrpMapIdx[0];
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BYTE GetBMSAddr(int nBattIdx)
{
	int n = 0;
	for(n = 0; n < sizeof(szLocalBMSGrpMapIdx)/sizeof(szLocalBMSGrpMapIdx[0]); n++)
	{
		if(n == nBattIdx)
		{
			return szLocalBMSGrpMapIdx[n];
		}
	}
	return szLocalBMSGrpMapIdx[0];
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_CAN_StuffMsgID(BYTE byFuncCode, BYTE bySubIdx, BYTE byBMSAddr, BYTE byEMSAddr, UINT uiDataLen, BYTE *pbyFrame)
{
	*pbyFrame = (BYTE)uiDataLen;
	*(pbyFrame + 1) = byFuncCode;
	*(pbyFrame + 2) = bySubIdx;
	*(pbyFrame + 3) = byBMSAddr;
	*(pbyFrame + 4) = byEMSAddr;

	//TRACE("Info:CAN_StuffMsgID--- %02x %02x %02x %02x ---\n",pbyFrame[1],pbyFrame[2],pbyFrame[3],pbyFrame[4]);
	return ;
}

void RecBmsMu2Log(IMEU_SAMPLER_ROUGH *pIMEU_Rough, BYTE* pbySendBuf, ssize_t uFrameSize)
{
	// Manual Rec Mu2 to Bmu CanLog
	if(pIMEU_Rough->bNeedRecBmsMu2CanLog == TRUE)
	{
		
		char log[1024];
		char tmp[64];
    	memset(log, 0, sizeof(log));
    	memset(tmp, 0, sizeof(tmp));

		int n = 0;
		for(n = 0; n < uFrameSize; n++)
		{
			snprintf(tmp, 32, " %02X", pbySendBuf[n]);
        	strcat(log, tmp);
		}

		// LOG_BMS_CAN_INFO("BMUtoMU2 : %s", log);
		LOG_BMS_CAN_INFO("%s", log);
		// printf("BMU to MU2: \n");
		// for(n=0; n<uFrameSize; n++)
		// {
		// 	printf("%s\n", log);
		// }

		if(fabs(pIMEU_Rough->tm_fLastStartRecBmsMu2canlog - _NOW_) > 3*60)
		{
			pIMEU_Rough->bNeedRecBmsMu2CanLog = FALSE;
		}
	}
	
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL YFY_CompBMSGrpMapIdx(BYTE nBMSGrpAddr, int *nBMSGrpMapIdx)
{	
	int n = 0;
	int nAddrCnt = sizeof(szLocalBMSGrpMapIdx) / sizeof(szLocalBMSGrpMapIdx[0]);

	//TRACE("nBMSGrpAddr=%d, nAddrCnt=%d\n", nBMSGrpAddr, nAddrCnt);
	for(n = 0; n < nAddrCnt; n++)
	{
		if(nBMSGrpAddr == szLocalBMSGrpMapIdx[n])
		{
			*nBMSGrpMapIdx = n % MAX_SINGLE_STBATT_NUM;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL YFY_Comp_IMEU3MapIdx(BYTE nIMEU3GrpAddr, int *nIMEU3GrpMapIdx)
{	
	int n = 0;
	int nAddrCnt = sizeof(szIMEU3MapIdx) / sizeof(szIMEU3MapIdx[0]);

	for(n = 0; n < nAddrCnt; n++)
	{
		if(nIMEU3GrpAddr == szIMEU3MapIdx[n])
		{
			*nIMEU3GrpMapIdx = n % MAX_IMEU_NUM;
			return TRUE;
		}
	}

	return FALSE;
}

BYTE getIMEU3Addr(int iDevIdx)
{
	BYTE addr = 0xff;

	int i = 0;
	int iCount = sizeof(szIMEU3MapIdx) / sizeof(szIMEU3MapIdx[0]);

	if (iDevIdx >= 0 && iDevIdx < iCount)
	{
		addr = szIMEU3MapIdx[iDevIdx];
	}

	return addr;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL YFY_CompBattProCmd(BYTE byFuncCode, BYTE bySubIndex, ID_CMD_YFY_BATT *emMsgType)
{
	if(byFuncCode == 0x16)  //CELL DATA
	{
		if((bySubIndex >= CELL_BALANCE_SUB_IDX_START) && (bySubIndex <= CELL_BALANCE_SUB_IDX_END))
		{
			*emMsgType = _CAN_R_CMD_S2C_CELLDATA1;
			return TRUE;
		}
		else if((bySubIndex >= CELL_VOLT_SUB_IDX_START) && (bySubIndex <= CELL_VOLT_SUB_IDX_END))
		{
			*emMsgType = _CAN_R_CMD_S2C_CELLDATA2;
			return TRUE;
		}
		else if((bySubIndex >= CELL_TEMP_SUB_IDX_START) && (bySubIndex <= CELL_TEMP_SUB_IDX_END))
		{
			*emMsgType = _CAN_R_CMD_S2C_CELLDATA3;
			return TRUE;
		}
		// else if((bySubIndex >= CELL_AFE_SUB_IDX_START) && (bySubIndex <= CELL_AFE_SUB_IDX_END))
		// {
		// 	*emMsgType = _CAN_R_CMD_S2C_CELLAFEWORKMODE;
		// 	return TRUE;
		// }
		else if((bySubIndex >= CELL_SAFE_SUB_IDX_START) && (bySubIndex <= CELL_SAFE_SUB_IDX_END))
		{
			*emMsgType = _CAN_R_CMD_S2C_CELLDATA8;
			return TRUE;
		}
	}
	else
	{
		UINT n = 0;
		for(n = 0; n < _MAX_YFY_CAN_CMD; n++)
		{
			if((byFuncCode == s_infy_BattPro[n].nFuncCode) && (bySubIndex == s_infy_BattPro[n].nSubIndex))
			{
				*emMsgType = s_infy_BattPro[n].emMsgType;
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL YFY_JudgeAndParseMsgID(A_CAN_YFY_BATT_FRAME* pFrame)
{
	UINT uiRawHead = 0;

	uiRawHead = ((UINT)pFrame->byRawData[1]) << 24;
	uiRawHead += ((UINT)pFrame->byRawData[2]) << 16;
	uiRawHead += ((UINT)pFrame->byRawData[3]) << 8;
	uiRawHead += ((UINT)pFrame->byRawData[4]);

	//TRACE("---- The Parsed Reply Frame Head: [%08X]----------------\n",uiRawHead);

	pFrame->byBMSGrpAddr = (BYTE)uiRawHead;
	if(YFY_CompBMSGrpMapIdx(pFrame->byBMSGrpAddr, &pFrame->nBMSGrpMapIdx) == FALSE)
	{
		//TRACE("Wrong nBMSGrpAddr =%02X\n", pFrame->byBMSGrpAddr);
		return FALSE;
	}
	
	pFrame->bySubIndex = (BYTE)(uiRawHead >> 16);
	pFrame->byFuncCode = (BYTE)(uiRawHead >> 24);
	if(YFY_CompBattProCmd(pFrame->byFuncCode, pFrame->bySubIndex, &pFrame->emMsgType) == FALSE)
	{
		//printf("Wrong byFuncCode=%02X, bySubIndex=%02X\n",pFrame->byFuncCode, pFrame->bySubIndex);
		return FALSE;
	} 

	memcpy(pFrame->byData, pFrame->byRawData + 5, CAN_DATA_AREA_LEN);

	return TRUE;
}


static void IMEU_CommStatus(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
	int n = 0;
	if(IMEU_TYPE_IMEU3 == pIMEU_Rough->emPOWER_IMEUType[0])
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			if(n >= MAX_IMEU_NUM)
			{
				continue;
			}

			// if(pIMEU_Rough->IMEU3CANRoughData[n].bFoundReply == FALSE)   //电池仓IMEU3 通信状态
			// {
			// 	if(pIMEU_Rough->IMEU3CANRoughData[n].nInterrupt_Times <= IMEU_COMM_FAIL_TIMES)
			// 	{
			// 		pIMEU_Rough->IMEU3CANRoughData[n].nInterrupt_Times++;
			// 	}
			// }
			// else if(pIMEU_Rough->IMEU3CANRoughData[n].bFoundReply == TRUE)
			// {
			// 	pIMEU_Rough->IMEU3CANRoughData[n].nInterrupt_Times = 0;
			// }

			if(pIMEU_Rough->PowerEU3_OtherEU3_Data[n].bFoundReply == FALSE)  //判断电源仓和其他的EU3通信状态
			{
				if(pIMEU_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times <= IMEU_COMM_FAIL_TIMES)
				{
					pIMEU_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times++;
				}
			}
			else if(pIMEU_Rough->PowerEU3_OtherEU3_Data[n].bFoundReply == TRUE)
			{
				pIMEU_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times = 0;
			}
			pIMEU_Rough->PowerEU3_OtherEU3_Data[n].bIsCommFail = (pIMEU_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times > IMEU_COMM_FAIL_TIMES) ? TRUE : FALSE;  //电源仓
		}
	}

	return;

}

void YFY_ReadAndParseCanData(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
	int nFrmCount;
	int n=0, nReadBytes=0, nReadLen=0;
	A_CAN_YFY_BATT_FRAME frmReply;
	ZERO_POBJS(&frmReply, 1);
	frmReply.pIMEU_Rough = pIMEU_Rough;

	IMEU2_CAN_FRAME IMEU2FrameReply;
	ZERO_POBJS(&IMEU2FrameReply, 1);

	IMEU3_CAN_FRAME IMEU3FrameReply;
	ZERO_POBJS(&IMEU3FrameReply, 1);

	BYTE *byRecvBuff = pIMEU_Rough->abyRcvBuf;
 	memset(byRecvBuff, 0, MAX_READ_BUFF_SIZE*CAN_FRAME_LEN);

	nReadLen = pIMEU_Rough->pComm->Read(pIMEU_Rough->hCanDevHanle, (void*)byRecvBuff, (ssize_t)(CAN_FRAME_LEN * MAX_READ_BUFF_SIZE));

	//printf("YFY BMS Recv: \n");
	//for(n=0; n<nReadLen; n++)
	//{
	//	printf("%02X ", byRecvBuff[n]);
	//}
	//printf("\n");
	
	//TRACE("Not Found Any Data!!!\n");
/*
	if(IMEU_TYPE_IMEU2 == pIMEU_Rough->emIMEUType[0])
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			pIMEU_Rough->IMEU2CANRoughData[n].bFoundReply = FALSE;
		}
	}
	else 
*/
	if(IMEU_TYPE_IMEU3 == pIMEU_Rough->emPOWER_IMEUType[0])//edit by Kuangyue 20251130
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			pIMEU_Rough->PowerEU3_OtherEU3_Data[n].bFoundReply = FALSE;
		}
	}

	if(nReadLen >= CAN_FRAME_LEN)
	{
		nFrmCount = nReadLen / CAN_FRAME_LEN;
		for(n = 0; n < nFrmCount; n++)
		{
			memcpy(frmReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
			
			/*
			if (IMEU_TYPE_IMEU2 == pIMEU_Rough->emIMEUType[0])
			{
				memcpy(IMEU2FrameReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
				if (judgeAndParseIMEU2MsgID(&IMEU2FrameReply))
				{
					if (IMEU2FrameReply.bySourceIdx >= 0 && IMEU2FrameReply.bySourceIdx < MAX_IMEU_NUM)
					{
						pIMEU_Rough->IMEU2CANRoughData[IMEU2FrameReply.bySourceIdx].bFoundReply = TRUE;

						processIMEU2CANData(&pIMEU_Rough->IMEU2CANRoughData[IMEU2FrameReply.bySourceIdx], &IMEU2FrameReply);
					}
				}
			}
			else 
			*/
			if (IMEU_TYPE_IMEU3 == pIMEU_Rough->emPOWER_IMEUType[0])//edit by Kuangyue 20251130
			{
				memcpy(IMEU3FrameReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
				if(judgeAndParseOtherIMEU3MsgID(&IMEU3FrameReply))   //解析电源仓和其他的IMEU3数据
				{
					if (IMEU3FrameReply.bySourceIdx >= 0 && IMEU3FrameReply.bySourceIdx < MAX_IMEU_NUM)
					{
						pIMEU_Rough->PowerEU3_OtherEU3_Data[IMEU3FrameReply.bySourceIdx].bFoundReply = TRUE;

						if(IMEU3FrameReply.emMsgType == _CAN_R_CMD_C2S_IMEU3UPGRADE)  //单独处理电源、STS EU3 OTA升级CAN报文，
						{
							memcpy(frmReply.byData, frmReply.byRawData + 5, CAN_DATA_AREA_LEN);
							frmReply.nBMSGrpMapIdx = IMEU3FrameReply.bySourceIdx;
							s_infy_BattPro[_CAN_R_CMD_C2S_UPGRADESTATU].pfnCmdHandler(&frmReply);		
						}
						else
						{
							processIMEU3CANData(&pIMEU_Rough->PowerEU3_OtherEU3_Data[IMEU3FrameReply.bySourceIdx], &IMEU3FrameReply);
						}
					}
				}

			}
		}
	}

	// Pro_YFY_Batt_CommStatu(pIMEU_Rough);

	IMEU_CommStatus(pIMEU_Rough);
	//proIMEU2CommStatu(pIMEU_Rough->);
	return;
}


/*********************************************************************************
*
*  FUNCTION NAME :
*  CREATOR       :
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :
*
*
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void YFY_IMEU_Sample(IMEU_SAMPLER_ROUGH *pIMEU_Rough)
{
	double 		tmNow = GetCurrentTime();
	static 		BOOL bFirstRun = TRUE;
	//VAR_VALUE 	varV;
	int 		n = 0;

	if(Mutex_Lock(pIMEU_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
	{	
		YFY_ReadAndParseCanData(pIMEU_Rough);
		Mutex_Unlock(pIMEU_Rough->hSamplerLock);
	}
	
	//--- 20230205 John Added For Remote Upgrade ---//

	if (IMEU_TYPE_IMEU3 == pIMEU_Rough->emPOWER_IMEUType[0])
	{
		// IMEU3ComStatus(pIMEU_Rough->IMEU3CANRoughData, MAX_IMEU_NUM); //电池仓 

		IMEU3PowerComStatus(pIMEU_Rough->PowerEU3_OtherEU3_Data, MAX_IMEU_NUM);  //电源仓
	}

	return;
}

