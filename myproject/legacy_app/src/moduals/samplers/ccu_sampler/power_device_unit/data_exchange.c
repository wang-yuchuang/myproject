// #define _TRACE_PRINT_ 
#include "../ccu_sampler_main.h"

BOOL Data_Swap1(ROUGH_CCU_DATA* pCCU)
{
    int iGrp = 0;   /* 组循环计数器 */
    int iMod = 0;   /* 模块循环计数器 */

    if (!pCCU) return FALSE; /* 检查指针有效性 */

    TRACEI("Data_Swap1: 开始数据映射\n");

    ACDC_DATA_INFO* pAcData = &pCCU->acdc_data; /* 获取指向ACDC数据实例的指针 */

    A_BLOB_RECT_GRP_DATA* pGrpRectData = &pCCU->strLSYSInfo.blobGrpRectData; /* 指向目标Blob组结构 */
    A_BLOB_SINGLE_RECT_INFO* pSigRectData = &pCCU->strLSYSInfo.singleRectData; /* 指向目标Blob单模块结构 */
	

    /* 遍历所有组 (ACDC Groups 1 & 2) */
    for (iGrp = 0; iGrp < N2402_MAX_GRP_NUM; iGrp++)
    {
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)iGrp; /* 计算当前组对应的类型 */
        PCSACDCGrpCtrlData_t* pGrpData = &pAcData->gPCSACDCCtrlGrp[iGrp]; /* 获取源组数据指针 */

        pGrpRectData->usGrpID[emRectType] = iGrp + 1; /* 映射组ID */
        pGrpRectData->nOnlineRectNum[emRectType] = (int)pGrpData->u32OnlineModNum; /* 映射在线模块数量 */
        TRACEI("Data_Swap1: 组%d 额定功率=%.1fkW 额定电流=%.1fA 控制电压=%.1fV 控制电流=%.1fA 在线=%d\n", iGrp+1,
            pGrpRectData->fRatedPower[emRectType], pGrpRectData->fRatedCurr[emRectType],
            pGrpRectData->fCtrlVolt[emRectType], pGrpRectData->fCtrlCurr[emRectType],
            (int)pGrpData->u32OnlineModNum);

        pGrpRectData->fRatedPower[emRectType] = (float)pGrpData->u32RatedPower * 0.000001f; /* 映射额定功率 */
        pGrpRectData->fRatedCurr[emRectType] = (float)pGrpData->u32RatedCurr * 0.001f; /* 映射额定电流 */
        pGrpRectData->fRealTime_MaxCurr[emRectType] = (float)pGrpData->u32MaxCurr * 0.001f; /* 映射最大电流 */

        // pGrpRectData->fCtrlVolt[emRectType] = (float)pGrpData->u32CtrlDCVolt * 0.001f; /* 映射控制直流电压 */
        // pGrpRectData->fCtrlCurr[emRectType] = (float)pGrpData->u32CtrlDCCurr * 0.001f; /* 映射控制直流电流 */
        // pGrpRectData->fCtrlPower[emRectType] = (float)pGrpData->u32CtrlPower * 0.001f; /* 映射控制功率 */

        pGrpRectData->fRealTime_BattSideVolt[emRectType] = (float)pGrpData->u32CurrDCVolt * 0.001f; /* 映射当前直流侧电压 */
        pGrpRectData->fRealTime_BattSideTotalCurr[emRectType] = (float)pGrpData->u32CurrDCCurr * 0.001f; /* 映射当前直流侧总电流 */
        pGrpRectData->fRealTimeSystemPower[emRectType] = (float)pGrpData->u32CurrSysPower * 0.000001f; /* 映射当前系统功率 */

        // pGrpRectData->emCtrlStatus[emRectType] = (CTRL_STATUS)pGrpData->u32CtrlState; /* 映射控制状态 */
        pGrpRectData->emRunningStatus[emRectType] = (RUNNING_STATUS)pGrpData->u32WorkState; /* 映射运行状态 */
		
		pCCU->strLSYSInfo.branchNodeData[emRectType].fACDC_Max_Output_Pwr_Ability = pGrpRectData->fRatedPower[emRectType];

		TRACEI("emRectType = %d, fACDC_Max_Output_Pwr_Ability = %f\n", emRectType, pCCU->strLSYSInfo.branchNodeData[emRectType].fACDC_Max_Output_Pwr_Ability);

		pCCU->strLSYSInfo.branchNodeData[iGrp].emACDC_WorkStatu = (RUNNING_STATUS)pGrpData->u32WorkState; /* 映射运行状态 */
		pCCU->strLSYSInfo.branchNodeData[iGrp].fACDC_RealTime_Volt = (float)pGrpData->u32CurrDCVolt * 0.001f;
		pCCU->strLSYSInfo.branchNodeData[iGrp].fACDC_RealTime_Curr = (float)pGrpData->u32CurrDCCurr * 0.001f;
		pCCU->strLSYSInfo.branchNodeData[iGrp].fACDCOutRelayVolt = (float)pGrpData->u32CurrDCVolt * 0.001f;
		pCCU->strLSYSInfo.branchNodeData[iGrp].fActvPwr_PCSGrp = (float)pGrpData->u32CurrSysPower * 0.000001f; /* 映射当前系统功率 */
		// pCCU->strLSYSInfo.branchNodeData[iGrp].fQ_PCSGrp;

    }

    /* 遍历所有组和模块进行单模块数据映射 */
    for (iGrp = 0; iGrp < N2402_MAX_GRP_NUM; iGrp++)
    {
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)iGrp; /* 计算当前组类型 */
		RECT_RAW_N2402_DATA* pModRaw = &pAcData->gs_N2402RectInfo[iGrp]; // todo 改成iGrp+iMod
        for (iMod = 0; iMod < 1; iMod++) //遍历模块数量 先设置为每组一个
        {
            /* 获取当前模块的源数据指针（1D数组按模块号索引） */
            // RECT_RAW_N2402_DATA* pModRaw = &pAcData->gs_N2402RectInfo[iMod];

            /* 跳过不在线、电压为0、或不属于当前组的模块 */
            // if (!pModRaw->bIsExisted || pModRaw->u32ModDCVolt == 0) continue;
            // if (pModRaw->u16BelongedGrp != (iGrp + 1)) continue;

            /* 直接使用数组元素访问，避免使用未定义的 A_BLOB_SINGLE_RECT 指针 */
            pSigRectData->arrRect[emRectType][iMod].bExist = TRUE; /* 映射状态存在标志 */
            pSigRectData->arrRect[emRectType][iMod].nBelongGrp = pModRaw->u16BelongedGrp; /* 映射所属组号 */
            pSigRectData->arrRect[emRectType][iMod].uModuID = iMod + 1; /* 映射单模块号 */

            /* 组合并映射RAM状态信息 (字节0-3) unused */
            // pSigRectData->arrRect[emRectType][iMod].unioRAMStatus.uiInverterStatus = 
            //     (pModRaw->StatCode_12.BYTE_FIELD[0]) | 
            //     ((USHORT)pModRaw->StatCode_12.BYTE_FIELD[1] << 8) |
            //     ((USHORT)pModRaw->StatCode_12.BYTE_FIELD[2] << 16) |
            //     ((USHORT)pModRaw->StatCode_12.BYTE_FIELD[3] << 24);

            /* 映射逆变状态表 (组合高低字节) */
            /* 1-1 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_1_1.uiInverterStatus = 
                (pModRaw->StatCode_12.BYTE_FIELD[0]) | ((USHORT)pModRaw->StatCode_12.BYTE_FIELD[1] << 8);
            /* 1-2 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_1_2.uiInverterStatus = 
                (pModRaw->StatCode_12.BYTE_FIELD[2]) | ((USHORT)pModRaw->StatCode_12.BYTE_FIELD[3] << 8);
            /* 2-1 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_2_1.uiInverterStatus = 
                (pModRaw->StatCode_13.BYTE_FIELD[0]) | ((USHORT)pModRaw->StatCode_13.BYTE_FIELD[1] << 8);
            /* 2-2 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_2_2.uiInverterStatus = 
                (pModRaw->StatCode_13.BYTE_FIELD[2]) | ((USHORT)pModRaw->StatCode_13.BYTE_FIELD[3] << 8);
            /* 3-1 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_3_1.uiInverterStatus = 
                (pModRaw->StatCode_14.BYTE_FIELD[0]) | ((USHORT)pModRaw->StatCode_14.BYTE_FIELD[1] << 8);
            /* 3-2 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_3_2.uiInverterStatus = 
                (pModRaw->StatCode_14.BYTE_FIELD[2]) | ((USHORT)pModRaw->StatCode_14.BYTE_FIELD[3] << 8);
            /* 4-1: 复用 StatCode_15 高位 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_4_1.uiInverterStatus = 
                (pModRaw->StatCode_15.BYTE_FIELD[0]) | ((USHORT)pModRaw->StatCode_15.BYTE_FIELD[1] << 8);
            /* 4-2: 复用 StatCode_15 高位 */
            pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_4_2.uiInverterStatus = 
                (pModRaw->StatCode_15.BYTE_FIELD[2]) | ((USHORT)pModRaw->StatCode_15.BYTE_FIELD[3] << 8);

            /* 映射软件版本号 */
            pSigRectData->arrRect[emRectType][iMod].fRect_DCDCVer = (float)pModRaw->u32DC_Ver; /* DC版本 */
            pSigRectData->arrRect[emRectType][iMod].fRect_PFCVer = (float)pModRaw->u32AC_Ver; /* PFC版本 */

            /* 映射条码信息 (字节拼接) */ // 24 25 26 27
    		BYTE fields[5];
            fields[0] = (pModRaw->bBarcode_24[2] >> 0x04);
            fields[1] = (pModRaw->bBarcode_25[0]);
            fields[2] = (pModRaw->bBarcode_25[1] >> 0x04);
            fields[4] = (pModRaw->bBarcode_25[1] & 0x0F);
			BYTE *pbarcode = pSigRectData->arrRect[emRectType][iMod].byBarCode;
			int i;
			for(i=0; i<3; ++i)
			{
				pbarcode[i*2] = '0' + (fields[i] / 10);
				pbarcode[i*2+1] = '0' + (fields[i] % 10);
			}
			USHORT temp = (USHORT)(pModRaw->bBarcode_25[2]<<8) + pModRaw->bBarcode_25[3];
            for (i = 4; i >= 0; i--) 
            {
                pbarcode[6 + i] = '0' + (temp % 10);
                temp /= 10;
            }
            pbarcode[11] = '0' + fields[4];

            pbarcode += 12;
            for(i = 0; i < 4;i++)
            {
                *pbarcode++ = pModRaw->bBarcode_27[i];
            }
            for(i = 0; i < 4;i++)
            {
                *pbarcode++ = pModRaw->bBarcode_28[i];
            }
            *pbarcode++ = pModRaw->bBarcode_24[2]&0x0f + 'A';

            *pbarcode++ = pModRaw->bBarcode_24[3]/10 + '0';
            *pbarcode++ = pModRaw->bBarcode_24[3]%10 + '0';

                // printf("BARCODE:");
                // int a;
                // for(a=0; a<24; ++a)
                // {
                //     printf("%c",pSigRectData->arrRect[emRectType][iMod].byBarCode[a]);
                // }
                // printf("\n");


            // pSigRectData->arrRect[emRectType][iMod].byBarCode[0] = ((USHORT)pModRaw->bBarcode_24[0] << 8) | pModRaw->bBarcode_24[1];
            // pSigRectData->arrRect[emRectType][iMod].byBarCode[1] = ((USHORT)pModRaw->bBarcode_25[0] << 8) | pModRaw->bBarcode_25[1];
            // pSigRectData->arrRect[emRectType][iMod].byBarCode[2] = ((USHORT)pModRaw->bBarcode_26[0] << 8) | pModRaw->bBarcode_26[1];
            // pSigRectData->arrRect[emRectType][iMod].byBarCode[3] = ((USHORT)pModRaw->bBarcode_27[0] << 8) | pModRaw->bBarcode_27[1];

            /* 映射电压电流数据 (0.1缩放) */
			//todo 是否彻底改用新的变量？fDCsideVoltage fDCsideCurrent
            pSigRectData->arrRect[emRectType][iMod].fDCsideVoltage = (float)pModRaw->u32ModDCVolt * 0.001f; /* 直流侧电压 */
            pSigRectData->arrRect[emRectType][iMod].fDCsideCurrent = (float)pModRaw->u32ModBattCurr * 0.001f; /* 直流侧电流 */
            // 原本
			pSigRectData->arrRect[emRectType][iMod].fRTOutputVolt = (float)pModRaw->u32ModDCVolt * 0.001f; /* 直流侧电压 */
            pSigRectData->arrRect[emRectType][iMod].fRTOutputCurr = (float)pModRaw->u32ModBattCurr * 0.001f; /* 直流侧电流 */
			
            pSigRectData->arrRect[emRectType][iMod].fModuBattSideVolt = pSigRectData->arrRect[emRectType][iMod].fDCsideVoltage; /* 电池侧电压引用 */
            pSigRectData->arrRect[emRectType][iMod].fModuBattSideCurr = pSigRectData->arrRect[emRectType][iMod].fDCsideCurrent; /* 电池侧电流引用 */

            /* 映射交流线电压 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fphase_LineVolt[0] = (float)pModRaw->u32ModACLineVolt_AB * 0.001f; /* AB线压 */
            pSigRectData->arrRect[emRectType][iMod].fphase_LineVolt[1] = (float)pModRaw->u32ModACLineVolt_BC * 0.001f; /* BC线压 */
            pSigRectData->arrRect[emRectType][iMod].fphase_LineVolt[2] = (float)pModRaw->u32ModACLineVolt_CA * 0.001f; /* CA线压 */

            /* 映射相电压 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fPh_InputVolt[0] = (float)pModRaw->u32ModACPhaseVolt_A * 0.001f; /* A相压 */
            pSigRectData->arrRect[emRectType][iMod].fPh_InputVolt[1] = (float)pModRaw->u32ModACPhaseVolt_B * 0.001f; /* B相压 */
            pSigRectData->arrRect[emRectType][iMod].fPh_InputVolt[2] = (float)pModRaw->u32ModACPhaseVolt_C * 0.001f; /* C相压 */

            /* 映射相电流 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fPh_InputCurr[0] = (float)pModRaw->u32ModACPhaseCurr_A * 0.001f; /* A相流 */
            pSigRectData->arrRect[emRectType][iMod].fPh_InputCurr[1] = (float)pModRaw->u32ModACPhaseCurr_B * 0.001f; /* B相流 */
            pSigRectData->arrRect[emRectType][iMod].fPh_InputCurr[2] = (float)pModRaw->u32ModACPhaseCurr_C * 0.001f; /* C相流 */

            /* 映射交流频率 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fAC_Freq = (float)pModRaw->u32ModACFreq * 0.001f;

            /* 映射有功功率 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fPtotalActive = (float)pModRaw->u32ModActivePower_Total * 0.000001f; /* 总有功 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Active[0] = (float)pModRaw->u32ModActivePower_A * 0.000001f; /* A相有功 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Active[1] = (float)pModRaw->u32ModActivePower_B * 0.000001f; /* B相有功 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Active[2] = (float)pModRaw->u32ModActivePower_C * 0.000001f; /* C相有功 */

            /* 映射无功功率 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fPtotalReactive = (float)pModRaw->u32ModReActivePower_Total * 0.000001f; /* 总无功 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Reactive[0] = (float)pModRaw->u32ModReActivePower_A * 0.000001f; /* A相无功 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Reactive[1] = (float)pModRaw->u32ModReActivePower_B * 0.000001f; /* B相无功 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Reactive[2] = (float)pModRaw->u32ModReActivePower_C * 0.000001f; /* C相无功 */

            /* 映射视在功率 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fPTotalApparent = (float)pModRaw->u32ModApparentPower_Total * 0.000001f; /* 总视在 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Apparent[0] = (float)pModRaw->u32ModApparentPower_A * 0.000001f; /* A相视在 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Apparent[1] = (float)pModRaw->u32ModApparentPower_B * 0.000001f; /* B相视在 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Apparent[2] = (float)pModRaw->u32ModApparentPower_C * 0.000001f; /* C相视在 */

            /* 映射功率因数 (0.001缩放) */
            pSigRectData->arrRect[emRectType][iMod].fPtotalFactor = (float)pModRaw->u32ModPowerFactor_Total * 0.001f; /* 总功率因数 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Factor[0] = (float)pModRaw->u32ModPowerFactor_A * 0.001f; /* A相功率因数 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Factor[1] = (float)pModRaw->u32ModPowerFactor_B * 0.001f; /* B相功率因数 */
            pSigRectData->arrRect[emRectType][iMod].fPphase_Factor[2] = (float)pModRaw->u32ModPowerFactor_C * 0.001f; /* C相功率因数 */

            /* 映射环境温度 (0.1缩放) */
            pSigRectData->arrRect[emRectType][iMod].fAmbit_Temp = (float)pModRaw->u32EnvTemp * 0.001f;
            TRACEI("Data_Swap1: 组%d 模块%d 映射完毕\n", iGrp+1, iMod+1);
            TRACEI("  [状态] 存在标志=%d 归属组=%d 模块ID=%d\n",
                pSigRectData->arrRect[emRectType][iMod].bExist,
                pSigRectData->arrRect[emRectType][iMod].nBelongGrp,
                pSigRectData->arrRect[emRectType][iMod].uModuID);
            // TRACEI("  [RAM状态] RAM状态=0x%08X\n",
            //     pSigRectData->arrRect[emRectType][iMod].unioRAMStatus.uiInverterStatus);
            TRACEI("  [逆变状态] 1-1=%u 1-2=%u 2-1=%u 2-2=%u 3-1=%u 3-2=%u 4-1=%u 4-2=%u\n",
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_1_1.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_1_2.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_2_1.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_2_2.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_3_1.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_3_2.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_4_1.uiInverterStatus,
                pSigRectData->arrRect[emRectType][iMod].unioInverterStatus_4_2.uiInverterStatus);
            TRACEI("  [版本号] DC=%.1f PFC=%.1f\n",
                pSigRectData->arrRect[emRectType][iMod].fRect_DCDCVer,
                pSigRectData->arrRect[emRectType][iMod].fRect_PFCVer);
            TRACEI("  [条码] 条码0=%u 条码1=%u 条码2=%u 条码3=%u\n",
                pSigRectData->arrRect[emRectType][iMod].byBarCode[0],
                pSigRectData->arrRect[emRectType][iMod].byBarCode[1],
                pSigRectData->arrRect[emRectType][iMod].byBarCode[2],
                pSigRectData->arrRect[emRectType][iMod].byBarCode[3]);
            TRACEI("  [直流侧] 电压=%.1fV 电流=%.1fA 电池侧电压=%.1fV 电池侧电流=%.1fA\n",
                pSigRectData->arrRect[emRectType][iMod].fDCsideVoltage,
                pSigRectData->arrRect[emRectType][iMod].fDCsideCurrent,
                pSigRectData->arrRect[emRectType][iMod].fModuBattSideVolt,
                pSigRectData->arrRect[emRectType][iMod].fModuBattSideCurr);
            TRACEI("  [交流线电压] AB=%.1fV BC=%.1fV CA=%.1fV\n",
                pSigRectData->arrRect[emRectType][iMod].fphase_LineVolt[0],
                pSigRectData->arrRect[emRectType][iMod].fphase_LineVolt[1],
                pSigRectData->arrRect[emRectType][iMod].fphase_LineVolt[2]);
            TRACEI("  [相电压] A=%.1fV B=%.1fV C=%.1fV\n",
                pSigRectData->arrRect[emRectType][iMod].fPh_InputVolt[0],
                pSigRectData->arrRect[emRectType][iMod].fPh_InputVolt[1],
                pSigRectData->arrRect[emRectType][iMod].fPh_InputVolt[2]);
            TRACEI("  [相电流] A=%.1fA B=%.1fA C=%.1fA\n",
                pSigRectData->arrRect[emRectType][iMod].fPh_InputCurr[0],
                pSigRectData->arrRect[emRectType][iMod].fPh_InputCurr[1],
                pSigRectData->arrRect[emRectType][iMod].fPh_InputCurr[2]);
            TRACEI("  [交流频率] Freq=%.1fHz\n", pSigRectData->arrRect[emRectType][iMod].fAC_Freq);
            TRACEI("  [有功功率] 总=%.1fkW A=%.1fkW B=%.1fkW C=%.1fkW\n",
                pSigRectData->arrRect[emRectType][iMod].fPtotalActive,
                pSigRectData->arrRect[emRectType][iMod].fPphase_Active[0],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Active[1],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Active[2]);
            TRACEI("  [无功功率] 总=%.1fkVar A=%.1fkVar B=%.1fkVar C=%.1fkVar\n",
                pSigRectData->arrRect[emRectType][iMod].fPtotalReactive,
                pSigRectData->arrRect[emRectType][iMod].fPphase_Reactive[0],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Reactive[1],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Reactive[2]);
            TRACEI("  [视在功率] 总=%.1fkVA A=%.1fkVA B=%.1fkVA C=%.1fkVA\n",
                pSigRectData->arrRect[emRectType][iMod].fPTotalApparent,
                pSigRectData->arrRect[emRectType][iMod].fPphase_Apparent[0],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Apparent[1],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Apparent[2]);
            TRACEI("  [功率因数] 总=%.3f A=%.3f B=%.3f C=%.3f\n",
                pSigRectData->arrRect[emRectType][iMod].fPtotalFactor,
                pSigRectData->arrRect[emRectType][iMod].fPphase_Factor[0],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Factor[1],
                pSigRectData->arrRect[emRectType][iMod].fPphase_Factor[2]);
            TRACEI("  [环境温度] %.1fC\n", pSigRectData->arrRect[emRectType][iMod].fAmbit_Temp);
        }
    }

    /* 统计在线模块总数 */
    int nTotalOnLinePM = 0;
    for(iGrp = 0; iGrp < N2402_MAX_GRP_NUM; iGrp++)
    {
        nTotalOnLinePM += pGrpRectData->nOnlineRectNum[(TYPE_SIGNLE_RECT)iGrp]; /* 累加各组数量 */
    }

    /* 如果无在线模块，清空单模块数据 */
    // if(nTotalOnLinePM == 0)
    // {
    //     TRACEI("Data_Swap1: 无在线模块，清空单模块数据\n");
    //     // memset(&pCCU->strLSYSInfo.singleRectData, 0 , sizeof(A_BLOB_SINGLE_RECT_INFO));
    // }

    // TRACEI("Data_Swap1: 映射完成 总在线模块数=%d\n", nTotalOnLinePM);
    return TRUE;
}

void PushPCSData(ROUGH_CCU_DATA* pCCU)
{
	// EMS_MGMT_STU *pEMS = &pCCU->emsMgmtData;

	//TRACE("---CCU Tcp Sampler Main PushAllData---\n");
	int n = 0; // m = 0;
	VAR_VALUE varV;
	char szSingleRectName[64] = "\0";

	varV.emValue = pCCU->emCCUState;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 4, varV);

	varV.emValue = pCCU->emSystemFailReason;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 6, varV);

 
	varV.emValue = pCCU->emsMgmtData.bEnterUrgetDischrgMode;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 9, varV);

	varV.emValue = pCCU->emsMgmtData.bEnterAnti_Reflux;
 	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 25, varV);

    varV.emValue = pCCU->emsMgmtData.bEnter_OverLoad;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 26, varV);
	// LJH240705 AitiOverLoad HVMeter 240822:跟随高压表最大需量+OA(防超需量模式)
	varV.emValue = pCCU->emsMgmtData.bEnter_OverLoadHV;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 28, varV);

	varV.emValue = pCCU->emErrCode_MgmtCfg;
	Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 200, 102, varV);

	varV.emValue =  pCCU->emCCUState;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 103, varV);
 
	//Push Single Battery DataInfo To Branch Struct
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		//pCCU->pDPR->Get(pCCU->hSampler, GETTYPE_SP_VAR, 501 + n, 551, &varV, 
		//		sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));

		pCCU->strLSYSInfo.branchNodeData[n].fBatt_RealTime_Volt = pCCU->blobSingleBattData[n].fPackTotalVolt;
		pCCU->strLSYSInfo.branchNodeData[n].fBatt_RealTime_Curr = pCCU->blobSingleBattData[n].fPackTotalCurr;
		pCCU->strLSYSInfo.branchNodeData[n].nSOC = pCCU->blobSingleBattData[n].nSOC;
		pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_DisChrg_Pwr = fabs(pCCU->blobSingleBattData[n].fPackTotalVolt*pCCU->blobSingleBattData[n].fAllow_MaxDisChargeCurr*0.001);
		pCCU->strLSYSInfo.branchNodeData[n].fBatt_Max_Chrg_Pwr_Req = fabs(pCCU->blobSingleBattData[n].fAllow_LmtChargeVolt*pCCU->blobSingleBattData[n].fAllow_MaxChargeCurr*0.001);	

		pCCU->strLSYSInfo.branchNodeData[n].nOnlineACDCNum = pCCU->strLSYSInfo.singleRectData.nOnlineNum[(TYPE_SIGNLE_RECT)n];
		pCCU->strLSYSInfo.branchNodeData[n].nOnlineMPPTNum = pCCU->strLSYSInfo.singleRectData.nOnlineNum[(TYPE_SIGNLE_RECT)(n + 4)];

		//Stuff Branch Node Data - 2021-10-11
		Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 211 + n, 201, (void*)&pCCU->strLSYSInfo.branchNodeData[n], sizeof(A_BLOB_BRANCH_GRP_RECTDATA), BLOB_TYPE_BRANCH_GRP_RECT_DATA);
	}

	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 101, (void*)&pCCU->strLSYSInfo.singleRectData, sizeof(A_BLOB_SINGLE_RECT_INFO), BLOB_TYPE_SINGLE_RECT_DATAINFO);
	
	Push_BlobValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 204, (void*)&pCCU->strLSYSInfo.blobGrpRectData, sizeof(A_BLOB_RECT_GRP_DATA), BLOB_TYPE_GRP_PM_DATAINFO);

	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		Push_BlobValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 221 + n, 203, (void*)&pCCU->emsMgmtData.calc_Node[n], sizeof(A_BLOB_BRANCH_NODE_CALC_DATA), BLOB_TYPE_BRANCH_NODE_CALC_DATA);
	}
	
 	for(n = 0; n < 8; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.mapSTSCode.byRunStatus >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 160 + n, varV);
	}
	
	varV.ulValue = (ULONG)pCCU->strLSYSInfo.mapAbuStatu1.byRunStatus;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 169, varV);

	varV.emValue = (_ENUM)pCCU->strLSYSInfo.mapAbuStatu1.Status.Slave0rMaster ? 0 : 1;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1, 13, varV);

	//Attention Alarm Bit Map: Stuff Status Parameter Signal should be continuously 2021-10-26
	//----- SigID 240~271 -----//
	// for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	// {
	// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap1.uiFlag >> n) & 0x01);
	// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 240 + n, varV);
	// }
	 
	// //----- SigID 304~335 -----//
	// for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	// {
	// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap3.uiFlag >> n) & 0x01);
	// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 304 + n, varV);
	// }
	
	// //----- SigID 336~367 -----// 20230905 John Modify DCLoad Alarm.
	
	// for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	// {
	// 	varV.emValue = (_ENUM)((pCCU->strLSYSInfo.sysAlarmMap4.uiFlag >> n) & 0x01);
	// 	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 336 + n, varV);
	// }
	

	//----- SigID 368~399 -----//
	for(n = 0; n < MAX_ALARMBIT_MAP; n++)
	{
		varV.emValue = (_ENUM)((pCCU->strLSYSInfo.pmGrpAlmMap.uiFlag >> n) & 0x01);
		Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 368 + n, varV);
	}
	
	//John 20230905. added For Judge Need Single PCS Alarm.
	BOOL bNeedShieldSinglePcsAlm = JudgeisNeedSkipSinglePCSAlarm(pCCU);

	//---- SigID 1201~1232 ----//
	UINT nOnline = 0;
	// if(pCCU->nStuffWaitTimes == 0)
	// {
 	// 	BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP1);

 	// 	for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1]; nOnline++)
	// 	{
	// 		if(bNeedShieldSinglePcsAlm || bOnIsoPending|| (pCCU->pNewSet.ccuSettings.byPCSType == 2))
	// 		{
	// 			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].mapPCSCode.uiPCSStatus = 0;
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1201 + n, varV);
	// 			continue;
	// 		}

	// 		for(n = 8; n < MAX_ALARMBIT_MAP; n++)
	// 		{
	// 			varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP1][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + nOnline, 1201 + n, varV);
	// 		}
	// 	}
	// }
	// if(pCCU->nStuffWaitTimes == 1)
	// {
 	// 	BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP2);

 	// 	for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2]; nOnline++)
	// 	{
	// 		if(bNeedShieldSinglePcsAlm || bOnIsoPending || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
	// 		{
	// 			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].mapPCSCode.uiPCSStatus = 0;
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1201 + n, varV);
	// 			continue;
	// 		}
	// 		for(n = 8; n < MAX_ALARMBIT_MAP; n++)
	// 		{
	// 			varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP2][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 257 + nOnline, 1201 + n, varV);
	// 		}
	// 	}
	// }

	// if(pCCU->nStuffWaitTimes == 2)
	// {
 	// 	BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP3);

 	// 	for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP3]; nOnline++)
	// 	{
	// 		if(bNeedShieldSinglePcsAlm || bOnIsoPending || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
	// 		{
	// 			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].mapPCSCode.uiPCSStatus = 0;
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1201 + n, varV);
	// 			continue;
	// 		}
	// 		for(n = 8; n < MAX_ALARMBIT_MAP; n++)
	// 		{
	// 			varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP3][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 263 + nOnline, 1201 + n, varV);
	// 		}
	// 	}
	// }

	// if(pCCU->nStuffWaitTimes == 3)
	// {
 	// 	BOOL bOnIsoPending = JudgeIsoPendingStatus(pCCU, TYPE_SIG_RECT_ACDC_GRP4);

 	// 	for(nOnline = 0; nOnline < pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP4]; nOnline++)
	// 	{
	// 		if(bNeedShieldSinglePcsAlm || bOnIsoPending || (pCCU->pNewSet.ccuSettings.byPCSType == 2))
	// 		{
	// 			pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].mapPCSCode.uiPCSStatus = 0;
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1201 + n, varV);
	// 			continue;
	// 		}
	// 		for(n = 8; n < MAX_ALARMBIT_MAP; n++)
	// 		{
	// 			varV.emValue = (_ENUM)((pCCU->strLSYSInfo.singleRectData.arrRect[TYPE_SIG_RECT_ACDC_GRP4][nOnline].mapPCSCode.uiPCSStatus >> n) & 0x01);
	// 			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 269 +  nOnline, 1201 + n, varV);
	// 		}
	// 	}
	// }
	if(pCCU->nStuffWaitTimes == 1)
	{
		float  fMax_SysTemp = 0.0;
		for(n = 0; n < MAX_AMBINENT_TEMP; n++)
		{
			varV.fValue = pCCU->strLSYSInfo.fAmbientTemp[n];
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 1, 41 + n, varV);
			//WWJ Suggest Normal Samp Temp Range:[-40,125];
			if((pCCU->strLSYSInfo.fAmbientTemp[n] < 125) && (pCCU->strLSYSInfo.fAmbientTemp[n] > -40))
			{
				fMax_SysTemp = MAX(fMax_SysTemp, pCCU->strLSYSInfo.fAmbientTemp[n]);
			}
		}
		//Stuff System Max Ambinent Temperature
		varV.fValue = fMax_SysTemp;
		Push_VarValue(SETTYPE_LOGIC_SP, pCCU->hSampler, 1, 3, varV);

	}
	if(pCCU->nStuffWaitTimes == 2)
	{
		pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP1] = 1;
		pCCU->strLSYSInfo.singleRectData.nOnlineNum[TYPE_SIG_RECT_ACDC_GRP2] = 1;

		for(n = 0; n < 2; ++n)
		{
			// 组号
			varV.ulValue  = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].nBelongGrp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5292, varV);

			// 单模块号
			varV.ulValue  = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].uModuID;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5293, varV);

			// 状态信息
			STU_SIGNLE_RECT_DATA *pArrRect = &pCCU->strLSYSInfo.singleRectData.arrRect[n][0];
			// switch (pCCU->pOldSet.ccuSettings.emACDCType)   // ACDC设备类型
			switch (pCCU->pOldSet.ccuSettings.byPCSType)
			{
				case ACDC_NONE:
					break;
				// case ACDC_N2402_235KW:
				case 3:
					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bReserved2,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bAuxFault,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bRelayErr,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bUserSetErr,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bBattEodAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bBattDodAlarm,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bVrtFault,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bOverBusVolt,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bReserved1,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bOnOff,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bCharge,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.bDischarge,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.emGridStatus,
						pArrRect->unioInverterStatus_1_1.ACDC_N2002_N2402.emRunningStatus
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5312, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bSysFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bAcFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bDcFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bBusFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bInnerOverCurr,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutShortCircuit,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutOverload,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOutDerating,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bCommFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bIslandingProtect,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bPhaseLockFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bOverTempFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bFanFault,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bCurrUnbalance,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.IDRepeat,
						pArrRect->unioInverterStatus_1_2.ACDC_N2002_N2402.bEOPFault
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5313, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bBusUnderVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputNoPower,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputPhaseLoss,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputTransientDrop,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bSwSampleOverCurr,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bActzOverCurr,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bInputVoltUnbalance,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bPhaseSeqError,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bSciCommFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bSpiCommFault,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcOverVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcUnderVolt,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcOverFreq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bAcUnderFreq,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bBusAbnormal,
						pArrRect->unioInverterStatus_2_1.ACDC_N2402.bBusOverVolt
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5314, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bOutUnderVoltProtect,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bOutShortAlarm,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bGridRelayOpen,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bGridRelayShort,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bInvRelayOpen,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bInvRelayShort,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bDcRelayFault,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bFreqSyncAbnormal,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bPhaseLockErr2,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bPhaseLockErr1,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bAcRelayFault,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bAcShortCircuit,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bIslandingProtect,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bEStop,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bBattReverseConn,
						pArrRect->unioInverterStatus_2_2.ACDC_N2402.bOutOverVolt
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5315, szSingleRectName, 64);

					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.Reserved8_15,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bParallelCommFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bDcSoftStartRelayFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bBusSoftStartFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bInvSoftStartFault,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.bOutCurrUnbalance,
						pArrRect->unioInverterStatus_3_1.ACDC_N2402.Reserved0_2
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5316, szSingleRectName, 64);


					snprintf(szSingleRectName, 64, "%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d", 
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bAuxPowerFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bInsulationFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bLeakageCurrFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bFanFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bVoSetOverProtect,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bVoSetTooLow,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIdRepeat,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bSoftStartResOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bCanCommFault,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bWaterInletOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bWaterOutletOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmAOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmBOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmCOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bIpmDOverTemp,
						pArrRect->unioInverterStatus_4_1.ACDC_N2402.bAmbientOverTemp
					);
					Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5318, szSingleRectName, 64);
					break;

				default:
					break;
			}

			// DC软件版本号
			varV.ulValue = (UINT)(pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fRect_DCDCVer*100);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1425, varV);
			
			// PFC软件版本号
			varV.ulValue = (UINT)(pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fRect_PFCVer*100);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1426, varV);
			
			// 条码信息
			DecodeRectBarcode(pCCU->strLSYSInfo.singleRectData.arrRect[n][0].uiBarCode, szSingleRectName, 64);
			TRACED("barcode: %s\n", szSingleRectName);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5294, szSingleRectName, 64);

			// 模块直流侧电压
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fDCsideVoltage;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5295, varV);

			// 模块直流侧电流
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fDCsideCurrent;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5296, varV);

			// 模块环境温度
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fAmbit_Temp;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5297, varV);

			// 交流线电压
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fphase_LineVolt[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fphase_LineVolt[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fphase_LineVolt[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5298, szSingleRectName, 64);

			// 交流相电压
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputVolt[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputVolt[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputVolt[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5299, szSingleRectName, 64);

			// 交流相电流
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputCurr[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputCurr[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPh_InputCurr[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5300, szSingleRectName, 64);

			// 交流频率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fAC_Freq;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5301, varV);
			
			// 总有功功率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPtotalActive;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5302, varV);

			// 三相有功功率
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Active[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Active[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Active[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5303, szSingleRectName, 64);

			// 总无功功率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPtotalReactive;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5304, varV);

			// 三相无功功率
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Reactive[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Reactive[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Reactive[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5305, szSingleRectName, 64);

			// 总视在功率
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPTotalApparent;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5306, varV);

			// 三相视在功率
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f", 
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Apparent[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Apparent[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Apparent[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5307, szSingleRectName, 64);

			// 总功率因数
			varV.fValue = pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPtotalFactor;
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5308, varV);

			// 三相功率因数
			snprintf(szSingleRectName, 64, "%.1f|%.1f|%.1f",
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Factor[0],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Factor[1],
					pCCU->strLSYSInfo.singleRectData.arrRect[n][0].fPphase_Factor[2]);
			Push_TextValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 5309, szSingleRectName, 64);
		}

		int j;
		for(n = 0; n < 2; ++n)
		{
			STU_SIGNLE_RECT_DATA *pArrRect = &pCCU->strLSYSInfo.singleRectData.arrRect[n][0];

			for(j = 0; j < BYTE_NUM; j++)
			{
				varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_12.BYTE_FIELD[1] >> j) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1209 + j, varV);
			}
			for(j = 0; j < BYTE_NUM; j++)
			{
				varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_12.BYTE_FIELD[2] >> j) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1217 + j, varV);
			}

			for(j = 0; j < BYTE_NUM; j++)
			{
				varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_12.BYTE_FIELD[3] >> j) & 0x01);
				Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1225 + j, varV);
			}

			/* ============ StatCode_13: 逆变状态表(0x13) 告警位 ============ */
			/* ---- BYTE_FIELD[0]: 通讯/电气 (bit0~7) ---- */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 0) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1298, varV); /* bit0: bSCIComFault         -> SCI通讯故障 (SCI Comm Failure) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 1) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1299, varV); /* bit1: bSPIComFault         -> SPI通讯故障 (SPI Comm Failure) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 2) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1438, varV); /* bit2: bACOverVoltage       -> 交流过压 (AC OverVoltage) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1437, varV); /* bit3: bACUnderVoltage      -> 交流欠压 (AC UnderVoltage) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 4) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1436, varV); /* bit4: bACOverFreq          -> 交流过频 (AC OverFrequency) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 5) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1435, varV); /* bit5: bACUnderFreq         -> 交流欠频 (AC UnderFrequency) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 6) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1434, varV); /* bit6: bBusAbnormal         -> 母线异常 (Bus Abnormal) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[0] >> 7) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1301, varV); /* bit7: bBusOverVoltage      -> 母线过压 (Bus OverVoltage) */

			/* ---- BYTE_FIELD[1]: 电网/采样异常 (bit0~7) ---- */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[1] >> 0) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1302, varV); /* bit0: bBusUnderVoltage     -> 母线欠压 (Bus UnderVoltage) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[1] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1442, varV); /* bit3: bInputFlicker        -> 输入闪断 (Input Flashing) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[1] >> 4) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1441, varV); /* bit4: bSoftwareOverCurrent -> 软件采样过流 (Software Sampling Overcurrent) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[1] >> 6) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1440, varV); /* bit6: bInputVolUnbalance   -> 三相输入不平衡 (AC_NoBalance) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[1] >> 7) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1439, varV); /* bit7: bPhaseSeqError       -> 相序错误 (Phase Sequence Error) */

			/* ---- BYTE_FIELD[2]: 锁相/继电器/输出 (bit0~7) ---- */
			varV.emValue = (_ENUM)(((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[2] >> 0) & 0x03) ? 1 : 0);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1446, varV); /* bit0|1: bPLLError(Bit7|6) -> 离网锁相错误 (Offline Phase Lock Failure) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[2] >> 2) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1444, varV); /* bit2: bACRelayFault        -> AC继电器故障 (AC Relay Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[2] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1289, varV); /* bit3: bACShortCircuit      -> 交流短路 (AC Short Circuit) */
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1445, varV); /* bit3: bACShortCircuit      -> AC输入短路 (AC Input Short Circuit) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[2] >> 7) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1443, varV); /* bit7: bOutputOverVoltage   -> 输出过压 (Output OverVoltage) */

			/* ---- BYTE_FIELD[3]: 继电器/同步 (bit0~7) ---- */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 0) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1447, varV); /* bit0: bOutputUnderVolProt  -> 输出欠压保护 (Output Undervoltage Protection) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 2) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1275, varV); /* bit2: bGridRelayOpen       -> 网侧继电器开路 (Grid Relay Open) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1276, varV); /* bit3: bGridRelayShort      -> 网侧继电器短路 (Grid Relay Short) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 4) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1277, varV); /* bit4: bInvRelayOpen        -> 逆变侧继电器开路 (Inverter Relay Open) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 5) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1278, varV); /* bit5: bInvRelayShort       -> 逆变继电器短路 (Inverter Relay Short) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 6) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1279, varV); /* bit6: bDCRelayFault        -> 直流继电器故障 (DC Relay Driver Failure) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_13.BYTE_FIELD[3] >> 7) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1296, varV); /* bit7: bSynchroAbnormal     -> 交流同步故障 (AC Sync Fault) */

			/* ============ StatCode_14: 逆变状态表(0x14) 告警位 ============ */
			/* ---- BYTE_FIELD[0]: 并机/软启/不均流 (bit0~4) ---- */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_14.BYTE_FIELD[0] >> 0) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1288, varV); /* bit0: bParallelComFault       -> 并机通讯故障 (Parallel Comm Failure) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_14.BYTE_FIELD[0] >> 1) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1287, varV); /* bit1: bDCRelaySoftStartFault  -> 直流软启继电器故障 (DC SoftStart Relay Fail) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_14.BYTE_FIELD[0] >> 2) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1273, varV); /* bit2: bBusSoftStartFault      -> 母线软启动故障 (Bus Soft Start Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_14.BYTE_FIELD[0] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1274, varV); /* bit3: bInvSoftStartFault      -> 逆变软启故障 (Inverter Soft Start Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_14.BYTE_FIELD[0] >> 4) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1448, varV); /* bit4: bOutputCurrentImbalance -> 输出电流不均流 (Uneven Output Current) */

			/* ============ StatCode_15: 温度/辅助告警表(0x15) 告警位 ============ */
			/* ---- BYTE_FIELD[0]: CAN通讯/温度 (bit0~7) ---- */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 0) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1297, varV); /* bit0: bCANComFault        -> CAN通讯故障 (CAN Comm Failure) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 1) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1454, varV); /* bit1: bInletOverTemp      -> 进水口过温 (Inlet Over Temperature) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 2) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1453, varV); /* bit2: bOutletOverTemp     -> 出水口过温 (Outlet Over Temperature) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1303, varV); /* bit3: bIPMA_OverTemp      -> 散热器/IPMA过温 (Radiator Over Temperature) */
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1452, varV); /* bit3: bIPMA_OverTemp      -> IPMA过温 (IPMA Over Temperature) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 4) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1451, varV); /* bit4: bIPMB_OverTemp      -> IPMB过温 (IPMB Over Temperature) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 5) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1450, varV); /* bit5: bIPMC_OverTemp      -> IPMC过温 (IPMC Over Temperature) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 6) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1449, varV); /* bit6: bIPMD_OverTemp      -> IPMD过温 (IPMD Over Temperature) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[0] >> 7) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1304, varV); /* bit7: bAmbientOverTemp    -> 环温过温 (Ambient Over Temperature) */

			/* ---- BYTE_FIELD[1]: 辅源/绝缘/风扇/设置 (bit0~7) ---- */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 0) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1305, varV); /* bit0: bAuxPowerFault          -> 辅源故障 (Auxiliary Power Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 1) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1457, varV); /* bit1: bInsulationDetectFault  -> 绝缘检测故障 (Insulation Detection Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 2) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1456, varV); /* bit2: bLeakageCurrentFault    -> 漏电流故障 (Leakage Current Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 3) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1308, varV); /* bit3: bFanFault               -> 风扇故障 (Fan Fault) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 4) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1309, varV); /* bit4: bVoSettingOverProt      -> 电压设置超保护值 (Vo Set OverProtPoint) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 5) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1310, varV); /* bit5: bVoSettingLow           -> 电压设置偏低 (Vo Setting is Low) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 6) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1312, varV); /* bit6: bIDDuplication          -> ID重复 (Duplicated ID) */
			varV.emValue = (_ENUM)((pCCU->acdc_data.gs_N2402RectInfo[n].StatCode_15.BYTE_FIELD[1] >> 7) & 0x01);
			Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 251 + n*6, 1455, varV); /* bit7: bSoftStartResOverTemp   -> 软起电阻过温 (Softstart Resistor Over Temperature) */
		}
	}

	pCCU->nStuffWaitTimes++; //Added by John 2022-03-31.In Order To Improve CPU Capbility
	if(pCCU->nStuffWaitTimes > 3)
	{
		pCCU->nStuffWaitTimes = 0;
	}

	varV.emValue = pCCU->strLSYSInfo.emRunningMode;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 152, varV);
 
	//Stuff MPPT KWH
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
	{
		if(fabs(pCCU->strLSYSInfo.fMPPTGenerateKWH[n] - pCCU->strLSYSInfo.fLast_MPPTGenerateKWH[n]) > 0.5)
		{
			pCCU->strLSYSInfo.fLast_MPPTGenerateKWH[n] = pCCU->strLSYSInfo.fMPPTGenerateKWH[n];
			varV.fValue = pCCU->strLSYSInfo.fMPPTGenerateKWH[n];
			Push_VarValue(SETTYPE_INTERN_SETT,  pCCU->hSampler, 200, 204 + n, varV);
		}
	}
 
	if(fabs(pCCU->strLSYSInfo.fDCLoad_CostKWH - pCCU->strLSYSInfo.fLast_DCLoad_CostKWH) > 1.0)
	{
		pCCU->strLSYSInfo.fLast_DCLoad_CostKWH = pCCU->strLSYSInfo.fDCLoad_CostKWH;
		varV.fValue = pCCU->strLSYSInfo.fDCLoad_CostKWH;
		Push_VarValue(SETTYPE_INTERN_SETT,  pCCU->hSampler, 200, 208, varV);
	}
	
	varV.emValue = pCCU->emCurrDiffAbnormal;
	Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 200, 1910, varV);
	return;
}
