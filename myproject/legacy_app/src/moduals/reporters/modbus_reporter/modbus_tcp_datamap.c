#include "modbus.h"

#define CAB_MAX  4
#define CABINET_CELL_RESERVED       300   // 预留格数

#define CELLV_SIG(cab, cell) (MD_SIG_CELLV_BASE + (cab)*CABINET_CELL_RESERVED + (cell))
#define CELLT_SIG(cab, cell) (MD_SIG_CELLT_BASE + (cab)*CABINET_CELL_RESERVED + (cell))


void addCellItems(MD_DATA_ITEM *items, int *idx,MODBUS_DATA_STU * pModbus)
{
	int step = 2;          
	int cab,cell;
    int baseVolt = 0x7200;  // 4 * 300 * 2
	int baseTemp = baseVolt + CAB_MAX * CABINET_CELL_RESERVED * step;

    for (cab = 0; cab < CAB_MAX; cab++) //单体电压
	{
        for (cell = 0; cell < A_CABINET_CELL_MAX_NUM; cell++) 
		{
            items[*idx].emSigType    	   = CELLV_SIG(cab, cell);

			items[*idx].bNeedSpecialPro    =  FALSE;

			items[*idx].nDevUnitId    	   =  0;
			items[*idx].nSigType    	   =  DAITYPE_STAT_PARAMVALUE_R;
			items[*idx].nSigId             = 0;

            items[*idx].emDataType         = MD_GET_VAL_FLOAT;
			items[*idx].nDataSize          = 1;
            items[*idx].nMultiNum          = step;
            items[*idx].nRegAdrStart       = baseVolt + (cab)*CABINET_CELL_RESERVED*step + (cell)*step;
            items[*idx].nRegAdrEnd         = items[*idx].nRegAdrStart + step - 1;

			items[*idx].nFlag0             = 0;
			items[*idx].nFlag1             = 1;

            items[*idx].ptrValue           = (void*)&(pModbus->strYC_CellInfo[cab].fcellVolt[cell]);\

			// if (cell == 0 || cell == 239)
			// {
			// 	printf("Volt cab[%d] cell[%d] nRegAdrStart[%04x] nRegAdrEnd[%04x]\n",cab,cell,items[*idx].nRegAdrStart,items[*idx].nRegAdrEnd);
			// }

            (*idx)++;
		}

	}

	for (cab = 0; cab < CAB_MAX; cab++) //单体温度
	{
        for (cell = 0; cell < A_CABINET_CELL_MAX_NUM; cell++) 
		{
            items[*idx].emSigType          = CELLT_SIG(cab, cell);

			items[*idx].bNeedSpecialPro    =  FALSE;

			items[*idx].nDevUnitId    	   =  0;
			items[*idx].nSigType    	   =  DAITYPE_STAT_PARAMVALUE_R;
			items[*idx].nSigId             = 0;

            items[*idx].emDataType         = MD_GET_VAL_FLOAT;
			items[*idx].nDataSize          = 1;
            items[*idx].nMultiNum          = step;
            items[*idx].nRegAdrStart       = baseTemp + (cab)*CABINET_CELL_RESERVED*step + (cell)*step;
            items[*idx].nRegAdrEnd         = items[*idx].nRegAdrStart + step - 1;

			items[*idx].nFlag0             = 0;
			items[*idx].nFlag1             = 1;

            items[*idx].ptrValue           = (void*)&(pModbus->strYC_CellInfo[cab].fcellTemp[cell]);

			// if (cell == 0 || cell == 239)
			// {
			// 	printf("Temp cab[%d] cell[%d] nRegAdrStart[%04x] nRegAdrEnd[%04x]\n",cab,cell,items[*idx].nRegAdrStart,items[*idx].nRegAdrEnd);
			// }

            (*idx)++;
        }
    }
}


/*==========================================================================*
 * FUNCTION : LoadModbusModelMapConfig
 * PURPOSE  : 
 * CALLS    :  
 *			  
 * CALLED BY: 
 * ARGUMENTS:   
 * RETURN   : DWORD 
 * COMMENTS : 
 * CREATOR  :					2019-08-11 9:25:10
 *==========================================================================*/
DWORD InitModbusDataMap(REPORTER *pReporter)
{
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET			valGet;
	int idx = 0;

	//1. Init Parameter
	pModbus->bAcceptConnOK = FALSE;
	pModbus->hMDDataComm = NULL;
	pModbus->nReadFailtTimes = 0;
 	pModbus->tmLastDataRefresh = _NOW_;

	// Init MapInfo
	pModbus->ModbusModelCfg.iTypeMapNum = MODBUS_CMD_NUM;
	pModbus->ModbusModelCfg.pTypeMapInfo[0].iFunctionCode = MODBUS_GETDATA_CMD;
	pModbus->ModbusModelCfg.pTypeMapInfo[0].iLengthFix = 0;
	pModbus->ModbusModelCfg.pTypeMapInfo[0].iLength = 8;
	pModbus->ModbusModelCfg.pTypeMapInfo[1].iFunctionCode = MODBUS_SETSINGLEDATA_CMD;
	pModbus->ModbusModelCfg.pTypeMapInfo[1].iLengthFix = 0;
	pModbus->ModbusModelCfg.pTypeMapInfo[1].iLength = 8;
	pModbus->ModbusModelCfg.pTypeMapInfo[2].iFunctionCode = MODBUS_SETMULTIDATA_CMD;
	pModbus->ModbusModelCfg.pTypeMapInfo[2].iLengthFix = 1;
	pModbus->ModbusModelCfg.pTypeMapInfo[2].iLength = 7;
	
	// Flag0: basic Value	Flag1: Coeff	Output Value = Value *Coeff +Basic Value 
	MD_DATA_ITEM mdDataItems[] =
	{	
		{MD_SIG_SysAvaliaStu,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0000 ,0x0000, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.emSysAvaliaStatu)},
		{MD_SIG_SysFailRsn,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0001 ,0x0001, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.emSysFailReason)},
		{MD_SIG_OnOffGrid,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0002 ,0x0002, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.emOnOffGrid)},
		{MD_SIG_BatCfgNum,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0003 ,0x0003, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.nBatCfgNum)},
		{MD_SIG_EmsCRC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0004 ,0x0004, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.emEmsCRC)},
		{MD_SIG_SW_Ver,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_UL,		1, 2, 0x0005 ,0x0006, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.ulSW_Ver)},
		//{MD_SIG_szResv,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_SPECIAL_1,1, 8, 0x0008 ,0x000F, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.szResv)},

		{MD_SIG_PgridMeter,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0010 ,0x0011, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPgridMeter)},
		{MD_SIG_PcabMeter,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0012 ,0x0013, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPcabMeter)},
		{MD_SIG_PauxMeter,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0014 ,0x0015, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPauxMeter)},
		{MD_SIG_PoffGridMeter,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0016 ,0x0017, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPoffGridMeter)},
		{MD_SIG_Pmppt,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0018 ,0x0019, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPmppt)},
		{MD_SIG_Pbms1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x001A ,0x001B, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[0])},
		{MD_SIG_Pbms2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x001C ,0x001D, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[1])},
		{MD_SIG_Pbms3,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x001E ,0x001F, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[2])},
		{MD_SIG_Pbms4,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0020 ,0x0021, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[3])},
		//{MD_SIG_Pbms5,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0022 ,0x0023, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[4])},
		//{MD_SIG_Pbms6,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0024 ,0x0025, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[5])},
		//{MD_SIG_Pbms7,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0026 ,0x0027, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[6])},
		//{MD_SIG_Pbms8,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0028 ,0x0029, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPbms[7])},
		{MD_SIG_Ppcs1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x002A ,0x002B, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[0])},
		{MD_SIG_Ppcs2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x002C ,0x002D, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[1])},
		{MD_SIG_Ppcs3,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x002E ,0x002F, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[2])},
		{MD_SIG_Ppcs4,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0030 ,0x0031, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[3])},
		//{MD_SIG_Ppcs5,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0032 ,0x0033, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[4])},
		//{MD_SIG_Ppcs6,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0034 ,0x0035, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[5])},
		//{MD_SIG_Ppcs7,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0036 ,0x0037, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[6])},
		//{MD_SIG_Ppcs8,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0038 ,0x0039, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fPpcs[7])},
		{MD_SIG_MaxSOC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0040 ,0x0040, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.emMaxSoc)},
		{MD_SIG_MinSOC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0041 ,0x0041, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.emMinSoc)},
		{MD_SIG_TotalSOC,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0042 ,0x0043, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalSoc_old)},

		{MD_SIG_SysCtlMode,					FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0050 ,	0x0050, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.usEmsCtlMode				)},
		{MD_SIG_TotalActPowTgt,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0051 ,	0x0052, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalActPowTgt				)},
		{MD_SIG_SysStaus,					FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0053 ,	0x0053, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.usSysStatus					)},
		{MD_SIG_RunMode,					FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x0054 ,	0x0054, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.usRunMode					)},
		{MD_SIG_TotalSoc,					FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0055 ,	0x0056, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalSoc					)},
		{MD_SIG_TotalActPow_fromCabMeter,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0057 ,	0x0058, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalActPow_fromCabMeter	)},
		{MD_SIG_TotalActPow_fromPcs,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0059 ,	0x005A, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fSysCtlMode_fromPcs			)},
		{MD_SIG_TotalMaxChgPowLimit,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x005B ,	0x005C, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalMaxChgPowLimit		)},
		{MD_SIG_TotalMaxDischgPowLimit,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x005D ,	0x005E, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalMaxDishgPowLimit		)},
		{MD_SIG_TotalChgkWh,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x005F ,	0x0060, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalChgkWh				)},
		{MD_SIG_TotalOutputCurr,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0061 ,	0x0062, 0, 1 ,	(void*)&(pModbus->strSys_SigMap.fTotalOutputCurr			)},

		{MD_SIG_Cab_PhAVolt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4100 ,0x4101, 0, 1 , (void*)&(pModbus->strMeter[0].fPhaseA_Volt) 		},
		{MD_SIG_Cab_PhBVolt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4102 ,0x4103, 0, 1 , (void*)&(pModbus->strMeter[0].fPhaseB_Volt) 		},
		{MD_SIG_Cab_PhCVolt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4104 ,0x4105, 0, 1 , (void*)&(pModbus->strMeter[0].fPhaseC_Volt) 		},
		{MD_SIG_Cab_PhACurt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4106 ,0x4107, 0, 1 , (void*)&(pModbus->strMeter[0].fPhaseA_Curt) 		},
		{MD_SIG_Cab_PhBCurt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4108 ,0x4109, 0, 1 , (void*)&(pModbus->strMeter[0].fPhaseB_Curt) 		},
		{MD_SIG_Cab_PhCCurt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x410A ,0x410B, 0, 1 , (void*)&(pModbus->strMeter[0].fPhaseC_Curt) 		},
		{MD_SIG_Cab_Pactive, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x410C ,0x410D, 0, 1 , (void*)&(pModbus->strMeter[0].fActive_Power) 		},
		{MD_SIG_Cab_Qreactive, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x410E ,0x410F, 0, 1 , (void*)&(pModbus->strMeter[0].fReactive_Power) 	},
		{MD_SIG_Cab_Freq , 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4110 ,0x4111, 0, 1 , (void*)&(pModbus->strMeter[0].f_Freq) 				},
		{MD_SIG_Cab_Factor, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4112 ,0x4113, 0, 1 , (void*)&(pModbus->strMeter[0].f_Factor) 			},
		{MD_SIG_Cab_Pos_Kwh, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4114 ,0x4115, 0, 1 , (void*)&(pModbus->strMeter[0].fPos_Kwh) 			},
		{MD_SIG_Cab_Neg_Kwh, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4116 ,0x4117, 0, 1 , (void*)&(pModbus->strMeter[0].fNeg_Kwh) 			},

		{MD_SIG_Cab_Current_Pos_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4118 ,0x4119, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Pos_Sum_Kwh) },
		{MD_SIG_Cab_Current_Pos_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x411A ,0x411B, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Pos_Sharp_Kwh) },
		{MD_SIG_Cab_Current_Pos_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x411C ,0x411D, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Pos_Peak_Kwh) },
		{MD_SIG_Cab_Current_Pos_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x411E ,0x411F, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Pos_Flat_Kwh) },
		{MD_SIG_Cab_Current_Pos_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4120 ,0x4121, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Pos_Wave_Kwh) },
		{MD_SIG_Cab_Current_Neg_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4122 ,0x4123, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Neg_Sum_Kwh) },
		{MD_SIG_Cab_Current_Neg_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4124 ,0x4125, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Neg_Sharp_Kwh) },
		{MD_SIG_Cab_Current_Neg_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4126 ,0x4127, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Neg_Peak_Kwh) },
		{MD_SIG_Cab_Current_Neg_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4128 ,0x4129, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Neg_Flat_Kwh) },
		{MD_SIG_Cab_Current_Neg_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x412A ,0x412B, 0, 1 , (void*)&(pModbus->strMeter[0].fCurrent_Neg_Wave_Kwh) },
		{MD_SIG_Cab_Last1_Pos_Sum_Kwh, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x412C ,0x412D, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Pos_Sum_Kwh) },
		{MD_SIG_Cab_Last1_Pos_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x412E ,0x412F, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Pos_Sharp_Kwh) },
		{MD_SIG_Cab_Last1_Pos_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4130 ,0x4131, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Pos_Peak_Kwh) },
		{MD_SIG_Cab_Last1_Pos_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4132 ,0x4133, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Pos_Flat_Kwh) },
		{MD_SIG_Cab_Last1_Pos_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4134 ,0x4135, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Pos_Wave_Kwh) },
		{MD_SIG_Cab_Last1_Neg_Sum_Kwh, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4136 ,0x4137, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Neg_Sum_Kwh) },
		{MD_SIG_Cab_Last1_Neg_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4138 ,0x4139, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Neg_Sharp_Kwh) },
		{MD_SIG_Cab_Last1_Neg_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x413A ,0x413B, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Neg_Peak_Kwh) },
		{MD_SIG_Cab_Last1_Neg_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x413C ,0x413D, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Neg_Flat_Kwh) },
		{MD_SIG_Cab_Last1_Neg_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x413E ,0x413F, 0, 1 , (void*)&(pModbus->strMeter[0].fLast1_Neg_Wave_Kwh) },
		{MD_SIG_Cab_Last2_Pos_Sum_Kwh, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4140 ,0x4141, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Pos_Sum_Kwh) },
		{MD_SIG_Cab_Last2_Pos_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4142 ,0x4143, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Pos_Sharp_Kwh) },
		{MD_SIG_Cab_Last2_Pos_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4144 ,0x4145, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Pos_Peak_Kwh) },
		{MD_SIG_Cab_Last2_Pos_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4146 ,0x4147, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Pos_Flat_Kwh) },
		{MD_SIG_Cab_Last2_Pos_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4148 ,0x4149, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Pos_Wave_Kwh) },
		{MD_SIG_Cab_Last2_Neg_Sum_Kwh, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x414A ,0x414B, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Neg_Sum_Kwh) },
		{MD_SIG_Cab_Last2_Neg_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x414C ,0x414D, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Neg_Sharp_Kwh) },
		{MD_SIG_Cab_Last2_Neg_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x414E ,0x414F, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Neg_Peak_Kwh) },
		{MD_SIG_Cab_Last2_Neg_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4150 ,0x4151, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Neg_Flat_Kwh) },
		{MD_SIG_Cab_Last2_Neg_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4152 ,0x4153, 0, 1 , (void*)&(pModbus->strMeter[0].fLast2_Neg_Wave_kwh) },

		{MD_SIG_Grid_PhAVolt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4200 ,0x4201, 0, 1 , (void*)&(pModbus->strMeter[1].fPhaseA_Volt) 		},
		{MD_SIG_Grid_PhBVolt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4202 ,0x4203, 0, 1 , (void*)&(pModbus->strMeter[1].fPhaseB_Volt) 		},
		{MD_SIG_Grid_PhCVolt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4204 ,0x4205, 0, 1 , (void*)&(pModbus->strMeter[1].fPhaseC_Volt) 		},
		{MD_SIG_Grid_PhACurt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4206 ,0x4207, 0, 1 , (void*)&(pModbus->strMeter[1].fPhaseA_Curt) 		},
		{MD_SIG_Grid_PhBCurt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4208 ,0x4209, 0, 1 , (void*)&(pModbus->strMeter[1].fPhaseB_Curt) 		},
		{MD_SIG_Grid_PhCCurt, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x420A ,0x420B, 0, 1 , (void*)&(pModbus->strMeter[1].fPhaseC_Curt) 		},
		{MD_SIG_Grid_Pactive, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x420C ,0x420D, 0, 1 , (void*)&(pModbus->strMeter[1].fActive_Power) 		},
		{MD_SIG_Grid_Qreactive, FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x420E ,0x420F, 0, 1 , (void*)&(pModbus->strMeter[1].fReactive_Power) 	},
		{MD_SIG_Grid_Freq , 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4210 ,0x4211, 0, 1 , (void*)&(pModbus->strMeter[1].f_Freq) 				},
		{MD_SIG_Grid_Factor, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4212 ,0x4213, 0, 1 , (void*)&(pModbus->strMeter[1].f_Factor) 			},
		{MD_SIG_Grid_Pos_Kwh, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4214 ,0x4215, 0, 1 , (void*)&(pModbus->strMeter[1].fPos_Kwh) 			},
		{MD_SIG_Grid_Neg_Kwh, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4216 ,0x4217, 0, 1 , (void*)&(pModbus->strMeter[1].fNeg_Kwh) 			},

		{MD_SIG_Grid_Current_Pos_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4218 ,0x4219, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Pos_Sum_Kwh) },
		{MD_SIG_Grid_Current_Pos_Sharp_Kwh, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x421A ,0x421B, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Pos_Sharp_Kwh) },
		{MD_SIG_Grid_Current_Pos_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x421C ,0x421D, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Pos_Peak_Kwh) },
		{MD_SIG_Grid_Current_Pos_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x421E ,0x421F, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Pos_Flat_Kwh) },
		{MD_SIG_Grid_Current_Pos_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4220 ,0x4221, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Pos_Wave_Kwh) },
		{MD_SIG_Grid_Current_Neg_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4222 ,0x4223, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Neg_Sum_Kwh) },
		{MD_SIG_Grid_Current_Neg_Sharp_Kwh, 	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4224 ,0x4225, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Neg_Sharp_Kwh) },
		{MD_SIG_Grid_Current_Neg_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4226 ,0x4227, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Neg_Peak_Kwh) },
		{MD_SIG_Grid_Current_Neg_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4228 ,0x4229, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Neg_Flat_Kwh) },
		{MD_SIG_Grid_Current_Neg_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x422A ,0x422B, 0, 1 , (void*)&(pModbus->strMeter[1].fCurrent_Neg_Wave_Kwh) },
		{MD_SIG_Grid_Last1_Pos_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x422C ,0x422D, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Pos_Sum_Kwh) },
		{MD_SIG_Grid_Last1_Pos_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x422E ,0x422F, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Pos_Sharp_Kwh) },
		{MD_SIG_Grid_Last1_Pos_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4230 ,0x4231, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Pos_Peak_Kwh) },
		{MD_SIG_Grid_Last1_Pos_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4232 ,0x4233, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Pos_Flat_Kwh) },
		{MD_SIG_Grid_Last1_Pos_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4234 ,0x4235, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Pos_Wave_Kwh) },
		{MD_SIG_Grid_Last1_Neg_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4236 ,0x4237, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Neg_Sum_Kwh) },
		{MD_SIG_Grid_Last1_Neg_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4238 ,0x4239, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Neg_Sharp_Kwh) },
		{MD_SIG_Grid_Last1_Neg_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x423A ,0x423B, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Neg_Peak_Kwh) },
		{MD_SIG_Grid_Last1_Neg_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x423C ,0x423D, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Neg_Flat_Kwh) },
		{MD_SIG_Grid_Last1_Neg_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x423E ,0x423F, 0, 1 , (void*)&(pModbus->strMeter[1].fLast1_Neg_Wave_Kwh) },
		{MD_SIG_Grid_Last2_Pos_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4240 ,0x4241, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Pos_Sum_Kwh) },
		{MD_SIG_Grid_Last2_Pos_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4242 ,0x4243, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Pos_Sharp_Kwh) },
		{MD_SIG_Grid_Last2_Pos_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4244 ,0x4245, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Pos_Peak_Kwh) },
		{MD_SIG_Grid_Last2_Pos_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4246 ,0x4247, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Pos_Flat_Kwh) },
		{MD_SIG_Grid_Last2_Pos_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4248 ,0x4249, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Pos_Wave_Kwh) },
		{MD_SIG_Grid_Last2_Neg_Sum_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x424A ,0x424B, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Neg_Sum_Kwh) },
		{MD_SIG_Grid_Last2_Neg_Sharp_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x424C ,0x424D, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Neg_Sharp_Kwh) },
		{MD_SIG_Grid_Last2_Neg_Peak_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x424E ,0x424F, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Neg_Peak_Kwh) },
		{MD_SIG_Grid_Last2_Neg_Flat_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4250 ,0x4251, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Neg_Flat_Kwh) },
		{MD_SIG_Grid_Last2_Neg_Wave_Kwh, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4252 ,0x4253, 0, 1 , (void*)&(pModbus->strMeter[1].fLast2_Neg_Wave_kwh) },


		{MD_SIG_BAT1_State, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4300 ,0x4301, 0, 1 , (void*)&(pModbus->strBAT[0].fRunState) 				},
		{MD_SIG_BAT1_SOC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4302 ,0x4303, 0, 1 , (void*)&(pModbus->strBAT[0].f_SOC) 					},
		{MD_SIG_BAT1_SOH,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4304 ,0x4305, 0, 1 , (void*)&(pModbus->strBAT[0].f_SOH) 					},
		{MD_SIG_BAT1_NotOnChgRsn,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4306 ,0x4307, 0, 1 , (void*)&(pModbus->strBAT[0].fNotStartChgReason) 		},
		{MD_SIG_BAT1_NotOnDischgRsn,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4308 ,0x4309, 0, 1 , (void*)&(pModbus->strBAT[0].fNotStartDischgReason) 	},
		{MD_SIG_BAT1_Volt, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x430A ,0x430B, 0, 1 , (void*)&(pModbus->strBAT[0].fRack_Volt) 				},
		{MD_SIG_BAT1_Curt,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x430C ,0x430D, 0, 1 , (void*)&(pModbus->strBAT[0].fRack_Curt) 				},
		{MD_SIG_BAT1_MaxChgV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x430E ,0x430F, 0, 1 , (void*)&(pModbus->strBAT[0].fMax_AllowChgVolt) 		},
		{MD_SIG_BAT1_MaxChgA, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4310 ,0x4311, 0, 1 , (void*)&(pModbus->strBAT[0].fMax_AllowChgCurt) 		},
		{MD_SIG_BAT1_MaxDisChgV,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4312 ,0x4313, 0, 1 , (void*)&(pModbus->strBAT[0].fMax_AllowDisChgVolt) 		},
		{MD_SIG_BAT1_MaxDisChgA,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4314 ,0x4315, 0, 1 , (void*)&(pModbus->strBAT[0].fMax_AllowDisChgCurt) 		},
		{MD_SIG_BAT1_MaxCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4316 ,0x4317, 0, 1 , (void*)&(pModbus->strBAT[0].fMax_CellTemp) 			},
		{MD_SIG_BAT1_MinCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4318 ,0x4319, 0, 1 , (void*)&(pModbus->strBAT[0].fMin_CellTemp) 			},
		{MD_SIG_BAT1_MaxCellV,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x431A ,0x431B, 0, 1 , (void*)&(pModbus->strBAT[0].fMax_CellVolt) 			},
		{MD_SIG_BAT1_MinCellV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x431C ,0x431D, 0, 1 , (void*)&(pModbus->strBAT[0].fMin_CellVolt) 			},
		{MD_SIG_BAT1_Pack1PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x431E ,0x431F, 0, 1 , (void*)&(pModbus->strBAT[0].fPosCopperTemp[0]) 		},
		{MD_SIG_BAT1_Pack1NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4320 ,0x4321, 0, 1 , (void*)&(pModbus->strBAT[0].fNegCopperTemp[0]) 		},
		{MD_SIG_BAT1_Pack2PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4322 ,0x4323, 0, 1 , (void*)&(pModbus->strBAT[0].fPosCopperTemp[1]) 		},
		{MD_SIG_BAT1_Pack2NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4324 ,0x4325, 0, 1 , (void*)&(pModbus->strBAT[0].fNegCopperTemp[1]) 		},
		{MD_SIG_BAT1_Pack3PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4326 ,0x4327, 0, 1 , (void*)&(pModbus->strBAT[0].fPosCopperTemp[2]) 		},
		{MD_SIG_BAT1_Pack3NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4328 ,0x4329, 0, 1 , (void*)&(pModbus->strBAT[0].fNegCopperTemp[2]) 		},
		{MD_SIG_BAT1_Pack4PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x432A ,0x432B, 0, 1 , (void*)&(pModbus->strBAT[0].fPosCopperTemp[3]) 		},
		{MD_SIG_BAT1_Pack4NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x432C ,0x432D, 0, 1 , (void*)&(pModbus->strBAT[0].fNegCopperTemp[3]) 		},
		{MD_SIG_BAT1_Pack5PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x432E ,0x432F, 0, 1 , (void*)&(pModbus->strBAT[0].fPosCopperTemp[4]) 		},
		{MD_SIG_BAT1_Pack5NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4330 ,0x4331, 0, 1 , (void*)&(pModbus->strBAT[0].fNegCopperTemp[4]) 		},
		

		{MD_SIG_BAT2_State, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4340 ,0x4341, 0, 1 , (void*)&(pModbus->strBAT[1].fRunState) 				},
		{MD_SIG_BAT2_SOC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4342 ,0x4343, 0, 1 , (void*)&(pModbus->strBAT[1].f_SOC) 					},
		{MD_SIG_BAT2_SOH,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4344 ,0x4345, 0, 1 , (void*)&(pModbus->strBAT[1].f_SOH) 					},
		{MD_SIG_BAT2_NotOnChgRsn,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4346 ,0x4347, 0, 1 , (void*)&(pModbus->strBAT[1].fNotStartChgReason) 		},
		{MD_SIG_BAT2_NotOnDischgRsn,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4348 ,0x4349, 0, 1 , (void*)&(pModbus->strBAT[1].fNotStartDischgReason) 	},
		{MD_SIG_BAT2_Volt, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x434A ,0x434B, 0, 1 , (void*)&(pModbus->strBAT[1].fRack_Volt) 				},
		{MD_SIG_BAT2_Curt,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x434C ,0x434D, 0, 1 , (void*)&(pModbus->strBAT[1].fRack_Curt) 				},
		{MD_SIG_BAT2_MaxChgV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x434E ,0x434F, 0, 1 , (void*)&(pModbus->strBAT[1].fMax_AllowChgVolt) 		},
		{MD_SIG_BAT2_MaxChgA, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4350 ,0x4351, 0, 1 , (void*)&(pModbus->strBAT[1].fMax_AllowChgCurt) 		},
		{MD_SIG_BAT2_MaxDisChgV,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4352 ,0x4353, 0, 1 , (void*)&(pModbus->strBAT[1].fMax_AllowDisChgVolt) 		},
		{MD_SIG_BAT2_MaxDisChgA,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4354 ,0x4355, 0, 1 , (void*)&(pModbus->strBAT[1].fMax_AllowDisChgCurt) 		},
		{MD_SIG_BAT2_MaxCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4356 ,0x4357, 0, 1 , (void*)&(pModbus->strBAT[1].fMax_CellTemp) 			},
		{MD_SIG_BAT2_MinCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4358 ,0x4359, 0, 1 , (void*)&(pModbus->strBAT[1].fMin_CellTemp) 			},
		{MD_SIG_BAT2_MaxCellV,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x435A ,0x435B, 0, 1 , (void*)&(pModbus->strBAT[1].fMax_CellVolt) 			},
		{MD_SIG_BAT2_MinCellV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x435C ,0x435D, 0, 1 , (void*)&(pModbus->strBAT[1].fMin_CellVolt) 			},
		{MD_SIG_BAT2_Pack1PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x435E ,0x435F, 0, 1 , (void*)&(pModbus->strBAT[1].fPosCopperTemp[0]) 		},
		{MD_SIG_BAT2_Pack1NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4360 ,0x4361, 0, 1 , (void*)&(pModbus->strBAT[1].fNegCopperTemp[0]) 		},
		{MD_SIG_BAT2_Pack2PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4362 ,0x4363, 0, 1 , (void*)&(pModbus->strBAT[1].fPosCopperTemp[1]) 		},
		{MD_SIG_BAT2_Pack2NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4364 ,0x4365, 0, 1 , (void*)&(pModbus->strBAT[1].fNegCopperTemp[1]) 		},
		{MD_SIG_BAT2_Pack3PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4366 ,0x4367, 0, 1 , (void*)&(pModbus->strBAT[1].fPosCopperTemp[2]) 		},
		{MD_SIG_BAT2_Pack3NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4368 ,0x4369, 0, 1 , (void*)&(pModbus->strBAT[1].fNegCopperTemp[2]) 		},
		{MD_SIG_BAT2_Pack4PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x436A ,0x436B, 0, 1 , (void*)&(pModbus->strBAT[1].fPosCopperTemp[3]) 		},
		{MD_SIG_BAT2_Pack4NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x436C ,0x436D, 0, 1 , (void*)&(pModbus->strBAT[1].fNegCopperTemp[3]) 		},
		{MD_SIG_BAT2_Pack5PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x436E ,0x436F, 0, 1 , (void*)&(pModbus->strBAT[1].fPosCopperTemp[4]) 		},
		{MD_SIG_BAT2_Pack5NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4370 ,0x4361, 0, 1 , (void*)&(pModbus->strBAT[1].fNegCopperTemp[4]) 		},


		{MD_SIG_BAT3_State, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4380 ,0x4381, 0, 1 , (void*)&(pModbus->strBAT[2].fRunState) 				},
		{MD_SIG_BAT3_SOC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4382 ,0x4383, 0, 1 , (void*)&(pModbus->strBAT[2].f_SOC) 					},
		{MD_SIG_BAT3_SOH,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4384 ,0x4385, 0, 1 , (void*)&(pModbus->strBAT[2].f_SOH) 					},
		{MD_SIG_BAT3_NotOnChgRsn,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4386 ,0x4387, 0, 1 , (void*)&(pModbus->strBAT[2].fNotStartChgReason) 		},
		{MD_SIG_BAT3_NotOnDischgRsn,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4388 ,0x4389, 0, 1 , (void*)&(pModbus->strBAT[2].fNotStartDischgReason) 	},
		{MD_SIG_BAT3_Volt, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x438A ,0x438B, 0, 1 , (void*)&(pModbus->strBAT[2].fRack_Volt) 				},
		{MD_SIG_BAT3_Curt,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x438C ,0x438D, 0, 1 , (void*)&(pModbus->strBAT[2].fRack_Curt) 				},
		{MD_SIG_BAT3_MaxChgV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x438E ,0x438F, 0, 1 , (void*)&(pModbus->strBAT[2].fMax_AllowChgVolt) 		},
		{MD_SIG_BAT3_MaxChgA, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4390 ,0x4391, 0, 1 , (void*)&(pModbus->strBAT[2].fMax_AllowChgCurt) 		},
		{MD_SIG_BAT3_MaxDisChgV,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4392 ,0x4393, 0, 1 , (void*)&(pModbus->strBAT[2].fMax_AllowDisChgVolt) 		},
		{MD_SIG_BAT3_MaxDisChgA,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4394 ,0x4395, 0, 1 , (void*)&(pModbus->strBAT[2].fMax_AllowDisChgCurt) 		},
		{MD_SIG_BAT3_MaxCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4396 ,0x4397, 0, 1 , (void*)&(pModbus->strBAT[2].fMax_CellTemp) 			},
		{MD_SIG_BAT3_MinCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4398 ,0x4399, 0, 1 , (void*)&(pModbus->strBAT[2].fMin_CellTemp) 			},
		{MD_SIG_BAT3_MaxCellV,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x439A ,0x439B, 0, 1 , (void*)&(pModbus->strBAT[2].fMax_CellVolt) 			},
		{MD_SIG_BAT3_MinCellV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x439C ,0x439D, 0, 1 , (void*)&(pModbus->strBAT[2].fMin_CellVolt) 			},
		{MD_SIG_BAT3_Pack1PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x439E ,0x439F, 0, 1 , (void*)&(pModbus->strBAT[2].fPosCopperTemp[0]) 		},
		{MD_SIG_BAT3_Pack1NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43A0 ,0x43A1, 0, 1 , (void*)&(pModbus->strBAT[2].fNegCopperTemp[0]) 		},
		{MD_SIG_BAT3_Pack2PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43A2 ,0x43A3, 0, 1 , (void*)&(pModbus->strBAT[2].fPosCopperTemp[1]) 		},
		{MD_SIG_BAT3_Pack2NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43A4 ,0x43A5, 0, 1 , (void*)&(pModbus->strBAT[2].fNegCopperTemp[1]) 		},
		{MD_SIG_BAT3_Pack3PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43A6 ,0x43A7, 0, 1 , (void*)&(pModbus->strBAT[2].fPosCopperTemp[2]) 		},
		{MD_SIG_BAT3_Pack3NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43A8 ,0x43A9, 0, 1 , (void*)&(pModbus->strBAT[2].fNegCopperTemp[2]) 		},
		{MD_SIG_BAT3_Pack4PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43AA ,0x43AB, 0, 1 , (void*)&(pModbus->strBAT[2].fPosCopperTemp[3]) 		},
		{MD_SIG_BAT3_Pack4NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43AC ,0x43AD, 0, 1 , (void*)&(pModbus->strBAT[2].fNegCopperTemp[3]) 		},
		{MD_SIG_BAT3_Pack5PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43AE ,0x43AF, 0, 1 , (void*)&(pModbus->strBAT[2].fPosCopperTemp[4]) 		},
		{MD_SIG_BAT3_Pack5NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43B0 ,0x43B1, 0, 1 , (void*)&(pModbus->strBAT[2].fNegCopperTemp[4]) 		},


		{MD_SIG_BAT4_State, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43C0 ,0x43C1, 0, 1 , (void*)&(pModbus->strBAT[3].fRunState) 				},
		{MD_SIG_BAT4_SOC,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43C2 ,0x43C3, 0, 1 , (void*)&(pModbus->strBAT[3].f_SOC) 					},
		{MD_SIG_BAT4_SOH,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43C4 ,0x43C5, 0, 1 , (void*)&(pModbus->strBAT[3].f_SOH) 					},
		{MD_SIG_BAT4_NotOnChgRsn,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43C6 ,0x43C7, 0, 1 , (void*)&(pModbus->strBAT[3].fNotStartChgReason) 		},
		{MD_SIG_BAT4_NotOnDischgRsn,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43C8 ,0x43C9, 0, 1 , (void*)&(pModbus->strBAT[3].fNotStartDischgReason) 	},
		{MD_SIG_BAT4_Volt, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43CA ,0x43CB, 0, 1 , (void*)&(pModbus->strBAT[3].fRack_Volt) 				},
		{MD_SIG_BAT4_Curt,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43CC ,0x43CD, 0, 1 , (void*)&(pModbus->strBAT[3].fRack_Curt) 				},
		{MD_SIG_BAT4_MaxChgV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43CE ,0x43CF, 0, 1 , (void*)&(pModbus->strBAT[3].fMax_AllowChgVolt) 		},
		{MD_SIG_BAT4_MaxChgA, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43D0 ,0x43D1, 0, 1 , (void*)&(pModbus->strBAT[3].fMax_AllowChgCurt) 		},
		{MD_SIG_BAT4_MaxDisChgV,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43D2 ,0x43D3, 0, 1 , (void*)&(pModbus->strBAT[3].fMax_AllowDisChgVolt) 		},
		{MD_SIG_BAT4_MaxDisChgA,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43D4 ,0x43D5, 0, 1 , (void*)&(pModbus->strBAT[3].fMax_AllowDisChgCurt) 		},
		{MD_SIG_BAT4_MaxCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43D6 ,0x43D7, 0, 1 , (void*)&(pModbus->strBAT[3].fMax_CellTemp) 			},
		{MD_SIG_BAT4_MinCellT, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43D8 ,0x43D9, 0, 1 , (void*)&(pModbus->strBAT[3].fMin_CellTemp) 			},
		{MD_SIG_BAT4_MaxCellV,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43DA ,0x43DB, 0, 1 , (void*)&(pModbus->strBAT[3].fMax_CellVolt) 			},
		{MD_SIG_BAT4_MinCellV, 		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43DC ,0x43DD, 0, 1 , (void*)&(pModbus->strBAT[3].fMin_CellVolt) 			},
		{MD_SIG_BAT4_Pack1PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43DE ,0x43DF, 0, 1 , (void*)&(pModbus->strBAT[3].fPosCopperTemp[0]) 		},
		{MD_SIG_BAT4_Pack1NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43E0 ,0x43E1, 0, 1 , (void*)&(pModbus->strBAT[3].fNegCopperTemp[0]) 		},
		{MD_SIG_BAT4_Pack2PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43E2 ,0x43E3, 0, 1 , (void*)&(pModbus->strBAT[3].fPosCopperTemp[1]) 		},
		{MD_SIG_BAT4_Pack2NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43E4 ,0x43E5, 0, 1 , (void*)&(pModbus->strBAT[3].fNegCopperTemp[1]) 		},
		{MD_SIG_BAT4_Pack3PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43E6 ,0x43E7, 0, 1 , (void*)&(pModbus->strBAT[3].fPosCopperTemp[2]) 		},
		{MD_SIG_BAT4_Pack3NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43E8 ,0x43E9, 0, 1 , (void*)&(pModbus->strBAT[3].fNegCopperTemp[2]) 		},
		{MD_SIG_BAT4_Pack4PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43EA ,0x43EB, 0, 1 , (void*)&(pModbus->strBAT[3].fPosCopperTemp[3]) 		},
		{MD_SIG_BAT4_Pack4NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43EC ,0x43ED, 0, 1 , (void*)&(pModbus->strBAT[3].fNegCopperTemp[3]) 		},
		{MD_SIG_BAT4_Pack5PosCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43EE ,0x43EF, 0, 1 , (void*)&(pModbus->strBAT[3].fPosCopperTemp[4]) 		},
		{MD_SIG_BAT4_Pack5NegCopperT,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x43F0 ,0x43F1, 0, 1 , (void*)&(pModbus->strBAT[3].fNegCopperTemp[4]) 		},

		{MD_SIG_PCS1_WorkMode,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4500 ,0x4501, 0, 1 ,  (void*)&(pModbus->strPCS.emWorkMode_Grp1) 	},
		{MD_SIG_PCS1_Vout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4502 ,0x4503, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputVolt_Grp1) 	}, 
		{MD_SIG_PCS1_Aout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4504 ,0x4505, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputCurt_Grp1) 	},
		{MD_SIG_PCS1_Resv1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4506 ,0x4507, 0, 1 ,  (void*)&(pModbus->strPCS.fResv1_Grp1) 		},
		{MD_SIG_PCS1_Resv2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4508 ,0x4509, 0, 1 ,  (void*)&(pModbus->strPCS.fResv2_Grp1) 		},
		{MD_SIG_PCS2_WorkMode,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x450A ,0x450B, 0, 1 ,  (void*)&(pModbus->strPCS.emWorkMode_Grp2) 	},
		{MD_SIG_PCS2_Vout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x450C ,0x450D, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputVolt_Grp2) 	},
		{MD_SIG_PCS2_Aout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x450E ,0x450F, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputCurt_Grp2) 	},
		{MD_SIG_PCS2_Resv1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4510 ,0x4511, 0, 1 ,  (void*)&(pModbus->strPCS.fResv1_Grp2) 		},
		{MD_SIG_PCS2_Resv2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4512 ,0x4513, 0, 1 ,  (void*)&(pModbus->strPCS.fResv2_Grp2) 		},
		{MD_SIG_PCS3_WorkMode,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4514 ,0x4515, 0, 1 ,  (void*)&(pModbus->strPCS.emWorkMode_Grp3) 	},
		{MD_SIG_PCS3_Vout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4516 ,0x4517, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputVolt_Grp3) 	},
		{MD_SIG_PCS3_Aout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4518 ,0x4519, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputCurt_Grp3) 	},
		{MD_SIG_PCS3_Resv1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x451A ,0x451B, 0, 1 ,  (void*)&(pModbus->strPCS.fResv1_Grp3) 		},
		{MD_SIG_PCS3_Resv2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x451C ,0x451D, 0, 1 ,  (void*)&(pModbus->strPCS.fResv2_Grp3) 		},
		{MD_SIG_PCS4_WorkMode,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x451E ,0x451F, 0, 1 ,  (void*)&(pModbus->strPCS.emWorkMode_Grp4) 	},
		{MD_SIG_PCS4_Vout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4520 ,0x4521, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputVolt_Grp4) 	},
		{MD_SIG_PCS4_Aout, 			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4522 ,0x4523, 0, 1 ,  (void*)&(pModbus->strPCS.fOutputCurt_Grp4) 	},
		{MD_SIG_PCS4_Resv1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4524 ,0x4525, 0, 1 ,  (void*)&(pModbus->strPCS.fResv1_Grp4) 		},
		{MD_SIG_PCS4_Resv2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4526 ,0x4527, 0, 1 ,  (void*)&(pModbus->strPCS.fResv2_Grp4) 		},

		// 4600
		{MD_SIG_STS_OnOffPower,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x4600 ,0x4600, 0, 1 ,  (void*)&(pModbus->strSTS.usSTS_OnOffStatus) 	},
		{MD_SIG_STS_OnOffGridStat,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x4601 ,0x4601, 0, 1 ,  (void*)&(pModbus->strSTS.usSTS_OnOffGrid) 	},
		{MD_SIG_STS_OnOffGridBusStat,FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x4602 ,0x4602, 0, 1 ,  (void*)&(pModbus->strSTS.usSTS_BusOnOffGrid) 	},
		{MD_SIG_STS_FaultStat,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x4603 ,0x4603, 0, 1 ,  (void*)&(pModbus->strSTS.usSTS_FaultStatus) 	},
		{MD_SIG_STS_GridAlarmStat,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x4604 ,0x4604, 0, 1 ,  (void*)&(pModbus->strSTS.usSTS_GridAlarmStatus)},
 
		{MD_SIG_Ctrl_Lock,  		TRUE,  200,		DAITYPE_CTRL_PARAMVALUE_RW,	 198,		MD_GET_VAL_USHORT,	1, 1, 0x6001 ,0x6001, 0, 1 ,(void*)&(pModbus->strCtrl.emSysLock) 	},
		{MD_SIG_Ctrl_Reboot,  		TRUE,  1,		DAITYPE_CTRL_PARAMVALUE_RW,	 2,		MD_GET_VAL_USHORT,	1, 1, 0x6002 ,0x6002, 0, 1 ,(void*)&(pModbus->strCtrl.emSWReboot) 	},
		{MD_SIG_Ctrl_CtrlMode,  	FALSE, 1001,	DAITYPE_SETT_PARAMVALUE_RW,3902,	MD_GET_VAL_USHORT,	1, 1, 0x6003 ,0x6003, 0, 1 ,(void*)&(pModbus->strCtrl.emCtrlMode) 	},
		{MD_SIG_Ctrl_RunMode,  		FALSE, 1001,	DAITYPE_SETT_PARAMVALUE_RW,3907,	MD_GET_VAL_USHORT,	1, 1, 0x6004 ,0x6004, 0, 1 ,(void*)&(pModbus->strCtrl.emRunMode) 	},
		{MD_SIG_Ctrl_ActivePower,  	FALSE, 1001,	DAITYPE_SETT_PARAMVALUE_RW,3901,	MD_GET_VAL_FLOAT,	1, 2, 0x6005 ,0x6006, 0, 1 ,(void*)&(pModbus->strCtrl.fActivePower) },
		{MD_SIG_Ctrl_ActivePower_Start, FALSE, 1001,	DAITYPE_SETT_PARAMVALUE_RW,3901,	MD_GET_VAL_FLOAT,	1, 2, 0x6021 ,0x6022, 0, 1 ,(void*)&(pModbus->strCtrl.fActivePower) },

		{MD_SIG_SectorNum,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6200 ,0x6200, 0, 1 ,(void*)&(pModbus->strLocal_Ems.nEmsSectorNum) 					},
		
		{MD_SIG_tmStart_0,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6201 ,0x6201, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[0].tmStart) 			},
		{MD_SIG_StopSOC_0,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6202 ,0x6202, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[0].nStopSOC) 			},
		{MD_SIG_PbatLmt_0,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6203 ,0x6203, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[0].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_0, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6204 ,0x6204, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[0].nActvPwrLmt) 		},
		{MD_SIG_QLmt_0,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6205 ,0x6205, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[0].nReActvPwrLmt) 	},
 		
		{MD_SIG_tmStart_1,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6206 ,0x6206, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[1].tmStart) 			},
		{MD_SIG_StopSOC_1,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6207 ,0x6207, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[1].nStopSOC) 			},
		{MD_SIG_PbatLmt_1,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6208 ,0x6208, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[1].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_1, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6209 ,0x6209, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[1].nActvPwrLmt) 		},
		{MD_SIG_QLmt_1,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x620A ,0x620A, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[1].nReActvPwrLmt) 	},

		{MD_SIG_tmStart_2,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x620B ,0x620B, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[2].tmStart) 			},
		{MD_SIG_StopSOC_2,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x620C ,0x620C, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[2].nStopSOC) 			},
		{MD_SIG_PbatLmt_2,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x621D ,0x621D, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[2].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_2, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x621E ,0x621E, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[2].nActvPwrLmt) 		},
		{MD_SIG_QLmt_2,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x621F ,0x621F, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[2].nReActvPwrLmt) 	},
 
		{MD_SIG_tmStart_3,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6220 ,0x6220, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[3].tmStart) 			},
		{MD_SIG_StopSOC_3,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6221 ,0x6221, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[3].nStopSOC) 			},
		{MD_SIG_PbatLmt_3,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6222 ,0x6222, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[3].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_3, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6223 ,0x6223, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[3].nActvPwrLmt) 		},
		{MD_SIG_QLmt_3,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6224 ,0x6224, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[3].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_4,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6225 ,0x6225, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[4].tmStart) 			},
		{MD_SIG_StopSOC_4,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6226 ,0x6226, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[4].nStopSOC) 			},
		{MD_SIG_PbatLmt_4,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6227 ,0x6227, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[4].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_4, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6228 ,0x6228, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[4].nActvPwrLmt) 		},
		{MD_SIG_QLmt_4,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6229 ,0x6229, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[4].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_5,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x622A ,0x622A, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[5].tmStart) 			},
		{MD_SIG_StopSOC_5,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x622B ,0x622B, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[5].nStopSOC) 			},
		{MD_SIG_PbatLmt_5,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x622C ,0x622C, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[5].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_5, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x622D ,0x622D, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[5].nActvPwrLmt) 		},
		{MD_SIG_QLmt_5,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x622E ,0x622E, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[5].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_6,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x622F ,0x622F, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[6].tmStart) 			},
		{MD_SIG_StopSOC_6,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6230 ,0x6230, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[6].nStopSOC) 			},
		{MD_SIG_PbatLmt_6,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6231 ,0x6231, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[6].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_6, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6232 ,0x6232, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[6].nActvPwrLmt) 		},
		{MD_SIG_QLmt_6,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6233 ,0x6233, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[6].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_7,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6234 ,0x6234, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[7].tmStart) 			},
		{MD_SIG_StopSOC_7,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6235 ,0x6235, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[7].nStopSOC) 			},
		{MD_SIG_PbatLmt_7,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6236 ,0x6236, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[7].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_7, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6237 ,0x6237, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[7].nActvPwrLmt) 		},
		{MD_SIG_QLmt_7,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6238 ,0x6238, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[7].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_8,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6239 ,0x6239, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[8].tmStart) 			},
		{MD_SIG_StopSOC_8,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x623A ,0x623A, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[8].nStopSOC) 			},
		{MD_SIG_PbatLmt_8,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x623B ,0x623B, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[8].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_8, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x623C ,0x623C, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[8].nActvPwrLmt) 		},
		{MD_SIG_QLmt_8,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x623D ,0x623D, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[8].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_9,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x623E ,0x623E, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[9].tmStart) 			},
		{MD_SIG_StopSOC_9,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x623F ,0x623F, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[9].nStopSOC) 			},
		{MD_SIG_PbatLmt_9,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6240 ,0x6240, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[9].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_9, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6241 ,0x6241, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[9].nActvPwrLmt) 		},
		{MD_SIG_QLmt_9,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6242 ,0x6242, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[9].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_10,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6243 ,0x6243, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[10].tmStart) 			},
		{MD_SIG_StopSOC_10,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6244 ,0x6244, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[10].nStopSOC) 		},
		{MD_SIG_PbatLmt_10,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6245 ,0x6245, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[10].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_10, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6246 ,0x6246, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[10].nActvPwrLmt) 		},
		{MD_SIG_QLmt_10,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x6247 ,0x6247, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[10].nReActvPwrLmt) 	},
 
 		{MD_SIG_tmStart_11,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6248 ,0x6248, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[11].tmStart) 			},
		{MD_SIG_StopSOC_11,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x6249 ,0x6249, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[11].nStopSOC) 		},
		{MD_SIG_PbatLmt_11,  		FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_USHORT,	1, 1, 0x624A ,0x624A, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[11].nBattGrpPwrLmt) 	},
		{MD_SIG_PLmt_11, 			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x624B ,0x624B, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[11].nActvPwrLmt) 		},
		{MD_SIG_QLmt_11,  			FALSE, 200,DAITYPE_SETT_PARAMVALUE_RW,201,	MD_GET_VAL_SHORT,	1, 1, 0x624C ,0x624C, 0, 1 ,(void*)&(pModbus->strLocal_Ems.arryEmsParam[11].nReActvPwrLmt) 	},
 
 		{MD_SIG_DateKwh,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6400 ,0x6401, 0, 1 ,  (void*)&(pModbus->strKWh.fDate) 						},
		{MD_SIG_Cab_PosDay,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6402 ,0x6403, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_PosDailyKwh) 			},
		{MD_SIG_Cab_NegDay,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6404 ,0x6405, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_NegDailyKwh) 			},
		{MD_SIG_Cab_PosDayShp,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6406 ,0x6407, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_PosDailySharpKwh) 		},
		{MD_SIG_Cab_PosDayPeak,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6408 ,0x6409, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_PosDailyPeakKwh) 		},
		{MD_SIG_Cab_PosDayFlat,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x640A ,0x640B, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_PosDailyFlatKwh) 		},
		{MD_SIG_Cab_PosDayValy,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x640C ,0x640D, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_PosDailyValleyKwh) 		},
		{MD_SIG_Cab_NegDayShp,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x640E ,0x640F, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_NegDailySharpKwh) 		},
		{MD_SIG_Cab_NegDayPeak,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6410 ,0x6411, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_NegDailyPeakKwh) 		},
		{MD_SIG_Cab_NegDayFlat,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6412 ,0x6413, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_NegDailyFlatKwh) 		},
		{MD_SIG_Cab_NegDayValy,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6414 ,0x6415, 0, 1 ,  (void*)&(pModbus->strKWh.fCab_NegDailyValleyKwh) 		},
		{MD_SIG_BAT1_DayChg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6416 ,0x6417, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT1_DailyChgKwh) 			},
		{MD_SIG_BAT1_DayDischg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6418 ,0x6419, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT1_DailyDischgKwh) 		},
		{MD_SIG_BAT2_DayChg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x641A ,0x641B, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT2_DailyChgKwh) 			},
		{MD_SIG_BAT2_DayDischg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x641C ,0x641D, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT2_DailyDischgKwh) 		},
		{MD_SIG_BAT3_DayChg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x641E ,0x641F, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT3_DailyChgKwh) 			},
		{MD_SIG_BAT3_DayDischg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6420 ,0x6421, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT3_DailyDischgKwh) 		},
		{MD_SIG_BAT4_DayChg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6422 ,0x6423, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT4_DailyChgKwh) 			},
		{MD_SIG_BAT4_DayDischg,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6424 ,0x6425, 0, 1 ,  (void*)&(pModbus->strKWh.fBAT4_DailyDischgKwh) 		},

		//0x6800
		{MD_SIG_BAT1_DoorOpen,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6800 ,0x6800, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emDoorOpen) 				},
		{MD_SIG_BAT1_FireTubeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6801 ,0x6801, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emFireTubeAlarm) 			},
		{MD_SIG_BAT1_SmokeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6802 ,0x6802, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emSmokeAlarm) 			},
		{MD_SIG_BAT1_GasAlarm,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6803 ,0x6803, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emGasAlarm) 				},
		{MD_SIG_BAT1_FloodAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6804 ,0x6804, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emFloodAlarm) 			},
		{MD_SIG_BAT1_AirCondHWAlarm,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6805 ,0x6805, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emAirCondHWAlarm) 		},
		{MD_SIG_BAT1_AirMachineState,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6806 ,0x6806, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emAirMachineState) 		},
		{MD_SIG_BAT1_WholeMachineStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6807 ,0x6807, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emWholeMachineStatus) 	},
		{MD_SIG_BAT1_InternalFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6808 ,0x6808, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emInternalFanStatus) 		},
		{MD_SIG_BAT1_OutsideFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6809 ,0x6809, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emOutsideFanStatus) 		},
		{MD_SIG_BAT1_CompressorStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x680A ,0x680A, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emCompressorStatus) 		},
		{MD_SIG_BAT1_ElectricHeatStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x680B ,0x680B, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emElectricHeatStatus) 	},
		{MD_SIG_BAT1_ForceAutoMode,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x680C ,0x680C, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].emForceAutoMode) 			},
		{MD_SIG_BAT1_CabTemp,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x680D ,0x680E, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].fCabTemp) 				},
		{MD_SIG_BAT1_CabHumi,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x680F ,0x6810, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].fCabHumi) 				},
		{MD_SIG_BAT1_Sensor1Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6811 ,0x6812, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].fSensor1Humi) 			},
		{MD_SIG_BAT1_Sensor1Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6813 ,0x6814, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].fSensor1Temp) 			},
		{MD_SIG_BAT1_Sensor2Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6815 ,0x6816, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].fSensor2Humi) 			},
		{MD_SIG_BAT1_Sensor2Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6817 ,0x6818, 0, 1 ,  (void*)&(pModbus->strThermalFire[0].fSensor2Temp) 			},

		//0x6900
		{MD_SIG_BAT2_DoorOpen,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6900 ,0x6900, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emDoorOpen) 				},
		{MD_SIG_BAT2_FireTubeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6901 ,0x6901, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emFireTubeAlarm) 			},
		{MD_SIG_BAT2_SmokeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6902 ,0x6902, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emSmokeAlarm) 			},
		{MD_SIG_BAT2_GasAlarm,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6903 ,0x6903, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emGasAlarm) 				},
		{MD_SIG_BAT2_FloodAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6904 ,0x6904, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emFloodAlarm) 			},
		{MD_SIG_BAT2_AirCondHWAlarm,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6905 ,0x6905, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emAirCondHWAlarm) 		},
		{MD_SIG_BAT2_AirMachineState,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6906 ,0x6906, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emAirMachineState) 		},
		{MD_SIG_BAT2_WholeMachineStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6907 ,0x6907, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emWholeMachineStatus) 	},
		{MD_SIG_BAT2_InternalFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6908 ,0x6908, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emInternalFanStatus) 		},
		{MD_SIG_BAT2_OutsideFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x6909 ,0x6909, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emOutsideFanStatus) 		},
		{MD_SIG_BAT2_CompressorStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x690A ,0x690A, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emCompressorStatus) 		},
		{MD_SIG_BAT2_ElectricHeatStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x690B ,0x690B, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emElectricHeatStatus) 	},
		{MD_SIG_BAT2_ForceAutoMode,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x690C ,0x690C, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].emForceAutoMode) 			},
		{MD_SIG_BAT2_CabTemp,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x690D ,0x690E, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].fCabTemp) 				},
		{MD_SIG_BAT2_CabHumi,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x690F ,0x6910, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].fCabHumi) 				},
		{MD_SIG_BAT2_Sensor1Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6911 ,0x6912, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].fSensor1Humi) 			},
		{MD_SIG_BAT2_Sensor1Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6913 ,0x6914, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].fSensor1Temp) 			},
		{MD_SIG_BAT2_Sensor2Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6915 ,0x6916, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].fSensor2Humi) 			},
		{MD_SIG_BAT2_Sensor2Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6917 ,0x6918, 0, 1 ,  (void*)&(pModbus->strThermalFire[1].fSensor2Temp) 			},

		//0x7000
		{MD_SIG_BAT3_DoorOpen,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7000 ,0x7000, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emDoorOpen) 				},
		{MD_SIG_BAT3_FireTubeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7001 ,0x7001, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emFireTubeAlarm) 			},
		{MD_SIG_BAT3_SmokeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7002 ,0x7002, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emSmokeAlarm) 			},
		{MD_SIG_BAT3_GasAlarm,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7003 ,0x7003, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emGasAlarm) 				},
		{MD_SIG_BAT3_FloodAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7004 ,0x7004, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emFloodAlarm) 			},
		{MD_SIG_BAT3_AirCondHWAlarm,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7005 ,0x7005, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emAirCondHWAlarm) 		},
		{MD_SIG_BAT3_AirMachineState,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7006 ,0x7006, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emAirMachineState) 		},
		{MD_SIG_BAT3_WholeMachineStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7007 ,0x7007, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emWholeMachineStatus) 	},
		{MD_SIG_BAT3_InternalFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7008 ,0x7008, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emInternalFanStatus) 		},
		{MD_SIG_BAT3_OutsideFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7009 ,0x7009, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emOutsideFanStatus) 		},
		{MD_SIG_BAT3_CompressorStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x700A ,0x700A, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emCompressorStatus) 		},
		{MD_SIG_BAT3_ElectricHeatStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x700B ,0x700B, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emElectricHeatStatus) 	},
		{MD_SIG_BAT3_ForceAutoMode,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x700C ,0x700C, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].emForceAutoMode) 			},
		{MD_SIG_BAT3_CabTemp,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x700D ,0x700E, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].fCabTemp) 				},
		{MD_SIG_BAT3_CabHumi,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x700F ,0x7010, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].fCabHumi) 				},
		{MD_SIG_BAT3_Sensor1Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7011 ,0x7012, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].fSensor1Humi) 			},
		{MD_SIG_BAT3_Sensor1Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7013 ,0x7014, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].fSensor1Temp) 			},
		{MD_SIG_BAT3_Sensor2Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7015 ,0x7016, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].fSensor2Humi) 			},
		{MD_SIG_BAT3_Sensor2Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7017 ,0x7018, 0, 1 ,  (void*)&(pModbus->strThermalFire[2].fSensor2Temp) 			},

		//0x7100
		{MD_SIG_BAT4_DoorOpen,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7100 ,0x7100, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emDoorOpen) 				},
		{MD_SIG_BAT4_FireTubeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7101 ,0x7101, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emFireTubeAlarm) 			},
		{MD_SIG_BAT4_SmokeAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7102 ,0x7102, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emSmokeAlarm) 			},
		{MD_SIG_BAT4_GasAlarm,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7103 ,0x7103, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emGasAlarm) 				},
		{MD_SIG_BAT4_FloodAlarm,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7104 ,0x7104, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emFloodAlarm) 			},
		{MD_SIG_BAT4_AirCondHWAlarm,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7105 ,0x7105, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emAirCondHWAlarm) 		},
		{MD_SIG_BAT4_AirMachineState,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7106 ,0x7106, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emAirMachineState) 		},
		{MD_SIG_BAT4_WholeMachineStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7107 ,0x7107, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emWholeMachineStatus) 	},
		{MD_SIG_BAT4_InternalFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7108 ,0x7108, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emInternalFanStatus) 		},
		{MD_SIG_BAT4_OutsideFanStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x7109 ,0x7109, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emOutsideFanStatus) 		},
		{MD_SIG_BAT4_CompressorStatus,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x710A ,0x710A, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emCompressorStatus) 		},
		{MD_SIG_BAT4_ElectricHeatStatus,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x710B ,0x710B, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emElectricHeatStatus) 	},
		{MD_SIG_BAT4_ForceAutoMode,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_USHORT,	1, 1, 0x710C ,0x710C, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].emForceAutoMode) 			},
		{MD_SIG_BAT4_CabTemp,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x710D ,0x710E, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].fCabTemp) 				},
		{MD_SIG_BAT4_CabHumi,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x710F ,0x7110, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].fCabHumi) 				},
		{MD_SIG_BAT4_Sensor1Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7111 ,0x7112, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].fSensor1Humi) 			},
		{MD_SIG_BAT4_Sensor1Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7113 ,0x7114, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].fSensor1Temp) 			},
		{MD_SIG_BAT4_Sensor2Humi,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7115 ,0x7116, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].fSensor2Humi) 			},
		{MD_SIG_BAT4_Sensor2Temp,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x7117 ,0x7118, 0, 1 ,  (void*)&(pModbus->strThermalFire[3].fSensor2Temp) 			},
		
		//0X7200~0x7B5F is used to cell_Volt; 0x7B60~0x84BF is used to cell_Temp;
 	};

	MD_DATA_ITEM allItems[3000];

	memcpy(allItems, mdDataItems, sizeof(mdDataItems));
	idx += ITEM_OF(mdDataItems);

	addCellItems(allItems, &idx,pModbus);
	printf("idx = [%d]\n",idx);

	pModbus->pmdDataItems = NEW(MD_DATA_ITEM,ITEM_OF(allItems));
	if(pModbus->pmdDataItems == NULL)
	{
		return ERR_CFG_NO_MEMORY;
	}

	memcpy(pModbus->pmdDataItems , allItems, sizeof(allItems));
	pModbus->nDataItemNum = idx;//ITEM_OF(allItems);
	
	//----------------Initalial Setting Parameter-------------------//
	ZERO_POBJS(&pModbus->emsPlan, 1);
	valGet.nGetSize = sizeof(A_BLOB_EMS_CONFIG_PLAN);
	pModbus->emsPlan.emBlobType = BLOB_TYPE_EMS_CONFIG_PLAN;
	valGet.val.blobValue = (BYTE*)&pModbus->emsPlan;
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
		200, 201, &valGet, sizeof(PARAM_VAL_GET));
	if (err != ERRCODE_DAI_OK)
	{
		TRACEX("ERROR: Backend settings Get Failed!\n");
		return ERR_CFG_FAIL;
	}

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW,
			500, 502, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->nCfgBMSNum = valGet.val.ulValue;
	if(pModbus->nCfgBMSNum > MAX_SINGLE_STBATT_NUM)
	{
		pModbus->nCfgBMSNum = MAX_SINGLE_STBATT_NUM;
	}

    memset(&pModbus->strC2S_Ems, 0 , sizeof(STR_PARAM_EMS));
    memset(&pModbus->strLocal_Ems, 0 , sizeof(STR_PARAM_EMS));

	return ERR_CFG_OK;
}

void Refresh_SigMapData(REPORTER *pReporter)
{
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;

	int 				n = 0,m = 0;
	int 				nRate = 1;
	UINT 				nInterval_Min = 0;
	PARAM_VAL_GET		valGet;
	ERRCODE_DAI			err;

	//-------------------Get System Status DataInfo-----------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 4, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSys_SigMap.emSysAvaliaStatu = valGet.val.emValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				1, 6, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSys_SigMap.emSysFailReason = valGet.val.emValue;

	pModbus->strSys_SigMap.emOnOffGrid = 0;//0-OnGrid, 1-OffGrid

	pModbus->strSys_SigMap.nBatCfgNum = (WORD)pModbus->nCfgBMSNum;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				200, 102, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSys_SigMap.emEmsCRC = valGet.val.emValue;
	
	pModbus->strSys_SigMap.ulSW_Ver = 20250922;//20230714

	ZERO_POBJS(&pModbus->clientInfoSum, 1);
	pModbus->clientInfoSum.emBlobType = BLOB_TYPE_SEMS_CLIENTNFO_SUM;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CLIENTINFO_SUM);
	valGet.val.blobValue = (BYTE*)&pModbus->clientInfoSum;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3900, &valGet, sizeof(PARAM_VAL_GET));

	pModbus->strSys_SigMap.emMaxSoc = pModbus->clientInfoSum.nSOCMaxSum;
	pModbus->strSys_SigMap.emMinSoc = pModbus->clientInfoSum.nSOCMinSum;
	pModbus->strSys_SigMap.fTotalSoc_old = pModbus->clientInfoSum.fSOCSum;

	// SEMES
	pModbus->strSys_SigMap.usEmsCtlMode					=	pModbus->clientInfoSum.emEMSCtrlMode_REMS;
	pModbus->strSys_SigMap.fTotalActPowTgt				=	pModbus->clientInfoSum.fActvPwrReq_REMS;
	pModbus->strSys_SigMap.usSysStatus					=	pModbus->clientInfoSum.emCCUStateSum;
	pModbus->strSys_SigMap.usRunMode					=	pModbus->clientInfoSum.emACDC_WorkStatuSum;
	pModbus->strSys_SigMap.fTotalSoc					=	pModbus->clientInfoSum.fSOCSum;
	pModbus->strSys_SigMap.fTotalActPow_fromCabMeter	=	pModbus->clientInfoSum.f_PcabMeterSum;
	pModbus->strSys_SigMap.fSysCtlMode_fromPcs			=	pModbus->clientInfoSum.fActvPwr_PCSGrpSum;
	pModbus->strSys_SigMap.fTotalMaxChgPowLimit			=	pModbus->clientInfoSum.fMax_Chrg_PwrAbilitySum;
	pModbus->strSys_SigMap.fTotalMaxDishgPowLimit		=	pModbus->clientInfoSum.fMax_DisChrg_PwrAbilitySum;
	pModbus->strSys_SigMap.fTotalChgkWh					=	pModbus->clientInfoSum.fChrgedEnergySum;
	pModbus->strSys_SigMap.fTotalOutputCurr				=	pModbus->clientInfoSum.fRealTime_MaxCurrSum;

	//---------------------- End ----------------------//

	ZERO_POBJS(&pModbus->esRunData, 1);
	pModbus->esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&pModbus->esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet, sizeof(PARAM_VAL_GET));

	//------------ Refresh RealTime Power----------------//
	pModbus->strSys_SigMap.fPgridMeter 		= pModbus->esRunData.f_PgridMeter;
 	pModbus->strSys_SigMap.fPcabMeter  		= pModbus->esRunData.f_PcabMeter;
	pModbus->strSys_SigMap.fPauxMeter 		= pModbus->esRunData.f_PauxMeter;
	pModbus->strSys_SigMap.fPoffGridMeter	= pModbus->esRunData.f_PoffGridMeter;
	pModbus->strSys_SigMap.fPmppt			= pModbus->esRunData.fTotalMPPT_Power;

	//----------- Refresh Cabinet Meter YC CabMeter SigValue -------------------//

	pModbus->strMeter[0].fPhaseA_Volt = pModbus->esRunData.fACVolt_L1;
	pModbus->strMeter[0].fPhaseB_Volt = pModbus->esRunData.fACVolt_L2;
	pModbus->strMeter[0].fPhaseC_Volt = pModbus->esRunData.fACVolt_L3;
	pModbus->strMeter[0].fPhaseA_Curt = pModbus->esRunData.fACCurr_L1;
	pModbus->strMeter[0].fPhaseB_Curt = pModbus->esRunData.fACCurr_L2;
	pModbus->strMeter[0].fPhaseC_Curt = pModbus->esRunData.fACCurr_L3;
 	pModbus->strMeter[0].fActive_Power = pModbus->esRunData.f_PcabMeter;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 404, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fReactive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 426, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].f_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 424, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].f_Freq = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 411, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fPos_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 422, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fNeg_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1491, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Pos_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1492, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Pos_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1493, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Pos_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1494, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Pos_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1495, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Pos_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1496, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Neg_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1497, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Neg_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1498, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Neg_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1499, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Neg_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1500, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fCurrent_Neg_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1501, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Pos_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1502, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Pos_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1503, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Pos_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1504, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Pos_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1505, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Pos_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1506, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Neg_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1507, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Neg_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1508, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Neg_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1509, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Neg_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1510, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast1_Neg_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1511, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Pos_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1512, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Pos_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1513, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Pos_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1514, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Pos_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1515, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Pos_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1516, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Neg_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1517, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Neg_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1518, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Neg_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1519, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Neg_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		407, 1520, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[0].fLast2_Neg_Wave_kwh = valGet.val.fValue;

	//----------------------------- Grid AC Meter ----------------------------//
	pModbus->strMeter[1].fPhaseA_Volt =  pModbus->esRunData.fGrid_ACVolt_L1;
	pModbus->strMeter[1].fPhaseB_Volt =  pModbus->esRunData.fGrid_ACVolt_L2;
	pModbus->strMeter[1].fPhaseC_Volt =  pModbus->esRunData.fGrid_ACVolt_L3;
	pModbus->strMeter[1].fPhaseA_Curt =  pModbus->esRunData.fGrid_ACCurr_L1;
	pModbus->strMeter[1].fPhaseB_Curt =  pModbus->esRunData.fGrid_ACCurr_L2;
	pModbus->strMeter[1].fPhaseC_Curt =  pModbus->esRunData.fGrid_ACCurr_L3;
 	pModbus->strMeter[1].fActive_Power = pModbus->esRunData.f_PgridMeter;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 404, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fReactive_Power = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 426, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].f_Factor = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 424, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].f_Freq = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 411, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fPos_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 422, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fNeg_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1491, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Pos_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1492, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Pos_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1493, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Pos_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1494, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Pos_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1495, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Pos_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1496, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Neg_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1497, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Neg_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1498, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Neg_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1499, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Neg_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1500, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fCurrent_Neg_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1501, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Pos_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1502, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Pos_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1503, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Pos_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1504, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Pos_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1505, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Pos_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1506, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Neg_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1507, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Neg_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1508, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Neg_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1509, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Neg_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1510, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast1_Neg_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1511, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Pos_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1512, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Pos_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1513, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Pos_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1514, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Pos_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1515, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Pos_Wave_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1516, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Neg_Sum_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1517, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Neg_Sharp_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1518, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Neg_Peak_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1519, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Neg_Flat_Kwh = valGet.val.fValue;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		408, 1520, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strMeter[1].fLast2_Neg_Wave_kwh = valGet.val.fValue;

	//-------------------Refresh Battery Group#1 SigValue -------------------//
	for(n = 0; n < pModbus->nCfgBMSNum; n++)
	{
		ZERO_POBJS(&pModbus->blobSiglBatt, 1);
		pModbus->blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
		valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
		valGet.val.blobValue = (BYTE*)&pModbus->blobSiglBatt;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				501 + n, 551, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			ZERO_POBJS(&pModbus->strBAT[n], 1);
			continue;
		}

		if(pModbus->blobSiglBatt.emBattStatus_Run <= STBATT_RUNSTAT_COMM_FAIL)
		{
			ZERO_POBJS(&pModbus->strBAT[n], 1);
			continue;
		}

		pModbus->strBAT[n].fRack_Volt = pModbus->blobSiglBatt.fPackTotalVolt;
		pModbus->strBAT[n].fRack_Curt = pModbus->blobSiglBatt.fPackTotalCurr;
		
		pModbus->strSys_SigMap.fPbms[n] = pModbus->blobSiglBatt.fPackTotalVolt*pModbus->blobSiglBatt.fPackTotalCurr*0.001;

		if(pModbus->blobSiglBatt.fPackTotalCurr < -1.0)
		{
			pModbus->strBAT[n].fRunState = STBATT_RUNSTAT_DISCHARGE;
		}
		else
		{
			pModbus->strBAT[n].fRunState = (float)pModbus->blobSiglBatt.emBattStatus_Run;
		}
		pModbus->strBAT[n].f_SOC = pModbus->blobSiglBatt.nSOC;
		pModbus->strBAT[n].f_SOH = pModbus->blobSiglBatt.nSOH;
		pModbus->strBAT[n].fMax_AllowChgVolt = pModbus->blobSiglBatt.fAllow_LmtChargeVolt;
		pModbus->strBAT[n].fMax_AllowChgCurt = pModbus->blobSiglBatt.fAllow_MaxChargeCurr;
		pModbus->strBAT[n].fMax_AllowDisChgVolt = pModbus->blobSiglBatt.fAllow_LmtDisChargeVolt;
		pModbus->strBAT[n].fMax_AllowDisChgCurt = pModbus->blobSiglBatt.fAllow_MaxDisChargeCurr;
		pModbus->strBAT[n].fMax_CellTemp = pModbus->blobSiglBatt.fMax_CellTemp;
		pModbus->strBAT[n].fMin_CellTemp = pModbus->blobSiglBatt.fMin_CellTemp;
		pModbus->strBAT[n].fMax_CellVolt = pModbus->blobSiglBatt.fMax_CellVolt;
		pModbus->strBAT[n].fMin_CellVolt = pModbus->blobSiglBatt.fMin_CellVolt;
		pModbus->strBAT[n].fNotStartChgReason =  (float)pModbus->blobSiglBatt.emReason_ChgStart;
		pModbus->strBAT[n].fNotStartDischgReason = (float)pModbus->blobSiglBatt.emReason_DisChgON;

		valGet.nGetSize = 4;
		int i = 0; // coolPackId
		for(i=0; i<5; ++i)
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 981 + n, 1700 + 10*i, &valGet, sizeof(PARAM_VAL_GET));
			pModbus->strBAT[n].fPosCopperTemp[i] = valGet.val.fValue;
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 981 + n, 1701 + 10*i, &valGet, sizeof(PARAM_VAL_GET));
			pModbus->strBAT[n].fNegCopperTemp[i] = valGet.val.fValue;
		}

		//----------------CELL Volt and Temp------------//
		ZERO_POBJS(&pModbus->blobIfyBMS, 1);
		ZERO_POBJS(&pModbus->strYC_CellInfo[n], 1);

		pModbus->blobIfyBMS.emBlobType = BLOB_TYPE_IFYBMS_DATAINFO;
		valGet.nGetSize = sizeof(A_BLOB_IFY_BMS_DATAINFO);
		valGet.val.blobValue = (BYTE*)&pModbus->blobIfyBMS;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		501 + n, 557, &valGet, sizeof(PARAM_VAL_GET));
		if(err != ERRCODE_DAI_OK)
		{
			ZERO_POBJS(&pModbus->strYC_CellInfo[n], 1);
			continue;
		}
		
		for(m = 0; m < A_CABINET_CELL_MAX_NUM; m++)
		{
			pModbus->strYC_CellInfo[n].fcellVolt[m] = (float)pModbus->blobIfyBMS.usCellVolt[m] * 0.001;
			pModbus->strYC_CellInfo[n].fcellTemp[m] = pModbus->blobIfyBMS.cCellTemp[m];
		}

	}

	//------------------- Refresh YaoCe PCS DataValue ---------------------//
	int nDirect = 1;
	pModbus->strPCS.fOutputVolt_Grp1 = pModbus->esRunData.fACDC_OutVolt[0];
	pModbus->strPCS.emWorkMode_Grp1  = (float)pModbus->esRunData.emACDC_WorkStatu[0];
	nDirect = (pModbus->esRunData.emACDC_WorkStatu[0] == STATU_ACDC_INVERTER) ? -1 : 1;
	pModbus->strPCS.fOutputCurt_Grp1 = nDirect*pModbus->esRunData.fACDC_OutCurr[0];
	pModbus->strSys_SigMap.fPpcs[0]	= pModbus->strPCS.fOutputVolt_Grp1*pModbus->strPCS.fOutputCurt_Grp1*0.001;
 
	pModbus->strPCS.fOutputVolt_Grp2 = pModbus->esRunData.fACDC_OutVolt[1];
 	pModbus->strPCS.emWorkMode_Grp2  = (float)pModbus->esRunData.emACDC_WorkStatu[1];
 	nDirect = (pModbus->esRunData.emACDC_WorkStatu[1] == STATU_ACDC_INVERTER) ? -1 : 1;
	pModbus->strPCS.fOutputCurt_Grp2 = nDirect*pModbus->esRunData.fACDC_OutCurr[1];
	pModbus->strSys_SigMap.fPpcs[1]	= pModbus->strPCS.fOutputVolt_Grp2*pModbus->strPCS.fOutputCurt_Grp2*0.001;

	pModbus->strPCS.fOutputVolt_Grp3 = pModbus->esRunData.fACDC_OutVolt[2];
	pModbus->strPCS.emWorkMode_Grp3  = (float)pModbus->esRunData.emACDC_WorkStatu[2];
	nDirect = (pModbus->esRunData.emACDC_WorkStatu[2] == STATU_ACDC_INVERTER) ? -1 : 1;
	pModbus->strPCS.fOutputCurt_Grp3 = nDirect*pModbus->esRunData.fACDC_OutCurr[2];
 	pModbus->strSys_SigMap.fPpcs[2]	= pModbus->strPCS.fOutputVolt_Grp3*pModbus->strPCS.fOutputCurt_Grp3*0.001;

	pModbus->strPCS.fOutputVolt_Grp4 = pModbus->esRunData.fACDC_OutVolt[3];
	pModbus->strPCS.emWorkMode_Grp4  = (float)pModbus->esRunData.emACDC_WorkStatu[3];
	nDirect = (pModbus->esRunData.emACDC_WorkStatu[3] == STATU_ACDC_INVERTER) ? -1 : 1;
	pModbus->strPCS.fOutputCurt_Grp4 = nDirect*pModbus->esRunData.fACDC_OutCurr[3];
 	pModbus->strSys_SigMap.fPpcs[3]	= pModbus->strPCS.fOutputVolt_Grp4*pModbus->strPCS.fOutputCurt_Grp4*0.001;

	//------------------- Refresh YaoCe STS DataValue ---------------------//
	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 160, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSTS.usSTS_OnOffStatus = valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 161, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSTS.usSTS_OnOffGrid = valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 162, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSTS.usSTS_BusOnOffGrid = valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 163, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSTS.usSTS_FaultStatus = valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 164, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strSTS.usSTS_GridAlarmStatus = valGet.val.emValue;

	//------------------------------END---------------------------------//

	pModbus->strLocal_Ems.nEmsSectorNum = MIN(MAX_EMS_SECTOR, pModbus->emsPlan.nTotal_EmsLmt_Sectors);
	for(n = 0; n < pModbus->strLocal_Ems.nEmsSectorNum; n++)
	{
		pModbus->strLocal_Ems.arryEmsParam[n].tmStart = nInterval_Min; 
		
		nInterval_Min = nInterval_Min + (pModbus->emsPlan.emsLmtSectors[n].nInterval/60);

		//pModbus->strLocal_Ems.arryEmsParam[n].nInterval = pModbus->emsPlan.emsLmtSectors[n].nInterval; 
		if(pModbus->emsPlan.emsLmtSectors[n].emBatt_WorkMode == BATT_WORKMODE_FIRST_DISCHRG)
		{
			nRate = -1;
		}
		//pModbus->strLocal_Ems.arryEmsParam[n].nBattGrpPwrLmt = pModbus->emsPlan.emsLmtSectors[n].emBatt_WorkMode; 
		pModbus->strLocal_Ems.arryEmsParam[n].nStopSOC = pModbus->emsPlan.emsLmtSectors[n].nStopSOC; 
		pModbus->strLocal_Ems.arryEmsParam[n].nBattGrpPwrLmt = pModbus->emsPlan.emsLmtSectors[n].nBattGrpPwrLmt; 
		pModbus->strLocal_Ems.arryEmsParam[n].nActvPwrLmt =  pModbus->emsPlan.emsLmtSectors[n].nActvPwrLmt*nRate; 
		pModbus->strLocal_Ems.arryEmsParam[n].nReActvPwrLmt = pModbus->emsPlan.emsLmtSectors[n].nReActvPwrLmt; 

		TRACEX("Idx=[%d] nInterval_Min=[%d] tmStart=[%d] nStopSOC=[%d] nBattGrpPwrLmt=[%d] nActvPwrLmt=[%d] nReActvPwrLmt=[%d]\n", n, nInterval_Min,\
			pModbus->strLocal_Ems.arryEmsParam[n].tmStart, \
			pModbus->strLocal_Ems.arryEmsParam[n].nStopSOC,\
			pModbus->strLocal_Ems.arryEmsParam[n].nBattGrpPwrLmt,\
			pModbus->strLocal_Ems.arryEmsParam[n].nActvPwrLmt,\
			pModbus->strLocal_Ems.arryEmsParam[n].nReActvPwrLmt);
	}

	//----------------KWH Statistic Data------------//
	Refresh_KWhStatistic_SigMap(pReporter);

	//-------------------Get Thermal And Fire DataInfo-----------------//
	for(n = 0; n < pModbus->nCfgBMSNum; n++)
	{
		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,   //电池柜前门被打开
			601 + n, 454, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emDoorOpen = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,   //火探管告警
			601 + n, 452, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emFireTubeAlarm = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //烟感告警
			601 + n, 453, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emSmokeAlarm = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //可燃气体告警
			601 + n, 455, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emGasAlarm = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //电池柜-水浸告警
			601 + n, 461, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emFloodAlarm = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //空调硬件告警
			601 + n, 458, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emAirCondHWAlarm = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //空调当前状态
			601 + n, 451, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emAirMachineState = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //整机状态(TF)
			601 + n, 472, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emWholeMachineStatus = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //内风机状态(TF)
			601 + n, 473, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emInternalFanStatus = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //外风机状态(TF)
			601 + n, 474, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emOutsideFanStatus = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //压缩机状态(TF)
			601 + n, 475, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emCompressorStatus = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //电加热状态(TF)
			601 + n, 476, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emElectricHeatStatus = valGet.val.emValue;

		valGet.nGetSize = 4;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,  //强制制冷模式(TF)
			601 + n, 477, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strThermalFire[n].emForceAutoMode = valGet.val.emValue;

		ZERO_POBJS(&pModbus->blobImeu2, 1);
		pModbus->blobImeu2.emBlobType = BLOB_TYPE_BATTCAB_IMEU2_V20;
		valGet.nGetSize = sizeof(A_BLOB_BATTCAB_IMEU2_V20);
		valGet.val.blobValue = (BYTE*)&pModbus->blobImeu2;
		err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
				601 + n, 450, &valGet, sizeof(PARAM_VAL_GET));

		pModbus->strThermalFire[n].fCabTemp = pModbus->blobImeu2.fRealTimeCabTemp;		//空调温度
		pModbus->strThermalFire[n].fCabHumi = pModbus->blobImeu2.fRealTimeCabHumi;  	//空调湿度
		pModbus->strThermalFire[n].fSensor1Humi = pModbus->blobImeu2.fHumidity[0];		//传感器#1 湿度
		pModbus->strThermalFire[n].fSensor1Temp = pModbus->blobImeu2.fTemperature[0];	//传感器#1 温度
		pModbus->strThermalFire[n].fSensor2Humi = pModbus->blobImeu2.fHumidity[1];		//传感器#2 湿度
		pModbus->strThermalFire[n].fSensor2Temp = pModbus->blobImeu2.fTemperature[1];	//传感器#2 温度
	}
	//------------------------------END---------------------------------//
	return;
}


//------------------------- KWH ----------------------------------//
void Refresh_KWhStatistic_SigMap(REPORTER *pReporter)
{
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET valGet;

	static UINT nLast_Day = 0;

	time_t tmNow = _NOW_;
    struct tm gmTime;
    gmtime_r(&tmNow, &gmTime);
    UINT nCurr_Day = gmTime.tm_mday - 1;
 	if((gmTime.tm_hour == 0) && (gmTime.tm_min < 10)) //Before 00:10 Send Last Day Value
	{
		nCurr_Day = nLast_Day;
	}

	if(nCurr_Day != nLast_Day)
	{
		nLast_Day = nCurr_Day;
	}

	if(nCurr_Day > MAX_DAYS_SECTOR)
	{
		TRACE("nCurr_Day=[%d] is Outof Range,GoBack!\n", nCurr_Day);
		return ;
	}

 	ZERO_POBJS(&pModbus->blobStatData ,	1);
	pModbus->blobStatData.emBlobType = BLOB_TYPE_ES_STATISTICDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_STATISTIC_DATA);
	valGet.val.blobValue = (BYTE*)&(pModbus->blobStatData);
	ERRCODE_DAI err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 79, &valGet, sizeof(PARAM_VAL_GET));
	if(err != ERRCODE_DAI_OK)
	{
		TRACE("Error: DAI Get Q204 Failed err=[%d]!\n", err);
		return ;
	}

	if((pModbus->blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day] == 0))
	{
		TRACE("nCurr_Day=[%d] Data Is Not Saved Yet,Go Back!\n");
		return ;
	}

	pModbus->strKWh.fDate = 
		pModbus->blobStatData.es_Data.days_Data.tmTimeStamp_D[nCurr_Day];

	pModbus->strKWh.fCab_PosDailyKwh	=
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_NegDailyKwh	=
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_PosDailySharpKwh =
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_PosDailyPeakKwh	=
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_PosDailyFlatKwh	=
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_PosDailyValleyKwh = 
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_NegDailySharpKwh =
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_NegDailyPeakKwh	=
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_NegDailyFlatKwh	=
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[nCurr_Day];

	pModbus->strKWh.fCab_NegDailyValleyKwh =
		pModbus->blobStatData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[nCurr_Day];

	pModbus->strKWh.fBAT1_DailyChgKwh = 
		pModbus->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[0][nCurr_Day];
		
	pModbus->strKWh.fBAT1_DailyDischgKwh	= 
		pModbus->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[0][nCurr_Day];

	pModbus->strKWh.fBAT2_DailyChgKwh = 
		pModbus->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[1][nCurr_Day];

	pModbus->strKWh.fBAT2_DailyDischgKwh	=
		pModbus->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[1][nCurr_Day];

	pModbus->strKWh.fBAT3_DailyChgKwh =
		pModbus->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[2][nCurr_Day];

	pModbus->strKWh.fBAT3_DailyDischgKwh	=
		pModbus->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[2][nCurr_Day];

	pModbus->strKWh.fBAT4_DailyChgKwh =
		pModbus->blobStatData.es_Data.days_Data.fBattTotalChrgKWH_D[3][nCurr_Day];

	pModbus->strKWh.fBAT4_DailyDischgKwh	=
		pModbus->blobStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[3][nCurr_Day];

	return;
}

//-----------------GangHuaShuNeng Add by XSY---------------------//
DWORD InitModbusDataMap_GHSN(REPORTER *pReporter)
{
	MODBUS_DATA_STU* pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	PARAM_VAL_GET			valGet;

	//1. Init Parameter
	pModbus->bAcceptConnOK = FALSE;
	pModbus->hMDDataComm = NULL;
	pModbus->nReadFailtTimes = 0;
 	pModbus->tmLastDataRefresh = _NOW_;

	// Init MapInfo
	pModbus->ModbusModelCfg.iTypeMapNum = MODBUS_CMD_NUM;
	pModbus->ModbusModelCfg.pTypeMapInfo[0].iFunctionCode = MODBUS_GETDATA_CMD;
	pModbus->ModbusModelCfg.pTypeMapInfo[0].iLengthFix = 0;
	pModbus->ModbusModelCfg.pTypeMapInfo[0].iLength = 8;
	pModbus->ModbusModelCfg.pTypeMapInfo[1].iFunctionCode = MODBUS_SETSINGLEDATA_CMD;
	pModbus->ModbusModelCfg.pTypeMapInfo[1].iLengthFix = 0;
	pModbus->ModbusModelCfg.pTypeMapInfo[1].iLength = 8;
	pModbus->ModbusModelCfg.pTypeMapInfo[2].iFunctionCode = MODBUS_SETMULTIDATA_CMD;
	pModbus->ModbusModelCfg.pTypeMapInfo[2].iLengthFix = 1;
	pModbus->ModbusModelCfg.pTypeMapInfo[2].iLength = 7;
	
	// Flag0: basic Value	Flag1: Coeff	Output Value = Value *Coeff +Basic Value 
	MD_DATA_ITEM mdDataItems[] =
	{	
		{MD_SIG_Src_Cod,			FALSE, 1001,DAITYPE_CTRL_PARAMVALUE_RW,3902,MD_GET_VAL_FLOAT,	1, 2, 0x00 ,0x01, 0, 1,	(void*)&(pModbus->strGHSN.fSrcCod)},
		{MD_SIG_TotP_Cod,			FALSE, 1001,DAITYPE_CTRL_PARAMVALUE_RW,3901,MD_GET_VAL_FLOAT,	1, 2, 0x02 ,0x03, 0, 1,	(void*)&(pModbus->strGHSN.fTotPCod)},
		{MD_SIG_TotQ_Cod,			FALSE, 0,	DAITYPE_CTRL_PARAMVALUE_RW,11,	MD_GET_VAL_FLOAT,	1, 2, 0x04 ,0x05, 0, 1,	(void*)&(pModbus->strGHSN.fTotQCod)},
		{MD_SIG_Run_Cod,			FALSE, 701,	DAITYPE_CTRL_PARAMVALUE_RW,1110,MD_GET_VAL_FLOAT,	1, 2, 0x06 ,0x07, 0, 1,	(void*)&(pModbus->strGHSN.fRunCod)},
		{MD_SIG_OffGrd_Cod,			FALSE, 200,	DAITYPE_CTRL_PARAMVALUE_RW,219,	MD_GET_VAL_FLOAT,	1, 2, 0x08 ,0x09, 0, 1,	(void*)&(pModbus->strGHSN.fOffGrdCod)},
		{MD_SIG_NowP,				FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0A ,0x0B, 0, 1,	(void*)&(pModbus->strGHSN.fNowP)},
		{MD_SIG_NowQ,				FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0C ,0x0D, 0, 1,	(void*)&(pModbus->strGHSN.fNowQ)},
		{MD_SIG_SOC,				FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x0E ,0x0F, 0, 1,	(void*)&(pModbus->strGHSN.fSoc)},
		{MD_SIG_SOH,				FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x10 ,0x01, 0, 1,	(void*)&(pModbus->strGHSN.fSoh)},
		{MD_SIG_Allow_ChgPow,		FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x12 ,0x13, 0, 1,	(void*)&(pModbus->strGHSN.fAllowChgPow)},
		{MD_SIG_Allow_DischgPow,	FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x14 ,0x15, 0, 1,	(void*)&(pModbus->strGHSN.fAllowDischgPow)},
		{MD_SIG_Sta,				FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x16 ,0x17, 0, 1,	(void*)&(pModbus->strGHSN.fSta)},
		{MD_SIG_Src_Cod_Feedback,	FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x18 ,0x19, 0, 1,	(void*)&(pModbus->strGHSN.fSrcCodFeedback)},
		{MD_SIG_TotP_Feedback,		FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x1A ,0x1B, 0, 1,	(void*)&(pModbus->strGHSN.fTotPCodFeedback)},
		{MD_SIG_TotQ_Feedback,		FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x1C ,0x1D, 0, 1,	(void*)&(pModbus->strGHSN.fTotQCodFeedback)},
		{MD_SIG_Run_Feedback,		FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x1E ,0x1F, 0, 1,	(void*)&(pModbus->strGHSN.fRunCodFeedback)},
		{MD_SIG_OffGrd_Feedback,	FALSE, 0,	DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x20 ,0x21, 0, 1,	(void*)&(pModbus->strGHSN.fOffGrdCodFeedback)},
		
		{MD_SIG_Forward_ActPow_Meter1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x22 ,0x23, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fForwardActPow)},
		{MD_SIG_Backward_ActPow_Meter1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x24 ,0x25, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fBackwardActPow)},
		{MD_SIG_Meter_Pow_Meter1,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x26 ,0x27, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fMeterPow)},
		{MD_SIG_Forward_SharpActPow_Meter1,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x28 ,0x29, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fForwardSharpActPow)},
		{MD_SIG_Forward_SPeakActPow_Meter1,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x2A ,0x2B, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fForwardPeakActPow)},
		{MD_SIG_Forward_SFlatActPow_Meter1,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x2C ,0x2D, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fForwardFlatActPow)},
		{MD_SIG_Forward_SValleyActPow_Meter1,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x2E ,0x2F, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fForwardValleyActPow)},
		{MD_SIG_Reverse_SharpActPow_Meter1,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x30 ,0x31, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fReverseSharpActPow)},
		{MD_SIG_Reverse_SPeakActPow_Meter1,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x32 ,0x33, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fReversePeakActPow)},
		{MD_SIG_Reverse_SFlatActPow_Meter1,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x34 ,0x35, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fReverseFlatActPow)},
		{MD_SIG_Reverse_SValleyActPow_Meter1,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x36 ,0x37, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fReverseValleyActPow)},
		{MD_SIG_Real_Demand_Meter1,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x38 ,0x39, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fRealDemand)},
		{MD_SIG_Max_Demand_Meter1,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x3A ,0x3B, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fMaxDemand)},
		{MD_SIG_Meter_CommSta_Meter1,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x3C ,0x3D, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[0].fMeterCommSta)},
		
		{MD_SIG_Forward_ActPow_Meter2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x3E ,0x3F, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fForwardActPow)},
		{MD_SIG_Backward_ActPow_Meter2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x40 ,0x41, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fBackwardActPow)},
		{MD_SIG_Meter_Pow_Meter2,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x42 ,0x43, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fMeterPow)},
		{MD_SIG_Forward_SharpActPow_Meter2,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x44 ,0x45, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fForwardSharpActPow)},
		{MD_SIG_Forward_SPeakActPow_Meter2,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x46 ,0x47, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fForwardPeakActPow)},
		{MD_SIG_Forward_SFlatActPow_Meter2,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x48 ,0x49, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fForwardFlatActPow)},
		{MD_SIG_Forward_SValleyActPow_Meter2,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4A ,0x4B, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fForwardValleyActPow)},
		{MD_SIG_Reverse_SharpActPow_Meter2,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4C ,0x4D, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fReverseSharpActPow)},
		{MD_SIG_Reverse_SPeakActPow_Meter2,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x4E ,0x4F, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fReversePeakActPow)},
		{MD_SIG_Reverse_SFlatActPow_Meter2,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x50 ,0x51, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fReverseFlatActPow)},
		{MD_SIG_Reverse_SValleyActPow_Meter2,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x52 ,0x53, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fReverseValleyActPow)},
		{MD_SIG_Real_Demand_Meter2,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x54 ,0x55, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fRealDemand)},
		{MD_SIG_Max_Demand_Meter2,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x56 ,0x57, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fMaxDemand)},
		{MD_SIG_Meter_CommSta_Meter2,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x58 ,0x59, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[1].fMeterCommSta)},

		{MD_SIG_Forward_ActPow_Meter3,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x5A ,0x5B, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fForwardActPow)},
		{MD_SIG_Backward_ActPow_Meter3,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x5C ,0x5D, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fBackwardActPow)},
		{MD_SIG_Meter_Pow_Meter3,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x5E ,0x5F, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fMeterPow)},
		{MD_SIG_Forward_SharpActPow_Meter3,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x60 ,0x61, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fForwardSharpActPow)},
		{MD_SIG_Forward_SPeakActPow_Meter3,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x62 ,0x63, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fForwardPeakActPow)},
		{MD_SIG_Forward_SFlatActPow_Meter3,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x64 ,0x65, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fForwardFlatActPow)},
		{MD_SIG_Forward_SValleyActPow_Meter3,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x66 ,0x67, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fForwardValleyActPow)},
		{MD_SIG_Reverse_SharpActPow_Meter3,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x68 ,0x69, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fReverseSharpActPow)},
		{MD_SIG_Reverse_SPeakActPow_Meter3,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6A ,0x6B, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fReversePeakActPow)},
		{MD_SIG_Reverse_SFlatActPow_Meter3,		FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6C ,0x6D, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fReverseFlatActPow)},
		{MD_SIG_Reverse_SValleyActPow_Meter3,	FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x6E ,0x6F, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fReverseValleyActPow)},
		{MD_SIG_Real_Demand_Meter3,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x70 ,0x71, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fRealDemand)},
		{MD_SIG_Max_Demand_Meter3,				FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x72 ,0x73, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fMaxDemand)},
		{MD_SIG_Meter_CommSta_Meter3,			FALSE, 0,DAITYPE_STAT_PARAMVALUE_R,0,	MD_GET_VAL_FLOAT,	1, 2, 0x74 ,0x75, 0, 1,	(void*)&(pModbus->strGHSN.strMeter[2].fMeterCommSta)},
	
	};

	pModbus->pmdDataItems = NEW(MD_DATA_ITEM,ITEM_OF(mdDataItems));
	if(pModbus->pmdDataItems == NULL)
	{
		return ERR_CFG_NO_MEMORY;
	}

	memcpy(pModbus->pmdDataItems , mdDataItems, sizeof(mdDataItems));
	pModbus->nDataItemNum = ITEM_OF(mdDataItems);

	return ERR_CFG_OK;
}

void Refresh_SigMapData_GHSN(REPORTER *pReporter)
{
	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	
	int 				n = 0;
	PARAM_VAL_GET		valGet;
	VAR_VALUE			varV;
	ERRCODE_DAI			err;

	// ESRUNDATA
	ZERO_POBJS(&pModbus->esRunData, 1);
	pModbus->esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&pModbus->esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		200, 80, &valGet, sizeof(PARAM_VAL_GET));

	// BATT
	ZERO_POBJS(&pModbus->blobSiglBatt, 1);
	pModbus->blobSiglBatt.emBlobType = BLOB_TYPE_SINGLE_STBATT_RUNINFO;
	valGet.nGetSize = sizeof(A_BLOB_SINGLE_STBATT_RUNINFO);
	valGet.val.blobValue = (BYTE*)&pModbus->blobSiglBatt;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501, 551, &valGet, sizeof(PARAM_VAL_GET));

	// METER
	ZERO_POBJS(&pModbus->esRunData, 1);
	pModbus->esRunData.emBlobType = BLOB_TYPE_ES_RUNDATA;
	valGet.nGetSize = sizeof(A_BLOB_ES_RUN_DATA);
	valGet.val.blobValue = (BYTE*)&pModbus->esRunData;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 200, 80, &valGet, sizeof(PARAM_VAL_GET));

	// EMS从机汇总
	ZERO_POBJS(&pModbus->clientInfoSum, 1);
	pModbus->clientInfoSum.emBlobType = BLOB_TYPE_SEMS_CLIENTNFO_SUM;
	valGet.nGetSize = sizeof(A_BLOB_SEMS_CLIENTINFO_SUM);
	valGet.val.blobValue = (BYTE*)&pModbus->clientInfoSum;
	err = pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R,
		1001, 3900, &valGet, sizeof(PARAM_VAL_GET));

// Start
	pModbus->strGHSN.fNowP				= (-1)*pModbus->esRunData.fActvPwr_PCSGrp[0];
	pModbus->strGHSN.fNowQ				= pModbus->esRunData.fQ_PCSGrp[0];
	pModbus->strGHSN.fSoc				= (float)pModbus->blobSiglBatt.nSOC;
	pModbus->strGHSN.fSoh				= (float)pModbus->blobSiglBatt.nSOH;
	pModbus->strGHSN.fAllowChgPow		= pModbus->blobSiglBatt.fAllow_MaxChargePow;
	pModbus->strGHSN.fAllowDischgPow	= pModbus->blobSiglBatt.fAllow_MaxDisChargePow;

	valGet.nGetSize = 4;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 501, 550, &valGet, sizeof(PARAM_VAL_GET));
	switch (valGet.val.emValue)
	{
	case STBATT_RUNSTAT_CHARGE:
		pModbus->strGHSN.fSta			= 2.0;
		break;
	case STBATT_RUNSTAT_DISCHARGE:
		pModbus->strGHSN.fSta			= 3.0;
		break;
	case STBATT_RUNSTAT_SLEEP:
	case STBATT_RUNSTAT_IDLE:
	case STBATT_RUNSTAT_FULL_CHARGE:
	case STBATT_RUNSTAT_FULL_DISCHARGE:
		pModbus->strGHSN.fSta			= 1.0;
		break;
	case STBATT_RUNSTAT_NOT_INSTALLED:
		pModbus->strGHSN.fSta			= 0.0;
		break;
	case STBATT_RUNSTAT_COMM_FAIL:
		pModbus->strGHSN.fSta			= 4.0;
		break;
	
	default:
		break;
	}
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, 1, 4, &valGet, sizeof(PARAM_VAL_GET));
	if(valGet.val.emValue == CCU_STATE_FAULTED)
	{
		pModbus->strGHSN.fSta			= 4.0;
	}
	
	pModbus->strGHSN.fSrcCodFeedback	= pModbus->strGHSN.fSrcCod;
	pModbus->strGHSN.fTotPCodFeedback	= pModbus->strGHSN.fTotPCod;
	pModbus->strGHSN.fTotQCodFeedback	= 0;//pModbus->strGHSN.fTotQCod;//
	// pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 701, 1110, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strGHSN.fRunCodFeedback	= pModbus->strGHSN.fRunCod;//(float)valGet.val.emValue;
	pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, 200, 219, &valGet, sizeof(PARAM_VAL_GET));
	pModbus->strGHSN.fOffGrdCodFeedback	= (float)valGet.val.emValue;


	//----------------Refresh Meter Data------------//
	// Meter1,2,3
	valGet.nGetSize = 4;
	int i, nDevId;
	for(i=0; i<3; i++)
	{
		if(i==0) nDevId = 407;
		else if(i==1) nDevId = 411;
		else if(i==2) nDevId = 408;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1491, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fForwardActPow			= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1496, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fBackwardActPow		= valGet.val.fValue;
		switch(i)
		{
			case 0:// 计量表
				pModbus->strGHSN.strMeter[i].fMeterPow				= (-1)*pModbus->esRunData.f_PcabMeter;
				break;
			case 1:// 防过载表
				pModbus->strGHSN.strMeter[i].fMeterPow				= (-1)*pModbus->esRunData.f_PauxMeter_3;
				break;
			case 2:// 防逆流表
				pModbus->strGHSN.strMeter[i].fMeterPow				= (-1)*pModbus->esRunData.f_PgridMeter;
				break;
		}
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1492, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fForwardSharpActPow	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1493, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fForwardPeakActPow		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1494, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fForwardFlatActPow		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1495, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fForwardValleyActPow	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1497, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fReverseSharpActPow	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1498, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fReversePeakActPow		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1499, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fReverseFlatActPow		= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 1500, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fReverseValleyActPow	= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 432, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fRealDemand			= valGet.val.fValue;
		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_STAT_PARAMVALUE_R, nDevId, 425, &valGet, sizeof(PARAM_VAL_GET));
		pModbus->strGHSN.strMeter[i].fMaxDemand				= valGet.val.fValue;

		// DEVICE* pDev = GetDevce_ById(nDevId);
		// if(pDev == NULL)
		// {
		// 	printf("can't get meter pDev %d\n", nDevId);
		// 	pModbus->strGHSN.strMeter[i].fMeterCommSta	= 1;
		// }
		// pModbus->strGHSN.strMeter[i].fMeterCommSta	= (pDev->emStatus == DEVICE_STATUS_NORMAL) ? 0 : 1;

		pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_SETT_PARAMVALUE_RW, nDevId, 401, &valGet, sizeof(PARAM_VAL_GET));
		if(valGet.val.emValue == 0)//0 :METER_TYPE_NotInstalled
		{
			pModbus->strGHSN.strMeter[i].fMeterCommSta	= 1.0;	// 未安装，即异常
		}
		else
		{
			pReporter->pDAI->Get((HANDLE)pReporter, DAITYPE_ALARMSTAT_R, nDevId, 401, &varV, 4);
			pModbus->strGHSN.strMeter[i].fMeterCommSta	= (float)varV.emValue;
		}

	}

	return;
}


void Stuff_Modbus_ArrayData(REPORTER *pReporter)
{
	TRACE("Now 111 Refresh_Modbus_Data\n");
	ERRCODE_DAI 			err;
	VAR_VALUE 				val;

	WORD					*pCpySrc;
	MD_DATA_ITEM			*pCurMdDataItem;
	int						i, n, nPosition, iRegAddr;
	A_BLOB_METER_VALUES  	*pMerterVals = NULL;
	UNION_COMPLEXTYPE 		unionType;

	MODBUS_DATA_STU *pModbus = (MODBUS_DATA_STU*)pReporter->pDataHeap;
	pCurMdDataItem = pModbus->pmdDataItems;
	memset(pModbus->szRespData2, 0xFF/*MODBUS_INVALIDDATA*/, sizeof(pModbus->szRespData2));
		
	for(n = 0; n < pModbus->nDataItemNum; n++, pCurMdDataItem++)
	{
		if(pCurMdDataItem == NULL)
		{
			TRACE("n=[%d] pCurMdDataItem=NULL\n", n);
			break;
		}

		iRegAddr = pCurMdDataItem->nRegAdrStart;
 
		// if((iRegAddr < pCurMdDataItem->nRegAdrStart) || (iRegAddr > pCurMdDataItem->nRegAdrEnd))
		// {
		// 	TRACE("Single mdDataItems=[%d] nPosition=[%d] iRegAddr=%d\n", n, nPosition, iRegAddr);
		// 	continue;
		// }

		//printf("333333333333 Idx=[%d] iRegAddr=[%d]\n", n, iRegAddr);

		pCpySrc =&(pModbus->szRespData2[iRegAddr]);

		switch(pCurMdDataItem->emDataType)
		{
		case MD_GET_VAL_SHORT :
			{
				
				val.lValue =  *((long*)pCurMdDataItem->ptrValue);
				
				//printf("4444444444 val.lValue=[%ld]\n", val.lValue);

				if(val.ulValue == MODBUS_INVALIDDATA)
				{
					*pCpySrc = 	MODBUS_INVALIDDATA;
				}
				else
				{
					*pCpySrc = (WORD)((int)(val.lValue *pCurMdDataItem->nFlag1 + pCurMdDataItem->nFlag0)%MODBUS_DIVISOR);
					//printf("->emSigType =%d fValue =%f *pCpySrc =%d\n", pCurMdDataItem->emSigType, val.fValue, *pCpySrc );
				}	
			}
			break;
		case MD_GET_VAL_USHORT:
			{

				val.ulValue =  *((unsigned long*)pCurMdDataItem->ptrValue);
				
				//printf("555555555555 val.lValue=[%ld]\n", val.lValue);

				if(val.ulValue == MODBUS_INVALIDDATA)
				{
					*pCpySrc = 	MODBUS_INVALIDDATA;
				}
				else
				{
					*pCpySrc = (WORD)((int)(val.lValue *pCurMdDataItem->nFlag1 + pCurMdDataItem->nFlag0)%MODBUS_DIVISOR);
					TRACE("->emSigType =%d fValue =%f *pCpySrc =%d\n", pCurMdDataItem->emSigType, val.fValue, *pCpySrc );
				}	
			}
			break;

		case MD_GET_VAL_FLOAT:
			{

				unionType.fValue =  *((float*)pCurMdDataItem->ptrValue);
				unionType.fValue = (unionType.fValue *pCurMdDataItem->nFlag1 + pCurMdDataItem->nFlag0);

				//printf("6666666666666b unionType.fValue=[%.1f]\n", unionType.fValue);

				if(unionType.fValue == MODBUS_INVALIDDATA)
				{
					*pCpySrc = 	MODBUS_INVALIDDATA;
					pCpySrc++;
					*pCpySrc = 	MODBUS_INVALIDDATA;

				}
				else
				{
					//*pCpySrc = (WORD)((int)(val.fValue *pCurMdDataItem->nFlag1 + pCurMdDataItem->nFlag0 + 0.5)%MODBUS_DIVISOR);
					//TRACE("->emSigType =%d fValue =%f *pCpySrc =%d\n", pCurMdDataItem->emSigType,valGet.val.fValue, *pCpySrc );
					//FLOAT Type Order: AB CD
					*pCpySrc = unionType.intValue[1];
					pCpySrc++;
					*pCpySrc = unionType.intValue[0];
    				//printf("Value Order:unionType.fValue=[%.1f] %02X|%02X|%02X|%02X|\n", \
					//	unionType.fValue, unionType.byValue[0], unionType.byValue[1], unionType.byValue[2], unionType.byValue[3]);
				}	
			}
			break;
		case MD_GET_VAL_UL:
			{
				//printf("7777777777777777\n");

				unionType.ulValue =  *((unsigned long*)pCurMdDataItem->ptrValue);
				
 				unionType.ulValue = (unionType.ulValue *pCurMdDataItem->nFlag1 + pCurMdDataItem->nFlag0);
				
				if(unionType.ulValue == MODBUS_INVALIDDATA)
				{
					*pCpySrc = 	MODBUS_INVALIDDATA;
					pCpySrc++;
					*pCpySrc = 	MODBUS_INVALIDDATA;
				}
				else
				{	
					*pCpySrc = unionType.intValue[1];
					pCpySrc++;
					*pCpySrc = unionType.intValue[0];
    				//printf("Value Order:unionType,ulValue=[%d] %02X|%02X|%02X|%02X|\n", \
					//	unionType.ulValue, unionType.byValue[0], unionType.byValue[1], unionType.byValue[2], unionType.byValue[3]);
				}
			}
			break;
		case MD_GET_VAL_LI:
			{
				pCpySrc = (DWORD*)&val.lValue;
			}
			break;
		case MD_GET_VAL_EM_VAL:
			{
				if(val.emValue == MODBUS_INVALIDDATA)
				{
					*pCpySrc = MODBUS_INVALIDDATA;
				}
				else
				{			
					*pCpySrc = (WORD)((int)(val.emValue *pCurMdDataItem->nFlag1 + pCurMdDataItem->nFlag0)%MODBUS_DIVISOR);
					TRACE("->emSigType =%d ulValue =%d *pCpySrc =%d\n", pCurMdDataItem->emSigType, val.ulValue, *pCpySrc );
				}
			}
			break;
		case MD_GET_VAL_EM_STR:
			{
				BYTE *pByte =  ((BYTE*)pCurMdDataItem->ptrValue);
				
				for(i=0; i<pCurMdDataItem->nDataSize; ++i)
				{
					if(pByte[0] == 0xFF && pByte[1] == 0xFF)
					{
						*pCpySrc = MODBUS_INVALIDDATA;
					}
					else
					{	
						*pCpySrc = pByte[0] << 8 | pByte[1];
					}

					// printf("%04X|", *pCpySrc);
	
					pCpySrc++;
					pByte += 2;
				}
					// printf("\n");
			}

			break;
		case MD_GET_VAL_SPECIAL_1:
			{
				//for(i=0; i < (sizeof(szSerialNumber)/2); i++)
				//{
				//	*pCpySrc++ = (szSerialNumber[i*2+1] & 0xFF) + (szSerialNumber[i*2] << 8);
 				//}

				//memcpy(pCpySrc, szSerialNumber, sizeof(szSerialNumber));
				//TRACE("pCpySrc =(DWORD*)valGet.val.blobValue sizeof(szSerialNumber)=%d !!!\n",sizeof(szSerialNumber));
			}	
			break;
		default:
			break;
		}
		//iRegAddr = iRegAddr + pCurMdDataItem->nDataSize;
	}
	
	//register data is stored in szRespData1, from 1 to N, the first data is in [0], not [1]
	memcpy(pModbus->szRespData1, pModbus->szRespData2, sizeof(pModbus->szRespData2));
	return;
}
