#include "dev_485_2_sampler_main.h"





USHORT ModbusCRC(BYTE *pData, int len)
{
	static BYTE s_byCRCHi[] =
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

	static BYTE s_byCRCLo[] =
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
	BYTE i = (BYTE)len;
	BYTE  byCRCHi = 0xff;
	BYTE  byCRCLo = 0xff;
	BYTE  byIdx;
	USHORT crc;

	while(i--)
	{
		byIdx = byCRCHi ^ *pData++;
		byCRCHi = byCRCLo ^ s_byCRCHi[byIdx];
		byCRCLo = s_byCRCLo[byIdx];
	}

	crc = byCRCHi;
	crc <<= 8;
	crc += byCRCLo;

	return crc;
}


typedef struct
{
	HOPESUN_SIG_TYPE	emVarValName;
	int     			nRegAddr;
	int					nDataLen;
	int					nValGain;
	int					nDataType;		//0 - signed int, 1 - unsigned int, 2 - fault/alarm code
	
}REGISTER_ITEM;


static REGISTER_ITEM mpptSigMap[] = 
{
	{HOPESUN_MPPT1_INPUT_VOLT,  0,  1,  10,  0},
	{HOPESUN_MPPT2_INPUT_VOLT,  1,  1,  10,  0},
	{HOPESUN_MPPT3_INPUT_VOLT,  2,  1,  10,  0},
	{HOPESUN_MPPT4_INPUT_VOLT,  3,  1,  10,  0},
	{HOPESUN_STRING01_INPUT_CURRENT,  8,  1,  100,  0},
	{HOPESUN_STRING02_INPUT_CURRENT,  9,  1,  100,  0},
	{HOPESUN_STRING03_INPUT_CURRENT,  10,  1,  100,  0},
	{HOPESUN_STRING04_INPUT_CURRENT,  11,  1,  100,  0},
	{HOPESUN_STRING05_INPUT_CURRENT,  12,  1,  100,  0},
	{HOPESUN_STRING06_INPUT_CURRENT,  13,  1,  100,  0},
	{HOPESUN_STRING07_INPUT_CURRENT,  14,  1,  100,  0},
	{HOPESUN_STRING08_INPUT_CURRENT,  15,  1,  100,  0},
	{HOPESUN_STRING09_INPUT_CURRENT,  16,  1,  100,  0},
	{HOPESUN_STRING10_INPUT_CURRENT,  17,  1,  100,  0},
	{HOPESUN_STRING11_INPUT_CURRENT,  18,  1,  100,  0},
	{HOPESUN_STRING12_INPUT_CURRENT,  19,  1,  100,  0},
	{HOPESUN_STRING13_INPUT_CURRENT,  20,  1,  100,  0},
	{HOPESUN_STRING14_INPUT_CURRENT,  21,  1,  100,  0},
	{HOPESUN_STRING15_INPUT_CURRENT,  22,  1,  100,  0},
	{HOPESUN_STRING16_INPUT_CURRENT,  23,  1,  100,  0},
	{HOPESUN_STRING17_INPUT_CURRENT,  88,  1,  100,  0},
	{HOPESUN_STRING18_INPUT_CURRENT,  89,  1,  100,  0},
	{HOPESUN_STRING19_INPUT_CURRENT,  90,  1,  100,  0},
	{HOPESUN_STRING20_INPUT_CURRENT,  91,  1,  100,  0},
	{HOPESUN_MPPT1_INPUT_POWER,  24,  1,  100,  1},
	{HOPESUN_MPPT2_INPUT_POWER,  25,  1,  100,  1},
	{HOPESUN_MPPT3_INPUT_POWER,  26,  1,  100,  1},
	{HOPESUN_MPPT4_INPUT_POWER,  27,  1,  100,  1},
	{HOPESUN_MPPT5_INPUT_POWER,  28,  1,  100,  1},
	{HOPESUN_MPPT6_INPUT_POWER,  29,  1,  100,  1},
	{HOPESUN_MPPT7_INPUT_POWER,  30,  1,  100,  1},
	{HOPESUN_MPPT8_INPUT_POWER,  31,  1,  100,  1},
	{HOPESUN_GRID_AB_VOLTAGE,  32,  1,  10,  0},
	{HOPESUN_GRID_BC_VOLTAGE,  33,  1,  10,  0},
	{HOPESUN_GRID_CA_VOLTAGE,  34,  1,  10,  0},
	{HOPESUN_GRID_PHASE_A_CURRENT,  35,  1,  10,  0},
	{HOPESUN_GRID_PHASE_B_CURRENT,  36,  1,  10,  0},
	{HOPESUN_GRID_PHASE_C_CURRENT,  37,  1,  10,  0},
	{HOPESUN_GRID_FREQUENCY,  38,  1,  100,  0},
	{HOPESUN_AC_ACTIVE_POWER,  39,  1,  100,  0},
	{HOPESUN_AC_REACTIVE_POWER,  40,  1,  100,  0},
	{HOPESUN_DC_INPUT_POWER,  41,  1,  100,  0},
	{HOPESUN_INVERTER_EFFICIENCY,  42,  1,  100,  1},
	{HOPESUN_POWER_FACTOR,  43,  1,  1000,  0},
	{HOPESUN_INTERNAL_TEMPERATURE,  44,  1,  10,  0},
	{HOPESUN_INSULATION_IMPEDANCE,  45,  1,  10,  1},
	{HOPESUN_INVERTER_POWER_ON_OFF_STATUS,  46,  1,  1,  1},
	{HOPESUN_INVERTER_STATUS,  47,  1,  1,  1},
	{HOPESUN_DAILY_ENERGY,  48,  2,  100,  1},
	{HOPESUN_TOTAL_ENERGY,  50,  2,  100,  1},
	{HOPESUN_CO2_REDUCTION,  52,  2,  100,  1},
	{HOPESUN_DAILY_OPERATION_TIME,  54,  1,  100,  1},
	{HOPESUN_TOTAL_OPERATION_TIME,  55,  2,  100,  1},
	{HOPESUN_INVERTER_POWER_ON_TIME,  57,  2,  1,  1},
	{HOPESUN_INVERTER_POWER_OFF_TIME,  59,  2,  1,  1},
	{HOPESUN_MPPT_FAULT_CODE_1,  61,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_2,  62,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_3,  63,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_4,  64,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_5,  65,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_6,  66,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_7,  67,  1,  1,  2},
	{HOPESUN_MPPT_FAULT_CODE_8,  68,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_1,  69,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_2,  70,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_3,  71,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_4,  72,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_5,  73,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_6,  74,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_7,  75,  1,  1,  2},
	{HOPESUN_MPPT_ALARM_CODE_8,  76,  1,  1,  2},
};

void static Parse_Response_Data(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, BYTE *pRecvBuf, int nWhichMppt)
{
	BYTE *pbyDataPos = pRecvBuf + 3;
	
	VAR_VALUE varV; 
	int n;
	int m;
	int nPos = 0;
	UINT uVal;
	REGISTER_ITEM* pItem = mpptSigMap;
	int nItemTotal = ITEM_OF(mpptSigMap);
	for(n = 0; n < nItemTotal; n++, pItem++)
	{
		nPos = pItem->nRegAddr * 2;
		varV.ulValue = 0;
		for(m = 0; m < pItem->nDataLen; m++)
		{
			uVal = ((UINT)(*(pbyDataPos + nPos)) << 8) + (UINT)(*(pbyDataPos + nPos + 1));
			varV.ulValue += (uVal << (16 * (pItem->nDataLen - m - 1)));
			//printf("step 1: %ld\n", varV.ulValue);
			nPos += 2;
		}
		if(pItem->nDataType == 0)
		{
			if(varV.lValue >> 15 == 1)
			{
				// varV.lValue = varV.lValue << 1;
				// varV.lValue = varV.lValue >> 1;
				pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue = 0;
			}
			else
			{
				pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue = (float)varV.lValue * (1.0 / (pItem->nValGain));
			}
			// pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue = (float)varV.lValue * (1.0 / (pItem->nValGain));
			//printf("step 2: %f\n", pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue);
		}
		else if(pItem->nDataType == 1)
		{
			pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue = (float)varV.ulValue * (1.0 / (pItem->nValGain));
			//printf("step 3: %f\n", pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue);
		}
		else if(pItem->nDataType == 2)
		{
			pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].ulValue = varV.ulValue;
			//printf("step 3: %f\n", pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue);
		}
		else
		{
			printf("Unknown data type\n");
		}
		//printf("step 4: %f\n", pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[pItem->emVarValName].fValue);
	}
}


BOOL Modbus_Mppt_HopeSun_Sample(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, int nWhichMppt, COMMPORT* pComm)
{
	HANDLE hSamplerLock = ((SAMPLER*)pDev_485_2_Rough->hSampler)->hSamplerLock;
	time_t now_time;
	time(&now_time); 
	printf("\n组%d,时间1为%s\n", nWhichMppt + 1, ctime(&now_time));
	//pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress = (BYTE)(nWhichMppt + 1);
	pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress = (BYTE)(nWhichMppt);

	if(Mutex_Lock(hSamplerLock, 2000) != ERR_MUTEX_OK)
	{
		printf("Mutex lock failed when sample!\n");
		return FALSE;
	}
	
	//Now prepare modbus send buffer
	BYTE* pSendBuf = pDev_485_2_Rough->abySendBuf;
	*pSendBuf++ = pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress;
	*pSendBuf++ = 0x03; //function code
	int nExpectDataLen;

	*pSendBuf++ = 0x9E; //start-hibyte
	*pSendBuf++ = 0x34; //start-lowbyte
	//*pSendBuf++ = 0x01; //start-hibyte
	//*pSendBuf++ = 0xf4; //start-lowbyte
	*pSendBuf++ = 0x00; //lenth-hibyte
	*pSendBuf++ = 77; //length-lowbyte
	nExpectDataLen = 77*2;//目前只测到逆变器关机时间这个参数，所以只有61个寄存器   *2024.3.10添加故障告警字，故现在为77个

	USHORT usCRC = ModbusCRC(pDev_485_2_Rough->abySendBuf, 6);
	*pSendBuf++ = (BYTE)(usCRC >> 8);
	*pSendBuf++ = (BYTE)(usCRC & 0xFF);

	//now send out the data to meter
	int nWriteBytes = pComm->Write(pDev_485_2_Rough->hPort, pDev_485_2_Rough->abySendBuf, 8);
	if(nWriteBytes < 8)
	{
		printf("%d\n",1111);
		return FALSE;
	}
	else
	{
		int k;
		printf("Modbus-RTU sample write:");
		for(k = 0; k < 8; k++)
		{
			printf("%02X-", pDev_485_2_Rough->abySendBuf[k]);
		}
		printf("\n");
		TRACE("\n");
	}
	time(&now_time); 
	printf("\n组%d,时间2为%s\n", nWhichMppt + 1, ctime(&now_time));

	//now read response frame from meter
	BYTE *pRecvBuf = pDev_485_2_Rough->abyRcvBuf;
	//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
	int n;
	int nReadBytes;
	time(&now_time); 
	printf("\n组%d,时间3为%s\n", nWhichMppt + 1, ctime(&now_time));

	//read back for 3 times
	for(n = 0; n < 3; n++)
	{
		nReadBytes = pComm->Read(pDev_485_2_Rough->hPort, pRecvBuf, 256);
		if(nReadBytes >= 7)
		{
			break;
		}
	}
	Mutex_Unlock(hSamplerLock);
	time(&now_time); 
	printf("\n组%d,时间4为%s\n", nWhichMppt + 1, ctime(&now_time));

	//check length
	if(nReadBytes < 7)
	{
		TRACEX("MOdbus read bytes[%d]\n", nReadBytes);
		printf("%d\n",2222);
		printf("Modbus-RTU sample read(less)%d:", nReadBytes);
		int k;
		for(k = 0; k < nReadBytes; k++)
		{
			printf("%02X-", pRecvBuf[k]);
		}
		printf("\n");
		return FALSE;
	}
	else
	{
		//TRACEX("Modbus-RTU YD read%d:", nReadBytes);
		printf("Modbus-RTU sample read%d:", nReadBytes);
		int k;
		for(k = 0; k < nReadBytes; k++)
		{
			printf("%02X-", pRecvBuf[k]);
		}
		printf("\n");
		TRACE("\n");
	}

	// Check Address
	if(*pRecvBuf != pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress)
	{
		TRACEX("Modbus-RTU YD Err111");
		printf("%d\n",3333);
		return FALSE;
	}

	// Check Function Code
	if(*(pRecvBuf + 1) != 0x03)
	{
		TRACEX("Modbus-RTU YD Err222");
		printf("%d\n",4444);
		return FALSE;
	}

	// Check Data Length
	int byDataLen = (int)((BYTE)(*(pRecvBuf + 2)));

	if(byDataLen != nExpectDataLen)
	{
		TRACEX("Modbus-RTU YD Err333");
		printf("%d, real len: %d, expect len: %d\n",5555, byDataLen, nExpectDataLen);
		return FALSE;
	}

	// Check CRC
	int nCRC_StartCal = 3 + (int)byDataLen;
	int nCRC_2CalcLen = nCRC_StartCal;
	USHORT usReadCRC = (((USHORT)(*(pRecvBuf + nCRC_StartCal))) << 8) + (USHORT)(*(pRecvBuf + nCRC_StartCal + 1));
	USHORT usCalcCRC = ModbusCRC(pRecvBuf, nCRC_2CalcLen);
	if(usReadCRC != usCalcCRC)
	{
		TRACE("CRC Error! Calc CRC = %04X\n", usCalcCRC);
		printf("%d\n",6666);
		return FALSE;
	}
	time(&now_time); 
	printf("\n组%d,时间5为%s\n", nWhichMppt + 1, ctime(&now_time));

	//now parse data
	Parse_Response_Data(pDev_485_2_Rough, pRecvBuf, nWhichMppt);
	time(&now_time); 
	printf("\n组%d,时间6为%s\n", nWhichMppt + 1, ctime(&now_time));

	//send data into the blob structure (in the order of the blob structure)
	A_BLOB_HOPESUN_MPPT_DATA *pMpptData = &pDev_485_2_Rough->mpptData[nWhichMppt].blobMpptData;

	pMpptData->emBlobType = 137;
	for(n = 0; n < 4; n++)
	{
		pMpptData->fMppt_InputVolt[n] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[n].fValue;
	}
	for(n = 0; n < 20; n++)
	{
		pMpptData->fString_InputCurr[n] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[4 + n].fValue;
	}
	for(n = 0; n < 8; n++)
	{
		pMpptData->fMppt_InputPower[n] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[24 + n].fValue;
	}

	pMpptData->fGrid_AC_Volt = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[32].fValue;
	pMpptData->fGrid_BC_Volt = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[33].fValue;
	pMpptData->fGrid_CA_Volt = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[34].fValue;

	pMpptData->fGrid_phaA_Curr = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[35].fValue;
	pMpptData->fGrid_phaB_Curr = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[36].fValue;
	pMpptData->fGrid_phaC_Curr = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[37].fValue;

	pMpptData->fGrid_Frequency = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[38].fValue;
	pMpptData->fAC_ActivePower = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[39].fValue;
	pMpptData->fAC_ReactivePower = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[40].fValue;
	pMpptData->fDC_InputPower = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[41].fValue;
	pMpptData->fInverter_efficiency = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[42].fValue;
	pMpptData->fPower_Factor = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[43].fValue;
	pMpptData->fInternal_Temp = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[44].fValue;
	pMpptData->fInsulation_Impedance = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[45].fValue;

	pMpptData->fInverter_Power_OnOff_Status = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[46].fValue;
	pMpptData->fInverter_Status = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[47].fValue;
	pMpptData->fDaily_Energy = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[48].fValue;
	pMpptData->fTotal_Energy = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[49].fValue;
	pMpptData->fCO2_Reduction = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[50].fValue;
	pMpptData->fDaily_Operation_Time = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[51].fValue;
	pMpptData->fTotal_Operation_Time = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[52].fValue;
	pMpptData->fInverter_Power_On_Time = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[53].fValue;
	pMpptData->fInverter_Power_Off_Time = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[54].fValue;

	pMpptData->unioMpptFault.szByMPPTFault[0] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[55].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[1] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[55].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[2] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[56].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[3] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[56].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[4] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[57].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[5] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[57].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[6] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[58].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[7] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[58].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[8] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[59].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[9] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[59].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[10] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[60].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[11] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[60].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[12] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[61].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[13] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[61].ulValue & 0XFF;
	pMpptData->unioMpptFault.szByMPPTFault[14] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[62].ulValue >> 8;
	pMpptData->unioMpptFault.szByMPPTFault[15] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[62].ulValue & 0XFF;

    pMpptData->unioMpptAlarm.szByMPPTAlarm[0] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[63].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[1] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[63].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[2] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[64].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[3] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[64].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[4] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[65].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[5] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[65].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[6] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[66].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[7] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[66].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[8] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[67].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[9] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[67].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[10] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[68].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[11] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[68].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[12] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[69].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[13] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[69].ulValue & 0XFF;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[14] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[70].ulValue >> 8;
	pMpptData->unioMpptAlarm.szByMPPTAlarm[15] = pDev_485_2_Rough->mpptData[nWhichMppt].varMpptRough[70].ulValue & 0XFF;



	printf("push前读到的mpptData电压1为%f\n", pDev_485_2_Rough->mpptData[nWhichMppt].blobMpptData.fMppt_InputVolt[0]);
	
	//push data
	Push_BlobValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 2000, (void*)&pDev_485_2_Rough->mpptData[nWhichMppt].blobMpptData, sizeof(A_BLOB_HOPESUN_MPPT_DATA), BLOB_TYPE_HOPESUN_MPPT_DATA);
	
	Push_Fault_Code(pDev_485_2_Rough, nWhichMppt, pMpptData->unioMpptFault);
	Push_Alarm_Code(pDev_485_2_Rough, nWhichMppt, pMpptData->unioMpptAlarm);

	return TRUE;
}

void Push_Fault_Code(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, int nWhichMppt, MPPT_FAULT_CODE unioMpptFault)
{
	VAR_VALUE 	varV;
	int n;
	//逆变器故障字1
	for(n = 0;n < 4;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[0] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1602 + n, varV);
	}
	varV.ulValue = unioMpptFault.Fault.InternalCommFault;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1606, varV);
	
	//逆变器故障字2
	for(n = 0;n < 8;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[2] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1607 + n, varV);
	}
	
	//逆变器故障字3
	for(n = 0;n < 6;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[4] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1615 + n, varV);
	}
	varV.ulValue = unioMpptFault.Fault.GridOverFreq;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1621, varV);
	varV.ulValue = unioMpptFault.Fault.GridUnderFreq;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1622, varV);
	varV.ulValue = unioMpptFault.Fault.IslandingProtection;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1623, varV);
	varV.ulValue = unioMpptFault.Fault.OutputGroundVoltAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1624, varV);
	varV.ulValue = unioMpptFault.Fault.LowVoltRideThroughProtection;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1625, varV);

	//逆变器故障字4
	for(n = 0;n < 8;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[6] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1626 + n, varV);
	}
	for(n = 8;n < 11;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[7] >> (n - 1)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1626 + n, varV);
	}
	varV.ulValue = unioMpptFault.Fault.InverterConversionEfficiencyLow;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1637, varV);
	varV.ulValue = unioMpptFault.Fault.ResidualCurrSuddenChangeExceeding;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1638, varV);

	//逆变器故障字5
	for(n = 0;n < 2;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[8] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1639 + n, varV);
	}
	for(n = 0;n < 3;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[8] >> (2 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1641 + n, varV);
	}
	
	//逆变器故障字6
	for(n = 0;n < 2;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[10] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1644 + n, varV);
	}

	//逆变器故障字7
	varV.ulValue = unioMpptFault.Fault.AuxiliaryPowerFault;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1646, varV);
	for(n = 0;n < 6;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[12] >> (5 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1647 + n, varV);
	}

	//逆变器故障字8
	for(n = 0;n < 4;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[14] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1653 + n, varV);
	}
	varV.ulValue = unioMpptFault.Fault.BusSoftwareOverVolt;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1657, varV);
	for(n = 0;n < 8;n++)
	{
		varV.ulValue = (unioMpptFault.szByMPPTFault[15] >> (7 - n)) & 1;
		Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler, 
			802 + nWhichMppt, 1658 + n, varV);
	}
	
}


void Push_Alarm_Code(RS485_2_SAMPLER_DATA* pDev_485_2_Rough, int nWhichMppt, MPPT_ALARM_CODE unioMpptAlarm)
{
	VAR_VALUE 	varV;
	int n;
	
	//逆变器故障字3
	varV.ulValue = unioMpptAlarm.Alarm.GridAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1666, varV);
	varV.ulValue = unioMpptAlarm.Alarm.GridVoltImbalanceExceeded;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1667, varV);
	varV.ulValue = unioMpptAlarm.Alarm.GridPhaOrderReversal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1668, varV);
	varV.ulValue = unioMpptAlarm.Alarm.GridVoltAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1669, varV);

	//逆变器故障字4
	varV.ulValue = unioMpptAlarm.Alarm.HighInverterConversionEfficiency;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1670, varV);

	//逆变器故障字5
	varV.ulValue = unioMpptAlarm.Alarm.DCVoltDetectionAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1671, varV);
	varV.ulValue = unioMpptAlarm.Alarm.HighDCInputVolt;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1672, varV);
	
	//逆变器故障字6
	varV.ulValue = unioMpptAlarm.Alarm.AC_SPDAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1673, varV);
	varV.ulValue = unioMpptAlarm.Alarm.InternalFanAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1674, varV);
	varV.ulValue = unioMpptAlarm.Alarm.ExternalFanAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1675, varV);

	//逆变器故障字7
	varV.ulValue = unioMpptAlarm.Alarm.DC_SPDAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1676, varV);
	varV.ulValue = unioMpptAlarm.Alarm.BoostCircuitOpenCircuit;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1677, varV);
	varV.ulValue = unioMpptAlarm.Alarm.StringExceptionAlarm;
	Push_VarValue(SETTYPE_RAW_SP, pDev_485_2_Rough->hSampler,
		802 + nWhichMppt, 1678, varV);
	
}


BYTE* Get_Send_Buffer_Register_Address(int nParamId, VAR_VALUE *pVarVal)
{
	BYTE static res[2];
	int addr;
	if(nParamId == 4502)
	{
		if(pVarVal->ulValue == 0){addr = 40200;}
		else{addr = 40201;}
	}
	else if(nParamId == 4503){addr = 40202;}
	else if(nParamId == 4504){addr = 40203;}
	else if(nParamId == 4505){addr = 40002;}
	else if(nParamId == 4506){addr = 40011;}
	else if(nParamId == 4507){addr = 40003;}
	else if(nParamId == 4508){addr = 40004;}
	else if(nParamId == 4509){addr = 40005;}
	else if(nParamId == 4510){addr = 40012;}
	else if(nParamId == 4511){addr = 40013;}
	else if(nParamId == 4514){addr = 40002;}
	else if(nParamId == 4515){addr = 40011;}
	else if(nParamId == 4516){addr = 40003;}
	else if(nParamId == 4517){addr = 40004;}
	else if(nParamId == 4518){addr = 40005;}
	else if(nParamId == 4519){addr = 40012;}
	else if(nParamId == 4520){addr = 40013;}
	else
	{
		TRACE("ParamId doesn't exist!");
	}

	res[1] = addr & 0xff;
	res[0] = (addr & 0xff00) >> 8;
	return res;
}


BYTE* Get_Send_Buffer_Data(int nParamId, VAR_VALUE *pVarVal, int nTotalNum)
{
	BYTE static res[2];
	int data = 0;
	if(nParamId == 4502){data = 1;}
	else if(nParamId == 4503){data = pVarVal->ulValue;}
	else if(nParamId == 4504){data = pVarVal->ulValue;}
	else if(nParamId == 4505){data = pVarVal->ulValue;}
	else if(nParamId == 4506){data = pVarVal->ulValue;}
	else if(nParamId == 4507){data = (int)(pVarVal->fValue * 10000);}
	else if(nParamId == 4508){data = (int)(pVarVal->fValue * 10);}
	else if(nParamId == 4509){data = (int)(pVarVal->fValue * 100);}
	else if(nParamId == 4510){data = (int)(pVarVal->fValue * 100);}
	else if(nParamId == 4511){data = (int)(pVarVal->fValue * 100);}
	else if(nParamId == 4514){data = pVarVal->ulValue;}
	else if(nParamId == 4515){data = pVarVal->ulValue;}
	else if(nParamId == 4516){data = (int)(pVarVal->fValue * 10000);}
	else if(nParamId == 4517){data = (int)(pVarVal->fValue * 10 / nTotalNum);}
	else if(nParamId == 4518){data = (int)(pVarVal->fValue * 100);}
	else if(nParamId == 4519){data = (int)(pVarVal->fValue * 100 / nTotalNum);}
	else if(nParamId == 4520){data = (int)(pVarVal->fValue * 100);}
	else
	{
		TRACE("ParamId doesn't exist!");
	}

	printf("send buffer data is %d\n", data);
	res[1] = data & 0xff;
	res[0] = (data & 0xff00) >> 8;

	if(data >> 31 == 1)
	{
		res[0] *= -1;
	}

	return res;
}


int Modbus_Mppt_HopeSun_SendCtrl_Single(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nPVInverterId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut)
{
	HANDLE hSamplerLock = ((SAMPLER*)pDev_485_2_Rough->hSampler)->hSamplerLock;
	if(emParaType == PARAM_TYPE_SETTING)
	{
		int nWhichMppt = nPVInverterId - 802;
		//pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress = (BYTE)(nWhichMppt + 1);
		pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress = (BYTE)(nWhichMppt);

		COMMPORT* pComm = ((SAMPLER*)pDev_485_2_Rough->hSampler)->pUsedCommPort;
		//Clear buffer
		pComm->Control(pDev_485_2_Rough->hPort, COMM_PURGE_RXCLEAR, NULL, 0);
		char szSett[] = "9600, N, 8, 1";

		pComm->Control(pDev_485_2_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)szSett,
			strlen(szSett));

		Sleep(50);

		if(Mutex_Lock(hSamplerLock, 7000) != ERR_MUTEX_OK)
		{
			printf("Mutex lock failed when single ctrl!\n");
			return ERR_CONTROL_TIMEOUT;
		}
		
		//Now prepare modbus send buffer
		BYTE* pSendBuf = pDev_485_2_Rough->abySendBuf;
		*pSendBuf++ = pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress;
		*pSendBuf++ = 0x06; //function code

		BYTE szSendBuffAddr[2];
		BYTE szSendBuffData[2];
		memcpy(szSendBuffAddr, Get_Send_Buffer_Register_Address(nParamId, pVarVal), 2);
		memcpy(szSendBuffData, Get_Send_Buffer_Data(nParamId, pVarVal, 1), 2);

		*pSendBuf++ = szSendBuffAddr[0]; //address-hibyte
		*pSendBuf++ = szSendBuffAddr[1]; //address-lowbyte
		*pSendBuf++ = szSendBuffData[0]; //data-hibyte
		*pSendBuf++ = szSendBuffData[1]; //data-lowbyte

		USHORT usCRC = ModbusCRC(pDev_485_2_Rough->abySendBuf, 6);
		*pSendBuf++ = (BYTE)(usCRC >> 8);
		*pSendBuf++ = (BYTE)(usCRC & 0xFF);

		//now send out the data to meter
		int nWriteBytes = pComm->Write(pDev_485_2_Rough->hPort, pDev_485_2_Rough->abySendBuf, 8);
		if(nWriteBytes < 8)
		{
			printf("\n%d\n",91111);
			return ERR_CONTROL_FAILED;
		}
		else
		{
			int k;
			printf("Modbus-RTU single ctrl write:");
			for(k = 0; k < 8; k++)
			{
				printf("%02X-", pDev_485_2_Rough->abySendBuf[k]);
			}
			printf("\n");
			TRACE("\n");
		}

		//Sleep(100); 
		//now read response frame from meter
		BYTE *pRecvBuf = pDev_485_2_Rough->abyRcvBuf;
		//memset(pRecvBuf, 0, RS485_RECV_BUF_LEN);
		int n;
		int nReadBytes;
		//read back for 3 times
		for(n = 0; n < 3; n++)
		{
			nReadBytes = pComm->Read(pDev_485_2_Rough->hPort, pRecvBuf, 256);
			if(nReadBytes == nWriteBytes)
			{
				break;
			}
		}
		Mutex_Unlock(hSamplerLock);
		
		//check length
		if(nReadBytes < 8)
		{
			TRACEX("MOdbus read bytes[%d]\n", nReadBytes);
			printf("\n%d,%d\n",92222, nReadBytes);
			return ERR_CONTROL_COMFAIL;
		}
		else
		{
			TRACEX("Modbus-RTU single ctrl read%d:", nReadBytes);
			printf("Modbus-RTU single ctrl read%d:", nReadBytes);
			int k;
			for(k = 0; k < nReadBytes; k++)
			{
				printf("%02X-", pRecvBuf[k]);
			}
			TRACE("\n");
			printf("\n");
		}

		// Check Slave Address
		if(*pRecvBuf != pDev_485_2_Rough->mpptData[nWhichMppt].byModbusAddress)
		{
			TRACEX("Modbus-RTU YD Err111");
			printf("\n%d,%X\n",93333, *pRecvBuf);
			return ERR_CONTROL_FAILED;
		}
		// Check Function Code
		if(*(pRecvBuf + 1) != 0x06)
		{
			TRACEX("Modbus-RTU YD Err222");
			printf("\n%d\n",94444);
			return ERR_CONTROL_FAILED;
		}

		// Check Register Address
		if(*(pRecvBuf + 2) != szSendBuffAddr[0] || *(pRecvBuf + 3) != szSendBuffAddr[1])
		{
			TRACEX("Modbus-RTU YD Err222");
			printf("\n%d\n",95555);
			return ERR_CONTROL_FAILED;
		}

		// Check Data
		if(*(pRecvBuf + 4) != szSendBuffData[0] || *(pRecvBuf + 5) != szSendBuffData[1])
		{
			TRACEX("Modbus-RTU YD Err222");
			printf("\n%d\n",96666);
			return ERR_CONTROL_FAILED;
		}

		// Check CRC
		if(*(pRecvBuf + 6) != (BYTE)(usCRC >> 8) || *(pRecvBuf + 7) != (BYTE)(usCRC & 0xFF))
		{
			TRACEX("Modbus-RTU YD Err222");
			printf("\n%d\n",97777);
			return ERR_CONTROL_FAILED;
		}
	}
	else
	{
		TRACEX("Not Setting Param\n");
		return ERR_CONTROL_NOTSUPPORTED;
	}
	return ERR_CONTROL_OK;
}


int Modbus_Mppt_HopeSun_SendCtrl_Total(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, 
	PARAM_TYPE	emParaType,
	int 		nDevId, 
	int 		nParamId, 
	VAR_VALUE 	*pVarVal, 
	int 		nValSize,
	int			nTimeOut,
	int			nTotalNum)  //Total PV Inverter number
{
	HANDLE hSamplerLock = ((SAMPLER*)pDev_485_2_Rough->hSampler)->hSamplerLock;
	if(emParaType == PARAM_TYPE_SETTING)
	{
		COMMPORT* pComm = ((SAMPLER*)pDev_485_2_Rough->hSampler)->pUsedCommPort;
		//Clear buffer
		pComm->Control(pDev_485_2_Rough->hPort, COMM_PURGE_RXCLEAR, NULL, 0);
		char szSett[] = "9600, N, 8, 1";

		pComm->Control(pDev_485_2_Rough->hPort, COMM_SET_SERIAL_ATTRIB, (void*)szSett,
			strlen(szSett));

		Sleep(50);

		if(Mutex_Lock(hSamplerLock, 7000) != ERR_MUTEX_OK)
		{
			printf("Mutex lock failed when total ctrl!\n");
			Mutex_Unlock(hSamplerLock);
			return ERR_CONTROL_TIMEOUT;
		}

		//Now prepare modbus send buffer
		BYTE* pSendBuf = pDev_485_2_Rough->abySendBuf;
		*pSendBuf++ = 0x00;
		*pSendBuf++ = 0x06; //function code

		BYTE szSendBuffAddr[2];
		BYTE szSendBuffData[2];
		memcpy(szSendBuffAddr, Get_Send_Buffer_Register_Address(nParamId, pVarVal), 2);
		memcpy(szSendBuffData, Get_Send_Buffer_Data(nParamId, pVarVal, nTotalNum), 2);

		*pSendBuf++ = szSendBuffAddr[0]; //address-hibyte
		*pSendBuf++ = szSendBuffAddr[1]; //address-lowbyte
		*pSendBuf++ = szSendBuffData[0]; //data-hibyte
		*pSendBuf++ = szSendBuffData[1]; //data-lowbyte

		USHORT usCRC = ModbusCRC(pDev_485_2_Rough->abySendBuf, 6);
		*pSendBuf++ = (BYTE)(usCRC >> 8);
		*pSendBuf++ = (BYTE)(usCRC & 0xFF);

		//now send out the data to meter
		int nWriteBytes = pComm->Write(pDev_485_2_Rough->hPort, pDev_485_2_Rough->abySendBuf, 8);
		if(nWriteBytes < 8)
		{
			printf("\n%d\n",91111);
			return ERR_CONTROL_FAILED;
		}
		else
		{
			int k;
			TRACEX("Modbus-RTU total ctrl write:");
			printf("Modbus-RTU total ctrl write:");
			for(k = 0; k < 8; k++)
			{
				printf("%02X-", pDev_485_2_Rough->abySendBuf[k]);
			}
			TRACE("\n");
			printf("\n");
		}
		Sleep(100); 
		Mutex_Unlock(hSamplerLock);
		printf("\nTotal ctrl message sent out successfully!\n");
	}
	return ERR_CONTROL_OK;
}