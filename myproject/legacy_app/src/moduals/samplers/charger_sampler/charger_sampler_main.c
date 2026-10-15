#include "charger_sampler_main.h"
DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
    // printf("Initialize Charger Sampler...\n");
	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;

	pSampler->pDataHeap = (void*)NEW(ROUGH_CHARGER_DATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}

    // 初始化 rough data
    ROUGH_CHARGER_DATA  *pCharger_Modbus_Rough = (ROUGH_CHARGER_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pCharger_Modbus_Rough, 1);

    pCharger_Modbus_Rough->hSampler = hSamplerSelf;
	pCharger_Modbus_Rough->nSamplerId = pSampler->info.nSamplerID;
	pCharger_Modbus_Rough->pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	pCharger_Modbus_Rough->hSamplerLock = pSampler->hSamplerLock;

    // printf("Charger Sampler [Initialize]SamplerId = [%d]\n", pCharger_Modbus_Rough->nSamplerId);

	InitModbusDataMap(pCharger_Modbus_Rough);

	if (!ChargerModbusTCPInitialize(pCharger_Modbus_Rough))
	{
		return FALSE;
	}

	// printf("Initialize Charger Sampler [Done].\n");
	return TRUE;
}

DLLExport int Sample(HANDLE hSamplerThis)
{
	// printf("Charger Sampler ==========================\n");

	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	ChargerModbusTCPSample(pSampler);

	// printf("Charger Sampler 【over】==========================\n");
	return ERR_SAMPLE_OK;
}

DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
    printf("pModbusRough:Control emParaType =[%d] nDevId=[%d] nParamId=[%d] nValSize=[%d]\n",emParaType, nDevId,nParamId,nValSize);
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	ROUGH_CHARGER_DATA *pModbusRough = (ROUGH_CHARGER_DATA*)pSampler->pDataHeap;
	// A_BLOB_SET_CHARGER_CMD* setChargerCmd = (A_BLOB_SET_CHARGER_CMD*)pVarVal->blobValue;

	// uint16_t nAddress = setChargerCmd->nAddress;
	// printf("nAddress = [%04x]\n", nAddress);
	// printf("setModbus->pData == NULL = [%d]\n", setChargerCmd->pData == NULL);
	// uint16_t nData = (uint16_t)setChargerCmd->pData;
	// char* szData = (char*)setChargerCmd->pData;
	// printf("nData == NULL = [%d]\n", nData == NULL);
	// printf("nData == NULL = [%d]\n", szData == NULL);
	// printf("nData = [%s]\n", szData);
	// printf("nData = [%04x]\n", nData);
	// int i;
	// for (i = 0; i < MID_MAX; ++i)
	// {
	// 	if (pModbusRough->ModbusDataMap[i].nRegAddrStart == nAddress)
	// 	{
	// 		printf("ccu_sampler_main:Control ModbusDataMap[%d].nRegAddrStart = [%04x]\n", i, nAddress);
	// 		int rc = modbus_write_register(pModbusRough->ctx, nAddress, nData);
	// 		if (rc == -1) 
	// 		{
	// 			// ❌ 写入失败
	// 			fprintf(stderr, "✗ Write failed: %s\n", modbus_strerror(errno));
	// 		} else if (rc == 1) 
	// 		{
	// 			// ✅ 写入成功
	// 			printf("✓ Write successful\n");
	// 		} else {
	// 			// ⚠️ 异常情况（理论上不会发生）
	// 			fprintf(stderr, "⚠ Unexpected return value: %d\n", rc);
	// 		}
	// 			break;
	// 	}
	// }

    int nRet = ERR_SAMPLE_OK;
	if(Mutex_Lock(pModbusRough->hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		TRACEE("ccu_sampler_main:Control ERR_CONTROL_TIMEOUT");
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_CONTROL)
	{
		if(nParamId == 100)
		{
			uint16_t nAddress = 0xC001;
			uint16_t nData = (uint16_t)pVarVal->emValue;
			printf("nAddress[%04x] nData[%04x]\n", nAddress, nData);
			int rc = modbus_write_register(pModbusRough->ctx, nAddress, nData);
			if (rc == -1) 
			{
				// ❌ 写入失败
				fprintf(stderr, "✗ Write failed: %s\n", modbus_strerror(errno));
			} else if (rc == 1) 
			{
				// ✅ 写入成功
				printf("✓ Write successful\n");
			} else {
				// ⚠️ 异常情况（理论上不会发生）
				fprintf(stderr, "⚠ Unexpected return value: %d\n", rc);
			}
		}
		else if (nParamId == 101)
		{
			uint16_t nAddress = 0xC008;
			uint16_t nData = (uint16_t)pVarVal->ulValue;
			printf("nAddress[%04x] nData[%04x]\n", nAddress, nData);
			int rc = modbus_write_register(pModbusRough->ctx, nAddress, nData);
			if (rc == -1) 
			{
				// ❌ 写入失败
				fprintf(stderr, "✗ Write failed: %s\n", modbus_strerror(errno));
			} else if (rc == 1) 
			{
				// ✅ 写入成功
				printf("✓ Write successful\n");
			} else {
				// ⚠️ 异常情况（理论上不会发生）
				fprintf(stderr, "⚠ Unexpected return value: %d\n", rc);
			}
		}
	}
	else
	{
		printf("Charger ModbusTCP: Not support [ParaType]\n");
	}

	Mutex_Unlock(pModbusRough->hSamplerLock);

	return nRet;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

// AA 7E AA B0 00 F9 00 01 00 00 70 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 A1 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0D 00 00 00 40 00 00 00 00 00 00 00 00 00 00 19 64 19 64 00 00 00 00 00 00 00 00 00 00 00 00 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 00 00 00 00 00 00 00 1E F4 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 0F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 0F 01 44 22 80 00 44 22 80 00 16 D5 0D

// AA 7E AA B0 00 F9 00 01 00 00 70 80 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 10 00 00 00 00 00 00 00 00 00 00 A1 00 00 00 00 00 00 00 00 00 00 00 00 00 00 0D 00 00 00 40 00 00 00 00 00 00 00 00 00 00 19 64 19 64 00 00 00 00 00 00 00 00 00 00 00 00 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 13 88 00 00 00 00 00 00 00 1E F4 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 01 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 FF FF 00 00 00 00 00 00 00 0F 00 00 00 00 00 00 00 00 00 00 00 00 00 00 09 0F 01 44 22 80 00 44 22 80 00 16 D5 0D