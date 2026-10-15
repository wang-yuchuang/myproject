#include "st_batt_sampler.h"

//#define TRACE	 printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf
//#define TRACEX	 printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

//gcc -o multi_usb_write multi_usb_write.c -lpthread -lm

/*********************************************************************************
*  
*  FUNCTION NAME : Init_BMSCANDataSave Entran
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void Init_BMSCANDataSave(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	//Initialize the producer thread semaphore, (assign 4, compare the next line, let the producer run first)
    //sem_init(&pSTBatt_Rough->stuCanSave.semProducer, 0, 4);   
    //sem_init(&pSTBatt_Rough->stuCanSave.semCustomer, 0, 0);  

	pSTBatt_Rough->stuCanSave.hMutex = Mutex_Create(TRUE);

    UINT nBMSIdx = 0;

    for(nBMSIdx = 0; nBMSIdx < MAX_SINGLE_STBATT_NUM; nBMSIdx++)
    {
		pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] = NULL;

        pSTBatt_Rough->stuCanSave.bIsStartedWrite[nBMSIdx] = FALSE;
		pSTBatt_Rough->stuCanSave.bNeedWriteMsg2File[nBMSIdx] = FALSE;
    	pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx] = FALSE;

        //Initial Customer Thread Signal ( Only 0 Sinal ,Put the consumer in a blocking state from start)
        sem_init(&pSTBatt_Rough->stuCanSave.hSemWrite[nBMSIdx], 0, 0);   
    }

	pSTBatt_Rough->stuCanSave.nUsedTotalFiles = 0;
 	pSTBatt_Rough->stuCanSave.bIsUSBDisk_Mounted = FALSE;

	return ;
}


char *GetMsgTypeStrByMsgTPId(TP_TRIGGER_MSG emMsgType, char *szMsgType)
{
	switch (emMsgType)
	{
		case TP_TRIG_MANUAL:
			strcpy(szMsgType, "Manual");  
			break;
		case TP_TRIG_FAULT:
			strcpy(szMsgType, "Fault");  
			break;
		case TP_TRIG_LOG:
			strcpy(szMsgType, "Log");  
			break;
		case TP_TRIG_WAVE:
			strcpy(szMsgType, "Wave");  
			break;
		case TP_TRIG_DOD:
			strcpy(szMsgType, "DOD");  
			break;
		default:
			break;
	}

	return szMsgType;
}

/*********************************************************************************
*  
*  FUNCTION NAME : PrepareSpace_4NewFile
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   : SD Exist, mv to SD,otherwise rm
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
static void PrepareSpace_4NewFile(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, BMSMSG_FILEINFO* pNewFileInfo)
{
	int n;
	char cmd[128];
	char szTime[32];
	char szFileName[64];

	TRACE("------------------------------------Now print all fileinfo--------------------------------\n");
	for(n = 0; n < pSTBatt_Rough->stuCanSave.nUsedTotalFiles; n++)
	{
		TRACE("[%d]-->%s\n", n+1, pSTBatt_Rough->stuCanSave.usedFileInfo[n].szFileName);
	}
	TRACE("----------------------------------------End Print--------------------------------\n");

	//--- SD Or U_Disk is Exist, mv file ,Otherwise, rm it
	char szPathDev[64];
	char* szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, NULL, szPathDev, 64);					
	if(szPath == NULL)
	{
		szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, NULL, szPathDev, 64);			
	}
	if(szPath != NULL)
	{
		sprintf(cmd, "mv %s %s", pSTBatt_Rough->stuCanSave.usedFileInfo[0].szFileName, szPath);
	}
	else
	{
		sprintf(cmd, "rm -f %s", pSTBatt_Rough->stuCanSave.usedFileInfo[0].szFileName);
	}

	_SYSTEM(cmd);
	

	for(n = 1; n < pSTBatt_Rough->stuCanSave.nUsedTotalFiles; n++)
	{
		if(pSTBatt_Rough->stuCanSave.pFileInfo == &pSTBatt_Rough->stuCanSave.usedFileInfo[n])
		{
			Mutex_Lock(pSTBatt_Rough->stuCanSave.hMutex, WAIT_INFINITE);
			pSTBatt_Rough->stuCanSave.pFileInfo= &pSTBatt_Rough->stuCanSave.usedFileInfo[n - 1];
			memmove(&pSTBatt_Rough->stuCanSave.usedFileInfo[n - 1], &pSTBatt_Rough->stuCanSave.usedFileInfo[n], sizeof(BMSMSG_FILEINFO));
			Mutex_Unlock(pSTBatt_Rough->stuCanSave.hMutex);
		}
		else
		{
			memmove(&pSTBatt_Rough->stuCanSave.usedFileInfo[n - 1], &pSTBatt_Rough->stuCanSave.usedFileInfo[n], sizeof(BMSMSG_FILEINFO));
		}
	}
	memcpy(&pSTBatt_Rough->stuCanSave.usedFileInfo[MAX_SUPPORTTED_VAR_FILES- 1], pNewFileInfo, sizeof(BMSMSG_FILEINFO));

	return;
}


/*********************************************************************************
*  
*  FUNCTION NAME : Start_WriteMsg2USB
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL Start_WriteMsg2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx, TP_TRIGGER_MSG emMsgType)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;

    BMSMSG_FILEINFO     fileInfo;
    memset(&fileInfo, 0 ,sizeof(BMSMSG_FILEINFO));
	fileInfo.nBMSIdx = nBMSIdx;
	fileInfo.tmFileGenerated = time(NULL);
	fileInfo.nTotalMsgNum = 0;

	char szDevID[64];
	varV.blobValue = (BYTE *)szDevID;

	pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 2, &varV, sizeof(szDevID));

	char szTime[32];
	char szMsgType[16]={"0"};


	TimeToString(fileInfo.tmFileGenerated, "%H%M%S_%y%m%d", szTime, sizeof(szTime));
	sprintf(fileInfo.szFileName, "/var/%s_%s_BMS#%d_%d_%s.trc", szDevID, GetMsgTypeStrByMsgTPId(emMsgType, szMsgType), nBMSIdx, emMsgType, szTime); //Need Use DevID Or SN

	//printf("nBMSIdx=[%d] fileInfo.szFileName=[%s] sizeof(szDevID)=[%d]\n", nBMSIdx, fileInfo.szFileName, sizeof(szDevID));

	if(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] != NULL)
	{
		TRACE("Error: nBMSIdx=[%d]fopenFile=[%s] Has Opened File!\n", nBMSIdx, fileInfo.szFileName);
		fclose(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx]);
		pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] = NULL;	

		return FALSE;
	}

	FILE *fp = fopen(fileInfo.szFileName,"a+t");
	if(fp == NULL)  
	{
		printf("Error: fopenFile=[%s] Failed!!!\n", fileInfo.szFileName);
		fp = fopen(fileInfo.szFileName,"a+t");
		if(fp == NULL)
		{
			return FALSE;
		}
	}
	
	pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] = fp;
	pSTBatt_Rough->stuCanSave.bIsStartedWrite[nBMSIdx] = TRUE;
	memset(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx], 0, SIZE_WRITE_BUFF);
	pSTBatt_Rough->stuCanSave.nTotalMsgNum[nBMSIdx] = 0;
	//pSTBatt_Rough->stuCanSave.tmFileGenerated[nBMSIdx] = time(NULL);
	
	char 	szTmp[ONE_MSG_LEN] = {0};
	float 	fTrcFileVer = 1.3;
	sprintf(szTmp, ";$FILEVERSION=%.1f\n;$STARTTIME=%.3f\n", fTrcFileVer, GetCurrentTime());
	strcat(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx], szTmp);

	//---- Process /var File Number ----//
	if(pSTBatt_Rough->stuCanSave.nUsedTotalFiles < MAX_SUPPORTTED_VAR_FILES)
	{
		memcpy(&pSTBatt_Rough->stuCanSave.usedFileInfo[pSTBatt_Rough->stuCanSave.nUsedTotalFiles], &fileInfo, sizeof(BMSMSG_FILEINFO));
	 
		pSTBatt_Rough->stuCanSave.pFileInfo = &pSTBatt_Rough->stuCanSave.usedFileInfo[pSTBatt_Rough->stuCanSave.nUsedTotalFiles];
	
		pSTBatt_Rough->stuCanSave.nUsedTotalFiles++;
	}
	else
	{
		pSTBatt_Rough->stuCanSave.nUsedTotalFiles = MAX_SUPPORTTED_VAR_FILES;
		
		PrepareSpace_4NewFile(pSTBatt_Rough, &fileInfo);

		pSTBatt_Rough->stuCanSave.pFileInfo = &pSTBatt_Rough->stuCanSave.usedFileInfo[MAX_SUPPORTTED_VAR_FILES - 1];	
	}

	//printf("-----------Create BMSMsgFile[%s] %s!!!-------------\n",\
	//	fileInfo.szFileName, (fp == NULL) ? "FAILED" : "OK");

	return TRUE;
}

typedef struct tagABMSMsg
{
	double tmMsgComing;
	ULONG ulCANId;
	BYTE byCanData[CAN_DATA_AREA_LEN];

}A_BMS_MSG;


/*********************************************************************************
*  
*  FUNCTION NAME : Pending_WriteMsg2USB
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
*$FILEVERSION And  $STARTTIME is necessary for *.trc DOC
*
;$FILEVERSION=1.3
;$STARTTIME=44675.7804745370
;
;   C:\CANData\CAN0_CHANNEL\CH0_0424CANData0.trc
;   Start time: 2022/04/24 18:43:53.000.0
;   Generated by PCAN-Explorer v0.0.0
;-------------------------------------------------------------------------------
;   Bus  Name         Connection       Protocol  Bit rate
;   2    Connection2  xxx@pcan_usb     CAN       0 kbit/s
;-------------------------------------------------------------------------------
;   Message Number
;   |         Time Offset (ms)
;   |         |       Bus
;   |         |       |    Type
;   |         |       |    |       ID (hex)
;   |         |       |    |       |    Reserved
;   |         |       |    |       |    |   Data Length Code
;   |         |       |    |       |    |   |    Data Bytes (hex) ...
;   |         |       |    |       |    |   |    |
;   |         |       |    |       |    |   |    |
;---+-- ------+------ +- --+-- ----+--- +- -+-- -+ -- -- -- -- -- -- --
     1)        46.400 1  Rx    040E8F92 00 8    9C 0B 00 00 77 00 00 00 
     2)        48.200 1  Rx    040E9291 00 8    00 00 00 00 00 00 00 00 
***********************************************************************************/
BOOL Pending_WriteMsg2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx, BYTE* pMsgData, UINT nMsgNum)
{
	A_BMS_MSG tmpMsg;
	int n = 0;
	int nStrLen = 0;
	BYTE byResv = 0x00;
	BYTE* pBuff = pMsgData;
	char szTmp[ONE_MSG_LEN] = {0};
	
	//printf("------nBMSIdx=[%d]--Pending_WriteMsg2USB---------\n", nBMSIdx);

	pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx] = FALSE;
	if(pSTBatt_Rough->stuCanSave.bIsStartedWrite[nBMSIdx])
	{
		//printf("------nBMSIdx=[%d]--Now put msgA[%d] to queue!!---------\n", nBMSIdx, nMsgNum);

		pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx] = FALSE;
		for(n = 0; n < nMsgNum; n++)
		{
			nStrLen = strlen(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx]);
 			if(nStrLen <= SIZE_WRITE_BUFF - 64) //Tmp Msg Length
			{
				tmpMsg.ulCANId = (ULONG)(pBuff[1] << 24);
				tmpMsg.ulCANId += (ULONG)(pBuff[2] << 16);
				tmpMsg.ulCANId += (ULONG)(pBuff[3] << 8);
				tmpMsg.ulCANId += pBuff[4];
				
				memcpy(tmpMsg.byCanData, pBuff + 5, CAN_DATA_AREA_LEN);

				tmpMsg.tmMsgComing = GetCurrentTime() -  pSTBatt_Rough->stuCanSave.pFileInfo->tmFileGenerated;

				pSTBatt_Rough->stuCanSave.nTotalMsgNum[nBMSIdx]++;

				sprintf(szTmp, " %d) %.2f %d Rx %08X %02X %d %02X %02X %02X %02X %02X %02X %02X %02X\n",\
					pSTBatt_Rough->stuCanSave.nTotalMsgNum[nBMSIdx],
					tmpMsg.tmMsgComing,
					nBMSIdx+1,
					tmpMsg.ulCANId,
					byResv,
					CAN_DATA_AREA_LEN,
					tmpMsg.byCanData[0], tmpMsg.byCanData[1], tmpMsg.byCanData[2], tmpMsg.byCanData[3],
					tmpMsg.byCanData[4], tmpMsg.byCanData[5], tmpMsg.byCanData[6], tmpMsg.byCanData[7]);

				strcat(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx], szTmp);


				//printf("-Writing nBMSIdx=[%d] nStrLen=[%d] szWriteBuff[nBMSIdx]=[%s] nTotalMsgNum=[%d]\n", \
				//	nBMSIdx, nStrLen, pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx],\
				//	pSTBatt_Rough->stuCanSave.nTotalMsgNum[nBMSIdx]);

                //pSTBatt_Rough->stuCanSave.pFileInfo_0->nTotalMsgNum++;
                if(nStrLen >= SIZE_WRITE_BUFF * 0.2)
                {
                    pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx] = TRUE;
                }  
            } 
			else
			{
				//printf("pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx]=[%d]\n", pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx]);
				
				pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx] = TRUE;
 				break;
			}

            pBuff += CAN_FRAME_LEN;
		}

        //printf("nStrLen=[%d] pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx]=[%d]\n", nStrLen, pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx]);
		
        if(pSTBatt_Rough->stuCanSave.bNeedPost[nBMSIdx])
		{
            Sem_Post(&pSTBatt_Rough->stuCanSave.hSemWrite[nBMSIdx], 1);
		}
	}

    return;
}

/*********************************************************************************
*  
*  FUNCTION NAME : End WriteMsg2USB
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL End_WriteMsg2USB(STBATT_SAMPLER_ROUGH *pSTBatt_Rough, UINT nBMSIdx)
{
	if(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] != NULL)
	{
		//printf("---End_WriteMsg2USB nBMSIdx=[%d]strlen(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx])=[%d]\n",\
		//	nBMSIdx, strlen(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx]));

		if(strlen(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx]) > 0)
		{
			fprintf(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx], "%s", pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx]);
		}

		fclose(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx]);

		Sleep_Ex(200);//Write Data2File Need Wait 100ms

        pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] = NULL;

    	pSTBatt_Rough->stuCanSave.bIsStartedWrite[nBMSIdx] = FALSE;
		//pSTBatt_Rough->stuCanSave.pFileInfo_0 = NULL;
    }

    return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : End WriteMsg2USB
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   :  075512345_Wave_BMS#0_1_194420_220510.trc
*  MsgType		 :	0-Suspend, 1-Manual, 2-Fault, 3-Log, 4-Wave, 5-DOD
*  CHANGE HISTORY: 
*  
***********************************************************************************/

static BOOL LoadExisted_MsgFiles_InVarDir(STBATT_SAMPLER_ROUGH *pSTBatt_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	DIR *dir;
	struct dirent *ptr;

 	int nYear, nMonth, nDay, nHour, nMin, nSec, nBMSIdx, emMsgType;
	char szPostFix[8];  
	int nRet = 0;
	char szTime[32];
 	char *szPtr = NULL;
	char szMsgType[16]={"0"};

	if ((dir=opendir("/var")) == NULL)
	{
		return FALSE;
	}

	struct tm gmTime;

	VAR_VALUE varV;
	char szDevID[64]= "\0";
	varV.blobValue = (BYTE *)szDevID;
	pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 2, &varV, sizeof(szDevID));

	while ((ptr=readdir(dir)) != NULL)
	{
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
		{
			//TRACEX("------Skip Found aBMSMsgFile:[%s/%s]\n-------","/home/work/Self_Test_File/C_Test",ptr->d_name);

			continue;
		}
		else if(ptr->d_type == 8)    ///8-file
		{
			TRACE("111------Found aBMSMsgFile:[%s/%s]\n-------","/var",ptr->d_name);
			if ((szPtr = strstr(ptr->d_name,"BMS#")) == NULL)
			{
				continue;
			}

			memset(szPostFix, 0, 8);
			nRet = sscanf(/*ptr->d_name*/szPtr+4, "%02d_%02d_%02d%02d%02d_%02d%02d%02d.%4s",
				 &nBMSIdx, &emMsgType,  &nHour, &nMin, &nSec, &nYear, &nMonth, &nDay, szPostFix);
			
			TRACE("222------Found USB_File: szPtr=[%s] [%s/%s], nRet = %d, szPostFix = %s\n-------",\
				szPtr, "/var", ptr->d_name, nRet, szPostFix);
			
			if((nRet == 9) && (strcmp(szPostFix, "trc") == 0))  
			{
				TRACE("333------Found aBMSMsgFile:[%s/%s]\n-------","/var",ptr->d_name);
				BMSMSG_FILEINFO* pFileInfo = &pSTBatt_Rough->stuCanSave.usedFileInfo[pSTBatt_Rough->stuCanSave.nUsedTotalFiles];
				gmTime.tm_year = 2000 + nYear - 1900;
				gmTime.tm_mon = nMonth - 1;
				gmTime.tm_mday = nDay;
				gmTime.tm_hour = nHour;
				gmTime.tm_min = nMin;
				gmTime.tm_sec = nSec;
				pFileInfo->tmFileGenerated = mktime(&gmTime);
				if(pFileInfo->tmFileGenerated <= 0)
				{
					continue;
				}
				pFileInfo->nBMSIdx = nBMSIdx;
				pFileInfo->nTotalMsgNum = 0; 
				TimeToString(pFileInfo->tmFileGenerated, "%H%M%S_%y%m%d", szTime, sizeof(szTime));
				sprintf(pFileInfo->szFileName, "/var/%s_%s_BMS#%d_%d_%s.trc", szDevID, GetMsgTypeStrByMsgTPId(emMsgType, szMsgType), nBMSIdx, emMsgType, szTime);
				pSTBatt_Rough->stuCanSave.nUsedTotalFiles++;
				if(pSTBatt_Rough->stuCanSave.nUsedTotalFiles >= MAX_SUPPORTTED_VAR_FILES)
				{
					break;
				}
			}
		}
		else
		{
			//TRACEX("------d_Type=[%d] :[%s/%s]\n-------", ptr->d_type, "/home/work/Self_Test_File/C_Test", ptr->d_name);
		}
	}

	closedir(dir);

 	if(pSTBatt_Rough->stuCanSave.nUsedTotalFiles >= 2)
	{
		BMSMSG_FILEINFO tmpFileInfo;
		int i,j;
		for(i = 0; i < pSTBatt_Rough->stuCanSave.nUsedTotalFiles; i++)
		{
			for(j = 0; i + j < pSTBatt_Rough->stuCanSave.nUsedTotalFiles - 1; j++)
			{
				if(pSTBatt_Rough->stuCanSave.usedFileInfo[j].tmFileGenerated > pSTBatt_Rough->stuCanSave.usedFileInfo[j + 1].tmFileGenerated)
				{
					memmove(&tmpFileInfo, &pSTBatt_Rough->stuCanSave.usedFileInfo[j], sizeof(BMSMSG_FILEINFO));
					memmove(&pSTBatt_Rough->stuCanSave.usedFileInfo[j], &pSTBatt_Rough->stuCanSave.usedFileInfo[j + 1], sizeof(BMSMSG_FILEINFO));
					memmove(&pSTBatt_Rough->stuCanSave.usedFileInfo[j + 1], &tmpFileInfo, sizeof(BMSMSG_FILEINFO));
				}
			}
		}
	}

	TRACE("------------------------------------Now print all USB-existed fileinfo--------------------------------\n");
	int n;
	for(n = 0; n < pSTBatt_Rough->stuCanSave.nUsedTotalFiles; n++)
	{
		TRACE("[%d]-->%s\n", n+1, pSTBatt_Rough->stuCanSave.usedFileInfo[n].szFileName);
	}
	TRACE("--------------------------------------------End Print--------------------------------\n");
	return TRUE;
}

/*********************************************************************************
*  
*  FUNCTION NAME : _Thread_BMSCanDataSave Entran
*  CREATOR       :  
*  CREATED DATE  : 2022-04-28 21:07
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void* _Thread_BMSCanDataSave(void* pArgs)
{
	STBATT_SAMPLER_ROUGH *pSTBatt_Rough = (STBATT_SAMPLER_ROUGH*)pArgs;
	SAMPLER* pSampler = (SAMPLER*)pSTBatt_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

 	static char szTmp[SIZE_WRITE_BUFF];
    //struct timespec ts;

	Init_BMSCANDataSave(pSTBatt_Rough);

	LoadExisted_MsgFiles_InVarDir(pSTBatt_Rough);


	while(!pSTBatt_Rough->stuCanSave.bNeedExitThread)
	{
		Sleep_Ex(200); 

		UINT nBMSIdx = 0;
		for(nBMSIdx = 0; nBMSIdx < MAX_SINGLE_STBATT_NUM; nBMSIdx++)
		{
			TRACE("nBMSIdx=[%d]---Cycle Running _Thread_BMSCanDataSave---\n",nBMSIdx);
			Update_BMSHealthData2DB(pSTBatt_Rough, nBMSIdx);

			if(!pSTBatt_Rough->stuCanSave.bIsStartedWrite[nBMSIdx])
			{
				continue;
			}

			if(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx] == NULL)
			{
				TRACE("Info:nBMSIdx=[%d] hFile==NULL\n", nBMSIdx);
 				continue;
			}

			//----- Start Wait Data Comming -----//
			pSTBatt_Rough->stuCanSave.bNeedWriteMsg2File[nBMSIdx] = FALSE;

			// Important, Wait semWrite Signal 1000ms
			if(Sem_Wait(&pSTBatt_Rough->stuCanSave.hSemWrite[nBMSIdx], 1000) == ERR_MUTEX_OK)
            {
                //printf("nBMSIdx=[%d]---sem_Wait Success!!---\n", nBMSIdx);
                if(strlen(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx]) >= (SIZE_WRITE_BUFF * 0.5))
                {
                    //printf("Recv Data From CAN Comm strlen=[%d]\n", strlen(pSTBatt_Rough->stuCanSave.szFrame));
                    memcpy(szTmp, pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx], SIZE_WRITE_BUFF);
                    memset(pSTBatt_Rough->stuCanSave.szWriteBuff[nBMSIdx], 0, SIZE_WRITE_BUFF);
                    pSTBatt_Rough->stuCanSave.bNeedWriteMsg2File[nBMSIdx] = TRUE;
                    
					//printf("nBMSIdx=[%d]---cpy ok ---\n", nBMSIdx);
                }

                if(pSTBatt_Rough->stuCanSave.bNeedWriteMsg2File[nBMSIdx])
                {
                    //printf("nBMSIdx=[%d]---fprintf  ---szTmp=[%s] GetCurrentTime()=[%.3f]\n", nBMSIdx, szTmp, GetCurrentTime());

                    fprintf(pSTBatt_Rough->stuCanSave.hFile[nBMSIdx], "%s", szTmp); 
                    
                    //printf("nBMSIdx=[%d]---fprintf ok ---GetCurrentTime()=[%.3f]\n", nBMSIdx, GetCurrentTime());
                }
            }
            else
            {
               //printf("nBMSIdx=[%d]--- Sem_wait Failed !!! ---\n", nBMSIdx);
            }

		}

	}

	return NULL;
}
