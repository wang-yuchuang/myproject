#include "fcgi_defs.h"
#include <ctype.h>

void Add_Trans_Into_Dictionary(cJSON* pDict, int nNewResId)
{
    // int n;
    // int nTotal = cJSON_GetArraySize(pDict);
    // cJSON* pJJJ;
    // for(n = 0; n < nTotal; n++)
    // {
    //     cJSON* pJ = cJSON_GetArrayItem(pDict, n);
    //     if((pJJJ = cJSON_GetObjectItem(pJ, "id")) != NULL)
    //     {
    //         if(pJJJ->valueint == nNewResId)
    //         {
    //             return;
    //         }
    //     }
    // }

    // //now add the item
    // pJJJ = cJSON_CreateObject();
    // cJSON_AddNumberToObject(pJJJ, "id", nNewResId);
    // cJSON_AddStringToObject(pJJJ, "tr", LoadLangText_ById(nNewResId));
    // cJSON_AddItemToArray(pDict, pJJJ);
    cJSON* pJChild = pDict->child;
    char szId[8];
    snprintf(szId, 8, "%d", nNewResId);
    while(pJChild)
    {
        if(strcmp(pJChild->string, szId) == 0)
        {
            return;
        }
        pJChild = pJChild->next;
    }
    
    cJSON_AddStringToObject(pDict, szId, LoadLangText_ById(nNewResId));
}

char* GetStrVal_FromWebParamVal(WEB_PARAM_VALUE* pParamVal, char* pszOut, int nValLen)
{
    memset(pszOut, 0, nValLen);
    if(pParamVal == NULL)
    {
        TRACEX("---now we got a NULL paramVal---\n");
        return pszOut;
    }

    int nEnumVal, n;
    struct in_addr ip;
    char szTime[32];
    switch (pParamVal->pParamInfo->emValType)
    {
    case WEB_VALTYPE_BOOL:
        snprintf(pszOut, nValLen, "%s", (pParamVal->currValue.emValue) ? "true" : "false");
        break;
    case WEB_VALTYPE_INT:
        //do not judge UINT/INT
        snprintf(pszOut, nValLen, "%d", pParamVal->currValue.lValue);
        break;
    case WEB_VALTYPE_STRING:
    case WEB_VALTYPE_PASSWORD:
        snprintf(pszOut, nValLen, "%s", (char*)pParamVal->currValue.blobValue);
        break;
    case WEB_VALTYPE_FLOAT_0:
        snprintf(pszOut, nValLen, "%.0f", pParamVal->currValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_1:
        snprintf(pszOut, nValLen, "%.1f", pParamVal->currValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_2:
        snprintf(pszOut, nValLen, "%.2f", pParamVal->currValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_3:
        snprintf(pszOut, nValLen, "%.3f", pParamVal->currValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_4:
        snprintf(pszOut, nValLen, "%.4f", pParamVal->currValue.fValue);
        break;
    case WEB_VALTYPE_IP:
        ip.s_addr = pParamVal->currValue.ulValue;
        snprintf(pszOut, nValLen, "%s", inet_ntoa(ip));
        break;
    case WEB_VALTYPE_DATETIME:
        snprintf(pszOut, nValLen, "%s", TimeToString((time_t)pParamVal->currValue.ulValue, TIME_CHN_FMT, szTime, 32));
        break;
    case WEB_VALTYPE_ENUM:    
        nEnumVal = pParamVal->currValue.emValue;
        for(n = 0; n < pParamVal->pParamInfo->nEnumTotal; n++)
        {
            if(pParamVal->pParamInfo->enumList[n].emVal == nEnumVal)
            {
                //snprintf(pszOut, nValLen, "%s", LoadLangText_ById(pParamVal->pParamInfo->enumList[n].nEnumName_ResId));
                snprintf(pszOut, nValLen, "%d", nEnumVal);
                break;
            }
        }
        break;
    default:
        break;
    }
    return pszOut;
}

char* GetReadStrVal_FromWebParamVal(WEB_PARAM_VALUE* pParamVal, char* pszOut, int nValLen)
{
    memset(pszOut, 0, nValLen);
    if(pParamVal == NULL)
    {
        TRACEX("---now we got a NULL paramVal---\n");
        return pszOut;
    }

    int nEnumVal, n;
    struct in_addr ip;
    char szTime[32];
    switch (pParamVal->pParamInfo->emValType)
    {
    case WEB_VALTYPE_BOOL:
        snprintf(pszOut, nValLen, "%s", (pParamVal->ReadValue.emValue) ? "true" : "false");
        break;
    case WEB_VALTYPE_INT:
        //do not judge UINT/INT
        snprintf(pszOut, nValLen, "%d", pParamVal->ReadValue.lValue);
        break;
    case WEB_VALTYPE_STRING:
    case WEB_VALTYPE_PASSWORD:
        snprintf(pszOut, nValLen, "%s", (char*)pParamVal->ReadValue.blobValue);
        break;
    case WEB_VALTYPE_FLOAT_0:
        snprintf(pszOut, nValLen, "%.0f", pParamVal->ReadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_1:
        snprintf(pszOut, nValLen, "%.1f", pParamVal->ReadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_2:
        snprintf(pszOut, nValLen, "%.2f", pParamVal->ReadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_3:
        snprintf(pszOut, nValLen, "%.3f", pParamVal->ReadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_4:
        snprintf(pszOut, nValLen, "%.4f", pParamVal->ReadValue.fValue);
        break;
    case WEB_VALTYPE_IP:
        ip.s_addr = pParamVal->ReadValue.ulValue;
        snprintf(pszOut, nValLen, "%s", inet_ntoa(ip));
        break;
    case WEB_VALTYPE_DATETIME:
        snprintf(pszOut, nValLen, "%s", TimeToString((time_t)pParamVal->ReadValue.ulValue, TIME_CHN_FMT, szTime, 32));
        break;
    case WEB_VALTYPE_ENUM:    
        nEnumVal = pParamVal->ReadValue.emValue;
        for(n = 0; n < pParamVal->pParamInfo->nEnumTotal; n++)
        {
            if(pParamVal->pParamInfo->enumList[n].emVal == nEnumVal)
            {
                //snprintf(pszOut, nValLen, "%s", LoadLangText_ById(pParamVal->pParamInfo->enumList[n].nEnumName_ResId));
                snprintf(pszOut, nValLen, "%d", nEnumVal);
                break;
            }
        }
        break;
    default:
        break;
    }
    return pszOut;
}

char* GetPCSRereadStrVal_FromWebParamVal(WEB_PARAM_VALUE* pParamVal, WEB_PARAM_VALUE* pPCSRereadVal, char* pszOut, int nValLen)
{
    memset(pszOut, 0, nValLen);
    if(pParamVal == NULL)
    {
        TRACEX("---now we got a NULL paramVal---\n");
        return pszOut;
    }
    void* blobData = pPCSRereadVal->RereadValue.blobValue;
    int nEnumVal, n;
    struct in_addr ip;
    char szTime[32];
    switch (pParamVal->pParamInfo->emValType)
    {
    case WEB_VALTYPE_BOOL:
        pParamVal->RereadValue.emValue = *(int*)blobData;
        snprintf(pszOut, nValLen, "%s", pParamVal->RereadValue.emValue ? "true" : "false");
        break;
    case WEB_VALTYPE_INT:
        //do not judge UINT/INT
        pParamVal->RereadValue.lValue = *(long*)blobData;
        snprintf(pszOut, nValLen, "%d", pParamVal->RereadValue.lValue);
        break;
    case WEB_VALTYPE_STRING:
    case WEB_VALTYPE_PASSWORD:
        snprintf(pszOut, nValLen, "%s", (char*)pParamVal->RereadValue.blobValue);
        break;
    case WEB_VALTYPE_FLOAT_0:
        pParamVal->RereadValue.fValue = *(float*)blobData;
        snprintf(pszOut, nValLen, "%.0f", pParamVal->RereadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_1:
        pParamVal->RereadValue.fValue = *(float*)blobData;
        snprintf(pszOut, nValLen, "%.1f", pParamVal->RereadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_2:
        pParamVal->RereadValue.fValue = *(float*)blobData;
        snprintf(pszOut, nValLen, "%.2f", pParamVal->RereadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_3:
        pParamVal->RereadValue.fValue = *(float*)blobData;
        snprintf(pszOut, nValLen, "%.3f", pParamVal->RereadValue.fValue);
        break;
    case WEB_VALTYPE_FLOAT_4:
        pParamVal->RereadValue.fValue = *(float*)blobData;
        snprintf(pszOut, nValLen, "%.4f", pParamVal->RereadValue.fValue);
        break;
    case WEB_VALTYPE_IP:
        pParamVal->RereadValue.ulValue = *(unsigned long*)blobData;
        ip.s_addr = pParamVal->RereadValue.ulValue;
        snprintf(pszOut, nValLen, "%s", inet_ntoa(ip));
        break;
    case WEB_VALTYPE_DATETIME:
        pParamVal->RereadValue.ulValue = *(unsigned long*)blobData;
        snprintf(pszOut, nValLen, "%s", TimeToString((time_t)pParamVal->RereadValue.ulValue, TIME_CHN_FMT, szTime, 32));
        break;
    case WEB_VALTYPE_ENUM:    
        pParamVal->RereadValue.emValue = *(int*)blobData;
        nEnumVal = pParamVal->RereadValue.emValue;
        for (n = 0; n < pParamVal->pParamInfo->nEnumTotal; n++)
        {
            if (pParamVal->pParamInfo->enumList[n].emVal == nEnumVal)
            {
                snprintf(pszOut, nValLen, "%d", nEnumVal);
                break;
            }
        }
        if (n >= pParamVal->pParamInfo->nEnumTotal)
        {
            snprintf(pszOut, nValLen, "%d", nEnumVal);
        }
        break;

    default:
        break;
    }
    return pszOut;
}

BOOL UploadFile_2Client(char* szLocalFilePath, char* szRemoteFileName)
{
    FILE *fp = fopen(szLocalFilePath, "r");
	if (fp == NULL)
	{
		return FALSE;
	}

    fseek(fp, 0, SEEK_END);
	int nFilelen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if((nFilelen < 1) || (nFilelen > 100 * 1024 * 1024))
    {
        fclose(fp);
        TRACEX("----the file size[%d] is WWWRONG!!!---\n", nFilelen);
        return FALSE;
    }

    //TRACEX("---now begin upload file[%s -> %s],filelen->%d---\n", szLocalFilePath, szRemoteFileName, nFilelen);

    WEB_OUT("Content-Type: application/octet-stream\r\n");
    WEB_OUT("Content-Disposition: attachment; filename=\"%s\"\r\n", szRemoteFileName);
    WEB_OUT("Content-Length: %d\r\n\r\n", nFilelen);
    int cc;
    int nCount = 0;
    do
    {
        cc = fgetc(fp);
        FCGI_putchar(cc);
        //TRACEX("%c", cc);
        nCount++;
    }while((cc != EOF) && (nCount < nFilelen));    

    fclose(fp);

    TRACEX("----upload file[%s -> %s] done---\n", szLocalFilePath, szRemoteFileName);

    return TRUE;
}

BOOL UploadBinFile_2Client(char* szLocalFilePath, char* szRemoteFileName)
{
    FILE *fp = fopen(szLocalFilePath, "rb");
    if (fp == NULL)
    {
        return FALSE;
    }

    fseek(fp, 0, SEEK_END);
    long nFilelen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if((nFilelen < 1) || (nFilelen > 100 * 1024 * 1024))
    {
        fclose(fp);
        TRACEX("----the file size[%d] is WWWRONG!!!---\n", nFilelen);
        return FALSE;
    }

    WEB_OUT("Content-Type: application/octet-stream\r\n");
    WEB_OUT("Content-Disposition: attachment; filename=\"%s\"\r\n", szRemoteFileName);
    WEB_OUT("Content-Length: %d\r\n\r\n", nFilelen);

    char buffer[4096];
    int bytes_read;
    int i;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        for (i = 0; i < bytes_read; ++i) {
            FCGI_putchar(buffer[i]);
        }
    }

    fclose(fp);

    TRACEX("----upload file[%s -> %s] done---\n", szLocalFilePath, szRemoteFileName);
    return TRUE;
}

BOOL DownloadFile_FromClient(char* szLocalFilePath)
{
    // if((pbyFileData == NULL) || (nDataLen < 1))
    // {
    //     TRACEX("---can not receive NULL file!!!---\n");
    //     return FALSE;
    // }

    // FILE *fp = fopen(szLocalFilePath, "w+");
	// if (fp == NULL)
	// {
	// 	return FALSE;
	// }

    // fwrite(pbyFileData, nDataLen, 1, fp);

    // fclose(fp);
    if(!Create_2BinFile_FromBody())
    {
        return FALSE;
    }

    rename(WEB_TEMPFILE_DOWNLOADED_RAW, szLocalFilePath);

    if(access(szLocalFilePath, R_OK) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

char* Make_RegExp_4StringParam(WEB_PARAM_VALUE* pParamVal)
{
    static char s_RegExpStr[64];
    memset(s_RegExpStr, 0, 64);
    int nLenLmt = (pParamVal->pParamInfo->nValueSize < 1) ? 1 : pParamVal->pParamInfo->nValueSize;
    /*if(pParamVal->pParamInfo->nValueSize < 1)
    {
        sprintf(s_RegExpStr, "^.*$");
    }
    else */if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_PASSWORD)
    {
        sprintf(s_RegExpStr, "^[0-9\\.]{1,%d}$", nLenLmt);
    }
    else
    {
        sprintf(s_RegExpStr, "^[a-zA-z0-9'!\"#$%c&()*,./:;?@^_`\\[\\]{|}~+<=>\\ -]{0,%d}$", '%', nLenLmt);
    }

    //hanlde the special param by Id
    int nParamId = pParamVal->pParamInfo->nParamId;
    if(pParamVal->pParamInfo->emParamType == PARAM_TYPE_SETTING)
    {
        //otp code: should be number
        if((nParamId == 12) || (nParamId == 202))
        {
            sprintf(s_RegExpStr, "^[0-9\\.]{1,%d}$", nLenLmt);
        }
        else if((nParamId == 18) || (nParamId == 2))
        {
            //for OCPP url&chargerId, there should be no SPACE and other special char
            sprintf(s_RegExpStr, "^[a-zA-z0-9'!\"#$%c&()./:;?@^_`\\[\\]{}~+<=>-]{0,%d}$", '%', nLenLmt);
        }
        else// if(nParamId == 77)  //added by Jimmy 20220215 to match all string from NO-SPACE-Stated!
        {
            if(nLenLmt <= 1)
            {
                sprintf(s_RegExpStr, "^.?$");
            }
            else
            {
                sprintf(s_RegExpStr, "^(.?|\\S(.){0,%d})$", nLenLmt - 1);
            }              
        }
    }
    else //control param 
    {

    }    
    
    return s_RegExpStr;
}

char* Make_RegExp_4String_SubBlobParam(WEB_SUBBLOB_ITEM* pSubBlob)
{
    static char s_RegExpStr[64];
    memset(s_RegExpStr, 0, 64);
    int nLenLmt = (pSubBlob->nValueSize < 1) ? 1 : pSubBlob->nValueSize;
    // if(pSubBlob->nValueSize < 1)
    // {
    //     sprintf(s_RegExpStr, "^.*$");
    // }
    // // else if(pParamVal->pParamInfo->emValType == WEB_VALTYPE_PASSWORD)
    // // {
    // //     sprintf(s_RegExpStr, "[0-9\\.]{1, %d}", pParamVal->pParamInfo->nValueSize);
    // // }
    // else
    {
        //sprintf(s_RegExpStr, "^[a-zA-z0-9'!\"#$%c&()*,./:;?@^_`\\[\\]{|}~+<=>\\ -]{0,%d}$", '%', nLenLmt);
        if(nLenLmt <= 1)
        {
            sprintf(s_RegExpStr, "^.?$");
        }
        else
        {
            sprintf(s_RegExpStr, "^(.?|\\S(.){0,%d})$", nLenLmt - 1);
        }     
    }
    
    return s_RegExpStr;
}

//-----------------------following are for multipart-form-data downloading parser--------------------
#define MAX_BULK_CHARBUFFER_LEN     8192
static int Get_BulkBuff_FromFCGI(char* szBulkBuff, int nReadLen)
{
    memset(szBulkBuff, 0, nReadLen);
    int nGetLength = 0;
    int i, ch;
    for (i = 0; i < nReadLen; i++) 
    {
       if ((ch = FCGI_getchar()) < 0) 
       {
             break;
       }
       szBulkBuff[i] = ch;
       nGetLength++;
    }

    return nGetLength;
}

static BOOL ParseBondary_FromContType(char* szContengType, MULTIPART_FORMDATA_STRU* pOut_StMultiData)
{
    char* szBTitle = "boundary=";
    int nBTitleLen = strlen(szBTitle);
    char* pos = strstr(szContengType, szBTitle);
    if(pos == NULL)
    {
        return FALSE;
    }
    pos += nBTitleLen;
    memset(pOut_StMultiData->szBondary, 0, 128);
    char c = *pos;
    int n = 0;
    while((c != '\n') && (c != '\f') && (c != '\r'))
    {
        pOut_StMultiData->szBondary[n] = c;
        n++;        
        if(n >= 127)
        {
            break;
        }
        c = *(pos + n);
    }
    
    return TRUE;
}

static char s_szTempBuff[MAX_BULK_CHARBUFFER_LEN]; //for parsing purose
static int s_nTempBuffLen = 0;
static char s_szRawFileName[64] = {0};
static char* Parse_HeaderOf_ASection(char* szStart, int length, int *pnParsedLen, BOOL *pbIsFile)
{
    unsigned int datalength = length;
	int linelength;
	char *data = szStart;
    *pbIsFile = FALSE;
	// parse headers one line at a time
	// 
	while(   (linelength = strcspn(data, "\r\f\n")    )     )
	{
		// figure out what the first character of newline is
		// and save it for later use
		//
		char cCR = data[linelength];
		
		// null terminate this line
		data[linelength] = 0;
		//TRACEX("\n<br>*****Found header line: '%s'*****<br>\n", data);
		// examine this header line for facts
		//
		// we have to search for all up front (from index 0),
		// since more than one can appear on the same line....
		//
		// POSSIBLE BUG SOURCE: Case senstivity of these identifiers..
		// 
		char *filenameptr=strstr(data, " filename=");
		char *nameptr=strstr(data, " name=");
		char *contentptr=strstr(data, "Content-Type");
				
		char *tempptr;
		if( filenameptr )
		{
			// find first quote
			tempptr = strstr(filenameptr, "\"");
			if(tempptr)
			{
				filenameptr = ++tempptr;
			}
			// find end quote
			tempptr = strstr(filenameptr, "\"");
			if(tempptr)
			{
				*tempptr = 0;
			}
			TRACEX("----the parsed filename=>%s----\n", filenameptr);
            *pbIsFile = TRUE;
            snprintf(s_szRawFileName, 64, "%s", filenameptr);
		}
		if( nameptr )
		{
			// find first quote
			tempptr = strstr(nameptr, "\"");
			if(tempptr)
			{
				nameptr = ++tempptr;
			}
			// find end quote
			tempptr = strstr(nameptr, "\"");
			if(tempptr)
			{
				*tempptr = 0;
				
			}
			TRACEX("----the parsed name=>%s----\n", nameptr);
		}			
		if(contentptr)
		{
			// find first space
			tempptr = strstr(contentptr, " ");
			if(tempptr)
			{
				contentptr=++tempptr;
			}
			// find end quote
			int seglength = strcspn(contentptr, ";");
			tempptr += seglength;
			*tempptr = 0;
			TRACEX("----the parsed contentType=>%s----\n", contentptr);
		}
		// reset data start to just beyond the null character
		// we set in the first line
		//
		datalength -= (linelength +1);
		data += linelength +1;
		// if we are sitting on top of the LF section of a CRLF, we need to move past it
		if(isspace(*data) && (*data != cCR))
		{
			data++;
			datalength--;
		}

		if(datalength < 0)
		{
			datalength = 0;
		}
	} // end of header parsing loop
	// chomp off the newlines at the beginning and end
	//
	// POSSIBLE BUG SOURCE: what if newlines are at the start or end of the actual data??
	// 
	//while(datalength && (*data == '\n' || *data  == 0x0a || *data == 0x0d))
	//{
	//		std::cerr << __FILE__ << ":" <<__LINE__ << ":removed Beginning Newline\n"; 
	//		data++;
	//		datalength--;
	//}
	//while(datalength && (data[datalength-1] == '\n' || data[datalength-1]  == 0x0a || data[datalength-1] == 0x0d ))
	//{
	//	std::cerr << __FILE__ << ":" <<__LINE__ << ":removed Trailing Newline\n"; 
	//	datalength--;
	//}
               // New way to chomp - take 2 off each end and call it a day
	//
    int a;
    for(a = 0; a < 2; a++)
	{
		if(datalength)
		{
			data++;
			datalength--;
		}
	} 
    
    *pnParsedLen = datalength;
	return data;
}

static char* My_StrStr(char* szSrc, const char* szM, int nSrcLen)
{
    char *pos = NULL;
    int nMLen = strlen(szM);
    int m, n;

    for(m = 0; m < nSrcLen - nMLen; m++)
    {
        if(szSrc[m] == szM[0])
        {
            pos = szSrc + m;
            for(n = 1; n < nMLen; n++)
            {
                if(szSrc[m + n] != szM[n])
                {
                    pos = NULL;
                    break;
                }           
            }
        }
        //found, so break!
        if(pos)
        {
            break;
        }
    }
    return pos;
}

static char* Find_LastMatch(char* szSrc, const char* szM, int nSrcLen)
{
    char *pos = NULL;
    int nLeftSrcLen = nSrcLen;
    char* lastMatch = NULL;
    char* posNext = szSrc;
    int nMLen = strlen(szM);
    do
    {
        pos = My_StrStr(posNext, szM, nLeftSrcLen);
        if(pos)
        {
            lastMatch = pos;
            posNext = pos + nMLen;
            nLeftSrcLen = nSrcLen - (posNext - szSrc);
            if(nLeftSrcLen < nMLen)
            {
                break;
            }
        }

    } while (pos);

    return lastMatch;    
}

static char* Parse_SectionData(char* szStart, MULTIPART_FORMDATA_STRU* pOut_StMultiData)
{
    char * szEndBondary_EndPos = 0;
    int nRealLen = s_nTempBuffLen - (szStart - s_szTempBuff);
    //now parse the fileName so that to know it is file download(notice the SPACE!!!)
    int nDataLen = 0;
    char* szStartBondary;
    BOOL bIsForm_File = FALSE;
    char *szPosDataStart = Parse_HeaderOf_ASection(szStart, nRealLen, &nDataLen, &bIsForm_File);
    TRACEX("---the parseHeadLen=>[%d, %d], szPosDataStart[%s]---\n", nRealLen, nDataLen, szPosDataStart);
    //char *pos = strstr(szStart, " filename=");
    //if(pos != NULL)
    if(bIsForm_File)
    {
        TRACEX("---now found a file, so parse it!!!---\n");
        //now handle the file download
#define TEMP_FFFF_1 "/opt/appdata/data/tmp_download_webfile.1"
#define TEMP_FFFF_2 "/opt/appdata/data/tmp_download_webfile.2"
// #define mTLS_PEM "/opt/appdata/data/input.pem"
#define mTLS_PEM "/app/config/tmp_download_webfile.pem"
#define mTLS_KEY "/app/config/tmp_download_webfile.key"
        char *szWhichFile;
        if(!pOut_StMultiData->bHasParsed_File1)
        {
            pOut_StMultiData->bHasParsed_File1 = TRUE;
            // char* extension = strrchr(s_szRawFileName, '.'); 
            if (s_szRawFileName != NULL)
            {   
                if (strcmp(s_szRawFileName, "client.pem") == 0)
                {   
                    TRACEX("---mfr_cert.pem---\n");
                    szWhichFile = mTLS_PEM;
                    sprintf(pOut_StMultiData->szPath_4File1, mTLS_PEM);
                }
                else if (strcmp(s_szRawFileName, "client.key") == 0)
                {   
                    TRACEX("---privatekey_mfr.key---\n");
                    szWhichFile = mTLS_KEY;
                    sprintf(pOut_StMultiData->szPath_4File1, mTLS_KEY);
                }
                else
                {
                    TRACEX("---normal file---\n");
                    szWhichFile = TEMP_FFFF_1;
                    sprintf(pOut_StMultiData->szPath_4File1, TEMP_FFFF_1);
                }
                snprintf(pOut_StMultiData->szRawFileName_FromClient, 64, "%s", s_szRawFileName);
            }
            
        }
        else
        {
            pOut_StMultiData->bHasParsed_File2 = TRUE;
            szWhichFile = TEMP_FFFF_2;
            sprintf(pOut_StMultiData->szPath_4File2, TEMP_FFFF_2);
        }
        
        FILE *fp = fopen(szWhichFile, "w+");
        if (fp == NULL)
        {
            return NULL;
        }

        // if(nDataLen > 0) 
        // {
        //     //need read more in case there 
        //     memmove(s_szTempBuff, szPosData, nDataLen);
        //     szPosData = s_szTempBuff;
        //     nRealLen = nDataLen + Get_BulkBuff_FromFCGI(szPosData + nDataLen, MAX_BULK_CHARBUFFER_LEN - nDataLen);
        //     szStartBondary = strstr(szPosData, pOut_StMultiData->szBondary); 
        //     TRACEX("---the foundEndBondary=>[%s], %d--\n", szStartBondary, szStartBondary - 1 - szPosData);   
        //     if(szStartBondary)
        //     {
        //         nDataLen = szStartBondary - 2 - szPosData - 2; //need skip -- and \d\a
        //         szEndBondary_EndPos = szStartBondary + strlen(pOut_StMultiData->szBondary) + 2 + 2 + 2; //2--(head) + 2--(tail) + \d\a
        //         int nLeftLen = szEndBondary_EndPos - szStart - 2;
        //         TRACEX("----nRealLen=>[%d], leftLen=%d---\n", nRealLen, nLeftLen);
        //         if((nLeftLen < 1) || (nLeftLen >= nRealLen))
        //         {
        //             szEndBondary_EndPos = NULL;
        //         }
        //     }
        // }

        if(nDataLen < 1) //shall not happen
        {
            fclose(fp);  
            return NULL;
        }  
        memmove(s_szTempBuff, szPosDataStart, nDataLen);
        szPosDataStart = s_szTempBuff;
          //now read more if have
        int nCount = 0;
        BOOL bFoundTail = FALSE;
        int nExtraReadLmt = strlen(pOut_StMultiData->szBondary) + 10; //resver more space in case there is error reading tail
        do
        {
            nCount++;            
            if(nCount > 1024*10) //max only receive 10*8M file
            {
                break;
            }

            //reserver bondary in case it read from next-time reading
            if(szPosDataStart == NULL)
            {
               nDataLen = Get_BulkBuff_FromFCGI(s_szTempBuff, MAX_BULK_CHARBUFFER_LEN - nExtraReadLmt); 
            }
            else 
            {
                szPosDataStart = NULL;
            }

            TRACEX("----%d : readLen=>%d---\n", nCount, nDataLen);
            if(nDataLen < 1)
            {
                break;
            }   
 
            szStartBondary = My_StrStr(s_szTempBuff, pOut_StMultiData->szBondary, nDataLen);//strstr(s_szTempBuff, pOut_StMultiData->szBondary);    
            if(szStartBondary)
            {
                nDataLen = szStartBondary - 2 - s_szTempBuff - 2; //need skip -- and \d\a
                szEndBondary_EndPos = szStartBondary + strlen(pOut_StMultiData->szBondary) + 2 + 2 + 2; //2--(head) + 2--(tail) + \d\a
                int nLeftLen = szEndBondary_EndPos - s_szTempBuff;
                TRACEX("---eeeeeeeee-nDataLen=>[%d], leftLen=%d---\n", nDataLen, nLeftLen);
                if((nLeftLen < 1) || (nLeftLen >= nDataLen))
                {   
                    TRACEX("-----Length Err!-----\n");
                    szEndBondary_EndPos = NULL;
                }
                //found tail,  so quit
                bFoundTail = TRUE;
                if(nDataLen > 0) 
                {   
                    fwrite(s_szTempBuff, nDataLen, 1, fp);
                    TRACEX("------Write File!------\n");
                }    
            }
            else 
            {
                //find the last '--' to join th the next reading
                char* szPPP = Find_LastMatch(s_szTempBuff, "--", nDataLen); //strstr(s_szTempBuff, "--");       
                BOOL bWritePartial = FALSE; 
                if(szPPP != NULL)
                {
                    int nLeneee = szPPP - s_szTempBuff - 2;//reserve \d\a
                    TRACEX("----RRRRRRRRRRRccccdtrtrtrtrt[%d, %d]-----\n", nLeneee, nDataLen);
                    if((nLeneee >= nDataLen - strlen(pOut_StMultiData->szBondary)) && (nLeneee > 0))
                    {    
                        TRACEX("----WritePartial111111111111-----\n");
                        fwrite(s_szTempBuff, nLeneee, 1, fp);    
                        bWritePartial = TRUE;     
                        s_nTempBuffLen = nDataLen - nLeneee;
                        memmove(s_szTempBuff, szPPP - 2, s_nTempBuffLen);     
                        int nExtraRead = Get_BulkBuff_FromFCGI(s_szTempBuff + s_nTempBuffLen, nExtraReadLmt);
                        s_nTempBuffLen += nExtraRead;  
                        szPosDataStart = s_szTempBuff;   //so that next NOT reading               
                    }
                }

                if(!bWritePartial)
                {
                    //just in case the tail has \r\n or - and we wrongly write them into file
                    if(nDataLen > nExtraReadLmt)
                    {
                        szPPP = s_szTempBuff + nDataLen - 1; 
                        int nCRRTCount = 0; 
                        while(szPPP >= s_szTempBuff && ((isspace(*szPPP)) || (*szPPP == '-'))) 
                        {
                            TRACEX("---Find Invalid Characters =>[%02x]---\n",*szPPP);
                            nDataLen -= 1;
                            nCRRTCount += 1;
                            szPPP -= 1;
                        }
                        if(nDataLen > 0)
                        {
                            fwrite(s_szTempBuff, nDataLen, 1, fp); 
                        }
                        bWritePartial = TRUE;    
                        char* pMemSrc = (szPPP + 1 >= s_szTempBuff) ? (szPPP + 1) : s_szTempBuff;
                        memmove(s_szTempBuff, pMemSrc, nCRRTCount);      
                        int nExtraRead = Get_BulkBuff_FromFCGI(s_szTempBuff + nCRRTCount, MAX_BULK_CHARBUFFER_LEN - nCRRTCount);
                        s_nTempBuffLen = nExtraRead + nCRRTCount;  
                        szPosDataStart = s_szTempBuff;   //so that next NOT reading  
                    }
                    if(!bWritePartial)
                    {
                        fwrite(s_szTempBuff, nDataLen, 1, fp);
                    }                    
                }
                //let datalen = readlen for next parsing process
                nDataLen = s_nTempBuffLen;
            }


        } while (!bFoundTail);
        
        fclose(fp);        
    }
    else
    {
        //normal body
        char* szStartBondary = My_StrStr(szStart, pOut_StMultiData->szBondary, nRealLen);    
        if(!szStartBondary)
        {
            //can not deal with more that 8K body   
            TRACEX("----the NONE-FILE section is too long, or NO header, len=>%d---\n", nRealLen);         
            return NULL;
        }
        else
        {
            nDataLen = szStartBondary - szPosDataStart - 2 -2; //need skip '--' and '\d\a'
            szEndBondary_EndPos = szStartBondary + strlen(pOut_StMultiData->szBondary) + 2; //2 \d\a
            int nLeftLen = szEndBondary_EndPos - szStart;
            TRACEX("---formData=>eeeeeeeee-nRealLen=>[%d], leftLen=%d, szEndBondary_EndPos=[%s]---\n", 
                nRealLen, nLeftLen, szEndBondary_EndPos);
            if((nLeftLen < 1) || (nLeftLen >= nRealLen))
            {
                szEndBondary_EndPos = NULL;
            }
        }        

        if((nDataLen > 0) && (nDataLen <= MAX_EXTERNBODY_4MULTIPART)) 
        {                  
            //now parse the externalBody
            pOut_StMultiData->bHasExternalBody = TRUE;
            memcpy(pOut_StMultiData->szExternBody, szPosDataStart, nDataLen);
            TRACEX("----the finally parsed szExternBody=>[%s]---\n", pOut_StMultiData->szExternBody);   
        }
    }    

    return szEndBondary_EndPos;
}

BOOL ParseMultiData_FromFCGIBuff(char* szContengType, MULTIPART_FORMDATA_STRU* pOut_StMultiData)
{
    if(!ParseBondary_FromContType(szContengType, pOut_StMultiData))
    {
        return FALSE;
    }
    
    s_nTempBuffLen = Get_BulkBuff_FromFCGI(s_szTempBuff, MAX_BULK_CHARBUFFER_LEN);
    TRACEX("---the GetBulkLenth=>%d, buff=>[%s]---\n", s_nTempBuffLen, s_szTempBuff);
    if(s_nTempBuffLen < 64)
    {
        return FALSE;
    }
    //now find startBondary
    char* szStartBondary = strstr(s_szTempBuff, pOut_StMultiData->szBondary);    
    if(szStartBondary == NULL)
    {
        TRACEX("--CAN NOT find first bondary--\n");
        return FALSE;
    }
    szStartBondary += strlen(pOut_StMultiData->szBondary) + 2;
    //parse first section
    char *pos;
    if((pos = Parse_SectionData(szStartBondary, pOut_StMultiData)) != NULL)
    {
        //right now we only support 2 sections
        if(pos != 0)
        {
            TRACEX("---now do the second parser---\n");
            int nMoveCount = s_nTempBuffLen - (pos - s_szTempBuff);
            memmove(s_szTempBuff, pos, nMoveCount);
            s_nTempBuffLen = nMoveCount + Get_BulkBuff_FromFCGI(s_szTempBuff + nMoveCount, MAX_BULK_CHARBUFFER_LEN - nMoveCount);
            Parse_SectionData(s_szTempBuff, pOut_StMultiData);
        }        
    }   

    return TRUE;
}
