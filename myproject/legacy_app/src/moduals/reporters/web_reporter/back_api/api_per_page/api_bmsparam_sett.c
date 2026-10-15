#include "../fcgi_defs.h"


void debug_to_file(const char* msg) {
    FILE *fp = fopen("/tmp/bms_debug.log", "a+");
    if (fp) {
        fprintf(fp, "[DEBUG] %s\n", msg);
        fclose(fp);
    }
}

static WEB_DISP_INFO ParaComSetDispList[10] = {{2757}, {7116}, {7117}, {7113}};
#define ParaComSetDispList_len 4

static void Web_StatusDispInfo_Init(FCGI_ROUGH_DATA* pFCGI_Rough, int Main_idx)
{
    int i;
    WEB_PARAM_VALUE* pWebParaVal;
    if((Main_idx == 2) || ((Main_idx == 6)))
    {
        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 500, 502);
        for(i = 0; i < MAX_BATTCABINET_NUM; ++i)
        { 
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[i + 1].bIsDisp = FALSE;
        }
        for(i = 0; i < pWebParaVal->currValue.ulValue; ++i)
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[i + 1].bIsDisp = TRUE;
        }
    }

    if(Main_idx == 0)
    {
        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 611, 451);
        if(pWebParaVal->currValue.emValue != IMEU_TYPE_IMEU3)
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[6].bIsDisp = FALSE;
        }
        else
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[6].bIsDisp = TRUE;
        }
        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 612, 451);
        if(pWebParaVal->currValue.emValue != 3)
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[7].bIsDisp = FALSE;
        }
        else
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[7].bIsDisp = TRUE;
        }

        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 621, 409);
        if(pWebParaVal->currValue.emValue != 2)
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[9].bIsDisp = FALSE;
        }
        else
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[9].bIsDisp = TRUE;
        }

        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 200, 217);
        if(pWebParaVal->currValue.emValue < 2)
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[10].bIsDisp = FALSE;
        }
        else
        {
            pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[10].bIsDisp = TRUE;
        }

    }

    pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 200, 217);
    ParaComSetDispList[2].bIsDisp = FALSE;
    if(pWebParaVal->currValue.emValue != 1)
    {
        ParaComSetDispList[2].bIsDisp = TRUE;
    } 
    return;
}

static BOOL Is_NeedSkipParam(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_PARAM_VALUE* pParamVal)
{
    int nParamId = pParamVal->pParamInfo->nParamId;
    if(/*(nParamId == 1) || (nParamId == 66) ||*/
        (nParamId == 4) || (nParamId == 24))
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

static int WEB_GetParamIdIndex(int nParamId) 
{
    int i;
    for (i = 0; i < PARAM_ID_CNT; i++)
    {
        if (g_PCS_ParamId_List[i] == nParamId)
        {
            return i;
        }
    }
    return 0;
}


static int GetCmdCodeByParamId(UINT nParamId, char* szOut_Cmd, UINT buf_size)
{
    int i;
    if (szOut_Cmd == NULL || buf_size == 0)
    {
        return FALSE;
    }
    for (i = 0; i < MAP_LENGTH; i++)
    {
        if (ParamToCmdMap[i].nParam == nParamId)
        {
            strcpy(szOut_Cmd, ParamToCmdMap[i].szCmd);
            return TRUE; 
        }
    }

    return FALSE;
}

static UINT Create_SettParamList_4Dev(FCGI_ROUGH_DATA* pFCGI_Rough, int idx1_1, int Sub_idx, cJSON* pJParamList)
{
    int n, m, k;
    int Main_idx = idx1_1;
    if(idx1_1 == 1100)
    {
        Main_idx = 0;
    }
    WEB_PARAM_VALUE* pParamVal = pFCGI_Rough->chargerDataRepo.pSettParList[Main_idx][Sub_idx];
    A_BLOB_SINGLE_RECT_INFO allsingleRectData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
    {
        TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
        Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
        return TRUE;
    }
    char szParamVal[255];
    char szPCSRereadVal[255];
    char szCommand[64];
    if(pParamVal == NULL)
    {
        return 0;
    }
    int Idx_num = 0;
    char szParma_Type[32] = "\0"; //参数id和 参数是设置还是控制参数
    // printf("main = %d, Sub = %d Total = %d\n", Main_idx, Sub_idx, pFCGI_Rough->chargerDataRepo.nParamTotal[Main_idx][Sub_idx]);
    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nParamTotal[Main_idx][Sub_idx]; n++, pParamVal++)
    {
        if(Is_NeedSkipParam(pFCGI_Rough, pParamVal))
        {
            continue;
        } 
        if(idx1_1 == 1100)
        {
            if((pParamVal->pParamInfo->nParamId != 217) && (pParamVal->pParamInfo->nParamId != 2))
            {
                continue;
            }
        }
        if(Main_idx == 4)//光伏
        {
            if (pParamVal->pParamInfo->emParamType == PARAM_TYPE_SETTING)
            {
                if((Sub_idx == 0) && (pParamVal->pParamInfo->nParamId == 453))
                   /* || pParamVal->pParamInfo->nParamId == 453
                    || pParamVal->pParamInfo->nParamId == 454
                    || pParamVal->pParamInfo->nParamId == 456
                    || pParamVal->pParamInfo->nParamId == 457))*/
                {
                    continue;
                }

                if((Sub_idx != 0) && (pParamVal->pParamInfo->nParamId == 440 || pParamVal->pParamInfo->nParamId == 439))
                {
                    continue;
                }
            }
        }

        cJSON* pJParam = cJSON_CreateObject();

        cJSON_AddNumberToObject(pJParam, "dev_id", pParamVal->nDevid);

        snprintf(szParma_Type, 32, "%d-%d", pParamVal->pParamInfo->nParamId, pParamVal->pParamInfo->emParamType);
        cJSON_AddStringToObject(pJParam, "id", szParma_Type);
        //cJSON_AddNumberToObject(pJParam, "id", pParamVal->pParamInfo->nParamId);

        cJSON_AddNumberToObject(pJParam, "idx", Idx_num);
        Idx_num++;
        cJSON_AddNumberToObject(pJParam, "name_resId", pParamVal->pParamInfo->nParamName_ResId);
        //Add_Trans_Into_Dictionary(pJDict, pParamVal->pParamInfo->nParamName_ResId);
        if(pParamVal->pParamInfo->nParamUnit_ResId > 0)
        {
            cJSON_AddNumberToObject(pJParam, "unit_resId", pParamVal->pParamInfo->nParamUnit_ResId);
            //Add_Trans_Into_Dictionary(pJDict, pParamVal->pParamInfo->nParamUnit_ResId);
        }
        cJSON_AddNumberToObject(pJParam, "val_type", pParamVal->pParamInfo->emValType);
        if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_ENUM)
        {
            cJSON* pJEnumList = cJSON_CreateArray();
            for(m = 0; m < pParamVal->pParamInfo->nEnumTotal; m++)
            {
                cJSON* pJEnum = cJSON_CreateObject();
                cJSON_AddNumberToObject(pJEnum, "val", pParamVal->pParamInfo->enumList[m].emVal);
                cJSON_AddNumberToObject(pJEnum, "key_res", pParamVal->pParamInfo->enumList[m].nEnumName_ResId);
                //Add_Trans_Into_Dictionary(pJDict, pParamVal->pParamInfo->enumList[m].nEnumName_ResId);
                cJSON_AddItemToArray(pJEnumList, pJEnum);
            }
            cJSON_AddItemToObject(pJParam, "enum_list", pJEnumList);
        }
        else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_INT)
        {
            cJSON_AddNumberToObject(pJParam, "val_min", (int)pParamVal->pParamInfo->fMinLimit);
            cJSON_AddNumberToObject(pJParam, "val_max", (int)pParamVal->pParamInfo->fMaxLimit);
            cJSON_AddNumberToObject(pJParam, "val_default", pParamVal->pParamInfo->varValDefault.lValue);
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
            else //if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_FLOAT_3)
            {
                nDotLen = 3;
            }
            cJSON_AddFloatToObject(pJParam, "val_min", pParamVal->pParamInfo->fMinLimit, nDotLen);
            cJSON_AddFloatToObject(pJParam, "val_max", pParamVal->pParamInfo->fMaxLimit, nDotLen);
            cJSON_AddFloatToObject(pJParam, "val_default", pParamVal->pParamInfo->varValDefault.fValue, nDotLen);
        }
        else if((pParamVal->pParamInfo->emValType == WEB_VALTYPE_STRING) ||
            (pParamVal->pParamInfo->emValType == WEB_VALTYPE_PASSWORD))
        {
            cJSON_AddStringToObject(pJParam, "reg_exp", Make_RegExp_4StringParam(pParamVal));
        }
        else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_OBJECT)
        {
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
                    cJSON_AddNumberToObject(pJSubBlob, "idx", m);
                    cJSON_AddNumberToObject(pJSubBlob, "name_resId", pSubBlob->nName_ResId);
                    //Add_Trans_Into_Dictionary(pJDict, pSubBlob->nName_ResId);
                    if(pSubBlob->nUnit_ResId > 0)
                    {
                        cJSON_AddNumberToObject(pJSubBlob, "unit_resId", pSubBlob->nUnit_ResId);
                        //Add_Trans_Into_Dictionary(pJDict, pSubBlob->nUnit_ResId);
                    }
                    int nValType = GetSubBlobParam_ValType(pSubBlob);
                    cJSON_AddNumberToObject(pJSubBlob, "val_type", nValType);
                    if(nValType == WEB_VALTYPE_ENUM)
                    {
                        cJSON* pJSubEnumList = cJSON_CreateArray();
                        for(k = 0; k < pSubBlob->nEnumTotal; k++)
                        {
                            cJSON* pJSubEnum = cJSON_CreateObject();
                            cJSON_AddNumberToObject(pJSubEnum, "val", pSubBlob->enumList[k].emVal);
                            cJSON_AddNumberToObject(pJSubEnum, "key_res", pSubBlob->enumList[k].nEnumName_ResId);
                            //Add_Trans_Into_Dictionary(pJDict, pSubBlob->enumList[k].nEnumName_ResId);
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
                    cJSON_AddStringToObject(pJSubBlob, "val_reread", 
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

                cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJParam);
                return 1;
                continue;
            }      
          
        }
        if(pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
        {

            if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_VALID) != WEB_PARAM_MASK_VALID)
            {
                cJSON_AddBoolToObject(pJParam, "valid", TRUE);
                cJSON_AddStringToObject(pJParam, "val_string", "0");
                cJSON_AddStringToObject(pJParam, "val_reread", "0");
            }
            else
            {
                cJSON_AddStringToObject(pJParam, "val_string", GetStrVal_FromWebParamVal(pParamVal, szParamVal, 255));
                cJSON_AddStringToObject(pJParam, "val_reread", GetReadStrVal_FromWebParamVal(pParamVal, szParamVal, 255));
                cJSON* pJSVal_Reread = cJSON_CreateArray();
            }     
        }  
              
        cJSON_AddItemToArray(pJParamList, pJParam);
    }

    return 0;
}

//static BOOL Make_ParamSet_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int nClasssId, int type_idx)
static BOOL Make_ParamSet_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int Main_idx, int Sub_idx)
{
    int n;
    cJSON* pJTabsInfo_Array = cJSON_CreateArray();
    WEB_RUN_DEV* pRunDev = pFCGI_Rough->chargerDataRepo.pAllRunDevs;
    A_BLOB_SINGLE_RECT_INFO allsingleRectData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
    {
        TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
        Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
        return TRUE;
    }
    cJSON* pJParamList = cJSON_CreateArray();
    if(Create_SettParamList_4Dev(pFCGI_Rough, Main_idx, Sub_idx, pJParamList) == 0)
    {
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJParamList);
    }
    // log_debug("11111111111111111");
    Web_StatusDispInfo_Init(pFCGI_Rough, Main_idx); 
    int j = 0;
    for(j = 0; j < pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].nSubTotal; ++j)
    {
        if(!pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[j].bIsDisp)
        {
            continue;
        }
        cJSON* pJTabsInfo_Object_tab = cJSON_CreateObject();
        //cJSON_AddStringToObject(pJTabsInfo_Object_tab, "label", LoadLangText_ById(ParaSetDispList[Main_idx][j].Value));
        cJSON_AddStringToObject(pJTabsInfo_Object_tab, "label", LoadLangText_ById(pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[j].Value));
        // printf("--The tab label is [%s]----\n", LoadLangText_ById(pFCGI_Rough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[j].Value));
        cJSON_AddNumberToObject(pJTabsInfo_Object_tab, "value", j);
        cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_tab);
    }
    // log_debug("33333333333333333333333");
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "tabs", pJTabsInfo_Array);
    if(pFCGI_Rough->chargerDataRepo.pSettParList[Main_idx][Sub_idx]->nDevid > 700 && pFCGI_Rough->chargerDataRepo.pSettParList[Main_idx][Sub_idx]->nDevid < 705)
    {
        int nDevidx = pFCGI_Rough->chargerDataRepo.pSettParList[Main_idx][Sub_idx]->nDevid - 701;
        int nRereadColumn = allsingleRectData.nOnlineNum[nDevidx];
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "pcs", nRereadColumn);
    }
    return TRUE;
}

static BOOL Make_ParamSet_Data2(FCGI_ROUGH_DATA* pFCGI_Rough, int Main_idx)
{
    int n;
    cJSON* pJTabsInfo_Array = cJSON_CreateArray();
    int Sub_idx = 0;
    cJSON* pJParamList = cJSON_CreateArray();
    switch(Main_idx)
    {
        case 0:
            Main_idx = 1100;
            Sub_idx = 0;
            break;
        case 1:
            Main_idx = 1;
            Sub_idx = 2;
            break;
        case 2:
            Main_idx = 0;
            Sub_idx = 5;
            break;
        case 3:
            Main_idx = 1;
            Sub_idx = 5;
            break;
        default:
            return FALSE;
    }
    if(Create_SettParamList_4Dev(pFCGI_Rough, Main_idx, Sub_idx, pJParamList) == 0)
    {
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJParamList);
    }
    // log_debug("111111111111111");

    Web_StatusDispInfo_Init(pFCGI_Rough, Main_idx); 
    int j = 0;
    for(j = 0; j < ParaComSetDispList_len; ++j)
    {
        if(ParaComSetDispList[j].bIsDisp)
        {
            continue;
        }
        cJSON* pJTabsInfo_Object_tab = cJSON_CreateObject();
        cJSON_AddStringToObject(pJTabsInfo_Object_tab, "label", LoadLangText_ById(ParaComSetDispList[j].Value));
        cJSON_AddNumberToObject(pJTabsInfo_Object_tab, "value", j);
        cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_tab);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "tabs", pJTabsInfo_Array);

    return TRUE;
}

static BOOL Parse_and_Handle_ParamSet(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
    WEB_PARAM_VALUE* pParamVal;
    WEB_PARAM_VALUE SenParamVal;
    char *szValString = NULL;
    int nClassId = -1;
    char szSetVal[32] = {0};
    int n;
    cJSON* pJsonSet = cJSON_Parse(pFCGI_Rough->szBodyRaw);
    if(pJsonSet == NULL)
    {
        TRACE("--The ParamSet string is not a JSON->[%s]----\n", pFCGI_Rough->szBodyRaw);
        Make_Web_OutPut(104, "The 'param to be set' is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }
    cJSON* temp;
    if ((temp = cJSON_GetObjectItem(pJsonSet, "type")) != NULL)
    {
        nClassId = temp->valueint;
    }

    log_debug("nClassId = %d\n", nClassId);

    if(nClassId != -1)
    {
        snprintf(szSetVal, sizeof(szSetVal), "%d", nClassId);

        WEB_PARAM_INFO   pParamInfo;
        memset(&pParamInfo, 0, sizeof(WEB_PARAM_INFO));
        SenParamVal.pParamInfo = &pParamInfo;
        SenParamVal.pParamInfo->uMask_AuthorityLevel = 11;
        SenParamVal.pParamInfo->emParamType = PARAM_TYPE_SETTING;
        SenParamVal.pParamInfo->nParamId = 519;
        SenParamVal.pParamInfo->emValType = WEB_VALTYPE_INT;
        SenParamVal.pParamInfo->fMaxLimit = 9999;
        SenParamVal.pParamInfo->fMinLimit = 0;

        log_debug("wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww\n");

        nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(&SenParamVal, 500, szSetVal, strlen(szSetVal));
        if(nSetCtrl_ExeRtn == WEB_ERRCODE_OK)
        {
            //now send back the refreshed run 
            Force_Update_ParamVals();
            //Make_ParamSet_Data(pFCGI_Rough, GetClass_byDevId(nDevId));
        }
        cJSON_Delete(pJsonSet);
        return TRUE; 
    }
    //parse dev_id
    if ((temp = cJSON_GetObjectItem(pJsonSet, "dev_id")) == NULL)
    {
        Make_Web_OutPut(104, "The 'dev_id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    int nDevId = temp->valueint;

    if(nDevId == 701)
    {
        WEB_PARAM_VALUE* PCSSet_Param = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 1, 1099);
        nDevId = 700 + PCSSet_Param->currValue.emValue;
    }

    //parse param_id
    if((temp = cJSON_GetObjectItem(pJsonSet, "id")) == NULL)
    {
        Make_Web_OutPut(104, "The 'id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    if(temp->type != cJSON_String)
    {
        Make_Web_OutPut(104, "The 'id' is not a string!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }

    

    //解析出参数ID和是控制参数还是设置参数
    char szId[32] = "\0";
    size_t id_len = strlen(temp->valuestring);
    strncpy(szId, temp->valuestring, id_len);
    szId[id_len] = '\0';
    int num_count = 0;
    int nums[2];
    num_count = parse_str_to_number(szId, "-", nums, 2);
    int nParamId = nums[0];
    PARAM_TYPE ParamType = nums[1];

    //parse val_string/sub_val_list
    // char *szValString = NULL;
    cJSON* pJSubValList = cJSON_GetObjectItem(pJsonSet, "sub_val_list");
    if((temp = cJSON_GetObjectItem(pJsonSet, "val_string")) != NULL)
    {
        if((temp->valuestring == NULL) || (temp->type != cJSON_String))
        {
            Make_Web_OutPut(104, "The 'val_string' string NOT correct!", NULL, NULL);
            cJSON_Delete(pJsonSet);
            return FALSE;
        }
        szValString = temp->valuestring;
    }
    
    if(pJSubValList != NULL)
    {
        if(pJSubValList->type != cJSON_Array)
        {
            Make_Web_OutPut(104, "The 'sub_val_list' NOT array!", NULL, NULL);
            cJSON_Delete(pJsonSet);
            return FALSE;
        }
    }

    pParamVal = Get_ASingle_WebParaVal(ParamType, nDevId, nParamId);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "The parameter is NOT found!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }

    log_debug("dev_id = %d, id = %d\n", nDevId, nParamId);
    
    //now set the param seperatedly
    // int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
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
            
            char *compVal = "1"; // OffGrid Val
            if(!strcmp(szValString, compVal)) // OnGrid->OffGrid
            {
                if((esRunData.fV_PCS3Phase[0][0] > 20) || (esRunData.fV_PCS3Phase[0][1] > 20) || (esRunData.fV_PCS3Phase[0][2] > 20))   //PCS����
                {
                    nSetCtrl_ExeRtn = WEB_ERRCODE_EXECUTE_FAIL;
                    PCSFlag = 1;
                }
                else     
                {
                    nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, szValString, strlen(szValString));
                    PCSFlag = 0;
    
                }
            }
            else // OffGrid->OnGrid
            {
                    nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, szValString, strlen(szValString));
                    PCSFlag = 0;
            }

        }
        else
        {
            nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, szValString, strlen(szValString));
            log_debug("22222222    dev_id = %d, id = %d\n", nDevId, nParamId);
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
                if((temp1 = cJSON_GetObjectItem(pJParamVal, "id")) == NULL)
                {
                    Make_Web_OutPut(104, "The sub 'id' NOT found!", NULL, NULL);
                    cJSON_Delete(pJsonSet);
                    return FALSE;
                }
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
        //Make_ParamSet_Data(pFCGI_Rough, GetClass_byDevId(nDevId));
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


//  批量设置需要让前端把所有value值直接传输过来， 直接 重新弄一个参数 拼接一下 ，再发送给控制里面去
BOOL webpage_handler_bmsparam_sett(FCGI_ROUGH_DATA* pFCGI_Rough)
{
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
        return Make_ParamSet_Data(pFCGI_Rough, 9, nClassId);
    }
    else
    {
        return Parse_and_Handle_ParamSet(pFCGI_Rough);
    }
    
    return TRUE;
}