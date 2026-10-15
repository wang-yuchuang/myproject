#include "../fcgi_defs.h"

#define LOG_FILE_PATH "/tmp/my_app.log"

#define LOG_TO_FILE(fmt, ...)                                      \
    do {                                                           \
        FILE *fp = fopen(LOG_FILE_PATH, "a");                      \
        if (fp != NULL) {                                          \
            fprintf(fp, fmt "\n", ##__VA_ARGS__);                  \
            fclose(fp);                                            \
        } else {                                                   \
            printf("无法打开日志文件！\n");                         \
        }                                                          \
    } while (0)


#define MAX_GRPPM_ROW                4
#define MAX_POWALLOC_RECT            4
// #define MAX_POWALLOC_ROW             11
// #define MAX_POWALLOC_PAGE            4

typedef enum tagRunInfoPageId
{
    RUN_INFO_PAGEID_ALLMODU = 0,  /*模块整体运行信息*/
    RUN_INFO_PAGEID_POWERALLOCATE, /*功率分配运行信息*/
    RUN_INFO_PAGEID_ACDC_GRPPM,  /*ACDC模块组运行信息*/
    RUN_INFO_PAGEID_DCDC_GRPPM,  /*DCDC模块组运行信息*/
    RUN_INFO_PAGEID_MPPT_GRPPM,  /*MPPT模块组运行信息*/
    RUN_INFO_PAGEID_SINGLEPM, /*单个模块运行信息*/

    RUN_INFO_PAGEID_MAX
}RUN_INFO_PAGEIDS;

typedef enum tagSEMSRunInfo
{
    SEMS_RUN_SLAVERUN_IDX = 0,
    SEMS_RUN_SLAVE_PCSINFO,

    SEME_RUN_MAX
}SEMS_RUN_INFO;

static int min(int a, int b)
{
    if(a <= b)
    {
        return a;
    }
    else
    {
        return b;
    }
}

static BOOL Handle_RunInfo_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int nPageIdx/* , int nSubPageIdx, int nRowsPerPage, int nSelectedGrp */)
{
    TRACEX("\n----now handling run_info data----\n\n");
    int n;
    char szGrpPMName[64] = "\0";
    // LJH241202 SEMS ClientInfo
    if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        int nSlaveNumber;
        if (!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 43, 4, (void*)&nSlaveNumber))
        {
            nSlaveNumber = 0;
        }
        // int nTotalPageNum = (nSlaveNumber+nRowsPerPage-1) / nRowsPerPage;
        // TRACED("Handle_RunInfo_Data nSlaveNumber:%d nTotalPageNum:%d nSubPageIdx:%d nRowsPerPage:%d min:%d",
        //     nSlaveNumber, nTotalPageNum, nSubPageIdx, nRowsPerPage, min(nSlaveNumber, (nSubPageIdx + 1) * nRowsPerPage));
        // if (nSubPageIdx < 0 || nSubPageIdx >= nTotalPageNum)
        // {
        //     Make_Web_OutPut(104, "The 'pageId'(sub page id) value INVALID!", NULL, NULL);
        //     return TRUE;
        // }

        cJSON* pJSClientInfo_SEMS = cJSON_CreateArray();
        char szGrpPMName[64] = "\0";
        size_t nUsed = 0;
        A_BLOB_SEMS_CLIENTINFO clientInfo;
        EMS_MGMT_STU* pEMS = &clientInfo.sems;
        A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData_SEMS;
	    A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node;
        SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS;
        A_BLOB_SEMS_CLIENTINFO_SUM clientSum;
        A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &clientSum;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3900, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM), (void*)pClientSum))
        {
            TRACEE("----ERRRRRR: can not get A_BLOB_SEMS_CLIENTINFO_SUM Data----%d", 3300);
        }
        TRACED("Handle_RunInfo_Data "
            "ClientSum emEMSCtrlMode_REMS:%d fActvPwrReq_REMS:%.1f emCCUStateSum:%d emPMCtrlModeSum:%d emACDC_WorkStatuSum:%d "
            "nSOCMinSum:%d nSOCMaxSum:%d fSOCSum:%.1f fActvPwr_PCSGrpSum:%.1f fQ_PCSGrpSum:%.1f "
            "\nfMax_Chrg_PwrAbilitySum:%.1f fMax_DisChrg_PwrAbilitySum:%.1f AbilitySum0:%.1f %.1f "
            "fChrgedEnergySum:%.1f fRealTime_MaxCurrSum:%.1f fACDC_RealTime_VoltSum:%.1f fACDC_RealTime_CurrSum:%.1f "
            "f_PgridMeterSum:%.1f f_PcabMeterSum:%.1f",
            pClientSum->emEMSCtrlMode_REMS, pClientSum->fActvPwrReq_REMS, pClientSum->emCCUStateSum, pClientSum->emPMCtrlModeSum, pClientSum->emACDC_WorkStatuSum, 
            pClientSum->nSOCMinSum, pClientSum->nSOCMaxSum, pClientSum->fSOCSum, pClientSum->fActvPwr_PCSGrpSum, pClientSum->fQ_PCSGrpSum,
            pClientSum->fMax_Chrg_PwrAbilitySum, pClientSum->fMax_DisChrg_PwrAbilitySum, pClientSum->fMax_Chrg_PwrAbilitySum0, pClientSum->fMax_DisChrg_PwrAbilitySum0,
            pClientSum->fChrgedEnergySum, pClientSum->fRealTime_MaxCurrSum, pClientSum->fACDC_RealTime_VoltSum, pClientSum->fACDC_RealTime_CurrSum,
            pClientSum->f_PgridMeterSum, pClientSum->f_PcabMeterSum);
        //provide general info

        if(nPageIdx == SEMS_RUN_SLAVERUN_IDX)
        {
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", nSlaveNumber);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 0);

            for(n = 0/* nSubPageIdx * nRowsPerPage */; n < nSlaveNumber/* min(nSlaveNumber, (nSubPageIdx + 1) * nRowsPerPage) */; n++)
            {
                if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3301 + n, sizeof(A_BLOB_SEMS_CLIENTINFO), (void*)&clientInfo))
                {
                    TRACEE("----ERRRRRR: can not get A_BLOB_SEMS_CLIENTINFO Data----%d", 3301 + n);
                    continue;
                }
                pBranchNodeData_SEMS =  &clientInfo.branchNodeData_SEMS;
                pCalc_Node =       &pEMS->calc_Node[n];
                pB6LSYSLast_SEMS = &pBranchNodeData_SEMS->b6LSYSLast_SEMS;
                cJSON* pJSGrpPMInfo = cJSON_CreateObject();

                cJSON_AddNumberToObject(pJSGrpPMInfo, "grp", n + 1);
                // IP Address
                snprintf(szGrpPMName, 64, "com:%s|%s:%d|No:%u",
                    clientInfo.bCommFailSemsClient?LoadLangText_ById(201):LoadLangText_ById(34),
                    inet_ntoa(*(struct in_addr *)clientInfo.stClientInfo.szAddr), clientInfo.stClientInfo.nPeerPort, pB6LSYSLast_SEMS->idxClient);
                cJSON_AddStringToObject(pJSGrpPMInfo, "rate_pwr", szGrpPMName);
                // 有功功率需求
                // switch (pCalc_Node->emBatt_WorkMode)
                switch (pEMS->planSectorNow.emBatt_WorkMode)
                {
                case BATT_WORKMODE_FIRST_DISCHRG: case BATT_WORKMODE_DISCHRG_ONLY:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(552)); // 807 2831
                    break;
                case BATT_WORKMODE_FIRST_CHRGING: case BATT_WORKMODE_CHRGING_ONLY:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(2830)); // 808 2832
                    break;
                case BATT_WORKMODE_STANDBY:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6710));
                    break;
                case BATT_WORKMODE_IDLE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6702));
                    break;
            
                default:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(31));
                    break;
                }
                if (pEMS->bCtrlByREMS)
                {
                    nUsed = strlen(szGrpPMName);
                    snprintf(szGrpPMName + nUsed, 64-nUsed,
                        "|R:%.1f", clientSum.fActvPwrReq_REMS);
                }
                nUsed = strlen(szGrpPMName);
                snprintf(szGrpPMName + nUsed, 64-nUsed,
                    "|PCh:%.1f/%.1f|ld:%.1f/%.1fkW",
                    pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessDischgLmt, pEMS->fPacload4Chg, pEMS->fPacload4Dischg);
                cJSON_AddStringToObject(pJSGrpPMInfo, "rate_curr", szGrpPMName);
                // 最大输出功率
                snprintf(szGrpPMName, 64, "ch:%.1f/%.1f|A:%.1f/%.1f|0:%.1f/%.1fkW|Curr:%.1fA",
                    pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum, pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum,
                    pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySumA, pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySumA,
                    pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum0, pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum0,
                    pB6LSYSLast_SEMS->fRealTime_MaxCurrSum);
                cJSON_AddStringToObject(pJSGrpPMInfo, "max_curr", szGrpPMName);
                // 工作模式
                switch (pB6LSYSLast_SEMS->emACDC_WorkStatuSum)
                {
                case STATU_ACDC_IDLE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(91));
                    break;
                case STATU_ACDC_RECTIFY:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(709));
                    break;
                case STATU_ACDC_INVERTER:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(710));
                    break;
                case STATU_ACDC_STANDBY:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6710));
                    break;
                default:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(31));
                    break;
                }
                nUsed = strlen(szGrpPMName);
                snprintf(szGrpPMName + nUsed, 64-nUsed,
                    "|s:%d|m:%d", pB6LSYSLast_SEMS->emCCUStateSum, pB6LSYSLast_SEMS->emPMCtrlModeSum);
                cJSON_AddStringToObject(pJSGrpPMInfo, "mode", szGrpPMName);
                // SOC
                snprintf(szGrpPMName, 64, "SOC:%d/%d/%.1f|Stop:%u|%.1fkWh",
                    pB6LSYSLast_SEMS->nSOCMinSum, pB6LSYSLast_SEMS->nSOCMaxSum, pB6LSYSLast_SEMS->fSOCSum, pCalc_Node->nStopSOCReq, pB6LSYSLast_SEMS->fChrgedEnergySum);
                cJSON_AddStringToObject(pJSGrpPMInfo, "ctrl_volt", szGrpPMName);
                // 功率分配
                snprintf(szGrpPMName, 64, "ch:%.1f|dc:%.1fkW",
                    pCalc_Node->fACDC_Distri_4_BattChrg_Pwr, pCalc_Node->fBatt_Distri_4_ACDC_Pwr);
                cJSON_AddStringToObject(pJSGrpPMInfo, "ctrl_curr", szGrpPMName);
                // 输出电压
                snprintf(szGrpPMName, 64, "%.1fV",
                    pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum);
                cJSON_AddStringToObject(pJSGrpPMInfo, "rtime_volt", szGrpPMName);
                // 输出电流
                snprintf(szGrpPMName, 64, "%.1fA",
                    pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum);
                cJSON_AddStringToObject(pJSGrpPMInfo, "rtime_curr", szGrpPMName);
                // 输出功率
                snprintf(szGrpPMName, 64, "cab:%.1f/%.1f|pcs:%.1f|G:%.1f/%.1fkW",
                    pB6LSYSLast_SEMS->f_PcabMeterSum, pEMS->f_PcabMeter,
                    pB6LSYSLast_SEMS->fActvPwr_PCSGrpSum,
                    pB6LSYSLast_SEMS->f_PgridMeterSum, pClientSum->f_PgridMeterSum);
                    //pEMS->fAtOvl_kW, pEMS->fAtRflx_kW);
                cJSON_AddStringToObject(pJSGrpPMInfo, "online", szGrpPMName);

                cJSON_AddItemToArray(pJSClientInfo_SEMS, pJSGrpPMInfo);
            }

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSClientInfo_SEMS);
        }

        if(nPageIdx == SEMS_RUN_SLAVE_PCSINFO)
        {
            SEMS_PCS_INFO_DATA* PCS_info;
            cJSON* pJSClientInfo_SEMS = cJSON_CreateArray();
            for(n = 0; n < nSlaveNumber; n++)
            {
                if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3301 + n, sizeof(A_BLOB_SEMS_CLIENTINFO), (void*)&clientInfo))
                {
                    TRACEE("----ERRRRRR: can not get A_BLOB_SEMS_CLIENTINFO Data----%d", 3301 + n);
                    continue;
                }
                PCS_info =  &clientInfo.stPcsInfoData; 
                int i;
                
                for(i = 0; i < PCS_info->nPCSNum; ++i)
                {
                    cJSON* pJSGrpPMInfo = cJSON_CreateObject();
                    cJSON_AddNumberToObject(pJSGrpPMInfo, "Grp", n + 1);
                    cJSON_AddNumberToObject(pJSGrpPMInfo, "Index", i + 1);
                    cJSON_AddStringToObject(pJSGrpPMInfo, "version", PCS_info->szVersion[i]);
                    cJSON_AddStringToObject(pJSGrpPMInfo, "barcode", PCS_info->szBar_code[i]);

                    cJSON_AddItemToArray(pJSClientInfo_SEMS, pJSGrpPMInfo);
                }
            }
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSClientInfo_SEMS);
        }

        return TRUE;
    }

    if(nPageIdx == RUN_INFO_PAGEID_ALLMODU)    /*模块整体信息*/
    {
        cJSON* pJSAllModuleInfo = cJSON_CreateArray();
        cJSON* pJSAMInfo = NULL;
        A_BLOB_RECT_GRP_DATA rectGrpData;
        char szGrpPMName[64] = "\0";

        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 204, sizeof(A_BLOB_RECT_GRP_DATA), (void*)&rectGrpData))
        {
            printf("----ERRRRRR: can not get Rect Grp Data----\n");
            Make_Web_OutPut(104, "can not get Rect Grp Data!", NULL, NULL);
            goto ERROR_1;
        }

        // rectGrpData.emModuleRunningMode = 0;
        // rectGrpData.emManuAutoMode = 0;
        // rectGrpData.emChrgDischrgCtrlFailReason = 0;
        // rectGrpData.unioModuleCfgInfo.usModuleGrpCfg = 1;
        // rectGrpData.unioModuleOnlineInfo.usModuleGrpOnline = 1;


        //provide general info
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", MAX_GRPPM_ROW);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 0);

        pJSAMInfo = cJSON_CreateObject();
        // 模块运行模式
        if(rectGrpData.emModuleRunningMode == 0)
        {
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(34));
            cJSON_AddStringToObject(pJSAMInfo, "mode", szGrpPMName);
        }
        else if(rectGrpData.emModuleRunningMode == 1)
        {
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(553));
            cJSON_AddStringToObject(pJSAMInfo, "mode", szGrpPMName);
        }
        else if(rectGrpData.emModuleRunningMode == 2)
        {
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(198));
            cJSON_AddStringToObject(pJSAMInfo, "mode", szGrpPMName);
        }
        else if(rectGrpData.emModuleRunningMode == 3)
        {
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(7218));
            cJSON_AddStringToObject(pJSAMInfo, "mode", szGrpPMName);
        }
        else
        {
            printf("----ERRRRRR: mode value is illegal----\n");
            Make_Web_OutPut(104, "The 'mode' value INVALID!", NULL, NULL);
            goto ERROR_1;
        }
        // 手动/自动模式
        if(rectGrpData.emManuAutoMode == 0)
        {
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(461));
            cJSON_AddStringToObject(pJSAMInfo, "ctrl", szGrpPMName);
        }
        else if(rectGrpData.emManuAutoMode == 1)
        {
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(460));
            cJSON_AddStringToObject(pJSAMInfo, "ctrl", szGrpPMName);
        }
        else
        {
            printf("----ERRRRRR: ctrl value is illegal----\n");
            Make_Web_OutPut(104, "The 'ctrl' value INVALID!", NULL, NULL);
            goto ERROR_1;
        }

        switch (rectGrpData.emChrgDischrgCtrlFailReason)
        {
        case 0:
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(34));
            break;
        case 1:
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9007));
            break;
        case 2:
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9008));
            break;
        case 3:
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9009));
            break;
        case 4:
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9010));
            break;
        case 5:
            snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9011));
            break;
        
        default:
            printf("----ERRRRRR: reason value is illegal----\n");
            Make_Web_OutPut(104, "The 'reason' value INVALID!", NULL, NULL);
            goto ERROR_1;
            // break;
        }
        cJSON_AddStringToObject(pJSAMInfo, "reason", szGrpPMName);

        snprintf(szGrpPMName, 64, "%d|%d|%d|%d|%d|%d",
                    rectGrpData.unioModuleCfgInfo.Info.ACDC_Grp_1, 
                    rectGrpData.unioModuleCfgInfo.Info.ACDC_Grp_2,
                    rectGrpData.unioModuleCfgInfo.Info.DCDC_Grp_1,
                    rectGrpData.unioModuleCfgInfo.Info.DCDC_Grp_2,
                    rectGrpData.unioModuleCfgInfo.Info.MPPT_Grp_1,
                    rectGrpData.unioModuleCfgInfo.Info.MPPT_Grp_2
                );
        cJSON_AddStringToObject(pJSAMInfo, "grp_cfg", szGrpPMName);

        snprintf(szGrpPMName, 64, "%d|%d|%d|%d|%d|%d",
                    rectGrpData.unioModuleOnlineInfo.Info.ACDC_Grp_1, 
                    rectGrpData.unioModuleOnlineInfo.Info.ACDC_Grp_2,
                    rectGrpData.unioModuleOnlineInfo.Info.DCDC_Grp_1,
                    rectGrpData.unioModuleOnlineInfo.Info.DCDC_Grp_2,
                    rectGrpData.unioModuleOnlineInfo.Info.MPPT_Grp_1,
                    rectGrpData.unioModuleOnlineInfo.Info.MPPT_Grp_2
                );
        cJSON_AddStringToObject(pJSAMInfo, "grp_ol", szGrpPMName);
        
        cJSON_AddItemToArray(pJSAllModuleInfo, pJSAMInfo);
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSAllModuleInfo);

        return TRUE;

ERROR_1:
        // 防止内存泄漏
        if(pJSAllModuleInfo != NULL)
        {
            cJSON_Delete(pJSAllModuleInfo);
        }
        if(pJSAMInfo != NULL)
        {
            cJSON_Delete(pJSAMInfo);
        }
        return TRUE;
    }

    if(nPageIdx == RUN_INFO_PAGEID_POWERALLOCATE)  /*功率分配运行数据*/
    {
        // if(nSubPageIdx < 0 || nSubPageIdx >= MAX_POWALLOC_PAGE)
        // {
        //     Make_Web_OutPut(104, "The 'pageId'(sub page id) value INVALID!", NULL, NULL);
        //     return TRUE;  
        // }

        cJSON* pJSPowAllocTotalInfo = cJSON_CreateArray();
        cJSON* pJSPowAllocInfo = cJSON_CreateObject();
        float fACReqPower = 0.0;
        float fPhaseA_Curr = 0.0;
        float fPhaseB_Curr = 0.0;
        float fPhaseC_Curr = 0.0;
        char szStringName[32] = "\0";
        char szPowAllocName[64] = "\0";
        
        //get data for this page
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 408, 400, 4, (void*)&fACReqPower))
        {
            printf("----ERRRRRR: can not get ACReqPower----\n");
            Make_Web_OutPut(104, "can not get ACReqPower!", NULL, NULL);
            goto ERROR_2;
        }
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 408, 414, 4, (void*)&fPhaseA_Curr))
        {
            printf("----ERRRRRR: can not get PhaseA_Curr----\n");
            Make_Web_OutPut(104, "can not get PhaseA_Curr!", NULL, NULL);
            goto ERROR_2;
        }
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 408, 416, 4, (void*)&fPhaseB_Curr))
        {
            printf("----ERRRRRR: can not get PhaseB_Curr----\n");
            Make_Web_OutPut(104, "can not get PhaseB_Curr!", NULL, NULL);
            goto ERROR_2;
        }
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 408, 418, 4, (void*)&fPhaseC_Curr))
        {
            printf("----ERRRRRR: can not get PhaseC_Curr----\n");
            Make_Web_OutPut(104, "can not get PhaseC_Curr!", NULL, NULL);
            goto ERROR_2;
        }

        A_BLOB_BRANCH_GRP_RECTDATA             szGrpRectData[MAX_POWALLOC_RECT];
        A_BLOB_BRANCH_NODE_CALC_DATA           szNodeCalcData[MAX_POWALLOC_RECT];

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 211 + n, 201, sizeof(A_BLOB_BRANCH_GRP_RECTDATA), (void*)&szGrpRectData[n]))
            {
                printf("----ERRRRRR: can not get BRANCH_GRP_RECTDATA----\n");
                Make_Web_OutPut(104, "can not get BRANCH_GRP_RECTDATA!", NULL, NULL);
                goto ERROR_2;
            }

            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 221 + n, 203, sizeof(A_BLOB_BRANCH_NODE_CALC_DATA), (void*)&szNodeCalcData[n]))
            {
                printf("----ERRRRRR: can not get BRANCH_NODE_CALC_DATA----\n");
                Make_Web_OutPut(104, "can not get BRANCH_NODE_CALC_DATA!", NULL, NULL);
                goto ERROR_2;
            }
        }

        //provide general info
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", MAX_POWALLOC_ROW * MAX_POWALLOC_PAGE);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", MAX_POWALLOC_PAGE);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", nSubPageIdx);
        

        //make ids and strings out of raw data
        //row 1
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 1);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(831));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.0fkW | %.0fA | %.0fA | %.0fA",
                    fACReqPower, fPhaseA_Curr, fPhaseB_Curr, fPhaseC_Curr);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);

        //row 2
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 2);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(784));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fV | %.1fA | %d%%",
                    szGrpRectData[n].fBatt_RealTime_Volt, szGrpRectData[n].fBatt_RealTime_Curr, szGrpRectData[n].nSOC);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);

        //row 3
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 3);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(785));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.0fV | %.0fA | %.0fkW  | %.0fV",
                    szGrpRectData[n].fACDC_RealTime_Volt, szGrpRectData[n].fACDC_RealTime_Curr,
                    szGrpRectData[n].fACDC_Max_Output_Pwr_Ability, szGrpRectData[n].fACDCOutRelayVolt);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 4
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 4);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(786));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fV | %.1fA | %.1fkW",
                    szGrpRectData[n].fDCDC_RealTime_Volt, szGrpRectData[n].fDCDC_RealTime_Curr,
                    szGrpRectData[n].fDCDC_Max_Output_Pwr_Ability);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 5
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 5);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(783));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fV | %.1fA | %.1fkW",
                    szGrpRectData[n].fMPPT_RealTime_Volt, szGrpRectData[n].fMPPT_RealTime_Curr,
                    szGrpRectData[n].fMPPT_RealTime_Volt*szGrpRectData[n].fMPPT_RealTime_Curr*0.001);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 6
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 6);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(792));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fkW | Avg:%.1fkW",
                    szNodeCalcData[n].fACDC_Distri_4_BattChrg_Pwr, szNodeCalcData[n].fAvg_ChrgLmtPower);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 7
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 7);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(789));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fkW | Avg:%.1fkW",
                    szNodeCalcData[n].fBatt_Distri_4_ACDC_Pwr, szNodeCalcData[n].fAvg_DisChrgingLmtPower);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 8
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 8);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(1971));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fkVar | Avg:%.1fkVar",
                    szNodeCalcData[n].fQoutput_Distri4ACDC, szNodeCalcData[n].fAvg_QoutputLmt);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 9
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 9);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(794));
        // LJH250221 Added fMax_DisChrg_PwrAbility && fMax_Chrg_PwrAbility
        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fkW | %.1fkW | %.1fkW",
                    szNodeCalcData[n].fMax_DisChrg_PwrAbility, szNodeCalcData[n].fBatt_Max_DisChrg_Pwr, szNodeCalcData[n].fRemain_Batt_DisChrg_Pwr);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);
        
        //row 10
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 10);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(791));

        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fkW | %.1fkW",
                    szNodeCalcData[n].fMax_Chrg_PwrAbility, szNodeCalcData[n].fBatt_Max_Chrg_Pwr_Req);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);

        //row 11
        pJSPowAllocInfo = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJSPowAllocInfo, "idx", 11);

        cJSON_AddStringToObject(pJSPowAllocInfo, "paramName", LoadLangText_ById(796));
        
        for(n = 0; n < MAX_POWALLOC_RECT; n++)
        {
            snprintf(szStringName, 32, "paramVal%d", n + 1);
            snprintf(szPowAllocName, 64, "%.1fkW",
                    szNodeCalcData[n].fRemain_Batt_Chrg_Pwr_Req);
            cJSON_AddStringToObject(pJSPowAllocInfo, szStringName, szPowAllocName);
        }
        cJSON_AddItemToArray(pJSPowAllocTotalInfo, pJSPowAllocInfo);

        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSPowAllocTotalInfo);

        return TRUE;

ERROR_2:
        // 防止内存泄漏
        if(pJSPowAllocTotalInfo != NULL)
        {
            cJSON_Delete(pJSPowAllocTotalInfo);
        }
        if(pJSPowAllocInfo != NULL)
        {
            cJSON_Delete(pJSPowAllocInfo);
        }
        return TRUE;
    }

    if(nPageIdx == RUN_INFO_PAGEID_ACDC_GRPPM)   /*ACDC模块组运行数据*/
    {
        cJSON* pJSACDCGrpPMTotalInfo = cJSON_CreateArray();
        cJSON* pJSACDCGrpPMInfo = NULL;
        A_BLOB_RECT_GRP_DATA rectGrpData;

        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 204, sizeof(A_BLOB_RECT_GRP_DATA), (void*)&rectGrpData))
        {
            printf("----ERRRRRR: can not get ACDC Rect Grp Data----\n");
            Make_Web_OutPut(104, "can not get ACDC Rect Grp Data!", NULL, NULL);
            goto ERROR_3;
        }

        //provide general info
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", MAX_GRPPM_ROW);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 0);

        for(n = TYPE_SIG_RECT_ACDC_GRP1/* nSubPageIdx * nRowsPerPage */; n <= TYPE_SIG_RECT_ACDC_GRP4/* min(MAX_GRPPM_ROW, (nSubPageIdx + 1) * nRowsPerPage) */; n++)
        {
            pJSACDCGrpPMInfo = cJSON_CreateObject();

            cJSON_AddNumberToObject(pJSACDCGrpPMInfo, "grp", n + 1);

            cJSON_AddNumberToObject(pJSACDCGrpPMInfo, "ol_num", rectGrpData.nOnlineRectNum[n]);

            snprintf(szGrpPMName, 64, "%.1fkW", rectGrpData.fRatedPower[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "rated_power", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRatedCurr[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "rated_curr", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRealTime_MaxCurr[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "max_curr", szGrpPMName);

            switch (rectGrpData.emCtrlStatus[n])
            {
                case CTRL_STATUS_OFF:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6702));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_stats", szGrpPMName);
                    break;

                case CTRL_STATUS_RECTIFY_POSITIVE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(709));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_stats", szGrpPMName);
                    break;
                
                case CTRL_STATUS_INVERTER_REVERSE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(710));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_stats", szGrpPMName);
                    break;

                default:
                    printf("----ERRRRRR: ctrl_stats value is illegal----\n");
                    Make_Web_OutPut(104, "The 'ctrl_stats' value INVALID!", NULL, NULL);
                    goto ERROR_3;
            }

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fCtrlVolt[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_dc_volt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fCtrlCurr[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_dc_curr", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fkW", rectGrpData.fCtrlPower[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_power", szGrpPMName);

            switch (rectGrpData.emRunningStatus[n])
            {
                case RUNNING_STATUS_ACDC_OFF:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6702));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_ACDC_RECTIFY:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(709));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;
                
                case RUNNING_STATUS_ACDC_INVERTER:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(710));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_ACDC_FAULT:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6581));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;
                
                case RUNNING_STATUS_ACDC_ALARM:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6887));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_ACDC_INCONSISTENT:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9030));
                    cJSON_AddStringToObject(pJSACDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                default:
                    printf("----ERRRRRR: run_stats value is illegal----\n");
                    Make_Web_OutPut(104, "The 'run_stats' value INVALID!", NULL, NULL);
                    goto ERROR_3;
            }

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fRealTime_BattSideVolt[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "rt_dc_volt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRealTime_BattSideTotalCurr[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "ctrl_dc_tol_curr", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fkW", rectGrpData.fRealTimeSystemPower[n]);
            cJSON_AddStringToObject(pJSACDCGrpPMInfo, "rt_sys_power", szGrpPMName);

            cJSON_AddItemToArray(pJSACDCGrpPMTotalInfo, pJSACDCGrpPMInfo);
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSACDCGrpPMTotalInfo);

        return TRUE;
ERROR_3:
        // 防止内存泄漏
        if(pJSACDCGrpPMTotalInfo != NULL)
        {
            cJSON_Delete(pJSACDCGrpPMTotalInfo);
        }
        if(pJSACDCGrpPMInfo != NULL)
        {
            cJSON_Delete(pJSACDCGrpPMInfo);
        }
        return TRUE;
    }

    if(nPageIdx == RUN_INFO_PAGEID_DCDC_GRPPM)   /*DCDC模块组运行数据*/
    {
        cJSON* pJSDCDCGrpPMTotalInfo = cJSON_CreateArray();
        cJSON* pJSDCDCGrpPMInfo = NULL;
        A_BLOB_RECT_GRP_DATA rectGrpData;

        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 204, sizeof(A_BLOB_RECT_GRP_DATA), (void*)&rectGrpData))
        {
            printf("----ERRRRRR: can not get DCDC Rect Grp Data----\n");
            Make_Web_OutPut(104, "can not get DCDC Rect Grp Data!", NULL, NULL);
            goto ERROR_4;
        }

        //provide general info
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", MAX_GRPPM_ROW);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 0);

        for(n = TYPE_SIG_RECT_DCDC_GRP1/* nSubPageIdx * nRowsPerPage */; n <= TYPE_SIG_RECT_DCDC_GRP4/* min(MAX_GRPPM_ROW, (nSubPageIdx + 1) * nRowsPerPage) */; n++)
        {
            pJSDCDCGrpPMInfo = cJSON_CreateObject();

            cJSON_AddNumberToObject(pJSDCDCGrpPMInfo, "grp", n + 1 - 8);

            cJSON_AddNumberToObject(pJSDCDCGrpPMInfo, "ol_num", rectGrpData.nOnlineRectNum[n]);

            snprintf(szGrpPMName, 64, "%.1fkW", rectGrpData.fRatedPower[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "rated_power", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRatedCurr[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "rated_curr", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRealTime_MaxCurr[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "max_curr", szGrpPMName);

            // LOG_TO_FILE("DCDC emCtrlStatus:[%d]", rectGrpData.emCtrlStatus[n]);

            switch (rectGrpData.emCtrlStatus[n])
            {
                case CTRL_STATUS_OFF:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6702));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_stats", szGrpPMName);
                    break;

                case CTRL_STATUS_RECTIFY_POSITIVE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9031));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_stats", szGrpPMName);
                    break;
                
                case CTRL_STATUS_INVERTER_REVERSE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9032));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_stats", szGrpPMName);
                    break;

                default:
                    printf("----ERRRRRR: ctrl_stats value is illegal----\n");
                    Make_Web_OutPut(104, "The 'ctrl_stats' value INVALID!", NULL, NULL);
                    goto ERROR_4;
            }

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fCtrlVolt[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_volt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fCtrlBattSideCutoffVolt[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_batt_cutoff_volt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fCtrlBattSideCurrLmt[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_batt_curr_lmt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fCtrlBusSideCurrLmt[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "ctrl_bus_curr_lmt", szGrpPMName);
            
            // LOG_TO_FILE("DCDC emRunningStatus:[%d]", rectGrpData.emRunningStatus[n]);

            switch (rectGrpData.emRunningStatus[n]+RUNNING_STATUS_DCDC_OFF)
            {
                case RUNNING_STATUS_DCDC_OFF:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6702));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_DCDC_POSITIVE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9031));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;
                
                case RUNNING_STATUS_DCDC_RESERVE:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9032));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_DCDC_FAULT:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6581));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;
                
                case RUNNING_STATUS_DCDC_ALARM:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6887));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_DCDC_INCONSISTENT:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9030));
                    cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                default:
                    printf("----ERRRRRR: run_stats value is illegal----\n");
                    Make_Web_OutPut(104, "The 'run_stats' value INVALID!", NULL, NULL);
                    goto ERROR_4;
            }

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fRealTime_BattSideVolt[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "rt_batt_volt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRealTime_BattSideTotalCurr[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "rt_batt_tol_curr", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fRealTime_BusSideVolt[n]);
            cJSON_AddStringToObject(pJSDCDCGrpPMInfo, "rt_bus_volt", szGrpPMName);

            cJSON_AddItemToArray(pJSDCDCGrpPMTotalInfo, pJSDCDCGrpPMInfo);
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSDCDCGrpPMTotalInfo);

        return TRUE;

ERROR_4:
        // 防止内存泄漏
        if(pJSDCDCGrpPMTotalInfo != NULL)
        {
            cJSON_Delete(pJSDCDCGrpPMTotalInfo);
        }
        if(pJSDCDCGrpPMInfo != NULL)
        {
            cJSON_Delete(pJSDCDCGrpPMInfo);
        }
        return TRUE;
    }

    if(nPageIdx == RUN_INFO_PAGEID_MPPT_GRPPM)   /*MPPT模块组运行数据*/
    {
        cJSON* pJSMPPTGrpPMTotalInfo = cJSON_CreateArray();
        cJSON* pJSMPPTGrpPMInfo = NULL;
        A_BLOB_RECT_GRP_DATA rectGrpData;

        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 204, sizeof(A_BLOB_RECT_GRP_DATA), (void*)&rectGrpData))
        {
            printf("----ERRRRRR: can not get MPPT Rect Grp Data----\n");
            Make_Web_OutPut(104, "can not get MPPT Rect Grp Data!", NULL, NULL);
            goto ERROR_5;
        }

        //provide general info
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", MAX_GRPPM_ROW);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
        // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 0);

        for(n = TYPE_SIG_RECT_MPPT_GRP1/* nSubPageIdx * nRowsPerPage */; n <= TYPE_SIG_RECT_MPPT_GRP4/* min(MAX_GRPPM_ROW, (nSubPageIdx + 1) * nRowsPerPage) */; n++)
        {
            pJSMPPTGrpPMInfo = cJSON_CreateObject();

            cJSON_AddNumberToObject(pJSMPPTGrpPMInfo, "grp", n + 1 - 4);

            cJSON_AddNumberToObject(pJSMPPTGrpPMInfo, "ol_num", rectGrpData.nOnlineRectNum[n]);

            snprintf(szGrpPMName, 64, "%.1fkW", rectGrpData.fRatedPower[n]);
            cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "rated_power", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRatedCurr[n]);
            cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "rated_curr", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fRealTime_MaxCurr[n]);
            cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "max_curr", szGrpPMName);

            // LOG_TO_FILE("MPPT emRunningStatus:[%d]", rectGrpData.emRunningStatus[n]);

            switch (rectGrpData.emRunningStatus[n]+RUNNING_STATUS_MPPT_OFF)
            {
                case RUNNING_STATUS_MPPT_OFF:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6702));
                    cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_MPPT_ON:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6703));
                    cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_MPPT_FAULT:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6581));
                    cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "run_stats", szGrpPMName);
                    break;
                
                case RUNNING_STATUS_MPPT_ALARM:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(6887));
                    cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                case RUNNING_STATUS_MPPT_INCONSISTENT:
                    snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(9030));
                    cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "run_stats", szGrpPMName);
                    break;

                default:
                    printf("----ERRRRRR: run_stats value is illegal----\n");
                    Make_Web_OutPut(104, "The 'run_stats' value INVALID!", NULL, NULL);
                    goto ERROR_5;
            }

            snprintf(szGrpPMName, 64, "%.1fV", rectGrpData.fSystemVolt[n]);
            cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "sys_volt", szGrpPMName);

            snprintf(szGrpPMName, 64, "%.1fA", rectGrpData.fSystemTotalCurr[n]);
            cJSON_AddStringToObject(pJSMPPTGrpPMInfo, "sys_tol_curr", szGrpPMName);

            cJSON_AddItemToArray(pJSMPPTGrpPMTotalInfo, pJSMPPTGrpPMInfo);
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSMPPTGrpPMTotalInfo);

        return TRUE;

ERROR_5:
        // 防止内存泄漏
        if(pJSMPPTGrpPMTotalInfo != NULL)
        {
            cJSON_Delete(pJSMPPTGrpPMTotalInfo);
        }
        if(pJSMPPTGrpPMInfo != NULL)
        {
            cJSON_Delete(pJSMPPTGrpPMInfo);
        }
        return TRUE;
    }

    if(nPageIdx == RUN_INFO_PAGEID_SINGLEPM)   /*单模块运行数据*/
    {
        A_BLOB_SINGLE_RECT_INFO allsingleRectData;
        cJSON* pJSSingleRectTotalInfo = cJSON_CreateArray();
        cJSON* pJSOnlineGrpInfo = cJSON_CreateObject();
        cJSON* pJSSingleRectInfo = NULL;
        char szSingleRectName[64] = "\0";
        // BOOL bfirstRectFlag = FALSE;
        // int nFirstRect = -1;

        char szSingleRectGrpName[MAX_RECT_TYPE][32];
        for(n = 0; n < MAX_RECT_TYPE - 1; n++)
        {
            memcpy(szSingleRectGrpName[n], LoadLangText_ById(1126 + n), 32);
        }
        memcpy(szSingleRectGrpName[12], "STS", 4);

        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
        {
            TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
            Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
            goto ERROR_6;
        }

        for(n = 0; n < MAX_RECT_TYPE; n++)
        {
            if(allsingleRectData.nOnlineNum[n] <= 0){continue;}
            // if(bfirstRectFlag == FALSE)
            // {
            //     nFirstRect = n;
            //     bfirstRectFlag = TRUE; 
            // }
            char szValueString[32] = "\0";
            snprintf(szValueString, 32, "%d", n);
            cJSON_AddStringToObject(pJSOnlineGrpInfo, szValueString, szSingleRectGrpName[n]);
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "grp_name_data", pJSOnlineGrpInfo);

        // int nSelectedGrpNow = 2;
        // if(nSelectedGrp == -1)
        // {
        //     if(nFirstRect != -1)
        //     {
        //         nSelectedGrpNow = nFirstRect;
        //     }
        //     else
        //     {
        //         Make_Web_OutPut(104, "No data for this page!", NULL, NULL);
        //         return TRUE;
        //     }
        // }
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "selected_grp", 2);

        A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 200, 200, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS), (void*)&blobCCUSett))
        {
            printf("----ERRRRRR: can not get CCU Set----\n");
            goto ERROR_6;
        }
        int GrpNum[MAX_RECT_TYPE];
        GrpNum[0] = blobCCUSett.ccuSettings.byNumGrp_1; //PCS
        GrpNum[1] = blobCCUSett.ccuSettings.byNumGrp_2;
        GrpNum[2] = blobCCUSett.ccuSettings.byNumGrp_3;
        GrpNum[3] = blobCCUSett.ccuSettings.byNumGrp_4;

        GrpNum[4] = blobCCUSett.ccuSettings.byNumGrp_5; //MPPT
        GrpNum[5] = blobCCUSett.ccuSettings.byNumGrp_6;
        GrpNum[6] = blobCCUSett.ccuSettings.byNumGrp_7;
        GrpNum[7] = blobCCUSett.ccuSettings.byNumGrp_8;



        WEB_PARAM_VALUE* pWebParaVal;
        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 500, 502);
        int nBatConfigNum =  pWebParaVal->currValue.ulValue;
        if(nBatConfigNum > MAX_BATTCABINET_NUM)
        {
            nBatConfigNum = MAX_BATTCABINET_NUM;
        }
        int j;
        for(j = 0; j < nBatConfigNum; ++j)
        {
            for(n = 0; n < GrpNum[j]; n++) //PCS 单体数据
            {
                pJSSingleRectInfo = cJSON_CreateObject();
                STU_SIGNLE_RECT_DATA singleRectData;
                singleRectData = allsingleRectData.arrRect[j][n];

                //make ids and strings out of raw data
                cJSON_AddNumberToObject(pJSSingleRectInfo, "grp", j);

                snprintf(szSingleRectName, 64, "%d", n + 1);
                cJSON_AddStringToObject(pJSSingleRectInfo, "addr", szSingleRectName);

                if(singleRectData.byCommStatu == 0)
                {
                    cJSON_AddStringToObject(pJSSingleRectInfo, "comm_status", LoadLangText_ById(34));
                }
                else
                {
                    cJSON_AddStringToObject(pJSSingleRectInfo, "comm_status", LoadLangText_ById(759));
                }

                snprintf(szSingleRectName, 64, "Psum:%.1fkW, Pabc:%.1f|%.1f|%.1fkW",
                        singleRectData.fPtotalActive,
                        singleRectData.fPphase_Active[0],
                        singleRectData.fPphase_Active[1],
                        singleRectData.fPphase_Active[2]);
                cJSON_AddStringToObject(pJSSingleRectInfo, "p", szSingleRectName);

                snprintf(szSingleRectName, 64, "Qsum:%.1fkVar, Qabc:%.1f|%.1f|%.1fkVar",
                        singleRectData.fPtotalReactive,
                        singleRectData.fPphase_Reactive[0],
                        singleRectData.fPphase_Reactive[1],
                        singleRectData.fPphase_Reactive[2]);
                cJSON_AddStringToObject(pJSSingleRectInfo, "q", szSingleRectName);

                snprintf(szSingleRectName, 64, "Uabc:%.1f|%.1f|%.1fV, Iabc:%.1f|%.1f|%.1fA",
                        singleRectData.fPh_InputVolt[0], singleRectData.fPh_InputVolt[1], singleRectData.fPh_InputVolt[2],
                        singleRectData.fPh_InputCurr[0], singleRectData.fPh_InputCurr[1], singleRectData.fPh_InputCurr[2]);
                cJSON_AddStringToObject(pJSSingleRectInfo, "in_ui", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.1fV|%.1fA", singleRectData.fRTOutputVolt, singleRectData.fRTOutputCurr);
                cJSON_AddStringToObject(pJSSingleRectInfo, "out_ui", szSingleRectName);

                // snprintf(szSingleRectName, 64, "%.0f\u2103", singleRectData.fAmbit_Temp);
                snprintf(szSingleRectName, 64, "%.1f\u2103", singleRectData.fAmbit_Temp);
                cJSON_AddStringToObject(pJSSingleRectInfo, "temp", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.1fHz", singleRectData.fAC_Freq);
                cJSON_AddStringToObject(pJSSingleRectInfo, "freq", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
                cJSON_AddStringToObject(pJSSingleRectInfo, "pfc_dcdc_ver", szSingleRectName);

                // snprintf(szSingleRectName, 64, "%12s|%12s|%12s",
                //         singleRectData.byRectErrCode,
                //         singleRectData.byInvertErrCode,
                //         singleRectData.byPCSFaultCode);
                // cJSON_AddStringToObject(pJSSingleRectInfo, "err_code", szSingleRectName);
                snprintf(szSingleRectName, 64, "%02X%02X%02X%02X|%02X%02X%02X%02X|%02X%02X%02X%02X|%02X%02X%02X%02X",
                        (BYTE)(singleRectData.unioInverterStatus_1_1.uiInverterStatus ),
                        (BYTE)(singleRectData.unioInverterStatus_1_1.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_1_2.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_1_2.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_2_1.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_2_1.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_2_2.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_2_2.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_3_1.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_3_1.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_3_2.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_3_2.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_4_1.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_4_1.uiInverterStatus>>8),
                        (BYTE)(singleRectData.unioInverterStatus_4_2.uiInverterStatus),
                        (BYTE)(singleRectData.unioInverterStatus_4_2.uiInverterStatus>>8)
                    );
                cJSON_AddStringToObject(pJSSingleRectInfo, "err_code", szSingleRectName);

                snprintf(szSingleRectName, 64, "%s", singleRectData.byBarCode);
                cJSON_AddStringToObject(pJSSingleRectInfo, "bar_code", szSingleRectName);

                cJSON_AddItemToArray(pJSSingleRectTotalInfo, pJSSingleRectInfo);
            } 
            for(n = 0; n < GrpNum[TYPE_SIG_RECT_MPPT_GRP1 + j]; n++) //MPPT 单体数据
            {
                cJSON* pJSSingleRectInfo = cJSON_CreateObject();
                STU_SIGNLE_RECT_DATA singleRectData;
                singleRectData = allsingleRectData.arrRect[j][n];

                //make ids and strings out of raw data
                cJSON_AddNumberToObject(pJSSingleRectInfo, "grp", j);

                snprintf(szSingleRectName, 64, "%d", n + 1);
                cJSON_AddStringToObject(pJSSingleRectInfo, "addr", szSingleRectName);

                if(singleRectData.byCommStatu == 0)
                {
                    cJSON_AddStringToObject(pJSSingleRectInfo, "comm_status", LoadLangText_ById(34));
                }
                else
                {
                    cJSON_AddStringToObject(pJSSingleRectInfo, "comm_status", LoadLangText_ById(759));
                }

                snprintf(szSingleRectName, 64, "Psum:%.1fkW, Pabc:%.1f|%.1f|%.1fkW",
                        singleRectData.fPtotalActive,
                        singleRectData.fPphase_Active[0],
                        singleRectData.fPphase_Active[1],
                        singleRectData.fPphase_Active[2]);
                cJSON_AddStringToObject(pJSSingleRectInfo, "p", szSingleRectName);

                snprintf(szSingleRectName, 64, "Qsum:%.1fkVar, Qabc:%.1f|%.1f|%.1fkVar",
                        singleRectData.fPtotalReactive,
                        singleRectData.fPphase_Reactive[0],
                        singleRectData.fPphase_Reactive[1],
                        singleRectData.fPphase_Reactive[2]);
                cJSON_AddStringToObject(pJSSingleRectInfo, "q", szSingleRectName);

                snprintf(szSingleRectName, 64, "Uabc:%.1f|%.1f|%.1fV, Iabc:%.1f|%.1f|%.1fA",
                        singleRectData.fPh_InputVolt[0], singleRectData.fPh_InputVolt[1], singleRectData.fPh_InputVolt[2],
                        singleRectData.fPh_InputCurr[0], singleRectData.fPh_InputCurr[1], singleRectData.fPh_InputCurr[2]);
                cJSON_AddStringToObject(pJSSingleRectInfo, "in_ui", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.1fV|%.1fA", singleRectData.fRTOutputVolt, singleRectData.fRTOutputCurr);
                cJSON_AddStringToObject(pJSSingleRectInfo, "out_ui", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.0f\u2103", singleRectData.fAmbit_Temp);
                cJSON_AddStringToObject(pJSSingleRectInfo, "temp", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.1fHz", singleRectData.fAC_Freq);
                cJSON_AddStringToObject(pJSSingleRectInfo, "freq", szSingleRectName);

                snprintf(szSingleRectName, 64, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
                cJSON_AddStringToObject(pJSSingleRectInfo, "pfc_dcdc_ver", szSingleRectName);

                snprintf(szSingleRectName, 64, "%12s|%12s|%12s",
                        singleRectData.byRectErrCode,
                        singleRectData.byInvertErrCode,
                        singleRectData.byPCSFaultCode);
                cJSON_AddStringToObject(pJSSingleRectInfo, "err_code", szSingleRectName);

                snprintf(szSingleRectName, 64, "%s", singleRectData.byBarCode);
                cJSON_AddStringToObject(pJSSingleRectInfo, "bar_code", szSingleRectName);

                cJSON_AddItemToArray(pJSSingleRectTotalInfo, pJSSingleRectInfo);
            }       
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSSingleRectTotalInfo);

        return TRUE;

ERROR_6:
        // 防止内存泄漏
        if(pJSSingleRectTotalInfo != NULL)
        {
            cJSON_Delete(pJSSingleRectTotalInfo);
        }
        if(pJSSingleRectInfo != NULL)
        {
            cJSON_Delete(pJSSingleRectInfo);
        }
        if(pJSOnlineGrpInfo != NULL)
        {
            cJSON_Delete(pJSOnlineGrpInfo);
        }
        return TRUE;
    }

    return TRUE;
}

BOOL webpage_handler_run_info(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----RUN_INFO page_handler was called!---\n");

    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nPageIdx = -1;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nPageIdx))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nPageIdx < 0) || (nPageIdx >= RUN_INFO_PAGEID_MAX))
        {
            Make_Web_OutPut(104, "The 'type' value INVALID!", NULL, NULL);
            return FALSE;  
        }

        // int nSubPageIdx = -1;
        // int nRowsPerPage = -1;      //how many rows per page
        // int nSelectedGrp = -1;

        // if(nPageIdx == 0)
        // {
        //     if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pageId=", &nSubPageIdx))
        //     {
        //         Make_Web_OutPut(104, "The 'pageId'(sub page id) NOT provided!", NULL, NULL);
        //         return FALSE;
        //     }

        //     if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "page_vol=", &nRowsPerPage))
        //     {
        //         Make_Web_OutPut(104, "The 'page_vol'(rows per page) NOT provided!", NULL, NULL);
        //         return FALSE;
        //     }
        // }

        // if(nPageIdx == 2)
        // {
        //     if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "grp_id=", &nSelectedGrp))
        //     {
        //         Make_Web_OutPut(104, "The 'grp_id'(selected group id) NOT provided!", NULL, NULL);
        //         return FALSE;
        //     }
        // }
        // return TRUE;
        return Handle_RunInfo_Data(pFCGI_Rough, nPageIdx/* , nSubPageIdx, nRowsPerPage, nSelectedGrp */);
    }
    else
    {
        Make_Web_OutPut(104, "Run_Info page does NOT allow POST!", NULL, NULL);
        return FALSE;  
    }
    
    return FALSE;
}