//#define _TRACE_PRINT_
#include "uart_server_main.h"

static COMMPORT* s_PMdComm = NULL;
static HANDLE 	 s_hServerPort = NULL;
 
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

int Modbus_MakeReqeustData(IN BYTE nAddr, IN int nFunCode, IN WORD wStartAddr, 
                            IN WORD wDataNum, OUT BYTE* pBuf)
{

	memset(pBuf, 0, MDBRTU_SEND_BUF_LEN);

	BYTE *pSendBuf = pBuf;
	
	*pSendBuf++ = nAddr;
	*pSendBuf++ = nFunCode;

	*pSendBuf++ = HIBYTE(wStartAddr);
	*pSendBuf++ = LOBYTE(wStartAddr);

	*pSendBuf++ = HIBYTE(wDataNum);
	*pSendBuf++ = LOBYTE(wDataNum);

	WORD wCRC = MDB_CRC(pBuf, 6);
	*pSendBuf++ = HIBYTE(wCRC);
	*pSendBuf++ = LOBYTE(wCRC);

	return 0;
}

void Modbus_ParseTempHumiData(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough, IN WORD wStartAddr, BYTE* pData, int nDataNum)
{
    WORD wAddr = wStartAddr;
    int i;
    TempAndHumiRead* pTempAndHumiRead = &(pSTServer_Rough->stTempAndHumiRead);
    for (i = 0; i < nDataNum; i++)
    {
        int j = wAddr + i;   //2000
		TRACEI("j=%x",j);
        switch (j)
        {
			case 0x0002:    //HY-BWD3K130C
			{
				if (pSTServer_Rough->stTempAndHumiRead.bInstallType == TEMPHUMIL_TYPE_HY_BWD3K130C) 
				{
					pTempAndHumiRead->fTemperature = (float)MAKEWORD(pData[1], pData[0])/ 10.0;
					TRACEI("HY-BWD3K130C Temp = %f",pTempAndHumiRead->fTemperature);
					//pData += 2;
				}

				break;
			}
			case 0x2000: //OMRON_E5CC
			{
				if (pSTServer_Rough->stTempAndHumiRead.bInstallType == TEMPHUMIL_TYPE_OMRON_E5CC) 
				{
					pTempAndHumiRead->fTemperature = (float)MAKEWORD(pData[1], pData[0]);
					TRACEI("OMRON_E5CC Temp = %f",pTempAndHumiRead->fTemperature);
					//pData += 2;
				}
				break;
			}
			// case 1:
			// {
			//     pTempAndHumiRead->fHumidity = (float)MAKEWORD(pData[1], pData[0]) / 10.0;
			//     pData += 2;
			//     break;
			// }
			default:
				break;
        }
    }
}

int Modbus_JudgeResponse(IN ST_UART_SERVER_ROUGHDATA *pSTServer_Rough, IN BYTE* pBufHead, 
                            IN int nLen, IN int nFunCode, IN BYTE nAddr, IN WORD wStartAddr, IN int nDataNum)
{
	BYTE* pBuf = pBufHead;

    // int j = 0;
	// printf("modbus recv frame : ");
	// for(j = 0; j < nLen; j++)
	// {
	// 	printf("%02X ",*(pBuf + j));
	// }
	// printf("\n");

    // Check Length
    if (nLen < 7)
    {
		TRACEE("Error: nLen=[%d]\n", nLen);
        return MDB_ERROR_RESPOND_DATA_LEN;
    }

	//BYTE byResp[9] = {0x01, 0x03, 0x04, 0x1A, 0x00, 0x4B, 0x00, 0x87, 0xAB};
    BYTE byAddr = *pBuf++; 

    // Check Address
    if(byAddr != nAddr)
    {
		TRACEE("Error: byAddr=[%02x], nAddr = [%02x]\n", byAddr, nAddr);
        return MDB_ERROR_ADDR;
    }
    BYTE byFunCode = *pBuf++;

    // Check Function Code
    if(nFunCode != byFunCode)
    {
		TRACEE("Error: nFunCode = [%d] byFunCode=[%d]\n", nFunCode, byFunCode);
        return MDB_ERROR_FUN_CODE;
    }
    BYTE nDataLen = *pBuf++;

    // Check Data Length
    if (nLen < nDataLen + 5)
    {
		TRACEE("Error: nLen=[%d] nDataLen=[%d]\n", nLen, nDataLen);
	    return MDB_ERROR_RESPOND_DATA_LEN;
    }

    // Check CRC
	//low bits in the front and high bits in the back
    WORD wReadCRC = MAKEWORD(*(pBuf + (int)nDataLen + 1), *(pBuf + (int)nDataLen));
    WORD wCalcCRC = MDB_CRC(pBufHead, nDataLen + 3);

	TRACEW("wReadCRC = %04X, wCalcCRC = %04X", wReadCRC, wCalcCRC);

    if(wReadCRC != wCalcCRC)
    {
	    TRACEE("Error: CRC Error! Calc CRC = %04X\n", wCalcCRC);
	    return MDB_ERROR_CRC;
    }

    if(nDataLen !=(nDataNum * 2))
    {
		TRACE("Error: byDataLen=[%d] wDataSize * 2=[%d]\n", nDataLen, nDataNum * 2);
	    return MDB_ERROR_RESPOND_DATA_LEN;
    }

    return MDB_ERROR_OK;
}

//edit by kuangyue 20251029
int Modbus_JudgeResponse_WriteMulReg(IN ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,
                         IN BYTE* pBufHead, IN int nLen,
                         IN int nFunCode, IN BYTE nAddr,
                         IN WORD wStartAddr, IN int nDataNum)
{
    BYTE* pBuf = pBufHead;

    if (nLen < 5) 
	{
        TRACEE("Error: nLen=[%d]\n", nLen);
        return MDB_ERROR_RESPOND_DATA_LEN;
    }

    BYTE byAddr = *pBuf++;
    if (byAddr != nAddr) 
	{
        TRACEE("Error: byAddr=[%02x], nAddr=[%02x]\n", byAddr, nAddr);
        return MDB_ERROR_ADDR;
    }

    BYTE byFunCode = *pBuf++;
    if (byFunCode != nFunCode) 
	{
        TRACEE("Error: nFunCode=[%d], byFunCode=[%d]\n", nFunCode, byFunCode);
        return MDB_ERROR_FUN_CODE;
    }

    WORD wReadCRC, wCalcCRC;

    if (nFunCode == 0x03) 
	{
        BYTE nDataLen = *pBuf++;
        if (nLen < nDataLen + 5) 
		{
            TRACEE("Error: nLen=[%d], nDataLen=[%d]\n", nLen, nDataLen);
            return MDB_ERROR_RESPOND_DATA_LEN;
        }

        wReadCRC = MAKEWORD(*(pBuf + nDataLen + 1), *(pBuf + nDataLen));
        wCalcCRC = MDB_CRC(pBufHead, nDataLen + 3);

    } 
	else if (nFunCode == 0x10) 
	{  
        if (nLen < 8) 
		{
            TRACEE("Error: nLen=[%d], expect 8\n", nLen);
            return MDB_ERROR_RESPOND_DATA_LEN;
        }

        wReadCRC = MAKEWORD(*(pBufHead + 7), *(pBufHead + 6));
        wCalcCRC = MDB_CRC(pBufHead, 6);

    } 
	else 
	{
        TRACEE("Error: Unsupported function code [%02X]\n", nFunCode);
        return MDB_ERROR_FUN_CODE;
    }

    TRACEW("CRC Check [Fun=0x%02X]: wReadCRC=%04X, wCalcCRC=%04X",
           nFunCode, wReadCRC, wCalcCRC);

    if (wReadCRC != wCalcCRC) 
	{
        TRACEE("Error: CRC mismatch! Calc CRC=%04X\n", wCalcCRC);
        return MDB_ERROR_CRC;
    }

    return MDB_ERROR_OK;
}
int Init_TempHumiData(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSTServer_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	TRACEI("Init_TempHumiData\n");
	memset(&(pSTServer_Rough->stTempAndHumiRead), 0, sizeof(TempAndHumiRead));

	VAR_VALUE varV;
	// install type
	pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_ST_VAR, 911, 915, &varV, 4);
	pSTServer_Rough->stTempAndHumiRead.bInstallType= varV.emValue;

	return 0;
}

void PushModbusValue(IN ST_UART_SERVER_ROUGHDATA *pSTServer_Rough)
{
	VAR_VALUE varV;
	varV.fValue = pSTServer_Rough->stTempAndHumiRead.fTemperature;
	TRACEI("-----------------fTemperature: %f ",pSTServer_Rough->stTempAndHumiRead.fTemperature);

	Push_VarValue(SETTYPE_RAW_SP, pSTServer_Rough->hSampler,
		911, 930, varV);

	varV.emValue = pSTServer_Rough->stTempAndHumiRead.CommFailflag;
	Push_VarValue(SETTYPE_RAW_SP, pSTServer_Rough->hSampler,
		911, 931, varV);

}

BOOL GetFireAlarmStatus(ST_UART_SERVER_ROUGHDATA* pSTServer_Rough)
{
	TRACEI("-----------------GetFireAlarmStatus----------------");
	SAMPLER* pSampler = (SAMPLER*)pSTServer_Rough->hSampler;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	SYSTEM_TYPE emSystemType;
	VAR_VALUE varV;
	int i = 0;
	int id = 0;
	int nESCNum = 0;
	BOOL isTargetAlarm = FALSE;
	static const int targetAlarmIds[] = {739, 813, 814, 815, 816}; // 目标告警 ID

	ERRCDDE_DPR err = pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_ST_VAR, 1, 100, &varV, 4); // System Type 系统类型
	emSystemType = (SYSTEM_TYPE)(err==ERR_DPR_OK ? varV.emValue : SYSTYPE_ESS_AIR);

	err = pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_ST_VAR, 1001, 3314, &varV, 4); // ESC Num 储能柜数量
	nESCNum = (UINT)(err==ERR_DPR_OK ? varV.ulValue : 0);

	if (emSystemType == SYSTYPE_EMS_STATION) // EMS 站级
	{
		TRACEI("System Type: EMS Station");
		A_BLOB_SEMS_CMD109 dataBloCmd902;
		varV.blobValue = (BYTE*)&dataBloCmd902;
		err = pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_SP_VAR, 1001, 3412, &varV, sizeof(A_BLOB_SEMS_CMD109)); // 储能柜总告警信息
		if(err != ERRCODE_DAI_OK)
		{
			return -1;
		}
		int iClientIdx = 0;  // only one client
		for (iClientIdx = 0; iClientIdx < nESCNum; ++iClientIdx)
		{
			int iAlarmCount = dataBloCmd902.clientActiveAlarms[iClientIdx].iAlarmCount;
			for (i = 0; i < iAlarmCount && i < MAX_QUERY_ACTV_ALM; ++i)
			{
				SC6_LDEV_109* pAlarm = 	&dataBloCmd902.clientActiveAlarms[iClientIdx].activeAlms[i];
				for (id = 0; id < sizeof(targetAlarmIds)/sizeof(targetAlarmIds[0]); id++)
				{
					if (pAlarm->nAlarmId == targetAlarmIds[id])
					{
						isTargetAlarm = TRUE;
						TRACE("Set alm_status to TRUE (0) for AlarmId=%d", pAlarm->nAlarmId);
						break;
					}
				}
			}
		}
	}
	else if (emSystemType == SYSTYPE_ESS_LIQUIDED || emSystemType == SYSTYPE_ESS_AIR) // ESS 液冷/风冷
	{
		// check fire alarm status from var 813-816
		int n = 0;
		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			// check target alarm ids
			for (id = 0; id < sizeof(targetAlarmIds)/sizeof(targetAlarmIds[0]); id++)
			{
				err = pDPR->Get(pSTServer_Rough->hSampler, GETTYPE_SP_VAR, 501 + n, targetAlarmIds[id], &varV, 4);
				if(err == ERR_DPR_OK && varV.emValue == 1)
				{
					isTargetAlarm = TRUE;
					TRACE("Set alm_status to TRUE (1) for AlarmId=%d", targetAlarmIds[id]);
					break;
				}
			}
		}
	}

	if(isTargetAlarm)
	{
		TRACED("Fire Alarm Active!");
		return TRUE;
	}
	TRACEI("isTargetAlarm = %d", isTargetAlarm);
	return FALSE;
}

int ModbusSigSamp(IN ST_UART_SERVER_ROUGHDATA *pSTServer_Rough, 
					IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo,
					IN BYTE nAddr, IN int nFunCode, IN WORD wStartAddr, 
                    IN WORD wDataNum)
{
	BYTE* bySendBuf = &pSTServer_Rough->sz_ByMdbSendBuffer;
	BYTE* byRecvBuf = &pSTServer_Rough->sz_ByMdbRecvBuffer;
	SAMPLER *pSampler = (SAMPLER *)pSTServer_Rough->hSampler;
	int i;


	if(Mutex_Lock(pSTServer_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)   //Pending
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}

	Modbus_MakeReqeustData(nAddr, nFunCode, wStartAddr, wDataNum, bySendBuf);

	// printf("modbus send frame : ");
	// for(i = 0; i < 8; i++)
	// {
	// 	printf("%02X ", bySendBuf[i]);
	// }
	// printf("\n");
	
	pSampler->pUsedCommPort->Write(pClientInfo->hClient, bySendBuf, 8);

	Sleep(100);

	int nLen = 0;
	for (i = 0; i < 3; i++)
	{
		nLen = pSampler->pUsedCommPort->Read(pClientInfo->hClient, byRecvBuf, MDB_RECV_BUF_LEN);
		if(nLen > 0)
		{
			break;
		} 
		Sleep(1000);
	}
	if (i == 3)
	{
		pSTServer_Rough->stTempAndHumiRead.CommFailCounter ++;
		if (pSTServer_Rough->stTempAndHumiRead.CommFailCounter == 10)
		{
			pSTServer_Rough->stTempAndHumiRead.CommFailflag=TRUE;
			TRACEE("Read Error: modbus no recv data \n");
		}
		Mutex_Unlock(pSTServer_Rough->hSamplerLock);
		return ERR_SAMPLE_FAILED;
	}

	// BYTE byResp[9] = {0x01, 0x03, 0x04, 0x1A, 0x00, 0x4B, 0x00, 0xCB, 0xDB};
	if (MDB_ERROR_OK != Modbus_JudgeResponse(pSTServer_Rough, pSTServer_Rough->sz_ByMdbRecvBuffer, nLen, nFunCode, nAddr, wStartAddr, wDataNum))
	{
		Mutex_Unlock(pSTServer_Rough->hSamplerLock);
		return ERR_SAMPLE_NEED_RESAMP;
	}
	else
	{
		if (pSTServer_Rough->stTempAndHumiRead.CommFailCounter >= 10)
		{
			pSTServer_Rough->stTempAndHumiRead.CommFailCounter = 0;
			pSTServer_Rough->stTempAndHumiRead.CommFailflag = FALSE;
		}
	}
	Modbus_ParseTempHumiData(pSTServer_Rough, wStartAddr, pSTServer_Rough->sz_ByMdbRecvBuffer + 3, wDataNum);


	Mutex_Unlock(pSTServer_Rough->hSamplerLock);
	return ERR_SAMPLE_OK;

}

//edit by kuangyue 20251029
int Modbus_MakeWriteMultiReq(IN BYTE nAddr, IN int nFunCode, IN WORD wStartAddr, IN WORD wDataNum, 
    						IN BYTE* pWriteData,  IN int nWriteLen,
    						OUT BYTE* pBuf)
{
    memset(pBuf, 0, MDBRTU_SEND_BUF_LEN);

    BYTE *pSendBuf = pBuf;

    *pSendBuf++ = nAddr;                // 从站地址
    *pSendBuf++ = nFunCode;             // 功能码

    *pSendBuf++ = HIBYTE(wStartAddr);   // 起始地址高字节
    *pSendBuf++ = LOBYTE(wStartAddr);   // 起始地址低字节

    *pSendBuf++ = HIBYTE(wDataNum);     // 寄存器数量高字节
    *pSendBuf++ = LOBYTE(wDataNum);     // 寄存器数量低字节

    *pSendBuf++ = (BYTE)nWriteLen;      // 数据字节数

    // 填充数据区
    memcpy(pSendBuf, pWriteData, nWriteLen);
    pSendBuf += nWriteLen;

    // 计算 CRC (前面的长度 = 7 + nWriteLen)
    WORD wCRC = MDB_CRC(pBuf, 7 + nWriteLen);

    *pSendBuf++ = HIBYTE(wCRC);
    *pSendBuf++ = LOBYTE(wCRC);

    return (pSendBuf - pBuf);  // 返回实际报文长度
}

void Modbus_ParseWriteResponseData(
    ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,
    IN WORD wStartAddr,
    BYTE* pData,
    IN int nRegNum)
{
    WORD wAddr = wStartAddr;
    int i;
    for (i = 0; i < nRegNum; i++)
    {
        int j = wAddr + i;
        TRACEI("Write response parsing, address=0x%04X", j);
        switch (j)
        {
			case 0x0010: // 寄存器 0x0010：报警器播放状态
			{
				WORD alarmStatus = MAKEWORD(pData[1], pData[0]);   // 定义变量存放寄存器值
				TRACED("TGSG-190-II-X Status = %d", alarmStatus);
				break;
			}
            case 0x0011: // 寄存器 0x0011：报警器音量
            {
				WORD voice = MAKEWORD(pData[1], pData[0]);
				TRACED("TGSG-190-II-X voice num = %d", voice);
                break;
            }
            case 0x0012: // 寄存器 0x0012：报警器音调
            {
				// 假设为整数值
				WORD VioStatus = MAKEWORD(pData[1], pData[0]);
				TRACED("TGSG-190-II-X Voice status = %d", VioStatus);
                break;
            }
            case 0x0013: // 寄存器 0x0013：报警器播放模式
            {
				WORD mode = MAKEWORD(pData[1], pData[0]);
				TRACED("TGSG-190-II-X Run mode = %d", mode);
                break;
            }
            case 0x0014: // 寄存器 0x0014：警示灯闪烁模式
            {
				WORD light_status = MAKEWORD(pData[1], pData[0]);
				TRACED("TGSG-190-II-X light status = %d", light_status);
                break;
            }
            default:
                TRACED("Unknown register address 0x%04X", j);
                break;
        }
        pData += 2; // 每个寄存器占 2 字节
    }
}

int ModbusWriteMultipleRegisters(
    IN ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,
    IN UART_SERVER_TCP_CLIENT_INFO *pClientInfo,
    IN BYTE nAddr, IN int nFunCode, IN WORD wStartAddr,
    IN WORD wDataNum,
    IN BYTE* byData,
    IN int nDataLen)
{
    BYTE* bySendBuf = &pSTServer_Rough->sz_ByMdbSendBuffer;
    BYTE* byRecvBuf = &pSTServer_Rough->sz_ByMdbRecvBuffer;
    SAMPLER *pSampler = (SAMPLER *)pSTServer_Rough->hSampler;
    int i;

    if (nFunCode != 0x10) // 仅支持功能码 0x10（写多个寄存器）
    {
        TRACEE("Invalid function code for write operation\n");
        return ERR_SAMPLE_FAILED;
    }

    // 获取互斥锁
    if (Mutex_Lock(pSTServer_Rough->hSamplerLock, WAIT_INFINITE) != ERR_MUTEX_OK)
    {
        return ERR_SAMPLE_NEED_RESAMP;
    }

    // 构造 Modbus 请求帧
    Modbus_MakeWriteMultiReq(nAddr, nFunCode, wStartAddr, wDataNum, byData, nDataLen, bySendBuf);
    int nSendLen = 7 + nDataLen + 2; // 地址(1) + 功能码(1) + 起始地址(2) + 寄存器数量(2) + 字节数(1) + 数据(nDataLen) + CRC(2)

    // 发送请求
	Sleep(300);

    pSampler->pUsedCommPort->Write(pClientInfo->hClient, bySendBuf, nSendLen);

    // 等待设备响应
    Sleep(100);

    // 尝试读取响应，最多尝试 3 次
    int nLen = 0;
    for (i = 0; i < 3; i++)
    {
        nLen = pSampler->pUsedCommPort->Read(pClientInfo->hClient, byRecvBuf, MDB_RECV_BUF_LEN);
        if (nLen > 0)
        {
            break;
        }
        Sleep(1000);
    }

    // 如果 3 次未收到响应，记录通信失败
    if (i == 3)
    {
        pSTServer_Rough->stTempAndHumiRead.CommFailCounter++;
        if (pSTServer_Rough->stTempAndHumiRead.CommFailCounter >= 10)
        {
            pSTServer_Rough->stTempAndHumiRead.CommFailflag = TRUE;
            TRACEE("Write Error: modbus no recv data\n");
        }
        Mutex_Unlock(pSTServer_Rough->hSamplerLock);
        return ERR_SAMPLE_FAILED;
    }

    // 验证响应并解析
    if (MDB_ERROR_OK != Modbus_JudgeResponse_WriteMulReg(pSTServer_Rough, byRecvBuf, nLen, nFunCode, nAddr, wStartAddr, wDataNum))
    {
        Mutex_Unlock(pSTServer_Rough->hSamplerLock);
        return ERR_SAMPLE_NEED_RESAMP;
    }
    else
    {
        // 通信成功，重置失败计数器
        if (pSTServer_Rough->stTempAndHumiRead.CommFailCounter >= 10)
        {
            pSTServer_Rough->stTempAndHumiRead.CommFailCounter = 0;
            pSTServer_Rough->stTempAndHumiRead.CommFailflag = FALSE;
        }

        // 解析响应数据
        Modbus_ParseWriteResponseData(pSTServer_Rough, wStartAddr, byRecvBuf + 3, wDataNum);
    }

    // 释放互斥锁
    Mutex_Unlock(pSTServer_Rough->hSamplerLock);
    return ERR_SAMPLE_OK;
}

int AlarmLightMain(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,UART_SERVER_TCP_CLIENT_INFO *pClientInfo)
{
	int errCode;
	int i;

	BYTE writeData[10] = {
		0x00, 0x03, 
		0x00, 0x10, 
		0x01, 0x03, 
		0x00, 0x02, 
		0x02, 0x01
	};//设置警示灯和报警音打开，音量为16 级，第1个文件夹第2曲目，单曲同步播放，警示灯爆闪

	errCode = ModbusWriteMultipleRegisters(pSTServer_Rough, pClientInfo, MDB_ADDR_TEMPHUMI, 0x10, 0x0010, 0x0005, writeData, sizeof(writeData));

	for ( i = 0; i < sizeof(writeData); i++) 
	{
		TRACEI("  writeData[%d] = 0x%02X", i, writeData[i]);
	}
	
	return errCode;
}
int ModbusSampMain(ST_UART_SERVER_ROUGHDATA *pSTServer_Rough,UART_SERVER_TCP_CLIENT_INFO *pClientInfo)
{
	int errCode;

	if(pSTServer_Rough->stTempAndHumiRead.bInstallType == TEMPHUMIL_TYPE_NOTINSTALL)  
	{
		return ERR_SAMPLE_TIMEOUT;
	}
	else if (pSTServer_Rough->stTempAndHumiRead.bInstallType == TEMPHUMIL_TYPE_OMRON_E5CC)
	{
		errCode=ModbusSigSamp(pSTServer_Rough, pClientInfo, MDB_ADDR_TEMPHUMI, MDB_FUN_CODE_03, 0x2000, 0001);
	}
	else if (pSTServer_Rough->stTempAndHumiRead.bInstallType == TEMPHUMIL_TYPE_HY_BWD3K130C)
	{
		errCode=ModbusSigSamp(pSTServer_Rough, pClientInfo, MDB_ADDR_TEMPHUMI_HY, MDB_FUN_CODE_03, 0x0002, 0001);
	}

	return errCode;
}
