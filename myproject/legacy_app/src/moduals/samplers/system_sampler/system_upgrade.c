#include "sampler_system.h"
#include "3rd_party/curl/curl.h"
#include "../../cJSON.h"


#define SDA1_FIRMWARE_ONECLICKUPGRADE_PATH "/media/sda1/upgradefile"
#define WEB_FIRMWARE_ONECLICKUPGRADE_PATH "/opt/appdata/data/upgradefile"

//BSU upgrade must be performed before BMU upgrade
static char UpdFileName_BatSide[UPGRADED_DVE_BatSide][64] = {
    "AIRIMEU3", 
    "IMEU3",
    "IMEU2",
    "BSU",
    "BMU",
};

//IMSU upgrade must be performed last
static char UpdFileName_ModSide[UPGRADED_DEV_ModSide][64] = {
    "N2101U2",
    "N2101U1",
    "N2102U2",
    "N2102U1",
    "2302U2",
    "2302U1",
    "IMSU"
};

static int UpdTypeMap[UPGRADED_DEV_ModSide] = {
    UPDT_TP_DCDC,
    UPDT_TP_PFC,
    UPDT_TP_ABU_U2,
    UPDT_TP_ABU_U1,
    UPDT_TP_MPPT_U2,
    UPDT_TP_MPPT_U1,
    UPDT_TP_IMSU
};

void Send_DebugMsg_2UI(IN SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bFinish, BOOL bError, BOOL bLog, char *pszFormat, ...)
{
	EVENT_UI_NOTICE evNote;
	ZERO_POBJS(&evNote, 1);
	evNote.emNotoceType = UI_NOTICETYPE_IMSU_UPDATE;
	evNote.byData[0] = bFinish; //LCD will judge if need to end debug
	char szDebug[128];
	va_list arglist;
	va_start(arglist, pszFormat);
	vsnprintf(szDebug, 128, pszFormat, arglist);
	va_end(arglist);
	memcpy(&evNote.byData[1], szDebug, 128);
	
	if(bLog)
	{
		LogOut(MAIN_MODULE_SAMPLER, "SYSTEM-Upgrade", 
			bError ? LOG_TYPE_ERROR : LOG_TYPE_WARING, szDebug);
	}
	static time_t tmLastSendMsg = 0;
	time_t tmNow = _NOW_;
	if(tmNow - tmLastSendMsg < 2)
	{
		//avoid send msg too quickly
		Sleep(1000);
	}
	tmLastSendMsg = _NOW_;

	SendMsg(pSys_Rough->hSampler, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN, 
		&evNote, sizeof(EVENT_UI_NOTICE));
}


void InitUpgradeData(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
    SAMPLER* pSampler =(SAMPLER*)pSys_Rough->hSampler;
    DATA_PROCESSOR* pDPR =  (DATA_PROCESSOR*)pSampler->hDPR;
    VAR_VALUE varV;

    UINT n = 0;
    UINT DEVnums[UPGRADED_DVE_BatSide] = {0};

    UINT nBatNum = 0, PackNum = 0, EU3Num = 0, AirCondEU3 = 0;

    pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 500, 502, &varV, 4);
    nBatNum = varV.ulValue;    

    for(n = 0; n < MAX_BATTCABINET_NUM; ++n)
    {
        pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 601 + n, 450, &varV, 4);
        if((IMEU_TYPE)varV.emValue == IMEU_TYPE_IMEU3)
        {
            EU3Num++;
        }
        if(n <= 1)  //Power EU3 and the air_cond are currently the only two devices.
        {
            pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 611 + n, 451, &varV, 4);
            if((IMEU_TYPE)varV.emValue > IMEU_TYPE_IMEU2)
            {
                EU3Num++; 
            }
            pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 951 + n, 448, &varV, 4);
            //printf("AIR EU3 COM STATUS = [%d]\n", varV.emValue);
            if(varV.emValue == 3)
            {
                AirCondEU3++;
            }
        }

    }

    pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_RAW_VAL, 500, 1944, &varV, 4);
    PackNum = varV.ulValue;
    //Devices that are not present are initialized to 0
    if(pSys_Rough->lcCfg.emSystemType == SYSTYPE_ESS_AIR)
    { 
        DEVnums[2] = nBatNum;           //EU2
        DEVnums[3] = nBatNum;           //BSU
        DEVnums[4] = nBatNum;          //BMU  
    }
    else
    { 
        DEVnums[0] = AirCondEU3;        //Air_Con EU3
        DEVnums[1] = EU3Num;            //EU3
        DEVnums[3] = nBatNum*PackNum;  //BSU
        DEVnums[4] = nBatNum;          //BMU
    }
    
    for(n = 0; n < UPGRADED_DVE_BatSide; ++n)
    {  
        pSys_Rough->sUpdInfo[n].bUpgradeFlag = FALSE;
        pSys_Rough->sUpdInfo[n].nUpgradedCount = 0;
        pSys_Rough->sUpdInfo[n].nDevnums = DEVnums[n];
        pSys_Rough->sUpdInfo[n].nUpdType = UPDT_TP_BMS;
        //printf("BAT  idx = [%d], nDevnums= [%d]\n", n, DEVnums[n]);
    }
    for(n = 0; n < UPGRADED_DEV_ModSide; ++n)
    {  
        pSys_Rough->sUpdInfo2[n].bUpgradeFlag = FALSE;
        pSys_Rough->sUpdInfo2[n].nUpdType = UpdTypeMap[n];
        //printf("PCS  idx = [%d], nUpdType = [%d]\n", n, UpdTypeMap[n]);
    }

    VAR_VALUE varVSett;
    A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
    ZERO_POBJS(&blobCCUSett, 1);
    varVSett.blobValue = (BYTE*)&blobCCUSett;
    pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 200, &varVSett, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
    if(blobCCUSett.ccuSettings.byPCSType == 0)
    { 
        strcpy(UpdFileName_ModSide[0], "N2001U2");
        strcpy(UpdFileName_ModSide[1], "N2001U1");
    }
    else if(blobCCUSett.ccuSettings.byPCSType == 1)
    {
        strcpy(UpdFileName_ModSide[0], "N2002U2");
        strcpy(UpdFileName_ModSide[1], "N2002U1");
    } 

    return;
}

void OneClick_Upgrade(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
    InitUpgradeData(pSys_Rough);
    const char* szFilePath = (pSys_Rough->bWebOneClivkupgrade == TRUE) ? \
        WEB_FIRMWARE_ONECLICKUPGRADE_PATH : SDA1_FIRMWARE_ONECLICKUPGRADE_PATH;

    int n = 0;
    //detect whether USB flash drive exists
	if(access(szFilePath, F_OK) != 0)
	{
		Send_DebugMsg_2UI(pSys_Rough, TRUE, TRUE, TRUE, "Fault: The upgrade folder does not exist!");
		return;
	}
    //read upgrade files from USB flash  drive 
    struct dirent* ent = NULL; 
    DIR *pDir = opendir(szFilePath);
    if(pDir == NULL)
    {
        return;
    }
    while((ent = readdir(pDir)) != NULL)
    {
        //printf("read file name = [%s]\n", ent->d_name);
        for(n = 0; n < UPGRADED_DVE_BatSide; ++n)
        {
            if(strstr(ent->d_name, UpdFileName_BatSide[n]) != NULL)
            {
                if((n == 1) && (strstr(ent->d_name, UpdFileName_BatSide[0]) != NULL))
                {
                    continue;
                }
                if(pSys_Rough->sUpdInfo[n].nDevnums > 0)
                {//If there are files but no devices, do not perform the upgrade.
                    pSys_Rough->sUpdInfo[n].bUpgradeFlag = TRUE;
                }

                snprintf(pSys_Rough->sUpdInfo[n].szUpgradeFielPath, 256, "%s/%s", szFilePath, ent->d_name);
                snprintf(pSys_Rough->sUpdInfo[n].file_name, 64, "%s", ent->d_name);

                //printf("d_name = [%s], UpdFileName_BatSide = [%s],  szUpgradeFielPath = [%s]\n", \
                ent->d_name, pSys_Rough->sUpdInfo[n].file_name, pSys_Rough->sUpdInfo[n].szUpgradeFielPath);
                continue;
            }
        }

        for(n = 0; n < UPGRADED_DEV_ModSide; ++n)
        {
            //printf("UpdFileName_ModSide = [%s]\n", UpdFileName_ModSide[n]);
            if(strstr(ent->d_name, UpdFileName_ModSide[n]) != NULL)
            {
                
                pSys_Rough->sUpdInfo2[n].bUpgradeFlag = TRUE;
                snprintf(pSys_Rough->sUpdInfo2[n].szUpgradeFielPath, 256, "%s/%s", szFilePath, ent->d_name);
                snprintf(pSys_Rough->sUpdInfo2[n].file_name, 64, "%s", ent->d_name);

                // printf("d_name = [%s], UpdFileName_BatSide = [%s],  szUpgradeFielPath = [%s]\n", \
                ent->d_name, pSys_Rough->sUpdInfo2[n].file_name, pSys_Rough->sUpdInfo2[n].szUpgradeFielPath);
                continue;
            }
        }

    }
    closedir(pDir);

	return;
}

void SendBMUUpgradeCmd(SYSTEM_SAMPLER_DATA *pSys_Rough, char* FilePath, char* FIleName, BYTE ADDR)
{
    SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;

    A_BLOB_UPDT_INFO_4CCU blbUpdtCtrl;
    ZERO_POBJS(&blbUpdtCtrl, 1);
    blbUpdtCtrl.emBlobType = BLOB_TYPE_UPGRADE_CCU;
    blbUpdtCtrl.bIs4Web = TRUE;
    blbUpdtCtrl.emUpdtMode = 1;
    blbUpdtCtrl.emUpdtType = UPDT_TP_BMS;
   
    blbUpdtCtrl.iDevAddr = ADDR;
    sprintf(blbUpdtCtrl.szBMSFilePath, FilePath);
    sprintf(blbUpdtCtrl.file_name, FIleName);

    Push_BlobValue(SETTYPE_INTERN_CTRL, (HANDLE)pSampler, 500, 500, 
				(void*)&blbUpdtCtrl, sizeof(A_BLOB_UPDT_INFO_4CCU), BLOB_TYPE_UPGRADE_CCU);
    return;
}

void SendPCSUpgradeCmd(SYSTEM_SAMPLER_DATA *pSys_Rough, char* FilePath, int UpdType, float VersionNum)
{
    SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;

    A_BLOB_UPDT_INFO_4CCU blbUpdtCtrl;
    ZERO_POBJS(&blbUpdtCtrl, 1);
    blbUpdtCtrl.emBlobType = BLOB_TYPE_UPGRADE_CCU;
    blbUpdtCtrl.bIs4Web = TRUE;
    blbUpdtCtrl.emUpdtMode = 0;
    blbUpdtCtrl.emUpdtType = UpdType;

      
    blbUpdtCtrl.fDCDC_SWVer_4Updt = VersionNum;
    blbUpdtCtrl.fIMSU_SWVer_4Updt = VersionNum;
    blbUpdtCtrl.fPFC_SWVer_4Updt =  VersionNum;

    if(blbUpdtCtrl.emUpdtType == UPDT_TP_IMSU)
    {
        sprintf(blbUpdtCtrl.szIMSUFilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_PFC)
    {
        sprintf(blbUpdtCtrl.szPFCFilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_DCDC)
    {
        sprintf(blbUpdtCtrl.szDCDCFilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_ABU_U1)
    {
        sprintf(blbUpdtCtrl.szPFCFilePath, FilePath);     
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_ABU_U2)
    {
        sprintf(blbUpdtCtrl.szDCDCFilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_EU2_1)
    {
        sprintf(blbUpdtCtrl.szIMEU2FilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_EU2_2)
    {
        sprintf(blbUpdtCtrl.szIMEU2FilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_MPPT_U1)
    {
        sprintf(blbUpdtCtrl.szPFCFilePath, FilePath);
    }
    else if(blbUpdtCtrl.emUpdtType == UPDT_TP_MPPT_U2)
    {
        sprintf(blbUpdtCtrl.szDCDCFilePath, FilePath);
    }
    else
    {
        return;
    }
   
    Push_BlobValue(SETTYPE_INTERN_CTRL, (HANDLE)pSampler, 200, 151, 
				(void*)&blbUpdtCtrl, sizeof(A_BLOB_UPDT_INFO_4CCU), BLOB_TYPE_UPGRADE_CCU);
    return;
}


BOOL refreshstatus(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
    int n = 0; 
    VAR_VALUE varV;
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
    ERRCDDE_DPR err;
    BOOL bRet = FALSE;
    for(n = 0; n < UPGRADED_DVE_BatSide; ++n)
    {
        if(pSys_Rough->sUpdInfo[n].bUpgradeFlag == TRUE)
        {
            bRet = TRUE;
            break;
        }
    }
   
   for(n = 0; n < UPGRADED_DEV_ModSide; ++n)
    {
        if( pSys_Rough->sUpdInfo2[n].bUpgradeFlag == TRUE)
        {
            bRet = TRUE;
            break;
        } 
    }
    err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, 200, 81, &varV, 4);
    if(varV.emValue != UPGRADE_IDLE)
    {
        pSys_Rough->tm_RefreshStatus = _NOW_;
        bRet = TRUE;
    }
    err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, 200, 86, &varV, 4);
    if(varV.emValue != UPGRADE_IDLE)
    {
        pSys_Rough->tm_RefreshStatus = _NOW_;
        bRet = TRUE;
    }
    //MU2模块升级传输完文件后，会隔一会状态才会改变
    err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, 200, 124, &varV, 4);
    if(varV.emValue == 1)
    {
        pSys_Rough->tm_RefreshStatus = _NOW_;
        bRet = TRUE;
    }
    if(ABS(_NOW_ - pSys_Rough->tm_RefreshStatus) < 65)
    {
        bRet = TRUE;
    }

    if(bRet == TRUE)
    {
        Send_DebugMsg_2UI(pSys_Rough, FALSE, FALSE, FALSE, "Info: Upgrading..., Please wait!");
        return TRUE;
    }
   
	varV.lValue = 256; 
    for(n = 0; n < 20; n++)
    {
        Push_VarValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 200, 82, varV);
        Push_VarValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 200, 85, varV);
    }

    
    return FALSE;
}


#define Air_IMEU3_MIN_ADDRESS           0x2B
#define IMEU3_MIN_ADDRESS               0x1E
#define IMEU2_MIN_ADDRESS               0x28
#define BSU_MIN_ADDRESS                 0x00
#define BMU_MIN_ADDRESS                 0x32
BYTE GetIddr(UINT n, UINT idx)
{
    if(n > UPGRADED_DVE_BatSide)
    {
        return 0;
    }
    if(n == 0)
    {
        return Air_IMEU3_MIN_ADDRESS + idx;
    }
    else if(n == 1)
    {
        return IMEU3_MIN_ADDRESS + idx;
    }
    else if(n == 2)
    {
        return IMEU2_MIN_ADDRESS + idx;
    }
    else if(n == 3)
    {
        return BSU_MIN_ADDRESS + idx;
    }

    return BMU_MIN_ADDRESS + idx;
}

/********************************************************************************* 
*  FUNCTION NAME : BatSideupgrade
*  CREATOR AND CREATED DATE : WLD  2025-05-14
*  DESCRIPTION   :  Upgrade BSU BMU EU3 AIRCON-EU3 STS-EU3 EU2 
***********************************************************************************/
void BatSideupgrade(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
    VAR_VALUE varV;
    int n = 0, i = 0;
    char buffer[64]; 
	ERRCDDE_DPR err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, 200, 81, &varV, 4);
    if(err != ERR_DPR_OK)
    {
        return;
    }
    
    if(varV.emValue != UPGRADE_IDLE)
    {
        pSys_Rough->tm_Updsendtimefirst = _NOW_;
        //Not in idle state   return
        return;
    }
    else 
    {
        pSys_Rough->tm_Updsendtimelast = _NOW_;
    }
    
    if((ABS(pSys_Rough->tm_Updsendtimefirst - pSys_Rough->tm_Updsendtimelast)) < 20) //upgrade interval between the two devices is a few seconds to prevent CAN bus contention
    { 
        return;
    }

    for(n = 0; n < UPGRADED_DVE_BatSide; ++n)
    {//Check if the device needs to be upgraded
    
       // printf("idx = [%d], bUpgradeFlag = [%d], nDevnums = [%d]\n", n, pSys_Rough->sUpdInfo[n].bUpgradeFlag, pSys_Rough->sUpdInfo[n].nDevnums);

        if(pSys_Rough->sUpdInfo[n].bUpgradeFlag == TRUE)
        { 
            //Send the upgrade file path, upgrade type, and device address
            BYTE ADDR = GetIddr(n, pSys_Rough->sUpdInfo[n].nUpgradedCount);

            if((_NOW_ - pSys_Rough->tm_Updsendtime) < 30)//Send at least every 30 seconds, otherwise the CAN will be turned of
            {
                return;
            }
            pSys_Rough->tm_Updsendtime = _NOW_;

            SendBMUUpgradeCmd(pSys_Rough, pSys_Rough->sUpdInfo[n].szUpgradeFielPath, pSys_Rough->sUpdInfo[n].file_name, ADDR);
            
            pSys_Rough->sUpdInfo[n].nUpgradedCount += 1;

            snprintf(buffer, 64, "Dev:%s Total:%d Upgrading %d", UpdFileName_BatSide[n], pSys_Rough->sUpdInfo[n].nDevnums, pSys_Rough->sUpdInfo[n].nUpgradedCount);
            Push_TextValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 200 , 84, buffer, 64);

            if(pSys_Rough->sUpdInfo[n].nUpgradedCount == pSys_Rough->sUpdInfo[n].nDevnums)
            {//Upgrade completed. Disable upgrade enable
                pSys_Rough->sUpdInfo[n].nUpgradedCount = 0;
                pSys_Rough->sUpdInfo[n].bUpgradeFlag = FALSE;
            }
            //printf("idx = [%d], nUpgradedCount = [%d], ADDR = [%02X]\n", n, pSys_Rough->sUpdInfo[n].nUpgradedCount, ADDR);
        }
    }   

    return;
}

float GetVersionNumber(const char* FileName)
{ 
    if(FileName == NULL)
    {
        return 0;
    }
    float VerNum = 0;
    const char* start = strstr(FileName, "V");
    if(start == NULL)
    {
        return 0;
    }
    start++;
    while((*start >= '0') && (*start <= '9'))
    {
        VerNum = VerNum * 10 + (*start - '0');
        start++;
    }
    return VerNum;
}

/********************************************************************************* 
*  FUNCTION NAME : ModSideupgrade
*  CREATOR AND CREATED DATE : WLD  2025-05-14
*  DESCRIPTION   :  Upgrade PCS-U1 PCS-U2 ABU-U1 ABU-U2 MPPT-U1 MPPT-U2 IMSU-D
***********************************************************************************/
void ModSideupgrade(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
    VAR_VALUE varV;
    VAR_VALUE varV_Statue;
    char buffer[64]; 
    int n = 0, i = 0;
	ERRCDDE_DPR err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, 200, 86, &varV, 4);
    if(err != ERR_DPR_OK)
    {
        return;
    }

    err = pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, 200, 124, &varV_Statue, 4);
    if(err != ERR_DPR_OK)
    {
        return;
    }

    if((varV.emValue != UPGRADE_IDLE) || ((UINT)varV_Statue.emValue == 1))
    {
        pSys_Rough->tm_PCSsendtimefirst = _NOW_;
        return;
    }
    else 
    {
        pSys_Rough->tm_PCSsendtimelast = _NOW_;
    }

    //after transferring the file, status become idel, but it is actually still upgrading
    if((ABS(pSys_Rough->tm_PCSsendtimefirst - pSys_Rough->tm_PCSsendtimelast)) < 45)
    {
        return;
    }
 
    for(n = 0; n < UPGRADED_DEV_ModSide; ++n)
    {
        //printf("idx = [%d], bUpgradeFlag = [%d], FileNamePCS = [%s]\n", n, pSys_Rough->sUpdInfo2[n].bUpgradeFlag, UpdFileName_ModSide[n]);
        if(pSys_Rough->sUpdInfo2[n].bUpgradeFlag == TRUE)
        {
            if((_NOW_ - pSys_Rough->tm_PCSsendtime) < 30)
            {
                return;
            }
                     
            pSys_Rough->tm_PCSsendtime = _NOW_;
            float Vernum = GetVersionNumber(pSys_Rough->sUpdInfo2[n].file_name);
            SendPCSUpgradeCmd(pSys_Rough, pSys_Rough->sUpdInfo2[n].szUpgradeFielPath, pSys_Rough->sUpdInfo2[n].nUpdType, Vernum);

            pSys_Rough->sUpdInfo2[n].bUpgradeFlag = FALSE;
            // printf("idx = [%d], szUpgradeFielPath = [%s], nUpdType = [%d], VerNum = [%f]\n",\
             n, pSys_Rough->sUpdInfo2[n].szUpgradeFielPath, pSys_Rough->sUpdInfo2[n].nUpdType, Vernum);

            snprintf(buffer, 64, "%s %s",UpdFileName_ModSide[n], pSys_Rough->sUpdInfo2[n].file_name);
            Push_TextValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 200 , 87, buffer, 64);               

        }
    }

    return;
}

void Send_DebugMsg_toUI(IN SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bShowButton, BOOL bError, BOOL bLog, char *pszFormat, ...)
{
	EVENT_UI_NOTICE evNote;
	ZERO_POBJS(&evNote, 1);
	evNote.emNotoceType = UI_NOTICETYPE_ONECLICK_SETTINGS;
	evNote.byData[0] = bShowButton; //LCD will judge if need to end debug
	char szDebug[128];
	va_list arglist;
	va_start(arglist, pszFormat);
	vsnprintf(szDebug, 128, pszFormat, arglist);
	va_end(arglist);
	memcpy(&evNote.byData[1], szDebug, 128);
	
	if(bLog)
	{
		LogOut(MAIN_MODULE_SAMPLER, "SYSTEM-Settings", 
			bError ? LOG_TYPE_ERROR : LOG_TYPE_WARING, szDebug);
	}
	static time_t tmLastSendMsg = 0;
	time_t tmNow = _NOW_;
	if(tmNow - tmLastSendMsg < 2)
	{
		//avoid send msg too quickly
		Sleep(1000);
	}
	tmLastSendMsg = _NOW_;

	SendMsg(pSys_Rough->hSampler, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN, 
		&evNote, sizeof(EVENT_UI_NOTICE));
}


USHORT Handle_OneClickSettins(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	char szCmd[128];
	char szPathDev[64];
	char* szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, NULL, szPathDev, 64);					
	if(szPath == NULL)
	{
		szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, NULL, szPathDev, 64);			
	}
	if(szPath == NULL)
	{
		// 不能通过直接覆盖数据库的方式进行参数设置，下发指令的配置会无法下发
		// printf("PATH:%s\n", szPath);
		// snprintf(szCmd, 128, "rm /data/nccu.data;");
		// _SYSTEM(szCmd);
		// snprintf(szCmd, 128, "cp -rf %s/nccu.data /data/;sync", szPath);
		// _SYSTEM(szCmd);

		// printf("CANNOT find para.json~~~~~~~~~~~~~~~\n");
		return TRUE;// fail need return TRUE!!!
	}

    DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;

	char path[128];
	snprintf(path, sizeof(path), "%s/paramConfig.json", szPath);
	// printf("path == %s\n", path);

	FILE* fp = fopen(path, "rb");
    if (!fp)
	{
        // printf("Error opening config file");
        return TRUE;
    }

    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    char* jsData = (char*)malloc(len + 1);
    size_t read_len = fread(jsData, 1, len, fp);
    fclose(fp);
    
    if (read_len != len)
	{
        free(jsData);
        // printf("File read error : read==%d, len==%d\n", read_len, len);
		return TRUE;
    }
    jsData[len] = '\0';

    cJSON* pRoot = cJSON_Parse(jsData);
    free(jsData);
    
    if (!pRoot)
	{
        // printf("JSON Error\n");
		return TRUE;
    }

    cJSON* configs = cJSON_GetObjectItem(pRoot, "configs");
	if (!configs || (configs->type != cJSON_Array))
	{
        cJSON_Delete(pRoot);
        printf("Invalid config format\n");
        return TRUE;
    }

	VAR_VALUE varVAlm;
	int nFlag = FALSE;
	VAR_VALUE varVSet;
	A_BLOB_CHARGERSPECIFIC_SETTINGS blobCCUSett;
	ZERO_POBJS(&blobCCUSett, 1);
	varVSet.blobValue = (BYTE*)&blobCCUSett;
	pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 200, 200, &varVSet, sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS));
	
	// printf("1 blobCCUSett.isBattGrp1_Installed = %d\n", blobCCUSett.ccuSettings.sysAtributesMap2.Set.isBattGrp1_Installed);

    // printf("to JSON\n");
	int i=0, nFailTimes=0;
	VAR_VALUE varV;
	memset(&varV, 0, sizeof(VAR_VALUE));
    cJSON* item;
    cJSON_ArrayForEach(item, configs)
	{
        cJSON* dev_id	 = cJSON_GetObjectItem(item, "dev_id");
        cJSON* param_id  = cJSON_GetObjectItem(item, "param_id");
        cJSON* data_type = cJSON_GetObjectItem(item, "data_type");
        cJSON* value 	 = cJSON_GetObjectItem(item, "value");

        if (!dev_id || !param_id || !data_type || !value)
		{
            // printf("Missing required field\n");
            continue;
        }

		if(dev_id->valueint == 200) // CCU专有设置
		{
			printf("200 : %d | ", param_id->valueint);
			switch(param_id->valueint)
			{
				case 0:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.EPOVoltLevel = value->valueint & 1;break;
				case 1:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.DoorOpenEnable = value->valueint & 1;break;
				case 2:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.SPDEnable = value->valueint & 1;break;
				case 3:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.ACGridInputMCB = value->valueint & 1;break;
				case 4:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.ACLoadOutputMCB = value->valueint & 1;break;
				case 5:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.AC_InputContactor = value->valueint & 1;break;
				case 6:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.N_LineContactorDI = value->valueint & 1;break;
				case 7:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.Leakage_Curr_Cfg = value->valueint & 1;break;
				case 8:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.Smoke_Sensor = value->valueint & 1;break;
				case 9:	blobCCUSett.ccuSettings.sysAtributesMap1.Set.Flooding_DI = value->valueint & 1;break;
				case 10:blobCCUSett.ccuSettings.sysAtributesMap1.Set.Topple_DI = value->valueint & 1;break;
				// case 11:blobCCUSett.ccuSettings.sysAtributesMap1.Set.LowVoltMCBBreaker = value->valueint & 1;break;
				case 12:blobCCUSett.ccuSettings.sysAtributesMap2.Set.isBattGrp1_Installed = value->valueint & 1;break;
				case 13:blobCCUSett.ccuSettings.sysAtributesMap2.Set.isBattGrp2_Installed = value->valueint & 1;break;
				case 14:blobCCUSett.ccuSettings.sysAtributesMap2.Set.isBattGrp3_Installed = value->valueint & 1;break;
				case 15:blobCCUSett.ccuSettings.sysAtributesMap2.Set.isBattGrp4_Installed = value->valueint & 1;break;
				case 16:blobCCUSett.ccuSettings.sysAtributesMap2.Set.STS_Grp1_Installed = value->valueint & 1;break;
				// case 17:blobCCUSett.ccuSettings.sysAtributesMap2.Set.STS_Grp2_Installed = value->valueint & 1;break;
				// case 18:blobCCUSett.ccuSettings.sysAtributesMap2.Set.STS_Grp3_Installed = value->valueint & 1;break;
				// case 19:blobCCUSett.ccuSettings.sysAtributesMap2.Set.STS_Grp4_Installed = value->valueint & 1;break;
				case 20:blobCCUSett.ccuSettings.sysAtributesMap2.Set.IMEU2_Grp1_Installed = value->valueint & 1;break;
				case 21:blobCCUSett.ccuSettings.sysAtributesMap2.Set.IMEU2_Grp2_Installed = value->valueint & 1;break;
				case 22:blobCCUSett.ccuSettings.sysAtributesMap2.Set.BmsGrp1_CANInstalled = value->valueint & 1;break;
				case 23:blobCCUSett.ccuSettings.sysAtributesMap2.Set.BmsGrp2_CANInstalled = value->valueint & 1;break;
				case 24:blobCCUSett.ccuSettings.sysAtributesMap2.Set.BmsGrp3_CANInstalled = value->valueint & 1;break;
				case 25:blobCCUSett.ccuSettings.sysAtributesMap2.Set.BmsGrp4_CANInstalled = value->valueint & 1;break;
				case 26:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp1_ForBidRun = value->valueint & 1;break;
				case 27:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp2_ForBidRun = value->valueint & 1;break;
				case 28:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp3_ForBidRun = value->valueint & 1;break;
				case 29:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp4_ForBidRun = value->valueint & 1;break;
				case 30:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp1_InputContactor = value->valueint & 1;break;
				case 31:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp2_InputContactor = value->valueint & 1;break;
				case 32:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp3_InputContactor = value->valueint & 1;break;
				case 33:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp4_InputContactor = value->valueint & 1;break;
				case 34:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp1_IsolationSwitch = value->valueint & 1;break;
				case 35:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp2_IsolationSwitch = value->valueint & 1;break;
				case 36:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp3_IsolationSwitch = value->valueint & 1;break;
				case 37:blobCCUSett.ccuSettings.sysAtributesMap3.Set.BattGrp4_IsolationSwitch = value->valueint & 1;break;
				// case 38:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp1_Contactor = value->valueint & 1;break;
				// case 39:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp2_Contactor = value->valueint & 1;break;
				// case 40:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp3_Contactor = value->valueint & 1;break;
				// case 41:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp4_Contactor = value->valueint & 1;break;
				// case 42:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp1_SPD_Detection = value->valueint & 1;break;
				// case 43:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp2_SPD_Detection = value->valueint & 1;break;
				// case 44:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp3_SPD_Detection = value->valueint & 1;break;
				// case 45:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp4_SPD_Detection = value->valueint & 1;break;
				// case 46:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp1_IsolationSwitch = value->valueint & 1;break;
				// case 47:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp2_IsolationSwitch = value->valueint & 1;break;
				// case 48:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp3_IsolationSwitch = value->valueint & 1;break;
				// case 49:blobCCUSett.ccuSettings.sysAtributesMap4.Set.PVGrp4_IsolationSwitch = value->valueint & 1;break;
				case 50:blobCCUSett.ccuSettings.sysAtributesMap6.Set.isIMIU1_Installed = value->valueint & 1;break;
				// case 51:blobCCUSett.ccuSettings.sysAtributesMap6.Set.isIMIU2_Installed = value->valueint & 1;break;
				// case 52:blobCCUSett.ccuSettings.sysAtributesMap6.Set.isIMIU3_Installed = value->valueint & 1;break;
				// case 53:blobCCUSett.ccuSettings.sysAtributesMap6.Set.isIMIU4_Installed = value->valueint & 1;break;
				case 54:blobCCUSett.ccuSettings.byNumGrp_1 = value->valueint;break;
				case 55:blobCCUSett.ccuSettings.byNumGrp_2 = value->valueint;break;
				case 56:blobCCUSett.ccuSettings.byNumGrp_3 = value->valueint;break;
				case 57:blobCCUSett.ccuSettings.byNumGrp_4 = value->valueint;break;
				// case 58:blobCCUSett.ccuSettings.byNumGrp_5 = value->valueint;break;
				// case 59:blobCCUSett.ccuSettings.byNumGrp_6 = value->valueint;break;
				// case 60:blobCCUSett.ccuSettings.byNumGrp_7 = value->valueint;break;
				// case 61:blobCCUSett.ccuSettings.byNumGrp_8 = value->valueint;break;
				// case 62:blobCCUSett.ccuSettings.byFan1_WorkMode_M = value->valueint;break;
				// case 63:blobCCUSett.ccuSettings.byFan1_FanWorkMode_N = value->valueint;break;
				// case 64:blobCCUSett.ccuSettings.nFan1_OverTemp4Cool = value->valueint;break;
				// case 65:blobCCUSett.ccuSettings.nFan1_UltraTemp4Cool = value->valueint;break;
				case 66:blobCCUSett.ccuSettings.byPCSType = value->valueint;break;
				case 67:blobCCUSett.ccuSettings.byDCorAC_AdjMode = value->valueint;break;

				default: break;

			}
			continue;
		}

		pDPR->Get(pSys_Rough->hSampler, GETTYPE_ALMSTATE, 200, 103, &varVAlm, 4);
		if(varVAlm.emValue == 1)
		{
			nFailTimes++;
		}

		if(!nFlag)
		{
			// printf("\n3 blobCCUSett.isBattGrp1_Installed = %d\n", blobCCUSett.ccuSettings.sysAtributesMap2.Set.isBattGrp1_Installed);
			Push_BlobValue(SETTYPE_INTERN_SETT, pSys_Rough->hSampler, 200, 200, (void*)&blobCCUSett, 
				sizeof(A_BLOB_CHARGERSPECIFIC_SETTINGS), BLOB_TYPE_CHARGER_SPECIFIC_SETTING);
			
			Sleep_Ex(2000);//1000ms
			nFlag=TRUE;
		}

        if (strcmp(data_type->valuestring, "int") == 0)
		{
            varV.ulValue = value->valueint;
        }
        else if (strcmp(data_type->valuestring, "float") == 0)
		{
            varV.fValue = (float)value->valuedouble;
        }
        else
		{
            // printf("Unsupported type\n");
			nFailTimes++;
            continue;
        }

		// ERRCDDE_DPR Ret;
        // Ret = 
        Push_VarValue(SETTYPE_INTERN_SETT, pSys_Rough->hSampler,dev_id->valueint, param_id->valueint, varV);
		if(varVAlm.emValue == 1 && 701<= dev_id->valueint && dev_id->valueint <= 705)
		{
			nFailTimes++;
			printf("nFailTimes == %d, devid == %d\n", nFailTimes, dev_id->valueint);
		}
		// if(Ret != ERR_DPR_OK)
		// {
		// 	nFailTimes++;
		// }

		switch(i%3)
		{
			case 0 : Send_DebugMsg_toUI(pSys_Rough, FALSE, FALSE, FALSE, "Setting params now, please wait.  ");i++;break;
			case 1 : Send_DebugMsg_toUI(pSys_Rough, FALSE, FALSE, FALSE, "Setting params now, please wait.. ");i++;break;
			case 2 : Send_DebugMsg_toUI(pSys_Rough, FALSE, FALSE, FALSE, "Setting params now, please wait...");i++;break;
		}
    }
	// for(;;)
	// {
		if(nFailTimes>0)
		{
            // printf("to 111\n");
			Send_DebugMsg_toUI(pSys_Rough, TRUE, FALSE, FALSE, "参数设置失败个数 = %d, 请手动设置", nFailTimes);//"Setting params failTimes == %d", nFailTimes);
		}
		else
		{
            // printf("to 222\n");
			Send_DebugMsg_toUI(pSys_Rough, FALSE, FALSE, FALSE, "设置成功！");// "Setting params Sucessfull");
			_SYSTEM_EX("reboot");
		}
	// }

    cJSON_Delete(pRoot);
            printf("to 333\n");

	return nFailTimes;
}
