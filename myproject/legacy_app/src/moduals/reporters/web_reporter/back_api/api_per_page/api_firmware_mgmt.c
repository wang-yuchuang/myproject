// #define _TRACE_PRINT_
#include "../fcgi_defs.h"

#define RECT_UPDT_START_STATUS_NOT           0
#define RECT_UPDT_START_STATUS_FAILED        1
#define RECT_UPDT_START_STATUS_OK            2
#define RECT_UPDT_START_STATUS_FINISHED      3
#define UPDT_ONECLICK_STATUS_FLAG            4
#define UPDT_MODEL_STATUS_FLAG                5

#define AIR_NUM 2

static BOOL IMMU2_Upgrade_Flag = FALSE;
static BOOL s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_NOT; //for answer the rect-updt process
static BOOL s_emBMSorModel_Status = RECT_UPDT_START_STATUS_NOT;

static BOOL IsBinFile_CheckOK(FCGI_ROUGH_DATA* pFCGI_Rough, A_BLOB_UPDT_INFO_4CCU* pCmd)
{
    //example name: V1902U2V119.bin
    char* posStart = pFCGI_Rough->stMultiFormData.szRawFileName_FromClient;
    int nFileNameLen = strlen(posStart);
    if(nFileNameLen < 10)
    {
        return FALSE;
    }

    //judge the ".bin" postfix
    char* posBin = strstr(posStart, ".bin");
    if(!posBin)
    {
        return FALSE;
    }

    //match U2 -> DC, U1 -> PFC, version
    //char* posUx = strstr(posStart, "U2");
    // UINT nSetVer = (UINT)(pCmd->fDCDC_SWVer_4Updt * 100);
    // if(pCmd->emUpdtType != UPDT_TP_DCDC) 
    // {
    //     posUx = strstr(posStart, "U1");
    //     nSetVer = (UINT)(pCmd->fPFC_SWVer_4Updt * 100);
    // }
    char* posUx = strstr(posStart, "U2");
    float fSetVer = pCmd->fDCDC_SWVer_4Updt;
    if(pCmd->emUpdtType != UPDT_TP_DCDC) 
    {
        posUx = strstr(posStart, "U1");
        fSetVer = pCmd->fPFC_SWVer_4Updt;
    }

    if(!posUx)
    {
        return FALSE;
    }

    //parse the version
    *posBin = '\0';
    posUx += 2;
    char *posVxx = strstr(posUx, "V");
    if(!posVxx)
    {
        return FALSE;
    }
    posVxx += 1;
    UINT nVer = (UINT)atoi(posVxx);
    if((nVer < 1) || (nVer > 9999))
    {
        return FALSE;
    }
    float fVV = (float)nVer / 100.0;
    //judge if input ver matches the file name
    if((fSetVer > 0.009) && (fabs(fSetVer - fVV) > 0.0009))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL Handle_Rect_Upgrade(FCGI_ROUGH_DATA* pFCGI_Rough, A_BLOB_UPDT_INFO_4CCU* pCmd)
{
    s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_NOT;

    TRACEX("-----------Start Create_2BinFile_FromBody PM Upgrade ----------\n");

    //if(pFCGI_Rough->nTempFileLen <= 0)
    BOOL bSuccess = FALSE;
    if(!Create_2BinFile_FromBody())
    {
        Make_Web_OutPut(104, "The data file is NOT received yet!", NULL,  NULL);
        s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
        return FALSE;                  
    }

    TRACEX("----------- Create_2BinFile_FromBody ok  PM Upgrade ----------\n");

    //for testing purpose only
    // char szDDDD[128];
    // snprintf(szDDDD, 128, "The sent raw file name is=>[%s]!!!", pFCGI_Rough->stMultiFormData.szRawFileName_FromClient);
    // Make_Web_OutPut(104, szDDDD, NULL,  NULL);
    // s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
    // goto clean;

    /*//John 2023-1-18 Disable For Test
     if(!IsBinFile_CheckOK(pFCGI_Rough, pCmd))
    {
        Make_Web_OutPut(104, "Wrong file or version, please check!", "File Check error!", NULL);
        s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
        goto clean;           
    }*/

#define RECT_FIRMWARE_WEB_IMPORT_FILENAME   "/opt/appdata/data/rect_firmware.import"
    //now downloaded done
    TRACEX("-----------Now downloaded rect-firmware file DONE!!!----------\n");
    rename(WEB_TEMPFILE_DOWNLOADED_RAW, RECT_FIRMWARE_WEB_IMPORT_FILENAME);

//     //extract the pkg
//     char szCmd[128];
// #define RECT_BIN_TEMPPATH "/opt/appdata/data/rect_bins/"
//     snprintf(szCmd, 128, "mkdir -p %s;unzip %s -d %s", RECT_BIN_TEMPPATH,
//         RECT_FIRMWARE_WEB_IMPORT_FILENAME, RECT_BIN_TEMPPATH);
//     _SYSTEM(szCmd);
//     TRACEX("sssssssssssss--->[%d]\n", pCmd->emUpdtType);
//     if((pCmd->emUpdtType == UPDT_TP_DCDC) || (pCmd->emUpdtType == UPDT_TP_BOTH))
//     {
//         if(access(RECT_BIN_TEMPPATH"dc.bin", R_OK) != 0) 
//         {
//             TRACEX("0000000000000\n");
//             Make_Web_OutPut(104, "The dc.bin file is NOT found!", NULL,  NULL);
//             goto clean;                  
//         }
//         else
//         {            
//             sprintf(pCmd->szDCDCFilePath, RECT_BIN_TEMPPATH"dc.bin");
//             TRACEX("1111111[%s]\n",pCmd->szDCDCFilePath);
//         }        
//     }
    
//     if((pCmd->emUpdtType == UPDT_TP_PFC) || (pCmd->emUpdtType == UPDT_TP_BOTH))
//     {        
//         if(access(RECT_BIN_TEMPPATH"pfc.bin", R_OK) != 0)
//         {
//             TRACEX("33333333333--->[%d]\n", pCmd->emUpdtType);
//             Make_Web_OutPut(104, "The pfc.bin file is NOT found!", NULL,  NULL);
//             goto clean;                 
//         }
//         else
//         {
//             sprintf(pCmd->szPFCFilePath, RECT_BIN_TEMPPATH"pfc.bin");
//             TRACEX("4444444--->[%d]\n", pCmd->emUpdtType);
//         }     
//     }
    TRACE("pCmd->szDCDCFilePath=[%s]\n",pCmd->szDCDCFilePath);
    TRACED("pFCGI_Rough->stMultiFormData.szRawFileName_FromClient=[%s]\n",pFCGI_Rough->stMultiFormData.szRawFileName_FromClient);
    UINT nDevID = 200;
    UINT nSigID = 151;//Default For CCU Upgrade
    s_emBMSorModel_Status = UPDT_MODEL_STATUS_FLAG;
    // 新增文件名检查逻辑
    // if (pCmd->emUpdtType == UPDT_TP_IMSU)
    // {
    //     if (strcmp(pFCGI_Rough->stMultiFormData.szRawFileName_FromClient, "IMSU_D_APP.bin") != 0)
    //     {
    //         Make_Web_OutPut(104, "Invalid file name! Expected 'IMSU_D_APP.bin'.", NULL, NULL);
    //         s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
    //         return FALSE;
    //     }
    
    if(pCmd->emUpdtType == UPDT_TP_IMSU)
    {
        sprintf(pCmd->szIMSUFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_PFC)
    {
        sprintf(pCmd->szPFCFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_DCDC)
    {
        sprintf(pCmd->szDCDCFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_ABU_U1)
    {
        sprintf(pCmd->szPFCFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);     
    }
    else if(pCmd->emUpdtType == UPDT_TP_ABU_U2)
    {
        sprintf(pCmd->szDCDCFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_EU2_1)
    {
        sprintf(pCmd->szIMEU2FilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_EU2_2)
    {
        sprintf(pCmd->szIMEU2FilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_MPPT_U1)
    {
        sprintf(pCmd->szPFCFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_MPPT_U2)
    {
        sprintf(pCmd->szDCDCFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    }
    else if(pCmd->emUpdtType == UPDT_TP_BMS)
    {
        sprintf(pCmd->szBMSFilePath, RECT_FIRMWARE_WEB_IMPORT_FILENAME);
        nDevID = 500;
        nSigID = 500;
        s_emBMSorModel_Status = RECT_UPDT_START_STATUS_NOT;
    }
    else
    {
        Make_Web_OutPut(104, "Sorry, only support DCDC or PFC mode!", NULL,  NULL);
        s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
        goto clean;
    }
    
    
    TRACEX("----check done! now we send the updt-rect cmd to main app for executing----\n");
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, nDevID, nSigID, (void*)pCmd, sizeof(A_BLOB_UPDT_INFO_4CCU));
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute upgrade Rect firmware error!", "Execute error!", NULL);
        s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
        goto clean;   
    }
    else
    {
        s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_OK;
    }   

    bSuccess = TRUE;

clean:    
    unlink(RECT_FIRMWARE_WEB_IMPORT_FILENAME);
    // sprintf(szCmd, "rm -rf "RECT_BIN_TEMPPATH);
    // _SYSTEM(szCmd);   
    Force_Update_ParamVals();
    Sleep_Ex(2000); //in case the get of status is too slow
    Force_Update_ParamVals();

    return bSuccess;
}

static BOOL Handle_IMMU2_Upgrade(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    //if(pFCGI_Rough->nTempFileLen <= 0)

    if(!Create_2BinFile_FromBody())
    {
        Make_Web_OutPut(104, "The data file is NOT received yet!", NULL,  NULL);
        return FALSE;                  
    }
    //now downloaded done
    TRACEX("-----------Now downloaded firmware file DONE!!!----------\n");
    rename(WEB_TEMPFILE_DOWNLOADED_RAW, IMMU2_FIRMWARE_WEB_IMPORT_FILENAME);
    
    VAR_VALUE varV;
    varV.emValue = 0;  
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 1, 18, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute upgrade IMMU2 firmware error!", "Execute error!", NULL);
        unlink(IMMU2_FIRMWARE_WEB_IMPORT_FILENAME);
        return FALSE;     
    }

    unlink(IMMU2_FIRMWARE_WEB_IMPORT_FILENAME);

    return TRUE;
}

static BOOL Handle_OneClick_Upgrade(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    if(!Create_2BinFile_FromBody())
    {
        Make_Web_OutPut(104, "The data file is NOT received yet!", NULL,  NULL);
        return FALSE;                  
    }
    //now downloaded done
    TRACEX("-----------Now downloaded firmware file DONE!!!----------\n");
    rename(WEB_TEMPFILE_DOWNLOADED_RAW, ONECLICK_FIRMWARE_WEB_IMPORT_FILENAME);

    pFCGI_Rough->tm_OneClickEnd = _NOW_;
    VAR_VALUE varV;
    varV.emValue = 0;  
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 1, 25, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute one click firmware error!", "Execute error!", NULL);
        unlink(ONECLICK_FIRMWARE_WEB_IMPORT_FILENAME);
        s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FAILED;
        return FALSE;     
    }
    else
    {
        s_emRectFirmwUpgr_StartSTatus = UPDT_ONECLICK_STATUS_FLAG;
    }

    unlink(ONECLICK_FIRMWARE_WEB_IMPORT_FILENAME);

    return TRUE;
}

static BOOL prepare_controller_version_data(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "immu2_sw_ver", pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_SWVer);
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "immu2_hw_ver", pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_HWVer);
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "lccu_ver", pFCGI_Rough->chargerDataRepo.versionInfos.szIMSU_D_Vers);
    return TRUE;
}

static BOOL prepare_version(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int total = 0;
    char szVVV[32];
    WEB_PARAM_VALUE     *pParamVal;
    char szType[32]; //升级类型+升级设备地址
    memset(szType, 0, 32);
    //cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", 5);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", 1);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", 1);
    cJSON* pJRecList = cJSON_CreateArray();

    cJSON* pJ_ARec = cJSON_CreateObject();
    cJSON_AddNumberToObject(pJ_ARec, "idx", total);
    snprintf(szType, 32, "%d", 100);
    cJSON_AddStringToObject(pJ_ARec, "type", szType);
    cJSON_AddStringToObject(pJ_ARec, "name", LoadLangText_ById(6677));
    cJSON_AddStringToObject(pJ_ARec, "version", "----");
    cJSON_AddItemToArray(pJRecList, pJ_ARec);
    total++;
    
    cJSON* pJ_ARec0 = cJSON_CreateObject();
    cJSON_AddNumberToObject(pJ_ARec0, "idx", total);
    snprintf(szType, 32, "%d", 101);
    cJSON_AddStringToObject(pJ_ARec0, "type", szType);
    cJSON_AddStringToObject(pJ_ARec0, "name", LoadLangText_ById(7102));
    cJSON_AddStringToObject(pJ_ARec0, "version", pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_SWVer);
    cJSON_AddItemToArray(pJRecList, pJ_ARec0);
    total++;
    
    cJSON* pJ_ARec1 = cJSON_CreateObject();
    cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
    snprintf(szType, 32, "%d", UPDT_TP_IMSU);
    cJSON_AddStringToObject(pJ_ARec1, "type", szType);
    cJSON_AddStringToObject(pJ_ARec1, "name", LoadLangText_ById(319));
    cJSON_AddStringToObject(pJ_ARec1, "version", pFCGI_Rough->chargerDataRepo.versionInfos.szIMSU_D_Vers);
    cJSON_AddItemToArray(pJRecList, pJ_ARec1);
    total++;
    
    A_BLOB_SINGLE_RECT_INFO allsingleRectData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
    {
        TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
        Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
        return TRUE;
    }
    
    A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 200, 200, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS), (void*)&blobCCUSett))
    {
        printf("----ERRRRRR: can not get CCU Set----\n");
        return;
    }
    int PCS_GrpNum[4] = {0};
    PCS_GrpNum[0] = blobCCUSett.ccuSettings.byNumGrp_1; //PCS
    PCS_GrpNum[1] = blobCCUSett.ccuSettings.byNumGrp_2;
    PCS_GrpNum[2] = blobCCUSett.ccuSettings.byNumGrp_3;
    PCS_GrpNum[3] = blobCCUSett.ccuSettings.byNumGrp_4;
    
    int MPPT_GrpNum[4] = {0};
    MPPT_GrpNum[0] = blobCCUSett.ccuSettings.byNumGrp_5;//MPPT
    MPPT_GrpNum[1] = blobCCUSett.ccuSettings.byNumGrp_6;
    MPPT_GrpNum[2] = blobCCUSett.ccuSettings.byNumGrp_7;
    MPPT_GrpNum[3] = blobCCUSett.ccuSettings.byNumGrp_8;

    int n = 0;
    int whichNum = 0, i = 0;
    char szName[32] = {0};
    char szSingleRectName[64] = "\0";
    STU_SIGNLE_RECT_DATA singleRectData;
    for(n = TYPE_SIG_RECT_ACDC_GRP1; n <= TYPE_SIG_RECT_ACDC_GRP4; n++)
    {
        whichNum = 0;
        for(i = 0; i < PCS_GrpNum[n]; ++i)
        {
            singleRectData = allsingleRectData.arrRect[n][i];
            
            snprintf(szSingleRectName, 64, "%.2f", singleRectData.fRect_PFCVer);
            memset(szVVV, 0, 32);
            snprintf(szVVV, 32, "%s", szSingleRectName);

            snprintf(szName, 32, "%d-PCS(U1)%s", ++whichNum, LoadLangText_ById(125));  

            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d", UPDT_TP_PFC);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", szName);
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
            total++;
        }
        whichNum = 0;
        for(i = 0; i <PCS_GrpNum[n]; ++i)
        {
            singleRectData = allsingleRectData.arrRect[n][i];
            
            snprintf(szSingleRectName, 64, "%.2f",singleRectData.fRect_DCDCVer);
            memset(szVVV, 0, 32);
            snprintf(szVVV, 32, "%s", szSingleRectName);
            snprintf(szName, 32, "%d-PCS(U2)%s", ++whichNum, LoadLangText_ById(125));  

            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d", UPDT_TP_DCDC);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", szName);
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
            total++;
        }        
    }
    
    for(n = TYPE_SIG_RECT_MPPT_GRP1; n <= TYPE_SIG_RECT_MPPT_GRP4; n++)
    {
        whichNum = 0;
        for(i = 0; i < MPPT_GrpNum[n - TYPE_SIG_RECT_MPPT_GRP1]; ++i)
        {
            singleRectData = allsingleRectData.arrRect[n][i];
            
            snprintf(szSingleRectName, 64, "%.2f", singleRectData.fRect_PFCVer);
            memset(szVVV, 0, 32);
            snprintf(szVVV, 32, "%s", szSingleRectName);
            snprintf(szName, 32, "%d-MPPT(U1)%s", ++whichNum, LoadLangText_ById(125));  

            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d", UPDT_TP_MPPT_U1);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", szName);
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
            total++;
        }
        whichNum = 0;
        for(i = 0; i < MPPT_GrpNum[n - TYPE_SIG_RECT_MPPT_GRP1]; ++i)
        {
            singleRectData = allsingleRectData.arrRect[n][i];
            
            snprintf(szSingleRectName, 64, "%.2f", singleRectData.fRect_DCDCVer);
            memset(szVVV, 0, 32);
            snprintf(szVVV, 32, "%s", szSingleRectName);
            snprintf(szName, 32, "%d-MPPT(U2)%s", ++whichNum, LoadLangText_ById(125));  

            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d", UPDT_TP_MPPT_U2);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", szName);
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
            total++;
        }

    }
    
    if(allsingleRectData.nOnlineNum[TYPE_SIG_RECT_STS] > 0)
    {
            singleRectData = allsingleRectData.arrRect[TYPE_SIG_RECT_STS][0];
            
            snprintf(szSingleRectName, 64, "%.2f", singleRectData.fRect_PFCVer);
            memset(szVVV, 0, 32);
            snprintf(szVVV, 32, "%s", szSingleRectName);
            snprintf(szName, 32, "ABU(U1)%s", LoadLangText_ById(125));  

            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d", UPDT_TP_ABU_U1);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", szName);
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
            total++;

            snprintf(szSingleRectName, 64, "%.2f", singleRectData.fRect_DCDCVer);
            memset(szVVV, 0, 32);
            snprintf(szVVV, 32, "%s", szSingleRectName);
            snprintf(szName, 32, "ABU(U2)%s", LoadLangText_ById(125));  

            cJSON* pJ_ARec2 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec2, "idx", total);
            snprintf(szType, 32, "%d", UPDT_TP_ABU_U2);
            cJSON_AddStringToObject(pJ_ARec2, "type", szType);
            cJSON_AddStringToObject(pJ_ARec2, "name", szName);
            cJSON_AddStringToObject(pJ_ARec2, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec2);
            total++;
    }

    for(n = 0; n < AIR_NUM; n++)
    {
        char *szAirVersion = NULL;
        memset(szVVV, 0, 32);
        pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 951 + n, 448);
        if(pParamVal == NULL)
        {
            Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
            return FALSE;  
        }
        TRACED("Air Conditioner emValue Type:%d", pParamVal->currValue.emValue);
        AIR_TYPE airType = (AIR_TYPE)(pParamVal->currValue.emValue);
        switch (airType)
        {
        case INFYPOWER_AIR:
            pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 951 + n, 1562);
            if(pParamVal == NULL)
            {
                Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
                return FALSE;  
            }

            szAirVersion = (char*)pParamVal->currValue.blobValue;
            if(szAirVersion != NULL && strlen(szAirVersion) > 0)
            {
                snprintf(szVVV, 32, "%s", szAirVersion);
            }
            else
            {
                snprintf(szVVV, 32, "--");
            }
            
            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, 43 + n);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", LoadLangText_ById(4014 + n));
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
            total++;
            break;
        case IMEU3_AIR:
            pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 951 + n, 3500);
            if(pParamVal == NULL)
            {
                Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
                return FALSE;  
            }

            szAirVersion = (char*)pParamVal->currValue.blobValue;
            if(szAirVersion != NULL && strlen(szAirVersion) > 0)
            {
                snprintf(szVVV, 32, "%s", szAirVersion);
            }
            else 
            {
                snprintf(szVVV, 32, "--");
            }
            
            cJSON* pJ_ARec2 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec2, "idx", total);
            snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, 43 + n);
            cJSON_AddStringToObject(pJ_ARec2, "type", szType);
            cJSON_AddStringToObject(pJ_ARec2, "name", LoadLangText_ById(4014 + n));
            cJSON_AddStringToObject(pJ_ARec2, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec2);
            total++;
            break;
        case TONGFEI_AIR:
        case AIR_NOT_INSTALLED:
            TRACED("Air Conditioner szAir_Cond_Ver Type NULL");
            break;
        default:
            break;
        }
    }
    
    int STBattNum = 0;
    pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 500, 502);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get battery number value!", "Initialize error!", NULL);
        return FALSE;  
    }
    STBattNum = pParamVal->currValue.ulValue;
    if(STBattNum > MAX_SINGLE_STBATT_NUM)
    {
        STBattNum = MAX_SINGLE_STBATT_NUM;
    }

    //7、1# 2# 3# 4# BMS Version STBattery Version
    for(n = 0; n < STBattNum; n++)
    {
        A_BLOB_SINGLE_STBATT_RUNINFO *Air_pVerInfo;
        memset(szVVV, 0, 32);
        pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 501 + n, 551);
        if(pParamVal == NULL)
        {
            Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
            return FALSE;
        }
        Air_pVerInfo = (A_BLOB_SINGLE_STBATT_RUNINFO*)pParamVal->currValue.blobValue;
        if(Air_pVerInfo->fVersion > 0)
        {
            snprintf(szVVV, 32, "%.3f", Air_pVerInfo->fVersion);
        }
        else
        {
            snprintf(szVVV, 32, "--");
        }

        cJSON* pJ_ARec1 = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
        snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, 50 + n);
        cJSON_AddStringToObject(pJ_ARec1, "type", szType);
        cJSON_AddStringToObject(pJ_ARec1, "name", LoadLangText_ById(4000 + n));
        cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
        cJSON_AddItemToArray(pJRecList, pJ_ARec1);
        total++;
    }

    int BSUnum = 0;
    if(pFCGI_Rough->emSystemType == SYSTYPE_ESS_AIR)
    {
        BSUnum = STBattNum;
    }
    else
    {
        pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 500, 1944);
        if(pParamVal == NULL)
        {
            Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
            return FALSE;  
        }
        BSUnum = MIN(10, STBattNum * pParamVal->currValue.ulValue);
    }
    
    for(n = 0; n < BSUnum; n++)
    {
        pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 500, 868 + n);
        if(pParamVal == NULL)
        {
            Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
            return FALSE;  
        }
        snprintf(szVVV, 32, "%s", pParamVal->currValue.blobValue);
        snprintf(szName, 32, "%d-%s", n + 1, LoadLangText_ById(4016));  

        cJSON* pJ_ARec1 = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
        snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, n);
        cJSON_AddStringToObject(pJ_ARec1, "type", szType);
        cJSON_AddStringToObject(pJ_ARec1, "name", szName);
        cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
        cJSON_AddItemToArray(pJRecList, pJ_ARec1);
        total++;
    }
    
    //电源柜 IMEU3/IMEU2 类型
    pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 611, 451);
    if(pParamVal == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get IMEU type value!", "Initialize error!", NULL);
        return FALSE;  
    }
    TRACED("POWER IMEU emValue Type:%d", pParamVal->currValue.emValue);
    IMEU_TYPE imeuType_Pow = (IMEU_TYPE)(pParamVal->currValue.emValue);
    char *szIMEU3_Version = NULL;
    switch (imeuType_Pow)
    {
    case IMEU_TYPE_IMEU3:
        memset(szVVV, 0, 32);
        pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 611, 1669);
        if(pParamVal == NULL)
        {
            Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
            return FALSE;  
        }
        
        szIMEU3_Version = (char*)pParamVal->currValue.blobValue;
        if(szIMEU3_Version != NULL && strlen(szIMEU3_Version) > 0)
        {
            snprintf(szVVV, 32, "%s", szIMEU3_Version);
        }
        else
        {
            snprintf(szVVV, 32, "--");
        }

        cJSON* pJ_ARec1 = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
        snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, 30);
        cJSON_AddStringToObject(pJ_ARec1, "type", szType);
        cJSON_AddStringToObject(pJ_ARec1, "name", LoadLangText_ById(4013));
        cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
        cJSON_AddItemToArray(pJRecList, pJ_ARec1);
        total++;
        break;
    case IMEU_TYPE_IMEU2:
    case IMEU_TYPE_NOT_INSTALLED:
        TRACED("Power szIMEU2_Ver Type NULL");
        break;
    }

    for(n = 0; n < STBattNum; n++)
    {
        A_BLOB_BATTCAB_IMEU2_V20 *IMU_pVerInfo;
        memset(szVVV, 0, 32);
        pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 601 + n, 450);
        if(pParamVal == NULL)
        {
            Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
            return FALSE;  
        }
        
        IMEU_TYPE imeuType_Bat = (IMEU_TYPE)(pParamVal->currValue.emValue);
        switch (imeuType_Bat)
        {
        case IMEU_TYPE_IMEU3://IMEU3
            pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 601 + n, 1633);
            if(pParamVal == NULL)
            {
                Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
                return FALSE;  
            }
            szIMEU3_Version = (char*)pParamVal->currValue.blobValue;
            if(szIMEU3_Version != NULL && strlen(szIMEU3_Version) > 0)
            {
                snprintf(szVVV, 32, "%s", szIMEU3_Version);
            }
            else
            {
                snprintf(szVVV, 32, "--");
            }

            cJSON* pJ_ARec1 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec1, "idx", total);
            snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, 31);
            cJSON_AddStringToObject(pJ_ARec1, "type", szType);
            cJSON_AddStringToObject(pJ_ARec1, "name", LoadLangText_ById(4008 + n));
            cJSON_AddStringToObject(pJ_ARec1, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec1);
           
            total++;
            break;
        case IMEU_TYPE_IMEU2://IMEU2
            pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 601 + n, 450);
            if(pParamVal == NULL)
            {
                Make_Web_OutPut(104, "Can NOT get back about config value!", "Initialize error!", NULL);
                return FALSE;
            }
            IMU_pVerInfo = (A_BLOB_BATTCAB_IMEU2_V20*)pParamVal->currValue.blobValue;
            if(strlen(IMU_pVerInfo->szSoftwareVer) > 1)
            {
                snprintf(szVVV, 32, "%s", IMU_pVerInfo->szSoftwareVer);
            }
            else
            {
                snprintf(szVVV, 32, "--");
            }

            cJSON* pJ_ARec2 = cJSON_CreateObject();
            cJSON_AddNumberToObject(pJ_ARec2, "idx", total);
            snprintf(szType, 32, "%d_%d", UPDT_TP_BMS, 40 + n);
            cJSON_AddStringToObject(pJ_ARec2, "type", szType);
            cJSON_AddStringToObject(pJ_ARec2, "name", LoadLangText_ById(4004 + n));
            cJSON_AddStringToObject(pJ_ARec2, "version", szVVV);
            cJSON_AddItemToArray(pJRecList, pJ_ARec2);
            total++;    
            
            break;
        case IMEU_TYPE_NOT_INSTALLED://Not Installed
            break;
        default:
            break;
        }
    }
    
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJRecList);
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", total);
    return TRUE;
}


static BOOL prepare_controller_histversion_data(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON* pJHisVerListArr = cJSON_CreateArray();
    int n;
    int nTotal = pFCGI_Rough->chargerDataRepo.nIMMU2HisVerTotal;
    IMMU2_HISVERITEM* pHisVer = pFCGI_Rough->chargerDataRepo.hisIMMU2VerList;
    char szTime[32];
    for(n = 0; n < nTotal; n++, pHisVer++)
    {
        cJSON* pJItem = cJSON_CreateObject();
        cJSON_AddStringToObject(pJItem, "rdate", 
            TimeToString(pHisVer->tmReleaseDate, SQLITE_DATE_FMT, szTime, 32));
        cJSON_AddStringToObject(pJItem, "ver", pHisVer->szCurrVer);
        cJSON_AddStringToObject(pJItem, "detail", pHisVer->szReleaseNotes);
        cJSON_AddItemToArray(pJHisVerListArr, pJItem);
    }
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "hisver_list", pJHisVerListArr);
    return TRUE;
}

static BOOL prepare_rectifier_version_data(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int n = 0;
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "rect_dc_ver", pFCGI_Rough->chargerDataRepo.versionInfos.szRect_Ver_DC);
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "rect_pfc_ver", pFCGI_Rough->chargerDataRepo.versionInfos.szRect_Ver_PFC);
    WEB_PARAM_VALUE* pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 250, 150);
    int nTotal = (pWebParaVal == NULL) ? 0 : pWebParaVal->currValue.ulValue;
    if(nTotal > 0)
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "online_total", nTotal);
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "run_total", nTotal); //will implment later
        //int n;
        //int nRunTotal = 0;
        // for(n = 0; n < nTotal; n++)
        // {
        //     pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 250, 150 + n));
        //     if(pWebParaVal != NULL)
        //     {

        //     }
        // }
    }
    else
    {
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "online_total", 0);
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "run_total", 0);
    }
    A_BLOB_ES_RUN_DATA blobRunData;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 80, sizeof(A_BLOB_ES_RUN_DATA), (void*)&blobRunData))
    {
        TRACEX("----ERRRRRR: cand not get ES RunData----\n");
        return TRUE;
    }
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "title_iDevAddr", LoadLangText_ById(2396));
    cJSON* pJArrDevs = cJSON_CreateArray();
    if(SYSTYPE_ESS_AIR == pFCGI_Rough->emSystemType)
    {
    for (n = 0; n < MAX_SINGLE_STBATT_NUM; ++n)
    {
        char szName[32] = {0};
        snprintf(szName, 32, "%d#BMU", n + 1);

        cJSON* pJBMU = cJSON_CreateObject();
        cJSON_AddStringToObject(pJBMU, "name", szName);
        cJSON_AddNumberToObject(pJBMU, "val", 50 + n);
        BOOL bDisabled = FALSE;
        if(STBATT_RUNSTAT_NOT_INSTALLED == blobRunData.emSTBatt_WorkStatu[n]    ||
          // STBATT_RUNSTAT_COMM_FAIL == blobRunData.emSTBatt_WorkStatu[n]        ||
            STBATT_RUNSTAT_CHARGE == blobRunData.emSTBatt_WorkStatu[n]          ||
           STBATT_RUNSTAT_DISCHARGE == blobRunData.emSTBatt_WorkStatu[n])
        {
            bDisabled = TRUE;
        }
        cJSON_AddBoolToObject(pJBMU, "disabled", bDisabled);
        A_BLOB_SINGLE_STBATT_RUNINFO stBattDataInfo;
        memset(&stBattDataInfo, 0, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
        float fVersion = 0;
        if (Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 501+n, 551, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO), (void*)&stBattDataInfo))
        {
            fVersion = stBattDataInfo.fVersion;
        }
        
        char szVer[32] = {0};
        if ((int)fVersion > 0)
        {
            snprintf(szVer, 32, "V%03dP%03d", (int)fVersion, ((int)(fVersion * 1000)) % 1000);
        }    
        cJSON_AddStringToObject(pJBMU, "ver", szVer);
        cJSON_AddItemToArray(pJArrDevs, pJBMU);

        snprintf(szName, 32, "%d#BSU", n + 1);
        cJSON* pJBSU = cJSON_CreateObject();
        cJSON_AddStringToObject(pJBSU, "name", szName);
        cJSON_AddNumberToObject(pJBSU, "val", n);
        cJSON_AddBoolToObject(pJBSU, "disabled", bDisabled);
        cJSON_AddStringToObject(pJBSU, "ver", "");
        cJSON_AddItemToArray(pJArrDevs, pJBSU);

            snprintf(szName, 32, "%d#IMEU2", n + 1);
            cJSON* pJIMEU2 = cJSON_CreateObject();
            cJSON_AddStringToObject(pJIMEU2, "name", szName);
            cJSON_AddNumberToObject(pJIMEU2, "val", 40 + n);
            cJSON_AddBoolToObject(pJIMEU2, "disabled", bDisabled);
            cJSON_AddStringToObject(pJIMEU2, "ver", "");
            cJSON_AddItemToArray(pJArrDevs, pJIMEU2);


        if (USER_LEVEL_ROOT == pFCGI_Rough->pCurrSess->emUserLvel)
        {
            snprintf(szName, 32, "%d#BMU_AUX", n + 1);
            cJSON* pJBMUS = cJSON_CreateObject();
            cJSON_AddStringToObject(pJBMUS, "name", szName);
            cJSON_AddNumberToObject(pJBMUS, "val", 200 + n);
            cJSON_AddBoolToObject(pJBMUS, "disabled", bDisabled);
            cJSON_AddStringToObject(pJBMUS, "ver", "");
            
            cJSON_AddItemToArray(pJArrDevs, pJBMUS);
        }
        }
    }
    else
    {
        WEB_PARAM_VALUE* pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_SETTING, 1, 101);
        for (n = 0; n < MAX_SINGLE_STBATT_NUM - 2; ++n)
        {
            char szName[32] = {0};
            snprintf(szName, 32, "%d#BMU", n + 1);
            cJSON* pJBMU = cJSON_CreateObject();
            cJSON_AddStringToObject(pJBMU, "name", szName);
            cJSON_AddNumberToObject(pJBMU, "val", 50 + n);
            BOOL bDisabled = FALSE;
            if(STBATT_RUNSTAT_NOT_INSTALLED == blobRunData.emSTBatt_WorkStatu[n]    ||
                STBATT_RUNSTAT_CHARGE == blobRunData.emSTBatt_WorkStatu[n]          ||
            STBATT_RUNSTAT_DISCHARGE == blobRunData.emSTBatt_WorkStatu[n])
            {
                bDisabled = TRUE;
            }
            cJSON_AddBoolToObject(pJBMU, "disabled", bDisabled);
            A_BLOB_SINGLE_STBATT_RUNINFO stBattDataInfo;
            memset(&stBattDataInfo, 0, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO));
            float fVersion = 0;
            if (Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 501 + n, 551, sizeof(A_BLOB_SINGLE_STBATT_RUNINFO), (void*)&stBattDataInfo))
            {
                fVersion = stBattDataInfo.fVersion;
            }
        
            char szVer[32] = {0};
            if ((int)fVersion > 0)
            {
                snprintf(szVer, 32, "V%03dP%03d", (int)fVersion, ((int)(fVersion * 1000)) % 1000);
            }    
            cJSON_AddStringToObject(pJBMU, "ver", szVer);
            cJSON_AddItemToArray(pJArrDevs, pJBMU);

            int i = 0;

            int iBSUNum = MAX_BSU_NUM;   

            if(LIQUID_COOLING_BACKUP == (ESS_DEVICE_TYPE)pWebParaVal->currValue.ulValue)
            {
                iBSUNum = 4;//液冷备电 一组电池只有4个BSU
            }

            for(i = 0; i < iBSUNum; i++)
            {
                snprintf(szName, 32, "%d#BSU-%d", n + 1, i + 1);
                cJSON* pJBSU = cJSON_CreateObject();
                cJSON_AddStringToObject(pJBSU, "name", szName);
                cJSON_AddNumberToObject(pJBSU, "val", i + n * iBSUNum);
                cJSON_AddBoolToObject(pJBSU, "disabled", bDisabled);
                cJSON_AddStringToObject(pJBSU, "ver", "");
                cJSON_AddItemToArray(pJArrDevs, pJBSU);
            }
            
            snprintf(szName, 32, "%d#IMEU3_Power", n + 1);
            cJSON* pJIMEU3_1 = cJSON_CreateObject();
            cJSON_AddStringToObject(pJIMEU3_1, "name", szName);
            cJSON_AddNumberToObject(pJIMEU3_1, "val", 30 + n*MAX_IMEU3_NUM);
            cJSON_AddBoolToObject(pJIMEU3_1, "disabled", bDisabled);
            cJSON_AddStringToObject(pJIMEU3_1, "ver", "");
            cJSON_AddItemToArray(pJArrDevs, pJIMEU3_1);

            snprintf(szName, 32, "%d#IMEU3_Battery", n + 1);
            cJSON* pJIMEU3_2 = cJSON_CreateObject();
            cJSON_AddStringToObject(pJIMEU3_2, "name", szName);
            cJSON_AddNumberToObject(pJIMEU3_2, "val", 31 + n*MAX_IMEU3_NUM);
            cJSON_AddBoolToObject(pJIMEU3_2, "disabled", bDisabled);
            cJSON_AddStringToObject(pJIMEU3_2, "ver", "");
            cJSON_AddItemToArray(pJArrDevs, pJIMEU3_2);

            if(LIQUID_COOLING_BACKUP == (ESS_DEVICE_TYPE)pWebParaVal->currValue.ulValue)
            {
                snprintf(szName, 32, " ");   //用来填充网页升级布局， 把两个电池组OTA升级按钮分开
                cJSON* ptianchong = cJSON_CreateObject();
                cJSON_AddStringToObject(ptianchong, "name", szName);
                cJSON_AddNumberToObject(ptianchong, "val", 1000 + n);
                cJSON_AddBoolToObject(ptianchong, "disabled", TRUE);
                cJSON_AddStringToObject(ptianchong, "ver", "");
                cJSON_AddItemToArray(pJArrDevs, ptianchong);
            }

        }

        if(LIQUID_COOLING_BACKUP == (ESS_DEVICE_TYPE)pWebParaVal->currValue.ulValue)
        {
            int j = 0;
            for(j = 0; j < 2; ++j)
            {
                char* sznameAir[32] = {0};
                snprintf(sznameAir, 32, "IMEU3_Air%d", j + 1);
                cJSON* pJIMEU3_AIR = cJSON_CreateObject();
                cJSON_AddStringToObject(pJIMEU3_AIR, "name", sznameAir);
                cJSON_AddNumberToObject(pJIMEU3_AIR, "val", 43 + j);
                cJSON_AddBoolToObject(pJIMEU3_AIR, "disabled", FALSE);
                cJSON_AddStringToObject(pJIMEU3_AIR, "ver", "");
                cJSON_AddItemToArray(pJArrDevs, pJIMEU3_AIR); 
            }
        }
        else if(LIQUID_COOLING == (ESS_DEVICE_TYPE)pWebParaVal->currValue.ulValue)
        {
                char* sznameAir[32] = {0};
                snprintf(sznameAir, 32, "IMEU3_Air");
                cJSON* pJIMEU3_AIR = cJSON_CreateObject();
                cJSON_AddStringToObject(pJIMEU3_AIR, "name", sznameAir);
                cJSON_AddNumberToObject(pJIMEU3_AIR, "val", 43);
                cJSON_AddBoolToObject(pJIMEU3_AIR, "disabled", FALSE);
                cJSON_AddStringToObject(pJIMEU3_AIR, "ver", "");
                cJSON_AddItemToArray(pJArrDevs, pJIMEU3_AIR);             
        }

        if(LIQUID_COOLING_ADD_STS == (ESS_DEVICE_TYPE)pWebParaVal->currValue.ulValue)
        {
            // ADD STS CABINET IMEU3
            cJSON* pJIMEU3_STS = cJSON_CreateObject();
            cJSON_AddStringToObject(pJIMEU3_STS, "name", "IMEU3_STSCabinet");
            cJSON_AddNumberToObject(pJIMEU3_STS, "val", 45);
            cJSON_AddBoolToObject(pJIMEU3_STS, "disabled", FALSE);
            cJSON_AddStringToObject(pJIMEU3_STS, "ver", "");
            cJSON_AddItemToArray(pJArrDevs, pJIMEU3_STS);
        }
        
    }
    
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "list_iDevAddr", pJArrDevs);
    
    return TRUE;
}

static BOOL prepare_IMMU2_upgrage_process_data(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "File is being transferred, please wait...");
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", 100);
    return TRUE;
}

static BOOL prepare_upgrage_process_data(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    int nProcess = 0;
    int nProcess2 = 0, nProcess3 = 0, ModStatus = 1;
    cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "Upgrading...");
    cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", nProcess);

    WEB_PARAM_VALUE* pWebParaVal;


    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, 82)) == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get the status, parater 82!", NULL, NULL);
        return FALSE;  
    }
    nProcess = pWebParaVal->currValue.lValue;

    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, 85)) == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get upgrade status,  parater 85!", NULL, NULL);
        return FALSE;  
    }
    nProcess2 = pWebParaVal->currValue.lValue;
    if(s_emBMSorModel_Status == UPDT_MODEL_STATUS_FLAG)
    {
        nProcess = nProcess2;
    }

    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, 124)) == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get upgrade status,  parater 124!", NULL, NULL);
        return FALSE;  
    }
    ModStatus = pWebParaVal->currValue.emValue;

    if((pWebParaVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, 200, 123)) == NULL)
    {
        Make_Web_OutPut(104, "Can NOT get upgrade status, 123!", NULL, NULL);
        return FALSE;  
    }
    nProcess3 = pWebParaVal->currValue.lValue;

    if(s_emRectFirmwUpgr_StartSTatus == RECT_UPDT_START_STATUS_FAILED)
    {
        nProcess = 255;
    }
    else if(s_emRectFirmwUpgr_StartSTatus == UPDT_ONECLICK_STATUS_FLAG)
    {
        if(nProcess != 256)
        {
            if((nProcess > 100) &&  (nProcess2 > 100))
            {
                if(ModStatus == 1)
                {
                    pFCGI_Rough->tm_OneClickEnd = _NOW_;
                }
                if((ABS(_NOW_ - pFCGI_Rough->tm_OneClickEnd) > 80) && (ModStatus != 1))
                {
                    nProcess = 256; //one-click upgrade finished
                }
                else
                {
                    nProcess = nProcess3; //still in process
                }
            }
            else
            {
                pFCGI_Rough->tm_OneClickEnd = _NOW_;
                nProcess = MIN(nProcess, nProcess2);
            }           
        }
    }
    else if(s_emRectFirmwUpgr_StartSTatus != RECT_UPDT_START_STATUS_OK)
    {
        nProcess = 253; //should wait or else another session may wrongly report status
    }   
    
    //static int s_nOldProcess = 254;
    if(nProcess == 255)
    {
        //s_bHasRectFirmUpdt_ProcessFinished = TRUE;
        //s_nOldProcess = nProcess;
        // if(s_emRectFirmwUpgr_StartSTatus == RECT_UPDT_START_STATUS_OK)
        // {
        //     s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FINISHED;
        // } 
        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "Upgrade FAILED!");
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", 255);        
    }
    else if(nProcess == 254)
    {
        //s_nOldProcess = nProcess;
        //s_bHasRectFirmUpdt_ProcessFinished = TRUE;
        // if(s_emRectFirmwUpgr_StartSTatus == RECT_UPDT_START_STATUS_OK)
        // {
        //     s_emRectFirmwUpgr_StartSTatus = RECT_UPDT_START_STATUS_FINISHED;
        // }  
        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "Upgrade SUCCESSFULLY!");
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", 254);        
    }
    else if(nProcess == 253)
    {
        // if(s_bHasRectFirmUpdt_ProcessFinished)
        // {
        //     cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", (s_nOldProcess == 255) ? "Upgrade FAILED!" : "Upgrade SUCCESSFULLY!");
        //     cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", s_nOldProcess);  
        // }
        // else
        {
            cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "Please wait...");
            cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", 0);   
        }
    }
    else if(nProcess == 256)
    {
        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "One-Click Upgrade Finished!");
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", 254); 
    }
    else
    {
        //if proces > 0 means already went to upgrading
        // if(nProcess > 0)
        // {
        //     s_bHasRectFirmUpdt_ProcessFinished = TRUE;
        //     s_nOldProcess = 254;
        // }
        cJSON_AddStringToObject(pFCGI_Rough->pJSON_Resp, "status", "Upgrading...");
        cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "process_bar", nProcess);
    } 
    
    return TRUE;
}


BOOL webpage_handler_firmware_mgmt(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----FIRMWARE_MGMT page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nGetType = -1;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nGetType))
        {            
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else
        {
            if(nGetType == 99)
            {
                if(IMMU2_Upgrade_Flag == TRUE)
                {
                    return prepare_IMMU2_upgrage_process_data(pFCGI_Rough);
                }
                return prepare_upgrage_process_data(pFCGI_Rough);
            }   

            VAR_VALUE varV;
            varV.emValue = 0;
            int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 500, 499, varV.abyValue, 4);
            if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
            {
                Make_Web_OutPut(104, "Execute one click firmware error!", "Execute error!", NULL);
                return FALSE;     
            }    

            return prepare_version(pFCGI_Rough);
            // if(nGetType == 0)
            // {
            //     return prepare_controller_version_data(pFCGI_Rough);
            // }
            // else if(nGetType == 1)
            // {
            //     return prepare_controller_histversion_data(pFCGI_Rough);
            // }
            // else if(nGetType == 2)
            // {
            //     return prepare_rectifier_version_data(pFCGI_Rough);
            // }

            // else 
            // {
            //     Make_Web_OutPut(104, "The 'type' has INVALID data!", NULL,  NULL);
            //     return FALSE;              
            // }
        }   
    }
    else //post
    {
        IMMU2_Upgrade_Flag = FALSE;
        //operation id
        // int nOpId = 0;
        // if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "op=", &nOpId))
        // {
        //     Make_Web_OutPut(104, "The 'op' NOT provided!", NULL, NULL);
        //     return FALSE;  
        // }
        // if(nOpId == 0) //upgrade IMMU2
        // {
        //     return Handle_IMMU2_Upgrade(pFCGI_Rough);
        // }
        // else if(nOpId == 1) //upgrade rectifier(s)
        // {
            char szQueryType[32] = "\0";
            if(!ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &szQueryType, 32))
            {            
                Make_Web_OutPut(104, "The 'type' 111 NOT provided!", NULL, NULL);
                return FALSE;  
            }
            
            int upgrade_type = -1;
            int upgrade_iddress = -1;
            int ret = 0;
            if(strchr(szQueryType, '_') != NULL)
            {
                ret = sscanf(szQueryType, "%d_%d", &upgrade_type, &upgrade_iddress);
            }
            else
            {
               
                ret = sscanf(szQueryType, "%d", &upgrade_type);
            }
            
            if(upgrade_type == 100)
            {
                return Handle_OneClick_Upgrade(pFCGI_Rough);
            }
            else if(upgrade_type == 101)
            {   
                IMMU2_Upgrade_Flag = TRUE;
                return Handle_IMMU2_Upgrade(pFCGI_Rough);
            }
            else
            {
                A_BLOB_UPDT_INFO_4CCU blbUpdtCtrl;
                ZERO_POBJS(&blbUpdtCtrl, 1);
                blbUpdtCtrl.emBlobType = BLOB_TYPE_UPGRADE_CCU;
                blbUpdtCtrl.bIs4Web = TRUE;

                if(ret == 2)
                {
                    blbUpdtCtrl.emUpdtMode = 1;
                    blbUpdtCtrl.iDevAddr = upgrade_iddress;
                }
                else if(ret == 1)
                {
                    blbUpdtCtrl.emUpdtMode = 0;
                }

                blbUpdtCtrl.emUpdtType = upgrade_type;

                ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "filename=", &blbUpdtCtrl.file_name, 64);
                blbUpdtCtrl.fIMSU_SWVer_4Updt = blbUpdtCtrl.fDCDC_SWVer_4Updt;
                blbUpdtCtrl.fBMS_SWVer_4Updt = blbUpdtCtrl.fDCDC_SWVer_4Updt;
                blbUpdtCtrl.fPFC_SWVer_4Updt = blbUpdtCtrl.fDCDC_SWVer_4Updt;

                return Handle_Rect_Upgrade(pFCGI_Rough, &blbUpdtCtrl);

            }


            // A_BLOB_UPDT_INFO_4CCU blbUpdtCtrl;
            // ZERO_POBJS(&blbUpdtCtrl, 1);
            // blbUpdtCtrl.emBlobType = BLOB_TYPE_UPGRADE_CCU;
            // blbUpdtCtrl.bIs4Web = TRUE;
            //update mode
            // if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "mode=", &blbUpdtCtrl.emUpdtMode))
            // {
            //     Make_Web_OutPut(104, "The 'mode' NOT provided!", NULL, NULL);
            //     return FALSE;  
            // }
            // else if((blbUpdtCtrl.emUpdtMode < 0) || (blbUpdtCtrl.emUpdtMode > 2))
            // {
            //     Make_Web_OutPut(104, "The 'mode' INVALID!", NULL, NULL);
            //     return FALSE;                 
            // }

            // if(blbUpdtCtrl.emUpdtMode == 2)
            // {
            //      return Handle_OneClick_Upgrade(pFCGI_Rough);
            // }

            //update type
            // if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "tp=", &blbUpdtCtrl.emUpdtType))
            // {
            //     Make_Web_OutPut(104, "The 'tp' NOT provided!", NULL, NULL);
            //     return FALSE;  
            // }
            // else if((blbUpdtCtrl.emUpdtType < 0) || (blbUpdtCtrl.emUpdtType > UPDT_TP_MPPT_U2))
            // {
            //     Make_Web_OutPut(104, "The 'tp' INVALID!", NULL, NULL);
            //     return FALSE;                 
            // }

            //blbUpdtCtrl.emUpdtMode = 1;
            //if(blbUpdtCtrl.emUpdtMode == 1) //BMS Upgrade  Mode
            //{
            //    blbUpdtCtrl.emUpdtType = UPDT_TP_BMS;
            //}
            //else if(blbUpdtCtrl.emUpdtMode == 2) //ABU_U1
            //{
            //    blbUpdtCtrl.emUpdtType = UPDT_TP_ABU_U1;
            //}
            //else if(blbUpdtCtrl.emUpdtMode == 3) //ABU_U2
            //{
            //    blbUpdtCtrl.emUpdtType = UPDT_TP_ABU_U2;
            //}
            //else if(blbUpdtCtrl.emUpdtMode == 4) //EU2_1
            //{
            //    blbUpdtCtrl.emUpdtType = UPDT_TP_EU2_1;
            //}
            //else if(blbUpdtCtrl.emUpdtMode == 5) //EU2_2
            //{
            //    blbUpdtCtrl.emUpdtType = UPDT_TP_EU2_2;
            //}
 

            //http://192.168.1.100/api/firmware_mgmt?op=1&mode=0&tp=1&pfc_version=167&_t=1705112812332

            //http://192.168.1.100/api/firmware_mgmt?op=1&mode=0&tp=2&dcdc_version=267&pfc_version=266&_t=1705112669507

            //version numbers
            // ParseFloat_FromQuery(pFCGI_Rough->szQueryRaw, "dcdc_version=", &blbUpdtCtrl.fDCDC_SWVer_4Updt);
            // ParseFloat_FromQuery(pFCGI_Rough->szQueryRaw, "pfc_version=", &blbUpdtCtrl.fPFC_SWVer_4Updt);
            // ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "iDevAddr=", &blbUpdtCtrl.iDevAddr);
            // ParseStr_FromQuery(pFCGI_Rough->szQueryRaw, "filename=", &blbUpdtCtrl.file_name, 64);

            
            // blbUpdtCtrl.iDevAddr = 50;

            // blbUpdtCtrl.fIMSU_SWVer_4Updt = blbUpdtCtrl.fDCDC_SWVer_4Updt;
	        // blbUpdtCtrl.fBMS_SWVer_4Updt = blbUpdtCtrl.fDCDC_SWVer_4Updt;
            // blbUpdtCtrl.fPFC_SWVer_4Updt = blbUpdtCtrl.fDCDC_SWVer_4Updt;

            
            // if(blbUpdtCtrl.emUpdtMode == 0) //CCU Upgrading
            // {
            //     //blbUpdtCtrl.emUpdtType 
            //     /*if((blbUpdtCtrl.emUpdtType == UPDT_TP_PFC))
            //         //|| (blbUpdtCtrl.emUpdtType == UPDT_TP_ABU_U1)
            //         //|| (blbUpdtCtrl.emUpdtType == UPDT_TP_EU2_1)
            //         //|| (blbUpdtCtrl.emUpdtType == UPDT_TP_EU2_2))
            //     {
            //        if (blbUpdtCtrl.fPFC_SWVer_4Updt <= 0.1)
            //        {
            //             Make_Web_OutPut(104, "U1_PFCVer NULL,Should Be UINT Type.Such as 168!", NULL, NULL);
            //             return FALSE;
            //        }  
            //     }

            //     if((blbUpdtCtrl.emUpdtType == UPDT_TP_DCDC))
            //         //|| (blbUpdtCtrl.emUpdtType == UPDT_TP_ABU_U2))
            //     {
            //         if(blbUpdtCtrl.fDCDC_SWVer_4Updt <= 0.1)
            //         {
            //             Make_Web_OutPut(104, "U2_DCVer NULL,Should Be UINT Type.Such as 268!", NULL, NULL);
            //             return FALSE;
            //         } 
            //     }*/
            // }
            // else if(blbUpdtCtrl.emUpdtMode == 1)//BMS Upgrade
            // {
            //         //Make_Web_OutPut(104, "U2_DCVer NULL,Should Be UINT Type.Such as 268!", NULL, NULL);
            //         //return FALSE;
			// 		blbUpdtCtrl.emUpdtType = UPDT_TP_BMS; 
            //     //}
            // }

            // return Handle_Rect_Upgrade(pFCGI_Rough, &blbUpdtCtrl);
        // }

        // Make_Web_OutPut(104, "The 'op' is NOT correct!", NULL, NULL);
        // return FALSE;  
    }

    return TRUE;
}