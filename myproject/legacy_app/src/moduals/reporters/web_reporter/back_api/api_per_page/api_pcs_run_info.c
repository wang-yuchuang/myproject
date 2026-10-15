//#define _TRACE_PRINT_
#include "../fcgi_defs.h"


static BOOL Handle_Pcs_RunInfo_PageData(FCGI_ROUGH_DATA* pFCGI_Rough, int nPageId, int nPageVolume, int nPcsIdx)
{
    // int n;
    // int m;
    // QUERYCOND_HISMETERVAL cond;
    // BMS_RUNTIMEDATA_LIST* pList = &pFCGI_Rough->chargerDataRepo.bmsRunData_Rec.bmsRunData;
    // int nTotal;
    // UINT   szTimeList[WEB_SUPPORT_MAX_BMSREC];
    // UINT   nTempValue;
    // int    nRunInfoTotal = 0;
    // int    nIdx = 1;

    // for(n = 0; n < WEB_SUPPORT_MAX_BMSREC; n++)
    // {
    //     szTimeList[n] = pFCGI_Rough->chargerDataRepo.bmsRecData[n].nDay;
    // }

    // for(n = WEB_SUPPORT_MAX_BMSREC - 2; n >= 0; n--)
    // {
    //     for(m = 0; m <= n; m++)
    //     {
    //         if(szTimeList[m] > szTimeList[m + 1])
    //         {
    //             nTempValue = szTimeList[m];
    //             szTimeList[m] = szTimeList[m + 1];
    //             szTimeList[m + 1] = nTempValue;
    //         }
    //     }
    // }

    // cJSON* pJSPcsRunInfoList = cJSON_CreateArray();
    // for(n = WEB_SUPPORT_MAX_BMSREC - 1; n >= 0; n--)
    // {
    //     cond.nGunIdx = szTimeList[n];
    //     if(!QueryChgRunData_byCond(&cond))
    //     {
    //         Make_Web_OutPut(104, "Did not find such bms_rundata record!", NULL, NULL);
    //         continue;
    //     }

    //     nTotal = MIN(pList->nBmsRuntimeDataTotal, MAX_HIS_BMS_RUNTIME_DATALIST);
    //     nRunInfoTotal += nTotal;

    //     for(m = nTotal; m >= 0; m--)
    //     {
    //         if(pList->stBmsRuntimeDataList[m].pcsRunInfoRec[0][0].nExist == 0)
    //         {
    //             continue;
    //         }

    //         cJSON* pJSPcsRunInfo = cJSON_CreateObject();
    //         cJSON_AddNumberToObject(pJSPcsRunInfo, "idx", nIdx);
    //         nIdx += 1;
    //         cJSON_AddItemToObject(pJSPcsRunInfo, "voltage", cJSON_CreateFloat(pList->stBmsRuntimeDataList[m].pcsRunInfoRec[0][0].fOutputVolt, 1));
    //         //pList->stBmsRuntimeDataList[m].pcsRunInfoRec[0][0].

    //         cJSON_AddItemToArray(pJSPcsRunInfoList, pJSPcsRunInfo);
    //     }
    // }

    // cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "rec_list", pJSPcsRunInfoList);

    // int nTotalPage = ((nRunInfoTotal % nPageVolume) == 0) ? 
    //     (nRunInfoTotal / nPageVolume) : ((nRunInfoTotal / nPageVolume) + 1);
    // //now add general info    
    // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "rec_total", nRunInfoTotal);
    // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "page_total", nTotalPage);
    // cJSON_AddNumberToObject(pFCGI_Rough->pJSON_Resp, "curr_pageId", nPageId);

    return TRUE;
}


BOOL webpage_handler_pcs_run_info(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----RUN_INFO page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nPageId = -1;
        int nPageVolume = -1;
        int nPcsIdx = -1;

        //page Id for get
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "pageId=", &nPageId))
        {
            Make_Web_OutPut(104, "The 'pageId' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        //page volume
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "page_vol=", &nPageVolume))
        {
            Make_Web_OutPut(104, "The 'page_vol' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        else if((nPageVolume < 10) || (nPageVolume > 1000))
        {
            Make_Web_OutPut(104, "The 'page_vol' shall be in range (10 ~ 1000)!", NULL, NULL);
            return FALSE;            
        }
        //now the query conditions
        if(ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "cond_pcs=", &nPcsIdx))
        {
            if(nPcsIdx < 0)
            {
                Make_Web_OutPut(104, "The 'cond_pcs' is OUT of range!", NULL, NULL);
                return FALSE;  
            }
        }
        return Handle_Pcs_RunInfo_PageData(pFCGI_Rough, nPageId, nPageVolume, nPcsIdx);
    }
    else
    {
        Make_Web_OutPut(104, "This page does NOT allow POST!",NULL,  NULL);
        return FALSE;
    }

    return TRUE;
}