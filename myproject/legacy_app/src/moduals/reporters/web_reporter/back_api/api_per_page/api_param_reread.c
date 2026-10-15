//#define _TRACE_PRINT_
#include "../fcgi_defs.h"
#define NCUU_DATA_DB_PATH "/data/nccu.data"

#ifndef cJSON_IsObject
#define cJSON_IsObject(item) ((item) && (item)->type == cJSON_Object)
#endif

#ifndef cJSON_IsArray
#define cJSON_IsArray(item) ((item) && (item)->type == cJSON_Array)
#endif
static BOOL Handle_Single_RereadParamRefresh(cJSON* pItem, FCGI_ROUGH_DATA* pFCGI_Rough, char* szErrMsgOut, int nErrMsgLen)
{
    if (szErrMsgOut != NULL && nErrMsgLen > 0)
    {
        memset(szErrMsgOut, 0, nErrMsgLen);
    }

    if (pItem == NULL || pItem->type != cJSON_Object)
    {
        TRACE("--Invalid JSON object item----\n");
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "Invalid JSON object item!");
        }
        return FALSE;
    }

    int nDevId = 0;
    int nParamId = 0;
    PARAM_TYPE ParamType = 0;
    WEB_PARAM_VALUE* pParamVal = NULL;

    cJSON* temp = cJSON_GetObjectItem(pItem, "dev_id");
    if (temp == NULL)
    {
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "The 'dev_id' is NOT provided!");
        }

        return FALSE;
    }
    if (temp->type != cJSON_Number)
    {
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "The 'dev_id' must be a number!");
        }

        return FALSE;
    }
    nDevId = temp->valueint;

    temp = cJSON_GetObjectItem(pItem, "id");
    if (temp == NULL)
    {
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "The 'id' is NOT provided!");
        }
        return FALSE;
    }
    TRACED("temp->valuestring = %s, temp->type = %d\n", temp->valuestring, temp->type);
    
    if (temp->type != cJSON_String)
    {
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "The 'id' is not a string!");
        }
        return FALSE;
    }
     
    char szId[32] = "\0";
    size_t id_len = strlen(temp->valuestring);
    strncpy(szId, temp->valuestring, sizeof(szId)-1);
    szId[sizeof(szId)-1] = '\0';
    
    int num_count = 0;
    int nums[2] = {0}; 
    num_count = parse_str_to_number(szId, "-", nums, 2);
    if (num_count != 2)
    {
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "The 'id' format error! Need 'xxx-xx'");
        }
        return FALSE;
    }

    nParamId = nums[0];
    ParamType = (PARAM_TYPE)nums[1];

    pParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_CONTROL, nDevId, 254);

    if (pParamVal == NULL || pParamVal->pParamInfo == NULL)
    {
        TRACED("NO Param: dev_id=%d, nParamId=%d, ParamType=%d\n", nDevId, nParamId, ParamType);
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "Parameter not found!");
        }
        return FALSE;
    }

    int nSetCtrl_ExeRtn = WEB_ERRCODE_OK;
    VAR_VALUE varV;
    varV.ulValue = nParamId; // fresh the pcs param value   
    nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, nDevId, 254, varV.abyValue, 4);
    
    if (nSetCtrl_ExeRtn == WEB_ERRCODE_OK)
    {
        Force_Update_ParamVals();
        TRACED("Param Refresh Success! dev_id=%d, id=%s\n", nDevId, szId);
        return TRUE;
    }
    else
    {
        if (szErrMsgOut != NULL && nErrMsgLen > 0)
        {
            snprintf(szErrMsgOut, nErrMsgLen-1, "[%d] Fresh Error: ErrorCode=>%d!", 
                    nParamId, nSetCtrl_ExeRtn);
        }
        Force_Update_ParamVals();
        return FALSE;
    }
}

static BOOL Parse_and_Handle_RereadParamRefresh(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    char szErrMsgOut[512] = {0};
    char szSingleErrMsg[128] = {0};
    TRACED("Parse_and_Handle_RereadParamRefresh\n");
    
    cJSON* pJsonSet = NULL;
    BOOL bResult = TRUE; 
    int i = 0;

    pJsonSet = cJSON_Parse(pFCGI_Rough->szBodyRaw);
    if (pJsonSet == NULL)
    {
        TRACE("--The ParamSet string is not a JSON->[%s]----\n", pFCGI_Rough->szBodyRaw);
        Make_Web_OutPut(104, "The 'param to be set' is NOT valid Json string!", NULL, NULL);
        return FALSE;
    }

    if (pJsonSet->type == cJSON_Object)
    {
        if (!Handle_Single_RereadParamRefresh(pJsonSet, pFCGI_Rough, szErrMsgOut, sizeof(szErrMsgOut)))
        {
            bResult = FALSE;
        }
    }
    else if (pJsonSet->type == cJSON_Array)
    {
        int arraySize = cJSON_GetArraySize(pJsonSet);

        for (i = 0; i < arraySize; i++)
        {
            cJSON* pItem = cJSON_GetArrayItem(pJsonSet, i);
            
            memset(szSingleErrMsg, 0, sizeof(szSingleErrMsg));
            if (!Handle_Single_RereadParamRefresh(pItem, pFCGI_Rough, szSingleErrMsg, sizeof(szSingleErrMsg)))
            {
                bResult = FALSE;
                
                if (strlen(szErrMsgOut) > 0)
                {
                    strncat(szErrMsgOut, " ", sizeof(szErrMsgOut) - strlen(szErrMsgOut) - 1);
                }
                strncat(szErrMsgOut, szSingleErrMsg, sizeof(szErrMsgOut) - strlen(szErrMsgOut) - 1);
            }
        }
    }
    
    if(!bResult)
    {
        if (strlen(szErrMsgOut) == 0)
        {
            snprintf(szErrMsgOut, sizeof(szErrMsgOut)-1, "Param Refresh Error!");
        }
        Make_Web_OutPut(104, szErrMsgOut, "Param Fresh Error!", NULL);
    }
    
    cJSON_Delete(pJsonSet);
    return bResult;
}

BOOL webpage_handler_param_reread(FCGI_ROUGH_DATA* pFCGI_Rough)
{ 
    TRACED("\n----param_reread page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        TRACED("webpage_handler_param_reread:HTTP_REQMETHOD_GET\n");
        return  TRUE;
    }

    else
    {
        TRACED("webpage_handler_param_reread:HTTP_REQMETHOD_POST\n");
        return Parse_and_Handle_RereadParamRefresh(pFCGI_Rough);
    }
    return TRUE;
}