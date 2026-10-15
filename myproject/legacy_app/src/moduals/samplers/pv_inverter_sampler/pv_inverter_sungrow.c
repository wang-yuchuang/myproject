#include "pv_inverter_main.h"
#include "pv_modbus_rtu.h"

int iTypeOfDCInput;

BOOL parseSungrowData_0x04_4949(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	// printf("parseSungrowData_0x04_0 called. iStartAddr = [%d]\n", iStartAddr);
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	// // 逆变器序列号 
	// int iSnIdx = iStartAddr + 61;
	// int i;
	// for (i = 0; i < 4; ++i)
	// {
	// 	USHORT usValue = getTwoBytes(iSnIdx + i, iStartAddr, pRcvBuf);
	// 	// 高字节在前
	// 	// pData->sn[i * 2]     = (char)(usValue >> 8);
	// 	// pData->sn[i * 2 + 1] = (char)(usValue & 0xFF);
	// 	int err = sprintf(pData->sn + i * 4, "%1x%1x%1x%1x",
    //         usValue & 0x0F,
    //         (usValue >> 4) & 0x0F,
    //         (usValue >> 8) & 0x0F,
    //         (usValue >> 12) & 0x0F);
	// }
	// printf("sn = [%s]\n", pData->sn);

	// 固件版本号（DSP）
	// USHORT usFwVer = getTwoBytes(4999, iStartAddr, pRcvBuf);
	// printf("usFwVer = [%04x]\n", usFwVer);
	// sprintf(pData->szFwVer, "%02x%02x", usFwVer & 0xFF, (usFwVer >> 8) & 0xFF);

	pData->iRatedPower	= getTwoBytes(5000, iStartAddr, pRcvBuf) * 0.1;	//额定有功功率  5000

	pData->fLineVoltAB	= getTwoBytes(5018, iStartAddr, pRcvBuf) * 0.1;	//电网线电压AB  5018
	pData->fLineVoltBC	= getTwoBytes(5019, iStartAddr, pRcvBuf) * 0.1;	//电网线电压BC  3035
	pData->fLineVoltCA	= getTwoBytes(5020, iStartAddr, pRcvBuf) * 0.1;	//电网线电压CA  5020

	pData->fPhaseVoltA	= getTwoBytes(5018, iStartAddr, pRcvBuf) * 0.1;	//电网相电压A  5018
	pData->fPhaseVoltB	= getTwoBytes(5019, iStartAddr, pRcvBuf) * 0.1;	//电网相电压B  5019
	pData->fPhaseVoltC	= getTwoBytes(5020, iStartAddr, pRcvBuf) * 0.1;	//电网相电压C  5020

	pData->fPhaseCurrA	= getTwoBytes(5021, iStartAddr, pRcvBuf) * 0.1;	//电网相电流A  5021
	pData->fPhaseCurrB	= getTwoBytes(5022, iStartAddr, pRcvBuf) * 0.1;	//电网相电流B  5022
	pData->fPhaseCurrC	= getTwoBytes(5023, iStartAddr, pRcvBuf) * 0.1;	//电网相电流C  5023

	// printf("fLineVoltAB = [%.1f], fLineVoltBC = [%.1f], fLineVoltCA = [%.1f]\n", pData->fLineVoltAB, pData->fLineVoltBC, pData->fLineVoltCA);
	// printf("fPhaseVoltA = [%.1f], fPhaseVoltB = [%.1f], fPhaseVoltC = [%.1f]\n", pData->fPhaseVoltA, pData->fPhaseVoltB, pData->fPhaseVoltC);
	// printf("fPhaseCurrA = [%.1f], fPhaseCurrB = [%.1f], fPhaseCurrC = [%.1f]\n", pData->fPhaseCurrA, pData->fPhaseCurrB, pData->fPhaseCurrC);

	pData->fFreq = getTwoBytes(5035, iStartAddr, pRcvBuf) * 0.1;	//电网频率  5035 精度 0.1
	pData->fOutputActivePower = getFourBytes(5030, iStartAddr, pRcvBuf);	//逆变器当前输出交流有功功率 W  5030
	pData->iOutputPowerFactor = getTwoBytes(5034, iStartAddr, pRcvBuf)*0.001;	//逆变器当前输出功率因数   5034
	pData->fInnerTemperature	= getTwoBytes(3092, iStartAddr, pRcvBuf) * 0.1;	//内部环境温度  5007
	
	pData->fDailyPowerGeneration = getTwoBytes(5002, iStartAddr, pRcvBuf) * 0.1;	//当日发电量 0.1kwh 5002
	pData->fTotalPowerGeneration = getFourBytes(5003, iStartAddr, pRcvBuf);	//总共发电量 kwh 5003

	// printf("fFreq = [%.2f], fOutputActivePower = [%.1f], iOutputPowerFactor = [%d], fInnerTemperature = [%.1f], iRealPowerPercent = [%d]\n",
	// 	pData->fFreq, pData->fOutputActivePower, pData->iOutputPowerFactor, pData->fInnerTemperature, pData->iRealPowerPercent);
	// printf("fDailyPowerGeneration = [%.1f], fTotalPowerGeneration = [%.1f]\n", 
	// 	pData->fDailyPowerGeneration, pData->fTotalPowerGeneration);

	// // 根据 3004 直流输入类型的值去读取PV组数据
	// iTypeOfDCInput = getTwoBytes(3003, iStartAddr, pRcvBuf);
	// // iTypeOfDCInput = 3;	// 测试用，强制为3路输入
	// if (iTypeOfDCInput < 4)
	// {
	// 	for (i = 0; i < 4; ++i)
	// 	{
	// 		pData->fPVGroupVolt[i] = getTwoBytes(3021 + i * 2, iStartAddr, pRcvBuf) * 0.1;	// PV1电压 3022
	// 		pData->fPVGroupCurr[i] = getTwoBytes(3021 + i * 2 + 1, iStartAddr, pRcvBuf) * 0.1;// PV1电流 3023
	// 		// printf("PV Group %d: Volt = [%.1f], Curr = [%.1f]\n", i + 1, pData->fPVGroupVolt[i], pData->fPVGroupCurr[i]);
	// 	}
	// }

	return TRUE;
}

// BOOL parseSungrowData_0x04_5070(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
// {
// 	// printf("parseSungrowData_0x04_1 called. iStartAddr = [%d]\n", iStartAddr);
// 	if (NULL == pRcvBuf || NULL == pOutData)
// 		return FALSE;

// 	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;
// 	int i;
	
// 	// iTypeOfDCInput = 5;	// 测试用，强制为 5

// 	if (iTypeOfDCInput >= 4)
// 	{
// 		// PV电压对应几路PV电流
// 		int nPVGroupNum = getTwoBytes(3286, iStartAddr, pRcvBuf);
// 		// printf("nPVGroupNum = [%d]\n", nPVGroupNum);
// 		// PV 电压
// 		for (i = 0; i < 16; ++i)
// 		{
// 			pData->fPVGroupVolt[i] = getTwoBytes(3320 + i, iStartAddr, pRcvBuf) * 0.1;
// 			// printf("PV Group [%d]: Volt = [%.1f]\n", i + 1, pData->fPVGroupVolt[i]);
// 		}
// 		// PV 电流 协议数据只有20个连续的pv电流，需要分开取
// 		for (i = 0; i < 20; ++i)
// 		{
// 			pData->fPVGroupCurr[i] = getTwoBytes(3300 + i, iStartAddr, pRcvBuf) * 0.1;
// 			// printf("PV Group [%d]: Curr = [%.1f]\n", i + 1, pData->fPVGroupCurr[i]);
// 		}
// 		for (i = 0; i < 10; ++i)
// 		{
// 			pData->fPVGroupCurr[20 + i] = getTwoBytes(3288 + i, iStartAddr, pRcvBuf) * 0.1;
// 			// printf("PV Group [%d]: Curr = [%.1f]\n", i + 1, pData->fPVGroupCurr[i]);
// 		}
// 		for (i = 0; i < 2; ++i)
// 		{
// 			pData->fPVGroupCurr[30 + i] = getTwoBytes(3336 + i, iStartAddr, pRcvBuf) * 0.1;
// 			// printf("PV Group [%d]: Curr = [%.1f]\n", i + 1, pData->fPVGroupCurr[i]);
// 		}
// 	}

// 	return TRUE;
// }



static MODBUS_REQUEST_INFO modbusRtuReqInfo[] = 
{
	// 阳光电源协议协议所有寄存器地址在使用时需减1访问
	{0x4, 4949, 100, parseSungrowData_0x04_4949},
	// {0x4, 5070, 80, parseSungrowData_0x04_5070}
};

int samplePVInverterSungrow(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich)
{
	// printf("samplePVInverterSungrow called.------------------\n");
	int i = 0;
	BOOL bRet = TRUE;
	// SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;

	PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
	PORT_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);

	BOOL IsSamplerValid = TRUE;
	for (i = 0; i < sizeof(modbusRtuReqInfo) / sizeof(modbusRtuReqInfo[0]); ++i)
	{
		bRet = sampleModbusRtuData(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 
				&modbusRtuReqInfo[i], 
				pPortInfo->pCommPort, 
				(HANDLE)pPortInfo->hPort, pPVOutData);
		static int iCommFailCount[MAX_PVInverter_NUM] = {0};
		if (!bRet)
		{
			iCommFailCount[inwhich]++;
			IsSamplerValid = FALSE;
			// printf("PV Inverter [Sungrow] Sample Failed[%d].\n");
			if (iCommFailCount[inwhich] > 5)
			{
				pPvInverterSample->stPVRunData[inwhich].bIsCommFail = TRUE;
				continue;
			} 
		}
		else
		{
			
			pPvInverterSample->stPVRunData[inwhich].bIsCommFail = FALSE; 
		}
	}

	return;
}

BOOL setPVCmdSungrow(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich)
{
	BOOL bRet = FALSE;
	// SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;

	PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
	PORT_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);

	switch (eCmd)
	{
		case PV_CMD_PowerOn:
		{
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 5005, 0xCF, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		case PV_CMD_PowerOff:
		{
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 5006, 0xCE, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		case PV_CMD_ActiveOutputPower:
		{
			// 先开启限功率开关, 5006 写入 0xAA 启用限功率开关
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 5006, 0xAA, pPortInfo->pCommPort, pPortInfo->hPort);
			if (!bRet)
			{
				return FALSE;
			}

			// 设置限功率值 5007 写入实际功率百分比
			int iPowerPercentage = usCmdData[0] * 100 / pPvInverterSample->stPVSettingParam[inwhich].iRatedPower;
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 5007, iPowerPercentage, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		default:
		{
			return FALSE;
		}

	}
	return bRet;
}