#include "pv_inverter_main.h"
#include "pv_modbus_rtu.h"

BOOL parseHuaweiData_0x03_30015(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;
	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	getStringFromRegs(30015, iStartAddr, 10, pRcvBuf, pData->sn, sizeof(pData->sn));  					//30015 10 SN
	getStringFromRegs(30025, iStartAddr, 10, pRcvBuf, pData->szFwVer, sizeof(pData->szFwVer));  		//30025 10 PN
	getStringFromRegs(30050, iStartAddr, 15, pRcvBuf, pData->szCtrlVer, sizeof(pData->szCtrlVer));  	//30050 15 软件版本
	getStringFromRegs(30068, iStartAddr, 2,  pRcvBuf, pData->szModbusVer, sizeof(pData->szModbusVer));  //30068 2 Modbus协议版本

	pData->iRatedPower	= getFourBytes(30073, iStartAddr, pRcvBuf);		//额定功率 KW
	// printf("[华为]额定采样功率：[%d] KW\n", pData->iRatedPower);

	return TRUE;
}

BOOL parseHuaweiData_0x03_32016(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;
	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;
	
	int i;
	for(i = 0; i < MAX_PV_NUM; ++i)
	{
		pData->fPVGroupVolt[i] = getTwoBytes(32016 + i * 2, iStartAddr, pRcvBuf) * 0.1;	//PV电压  V
		pData->fPVGroupCurr[i] = getTwoBytes(32016 + i * 2 + 1, iStartAddr, pRcvBuf) * 0.01;	//PV电流 A
	}

	pData->fPVInputTotalPower = getFourBytes(32064, iStartAddr, pRcvBuf);	//光伏直流输入总功率 KW
	pData->fLineVoltAB	= getTwoBytes(32066, iStartAddr, pRcvBuf) * 0.1;	//电网线电压AB  V
	pData->fLineVoltBC	= getTwoBytes(32067, iStartAddr, pRcvBuf) * 0.1;	//电网线电压BC  V 
	pData->fLineVoltCA	= getTwoBytes(32068, iStartAddr, pRcvBuf) * 0.1;	//电网线电压CA  V

	pData->fPhaseVoltA	= getTwoBytes(32069, iStartAddr, pRcvBuf) * 0.1;	//电网相电压A   V
	pData->fPhaseVoltB	= getTwoBytes(32070, iStartAddr, pRcvBuf) * 0.1;	//电网相电压B   V
	pData->fPhaseVoltC	= getTwoBytes(32071, iStartAddr, pRcvBuf) * 0.1;	//电网相电压C   V

	pData->fPhaseCurrA	= getFourBytes(32072, iStartAddr, pRcvBuf) * 0.001;	//电网相电流A   A
	pData->fPhaseCurrB	= getFourBytes(32074, iStartAddr, pRcvBuf) * 0.001;	//电网相电流B   A
	pData->fPhaseCurrC	= getFourBytes(32076, iStartAddr, pRcvBuf) * 0.001;	//电网相电流C   A

	pData->fOutputActivePower = getFourBytes(32080, iStartAddr, pRcvBuf);	//有功功率 KW

	pData->iOutputPowerFactor = getTwoBytes(32084, iStartAddr, pRcvBuf) * 0.001;	//逆变器当前输出功率因数 

	pData->fFreq = getTwoBytes(32085, iStartAddr, pRcvBuf) * 0.01;	//电网频率  

	pData->iRealPowerPercent = getTwoBytes(32086, iStartAddr, pRcvBuf) * 0.01;  //逆变器效率!

	pData->fInnerTemperature	= getTwoBytes(32087, iStartAddr, pRcvBuf) * 0.1;	//内部温度

/*
0x0000 待机：初始化 0x0001 待机：绝缘阻抗检测 0x0002 待机：光照检测 0x0003 待机：电网检测
0x0100 启动
0x0200 并网（离网模式：运行）
0x0201 并网:限功率（离网模式：运行:限功率）
0x0202 并网:自降额（离网模式：运行:自降额）
0x0203 离网运行
0x0300 关机：异常关机 0x0301 关机：指令关机 0x0302 关机： OVGR 0x0303 关机：通信断链 0x0304 关机：限功率 0x0305 关机：需手动开机
0x0306 关机：直流开关断开 0x0307 关机：快速关断 0x0308 关机：输入欠功率
0x0401 电网调度： cosϕ-P曲线 0x0402 电网调度： Q-U 曲线 0x0403 电网调度： PF-U曲线 0x0404 电网调度：干接点 0x0405 电网调度： Q-P曲线
0x0500 点检就绪 0x0501 点检中 0x0600 巡检中 0X0700 AFCI自检 0X0800 IV扫描中 0X0900 直流输入检测 0X0A00 运行：脱网充电
0xA000 待机：无光照
*/
	UINT PvStatusTemp = getTwoBytes(32089, iStartAddr, pRcvBuf);	//32089 1 设备状态
	if(PvStatusTemp >= 0x0200 && PvStatusTemp <= 0x0202)
	{
		pData->iWorkMode = PV_STATUS_RUNNING;
		pData->nOnOffStatus = 1;
	}
	else if ((PvStatusTemp >= 0x0300 && PvStatusTemp <= 0x0308) || (PvStatusTemp == 0x0203))
	{
		pData->iWorkMode = PV_STATUS_FAULT;
		pData->nOnOffStatus = 0;
	}
	else
	{
		pData->iWorkMode = PV_STATUS_WAITING;
		pData->nOnOffStatus = 1;
	}

	pData->uErrCode	= getTwoBytes(32090, iStartAddr, pRcvBuf);	//32090 1 故障码

	UINT uTimeTemp = getFourBytes(32091, iStartAddr, pRcvBuf); 	//32091 2 开机时间 纪元秒
	time_t t = (time_t)uTimeTemp;
	struct tm * tm_time = localtime(&t);
	
	pData->usYear = tm_time->tm_year + 1900;
	pData->usMonth = tm_time->tm_mon + 1;
	pData->usDay = tm_time->tm_mday;
	pData->usHour = tm_time->tm_hour;
	pData->usMin = tm_time->tm_min;
	pData->usSecond = tm_time->tm_sec;
    
	pData->fTotalPowerGeneration = getFourBytes(32106, iStartAddr, pRcvBuf) * 0.01;	//总共发电量 kWh
	pData->fPVDCTotalPowerGeneration = getFourBytes(32108, iStartAddr, pRcvBuf) * 0.01;	//32108 1 直流输入总电量kwh
	pData->fDailyPowerGeneration = getFourBytes(32114, iStartAddr, pRcvBuf) * 0.01;	//当日发电量 kWh
	

	return TRUE;
}

BOOL parseHuaweiData_0x03_40125(int iStartAddr, unsigned char* pRcvBuf, void* pOutData)
{
	if (NULL == pRcvBuf || NULL == pOutData)
		return FALSE;
	PV_INVERTER_DATA* pData = (PV_INVERTER_DATA*)pOutData;

	pData->nParallelAntiBackflowPowerLimit = getTwoBytes(40125, iStartAddr, pRcvBuf) * 0.1;  //有功功率百分比降额(低精度)

	return TRUE;
}

static MODBUS_REQUEST_INFO modbusRtuReqInfo[] = 
{
	{0x3, 30015, 80, parseHuaweiData_0x03_30015},
	{0x3, 32016, 110, parseHuaweiData_0x03_32016},
	{0x3, 40125, 1, parseHuaweiData_0x03_40125}
};


//modbus rtu
int samplePVInverterHuawei(ST_PV_ROUGHDATA* pPvInverterSample, PV_INVERTER_DATA* pPVOutData, UINT inwhich)
{
	// printf("sample[Huawei] called.------------------\n");
	int i = 0;
	BOOL bRet = TRUE;
	// SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;

	// 获取该设备对应的端口
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
			iCommFailCount[inwhich]++;
			IsSamplerValid = FALSE;
			// printf("PV Inverter [Huawei][%d] Sample Failed.\n", inwhich);
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
		Sleep(100);
	}
	// printf("sample[Huawei][%d]bIsCommFail = [%d]\n", inwhich, pPvInverterSample->stPVRunData[inwhich].bIsCommFail);

	return IsSamplerValid;
}



BOOL setPVCmdHuawei(ST_PV_ROUGHDATA* pPvInverterSample, ENU_PV_CMD eCmd, SHORT* usCmdData, UINT inwhich)
{
	BOOL bRet = FALSE;
	// SAMPLER* pSampler = (SAMPLER*)pPvInverterSample->hSampler;
	// int rated = pPvInverterSample->stPVSettingParam[inwhich].iRatedPower; //KW
	// 获取该设备对应的端口
    PHY_INTER_TYPE ePortType = pPvInverterSample->stPVSettingParam[inwhich].ePhyInterface;
	PORT_INFO* pPortInfo = Get_Port_By_Type(pPvInverterSample, ePortType);

	if (pPortInfo == NULL || !pPortInfo->bIsOpened)
    {
        TRACEE("Port not available for Growatt command to inverter[%d]\n", inwhich);
        return FALSE;
    }

	int rated = pPvInverterSample->blobPVTotalinfo.iTotalRatedPower;
	int percent = 0;

	// printf("setPVCmdHuawei[%d]cmd[%d] =  [%d]\n", inwhich, eCmd, *usCmdData);
	switch (eCmd)
	{
		case PV_CMD_PowerOn:
		{
			TRACED("cmd :PV_CMD_PowerOn\n");
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 40200, 1, pPortInfo->pCommPort, pPortInfo->hPort);
			TRACED("PV_CMD_PowerOn :bRet = %d",bRet);
			break;
		}
		case PV_CMD_PowerOff:
		{
			TRACED("cmd :PV_CMD_PowerOff\n");
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 40201, 1, pPortInfo->pCommPort, pPortInfo->hPort);
			TRACED("PV_CMD_PowerOff :bRet = %d",bRet);
			break;
		}
		case PV_CMD_ActiveOutputPower:  //*10 下发
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
		
			TRACEW("PV_CMD_OutputPower-- OutputPower=%hd RatePower=%d iPowerPercentage=%d\n",curActPower,rated,percent);
			bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 40125, percent * 10, pPortInfo->pCommPort, pPortInfo->hPort);
			break;
		}
		case PV_CMD_ActiveOutputPower_NoLimit:  //*10 下发
		{
			TRACEW("PV_CMD_ActiveOutputPower_NoLimit \n");
			if (pPvInverterSample->stPVRunData[inwhich].blobSinglePVdata.PVInverterData.nParallelAntiBackflowPowerLimit == 100)
			{
				bRet = TRUE;
			}
			else
			{
				percent = 100;
				bRet = writeModbusRtuSignalRegister(pPvInverterSample->stPVSettingParam[inwhich].iAddr, 40125, percent * 10, pPortInfo->pCommPort, pPortInfo->hPort);
			}
			break;
		}
		default:
		{
			return FALSE;
		}

	}
	// printf("setPVCmdHuawei[%d]cmd[%d] bRet = [%d]\n", inwhich, eCmd, bRet);

	return bRet;
}