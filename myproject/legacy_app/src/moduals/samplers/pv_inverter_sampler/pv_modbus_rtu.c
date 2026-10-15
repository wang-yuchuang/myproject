// #define _TRACE_PRINT_
#include "pv_modbus_rtu.h"


unsigned short getTwoBytes(int iRegAddr, int iBufStartAddr, BYTE *pBuf)
{
	if (iRegAddr < iBufStartAddr)
		return 0;

	int iAddr = (iRegAddr - iBufStartAddr) * 2;
	return (unsigned short)pBuf[iAddr] << 8 | (unsigned short)pBuf[iAddr + 1];
}

unsigned int getFourBytes(int iRegAddr, int iBufStartAddr, BYTE *pBuf)
{
	if (iRegAddr < iBufStartAddr)
		return 0;
		
	int iAddr = (iRegAddr - iBufStartAddr) * 2;
	return (unsigned int)pBuf[iAddr] << 24 | (unsigned int)pBuf[iAddr + 1] << 16 | (unsigned int)pBuf[iAddr + 2] << 8  | (unsigned int)pBuf[iAddr + 3];
}

void getStringFromRegs(int iRegAddr,
                       int iBufStartAddr,
                       int iRegNum,
                       BYTE *pBuf,
                       char *pOutStr,
                       int iOutSize)
{
    if (iRegAddr < iBufStartAddr || pOutStr == NULL || pBuf == NULL)
        return;

    int iAddr = (iRegAddr - iBufStartAddr) * 2;
    int iByteNum = iRegNum * 2;

    int i, j = 0;

    for (i = 0; i < iByteNum && j < iOutSize - 1; i++)
    {
        char ch = (char)pBuf[iAddr + i];

        if (ch == '\0')
            break;

        pOutStr[j++] = ch;
    }

    pOutStr[j] = '\0';
}

USHORT Modbus_CRC16(const BYTE *data, USHORT len) 
{
    USHORT crc = MODBUS_CRC16_INIT;
    while (len--) {
        crc ^= *data++;
		int i;
        for (i=0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ MODBUS_CRC16_POLY;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

BOOL checkModbusRtuFrameValid(BYTE* pDataBuf, int iDataLen, BYTE byAddr, BYTE byFunCode, USHORT uRequestNum)
{
	if (iDataLen < MODBUS_RTU_MIN_LEN || iDataLen > MODBUS_RTU_MAX_LEN) 
	{
        TRACEE("Frame length is insufficient: %d\n", iDataLen);
        return FALSE;
    }

	BYTE* pData = pDataBuf;

	if (byAddr != pData[0])
	{
		TRACEE("byAddr != pData[0] \n");
		return FALSE;
	}

	if (byFunCode != pData[1])
	{
		TRACEE("byFunCode != pData[1] \n");
		return FALSE;
	}

	if (pDataBuf[2] != uRequestNum * 2)
	{
		TRACEE("pDataBuf[2] != uRequestNum * 2 \n");
		return FALSE;
	}

	int iExpectLen = 5 + uRequestNum * 2;
	if (iDataLen != iExpectLen)
	{
		TRACEE("iDataLen != iExpectLen \n");
		return FALSE;
	}

	USHORT usCRC = Modbus_CRC16(pDataBuf, iDataLen - 2);
	USHORT usCRCRecv = 	(pDataBuf[iDataLen - 1] << 8) | pDataBuf[iDataLen - 2];
	if (usCRC != usCRCRecv)
	{
		TRACEE("usCRC != usCRCRecv \n");
		return FALSE;
	}

	return TRUE;
}

BOOL packModbusRtuDataToSend(BYTE byAddr, BYTE byFunCode, USHORT usRegAddr, USHORT usRegNum, BYTE* pSendBuf, int iBufLen)
{
	if (NULL == pSendBuf || iBufLen < MODBUS_RTU_SEND_COMMAND_LEN)
		return FALSE;

    BYTE* pBuf = pSendBuf;

    *pBuf++ = byAddr;
	*pBuf++ = byFunCode; //function code

	*pBuf++ = (BYTE)(usRegAddr >> 8);
	*pBuf++ = (BYTE)(usRegAddr & 0Xff);

	*pBuf++ = (BYTE)(usRegNum >> 8);
	*pBuf++ = (BYTE)(usRegNum & 0Xff);

	USHORT usCRC = Modbus_CRC16(pSendBuf, 6);
	//*pBuf++ = (BYTE)(usCRC >> 8);
	//*pBuf++ = (BYTE)(usCRC & 0Xff);

	
	*pBuf++ = (BYTE)(usCRC & 0Xff);
	*pBuf++ = (BYTE)(usCRC >> 8);

    return TRUE;
}

BOOL sampleModbusRtuData(int iSlaveAddr, MODBUS_REQUEST_INFO* pReqInfo, COMMPORT* pCOMPort, HANDLE hPort, void* pOutData)
{
	BYTE	bySendBuf[MODBUS_RTU_SEND_COMMAND_LEN];
	BYTE	byRecvBuf[MAX_RECEIVE_BUF_LEN];

	BOOL bPackRet = packModbusRtuDataToSend(iSlaveAddr, pReqInfo->byFunCode, pReqInfo->usRegAddr, pReqInfo->usRegNum, bySendBuf, MODBUS_RTU_SEND_COMMAND_LEN);
	if (!bPackRet)
	{
		// printf("packModbusRtuDataToSend[%04X] ERROR!\n", pReqInfo->usRegAddr);
		TRACEE("packModbusRtuDataToSend ERROR!\n");
		return FALSE;
	}
	
	int nWriteBytes = pCOMPort->Write(hPort, bySendBuf, MODBUS_RTU_SEND_COMMAND_LEN);

	if(nWriteBytes < MODBUS_RTU_SEND_COMMAND_LEN)
	{
		// printf("nWriteBytes < MODBUS_RTU_SEND_COMMAND_LEN[%04X]\n", pReqInfo->usRegAddr);
		TRACEE("nWriteBytes < MODBUS_RTU_SEND_COMMAND_LEN \n");
		return FALSE;
	}
	
	// int k;
	// printf("Modbus-RTU Send:");
	// for(k = 0; k < MODBUS_RTU_SEND_COMMAND_LEN; k++)
	// {
	// 	printf("%02X ", bySendBuf[k]);
	// }
	// printf("\n");
	
	Sleep(20);
	
	int n;
	int nReadBytes = 0;
	
	//read back (单次读: 块级重试会累加超时导致断线检测过慢,
	//sampler 层已有 3 次连续失败容错, 块级不需要重试)
	memset(byRecvBuf, 0, sizeof(byRecvBuf));
	nReadBytes = pCOMPort->Read(hPort, byRecvBuf, MAX_RECEIVE_BUF_LEN);

	//check length
	if(nReadBytes < 7)
	{
		// printf("Modbus-RTU receive[%04X] bytes[%d] < 7\n", pReqInfo->usRegAddr, nReadBytes);
		TRACEE("MOdbus read bytes[%d]\n", nReadBytes);
		return FALSE;
	}

	// printf("Modbus-RTU receive length: %d  data: ", nReadBytes);

	// for(k = 0; k < nReadBytes; k++)
	// {
	// 	printf("%02X ", byRecvBuf[k]);
	// }
	// printf("\n");

	BOOL bValid = checkModbusRtuFrameValid(byRecvBuf, nReadBytes, iSlaveAddr, pReqInfo->byFunCode, pReqInfo->usRegNum);
	if (!bValid)
	{
		// printf("checkModbusRtuFrameValid[%04X] ERROR!\n", pReqInfo->usRegAddr);
		TRACEE("checkModbusRtuFrameValid ERROR!\n");
		return FALSE;
	}
		

	pReqInfo->fnParseModbusRtuData(pReqInfo->usRegAddr, &byRecvBuf[3], pOutData);

	return TRUE;
}


BOOL packModbusRtuWriteSignal(BYTE byAddr, USHORT usRegAddr, SHORT usWriteVal, BYTE* pSendBuf, int iBufLen)
{
	if (NULL == pSendBuf || iBufLen < MODBUS_RTU_SEND_COMMAND_LEN)
	{
		LOGFD("pv pv_modbus_rtu:packModbusRtuWriteSignal return FALSE pSendBuf:%x iBufLen:%d", pSendBuf, iBufLen);
		return FALSE;
	}

    BYTE* pBuf = pSendBuf;

    *pBuf++ = byAddr;
	*pBuf++ = 0x06; //function code

	*pBuf++ = (BYTE)(usRegAddr >> 8);
	*pBuf++ = (BYTE)(usRegAddr & 0Xff);

	*pBuf++ = (BYTE)(usWriteVal >> 8);
	*pBuf++ = (BYTE)(usWriteVal & 0Xff);

	USHORT usCRC = Modbus_CRC16(pSendBuf, 6);
	
	*pBuf++ = (BYTE)(usCRC & 0Xff);
	*pBuf++ = (BYTE)(usCRC >> 8);

    return TRUE;
}

BOOL writeModbusRtuSignalRegister(int iSlaveAddr, USHORT usRegAddr, SHORT usWriteVal, COMMPORT* pCOMPort, HANDLE hPort)
{
	BYTE	bySendBuf[MODBUS_RTU_SEND_COMMAND_LEN];
	BYTE	byRecvBuf[MAX_RECEIVE_BUF_LEN];
	int		retry;

	for(retry = 0; retry < 2; retry++)
	{
		BOOL bPackRet = packModbusRtuWriteSignal(iSlaveAddr, usRegAddr, usWriteVal, bySendBuf, MODBUS_RTU_SEND_COMMAND_LEN);
		if (!bPackRet)
		{
			LOGFD("pv pv_modbus_rtu:writeModbusRtuSignalRegister return FALSE !bPackRet");
			return FALSE;
		}

		/* Purge RX buffer before write to flush stale data from periodic Sample() reads.
		 * Without this, the receive buffer may contain leftover response bytes from the
		 * last Modbus read cycle, causing the echo memcmp below to fail intermittently. */
		pCOMPort->Control(hPort, COMM_PURGE_RXCLEAR, NULL, 0);
		usleep(5000);  /* 5ms settle after purge */

		int nWriteBytes = pCOMPort->Write(hPort, bySendBuf, MODBUS_RTU_SEND_COMMAND_LEN);

		if(nWriteBytes < MODBUS_RTU_SEND_COMMAND_LEN)
		{
			LOGFD("pv pv_modbus_rtu:writeModbusRtuSignalRegister return FALSE nWriteBytes:%d", nWriteBytes);
			return FALSE;
		}

		int k;
		TRACE("Modbus-RTU Send:");
		for(k = 0; k < MODBUS_RTU_SEND_COMMAND_LEN; k++)
		{
			TRACE("%02X ", bySendBuf[k]);
		}
		TRACE("\n");

		Sleep(30);  /* 30ms wait for slave echo (was 20, increased for reliability) */

		int n;
		int nReadBytes = 0;

		/* Read back echo (retry read up to 3 times for short responses) */
		memset(byRecvBuf, 0, sizeof(byRecvBuf));
		for(n = 0; n < 3; n++)
		{
			nReadBytes = pCOMPort->Read(hPort, byRecvBuf, MAX_RECEIVE_BUF_LEN);
			if(nReadBytes >= MODBUS_RTU_SEND_COMMAND_LEN)
			{
				break;
			}
			Sleep(20);
		}

		TRACE("Modbus-RTU receive length: %d  data:\n", nReadBytes);

		for(k = 0; k < nReadBytes; k++)
		{
			TRACE("%02X ", byRecvBuf[k]);
		}
		TRACE("\n");

		if (nReadBytes >= MODBUS_RTU_SEND_COMMAND_LEN && 0 == memcmp(bySendBuf, byRecvBuf, MODBUS_RTU_SEND_COMMAND_LEN))
		{
			return TRUE;  /* success */
		}

		/* First attempt failed — log and retry */
		LOGFD("pv pv_modbus_rtu:writeModbusRtuSignalRegister attempt %d failed (reg=0x%04X, read=%d bytes)",
		       retry + 1, usRegAddr, nReadBytes);
	}

	/* Both attempts failed */
	LOGFD("pv pv_modbus_rtu:writeModbusRtuSignalRegister FAILED after 2 retries (reg=0x%04X)", usRegAddr);
	return FALSE;
}

int packModbusRtuWriteMultiple(BYTE byAddr, USHORT usRegAddr, USHORT iWriteRegNum, SHORT* usValue, BYTE* pSendBuf, int iBufLen)
{
	if (NULL == pSendBuf || /*iBufLen < MODBUS_RTU_SEND_COMMAND_LEN ||*/ iWriteRegNum >= 100)
		return 0;

    BYTE* pBuf = pSendBuf;

    *pBuf++ = byAddr;
	*pBuf++ = 0x10; //function code

	*pBuf++ = (BYTE)(usRegAddr >> 8);
	*pBuf++ = (BYTE)(usRegAddr & 0Xff);

	*pBuf++ = (BYTE)(iWriteRegNum >> 8);
	*pBuf++ = (BYTE)(iWriteRegNum & 0Xff);

	*pBuf++ = (BYTE)(iWriteRegNum * 2);

	int i = 0;
	for (i = 0; i < iWriteRegNum; ++i)
	{
		*pBuf++ = (BYTE)(usValue[i] >> 8);
		*pBuf++ = (BYTE)(usValue[i] & 0Xff);
	}

	USHORT usCRC = Modbus_CRC16(pSendBuf, 6);
	
	*pBuf++ = (BYTE)(usCRC & 0Xff);
	*pBuf++ = (BYTE)(usCRC >> 8);

	int iLen = pBuf - pSendBuf;
    return iLen;
}

BOOL writeModbusRtuMultipleRegister(int iSlaveAddr, USHORT usRegAddr, USHORT iWriteRegNum, SHORT* usValue, COMMPORT* pCOMPort, HANDLE hPort)
{
	BYTE	bySendBuf[MODBUS_RTU_MAX_LEN];
	BYTE	byRecvBuf[MAX_RECEIVE_BUF_LEN];

	int iPackLen = packModbusRtuWriteMultiple(iSlaveAddr, usRegAddr, iWriteRegNum, usValue,  bySendBuf, MODBUS_RTU_MAX_LEN);
	if (iPackLen <= 0)
		return FALSE;
		
	int nWriteBytes = pCOMPort->Write(hPort, bySendBuf, iPackLen);

	if(nWriteBytes < iPackLen)
	{
		return FALSE;
	}

	int k;
	printf("Modbus-RTU Send:");
	for(k = 0; k < iPackLen; k++)
	{
		printf("%02X ", bySendBuf[k]);
	}
	printf("\n");
	
	Sleep(20);
	
	int n;
	int nReadBytes = 0;
	
	//read back for 2 times
	memset(byRecvBuf, 0, sizeof(byRecvBuf));
	for(n = 0; n < 2; n++)
	{
		nReadBytes = pCOMPort->Read(hPort, byRecvBuf, MAX_RECEIVE_BUF_LEN);
		if(nReadBytes >= 7)
		{
			break;
		}
		Sleep(20);
	}

	printf("Modbus-RTU receive length: %d  data:\n", nReadBytes);

	for(k = 0; k < nReadBytes; k++)
	{
		printf("%02X ", byRecvBuf[k]);
	}
	printf("\n");

	if (0 != memcmp(bySendBuf, byRecvBuf, 6))
	{
		return FALSE;
	}

	return TRUE;
}

