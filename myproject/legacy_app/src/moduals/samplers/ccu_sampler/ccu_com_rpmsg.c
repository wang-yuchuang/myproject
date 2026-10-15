// #define _TRACE_PRINT_   //RPMSG
#include "ccu_sampler_main.h"


#define LOG_FILE_PATH "/tmp/my_app.log"

#define LOG_TO_FILE(fmt, ...)                                      \
    do {                                                           \
        FILE *fp = fopen(LOG_FILE_PATH, "a");                      \
        if (fp != NULL) {                                          \
            fprintf(fp, fmt "\n", ##__VA_ARGS__);                  \
            fclose(fp);                                            \
        } else {                                                   \
            TRACED("无法打开日志文件！\n");                         \
        }                                                          \
    } while (0)


#if 0
/*********************************************************************************
*  
*  FUNCTION NAME :  CCU_RPMSG_Close_Client
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/06 15:51:15
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CCU_RPMSG_Close_Client(ROUGH_CCU_DATA* pCCU)
{
    TRACED("ccu_sampler_main:CCU_RPMSG_CloseClient");
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    if (pCCU->hDataComm != NULL)
    {
        pSampler->pUsedCommPort->Close(pCCU->hDataComm);
        pCCU->hDataComm = NULL;
    }
}

/*********************************************************************************
*  
*  FUNCTION NAME :  CCU_Check_And_ReOpen_CommPort
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/06 15:52:31
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void CCU_Check_And_ReOpen_CommPort(SAMPLER* pSampler)
{
    int nErrCode = 0;
    pSampler->hCommPort = pSampler->pUsedCommPort->Open(
        pSampler->pUsedCommPort->info.szPortAttri,
        pSampler->info.szSamplerSetting,
        COMM_SERVER_MODE, 100, &nErrCode
    );
    if ((ERR_COMM_OK != nErrCode) || (pSampler->hCommPort == NULL))
    {
        pSampler->hCommPort = NULL;
        TRACEF("CCU_Check_And_ReOpen_CommPort_rpmsg:Check_And_ReOpen_CommPort<%s> failed", pSampler->pUsedCommPort->info.szPortAttri);
		return FALSE;
    }
    return TRUE;
}
#endif

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/06 15:56:31
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void SendRpMsg_CCU(ROUGH_CCU_DATA* pCCU, FUNCTION_CODE emFunCode, 
    RP_MSGTYPE emMsgTp, int nDataLen, BYTE* pData)
{
    
    TRACED("[RPMSG-TRACED] SendRpMsg_CCU: ENTER emFC=[%d] emMsgTp=[%d] nDataLen=[%d] pData[0-2]=[%02X,%02X,%02X] bReady=[%d]",
        emFunCode, emMsgTp, nDataLen, pData[0], pData[1], pData[2], pCCU->bRpmsgPCS_CfgReady);
    int             i = 0, n = 0;
    BYTE            byBuff[AR_MAX_DATA_LEN];   // DataLen MAX（1024）
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;
    memset(byBuff, 0, AR_MAX_DATA_LEN);

	// 先检查核间通信通道是否建立
	if(!pCCU->bRpmsgPCS_CfgReady)
	{
        TRACED("SendRpMsg_CCU Function bRpmsgPCS_CfgReady==FALSE!!");
		return;
	}

    // 1. Function Code
    if((emFunCode != FUNCODE_REQ_MODIFY) && (emFunCode != FUNCODE_REQ_QUERY))
    {
        TRACED("SendRpMsg_CCU Function Code Error! emFunCode:[%d]", emFunCode);
        return;
    }

    // 2. RpMsg Type
    if((emMsgTp < RP_MSG_MODULE_TYPE_CFG) || (emMsgTp  > RP_MSG_ACDC_PARAMS_SET))
    {
        TRACED("SendRpMsg_CCU RpMsg Type Out of Range! emMsgTp:[%d]", emMsgTp);
        return;
    }

    // 3. Comm
    if(pData[0] == 0x0c)
    {
        // code
    }
    else if(pData[0] == 0x0d)
    {
        // code
    }
    else
    {
        TRACED("SendRpMsg_CCU Must be 0x0c or 0x0d.");
        return;
    }

    // 4. Max Index
    int SubIdxNum = (int)pCCU->rpFrameInfo[emMsgTp].usMaxSubIdxNum;
    if(pData[1] > SubIdxNum || pData[1] <= 0)
    {
        TRACED("SendRpMsg_CCU Maximum Sub-index Out of Bounds! SubIdxNum:[%d]", pData[1]);
        return;
    }

    // 5. Start Index
    int StartIdx = pData[2];
    if((StartIdx < 0) || (StartIdx > (SubIdxNum - 1)))
    {
        TRACED("SendRpMsg_CCU Start Sub-index Out of Bounds! Start Sub_Index:[%d]", pData[2]);
        return;
    }

    int nSendLen = nDataLen + 8;
    // DataLen
    byBuff[0] = (BYTE)(nSendLen);
    byBuff[1] = (BYTE)(nSendLen >> 8);
    byBuff[2] = (BYTE)(nSendLen >> 16);
    byBuff[3] = (BYTE)(nSendLen >> 24);
    // Function Code
    byBuff[4] = (BYTE)emFunCode;
    // MainIdx
    byBuff[5] = (BYTE)pCCU->rpFrameInfo[emMsgTp].byCmd;
    // dstAddr
    byBuff[6] = ADDR_R_KERNEL;
    // srcAddr
    byBuff[7] = ADDR_A_KERNEL_MIN;

    // Data
    for(n = 0; n < nDataLen; n++)
    {
        byBuff[8 + n] = pData[n];
    }

    TRACED("Sending...\n");
    for(int i = 0; i < nSendLen; i++)
    {
        TRACED("[%02X]", byBuff[i]);
    }
    TRACED("\n");
    
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/03/06 16:08:54
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Recv_RpMsg_CCU_Msg(ROUGH_CCU_DATA* pCCU)
{
    AR_FRAME strMsgFrame;
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

    BYTE byBuff[AR_MAX_DATA_LEN + 3];   // SOI（2） + EOI（1） + DataLen（2） + DataLen MAX（1024）//TODO:静态数组，清空或者用返回字节读取
    memset(byBuff, 0, sizeof(byBuff));

    int nReadBytes = pCCU->pRpmsgComm.Read(pCCU->hRpmsgPCS_InfoDev, (char*)byBuff, sizeof(byBuff));

    if (nReadBytes >= RPMSG_MIN_LEN)
    {
        BYTE* pFoundPos = byBuff;
        do
        {
            // 根据接收到的原始数据帧找到解析函数
            RPMSG_FRAME_INFO* pFrm = Find_RPMsgFrame(pCCU, pFoundPos, nReadBytes, &strMsgFrame);
            if (pFrm == NULL)
            {
                TRACED("---Get Frm Is NULL---\n");
                break;
            }
            int ret = pFrm->pfnParser(pCCU, &strMsgFrame);

            if(!ret)
            {
                TRACED("---Parse Frame Failed!!!!!!!!!!---\n");
                break;
            }
        } while(0);
        
    }

#if 0
    /* ALL_MODULES: MAP_USHORT, 2 bytes per field */
    BYTE rxbuf_USHORT[24] = {
        0xAB, 0XCD,               // SOI
        0x15,0x00,0x00,0x00,     // DataLen = 21
        0x08,                     // FunCode
        0x50,                     // MainIndex
        0x5A,                     // DstAddr
        0x32,                     // SrcAddr
        0x0c,                     // FrameFlag
        0x05,                     // SubIdxNum = 5
        0x00,                     // StartSubIdx
        0x01,0x00,                // USHORT Data1 = 1
        0x01,0x00,                // USHORT Data2 = 1
        0x02,0x00,                // USHORT Data3 = 2
        0x09,0x00,                // USHORT Data4 = 9
        0x01,0x00,                // USHORT Data5 = 5
        0xEF                      // EOI
    };

    /* GRP/SIG: MAP_UINT, 4 bytes per field */
    BYTE rxbuf_UINT[34] = {
        0xAB, 0XCD,               // SOI
        0x1F,0x00,0x00,0x00,     // DataLen = 31
        0x08,                     // FunCode
        0x51,                     // MainIndex (will increment)
        0x5A,                     // DstAddr
        0x32,                     // SrcAddr
        0x0c,                     // FrameFlag
        0x05,                     // SubIdxNum = 5
        0x00,                     // StartSubIdx
        0x01,0x00,0x00,0x00,     // UINT Data1 = 1
        0x01,0x00,0x00,0x00,     // UINT Data2 = 1
        0x02,0x00,0x00,0x00,     // UINT Data3 = 2
        0x09,0x00,0x00,0x00,     // UINT Data4 = 9
        0x01,0x00,0x00,0x00,     // UINT Data5 = 5
        0xEF                      // EOI
    };

    /* SIG: SubIdxNum=17 覆盖到条码字段(13~16), 全部UINT 4字节 */
    BYTE rxbuf_UINT_SIG[82] = {
        0xAB, 0XCD,               // SOI
        0x4F,0x00,0x00,0x00,     // DataLen = 79
        0x08,                     // FunCode
        0x55,                     // MainIndex = ACDC_SIG_1
        0x5A,                     // DstAddr
        0x32,                     // SrcAddr
        0x0c,                     // FrameFlag
        0x11,                     // SubIdxNum = 17
        0x00,                     // StartSubIdx
        // subidx 0: uiBelongGrpID = 1, subidx 1: uiModuID = 1
        0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
        // subidx 2~12: 11 × 4 = 44 bytes of zeros
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,
        // subidx 13~16: 条码信息
        0x00,0x00,0xB0,0x00,       // subidx 13: uiBarcode_1 = 0x00B00000 (LE)
        0x18,0x21,0x00,0x0B,       // subidx 14: uiBarcode_2 = 0x0B002118 (LE)
        0x4E,0x32,0x34,0x30,       // subidx 15: uiBarcode_3 = 0x3034324E (LE)
        0x32,0x30,0x30,0x31,       // subidx 16: uiBarcode_4 = 0x31303032 (LE)
        0xEF                      // EOI
    };


    // LOG_TO_FILE("\n----RUN_INFO page_handler was called!---\n");

    /* i=0: ALL_MODULES, i=1~4: GRP, i=5~8: SIG (barcode test) */
    for (int i = 0; i < 9; i++)
    {
        BYTE* prxbuf;
        int nReadBytes;

        if (i == 0)
        {
            prxbuf = rxbuf_USHORT;
            nReadBytes = sizeof(rxbuf_USHORT);
        }
        else if (i < 5)
        {
            rxbuf_UINT[7] = 0x50 + i;
            prxbuf = rxbuf_UINT;
            nReadBytes = sizeof(rxbuf_UINT);
        }
        else
        {
            rxbuf_UINT_SIG[7] = 0x55 + (i - 5);
            prxbuf = rxbuf_UINT_SIG;
            nReadBytes = sizeof(rxbuf_UINT_SIG);
        }

        memcpy(byBuff, prxbuf, nReadBytes);

        if (nReadBytes >= RPMSG_MIN_LEN)
        {
            BYTE* pFoundPos = byBuff;
            do
            {
                // 根据接收到的原始数据帧找到解析函数
                RPMSG_FRAME_INFO* pFrm = Find_RPMsgFrame(pCCU, pFoundPos, nReadBytes, &strMsgFrame);
                if (pFrm == NULL)
                {
                    TRACED("---Get Frm Is NULL---\n");
                    break;
                }
                int ret = pFrm->pfnParser(pCCU, &strMsgFrame);

                if(!ret)
                {
                    TRACED("---Parse Frame Failed!!!!!!!!!!---\n");
                    break;
                }

            } while(0);
            
        }
#if 1
        int n;

        TRACED("---------------------------strAllModuInfo -----------------------------\n");
        USHORT* pData = (USHORT*)(&pCCU->strAllModuInfo.usModuRunningMode);

        for (n = 0; n < 5; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pData);
            pData++;
        }

        TRACED("---------------------------strACDCGrpInfo -----------------------------\n");
        UINT* pd= (UINT*)(&pCCU->strACDCGrpInfo.uiOnlineModuNum);

        for (n = 0; n < 12; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pd);
            pd++;
        }

        TRACED("---------------------------strDCDCGrpInfo -----------------------------\n");
        pd = (UINT*)(&pCCU->strDCDCGrpInfo.uiOnlineModuNum);

        for (n = 0; n < 13; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pd);
            pd++;
        }

        TRACED("---------------------------strMPPTGrpInfo ----------------------------\n");
        pd = (UINT*)(&pCCU->strMPPTGrpInfo.uiOnlineModuNum);

        for (n = 0; n < 8; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pd);
            pd++;
        }

        TRACED("---------------------------strACDCSigInfo ---------------------------\n");
        pd = (UINT*)(&pCCU->strACDCSigInfo.uiBelongGrpID);

        for (n = 0; n < 46; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pd);
            pd++;
        }

        TRACED("---------------------------strDCDCSigInfo ---------------------------\n");
        pd = (UINT*)(&pCCU->strDCDCSigInfo.uiBelongGrpID);

        for (n = 0; n < 21; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pd);
            pd++;
        }

        TRACED("---------------------------strMPPTSigInfo ---------------------------\n");
        pd = (UINT*)(&pCCU->strMPPTSigInfo.uiBelongGrpID);

        for (n = 0; n < 15; n++)
        {
            TRACED("pData[%d] = [%02x]\n", n, *pd);
            pd++;
        }
#endif

    }
#endif
    
}

/*===============================================================================
 *  FUNCTION: Push_RpmsgReply_2_DPR
 *  PURPOSE : 将 RPMSG 回复帧解析后的配置数据推送到 DPR 状态参数 5320~5333
 *  MAPPING :
 *    RP_MSG_MODULE_TYPE_CFG   (0x6E) → 5320~5322 (strModuleTypeCfg)
 *    RP_MSG_MODULE_NUMBER_CFG (0x6F) → 5323~5328 (strModuleNumCfg)
 *    RP_MSG_MAUNL_AUTO_MODE   (0x72) → 5329     (strManulAutoCtrl)
 *    RP_MSG_ON_OFF_GRID_CTRL  (0x72) → 5330~5333 (strN2002_OnOffGridCtrl)
 *===============================================================================*/
static void Push_RpmsgReply_2_DPR(ROUGH_CCU_DATA* pCCU, RP_MSGTYPE emMsgTp)
{
    VAR_VALUE varV;
    memset(&varV, 0, sizeof(varV));

    switch (emMsgTp)
    {
    case RP_MSG_MODULE_TYPE_CFG:
        varV.emValue = pCCU->strModuleTypeCfg.byACDC_Module;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5320, varV);
        varV.emValue = pCCU->strModuleTypeCfg.byDCDC_Module;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5321, varV);
        varV.emValue = pCCU->strModuleTypeCfg.byMPPT_Module;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5322, varV);
        break;

    case RP_MSG_MODULE_NUMBER_CFG:
        varV.lValue = pCCU->strModuleNumCfg.byACDC_GRP1_SigNum;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5323, varV);
        varV.lValue = pCCU->strModuleNumCfg.byACDC_GRP2_SigNum;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5324, varV);
        varV.lValue = pCCU->strModuleNumCfg.byDCDC_GRP1_SigNum;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5325, varV);
        varV.lValue = pCCU->strModuleNumCfg.byDCDC_GRP2_SigNum;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5326, varV);
        varV.lValue = pCCU->strModuleNumCfg.byMPPT_GRP1_SigNum;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5327, varV);
        varV.lValue = pCCU->strModuleNumCfg.byMPPT_GRP2_SigNum;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5328, varV);
        break;

    case RP_MSG_MAUNL_AUTO_MODE:
        varV.emValue = pCCU->strManulAutoCtrl.byManulAutoMode;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5329, varV);
        break;

    case RP_MSG_ON_OFF_GRID_CTRL:
        varV.emValue = pCCU->strN2002_OnOffGridCtrl.byABU_Enable;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5330, varV);
        varV.emValue = pCCU->strN2002_OnOffGridCtrl.byOnOffGridAutoSwitchMode;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5331, varV);
        varV.emValue = pCCU->strN2002_OnOffGridCtrl.byOnOffGridMode;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5332, varV);
        varV.emValue = pCCU->strN2002_OnOffGridCtrl.byABU_OnOff;
        Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5333, varV);
        break;
    
    // case RP_MSG_ACDC_PARAMS_SET:
    //     varV.emValue = pCCU->strN2002_OnOffGridCtrl.byABU_OnOff;
    //     Push_VarValue(SETTYPE_RAW_SP, pCCU->hSampler, 701, 5333, varV);
    //     break;

    default:
        break;
    }
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/25 13:53:37
*  DESCRIPTION   :  Receive reply  message of CCU config.
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Recv_RpMsg_CCU_Reply_Msg(ROUGH_CCU_DATA* pCCU)
{
    AR_FRAME strMsgFrame;
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

    BYTE byBuff[AR_MAX_DATA_LEN + 3];   // SOI（2） + EOI（1） + DataLen（2） + DataLen MAX（1024）//TODO:静态数组，清空或者用返回字节读取
    memset(byBuff, 0, sizeof(byBuff));

    int nReadBytes = pCCU->pRpmsgComm.Read(pCCU->hRpmsgPCS_CfgDev, (char*)byBuff, sizeof(byBuff));

    if (nReadBytes >= RPMSG_MIN_LEN)
    {
        BYTE* pFoundPos = byBuff;
        do
        {
            // 根据接收到的原始数据帧找到解析函数
            RPMSG_FRAME_INFO* pFrm = Find_RPMsgFrame(pCCU, pFoundPos, nReadBytes, &strMsgFrame);
            if (pFrm == NULL)
            {
                TRACED("---Get Frm Is NULL---\n");
                break;
            }
            int ret = pFrm->pfnParser(pCCU, &strMsgFrame);

            if(!ret)
            {
                TRACED("---Parse Frame Failed!!!!!!!!!!---\n");
                break;
            }

            /* 解析成功后，将解析结果推送到 DPR 状态参数表 5320~5333 */
            Push_RpmsgReply_2_DPR(pCCU, pFrm->msgTp);

        } while(0);
        
    }
    
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/25 14:25:11
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void BuildRpmsgPort(ROUGH_CCU_DATA* pCCU) 
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

    int nErrCode = 0;

    nErrCode = pCCU->pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT, 12, NULL, &pCCU->pRpmsgComm, sizeof(COMMPORT));

    if(ERR_COMM_OK != nErrCode)
    {
        TRACED("Open CCU RPMSGComm Failed !!! nErrCode=[%d]\n", nErrCode);
        return;
    }

    pCCU->bRpmsgPortReady = TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/25 14:27:54
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void OpenRpmsgPCSInfoDev(ROUGH_CCU_DATA* pCCU) 
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

    int nErrCode = 0;
    pCCU->hRpmsgPCS_InfoDev = pCCU->pRpmsgComm.Open(
        RPMSG_PCS_INFO,
        pSampler->info.szSamplerSetting,
        3, 0, &nErrCode);
    if(pCCU->hRpmsgPCS_InfoDev == NULL)
    {
        TRACED("Open hRpmsg PCS Info Dev Failed !!! hRpmsgPCS_InfoDev is NULL\n");
        return;
    }

    if(ERR_COMM_OK != nErrCode)
    {
        TRACED("Open hRpmsg PCS Info De Failed !!! nErrCode=[%d]\n", nErrCode);
        return;
    }

    pCCU->bRpmsgPCS_InfoReady = TRUE;

    TRACED("Open hRpmsg PCS Info Dev Succeed!!!!!!!!!!!!!!!!!\n");
}

/*********************************************************************************
*  
*  FUNCTION NAME :  
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/25 14:31:04
*  DESCRIPTION   :  
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void OpenRpmsgPCSCfgDev(ROUGH_CCU_DATA* pCCU) 
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

    int nErrCode = 0;
    pCCU->hRpmsgPCS_CfgDev = pCCU->pRpmsgComm.Open(
        RPMSG_PCS_CFG_INFO,
        pSampler->info.szSamplerSetting,
        3, 0, &nErrCode);

    if(pCCU->hRpmsgPCS_CfgDev == NULL)
    {
        TRACED("Open hRpmsg PCS Config Dev Failed !!! hRpmsgPCS_InfoDev is NULL\n");
        return;
    }

    if(ERR_COMM_OK != nErrCode)
    {
        TRACED("Open hRpmsg PCS Config Dev Failed !!! nErrCode=[%d]\n", nErrCode);
        return;
    }

    pCCU->bRpmsgPCS_CfgReady = TRUE;

    TRACED("Open hRpmsg PCS Config Dev Succeed!!!!!!!!!!!!!!!!!\n");
}

/*********************************************************************************
*  
*  FUNCTION NAME :  BuildRpmsgCommChannel
*  CREATOR       :  Lyu Zhiyue
*  CREATED DATE  :  2026/05/25 14:01:33
*  DESCRIPTION   :  Building RPMSG Communication Channels and Drivers.
*            
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void BuildRpmsgCommChannel(ROUGH_CCU_DATA* pCCU)
{
    SAMPLER* pSampler = (SAMPLER*)pCCU->hSampler;

    if(!pCCU->bRpmsgPortReady)
    {
        BuildRpmsgPort(pCCU);
    }

    if(pCCU->bRpmsgPortReady && (!pCCU->bRpmsgPCS_InfoReady))
    {
        OpenRpmsgPCSInfoDev(pCCU);
    }

    if(pCCU->bRpmsgPortReady && (!pCCU->bRpmsgPCS_CfgReady))
    {
        OpenRpmsgPCSCfgDev(pCCU);
    }

    // if(pCCU->hRpmsgCycleDev==NULL)
    // {
    //     pCCU->hRpmsgCycleDev = pCCU->pRpmsgComm.Open(
    //         RPMSG_CYCLE_INFO,
    //         pSampler->info.szSamplerSetting,
    //         3, 0, NULL);
    // }
}