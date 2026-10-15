// #define _TRACE_PRINT_
#include "../fcgi_defs.h"

#define MAX_STAT_CURVE_POINTS   31
#define STAT_TYPE_BY_MONTH       0
#define STAT_TYPE_BY_WEEK        1
#define STAT_TYPE_BY_HOUR        2
#define DATE_TYPE_PER_ALL        0
#define DATE_TYPE_PER_THIS       1
#define DATE_TYPE_PER_LAST       2

#define MAX_TABLE_ROW            7

typedef enum tagHomePageId
{
    HOME_PAGEID_TOPOLOGY = 0,
    HOME_PAGEID_ENERGYFLOW,
    HOME_PAGEID_SCENE,
    HOME_PAGEID_ONOFFGRID,

    HOME_PAGEID_MAX
}HOME_PAGEIDS;

typedef enum tagESFlowStatus
{
    ESFLOW_STAT_OFF = 0,
    ESFLOW_STAT_POS,
    ESFLOW_STAT_NEG
}ES_FLOW_STATUS;

typedef enum tagStatisticGraphId
{
    STAT_GRAPH_ID_ENERGY_STAT = 0,
    STAT_GRAPH_ID_ENERGY_TREND,
    STAT_GRAPH_ID_BATT_SOC,
    STAT_GRAPH_ID_TEMP_CURVE,
    STAT_GRAPH_ID_EFF_CURVE,
    STAT_GRAPH_ID_SIN_CELL,//edit by Kuangyue 20241107

    STAT_GRAPH_ID_MAX
}STAT_GRAPH_IDS;

typedef enum tagStatisticDevTypes
{
    STAT_DEV_TYPE_AC = 0,
    STAT_DEV_TYPE_MPPT1,
    STAT_DEV_TYPE_MPPT2,
    STAT_DEV_TYPE_BATT1_CHRG,
    STAT_DEV_TYPE_BATT1_DISCHRG,
    STAT_DEV_TYPE_BATT2_CHRG,
    STAT_DEV_TYPE_BATT2_DISCHRG,

    STAT_DEV_TYPE_MAX
}STAT_DEV_TYPES;

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

static int Get_AlarmCA_TotalInRun(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int nTotal = 0;
    int n;
    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nActiveAlmTotal; n++)
    {
        if(pFCGI_Rough->chargerDataRepo.activeAlms[n].pAlarmInfo->emLevel == ALARM_LEVEL_CRITICAL)
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


static BOOL Get_GunTypeAndNameAndStatus_ById(FCGI_ROUGH_DATA* pFCGI_Rough, IN int nGunIdx, OUT int* pnType, OUT char* pszName, OUT int* pnStatus, OUT WEB_PARAM_VALUE** ppWebGunStatParam)
{
    return FALSE;
    /*if((nGunIdx < 0) || (nGunIdx >= MAX_SUPPORT_GUN_NUM))
    {
        return FALSE;
    }*/
    /*
    //get status param
    WEB_PARAM_VALUE* pWParVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 201 + nGunIdx, 101);
    if(pWParVal == NULL)
    {
        TRACEX("-ERRRRRR--1\n");
        return FALSE;
    }
    *ppWebGunStatParam = pWParVal;
    int nGunRawStatus = pWParVal->currValue.emValue;
    if(nGunRawStatus == GUN_STATE_NOT_INSTALL)
    {
        TRACEX("-ERRRRRR--2\n");
        return FALSE;
    }
    switch (nGunRawStatus)
    {
    case GUN_STATE_IDLE:
    case GUN_STATE_CHARGE_FINISHED:
        *pnStatus = 0;
        break;
    case GUN_STATE_LOCKED:
        *pnStatus = 3; 
        break;
    case GUN_STATE_RESERVE_PENDING:
        *pnStatus = 4;   
        break;
    case GUN_STATE_WAITING_START:
        *pnStatus = 5; //waiting status should be added on Web later
        break;
    case GUN_STATE_HANDSHAKE:
    case GUN_STATE_PREPARE_4CHARGE:
    case GUN_STATE_CHARGING:
    case GUN_STATE_CHARGE_PAUSED:
    case GUN_STATE_WAITING_END:
        *pnStatus = 1;   
        break;
    default:
        *pnStatus = 2;  
        break;
    }


    int nSysCfg = CP_TYPE_CCS_CCS;//blobChgSet.emSystemConfiguration;
    VAR_VALUE varV;
    Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 23, 4, (void*)varV.abyValue);
    BOOL bIsCCS1 = (varV.emValue > 0) ? TRUE : FALSE;

    switch (nSysCfg)
    {
    case CP_TYPE_CCS_CHADEMO_AC:
        if(nGunIdx == 0)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC1, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 1)
        {
            *pnType = 1;
            sprintf(pszName, "DC2, CHAdEMo");
        }
        else if(nGunIdx == 2)
        {
            *pnType = 3;
            sprintf(pszName, "AC, Type-2");
        }
        else
        {
            return FALSE;
        }        
        break;	
    case CP_TYPE_CCS_CHADEMO:	
        if(nGunIdx == 0)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC1, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 1)
        {
            *pnType = 1;
            sprintf(pszName, "DC2, CHAdEMo");
        }
        else
        {
            return FALSE;
        }        
        break;		
    case CP_TYPE_CCS_CCS:	
        if(nGunIdx == 0)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC1, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 1)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC2, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 2)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC3, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 3)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC4, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else
        {
            return FALSE;
        }        
        break;			
    case CP_TYPE_CHADEMO_SINGLE:	
        if(nGunIdx == 0)
        {
            *pnType = 1;
            sprintf(pszName, "DC1, CHAdEMo");
        }
        else
        {
            return FALSE;
        }        
        break;	
    case CP_TYPE_CCS_SINGLE:
        if(nGunIdx == 0)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC1, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else
        {
            return FALSE;
        }        
        break;			
    case CP_TYPE_GBT_CHADEMO:	
        if(nGunIdx == 0)
        {
            *pnType = 2;
            sprintf(pszName, "DC1, GBT");
        }
        else if(nGunIdx == 1)
        {
            *pnType = 1;
            sprintf(pszName, "DC2, CHAdEMo");
        }
        else
        {
            return FALSE;
        }        
        break;		
    case CP_TYPE_GBT_CCS:	
        if(nGunIdx == 0)
        {
            *pnType = 2;
            sprintf(pszName, "DC1, GBT");
        }
        else if(nGunIdx == 1)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC2, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else
        {
            return FALSE;
        }        
        break;			
    case CP_TYPE_CCS_AC:	
        if(nGunIdx == 0)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC1, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 1)
        {
            *pnType = 3;
            sprintf(pszName, "AC, Type-2");
        }
        else
        {
            return FALSE;
        }        
        break;			
    case CP_TYPE_GBT_CHADEMO_AC:	
        if(nGunIdx == 0)
        {
            *pnType = 2;
            sprintf(pszName, "DC1, GBT");
        }
        else if(nGunIdx == 1)
        {
            *pnType = 1;
            sprintf(pszName, "DC2, CHAdEMo");
        }
        else
        {
            return FALSE;
        }        
        break;		
    case CP_TYPE_GBT_CCS_AC:	
        if(nGunIdx == 0)
        {
            *pnType = 2;
            sprintf(pszName, "DC1, GBT");
        }
        else if(nGunIdx == 1)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC2, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 2)
        {
            *pnType = 3;
            sprintf(pszName, "AC, Type-2");
        }
        else
        {
            return FALSE;
        }        
        break;	
    case CP_TYPE_CCS_CCS_AC:	
        if(nGunIdx == 0)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC1, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 1)
        {
            *pnType = bIsCCS1 ? 0 : 4;
            sprintf(pszName, "DC2, %s", bIsCCS1 ? "CCS-1" : "CCS-2");
        }
        else if(nGunIdx == 2)
        {
            *pnType = 3;
            sprintf(pszName, "AC, Type-2");
        }
        else
        {
            return FALSE;
        } 
        break;	       	
    case CP_TYPE_GBT_SINGLE:	
        if(nGunIdx == 0)
        {
            *pnType = 2;
            sprintf(pszName, "DC1, GBT");
        }
        else
        {
            return FALSE;
        }        
        break;		
    case CP_TYPE_GBT_GBT:	
        if(nGunIdx == 0)
        {
            *pnType = 2;
            sprintf(pszName, "DC1, GBT");
        }
        else if(nGunIdx == 1)
        {
            *pnType = 2;
            sprintf(pszName, "DC2, GBT");
        }
        else
        {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }
    */
    return TRUE;
}


static BOOL Handle_Home_PageData(FCGI_ROUGH_DATA* pFCGI_Rough, int nPageIdx)
{
    TRACEX("\n----now handling home_page data----\n\n");
    WEB_PARAM_VALUE* pWebParaVal;
    char* szStringVal;
    int n;
    cJSON* pDictionary = cJSON_CreateObject();

    if (pFCGI_Rough->emSystemType <= SYSTYPE_ESS_LIQUIDED)
    {
        //get system run info
        A_BLOB_ES_RUN_DATA esRunData;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 80, sizeof(A_BLOB_ES_RUN_DATA), (void*)&esRunData))
        {
            TRACEX("----ERRRRRR: can not get ES RunData----\n");
            return TRUE;
        }

        //get pv info
        A_BLOB_PV_INVERTER_DATA_TOTAL pvInverterDataTotal;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1050, 3697, sizeof(A_BLOB_PV_INVERTER_DATA_TOTAL), (void*)&pvInverterDataTotal))
        {
            TRACEX("----ERRRRRR: can not get PV Inverter Data Total----\n");
            return TRUE;
        }

        int Pv_Enable = 0; //光伏使能
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1050, 458, 4, (void*)&Pv_Enable))
        {
            TRACEX("----ERRRRRR: can not get pV----\n");
            return TRUE;
        }
        int Pv_Flag = (Pv_Enable == 1) ? 0 : 1; //0是显示，1不显示

        int F_Meter_Enable = 0; //电表F如果安装就显示到光伏逆变器功率
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 406, 401, 4, (void*)&F_Meter_Enable))
        {
            TRACEX("----ERRRRRR: can not get meter----\n");
            return TRUE;
        }

        float fPvPower = 0.0f;
        if(F_Meter_Enable == 1) //安装F电表
        {
            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 406, 400, 4, (void*)&fPvPower))
            {
                TRACEX("----ERRRRRR: can not get meter400----\n");
                return TRUE;
            }
        }
        else
        {
            fPvPower = pvInverterDataTotal.fTotalPVOutputActPower;
        }

        float fPVActiveOutputPercent = 0.0f;
        if(pvInverterDataTotal.iTotalRatedPower == 0)//避免计算百分比除数为0
        {
            fPVActiveOutputPercent = 0.0f;
        }
        else 
        {
            fPVActiveOutputPercent = fPvPower / (float)pvInverterDataTotal.iTotalRatedPower * 100;
        }

        if(nPageIdx == HOME_PAGEID_TOPOLOGY)
        {
            //Topology begins

            //--- PV Inverter DataInfo  ---//
            cJSON* pJSPVInverterInfo = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJSPVInverterInfo, "photovoltaic", Pv_Flag); //0是显示，1不显示
            cJSON_AddFloatToObject(pJSPVInverterInfo, "p_pvac", fPvPower, 1);
            cJSON_AddFloatToObject(pJSPVInverterInfo, "active_percent", fPVActiveOutputPercent, 1);
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "pv_inverter_info", pJSPVInverterInfo);


            //--- ACGrid Meter DataInfo ---//
            char szGridMeterName[64] = "\0";
            snprintf(szGridMeterName, 64, "%.1f|%.1f|%.1fV\n%.1f|%.1f|%.1fA\nP:%.2fkW Q:%.1fkVar",
                esRunData.fGrid_ACVolt_L1, esRunData.fGrid_ACVolt_L2, esRunData.fGrid_ACVolt_L3, 
                esRunData.fGrid_ACCurr_L1, esRunData.fGrid_ACCurr_L2, esRunData.fGrid_ACCurr_L3,
                esRunData.f_PgridMeter,esRunData.f_QgridMeter);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "acgrid_meter_info", szGridMeterName);


            //--- Cabinet Meter DataInfo ---//
            char szCabMeterName[64] = "\0";
            snprintf(szCabMeterName, 64, "%.1f|%.1f|%.1fV\n%.1f|%.1f|%.1fA\nP:%.1fkW Q:%.1fkVar",
                esRunData.fACVolt_L1, esRunData.fACVolt_L2, esRunData.fACVolt_L3,
                esRunData.fACCurr_L1, esRunData.fACCurr_L2, esRunData.fACCurr_L3,
                esRunData.f_PcabMeter, esRunData.f_QcabMeter);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "cabinet_meter_info", szCabMeterName);
            

            //---  STS DataInfo ---//
            cJSON* pJSSTSInfo = cJSON_CreateObject();
            char szSTSName[64] = "\0";
            snprintf(szSTSName, 64, "Mode:%s\nABU:%s\nBus:%s\n%.1fHz | %.0f\u2103\n",
                (esRunData.mapSTSCode.Status.ManualAuto_Mode==0)?"Auto":"Manual", 
                (esRunData.mapSTSCode.Status.OnOffGrid_State==0)?"ON-Grid":"OFF-Grid",
                (esRunData.mapSTSCode.Status.STS2Pcs_OnOffGridState==0)?"ON-Grid":"OFF-Grid", 
                esRunData.fAC_Freq, esRunData.fAmbit_Temp);
            cJSON_AddStringToObject(pJSSTSInfo, "name", szSTSName);

            int nSTSEnumResId = 0;
            int nSTSinstalled;
            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 705, 1128, 4, (void*)&nSTSinstalled))
            {
                TRACEX("----ERRRRRR: can not get CCU Specific Settings----\n");
                return TRUE;
            }
            if(nSTSinstalled == 1)
            {
                nSTSEnumResId = 267; //NotInstalled
            }
            else if(nSTSinstalled == 0)
            {
                nSTSEnumResId = 39; //Connected
                /*if(esRunData.mapSTSCode.Status.Power_OnOff == 0)
                {
                    nSTSEnumResId = 33; //DisConnected
                }
                else if(esRunData.mapSTSCode.Status.Power_OnOff == 1)
                {
                    nSTSEnumResId = 39; //Connected
                }*/
            }
            else
            {
                TRACEX("----ERRRRRR: do not know STS install status----\n");
                nSTSEnumResId = 267;
            }
            Add_Trans_Into_Dictionary(pDictionary, nSTSEnumResId);
            cJSON_AddNumberToObject(pJSSTSInfo, "status_resId", nSTSEnumResId);
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "sts_info", pJSSTSInfo);


            //---  AC PCS DataInfo ---//
            cJSON*  pJSACPcsTotalInfo = cJSON_CreateArray();
            char    szACPcsName[64] = "\0";
            int     nACPcsBattIdx = 0;
            int     nACPcsEnumResId = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }

                cJSON* pJSACPcsInfo = cJSON_CreateObject();
                nACPcsBattIdx = n + 1;
                cJSON_AddNumberToObject(pJSACPcsInfo, "id", nACPcsBattIdx); 
                snprintf(szACPcsName, 64, "Uabc:%.1f|%.1f|%.1fV\nIabc:%.0f|%.0f|%.0fA\nP:%.1fkW Q:%.1fkVar",
                    esRunData.fV_PCS3Phase[n][0], esRunData.fV_PCS3Phase[n][1], esRunData.fV_PCS3Phase[n][2],
                    esRunData.fI_PCS3Phase[n][0], esRunData.fI_PCS3Phase[n][1], esRunData.fI_PCS3Phase[n][2],
                    esRunData.fActvPwr_PCSGrp[n],esRunData.fQ_PCSGrp[n]);

                cJSON_AddStringToObject(pJSACPcsInfo, "name", szACPcsName); 

                if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_IDLE)
                {
                    nACPcsEnumResId = 213;
                }
                else if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_RECTIFY)
                {
                    nACPcsEnumResId = 709;
                }
                else if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_INVERTER)
                {
                    nACPcsEnumResId = 710;
                }
                else if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_STANDBY)
                {
                    nACPcsEnumResId = 90;
                }
                Add_Trans_Into_Dictionary(pDictionary, nACPcsEnumResId);
                cJSON_AddNumberToObject(pJSACPcsInfo, "status_resId", nACPcsEnumResId); 
                cJSON_AddItemToArray(pJSACPcsTotalInfo, pJSACPcsInfo); 
            } 
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "ac_pcs_info", pJSACPcsTotalInfo);


            //---  DC PCS DataInfo ---//
            cJSON*  pJSDCPcsTotalInfo = cJSON_CreateArray();
            char    szDCPcsName[64] = "\0";
            int     nDCPcsBattIdx = 0;
            int     nDCPcsEnumResId = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }

                cJSON* pJSDCPcsInfo = cJSON_CreateObject();
                nDCPcsBattIdx = n + 1;
                cJSON_AddNumberToObject(pJSDCPcsInfo, "id", nDCPcsBattIdx); 
                snprintf(szDCPcsName, 64, "Ud:%.1fV\nId:%.1fA",
                    esRunData.fACDC_OutVolt[n], esRunData.fACDC_OutCurr[n]);

                cJSON_AddStringToObject(pJSDCPcsInfo, "name", szDCPcsName); 
                cJSON_AddItemToArray(pJSDCPcsTotalInfo, pJSDCPcsInfo); 
            } 
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dc_pcs_info", pJSDCPcsTotalInfo);


            //---  Battery DataInfo ---//
            cJSON*  pJSBattTotalInfo = cJSON_CreateArray();
            char    szBattName[64] = "\0";
            int     nBattIdx = 0;
            int     nBattEnumResId = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }

                cJSON* pJSBattInfo = cJSON_CreateObject();
                nBattIdx = n + 1;
                cJSON_AddNumberToObject(pJSBattInfo, "id", nBattIdx); 
                snprintf(szBattName, 64, "Ud:%.1fV Id:%.1fA\nSOC:%d%% T:%.1f\u2103\n",
                    esRunData.fSTBatt_OutVolt[n], esRunData.fSTBatt_OutCurr[n], 
                    esRunData.nSTBatt_SOC[n], esRunData.fSTBatt_Tmax[n]);

                cJSON_AddStringToObject(pJSBattInfo, "name", szBattName); 

                pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + n, 550); 
                if(pWebParaVal == NULL) 
                {
                    printf("Error: Battery RunStatus:pWebParaVal == NULL\n");
                    break;
                }
                szStringVal = GetParam_EnumString_FromVal(pWebParaVal, &nBattEnumResId);
                if(szStringVal)
                {
                    cJSON_AddStringToObject(pJSBattInfo, "status", szStringVal); 
                }
                cJSON_AddItemToArray(pJSBattTotalInfo, pJSBattInfo);
            } 
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "battery_info", pJSBattTotalInfo);


            //---  ABU Input DataInfo ---//
            cJSON* pJSAbuInputInfo = cJSON_CreateObject();
            char szABUInName[64] = "\0";

            if(esRunData.emABUInstalled == 1)//1-Installed.
            {
                cJSON_AddNumberToObject(pJSAbuInputInfo, "abu_install_status", 1);
            }
            else
            {
                cJSON_AddNumberToObject(pJSAbuInputInfo, "abu_install_status", 0);
            }
            snprintf(szABUInName, 64, "ABU-IN:\n%.1fV\n%.1fV\n%.1fV",
                esRunData.fSTS_InputVolt[0],
                esRunData.fSTS_InputVolt[1],
                esRunData.fSTS_InputVolt[2]);
            cJSON_AddStringToObject(pJSAbuInputInfo, "name", szABUInName);
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "abu_in_info", pJSAbuInputInfo);


            //---  ABU Output DataInfo ---//
            cJSON* pJSAbuOutputInfo = cJSON_CreateObject();
            char szABUOutName[64] = "\0";

            if(esRunData.emABUInstalled == 1)//1-Installed.
            {
                cJSON_AddNumberToObject(pJSAbuOutputInfo, "abu_install_status", 1);
            }
            else
            {
                cJSON_AddNumberToObject(pJSAbuOutputInfo, "abu_install_status", 0);
            }
            snprintf(szABUOutName, 64, "ABU-OUT:\n%.1fV\n%.1fV\n%.1fV",
                esRunData.fSTS_OutputVolt[0], 
                esRunData.fSTS_OutputVolt[1],
                esRunData.fSTS_OutputVolt[2]);
            cJSON_AddStringToObject(pJSAbuOutputInfo, "name", szABUOutName);
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "abu_out_info", pJSAbuOutputInfo);

            //---  Network Status DataInfo ---//
            cJSON* pJSNetworkStatusInfo = cJSON_CreateObject();
            int nNetworkEnumResId = 0;
            A_BLOB_NETWORKINFO netInfo;
            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 3, sizeof(A_BLOB_NETWORKINFO), (void*)&netInfo))
            {
                TRACEX("----ERRRRRR: can not get Network Communication Status----\n");
                return TRUE;
            }
            else
            {
                cJSON_AddNumberToObject(pJSNetworkStatusInfo, "wireless_network_status", netInfo.netInfo_usb0.emNetPort_Status);
                cJSON_AddNumberToObject(pJSNetworkStatusInfo, "wired_network1_status", netInfo.netInfo_eht0.emNetPort_Status);
                cJSON_AddNumberToObject(pJSNetworkStatusInfo, "wired_network2_status", netInfo.netInfo_eht1.emNetPort_Status);
                cJSON_AddNumberToObject(pJSNetworkStatusInfo, "signal_level", netInfo.lteModemINfo.nSignal_Level);
            }
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "network_status_info", pJSNetworkStatusInfo);


            //---  Server Status DataInfo ---//
            int nIsServerCommOK;

            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1, 5, 4, (void*)&nIsServerCommOK))
            {
                TRACEX("----ERRRRRR: can not get Server Communication Status----\n");
                return TRUE;
            }
            else
            {
                //0-Normal 1-abnormal
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "server_status_info", nIsServerCommOK);
            }
            


            //---  PCS Contactor Status DataInfo ---//
            cJSON* pJSPcsContactorTotalInfo = cJSON_CreateArray();

            if(!esRunData.emSTBatt_WorkStatu[0] == STBATT_RUNSTAT_NOT_INSTALLED)
            {
                cJSON* pJSPcsContactorInfo = cJSON_CreateObject();
                if(esRunData.mapDIStatus.Status.DI_PCSGrp1_Contactor == 0)
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 0);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
                else
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 1);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
            }
            if(!esRunData.emSTBatt_WorkStatu[1] == STBATT_RUNSTAT_NOT_INSTALLED)
            {
                cJSON* pJSPcsContactorInfo = cJSON_CreateObject();
                if(esRunData.mapDIStatus.Status.DI_PCSGrp2_Contactor == 0)
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 0);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
                else
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 1);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
            }
            if(!esRunData.emSTBatt_WorkStatu[2] == STBATT_RUNSTAT_NOT_INSTALLED)
            {
                cJSON* pJSPcsContactorInfo = cJSON_CreateObject();
                if(esRunData.mapDIStatus.Status.DI_PCSGrp3_Contactor == 0)
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 0);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
                else
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 1);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
            }
            if(!esRunData.emSTBatt_WorkStatu[3] == STBATT_RUNSTAT_NOT_INSTALLED)
            {
                cJSON* pJSPcsContactorInfo = cJSON_CreateObject();
                if(esRunData.mapDIStatus.Status.DI_PCSGrp3_Contactor == 0)
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 0);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
                else
                {
                    cJSON_AddNumberToObject(pJSPcsContactorInfo, "value", 1);
                    cJSON_AddItemToArray(pJSPcsContactorTotalInfo, pJSPcsContactorInfo);
                }
            }
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "pcs_contactor_info", pJSPcsContactorTotalInfo);


            //---  AC Load DataInfo ---//
            char szACLoadName[64] = "\0";
            snprintf(szACLoadName, 64, "%.2fkW", esRunData.fACLoad_Power);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "ac_load_info", szACLoadName);


            //---  MPPT DataInfo ---//
            cJSON* pJSMpptTotalInfo = cJSON_CreateArray();
            char szMpptName[64] = "\0";
            int nMpptIdx = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }
                cJSON* pJSMpptInfo = cJSON_CreateObject();
                if(esRunData.isMPPT_Grp_Exist[n] == 1)
                {
                    cJSON_AddNumberToObject(pJSMpptInfo, "install_status", 1);
                }
                else
                {
                    cJSON_AddNumberToObject(pJSMpptInfo, "install_status", 0);
                }
                nMpptIdx = n + 1;
                cJSON_AddNumberToObject(pJSMpptInfo, "id", nMpptIdx); 
                snprintf(szMpptName, 64, "Ud:%.1fV\nId:%.1fA", esRunData.fMPPT_OutVolt[n], esRunData.fMPPT_OutCurr[n]);
                cJSON_AddStringToObject(pJSMpptInfo, "name", szMpptName);
                cJSON_AddItemToArray(pJSMpptTotalInfo, pJSMpptInfo);
            }
            
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "mppt_total_info", pJSMpptTotalInfo);


            //---  ACDC Outrelay DataInfo ---//
            cJSON* pJSACDCOutTotalInfo = cJSON_CreateArray();
            char szACDCOutName[64] = "\0";
            int nACDCOutIdx = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }
                cJSON* pJSACDCOutInfo = cJSON_CreateObject();
                nACDCOutIdx = n + 1;
                cJSON_AddNumberToObject(pJSACDCOutInfo, "id", nACDCOutIdx); 
                snprintf(szACDCOutName, 64, "Uout:%.1fV", esRunData.fACDCOutRelayVolt[n]);
                cJSON_AddStringToObject(pJSACDCOutInfo, "name", szACDCOutName);
                cJSON_AddItemToArray(pJSACDCOutTotalInfo, pJSACDCOutInfo);
            }
            
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "acdc_out_total_info", pJSACDCOutTotalInfo);


            //---  Cabinet qf DataInfo ---//
            if(esRunData.mapDIStatus.Status.DI_PwrCabQF1 == 0)
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "cabinet_qf_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "cabinet_qf_info", 1);//1-DIS_CONNECTED
            }


            //---  Grid qf DataInfo ---//
            if(esRunData.mapDIStatus.Status.DI_UserGridQF == 0)
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "grid_qf_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "grid_qf_info", 1);
            }


            //---  Phase N Contactor DataInfo ---//
            if(esRunData.mapDIStatus.Status.DI_PhaN_Contactor == 0)
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "phaN_contactor_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "phaN_contactor_info", 1);
            }

            
            //---  Load qf DataInfo ---//
            if(esRunData.mapDIStatus.Status.DI_ACLoadOutputMCBQF2 == 0)
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "load_qf_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "load_qf_info", 1);
            }


            //---  energy flow DataInfo ---//
            cJSON* pJSEnergyFlowTotalInfo = cJSON_CreateObject();
            cJSON* pJSEnergyFlowGrpInfo = cJSON_CreateArray();
            int emState;
            //flag 1
            emState = (esRunData.emGridAC_WorkStatus == SOURCE_AC_PENDING_OUT) ? ESFLOW_STAT_POS :
                    ((esRunData.emGridAC_WorkStatus == SOURCE_AC_PENDING_IN) ? ESFLOW_STAT_NEG : ESFLOW_STAT_OFF);
            cJSON_AddNumberToObject(pJSEnergyFlowTotalInfo, "flag_acgrid", emState);

            //flag 2
            emState = (esRunData.emACLoad_WorkStatus == SOURCE_AC_PENDING_OUT) ? ESFLOW_STAT_POS :
                    ((esRunData.emACLoad_WorkStatus == SOURCE_AC_PENDING_IN) ? ESFLOW_STAT_NEG : ESFLOW_STAT_OFF);
            cJSON_AddNumberToObject(pJSEnergyFlowTotalInfo, "flag_acload", emState);

            //flag 3
            emState = (esRunData.emAC_WorkStatus == SOURCE_AC_PENDING_OUT) ? ESFLOW_STAT_POS :
                    ((esRunData.emAC_WorkStatus == SOURCE_AC_PENDING_IN) ? ESFLOW_STAT_NEG : ESFLOW_STAT_OFF);
            cJSON_AddNumberToObject(pJSEnergyFlowTotalInfo, "flag_ac", emState);

            //flag 4-end
            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }

                cJSON* pJSEnergyFlowInfo = cJSON_CreateObject();

                cJSON_AddNumberToObject(pJSEnergyFlowInfo, "id", n + 1);
                emState = ((esRunData.emACDC_WorkStatu[n] == STATU_ACDC_IDLE)||(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_STANDBY)) ? ESFLOW_STAT_OFF :
                        ((esRunData.emACDC_WorkStatu[n] == STATU_ACDC_RECTIFY) ? ESFLOW_STAT_POS : ESFLOW_STAT_NEG);
                cJSON_AddNumberToObject(pJSEnergyFlowInfo, "flag_acdc", emState);

                emState = (esRunData.fMPPT_OutCurr[n] < 1.0) ? ESFLOW_STAT_OFF : ESFLOW_STAT_POS;
                cJSON_AddNumberToObject(pJSEnergyFlowInfo, "flag_mppt", emState);

                emState = (esRunData.fSTBatt_OutCurr[n] > 1.0) ? ESFLOW_STAT_NEG :
                        ((esRunData.fSTBatt_OutCurr[n] < -1.0) ? ESFLOW_STAT_POS : ESFLOW_STAT_OFF);
                cJSON_AddNumberToObject(pJSEnergyFlowInfo, "flag_batt", emState);

                cJSON_AddItemToArray(pJSEnergyFlowGrpInfo, pJSEnergyFlowInfo);
            }

            cJSON_AddItemToObject(pJSEnergyFlowTotalInfo, "grp_flags", pJSEnergyFlowGrpInfo);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "energy_flow_flags", pJSEnergyFlowTotalInfo);
            //Topology ends

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dictionary", pDictionary);
            return TRUE;
        }

        if(nPageIdx == HOME_PAGEID_ENERGYFLOW)
        {
            //Energy Stack begins
            //---  Transfer Meter DataInfo ---//
            char szTransferMeterName[64] = "\0";
            snprintf(szTransferMeterName, 64, "Psum:%.1fkW\nQsum:%.1fkVar", esRunData.f_PgridMeter, esRunData.f_QgridMeter);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "transfer_meter_info", szTransferMeterName);


            //---  Slave Number DataInfo ---//
            int nSlaveNumber = 0;
            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 27, 4, (void*)&nSlaveNumber))
            {
                TRACEX("----ERRRRRR: can not get Slave Number DataInfo----\n");
                //return TRUE;
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "slave_number", nSlaveNumber);
            }


            //---  Slave Meter DataInfo ---//
            int nSlaveMeterIdx = 0;
            char szSlaveMeterName[64] = "\0";
            cJSON* pJSSlaveMeterTotalInfo = cJSON_CreateArray();
            A_BLOB_METER_VALUES meterValues;

            for(n = 0; n <= MIN(nSlaveNumber, MAX_SLVMETER_NUM); n++)
            {
                //if(n > MAX_SLVMETER_NUM - 1)
                //{
                //    break;
            // }
                cJSON* pJSSlaveMeterInfo = cJSON_CreateObject();
                cJSON* pJSSlaveMeterMainData = cJSON_CreateArray();
                cJSON* pJSSlaveMeterDetailedData1 = cJSON_CreateArray();
                cJSON* pJSSlaveMeterDetailedData2 = cJSON_CreateArray();

                nSlaveMeterIdx = n + 1;
                cJSON_AddNumberToObject(pJSSlaveMeterInfo, "id", nSlaveMeterIdx);

                snprintf(szSlaveMeterName, 64, "%s", esRunData.szIP[n]);
                cJSON_AddItemToArray(pJSSlaveMeterMainData, cJSON_CreateString(szSlaveMeterName));

                snprintf(szSlaveMeterName, 64, "Tp:%d", esRunData.emDataFrom[n]);
                cJSON_AddItemToArray(pJSSlaveMeterMainData, cJSON_CreateString(szSlaveMeterName));

                snprintf(szSlaveMeterName, 64, "%.1fkW", esRunData.f_PMeter[n]);
                cJSON_AddItemToArray(pJSSlaveMeterMainData, cJSON_CreateString(szSlaveMeterName));
                
                snprintf(szSlaveMeterName, 64, "%.1fkVar", esRunData.f_QMeter[n]);
                cJSON_AddItemToArray(pJSSlaveMeterMainData, cJSON_CreateString(szSlaveMeterName));

                snprintf(szSlaveMeterName, 64, "%s", esRunData.szRefreshTM[n]);
                cJSON_AddItemToArray(pJSSlaveMeterMainData, cJSON_CreateString(szSlaveMeterName));

                cJSON_AddItemToObject(pJSSlaveMeterInfo, "main_data", pJSSlaveMeterMainData);

                if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 800 + n, 430, sizeof(A_BLOB_METER_VALUES), (void*)&meterValues))
                {
                    TRACEX("----ERRRRRR: can not get %dth meter values----\n", n + 1);
                    //return TRUE;
                }
                else
                {
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Vabc"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Iabc"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Psum"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Pabc"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Qsum"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Qabc"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Pos"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Neg"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("#1"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("#2"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Freq"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("PF"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("PFabc"));
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData1, cJSON_CreateString("Pmax"));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1f|%.1f|%.1fV",
                        meterValues.fPhA_Volt, meterValues.fPhB_Volt, meterValues.fPhC_Volt);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1f|%.1f|%.1fA",
                        meterValues.fPhA_Curr, meterValues.fPhB_Curr, meterValues.fPhC_Curr);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkW",
                        meterValues.fRealTime_ActivePower);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1f|%.1f|%.1fkW",
                        meterValues.fPhA_ActivePower, meterValues.fPhB_ActivePower, meterValues.fPhC_ActivePower);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkVar",
                        meterValues.fReactivePower);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1f|%.1f|%.1fkVar",
                        meterValues.fPhA_ReactivePower, meterValues.fPhB_ReactivePower, meterValues.fPhC_ReactivePower);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkWh",
                        meterValues.fRealTime_kWh);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkWh",
                        meterValues.fReverse_kWh);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkVarh",
                        meterValues.fRealTime_kVarh1);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkVarh",
                        meterValues.fRealTime_kVarh2);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fHz", meterValues.fAC_frequency);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1f",
                        meterValues.fPower_Factor);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1f|%.1f|%.1f",
                        meterValues.fPhA_PowerFactor, meterValues.fPhB_PowerFactor, meterValues.fPhC_PowerFactor);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    snprintf(szSlaveMeterName, 64, 
                        "%.1fkW", meterValues.fPmaxDemand);
                    cJSON_AddItemToArray(pJSSlaveMeterDetailedData2, cJSON_CreateString(szSlaveMeterName));

                    cJSON_AddItemToObject(pJSSlaveMeterInfo, "detailed_data1", pJSSlaveMeterDetailedData1);
                    cJSON_AddItemToObject(pJSSlaveMeterInfo, "detailed_data2", pJSSlaveMeterDetailedData2);
                }

                cJSON_AddItemToArray(pJSSlaveMeterTotalInfo, pJSSlaveMeterInfo);
            }
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "slave_meter_info", pJSSlaveMeterTotalInfo);
            //Energy Stack ends

            return TRUE;
        }

        if(nPageIdx == HOME_PAGEID_SCENE)
        {
            //Scene Graph begins
            //---  Scene Graph DataInfo ---//
            cJSON* pJSSceneGraphDataInfo = cJSON_CreateObject();
            char szSceneGraphName[64] = "\0";

            cJSON* pJSBranch1DataInfo = cJSON_CreateObject();
            cJSON* pJSGridMeterDataInfo = cJSON_CreateObject();
            cJSON* pJSBranchMeterDataInfo = cJSON_CreateObject();
            cJSON* pJSIndustrialMeterDataInfo = cJSON_CreateObject();
            cJSON* pJSBranch2DataInfo = cJSON_CreateObject();
            cJSON* pJSCivilMeterDataInfo = cJSON_CreateObject();
            cJSON* pJSIndustrial2MeterDataInfo = cJSON_CreateObject();
            cJSON* pJSESSDataInfo = cJSON_CreateObject();
            cJSON* pJSAllLoadDataInfo = cJSON_CreateObject();
            cJSON* pJSSceneGraphFormDataInfo = cJSON_CreateObject();

            if(esRunData.emAuxPos == AUXMETER_NOT_INSTALLED)
            {
                cJSON_AddNumberToObject(pJSBranch1DataInfo, "branch1_flag", 0);
                cJSON_AddStringToObject(pJSBranch1DataInfo, "value_branch1_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSGridMeterDataInfo, "label_grid_meter_flag", 0);
                cJSON_AddStringToObject(pJSGridMeterDataInfo, "value_grid_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSBranchMeterDataInfo, "label_branch_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PgridMeter);
                cJSON_AddStringToObject(pJSBranchMeterDataInfo, "value_branch_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSIndustrialMeterDataInfo, "label_industrial_meter_flag", 0);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fPacload4Dischg);
                cJSON_AddStringToObject(pJSIndustrialMeterDataInfo, "value_industrial_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSBranch2DataInfo, "branch2_flag", 0);
                cJSON_AddStringToObject(pJSBranch2DataInfo, "value_branch2_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSCivilMeterDataInfo, "label_civil_meter_flag", 0);
                cJSON_AddStringToObject(pJSCivilMeterDataInfo, "value_civil_meter", cJSON_CreateString(""));

                cJSON_AddStringToObject(pJSIndustrial2MeterDataInfo, "value_industrial2_meter", cJSON_CreateString(""));

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PcabMeter);
                cJSON_AddStringToObject(pJSESSDataInfo, "value_ess_meter", szSceneGraphName);

                cJSON_AddStringToObject(pJSAllLoadDataInfo, "value_all_load", cJSON_CreateString(""));
            }

            else if(esRunData.emAuxPos == AUXMETER_POS_BRANCH_TRANSFER)
            {
                cJSON_AddNumberToObject(pJSBranch1DataInfo, "branch1_flag", 0);
                cJSON_AddStringToObject(pJSBranch1DataInfo, "value_branch1_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSGridMeterDataInfo, "label_grid_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PgridMeter);
                cJSON_AddStringToObject(pJSGridMeterDataInfo, "value_grid_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSBranchMeterDataInfo, "label_branch_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PauxMeter);
                cJSON_AddStringToObject(pJSBranchMeterDataInfo, "value_branch_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSIndustrialMeterDataInfo, "label_industrial_meter_flag", 0);
                cJSON_AddStringToObject(pJSIndustrialMeterDataInfo, "value_industrial_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSBranch2DataInfo, "branch2_flag", 0);
                cJSON_AddStringToObject(pJSBranch2DataInfo, "value_branch2_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSCivilMeterDataInfo, "label_civil_meter_flag", 0);
                cJSON_AddStringToObject(pJSCivilMeterDataInfo, "value_civil_meter", cJSON_CreateString(""));

                cJSON_AddStringToObject(pJSIndustrial2MeterDataInfo, "value_industrial2_meter", cJSON_CreateString(""));

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PcabMeter);
                cJSON_AddStringToObject(pJSESSDataInfo, "value_ess_meter", szSceneGraphName);

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fPacload4Dischg);
                cJSON_AddStringToObject(pJSAllLoadDataInfo, "value_all_load", szSceneGraphName);
            }

            else if(esRunData.emAuxPos == AUXMETER_POS_CIVIL_ELECTRIC)
            {
                cJSON_AddNumberToObject(pJSBranch1DataInfo, "branch1_flag", 0);
                cJSON_AddStringToObject(pJSBranch1DataInfo, "value_branch1_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSGridMeterDataInfo, "label_grid_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PgridMeter);
                cJSON_AddStringToObject(pJSGridMeterDataInfo, "value_grid_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSBranchMeterDataInfo, "label_branch_meter_flag", 0);
                cJSON_AddStringToObject(pJSBranchMeterDataInfo, "value_branch_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSIndustrialMeterDataInfo, "label_industrial_meter_flag", 0);
                cJSON_AddStringToObject(pJSIndustrialMeterDataInfo, "value_industrial_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSBranch2DataInfo, "branch2_flag", 0);
                cJSON_AddStringToObject(pJSBranch2DataInfo, "value_branch2_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSCivilMeterDataInfo, "label_civil_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PauxMeter);
                cJSON_AddStringToObject(pJSCivilMeterDataInfo, "value_civil_meter", szSceneGraphName);

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fPacload4Dischg);
                cJSON_AddStringToObject(pJSIndustrial2MeterDataInfo, "value_industrial2_meter", szSceneGraphName);

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PcabMeter);
                cJSON_AddStringToObject(pJSESSDataInfo, "value_ess_meter", szSceneGraphName);

                cJSON_AddStringToObject(pJSAllLoadDataInfo, "value_all_load", cJSON_CreateString(""));
            }

            else if(esRunData.emAuxPos == AUXMETER_POS_ACLOAD)
            {
                cJSON_AddNumberToObject(pJSBranch1DataInfo, "branch1_flag", 0);
                cJSON_AddStringToObject(pJSBranch1DataInfo, "value_branch1_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSGridMeterDataInfo, "label_grid_meter_flag", 0);
                cJSON_AddStringToObject(pJSGridMeterDataInfo, "value_grid_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSBranchMeterDataInfo, "label_branch_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PgridMeter);
                cJSON_AddStringToObject(pJSBranchMeterDataInfo, "value_branch_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSIndustrialMeterDataInfo, "label_industrial_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fPacload4Dischg);
                cJSON_AddStringToObject(pJSIndustrialMeterDataInfo, "value_industrial_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSBranch2DataInfo, "branch2_flag", 0);
                cJSON_AddStringToObject(pJSBranch2DataInfo, "value_branch2_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSCivilMeterDataInfo, "label_civil_meter_flag", 0);
                cJSON_AddStringToObject(pJSCivilMeterDataInfo, "value_civil_meter", cJSON_CreateString(""));

                cJSON_AddStringToObject(pJSIndustrial2MeterDataInfo, "value_industrial2_meter", cJSON_CreateString(""));

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PcabMeter);
                cJSON_AddStringToObject(pJSESSDataInfo, "value_ess_meter", szSceneGraphName);

                cJSON_AddStringToObject(pJSAllLoadDataInfo, "value_all_load", cJSON_CreateString(""));
            }

            else//AUXMETER_MULTI_BANCH
            {
                cJSON_AddNumberToObject(pJSBranch1DataInfo, "branch1_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PauxMeter);
                cJSON_AddStringToObject(pJSBranch1DataInfo, "value_branch1_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSGridMeterDataInfo, "label_grid_meter_flag", 0);
                cJSON_AddStringToObject(pJSGridMeterDataInfo, "value_grid_meter", cJSON_CreateString(""));

                cJSON_AddNumberToObject(pJSBranchMeterDataInfo, "label_branch_meter_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PgridMeter);
                cJSON_AddStringToObject(pJSBranchMeterDataInfo, "value_branch_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSIndustrialMeterDataInfo, "label_industrial_meter_flag", 0);
                snprintf(szSceneGraphName, 64, "%.1fkW", (esRunData.f_PgridMeter - esRunData.f_PcabMeter));
                cJSON_AddStringToObject(pJSIndustrialMeterDataInfo, "value_industrial_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSBranch2DataInfo, "branch2_flag", 1);
                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PauxMeter_3);
                cJSON_AddStringToObject(pJSBranch2DataInfo, "value_branch2_meter", szSceneGraphName);

                cJSON_AddNumberToObject(pJSCivilMeterDataInfo, "label_civil_meter_flag", 0);
                cJSON_AddStringToObject(pJSCivilMeterDataInfo, "value_civil_meter", cJSON_CreateString(""));

                cJSON_AddStringToObject(pJSIndustrial2MeterDataInfo, "value_industrial2_meter", cJSON_CreateString(""));

                snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PcabMeter);
                cJSON_AddStringToObject(pJSESSDataInfo, "value_ess_meter", szSceneGraphName);

                cJSON_AddStringToObject(pJSAllLoadDataInfo, "value_all_load", cJSON_CreateString(""));
            }

            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#1", pJSBranch1DataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#2", pJSGridMeterDataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#3", pJSBranchMeterDataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#4", pJSIndustrialMeterDataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#5", pJSBranch2DataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#6", pJSCivilMeterDataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#7", pJSIndustrial2MeterDataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#8", pJSESSDataInfo);
            cJSON_AddItemToObject(pJSSceneGraphDataInfo, "#9", pJSAllLoadDataInfo);
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "scene_graph_data", pJSSceneGraphDataInfo);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PgridMeter);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Pgrid_meter", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PcabMeter);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Pcab_meter", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PauxMeter);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Paux_meter", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.f_PauxMeter_3);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Paux_meter3", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fPacload4Chg);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Pacload_Chg", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW|%.1fkW", esRunData.fPacload4Dischg, esRunData.fAvg_Pacload4Dischg);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Pacload_Dischg", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fTotal_PessChgAvaliable);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Pess_ChgAvliab", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%.1fkW", esRunData.fTotal_PessDischgLmt);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "Pess_DischgLmt", szSceneGraphName);

            snprintf(szSceneGraphName, 64, "%d", esRunData.nTMSector);
            cJSON_AddStringToObject(pJSSceneGraphFormDataInfo, "EMS_Sector", szSceneGraphName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "scene_graph_form", pJSSceneGraphFormDataInfo);
            //Scene Graph ends

            return TRUE;
        }

        if(nPageIdx == HOME_PAGEID_ONOFFGRID)
        {
            //Off-Grid begins
            //---  Qf1 DataInfo ---//
            if(esRunData.mapDIStatus.Status.DI_PwrCabQF1 == 0)
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "qf1_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "qf1_info", 1);
            }


            //---  Qf2 DataInfo ---//
            if((esRunData.fSTS_OutputVolt[0] > 50) && 
            (esRunData.fVoffLoad[0] < 50))
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "qf2_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "qf2_info", 1);
            }


            //---  Qf3 DataInfo ---//
            if((esRunData.fACVolt_L1 > 50) &&
            (esRunData.fSTS_OutputVolt[0] < 50) &&
            (esRunData.fVoffLoad[0] > 50))
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "qf3_info", 0);
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "qf3_info", 1);
            }


            //---  Off Grid STS Status DataInfo ---//
            cJSON* cJSSTSStatusInfo = cJSON_CreateObject();
            char szSTSStatusName[64] = "\0";
            int nSTSInstallStatus;
            if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 705, 1128, 4, (void*)&nSTSInstallStatus))
            {
                TRACEX("----ERRRRRR: can not get CCU Specific Settings----\n");
                return TRUE;
            }
            if(nSTSInstallStatus == 1)
            {
                cJSON_AddStringToObject(cJSSTSStatusInfo, "STS Status", "Not Installed"); //NotInstalled
            }
            else if(nSTSInstallStatus == 0)
            {
                if(esRunData.mapSTSCode.Status.Power_OnOff == 0)
                {
                cJSON_AddStringToObject(cJSSTSStatusInfo, "STS Status", "Off"); //DisConnected
                }
                else if(esRunData.mapSTSCode.Status.Power_OnOff == 1)
                {
                cJSON_AddStringToObject(cJSSTSStatusInfo, "STS Status", "On"); //Connected
                }
            }
            else
            {
                TRACEX("----ERRRRRR: do not know STS install status----\n");
                cJSON_AddStringToObject(cJSSTSStatusInfo, "STS Status", "Not Installed");
            }
            
            snprintf(szSTSStatusName, 64, "%s",
            (esRunData.mapSTSCode.Status.ManualAuto_Mode == 0)?"Auto":"Manual");
            cJSON_AddStringToObject(cJSSTSStatusInfo, "Mode", szSTSStatusName);

            snprintf(szSTSStatusName, 64, "%s",
            (esRunData.mapSTSCode.Status.OnOffGrid_State == 0)?"ON-Grid":"OFF-Grid");
            cJSON_AddStringToObject(cJSSTSStatusInfo, "STS", szSTSStatusName);

            snprintf(szSTSStatusName, 64, "%s",
            (esRunData.mapSTSCode.Status.STS2Pcs_OnOffGridState == 0)?"ON-Grid":"OFF-Grid");
            cJSON_AddStringToObject(cJSSTSStatusInfo, "Bus", szSTSStatusName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "sts_status", cJSSTSStatusInfo);


            //---  Off Grid STS Detail DataInfo ---//
            cJSON* cJSSTSDetailInfo = cJSON_CreateObject();
            char szSTSDetailedName[64] = "\0";

            snprintf(szSTSDetailedName, 64, "%.1fHz", esRunData.fAC_Freq);
            cJSON_AddStringToObject(cJSSTSDetailInfo, "Freq", szSTSDetailedName);

            snprintf(szSTSDetailedName, 64, "%.0f\u2103", esRunData.fAmbit_Temp);
            cJSON_AddStringToObject(cJSSTSDetailInfo, "T", szSTSDetailedName);

            snprintf(szSTSDetailedName, 64, "%.2fkW", esRunData.fPtotalActive);
            cJSON_AddStringToObject(cJSSTSDetailInfo, "P", szSTSDetailedName);

            snprintf(szSTSDetailedName, 64, "%.1fkVar", esRunData.fPtotalReactive);
            cJSON_AddStringToObject(cJSSTSDetailInfo, "Q", szSTSDetailedName);

            snprintf(szSTSDetailedName, 64, "%.2f", esRunData.fPtotalFactor);
            cJSON_AddStringToObject(cJSSTSDetailInfo, "PF", szSTSDetailedName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "sts_detail", cJSSTSDetailInfo);


            //---  Off Grid STS Input DataInfo ---//
            cJSON* cJSSTSInputInfo = cJSON_CreateObject();
            char szSTSInName[64] = "\0";

            snprintf(szSTSInName, 64, "%.1f|%.1f|%.1fV", 
            esRunData.fSTS_InputVolt[0], esRunData.fSTS_InputVolt[1], esRunData.fSTS_InputVolt[2]);
            cJSON_AddStringToObject(cJSSTSInputInfo, "Uabc", szSTSInName);

            snprintf(szSTSInName, 64, "%.1f|%.1f|%.1fA", 
            esRunData.fSTS_InputCurt[0], esRunData.fSTS_InputCurt[1], esRunData.fSTS_InputCurt[2]);
            cJSON_AddStringToObject(cJSSTSInputInfo, "Iabc", szSTSInName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "sts_in_info", cJSSTSInputInfo);


            //---  Off Grid STS Output DataInfo ---//
            cJSON* cJSSTSOutputInfo = cJSON_CreateObject();
            char szSTSOutName[64] = "\0";

            snprintf(szSTSOutName, 64, "%.1f|%.1f|%.1fV", 
            esRunData.fSTS_OutputVolt[0], esRunData.fSTS_OutputVolt[1], esRunData.fSTS_OutputVolt[2]);
            cJSON_AddStringToObject(cJSSTSOutputInfo, "Uabc", szSTSOutName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "sts_out_info", cJSSTSOutputInfo);


            //---  Off Grid AC Cabinet Meter DataInfo ---//
            cJSON* cJSACCabMeterInfo = cJSON_CreateObject();
            char szACCabMeterName[64] = "\0";

            snprintf(szACCabMeterName, 64, "%.1f|%.1f|%.1fV", 
            esRunData.fACVolt_L1, esRunData.fACVolt_L2, esRunData.fACVolt_L3);
            cJSON_AddStringToObject(cJSACCabMeterInfo, "Uabc", szACCabMeterName);

            snprintf(szACCabMeterName, 64, "%.1f|%.1f|%.1fA", 
            esRunData.fACCurr_L1, esRunData.fACCurr_L2, esRunData.fACCurr_L3);
            cJSON_AddStringToObject(cJSACCabMeterInfo, "Iabc", szACCabMeterName);

            snprintf(szACCabMeterName, 64, "%.2fkW", esRunData.fLocalAC_Power);
            cJSON_AddStringToObject(cJSACCabMeterInfo, "P", szACCabMeterName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "ac_cab_meter_info", cJSACCabMeterInfo);


            //---  Off Grid Load Meter DataInfo ---//
            cJSON* cJSOffGridLoadMeterInfo = cJSON_CreateObject();
            char szOffGridLoadMeterName[64] = "\0";

            snprintf(szOffGridLoadMeterName, 64, "%.1f|%.1f|%.1fV", 
            esRunData.fVoffLoad[0], esRunData.fVoffLoad[1], esRunData.fVoffLoad[2]);
            cJSON_AddStringToObject(cJSOffGridLoadMeterInfo, "Uabc", szOffGridLoadMeterName);

            snprintf(szOffGridLoadMeterName, 64, "%.2fkW", esRunData.fPoffgrid_LoadMeter);
            cJSON_AddStringToObject(cJSOffGridLoadMeterInfo, "P", szOffGridLoadMeterName);

            snprintf(szOffGridLoadMeterName, 64, "%.2fkVar", esRunData.fQoff_LoadMeter);
            cJSON_AddStringToObject(cJSOffGridLoadMeterInfo, "Q", szOffGridLoadMeterName);

            snprintf(szOffGridLoadMeterName, 64, "%.2fHz", esRunData.fFreqoff_LoadMeter);
            cJSON_AddStringToObject(cJSOffGridLoadMeterInfo, "Freq", szOffGridLoadMeterName);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "off_grid_load_meter_info", cJSOffGridLoadMeterInfo);


            //---  Off Grid PCS DataInfo ---//
            cJSON*  pJSOffGridPcsTotalInfo = cJSON_CreateArray();
            char    szOffGridPcsName[64] = "\0";
            int     nOffGridPcsBattIdx = 0;
            int     nOffGridPcsEnumResId = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }

                cJSON* pJSOffGridPcsInfo = cJSON_CreateObject();
                nOffGridPcsBattIdx = n + 1;
                cJSON_AddNumberToObject(pJSOffGridPcsInfo, "id", nOffGridPcsBattIdx); 

                if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_IDLE)
                {
                    nOffGridPcsEnumResId = 213;
                }
                else if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_RECTIFY)
                {
                    nOffGridPcsEnumResId = 709;
                }
                else if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_INVERTER)
                {
                    nOffGridPcsEnumResId = 710;
                }
                else if(esRunData.emACDC_WorkStatu[n] == STATU_ACDC_STANDBY)
                {
                    nOffGridPcsEnumResId = 90;
                }
                snprintf(szOffGridPcsName, 64, "%s|%.1fkW",
                    LoadLangText_ById(nOffGridPcsEnumResId), esRunData.fACDC_OutVolt[n]*esRunData.fACDC_OutCurr[n]*0.001);

                cJSON_AddStringToObject(pJSOffGridPcsInfo, "name", szOffGridPcsName); 
                cJSON_AddItemToArray(pJSOffGridPcsTotalInfo, pJSOffGridPcsInfo); 
            } 

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "off_grid_pcs_info", pJSOffGridPcsTotalInfo);


            //---  Off Grid Battery DataInfo ---//
            cJSON*  pJSOffGridBattTotalInfo = cJSON_CreateArray();
            char    szOffGridBattName[64] = "\0";
            int     nOffGridBattIdx = 0;
            int     nOffGridBattEnumResId = 0;

            for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
            {
                if(esRunData.emSTBatt_WorkStatu[n] == STBATT_RUNSTAT_NOT_INSTALLED)
                {
                    continue;
                }

                cJSON* pJSOffGridBattInfo = cJSON_CreateObject();
                cJSON* pJSOffGridBattName = cJSON_CreateArray();
                nOffGridBattIdx = n + 1;
                cJSON_AddNumberToObject(pJSOffGridBattInfo, "id", nOffGridBattIdx); 
                
                snprintf(szOffGridBattName, 64, "%.1fkW", esRunData.fSTBatt_OutVolt[n]*esRunData.fSTBatt_OutCurr[n]*0.001);
                cJSON_AddItemToArray(pJSOffGridBattName, cJSON_CreateString(szOffGridBattName));

                snprintf(szOffGridBattName, 64, "%d%%", esRunData.nSTBatt_SOC[n]);
                cJSON_AddItemToArray(pJSOffGridBattName, cJSON_CreateString(szOffGridBattName));

                pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + n, 550); 
                if(pWebParaVal == NULL) 
                {
                    printf("Error: Battery RunStatus:pWebParaVal == NULL\n");
                    break;
                }
                szStringVal = GetParam_EnumString_FromVal(pWebParaVal, &nOffGridBattEnumResId);
                if(szStringVal)
                {
                    cJSON_AddItemToArray(pJSOffGridBattName, cJSON_CreateString(szStringVal));
                }

                cJSON_AddItemToObject(pJSOffGridBattInfo, "name", pJSOffGridBattName); 
                cJSON_AddItemToArray(pJSOffGridBattTotalInfo, pJSOffGridBattInfo);
            } 
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "off_grid_battery_info", pJSOffGridBattTotalInfo);
            //Off-Grid ends

            return TRUE;
        }
    }
    else if (pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        //get station info
        A_BLOB_SEMS_CLIENTINFO_SUM  clientInfoSum = {0};
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3900, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM), (void*)&clientInfoSum))
        {
            TRACEX("----ERRRRRR: can not get clientInfoSum Data----\n");
            // printf("----ERRRRRR: can not get clientInfoSum Data----\n");
            return TRUE;
        }

        int Pv_Enable = 0; //光伏使能
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1050, 458, 4, (void*)&Pv_Enable))
        {
            TRACEX("----ERRRRRR: can not get pV----\n");
            return TRUE;
        }
        int Pv_Flag = (Pv_Enable == 1) ? 0 : 1; //0是显示，1不显示

        int F_Meter_Enable = 0; //电表F如果安装就显示到光伏逆变器功率
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 406, 401, 4, (void*)&F_Meter_Enable))
        {
            TRACEX("----ERRRRRR: can not get meter----\n");
            return TRUE;
        }

        float fPvPower = 0.0f;
        A_BLOB_PV_INVERTER_DATA_TOTAL pvInverterDataTotal;
        if(F_Meter_Enable == 1) //安装F电表
        {
            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 406, 400, 4, (void*)&fPvPower))
            {
                TRACEX("----ERRRRRR: can not get meter400----\n");
                return TRUE;
            }
        }
        else
        {
            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1050, 3697, sizeof(A_BLOB_PV_INVERTER_DATA_TOTAL), (void*)&pvInverterDataTotal))
            {
                TRACEX("----ERRRRRR: can not get PV Inverter Data Total----\n");
                return TRUE;
            }
            fPvPower = pvInverterDataTotal.fTotalPVOutputActPower;
        }

        float fPVActiveOutputPercent = 0.0f;
        if(pvInverterDataTotal.iTotalRatedPower == 0)//避免计算百分比除数为0
        {
            fPVActiveOutputPercent = 0.0f;
        }
        else 
        {
            fPVActiveOutputPercent = fPvPower / (float)pvInverterDataTotal.iTotalRatedPower * 100;
        }

        cJSON* PV_Data = cJSON_CreateObject();
        cJSON_AddNumberToObject(PV_Data, "photovoltaic", Pv_Flag); //0是显示，1不显示
        cJSON_AddFloatToObject(PV_Data, "p_pvac", fPvPower, 1);
        cJSON_AddFloatToObject(PV_Data, "active_percent", fPVActiveOutputPercent, 1);
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "pv_inverter_info", PV_Data);

        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "topolopy", cJSON_CreateNumber(1));
        cJSON* pJSStationInfo = cJSON_CreateArray();
        // 柜内表总表
        A_BLOB_ES_RUN_DATA esRunData;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 80, sizeof(A_BLOB_ES_RUN_DATA), (void*)&esRunData))
        {
            TRACEX("----ERRRRRR: can not get ES RunData----\n");
            return TRUE;
        }
        int  emInstallCabaleSum;
        Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 407, 401, sizeof(int), (void*)&emInstallCabaleSum);
        // printf("----emInstallCabaleSum = [%d]----\n", emInstallCabaleSum);
        if (emInstallCabaleSum == 0)
        {
            // 没有柜内总表用柜内表加和数据
            float fPsum = clientInfoSum.f_PcabMeterSum;
            char szPsum[64] = "\0";
            sprintf(szPsum, "P : %.1fkW", fPsum);
            cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString(szPsum));
            int nflow2;
            if (fPsum >= 0)
            {
                nflow2 = 0;
            }
            else
            {
                nflow2 = 1;
            }
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "flow2", nflow2);
        }
        else
        {
            // 有柜内表总表用表数据
            char szCableSumMeterV[64] = {0};
            snprintf(szCableSumMeterV, 64, "%.1f | %.1f | %.1f V", 
                esRunData.fACVolt_L1, esRunData.fACVolt_L2, esRunData.fACVolt_L3);
            cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString(szCableSumMeterV));
            char szCableSumMeterI[64] = {0};
            snprintf(szCableSumMeterI, 64, "%.1f | %.1f | %.1f A", 
                esRunData.fACCurr_L1, esRunData.fACCurr_L2, esRunData.fACCurr_L3);
            cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString(szCableSumMeterI));
            char szCableSumMeterP[64] = {0};
            snprintf(szCableSumMeterP, 64, "P : %.1f KW", esRunData.f_PcabMeter);
            cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString(szCableSumMeterP));
            char szCableSumMeterQ[64] = {0};
            snprintf(szCableSumMeterQ, 64, "Q : %.1f KVar", esRunData.f_QcabMeter);
            cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString(szCableSumMeterQ));
            int nflow2;
            if (esRunData.f_PcabMeter >= 0)
            {
                nflow2 = 0;
            }
            else
            {
                nflow2 = 1;
            }
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "flow2", nflow2);
        }
        
        // // 充电电能
        // float fChrgedE = clientInfoSum.fChrgedEnergySum;
        // char szChrgedE[64] = {0};
        // sprintf(szChrgedE, "Chrged E : %.1f kWh", fChrgedE);
        // cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString(szChrgedE));
        // cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString("Dischange E: 10 kWh"));
        // cJSON_AddItemToArray(pJSStationInfo, cJSON_CreateString("Efficiency : 92 %"));
        
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "stationcolData", pJSStationInfo);


        // grid side info
        cJSON* pJSGridInfo = cJSON_CreateArray();
        // cJSON_AddItemToArray(pJSGridInfo, cJSON_CreateString("Grid Meter Status : [OK]"));
        char szGridMeterV[64] = {0};
        snprintf(szGridMeterV, 64, "%.1f | %.1f | %.1f V", 
            esRunData.fGrid_ACVolt_L1, esRunData.fGrid_ACVolt_L2, esRunData.fGrid_ACVolt_L3);
        cJSON_AddItemToArray(pJSGridInfo, cJSON_CreateString(szGridMeterV));
        char szGridMeterI[64] = {0};
        snprintf(szGridMeterI, 64, "%.1f | %.1f | %.1f A", 
            esRunData.fGrid_ACCurr_L1, esRunData.fGrid_ACCurr_L2, esRunData.fGrid_ACCurr_L3);
        cJSON_AddItemToArray(pJSGridInfo, cJSON_CreateString(szGridMeterI));
        char szGridMeterP[64] = {0};
        snprintf(szGridMeterP, 64, "P : %.1f KW", esRunData.f_PgridMeter);
        cJSON_AddItemToArray(pJSGridInfo, cJSON_CreateString(szGridMeterP));
        char szGridMeterQ[64] = {0};
        snprintf(szGridMeterQ, 64, "Q : %.1f KVar", esRunData.f_QgridMeter);
        cJSON_AddItemToArray(pJSGridInfo, cJSON_CreateString(szGridMeterQ));
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "meterData", pJSGridInfo);
        // printf("orgnaze [meterData] successfully\n");
        // printf("33333333333333333\n");
        // printf("pJSON_Resp = %s\n", cJSON_Print(pFCGI_Rough->pJSON_Resp));

        //get client info
        // PARAM_VAL_GET valGet;
        A_BLOB_CMD155 singleESCInfo;
        // singleESCInfo.emBlobType = BLOB_TYPE_CMD155;
        // valGet.nGetSize = sizeof(A_BLOB_CMD155);
        // valGet.val.blobValue = (BYTE*)&singleESCInfo;
        // ERRCODE_DAI err = pFCGI_Rough->Get(hReporter, DAITYPE_STAT_PARAMVALUE_R,
        //     1001, 3405, &valGet, sizeof(PARAM_VAL_GET));
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3405, sizeof(A_BLOB_CMD155), (void*)&singleESCInfo))
        {
            // TRACEX("----ERRRRRR: can not get singleESCInfo----\n");
            // printf("MAIN page----ERRRRRR: can not get singleESCInfo Data----\n");
            return TRUE;
        }
        A_BLOB_SEMS_CMD109 dataBloCmd109;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3409, sizeof(A_BLOB_SEMS_CMD109), (void*)&dataBloCmd109))
        {
            TRACEX("----ERRRRRR: can not get A_BLOB_SEMS_CMD109 Data----\n");
            // printf("----ERRRRRR: can not get A_BLOB_SEMS_CMD109 Data----\n");
            return TRUE;
        }
        cJSON* pJESCInfo = cJSON_CreateArray();
        int nESCNum = 0;
        Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1001, 3314, sizeof(int), (void*)&nESCNum);
        // printf("nESCNum = %d\n", nESCNum);
        int i;
        for (i = 0; i < nESCNum; ++i)
        {
            cJSON* pJESCInfoItem = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJESCInfoItem, "onoff", i % 2);
            cJSON* pJStatus = cJSON_CreateObject();
            // 单柜通信状态
            int nESCStatus;

            int nESCCommStatus;
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3001 + i, sizeof(int), (void*)&nESCCommStatus);
            // printf("nESCCommStatus = %d\n", nESCCommStatus);
            if (nESCCommStatus == 1)
            {
                nESCStatus = 0; // 离线
                szStringVal = LoadLangText_ById(495);
            }
            else
            {
                // 充放电状态
                switch(singleESCInfo.escDatas[i].acdc_workstat)
                {
                    case STATU_ACDC_STANDBY:    // 待机
                        {
                            nESCStatus = 6;
                            szStringVal = LoadLangText_ById(6710);
                        }
                        break;
                    case STATU_ACDC_RECTIFY:       // 充电
                        {
                            nESCStatus = 1;
                            szStringVal = LoadLangText_ById(216);
                        }
                        break;
                    case STATU_ACDC_INVERTER:      // 放电
                        {
                            nESCStatus = 2;
                            szStringVal = LoadLangText_ById(502);
                        }
                        break;
                    default:
                        {
                            if (dataBloCmd109.clientActiveAlarms[i].iAlarmCount > 0)
                            {
                                nESCStatus = 3; // 有告警
                                szStringVal = LoadLangText_ById(553);
                            }
                            else
                            {
                                nESCStatus = 5; // 通信正常
                                szStringVal = LoadLangText_ById(34);
                            }
                        }
                        break;
                }            
            }
            cJSON_AddStringToObject(pJStatus, "label", szStringVal);
            cJSON_AddNumberToObject(pJStatus, "value", nESCStatus);
            cJSON_AddItemToObject(pJESCInfoItem, "status", pJStatus);
            cJSON* pJData = cJSON_CreateArray();
            // 单柜 SOC
            char szESC_SOC[64] = {0};
            snprintf(szESC_SOC, 64, "SOC : %d %%", singleESCInfo.escDatas[i].batt_soc);
            cJSON_AddItemToArray(pJData, cJSON_CreateString(szESC_SOC));
            // 单柜电池直流侧电压
            char szESC_DC_Volt[64] = {0};
            snprintf(szESC_DC_Volt, 64, "DC Voltage : %.1f V", singleESCInfo.escDatas[i].batt_volt);
            cJSON_AddItemToArray(pJData, cJSON_CreateString(szESC_DC_Volt));
            // 单柜电池直流侧电流
            char szESC_DC_Curr[64] = {0};
            snprintf(szESC_DC_Curr, 64, "DC Current : %.1f A", singleESCInfo.escDatas[i].batt_curr);
            cJSON_AddItemToArray(pJData, cJSON_CreateString(szESC_DC_Curr));
            // 单柜电表有功功率
            char szESC_Cab_Power[64] = {0};
            snprintf(szESC_Cab_Power, 64, "Cab Meter P : %.1f kW", singleESCInfo.escDatas[i].kw_meter_cab);
            cJSON_AddItemToArray(pJData, cJSON_CreateString(szESC_Cab_Power));
            cJSON_AddItemToObject(pJESCInfoItem, "data", pJData);
            cJSON_AddItemToArray(pJESCInfo, pJESCInfoItem);
        }
        // printf("pJESCInfo = %s\n", cJSON_Print(pJESCInfo));
        // printf("pJSON_Resp = %s\n", cJSON_Print(pFCGI_Rough->pJSON_Resp));
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "deviceData", pJESCInfo);

        // 负载
        // float nPLoad = clientInfoSum.f_PgridMeterSum - clientInfoSum.f_PcabMeterSum;
        float f_P_Cab;
        if (emInstallCabaleSum == 0)
        {
            f_P_Cab = clientInfoSum.f_PcabMeterSum; // 没有柜内总表用柜内表加和数据
        }
        else
        {
            f_P_Cab = esRunData.f_PcabMeter; // 有柜内表总表用表数据
        }
        float nPLoad = esRunData.f_PgridMeter - f_P_Cab;
        char szPLoad[64] = {0};
        snprintf(szPLoad, 64, "Pload : %.1f kW", nPLoad);
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "loadData", cJSON_CreateString(szPLoad));
        int nflow1;
        if (nPLoad >= 0)
        {
            nflow1 = 0;
        }
        else
        {
            nflow1 = 1;
        }
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "flow1", nflow1);

        // printf("pJSON_Resp = %s\n", cJSON_Print(pFCGI_Rough->pJSON_Resp));
    }
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
                // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_type", 3);  
                // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_level", 5);
                TRACEW("sever_conn_status = 2; signal_type = 3; signal_level = 5");
            }
            else
            {
                cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "sever_conn_status", 1);
                // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_level", 0);   
                // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "signal_type", 0);  
                TRACEW("sever_conn_status = 1; signal_type = 0; signal_level = 0");
            }     
        }
    }
    //edit by Kuangyue
    //sw versions
    // char szVVV[32];
    // memset(szVVV, 0, 32);
    // snprintf(szVVV, 32, "%s, %s", pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_SWVer,
    //     pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_HWVer);
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "swver_immu2", szVVV); 

    // memset(szVVV, 0, 32);
    // snprintf(szVVV, 32, "%s", pFCGI_Rough->chargerDataRepo.versionInfos.szIMSU_D_Vers);
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "swver_imsud", szVVV);

    // memset(szVVV, 0, 32);
    // snprintf(szVVV, 32, "%s", pFCGI_Rough->chargerDataRepo.versionInfos.szSTBatt_Ver);
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "swver_rectifier", szVVV);  


    //---  Tab or Hint DataInfo ---//
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "topology", LoadLangText_ById(2070));
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "scene_graph", LoadLangText_ById(2072));
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "energy_stack", LoadLangText_ById(2373));
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "no_device", LoadLangText_ById(84));
    // cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "off_grid", LoadLangText_ById(2073));


    //---  System Type DataInfo ---//
    int nSystemType = 0;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 8, 4, (void*)&nSystemType))
    {
        TRACEX("----ERRRRRR: can not get System Type DataInfo----\n");
        return TRUE;
    }
    else
    {
       // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "system_type", nSystemType);
    }


    //---  Stat Graph Types DataInfo ---//
    // cJSON* pJSStatGraphTypesTotalInfo = cJSON_CreateArray();

    // cJSON* pJSStatGraphTypesInfo = cJSON_CreateObject();
    // cJSON_AddStringToObject(pJSStatGraphTypesInfo, "name", LoadLangText_ById(826));
    // cJSON_AddNumberToObject(pJSStatGraphTypesInfo, "val", 0);
    // cJSON_AddItemToArray(pJSStatGraphTypesTotalInfo, pJSStatGraphTypesInfo);

    // pJSStatGraphTypesInfo = cJSON_CreateObject();
    // cJSON_AddStringToObject(pJSStatGraphTypesInfo, "name", LoadLangText_ById(827));
    // cJSON_AddNumberToObject(pJSStatGraphTypesInfo, "val", 1);
    // cJSON_AddItemToArray(pJSStatGraphTypesTotalInfo, pJSStatGraphTypesInfo);

    // pJSStatGraphTypesInfo = cJSON_CreateObject();
    // cJSON_AddStringToObject(pJSStatGraphTypesInfo, "name", LoadLangText_ById(830));
    // cJSON_AddNumberToObject(pJSStatGraphTypesInfo, "val", 2);
    // cJSON_AddItemToArray(pJSStatGraphTypesTotalInfo, pJSStatGraphTypesInfo);

    // cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "types", pJSStatGraphTypesTotalInfo);


    // cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dictionary", pDictionary);
    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handle_SingleCell_Info
*  CREATOR       : Kuangyue
*  CREATED DATE  : 2024-11-21 15:45
*  DESCRIPTION   : Used to reload database resources, update battery abnormal cell information, 
*                  and create the cabinet information and count the number of abnormal cells 
*                  in each pack. The result is stored in the response JSON object for sending to the frontend.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
//edit by Kuangyue 20241114
static BOOL Handle_SingleCell_Info(FCGI_ROUGH_DATA* pFCGI_Rough, A_BLOB_IFY_BMS_DATAINFO* bmsDataInfo, int nPack_num, int nSingleCell_num, A_BLOB_CELL_BARCODE* CellBarcode)
{
    TRACED("---Handle_SingleCell_Info!!!---\n");
    cJSON *pCabinetList = cJSON_CreateArray();
    int cabinetIdx;
    for (cabinetIdx = 0; cabinetIdx < SYSTYPE_BATTERY_NUM; cabinetIdx++) // assuming there are 4 cabinets ：1-4个电池柜
    {
        int packIdx;
        cJSON* pCabinet = cJSON_CreateObject();
        cJSON* pPackList = cJSON_CreateArray();
        
        // Cabinet information
        cJSON_AddStringToObject(pCabinet, "name", LoadLangText_ById(346 + cabinetIdx));  // Name of the cabinet
        cJSON_AddNumberToObject(pCabinet, "val", cabinetIdx);
        
        for (packIdx = 1; packIdx <= nPack_num; packIdx++) 
        {
            cJSON* pSerialList = cJSON_CreateObject();
            char serialStr[8];
            
            int abnormalCount = 0; // Abnormal cell counter for this pack
            int startIdx = nSingleCell_num * (packIdx - 1);
            int endIdx = nSingleCell_num * packIdx;
            int i;
            for (i = startIdx; i < endIdx; i++)
            {
                // Check if any condition is met to mark the cell as abnormal

                if (bmsDataInfo->cCellTemp[i] < -20.0 || bmsDataInfo->cCellTemp[i] > 38.0 )
                {
                    abnormalCount++;
                }
            }
            snprintf(serialStr, sizeof(serialStr), "%d", packIdx);
            cJSON_AddStringToObject(pSerialList, "serial", serialStr);
            cJSON_AddNumberToObject(pSerialList, "alarm", abnormalCount);
            
            if (CellBarcode != NULL) {
                int nBarcodePackIdx = packIdx - 1;
                char* pPackBarcode = CellBarcode->cASCIIPackBarcode[nBarcodePackIdx];
                char* pBSUBarcode = CellBarcode->cASCIIBSUBarcode[nBarcodePackIdx];
                cJSON_AddStringToObject(pSerialList, "pack_barcode", pPackBarcode);
                cJSON_AddStringToObject(pSerialList, "bsu_barcode", pBSUBarcode);
            }

            cJSON_AddItemToArray(pPackList, pSerialList);
        }
        cJSON_AddItemToObject(pCabinet, "packList", pPackList);
        cJSON_AddItemToArray(pCabinetList, pCabinet);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "cabinet_select", pCabinetList);

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Handel_Get_BattCellInfo
*  CREATOR       : Kuangyue
*  CREATED DATE  : 2024-11-21 20:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static BOOL Handel_Get_BattCellInfo(FCGI_ROUGH_DATA* pFCGI_Rough, A_BLOB_IFY_BMS_DATAINFO* BmsDataInfo, int nPackIdx, int nPacknum, int nSingleCell_num, A_BLOB_CELL_BARCODE* CellBarcode)
{
    TRACED("---Handel_Get_BattCellInfo!!!---\n");
    char szStringName[32] = "\0";
    cJSON* pSerialNameInfo = cJSON_CreateObject();
    int startIdx = 0, endIdx = 0;

    cJSON* pCellList = cJSON_CreateArray();
    cJSON_AddStringToObject(pSerialNameInfo, "serialname", LoadLangText_ById(6276));//电池编号
    cJSON_AddStringToObject(pSerialNameInfo, "voltname", LoadLangText_ById(326));//电压
    cJSON_AddStringToObject(pSerialNameInfo, "resistancename", LoadLangText_ById(6277));//内阻
    // cJSON_AddStringToObject(pSerialNameInfo, "balancename", LoadLangText_ById(6278));//均衡态

    cJSON_AddStringToObject(pSerialNameInfo, "bchargename", LoadLangText_ById(9052));//均充状态
    cJSON_AddStringToObject(pSerialNameInfo, "bdischargename", LoadLangText_ById(9053));//均放状态

    cJSON_AddStringToObject(pSerialNameInfo, "tempname", LoadLangText_ById(6279));//温度

    cJSON *pItem;
    cJSON_ArrayForEach(pItem, pSerialNameInfo)
    {
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, pItem->string, cJSON_Duplicate(pItem, 1));
    }

    startIdx = nSingleCell_num * (nPackIdx - 1);
    endIdx = nSingleCell_num * nPackIdx;

    int nBarcodePackIdx = nPackIdx - 1;

    int i;
    for (i = startIdx; i < endIdx; i++)
    {
        cJSON* pJSTableInfo = cJSON_CreateObject();

        // nCellNo从1
        int nBarcodeCellIdx = (i - startIdx) + 1;

        // Add cell serial number and data to JSON object
        snprintf(szStringName, 32, "%d", i); // Serial number
        cJSON_AddStringToObject(pJSTableInfo, "serial", szStringName);

        snprintf(szStringName, 32, "%.2f", (float)BmsDataInfo->sCellTemp_836[i] * 0.1); // Temperature
        cJSON_AddStringToObject(pJSTableInfo, "temp", szStringName);

        snprintf(szStringName, 32, "%.3f", (float)BmsDataInfo->usCellVolt_836[i] * 0.001); // Voltage
        cJSON_AddStringToObject(pJSTableInfo, "volt", szStringName);

        // snprintf(szStringName, 32, "%.2f", BmsDataInfo->fCellRes[i]); // Resistance
        // cJSON_AddStringToObject(pJSTableInfo, "resistance", szStringName);

        // snprintf(szStringName, 32, "%d", BmsDataInfo->byCellBalanceStat[i]); // Balance status
        // cJSON_AddStringToObject(pJSTableInfo, "balance", szStringName);

        snprintf(szStringName, 32, "%d", BmsDataInfo->byCellBalanceStat_836[i]); // Charge Balance status
        cJSON_AddStringToObject(pJSTableInfo, "balancecharge", szStringName);

        snprintf(szStringName, 32, "%d", BmsDataInfo->byCellDischargeStat_836[i]); // Discharge Balance status
        cJSON_AddStringToObject(pJSTableInfo, "balancedischarge", szStringName);

        snprintf(szStringName, 32, "%.2f", (float)BmsDataInfo->uiCellResReal_836[i]*0.001); // Resistance
        cJSON_AddStringToObject(pJSTableInfo, "resistance", szStringName);

        // snprintf(szStringName, 32, "%.2f", BmsDataInfo->fSOC); // SOC
        // cJSON_AddStringToObject(pJSTableInfo, "soc", szStringName);

        // snprintf(szStringName, 32, "%.2f", BmsDataInfo->fSOH); // SOH
        // cJSON_AddStringToObject(pJSTableInfo, "soh", szStringName);

        int status = 0; // Default is normal
        if (BmsDataInfo->cCellTemp[i] < -20.0 || BmsDataInfo->cCellTemp[i] > 38.0) 
        {
            status = 1;
        }
        
        char* pBarcode = "";
        if (CellBarcode != NULL)
        {
            if (nBarcodeCellIdx >= 1 && nBarcodeCellIdx <= 52)
            {
                pBarcode = CellBarcode->cASCIICellBarcode[nBarcodePackIdx][nBarcodeCellIdx];

                BOOL bValid = FALSE;
                if (pBarcode != NULL && pBarcode[0] != '\0')
                {
                    int j;
                    for (j = 0; j < 24 && pBarcode[j] != '\0'; j++)
                    {
                        if (pBarcode[j] >= 32 && pBarcode[j] <= 126)
                        {
                            bValid = TRUE;
                            break;
                        }
                    }
                }
                
                if (!bValid)
                {
                    pBarcode = "";
                }
            }
            else
            {
                pBarcode = "";
            }
        }
        cJSON_AddStringToObject(pJSTableInfo, "barcode", pBarcode);
        
        cJSON_AddNumberToObject(pJSTableInfo, "status", status);

        cJSON_AddItemToArray(pCellList, pJSTableInfo);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "cellList", pCellList);

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : Process_BMS_Data
*  CREATOR       : Kuangyue
*  CREATED DATE  : 2024-11-26 20:36
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/

static void Process_BMS_Data(int nCabIdx, int nPackIdx, int nPacknum, int nSingleCell_num, void *pFCGI_Rough) 
{
    if (nPackIdx >= 1 && nPackIdx <= nPacknum) 
    {
        A_BLOB_IFY_BMS_DATAINFO bmsDataInfo;
        
        if (!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 501 + nCabIdx, 557, sizeof(bmsDataInfo), (void*)&bmsDataInfo)) 
        {
            TRACEX("----ERRRRRR: can not get SingleCell StatData----\n");
            return;
        }
        
        A_BLOB_CELL_BARCODE cellBarcode;
        
        if (!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 500, 2612, sizeof(cellBarcode), (void*)&cellBarcode)) 
        {
            TRACEX("----ERRRRRR: can not get SingleCell Barcode----\n");
            return;
        }

        Handle_SingleCell_Info(pFCGI_Rough, &bmsDataInfo, nPacknum, nSingleCell_num, &cellBarcode);
        
        Handel_Get_BattCellInfo(pFCGI_Rough, &bmsDataInfo, nPackIdx, nPacknum, nSingleCell_num, &cellBarcode);  // 把电池单体的信息传递给home
    }
}

static BOOL Handle_Home_ChartData(FCGI_ROUGH_DATA* pFCGI_Rough, int nChartIdx, int nDevtype, int nRange, int nCabIdx, int nPackIdx, int nTimeType, int nPageId, char szQueryTime[], int nPageVol, int nDaySector) //edit by Kuangyue 20241107
{
    TRACEX("\n----now handling home_page chart data----\n\n");
    WEB_PARAM_VALUE* pWebParaVal;
    char* szStringVal;
    int n;

    //get system stat info
    A_BLOB_ES_STATISTIC_DATA esStatData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 79, sizeof(A_BLOB_ES_STATISTIC_DATA), (void*)&esStatData))
    {
        TRACEX("----ERRRRRR: can not get ES StatData----\n");
        return TRUE;
    }

    if(nChartIdx == STAT_GRAPH_ID_ENERGY_STAT)
    {
        cJSON* pJSTimeSelectTotalInfo = cJSON_CreateArray();
        cJSON* pJSHeaderTotalInfo = cJSON_CreateObject();

        //time select info
        cJSON* pJSTimeSelectInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSTimeSelectInfo, "name", LoadLangText_ById(828));
        cJSON_AddNumberToObject(pJSTimeSelectInfo, "val", 0);
        cJSON_AddItemToArray(pJSTimeSelectTotalInfo, pJSTimeSelectInfo);

        pJSTimeSelectInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSTimeSelectInfo, "name", LoadLangText_ById(825));
        cJSON_AddNumberToObject(pJSTimeSelectInfo, "val", 1);
        cJSON_AddItemToArray(pJSTimeSelectTotalInfo, pJSTimeSelectInfo);

        pJSTimeSelectInfo = cJSON_CreateObject();
        cJSON_AddStringToObject(pJSTimeSelectInfo, "name", LoadLangText_ById(823));
        cJSON_AddNumberToObject(pJSTimeSelectInfo, "val", 2);
        cJSON_AddItemToArray(pJSTimeSelectTotalInfo, pJSTimeSelectInfo);

        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "time_select", pJSTimeSelectTotalInfo);
        TRACED("STAT_GRAPH_ID_ENERGY_STAT OK!!!!!!!!!!!!!");
        //table header info
        cJSON_AddStringToObject(pJSHeaderTotalInfo, "time", "Time");

        cJSON* pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("UserAC"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("IN/OUT"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("kWh"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "userac", pJSHeaderInfo);

        pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("CabAC"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("IN/OUT"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("kWh"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "cabac", pJSHeaderInfo);

        pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("PV1/2/3/4"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("kWh"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "pv", pJSHeaderInfo);

        pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("BAT#1"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("(Chrg/Dischg)"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "bat1", pJSHeaderInfo);

        pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("BAT#2"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("(Chrg/Dischg)"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "bat2", pJSHeaderInfo);

        pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("BAT#3"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("(Chrg/Dischg)"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "bat3", pJSHeaderInfo);

        pJSHeaderInfo = cJSON_CreateArray();
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("BAT#4"));
        cJSON_AddItemToArray(pJSHeaderInfo, cJSON_CreateString("(Chrg/Dischg)"));
        cJSON_AddItemToObject(pJSHeaderTotalInfo, "bat4", pJSHeaderInfo);

        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "table_header", pJSHeaderTotalInfo);

        if(nTimeType == 0)
        {
            //page info
            int max_rec = MAX_HOURS_SECTOR;
            //int max_row = MAX_TABLE_ROW;
            int max_row = nPageVol;
            TRACEX("pagevol = %d\n", nPageVol);
            int nTotalPage = max_rec / max_row + ((max_rec % max_row == 0)? 0 : 1);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", max_rec);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", nPageId);

            //table list
            cJSON* pJSTableTotalInfo = cJSON_CreateArray();
            char szStringName[32] = "\0";

            for(n = 0; n < nPageVol; n++)
            {
                cJSON* pJSTableInfo = cJSON_CreateObject();
                int nId = n + nPageId * nPageVol;
                if(nId >= MAX_HOURS_SECTOR){break;}

                snprintf(szStringName, 32, "%02d:%02d~%02d:%02d",
                nId / 2, nId % 2 * 30,
                (nId + 1) / 2, (nId + 1) % 2 * 30);
                cJSON_AddStringToObject(pJSTableInfo, "time", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.hours_Data.fAC_CostKWH_Sector[nId],
                esStatData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[nId]);
                cJSON_AddStringToObject(pJSTableInfo, "userac", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[nId],
                esStatData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[nId]);
                cJSON_AddStringToObject(pJSTableInfo, "cabac", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f|%.1f|%.1f",
                esStatData.es_Data.hours_Data.fMppt_GKWH_Sector[0][nId],
                esStatData.es_Data.hours_Data.fMppt_GKWH_Sector[1][nId],
                esStatData.es_Data.hours_Data.fMppt_GKWH_Sector[2][nId],
                esStatData.es_Data.hours_Data.fMppt_GKWH_Sector[3][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "pv", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[0][nId],
                esStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[0][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat1", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[1][nId],
                esStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[1][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat2", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[2][nId],
                esStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[2][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat3", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[3][nId],
                esStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[3][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat4", szStringName);

                cJSON_AddItemToArray(pJSTableTotalInfo, pJSTableInfo);
            }

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "table_list", pJSTableTotalInfo);
        }

        else if(nTimeType == 1)
        {
            //page info
            int max_rec = MAX_YEARS_SECTOR;
            //int max_row = MAX_TABLE_ROW;
            int max_row = nPageVol;
            int nTotalPage = max_rec / max_row + ((max_rec % max_row == 0)? 0 : 1);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", max_rec);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", nPageId);

            //table list
            cJSON* pJSTableTotalInfo = cJSON_CreateArray();
            time_t tmTimeStamp;
            struct tm *tmTime;
            char szStringTime[32] = "\0";
            char szStringName[32] = "\0";

            for(n = 0; n < nPageVol; n++)
            {
                cJSON* pJSTableInfo = cJSON_CreateObject();
                int nId = n + nPageId * nPageVol;
                if(nId >= MAX_YEARS_SECTOR){break;}

                tmTimeStamp = esStatData.es_Data.years_Data.tmTimeStamp_Y[nId];
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01
                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d",
                        tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                        tmTime->tm_hour, tmTime->tm_min);
                cJSON_AddStringToObject(pJSTableInfo, "time", szStringTime);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.years_Data.fACTotalCostKWH_Y[nId],
                esStatData.es_Data.years_Data.fAC_RevrtOutPutKWH_Y[nId]);
                cJSON_AddStringToObject(pJSTableInfo, "userac", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.years_Data.fLocalAC_INPUTKWH_Y[nId],
                esStatData.es_Data.years_Data.fLocalAC_OUTPUTKWH_Y[nId]);
                cJSON_AddStringToObject(pJSTableInfo, "cabac", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f|%.1f|%.1f",
                esStatData.es_Data.years_Data.fMpptTotalGKWH_Y[0][nId],
                esStatData.es_Data.years_Data.fMpptTotalGKWH_Y[1][nId],
                esStatData.es_Data.years_Data.fMpptTotalGKWH_Y[2][nId],
                esStatData.es_Data.years_Data.fMpptTotalGKWH_Y[3][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "pv", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[0][nId],
                esStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[0][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat1", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[1][nId],
                esStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[1][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat2", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[2][nId],
                esStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[2][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat3", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[3][nId],
                esStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[3][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat4", szStringName);

                cJSON_AddItemToArray(pJSTableTotalInfo, pJSTableInfo);
            }

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "table_list", pJSTableTotalInfo);
        }

        else if(nTimeType == 2)
        {
            //page info
            int max_rec = MAX_DAYS_SECTOR;
            //int max_row = MAX_TABLE_ROW;
            int max_row = nPageVol;
            int nTotalPage = max_rec / max_row + ((max_rec % max_row == 0)? 0 : 1);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", MAX_DAYS_SECTOR);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", nPageId);

            //table list
            cJSON* pJSTableTotalInfo = cJSON_CreateArray();
            time_t tmTimeStamp;
            struct tm *tmTime;
            char szStringTime[32] = "\0";
            char szStringName[32] = "\0";

            for(n = 0; n < nPageVol; n++)
            {
                cJSON* pJSTableInfo = cJSON_CreateObject();
                int nId = n + nPageId * nPageVol;
                if(nId >= MAX_DAYS_SECTOR){break;}

                tmTimeStamp = esStatData.es_Data.days_Data.tmTimeStamp_D[nId];
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01
                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d",
                        tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                        tmTime->tm_hour, tmTime->tm_min);
                cJSON_AddStringToObject(pJSTableInfo, "time", szStringTime);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.days_Data.fACTotalCostKWH_D[nId],
                esStatData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[nId]);
                cJSON_AddStringToObject(pJSTableInfo, "userac", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nId],
                esStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nId]);
                cJSON_AddStringToObject(pJSTableInfo, "cabac", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f|%.1f|%.1f",
                esStatData.es_Data.days_Data.fMpptTotalGKWH_D[0][nId],
                esStatData.es_Data.days_Data.fMpptTotalGKWH_D[1][nId],
                esStatData.es_Data.days_Data.fMpptTotalGKWH_D[2][nId],
                esStatData.es_Data.days_Data.fMpptTotalGKWH_D[3][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "pv", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.days_Data.fBattTotalChrgKWH_D[0][nId],
                esStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[0][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat1", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.days_Data.fBattTotalChrgKWH_D[1][nId],
                esStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[1][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat2", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.days_Data.fBattTotalChrgKWH_D[2][nId],
                esStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[2][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat3", szStringName);

                snprintf(szStringName, 32, "%.1f|%.1f",
                esStatData.es_Data.days_Data.fBattTotalChrgKWH_D[3][nId],
                esStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[3][nId]);
                cJSON_AddStringToObject(pJSTableInfo, "bat4", szStringName);

                cJSON_AddItemToArray(pJSTableTotalInfo, pJSTableInfo);
            }

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "table_list", pJSTableTotalInfo);
        }
    }

    if(nChartIdx == STAT_GRAPH_ID_ENERGY_TREND)
    {
        // if(nRange == 0)
        // {
        //     cJSON* pJSTrendDataX = cJSON_CreateArray();
        //     cJSON* pJSTrendDataY = cJSON_CreateArray();
        //     char szStringValue[32] = "\0";
        //     char  szStringTime[32] = "\0";
        //     time_t tmTimeStamp;
        //     struct tm *tmTime;
        //     float fValue = 0.0;
        //     int m;

        //     cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "kWh");

        //     for(n = 0; n < MAX_WEEKLY_SECTOR; n++)
        //     {
        //         for(m = 0; m < MAX_HOURS_SECTOR; m++)
        //         {
        //             tmTimeStamp = esStatData.es_Data.week_Data.tmTimeStamp_W[n][m];
        //             tmTime = gmtime(&tmTimeStamp);
        //             if(tmTime->tm_year != 70){break;}  //1970-01-01
        //         }
        //         snprintf(szStringTime, 32, "%d-%.2d-%.2d",
        //                 tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday);
        //         cJSON_AddItemToArray(pJSTrendDataX, cJSON_CreateString(szStringTime));
        //     }
        //     cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dataAxis", pJSTrendDataX);

        //     for(n = 0; n < MAX_WEEKLY_SECTOR; n++)
        //     {
        //         if(nDevtype == STAT_DEV_TYPE_AC)
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fACTotalCostKWH_W[n][m];}
        //         }
        //         else if(nDevtype == STAT_DEV_TYPE_MPPT1)
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fMpptTotalGKWH_W[0][n][m];}
        //         }
        //         else if(nDevtype == STAT_DEV_TYPE_MPPT2)
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fMpptTotalGKWH_W[1][n][m];}
        //         }
        //         else if(nDevtype == STAT_DEV_TYPE_BATT1_CHRG)
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fBattTotalChrgKWH_W[0][n][m];}
        //         }
        //         else if(nDevtype == STAT_DEV_TYPE_BATT1_DISCHRG)
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fBattTotalDisChrgKWH_W[0][n][m];}   
        //         }
        //         else if(nDevtype == STAT_DEV_TYPE_BATT2_CHRG)
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fBattTotalChrgKWH_W[1][n][m];}
        //         }
        //         else
        //         {
        //             for(m = 0; m < MAX_HOURS_SECTOR; m++){fValue += esStatData.es_Data.week_Data.fBattTotalDisChrgKWH_W[1][n][m];}
        //         }

        //         cJSON_AddItemToArray(pJSTrendDataY, cJSON_CreateNumber(fValue));
        //         fValue = 0.0;
        //     }
        //     cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "electricity_data", pJSTrendDataY);
        // }

        if(nRange == 1)
        {
            cJSON* pJSHistogramDataX = cJSON_CreateArray();
            cJSON* pJSHistogramDataY = cJSON_CreateArray();
            char szStringValue[32] = "\0";
            float fValue;

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "KWH");

            for(n = 0; n < MAX_HOURS_SECTOR; n++)
            {
                snprintf(szStringValue, 32, "%.2d:%.2d", n / 2, (n % 2) * 30);
                cJSON_AddItemToArray(pJSHistogramDataX, cJSON_CreateString(szStringValue));
            }
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dataAxis", pJSHistogramDataX);

            for(n = 0; n < MAX_HOURS_SECTOR; n++)
            {
                if(nDevtype == STAT_DEV_TYPE_AC)
                {
                    fValue = esStatData.es_Data.hours_Data.fAC_CostKWH_Sector[n];
                }
                else if(nDevtype == STAT_DEV_TYPE_MPPT1)
                {
                    fValue = esStatData.es_Data.hours_Data.fMppt_GKWH_Sector[0][n];
                }
                else if(nDevtype == STAT_DEV_TYPE_MPPT2)
                {
                    fValue = esStatData.es_Data.hours_Data.fMppt_GKWH_Sector[1][n];
                }
                else if(nDevtype == STAT_DEV_TYPE_BATT1_CHRG)
                {
                    fValue = esStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[0][n];
                }
                else if(nDevtype == STAT_DEV_TYPE_BATT1_DISCHRG)
                {
                    fValue = esStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[0][n];
                }
                else if(nDevtype == STAT_DEV_TYPE_BATT2_CHRG)
                {
                    fValue = esStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[1][n];
                }
                else
                {
                    fValue = esStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[1][n];
                }

                cJSON_AddItemToArray(pJSHistogramDataY, cJSON_CreateNumber(fValue));
            }
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "electricity_data", pJSHistogramDataY);
        }
    }

    if(nChartIdx == STAT_GRAPH_ID_BATT_SOC)
    {
        time_t tmTimeStamp;
        struct tm *tmTime;
        char  szStringTime[32] = "\0";
        char  szStringName[32] = "\0";
        cJSON* pJSSocDataAxisY = cJSON_CreateArray();
        float fValue;
        int m;

        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "%");

        for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
        {
            cJSON* pJSSocDataSingleBatt = cJSON_CreateObject();
            cJSON* pJSSocDataSingleBattValueList = cJSON_CreateArray();

            snprintf(szStringName, 32, "SOC%d", n + 1);
            cJSON_AddStringToObject(pJSSocDataSingleBatt, "name", szStringName);

            for(m = 0; m < esStatData.es_Data.hours_Data.nSOC_STBatt_TotalPoint[n]; m++)
            {
                cJSON* pJSSocDataSingleBattValue = cJSON_CreateArray();

                tmTimeStamp = esStatData.es_Data.hours_Data.soc_RT_STBatt[n][m].tmTimeStamp;
                tmTime = gmtime(&tmTimeStamp);
                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d",
                        tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                        tmTime->tm_hour, tmTime->tm_min);
                
                fValue = esStatData.es_Data.hours_Data.soc_RT_STBatt[n][m].fValue;
                cJSON_AddItemToArray(pJSSocDataSingleBattValue, cJSON_CreateString(szStringTime));
                cJSON_AddItemToArray(pJSSocDataSingleBattValue, cJSON_CreateNumber(fValue));
                cJSON_AddItemToArray(pJSSocDataSingleBattValueList, pJSSocDataSingleBattValue);
            }
            cJSON_AddItemToObject(pJSSocDataSingleBatt, "val_list", pJSSocDataSingleBattValueList);
            cJSON_AddItemToArray(pJSSocDataAxisY, pJSSocDataSingleBatt);
        }

        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "soc_data", pJSSocDataAxisY);
    }

    if(nChartIdx == STAT_GRAPH_ID_TEMP_CURVE)
    {
        if(nRange == 0)
        {
            time_t tmTimeStamp;
            struct tm *tmTime;
            char  szStringTime[32] = "\0";
            char  szStringName[32] = "\0";
            UINT   szTimeList[WEB_SUPPORT_MAX_BMSREC];
            UINT   nTempValue;
            cJSON* pJSTempDataAxisY = cJSON_CreateArray();
            cJSON* pJSTempDataAxisX = cJSON_CreateArray();
            float fValue;
            int m;
            QUERYCOND_HISMETERVAL cond;
            BMS_RUNTIMEDATA_LIST* pList = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.bmsRunData;
            int nTotal;

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "\u2103");

            //Max Temp prepare
            cJSON* pJSTempDataMaxTemp = cJSON_CreateObject();
            cJSON* pJSTempDataMaxTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataMaxTemp, "name", LoadLangText_ById(675));

            //Min Temp prepare
            cJSON* pJSTempDataMinTemp = cJSON_CreateObject();
            cJSON* pJSTempDataMinTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataMinTemp, "name", LoadLangText_ById(676));

            //Temp Diff prepare
            cJSON* pJSTempDataTempDiff = cJSON_CreateObject();
            cJSON* pJSTempDataTempDiffValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataTempDiff, "name", LoadLangText_ById(6018));

            //InWater Temp prepare
            cJSON* pJSTempDataInWaterTemp = cJSON_CreateObject();
            cJSON* pJSTempDataInWaterTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataInWaterTemp, "name", LoadLangText_ById(5544));

            //OutWater Temp prepare
            cJSON* pJSTempDataOutWaterTemp = cJSON_CreateObject();
            cJSON* pJSTempDataOutWaterTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataOutWaterTemp, "name", LoadLangText_ById(5545));

            //Water Temp Diff prepare
            cJSON* pJSTempDataWaterTempDiff = cJSON_CreateObject();
            cJSON* pJSTempDataWaterTempDiffValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataWaterTempDiff, "name", LoadLangText_ById(6022));

            for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
            {
                szTimeList[n] = pFCGI_Rough->chargerDataRepo.bmsRecData[n].nDay;
            }

            for(n = WEB_SUPPORT_MAX_BMSREC - 2; n >= 0; n--)
            {
                for(m = 0; m <= n; m++)
                {
                    if(szTimeList[m] > szTimeList[m + 1])
                    {
                        nTempValue = szTimeList[m];
                        szTimeList[m] = szTimeList[m + 1];
                        szTimeList[m + 1] = nTempValue;
                    }
                }
            }

            for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
            {
                cond.nGunIdx = szTimeList[n];
                if(!QueryChgRunData_byCond(&cond))
                {
                    TRACEX("Did not find such bms_rundata record!");
                    //Make_Web_OutPut(104, "Did not find such bms_rundata record!", NULL, NULL);
                    continue;
                }
                
                nTotal = MIN(pList->nBmsRuntimeDataTotal, MAX_HIS_BMS_RUNTIME_DATALIST);

                //Max Temp, Min Temp, Max Temp Diff
                float fMaxTempDiff = 0;
                float fMaxTemp;
                float fMinTemp;
                time_t tmTimeStampMaxTempDiff;
                for(m = 0; m < nTotal; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}

                    if(pList->stBmsRuntimeDataList[m].fMax_CellTemp - pList->stBmsRuntimeDataList[m].fMin_CellTemp > fMaxTempDiff && pList->stBmsRuntimeDataList[m].fMin_CellTemp > 0)
                    {
                        fMaxTempDiff = pList->stBmsRuntimeDataList[m].fMax_CellTemp - pList->stBmsRuntimeDataList[m].fMin_CellTemp;
                        fMaxTemp = pList->stBmsRuntimeDataList[m].fMax_CellTemp;
                        fMinTemp = pList->stBmsRuntimeDataList[m].fMin_CellTemp;
                    }
                }

                if(fMaxTempDiff == 0){continue;}

                cJSON_AddItemToArray(pJSTempDataMaxTempValueList, cJSON_CreateFloat(fMaxTemp, 1));
                cJSON_AddItemToArray(pJSTempDataMinTempValueList, cJSON_CreateFloat(fMinTemp, 1));
                cJSON_AddItemToArray(pJSTempDataTempDiffValueList, cJSON_CreateFloat(fMaxTempDiff, 1));

                //InWater Temp, OutWater Temp, Max In-Out Water Temp Diff
                float fMaxInOutWaterTempDiff = 0;
                float fInWaterTemp = pList->stBmsRuntimeDataList[0].fBattInWaterTemp;
                float fOutWaterTemp = pList->stBmsRuntimeDataList[0].fBattOutWaterTemp;
                time_t tmTimeStampMaxInOutWaterTempDiff;
                for(m = 0; m < nTotal; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}

                    if(pList->stBmsRuntimeDataList[m].fBattInWaterTemp - pList->stBmsRuntimeDataList[m].fBattOutWaterTemp > fMaxInOutWaterTempDiff && pList->stBmsRuntimeDataList[m].fBattOutWaterTemp > 0)
                    {
                        fMaxInOutWaterTempDiff = pList->stBmsRuntimeDataList[m].fBattInWaterTemp - pList->stBmsRuntimeDataList[m].fBattOutWaterTemp;
                        fInWaterTemp = pList->stBmsRuntimeDataList[m].fBattInWaterTemp;
                        fOutWaterTemp = pList->stBmsRuntimeDataList[m].fBattOutWaterTemp;
                    }
                }

                cJSON_AddItemToArray(pJSTempDataInWaterTempValueList, cJSON_CreateFloat(fInWaterTemp, 1));
                cJSON_AddItemToArray(pJSTempDataOutWaterTempValueList, cJSON_CreateFloat(fOutWaterTemp, 1));

                tmTimeStamp = szTimeList[n];
                tmTime = gmtime(&tmTimeStamp);
                snprintf(szStringTime, 32, "%d-%.2d-%.2d",
                                tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday);
                cJSON_AddItemToArray(pJSTempDataAxisX, cJSON_CreateString(szStringTime));
            }

            cJSON_AddItemToObject(pJSTempDataMaxTemp, "val_list", pJSTempDataMaxTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataMaxTemp);

            cJSON_AddItemToObject(pJSTempDataMinTemp, "val_list", pJSTempDataMinTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataMinTemp);

            cJSON_AddItemToObject(pJSTempDataTempDiff, "val_list", pJSTempDataTempDiffValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataTempDiff);

            cJSON_AddItemToObject(pJSTempDataInWaterTemp, "val_list", pJSTempDataInWaterTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataInWaterTemp);
            
            cJSON_AddItemToObject(pJSTempDataOutWaterTemp, "val_list", pJSTempDataOutWaterTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataOutWaterTemp);

            cJSON_AddItemToObject(pJSTempDataWaterTempDiff, "val_list", pJSTempDataWaterTempDiffValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataWaterTempDiff);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "temp_data", pJSTempDataAxisY);
            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dataAxis", pJSTempDataAxisX);
        }

        if(nRange == 1)
        {
            time_t tmTimeStamp;
            struct tm *tmTime;
            char  szStringTime[32] = "\0";
            char  szStringName[32] = "\0";
            char  szQueryTime32[32] = "\0";
            cJSON* pJSTempDataAxisY = cJSON_CreateArray();
            float fValue;
            int m;
            QUERYCOND_HISMETERVAL cond;
            BMS_RUNTIMEDATA_LIST* pList = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.bmsRunData;
            int nFlag = 0;
            int nTotal;

            strncpy(szQueryTime32, szQueryTime, 32);

            for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
            {
                tmTimeStamp = pFCGI_Rough->chargerDataRepo.bmsRecData[n].nDay;
                tmTime = gmtime(&tmTimeStamp);
                snprintf(szStringTime, 32, "%d-%.2d-%.2d",
                        tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday);
                if(strcmp(szStringTime, szQueryTime32) == 0)
                {
                    nFlag = 1;
                    break;
                }
            }

            if(nFlag == 0)
            {
                TRACEX("this date's data doesn't exist!");
                //Make_Web_OutPut(104, "this date's data doesn't exist!", NULL, NULL);
                return TRUE;
            }

            cond.nGunIdx = pFCGI_Rough->chargerDataRepo.bmsRecData[n].nDay;
            if(!QueryChgRunData_byCond(&cond))
            {
                //Make_Web_OutPut(104, "Did not find such bms_rundata record!", NULL, NULL);
                return TRUE;
            }

            nTotal = MIN(pList->nBmsRuntimeDataTotal, MAX_HIS_BMS_RUNTIME_DATALIST);

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "\u2103");

            //Max Temp
            cJSON* pJSTempDataMaxTemp = cJSON_CreateObject();
            cJSON* pJSTempDataMaxTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataMaxTemp, "name", LoadLangText_ById(675));

            for(m = 0; m < nTotal; m++)
            {
                if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                cJSON* pJSTempDataMaxTempValue = cJSON_CreateArray();

                tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01

                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                fValue = pList->stBmsRuntimeDataList[m].fMax_CellTemp;
                if(fValue <= 0){continue;}
                cJSON_AddItemToArray(pJSTempDataMaxTempValue, cJSON_CreateString(szStringTime));
                cJSON_AddItemToArray(pJSTempDataMaxTempValue, cJSON_CreateFloat(fValue, 1));
                cJSON_AddItemToArray(pJSTempDataMaxTempValueList, pJSTempDataMaxTempValue);
            }
            cJSON_AddItemToObject(pJSTempDataMaxTemp, "val_list", pJSTempDataMaxTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataMaxTemp);

            //Min Temp
            cJSON* pJSTempDataMinTemp = cJSON_CreateObject();
            cJSON* pJSTempDataMinTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataMinTemp, "name", LoadLangText_ById(676));

            for(m = 0; m < nTotal; m++)
            {
                if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                cJSON* pJSTempDataMinTempValue = cJSON_CreateArray();

                tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01

                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                fValue = pList->stBmsRuntimeDataList[m].fMin_CellTemp;
                if(fValue <= 0){continue;}
                cJSON_AddItemToArray(pJSTempDataMinTempValue, cJSON_CreateString(szStringTime));
                cJSON_AddItemToArray(pJSTempDataMinTempValue, cJSON_CreateFloat(fValue, 1));
                cJSON_AddItemToArray(pJSTempDataMinTempValueList, pJSTempDataMinTempValue);
            }
            cJSON_AddItemToObject(pJSTempDataMinTemp, "val_list", pJSTempDataMinTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataMinTemp);

            //Temp Diff
            cJSON* pJSTempDataTempDiff = cJSON_CreateObject();
            cJSON* pJSTempDataTempDiffValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataTempDiff, "name", LoadLangText_ById(6018));

            for(m = 0; m < nTotal; m++)
            {
                if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                cJSON* pJSTempDataTempDiffValue = cJSON_CreateArray();

                tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01

                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                fValue = (pList->stBmsRuntimeDataList[m].fMax_CellTemp - pList->stBmsRuntimeDataList[m].fMin_CellTemp);
                if(fValue <= 0){continue;}
                cJSON_AddItemToArray(pJSTempDataTempDiffValue, cJSON_CreateString(szStringTime));
                cJSON_AddItemToArray(pJSTempDataTempDiffValue, cJSON_CreateFloat(fValue, 1));
                cJSON_AddItemToArray(pJSTempDataTempDiffValueList, pJSTempDataTempDiffValue);
            }
            cJSON_AddItemToObject(pJSTempDataTempDiff, "val_list", pJSTempDataTempDiffValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataTempDiff);

            //InWater Temp
            cJSON* pJSTempDataInWaterTemp = cJSON_CreateObject();
            cJSON* pJSTempDataInWaterTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataInWaterTemp, "name", LoadLangText_ById(5544));

            for(m = 0; m < nTotal; m++)
            {
                if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                cJSON* pJSTempDataInWaterTempValue = cJSON_CreateArray();

                tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01

                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                fValue = pList->stBmsRuntimeDataList[m].fBattInWaterTemp;
                if(fValue <= 0){continue;}
                cJSON_AddItemToArray(pJSTempDataInWaterTempValue, cJSON_CreateString(szStringTime));
                cJSON_AddItemToArray(pJSTempDataInWaterTempValue, cJSON_CreateFloat(fValue, 1));
                cJSON_AddItemToArray(pJSTempDataInWaterTempValueList, pJSTempDataInWaterTempValue);
            }
            cJSON_AddItemToObject(pJSTempDataInWaterTemp, "val_list", pJSTempDataInWaterTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataInWaterTemp);

            //OutWater Temp
            cJSON* pJSTempDataOutWaterTemp = cJSON_CreateObject();
            cJSON* pJSTempDataOutWaterTempValueList = cJSON_CreateArray();

            cJSON_AddStringToObject(pJSTempDataOutWaterTemp, "name", LoadLangText_ById(5545));

            for(m = 0; m < nTotal; m++)
            {
                if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                cJSON* pJSTempDataOutWaterTempValue = cJSON_CreateArray();

                tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year == 70){continue;}  //1970-01-01

                snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                fValue = pList->stBmsRuntimeDataList[m].fBattOutWaterTemp;
                if(fValue <= 0){continue;}
                cJSON_AddItemToArray(pJSTempDataOutWaterTempValue, cJSON_CreateString(szStringTime));
                cJSON_AddItemToArray(pJSTempDataOutWaterTempValue, cJSON_CreateFloat(fValue, 1));
                cJSON_AddItemToArray(pJSTempDataOutWaterTempValueList, pJSTempDataOutWaterTempValue);
            }
            cJSON_AddItemToObject(pJSTempDataOutWaterTemp, "val_list", pJSTempDataOutWaterTempValueList);
            cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataOutWaterTemp);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "temp_data", pJSTempDataAxisY);
        }

        if(nRange == 2)
        {
            time_t tmTimeStamp;
            struct tm *tmTime;
            struct tm tm;
            char  szStringTime[32] = "\0";
            char  szStringName[32] = "\0";
            char  szQueryTime64[64] = "\0";
            char  szQuertTimeStart[20] = "\0";
            char  szQuertTimeEnd[20] = "\0";
            UINT   szTimeList[WEB_SUPPORT_MAX_BMSREC];
            UINT   nTempValue;
            time_t tmStart;
            time_t tmEnd;
            cJSON* pJSTempDataAxisY = cJSON_CreateArray();
            float fValue;
            int m;
            QUERYCOND_HISMETERVAL cond;
            BMS_RUNTIMEDATA_LIST* pList = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.bmsRunData;
            int nTotal;
            int nTotalLow = 0;
            int nTotalHigh = 0;

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "\u2103");
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis_right", "A");

            strncpy(szQueryTime64, szQueryTime, 64);
            snprintf(szQuertTimeStart, 20, "%s", szQueryTime64);
            char *temp = &szQueryTime64[20];
            snprintf(szQuertTimeEnd, 20, "%s", temp);

            char year_str[5], mon_str[3], day_str[3], hour_str[3], min_str[3], sec_str[3];
            sscanf(szQuertTimeStart, "%4s-%2s-%2s-%2s:%2s:%2s", year_str, mon_str, day_str, hour_str, min_str, sec_str);
            tm.tm_year = atoi(year_str) - 1900;
            tm.tm_mon = atoi(mon_str) - 1;
            tm.tm_mday = atoi(day_str);
            tm.tm_hour = atoi(hour_str);
            tm.tm_min = atoi(min_str);
            tm.tm_sec = atoi(sec_str);
            tm.tm_isdst = 0;
            tmStart = mktime(&tm);

            sscanf(szQuertTimeEnd, "%4s-%2s-%2s-%2s:%2s:%2s", year_str, mon_str, day_str, hour_str, min_str, sec_str);
            tm.tm_year = atoi(year_str) - 1900;
            tm.tm_mon = atoi(mon_str) - 1;
            tm.tm_mday = atoi(day_str);
            tm.tm_hour = atoi(hour_str);
            tm.tm_min = atoi(min_str);
            tm.tm_sec = atoi(sec_str);
            tm.tm_isdst = 0;
            tmEnd = mktime(&tm);

            for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
            {
                szTimeList[n] = pFCGI_Rough->chargerDataRepo.bmsRecData[n].nDay;
            }

            for(n = WEB_SUPPORT_MAX_BMSREC - 2; n >= 0; n--)
            {
                for(m = 0; m <= n; m++)
                {
                    if(szTimeList[m] > szTimeList[m + 1])
                    {
                        nTempValue = szTimeList[m];
                        szTimeList[m] = szTimeList[m + 1];
                        szTimeList[m + 1] = nTempValue;
                    }
                }
            }

            //for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
            {
                cond.nGunIdx = szTimeList[(nDaySector - 1) / 4];
                if(!QueryChgRunData_byCond(&cond))
                {
                    //Make_Web_OutPut(104, "Did not find such bms_rundata record!", NULL, NULL);
                    //continue;
                }

                //Only give half of the data each time requested
                nTotal = MIN(pList->nBmsRuntimeDataTotal, MAX_HIS_BMS_RUNTIME_DATALIST);
                if((nDaySector - 1) % 4 == 0)
                {
                    nTotalLow = 0;
                    nTotalHigh = nTotal / 4;
                }
                else if((nDaySector - 1) % 4 == 1)
                {
                    nTotalLow = nTotal / 4 + 1;
                    nTotalHigh = 2 * nTotal / 4;
                }
                else if((nDaySector - 1) % 4 == 2)
                {
                    nTotalLow = 2 * nTotal / 4 + 1;
                    nTotalHigh = 3 * nTotal / 4;
                }
                else if((nDaySector - 1) % 4 == 3)
                {
                    nTotalLow = 3 * nTotal / 4 + 1;
                    nTotalHigh = nTotal;
                }

                //Max Temp
                cJSON* pJSTempDataMaxTemp = cJSON_CreateObject();
                cJSON* pJSTempDataMaxTempValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataMaxTemp, "name", LoadLangText_ById(675));
                    
                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataMaxTempValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fMax_CellTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fMax_CellTemp;
                    cJSON_AddItemToArray(pJSTempDataMaxTempValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataMaxTempValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataMaxTempValueList, pJSTempDataMaxTempValue);
                }
                cJSON_AddItemToObject(pJSTempDataMaxTemp, "val_list", pJSTempDataMaxTempValueList);
                cJSON_AddItemToObject(pJSTempDataMaxTemp, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataMaxTemp);

                //Min Temp
                cJSON* pJSTempDataMinTemp = cJSON_CreateObject();
                cJSON* pJSTempDataMinTempValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataMinTemp, "name", LoadLangText_ById(676));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataMinTempValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fMin_CellTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fMin_CellTemp;
                    cJSON_AddItemToArray(pJSTempDataMinTempValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataMinTempValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataMinTempValueList, pJSTempDataMinTempValue);
                }
                cJSON_AddItemToObject(pJSTempDataMinTemp, "val_list", pJSTempDataMinTempValueList);
                cJSON_AddItemToObject(pJSTempDataMinTemp, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataMinTemp);

                //Temp Diff
                cJSON* pJSTempDataTempDiff = cJSON_CreateObject();
                cJSON* pJSTempDataTempDiffValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataTempDiff, "name", LoadLangText_ById(6018));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataTempDiffValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fMin_CellTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fMax_CellTemp - pList->stBmsRuntimeDataList[m].fMin_CellTemp;
                    cJSON_AddItemToArray(pJSTempDataTempDiffValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataTempDiffValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataTempDiffValueList, pJSTempDataTempDiffValue);
                }
                cJSON_AddItemToObject(pJSTempDataTempDiff, "val_list", pJSTempDataTempDiffValueList);
                cJSON_AddItemToObject(pJSTempDataTempDiff, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataTempDiff);

                //InWater Temp
                cJSON* pJSTempDataInWaterTemp = cJSON_CreateObject();
                cJSON* pJSTempDataInWaterTempValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataInWaterTemp, "name", LoadLangText_ById(5544));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataInWaterTempValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fBattInWaterTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fBattInWaterTemp;
                    cJSON_AddItemToArray(pJSTempDataInWaterTempValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataInWaterTempValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataInWaterTempValueList, pJSTempDataInWaterTempValue);
                }
                cJSON_AddItemToObject(pJSTempDataInWaterTemp, "val_list", pJSTempDataInWaterTempValueList);
                cJSON_AddItemToObject(pJSTempDataInWaterTemp, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataInWaterTemp);

                //OutWater Temp
                cJSON* pJSTempDataOutWaterTemp = cJSON_CreateObject();
                cJSON* pJSTempDataOutWaterTempValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataOutWaterTemp, "name", LoadLangText_ById(5545));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataOutWaterTempValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fBattOutWaterTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fBattOutWaterTemp;
                    cJSON_AddItemToArray(pJSTempDataOutWaterTempValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataOutWaterTempValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataOutWaterTempValueList, pJSTempDataOutWaterTempValue);
                }
                cJSON_AddItemToObject(pJSTempDataOutWaterTemp, "val_list", pJSTempDataOutWaterTempValueList);
                cJSON_AddItemToObject(pJSTempDataOutWaterTemp, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataOutWaterTemp);

                //Water Temp Diff
                cJSON* pJSTempDataWaterTempDiff = cJSON_CreateObject();
                cJSON* pJSTempDataWaterTempDiffValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataWaterTempDiff, "name", LoadLangText_ById(6022));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataWaterTempDiffValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fBattOutWaterTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fBattInWaterTemp - pList->stBmsRuntimeDataList[m].fBattOutWaterTemp;
                    cJSON_AddItemToArray(pJSTempDataWaterTempDiffValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataWaterTempDiffValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataWaterTempDiffValueList, pJSTempDataWaterTempDiffValue);
                }
                cJSON_AddItemToObject(pJSTempDataWaterTempDiff, "val_list", pJSTempDataWaterTempDiffValueList);
                cJSON_AddItemToObject(pJSTempDataWaterTempDiff, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataWaterTempDiff);

                //Ambient Temp
                cJSON* pJSTempDataAmbientTemp = cJSON_CreateObject();
                cJSON* pJSTempDataAmbientTempValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataAmbientTemp, "name", LoadLangText_ById(6023));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataAmbientTempValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    if(pList->stBmsRuntimeDataList[m].fSysAmbientTemp <= 0){continue;}
                    fValue = pList->stBmsRuntimeDataList[m].fSysAmbientTemp;
                    cJSON_AddItemToArray(pJSTempDataAmbientTempValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataAmbientTempValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataAmbientTempValueList, pJSTempDataAmbientTempValue);
                }
                cJSON_AddItemToObject(pJSTempDataAmbientTemp, "val_list", pJSTempDataAmbientTempValueList);
                cJSON_AddItemToObject(pJSTempDataAmbientTemp, "yAxisIndex", cJSON_CreateNumber(0));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataAmbientTemp);

                //Current
                cJSON* pJSTempDataCurrent = cJSON_CreateObject();
                cJSON* pJSTempDataCurrentValueList = cJSON_CreateArray();

                cJSON_AddStringToObject(pJSTempDataCurrent, "name", LoadLangText_ById(364));

                for(m = nTotalLow; m < nTotalHigh; m++)
                {
                    if(pList->stBmsRuntimeDataList[m].nBmsIdx != nDevtype){continue;}
                    cJSON* pJSTempDataCurrentValue = cJSON_CreateArray();

                    tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
                    if(tmTimeStamp < tmStart || tmTimeStamp > tmEnd){continue;}
                    tmTime = gmtime(&tmTimeStamp);
                    if(tmTime->tm_year == 70){continue;}  //1970-01-01

                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    fValue = pList->stBmsRuntimeDataList[m].fPackTotalCurr;
                    cJSON_AddItemToArray(pJSTempDataCurrentValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSTempDataCurrentValue, cJSON_CreateFloat(fValue, 1));
                    cJSON_AddItemToArray(pJSTempDataCurrentValueList, pJSTempDataCurrentValue);
                }
                cJSON_AddItemToObject(pJSTempDataCurrent, "val_list", pJSTempDataCurrentValueList);
                cJSON_AddItemToObject(pJSTempDataCurrent, "yAxisIndex", cJSON_CreateNumber(1));
                cJSON_AddItemToArray(pJSTempDataAxisY, pJSTempDataCurrent);
            }

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "temp_data", pJSTempDataAxisY);
        }
    }

    if(nChartIdx == STAT_GRAPH_ID_EFF_CURVE)
    {
        if(nRange == 0)
        {
            time_t tmTimeStamp;
            struct tm *tmTime;
            char  szStringTime[32] = "\0";
            char  szStringName[32] = "\0";
            cJSON* pJSEFFDataAxisY = cJSON_CreateArray();
            float fValue;
            float fValueIn;
            float fValueOut;
            int m;
            char szSql[512];
            sqlite3_stmt * stat;
            sqlite3* data_db;
            float fEfficiency = -1.0;
            EFFICIENCY_DATA_QUERY Data_LastRow;
            EFFICIENCY_DATA_QUERY Data_ThisRow;
            int nCount = 0;
            char szStringVal[32] = "\0";
            VAR_VALUE varV;
            void* pData;

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "%");

            //Efficiency Data
            cJSON* pJSEffData = cJSON_CreateObject();
            cJSON* pJSEffDataValueList = cJSON_CreateArray();
            cJSON_AddStringToObject(pJSEffData, "name", LoadLangText_ById(6021));

            //get today's data
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1897, 4, (void*)&tmTimeStamp);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1898, 4, (void*)&fValueIn);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1899, 4, (void*)&fValueOut);

            if(tmTimeStamp != 0)
            {
                cJSON* pJSEffDataValue = cJSON_CreateArray();

                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year != 70 && fValueIn != 0.0 && fValueOut != 0.0)
                {
                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    fValue = fValueOut / fValueIn * 100;
                    
                    cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
                    cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
                }
            }

            //get data from nccu.data
            data_db = Open_Lock_SqliteDB();
            sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE (Type = 0 OR Type = 1)");
            printf("Now begin to query efficiency data!\n");
            sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
            while(sqlite3_step(stat) == SQLITE_ROW)
            {
                if(nCount == 0) //first time
                {
                    Data_LastRow.nType = sqlite3_column_int(stat, 1);
                    Data_LastRow.tmTime = sqlite3_column_int64(stat, 2);
                    void* pData = sqlite3_column_blob(stat, 3);
                    varV.blobValue = pData;
                    sprintf(szStringVal, "%s", (char*)varV.blobValue);
                    Data_LastRow.fValue = atof(szStringVal);
                    nCount = 1;
                    continue;
                }

                Data_ThisRow.nType = sqlite3_column_int(stat, 1);
                Data_ThisRow.tmTime = sqlite3_column_int64(stat, 2);
                pData = sqlite3_column_blob(stat, 3);
                varV.blobValue = pData;
                sprintf(szStringVal, "%s", (char*)varV.blobValue);
                Data_ThisRow.fValue = atof(szStringVal);

                if(Data_LastRow.tmTime <= 0 || Data_ThisRow.tmTime <= 0 || Data_LastRow.fValue > 12000 || Data_ThisRow.fValue > 12000)
                {
                    Data_LastRow.nType = Data_ThisRow.nType;
                    Data_LastRow.tmTime = Data_ThisRow.tmTime;
                    Data_LastRow.fValue = Data_ThisRow.fValue;
                    continue;
                }

                if(Data_LastRow.tmTime == Data_ThisRow.tmTime)
                {
                    if(Data_LastRow.nType == 0 && Data_ThisRow.nType == 1)
                    {
                        cJSON* pJSEffDataValue = cJSON_CreateArray();

                        tmTime = gmtime(&Data_ThisRow.tmTime);
                        if(tmTime->tm_year == 70){continue;}  //1970-01-01
                        snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                                tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                                tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                        if(Data_LastRow.fValue == 0.0 || Data_ThisRow.fValue == 0.0){continue;}
                        fValue = Data_ThisRow.fValue / Data_LastRow.fValue * 100;
                        
                        cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
                        cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
                        cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
                    }
                }

                Data_LastRow.nType = Data_ThisRow.nType;
                Data_LastRow.tmTime = Data_ThisRow.tmTime;
                Data_LastRow.fValue = Data_ThisRow.fValue;
            }

            Close_UnLock_SqliteDB();
            
            cJSON_AddItemToObject(pJSEffData, "val_list", pJSEffDataValueList);
            cJSON_AddItemToArray(pJSEFFDataAxisY, pJSEffData);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "efficiency_data", pJSEFFDataAxisY);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "efficiency_name", LoadLangText_ById(6021));

            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 407, 411, 4, (void*)&fValueIn);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 407, 422, 4, (void*)&fValueOut);
            if(fValueIn != 0.0)
            {
                cJSON_AddFloatToObject(pFCGI_Rough->pJSON_Resp, "efficiency_value", fValueOut/fValueIn * 100, 2);
            }
        }

        if(nRange == 1)
        {
            time_t tmTimeStamp;
            struct tm *tmTime;
            char  szStringTime[32] = "\0";
            char  szStringName[32] = "\0";
            cJSON* pJSEFFDataAxisY = cJSON_CreateArray();
            float fValue;
            float fValueIn;
            float fValueOut;
            int m;
            char szSql[512];
            sqlite3_stmt * stat;
            sqlite3* data_db;
            float fEfficiency = -1.0;
            EFFICIENCY_DATA_QUERY Data_LastRow;
            EFFICIENCY_DATA_QUERY Data_ThisRow;
            int nCount = 0;
            char szStringVal[32] = "\0";
            VAR_VALUE varV;
            void* pData;

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "%");

            //Efficiency Data
            cJSON* pJSEffData = cJSON_CreateObject();
            cJSON* pJSEffDataValueList = cJSON_CreateArray();
            cJSON_AddStringToObject(pJSEffData, "name", LoadLangText_ById(6021));

            //get this month's data
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1897, 4, (void*)&tmTimeStamp);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1900, 4, (void*)&fValueIn);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1901, 4, (void*)&fValueOut);

            if(tmTimeStamp != 0)
            {
                cJSON* pJSEffDataValue = cJSON_CreateArray();

                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year != 70 && fValueIn != 0.0 && fValueOut != 0.0)
                {
                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    fValue = fValueOut / fValueIn * 100;
                    
                    cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
                    cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
                }
            }

            //get data from nccu.data
            data_db = Open_Lock_SqliteDB();
            sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE (Type = 2 OR Type = 3)");
            printf("Now begin to query efficiency data!\n");
            sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
            while(sqlite3_step(stat) == SQLITE_ROW)
            {
                if(nCount == 0) //first time
                {
                    Data_LastRow.nType = sqlite3_column_int(stat, 1);
                    Data_LastRow.tmTime = sqlite3_column_int64(stat, 2);
                    void* pData = sqlite3_column_blob(stat, 3);
                    varV.blobValue = pData;
                    sprintf(szStringVal, "%s", (char*)varV.blobValue);
                    Data_LastRow.fValue = atof(szStringVal);
                    nCount = 1;
                    continue;
                }

                Data_ThisRow.nType = sqlite3_column_int(stat, 1);
                Data_ThisRow.tmTime = sqlite3_column_int64(stat, 2);
                pData = sqlite3_column_blob(stat, 3);
                varV.blobValue = pData;
                sprintf(szStringVal, "%s", (char*)varV.blobValue);
                Data_ThisRow.fValue = atof(szStringVal);

                if(Data_LastRow.tmTime <= 0 || Data_ThisRow.tmTime <= 0 || Data_LastRow.fValue > 12000 || Data_ThisRow.fValue > 12000)
                {
                    Data_LastRow.nType = Data_ThisRow.nType;
                    Data_LastRow.tmTime = Data_ThisRow.tmTime;
                    Data_LastRow.fValue = Data_ThisRow.fValue;
                    continue;
                }

                if(Data_LastRow.tmTime == Data_ThisRow.tmTime)
                {
                    if(Data_LastRow.nType == 2 && Data_ThisRow.nType == 3)
                    {
                        cJSON* pJSEffDataValue = cJSON_CreateArray();

                        tmTime = gmtime(&Data_ThisRow.tmTime);
                        if(tmTime->tm_year == 70){continue;}  //1970-01-01
                        snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                                tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                                tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                        if(Data_LastRow.fValue == 0.0){continue;}
                        //if(Data_LastRow.fValue == 0.0 || Data_ThisRow.fValue == 0.0){continue;}
                        fValue = Data_ThisRow.fValue / Data_LastRow.fValue * 100;
                        
                        cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
                        cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
                        cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
                    }
                }

                Data_LastRow.nType = Data_ThisRow.nType;
                Data_LastRow.tmTime = Data_ThisRow.tmTime;
                Data_LastRow.fValue = Data_ThisRow.fValue;
            }

            Close_UnLock_SqliteDB();
            
            cJSON_AddItemToObject(pJSEffData, "val_list", pJSEffDataValueList);
            cJSON_AddItemToArray(pJSEFFDataAxisY, pJSEffData);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "efficiency_data", pJSEFFDataAxisY);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "efficiency_name", LoadLangText_ById(6021));

            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 407, 411, 4, (void*)&fValueIn);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 407, 422, 4, (void*)&fValueOut);
            if(fValueIn != 0.0)
            {
                cJSON_AddFloatToObject(pFCGI_Rough->pJSON_Resp, "efficiency_value", fValueOut/fValueIn * 100, 2);
            }
        }

        if(nRange == 2)
        {
            time_t tmTimeStamp;
            struct tm *tmTime;
            char  szStringTime[32] = "\0";
            char  szStringName[32] = "\0";
            cJSON* pJSEFFDataAxisY = cJSON_CreateArray();
            float fValue;
            float fValueIn;
            float fValueOut;
            int m;
            char szSql[512];
            sqlite3_stmt * stat;
            sqlite3* data_db;
            float fEfficiency = -1.0;
            EFFICIENCY_DATA_QUERY Data_LastRow;
            EFFICIENCY_DATA_QUERY Data_ThisRow;
            int nCount = 0;
            char szStringVal[32] = "\0";
            VAR_VALUE varV;
            void* pData;

            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "%");

            //Efficiency Data
            cJSON* pJSEffData = cJSON_CreateObject();
            cJSON* pJSEffDataValueList = cJSON_CreateArray();
            cJSON_AddStringToObject(pJSEffData, "name", LoadLangText_ById(6021));
            
            //get this year's data
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1897, 4, (void*)&tmTimeStamp);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1902, 4, (void*)&fValueIn);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 1903, 4, (void*)&fValueOut);

            if(tmTimeStamp != 0)
            {
                cJSON* pJSEffDataValue = cJSON_CreateArray();

                tmTime = gmtime(&tmTimeStamp);
                if(tmTime->tm_year != 70 && fValueIn != 0.0 && fValueOut != 0.0)
                {
                    snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                            tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                            tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                    fValue = fValueOut / fValueIn * 100;
                    
                    cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
                    cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
                    cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
                }
            }

            //get data from nccu.data
            data_db = Open_Lock_SqliteDB();
            sprintf(szSql, "SELECT * FROM Data_AccumulatedData WHERE (Type = 4 OR Type = 5)");
            printf("Now begin to query efficiency data!\n");
            sqlite3_prepare_v2(data_db, szSql, -1, &stat, NULL);
            while(sqlite3_step(stat) == SQLITE_ROW)
            {
                if(nCount == 0) //first time
                {
                    Data_LastRow.nType = sqlite3_column_int(stat, 1);
                    Data_LastRow.tmTime = sqlite3_column_int64(stat, 2);
                    void* pData = sqlite3_column_blob(stat, 3);
                    varV.blobValue = pData;
                    sprintf(szStringVal, "%s", (char*)varV.blobValue);
                    Data_LastRow.fValue = atof(szStringVal);
                    nCount = 1;
                    continue;
                }

                Data_ThisRow.nType = sqlite3_column_int(stat, 1);
                Data_ThisRow.tmTime = sqlite3_column_int64(stat, 2);
                pData = sqlite3_column_blob(stat, 3);
                varV.blobValue = pData;
                sprintf(szStringVal, "%s", (char*)varV.blobValue);
                Data_ThisRow.fValue = atof(szStringVal);

                if(Data_LastRow.tmTime <= 0 || Data_ThisRow.tmTime <= 0 || Data_LastRow.fValue > 12000 || Data_ThisRow.fValue > 12000)
                {
                    Data_LastRow.nType = Data_ThisRow.nType;
                    Data_LastRow.tmTime = Data_ThisRow.tmTime;
                    Data_LastRow.fValue = Data_ThisRow.fValue;
                    continue;
                }

                if(Data_LastRow.tmTime == Data_ThisRow.tmTime)
                {
                    if(Data_LastRow.nType == 4 && Data_ThisRow.nType == 5)
                    {
                        cJSON* pJSEffDataValue = cJSON_CreateArray();

                        tmTime = gmtime(&Data_ThisRow.tmTime);
                        if(tmTime->tm_year == 70){continue;}  //1970-01-01
                        snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
                                tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
                                tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
                        if(Data_LastRow.fValue == 0.0){continue;}
                        //if(Data_LastRow.fValue == 0.0 || Data_ThisRow.fValue == 0.0){continue;}
                        fValue = Data_ThisRow.fValue / Data_LastRow.fValue * 100;
                        
                        cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
                        cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
                        cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
                    }
                }

                Data_LastRow.nType = Data_ThisRow.nType;
                Data_LastRow.tmTime = Data_ThisRow.tmTime;
                Data_LastRow.fValue = Data_ThisRow.fValue;
            }
            
            Close_UnLock_SqliteDB();
            
            cJSON_AddItemToObject(pJSEffData, "val_list", pJSEffDataValueList);
            cJSON_AddItemToArray(pJSEFFDataAxisY, pJSEffData);

            cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "efficiency_data", pJSEFFDataAxisY);
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "efficiency_name", LoadLangText_ById(6021));

            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 407, 411, 4, (void*)&fValueIn);
            Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 407, 422, 4, (void*)&fValueOut);
            if(fValueIn != 0.0)
            {
                cJSON_AddFloatToObject(pFCGI_Rough->pJSON_Resp, "efficiency_value", fValueOut/fValueIn * 100, 2);
            }
        }

        // if(nRange == 3)
        // {
        //     time_t tmTimeStamp;
        //     struct tm *tmTime;
        //     struct tm tm;
        //     char  szStringTime[32] = "\0";
        //     char  szStringName[32] = "\0";
        //     cJSON* pJSEFFDataAxisY = cJSON_CreateArray();
        //     float fValue;
        //     int m;
        //     QUERYCOND_HISMETERVAL cond;
        //     BMS_RUNTIMEDATA_LIST* pList = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.bmsRunData;
        //     int nTotal;

        //     cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y_axis", "%");

        //     for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
        //     {
        //         cond.nGunIdx = pFCGI_Rough->chargerDataRepo.bmsRecData[n].nDay;
        //         if(!QueryChgRunData_byCond(&cond))
        //         {
        //             //Make_Web_OutPut(104, "Did not find such bms_rundata record!", NULL, NULL);
        //             continue;
        //         }

        //         nTotal = MIN(pList->nBmsRuntimeDataTotal, MAX_HIS_BMS_RUNTIME_DATALIST);

        //         //Cumulative Efficiency Data
        //         cJSON* pJSEffData = cJSON_CreateObject();
        //         cJSON* pJSEffDataValueList = cJSON_CreateArray();

        //         cJSON_AddStringToObject(pJSEffData, "name", LoadLangText_ById(6021));
                    
        //         for(m = 0; m < nTotal; m++)
        //         {
        //             cJSON* pJSEffDataValue = cJSON_CreateArray();

        //             tmTimeStamp = pList->stBmsRuntimeDataList[m].tmSaved;
        //             tmTime = gmtime(&tmTimeStamp);
        //             if(tmTime->tm_year == 70){continue;}  //1970-01-01

        //             snprintf(szStringTime, 32, "%d-%.2d-%.2d %.2d:%.2d:%.2d",
        //                     tmTime->tm_year + 1900, tmTime->tm_mon + 1, tmTime->tm_mday,
        //                     tmTime->tm_hour, tmTime->tm_min, tmTime->tm_sec);
        //             if(pList->stBmsRuntimeDataList[m].fCabMeter_Power[0] <= 0){continue;}
        //             fValue = pList->stBmsRuntimeDataList[m].fCabMeter_Power[2] / pList->stBmsRuntimeDataList[m].fCabMeter_Power[0] * 100;
        //             cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateString(szStringTime));
        //             cJSON_AddItemToArray(pJSEffDataValue, cJSON_CreateFloat(fValue, 2));
        //             cJSON_AddItemToArray(pJSEffDataValueList, pJSEffDataValue);
        //         }
        //         cJSON_AddItemToObject(pJSEffData, "val_list", pJSEffDataValueList);
        //         cJSON_AddItemToArray(pJSEFFDataAxisY, pJSEffData);
        //     }

        //     cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "efficiency_data", pJSEFFDataAxisY);
        // }
    }

    if(nChartIdx == STAT_GRAPH_ID_SIN_CELL)
    {
        WEB_PARAM_VALUE* pWParVal;
        if ((pWParVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 500, 1944)) == NULL) 
        {
            TRACEX("-ERRRRRR--1\n");
            return;
        }
        UINT nPacknum = pWParVal->currValue.ulValue;

        //get single cell number of per pack
        if ((pWParVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 500, 1945)) == NULL)
        {
            TRACEX("-ERRRRRR--2\n");
            return TRUE;
        }
        UINT nSingleCell_num = pWParVal->currValue.ulValue;

        if((nCabIdx >= MAX_SINGLE_STBATT_NUM) && (nCabIdx < 0))
        {
            return;
        }

        // edit by Kuangyue 20241114

        if (nCabIdx >= 0 && nCabIdx <= 3) 
        {
            Process_BMS_Data(nCabIdx, nPackIdx, nPacknum, nSingleCell_num, pFCGI_Rough);
        }
    }
    return TRUE;
}


static int Compare_Time_4ChgRec(time_t tmRec, int nStat_TypeSel, int nDate_Type_Sel, int nCurrSel)
{
    tmRec = Conv_UTC_2LocalTime(tmRec);
    time_t tmNow = Conv_UTC_2LocalTime(_NOW_);;
    //if is future rec, we return +
    // if(tmRec > tmNow)
    // {
    //     return 1;
    // }

    //conevetn time=>tm
    struct tm gmTime_Rec;
	gmtime_r(&tmRec, &gmTime_Rec);
    //compare baseline
    int nComp;
     
    if((nStat_TypeSel % 10) == STAT_TYPE_BY_WEEK)
    {
        nComp = gmTime_Rec.tm_wday;//by week, [0-6], 0 means Sunday
    }
    else if((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR)
    {
        nComp = gmTime_Rec.tm_hour;//by hour, [0-23]
    }
    else //if((nStat_TypeSel % 10) == STAT_TYPE_BY_MONTH)
    {
        nComp = gmTime_Rec.tm_mday - 1; //by month(default), tm_mday->[1-31]
    }   
    //now we compare the time respectively
    if(nDate_Type_Sel == DATE_TYPE_PER_ALL) //all
    {
        return nComp - nCurrSel;
    }
    else if(nDate_Type_Sel == DATE_TYPE_PER_THIS) //this
    {        
        struct tm gmTime_Now;
        gmtime_r(&tmNow, &gmTime_Now);
        gmTime_Now.tm_hour = 0;
        gmTime_Now.tm_min = 0;
        gmTime_Now.tm_sec = 0;
        time_t tmBegin_Today = mktime(&gmTime_Now);	
        time_t tmBegin_This = tmBegin_Today;
        if((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR) //this day
        {
            tmBegin_This = tmBegin_Today;
        }
        else if((nStat_TypeSel % 10) == STAT_TYPE_BY_WEEK) //this week
        {
            tmBegin_This -= (SEC_PER_DAY * gmTime_Now.tm_wday);
            // char szTime[32], szTime1[32];
            // TRACEX("---now[%s], tmBeginThis[%s], tm_wday[%d]---\n",
            //     TimeToString(tmNow, TIME_CHN_FMT, szTime, 32),
            //     TimeToString(tmBegin_This, TIME_CHN_FMT, szTime1, 32));
        }
        else if((nStat_TypeSel % 10) == STAT_TYPE_BY_MONTH) //this month
        {
            gmTime_Now.tm_mday = 1;
            tmBegin_This = mktime(&gmTime_Now);
            
            // char szTime[32], szTime1[32];
            // TRACEX("---now[%s], tmBeginThis[%s]---\n",
            //     TimeToString(tmNow, TIME_CHN_FMT, szTime, 32),
            //     TimeToString(tmBegin_This, TIME_CHN_FMT, szTime1, 32));
        }
        //now compare
        if(tmRec < tmBegin_This)
        {
            return -1;
        }        
        else
        {
            return nComp - nCurrSel;
        }        
    }
    else if(nDate_Type_Sel == DATE_TYPE_PER_LAST) //last 
    {  
        struct tm gmTime_Now;
        gmtime_r(&tmNow, &gmTime_Now);
        gmTime_Now.tm_hour = 0;
        gmTime_Now.tm_min = 0;
        gmTime_Now.tm_sec = 0;
        time_t tmBegin_Today = mktime(&gmTime_Now);
        time_t tmBegin_Last = 0;
        time_t tmEnd_Last = 0;
        if((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR) //last day
        {
            tmBegin_Last = tmBegin_Today - SEC_PER_DAY;
            tmEnd_Last = tmBegin_Today;
        }
        else if((nStat_TypeSel % 10) == STAT_TYPE_BY_WEEK) //last week
        {
            tmBegin_Last = tmBegin_Today - (SEC_PER_DAY * (gmTime_Now.tm_wday + 7));
            tmEnd_Last = tmBegin_Today - (SEC_PER_DAY * gmTime_Now.tm_wday);
        }
        else //if((nStat_TypeSel % 10) == STAT_TYPE_BY_MONTH) //last month
        {
            tmEnd_Last = tmBegin_Today - (SEC_PER_DAY * (gmTime_Now.tm_mday - 1));
            gmTime_Now.tm_mday = 1;
            if(gmTime_Now.tm_mon > 0)
            {
                gmTime_Now.tm_mon -= 1;
                tmBegin_Last = mktime(&gmTime_Now);
            }
            else 
            {
                gmTime_Now.tm_mon = 11,
                gmTime_Now.tm_year -= 1;
                tmBegin_Last = mktime(&gmTime_Now);
            }
        }
        //now compare
        if(tmRec < tmBegin_Last)
        {
            return -1;
        }
        else if(tmRec >= tmEnd_Last)
        {
            return 1;
        }
        else
        {
            return nComp - nCurrSel;
        }   
    }

    return -99999;
}

static int Get_CurveData_Points(int nStat_TypeSel, int nDate_Type_Sel)
{
    int nPointsTotal = 31; //default is month
    time_t tmNow = Conv_UTC_2LocalTime(_NOW_);        
    struct tm gmTime_Now;
    gmtime_r(&tmNow, &gmTime_Now);
    if((nStat_TypeSel % 10) == STAT_TYPE_BY_WEEK)
    {
        nPointsTotal = 7;
        if(nDate_Type_Sel == DATE_TYPE_PER_THIS) //this
        {
            nPointsTotal = gmTime_Now.tm_wday + 1;
        }
    }
    else if((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR)
    {
        nPointsTotal = 24;
        if(nDate_Type_Sel == DATE_TYPE_PER_THIS) //this
        {
            nPointsTotal = gmTime_Now.tm_hour + 1;
        }
    }
    else //default is month
    {
        nPointsTotal = 31; 
        if(nDate_Type_Sel == DATE_TYPE_PER_THIS) //this
        {
            nPointsTotal = gmTime_Now.tm_mday;
        }
    }

    return nPointsTotal;
}

static BOOL Make_CurveData_ByStatTimes(FCGI_ROUGH_DATA* pFCGI_Rough, int nStat_TypeSel, int nDate_Type_Sel)
{
    typedef struct tagStatTimes
    {
        //BOOL bHasCurveData[MAX_STAT_CURVE_POINTS];
        int nTimesCurveData[MAX_STAT_CURVE_POINTS];
    }STAT_TIMES;
    
    STAT_TIMES stat_AllTimes;
    ZERO_POBJS(&stat_AllTimes, 1);

    int nPointsTotal = 0;
    cJSON* pJCurvAll = cJSON_CreateObject();
    cJSON* pJCurvArray = cJSON_CreateArray();

    cJSON_AddStringToObject(pJCurvAll, "name", "All BMS Total Charge Times");
    cJSON_AddItemToObject(pJCurvAll, "val_list", cJSON_CreateIntArray(stat_AllTimes.nTimesCurveData, nPointsTotal));
    cJSON_AddItemToArray(pJCurvArray, pJCurvAll);    
    //do not forget to add the currvData to targetJSON
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "curve_data", pJCurvArray);
    return TRUE;
}
/*
static BOOL Make_CurveData_ByStatTimes(FCGI_ROUGH_DATA* pFCGI_Rough, int nStat_TypeSel, int nDate_Type_Sel)
{
    typedef struct tagStatTimes
    {
        //BOOL bHasCurveData[MAX_STAT_CURVE_POINTS];
        int nTimesCurveData[MAX_STAT_CURVE_POINTS];
    }STAT_TIMES;
    
    float fStat_ChgTotalkWh = 0.0;
    int nStat_ChgTotalTimes = 0;
    int nTotal = pFCGI_Rough->chargerDataRepo.nHisChgRecTotal;
    STAT_TIMES stat_GunTimes[MAX_SUPPORT_GUN_NUM];
    ZERO_POBJS(stat_GunTimes, MAX_SUPPORT_GUN_NUM);
    STAT_TIMES stat_AllTimes;
    ZERO_POBJS(&stat_AllTimes, 1);
    HIS_CHARGE_RECORD* pHisRecItem = pFCGI_Rough->chargerDataRepo.hisChgRecords;
    int n, k;
    int nGunIdx = 0;
    int nPointsTotal = Get_CurveData_Points(nStat_TypeSel, nDate_Type_Sel);    

    for(n = 0; n < nTotal; n++, pHisRecItem++)
    {
        for(k = 0; k < nPointsTotal; k++)
        {
            if(Compare_Time_4ChgRec(pHisRecItem->tmChargeStart, nStat_TypeSel, nDate_Type_Sel, k) == 0)
            {
                nGunIdx = pHisRecItem->emGunNum - 1;
                if((nGunIdx < 0) || (nGunIdx >= MAX_SUPPORT_GUN_NUM))
                {
                    TRACEX("---the GunNUM[%d] is OUT of Range!!!---\n", pHisRecItem->emGunNum);
                    continue;
                }
                //stat_GunTimes[nGunIdx].bHasCurveData[k] = TRUE;
                stat_GunTimes[nGunIdx].nTimesCurveData[k] += 1;
                //stat_AllTimes.bHasCurveData[k] = TRUE;
                stat_AllTimes.nTimesCurveData[k] += 1;
                nStat_ChgTotalTimes += 1;
                fStat_ChgTotalkWh += pHisRecItem->extraRecData.runData.fCostkWh;
                break;
            }
        }
    }
    //now push the data to JSON
    cJSON_AddFloatToObject(pFCGI_Rough->pJSON_Resp, "total_chgkWh", fStat_ChgTotalkWh, 1);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "total_chgTimes", nStat_ChgTotalTimes); 
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y-axis", "times"); //will re-use other res-id instead
    cJSON* pJCurvArray = cJSON_CreateArray();
    WEB_PARAM_VALUE* pWParVal;
    int nGunRawStatus;
    char szName[32];
    
    //each gun Stat
    for(k = 0; k < MAX_SUPPORT_GUN_NUM; k++)
    {
        //get status param
        pWParVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 201 + k, 101);
        if(pWParVal == NULL)
        {
            TRACEX("-ERRRRRR--1\n");
            break;
        }
        nGunRawStatus = pWParVal->currValue.emValue;
        if(nGunRawStatus == GUN_STATE_NOT_INSTALL)
        {
            //TRACEX("-ERRRRRR--2\n");
            break;
        }
        cJSON* pJCurvGun = cJSON_CreateObject();
        //now make name
        snprintf(szName, 32, "Gun-%d Total Charge Times", k + 1);
        cJSON_AddStringToObject(pJCurvGun, "name", szName);
        cJSON_AddItemToObject(pJCurvGun, "val_list", cJSON_CreateIntArray(stat_GunTimes[k].nTimesCurveData, nPointsTotal));
        cJSON_AddItemToArray(pJCurvArray, pJCurvGun);
    }
    //all stat
    cJSON* pJCurvAll = cJSON_CreateObject();
    cJSON_AddStringToObject(pJCurvAll, "name", "All Guns Total Charge Times");
    cJSON_AddItemToObject(pJCurvAll, "val_list", cJSON_CreateIntArray(stat_AllTimes.nTimesCurveData, nPointsTotal));
    cJSON_AddItemToArray(pJCurvArray, pJCurvAll);    
    //do not forget to add the currvData to targetJSON
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "curve_data", pJCurvArray);
    return TRUE;
}
*/

/*
static float Get_StatkWh_by48Poinit_RawData(FCGI_ROUGH_DATA* pFCGI_Rough, HIS_CHARGE_RECORD* pHisRecItem, int nStat_TypeSel, int nCurrSel)
{
    float fkWh = 0.0;
    int n;    
    float fHour; 
    if((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR)
    {
        //get the 
        fHour = (float)nCurrSel - pFCGI_Rough->fTimeOffsetHrs;
        if(fHour < -0.001) //means the last day
        {
            fHour += 24.0;
        }
        else if(fHour > 23.9) //means the tomorrow day
        {
            fHour -= 23.9;
        }
        n = (int)(fHour * 2);
        if((n >= 0) && (n < 48))
        {
            fkWh += pHisRecItem->extraRecData.runData.f48Stat_kWh[n];
        }
        n += 1;
        if(n < 48)
        {
            fkWh += pHisRecItem->extraRecData.runData.f48Stat_kWh[n];
        }
    }
    else //for week/month, we need get rid of the over-day data
    {
        time_t tmStart = Conv_UTC_2LocalTime(pHisRecItem->tmChargeStart);
        time_t tmEnd = Conv_UTC_2LocalTime(pHisRecItem->extraRecData.runData.tmChargeEnd);
        struct tm gmTime_Start;
        gmtime_r(&tmStart, &gmTime_Start);
        struct tm gmTime_End;
        gmtime_r(&tmEnd, &gmTime_End);
        BOOL bIsThisOrOver = TRUE; //true means this, or else means over
        if(gmTime_Start.tm_wday != gmTime_End.tm_wday) //start != end, means over-day
        {
            //we think the charging can not cover 2 days
            if(((nStat_TypeSel % 10) == STAT_TYPE_BY_WEEK) &&
                ((nCurrSel - gmTime_Start.tm_wday) != 0))
            {
                bIsThisOrOver = FALSE;
            }
            else if(((nStat_TypeSel % 10) == STAT_TYPE_BY_MONTH) &&
                ((nCurrSel + 1 - gmTime_Start.tm_mday) != 0))
            {
                bIsThisOrOver = FALSE;
            }

            float fStartPoint, fEndPoint;
            if(bIsThisOrOver)
            {
                //need use UTC because 48-point are of UTC
                tmStart -= 3600 * pFCGI_Rough->fTimeOffsetHrs;
                gmtime_r(&tmStart, &gmTime_Start);
                fStartPoint = (float)gmTime_Start.tm_hour;// + ((float)gmTime_Start.tm_min / 60.0);
                gmTime_Start.tm_hour = 0;
                gmTime_Start.tm_min = 0;
                gmTime_Start.tm_sec = 0; 
                tmStart = mktime(&gmTime_Start) + SEC_PER_DAY - (int)3600 * pFCGI_Rough->fTimeOffsetHrs;
                gmtime_r(&tmStart, &gmTime_Start);
                fEndPoint = (float)gmTime_Start.tm_hour + ((float)gmTime_Start.tm_min / 60.0);
            }   
            else
            {
                gmTime_Start.tm_hour = 0;
                gmTime_Start.tm_min = 0;
                gmTime_Start.tm_sec = 0; 
                tmStart = mktime(&gmTime_Start) - (int)3600 * pFCGI_Rough->fTimeOffsetHrs;
                gmtime_r(&tmStart, &gmTime_Start);
                fStartPoint = (float)gmTime_Start.tm_hour + (float)gmTime_Start.tm_min / 60.0;
                tmEnd -= 3600 * pFCGI_Rough->fTimeOffsetHrs;
                gmtime_r(&tmEnd, &gmTime_End);
                fEndPoint = (float)gmTime_End.tm_hour + ((float)gmTime_End.tm_min / 60.0) + 0.5;
            }
                     
  
            //TRACEX("----11111[%.1f, %.1f]----\n", fStartPoint, fEndPoint);
            for(n = 0; n < 48; n++)
            {
                fHour = n * 0.5;
                if(((fHour - fStartPoint) > -0.001) && (((fEndPoint - fHour) > 0.001) || 
                    ((fEndPoint - fStartPoint)  < 0.001)))
                {
                    //TRACEX("---hout->%.1f, fStartPoint->%.1f, kwh->%.2f---\n", fHour, fStartPoint, 
                    //    pHisRecItem->extraRecData.runData.f48Stat_kWh[n]);
                    fkWh += pHisRecItem->extraRecData.runData.f48Stat_kWh[n];
                } 
            }
        }
        else
        {
            //TRACEX("----22222----\n");
            fkWh = pHisRecItem->extraRecData.runData.fCostkWh;
        }        
    }

    return fkWh;
}
*/

static BOOL Make_CurveData_ByStatkWh(FCGI_ROUGH_DATA* pFCGI_Rough, int nStat_TypeSel, int nDate_Type_Sel)
{
    typedef struct tagStatkWh
    {
        //BOOL bHasCurveData[MAX_STAT_CURVE_POINTS];
        float fkWhCurveData[MAX_STAT_CURVE_POINTS];
    }STAT_KWH;
    
    int nPointsTotal = 0;
    STAT_KWH stat_AllkWh;
    ZERO_POBJS(&stat_AllkWh, 1);

    //all stat
    cJSON* pJCurvAll = cJSON_CreateObject();
    cJSON* pJCurvArray = cJSON_CreateArray();

    cJSON_AddStringToObject(pJCurvAll, "name", "All BMS Total Charge kWh");
    cJSON_AddItemToObject(pJCurvAll, "val_list", cJSON_CreateFloatArrayEx(stat_AllkWh.fkWhCurveData, 1, nPointsTotal));
    cJSON_AddItemToArray(pJCurvArray, pJCurvAll);    
    //do not forget to add the currvData to targetJSON
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "curve_data", pJCurvArray);
    return TRUE;
}
/*
static BOOL Make_CurveData_ByStatkWh(FCGI_ROUGH_DATA* pFCGI_Rough, int nStat_TypeSel, int nDate_Type_Sel)
{
    typedef struct tagStatkWh
    {
        //BOOL bHasCurveData[MAX_STAT_CURVE_POINTS];
        float fkWhCurveData[MAX_STAT_CURVE_POINTS];
    }STAT_KWH;
    
    float fStat_ChgTotalkWh = 0.0;
    int nStat_ChgTotalTimes = 0;
    int nTotal = pFCGI_Rough->chargerDataRepo.nHisChgRecTotal;
    STAT_KWH stat_GunkWh[MAX_SUPPORT_GUN_NUM];
    ZERO_POBJS(stat_GunkWh, MAX_SUPPORT_GUN_NUM);
    STAT_KWH stat_AllkWh;
    ZERO_POBJS(&stat_AllkWh, 1);
    HIS_CHARGE_RECORD* pHisRecItem = pFCGI_Rough->chargerDataRepo.hisChgRecords;
    int n, k;
    int nGunIdx = 0;
    int nPointsTotal = Get_CurveData_Points(nStat_TypeSel, nDate_Type_Sel);  
    BOOL bMatched = FALSE;
    float fGetkWh;
    BOOL bIsGetPoint;
    BOOL bTimesCount = FALSE;
    for(n = 0; n < nTotal; n++, pHisRecItem++)
    {
        bMatched = FALSE;
        bTimesCount = FALSE;
        for(k = 0; k < nPointsTotal; k++)
        {            
            bIsGetPoint = FALSE;
            if((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR)
            {
                //will always get it for 48kWhpoint
                // //conditon @1 : if beginTime <= curr <= endtime for hour
                // if((Compare_Time_4ChgRec(pHisRecItem->tmChargeStart, nStat_TypeSel, nDate_Type_Sel, k) <= 0) &&
                //     (Compare_Time_4ChgRec(pHisRecItem->extraRecData.runData.tmChargeEnd, nStat_TypeSel, nDate_Type_Sel, k) >= 0))
                {
                    bIsGetPoint = TRUE;
                }
            }
            else
            {
                //conditon @1 : if beginTime == curr || curr == endtime for week/month
                if((Compare_Time_4ChgRec(pHisRecItem->tmChargeStart, nStat_TypeSel, nDate_Type_Sel, k) == 0) ||
                    (Compare_Time_4ChgRec(pHisRecItem->extraRecData.runData.tmChargeEnd, nStat_TypeSel, nDate_Type_Sel, k) == 0))
                {
                    bIsGetPoint = TRUE;
                }
            }  
            //for Times count, we only judge the startTime
            if(Compare_Time_4ChgRec(pHisRecItem->tmChargeStart, nStat_TypeSel, nDate_Type_Sel, k) == 0)   
            {
                bTimesCount = TRUE;
            }

            if(bIsGetPoint)
            {
                nGunIdx = pHisRecItem->emGunNum - 1;
                if((nGunIdx < 0) || (nGunIdx >= MAX_SUPPORT_GUN_NUM))
                {
                    TRACEX("---the GunNUM[%d] is OUT of Range!!!---\n", pHisRecItem->emGunNum);
                    break;
                }
                //for hour
                if((!bTimesCount) && ((nStat_TypeSel % 10) == STAT_TYPE_BY_HOUR) &&
                    (nDate_Type_Sel != DATE_TYPE_PER_ALL))
                {
                    continue;
                }
                fGetkWh = Get_StatkWh_by48Poinit_RawData(pFCGI_Rough, pHisRecItem, nStat_TypeSel, k);
                //TRACEX("---[%d, %d], getkWh -> %.1f---\n", n, k, fGetkWh);
                stat_GunkWh[nGunIdx].fkWhCurveData[k] += fGetkWh;
                stat_AllkWh.fkWhCurveData[k] += fGetkWh;
                fStat_ChgTotalkWh += fGetkWh;
                //nStat_ChgTotalTimes += 1;
                //fStat_ChgTotalkWh += pHisRecItem->extraRecData.runData.fCostkWh;
                bMatched = TRUE;
            }
        }
        //total times should be count here
        if(bMatched && bTimesCount)
        {
            nStat_ChgTotalTimes++;
        }
    }
    //now push the data to JSON
    cJSON_AddFloatToObject(pFCGI_Rough->pJSON_Resp, "total_chgkWh", fStat_ChgTotalkWh, 1);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "total_chgTimes", nStat_ChgTotalTimes); 
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "y-axis", "kWh"); //will re-use other res-id instead
    cJSON* pJCurvArray = cJSON_CreateArray();
    WEB_PARAM_VALUE* pWParVal;
    int nGunRawStatus;
    char szName[32];
    
    //each gun Stat
    for(k = 0; k < MAX_SUPPORT_GUN_NUM; k++)
    {
        //get status param
        pWParVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 201 + k, 101);
        if(pWParVal == NULL)
        {
            TRACEX("-ERRRRRR--1\n");
            break;
        }
        nGunRawStatus = pWParVal->currValue.emValue;
        if(nGunRawStatus == GUN_STATE_NOT_INSTALL)
        {
            //TRACEX("-ERRRRRR--2\n");
            break;
        }
        cJSON* pJCurvGun = cJSON_CreateObject();
        //now make name
        snprintf(szName, 32, "Gun-%d Total Charge kWh", k + 1);
        cJSON_AddStringToObject(pJCurvGun, "name", szName);
        cJSON_AddItemToObject(pJCurvGun, "val_list", cJSON_CreateFloatArrayEx(stat_GunkWh[k].fkWhCurveData, 1, nPointsTotal));
        cJSON_AddItemToArray(pJCurvArray, pJCurvGun);
    }
    //all stat
    cJSON* pJCurvAll = cJSON_CreateObject();
    cJSON_AddStringToObject(pJCurvAll, "name", "All Guns Total Charge kWh");
    cJSON_AddItemToObject(pJCurvAll, "val_list", cJSON_CreateFloatArrayEx(stat_AllkWh.fkWhCurveData, 1, nPointsTotal));
    cJSON_AddItemToArray(pJCurvArray, pJCurvAll);    
    //do not forget to add the currvData to targetJSON
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "curve_data", pJCurvArray);
    return TRUE;
}
*/

static BOOL Handle_Home_CurveData(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int nStat_TypeSel = 0; //0-kWh By Month, 1-kWh By Week, 2-kWh By Hour; 10-Times By Month, 11-Times By Week, 12-Times By Hour
    int nDate_Type_Sel = 0; //0-All, 1-this(month/week.day), 2-last(month/week/day)
    if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "stat_sel=", &nStat_TypeSel))
    {
        Make_Web_OutPut(104, "The 'stat_sel' NOT provided!", NULL, NULL);
        return FALSE;  
    }
    if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "date_sel=", &nDate_Type_Sel))
    {
        Make_Web_OutPut(104, "The 'date_sel' NOT provided!", NULL,  NULL);
        return FALSE;  
    }
    //now get the stat data (either by times or by kWh)
    if((nStat_TypeSel >= 10) && (nStat_TypeSel <= 12))
    {
        return Make_CurveData_ByStatTimes(pFCGI_Rough, nStat_TypeSel, nDate_Type_Sel);
    }
    else if((nStat_TypeSel >= 0) && (nStat_TypeSel <= 2))
    {
        return Make_CurveData_ByStatkWh(pFCGI_Rough, nStat_TypeSel, nDate_Type_Sel);
    }

    Make_Web_OutPut(104, "The 'stat_sel' is NOT correct!", NULL, NULL);
    return FALSE;
}

BOOL webpage_handler_home(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----HOME page_handler was called!---\n");

    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nGetDataLevel = -1;
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "get_data=", &nGetDataLevel))
        {
            if(nGetDataLevel == 0)
            {
                return Handle_Home_GeneralData(pFCGI_Rough);
            }
            else
            {
                Make_Web_OutPut(104, "The 'get_data' value INVALID!", NULL, NULL);
                return FALSE;  
            } 
        }
        int nCabIdx = -1;        //edit by Kuangyue 20241107
        int nPackIdx = 1;        //edit by Kuangyue 20241107, add packName
        int nChartIdx = -1;
        int nDevtype = -1;
        int nRange = -1;
        int nTimeType = -1;
        int nPageId = -1;
        int nPageVol = -1;
        int nDaySector = -1;
        char szQueryTime[64] = "\0";
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "chart=", &nChartIdx))
        {
            if(nChartIdx >= 0 && nChartIdx < STAT_GRAPH_ID_MAX)
            {
                if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "devtype=", &nDevtype))
                {
                    if(nDevtype < 0 || nDevtype >= STAT_DEV_TYPE_MAX)
                    {
                        Make_Web_OutPut(104, "The 'devtype' value INVALID!", NULL, NULL);
                        return FALSE;
                    }
                }
                if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "range=", &nRange))
                {
                    if(nRange < 0 || nRange >= 4)
                    {
                        Make_Web_OutPut(104, "The 'range' value INVALID!", NULL, NULL);
                        return FALSE;
                    }
                }
                if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cabinet=", &nCabIdx))    //edit by Kuangyue 20241107
                {
                    if(nCabIdx < 0 || nCabIdx >= 4)
                    {
                        Make_Web_OutPut(104, "The 'nCabIdx' value INVALID!", NULL, NULL);
                        return FALSE;
                    }
                }
                if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "timetype=", &nTimeType))
                {
                    if(nTimeType < 0 || nTimeType >= 3)
                    {
                        Make_Web_OutPut(104, "The 'timetype' value INVALID!", NULL, NULL);
                        return FALSE;
                    }
                }
                ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "time=", &szQueryTime, 64);
                ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pageId=", &nPageId);
                ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "page_vol=", &nPageVol);
                ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pack=", &nPackIdx);//edit by Kuangyue 20241107, add packName
                ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "day=", &nDaySector);
                return Handle_Home_ChartData(pFCGI_Rough, nChartIdx, nDevtype, nRange, nCabIdx, nPackIdx, nTimeType, nPageId, szQueryTime, nPageVol, nDaySector);  //edit by Kuangyue 20241107
            }
            else
            {
                Make_Web_OutPut(104, "The 'chart' value INVALID!", NULL, NULL);
                return FALSE;
            }
        }

        int nPageIdx = -1;
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nPageIdx))
        {
            if(nPageIdx >= 0 && nPageIdx < HOME_PAGEID_MAX)
            {
                return Handle_Home_PageData(pFCGI_Rough, nPageIdx);
            }
            else
            {
                Make_Web_OutPut(104, "The 'type' value INVALID!", NULL, NULL);
                return FALSE;
            }
        }

        Make_Web_OutPut(104, "The end of the request url is illegal!", NULL, NULL);
        return FALSE;  
    }
    else if (pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_POST)
    {
        // printf("szQueryRaw = \n%s\n", pFCGI_Rough->szQueryRaw);
        // printf("emContType = [%d]\n", pFCGI_Rough->emContType);
        // printf("szBodyRaw = [\n%s\n]\n", pFCGI_Rough->szBodyRaw);
        cJSON* pJSON_Body = cJSON_Parse(pFCGI_Rough->szBodyRaw);
        //handle the POST request
        cJSON* pJDevice = NULL;
        cJSON* pJOnOff = NULL;
#define HOME_DEVICE_MAX 20

        cJSON* temp;
        //parse device
        if ((pJDevice = cJSON_GetObjectItem(pJSON_Body, "device")) == NULL)
        {
            Make_Web_OutPut(104, "The 'device' is NOT provided!", NULL, NULL);
            cJSON_Delete(pJSON_Body);
            return FALSE;
        }
        else if(pJDevice->valueint < 0 || pJDevice->valueint >= HOME_DEVICE_MAX)
        {
            Make_Web_OutPut(104, "The 'device' value INVALID!", NULL, NULL);
            return FALSE;
        }

        //parse onoff
        if ((pJOnOff = cJSON_GetObjectItem(pJSON_Body, "onoff")) == NULL)
        {
            Make_Web_OutPut(104, "The 'device' is NOT provided!", NULL, NULL);
            cJSON_Delete(pJSON_Body);
            return FALSE;
        }
        else if(pJOnOff->valueint < 0 || pJOnOff->valueint > 1)
        {
            Make_Web_OutPut(104, "The 'onoff' value INVALID!", NULL, NULL);
            return FALSE;
        }

        //now we can handle the post

    }
    else
    {
        Make_Web_OutPut(104, "Home page does NOT allow !",NULL,  NULL);
        return FALSE;  
    }
    
    return FALSE;
}

