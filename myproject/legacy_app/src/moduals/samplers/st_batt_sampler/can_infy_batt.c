//#define _TRACE_PRINT_
#include "st_batt_sampler.h"
#include "can_infy_batt.h"
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

static YFY_BATT_RUN_DATA    s_infy_BattRunData[MAX_SINGLE_STBATT_NUM];  
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

void RecBmsMu2Log(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE* pbySendBuf, ssize_t uFrameSize)
{
	// Manual Rec Mu2 to Bmu CanLog
	if(pSTBatt_Rough->bNeedRecBmsMu2CanLog == TRUE)
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

		if(fabs(pSTBatt_Rough->tm_fLastStartRecBmsMu2canlog - _NOW_) > 3*60)
		{
			pSTBatt_Rough->bNeedRecBmsMu2CanLog = FALSE;
		}
	}
	
}

// 持续保存最新200条BMUCAN报文，当告警发生时，继续保存告警发生后的200条报文
void RecBmsMu2AlarmLog(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BYTE* pbySendBuf, ssize_t uFrameSize)
{
	// Auto Rec bms canlog, when alm trigger rec to file
	CanRecCirBuffer* canRecBuff = &pSTBatt_Rough->bmuCanRecCirBuffer;
	CanRecBuffer* canAlmRecBuff = &pSTBatt_Rough->bmuAlmCanRecCirBuffer;

	TRACE("STATS == %d\n", s_infy_BattRunData[0].usBattWorkState);
	if(s_infy_BattRunData[0].usBattWorkState == 5 // 5:fault
		&& ABS(pSTBatt_Rough->tm_fLastRecBmsAlmcanlog - _NOW_) > 5*60) // 最多5min记录一次
	{
		canAlmRecBuff->isActive = TRUE;
	}

	if(canAlmRecBuff->isActive == FALSE)
	{
		memcpy(canRecBuff->buffer[canRecBuff->head], pbySendBuf, uFrameSize);
		canRecBuff->head = (canRecBuff->head + 1) % 200;
		if(canRecBuff->count < 200)
		{
			canRecBuff->count++;
		}
		else
		{
			canRecBuff->tail = (canRecBuff->tail + 1) % 200;
		}
	}
	else //if(canAlmRecBuff->isActive == TRUE)
	{
		if(canAlmRecBuff->count < 200)
		{
			memcpy(canAlmRecBuff->buffer[canAlmRecBuff->count], pbySendBuf, uFrameSize);
			canAlmRecBuff->count++;
		}
		else
		{
			char log[1024];
			char tmp[64];
			memset(log, 0, sizeof(log));
			memset(tmp, 0, sizeof(tmp));
			
			int m, n;
			LOG_BMS_ALM_CAN_INFO("----------follow before Alarm occur : -----------");
			while(canRecBuff->count-- > 0)
			{
				for(n = 0; n < CAN_FRAME_LEN; ++n)
				{
					snprintf(tmp, 32, " %02X", canRecBuff->buffer[canRecBuff->tail][n]);
					strcat(log, tmp);
				}
				LOG_BMS_ALM_CAN_INFO("%s", log);
				memset(log, 0, sizeof(log));
				canRecBuff->tail = (canRecBuff->tail + 1) % 200;
			}
			LOG_BMS_ALM_CAN_INFO("----------follow after Alarm occur : -----------");
			for(m=0; m < canAlmRecBuff->count; ++m)
			{
				for(n = 0; n < CAN_FRAME_LEN; ++n)
				{
					snprintf(tmp, 32, " %02X", canAlmRecBuff->buffer[m][n]);
					strcat(log, tmp);
				}
				LOG_BMS_ALM_CAN_INFO("%s", log);
				memset(log, 0, sizeof(log));
			}
			canRecBuff->count = 0;
			canRecBuff->head = 0;
			canRecBuff->tail = 0;

			canAlmRecBuff->count = 0;
			canAlmRecBuff->isActive = FALSE;
			pSTBatt_Rough->tm_fLastRecBmsAlmcanlog = _NOW_;
		}
	}
}

static void SaveBmsCanAlrm(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	char szSave[128] = {0};
	char szCAN[16] = {0};
	strcat(szSave, "CAN ID  0x");
	int i = 0;
	for(i = 1; i <= 4; ++i)
	{
		snprintf(szCAN, 16, "%02X", pFrmReply->byRawData[i]);
		strcat(szSave, szCAN);
	}
	strcat(szSave, "  DATA :  ");
	for(i = 0; i < CAN_DATA_AREA_LEN; ++i)
	{
		snprintf(szCAN, 16, "%02X ", pFrmReply->byData[i]);
		strcat(szSave, szCAN);
	}

	LOG_BMS_HIS_ALM_INFO("%s", szSave);

	return;
}



/*********************************************************************************
*  
*  FUNCTION NAME : Parse_S2C_Status
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_Status(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState = CAN_StringToUShort(pFrmReply->byData); 
	if (s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState < 0 || s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState > 5)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState = YFYBATT_SYSSTAT_HV_UP;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usBattWorkState = CAN_StringToUShort(pFrmReply->byData + 2);
	if (s_infy_BattRunData[nBMSGrpMapIdx].usBattWorkState < 0 || s_infy_BattRunData[nBMSGrpMapIdx].usBattWorkState > 6)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].usBattWorkState = 3;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].emBattChgState = CAN_StringToUShort(pFrmReply->byData + 4);

#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Status --BMS保护状态(0:未就绪 1:就绪 2:充电截止 3:放电截止 4:充放电限制 5:充放电保护)[%d]\n",s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState);
	printf("BMS工作状态(0:开机 1:待命 2:启动 3:运行 4:停机 5:故障 6:关机)[%d]\n",s_infy_BattRunData[nBMSGrpMapIdx].usBattWorkState);
	printf("BMS充放状态(0:空闲 1:充电 2:放电)[%d]\n",s_infy_BattRunData[nBMSGrpMapIdx].emBattChgState);
#endif
	//TRACEX("S2C_Status --nBMSGrpMapIdx=[%d] emBattSysState[%d], emBattChgState[%d]\n", \
	//	nBMSGrpMapIdx, s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState, s_infy_BattRunData[nBMSGrpMapIdx].emBattChgState);
	
	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Parse_S2C_Limit1
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_Limit1(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1; 
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1; 
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargePow = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1; 
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargePow = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1; 
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Limit1 -- 最大允许充电限流值[%0.1f],最大允许放电限流值[%0.1f],最大允许充电限功率值[%0.1f],最大允许放电限功率值[%0.1f]\n",
		s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr,s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr,
		s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargePow,s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargePow); 
#endif
	//TRACE("S2C_Limit1 -- fAllow_MaxChargeCurr:%f, fAllow_MaxDisChargeCurr:%f\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr, s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr);

	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Parse_S2C_Limit2
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_Limit2(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeVolt = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1; 
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeVolt = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1; 
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Limit2 -- 最大允许充电限压值[%0.1f],最大允许放电限压值[%0.1f]\n",
		s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeVolt,s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeVolt);
#endif
	//TRACE("S2C_Limit2 -- fAllow_MaxChargeVolt:%f, fAllow_MaxDisChargeVolt:%f\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeVolt, s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeVolt);

	return TRUE;
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
static BOOL Parse_S2C_Alarm(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	//s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode.uFlag[0] = CAN_StringToUint(pFrmReply->byData); 
	//s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode.uFlag[1] = CAN_StringToUShort(pFrmReply->byData + 4);  

	IFY_BATT_ALARM_CODE unLastunioBattAlarmCode = s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode;

	memcpy(s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode.szByIfyAlarm, pFrmReply->byData, CAN_DATA_AREA_LEN);
	
	if(memcmp(&unLastunioBattAlarmCode, &s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode, CAN_DATA_AREA_LEN) != 0)
	{
		SaveBmsCanAlrm(pFrmReply);
	}

	int n = 0, nOffset = 0;
	//printf("S2C_Alarm -- BMS告警码:");
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
    {
        nOffset  += sprintf(s_infy_BattRunData[nBMSGrpMapIdx].szInfy_AlarmInfo + nOffset, "%02X|",pFrmReply->byData[n]);
		//printf("%02X|", pFrmReply->byData[n]);
	}
	//printf("\n");
    s_infy_BattRunData[nBMSGrpMapIdx].szInfy_AlarmInfo[nOffset] ='\0';

	//TRACE("S2C_Alarm -- unioBattAlarmCode.uFlag[0]:%d, unioBattAlarmCode.uFlag[1]:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode.uFlag[0], s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode.uFlag[1]);

	return TRUE;
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
static BOOL Parse_S2C_Fault(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	//s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode.uFlag[0] = CAN_StringToUint(pFrmReply->byData); 
	//s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode.uFlag[1] = CAN_StringToUShort(pFrmReply->byData + 4);  
	
	IFY_BATT_FAULT_CODE unLastunioBattFaultCode = s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode;

	memcpy(s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode.szByIfyFault, pFrmReply->byData, CAN_DATA_AREA_LEN);

	if(memcmp(&unLastunioBattFaultCode, &s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode, CAN_DATA_AREA_LEN) != 0)
	{
		SaveBmsCanAlrm(pFrmReply);
	}
	
	int n = 0, nOffset = 0;
	//printf("S2C_Fault -- BMS故障码:");
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
    {
        nOffset  += sprintf(s_infy_BattRunData[nBMSGrpMapIdx].szInfy_FaultInfo + nOffset, "%02X|",pFrmReply->byData[n]);
		TRACE("%02X|", pFrmReply->byData[n]);
    }
	//printf("\n");
    s_infy_BattRunData[nBMSGrpMapIdx].szInfy_FaultInfo[nOffset] ='\0';


	//TRACE("S2C_Fault -- unioBattFaultCode.uFlag[0]:%d, unioBattFaultCode2.uFlag[0]:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode.uFlag[0], s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode.uFlag[0]);

	return TRUE;
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
static BOOL Parse_S2C_GPIO(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	//s_infy_BattRunData[nBMSGrpMapIdx].unioBattDICheckStat.usFlag = CAN_StringToUShort(pFrmReply->byData); 
	//s_infy_BattRunData[nBMSGrpMapIdx].unioBattDOCtrlStat.usFlag = CAN_StringToUShort(pFrmReply->byData + 2);  
	
	memcpy(s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat, pFrmReply->byData, CAN_DATA_AREA_LEN);

	int n = 0, nOffset = 0;
	//printf("S2C_GPIO -- BMS_GPIO:");
	for(n = 0; n < CAN_DATA_AREA_LEN; n++)
	{
		nOffset  += sprintf(s_infy_BattRunData[nBMSGrpMapIdx].szInfy_GPIOInfo + nOffset, "%02X|",pFrmReply->byData[n]);
		TRACE("%02X|", pFrmReply->byData[n]);
	}
	//printf("\n");
	s_infy_BattRunData[nBMSGrpMapIdx].szInfy_GPIOInfo[nOffset] ='\0';

	//TRACE("S2C_GPIO -- unioBattDICheckStat.usFlag:%d, unioBattDOCtrlStat.usFlag:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDICheckStat.usFlag, s_infy_BattRunData[nBMSGrpMapIdx].unioBattDOCtrlStat.usFlag);

	return TRUE;
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
static BOOL Parse_S2C_SumData1(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	//--- BMS Charging is Negative, Discharging is Positive ---//
	s_infy_BattRunData[nBMSGrpMapIdx].fPackTotalVolt = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].fOutputCurrent = ((float)CAN_StringToShort(pFrmReply->byData + 2)) * 0.1 * (-1); 
	s_infy_BattRunData[nBMSGrpMapIdx].fOutputPower = ((float)CAN_StringToShort(pFrmReply->byData + 4)) * 0.1 * (-1); 
	s_infy_BattRunData[nBMSGrpMapIdx].fOutsideVolt = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData1 -- 总压采样值[%0.1f],输出电流值[%0.1f],输出功率值[%0.1f],外端电压采样值[%0.1f]\n",
		s_infy_BattRunData[nBMSGrpMapIdx].fPackTotalVolt,s_infy_BattRunData[nBMSGrpMapIdx].fOutputCurrent,
		s_infy_BattRunData[nBMSGrpMapIdx].fOutputPower,s_infy_BattRunData[nBMSGrpMapIdx].fOutsideVolt);
#endif
	//TRACE("S2C_SumData1 -- fPackTotalVolt:%f, fOutputCurrent:%f, fOutputPower:%f, fOutsideVolt:%f\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fPackTotalVolt, s_infy_BattRunData[nBMSGrpMapIdx].fOutputCurrent, s_infy_BattRunData[nBMSGrpMapIdx].fOutputPower, s_infy_BattRunData[nBMSGrpMapIdx].fOutsideVolt);

	return TRUE;
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
static BOOL Parse_S2C_SumData2(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fSOC = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].fSOE = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1; 
	s_infy_BattRunData[nBMSGrpMapIdx].fSOH = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1; 
	s_infy_BattRunData[nBMSGrpMapIdx].fTotalCapacity = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData2 -- 电池SOC[%0.1f],电池SOE[%0.1f],电池SOH[%0.1f],电池实际总容量[%0.1f]\n",
		s_infy_BattRunData[nBMSGrpMapIdx].fSOC,s_infy_BattRunData[nBMSGrpMapIdx].fSOE,
		s_infy_BattRunData[nBMSGrpMapIdx].fSOH,s_infy_BattRunData[nBMSGrpMapIdx].fTotalCapacity);
#endif
	//TRACE("S2C_SumData2 -- fSOC:%f, fSOE:%f, fSOH:%f, fTotalCapacity:%f\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fSOC, s_infy_BattRunData[nBMSGrpMapIdx].fSOE, s_infy_BattRunData[nBMSGrpMapIdx].fSOH, s_infy_BattRunData[nBMSGrpMapIdx].fTotalCapacity);

	return TRUE;
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
static BOOL Parse_S2C_SumData3(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fCellAddTotalVolt = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].usCellAverageVolt = CAN_StringToUShort(pFrmReply->byData + 2); 
	s_infy_BattRunData[nBMSGrpMapIdx].sCellAverageTemp = CAN_StringToShort(pFrmReply->byData + 4); 
	s_infy_BattRunData[nBMSGrpMapIdx].sBattCabinetTemp = CAN_StringToShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData3 -- 单体累计总压[%0.1f],单体平均电压[%d],单体平均温度[%d],电池柜内温度[%d]\n",
		s_infy_BattRunData[nBMSGrpMapIdx].fCellAddTotalVolt, s_infy_BattRunData[nBMSGrpMapIdx].usCellAverageVolt,
		s_infy_BattRunData[nBMSGrpMapIdx].sCellAverageTemp, s_infy_BattRunData[nBMSGrpMapIdx].sBattCabinetTemp);
#endif
	//TRACE("S2C_SumData3 -- fCellAddTotalVolt:%f, usCellAverageVolt:%d, sCellAverageTemp:%d, sBattCabinetTemp:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fCellAddTotalVolt, s_infy_BattRunData[nBMSGrpMapIdx].usCellAverageVolt, \
	//	s_infy_BattRunData[nBMSGrpMapIdx].sCellAverageTemp, s_infy_BattRunData[nBMSGrpMapIdx].sBattCabinetTemp);

	return TRUE;
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
static BOOL Parse_S2C_SumData4(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Cell = CAN_StringToUShort(pFrmReply->byData);
	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Volt = CAN_StringToUShort(pFrmReply->byData + 2); 
	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Cell = CAN_StringToUShort(pFrmReply->byData + 4); 
	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Volt = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData4 -- 最大电压单体序号:%d, 单体最大电压值:%d, 最小电压单体序号:%d, 单体最小电压值:%d\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Cell, s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Volt, \
		s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Cell, s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Volt);
#endif

	//TRACE("S2C_SumData4 -- usMaxCellVlot_Cell:%d, usMaxCellVlot_Volt:%d, usMinCellVlot_Cell:%d, usMinCellVlot_Volt:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Cell, s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Volt, \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Cell, s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Volt);

	return TRUE;
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
static BOOL Parse_S2C_SumData5(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellTemp_Cell = CAN_StringToUShort(pFrmReply->byData);
	s_infy_BattRunData[nBMSGrpMapIdx].sMaxCellTemp_Temp = CAN_StringToShort(pFrmReply->byData + 2); 
	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellTemp_Cell = CAN_StringToUShort(pFrmReply->byData + 4); 
	s_infy_BattRunData[nBMSGrpMapIdx].sMinCellTemp_Temp = CAN_StringToShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData5 -- 最高单体温度序号:%d, 单体最高温度值:%d, 最低单体温度序号:%d, 单体最低温度值:%d\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellTemp_Cell, s_infy_BattRunData[nBMSGrpMapIdx].sMaxCellTemp_Temp, \
		s_infy_BattRunData[nBMSGrpMapIdx].usMinCellTemp_Cell, s_infy_BattRunData[nBMSGrpMapIdx].sMinCellTemp_Temp);
#endif
	//TRACE("S2C_SumData5 -- usMaxCellTemp_Cell:%d, sMaxCellTemp_Temp:%d, usMinCellTemp_Cell:%d, sMinCellTemp_Temp:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellTemp_Cell, s_infy_BattRunData[nBMSGrpMapIdx].sMaxCellTemp_Temp, \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellTemp_Cell, s_infy_BattRunData[nBMSGrpMapIdx].sMinCellTemp_Temp);

	return TRUE;
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
static BOOL Parse_S2C_SumData7(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fNegativeBusVolt = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1;
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData7 -- 负母线电压:%0.1f\n", s_infy_BattRunData[nBMSGrpMapIdx].fNegativeBusVolt);
#endif
	//TRACE("S2C_SumData7 -- fNegativeBusVolt:%f\n", s_infy_BattRunData[nBMSGrpMapIdx].fNegativeBusVolt);

	return TRUE;
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
static BOOL Parse_S2C_SumData8(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_P_R = ((float)CAN_StringToUShort(pFrmReply->byData + 4));
	s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_N_R = ((float)CAN_StringToUShort(pFrmReply->byData + 6));
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_SumData8 -- 正极绝缘阻值:%f, 负极绝缘阻值:%f\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_P_R, s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_N_R);
#endif
	//TRACE("S2C_SumData8 -- fInsulation_P_R:%f, fInsulation_N_R:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_P_R, s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_N_R);

	return TRUE;
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
static BOOL Parse_S2C_Statistics1(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usSystemRunTime = CAN_StringToUShort(pFrmReply->byData);
	s_infy_BattRunData[nBMSGrpMapIdx].usCanChargeTime = CAN_StringToUShort(pFrmReply->byData + 2); 
	s_infy_BattRunData[nBMSGrpMapIdx].usCanDischargeTime = CAN_StringToUShort(pFrmReply->byData + 4); 
	s_infy_BattRunData[nBMSGrpMapIdx].usCycleTimes = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Statistics1 -- 系统运行时长:%d, 电池可充电时长:%d, 电池可放电时长:%d, 电池充放电循环次数:%d\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].usSystemRunTime, s_infy_BattRunData[nBMSGrpMapIdx].usCanChargeTime, \
		s_infy_BattRunData[nBMSGrpMapIdx].usCanDischargeTime, s_infy_BattRunData[nBMSGrpMapIdx].usCycleTimes);
#endif	
	//TRACE("S2C_Statistics1 -- usSystemRunTime:%d, usCanChargeTime:%d, usCanDischargeTime:%d, usCycleTimes:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usSystemRunTime, s_infy_BattRunData[nBMSGrpMapIdx].usCanChargeTime, \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usCanDischargeTime, s_infy_BattRunData[nBMSGrpMapIdx].usCycleTimes);

	return TRUE;
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
static BOOL Parse_S2C_Statistics2(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usMonAlarmNum = CAN_StringToUShort(pFrmReply->byData);
	s_infy_BattRunData[nBMSGrpMapIdx].usMonFaultNum = CAN_StringToUShort(pFrmReply->byData + 2); 
	s_infy_BattRunData[nBMSGrpMapIdx].usAccuAlarmNum = CAN_StringToUShort(pFrmReply->byData + 4); 
	s_infy_BattRunData[nBMSGrpMapIdx].usAccuFaultNum = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Statistics2 -- 月告警数目:%d, 月故障数目:%d, 累计告警数目:%d, 累计故障数目:%d\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].usMonAlarmNum, s_infy_BattRunData[nBMSGrpMapIdx].usMonFaultNum, \
		s_infy_BattRunData[nBMSGrpMapIdx].usAccuAlarmNum, s_infy_BattRunData[nBMSGrpMapIdx].usAccuFaultNum);
#endif
	//TRACE("S2C_Statistics2 -- usMonAlarmNum:%d, usMonFaultNum:%d, usAccuAlarmNum:%d, usAccuFaultNum:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usMonAlarmNum, s_infy_BattRunData[nBMSGrpMapIdx].usMonFaultNum, \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usAccuAlarmNum, s_infy_BattRunData[nBMSGrpMapIdx].usAccuFaultNum);

	return TRUE;
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
static BOOL Parse_S2C_Statistics3(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fRemainEnergy = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].fTotalEnergy = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].fSingleChgCapacity = CAN_StringToUShort(pFrmReply->byData + 4); 
	s_infy_BattRunData[nBMSGrpMapIdx].fSingleDischgCapacity = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Statistics3 -- 电池剩余电能:%dkWh, 电池总电能:%dkWh, 电池单次充电量:%dAH, 电池单次放电量:%dAH\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].fRemainEnergy, s_infy_BattRunData[nBMSGrpMapIdx].fTotalEnergy, \
		s_infy_BattRunData[nBMSGrpMapIdx].fSingleChgCapacity, s_infy_BattRunData[nBMSGrpMapIdx].fSingleDischgCapacity);
#endif
	//TRACE("S2C_Statistics3 -- fRemainEnergy:%d, fTotalEnergy:%d, fSingleChgCapacity:%d, fSingleDischgCapacity:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fRemainEnergy, s_infy_BattRunData[nBMSGrpMapIdx].fTotalEnergy, \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fSingleChgCapacity, s_infy_BattRunData[nBMSGrpMapIdx].fSingleDischgCapacity);

	return TRUE;
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
static BOOL Parse_S2C_Statistics4(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgCapacity = CAN_StringToUint(pFrmReply->byData); 
	s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgCapacity = CAN_StringToUint(pFrmReply->byData + 4);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Statistics4 -- 电池累计充电量:%dAH, 电池累计放电量:%dAH\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgCapacity, s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgCapacity);
#endif
	//TRACE("S2C_Statistics4 -- fAccuChgCapacity:%d, fAccuDischgCapacity:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgCapacity, s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgCapacity);

	return TRUE;
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
static BOOL Parse_S2C_Statistics5(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgEnergy = ((float)CAN_StringToUint(pFrmReply->byData)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgEnergy = ((float)CAN_StringToUint(pFrmReply->byData + 4)) * 0.1;
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Statistics5 -- 电池累计充电能量:%fKWh, 电池累计放电能量:%fKWh\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgEnergy, s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgEnergy);
#endif
	//TRACE("S2C_Statistics5 -- fAccuChgEnergy:%f, fAccuDischgEnergy:%f\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgEnergy, s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgEnergy);

	return TRUE;
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
static BOOL Parse_S2C_Statistics6(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fNowDOD = ((float)CAN_StringToUShort(pFrmReply->byData)) * 0.1;
	s_infy_BattRunData[nBMSGrpMapIdx].fDischgStartDOD = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Statistics6 -- 电池当前DOD:%f, 放电起始DOD:%f\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].fNowDOD, s_infy_BattRunData[nBMSGrpMapIdx].fDischgStartDOD);
#endif
	//TRACE("S2C_Statistics6 -- fNowDOD:%f, fDischgStartDOD:%f\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].fNowDOD, s_infy_BattRunData[nBMSGrpMapIdx].fDischgStartDOD);

	return TRUE;
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
static BOOL Parse_S2C_Diagnosis(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSLev = CAN_StringToUShort(pFrmReply->byData);
	s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSCode = CAN_StringToUShort(pFrmReply->byData + 2);
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_Diagnosis -- 诊断SOS级别:%d, 诊断SOS异常标志:%d\n", \
		s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSLev, s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSCode);
#endif

	//TRACE("S2C_Diagnosis -- usDiagSOSLev:%d, usDiagSOSCode:%d\n", \
	//	s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSLev, s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSCode);

	return TRUE;
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
static BOOL Parse_S2C_CellData1(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	int nSubIndex = pFrmReply->bySubIndex;
	int nBalanceSubIdx, nArrIdx, nByteNum, nBitNum, n;
	
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if((nSubIndex < CELL_BALANCE_SUB_IDX_START) || (nSubIndex > CELL_BALANCE_SUB_IDX_END))
	{
		return FALSE;
	}

	nBalanceSubIdx = nSubIndex - CELL_BALANCE_SUB_IDX_START;

	for(n = 0; n < A_BALANCE_MSG_DATA_NUM; n++)
	{
		nArrIdx = nBalanceSubIdx * A_BALANCE_MSG_DATA_NUM + n;
		if(nArrIdx >= A_CABINET_CELL_MAX_NUM)
		{	
			break;
		}
		nByteNum = n / 8;
		nBitNum = n % 8;
		s_infy_BattRunData[nBMSGrpMapIdx].byCellBalanceStat[nArrIdx] = (pFrmReply->byData[nByteNum] >> nBitNum) & 0x01;
	}

	// printf("nBMSGrpMapIdx=[%d] S2C_CellData1 -- nBalanceSubIdx:%d 单体均衡状态:\n", nBMSGrpMapIdx, nBalanceSubIdx);
	//for(n=0; n<A_BALANCE_MSG_DATA_NUM; n++)
	//{
	//	printf("%02X ", s_infy_BattRunData[nBMSGrpMapIdx].byCellBalanceStat[nBalanceSubIdx * A_BALANCE_MSG_DATA_NUM + n]);
	//}
	//printf("\n");
	
	return TRUE;
}

static BOOL Parse_S2C_CellData8(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	int nSubIndex = pFrmReply->bySubIndex;
	int nSafeSubIdx, nArrIdx, nByteNum, nBitNum, n;
	
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if((nSubIndex < CELL_SAFE_SUB_IDX_START) || (nSubIndex > CELL_SAFE_SUB_IDX_END))
	{
		return FALSE;
	}

	nSafeSubIdx = nSubIndex - CELL_SAFE_SUB_IDX_START;

	for(n = 0; n < A_SAFE_MSG_DATA_NUM; n++)
	{
		nArrIdx = nSafeSubIdx * A_SAFE_MSG_DATA_NUM + n;
		if(nArrIdx >= A_CABINET_CELL_MAX_NUM)
		{	
			break;
		}
		nByteNum = n / 8;
		nBitNum = n % 8;
		s_infy_BattRunData[nBMSGrpMapIdx].byCellSafeStat[nArrIdx] = (pFrmReply->byData[nByteNum] >> nBitNum) & 0x01;
	}

	// printf("nBMSGrpMapIdx=[%d] S2C_CellData8 -- nSafeSubIdx:%d 单体安全状态:\n", nBMSGrpMapIdx, nSafeSubIdx);
	// for(n=0; n<A_SAFE_MSG_DATA_NUM; n++)
	// {
	// 	printf("%02X ", s_infy_BattRunData[nBMSGrpMapIdx].byCellSafeStat[nSafeSubIdx * A_SAFE_MSG_DATA_NUM + n]);
	// }
	// printf("\n");
	
	return TRUE;
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
static BOOL Parse_S2C_CellData2(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	int nSubIndex = pFrmReply->bySubIndex;
	int nCellVoltSubIdx, nArrIdx, n;
	
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if((nSubIndex < CELL_VOLT_SUB_IDX_START) || (nSubIndex > CELL_VOLT_SUB_IDX_END))
	{
		return FALSE;
	}

	nCellVoltSubIdx = nSubIndex - CELL_VOLT_SUB_IDX_START;

	for(n=0; n<A_VOLT_MSG_DATA_NUM; n++)
	{
		nArrIdx = nCellVoltSubIdx * A_VOLT_MSG_DATA_NUM + n;
		if(nArrIdx >= A_CABINET_CELL_MAX_NUM)
		{	
			break;
		}
		s_infy_BattRunData[nBMSGrpMapIdx].usCellVolt[nArrIdx] = CAN_StringToUShort(pFrmReply->byData + 2*n);
	}

	// printf("nBMSGrpMapIdx=[%d] S2C_CellData2 -- nCellVoltSubIdx:%d, 单体电压:\n", nBMSGrpMapIdx, nCellVoltSubIdx);
	//for(n=0; n<A_VOLT_MSG_DATA_NUM; n++)
	//{
	//	printf("%4X ", s_infy_BattRunData[nBMSGrpMapIdx].usCellVolt[nCellVoltSubIdx * A_VOLT_MSG_DATA_NUM + n]);
	//}
	//printf("\n");
	
	return TRUE;
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
static BOOL Parse_S2C_CellData3(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int A_TEMP_MSG_DATA_NUM = 4;

	if (TEMP_PRECISION_ONE == pFrmReply->pSTBatt_Rough->eBattCellTempPrecison)
	{
		A_TEMP_MSG_DATA_NUM = 8;
	}

	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	int nSubIndex = pFrmReply->bySubIndex;
	int nCellTempSubIdx = 0, nArrIdx = 0, n = 0;
	
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if((nSubIndex < CELL_TEMP_SUB_IDX_START) || (nSubIndex > CELL_TEMP_SUB_IDX_END))
	{
		return FALSE;
	}

	nCellTempSubIdx = nSubIndex - CELL_TEMP_SUB_IDX_START;

	for(n = 0; n < A_TEMP_MSG_DATA_NUM; n++)
	{
		nArrIdx = nCellTempSubIdx * A_TEMP_MSG_DATA_NUM + n;
		if(nArrIdx >= A_CABINET_CELL_MAX_NUM)
		{	
			break;
		}

		if (TEMP_PRECISION_ONE == pFrmReply->pSTBatt_Rough->eBattCellTempPrecison)
		{
			s_infy_BattRunData[nBMSGrpMapIdx].cCellTemp[nArrIdx] = pFrmReply->byData[n];
		}
		else
		{
			s_infy_BattRunData[nBMSGrpMapIdx].cCellTemp[nArrIdx] = ((float)CAN_StringToShort(pFrmReply->byData + 2 * n)) * 0.1;
		}

	}

	// printf("nBMSGrpMapIdx=[%d] S2C_CellData3 -- nCellTempSubIdx:%d, 单体温度:\n", nBMSGrpMapIdx, nCellTempSubIdx);
	// for(n=0; n<A_TEMP_MSG_DATA_NUM; n++)
	// {
	// 	printf("%4X ", s_infy_BattRunData[nBMSGrpMapIdx].usCellVolt[nCellTempSubIdx * A_TEMP_MSG_DATA_NUM + n]);
	// }
	// printf("\n");
	
	return TRUE;
}


static BOOL Parse_S2C_InnerResis_1(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
 	UINT nCellResIdx = 0;

	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
 
	nCellResIdx =  (UINT)pFrmReply->byData[1];
	if((nCellResIdx+3) > A_CABINET_CELL_MAX_NUM)
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx++] = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;
 	s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx++] = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.01;
	s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx] = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.01;

	if(nCellResIdx == A_CABINET_CELL_MAX_NUM - 1)
	{
		int res_flag = FINISHED;
		VAR_VALUE varV;
		varV.emValue = res_flag;
		Push_VarValue(SETTYPE_RAW_SP, pFrmReply->pSTBatt_Rough->hSampler, 501, 3904, varV);
		TRACED("=========Parse_S2C_InnerResis 240 success: %d=========\n", varV.emValue);
	}

#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_InnerResis -- 电池序号%d-内阻[%f], 电池序号%d-内阻[%f], 电池序号%d-内阻[%f]\n",nCellResIdx,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx],
		nCellResIdx++,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx],
		nCellResIdx++,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx]);
#endif	
	return TRUE;
}

static BOOL Parse_S2C_InnerResis_2(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
 	UINT nCellResIdx = 0;

	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
 
	nCellResIdx =  (UINT)pFrmReply->byData[1] + 240;
	if((nCellResIdx+3) > A_CABINET_CELL_MAX_NUM + 1)
	{		
			return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx++] = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;
	s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx++] = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.01;
	if(nCellResIdx == 258)
	{
		return TRUE;
	}
	s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx] = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.01;


	/*if(nCellResIdx == A_CABINET_CELL_MAX_NUM - 1)
	{
		int res_flag = FINISHED;
		VAR_VALUE varV;
		varV.emValue = res_flag;
		Push_VarValue(SETTYPE_RAW_SP, pFrmReply->pSTBatt_Rough->hSampler, 501, 3904, varV);
		TRACED("=========Parse_S2C_InnerResis 240 success: %d=========\n", varV.emValue);
	}*/

	return TRUE;
}


static BOOL Parse_S2C_Zr_InnerResis(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
 	UINT nCellResIdx = 0;

	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM + 1) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
 
	nCellResIdx =  (UINT)pFrmReply->byData[1];
	if((nCellResIdx+3) > A_CABINET_CELL_MAX_NUM)
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usCellZrRes[nCellResIdx++] = CAN_StringToUShort(pFrmReply->byData + 2); 
 	s_infy_BattRunData[nBMSGrpMapIdx].usCellZrRes[nCellResIdx++] = CAN_StringToUShort(pFrmReply->byData + 4);
	s_infy_BattRunData[nBMSGrpMapIdx].usCellZrRes[nCellResIdx] = CAN_StringToUShort(pFrmReply->byData + 6);
	UINT nCellResIdx1 = 0;
	nCellResIdx1 = nCellResIdx - 1;
	UINT nCellResIdx2 = 0;
	nCellResIdx2 = nCellResIdx - 2;
	// printf("Parse_S2C_Zr_InnerResis idx: %d ====value = %d\n", nCellResIdx2, s_infy_BattRunData[nBMSGrpMapIdx].usCellZrRes[nCellResIdx2]);
	// printf("Parse_S2C_Zr_InnerResis idx: %d ====value = %d\n", nCellResIdx1, s_infy_BattRunData[nBMSGrpMapIdx].usCellZrRes[nCellResIdx1]);
	// printf("Parse_S2C_Zr_InnerResis idx: %d ====value = %d\n", nCellResIdx, s_infy_BattRunData[nBMSGrpMapIdx].usCellZrRes[nCellResIdx]);
	

	if(nCellResIdx == A_CABINET_CELL_MAX_NUM - 1) 
	{
		int res_flag = FINISHED;
		VAR_VALUE varV;
		varV.emValue = res_flag;
		Push_VarValue(SETTYPE_RAW_SP, pFrmReply->pSTBatt_Rough->hSampler, 501, 3904, varV);
		// printf("=========Parse_S2C_Zr_InnerResis 240 success: %d=========\n", varV.emValue);
	}
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_InnerResis -- 电池序号%d-内阻[%f], 电池序号%d-内阻[%f], 电池序号%d-内阻[%f]\n",nCellResIdx,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx],
		nCellResIdx++,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx],
		nCellResIdx++,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx]);
#endif	
	return TRUE;
}

static BOOL Parse_S2C_Zi_InnerResis(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
 	UINT nCellResIdx = 0;

	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM + 1) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
 
	nCellResIdx =  (UINT)pFrmReply->byData[1];
	if((nCellResIdx+3) > A_CABINET_CELL_MAX_NUM)
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].usCellZiRes[nCellResIdx++] = CAN_StringToUShort(pFrmReply->byData + 2); 
 	s_infy_BattRunData[nBMSGrpMapIdx].usCellZiRes[nCellResIdx++] = CAN_StringToUShort(pFrmReply->byData + 4);
	s_infy_BattRunData[nBMSGrpMapIdx].usCellZiRes[nCellResIdx] = CAN_StringToUShort(pFrmReply->byData + 6);
	UINT nCellResIdx1 = 0;
	nCellResIdx1 = nCellResIdx - 1;
	UINT nCellResIdx2 = 0;
	nCellResIdx2 = nCellResIdx - 2;
	// printf("Parse_S2C_Zi_InnerResis idx: %d ====value = %d\n", nCellResIdx2, s_infy_BattRunData[nBMSGrpMapIdx].usCellZiRes[nCellResIdx2]);
	// printf("Parse_S2C_Zi_InnerResis idx: %d ====value = %d\n", nCellResIdx1, s_infy_BattRunData[nBMSGrpMapIdx].usCellZiRes[nCellResIdx1]);
	// printf("Parse_S2C_Zi_InnerResis idx: %d ====value = %d\n", nCellResIdx, s_infy_BattRunData[nBMSGrpMapIdx].usCellZiRes[nCellResIdx]);
	

	if(nCellResIdx == A_CABINET_CELL_MAX_NUM - 1)
	{
		int res_flag = FINISHED;
		VAR_VALUE varV;
		varV.emValue = res_flag;
		Push_VarValue(SETTYPE_RAW_SP, pFrmReply->pSTBatt_Rough->hSampler, 501, 3906, varV);
		// printf("=========Parse_S2C_Zi_InnerResis 240 success: %d=========\n", varV.emValue);

	}

#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_InnerResis -- 电池序号%d-内阻[%f], 电池序号%d-内阻[%f], 电池序号%d-内阻[%f]\n",nCellResIdx,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx],
		nCellResIdx++,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx],
		nCellResIdx++,s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx]);
#endif	
	return TRUE;
}


// XSY250417 get Afe workmode -> if not normal(4) -> c2s(YFY_PackC2S_AFESrTest_CtrlCmd) -> get afe staus(s2c_celldata6) -> save data;
static BOOL Parse_S2C_CellAFEWorkMode(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	int nSubIndex = pFrmReply->bySubIndex;
	int nCellAFEWorkModeSubIdx, nArrIdx, n, m, k;
	time_t tmNow = _NOW_;
	
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if((nSubIndex < CELL_AFE_SUB_IDX_START) || (nSubIndex > CELL_AFE_SUB_IDX_END))
	{
		return FALSE;
	}

	nCellAFEWorkModeSubIdx = nSubIndex - CELL_AFE_SUB_IDX_START;

	for(n=0; n<A_AFEWORKMODE_MSG_DATA_NUM; n++)
	{
		nArrIdx = nCellAFEWorkModeSubIdx * A_AFEWORKMODE_MSG_DATA_NUM + n;
		if(nArrIdx >= A_CABINET_CELL_MAX_NUM)
		{	
			break;
		}
		m = n/2;
		k = (n+1)%2;
		s_infy_BattRunData[nBMSGrpMapIdx].byCellAFEWorkMode[nArrIdx] = (pFrmReply->byData[m] >> 4*k) & 0x0F;
		// printf("AFEWM[%d]==[%d]\n", nArrIdx, s_infy_BattRunData[nBMSGrpMapIdx].byCellAFEWorkMode[nArrIdx]);
		if(s_infy_BattRunData[nBMSGrpMapIdx].byCellAFEWorkMode[nArrIdx] != 4) // 4-normal
		{
			s_infy_BattRunData[nBMSGrpMapIdx].bAfeWorkModeAbnormal = 1;
			return TRUE;
		}
	}
	if(fabs(tmNow - s_infy_BattRunData[nBMSGrpMapIdx].t_LastSendC2S_AfeSrTest) > 10)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].bAfeWorkModeAbnormal = 0;
	}


	// printf("nBMSGrpMapIdx=[%d] S2C_CellAFEWorkMode -- nCellAFEWorkModeSubIdx:%d, byCellAFEWorkMode:\n", nBMSGrpMapIdx, nCellAFEWorkModeSubIdx);
	// for(n=0; n<A_AFEWORKMODE_MSG_DATA_NUM; n++)
	// {
	// 	printf("%4X| ", s_infy_BattRunData[nBMSGrpMapIdx].byCellAFEWorkMode[nCellAFEWorkModeSubIdx * A_AFEWORKMODE_MSG_DATA_NUM + n]);
	// }
	// printf("\n");
	
	return TRUE;
}

static BOOL Parse_S2C_CellData6(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
 	UINT nCellAfeIdx = 0;

	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
 
	nCellAfeIdx =  (UINT)pFrmReply->byData[1];
	if((nCellAfeIdx+3) > A_CABINET_CELL_MAX_NUM)
	{
		return FALSE;
	}

	int i;
	for(i=0; i<3 ;i++)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].usNewCellAFESrStatus[nCellAfeIdx+i] = CAN_StringToUShort(pFrmReply->byData + 2*(i+1));
		// printf("AFESrStatus[%d] == [%d]\n", nCellAfeIdx+i, s_infy_BattRunData[nBMSGrpMapIdx].usNewCellAFESrStatus[nCellAfeIdx+i]);
		// if(s_infy_BattRunData[nBMSGrpMapIdx].usNewCellAFESrStatus[nCellAfeIdx+i] != 0 
		// 	&& s_infy_BattRunData[nBMSGrpMapIdx].usNewCellAFESrStatus[nCellAfeIdx+i] != 0x100)
		// {
		// 	s_infy_BattRunData[nBMSGrpMapIdx].usOldAFEAbnormalStatus = s_infy_BattRunData[nBMSGrpMapIdx].usNewCellAFESrStatus[nCellAfeIdx+i];
		// 	// printf("AFESrStatus[%d] == [%d]\n", i, s_infy_BattRunData[nBMSGrpMapIdx].usOldAFEAbnormalStatus);
		// 	return TRUE;
		// }
	}
	// s_infy_BattRunData[nBMSGrpMapIdx].usOldAFEAbnormalStatus = 0;

	return TRUE;
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
static BOOL Parse_S2C_DevInfo(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].bFoundReply = TRUE;
    s_infy_BattRunData[nBMSGrpMapIdx].bBattInfo_Regisited = TRUE;  
	
	s_infy_BattRunData[nBMSGrpMapIdx].byBMS_Alive = pFrmReply->byData[0];
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_DevInfo -- BMS生命信号:%d \n", s_infy_BattRunData[nBMSGrpMapIdx].byBMS_Alive);
#endif
	//TRACE("S2C_DevInfo -- byBMS_Alive:%d \n", s_infy_BattRunData[nBMSGrpMapIdx].byBMS_Alive);

	return TRUE;
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
static BOOL Parse_S2C_CfgInfo(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	if(pFrmReply->byData[1] == 0x06)  //BMS_CellX_Num
	{
		s_infy_BattRunData[nBMSGrpMapIdx].usCellVoltTotalNum = CAN_StringToUShort(pFrmReply->byData + 4) & 0x3FF;  //10 Bit
		s_infy_BattRunData[nBMSGrpMapIdx].usCellTempTotalNum = CAN_StringToUShort(pFrmReply->byData + 6) & 0x3FF;  //10 Bit
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_CfgInfo_0X06 -- 单体电压总数目:%d, 单体温度总数目:%d \n", \
			s_infy_BattRunData[nBMSGrpMapIdx].usCellVoltTotalNum, s_infy_BattRunData[nBMSGrpMapIdx].usCellTempTotalNum);
#endif
		//TRACE("S2C_CfgInfo_0X06 -- usCellVoltTotalNum:%d, usCellTempTotalNum:%d \n", \
		//	s_infy_BattRunData[nBMSGrpMapIdx].usCellVoltTotalNum, s_infy_BattRunData[nBMSGrpMapIdx].usCellTempTotalNum);
	}
	else if(pFrmReply->byData[1] == 0x09)  //BMS_Stand_X
	{
		s_infy_BattRunData[nBMSGrpMapIdx].fStandTotalVoltage = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].fStandCapacity = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].fStandEnergy = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_CfgInfo_0X09 -- Rack标称电压:%f, Rack标称电量:%f, Rack标称电能量:%f \n", \
			s_infy_BattRunData[nBMSGrpMapIdx].fStandTotalVoltage, s_infy_BattRunData[nBMSGrpMapIdx].fStandCapacity, s_infy_BattRunData[nBMSGrpMapIdx].fStandEnergy);
#endif
		//TRACE("S2C_CfgInfo_0X09 -- fStandTotalVoltage:%f, fStandCapacity:%f, fStandEnergy:%f \n", \
		//	s_infy_BattRunData[nBMSGrpMapIdx].fStandTotalVoltage, s_infy_BattRunData[nBMSGrpMapIdx].fStandCapacity, s_infy_BattRunData[nBMSGrpMapIdx].fStandEnergy);
	}
	else if(pFrmReply->byData[1] == 0x0C)  //BMS_X_Version
	{
		memset(s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion));
		memset(s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion));
		memset(s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion));
		sprintf(s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion, "V%d.%d", pFrmReply->byData[3], pFrmReply->byData[2]);
		sprintf(s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion, "V%d.%03d", pFrmReply->byData[5], pFrmReply->byData[4]);//edit by Kuangyue 20250227
		sprintf(s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion, "V%d.%d", pFrmReply->byData[7], pFrmReply->byData[6]);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_CfgInfo_0X0C -- 系统版本号:%s, 软件版本号:%s, 硬件版本号:%s \n", \
			s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion, s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion, s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion);
		printf("S2C_CfgInfo_0X0C byData[]:\n");
		int n;
		for(n=2; n<6; n++)
		{
			printf("%2X ", pFrmReply->byData[n]);
		}
		printf("\n");
#endif
	}

	return TRUE;
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
static BOOL Parse_S2C_IsolatRes(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	s_infy_BattRunData[nBMSGrpMapIdx].emIsoResult = CAN_StringToUShort(pFrmReply->byData + 4); 
#ifdef BATT_CAN_INFO_DEBUG
	printf("nBMSGrpMapIdx=[%d] 绝缘检测状态结果(0:自动检测 1:正在检测 2:停止检测 3:检测完成)=[%d]\n", nBMSGrpMapIdx, s_infy_BattRunData[nBMSGrpMapIdx].emIsoResult);
#endif
	TRACE("nBMSGrpMapIdx=[%d] emIsoResult=[%d]\n", nBMSGrpMapIdx, s_infy_BattRunData[nBMSGrpMapIdx].emIsoResult);

	return TRUE;
}



/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2022-05-5 11:13:53
*  DESCRIPTION   : Total Record Number
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_RecNum(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	if(pFrmReply->byData[1] == 0x00)  
	{
		s_infy_BattRunData[nBMSGrpMapIdx].nFaultRecNum = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].nLogRecNum = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].nWaveRecNum = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x03)	//Multi Frame Research Index
	{
		s_infy_BattRunData[nBMSGrpMapIdx].nDODRecNum = CAN_StringToUShort(pFrmReply->byData + 2);
	}

	TRACEX("Idx=[%d] Total nFaultRecNum=[%d] nLogRecNum=[%d] nWaveRecNum=[%d] nDODRecNum=[%d]\n", \
		nBMSGrpMapIdx,\
		s_infy_BattRunData[nBMSGrpMapIdx].nFaultRecNum,\
		s_infy_BattRunData[nBMSGrpMapIdx].nLogRecNum,\
		s_infy_BattRunData[nBMSGrpMapIdx].nWaveRecNum,\
		s_infy_BattRunData[nBMSGrpMapIdx].nDODRecNum);

	TRACEX("Idx=[%d] Rec Num  %02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|\n", \
		nBMSGrpMapIdx,pFrmReply->byData[0],pFrmReply->byData[1],
		pFrmReply->byData[2], pFrmReply->byData[3],pFrmReply->byData[4],
		pFrmReply->byData[5],pFrmReply->byData[6],pFrmReply->byData[7]);



	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2022-05-5 11:13:53
*  DESCRIPTION   : Fault Record 0x0C1Fyyxx
*				Directly Save To File, No Need Parse
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_FaultRec(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	/*int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if(pFrmReply->byData[0] == 0x00)
	{

	}
	else if(pFrmReply->byData[0] == 0x03)
	{

	}
	else if(pFrmReply->byData[0] == 0x06)
	{

	}
	else if(pFrmReply->byData[0] == 0x09)
	{
		
	}

	TRACEX("Idx=[%d] Fault Record %02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|\n", \
		nBMSGrpMapIdx,pFrmReply->byData[0],pFrmReply->byData[1],
		pFrmReply->byData[2], pFrmReply->byData[3],pFrmReply->byData[4],
		pFrmReply->byData[5],pFrmReply->byData[6],pFrmReply->byData[7]);
	*/
	return TRUE;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : John
*  CREATED DATE  : 2022-05-5 11:13:53
*  DESCRIPTION   : Log Record 0x0b20yyxx  xx-BMS Addr, yy-PCS Addr
**				Directly Save To File, No Need Parse
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_LogRec(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	/*int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	
	if(pFrmReply->byData[0] == 0x00)
	{

	}
	else if(pFrmReply->byData[0] == 0x03)
	{

	}
	else if(pFrmReply->byData[0] == 0x06)
	{

	}
	else if(pFrmReply->byData[0] == 0x09)
	{
		
	}

 	TRACEX("Idx=[%d] Log Rec: %02X|%02X|%02X|%02X|%02X|%02X|%02X|%02X|\n", \
		nBMSGrpMapIdx,pFrmReply->byData[0],pFrmReply->byData[1],
		pFrmReply->byData[2], pFrmReply->byData[3],pFrmReply->byData[4],
		pFrmReply->byData[5],pFrmReply->byData[6],pFrmReply->byData[7]);
	*/
	return TRUE;
}



/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : Jonh
*  CREATED DATE  : 2022-04-13 11:13:53
*  DESCRIPTION   : Wave Record 0x0b21yyxx
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_WaveRec(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	/*
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	s_infy_BattRunData[nBMSGrpMapIdx].bRead_WaveRecOK = TRUE;
	
	TRACEX("nBMSGrpMapIdx=[%d] Wave Rec\n", nBMSGrpMapIdx);
	*/
	return TRUE;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       : Jonh
*  CREATED DATE  : 2022-05-5 11:13:53
*  DESCRIPTION   : DOD Record 0x0b22yyxx
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Parse_S2C_DODRec(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	/*int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	TRACEX("nBMSGrpMapIdx=[%d] DOD Rec\n", nBMSGrpMapIdx);
	*/
	return TRUE;
}

 /*********************************************************************************
 *  
 *  FUNCTION NAME : Parse_S2C_AuthRes 
 *  CREATOR       :  
 *  CREATED DATE  : 2022-07-04 11:31:53
 *  DESCRIPTION   : Get BMS Permission Level
 *				    
 *  
 *  CHANGE HISTORY: 
 *  
 ***********************************************************************************/
 static BOOL Parse_S2C_AuthRes(A_CAN_YFY_BATT_FRAME *pFrmReply)
 {
 	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}
 
 	s_infy_BattRunData[nBMSGrpMapIdx].uiPermissionLevel = CAN_StringToUShort(pFrmReply->byData + 2); 
#ifdef BATT_CAN_INFO_DEBUG
	printf("Parse_S2C_AuthRes, nBMSGrpMapIdx=[%d] 用户权限级别=[%d]\n", nBMSGrpMapIdx, s_infy_BattRunData[nBMSGrpMapIdx].uiPermissionLevel);
#endif
 	TRACE("Parse_S2C_AuthRes, nBMSGrpMapIdx=[%d] uiPermissionLevel=[%d]\n", nBMSGrpMapIdx, s_infy_BattRunData[nBMSGrpMapIdx].uiPermissionLevel);
 
 	return TRUE;
 }


// LJH240715 aircond
static BOOL Parse_S2C_AirCondRd(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	int n;
	VAR_VALUE varV;
	if(pFrmReply->byData[1] == 0x19)  // 开启空调制冷电池温度
	{
		for (n=0; n<6; n++)
		{
			int idxSub = 0 + n;

			float fTemp = (float)(pFrmReply->byData[2+n]);
			varV.fValue = fTemp;
			Push_VarValue(SETTYPE_INTERN_SETT, pFrmReply->pSTBatt_Rough->hSampler, 501+nBMSGrpMapIdx, 8000+idxSub, varV);
			Push_VarValue(SETTYPE_LOGIC_SP, pFrmReply->pSTBatt_Rough->hSampler, 501+nBMSGrpMapIdx, 1950+idxSub, varV);
		}
		TRACEI("Parse_S2C_AirCondRd 25 data:%s", printableBytes(pFrmReply->byRawData, CAN_FRAME_LEN));
	}
	else if(pFrmReply->byData[1] == 0x1F)  // 开启空调制热环境温度
	{
		for (n=0; n<2; n++)
		{
			int idxSub = 6 + n;

			float fTemp = (float)(pFrmReply->byData[2+n]);
			varV.fValue = fTemp;
			Push_VarValue(SETTYPE_INTERN_SETT, pFrmReply->pSTBatt_Rough->hSampler, 501+nBMSGrpMapIdx, 8000+idxSub, varV);
			Push_VarValue(SETTYPE_LOGIC_SP, pFrmReply->pSTBatt_Rough->hSampler, 501+nBMSGrpMapIdx, 1950+idxSub, varV);
		}
		s_infy_BattRunData[nBMSGrpMapIdx].bNeedGetTemp4Aircond = FALSE;
		TRACEI("Parse_S2C_AirCondRd 31 data:%s", printableBytes(pFrmReply->byRawData, CAN_FRAME_LEN));
	}

	return TRUE;
}

static BOOL Parse_S2C_AirCondSet(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	TRACEI("Parse_S2C_AirCondSet OK data:%s", printableBytes(pFrmReply->byRawData, CAN_FRAME_LEN));

	return TRUE;
}


  static BOOL Parse_S2C_AIRINFO1(A_CAN_YFY_BATT_FRAME *pFrmReply)
 {
 	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_AIRINFO1 nBMSGrpMapIdx = [%d] -------------------",nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}
	
	// int n = 0;
	// printf("Parse_S2C_AIRINFO1  Recv: ");
	// for(n=0; n<8; n++)
	// {
	// 	printf("%02X ", pFrmReply->byData[n]);
	// }
	// printf("\n");
	
	if(pFrmReply->byData[1] == 0x00)  
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nPowerStatus = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nWorkMode = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fColdTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;   //0.1 [100-300]
	}
	else if(pFrmReply->byData[1] == 0x03)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nColdOpenDiff = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nColdStopDiff = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fHeatTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;   //0.1 [20-100]
	}
	else if(pFrmReply->byData[1] == 0x06)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fHeatDiff = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;  //0.1 [10-100]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nRefrigerantType = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nUpMachineBaud = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x09)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nContactAddr = CAN_StringToUShort(pFrmReply->byData + 2);
	}

 	return TRUE;
 }

  static BOOL Parse_S2C_AIRINFO2(A_CAN_YFY_BATT_FRAME *pFrmReply)
 {
 	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_AIRINFO2 nBMSGrpMapIdx = [%d] -------------------",nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	// int n = 0;
 	// printf("Parse_S2C_AIRINFO2  Recv: ");
	// for(n=0; n<8; n++)
	// {
	// 	printf("%02X ", pFrmReply->byData[n]);
	// }
	// printf("\n");

	if(pFrmReply->byData[1] == 0x00)  
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nOutsideMachinAbility = CAN_StringToUShort(pFrmReply->byData + 2);
		//s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nSoftVer = CAN_StringToUShort(pFrmReply->byData + 4);
		float fVerTemp = 0.00;
		memset(s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.byBMSAirSoftVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.byBMSAirSoftVersion));

		fVerTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.01;
		sprintf(s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.byBMSAirSoftVersion, "V%.2f", fVerTemp);
		//sprintf(s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.byBMSAirSoftVersion, "V%d.%02d", pFrmReply->byData[4], pFrmReply->byData[5]);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nPLS = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x03)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1TargetCompFreq = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;//0.1HZ
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1CurrentCompFreq = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;//0.1HZ
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nFanTargetSpeed = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x06)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fExhaustSuperheat = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1; //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1AmbientTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1ExhaustTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;  //0.1 [0-65535]
	}
	else if(pFrmReply->byData[1] == 0x09)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1SuctionTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fModuleInletTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fModuleOutletTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1; //0.1 [0-65535]
	}
	else if(pFrmReply->byData[1] == 0x0c)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fBatteryOutletTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fBatteryInletTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nPumpActualSpeed = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x0f)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fModuleOutletPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;  //0.01 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fBatteryOutletPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.01; //0.01 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1HighSatTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;   //0.1 [0-65535]
	}
	else if(pFrmReply->byData[1] == 0x12)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1LowSatTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;   //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSuctionSuperheat = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1LowPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.01;  //0.01 [0-65535]
	}
	else if(pFrmReply->byData[1] == 0x15)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1HighPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;   //0.01 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1ACInputVoltage = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;  //0.1 [0-65535]
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1ACInputCurrent = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;  //0.1 [0-65535]
	}
	else if(pFrmReply->byData[1] == 0x18)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.nSys1CompressorCurrent = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.fSys1DCBusVoltage = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;   //0.1 [0-65535]
	}
 
 	return TRUE;
 }

  static BOOL Parse_S2C_AIRINFO3(A_CAN_YFY_BATT_FRAME *pFrmReply)
 {

 	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_AIRINFO3 nBMSGrpMapIdx = [%d] -------------------",nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	// int n = 0;
	// printf("Parse_S2C_AIRINFO3  Recv: ");
	// for(n=0; n<8; n++)
	// {
	// 	printf("%02X ", pFrmReply->byData[n]);
	// }
	// printf("\n");

 	if(pFrmReply->byData[1] == 0x00)  
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.AirSysFault1.uiFlag = CAN_StringToUShort(pFrmReply->byData + 2);//pFrmReply->byData[2];
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.AirSysFault2.uiFlag = CAN_StringToUShort(pFrmReply->byData + 4);//pFrmReply->byData[4];
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.AirSysFault3.uiFlag = CAN_StringToUShort(pFrmReply->byData + 6);//pFrmReply->byData[6];
	}
	else if(pFrmReply->byData[1] == 0x03)	
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.AirSysFault4.uiFlag = CAN_StringToUShort(pFrmReply->byData + 2);//pFrmReply->byData[2];
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.AirSysFault5.uiFlag = CAN_StringToUShort(pFrmReply->byData + 4);//pFrmReply->byData[4];
		s_infy_BattRunData[nBMSGrpMapIdx].stAirStatusAlarm.AirSysFault6.uiFlag = CAN_StringToUShort(pFrmReply->byData + 6);//pFrmReply->byData[6];
	}

 	return TRUE;
 }

static BOOL Parse_S2C_PACKINFO1(A_CAN_YFY_BATT_FRAME *pFrmReply)//PACK1-5 正极连接器温度 负极连接器温度 水浸频率
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_PACKINFO1 nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	if(pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[0] = ((float)CAN_StringToUShort(pFrmReply->byData + 2));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[0] = ((float)CAN_StringToUShort(pFrmReply->byData + 4));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[0] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO1-0x00 -- PACK1正极连接器温度[%f],PACK1负极连接器温度[%f],PACK1水浸频率[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[0],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[0],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[0]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[1] = ((float)CAN_StringToUShort(pFrmReply->byData + 2));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[1] = ((float)CAN_StringToUShort(pFrmReply->byData + 4));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[1] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO1-0x03 -- PACK2正极连接器温度[%f],PACK2负极连接器温度[%f],PACK2水浸频率[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[1],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[1],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[1]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[2] = ((float)CAN_StringToUShort(pFrmReply->byData + 2));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[2] = ((float)CAN_StringToUShort(pFrmReply->byData + 4));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[2] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO1-0x06 -- PACK3正极连接器温度[%f],PACK3负极连接器温度[%f],PACK3水浸频率[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[2],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[2],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[2]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[3] = ((float)CAN_StringToUShort(pFrmReply->byData + 2));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[3] = ((float)CAN_StringToUShort(pFrmReply->byData + 4));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[3] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO1-0x09 -- PACK4正极连接器温度[%f],PACK4负极连接器温度[%f],PACK4水浸频率[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[3],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[3],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[3]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x0C)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[4] = ((float)CAN_StringToUShort(pFrmReply->byData + 2));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[4] = ((float)CAN_StringToUShort(pFrmReply->byData + 4));
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[4] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO1-0x0C -- PACK5正极连接器温度[%f],PACK5负极连接器温度[%f],PACK5水浸频率[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[4],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[4],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[4]);
#endif

	}

	return TRUE;

}

static BOOL Parse_S2C_PACKINFO2(A_CAN_YFY_BATT_FRAME *pFrmReply)//PACK1-5 高压互锁幅值 高压互锁占空比
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_PACKINFO2 nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	if(pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[0] = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[0] = CAN_StringToUShort(pFrmReply->byData + 4);
		
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[1] = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.01;
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO2-0x00 -- PACK1高压互锁幅值[%d],PACK1高压互锁占空比[%d],PACK2高压互锁幅值[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[0],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[0],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[1]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[1] = CAN_StringToUShort(pFrmReply->byData + 2);

		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[2] = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.01;
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[2] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO2-0x03 -- PACK2高压互锁占空比[%d],PACK3高压互锁幅值[%d],PACK3高压互锁占空比[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[1],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[2],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[2]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[3] = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[3] = CAN_StringToUShort(pFrmReply->byData + 4);

		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[4] = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.01;
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO2-0x06 -- PACK4高压互锁幅值[%d],PACK4高压互锁占空比[%d],PACK5高压互锁幅值[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[3],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[3],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[4]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[4] = CAN_StringToUShort(pFrmReply->byData + 2);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO2-0x09 -- PACK5高压互锁占空比[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[4]);
#endif
	}

	return TRUE;
	
}

static BOOL Parse_S2C_PACKINFO3(A_CAN_YFY_BATT_FRAME *pFrmReply)//PACK1-5 气溶胶浓度 H2浓度 气溶胶状态及故障信息
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_PACKINFO3 nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}


	if(pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[0] = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[0] = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[0] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO3-0x00 -- PACK1气溶胶浓度[%d],PACK1H2浓度[%d],PACK1气溶胶状态及故障信息[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[0],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[0],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[0]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x03)
	{

		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[1] = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[1] = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[1] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO3-0x03 -- PACK2气溶胶浓度[%d],PACK2H2浓度[%d],PACK2气溶胶状态及故障信息[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[1],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[1],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[1]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[2] = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[2] = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[2] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO3-0x06 -- PACK3气溶胶浓度[%d],PACK3H2浓度[%d],PACK3气溶胶状态及故障信息[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[2],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[2],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[2]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[3] = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[3] = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[3] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO3-0x09 -- PACK4气溶胶浓度[%d],PACK4H2浓度[%d],PACK4气溶胶状态及故障信息[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[3],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[3],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[3]);
#endif
	}
	else if(pFrmReply->byData[1] == 0x0C)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[4] = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[4] = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[4] = CAN_StringToUShort(pFrmReply->byData + 6);
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_PACKINFO3-0x0C -- PACK5气溶胶浓度[%d],PACK5H2浓度[%d],PACK5气溶胶状态及故障信息[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[4],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[4],
			s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[4]);
#endif
	}

	return TRUE;
}

static BOOL Parse_S2C_TFAIRPARAINFO(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_TFAIRPARAINFO nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	if(pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nStartupMode = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nWorkingMode = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nSuplyWater = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fRefrigerationPresetTemperature = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fCoolingTemperatureTifference = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fHeatingTresetTemperature = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fHeatingTemperatureTifference = CAN_StringToUShort(pFrmReply->byData + 2);
	}

	return TRUE;
}

static BOOL Parse_S2C_TFAIRSTAINFO(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_TFAIRSTAINFO nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	if(pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nUnitOperatingStatus = (USHORT)pFrmReply->byData[2];
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nUnitOperatingMode = (USHORT)pFrmReply->byData[3];
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nSwitchingInput = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nRelayInput = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fLiquidSupplyTemperature = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fReturnLiquidTemperature = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fSystemSteamingIn = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fSteamOutTemperature = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fAmbientTemperature = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.fSystemCondensationTemperature = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if(pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nCurrentOpeningDegree = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nCurrentSpeedWaterPump = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nCurrentSpeedCompressor = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x0C)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFanSpeed = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nWaterSupplyPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nReturnWaterPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if(pFrmReply->byData[1] == 0x0F)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nExhaustPressure = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
	}

	return TRUE;
}


static BOOL Parse_S2C_TFAIRERRINFO(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_TFAIRERRINFO nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	if(pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus1 = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus2 = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus3 = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus4 = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus5 = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus6 = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if(pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nFaultStatus7 = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nVariableFrequencyFault1 = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stTongFeiAirStatusAlarm.nVariableFrequencyFault2 = CAN_StringToUShort(pFrmReply->byData + 6);
	}

	return TRUE;
}
static BOOL Parse_S2C_IMEU3AIRCONTROLINFO(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	//edit by kuangyue 20250811
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_IMEU3AIRCONTROLINFO nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}
	if (pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.byCellPwrState = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.byPcsPwrState = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.usBatteryMode = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	if (pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.byControlObj = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.fCoolingTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.fHeatingTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	if (pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.byNoiseReductionMode = (CAN_StringToUShort(pFrmReply->byData + 2));
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.fCoolingTempDiff = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.fCoolingStopTempDiff = ((float)CAN_StringToShort(pFrmReply->byData + 6)) * 0.1;
	}
	if (pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.fHeatingTempDiff = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirControlStatus.fHeatingStopTempDiff = ((float)CAN_StringToShort(pFrmReply->byData + 4)) * 0.1;
	}
}

static BOOL Parse_S2C_IMEU3AIRPARAINFO(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	//edit by kuangyue 20250217
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_IMEU3AIRPARAINFO nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}
	if (pFrmReply->byData[1] == 0x00)
	{
		// s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.bSoftVer = CAN_StringToUShort(pFrmReply->byData + 2);
		//float fVerTemp = 0.00;
		//memset(s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.bSoftVer, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.bSoftVer));
		//fVerTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2));
		//sprintf(s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.bSoftVer, "V%.2f", fVerTemp);

		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.bSoftVer = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uPowerStat = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uOpMode = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uOpStep = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uErrCode = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fTgtTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fRetTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fOverheatTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fSatTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uExpValveOpen = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uTotalCap = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uCompTgtHz = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x0C)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uCompOpHz = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.uCompOpPhase = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nFaultLv = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x0F)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fexhaustTempFault = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fEvapInTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fCoolInTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x12)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fModuleOutTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fModuleInTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fBattInTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x15)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fBattOutTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fCoolModOutTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fCoolBattOutTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x18)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fAmbTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fHighPress = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fLowPress = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x1B)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fBattOutPress = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fModuleOutPress = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fCoolInPress = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x1E)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fCoolOutPress = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nFaultLvl = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nVfFault2 = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x21)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nPfcFault = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nCompFault1 = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nCompFault2 = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x24)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.nSysStatFault = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fBattPumpHz = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fBattPumpDuty = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x27)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fModulePumpHz = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fModulePumpDuty = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fDrvFanHz = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x2A)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fDrvFanDuty = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fFanHz = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stIMEU3AirStatusFault.fFanDuty = CAN_StringToUShort(pFrmReply->byData + 6);
	}



	return TRUE;
}
//edit by kuangyue 20250306
static void setXinMiaoAirFaultStatus(XINMIAO_AIR_FAULT_STATUS_BITS* fs, USHORT faultCode) 
{
    fs->uiFlag = 0;

    typedef struct 
	{
        USHORT code;
        int bitIndex;
    } FaultMap;

    FaultMap faultMap[] = 
	{
		{0x00, 0}, {0x03, 1}, {0x04, 2}, {0x05, 3}, {0x06, 4}, {0x07, 5},
        {0x08, 6}, {0x09, 7}, {0x0B, 8}, {0x0F, 9}, {0x10, 10}, {0x11, 11},
        {0x12, 12}, {0x13, 13}, {0x18, 14}, {0x19, 15}, {0x1A, 16}, {0x1D, 17},
        {0x1E, 18}, {0x20, 19}, {0x21, 20}, {0x24, 21}, {0x25, 22}, {0x26, 23},
        {0x27, 24}, {0x29, 25}, {0x2A, 26}, {0x2C, 27}, {0x2E, 28}, {0x2F, 29},
        {0x30, 30}, {0x31, 31}, {0x32, 32}, {0x34, 33}, {0x35, 34}, {0x37, 35},
        {0x41, 36}, {0x33, 37},
    };

    size_t mapSize = sizeof(faultMap) / sizeof(faultMap[0]);
    int found = 0;
	size_t i;
    for ( i = 0; i < mapSize; i++) 
	{
        if (faultMap[i].code == faultCode) 
		{
            fs->uiFlag |= (1ULL << faultMap[i].bitIndex);
            found = 1;
            // break;//单故障模式
        }
    }

    if (!found) TRACED("未知故障代码: 0x%02X\n", faultCode);
}


static BOOL Parse_S2C_XINMIAOAIRPARAINFO1(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	//edit by kuangyue 20250217
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_XINMIAOAIRPARAINFO1 nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}
	if (pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nPumpStat = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nComp1Stat = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nComp2Stat = CAN_StringToUShort(pFrmReply->byData + 6);
	}
	else if (pFrmReply->byData[1] == 0x03)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nHeatStat = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nCoolFanStat = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.fOutTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.1;
	}
	else if (pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.fRetTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.fOutPress = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.01;
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.fRetPress = ((float)CAN_StringToUShort(pFrmReply->byData + 6)) * 0.01;
	}
	else if (pFrmReply->byData[1] == 0x09)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.fenvTempFault = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.1;
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nMakeUpPump = CAN_StringToUShort(pFrmReply->byData + 4);
		// s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.uFaultCode = CAN_StringToUShort(pFrmReply->byData + 6);
		USHORT faultCodeValue = CAN_StringToUShort(pFrmReply->byData + 6);		//edit by kuangyue 20250306
		s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nFaultCode = faultCodeValue;
		XINMIAO_AIR_FAULT_STATUS_BITS* fs = &s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.faultBits;
		setXinMiaoAirFaultStatus(fs, faultCodeValue);
	}

	return TRUE;
}

static BOOL Parse_S2C_AfeEnumlInfo(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	//----------0x10dfffxx----------
	//edit by kuangyue 20250714
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_AfeEnumlInfo nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}
	if (pFrmReply->byData[1] == 0x00)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu1BotNum = (UINT)pFrmReply->byData[2]; // 0x30 -> 0x0030
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu1TopNum = (UINT)pFrmReply->byData[3]; // 0x00 -> 0x0000
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu2BotNum = (UINT)pFrmReply->byData[4]; // 0x30 -> 0x0030
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu2TopNum = (UINT)pFrmReply->byData[5]; // 0x00 -> 0x0000
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu3BotNum = (UINT)pFrmReply->byData[6]; // 0x30 -> 0x0030
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu3TopNum = (UINT)pFrmReply->byData[7]; // 0x00 -> 0x0000
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu4BotNum = (UINT)pFrmReply->byData[2];
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu4TopNum = (UINT)pFrmReply->byData[3];
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu5BotNum = (UINT)pFrmReply->byData[4];
		s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.usBsu5TopNum = (UINT)pFrmReply->byData[5];
	}
	return TRUE;
}

static BOOL Parse_S2C_XINMIAOAIRPARAINFO2(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	//edit by kuangyue 20250217
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_XINMIAOAIRPARAINFO2 nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nWorkMode = CAN_StringToUShort(pFrmReply->byData);
	s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.nPowerStat = CAN_StringToUShort(pFrmReply->byData + 2); 
	s_infy_BattRunData[nBMSGrpMapIdx].stXinMiaoAirStatusFault.fTgtTemp = ((float)CAN_StringToUShort(pFrmReply->byData + 4)) * 0.1; 
}

static BOOL Parse_S2C_LEAKAGEDATA(A_CAN_YFY_BATT_FRAME *pFrmReply)//xsy..
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_LEAKAGEDATA nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	USHORT nFleakCur = CAN_StringToUShort(pFrmReply->byData);
	int sign = (nFleakCur & (1 << 15)) ? -1 : 1; // 符号位
	nFleakCur &= 0x7FFF;
	s_infy_BattRunData[nBMSGrpMapIdx].fLeakageCurrent = ((float)nFleakCur) * sign * 0.01;
		s_infy_BattRunData[nBMSGrpMapIdx].nSoftVer = ((float)CAN_StringToUShort(pFrmReply->byData + 2)) * 0.01;
#ifdef BATT_CAN_INFO_DEBUG
	printf("S2C_LEAKAGEDATA -- 漏电流[%f],软件版本号[%d]\n",s_infy_BattRunData[nBMSGrpMapIdx].fLeakageCurrent,s_infy_BattRunData[nBMSGrpMapIdx].nSoftVer);
#endif
	return TRUE;
}

static BOOL Parse_S2C_LONGALARM(A_CAN_YFY_BATT_FRAME *pFrmReply)//edit by Kuangyue 20241202
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	TRACEW("------------------Parse_S2C_LONGALARM nBMSGrpMapIdx = [%d] -------------------", nBMSGrpMapIdx);
 	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
 	{
 		return FALSE;
 	}

	if(pFrmReply->byData[1] == 0x00)
	{
		UINT nLastAlarmInfo1 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo1;
		UINT nLastAlarmInfo2 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo2;
		UINT nLastAlarmInfo3 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo3;

		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo1 = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo2 = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo3 = CAN_StringToUShort(pFrmReply->byData + 6);

		if((nLastAlarmInfo1 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo1) ||
		   (nLastAlarmInfo2 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo2) ||
		   (nLastAlarmInfo3 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo3))
		   {
			  	SaveBmsCanAlrm(pFrmReply);
		   }

#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_LONGALARM-0x00 -- nAlarmInfo1[%d], nAlarmInfo2[%d], nAlarmInfo3[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo1,
			s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo2,
			s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo3);
#endif
	}
	else if(pFrmReply->byData[1] == 0x03)
	{
		UINT nLastAlarmInfo4 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo4;
		UINT nLastAlarmInfo5 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo5;
		UINT nLastAlarmInfo6 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo6;
		
		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo4 = CAN_StringToUShort(pFrmReply->byData + 2);
		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo5 = CAN_StringToUShort(pFrmReply->byData + 4);
		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo6 = CAN_StringToUShort(pFrmReply->byData + 6);

		if((nLastAlarmInfo4 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo4) ||
		   (nLastAlarmInfo5 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo5) ||
		   (nLastAlarmInfo6 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo6))
		{
			SaveBmsCanAlrm(pFrmReply);
		}

#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_LONGALARM-0x03 -- nAlarmInfo4[%d], nAlarmInfo5[%d], nAlarmInfo6[%d]\n",
			s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo4,
			s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo5,
			s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo6);
#endif
	}
	else if(pFrmReply->byData[1] == 0x06)
	{
		UINT nLastAlarmInfo7 = s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo7;

		s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo7 = CAN_StringToUShort(pFrmReply->byData + 2);

		if(nLastAlarmInfo7 != s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo7)
		{
			SaveBmsCanAlrm(pFrmReply);
		}
#ifdef BATT_CAN_INFO_DEBUG
		printf("S2C_LONGALARM-0x06 -- nAlarmInfo7[%d]\n",s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo7);
#endif
	}

	return TRUE;
}

static A_BLOB_CELL_BARCODE s_barcodeBlob;

static BOOL Load_Barcode_From_DB(SAMPLER* pSampler)
{
    if (pSampler == NULL || pSampler->hDPR == NULL) {
        return FALSE;
    }
    
    static int s_loaded = 0;
    if (s_loaded) {
        return TRUE;  // 已经加载过了
    }
    
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
    VAR_VALUE varValue;
    memset(&varValue, 0, sizeof(VAR_VALUE));
    
    varValue.blobValue = (BYTE*)&s_barcodeBlob;
    
    memset(&s_barcodeBlob, 0, sizeof(s_barcodeBlob));
    
    int result = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 
                           500, 2612, &varValue, 
                           sizeof(A_BLOB_CELL_BARCODE));
    
    if (result == ERR_DPR_OK) {
        s_loaded = 1;
        return TRUE;
    } else {
        return FALSE;
    }
}
// 解析电芯条码
static BOOL Parse_S2C_CellBarcode(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	if (pFrmReply->pSTBatt_Rough != NULL && 
        pFrmReply->pSTBatt_Rough->hSampler != NULL) {
        Load_Barcode_From_DB((SAMPLER*)pFrmReply->pSTBatt_Rough->hSampler);
    }

	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
    int nPackIndex = pFrmReply->byBMSGrpAddr;
    int nSegmentType = pFrmReply->byData[0];
    int nSecondByte = pFrmReply->byData[1];
    int nCellNo = pFrmReply->byData[2];
    int i = 0;
    
    if(nCellNo < 0 || nCellNo >= 52) return FALSE;
    if((nBMSGrpMapIdx >= 15) || (nBMSGrpMapIdx < 0)) return FALSE;
    
    int nSegmentIndex = -1;
    int nBarcodesInSegment = 5;
    
    if(nSegmentType == 0x45 && nSecondByte == 0x00) {
        nSegmentIndex = 0;
    } else if(nSegmentType == 0x05) {
        switch(nSecondByte) {
            case 0x05: nSegmentIndex = 1; break;
            case 0x0A: nSegmentIndex = 2; break;
            case 0x0F: nSegmentIndex = 3; break;
            default: return FALSE;
        }
    } else if(nSegmentType == 0x84 && nSecondByte == 0x14) {
        nSegmentIndex = 4;
        nBarcodesInSegment = 4;
    } else {
        return FALSE;
    }
    
    int startIndex = nSegmentIndex * 5;
    
    for(i = 0; i < nBarcodesInSegment; i++) {
        BYTE rawByte = pFrmReply->byData[3 + i];
        int storageIndex = startIndex + i;
        
        if(storageIndex < 24) {
            // 1. 存储原始数据
            s_barcodeBlob.byRawCellBarcode[nBMSGrpMapIdx][nCellNo][storageIndex] = rawByte;
            
            // 2. 转换并存储ASCII
            if(rawByte >= 32 && rawByte <= 126) {
                s_barcodeBlob.cASCIICellBarcode[nBMSGrpMapIdx][nCellNo][storageIndex] = rawByte;
            } else if(rawByte == 0) {
                s_barcodeBlob.cASCIICellBarcode[nBMSGrpMapIdx][nCellNo][storageIndex] = '\0';
            } else {
                s_barcodeBlob.cASCIICellBarcode[nBMSGrpMapIdx][nCellNo][storageIndex] = '.';
            }
        }
    }

    if(nSegmentIndex == 4) {
        s_barcodeBlob.cASCIICellBarcode[nBMSGrpMapIdx][nCellNo][24] = '\0';

        if(pFrmReply->pSTBatt_Rough != NULL && pFrmReply->pSTBatt_Rough->hSampler != NULL) {
            Push_BlobValue(SETTYPE_RAW_ST,
                          pFrmReply->pSTBatt_Rough->hSampler,
                          500,
                          2612,
                          (void*)&s_barcodeBlob,
                          sizeof(A_BLOB_CELL_BARCODE),
                          BLOB_TYPE_CELL_BARCODE);
        }
    }
    
    #ifdef BATT_CAN_INFO_DEBUG
        printf("\n========== 电芯条码数据 ==========\n");
        printf("Pack%d 电芯%d 分段%d/5\n", nPackIndex, nCellNo, nSegmentIndex + 1);
        
        printf("当前分段数据: ");
        for(i = 3; i < 3 + nBarcodesInSegment; i++) {
            printf("0x%02X ", pFrmReply->byData[i]);
        }
        printf("\n");
        
        if(nSegmentIndex == 4) {
            printf("\n✓ 完整条码接收完成！\n");
            printf("ASCII: \"");
            for(i = 0; i < 24; i++) {
                char ch = s_barcodeBlob.cASCIICellBarcode[nBMSGrpMapIdx][nCellNo][i];
                if(ch == '\0' || ch == 0) break;
                if(ch >= 32 && ch <= 126) printf("%c", ch);
                else printf(".");
            }
            printf("\"\n");
        }
        printf("==================================\n");
    #endif
    
    return TRUE;
}

// 解析Pack条码
static BOOL Parse_S2C_PackBarcode(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	if (pFrmReply->pSTBatt_Rough != NULL && 
        pFrmReply->pSTBatt_Rough->hSampler != NULL) {
        Load_Barcode_From_DB((SAMPLER*)pFrmReply->pSTBatt_Rough->hSampler);
    }

    int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
    int nPackIndex = pFrmReply->byBMSGrpAddr;
    int nSegmentType = pFrmReply->byData[0];
    int nSecondByte = pFrmReply->byData[1];
    int i = 0;
    
    if((nBMSGrpMapIdx >= 15) || (nBMSGrpMapIdx < 0)) return FALSE;
    
    int nSegmentIndex = -1;
    int nBarcodesInSegment = 6;
    
    if(nSegmentType == 0x46 && nSecondByte == 0x00) {
        nSegmentIndex = 0;
    } else if(nSegmentType == 0x06) {
        switch(nSecondByte) {
            case 0x06: nSegmentIndex = 1; break;
            case 0x0C: nSegmentIndex = 2; break;
            default: return FALSE;
        }
    } else if(nSegmentType == 0x82 && nSecondByte == 0x12) {
        nSegmentIndex = 3;
        nBarcodesInSegment = 2;
    } else {
        return FALSE;
    }
    
    int startIndex = nSegmentIndex * 6;
    
    for(i = 0; i < nBarcodesInSegment; i++) {
        BYTE rawByte = pFrmReply->byData[2 + i];
        int storageIndex = startIndex + i;
        
        if(storageIndex < 20) {
            s_barcodeBlob.byRawPackBarcode[nBMSGrpMapIdx][storageIndex] = rawByte;
            
            if(rawByte >= 32 && rawByte <= 126) {
                s_barcodeBlob.cASCIIPackBarcode[nBMSGrpMapIdx][storageIndex] = rawByte;
            } else if(rawByte == 0) {
                s_barcodeBlob.cASCIIPackBarcode[nBMSGrpMapIdx][storageIndex] = '\0';
            } else {
                s_barcodeBlob.cASCIIPackBarcode[nBMSGrpMapIdx][storageIndex] = '.';
            }
        }
    }

    if(nSegmentIndex == 3) {
        s_barcodeBlob.cASCIIPackBarcode[nBMSGrpMapIdx][20] = '\0';
        
        if(pFrmReply->pSTBatt_Rough != NULL && pFrmReply->pSTBatt_Rough->hSampler != NULL) {
            Push_BlobValue(SETTYPE_RAW_ST,
                          pFrmReply->pSTBatt_Rough->hSampler,
                          500,
                          2612,
                          (void*)&s_barcodeBlob,
                          sizeof(A_BLOB_CELL_BARCODE),
                          BLOB_TYPE_CELL_BARCODE);
        }
    }
    
    #ifdef BATT_CAN_INFO_DEBUG
        printf("\n========== Pack条码数据 ==========\n");
        printf("Pack%d 分段%d/4\n", nPackIndex, nSegmentIndex + 1);
        
        printf("当前分段数据: ");
        for(i = 2; i < 2 + nBarcodesInSegment; i++) {
            printf("0x%02X ", pFrmReply->byData[i]);
        }
        printf("\n");
        
        printf("当前累积原始数据: ");
        int currentTotal = startIndex + nBarcodesInSegment;
        for(i = 0; i < currentTotal && i < 20; i++) {
            printf("0x%02X ", s_barcodeBlob.byRawPackBarcode[nBMSGrpMapIdx][i]);
        }
        printf("\n");
        
        printf("当前累积ASCII: \"");
        for(i = 0; i < currentTotal && i < 20; i++) {
            char ch = s_barcodeBlob.cASCIIPackBarcode[nBMSGrpMapIdx][i];
            if(ch == '\0' || ch == 0) break;
            printf("%c", ch);
        }
        printf("\"\n");
        
        if(nSegmentIndex == 3) {
            printf("\n✓ Pack条码接收完成！共%d字节\n", currentTotal);
            printf("完整ASCII条码: \"%s\"\n", s_barcodeBlob.cASCIIPackBarcode[nBMSGrpMapIdx]);
            
            printf("完整原始数据: ");
            for(i = 0; i < 20; i++) {
                printf("0x%02X ", s_barcodeBlob.byRawPackBarcode[nBMSGrpMapIdx][i]);
            }
            printf("\n");
        }
        printf("==================================\n");
    #endif
    
    return TRUE;
}

// 解析BSU条码
static BOOL Parse_S2C_BSUBarcode(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	if (pFrmReply->pSTBatt_Rough != NULL && 
        pFrmReply->pSTBatt_Rough->hSampler != NULL) {
        Load_Barcode_From_DB((SAMPLER*)pFrmReply->pSTBatt_Rough->hSampler);
    }

    int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
    int nPackIndex = pFrmReply->byBMSGrpAddr;
    int nSegmentType = pFrmReply->byData[0];
    int nSecondByte = pFrmReply->byData[1];
    int i = 0;
    
    if((nBMSGrpMapIdx >= 15) || (nBMSGrpMapIdx < 0)) return FALSE;
    
    int nSegmentIndex = -1;
    int nBarcodesInSegment = 6;
    
    if(nSegmentType == 0x46 && nSecondByte == 0x00) {
        nSegmentIndex = 0;
    } else if(nSegmentType == 0x06) {
        switch(nSecondByte) {
            case 0x06: nSegmentIndex = 1; break;
            case 0x0C: nSegmentIndex = 2; break;
            default: return FALSE;
        }
    } else if(nSegmentType == 0x85 && nSecondByte == 0x12) {
        nSegmentIndex = 3;
        nBarcodesInSegment = 5;
    } else {
        return FALSE;
    }
    
    int startIndex = nSegmentIndex * 6;
    
    for(i = 0; i < nBarcodesInSegment; i++) {
        BYTE rawByte = pFrmReply->byData[2 + i];
        int storageIndex = startIndex + i;
        
        if(storageIndex < 23) {
            s_barcodeBlob.byRawBSUBarcode[nBMSGrpMapIdx][storageIndex] = rawByte;
            
            if(rawByte >= 32 && rawByte <= 126) {
                s_barcodeBlob.cASCIIBSUBarcode[nBMSGrpMapIdx][storageIndex] = rawByte;
            } else if(rawByte == 0) {
                s_barcodeBlob.cASCIIBSUBarcode[nBMSGrpMapIdx][storageIndex] = '\0';
            } else {
                s_barcodeBlob.cASCIIBSUBarcode[nBMSGrpMapIdx][storageIndex] = '.';
            }
        }
    }

    if(nSegmentIndex == 3) {
        s_barcodeBlob.cASCIIBSUBarcode[nBMSGrpMapIdx][23] = '\0';
        
        if(pFrmReply->pSTBatt_Rough != NULL && pFrmReply->pSTBatt_Rough->hSampler != NULL) {
            Push_BlobValue(SETTYPE_RAW_ST,
                          pFrmReply->pSTBatt_Rough->hSampler,
                          500,
                          2612,
                          (void*)&s_barcodeBlob,
                          sizeof(A_BLOB_CELL_BARCODE),
                          BLOB_TYPE_CELL_BARCODE);
        }
    }
    
    #ifdef BATT_CAN_INFO_DEBUG
        printf("\n========== BSU条码数据 ==========\n");
        printf("Pack%d BSU 分段%d/4\n", nPackIndex, nSegmentIndex + 1);
        
        printf("当前分段数据: ");
        for(i = 2; i < 2 + nBarcodesInSegment; i++) {
            printf("0x%02X ", pFrmReply->byData[i]);
        }
        printf("\n");
        
        printf("当前累积原始数据: ");
        int currentTotal = startIndex + nBarcodesInSegment;
        for(i = 0; i < currentTotal && i < 23; i++) {
            printf("0x%02X ", s_barcodeBlob.byRawBSUBarcode[nBMSGrpMapIdx][i]);
        }
        printf("\n");
        
        printf("当前累积ASCII: \"");
        for(i = 0; i < currentTotal && i < 23; i++) {
            char ch = s_barcodeBlob.cASCIIBSUBarcode[nBMSGrpMapIdx][i];
            if(ch == '\0' || ch == 0) break;
            printf("%c", ch);
        }
        printf("\"\n");
        
        if(nSegmentIndex == 3) {
            printf("\n✓ BSU条码接收完成！共%d字节\n", currentTotal);
            printf("完整ASCII条码: \"%s\"\n", s_barcodeBlob.cASCIIBSUBarcode[nBMSGrpMapIdx]);
            
            printf("完整原始数据: ");
            for(i = 0; i < 23; i++) {
                printf("0x%02X ", s_barcodeBlob.byRawBSUBarcode[nBMSGrpMapIdx][i]);
            }
            printf("\n");
        }
        printf("==================================\n");
    #endif
    
    return TRUE;
}

// 解析条码写入权限回复
static BOOL Parse_S2C_Barcode_Permission(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nPackIndex = pFrmReply->byBMSGrpAddr;
    int nStatus = pFrmReply->byData[2];

	switch (nStatus) {
		case 0x00:  // 密码错误
			s_barcodeBlob.byPermissionStatus = 0;
			break;
			
		case 0x01:  // 开启写入权限成功
			s_barcodeBlob.byPermissionStatus = 1;
			break;
			
		case 0x02:  // 关闭写入权限成功
			s_barcodeBlob.byPermissionStatus = 2;
			break;
			
		default:
			return FALSE;
	}
	#ifdef BATT_CAN_INFO_DEBUG
		printf("========== 写入权限回复 ==========\n");
		printf("Pack Index: %d\n", nPackIndex);
		printf("Status: 0x%02X ", nStatus);
		switch(nStatus) {
			case 0x00: printf("(密码错误)"); break;
			case 0x01: printf("(开启权限)"); break;
			case 0x02: printf("(关闭权限)"); break;
		}
		printf("\n==================================\n");
	#endif
	
	return TRUE;
}

// 发送电芯条码请求
void YFY_C2S_Query_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int ntype, int nPackIdx, int nCellIdx)
{
	// printf("YFY_C2S_Query_Barcode: ntype=%d, nPackIdx=%d, nCellIdx=%d\n", ntype, nPackIdx, nCellIdx);
    static BYTE byCmdData[CAN_FRAME_LEN] = {0};
    ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
    BYTE* pbySendBuf = byCmdData;
    
    BYTE byCRC;
    A_CAN_YFY_BATT_FRAME frm;
    ZERO_POBJS(&frm, 1);

	BYTE bySubIdx = 0;
	BOOL bNeedCellIdx = FALSE;
	switch(ntype) {
		case 0:
			bySubIdx = 0xE8;
			bNeedCellIdx = TRUE;
			break;
		case 1:
			bySubIdx = 0xE7;
			break;
		case 2:
			bySubIdx = 0xE6;
			break;
		default:
			return;
    }
    
    frm.byBMSGrpAddr = (BYTE)(nPackIdx);
    frm.byEMSAddr = 0x5A;
    
    YFY_CAN_StuffMsgID(0x0B, bySubIdx, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
    
    UINT nMsgID = (pbySendBuf[1]<<24) | (pbySendBuf[2]<<16) | (pbySendBuf[3]<<8) | pbySendBuf[4];

    frm.byData[0] = 0xC0; 
    frm.byData[1] = 0x00; 
    frm.byData[2] = bNeedCellIdx ? (BYTE)nCellIdx : 0x00;
    frm.byData[3] = 0x00; 
    frm.byData[4] = 0x00; 
    frm.byData[5] = 0x00; 
    frm.byData[6] = 0x00;

    byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
    frm.byData[7] = byCRC;
    
    memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

    ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, 
                                                     (void *)pbySendBuf, 
                                                     CAN_FRAME_LEN);
    if(nWriteSize < 0)
    {
        TRACE("Write To CAN Port Failed !!!\n");
    }

    TRACE("SUCCESS: CAN frame with ID 0x%08X sent successfully\n", nMsgID);
    
    return;
}

// 写入条码前后发送开启关闭权限
void YFY_C2S_Barcode_WritePermission(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nPackIdx, BOOL bOpenPermission)
{
    static BYTE byCmdData[CAN_FRAME_LEN] = {0};
    ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
    BYTE* pbySendBuf = byCmdData;
    
    BYTE byCRC;
    A_CAN_YFY_BATT_FRAME frm;
    ZERO_POBJS(&frm, 1);

    frm.byBMSGrpAddr = (BYTE)(nPackIdx);
    frm.byEMSAddr = 0x5A;
    BYTE bySubIdx = 0xE9;
    
    YFY_CAN_StuffMsgID(0x05, bySubIdx, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
    
    UINT nMsgID = (pbySendBuf[1]<<24) | (pbySendBuf[2]<<16) | (pbySendBuf[3]<<8) | pbySendBuf[4];

	frm.byData[0] = 0xC4;
    frm.byData[1] = 0x00;
    if (bOpenPermission) {
        // 开启写入权限密码：20251212
        frm.byData[2] = 0x20;
        frm.byData[3] = 0x25;
        frm.byData[4] = 0x12;
        frm.byData[5] = 0x12;
    } else {
        // 关闭写入权限密码：13572468
        frm.byData[2] = 0x13;
        frm.byData[3] = 0x57;
        frm.byData[4] = 0x24;
        frm.byData[5] = 0x68;
    }
	frm.byData[6] = 0x00;
	// frm.byData[7] = 0x00;

    byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
    frm.byData[7] = byCRC;
    
    memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, 
                                                     (void *)pbySendBuf, 
                                                     CAN_FRAME_LEN);
    if(nWriteSize < 0)
    {
        TRACE("Write To CAN Port Failed !!!\n");
        return;
    }

	#ifdef BATT_CAN_INFO_DEBUG
		printf("========== %s写入权限报文 ==========\n", 
			bOpenPermission ? "开启" : "关闭");
		printf("完整CAN帧 (%d字节):\n", CAN_FRAME_LEN);
		int j;
		for (j = 0; j < CAN_FRAME_LEN; j++) {
			printf("0x%02X ", pbySendBuf[j]);
			if ((j + 1) % 8 == 0) printf("\n");
		}
		printf("\n");
		
		printf("帧ID: 0x%08X\n", nMsgID);
		printf("数据部分 (8字节):\n");
		int i;
		for (i = 0; i < 8; i++) {
			printf("0x%02X ", frm.byData[i]);
		}
		printf("\n");
	#endif

    TRACE("SUCCESS: Write permission frame with ID 0x%08X sent successfully\n", nMsgID);
}

// 写入条码
void YFY_C2S_Write_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int ntype, int nPackIdx, int nCellIdx, char *szBarcode)
{
	s_barcodeBlob.byPermissionStatus = -1;
	YFY_C2S_Barcode_WritePermission(pSTBatt_Rough, nPackIdx, TRUE);
	Sleep(150);

	if(s_barcodeBlob.byPermissionStatus != 1)
	{
		TRACE("ERROR: Pack%d - Write permission not opened!\n", nPackIdx);
		return;
	}

	BOOL bSendSuccess = TRUE;
	switch (ntype) {
        case 0:
            bSendSuccess = Send_Cell_Barcode(pSTBatt_Rough, nPackIdx, nCellIdx, szBarcode);
            break;
            
        case 1:
            bSendSuccess = Send_Pack_Barcode(pSTBatt_Rough, nPackIdx, szBarcode);
            break;
            
        case 2:
            bSendSuccess = Send_BSU_Barcode(pSTBatt_Rough, nPackIdx, szBarcode);
            break;
            
        default:
            TRACE("ERROR: Unknown barcode type %d\n", ntype);
            return;
    }

	Sleep(100);
	YFY_C2S_Barcode_WritePermission(pSTBatt_Rough, nPackIdx, FALSE);
}
// 电芯条码
BOOL Send_Cell_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
                                 int nPackIdx, 
                                 int nCellIdx, 
                                 const char *szBarcode)
{
    BYTE segmentHeaders[5][2] = {
        {0x45, 0x00},
        {0x05, 0x05},
        {0x05, 0x0A},
        {0x05, 0x0F},
        {0x84, 0x14}
    };
	int nBarcodeLen = strlen(szBarcode);
	BOOL bSendSuccess = TRUE;

	int nSegment;
	for (nSegment = 0; nSegment < 5; nSegment++) {
		static BYTE byCmdData[CAN_FRAME_LEN] = {0};
		ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
		BYTE* pbySendBuf = byCmdData;
		
		A_CAN_YFY_BATT_FRAME frm;
		ZERO_POBJS(&frm, 1);

		frm.byBMSGrpAddr = (BYTE)(nPackIdx);
        frm.byEMSAddr = 0x5A;

	    YFY_CAN_StuffMsgID(0x05, 0xE8, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);

		frm.byData[0] = segmentHeaders[nSegment][0];
        frm.byData[1] = segmentHeaders[nSegment][1];
		frm.byData[2] = (BYTE)nCellIdx;

		int nStartIndex = nSegment * 5;
		int i;
        for (i = 0; i < 5; i++) {
            int nCharIndex = nStartIndex + i;
            if (nCharIndex < nBarcodeLen) {
                frm.byData[3 + i] = (BYTE)szBarcode[nCharIndex];
            } else {
                frm.byData[3 + i] = 0x00;
            }
        }
        
        if (nSegment == 4 && nBarcodeLen < 24) {
            frm.byData[7] = 0x00;
        }
        
        memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
        
        ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, 
                                                 (void *)pbySendBuf, 
                                                 CAN_FRAME_LEN);
        if (nWriteSize < 0) {
            TRACE("ERROR: Failed to send segment %d!\n", nSegment + 1);
			bSendSuccess = FALSE;
            break;
        }
        
        #ifdef BATT_CAN_INFO_DEBUG
            printf("分段%d数据: ", nSegment + 1);
			int x;
            for (x = 0; x < 8; x++) {
                printf("0x%02X ", frm.byData[x]);
            }
            printf(" | 字符: ");
			int j;
            for (j = 3; j < 8; j++) {
                if (frm.byData[j] >= 32 && frm.byData[j] <= 126) {
                    printf("%c", frm.byData[j]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
        #endif
        
        TRACE("分段 %d/5 发送成功\n", nSegment + 1);
	}
    
    return bSendSuccess;
}
// Pack条码
BOOL Send_Pack_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
                                      int nPackIdx, 
                                      const char *szBarcode)
{
    BYTE segmentHeaders[4][2] = {
        {0x46, 0x00},
        {0x06, 0x06},
        {0x06, 0x0C},
        {0x82, 0x12}
    };
    
    int nBarcodeLen = strlen(szBarcode);
    BOOL bSendSuccess = TRUE;
    
	int nSegment;
    for (nSegment = 0; nSegment < 4; nSegment++) {
        static BYTE byCmdData[CAN_FRAME_LEN] = {0};
        ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
        BYTE* pbySendBuf = byCmdData;
        
        A_CAN_YFY_BATT_FRAME frm;
        ZERO_POBJS(&frm, 1);

        frm.byBMSGrpAddr = (BYTE)(nPackIdx);
        frm.byEMSAddr = 0x5A;

        YFY_CAN_StuffMsgID(0x05, 0xE7, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);

        frm.byData[0] = segmentHeaders[nSegment][0];
        frm.byData[1] = segmentHeaders[nSegment][1];
        
        int nStartIndex = nSegment * 6;
        int i;
        for (i = 0; i < 6; i++) {
            int nCharIndex = nStartIndex + i;
            if (nCharIndex < nBarcodeLen) {
                frm.byData[2 + i] = (BYTE)szBarcode[nCharIndex];
            } else {
                frm.byData[2 + i] = 0x00;
            }
        }
        
        if (nSegment == 3) {
            if (nBarcodeLen < 20) {
                int nFilledChars = nBarcodeLen - nStartIndex;
                if (nFilledChars < 0) nFilledChars = 0;
                int l;
                for (l = nFilledChars; l < 6; l++) {
                    frm.byData[2 + l] = 0x00;
                }
            }
        }
        
        memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
        
        ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, 
                                                         (void *)pbySendBuf, 
                                                         CAN_FRAME_LEN);
        if (nWriteSize < 0) {
            TRACE("ERROR: Failed to send Pack barcode segment %d!\n", nSegment + 1);
            bSendSuccess = FALSE;
            break;
        }

        #ifdef BATT_CAN_INFO_DEBUG
            printf("Pack条码分段%d: ", nSegment + 1);
			int x;
            for (x = 0; x < 8; x++) {
                printf("0x%02X ", frm.byData[x]);
            }
            printf(" | 字符: ");

            int j;
            for (j = 2; j < 8; j++) {  // 从索引2开始是条码数据
                if (frm.byData[j] >= 32 && frm.byData[j] <= 126) {
                    printf("%c", frm.byData[j]);
                } else if (frm.byData[j] == 0x00) {
                    printf("\\0");
                } else {
                    printf(".");
                }
            }
            printf("\n");
        #endif
        
        TRACE("Pack条码分段 %d/4 发送成功\n", nSegment + 1);
       
    }
    
    return bSendSuccess;
}
// BSU条码
BOOL Send_BSU_Barcode(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, 
                                     int nPackIdx, 
                                     const char *szBarcode)
{
    BYTE segmentHeaders[4][2] = {
        {0x46, 0x00},
        {0x06, 0x06},
        {0x06, 0x0C},
        {0x85, 0x12}
    };
    
    int nBarcodeLen = strlen(szBarcode);
    BOOL bSendSuccess = TRUE;
    
	int nSegment;
    for (nSegment = 0; nSegment < 4; nSegment++) {
        static BYTE byCmdData[CAN_FRAME_LEN] = {0};
        ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
        BYTE* pbySendBuf = byCmdData;
        
        A_CAN_YFY_BATT_FRAME frm;
        ZERO_POBJS(&frm, 1);

        frm.byBMSGrpAddr = (BYTE)(nPackIdx);
        frm.byEMSAddr = 0x5A;

        YFY_CAN_StuffMsgID(0x05, 0xE6, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);

        frm.byData[0] = segmentHeaders[nSegment][0];
        frm.byData[1] = segmentHeaders[nSegment][1];
        
        int nStartIndex = nSegment * 6;
        int i;
        for (i = 0; i < 6; i++) {
            int nCharIndex = nStartIndex + i;
            if (nCharIndex < nBarcodeLen) {
                frm.byData[2 + i] = (BYTE)szBarcode[nCharIndex];
            } else {
                frm.byData[2 + i] = 0x00;
            }
        }
        
         if (nSegment == 3) {
            frm.byData[7] = 0x00;
            
            int nRemainingChars = nBarcodeLen - nStartIndex;
            if (nRemainingChars < 0) nRemainingChars = 0;
            
            if (nRemainingChars < 5) {
				int l;
                for (l = nRemainingChars; l < 5; l++) {
                    frm.byData[2 + l] = 0x00;
                }
            }
        }
        
        memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
        
        ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, 
                                                         (void *)pbySendBuf, 
                                                         CAN_FRAME_LEN);
        if (nWriteSize < 0) {
            TRACE("ERROR: Failed to send BSU barcode segment %d!\n", nSegment + 1);
            bSendSuccess = FALSE;
            break;
        }

		#ifdef BATT_CAN_INFO_DEBUG
            printf("BSU条码分段%d: ", nSegment + 1);
            printf("Header: 0x%02X 0x%02X | Data: ", frm.byData[0], frm.byData[1]);
			int x;
            for (x = 2; x < 8; x++) {
                printf("0x%02X ", frm.byData[x]);
            }
            printf(" | 字符: ");
			int j;
            for (j = 2; j < 8; j++) {
                if (frm.byData[j] >= 32 && frm.byData[j] <= 126) {
                    printf("%c", frm.byData[j]);
                } else if (frm.byData[j] == 0x00) {
                    printf("_");
                } else {
                    printf(".");
                }
            }
            printf("\n");
        #endif
        
        TRACE("BSU条码分段 %d/4 发送成功\n", nSegment + 1);
    }
    
    return bSendSuccess;
}

 static void YFY_PackC2S_PermissionLev_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
 {
 	BYTE szPassword[2][4] = { {0x00, 0x00, 0x00, 0x00}, {0x00, 0x01, 0x5A, 0xEE} };
 	UINT emIndex = pSTBatt_Rough->emEnterSuperuser[nBattIdx];
	emIndex = (emIndex >= 2) ? 0 : emIndex;

 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x55, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0xC2;
 	frm.byData[1] = 0x00;
 	frm.byData[2] = szPassword[emIndex][0];
 	frm.byData[3] = szPassword[emIndex][1];
 	frm.byData[4] = szPassword[emIndex][2];
 	frm.byData[5] = szPassword[emIndex][3];
 	frm.byData[6] = 0x00;
 
 	TRACE("YFY_PackC2S_PermissionLev_CtrlCmd Send\n");
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
 
 	return;
 }
 
 /*********************************************************************************
 *  
 *  FUNCTION NAME :  YFY_PackC2S_ClearFault_CtrlCmd
 *  CREATOR       :  
 *  CREATED DATE  : 2023-07-04 16:23
 *  DESCRIPTION   :  
 *				    
 *  
 *  CHANGE HISTORY: 
 *  
 ***********************************************************************************/
 static void YFY_PackC2S_ClearFault_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
 {
 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x02;
 	frm.byData[2] = (pSTBatt_Rough->emClearFaultType[nBattIdx] == 0) ? 0x01 : 0x0D;
 	frm.byData[3] = 0x00;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	TRACE("YFY_PackC2S_ClearFault_CtrlCmd Send\n");
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
 
 	return;
}

 /*********************************************************************************
 *  
 *  FUNCTION NAME :  YFY_PackC2S_EmsStatu_CtrlCmd
 *  CREATOR       :  
 *  CREATED DATE  : 2024-04-16 16:23
 *  DESCRIPTION   :  
 *				    
 *  
 *  CHANGE HISTORY: 
 *  
 ***********************************************************************************/
static void YFY_PackC2S_EmsStatu_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
 {
 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x03;
 	frm.byData[2] = s_infy_BattRunData[nBattIdx].unioLigHT.szByLigHTState[0];
 	frm.byData[3] = s_infy_BattRunData[nBattIdx].unioLigHT.szByLigHTState[1];
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	TRACE("YFY_PackC2S_EmsStatu_CtrlCmd Send\n");
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
 
 	return;
}

static void YFY_PackC2S_CMD_QF3(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	static BYTE byCmdData[CAN_FRAME_LEN] = {0};
	ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
	BYTE* pbySendBuf = byCmdData;

	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = 0xFF;
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x57, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x0C;
	frm.byData[2] = 0x01;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00; 
	frm.byData[5] = 0x00; 
	frm.byData[6] = 0x00;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
   	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
    return;
}


static void YFY_PackC2S_EU3Air_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, int Value)
 {
	static BYTE byCmdData[CAN_FRAME_LEN] = {0};
	ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
	BYTE* pbySendBuf = byCmdData;

 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x04;
 	frm.byData[2] = Value == 0? 0x00 : 0x01;
 	frm.byData[3] = 0x00;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	TRACE("YFY_PackC2S_EmsStatu_CtrlCmd Send\n");
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

  	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nBattIdx, pbySendBuf, 1);
		}
	}
 	return;
}

static void YFY_PackC2S_Rint_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
 {
 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x05;
 	frm.byData[2] = pSTBatt_Rough->nRintFreqIdx[nBattIdx] & 0xFF;
 	frm.byData[3] = (pSTBatt_Rough->nRintFreqIdx[nBattIdx] >> 8) & 0xFF;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;

	// printf("frm.byData: ");
	// int i;
	// for (i = 0; i <= 6; ++i) {
    // 	printf("0x%02X ", frm.byData[i]);
	// }
	// printf("\n");
 
 	TRACE("YFY_PackC2S_Rint_CtrlCmd Send\n");
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
 
 	return;
}

//XSY250417
static void YFY_PackC2S_AFESrTest_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
 {
 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
	
	BYTE byAfeSrTestCmd = 1;
  
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x06;
	frm.byData[2] = byAfeSrTestCmd & 0xFF;
	frm.byData[3] = (byAfeSrTestCmd >> 8) & 0xFF;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	TRACE("YFY_PackC2S_AFESrTest_CtrlCmd Send\n");
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
 
 	return;
}

static void YFY_PackC2S_EU3AirNoise_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx,  BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x08;
	if(pSTBatt_Rough->emAirNoise[nBattIdx] == 0)
	{
		frm.byData[2] = 0;
	}
	else if(pSTBatt_Rough->emAirNoise[nBattIdx] == 1)
	{
		frm.byData[2] = 0x01;
	}
	else if(pSTBatt_Rough->emAirNoise[nBattIdx] == 2)
	{
		frm.byData[2] = 0x02;
	}
	else if(pSTBatt_Rough->emAirNoise[nBattIdx] == 3)
	{
		frm.byData[2] = 0x03;
	}
	else if(pSTBatt_Rough->emAirNoise[nBattIdx] == 4)
	{
		frm.byData[2] = 0x04;
	}
	else if(pSTBatt_Rough->emAirNoise[nBattIdx] == 5)
	{
		frm.byData[2] = 0x05;
	}
	else if(pSTBatt_Rough->emAirNoise[nBattIdx] == 6)
	{
		frm.byData[2] = 0x06;
	}
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	TRACE("YFY_PackC2S_EU3AirNoise_CtrlCmd Send: %02X %02X %02X %02X\n",\
		frm.byData[0], frm.byData[1], frm.byData[2], frm.byData[3]);

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	return;
}

static void YFY_PackC2S_KeepPowerMode_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx,  BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];

	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x1D;
	if(pSTBatt_Rough->bKeepPowerMode[nBattIdx] == 0)
	{
		frm.byData[2] = 0;
	}
	else if(pSTBatt_Rough->bKeepPowerMode[nBattIdx] == 1)
	{
		frm.byData[2] = 0x01;
	}
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	TRACE("YFY_PackC2S_KeepPowerMode_CtrlCmd Send: %02X %02X %02X %02X\n",\
		frm.byData[0], frm.byData[1], frm.byData[2], frm.byData[3]);

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	return;
}


/*********************************************************************************
*  FUNCTION NAME :  YFY_PackC2S_AfeReset_CtrlCmd
*  DESCRIPTION   :  INFY C2S_CMD_AfeReset, CAN 0x0556xxyy C1 1F 01 [PACK] 00 00 00 00
*                  When cell enters Safe state, remote AFE reset for target PACK
***********************************************************************************/
static void YFY_PackC2S_AfeReset_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);

	frm.byData[0] = 0xC1;  /* CMD_Frame_Flag */
	frm.byData[1] = 0x1F;  /* CMD_Frame_SubId for AFE Reset */
	frm.byData[2] = 0x01;  /* AFE Reset: 1=reset */
	frm.byData[3] = (BYTE)(pSTBatt_Rough->Afe_Reset_PackNum[nBattIdx]);  /* PACK 1-5 */
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;  /* CRC not used */

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	TRACE("YFY_PackC2S_AfeReset_CtrlCmd nBattIdx=%d PACK=%d\n", nBattIdx, nBattIdx + 1);
	return;
}

/*********************************************************************************
*  FUNCTION NAME :  YFY_PackC2S_BsuReset_CtrlCmd
*  DESCRIPTION   :  INFY C2S_CMD_BsuReset, CAN 0x0556xxyy C1 20 01 [PACK] 00 00 00 00
*                  After AFE reset, remote BSU reset for target PACK
***********************************************************************************/
static void YFY_PackC2S_BsuReset_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);

	frm.byData[0] = 0xC1;  /* CMD_Frame_Flag */
	frm.byData[1] = 0x20;  /* CMD_Frame_SubId for BSU Reset */
	frm.byData[2] = 0x01;  /* BSU Reset: 1=reset */
	frm.byData[3] = (BYTE)(pSTBatt_Rough->Bus_Reset_PackNum[nBattIdx]);  /* PACK 1-5 */
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;  /* CRC not used */

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	TRACE("YFY_PackC2S_BsuReset_CtrlCmd nBattIdx=%d PACK=%d\n", nBattIdx, nBattIdx + 1);
	return;
}

static void YFY_PackC2S_CalibratedSOC_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx,  BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xc1;
	frm.byData[1] = 0x1E;
	frm.byData[2] = pSTBatt_Rough->nCalibratedSOC & 0xFF;
	frm.byData[3] = pSTBatt_Rough->nCalibratedSOC >> 8;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	printf("YFY_PackC2S_CalibratedSOC_CtrlCmd Send: %02X %02X %02X %02X\n",\
		frm.byData[0], frm.byData[1], frm.byData[2], frm.byData[3]);

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	return;
}

static void YFY_PackC2S_WorkMode_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
 {
 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);
 
 	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x01;
	if(pSTBatt_Rough->emWorkMode[nBattIdx] == 0)
	{
		frm.byData[2] = 0;
	}
	else if(pSTBatt_Rough->emWorkMode[nBattIdx] == 1)
	{
		frm.byData[2] = 0x01;
	}
	else if(pSTBatt_Rough->emWorkMode[nBattIdx] == 2)
	{
		frm.byData[2] = 0x02;
	}
	else
	{
		frm.byData[2] = 0x05;
	}

 	frm.byData[3] = 0x00;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	TRACE("YFY_PackC2S_WorkMode_CtrlCmd Send: %02X %02X %02X %02X\n",\
		frm.byData[0], frm.byData[1], frm.byData[2], frm.byData[3]);
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
 
 	return;
 }

void QueryBSUVersion(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	static BYTE byCmdData[CAN_FRAME_LEN] = {0};
	ZERO_POBJS(byCmdData, CAN_FRAME_LEN);
	BYTE* pbySendBuf = byCmdData;

	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = 0xFF;
 	frm.byBMSGrpAddr  = 0xFF;
 
 	YFY_CAN_StuffMsgID(0x1B, 0xF0, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
  
 	frm.byData[0] = 0x00;
 	frm.byData[1] = 0x00;
	frm.byData[2] = 0x00;
 	frm.byData[3] = 0x00;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
   	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);

	if(nWriteSize < 0)
	{
		printf("Write To CAN Port Failed !!!\n");
	}

	return;
}
static BOOL Parse_C2S_UpgradeStatu(A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}
	SAMPLER* pSampler = (SAMPLER*) pFrmReply->pSTBatt_Rough->hSampler;
	if(pFrmReply->byData[4] == 0xfb && pFrmReply->byData[5] == 0x55)
	{//parse BSU Soft version
		char szversion[16] = {0};
		sprintf(szversion, "V%d.%02d", pFrmReply->byData[7], pFrmReply->byData[6]); // LJH260227 %02X->%02d
		snprintf(pFrmReply->pSTBatt_Rough->szBsuVersion, sizeof(pFrmReply->pSTBatt_Rough->szBsuVersion), "%s", szversion);
		//printf("BSU Version: %s\n", szversion);
		int idx = pFrmReply->byRawData[4];
		if(idx <= 9)
		{
			Push_TextValue(SETTYPE_RAW_SP, pSampler, 500, 868 + idx, szversion, 16);
		}
	} 

	STBATT_SAMPLER_ROUGH *pSTBatt_Rough = pFrmReply->pSTBatt_Rough;
	UPGRADE_DEV_TYPE eDevType = pSTBatt_Rough->stUpgradeParam.eDevType;
	BYTE byDevAddr = pSTBatt_Rough->stUpgradeParam.byDevAddr;
	if (pFrmReply->byRawData[3] != eDevType || pFrmReply->byRawData[4] != byDevAddr)
	{
		TRACE("filter upgrade frame: ");
		int n = 0;
		for (n = 0; n < CAN_FRAME_LEN; ++n)
		{
			TRACE("%02X ", pFrmReply->byRawData[n]);
		}
		TRACE("\n");
		return FALSE;
	}
	
	memcpy(s_infy_BattRunData[nBMSGrpMapIdx].byData, pFrmReply->byData, CAN_DATA_AREA_LEN);
	
#if 0
	s_infy_BattRunData[nBMSGrpMapIdx].nRemtDownIndex = ((int)pFrmReply->byData[0] & 0x0ff) | (((int)pFrmReply->byData[1] & 0x000f) << 8);
	s_infy_BattRunData[nBMSGrpMapIdx].emRemtReptStat = (BMS_REPORT_STATE)(pFrmReply->byData[1] >> 4);
	s_infy_BattRunData[nBMSGrpMapIdx].nRemtRunModel = pFrmReply->byData[2];
	s_infy_BattRunData[nBMSGrpMapIdx].nRemtRunState = pFrmReply->byData[3];
	s_infy_BattRunData[nBMSGrpMapIdx].nRemtVersion = ((int)pFrmReply->byData[4] & 0xff) | (((int)pFrmReply->byData[5] & 0x00ff) << 8) | (((int)pFrmReply->byData[6] & 0x00ff) << 16) | (((int)pFrmReply->byData[7] & 0x00ff) << 24);

	TRACE("nRemtDownIndex=[%d] emRemtReptStat=[%d] nRemtRunModel=[%d] nRemtRunState=[%d] nRemVersion=[%d]\n",\
		s_infy_BattRunData[nBMSGrpMapIdx].nRemtDownIndex, \
		s_infy_BattRunData[nBMSGrpMapIdx].emRemtReptStat, \
		s_infy_BattRunData[nBMSGrpMapIdx].nRemtRunModel, \ 
		s_infy_BattRunData[nBMSGrpMapIdx].nRemtRunState, \ 
		s_infy_BattRunData[nBMSGrpMapIdx].nRemtVersion);
#endif
	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Read Total Record Number
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_RD_RecNum(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
 	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x0B, 0x1E, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC0;
	frm.byData[1] = 0x00;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//TRACEX("---nBattIdx=[%d] RD_RecNum---nMsgID=[%d]\n", nBattIdx, nMsgID);
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
static void YFY_PackC2S_CMD_HV_Farme(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx,  BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x56, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x00;
	frm.byData[2] = (BYTE)s_infy_BattRunData[nBattIdx].byPCS_Cmd;
	frm.byData[3] = (BYTE)(s_infy_BattRunData[nBattIdx].byPCS_Cmd >> 8);
	frm.byData[4] = 0x00; 
	frm.byData[5] = 0x00; 
	frm.byData[6] = 0x00;

	//TRACE("s_infy_BattRunData[nBattIdx].byPCS_Cmd:%d\n", s_infy_BattRunData[nBattIdx].byPCS_Cmd);

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :	
*  CREATOR		 :	
*  CREATED DATE  : 2022-11-09 18:31:55
*  DESCRIPTION	 :	
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_Reboot_BMS(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x1F, 0xF1, 0xFF, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0x01;
	frm.byData[1] = 0x02;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	TRACE("---nBattIdx=[%d] Reboot_BMS---nMsgID=[%d]\n", nBattIdx, nMsgID);
	return;
}
/*********************************************************************************
*  
*  FUNCTION NAME :  YFY_PackC2S_ISO_CtrlCmd
*  CREATOR       :  
*  CREATED DATE  : 2022-08-23 11:13:53
*  DESCRIPTION   :  
*				    
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_ISO_CtrlCmd(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx,  BYTE *pbySendBuf)
{
	if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus == TRIG_NO_TRIGGER)
	{
		return;
	}

	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x57, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x07;
	frm.byData[2] = (BYTE)pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus;
	frm.byData[3] = (BYTE)(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus >> 8);
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	TRACE("s_infy_BattRunData[nBattIdx].byPCS_Cmd:%d\n", s_infy_BattRunData[nBattIdx].byPCS_Cmd);

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  struct tm {
	int tm_sec; 	//代表目前秒数，正常范围为0-59，但允许至61秒 /
	int tm_min; 	//代表目前分数，范围0-59/
	int tm_hour; 	//从午夜算起的时数，范围为0-23/
	int tm_mday; 	//目前月份的日数，范围01-31/
	int tm_mon; 	//代表目前月份，从一月算起，范围从0-11//
	int tm_year; 	//从1900 年算起至今的年数//
	int tm_wday; 	//一星期的日数，从星期一算起，范围为0-6 //
	int tm_yday; 	//从今年1月1日算起至今的天数，范围为0-365//
	int tm_isdst; 	//日光节约时间的旗标//
	};
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_RTC_Time_Farme(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
 	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	struct tm gmTime;
	float fTimezone = GetTimeZone_OffSetHrs(pSTBatt_Rough->hSampler);
	double douTmNow = (fTimezone * 3600) + GetCurrentTime();
	time_t tmNow = (time_t)douTmNow;
	double douTmNowDec = douTmNow - tmNow;
	USHORT usMS = (SHORT)(douTmNowDec * 1000);
	gmtime_r(&tmNow, &gmTime);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x00, 0x00, 0xFF, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	 
	TRACE("gmTime.tm_year[%d], gmTime.tm_mday[%d], gmTime.tm_mon[%d], gmTime.tm_min[%d],  gmTime.tm_hour[%d], gmTime.tm_sec[%d]\n",\
		gmTime.tm_year, gmTime.tm_mday, gmTime.tm_mon, gmTime.tm_min,  gmTime.tm_hour, gmTime.tm_sec );

	UINT nYear = gmTime.tm_year  + 1900; 
	frm.byData[0] = (BYTE)nYear; //Low First  
	frm.byData[1] = (BYTE)(nYear >> 8);
	frm.byData[2] = (BYTE)gmTime.tm_mday;
	frm.byData[3] = (BYTE)(gmTime.tm_mon + 1);
	frm.byData[4] = (BYTE)gmTime.tm_min;
	frm.byData[5] = (BYTE)gmTime.tm_hour;
	frm.byData[6] = (BYTE)usMS;
	frm.byData[7] = (BYTE)(usMS >> 8);

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Read Fault Record
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_RD_FaultRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x0B, 0x1F, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC0;
	frm.byData[1] = 0x00;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] RD_FaultRec---nMsgID=[%d]\n", nBattIdx, nMsgID);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Confirm Fault Record
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_Confirm_FaultRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x27, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x02;
	frm.byData[2] = 0x5B;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_FaultRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  Read Log Record
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_RD_LogRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
 	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x0B, 0x20, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC0;
	frm.byData[1] = 0x00;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
 	frm.byData[7] = 0x00;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] RD_LogRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_Confirm_LogRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x27, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x03;
	frm.byData[2] = 0x5B;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_LogRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}



/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_RD_WaveRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
 	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x0B, 0x21, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC0;
	frm.byData[1] = 0x00;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
 	frm.byData[7] = 0x00;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] RD_WaveRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:13:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_Confirm_WaveRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x27, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x04;
	frm.byData[2] = 0x5B;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_WaveRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:19:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_RD_DODRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x0B, 0x22, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC0;
	frm.byData[1] = 0x00;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
 	frm.byData[7] = 0x00;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] RD_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-05-05 11:19:53
*  DESCRIPTION   :  
*				
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_PackC2S_Confirm_DODRec(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x05, 0x27, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC1;
	frm.byData[1] = 0x05;
	frm.byData[2] = 0x5B;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;

	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

// LJH240715 aircond
static void YFY_PackC2S_RD_AirCond(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x0B, 0x68, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = 0xC0;
	frm.byData[1] = 0x19;
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	TRACED("can_infy_batt::YFY_PackC2S_RD_AirCond nBattIdx:%d Send<%s>", nBattIdx, printableBytes(pbySendBuf, CAN_FRAME_LEN));
	return;
}

static void YFY_PackC2S_SetAirCond(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf, YFY_C2S_CMD emCmdNum)
{
 	BYTE byCRC;
 	A_CAN_YFY_BATT_FRAME frm;
 	ZERO_POBJS(&frm, 1);
 
 	frm.byEMSAddr  = GetEMSAddr(nBattIdx);
 	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

 	YFY_CAN_StuffMsgID(0x05, 0x6D, frm.byBMSGrpAddr, frm.byEMSAddr, CAN_DATA_AREA_LEN, pbySendBuf);
 	
 	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
	int offset = emCmdNum - CAN_S_CMD_C2S_AIRCOND_BATT_COOLON;
 	frm.byData[0] = 0xC1;
 	frm.byData[1] = 0x19 + offset;
 	frm.byData[2] = pSTBatt_Rough->nSetTemp4Aircond;
 	frm.byData[3] = 0x00;
 	frm.byData[4] = 0x00;
 	frm.byData[5] = 0x00;
 	frm.byData[6] = 0x00;
 
 	byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
 	frm.byData[7] = byCRC;
 
 	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);
	TRACED("can_infy_batt::YFY_PackC2S_SetAirCond nBattIdx:%d offset:%d nSetTemp4Aircond:%d Send<%s>",
		nBattIdx, offset, pSTBatt_Rough->nSetTemp4Aircond, printableBytes(pbySendBuf, CAN_FRAME_LEN));
 	return;
}

static void YFY_C2M_SelfCheck_Res1(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x32, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_V_Num & 0xFF;
	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_V_Num >> 8) & 0xFF;
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_T_Num & 0xFF;
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_T_Num >> 8) & 0xFF;
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Tot_V & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Tot_V >> 8) & 0xFF; // 高字节
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Add_V & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Add_V >> 8) & 0xFF; // 高字节

	// PRINT for debug
	// printf("----- inside RES_1: batt %d -----\n", nBattIdx);
	// printf("V num: %u\n", frm.byData[0]);
	// printf("T num: %u\n", frm.byData[2]);
	// unsigned int test;
	// test = frm.byData[4] | (frm.byData[5] << 8);
	// printf("Total V: %d\n", test);
	// printf("Total V before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Tot_V);
	// unsigned int test_2;
	// test_2 = frm.byData[6] | (frm.byData[7] << 8);
	// printf("Total add V: %d\n", test_2);
	// printf("Total add V before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Add_V);

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	// printf("---nBattIdx=[%d] selfcheck_res1 ---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_SelfCheck_Res2(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x33, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MaxVoltage & 0xFF; // 低字节
	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MaxVoltage >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MinVoltage & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MinVoltage >> 8) & 0xFF; // 高字节
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_AvgVolt & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_AvgVolt >> 8) & 0xFF; // 高字节
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeVolt & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeVolt >> 8) & 0xFF; // 高字节

	// printf("----- inside RES_2: batt %d -----\n", nBattIdx);
	// unsigned int test;
	// test = frm.byData[0] | (frm.byData[1] << 8);
	// printf("Max V: %d\n", test);
	// printf("Max V before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MaxVoltage);
	// unsigned int test_2;
	// test_2 = frm.byData[6] | (frm.byData[7] << 8);
	// printf("V range: %d\n", test_2);
	// printf("V range before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeVolt);
	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	// printf("---nBattIdx=[%d] selfcheck_res2 ---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_SelfCheck_Res3(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x34, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MaxTemperature & 0xFF; // 低字节
	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MaxTemperature >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MinTemperature & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MinTemperature >> 8) & 0xFF; // 高字节
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_AvgTemp & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_AvgTemp >> 8) & 0xFF; // 高字节
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeTemperature & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeTemperature >> 8) & 0xFF; // 高字节

	// printf("----- inside RES_3: batt %d -----\n", nBattIdx);
	// unsigned int test;
	// test = frm.byData[0] | (frm.byData[1] << 8);
	// printf("Max T: %d\n", test);
	// printf("Max T before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_MaxTemperature);
	// unsigned int test_2;
	// test_2 = frm.byData[6] | (frm.byData[7] << 8);
	// printf("T range: %d\n", test_2);
	// printf("T range before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeTemperature);

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	// printf("---nBattIdx=[%d] selfcheck_res3 ---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_SelfCheck_Res4(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x35, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_TotAdd_Gap & 0xFF; // 低字节
	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_TotAdd_Gap >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeRes & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeRes >> 8) & 0xFF; // 高字节
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;

	// printf("SSSSSSSSSSRES4444444:::RES_RANGE: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_RangeRes);
	// printf("SSSSSSSSSSRES4444444:::2,3: %d, %d\n", frm.byData[2], frm.byData[3]);

	// printf("----- inside RES_4: batt %d -----\n", nBattIdx);
	// unsigned int test;
	// test = frm.byData[0] | (frm.byData[1] << 8);
	// printf("add vol gap: %d\n", test);
	// printf("add vol gap before: %d\n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_TotAdd_Gap);

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	// printf("---nBattIdx=[%d] selfcheck_res4 ---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_ChargeCheck_Res1(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x37, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LMaxTemperature & 0xFF; // 低字节
	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LMaxTemperature >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LRangeTemperature & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LRangeTemperature >> 8) & 0xFF; // 高字节
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LMaxVol & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LMaxVol >> 8) & 0xFF; // 高字节
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LTermRangeVol & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LTermRangeVol >> 8) & 0xFF; // 高字节

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;
	// printf("BMS_LTermRangeVol : %d/n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LTermRangeVol);

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_ChargeCheck_Res2(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x38, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LMinTemperature & 0xFF; // 低字节
 	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LMinTemperature >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Charge_Energy & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Charge_Energy >> 8) & 0xFF; // 高字节
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Charge_Capacity & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Charge_Capacity >> 8) & 0xFF; // 高字节
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LPlateauRangeVol & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LPlateauRangeVol >> 8) & 0xFF; // 高字节

	// printf("BMS_LPlateauRangeVol : %d/n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LPlateauRangeVol);
	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_ChargeCheck_Res3(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x39, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LRangeRes & 0xFF; // 低字节
 	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LRangeRes >> 8) & 0xFF; // 高字节
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;

	// printf("BMS_LPlateauRangeVol : %d/n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_LPlateauRangeVol);
	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_DischargeCheck_Res1(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x40, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SMaxTemperature & 0xFF; // 低字节
	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SMaxTemperature >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SRangeTemperature & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SRangeTemperature >> 8) & 0xFF; // 高字节
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SMinVol & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SMinVol >> 8) & 0xFF; // 高字节
	// frm.byData[6] = 0x00;
	// frm.byData[7] = 0x00;
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_STermRangeVol & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_STermRangeVol >> 8) & 0xFF; // 高字节
	// printf("BMS_STermRangeVol : %d/n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_STermRangeVol);

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_DischargeCheck_Res2(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x41, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SMinTemperature & 0xFF; // 低字节
 	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SMinTemperature >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Discharge_Energy & 0xFF; // 低字节
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Discharge_Energy >> 8) & 0xFF; // 高字节
	frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Discharge_Capacity & 0xFF; // 低字节
	frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Discharge_Capacity >> 8) & 0xFF; // 高字节
	// frm.byData[6] = 0x00;
	// frm.byData[7] = 0x00;
	frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SPlateauRangeVol & 0xFF; // 低字节
	frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SPlateauRangeVol >> 8) & 0xFF; // 高字节
	// printf("BMS_SPlateauRangeVol : %d/n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SPlateauRangeVol);

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_DischargeCheck_Res3(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x44, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SRangeRes & 0xFF; // 低字节
 	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SRangeRes >> 8) & 0xFF; // 高字节
	frm.byData[2] = 0x00;
	frm.byData[3] = 0x00;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;
	// printf("BMS_SPlateauRangeVol : %d/n", pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_SPlateauRangeVol);

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_Total_Res(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf)
{
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x42, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
 
	frm.byData[0] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_EnergyEfficiency & 0xFF; // 低字节
 	frm.byData[1] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_EnergyEfficiency >> 8) & 0xFF; // 高字节
	frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_TotalSysEfficiency & 0xFF;//xsy
	frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_TotalSysEfficiency >> 8) & 0xFF;
	frm.byData[4] = 0x00;
	frm.byData[5] = 0x00;
	frm.byData[6] = 0x00;
	frm.byData[7] = 0x00;

	// byCRC = Crc_ComputeCRC8((nMsgID & 0xffff), &frm.byData[0], 7);
	// frm.byData[7] = byCRC;

	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

static void YFY_C2M_HeartBeat(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx, BYTE *pbySendBuf, int teststate)
{	
	// printf("send out heartbeat, n: %d, teststate: %d\n", nBattIdx, teststate);
	//BYTE byCRC;
	A_CAN_YFY_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	//frm.byEMSAddr  = GetEMSAddr(nBattIdx);
	frm.byBMSGrpAddr  = GetBMSAddr(nBattIdx);

	YFY_CAN_StuffMsgID(0x15, 0x43, 0xff, frm.byBMSGrpAddr, CAN_DATA_AREA_LEN, pbySendBuf);
	
	UINT nMsgID = (pbySendBuf[1]<<24) + (pbySendBuf[2]<<16) + (pbySendBuf[3]<<8) + pbySendBuf[4];
	
	switch (teststate)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		frm.byData[0] = teststate;
 		frm.byData[1] = 0x00;
		frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Self_Result & 0xFF; // 低字节
 		frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Self_Result >> 8) & 0xFF; // 高字节
 		frm.byData[4] = 0x00;
 		frm.byData[5] = 0x00;
 		frm.byData[6] = 0x00;
 		frm.byData[7] = 0x00;		
		break;

	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
		frm.byData[0] = teststate;
 		frm.byData[1] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Total_Result;
		frm.byData[2] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Self_Result & 0xFF; // 低字节
 		frm.byData[3] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Self_Result >> 8) & 0xFF; // 高字节
		frm.byData[4] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Charge_Result & 0xFF; // 低字节
		frm.byData[5] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_Charge_Result >> 8) & 0xFF; // 高字节
		frm.byData[6] = pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_DisCharge_Result & 0xFF; // 低字节
		frm.byData[7] = (pSTBatt_Rough->bmsAutoTestResult[nBattIdx].BMS_DisCharge_Result >> 8) & 0xFF; // 高字节		
		break;

	default:
		break;
	}

	// printf("byData[0]: %d\n",  frm.byData[0]);
 	
	memcpy(pbySendBuf + 5, frm.byData, CAN_DATA_AREA_LEN);

	//printf("---nBattIdx=[%d] Confirm_DODRec---nMsgID=[%d]\n", nBattIdx, nMsgID);
 	return;
}

#define  INIT_YFYBATT_CMD(p, _emMsgType, _emFuncCode, _nSubIdx, _pfnHandler) \
	((p)->emMsgType = (_emMsgType), \
	(p)->nFuncCode = (_emFuncCode), \
	(p)->nSubIndex = (_nSubIdx), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)++);
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
void YFY_Init_RoughData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n)
{
	//int nBattPackCellNum = 0;

	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];

	s_infy_BattRunData[n].emBattSysState = YFYBATT_SYSSTAT_HV_DOWN;
	s_infy_BattRunData[n].emBattChgState = YFYBATT_CHGSTAT_IDLE;
	s_infy_BattRunData[n].fAllow_MaxChargeCurr 		= 0.0;
	s_infy_BattRunData[n].fAllow_MaxDisChargeCurr 	= 0.0;
	s_infy_BattRunData[n].fAllow_MaxChargePow 		= 0.0;
	s_infy_BattRunData[n].fAllow_MaxDisChargePow 	= 0.0;
	s_infy_BattRunData[n].fAllow_MaxChargeVolt 		= 0.0;
	s_infy_BattRunData[n].fAllow_MaxDisChargeVolt 	= 0.0;
	memset(s_infy_BattRunData[n].unioBattAlarmCode.szByIfyAlarm, 	0,	CAN_DATA_AREA_LEN);
	memset(s_infy_BattRunData[n].unioBattFaultCode.szByIfyFault, 	0,	CAN_DATA_AREA_LEN);
	memset(s_infy_BattRunData[n].unioBattDIDOStat.szByDIDOStat,		0,	CAN_DATA_AREA_LEN);
 	s_infy_BattRunData[n].fPackTotalVolt 	= 0.0;
	s_infy_BattRunData[n].fOutputCurrent 	= 0.0;
	s_infy_BattRunData[n].fOutputPower 		= 0.0;
	s_infy_BattRunData[n].fOutsideVolt 		= 0.0;
	s_infy_BattRunData[n].fSOC 				= 0.0;
	s_infy_BattRunData[n].fSOH 				= 0.0;
	s_infy_BattRunData[n].fSOE 				= 0.0;
	s_infy_BattRunData[n].fTotalCapacity 	= 0.0;
	s_infy_BattRunData[n].fCellAddTotalVolt = 0.0;
	s_infy_BattRunData[n].usCellAverageVolt = 0;
	s_infy_BattRunData[n].sCellAverageTemp 	= 0;
	s_infy_BattRunData[n].sBattCabinetTemp 	= 0;
	s_infy_BattRunData[n].usMaxCellVlot_Cell = 0;
	s_infy_BattRunData[n].usMaxCellVlot_Volt = 0;
	s_infy_BattRunData[n].usMinCellVlot_Cell = 0;
	s_infy_BattRunData[n].usMinCellVlot_Volt = 0;
	s_infy_BattRunData[n].usMaxCellTemp_Cell = 0;
	s_infy_BattRunData[n].sMaxCellTemp_Temp = 0;
	s_infy_BattRunData[n].usMinCellTemp_Cell = 0;
	s_infy_BattRunData[n].sMinCellTemp_Temp = 0;
	s_infy_BattRunData[n].fNegativeBusVolt = 0.0;
	s_infy_BattRunData[n].fInsulation_P_R = 0.0;
	s_infy_BattRunData[n].fInsulation_N_R = 0.0;
	s_infy_BattRunData[n].usSystemRunTime = 0;
	s_infy_BattRunData[n].usCanChargeTime = 0;
	s_infy_BattRunData[n].usCanDischargeTime = 0;
	s_infy_BattRunData[n].usCycleTimes = 0;
	s_infy_BattRunData[n].usMonAlarmNum = 0;
	s_infy_BattRunData[n].usMonFaultNum = 0;
	s_infy_BattRunData[n].usAccuAlarmNum = 0.;
	s_infy_BattRunData[n].usAccuFaultNum = 0;
	s_infy_BattRunData[n].fRemainEnergy = 0.0;
	s_infy_BattRunData[n].fTotalEnergy = 0.0;
	s_infy_BattRunData[n].fSingleChgCapacity = 0.0;
	s_infy_BattRunData[n].fSingleDischgCapacity = 0.0;
	s_infy_BattRunData[n].fAccuChgCapacity = 0.0;
	s_infy_BattRunData[n].fAccuDischgCapacity = 0.0;
	s_infy_BattRunData[n].fAccuChgEnergy = 0.0;
	s_infy_BattRunData[n].fAccuDischgEnergy = 0.0;
	s_infy_BattRunData[n].fNowDOD = 0.0;
	s_infy_BattRunData[n].fDischgStartDOD = 0.0;
	s_infy_BattRunData[n].usDiagSOSLev = 0;
	s_infy_BattRunData[n].usDiagSOSCode = 0;
	s_infy_BattRunData[n].emIsoResult = STAT_AUTO_DETECTION;

	memset(s_infy_BattRunData[n].byCellBalanceStat, 0, sizeof(s_infy_BattRunData[n].byCellBalanceStat));
	memset(s_infy_BattRunData[n].byCellSafeStat, 0, sizeof(s_infy_BattRunData[n].byCellSafeStat));
	memset(s_infy_BattRunData[n].usCellVolt, 		0, sizeof(s_infy_BattRunData[n].usCellVolt));
	memset(s_infy_BattRunData[n].cCellTemp, 		0, sizeof(s_infy_BattRunData[n].cCellTemp));
	memset(s_infy_BattRunData[n].fCellRes, 			0, sizeof(s_infy_BattRunData[n].fCellRes));
	memset(s_infy_BattRunData[n].usCellZrRes, 		0, sizeof(s_infy_BattRunData[n].usCellZrRes));
	memset(s_infy_BattRunData[n].usCellZiRes, 		0, sizeof(s_infy_BattRunData[n].usCellZiRes));
	memset(s_infy_BattRunData[n].byCellAFEWorkMode, 4, sizeof(s_infy_BattRunData[n].byCellAFEWorkMode));//XSY250417 4--normal
	memset(s_infy_BattRunData[n].usNewCellAFESrStatus, 0, sizeof(s_infy_BattRunData[n].usNewCellAFESrStatus));
	s_infy_BattRunData[n].bAfeWorkModeAbnormal 		= 0; // do not need afeSrTest

	s_infy_BattRunData[n].byBMS_Alive 			= 0;
	s_infy_BattRunData[n].usCellVoltTotalNum 	= 0;
	s_infy_BattRunData[n].usCellTempTotalNum 	= 0;
	s_infy_BattRunData[n].fStandTotalVoltage 	= 0.0;
	s_infy_BattRunData[n].fStandCapacity 		= 0.0;
	s_infy_BattRunData[n].fStandEnergy 			= 0.0;
	
	//s_infy_BattRunData[n].usSystemVersion 	= 0.0;
	//s_infy_BattRunData[n].usSoftwareVersion 	= 0.0;
	
	s_infy_BattRunData[n].fLeakageCurrent	= 0.0;
	s_infy_BattRunData[n].nSoftVer 		= 0.0;

	//edit by Kuangyue 20241202
	s_infy_BattRunData[n].nAlarmInfo1 	= 0;
	s_infy_BattRunData[n].nAlarmInfo2 	= 0;
	s_infy_BattRunData[n].nAlarmInfo3 	= 0;
	s_infy_BattRunData[n].nAlarmInfo4 	= 0;
	s_infy_BattRunData[n].nAlarmInfo5 	= 0;
	s_infy_BattRunData[n].nAlarmInfo6 	= 0;
	s_infy_BattRunData[n].nAlarmInfo7 	= 0;
	
	pSingleRunInfo->fPackTotalVolt 			= 0;
	pSingleRunInfo->fPackTotalCurr 			= 0;
	pSingleRunInfo->nSOC 					= 0;
	pSingleRunInfo->nSOH 					= 0;
	pSingleRunInfo->fAllow_LmtChargeVolt 	= 0;
	pSingleRunInfo->fAllow_LmtDisChargeVolt = 0;
	pSingleRunInfo->fAllow_MaxChargeCurr 	= 0;
	pSingleRunInfo->fAllow_MaxDisChargeCurr = 0;
	pSingleRunInfo->fAllow_MaxChargePow 	= 0;
	pSingleRunInfo->fAllow_MaxDisChargePow = 0;

    pSingleRunInfo->fMax_CellVolt 			= 0;
    pSingleRunInfo->fMin_CellVolt 			= 0;
	pSingleRunInfo->fAvg_CellVolt			= 0;

    pSingleRunInfo->fMax_CellTemp 			= 0;
    pSingleRunInfo->fMin_CellTemp 			= 0;
    pSingleRunInfo->fAvg_CellTemp 			= 0;
	
	pSingleRunInfo->nLifeCycleTimes 		= 0;
	pSingleRunInfo->fVersion	 			= 0;
	
	//char *pEnd;
	//pSingleRunInfo->fVersion = strtof(&s_infy_BattRunData[n].bySoftwareVersion[1], &pEnd);
	
	memset(pSingleRunInfo->szFault, 		0,	CAN_DATA_AREA_LEN);
	memset(pSingleRunInfo->szAlarm, 		0,	CAN_DATA_AREA_LEN);
	memset(pSingleRunInfo->szDIDOStatu,		0,	CAN_DATA_AREA_LEN);


	pSingleRunInfo->uiBAT_Ucell_Max_CSC		= 0;
	pSingleRunInfo->uiBAT_Ucell_Min_CSC		= 0;
	pSingleRunInfo->uiBAT_Ucell_Max_CELL 	=  0;
	pSingleRunInfo->uiBAT_Ucell_Min_CELL 	=  0;

	pSingleRunInfo->nBAT_T_Max_CSC 			= 0;
	pSingleRunInfo->nBAT_T_Min_CSC 			= 0;
	pSingleRunInfo->uiBAT_T_Max_CELL		= 0;
	pSingleRunInfo->uiBAT_T_Min_CELL		= 0;

	pSingleRunInfo->emIsoResult				= STAT_AUTO_DETECTION;
	//pSingleRunInfo->fBAT_Ucell_Max 			=  0;
	//pSingleRunInfo->fBAT_Ucell_Min 			=  0;

	//pSingleRunInfo->fBAT_Ucell_Avg 			=  0;
	//pSingleRunInfo->nBAT_T_Max 				=  0;
	//pSingleRunInfo->nBAT_T_Min 				=  0;
	//pSingleRunInfo->nBAT_T_Avg 				=  0;

	pSingleRunInfo->byBattAddr 					= s_infy_BattRunData[n].byBattAddr;
 	pSingleRunInfo->bIsBatt_ChargeForbidden 	= TRUE;
	pSingleRunInfo->bIsBatt_DisChargeForbidden 	= TRUE;

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
void YFY_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//Ctrl DO5, Break Batt Contactor
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSTBatt_Rough->nCtrlTimes[n]	 = 0;
    	pSTBatt_Rough->nPowerONTimes[n]	 = 0;
		CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
	}
 	
	//---------Init cmd-------------
	TAG_YFY_BATT_PRO *p = &s_infy_BattPro[0];
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATUS,			0x14, 		0x32,			Parse_S2C_Status 		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_LIMIT1, 			0x14, 		0x33,			Parse_S2C_Limit1		);	
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_LIMIT2, 			0x14, 		0x34,			Parse_S2C_Limit2		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_ALARM, 			0x14, 		0x35,			Parse_S2C_Alarm			);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_FAULT,	 		0x14, 		0x36,			Parse_S2C_Fault			);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_GPIO, 	  		0x14, 		0x38,			Parse_S2C_GPIO			);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA1,  		0x14, 		0x3C,			Parse_S2C_SumData1		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA2,  		0x14, 		0x3D,			Parse_S2C_SumData2		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA3,  		0x14, 		0x3E,			Parse_S2C_SumData3		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA4,  		0x14, 		0x3F,			Parse_S2C_SumData4		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA5,  		0x14, 		0x40,			Parse_S2C_SumData5		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA7,  		0x14, 		0x42,			Parse_S2C_SumData7		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_SUMDATA8,  		0x14, 		0x43,			Parse_S2C_SumData8		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATISTICS1, 	0x14, 		0x46,			Parse_S2C_Statistics1	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATISTICS2, 	0x14, 		0x47,			Parse_S2C_Statistics2	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATISTICS3, 	0x14, 		0x48,			Parse_S2C_Statistics3	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATISTICS4, 	0x14, 		0x49,			Parse_S2C_Statistics4	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATISTICS5, 	0x14, 		0x4A,			Parse_S2C_Statistics5	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_STATISTICS6, 	0x14, 		0x4B,			Parse_S2C_Statistics6	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_DIAGNOSIS, 		0x14, 		0x4C,			Parse_S2C_Diagnosis		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CELLDATA1, 		0x16, 		0x00,			Parse_S2C_CellData1		);  //nSubIndex uncertain
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CELLDATA8, 		0x16, 		0x00,			Parse_S2C_CellData8		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CELLDATA2, 		0x16, 		0x00,			Parse_S2C_CellData2		);  //nSubIndex uncertain
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CELLDATA3, 		0x16, 		0x00,			Parse_S2C_CellData3		);  //nSubIndex uncertain
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_CELL_BARCODE_QUERY, 	0x0C, 		0xE8, 		Parse_S2C_CellBarcode	);
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_PACK_BARCODE_QUERY, 	0x0C, 		0xE7, 		Parse_S2C_PackBarcode	);
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_BSU_BARCODE_QUERY, 	0x0C, 		0xE6, 		Parse_S2C_BSUBarcode	);
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_Permission_BARCODE, 	0x06, 		0xE9, 		Parse_S2C_Barcode_Permission	);
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_CELL_BARCODE_WRITE, 	0x06, 		0xE8, 		Parse_S2C_CellBarcode	);
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_PACK_BARCODE_WRITE, 	0x06, 		0xE7, 		Parse_S2C_PackBarcode	);
	INIT_YFYBATT_CMD(p, 	_CAN_R_CMD_S2C_BSU_BARCODE_WRITE, 	0x06, 		0xE6, 		Parse_S2C_BSUBarcode	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CELLAFEWORKMODE, 0x16, 		0x00,			Parse_S2C_CellAFEWorkMode);  //nSubIndex uncertain
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CELLDATA6, 		0x10, 		0x62,			Parse_S2C_CellData6		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_DEVINFO, 		0x03, 		0x01,			Parse_S2C_DevInfo		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_CFGINFO, 		0x10, 		0x28,			Parse_S2C_CfgInfo		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_ISOLATRES, 		0x10, 		0x51,			Parse_S2C_IsolatRes		);
 	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_AUTH, 			0x10, 		0x50,			Parse_S2C_AuthRes		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_RECORDNUM, 		0x0C, 		0x1E,			Parse_S2C_RecNum		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_FAULTREC, 		0x0C, 		0x1F,			Parse_S2C_FaultRec		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_LOGREC, 			0x0C, 		0x20,			Parse_S2C_LogRec		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_WAVEREC, 		0x0C, 		0x21,			Parse_S2C_WaveRec		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_DODREC, 			0x0C, 		0x22,			Parse_S2C_DODRec		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_INNERRES_1, 		0x10, 		0x60,			Parse_S2C_InnerResis_1	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_INNERRES_2, 		0x10, 		0x61,			Parse_S2C_InnerResis_2	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_ZR_INNERRES, 	0x10, 		0xe2,			Parse_S2C_Zr_InnerResis	);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_ZI_INNERRES, 	0x10, 		0xe4,			Parse_S2C_Zi_InnerResis	);

	//INIT_YFYBATT_CMD(p,		_CAN_R_CMD_C2S_UPGRADESTATU,  	0x1C,		0xFA02,			Parse_C2S_UpgradeStatu ); //0x1CFA02,0xFF-Broadcast cmd,BMS Addr:0x32~0x37.
	INIT_YFYBATT_CMD(p,     _CAN_R_CMD_C2S_UPGRADESTATU,  		0x1C,		0xF0,		Parse_C2S_UpgradeStatu );
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_AIRINFO1, 			0x10, 		0xce,		Parse_S2C_AIRINFO1		); //add by lpl 240805
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_AIRINFO2, 			0x10, 		0xcf,		Parse_S2C_AIRINFO2		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_S2C_AIRINFO3, 			0x10, 		0xd0,		Parse_S2C_AIRINFO3		);

	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_PACKINFO1, 	0x10, 		0xd1,		Parse_S2C_PACKINFO1		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_PACKINFO2, 	0x10, 		0xd2,		Parse_S2C_PACKINFO2		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_PACKINFO3, 	0x10, 		0xd3,		Parse_S2C_PACKINFO3		);

	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_TFAIRPARAINFO, 	0x10, 		0xd4,		Parse_S2C_TFAIRPARAINFO		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_TFAIRSTAINFO, 	    0x10, 		0xd5,		Parse_S2C_TFAIRSTAINFO		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_S2C_TFAIRERRINFO, 	0x10, 		0xd6,		Parse_S2C_TFAIRERRINFO		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_LEAKAGEDATA, 	0x14, 		0xe1,		Parse_S2C_LEAKAGEDATA		);//xsy
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_LONGALARM, 	0x10, 		0x35,		Parse_S2C_LONGALARM		);//edit by Kuangyue 20241202
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_IMEU3AIRCONTROL, 	0x10, 		0xca,			Parse_S2C_IMEU3AIRCONTROLINFO		);//edit by kuangyue 20250811
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_IMEU3AIR, 		0x10, 		0xcb,			Parse_S2C_IMEU3AIRPARAINFO		);//edit by kuangyue 20250217
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_XINMIAOAIR1, 	0x10, 		0xd7,			Parse_S2C_XINMIAOAIRPARAINFO1		);//edit by kuangyue 20250217
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_XINMIAOAIR2, 	0x14, 		0xd8,			Parse_S2C_XINMIAOAIRPARAINFO2		);
	INIT_YFYBATT_CMD(p,		_CAN_R_CMD_Parse_S2C_AFEINFO, 	0x10, 		0xdf,				Parse_S2C_AfeEnumlInfo		);// edit by kuangyue 20250714

 	//INIT_NDBATT_CMD(p,	_CAN_S_CMD_C2B_STATU,		Send_C2BStatus);
	// LJH240715 aircond
	INIT_YFYBATT_CMD(p,     _CAN_R_CMD_S2C_AIRCOND_RD, 	    0x0C,		0x68,		    Parse_S2C_AirCondRd		);
	INIT_YFYBATT_CMD(p,     _CAN_R_CMD_S2C_AIRCOND_SET,	    0x06,		0x6D,		    Parse_S2C_AirCondSet 	);
	pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt 			= 0;
 	pSTBatt_Rough->stUpgradeParam.emUpgradeStatus 				= UPGRADE_IDLE;
	pSTBatt_Rough->stUpgradeParam.stUpdownFileInfo.pbyReadBuf 	= NULL;
	
    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
	    YFY_Init_RoughData(pSTBatt_Rough, n);

	    s_infy_BattRunData[n].byPCS_Cmd				 	= STBATT_YFY_POWERON_CMD;
	    s_infy_BattRunData[n].t_LastSendC2S_CMD_HV	 	= 0.0;
 	    s_infy_BattRunData[n].t_LastSendC2S_RTC		 	= 0.0;
		s_infy_BattRunData[n].t_LastSendC2S_RecNum   	= 0.0;
		s_infy_BattRunData[n].t_LastSendC2M_Res		 	= 0.0;
		s_infy_BattRunData[n].t_LastSendC2S_AfeSrTest	= 0.0;
	    s_infy_BattRunData[n].bFoundReply			 	= FALSE;
	    s_infy_BattRunData[n].nInterrupt_Times		 	= 0;
	    s_infy_BattRunData[n].bIsCommFail			 	= FALSE;
        s_infy_BattRunData[n].bBattInfo_Regisited	 	= FALSE; //if the battGrp info NOT registered
	
		memset(&s_infy_BattRunData[n].unioLigHT, 0, sizeof(IFY_BATT_LIGHT_STAT));
		// LJH240715 aircond 启动时查询温度值
		s_infy_BattRunData[n].bNeedGetTemp4Aircond = TRUE;
		s_infy_BattRunData[n].t_GetTemp4Aircond    = GetCurrentTime();
	}
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
static void Compare_OnlineNum2CfgNum(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    static  int 	s_LastNumbStatu = 0;
	int     		nCurrNumbStatu;
	VAR_VALUE 		varV;

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
static void Pro_YFY_Batt_CommStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
 	int n = 0 ;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
 		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
        {
			continue;
        }

		if(s_infy_BattRunData[n].bFoundReply == FALSE)
		{
			if(s_infy_BattRunData[n].nInterrupt_Times <= BATT_COMM_FAIL_TIMES)
			{
				s_infy_BattRunData[n].nInterrupt_Times++;
			}
		}
		else
		{
			s_infy_BattRunData[n].nInterrupt_Times = 0;
		}
			
		//TRACEX("*** ->s_infy_BattRunData[%d].bIsCommFail = %d ->s_infy_BattRunData[n].nInterrupt_Times=%d bFoundReply=%d\n",\
		// 	n, s_infy_BattRunData[n].bIsCommFail,s_infy_BattRunData[n].nInterrupt_Times,s_infy_BattRunData[n].bFoundReply);

		s_infy_BattRunData[n].bIsCommFail = (s_infy_BattRunData[n].nInterrupt_Times > BATT_COMM_FAIL_TIMES) ? TRUE : FALSE;

		if(s_infy_BattRunData[n].bIsCommFail)
		{
			s_infy_BattRunData[n].bBattInfo_Regisited = FALSE;
			//TRACE("pSTBatt_Rough->bCtrlOutRelayClose[%d]=%d\n", n, pSTBatt_Rough->bCtrlOutRelayClose[n]);
			if(s_infy_BattRunData[n].fOutputCurrent < 1.0)
			{
				if(pSTBatt_Rough->bCtrlOutRelayClose[n] == STAT_RELAY_ON)
				{
 					CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
					//LogOut(MAIN_MODULE_SAMPLER, "ND Batt Sampler", LOG_TYPE_ERROR, 
					//"ND Batt V3.3 Grp=[%d] CommFail,Volt=[%.1f] Curr[%.1f]",n,s_infy_BattRunData[n].fPackTotalVolt, s_infy_BattRunData[n].fPackTotalCurr);
				}
			}	
		}	
	}

	Compare_OnlineNum2CfgNum(pSTBatt_Rough);

	return ;
}

static void IMEU_CommStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	/*
	if(IMEU_TYPE_IMEU2 == pSTBatt_Rough->emIMEUType[0])
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			if(n >= MAX_IMEU_NUM)
			{
				continue;
			}

			if(pSTBatt_Rough->IMEU2CANRoughData[n].bFoundReply == FALSE)
			{
				if(pSTBatt_Rough->IMEU2CANRoughData[n].nInterrupt_Times <= IMEU_COMM_FAIL_TIMES)
				{
					pSTBatt_Rough->IMEU2CANRoughData[n].nInterrupt_Times++;
				}
			}
			else if(pSTBatt_Rough->IMEU2CANRoughData[n].bFoundReply == TRUE)
			{
				pSTBatt_Rough->IMEU2CANRoughData[n].nInterrupt_Times = 0;
			}

			pSTBatt_Rough->IMEU2CANRoughData[n].bIsCommFail = (pSTBatt_Rough->IMEU2CANRoughData[n].nInterrupt_Times > IMEU_COMM_FAIL_TIMES) ? TRUE : FALSE;

		}
	}
	else  
	*/
	if(IMEU_TYPE_IMEU3 == pSTBatt_Rough->emIMEUType[0])
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			if(n >= MAX_IMEU_NUM)
			{
				continue;
			}

			if(pSTBatt_Rough->IMEU3CANRoughData[n].bFoundReply == FALSE)   //电池仓IMEU3 通信状态
			{
				if(pSTBatt_Rough->IMEU3CANRoughData[n].nInterrupt_Times <= IMEU_COMM_FAIL_TIMES)
				{
					pSTBatt_Rough->IMEU3CANRoughData[n].nInterrupt_Times++;
				}
			}
			else if(pSTBatt_Rough->IMEU3CANRoughData[n].bFoundReply == TRUE)
			{
				pSTBatt_Rough->IMEU3CANRoughData[n].nInterrupt_Times = 0;
			}

			if(pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].bFoundReply == FALSE)  //判断电源仓和其他的EU3通信状态
			{
				if(pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times <= IMEU_COMM_FAIL_TIMES)
				{
					pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times++;
				}
			}
			else if(pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].bFoundReply == TRUE)
			{
				pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times = 0;
			}

			pSTBatt_Rough->IMEU3CANRoughData[n].bIsCommFail = (pSTBatt_Rough->IMEU3CANRoughData[n].nInterrupt_Times > IMEU_COMM_FAIL_TIMES) ? TRUE : FALSE; //电池仓
			pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].bIsCommFail = (pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].nInterrupt_Times > IMEU_COMM_FAIL_TIMES) ? TRUE : FALSE;  //电源仓
		}
	}

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
static void YFY_Send_C2S_Status(STBATT_SAMPLER_ROUGH  *pSTBatt_Rough,  int nBattIdx, YFY_C2S_CMD emCmdNum)
{
	static BYTE byRawData[CAN_FRAME_LEN] = {0};
	ZERO_POBJS(byRawData, CAN_FRAME_LEN);
	BYTE* pbySendBuf = byRawData;
 
	switch (emCmdNum)
	{
		case CAN_S_CMD_C2S_CMD_HV:
			YFY_PackC2S_CMD_HV_Farme(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2S_RTC_TIME:
			YFY_PackC2S_RTC_Time_Farme(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_RD_CMD_C2S_REC_NUM:
			YFY_PackC2S_RD_RecNum(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_RD_CMD_C2S_FAULT_REC:
			YFY_PackC2S_RD_FaultRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_CFM_CMD_C2S_FAULT_REC:
			YFY_PackC2S_Confirm_FaultRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_RD_CMD_C2S_LOG_REC:
			YFY_PackC2S_RD_LogRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_CFM_CMD_C2S_LOG_REC:
			YFY_PackC2S_Confirm_LogRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_RD_CMD_C2S_WAVE_REC:
			YFY_PackC2S_RD_WaveRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_CFM_CMD_C2S_WAVE_REC:
			YFY_PackC2S_Confirm_WaveRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_RD_CMD_C2S_DOD_REC:
			YFY_PackC2S_RD_DODRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_CFM_CMD_C2S_DOD_REC:
			YFY_PackC2S_Confirm_DODRec(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_S_CMD_C2S_REBOOT:
			YFY_PackC2S_Reboot_BMS(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2S_ISOLATION_CTRL:
			YFY_PackC2S_ISO_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_S_CMD_C2S_PERMISS_LEV_CTRL:
 			YFY_PackC2S_PermissionLev_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
 			break;
 
 		case CAN_S_CMD_C2S_CLEAR_FAULT:
 			YFY_PackC2S_ClearFault_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
 			break;

		case CAN_S_CMD_C2S_WORK_MODE:
 			YFY_PackC2S_WorkMode_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2S_EMSSTATUS:
			YFY_PackC2S_EmsStatu_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		// LJH240715 aircond
		case CAN_RD_CMD_C2S_AIRCOND:
			YFY_PackC2S_RD_AirCond(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		case CAN_S_CMD_C2S_AIRCOND_BATT_COOLON:  case CAN_S_CMD_C2S_AIRCOND_BATT_COOLOFF:  case CAN_S_CMD_C2S_AIRCOND_BATT_HEATON:  case CAN_S_CMD_C2S_AIRCOND_BATT_HEATOFF:
		case CAN_S_CMD_C2S_AIRCOND_AMBIT_COOLON: case CAN_S_CMD_C2S_AIRCOND_AMBIT_COOLOFF: case CAN_S_CMD_C2S_AIRCOND_AMBIT_HEATON: case CAN_S_CMD_C2S_AIRCOND_AMBIT_HEATOFF:
			YFY_PackC2S_SetAirCond(pSTBatt_Rough, nBattIdx, pbySendBuf, emCmdNum);
			break;
		
		case CAN_S_CMD_C2S_RINTPOINT:
			YFY_PackC2S_Rint_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

			case CAN_S_CMD_C2S_AFESRTEST:
			YFY_PackC2S_AFESrTest_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		case CAN_S_CMD_C2S_EU3AirNoise:
 			YFY_PackC2S_EU3AirNoise_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
 			break;
		case CAN_S_CMD_C2S_CalibratedSOC:
			YFY_PackC2S_CalibratedSOC_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2S_KEEP_POWER_MODE:
 			YFY_PackC2S_KeepPowerMode_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
 			break;

		case CAN_S_CMD_C2S_AFE_RESET:
			YFY_PackC2S_AfeReset_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2S_BSU_RESET:
			YFY_PackC2S_BsuReset_CtrlCmd(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		default:
			break;
	}
 

 	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		TRACE("Write To CAN Port Failed !!!\n");
	}
	else
	{
		// RecBmsMu2Log(pSTBatt_Rough, pbySendBuf, nWriteSize);
		// RecBmsMu2AlarmLog(pSTBatt_Rough, pbySendBuf, nWriteSize);

		if((pSTBatt_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nBattIdx, pbySendBuf, 1);
		}
	}
	
	
	//printf("YFY BMS Send:\n");
	//int n = 0;
	//for(n = 0; n < nWriteSize; n++)
	//{
	//	printf("%02X|", pbySendBuf[n]);
	//}
	//printf("\n");
	
	return;
}

static void YFY_Send_C2M_Status(STBATT_SAMPLER_ROUGH  *pSTBatt_Rough,  int nBattIdx, YFY_C2S_CMD emCmdNum, int TestState)
{
	static BYTE byRawData[CAN_FRAME_LEN] = {0};
	BYTE* pbySendBuf = byRawData;
 
	switch (emCmdNum)
	{
		case CAN_S_CMD_C2M_SELFCHECK_RES1:
			YFY_C2M_SelfCheck_Res1(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_SELFCHECK_RES2:
			YFY_C2M_SelfCheck_Res2(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_SELFCHECK_RES3:
			YFY_C2M_SelfCheck_Res3(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_SELFCHECK_RES4:
			YFY_C2M_SelfCheck_Res4(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_CHARGECHECK_RES1:
			YFY_C2M_ChargeCheck_Res1(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_CHARGECHECK_RES2:
			YFY_C2M_ChargeCheck_Res2(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_CHARGECHECK_RES3:
			YFY_C2M_ChargeCheck_Res3(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_DISCHARGECHECK_RES1:
			YFY_C2M_DischargeCheck_Res1(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_S_CMD_C2M_DISCHARGECHECK_RES2:
			YFY_C2M_DischargeCheck_Res2(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_DISCHARGECHECK_RES3:
			YFY_C2M_DischargeCheck_Res3(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;

		case CAN_S_CMD_C2M_TOTAL_RES:
			YFY_C2M_Total_Res(pSTBatt_Rough, nBattIdx, pbySendBuf);
			break;
		
		case CAN_S_CMD_C2M_HEARTBEAT:
			YFY_C2M_HeartBeat(pSTBatt_Rough, nBattIdx, pbySendBuf, TestState);
			break;

		default:
			break;
	}
 

 	ssize_t nWriteSize = pSTBatt_Rough->pComm->Write(pSTBatt_Rough->hCanDevHanle, (void *)pbySendBuf, CAN_FRAME_LEN);
	if(nWriteSize < 0)
	{
		printf("Write To CAN Port Failed !!!\n");
	}
	else
	{
		if((pSTBatt_Rough->stuCanSave.stuStatMach[nBattIdx].emSaveState == STAT_ON_PENDING) &&
			(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBattIdx] == TP_TRIG_MANUAL))
		{
			Pending_WriteMsg2USB(pSTBatt_Rough, nBattIdx, pbySendBuf, 1);
		}
	}
	
	
	//printf("YFY BMS Send:\n");
	//int n = 0;
	//for(n = 0; n < nWriteSize; n++)
	//{
	//	printf("%02X|", pbySendBuf[n]);
	//}
	//printf("\n");
	
	return;
}

static BOOL Tigger_IfyBmsFrameWrite2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, A_CAN_YFY_BATT_FRAME *pFrmReply)
{
	int nBMSGrpMapIdx = pFrmReply->nBMSGrpMapIdx;
	if((nBMSGrpMapIdx >= MAX_SINGLE_STBATT_NUM) || (nBMSGrpMapIdx < 0))
	{
		return FALSE;
	}

	if(pSTBatt_Rough->stuCanSave.stuStatMach[nBMSGrpMapIdx].emSaveState != STAT_ON_PENDING)
	{
		return FALSE;
	}

	switch (pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSGrpMapIdx])
	{
		case TP_TRIG_MANUAL:
			Pending_WriteMsg2USB(pSTBatt_Rough, nBMSGrpMapIdx, pFrmReply->byRawData, 1);
		break;

		case TP_TRIG_FAULT:
			if(pFrmReply->emMsgType == _CAN_R_CMD_S2C_FAULTREC)
			{
				Pending_WriteMsg2USB(pSTBatt_Rough, nBMSGrpMapIdx, pFrmReply->byRawData, 1);	
				YFY_Send_C2S_Status(pSTBatt_Rough, nBMSGrpMapIdx, CAN_CFM_CMD_C2S_FAULT_REC); 
 			}
		break;

		case TP_TRIG_LOG:
			if(pFrmReply->emMsgType == _CAN_R_CMD_S2C_LOGREC)
			{
				Pending_WriteMsg2USB(pSTBatt_Rough, nBMSGrpMapIdx, pFrmReply->byRawData, 1);
				YFY_Send_C2S_Status(pSTBatt_Rough, nBMSGrpMapIdx, CAN_CFM_CMD_C2S_LOG_REC); 
				
 			}
		break;

		case TP_TRIG_WAVE:
			if(pFrmReply->emMsgType == _CAN_R_CMD_S2C_WAVEREC)
			{
				Pending_WriteMsg2USB(pSTBatt_Rough, nBMSGrpMapIdx, pFrmReply->byRawData, 1);
				YFY_Send_C2S_Status(pSTBatt_Rough, nBMSGrpMapIdx, CAN_CFM_CMD_C2S_WAVE_REC); 
				
 			}
		break;

		case TP_TRIG_DOD:
			if(pFrmReply->emMsgType == _CAN_R_CMD_S2C_DODREC)
			{
				Pending_WriteMsg2USB(pSTBatt_Rough, nBMSGrpMapIdx, pFrmReply->byRawData, 1);
				YFY_Send_C2S_Status(pSTBatt_Rough, nBMSGrpMapIdx, CAN_CFM_CMD_C2S_DOD_REC); 
 			}
		break;
		default:
			break;
	}

	return TRUE;
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
void YFY_ReadAndParseCanData(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int nFrmCount;
	int n=0, nReadBytes=0, nReadLen=0;
	A_CAN_YFY_BATT_FRAME frmReply;
	ZERO_POBJS(&frmReply, 1);
	frmReply.pSTBatt_Rough = pSTBatt_Rough;

	IMEU2_CAN_FRAME IMEU2FrameReply;
	ZERO_POBJS(&IMEU2FrameReply, 1);

	IMEU3_CAN_FRAME IMEU3FrameReply;
	ZERO_POBJS(&IMEU3FrameReply, 1);

	BYTE *byRecvBuff = pSTBatt_Rough->abyRcvBuf;
 	memset(byRecvBuff, 0, MAX_READ_BUFF_SIZE*CAN_FRAME_LEN);

	nReadLen = pSTBatt_Rough->pComm->Read(pSTBatt_Rough->hCanDevHanle, (void*)byRecvBuff, (ssize_t)(CAN_FRAME_LEN * MAX_READ_BUFF_SIZE));

	//printf("YFY BMS Recv: \n");
	//for(n=0; n<nReadLen; n++)
	//{
	//	printf("%02X ", byRecvBuff[n]);
	//}
	//printf("\n");
	
	//TRACE("Not Found Any Data!!!\n");
	for(n = 0; n < MAX_SINGLE_STBATT_NUM ; n++)
	{
		s_infy_BattRunData[n].bFoundReply = FALSE;
	}

/*
	if(IMEU_TYPE_IMEU2 == pSTBatt_Rough->emIMEUType[0])
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			pSTBatt_Rough->IMEU2CANRoughData[n].bFoundReply = FALSE;
		}
	}
	else 
*/
	if(IMEU_TYPE_IMEU3 == pSTBatt_Rough->emIMEUType[0])
	{
		for(n = 0; n < MAX_IMEU_NUM; n++)
		{
			pSTBatt_Rough->IMEU3CANRoughData[n].bFoundReply = FALSE;

			pSTBatt_Rough->PowerEU3_OtherEU3_Data[n].bFoundReply = FALSE;
		}
	}

	if(nReadLen >= CAN_FRAME_LEN)
	{
		nFrmCount = nReadLen / CAN_FRAME_LEN;
		for(n = 0; n < nFrmCount; n++)
		{
			memcpy(frmReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);

			BOOL bAlreadyProcessed = FALSE;
			if(YFY_JudgeAndParseMsgID(&frmReply))
			{
				bAlreadyProcessed = TRUE;
				s_infy_BattRunData[frmReply.nBMSGrpMapIdx].bFoundReply = TRUE;
				s_infy_BattPro[frmReply.emMsgType].pfnCmdHandler(&frmReply);

				// Bms Can Record
				RecBmsMu2Log(pSTBatt_Rough, frmReply.byRawData, CAN_FRAME_LEN);
				RecBmsMu2AlarmLog(pSTBatt_Rough, frmReply.byRawData, CAN_FRAME_LEN);

				//John 20220429 Added For Trigger Msg Save
				Tigger_IfyBmsFrameWrite2USB(pSTBatt_Rough, &frmReply);
				//if((pSTBatt_Rough->stuCanSave.stuStatMach[frmReply.nBMSGrpMapIdx].emSaveState == STAT_ON_PENDING) &&
				//	(pSTBatt_Rough->stuCanSave.emTrigMsgTp[frmReply.nBMSGrpMapIdx] == TP_TRIG_MANUAL))
				//{
				//	Pending_WriteMsg2USB(pSTBatt_Rough, frmReply.nBMSGrpMapIdx, frmReply.byRawData, 1);
				//}
			}

			if (bAlreadyProcessed){
				continue;
			}
			
			/*
			if (IMEU_TYPE_IMEU2 == pSTBatt_Rough->emIMEUType[0])
			{
				memcpy(IMEU2FrameReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
				if (judgeAndParseIMEU2MsgID(&IMEU2FrameReply))
				{
					if (IMEU2FrameReply.bySourceIdx >= 0 && IMEU2FrameReply.bySourceIdx < MAX_IMEU_NUM)
					{
						pSTBatt_Rough->IMEU2CANRoughData[IMEU2FrameReply.bySourceIdx].bFoundReply = TRUE;

						processIMEU2CANData(&pSTBatt_Rough->IMEU2CANRoughData[IMEU2FrameReply.bySourceIdx], &IMEU2FrameReply);
					}
				}
			}
			else 
			*/
			if (IMEU_TYPE_IMEU3 == pSTBatt_Rough->emIMEUType[0])
			{
				memcpy(IMEU3FrameReply.byRawData, byRecvBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
				if (judgeAndParseIMEU3MsgID(&IMEU3FrameReply))   //电池仓IMEU3数据
				{
					if (IMEU3FrameReply.bySourceIdx >= 0 && IMEU3FrameReply.bySourceIdx < MAX_IMEU_NUM)
					{
						pSTBatt_Rough->IMEU3CANRoughData[IMEU3FrameReply.bySourceIdx].bFoundReply = TRUE;					
						
						processIMEU3CANData(&pSTBatt_Rough->IMEU3CANRoughData[IMEU3FrameReply.bySourceIdx], &IMEU3FrameReply);
					}
				}

				
				if(judgeAndParseOtherIMEU3MsgID(&IMEU3FrameReply))   //解析电源仓和其他的IMEU3数据
				{
					if (IMEU3FrameReply.bySourceIdx >= 0 && IMEU3FrameReply.bySourceIdx < MAX_IMEU_NUM)
					{
						pSTBatt_Rough->PowerEU3_OtherEU3_Data[IMEU3FrameReply.bySourceIdx].bFoundReply = TRUE;

						if(IMEU3FrameReply.emMsgType == _CAN_R_CMD_C2S_IMEU3UPGRADE)  //单独处理电源、STS EU3 OTA升级CAN报文，
						{
							memcpy(frmReply.byData, frmReply.byRawData + 5, CAN_DATA_AREA_LEN);
							frmReply.nBMSGrpMapIdx = IMEU3FrameReply.bySourceIdx;
							s_infy_BattPro[_CAN_R_CMD_C2S_UPGRADESTATU].pfnCmdHandler(&frmReply);		
						}
						else
						{
							processIMEU3CANData(&pSTBatt_Rough->PowerEU3_OtherEU3_Data[IMEU3FrameReply.bySourceIdx], &IMEU3FrameReply);
						}
						
					}
				}

			}
		}
	}
	
	if(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt)
	{
		//pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt = 0;
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			if(s_infy_BattRunData[n].bBattInfo_Regisited == TRUE)
			{
				s_infy_BattRunData[n].byBattAddr = szLocalBMSGrpMapIdx[n];
				//pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt++;
				TRACE("n=%d nTotalOnlineBatt=%d bBattInfo_Regisited=%d\n", \
					n, pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt, s_infy_BattRunData[n].bBattInfo_Regisited);
			}
		}
	}

	Pro_YFY_Batt_CommStatu(pSTBatt_Rough);

	IMEU_CommStatus(pSTBatt_Rough);
	//proIMEU2CommStatu(pSTBatt_Rough->);
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
static void YFY_RefreshBattCabinetAlarm(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	VAR_VALUE varV;
	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 456, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.IMEU_CommFail = varV.emValue;
		//TRACE("IMEU CommFail:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 451, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.Fire_Alarm = varV.emValue;	
		//TRACEX("Flood Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 452, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.Smoke_Alarm = varV.emValue;	
		//TRACEX("DoorOpen Alm:varV.emValue=%d\n",varV.emValue);

		if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
		{//WLD01206 备电柜只有一个EU3，需要绑定到两组电池
			pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
				601, 453, &varV, 4);
			s_infy_BattRunData[n].mapSysFault.Fault.DoorOpen_Alarm = varV.emValue;

			pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
				611, 3096, &varV, 4);
			s_infy_BattRunData[n].mapSysFault.Fault.DoorOpen_Alarm = s_infy_BattRunData[n].mapSysFault.Fault.DoorOpen_Alarm | varV.emValue;
		}
		else
		{
			pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
				601 + n, 453, &varV, 4);
			s_infy_BattRunData[n].mapSysFault.Fault.DoorOpen_Alarm = varV.emValue;		
		}

		//TRACEX("Smoke Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 454, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.Flammable_Gas = varV.emValue;	
		//TRACEX("Fire Alm:varV.emValue=%d\n",varV.emValue);

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 462, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.CabSafe_AbnormalState = varV.emValue;	

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 457, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.AirCond_CommFail = varV.emValue;
		
		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 450, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.AirCond_Alarm = varV.emValue;

		if(pSTBatt_Rough->blobSingleBattInfo[n].fMax_CellTemp < 45)//Off-Grid AirCond is PowerOff. 
		{
			s_infy_BattRunData[n].mapSysFault.Fault.AirCond_CommFail 	= 0;
			s_infy_BattRunData[n].mapSysFault.Fault.AirCond_Alarm 		= 0;
		}

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			601 + n, 461, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.Flood_Alarm = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			200, 304 + n, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.BattGrp_ForbidRun = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			200, 308 + n, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.BattGrp_InputContactor = varV.emValue;

		pSTBatt_Rough->pDPR->Get(pSTBatt_Rough->hSampler, GETTYPE_ALMSTATE,
			200, 312 + n, &varV, 4);
		s_infy_BattRunData[n].mapSysFault.Fault.BattGrp_IsolationSwitch = varV.emValue;

	}

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
static void  Map_RawData2IfyBmsBlob(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int n)
{
	A_BLOB_IFY_BMS_DATAINFO			*pBlobIfyBMS = &pSTBatt_Rough->blobIfyBMS[n];
	A_BLOB_SINGLE_STBATT_RUNINFO    *pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];

	UINT 							nBattPackCellNum = 0;

    //------------------------Map Public Single Battery Blob -------------------------//
 	switch(s_infy_BattRunData[n].emBattSysState) 
	{
		case YFYBATT_SYSSTAT_HV_DOWN: 			pSingleRunInfo->emPowerONStatu = 0;		break;
		case YFYBATT_SYSSTAT_HV_UP: 			pSingleRunInfo->emPowerONStatu = 1;		break;
		case YFYBATT_SYSSTAT_CHARGE_FULL:		pSingleRunInfo->emPowerONStatu = 1;		break;
		case YFYBATT_SYSSTAT_DISCHARGE_FULL:	pSingleRunInfo->emPowerONStatu = 1;		break;
		case YFYBATT_SYSSTAT_CHG_DISC_FORBID:	pSingleRunInfo->emPowerONStatu = 1;		break;
		case YFYBATT_SYSSTAT_HV_DOWN_PROTECT:	pSingleRunInfo->emPowerONStatu = 3;		break;
		default:								pSingleRunInfo->emPowerONStatu = 0;		break;
	}
	
	int nCellNum = 0; //电池组电芯数量
	if(SYSTYPE_ESS_AIR == pSTBatt_Rough->lcCfg.emSystemType)
	{
		nCellNum = SYSTYPE_ESS_AIR_PACK_NUM*SYSTYPE_ESS_AIR_SINGLE_CELL_NUM;
	}
	else if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
	{
		nCellNum = LIQUID_COOLING_BACKUP_PACK_NUM*SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM;
	}
	else
	{
		nCellNum = SYSTYPE_ESS_LIQUIDED_PACK_NUM*SYSTYPE_ESS_LIQUIDED_SINGLE_CELL_NUM;
	}
	float fCellVoltSum = nCellNum * pSingleRunInfo->fAvg_CellVolt;
	int maxVolt = MAX(MAX(s_infy_BattRunData[n].fOutsideVolt, s_infy_BattRunData[n].fPackTotalVolt), fCellVoltSum); 
	int minVolt = MIN(MIN(s_infy_BattRunData[n].fOutsideVolt, s_infy_BattRunData[n].fPackTotalVolt), fCellVoltSum);

	if((pSingleRunInfo->emPowerONStatu == 0) && (ABS(maxVolt - minVolt) < 20))
	{
		pSingleRunInfo->emPowerONStatu = 1;
	}

	pSingleRunInfo->fPackTotalVolt =                    s_infy_BattRunData[n].fPackTotalVolt;
	pSingleRunInfo->fPackTotalCurr =                    s_infy_BattRunData[n].fOutputCurrent;
	pSingleRunInfo->nSOC =                              (UINT)s_infy_BattRunData[n].fSOC;
	pSingleRunInfo->nSOH =                              (UINT)s_infy_BattRunData[n].fSOH;
	pSingleRunInfo->fAllow_LmtChargeVolt =              s_infy_BattRunData[n].fAllow_MaxChargeVolt;
	pSingleRunInfo->fAllow_LmtDisChargeVolt =           s_infy_BattRunData[n].fAllow_MaxDisChargeVolt;
	pSingleRunInfo->fAllow_MaxChargeCurr =              s_infy_BattRunData[n].fAllow_MaxChargeCurr;
	pSingleRunInfo->fAllow_MaxDisChargeCurr =           s_infy_BattRunData[n].fAllow_MaxDisChargeCurr;
	pSingleRunInfo->fAllow_MaxChargePow =              s_infy_BattRunData[n].fAllow_MaxChargePow;
	pSingleRunInfo->fAllow_MaxDisChargePow =           s_infy_BattRunData[n].fAllow_MaxDisChargePow;

	pSingleRunInfo->fMax_CellVolt =                     ((float)s_infy_BattRunData[n].usMaxCellVlot_Volt) *0.001;
	pSingleRunInfo->fMin_CellVolt =                     ((float)s_infy_BattRunData[n].usMinCellVlot_Volt) *0.001;

	if (TEMP_PRECISION_ONE == pSTBatt_Rough->eBattCellTempPrecison)
	{
		pSingleRunInfo->fMax_CellTemp =                     ((float)s_infy_BattRunData[n].sMaxCellTemp_Temp);
		pSingleRunInfo->fMin_CellTemp =                     ((float)s_infy_BattRunData[n].sMinCellTemp_Temp);
	}
	else
	{
		pSingleRunInfo->fMax_CellTemp =                     ((float)s_infy_BattRunData[n].sMaxCellTemp_Temp) *0.1;
		pSingleRunInfo->fMin_CellTemp =                     ((float)s_infy_BattRunData[n].sMinCellTemp_Temp) *0.1;
	}
	
	pSingleRunInfo->nLifeCycleTimes =                   s_infy_BattRunData[n].usCycleTimes;
	
	memcpy(pSingleRunInfo->szInfy_AlarmInfo, s_infy_BattRunData[n].szInfy_AlarmInfo, sizeof(pSingleRunInfo->szInfy_AlarmInfo));
	memcpy(pSingleRunInfo->szInfy_FaultInfo, s_infy_BattRunData[n].szInfy_FaultInfo, sizeof(pSingleRunInfo->szInfy_FaultInfo));
	memcpy(pSingleRunInfo->szInfy_GPIOInfo,  s_infy_BattRunData[n].szInfy_GPIOInfo,  sizeof(pSingleRunInfo->szInfy_GPIOInfo));

	memcpy(pSingleRunInfo->szAlarm, 	s_infy_BattRunData[n].unioBattAlarmCode.szByIfyAlarm, 	CAN_DATA_AREA_LEN);
	memcpy(pSingleRunInfo->szFault, 	s_infy_BattRunData[n].unioBattFaultCode.szByIfyFault, 	CAN_DATA_AREA_LEN);
	memcpy(pSingleRunInfo->szDIDOStatu, s_infy_BattRunData[n].unioBattDIDOStat.szByDIDOStat, 	CAN_DATA_AREA_LEN);


	pSingleRunInfo->fChrg_Energy = s_infy_BattRunData[n].fTotalEnergy - s_infy_BattRunData[n].fRemainEnergy;
	pSingleRunInfo->fDisChrging_Energy = s_infy_BattRunData[n].fRemainEnergy;
	pSingleRunInfo->fSingleChrgCapacity = s_infy_BattRunData[n].fSingleChgCapacity;
	pSingleRunInfo->fSingleDischrgCapacity = s_infy_BattRunData[n].fSingleDischgCapacity;
	pSingleRunInfo->AccuChrgCapacity  = s_infy_BattRunData[n].fAccuChgCapacity;
	pSingleRunInfo->fAccuDischrgCapacity = s_infy_BattRunData[n].fAccuDischgCapacity;
	pSingleRunInfo->fAccuChrgEnergy  = s_infy_BattRunData[n].fAccuChgEnergy;
	pSingleRunInfo->fAccuDischrgEnergy  = s_infy_BattRunData[n].fAccuDischgEnergy;

	pSingleRunInfo->fTotal_RateCapacity = s_infy_BattRunData[n].fTotalCapacity;

	pSingleRunInfo->fAdd_ChgCapacity = s_infy_BattRunData[n].fAccuChgCapacity; 
	pSingleRunInfo->fAdd_DischgCapacity = s_infy_BattRunData[n].fAccuDischgCapacity;
	pSingleRunInfo->fAdd_ChgEnergy = s_infy_BattRunData[n].fAccuChgEnergy;
	pSingleRunInfo->fAdd_DischgEnergy = s_infy_BattRunData[n].fAccuDischgEnergy;

	pSingleRunInfo->fInsulation_P_R = s_infy_BattRunData[n].fInsulation_P_R;
	pSingleRunInfo->fInsulation_N_R = s_infy_BattRunData[n].fInsulation_N_R;

	pSingleRunInfo->emIsoResult = s_infy_BattRunData[n].emIsoResult; 

	pSingleRunInfo->fOutsideVolt = s_infy_BattRunData[n].fOutsideVolt;
	pSingleRunInfo->fNegativeBusVolt = s_infy_BattRunData[n].fNegativeBusVolt;
	pSingleRunInfo->usBattWorkState = s_infy_BattRunData[n].usBattWorkState;
	pSingleRunInfo->uiPermissionLevel = s_infy_BattRunData[n].uiPermissionLevel;
	char *pEnd;
	pSingleRunInfo->fVersion = strtof(&s_infy_BattRunData[n].bySoftwareVersion[1], &pEnd);

    if(s_infy_BattRunData[n].usCellVoltTotalNum % 16 == 0)
    {	
		//Volt 
    	nBattPackCellNum = 16;
    	pSingleRunInfo->uiBAT_Ucell_Max_CSC = (s_infy_BattRunData[n].usMaxCellVlot_Cell + nBattPackCellNum - 1) / nBattPackCellNum;
    	pSingleRunInfo->uiBAT_Ucell_Min_CSC = (s_infy_BattRunData[n].usMinCellVlot_Cell + nBattPackCellNum - 1) / nBattPackCellNum;
		
		pSingleRunInfo->uiBAT_Ucell_Max_CELL = ((s_infy_BattRunData[n].usMaxCellVlot_Cell - 1) % nBattPackCellNum) + 1;
    	pSingleRunInfo->uiBAT_Ucell_Min_CELL = ((s_infy_BattRunData[n].usMinCellVlot_Cell - 1) % nBattPackCellNum) + 1;
	}
	else if(s_infy_BattRunData[n].usCellVoltTotalNum % 14 == 0)
	{
		nBattPackCellNum = 14;
		pSingleRunInfo->uiBAT_Ucell_Max_CSC = (s_infy_BattRunData[n].usMaxCellVlot_Cell + nBattPackCellNum - 1) / nBattPackCellNum;
		pSingleRunInfo->uiBAT_Ucell_Min_CSC = (s_infy_BattRunData[n].usMinCellVlot_Cell + nBattPackCellNum - 1) / nBattPackCellNum;
	
		pSingleRunInfo->uiBAT_Ucell_Max_CELL = ((s_infy_BattRunData[n].usMaxCellVlot_Cell - 1) % nBattPackCellNum) + 1;
    	pSingleRunInfo->uiBAT_Ucell_Min_CELL = ((s_infy_BattRunData[n].usMinCellVlot_Cell - 1) % nBattPackCellNum) + 1;
	}
	else
	{
		//Volt
		pSingleRunInfo->uiBAT_Ucell_Max_CSC = 0;
		pSingleRunInfo->uiBAT_Ucell_Min_CSC = 0;

		pSingleRunInfo->uiBAT_Ucell_Max_CELL = 0;
		pSingleRunInfo->uiBAT_Ucell_Min_CELL = 0;
	}

	pSingleRunInfo->nBAT_T_Max_CSC = (s_infy_BattRunData[n].usMaxCellTemp_Cell + 3) / 4;
    pSingleRunInfo->nBAT_T_Min_CSC = (s_infy_BattRunData[n].usMinCellTemp_Cell + 3) / 4;

	pSingleRunInfo->uiBAT_T_Max_CELL = ((s_infy_BattRunData[n].usMaxCellTemp_Cell + 3) % 4) + 1;
    pSingleRunInfo->uiBAT_T_Min_CELL = ((s_infy_BattRunData[n].usMinCellTemp_Cell + 3) % 4) + 1;

    //pSingleRunInfo->uiBAT_Ucell_Max_CELL = s_infy_BattRunData[n].usMaxCellVlot_Cell;
    //pSingleRunInfo->uiBAT_Ucell_Min_CELL = s_infy_BattRunData[n].usMinCellVlot_Cell;

	//pSingleRunInfo->fBAT_Ucell_Max = ((float)s_infy_BattRunData[n].usMaxCellVlot_Volt)*0.001;
    //pSingleRunInfo->fBAT_Ucell_Min = ((float)s_infy_BattRunData[n].usMinCellVlot_Volt)*0.001;

    pSingleRunInfo->fAvg_CellVolt = ((float)s_infy_BattRunData[n].usCellAverageVolt) * 0.001;
    //pSingleRunInfo->nBAT_T_Max = s_infy_BattRunData[n].sMaxCellTemp_Temp;
    //pSingleRunInfo->nBAT_T_Min = s_infy_BattRunData[n].sMinCellTemp_Temp;
	if (TEMP_PRECISION_ONE == pSTBatt_Rough->eBattCellTempPrecison)
	{
		 pSingleRunInfo->fAvg_CellTemp = (float)s_infy_BattRunData[n].sCellAverageTemp;
	}
	else
	{
		 pSingleRunInfo->fAvg_CellTemp = ((float)s_infy_BattRunData[n].sCellAverageTemp) * 0.1;
	}
   

    pSingleRunInfo->byBattAddr = s_infy_BattRunData[n].byBattAddr;

    pSingleRunInfo->bIsBatt_ChargeForbidden = FLOAT_EQU_ZERO(s_infy_BattRunData[n].fAllow_MaxChargeCurr) ? TRUE : FALSE;
    pSingleRunInfo->bIsBatt_DisChargeForbidden = FLOAT_EQU_ZERO(s_infy_BattRunData[n].fAllow_MaxDisChargeCurr) ? TRUE : FALSE;

    //-----------------------------------End------------------------------------------//

    //---------------------------------Map Infy BMS Blob DataInfo---------------------//
	//DIDO 
	memcpy(pBlobIfyBMS->unioBattDIDOStat.szByDIDOStat, s_infy_BattRunData[n].unioBattDIDOStat.szByDIDOStat, CAN_DATA_AREA_LEN);

	memcpy(pBlobIfyBMS->byCellBalanceStat, 	s_infy_BattRunData[n].byCellBalanceStat, 	A_CABINET_CELL_MAX_NUM);
	memcpy(pBlobIfyBMS->byCellSafeStat, 	s_infy_BattRunData[n].byCellSafeStat, 		A_CABINET_CELL_MAX_NUM);
	memcpy(pBlobIfyBMS->usCellVolt, 		s_infy_BattRunData[n].usCellVolt, 			A_CABINET_CELL_MAX_NUM*2);
	memcpy(pBlobIfyBMS->cCellTemp, 			s_infy_BattRunData[n].cCellTemp, 			A_CABINET_CELL_MAX_NUM*4);
	memcpy(pBlobIfyBMS->fCellRes, 			s_infy_BattRunData[n].fCellRes, 			A_CABINET_CELL_MAX_NUM*4);
	memcpy(pBlobIfyBMS->usCellZrRes, 			s_infy_BattRunData[n].usCellZrRes, 			A_CABINET_CELL_MAX_NUM*4);
	memcpy(pBlobIfyBMS->usCellZiRes, 			s_infy_BattRunData[n].usCellZiRes, 			A_CABINET_CELL_MAX_NUM*4);
	memcpy(pBlobIfyBMS->byCellAFEWorkMode, 	s_infy_BattRunData[n].byCellAFEWorkMode, 	A_CABINET_CELL_MAX_NUM);//XSY250417
	pBlobIfyBMS->bAfeWorkModeAbnormal = 	s_infy_BattRunData[n].bAfeWorkModeAbnormal;
	memcpy(pBlobIfyBMS->usNewCellAFESrStatus, 	s_infy_BattRunData[n].usNewCellAFESrStatus, 	A_CABINET_CELL_MAX_NUM*2);
	if(s_infy_BattRunData[n].bAfeWorkModeAbnormal)
	{
		if(memcmp(pBlobIfyBMS->usNewCellAFESrStatus, pBlobIfyBMS->usOldAFEAbnormalStatus, sizeof(pBlobIfyBMS->usOldAFEAbnormalStatus)) != 0)
		{
			memcpy(pBlobIfyBMS->usOldAFEAbnormalStatus, 	s_infy_BattRunData[n].usNewCellAFESrStatus, 	A_CABINET_CELL_MAX_NUM*2);
			// save 240 afe data
			FILE* fp;
			fp = fopen("/data/AfeSrStatus.csv", "a");
			if(!fp)
			{
				fclose(fp);
				LogOut(MAIN_MODULE_MAIN, "AFE DATA SAVE", LOG_TYPE_ERROR,
					"Can NOT fopen AfeSrStatus.csv, please check!");
			}
			else
			{
				char strLine[1024]={0};
				char szTmp[64]={0};
				char szTime[64]={0};
				TimeToString(_NOW_, TIME_CHN_FMT, szTime, sizeof(szTime));
				snprintf(szTmp, 64, "%s,", szTime);
				strcat(strLine, szTmp);
				int id;
				for(id=0; id<A_CABINET_CELL_MAX_NUM; ++id)
				{
					snprintf(szTmp, 64, "%d%s", 
						s_infy_BattRunData[n].usNewCellAFESrStatus[id], 
						(id < (A_CABINET_CELL_MAX_NUM-1)) ? "," : "\n");
					strcat(strLine, szTmp);			
				}
				fputs(strLine, fp);
				fflush(fp);		
				fclose(fp);
			}
		}
	}


	pBlobIfyBMS->emBattSysState = 					s_infy_BattRunData[n].emBattSysState ;
	pBlobIfyBMS->emBattChgState = 					s_infy_BattRunData[n].emBattChgState ;

	//--- 0x1433ffxx ---//
	pBlobIfyBMS->fAllow_MaxChargeCurr =              s_infy_BattRunData[n].fAllow_MaxChargeCurr;
	pBlobIfyBMS->fAllow_MaxDisChargeCurr =           s_infy_BattRunData[n].fAllow_MaxDisChargeCurr;
	pBlobIfyBMS->fAllow_MaxChargePow =              s_infy_BattRunData[n].fAllow_MaxChargePow;
	pBlobIfyBMS->fAllow_MaxDisChargePow =           s_infy_BattRunData[n].fAllow_MaxDisChargePow;

	//--- 0x1434ffxx  ---//
	pBlobIfyBMS->fAllow_LmtChargeVolt =              s_infy_BattRunData[n].fAllow_MaxChargeVolt;
	pBlobIfyBMS->fAllow_LmtDisChargeVolt =           s_infy_BattRunData[n].fAllow_MaxDisChargeVolt;

	//--- 0x1436ffxx  ---//
	pBlobIfyBMS->unioBattFault	=				 	s_infy_BattRunData[n].unioBattFaultCode;

	//--- 0x143cffxx  ---//
    pBlobIfyBMS->fPackTotalVolt =                    s_infy_BattRunData[n].fPackTotalVolt;
    pBlobIfyBMS->fPackTotalCurr =                    s_infy_BattRunData[n].fOutputCurrent;
    pBlobIfyBMS->fOutputPower =                    	 s_infy_BattRunData[n].fOutputPower;
    pBlobIfyBMS->fOutsideVolt =                    	 s_infy_BattRunData[n].fOutsideVolt;

	//--- 0x143dffxx  ---//
    pBlobIfyBMS->fSOC =                              (UINT)s_infy_BattRunData[n].fSOC;
	pBlobIfyBMS->fSOE =                            	 s_infy_BattRunData[n].fSOE;
    pBlobIfyBMS->fSOH =                              (UINT)s_infy_BattRunData[n].fSOH;
    pBlobIfyBMS->fTotalCapacity =                  	 s_infy_BattRunData[n].fTotalCapacity;

	//--- 0x143effxx  ---//
	pBlobIfyBMS->fCellAddTotalVolt =               	 s_infy_BattRunData[n].fCellAddTotalVolt;
	pBlobIfyBMS->usCellAverageVolt = 				 s_infy_BattRunData[n].usCellAverageVolt;
	pBlobIfyBMS->sCellAverageTemp = 				 s_infy_BattRunData[n].sCellAverageTemp;
	pBlobIfyBMS->sBattCabinetTemp = 				 s_infy_BattRunData[n].sCellAverageTemp;

	//--- 0x143fffxx  ---//
	pBlobIfyBMS->usMaxCellVlot_Cell = 				 s_infy_BattRunData[n].usMaxCellVlot_Cell;
	pBlobIfyBMS->usMaxCellVlot_Volt = 				 s_infy_BattRunData[n].usMaxCellVlot_Volt;
	pBlobIfyBMS->usMinCellVlot_Cell = 				 s_infy_BattRunData[n].usMinCellVlot_Cell;
	pBlobIfyBMS->usMinCellVlot_Volt = 				 s_infy_BattRunData[n].usMinCellVlot_Volt;

	//--- 0x1440ffxx  ---//
	pBlobIfyBMS->usMaxCellTemp_Cell =				s_infy_BattRunData[n].usMaxCellTemp_Cell;  
	pBlobIfyBMS->sMaxCellTemp_Temp 	=				s_infy_BattRunData[n].sMaxCellTemp_Temp;
	pBlobIfyBMS->usMinCellTemp_Cell =				s_infy_BattRunData[n].usMinCellTemp_Cell;
	pBlobIfyBMS->sMinCellTemp_Temp 	=				s_infy_BattRunData[n].sMinCellTemp_Temp; 

	//--- 0x1442ffxx ---//
 	pBlobIfyBMS->fNegativeBusVolt = 				s_infy_BattRunData[n].fNegativeBusVolt;

	//--- 0x1443ffxx  --//
	pBlobIfyBMS->fInsulation_P_R = 					s_infy_BattRunData[n].fInsulation_P_R;
	pBlobIfyBMS->fInsulation_N_R = 					s_infy_BattRunData[n].fInsulation_N_R;

	//--- 0x1446ffxx ---//
	pBlobIfyBMS->usSystemRunTime 	= 				s_infy_BattRunData[n].usSystemRunTime;
	pBlobIfyBMS->usCanChargeTime	= 				s_infy_BattRunData[n].usCanChargeTime;
	pBlobIfyBMS->usCanDischargeTime	= 				s_infy_BattRunData[n].usCanDischargeTime;
	pBlobIfyBMS->usCycleTimes		= 				s_infy_BattRunData[n].usCycleTimes;

	//--- 0x1447ffxx ---//
	pBlobIfyBMS->usMonAlarmNum	=					s_infy_BattRunData[n].usMonAlarmNum;
	pBlobIfyBMS->usMonFaultNum	=					s_infy_BattRunData[n].usMonFaultNum;
	pBlobIfyBMS->usAccuAlarmNum	=					s_infy_BattRunData[n].usAccuAlarmNum;
	pBlobIfyBMS->usAccuFaultNum	=					s_infy_BattRunData[n].usAccuFaultNum;

	//--- 0x1448ffxx  ---//
	pBlobIfyBMS->fRemainEnergy			=			s_infy_BattRunData[n].fRemainEnergy;
	pBlobIfyBMS->fTotalEnergy			=			s_infy_BattRunData[n].fTotalEnergy;
	pBlobIfyBMS->fSingleChgCapacity		=			s_infy_BattRunData[n].fSingleChgCapacity;
	pBlobIfyBMS->fSingleDischgCapacity	=			s_infy_BattRunData[n].fSingleDischgCapacity;
	
	//---0x1449ffxx  -----//
	pBlobIfyBMS->fAccuChgCapacity		=			s_infy_BattRunData[n].fAccuChgCapacity;
	pBlobIfyBMS->fAccuDischgCapacity	=			s_infy_BattRunData[n].fAccuDischgCapacity;
	
	//---0x144affxx  -----//
	pBlobIfyBMS->fAccuChgEnergy			= 			s_infy_BattRunData[n].fAccuChgEnergy;
	pBlobIfyBMS->fAccuDischgEnergy		= 			s_infy_BattRunData[n].fAccuDischgEnergy;
	
	//---0x144bffxx  -----//
	pBlobIfyBMS->fNowDOD				= 			s_infy_BattRunData[n].fNowDOD;
	pBlobIfyBMS->fDischgStartDOD		= 			s_infy_BattRunData[n].fDischgStartDOD;

	//---0x144cffxx  -----//
	pBlobIfyBMS->usDiagSOSLev 	=					s_infy_BattRunData[n].usDiagSOSLev;
	pBlobIfyBMS->usDiagSOSCode 	=					s_infy_BattRunData[n].usDiagSOSCode;

	//--- 0x0301ffxx  -----//
	pBlobIfyBMS->byBMS_Alive =						s_infy_BattRunData[n].byBMS_Alive;

	//--- 0x1028ffxx  -----//
	pBlobIfyBMS->usCellVoltTotalNum	=				s_infy_BattRunData[n].usCellVoltTotalNum;
	pBlobIfyBMS->usCellTempTotalNum =				s_infy_BattRunData[n].usCellTempTotalNum;
	pBlobIfyBMS->fStandTotalVoltage =				s_infy_BattRunData[n].fStandTotalVoltage;
	pBlobIfyBMS->fStandCapacity		=				s_infy_BattRunData[n].fStandCapacity;
	pBlobIfyBMS->fStandEnergy 		=				s_infy_BattRunData[n].fStandEnergy;

	memcpy(pBlobIfyBMS->bySystemVersion,   s_infy_BattRunData[n].bySystemVersion,   8);
	memcpy(pBlobIfyBMS->bySoftwareVersion, s_infy_BattRunData[n].bySoftwareVersion, 8);
	memcpy(pBlobIfyBMS->byHardwareVersion, s_infy_BattRunData[n].byHardwareVersion, 8);

 	//--- 0x1050ffxx  -----//
 	pBlobIfyBMS->uiPermissionLevel 	=				s_infy_BattRunData[n].uiPermissionLevel;

 	//--- 0x14e1ffxx  -----//
 	pBlobIfyBMS->fLeakageCurrent 	=				s_infy_BattRunData[n].fLeakageCurrent;
 	pBlobIfyBMS->nSoftVer 			=				s_infy_BattRunData[n].nSoftVer;

	//edit by Kuangyue 20241202
	pBlobIfyBMS->nAlarmInfo1 		= 				s_infy_BattRunData[n].nAlarmInfo1;
	pBlobIfyBMS->nAlarmInfo2 		= 				s_infy_BattRunData[n].nAlarmInfo2;
	pBlobIfyBMS->nAlarmInfo3 		= 				s_infy_BattRunData[n].nAlarmInfo3;
	pBlobIfyBMS->nAlarmInfo4 		= 				s_infy_BattRunData[n].nAlarmInfo4;
	pBlobIfyBMS->nAlarmInfo5 		= 				s_infy_BattRunData[n].nAlarmInfo5;
	pBlobIfyBMS->nAlarmInfo6 		= 				s_infy_BattRunData[n].nAlarmInfo6;
	pBlobIfyBMS->nAlarmInfo7 		= 				s_infy_BattRunData[n].nAlarmInfo7;

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
static void YFY_Map_BattRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
    A_BLOB_GROUP_STBATT_RUNINFO     *pGrpRunInfo = &pSTBatt_Rough->blobGroupBattInfo;
	A_BLOB_SINGLE_STBATT_RUNINFO    *pSingleRunInfo = NULL;
	A_BLOB_IFY_BMS_DATAINFO         *pBlobIfyBMS = NULL;
	int nOnlineTotal = 0;
	int nBattPackCellNum = 0;

    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
        pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
        pBlobIfyBMS = &pSTBatt_Rough->blobIfyBMS[n];

 		//skip those not configed
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			ZERO_POBJS(pSingleRunInfo, 1);
            ZERO_POBJS(pBlobIfyBMS, 1);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_NOT_INSTALLED;
			TRACEX("Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		else if((s_infy_BattRunData[n].bIsCommFail) ||
			(!s_infy_BattRunData[n].bBattInfo_Regisited))
		{
			TRACEX("BattIdx=[%d] bIsCommFail = %d, bBattInfo_Regisited = %d\n", n, s_infy_BattRunData[n].bIsCommFail, s_infy_BattRunData[n].bBattInfo_Regisited);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;
			ND_V33_Init_RoughData(pSTBatt_Rough, n);
 			//nOnlineTotal++;
			continue;
		}

        nOnlineTotal++;

        Map_RawData2IfyBmsBlob(pSTBatt_Rough, n);
         
        //Now handle status
        //TRACE("s_infy_BattRunData[n].emBattSysState:%d\n", s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState);
        switch(s_infy_BattRunData[n].emBattSysState)
        {
            case YFYBATT_SYSSTAT_HV_UP: //Normal Statu
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
                    break;
                }
            case YFYBATT_SYSSTAT_CHARGE_FULL: //Full Charge Statu
                {
                    pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_FULL_CHARGE;
                    //pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
                    break;
                }
            case YFYBATT_SYSSTAT_DISCHARGE_FULL: //Full DisCharge Statu
                {
                    pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_FULL_DISCHARGE;
                    //pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
                    break;
                }
            case YFYBATT_SYSSTAT_CHG_DISC_FORBID: //Forbid Charge & Discharge
                {
					if(FLOAT_EQU_ZERO(s_infy_BattRunData[n].fAllow_MaxChargeCurr) &&
						FLOAT_EQU_ZERO(s_infy_BattRunData[n].fAllow_MaxDisChargeCurr))
					{
						pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
						//pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
						//pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
					}
					else if(pSingleRunInfo->fPackTotalCurr > 2.0)
                    {
                        pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_CHARGE;
                    }
                    else if(pSingleRunInfo->fPackTotalCurr < -2.0)
                    {
                        pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_DISCHARGE;
                    }
					else
					{
						pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
					}
                    break;
                }
            case YFYBATT_SYSSTAT_HV_DOWN_PROTECT: //Protect
                {
                    pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
                    break;
                }
            default: //0-Initial Statu
                {
                    pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
                }
                break;
        }
        //alarm status
        
        //if((s_infy_BattRunData[n].unioBattFaultCode.uFlag[0] > 0)  || 
        //    (s_infy_BattRunData[n].unioBattFaultCode.uFlag[1] > 0) || 
        //    (s_infy_BattRunData[n].mapSysFault.uiFlag > 0))
		if(s_infy_BattRunData[n].mapSysFault.uiFlag > 0) //2022-9-26 SBK Req Not Process BMS Fault
        {
			TRACE("Idx=[%d]\n", s_infy_BattRunData[n].unioBattFaultCode.uFlag[0],\
				s_infy_BattRunData[n].unioBattFaultCode.uFlag[1],\
				s_infy_BattRunData[n].mapSysFault.uiFlag);

            pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_FAULT;
            //must set status into sleep in case BMS not set
            //pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
            //pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
            pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
        }
        else if(s_infy_BattRunData[n].emBattSysState == YFYBATT_SYSSTAT_HV_DOWN_PROTECT)
        {
            pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_PROTECT;
            //must set status into sleep in case BMS not set
            //pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
            //pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
            pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
        }
        else if((s_infy_BattRunData[n].unioBattAlarmCode.uFlag[0] > 0)  || 
            (s_infy_BattRunData[n].unioBattAlarmCode.uFlag[1] > 0) )
        {
            if( (fabs(pSingleRunInfo->fAllow_MaxDisChargeCurr) < 5.0) &&
                (fabs(pSingleRunInfo->fAllow_MaxChargeCurr) < 5.0) )   
            {
                pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_ALARMING;
                //pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
                //pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
                //must set status into sleep in case BMS not set
                pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
            }
            else
            {
                pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
				//SBK Suggeset NoNeed Process Alarm Status
                //must set status into sleep in case BMS not set
				//John 2022-09-17, Alarm Statu Need Judge SOC Value.
				if(pSingleRunInfo->nSOC <= (pSTBatt_Rough->nProtectSOC + pSTBatt_Rough->nPVSysBattRetSOC))
				{
					//Mppt Chrging to Battery 2A,cause Battery SOC=30->31, then cause ACDC Running ON->OFF->ON
					TRACE("BattIdx=[%d]SOC=%d nProtectSOC=%d emBattStatus_Run=%d emLast_BattStatus_Run=%d\n",\
						n,pSingleRunInfo->nSOC , pSTBatt_Rough->nProtectSOC, pSingleRunInfo->emBattStatus_Run,\
						pSingleRunInfo->emLast_BattStatus_Run);

					if((pSingleRunInfo->nSOC <= pSTBatt_Rough->nProtectSOC) || (pSingleRunInfo->emLast_BattStatus_Run == STBATT_RUNSTAT_SLEEP))
					{
						//pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
						pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
					}
				}
            }
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
                //pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
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
		//John Remove This Log,Due To BMS Sometimes Status will Normal->Abnormal
        /*if( (uLastAlarm[n][0] != s_infy_BattRunData[n].unioBattAlarmCode.uFlag[0]) ||
            (uLastAlarm[n][1] != s_infy_BattRunData[n].unioBattAlarmCode.uFlag[1]) ||
            (uLastFault[n][0] != s_infy_BattRunData[n].unioBattFaultCode.uFlag[0]) ||
            (uLastFault[n][1] != s_infy_BattRunData[n].unioBattFaultCode.uFlag[1]))			
        {
            uLastAlarm[n][0] = s_infy_BattRunData[n].unioBattAlarmCode.uFlag[0];
            uLastAlarm[n][1] = s_infy_BattRunData[n].unioBattAlarmCode.uFlag[1];
            uLastFault[n][0] = s_infy_BattRunData[n].unioBattFaultCode.uFlag[0];
            uLastFault[n][1] = s_infy_BattRunData[n].unioBattFaultCode.uFlag[1];
            
            if((uLastAlarm[n][0] > 0) || (uLastAlarm[n][1] > 0) || (uLastFault[n][0] > 0) || (uLastFault[n][1] > 0))
            {
                LogOut(MAIN_MODULE_SAMPLER, "BAT", LOG_TYPE_ERROR,
                    "Alm:[%8X%8X|%8X%8X]", uLastAlarm[0], uLastAlarm[1], uLastFault[0], uLastFault[1]);
            }
        }*/
        pSingleRunInfo->emLast_BattStatus_Run = pSingleRunInfo->emBattStatus_Run;
    }//End For Cycle

	pGrpRunInfo->nTotalOnlineBatt = nOnlineTotal;
	
	return;
}

static void YFY_SingleAirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,int nBattIdx)
{
	AIR_STATUS_ALARM_FAULT     *pAirRoughInfo = &pSTBatt_Rough->stAirStatusAlarm[nBattIdx];

	//---liquid AIR status01---//
	pAirRoughInfo->nPowerStatus  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nPowerStatus; 
	pAirRoughInfo->nWorkMode  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nWorkMode; 
	pAirRoughInfo->fColdTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fColdTemp; 

	pAirRoughInfo->nColdOpenDiff  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nColdOpenDiff; 
	pAirRoughInfo->nColdStopDiff  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nColdStopDiff; 
	pAirRoughInfo->fHeatTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fHeatTemp; 

	pAirRoughInfo->fHeatDiff  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fHeatDiff; 
	pAirRoughInfo->nRefrigerantType  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nRefrigerantType; 
	pAirRoughInfo->nUpMachineBaud  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nUpMachineBaud; 

	pAirRoughInfo->nContactAddr  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nContactAddr; 

	//---liquid AIR status02---//
	pAirRoughInfo->nOutsideMachinAbility  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nOutsideMachinAbility; 
	//pAirRoughInfo->nSoftVer  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nSoftVer; 
	memcpy(pAirRoughInfo->byBMSAirSoftVersion,   s_infy_BattRunData[nBattIdx].stAirStatusAlarm.byBMSAirSoftVersion,   16);
	pAirRoughInfo->nPLS  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nPLS; 

	pAirRoughInfo->fSys1TargetCompFreq  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1TargetCompFreq; 
	pAirRoughInfo->fSys1CurrentCompFreq  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1CurrentCompFreq; 
	pAirRoughInfo->nFanTargetSpeed  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nFanTargetSpeed; 

	pAirRoughInfo->fExhaustSuperheat  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fExhaustSuperheat; 
	pAirRoughInfo->fSys1AmbientTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1AmbientTemp; 
	pAirRoughInfo->fSys1ExhaustTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1ExhaustTemp; 

	pAirRoughInfo->fSys1SuctionTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1SuctionTemp; 
	pAirRoughInfo->fModuleInletTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fModuleInletTemp; 
	pAirRoughInfo->fModuleOutletTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fModuleOutletTemp; 

	pAirRoughInfo->fBatteryInletTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fBatteryInletTemp; 
	pAirRoughInfo->fBatteryOutletTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fBatteryOutletTemp; 
	pAirRoughInfo->nPumpActualSpeed  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nPumpActualSpeed; 

	pAirRoughInfo->fModuleOutletPressure  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fModuleOutletPressure; 
	pAirRoughInfo->fBatteryOutletPressure  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fBatteryOutletPressure; 
	pAirRoughInfo->fSys1HighSatTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1HighSatTemp; 

	pAirRoughInfo->fSys1LowSatTemp  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1LowSatTemp; 
	pAirRoughInfo->fSuctionSuperheat  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSuctionSuperheat; 
	pAirRoughInfo->fSys1LowPressure  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1LowPressure; 

	pAirRoughInfo->fSys1HighPressure  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1HighPressure; 
	pAirRoughInfo->fSys1ACInputVoltage  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1ACInputVoltage; 
	pAirRoughInfo->fSys1ACInputCurrent  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1ACInputCurrent; 

	pAirRoughInfo->nSys1CompressorCurrent  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.nSys1CompressorCurrent; 
	pAirRoughInfo->fSys1DCBusVoltage  = s_infy_BattRunData[nBattIdx].stAirStatusAlarm.fSys1DCBusVoltage; 

	//---liquid AIR status03---//
	memcpy(&pAirRoughInfo->AirSysFault1,&s_infy_BattRunData[nBattIdx].stAirStatusAlarm.AirSysFault1,sizeof(STSYS_FAULT_MAP_TATE1BIT));
	memcpy(&pAirRoughInfo->AirSysFault2,&s_infy_BattRunData[nBattIdx].stAirStatusAlarm.AirSysFault2,sizeof(STSYS_FAULT_MAP_TATE2BIT));
	memcpy(&pAirRoughInfo->AirSysFault3,&s_infy_BattRunData[nBattIdx].stAirStatusAlarm.AirSysFault3,sizeof(STSYS_FAULT_MAP_TATE3BIT));
	memcpy(&pAirRoughInfo->AirSysFault4,&s_infy_BattRunData[nBattIdx].stAirStatusAlarm.AirSysFault4,sizeof(STSYS_FAULT_MAP_TATE4BIT));
	memcpy(&pAirRoughInfo->AirSysFault5,&s_infy_BattRunData[nBattIdx].stAirStatusAlarm.AirSysFault5,sizeof(STSYS_FAULT_MAP_TATE5BIT));
	memcpy(&pAirRoughInfo->AirSysFault6,&s_infy_BattRunData[nBattIdx].stAirStatusAlarm.AirSysFault6,sizeof(STSYS_FAULT_MAP_TATE6BIT));
	return;
}

static void YFY_AirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("YFY_AirRawData_2Rough---Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		YFY_SingleAirRawData_2Rough(pSTBatt_Rough,n);
	}
	
	return;
}

static void TongFei_SingleAirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,int nBattIdx)
{
	TONGFEI_AIR_STATUS_ALARM_FAULT		*pTongFeiAirRoughInfo = &pSTBatt_Rough->stTongFeiAirStatusAlarm[nBattIdx];
	
	//空调参数信息
	pTongFeiAirRoughInfo->nStartupMode = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nStartupMode;
	pTongFeiAirRoughInfo->nWorkingMode = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nWorkingMode;
	pTongFeiAirRoughInfo->nSuplyWater = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nSuplyWater;

	pTongFeiAirRoughInfo->fRefrigerationPresetTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fRefrigerationPresetTemperature;
	pTongFeiAirRoughInfo->fCoolingTemperatureTifference = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fCoolingTemperatureTifference;
	pTongFeiAirRoughInfo->fHeatingTresetTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fHeatingTresetTemperature;

	pTongFeiAirRoughInfo->fHeatingTemperatureTifference = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fHeatingTemperatureTifference;


	//空调状态信息
	pTongFeiAirRoughInfo->nUnitOperatingStatus = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nUnitOperatingStatus;
	pTongFeiAirRoughInfo->nUnitOperatingMode = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nUnitOperatingMode;
	pTongFeiAirRoughInfo->nSwitchingInput = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nSwitchingInput;
	pTongFeiAirRoughInfo->nRelayInput = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nRelayInput;

	pTongFeiAirRoughInfo->fLiquidSupplyTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fLiquidSupplyTemperature;
	pTongFeiAirRoughInfo->fReturnLiquidTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fReturnLiquidTemperature;
	pTongFeiAirRoughInfo->fSystemSteamingIn = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fSystemSteamingIn;

	pTongFeiAirRoughInfo->fSteamOutTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fSteamOutTemperature;
	pTongFeiAirRoughInfo->fAmbientTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fAmbientTemperature;
	pTongFeiAirRoughInfo->fSystemCondensationTemperature = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.fSystemCondensationTemperature;

	pTongFeiAirRoughInfo->nCurrentOpeningDegree = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nCurrentOpeningDegree;
	pTongFeiAirRoughInfo->nCurrentSpeedWaterPump = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nCurrentSpeedWaterPump;
	pTongFeiAirRoughInfo->nCurrentSpeedCompressor = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nCurrentSpeedCompressor;

	pTongFeiAirRoughInfo->nFanSpeed = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nFanSpeed;
	pTongFeiAirRoughInfo->nWaterSupplyPressure = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nWaterSupplyPressure;
	pTongFeiAirRoughInfo->nReturnWaterPressure = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nReturnWaterPressure;

	pTongFeiAirRoughInfo->nExhaustPressure = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nExhaustPressure;


	//空调故障信息
	pTongFeiAirRoughInfo->nFaultStatus1 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nFaultStatus1;
	pTongFeiAirRoughInfo->nFaultStatus2 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nFaultStatus2;
	pTongFeiAirRoughInfo->nFaultStatus3 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nFaultStatus3;

	pTongFeiAirRoughInfo->nFaultStatus4 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nFaultStatus4;
	pTongFeiAirRoughInfo->nFaultStatus5 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nFaultStatus5;

	pTongFeiAirRoughInfo->nVariableFrequencyFault1 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nVariableFrequencyFault1;
	pTongFeiAirRoughInfo->nVariableFrequencyFault2 = s_infy_BattRunData[nBattIdx].stTongFeiAirStatusAlarm.nVariableFrequencyFault2;


	return;
}

static void TongFei_AirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("YFY_AirRawData_2Rough---Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		TongFei_SingleAirRawData_2Rough(pSTBatt_Rough, n);
	}
	
	return;
}
static void IMEU3_SingleAirContralRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,int nBattIdx)
{
	//edit by kuangyue 20250811
	IMEU3_AIR_STATUS_CONTROL_INFO *pIMEU3AirControlInfo = &pSTBatt_Rough->stIMEU3AirControlInfo[nBattIdx];

	pIMEU3AirControlInfo->byCellPwrState = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.byCellPwrState;
	pIMEU3AirControlInfo->byPcsPwrState = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.byPcsPwrState;
	pIMEU3AirControlInfo->usBatteryMode = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.usBatteryMode;

	pIMEU3AirControlInfo->byControlObj = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.byControlObj;
	pIMEU3AirControlInfo->fCoolingTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.fCoolingTemp;
	pIMEU3AirControlInfo->fHeatingTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.fHeatingTemp;

	pIMEU3AirControlInfo->byNoiseReductionMode = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.byNoiseReductionMode;
	pIMEU3AirControlInfo->fCoolingTempDiff = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.fCoolingTempDiff;
	pIMEU3AirControlInfo->fHeatingTempDiff = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.fHeatingTempDiff;

	pIMEU3AirControlInfo->fCoolingStopTempDiff = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.fCoolingStopTempDiff;
	pIMEU3AirControlInfo->fHeatingStopTempDiff = s_infy_BattRunData[nBattIdx].stIMEU3AirControlStatus.fHeatingStopTempDiff;
}

static void IMEU3_SingleAirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,int nBattIdx)
{
	//edit by kuangyue 20250217
	IMEU3_AIR_STATUS_ALARM_FAULT *pIMEU3AirRoughInfo = &pSTBatt_Rough->stIMEU3AirStatusFault[nBattIdx];

	// 空调参数信息
	pIMEU3AirRoughInfo->bSoftVer = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.bSoftVer;
	pIMEU3AirRoughInfo->uPowerStat = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uPowerStat;
	pIMEU3AirRoughInfo->uOpMode = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uOpMode;

	pIMEU3AirRoughInfo->uOpStep = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uOpStep;
	pIMEU3AirRoughInfo->uErrCode = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uErrCode;
	pIMEU3AirRoughInfo->fTgtTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fTgtTemp;

	pIMEU3AirRoughInfo->fRetTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fRetTemp;
	pIMEU3AirRoughInfo->fOverheatTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fOverheatTemp;
	pIMEU3AirRoughInfo->fSatTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fSatTemp;

	pIMEU3AirRoughInfo->uExpValveOpen = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uExpValveOpen;
	pIMEU3AirRoughInfo->uTotalCap = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uTotalCap;
	pIMEU3AirRoughInfo->uCompTgtHz = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uCompTgtHz;

	pIMEU3AirRoughInfo->uCompOpHz = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uCompOpHz;
	pIMEU3AirRoughInfo->uCompOpPhase = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.uCompOpPhase;
	pIMEU3AirRoughInfo->nFaultLv = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nFaultLv;

	pIMEU3AirRoughInfo->fexhaustTempFault = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fexhaustTempFault;
	pIMEU3AirRoughInfo->fEvapInTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fEvapInTemp;
	pIMEU3AirRoughInfo->fCoolInTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fCoolInTemp;

	pIMEU3AirRoughInfo->fModuleOutTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fModuleOutTemp;
	pIMEU3AirRoughInfo->fModuleInTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fModuleInTemp;
	pIMEU3AirRoughInfo->fBattInTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fBattInTemp;

	pIMEU3AirRoughInfo->fBattOutTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fBattOutTemp;
	pIMEU3AirRoughInfo->fCoolModOutTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fCoolModOutTemp;
	pIMEU3AirRoughInfo->fCoolBattOutTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fCoolBattOutTemp;

	pIMEU3AirRoughInfo->fAmbTemp = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fAmbTemp;
	pIMEU3AirRoughInfo->fHighPress = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fHighPress;
	pIMEU3AirRoughInfo->fLowPress = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fLowPress;

	pIMEU3AirRoughInfo->fBattOutPress = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fBattOutPress;
	pIMEU3AirRoughInfo->fModuleOutPress = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fModuleOutPress;
	pIMEU3AirRoughInfo->fCoolInPress = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fCoolInPress;

	pIMEU3AirRoughInfo->fCoolOutPress = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fCoolOutPress;
	pIMEU3AirRoughInfo->nFaultLvl = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nFaultLvl;
	pIMEU3AirRoughInfo->nVfFault2 = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nVfFault2;

	pIMEU3AirRoughInfo->nPfcFault = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nPfcFault;
	pIMEU3AirRoughInfo->nCompFault1 = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nCompFault1;
	pIMEU3AirRoughInfo->nCompFault2 = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nCompFault2;

	pIMEU3AirRoughInfo->nSysStatFault = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.nSysStatFault;
	pIMEU3AirRoughInfo->fBattPumpHz = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fBattPumpHz;
	pIMEU3AirRoughInfo->fBattPumpDuty = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fBattPumpDuty;

	pIMEU3AirRoughInfo->fModulePumpHz = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fModulePumpHz;
	pIMEU3AirRoughInfo->fModulePumpDuty = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fModulePumpDuty;
	pIMEU3AirRoughInfo->fDrvFanHz = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fDrvFanHz;

	pIMEU3AirRoughInfo->fDrvFanDuty = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fDrvFanDuty;
	pIMEU3AirRoughInfo->fFanHz = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fFanHz;
	pIMEU3AirRoughInfo->fFanDuty = s_infy_BattRunData[nBattIdx].stIMEU3AirStatusFault.fFanDuty;



	return;
}

static void IMEU3_AirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//edit by kuangyue 20250217
	int n = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("YFY_AirRawData_2Rough---Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		IMEU3_SingleAirRawData_2Rough(pSTBatt_Rough, n);
		IMEU3_SingleAirContralRawData_2Rough(pSTBatt_Rough, n);//edit by kuangyue 20250811
	}
	
	return;
}

static void XinMiao_SingleAirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,int nBattIdx)
{
	//edit by kuangyue 20250217
	XINMIAO_AIR_STATUS_ALARM_FAULT *pXinMiaoAirRoughInfo = &pSTBatt_Rough->stXinMiaoAirStatusFault[nBattIdx];

	// 空调参数信息
	pXinMiaoAirRoughInfo->nPumpStat = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nPumpStat;
	pXinMiaoAirRoughInfo->nComp1Stat = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nComp1Stat;
	pXinMiaoAirRoughInfo->nComp2Stat = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nComp2Stat;

	pXinMiaoAirRoughInfo->nHeatStat = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nHeatStat;
	pXinMiaoAirRoughInfo->nCoolFanStat = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nCoolFanStat;
	pXinMiaoAirRoughInfo->fOutTemp = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.fOutTemp;

	pXinMiaoAirRoughInfo->fRetTemp = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.fRetTemp;
	pXinMiaoAirRoughInfo->fOutPress = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.fOutPress;
	pXinMiaoAirRoughInfo->fRetPress = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.fRetPress;

	pXinMiaoAirRoughInfo->fenvTempFault = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.fenvTempFault;
	pXinMiaoAirRoughInfo->nMakeUpPump = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nMakeUpPump;
	pXinMiaoAirRoughInfo->nFaultCode = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nFaultCode;

	pXinMiaoAirRoughInfo->nWorkMode = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nWorkMode;
	pXinMiaoAirRoughInfo->nPowerStat = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.nPowerStat; 
	pXinMiaoAirRoughInfo->fTgtTemp = s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.fTgtTemp; 

	memcpy(&pXinMiaoAirRoughInfo->faultBits,&s_infy_BattRunData[nBattIdx].stXinMiaoAirStatusFault.faultBits,sizeof(XINMIAO_AIR_FAULT_STATUS_BITS));

	return;
}

static void XinMiao_AirRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//edit by kuangyue 20250217
	int n = 0;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("YFY_AirRawData_2Rough---Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		XinMiao_SingleAirRawData_2Rough(pSTBatt_Rough, n);
	}
	
	return;
}

static void YFY_SinglePackRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough,int nBattIdx)
{
	LIQUID_COOLING_PACK_STATUS   *pPackRoughInfo = &pSTBatt_Rough->stLiquidCoolingPack[nBattIdx];

	int i = 0;

	for(i = 0; i < MAX_COOLINGPACK_NUM; i++)
	{

		pPackRoughInfo->fPositiveTemp[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.fPositiveTemp[i];
		pPackRoughInfo->fNegativeTemp[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.fNegativeTemp[i];
		pPackRoughInfo->uImmersionFreq[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.uImmersionFreq[i];

		pPackRoughInfo->fHighVoltageValue[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.fHighVoltageValue[i];
		pPackRoughInfo->uHighVoltageDuty[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.uHighVoltageDuty[i];

		pPackRoughInfo->uSol[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.uSol[i];
		pPackRoughInfo->uH2[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.uH2[i];
		pPackRoughInfo->uAerosolStatusAndFault[i] = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.uAerosolStatusAndFault[i];

	}
		pPackRoughInfo->usBsu1BotNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu1BotNum;
		pPackRoughInfo->usBsu1TopNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu1TopNum;
		pPackRoughInfo->usBsu2BotNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu2BotNum;

		pPackRoughInfo->usBsu2TopNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu2TopNum;
		pPackRoughInfo->usBsu3BotNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu3BotNum;
		pPackRoughInfo->usBsu3TopNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu3TopNum;

		pPackRoughInfo->usBsu4BotNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu4BotNum;
		pPackRoughInfo->usBsu4TopNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu4TopNum;
		pPackRoughInfo->usBsu5BotNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu5BotNum;

		pPackRoughInfo->usBsu5TopNum = s_infy_BattRunData[nBattIdx].stLiquidCoolingPack.usBsu5TopNum;

	return;

}


static void YFY_PackRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			//TRACEW("YFY_AirRawData_2Rough---Skip Single Battery Idx=[%d]\n",n);
			continue;
		}
		YFY_SinglePackRawData_2Rough(pSTBatt_Rough, n);
	}
	
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
static BOOL YFY_JudgeIfNeedReconfig(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int 	n = 0;
	BOOL 	bNeed = FALSE;

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			break;
		}
		if((!s_infy_BattRunData[n].bBattInfo_Regisited) ||
        	s_infy_BattRunData[n].bIsCommFail || 
			(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt))
        {
			TRACEX("n=%d bIsCommFail=%d nSystemCfgBattNum=%d nTotalOnlineBatt=%d bBattInfo_Regisited=%d\n",\
				n, s_infy_BattRunData[n].bIsCommFail, pSTBatt_Rough->nSystemCfgBattNum,\
				pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt,s_infy_BattRunData[n].bBattInfo_Regisited);

            bNeed = TRUE;
            break;
        }
	}

	return bNeed;
}




/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2022-01-13 11:13:53
*  DESCRIPTION   :  
*				    
*  Emergency Alarm Need Send Power Off
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void YFY_JudgeBMSAvaliableStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{	
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	int         n = 0;
	VAR_VALUE 	varV;

 	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 200, 308 + n, &varV, 4);
		pSTBatt_Rough->bCtrlOutRelayClose[n] = (varV.emValue == 0) ? STAT_RELAY_ON : STAT_RELAY_OFF;

		if(s_infy_BattRunData[n].bIsCommFail == TRUE)
		{
			continue;
		}
		//QF3   ---------
		
		pDPR->Get((HANDLE)pSampler, GETTYPE_ALMSTATE, 200, 241, &varV, 4);
		s_infy_BattRunData[n].byPCS_Cmd = (varV.emValue == 0) ? STBATT_YFY_POWERON_CMD : STBATT_YFY_POWEROFF_CMD;

		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 200, 252, &varV, sizeof(VAR_VALUE));
		s_infy_BattRunData[n].byPCS_Cmd = (varV.emValue == 0) ? STBATT_YFY_POWERON_CMD : STBATT_YFY_POWEROFF_CMD;

	}
	return;
}


static void YFY_RefreshLigHTState(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	
	int         n = 0;
	VAR_VALUE 	varV;

 	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		if(s_infy_BattRunData[n].bIsCommFail == TRUE)
		{
			memset(&s_infy_BattRunData[n].unioLigHT, 0, sizeof(IFY_BATT_LIGHT_STAT));
			continue;
		}
		
 		s_infy_BattRunData[n].unioLigHT.Stat.Chg_Stat 		= (s_infy_BattRunData[n].emBattChgState == YFYBATT_CHGSTAT_CHARGE) ? 1 : 0;
		s_infy_BattRunData[n].unioLigHT.Stat.Dischg_Stat 	= (s_infy_BattRunData[n].emBattChgState == YFYBATT_CHGSTAT_DISCHARGE) ? 1 : 0;
 		s_infy_BattRunData[n].unioLigHT.Stat.SysFault 		= (pSTBatt_Rough->ccuStatu == CCU_STATE_FAULTED) ? 1 : 0;
 		s_infy_BattRunData[n].unioLigHT.Stat.IdleStat 		= (s_infy_BattRunData[n].emBattChgState == YFYBATT_CHGSTAT_IDLE) ? 1 : 0;
 		s_infy_BattRunData[n].unioLigHT.Stat.OffGridStat	= (pSTBatt_Rough->esRunData.mapSTSCode.Status.OnOffGrid_State == 1) ? 1 : 0;
 		s_infy_BattRunData[n].unioLigHT.Stat.MpptStat 		= (pSTBatt_Rough->esRunData.emMPPT_WorkStatu[n] == STATU_MPPT_RECTIFY) ? 1 : 0;
	}
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

static void YFY_NormalRunJudgeBattCabinetStatu(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n = 0;
	time_t tmNow = _NOW_;

 	for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	{
		if(s_infy_BattRunData[n].bIsCommFail == TRUE)
		{
			continue;
		}
		
		if(s_infy_BattRunData[n].byPCS_Cmd != STBATT_V33_POWERON_CMD)
	 	{
			 continue;
		}
		
 		if((s_infy_BattRunData[n].emBattSysState == YFYBATT_SYSSTAT_HV_DOWN) ||
		   (s_infy_BattRunData[n].emBattSysState == YFYBATT_SYSSTAT_HV_DOWN_PROTECT) ||
			isBMSRunAbnormal(pSTBatt_Rough, n, s_infy_BattRunData[n].fOutputCurrent))//CCU CommFail Alarm
		{
			UINT nWaitBMSPowerTimes = (s_infy_BattRunData[n].emBattSysState == YFYBATT_SYSSTAT_HV_DOWN_PROTECT) ? 300 : 30;//Added By John2022-03-29,SBK Advs Need Wait 30S

			pSTBatt_Rough->nPowerONTimes[n]++;
			
			TRACE("nPowerONTimes=%d s_infy_BattRunData[%d].byPCS_Cmd[%d] s_infy_BattRunData[n].emBattSysState[%d]\n",\
				pSTBatt_Rough->nPowerONTimes[n],n,s_infy_BattRunData[n].byPCS_Cmd,n, s_infy_BattRunData[n].emBattSysState);
			
			if(pSTBatt_Rough->nPowerONTimes[n] > nWaitBMSPowerTimes) //continues POWER ON Over 10 times,Power OFF
			{
				//Fault Alarm,need system power off or ctrl wake up battery
				pSTBatt_Rough->nPowerONTimes[n] = 0;
				s_infy_BattRunData[n].mapSysFault.Fault.PowerON_Fail = TRUE;
				if(pSTBatt_Rough->bCtrlOutRelayClose[n] == STAT_RELAY_ON)
				{
					CtrlBattCabinetContactor(pSTBatt_Rough, n, FALSE);
					//LogOut(MAIN_MODULE_SAMPLER, "ND Batt Sampler", LOG_TYPE_ERROR, "ND Batt Idx=[%d] V3.3 Power ON Failed!",n);	
				}
				TRACE("POWER ON FAILED: ND Batt Idx=[%d] V3.3 Power ON Failed!\n", n);
			}
		}
		else  
		{
			//Power ON Success !!!
			//TRACE("POWER ON Success: s_infy_BattRunData[n].mapSysFault.Fault.PowerON_Fail = FALSE\n");
			s_infy_BattRunData[n].mapSysFault.Fault.PowerON_Fail = FALSE;
			if(pSTBatt_Rough->bCtrlOutRelayClose[n] == STAT_RELAY_OFF)
			{

				if ((fabs(tmNow - pSTBatt_Rough->t_LastTrigIsoStartCmd[n]) < 10) 
					|| (s_infy_BattRunData[n].emIsoResult == STAT_ON_DETECTING))
				{
					//Do Nothing,Wait BMS IsoDetecting.
				}
				else
				{
					CtrlBattCabinetContactor(pSTBatt_Rough, n, TRUE);
					Sleep(500);//Avoid Off-Grid All PCS AC Contactor Connnected At someTime Will Cause Abnormal
				}
				TRACE("POWER ON Again: Battery IDx=[%d] Close Cabinet Contactor\n", n);
			}
		}
	}
	return;
}



static void YFY_IsolationCtrlCmd4Bms(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	if((s_infy_BattRunData[nBattIdx].emBattSysState == YFYBATT_SYSSTAT_HV_DOWN) ||
		(s_infy_BattRunData[nBattIdx].emBattSysState == YFYBATT_SYSSTAT_HV_DOWN_PROTECT)) //BMS Not Power_ON
	{
		if(s_infy_BattRunData[nBattIdx].emIsoResult == STAT_ON_STOPPING)
		{
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus = TRIG_AUTO_DETECTION;
			TRACE("Idx=[%d] Not Power ON emIsoResult=[%d]\n", nBattIdx, s_infy_BattRunData[nBattIdx].emIsoResult);
		}
		else
		{
			pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus = TRIG_NO_TRIGGER;
		}
	}
	else
	{
		pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus = Judge_BMSIsoTriggerStatus(pSTBatt_Rough, nBattIdx);
	}

	if(pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus != TRIG_NO_TRIGGER)
	{
		TRACE("Idx=[%d] emIsoResult=[%d] emTrigIsoStatus=[%d] Iso Cmd.\n", \
			nBattIdx, s_infy_BattRunData[nBattIdx].emIsoResult,pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus);
		
		if((pSTBatt_Rough->blobSingleBattInfo[nBattIdx].emTrigIsoStatus == TRIG_START_DETECTING)
			&& (pSTBatt_Rough->bCtrlOutRelayClose[nBattIdx] == STAT_RELAY_ON))
		{
			CtrlBattCabinetContactor(pSTBatt_Rough, nBattIdx, FALSE);

			Sleep(200); //Wait 200mS
		}


		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_ISOLATION_CTRL); 
	}

	return;
}


static void YFY_StateMachine4BMSMsg(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBMSIdx)
{
	STU_BMS_CANMSG_STATMACH *pStatMach = &pSTBatt_Rough->stuCanSave.stuStatMach[nBMSIdx];
	
	switch (pStatMach->emSaveState)
	{
		case STAT_ON_START:
		{
			if(s_infy_BattRunData[nBMSIdx].nFaultRecNum > MAX_RECSAVE_NUM)
			{
				pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_FAULT; 
			}
			else if(s_infy_BattRunData[nBMSIdx].nLogRecNum > MAX_RECSAVE_NUM)
			{
				pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_LOG; 
			}
			else if(s_infy_BattRunData[nBMSIdx].nWaveRecNum > MAX_RECSAVE_NUM)
			{
				pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_WAVE; 
			}
			//else if(s_infy_BattRunData[nBMSIdx].nDODRecNum > 100)
			//{
			//	pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_DOD; 
			//}
			
			break;
		}
		
		case STAT_ON_PENDING:
		{
			if(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] == TP_TRIG_FAULT)
			{
				if(s_infy_BattRunData[nBMSIdx].nFaultRecNum > 0)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, nBMSIdx, CAN_RD_CMD_C2S_FAULT_REC);
				}
				else
				{
					pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_SUSPEND;
				}
			}
			else if(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] == TP_TRIG_LOG)
			{
				if(s_infy_BattRunData[nBMSIdx].nLogRecNum > 0)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, nBMSIdx, CAN_RD_CMD_C2S_LOG_REC); 
				}
				else
				{
					pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_SUSPEND;
				}
			}
			else if(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] == TP_TRIG_WAVE)
			{
				if(s_infy_BattRunData[nBMSIdx].nWaveRecNum > 0)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, nBMSIdx, CAN_RD_CMD_C2S_WAVE_REC); 
				}
				else
				{
					pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_SUSPEND; 
				}
			}
			else if(pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] == TP_TRIG_DOD)
			{
				if(s_infy_BattRunData[nBMSIdx].nDODRecNum > 0)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, nBMSIdx, CAN_RD_CMD_C2S_DOD_REC); 
				}
				else
				{
					pSTBatt_Rough->stuCanSave.emTrigMsgTp[nBMSIdx] = TP_TRIG_SUSPEND; 
				}
			}

			break;
		}

		case STAT_ON_END:
		{
			break;
		}
		default:	
			break;
	}

	return;
}


void GetBMSUpgradeStatus(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	UINT n = 0;
	//for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(s_infy_BattRunData[n].bIsCommFail == TRUE)
		{
			memset(&pSTBatt_Rough->stUpgradeParam.byData[n], 0, CAN_DATA_AREA_LEN);
			continue;
		}

		memcpy(&pSTBatt_Rough->stUpgradeParam.byData[n], s_infy_BattRunData[n].byData, CAN_DATA_AREA_LEN);
	}

	return;
}

void ClearBattUpgradeRcvBuffer()
{
	UINT n = 0;
	for (n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		memset(s_infy_BattRunData[n].byData, 0, CAN_DATA_AREA_LEN);
	}
	return;
}

static int PCSAndMPPT_TempJudg(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, int nBattIdx)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	VAR_VALUE varRect;

	A_BLOB_BRANCH_GRP_RECTDATA bGrpRect;
	ZERO_POBJS(&bGrpRect, 1);
	varRect.blobValue = (BYTE*)&bGrpRect;
	pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 211 + nBattIdx, 201, &varRect, sizeof(A_BLOB_BRANCH_GRP_RECTDATA));

	BOOL bShudownFlag = FALSE;

	int i = 0;
	for(i = 0; i < bGrpRect.nOnlineACDCNum; ++i)
	{
		
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 251 + nBattIdx*6 + i, 2263, &varV, 4);  //板温最高温度
		if(varV.fValue > pSTBatt_Rough->f35KW_OpenTemp)
		{
			return 1;
		}
		if(varV.fValue > pSTBatt_Rough->f35KW_ShutdownTemp)
		{
			bShudownFlag = TRUE;
		}
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 251 + nBattIdx*6 + i, 2264, &varV, 4);  //功率器件最高温度
		if(varV.fValue > pSTBatt_Rough->f35KW_OpenTemp)
		{
			return 1;
		}
		if(varV.fValue > pSTBatt_Rough->f35KW_ShutdownTemp)
		{
			bShudownFlag = TRUE;
		}
	}

	for(i = 0; i < bGrpRect.nOnlineMPPTNum; ++i)
	{
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 251 + nBattIdx*6 + i, 2289, &varV, 4);  //MPPT DC板温
		if(varV.fValue > pSTBatt_Rough->f35KW_OpenTemp)
		{
			return 1;
		}
		if(varV.fValue > pSTBatt_Rough->f35KW_ShutdownTemp)
		{
			bShudownFlag = TRUE;
		}
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 251 + nBattIdx*6 + i, 2290, &varV, 4);  //MPPT PFC板温
		if(varV.fValue > pSTBatt_Rough->f35KW_OpenTemp)
		{
			return 1;
		}
		if(varV.fValue > pSTBatt_Rough->f35KW_ShutdownTemp)
		{
			bShudownFlag = TRUE;
		}
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 251 + nBattIdx*6 + i, 2291, &varV, 4);  //MPPT DC变压器温度
		if(varV.fValue > (pSTBatt_Rough->f35KW_OpenTemp + 30))
		{
			return 1;
		}
		if(varV.fValue > pSTBatt_Rough->f35KW_ShutdownTemp)
		{
			bShudownFlag = TRUE;
		}
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 251 + nBattIdx*6 + i, 2292, &varV, 4);  //MPPT PFC变压器温度
		if(varV.fValue > (pSTBatt_Rough->f35KW_OpenTemp + 30))
		{
			return 1;
		}
		if(varV.fValue > pSTBatt_Rough->f35KW_ShutdownTemp)
		{
			bShudownFlag = TRUE;
		}
	}

	if(bShudownFlag == FALSE)
	{
		return 2;
	}

	return 0;  //返回0；保持液冷空调模块液冷状态
}

/*为了应对认证测试环境下无电池情况，需手动填充假数据*/
void YFY_MakeSomeFakeData(STBATT_SAMPLER_ROUGH * pSTBatt_Rough)
{
	//printf("YFY_MakeSomeFakeData --start\n");
	int nBMSGrpMapIdx = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].bIsCommFail = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].bBattInfo_Regisited = 1;

	s_infy_BattRunData[nBMSGrpMapIdx].emBattSysState = YFYBATT_SYSSTAT_HV_UP; //就绪
	s_infy_BattRunData[nBMSGrpMapIdx].usBattWorkState = 3;//运行
	s_infy_BattRunData[nBMSGrpMapIdx].emBattChgState = YFYBATT_CHGSTAT_IDLE;//空闲

	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeCurr = 295.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeCurr = 295.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargePow = 235.3;
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargePow = 235.3;

	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxChargeVolt = 848.5;
	s_infy_BattRunData[nBMSGrpMapIdx].fAllow_MaxDisChargeVolt = 696.0;

	memset(s_infy_BattRunData[nBMSGrpMapIdx].unioBattAlarmCode.szByIfyAlarm, 0, CAN_DATA_AREA_LEN);
	memset(s_infy_BattRunData[nBMSGrpMapIdx].unioBattFaultCode.szByIfyFault, 0, CAN_DATA_AREA_LEN);
	//memcpy(s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat, 0, CAN_DATA_AREA_LEN);

	// 03|6C|55|E1|19|00|17|00|
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[0] = 0x03;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[1] = 0x6c;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[2] = 0x55;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[3] = 0xe1;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[4] = 0x19;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[5] = 0x00;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[6] = 0x17;
	s_infy_BattRunData[nBMSGrpMapIdx].unioBattDIDOStat.szByDIDOStat[7] = 0x00;

	s_infy_BattRunData[nBMSGrpMapIdx].fPackTotalVolt = 848.5;
	if(pSTBatt_Rough->branchGrpData[nBMSGrpMapIdx].emACDC_WorkStatu == STATU_ACDC_RECTIFY
		|| pSTBatt_Rough->branchGrpData[nBMSGrpMapIdx].emACDC_WorkStatu == STATU_ACDC_INVERTER)
	{
		s_infy_BattRunData[nBMSGrpMapIdx].fOutputCurrent = pSTBatt_Rough->branchGrpData[nBMSGrpMapIdx].fACDC_RealTime_Curr;
		s_infy_BattRunData[nBMSGrpMapIdx].fOutputPower = 0.001 * pSTBatt_Rough->branchGrpData[nBMSGrpMapIdx].fACDC_RealTime_Volt * pSTBatt_Rough->branchGrpData[nBMSGrpMapIdx].fACDC_RealTime_Curr;
	}
	else
	{
		s_infy_BattRunData[nBMSGrpMapIdx].fOutputCurrent = -0.0;
		s_infy_BattRunData[nBMSGrpMapIdx].fOutputPower = -0.0;
	}
	s_infy_BattRunData[nBMSGrpMapIdx].fOutsideVolt = 772.6;

	s_infy_BattRunData[nBMSGrpMapIdx].fSOC = 82.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fSOE = 82.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fSOH = 100;
	s_infy_BattRunData[nBMSGrpMapIdx].fTotalCapacity = 281.8;

	s_infy_BattRunData[nBMSGrpMapIdx].fCellAddTotalVolt = 800.4;
	s_infy_BattRunData[nBMSGrpMapIdx].usCellAverageVolt = 3335;
	s_infy_BattRunData[nBMSGrpMapIdx].sCellAverageTemp = 284;
	s_infy_BattRunData[nBMSGrpMapIdx].sBattCabinetTemp = 41;

	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Cell = 6;
	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellVlot_Volt = 3336;
	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Cell = 5;
	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellVlot_Volt = 3334;

	s_infy_BattRunData[nBMSGrpMapIdx].usMaxCellTemp_Cell = 49;
	s_infy_BattRunData[nBMSGrpMapIdx].sMaxCellTemp_Temp = 294;
	s_infy_BattRunData[nBMSGrpMapIdx].usMinCellTemp_Cell = 28;
	s_infy_BattRunData[nBMSGrpMapIdx].sMinCellTemp_Temp = 276;

	s_infy_BattRunData[nBMSGrpMapIdx].fNegativeBusVolt = 398.0;

	s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_P_R = 3000.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fInsulation_N_R = 1229.0;

	s_infy_BattRunData[nBMSGrpMapIdx].usSystemRunTime = 87;
	s_infy_BattRunData[nBMSGrpMapIdx].usCanChargeTime = 8;
	s_infy_BattRunData[nBMSGrpMapIdx].usCanDischargeTime = 43;
	s_infy_BattRunData[nBMSGrpMapIdx].usCycleTimes = 9;

	s_infy_BattRunData[nBMSGrpMapIdx].usMonAlarmNum = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].usMonFaultNum = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].usAccuAlarmNum = 22;
	s_infy_BattRunData[nBMSGrpMapIdx].usAccuFaultNum = 119;

//读取信息有误;
	s_infy_BattRunData[nBMSGrpMapIdx].fRemainEnergy = 4568;
	s_infy_BattRunData[nBMSGrpMapIdx].fTotalEnergy = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].fSingleChgCapacity = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].fSingleDischgCapacity = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgCapacity = 4568;
	s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgCapacity = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].fAccuChgEnergy = 1903.70;
	s_infy_BattRunData[nBMSGrpMapIdx].fAccuDischgEnergy = 1725.70;

	s_infy_BattRunData[nBMSGrpMapIdx].fNowDOD = 99.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fDischgStartDOD = 0.0;

	s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSLev = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].usDiagSOSCode = 0;

	// 单体均衡状态
	// 单体电压
	// 单体温度
	// 单体内阻
	// s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx++] 
	// s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx++] 
	// s_infy_BattRunData[nBMSGrpMapIdx].fCellRes[nCellResIdx] = 

	s_infy_BattRunData[nBMSGrpMapIdx].byBMS_Alive = 76;

	s_infy_BattRunData[nBMSGrpMapIdx].usCellVoltTotalNum = 240;
	s_infy_BattRunData[nBMSGrpMapIdx].usCellTempTotalNum =  240;

	s_infy_BattRunData[nBMSGrpMapIdx].fStandTotalVoltage = 792.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fStandCapacity = 285.0;
	s_infy_BattRunData[nBMSGrpMapIdx].fStandEnergy = 218.8;


	memset(s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion));
	memset(s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion));
	memset(s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion, '\0', sizeof(s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion));
	sprintf(s_infy_BattRunData[nBMSGrpMapIdx].bySystemVersion, "V%d.%d", 1, 2);
	sprintf(s_infy_BattRunData[nBMSGrpMapIdx].bySoftwareVersion, "V%d.%03d", 1, 2);
	sprintf(s_infy_BattRunData[nBMSGrpMapIdx].byHardwareVersion, "V%d.%d", 1, 2);

	s_infy_BattRunData[nBMSGrpMapIdx].emIsoResult = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].uiPermissionLevel = 1;

	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[0] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[0] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[0] = 1060;

	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[1] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[1] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[1] = 1060;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[2] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[2] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[2] = 1060;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[3] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[3] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[3] = 1060;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fPositiveTemp[4] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fNegativeTemp[4] = 30.0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uImmersionFreq[4] = 1060;


//读取信息有误;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[0] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[0] = 49;

	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[1] = 0;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[1] = 49;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[2] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[2] = 49;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[3] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[3] = 49;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.fHighVoltageValue[4] = 0;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uHighVoltageDuty[4] = 49;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[0] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[0] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[0] = 0;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[1] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[1] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[1] = 0;


	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[2] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[2] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[2] = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[3] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[3] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[3] = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uSol[4] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uH2[4] = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].stLiquidCoolingPack.uAerosolStatusAndFault[4] = 0;


	s_infy_BattRunData[nBMSGrpMapIdx].fLeakageCurrent = 0.63;
	s_infy_BattRunData[nBMSGrpMapIdx].nSoftVer = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo1 = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo2 = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo3 = 0;


	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo4 = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo5 = 0;
	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo6 = 0;

	s_infy_BattRunData[nBMSGrpMapIdx].nAlarmInfo7 = 0;
	//printf("YFY_MakeSomeFakeData --end\n");
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
void YFY_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	double 		tmNow = GetCurrentTime();
	static 		BOOL bFirstRun = TRUE;
	//VAR_VALUE 	varV;
	int 		n = 0;

	if (pSTBatt_Rough->emBattMFG == BATT_MFG_INFY_SELF_500K)
	{
		//---  YFY_Batt_Sample Running  ---//  
		if(YFY_JudgeIfNeedReconfig(pSTBatt_Rough))
		{
			YFY_ReadAndParseCanData(pSTBatt_Rough);
			//TRACE("Here Waitting Batt Reconfig\n"); 
		}
		
		for(n = 0; n < pSTBatt_Rough->nSystemCfgBattNum; n++)
		{
			if(s_infy_BattRunData[n].bIsCommFail == TRUE)
			{
				continue;
			}

		    if(fabs(tmNow - s_infy_BattRunData[n].t_LastSendC2S_CMD_HV) >= 1.0)
		    {
		        if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
		        {
		            YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_S_CMD_C2S_CMD_HV);
					YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_S_CMD_C2S_EMSSTATUS); 
		            s_infy_BattRunData[n].t_LastSendC2S_CMD_HV = tmNow;
		            Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
		        }
		    }
			
		    if(fabs(tmNow - s_infy_BattRunData[n].t_LastSendC2S_RTC) >= 120.0)
		    {
		        if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
		        {
		            YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_S_CMD_C2S_RTC_TIME); 
		            s_infy_BattRunData[n].t_LastSendC2S_RTC = tmNow;
		            Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
		        }
		    }

			if(fabs(tmNow - s_infy_BattRunData[n].t_LastSendC2S_RecNum) >= 10.0)
			{
				//printf("-- n=[%d] Cycle Send Total Rec Num t_LastSendC2S_RecNum=[%.1f]\n", n, s_infy_BattRunData[n].t_LastSendC2S_RecNum);
				if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_RD_CMD_C2S_REC_NUM); 
					
					s_infy_BattRunData[n].t_LastSendC2S_RecNum = tmNow;
					
					Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
				}
			}
			//WLD01206 EU3Air ON/OFF 备电柜需要控制空调模块液冷
			if(pSTBatt_Rough->emESSDevType == LIQUID_COOLING_BACKUP)
			{
				if(PCSAndMPPT_TempJudg(pSTBatt_Rough, n) == 1)  //高于设置温度返回1，开启模块液路
				{
					if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
					{
						YFY_PackC2S_EU3Air_CtrlCmd(pSTBatt_Rough, n, 1);
						Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
					}
				}
				else if(PCSAndMPPT_TempJudg(pSTBatt_Rough, n) == 2) //温度都小于设置温度，关闭液路
				{
					if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
					{
						YFY_PackC2S_EU3Air_CtrlCmd(pSTBatt_Rough, n, 0);
						Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
					}
				}

				if((pSTBatt_Rough->QF3_Enable == 1)&&((pSTBatt_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo.uDIStatus >> 17)&0x01) && (ABS(_NOW_ - pSTBatt_Rough->t_QF3_triger) > 6))
				{
					YFY_PackC2S_CMD_QF3(pSTBatt_Rough, n);
				}
				else if(((pSTBatt_Rough->PowerEU3_OtherEU3_Data[0].stIMEU3DataInfo.uDIStatus >> 17)&0x01) == 0)
				{
					pSTBatt_Rough->t_QF3_triger = _NOW_;
				}
				// TRACEX("bIsCalibrated = %d bOCVFlag = %d\n", pSTBatt_Rough->bIsCalibrated, pSTBatt_Rough->bOCVFlag);
				if(pSTBatt_Rough->bIsCalibrated == TRUE && pSTBatt_Rough->bOCVFlag == TRUE)              
				{
					printf("Send To BMS Calibrated SOC = [%d]\n", pSTBatt_Rough->nCalibratedSOC);
					YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_S_CMD_C2S_CalibratedSOC);
					pSTBatt_Rough->bIsCalibrated = FALSE;
				}						
			}
			//WLD250525 Query BSU soft version, if not exist, then query it.
			// if(strlen(pSTBatt_Rough->szBsuVersion) == 0)
			// {
			// 	if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
			// 	{
			// 		QueryBSUVersion(pSTBatt_Rough);
			// 		Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
			// 	}
			// }

			// XSY250417
			if(fabs(tmNow - s_infy_BattRunData[n].t_LastSendC2S_AfeSrTest) >= 120 && s_infy_BattRunData[n].bAfeWorkModeAbnormal)
			{
				if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_S_CMD_C2S_AFESRTEST);
					s_infy_BattRunData[n].t_LastSendC2S_AfeSrTest = tmNow;
					Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
				}
			}

			// LJH240715 aircond
			if (s_infy_BattRunData[n].bNeedGetTemp4Aircond &&
				(fabs(tmNow - s_infy_BattRunData[n].t_GetTemp4Aircond) > 1) && (fabs(tmNow - s_infy_BattRunData[n].t_GetTemp4Aircond) < 10.0))
			{
				if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
				{
					YFY_Send_C2S_Status(pSTBatt_Rough, n, CAN_RD_CMD_C2S_AIRCOND);
					Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
				}
				Sleep(500);
			}

			//20230908:Comm With YZY, Shild Errlog Function.
			/*if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
			{
				YFY_StateMachine4BMSMsg(pSTBatt_Rough, n);
				Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
			}*/

			if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
			{
				YFY_IsolationCtrlCmd4Bms(pSTBatt_Rough, n);
				Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
			}
			if((TESTMODE_CTRL_PM_MODE <= pSTBatt_Rough->emPMCtrlMode)
				&& (pSTBatt_Rough->emPMCtrlMode <= TESTMODE_ShortTime_CTRL_PM_MODE)) // 单簇先放/充、对拖、短时
			{
				// printf("S***************\n");
				// printf("n: %d, tmnow: %lf\n", n, tmNow);
				// printf("last send: %lf\n", s_infy_BattRunData[n].t_LastSendC2M_Res);
				
				float timetoSend = 1.0;
				if(fabs(tmNow - s_infy_BattRunData[n].t_LastSendC2M_Res) >= timetoSend)
				{
					int TestState = pSTBatt_Rough->bmsAutoTestResult[n].testState;
					// printf("n: %d, over %.1fs, teststate: %d.\n", n, timetoSend, TestState);
					// Send Heartbeat
					if (TestState != STATE_NULL)
					{	
						// printf("n: %d, send out heart beat!\n", n);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_HEARTBEAT, TestState);
					}
					// Send Test Result
					if (TestState == STATE_NULL)
					{	
						continue;
					}
					else
					{	
						// printf("n: %d, send out msg!\n", n);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_SELFCHECK_RES1, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_SELFCHECK_RES2, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_SELFCHECK_RES3, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_SELFCHECK_RES4, TestState);

						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_CHARGECHECK_RES1, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_CHARGECHECK_RES2, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_CHARGECHECK_RES3, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_DISCHARGECHECK_RES1, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_DISCHARGECHECK_RES2, TestState);
						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_DISCHARGECHECK_RES3, TestState);

						YFY_Send_C2M_Status(pSTBatt_Rough, n, CAN_S_CMD_C2M_TOTAL_RES, TestState);
					}
					
					s_infy_BattRunData[n].t_LastSendC2M_Res = tmNow;
		            Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
				}
				// else
				// {
				// 	printf("n: %d, do not reach %.1fs to send.\n", n, timetoSend);
				// }
				// printf("E***************\n");
			}
		}

		if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
		{	
			YFY_ReadAndParseCanData(pSTBatt_Rough);
			Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
		}
		
		//--- 20230205 John Added For Remote Upgrade ---//
		if(pSTBatt_Rough->stUpgradeParam.emUpgradeStatus != UPGRADE_IDLE)
		{
			GetBMSUpgradeStatus(pSTBatt_Rough);
			if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
			{
				HandleBatteryUpgrading(pSTBatt_Rough);			
			}
			Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
			
		}
	}
	else	//BATT_MFG_TEST_NO_BATT
	{
		pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt = pSTBatt_Rough->nSystemCfgBattNum;
	}

	YFY_RefreshBattCabinetAlarm(pSTBatt_Rough);

	if (pSTBatt_Rough->emBattMFG == BATT_MFG_TEST_NO_BATT)
	{
		YFY_MakeSomeFakeData(pSTBatt_Rough);
	}
	YFY_Map_BattRawData_2Rough(pSTBatt_Rough);

/*
	if (IMEU_TYPE_IMEU2 == pSTBatt_Rough->emIMEUType[0])
	{
		IMEU2CANRawData2Rough(pSTBatt_Rough->IMEU2CANRoughData, MAX_IMEU_NUM);
	}
	else
*/
	if (IMEU_TYPE_IMEU3 == pSTBatt_Rough->emIMEUType[0])
	{
		IMEU3ComStatus(pSTBatt_Rough->IMEU3CANRoughData, MAX_IMEU_NUM); //电池仓 

		IMEU3PowerComStatus(pSTBatt_Rough->PowerEU3_OtherEU3_Data, MAX_IMEU_NUM);  //电源仓
	}
	
	YFY_AirRawData_2Rough(pSTBatt_Rough);

	TongFei_AirRawData_2Rough(pSTBatt_Rough);   

	//edit by kuangyue 20250217
	IMEU3_AirRawData_2Rough(pSTBatt_Rough);

	XinMiao_AirRawData_2Rough(pSTBatt_Rough);

	YFY_PackRawData_2Rough(pSTBatt_Rough);

	
	/*
	if(pSTBatt_Rough->stUpgradeParam.emUpgradeStatus != UPGRADE_IDLE)
	{
		if(Mutex_Lock(pSTBatt_Rough->hSamplerLock, 100) == ERR_MUTEX_OK)
		{
			HandleBatteryUpgrading(pSTBatt_Rough);			
		}
		Mutex_Unlock(pSTBatt_Rough->hSamplerLock);
	}*/

	//-----   Temporary solution   -----//
	if(bFirstRun)
	{
		if(ABS(tmNow - pSTBatt_Rough->t_FirstBattBoot) > 30)//Wait BMS 30S,then Send Power on Cmd
		{
			bFirstRun = FALSE;
			for(n = 0; n < pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt; n++)
			{
				//s_infy_BattRunData[n].byPCS_Cmd = STBATT_V33_POWERON_CMD;
				TRACE("Waiting 20S ,then POWER ON !!!\n");
			}
		}
		return;
	}
	
	YFY_JudgeBMSAvaliableStatu(pSTBatt_Rough);

	YFY_RefreshLigHTState(pSTBatt_Rough);

	YFY_NormalRunJudgeBattCabinetStatu(pSTBatt_Rough);

	return;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  YFY_Batt_Control
*  CREATOR       :  
*  CREATED DATE  : 2021-12-23 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL YFY_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nBattIdx)
{
	BOOL bRtn = FALSE;
	TRACE("-----Now received cmd[%d] for YFY Batt[%d], bIscommFail = %d--------\n", emCmdTp, 
		s_infy_BattRunData[nBattIdx].bIsCommFail);
		
	if(s_infy_BattRunData[nBattIdx].bIsCommFail)
	{
		return FALSE; 
	}
  
	if(emCmdTp == CTRL_STBATT_WAKE_UP)
	{
		TRACEX("YFY BATTWAKE_UP -Power ON\n");
		s_infy_BattRunData[nBattIdx].byPCS_Cmd = STBATT_YFY_POWERON_CMD;
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_GO_SLEEP)
	{
		TRACE("YFY BATT_GOTO_SLEEP Power OFF\n");
		s_infy_BattRunData[nBattIdx].byPCS_Cmd = STBATT_YFY_POWEROFF_CMD;
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_REBOOT)
	{
		TRACE("YFY CTRL STBATT REBOOT\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_REBOOT); 
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_SUPER_USER)
	{
		TRACE("YFY CTRL STBATT SUPER USER\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_PERMISS_LEV_CTRL); 
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_CLEAR_FAULT)
	{
		TRACE("YFY CTRL STBATT CLEAR FAULT\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_CLEAR_FAULT); 
		bRtn = TRUE;
	} 
	else if(emCmdTp == CTRL_STBATT_WORK_MODE)
	{
		TRACE("YFY CTRL CTRL_STBATT_CMD_MODE\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_WORK_MODE); 
		bRtn = TRUE;
	}
	else if (emCmdTp >= CTRL_STBATT_SETAIRCOND_BATT_COOLON && emCmdTp<=CTRL_STBATT_SETAIRCOND_AMBIT_HEATOFF)
	{
		TRACED("YFY_Batt_Control CTRL_STBATT_SETAIRCOND_BATT_COOLON");
		s_infy_BattRunData[nBattIdx].bNeedGetTemp4Aircond = TRUE;
		s_infy_BattRunData[nBattIdx].t_GetTemp4Aircond    = GetCurrentTime();
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_AIRCOND_BATT_COOLON + emCmdTp - CTRL_STBATT_SETAIRCOND_BATT_COOLON); 
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_RINT_POINT)
	{
		TRACE("YFY CTRL CTRL_STBATT_RINT_POINT\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_RINTPOINT); 
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_EU3_AIRNOISE)
	{
		TRACE("YFY CTRL CTRL_STBATT_EU3_AIRNOISE\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_EU3AirNoise); 
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_KEEP_POWER_MODE)
	{
		TRACE("YFY CTRL CTRL_STBATT_KEEP_POWER_MODE\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_KEEP_POWER_MODE); 
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_AFE_RESET)
	{
		TRACE("YFY CTRL CTRL_STBATT_AFE_RESET\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_AFE_RESET);
		bRtn = TRUE;
	}
	else if(emCmdTp == CTRL_STBATT_BSU_RESET)
	{
		TRACE("YFY CTRL CTRL_STBATT_BSU_RESET\n");
		YFY_Send_C2S_Status(pSTBatt_Rough, nBattIdx, CAN_S_CMD_C2S_BSU_RESET);
		bRtn = TRUE;
	}




	return bRtn;
}




