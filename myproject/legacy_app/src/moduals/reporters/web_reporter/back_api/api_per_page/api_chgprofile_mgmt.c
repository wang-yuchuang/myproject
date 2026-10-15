#include "../fcgi_defs.h"
/*
static char* Make_OCPPTimeString(time_t tmInout, char* pszOut)
{
	struct tm currenttime;

	//tmInout = _NOW_; // test purpose  nned to be Comment in Final Version 
	
	gmtime_r(&tmInout,&currenttime);
	currenttime.tm_year = 2000 + (currenttime.tm_year - 100);
	char timeinstring[32] = {'\0'};

	sprintf(timeinstring, "%4d-%02d-%02dT%02d:%02d:%02dZ",currenttime.tm_year, currenttime.tm_mon + 1, 
		currenttime.tm_mday, currenttime.tm_hour, currenttime.tm_min, currenttime.tm_sec);

	memcpy(pszOut, timeinstring,32);
	return pszOut;
}

static void Add_ASingleProfile_2Json(A_CHARGE_PROFILE_CONTENT* pChgProfile, int nBelongedConn, CP_PURPOSE emPurpose, cJSON* pOutJson)
{
    //belongged connectorId(special for web)
    if(nBelongedConn > -1)
    {
        cJSON_AddNumberToObject(pOutJson, "belongConnId", nBelongedConn);
    }

    //id
    cJSON_AddNumberToObject(pOutJson, "chargingProfileId", pChgProfile->nChargingProfile_ID);
    //transId
    if(pChgProfile->nTransaction_ID != CHARGEPROFILE_TRANSID_NOTUSED)
    {
        cJSON_AddNumberToObject(pOutJson, "transactionId", pChgProfile->nTransaction_ID);
    }
    //stackLevel
    cJSON_AddNumberToObject(pOutJson, "stackLevel", pChgProfile->nStackLevel);
    //purpose
    cJSON_AddStringToObject(pOutJson, "chargingProfilePurpose", 
        (emPurpose == CP_CPMaxProfile) ? "ChargePointMaxProfile" :
        ((emPurpose == CP_TxDefaultProfile) ? "TxDefaultProfile" : "TxProfile"));
    //kind & currency
    cJSON_AddStringToObject(pOutJson, "chargingProfileKind", 
        (pChgProfile->emChargingProfile_Kind == CP_Kind_Absolute) ? "Absolute" :
        ((pChgProfile->emChargingProfile_Kind == CP_Kind_Recurring) ? "Recurring" : "Relative"));
    if(pChgProfile->emRecurrencyKind != CP_Recurrency_None)
    {
        cJSON_AddStringToObject(pOutJson, "recurrencyKind", 
            (pChgProfile->emRecurrencyKind == CP_Recurrency_Daily) ? "Daily" : "Weekly");
    }
    //validFrom&To
    char szTime[64];
    if(pChgProfile->dtValidFrom != 0)
    {
        cJSON_AddStringToObject(pOutJson, "validFrom",  
            Make_OCPPTimeString(pChgProfile->dtValidFrom, szTime));
    }
    if(pChgProfile->dtValidTo != 0)
    {
        cJSON_AddStringToObject(pOutJson, "validTo",  
            Make_OCPPTimeString(pChgProfile->dtValidTo, szTime));
    }
    //schedule
    cJSON* pJSched = cJSON_CreateObject();
    CHARGING_SCHEDULE* pSchd = &pChgProfile->chargingSchedule;
    //sche-duration
    if(pSchd->nDuration > 0)
    {
        cJSON_AddNumberToObject(pJSched, "duration", pSchd->nDuration);
    }
    //sche-start_schedule
    if(pSchd->dtStartSchedule != 0)
    {
        cJSON_AddStringToObject(pJSched, "startSchedule",  
            Make_OCPPTimeString(pSchd->dtStartSchedule, szTime));
    }
    //sche-chargingRateUnit
    cJSON_AddStringToObject(pJSched, "chargingRateUnit", 
        (pSchd->emChargingRateUnit == CP_ChargingRateUnit_W) ? "W" : "A");
    //sche-schPeriods
    int n;
    int nPeriodsTotal = MIN(pSchd->nNumberCharingSchedulePeriod, MAX_CP_CHARGINGPERIOD_NUMBDER);
    cJSON* pJPeriodsArr = cJSON_CreateArray();
    CHARGINGSCHEDULE_PERIOD* pPeri = pSchd->charingSchedulePeriodList;
    for(n = 0; n < nPeriodsTotal; n++, pPeri++)
    {
        cJSON* pJPeriodItem = cJSON_CreateObject();
        cJSON_AddNumberToObject(pJPeriodItem, "startPeriod", pPeri->nStartPeriod);
        cJSON_AddFloatToObject(pJPeriodItem, "limit", pPeri->fLimit, 1);
        cJSON_AddItemToArray(pJPeriodsArr, pJPeriodItem);
    }
    cJSON_AddItemToObject(pJSched, "chargingSchedulePeriod", pJPeriodsArr);
    //add back the sche=>profile
    cJSON_AddItemToObject(pOutJson, "chargingSchedule", pJSched);
}

static void Add_AllChPrfiles_2Json(FCGI_ROUGH_DATA* pFCGI_Rough, cJSON* pJAllProf)
{
    int n;
    A_BLOB_CHARGINGPROFILE* pAllProfiles = &pFCGI_Rough->chargerDataRepo.chgProfiles;
    //make max-profile
    for(n = 0; n < MIN(pAllProfiles->nInstalled_MaxCP, MAX_CHARGEPROFILE_STACKLEVEL); n++)
    {
        cJSON* pJMaxSingle = cJSON_CreateObject();
        Add_ASingleProfile_2Json(&pAllProfiles->chargeProfile_MaxCP[n], -1, CP_CPMaxProfile, pJMaxSingle);
        cJSON_AddItemToArray(pJAllProf, pJMaxSingle);
    }
    //make default-profile
    int kk;
    for(kk = 0; kk < 3; kk++)
    {
        for(n = 0; n < MIN(pAllProfiles->nInstalled_TxDefault[kk], MAX_CHARGEPROFILE_STACKLEVEL); n++)
        {
            cJSON* pJDefaultSingle = cJSON_CreateObject();
            Add_ASingleProfile_2Json(&pAllProfiles->chargeProfile_TxDefault[n][kk], kk, CP_TxDefaultProfile, pJDefaultSingle);
            cJSON_AddItemToArray(pJAllProf, pJDefaultSingle);
        }
    }
    //make curr-Profile
    for(kk = 0; kk < 3; kk++)
    {
        if(pAllProfiles->bIs_TxCurrent_Installed[kk])
        {
            cJSON* pJCurrSingle = cJSON_CreateObject();
            Add_ASingleProfile_2Json(&pAllProfiles->chargeProfile_TxCurrent[kk], kk, CP_TxProfile, pJCurrSingle);
            cJSON_AddItemToArray(pJAllProf, pJCurrSingle);
        }
    }    
}

static BOOL Make_ChgProfiles_WebOut(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    cJSON* pJAllProf = cJSON_CreateArray();    
    Add_AllChPrfiles_2Json(pFCGI_Rough, pJAllProf);
    cJSON_AddItemToObject(pFCGI_Rough->pJSON_Resp, "list_ChargingProfiles", pJAllProf);
    return TRUE;
}

static BOOL Handle_Chgprofile_Download2Client(FCGI_ROUGH_DATA* pFCGI_Rough)
{   
    TRACE("Handle_Chgprofile_Download2Client\n");
    //make the json
    cJSON* pJAllProf = cJSON_CreateArray();
    Add_AllChPrfiles_2Json(pFCGI_Rough, pJAllProf);
    cJSON* pAll = cJSON_CreateObject();
    cJSON_AddItemToObject(pAll, "list_ChargingProfiles", pJAllProf);
    //json => tmpfile in var/volatile
#define CHGPROFILE_TMP_FILE "/opt/appdata/data/chgprofiles.json"
    FILE *fp = fopen(CHGPROFILE_TMP_FILE, "w+");
	if (fp == NULL)
	{
		return FALSE;
	}
    char *szJONOut = cJSON_Print_Formatted(pAll);    
    fwrite(szJONOut, strlen(szJONOut), 1, fp);
    free(szJONOut);
    cJSON_Delete(pAll);
    fclose(fp);

    //now upload the file to client
    if(!UploadFile_2Client(CHGPROFILE_TMP_FILE, "chargeProfiles.json"))
    {        
        Make_Web_OutPut(104, "Execute export chargeProfiles error!", "Execute error!", NULL);
    }

    unlink(CHGPROFILE_TMP_FILE);
    return FALSE;
}

static BOOL Parse_SubJson_2SingleProfile(cJSON* pJSingleItem, CP_PURPOSE emPurpose, OUT A_CHARGE_PROFILE_CONTENT* pProfile)
{
    cJSON* pJTmp;
    //id
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "chargingProfileId")) == NULL)
    {
        TRACEX("---chargingProfileId is missing---\n");
        return FALSE;
    }
    pProfile->nChargingProfile_ID = pJTmp->valueint;
    //stackLevel
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "stackLevel")) == NULL)
    {
        TRACEX("---stackLevel is missing---\n");
        return FALSE;
    }
    pProfile->nStackLevel = pJTmp->valueint;
    //chargingProfileKind
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "chargingProfileKind")) == NULL)
    {
        TRACEX("---chargingProfileKind is missing---\n");
        return FALSE;
    }
    else if((pJTmp->valuestring == NULL) || (strlen(pJTmp->valuestring) < 3))
    {
        TRACEX("---chargingProfileKind is wrong format---\n");
        return FALSE;        
    }
    else 
    {
        if(strcmp(pJTmp->valuestring, "Absolute") == 0)
        {
            pProfile->emChargingProfile_Kind = CP_Kind_Absolute;
        }
        else if(strcmp(pJTmp->valuestring, "Recurring") == 0)
        {
            pProfile->emChargingProfile_Kind = CP_Kind_Recurring;
        }
        else if(strcmp(pJTmp->valuestring, "Relative") == 0)
        {
            pProfile->emChargingProfile_Kind = CP_Kind_Relative;
        }
        else
        {
            TRACEX("---chargingProfileKind[%s] NOT found---\n", pJTmp->valuestring);
            return FALSE;   
        }        
    }
    //transactionId
    pProfile->nTransaction_ID = CHARGEPROFILE_TRANSID_NOTUSED;
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "chargingProfileKind")) != NULL)
    {
        if(emPurpose != CP_TxProfile)
        {
           pProfile->nTransaction_ID = pJTmp->valueint; 
        }   
        else
        {
            //just ignor it, do not return false
        }             
    }
    //recurrencyKind
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "recurrencyKind")) != NULL)
    {
        if((pJTmp->valuestring != NULL) && (strlen(pJTmp->valuestring) > 3))
        {
            if(strcmp(pJTmp->valuestring, "Daily") == 0)
            {
                pProfile->emRecurrencyKind = CP_Recurrency_Daily;
            }
            else if(strcmp(pJTmp->valuestring, "Weekly") == 0)
            {
                pProfile->emRecurrencyKind = CP_Recurrency_Weekly;
            }
            else
            {
                //just ignor it, do not return false
            }            
        }
    }
    //validFrom
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "validFrom")) != NULL)
    {
        if((pJTmp->valuestring != NULL) && (strlen(pJTmp->valuestring) > 5))
        {
            if(!Parse_ISO8601TimeStrEx(pJTmp->valuestring, &pProfile->dtValidFrom))
            {
                TRACEX("---the validFrom[%s] is wrong format!---\n", pJTmp->valuestring);
                return FALSE;
            }
        }
    }
    //validTo
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "validTo")) != NULL)
    {
        if((pJTmp->valuestring != NULL) && (strlen(pJTmp->valuestring) > 5))
        {
            if(!Parse_ISO8601TimeStrEx(pJTmp->valuestring, &pProfile->dtValidTo))
            {
                TRACEX("---the validTo[%s] is wrong format!---\n", pJTmp->valuestring);
                return FALSE;
            }
        }
    }
    //chargingSchedule
    cJSON* pJChgSchedule;
    CHARGING_SCHEDULE* pSched = &pProfile->chargingSchedule;
    if((pJChgSchedule = cJSON_GetObjectItem(pJSingleItem, "chargingSchedule")) == NULL)
    {
        TRACEX("---chargingSchedule is missing---\n");
        return FALSE;
    }
    else if(pJChgSchedule->type != cJSON_Object)
    {
        TRACEX("---chargingSchedule is wrong format---\n");
        return FALSE;        
    }
    //sche-chargingRateUnit
    if((pJTmp = cJSON_GetObjectItem(pJChgSchedule, "chargingRateUnit")) == NULL)
    {
        TRACEX("---chargingRateUnit is missing---\n");
        return FALSE;
    }
    else if((pJTmp->valuestring == NULL) || (strlen(pJTmp->valuestring) < 1))
    {
        TRACEX("---chargingRateUnit is wrong format---\n");
        return FALSE;        
    }
    else 
    {
        if(strcmp(pJTmp->valuestring, "W") == 0)
        {
            pSched->emChargingRateUnit = CP_ChargingRateUnit_W;
        }
        else if(strcmp(pJTmp->valuestring, "A") == 0)
        {
            pSched->emChargingRateUnit = CP_ChargingRateUnit_A;
        }
        else
        {
            TRACEX("---chargingRateUnit[%s] is UNKOWN---\n", pJTmp->valuestring);
            return FALSE;      
        }        
    }
    //sche-duration
    if((pJTmp = cJSON_GetObjectItem(pJChgSchedule, "duration")) != NULL)
    {
        pSched->nDuration = pJTmp->valueint;
        if(pSched->nDuration <= 0)
        {
            TRACEX("---wrong duration[%d]---\n", pSched->nDuration);
            return FALSE;                
        }
    }
    //sche-startSchedule
    if((pJTmp = cJSON_GetObjectItem(pJChgSchedule, "startSchedule")) != NULL)
    {
        if((pJTmp->valuestring != NULL) && (strlen(pJTmp->valuestring) > 5))
        {
            if(!Parse_ISO8601TimeStrEx(pJTmp->valuestring, &pSched->dtStartSchedule))
            {
                TRACEX("---the startSchedule[%s] is wrong format!---\n", pJTmp->valuestring);
                return FALSE;
            }
        }
    }
    //sche-chargingSchedulePeriod
    if((pJTmp = cJSON_GetObjectItem(pJChgSchedule, "chargingSchedulePeriod")) == NULL)
    {
        TRACEX("---chargingSchedulePeriod is missing---\n");
        return FALSE;
    }
    else if(pJTmp->type != cJSON_Array)
    {
        TRACEX("---chargingSchedulePeriod is wrong format---\n");
        return FALSE;        
    }
    int nPeriodsTotal = cJSON_GetArraySize(pJTmp);
    if((nPeriodsTotal < 1) || (nPeriodsTotal > MAX_CP_CHARGINGPERIOD_NUMBDER))
    {
        TRACEX("---periodsTotal[%d] is either too small or too big---\n", nPeriodsTotal);
        return FALSE;          
    }
    int n;
    cJSON* pJArrItem;
    cJSON* pJTTTT;
    CHARGINGSCHEDULE_PERIOD* pPeriod = pSched->charingSchedulePeriodList;
    for(n = 0; n < nPeriodsTotal; n++, pPeriod++)
    {
        pJArrItem = cJSON_GetArrayItem(pJTmp, n);
        //startPeriod
        if((pJTTTT = cJSON_GetObjectItem(pJArrItem, "startPeriod")) == NULL)
        {
            TRACEX("---startPeriod is missing---\n");
            return FALSE;                 
        }
        else if(pJTTTT->valueint < 0)
        {
            TRACEX("---startPeriod is WRONG format---\n");
            return FALSE; 
        }
        pPeriod->nStartPeriod = pJTTTT->valueint;        
        //limit
        if((pJTTTT = cJSON_GetObjectItem(pJArrItem, "limit")) == NULL)
        {
            TRACEX("---limit is missing---\n");
            return FALSE;                 
        }
        pPeriod->fLimit = (float)pJTTTT->valuedouble;
    }

    pPeriod = pSched->nNumberCharingSchedulePeriod = nPeriodsTotal;
    return TRUE;
}

static BOOL Parse_SubJson_2Profiles(cJSON* pJSingleItem, OUT A_BLOB_CHARGINGPROFILE* pOutProfiles)
{
    //first parse the purpose
    cJSON* pJTmp;
    A_CHARGE_PROFILE_CONTENT* pSingleProfile;
    if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "chargingProfilePurpose")) == NULL)
    {
        TRACEX("---NO found chargingProfilePurpose!!!---\n");
        return FALSE;
    }
    else if((pJTmp->valuestring == NULL) || (pJTmp->type != cJSON_String))
    {
        TRACEX("---Wrong format for chargingProfilePurpose!!!---\n");
        return FALSE;
    }
    else if(strcmp(pJTmp->valuestring, "ChargePointMaxProfile") == 0)
    {
        if(pOutProfiles->nInstalled_MaxCP >= MAX_CHARGEPROFILE_STACKLEVEL)
        {
            pSingleProfile = &pOutProfiles->chargeProfile_MaxCP[MAX_CHARGEPROFILE_STACKLEVEL - 1];            
        }
        else
        {
            pSingleProfile = &pOutProfiles->chargeProfile_MaxCP[pOutProfiles->nInstalled_MaxCP]; 
        }
        //now handle the parse
        if(!Parse_SubJson_2SingleProfile(pJSingleItem, CP_CPMaxProfile, pSingleProfile))
        {
            return FALSE;
        }
        pOutProfiles->nInstalled_MaxCP = (pOutProfiles->nInstalled_MaxCP >= MAX_CHARGEPROFILE_STACKLEVEL) ? 
            MAX_CHARGEPROFILE_STACKLEVEL : (pOutProfiles->nInstalled_MaxCP + 1);        
    }
    else if(strcmp(pJTmp->valuestring, "TxDefaultProfile") == 0)
    {
        if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "belongConnId")) == NULL)
        {
            TRACEX("----can not get the belongConn for TxDefaultProfile---\n");
            return FALSE;
        }
        int nConnIdx = pJTmp->valueint;
        if((nConnIdx < 0) || (nConnIdx > 3))
        {
            TRACEX("----the belongedConn[%d] is NOT correct!---\n", nConnIdx);
            return FALSE;            
        }
        if(pOutProfiles->nInstalled_TxDefault[nConnIdx] >= MAX_CHARGEPROFILE_STACKLEVEL)
        {
            pSingleProfile = &pOutProfiles->chargeProfile_TxDefault[MAX_CHARGEPROFILE_STACKLEVEL - 1][nConnIdx];            
        }
        else
        {
            pSingleProfile = &pOutProfiles->chargeProfile_TxDefault[pOutProfiles->nInstalled_TxDefault[nConnIdx]][nConnIdx]; 
        }
        //now handle the parse
        if(!Parse_SubJson_2SingleProfile(pJSingleItem, CP_TxDefaultProfile, pSingleProfile))
        {
            return FALSE;
        }
        pOutProfiles->nInstalled_TxDefault[nConnIdx] = (pOutProfiles->nInstalled_TxDefault[nConnIdx] >= MAX_CHARGEPROFILE_STACKLEVEL) ? 
            MAX_CHARGEPROFILE_STACKLEVEL : (pOutProfiles->nInstalled_TxDefault[nConnIdx] + 1);        
    }
    else if(strcmp(pJTmp->valuestring, "TxProfile") == 0)
    {
        if((pJTmp = cJSON_GetObjectItem(pJSingleItem, "belongConnId")) == NULL)
        {
            TRACEX("----can not get the belongConn for TxDefaultProfile---\n");
            return FALSE;
        }
        int nConnIdx = pJTmp->valueint;
        if((nConnIdx < 0) || (nConnIdx > 3))
        {
            TRACEX("----the belongedConn[%d] is NOT correct!---\n", nConnIdx);
            return FALSE;            
        }

        pSingleProfile = &pOutProfiles->chargeProfile_TxCurrent[nConnIdx]; 
      
        //now handle the parse
        if(!Parse_SubJson_2SingleProfile(pJSingleItem, CP_TxProfile, pSingleProfile))
        {
            return FALSE;
        }
        pOutProfiles->bIs_TxCurrent_Installed[nConnIdx] = TRUE;        
    }
    else
    {
        TRACEX("---The purpose[%s] is UNKOWN!---\n", pJTmp->valuestring);
        return FALSE;
    }
    
    return TRUE;
}

static BOOL ParseJSON_fromProfileString(char* szRawStr, OUT A_BLOB_CHARGINGPROFILE* pOutProfiles)
{
    BOOL bParseOK = TRUE;
    cJSON* pJParsed = cJSON_Parse(szRawStr);
    if(pJParsed == NULL)
    {
        TRACEX("----CAN not parse the string=>json-----\n");
        return FALSE;
    }
    cJSON* pJAllProf = pJParsed->child;
    if(pJAllProf == NULL)
    {
        TRACEX("----CAN not parse the arr-string=>json-----\n");
        cJSON_Delete(pJParsed);
        return FALSE;        
    }
    else if((strcmp(pJAllProf->string, "list_ChargingProfiles") != 0) || 
        (pJAllProf->type != cJSON_Array))
    {
        TRACEX("----The arr-name[%s], type[%d] NOT correct-----\n", pJAllProf->string, pJAllProf->type);
        cJSON_Delete(pJParsed);
        return FALSE;                
    }

    //now parse the jSON => profile one by one
    ZERO_POBJS(pOutProfiles, 1);
    int n;
    for(n = 0; n < cJSON_GetArraySize(pJAllProf); n++)
    {
        if(!Parse_SubJson_2Profiles(cJSON_GetArrayItem(pJAllProf, n), pOutProfiles))
        {
            bParseOK = FALSE;
            break;
        }
    }

    cJSON_Delete(pJParsed);
    return bParseOK;
}

static BOOL Handle_Chgprofile_UploadFromClient(FCGI_ROUGH_DATA* pFCGI_Rough)
{
#define CHGPROFILE_IMPORT_FILENAME  "/opt/appdata/data/chgprofiles.import"
// #define CHGPROFILE_IMPORT_FILENAME  "/app/config/chgprofiles.import"
    if(!DownloadFile_FromClient(CHGPROFILE_IMPORT_FILENAME))
    {   
        TRACEX("Download process is FAILED!\n");
        Make_Web_OutPut(104, "Download process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    //now parse and extract for the listChgProfile
    FILE *fp = fopen(CHGPROFILE_IMPORT_FILENAME, "r");
	if (fp == NULL)
	{   
        TRACEX("Can NOT open the file!\n");
        Make_Web_OutPut(104, "Can NOT open the file!", NULL, NULL);
        unlink(CHGPROFILE_IMPORT_FILENAME);
		return FALSE;
	}
    fseek(fp, 0, SEEK_END);
	int nFilelen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if((nFilelen < 3) || (nFilelen > 100 * 1024)) //max shoule be 100Kb
    {   
        TRACEX("Wrong length for the profile!\n");
        Make_Web_OutPut(104, "Wrong length for the profile!", NULL, NULL);
        fclose(fp);
        unlink(CHGPROFILE_IMPORT_FILENAME);
		return FALSE;
    }
    char* szRaw = NEW(char, nFilelen);
    fread(szRaw, nFilelen, 1, fp);
    fclose(fp);
    // unlink(CHGPROFILE_IMPORT_FILENAME);

    TRACEX("szRaw: %s\n", szRaw);

    // // parse the file
    // A_BLOB_CHARGINGPROFILE blobProfiles;
    // BOOL bParseOK = ParseJSON_fromProfileString(szRaw, &blobProfiles);
    // DELETE(szRaw);
    // if(!bParseOK)
    // {
    //     Make_Web_OutPut(104, "Parse FAILED for the profile!", NULL, NULL);
	// 	return FALSE;
    // }

    // //now send the command to main for execution
    // blobProfiles.emBlobType = BLOB_TYPE_CHARGEPROFILE;
    // int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
    //         TRUE, 1, 15, &blobProfiles, sizeof(A_BLOB_CHARGINGPROFILE));
    // if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    // {
    //     Make_Web_OutPut(104, "Execute import chgProfile error!", "Execute error!", NULL);
    //     return FALSE;     
    // }

    // //now copy the new to overwrite our existing profile
    // memcpy(&pFCGI_Rough->chargerDataRepo.chgProfiles, &blobProfiles, sizeof(A_BLOB_CHARGINGPROFILE));
    // pFCGI_Rough->bNeedUpdateChgProfiles = TRUE;

    // now send the updated profiles
    // return Make_ChgProfiles_WebOut(pFCGI_Rough);
    // return TRUE;
}
*/

static BOOL Handle_BMS_CONFIG(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    //if(pFCGI_Rough->nTempFileLen <= 0)

    if(!Create_2BinFile_FromBody())
    {
        Make_Web_OutPut(104, "The data file is NOT received yet!", NULL,  NULL);
        return FALSE;                  
    }
    //now downloaded done
    TRACEX("-----------Now downloaded firmware file DONE!!!----------\n");
    rename(WEB_TEMPFILE_DOWNLOADED_RAW, BMS_CONFIG_PARAMERTER);
    
    VAR_VALUE varV;
    varV.emValue = 0;  
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 500, 498, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute upgrade IMMU2 firmware error!", "Execute error!", NULL);
        unlink(BMS_CONFIG_PARAMERTER);
        return FALSE;     
    }

    unlink(BMS_CONFIG_PARAMERTER);

    return TRUE;
}


BOOL webpage_handler_chgprofile_mgmt(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----CHGPROFILE_MGMT page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        // return Make_ChgProfiles_WebOut(pFCGI_Rough);
        return TRUE;
    }
    else //post
    {
        int nOpId = -99;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "op=", &nOpId))
        {
            Make_Web_OutPut(104, "The 'op' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        if(nOpId == 0)
        {   
            // TRACE("\n----DOWNLOAD---\n");
            // return Handle_Chgprofile_Download2Client(pFCGI_Rough);
            return TRUE;
        }
        else if(nOpId == 1)
        {   
            TRACE("\n----UPLOAD---\n");
            return Create_2BinFile_FromBody_mTLS();
            // return Create_2BinFile_FromBody();
        }
        else if(nOpId == 3)
        {   
            TRACE("\n----UPLOAD---\n");
            return Handle_BMS_CONFIG(pFCGI_Rough);
        }

        Make_Web_OutPut(104, "The 'op' is NOT correct!", NULL, NULL);
        return FALSE;  
    }

    return TRUE;
}
