#include "../fcgi_defs.h"

#include "../fcgi_defs.h"

static BOOL Is_NeedSkipParam(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_PARAM_VALUE* pParamVal)
{
    int nParamId = pParamVal->pParamInfo->nParamId;
    if((nParamId == 4) || (nParamId == 14) || (nParamId == 12) || 
        (nParamId == 15) || (nParamId == 151) ||
        (nParamId == 301) || (nParamId == 306))
    {
        return TRUE;
    }
    // LJH241101 MU3 LCD:10 20
#ifdef IMMU3
    if((nParamId == 10) || (nParamId == 20))
    {
        return TRUE;
    }
#endif

    //user authriry
    USER_LOGIN_LEVEL emUserLevel = pFCGI_Rough->pCurrSess->emUserLvel;
    UINT byMask = 0;
    if(emUserLevel == USER_LEVEL_ENGINEER)
    {
        byMask = (pParamVal->pParamInfo->uMask_AuthorityLevel & 0x03);
    }
    else if(emUserLevel == USER_LEVEL_ADMIN)
    {
        byMask = ((pParamVal->pParamInfo->uMask_AuthorityLevel >> 2) & 0x03);
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

static BOOL Create_CtrlParamList_4Dev(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_RUN_DEV* pRunDev, cJSON* pJParamList/*, cJSON* pJDict*/)
{
    int n, m, k;
    WEB_PARAM_VALUE* pParamVal = pRunDev->pCtrlParamList;
    for(n = 0; n < pRunDev->nCtrlParamTotal; n++, pParamVal++)
    {
        //skip those no need on paramSet
        if(Is_NeedSkipParam(pFCGI_Rough, pParamVal))
        {
            continue;
        }

        cJSON* pJParam = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJParam, "id", pParamVal->pParamInfo->nParamId);
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
                TRACEX("----Successfully get blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);

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

                    cJSON_AddBoolToObject(pJSubBlob, "ctrl_able", pSubBlob->bSetAble);  
                    cJSON_AddItemToArray(pJSubParamList, pJSubBlob);
                }
                cJSON_AddItemToObject(pJParam, "subparam_list", pJSubParamList);
            }                
        }

        //now for the value
        //if(pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
        {
            //controlable
            BOOL bControlAble = TRUE;
            if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_CTRLSET) != WEB_PARAM_MASK_CTRLSET)
            {
               bControlAble = FALSE;
            }     
            cJSON_AddBoolToObject(pJParam, "ctrl_able", bControlAble);  
        }  
        

        //now add back the item to array
        cJSON_AddItemToArray(pJParamList, pJParam);
    }

    return TRUE;
}

static BOOL Make_ManCtrl_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int nClasssId)
{
    // static INT_KEYVAL s_AllClasses[] = 
    // {
    //     {DEV_CLASS_CHARGER_SYSTEM,          93},
    //     {DEV_CLASS_CCU,                     196},
    //     {DEV_CLASS_METERS,                  234},
    //     {DEV_CLASS_CARD_READER,             98},
    //     {DEV_CLASS_OTHER,                   202},
    // };
    // //cJSON* pJDict = cJSON_CreateArray();
    // cJSON* pJDict = cJSON_CreateObject();
     int n;
    // cJSON* pJAllClassArr = cJSON_CreateArray();
    // //1: all class info
    // int n;
    // for(n = 0; n < ITEM_OF(s_AllClasses); n++)
    // {
    //     cJSON* pJClass = cJSON_CreateObject();
    //     cJSON_AddNumberToObject(pJClass, "id", s_AllClasses[n].key);
    //     cJSON_AddNumberToObject(pJClass, "name_resId", s_AllClasses[n].val);
    //     Add_Trans_Into_Dictionary(pJDict, s_AllClasses[n].val);
    //     cJSON_AddItemToArray(pJAllClassArr, pJClass);
    // }
    //2: current class info
    cJSON* pJCurrClassInfo = cJSON_CreateObject();
    cJSON_AddNumberToObject(pJCurrClassInfo, "id", nClasssId);
    cJSON* pJDevList = cJSON_CreateArray();
    WEB_RUN_DEV* pRunDev = pFCGI_Rough->chargerDataRepo.pAllRunDevs;
    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nDevInfoTotal; n++, pRunDev++)
    {
        if(pRunDev->pDevInfo->emDevClass == nClasssId)
        {
            cJSON* pJDev = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJDev, "id", pRunDev->pDevInfo->nDevId);
            cJSON_AddNumberToObject(pJDev, "name_resId", pRunDev->pDevInfo->nDevName_ResId);
            //Add_Trans_Into_Dictionary(pJDict, pRunDev->pDevInfo->nDevName_ResId);
            cJSON* pJParamList = cJSON_CreateArray();
            Create_CtrlParamList_4Dev(pFCGI_Rough, pRunDev, pJParamList/*, pJDict*/);
            cJSON_AddItemToObject(pJDev, "param_list", pJParamList);
            cJSON_AddItemToArray(pJDevList, pJDev);
        }
    }
    cJSON_AddItemToObject(pJCurrClassInfo, "devices_info", pJDevList);

    //now add back all data
    //cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dictionary", pJDict);
    //cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "class_list", pJAllClassArr);
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "curr_classInfo", pJCurrClassInfo);

    return TRUE;
}

// static BOOL Handle_ManCtrl_Run(FCGI_ROUGH_DATA* pFCGI_Rough, int nClasssId)
// {
//     int m,n,k;
//     cJSON* pJDevValList = cJSON_CreateArray();
//     WEB_RUN_DEV* pRunDev = pFCGI_Rough->chargerDataRepo.pAllRunDevs;
//     BOOL bNeedHideDev;
//     WEB_PARAM_VALUE* pParamVal;
//     char szParamVal[255];
//     for(n = 0; n < pFCGI_Rough->chargerDataRepo.nDevInfoTotal; n++, pRunDev++)
//     {
//         if(pRunDev->pDevInfo->emDevClass == nClasssId)
//         {
//             bNeedHideDev = FALSE; //judge if need hide this device
//             cJSON* pJDevVal = cJSON_CreateObject();
//             cJSON_AddNumberToObject(pJDevVal, "id", pRunDev->pDevInfo->nDevId);
//             cJSON_AddBoolToObject(pJDevVal, "hide", bNeedHideDev);
//             cJSON* pJParamValList = cJSON_CreateArray();
//             pParamVal = pRunDev->pCtrlParamList;
//             for(m = 0; m < pRunDev->nCtrlParamTotal; m++, pParamVal++)
//             {
//                 cJSON* pJParam = cJSON_CreateObject();
//                 cJSON_AddNumberToObject(pJParam, "id", pParamVal->pParamInfo->nParamId);
//                 BOOL bControlAble = TRUE;
//                 if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_OBJECT)
//                 {                    
//                     A_WEBBLOB_PARAM* pBlobParam = Get_ABlobParam_ByParamVal(pParamVal);
//                     if(!pBlobParam)
//                     {
//                         TRACEX("----Get an error for such blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);
//                         //return FALSE;
//                         cJSON_Delete(pJParam);
//                         continue;
//                     }
//                     cJSON* pJSubParamList = cJSON_CreateArray();
//                     WEB_SUBBLOB_ITEM* pSubBlob = pBlobParam->pSubBlobList;
//                     for(k = 0; k < pBlobParam->nSubBlobTotal; k++, pSubBlob++)
//                     {
//                         cJSON* pJSubBlob = cJSON_CreateObject();
//                         cJSON_AddNumberToObject(pJSubBlob, "id", pSubBlob->nSubParam_UniqId);
//                         //cJSON_AddStringToObject(pJSubBlob, "val_string", 
//                         //        GetStrVal_FromWebSubBlobVal(pSubBlob, szParamVal, 255));    
//                         //controlable
//                         if(!pSubBlob->bSetAble)
//                         {
//                             bControlAble = FALSE;
//                         }     
//                         cJSON_AddBoolToObject(pJSubBlob, "ctrl_able", bControlAble);            
//                         cJSON_AddItemToArray(pJSubParamList, pJSubBlob);
//                     }
//                     cJSON_AddItemToObject(pJParam, "subparam_list", pJSubParamList);
//                 }
//                 else
//                 {
//                     // if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_VALID) != WEB_PARAM_MASK_VALID)
//                     // {
//                     //     cJSON_AddBoolToObject(pJParam, "valid", FALSE);
//                     // }
//                     // else
//                     // {
//                     //     cJSON_AddStringToObject(pJParam, "val_string", GetStrVal_FromWebParamVal(pParamVal, szParamVal, 255));
//                     // }
//                     //controlable
//                     if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_CTRLSET) != WEB_PARAM_MASK_CTRLSET)
//                     {
//                        bControlAble = FALSE;
//                     }     
//                     cJSON_AddBoolToObject(pJParam, "ctrl_able", bControlAble);  
//                 }  

//                 cJSON_AddItemToArray(pJParamValList, pJParam);
//             }
//             cJSON_AddItemToObject(pJDevVal, "param_list", pJParamValList);
//             cJSON_AddItemToArray(pJDevValList, pJDevVal);
//         }
//     }
//     cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "dev_values", pJDevValList);
//     return TRUE;
// }

static BOOL Parse_and_Handle_ManCtrl(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int n;
    cJSON* pJsonSet = cJSON_Parse(pFCGI_Rough->szBodyRaw);
    if(pJsonSet == NULL)
    {
        TRACEX("--The ManCtrl string is not a JSON->[%s]----\n", pFCGI_Rough->szBodyRaw);
        Make_Web_OutPut(104, "The 'param to be set' is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }
    cJSON* temp;
    //parse dev_id
    if ((temp = cJSON_GetObjectItem(pJsonSet, "dev_id")) == NULL)
    {
        Make_Web_OutPut(104, "The 'dev_id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    int nDevId = temp->valueint;
    //parse param_id
    if((temp = cJSON_GetObjectItem(pJsonSet, "param_id")) == NULL)
    {
        Make_Web_OutPut(104, "The 'param_id' is NOT provided!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    int nParamId = temp->valueint;
    //parse val_string/sub_val_list
    char *szValString = NULL;
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

    WEB_PARAM_VALUE* pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_CONTROL, nDevId, nParamId);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "The parameter is NOT found!", NULL, NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    
    //now set the param seperatedly
    int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
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
        nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, nDevId, szValString, strlen(szValString));
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
        Make_ManCtrl_Data(pFCGI_Rough, GetClass_byDevId(nDevId));
    }
    else
    {
        char szErrCode[64];
        snprintf(szErrCode, 64, "Execute errorCode=>%d!", nSetCtrl_ExeRtn);
        Make_Web_OutPut(104, szErrCode, "Param Contol Error!", NULL);
        cJSON_Delete(pJsonSet);
        return FALSE;
    }
    
    cJSON_Delete(pJsonSet);
    return TRUE;   
}

BOOL webpage_handler_manu_ctrl(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----PARAM_SET page_handler was called!---\n");

    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nClassId = -1;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nClassId))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nClassId < DEV_CLASS_CHARGER_SYSTEM) || (nClassId > DEV_CLASS_OTHER))
        {
            Make_Web_OutPut(104, "The 'type' value INVALID!", NULL, NULL);
            return FALSE;  
        }

        return Make_ManCtrl_Data(pFCGI_Rough, nClassId);
    }
    else
    {
        return Parse_and_Handle_ManCtrl(pFCGI_Rough);
    }
    
    return FALSE;
}