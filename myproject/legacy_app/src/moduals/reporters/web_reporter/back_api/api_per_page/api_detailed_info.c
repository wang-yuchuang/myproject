#include "../fcgi_defs.h"
#define AIR_NUM 2
#define AIR_TAB_NUM 4

static void Web_StatusDispInfo_Init(FCGI_ROUGH_DATA* pFCGI_Rough, int Main_idx)
{
    int i, j;
    WEB_PARAM_VALUE* pWebParaVal;
    if((Main_idx == 0) || (Main_idx == 2) || (Main_idx == 3) || (Main_idx == 4))
    {
        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 500, 502);
        for(i = 0; i < MAX_BATTCABINET_NUM; ++i)
        {  
            switch(Main_idx)
            {
                case 0:
                     pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 6].bIsDisp = FALSE;
                // case 3:
                // case 4:
                //     pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i].bIsDisp = FALSE;
                    break;
                case 2:
                    pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 1].bIsDisp = FALSE;
                    break;
                default:
                    break;
            }
        }
        for(i = 0; i < pWebParaVal->currValue.ulValue; ++i)
        {
            switch(Main_idx)
            {
                // case 0:
                // case 3:
                // case 4:
                //     pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i].bIsDisp = TRUE;
                //     break;
                case 2:
                    pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 1].bIsDisp = TRUE;
                    break;
                default:
                    break;
            }
            WEB_PARAM_VALUE* pWebParaVal1 = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 601 + i, 450);
            if(pWebParaVal1->currValue.emValue != IMEU_TYPE_NOT_INSTALLED)
            {
                 pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 6].bIsDisp = TRUE;
            }
        }
        if(Main_idx == 0)
        {
            pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 611, 451);
            if(pWebParaVal->currValue.emValue != IMEU_TYPE_IMEU3)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[4].bIsDisp = FALSE;
            }
            else
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[4].bIsDisp = TRUE;
            }
            pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 612, 451);
            if(pWebParaVal->currValue.emValue != 3)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[5].bIsDisp = FALSE;
            }
            else
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[5].bIsDisp = TRUE;
            }            
            pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 621, 409);//edit by Kuangyue 20251130
            if(pWebParaVal->currValue.emValue != 2)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[10].bIsDisp = FALSE;
            }
            else
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[10].bIsDisp = TRUE;
            }
        }

        if(Main_idx == 2)
        {
            for(i = 0; i < 2; ++i)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 5].bIsDisp = FALSE;
            }
            for(i = 0; i < 2; ++i)
            {
                pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 981 + i, 449);
                if(pWebParaVal->currValue.emValue == 1)
                {
                    pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 5].bIsDisp = TRUE;
                }
            }

        }

        A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 200, 200, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS), (void*)&blobCCUSett))
        {
            printf("----ERRRRRR: can not get CCU Set----\n");
            return;
        }
        
        if(Main_idx == 3)
        {
            for(j = 1; j <= 24; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[j].bIsDisp = FALSE;
            }

            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_1; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[1 + j].bIsDisp = TRUE;
            }
            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_2; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[7 + j].bIsDisp = TRUE;
            }
            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_3; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[13 + j].bIsDisp = TRUE;
            }
            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_4; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[19 + j].bIsDisp = TRUE;
            }

        }  

        if(Main_idx == 4)  
        {
            for(j = 0; j <= 23; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[j].bIsDisp = FALSE;
            }

            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_5; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[j].bIsDisp = TRUE;
            }
            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_6; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[6 + j].bIsDisp = TRUE;
            }
            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_7; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[10 + j].bIsDisp = TRUE;
            }
            for(j = 0; j < blobCCUSett.ccuSettings.byNumGrp_8; ++j)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[18 + j].bIsDisp = TRUE;
            }            
        }   
    
    }
    // if((pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION) && (Main_idx == 0))
    // {
    //     for(i = 1; i < MAX_BATTCABINET_NUM; ++i)
    //     {  
    //         pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i].bIsDisp = FALSE;
    //     }
    //     pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[0].bIsDisp = TRUE;
    //     pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[0].Value = 237;
    // }

    if(Main_idx == 2)
    {   
        for(i = 0; i < 2; ++i)
        {
            pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 5].bIsDisp = FALSE;
        }
        for(i = 0; i < 2; ++i)
        {
            pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 981 + i, 449);
            if(pWebParaVal->currValue.emValue != 0)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 5].bIsDisp = TRUE;
            }
        }
    }

    if(Main_idx == 6)
    {
        //热管理设备信息页: 空调1/2未安装时隐藏对应 tab (与参数设置页一致)
        for(i = 0; i < AIR_NUM; ++i)
        {
            pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 951 + i, 448);
            if((pWebParaVal != NULL) && (pWebParaVal->currValue.emValue == AIR_NOT_INSTALLED))
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 1].bIsDisp = FALSE;
            }
        }
        //热管理设备信息页: 空调3/4 恒隐藏 (系统最多 AIR_NUM 台空调, 设备映射钳位到 951)
        for(i = AIR_NUM; i < AIR_TAB_NUM; ++i)
        {
            pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i + 1].bIsDisp = FALSE;
        }
        //热管理设备信息页: 水冷机未安装时隐藏对应 tab (与参数设置页一致)
        pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 961, 2099);
        if((pWebParaVal != NULL) && (pWebParaVal->currValue.emValue == 0))  // 0 = 未安装
        {
            pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[5].bIsDisp = FALSE;
        }
    }

    if(Main_idx == 5)
    {
        for(i = 0; i <  pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].nSubTotal; ++i)
        {  
            pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i].bIsDisp = FALSE;
        }
        for(i = 0; i < pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].nSubTotal; ++i)
        {  
            if(i < 11)
            {
                pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 401 + i, 401);
            }
            else if(i < 14)
            {
                pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 430 + i - 11, 401);
            }
            else
            {
                pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 440 + i - 14, 401);
            }
            if(pWebParaVal->currValue.emValue != 0)
            {
                pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[i].bIsDisp = TRUE;
            }
        }
    }

    return;
}

static BOOL Is_NeedSkipParam(FCGI_ROUGH_DATA* pFCGI_Rough, WEB_PARAM_VALUE* pParamVal)
{
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

static BOOL Create_StatusParamList_4Dev(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJParamList, int Main_idx, int Sub_idx)
{
    int n, m, k;
    WEB_PARAM_VALUE* pParamVal = pFCGI_Rough->chargerDataRepo.pstatusParList[Main_idx][Sub_idx];

    UINT Idx_Num = 0;
    char szParamVal[255];
    for(n = 0; n < pFCGI_Rough->chargerDataRepo.nTotal[Main_idx][Sub_idx]; n++, pParamVal++)
    {
        /*//do not handle the object 
        if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_OBJECT)
        {
            continue;
        }*/

       if(Main_idx == 4)//光伏
       {
            if((Sub_idx == 24)&&(pParamVal->pParamInfo->nParamId == 3699)) //光伏逆变器总的数据，不要单个逆变器数据
            {
                continue;
            }
            else if((Sub_idx >=25)&&(pParamVal->pParamInfo->nParamId == 3697)) //光伏逆变器单个数据不要总的数据
            {       
                continue;
            }
       }
        //skip those no need on paramSet
        if(Is_NeedSkipParam(pFCGI_Rough, pParamVal))
        {
            continue;
        }
         cJSON* pJParam = cJSON_CreateObject();
        if(pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
        {
            //cJSON* pJParam = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJParam, "id", pParamVal->pParamInfo->nParamId);
            cJSON_AddNumberToObject(pJParam, "idx", Idx_Num);
            cJSON_AddNumberToObject(pJParam, "name_resId", pParamVal->pParamInfo->nParamName_ResId);
            // if(pParamVal->pParamInfo->nParamUnit_ResId > 0)
            // {
                cJSON_AddNumberToObject(pJParam, "unit_resId", pParamVal->pParamInfo->nParamUnit_ResId);
            // }
            cJSON_AddNumberToObject(pJParam, "val_type", pParamVal->pParamInfo->emValType);

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
            if(pParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
            {
                //now for valid&value
                if((pParamVal->m_emValueAttribute & WEB_PARAM_MASK_VALID) != WEB_PARAM_MASK_VALID)
                {
                    cJSON_AddBoolToObject(pJParam, "valid", TRUE);
                    cJSON_AddStringToObject(pJParam, "val_string", "--");
                }
                else
                {
                    cJSON_AddBoolToObject(pJParam, "valid", TRUE);
                    cJSON_AddStringToObject(pJParam, "val_string", GetStrVal_FromWebParamVal(pParamVal, szParamVal, 255));
                }
            }
            cJSON_AddItemToArray(pJParamList, pJParam);
            Idx_Num++;     
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
                int num = 0;

                TRACEX("----Successfully get blob param[%d]!!!----\n", pParamVal->pParamInfo->nParamId);
                cJSON* pJSubParamList = cJSON_CreateArray();
                WEB_SUBBLOB_ITEM* pSubBlob = pBlobParam->pSubBlobList;
                for(m = 0; m < pBlobParam->nSubBlobTotal; m++, pSubBlob++)
                {
                    
                    if((pParamVal->pParamInfo->nParamId == 204) && (num > 8)) //special for ify bms data
                    {
                        continue;
                    }
                    num++;

                    cJSON* pJParam = cJSON_CreateObject();
                    cJSON_AddNumberToObject(pJParam, "id", pSubBlob->nSubParam_UniqId);
                    cJSON_AddNumberToObject(pJParam, "idx", Idx_Num);
                    cJSON_AddNumberToObject(pJParam, "name_resId",  pSubBlob->nName_ResId);
                    // if(pParamVal->pParamInfo->nParamUnit_ResId > 0)
                    // {
                        cJSON_AddNumberToObject(pJParam, "unit_resId", pSubBlob->nUnit_ResId);
                    // }
                    int nValType = GetSubBlobParam_ValType(pSubBlob);
                    cJSON_AddNumberToObject(pJParam, "val_type", nValType);
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
                        cJSON_AddItemToObject(pJParam, "enum_list", pJSubEnumList);
                    } 
                  
                    cJSON_AddStringToObject(pJParam, "val_string", GetStrVal_FromWebSubBlobVal(pSubBlob, szParamVal, 255) ); 

                    //cJSON_AddItemToArray(pJSubParamList, pJSubBlob);
                    Idx_Num++;
                    cJSON_AddItemToArray(pJParamList, pJParam);
                }

            }                
        }
        //now add back the item to array
        //cJSON_AddItemToArray(pJParamList, pJParam);
    }

    return TRUE;
}

// #define MAX_POWALLOC_RECT            4
// static BOOL Create_StatusParamList_runInfo(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJParamList, int Main_idx, int Sub_idx)
// {
//     A_BLOB_BRANCH_GRP_RECTDATA             szGrpRectData[MAX_POWALLOC_RECT];
//     A_BLOB_BRANCH_NODE_CALC_DATA           szNodeCalcData[MAX_POWALLOC_RECT];
//     int n = 0;
//     for(n = 0; n < MAX_POWALLOC_RECT; n++)
//     {
//         if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 211 + n, 201, sizeof(A_BLOB_BRANCH_GRP_RECTDATA), (void*)&szGrpRectData[n]))
//         {
//             TRACEX("----ERRRRRR: can not get BRANCH_GRP_RECTDATA----\n");
//             Make_Web_OutPut(104, "can not get BRANCH_GRP_RECTDATA!", NULL, NULL);
//             return TRUE;
//         }

//         if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 221 + n, 203, sizeof(A_BLOB_BRANCH_NODE_CALC_DATA), (void*)&szNodeCalcData[n]))
//         {
//             TRACEX("----ERRRRRR: can not get BRANCH_NODE_CALC_DATA----\n");
//             Make_Web_OutPut(104, "can not get BRANCH_NODE_CALC_DATA!", NULL, NULL);
//             return TRUE;
//         }
//     }

//     cJSON* pJParam = cJSON_CreateObject();
//     int Idx_Num = 0;
//     char szPowAllocName[64] = "\0";
//     cJSON_AddNumberToObject(pJParam, "id", 784);
//     cJSON_AddNumberToObject(pJParam, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam, "name_resId", 784);
//     cJSON_AddNumberToObject(pJParam, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fV | %.1fA | %d%%",
//         szGrpRectData[Sub_idx].fBatt_RealTime_Volt, szGrpRectData[Sub_idx].fBatt_RealTime_Curr, szGrpRectData[Sub_idx].nSOC);
//     cJSON_AddStringToObject(pJParam, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam);
    
//     Idx_Num++;
//     cJSON* pJParam1 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam1, "id", 785);
//     cJSON_AddNumberToObject(pJParam1, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam1, "name_resId", 785);
//     cJSON_AddNumberToObject(pJParam1, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam1, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.0fV | %.0fA | %.0fkW  | %.0fV",
//             szGrpRectData[Sub_idx].fACDC_RealTime_Volt, szGrpRectData[Sub_idx].fACDC_RealTime_Curr,
//             szGrpRectData[Sub_idx].fACDC_Max_Output_Pwr_Ability, szGrpRectData[Sub_idx].fACDCOutRelayVolt);
//     cJSON_AddStringToObject(pJParam1, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam1);

//     Idx_Num++;
//     cJSON* pJParam2 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam2, "id", 786);
//     cJSON_AddNumberToObject(pJParam2, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam2, "name_resId", 786);
//     cJSON_AddNumberToObject(pJParam2, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam2, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fV | %.1fA | %.1fkW",
//             szGrpRectData[Sub_idx].fDCDC_RealTime_Volt, szGrpRectData[Sub_idx].fDCDC_RealTime_Curr,
//             szGrpRectData[Sub_idx].fDCDC_Max_Output_Pwr_Ability);
//     cJSON_AddStringToObject(pJParam2, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam2);  
    
//     Idx_Num++;
//     cJSON* pJParam3 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam3, "id", 783);
//     cJSON_AddNumberToObject(pJParam3, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam3, "name_resId", 783);
//     cJSON_AddNumberToObject(pJParam3, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam3, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fV | %.1fA | %.1fkW",
//             szGrpRectData[Sub_idx].fMPPT_RealTime_Volt, szGrpRectData[Sub_idx].fMPPT_RealTime_Curr,
//             szGrpRectData[Sub_idx].fMPPT_RealTime_Volt*szGrpRectData[Sub_idx].fMPPT_RealTime_Curr*0.001);
//     cJSON_AddStringToObject(pJParam3, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam3);
    
//     Idx_Num++;
//     cJSON* pJParam4 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam4, "id", 792);
//     cJSON_AddNumberToObject(pJParam4, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam4, "name_resId", 792);
//     cJSON_AddNumberToObject(pJParam4, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam4, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkW | Avg:%.1fkW",
//             szNodeCalcData[Sub_idx].fACDC_Distri_4_BattChrg_Pwr, szNodeCalcData[Sub_idx].fAvg_ChrgLmtPower);
//     cJSON_AddStringToObject(pJParam4, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam4);
    
//     Idx_Num++;
//     cJSON* pJParam5 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam5, "id", 789);
//     cJSON_AddNumberToObject(pJParam5, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam5, "name_resId", 789);
//     cJSON_AddNumberToObject(pJParam5, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam5, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkW | Avg:%.1fkW",
//             szNodeCalcData[Sub_idx].fBatt_Distri_4_ACDC_Pwr, szNodeCalcData[Sub_idx].fAvg_DisChrgingLmtPower);
//     cJSON_AddStringToObject(pJParam5, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam5);
    
//     Idx_Num++;
//     cJSON* pJParam6 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam6, "id", 1971);
//     cJSON_AddNumberToObject(pJParam6, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam6, "name_resId", 1971);
//     cJSON_AddNumberToObject(pJParam6, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam6, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkVar | Avg:%.1fkVar",
//         szNodeCalcData[Sub_idx].fQoutput_Distri4ACDC, szNodeCalcData[Sub_idx].fAvg_QoutputLmt);
//     cJSON_AddStringToObject(pJParam6, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam6);
    
//     Idx_Num++;
//     cJSON* pJParam7 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam7, "id", 794);
//     cJSON_AddNumberToObject(pJParam7, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam7, "name_resId", 794);
//     cJSON_AddNumberToObject(pJParam7, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam7, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkW | %.1fkW | %.1fkW",
//                 szNodeCalcData[Sub_idx].fMax_DisChrg_PwrAbility, szNodeCalcData[Sub_idx].fBatt_Max_DisChrg_Pwr, szNodeCalcData[Sub_idx].fRemain_Batt_DisChrg_Pwr);
//     cJSON_AddStringToObject(pJParam7, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam7);
    
//     Idx_Num++;
//     cJSON* pJParam8 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam8, "id", 791);
//     cJSON_AddNumberToObject(pJParam8, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam8, "name_resId", 791);
//     cJSON_AddNumberToObject(pJParam8, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam8, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkW | %.1fkW",
//             szNodeCalcData[Sub_idx].fMax_Chrg_PwrAbility, szNodeCalcData[Sub_idx].fBatt_Max_Chrg_Pwr_Req);
//     cJSON_AddStringToObject(pJParam8, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam8);
    
//     Idx_Num++;
//     cJSON* pJParam9 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam9, "id", 796);
//     cJSON_AddNumberToObject(pJParam9, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam9, "name_resId", 796);
//     cJSON_AddNumberToObject(pJParam9, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam9, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkW",
//             szNodeCalcData[Sub_idx].fRemain_Batt_Chrg_Pwr_Req);
//     cJSON_AddStringToObject(pJParam9, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam9);   
    
//     return TRUE;
// }

// static BOOL Create_StatusParamList_Grp(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJParamList,TYPE_SIGNLE_RECT Inv_Type, int Main_idx, int Sub_idx)
// {

//     A_BLOB_RECT_GRP_DATA rectGrpData;
//     char szGrpPMName[64] = "\0";

//     if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 204, sizeof(A_BLOB_RECT_GRP_DATA), (void*)&rectGrpData))
//     {
//         TRACEX("----ERRRRRR: can not get Rect Grp Data----\n");
//         Make_Web_OutPut(104, "can not get Rect Grp Data!", NULL, NULL);
//         return TRUE;
//     }
    
//     cJSON* pJParam = cJSON_CreateObject();
//     int Idx_Num = 0;
//     char szPowAllocName[64] = "\0";
//     cJSON_AddNumberToObject(pJParam, "id", 2381);
//     cJSON_AddNumberToObject(pJParam, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam, "name_resId", 2381);
//     cJSON_AddNumberToObject(pJParam, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fkW", rectGrpData.fRatedPower[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam);

//     Idx_Num++;
//     cJSON* pJParam1 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam1, "id", 2382);
//     cJSON_AddNumberToObject(pJParam1, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam1, "name_resId", 2382);
//     cJSON_AddNumberToObject(pJParam1, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam1, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fA", rectGrpData.fRatedCurr[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam1, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam1);
    
//     Idx_Num++;
//     cJSON* pJParam2 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam2, "id", 122);
//     cJSON_AddNumberToObject(pJParam2, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam2, "name_resId", 122);
//     cJSON_AddNumberToObject(pJParam2, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam2, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fA", rectGrpData.fRealTime_MaxCurr[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam2, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam2);  
    
//     Idx_Num++;
//     cJSON* pJParam3 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam3, "id", 1920);
//     cJSON_AddNumberToObject(pJParam3, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam3, "name_resId", 1920);
//     cJSON_AddNumberToObject(pJParam3, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam3, "val_type", 21); 
//     if(rectGrpData.emWorkMode[Inv_Type + Sub_idx] == 0)
//     {
//         snprintf(szPowAllocName, 64, "%s", LoadLangText_ById(91));

//     }
//     else if(rectGrpData.emWorkMode[Inv_Type + Sub_idx] == 1)
//     {
//         snprintf(szPowAllocName, 64, "%s", LoadLangText_ById(709));
//     }
//     else if(rectGrpData.emWorkMode[Inv_Type + Sub_idx] == 2)
//     {
//         snprintf(szPowAllocName, 64, "%s", LoadLangText_ById(710));
//     }
//     else if(rectGrpData.emWorkMode[Inv_Type + Sub_idx] == 3)
//     {
//         snprintf(szPowAllocName, 64, "%s", LoadLangText_ById(714));
//     }
   
//     cJSON_AddStringToObject(pJParam3, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam3);
    
//     Idx_Num++;
//     cJSON* pJParam4 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam4, "id", 2383);
//     cJSON_AddNumberToObject(pJParam4, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam4, "name_resId", 2383);
//     cJSON_AddNumberToObject(pJParam4, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam4, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fV", rectGrpData.fCtrlVolt[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam4, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam4);
    
//     Idx_Num++;
//     cJSON* pJParam5 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam5, "id", 2384);
//     cJSON_AddNumberToObject(pJParam5, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam5, "name_resId", 2384);
//     cJSON_AddNumberToObject(pJParam5, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam5, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fA", rectGrpData.fCtrlCurr[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam5, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam5);
    
//     Idx_Num++;
//     cJSON* pJParam6 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam6, "id", 453);
//     cJSON_AddNumberToObject(pJParam6, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam6, "name_resId", 453);
//     cJSON_AddNumberToObject(pJParam6, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam6, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fV", rectGrpData.fRealTimeVolt[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam6, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam6);
    
//     Idx_Num++;
//     cJSON* pJParam7 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam7, "id", 454);
//     cJSON_AddNumberToObject(pJParam7, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam7, "name_resId", 454);
//     cJSON_AddNumberToObject(pJParam7, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam7, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fA", rectGrpData.fRealTimeCurr[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam7, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam7);
    
//     Idx_Num++;
//     cJSON* pJParam8 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam8, "id", 2385);
//     cJSON_AddNumberToObject(pJParam8, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam8, "name_resId", 2385);
//     cJSON_AddNumberToObject(pJParam8, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam8, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%d", rectGrpData.nOnlineRectNum[Inv_Type + Sub_idx]);
//     cJSON_AddStringToObject(pJParam8, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam8);
    
//     return TRUE;
// }

// static BOOL Create_StatusParamList_sing(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJParamList, int Main_idx, int Sub_idx)
// {

//     A_BLOB_SINGLE_RECT_INFO allsingleRectData;
//     if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
//     {
//         TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
//         Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
//         return TRUE;
//     }
    
//     int Grp = (Sub_idx - 5)/6;
//     int Sig_idx = (Sub_idx - 5)%6;

//     STU_SIGNLE_RECT_DATA singleRectData;
//     singleRectData = allsingleRectData.arrRect[Grp][Sig_idx];
    
//     cJSON* pJParam = cJSON_CreateObject();
//     int Idx_Num = 0;
//     char szPowAllocName[64] = "\0";
//     cJSON_AddNumberToObject(pJParam, "id", 159);
//     cJSON_AddNumberToObject(pJParam, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam, "name_resId", 159);
//     cJSON_AddNumberToObject(pJParam, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "Psum:%.1fkW, Pabc:%.1f|%.1f|%.1fkW",
//             singleRectData.fPtotalActive,
//             singleRectData.fPphase_Active[0],
//             singleRectData.fPphase_Active[1],
//             singleRectData.fPphase_Active[2]);
//     cJSON_AddStringToObject(pJParam, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam);
    
//     Idx_Num++;
//     cJSON* pJParam1 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam1, "id", 160);
//     cJSON_AddNumberToObject(pJParam1, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam1, "name_resId", 160);
//     cJSON_AddNumberToObject(pJParam1, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam1, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "Qsum:%.1fkVar, Qabc:%.1f|%.1f|%.1fkVar",
//             singleRectData.fPtotalReactive,
//             singleRectData.fPphase_Reactive[0],
//             singleRectData.fPphase_Reactive[1],
//             singleRectData.fPphase_Reactive[2]);
//     cJSON_AddStringToObject(pJParam1, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam1);
    
//     Idx_Num++;
//     cJSON* pJParam2 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam2, "id", 1702);
//     cJSON_AddNumberToObject(pJParam2, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam2, "name_resId", 1702);
//     cJSON_AddNumberToObject(pJParam2, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam2, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "Uabc:%.1f|%.1f|%.1fV, Iabc:%.1f|%.1f|%.1fA",
//             singleRectData.fPh_InputVolt[0], singleRectData.fPh_InputVolt[1], singleRectData.fPh_InputVolt[2],
//             singleRectData.fPh_InputCurr[0], singleRectData.fPh_InputCurr[1], singleRectData.fPh_InputCurr[2]);
//     cJSON_AddStringToObject(pJParam2, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam2);  
    
//     Idx_Num++;
//     cJSON* pJParam3 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam3, "id", 1701);
//     cJSON_AddNumberToObject(pJParam3, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam3, "name_resId", 1701);
//     cJSON_AddNumberToObject(pJParam3, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam3, "val_type", 21); 
//      snprintf(szPowAllocName, 64, "%.1fV|%.1fA", singleRectData.fRTOutputVolt, singleRectData.fRTOutputCurr);
//     cJSON_AddStringToObject(pJParam3, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam3);
    
//     Idx_Num++;
//     cJSON* pJParam4 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam4, "id", 610);
//     cJSON_AddNumberToObject(pJParam4, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam4, "name_resId", 610);
//     cJSON_AddNumberToObject(pJParam4, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam4, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.0f\u2103", singleRectData.fAmbit_Temp);
//     cJSON_AddStringToObject(pJParam4, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam4);
    
//     Idx_Num++;
//     cJSON* pJParam5 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam5, "id", 143);
//     cJSON_AddNumberToObject(pJParam5, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam5, "name_resId", 143);
//     cJSON_AddNumberToObject(pJParam5, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam5, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.1fHz", singleRectData.fAC_Freq);
//     cJSON_AddStringToObject(pJParam5, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam5);

//     Idx_Num++;
//     cJSON* pJParam6 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam6, "id", 125);
//     cJSON_AddNumberToObject(pJParam6, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam6, "name_resId", 125);
//     cJSON_AddNumberToObject(pJParam6, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam6, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
//     cJSON_AddStringToObject(pJParam6, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam6);

//     Idx_Num++;
//     cJSON* pJParam7 = cJSON_CreateObject();
//     cJSON_AddNumberToObject(pJParam7, "id", 315);
//     cJSON_AddNumberToObject(pJParam7, "idx", Idx_Num);
//     cJSON_AddNumberToObject(pJParam7, "name_resId", 315);
//     cJSON_AddNumberToObject(pJParam7, "unit_resId", 0);
//     cJSON_AddNumberToObject(pJParam7, "val_type", 21); 
//     snprintf(szPowAllocName, 64, "%s", singleRectData.byBarCode);
//     cJSON_AddStringToObject(pJParam7, "val_string", szPowAllocName);
//     cJSON_AddItemToArray(pJParamList, pJParam7);

//     return TRUE;
// }

// static BOOL Create_StatusParamList_SEMS(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJParamList)
// {
//     int nSlaveNumber;
//     if (!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 43, 4, (void*)&nSlaveNumber))
//     {
//         nSlaveNumber = 0;
//     }

//     char szGrpPMName[64] = "\0";
//     size_t nUsed = 0;
//     A_BLOB_SEMS_CLIENTINFO clientInfo;
//     EMS_MGMT_STU* pEMS = &clientInfo.sems;
//     A_BLOB_BRANCH_GRP_RECTDATA* pBranchNodeData_SEMS;
//     A_BLOB_BRANCH_NODE_CALC_DATA* pCalc_Node;
//     SEMS_B6_LSYS_DATA_INFO* pB6LSYSLast_SEMS;
//     A_BLOB_SEMS_CLIENTINFO_SUM clientSum;
//     A_BLOB_SEMS_CLIENTINFO_SUM* pClientSum = &clientSum;
//     if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3900, sizeof(A_BLOB_SEMS_CLIENTINFO_SUM), (void*)pClientSum))
//     {
//         TRACEE("----ERRRRRR: can not get A_BLOB_SEMS_CLIENTINFO_SUM Data----%d", 3300);
//     }    

//     int Idx_Num = 0;
//     int n;
//     for(n = 0; n < nSlaveNumber; n++)
//     {
//         if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3301 + n, sizeof(A_BLOB_SEMS_CLIENTINFO), (void*)&clientInfo))
//         {
//             TRACEE("----ERRRRRR: can not get A_BLOB_SEMS_CLIENTINFO Data----%d", 3301 + n);
//             continue;
//         }
//         pBranchNodeData_SEMS =  &clientInfo.branchNodeData_SEMS;
//         pCalc_Node =       &pEMS->calc_Node[n];
//         pB6LSYSLast_SEMS = &pBranchNodeData_SEMS->b6LSYSLast_SEMS;
//         cJSON* pJSGrpPMInfo = cJSON_CreateObject();

//         cJSON* pJParam = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam, "id", 109);
//         cJSON_AddNumberToObject(pJParam, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam, "name_resId", 109);
//         cJSON_AddNumberToObject(pJParam, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "%d", n + 1);
//         cJSON_AddStringToObject(pJParam, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam);
//         Idx_Num++;

//         // IP Address
//         cJSON* pJParam1 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam1, "id", 86);
//         cJSON_AddNumberToObject(pJParam1, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam1, "name_resId", 86);
//         cJSON_AddNumberToObject(pJParam1, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam1, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "com:%s|%s:%d|No:%u",
//             clientInfo.bCommFailSemsClient?LoadLangText_ById(201):LoadLangText_ById(34),
//             inet_ntoa(*(struct in_addr *)clientInfo.stClientInfo.szAddr), clientInfo.stClientInfo.nPeerPort, pB6LSYSLast_SEMS->idxClient);
//         cJSON_AddStringToObject(pJParam1, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam1);
//         Idx_Num++;
        
//         // 有功功率需求
//         // switch (pCalc_Node->emBatt_WorkMode)
//         switch (pEMS->planSectorNow.emBatt_WorkMode)
//         {
//         case BATT_WORKMODE_FIRST_DISCHRG:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(552));
//             break;
//         case BATT_WORKMODE_FIRST_CHRGING:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(808));
//             break;
//         case BATT_WORKMODE_IDLE:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(213));
//             break;
//         default:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(31));
//             break;
//         }
//         if (pEMS->enableREMSCtrl && clientSum.emEMSCtrlMode_REMS == EMSCtrlMode_Remote)
//         {
//             nUsed = strlen(szGrpPMName);
//             snprintf(szGrpPMName + nUsed, 64-nUsed,
//                 "|RP:%.1f", clientSum.fActvPwrReq_REMS);
//         }
//         nUsed = strlen(szGrpPMName);

//         cJSON* pJParam2 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam2, "id", 3990);
//         cJSON_AddNumberToObject(pJParam2, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam2, "name_resId", 3990);
//         cJSON_AddNumberToObject(pJParam2, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam2, "val_type", 21); 
//         snprintf(szGrpPMName + nUsed, 64-nUsed,
//             "|PCh:%.1f/%.1f|ld:%.1f/%.1fkW",
//             pEMS->fTotal_PessChgAvaliable, pEMS->fTotal_PessDischgLmt, pEMS->fPacload4Chg, pEMS->fPacload4Dischg);
//         cJSON_AddStringToObject(pJParam2, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam2);
//         Idx_Num++;

//        // 最大输出功率
//         cJSON* pJParam3 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam3, "id", 123);
//         cJSON_AddNumberToObject(pJParam3, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam3, "name_resId", 123);
//         cJSON_AddNumberToObject(pJParam3, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam3, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "ch:%.1f/%.1f|0:%.1f/%.1fkW|Curr:%.1fA",
//             pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum, pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum,
//             pB6LSYSLast_SEMS->fMax_Chrg_PwrAbilitySum0, pB6LSYSLast_SEMS->fMax_DisChrg_PwrAbilitySum0,
//             pB6LSYSLast_SEMS->fRealTime_MaxCurrSum);
//         cJSON_AddStringToObject(pJParam3, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam3);
//         Idx_Num++;

//         // 工作模式
//         switch (pB6LSYSLast_SEMS->emACDC_WorkStatuSum)
//         {
//         case STATU_ACDC_IDLE:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(91));
//             break;
//         case STATU_ACDC_RECTIFY:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(709));
//             break;
//         case STATU_ACDC_INVERTER:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(710));
//             break;
//         case STATU_ACDC_STANDBY:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(5567));
//             break;
//         default:
//             snprintf(szGrpPMName, 64, "%s", LoadLangText_ById(31));
//             break;
//         }
//         nUsed = strlen(szGrpPMName);

//         cJSON* pJParam4 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam4, "id", 1920);
//         cJSON_AddNumberToObject(pJParam4, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam4, "name_resId", 1920);
//         cJSON_AddNumberToObject(pJParam4, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam4, "val_type", 21); 
//         snprintf(szGrpPMName + nUsed, 64-nUsed,
//             "|s:%d|m:%d", pB6LSYSLast_SEMS->emCCUStateSum, pB6LSYSLast_SEMS->emPMCtrlModeSum);
//         cJSON_AddStringToObject(pJParam4, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam4);
//         Idx_Num++;

//         // SOC
//         cJSON* pJParam5 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam5, "id", 115);
//         cJSON_AddNumberToObject(pJParam5, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam5, "name_resId", 115);
//         cJSON_AddNumberToObject(pJParam5, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam5, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "SOC:%d/%d/%.1f|Stop:%u|%.1fkWh",
//             pB6LSYSLast_SEMS->nSOCMinSum, pB6LSYSLast_SEMS->nSOCMaxSum, pB6LSYSLast_SEMS->fSOCSum, pCalc_Node->nStopSOCReq, pB6LSYSLast_SEMS->fChrgedEnergySum);
//         cJSON_AddStringToObject(pJParam5, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam5);
//         Idx_Num++;

//         // 功率分配
//         cJSON* pJParam6 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam6, "id", 1445);
//         cJSON_AddNumberToObject(pJParam6, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam6, "name_resId", 1445);
//         cJSON_AddNumberToObject(pJParam6, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam6, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "ch:%.1f|dis:%.1fkW",
//             pCalc_Node->fACDC_Distri_4_BattChrg_Pwr, pCalc_Node->fBatt_Distri_4_ACDC_Pwr);
//         cJSON_AddStringToObject(pJParam6, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam6);
//         Idx_Num++;

//         // 输出电压
//         cJSON* pJParam7 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam7, "id", 121);
//         cJSON_AddNumberToObject(pJParam7, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam7, "name_resId", 121);
//         cJSON_AddNumberToObject(pJParam7, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam7, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "%.1fV",
//             pB6LSYSLast_SEMS->fACDC_RealTime_VoltSum);
//         cJSON_AddStringToObject(pJParam7, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam7);
//         Idx_Num++;

//         // 输出电流
//         cJSON* pJParam8 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam8, "id", 454);
//         cJSON_AddNumberToObject(pJParam8, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam8, "name_resId", 454);
//         cJSON_AddNumberToObject(pJParam8, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam8, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "%.1fA",
//             pB6LSYSLast_SEMS->fACDC_RealTime_CurrSum);
//         cJSON_AddStringToObject(pJParam8, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam8);
//         Idx_Num++;

//         // 输出功率
//         cJSON* pJParam9 = cJSON_CreateObject();
//         cJSON_AddNumberToObject(pJParam9, "id", 2805);
//         cJSON_AddNumberToObject(pJParam9, "idx", Idx_Num);
//         cJSON_AddNumberToObject(pJParam9, "name_resId", 2805);
//         cJSON_AddNumberToObject(pJParam9, "unit_resId", 0);
//         cJSON_AddNumberToObject(pJParam9, "val_type", 21); 
//         snprintf(szGrpPMName, 64, "cab:%.1f/%.1f|pcs:%.1f|G:%.1f/%.1fkW",
//             pB6LSYSLast_SEMS->f_PcabMeterSum, pEMS->f_PcabMeter,
//             pB6LSYSLast_SEMS->fActvPwr_PCSGrpSum,
//             pB6LSYSLast_SEMS->f_PgridMeterSum, pClientSum->f_PgridMeterSum);
//         cJSON_AddStringToObject(pJParam9, "val_string", szGrpPMName);
//         cJSON_AddItemToArray(pJParamList, pJParam9);
//         Idx_Num++;
//     }
//     return TRUE;
// }

void AddWebBlobCmd155_FromASingleESCInfo(WEB_BLOB_CMD155* webSingleESCInfo, A_BLOB_CMD155* singleESCInfo, int nESCNum)
{
    int i;
    for (i = 0; i < nESCNum; ++i)
    {
        webSingleESCInfo->escDatas[i].dev_id = singleESCInfo->escDatas[i].dev_id;
        webSingleESCInfo->escDatas[i].tmStamp = BcdTimeStmp2time_t(&singleESCInfo->escDatas[i].tmStamp);
        // webSingleESCInfo->escDatas[i].tmStamp = 1751014426; // for test, will be replaced by real time
        webSingleESCInfo->escDatas[i].ac_workstat = singleESCInfo->escDatas[i].ac_workstat;
        webSingleESCInfo->escDatas[i].ac_volts[0] = singleESCInfo->escDatas[i].ac_volts[0];
        webSingleESCInfo->escDatas[i].ac_volts[1] = singleESCInfo->escDatas[i].ac_volts[1];
        webSingleESCInfo->escDatas[i].ac_volts[2] = singleESCInfo->escDatas[i].ac_volts[2];
        webSingleESCInfo->escDatas[i].ac_currs[0] = singleESCInfo->escDatas[i].ac_currs[0];
        webSingleESCInfo->escDatas[i].ac_currs[1] = singleESCInfo->escDatas[i].ac_currs[1];
        webSingleESCInfo->escDatas[i].ac_currs[2] = singleESCInfo->escDatas[i].ac_currs[2];
        webSingleESCInfo->escDatas[i].kw_meter_cab = singleESCInfo->escDatas[i].kw_meter_cab;
        webSingleESCInfo->escDatas[i].acdc_workstat = singleESCInfo->escDatas[i].acdc_workstat;
        webSingleESCInfo->escDatas[i].acdc_volt = singleESCInfo->escDatas[i].acdc_volt;
        webSingleESCInfo->escDatas[i].acdc_curr = singleESCInfo->escDatas[i].acdc_curr;
        webSingleESCInfo->escDatas[i].acdc_ext_volt = singleESCInfo->escDatas[i].acdc_ext_volt;
        webSingleESCInfo->escDatas[i].batt_workstat = singleESCInfo->escDatas[i].batt_workstat;
        webSingleESCInfo->escDatas[i].batt_soc = singleESCInfo->escDatas[i].batt_soc;
        webSingleESCInfo->escDatas[i].batt_volt = singleESCInfo->escDatas[i].batt_volt;
        webSingleESCInfo->escDatas[i].batt_curr = singleESCInfo->escDatas[i].batt_curr;
    }
    
}

static int Create_StatusParamList_4Cabinet(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJParamList, WEB_BLOB_CMD155* webSingleESCInfo, int nIndex)
{
    // printf("----Create_StatusParamList_4Cabinet called!----\n");

    // param name, param unit, param value

    WEB_ESC_INFO CabinetItem[] = {
        
        {117, 0,        WEB_VALTYPE_INT,        (void*)&(webSingleESCInfo->escDatas[nIndex].dev_id)},
        {167, 0,        WEB_VALTYPE_TIME,       (void*)&(webSingleESCInfo->escDatas[nIndex].tmStamp)},
        // {235, 0,        WEB_VALTYPE_INT,        (void*)&(webSingleESCInfo->escDatas[nIndex].ac_workstat)},
        {152, 161,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].ac_volts[2])},
        {154, 161,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].ac_volts[1])},
        {156, 161,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].ac_volts[0])},
        {153, 162,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].ac_currs[2])},
        {155, 162,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].ac_currs[1])},
        {157, 162,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].ac_currs[0])},
        {159, 165,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].kw_meter_cab)},
        // {1382, 0,       WEB_VALTYPE_INT,        (void*)&(webSingleESCInfo->escDatas[nIndex].acdc_workstat)},
        {1380, 161,     WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].acdc_volt)},
        {1381, 162,     WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].acdc_curr)},
        {3, 161,        WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].acdc_ext_volt)},
        // {643, 0,        WEB_VALTYPE_INT,        (void*)&(webSingleESCInfo->escDatas[nIndex].batt_workstat)},
        {571, 16,       WEB_VALTYPE_INT,        (void*)&(webSingleESCInfo->escDatas[nIndex].batt_soc)},
        {569, 161,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].batt_volt)},
        {570, 162,      WEB_VALTYPE_FLOAT_2,    (void*)&(webSingleESCInfo->escDatas[nIndex].batt_curr)}
    };
    int nESCInfoNum = ITEM_OF(CabinetItem);
    int i;
    for (i = 0; i < nESCInfoNum; i++)
    {
        cJSON* pJParam = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJParam, "id", i); // any id is ok, cabinet has no id
        cJSON_AddNumberToObject(pJParam, "idx", i + nIndex * nESCInfoNum); // any id is ok, cabinet has no id
        cJSON_AddNumberToObject(pJParam, "name_resId", CabinetItem[i].nParamNameResId); //cabinet has no name
        cJSON_AddNumberToObject(pJParam, "unit_resId", CabinetItem[i].nParamUnitResId);
        cJSON_AddNumberToObject(pJParam, "val_type", CabinetItem[i].emValType);  // expect enum and object, any type is ok
        char szParamVal[255] = {0};
        switch (CabinetItem[i].emValType)
        {
        case WEB_VALTYPE_INT:
            {
                sprintf(szParamVal, "%d", *(int*)(CabinetItem[i].ptrValue));
            }
            break;
        case WEB_VALTYPE_FLOAT_0:
            {
                sprintf(szParamVal, "%.0f", *(float*)(CabinetItem[i].ptrValue));
            }
            break;
        case WEB_VALTYPE_FLOAT_1:
            {
                sprintf(szParamVal, "%.1f", *(float*)CabinetItem[i].ptrValue);
            }
            break;
        case WEB_VALTYPE_FLOAT_2:
            {
                sprintf(szParamVal, "%.2f", *(float*)CabinetItem[i].ptrValue);
            }
            break;
        case WEB_VALTYPE_FLOAT_3:
            {
                sprintf(szParamVal, "%.3f", *(float*)CabinetItem[i].ptrValue);
            }
            break;
        case WEB_VALTYPE_FLOAT_4:
            {
                sprintf(szParamVal, "%.4f", *(float*)CabinetItem[i].ptrValue);
            }
            break;
        case WEB_VALTYPE_TIME:
            {
                // time is in seconds, convert to HH:MM:SS
                int totalSeconds = *(int*)(CabinetItem[i].ptrValue);
                TimeToString(totalSeconds, TIME_CHN_FMT, 
		            szParamVal, sizeof(szParamVal));
            }
            break;
        default:
            break;
        }
        cJSON_AddStringToObject(pJParam, "val_string", szParamVal);
        cJSON_AddItemToArray(pJParamList, pJParam); 
    }
    // printf("pJParamList = [%s]\n", cJSON_Print(pJParamList));
}

static BOOL Handle_DetailedInfo_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int Main_idx, int Sub_idx)
{
    int n;
    cJSON* pJDevList = cJSON_CreateArray();
    WEB_RUN_DEV* pRunDev = pFCGI_Rough->chargerDataRepo.pAllRunDevs;

    cJSON* pJParamList = cJSON_CreateArray();

    // if(Main_idx == 0)
    // {
    //    if((pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION) && (Sub_idx == 0))
    //    {
    //         Create_StatusParamList_SEMS(pFCGI_Rough, pJParamList);
    //    }
    //    else if((pFCGI_Rough->emSystemType != SYSTYPE_EMS_STATION) && (Sub_idx >= 0) && (Sub_idx <= 3))
    //    {
    //         Create_StatusParamList_runInfo(pFCGI_Rough, pJParamList, Main_idx, Sub_idx);
    //    }
        
    // }
    // else if((Main_idx == 3) && (Sub_idx >= 0) && (Sub_idx <= 3))
    // {
    //     Create_StatusParamList_Grp(pFCGI_Rough, pJParamList, TYPE_SIG_RECT_ACDC_GRP1, Main_idx, Sub_idx);
    // }
    // else if((Main_idx == 3) && (Sub_idx >= 5))
    // {
    //     Create_StatusParamList_sing(pFCGI_Rough, pJParamList, Main_idx, Sub_idx);
    // }
    // else if((Main_idx == 4) && (Sub_idx >= 0) && (Sub_idx <= 3))
    // {
    //     Create_StatusParamList_Grp(pFCGI_Rough, pJParamList, TYPE_SIG_RECT_MPPT_GRP1, Main_idx, Sub_idx);
    // }

    if((Main_idx == 1) && (Sub_idx == 4) && pFCGI_Rough->emSystemType == SYSTYPE_EMS_STATION)
    {
        // 获取柜子数量
        // 取出单柜子的信息
        A_BLOB_CMD155 singleESCInfo = {0};
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 1001, 3405, sizeof(A_BLOB_CMD155), (void*)&singleESCInfo))
        {
            TRACEX("----ERRRRRR: can not get singleESCInfo----\n");
            return TRUE;
        }
        int nESCNum = 0;
        Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1001, 3314, sizeof(int), (void*)&nESCNum);
        // 同步单柜子的信息到数据展示架构适配展示的结构体中 
        WEB_BLOB_CMD155 webSingleESCInfo = {0};
        AddWebBlobCmd155_FromASingleESCInfo(&webSingleESCInfo, &singleESCInfo, nESCNum);
        int i;
        for (i = 0; i < nESCNum; ++i)
        {
            cJSON* pJDev = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJDev, "id", 4);
            // for single DEV_CLASS_CABINET
            cJSON_AddBoolToObject(pJDev, "hide", /*bNeedHideDev*/ FALSE); 
            cJSON_AddNumberToObject(pJDev, "name_resId", 3301 + i);
            // cJSON* pJParamList = cJSON_CreateArray();
            Create_StatusParamList_4Cabinet(pFCGI_Rough, pJParamList, &webSingleESCInfo, i);
            
        }
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJParamList);
    }
    else
    {
        Create_StatusParamList_4Dev(pFCGI_Rough, pJParamList, Main_idx, Sub_idx);
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "data", pJParamList);  
    }

    Web_StatusDispInfo_Init(pFCGI_Rough, Main_idx);       
    cJSON* pJTabsInfo_Array = cJSON_CreateArray();
    int j = 0;

    for(j = 0; j < pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].nSubTotal; ++j)
    {
        if(!pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[j].bIsDisp)
        {
            continue;
        }
        cJSON* pJTabsInfo_Object_tab = cJSON_CreateObject();
        //cJSON_AddStringToObject(pJTabsInfo_Object_tab, "label", LoadLangText_ById(StatusParmDispList[Main_idx][j].Value));
        cJSON_AddStringToObject(pJTabsInfo_Object_tab, "label", LoadLangText_ById(pFCGI_Rough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[j].Value));
        cJSON_AddNumberToObject(pJTabsInfo_Object_tab, "value", j);
        cJSON_AddItemToArray(pJTabsInfo_Array, pJTabsInfo_Object_tab);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "tabs", pJTabsInfo_Array);

    return TRUE;
}


BOOL webpage_handler_detail_info(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----DETAIL_INFO page_handler was called!---\n");

    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int Main_idx = -1; //一级目录
        int Sub_idx = -1;  //二级目录
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "params=", &Main_idx))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }

        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &Sub_idx))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }

        return Handle_DetailedInfo_Data(pFCGI_Rough, Main_idx, Sub_idx);
    }
    else
    {
        Make_Web_OutPut(104, "Detailed_Info page does NOT allow POST!", NULL, NULL);
        return FALSE;  
    }
    
    return FALSE;
}

