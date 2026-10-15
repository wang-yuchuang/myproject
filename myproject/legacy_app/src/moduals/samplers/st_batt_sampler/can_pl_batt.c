#include "st_batt_sampler.h"

//-----------------------data definition---------------------
#define BATT_R_ESMB_GRP_42_10			0x10
#define BATT_R_ESMB_GRP_42_20			0x20
#define BATT_R_ESMB_GRP_42_30			0x30
#define BATT_R_ESMB_GRP_42_40		    0x40
#define BATT_R_ESMB_GRP_42_50			0x50
#define BATT_R_ESMB_GRP_42_60			0x60
#define BATT_R_ESMB_GRP_42_70			0x70
#define BATT_R_ESMB_GRP_42_80			0x80
#define BATT_R_ESMB_GRP_42_90			0x90
#define BATT_R_SYS_EQUIP_73_10          0x10
#define BATT_R_SYS_EQUIP_73_20          0x20

typedef union tagBattFaultByte
{
	BYTE	byFlag;
	struct
	{
		BYTE	VoltSensorError : 1;
		BYTE	TempSensorError : 1;
		BYTE	InternalCommError : 1;
		BYTE	InputOverVoltError : 1;
		BYTE	InputTranspositionError : 1;
		BYTE	RelayCheckError : 1;
		BYTE	BattCellError : 1;
		BYTE	OtherError : 1;
	}Fault;
}BATT_FAULT_MAP;

typedef union tagBattFaultByte_Ext
{
	BYTE	byFlag;
	struct
	{
		BYTE	ShutdownCircuitError : 1;
		BYTE	BMIC_Error : 1;
		BYTE	Internal_BusError : 1;
		BYTE	SelfTest_Error : 1;
	}Fault;
}BATT_FAULT_MAP_EXT;

typedef union tagBattAlarmUShort
{
	USHORT	usFlag;
	struct
	{
		USHORT	SingleCell_LowVolt : 1;
		USHORT	SingleCell_HighVolt : 1;
		USHORT	Discharge_LowVolt : 1;
		USHORT	Charge_HighVolt : 1;
		USHORT	Charge_LowTemp : 1;
		USHORT	Charge_HighTemp : 1;
		USHORT	Discharge_LowTemp : 1;
		USHORT	Discharge_HighTemp : 1;
		USHORT	Charge_OverCurr : 1;
		USHORT	Discharge_OverCurr : 1;
        USHORT	Module_LowVolt : 1;
        USHORT	Module_HighVolt : 1;
	}Alarm;
}BATT_ALARM_MAP;

typedef union tagBattProtectUShort
{
	USHORT	usFlag;
	struct
	{
		USHORT	SingleCell_UnderVolt : 1;
		USHORT	SingleCell_OverVolt : 1;
		USHORT	Discharge_UnderVolt : 1;
		USHORT	Charge_OverVolt : 1;
		USHORT	Charge_UnderTemp : 1;
		USHORT	Charge_OverTemp : 1;
		USHORT	Discharge_UnderTemp : 1;
		USHORT	Discharge_OverTemp : 1;
		USHORT	Charge_OverCurrProtect : 1;
		USHORT	Discharge_OverCurrProtect : 1;
        USHORT	Module_UnderVolt : 1;
        USHORT	Module_OverVolt : 1;
	}Alarm;
}BATT_PROTECT_MAP;

typedef enum tagstBattCmd_Idx
{ 
	_CAN_R_CMD_G_4200_0 = 0,    /* Ensemble information          */
	_CAN_R_CMD_G_4200_2,        /* system equipment information  */
	
    _CAN_R_CMD_S_8200,          /* Sleep / Awake Command     */
	_CAN_R_CMD_S_8210,          /* Charge/Discharge Command  */
    _CAN_R_CMD_S_8400,          /* Temporary masking “external comm error” command */
 
	_MAX_CAN_R_CMD
}ID_CMD_BATT;

typedef struct tagAPLBattFrame
{
	BYTE byRawData[CAN_FRAME_LEN];

	/*CAN Protocal Head Info,Total 29 bits,extend frame*/
	BYTE	byErrCode;
	BYTE	byResv1;
	BYTE	byResv2;
	BYTE	byCmdType;
	BYTE	bySubCmd;
    UINT     nGrpIdx;          //0~F,Max 16
	
	BYTE    byData[CAN_DATA_AREA_LEN];      /* Data Area */

	BOOL    bIsCtrlCmd;     /* if true, need send urgently */

}A_CAN_BATT_FRAME;

#define MKAE_PL_BATT_FRAME(pFrame, _byCmdType, _bySubCmd, _nGrpIdx, _bIsCtrlCmd) \
		((pFrame)->byCmdType = (_byCmdType), \
		(pFrame)->bySubCmd = (_bySubCmd), \
		(pFrame)->nGrpIdx = (_nGrpIdx), \
		(pFrame)->bIsCtrlCmd = (_bIsCtrlCmd))

typedef ERR_CODE_CAN_R (*CAN_BATT_SAMPLE_PROC)( int nIdx, void* pRawData, void* pSetData);
typedef struct tagACANFrame
{
	ID_CMD_BATT nCmdType;

	CAN_BATT_SAMPLE_PROC pfnCmdHandler;

}CAN_BATT_FRAME_INFO;

typedef struct tagPLBattRunData
{
    //from 0x4210
    float               fPackTotalVolt;
    float               fPackTotalCurr;
    float               fMainControllerTemperature;
    BYTE                bySOC;
    BYTE                bySOH;
    //from 0x4220
    float               fCharge_Cutoff_Volt;
    float               fDisCharge_Cutoff_Volt;
    float               fAllow_MaxChargeCurr;
    float               fAllow_MaxDisChargeCurr;
    //from 0x4230
    float               fMax_CellVolt;
    float               fMin_CellVolt;
    USHORT              usMaxCellVolt_Idx;
    USHORT              usMinCellVolt_Idx;
    //from 0x4240
    float               fMax_CellTemp;
    float               fMin_CellTemp;
    USHORT              usMaxCellTemp_Idx;
    USHORT              usMinCellTemp_Idx;
    //from 0x4250
    BYTE                byBattRunStatus; //0: Sleep, 1: Charge, 2: Discharge, 3: Idle
        //following 2 bites are from 
    BOOL                bNeedBalanceCharge;
    BOOL                bNeedForceCharge;
    USHORT              usCyclePeriod;
         //fault/alarm/protect
    BATT_FAULT_MAP      mapFault;
    BATT_ALARM_MAP      mapAlarm;
    BATT_PROTECT_MAP    mapProtect;
    //from 0x4280
    BOOL                bIsBatt_ChargeForbidden;
    BOOL                bIsBatt_DisChargeForbidden;
    //from 0x4290
    BATT_FAULT_MAP_EXT  mapFault_Ext;

    //from 0x7310
    BYTE                byHW_Ver;
    BYTE                byHW_V_Ver;
    BYTE                byHW_R_Ver;
    USHORT              usSW_V_Ver;
    USHORT              usSW_Ver;
    //from 0x7320
    USHORT              usBattQty;
    USHORT              usBattModuleQty;
    USHORT              usCellInModuleQty;
    float               fVoltageLevel;
    float               fBatt_AH;

    int                 bFoundReply;
	int			        nInterrupt_Times;
	BOOL		        bIsCommFail;
    BOOL                bBattInfo_Regisited; //if the battGrp info NOT registered, should send 4200+2

    // float               fCalc_MaxDischargeCurr; //this is calculated max discharge  current,for POWER, can multi the packTocolVolt;
    // //calc total charge/discharge energy 
    // double               fCalc_ChargedEnergy;
    // double               fCalc_DisChargedEnergy;
    
    // double               fLast_ChrgedEnergyReading;
    // double               fLast_DisChrgedEnergyReading;
}PL_BATT_RUN_DATA;
//-----------------end of data definition------------------

#define  INIT_PLBATT_CMD(p, _cmdType, _pfnHandler) \
	((p)->nCmdType = (_cmdType), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)++);


static PL_BATT_RUN_DATA     s_pl_BattRunData[MAX_SINGLE_STBATT_NUM]; //max 6 grp of batt
static CAN_BATT_FRAME_INFO	s_cmdInfos[_MAX_CAN_R_CMD];


/*********************************************************************************
*  
*  FUNCTION NAME : Do_CanSession
*  CREATOR       :  
*  CREATED DATE  : 5019-7-25 09:24:23
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static ERR_CODE_CAN_R Do_CanSession(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, CAN_BATT_SAMPLE_PROC pfnHandler, int nIdx, BOOL bIsSetCmd, void* pSetData)
{
	if(bIsSetCmd)//Ctrl Cmd, Must be Locked//
	{
		Mutex_Lock(pSTBatt_Rough->hSamplerLock, WAIT_INFINITE);
	}
	else
	{
		if (Mutex_Lock(pSTBatt_Rough->hSamplerLock, MAX_CAN_SAMPLER_LOCK_WAIT) != ERR_MUTEX_OK)
		{
			return ERR_CAN_R_NO_LOCK;
		}
	}

	ERR_CODE_CAN_R emRet = pfnHandler(nIdx, (void*)pSTBatt_Rough, pSetData);

	Mutex_Unlock(pSTBatt_Rough->hSamplerLock);

	return emRet;
}


static BOOL Judge_IfNeedSample_BattDevInfo(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
    int n;
    BOOL bNeed = FALSE;
    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
		TRACE("->pl_BattRunData[%d].bBattInfo_Regisited =%d ->pl_BattRunData[n].bIsCommFail =%d ->emSysBattInstallType=%d\n",\
			n,
			s_pl_BattRunData[n].bBattInfo_Regisited,
			s_pl_BattRunData[n].bIsCommFail,
			pSTBatt_Rough->emSysBattInstallType);

        if((pSTBatt_Rough->emSysBattInstallType < 1) && (n > 0))
        {
            break;

        }
        if((!s_pl_BattRunData[n].bBattInfo_Regisited) ||
            s_pl_BattRunData[n].bIsCommFail)
        {
            bNeed = TRUE;
            break;
        }
    }
    return bNeed;
}

static BOOL Judge_IfNeedBatt_ReConfig(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	 int n;
    BOOL bNeed = FALSE;
    for(n = 0; n < MAX_SINGLE_STBATT_NUM ; n++)
    {
		TRACE("->pl_BattRunData[%d].bBattInfo_Regisited =%d ->pl_BattRunData[n].bIsCommFail =%d ->nSystemCfgBattNum=%d  ->nTotalInstalledBatt=%d\n",\
			n,
			s_pl_BattRunData[n].bBattInfo_Regisited,
			s_pl_BattRunData[n].bIsCommFail,
			pSTBatt_Rough->nSystemCfgBattNum,
			pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt);

        if(n >= pSTBatt_Rough->nSystemCfgBattNum)
        {
            break;
        }
        if((!s_pl_BattRunData[n].bBattInfo_Regisited) ||
            s_pl_BattRunData[n].bIsCommFail || 
			(pSTBatt_Rough->nSystemCfgBattNum != pSTBatt_Rough->blobGroupBattInfo.nTotalOnlineBatt))
        {
            bNeed = TRUE;
            break;
        }
    }
    return bNeed;
}


/*********************************************************************************
*  
*  FUNCTION NAME : CAN_StringToUShort
*  CREATOR       :  
*  CREATED DATE  : 2019-9-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
USHORT CAN_StringToUShort(BYTE* strInput) //LSB
{
	USHORT usValue = ((USHORT)(strInput[0]))
		+ (((USHORT)(strInput[1])) << 8);

	return usValue;
}

/*********************************************************************************
*  
*  FUNCTION NAME : CAN_StuffHead
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CAN_StuffHead(UINT uiCmdType, 
	UINT uiSubCmd,
	UINT nGrpIdx,
	UINT uiLen,
	BYTE *pbyFrame)
{
	*pbyFrame = (BYTE)uiLen;
	UINT uiFrmID = 0;

	//uiFrmID = (0x00 << 24);
	//uiFrmID += ((UINT)(0x00) << 16);
	uiFrmID += ((UINT)(uiCmdType << 8));
	uiFrmID += ((UINT)(uiSubCmd + nGrpIdx));

	//uiFrmID = (uiFrmID << 3);

	TRACE("--------Ready to Send Frame[%08X]-------\n",uiFrmID);

	*(pbyFrame + 1) = (BYTE)(uiFrmID >> 24);
	*(pbyFrame + 2) = (BYTE)(uiFrmID >> 16);
	*(pbyFrame + 3) = (BYTE)(uiFrmID >> 8);
	*(pbyFrame + 4) = (BYTE)(uiFrmID);

	TRACE("--------- %02x %02x %02x %02x\n",pbyFrame[1],pbyFrame[2],pbyFrame[3],pbyFrame[4]);
	return ;
}

/*********************************************************************************
*  
*  FUNCTION NAME : CAN_ParseHead
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CAN_ParseHead(A_CAN_BATT_FRAME* pFrame)
{
	UINT uiRawHead = 0;
	uiRawHead = ((UINT)pFrame->byRawData[1]) << 24;
	uiRawHead += ((UINT)pFrame->byRawData[2]) << 16;
	uiRawHead += ((UINT)pFrame->byRawData[3]) << 8;
	uiRawHead += ((UINT)pFrame->byRawData[4]);
	//uiRawHead = (uiRawHead >> 3);

	TRACE("---- The Parsed Reply Frame Head: [%08X]----------------\n",uiRawHead);

	pFrame->nGrpIdx = (UINT)(((BYTE)uiRawHead) & 0x0F);
	pFrame->bySubCmd = ((BYTE)uiRawHead) & 0xF0;
	pFrame->byCmdType = (BYTE)(uiRawHead >> 8);
	pFrame->byResv2 = (BYTE)(uiRawHead >> 16);
	pFrame->byResv1 = (BYTE)(uiRawHead >> 24);

	TRACE("->byResv1=%02X ->byResv2=%02X ->byCmdType=%02X ->bySubCmd=%02X ->nGrpIdx=%d\n",\
		pFrame->byResv1,pFrame->byResv2,pFrame->byCmdType,pFrame->bySubCmd,pFrame->nGrpIdx);

	memcpy(pFrame->byData, pFrame->byRawData + 5, CAN_DATA_AREA_LEN);
}

/*********************************************************************************
*  
*  FUNCTION NAME : UnPack_Batt_4200_0_Cmd
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static _ENUM UnPack_Batt_4200_0_Cmd(IN A_CAN_BATT_FRAME *pFrm)
{
	int nGrpIdx = pFrm->nGrpIdx - 1;
	TRACE("Enter UnPack_Batt_4200_0_Cmd nGrpIdx=[%d]\n", nGrpIdx);
	if(nGrpIdx >= MAX_SINGLE_STBATT_NUM || nGrpIdx < 0)
	{
		nGrpIdx = 0;
	}

	s_pl_BattRunData[nGrpIdx].bFoundReply = TRUE;

	switch(pFrm->bySubCmd)
	{
		case BATT_R_ESMB_GRP_42_10:
		{
			TRACE("BATT_R_ESMB_GRP_42_10 !!!\n");

			s_pl_BattRunData[nGrpIdx].fPackTotalVolt = ((float)CAN_StringToUShort(pFrm->byData))*0.1;
			s_pl_BattRunData[nGrpIdx].fPackTotalCurr = ((float)CAN_StringToUShort(pFrm->byData + 2))*0.1 -3000;
			s_pl_BattRunData[nGrpIdx].fMainControllerTemperature = ((float)CAN_StringToUShort(pFrm->byData + 4))*0.1 - 100;
			s_pl_BattRunData[nGrpIdx].bySOC = *(pFrm->byData + 6);
			s_pl_BattRunData[nGrpIdx].bySOH = *(pFrm->byData + 7);

			TRACE("nGrpIdx=[%d] fPackTotalVolt=[%f] fPackTotalCurr=[%f] fMainControllerTemperature=[%f] bySOC=[%d] bySOH=[%d]\n",\
			nGrpIdx,
			s_pl_BattRunData[nGrpIdx].fPackTotalVolt,
			s_pl_BattRunData[nGrpIdx].fPackTotalCurr,
			s_pl_BattRunData[nGrpIdx].fMainControllerTemperature,
			s_pl_BattRunData[nGrpIdx].bySOC,
			s_pl_BattRunData[nGrpIdx].bySOH);

			break;
		}			
 		case BATT_R_ESMB_GRP_42_20:
		{
			TRACE("BATT_R_ESMB_GRP_42_20 !!!\n");
			s_pl_BattRunData[nGrpIdx].fCharge_Cutoff_Volt = ((float)CAN_StringToUShort(pFrm->byData))*0.1;
    		s_pl_BattRunData[nGrpIdx].fDisCharge_Cutoff_Volt = ((float)CAN_StringToUShort(pFrm->byData + 2))*0.1;
    		s_pl_BattRunData[nGrpIdx].fAllow_MaxChargeCurr = ((float)CAN_StringToUShort(pFrm->byData + 4))*0.1 -3000;
    		s_pl_BattRunData[nGrpIdx].fAllow_MaxDisChargeCurr = ((float)CAN_StringToUShort(pFrm->byData + 6))*0.1 -3000;

			TRACE("fCharge_Cutoff_Volt=%f fDisCharge_Cutoff_Volt=%f fAllow_MaxChargeCurr=%f fAllow_MaxDisChargeCurr=%f\n",\
			s_pl_BattRunData[nGrpIdx].fCharge_Cutoff_Volt,
			s_pl_BattRunData[nGrpIdx].fDisCharge_Cutoff_Volt,
			s_pl_BattRunData[nGrpIdx].fAllow_MaxChargeCurr,
			s_pl_BattRunData[nGrpIdx].fAllow_MaxDisChargeCurr);
			break;
		}						
 		case BATT_R_ESMB_GRP_42_30:
		{
			TRACE("BATT_R_ESMB_GRP_42_30 !!!\n");
			s_pl_BattRunData[nGrpIdx].fMax_CellVolt = (float)CAN_StringToUShort(pFrm->byData)*0.001;
    		s_pl_BattRunData[nGrpIdx].fMin_CellVolt = (float)CAN_StringToUShort(pFrm->byData + 2)*0.001;
    		s_pl_BattRunData[nGrpIdx].usMaxCellVolt_Idx = CAN_StringToUShort(pFrm->byData + 4);
    		s_pl_BattRunData[nGrpIdx].usMinCellVolt_Idx = CAN_StringToUShort(pFrm->byData + 6);

			TRACE("fMax_CellVolt=%f fMin_CellVolt=%f usMaxCellVolt_Idx=%d usMinCellVolt_Idx=%d\n",\
			s_pl_BattRunData[nGrpIdx].fMax_CellVolt,
			s_pl_BattRunData[nGrpIdx].fMin_CellVolt,
			s_pl_BattRunData[nGrpIdx].usMaxCellVolt_Idx,
			s_pl_BattRunData[nGrpIdx].usMinCellVolt_Idx);
			break;
		}						
  		case BATT_R_ESMB_GRP_42_40:
		{
			TRACE("BATT_R_ESMB_GRP_42_40 !!!\n");
			s_pl_BattRunData[nGrpIdx].fMax_CellTemp = (float)CAN_StringToUShort(pFrm->byData)*0.1 - 100; 
    		s_pl_BattRunData[nGrpIdx].fMin_CellTemp = (float)CAN_StringToUShort(pFrm->byData + 2)*0.1 - 100;
    		s_pl_BattRunData[nGrpIdx].usMaxCellTemp_Idx = CAN_StringToUShort(pFrm->byData + 4);
    		s_pl_BattRunData[nGrpIdx].usMinCellTemp_Idx = CAN_StringToUShort(pFrm->byData + 6);

			TRACE("fMax_CellTemp=%f fMin_CellTemp=%f usMaxCellTemp_Idx=%d usMinCellTemp_Idx=%d\n",\
			s_pl_BattRunData[nGrpIdx].fMax_CellTemp,
			s_pl_BattRunData[nGrpIdx].fMin_CellTemp,
			s_pl_BattRunData[nGrpIdx].usMaxCellTemp_Idx,
			s_pl_BattRunData[nGrpIdx].usMinCellTemp_Idx);
			break;
		}					    
  		case BATT_R_ESMB_GRP_42_50:
		{
			TRACE("BATT_R_ESMB_GRP_42_50 !!!\n");
			s_pl_BattRunData[nGrpIdx].byBattRunStatus = (*(pFrm->byData)) & 0x07;
			s_pl_BattRunData[nGrpIdx].bNeedForceCharge =(((*(pFrm->byData)) & 0x08) > 0) ? TRUE : FALSE;
			s_pl_BattRunData[nGrpIdx].bNeedBalanceCharge = (((*(pFrm->byData)) & 0x10) > 0) ? TRUE :FALSE;
			s_pl_BattRunData[nGrpIdx].usCyclePeriod = CAN_StringToUShort(pFrm->byData + 1);
         	//fault/alarm/protect
   			s_pl_BattRunData[nGrpIdx].mapFault.byFlag = *(pFrm->byData + 3);
    		s_pl_BattRunData[nGrpIdx].mapAlarm.usFlag = CAN_StringToUShort(pFrm->byData + 4);
    		s_pl_BattRunData[nGrpIdx].mapProtect.usFlag = CAN_StringToUShort(pFrm->byData + 6);
			
			TRACE("byBattRunStatus=%02X usCyclePeriod=%d mapFault.byFlag=%02X mapAlarm.usFlag=%02X mapProtect.usFlag=%02X bNeedForceCharge=%d bNeedBalanceCharge=%d\n",\
			s_pl_BattRunData[nGrpIdx].byBattRunStatus,\
			s_pl_BattRunData[nGrpIdx].usCyclePeriod,\
			s_pl_BattRunData[nGrpIdx].mapFault.byFlag,\
			s_pl_BattRunData[nGrpIdx].mapAlarm.usFlag,\
			s_pl_BattRunData[nGrpIdx].mapProtect.usFlag,\
			s_pl_BattRunData[nGrpIdx].bNeedForceCharge,\
			s_pl_BattRunData[nGrpIdx].bNeedBalanceCharge);
			break;
		}						
  		case BATT_R_ESMB_GRP_42_60:
		{
			TRACE("BATT_R_ESMB_GRP_42_60 !!!\n");
			break;
		}						
  		case BATT_R_ESMB_GRP_42_70:
		{
			TRACE("BATT_R_ESMB_GRP_42_70 !!!\n");
			break;
		}						
  		case BATT_R_ESMB_GRP_42_80:
		{
			TRACE("BATT_R_ESMB_GRP_42_80 !!!\n");
			s_pl_BattRunData[nGrpIdx].bIsBatt_ChargeForbidden = ((*(pFrm->byData)) == 0XAA) ? TRUE : FALSE;
    		s_pl_BattRunData[nGrpIdx].bIsBatt_DisChargeForbidden = ((*(pFrm->byData + 1)) == 0xAA) ? TRUE : FALSE;
			
			TRACE("bIsBatt_ChargeForbidden =%d bIsBatt_DisChargeForbidden=%d byData[0]=%02X\n",
				s_pl_BattRunData[nGrpIdx].bIsBatt_ChargeForbidden,
				s_pl_BattRunData[nGrpIdx].bIsBatt_DisChargeForbidden,
				*(pFrm->byData));
			break;
		}						
 		case BATT_R_ESMB_GRP_42_90:
		{
			TRACE("BATT_R_ESMB_GRP_42_90 !!!\n");
			s_pl_BattRunData[nGrpIdx].mapFault_Ext.byFlag = *(pFrm->byData);
			TRACE("mapFault_Ext.byFlag =%02X\n",\
			s_pl_BattRunData[nGrpIdx].mapFault_Ext.byFlag);
			break;
		}
		default:printf("4200_0 Not Found Anything\n"); break;
	}

	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UnPack_Batt_4200_2_Cmd
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static _ENUM UnPack_Batt_4200_2_Cmd(IN A_CAN_BATT_FRAME *pFrm )
{
	//pBattData->nTotalInstalledBatt = MAX(pBattData->nTotalInstalledBatt, pFrm->nGrpIdx);
	int nGrpIdx = pFrm->nGrpIdx -1 ;

	// TRACE("Enter UnPack_Batt_4200_2_Cmd nGrpIdx=[%d] nTotalInstalledBatt=[%d]\n",\
	// nGrpIdx,\
	// pBattData->nTotalInstalledBatt);

	if(nGrpIdx >= MAX_SINGLE_STBATT_NUM || nGrpIdx < 0)
	{
		nGrpIdx = 0;
	}
	
	s_pl_BattRunData[nGrpIdx].bFoundReply = TRUE;
	s_pl_BattRunData[nGrpIdx].bBattInfo_Regisited = TRUE;
	switch(pFrm->bySubCmd)
	{
		case BATT_R_SYS_EQUIP_73_10:
		{
			TRACE("BATT_R_SYS_EQUIP_73_10 !!!\n");
			s_pl_BattRunData[nGrpIdx].byHW_Ver = *(pFrm->byData);
			s_pl_BattRunData[nGrpIdx].byHW_V_Ver = *(pFrm->byData + 2);
			s_pl_BattRunData[nGrpIdx].byHW_R_Ver = *(pFrm->byData + 3);
			s_pl_BattRunData[nGrpIdx].usSW_V_Ver = *(pFrm->byData + 4);
			s_pl_BattRunData[nGrpIdx].usSW_Ver = *(pFrm->byData + 5);

			TRACE("byHW_Ver=%02X byHW_V_Ver=%02X byHW_R_Ver=%02X usSW_V_Ver=%d usSW_Ver=%d\n",\
			s_pl_BattRunData[nGrpIdx].byHW_Ver,
			s_pl_BattRunData[nGrpIdx].byHW_V_Ver,
			s_pl_BattRunData[nGrpIdx].byHW_R_Ver,
			s_pl_BattRunData[nGrpIdx].usSW_V_Ver,
			s_pl_BattRunData[nGrpIdx].usSW_Ver);

			break;
		}			
 		case BATT_R_SYS_EQUIP_73_20:
		{
			TRACE("BATT_R_SYS_EQUIP_73_20 !!!\n");
			 //from 0x7320
			s_pl_BattRunData[nGrpIdx].usBattQty = CAN_StringToUShort(pFrm->byData);
			s_pl_BattRunData[nGrpIdx].usBattModuleQty = (USHORT)(*(pFrm->byData + 2));
			s_pl_BattRunData[nGrpIdx].usCellInModuleQty = (USHORT)(*(pFrm->byData + 3));
			s_pl_BattRunData[nGrpIdx].fVoltageLevel = (float)CAN_StringToUShort(pFrm->byData + 4);
			s_pl_BattRunData[nGrpIdx].fBatt_AH = (float)CAN_StringToUShort(pFrm->byData + 6);

			TRACE("usBattQty=%d usBattModuleQty=%d usCellInModuleQty=%d fVoltageLevel=%f fBatt_AH=%f\n",\
			s_pl_BattRunData[nGrpIdx].usBattQty,
			s_pl_BattRunData[nGrpIdx].usBattModuleQty,
			s_pl_BattRunData[nGrpIdx].usCellInModuleQty,
			s_pl_BattRunData[nGrpIdx].fVoltageLevel,
			s_pl_BattRunData[nGrpIdx].fBatt_AH);
			break;
		}						
 	 				
		default: TRACE("4200_2 Not Found Anything\n");	break;				
	}
           
	return nGrpIdx;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Pack_PLBattCmd_AndGetReply
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static int Pack_PLBattCmd_AndGetReply(IN STBATT_SAMPLER_ROUGH *pBattData, IN A_CAN_BATT_FRAME* pSentFrame, IN BOOL bNeedGetReply)
{
	BYTE byRawData[CAN_FRAME_LEN] = {0};
	BYTE* pbySendBuf = byRawData;

	int nReadCount = 0, nReadLen = 0, n = 0;
	BOOL bFoundReply = FALSE;
	A_CAN_BATT_FRAME	frmReply;
	_ENUM 				emWhichGrp;

	//Stuff first 29 bits in the send frame
	CAN_StuffHead(pSentFrame->byCmdType, pSentFrame->bySubCmd,
					pSentFrame->nGrpIdx, CAN_DATA_AREA_LEN, pbySendBuf);
	
	memcpy(pbySendBuf + 5, pSentFrame->byData, CAN_DATA_AREA_LEN);
	
	static int nWriteErrCounter = 0;
	int nWriteBytes = Write_CAN1(pBattData, (void *)pbySendBuf, CAN_FRAME_LEN);
	if(nWriteBytes < 0)  
	{
		TRACE("-Pack_PLBattCmd_AndGetReply --Write Bytes = %d,\n",nWriteBytes);
		nWriteBytes = Write_CAN1(pBattData, (void *)pbySendBuf, CAN_FRAME_LEN);
		if(nWriteBytes < 0)
		{
			for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
			{
				s_pl_BattRunData[n].bFoundReply = FALSE;
			}
			return CAN_WRITE_ERROR;
		}
	}
	TRACE("nWriteBytes = %d\n", nWriteBytes);
	//---------Now get back reply if needed-----------//
	if(bNeedGetReply)
	{
		BYTE *byRecBuff = pBattData->abyRcvBuf;
		memset(byRecBuff, 0, MAX_READ_BUFF_SIZE * CAN_FRAME_LEN);
		ZERO_POBJS(&frmReply, 1);
		do
		{
			Sleep(20);
			nReadLen = pBattData->pComm->Read(pBattData->hCanDevHanle, (void *)byRecBuff, (ssize_t)(CAN_FRAME_LEN * MAX_READ_BUFF_SIZE));
			if(nReadLen > CAN_FRAME_LEN * MAX_READ_BUFF_SIZE)
			{
				nReadLen = CAN_FRAME_LEN * MAX_READ_BUFF_SIZE;
			}
			TRACE("CAN_STBATT_nReadLen =%d\n",nReadLen);
			if(nReadLen >= CAN_FRAME_LEN)
			{
				for(n = 0;n < nReadLen / CAN_FRAME_LEN;n++)
				{
					memcpy(frmReply.byRawData, byRecBuff + n*CAN_FRAME_LEN, CAN_FRAME_LEN);
					CAN_ParseHead(&frmReply);
					//Just Judge CmdType//
					bFoundReply = TRUE;
					if(frmReply.byCmdType == 0x42)
					{	
						emWhichGrp = UnPack_Batt_4200_0_Cmd(&frmReply);
					}
					else if(frmReply.byCmdType == 0x73)
					{
						emWhichGrp = UnPack_Batt_4200_2_Cmd(&frmReply);
					}
				}
			}
			else
			{
				//Sleep(10);
			}
			if(bFoundReply)
			{
				break;
			}
			nReadCount++;
		}while (nReadCount < MAX_READ_RETRY_TIMES); //Max wait 75ms

		if(!bFoundReply)
		{
			TRACE("\n------------Error while reading CANBus data, nReadCount = %d!!!\n",nReadCount);
			for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
			{
				s_pl_BattRunData[n].bFoundReply = FALSE;
			}
			return CAN_READ_ERROR;
		}
	}
	else
	{
		//Do Nothing
	}

	return CAN_SAMPLE_OK;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Pack_PLBattCmd_AndGetReply
*  CREATOR       :  
*  CREATED DATE  : 2019-7-31 16:27:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Proc_Comm_Statu(STBATT_SAMPLER_ROUGH *pBattData)
{
	int n = 0 ;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
 		if(n >= pBattData->nSystemCfgBattNum)
        {
			// s_pl_BattRunData[n].bIsCommFail = FALSE; // Only one Batt,so Batt 2 default ok 
            // TRACE("s_pl_BattRunData[%d].bIsCommFail =%d\n",\
			// 	n, s_pl_BattRunData[n].bIsCommFail);
			continue;
        }

		if(s_pl_BattRunData[n].bFoundReply == FALSE)
		{
			if(s_pl_BattRunData[n].nInterrupt_Times <= BATT_COMM_FAIL_TIMES)
			{
				s_pl_BattRunData[n].nInterrupt_Times++;
			}
		}
		else
		{
			s_pl_BattRunData[n].nInterrupt_Times = 0;
		}
			
		TRACE("333 ->pl_BattRunData[%d].bIsCommFail = %d ->pl_BattRunData[n].nInterrupt_Times=%d bFoundReply=%d\n",\
		 	n, s_pl_BattRunData[n].bIsCommFail,s_pl_BattRunData[n].nInterrupt_Times,s_pl_BattRunData[n].bFoundReply);

		s_pl_BattRunData[n].bIsCommFail = (s_pl_BattRunData[n].nInterrupt_Times > BATT_COMM_FAIL_TIMES) ? TRUE : FALSE;

		if(s_pl_BattRunData[n].bIsCommFail)
		{
			s_pl_BattRunData[n].bBattInfo_Regisited = FALSE;
		}
	}
}

/*********************************************************************************
*  
*  FUNCTION NAME : PL_Do_Comm
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   : Pack Can Cmd and Get Reply
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static ERR_CODE_CAN_R PL_Do_Comm(STBATT_SAMPLER_ROUGH *pBattData, A_CAN_BATT_FRAME* pFrm, BYTE byCmdType, BYTE bySubCmd, int nGrpIdx, BOOL bIsSetCmd)
{
	ERR_CODE_CAN_R 	emRet = ERR_CAN_R_NO_ERR;

	MKAE_PL_BATT_FRAME(pFrm, byCmdType, bySubCmd, nGrpIdx, bIsSetCmd);

	BOOL bIsNeedReply = TRUE;
	if(bIsSetCmd == TRUE)
	{
		bIsNeedReply = FALSE;
	}

	int nSampeRet = Pack_PLBattCmd_AndGetReply(pBattData, pFrm, bIsNeedReply);
	if(nSampeRet != CAN_SAMPLE_OK)
	{
		emRet = ERR_CAN_R_COMM_FAIL;
	}
	else
	{
		emRet = ERR_CAN_R_NO_ERR;
	}

	Proc_Comm_Statu(pBattData);

	return emRet;

}

/*********************************************************************************
*  
*  FUNCTION NAME : Get_ESMB_Info
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   : Ensemble information
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERR_CODE_CAN_R Get_ESMB_Info(int nIdx, void* pRawData, void* pSetData )
{
	TRACE("Enter Get_ESMB_Info !!!\n");
	UNUSED(pSetData);
	STBATT_SAMPLER_ROUGH *pBattData = (STBATT_SAMPLER_ROUGH*)pRawData;
	A_CAN_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byData[0] = 0x00;  

	ERR_CODE_CAN_R emRet = PL_Do_Comm(pBattData, &frm, 0x42, 0x00, nIdx, FALSE );
	if(emRet == ERR_CAN_R_NO_ERR)
	{
		TRACE("Parse _CAN_R_CMD_G_4200_0 !!!\n");
	}
 
	return emRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Get_SysEquip_Info
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   : system equipment information
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERR_CODE_CAN_R Get_SysEquip_Info( int nIdx, void* pRawData, void* pSetData )
{
	TRACE("Enter Get_SysEquip_Info !!!\n");
	
	UNUSED(pSetData);
	STBATT_SAMPLER_ROUGH *pBattData = (STBATT_SAMPLER_ROUGH*)pRawData;
	A_CAN_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byData[0] = 0x02;

	ERR_CODE_CAN_R emRet = PL_Do_Comm(pBattData, &frm, 0x42, 0x00, nIdx, FALSE );
	if(emRet == ERR_CAN_R_NO_ERR)
	{
		TRACE("Parse _CAN_R_CMD_G_4200_2 !!!\n");
	}
	
	return emRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Set_SleepOrAwake
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   : Sleep / Awake Command
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERR_CODE_CAN_R Set_SleepOrAwake( int nIdx, void* pRawData, void* pSetData )
{
	TRACE("Enter Set_SleepOrAwake !!!\n");
	ASSERT(pSetData);

	BOOL bIsEnterSleepMode = (BOOL)(*(int*)pSetData);
	STBATT_SAMPLER_ROUGH *pBattData = (STBATT_SAMPLER_ROUGH*)pRawData;
	A_CAN_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byData[0] = (bIsEnterSleepMode == TRUE) ? 0x55 : 0xAA;  
	
	ERR_CODE_CAN_R emRet = PL_Do_Comm(pBattData, &frm, 0x82, 0x00, nIdx, TRUE);
	if(emRet == ERR_CAN_R_NO_ERR)
	{
		TRACE("Parse _CAN_R_CMD_G_8200_[%d] bIsEnterSleepMode=%d  ok !!!\n",nIdx ,bIsEnterSleepMode);
	}

	return emRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Set_ChrgOrDischrg
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   : Charge/Discharge Command
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERR_CODE_CAN_R Set_ChrgOrDischrg( int nIdx, void* pRawData, void* pSetData )
{
	printf("Enter Set_ChrgOrDischrg !!!\n");
	ASSERT(pSetData);

	BOOL bIsChrgCmd = (BOOL)(*(int*)pSetData);
	STBATT_SAMPLER_ROUGH *pBattData = (STBATT_SAMPLER_ROUGH*)pRawData;
	A_CAN_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	(bIsChrgCmd == TRUE) ? (frm.byData[0]=0xAA) : (frm.byData[1]=0xAA) ;  
	
	ERR_CODE_CAN_R emRet = PL_Do_Comm(pBattData, &frm, 0x82, 0x10, nIdx, TRUE);
	if(emRet == ERR_CAN_R_NO_ERR)
	{
		TRACE("Parse _CAN_R_CMD_G_8210_[%d] bIsChrgCmd=[%d] ok!!!\n", nIdx, bIsChrgCmd);
	}

	return emRet;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Set_TempMaskErr
*  CREATOR       :  
*  CREATED DATE  : 2019-7-26 11:22:53
*  DESCRIPTION   : Temporary masking “external communication error” command
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
ERR_CODE_CAN_R Set_TempMaskErr( int nIdx, void* pRawData, void* pSetData )
{
	printf("Enter Set_TempMaskErr !!!\n");
	UNUSED(pSetData);
	STBATT_SAMPLER_ROUGH *pBattData = (STBATT_SAMPLER_ROUGH*)pRawData;
	A_CAN_BATT_FRAME frm;
	ZERO_POBJS(&frm, 1);

	frm.byData[0] = 0xAA;
	
	ERR_CODE_CAN_R emRet = PL_Do_Comm(pBattData, &frm, 0x82, 0x40, nIdx, TRUE);
	if(emRet == ERR_CAN_R_NO_ERR)
	{
		TRACE("Parse _CAN_R_CMD_G_8240_[%d] ok !!!\n",nIdx);
	}
	return emRet;
}

static void Map_BattRawData_2Rough(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	int n;
	VAR_VALUE varV;
	A_BLOB_GROUP_STBATT_RUNINFO* pGrpRunInfo = &pSTBatt_Rough->blobGroupBattInfo;
	A_BLOB_SINGLE_STBATT_RUNINFO* pSingleRunInfo = NULL;
	int nOnlineTotal = 0;
	int nWorkTotal = 0;
	BOOL bGrpNeedForbid_Charge = FALSE;
	BOOL bGrpNeedForbid_DisCharge = FALSE;

	for(n =0 ;n < MAX_SINGLE_STBATT_NUM; n++)
	{
		pSingleRunInfo = &pSTBatt_Rough->blobSingleBattInfo[n];
		//skip those not configed
		if(n >= pSTBatt_Rough->nSystemCfgBattNum)
		{
			ZERO_POBJS(pSingleRunInfo, 1);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_NOT_INSTALLED;
			continue;
		}
		else if((s_pl_BattRunData[n].bIsCommFail) ||
			(!s_pl_BattRunData[n].bBattInfo_Regisited))
		{
			TRACEX("bIsCommFail = %d, bBattInfo_Regisited = %d\n", s_pl_BattRunData[n].bIsCommFail,
				s_pl_BattRunData[n].bBattInfo_Regisited);
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_COMM_FAIL;
			nOnlineTotal++;
			continue;
		}

		nOnlineTotal++;
	
		pSingleRunInfo->fPackTotalVolt = s_pl_BattRunData[n].fPackTotalVolt;
		pSingleRunInfo->fPackTotalCurr = s_pl_BattRunData[n].fPackTotalCurr;
		pSingleRunInfo->nSOC = s_pl_BattRunData[n].bySOC;
		pSingleRunInfo->nSOH = s_pl_BattRunData[n].bySOH;
		pSingleRunInfo->fAllow_LmtChargeVolt = s_pl_BattRunData[n].fCharge_Cutoff_Volt;
		pSingleRunInfo->fAllow_LmtDisChargeVolt = s_pl_BattRunData[n].fDisCharge_Cutoff_Volt;
		pSingleRunInfo->fAllow_MaxChargeCurr = s_pl_BattRunData[n].fAllow_MaxChargeCurr;
		pSingleRunInfo->fAllow_MaxDisChargeCurr = s_pl_BattRunData[n].fAllow_MaxDisChargeCurr;

		//stBatt_DataInfo.emBattStatus_Run = ProBattRunStatu(pSTBatt_Rough, n);  
		//stBatt_DataInfo.nSOC = (int)s_pl_BattRunData[n].bySOC;
		//stBatt_DataInfo.fAllow_MaxOutputPower = ABS(s_pl_BattRunData[n].fPackTotalVolt*s_pl_BattRunData[n].fAllow_MaxDisChargeCurr*0.001);
    		 
    	pSingleRunInfo->fMax_CellVolt = s_pl_BattRunData[n].fMax_CellVolt;
    	pSingleRunInfo->fMin_CellVolt = s_pl_BattRunData[n].fMin_CellVolt;  
    	pSingleRunInfo->fMax_CellTemp = s_pl_BattRunData[n].fMax_CellTemp;
    	pSingleRunInfo->fMin_CellTemp = s_pl_BattRunData[n].fMin_CellTemp;
    	pSingleRunInfo->nLifeCycleTimes = s_pl_BattRunData[n].usCyclePeriod;

		// if(s_pl_BattRunData[n].bIsBatt_ChargeForbidden)
		// {
		// 	bGrpNeedForbid_Charge = TRUE;
		// }
		bGrpNeedForbid_Charge = s_pl_BattRunData[n].bIsBatt_ChargeForbidden ? TRUE : FALSE; //if all is forbidden, then forbidden
		pSingleRunInfo->bIsBatt_ChargeForbidden = s_pl_BattRunData[n].bIsBatt_ChargeForbidden;

		if(s_pl_BattRunData[n].bIsBatt_DisChargeForbidden)
		{
			bGrpNeedForbid_DisCharge = TRUE;
		}

		pSingleRunInfo->bIsBatt_DisChargeForbidden = s_pl_BattRunData[n].bIsBatt_DisChargeForbidden;

		//Now handle status
		switch (s_pl_BattRunData[n].byBattRunStatus)
		{
		case 1:
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_CHARGE;
			nWorkTotal++;
			break;
		case 2:
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_DISCHARGE;
			nWorkTotal++;
			break;
		case 3:
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_IDLE;
			break;
		case 0:
		default:
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
			break;
		}
		//alarm status
		if(s_pl_BattRunData[n].mapFault.byFlag > 0)
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_FAULT;
			//must set status into sleep in case BMS not set
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
		}
		else if(s_pl_BattRunData[n].mapProtect.usFlag > 0)
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_PROTECT;
			//must set status into sleep in case BMS not set
			pSingleRunInfo->fAllow_MaxDisChargeCurr = 0.0;
			pSingleRunInfo->fAllow_MaxChargeCurr = 0.0;
			pSingleRunInfo->emBattStatus_Run = STBATT_RUNSTAT_SLEEP;
		}
		else if(s_pl_BattRunData[n].mapAlarm.usFlag > 0)
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_ALARMING;
		}
		else
		{
			pSingleRunInfo->emBattStatus_Alm = STBATT_ALMSTAT_NOMAL;
		}
	}

	pGrpRunInfo->nTotalOnlineBatt = nOnlineTotal;
	pGrpRunInfo->nTotalInUseBatt = nWorkTotal;

	pGrpRunInfo->bIsBatt_ChargeForbidden = bGrpNeedForbid_Charge;
	pGrpRunInfo->bIsBatt_DisChargeForbidden = bGrpNeedForbid_DisCharge;
}

void PL_Batt_Sample(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	ERR_CODE_CAN_R errCode;

	if(Judge_IfNeedBatt_ReConfig(pSTBatt_Rough))
	{
		//now send 4200+2 to gather batt device info
		errCode = Do_CanSession( pSTBatt_Rough, s_cmdInfos[_CAN_R_CMD_G_4200_2].pfnCmdHandler, CAN_CMD_BROADCAST, FALSE, NULL); 
		if ( errCode != ERR_CAN_R_NO_ERR  )
		{
			TRACE("Get 4200_2 Error !\n");
		}
	}

    //now sampling group Batt run info
	errCode = Do_CanSession( pSTBatt_Rough, s_cmdInfos[_CAN_R_CMD_G_4200_0].pfnCmdHandler, CAN_CMD_BROADCAST, FALSE, NULL); 
	if ( errCode == ERR_CAN_R_NO_ERR )
	{
		TRACE("Get 4200_0 ok !\n");	
	}
	else
	{
		/* code */
		TRACE("Get 4200_0 Error !\n");
	}
	
	Map_BattRawData_2Rough(pSTBatt_Rough);
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  
*  CREATED DATE  : 2019-9-30 14:22:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void PL_Batt_Init(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//---------Init cmd-------------
	CAN_BATT_FRAME_INFO* p = &s_cmdInfos[0];
	INIT_PLBATT_CMD(p,	_CAN_R_CMD_G_4200_0,	Get_ESMB_Info);
	INIT_PLBATT_CMD(p,	_CAN_R_CMD_G_4200_2,	Get_SysEquip_Info);	
	INIT_PLBATT_CMD(p,	_CAN_R_CMD_S_8200,		Set_SleepOrAwake);
	INIT_PLBATT_CMD(p,	_CAN_R_CMD_S_8210,		Set_ChrgOrDischrg);
 	INIT_PLBATT_CMD(p,	_CAN_R_CMD_S_8400,		Set_TempMaskErr);

	//---------Init rough-------------
	int i;
	for(i = 0; i < MAX_SINGLE_STBATT_NUM; i++)
	{
		memset(&s_pl_BattRunData[i], 0 ,sizeof(PL_BATT_RUN_DATA));
		
		//from 0x4210
		s_pl_BattRunData[i].fPackTotalVolt = 0;
		s_pl_BattRunData[i].fPackTotalCurr = 0;
		s_pl_BattRunData[i].fMainControllerTemperature = 0;
		s_pl_BattRunData[i].bySOC = 0;
		s_pl_BattRunData[i].bySOH = 0;
		
		//from 0x4220
		s_pl_BattRunData[i].fCharge_Cutoff_Volt = 0;
		s_pl_BattRunData[i].fDisCharge_Cutoff_Volt = 0;
		s_pl_BattRunData[i].fAllow_MaxChargeCurr = 0;
		s_pl_BattRunData[i].fAllow_MaxDisChargeCurr = 0;
		
		//from 0x4230
		s_pl_BattRunData[i].fMax_CellVolt = 0;
		s_pl_BattRunData[i].fMin_CellVolt = 0;
		s_pl_BattRunData[i].usMaxCellVolt_Idx = 0;
		s_pl_BattRunData[i].usMinCellVolt_Idx = 0;
		
		//from 0x4240
		s_pl_BattRunData[i].fMax_CellTemp = 0;
		s_pl_BattRunData[i].fMin_CellTemp = 0;
		s_pl_BattRunData[i].usMaxCellTemp_Idx = 0;
		s_pl_BattRunData[i].usMinCellTemp_Idx = 0;
		
		//from 0x4250
		s_pl_BattRunData[i].byBattRunStatus = 0; //0: Sleep, 1: Charge, 2: Discharge, 3: Idle
		//following 2 bites are from 
		s_pl_BattRunData[i].bNeedBalanceCharge = FALSE;
		s_pl_BattRunData[i].bNeedForceCharge = FALSE;
		s_pl_BattRunData[i].usCyclePeriod = 0;
		//fault/alarm/protect
		s_pl_BattRunData[i].mapFault.byFlag = 0;
		s_pl_BattRunData[i].mapAlarm.usFlag = 0;
		s_pl_BattRunData[i].mapProtect.usFlag = 0;
		
		//from 0x4280
		s_pl_BattRunData[i].bIsBatt_ChargeForbidden = FALSE;
		s_pl_BattRunData[i].bIsBatt_DisChargeForbidden = FALSE;
		
		//from 0x4290
		s_pl_BattRunData[i].mapFault_Ext.byFlag  = 0;

		//from 0x7310
		s_pl_BattRunData[i].byHW_Ver = 0;
		s_pl_BattRunData[i].byHW_V_Ver = 0;
		s_pl_BattRunData[i].byHW_R_Ver = 0;
		s_pl_BattRunData[i].usSW_V_Ver = 0;
		s_pl_BattRunData[i].usSW_Ver = 0;
		
		//from 0x7320
		s_pl_BattRunData[i].usBattQty = 0;
		s_pl_BattRunData[i].usBattModuleQty = 0;
		s_pl_BattRunData[i].usCellInModuleQty = 0;
		s_pl_BattRunData[i].fVoltageLevel = 0;
		s_pl_BattRunData[i].fBatt_AH = 0;

		s_pl_BattRunData[i].nInterrupt_Times = 0;
		s_pl_BattRunData[i].bIsCommFail = FALSE;
		s_pl_BattRunData[i].bBattInfo_Regisited = FALSE; //if the battGrp info NOT registered, should send 4200+2

		s_pl_BattRunData[i].bFoundReply = FALSE;
	}
}

BOOL PL_Batt_Control(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, STBATT_CTRL_CMDTYPE emCmdTp, int nWhichBatt)
{
	BOOL bRtn = FALSE;
	TRACEX("-----Now received cmd[%d] for Batt[%d], bIscommFail = %d--------\n", emCmdTp, nWhichBatt, 
		s_pl_BattRunData[nWhichBatt].bIsCommFail);
		
	if(s_pl_BattRunData[nWhichBatt].bIsCommFail)
	{
		return FALSE;
	}
	ERR_CODE_CAN_R errCode;
	BOOL bEnterSleepMode = FALSE;
	BOOL bEnterChrg = TRUE;
	if(emCmdTp == CTRL_STBATT_WAKE_UP)
	{
		TRACE(" BATT[%d]_WAKE_UP \n", nWhichBatt);
		errCode = Do_CanSession( pSTBatt_Rough, s_cmdInfos[_CAN_R_CMD_S_8200].pfnCmdHandler, 
			nWhichBatt, TRUE, &bEnterSleepMode); 
		errCode = Do_CanSession( pSTBatt_Rough, s_cmdInfos[_CAN_R_CMD_S_8210].pfnCmdHandler, 
			nWhichBatt, TRUE, &bEnterChrg); 
		bRtn = (errCode == ERR_CAN_R_NO_ERR) ? TRUE : FALSE;
	}
	else if(emCmdTp == CTRL_STBATT_GO_SLEEP)
	{
		TRACE(" BATT[%d]_GOTO_SLEEP \n", nWhichBatt);
		bEnterSleepMode = TRUE;
		errCode = Do_CanSession( pSTBatt_Rough, s_cmdInfos[_CAN_R_CMD_S_8200].pfnCmdHandler, 
			nWhichBatt, TRUE, &bEnterSleepMode);
		bRtn = (errCode == ERR_CAN_R_NO_ERR) ? TRUE : FALSE;
	}
	else if(emCmdTp == CTRL_STBATT_MASK_ERR)
	{
		errCode = Do_CanSession( pSTBatt_Rough, s_cmdInfos[_CAN_R_CMD_S_8400].pfnCmdHandler, 
			nWhichBatt, TRUE, NULL);
		bRtn = (errCode == ERR_CAN_R_NO_ERR) ? TRUE : FALSE;
	}
	return bRtn;
}
