// #define _TRACE_PRINT_
#include "pv_inverter_main.h"
#include "pv_modbus_rtu.h"


BOOL parseGrowattData_0x04_0(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	UINT PvStatusTemp = getTwoBytes(0, iStartAddr, pRcvBuf);	//逆变器运行状态 0x04 0   //工作状态 0-等待，1-运行 2-预留 3-故障
	if(PvStatusTemp == 0)
	{
		pData->iWorkMode = PV_STATUS_WAITING;
	}
	else if (PvStatusTemp == 1)
	{
		pData->iWorkMode = PV_STATUS_RUNNING;
	}
	else
	{
		pData->iWorkMode = PV_STATUS_FAULT;
	}
						
	USHORT usMainErrcode = getTwoBytes(105, iStartAddr, pRcvBuf); //故障码 0x04 105 107
	USHORT usSubErrcode = getTwoBytes(107, iStartAddr, pRcvBuf);

	pData->uErrCode = usMainErrcode << 16 | usSubErrcode;

	USHORT usMainWarncode = pRcvBuf[112 * 2]; //告警码 0x04 112 111
	USHORT usSubWarncode = getTwoBytes(111, iStartAddr, pRcvBuf);
	pData->uWarnCode = usMainWarncode << 16 | usSubWarncode;

	pData->fLineVoltAB	= getTwoBytes(50, iStartAddr, pRcvBuf) * 0.1;	//电网线电压AB  0x04 50
	pData->fLineVoltBC	= getTwoBytes(51, iStartAddr, pRcvBuf) * 0.1;	//电网线电压BC  0x04 51
	pData->fLineVoltCA	= getTwoBytes(52, iStartAddr, pRcvBuf) * 0.1;	//电网线电压CA  0x04 52

	pData->fPhaseVoltA	= getTwoBytes(38, iStartAddr, pRcvBuf) * 0.1;	//电网相电压A  0x04 38
	pData->fPhaseVoltB	= getTwoBytes(42, iStartAddr, pRcvBuf) * 0.1;	//电网相电压B  0x04 42
	pData->fPhaseVoltC	= getTwoBytes(46, iStartAddr, pRcvBuf) * 0.1;	//电网相电压C  0x04 46

	pData->fPhaseCurrA	= getTwoBytes(39, iStartAddr, pRcvBuf) * 0.1;	//电网相电流A  0x04 39
	pData->fPhaseCurrB	= getTwoBytes(43, iStartAddr, pRcvBuf) * 0.1;	//电网相电流B  0x04 43
	pData->fPhaseCurrC	= getTwoBytes(47, iStartAddr, pRcvBuf) * 0.1;	//电网相电流C  0x04 47
   
	pData->fFreq = getTwoBytes(37, iStartAddr, pRcvBuf) * 0.01;	//电网频率  0x04 37
	pData->fOutputActivePower = getFourBytes(35, iStartAddr, pRcvBuf) * 0.1;	//逆变器当前输出交流有功功率 W  0x04 35
    pData->iOutputPowerFactor = getTwoBytes(100, iStartAddr, pRcvBuf);	//逆变器当前输出功率因数   0x04 100
    
    pData->fPVInputTotalPower = getFourBytes(1, iStartAddr, pRcvBuf) * 0.1;	//光伏直流输入总功率 W  W  0x04 1 2
    
	pData->fInnerTemperature	= getTwoBytes(94, iStartAddr, pRcvBuf) * 0.1;	//内部温度  0x04 94

	pData->iRealPowerPercent = getTwoBytes(101, iStartAddr, pRcvBuf);			//实际功率百分比  0x04 101
	
	pData->fDailyPowerGeneration = getFourBytes(53, iStartAddr, pRcvBuf) * 0.1;	//当日发电量 kWh  0x04 53 54
	pData->fTotalPowerGeneration = getFourBytes(55, iStartAddr, pRcvBuf) * 0.1;	//总共发电量 kWh  0x04 55 56
	
	int nPvGrpId = 0;
	for(nPvGrpId; nPvGrpId < 8; ++nPvGrpId)
	{
		pData->fPVGroupVolt[nPvGrpId] = getTwoBytes(3 + nPvGrpId * 4, iStartAddr, pRcvBuf) * 0.1;	//PV1电压 V  0x04 3
		pData->fPVGroupCurr[nPvGrpId] = getTwoBytes(4 + nPvGrpId * 4, iStartAddr, pRcvBuf) * 0.1;	//PV1电流

		pData->fPvGrpDailyPowerGeneration[nPvGrpId] = getFourBytes(59 + nPvGrpId * 4, iStartAddr, pRcvBuf) * 0.1;// PV组当日发电量 kWh
		pData->fPvGrpTotalPowerGeneration[nPvGrpId] = getFourBytes(61 + nPvGrpId * 4, iStartAddr, pRcvBuf) * 0.1;// PV组总共发电量 kWh
	}

	pData->fPVDCTotalPowerGeneration = getFourBytes(91, iStartAddr, pRcvBuf) * 0.1;	//光伏直流输入总电量kWh  0x04 91 92

	return TRUE;
}

BOOL parseGrowattData_0x03_0(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	pData->nOnOffStatus = getTwoBytes(0, iStartAddr, pRcvBuf);	//0x03 0  开关机状态
	
	int i = 0;
	int iSnIdx = 23 * 2;
	for (i = 0; i < 10; ++i)
	{
		pData->sn[i] = pRcvBuf[iSnIdx++];		//0x03 23 序列号
	}

	memset(pData->szFwVer, 0, sizeof(pData->szFwVer));	//0x03 9 固件版本
	snprintf(pData->szFwVer, MAX_VERSION_LEN, "%c%c%c%c%c%c", pRcvBuf[18], pRcvBuf[19], pRcvBuf[20], pRcvBuf[21], pRcvBuf[22], pRcvBuf[23]);

	memset(pData->szCtrlVer, 0, sizeof(pData->szCtrlVer));	//0x03 12 控制固件版本
	snprintf(pData->szCtrlVer, MAX_VERSION_LEN, "%c%c%c%c.%u", pRcvBuf[24], pRcvBuf[25], pRcvBuf[26], pRcvBuf[27],  getTwoBytes(14, iStartAddr, pRcvBuf));

	USHORT usModbusVer = getTwoBytes(88, iStartAddr, pRcvBuf);	//0x03 88 Modbus版本
	memset(pData->szModbusVer, 0, sizeof(pData->szModbusVer));
	snprintf(pData->szModbusVer, MAX_VERSION_LEN, "V%u.%02u", usModbusVer / 100, usModbusVer % 100);
	
	pData->iRatedPower = getFourBytes(6, iStartAddr, pRcvBuf) * 0.1;	//0x03 6 7 额定功率
	
	pData->iActivePowerRate = getTwoBytes(3, iStartAddr, pRcvBuf);	//最大输出有功功率百分比  0x03 3 [0-100] 255功率不被限制
	pData->nReActivePowerPercen = getTwoBytes(4, iStartAddr, pRcvBuf);	//最大输出有功功率百分比  0x03 4 [-100-100] 255功率不被限制

	pData->usYear = getTwoBytes(45, iStartAddr, pRcvBuf);	//0x03 45-50 系统时间 年-月-日-时-分-秒
	pData->usMonth = getTwoBytes(46, iStartAddr, pRcvBuf);
	pData->usDay = getTwoBytes(47, iStartAddr, pRcvBuf);
	pData->usHour = getTwoBytes(48, iStartAddr, pRcvBuf);
	pData->usMin = getTwoBytes(49, iStartAddr, pRcvBuf);
	pData->usSecond = getTwoBytes(50, iStartAddr, pRcvBuf);

	return TRUE;
}


BOOL parseGrowattData_0x04_875(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)  //仅MAX 100-125KTL3-X LV 机型支持
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	int i = 0;
	for (i = 0; i < 8; ++i)
	{
		pData->fPVGroupVolt[i + 8] = getTwoBytes(875 + i * 4, iStartAddr, pRcvBuf) * 0.1; //PV9 - PV16  电压
		pData->fPVGroupCurr[i + 8] = getTwoBytes(876 + i * 4, iStartAddr, pRcvBuf) * 0.1; //PV9 - PV16 电流
		
		pData->fPvGrpDailyPowerGeneration[i + 8] = getFourBytes(907 + i * 4, iStartAddr, pRcvBuf) * 0.1;// PV9 - PV16 PV组当日发电量 kWh
		pData->fPvGrpTotalPowerGeneration[i + 8] = getFourBytes(909 + i * 4, iStartAddr, pRcvBuf) * 0.1;// PV9 - PV16 PV组总共发电量 kWh
	}
	return TRUE;
}

BOOL parseGrowattData_0x03_306(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;

	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	pData->nParallelAntiBackflowEnable = getTwoBytes(306, iStartAddr, pRcvBuf);
	pData->nAntiBackflowFailureResponseTime = getTwoBytes(307, iStartAddr, pRcvBuf);
	pData->nParallelAntiBackflowPowerLimit = getTwoBytes(308, iStartAddr, pRcvBuf) * 0.1;

	return TRUE;
}


static MODBUS_REQUEST_INFO modbusRtuReqInfo[] = 
{
	{0x4, 0, 115, parseGrowattData_0x04_0},
	{0x3, 0, 90, parseGrowattData_0x03_0},
	{0x4, 875, 70, parseGrowattData_0x04_875},
	{0x3, 306, 3, parseGrowattData_0x03_306}
};

//modbus rtu
int samplePVInverterGrowatt(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich)
{
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
		// TRACED("samplePVInverterGrowatt[%d] Ret=[%d]\n",inwhich,bRet);
		static int iCommFailCount[MAX_PVInverter_NUM] = {0};
		if (!bRet)
		{
			iCommFailCount[inwhich]++;
			IsSamplerValid = FALSE;
			// printf("PV Inverter [Growatt] Sample Failed[%d].\n");
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
	}

	return IsSamplerValid;
}


BOOL setPVCmdGrowatt(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich)
{
	BOOL bRet = FALSE;
	SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;
	int rated = pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.iRatedPower * 0.001; //KW
	int percent = 0;

	PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
	PORT_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);

	switch (eCmd)
	{
		case PV_CMD_PowerOn:
		{
			TRACEW("PV_CMD_PowerOn..\n");
			if (pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.nOnOffStatus == 1)
			{
				bRet = TRUE;
			}
			else
			{
				bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 0, 1, pPortInfo->pCommPort, pPortInfo->hPort);
			}
			break;
		}
		case PV_CMD_PowerOff:
		{
			TRACEW("PV_CMD_PowerOff..\n");
			if (pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.nOnOffStatus == 0)
			{
				bRet = TRUE;
			}
			else
			{
				bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 0, 0, pPortInfo->pCommPort, pPortInfo->hPort);
			}
			break;
		}
		case PV_CMD_MaxActivePowerRate://0-100  0x03
		{
			if (rated <= 0)
			{
				return FALSE;
			}

			SHORT curActPower = *usCmdData;
			percent = (int)((float)curActPower * 100.0f / rated); 		

			if (percent > 100)
			{
				percent = 100;
			}
		
			TRACEW("PV_CMD_MaxActivePowerRate-- OutputPower=%hd RatePower=%d iPowerPercentage=%d\n",curActPower,rated,percent);
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 3, percent, pPortInfo->pCommPort, pPortInfo->hPort);//0-100或255
			break;
		}
		case PV_CMD_AntiFlux_Enable:
		{
			SHORT shEnable = *usCmdData;
			TRACEW("PV_CMD_AntiFlux_Enable.. shEnable[%hd] nParallelAntiBackflowEnable[%d]\n",shEnable,pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.nParallelAntiBackflowEnable);
			if (pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.nParallelAntiBackflowEnable == shEnable)
			{
				bRet = TRUE;
			}
			else
			{
				bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 306, shEnable, pPortInfo->pCommPort, pPortInfo->hPort);
			}
			break;
		}
		case PV_CMD_ActiveOutputPower:
		{
			if (rated <= 0)
			{
				LOGFD("pv pv_inverter_main:setPVCmdGrowatt return FALSE PV_CMD_ActiveOutputPower rated:%d", rated);
				return FALSE;
			}

			SHORT curActPower = *usCmdData;
			percent = (int)((float)curActPower * 100.0f / rated); 		

			if (percent > 100)
			{
				percent = 100;
			}
		
			TRACEW("PV_CMD_OutputPower-- OutputPower=%hd RatePower=%d iPowerPercentage=%d\n",curActPower,rated,percent);
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 308, percent * 10, pPortInfo->pCommPort, pPortInfo->hPort); //0-1000
			break;
		}
		case PV_CMD_ActiveOutputPower_NoLimit:
		{
			TRACEW("PV_CMD_ActiveOutputPower_NoLimit \n");
			if (pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.nParallelAntiBackflowPowerLimit == 100)
			{
				bRet = TRUE;
			}
			else
			{
				percent = 100;
				bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 308, percent * 10, pPortInfo->pCommPort, pPortInfo->hPort);//0-1000
			}
			break;
		}
		case PV_CMD_ReActiveOutputPower://-100-100
		{
			if (rated <= 0)
			{
				return FALSE;
			}
				
			SHORT curReActPower = *usCmdData;
			percent = (int)((float)curReActPower * 100.0f / rated);

			if (percent > 100) 
			{
				percent = 100;
			}

			if (percent < -100)
			{
				percent = -100;
			}
				
			TRACEW("PV_CMD_ReActiveOutputPower-- ReOutputPower=%hd RatePower=%d iPowerPercentage=%d\n",curReActPower,rated,percent);
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 4, percent, pPortInfo->pCommPort, pPortInfo->hPort);

			break;
		}
		case PV_CMD_ReActiveOutputPower_NoLimit://-100-100
		{
			percent = 100;
			TRACEW("PV_CMD_ReActiveOutputPower_NoLimit \n");
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 4, percent, pPortInfo->pCommPort, pPortInfo->hPort);

			break;
		}
		case PV_CMD_TIMESYSCTR:
		{
			
			bRet = writeModbusRtuMultipleRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 45, 6, usCmdData, pPortInfo->pCommPort, pPortInfo->hPort);
			TRACEW("----PV_CMD_TIMESYSCTR-----bRet: %d", bRet);		
			break;
		}
		default:
		{
			return FALSE;
		}

	}
	return bRet;
}
