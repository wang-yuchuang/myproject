#include "std_sys.h"
#include <dlfcn.h>		// for dl*()
#include "base_defines/basefunc.h"
#include "base_defines/iNew.h"
#include "base_defines/iMutex.h"
#include "base_defines/iThread.h"
#include "base_defines/iLog.h"
#include "3rd_party/zlib/zlib.h"

#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>

UINT All_TypesMetersID[METER_TYPES_MAX][METER_TYPE_NUM] = {
    {407, 430, 431, 432},  	// 柜内表
    {408, 440, 441, 442},   // 网侧表
    {406, 0, 0, 0},        	// 光伏表
    {409, 410, 411, 0}        	// 輔助表
};

static HANDLE  g_hMutexSystemHandler = NULL;  //For SYSTEM cmd handler
#define  __MAX_CMD_CHARS	512 
#define __CMD_MEM_SIZE		1024
#define __SYSHAND_PID_FILE	"/opt/appdata/data/syshand.pid"

#define __SYSCMD_LOCKER_FILE "/opt/appdata/data/.syscmd_locker"
static int g_fd_lockerFile = -1;
static int s_nCurrCloudIdx = 1;
#define MAX_CLOUD_BACKUP_NUM 1500
//#define ANSWER_MWM_SIZE 8
struct _sCmdItem
{
	int iCmdType;
	char szCmd[__MAX_CMD_CHARS];
	int iCmdRet;
};
typedef struct _sCmdItem CMD;

#define _SEM_CMDWAIT_PATH "/_semSysHandWait"
#define _SEM_ANSWER_PATH "/_semSysHandAnsw"
#define _KEY_T_SYSHAND  (ftok("/app",15))
#define IPCKEY 8765
//#define _KEY_T_APP  (ftok("/app/bin/app",0))

static BOOL InitSysHandler(void);
static sem_t* psemWaitCmd = NULL;
static sem_t* psemAnswer = NULL;
static int shmid_cmd = -1;
static time_t s_tmLastSendHeartBeat = 0;
/*
 static sem_t* psemWaitQtCtrlLEdCmd = NULL;
 static sem_t* psemAnswerQtCtrlLEdCmd = NULL;
 static int s_fdLEDDev = -1;
 #define _SEM_DIDO_PATH "/_semDIDOWait"
 
 static void InitLEDDevCtrl(void)
 {
 	if(s_fdLEDDev < 0)
 	{
 		s_fdLEDDev = open(DIDO_NAME, O_RDWR);
 		if(s_fdLEDDev < 0)
 		{
 			TRACE("Open dido Failed!!\n");
 		}
 		else
 		{
  			TRACE("\n DXI_SysHandle: Open dido Success\n");
 		}	
 	}
 	return ;
 }
 
 void PostSignal_4Qt(void)
 {
  	TRACE("Enter PostSignal_4Qt \n");
 	if((psemWaitQtCtrlLEdCmd == NULL))
 	{
 		psemWaitQtCtrlLEdCmd = sem_open(_SEM_DIDO_PATH,O_CREAT, 0666, 0);
  		if(psemWaitQtCtrlLEdCmd == NULL)
 		{
 			TRACE("\n------psemWaitQtCtrlLEdCmd initialize error!!!\n");
 			return;
 		}
 	}
 
 	sem_post(psemWaitQtCtrlLEdCmd);
 
 	//printf("\n [dxi-syshand sem_post]\n");
 	return;
 }
 
 void Do_Ctrl(int ulWhichDO, BOOL bOn)
 {
 	if(s_fdLEDDev < 0 )
 	{
 		return;
 	}
 	ioctl(s_fdLEDDev, ulWhichDO, bOn ? 1 : 0);
 	//TRACE("\n [dxi-syshand Do_Ctrl]:ulWhichDO = %d bOn=%d\n", ulWhichDO, bOn);
 	return;
 }
 
 BOOL Get_DI(int nWhichDI)
 {
 	if(s_fdLEDDev < 0)
 	{
 		return;
 	}
 	//printf("Get DI Info nWhichDI=[%d]\n", nWhichDI);
 	USHORT	usDIData = 0;
 	BOOL bOn = FALSE;
 
 	if((nWhichDI < IMMU2_DI1) || (nWhichDI > IMMU2_DI4))
 	{
 		return FALSE;
 	}
 
 	int iRtn = read(s_fdLEDDev, &usDIData, 2);
 	if(iRtn > 0)
 	{
 		bOn = (((usDIData >> nWhichDI) & 0x01) == 0x01) ? TRUE : FALSE;
 	}
 	return bOn;
 }
 
  
 void Wait_QtCmd_andExecute(int nMaxWait_mSec)
 {
 	if(psemAnswerQtCtrlLEdCmd == NULL) 
 	{
 		psemAnswerQtCtrlLEdCmd = sem_open(_SEM_DIDO_PATH,O_CREAT, 0666, 0);
 		if(psemAnswerQtCtrlLEdCmd == NULL)
 		{
 			return;
 		}
 	}
 	
 	if(s_fdLEDDev < 0)
 	{
 		InitLEDDevCtrl();
 		if(s_fdLEDDev < 0)
 		{
 			return ;
 		}
 	}
  
 	while(nMaxWait_mSec--)
 	{
 		//printf("[DXI_SYSHandle sem_trywait ] nMaxWait_mSec=%d\n", nMaxWait_mSec);
 		if(sem_trywait(psemAnswerQtCtrlLEdCmd) == 0)
 		{
 			ioctl(s_fdLEDDev, IOCTL_BEEP_100, 1);
  			TRACE("\n [dxi-syshand sem_trywait]\n");
 			break;
 		}
 	}
 	return;	
 }
 */
 
static BOOL InitSysHandler()
{
	psemWaitCmd =  sem_open(_SEM_CMDWAIT_PATH, O_CREAT,0666, 0);
	if(psemWaitCmd == SEM_FAILED || psemWaitCmd == NULL)
	{
		psemWaitCmd = NULL;
		printf( "[dxi-syshand]:sem_open waitCmd Failed!\n" );
		return FALSE;
	}
	psemAnswer =  sem_open(_SEM_ANSWER_PATH, O_CREAT,0666, 0);
	if(psemAnswer == SEM_FAILED || psemAnswer == NULL)
	{
		psemAnswer = NULL;
		printf( "[dxi-syshand]:sem_open semAnswer Failed!\n" );
		return FALSE;
	}
	//printf("\n[dxi-syshand]:init sem OK\n");

	key_t keySys = _KEY_T_SYSHAND;
	//key_t keySys = IPCKEY;
	//printf("\n[dxi-syshand]:keyt is [%d] for pid(%d)\n",keySys, getpid());
	shmid_cmd = shmget(keySys,0,0666);
	if(shmid_cmd == -1)
	{
		printf("\n\n[dxi-syshand]:shmget ini ERRRRR!!!");
		return FALSE;
	}

	g_hMutexSystemHandler = Mutex_Create(TRUE);

	if (g_hMutexSystemHandler == NULL)
	{
		//	  Create Mutex for System cmd failed");

		//AppLogOut(DXI_MODULE_TASK, APP_LOG_ERROR, 
		//	"[%s]--InitDataExchangeModule: ERROR: "
		//	"Create Mutex for System cmd failed!\n\r",
		//	__FILE__);

		return FALSE;
	}

	//initially open the locker file
	g_fd_lockerFile = open(__SYSCMD_LOCKER_FILE, O_RDWR|O_CREAT, 0644);
    if (g_fd_lockerFile < 0) 
	{
        printf("open the lockerfile error!!!");
        return FALSE;
    }

	//printf("\n[dxi-syshand]:open locker file[%d] for pid(%d)\n",g_fd_lockerFile, getpid());
	//system("reboot");
	s_tmLastSendHeartBeat = _NOW_;
	return TRUE;
}

char *RemoveStringWhiteSpace(IN char *pszBuf)
{
	char    *p = pszBuf;
	ASSERT(p != NULL);

	char    *pFirstNonSpace = pszBuf, *pLastWhiteSpace = NULL;
	BOOL    bFound = FALSE;  /* first non-space character found flag */

	while (*p)
	{
#define IS_WHITE_SPACE(c)	( ((c) == SPACE) || ((c) == TAB) )
		if (IS_WHITE_SPACE(*p))
		{
			if(pLastWhiteSpace == NULL)
			{
				pLastWhiteSpace = p;
			}
		}
		else 
		{
			if (!bFound)
			{
				pFirstNonSpace = p;
				bFound = TRUE;
			}

			pLastWhiteSpace = NULL;
		}

		p++;
	}

	if (pLastWhiteSpace != NULL)
	{
		*pLastWhiteSpace = 0;
	}

	return pFirstNonSpace;
}

void Do_Proc_HeartBeat(void)
{
	if((psemWaitCmd == NULL) || (psemAnswer == NULL) || (shmid_cmd == -1) ||
		(g_fd_lockerFile < 0))
	{
		BOOL bIni = InitSysHandler();
		if(!bIni)
		{
			printf("\n------system handler initialize error!!!\n");
			return;
		}
	}

	time_t tmNow = _NOW_;
	int iTimes = 0,iSemRet = 0;
	CMD* pCmd;
	//do not heartbeat within 10 sec
	if(ABS(tmNow - s_tmLastSendHeartBeat) < 10)
	{
		return;
	}

#define _LOCK_HEARTBEAT_WAIT_TIMEOUT		100
	if (Mutex_Lock(g_hMutexSystemHandler,_LOCK_HEARTBEAT_WAIT_TIMEOUT) != ERR_MUTEX_OK)
	{
		//if already busy, we think it is in cmd process
		s_tmLastSendHeartBeat = tmNow;
		return;
	}

	//Jimmy added 20200605: lock it and unlock after execution
	BOOL bLocked = FALSE;
	for(iTimes = 0; iTimes < 300; iTimes++) //max wait 2sec
	{
		if(flock(g_fd_lockerFile, LOCK_EX | LOCK_NB) >= 0)
		{
			bLocked = TRUE;
			// printf("--now locked file for system[%d], used time[%d]--\n", 
			// 	getpid(), 20 * (iTimes + 1));
			break;
		}
		Sleep_Ex(10);	
	}

	if(!bLocked)
	{
		printf("flock() error for heartbeat, pid[%d]!\n", getpid());
		flock(g_fd_lockerFile, LOCK_UN);
		Mutex_Unlock(g_hMutexSystemHandler);		
		return;
	}

	{
		pCmd = (CMD*)shmat(shmid_cmd,0,0);  
		if(pCmd == (CMD*)-1)
		{
			//printf("\n[dxi-syshand]:Write CMD is fail\n");
			flock(g_fd_lockerFile, LOCK_UN);
			Mutex_Unlock(g_hMutexSystemHandler);			
			return;
		}

		pCmd->iCmdType = 99; //means heartbeat
		pCmd->iCmdRet = -1;
		if(shmdt((void*)pCmd) == -1)
		{
			printf("\n\n[dxi-syshand]:shmdt is fail\n");
		}
		sem_post(psemWaitCmd);
		BOOL bAnswOk = FALSE;
		for(iTimes = 0; iTimes < 3000; iTimes++)
		{
			iSemRet = sem_trywait(psemAnswer);
			if(iSemRet == 0)
			{
				bAnswOk = TRUE;
				pCmd = (CMD*)shmat(shmid_cmd,0,0);  
				if(pCmd == (CMD*)-1)
				{
					printf("\n[dxi-syshand]:Write CMD is fail\n");
					break;
				}

				if(shmdt((void*)pCmd) == -1)
				{
					printf("\n\n[dxi-syshand]:shmdt is fail\n");
				}

				break;
			}
			//Sleep(300);
			Sleep_Ex(10);	
		}
		if(!bAnswOk)
		{
			TRACEX("\n\n[dxi-syshand]:Syshand process NO answer, need restart system\n");
			LogOut(MAIN_MODULE_PUBLIB, "SystemCall", LOG_TYPE_ERROR,
				"HEARTBEAT NO answer, need restart system.");
			//system("reboot");
			raise(SIGUSR1); 
		}
	}
	//RunThread_Heartbeat(RunThread_GetId(NULL));
	flock(g_fd_lockerFile, LOCK_UN);
	Mutex_Unlock(g_hMutexSystemHandler);	
	s_tmLastSendHeartBeat = tmNow;
}

#define _SYSTEM_LOCK_WAIT_TIMEOUT		30000
int _SYSTEM(char* pszCmd)
{
#define _SYSTEM_ERR_OK			0
#define _SYSTEM_ERR_CMD_EMPT	-5
#define _SYSTEM_ERR_EXE			-9
#define _SYSTEM_ERR_INI			-10
#define _SYSTEM_ERR_EXE_TIMEOUT	-11

	int nError = _SYSTEM_ERR_OK;
	CMD* pCmd;
	int iTimes = 0,iSemRet = 0;
#ifdef _DEBUG
	//return system(pszCmd);
	system(pszCmd);
	return _SYSTEM_ERR_OK;
#endif

	if((psemWaitCmd == NULL) || (psemAnswer == NULL) || (shmid_cmd == -1) ||
		(g_fd_lockerFile < 0))
	{
		BOOL bIni = InitSysHandler();
		if(!bIni)
		{
			printf("\n------system handler initialize error!!!\n");
			return _SYSTEM_ERR_INI;
		}
	}

	if (Mutex_Lock(g_hMutexSystemHandler, _SYSTEM_LOCK_WAIT_TIMEOUT) != ERR_MUTEX_OK)
	{
		return _SYSTEM_ERR_EXE_TIMEOUT;
	}
	//while(Mutex_Lock(g_hMutexSystemHandler,_LOCK_WAIT_TIMEOUT) != ERR_MUTEX_OK)
	//{
	//	
	//}

	//Jimmy added 20200605: lock it and unlock after execution
	BOOL bLocked = FALSE;
	for(iTimes = 0; iTimes < 300; iTimes++) //max wait 2sec
	{
		if(flock(g_fd_lockerFile, LOCK_EX | LOCK_NB) >= 0)
		{
			bLocked = TRUE;
			// printf("--now locked file for system[%d], used time[%d]--\n", 
			// 	getpid(), 20 * (iTimes + 1));
			break;
		}
		Sleep_Ex(10);	
	}

	if(!bLocked)
	{
		printf("flock() error for system[%s], pid[%d]!\n", pszCmd, getpid());
		flock(g_fd_lockerFile, LOCK_UN);
		Mutex_Unlock(g_hMutexSystemHandler);		
		return _SYSTEM_ERR_EXE_TIMEOUT;
	}

	if(strcmp(RemoveStringWhiteSpace(pszCmd),"") == 0)
	{
		nError = _SYSTEM_ERR_CMD_EMPT;
	}
	else
	{
		pCmd = (CMD*)shmat(shmid_cmd,0,0);  
		if(pCmd == (CMD*)-1)
		{
			nError = _SYSTEM_ERR_EXE;
			//printf("\n[dxi-syshand]:Write CMD is fail\n");
			flock(g_fd_lockerFile, LOCK_UN);
			Mutex_Unlock(g_hMutexSystemHandler);			
			return nError;
		}
		snprintf(pCmd->szCmd, sizeof(pCmd->szCmd), "%s", pszCmd);
		pCmd->iCmdType = 0;
		pCmd->iCmdRet = -1;
		if(shmdt((void*)pCmd) == -1)
		{
			printf("\n\n[dxi-syshand]:shmdt is fail\n");
		}
		sem_post(psemWaitCmd);
		BOOL bAnswOk = FALSE;
		for(iTimes = 0; iTimes < 3000; iTimes++)
		{
			iSemRet = sem_trywait(psemAnswer);
			if(iSemRet == 0)
			{
				bAnswOk = TRUE;
				pCmd = (CMD*)shmat(shmid_cmd,0,0);  
				if(pCmd == (CMD*)-1)
				{
					nError = _SYSTEM_ERR_EXE;
					printf("\n[dxi-syshand]:Write CMD is fail\n");
					break;
				}
				if(pCmd->iCmdRet == 0)
				{
					nError = _SYSTEM_ERR_OK;
				}
				else
				{
					nError = _SYSTEM_ERR_EXE;
				}
				if(shmdt((void*)pCmd) == -1)
				{
					printf("\n\n[dxi-syshand]:shmdt is fail\n");
				}

				break;
			}
			//Sleep(300);
			Sleep_Ex(10);	
		}
		if(!bAnswOk)
		{
			nError = _SYSTEM_ERR_EXE_TIMEOUT;
			TRACEX("\n\n[dxi-syshand]:Syshand process NO answer, need restart system\n");
			LogOut(MAIN_MODULE_PUBLIB, "SystemCall", LOG_TYPE_ERROR,
				"CMD[%s] NO answer, need restart system.", pszCmd);
			//system("reboot");
			raise(SIGUSR1); 
		}
	}
	RunThread_Heartbeat(RunThread_GetId(NULL));
	flock(g_fd_lockerFile, LOCK_UN);
	Mutex_Unlock(g_hMutexSystemHandler);	
	return nError;
}

int _SYSTEM_EX(char* pszCmd)
{
	int nError = _SYSTEM_ERR_OK;
	CMD* pCmd;
	int iTimes = 0,iSemRet = 0;

#ifdef _DEBUG
	FILE   *stream;
	char buf[64] = {0};
	stream = popen(pszCmd, "r");
	if(stream == NULL)
	{
		return -1;
	}
	fread(&buf, 64, 1,stream);
	pclose( stream );

	int nRtn = atoi(buf);
	return nRtn;
#endif

	if((psemWaitCmd == NULL) || (psemAnswer == NULL) || (shmid_cmd == -1) ||
			(g_fd_lockerFile < 0))
	{
		BOOL bIni = InitSysHandler();
		if(!bIni)
		{
			printf("\n------system handler initialize error!!!\n");
			return _SYSTEM_ERR_INI;
		}
	}

	if (Mutex_Lock(g_hMutexSystemHandler, _SYSTEM_LOCK_WAIT_TIMEOUT) != ERR_MUTEX_OK)
	{
		return _SYSTEM_ERR_EXE_TIMEOUT;
	}
	//while(Mutex_Lock(g_hMutexSystemHandler,_LOCK_WAIT_TIMEOUT) != ERR_MUTEX_OK)
	//{

	//}
	//Jimmy added 20200605: lock it and unlock after execution
	BOOL bLocked = FALSE;
	for(iTimes = 0; iTimes < 300; iTimes++) //max wait 3sec
	{
		if(flock(g_fd_lockerFile, LOCK_EX | LOCK_NB) >= 0)
		{
			bLocked = TRUE;
			//printf("--now locked file for system_ex[%d], used time[%d]--\n", 
			//	getpid(), 20 * (iTimes + 1));
			break;
		}
		Sleep_Ex(10);	
	}

	if(!bLocked)
	{
		printf("flock() error for system_ex[%s], pid[%d]!\n", pszCmd, getpid());
		flock(g_fd_lockerFile, LOCK_UN);
		Mutex_Unlock(g_hMutexSystemHandler);
		return _SYSTEM_ERR_EXE_TIMEOUT;
	}


	if(strcmp(RemoveStringWhiteSpace(pszCmd),"") == 0)
	{
		nError = _SYSTEM_ERR_CMD_EMPT;
	}
	else
	{
		pCmd = (CMD*)shmat(shmid_cmd,0,0);  
		if(pCmd == (CMD*)-1)
		{
			nError = _SYSTEM_ERR_EXE;
			//printf("\n[dxi-syshand]:Write CMD is fail\n");
			flock(g_fd_lockerFile, LOCK_UN);
			Mutex_Unlock(g_hMutexSystemHandler);			
			return nError;
		}
		snprintf(pCmd->szCmd, sizeof(pCmd->szCmd), "%s", pszCmd);
		pCmd->iCmdType = 1;
		pCmd->iCmdRet = -1;
		if(shmdt((void*)pCmd) == -1)
		{
			printf("\n\n[dxi-syshand]:shmdt is fail\n");
		}
		sem_post(psemWaitCmd);
		BOOL bAnswOk = FALSE;
		for(iTimes = 0; iTimes < 3000; iTimes++)
		{
			iSemRet = sem_trywait(psemAnswer);
			if(iSemRet == 0)
			{
				bAnswOk = TRUE;
				pCmd = (CMD*)shmat(shmid_cmd,0,0);  
				if(pCmd == (CMD*)-1)
				{
					nError = _SYSTEM_ERR_EXE;
					printf("\n[dxi-syshand]:Write CMD is fail\n");
					break;
				}
				//if(pCmd->iCmdRet == 0)
				//{
				//	nError = _SYSTEM_ERR_OK;
				//}
				//else
				//{
				//	nError = _SYSTEM_ERR_EXE;
				//}
				nError = pCmd->iCmdRet;
				if(shmdt((void*)pCmd) == -1)
				{
					printf("\n\n[dxi-syshand]:shmdt is fail\n");
				}

				break;
			}			
			//Sleep(300);
			Sleep_Ex(10);
		}
		if(!bAnswOk)
		{
			nError = _SYSTEM_ERR_EXE_TIMEOUT;
			TRACEX("\n\n[dxi-syshand]:Syshand process NO answer, need restart system\n");
			LogOut(MAIN_MODULE_PUBLIB, "SystemCall", LOG_TYPE_ERROR,
				"CMD[%s] NO answer, need restart system.", pszCmd);
			//system("reboot");
			raise(SIGUSR1);
		}
	}
	RunThread_Heartbeat(RunThread_GetId(NULL));
	flock(g_fd_lockerFile, LOCK_UN);
	Mutex_Unlock(g_hMutexSystemHandler);	
	return nError;
}


char *strncpyz(OUT char *pDst, IN const char *pSrc, IN int nDstLen)
{
	char *p = pDst;
	if (p == NULL)
	{
		return NULL;
	}
	if(pSrc != NULL)
	{
		for( nDstLen -= 1; *pSrc && (nDstLen > 0); nDstLen -- )
		{
			*p ++ = *pSrc ++;
		}
	}
	*p = '\0';   /* append a string end flag '\0'    */
	return pDst;
}


char *strncpyz_f(OUT char *pDst, IN const char *pSrc, IN int nDstLen, 
				REPLACE_ILLEGAL_CHR_PROC pfn)
{
    char *p = pDst;

    if (p == NULL)
	{
        return NULL;
	}

	if (pSrc != NULL)
	{
		for( nDstLen -= 1; *pSrc && (nDstLen > 0); nDstLen -- )
		{
			//*p = *pSrc ++;
			*p ++ = pfn(*pSrc ++);
		}
	}

    *p = '\0';   /* append a string end flag '\0'    */

    return pDst;
}


long GetFileLength( IN FILE *fp )
{
	long lCurPos = ftell( fp );     // get cur pos
	long lFileLen;
	
	fseek( fp, 0, SEEK_END );       // move pointer to end
	
	lFileLen = ftell( fp );         // get cur pos is the file length
	
	fseek( fp, lCurPos, SEEK_SET ); // restore the pos
	
	return lFileLen;
}


#define SELECT_MAX_WAIT_INTERVAL	MAX_WAIT_INTERVAL	// second


#define _USES_SELECT_SLEEP	1

//Jimmy note: only sleep max_sec then heartbeat
#define  MAX_SLEEEP_SEC_WITHOUT_HERATBEAT		10

#ifdef _USES_SELECT_SLEEP
void Sleep( IN DWORD dwMilliseconds )
{
	struct timeval tv;
	if (dwMilliseconds == 0)
	{
		sleep(0);
		return;
	}

	while (dwMilliseconds > 0)
	{
		if(MAX_SLEEEP_SEC_WITHOUT_HERATBEAT* 1000 <= dwMilliseconds)
		{
			RUN_THREAD_HEARTBEAT();
		}
	
		//20171118, modified by Jimmy, to remove heartbead macro here and do it in thread itself instead
		if (dwMilliseconds < (SELECT_MAX_WAIT_INTERVAL*1000))
		{
			tv.tv_usec = (suseconds_t)dwMilliseconds%1000*1000;		/* usec     */
			tv.tv_sec  = (time_t)dwMilliseconds/1000;			/* seconds  */

			dwMilliseconds = 0;
		}
		else
		{
			tv.tv_usec = 0;							/* usec     */
			tv.tv_sec  = SELECT_MAX_WAIT_INTERVAL;  /* seconds  */

			dwMilliseconds -= (SELECT_MAX_WAIT_INTERVAL*1000);

			RUN_THREAD_HEARTBEAT();
		}
		
		select(0,NULL,NULL,NULL, &tv); 
	}
}

#else

void Sleep( IN DWORD dwMilliseconds )
{
	int	nSeconds		= (int)(dwMilliseconds/1000);
	int	nMicroSeconds  = (int)(dwMilliseconds%1000*1000);

	// if no us, don't sleep.
	if (nMicroSeconds > 0)
	{
		usleep( (unsigned int)nMicroSeconds );
	}

	// sometimes, we need to sleep(0) to give up the CPU. 
	while (nSeconds >= 0)
	{
		//if(MAX_SLEEEP_SEC_WITHOUT_HERATBEAT <= nSeconds)
		{
			RUN_THREAD_HEARTBEAT();
		}

		if (nSeconds <= MAX_WAIT_INTERVAL)
		{
			sleep((unsigned int)nSeconds);
			break;
		}
		else
		{
			sleep(MAX_WAIT_INTERVAL);
			nSeconds -= MAX_WAIT_INTERVAL;
		}
	}
}
#endif

void Sleep_Ex( IN DWORD dwMilliseconds )
{
	struct timeval tv;

	if (dwMilliseconds == 0)
	{
		return;
	}

	while (dwMilliseconds > 0)
	{
		if (dwMilliseconds < (SELECT_MAX_WAIT_INTERVAL*1000))
		{
			tv.tv_usec = (suseconds_t)dwMilliseconds%1000*1000;		/* usec     */
			tv.tv_sec  = (time_t)dwMilliseconds/1000;			/* seconds  */

			dwMilliseconds = 0;
		}
		else
		{
			tv.tv_usec = 0;							/* usec     */
			tv.tv_sec  = SELECT_MAX_WAIT_INTERVAL;  /* seconds  */

			dwMilliseconds -= (SELECT_MAX_WAIT_INTERVAL*1000);
		}

		select(0,NULL,NULL,NULL, &tv); 
	}
}

char * NEW_strdup(IN const char *szSrc)
{
	if (szSrc == NULL)
	{
		return NULL;
	}

	char *p;
	size_t  iLen = strlen(szSrc);
	
	p = NEW(char, iLen + 1);
	if (p == NULL)
	{
		return NULL;
	}

	strcpy(p, szSrc);
	//p[iLen] = '\0';

	return p;
}

char * NEW_strdup_f(const char *szSrc,REPLACE_ILLEGAL_CHR_PROC pfn)
{
	if (szSrc == NULL)
	{
		return NULL;
	}

	char *p;
	size_t  iLen = strlen(szSrc);

	p = NEW(char, iLen + 1);
	if (p == NULL)
	{
		return NULL;
	}

	strncpyz_f(p, szSrc, (int)(iLen + 1), pfn);
	//strcpy(p, szSrc);
	//p[iLen] = '\0';

	return p;
}


char * NEW_straddz(const char *szSrc1, const char *szSrc2)
{
	char *szDst, *p;
	size_t iLen1, iLen2;

	if (szSrc1 == NULL || szSrc2 == NULL)
	{
		return NULL;
	}

	iLen1 = strlen(szSrc1);
	iLen2 = strlen(szSrc2);

	szDst = NEW(char, iLen1 + iLen2 +1);
	if (szDst == NULL)
	{
		return NULL;
	}
	
	strcpy(szDst, szSrc1);
	p = szDst + iLen1;
	strcpy(p, szSrc2);
	szDst[iLen1 + iLen2] = '\0';

	return szDst;
}

char * NEW_strcpy(char **szDst, const char *szSrc)
{
	size_t iDstLen, iSrcLen;
	char *p;

	if (szDst == NULL || szSrc == NULL)
	{
		return NULL;
	}

	if (*szDst == NULL)
	{
		return NULL;
	}

	iDstLen = strlen(*szDst);
	iSrcLen = strlen(szSrc);

	if (iDstLen >= iSrcLen)
	{
		strcpy(*szDst, szSrc);

		return *szDst;
	}
	else
	{
		p = NEW(char, iSrcLen + 1);

		if (p == NULL)
		{
			return NULL;
		}

		strcpy(p, szSrc);

		DELETE(*szDst);
		*szDst = p;
	
		return p;
	}
}


int WaitFiledReadable(int fd, int nmsTimeOut/* milliseconds*/)
{
    fd_set fdset, fderr;
    struct timeval tv;
    int rv = WAIT_DATA_TIMEOUT;

	while (nmsTimeOut > 0)
	{
		// need add ClearWDT() here.

		if (nmsTimeOut < (SELECT_MAX_WAIT_INTERVAL*1000))
		{
			tv.tv_usec = nmsTimeOut%1000*1000;		/* usec     */
			tv.tv_sec  = nmsTimeOut/1000;			/* seconds  */

			nmsTimeOut = 0;
		}
		else
		{
			tv.tv_usec = 0;							/* usec     */
			tv.tv_sec  = SELECT_MAX_WAIT_INTERVAL;  /* seconds  */

			RUN_THREAD_HEARTBEAT();

			nmsTimeOut -= (SELECT_MAX_WAIT_INTERVAL*1000);
		}

		if((fd >= FD_SETSIZE) || (fd < 0))
		{
			//printf("---------The fdSet[%d, %d] OVERAGE ERROR--------\n", fd, FD_SETSIZE);
			return WAIT_DATA_ERROR;
		}

		FD_ZERO(&fdset);		/*  Initializes the set to the NULL set. */
		FD_SET(fd, &fdset);		/*  Adds descriptor s to set. */

		fderr = fdset;
		
		/* determines the status of one or more sockets, waiting if necessary, 
		 * to perform synchronous I/O.     */
#ifdef unix
		rv = select(fd+1, &fdset, NULL, &fderr, &tv);
#else
		rv = select(FD_SETSIZE, &fdset, NULL, &fderr, &tv);
#endif

		if (rv > 0)	// data ready or error.
		{
			rv = (FD_ISSET(fd, &fdset)) ? WAIT_DATA_READY	// OK
				: (FD_ISSET(fd, &fderr)) ? WAIT_DATA_EXCEPT	// except
				: WAIT_DATA_ERROR;							// error??
			break;
		}
		else if (rv < 0)
		{
			rv = WAIT_DATA_ERROR;							// error??
			break;
		}
	}

    return rv;  /* select error, or time out */
}



int WaitFiledWritable(int fd, int nmsTimeOut/* milliseconds*/)
{
    fd_set fdset, fderr;
    struct timeval tv;
    int rv = WAIT_DATA_TIMEOUT;

	while (nmsTimeOut > 0)
	{
		// need add ClearWDT() here.
		if (nmsTimeOut < (SELECT_MAX_WAIT_INTERVAL*1000))
		{
			tv.tv_usec = nmsTimeOut%1000*1000;		/* usec     */
			tv.tv_sec  = nmsTimeOut/1000;			/* seconds  */

			nmsTimeOut = 0;
		}
		else
		{
			tv.tv_usec = 0;							/* usec     */
			tv.tv_sec  = SELECT_MAX_WAIT_INTERVAL;  /* seconds  */

			RUN_THREAD_HEARTBEAT();

			nmsTimeOut -= (SELECT_MAX_WAIT_INTERVAL*1000);
		}

		FD_ZERO(&fdset);		/*  Initializes the set to the NULL set. */
		FD_SET(fd, &fdset);		/*  Adds descriptor s to set. */

		fderr = fdset;
		
		/* determines the status of one or more sockets, waiting if necessary, 
		* to perform synchronous I/O.     */
#ifdef unix
		rv = select(fd+1, NULL, &fdset, &fderr, &tv);
#else
		rv = select(FD_SETSIZE, NULL, &fdset, &fderr, &tv);
#endif

		if (rv > 0)	// data ready or error.
		{
			rv = (FD_ISSET(fd, &fdset)) ? WAIT_DATA_READY	// OK
				: (FD_ISSET(fd, &fderr)) ? WAIT_DATA_EXCEPT	// except
				: WAIT_DATA_ERROR;							// error??
			break;
		}
		else if (rv < 0)
		{
			rv = WAIT_DATA_ERROR;							// error??
			break;
		}
	}

    return rv;  /* select error, or time out */
}

#ifdef unix
/***
*char *_strupr(string) - map lower-case characters in a string to upper-case
*
*Purpose:
*   _strupr() converts lower-case characters in a null-terminated string
*   to their upper-case equivalents.  Conversion is done in place and
*   characters other than lower-case letters are not modified.
*
*   In the C locale, this function modifies only 7-bit ASCII characters
*   in the range 0x61 through 0x7A ('a' through 'z').
*
*   If the locale is not the 'C' locale, MapStringW() is used to do
*   the work.  Assumes enough space in the string to hold result.
*
*Entry:
*   char *string - string to change to upper case
*
*Exit:
*   input string address
*
*Exceptions:
*   The original string is returned unchanged on any error.
*
*******************************************************************************/

char * strupr(char * string)
{
    char * cp;

    for (cp=string; *cp; ++cp)
    {
        if ('a' <= *cp && *cp <= 'z')
            *cp = (char)((*cp) + 'A' - 'a');
    }

    return(string);
}

#endif

int Dec2Bcd(int num)
{
    int bcd = 0;
	int i = 0;
	while (num > 0)
	{
		bcd |= (num % 10) << (i * 4);
		i++;
		num /= 10;
	}
	return bcd;
}

int Bcd2Dec(int num)
{
    int dec = 0;
    int temp = 1;
    while (num > 0)
    {
        dec += (num & 0xf) * temp;
        num >>= 4;
        temp *= 10;
    }
    return dec;
}

char* printableBcdTimeStmp(const BcdTimeStmp* pBcdTime)
{
#define SIZE_BUFF_BcdTimeStmp  (64)
	static char bufferBcdTimeStmp[SIZE_BUFF_BcdTimeStmp];
	snprintf(bufferBcdTimeStmp, SIZE_BUFF_BcdTimeStmp,
		"%02x%02x-%02x-%02x %02x:%02x:%02x:%03d %.3f",
		pBcdTime->yearH, pBcdTime->yearL, pBcdTime->month, pBcdTime->day,
		pBcdTime->hour, pBcdTime->minute, pBcdTime->second,
		pBcdTime->msecond, BcdTimeStmp2DoubleTime(pBcdTime));
	return bufferBcdTimeStmp;
}

BcdTimeStmp GetCurrentBcdTimeStmp()
{
	BcdTimeStmp timeStmp;
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	struct tm *timenow;
	timenow = localtime( &tv.tv_sec );
	int year = (timenow->tm_year + 1900);
    // 年月日
	timeStmp.yearH =   (BYTE)Dec2Bcd(year/100);
	timeStmp.yearL =   (BYTE)Dec2Bcd(year%100);
    timeStmp.month =   (BYTE)Dec2Bcd(timenow->tm_mon+1);
    timeStmp.day =     (BYTE)Dec2Bcd(timenow->tm_mday);
    // 时分秒
    timeStmp.hour =    (BYTE)Dec2Bcd(timenow->tm_hour);
    timeStmp.minute =  (BYTE)Dec2Bcd(timenow->tm_min);
    timeStmp.second =  (BYTE)Dec2Bcd(timenow->tm_sec);
	timeStmp.msecond =  tv.tv_usec/1000;
	return timeStmp;
}

time_t BcdTimeStmp2time_t(const BcdTimeStmp* pBcdTime)
{
    struct tm tm = { 0 };
    tm.tm_sec  = Bcd2Dec(pBcdTime->second);
    tm.tm_min  = Bcd2Dec(pBcdTime->minute);
    tm.tm_hour = Bcd2Dec(pBcdTime->hour);
    tm.tm_mday = Bcd2Dec(pBcdTime->day);
    tm.tm_mon  = Bcd2Dec(pBcdTime->month) - 1;
    tm.tm_year = Bcd2Dec(pBcdTime->yearH) * 100 + Bcd2Dec(pBcdTime->yearL) - 1900;
	return mktime(&tm);
}

double BcdTimeStmp2DoubleTime(const BcdTimeStmp* pBcdTime)
{
	time_t toTime_t = BcdTimeStmp2time_t(pBcdTime);
	return ((double)toTime_t + (double)pBcdTime->msecond/1000);
}

double GetCurrentTime(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (double)tv.tv_sec + (double)tv.tv_usec/(double)1000000;
}

UINT ElapseS(time_t timeS)
{
	return (UINT)(ABS(_NOW_ - timeS));
}

UINT ElapseMS(double dTimeMS)
{
	return (UINT)(ABS((GetCurrentTime() - dTimeMS) * 1000));
}

UINT ElapseMSBcdTimeStmp(const BcdTimeStmp* pBcdTime)
{
	return (UINT)(ABS((GetCurrentTime() - BcdTimeStmp2DoubleTime(pBcdTime)) * 1000));
}

BOOL HasEMSFeature(SYSTEM_TYPE emSystemType)
{
	return (emSystemType==SYSTYPE_ESS_AIR || emSystemType==SYSTYPE_ESS_LIQUIDED);
}

HANDLE LoadDynamicLibrary(IN const char *pszLibName,
					   IN int			nSym,
					   IN const char	*ppszSymName[],
					   OUT HANDLE		*pfnProc[],
					   IN BOOL			bNeedAllSym)
{
	int		i;
	HANDLE  hLib;

	//1. open lib
	printf("[BOOT] dlopen: loading %s... ", pszLibName); fflush(stdout);
	hLib = (HANDLE)dlopen(pszLibName, RTLD_LAZY);
	if (hLib == NULL)
	{
		const char *err = dlerror();
		printf("FAILED: %s\n", err); fflush(stdout);
		TRACE("[LoadDynamicLibrary] -- dlopen %s fails on error [%s].\n",
			pszLibName, err);
		return NULL;
	}

	//2. load all symbols
	for (i = 0; i < nSym; i++)
	{
		*pfnProc[i] = (HANDLE)dlsym(hLib, ppszSymName[i]);
		if (*pfnProc[i] == NULL)
		{
			TRACE("[LoadDynamicLibrary] -- Fails on getting the "
				"address of \"%s\".\n",
				ppszSymName[i]);

			if (bNeedAllSym)	// error. due to all sym shall be found
			{				
				dlclose(hLib);
				return NULL;
			}
		}
	}

	return hLib;	// OK
}


void UnloadDynamicLibrary(IN HANDLE hLib)
{
	if (hLib != NULL)
	{
		dlclose(hLib);
	}
}

char *MakeFullFileName(OUT char *pFullName,
							  IN const char *pszRootDir,
							  IN const char *pszSubDir,
							  IN const char *pszFileName)
{
	if ((pszRootDir == NULL) || (*pszRootDir == 0))
	{
		pszRootDir = ".";
	}

	if ((pszSubDir == NULL) || (*pszSubDir == 0))
	{
		pszSubDir = ".";
	}

	snprintf(pFullName, MAX_FILE_PATH, "%s/%s/%s",
		pszRootDir, pszSubDir, pszFileName);

	return pFullName;
}

int FindIntItemIndex(IN int nItemToFind, IN int *pIntArray, IN int nItemNum)
{
	int i;

	if (pIntArray != NULL)
	{
		for (i = 0; i < nItemNum; i++, pIntArray++)
		{
			if(nItemToFind == *pIntArray)
			{
				return i;
			}
		}
	}

	return -1;
}


char *TimeToString(IN time_t tmTime, const char *fmt, 
				   OUT char *strTime, IN int nLenStrTime)
{
	struct tm gmTime;

	gmtime_r(&tmTime, &gmTime);

	// convert time to yyyy-mm-dd hh:mm:ss
	strftime(strTime, (size_t)nLenStrTime, fmt, &gmTime);

	return strTime;
}

time_t StringToTime(IN char* strTime, IN const char *fmt)
{
	struct tm gmTime;
	memset(&gmTime, 0, sizeof(struct tm)); //must add this, if not ,result maybe invalid!!!
	if(strptime(strTime, fmt, &gmTime) == NULL)
	{
		return 0; //added by Jimmy 20201022, if failed should set to 0 
	}
	//TRACE("----strTime = [%s]. tmYMD[%04d-%02d-%02d %02d:%02d:%02d]----\n",strTime,
	//	gmTime.tm_year + 1900, gmTime.tm_mon + 1, gmTime.tm_mday, 
	//	gmTime.tm_hour, gmTime.tm_min, gmTime.tm_sec);
	return mktime(&gmTime);
}

time_t ConvDateTime_2DateOnly(time_t tt)
{
	struct tm gmTime;
	gmtime_r(&tt, &gmTime);
	gmTime.tm_sec = 0;
	gmTime.tm_min = 0;
	gmTime.tm_hour = 0;

	time_t tmNew = mktime(&gmTime);
	TRACEX("---Conv time(%d) -> date(%d)---", tt, tmNew);
	
	return tmNew;
}


char *AcurTimeToString(IN double fTime, const char *fmt, OUT char *strTime, IN int nLenStrTime)
{
	struct tm gmTime;

	time_t tmTime = (time_t)fTime;

	double fLeftTime = fTime - tmTime;

	gmtime_r(&tmTime, &gmTime);

	// convert time to yyyy-mm-dd hh:mm:ss
	strftime(strTime, (size_t)nLenStrTime, fmt, &gmTime);

	char szLeftTm[8] = {0};
	sprintf(szLeftTm, ".%03d", ABS((int)(fLeftTime * 1000)));

	strcat(strTime, szLeftTm);

	return strTime;
}

BOOL CheckInt_AndParse(IN char* szSrc, OUT int* pnOut)
{
	char *p = szSrc;
	int nLen = (int)strlen(szSrc);
	int n;
	for(n = 0; n < nLen; n++, p++)
	{
		if(((*p) < '0') || ((*p) > '9'))
		{
			return FALSE;
		}
	}
	*pnOut = atoi(szSrc);
	return TRUE;
}

int GetSplitStringCount(IN char *p, IN int cSplitter)
{
	int  cnt = 0;

	while (*p)
	{
		// skip the first splitters and white space
		while ((*p != 0) && ((*p == (char)cSplitter) || IS_WHITE_SPACE(*p)))
		{
			p++;
		}

		// set dest addr ref.
		cnt++;

		// skip the non-white chars
		while ((*p != 0) && (*p != (char)cSplitter))
		{
			p++;
		}
	}

	return cnt;
}

char *SplitString(IN char *pszSrc, OUT char **ppszDst, IN int cSplitter)
{
	char    *p;
	ASSERT(pszSrc != NULL);

	/* skip the first splitters and white space */
	p = pszSrc;
	
	while ((*p != 0) && ((*p == (char)cSplitter) || IS_WHITE_SPACE(*p)))
	{
		p++;
	}


	/* set dest addr ref */
	*ppszDst = p;
	
	for (; (*p != 0) && (*p != (char)cSplitter); p++)
	{

	}

	if (*p == (char)cSplitter)
	{
		*p = 0; // set the end of string
		p++;   // goto next string
	}

	RemoveStringWhiteSpace(*ppszDst); // remove the rear space

	return p;
}

char *Trim_String(IN char *pszBuf, char cTrim)
{
	char    *p = pszBuf;
	char    *pFirstNonSpace = pszBuf, *pLastWhiteSpace = NULL;
	BOOL    bFound = FALSE;  /* first non-space character found flag */

	while (*p)
	{
		if ((*p) == cTrim)
		{
			if(pLastWhiteSpace == NULL)
			{
				pLastWhiteSpace = p;
			}
		}
		else 
		{
			if (!bFound)
			{
				pFirstNonSpace = p;
				bFound = TRUE;
			}

			pLastWhiteSpace = NULL;
		}

		p++;
	}

	if (pLastWhiteSpace != NULL)
	{
		*pLastWhiteSpace = 0;
	}

	return pFirstNonSpace;
}



static char *SplitISO8601Text(IN char *S, OUT char *D, IN int cSep, IN int *bTimeZone)
{
	int i = 0;

	// 1. get rid of the front space
	while(*S == SPACE)
	{
		S++;
	}
	// 2. copy the non-space char
	while( *S )
	{
		//added by Jimmy 20210717
		if((*S ==' ') && (i < 15))
		{
			*S = 'T';
		}

		D[i++] = *S++;
		if(((*S == '+') || (*S == '-')) && (i >= 15)) //avoid mix with %Y-%M-%D
		{
			*bTimeZone = TRUE;
		}

		if (*S == (char)cSep)	// skip COMMA_SKIP
		{
			//printf("Skip .\n");
			//S = S + 4;
			S++;
			while((*S >= '0') && (*S <= '9'))
			{
				S++;
				if((*S == '+') || (*S == '-'))
				{
					*bTimeZone = TRUE;
				}
			}
		}
	}
	// 3. get rid of the rear space
	i --;
	while((i >= 0) && (D[i] == SPACE))
	{
		i --;
	}
	i ++;

	D[i]=0;
	//TRACE("S = %s D = %s i =%d \n",S ,D,i);

	return D;
}

/*
dateTimeOfDayBasic = calendarDateBasic, "T", timeCompleteBasic,
[ "Z" | UTCoffsetBasic ] ;
dateTimeOfDayExt   = calendarDateExt, "T", timeCompleteExt,
[ "Z" | UTCoffsetExt ] ;

Time : hh:mm:ss.sss  or  hhmmss.sss 

Basic format:
	YYYYMMDDThhmmss			Example:19850412T101530
	YYYYMMDDThhmmssZ				19850412T101530Z
	YYYYMMDDThhmmss±hhmm			19850412T101530+0400
	YYYYMMDDThhmmss±hh				19850412T101530+04

Extended format: 
	YYYY-MM-DDThh:mm:ss		Example:1985-04-12T10:15:30
	YYYY-MM-DDThh:mm:ssZ			1985-04-12T10:15:30Z   //Z UTC Time 0 Time Zone
	YYYY-MM-DDThh:mm:ss±hh:mm		1985-04-12T10:15:30+04:00
	YYYY-MM-DDThh:mm:ss±hh			1985-04-12T10:15:30+04
*/
time_t Parse_ISO8601TimeStr(char *pTimeStr)
{
	int bHasTimeZone = FALSE;
	struct tm time_struct;
	time_t t = _NOW_;
	int sec_gmtoff;
	memset(&time_struct, 0 ,sizeof(struct tm));

	char p[MAX_TIME_LEN], *pszOut = pTimeStr;
	if ((pTimeStr == NULL) || (*pTimeStr == 0) || strlen(pTimeStr) >= MAX_TIME_LEN)	// too long.
	{
		TRACE("Error Time String\n");
		return t;
	}
	pszOut = SplitISO8601Text(pszOut, p, COMMA_SKIP,&bHasTimeZone);

	//TRACE("After pszOut  = %s bHasTimeZone =%s \n\n",pszOut,(bHasTimeZone==TRUE)?"TRUE":"FALSE");

	if(strptime(pszOut,ISO8601_FMT_1,&time_struct))
	{
		//TRACE("ISO8601_FMT_1 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld   \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;

		// t = mktime(&time_struct) - sec_gmtoff;
		t = mktime(&time_struct); //Do not use time zone to update the system time. Added by Jeffery 20241025.
	}
	else if(strptime(pszOut,ISO8601_FMT_2,&time_struct) && bHasTimeZone == FALSE)
	{
		//TRACE("ISO8601_FMT_2 : %d-%d-%d  %d:%d:%d  tm_gmtoff = %ld \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		t = mktime(&time_struct);
	}
	else if(strptime(pszOut,ISO8601_FMT_3,&time_struct))
	{
		//TRACE("ISO8601_FMT_3 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld  \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;

		// t = mktime(&time_struct) - sec_gmtoff;
		t = mktime(&time_struct); //Do not use time zone to update the system time. Added by Jeffery 20241025.
	}
	else if(strptime(pszOut,ISO8601_FMT_4,&time_struct) && bHasTimeZone == FALSE)
	{
		//TRACE("ISO8601_FMT_4 : %d-%d-%d  %d:%d:%d  tm_gmtoff = %ld \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, time_struct.tm_gmtoff);
		t = mktime(&time_struct);
	}

	return t;
}

int Parse_ISO8601TimeZone(char *pTimeStr)
{
	int bHasTimeZone = FALSE;
	struct tm time_struct;
	time_t t = _NOW_;
	int sec_gmtoff;
	int hour_gmtoff; //Calculate the time zone from ISO8602 for cloud message. Added by Jeffery 20241025.
	memset(&time_struct, 0 ,sizeof(struct tm));

	char p[MAX_TIME_LEN], *pszOut = pTimeStr;
	if ((pTimeStr == NULL) || (*pTimeStr == 0) || strlen(pTimeStr) >= MAX_TIME_LEN)	// too long.
	{
		TRACE("Error Time String\n");
		return 0;
	}
	pszOut = SplitISO8601Text(pszOut, p, COMMA_SKIP,&bHasTimeZone);

	//TRACE("After pszOut  = %s bHasTimeZone =%s \n\n",pszOut,(bHasTimeZone==TRUE)?"TRUE":"FALSE");

	hour_gmtoff = 0;

	if(strptime(pszOut,ISO8601_FMT_1,&time_struct))
	{
		//TRACE("ISO8601_FMT_1 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld   \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;
		hour_gmtoff = sec_gmtoff / 3600;
	}
	else if(strptime(pszOut,ISO8601_FMT_2,&time_struct) && bHasTimeZone == FALSE)
	{
		//TRACE("ISO8601_FMT_2 : %d-%d-%d  %d:%d:%d  tm_gmtoff = %ld \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		t = mktime(&time_struct);
	}
	else if(strptime(pszOut,ISO8601_FMT_3,&time_struct))
	{
		//TRACE("ISO8601_FMT_3 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld  \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;
		hour_gmtoff = sec_gmtoff / 3600;
	}
	else if(strptime(pszOut,ISO8601_FMT_4,&time_struct) && bHasTimeZone == FALSE)
	{
		//TRACE("ISO8601_FMT_4 : %d-%d-%d  %d:%d:%d  tm_gmtoff = %ld \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, time_struct.tm_gmtoff);
		t = mktime(&time_struct);
	}

	return hour_gmtoff;

}


BOOL Parse_ISO8601TimeStrEx(char *pTimeStr, OUT time_t* pOutTime)
{
	int bHasTimeZone = FALSE;
	struct tm time_struct;
	time_t t = 0;
	int sec_gmtoff;
	memset(&time_struct, 0 ,sizeof(struct tm));

	char p[MAX_TIME_LEN], *pszOut = pTimeStr;
	if ((pTimeStr == NULL) || (*pTimeStr == 0) || strlen(pTimeStr) >= MAX_TIME_LEN)	// too long.
	{
		TRACE("Error Time String\n");
		return FALSE;
	}
	pszOut = SplitISO8601Text(pszOut, p, COMMA_SKIP,&bHasTimeZone);

	//TRACE("After pszOut  = %s bHasTimeZone =%s \n\n",pszOut,(bHasTimeZone==TRUE)?"TRUE":"FALSE");
	//added by Jimmy 20201009, for web support 
	if(strptime(pszOut,TIME_CHN_FMT,&time_struct))
	{
		//TRACE("ISO8601_FMT_1 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld   \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;
		t = mktime(&time_struct) - sec_gmtoff;
	}
	else if(strptime(pszOut,ISO8601_FMT_1,&time_struct))
	{
		//TRACE("ISO8601_FMT_1 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld   \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;
		t = mktime(&time_struct) - sec_gmtoff;
	}
	else if(strptime(pszOut,ISO8601_FMT_2,&time_struct) && bHasTimeZone == FALSE)
	{
		//TRACE("ISO8601_FMT_2 : %d-%d-%d  %d:%d:%d  tm_gmtoff = %ld \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		t = mktime(&time_struct) ;
	}
	else if(strptime(pszOut,ISO8601_FMT_3,&time_struct))
	{
		//TRACE("ISO8601_FMT_3 : %d-%d-%d  %d:%d:%d tm_gmtoff = %ld  \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec,time_struct.tm_gmtoff);
		sec_gmtoff = time_struct.tm_gmtoff;
		t = mktime(&time_struct) - sec_gmtoff;
	}
	else if(strptime(pszOut,ISO8601_FMT_4,&time_struct) && bHasTimeZone == FALSE)
	{
		//TRACE("ISO8601_FMT_4 : %d-%d-%d  %d:%d:%d  tm_gmtoff = %ld \n\n", 
		//	time_struct.tm_year, time_struct.tm_mon, time_struct.tm_mday,time_struct.tm_hour, time_struct.tm_min, time_struct.tm_sec, time_struct.tm_gmtoff);
		t = mktime(&time_struct) ;
	}

	*pOutTime = t;

	return (t == 0) ? FALSE : TRUE;
}


static char dec2hex(char c)
{
	if (/*0 <= c && */c <= 9) 
	{
		return c + '0';
	} 
	else if (10 <= c && c <= 15) 
	{
		return c + 'A' - 10;
	} 
	else 
	{
		return 0;
	}
}

static BOOL IsCharReservedRFC3986(char c)
{
	static const char* s_RFCReserveCharSet = "!*'();:@&=+$,/?#[]";
	int n;
	for(n = 0; n < strlen(s_RFCReserveCharSet); n++)
	{
		if(s_RFCReserveCharSet[n] == c)
		{
			return TRUE;
		}
	}
	return FALSE;
}

char* Percent_EncodeUrl(char* szSrcUrl)
{
	int i = 0;
	int len = MIN(strlen(szSrcUrl), 255);
	int res_len = 0;
	char res[255];

	for (i = 0; i < len; ++i) 
	{
		char c = szSrcUrl[i];
		if (('0' <= c && c <= '9') ||
			('a' <= c && c <= 'z') ||
			('A' <= c && c <= 'Z') || 
			(c == '.') || (c == '-') ||
			(c == '_') || (c == '~')) //RFC3986 reserved char
		{
			res[res_len++] = c;
		}
		else if(IsCharReservedRFC3986(c))
		{
			res[res_len++] = c;
		}
		else
		{
			int j = (short)c;
			if (j < 0)
			{
				j += 256;
			}
			int i1, i0;
			i1 = j / 16;
			i0 = j - i1 * 16;
			res[res_len++] = '%';
			res[res_len++] = dec2hex((char)i1);
			res[res_len++] = dec2hex((char)i0);
		}
	}

	res[res_len] = '\0';
	strcpy(szSrcUrl, res);

	TRACEX("The complete OCPP server URL is [%s]\n", szSrcUrl);
	return szSrcUrl;
}

char* Percent_DecodeUrl(char* szSrcUrl)
{
	int i = 0;
	int len = MIN(strlen(szSrcUrl), 255);
	int res_len = 0;
	char res[255];
	char szHex[2];
	unsigned int cConv;
	char c;
	for (i = 0; i < len; i++) 
	{
		c = szSrcUrl[i];
		if(c == '%')
		{
			if(i < len - 2)
			{
				szHex[0] = szSrcUrl[i + 1];
				szHex[1] = szSrcUrl[i + 2];
				if(sscanf(szHex, "%02X", &cConv) > 0)
				{
					res[res_len++] = (char)cConv;
				}
				else if(sscanf(szHex, "%02x", &cConv) > 0)
				{
					res[res_len++] = (char)cConv;
				}
				i += 2;
			}
		}
		else
		{
			res[res_len++] = c;
		}
	}

	res[res_len] = '\0';
	strcpy(szSrcUrl, res);

	TRACEX("The decoded URL is [%s]\n", szSrcUrl);
	return szSrcUrl;
}

//---------------The following funcs are added by Jimmy 20200522 for comressing data on Sqlite table blob--------
static UINT MakeChkSum_4CompData(BYTE* pDataDest, int nSize)
{
	UINT nChkSum = 0;
	int n;
	for(n = 0; n < MIN(1000, nSize); n++)
	{
		nChkSum += pDataDest[0];
	}
	return nChkSum;
}

static void ParseCompStream_2Bytes(MY_COMPRESSED_STREAM* pSteam, BYTE* pOut)
{
	pOut[0] = pSteam->bySOI;
	VAR_VALUE varV;
	varV.ulValue = pSteam->nLenOfSrc;
	memcpy(pOut + 1, varV.abyValue, 4);
	varV.ulValue = pSteam->nChkSumOf1K;
	memcpy(pOut + 5, varV.abyValue, 4);
	varV.ulValue = pSteam->nLenOfDest;
	memcpy(pOut + 9, varV.abyValue, 4);
}

int sqlite3_bind_blob_compress(sqlite3_stmt* pStmt, int idx, const void* pDataSrc, int nSize, void** ppDataOut, void(*pfnDispose)(void*))
{
	UINT nCompMaxDest = compressBound(nSize);	
	BYTE* pDataDest = NULL;
	*ppDataOut = NULL;
	if(nCompMaxDest > 0)
	{
		pDataDest = NEW(BYTE, nCompMaxDest + MYCOMP_HEAD_LEN);
		if(pDataDest == NULL)
		{
			return -1;
		}
	}
	else
	{
		TRACEX("TTHHTEEEEEEEEEEEEEEEEEEEEEE");
		pDataDest = NEW(BYTE, MYCOMP_HEAD_LEN);
	}

	//now compress the data
	int nRet = compress(pDataDest + MYCOMP_HEAD_LEN, &nCompMaxDest, 
						(BYTE*)pDataSrc, (UINT)nSize);
	BYTE *pOutB = (BYTE*)pDataDest;
	if(nRet != Z_OK)
	{
		//failed, so we bind the raw data
		TRACEX("Bind failed %d, so we have to use raw data!\n", nRet);
		DELETE(pDataDest);
		return sqlite3_bind_blob(pStmt, idx, (void*)pDataSrc, nSize, pfnDispose);
	}
	else
	{
		TRACE("\nCOMP[%d->%d], COMP_DATA[%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X-%02X]\n", 
			nSize, nCompMaxDest, pOutB[MYCOMP_HEAD_LEN], pOutB[MYCOMP_HEAD_LEN + 1],
			pOutB[MYCOMP_HEAD_LEN + 2], pOutB[MYCOMP_HEAD_LEN + 3],
			pOutB[MYCOMP_HEAD_LEN + 4], pOutB[MYCOMP_HEAD_LEN + 5],
			pOutB[MYCOMP_HEAD_LEN + 6], pOutB[MYCOMP_HEAD_LEN + 7],
			pOutB[MYCOMP_HEAD_LEN + 8]);
	}
	

	//now bind the compressed data
	// MY_COMPRESSED_STREAM* pSteam = (MY_COMPRESSED_STREAM*)pDataDest;
	// pSteam->bySOI = MYCOMP_SOI;
	// pSteam->nLenOfSrc = (UINT)nSize;
	// pSteam->nChkSumOf1K = MakeChkSum_4CompData(pDataDest + MYCOMP_HEAD_LEN, nCompMaxDest);
	// pSteam->nLenOfDest = nCompMaxDest;	
	MY_COMPRESSED_STREAM steam;
	ZERO_POBJS(&steam, 1);
	steam.bySOI = MYCOMP_SOI;
	steam.nLenOfSrc = (UINT)nSize;
	steam.nChkSumOf1K = MakeChkSum_4CompData(pDataDest + MYCOMP_HEAD_LEN, nCompMaxDest);
	steam.nLenOfDest = nCompMaxDest;	
	//memcpy(pDataDest, &steam, MYCOMP_HEAD_LEN);
	ParseCompStream_2Bytes(&steam, pDataDest);
	// TRACEX("\n----------Now print the compressed data,rawsize=%d,compsize=%d-------\n", nSize, nCompMaxDest);
	// int n;	
	// for( n = 0; n < nCompMaxDest + MYCOMP_HEAD_LEN; n++)
	// {
	// 	TRACE("%02X-", pOutB[n]);
	// }
	// TRACEX("\n----------End of print-----------\n");

	*ppDataOut = pDataDest;
	return sqlite3_bind_blob(pStmt, idx, (void*)pDataDest, nCompMaxDest + MYCOMP_HEAD_LEN, pfnDispose);
}

void sqlite3_finalize_compressed(sqlite3_stmt* pStmt, void* pDataDest)
{
	SAFELY_DELETE(pDataDest);
	sqlite3_finalize(pStmt);
}

MY_COMPRESSED_STREAM ParseBytes_2CompStream(BYTE* pData)
{
	MY_COMPRESSED_STREAM steam;
	steam.bySOI = pData[0];
	VAR_VALUE varV;
	memcpy(varV.abyValue, pData + 1, 4);
	steam.nLenOfSrc = varV.ulValue;
	memcpy(varV.abyValue, pData + 5, 4);
	steam.nChkSumOf1K = varV.ulValue;
	memcpy(varV.abyValue, pData + 9, 4);
	steam.nLenOfDest = varV.ulValue;

	return steam;
}

BOOL sqlite3_parse_blob_uncompressed(sqlite3_stmt* pStmt, int idx, OUT void* pUncompDest, int nUnCompDestSize)
{
	int nRawBytes = sqlite3_column_bytes(pStmt, idx);
	if(nRawBytes < 1)
	{
		return FALSE;
	}
	BYTE *pDataUnCompRaw = NEW(BYTE, nRawBytes);
	if(pDataUnCompRaw == NULL)
	{
		TRACEX("Oh, can not allocate memory, so need quit now!\n");
		return FALSE;
	}
	memcpy(pDataUnCompRaw, sqlite3_column_blob(pStmt, idx), nRawBytes);
	BOOL bUnCompOK = FALSE;
	if(nRawBytes >= MYCOMP_HEAD_LEN)
	{
		//MY_COMPRESSED_STREAM* pSteam = (MY_COMPRESSED_STREAM*)pDataUnCompRaw;
		MY_COMPRESSED_STREAM steam = ParseBytes_2CompStream(pDataUnCompRaw); 
		MY_COMPRESSED_STREAM* pSteam = &steam;
		//TRACEX("The parsed Stream[%02X, %d, %08X, %d], rawBytes[%d]\n", steam.bySOI, steam.nLenOfSrc,
		//		steam.nChkSumOf1K, steam.nLenOfDest, nRawBytes);
		//memcpy(pSteam, pDataUnCompRaw, MYCOMP_HEAD_LEN);
		if((pSteam->bySOI == MYCOMP_SOI) && 
			(pSteam->nLenOfDest == (nRawBytes - MYCOMP_HEAD_LEN)))
		{			
			UINT nChkSum = MakeChkSum_4CompData(pDataUnCompRaw + MYCOMP_HEAD_LEN, nRawBytes - MYCOMP_HEAD_LEN);
			TRACEX("The ClacSUm and raw sum are [%d. %d, deslt %d]\n", 
				nChkSum, pSteam->nChkSumOf1K, pSteam->nLenOfDest);
			if(nChkSum == pSteam->nChkSumOf1K)
			{
				//TRACEX("Begin NEWWWWWWWWWWWWW %d\n" ,pSteam->nLenOfSrc);
				BYTE *pData2BeUncomp = NEW(BYTE, pSteam->nLenOfSrc);
				if(pData2BeUncomp == NULL)
				{
					TRACEX("Oh, can not allocate memory +1, so need quit now!\n");
					DELETE(pDataUnCompRaw);
					return FALSE;
				}
				TRACEX("Begin doing the uncompress for steam\n");
				int nRet = uncompress(pData2BeUncomp, &pSteam->nLenOfSrc, 
								pDataUnCompRaw + MYCOMP_HEAD_LEN, pSteam->nLenOfDest);
				if(nRet != Z_OK)
				{
					TRACEX("Oh, UnCompress Failed %d\n", nRet);
					DELETE(pDataUnCompRaw);
					DELETE(pData2BeUncomp);
					return FALSE;
				}
				//if(pSteam->nLenOfDest >= nUnCompDestSize)
				{
					//TRACEX("Compressed OK, now copy back the data\n");
					bUnCompOK = TRUE;
					memcpy(pUncompDest, pData2BeUncomp, MIN(nUnCompDestSize, pSteam->nLenOfSrc));
				}	
				//else
				// {
				// 	TRACEX("Oh, UnCompress success but size not match[%d, %d]\n", 
				// 			pSteam->nLenOfDest, nUnCompDestSize);
				// }	
				DELETE(pData2BeUncomp);						
			}
		}
	}

	if(!bUnCompOK)
	{
		TRACEX("UMCOMP FAILED so we think this is RAWData!\n");
		//if(nRawBytes >= nUnCompDestSize)
		{
			memcpy(pUncompDest, pDataUnCompRaw, MIN(nUnCompDestSize, nRawBytes));
			bUnCompOK = TRUE;
		}
	}
	
	DELETE(pDataUnCompRaw);
	
	return bUnCompOK;
}

size_t sqlite3_sizeof_blob_uncompressed(sqlite3_stmt* pStmt, int idx)
{
	int nRawBytes = sqlite3_column_bytes(pStmt, idx);
	if(nRawBytes < 1)
	{
		return 0;
	}
	BYTE *pDataUnCompRaw = NEW(BYTE, nRawBytes);
	if(pDataUnCompRaw == NULL)
	{
		TRACEX("Oh, can not allocate memory, so need quit now!\n");
		return 0;
	}
	memcpy(pDataUnCompRaw, sqlite3_column_blob(pStmt, idx), nRawBytes);
	if(nRawBytes >= MYCOMP_HEAD_LEN)
	{
		//MY_COMPRESSED_STREAM* pSteam = (MY_COMPRESSED_STREAM*)pDataUnCompRaw;
		MY_COMPRESSED_STREAM steam = ParseBytes_2CompStream(pDataUnCompRaw); 
		MY_COMPRESSED_STREAM* pSteam = &steam;
		//TRACEX("The parsed Stream[%02X, %d, %08X, %d], rawBytes[%d]\n", steam.bySOI, steam.nLenOfSrc,
		//		steam.nChkSumOf1K, steam.nLenOfDest, nRawBytes);
		//memcpy(pSteam, pDataUnCompRaw, MYCOMP_HEAD_LEN);
		if((pSteam->bySOI == MYCOMP_SOI) && 
			(pSteam->nLenOfDest == (nRawBytes - MYCOMP_HEAD_LEN)))
		{			
			UINT nChkSum = MakeChkSum_4CompData(pDataUnCompRaw + MYCOMP_HEAD_LEN, nRawBytes - MYCOMP_HEAD_LEN);
			//TRACEX("The ClacSUm and raw sum are [%d. %d, deslt %d]\n", 
			//	nChkSum, pSteam->nChkSumOf1K, pSteam->nLenOfDest);
			if(nChkSum == pSteam->nChkSumOf1K)
			{
				DELETE(pDataUnCompRaw);
				return (size_t)pSteam->nLenOfSrc;
			}
		}
	}
	
	DELETE(pDataUnCompRaw);
	TRACEX("CanNOT find the header od compData, soFAILED!!!\n");
	return 0;
}

static char szIMMU2_HWVer[32] = {0};
char* GetIMMU2_HWVersion(void)
{
	static BOOL s_bFirstCall = TRUE;
	if(s_bFirstCall)
	{
		s_bFirstCall = FALSE;
		if(_SYSTEM_EX("hostname | grep -c a02") > 0)
		{
			sprintf(szIMMU2_HWVer, "A02");
		}
		else if(_SYSTEM_EX("hostname | grep -c a01") > 0)
		{
			sprintf(szIMMU2_HWVer, "A01");
		}
		else
		{
			sprintf(szIMMU2_HWVer, "A00");
		}
	}
	return szIMMU2_HWVer;
}

BOOL FindMatchFile_FromDir(char* szFindPath, char* szMatchName, char* szFoundFileName)
{
	struct dirent* ent = NULL; 
    DIR *pDir = opendir(szFindPath);
	if(pDir == NULL)
	{
		return FALSE;
	}
	BOOL bFound = FALSE;
    while (NULL != (ent = readdir(pDir))) 
    { 
		if(szMatchName == NULL)
		{
			bFound = TRUE;
			break;
		}

        if(strstr(ent->d_name, szMatchName) != NULL)
		{
			if(szFoundFileName != NULL)
			{
				sprintf(szFoundFileName, ent->d_name);
			}
			bFound = TRUE;
			break;
		}
    } 

	closedir(pDir);
	return bFound;
}

BOOL JudgeIfRunInNew_Kernel_4(void)
{
	FILE *f = popen("uname -a | grep '4.19'", "r");
	if(f)
	{
		char szBuff[64];
		memset(szBuff, 0, 64);
		fread(szBuff, 48, 1, f);
		pclose(f);		
		if(strlen(szBuff) > 1)
		{
			return TRUE;
		}		
	}

	return FALSE;
}

static char* GetMedia_nDevPath_FromProcFileStr(char* szProcMedia, BOOL bIsUSB, char* szPath, int nPathLen)
{
	if(szProcMedia == NULL)
	{
		return NULL;
	}

	//step 1: find '/dev/xxx'
	//printf("-------------bIsUSB=>%d, szProcMedia=>[%s]------\n", bIsUSB, szProcMedia);
	char* szPos = strstr(szProcMedia, bIsUSB ? "/dev/sd" : "/dev/mmc");
	if(szPos == NULL)
	{
		//printf("-------------rrrrrrr-111111111------\n");
		return NULL;
	}

	//step 2: find the devPath
	char szDev[64];
	memset(szDev, 0, 64);
	int n;
	char c;
	for(n = 0; n < MIN(strlen(szPos), 63); n++)
	{
		c = szPos[n];
		if((n >= 7) && ((c == ' ') || (c == '\t')))
		{
			break;
		} 
		szDev[n] = c;
	}

	//step 4: find if the '/dev/xx' exists or NOT, it not, means it was NOT unmounted properly last time
	char szDevCmd[128];
	//snprintf(szDevCmd, 128, "ls %s 2>/dev/null | wc -l", szDev);
	//if(_SYSTEM_EX(szDevCmd) <= 0)
	if(access(szDev, F_OK) != 0)	
	{
		//printf("-------Can NOT find such dev => [%s]------\n", szDev);
		return NULL;
	}

	//step 5: fin the path
	szPos += strlen(szDev) + 1;
	char* szPos1 = strstr(szPos, bIsUSB ? "/media/sd" : "/media/mmc");
	if(szPos1 == NULL)
	{
		//printf("-------------rrrrrrr-22222222------\n");
		return NULL;
	}
	else if(szPos1 - szPos > 5)
	{
		//printf("-------------rrrrrrr-3333333------\n");
		return NULL;
	}

	//step 6: pare the path finally
	memset(szPath, 0, nPathLen);
	for(n = 0; n < MIN(strlen(szPos1), nPathLen - 1); n++)
	{
		c = szPos1[n];
		if((n >= 9) && ((c == ' ') || (c == '\t')))
		{
			break;
		} 
		szPath[n] = c;
	}

	//printf("-----OOOOOOk, The dev=>[%s], Path=>[%s]-----\n", szDev, szPath);

	return szPos1;
}

char* FindFilePath_In_USB_OrSD_MountedDir(BOOL bFindUSB, char* szFile2Find, char* szPath, int nPathLen)
{
	//static char szPath[64] = {0};
	//first judge if dev file exists
	BOOL bIsDevExists = FALSE;
	int n;
	if(bFindUSB)
	{
		//bIsDevExists = (_SYSTEM_EX("ls /dev/sd* 2>/dev/null | wc -l") > 0) ? TRUE : FALSE;
		if((access("/dev/sda", F_OK) == 0) || 
			(access("/dev/sdb", F_OK) == 0) ||
			(access("/dev/sdc", F_OK) == 0) ||
			(access("/dev/sdd", F_OK) == 0) ||
			(access("/dev/sde", F_OK) == 0) ||
			(access("/dev/sdf", F_OK) == 0) ||
			(access("/dev/sdg", F_OK) == 0) ||
			(access("/dev/sdh", F_OK) == 0))
		{
			bIsDevExists = TRUE;
		}
	}
	else
	{
		//bIsDevExists = (_SYSTEM_EX("ls /dev/mmc* 2>/dev/null | wc -l") > 0) ? TRUE : FALSE;
		if(access("/dev/mmcblk0", F_OK) == 0)
		{
			bIsDevExists = TRUE;
		}
	}

	//printf("\n---[%.3f]:bIsDevExists=>%d, bFindUSB=>[%d]---\n", GetCurrentTime(), bIsDevExists, bFindUSB);

	if(!bIsDevExists)
	{
		return NULL;
	}

	//then judge if dev was mounted and where it was mounted	
	BOOL bFoundPath = FALSE;
	FILE *fp = fopen("/proc/mounts", "r");
	if (fp == NULL)
	{
		return NULL;
	}
	char szMount[4096] = {0};
	//int nLen = 0;
	int nTimes = 0;
	char* szMedia;
	if(fread(szMount, 1, 4096, fp) > 5)
	{
		char* szPos = szMount;
		while (szPos != NULL) //Jimmy: may have multiple dev mounted, should search recuirisely
		{		
			//memset(szPath, 0, nPathLen);	
			char* szMedia = GetMedia_nDevPath_FromProcFileStr(szPos, bFindUSB, szPath, nPathLen);
			if(szMedia != NULL)
			{
				//nLen = strlen(szMedia);
				//snprintf(szPath, 64, Trim_String(szMedia, ' '));
				if(strlen(szPath) > 9)
				{
					// printf("\n---[%.3f]:nTimes=>%d, szPath=>[%s], szFile2Find=>[%s]---\n", 
					// 	GetCurrentTime(), nTimes, szPath, (szFile2Find == NULL) ? "NULL" : szFile2Find);
					if(szFile2Find == NULL)
					{
						bFoundPath = TRUE;
						break;
					}
					else
					{
						char szFileName[128];
						snprintf(szFileName, 128, "%s/%s", szPath, szFile2Find);
						if(access(szFileName, F_OK) == 0) 
						{
							bFoundPath = TRUE;
							break;
						}
					}
				}
			}
			else
			{
				//printf("-------------rrrrrrr-EEEE------\n");
				//break;
				szMedia = strstr(szPos, bFindUSB ? "/media/sd" : "/media/mmc");
				if(szMedia == NULL)
				{
					//printf("-------------rrrrrrr-EEEE------\n");
					break;
				}
			}
			//next loop
			szPos = szMedia + 10;
			nTimes++;
			if((szPos - szMount + 10) >= strlen(szMount))
			{
				break;
			}
		} 
	}	

	fclose(fp);

	return bFoundPath ? szPath : NULL;
}


#include "openssl/bio.h"
#include "openssl/evp.h"
#include "openssl/objects.h"
#include "openssl/x509v3.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/opensslv.h"
#include "openssl/md5.h"
BOOL Base64_Encode(const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size)
{
	BIO     * b64;
	BIO     * bmem;
	BUF_MEM * bptr;

	if((content == NULL) || 
		(output == NULL) || 
		(length <= 0) || 
		(output_size == NULL))
	{
		return FALSE;
	}		
	
	/* create bio */
	b64  = BIO_new (BIO_f_base64());
	bmem = BIO_new (BIO_s_mem());
	
	/* push */
	b64  = BIO_push(b64, bmem);
	
	if(BIO_write (b64, content, length) != length) 
	{
		BIO_free_all (b64);
		return FALSE;
	}

	if(BIO_flush (b64) != 1) 
	{
		BIO_free_all (b64);
		return FALSE;
	}

	/* now get content */
	BIO_get_mem_ptr (b64, &bptr);
	
	/* check output size */
	if((*output_size) < bptr->length) 
	{
		BIO_free_all (b64);
		*output_size = bptr->length;
		return FALSE;
	}

	memcpy(output, bptr->data, bptr->length - 1);
	output[bptr->length-1] = 0;

	BIO_free_all (b64);

	return TRUE;
}

BOOL Base64_Decode(const char * content, 
				  int          length, 
				  char       * output, 
				  int        * output_size)
{
	BIO     * b64;
	BIO     * bmem;

	if ((content == NULL) || 
		(output == NULL) || 
		(length <= 0) || 
		(output_size == NULL))
	{
		return FALSE;
	}		

	/* create bio */
	bmem = BIO_new_mem_buf ((void *) content, length);
	b64  = BIO_new (BIO_f_base64());
	//BIO_set_flags (b64, BIO_FLAGS_BASE64_NO_NL);
	
	/* push */
	bmem  = BIO_push(b64, bmem);
	
	int nReadSize = *output_size;
	int nRet = BIO_read (bmem, (void*)output, nReadSize);	
	//Jimmy added: because there can be of no new line
	if(nRet <= 0)
	{
		//printf("-------ERRRRRR, Will try NO new line MODE------\n");
		BIO_free_all (bmem);
		bmem = BIO_new_mem_buf ((void *)content, length);
		b64  = BIO_new (BIO_f_base64());
		BIO_set_flags (b64, BIO_FLAGS_BASE64_NO_NL);
		bmem  = BIO_push(b64, bmem);
		nRet = BIO_read (bmem, (void*)output, nReadSize);
	}
	//printf("The szBase64 = [%s], leng = %d, nReadSize = %d, decoded outbytes = %d\n",content, length, nReadSize, nRet);
	
	*output_size = nRet;
	BIO_free_all (bmem);

	return (nRet > 0) ? TRUE : FALSE;
}

BOOL MD5_Encode(const unsigned char * content, 
				  size_t     		length, 
				  unsigned char     * output,
				  int				nOutlen)
{
	if((nOutlen < 33) ||
		(length < 1) || (content == NULL) || 
		(output == NULL))
	{
		return FALSE;
	}

	//return (MD5(content, (size_t)length, output) == NULL) ? FALSE : TRUE;
	BYTE byMD5[MD5_DIGEST_LENGTH];
	memset(byMD5, 0, MD5_DIGEST_LENGTH);
	if(MD5(content, (size_t)length, byMD5) == NULL)
	{
		return FALSE;
	}	
	
	int n;
	for(n = 0; n < MD5_DIGEST_LENGTH; n++)
	{
		sprintf(output + (2 * n), "%02X", byMD5[n]);
	}	
	//printf("---publib-calc-md5, src->[%s], md5->[%s]---\n", content, output);

	return TRUE;    
}

char*  ConvertString_2UpperCase(char* src)
{
	int n;
	char c;
	for(n = 0; n < strlen(src); n++)
	{
		c = *(src + n);
		if(islower(c))
		{
			*(src + n) = toupper(c);
		}
	}
	
	return src;
}

char*  ConvertString_2LowerCase(char* src)
{
	int n;
	char c;
	for(n = 0; n < strlen(src); n++)
	{
		c = *(src + n);
		if(isupper(c))
		{
			*(src + n) = tolower(c);
		}
	}

	return src;
}


LC_TYPE GetLCType(UINT idxClientLC)
{
	if (idxClientLC == 0)
	{
		return LC_TYPE_1st;
	}

	if (idxClientLC % MASK_NUM_LC_4cab > 0)
	{
		return LC_TYPE_4cab;
	}
	else
	{
		if ((idxClientLC / MASK_NUM_LC_2nd > 0) && (idxClientLC % MASK_NUM_LC_2nd == 0))
		{
			return LC_TYPE_2nd;
		}
	}
	return LC_TYPE_3rd;
}
// nn xx xxx from 0
UINT GetIdx0_LC2nd(UINT idxClientLC)
{
	return (UINT)(idxClientLC / MASK_NUM_LC_2nd - 1);
}
// xx xx nnn from 0
UINT GetIdx0_LC4cab(UINT idxClientLC)
{
	return (UINT)(idxClientLC % MASK_NUM_LC_4cab -1);
}

char*  printableBytes(const BYTE* data, int len)
{
	if (data==NULL || len<1)
	{
		return "";
	}

#define SIZE_BUFF_PRINT  (1024*10)
	int nMaxLen = SIZE_BUFF_PRINT/3;
	static char buffer[SIZE_BUFF_PRINT] = "";
    int i = 0;
    memset(buffer, 0, sizeof(buffer));
	len = (len>nMaxLen) ? nMaxLen : len;
    for (i=0; i<len-1; i++)
    {
        sprintf(buffer+strlen(buffer), "%02X ", data[i]);
    }
    sprintf(buffer+strlen(buffer), "%02X", data[len-1]);
    return buffer;
}

// added by LPL 24-04-29 peintf with color
#include <sys/syscall.h>
typedef enum tagTraceType {
    TraceTypeDbug,
	TraceTypeInfo,
	TraceTypeWarn,
	TraceTypeEror,
	TraceTypeFatal,
	TraceType_MAX
} Enum_TraceType;

static pid_t gettid(void)
{
	return syscall(SYS_gettid);
}

static void trace_header(Enum_TraceType traceType)
{
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	struct tm *timenow;
	timenow = localtime( &tv.tv_sec );

#define SIZE_BUFF_TRACETYPE  (6)
	static char szTraceType[SIZE_BUFF_TRACETYPE];
	memset(szTraceType, 0, SIZE_BUFF_TRACETYPE);
	switch (traceType)
	{
	case TraceTypeDbug:
		strncpy(szTraceType, "dbug", SIZE_BUFF_TRACETYPE);
		break;
	case TraceTypeInfo:
		printf("\033[32m");
		strncpy(szTraceType, "info", SIZE_BUFF_TRACETYPE);
		break;
	case TraceTypeWarn:
		printf("\033[33m");
		strncpy(szTraceType, "warn", SIZE_BUFF_TRACETYPE);
		break;
	case TraceTypeEror:
		printf("\033[31m");
		strncpy(szTraceType, "eror", SIZE_BUFF_TRACETYPE);
		break;
	case TraceTypeFatal:
		printf("\033[35m");
		strncpy(szTraceType, "fatl", SIZE_BUFF_TRACETYPE);
		break;

	default:
		break;
	}

	printf("%04d-%02d-%02d %02d:%02d:%02d:%03d [%06x] <%s> ",
		timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday,
		timenow->tm_hour, timenow->tm_min, timenow->tm_sec,
		(int)tv.tv_usec/1000, (unsigned int)gettid(), szTraceType);
}

static void trace_tail(void)
{
	printf("\033[0m\n");
}

void trace_debug(const char *format, ...)
{
	trace_header(TraceTypeDbug);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
	//trace_tail();
}


void trace_info(const char *format, ...)
{
	trace_header(TraceTypeInfo);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	trace_tail();
}

void trace_warn(const char *format, ...)
{
	trace_header(TraceTypeWarn);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	trace_tail();
}

void trace_error(const char *file, int line, const char *format, ...)
{
	trace_header(TraceTypeEror);

	printf("F[%s] L[%d] ", file, line);
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	trace_tail();
}

void trace_fatal(const char *file, int line, const char *format, ...)
{
#define BUFF_SIZE_FATAL 1024
	char szBuff[BUFF_SIZE_FATAL];
	va_list args;

	trace_header(TraceTypeFatal);

	va_start(args, format);
	vsnprintf(szBuff, BUFF_SIZE_FATAL, format, args);
	va_end(args);

	/* 只解析一次参数：原实现先 vprintf 再 vsnprintf 复用同一个 va_list，属 UB（aarch64 上会读到越界） */
	printf("F[%s] L[%d] %s", file, line, szBuff);

	trace_tail();

	/* szBuff MUST 作为参数而非格式串传入：直接当格式串会让消息里的 '%' 被二次解释（格式串漏洞） */
	LogOut(MAIN_MODULE_PUBLIB, "LogFatal", LOG_TYPE_ERROR, "%s", szBuff);
}

BYTE* reverseBytes(BYTE* pBytes, int len)
{
	if (len > 1)
	{
		int i, half = len / 2;
		for (i = 0; i < half; i++)
		{
			BYTE tmp = pBytes[i];
			pBytes[i] = pBytes[len - i - 1];
			pBytes[len - i - 1] = tmp;
		}
	}
	return pBytes;
}

static BOOL s_bInitCloudIdx = FALSE;
BOOL Save_CloudToDb(time_t t_SaveTime, void *blobDataStorage, UINT nDataStorageLen)
{
	if(blobDataStorage == NULL)
	{
		printf("Save_CloudToDb: blobDataStorage is NULL!\n");
		return FALSE;
	}
	char* szErrMsg = NULL;
	sqlite3_stmt * init_stat;
	sqlite3* data_db = Open_Lock_Database(DB_TYPE_CLOUDDATA);
	if(data_db == NULL)
	{
		printf("Save_CloudToDb: Open_Lock_SqliteDB failed!\n");
		return FALSE;
	}

	if(!s_bInitCloudIdx)
	{//初始化时读取数据库当前最大索引
		s_bInitCloudIdx = TRUE;

		sqlite3_exec(data_db, "CREATE TABLE IF NOT EXISTS Data_CloudBackupInfo ("
		"Idx          		INTEGER  PRIMARY KEY,"
		"Time  				INTEGER NOT NULL,"
		"DataInfo   		BLOB NOT NULL)",
		NULL, NULL, &szErrMsg);
		if(szErrMsg != NULL)
		{
			TRACE("Can not create Data_CloudBackupInfo table, reason = [%s].\n", szErrMsg);
			sqlite3_free(szErrMsg);
			return FALSE;
		}

		sqlite3_prepare_v2(data_db, "select max(Idx) from Data_CloudBackupInfo;", -1, &init_stat, NULL);
		int nSqlRet = sqlite3_step(init_stat);
		if(nSqlRet == SQLITE_ROW)
		{
			s_nCurrCloudIdx = sqlite3_column_int(init_stat, 0) + 1;
			if(s_nCurrCloudIdx < 1 || s_nCurrCloudIdx > MAX_CLOUD_BACKUP_NUM)
			{
				s_nCurrCloudIdx = 1;
			}
		}
		else
		{
			s_nCurrCloudIdx = 1;
		}
		sqlite3_finalize(init_stat);
	}

	sqlite3_stmt * stat;
	void* pOut = NULL;
	sqlite3_prepare_v2(data_db, "replace into Data_CloudBackupInfo values (?, ?, ?);", -1, &stat, NULL);
	sqlite3_bind_int(stat, 1, s_nCurrCloudIdx);
	sqlite3_bind_int64(stat, 2, t_SaveTime);
	sqlite3_bind_blob_compress(stat, 3, (void*)blobDataStorage, nDataStorageLen, &pOut, NULL);

	int nSqlRet = sqlite3_step(stat);
	sqlite3_finalize_compressed(stat, pOut);//delete stat

	s_nCurrCloudIdx++;
	if(s_nCurrCloudIdx > MAX_CLOUD_BACKUP_NUM)
	{
		s_nCurrCloudIdx = 1;
	}

	Close_UnLock_Database(DB_TYPE_CLOUDDATA);
	
	return TRUE;
}
