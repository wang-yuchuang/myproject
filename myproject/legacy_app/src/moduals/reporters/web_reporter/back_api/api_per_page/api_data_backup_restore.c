#include "../fcgi_defs.h"

static char* CreateFileName_4ExportChgRecs(FCGI_ROUGH_DATA* pFCGI_Rough, char* pszExport)
{
    char szTemp[32];  
    memset(szTemp, 0, 32);   
    snprintf(pszExport, 96,"%s_%s%s_%s_%s.tar.gz", 
        pFCGI_Rough->chargerDataRepo.versionInfos.szDevID, 
        pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_SWVer, 
        pFCGI_Rough->chargerDataRepo.versionInfos.szIMMU2_HWVer,
        pFCGI_Rough->chargerDataRepo.versionInfos.szIMSU_D_Vers,
        TimeToString(Conv_UTC_2LocalTime(_NOW_), TIME_CHN_FMT_EX, szTemp, 32));

    return pszExport;
}


static BOOL Handle_SysDBDownload(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    VAR_VALUE varV;
    varV.emValue = 0; //export    
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 1, 16, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        unlink(SYSDB_WEB_EXPORT_FILENAME);
        Make_Web_OutPut(104, "Execute export sysDB error!", "Execute error!", NULL);
        return FALSE;     
    }

    //now wait the 'tar' cmd to execute 
    char szSysCmd[64];
    snprintf(szSysCmd, 64, "ps | grep 'tar -czf' | grep -cv 'grep'");
    int nCmdRet = 0;// = _SYSTEM_EX(szSysCmd);    
    int nCount = 0;
    BOOL bZipOK = FALSE;
    do
    {
        nCount++;
        Sleep(1000);
        nCmdRet = _SYSTEM_EX(szSysCmd);
        //printf("----Wait[%d], the nCmdRet = %d----\n", nCount, nCmdRet);
        if(nCmdRet < 1)
        {
            bZipOK = TRUE;
            break;
        }
    } while (nCount < 90); //wait 90s at most

    if(!bZipOK)
    {
        unlink(SYSDB_WEB_EXPORT_FILENAME);
        Make_Web_OutPut(104, "Execute compress sysDB error!", "Execute error!", NULL);
        return FALSE;     
    }
    

    char szExportName[128] = "file";   
    if(!UploadFile_2Client(SYSDB_WEB_EXPORT_FILENAME, /*"nccu.data"*/CreateFileName_4ExportChgRecs(pFCGI_Rough, szExportName)))
    {
        unlink(SYSDB_WEB_EXPORT_FILENAME);
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    unlink(SYSDB_WEB_EXPORT_FILENAME);

    return FALSE;
}

static BOOL Handle_ScreenShot_Download(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    char szSysCmd[128];
#define TEMP_SCREENSHOT_FILE "screenshot.export"
	snprintf(szSysCmd, 128, "cd /opt/appdata/data; screenshot %s", TEMP_SCREENSHOT_FILE);		
	TRACEX("---Now begin run the system cmd[%s]---", szSysCmd);
	_SYSTEM(szSysCmd);	

    if(!UploadFile_2Client("/opt/appdata/data/"TEMP_SCREENSHOT_FILE, "screenshot.png"))
    {
        unlink("/opt/appdata/data/"TEMP_SCREENSHOT_FILE);
        Make_Web_OutPut(104, "Upload process is FAILED!", NULL, NULL);
        return FALSE;      
    }

    unlink("/opt/appdata/data/"TEMP_SCREENSHOT_FILE);

    return FALSE;
}

static BOOL Handle_SysDBUpload(FCGI_ROUGH_DATA* pFCGI_Rough, BOOL bRestoreAll)
{
    //added by Jimmy 20220402: only allow the level-0 to do restore
    if(pFCGI_Rough->pCurrSess->bIsSess_FromNPS && 
        (pFCGI_Rough->pCurrSess->emMBE_UserLevel != MBE_REMOTE_USERLEVEL_0))
    {
        Make_Web_OutPut(104, "Operation NOT allowed for Infy-MBE user!", NULL, NULL);
        return FALSE;
    }

    // if(pFCGI_Rough->nTempFileLen <= 0)
    // {
    //     Make_Web_OutPut(104, "The data file is NOT received yet!", NULL,  NULL);
    //     return FALSE;                  
    // }
    // //now backup already done
    // TRACEX("-----------Now backup file DONE!!!----------\n");
    // rename(WEB_TEMPFILE_DOWNLOADED_RAW, SYSDB_WEB_IMPORT_FILENAME);
#define SYSDB_RRRRRRAW_FILE          "/opt/appdata/data/sysdb.raw"  
#define SYSDB_EXTRACTED_PATH          "/opt/appdata/data/extract/"  
    //_SYSTEM("rm -rf "SYSDB_EXTRACTED_PATH);; //remove old file if have
    if(!DownloadFile_FromClient(SYSDB_RRRRRRAW_FILE))
    {
        Make_Web_OutPut(104, "Download process is FAILED!", NULL, NULL);
        return FALSE;      
    }
    
    char szSysCmd[128];
    snprintf(szSysCmd, 128, "mkdir -p %s; tar -zxf %s -C %s", 
        SYSDB_EXTRACTED_PATH, SYSDB_RRRRRRAW_FILE, SYSDB_EXTRACTED_PATH);		
	_SYSTEM(szSysCmd);	

    if(access(SYSDB_EXTRACTED_PATH"nccu.data", R_OK) == 0)		
    {
        snprintf(szSysCmd, 128, "mv -f %s %s", SYSDB_EXTRACTED_PATH"nccu.data", SYSDB_WEB_IMPORT_FILENAME);
        _SYSTEM(szSysCmd);
        unlink(SYSDB_RRRRRRAW_FILE);    
    }
    else
    {
        rename(SYSDB_RRRRRRAW_FILE, SYSDB_WEB_IMPORT_FILENAME);        
    }    

    _SYSTEM("rm -rf "SYSDB_EXTRACTED_PATH);; //remove the extracted file
    
    VAR_VALUE varV;
    varV.emValue = 1; //import    
    int nSetCtrl_ExeRtn = Handle_Special_SingleParam_SetCtrl(
            FALSE, 1, bRestoreAll ? 17 : 16, varV.abyValue, 4);
    if(nSetCtrl_ExeRtn != WEB_ERRCODE_OK)
    {
        Make_Web_OutPut(104, "Execute import sysDB error!", "Execute error!", NULL);
        unlink(SYSDB_WEB_IMPORT_FILENAME); 
        return FALSE;     
    }


    unlink(SYSDB_WEB_IMPORT_FILENAME);    

    return TRUE;
}


BOOL webpage_handler_data_bakandrest(FCGI_ROUGH_DATA* pFCGI_Rough)
{
    TRACEX("\n----DATA_BAKANDREST page_handler was called!---\n");
    if(pFCGI_Rough->emHttpReqMethod == HTTP_REQMETHOD_GET)
    {
        int nOpId = 0;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "op=", &nOpId))
        {
            Make_Web_OutPut(104, "The 'op' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        
        if(nOpId == 99)
        {
            return Handle_ScreenShot_Download(pFCGI_Rough);
        }
        else
        {
            Make_Web_OutPut(104, "The 'op' for GET is NOT correct!", NULL, NULL);
            return FALSE;  
        } 
    }
    else //post
    {
        //operation id
        int nOpId = 0;
        if(!ParseInt_FromQuery(pFCGI_Rough->szQueryRaw, "op=", &nOpId))
        {
            Make_Web_OutPut(104, "The 'op' NOT provided!", NULL, NULL);
            return FALSE;  
        }
        if(nOpId == 0) //backup
        {
            return Handle_SysDBDownload(pFCGI_Rough);
        }
        else if((nOpId == 1) || (nOpId == 2)) //restore sett/all
        {
            return Handle_SysDBUpload(pFCGI_Rough, (nOpId == 2) ? TRUE : FALSE);
        }
        else if(nOpId == 99)
        {
            return Handle_ScreenShot_Download(pFCGI_Rough);
        }

        Make_Web_OutPut(104, "The 'op' is NOT correct!", NULL, NULL);
        return FALSE;  
    }

    return TRUE;
}