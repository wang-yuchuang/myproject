//#define _TRACE_PRINT_
#include "../fcgi_defs.h"

static char* GetParam_EnumString_FromVal(WEB_PARAM_VALUE* pWebParaVal, OUT int* pnEnumResId)
{
    BYTE nEnumVal = (BYTE)pWebParaVal->currValue.emValue;    
    if((pWebParaVal->m_emValueAttribute & WEB_PARAM_MASK_VALID) != WEB_PARAM_MASK_VALID)
    {
        return NULL;
    }
    int n;
    WEB_ENUM_ITEM* pEnumIt = pWebParaVal->pParamInfo->enumList;
    for( n = 0; n < pWebParaVal->pParamInfo->nEnumTotal; n++, pEnumIt++)
    {
        if(pEnumIt->emVal == nEnumVal)
        {
            if(pnEnumResId != NULL)
            {
                *pnEnumResId = pEnumIt->nEnumName_ResId;
            }
            return LoadLangText_ById(pEnumIt->nEnumName_ResId);
        }
    }

    return NULL;
}


static int Get_ActiveAlarm_TotalInRun(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int nTotal = 0;
    int n;
    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nActiveAlmTotal; n++)
    {
        if(pFCGI_Rough->chargerDataRepo.activeAlms[n].tmLastCeased == 0)
        {
            nTotal++;
        }
    }
    return nTotal;
}


static cJSON* Make_ActiveAlmTable_Json(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pActAlmTable, cJSON* pDict)
{
    int n;
    WEB_ALARM_VALUE* pWebAlmVal = pFCGI_Rough->chargerDataRepo.activeAlms;
    int nLevelRes, nAlmStatusRes;
    char szAlmDiscripr[64];
    int nAlmStatus; //0 - lasting, 1- end
    float fAlmLastMin;
    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nActiveAlmTotal; n++, pWebAlmVal++)
    {
        cJSON* pJAlmItem = cJSON_CreateObject();
        //level/level_res
        cJSON_AddNumberToObject(pJAlmItem, "id", n + 1);
        cJSON_AddNumberToObject(pJAlmItem, "level", pWebAlmVal->pAlarmInfo->emLevel - 1);
        nLevelRes = (pWebAlmVal->pAlarmInfo->emLevel == ALARM_LEVEL_OBSERVATION) ? 4 :
            ((pWebAlmVal->pAlarmInfo->emLevel == ALARM_LEVEL_MAJOR) ? 5 : 6);
        cJSON_AddNumberToObject(pJAlmItem, "level_res", nLevelRes);
        Add_Trans_Into_Dictionary(pDict, nLevelRes); //do not forget add the trans=>dictionary
        //alarm_description
        snprintf(szAlmDiscripr, 64, "[%s]: %s", LoadLangText_ById(pWebAlmVal->pBelongDev->nDevName_ResId),
            LoadLangText_ById(pWebAlmVal->pAlarmInfo->nAlmName_ResId));
        cJSON_AddStringToObject(pJAlmItem, "alm_description", szAlmDiscripr);
        //begin time
        cJSON_AddStringToObject(pJAlmItem, "begin_time", 
                TimeToString(Conv_UTC_2LocalTime(pWebAlmVal->tmLastOccured), TIME_CHN_FMT, szAlmDiscripr, 32));
        //alm status
        nAlmStatus = (pWebAlmVal->tmLastCeased == 0) ? 0 : 1;
        cJSON_AddNumberToObject(pJAlmItem, "alm_status", nAlmStatus);
        nAlmStatusRes = (nAlmStatus == 0) ? 368 : 369;
        Add_Trans_Into_Dictionary(pDict, nAlmStatusRes); //do not forget add the trans=>dictionary
        cJSON_AddNumberToObject(pJAlmItem, "alm_status_res", nAlmStatusRes);
        //alm last min
        fAlmLastMin = (float)(((nAlmStatus == 1) ? pWebAlmVal->tmLastCeased : _NOW_) - 
            pWebAlmVal->tmLastOccured) / 60.0;
        cJSON_AddFloatToObject(pJAlmItem, "alm_last", fAlmLastMin, 1);
        //now add back the item into Json Array
        cJSON_AddItemToArray(pActAlmTable, pJAlmItem);
    }

    return pActAlmTable;
}


static BOOL Handle_Home_PageData(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    printf("\n----now handling home_page data----\n\n");
    TRACEX("\n----now handling home_page data----\n\n");
    WEB_PARAM_VALUE* pWebParaVal;
    char* szStringVal;
    int n;
    int m;

    //get system run info
    A_BLOB_ES_RUN_DATA esRunData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 80, sizeof(A_BLOB_ES_RUN_DATA), (void*)&esRunData))
    {
        TRACEX("----ERRRRRR: can not get ES RunData----\n");
        return TRUE;
    }

    //get single rect info
    A_BLOB_SINGLE_RECT_INFO allsingleRectData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
    {
        TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
        Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
        return TRUE;
    }

    //Topology begins
    cJSON* pJSPcsTopologyTotalInfo = cJSON_CreateArray();
    int nRectOnOff = 0; //0-off, 1-on
    int nRectEnergyCtrl = 0; //0-DC Adjust, 1-AC Adjust
    int nRectEnergyMode = 0; //0-Stable Current, 1-Stable Volt, 2-Stable Power
    float fTotalGridPower = 0.0;

    for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
    {
        for(m = 0; m < MAX_SINGLE_RECT_NUM; m++)
        {
            STU_SIGNLE_RECT_DATA    singleRectData;
            singleRectData = allsingleRectData.arrRect[n][m];

            if(singleRectData.bExist == 0){continue;}

            //for energy flow
            cJSON* pJSPcsTopologyInfo = cJSON_CreateObject();

            //status light
            if(esRunData.mapPCSSysStatu[n].SysStatu.Byte0_Bit0to2 == 0 || esRunData.mapPCSSysStatu[n].SysStatu.Byte0_Bit0to2 == 2) //grey for power off/wait
            {
                cJSON_AddNumberToObject(pJSPcsTopologyInfo, "type", 2);
            }

            else if(esRunData.mapPCSSysStatu[n].SysStatu.Byte0_Bit0to2 == 3 || esRunData.mapPCSSysStatu[n].SysStatu.Byte0_Bit0to2 == 4) //red for alarm
            {
                cJSON_AddNumberToObject(pJSPcsTopologyInfo, "type", 1);
                
            }

            else if(esRunData.mapPCSSysStatu[n].SysStatu.Byte0_Bit0to2 == 1) //green for running
            {
                cJSON_AddNumberToObject(pJSPcsTopologyInfo, "type", 0);
                
            }

            //energy flow
            if(esRunData.emACDC_WorkStatu[n] == 1)
            {
                cJSON_AddNumberToObject(pJSPcsTopologyInfo, "flow", 2);
                fTotalGridPower += singleRectData.fPtotalActive;
            }

            else if(esRunData.emACDC_WorkStatu[n] == 2)
            {
                cJSON_AddNumberToObject(pJSPcsTopologyInfo, "flow", 1);
                fTotalGridPower -= singleRectData.fPtotalActive;
            }

            else
            {
                cJSON_AddNumberToObject(pJSPcsTopologyInfo, "flow", 0);
            }

            //for param quick setting
            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 251 + n, 1110, 4, (void*)&nRectOnOff))
            {
                TRACEX("----ERRRRRR: can not get Rect On/Off Specific Settings----\n");
            }
            cJSON_AddNumberToObject(pJSPcsTopologyInfo, "onoff", nRectOnOff);

            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 251 + n, 3111, 4, (void*)&nRectEnergyCtrl))
            {
                TRACEX("----ERRRRRR: can not get Rect Energy Ctrl Specific Settings----\n");
            }
            cJSON_AddNumberToObject(pJSPcsTopologyInfo, "powerValue", nRectEnergyCtrl);

            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 251 + n, 3112, 4, (void*)&nRectEnergyMode))
            {
                TRACEX("----ERRRRRR: can not get Rect Energy Mode Specific Settings----\n");
            }
            cJSON_AddNumberToObject(pJSPcsTopologyInfo, "electricityValue", nRectEnergyMode);

            //for run info data
            cJSON* pJSPcsTotalRunInfo = cJSON_CreateArray();
            int nKey = 1;
            char szStringValue[64] = "\0";

            cJSON* pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(148));
            // if(pFCGI_Rough->emSystemType == SYSTYPE_SCADA_INPCS235KW)
            // {
            //     snprintf(szStringValue, 64, "%.1fV", singleRectData.fDCsideVoltage);
            // }
            // else
            {
                snprintf(szStringValue, 64, "%.1fV", singleRectData.fRTOutputVolt);
            }
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(149));
            // if(pFCGI_Rough->emSystemType == SYSTYPE_SCADA_INPCS235KW)
            // {
            //     snprintf(szStringValue, 64, "%.1fA", singleRectData.fDCsideCurrent);
            // }
            // else
            {
                snprintf(szStringValue, 64, "%.1fA", singleRectData.fRTOutputCurr);
            }
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6874));
            snprintf(szStringValue, 64, "%.1fkW", singleRectData.fRTOutputVolt * singleRectData.fRTOutputCurr / 1000);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            //-d未上传，暂不显示
            // pJSPcsRunInfoRow = cJSON_CreateObject();
            // cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            // cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            // cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6875));
            // snprintf(szStringValue, 64, "");
            // cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            // cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            // nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(1920));
            if(singleRectData.fRTOutputVolt * singleRectData.fRTOutputCurr / 1000 < 0.1)
            {
                snprintf(szStringValue, 64, LoadLangText_ById(91));
            }
            else if(singleRectData.emWorkMode == 0) //rectify
            {
                snprintf(szStringValue, 64, LoadLangText_ById(709));
            }
            else if(singleRectData.emWorkMode == 1) //invert
            {
                snprintf(szStringValue, 64, LoadLangText_ById(710));
            }
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(143));
            snprintf(szStringValue, 64, "%.1fHz", singleRectData.fAC_Freq);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6135));
            snprintf(szStringValue, 64, "%.1f\u2103", singleRectData.fAmbit_Temp);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            //-d未上传，暂不显示
            // pJSPcsRunInfoRow = cJSON_CreateObject();
            // cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            // cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1702));
            // cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", "散热器温度值");
            // snprintf(szStringValue, 64, "");
            // cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            // cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            // nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1701));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6876));
            snprintf(szStringValue, 64, "%.1fkW", singleRectData.fPtotalActive);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1701));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6877));
            snprintf(szStringValue, 64, "%.1fkVar", singleRectData.fPtotalReactive);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1701));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6878));
            snprintf(szStringValue, 64, "%.1fkVar", singleRectData.fPTotalApparent);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(1701));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(142));
            snprintf(szStringValue, 64, "%.2f", singleRectData.fPtotalFactor);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6872));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6879));
            snprintf(szStringValue, 64, "%s: %.1fV %s: %.1fV %s: %.1fV", 
                LoadLangText_ById(6881), singleRectData.fPh_InputVolt[0],
                LoadLangText_ById(6882), singleRectData.fPh_InputVolt[1],
                LoadLangText_ById(6883), singleRectData.fPh_InputVolt[2]);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6872));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6880));
            snprintf(szStringValue, 64, "%s: %.1fA %s: %.1fA %s: %.1fA", 
                LoadLangText_ById(6881), singleRectData.fPh_InputCurr[0],
                LoadLangText_ById(6882), singleRectData.fPh_InputCurr[1],
                LoadLangText_ById(6883), singleRectData.fPh_InputCurr[2]);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6872));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6884));
            snprintf(szStringValue, 64, "%s: %.1fkW %s: %.1fkW %s: %.1fkW", 
                LoadLangText_ById(6881), singleRectData.fPphase_Active[0],
                LoadLangText_ById(6882), singleRectData.fPphase_Active[1],
                LoadLangText_ById(6883), singleRectData.fPphase_Active[2]);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6872));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6885));
            snprintf(szStringValue, 64, "%s: %.1fkVar %s: %.1fkVar %s: %.1fkVar", 
                LoadLangText_ById(6881), singleRectData.fPphase_Reactive[0],
                LoadLangText_ById(6882), singleRectData.fPphase_Reactive[1],
                LoadLangText_ById(6883), singleRectData.fPphase_Reactive[2]);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6872));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6898));
            snprintf(szStringValue, 64, "%s: %.1fkVar %s: %.1fkVar %s: %.1fkVar", 
                LoadLangText_ById(6881), singleRectData.fPphase_Apparent[0],
                LoadLangText_ById(6882), singleRectData.fPphase_Apparent[1],
                LoadLangText_ById(6883), singleRectData.fPphase_Apparent[2]);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6872));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(6899));
            snprintf(szStringValue, 64, "%s: %.1fkVar %s: %.1fkVar %s: %.1fkVar", 
                LoadLangText_ById(6881), singleRectData.fPphase_Factor[0],
                LoadLangText_ById(6882), singleRectData.fPphase_Factor[1],
                LoadLangText_ById(6883), singleRectData.fPphase_Factor[2]);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6873));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(469));
            snprintf(szStringValue, 64, "%.3f", singleRectData.fRect_PFCVer);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6873));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(468));
            snprintf(szStringValue, 64, "%.3f", singleRectData.fRect_DCDCVer);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            pJSPcsRunInfoRow = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPcsRunInfoRow, "key", nKey);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "type", LoadLangText_ById(6873));
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "name", LoadLangText_ById(315));
            snprintf(szStringValue, 64, "%s", singleRectData.byBarCode);
            cJSON_AddStringToObject(pJSPcsRunInfoRow, "value", szStringValue);
            cJSON_AddItemToArray(pJSPcsTotalRunInfo, pJSPcsRunInfoRow);
            nKey++;

            cJSON_AddItemToObject(pJSPcsTopologyInfo, "tableData", pJSPcsTotalRunInfo);

            cJSON_AddItemToArray(pJSPcsTopologyTotalInfo, pJSPcsTopologyInfo);
        }
    }

    if(fTotalGridPower > 0.1)
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "all", 2);
    }

    else if(fTotalGridPower < -0.1)
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "all", 1);
    }

    else
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "all", 0);
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "pcsinfo", pJSPcsTopologyTotalInfo);

    return TRUE;
}


static BOOL Handle_Home_GeneralData(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----now handling home_general data----\n\n");
    WEB_PARAM_VALUE* pWebParaVal;
    char* szStringVal;
    char szTime[32];
    int n;
    A_BLOB_NETWORKINFO* pNetWorkInfo;
    //dictionary
    //cJSON* pDictionary = cJSON_CreateArray();
    cJSON* pDictionary = cJSON_CreateObject();
    cJSON_AddStringToObject(pDictionary, "OA", LoadLangText_ById(4));     //OA
    cJSON_AddStringToObject(pDictionary, "MA", LoadLangText_ById(5));     //MA
    cJSON_AddStringToObject(pDictionary, "CA", LoadLangText_ById(6));     //CA
    cJSON_AddStringToObject(pDictionary, "ALL", LoadLangText_ById(998));  //ALL
    //sys_runstatus
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 1, 4)) == NULL)
    {
        Make_Web_OutPut(104, "The 'sys_runstatus' NOT found!", NULL, NULL);
        return FALSE;
    }
    szStringVal = GetParam_EnumString_FromVal(pWebParaVal, NULL);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sys_runstatus", pWebParaVal->currValue.emValue);
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "sys_runstatus_str", 
        (szStringVal == NULL) ? "--" : szStringVal);
    //sys_alarmstat/total
    int nActiveAlmTotal = Get_ActiveAlarm_TotalInRun(pFCGI_Rough);
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "sys_alarmstat", 
        (nActiveAlmTotal < 1) ? LoadLangText_ById(108) : LoadLangText_ById(201));
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sys_alarm_total", nActiveAlmTotal);
    //active alarm table
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "alarm_list", 
        Make_ActiveAlmTable_Json(pFCGI_Rough, cJSON_CreateArray(), pDictionary));
    //sys_date_time
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "sys_datetime", 
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT, szTime, 32));
    //sever_conn_status/status_str
    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 1, 5)) == NULL)
    {
        Make_Web_OutPut(104, "The 'sever_conn_status' NOT found!", NULL, NULL);
        return FALSE;
    }
    TRACED("Get 1, 5 = [%d]",pWebParaVal->currValue.emValue);

    szStringVal = GetParam_EnumString_FromVal(pWebParaVal, NULL);
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "sever_conn_stat_str", 
        (szStringVal == NULL) ? LoadLangText_ById(201) : szStringVal);
    TRACEI("sever_conn_stat_str=[%s]",szStringVal);

    if(pWebParaVal->currValue.emValue > 0)
    {
        //disconnected
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sever_conn_status", 0);
        TRACEW("sever_conn_status == 0");
    }
    else
    {
        BOOL bGetNetWorkOK = FALSE;
        if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 1, 3)) != NULL)
        {
            if(pWebParaVal->pParamInfo->nValueSize == sizeof(A_BLOB_NETWORKINFO))
            {
                bGetNetWorkOK = TRUE;
                pNetWorkInfo = (A_BLOB_NETWORKINFO*)pWebParaVal->currValue.blobValue;
            }
        }
        if(!bGetNetWorkOK)
        {
            //Make_Web_OutPut(104, "Get network info FAILED!", NULL, NULL);
            //return FALSE;     
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sever_conn_status", 0);       
        }
        else
        {
            TRACED("Get netInfo_usb0.emNetPort_Status [%d]",pNetWorkInfo->netInfo_usb0.emNetPort_Status);
            //for wireless/wired
            if(pNetWorkInfo->netInfo_usb0.emNetPort_Status == NETPORT_STATUS_CONNECTED)
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sever_conn_status", 2);
                //will parse signal type/level later     
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_type", 3);  
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_level", 5);
                TRACEW("sever_conn_status = 2; signal_type = 3; signal_level = 5");
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sever_conn_status", 1);
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_level", 0);   
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_type", 0);  
                TRACEW("sever_conn_status = 1; signal_type = 0; signal_level = 0");
            }     
        }
    }


    //---  Tab or Hint DataInfo ---//
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "topology", LoadLangText_ById(2070));
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "scene_graph", LoadLangText_ById(2072));
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "energy_stack", LoadLangText_ById(2373));
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "no_device", LoadLangText_ById(84));
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "off_grid", LoadLangText_ById(2073));


    //---  System Type DataInfo ---//
    int nSystemType = 0;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 8, 4, (void*)&nSystemType))
    {
        TRACEX("----ERRRRRR: can not get System Type DataInfo----\n");
        return TRUE;
    }
    else
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "system_type", nSystemType);
    }


    //---  Stat Graph Types DataInfo ---//
    cJSON* pJSStatGraphTypesTotalInfo = cJSON_CreateArray();

    cJSON* pJSStatGraphTypesInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSStatGraphTypesInfo, "name", LoadLangText_ById(826));
    cJSON_AddNumberToObject(pJSStatGraphTypesInfo, "val", 0);
    cJSON_AddItemToArray(pJSStatGraphTypesTotalInfo, pJSStatGraphTypesInfo);

    pJSStatGraphTypesInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSStatGraphTypesInfo, "name", LoadLangText_ById(827));
    cJSON_AddNumberToObject(pJSStatGraphTypesInfo, "val", 1);
    cJSON_AddItemToArray(pJSStatGraphTypesTotalInfo, pJSStatGraphTypesInfo);

    pJSStatGraphTypesInfo = cJSON_CreateObject();
    cJSON_AddStringToObject(pJSStatGraphTypesInfo, "name", LoadLangText_ById(830));
    cJSON_AddNumberToObject(pJSStatGraphTypesInfo, "val", 2);
    cJSON_AddItemToArray(pJSStatGraphTypesTotalInfo, pJSStatGraphTypesInfo);

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "types", pJSStatGraphTypesTotalInfo);


    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dictionary", pDictionary);
    return TRUE;
}


static BOOL Parse_and_Handle_Home_Page_QuickSet(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON* pJsonSet = cJSON_Parse(pFCGI_Rough->szBodyRaw);
    if(pJsonSet == NULL)
    {
        TRACEX("--The ParamSet string is not a JSON->[%s]----\n", pFCGI_Rough->szBodyRaw);
        Make_Web_OutPut(104, "The 'param to be set' is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }

    cJSON* temp;

    //parse pcs id
    if((temp = cJSON_GetObjectItem(pJsonSet, "pcs")) == NULL)
    {
        Make_Web_OutPut(104, "The 'pcs' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
}


BOOL webpage_handler_pcs_home(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----HOME page_handler was called!---\n");

    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        return Handle_Home_PageData(pFCGI_Rough);
    }
    else
    {
        //now need parse amd handle Home page quieck set data
        return Parse_and_Handle_Home_Page_QuickSet(pFCGI_Rough);
    }
    
    return FALSE;
}

