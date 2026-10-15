/**
 *                       _oo0oo_
 *                      o8888888o
 *                      88" . "88
 *                      (| -_- |)
 *                      0\  =  /0
 *                    ___/`---'\___
 *                  .' \\|     |// '.
 *                 / \\|||  :  |||// \
 *                / _||||| -:- |||||- \
 *               |   | \\\  -  /// |   |
 *               | \_|  ''\---/''  |_/ |
 *               \  .-\__  '-'  ___/-. /
 *             ___'. .'  /--.--\  `. .'___
 *          ."" '<  `.___\_<|>_/___.' >' "".
 *         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 *         \  \ `_.   \_ __\ /__ _/   .-` /  /
 *     =====`-.____`.___ \_____/___.-`___.-'=====
 *                       `=---='
 *
 *     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *               佛祖保佑         永无BUG
 *
 *********************************************************************/

// #define _TRACE_PRINT_
#include <signal.h>
#include <setjmp.h>

#include "main.h"
#include "3rd_party/sqlite/sqlite3.h"
#include "../moduals/data_processor/dpr_loader.h"
#include "../moduals/samplers/sampler_loader.h"
#include "../comm_ports/comm_loader.h"
#include "../data_interfaces/dai_loader.h"
#include "../moduals/reporters/reporters_loader.h"

static jmp_buf s_JumpBufFaultError;
static CHARGER* s_pChargeThis;
#define MAIN_SLEEP_INTERVAL			100	// 100 msec. 
#define THREAD_MANAGER_INIT_TICKS   ((60*1000)/MAIN_SLEEP_INTERVAL+1)
static int s_nThreadManagerTicks = THREAD_MANAGER_INIT_TICKS;


static DWORD ThreadManager_EventHandler(IN DWORD dwThreadEvent, IN HANDLE hThread, IN const char *pszThreadName)
{
	UNUSED(pszThreadName);

	WatchDog_Feed();

	if(THREAD_EVENT_NO_RESPONSE == dwThreadEvent)
	{
		LogOut(MAIN_MODULE_MAIN, "ThreadManager", LOG_TYPE_ERROR,
			"Thread [%s] is no response, system will reboot now...", pszThreadName);

		s_pChargeThis->bExit = TRUE;
		s_pChargeThis->nExitCode  = SYS_EXIT_BY_THREAD;

		//continue this thread, but we need quit system
		//return THREAD_CONTINUE_THIS;
	}
	else if (THREAD_EVENT_CANNOT_BE_STOPPED == dwThreadEvent)
	{
		LogOut(MAIN_MODULE_MAIN, "ThreadManager", LOG_TYPE_ERROR,
			"Thread %s can NOT be stopped while exiting, it will be canceled.", pszThreadName);
		//continue this thread, but we need quit system
		//return THREAD_CONTINUE_THIS;
	}
	else if(THREAD_EVENT_IS_STOPPED == dwThreadEvent)
	{
		if(!s_pChargeThis->bExit)
		{
			LogOut(MAIN_MODULE_MAIN, "ThreadManager", LOG_TYPE_ERROR,
				"Thread [%s] is stopped, system will reboot now...", pszThreadName);

			s_pChargeThis->bExit = TRUE;
			s_pChargeThis->nExitCode  = SYS_EXIT_BY_THREAD;
		}

		//please do not do this, or else Thread Manager will be dead-locked.
		//return THREAD_CANCEL_ALL;
	}

	//feed ThreadManager itself
	s_nThreadManagerTicks = THREAD_MANAGER_INIT_TICKS;

	return THREAD_CONTINUE_RUN;
}


void Main_Exit(int nExitCode)
{
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "+---------------------------------------+");
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "***The application now is exiting(ExitCode:%d)......", nExitCode);
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "+--------------NCCU EXIT----------------+\n");

	SAMPLER* pSampler = s_pChargeThis->samplers.p;
	int n;
	for(n = 0; n < s_pChargeThis->samplers.nTotal; n++,pSampler++)
	{
		pSampler->bExit = TRUE;
	}

	REPORTER* pReporter = s_pChargeThis->reporters.p;
	for(n = 0; n < s_pChargeThis->reporters.nTotal; n++, pReporter++)
	{
		pReporter->bExit = TRUE;
	}

	s_pChargeThis->bExit = TRUE;
	s_pChargeThis->nExitCode = nExitCode;


	WatchDog_Close(FALSE);	// do NOT disable dog, to cause system restart by dog
	longjmp(s_JumpBufFaultError, nExitCode);
}

static BOOL Create_Charger(sqlite3* pDb)
{
	TRACE("Now Init Log Repo! time=[%ld]\n", time(NULL));
	//Now Init Log module
	if(!Init_LogRepo(pDb))
	{
		TRACE("Can not initialize log repository, so exit system!");
		return FALSE;
	}

	CHARGER* pCharger = NEW(CHARGER, 1); //Create the structure
	if(pCharger == NULL)
	{
		return FALSE;
	}
	pCharger->hRunDB = (HANDLE)pDb;
	pCharger->bExit = FALSE;
	pCharger->nExitCode = SYS_EXIT_NOTEXIT;
	pCharger->hqEvent = Queue_Create(MAX_EVENTS_QUEUE, sizeof(EVENT_ARG), 0);
	
	if(pCharger->hqEvent == NULL)
	{
		return FALSE;
	}
	s_pChargeThis = pCharger;
	return TRUE;
}

static BOOL Main_LoadModuals(void)
{
	printf("[BOOT] LoadModuals: 1/6 Init_CreateDataProcessor\n");
	if(!Init_CreateDataProcessor(s_pChargeThis))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadModuals", LOG_TYPE_ERROR,
			"Failed to create dpr!");
		return FALSE;
	}

	printf("[BOOT] LoadModuals: 2/6 LoadConfigAndData\n");
	if(!s_pChargeThis->dataprocessor.LoadConfigAndData(s_pChargeThis->hRunDB))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadModuals", LOG_TYPE_ERROR,
			"Failed to load config and data!");
		return FALSE;
	}	

	printf("[BOOT] LoadModuals: 3/6 Init_LoadDAI\n");
	if(!Init_LoadDAI(s_pChargeThis))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadModuals", LOG_TYPE_ERROR,
			"Failed to load DAI!");
		return FALSE;
	}

	printf("[BOOT] LoadModuals: 4/6 Init_Load_AllCommPorts\n");
	if(!Init_Load_AllCommPorts(s_pChargeThis))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadCommports", LOG_TYPE_ERROR,
			"Failed to load CommPorts!");
		return FALSE;
	}

	//Now go to dynamic tasks, SQLITE shall be set to RUN-TIME mode!
	Set_SqliteDB_2RunTime_Mode();

	printf("[BOOT] LoadModuals: 5/6 Init_Load_AllSamplers\n");
	if(!Init_Load_AllSamplers(s_pChargeThis))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadModuals", LOG_TYPE_ERROR,
			"Failed to load samplers!");
		return FALSE;
	}

	printf("[BOOT] LoadModuals: 6/6 Init_Load_AllReporters\n");
	if(!Init_Load_AllReporters(s_pChargeThis))
	{
		LogOut(MAIN_MODULE_MAIN, "LoadModuals", LOG_TYPE_ERROR,
			"Failed to load reporters!");
		return FALSE;
	}

	printf("[BOOT] LoadModuals: all done\n");
	return TRUE;
}

static void Main_Run(void)
{
	s_pChargeThis->dataprocessor.DoDataProcessing(TRUE);
}

int main(int argc, char *argv[])
{
	if (Main_ProcessCmdLine(argc, argv) == -1)
	{
		printf("111111111111\n");
		return 0;
	}
#ifdef IMMU3
	const char* pszPlatform = "IMMU3";
#else
    const char* pszPlatform = "IMMU2";
#endif
	TRACEI("main::main %s %s %s", pszPlatform, __DATE__, __TIME__);
	MemoryPool_Init();

	//added by Jimmy 20210729, to wait the other processes to finish the DB operations before continue
	//printf("---------[%.3f]: Now BEGIN wait other DB OPerations--------\n", GetCurrentTime());
	int nWaitDBLocker_Times = 0;
	do
	{
		if(Lock_SqlDB_FileLocker(1000))
		{
			printf("Info: Lock SqlDB Success,means other Thread release the resource!\n");
			break;
		}
		nWaitDBLocker_Times++;
	} while (nWaitDBLocker_Times < 30);
	Unlock_SqlDB_FileLocker();
	//printf("---------[%.3f]: Now FINISH wait other DB OPerations[%d]--------\n", GetCurrentTime(), nWaitDBLocker_Times);
	
printf("[BOOT] nccu_app: step 1 - sqlite init\n");
	TRACE("Now Init sqlite3!\n");
	//sqlite3_config(SQLITE_CONFIG_MULTITHREAD); //multi-thread mode, each sql handle shall be created by its own thread
	sqlite3_config(SQLITE_CONFIG_SERIALIZED); //Serieal mode, only one handle should be used by different threads
	sqlite3_initialize();

printf("[BOOT] nccu_app: step 2 - open/create database\n");
	//Open/Create sqlDB
	char	szDBFile[256] = {0};
	sqlite3 *db;
	MakeFullFileName(szDBFile, \
		getenv(ENV_VARIABLE_DATA_PATH), NULL, "nccu.data");
	int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
	//BOOL bDataDBEmpty = FALSE;
	if(rc != SQLITE_OK)
	{
		//bDataDBEmpty = TRUE;
		rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
		if(rc != SQLITE_OK)
		{
			printf("Can not create data database[%s], [%s], so exit system!\n", szDBFile, getenv(ENV_VARIABLE_DATA_PATH));
			return -1;
		}
	}

	sqlite3_busy_timeout(db, 20000); //set default BUSY_HANDLER for sql database
	//sqlite3_exec(db,"PRAGMA vdbe_trace = ON;", NULL, NULL, NULL); //open debug switch
	//sqlite3_exec(db, "PRAGMA temp_store = 2;", NULL, NULL, NULL); // use memory to store temporary files

printf("[BOOT] nccu_app: step 3 - InitCreatDataBase\n");
	if(!InitCreatDataBase())
	{
		printf("Database initialization failed\n");
		return -1;
	}
printf("[BOOT] nccu_app: step 4 - Init_Database\n");
	if(!Init_Database())
	{
		printf("Failed to create the log table\n");
		return -1;
	}
	Database_Switch_RunMode();

printf("[BOOT] nccu_app: step 5 - Init_CreadDataDB\n");
	if(!Init_CreadDataDB((HANDLE)db))
	{
		printf("---Init_CreadDataDB Failed---\n");
		sqlite3_close(db);
		return -1;
	}

printf("[BOOT] nccu_app: step 6 - test_log_init\n");
	if (!test_log_init())//init log4c 
	{
		printf("[BOOT] nccu_app: step 6 FAILED - test_log_init returned FALSE\n");
		return -1;
	}
	//Log4cOut();
	LOG_INFO("log4c init successfully !!!");
	
	//1. Create charger
	TRACE("Now Create Charger!\n");
printf("[BOOT] nccu_app: step 7 - Create_Charger\n");
	if(!Create_Charger(db))
	{
		printf("---Create_Charger Failed---\n");
		LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_ERROR, "Cannot create charger, now exit!");
		return -1;
	}

	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "+-----------NCCU STARTUP----------+");
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "***The application now is initializing......");
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "+--------------------------+");

	//Jimmy added 20200312, if sys_hand not initilized OK, then quit the app
	int nMaxWait = 0;
printf("[BOOT] nccu_app: step 8 - waiting syshand.run\n");
while (access("/opt/appdata/data/syshand.run", R_OK) != 0)
{
	Sleep_Ex(100);
	nMaxWait++;
	if(nMaxWait > 100) // 1.1s -> 10.1s：run_app 的存活检查（pidof）可能先于 syshand 写完就绪标记通过，
	                  // 原 1.1s 在启动负载高时会造成误判退出（keep-app-alive 2026-09）
	{
		printf("[BOOT] nccu_app: step 8 FAILED - syshand.run not found after 10s\n");
		LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "***Initialize FAILED as sys_hand NOT OK (maybe in upgrade).....");
		return 0;
	} /* end if(nMaxWait > 100) */
}
printf("[BOOT] nccu_app: step 8 - syshand.run found\n");

	//2. Open Hardware watchdog
printf("[BOOT] nccu_app: step 9 - WatchDog_Open (disabled)\n");
	WatchDog_Open();

	//3. Install signal handlers 
printf("[BOOT] nccu_app: step 10 - Install_SigHandlers\n");
	if(!Install_SigHandlers((void*)s_pChargeThis))
	{
		LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_ERROR, "Failed to install sig handlers, now exit!");
		return -1;
	}
	
	//4.Now set exit jmp
	TRACE("Now Set Exit jmp!\n");
	int nExitCode = setjmp(s_JumpBufFaultError);
	if(nExitCode != 0)
	{
		printf("[BOOT] nccu_app: longjmp received! exitCode=%d\n", nExitCode); fflush(stdout);
		TRACE("Received a longJmp, code is(%d).\n", nExitCode);
//		if(nExitCode == SYS_EXIT_BY_INIT)
//		{
			//system("/app/stoprun");
//			//AppLogOut(SYS_MAIN, APP_LOG_ERROR,
//			//	"***** Longjumped for fault error(Code %d). System exiting.*****\n", nExitCode);
//			//tell qt to go to error page
//			//while(1)
//			//{
//			//	sleep(3);
//			//	//need to reboot manually
//			//	//do nothing
//			//}
//		}
//		else
//		{
//			//正常退出，无需记错误日志,sigHandeler已经记录
//			//tell qt to goto error page
//			//_SYSTEM("/sbin/reboot");  //用户通知的重启，则直接在这里处理			
//#ifndef _DEBUG
//			system("/sbin/reboot");
//#endif
//		}
//		TRACE("\n----Now wait 5 seconds and reboot!---\n");
//		Sleep(5000);
		//Jimmy note 20200426: after handled SIGTERM, should exit system 'gracefully'
		//LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "***Waiting threads being canceled......");
		RunThread_ManagerExit(10000, TRUE);	// must be at last
		_exit(nExitCode);	// must call _exit. do NOT call exit.
		//return nExitCode;
	}

	//5. Init ThreadManager
	TRACE("Now Init ThreadManager.");
	printf("[BOOT] nccu_app: step 12 - Init ThreadManager\n");
	if(RunThread_ManagerInit(ThreadManager_EventHandler) != ERR_THREAD_OK)
	{
		LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_ERROR, "Init thread manager failed, now exit!");
		return -1;
	}

	//6. Moduals loading
	TRACE("Now Load Moduals.\n");
	printf("[BOOT] nccu_app: step 13 - LoadModuals\n");
	if(!Main_LoadModuals())
	{
		LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_ERROR, "Failed to load moduals, now exit!");
		return -1;
	}

	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "---------Init done successfully!---------");
	s_pChargeThis->bIsInitOK = TRUE;
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "...The application now is running......");
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, "--- %s %s %s ver[%s, %s] ---",
		pszPlatform, __DATE__, __TIME__, s_pChargeThis->info.szSWVer, GetIMMU2_HWVersion());
	LOGFD("--- %s %s %s ver[%s, %s] ---",
		pszPlatform, __DATE__, __TIME__, s_pChargeThis->info.szSWVer, GetIMMU2_HWVersion());
	printf("[BOOT] nccu_app: step 14 - Main_Run\n");
	//7. Run
	Sleep(1000); //wait all moduals to be intialized
	Main_Run();
	//8 Do main run task
	printf("[BOOT] nccu_app: step 15 - entering event loop\n");
	EVENT_ARG ev_arg;
	s_pChargeThis->bResetCmdPending = FALSE;
	int nSysResetCounter = 0;
	int nLoopCount = 0;
	int nHeartFailCount = 0;
	while(!s_pChargeThis->bExit)
	{
		nLoopCount++;
		if((nLoopCount % 100) == 0) { printf("[BOOT] nccu_app: main loop alive (count=%d)\n", nLoopCount); fflush(stdout); }
		if(nLoopCount == 1) { printf("[BOOT] nccu_app: event loop iteration 1 start\n"); fflush(stdout); }

		//added by Jimmy 20201107 for syshand heartbeat so that we know app alive
		if(nLoopCount == 1) { printf("[BOOT] nccu_app: calling Do_Proc_HeartBeat\n"); fflush(stdout); }
		Do_Proc_HeartBeat();
		if(nLoopCount == 1) { printf("[BOOT] nccu_app: Do_Proc_HeartBeat done\n"); fflush(stdout); }

		//the Event loop
		if(Queue_Get(s_pChargeThis->hqEvent, &ev_arg, FALSE, 100) == ERR_QUEUE_OK)
		{
			if(nLoopCount <= 3) { printf("[BOOT] nccu_app: event received, calling DoEvent\n"); fflush(stdout); }
			s_pChargeThis->dataprocessor.DoEvent((HANDLE)&ev_arg);
			if(nLoopCount <= 3) { printf("[BOOT] nccu_app: DoEvent returned, bExit=%d\n", s_pChargeThis->bExit); fflush(stdout); }
			continue;
		}

		if(s_pChargeThis->bResetCmdPending) //handle exit after every event is handled properly
		{
			if(nSysResetCounter == 0)
			{
				LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_ERROR, 
					"Received system exit cmd, now system is exiting in 10 sec...");
			}
			nSysResetCounter++;
			if(nSysResetCounter >= 100) //wait max 10sec for handling the remaining events
			{
				s_pChargeThis->bExit = TRUE;
				_SYSTEM("reboot");
				break;				
			}
		}

		//TRACEX("-----------------s_nThreadManagerTicks = %d-----------------\n",s_nThreadManagerTicks);
		s_nThreadManagerTicks--;
		if(nLoopCount == 1) { printf("[BOOT] nccu_app: s_nThreadManagerTicks=%d\n", s_nThreadManagerTicks); fflush(stdout); }
		if (s_nThreadManagerTicks < 0)
		{
			LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_ERROR, 
				"PANIC: Thread manager is no response, the system will exit and restart!");

			s_pChargeThis->bExit = TRUE;
			s_pChargeThis->nExitCode = SYS_EXIT_BY_THREADMANAGER;
		}
	}

	int nResult = RunThread_ManagerExit(10000, TRUE);	// must be at last
	LogOut(MAIN_MODULE_MAIN, "Main", LOG_TYPE_MILESTONE, 
		"Run thread manager has exited with %d(%08X), %s.", 
		nResult, nResult, (nResult == ERR_THREAD_OK) ? "OK" : "FAILURE");

	Main_Exit(s_pChargeThis->nExitCode/*SYS_EXIT_BY_USER*/);  //if run here, the only reason is that user do this

	MemoryPool_Exit();
	Exit_LogRepo();
	sqlite3_close(db);
	ExitDatabase();
	sqlite3_shutdown();
	return s_pChargeThis->nExitCode; /* 传播退出原因：0=正常停止，非 0=故障（见 charger_model.h 的 SYS_EXIT_* / SYS_EXIT_BY_*） */
}


