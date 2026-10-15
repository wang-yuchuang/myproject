#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 
#include <sys/sem.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/wait.h>  

#include "handler.h"
#include "pthread.h"

#define _EXIT_OK 1
#define _EXIT_NOOK 0
#define _SEM_CMDWAIT_PATH "/_semSysHandWait"
#define _SEM_ANSWER_PATH "/_semSysHandAnsw"
#define _KEY_T_SYSHAND  (ftok("/app",15)) //syshand̵ID
//#define IPCKEY 8765
//#define _KEY_T_APP  (ftok("/app/bin/app",0)) //app̵ID
#define BOOL int
#define TRUE 1
#define FALSE 0

static BOOL InitThis(void);
static int HandleCmd(CMD* pCmd);
static int Handle_sh(CMD* pCmd);
static sem_t* psemWaitCmd = NULL; //
static sem_t* psemAnswer = NULL; //
static int shmid_cmd = -1; //cmdid,app (Ϊsyshand)
// static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER; //
// #define LOCK_THREAD_MUTEX()		pthread_mutex_lock(&gMutex)
// #define UNLOCK_THREAD_MUTEX()	pthread_mutex_unlock(&gMutex)
// #define DESTROY_THREAD_MUTEX()	pthread_mutex_destroy(&gMutex)

//static int shmid_answ = -1; //answerid,syshand
//static int* ithread(void* pArg);
//
//static int* ithread(void* pArg)
//{
//	int iCmdRet = ERR_OK;
//	CMD* pCmd;
//	BOOL bStopRun = *(BOOL*)(pArg);
//	int iExitCode = _EXIT_NOOK;	
//	//while(!bStopRun)
//	do
//	{
//		printf("\n------Jimmy, MAINLOOP begins: %d",bStopRun);
//		//sem_wait(psemWaitCmd);//ȴ
//		int iRr = sem_trywait(psemWaitCmd);
//		if(iRr != 0)
//		{
//			printf("\n------Jimmy, NO SIGNAL !!!!");
//			sleep(2);
//			continue;
//		}
//		pCmd = (CMD*)shmat(shmid_cmd,0,0);  
//		if(pCmd == (CMD*)-1)
//		{
//			printf("[sysHandler]:Read CMD is fail\n");
//			continue;
//		}
//		iCmdRet = HandleCmd(pCmd);
//		//pIRet = (int*)shmat(shmid_answ,0,0);
//		//(*pIRet) = iCmdRet;
//		pCmd->iCmdRet = iCmdRet;
//		if(shmdt((void*)pCmd) == -1)
//		{
//			printf("\n\n[sysHandler]:shmdt is fail\n");
//		}
//		//ͷźapp
//		sem_post(psemAnswer);
//	}
//	while(1);
//	return iExitCode;
//}
int Main_Loop(void)
{
	int iCmdRet = ERR_OK;
	CMD* pCmd;


	int iExitCode = _EXIT_NOOK;	
	BOOL bIsRun = TRUE;
	bIsRun = InitThis();
	//if(!bIsRun)
	//{
	//	raise(SIGUSR2); 
	//}
	
	//printf("\n\n-----[sysHandler]: Init OK!!!");
	//pthread_t gThread;
	//pthread_create(&gThread, NULL, ithread, (void*)&bStopRun);
	//for heartbeat of main APP, added by Jimmy 20201107
	time_t	tmLastHeartBeat = _NOW_;
	int		nNoHeartBeatCounter = 0;
	//Init_LogRepo(NULL); //to create the index
	LogOut_Ex(MAIN_MODULE_SYSHAND, "INIT_SYS", LOG_TYPE_MILESTONE,
                    "****SysHand process is up now!*******");
	while(bIsRun)
	{
		//LOCK_THREAD_MUTEX();
		//printf("\n------[sysHandler]Jimmy, MAINLOOP begins: %d",bIsRun);
		//sem_wait(psemWaitCmd);//
		time_t tmNow = time(NULL);
		//int iRr = sem_trywait(psemWaitCmd);
		struct timespec ts;
		ts.tv_sec = tmNow + 5;   //wait 5sec
        ts.tv_nsec = 0;
		int iRr = sem_timedwait(psemWaitCmd, &ts);
		if(iRr != 0)
		{
			//printf("\n------Jimmy, NO SIGNAL !!!!");
			//sleep(1);
			//UNLOCK_THREAD_MUTEX();			
			if(ABS(tmNow - tmLastHeartBeat) >= 10)
			{
				nNoHeartBeatCounter++;
				tmLastHeartBeat = tmNow;
			if(nNoHeartBeatCounter >= 18)
			{
				LogOut_Ex(MAIN_MODULE_SYSHAND, "HEARTBEAT", LOG_TYPE_ERROR,
					"Main APP heartbeat lost; keep serving as OS HAL (no reboot, no exit)");
				printf("[syshand] heartbeat lost; staying alive (reboot/exit disabled)\n");
				nNoHeartBeatCounter = 0;  // 重置计数器，避免反复刷日志（约每 180s 一条）
			}
		}
		//	Sleep_Ex(10);
			continue;
		}
		//printf("\n\n------[sysHandler]Jimmy, I received a cmd !!!!");
		tmLastHeartBeat = tmNow; //whatever we receive, we consider a heartbeat
		nNoHeartBeatCounter = 0;
		pCmd = (CMD*)shmat(shmid_cmd,0,0);  
		if(pCmd == (CMD*)-1)
		{
			printf("[sysHandler]:Read CMD is fail\n");
			//UNLOCK_THREAD_MUTEX();
			continue;
		}
		switch(pCmd->iCmdType)
		{
		case 1:
			iCmdRet = Handle_sh(pCmd);
			break;
		case 99:
			//printf("---received heartbeat from main app---\n");
			iCmdRet = 0;
			break;
		case 0:
		default:
			iCmdRet = HandleCmd(pCmd);
			break;
		}
		//iCmdRet = HandleCmd(pCmd);
		//pIRet = (int*)shmat(shmid_answ,0,0);
		//(*pIRet) = iCmdRet;
		pCmd->iCmdRet = iCmdRet;
		if(shmdt((void*)pCmd) == -1)
		{
			printf("\n\n[sysHandler]:shmdt is fail\n");
		}
		//ͷźapp
		sem_post(psemAnswer);
		//sleep(1); // 
		//UNLOCK_THREAD_MUTEX();
	}
	//while(0);
	//sleep(5);
	//pthread_join(gThread,&iExitCode);

	//Jimmy systemwhi 
	//char pszCmd[128];
	//sprintf("rm %s -f",_SYSHAND_PID_FILE);
	////system("reboot");
	//system(pszCmd);
 	//sleep(5);
	//DESTROY_THREAD_MUTEX();

	//sleep(3);
	//kill(-1, SIGUSR2); //send error so app knows, -1 means all process
	//raise(SIGUSR2);//raise can only send sig to itself

	return iExitCode;
}
static BOOL InitThis()
{
	// 
	psemWaitCmd =  sem_open(_SEM_CMDWAIT_PATH, O_CREAT,0666, 0);//źʼֵΪ0
	if(psemWaitCmd == SEM_FAILED || psemWaitCmd == NULL)
	{
		printf( "[sysHandler]:sem_open waitCmd Failed!\n" );
		return FALSE;
	}
	psemAnswer =  sem_open(_SEM_ANSWER_PATH, O_CREAT,0666, 0);//źʼֵΪ0
	if(psemAnswer == SEM_FAILED || psemAnswer == NULL)
	{
		printf( "[sysHandler]:sem_open semAnswer Failed!\n" );
		return FALSE;
	}
	//printf("\n\n[sysHandler]:init sem OK");
	//
	key_t keySys = _KEY_T_SYSHAND;
	//key_t keySys = IPCKEY;
	//printf("\n\n[sysHandler]:keyt is [%d]",keySys);
	//sleep(5);
	shmid_cmd = shmget(keySys,CMD_MEM_SIZE,0666 | IPC_CREAT);
	if(shmid_cmd == -1)
	{
		printf("\n\n[sysHandler]:shmget ini ERRRRR!!!");
		return FALSE;
	}
	{
		char* p = (char*)shmat(shmid_cmd, NULL, 0);
		if(p != (char*)-1) { memset(p, 0, CMD_MEM_SIZE); shmdt(p); }
	}
	//system("reboot");

	//added by Jimmy for app to judge this is ready
	system("touch /opt/appdata/data/syshand.run");

	return TRUE;
}
#define _SYS_EXE_OK				0
#define  _SYS_EXE_NO_SCRIPT		1
#define  _SYS_EXE_FAIL			2

static int HandleCmd(CMD* pCmd)
{
	int iType = pCmd->iCmdType;
	int iRet = _SYS_EXE_OK;
	//switch(iType)
	//{
	//case 0:
	//default:
	//	iRet = system(pCmd->szCmd);
	//	break;
	//}
	iRet = system(pCmd->szCmd);
	if (-1 == iRet)  
	{
		iRet = _SYS_EXE_NO_SCRIPT;
		printf("\n[sysHandler]:system script error!");  
	}
	else
	{
		//printf("\n[sysHandler]:exit status value = [0x%x]\n", iRet);  

		if(WIFEXITED(iRet))  
		{
			if(0 == WEXITSTATUS(iRet))  
			{
				iRet = _SYS_EXE_OK;
				//printf("\n[sysHandler]:run shell script->(%s) successfully.\n", pCmd->szCmd);  
			}
			else
			{
				printf("\n[sysHandler]:run shell script->(%s) fail, script exit code: %d\n", pCmd->szCmd, WEXITSTATUS(iRet));  
				iRet = _SYS_EXE_FAIL;
			}
		}
		else
		{
			printf("\n[sysHandler]:exit status = [%d]\n", WEXITSTATUS(iRet));
			iRet = _SYS_EXE_FAIL;
		}
	}
	return iRet;
}

static int Handle_sh(CMD* pCmd)
{
	FILE   *stream;
	char buf = '0';
	
	stream = popen(pCmd->szCmd, "r");
	
	if(stream == NULL)
	{
		printf("\n[sysHandler]: popen called failed or sh file not existed!!!\n");
		return -1;
	}

	fread(&buf,sizeof(char), 1,stream);

	pclose( stream );

	int nRtn = (int)(buf - '0');

	//printf("-----Received app cmd[%s], result = %d, buf = %c-----\n", pCmd->szCmd, nRtn, buf);

	return nRtn;
}

