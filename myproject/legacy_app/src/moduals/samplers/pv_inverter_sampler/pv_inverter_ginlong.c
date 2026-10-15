#include "pv_inverter_main.h"
#include "pv_modbus_rtu.h"

int iTypeOfDCInput;

BOOL parseGinlongData_0x04_2999(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	// printf("parseGinlongData_0x04_0 called. iStartAddr = [%d]\n", iStartAddr);
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	// 逆变器序列号 
	int iSnIdx = iStartAddr + 61;
	int i;
	for (i = 0; i < 4; ++i)
	{
		USHORT usValue = getTwoBytes(iSnIdx + i, iStartAddr, pRcvBuf);
		// 高字节在前
		// pData->sn[i * 2]     = (char)(usValue >> 8);
		// pData->sn[i * 2 + 1] = (char)(usValue & 0xFF);
		int err = sprintf(pData->sn + i * 4, "%1x%1x%1x%1x",
            usValue & 0x0F,
            (usValue >> 4) & 0x0F,
            (usValue >> 8) & 0x0F,
            (usValue >> 12) & 0x0F);
	}
	// printf("sn = [%s]\n", pData->sn);

	// 固件版本号（DSP）
	USHORT usFwVer = getTwoBytes(3001, iStartAddr, pRcvBuf);
	// printf("usFwVer = [%04x]\n", usFwVer);
	sprintf(pData->szFwVer, "%02x%02x", usFwVer & 0xFF, (usFwVer >> 8) & 0xFF);

	UINT PvStatusTemp1 = getTwoBytes(3029, iStartAddr, pRcvBuf);	// 运行状态字1
	UINT PvStatusTemp2 = getTwoBytes(3043, iStartAddr, pRcvBuf);	// 运行状态字2
	pData->uWarnCode = PvStatusTemp2 << 16 | PvStatusTemp1;
	if (PvStatusTemp2 == 0x0003)
	{
		pData->iWorkMode = PV_STATUS_RUNNING;	// 正常运行
	}
	else if (PvStatusTemp2 == 0x0000)
	{
		pData->iWorkMode = PV_STATUS_RUNNING;	// 
	}
	else
	{
		pData->iWorkMode = PV_STATUS_FAULT;
	}


	pData->fLineVoltAB	= getTwoBytes(3033, iStartAddr, pRcvBuf) * 0.1;	//电网线电压AB  3034
	pData->fLineVoltBC	= getTwoBytes(3034, iStartAddr, pRcvBuf) * 0.1;	//电网线电压BC  3035
	pData->fLineVoltCA	= getTwoBytes(3035, iStartAddr, pRcvBuf) * 0.1;	//电网线电压CA  3036

	pData->fPhaseVoltA	= getTwoBytes(3033, iStartAddr, pRcvBuf) * 0.1;	//电网相电压A  3034
	pData->fPhaseVoltB	= getTwoBytes(3034, iStartAddr, pRcvBuf) * 0.1;	//电网相电压B  3035
	pData->fPhaseVoltC	= getTwoBytes(3035, iStartAddr, pRcvBuf) * 0.1;	//电网相电压C  3036

	pData->fPhaseCurrA	= getTwoBytes(3036, iStartAddr, pRcvBuf) * 0.1;	//电网相电流A  3037
	pData->fPhaseCurrB	= getTwoBytes(3037, iStartAddr, pRcvBuf) * 0.1;	//电网相电流B  3038
	pData->fPhaseCurrC	= getTwoBytes(3038, iStartAddr, pRcvBuf) * 0.1;	//电网相电流C  3039

	// printf("fLineVoltAB = [%.1f], fLineVoltBC = [%.1f], fLineVoltCA = [%.1f]\n", pData->fLineVoltAB, pData->fLineVoltBC, pData->fLineVoltCA);
	// printf("fPhaseVoltA = [%.1f], fPhaseVoltB = [%.1f], fPhaseVoltC = [%.1f]\n", pData->fPhaseVoltA, pData->fPhaseVoltB, pData->fPhaseVoltC);
	// printf("fPhaseCurrA = [%.1f], fPhaseCurrB = [%.1f], fPhaseCurrC = [%.1f]\n", pData->fPhaseCurrA, pData->fPhaseCurrB, pData->fPhaseCurrC);

	pData->fFreq = getTwoBytes(3042, iStartAddr, pRcvBuf) * 0.01;	//电网频率  3043
	pData->fOutputActivePower = getFourBytes(3004, iStartAddr, pRcvBuf);	//逆变器当前输出交流有功功率 W  3005
	pData->iOutputPowerFactor = getTwoBytes(3050, iStartAddr, pRcvBuf);	//逆变器当前输出功率因数   3051
	// pData->fPVInputTotalPower = getFourBytes(4, iStartAddr, pRcvBuf) * 0.1;	//光伏直流输入总功率 W  W  3004
	pData->fInnerTemperature	= getTwoBytes(3092, iStartAddr, pRcvBuf) * 0.1;	//内部环境温度  3093
	pData->iRealPowerPercent = getTwoBytes(3049, iStartAddr, pRcvBuf);			//实际功率百分比  3050
	
	pData->fDailyPowerGeneration = getTwoBytes(3014, iStartAddr, pRcvBuf) * 0.1;	//当日发电量 3015
	pData->fTotalPowerGeneration = getFourBytes(3008, iStartAddr, pRcvBuf);	//总共发电量 3009

	// printf("fFreq = [%.2f], fOutputActivePower = [%.1f], iOutputPowerFactor = [%d], fInnerTemperature = [%.1f], iRealPowerPercent = [%d]\n",
	// 	pData->fFreq, pData->fOutputActivePower, pData->iOutputPowerFactor, pData->fInnerTemperature, pData->iRealPowerPercent);
	// printf("fDailyPowerGeneration = [%.1f], fTotalPowerGeneration = [%.1f]\n", 
	// 	pData->fDailyPowerGeneration, pData->fTotalPowerGeneration);

	// 根据 3004 直流输入类型的值去读取PV组数据
	iTypeOfDCInput = getTwoBytes(3003, iStartAddr, pRcvBuf);
	// iTypeOfDCInput = 3;	// 测试用，强制为3路输入
	if (iTypeOfDCInput < 4)
	{
		for (i = 0; i < 4; ++i)
		{
			pData->fPVGroupVolt[i] = getTwoBytes(3021 + i * 2, iStartAddr, pRcvBuf) * 0.1;	// PV1电压 3022
			pData->fPVGroupCurr[i] = getTwoBytes(3021 + i * 2 + 1, iStartAddr, pRcvBuf) * 0.1;// PV1电流 3023
			// printf("PV Group %d: Volt = [%.1f], Curr = [%.1f]\n", i + 1, pData->fPVGroupVolt[i], pData->fPVGroupCurr[i]);
		}
	}

	pData->usYear = getTwoBytes(3072, iStartAddr, pRcvBuf);	// 本地系统时间 3073
	pData->usMonth = getTwoBytes(3073, iStartAddr, pRcvBuf);
	pData->usDay = getTwoBytes(3074, iStartAddr, pRcvBuf);
	pData->usHour = getTwoBytes(3075, iStartAddr, pRcvBuf);
	pData->usMin = getTwoBytes(3076, iStartAddr, pRcvBuf);
	pData->usSecond = getTwoBytes(3077, iStartAddr, pRcvBuf);

	// printf("System Time: %d-%d-%d %d:%d:%d\n",
	// 	pData->usYear, pData->usMonth, pData->usDay,
	// 	pData->usHour, pData->usMin, pData->usSecond);

	// PV组当日发电量 无
	// PV组总共发电量 无
	return TRUE;
}

BOOL parseGinlongData_0x04_3099(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	pData->iRatedPower = getTwoBytes(3109, iStartAddr, pRcvBuf) * 10;	//额定功率(本机标定视在功率)
	// printf("[锦浪]额定采样功率：[%d] KW\n", pData->iRatedPower);

}

BOOL parseGinlongData_0x04_3279(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	// printf("parseGinlongData_0x04_1 called. iStartAddr = [%d]\n", iStartAddr);
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;
	int i;
	
	// iTypeOfDCInput = 5;	// 测试用，强制为 5

	if (iTypeOfDCInput >= 4)
	{
		// PV电压对应几路PV电流
		int nPVGroupNum = getTwoBytes(3286, iStartAddr, pRcvBuf);
		// printf("nPVGroupNum = [%d]\n", nPVGroupNum);
		// PV 电压
		for (i = 0; i < 16; ++i)
		{
			pData->fPVGroupVolt[i] = getTwoBytes(3320 + i, iStartAddr, pRcvBuf) * 0.1;
			// printf("PV Group [%d]: Volt = [%.1f]\n", i + 1, pData->fPVGroupVolt[i]);
		}
		// PV 电流 协议数据只有20个连续的pv电流，需要分开取
		for (i = 0; i < 20; ++i)
		{
			pData->fPVGroupCurr[i] = getTwoBytes(3300 + i, iStartAddr, pRcvBuf) * 0.1;
			// printf("PV Group [%d]: Curr = [%.1f]\n", i + 1, pData->fPVGroupCurr[i]);
		}
		for (i = 0; i < 10; ++i)
		{
			pData->fPVGroupCurr[20 + i] = getTwoBytes(3288 + i, iStartAddr, pRcvBuf) * 0.1;
			// printf("PV Group [%d]: Curr = [%.1f]\n", i + 1, pData->fPVGroupCurr[i]);
		}
		for (i = 0; i < 2; ++i)
		{
			pData->fPVGroupCurr[30 + i] = getTwoBytes(3336 + i, iStartAddr, pRcvBuf) * 0.1;
			// printf("PV Group [%d]: Curr = [%.1f]\n", i + 1, pData->fPVGroupCurr[i]);
		}
	}

	return TRUE;
}



static MODBUS_REQUEST_INFO modbusRtuReqInfo[] = 
{
	// 协议规定地址与实际信息帧中的地址不同，需要向上偏移一位，比如 3000 地址偏移后是 2999
	{0x4, 2999, 100, parseGinlongData_0x04_2999},
	{0x4, 3099, 50, parseGinlongData_0x04_3099},
	{0x4, 3279, 80, parseGinlongData_0x04_3279}
};

int samplePVInverterGinlong(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich)
{
	// printf("sample[Ginlong] called.------------------\n");
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
				pPortInfo->hPort, pPVOutData);
		static int iCommFailCount[MAX_PVInverter_NUM] = {0};
		if (!bRet)
		{
			// pPvInverterSample->bCommStatusOk = FALSE;
			iCommFailCount[inwhich]++;
			IsSamplerValid = FALSE;
			// printf("PV Inverter [Ginlang] Sample Failed[%d].\n");
			if (iCommFailCount[inwhich] > 5)
			{
				pPvInverterSample->stPVRunData[inwhich].bIsCommFail = TRUE;
				continue;
			} 
		}
		else
		{
			iCommFailCount[inwhich] = 0;
			pPvInverterSample->stPVRunData[inwhich].bIsCommFail = FALSE; 
		}
		Sleep(300);
	}

	return IsSamplerValid;
}

BOOL setPVCmdGinlong(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich)
{
	BOOL bRet = FALSE;
	// SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;

	PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
	PORT_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);

	Sleep(700);	// 控制报文之间延时700ms
	switch (eCmd)
	{
		case PV_CMD_PowerOn:
		{
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3006, 0xBE, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		case PV_CMD_PowerOff:
		{
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3006, 0xDE, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		case PV_CMD_ActiveOutputPower:
		{
			// 先开启限功率开关, 3069 写入 0xAA 启用限功率开关
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3069, 0xAA, pPortInfo->pCommPort, pPortInfo->hPort);
			// printf("bRet1 = [%d]\n", bRet);
			if (!bRet)
			{
				return FALSE;
			}
			Sleep(700);	// 控制报文之间延时700ms

			// 设置限功率值 3051 写入实际功率百分比
			int iPowerPercentage = usCmdData[0] * 10000 / pPvInverterSample->stPVSettingParam[inwhich].iRatedPower;
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3051, iPowerPercentage, pPortInfo->pCommPort, pPortInfo->hPort);
			// printf("bRet2 = [%d]\n", bRet);
			break;
		}
		case PV_CMD_TIMESYSCTR:
		{
			// usCmdData[0] = usCmdData[0] % 100; //ginlong协议 年 取后两位
			// bRet = writeModbusRtuMultipleRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3073, 6, usCmdData, pPortInfo->pCommPort, pPortInfo->hPort);
			// //TRACED("----PV_CMD_TIMESYSCTR-----bRet: %d", bRet);	
			break;
		}
		case PV_CMD_ActivePowerSlope:
		{
			int iSlope = usCmdData[0]; // 0-100 代表 0%-100%
			printf("iSlope = [%d]\n", iSlope);
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3156, iSlope, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		case PV_CMD_PoweroffSave:
		{
			int enable = usCmdData[0]; // 0-不使能 1-使能
			printf("enable = [%d]\n", enable);
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3068, enable, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		default:
		{
			return FALSE;
		}

	}
	return bRet;
}
