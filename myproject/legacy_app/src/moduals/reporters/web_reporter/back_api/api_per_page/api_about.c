#include "../fcgi_defs.h"

#define AIR_NUM 2

typedef enum tagAboutSetMenuId
{
    ABOUTSET_MENUID_VERSIONSET = 0,
    ABOUTSET_MENUID_MAX
}ABOUTSET_MENUIDS;

static BOOL Make_AbortSet_Data(FCGI_ROUGH_DATA* pFCGI_Rough, int nClasssId)
{
    WEB_PARAM_VALUE            *pParamVal;
    cJSON *moreinfo = cJSON_CreateArray();

    if(nClasssId == 0)
    {
        int n;
        char szVVV[32];
        int STBattNum = 0;
        //edit by Kuangyue

        //1、软件版本信息
        memset(szVVV, 0, 32);
        cJSON *SWVer = cJSON_CreateObject();
        snprintf(szVVV, 32, "%s, %s", pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_SWVer,
            pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_HWVer);
        cJSON_AddStringToObject(SWVer, "lable", LoadLangText_ById(125));
        cJSON_AddStringToObject(SWVer, "value", szVVV); 
        cJSON_AddItemToArray(moreinfo, SWVer);

        //2、CCU固件版本	
        memset(szVVV, 0, 32);
        cJSON *D_Vers = cJSON_CreateObject();
        snprintf(szVVV, 32, "%s", pFCGI_Rough->chargerDataRepo.versionInfos.szIMSU_D_Vers);
        cJSON_AddStringToObject(D_Vers, "lable", LoadLangText_ById(319));
        cJSON_AddStringToObject(D_Vers, "value", szVVV);
        cJSON_AddItemToArray(moreinfo, D_Vers);

        A_BLOB_SINGLE_RECT_INFO allsingleRectData;
        if(!Get_ASingle_ParamVal(PARAM_TYPE_STATUS, 200, 101, sizeof(A_BLOB_SINGLE_RECT_INFO), (void*)&allsingleRectData))
        {
            TRACEX("----ERRRRRR: can not get all Single Rect Data----\n");
            Make_Web_OutPut(104, "can not get all Single Rect Data!", NULL, NULL);
            return TRUE;
        }
        int whichNum = 0, i = 0;
        char szName[32] = {0};
        char szSingleRectName[64] = "\0";
        STU_SIGNLE_RECT_DATA singleRectData;
        for(n = TYPE_SIG_RECT_ACDC_GRP1; n <= TYPE_SIG_RECT_ACDC_GRP4; n++)
        {
            for(i = 0; i < (int)allsingleRectData.nOnlineNum[n]; ++i)
            {
                singleRectData = allsingleRectData.arrRect[n][i];
                
                snprintf(szSingleRectName, 64, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
                memset(szVVV, 0, 32);
                snprintf(szVVV, 32, "%s", szSingleRectName);
                snprintf(szName, 32, "%d-AC/DC(U1/U2)%s", ++whichNum, LoadLangText_ById(125));  

                cJSON *Ver_DC = cJSON_CreateObject();
                cJSON_AddStringToObject(Ver_DC, "lable", szName);
                cJSON_AddStringToObject(Ver_DC, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, Ver_DC);
            }
        }
        whichNum = 0;
        for(n = TYPE_SIG_RECT_MPPT_GRP1; n <= TYPE_SIG_RECT_MPPT_GRP4; n++)
        {
            for(i = 0; i < (int)allsingleRectData.nOnlineNum[n]; ++i)
            {
                singleRectData = allsingleRectData.arrRect[n][i];
                
                snprintf(szSingleRectName, 64, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
                memset(szVVV, 0, 32);
                snprintf(szVVV, 32, "%s", szSingleRectName);
                snprintf(szName, 32, "%d-MPPT(U1/U2)%s", ++whichNum, LoadLangText_ById(125));  

                cJSON *Ver_PFC = cJSON_CreateObject();
                cJSON_AddStringToObject(Ver_PFC, "lable", szName);
                cJSON_AddStringToObject(Ver_PFC, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, Ver_PFC);
            }
        }
        if(allsingleRectData.nOnlineNum[TYPE_SIG_RECT_STS] > 0)
        {
                singleRectData = allsingleRectData.arrRect[TYPE_SIG_RECT_STS][0];
                
                snprintf(szSingleRectName, 64, "%.2f|%.2f", singleRectData.fRect_PFCVer, singleRectData.fRect_DCDCVer);
                memset(szVVV, 0, 32);
                snprintf(szVVV, 32, "%s", szSingleRectName);
                snprintf(szName, 32, "ABU(U1/U2)%s", LoadLangText_ById(125));  

                cJSON *ABU_Ver = cJSON_CreateObject();
                cJSON_AddStringToObject(ABU_Ver, "lable", szName);
                cJSON_AddStringToObject(ABU_Ver, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, ABU_Ver);
        }

        //6、空调安装类型：英飞源空调/Tongfei空调
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
                
                TRACED("Kuangyue: pVerInfo->Air_Cond_Ver[%d]%s", n, szVVV);
                cJSON *Air_Type = cJSON_CreateObject();
                cJSON_AddStringToObject(Air_Type, "lable", LoadLangText_ById(4014 + n));
                cJSON_AddStringToObject(Air_Type, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, Air_Type);
                break;
                //edit by kuangyue 20250217
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
                
                TRACED("Kuangyue: pVerInfo->Air_Cond_Ver[%d]%s", n, szVVV);
                cJSON *Air_Type1 = cJSON_CreateObject();
                cJSON_AddStringToObject(Air_Type1, "lable", LoadLangText_ById(4014 + n));
                cJSON_AddStringToObject(Air_Type1, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, Air_Type1);
                break;
            case TONGFEI_AIR:
            case AIR_NOT_INSTALLED:
                TRACED("Air Conditioner szAir_Cond_Ver Type NULL");
                break;
            default:
                break;
            }
        }

        //Get STBattery Number 获取电池组数量
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
                snprintf(szVVV, 32, "%.3f", Air_pVerInfo->fVersion);//edit by Kuangyue 20250227
            }
            else
            {
                snprintf(szVVV, 32, "--");
            }
            TRACED("Kuangyue: pVerInfo->STBatt_Ver[%d]%s", n, szVVV);
            cJSON *STBatt = cJSON_CreateObject();
            cJSON_AddStringToObject(STBatt, "lable", LoadLangText_ById(4000 + n));
            cJSON_AddStringToObject(STBatt, "value", szVVV);
            cJSON_AddItemToArray(moreinfo, STBatt);
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
            cJSON *BSU_Ver = cJSON_CreateObject();
            cJSON_AddStringToObject(BSU_Ver, "lable", szName);
            cJSON_AddStringToObject(BSU_Ver, "value", szVVV);
            cJSON_AddItemToArray(moreinfo, BSU_Ver);
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
            
            TRACED("Kuangyue: pVerInfo->IMEU3_Ver_Pow:%s", szVVV);
            cJSON *STBattIMEU3 = cJSON_CreateObject();
            cJSON_AddStringToObject(STBattIMEU3, "lable", LoadLangText_ById(4013));
            cJSON_AddStringToObject(STBattIMEU3, "value", szVVV);
            cJSON_AddItemToArray(moreinfo, STBattIMEU3);
            break;
        case IMEU_TYPE_IMEU2:
        case IMEU_TYPE_NOT_INSTALLED:
            TRACED("Power szIMEU2_Ver Type NULL");
            break;
        }

        //8、电池柜 IMEU3/IMEU2 数据
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
            TRACED("Battery IMEU emValue Type:%d", pParamVal->currValue.emValue);
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
                TRACED("Kuangyue: pVerInfo->IMEU3_Ver_Bat[%d]%s", n, szVVV);
                cJSON *STBattIMEU3 = cJSON_CreateObject();
                cJSON_AddStringToObject(STBattIMEU3, "lable", LoadLangText_ById(4008 + n));
                cJSON_AddStringToObject(STBattIMEU3, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, STBattIMEU3);
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
                TRACED("Kuangyue: pVerInfo->IMEU2_Ver_Bat[%d]%s", n, szVVV);
                cJSON *STBattIMEU2 = cJSON_CreateObject();
                cJSON_AddStringToObject(STBattIMEU2, "lable", LoadLangText_ById(4004 + n));
                cJSON_AddStringToObject(STBattIMEU2, "value", szVVV);
                cJSON_AddItemToArray(moreinfo, STBattIMEU2);
                break;
            case IMEU_TYPE_NOT_INSTALLED://Not Installed
                TRACED("Kuangyue: IMEU_TYPE_NOT_INSTALLED");
                break;
            default:
                break;
            }
        }
    }

    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "moreinfo", moreinfo);
    return TRUE;
}


BOOL webpage_handler_about_info(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----ABORT page_handler was called!---\n");

    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nClassId = -1;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nClassId))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }

        if((nClassId >= 0) && (nClassId < ABOUTSET_MENUID_MAX))
        {
            VAR_VALUE varV;
            varV.emValue = 0;
            int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 500, 499, varV.abyValue, 4);
            if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
            {
                Make_Web_OutPut(104, "Execute one click firmware error!", "Execute error!", NULL);
                return FALSE;     
            }

            return Make_AbortSet_Data(pFCGI_Rough, nClassId);
        }
        else
        {
            Make_Web_OutPut(104, "The 'type' value INVALID!", NULL, NULL);
            return FALSE;  
        }

    }
    else
    {
        Make_Web_OutPut(104, "Abort page does NOT allow POST!",NULL,  NULL);
        return FALSE;  
    }

    return TRUE;
}

