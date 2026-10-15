#include "../fcgi_defs.h"

static BOOL Is_NeedSkipParam(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_PARAM_VALUE* pParamVal)
{
    int nParamId = pParamVal->pParamInfo->nParamId;
    if(/*(nParamId == 1) || (nParamId == 66) ||*/
        (nParamId == 4) || (nParamId == 24) || (nParamId == 12))
    {
        return TRUE;
    }

    //user authriry
    USER_LOGIN_LEVEL emUserLevel = pFCGI_Rough->pCurrSess->emUserLvel;
    UINT byMask = 0;
    if(emUserLevel == USER_LEVEL_ENGINEER)
    {
        byMask = (pParamVal->pParamInfo->uMask_AuthorityLevel & 0x01);
    }
    else if(emUserLevel == USER_LEVEL_ADMIN)
    {
        byMask = ((pParamVal->pParamInfo->uMask_AuthorityLevel >> 2) & 0x01);
    }
    //if only readable, then disable the display
    if(byMask != 0)
    {
        return TRUE;
    }

    //displayable
    if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_DISP) != WEB_PARAM_MASK_DISP)
    {
        return TRUE;
    } 

    return FALSE;
}

static BOOL IsParam_WriteAble(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_PARAM_VALUE* pParamVal)
{
    if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_CTRLSET) != WEB_PARAM_MASK_CTRLSET)
    {
        return FALSE;
    }

    //added by Jimmy 20210907: do not allow set network-settting for NPS in case of wrong set
    if(pFCGI_Rough->pCurrSess->bIsSess_FromNPS && (pParamVal->pParamInfo->nParamId == 3))
    {
        return FALSE;
    }

    //user authriry
    USER_LOGIN_LEVEL emUserLevel = pFCGI_Rough->pCurrSess->emUserLvel;
    UINT byMask = 0;
    if(emUserLevel == USER_LEVEL_ENGINEER)
    {
        byMask = (pParamVal->pParamInfo->uMask_AuthorityLevel & 0x02);
    }
    else if(emUserLevel == USER_LEVEL_ADMIN)
    {
        byMask = ((pParamVal->pParamInfo->uMask_AuthorityLevel >> 2) & 0x02);
    }
   
    if(byMask == 0x02)
    {
        return FALSE;
    }

    return TRUE;
}

// 分类
static WEB_PCS_PARAM_TYPE Get_PcsParamType_ByParamId(WEB_PARAM_INFO* paramInfo)
{
    int nParamId = paramInfo->nParamId;
    static int* ParamTypeList[WEB_PCS_PARAM_TYPE_MAX];
    ParamTypeList[WEB_PCS_PARAM_TYPE_BASE] = (int[]){1101, 1102, 1110, 1127, 2101, 2102, 2105, 2110, 2117,  
                                                    2118, 3111, 3112, 3123, 3160, 3161, 
                                                    /*哨兵*/-1};                                                 
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTHORITY] = (int []){
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BATPROTECT] = (int []){5101, 5102, 5103, 5104, 5105,
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_OTHER] = (int []){1113, 2140, 2142, 3113, 3114, 3124, 3125,
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_SPECIAL] = (int []){-1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_MONITOR] = (int []){-1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_NET] = (int []){3, -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_BACKCONFIG] = (int []){26, -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_STARTINFO] = (int []){17, -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_ONGRID] = (int []){220, -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_OFFGRID] = (int []){221, -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_AUTOCTL625] = (int []){210, -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_BLOB_CCU] = (int []){200, -1};

    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_COUNTRIES] = (int []){31121, 2101, 2102, 2141, 
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_VOLPROTECTION] = (int []){2120, 2122, 2121, 2123, 2130, 2126, 
                                                    2125, 2127, 
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_FREQPROTECTION] = (int []){2128, 21210, 2129, 21211, 21212, 
                                                    21214, 21213, 21215, 
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_OVERFLOADSHEDDING] = (int []){31132, 3131, 3135, 3133, 3132, /*指数响应使能预留*/
                                                    /*指数响应时间预留*/31156, /*过频降载停止点(同回载点)*/31315, 
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_UNDERFLOADING] = (int []){31133, 3136, /*欠频回载点预留*/3138, 31310, /*欠频回载速率预留*/                            
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_OVERVLOADSHEDDING] = (int []){31138, 3126, 3127, 3128, /*过压降载速率预留*/
                                                    /*自动切换充放电使能预留*//*过压降载结束点功率1*//*过压降载指数开环响应时间*/
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_REACTIVEPOWER] = (int []){311313, 2117, 3161, 311315, 3163, 2105, 
                                                    3165, 3168, 3164, 31615, 3166, 316151, 3167, 3169, 
                                                    31157, 31914, 31915, /*Vref电压设置*//*Vref电压跟踪电网电压使能位*/
                                                    /*Vref电压跟踪响应时间*/3160, /*P1,P2,P3,Q1,Q2,Q3*/
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_LOWVRIDETHROUGH] = (int []){31130, 3180, 3182, 3183, 3184, 3185, 3186, 3187, 
                                                    3188, 3189, 31810, 
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_HIGHVRIDETHROUGH] = (int []){31131, 3190, 3192, 3193, 3194, 3195, 3196, 3197, 
                                                    3198, 3199, 31910, 
                                                    -1};
    ParamTypeList[WEB_PCS_PARAM_TYPE_AUTU_RECONNECT] = (int []){/*使能重连*/31181, 31191, 311101, 311111, /*Enter service  Delay*/
                                                    /*Enter service 随机延迟使能*//*Enter service period*/
                                                    -1};

    int n;
    for(n = 0; n < WEB_PCS_PARAM_TYPE_MAX; n++)
    {
        int* pParamList = ParamTypeList[n];
        int m;
        for(m = 0; pParamList[m] != -1; m++)
        {
            // printf("nParamId = [%d], pParamList[%d][%d] = [%d]\n", nParamId, n, m, pParamList[m]);
            if(nParamId == pParamList[m])
            {
                // printf("find nParamId = [%d], pParamList[%d][%d] = [%d]\n", nParamId, n, m, pParamList[m]);
                return n;
            }
        }
    }

    if (paramInfo->nDevModelId == 100 || paramInfo->nDevModelId == 201)
    {
        return WEB_PCS_PARAM_TYPE_MONITOR;
    }

    if (paramInfo->nDevModelId == 700)
    {
        return WEB_PCS_PARAM_TYPE_OTHER;
    }
}

/**
 * @brief 判断是否是同一个sidebar内的参数
 * 
 * @param MainParam 主类型
 * @param SubParam  子类型
 */
static BOOL IsSubSidebar(WEB_PCS_PARAM_TYPE MainParam, WEB_PCS_PARAM_TYPE SubParam)
{
    switch (MainParam)
    {
    case WEB_PCS_PARAM_TYPE_BASE:
        if (SubParam == WEB_PCS_PARAM_TYPE_BASE ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_NET ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_ONGRID ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_OFFGRID ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_AUTOCTL625)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_AUTHORITY:
        if (SubParam == WEB_PCS_PARAM_TYPE_AUTHORITY ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_COUNTRIES ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_VOLPROTECTION ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_FREQPROTECTION ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_OVERFLOADSHEDDING ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_UNDERFLOADING ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_OVERVLOADSHEDDING ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_REACTIVEPOWER ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_LOWVRIDETHROUGH ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_HIGHVRIDETHROUGH ||
            SubParam == WEB_PCS_PARAM_TYPE_AUTU_RECONNECT)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BATPROTECT:
        if (SubParam == WEB_PCS_PARAM_TYPE_BATPROTECT)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_OTHER:
        if (SubParam == WEB_PCS_PARAM_TYPE_OTHER ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_STARTINFO)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_SPECIAL:
        if (SubParam == WEB_PCS_PARAM_TYPE_SPECIAL)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_MONITOR:
        if (SubParam == WEB_PCS_PARAM_TYPE_MONITOR ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_BACKCONFIG ||
            SubParam == WEB_PCS_PARAM_TYPE_BLOB_CCU)
        {
            return TRUE;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return FALSE;
}

static BOOL IsBlobDevMatchParam(WEB_PARAM_INFO* Dev, WEB_PCS_PARAM_TYPE ParamType)
{
    switch (ParamType)
    {
    case WEB_PCS_PARAM_TYPE_BLOB_NET:
        if (Dev->nParamId == 3)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BLOB_BACKCONFIG:
        if (Dev->nParamId == 26)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BLOB_STARTINFO:
        if (Dev->nParamId == 17)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BLOB_ONGRID:
        if (Dev->nParamId == 220)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BLOB_OFFGRID:
        if (Dev->nParamId == 221)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BLOB_AUTOCTL625:
        if (Dev->nParamId == 210)
        {
            return TRUE;
        }
        break;
    case WEB_PCS_PARAM_TYPE_BLOB_CCU:
        if (Dev->nParamId == 200)
        {
            return TRUE;
        }
        break;
    default:
        return FALSE;
        break;
    }
    return FALSE;
}

static BOOL Create_SettParamList_4Dev(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_RUN_DEV* pRunDev, cJSON* pJDataInfo, cJSON* pJTabsInfo_Array, WEB_PCS_PARAM_TYPE TabsID, int nSidebarID)
{
    // printf("==================Create_SettParamList_4Dev================\n");
    int n, m, k;
    int index = 1;
    WEB_PARAM_VALUE* pParamVal = pRunDev->pSettParamList;
    char szParamVal[255];
    for(n = 0; n < pRunDev->nSettParamTotal; n++, pParamVal++)
    {
        //skip those no need on paramSet
        if(Is_NeedSkipParam(pFCGI_Rough, pParamVal))
        {
            continue;
        }

        // 找出此参数是属于哪个 tabs 类型的参数
        WEB_PCS_PARAM_TYPE param_type = Get_PcsParamType_ByParamId(pParamVal->pParamInfo);

        // 过滤不用在本 sidebar 显示的参数
        BOOL bIsSubType = IsSubSidebar(nSidebarID, param_type);
        // printf("param_type = [%d], nSidebarID = [%d], param = [%d]\n", param_type, nSidebarID, pParamVal->pParamInfo->nParamId);
        if (!bIsSubType)
        {
            continue;
        }

        // 过滤不用在本 tabs 显示的参数
        if (param_type > WEB_PCS_PARAM_TYPE_MONITOR && 
            TabsID != param_type && 
            pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
        {
            continue;
        }

        cJSON* pJParam = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJParam, "id", pParamVal->pParamInfo->nParamId);
        cJSON_AddNumberToObject(pJParam, "name_resId", pParamVal->pParamInfo->nParamName_ResId);
        if(pParamVal->pParamInfo->nParamUnit_ResId > 0)
        {
            cJSON_AddNumberToObject(pJParam, "unit_resId", pParamVal->pParamInfo->nParamUnit_ResId);
        }
        int dev_id = 0;
        // 指定 PCS 需要的 DevID
        dev_id = pRunDev->pDevInfo->nDevId;
        cJSON_AddNumberToObject(pJParam, "dev_id", dev_id);
        cJSON_AddNumberToObject(pJParam, "val_type", pParamVal->pParamInfo->emValType);
        // printf("-----------create param view n = [%d], TabsID = [%d], param = [%d], val_type = [%d]--------------------------------\n", 
        //         n, TabsID, pParamVal->pParamInfo->nParamId, pParamVal->pParamInfo->emValType);

        // 如果是 BLOB 类型的参数，需要特殊处理
        if (TabsID >= WEB_PCS_PARAM_TYPE_BLOB_NET && TabsID <= WEB_PCS_PARAM_TYPE_BLOB_CCU)
        {
            // printf("收到 BLOB 类型参数 [%d] 请求, param = [%d]\n", TabsID, pParamVal->pParamInfo->nParamId);
            // 检查请求类型是否与本参数值匹配
            if (!IsBlobDevMatchParam(pParamVal->pParamInfo, TabsID))
            {
                // 不匹配的 BLOB 参数仅保留 tab 信息
                if (bIsSubType && pParamVal->pParamInfo->emValType == WEB_VALTYPE_OBJECT)
                {
                    // printf("加了tabs1111111111111111111111111111111111111111111\n");
                    cJSON* pJTabsInfo_Object_1 = cJSON_CreateObject();
                    cJSON_AddStringToObject(pJTabsInfo_Object_1, "label", LoadLangText_ById(pParamVal->pParamInfo->nParamName_ResId));
                    cJSON_AddNumberToObject(pJTabsInfo_Object_1, "value", param_type);
                    cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_1);
                }
                continue;
            }
            A_WEBBLOB_PARAM* pBlobParam = Get_ABlobParam_ByParamVal(pParamVal);
            if(!pBlobParam)
            {
                TRACEX("----Get an error for such blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);
                //return FALSE;
                cJSON_Delete(pJParam);
                continue;
            }    
            else
            {
                TRACE("----Successfully get blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);
                cJSON* pJSubParamList = cJSON_CreateArray();
                WEB_SUBBLOB_ITEM* pSubBlob = pBlobParam->pSubBlobList;
                for(m = 0; m < pBlobParam->nSubBlobTotal; m++, pSubBlob++)
                {
                    cJSON* pJSubBlob = cJSON_CreateObject();
                    cJSON_AddNumberToObject(pJSubBlob, "id", pSubBlob->nSubParam_UniqId);
                    cJSON_AddNumberToObject(pJSubBlob, "name_resId", pSubBlob->nName_ResId);
                    //Add_Trans_Into_Dictionary(pJDict, pSubBlob->nName_ResId);
                    if(pSubBlob->nUnit_ResId > 0)
                    {
                        cJSON_AddNumberToObject(pJSubBlob, "unit_resId", pSubBlob->nUnit_ResId);
                        //Add_Trans_Into_Dictionary(pJDict, pSubBlob->nUnit_ResId);
                    }
                    int nValType = GetSubBlobParam_ValType(pSubBlob);
                    cJSON_AddNumberToObject(pJSubBlob, "val_type", nValType);
                    cJSON_AddNumberToObject(pJSubBlob, "idx", index++);
                    if(nValType == WEB_VALTYPE_ENUM)
                    {
                        cJSON* pJSubEnumList = cJSON_CreateArray();
                        for(k = 0; k < pSubBlob->nEnumTotal; k++)
                        {
                            cJSON* pJSubEnum = cJSON_CreateObject();
                            cJSON_AddNumberToObject(pJSubEnum, "val", pSubBlob->enumList[k].emVal);
                            cJSON_AddNumberToObject(pJSubEnum, "key_res", pSubBlob->enumList[k].nEnumName_ResId);
                            cJSON_AddItemToArray(pJSubEnumList, pJSubEnum);
                        }
                        cJSON_AddItemToObject(pJSubBlob, "enum_list", pJSubEnumList);
                    }
                    else if(nValType == WEB_VALTYPE_INT)
                    {
                        cJSON_AddNumberToObject(pJSubBlob, "val_min", (int)pSubBlob->fMinRange);
                        cJSON_AddNumberToObject(pJSubBlob, "val_max", (int)pSubBlob->fMaxRange);
                    }
                    else if((nValType >= WEB_VALTYPE_FLOAT_0) && (nValType <= WEB_VALTYPE_FLOAT_4))
                    {
                        int nDotLen = 0;
                        if(nValType == WEB_VALTYPE_FLOAT_1)
                        {
                            nDotLen = 1;
                        }
                        else if(nValType == WEB_VALTYPE_FLOAT_2)
                        {
                            nDotLen = 2;
                        }
                        else //if(nValType == WEB_VALTYPE_FLOAT_3)
                        {
                            nDotLen = 3;
                        }
                        cJSON_AddFloatToObject(pJSubBlob, "val_min", pSubBlob->fMinRange, nDotLen);
                        cJSON_AddFloatToObject(pJSubBlob, "val_max", pSubBlob->fMaxRange, nDotLen);
                    }
                    else if((nValType == WEB_VALTYPE_STRING) ||
                        (nValType == WEB_VALTYPE_PASSWORD))
                    {
                        cJSON_AddStringToObject(pJSubBlob, "reg_exp", Make_RegExp_4String_SubBlobParam(pSubBlob));
                    }
                    cJSON_AddStringToObject(pJSubBlob, "val_string", 
                        GetStrVal_FromWebSubBlobVal(pSubBlob, szParamVal, 255));    
                    //setable
                    //if(!pSubBlob->bSetAble)
                    if(!Is_SubBlobParam_SetCtrlAble(pSubBlob, (int)pFCGI_Rough->pCurrSess->emUserLvel))
                    {
                        cJSON_AddBoolToObject(pJSubBlob, "ro", TRUE);
                    }   
                    cJSON_AddItemToArray(pJSubParamList, pJSubBlob);
                }
                cJSON_AddItemToObject(pJParam, "subparam_list", pJSubParamList);

                cJSON* pJTabsInfo_Object_1 = cJSON_CreateObject();
                cJSON_AddStringToObject(pJTabsInfo_Object_1, "label", LoadLangText_ById(pParamVal->pParamInfo->nParamName_ResId));
                cJSON_AddNumberToObject(pJTabsInfo_Object_1, "value", (int)param_type);
                cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_1);

                // printf("内部加入 data\n");
                cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJParam);

                continue;
            }
        }

        // printf("加载普通类型参数[%d], TabsID = [%d], sidebarID = [%d]..................\n", 
        //         pParamVal->pParamInfo->nParamId, TabsID, nSidebarID);
        char szTipText[255];
        if (dev_id >= 700 && dev_id <= 704)
        {
            snprintf(szTipText, 255, "我是 PCS%d ", dev_id - 700);
        }
        else
        {
            snprintf(szTipText, 255, "我是设备号 %d ", dev_id);
        }
        snprintf(szTipText, 255, "第 %d 个参数", index);
        cJSON_AddStringToObject(pJParam, "tip", szTipText);
        cJSON_AddNumberToObject(pJParam, "idx", index++);
        if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_ENUM)
        {
            cJSON* pJEnumList = cJSON_CreateArray();
            for(m = 0; m < pParamVal->pParamInfo->nEnumTotal; m++)
            {
                cJSON* pJEnum = cJSON_CreateObject();
                cJSON_AddNumberToObject(pJEnum, "val", pParamVal->pParamInfo->enumList[m].emVal);
                cJSON_AddNumberToObject(pJEnum, "key_res", pParamVal->pParamInfo->enumList[m].nEnumName_ResId);
                cJSON_AddItemToArray(pJEnumList, pJEnum);
            }
            cJSON_AddItemToObject(pJParam, "enum_list", pJEnumList);
        }
        else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_INT)
        {
            cJSON_AddNumberToObject(pJParam, "val_min", (int)pParamVal->pParamInfo->fMinLimit);
            cJSON_AddNumberToObject(pJParam, "val_max", (int)pParamVal->pParamInfo->fMaxLimit);
            cJSON_AddNumberToObject(pJParam, "val_default", pParamVal->pParamInfo->varValDefault.lValue);
            // printf("val_default INT = [%f] ID=[%d]\n", pParamVal->pParamInfo->varValDefault.lValue, pParamVal->pParamInfo->nParamId);
        }
        else if((pParamVal->pParamInfo->emValType >= WEB_VALTYPE_FLOAT_0) && (pParamVal->pParamInfo->emValType <= WEB_VALTYPE_FLOAT_4))
        {
            int nDotLen = 0;
            if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_FLOAT_1)
            {
                nDotLen = 1;
            }
            else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_FLOAT_2)
            {
                nDotLen = 2;
            }
            else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_FLOAT_3)
            {
                nDotLen = 3;
            }
            cJSON_AddFloatToObject(pJParam, "val_min", pParamVal->pParamInfo->fMinLimit, nDotLen);
            cJSON_AddFloatToObject(pJParam, "val_max", pParamVal->pParamInfo->fMaxLimit, nDotLen);
            cJSON_AddFloatToObject(pJParam, "val_default", pParamVal->pParamInfo->varValDefault.fValue, nDotLen);
            // printf("val_default FLOAT = [%f] ID=[%d]\n", pParamVal->pParamInfo->varValDefault.fValue, pParamVal->pParamInfo->nParamId);
        }
        else if((pParamVal->pParamInfo->emValType == WEB_VALTYPE_STRING) ||
            (pParamVal->pParamInfo->emValType == WEB_VALTYPE_PASSWORD))
        {
            cJSON_AddStringToObject(pJParam, "reg_exp", Make_RegExp_4StringParam(pParamVal));
            cJSON_AddStringToObject(pJParam, "val_string", pParamVal->pParamInfo->varValDefault.abyValue);
            // printf("val_default STRING = [%s] ID=[%d]\n", pParamVal->pParamInfo->varValDefault.abyValue, pParamVal->pParamInfo->nParamId);
        }
        else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_OBJECT)
        {
            A_WEBBLOB_PARAM* pBlobParam = Get_ABlobParam_ByParamVal(pParamVal);
            if(!pBlobParam)
            {
                TRACEX("----Get an error for such blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);
                cJSON_Delete(pJParam);
                continue;
            }    
            else
            {
                TRACE("----Successfully get blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);

                // printf("加了tabs3333333333333333333333333333333333333333333\n");
                cJSON* pJTabsInfo_Object_1 = cJSON_CreateObject();
                cJSON_AddStringToObject(pJTabsInfo_Object_1, "label", LoadLangText_ById(pParamVal->pParamInfo->nParamName_ResId));
                cJSON_AddNumberToObject(pJTabsInfo_Object_1, "value", (int)param_type);
                cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_1);
                continue;
            }                
        }

        //now for the value of none-object
        if(pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
        {
            // TRACEX("---param[%d,%d=>%s], m_emValueAttribute=>%d---\n", pRunDev->pDevInfo->nDevId, pParamVal->pParamInfo->nParamId,
            //     LoadLangText_ById(pParamVal->pParamInfo->nParamName_ResId), pParamVal->m_emValueAttribute);
            if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_VALID) != WEB_PARAM_MASK_VALID)
            {
                cJSON_AddBoolToObject(pJParam, "valid", FALSE);
                cJSON_AddStringToObject(pJParam, "val_string", "--");
            }
            else
            {
                //cJSON_AddBoolToObject(pJParam, "valid", TRUE);
                cJSON_AddStringToObject(pJParam, "val_string", GetStrVal_FromWebParamVal(pParamVal, szParamVal, 255));
            }
            // //setable/displayable
            // if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_DISP) != WEB_PARAM_MASK_DISP)
            // {
            //     cJSON_AddBoolToObject(pJParam, "hide", TRUE);
            // } 
            // /*else */if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_CTRLSET) != WEB_PARAM_MASK_CTRLSET)
            // {
            //     cJSON_AddBoolToObject(pJParam, "ro", TRUE);
            // }                    
        }  
        //else if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_CTRLSET) != WEB_PARAM_MASK_CTRLSET)
        if(!IsParam_WriteAble(pFCGI_Rough, pParamVal))
        {
            cJSON_AddBoolToObject(pJParam, "ro", TRUE);
        }          
        
        //now add back the item to array
        cJSON_AddItemToArray(pJDataInfo, pJParam);
    }

    return TRUE;
}

static BOOL Is_NeedSkipDevClass(WEB_DEV_CLASS emDevClass)
{
    if((emDevClass == DEV_CLASS_CHARGER_SYSTEM) || 
        (emDevClass == DEV_CLASS_CCU) || 
        (emDevClass == DEV_CLASS_PCS_GROUP))
    {
        return FALSE;
    }
    return TRUE;
}

static BOOL Is_NeedSkipDevID(FCGI_ROUGH_DATA *pFCGI_Rough, int nDevId, int nPcsIndex)
{
    if(nDevId == 1 || nDevId == 200)
    {
        return FALSE;
    }

    // // 235kW PCS 监控只显示 PCS 组的参数（235kWPCS 使用设备号 700）
    // if (pFCGI_Rough->emSystemType == SYSTYPE_SCADA_INPCS235KW && nDevId == 700)
    // {
    //     return FALSE;
    // }

    // // 62.5kW PCS 监控显示 4 个不同组的 PCS 设置信息（62.5kWPCS 使用设备号 701-704）
    // if (pFCGI_Rough->emSystemType == SYSTYPE_SCADA_INPCS && nDevId >= 701 && nDevId <= 704)
    // {
    //     if ((nDevId - 700) == nPcsIndex)
    //     {
    //         return FALSE;
    //     }
    // }

    return TRUE;
}

static BOOL Make_ParamSet_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int nClasssId, int nParam, int nPcsIndex)
{
    // printf("===========Make_ParamSet_Data===========\n");
    int n;

    cJSON* pJDataInfo = cJSON_CreateArray();
    // add tabs str base info
    char* szClasssName[] = {LoadLangText_ById(2757), 
                            LoadLangText_ById(2758), 
                            LoadLangText_ById(2759), 
                            LoadLangText_ById(2760), 
                            LoadLangText_ById(2761),
                            LoadLangText_ById(2762)};
    cJSON* pJTabsInfo_Array = cJSON_CreateArray();
    cJSON* pJPcsIndex_Array = cJSON_CreateArray();
    cJSON* pJTabsInfo_Object = cJSON_CreateObject();
    // nParam 代表左侧栏的分类， classsId 代表 tabs 的分类
    cJSON_AddStringToObject(pJTabsInfo_Object, "label", szClasssName[nParam]);
    WEB_PCS_PARAM_TYPE emParamType = (WEB_PCS_PARAM_TYPE)nParam;
    cJSON_AddNumberToObject(pJTabsInfo_Object, "value", emParamType);
    cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object);
    WEB_RUN_DEV* pRunDev = pFCGI_Rough->chargerDataRepo.pAllRunDevs;
    // printf("DevInfoTotal = [%d], nClasssId = [%d], nParam = [%d]\n", 
    //         pFCGI_Rough->chargerDataRepo.nDevInfoTotal, nClasssId, nParam);

    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nDevInfoTotal; n++, pRunDev++)
    {
        // （系统级分类过滤）遵守原始数据库属性，过滤 DevClass 中不需要 PCS 参数设置中显示的模型分类
        if(Is_NeedSkipDevClass(pRunDev->pDevInfo->emDevClass))
        {
            continue;
        }

        // （设备级过滤）过滤 DevId 中不需要 PCS 参数设置中显示的设备
        if(Is_NeedSkipDevID(pFCGI_Rough, pRunDev->pDevInfo->nDevId, nPcsIndex))
        {
            continue;
        }

        // printf("n = [%d], nDevId = [%d]\n", n, pRunDev->pDevInfo->nDevId);
        // （大分类细分 PCS 参数，并将 blob 单独提出来）
        Create_SettParamList_4Dev(pFCGI_Rough, pRunDev, pJDataInfo, pJTabsInfo_Array, nClasssId, nParam);
        
    }

    if (nClasssId <= WEB_PCS_PARAM_TYPE_MONITOR || nClasssId >= WEB_PCS_PARAM_TYPE_AUTU_COUNTRIES)
    {
        // printf("外部加入 data\n");
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJDataInfo);
    }

    
    if (nParam == WEB_PCS_PARAM_TYPE_AUTHORITY)
    {
        int i;
        // 认证主页面加入所有 tabs，其他页面的 tabs 由 Create_SettParamList_4Dev 内部加入
        for (i = 2763; i <= 2772; i++)
        {
            cJSON* pJTabsInfo_Object_tab = cJSON_CreateObject();
            cJSON_AddStringToObject(pJTabsInfo_Object_tab, "label", LoadLangText_ById(i));
            cJSON_AddNumberToObject(pJTabsInfo_Object_tab, "value", WEB_PCS_PARAM_TYPE_AUTU_COUNTRIES + i - 2763);
            cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_tab);
        }
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "tabs", pJTabsInfo_Array);

    // if (pFCGI_Rough->emSystemType == SYSTYPE_SCADA_INPCS && (
    //     nParam >= WEB_PCS_PARAM_TYPE_BASE && nParam <= WEB_PCS_PARAM_TYPE_SPECIAL ||
    //     nParam >= WEB_PCS_PARAM_TYPE_AUTU_COUNTRIES && nParam <= WEB_PCS_PARAM_TYPE_AUTU_RECONNECT))
    // {
    //     int i;
    //     // 需要区分 PCS 编号的加入编号，其他页面的编号由 Create_SettParamList_4Dev 内部加入
    //     for (i = 1; i <= 4; ++i)
    //     {
    //         cJSON* pJPcsIndex_Object = cJSON_CreateObject();
    //         char szPcsName[32] = {0};
    //         snprintf(szPcsName, 32, "PCS%d", i);
    //         cJSON_AddStringToObject(pJPcsIndex_Object, "label", szPcsName);
    //         cJSON_AddNumberToObject(pJPcsIndex_Object, "value", i);
    //         cJSON_AddItemToArray(pJPcsIndex_Array, pJPcsIndex_Object);
    //     }
    // }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "pcs", pJPcsIndex_Array);

    return TRUE;
}

static BOOL Parse_and_Handle_ParamSet(FCGI_ROUGH_DATA* pFCGI_Rough, int nParam)
{
    int n;
    cJSON* pJsonSet = cJSON_Parse(pFCGI_Rough->szBodyRaw);
    if(pJsonSet == NULL)
    {
        TRACE("--The ParamSet string is not a JSON->[%s]----\n", pFCGI_Rough->szBodyRaw);
        Make_Web_OutPut(104, "The 'param to be set' is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }
    // printf("pJsonSet = [%s]\n", cJSON_Print(pJsonSet));

    cJSON* temp;
    // parse dev_id
    if ((temp = cJSON_GetObjectItem(pJsonSet, "dev_id")) == NULL)
    {
        Make_Web_OutPut(104, "The 'dev_id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    int nDevId = temp->valueint;
    // printf("set json nDevId = [%d]\n", nDevId);

    // parse param_id
    if((temp = cJSON_GetObjectItem(pJsonSet, "id")) == NULL)
    {
        Make_Web_OutPut(104, "The 'id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    int nParamId = temp->valueint;
    // printf("set json nParamId = [%d]\n", nParamId);

    // parse param_id
    if((temp = cJSON_GetObjectItem(pJsonSet, "pcs_idx")) == NULL)
    {
        Make_Web_OutPut(104, "The 'id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    int nPcsIndex = temp->valueint;
    // printf("set json nPcsIndex = [%d]\n", nPcsIndex);

    //parse val_string/sub_val_list
    char *szValString = NULL;
    cJSON* pJSubValList = cJSON_GetObjectItem(pJsonSet, "sub_val_list");
    if((temp = cJSON_GetObjectItem(pJsonSet, "val_string")) != NULL)
    {
        printf("val_string = [%s]\n", temp->valuestring);
        if((temp->valuestring == NULL) || (temp->type != cJSON_String))
        {
            Make_Web_OutPut(104, "The 'val_string' string NOT correct!", NULL, NULL);
            cJSON_Delete(pJsonSet);
            return FALSE;
        }
        szValString = temp->valuestring;
    }
    // printf("szValString = [%s]\n", szValString);


    WEB_PARAM_VALUE* pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, nDevId, nParamId);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "The parameter is NOT found!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    
    //now set the param seperatedly
    int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
    int PCSFlag = 0;
    if(pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
    {
        //single param
        if(szValString == NULL)
        {
            Make_Web_OutPut(104, "The val_string is NOT found for single param!", NULL, NULL);
            cJSON_Delete(pJsonSet);
            return FALSE;
        }        
        //now handle the normal cmd
        if(nDevId == 200 && pParamVal->pParamInfo->nParamId == 219) 
        {
            A_BLOB_ES_RUN_DATA esRunData;
            if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 80, sizeof(A_BLOB_ES_RUN_DATA), (void*)&esRunData))
            {
                TRACEX("----ERRRRRR: can not get ES RunData----\n");
                return TRUE;
            }
            
            if((esRunData.fV_PCS3Phase[0][0] > 0) || (esRunData.fV_PCS3Phase[0][1] > 0) || (esRunData.fV_PCS3Phase[0][2] > 0))   //PCS����
            {
                nSetCtrl_ExeRtn = WEB_ERRCODE_EXECUTE_FAIL;
                PCSFlag = 1;
            }
            else     //PCS����     
            {
                nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, szValString, strlen(szValString));
                PCSFlag = 0;

            }

        }
        else
        {
            nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, szValString, strlen(szValString));
        }
        
    }
    else
    {
        if(pJSubValList == NULL)
        {
            Make_Web_OutPut(104, "The sub_val_list is NOT found for blob param!", NULL, NULL);
            cJSON_Delete(pJsonSet);
            return FALSE;
        }     
        A_WEBBLOB_PARAM blobParamVal;
        if(!Create_AWebBlob_Param(&blobParamVal, pParamVal, NULL))
        {
            Make_Web_OutPut(104, "Can NOT create the blob!", NULL, NULL);
            cJSON_Delete(pJsonSet);
            return FALSE;
        }
        int nParam2SetTotal = cJSON_GetArraySize(pJSubValList);
        cJSON* temp1;
        for(n = 0; n < nParam2SetTotal; n++)
        {
            cJSON* pJParamVal = cJSON_GetArrayItem(pJSubValList, n);
            if(pJParamVal == NULL)
            {
                Make_Web_OutPut(104, "Parse param array error!", NULL, NULL);
                cJSON_Delete(pJsonSet);
                return FALSE;
            }
            else if(pJParamVal->type != cJSON_Object)
            {
                Make_Web_OutPut(104, "The sub paramlist format is WRONG!", NULL, NULL);
                cJSON_Delete(pJsonSet);
                return FALSE;
            }
            if((temp1 = cJSON_GetObjectItem(pJParamVal, "param_id")) == NULL)
            {
                Make_Web_OutPut(104, "The sub 'param_id' NOT found!", NULL, NULL);
                cJSON_Delete(pJsonSet);
                return FALSE;
            }
            nParamId = temp1->valueint;
            if((temp1 = cJSON_GetObjectItem(pJParamVal, "val_string")) == NULL)
            {
                Make_Web_OutPut(104, "The sub 'val_string' NOT found!", NULL, NULL);
                cJSON_Delete(pJsonSet);
                return FALSE;
            }
            else if((temp1->valuestring == NULL) || (temp1->type != cJSON_String))
            {
                Make_Web_OutPut(104, "The sub 'val_string' string NOT correct!", NULL, NULL);
                cJSON_Delete(pJsonSet);
                return FALSE;
            }

            //new set the string to the sub-blob
            if(!SetStr_2WebSubBlob_ParamVal(&blobParamVal, nParamId, 
                temp1->valuestring, strlen(temp1->valuestring)))
            {
                Make_Web_OutPut(104, "Can NOT set the value for a SubBlobParam!", NULL, NULL);
                cJSON_Delete(pJsonSet);                
                return FALSE;
            }   
        }//for

        //now send the blob cmd
        nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, blobParamVal.pBlobVal_Raw, 
                pParamVal->nBlobSize);
        Delete_AWebBlob_Param(&blobParamVal);    
    }

    if(nSetCtrl_ExeRtn == WEB_ERRCODE_OK)
    {
        //now send back the refreshed run 
        Force_Update_ParamVals();
        Make_ParamSet_Data(pFCGI_Rough, GetClass_byDevId(nDevId), nParam, nPcsIndex);
    }
    else
    {
        char szErrCode[64];
        if(PCSFlag == 1)
        {
            snprintf(szErrCode, 64, "Error! Grid connected status, not allowed to commands to PCS");
        }
        else
        {
            snprintf(szErrCode, 64, "Execute errorCode=>%d!", nSetCtrl_ExeRtn);
        }

        Make_Web_OutPut(104, szErrCode, "Param Set Error!", NULL);
        cJSON_Delete(pJsonSet);
        //also need update it
        Force_Update_ParamVals();
        return FALSE;
    }
    
    cJSON_Delete(pJsonSet);
    return TRUE;   
}

BOOL webpage_handler_pcs_param_sett(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int nParams = -1;
    int pcs_idx = -1;
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        // int nGetDataLevel = -1;
        // if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "get_data=", &nGetDataLevel))
        // {
        //     Make_Web_OutPut(104, "The 'get_data' NOT provided!", NULL, NULL);
        //     return FALSE;  
        // }

        int nClassId = -1;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nClassId))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nClassId < WEB_PCS_PARAM_TYPE_BASE) || (nClassId > WEB_PCS_PARAM_TYPE_MAX))
        {
            Make_Web_OutPut(104, "The 'type' value INVALID!", NULL, NULL);
            return FALSE;  
        }

        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "params=", &nParams))
        {
            Make_Web_OutPut(104, "The 'params' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nClassId < WEB_PCS_PARAM_TYPE_BASE) || (nClassId > WEB_PCS_PARAM_TYPE_MAX))
        {
            Make_Web_OutPut(104, "The 'params' value INVALID!", NULL, NULL);
            return FALSE;  
        }

        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pcs_idx=", &pcs_idx))
        {
            Make_Web_OutPut(104, "The 'pcs_idx' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((pcs_idx < 0) || (pcs_idx > 4))
        {
            Make_Web_OutPut(104, "The 'pcs_idx' value INVALID!", NULL, NULL);
            return FALSE;  
        }

        // printf("----------------------------nClassId = [%d], nParams = [%d]\n", nClassId, nParams);

        return Make_ParamSet_Data(pFCGI_Rough, nClassId, nParams, pcs_idx);
    }
    else
    {
        // POST handler
        return Parse_and_Handle_ParamSet(pFCGI_Rough, nParams);
    }
    
    return FALSE;
}