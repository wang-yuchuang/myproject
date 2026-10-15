//#define TRACE_2DBLOG

#include <sys/types.h> 
#include <stdarg.h>
#include <pthread.h>
#include "std_sys.h"
#include "base_defines/iLog.h"
#include "base_defines/basefunc.h"
#include "app_incs.h"
//#include "base_defines/iThread.h"

static pthread_mutex_t	s_sqLiteDBLock = PTHREAD_MUTEX_INITIALIZER;
//#define LOCK_APP_LOG_INIT()	pthread_mutex_init(&s_hWriteLogLock, NULL)
//Jimmy 20210512: changed to trylock for safety
//#define LOCK_SQLITE_DB_SYNC()		pthread_mutex_lock(&s_sqLiteDBLock)
static BOOL Lock_SqlDB_NoBlock(int n_mSec)
{
	int iTimes = n_mSec / 10;
	do 
	{
		if(pthread_mutex_trylock(&s_sqLiteDBLock) == 0) 
		{
			//printf("--------Lock-SQLDB mutex OK => %d------\n", iTimes);
			return TRUE;
		}
		iTimes--; 
		Sleep_Ex(10);
	}while(iTimes > 0);

	//printf("--------Lock-SQLDB mutex FFFFFFailed => %d------\n", iTimes);
	return FALSE;
}
#define LOCK_SQLITE_DB_SYNC(n)		Lock_SqlDB_NoBlock(n)
#define UNLOCK_SQLITE_DB_SYNC()		pthread_mutex_unlock(&s_sqLiteDBLock)
#define DESTROY_SQLITE_DB_SYNC()	pthread_mutex_destroy(&s_sqLiteDBLock)

static sqlite3*	s_LogDB = NULL;
static BOOL s_bIsSqlIn_RuntimMode = FALSE; //when in runtime mode, shall locl/unlock each time
static BOOL	s_bLogRepoInitOK = FALSE;
static UINT	s_nCurrLogIdx = 1;
static UINT	s_nCurrDumpIdx = 1;
static UINT s_nCurrMBELogIdx = 1;
static UINT s_nBattDegrLogIdx = 1;
//use for log index and shall be locked over different thread/process
static int g_fd_locker_LogIndexFile = -1;
#define VAR_SET_LOGIDX_FILENAME	"/opt/appdata/data/.logIdx"
BOOL Lock_SqlDB_FileLocker(int nTimeOut_MSec)
{
	if(g_fd_locker_LogIndexFile <= 0)
	{
		g_fd_locker_LogIndexFile = open(VAR_SET_LOGIDX_FILENAME, O_RDWR | O_CREAT, 0644);
		if (g_fd_locker_LogIndexFile <= 0) 
		{
			//printf("open the g_fd_locker_LogIndexFile error!!!");
			return FALSE;
		}
	}

	if(nTimeOut_MSec < 0)
	{
		//printf("NO need use lock, so skip it!!!");
		return TRUE; //means no lock but do the read/write regardless of the LOCK
	}

	int iTimes = nTimeOut_MSec / 10;

	do
	{
		if(flock(g_fd_locker_LogIndexFile, LOCK_EX | LOCK_NB) >= 0)
		{
			//printf("****Lock OK within mSec => %d!!!\n", iTimes * 10);
			return TRUE;
		}
		Sleep_Ex(10);	
		iTimes--;
	}while(iTimes > 0);

	//printf("****Lock FFFFFFFFFFFailed => %d!!!\n", iTimes);
	return FALSE;
}

void Unlock_SqlDB_FileLocker(void)
{
	if(g_fd_locker_LogIndexFile <= 0)
	{
		close(g_fd_locker_LogIndexFile);
		return;
	}

	flock(g_fd_locker_LogIndexFile, LOCK_UN);
	//do not close the file here
	// if(close(g_fd_locker_LogIndexFile) == 0)
	// {
	// 	g_fd_locker_LogIndexFile = -1;
	// }
}


void Exit_LogRepo(void)
{
	//DESTROY_SQLITE_DB_SYNC();
	close(g_fd_locker_LogIndexFile);
}

static int Get_SQlMaxEventIdx(void* pInternalArg, int nColomn, char** ppszColumData, char** ppszColumnName)
{
	if((pInternalArg == NULL) || (ppszColumData == NULL))
	{
		return 1;
	}
	UNUSED(ppszColumnName);
	if(nColomn < 1)
	{
		return 1;
	}
	else if(ppszColumData[0] == NULL)
	{
		return 1;
	}
	//TRACEX("nColoum = %d, ppszColumData = %d\n", nColomn, ppszColumData);
	//Jimmy: do not add 1, but add it in main logic
	*(int*)pInternalArg = atoi(ppszColumData[0]); // + 1;

	return 0;
}

static void Set_LogIdx_ToVar(void)
{
	//FILE* pFile = fopen(VAR_SET_LOGIDX_FILENAME, "w+");
	//if(pFile != NULL)
	if(g_fd_locker_LogIndexFile > 0)
	{
		//char szLogIdx[16];
		//snprintf(szLogIdx, 16, "%d\n", s_nCurrLogIdx);
		//fputs(szLogIdx, pFile);
		//fclose(pFile);
		VAR_VALUE varV;
		varV.ulValue = s_nCurrLogIdx;
		lseek(g_fd_locker_LogIndexFile, 0, SEEK_SET);
		//Jimmy note: if failed to write: usually means file was deleted by other proc
		if(write(g_fd_locker_LogIndexFile, varV.abyValue, 4) < 4)
		{
			close(g_fd_locker_LogIndexFile);
			g_fd_locker_LogIndexFile = -1;
		}
		//printf("----[proc%d]: set logIdx => %d----\n", getpid(), s_nCurrLogIdx);
	}
	// else
	// {
	// 	printf("----[proc%d]: set logIdx FFFFFFFFFFFailed => %d----\n", getpid(), s_nCurrLogIdx);
	// }
}

static int Get_LogIdx_FromVar(void)
{
	//FILE* pFile = fopen(VAR_SET_LOGIDX_FILENAME, "r");
	//if(pFile != NULL)
	if(g_fd_locker_LogIndexFile > 0)
	{
		//char szLogIdx[16];		
		//if(fgets(szLogIdx, 16, pFile) != NULL)
		//{
			//int nLoggIdx = atoi(szLogIdx);
			VAR_VALUE varV;
			varV.ulValue = 0;
			lseek(g_fd_locker_LogIndexFile, 0, SEEK_SET);
			read(g_fd_locker_LogIndexFile, varV.abyValue, 4);
			//printf("---[proc%d]: Get Idx From VAR =>[%d, %d]---\n", getpid(), varV.ulValue, s_nCurrLogIdx);
			if((varV.ulValue > 0) && (varV.ulValue <= APP_LOG_SIZE))
			{
				s_nCurrLogIdx = varV.ulValue;	
			}			
		//}
		//fclose(pFile);
	}
	// else
	// {
	// 	printf("----[proc%d]: Get Idx FFFFFFFFailed => %d----\n", getpid(), s_nCurrLogIdx);
	// }

	return s_nCurrLogIdx;
}

BOOL Init_LogRepo(HANDLE hRepo)
{
	//printf("---------PID[%d], [%.3f]: Now BEGIN wait other DB OPerations--------\n", getpid(), GetCurrentTime());
	//!!!!!!!Jimmy note 20210730: found that if rec is 30000, the select would cost ~2.5s.!!!!!!!!!!!
	if(!Lock_SqlDB_FileLocker(10000)) //Jimmy changed 3s => 10s for there is chance for timeout when last SQL cost too much time
	{
		printf("---------Lock sqlDB fileLocker FAILED--------\n");
		Unlock_SqlDB_FileLocker();
		return FALSE;
	}

	BOOL bAlreadyGot_AppLogIdx = FALSE;
	Get_LogIdx_FromVar();
	if(s_nCurrLogIdx > 1)
	{
		bAlreadyGot_AppLogIdx = TRUE;
	}

	if(hRepo == NULL)
	{
		//added by Jimmy: if already get Idx from varFile, then no need read back from DB.
		// Get_LogIdx_FromVar();
		// if(s_nCurrLogIdx > 1)
		if(bAlreadyGot_AppLogIdx)
		{
			Unlock_SqlDB_FileLocker();
			s_bLogRepoInitOK = TRUE;
			//printf("---------PID[%d], [%.3f]: Now CCCANCEL wait other DB OPerations--------\n", getpid(), GetCurrentTime());
			return TRUE;
		}

		char	szDBFile[256] = {0};
		MakeFullFileName(szDBFile, \
			getenv(ENV_VARIABLE_DATA_PATH), NULL, "nccu.data");

		int rc = sqlite3_open_v2(szDBFile, &s_LogDB, SQLITE_OPEN_READWRITE, NULL);
		if(rc == SQLITE_OK)
		{
			sqlite3_busy_timeout(s_LogDB, 20000); //set default BUSY_HANDLER for sql database
			// sqlite3_exec(db,"SELECT Log_Idx FROM Data_AppLog WHERE Log_Time IN(SELECT max(Log_Time) FROM Data_AppLog)",
			// 	Get_TotalLogRec, NULL, NULL);
		}
		else
		{
			sqlite3_close(s_LogDB);
			Unlock_SqlDB_FileLocker();
			return FALSE;
		}
	}
	else
	{
		s_LogDB = (sqlite3*)hRepo;
	}	
	
	s_bIsSqlIn_RuntimMode = FALSE;
	//LOCK_APP_LOG_INIT();
	//judge if table can be opened
	char* szErrMsg = NULL;
	int nTotalLogItem, nCurrIdx;
	char szSql[256];
	
	//according to official doc, 'create table if not exits...' statement will still return error but has no error msg
	int nErrCode = -1;
	if(!bAlreadyGot_AppLogIdx)
	{
		nErrCode = sqlite3_exec(s_LogDB,"CREATE TABLE IF NOT EXISTS Data_AppLog("
				"Log_Idx INTEGER PRIMARY KEY, "
				"Log_Type INTEGER,"
				"Log_Time DOUBLE,"
				"Log_Data BLOB NOT NULL)",
				NULL, NULL, &szErrMsg);
		if(szErrMsg !=NULL)
		{
			TRACE("Can not create Log table, reason = [%s].\n", szErrMsg);
			sqlite3_free(szErrMsg);
			if(hRepo == NULL)
			{
				sqlite3_close(s_LogDB);
			}
			Unlock_SqlDB_FileLocker();
			return FALSE;
		}

		if(nErrCode == SQLITE_OK) //if not OK, means create, so do not get idx
		{
			szErrMsg = NULL;
			nTotalLogItem = 0;
			nCurrIdx = -1;
			// nErrCode = sqlite3_exec(s_LogDB,"SELECT count(*) FROM Data_AppLog",
			// 	Get_TotalLogRec, NULL, &szErrMsg);
			nErrCode = sqlite3_exec(s_LogDB,"SELECT count(*) FROM Data_AppLog",
				Get_SQlMaxEventIdx, &nTotalLogItem, &szErrMsg);
			// snprintf(szSql, 256, "SELECT Log_Idx FROM Data_AppLog WHERE Log_Time "
			// 	"IN(SELECT max(Log_Time) FROM Data_AppLog WHERE Log_Time < %d)", _NOW_ + 5);
			snprintf(szSql, 256, "SELECT Log_Idx, max(Log_Time) FROM Data_AppLog WHERE Log_Time < %d", _NOW_ + 5);
			sqlite3_exec(s_LogDB, szSql, Get_SQlMaxEventIdx, &nCurrIdx, &szErrMsg);
			if(nErrCode != SQLITE_OK)
			{
				sqlite3_free(szErrMsg);
				if(hRepo == NULL)
				{
					sqlite3_close(s_LogDB);
				}
				Unlock_SqlDB_FileLocker();
				return FALSE;
			}
			//printf("---[%.3f]: the get Data_AppLog.Idx =>%d,%d---\n", GetCurrentTime(), nCurrIdx, s_nCurrLogIdx);
			// else if(s_nCurrLogIdx > APP_LOG_SIZE)
			// {
			// 	nErrCode = sqlite3_exec(s_LogDB,"SELECT Log_Idx FROM Data_AppLog WHERE Log_Time IN(SELECT max(Log_Time) FROM Data_AppLog)",
			// 		Get_TotalLogRec, NULL, &szErrMsg);
			// 	if(nErrCode != SQLITE_OK)
			// 	{
			// 		sqlite3_free(szErrMsg);
			// 		return FALSE;
			// 	}
			// }
			Get_LogIdx_FromVar(); //also get logIdx from VarFile if existed
			//if(nCurrIdx >= 0)
			if((s_nCurrLogIdx == 1) && (nCurrIdx >= 1))
			{
				//printf("-----the fetched s_nCurrLogIdx => %d, %d----\n", s_nCurrLogIdx, nCurrIdx + 1);
				s_nCurrLogIdx = MAX(s_nCurrLogIdx, nCurrIdx + 1);
			}		
			//do not judge the TotalLogItem
			if(s_nCurrLogIdx > APP_LOG_SIZE)
			{
				s_nCurrLogIdx = 1;
			}		

			Set_LogIdx_ToVar();
			//printf("---the total logItem=>%d, currIdx=>%d---\n", nTotalLogItem, nCurrIdx);
		}
		TRACE("The current logIdx = %d.\n", s_nCurrLogIdx);
	}

	if(hRepo != NULL) //Jimmy changed 20210730 only needs to be gotten for MAIN-APP
	{
		//Jimmy added 20190225 for Data dumping
		nErrCode = sqlite3_exec(s_LogDB,"CREATE TABLE IF NOT EXISTS Data_DumpLog("
			"Dump_Idx INTEGER PRIMARY KEY, "
			"Dump_Src INTEGER,"
			"Dump_Type INTEGER,"
			"Dump_Time DOUBLE,"
			"Dump_Data STRING)",
			NULL, NULL, &szErrMsg);
		if(szErrMsg !=NULL)
		{
			TRACE("Can not create DataDump table, reason = [%s].\n", szErrMsg);
			sqlite3_free(szErrMsg);
			if(hRepo == NULL)
			{
				sqlite3_close(s_LogDB);
			}
			Unlock_SqlDB_FileLocker();
			return FALSE;
		}

		if(nErrCode == SQLITE_OK) //if not OK, means create, so do not get idx
		{
			szErrMsg = NULL;
			nTotalLogItem = 0;
			nCurrIdx = -1;
			// nErrCode = sqlite3_exec(s_LogDB,"SELECT count(*) FROM Data_DumpLog",
			// 	Get_TotalLogRec, NULL, &szErrMsg);
			nErrCode = sqlite3_exec(s_LogDB,"SELECT count(*) FROM Data_DumpLog",
				Get_SQlMaxEventIdx, &nTotalLogItem, &szErrMsg);
			// snprintf(szSql, 256, "SELECT Dump_Idx FROM Data_DumpLog WHERE Dump_Time "
			// 	"IN(SELECT max(Dump_Time) FROM Data_DumpLog WHERE Dump_Time < %d)", _NOW_);
			snprintf(szSql, 256, "SELECT Dump_Idx, max(Dump_Time) FROM Data_DumpLog WHERE Dump_Time < %d", _NOW_);
			sqlite3_exec(s_LogDB, szSql, Get_SQlMaxEventIdx, &nCurrIdx, &szErrMsg);
			if(nErrCode != SQLITE_OK)
			{
				sqlite3_free(szErrMsg);
				if(hRepo == NULL)
				{
					sqlite3_close(s_LogDB);
				}
				Unlock_SqlDB_FileLocker();
				return FALSE;
			}
			//printf("---[%.3f]: the get Data_DumpLog.Idx =>%d,%d---\n", GetCurrentTime(), nCurrIdx, s_nCurrDumpIdx);
			// else if(s_nCurrDumpIdx > APP_DATADUMP_SIZE)
			// {
			// 	nErrCode = sqlite3_exec(s_LogDB,"SELECT Dump_Idx FROM Data_DumpLog WHERE Dump_Time IN(SELECT max(Dump_Time) FROM Data_DumpLog)",
			// 		Get_TotalDumpRec, NULL, &szErrMsg);
			// 	if(nErrCode != SQLITE_OK)
			// 	{
			// 		sqlite3_free(szErrMsg);
			// 		return FALSE;
			// 	}
			// }
			if(nCurrIdx >= 0)
			{
				s_nCurrDumpIdx = nCurrIdx + 1;
			}

			//do not judge the TotalLogItem
			if(s_nCurrDumpIdx > APP_DATADUMP_SIZE)
			{
				s_nCurrDumpIdx = 1;
			}

			//printf("---the total dumpItem=>%d, currIdx=>%d---\n", nTotalLogItem, nCurrIdx);
		}
		TRACE("The current dumpIdx = %d.\n", s_nCurrDumpIdx);		
	}

	sqlite3_free(szErrMsg);
	s_bLogRepoInitOK = TRUE;
	if(hRepo == NULL)
	{
		sqlite3_close(s_LogDB);
	}

	//printf("---------PID[%d], [%.3f]: Now END wait other DB OPerations--------\n", getpid(), GetCurrentTime());
	
	Unlock_SqlDB_FileLocker();
	return TRUE;
}

void Set_SqliteDB_2RunTime_Mode()
{
	// LOCK_APP_LOG_SYNC();
	// s_LogDB = (sqlite3*)hNewRepo;
	// UNLOCK_APP_LOG_SYNC();
	LOCK_SQLITE_DB_SYNC(20000);
	sqlite3_close_v2(s_LogDB);
	s_LogDB = NULL;
	s_bIsSqlIn_RuntimMode = TRUE;
	UNLOCK_SQLITE_DB_SYNC();
}

//Jimmy note: need use sys_call to use gettid
// #include <sys/syscall.h>
// pid_t gettid()
// {
// 	return syscall(SYS_gettid);
// }

//Jimmy optimized here to avoid deadlock for multi-thread
static pthread_t s_hLastLockedThread = 0; //0 means not locked by any thead
//static UINT s_nLastLockedThreadId = 0;
static BOOL s_bInRecrusive = FALSE;
static BOOL s_bIsSelLock_Locked = FALSE;
sqlite3* Open_Lock_SqliteDB()
{
	// pthread_t hSelf = pthread_self();
	// if((s_hLastLockedThread == hSelf) && (s_LogDB != NULL)) //it is safe if same thrad use the same DB
	// {
	// 	TRACE("------------Same thread[%lu], so do not need LockDB-------\n", hSelf);
	// 	return s_LogDB;
	// }

	//UINT nSelfThreadId = (UINT)gettid();
	//if((s_nLastLockedThreadId == nSelfThreadId) && (s_LogDB != NULL))

	pthread_t hSelf = pthread_self();
	if((pthread_equal(s_hLastLockedThread, hSelf)) && (s_LogDB != NULL)) //it is safe if same thrad use the same DB
	{
		//printf("---------now lock the same DB for thread[%lu]---------\n", hSelf);
		s_bInRecrusive = TRUE;
		Lock_SqlDB_FileLocker(-1); //means no lock
		return s_LogDB;
	}

	if(!Lock_SqlDB_FileLocker(2000))
	{
		//do NOT unlock since the SetLogId will use the fd
		//Unlock_SqlDB_FileLocker();
	}
	
	//printf("------------Now TRRRRRRRRRRRRy LockDB for thread[%lu -> %lu]-------\n", s_hLastLockedThread, hSelf);

	sqlite3* db = NULL; //= (sqlite3*)pCharger->hRunDB;

	//use static instead so do not need initilize it everytime 20210219
	static char	szDBFile[256] = {0};
	if(szDBFile[0] == 0)
	{
		MakeFullFileName(szDBFile, \
			getenv(ENV_VARIABLE_DATA_PATH), NULL, "nccu.data");		
	}

	//Jimmy revised it here 20230223 just to see if there is any differences
	if(LOCK_SQLITE_DB_SYNC(2000))
	{
		s_bIsSelLock_Locked = TRUE;
	}

	s_bInRecrusive = FALSE;

	//printf("------------Now LockDB for thread[%lu -> %lu]-------\n", s_hLastLockedThread, hSelf);

	//addded by Jimmy 20210218
	if(s_LogDB != NULL)
	{
		//changed by Jimmy 20230322 for sqlite3_close_v2 is easy to cause crash and needs to leav the DB with existing handle to be able to continue
		//sqlite3_close_v2(s_LogDB);
		//s_LogDB = NULL;	
		if(sqlite3_close(s_LogDB) == SQLITE_BUSY)
		{
			LogOut_A("LOG-SELF", "EXCEPTION", LOG_TYPE_ERROR, "[Open_Lock_SqliteDB]: DB close busy, force ROLLBACK and close!");
			sqlite3_exec(s_LogDB, "ROLLBACK;", NULL, NULL, NULL);
			sqlite3_close(s_LogDB);
		}
		s_LogDB = NULL;
	}

	int rc = sqlite3_open_v2(szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
	if(rc == SQLITE_OK)
	{
		sqlite3_busy_timeout(db, 20000); //set default BUSY_HANDLER for sql database
		s_LogDB = db;
	}
	else
	{
		db = NULL; //shall not be happening
	}

	s_hLastLockedThread = hSelf;

	return db;
}

void Close_UnLock_SqliteDB()
{
	Unlock_SqlDB_FileLocker();
	//sqlite3_close(s_LogDB);
	if(!s_bInRecrusive)
	{
		//printf("----Now unlock the DB for thread[%lu]----\n", pthread_self());
		s_hLastLockedThread = 0;
		if(s_LogDB != NULL)
		{
			//sqlite3_close_v2(s_LogDB);
			//s_LogDB = NULL;		
			int nRet = sqlite3_close(s_LogDB);
			if(nRet == SQLITE_OK)
			{
				s_LogDB = NULL;				
			}	
			else
			{
				LogOut_A("LOG-SELF", "EXCEPTION", LOG_TYPE_ERROR, "[Close_UnLock_SqliteDB]: DB close NOK -> [%d]!!!", nRet);
			}
		}
	}
	else
	{
		//printf("----Now SKKKKKKKip the unlock of the DB for thread[%lu]----\n", pthread_self());
		//need set recrusive => false in order the DB to be closed after recrusive done
		s_bInRecrusive = FALSE;		
		return; //no need unlock since it is not locked
	}

	if(s_bIsSelLock_Locked)
	{
		UNLOCK_SQLITE_DB_SYNC();
		s_bIsSelLock_Locked = FALSE;
	}	
}

// void LogOut_A(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
// {
// 	va_list arglist;
// 	char szBuf[512] = {0};
// 	char szThreadName[64] = {0};

// 	char szHeader[64] = {0};
// 	snprintf(szHeader, 64, "%s\t%s\t[%s]\t", szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64));

// 	//strcat(szBuf, RunThread_GetName(NULL, szThreadName, 32));
// 	//TRACE("szTHread = %s.\n", szThreadName);
// 	char szContent[512] = {0};
// 	va_start(arglist, pszFormat);
// 	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
// 	vsnprintf(szContent, 512, pszFormat, arglist);
// 	va_end(arglist);

// 	snprintf(szBuf, 512, "%s%s", szHeader, szContent);
// 	TRACE("------The Log_A Content[%s], and s_LogDB = %d---\n", szBuf, s_LogDB);

// 	//Jimmy observed20190105: if some log time is higher than right now, then the log will be overwritten, so comment this line
// 	// static int s_nLastWriteRecPos = 0;
// 	// sqlite3_exec(s_LogDB,"SELECT Log_Idx FROM Data_AppLog WHERE Log_Time IN(SELECT max(Log_Time) FROM Data_AppLog)",
// 	// 	Get_TotalLogRec, NULL, NULL);

// 	// if(s_nCurrLogIdx <= s_nLastWriteRecPos)
// 	// {
// 	// 	s_nCurrLogIdx = s_nLastWriteRecPos + 1;
// 	// }

// 	//replace means it exist then update, or else insert
// 	sqlite3_stmt * stat;
// 	sqlite3_prepare_v2(s_LogDB,"replace into Data_AppLog values (?,?,?,?);",-1,&stat,NULL);
// 	sqlite3_bind_int(stat,1, /*(int)s_nCurrLogIdx*/ Get_LogIdx_FromVar());
// 	sqlite3_bind_int(stat,2, emLogType);
// 	sqlite3_bind_double(stat,3, GetCurrentTime());
// 	sqlite3_bind_blob(stat, 4, (void*)szBuf, strlen(szBuf) + 1, NULL);
// 	int nSqlRet = sqlite3_step(stat);
// 	sqlite3_finalize(stat);//delete stat

// 	// s_nLastWriteRecPos = s_nCurrLogIdx;
// 	// if(s_nLastWriteRecPos >= APP_LOG_SIZE)
// 	// {
// 	// 	s_nLastWriteRecPos = 0;
// 	// }
// 	Lock_SqlDB_FileLocker(1000);	

// 	TRACE("[Log_A %d]: %s\n", s_nCurrLogIdx, szBuf);
// 	if(s_nCurrLogIdx < APP_LOG_SIZE)
// 	{
// 		s_nCurrLogIdx++;
// 	}
// 	else
// 	{
// 		s_nCurrLogIdx = 1;
// 	}

// 	Set_LogIdx_ToVar();	

// 	Unlock_SqlDB_FileLocker();
	
// 	if(nSqlRet != SQLITE_DONE)
// 	{
// 		TRACEX("---Log_A: Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_AppLog values (?,?,?,?);", 
// 			nSqlRet, sqlite3_errmsg(s_LogDB));
// 		//system("reboot");
// 	}
// }

// BOOL LogOut(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
// {
// #ifdef TRACE_2DBLOG
// 	#define BUFF_SIZE_LOGOUT (1024)
// #else
// 	#define BUFF_SIZE_LOGOUT (512)
// #endif
// 	if(!s_bLogRepoInitOK)
// 	{
// 		return FALSE;
// 	}

// 	va_list arglist;
// 	char szBuf[BUFF_SIZE_LOGOUT] = {0};
// 	//char szThreadName[32] = {0};
// 	char szThreadName[64] = {0};
	
// 	char szHeader[64];
// #ifdef TRACE_2DBLOG
// 	struct timeval  tv;
// 	gettimeofday(&tv, NULL);
// 	struct tm *timenow;
// 	timenow = localtime( &tv.tv_sec );
// 	snprintf(szHeader, 64, "%04d-%02d-%02d %02d:%02d:%02d:%03d %s\t%s\t[%s]\t",
// 		timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec, (int)tv.tv_usec/1000,
// 		szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64));
// #else
// 	snprintf(szHeader, 64, "%s\t%s\t[%s]\t", szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64));
// #endif

// 	//strcat(szBuf, RunThread_GetName(NULL, szThreadName, 32));
// 	//TRACE("szTHread = %s.\n", szThreadName);

// 	char szContent[BUFF_SIZE_LOGOUT];
// 	va_start(arglist, pszFormat);
// 	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
// 	vsnprintf(szContent, BUFF_SIZE_LOGOUT, pszFormat, arglist);
// 	va_end(arglist);

// 	snprintf(szBuf, BUFF_SIZE_LOGOUT, "%s%s", szHeader, szContent);	
// 	//Jimmy observed20190105: if some log time is higher than right now, then the log will be overwritten, so comment this line
// 	// static int s_nLastWriteRecPos = 0;
// 	// sqlite3_exec(s_LogDB,"SELECT Log_Idx FROM Data_AppLog WHERE Log_Time IN(SELECT max(Log_Time) FROM Data_AppLog)",
// 	// 	Get_TotalLogRec, NULL, NULL);

// 	// if(s_nCurrLogIdx <= s_nLastWriteRecPos)
// 	// {
// 	// 	s_nCurrLogIdx = s_nLastWriteRecPos + 1;
// 	// }

// 	//Lock
// 	if(s_bIsSqlIn_RuntimMode)
// 	{
// 		Open_Lock_SqliteDB();
// 	}
// 	else
// 	{
// 		Lock_SqlDB_FileLocker(1000);	
// 	}

// 	TRACEX("------The Log_Out Content[%s], and s_LogDB = %lu---\n", szBuf, s_LogDB);
// 	//replace means it exist then update, or else insert
// 	sqlite3_stmt * stat;
// 	//BYTE* pOut = NULL;
// 	sqlite3_prepare_v2(s_LogDB,"replace into Data_AppLog values (?,?,?,?);",-1,&stat,NULL);
// 	sqlite3_bind_int(stat,1, /*(int)s_nCurrLogIdx*/ Get_LogIdx_FromVar());
// 	sqlite3_bind_int(stat,2, emLogType);
// 	sqlite3_bind_double(stat,3, GetCurrentTime());
// 	sqlite3_bind_blob(stat, 4, (void*)szBuf, strlen(szBuf) + 1, NULL);
// 	//sqlite3_bind_blob_compress(stat, 4, szBuf, strlen(szBuf) + 1, pOut, NULL);
// 	int nSqlRet = sqlite3_step(stat);
// 	sqlite3_finalize(stat);//delete stat
// 	//sqlite3_finalize_compressed(stat, pOut);//delete stat
	
// 	// s_nLastWriteRecPos = s_nCurrLogIdx;
// 	// if(s_nLastWriteRecPos >= APP_LOG_SIZE)
// 	// {
// 	// 	s_nLastWriteRecPos = 0;
// 	// }
// 	TRACE("[Log %d]: %s\n", s_nCurrLogIdx, szBuf);
// 	if(s_nCurrLogIdx < APP_LOG_SIZE)
// 	{
// 		s_nCurrLogIdx++;
// 	}
// 	else
// 	{
// 		s_nCurrLogIdx = 1;
// 	}

// 	Set_LogIdx_ToVar();	
	
// 	if(nSqlRet != SQLITE_DONE)
// 	{
// 		TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_AppLog values (?,?,?,?);", 
// 			nSqlRet, sqlite3_errmsg(s_LogDB));
// 		//system("reboot");
// 	}

// 	//unlock
// 	if(s_bIsSqlIn_RuntimMode)
// 	{
// 		Close_UnLock_SqliteDB();
// 	}
// 	else
// 	{
// 		Unlock_SqlDB_FileLocker();
// 	}

// 	return TRUE;
// }

// BOOL DumpOut(const IN int nDumpSrc, IN DUMP_TYPE emDumpType, IN char *szDumpData)
// {
// 	char szBuf[MAX_DUM_DATASIZE] = {0};
// 	//char szThreadName[32] = {0};
// 	if (!s_bLogRepoInitOK)
// 	{
// 		return FALSE;
// 	}

// 	//Lock
// 	if(s_bIsSqlIn_RuntimMode)
// 	{
// 		Open_Lock_SqliteDB();
// 	}

// 	snprintf(szBuf, MAX_DUM_DATASIZE, "%s", szDumpData);

// 	//Jimmy observed20190105: if some log time is higher than right now, then the log will be overwritten, so comment this line
// 	// static int s_nLastWriteRecPos = 0;
// 	// sqlite3_exec(s_LogDB,"SELECT Dump_Idx FROM Data_DumpLog WHERE Dump_Time IN(SELECT max(Dump_Time) FROM Data_DumpLog)",
// 	// 	Get_TotalDumpRec, NULL, NULL);

// 	// if(s_nCurrDumpIdx <= s_nLastWriteRecPos)
// 	// {
// 	// 	s_nCurrDumpIdx = s_nLastWriteRecPos + 1;
// 	// }

// 	//replace means it exist then update, or else insert
// 	sqlite3_stmt * stat;
// 	sqlite3_prepare_v2(s_LogDB,"replace into Data_DumpLog values (?,?,?,?,?);",-1,&stat,NULL);
// 	sqlite3_bind_int(stat,1, (int)s_nCurrDumpIdx);
// 	sqlite3_bind_int(stat,2, nDumpSrc);
// 	sqlite3_bind_int(stat,3, emDumpType);
// 	sqlite3_bind_double(stat,4, GetCurrentTime());
// 	sqlite3_bind_text(stat, 5, szBuf, strlen(szBuf), NULL);
// 	int nSqlRet = sqlite3_step(stat);
// 	sqlite3_finalize(stat);//delete stat

// 	// s_nLastWriteRecPos = s_nCurrDumpIdx;
// 	// if(s_nLastWriteRecPos >= APP_DATADUMP_SIZE)
// 	// {
// 	// 	s_nLastWriteRecPos = 0;
// 	// }
// 	if(s_nCurrDumpIdx < APP_DATADUMP_SIZE)
// 	{
// 		s_nCurrDumpIdx++;
// 	}
// 	else
// 	{
// 		s_nCurrDumpIdx = 1;
// 	}

// 	//unlock
// 	if(s_bIsSqlIn_RuntimMode)
// 	{
// 		Close_UnLock_SqliteDB();
// 	}

// 	TRACE("[Dump %d]: %s\n", s_nCurrDumpIdx, szBuf);

// 	if(nSqlRet != SQLITE_DONE)
// 	{
// 		TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_DumpLog values (?,?,?,?);", 
// 			nSqlRet, sqlite3_errmsg(s_LogDB));
// 		//system("reboot");
// 	}

// 	return TRUE;
// }

// void LogOut_Ex(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
// {
// 	if(!s_bLogRepoInitOK)
// 	{
// 		//BOOL bOK = Init_LogRepo(NULL);
// 		//printf("----NNNNOw ext-proc initRepo => %s----\n", bOK ? "SUCCESSFULLY!!!" : "FAILED!!!");
// 		Init_LogRepo(NULL);
// 	}

// 	va_list arglist;
// 	char szBuf[512] = {0};
// 	//char szThreadName[64] = {0};

// 	char szHeader[64] = {0};
// 	snprintf(szHeader, 64, "%s\t%s\t[%s]\t", szModualName, szSubModualName, "N/A" /*RunThread_GetName(NULL, szThreadName, 64)*/);

// 	//strcat(szBuf, RunThread_GetName(NULL, szThreadName, 32));
// 	//TRACE("szTHread = %s.\n", szThreadName);
// 	char szContent[512] = {0};
// 	va_start(arglist, pszFormat);
// 	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
// 	vsnprintf(szContent, 512, pszFormat, arglist);
// 	va_end(arglist);

// 	snprintf(szBuf, 512, "%s%s", szHeader, szContent);
// 	TRACE("------The Log_Ex Content[%s]---\n", szBuf);

// 	sqlite3* db = NULL; //= (sqlite3*)pCharger->hRunDB;

// 	char	szDBFile[256] = {0};
// 	//added by Jimmy 20201211: if env not provided, should use default PATH
// 	char *szDataRoot = getenv(ENV_VARIABLE_DATA_PATH);
// 	if(szDataRoot != NULL)
// 	{
// 		MakeFullFileName(szDBFile, szDataRoot, NULL, "nccu.data");
// 	}
// 	else
// 	{
// 		MakeFullFileName(szDBFile, "/data", NULL, "nccu.data");
// 	}	

// 	if(!Lock_SqlDB_FileLocker(2000))
// 	{
// 		//printf("---------Lock sqlDB fileLocker FAILED--------\n");
// 		Unlock_SqlDB_FileLocker();
// 		return;
// 	}

// 	int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
// 	if(rc == SQLITE_OK)
// 	{
// 		sqlite3_busy_timeout(db, 20000); //set default BUSY_HANDLER for sql database
// 		// sqlite3_exec(db,"SELECT Log_Idx FROM Data_AppLog WHERE Log_Time IN(SELECT max(Log_Time) FROM Data_AppLog)",
// 		// 	Get_TotalLogRec, NULL, NULL);
// 	}
// 	else
// 	{
// 		sqlite3_close(db);
// 		Unlock_SqlDB_FileLocker();
// 		return;
// 	}

// 	//Jimmy observed20190105: if some log time is higher than right now, then the log will be overwritten, so comment this line
// 	// static int s_nLastWriteRecPos = 0;
// 	// if(s_nCurrLogIdx >= APP_LOG_SIZE)
// 	// {
// 	// 	s_nCurrLogIdx = 1;
// 	// }
// 	// else if(s_nCurrLogIdx <= s_nLastWriteRecPos)
// 	// {
// 	// 	s_nCurrLogIdx = s_nLastWriteRecPos + 1;
// 	// }

// 	//replace means it exist then update, or else insert
// 	sqlite3_stmt * stat;
// 	sqlite3_prepare_v2(db,"replace into Data_AppLog values (?,?,?,?);",-1,&stat,NULL);
// 	sqlite3_bind_int(stat,1, /*(int)s_nCurrLogIdx*/Get_LogIdx_FromVar());
// 	sqlite3_bind_int(stat,2, emLogType);
// 	sqlite3_bind_double(stat,3, GetCurrentTime());
// 	sqlite3_bind_blob(stat, 4, (void*)szBuf, strlen(szBuf) + 1, NULL);
// 	int nSqlRet = sqlite3_step(stat);
// 	sqlite3_finalize(stat);//delete stat

// 	sqlite3_close_v2(db);

// 	//s_nLastWriteRecPos = s_nCurrLogIdx;

// 	//printf("[LogEx %d]: %s\n", s_nCurrLogIdx, szBuf);
// 	if(s_nCurrLogIdx < APP_LOG_SIZE)
// 	{
// 		s_nCurrLogIdx++;
// 	}
// 	else
// 	{
// 		s_nCurrLogIdx = 1;
// 	}

// 	Set_LogIdx_ToVar();	

// 	Unlock_SqlDB_FileLocker();
// }


BOOL LogOut4c(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
{
	static const int BUFF_SIZE_LogOut4c = 2048;

	char szThreadName[64] = {0};
	char szContent[BUFF_SIZE_LogOut4c];

	va_list arglist;
	va_start(arglist, pszFormat);
	vsnprintf(szContent, BUFF_SIZE_LogOut4c, pszFormat, arglist);
	va_end(arglist);

	LOGFD("LogOut4c:%d-%s\t%s\t[%s]\t%s", emLogType, szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64), szContent);

	return TRUE;
}

// static char* MakeLogType(IN int nLogType, OUT char* szLogTp)
// {
// 	switch (nLogType)
// 	{
// 	case LOG_TYPE_INFO:
// 		sprintf(szLogTp, "%s", "INFO");
// 		break;
// 	case LOG_TYPE_WARING:
// 		sprintf(szLogTp, "%s", "WARN");
// 		break;
// 	case LOG_TYPE_ERROR:
// 		sprintf(szLogTp, "%s", "ERROR");
// 		break;
// 	case LOG_TYPE_MILESTONE:
// 		sprintf(szLogTp, "%s", "MILESTONE");
// 		break;
// 	default:
// 		sprintf(szLogTp, "%s", "UNKNOWN");
// 		break;
// 	}
// 	return szLogTp;
// }

// static char *MakeDumpType(IN int nDumpType, OUT char *szDumpTp)
// {
// 	switch (nDumpType)
// 	{
// 	case DUMP_TYPE_OPEN_COMM:
// 		sprintf(szDumpTp, "%s", "OPEN_COMM");
// 		break;
// 	case DUMP_TYPE_CLOSE_COMM:
// 		sprintf(szDumpTp, "%s", "CLOSE_COMM");
// 		break;
// 	case DUMP_TYPE_INFO:
// 		sprintf(szDumpTp, "%s", "INFO");
// 		break;
// 	case DUMP_TYPE_READ:
// 		sprintf(szDumpTp, "%s", "READ");
// 		break;
// 	case DUMP_TYPE_WRITE:
// 		sprintf(szDumpTp, "%s", "WRITE");
// 		break;
// 	default:
// 		sprintf(szDumpTp, "%s", "UNKNOWN");
// 		break;
// 	}
// 	return szDumpTp;
// }

// int LogCat(char* szFile, BOOL bInternal)
// {
// 	// write head msg
// 	FILE *fp = fopen( szFile, "w");
// 	if (fp == NULL)
// 	{
// 		return -1;
// 	}

// 	sqlite3_stmt * stat;
// 	sqlite3* db;
// 	if(bInternal)
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			db = Open_Lock_Database(DB_TYPE_APPLOG);
// 		}
// 	}
// 	else
// 	{
// 		char szDBFile[256] = {0};
// 		MakeFullFileName(szDBFile, \
// 			getenv("NCCU_DATA_PATH"), NULL, "AppLog.data");
// 		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READONLY, NULL);
// 		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
// 		if(rc != SQLITE_OK)
// 		{
// 			fclose(fp);
// 			return -1;
// 		}

// 		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
// 	}

// 	sqlite3_prepare_v2(db,"SELECT * FROM Data_AppLog ORDER BY Log_Time ASC",-1,&stat,NULL);
// 	int nLogIdx = 1;
// 	char szTime[32];
// 	char szLogType[16];
// 	int nLogTotal = 0;
// 	//char szLogContent[256] = {0};

// 	while (sqlite3_step(stat) == SQLITE_ROW)
// 	{
// 		nLogIdx = sqlite3_column_int(stat, 0);
// 		int nLogType = sqlite3_column_int(stat, 1);
// 		double fLogTime = sqlite3_column_double(stat, 2);
// 		AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32);
// 		const UCHAR *szLog = sqlite3_column_text(stat, 3);
// 		//sqlite3_parse_blob_uncompressed(stat, 3, (void*)szLogContent, 256);
// 		fprintf(fp, "[%04d]: %d\t%s\t%s\t%s\n", nLogTotal + 1, nLogIdx, MakeLogType(nLogType, szLogType),
// 			szTime, szLog /*szLogContent*/);
// 		nLogTotal++;
// 	}

// 	sqlite3_finalize(stat);
// 	fclose(fp);

// 	if(!bInternal)
// 	{
// 		sqlite3_close_v2(db);
// 	}
// 	else
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			Close_UnLock_Database(DB_TYPE_APPLOG);
// 		}
// 	}

// 	return nLogTotal;
// }

// void Log_Delete(BOOL bInternal, int nLogDeleteType)
// {
// 	sqlite3* db;
// 	if(bInternal)
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			db = Open_Lock_SqliteDB();
// 		}
// 	}
// 	else
// 	{
// 		Lock_SqlDB_FileLocker(3000);
// 		char szDBFile[256] = {0};
// 		MakeFullFileName(szDBFile, \
// 			getenv("NCCU_DATA_PATH"), NULL, "nccu.data");
// 		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
// 		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
// 		if(rc != SQLITE_OK)
// 		{
// 			Unlock_SqlDB_FileLocker();
// 			return;
// 		}

// 		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
// 	}

// 	char* szErrMsg = NULL;
// 	if(nLogDeleteType == LOG_DELTE_DUMPLOG)
// 	{
// 		sqlite3_exec(db,"DELETE FROM Data_DumpLog", NULL, NULL, &szErrMsg);
// 		s_nCurrDumpIdx = 1;
// 	}
// 	else if(nLogDeleteType == LOG_DELTE_MBELOG)
// 	{
// 		sqlite3_exec(db,"DELETE FROM Data_MBELog", NULL, NULL, &szErrMsg);
// 		s_nCurrMBELogIdx = 1;
// 	}
// 	else
// 	{
// 		sqlite3_exec(db,"DELETE FROM Data_AppLog", NULL, NULL, &szErrMsg);
// 		s_nCurrLogIdx = 1;
// 	}	

// 	if(szErrMsg != NULL)
// 	{
// 		sqlite3_free(szErrMsg); 
// 	}
// 	if(!bInternal)
// 	{
// 		sqlite3_close_v2(db);
// 	}
// 	else
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			Close_UnLock_SqliteDB();
// 		}
// 	}

// 	Unlock_SqlDB_FileLocker();
// }


// int LogDump(char* szFile, BOOL bInternal)
// {
// 	FILE *fp;
// 	fp = fopen(szFile, "w");
// 	if(fp == NULL)
// 	{
// 		return -1;
// 	}
// 	sqlite3_stmt * stat;
// 	sqlite3* db;
// 	if(bInternal)
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			db = Open_Lock_SqliteDB();
// 		}
// 	}
// 	else
// 	{
// 		char szDBFile[256] = {0};
// 		MakeFullFileName(szDBFile,\
// 			getenv("NCCU_DATA_PATH"), NULL, "nccu.data");
// 		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READONLY, NULL);
// 		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
// 		if(rc != SQLITE_OK)
// 		{
// 			fclose(fp);
// 			return -1;
// 		}

// 		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
// 	}

// 	sqlite3_prepare_v2(db,"select * from (select * from Data_DumpLog order by Dump_Time DESC limit 5000) order by Dump_Time ASC;",-1,&stat,NULL);
// 	int nLogIdx = 1;
// 	char szTime[32];
// 	char szDumpType[16];
// 	int nLogTotal = 0;

// 	while (sqlite3_step(stat) == SQLITE_ROW)
// 	{
// 		nLogIdx = sqlite3_column_int(stat, 0);
// 		int nDumpSrc = sqlite3_column_int(stat, 1);
// 		int nDumpType = sqlite3_column_int(stat, 2);
// 		double fLogTime = sqlite3_column_double(stat, 3);
// 		AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32);
// 		const UCHAR *szLog = sqlite3_column_text(stat, 4);
// 		fprintf(fp, "[%04d]: %d\t%s\t%s\t%s\t%s\n", nLogTotal + 1, nLogIdx, "Usr_Rept",MakeDumpType(nDumpType, szDumpType),
// 			szTime, szLog);
// 		nLogTotal++;
// 	}

// 	sqlite3_finalize(stat);
// 	fclose(fp);

// 	if(!bInternal)
// 	{
// 		sqlite3_close_v2(db);
// 	}
// 	else
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			Close_UnLock_SqliteDB();
// 		}
// 	}
// 	return nLogTotal;
// }

// #define MAX_MBELOG_TOTAL 500
// void LogOut_MBE(IN char *pszFormat, ...)
// {
// 	Open_Lock_SqliteDB();
// 	static BOOL s_bIsInitMBEDBOK = FALSE;
// 	static UINT s_nCurrMBELogIdx = 1;
// 	if(!s_bIsInitMBEDBOK)
// 	{
// 		char* szErrMsg = NULL;
// 		//according to official doc, 'create table if not exits...' statement will still return error but has no error msg
// 		int nErrCode = sqlite3_exec(s_LogDB,"CREATE TABLE IF NOT EXISTS Data_MBELog("
// 			"Log_Idx INTEGER PRIMARY KEY, "
// 			"Log_Time DOUBLE, "
// 			"Log_Data BLOB NOT NULL)",
// 			NULL, NULL, &szErrMsg);
// 		if(szErrMsg != NULL)
// 		{
// 			//printf("Can not create Log table, reason = [%s].\n", szErrMsg);
// 			sqlite3_free(szErrMsg);
// 			Close_UnLock_SqliteDB();
// 			return;
// 		}
// 		char szSql[256];
// 		int nGetIdx = -1;
// 		sqlite3_exec(s_LogDB,  "SELECT count(*) FROM Data_MBELog", Get_SQlMaxEventIdx, &nGetIdx, NULL);
// 		if(nGetIdx > MAX_MBELOG_TOTAL)
// 		{
// 			s_nCurrMBELogIdx = 1;
// 			//Jimmy added 20210902 to shorten the volome 5000 -> 1000
// 			sqlite3_exec(s_LogDB,  "DELETE from Data_MBELog", NULL, NULL, NULL);
// 		}
// 		else
// 		{
// 			nGetIdx = -1;
// 			// snprintf(szSql, 256, "SELECT Log_Idx FROM Data_MBELog WHERE Log_Time "
// 			// 	"IN(SELECT max(Log_Time) FROM Data_MBELog WHERE Log_Time < %d)", _NOW_ + 5);
// 			snprintf(szSql, 256, "SELECT Log_Idx, max(Log_Time) FROM Data_MBELog WHERE Log_Time < %d", _NOW_ + 5);
// 			sqlite3_exec(s_LogDB,  szSql, Get_SQlMaxEventIdx, &nGetIdx, NULL);
// 			if(nGetIdx > 0)
// 			{
// 				s_nCurrMBELogIdx = nGetIdx + 1;
// 			}			
// 		}

// 		//printf("------The MBELogIdx => %d-----\n", s_nCurrMBELogIdx);
// 		s_bIsInitMBEDBOK = TRUE;
// 	}

// 	char szBuf[4096] = {0};
// 	va_list arglist;
// 	va_start(arglist, pszFormat);
// 	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
// 	vsnprintf(szBuf, 4096, pszFormat, arglist);
// 	va_end(arglist);

// 	//replace means if exist then update, or else insert
// 	sqlite3_stmt * stat;
// 	sqlite3_prepare_v2(s_LogDB,"replace into Data_MBELog values (?,?,?);",-1,&stat,NULL);
// 	sqlite3_bind_int(stat, 1, s_nCurrMBELogIdx);
// 	sqlite3_bind_double(stat, 2, GetCurrentTime());
// 	sqlite3_bind_blob(stat, 3, (void*)szBuf, strlen(szBuf) + 1, NULL);
// 	int nSqlRet = sqlite3_step(stat);
// 	sqlite3_finalize(stat);//delete stat

// 	//---------for debug purpose-------
// 	//printf("\n%s\n", szBuf);
// 	//---------------------------------

// 	if(s_nCurrMBELogIdx < MAX_MBELOG_TOTAL)
// 	{
// 		s_nCurrMBELogIdx++;
// 	}
// 	else
// 	{
// 		s_nCurrMBELogIdx = 1;
// 	}

// 	Close_UnLock_SqliteDB();
// }

// int Log_Dump_MBE(char* szFile, BOOL bInternal)
// {
// 	FILE *fp;
// 	fp = fopen(szFile, "w");
// 	if(fp == NULL)
// 	{
// 		return -1;
// 	}
// 	sqlite3_stmt * stat;
// 	sqlite3* db;
// 	if(bInternal)
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			db = Open_Lock_SqliteDB();
// 		}
// 	}
// 	else
// 	{
// 		char szDBFile[256] = {0};
// 		MakeFullFileName(szDBFile,\
// 			getenv("NCCU_DATA_PATH"), NULL, "nccu.data");
// 		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READONLY, NULL);
// 		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
// 		if(rc != SQLITE_OK)
// 		{
// 			fclose(fp);
// 			return -1;
// 		}

// 		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
// 	}

// 	sqlite3_prepare_v2(db,"select * from Data_MBELog order by Log_Time ASC",-1,&stat,NULL);
// 	int nLogIdx = 1;
// 	char szTime[32];
// 	char szDumpType[16];
// 	int nLogTotal = 0;

// 	while (sqlite3_step(stat) == SQLITE_ROW)
// 	{
// 		nLogIdx = sqlite3_column_int(stat, 0);
// 		const UCHAR *szLog = sqlite3_column_text(stat, 2);
// 		fprintf(fp, "[%04d]: %d\t%s\n", nLogTotal+ 1, nLogIdx, szLog);
// 		nLogTotal++;
// 	}

// 	sqlite3_finalize(stat);
// 	fclose(fp);

// 	if(!bInternal)
// 	{
// 		sqlite3_close_v2(db);
// 	}
// 	else
// 	{
// 		if(s_bIsSqlIn_RuntimMode)
// 		{
// 			Close_UnLock_SqliteDB();
// 		}
// 	}
// 	return nLogTotal;
// }

#define MAX_BATTDEGRLOG_TOTAL 500
void LogOut_BattDegr(IN BATTDEGR_LOG_TYPE logType, IN const char *logDate, IN char *pszFormat, ...)
{
    Open_Lock_SqliteDB();

    static BOOL s_bIsInitBattDegrDBOK = FALSE;
    static UINT s_nBattDegrLogIdx = 1;

    if (!s_bIsInitBattDegrDBOK)
    {
        char* szErrMsg = NULL;

        int nErrCode = sqlite3_exec(s_LogDB, 
            "CREATE TABLE IF NOT EXISTS Data_BattDegrData("
            "Log_Idx INTEGER PRIMARY KEY, "
            "Log_Time DOUBLE, "
            "Log_Date TEXT NOT NULL, "
            "Log_Type INTEGER NOT NULL, "
            "Log_Data BLOB NOT NULL)",
            NULL, NULL, &szErrMsg);

        if (szErrMsg != NULL)
        {
            sqlite3_free(szErrMsg);
            Close_UnLock_SqliteDB();
            return;
        }

        char szSql[256];
        int nGetIdx = -1;

		
        sqlite3_exec(s_LogDB, "SELECT count(*) FROM Data_BattDegrData", Get_SQlMaxEventIdx, &nGetIdx, NULL);
		nGetIdx = -1;

		snprintf(szSql, 256, "SELECT Log_Idx, max(Log_Time) FROM Data_BattDegrData WHERE Log_Time < %d", _NOW_ + 5);
		sqlite3_exec(s_LogDB, szSql, Get_SQlMaxEventIdx, &nGetIdx, NULL);

		if (nGetIdx > 0)
		{
			s_nBattDegrLogIdx = nGetIdx + 1;
		}

        s_bIsInitBattDegrDBOK = TRUE;
    }

    char szBuf[4096] = {0};
    va_list arglist;
    va_start(arglist, pszFormat);
    vsnprintf(szBuf, 4096, pszFormat, arglist);
    va_end(arglist);

    sqlite3_stmt *stat;
    sqlite3_prepare_v2(s_LogDB, "REPLACE INTO Data_BattDegrData (Log_Idx, Log_Time, Log_Date, Log_Type, Log_Data) VALUES (?,?,?,?,?);", -1, &stat, NULL);
    sqlite3_bind_int(stat, 1, s_nBattDegrLogIdx);
    sqlite3_bind_double(stat, 2, GetCurrentTime());
    sqlite3_bind_text(stat, 3, logDate, -1, SQLITE_STATIC);
    sqlite3_bind_int(stat, 4, logType);
    sqlite3_bind_blob(stat, 5, (void*)szBuf, strlen(szBuf) + 1, NULL);
    int nSqlRet = sqlite3_step(stat);
    sqlite3_finalize(stat);

    if (s_nBattDegrLogIdx < MAX_BATTDEGRLOG_TOTAL)
    {
        s_nBattDegrLogIdx++;
    }
    else
    {
        s_nBattDegrLogIdx = 1;
    }

    Close_UnLock_SqliteDB();
}

