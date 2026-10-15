// #define _TRACE_PRINT_
#include "../fcgi_defs.h"

static BOOL Handel_Get_Cell_Barcode(FCGI_ROUGH_DATA* pFCGI_Rough, int nType, int nPackIdx, int nCellIdx)
{
    int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
    VAR_VALUE varV;
    if(nType == 0){
        varV.lValue = nPackIdx * 1000 + nCellIdx;
        nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
        FALSE, 500, 666, varV.abyValue, 4);
    } else if(nType == 1){
        varV.lValue = nPackIdx;
        nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
        FALSE, 500, 667, varV.abyValue, 4);
    } else if(nType == 2){
        varV.lValue = nPackIdx;
        nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
        FALSE, 500, 668, varV.abyValue, 4);
    } else {
        Make_Web_OutPut(104, "Get barcode error!", "Invalid type parameter!", NULL);
        return FALSE;
    }

    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Get barcode error!", "Execute error!", NULL);
        return FALSE;   
    }

    return TRUE;
}

static BOOL Handel_Get_Barcode_Permission(FCGI_ROUGH_DATA* pFCGI_Rough, int nType, int nPassword)
{
    if(nType == 99 && nPassword == 20251212){
        cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "barcode_permission", cJSON_CreateBool(1));
    } else if(nType == 99 && nPassword != 20251212){
        Make_Web_OutPut(104, "Get password error!", "Execute error!", NULL);
        return FALSE;   
    }

    return TRUE;
}

static BOOL Handel_Set_Cell_Barcode(FCGI_ROUGH_DATA* pFCGI_Rough, int nType, int nPackIdx, int nCellIdx, char* szNewBarcode)
{
    int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
    
    char sendData[128];
    memset(sendData, 0, sizeof(sendData));
    snprintf(sendData, sizeof(sendData), "%d:%d:%d:%s", nType, nPackIdx, nCellIdx, szNewBarcode);
    
    if(nType == 0 || nType == 1|| nType == 2){
        WEB_PARAM_VALUE* pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_CONTROL, 500, 669);
        nSetCtrl_ExeRtn = Handle_SingleParam_SetCtrl(pParamVal, 500, sendData, strlen(sendData) + 1);
    } else {
        Make_Web_OutPut(104, "Set barcode error!", "Invalid type parameter!", NULL);
        return FALSE;
    }

    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Set barcode error!", "Execute error!", NULL);
        return FALSE;   
    }

    return TRUE;
}

BOOL webpage_handler_cell_barcode(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----CELL BARCODE page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nType = 0; 
        int nPackIdx = 0; 
        int nCellIdx = 1; 

        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "type=", &nType))
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pack=", &nPackIdx))
        {
            Make_Web_OutPut(104, "The 'pack' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        if(nType == 0){
            if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cell=", &nCellIdx))
            {
                Make_Web_OutPut(104, "The 'cell' NOT provided!", NULL, NULL);
                return FALSE;  
            }
        }

        return Handel_Get_Cell_Barcode(pFCGI_Rough, nType, nPackIdx, nCellIdx);
    }
    else 
    {
        cJSON* pJSON_Body = cJSON_Parse(pFCGI_Rough->szBodyRaw);
        cJSON* pJType = NULL;
       
        if((pJType = cJSON_GetObjectItem(pJSON_Body, "type")) == NULL)
        {
            Make_Web_OutPut(104, "The 'type' NOT provided!", NULL, NULL);
            cJSON_Delete(pJSON_Body);
            return FALSE;  
        }
        int nTypeValue = pJType->valueint;
        
        if(nTypeValue == 99)
        {   
            cJSON* pJPassword = NULL;
            if((pJPassword = cJSON_GetObjectItem(pJSON_Body, "password")) == NULL)
            {
                Make_Web_OutPut(104, "The 'password' NOT provided!", NULL, NULL);
                cJSON_Delete(pJSON_Body);
                return FALSE;  
            }
            int nPasswordValue = pJPassword->valueint;
            return Handel_Get_Barcode_Permission(pFCGI_Rough, nTypeValue, nPasswordValue);
        }
        else
        {
            cJSON* pJNewBarcode = NULL;
            cJSON* pJPack = NULL;
            cJSON* pJCell = NULL;
            
            if((pJNewBarcode = cJSON_GetObjectItem(pJSON_Body, "newbarcode")) == NULL)
            {
                Make_Web_OutPut(104, "The 'newbarcode' NOT provided!", NULL, NULL);
                cJSON_Delete(pJSON_Body);
                return FALSE;  
            }
            char* szNewBarcode = pJNewBarcode->valuestring; 

            if((pJPack = cJSON_GetObjectItem(pJSON_Body, "pack")) == NULL)
            {
                Make_Web_OutPut(104, "The 'pack' NOT provided!", NULL, NULL);
                cJSON_Delete(pJSON_Body);
                return FALSE;  
            }
            int nPackIdx = pJPack->valueint;

            int nCellIdx = 1;
            if(nTypeValue == 0){
                if((pJCell = cJSON_GetObjectItem(pJSON_Body, "cell")) == NULL)
                {
                    Make_Web_OutPut(104, "The 'cell' NOT provided!", NULL, NULL);
                    cJSON_Delete(pJSON_Body);
                    return FALSE;  
                }
                int nCellIdx = pJCell->valueint;
            }

            return Handel_Set_Cell_Barcode(pFCGI_Rough, nTypeValue, nPackIdx, nCellIdx, szNewBarcode);
        }
    }

    return TRUE;
}