// #define _TRACE_PRINT_   //RPMSG

#include "ccu_sampler_main.h"

typedef enum tagDataSwapType
{
    DATA_SWAP_AMI = 0,   // 整体模块信息
    DATA_SWAP_ACDC_GRP,  // ACDC模块组信息
    DATA_SWAP_DCDC_GRP,  // DCDC模块组信息
    DATA_SWAP_MPPT_GRP,  // MPPT模块组信息
    DATA_SWAP_ACDC_Sig,  // ACDC单模块信息
    DATA_SWAP_DCDC_Sig,  // DCDC单模块信息
    DATA_SWAP_MPPT_Sig,  // MPPT单模块信息
    MAX_DATA_SWAP
} DATA_SWAP_TYPE;

RPMSG_FRAME_INFO* Find_RPMsgFrame(IN ROUGH_CCU_DATA* pCCU, IN BYTE* pRaw, IN int nReadLen,
    OUT AR_FRAME* pMsgFrame)
{
    int n, m;
    BYTE* byPos = pRaw;
    UINT nParsedDataLen = 0;

    RPMSG_FRAME_INFO* pFoundMsg = NULL;
    //Notice: only find the first frame, if have multi frames, ignore the followed others
    
    for(n = 0; n < nReadLen - RPMSG_MIN_LEN + 1; n++)
    {
        // 1. Found SOI
        if((byPos[n] == (BYTE)(RPMSG_SOI >> 8)) && 
            (byPos[n+1] == (BYTE)(RPMSG_SOI & 0xFF)))
        {
            // 2. Get Data Length
            nParsedDataLen = (UINT)(byPos[n+2] | (byPos[n+3] << 8) | (byPos[n+4] << 16) | (byPos[n+5] << 24));

            // 3. data
            if (!AR_ProcessFrame(&byPos[n+2], pMsgFrame))
            {
                TRACED("Process Frame[%02X] failed! \n", byPos[n+2]);
                continue;
            }

            // Check
            if((pMsgFrame->ID_SrcAddr != ADDR_R_KERNEL) || 
                (pMsgFrame->ID_DstAddr < ADDR_A_KERNEL_MIN) ||
                (pMsgFrame->ID_DstAddr > ADDR_A_KERNEL_MAX)  ||
                ((pMsgFrame->ID_FunCode != FUNCODE_PROACTIVE_UDP) &&
                 (pMsgFrame->ID_FunCode != FUNCODE_MODIFY_REPLY)  &&
                 (pMsgFrame->ID_FunCode != FUNCODE_QUERY_REPLY)))
            {
                continue;
            }

            for(m = 0; m < RP_MSG_MAX; m++)
            {
                if(pCCU->rpFrameInfo[m].byCmd == pMsgFrame->ID_MainIndex)
                {
                    pFoundMsg = &pCCU->rpFrameInfo[m];
                    break;
                }
            }

            if (pFoundMsg == NULL)
            {
                continue;
            }

            // check data length
            if((nParsedDataLen > AR_MAX_DATA_LEN) || (pMsgFrame->ID_Datalen != nParsedDataLen))
            {
                TRACED("Found Msg[%02X] but the datalen[%d] is INVALID\n", byPos[n + 2], nParsedDataLen);
				//return NULL;
				pFoundMsg = NULL;
				continue;
            } 

            // 4. EOI
            if (byPos[n + 2 + nParsedDataLen] != RPMSG_EOI)  // +2 is to skip SOI(2) and DataLen(2)
            {
                TRACED("Found Msg[%02X] but the EOI[%02X] is INVALID \n", byPos[n + 2],
                    byPos[n + 2 + nParsedDataLen]);
                // return NULL;
                pFoundMsg = NULL;
                continue;
            }
            break;
        }
    }
    return pFoundMsg;
}

static ULONG Bytes2Ulong(BYTE* pBytes, int nBytes)
{
    ULONG ulVal = 0;
    int n;
    for (n = 0; n < nBytes; n++)
    {
        ulVal += (pBytes[n] << (n * 8));
    }

    return ulVal;
}

int Parse_By_Dictionary(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame, const RPMSG_PARSER_RULE* pDict, BYTE nDictSize)
{
    USHORT usParsedLen = 0;
    UINT   n = 0;
    SHORT  sVal;
    BYTE nSubIdx_Num = 0;
    BYTE nSubInx_Start = 0;

    BYTE* pData = &pMsgFrame->Data;

    // Check
    if ((BYTE)pData[0] != FRAME_FLAG)
    {
        TRACED("Invalid frame identifier. Frame Flag[%02X] \n", (BYTE)pData[0]);
        return FALSE;
    }
    pData += 1;
    usParsedLen += 1; 

    // Sub index number
    nSubIdx_Num = (BYTE)pData[0];
    // TRACED("nSubIdx_Num = [%d]\n", nSubIdx_Num);
    pData += 1;
    usParsedLen += 1; 

    // Sub index start
    nSubInx_Start = (BYTE)pData[0];
    // TRACED("nSubInx_Start = [%d]\n", nSubInx_Start);
    pData += 1;
    usParsedLen += 1;
    
    // check
    if ((nDictSize - nSubInx_Start) <  nSubIdx_Num)
    {
        TRACED("Sub-index number out of bounds. nSubIdx_Num：[%d], nDictSize:[%d], nSubInx_Start:[%d].\n", 
                nSubIdx_Num, nDictSize, nSubInx_Start);
        return FALSE;
    }

    BYTE nCurrentIdx = nSubInx_Start;

    for (n = 0; n < nSubIdx_Num; n++)
    {
        // check
        if (nCurrentIdx < 0 || nCurrentIdx >= nDictSize)
        {
            TRACED("Unknown index: %d\n", nCurrentIdx);
            return FALSE;
        }
        // Parse Rule
        const RPMSG_PARSER_RULE* pRule = &pDict[nCurrentIdx];

        // Boundary Protect
        if (usParsedLen + pRule->nLen + 8 > pMsgFrame->ID_Datalen)
        {
            TRACED("Payload length error at index[%d].\n", nCurrentIdx);
            return FALSE;
        } 

        // Absolute address
        HANDLE pTargetVar = (HANDLE)((BYTE*)pCCU + pRule->nOffset);

        // Copy
        switch (pRule->emDataType)
        {
            case MDT_BYTE:
                *((BYTE*)pTargetVar) = (BYTE)Bytes2Ulong(pData, pRule->nLen);
                break;

            case MDT_USHORT:
                *((USHORT*)pTargetVar) = (USHORT)Bytes2Ulong(pData, pRule->nLen);
                break;

            case MDT_UINT:
                *((UINT*)pTargetVar) = (UINT)Bytes2Ulong(pData, pRule->nLen);
                break;
                
            default:
                TRACED("Unsupported data length: [%d]\n", pRule->nLen);
                break;
        }

        // offset
        pData += pRule->nLen;
        usParsedLen += pRule->nLen;
        nCurrentIdx++;
    }

    if ((usParsedLen+8) != pMsgFrame->ID_Datalen)
    {
        TRACED("Data length mismatch. usParsedLen:[%d], ID_Datalen:[%d]", usParsedLen+8, pMsgFrame->ID_Datalen);
        return FALSE;
    }

    return TRUE;
}

static void Map_InverterStatus_ToPCSStatus(STU_SIGNLE_RECT_DATA* pRect)
{
    // 将 InverterStatus 位域组装为 PushAllData 使用的 uiPCSStatus UINT 位图
    // _1_1 bit14(bAuxFault) 重映射到 PCSStatuCode bit15(AuxPower_Fault)
    // PCSStatuCode bit14(SoftStartFault) 预留置0
    UINT usPCSStatus_Low16 = pRect->unioInverterStatus_1_1.uiInverterStatus;
    usPCSStatus_Low16 = (usPCSStatus_Low16 & ~((1u<<14)|(1u<<15))) | ((usPCSStatus_Low16 & (1u<<14)) << 1);
    pRect->mapPCSCode.uiPCSStatus = 
        ((UINT)pRect->unioInverterStatus_1_2.uiInverterStatus << 16) | usPCSStatus_Low16;
}


static USHORT GetGroupIndexByMainIndex(int nMainIndex)
{
    switch(nMainIndex)
    {
        case 0x51: // RP_MSG_ACDC_GRP_1_INFO
        case 0x53: // RP_MSG_DCDC_GRP_1_INFO
        case 0x55: // RP_MSG_ACDC_SIG_1_INFO
        case 0x57: // RP_MSG_DCDC_SIG_1_INFO
            return 0; // 组1
        case 0x52: // RP_MSG_ACDC_GRP_2_INFO
        case 0x54: // RP_MSG_DCDC_GRP_2_INFO
        case 0x56: // RP_MSG_ACDC_SIG_2_INFO
        case 0x58: // RP_MSG_DCDC_SIG_2_INFO
            return 1; // 组2
        default:
            return -1; // 无效主索引
    }
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/30 14:09:40
*  DESCRIPTION   :  核间通信数据交互，提高代码复用性
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Data_Swap(ROUGH_CCU_DATA* pCCU, DATA_SWAP_TYPE emSwapType, USHORT nGrpID)
{
    int i = 0, n = 0;
    A_BLOB_RECT_GRP_DATA* pGrpRectData = &pCCU->strLSYSInfo.blobGrpRectData;
    A_BLOB_SINGLE_RECT_INFO* pSigRectData = &pCCU->strLSYSInfo.singleRectData;

    BOOL ret = TRUE;
    
    if(emSwapType == DATA_SWAP_AMI)
    {
        // 模块整体信息
        if (pCCU->strAllModuInfo.usModuRunningMode >= MAX_MODULE_RUNNING)
        {
            TRACED("----ERRRRRR: ModuRunningMode value is illegal, ModuRunningMode:[%d]----\n", 
                pCCU->strAllModuInfo.usModuRunningMode);
            ret = FALSE;
        }
        pGrpRectData->emModuleRunningMode = (MODULE_RUNNING_MODE)pCCU->strAllModuInfo.usModuRunningMode;

        // 模块手动/自动模式
        if (pCCU->strAllModuInfo.usCtrlMode >= MAX_MODULE_CTRL)
        {
            TRACED("----ERRRRRR: CtrlMode value is illegal, CtrlMode:[%d]----\n", pCCU->strAllModuInfo.usCtrlMode);
            ret = FALSE;
        }
        pGrpRectData->emManuAutoMode = (MODULE_CTRL_MODE)pCCU->strAllModuInfo.usCtrlMode;

        // 无法控制充放电原因
        if (pCCU->strAllModuInfo.usChgDischgFailReason >= MAX_CHG_DISCHG_CTRL)
        {
            TRACED("----ERRRRRR: ChgDischgFailReason value is illegal, ChgDischgFailReason:[%d]----\n",
                pCCU->strAllModuInfo.usChgDischgFailReason);
            ret = FALSE;
        }
        pGrpRectData->emChrgDischrgCtrlFailReason = (CHG_DIDCHG_CTRL_FAIL_REASON)pCCU->strAllModuInfo.usChgDischgFailReason;

        // 模块组配置信息
        // TRACED("----pCCU->strAllModuInfo.usModuGrp_CfgInfo:[%d]----\n", pCCU->strAllModuInfo.usModuGrp_CfgInfo);
        pGrpRectData->unioModuleCfgInfo.usModuleGrpCfg = pCCU->strAllModuInfo.usModuGrp_CfgInfo;
        // 模块组在线信息
        // TRACED("----pCCU->strAllModuInfo.usModuGrp_OnlineInfo:[%d]----\n", pCCU->strAllModuInfo.usModuGrp_OnlineInfo);
        pGrpRectData->unioModuleOnlineInfo.usModuleGrpOnline = pCCU->strAllModuInfo.usModuGrp_OnlineInfo;
    }
    else if(emSwapType == DATA_SWAP_ACDC_GRP)
    {
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)(nGrpID);

        // 组内在线模块数量
        UINT nOnLineNumber = (UINT)pCCU->strACDCGrpInfo.uiOnlineModuNum;
        if(nOnLineNumber >= MAX_SINGLE_RECT_NUM)
        {
            TRACED("nOnLineNumber=[%d]\n", nOnLineNumber);
            ret = FALSE;
        }
        pGrpRectData->nOnlineRectNum[emRectType] = (int)nOnLineNumber;
        pSigRectData->nOnlineNum[emRectType] = (UINT)nOnLineNumber;

        // 额定功率
        pGrpRectData->fRatedPower[emRectType] = ((float)pCCU->strACDCGrpInfo.uiRatedPower);

        // 额定电流
        pGrpRectData->fRatedCurr[emRectType] = ((float)pCCU->strACDCGrpInfo.uiRatedCurr);

        // 最大电流
        pGrpRectData->fRealTime_MaxCurr[emRectType] = ((float)pCCU->strACDCGrpInfo.uiMaxCurr);

        // 控制状态
        if(pCCU->strACDCGrpInfo.uiCtrlState >= MAX_CTRL_STATUS)
        {
            TRACED("uiCtrlState=[%d]\n", pCCU->strACDCGrpInfo.uiCtrlState);
            ret = FALSE;
        }
        pGrpRectData->emCtrlStatus[emRectType] = (CTRL_STATUS)(pCCU->strACDCGrpInfo.uiCtrlState);

        // 控制直流侧电压
        pGrpRectData->fCtrlVolt[emRectType] = ((float)pCCU->strACDCGrpInfo.uiCtrlDcVolt)*0.1;

        // 控制直流侧电流
        pGrpRectData->fCtrlCurr[emRectType] = ((float)pCCU->strACDCGrpInfo.uiCtrlDcCurr)*0.1;

        // 控制功率
        pGrpRectData->fCtrlPower[emRectType] = ((float)pCCU->strACDCGrpInfo.uiCtrlPower)*0.1;

        // 运行状态
        if(pCCU->strACDCGrpInfo.uiRunningState > RUNNING_STATUS_ACDC_INCONSISTENT)
        {
            TRACED("uiRunningState=[%d]\n", pCCU->strACDCGrpInfo.uiRunningState);
            ret = FALSE;
        }
        pGrpRectData->emRunningStatus[emRectType] = (RUNNING_STATUS)(pCCU->strACDCGrpInfo.uiRunningState);

        // 当前直流侧电压
        pGrpRectData->fRealTime_BattSideVolt[emRectType] = ((float)pCCU->strACDCGrpInfo.uiCurDcVolt);

        // 当前直流侧总电流
        pGrpRectData->fRealTime_BattSideTotalCurr[emRectType] = ((float)pCCU->strACDCGrpInfo.uiCurDcTotalCurr);

        // 当前系统功率
        pGrpRectData->fRealTimeSystemPower[emRectType] = ((float)pCCU->strACDCGrpInfo.uiCurSysPower);
    }
    else if(emSwapType == DATA_SWAP_DCDC_GRP)
    {
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)(nGrpID + 8);

        // 组内在线模块数量
        UINT nOnLineNumber = (UINT)pCCU->strDCDCGrpInfo.uiOnlineModuNum;
        if(nOnLineNumber >= MAX_SINGLE_RECT_NUM)
        {
            TRACED("nOnLineNumber=[%d]\n", nOnLineNumber);
            ret = FALSE;
        }
        pGrpRectData->nOnlineRectNum[emRectType] = (int)nOnLineNumber;
        pSigRectData->nOnlineNum[emRectType] = (UINT)nOnLineNumber;

        // 额定功率
        pGrpRectData->fRatedPower[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiRatedPower)*0.1;

        // 额定电流
        pGrpRectData->fRatedCurr[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiRatedCurr)*0.1;

        // 最大电流
        pGrpRectData->fRealTime_MaxCurr[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiMaxCurr)*0.1;

        // 控制状态
        if(pCCU->strDCDCGrpInfo.uiCtrlState >= MAX_CTRL_STATUS)
        {
            TRACED("uiCtrlState=[%d]\n", pCCU->strDCDCGrpInfo.uiCtrlState);
            ret = FALSE;
        }
        pGrpRectData->emCtrlStatus[emRectType] = (CTRL_STATUS)(pCCU->strDCDCGrpInfo.uiCtrlState);

        // 控制直流侧电压
        pGrpRectData->fCtrlVolt[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCtrlVolt)*0.1;

        // 控制电池侧放电截止电压
        pGrpRectData->fCtrlBattSideCutoffVolt[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCtrlBattDischgCutoffVolt)*0.1;

        // 控制电池侧限流值
        pGrpRectData->fCtrlBattSideCurrLmt[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCtrlBattCurrLmt)*0.1;

        // 控制母线侧限流值
        pGrpRectData->fCtrlBusSideCurrLmt[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCtrlBusCurrLmt)*0.1;

        // 运行状态
        pGrpRectData->emRunningStatus[emRectType] = (RUNNING_STATUS)(pCCU->strDCDCGrpInfo.uiRunningState);

        // 当前电池侧电压
        pGrpRectData->fRealTime_BattSideVolt[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCurBattVolt)*0.1;

        // 当前直流侧总电流
        pGrpRectData->fRealTime_BattSideTotalCurr[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCurBattTotalCurr)*0.1;

        // 当前母线侧电压
        pGrpRectData->fRealTime_BusSideVolt[emRectType] = ((float)pCCU->strDCDCGrpInfo.uiCurBusVolt)*0.1;
    }
    else if(emSwapType == DATA_SWAP_MPPT_GRP)
    {
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)(nGrpID + 4);

        // 组内在线模块数量
        UINT nOnLineNumber = (UINT)pCCU->strMPPTGrpInfo.uiOnlineModuNum;
        if(nOnLineNumber >= MAX_SINGLE_RECT_NUM)
        {
            TRACED("nOnLineNumber=[%d]\n", nOnLineNumber);
            ret = FALSE;
        }
        pGrpRectData->nOnlineRectNum[emRectType] = (int)nOnLineNumber;
        pSigRectData->nOnlineNum[emRectType] = (UINT)nOnLineNumber;

        // 额定功率
        pGrpRectData->fRatedPower[emRectType] = ((float)pCCU->strMPPTGrpInfo.uiRatedPower)*0.1;

        // 额定电流
        pGrpRectData->fRatedCurr[emRectType] = ((float)pCCU->strMPPTGrpInfo.uiRatedCurr)*0.1;

        // 最大电流
        pGrpRectData->fRealTime_MaxCurr[emRectType] = ((float)pCCU->strMPPTGrpInfo.uiMaxCurr)*0.1;

        // 运行状态
        pGrpRectData->emRunningStatus[emRectType] = (RUNNING_STATUS)(pCCU->strMPPTGrpInfo.uiRunningState);

        // 系统电压
        pGrpRectData->fSystemVolt[emRectType] = ((float)pCCU->strMPPTGrpInfo.uiSysVolt)*0.1;

        // 系统总电流
        pGrpRectData->fSystemTotalCurr[emRectType] = ((float)pCCU->strMPPTGrpInfo.uiSysTotalCurr)*0.1;
    }
    else if(emSwapType == DATA_SWAP_ACDC_Sig)
    {
        // USHORT nGrpID = pCCU->strACDCSigInfo.uiBelongGrpID;
        if((nGrpID < 0) || (nGrpID > 3))
        {
            TRACED("DATA_SWAP_ACDC_Sig Group number out of range, BelongGrpID:[%d]\n", nGrpID+1);
            return FALSE;
        }
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)(nGrpID);
        UINT nOnLineNumber = pGrpRectData->nOnlineRectNum[emRectType];
        UINT nModuID = (UINT)pCCU->strACDCSigInfo.uiModuID;

        if(nModuID > nOnLineNumber)
        {
            TRACED("The Module ID exceeds the online count, nModuID:[%d], nOnLineNumber:[%d].", nModuID, nOnLineNumber);
            return FALSE;
        }

        n = nModuID - 1;
        pSigRectData->arrRect[emRectType][n].bExist = TRUE;

        // 所属组号
        pSigRectData->arrRect[emRectType][n].nBelongGrp = (UINT)pCCU->strACDCSigInfo.uiBelongGrpID;

        // 单模块号
        pSigRectData->arrRect[emRectType][n].uModuID = (UINT)pCCU->strACDCSigInfo.uiModuID;

        // RAM状态信息
        pSigRectData->arrRect[emRectType][n].unioRAMStatus.uiInverterStatus = pCCU->strACDCSigInfo.uiRamStateInfo;

        // 逆变状态表1-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_1_1.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_1_1;

        // 逆变状态表1-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_1_2.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_1_2;

        // 逆变状态表2-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_2_1.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_2_1;

        // 逆变状态表2-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_2_2.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_2_2;

        // 逆变状态表3-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_3_1.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_3_1;

        // 逆变状态表3-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_3_2.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_3_2;

        // 逆变状态表4-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_4_1.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_4_1;

        // 逆变状态表4-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_4_2.uiInverterStatus = pCCU->strACDCSigInfo.uiInvState_4_2;

        // DC软件版本号
        pSigRectData->arrRect[emRectType][n].fRect_DCDCVer = (float)pCCU->strACDCSigInfo.uiDcSoftVer;

        // PFC软件版本号
        pSigRectData->arrRect[emRectType][n].fRect_PFCVer = (float)pCCU->strACDCSigInfo.uiPfcSoftVer;

        // 条码信息1
        pSigRectData->arrRect[emRectType][n].uiBarCode[0] = pCCU->strACDCSigInfo.uiBarcode_1;

        // 条码信息2
        pSigRectData->arrRect[emRectType][n].uiBarCode[1] = pCCU->strACDCSigInfo.uiBarcode_2;

        // 条码信息3
        pSigRectData->arrRect[emRectType][n].uiBarCode[2] = pCCU->strACDCSigInfo.uiBarcode_3;

        // 条码信息4
        pSigRectData->arrRect[emRectType][n].uiBarCode[3] = pCCU->strACDCSigInfo.uiBarcode_4;

        // 模块直流侧电压
        pSigRectData->arrRect[emRectType][n].fDCsideVoltage = ((float)pCCU->strACDCSigInfo.uiModuDcVolt);

        // 模块直流侧电流
        pSigRectData->arrRect[emRectType][n].fDCsideCurrent = ((float)pCCU->strACDCSigInfo.uiModuDcCurr);

        // 模块环境温度
        pSigRectData->arrRect[emRectType][n].fAmbit_Temp = ((float)pCCU->strACDCSigInfo.uiModuEnvTemp);

        // 交流AB线电压
        pSigRectData->arrRect[emRectType][n].fphase_LineVolt[0] = ((float)pCCU->strACDCSigInfo.uiAcLineVolt_AB);

        // 交流BC线电压
        pSigRectData->arrRect[emRectType][n].fphase_LineVolt[1] = ((float)pCCU->strACDCSigInfo.uiAcLineVolt_BC);

        // 交流CA线电压
        pSigRectData->arrRect[emRectType][n].fphase_LineVolt[2] = ((float)pCCU->strACDCSigInfo.uiAcLineVolt_CA);
        
        // 交流A相相电压
        pSigRectData->arrRect[emRectType][n].fPh_InputVolt[0] = ((float)pCCU->strACDCSigInfo.uiAcPhaseVolt_A);

        // 交流B相相电压
        pSigRectData->arrRect[emRectType][n].fPh_InputVolt[1] = ((float)pCCU->strACDCSigInfo.uiAcPhaseVolt_B);

        // 交流C相相电压
        pSigRectData->arrRect[emRectType][n].fPh_InputVolt[2] = ((float)pCCU->strACDCSigInfo.uiAcPhaseVolt_C);

        // 交流A相电流
        pSigRectData->arrRect[emRectType][n].fPh_InputCurr[0] = ((float)pCCU->strACDCSigInfo.uiAcPhaseCurr_A);

        // 交流B相电流
        pSigRectData->arrRect[emRectType][n].fPh_InputCurr[1] = ((float)pCCU->strACDCSigInfo.uiAcPhaseCurr_B);

        // 交流C相电流
        pSigRectData->arrRect[emRectType][n].fPh_InputCurr[2] = ((float)pCCU->strACDCSigInfo.uiAcPhaseCurr_C);

        // 交流频率
        pSigRectData->arrRect[emRectType][n].fAC_Freq = ((float)pCCU->strACDCSigInfo.uiAcFreq);

        // 总有功功率
        pSigRectData->arrRect[emRectType][n].fPtotalActive = ((float)pCCU->strACDCSigInfo.uiTotalActivePower);

        // A相有功功率
        pSigRectData->arrRect[emRectType][n].fPphase_Active[0] = ((float)pCCU->strACDCSigInfo.uiActivePower_A);

        // B相有功功率
        pSigRectData->arrRect[emRectType][n].fPphase_Active[1] = ((float)pCCU->strACDCSigInfo.uiActivePower_B);

        // C相有功功率
        pSigRectData->arrRect[emRectType][n].fPphase_Active[2] = ((float)pCCU->strACDCSigInfo.uiActivePower_C);

        // 总无功功率
        pSigRectData->arrRect[emRectType][n].fPtotalReactive = ((float)pCCU->strACDCSigInfo.uiTotalReactivePower);

        // A相无功功率
        pSigRectData->arrRect[emRectType][n].fPphase_Reactive[0] = ((float)pCCU->strACDCSigInfo.uiReactivePower_A);

        // B相无功功率
        pSigRectData->arrRect[emRectType][n].fPphase_Reactive[1] = ((float)pCCU->strACDCSigInfo.uiReactivePower_B);

        // C相无功功率
        pSigRectData->arrRect[emRectType][n].fPphase_Reactive[2] = ((float)pCCU->strACDCSigInfo.uiReactivePower_C);

        // 总视在功率
        pSigRectData->arrRect[emRectType][n].fPTotalApparent = ((float)pCCU->strACDCSigInfo.uiTotalApparentPower);

        // A相视在功率
        pSigRectData->arrRect[emRectType][n].fPphase_Apparent[0] = ((float)pCCU->strACDCSigInfo.uiApparentPower_A);

        // B相视在功率
        pSigRectData->arrRect[emRectType][n].fPphase_Apparent[1] = ((float)pCCU->strACDCSigInfo.uiApparentPower_B);

        // C相视在功率
        pSigRectData->arrRect[emRectType][n].fPphase_Apparent[2] = ((float)pCCU->strACDCSigInfo.uiApparentPower_C);

        // 总功率因数
        pSigRectData->arrRect[emRectType][n].fPtotalFactor = ((float)pCCU->strACDCSigInfo.uiTotalPowerFactor)*0.001;

        // A相功率因数
        pSigRectData->arrRect[emRectType][n].fPphase_Factor[0] = ((float)pCCU->strACDCSigInfo.uiPowerFactor_A)*0.001;

        // B相功率因数
        pSigRectData->arrRect[emRectType][n].fPphase_Factor[1] = ((float)pCCU->strACDCSigInfo.uiPowerFactor_B)*0.001;

        // C相功率因数
        pSigRectData->arrRect[emRectType][n].fPphase_Factor[2] = ((float)pCCU->strACDCSigInfo.uiPowerFactor_C)*0.001;

        /* Map 旧数据复用 逆变状态表1-1 1-2*/
        Map_InverterStatus_ToPCSStatus(&pSigRectData->arrRect[emRectType][n]);
    }
    else if(emSwapType == DATA_SWAP_DCDC_Sig)
    {
        // USHORT nGrpID = pCCU->strDCDCSigInfo.uiBelongGrpID;
        if((nGrpID < 0) || (nGrpID > 3))
        {
            TRACED("DATA_SWAP_DCDC_Sig Group number out of range, BelongGrpID:[%d]\n", nGrpID+1);
            return FALSE;
        }
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)(nGrpID + 8);
        UINT nOnLineNumber = pGrpRectData->nOnlineRectNum[emRectType];
        UINT nModuID = (UINT)pCCU->strDCDCSigInfo.uiModuID;

        if(nModuID > nOnLineNumber)
        {
            TRACED("The Module ID exceeds the online count, nModuID:[%d], nOnLineNumber:[%d].", nModuID, nOnLineNumber);
            return FALSE;
        }

        n = nModuID - 1;
        pSigRectData->arrRect[emRectType][n].bExist = TRUE;

        // 所属组号
        pSigRectData->arrRect[emRectType][n].nBelongGrp = (UINT)pCCU->strDCDCSigInfo.uiBelongGrpID;

        // 单模块号
        pSigRectData->arrRect[emRectType][n].uModuID = (UINT)pCCU->strDCDCSigInfo.uiModuID;

        // 逆变状态表1-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_1_1.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_1_1;

        // 逆变状态表1-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_1_2.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_1_2;

        // 逆变状态表2-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_2_1.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_2_1;

        // 逆变状态表2-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_2_2.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_2_2;

        // 逆变状态表3-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_3_1.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_3_1;

        // 逆变状态表3-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_3_2.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_3_2;

        // 逆变状态表4-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_4_1.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_4_1;

        // 逆变状态表4-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_4_2.uiInverterStatus = pCCU->strDCDCSigInfo.uiInvState_4_2;

        // DC软件版本号
        pSigRectData->arrRect[emRectType][n].fRect_DCDCVer = (float)pCCU->strDCDCSigInfo.uiDcSoftVer;

        // PFC软件版本号
        pSigRectData->arrRect[emRectType][n].fRect_PFCVer = (float)pCCU->strDCDCSigInfo.uiPfcSoftVer;

        // 条码信息1
        pSigRectData->arrRect[emRectType][n].uiBarCode[0] = pCCU->strDCDCSigInfo.uiBarcode_1;

        // 条码信息2
        pSigRectData->arrRect[emRectType][n].uiBarCode[1] = pCCU->strDCDCSigInfo.uiBarcode_2;

        // 条码信息3
        pSigRectData->arrRect[emRectType][n].uiBarCode[2] = pCCU->strDCDCSigInfo.uiBarcode_3;

        // 条码信息4
        pSigRectData->arrRect[emRectType][n].uiBarCode[3] = pCCU->strDCDCSigInfo.uiBarcode_4;

        // 模块电池侧电压
        pSigRectData->arrRect[emRectType][n].fModuBattSideVolt = ((float)pCCU->strDCDCSigInfo.uiModuBattVolt)*0.1;

        // 模块电池侧电流
        pSigRectData->arrRect[emRectType][n].fModuBattSideCurr = ((float)pCCU->strDCDCSigInfo.uiModuBattCurr)*0.1;

        // 模块母线侧电压
        pSigRectData->arrRect[emRectType][n].fModuBusSideVolt = ((float)pCCU->strDCDCSigInfo.uiModuBusVolt*0.1);

        // 模块母线侧电流
        pSigRectData->arrRect[emRectType][n].fModuBusSideCurr = ((float)pCCU->strDCDCSigInfo.uiModuBusCurr*0.1);

        // 模块环境温度
        pSigRectData->arrRect[emRectType][n].fAmbit_Temp = ((float)pCCU->strDCDCSigInfo.uiModuEnvTemp*0.1);
    }
    else if(emSwapType == DATA_SWAP_MPPT_Sig)
    {
        // USHORT nGrpID = pCCU->strMPPTSigInfo.uiBelongGrpID;
        if((nGrpID < 0) || (nGrpID > 3))
        {
            TRACED("DATA_SWAP_MPPT_Sig Group number out of range, BelongGrpID:[%d]\n", nGrpID+1);
            return FALSE;
        }
        TYPE_SIGNLE_RECT emRectType = (TYPE_SIGNLE_RECT)(nGrpID + 4);
        UINT nOnLineNumber = pGrpRectData->nOnlineRectNum[emRectType];
        UINT nModuID = (UINT)pCCU->strMPPTSigInfo.uiModuID;

        if(nModuID > nOnLineNumber)
        {
            TRACED("The Module ID exceeds the online count, nModuID:[%d], nOnLineNumber:[%d].", nModuID, nOnLineNumber);
            return FALSE;
        }

        n = nModuID - 1;
        pSigRectData->arrRect[emRectType][n].bExist = TRUE;

        // 所属组号
        pSigRectData->arrRect[emRectType][n].nBelongGrp = (UINT)pCCU->strMPPTSigInfo.uiBelongGrpID;

        // 单模块号
        pSigRectData->arrRect[emRectType][n].uModuID = (UINT)pCCU->strMPPTSigInfo.uiModuID;

        // 逆变状态表1-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_1_1.uiInverterStatus = pCCU->strMPPTSigInfo.uiInvState_1_1;

        // 逆变状态表1-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_1_2.uiInverterStatus = pCCU->strMPPTSigInfo.uiInvState_1_2;

        // 逆变状态表2-1
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_2_1.uiInverterStatus = pCCU->strMPPTSigInfo.uiInvState_2_1;

        // 逆变状态表2-2
        pSigRectData->arrRect[emRectType][n].unioInverterStatus_2_2.uiInverterStatus = pCCU->strMPPTSigInfo.uiInvState_2_2;

        // DC软件版本号
        pSigRectData->arrRect[emRectType][n].fRect_DCDCVer = (float)pCCU->strMPPTSigInfo.uiDcSoftVer;

        // PFC软件版本号
        pSigRectData->arrRect[emRectType][n].fRect_PFCVer = (float)pCCU->strMPPTSigInfo.uiPfcSoftVer;

        // 条码信息1
        pSigRectData->arrRect[emRectType][n].uiBarCode[0] = pCCU->strMPPTSigInfo.uiBarcode_1;

        // 条码信息2
        pSigRectData->arrRect[emRectType][n].uiBarCode[1] = pCCU->strMPPTSigInfo.uiBarcode_2;

        // 条码信息3
        pSigRectData->arrRect[emRectType][n].uiBarCode[2] = pCCU->strMPPTSigInfo.uiBarcode_3;

        // 条码信息4
        pSigRectData->arrRect[emRectType][n].uiBarCode[3] = pCCU->strMPPTSigInfo.uiBarcode_4;

        // 模块电压
        pSigRectData->arrRect[emRectType][n].fModuVolt = ((float)pCCU->strMPPTSigInfo.uiModuVolt)*0.1;

        // 模块电流
        pSigRectData->arrRect[emRectType][n].fModuCurr = ((float)pCCU->strMPPTSigInfo.uiModuCurr)*0.1;

        // 模块环境温度
        pSigRectData->arrRect[emRectType][n].fAmbit_Temp = ((float)pCCU->strMPPTSigInfo.uiModuEnvTemp)*0.1;  
    }

    // When All Online PM is 0,Init Single PM Blob
	int nTotalOnLinePM = 0;
	for(n = 0; n < MAX_RECT_TYPE; n++)
	{
		nTotalOnLinePM += pCCU->strLSYSInfo.blobGrpRectData.nOnlineRectNum[n];
	}

	if(nTotalOnLinePM == 0)
	{
		memset(&pCCU->strLSYSInfo.singleRectData, 0 , sizeof(A_BLOB_SINGLE_RECT_INFO));
	}

    return ret;
}

static const RPMSG_PARSER_RULE s_AllModulesDict[] = {
    // 0. Module Running Mode
    MAP_USHORT(0, ROUGH_CCU_DATA, strAllModuInfo, usModuRunningMode),
    // 1. Control Mode
    MAP_USHORT(1, ROUGH_CCU_DATA, strAllModuInfo, usCtrlMode),
    // 2. Charge or Discharge Failed Reason
    MAP_USHORT(2, ROUGH_CCU_DATA, strAllModuInfo, usChgDischgFailReason),
    // 3. Module Group Config Information
    MAP_USHORT(3, ROUGH_CCU_DATA, strAllModuInfo, usModuGrp_CfgInfo),
    // 4. Module Group Online Infomation
    MAP_USHORT(4, ROUGH_CCU_DATA, strAllModuInfo, usModuGrp_OnlineInfo)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  Parse_AllModulesInfo
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/07 17:26:38
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_AllModulesInfo(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing all modules information------------:\n");
    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_AllModulesDict, ITEM_OF(s_AllModulesDict)))
    {
        TRACED("All modules infomation parser failed!\n");
        return FALSE;
    }

    // Data Swap
    
    BOOL ret = Data_Swap(pCCU, DATA_SWAP_AMI, 0);

    return ret;
}

static const RPMSG_PARSER_RULE s_ACDCGrpDict[] = {
    // 0. Inner Group Online Module Number
    MAP_UINT(0, ROUGH_CCU_DATA, strACDCGrpInfo, uiOnlineModuNum),
    // 1. Rated Power
    MAP_UINT(1, ROUGH_CCU_DATA, strACDCGrpInfo, uiRatedPower),
    // 2. Rated Current
    MAP_UINT(2, ROUGH_CCU_DATA, strACDCGrpInfo, uiRatedCurr),
    // 3. Max Current
    MAP_UINT(3, ROUGH_CCU_DATA, strACDCGrpInfo, uiMaxCurr),
    // 4. Control Status
    MAP_UINT(4, ROUGH_CCU_DATA, strACDCGrpInfo, uiCtrlState),
    // 5. Control DC Side Voltage
    MAP_UINT(5, ROUGH_CCU_DATA, strACDCGrpInfo, uiCtrlDcVolt),
    // 6. Control DC Side Current
    MAP_UINT(6, ROUGH_CCU_DATA, strACDCGrpInfo, uiCtrlDcCurr),
    // 7. Control Power
    MAP_UINT(7, ROUGH_CCU_DATA, strACDCGrpInfo, uiCtrlPower),
    // 8. Running Status
    MAP_UINT(8, ROUGH_CCU_DATA, strACDCGrpInfo, uiRunningState),
    // 9. Current DC Side Voltage
    MAP_UINT(9, ROUGH_CCU_DATA, strACDCGrpInfo, uiCurDcVolt),
    // 10. Current DC Side Total Current
    MAP_UINT(10, ROUGH_CCU_DATA, strACDCGrpInfo, uiCurDcTotalCurr),
    // 11. Current System Power
    MAP_UINT(11, ROUGH_CCU_DATA, strACDCGrpInfo, uiCurSysPower)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/07 17:54:55
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_ACDC_Grp_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing ACDC group information------------:\n");
    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_ACDCGrpDict, ITEM_OF(s_ACDCGrpDict)))
    {
        return FALSE;
    }

    USHORT nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);

    BOOL ret = Data_Swap(pCCU, DATA_SWAP_ACDC_GRP, nGroupIndex);

    return ret;
}


static const RPMSG_PARSER_RULE s_DCDCGrpDict[] = {
    // 0. Inner Group Online Module Number
    MAP_UINT(0, ROUGH_CCU_DATA, strDCDCGrpInfo, uiOnlineModuNum),
    // 1. Rated Power
    MAP_UINT(1, ROUGH_CCU_DATA, strDCDCGrpInfo, uiRatedPower),
    // 2. Rated Current
    MAP_UINT(2, ROUGH_CCU_DATA, strDCDCGrpInfo, uiRatedCurr),
    // 3. Max Current
    MAP_UINT(3, ROUGH_CCU_DATA, strDCDCGrpInfo, uiMaxCurr),
    // 4. Control Status
    MAP_UINT(4, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCtrlState),
    // 5. Control Voltage
    MAP_UINT(5, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCtrlVolt),
    // 6. Control Battery Side Discharge Cutoff Voltage
    MAP_UINT(6, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCtrlBattDischgCutoffVolt),
    // 7. Control Battery Side Limit Current
    MAP_UINT(7, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCtrlBattCurrLmt),
    // 8. Control Bus Side Limit Current
    MAP_UINT(8, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCtrlBusCurrLmt),
    // 9. Running Status
    MAP_UINT(9, ROUGH_CCU_DATA, strDCDCGrpInfo, uiRunningState),
    // 10. Current Battery Side Voltage
    MAP_UINT(10, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCurBattVolt),
    // 11. Current Battery Side Total Current
    MAP_UINT(11, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCurBattTotalCurr),
    // 12. Current Bus Side Voltage
    MAP_UINT(12, ROUGH_CCU_DATA, strDCDCGrpInfo, uiCurBusVolt)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/09 11:55:56
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_DCDC_Grp_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing DCDC group information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_DCDCGrpDict, ITEM_OF(s_DCDCGrpDict)))
    {
        return FALSE;
    }

    USHORT nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);

    BOOL ret = Data_Swap(pCCU, DATA_SWAP_DCDC_GRP, nGroupIndex);

    return ret;
}

static const RPMSG_PARSER_RULE s_MPPTGrpDict[] = {
    // 0. Inner Group Online Module Number
    MAP_UINT(0, ROUGH_CCU_DATA, strMPPTGrpInfo, uiOnlineModuNum),
    // 1. Rated Power
    MAP_UINT(1, ROUGH_CCU_DATA, strMPPTGrpInfo, uiRatedPower),
    // 2. Rated Current
    MAP_UINT(2, ROUGH_CCU_DATA, strMPPTGrpInfo, uiRatedCurr),
    // 3. Max Current
    MAP_UINT(3, ROUGH_CCU_DATA, strMPPTGrpInfo, uiMaxCurr),
    // 4. Running Status
    MAP_UINT(4, ROUGH_CCU_DATA, strMPPTGrpInfo, uiRunningState),
    // 5. System Voltage
    MAP_UINT(5, ROUGH_CCU_DATA, strMPPTGrpInfo, uiSysVolt),
    // 6. System Total Current
    MAP_UINT(6, ROUGH_CCU_DATA, strMPPTGrpInfo, uiSysTotalCurr)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/09 12:03:17
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_MPPT_Grp_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT group information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_MPPTGrpDict, ITEM_OF(s_MPPTGrpDict)))
    {
        return FALSE;
    }

    USHORT nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);
    
    BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_GRP, nGroupIndex);

    return ret;
}

static const RPMSG_PARSER_RULE s_ACDCSigDict[] = { 
    // 0. Belong Group Id
    MAP_UINT(0, ROUGH_CCU_DATA, strACDCSigInfo, uiBelongGrpID),
    // 1. Single Module ID
    MAP_UINT(1, ROUGH_CCU_DATA, strACDCSigInfo, uiModuID),
    // 2. RAM Status Info
    MAP_UINT(2, ROUGH_CCU_DATA, strACDCSigInfo, uiRamStateInfo),
    // 3. Inverter State Table 1-1
    MAP_UINT(3, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_1_1),
    // 4. Inverter State Table 1-2
    MAP_UINT(4, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_1_2),
    // 5. Inverter State Table 2-1
    MAP_UINT(5, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_2_1),
    // 6. Inverter State Table 2-2
    MAP_UINT(6, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_2_2),
    // 7. Inverter State Table 3-1
    MAP_UINT(7, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_3_1),
    // 8. Inverter State Table 3-2
    MAP_UINT(8, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_3_2),
    // 9. Inverter State Table 4-1
    MAP_UINT(9, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_4_1),
    // 10. Inverter State Table 4-2
    MAP_UINT(10, ROUGH_CCU_DATA, strACDCSigInfo, uiInvState_4_2),
    // 11. DC SW Version
    MAP_UINT(11, ROUGH_CCU_DATA, strACDCSigInfo, uiDcSoftVer),
    // 12. PFC SW Version
    MAP_UINT(12, ROUGH_CCU_DATA, strACDCSigInfo, uiPfcSoftVer),
    // 13. Barcode Information 1
    MAP_UINT(13, ROUGH_CCU_DATA, strACDCSigInfo, uiBarcode_1),
    // 14. Barcode Information 2
    MAP_UINT(14, ROUGH_CCU_DATA, strACDCSigInfo, uiBarcode_2),
    // 15. Barcode Information 3
    MAP_UINT(15, ROUGH_CCU_DATA, strACDCSigInfo, uiBarcode_3),
    // 16. Barcode Information 4
    MAP_UINT(16, ROUGH_CCU_DATA, strACDCSigInfo, uiBarcode_4),
    // 17. Module DC Side Voltage
    MAP_UINT(17, ROUGH_CCU_DATA, strACDCSigInfo, uiModuDcVolt),
    // 18. Module DC Side Current
    MAP_UINT(18, ROUGH_CCU_DATA, strACDCSigInfo, uiModuDcCurr),
    // 19. Module Ambient Temperature
    MAP_UINT(19, ROUGH_CCU_DATA, strACDCSigInfo, uiModuEnvTemp),
    // 20. AC Phase AB Line Voltage
    MAP_UINT(20, ROUGH_CCU_DATA, strACDCSigInfo, uiAcLineVolt_AB),
    // 21. AC Phase BC Line Voltage
    MAP_UINT(21, ROUGH_CCU_DATA, strACDCSigInfo, uiAcLineVolt_BC),
    // 22. AC Phase CA Line Voltage
    MAP_UINT(22, ROUGH_CCU_DATA, strACDCSigInfo, uiAcLineVolt_CA),
    // 23. AC Phase A Voltage
    MAP_UINT(23, ROUGH_CCU_DATA, strACDCSigInfo, uiAcPhaseVolt_A),
    // 24. AC Phase B Voltage
    MAP_UINT(24, ROUGH_CCU_DATA, strACDCSigInfo, uiAcPhaseVolt_B),
    // 25. AC Phase C Voltage
    MAP_UINT(25, ROUGH_CCU_DATA, strACDCSigInfo, uiAcPhaseVolt_C),
    // 26. AC Phase A Current
    MAP_UINT(26, ROUGH_CCU_DATA, strACDCSigInfo, uiAcPhaseCurr_A),
    // 27. AC Phase B Current
    MAP_UINT(27, ROUGH_CCU_DATA, strACDCSigInfo, uiAcPhaseCurr_B),
    // 28. AC Phase C Current
    MAP_UINT(28, ROUGH_CCU_DATA, strACDCSigInfo, uiAcPhaseCurr_C),
    // 29. AC Frequency
    MAP_UINT(29, ROUGH_CCU_DATA, strACDCSigInfo, uiAcFreq),
    // 30. Total ACtive Power
    MAP_UINT(30, ROUGH_CCU_DATA, strACDCSigInfo, uiTotalActivePower),
    // 31. Phase A Active Power
    MAP_UINT(31, ROUGH_CCU_DATA, strACDCSigInfo, uiActivePower_A),
    // 32. Phase B Active Power
    MAP_UINT(32, ROUGH_CCU_DATA, strACDCSigInfo, uiActivePower_B),
    // 33. Phase C Active Power
    MAP_UINT(33, ROUGH_CCU_DATA, strACDCSigInfo, uiActivePower_C),
    // 34. Total Reactive Power
    MAP_UINT(34, ROUGH_CCU_DATA, strACDCSigInfo, uiTotalReactivePower),
    // 35. Phase A Reactive Power
    MAP_UINT(35, ROUGH_CCU_DATA, strACDCSigInfo, uiReactivePower_A),
    // 36. Phase B Reactive Power
    MAP_UINT(36, ROUGH_CCU_DATA, strACDCSigInfo, uiReactivePower_B),
    // 37. Phase C Reactive Power
    MAP_UINT(37, ROUGH_CCU_DATA, strACDCSigInfo, uiReactivePower_C),
    // 38. Total Apparent Power
    MAP_UINT(38, ROUGH_CCU_DATA, strACDCSigInfo, uiTotalApparentPower),
    // 39. Phase A Apparent Power
    MAP_UINT(39, ROUGH_CCU_DATA, strACDCSigInfo, uiApparentPower_A),
    // 40. Phase B Apparent Power
    MAP_UINT(40, ROUGH_CCU_DATA, strACDCSigInfo, uiApparentPower_B),
    // 41. Phase C Apparent Power
    MAP_UINT(41, ROUGH_CCU_DATA, strACDCSigInfo, uiApparentPower_C),
    // 42. Total Power Factor
    MAP_UINT(42, ROUGH_CCU_DATA, strACDCSigInfo, uiTotalPowerFactor),
    // 43. Phase A Power Factor
    MAP_UINT(43, ROUGH_CCU_DATA, strACDCSigInfo, uiPowerFactor_A),
    // 44. Phase B Power Factor
    MAP_UINT(44, ROUGH_CCU_DATA, strACDCSigInfo, uiPowerFactor_B),
    // 45. Phase C Power Factor
    MAP_UINT(45, ROUGH_CCU_DATA, strACDCSigInfo, uiPowerFactor_C)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/09 12:08:03
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_ACDC_Sig_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing ACDC single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_ACDCSigDict, ITEM_OF(s_ACDCSigDict)))
    {
        return FALSE;
    }

    USHORT nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);

    BOOL ret = Data_Swap(pCCU, DATA_SWAP_ACDC_Sig, nGroupIndex);

    return ret;
}

static const RPMSG_PARSER_RULE s_DCDCSigDict[] = {
    // 0. Belong Group ID
    MAP_UINT(0, ROUGH_CCU_DATA, strDCDCSigInfo, uiBelongGrpID),
    // 1. Single Module ID
    MAP_UINT(1, ROUGH_CCU_DATA, strDCDCSigInfo, uiModuID),
    // 2. Inverter State Table 1-1
    MAP_UINT(2, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_1_1),
    // 3. Inverter State Table 1-2
    MAP_UINT(3, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_1_2),
    // 4. Inverter State Table 2-1
    MAP_UINT(4, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_2_1),
    // 5. Inverter State Table 2-2
    MAP_UINT(5, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_2_2),
    // 6. Inverter State Table 3-1
    MAP_UINT(6, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_3_1),
    // 7. Inverter State Table 3-2
    MAP_UINT(7, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_3_2),
    // 8. Inverter State Table 4-1
    MAP_UINT(8, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_4_1),
    // 9. Inverter State Table 4-2
    MAP_UINT(9, ROUGH_CCU_DATA, strDCDCSigInfo, uiInvState_4_2),
    // 10. DC SW Version
    MAP_UINT(10, ROUGH_CCU_DATA, strDCDCSigInfo, uiDcSoftVer),
    // 11. PFC SW Version
    MAP_UINT(11, ROUGH_CCU_DATA, strDCDCSigInfo, uiPfcSoftVer),
    // 12. Barcode Information 1
    MAP_UINT(12, ROUGH_CCU_DATA, strDCDCSigInfo, uiBarcode_1),
    // 13. Barcode Information 2
    MAP_UINT(13, ROUGH_CCU_DATA, strDCDCSigInfo, uiBarcode_2),
    // 14. Barcode Information 3
    MAP_UINT(14, ROUGH_CCU_DATA, strDCDCSigInfo, uiBarcode_3),
    // 15. Barcode Information 4
    MAP_UINT(15, ROUGH_CCU_DATA, strDCDCSigInfo, uiBarcode_4),
    // 16. Module Battery Side Voltage
    MAP_UINT(16, ROUGH_CCU_DATA, strDCDCSigInfo, uiModuBattVolt),
    // 17. Module Battery Side Current
    MAP_UINT(17, ROUGH_CCU_DATA, strDCDCSigInfo, uiModuBattCurr),
    // 18. Module Bus Side Voltage
    MAP_UINT(18, ROUGH_CCU_DATA, strDCDCSigInfo, uiModuBusVolt),
    // 19. Module Bus Side Current
    MAP_UINT(19, ROUGH_CCU_DATA, strDCDCSigInfo, uiModuBusCurr),
    // 20. Module Ambient Temperature
    MAP_UINT(20, ROUGH_CCU_DATA, strDCDCSigInfo, uiModuEnvTemp)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/09 14:08:20
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_DCDC_Sig_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing DCDC single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_DCDCSigDict, ITEM_OF(s_DCDCSigDict)))
    {
        return FALSE;
    }

    USHORT nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);

    BOOL ret = Data_Swap(pCCU, DATA_SWAP_DCDC_Sig, nGroupIndex);

    return ret;
}

static const RPMSG_PARSER_RULE s_MPPTSigDict[] = {
    // 0. Belong Group ID
    MAP_UINT(0, ROUGH_CCU_DATA, strMPPTSigInfo, uiBelongGrpID),
    // 1. Single Module ID
    MAP_UINT(1, ROUGH_CCU_DATA, strMPPTSigInfo, uiModuID),
    // 2. Inverter State Table 1-1
    MAP_UINT(2, ROUGH_CCU_DATA, strMPPTSigInfo, uiInvState_1_1),
    // 3. Inverter State Table 1-2
    MAP_UINT(3, ROUGH_CCU_DATA, strMPPTSigInfo, uiInvState_1_2),
    // 4. Inverter State Table 2-1
    MAP_UINT(4, ROUGH_CCU_DATA, strMPPTSigInfo, uiInvState_2_1),
    // 5. Inverter State Table 2-2
    MAP_UINT(5, ROUGH_CCU_DATA, strMPPTSigInfo, uiInvState_2_2),
    // 6. DC SW Verison
    MAP_UINT(6, ROUGH_CCU_DATA, strMPPTSigInfo, uiDcSoftVer),
    // 7. PFC SW Version
    MAP_UINT(7, ROUGH_CCU_DATA, strMPPTSigInfo, uiPfcSoftVer),
    // 8. Barcode Information 1
    MAP_UINT(8, ROUGH_CCU_DATA, strMPPTSigInfo, uiBarcode_1),
    // 9. Barcode Information 2
    MAP_UINT(9, ROUGH_CCU_DATA, strMPPTSigInfo, uiBarcode_2),
    // 10. Barcode Information 3
    MAP_UINT(10, ROUGH_CCU_DATA, strMPPTSigInfo, uiBarcode_3),
    // 11. Barcode Information 4
    MAP_UINT(11, ROUGH_CCU_DATA, strMPPTSigInfo, uiBarcode_4),
    // 12. Module Voltage
    MAP_UINT(12, ROUGH_CCU_DATA, strMPPTSigInfo, uiModuVolt),
    // 13. Module Current
    MAP_UINT(13, ROUGH_CCU_DATA, strMPPTSigInfo, uiModuCurr),
    // 14. Module Ambient Temperature
    MAP_UINT(14, ROUGH_CCU_DATA, strMPPTSigInfo, uiModuEnvTemp)
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/09 14:15:24
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_MPPT_Sig_Info(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_MPPTSigDict, ITEM_OF(s_MPPTSigDict)))
    {
        return FALSE;
    }

    USHORT nGroupIndex = GetGroupIndexByMainIndex(pMsgFrame->ID_MainIndex);

    BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig, nGroupIndex);

    return ret;
}

static const RPMSG_PARSER_RULE s_ModuleTypeConfigDict[] = {
    // 0. ACDC Module
    MAP_BYTE(0, ROUGH_CCU_DATA, strModuleTypeCfg, byACDC_Module),
    // 1. DCDC Module
    MAP_BYTE(1, ROUGH_CCU_DATA, strModuleTypeCfg, byDCDC_Module),
    // 2. MPPT Module
    MAP_BYTE(2, ROUGH_CCU_DATA, strModuleTypeCfg, byMPPT_Module),
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/12 15:17:32
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_Module_Type_Cfg(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_ModuleTypeConfigDict, ITEM_OF(s_ModuleTypeConfigDict)))
    {
        return FALSE;
    }

    // BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig);

    return 0;
}

static const RPMSG_PARSER_RULE s_ModuleNumCfgDict[] = {
    // 0. ACDC Group 1 single Number
    MAP_BYTE(0, ROUGH_CCU_DATA, strModuleNumCfg, byACDC_GRP1_SigNum),
    // 1. ACDC Group 2 single Number
    MAP_BYTE(1, ROUGH_CCU_DATA, strModuleNumCfg, byACDC_GRP2_SigNum),
    // 2. DCDC Group 1 single Number
    MAP_BYTE(2, ROUGH_CCU_DATA, strModuleNumCfg, byDCDC_GRP1_SigNum),
    // 3. DCDC Group 2 single Number
    MAP_BYTE(3, ROUGH_CCU_DATA, strModuleNumCfg, byDCDC_GRP2_SigNum),
    // 4. MPPT Group 1 single Number
    MAP_BYTE(4, ROUGH_CCU_DATA, strModuleNumCfg, byMPPT_GRP1_SigNum),
    // 5. MPPT Group 2 single Number
    MAP_BYTE(5, ROUGH_CCU_DATA, strModuleNumCfg, byMPPT_GRP2_SigNum),
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/12 15:17:32
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_Module_Num_Cfg(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_ModuleNumCfgDict, ITEM_OF(s_ModuleNumCfgDict)))
    {
        return FALSE;
    }

    // BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig);

    return 0;
}

static const RPMSG_PARSER_RULE s_ACDC_ChrgDisChrgCtrlDict[] = {
    // 0. Charge Discharge Mode
    MAP_USHORT(0, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usChrg_DisChrg_Mode),
    // 1. Control Module Group Id
    MAP_USHORT(1, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usCtrl_Module_GrpID),
    // 2. DC Side Control Voltage
    MAP_USHORT(2, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usDC_Side_Ctrl_Vlot),
    // 3. DC Side Control Current
    MAP_USHORT(3, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usDC_Side_Ctrl_Curr),
    // 4. AC Side Control Power
    MAP_USHORT(4, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usAC_Side_Ctrl_Power),
    // 5. Scheduling Mode
    MAP_USHORT(5, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usScheduling_Mode),
    // 6. Reactive Power
    MAP_USHORT(6, ROUGH_CCU_DATA, strACDC_ChrgDisChrgCtrl, usReactive_Power),
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/12 15:17:32
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_ACDC_ChrgDischarge_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_ACDC_ChrgDisChrgCtrlDict, ITEM_OF(s_ACDC_ChrgDisChrgCtrlDict)))
    {
        return FALSE;
    }

    // BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig);

    return 0;
}

static const RPMSG_PARSER_RULE s_DCDC_ChrgDisChrgCtrlDict[] = {
    // 0. Charge Discharge Mode
    MAP_USHORT(0, ROUGH_CCU_DATA, strDCDC_ChrgDisChrgCtrl, usChrg_DisChrg_Mode),
    // 1. Control Module Group Id
    MAP_USHORT(1, ROUGH_CCU_DATA, strDCDC_ChrgDisChrgCtrl, usCtrl_Module_GrpID),
    // 2. Ouput Voltage
    MAP_USHORT(2, ROUGH_CCU_DATA, strDCDC_ChrgDisChrgCtrl, usOutPut_Vlot),
    // 3. Battery Discharge Cut-Off Voltage
    MAP_USHORT(3, ROUGH_CCU_DATA, strDCDC_ChrgDisChrgCtrl, usBattDisChrg_CutoffVolt),
    // 4. Battery Side Current Limit
    MAP_USHORT(4, ROUGH_CCU_DATA, strDCDC_ChrgDisChrgCtrl, usBattSide_CurrLmt),
    // 5. Bus Side Current Limit
    MAP_USHORT(5, ROUGH_CCU_DATA, strDCDC_ChrgDisChrgCtrl, usBusSide_CurrLmt),
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/12 15:17:32
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_DCDC_ChrgDischarge_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_DCDC_ChrgDisChrgCtrlDict, ITEM_OF(s_DCDC_ChrgDisChrgCtrlDict)))
    {
        return FALSE;
    }

    // BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig);

    return 0;
}

static const RPMSG_PARSER_RULE s_ManulAutoCtrlDict[] = {
    // 0. Manul Auto Mode
    MAP_BYTE(0, ROUGH_CCU_DATA, strManulAutoCtrl, byManulAutoMode),
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/12 15:17:32
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_Manul_Auto_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_ManulAutoCtrlDict, ITEM_OF(s_ManulAutoCtrlDict)))
    {
        return FALSE;
    }

    // BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig);

    return 0;
}

static const RPMSG_PARSER_RULE s_N2002_OnOffGridCtrlDict[] = {
    // 0. ABU Enable
    MAP_BYTE(0, ROUGH_CCU_DATA, strN2002_OnOffGridCtrl, byABU_Enable),
    // 1. On Off Grid Auto Switch Mode
    MAP_BYTE(1, ROUGH_CCU_DATA, strN2002_OnOffGridCtrl, byOnOffGridAutoSwitchMode),
    // 2. On Off Grid Mode
    MAP_BYTE(2, ROUGH_CCU_DATA, strN2002_OnOffGridCtrl, byOnOffGridMode),
    // 3. ABU_On Off
    MAP_BYTE(3, ROUGH_CCU_DATA, strN2002_OnOffGridCtrl, byABU_OnOff),
};
/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/12 15:17:32
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
int Parse_N2002_OnOffGrid_Ctrl(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    // TRACED("------------now beging parsing MPPT single information------------:\n");

    if (!Parse_By_Dictionary(pCCU, pMsgFrame, s_N2002_OnOffGridCtrlDict, ITEM_OF(s_N2002_OnOffGridCtrlDict)))
    {
        return FALSE;
    }

    // BOOL ret = Data_Swap(pCCU, DATA_SWAP_MPPT_Sig);

    return 0;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/06/16
*  DESCRIPTION   :  ACDC 参数设置 9 个子索引
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static const RPMSG_PARSER_RULE s_ACDCParamsSetDict[] = {
    // 0. Dispatch Mode
    MAP_USHORT(0, ROUGH_CCU_DATA, strACDCParamsSet, usDispatchMode),
    // 1. Start Method
    MAP_USHORT(1, ROUGH_CCU_DATA, strACDCParamsSet, usStartMethod),
    // 2. On/Off Grid Mode
    MAP_USHORT(2, ROUGH_CCU_DATA, strACDCParamsSet, usOnOffGridMode),
    // 3. Storage Mode
    MAP_USHORT(3, ROUGH_CCU_DATA, strACDCParamsSet, usStorageMode),
    // 4. Battery Charge End Voltage
    MAP_USHORT(4, ROUGH_CCU_DATA, strACDCParamsSet, usBattChgEndVolt),
    // 5. Battery Charge Over-current
    MAP_USHORT(5, ROUGH_CCU_DATA, strACDCParamsSet, usBattChgOverCurr),
    // 6. Battery Discharge End Voltage
    MAP_USHORT(6, ROUGH_CCU_DATA, strACDCParamsSet, usBattDischgEndVolt),
    // 7. Battery Discharge Over-current
    MAP_USHORT(7, ROUGH_CCU_DATA, strACDCParamsSet, usBattDischgOverCurr),
    // 8. Battery DOD Voltage
    MAP_USHORT(8, ROUGH_CCU_DATA, strACDCParamsSet, usBattDODVolt),
};

int Parse_ACDC_Params_Set(ROUGH_CCU_DATA* pCCU, AR_FRAME* pMsgFrame)
{
    return Parse_By_Dictionary(pCCU, pMsgFrame, s_ACDCParamsSetDict, ITEM_OF(s_ACDCParamsSetDict));
}
