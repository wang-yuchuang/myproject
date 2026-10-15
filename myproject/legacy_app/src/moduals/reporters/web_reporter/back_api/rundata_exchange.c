// #define _TRACE_PRINT_
#include "fcgi_defs.h"

static FCGI_ROUGH_DATA* s_pFCGIRough;
typedef BOOL (*DB_PROC_FUNC)(sqlite3* db, void* pArg);

static BOOL Process_DB_OP(WebDatabaseType bIsCfgDB, DB_PROC_FUNC pfnHandler, void* pArg)
{
    sqlite3 *db;

    const char* db_path = NULL;

    switch(bIsCfgDB)
    {
        case WEB_DB_TYPE_SYSTEM:
            db_path = "/app/config/System.cfg";
            break;
        case WEB_DB_TYPE_NCCU_DATA:
            db_path = "/data/nccu.data";
            break;
        case WEB_DB_TYPE_BMS_DATA:
            db_path = "/data/BmsRunData.data";
            break;  
        case WEB_DB_TYPE_APP_LOG:
            db_path = "/data/AppLog.data";
            break;
        default:
            return FALSE;          
    }

    int rc = sqlite3_open_v2( db_path/* bIsCfgDB ? "/app/config/System.cfg" : "/data/nccu.data" */,
            &db, /*SQLITE_OPEN_READWRITE*/SQLITE_OPEN_READONLY, NULL); //has to be read-only
    if(rc != SQLITE_OK)
    {
        TRACEX("----can NOT open %s db---\n", bIsCfgDB ? "CFG" : "DATA");
        return FALSE;
    }

    //added by Jimmy for timeout handler, if not set, query may get SQLITE_MISUSE error
    sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database

    //now execute the operation
    BOOL bSuccess = TRUE;
    if(pfnHandler)
    {
        if(!pfnHandler(db, pArg))
        {
            bSuccess = FALSE;
        }
    }

    TRACEX("----now handle the %d db OP %s!---\n", bIsCfgDB, 
            bSuccess ? "successfully" : "FAILED");
    //now close the db. because we use it and then close it
    sqlite3_close(db);

    return bSuccess;
}

static BOOL GetTotalResource_Count(sqlite3* db, void* pArg)
{
	sqlite3_stmt * stat;
	sqlite3_prepare_v2(db, "select count(*) from Resources", -1, &stat, NULL);
    int nCount = -1;
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        //if(sqlite3_column_type(stat, 0) == SQLITE_INTEGER)
        {
           nCount = sqlite3_column_int(stat, 0); 
        } 

        TRACEX("---The resource total count = %d, sqTP = %d---\n", nCount, sqlite3_column_type(stat, 0));
    
        break;
    }    

    sqlite3_finalize(stat);
    // LJH250125 3000->5000
    s_pFCGIRough->stResInfo.nResTotal = MIN((UINT)nCount, 6000); //in case wrong parse
    
    return (nCount < 0) ? FALSE : TRUE;
}

static BOOL GetWeb_MainIdx(sqlite3* db, void* pArg)
{
    sqlite3_stmt * stat;
	sqlite3_prepare_v2(db, "select * from Web_MainIdx", -1, &stat, NULL);

    WEBMAINIDX Web_MainIdx[MAX_MANIDX_NUM];
    UINT nCount = 0;

	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        Web_MainIdx[nCount].nParaId = sqlite3_column_int(stat, 0); 
        Web_MainIdx[nCount].nSetOrStatus = sqlite3_column_int(stat, 2);
        Web_MainIdx[nCount].nResId = sqlite3_column_int(stat, 3);
        nCount++;
    }    
    sqlite3_finalize(stat);
    
    UINT Set_num = 0;
    UINT Status_num = 0;

    int n = 0;
    for(n = 0; n < nCount; n++)
    {   
        if(Web_MainIdx[n].nSetOrStatus == 1) //setting
        {
            s_pFCGIRough->chargerDataRepo.ParaSetDispList1[Set_num].nRes_Id = Web_MainIdx[n].nResId;
            s_pFCGIRough->chargerDataRepo.ParaSetDispList1[Set_num].main_id = Web_MainIdx[n].nParaId;
            Set_num++;
            // printf("----Set num=%d, ResId=%d---\n", Set_num, Web_MainIdx[n].nResId);
        }
        else if(Web_MainIdx[n].nSetOrStatus == 2)//status
        {
            s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[Status_num].nRes_Id = Web_MainIdx[n].nResId;
            s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[Status_num].main_id = Web_MainIdx[n].nParaId;
            Status_num++;
            // printf("----Status num=%d, ResId=%d---main_id=%d\n", Status_num, Web_MainIdx[n].nResId, Web_MainIdx[n].nParaId);
        }
    }
    s_pFCGIRough->chargerDataRepo.Setnum = Set_num;
    s_pFCGIRough->chargerDataRepo.Stanum = Status_num;
    return TRUE;
}

static BOOL GetWeb_SubIdx(sqlite3* db, void* pArg)
{
    sqlite3_stmt * stat;
	sqlite3_prepare_v2(db, "select * from Web_SubIdx", -1, &stat, NULL);

    WEBSUBIDX Web_SubIdx[MAX_SUBIDX_NUM];
    UINT nCount = 0;

	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        Web_SubIdx[nCount].nParaID = sqlite3_column_int(stat, 0); 
        Web_SubIdx[nCount].nBellongMainID = sqlite3_column_int(stat, 2);
        Web_SubIdx[nCount].nSetOrStatus = sqlite3_column_int(stat, 3);
        Web_SubIdx[nCount].nResId = sqlite3_column_int(stat, 4);
        nCount++;
    }    
    sqlite3_finalize(stat);

    int m, n;
    int i = 0;
    for(i = 0; i < s_pFCGIRough->chargerDataRepo.Setnum; i++)
    {
        s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal = 0;
    }
    // printf("----%d-------\n", s_pFCGIRough->chargerDataRepo.Setnum);

    for(m = 0; m < s_pFCGIRough->chargerDataRepo.Setnum; m++)
    {
        for(n = 0; n < nCount; n++)
        {
            if((Web_SubIdx[n].nBellongMainID == s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].main_id) &&
               (Web_SubIdx[n].nSetOrStatus == 1))
            {
                s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].SubDispList[
                    s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].nSubTotal].Value = Web_SubIdx[n].nResId;

                s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].SubDispList[
                    s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].nSubTotal].Sub_id = Web_SubIdx[n].nParaID;

                s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].SubDispList[
                    s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].nSubTotal].bIsDisp = TRUE;

                s_pFCGIRough->chargerDataRepo.ParaSetDispList1[m].nSubTotal++;

            }
        }
    }
    // printf("----%d-------\n", s_pFCGIRough->chargerDataRepo.Stanum);
    for(i = 0; i < s_pFCGIRough->chargerDataRepo.Stanum; i++)
    {
        s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].nSubTotal = 0;
    }
    for(m = 0; m < s_pFCGIRough->chargerDataRepo.Stanum; m++)
    {
        for(n = 0; n < nCount; n++)
        {
            if((Web_SubIdx[n].nBellongMainID == s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].main_id) &&
               (Web_SubIdx[n].nSetOrStatus == 2))
            {
                s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].SubDispList[
                    s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].nSubTotal].Value = Web_SubIdx[n].nResId;

                s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].SubDispList[
                    s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].nSubTotal].Sub_id = Web_SubIdx[n].nParaID;

                s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].SubDispList[
                    s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].nSubTotal].bIsDisp = TRUE;

                s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].nSubTotal++;

                // printf("%d----Status MainIdx ResId=%d, SubIdx ResId=%d---\n", m, \
                //     s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[m].main_id,
                //     Web_SubIdx[n].nResId);
            }
        }
    }
    return TRUE;
}



#define ADD_LANGINFO(_langID, _resId) \ 
    do{ \
        s_pFCGIRough->stResInfo.langInfos[s_pFCGIRough->stResInfo.nLangTotal].nLangId = (_langID); \
        s_pFCGIRough->stResInfo.langInfos[s_pFCGIRough->stResInfo.nLangTotal].nLangName_ResId = (_resId); \
        s_pFCGIRough->stResInfo.nLangTotal++; \
    }while(0)
    

static BOOL Init_AllLangInfo(FCGI_ROUGH_DATA* pFCGIRough)
{
    pFCGIRough->stResInfo.nLangTotal = 0;
    sprintf(pFCGIRough->szUnkownRes, "%s", "UNKOWN");
    //all lang list and res_id
    ADD_LANGINFO(LANG_TYPE_CHINESE,                           232);
    ADD_LANGINFO(LANG_TYPE_ENGLISH,                           233);
    ADD_LANGINFO(LANG_TYPE_ITALIAN,                           401);
    ADD_LANGINFO(LANG_TYPE_CHINESE_TRADITIONAL,               420);
    ADD_LANGINFO(LANG_TYPE_GERMAN,                            421);
    ADD_LANGINFO(LANG_TYPE_SPNANISH,                          487);
    ADD_LANGINFO(LANG_TYPE_PORTUGUESE,                        488);
    ADD_LANGINFO(LANG_TYPE_FRENCH,                            2098);
    ADD_LANGINFO(LANG_TYPE_AUSTRIAN_CERTIFICATION,            7006);
    //ADD_LANGINFO(LANG_TYPE_CZECH,                             500);
    //ADD_LANGINFO(LANG_TYPE_POLISH,                            571);
    //ADD_LANGINFO(LANG_TYPE_HUNGARIAN,                         575);
    //ADD_LANGINFO(LANG_TYPE_GREEK,                             635);
    
    //GET resources total
    return Process_DB_OP(WEB_DB_TYPE_SYSTEM, GetTotalResource_Count, NULL);
}

static BOOL Load_Resource_ByLangId(sqlite3* db, void* pArg)
{
    TRANS_DICTIONARY* pDict = (TRANS_DICTIONARY*)pArg;
    if(pDict == NULL)
    {
        return FALSE;
    }
    
    sqlite3_stmt * stat;
	sqlite3_prepare_v2(db, "select * from Resources", -1, &stat, NULL);
    int nCount = 0;
    UINT nLangId = (UINT)pDict->nLangId;
    int nLenLmt = 0;
    char* pText;
    TRANS_TEXT* pTrans;
    pDict->nLoadedTextTotal = 0;
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        nLenLmt = sqlite3_column_int(stat, 2); 
        if((nLenLmt < 0) || (nLenLmt > 256))
        {
            continue;
        }
        else if(nLenLmt == 0) //should not be '0'
        {
            nLenLmt = 1;
        }

        pText = sqlite3_column_text(stat, 3 + nLangId);
        if(pText != NULL)
        {
            //added by Jimmy 20201026 for length compitible
            int nLenLmtEx = strlen(pText) + 1;
            nLenLmt = MAX(nLenLmt, nLenLmtEx);
            pTrans = &pDict->pTransTexts[nCount];
            pTrans->nResId = sqlite3_column_int(stat, 0);            
            pTrans->nLenLmt = (USHORT)nLenLmt;
            pTrans->szRaw = NEW(char, nLenLmt);
            snprintf(pTrans->szRaw, nLenLmt, "%s", sqlite3_column_text(stat, 1));
            pTrans->szTrans = NEW(char, nLenLmt);
            memset(pTrans->szTrans, 0, nLenLmt);
            if(pTrans->nResId != 0)
            {
                //Jimmy: for res-0, we should leave it empty
                snprintf(pTrans->szTrans, nLenLmt, "%s", pText);
            }
            // TRACEX("%d \t\t %d \t\t %s\t\t %s\t\t len=>%d,%d,%d,%d\n", pTrans->nResId, pTrans->nLenLmt,
            //         pTrans->szRaw, pTrans->szTrans, 
            //         strlen(pTrans->szTrans), strlen(pText), nLenLmt, nLenLmtEx);
        }
        else
        {
            //TRACEX("----found a NULL textttt-----\n");
            continue;
        }        

        nCount++;
        if(nCount >= s_pFCGIRough->stResInfo.nResTotal)
        {
            break;
        } 
    }    

    pDict->nLoadedTextTotal = nCount;

    sqlite3_finalize(stat);
    
    return TRUE;
}

static TRANS_DICTIONARY* Find_ReuseDict(FCGI_ROUGH_DATA* pFCGIRough)
{
    int n, m;
    BOOL bHasUsed = FALSE;
    //if failed we should use the last one
    TRANS_DICTIONARY* pDict = &pFCGIRough->stDictionery[MAX_ALLOW_WEB_USER - 1];
    for(n = 0; n < /*pFCGIRough->nLoaded_DictTotal*/MAX_ALLOW_WEB_USER; n++)
    {     
        bHasUsed = FALSE;
        for(m = 0; m < pFCGIRough->nActiveUserTotal; m++)
        {
            if(pFCGIRough->pCurrSess == (&pFCGIRough->stSessionInfos[m]))
            {
                continue;
            }
            if(pFCGIRough->stSessionInfos[m].pCurrDict == &pFCGIRough->stDictionery[n])
            {
                bHasUsed = TRUE;
                break;
            }
        }
        if(!bHasUsed)
        {
            pDict = &pFCGIRough->stDictionery[n];
            break;
        }
    }

    return pDict;
}

TRANS_DICTIONARY* Load_LangResources(FCGI_ROUGH_DATA* pFCGIRough, int nLangId)
{
    if(pFCGIRough->nLoaded_DictTotal < 1)
    {
        if(!Init_AllLangInfo(pFCGIRough))
        {
            return NULL;
        }
    }

    TRACEX("------now load lang ->%d-----\n", nLangId);

    int n;
    for(n = 0; n < pFCGIRough->nLoaded_DictTotal; n++)
    {
        if((pFCGIRough->stDictionery[n].nLoadedTextTotal > 0) && 
            (pFCGIRough->stDictionery[n].nLangId == nLangId))
        {
            //already there!
            TRACEX("------lang(%d) already there-----\n", nLangId);
            return &pFCGIRough->stDictionery[n];
        }
    }

    //if all users are logged in, we need consider it carefully!!!
    TRANS_DICTIONARY* pDict;
    if(pFCGIRough->nLoaded_DictTotal == MAX_ALLOW_WEB_USER)
    {
        pDict = Find_ReuseDict(pFCGIRough);

        //just delete old strings for all TRANS
        int nn;
        for(nn = 0; nn < pDict->nLoadedTextTotal; nn++)
        {
            DELETE(pDict->pTransTexts[nn].szRaw);
            DELETE(pDict->pTransTexts[nn].szTrans);
        }
        
        pDict->nLangId = nLangId;
        TRACEX("----now reuse the last LANGRES----\n");
        //now parse all the resources
        if(!Process_DB_OP(WEB_DB_TYPE_SYSTEM, Load_Resource_ByLangId, pDict))
        {
            return NULL;
        }
    }
    else
    {
        pDict = &pFCGIRough->stDictionery[pFCGIRough->nLoaded_DictTotal];
        pDict->nLangId = nLangId;
        //now create dict
        pDict->pTransTexts = NEW(TRANS_TEXT, pFCGIRough->stResInfo.nResTotal);
        if(pDict->pTransTexts == NULL)
        {
            return NULL;
        }
        //now parse all the resources
        if(!Process_DB_OP(WEB_DB_TYPE_SYSTEM, Load_Resource_ByLangId, pDict))
        {
            return NULL;
        }
        
        pFCGIRough->nLoaded_DictTotal++;
        TRACEX("------Total(%d) lang loaded!-----\n", pFCGIRough->nLoaded_DictTotal);
    }   

    return pDict;
}

static TRANS_DICTIONARY* FindDict_ByLangId(int nLang)
{
    int n;
    for(n = 0; n < s_pFCGIRough->nLoaded_DictTotal; n++)
    {
        if(nLang == s_pFCGIRough->stDictionery[n].nLangId)
        {
            return &s_pFCGIRough->stDictionery[n];
        }
    }

    return NULL;
}

char* LoadLangText_ById(int nResId)
{
    int m;
    char* szText = s_pFCGIRough->szUnkownRes;
    TRANS_DICTIONARY* pDict = NULL;// = &s_pFCGIRough->stDictionery[0];
    if(s_pFCGIRough->pCurrSess != NULL)
    {
        //TRACEX("---Now get from pCurrDict---\n");
        pDict = s_pFCGIRough->pCurrSess->pCurrDict;         
    }  
    else 
    {
        int nLang = s_pFCGIRough->nInitLangSel;
        if((nLang < 0) && (nLang >= LANG_TYPE_MAX))
        {
            nLang = LANG_TYPE_ENGLISH;
            //pDict = &s_pFCGIRough->stDictionery[nLang];
        }

        pDict = FindDict_ByLangId(nLang);
        // TRACEX("---Now get nInitLangSel->%d, nLang->%d, TotalLang = %d---\n", 
        //     s_pFCGIRough->nInitLangSel, nLang, s_pFCGIRough->nLoaded_DictTotal);
    }

    if(pDict != NULL)
    {
        //TRACEX("---Now dictTotalText->%d---\n", pDict->nLoadedTextTotal);
        for(m = 0; m < pDict->nLoadedTextTotal; m++)
        {
            if(pDict->pTransTexts[m].nResId == nResId)
            {
                if(nResId == 0)
                {
                    TRACEX("----the resId0 trans=>[%s], len=>%d----\n", 
                        pDict->pTransTexts[m].szTrans, strlen(pDict->pTransTexts[m].szTrans));
                }
                return pDict->pTransTexts[m].szTrans;
            }
        }   
    }

    return szText;
}

void Clean_DataExchange(FCGI_ROUGH_DATA* pFCGIRough)
{
    sqlite3_shutdown();
}

static void Set_IPCMode(BOOL bPushPull)
{
	HANDLE hPort = s_pFCGIRough->hIPC_CommPort;
	int nMode = bPushPull ? IPC_MODE_PUSH_PULL : IPC_MODE_REQ_REP;
	Web_IPC_CommControl(hPort, COMM_SET_IPC_MODE, &nMode, 4);
}

static void IPC_Send_AReq_2Reporter(WEB_REQTYPE emReqType, BYTE* pbyData, int nDataLen)
{
    //now make header for reply frame
    VAR_VALUE varV;
    int n;
	varV.ulValue = emReqType;      
	for(n = 0; n < nDataLen; n++)
	{
		varV.ulValue += pbyData[n];
	}
	TRACEX("---Now send out a Rep.DataLen = %d, and emReqType = %d, chkSum->%d.----\n", 
        nDataLen + 1, emReqType, varV.ulValue);
    memset(s_pFCGIRough->byReqOut_Buffer, 0, MAX_WEB_REQDATA_SIZE);
	//header
	sprintf(s_pFCGIRough->byReqOut_Buffer, "W-REQ:");
	memcpy(s_pFCGIRough->byReqOut_Buffer + 6, varV.abyValue, 4);
	s_pFCGIRough->byReqOut_Buffer[10] = ':';
	varV.ulValue = nDataLen + 1;
	memcpy(s_pFCGIRough->byReqOut_Buffer + 11, varV.abyValue, 4);
	s_pFCGIRough->byReqOut_Buffer[15] = ':';
    s_pFCGIRough->byReqOut_Buffer[16] = (BYTE)emReqType;
    for(n = 0; n < nDataLen; n++)
	{
		s_pFCGIRough->byReqOut_Buffer[17 + n] = pbyData[n];
	}

    int nWriteBytes = WEB_IPC_HEADER_LEN + nDataLen + 1;
    Set_IPCMode(FALSE);
    int nWrrr = Web_IPC_CommWrite(s_pFCGIRough->hIPC_CommPort, 
        (char*)s_pFCGIRough->byReqOut_Buffer, nWriteBytes);
    
    TRACEX("----Write %d-Bytes into IPC reporter side, toBewwitr->%d----\n", nWrrr, nWriteBytes);
}

static BOOL CheckFrame_Chkum(BYTE* pData, int nSize, ULONG ulChkSum)
{
	ULONG ulCalcChk = 0;
	int n;
	for( n = 0; n < nSize; n++)
	{
		ulCalcChk += pData[n];
	}
	if(ulCalcChk != ulChkSum)
	{
		return FALSE;
	}
	return TRUE;
}

static BOOL Read_andParse_SecondData(WEB_REQTYPE emReqType, int nTimeOutMSec)
{	
    char *pField;
	VAR_VALUE varV;
	int ulChkSun =0;
	int ulFrameSize = 0;
    BYTE byParsedReqType, byParsedRTN;
    memset(s_pFCGIRough->byRepIn_Buffer, 0 , MAX_WEB_REPDATA_SIZE);
    BOOL bFoundReply = FALSE;
    do
    {
        s_pFCGIRough->nReadBytes = Web_IPC_CommRead(s_pFCGIRough->hIPC_CommPort, 
            (char*)s_pFCGIRough->byRepIn_Buffer, MAX_WEB_REPDATA_SIZE);
        if(s_pFCGIRough->nReadBytes > 0)
        {		
            bFoundReply = TRUE;
            break;
        }
        nTimeOutMSec -= WEB_IPC_RW_TIMEOUT_MSEC;
    }while (nTimeOutMSec > 0);

    if(!bFoundReply)
    {
        TRACEI("----NO Rep from reporter side, %d, %d----\n", 
            s_pFCGIRough->nReadBytes, nTimeOutMSec);
        return FALSE;
    }

    //now parse the reply	
    char* szBuffer = s_pFCGIRough->byRepIn_Buffer;
	int nReadBytes = s_pFCGIRough->nReadBytes;
    // printf("----Read_andParse_SecondData readBytes=%d----\n", nReadBytes);
	if(nReadBytes > 0)
	{
        pField = szBuffer;
		memcpy(varV.abyValue, pField, 4);
		ulChkSun = varV.ulValue;
		
		pField += 4;
		memcpy(varV.abyValue, pField, 4);
		ulFrameSize = varV.ulValue;

        // printf("........ulChkSun = %d, ulFrameSize = %d........\n", ulChkSun, ulFrameSize);
        if(ulFrameSize + 8 > nReadBytes)
		{
			TRACEI("frame len not correct! ulFramLen = %d\n", ulFrameSize);
			return FALSE;
		}

        if(!CheckFrame_Chkum((BYTE*)(szBuffer + 8), ulFrameSize, ulChkSun))
		{
			TRACEI("frame chksum not correct!\n");
			return FALSE;
		}
        s_pFCGIRough->nParsed_InDataLen = ulFrameSize;
		s_pFCGIRough->pbyParsed_InDataPos = szBuffer + 8;
        return TRUE;	
	}    
	
    return FALSE;
}

static BOOL Read_andParse_WebIPC_Rep(WEB_REQTYPE emReqType, int nTimeOutMSec)
{	
    char *pField;
	VAR_VALUE varV;
	int ulChkSun =0;
	int ulFrameSize = 0;
    BYTE byParsedReqType, byParsedRTN;
    //get req from WEB front
	//ZERO_POBJS(s_pFCGIRough->byRepIn_Buffer, MAX_WEB_REPDATA_SIZE);
    memset(s_pFCGIRough->byRepIn_Buffer, 0 , MAX_WEB_REPDATA_SIZE);
    BOOL bFoundReply = FALSE;
    do
    {
        s_pFCGIRough->nReadBytes = Web_IPC_CommRead(s_pFCGIRough->hIPC_CommPort, 
            (char*)s_pFCGIRough->byRepIn_Buffer, MAX_WEB_REPDATA_SIZE);
        if(s_pFCGIRough->nReadBytes > WEB_IPC_HEADER_LEN)
        {		
            bFoundReply = TRUE;
            break;
        }
        nTimeOutMSec -= WEB_IPC_RW_TIMEOUT_MSEC;
        //the com_ipc do NOT block the reading
        //Sleep_Ex(WEB_IPC_RW_TIMEOUT_MSEC);

    }while (nTimeOutMSec > 0);

    if(!bFoundReply)
    {
        TRACEX("----NO Rep from reporter side, %d, %d----\n", 
            s_pFCGIRough->nReadBytes, nTimeOutMSec);
        return FALSE;
    }

    //now parse the reply	
    char* szBuffer = s_pFCGIRough->byRepIn_Buffer;
	int nReadBytes = s_pFCGIRough->nReadBytes;
    //TRACEX("req head is [%s], nReadBytes:%d\n", szBuffer, nReadBytes);
	while(nReadBytes > 0)
	{
		if(strncmp(szBuffer, "W-REP:", 6) != 0)
		{			
			szBuffer +=  1;
			nReadBytes -= 1;
			continue;
		}
		//TRACEX("szBuff = %08X-%08X-%08X-%08X-%08X.\n", *(ULONG*)szBuffer, *(ULONG*)(szBuffer + 4),
		//	*(ULONG*)(szBuffer + 8), *(ULONG*)(szBuffer + 12), *(ULONG*)(szBuffer + 16));
		//szBuffer = SplitString(szBuffer, &pField, COLON);
		pField = szBuffer + 6;
		memcpy(varV.abyValue, pField, 4);
		ulChkSun = varV.ulValue;
		//szBuffer = SplitString(szBuffer, &pField, COLON);
		pField += 5;
		memcpy(varV.abyValue, pField, 4);
		ulFrameSize = varV.ulValue;
		if(ulFrameSize + 16 > nReadBytes)
		{
			TRACEX("frame len not correct! ulFramLen = %d\n", ulFrameSize);
			break;
		}
		
		if(!CheckFrame_Chkum((BYTE*)(szBuffer + 16), ulFrameSize, ulChkSun))
		{
			TRACEX("frame chksum not correct!\n");
			break;
		}
		pField += 5;
		szBuffer = pField;
		//parsed OK, now handle req-data
		byParsedReqType = *(BYTE*)szBuffer;
        byParsedRTN = *(BYTE*)(szBuffer + 1);
		szBuffer += 2;
		ulFrameSize -= 2;

		if(byParsedReqType != emReqType)
		{
            TRACEX("---DFFF on req -> [%d, %d]---\n", byParsedReqType, emReqType);
			return FALSE;
		}
        TRACEX("----now parsed OK for req->%d, rtn->%d, frmDatasize->%d----\n", 
            emReqType, byParsedRTN, ulFrameSize);
        s_pFCGIRough->byParsedRTN = byParsedRTN;
		s_pFCGIRough->pbyParsed_InDataPos = szBuffer;
		s_pFCGIRough->nParsed_InDataLen = ulFrameSize;	
        if(((s_pFCGIRough->byParsedRTN == WEB_ERRCODE_OK) && 
            (emReqType < WEB_REQTYPE_SET_PARAM_SERETCTRL)) ||
            (emReqType >= WEB_REQTYPE_SET_PARAM_SERETCTRL))
        {
            return TRUE;
        }		
	}    
	
    return FALSE;
}

static BOOL IPC_GetVersionInfos(void)
{
    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_VERSION_IINFO, NULL, 0);

    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_VERSION_IINFO, 500))
    {
        if(s_pFCGIRough->nParsed_InDataLen >= sizeof(WEB_VERSION_INFO))
        {
            memcpy(&s_pFCGIRough->chargerDataRepo.versionInfos, 
                s_pFCGIRough->pbyParsed_InDataPos, sizeof(WEB_VERSION_INFO));
            TRACEX("----Now get the IMMU2-ver->[%s, %s], IMSU-D Ver->[%s], Recfier->[%s, %s]----\n", 
                s_pFCGIRough->chargerDataRepo.versionInfos.szIMMU2_SWVer,
                s_pFCGIRough->chargerDataRepo.versionInfos.szIMMU2_HWVer,
                s_pFCGIRough->chargerDataRepo.versionInfos.szIMSU_D_Vers,
                s_pFCGIRough->chargerDataRepo.versionInfos.szRect_Ver_DC,
                s_pFCGIRough->chargerDataRepo.versionInfos.szRect_Ver_PFC);
            bParseOK = TRUE;
        }
    }

    return bParseOK;
}

static BOOL IPC_GetAllDevInfos(void)
{
    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_ALL_DEVINFO, NULL, 0);
    
    VAR_VALUE varV;
    int nDevTotal = 0;
    int nDevInfoItemSize = sizeof(WEB_DEVICE_INFO);
    int n;
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_ALL_DEVINFO, 500))
    {
        int nDataLen = s_pFCGIRough->nParsed_InDataLen;
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        if(s_pFCGIRough->nParsed_InDataLen >= (sizeof(WEB_DEVICE_INFO) + 4))
        {
            memcpy(varV.abyValue, pDataPos, 4);
            pDataPos += 4;
            nDataLen -= 4;
            nDevTotal = varV.ulValue;
            TRACEX("------Get the devTotal -> (%d)------\n", nDevTotal);
            if(nDevTotal < 1 || (nDevTotal > 65535))
            {                
                return FALSE;
            }
            s_pFCGIRough->chargerDataRepo.nDevInfoTotal = nDevTotal;
            s_pFCGIRough->chargerDataRepo.pAllDevInfo = NEW(WEB_DEVICE_INFO, nDevTotal);
            s_pFCGIRough->chargerDataRepo.pAllRunDevs = NEW(WEB_RUN_DEV, nDevTotal);
            if((s_pFCGIRough->chargerDataRepo.pAllDevInfo == NULL) ||
                (s_pFCGIRough->chargerDataRepo.pAllRunDevs == NULL))
            {
                return FALSE;
            }
            //now copy all the devInfos
            for(n = 0; ((n < nDevTotal) && (nDataLen > 0)); n++)
            {
                memcpy(&s_pFCGIRough->chargerDataRepo.pAllDevInfo[n], pDataPos, nDevInfoItemSize);
                TRACEX("------Get the single devInfo[%d, %d, %s]------\n", 
                    s_pFCGIRough->chargerDataRepo.pAllDevInfo[n].nDevId,
                    s_pFCGIRough->chargerDataRepo.pAllDevInfo[n].nDevName_ResId,
                    LoadLangText_ById(s_pFCGIRough->chargerDataRepo.pAllDevInfo[n].nDevName_ResId));
                pDataPos += nDevInfoItemSize;
                nDataLen -= nDevInfoItemSize;
            }
            if(nDataLen != 0)
            {
                TRACEX("------warning, the devInfo data or the parsing is NOT correct-----\n");
            }
            bParseOK = TRUE;
        }
    }

    return bParseOK;
}

static BOOL IPC_GetAllParamInfos(void)
{
    BOOL bParseOK = FALSE; 
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_ALL_PARAMINFO_1, NULL, 0); //第一次发送数据，先获取一段数据
    if(!Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_ALL_PARAMINFO, 1000))
    {
        
        return FALSE;
    }
    // printf("----Read first data OK, len=%d----\n", s_pFCGIRough->nParsed_InDataLen);
    //先获取第一部分数据，并且对数据进行效验后立刻获取第二部分数据
    // s_pFCGIRough->byParsedRTN = byParsedRTN;
    // s_pFCGIRough->pbyParsed_InDataPos = szBuffer;
    // s_pFCGIRough->nParsed_InDataLen = ulFrameSize;	
    BYTE by_Buffer[MAX_WEB_REPDATA_SIZE];
    memset(by_Buffer, 0, MAX_WEB_REPDATA_SIZE);
    memcpy(by_Buffer, s_pFCGIRough->pbyParsed_InDataPos, s_pFCGIRough->nParsed_InDataLen);
    int nFirstDataLen = s_pFCGIRough->nParsed_InDataLen;

    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_ALL_PARAMINFO_2, NULL, 0); //第二次发送数据，获取剩余数据
    if(!Read_andParse_SecondData(WEB_REQTYPE_GET_ALL_PARAMINFO_2, 1000))
    {
        
        return FALSE;
    }
    memcpy(by_Buffer + nFirstDataLen, s_pFCGIRough->pbyParsed_InDataPos, s_pFCGIRough->nParsed_InDataLen);
    s_pFCGIRough->pbyParsed_InDataPos = by_Buffer;
    s_pFCGIRough->nParsed_InDataLen = nFirstDataLen + s_pFCGIRough->nParsed_InDataLen;

    VAR_VALUE varV;    
    int nParamTotal = 0;
    int nParamInfoItemSize = sizeof(WEB_PARAM_INFO);
    int n;
    // if(Read_andParse_SecondData(WEB_REQTYPE_GET_ALL_PARAMINFO_2, 1000))
    // {
        int nDataLen = s_pFCGIRough->nParsed_InDataLen;
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        if(s_pFCGIRough->nParsed_InDataLen >= (sizeof(WEB_PARAM_INFO) + 4))
        {
            memcpy(varV.abyValue, pDataPos, 4);
            pDataPos += 4;
            nDataLen -= 4;
            nParamTotal = varV.ulValue;
            if(nParamTotal < 1 || (nParamTotal > 65535))
            {
                return FALSE;
            }
            TRACEX("---Now get paramTotal->%d, nDataLen->%d---\n", nParamTotal, nDataLen);
            s_pFCGIRough->chargerDataRepo.nParamInfosTotal = nParamTotal;
            s_pFCGIRough->chargerDataRepo.pAllParamInfo = NEW(WEB_PARAM_INFO, nParamTotal);
            if(s_pFCGIRough->chargerDataRepo.pAllParamInfo == NULL)
            {
                return FALSE;
            }
            //now copy all the devInfos
            for(n = 0; ((n < nParamTotal) && (nDataLen > 0)); n++)
            {
                WEB_PARAM_INFO* pPar = &s_pFCGIRough->chargerDataRepo.pAllParamInfo[n];
                memcpy(pPar, pDataPos, nParamInfoItemSize);
                pDataPos += nParamInfoItemSize;
                nDataLen -= nParamInfoItemSize;
                TRACEX("Get %s_param, id->%d, devModel->%d, name->%d,%s, unit->%d,%s, uAuthMask->%d---\n",
                    (pPar->emParamType == PARAM_TYPE_STATUS) ? "STATUS" :
                    ((pPar->emParamType == PARAM_TYPE_CONTROL) ? "CTRL" : "SETT"),
                    pPar->nParamId, pPar->nDevModelId,
                    pPar->nParamName_ResId, LoadLangText_ById(pPar->nParamName_ResId),
                    pPar->nParamUnit_ResId, LoadLangText_ById(pPar->nParamUnit_ResId),
                    pPar->uMask_AuthorityLevel);
            }
            if(nDataLen != 0)
            {
                TRACEX("------warning, the parmInfo data or the parsing is NOT correct-----\n");
            }
            bParseOK = TRUE;
        }
    // }

    return bParseOK;
}

static BOOL IPC_GetAllAlarmInfos(void)
{
    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_ALL_ALARMINFO, NULL, 0);    
    VAR_VALUE varV;
    
    int nAlarmTotal = 0;
    int nAlmInfoItemSize = sizeof(WEB_ALARM_INFO);
    int n;
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_ALL_ALARMINFO, 500))
    {
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = s_pFCGIRough->nParsed_InDataLen;
        if(s_pFCGIRough->nParsed_InDataLen >= (sizeof(WEB_ALARM_INFO) + 4))
        {
            memcpy(varV.abyValue, pDataPos, 4);
            pDataPos += 4;
            nDataLen -= 4;
            nAlarmTotal = varV.ulValue;
            if(nAlarmTotal < 1 || (nAlarmTotal > 65535))
            {
                return FALSE;
            }
            TRACEX("---Now get alarm total->%d---\n", nAlarmTotal);
            s_pFCGIRough->chargerDataRepo.nAlarmInfosTotal = nAlarmTotal;
            s_pFCGIRough->chargerDataRepo.pAllAlarmInfo = NEW(WEB_ALARM_INFO, nAlarmTotal);
            if(s_pFCGIRough->chargerDataRepo.pAllAlarmInfo == NULL)
            {
                return FALSE;
            }
            //now copy all the devInfos
            for(n = 0; ((n < nAlarmTotal) && (nDataLen > 0)); n++)
            {
                memcpy(&s_pFCGIRough->chargerDataRepo.pAllAlarmInfo[n], pDataPos, nAlmInfoItemSize);
                pDataPos += nAlmInfoItemSize;
                nDataLen -= nAlmInfoItemSize;
                TRACEX("---Now get the alarm[%d, %s, %d]---\n",
                    s_pFCGIRough->chargerDataRepo.pAllAlarmInfo[n].nAlarmId,
                    LoadLangText_ById(s_pFCGIRough->chargerDataRepo.pAllAlarmInfo[n].nAlmName_ResId),
                    s_pFCGIRough->chargerDataRepo.pAllAlarmInfo[n].emLevel);
            }
            if(nDataLen != 0)
            {
                TRACEX("------warning, the alarmInfo data or the parsing is NOT correct-----\n");
            }
            bParseOK = TRUE;
        }
    }

    return bParseOK;
}

static int Find_ParamTypeTotal_ByDevModelId_andParamType(PARAM_TYPE tp, int nDevModelId)
{
    int n;
    int nParamTotal = s_pFCGIRough->chargerDataRepo.nParamInfosTotal;
    WEB_PARAM_INFO* pParamInfo = s_pFCGIRough->chargerDataRepo.pAllParamInfo;
    int nFoundTotal = 0;
    for(n = 0; n < nParamTotal; n++, pParamInfo++)
    {
        if((pParamInfo->emParamType == tp) && (pParamInfo->nDevModelId == nDevModelId))
        {
            nFoundTotal += 1;
        }
    }

    return nFoundTotal;
}

static int Find_ParamTypeTotal_Webid(PARAM_TYPE tp, int Main_idx, int Sub_idx)
{
    int n;
    int nParamTotal = s_pFCGIRough->chargerDataRepo.nParamInfosTotal;
    WEB_PARAM_INFO* pParamInfo = s_pFCGIRough->chargerDataRepo.pAllParamInfo;
    
    int nFoundTotal = 0;
    for(n = 0; n < nParamTotal; n++, pParamInfo++)
    {   
        if((pParamInfo->emParamType == tp))
        {
            if((pParamInfo->emParamType == PARAM_TYPE_STATUS) && (pParamInfo->Web_1 == s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[Main_idx].main_id) &&
               (pParamInfo->Web_2 == s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[Main_idx].SubDispList[Sub_idx].Sub_id))
            {

                 nFoundTotal += 1;
            }
            else if((pParamInfo->emParamType == PARAM_TYPE_SETTING || pParamInfo->emParamType == PARAM_TYPE_CONTROL) && (pParamInfo->Web_1 == s_pFCGIRough->chargerDataRepo.ParaSetDispList1[Main_idx].main_id) &&
               (pParamInfo->Web_2 == s_pFCGIRough->chargerDataRepo.ParaSetDispList1[Main_idx].SubDispList[Sub_idx].Sub_id))
            {
                nFoundTotal += 1;
            }
        }
    }
    return nFoundTotal;
}

static void Make_String_OrBlobParam_4RunDev(WEB_PARAM_VALUE* pParMatched)
{
    WEB_PARAM_INFO* pParamInfo = pParMatched->pParamInfo;
    TRACEX("---New %s_param, blobId->%d, valSize->%d---\n", 
        (pParamInfo->emValType == WEB_VALTYPE_STRING) ? "string" :
        ((pParamInfo->emValType == WEB_VALTYPE_PASSWORD) ? "passwd" : "object"),
        pParamInfo->nBlobId, pParMatched->pParamInfo->nValueSize);
    
    pParMatched->currValue.blobValue = NEW(BYTE, pParMatched->pParamInfo->nValueSize);
    pParMatched->nBlobSize = pParMatched->pParamInfo->nValueSize;
    //now we need create BlobElements for Blob
    if(pParamInfo->emValType == WEB_VALTYPE_OBJECT)
    {
        A_WEBBLOB_PARAM* pBlobParam = &s_pFCGIRough->blobParamList[s_pFCGIRough->nBlobParamTotal];
        if(Create_AWebBlob_Param(pBlobParam, pParMatched, pParMatched->currValue.blobValue))
        {                            
            s_pFCGIRough->nBlobParamTotal++;
            TRACE("---Now crerate Blob[%d] successfully, Total[%d]---\n", 
                    pParamInfo->nBlobId, s_pFCGIRough->nBlobParamTotal);
        }
        else
        {
            TRACEX("---Now crerate Blob[%d] FAILED---\n", pParamInfo->nBlobId);
        }                        
    }
}

static void Sort_ParamList_ByPrio_and_MakeBlob1(WEB_PARAM_VALUE* pSrcList, int nSrcListCount)
{
    int m, n;
    
    WEB_PARAM_VALUE pTmp;
    for(m = 0; m < nSrcListCount - 1; m++)
    {
        for(n = m + 1; n < nSrcListCount; n++)
        {
            if((pSrcList[n].pParamInfo->nDisplayPriority < pSrcList[m].pParamInfo->nDisplayPriority) ||
                ((pSrcList[n].pParamInfo->nDisplayPriority == pSrcList[m].pParamInfo->nDisplayPriority) &&
                (pSrcList[n].pParamInfo->nParamId < pSrcList[m].pParamInfo->nParamId)))
            {
                pTmp = pSrcList[m];
                pSrcList[m] = pSrcList[n];
                pSrcList[n] = pTmp;
            }
        }
    }

    //now rebuild the blob
    WEB_PARAM_VALUE* pParMatched = pSrcList;
    for(m = 0; m < nSrcListCount; m++, pParMatched++)
    {
        //for Blob & String, we need create the memory for it
        if(pParMatched->pParamInfo->emValType >= WEB_VALTYPE_OBJECT)
        {
            Make_String_OrBlobParam_4RunDev(pParMatched);                
        }
    }
}

static void Sort_ParamList_ByPrio_and_MakeBlob(WEB_PARAM_VALUE* pSrcList, int nSrcListCount)
{
    int m, n;
    
    WEB_PARAM_INFO* pTmp;
    for(m = 0; m < nSrcListCount - 1; m++)
    {
        for(n = m + 1; n < nSrcListCount; n++)
        {
            if((pSrcList[n].pParamInfo->nDisplayPriority < pSrcList[m].pParamInfo->nDisplayPriority) ||
                ((pSrcList[n].pParamInfo->nDisplayPriority == pSrcList[m].pParamInfo->nDisplayPriority) &&
                (pSrcList[n].pParamInfo->nParamId < pSrcList[m].pParamInfo->nParamId)))
            {
                pTmp = pSrcList[m].pParamInfo;
                pSrcList[m].pParamInfo = pSrcList[n].pParamInfo;
                pSrcList[n].pParamInfo = pTmp;
            }
        }
    }
    //now rebuild the blob
    WEB_PARAM_VALUE* pParMatched = pSrcList;
    for(m = 0; m < nSrcListCount; m++, pParMatched++)
    {
        //for Blob & String, we need create the memory for it
        if(pParMatched->pParamInfo->emValType >= WEB_VALTYPE_OBJECT)
        {
            Make_String_OrBlobParam_4RunDev(pParMatched);                
        }
    }
}


//DEFINE_DEV_MODEL
static int Find_BelongDev(int Main_idx, int Sub_idx, UINT DEV_mod, PARAM_TYPE type)
{
    switch(DEV_mod)
    {
        case DEV_MODEL_ESS:
            return 1;
            break;
        case DEV_MODEL_CCU:
            return 200;
            break;
        case DEV_MODEL_GPRDATA:
            return 211;
            break;
        case DEV_MODEL_SIGDATA:
            if(type == PARAM_TYPE_STATUS)
            {
                if(Main_idx == 3) //PCS
                {
                    if(Sub_idx == 0)
                    {
                        return 275;
                    }
                    else
                    {
                        return 251 + Sub_idx - 1;
                    }
                }
                else if(Main_idx == 4) //MPPT
                {
                    return 251 + Sub_idx;
                } 
            }
            else
            {
                return 251;
            }
            break;
        case DEV_MODEL_METER:
            if (Sub_idx <= 10)      //0-10 Rough
            {
                return 401 + Sub_idx;
            }
            else if (Sub_idx <= 13 && Sub_idx >= 11)    //11-13 Cab 
            {
                return 419 + Sub_idx;
            }
            else    //14-16 Grid
            {
                return 426 + Sub_idx;
            }
            break;
        case DEV_MODEL_BAT_GRP:
            return 500;
            break;
        case DEV_MODEL_BAT:
            return 501 + Sub_idx - 1;
            break;
        case DEV_MODEL_BAT_EU3:
            if((type == PARAM_TYPE_STATUS) && (Main_idx == 0))
            {
                return 601 + Sub_idx - 6;
            }
            else if((Main_idx == 6) && (Sub_idx != 0))
            {
                return 601 + Sub_idx - 1;
            }
            else
            {
                return 601;
            }
            break;
        case DEV_MODEL_POWER_EU3:
            if((type == PARAM_TYPE_STATUS) && (Main_idx == 0))
            {
                return 611 + Sub_idx - 4;
            }
            else if((type == PARAM_TYPE_SETTING) && (Main_idx == 0))
            {
                return 611 + Sub_idx - 6;
            }
            else
            {
                return 611;
            }
            break;
        case DEV_MODEL_MULCUBE_EU3:
            if((type == PARAM_TYPE_STATUS) && (Main_idx == 0) && ((Sub_idx == 10)))//edit by Kuangyue 20251130
            {
                return 621;
            }
            else if((type == PARAM_TYPE_SETTING) && (Main_idx == 0) && ((Sub_idx == 9)))
            {
                return 621;
            }
            break;
        case DEV_MODEL_PCS:
            // if((type == PARAM_TYPE_SETTING)&&(Sub_idx == 1))//ABU设备地址
            // {
            //     return 705;
            // }
            // else
            // {
                return 701;
            // }
            break;
        case DEV_MODEL_METER_VAL:
            return 801;
            break;
        case DEV_MODEL_UART_SERVER:
            return 901;
            break;
        case DEV_MODEL_TEMP_CONT:
            return 911;
            break;
        case DEV_MODEL_AIR_CONDI:
                if((951 + Sub_idx - 1) >= 953)
                {
                    return 951;
                }
                return 951 + Sub_idx - 1;
            break;
        case DEV_MODEL_PACK:
            if(type == PARAM_TYPE_STATUS)
            {
                return 981 + Sub_idx - 5;
            }
            break;
        case DEV_MODEL_AEMS:
            return 991;
            break;
        case DEV_MODEL_SEMS:
            return 1001;
            break;
        case DEV_MODEL_WATER_CHILLER:   // NJX v5.3: chiller devId=961 (CHILLER_DEV_ID in auxdev_sampler_main.h)
            return 961;
            break;
        case DEV_MODEL_PV_INVT:
            if((type == PARAM_TYPE_SETTING) && (Sub_idx <= 10) && (Main_idx == 4))
            {
                return 1050 + Sub_idx;
            }
            else if((type == PARAM_TYPE_STATUS) && (Sub_idx >= 24))
            {
                return 1050 + Sub_idx - 24;
            } 
            return 1050;
            break;
        
    }
    
    return -1;
}


static BOOL Init_CreateStructs_4WebStatic(void)
{
    int m, n;
    int nDevTotal = s_pFCGIRough->chargerDataRepo.nDevInfoTotal;
    WEB_DEVICE_INFO* pDevInfo = s_pFCGIRough->chargerDataRepo.pAllDevInfo;
    WEB_RUN_DEV* pDevRun = s_pFCGIRough->chargerDataRepo.pAllRunDevs;
    int nParamTotal = s_pFCGIRough->chargerDataRepo.nParamInfosTotal;
    WEB_PARAM_INFO* pParamInfo = s_pFCGIRough->chargerDataRepo.pAllParamInfo;
    UINT nFound_StatusParam = 0;
    UINT nFound_SettParam = 0;
    UINT nFound_CtrlParam = 0;
    s_pFCGIRough->nBlobParamTotal = 0;
    A_WEBBLOB_PARAM* pBlobParam = &s_pFCGIRough->blobParamList[0];

    s_pFCGIRough->chargerDataRepo.pSettParList = (WEB_PARAM_VALUE***)malloc(sizeof(WEB_PARAM_VALUE**) * s_pFCGIRough->chargerDataRepo.Setnum);
    s_pFCGIRough->chargerDataRepo.pstatusParList = (WEB_PARAM_VALUE***)malloc(sizeof(WEB_PARAM_VALUE**) * s_pFCGIRough->chargerDataRepo.Stanum);

    int k = 0;
    for(k = 0; k < s_pFCGIRough->chargerDataRepo.Setnum; ++k)
    {
        s_pFCGIRough->chargerDataRepo.pSettParList[k] = (WEB_PARAM_VALUE**)malloc(sizeof(WEB_PARAM_VALUE*) * s_pFCGIRough->chargerDataRepo.ParaSetDispList1[k].nSubTotal);
    }
    for(k = 0; k < s_pFCGIRough->chargerDataRepo.Stanum; ++k)
    {
        s_pFCGIRough->chargerDataRepo.pstatusParList[k] = (WEB_PARAM_VALUE**)malloc(sizeof(WEB_PARAM_VALUE*) * s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[k].nSubTotal);
    }

    
    int i = 0, j = 0;
    for(i = 0; i < s_pFCGIRough->chargerDataRepo.Setnum; ++i)
    {
        for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal; ++j)
        {
            int sub_idx = j;
            switch(i)
            {
                case 2:
                    if(j >= 1 && j <=4)
                    {
                        sub_idx = 1;
                    }
                    break;
                case 4:
                    sub_idx = 0;
                    break;                    
                case 5:
                    sub_idx = 0;
                    break;
                case 6:
                    if(j >= 1 && j <= 4)
                    {
                        sub_idx = 1;
                    }
                    break;
            } 

            s_pFCGIRough->chargerDataRepo.nParamTotal[i][j] = Find_ParamTypeTotal_Webid(PARAM_TYPE_SETTING, i, sub_idx);
            s_pFCGIRough->chargerDataRepo.nParamTotal[i][j] += Find_ParamTypeTotal_Webid(PARAM_TYPE_CONTROL, i, sub_idx);

            if(s_pFCGIRough->chargerDataRepo.nParamTotal[i][j] > 0)
            {
                s_pFCGIRough->chargerDataRepo.pSettParList[i][j] = NEW(WEB_PARAM_VALUE, s_pFCGIRough->chargerDataRepo.nParamTotal[i][j]);
                ZERO_POBJS(s_pFCGIRough->chargerDataRepo.pSettParList[i][j], s_pFCGIRough->chargerDataRepo.nParamTotal[i][j]);
            }
            else
            {
                s_pFCGIRough->chargerDataRepo.pSettParList[i][j] = NULL;
            }
            pParamInfo = s_pFCGIRough->chargerDataRepo.pAllParamInfo;
            nFound_SettParam = 0;

            for(n = 0; n < nParamTotal; n++, pParamInfo++)
            {
                int Main_idx = s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].main_id;
                int Sub_idx = s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].SubDispList[j].Sub_id;  
                
                switch(Main_idx)
                {
                    case 3:
                        if((j >= 1) && (j <= 4))
                        {
                            Sub_idx = 42;
                        }
                        break;
                    case 5:
                        Sub_idx = 80;
                        break;                        
                    case 6:
                        Sub_idx = 101;
                        break;
                    case 7:
                        if((j >=1) && (j <=4))
                        {
                            Sub_idx = 122;
                        }
                        break;
                }
                
                if(((pParamInfo->emParamType == PARAM_TYPE_SETTING)  || (pParamInfo->emParamType == PARAM_TYPE_CONTROL) )&& 
                    (nFound_SettParam < s_pFCGIRough->chargerDataRepo.nParamTotal[i][j]))
                {   
                    if((pParamInfo->Web_1 == Main_idx) &&
                       (pParamInfo->Web_2 == Sub_idx))
                    {
                        WEB_PARAM_VALUE* pCurSettParam = &(s_pFCGIRough->chargerDataRepo.pSettParList[i][j][nFound_SettParam]);
                        pCurSettParam->pParamInfo = pParamInfo;
                        UINT Dev_id = pParamInfo->nDevModelId;
                        pCurSettParam->nDevid = Find_BelongDev(i, j, Dev_id, PARAM_TYPE_SETTING);
                        nFound_SettParam++;                      
                    }
                } 
            }
            Sort_ParamList_ByPrio_and_MakeBlob1(s_pFCGIRough->chargerDataRepo.pSettParList[i][j], s_pFCGIRough->chargerDataRepo.nParamTotal[i][j]);
            // printf("i = %d, j = %d, nSubTotal = %d, nParamTotal = %d, nFound_SettParam = %d\n", i, j,s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal, s_pFCGIRough->chargerDataRepo.nParamTotal[i][j], nFound_SettParam);
        }
    }
    

    for(i = 0; i < s_pFCGIRough->chargerDataRepo.Stanum; ++i)
    {
        for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].nSubTotal; ++j)
        {
            int sub_idx = j;
            switch(i)
            {
                case 0:
                    if(j >= 6 && j <= 9)
                    {
                        sub_idx = 6;
                    }
                    break;
                case 2:
                    if(j >= 1 && j <=4)
                    {
                        sub_idx = 1;
                    }
                    else if(j > 4 && j <= 6)
                    {
                        sub_idx = 5;
                    }
                    break;
                case 3:
                    if(j >= 1)
                    {
                        sub_idx = 1;
                    }
                    break;
                case 4:
                    if((j >= 0) && (j <= 23))
                    {
                        sub_idx = 0;
                    }
                    else if(j >= 24)
                    {
                        sub_idx = 24;
                    }
                    break;
                case 5:
                    sub_idx = 0;
                    break;
                case 6:
                    if(j >= 1 && j <= 4)
                    {
                        sub_idx = 1;
                    }
                    break;
            }

            s_pFCGIRough->chargerDataRepo.nTotal[i][j] =  Find_ParamTypeTotal_Webid(PARAM_TYPE_STATUS, i, sub_idx);
            if(s_pFCGIRough->chargerDataRepo.nTotal[i][j] > 0)
            {
                s_pFCGIRough->chargerDataRepo.pstatusParList[i][j] = NEW(WEB_PARAM_VALUE, s_pFCGIRough->chargerDataRepo.nTotal[i][j]);
                ZERO_POBJS(s_pFCGIRough->chargerDataRepo.pstatusParList[i][j], s_pFCGIRough->chargerDataRepo.nTotal[i][j]);
            }
            else
            {
                s_pFCGIRough->chargerDataRepo.pstatusParList[i][j] = NULL;
            }
            pParamInfo = s_pFCGIRough->chargerDataRepo.pAllParamInfo;
            nFound_StatusParam = 0;
            for(n = 0; n < nParamTotal; n++, pParamInfo++)
            {
                int Main_idx = s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].main_id;
                int Sub_idx = s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].SubDispList[j].Sub_id;   
                switch(Main_idx)
                {
                    case 20:
                        if(j >= 6 && j <= 9)
                        {
                            Sub_idx = 511;
                        }
                        break;
                    case 22:
                        if((j >= 1) && (j <= 4))
                        {
                            Sub_idx = 542;
                        }
                        else if((j > 4) && (j <= 6))
                        {
                            Sub_idx = 546;
                        }
                        break;
                    case 23:
                        if(j >= 1)
                        {
                            Sub_idx = 566;
                        }
                        break;
                    case 24:
                        if((j >= 0) && (j <= 23))
                        {
                            Sub_idx = 585;
                        }
                        else if(j >= 24)
                        {
                            Sub_idx = 1018;
                        }
                        break;
                    case 25:
                        Sub_idx = 601;
                        break;
                    case 26:
                        if((j >=1) && (j <=4))
                        {
                            Sub_idx = 622;
                        }
                        break;
                }   
                if((pParamInfo->emParamType == PARAM_TYPE_STATUS) && 
                    (nFound_StatusParam < s_pFCGIRough->chargerDataRepo.nTotal[i][j]))
                {
                    if((pParamInfo->Web_1 == Main_idx) &&
                       (pParamInfo->Web_2 == Sub_idx))
                    {
                        WEB_PARAM_VALUE* pCurStatusParam = &(s_pFCGIRough->chargerDataRepo.pstatusParList[i][j][nFound_StatusParam]);
                        pCurStatusParam->pParamInfo = pParamInfo;
                        UINT Dev_id = pParamInfo->nDevModelId;
                        pCurStatusParam->nDevid = Find_BelongDev(i, j, Dev_id, PARAM_TYPE_STATUS);
                        nFound_StatusParam++;                  
                    }
         
                } 
            }
            Sort_ParamList_ByPrio_and_MakeBlob1(s_pFCGIRough->chargerDataRepo.pstatusParList[i][j], s_pFCGIRough->chargerDataRepo.nTotal[i][j]);
            // printf("i = %d, j = %d, nSubTotal, nTotal = %d, nFound_StatusParam = %d\n", i, j,s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].nSubTotal, s_pFCGIRough->chargerDataRepo.nTotal[i][j], nFound_StatusParam);
        }
    }
    

    for(m = 0; m < nDevTotal; m++, pDevInfo++, pDevRun++)
    {
        
        pDevRun->pDevInfo = pDevInfo;
        pDevRun->nStatusParamTotal = Find_ParamTypeTotal_ByDevModelId_andParamType(PARAM_TYPE_STATUS, pDevInfo->nDevModelId);
        pDevRun->nSettParamTotal = Find_ParamTypeTotal_ByDevModelId_andParamType(PARAM_TYPE_SETTING, pDevInfo->nDevModelId);
        pDevRun->nCtrlParamTotal = Find_ParamTypeTotal_ByDevModelId_andParamType(PARAM_TYPE_CONTROL, pDevInfo->nDevModelId);
        
        //now create the 3-type param val structs
        if(pDevRun->nStatusParamTotal > 0)
        {
            pDevRun->pStatusParamList = NEW(WEB_PARAM_VALUE, pDevRun->nStatusParamTotal);
            ZERO_POBJS(pDevRun->pStatusParamList, pDevRun->nStatusParamTotal);
        }
        if(pDevRun->nSettParamTotal > 0)
        {
            pDevRun->pSettParamList = NEW(WEB_PARAM_VALUE, pDevRun->nSettParamTotal);
            ZERO_POBJS(pDevRun->pSettParamList, pDevRun->nSettParamTotal);
        }
        if(pDevRun->nCtrlParamTotal > 0)
        {
            pDevRun->pCtrlParamList = NEW(WEB_PARAM_VALUE, pDevRun->nCtrlParamTotal);
            ZERO_POBJS(pDevRun->pCtrlParamList, pDevRun->nCtrlParamTotal);
        }
        //now create all the links for the param vals
        pParamInfo = s_pFCGIRough->chargerDataRepo.pAllParamInfo;
        nFound_StatusParam = 0;
        nFound_SettParam = 0;
        nFound_CtrlParam = 0;
        int kkk;
        for(n = 0; n < nParamTotal; n++, pParamInfo++)
        {
            WEB_PARAM_VALUE* pParMatched = NULL;
            if(pParamInfo->nDevModelId == pDevInfo->nDevModelId)
            {
                if((pParamInfo->emParamType == PARAM_TYPE_STATUS) && 
                    (nFound_StatusParam < pDevRun->nStatusParamTotal))
                {
                    pDevRun->pStatusParamList[nFound_StatusParam].pParamInfo = pParamInfo;
                    TRACE("--Create_StatParam[%d, %d]--\n", pDevRun->pDevInfo->nDevId, pParamInfo->nParamId);
                    pParMatched = &pDevRun->pStatusParamList[nFound_StatusParam];
                    nFound_StatusParam++;                    
                }
                else if((pParamInfo->emParamType == PARAM_TYPE_SETTING) && 
                    (nFound_SettParam < pDevRun->nSettParamTotal))
                {
                    pDevRun->pSettParamList[nFound_SettParam].pParamInfo = pParamInfo;
                    TRACE("--Create_SettParam[%d, %d]--\n", pDevRun->pDevInfo->nDevId, pParamInfo->nParamId);
                    pParMatched = &pDevRun->pSettParamList[nFound_SettParam];
                    nFound_SettParam++;                    
                }
                else if((pParamInfo->emParamType == PARAM_TYPE_CONTROL) && 
                    (nFound_CtrlParam < pDevRun->nCtrlParamTotal))
                {
                    pDevRun->pCtrlParamList[nFound_CtrlParam].pParamInfo = pParamInfo;
                    TRACE("--Create_CtrlParam[%d, %d]--\n", pDevRun->pDevInfo->nDevId, pParamInfo->nParamId);
                    pParMatched = &pDevRun->pCtrlParamList[nFound_CtrlParam];
                    nFound_CtrlParam++;                    
                }
            }
        }
        
        TRACE("--toCreadte[%d,%d,%d], find[%d,%d,%d]--\n", 
            pDevRun->nStatusParamTotal, pDevRun->nSettParamTotal, pDevRun->nCtrlParamTotal,
            nFound_StatusParam, nFound_SettParam, nFound_CtrlParam);
        Sort_ParamList_ByPrio_and_MakeBlob(pDevRun->pStatusParamList, pDevRun->nStatusParamTotal);
        Sort_ParamList_ByPrio_and_MakeBlob(pDevRun->pSettParamList, pDevRun->nSettParamTotal);
        Sort_ParamList_ByPrio_and_MakeBlob(pDevRun->pCtrlParamList, pDevRun->nCtrlParamTotal);
    }

    
    TRACEX("------NOW cREADETE structes for web OKKKKKK---\n");
    return TRUE;
}

/*
static BOOL IPC_GetLocalList(void)
{
    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_LOCALLIST, NULL, 0);
    VAR_VALUE varV;
    int nParsedAuthCount = 0;
    int nTotalItem, nListVersion;
    int n;
    size_t nAuthSize = sizeof(AUTHORIZATION_DATA);
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_LOCALLIST, 500))
    {
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = (int)s_pFCGIRough->nParsed_InDataLen;
        if(nDataLen >= 8)
        {
            memcpy(varV.abyValue, pDataPos, 4);
            nListVersion = varV.lValue;
            memcpy(varV.abyValue, pDataPos + 4, 4);
            nTotalItem = varV.lValue;
            nDataLen -= 8;
            pDataPos += 8;
            while(nDataLen > 0)
            {
                memcpy(&s_pFCGIRough->chargerDataRepo.authLocalList[nParsedAuthCount],
                     pDataPos, nAuthSize);
                nParsedAuthCount++;
                nDataLen -= nAuthSize;
                pDataPos += nAuthSize;
                //TRACEX("---nParsedAuthCount%d, nDataLen%d---\n", nParsedAuthCount, nDataLen);
            }

            s_pFCGIRough->chargerDataRepo.nLocalListVersion = nListVersion;
            s_pFCGIRough->chargerDataRepo.nLocalListItemTotal = nParsedAuthCount;
            if(nParsedAuthCount == nTotalItem)
            {
                bParseOK = TRUE;
            } 
            TRACEX("----parsed locallist OK, version->%d, total[%d, %d]---\n",
                nListVersion, nParsedAuthCount, nTotalItem);           
        }
    }
    s_pFCGIRough->bNeedUpdateLocalList = FALSE;
    return bParseOK;
}
*/
/*
static BOOL IPC_GetLocalCache(void)
{
    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_LOCALCACHE, NULL, 0);
    VAR_VALUE varV;
    size_t nBlobSize = sizeof(A_BLOB_AUTHORIZECACHE);
    A_BLOB_AUTHORIZECACHE blobCache;
    //AUTHORIZATION_DATA* pAuth = s_pFCGIRough->chargerDataRepo.authLocalCache;
    size_t nAuthSize = sizeof(AUTHORIZATION_DATA);
    int n;
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_LOCALCACHE, 500))
    {
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = (int)s_pFCGIRough->nParsed_InDataLen;  
        // A_BLOB_AUTHORIZECACHE* pCacccc = (A_BLOB_AUTHORIZECACHE*)pDataPos;
        // TRACEX("-----nDataLen[%d], nBlobSize%d, %d, %d-----\n", nDataLen, nBlobSize,
        //     pCacccc->nAuthorizationDataNumber, pCacccc->emBlobType);
        if(nDataLen == nBlobSize)
        {
            memcpy((void*)&blobCache, (void*)pDataPos, nBlobSize);     
            // TRACEX("----11111111parsed localcache OK, total[%d, %d]---\n",
            //     blobCache.nAuthorizationDataNumber, blobCache.emBlobType);     
            if(blobCache.nAuthorizationDataNumber <= WEB_SUPPORTED_MAX_LOCALCACHE) 
            {
                s_pFCGIRough->chargerDataRepo.nLocalCacheItemTotal = 
                    blobCache.nAuthorizationDataNumber;
                for(n = 0; n < blobCache.nAuthorizationDataNumber; n++)
                {
                    memcpy((void*)&s_pFCGIRough->chargerDataRepo.authLocalCache[n],
                            (void*)&blobCache.authorizationDataList[n], nAuthSize);     
                }
                bParseOK = TRUE;                 
            }      
        
            TRACEX("----22222222parsed localcache OK, total[%d, %d]---\n",
                blobCache.nAuthorizationDataNumber, blobCache.emBlobType);           
        }
    }
    s_pFCGIRough->bNeedUpdateCache = FALSE;
    return bParseOK;
}
*/
/*
static BOOL IPC_GetChgProfiles(void)
{
    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_CHGPROFILES, NULL, 0);
    VAR_VALUE varV;
    size_t nBlobSize = sizeof(A_BLOB_CHARGINGPROFILE);    
    A_BLOB_CHARGINGPROFILE* pProfles = &s_pFCGIRough->chargerDataRepo.chgProfiles;
    int n;
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_CHGPROFILES, 500))
    {
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = (int)s_pFCGIRough->nParsed_InDataLen;  
        // A_BLOB_AUTHORIZECACHE* pCacccc = (A_BLOB_AUTHORIZECACHE*)pDataPos;
        // TRACEX("-----nDataLen[%d], nBlobSize%d, %d, %d-----\n", nDataLen, nBlobSize,
        //     pCacccc->nAuthorizationDataNumber, pCacccc->emBlobType);
        if(nDataLen == nBlobSize)
        {
            memcpy((void*)pProfles, (void*)pDataPos, nBlobSize);  
            bParseOK = TRUE;    
            TRACEX("----22222222parsed chgProfile OK, maxCP->%d, defaultCP->[%d,%d,%d], currentCP->[%d,%d,%d]----\n",
                pProfles->nInstalled_MaxCP, 
                pProfles->nInstalled_TxDefault[0], pProfles->nInstalled_TxDefault[1], pProfles->nInstalled_TxDefault[2],
                pProfles->bIs_TxCurrent_Installed[0], pProfles->bIs_TxCurrent_Installed[1], pProfles->bIs_TxCurrent_Installed[2]);           
        }
    }
    s_pFCGIRough->bNeedUpdateChgProfiles = FALSE;
    return bParseOK;
}
*/

static BOOL IPC_GetAllParamVals(void)
{
    int nDevTotal = s_pFCGIRough->chargerDataRepo.nDevInfoTotal;
    WEB_RUN_DEV* pDevRun = s_pFCGIRough->chargerDataRepo.pAllRunDevs;
    int nParamTpTotal;
    WEB_PARAM_VALUE* pWebParVal;
    VAR_VALUE varV;
    int m,n;

    //now prepare the sending buffer
    static BYTE byParamArrs[MAX_WEB_REQDATA_SIZE];
    memset(byParamArrs, 0, MAX_WEB_REQDATA_SIZE);
    int nReqSendDataLen = 0;
    for(m = 0; m < nDevTotal; m++, pDevRun++)
    {
        //status params
        nParamTpTotal = pDevRun->nStatusParamTotal;
        pWebParVal = pDevRun->pStatusParamList;
        for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
        {
            byParamArrs[nReqSendDataLen] = (BYTE)PARAM_TYPE_STATUS;
            nReqSendDataLen += 1;
            varV.lValue = pDevRun->pDevInfo->nDevId;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
            varV.lValue = pWebParVal->pParamInfo->nParamId;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
            varV.lValue = pWebParVal->pParamInfo->nValueSize;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
        //sett params
        nParamTpTotal = pDevRun->nSettParamTotal;
        pWebParVal = pDevRun->pSettParamList;
        for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
        {
            byParamArrs[nReqSendDataLen] = (BYTE)PARAM_TYPE_SETTING;
            nReqSendDataLen += 1;
            varV.lValue = pDevRun->pDevInfo->nDevId;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
            varV.lValue = pWebParVal->pParamInfo->nParamId;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
            varV.lValue = pWebParVal->pParamInfo->nValueSize;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
        //ctrl params
        nParamTpTotal = pDevRun->nCtrlParamTotal;
        pWebParVal = pDevRun->pCtrlParamList;
        for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
        {
            byParamArrs[nReqSendDataLen] = (BYTE)PARAM_TYPE_CONTROL;
            nReqSendDataLen += 1;
            varV.lValue = pDevRun->pDevInfo->nDevId;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
            varV.lValue = pWebParVal->pParamInfo->nParamId;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
            varV.lValue = pWebParVal->pParamInfo->nValueSize;
            memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
    }

    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_ALL_PARAMVALS, byParamArrs, nReqSendDataLen);    
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_ALL_PARAMVALS, 500))
    {
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = s_pFCGIRough->nParsed_InDataLen;
        if((s_pFCGIRough->byParsedRTN != WEB_ERRCODE_OK) || 
            (nDataLen < 100))
        {
            TRACE("----RTN is NOOK for getVal!! nLen->%d--\n", nDataLen);
            return FALSE;
        }
        //now parse the param val the same order as send
        pDevRun = s_pFCGIRough->chargerDataRepo.pAllRunDevs;
        for(m = 0; m < nDevTotal; m++, pDevRun++)
        {
            int kkk;
            for(kkk = 0; kkk < 3; kkk++)
            {
                //status params
                nParamTpTotal = (kkk == 0) ? pDevRun->nStatusParamTotal :
                    ((kkk == 1) ? pDevRun->nSettParamTotal : pDevRun->nCtrlParamTotal);
                pWebParVal = (kkk == 0) ? pDevRun->pStatusParamList :
                    ((kkk == 1) ? pDevRun->pSettParamList : pDevRun->pCtrlParamList);
                for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
                {
                    memcpy(varV.abyValue, pDataPos, 4);
                    pWebParVal->m_emValueAttribute = varV.ulValue;                    
                    nDataLen -= 4;
                    pDataPos += 4;
                    if(pWebParVal->pParamInfo->nValueSize > 4)
                    {
                        /* NJX debug: detect IPC payload size mismatch (the
                         * WaterChiller field grew sizeof(A_BLOB_DEVINSTALLCONFIG)
                         * by 4 bytes; if nccu_app sends old-size data, this memcpy
                         * would overrun and desync the parse loop. */
                        if (nDataLen < pWebParVal->pParamInfo->nValueSize) {
                            
                            break;
                        }
                        //TRACE("---Get Blob_Param[%d, %d, %d], varSize->%d, nDataLen->%d---\n",
                        //   pDevRun->pDevInfo->nDevId, pWebParVal->pParamInfo->emValType,
                        //    pWebParVal->pParamInfo->nParamId, pWebParVal->pParamInfo->nValueSize, nDataLen);
                        memcpy(pWebParVal->currValue.blobValue, pDataPos, 
                            pWebParVal->pParamInfo->nValueSize);
                        nDataLen -= pWebParVal->pParamInfo->nValueSize;
                        pDataPos += pWebParVal->pParamInfo->nValueSize;
                    }
                    else
                    {
                        //TRACE("---Get Normal_Param[%d, %d, %d], varSize->%d, nDataLen->%d---\n",
                        //    pDevRun->pDevInfo->nDevId, pWebParVal->pParamInfo->emValType,
                        //    pWebParVal->pParamInfo->nParamId, pWebParVal->pParamInfo->nValueSize, nDataLen);
                        memcpy(pWebParVal->currValue.abyValue, pDataPos, 4);
                        nDataLen -= 4;
                        pDataPos += 4;
                    }
                }
            }
        }

        //if error we just record this
        if(nDataLen != 0)
        {
            TRACEX("--ERRRRRRRRdalalen for GetParamVals--\n");
            //return FALSE;
        }
        bParseOK = TRUE;
    }

    TRACEX("----Get All the paramVals %s---\n", bParseOK ? "OK" : "FAILED");
    return bParseOK;
}

static BOOL IPC_GetAllParamVals_NewWeb(void)
{
    int nParamTpTotal;
    WEB_PARAM_VALUE* pWebParVal;
    int DevId;
    VAR_VALUE varV;
    int m,n;
    //now prepare the sending buffer
    static BYTE byParamArrs[MAX_WEB_REQDATA_SIZE];
    memset(byParamArrs, 0, MAX_WEB_REQDATA_SIZE);
    int nReqSendDataLen = 0;
    int i = 0, j = 0;
    
    for(i = 0; i < s_pFCGIRough->chargerDataRepo.Setnum; ++i)
    {
        for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal; ++j)
        {
            nParamTpTotal = s_pFCGIRough->chargerDataRepo.nParamTotal[i][j];
            pWebParVal = s_pFCGIRough->chargerDataRepo.pSettParList[i][j];  
            for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
            {   
                // printf("-----n = %d, DevId=%d, ParamId=%d---web1 = %d, web2 = %d, type=%d\n", n, pWebParVal->nDevid, pWebParVal->pParamInfo->nParamId,\
                // pWebParVal->pParamInfo->Web_1, pWebParVal->pParamInfo->Web_2, pWebParVal->pParamInfo->emParamType);

                byParamArrs[nReqSendDataLen] = (BYTE)pWebParVal->pParamInfo->emParamType;
                nReqSendDataLen += 1;
                varV.lValue = pWebParVal->nDevid;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
                varV.lValue = pWebParVal->pParamInfo->nParamId;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
                varV.lValue = pWebParVal->pParamInfo->nValueSize;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
            }
        }
       //printf("-----Finished sett params for i=%d---\n", i);
    }

    for(i = 0; i < s_pFCGIRough->chargerDataRepo.Stanum; ++i)
    {
    //printf("---main_idx = %d---\n", i);
        for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].nSubTotal; ++j)
        {
            nParamTpTotal = s_pFCGIRough->chargerDataRepo.nTotal[i][j];
            pWebParVal = s_pFCGIRough->chargerDataRepo.pstatusParList[i][j];
            for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
            {
                //printf("-----n = %d, DevId=%d, ParamId=%d--- web1 = %d, web2 = %d\n", n, pWebParVal->nDevid, pWebParVal->pParamInfo->nParamId,\
                pWebParVal->pParamInfo->Web_1, pWebParVal->pParamInfo->Web_2);
                byParamArrs[nReqSendDataLen] = (BYTE)PARAM_TYPE_STATUS;
                nReqSendDataLen += 1;
                varV.lValue = pWebParVal->nDevid;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
                varV.lValue = pWebParVal->pParamInfo->nParamId;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
                varV.lValue = pWebParVal->pParamInfo->nValueSize;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
            }
        }  
    }

    BOOL bParseOK = FALSE;
    
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_ALL_PARAMVALS, byParamArrs, nReqSendDataLen);  
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_ALL_PARAMVALS, 500))
    {
        
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = s_pFCGIRough->nParsed_InDataLen;
        if((s_pFCGIRough->byParsedRTN != WEB_ERRCODE_OK) || 
            (nDataLen < 100))
        {
            
            return FALSE;
        }  
         //printf("...11111111111111111111\n");
        
         for(i = 0; i < s_pFCGIRough->chargerDataRepo.Setnum; ++i)
         {
            //printf("Set nSubTotal = %d done---\n", s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal);
             for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal; ++j)
             {
                nParamTpTotal = s_pFCGIRough->chargerDataRepo.nParamTotal[i][j];
                pWebParVal = s_pFCGIRough->chargerDataRepo.pSettParList[i][j];  
                for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
                {  
                    //printf("parse-----n = %d, DevId=%d, ParamId=%d---web1 = %d, web2 = %d, type=%d\n", n, pWebParVal->nDevid, pWebParVal->pParamInfo->nParamId,\
                    pWebParVal->pParamInfo->Web_1, pWebParVal->pParamInfo->Web_2, pWebParVal->pParamInfo->emParamType);

                    if (nDataLen < 4) { return FALSE; }
                    memcpy(varV.abyValue, pDataPos, 4);
                    pWebParVal->m_emValueAttribute = varV.ulValue;
                    nDataLen -= 4;
                    pDataPos += 4;
                    if(pWebParVal->pParamInfo->nValueSize > 4)
                    {
                        if (nDataLen < pWebParVal->pParamInfo->nValueSize) { return FALSE; }
                        memcpy(pWebParVal->currValue.blobValue, pDataPos,
                            pWebParVal->pParamInfo->nValueSize);
                        nDataLen -= pWebParVal->pParamInfo->nValueSize;
                        pDataPos += pWebParVal->pParamInfo->nValueSize;
                    }
                    else
                    {
                        memcpy(pWebParVal->currValue.abyValue, pDataPos, 4);
                        nDataLen -= 4;
                        pDataPos += 4;
                    }
                }
             }
        }
        
        for(i = 0; i < s_pFCGIRough->chargerDataRepo.Stanum; ++i)
        {
            //printf("----i = %d---\n", i);
            //printf("status nSubTotal = %d---\n", s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].nSubTotal);
            for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaStatusDispList1[i].nSubTotal; ++j)
            {
                nParamTpTotal = s_pFCGIRough->chargerDataRepo.nTotal[i][j];
                pWebParVal = s_pFCGIRough->chargerDataRepo.pstatusParList[i][j];
                for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
                {
                    if (nDataLen < 4) { return FALSE; }
                    memcpy(varV.abyValue, pDataPos, 4);
                    pWebParVal->m_emValueAttribute = varV.ulValue;
                    nDataLen -= 4;
                    pDataPos += 4;
                    if(pWebParVal->pParamInfo->nValueSize > 4)
                    {
                        if (nDataLen < pWebParVal->pParamInfo->nValueSize) { return FALSE; }
                        memcpy(pWebParVal->currValue.blobValue, pDataPos,
                            pWebParVal->pParamInfo->nValueSize);
                        nDataLen -= pWebParVal->pParamInfo->nValueSize;
                        pDataPos += pWebParVal->pParamInfo->nValueSize;
                    }
                    else
                    {
                        memcpy(pWebParVal->currValue.abyValue, pDataPos, 4);
                        nDataLen -= 4;
                        pDataPos += 4;
                    }
                }
            }
        }
        if(nDataLen != 0)
        {
            
        }
        
        bParseOK = TRUE;    
    }
    else
    {
        
    }
    
    return bParseOK;
}

static BOOL IPC_GetReadbackValue(void)
{
    int nParamTpTotal;
    WEB_PARAM_VALUE* pWebParVal;
    int DevId;
    VAR_VALUE varV;
    int m,n;
    //now prepare the sending buffer
    static BYTE byParamArrs[MAX_WEB_REQDATA_SIZE];
    memset(byParamArrs, 0, MAX_WEB_REQDATA_SIZE);
    int nReqSendDataLen = 0;
    int i = 0, j = 0;
    
    // for(i = 0; i < s_pFCGIRough->chargerDataRepo.Setnum; ++i)
    // {
        for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaSetDispList1[9].nSubTotal; ++j)
        {
            // if(i != 9)
            // {
            //     continue;  //非BMS参数回读  不去获取回读值
            // }
            nParamTpTotal = s_pFCGIRough->chargerDataRepo.nParamTotal[9][j];
            pWebParVal = s_pFCGIRough->chargerDataRepo.pSettParList[9][j];  
            for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
            {   
                TRACEI("-----n = %d, DevId=%d, ParamId=%d---web1 = %d, web2 = %d, type=%d\n", n, pWebParVal->nDevid, pWebParVal->pParamInfo->nParamId,\
                pWebParVal->pParamInfo->Web_1, pWebParVal->pParamInfo->Web_2, pWebParVal->pParamInfo->emParamType);

                byParamArrs[nReqSendDataLen] = (BYTE)pWebParVal->pParamInfo->emParamType;
                nReqSendDataLen += 1;
                varV.lValue = pWebParVal->nDevid;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
                varV.lValue = pWebParVal->pParamInfo->nParamId;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
                varV.lValue = pWebParVal->pParamInfo->nValueSize;
                memcpy(byParamArrs + nReqSendDataLen, varV.abyValue, 4);
                nReqSendDataLen += 4;
            }
        }
       //printf("-----Finished sett params for i=%d---\n", i);
    // }

    BOOL bParseOK = FALSE;
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_GET_READBACK_VALS, byParamArrs, nReqSendDataLen);  
    if(Read_andParse_WebIPC_Rep(WEB_REQTYPE_GET_READBACK_VALS, 500))
    {
        BYTE* pDataPos = s_pFCGIRough->pbyParsed_InDataPos;
        int nDataLen = s_pFCGIRough->nParsed_InDataLen;
        if((s_pFCGIRough->byParsedRTN != WEB_ERRCODE_OK) || 
            (nDataLen < 100))
        {
            //printf("----RTN is NOOK for getVal!! nLen->%d--\n", nDataLen);
            return FALSE;
        }  
         //printf("...11111111111111111111\n");
        //  for(i = 0; i < s_pFCGIRough->chargerDataRepo.Setnum; ++i)
        //  {
            //printf("Set nSubTotal = %d done---\n", s_pFCGIRough->chargerDataRepo.ParaSetDispList1[i].nSubTotal);
             for(j = 0; j < s_pFCGIRough->chargerDataRepo.ParaSetDispList1[9].nSubTotal; ++j)
             {
                nParamTpTotal = s_pFCGIRough->chargerDataRepo.nParamTotal[9][j];
                pWebParVal = s_pFCGIRough->chargerDataRepo.pSettParList[9][j];  
                for(n = 0; n < nParamTpTotal; n++, pWebParVal++)
                {  
                    //printf("parse-----n = %d, DevId=%d, ParamId=%d---web1 = %d, web2 = %d, type=%d\n", n, pWebParVal->nDevid, pWebParVal->pParamInfo->nParamId,\
                    pWebParVal->pParamInfo->Web_1, pWebParVal->pParamInfo->Web_2, pWebParVal->pParamInfo->emParamType);

                    memcpy(varV.abyValue, pDataPos, 4);
                    pWebParVal->m_emValueAttribute = varV.ulValue;                    
                    nDataLen -= 4;
                    pDataPos += 4;
                    if(pWebParVal->pParamInfo->nValueSize > 4)
                    {       
                        memcpy(pWebParVal->ReadValue.blobValue, pDataPos, 
                            pWebParVal->pParamInfo->nValueSize);
                        nDataLen -= pWebParVal->pParamInfo->nValueSize;
                        pDataPos += pWebParVal->pParamInfo->nValueSize;
                    }
                    else
                    {
                        memcpy(pWebParVal->ReadValue.abyValue, pDataPos, 4);
                        nDataLen -= 4;
                        pDataPos += 4;
                    }
                }
             }
        // }

        if(nDataLen != 0)
        {
            //printf("Set--ERRRRRRRRdalalen for GetParamVals--\n");
            //return FALSE;
        }
        bParseOK = TRUE;    
    }
    return bParseOK;
}

static WEB_DEVICE_INFO* Find_DevInfo_ById(int nDevId)
{
    WEB_DEVICE_INFO* pDev = s_pFCGIRough->chargerDataRepo.pAllDevInfo;
    int nTotal = s_pFCGIRough->chargerDataRepo.nDevInfoTotal;
    int n;
    for(n = 0; n < nTotal; n++, pDev++)
    {
        if(pDev->nDevId == nDevId)
        {
            return pDev;
        }
    }
    return NULL;
}

static WEB_ALARM_INFO* Find_AlmInfo_ById(int nAlmId)
{
    WEB_ALARM_INFO* pAlm = s_pFCGIRough->chargerDataRepo.pAllAlarmInfo;
    int nTotal = s_pFCGIRough->chargerDataRepo.nAlarmInfosTotal;
    int n;
    for(n = 0; n < nTotal; n++, pAlm++)
    {
        if(pAlm->nAlarmId == nAlmId)
        {
            return pAlm;
        }
    }
    return NULL;
}

// static void Web_Refresh_AllAlarms(sqlite3* db, void* pArg)
// {
//     UNUSED(pArg);
    
//     sqlite3_stmt * stat;
// 	sqlite3_prepare_v2(db, "select * from Data_Alarms where Alarm_Status = 1 order by Alarm_StartTime DESC", -1, &stat, NULL);
//     int nCount = 0;
//     WEB_ALARM_VALUE* pAlmVal = s_pFCGIRough->chargerDataRepo.activeAlms;
//     int nVal;
    
// 	while(sqlite3_step(stat) == SQLITE_ROW)
//     {
//         //will implement it later
//     }

//     s_pFCGIRough->bIsAlarmNeedRefresh = FALSE;

//     sqlite3_finalize(stat);
// }

static BOOL Web_QueryActiveAlarms(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
	//sqlite3_prepare_v2(db, "select * from Data_Alarms where Alarm_Status > 0 order by Alarm_StartTime DESC", -1, &stat, NULL);
    //John changed 2022-08-16: Only display Active alarms Not Display suppressed.
    sqlite3_prepare_v2(db, "select * from Data_Alarms where Alarm_Status = 1 order by Alarm_StartTime DESC", -1, &stat, NULL);
    int nCount = 0;
    WEB_ALARM_VALUE* pAlmVal = s_pFCGIRough->chargerDataRepo.activeAlms;

    int nVal;
    
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        //dev
        nVal = sqlite3_column_int(stat, 1); 
        pAlmVal->pBelongDev = Find_DevInfo_ById(nVal);
        if(pAlmVal->pBelongDev == NULL)
        {
            TRACEX("--Find NULL dev, so skip it--\n");
            continue;
        }
        //alm
        nVal = sqlite3_column_int(stat, 2); 
        pAlmVal->pAlarmInfo = Find_AlmInfo_ById(nVal);
        if(pAlmVal->pAlarmInfo == NULL)
        {
            TRACEX("--Find NULL almInfo, so skip it--\n");
            continue;
        }
        //starttime
        pAlmVal->tmLastOccured = (time_t)sqlite3_column_int(stat, 5);   

        // TRACEX("---Got a new alarm->[%s, %s], %d---\n", 
        //     LoadLangText_ById(pAlmVal->pBelongDev->nDevName_ResId),
        //     LoadLangText_ById(pAlmVal->pAlarmInfo->nAlmName_ResId),
        //     pAlmVal->tmLastOccured);

        nCount++;
        pAlmVal++;
        if(nCount >= WEB_MAX_SUPPORT_ACTALARM)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nActiveAlmTotal = nCount;
    TRACE("---Refresh activeAlaram OK!!!nCount=[%d]---\n", nCount);
    //s_pFCGIRough->bIsAlarmNeedRefresh = FALSE;

    sqlite3_finalize(stat);
    
    return TRUE;
}

static BOOL Web_QueryHistAlarms(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
	//sqlite3_prepare_v2(db, "select * from Data_Alarms where Alarm_Status = 0 order by Alarm_StartTime DESC", -1, &stat, NULL);
    sqlite3_prepare_v2(db, "select * from Data_Alarms order by Alarm_StartTime DESC", -1, &stat, NULL);
    int nCount = 0;
    WEB_ALARM_VALUE* pAlmVal = s_pFCGIRough->chargerDataRepo.histAlms;
    int nVal;
    
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        //dev
        nVal = sqlite3_column_int(stat, 1); 
        pAlmVal->pBelongDev = Find_DevInfo_ById(nVal);
        if(pAlmVal->pBelongDev == NULL)
        {
            TRACEX("--Find NULL dev, so skip it--\n");
            continue;
        }
        //alm
        nVal = sqlite3_column_int(stat, 2); 
        pAlmVal->pAlarmInfo = Find_AlmInfo_ById(nVal);
        if(pAlmVal->pAlarmInfo == NULL)
        {
            TRACEX("--Find NULL almInfo, so skip it--\n");
            continue;
        }
        //alm status
        pAlmVal->nAlarmStatus = sqlite3_column_int(stat, 3);

        //starttime
        pAlmVal->tmLastOccured = (time_t)sqlite3_column_int(stat, 5);   

        //endtime
        pAlmVal->tmLastCeased = (time_t)sqlite3_column_int(stat, 6);   

        // TRACEX("---Got a new alarm->[%s, %s], %d---\n", 
        //     LoadLangText_ById(pAlmVal->pBelongDev->nDevName_ResId),
        //     LoadLangText_ById(pAlmVal->pAlarmInfo->nAlmName_ResId),
        //     pAlmVal->tmLastOccured);

        nCount++;
        pAlmVal++;
        if(nCount >= WEB_MAX_SUPPORT_HISTORYALARM)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nHistAlmTotal = nCount;

    //s_pFCGIRough->bIsHisAlarmNeedRefresh = FALSE;
    
    sqlite3_finalize(stat);
    TRACEX("---Refresh histAlaram OK!!!---\n");
    return TRUE;
}

//----          BMS Historical Fault Records          ----//
static BOOL Web_QueryBmsHistFault(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    sqlite3_stmt *stat;
    char szSql[256];

    snprintf(szSql, sizeof(szSql),
        "SELECT * FROM Data_BmsHistFault ORDER BY Idx DESC LIMIT %d",
        WEB_MAX_SUPPORT_BMSHISTFAULT);
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

    int nExisting = 0;
    EVENT_BMS_HISTFAULT *pRec = s_pFCGIRough->chargerDataRepo.bmsHistFaults;

    while(sqlite3_step(stat) == SQLITE_ROW && nExisting < WEB_MAX_SUPPORT_BMSHISTFAULT)
    {
        pRec->nBattGrp      = sqlite3_column_int(stat, 1);
        pRec->nFaultType    = sqlite3_column_int(stat, 2);
        pRec->nFaultCode    = sqlite3_column_int(stat, 3);
        pRec->tmFault       = (time_t)sqlite3_column_int(stat, 4);
        pRec->nProtectState = sqlite3_column_int(stat, 5);
        pRec->nWorkState    = sqlite3_column_int(stat, 6);
        pRec->nCurrent      = sqlite3_column_int(stat, 7);
        pRec->nCustom3      = sqlite3_column_int(stat, 8);
        pRec->nCustom4      = sqlite3_column_int(stat, 9);
        pRec->nCustom5      = sqlite3_column_int(stat, 10);
        pRec->nStatus       = sqlite3_column_int(stat, 12);
        nExisting++;
        pRec++;
    }
    s_pFCGIRough->chargerDataRepo.nBmsHistFaultTotal = nExisting;
    sqlite3_finalize(stat);
    TRACEX("---Refresh BmsHistFault OK!!! nExisting=[%d]---\n", nExisting);
    return TRUE;
}

//----          BMS Historical Event Records          ----//
static BOOL Web_QueryBmsHistEvent(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    sqlite3_stmt *stat;
    char szSql[256];

    snprintf(szSql, sizeof(szSql),
        "SELECT * FROM Data_BmsHistEvent ORDER BY Idx DESC LIMIT %d",
        WEB_MAX_SUPPORT_BMSHISTEVENT);
    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);

    int nExisting = 0;
    EVENT_BMS_HISTEVENT *pRec = s_pFCGIRough->chargerDataRepo.bmsHistEvents;

    while(sqlite3_step(stat) == SQLITE_ROW && nExisting < WEB_MAX_SUPPORT_BMSHISTEVENT)
    {
        pRec->nBattGrp      = sqlite3_column_int(stat, 1);
        pRec->nEventType    = sqlite3_column_int(stat, 2);
        pRec->nEventCode    = sqlite3_column_int(stat, 3);
        pRec->tmEvent       = (time_t)sqlite3_column_int(stat, 4);
        pRec->nProtectState = sqlite3_column_int(stat, 5);
        pRec->nWorkState    = sqlite3_column_int(stat, 6);
        pRec->nCurrent      = sqlite3_column_int(stat, 7);
        pRec->nCustom3      = sqlite3_column_int(stat, 8);
        pRec->nCustom4      = sqlite3_column_int(stat, 9);
        pRec->nCustom5      = sqlite3_column_int(stat, 10);
        nExisting++;
        pRec++;
    }
    s_pFCGIRough->chargerDataRepo.nBmsHistEventTotal = nExisting;
    sqlite3_finalize(stat);
    TRACEX("---Refresh BmsHistEvent OK!!! nExisting=[%d]---\n", nExisting);
    return TRUE;
}

//----          2021-12-31 Added By John           ----//
static BOOL Web_Query_BmsRecords(sqlite3* db, void* pArg)
{

    UNUSED(pArg);
    sqlite3_stmt * stat;
	sqlite3_prepare_v2(db, "select DISTINCT Day from Data_RealTimeBmsData order by Day DESC", -1, &stat, NULL);
    int nCount = 0;
    // WEB_BMSREC_DATA* pBmsRecItem = s_pFCGIRough->chargerDataRepo.bmsRecData;
    // int nVal;
    // int kkk;
    memset(s_pFCGIRough->chargerDataRepo.tTimeSum, 0, sizeof(UINT)*WEB_SUPPORT_MAX_BMSREC);
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        // ZERO_POBJS(pBmsRecItem, 1);
        // pBmsRecItem->nIdx = sqlite3_column_int(stat, 0); 
        // pBmsRecItem->nDay = sqlite3_column_int(stat, 1); 
        // pBmsRecItem->tmLogTime = (time_t)sqlite3_column_int(stat, 2); 
        
        s_pFCGIRough->chargerDataRepo.tTimeSum[nCount] = sqlite3_column_int(stat, 0); 

        // printf("nCount = %d, time = %lld\n", nCount, s_pFCGIRough->chargerDataRepo.tTimeSum[nCount]);

        //TRACEX("Got a new BMS Record->[%d, %d, %d] nCount=[%d]\n", pBmsRecItem->nIdx, pBmsRecItem->nDay, pBmsRecItem->tmLogTime,nCount);
        nCount++;
        // pBmsRecItem++;
        if(nCount >= WEB_SUPPORT_MAX_BMSREC)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nBmsRecTotal = nCount;

    //s_pFCGIRough->bNeedUpdateBmsRecord = FALSE;

    sqlite3_finalize(stat);
    
    return TRUE;
} 

//----      2022-05-17 Added By John For BMS Health Statistic Data    ----//
static BOOL Web_Query_HealthRec(sqlite3* db, void* pArg)
{
    //printf("--- Web_Query_HealthRec Start TM=[%.1f]---\n", GetCurrentTime());
    sqlite3_stmt * stat;
	sqlite3_prepare(db, "select * from Data_BMSHealthData order by Day ASC", -1, &stat, NULL);
    UINT nCount = 0;
    HIS_HEALTH_STATIC_DAILY* pHealthItem = s_pFCGIRough->chargerDataRepo.healthRec; 
 
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        ZERO_POBJS(pHealthItem, 1);
        pHealthItem->nIdx = (BYTE)sqlite3_column_int(stat, 0); 
        pHealthItem->nDay = sqlite3_column_int(stat, 1);
        pHealthItem->tmLogTime = (time_t)sqlite3_column_int(stat, 2); 

        int nRecSize = sqlite3_column_bytes(stat, 3);
        int nCompressSize = sqlite3_sizeof_blob_uncompressed(stat, 3);
        
        //printf("nRecSize=[%d] nCompressSize=[%d] nIdx=[%d] nDay=[%d] tmLogTime=[%ld]\n", \
        //    nRecSize, nCompressSize, pHealthItem->nIdx, pHealthItem->nDay ,pHealthItem->tmLogTime);
        
        if(nCompressSize > 0)
        {   
			sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&pHealthItem->bmsHealthData, sizeof(A_BMS_HEALTH_LIST));
            //TRACEX("----parsedCPOMMMOK---nDay=[%d] tmLogTime=[%ld]\n", pHealthItem->nIdx, pHealthItem->nDay);
        }

        nCount++;
        pHealthItem++;
        
        if(nCount >= WEB_MAX_SUPPORT_HEALTH)
        {
            break;
        } 
    }

    s_pFCGIRough->chargerDataRepo.nHealthRecTotal = nCount;

    TRACE("---Web_Query_HealthRec End nHealthRecTotal=[%d] TM=[%.1f]---\n", s_pFCGIRough->chargerDataRepo.nHealthRecTotal, GetCurrentTime());

    sqlite3_finalize(stat);
    
    return TRUE;
}
static int GetParamIdIndex(int nParamId) 
{
    int i;
    for (i = 0; i < PARAM_ID_CNT; i++)
    {
        if (g_PCS_ParamId_List[i] == nParamId)
        {
            return i;
        }
    }
    return -1;
}

static BOOL Web_QueryPCSRereadParam(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt* stmt = NULL;
    int nRet = -1;
    char sql[512] = {0};
    int idx;
    snprintf(sql, sizeof(sql), 
             "SELECT "
             "Device_Id, Param_Id, " 
             "RereadValue_SingleModule_1, RereadValue_SingleModule_2, "
             "RereadValue_SingleModule_3, RereadValue_SingleModule_4, "
             "RereadValue_SingleModule_5, RereadValue_SingleModule_6, "
             "RereadValue_SingleModule_7, RereadValue_SingleModule_8 "
             "FROM Data_SavedSettings "
             "WHERE Device_Id BETWEEN 701 AND 704");

    nRet = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if(nRet != SQLITE_OK)
    {
        TRACEX("Read_RereadPCSParam_For_Dev701_704: 编译SQL失败! errCode=%d, errMsg=[%s], SQL=[%s]\n",
               nRet, sqlite3_errmsg(db), sql);
        return FALSE;
    }

    BOOL bHasData = FALSE;
    while ((nRet = sqlite3_step(stmt)) == SQLITE_ROW)
    {   
        bHasData = TRUE;
        int nDevId = sqlite3_column_int(stmt, 0);
        int nParamId = sqlite3_column_int(stmt, 1);        
        s_pFCGIRough->chargerDataRepo.nPCSGroupIdx  = nDevId;
        s_pFCGIRough->chargerDataRepo.nRereadParamId = nParamId; 
        int nParamidx = GetParamIdIndex(nParamId);
        if (nParamidx < 0)
        {
            continue;
        }

        for (idx = 0; idx < 8; ++idx)
        {
            const void* pBlobData = sqlite3_column_blob(stmt, 2 + idx);
            int nBlobSize = sqlite3_column_bytes(stmt, 2 + idx); 
            const unsigned char* pData = (const unsigned char*)pBlobData;
            int i;
            for (i = 0; i < nBlobSize; i++)
            {
                TRACEX("%02X ", pData[i]);
                if ((i + 1) % 16 == 0)
                {
                    TRACEX("\n");
                }
            }

            if (s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].RereadValue.blobValue != NULL)
            {
                free(s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].RereadValue.blobValue);
                s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].RereadValue.blobValue = NULL;
                s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].nBlobSize = 0;
            }

            if (pBlobData != NULL && nBlobSize > 0)
            {
                s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].RereadValue.blobValue = malloc(nBlobSize);
                if(s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].RereadValue.blobValue == NULL)
                {
                    sqlite3_finalize(stmt);
                    return FALSE;
                }
                memcpy(s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].RereadValue.blobValue, pBlobData, nBlobSize);
                s_pFCGIRough->chargerDataRepo.pRereadPCSParam[nDevId - 701][nParamidx][idx].nBlobSize = nBlobSize;
            }

            s_pFCGIRough->chargerDataRepo.nSingleRectIdx = idx;
        }
    }
    if (stmt != NULL)
    {
        sqlite3_finalize(stmt);
        stmt = NULL;
    }

    else
    { 
        TRACEX("Web_QueryPCSRereadParam: Execute sql fail! errCode=%d, errMsg=[%s]\n", nRet, sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return FALSE;
    }

    sqlite3_finalize(stmt);
    return bHasData;

}
/*
static BOOL Web_Query_ChargeRecords(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    //Jimmy note: we just query all(including active charging records)
    sqlite3_stmt * stat;
	sqlite3_prepare(db, "select * from Data_ChargeRecords order by Charge_BeginTime DESC", -1, &stat, NULL);
    int nCount = 0;
    HIS_CHARGE_RECORD* pHisRecItem = s_pFCGIRough->chargerDataRepo.hisChgRecords;
    int nVal;
    int kkk;
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        ZERO_POBJS(pHisRecItem, 1);
        pHisRecItem->emGunNum = (BYTE)sqlite3_column_int(stat, 1); 
        pHisRecItem->tmChargeStart = (time_t)sqlite3_column_int(stat, 2); 
        pHisRecItem->nSessionID = sqlite3_column_int(stat, 3); 
        //user card
        if((nVal = sqlite3_column_bytes(stat, 4)) > 0)
        {
            memcpy(pHisRecItem->szUserID, sqlite3_column_blob(stat, 4), MIN(64, nVal));
        }
        //car VIN
        if((nVal = sqlite3_column_bytes(stat, 5)) > 0)
        {
            memcpy(pHisRecItem->szVIN, sqlite3_column_blob(stat, 5), MIN(64, nVal));
        }

        pHisRecItem->emChargeStartMethod = (CHARGE_START_MOTHOD)sqlite3_column_int(stat, 6); 
        pHisRecItem->emChargeStatus = sqlite3_column_int(stat, 7); 
        pHisRecItem->nReportFlag = sqlite3_column_int(stat, 8); 
        pHisRecItem->bIsOfflinePayedOK = sqlite3_column_int(stat, 9); 
        nVal = sqlite3_column_bytes(stat, 10);
        if((nVal > sizeof(CHARGE_REC_EXTRADATA)) ||
            (nVal < 5))
        {
            TRACEX("--Find INVALID chargeRecord, so skip it--\n");
            continue;
        }
        memcpy(&pHisRecItem->extraRecData, sqlite3_column_blob(stat, 10), 
            MIN(sizeof(CHARGE_REC_EXTRADATA), nVal));

        // TRACEX("---%d, Got a new chargeRecord->[%d, %d], %d, totalAmount->%.1f, and the 48points->[", nCount + 1,
        //     pHisRecItem->emGunNum, pHisRecItem->nSessionID, 
        //     pHisRecItem->tmChargeStart, pHisRecItem->extraRecData.runData.fCostkWh);
        // for(kkk = 0; kkk < 48; kkk++)
        // {
        //     TRACEX("%.1f, ", pHisRecItem->extraRecData.runData.f48Stat_kWh[kkk]);
        // }
        // TRACEX("]---\n");
        
        nCount++;
        pHisRecItem++;
        if(nCount >= WEB_MAX_SUPPORT_CHGREC)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nHisChgRecTotal = nCount;

    s_pFCGIRough->bNeedUpdateChgRecord = FALSE;

    sqlite3_finalize(stat);
    
    return TRUE;
}*/

static void Get_OrRefresh_TimeZoneOffset(void)
{
    char szTimeOffset[8];
    memset(szTimeOffset, 0, 8);
    float fOffsetHrs = 0.0;
    s_pFCGIRough->fTimeOffsetHrs = 0.0;
    if(!Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 66, 8, (void*)szTimeOffset))
    {
        TRACEX("----CAN not get the timeZone---\n");
        return;
    }

    BOOL bMinus = FALSE;
    if((szTimeOffset[0] != '+') && (szTimeOffset[0] != '-'))
    {
        TRACEX("----CAN not get the timeZone---\n");
        return;
    }
    else
    {
        bMinus = (szTimeOffset[0] == '-') ? TRUE : FALSE;
    }
    
    int n;
    char szInt[8];
    memset(szInt, 0, 8);
    int nPos = -1;
    for(n = 1; n < 8; n++)
    {
        szInt[n - 1] = szTimeOffset[n];
        if(szTimeOffset[n] == ':')
        {
            nPos = n;
            break;
        }
    }
    if(nPos == -1)
    {
        TRACEX("----CAN not find the ':' in timeZone string---\n");
        return;
    }
    int nInt = atoi(szInt);
    int nDigit = atoi(szTimeOffset + nPos + 1);
    fOffsetHrs = (float)nInt + (float)nDigit/60;
    fOffsetHrs *= (bMinus ? -1 : 1);
    TRACEX("---fOffsetHrs->%.2f, [%d,%d]---\n", fOffsetHrs, nInt, nDigit);
    s_pFCGIRough->fTimeOffsetHrs = fOffsetHrs;
}

static BOOL Web_QuerySysLogs(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
    //sqlite3_prepare_v2(db, "select * from Data_AppLog order by Log_Time DESC", -1, &stat, NULL);
    sqlite3_prepare_v2(db, "select * from Data_AppLog order by Log_Time", -1, &stat, NULL);
    int nCount = 0;
    WEB_SYSLOG* pSysLog = s_pFCGIRough->chargerDataRepo.sysLogs;
    int nLogSize;

    //empty the old logs
    int n;
    for(n = 0; n < s_pFCGIRough->chargerDataRepo.nSysLogTotal; n++, pSysLog++)
    {
        SAFELY_DELETE(pSysLog->szLog);
        pSysLog->szLog = NULL;
    }
    s_pFCGIRough->chargerDataRepo.nSysLogTotal = 0;
    
    //now re-query the logs
    pSysLog = s_pFCGIRough->chargerDataRepo.sysLogs;
    // int nRet = sqlite3_step(stat);
    // if(nRet != SQLITE_ROW)
    // {
    //     TRACEX("----sysLog query sql RTN fail->%d---\n", nRet);
    // }
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        //logId / logType / logTime
        pSysLog->nLogId = sqlite3_column_int(stat, 0); 
        pSysLog->emLogType = (LOG_TYPE)sqlite3_column_int(stat, 1); 
        pSysLog->fLogTime = sqlite3_column_double(stat, 2); 
        nLogSize = sqlite3_column_bytes(stat, 3) + 1;
        if((nLogSize < 1) || (nLogSize > 256))
        {
            continue;
        }
        pSysLog->szLog = NEW(char, nLogSize + 1);
        snprintf(pSysLog->szLog, nLogSize, "%s", sqlite3_column_text(stat, 3));
        // TRACEX("---Got a new syslog->%d \t %d \t %.3f \t %s---\n", 
        //     pSysLog->nLogId, pSysLog->emLogType, pSysLog->fLogTime,
        //     pSysLog->szLog);
        nCount++;
        pSysLog++;
        if(nCount >= WEB_SUPPORT_MAX_SYSLOG)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nSysLogTotal = nCount;
    //TRACEX("---now update sysLog done->[%d]---\n", nCount);
    sqlite3_finalize(stat);
    
    return TRUE;
}

static BOOL Web_QueryOCPPLogs(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
    sqlite3_prepare_v2(db, "select * from Data_DumpLog where Dump_Src = 1 order by Dump_Time DESC", -1, &stat, NULL);
    //sqlite3_prepare_v2(db, "select * from Data_DumpLog where Dump_Src = 0 order by Dump_Time DESC limit 10000", -1, &stat, NULL);
    int nCount = 0;
    WEB_OCPPLOG* pOCPPLog = s_pFCGIRough->chargerDataRepo.ocppLogs;
    int nLogSize;

    //empty the old logs
    int n;
    for(n = 0; n < s_pFCGIRough->chargerDataRepo.nOCPPLogTotal; n++, pOCPPLog++)
    {
        SAFELY_DELETE(pOCPPLog->szMsg);
        pOCPPLog->szMsg = NULL;
    }
    s_pFCGIRough->chargerDataRepo.nOCPPLogTotal = 0;
    
    //now re-query the logs
    pOCPPLog = s_pFCGIRough->chargerDataRepo.ocppLogs;
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        //logId / logType / logTime
        pOCPPLog->nLogId = sqlite3_column_int(stat, 0); 
        pOCPPLog->emDumpType = (LOG_TYPE)sqlite3_column_int(stat, 2); 
        pOCPPLog->fLogTime = sqlite3_column_double(stat, 3); 
        nLogSize = sqlite3_column_bytes(stat, 4) + 1;
        // if((nLogSize < 1) || (nLogSize > 2048)) //changed by Jimmy from 512=>2048 'cause some meterValues is > 512
        // {
        //     continue;
        // }
        if(nLogSize < 1)
        {
            continue;
        }
        else if(nLogSize > 2048) //max display 2048char
        {
            nLogSize = 2048;
        }
        pOCPPLog->szMsg = NEW(char, nLogSize + 1);
        snprintf(pOCPPLog->szMsg, nLogSize, "%s", sqlite3_column_text(stat, 4));
        // TRACEX("---Got a new ocppLog->%d \t %d \t %.3f \t %s---\n", 
        //     pOCPPLog->nLogId, pOCPPLog->emDumpType, pOCPPLog->fLogTime,
        //     pOCPPLog->szMsg);
        nCount++;
        pOCPPLog++;
        if(nCount >= WEB_SUPPORT_MAX_OCPPLOG)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nOCPPLogTotal = nCount;

    sqlite3_finalize(stat);
    
    return TRUE;
}

static BOOL Web_QueryHisEvents(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
    sqlite3_prepare_v2(db, "select * from Data_Events order by Begin_Time DESC", -1, &stat, NULL);
    int nCount = 0;
    WEB_HIS_EVENT* pHisEv = s_pFCGIRough->chargerDataRepo.hisEvents;

    //empty the old records
    int n;
    for(n = 0; n < s_pFCGIRough->chargerDataRepo.nHisEventTotal; n++, pHisEv++)
    {
        SAFELY_DELETE(pHisEv->pEvData);
        pHisEv->pEvData = NULL;
    }
    s_pFCGIRough->chargerDataRepo.nHisEventTotal = 0;
    
    //now re-query the logs
    pHisEv = s_pFCGIRough->chargerDataRepo.hisEvents;
    BOOL bIsInvalidEv = FALSE;
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        bIsInvalidEv = FALSE;
        pHisEv->emEvType = sqlite3_column_int(stat, 1); 
        pHisEv->nEvDataSize = (UINT)sqlite3_column_bytes(stat, 3); 
        if((pHisEv->emEvType == EVENT_TYPE_SYSTEM_EXIT) && 
            (pHisEv->nEvDataSize == sizeof(EVENT_SYSTEM_EXIT)))
        {
            bIsInvalidEv = TRUE;
            TRACEX("---Get a EVENT_SYSTEM_EXIT event---\n");
        }
        else if((pHisEv->emEvType == EVENT_TYPE_FIRMWARE_UPDATE) && 
            (pHisEv->nEvDataSize == sizeof(EVENT_FIRMWAREUPDATE_STATUS)))
        {
            bIsInvalidEv = TRUE;
            TRACEX("---Get a EVENT_FIRMWAREUPDATE_STATUS event---\n");
        }
        else if((pHisEv->emEvType == EVENT_TYPE_DIAGNOSTIC_UPLOAD) && 
            (pHisEv->nEvDataSize == sizeof(EVENT_DIAGNOUPLOAD_STATUS)))
        {
            bIsInvalidEv = TRUE;
            TRACEX("---Get a EVENT_DIAGNOUPLOAD_STATUS event---\n");
        }
        else if((pHisEv->emEvType == EVENT_TYPE_IN_MAINTAINING) && 
            (pHisEv->nEvDataSize == 0))
        {
            bIsInvalidEv = TRUE;
            TRACEX("---Get a EVENT_TYPE_IN_MAINTAINING event---\n");
        }
        else if((pHisEv->emEvType == EVENT_TYPE_ABILITY_CHANGE) && 
            (pHisEv->nEvDataSize == sizeof(EVENT_ABILITY_CHG)))
        {
            bIsInvalidEv = TRUE;
            TRACEX("---Get a EVENT_ABILITY_CHG event---\n");
        }
 

        if(!bIsInvalidEv)
        {
            continue;
        }
        else if(pHisEv->nEvDataSize > 0)
        {
            pHisEv->pEvData = NEW(BYTE, pHisEv->nEvDataSize);
            memcpy(pHisEv->pEvData, sqlite3_column_blob(stat, 3), pHisEv->nEvDataSize);
        }
        int nStatus = sqlite3_column_int(stat, 2);        
        pHisEv->tmBegin = (time_t)sqlite3_column_int(stat, 4); 
        pHisEv->tmEnd = (nStatus == 0) ? (time_t)sqlite3_column_int(stat, 5) : 0; 

        nCount++;
        pHisEv++;
        if(nCount >= WEB_SUPPORT_MAX_HISEVENT)
        {
            break;
        } 
    }    

    s_pFCGIRough->chargerDataRepo.nHisEventTotal = nCount;

    sqlite3_finalize(stat);
    
    return TRUE;
}

static BOOL Web_QueryHisData(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
    //sqlite3_prepare_v2(db, "select * from Data_HisStatusData order by Record_Time DESC limit 10000", -1, &stat, NULL);
    sqlite3_prepare_v2(db, "select * from Data_HisStatusData order by Record_Time DESC", -1, &stat, NULL);
    int nCount = 0;
    WEB_HIS_DATA* pHisData = s_pFCGIRough->chargerDataRepo.hisData;
    
    //now re-query the logs
    int nParamId;
    WEB_PARAM_VALUE* pWebParamVal;
	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        pHisData->tmD_RecordTime = sqlite3_column_double(stat, 1); 
        pHisData->nDevId = sqlite3_column_int(stat, 2); 
        nParamId = sqlite3_column_int(stat, 3); 
        pWebParamVal = Get_ASingle_WebParaVal(PARAM_TYPE_STATUS, pHisData->nDevId, nParamId);
        if(pWebParamVal == NULL)
        {
            continue;
        }
        pHisData->pParamInfo = pWebParamVal->pParamInfo; 
        pHisData->fParamVal = (float)sqlite3_column_double(stat, 4); 

        nCount++;
        pHisData++;
        if(nCount >= WEB_SUPPORT_MAX_HISDATA)
        {
            break;
        } 
    }    
    TRACEX("-------Get HISDATA total: %d-------\n", nCount);
    s_pFCGIRough->chargerDataRepo.nHisDataTotal = nCount;

    sqlite3_finalize(stat);
    
    return TRUE;
}

/*
static BOOL Web_QueryMeterVals_ByGunAndStartTime(sqlite3* db, void* pArg)
{
    //UNUSED(pArg);
    QUERYCOND_HISMETERVAL* pCond = (QUERYCOND_HISMETERVAL*)pArg;
    sqlite3_stmt * stat;
    char szSql[255];
    sprintf(szSql, "SELECT * FROM Data_ChargeMeterValues Where Gun_Idx = %d AND Charge_Begin_Time = %d",
			pCond->nGunIdx, pCond->tmChargeStart);
    sqlite3_prepare(db, szSql, -1, &stat, NULL);
    BOOL bQueryOK = FALSE;
    //now re-query the meterVal-Record
    HIS_METER_VALUE_REC* pMeterValRec = &s_pFCGIRough->chargerDataRepo.merterVal_Rec;
	if(sqlite3_step(stat) == SQLITE_ROW)
    {
        int nRecSize = sqlite3_column_bytes(stat, 3);
        int nCompressSize = sqlite3_sizeof_blob_uncompressed(stat, 3);
        if(nCompressSize > 0)
        {   
            bQueryOK = TRUE;
            pMeterValRec->nGunNum = sqlite3_column_int(stat, 1);
			pMeterValRec->tmChargeStart = sqlite3_column_int(stat, 2);
			sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&pMeterValRec->meterData, sizeof(CHARGE_METERITEM_LIST));
            //TRACEX("----parsedCPOMMMOK---\n");
        }    
        else if(nRecSize >= sizeof(CHARGE_METERITEM_LIST))    
        {
            bQueryOK = TRUE;
            pMeterValRec->nGunNum = sqlite3_column_int(stat, 1);
			pMeterValRec->tmChargeStart = sqlite3_column_int(stat, 2);
			memcpy(&pMeterValRec->meterData, sqlite3_column_blob(stat, 3), 
				sizeof(CHARGE_METERITEM_LIST));
            //TRACEX("----parsedNNNNNNNOOCPOMMMOK---\n");
        }
    }   

    TRACEX("\n-----The sql[%s], bQueryOK[%d], [%d, %d, %d]-----\n", 
        szSql, bQueryOK, s_pFCGIRough->chargerDataRepo.merterVal_Rec.nGunNum, 
        s_pFCGIRough->chargerDataRepo.merterVal_Rec.tmChargeStart,
        s_pFCGIRough->chargerDataRepo.merterVal_Rec.meterData.nMeterValTotal); 

    sqlite3_finalize(stat);
    
    return bQueryOK;
}
*/
static BOOL Web_QueryChgRunData_ByGunAndStartTime(sqlite3* db, void* pArg)
{
    //UNUSED(pArg);
    BOOL bQueryOK = FALSE;
    
    QUERYCOND_HISMETERVAL* pCond = (QUERYCOND_HISMETERVAL*)pArg;
    sqlite3_stmt * stat;
    char szSql[255];
    //sprintf(szSql, "SELECT * FROM Data_RealTimeBmsData Where Gun_Idx = %d AND Charge_Begin_Time = %d",
	//		pCond->nGunIdx, pCond->tmChargeStart);

    //int nIdx = 1;
    sprintf(szSql, "SELECT * FROM Data_RealTimeBmsData Where Day = %d", pCond->nGunIdx);

    sqlite3_prepare_v2(db, szSql, -1, &stat, NULL);
    //now re-query the chagRun-Record
    HIS_BMS_RUNTIMEDATA_REC* pBmsRunData_Rec = &s_pFCGIRough->chargerDataRepo.bmsRunData_Rec;
	if(sqlite3_step(stat) == SQLITE_ROW)
    {
        int nRecSize = sqlite3_column_bytes(stat, 3);
        int nCompressSize = sqlite3_sizeof_blob_uncompressed(stat, 3);
        //printf("nRecSize=[%d] nCompressSize=[%d]\n", nRecSize, nCompressSize);
        if(nCompressSize > 0)
        {   
            bQueryOK = TRUE;
            pBmsRunData_Rec->nDay = sqlite3_column_int(stat, 1);
			pBmsRunData_Rec->tmLogTime = sqlite3_column_int(stat, 2);
            // LJH240925 RunData LJH250303 delete BMS_RUNTIMEDATA_LIST_V1
            // if (nCompressSize == sizeof(BMS_RUNTIMEDATA_LIST_V1))
            // {
            //     BMS_RUNTIMEDATA_LIST_V1 bmsRunData_V1;
            //     ZERO_POBJS(&bmsRunData_V1, 1);

            //     sqlite3_parse_blob_uncompressed(stat, 3, 
            //             (void*)&bmsRunData_V1, sizeof(BMS_RUNTIMEDATA_LIST_V1));
            //     int n = 0;
            //     pBmsRunData_Rec->bmsRunData.nBmsRuntimeDataTotal = bmsRunData_V1.nBmsRuntimeDataTotal;
            //     for (n=0; n<bmsRunData_V1.nBmsRuntimeDataTotal; ++n)
            //     {
            //         memcpy(&pBmsRunData_Rec->bmsRunData.stBmsRuntimeDataList[n],
            //             &bmsRunData_V1.stBmsRuntimeDataList[n], sizeof(A_BMS_RUNTIME_DATA_ITEM_V1));
            //     }
            // }
            // else
            if (nCompressSize == sizeof(BMS_RUNTIMEDATA_LIST))
            {
                sqlite3_parse_blob_uncompressed(stat, 3, 
					(void*)&pBmsRunData_Rec->bmsRunData, sizeof(BMS_RUNTIMEDATA_LIST));
            }
            TRACEX("----parsedCPOMMMOK---nDay=[%d] tmLogTime=[%ld]\n", pBmsRunData_Rec->nDay, pBmsRunData_Rec->tmLogTime);
        }    
    }   

    // TRACEX("\n-----The sql[%s], bQueryOK[%d], [%d, %d, %d]-----\n", 
    //     szSql, bQueryOK, s_pFCGIRough->chargerDataRepo.merterVal_Rec.nGunNum, 
    //     s_pFCGIRough->chargerDataRepo.merterVal_Rec.tmChargeStart,
    //     s_pFCGIRough->chargerDataRepo.merterVal_Rec.meterData.nMeterValTotal); 

    sqlite3_finalize(stat);
    
    return bQueryOK;
}


BOOL QueryChgRunData_byCond(QUERYCOND_HISMETERVAL* pCond)
{
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryChgRunData_ByGunAndStartTime, pCond))
    {
        return FALSE; 
    }

    return TRUE;
}

static BOOL Web_QueryIMMU2HisVerData(sqlite3* db, void* pArg)
{
    UNUSED(pArg);
    
    sqlite3_stmt * stat;
    sqlite3_prepare_v2(db, "select * from Revisions order by Idx DESC", -1, &stat, NULL);
    int nCount = 0;
    IMMU2_HISVERITEM* pHisVer = s_pFCGIRough->chargerDataRepo.hisIMMU2VerList;

	while(sqlite3_step(stat) == SQLITE_ROW)
    {
        char* szDate = (char*)sqlite3_column_text(stat, 4);
        pHisVer->tmReleaseDate = StringToTime(szDate, SQLITE_DATE_FMT); 
        szDate = (char*)sqlite3_column_text(stat, 2);
        snprintf(pHisVer->szCurrVer, 32, "%s", szDate);
        szDate = (char*)sqlite3_column_text(stat, 6);
        snprintf(pHisVer->szReleaseNotes, 128, "%s", szDate);

        // char szTime[32];
        // TRACEX("---%d \t%s \t%s \t%s---\n", sqlite3_column_int(stat, 0), 
        //     pHisVer->szCurrVer,
        //     TimeToString(pHisVer->tmReleaseDate, SQLITE_DATE_FMT, szTime, 32),
        //     pHisVer->szReleaseNotes);

        nCount++;
        pHisVer++;
        if(nCount >= WEB_MAX_SUPPORT_IMMUVERHIST)
        {
            break;
        } 
    }    
    TRACEX("-------Get IMMU2HISVER total: %d-------\n", nCount);
    s_pFCGIRough->chargerDataRepo.nIMMU2HisVerTotal = nCount;

    sqlite3_finalize(stat);
    
    return TRUE;
}


BOOL QueryMeterVal_byCond(QUERYCOND_HISMETERVAL* pCond)
{
    //if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryMeterVals_ByGunAndStartTime, pCond))
    //{
    //    return FALSE; 
    //}

    return TRUE;
}

static BOOL Wait_API_CommReady(FCGI_ROUGH_DATA* pFCGIRough)
{
    static BOOL bIsIPCOpenOK = FALSE;
    if(!bIsIPCOpenOK)
    {
        int nErrCode;
        pFCGIRough->hIPC_CommPort = Web_IPC_CommOpen("COM_WEB_IPC", WEB_IPC_KEY, 
            COMM_CLIENT_MODE, WEB_IPC_RW_TIMEOUT_MSEC, &nErrCode);
        if(pFCGIRough->hIPC_CommPort == NULL)
        {
            return FALSE;
        }
    }

    bIsIPCOpenOK = TRUE;

    //Step 1: get version info
    if(!IPC_GetVersionInfos())
    {
        TRACEW("IPC_GetVersionInfos FALSE");
        //Web_IPC_CommClose(pFCGIRough->hIPC_CommPort);
        return FALSE;
    }
    

    //Step 2: get devInfo
    if(!IPC_GetAllDevInfos())
    {        
        
        return FALSE;
    }
    
    
    //Step 3: get paramInfo
    if(!IPC_GetAllParamInfos())
    {
        
        return FALSE;
    }
    

    //Step 4: get alarmInfo
    if(!IPC_GetAllAlarmInfos())
    {
        
        return FALSE;       
    }
    
    
    
    if(!Process_DB_OP(WEB_DB_TYPE_SYSTEM, GetWeb_MainIdx, NULL))
    {
        
        return FALSE;             
    }
    
    
    
    if(!Process_DB_OP(WEB_DB_TYPE_SYSTEM, GetWeb_SubIdx, NULL))
    {
        
        return FALSE;             
    }
    

    //Step 5: now create structures for all static
    
    if(!Init_CreateStructs_4WebStatic())
    {
        
        return FALSE;             
    }
    

    //Step 6: get all the param init values
    
    if(!IPC_GetAllParamVals())
    {
        
        return FALSE;             
    }
    

    
    if(!IPC_GetAllParamVals_NewWeb())
    {
        
        return FALSE;             
    }

    
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryActiveAlarms, NULL))
    {
        
        return FALSE; 
    }

    //Step 8: get oringinal BMS Rec
    
    if(!Process_DB_OP(WEB_DB_TYPE_BMS_DATA, Web_Query_BmsRecords, NULL))
    {
        
        return FALSE; 
    }

    //Step 9: get timeZone offset
    
    Get_OrRefresh_TimeZoneOffset();

    //Step 10: get hisAlarms    
    
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryHistAlarms, NULL))
    {
        
        return FALSE; 
    }

    //Step 11: get syslog    
    
    if(!Process_DB_OP(WEB_DB_TYPE_APP_LOG, Web_QuerySysLogs, NULL))
    {
        
        return FALSE; 
    }

    //Step 12: get ocppLog    
    
    if(!Process_DB_OP(WEB_DB_TYPE_APP_LOG, Web_QueryOCPPLogs, NULL))
    {
        
        return FALSE; 
    }

    //Step 13: get hisEvent    
    
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryHisEvents, NULL))
    {
        
        return FALSE; 
    }

    //Step 14: get hisData    
    
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryHisData, NULL))
    {
        
        return FALSE; 
    }

    //Step 15: get localList
    //if(!IPC_GetLocalList())
    //{
    //    //Web_IPC_CommClose(pFCGIRough->hIPC_CommPort);
    //    //return FALSE;
    //    TRACEX("----GET LOCALLIST FAILED!!!!---\n");
    //}

    //Step 16: get localCache
    //if(!IPC_GetLocalCache())
    //{
    //    //Web_IPC_CommClose(pFCGIRough->hIPC_CommPort);
    //    //return FALSE;
    //    TRACEX("----GET LOCALCACHE FAILED!!!!---\n");
    //}

    //Step 17: get IMMU2 hisVer table
    
    if(!Process_DB_OP(WEB_DB_TYPE_SYSTEM, Web_QueryIMMU2HisVerData, NULL))
    {
        
        return FALSE; 
    }

    //Step 18: get chargeProfiles    
   /*if(!IPC_GetChgProfiles())
    {
        //Web_IPC_CommClose(pFCGIRough->hIPC_CommPort);
        //return FALSE;
        TRACEX("----GET CHGPROFILES FAILED!!!!---\n");
    }
    */

    
    //Step 19: get BMS hist fault records
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryBmsHistFault, NULL))
    {
        return FALSE; 
    }
    
    //Step 20: get BMS hist event records
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryBmsHistEvent, NULL))
    {
        return FALSE; 
    }
    return TRUE;
}

static BOOL IPC_GetMsg_FromMain(FCGI_ROUGH_DATA* pFCGIRough)
{
    WEB_FROMAAPP_MSG msg;
	ZERO_POBJS(&msg, 1);
    int nMsgSize = sizeof(WEB_FROMAAPP_MSG);    
    //read a msg from buffer
    int nReadBytes = Web_IPC_CommRead(pFCGIRough->hIPC_CommPort, (char*)&msg, nMsgSize);
    if(nReadBytes != nMsgSize)
    {		
        TRACEX("----NO msg got from reporter side %d----\n",nReadBytes);
        return FALSE;
    }
    TRACEX("---now received a msg from main---\n");
    //now check msg format
    if(msg.bySOI != WEB_IPC_MSG_SOI)
    {
        TRACEX("---Received wrong SOI[%d]---\n", msg.bySOI);
        return FALSE;
    }
    else if((msg.ulLength < 2) || (msg.ulLength > MAX_WEB_MSG_SIZE))
    {
        TRACEX("---Received wrong ulLength[%d]---\n", msg.ulLength);
        return FALSE;
    }

    //now check the chksum
    int n;
    UINT ulCalcChksum = 0;
    for(n = 0; n < msg.ulLength - 2; n++)
    {
        ulCalcChksum += msg.byEvData[n];
    }
    if(ulCalcChksum != msg.ulChkSum)
    {
        TRACEX("---Received wrong ulChkSum[%d, %s]---\n", msg.ulChkSum, ulCalcChksum);
        return FALSE;        
    }
    //whatever event, should update the log&hisEvent
    pFCGIRough->bNeedUpdateSysLog = TRUE;
    pFCGIRough->bNeedUpdateHisEvent = TRUE;
	pFCGIRough->bNeedUpdateOCPPLog = TRUE; //added by Jimmy 20220321

    //now get evTy and msgTp
    if(EVENT_TYPE_CFG_CHANGED == msg.byEvType)
    {
        EVENT_PARAM_CHG* pCfgChg = (EVENT_PARAM_CHG*)msg.byEvData;
        TRACEX("---received CFG_CHG ,paramId[%d] event from main---\n", pCfgChg->parmPTR.nSubId);
        if(pCfgChg->parmPTR.nSubId == 15)
        {
            pFCGIRough->bNeedUpdateChgProfiles = TRUE;
        }
        else if(pCfgChg->parmPTR.nSubId == 6)
        {
            pFCGIRough->bNeedUpdateLocalList = TRUE;
        }
        else if(pCfgChg->parmPTR.nSubId == 7)
        {
            pFCGIRough->bNeedUpdateCache = TRUE;
        }
        else if(pCfgChg->parmPTR.nId > 700 && pCfgChg->parmPTR.nId < 705)
        {
            pFCGIRough->bNeedUpdataPCSReread = TRUE;
            pFCGIRough->nUpdateDevId = pCfgChg->parmPTR.nId;
            pFCGIRough->nUpdateParamId = pCfgChg->parmPTR.nSubId;
        }
        else
        {
            pFCGIRough->bNeedRefreshCfg = TRUE;
        }        
    }
    else if(EVENT_TYPE_ALARM == msg.byEvType)
    {
        TRACEX("---received EVENT_ALARM---\n");
        pFCGIRough->bIsAlarmNeedRefresh = TRUE;
        pFCGIRough->bIsHisAlarmNeedRefresh = TRUE;
    }
    else if(EVENT_TYPE_BMS_HISTFAULT == msg.byEvType)
    {
        TRACEX("---received EVENT_BMS_HISTFAULT---\n");
        pFCGIRough->bIsBmsHistFaultNeedRefresh = TRUE;
    }
    else if(EVENT_TYPE_BMS_HISTEVENT == msg.byEvType)
    {
        TRACEX("---received EVENT_BMS_HISTEVENT---\n");
        pFCGIRough->bIsBmsHistEventNeedRefresh = TRUE;
    }
   //else if(EVENT_TYPE_BMSRUNDATA_UPDATE == msg.byEvType)//MsgSize is Over 4096,cause Not Run To Here.
   //{
   //    pFCGIRough->bNeedUpdateBmsRecord = TRUE;
   //    printf("!!!EVENT_TYPE_BMSRUNDATA_UPDATE STATUS_CHANGED\n");
   //}
   //else if(EVENT_TYPE_HEALTHDATA_UPDATE == msg.byEvType)//MsgSize is Over 4096,cause Not Run To Here.
   //{
   //    pFCGIRough->bNeedUpdateHealthRec = TRUE;
   //}
    //else if(EVENT_TYPE_BMSRUNDATA_UPDATE == msg.byEvType)
    //{
    //    //need update hisData when charging related event happened
    //    //pFCGIRough->bNeedUpdateHisData = TRUE;
    //    printf("msg.byEvType=[%d] msg.byMsgType=[%d]\n", msg.byEvType, msg.byMsgType);
    //    if(EVENT_MSG_STATUS_CHANGED == msg.byMsgType)
    //    {
    //        TRACEX("---received CHAGEING %s event---\n", 
    //            (EVENT_MSG_BEGIN == msg.byMsgType) ? "begin" : "STATUS Changed");
    //        pFCGIRough->bNeedUpdateBmsRecord = TRUE;
    //    }
    //}

    return TRUE;
}

void* Thread_WebComm(void* pArgs)
{
	FCGI_ROUGH_DATA* pFCGIRough = (FCGI_ROUGH_DATA*)pArgs;
    //TRACEX("---------INIT thread for web OKKKKK----\n");
    double tmNow = 0.0;
    int n;
    static double s_tmLastRefreshAllParmVal = 0.0;
    static double s_tmLastRefreshSyslog = 0.0;
    static double s_tmLastRefreshOCPPlog = 0.0;
    static double s_tmLastRefreshHisEvent = 0.0;
    static double s_tmLastRefreshHisData = 0.0;
    static double s_tmLastRefreshVersionInfo = 0.0;
    static double s_tmLastRefreshBmsRec = 0.0;
    static double s_tmLastRefreshHealth = 0.0;
    static double s_tmLastQuery_ActAlarms = 0.0;
    static double s_tmLastQuery_HisAlarms = 0.0;
    static double s_tmLastQuery_RereadPCSParam = 0.0;
    static double s_tmLastRefreshBmsHistFault = 0.0;
    static double s_tmLastRefreshBmsHistEvent = 0.0;
    while(!(*pFCGIRough->p_bIsNeedExit))
    {
        tmNow = GetCurrentTime();
        //now try to get msg from main
        //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE); 
        if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
        {
            Sleep_Ex(100);
            continue;
        }
        Set_IPCMode(TRUE);
        IPC_GetMsg_FromMain(pFCGIRough);
        Mutex_Unlock(pFCGIRough->hDataLock);
        
        //step1 :check if session timeout
        //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);    
        if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
        {
            Sleep_Ex(100);
            continue;
        }     
        for(n = 0; n < pFCGIRough->nActiveUserTotal; n++)
        {
            if(pFCGIRough->stSessionInfos[n].bIsInHandling_HttpRequst)
            {
                continue;
            }
            else if(ABS(_NOW_ - pFCGIRough->stSessionInfos[n].tmLastReceiveRequest) > WEB_NO_REQ_TIMEOUT_SEC)
            {
                pFCGIRough->stSessionInfos[n].tmLastReceiveRequest = _NOW_;
                pFCGIRough->stSessionInfos[n].nNoRecievedRequstCounter++;
                if(pFCGIRough->stSessionInfos[n].nNoRecievedRequstCounter >= 10) //changed by Jimmy 20210608 from 60 => 300 sec to avoid wrong timeout during DOWNLOADING process
                {
                    DeleteSession(pFCGIRough, n, FALSE);
                }                
            }
        }
        Mutex_Unlock(pFCGIRough->hDataLock);

        //Step2: refresh all param vals
        if((fabs(tmNow - s_tmLastRefreshAllParmVal) > 1.0) ||
            pFCGIRough->bNeedRefreshCfg)
        {
            //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
            if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
            {
                Sleep_Ex(100);
                continue;
            }
            IPC_GetAllParamVals();
            IPC_GetAllParamVals_NewWeb();
            IPC_GetReadbackValue();
            if(pFCGIRough->bNeedRefreshCfg)
            {
                Get_OrRefresh_TimeZoneOffset();
            }
            s_tmLastRefreshAllParmVal = tmNow;
            pFCGIRough->bNeedRefreshCfg = FALSE;
            Mutex_Unlock(pFCGIRough->hDataLock);
        }
        
        //added by Jimmy 20210112 for recifier info update
        if(fabs(tmNow - s_tmLastRefreshVersionInfo) >= 10.0)
        {
            //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
            if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
            {
                Sleep_Ex(100);
                continue;
            }
            IPC_GetVersionInfos();  
            s_tmLastRefreshVersionInfo = tmNow;
            Mutex_Unlock(pFCGIRough->hDataLock);     
        }

        //Step3: check/refresh active alarms
        if(fabs(tmNow - s_tmLastQuery_ActAlarms) > 10.0)
        {
        	if(s_pFCGIRough->bIsAlarmNeedRefresh)
        	{
            //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
            if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
            {
                Sleep_Ex(100);
                continue;
            }
            Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryActiveAlarms, NULL);
			s_tmLastQuery_ActAlarms = tmNow;
            s_pFCGIRough->bIsAlarmNeedRefresh = FALSE;
            Mutex_Unlock(pFCGIRough->hDataLock);
        	}
		}
		if(fabs(tmNow - s_tmLastQuery_HisAlarms) > 10.0)
        {
		    if(s_pFCGIRough->bIsHisAlarmNeedRefresh)
	        {
	            //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
	            if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
	            {
	                Sleep_Ex(100);
	                continue;
	            }
	            Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryHistAlarms, NULL);
	            s_tmLastQuery_HisAlarms = tmNow;
	            s_pFCGIRough->bIsHisAlarmNeedRefresh = FALSE;
	            Mutex_Unlock(pFCGIRough->hDataLock);
	        }
		}
        
		// BMS hist fault records: event-driven, rate-limited (10s min interval)
		if(fabs(tmNow - s_tmLastRefreshBmsHistFault) > 10.0)
		{
		    if(s_pFCGIRough->bIsBmsHistFaultNeedRefresh)
		    {
		        if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
		        {
		            Sleep_Ex(100);
		            continue;
		        }
		        Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryBmsHistFault, NULL);
		        s_tmLastRefreshBmsHistFault = tmNow;
		        s_pFCGIRough->bIsBmsHistFaultNeedRefresh = FALSE;
	            Mutex_Unlock(pFCGIRough->hDataLock);
	        }
		}
        
		// BMS hist event records: event-driven, rate-limited (10s min interval)
		if(fabs(tmNow - s_tmLastRefreshBmsHistEvent) > 10.0)
		{
		    if(s_pFCGIRough->bIsBmsHistEventNeedRefresh)
		    {
		        if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
		        {
		            Sleep_Ex(100);
		            continue;
		        }
		        Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryBmsHistEvent, NULL);
		        s_tmLastRefreshBmsHistEvent = tmNow;
		        s_pFCGIRough->bIsBmsHistEventNeedRefresh = FALSE;
	            Mutex_Unlock(pFCGIRough->hDataLock);
	        }
		}
        
		if(fabs(tmNow - s_tmLastRefreshBmsRec) >= 1800.0) 
        {
			//if(s_pFCGIRough->bNeedUpdateBmsRecord)
			{
	            //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
	            if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
	            {
	                Sleep_Ex(100);
	                continue;
	            }
                Process_DB_OP(WEB_DB_TYPE_BMS_DATA, Web_Query_BmsRecords, NULL);
	            s_tmLastRefreshBmsRec = tmNow;
				//s_pFCGIRough->bNeedUpdateBmsRecord = FALSE;
	            Mutex_Unlock(pFCGIRough->hDataLock);            
			}
        }
        
        if(fabs(tmNow - s_tmLastRefreshHealth) >= 86400.0)
        {
            //if(pFCGIRough->bNeedUpdateHealthRec)//MSg Size is Over 4096.Web_Main Not Pro This Event
            {
                //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
                if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
                {
                    Sleep_Ex(100);
                    continue;
                }
                Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_Query_HealthRec, NULL);
                s_tmLastRefreshHealth = tmNow;
                //pFCGIRough->bNeedUpdateHealthRec = FALSE;
                Mutex_Unlock(pFCGIRough->hDataLock);   
            }        
        }
        
        if(fabs(tmNow - s_tmLastRefreshSyslog) > 30.0)
        {
            if(s_pFCGIRough->bNeedUpdateSysLog || (fabs(tmNow - s_tmLastRefreshSyslog) > 3600.0))
            {
                //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
                if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
                {
                    Sleep_Ex(100);
                    continue;
                }
                Process_DB_OP(WEB_DB_TYPE_APP_LOG, Web_QuerySysLogs, NULL);
                s_tmLastRefreshSyslog = tmNow;
                s_pFCGIRough->bNeedUpdateSysLog = FALSE;
				s_pFCGIRough->chargerDataRepo.bIsNoticeAPI_ForceRequery_SysLog = TRUE;
                Mutex_Unlock(pFCGIRough->hDataLock);              
            }
        }

        if(fabs(tmNow - s_tmLastRefreshOCPPlog) > 30.0)
        {
            if(s_pFCGIRough->bNeedUpdateOCPPLog || (fabs(tmNow - s_tmLastRefreshOCPPlog) > 7200.0))
            {
                //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
                if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
                {
                    Sleep_Ex(100);
                    continue;
                }
                Process_DB_OP(WEB_DB_TYPE_APP_LOG, Web_QueryOCPPLogs, NULL);
                s_tmLastRefreshOCPPlog = tmNow;
                s_pFCGIRough->bNeedUpdateOCPPLog = FALSE;
				s_pFCGIRough->chargerDataRepo.bIsNoticeAPI_ForceRequery_OCPPLog = TRUE;
                Mutex_Unlock(pFCGIRough->hDataLock);              
            }
        }

        if(fabs(tmNow - s_tmLastRefreshHisEvent) > 60.0)
        { 
            if(s_pFCGIRough->bNeedUpdateHisEvent || (fabs(tmNow - s_tmLastRefreshHisEvent) > 86400.0))
            {
                //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
                if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
                {
                    Sleep_Ex(100);
                    continue;
                }
                Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryHisEvents, NULL);
                s_tmLastRefreshHisEvent = tmNow;
                s_pFCGIRough->bNeedUpdateHisEvent = FALSE;
				s_pFCGIRough->chargerDataRepo.bIsNoticeAPI_ForceRequery_HisEvent = TRUE;
                Mutex_Unlock(pFCGIRough->hDataLock);              
            }
        }

        if(fabs(tmNow - s_tmLastRefreshHisData) > 60.0)
        {
            if(s_pFCGIRough->bNeedUpdateHisData || (fabs(tmNow - s_tmLastRefreshHisData) > 86400.0))
            {
                //Mutex_Lock(pFCGIRough->hDataLock, WAIT_INFINITE);
                if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
                {
                    Sleep_Ex(100);
                    continue;
                }
                Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, Web_QueryHisData, NULL);
                s_tmLastRefreshHisData = tmNow;
                s_pFCGIRough->bNeedUpdateHisData = FALSE;
				s_pFCGIRough->chargerDataRepo.bIsNoticeAPI_ForceRequery_HisData = TRUE;
                Mutex_Unlock(pFCGIRough->hDataLock);              
            }
        }

        if(fabs(tmNow - s_tmLastQuery_RereadPCSParam) > 10.0)
        {
		    if(s_pFCGIRough->bNeedUpdataPCSReread)
	        {
	            if(Mutex_Lock(pFCGIRough->hDataLock, 50) != ERR_MUTEX_OK)
	            {
	                Sleep_Ex(100);
	                continue;
	            }
	            Process_DB_OP(FALSE, Web_QueryPCSRereadParam, NULL);
	            s_tmLastQuery_RereadPCSParam = tmNow;
	            s_pFCGIRough->bNeedUpdataPCSReread = FALSE;
	            Mutex_Unlock(pFCGIRough->hDataLock);                 
	        }
		}

        Sleep_Ex(100);
    }

    return NULL;
}

BOOL Init_DataExchange(FCGI_ROUGH_DATA* pFCGIRough)
{ 
    
    s_pFCGIRough = pFCGIRough;
    sqlite3_config(SQLITE_CONFIG_SERIALIZED); //Serieal mode, only one handle should be used by different threads
	sqlite3_initialize();
    //just try if we can open the DB cfg&data OK
    
    if(!Process_DB_OP(WEB_DB_TYPE_SYSTEM, NULL, NULL))
    {
        
        LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_MILESTONE,
                    "11");
        
        return FALSE;
    }
    
    
    if(!Process_DB_OP(WEB_DB_TYPE_NCCU_DATA, NULL, NULL))
    {     
        
        LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_MILESTONE,
            "22");   
        TRACE("222\n");
        return FALSE;
    }
    
    

    //Step1: load default english resource table
    if(Load_LangResources(pFCGIRough, LANG_TYPE_ENGLISH) == NULL)
    {
        
        LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_MILESTONE,
                    "33");
        TRACE("333\n");
        return FALSE;
    }
    
    
    // if(!Load_LangResources(pFCGIRough, LANG_TYPE_CHINESE))
    // {
    //     return FALSE;
    // }

    //set defaultLang => English
    s_pFCGIRough->nInitLangSel = LANG_TYPE_ENGLISH;

    //now wait the charger init
    pFCGIRough->bIsChargerRunData_Ready = FALSE;
    //for(;;)
    {
        if(*pFCGIRough->p_bIsNeedExit)
        {
            //break;
            
            TRACE("444\n");
            return FALSE;
        }
        //now wait API reporter comm init
        if(!Wait_API_CommReady(pFCGIRough))
        {
            //break;
            
            TRACE("Wait_API_CommReady FALSE\n");
            return FALSE;
        }
    }
    
    //now get other init data from DB(SQL query)
    //InitSysLog and others
    pFCGIRough->bIsChargerRunData_Ready = TRUE;
    // LJH241101 MU3
    
    pFCGIRough->emSystemType = SYSTYPE_ESS_AIR;
    Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 100, 4, (void*)&pFCGIRough->emSystemType);
    UINT uSlaveSerial = 0;
    Get_ASingle_ParamVal(PARAM_TYPE_SETTING, 1, 42, 4, (void*)&uSlaveSerial);
    pFCGIRough->bClientLC = (uSlaveSerial==0 ? FALSE : TRUE);
    TRACED("rundata_exchange:Init_DataExchange emSystemType:%d bClientLC:%d", pFCGIRough->emSystemType, pFCGIRough->bClientLC);
    //Step2: create a data_comm thread and run it!
    
    if (pthread_create(&pFCGIRough->hThead_DataComm, 
		NULL, Thread_WebComm,
		(void *)pFCGIRough) != 0)
	{
        
		return FALSE;
	}
	pthread_detach(pFCGIRough->hThead_DataComm);


    return TRUE;
}

WEB_PARAM_VALUE* Get_ASingle_WebParaVal(IN PARAM_TYPE emParTp, IN int nDevId, IN int nParamId)
{
    int n,k;
    int nDevTotal = s_pFCGIRough->chargerDataRepo.nDevInfoTotal;
    WEB_RUN_DEV* pRunDev = s_pFCGIRough->chargerDataRepo.pAllRunDevs;
    int nParamTotal;
    WEB_PARAM_VALUE* pWebParamVal = NULL;
    for(n = 0; n < nDevTotal; n++, pRunDev++)
    {
        if(pRunDev->pDevInfo->nDevId == nDevId)
        {
            nParamTotal = (emParTp == PARAM_TYPE_STATUS) ? pRunDev->nStatusParamTotal :
                ((emParTp == PARAM_TYPE_SETTING) ? pRunDev->nSettParamTotal : pRunDev->nCtrlParamTotal);
            pWebParamVal = (emParTp == PARAM_TYPE_STATUS) ? pRunDev->pStatusParamList :
                ((emParTp == PARAM_TYPE_SETTING) ? pRunDev->pSettParamList : pRunDev->pCtrlParamList);
            for(k = 0; k < nParamTotal; k++, pWebParamVal++)
            {
                if(pWebParamVal->pParamInfo->nParamId == nParamId)
                {
                    return pWebParamVal;
                }
            }
            break;
        }
    } 

    return NULL;
}

A_WEBBLOB_PARAM* Get_ABlobParam_ByParamVal(WEB_PARAM_VALUE* pParamVal)
{
    int n;
    for(n = 0; n < s_pFCGIRough->nBlobParamTotal; n++)
    {
        if(s_pFCGIRough->blobParamList[n].pParamVal == pParamVal)
        {
            return &s_pFCGIRough->blobParamList[n];
        }
        //TRACE("nBlobParamTotal=%d nDevModelId=[%d] nParamId=[%d]\n", \
        //    s_pFCGIRough->nBlobParamTotal, \ 
        //    s_pFCGIRough->blobParamList[n].pParamVal->pParamInfo->nDevModelId,\
        //   s_pFCGIRough->blobParamList[n].pParamVal->pParamInfo->nParamId);
    }
    return NULL;
}

A_WEBBLOB_PARAM* Get_ABlobParam_ByInfo(IN PARAM_TYPE emParTp, IN int nDevId, IN int nParamId)
{
    WEB_PARAM_VALUE* pWebParamVal = Get_ASingle_WebParaVal(emParTp, nDevId, nParamId);
    if(pWebParamVal != NULL)
    {
        return Get_ABlobParam_ByParamVal(pWebParamVal);
    }

    return NULL;
}


BOOL Get_ASingle_ParamVal(IN PARAM_TYPE emParTp, IN int nDevId, IN int nParamId, int nGetValSize, OUT void* pVal)
{
    WEB_PARAM_VALUE* pWebParamVal = Get_ASingle_WebParaVal(emParTp, nDevId, nParamId);
    if(pWebParamVal != NULL)
    {
        // TRACEX("----GET OK for[%d,%d,%d], valSize=>[%d,%d]---\n", emParTp, nDevId, nParamId,
        //     pWebParamVal->pParamInfo->nValueSize, nGetValSize);

        if(pWebParamVal->pParamInfo->nValueSize <= nGetValSize)
        {
            if(pWebParamVal->pParamInfo->emValType >= WEB_VALTYPE_OBJECT)
            {
                memcpy(pVal, pWebParamVal->currValue.blobValue, pWebParamVal->pParamInfo->nValueSize);
            }
            else
            {
                memcpy(pVal, pWebParamVal->currValue.abyValue, MIN(4, pWebParamVal->pParamInfo->nValueSize));
            }            
            return TRUE;
        }       
    }

    return FALSE;
}

void Force_Update_ParamVals(void)
{
    //do not lock here, as the session is Locked Already
    //Mutex_Lock(s_pFCGIRough->hDataLock, WAIT_INFINITE);
    IPC_GetAllParamVals();
    IPC_GetAllParamVals_NewWeb();
    IPC_GetReadbackValue();
    //Mutex_Unlock(s_pFCGIRough->hDataLock);
}

time_t Conv_UTC_2LocalTime(time_t tmUTC)
{    
    return tmUTC + (time_t)(s_pFCGIRough->fTimeOffsetHrs * 3600);
}

double Conv_UTC_2LocalTime_D(double tmDUTC)
{    
    return tmDUTC + (double)(s_pFCGIRough->fTimeOffsetHrs * 3600);
}

int GetClass_byDevId(int nDevid)
{
    int n;
    int nDevTotal = s_pFCGIRough->chargerDataRepo.nDevInfoTotal;
    WEB_RUN_DEV* pRunDev = s_pFCGIRough->chargerDataRepo.pAllRunDevs;
    for(n = 0; n < nDevTotal; n++, pRunDev++)
    {
        if(pRunDev->pDevInfo->nDevId == nDevid)
        {
            return pRunDev->pDevInfo->emDevClass;
        }
    }

    return 0;
}

int Handle_SingleParam_SetCtrl(WEB_PARAM_VALUE* pWebParamVal, int nDevId, void* pSetVal, int nValLen)
{
    if(pWebParamVal == NULL)
    {
        return WEB_ERRCODE_WRONG_FORMAT;
    }
    //juege if auth level is OK first
    int nUserLevel = s_pFCGIRough->pCurrSess->emUserLvel;
    BYTE byMask = 0;
    if(nUserLevel == USER_LEVEL_ENGINEER)
    {
        byMask = (pWebParamVal->pParamInfo->uMask_AuthorityLevel & LEVEL_AUTHORITY_MASK_BIT_READ);
    }
    else if(nUserLevel == USER_LEVEL_ADMIN)
    {
        byMask = ((pWebParamVal->pParamInfo->uMask_AuthorityLevel >> 2) & LEVEL_AUTHORITY_MASK_BIT_READ);
    }
    //if only readable, then disable the display
    if(byMask == LEVEL_AUTHORITY_MASK_BIT_READ)
    {
        return WEB_ERRCODE_WRONG_AUTHLEVEL;
    }

    BYTE byParamVal_SentData[MAX_WEB_REQDATA_SIZE];
    memset(byParamVal_SentData, 0, MAX_WEB_REQDATA_SIZE);
    VAR_VALUE varV;
    int nReqSendDataLen = 13;
    byParamVal_SentData[0] = (pWebParamVal->pParamInfo->emParamType == PARAM_TYPE_CONTROL) ? 1 : 0; //1 for ctrl, 0 for set
    varV.lValue = nDevId;
    memcpy(byParamVal_SentData + 1, varV.abyValue, 4);
    varV.ulValue = (UINT)pWebParamVal->pParamInfo->nParamId;
    memcpy(byParamVal_SentData + 5, varV.abyValue, 4);
    BYTE* pDataPos = byParamVal_SentData + 13;
    //now the value
    if(pWebParamVal->pParamInfo->emValType != WEB_VALTYPE_OBJECT)
    {        
        if((pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_BOOL) ||
            (pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_ENUM) ||
            (pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_INT))
        {
            varV.ulValue = (UINT)atoi((char*)pSetVal);
            if((varV.lValue > pWebParamVal->pParamInfo->fMaxLimit) ||
                (varV.lValue < pWebParamVal->pParamInfo->fMinLimit))
            {
                return WEB_ERRCODE_OUT_OF_RANGE;
            }
            nValLen = 4;
            memcpy(pDataPos, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
        else if((pWebParamVal->pParamInfo->emValType >= WEB_VALTYPE_FLOAT_0) &&
            (pWebParamVal->pParamInfo->emValType <= WEB_VALTYPE_FLOAT_4))
        {
            varV.fValue = (float)atof((char*)pSetVal);
            if((varV.fValue > pWebParamVal->pParamInfo->fMaxLimit) ||
                (varV.fValue < pWebParamVal->pParamInfo->fMinLimit))
            {
                return WEB_ERRCODE_OUT_OF_RANGE;
            }
            nValLen = 4;
            memcpy(pDataPos, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
        else if((pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_STRING) ||
            (pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_PASSWORD))
        {
            if(nValLen > pWebParamVal->pParamInfo->nValueSize)
            {
                return WEB_ERRCODE_WRONG_LEN;
            }
            memset(pDataPos, 0, pWebParamVal->pParamInfo->nValueSize);
            memcpy(pDataPos, pSetVal, nValLen);
            nValLen = MAX(pWebParamVal->pParamInfo->nValueSize, 4);//nValLen + 1;
            nReqSendDataLen += nValLen;//nValLen + 1;            
        }
        else if(pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_IP)
        {
            nValLen = 4;
            varV.ulValue = (UINT)inet_addr((char*)pSetVal);
            memcpy(pDataPos, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
        else if(pWebParamVal->pParamInfo->emValType == WEB_VALTYPE_DATETIME)
        {
            nValLen = 4;
            varV.ulValue = (UINT)StringToTime((char*)pSetVal, TIME_CHN_FMT);
            if(varV.ulValue == 0) //date-time set
            {
                return WEB_ERRCODE_WRONG_FORMAT;
            }
            memcpy(pDataPos, varV.abyValue, 4);
            nReqSendDataLen += 4;
        }
        else
        {
            //not supported yet
            return WEB_ERRCODE_WRONG_FORMAT;
        }        
    }
    else
    {
        memcpy(pDataPos, pSetVal, nValLen);
        nReqSendDataLen += nValLen;
    }
    
    //value length
    varV.ulValue = (UINT)nValLen;
    memcpy(byParamVal_SentData + 9, varV.abyValue, 4);
    TRACEX("---Now send the paramset[%d, %d, %d], val.ulVal->[%s] to main APP---\n",
        nDevId, pWebParamVal->pParamInfo->nParamId, nValLen, (char*)pSetVal);
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_SET_PARAM_SERETCTRL, byParamVal_SentData, nReqSendDataLen);    
    if(!Read_andParse_WebIPC_Rep(WEB_REQTYPE_SET_PARAM_SERETCTRL, MAX_WAIT_TIMEOUT_SEC_4SETCTRL))
    {
        return WEB_ERRCODE_TIMEOUT;
    }

    if(s_pFCGIRough->byParsedRTN != WEB_ERRCODE_OK)
    {
        return s_pFCGIRough->byParsedRTN;
    }

    if(s_pFCGIRough->pCurrSess != NULL)
    {
        LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_INFO,
                "User[%s, %c] just executed the [%s] cmd [%d, %d] successfully!",
                s_pFCGIRough->pCurrSess->szToken, 
                s_pFCGIRough->pCurrSess->bIsSess_FromNPS ? 'Y' : 'N',
                (pWebParamVal->pParamInfo->emParamType == PARAM_TYPE_CONTROL) ? "CTRL" : "SET", 
                nDevId, pWebParamVal->pParamInfo->nParamId);
    }

    return WEB_ERRCODE_OK;
}

int Handle_Special_SingleParam_SetCtrl(BOOL bIsSetParam, int nDevId, int nParamId, void* pSetVal, int nValLen)
{
    BYTE byParamVal_SentData[MAX_WEB_REQDATA_SIZE];
    memset(byParamVal_SentData, 0, MAX_WEB_REQDATA_SIZE);
    VAR_VALUE varV;
    int nReqSendDataLen = 13;
    byParamVal_SentData[0] = (bIsSetParam) ? 0 : 1; //1 for ctrl, 0 for set
    varV.lValue = nDevId;
    memcpy(byParamVal_SentData + 1, varV.abyValue, 4);
    varV.lValue = nParamId;
    memcpy(byParamVal_SentData + 5, varV.abyValue, 4);
    BYTE* pDataPos = byParamVal_SentData + 13;
    //now the value
    memcpy(pDataPos, pSetVal, nValLen);
    nReqSendDataLen += nValLen;    
    
    //value length
    varV.ulValue = (UINT)nValLen;
    memcpy(byParamVal_SentData + 9, varV.abyValue, 4);
    TRACEX("---Now send the spssss paramset[%d, %d, %d], val.ulVal->[%s] to main APP---\n",
        nDevId, nParamId, nValLen, (char*)pSetVal);
    IPC_Send_AReq_2Reporter(WEB_REQTYPE_SET_PARAM_SERETCTRL, byParamVal_SentData, nReqSendDataLen);    
    if(!Read_andParse_WebIPC_Rep(WEB_REQTYPE_SET_PARAM_SERETCTRL, MAX_WAIT_TIMEOUT_SEC_4SETCTRL_SPECIAL))
    {
        return WEB_ERRCODE_TIMEOUT;
    }

    if(s_pFCGIRough->byParsedRTN != WEB_ERRCODE_OK)
    {
        return s_pFCGIRough->byParsedRTN;
    }

    if(s_pFCGIRough->pCurrSess != NULL)
    {
        LogOut_Ex(MAIN_MODULE_WEB, "FCGI_Main", LOG_TYPE_INFO,
                "User[%s, %c] just executed the SPEC [%s] cmd [%d, %d] successfully!",
                s_pFCGIRough->pCurrSess->szToken, 
                s_pFCGIRough->pCurrSess->bIsSess_FromNPS ? 'Y' : 'N',
                bIsSetParam ? "SET" : "CTRL", nDevId, nParamId);
    }

    return WEB_ERRCODE_OK;
}

int parse_str_to_number(const char* str, const char* delim, int* numbers, int max_numbers) 
{
    char* str_copy = strdup(str); 
    if (str_copy == NULL) {
        return -1;
    }

    char* token = strtok(str_copy, delim);
    int count = 0;

    while (token != NULL && count < max_numbers) {
        numbers[count] = atol(token); 
        count++;
        token = strtok(NULL, delim);
    }

    free(str_copy); 
    return count; 
}

