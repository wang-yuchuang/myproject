// #define _TRACE_PRINT_
#include <sys/types.h> 
#include <stdarg.h>
#include <pthread.h>
#include "std_sys.h"
#include "base_defines/iLog.h"
#include "base_defines/basefunc.h"
#include "app_incs.h"

#define LOCK_SQLITE_DATABASE_SYNC(Type,n)		Lock_SqlDB_NoBlock((Type),(n))
#define UNLOCK_SQLITE_DATABASE_SYNC(Type)		pthread_mutex_unlock(&g_DbInfoArray[(Type)].s_sqLiteDBLock)
#define DESTROY_SQLITE_DATABASE_SYNC(Type)	pthread_mutex_destroy(&g_DbInfoArray[(Type)].s_sqLiteDBLock)

static BOOL s_bIsInitDumpDBOK = FALSE;
static UINT s_nCurrDumpLogIdx = 1;

static BOOL s_bDbInitOK = FALSE;
static UINT s_nCurrAppLogIdx = 1;

#define MAX_MBELOG_TOTAL 500
static BOOL s_bIsInitMBEDBOK = FALSE;
static UINT s_nCurrMBELogIdx = 1;


/*********************************************************************************
*  CREATOR       : Wanlu
*  CREATED DATE  : 2026-1-16
*  DESCRIPTION   : 为全局共享数据库信息结构体变量赋初值，每个数据库拥有进程互斥的文件锁，线程互斥的互斥锁等数据结构
*  
***********************************************************************************/
static DATABASE_INFO g_DbInfoArray[DB_TYPE_MAX] = 
{
    [DB_TYPE_APPLOG] = {.szDbPath = {0},.pDbHandle = NULL, .hLastLockThread = 0,\
                        .szDbFileName = "AppLog.data", .nLockFd = -1, .szLockFileName = "/opt/appdata/data/.applog.lock",\
                        .s_bIsSelLock_Locked = FALSE, .s_bInRecrusive = FALSE, .s_nCurrLogIdx = 1, .s_bIsSqlIn_RuntimMode = FALSE,\
                        .s_sqLiteDBLock = PTHREAD_MUTEX_INITIALIZER},

    [DB_TYPE_BMSDATA] = {.szDbPath = {0},.pDbHandle = NULL, .hLastLockThread = 0,\
                         .szDbFileName = "BmsRunData.data", .nLockFd = -1, .szLockFileName = "/opt/appdata/data/.bmsrundata.lock",\
                         .s_bIsSelLock_Locked = FALSE, .s_bInRecrusive = FALSE, .s_nCurrLogIdx = 1, .s_bIsSqlIn_RuntimMode = FALSE,\
                         .s_sqLiteDBLock = PTHREAD_MUTEX_INITIALIZER},

    [DB_TYPE_CLOUDDATA] = {.szDbPath = {0},.pDbHandle = NULL, .hLastLockThread = 0,\
                           .szDbFileName = "CloudData.data", .nLockFd = -1,.szLockFileName = "/opt/appdata/data/.clouddata.lock",\
                           .s_bIsSelLock_Locked = FALSE, .s_bInRecrusive = FALSE, .s_nCurrLogIdx = 1, .s_bIsSqlIn_RuntimMode = FALSE,\
                           .s_sqLiteDBLock = PTHREAD_MUTEX_INITIALIZER},    
};


/*********************************************************************************
*  FUNCTION NAME : InitCreatDataBase()
*  CREATOR       : Wanlu
*  CREATED DATE  : 2026-1-16
*  DESCRIPTION   : 初始化创建数据库
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL InitCreatDataBase()
{	
	int n = 0;
	for(n = 0; n < DB_TYPE_MAX; ++n)
	{
		char szDBfileBmsRunData[256] = {0};
		sqlite3* db;
		MakeFullFileName(szDBfileBmsRunData, getenv(ENV_VARIABLE_DATA_PATH), NULL, g_DbInfoArray[n].szDbFileName);

		int rc = sqlite3_open_v2(szDBfileBmsRunData, &db, SQLITE_OPEN_READWRITE, NULL);
		if(rc != SQLITE_OK)
		{
			rc = sqlite3_open_v2(szDBfileBmsRunData, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
			if(rc != SQLITE_OK)
			{
				TRACEI("Can not creat data database[%s], [%s], so exit system\n", szDBfileBmsRunData, getenv(ENV_VARIABLE_DATA_PATH));
				sqlite3_close(db);
				return FALSE;
			}
		}
		sqlite3_busy_timeout(db, 20000);
		g_DbInfoArray[n].pDbHandle = db;	
	}
	return TRUE;
}

/*********************************************************************************
*  FUNCTION NAME : ExitDatabase()
*  CREATOR       : Wanlu
*  CREATED DATE  : 2026-1-16
*  DESCRIPTION   :  程序异常退出时，释放数据库资源
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void ExitDatabase()
{
	int n = 0;
	for(n = 0; n < DB_TYPE_MAX; ++n)
	{
		close(g_DbInfoArray[n].nLockFd);
		sqlite3_close(g_DbInfoArray[n].pDbHandle);
	}
	return;
}

static BOOL Lock_SqlDB_NoBlock(DATABASE_TYPE WhichDatabase,int n_mSec)
{
	int iTimes = n_mSec / 10;
	do 
	{
		if(pthread_mutex_trylock(&g_DbInfoArray[WhichDatabase].s_sqLiteDBLock) == 0) 
		{
			return TRUE;
		}
		iTimes--; 
		Sleep_Ex(10);
	}while(iTimes > 0);
	return FALSE;
}

/*********************************************************************************
*  FUNCTION NAME : ExitDatabase()
*  CREATOR       : Wanlu
*  CREATED DATE  : 2026-1-17
*  DESCRIPTION   : 初始化数据库后，释放旧的数据库句柄，以便后面其他线程重新打开
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
void Database_Switch_RunMode()
{
	int n = 0;
	for(n = 0; n < DB_TYPE_MAX; ++n)
	{
		LOCK_SQLITE_DATABASE_SYNC((DATABASE_TYPE)n ,20000);
		sqlite3_close_v2(g_DbInfoArray[n].pDbHandle);
		g_DbInfoArray[n].pDbHandle = NULL;
		g_DbInfoArray[n].s_bIsSqlIn_RuntimMode = TRUE;
		UNLOCK_SQLITE_DATABASE_SYNC((DATABASE_TYPE)n);
	}
	return;
}

/*********************************************************************************
*  FUNCTION NAME : ExitDatabase()
*  CREATOR       : Wanlu
*  CREATED DATE  : 2026-1-17
*  DESCRIPTION   : 数据库文件锁, 进程间的互斥
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
BOOL Lock_DB_Private_FileLocker(DATABASE_INFO* pCurrDbInfo, int nTimeOut_MSec)
{
    if(pCurrDbInfo->nLockFd <= 0)
    {
        pCurrDbInfo->nLockFd = open(pCurrDbInfo->szLockFileName, O_RDWR | O_CREAT, 0644);
        if(pCurrDbInfo->nLockFd <= 0)
        {
            TRACEI("Open Lock file fail [%s]\n", pCurrDbInfo->szLockFileName);
            return FALSE;
        }
    }

    if(nTimeOut_MSec < 0)
    {
        return TRUE;
    }

    int iTimes = nTimeOut_MSec / 10;

    do
    {
        if(flock(pCurrDbInfo->nLockFd, LOCK_EX | LOCK_NB) >= 0)
        {
            return TRUE;
        }
        Sleep_Ex(10);
        iTimes--;
    }while(iTimes > 0);

    return FALSE;
}

void Unlock_DB_Private_FileLocker(DATABASE_INFO* pCurrDbInfo)
{
    if(pCurrDbInfo->nLockFd <= 0)
    {
        close(pCurrDbInfo->nLockFd);
        return;
    }
    flock(pCurrDbInfo->nLockFd, LOCK_UN);
}

sqlite3* Open_Lock_Database(DATABASE_TYPE WhichDatabase)
{
    if(WhichDatabase >= DB_TYPE_MAX)
    {
        TRACEI("Open the wrong database: [%d]\n", WhichDatabase);
        return NULL;
    }

    DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[WhichDatabase];

    pthread_t hSelf = pthread_self();
    if((pthread_equal(pCurrDbInfo->hLastLockThread, hSelf)) && (pCurrDbInfo->pDbHandle != NULL))
    {
        pCurrDbInfo->s_bInRecrusive = TRUE;
        Lock_DB_Private_FileLocker(pCurrDbInfo ,-1);
        return pCurrDbInfo->pDbHandle;
    }

    if(!Lock_DB_Private_FileLocker(pCurrDbInfo, 20000))
    {

    }

	sqlite3* db = NULL;
    if(pCurrDbInfo->szDbPath[0] == 0)
    {
        MakeFullFileName(pCurrDbInfo->szDbPath, getenv(ENV_VARIABLE_DATA_PATH), NULL, pCurrDbInfo->szDbFileName);
    }
	
    if(LOCK_SQLITE_DATABASE_SYNC(WhichDatabase, 2000))
	{
		pCurrDbInfo->s_bIsSelLock_Locked = TRUE;
	}
	pCurrDbInfo->s_bInRecrusive = FALSE;

	if(pCurrDbInfo->pDbHandle != NULL)
	{
		if(sqlite3_close(pCurrDbInfo->pDbHandle) == SQLITE_BUSY)
		{
			LogOut_A("LOG-SELF", "EXCEPTION", LOG_TYPE_ERROR, "[Open_Lock_SqliteDB]: DB [%s] close busy, so we REUSE it!!!", pCurrDbInfo->szDbFileName);
			return pCurrDbInfo->pDbHandle;
		}
		pCurrDbInfo->pDbHandle = NULL;
	}

	int rc = sqlite3_open_v2(pCurrDbInfo->szDbPath, &db, SQLITE_OPEN_READWRITE, NULL);
	if(rc == SQLITE_OK)
	{
		sqlite3_busy_timeout(db, 20000); 
		pCurrDbInfo->pDbHandle = db;
	}
	else
	{
		db = NULL; 
	}

	pCurrDbInfo->hLastLockThread = hSelf;
	return db;
}

void Close_UnLock_Database(DATABASE_TYPE WhichDatabase)
{
    if(WhichDatabase >= DB_TYPE_MAX)
    {
        TRACEI("fail to close database\n");
        return;
    }
    DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[WhichDatabase];
    Unlock_DB_Private_FileLocker(pCurrDbInfo);
    if(!pCurrDbInfo->s_bInRecrusive)
    {
        pCurrDbInfo->hLastLockThread = 0;
        if(pCurrDbInfo->pDbHandle != NULL)
        {
            int nRet = sqlite3_close(pCurrDbInfo->pDbHandle);
            if(nRet == SQLITE_OK)
            {
                pCurrDbInfo->pDbHandle = NULL;
            }
            else
            {
                LogOut_A("LOG-SELF", "EXCEPTION", LOG_TYPE_ERROR, "[Close_UnLock_Database]: DB close NOK -> [%d] [%s]!!!", nRet,pCurrDbInfo->szDbFileName);
            }
        }
    }
    else
    {
        pCurrDbInfo->s_bInRecrusive = FALSE;
        return;
    }

    if(pCurrDbInfo->s_bIsSelLock_Locked)
    {
        UNLOCK_SQLITE_DATABASE_SYNC(WhichDatabase);
        pCurrDbInfo->s_bIsSelLock_Locked = FALSE;
    }
    return;
}

static void Set_LogIdx_ToVar(DATABASE_TYPE WhichDatabase)
{
    DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[WhichDatabase];
	if(pCurrDbInfo->nLockFd > 0)
	{
		VAR_VALUE varV;
		varV.ulValue = pCurrDbInfo->s_nCurrLogIdx;

		TRACEI("Set_LogIdx_ToVar: LogIdx = %d\n", pCurrDbInfo->s_nCurrLogIdx);

		lseek(pCurrDbInfo->nLockFd, 0, SEEK_SET);
		if(write(pCurrDbInfo->nLockFd, varV.abyValue, 4) < 4)
		{
			close(pCurrDbInfo->nLockFd);
			pCurrDbInfo->nLockFd = -1;
		}
	}
}

static int Get_LogIdx_FromVar(DATABASE_TYPE WhichDatabase)
{
    DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[WhichDatabase];
	if(pCurrDbInfo->nLockFd > 0)
	{
			VAR_VALUE varV;
			varV.ulValue = 0;
			lseek(pCurrDbInfo->nLockFd, 0, SEEK_SET);
			read(pCurrDbInfo->nLockFd, varV.abyValue, 4);
			if((varV.ulValue > 0) && (varV.ulValue <= APP_LOG_SIZE))
			{
				pCurrDbInfo->s_nCurrLogIdx = varV.ulValue;	
			}			
	}
	TRACEI("Get_LogIdx_FromVar: LogIdx = %d\n", pCurrDbInfo->s_nCurrLogIdx);
	return pCurrDbInfo->s_nCurrLogIdx;
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

BOOL Init_Database()
{
    DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[DB_TYPE_APPLOG];
    if(!Lock_DB_Private_FileLocker(pCurrDbInfo, 10000))
    {
        TRACEI("---------Lock Applog fileLocker FAILED--------\n");
        Unlock_DB_Private_FileLocker(pCurrDbInfo);
        return FALSE;
    }

    BOOL bAppLogIdx = FALSE;
    Get_LogIdx_FromVar(DB_TYPE_APPLOG);

	TRACEI("Init_Database: Current AppLog Idx = %d\n", pCurrDbInfo->s_nCurrLogIdx);

	pCurrDbInfo->s_bIsSqlIn_RuntimMode = FALSE;
	
	char* szErrMsg = NULL;
	UINT nTotalLogItem, nCurrIdx;
	char szSql[256];
	
	int nErrCode = -1;
	if(!bAppLogIdx)
	{
		nErrCode = sqlite3_exec(pCurrDbInfo->pDbHandle,"CREATE TABLE IF NOT EXISTS Data_AppLog("
				"Log_Idx INTEGER PRIMARY KEY, "
				"Log_Type INTEGER,"
				"Log_Time DOUBLE,"
				"Log_Data BLOB NOT NULL)",
				NULL, NULL, &szErrMsg);
		if(szErrMsg !=NULL)
		{
			TRACEI("Can not create Log table, reason = [%s].\n", szErrMsg);
			sqlite3_free(szErrMsg);
			Unlock_DB_Private_FileLocker(pCurrDbInfo);
			return FALSE;
		}
		if(nErrCode == SQLITE_OK) 
		{
			szErrMsg = NULL;
			nTotalLogItem = 0;
			nCurrIdx = 0;
			nErrCode = sqlite3_exec(pCurrDbInfo->pDbHandle,"SELECT count(*) FROM Data_AppLog",
				Get_SQlMaxEventIdx, &nTotalLogItem, &szErrMsg);

			snprintf(szSql, 256, "SELECT Log_Idx, max(Log_Time) FROM Data_AppLog WHERE Log_Time < %d", _NOW_ + 5);
			sqlite3_exec(pCurrDbInfo->pDbHandle, szSql, Get_SQlMaxEventIdx, &nCurrIdx, &szErrMsg);
			if(nErrCode != SQLITE_OK)
			{
				sqlite3_free(szErrMsg);
				Unlock_DB_Private_FileLocker(pCurrDbInfo);
				return FALSE;
			}
			Get_LogIdx_FromVar(DB_TYPE_APPLOG);
			if((pCurrDbInfo->s_nCurrLogIdx == 1) && (nCurrIdx >= 1))
			{
				pCurrDbInfo->s_nCurrLogIdx = MAX(pCurrDbInfo->s_nCurrLogIdx, nCurrIdx + 1);
			}		
			if(pCurrDbInfo->s_nCurrLogIdx > APP_LOG_SIZE)
			{
				pCurrDbInfo->s_nCurrLogIdx = 1;
			}		
			Set_LogIdx_ToVar(DB_TYPE_APPLOG);
		}
		TRACEI("The current logIdx = %d.\n", pCurrDbInfo->s_nCurrLogIdx);
	}
    sqlite3_free(szErrMsg);
    sqlite3* Db_AppLog = Open_Lock_Database(DB_TYPE_APPLOG);
    if(Db_AppLog == NULL)
    {
        TRACEI("Open app log database NULL\n");
        return FALSE;
    }

    s_bDbInitOK = TRUE; 
	Unlock_DB_Private_FileLocker(pCurrDbInfo);

	DATABASE_INFO* pCurrBmsInfo= &g_DbInfoArray[DB_TYPE_BMSDATA];
	szErrMsg = NULL;
	sqlite3_exec(pCurrBmsInfo->pDbHandle, "CREATE TABLE IF NOT EXISTS Data_RealTimeBmsData ("
		"Idx          	INTEGER  PRIMARY KEY,"
		"Day   			INTEGER  NOT NULL,"
		"Log_Time  		DATETIME NOT NULL,"
		"Runtime_Data   BLOB     NOT NULL)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_RealTimeBmsData table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	sqlite3_exec(pCurrBmsInfo->pDbHandle, "CREATE TABLE IF NOT EXISTS Data_DODHistory ("
		"Idx           	INTEGER  PRIMARY KEY,"
		"Group_No      	INTEGER  NOT NULL,"
		"Seq_Id        	INTEGER  NOT NULL,"
		"Time_Year     	INTEGER  NOT NULL,"
		"Time_MonthDay 	INTEGER  NOT NULL,"
		"Time_HourMinute INTEGER NOT NULL,"
		"Time_Second   	INTEGER  NOT NULL,"
		"Dod           	INTEGER  NOT NULL,"
		"Dischg_Qty    	INTEGER  NOT NULL,"
		"Chrg_Qty      	INTEGER  NOT NULL,"
		"Dischg_Energy 	INTEGER  NOT NULL,"
		"Chrg_Energy   	INTEGER  NOT NULL,"
		"Save_Time     	INTEGER  NOT NULL)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_DODHistory table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	sqlite3_exec(pCurrBmsInfo->pDbHandle, "CREATE TABLE IF NOT EXISTS Data_SOSHistory ("
		"Idx           	INTEGER  PRIMARY KEY,"
		"Group_No      	INTEGER  NOT NULL,"
		"Seq_Id        	INTEGER  NOT NULL,"
		"Time_Year     	INTEGER  NOT NULL,"
		"Time_MonthDay 	INTEGER  NOT NULL,"
		"Time_HourMinute INTEGER NOT NULL,"
		"Time_Second   	INTEGER  NOT NULL,"
		"Sos_Level     	INTEGER  NOT NULL,"
		"Reason_Bitmap 	INTEGER  NOT NULL,"
		"Event_Type    	INTEGER  NOT NULL,"
		"Reserved_1    	INTEGER  NOT NULL,"
		"Reserved_2    	INTEGER  NOT NULL,"
		"Save_Time     	INTEGER  NOT NULL)",
		NULL, NULL, &szErrMsg);
	if(szErrMsg != NULL)
	{
		TRACE("Can not create Data_SOSHistory table, reason = [%s].\n", szErrMsg);
		sqlite3_free(szErrMsg);
		return FALSE;
	}

	
	return TRUE;
}

BOOL LogOut(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
{
    DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[DB_TYPE_APPLOG];
#ifdef TRACE_2DBLOG
	#define BUFF_SIZE_LOGOUT (1024)
#else
	#define BUFF_SIZE_LOGOUT (512)
#endif
	if(!s_bDbInitOK)
	{
		return FALSE;
	}

	va_list arglist;
	char szBuf[BUFF_SIZE_LOGOUT] = {0};
	//char szThreadName[32] = {0};
	char szThreadName[64] = {0};
	
	char szHeader[64];
#ifdef TRACE_2DBLOG
	struct timeval  tv;
	gettimeofday(&tv, NULL);
	struct tm *timenow;
	timenow = localtime( &tv.tv_sec );
	snprintf(szHeader, 64, "%04d-%02d-%02d %02d:%02d:%02d:%03d %s\t%s\t[%s]\t",
		timenow->tm_year+1900, timenow->tm_mon+1, timenow->tm_mday, timenow->tm_hour, timenow->tm_min, timenow->tm_sec, (int)tv.tv_usec/1000,
		szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64));
#else
	snprintf(szHeader, 64, "%s\t%s\t[%s]\t", szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64));
#endif

	char szContent[BUFF_SIZE_LOGOUT];
	va_start(arglist, pszFormat);
	vsnprintf(szContent, BUFF_SIZE_LOGOUT, pszFormat, arglist);
	va_end(arglist);
	snprintf(szBuf, BUFF_SIZE_LOGOUT, "%s%s", szHeader, szContent);
	LOGFD("LogOut:%d-%s", emLogType, szBuf);
	if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
	{
		Open_Lock_Database(DB_TYPE_APPLOG);
	}
	else
	{
		Lock_DB_Private_FileLocker(pCurrDbInfo, 1000);	
	}

	TRACEI("------The Log_Out Content[%s], and s_LogDB = %lu---\n", szBuf, g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle);
	sqlite3_stmt * stat;
	sqlite3_prepare_v2(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"replace into Data_AppLog values (?,?,?,?);",-1,&stat,NULL);
	sqlite3_bind_int(stat,1, /*(int)s_nCurrLogIdx*/ Get_LogIdx_FromVar(DB_TYPE_APPLOG));
	sqlite3_bind_int(stat,2, emLogType);
	sqlite3_bind_double(stat,3, GetCurrentTime());
	sqlite3_bind_blob(stat, 4, (void*)szBuf, strlen(szBuf) + 1, NULL);;
	int nSqlRet = sqlite3_step(stat);
	sqlite3_finalize(stat);
	TRACEI("[Log %d]: %s\n", g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx, szBuf);

	if(g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx < APP_LOG_SIZE)
	{
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx++;
	}
	else
	{
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx = 1;
	}

	Set_LogIdx_ToVar(DB_TYPE_APPLOG);	
	
	if(nSqlRet != SQLITE_DONE)
	{
		TRACEI("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_AppLog values (?,?,?,?);", 
			nSqlRet, sqlite3_errmsg(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle));
	}

	if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
	{
		Close_UnLock_Database(DB_TYPE_APPLOG);
	}
	else
	{
		Unlock_DB_Private_FileLocker(pCurrDbInfo);
	}
	return TRUE;
}

void LogOut_A(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
{
	DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[DB_TYPE_APPLOG];

	if(!s_bDbInitOK)
	{
		return;
	}

	Open_Lock_Database(DB_TYPE_APPLOG);
	
	va_list arglist;
	char szBuf[512] = {0};
	char szThreadName[64] = {0};

	char szHeader[64] = {0};
	snprintf(szHeader, 64, "%s\t%s\t[%s]\t", szModualName, szSubModualName, RunThread_GetName(NULL, szThreadName, 64));

	//strcat(szBuf, RunThread_GetName(NULL, szThreadName, 32));
	//TRACE("szTHread = %s.\n", szThreadName);
	char szContent[512] = {0};
	va_start(arglist, pszFormat);
	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
	vsnprintf(szContent, 512, pszFormat, arglist);
	va_end(arglist);

	snprintf(szBuf, 512, "%s%s", szHeader, szContent);
	LOGFD("LogOut_A:%d-%s", emLogType, szBuf);

	sqlite3_stmt * stat;
	sqlite3_prepare_v2(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"replace into Data_AppLog values (?,?,?,?);",-1,&stat,NULL);
	sqlite3_bind_int(stat,1, /*(int)s_nCurrLogIdx*/ Get_LogIdx_FromVar(DB_TYPE_APPLOG));
	sqlite3_bind_int(stat,2, emLogType);
	sqlite3_bind_double(stat,3, GetCurrentTime());
	sqlite3_bind_blob(stat, 4, (void*)szBuf, strlen(szBuf) + 1, NULL);
	int nSqlRet = sqlite3_step(stat);
	sqlite3_finalize(stat);//delete stat

	Lock_DB_Private_FileLocker(pCurrDbInfo, 1000);		

	TRACEI("[Log_A %d]: %s\n", g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx, szBuf);

	if(g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx < APP_LOG_SIZE)
	{
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx++;
	}
	else
	{
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx = 1;
	}

	Set_LogIdx_ToVar(DB_TYPE_APPLOG);	

	Unlock_DB_Private_FileLocker(pCurrDbInfo);
	
	if(nSqlRet != SQLITE_DONE)
	{
		// TRACEX("---Log_A: Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_AppLog values (?,?,?,?);", 
		// 	nSqlRet, sqlite3_errmsg(s_LogDB));
		//system("reboot");
	}
	Close_UnLock_Database(DB_TYPE_APPLOG);

	return;
}

void LogOut_Ex(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...)
{
	DATABASE_INFO* pCurrDbInfo = &g_DbInfoArray[DB_TYPE_APPLOG];

	if(!s_bDbInitOK)
	{
		InitCreatDataBase();
		Init_Database();
	}

	va_list arglist;
	char szBuf[512] = {0};
	//char szThreadName[64] = {0};

	char szHeader[64] = {0};
	snprintf(szHeader, 64, "%s\t%s\t[%s]\t", szModualName, szSubModualName, "N/A" /*RunThread_GetName(NULL, szThreadName, 64)*/);

	//strcat(szBuf, RunThread_GetName(NULL, szThreadName, 32));
	//TRACE("szTHread = %s.\n", szThreadName);
	char szContent[512] = {0};
	va_start(arglist, pszFormat);
	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
	vsnprintf(szContent, 512, pszFormat, arglist);
	va_end(arglist);

	snprintf(szBuf, 512, "%s%s", szHeader, szContent);
	LOGFD("LogOut_Ex:%s", szBuf);
	TRACEI("------The Log_Ex Content[%s]---\n", szBuf);


	if(!Lock_DB_Private_FileLocker(pCurrDbInfo, 2000))
	{
		//TRACEI("---------Lock sqlDB fileLocker FAILED--------\n");
		Unlock_DB_Private_FileLocker(pCurrDbInfo);
		return;
	}

	sqlite3* db = NULL; //= (sqlite3*)pCharger->hRunDB;

	char	szDBFile[256] = {0};
	//added by Jimmy 20201211: if env not provided, should use default PATH
	char *szDataRoot = getenv(ENV_VARIABLE_DATA_PATH);
	if(szDataRoot != NULL)
	{
		MakeFullFileName(szDBFile, szDataRoot, NULL, "AppLog.data");
	}
	else
	{
		MakeFullFileName(szDBFile, "/data", NULL, "AppLog.data");
	}	

	if(!Lock_DB_Private_FileLocker(pCurrDbInfo, 2000))
	{
		//printf("---------Lock sqlDB fileLocker FAILED--------\n");
		Unlock_DB_Private_FileLocker(pCurrDbInfo);
		return;
	}

	int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
	if(rc == SQLITE_OK)
	{
		sqlite3_busy_timeout(db, 20000); //set default BUSY_HANDLER for sql database
	}
	else
	{
		sqlite3_close(db);
		Unlock_DB_Private_FileLocker(pCurrDbInfo);
		return;
	}

	//replace means it exist then update, or else insert
	sqlite3_stmt * stat;
	sqlite3_prepare_v2(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"replace into Data_AppLog values (?,?,?,?);",-1,&stat,NULL);
	sqlite3_bind_int(stat,1, /*(int)s_nCurrLogIdx*/Get_LogIdx_FromVar(DB_TYPE_APPLOG));
	sqlite3_bind_int(stat,2, emLogType);
	sqlite3_bind_double(stat,3, GetCurrentTime());
	sqlite3_bind_blob(stat, 4, (void*)szBuf, strlen(szBuf) + 1, NULL);
	int nSqlRet = sqlite3_step(stat);
	sqlite3_finalize(stat);//delete stat

	TRACEI("[LogEx %d]: %s\n", g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx, szBuf);

	if(g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx < APP_LOG_SIZE)
	{
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx++;
	}
	else
	{
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx = 1;
	}

	Set_LogIdx_ToVar(DB_TYPE_APPLOG);

	Unlock_DB_Private_FileLocker(pCurrDbInfo);

}

BOOL DumpOut(const IN int nDumpSrc, IN DUMP_TYPE emDumpType, IN char *szDumpData)
{
	char szBuf[MAX_DUM_DATASIZE] = {0};
	//Lock
	if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
	{
		Open_Lock_Database(DB_TYPE_APPLOG);
	}

	if(!s_bIsInitDumpDBOK)
	{
		char* szErrMsg = NULL;
		int nErrCode = sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"CREATE TABLE IF NOT EXISTS Data_MBELog("
			"Log_Idx INTEGER PRIMARY KEY, "
			"Log_Time DOUBLE, "
			"Log_Data BLOB NOT NULL)",
			NULL, NULL, &szErrMsg);
		if(szErrMsg != NULL)
		{
			sqlite3_free(szErrMsg);
			Close_UnLock_SqliteDB();
			return FALSE;
		}
		char szSql[256];
		UINT nGetIdx = 0;
		sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,  "SELECT count(*) FROM Data_MBELog", Get_SQlMaxEventIdx, &nGetIdx, NULL);
		if(nGetIdx > APP_DATADUMP_SIZE)
		{
			s_nCurrDumpLogIdx = 1;
			sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,  "DELETE from Data_MBELog", NULL, NULL, NULL);
		}
		else
		{
			nGetIdx = 0;
			snprintf(szSql, 256, "SELECT Log_Idx, max(Log_Time) FROM Data_MBELog WHERE Log_Time < %d", _NOW_ + 5);
			sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,  szSql, Get_SQlMaxEventIdx, &nGetIdx, NULL);
			if(nGetIdx > 0)
			{
				s_nCurrDumpLogIdx = nGetIdx + 1;
			}			
		}
		s_bIsInitDumpDBOK = TRUE;
	}

	snprintf(szBuf, MAX_DUM_DATASIZE, "%s", szDumpData);

	sqlite3_stmt * stat;
	sqlite3_prepare_v2(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"replace into Data_DumpLog values (?,?,?,?,?);",-1,&stat,NULL);
	sqlite3_bind_int(stat,1, (int)s_nCurrDumpLogIdx);
	sqlite3_bind_int(stat,2, nDumpSrc);
	sqlite3_bind_int(stat,3, emDumpType);
	sqlite3_bind_double(stat,4, GetCurrentTime());
	sqlite3_bind_text(stat, 5, szBuf, strlen(szBuf), NULL);
	int nSqlRet = sqlite3_step(stat);
	sqlite3_finalize(stat);//delete stat

	if(s_nCurrDumpLogIdx < APP_DATADUMP_SIZE)
	{
		s_nCurrDumpLogIdx++;
	}
	else
	{
		s_nCurrDumpLogIdx = 1;
	}
	//unlock
	if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
	{
		Close_UnLock_Database(DB_TYPE_APPLOG);
	}

	if(nSqlRet != SQLITE_DONE)
	{
		TRACEX("---Execute sql[%s], errCode = %d, errMsg = [%s]---\n", "replace into Data_DumpLog values (?,?,?,?);", 
			nSqlRet, sqlite3_errmsg(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle));
		//system("reboot");
	}

	return TRUE;
}

static char* MakeLogType(IN int nLogType, OUT char* szLogTp)
{
	switch (nLogType)
	{
	case LOG_TYPE_INFO:
		sprintf(szLogTp, "%s", "INFO");
		break;
	case LOG_TYPE_WARING:
		sprintf(szLogTp, "%s", "WARN");
		break;
	case LOG_TYPE_ERROR:
		sprintf(szLogTp, "%s", "ERROR");
		break;
	case LOG_TYPE_MILESTONE:
		sprintf(szLogTp, "%s", "MILESTONE");
		break;
	default:
		sprintf(szLogTp, "%s", "UNKNOWN");
		break;
	}
	return szLogTp;
}

static char *MakeDumpType(IN int nDumpType, OUT char *szDumpTp)
{
	switch (nDumpType)
	{
	case DUMP_TYPE_OPEN_COMM:
		sprintf(szDumpTp, "%s", "OPEN_COMM");
		break;
	case DUMP_TYPE_CLOSE_COMM:
		sprintf(szDumpTp, "%s", "CLOSE_COMM");
		break;
	case DUMP_TYPE_INFO:
		sprintf(szDumpTp, "%s", "INFO");
		break;
	case DUMP_TYPE_READ:
		sprintf(szDumpTp, "%s", "READ");
		break;
	case DUMP_TYPE_WRITE:
		sprintf(szDumpTp, "%s", "WRITE");
		break;
	default:
		sprintf(szDumpTp, "%s", "UNKNOWN");
		break;
	}
	return szDumpTp;
}

int LogCat(char* szFile, BOOL bInternal)
{
	// write head msg
	FILE *fp = fopen( szFile, "w");
	if (fp == NULL)
	{
		return -1;
	}

	sqlite3_stmt * stat;
	sqlite3* db;
	if(bInternal)
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			db = Open_Lock_Database(DB_TYPE_APPLOG);
		}
	}
	else
	{
		char szDBFile[256] = {0};
		MakeFullFileName(szDBFile, \
			getenv("NCCU_DATA_PATH"), NULL, "AppLog.data");
		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READONLY, NULL);
		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
		if(rc != SQLITE_OK)
		{
			fclose(fp);
			return -1;
		}

		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
	}

	sqlite3_prepare_v2(db,"SELECT * FROM Data_AppLog ORDER BY Log_Time ASC",-1,&stat,NULL);
	int nLogIdx = 1;
	char szTime[32];
	char szLogType[16];
	int nLogTotal = 0;
	//char szLogContent[256] = {0};

	while (sqlite3_step(stat) == SQLITE_ROW)
	{
		nLogIdx = sqlite3_column_int(stat, 0);
		int nLogType = sqlite3_column_int(stat, 1);
		double fLogTime = sqlite3_column_double(stat, 2);
		AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32);
		const UCHAR *szLog = sqlite3_column_text(stat, 3);
		//sqlite3_parse_blob_uncompressed(stat, 3, (void*)szLogContent, 256);
		fprintf(fp, "[%04d]: %d\t%s\t%s\t%s\n", nLogTotal + 1, nLogIdx, MakeLogType(nLogType, szLogType),
			szTime, szLog /*szLogContent*/);
		nLogTotal++;
	}

	sqlite3_finalize(stat);
	fclose(fp);

	if(!bInternal)
	{
		sqlite3_close_v2(db);
	}
	else
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			Close_UnLock_Database(DB_TYPE_APPLOG);
		}
	}

	return nLogTotal;
}

void Log_Delete(BOOL bInternal, int nLogDeleteType)
{
	sqlite3* db;
	if(bInternal)
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			db = Open_Lock_Database(DB_TYPE_APPLOG);
		}
	}
	else
	{
		Lock_SqlDB_FileLocker(3000);
		char szDBFile[256] = {0};
		MakeFullFileName(szDBFile, \
			getenv("NCCU_DATA_PATH"), NULL, "AppLog.data");
		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READWRITE, NULL);
		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
		if(rc != SQLITE_OK)
		{
			Unlock_SqlDB_FileLocker();
			return;
		}

		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
	}

	char* szErrMsg = NULL;
	if(nLogDeleteType == LOG_DELTE_DUMPLOG)
	{
		sqlite3_exec(db,"DELETE FROM Data_DumpLog", NULL, NULL, &szErrMsg);
		s_nCurrDumpLogIdx = 1;
	}
	else if(nLogDeleteType == LOG_DELTE_MBELOG)
	{
		sqlite3_exec(db,"DELETE FROM Data_MBELog", NULL, NULL, &szErrMsg);
		s_nCurrMBELogIdx = 1;
	}
	else
	{
		sqlite3_exec(db,"DELETE FROM Data_AppLog", NULL, NULL, &szErrMsg);
		g_DbInfoArray[DB_TYPE_APPLOG].s_nCurrLogIdx = 1;
	}	

	if(szErrMsg != NULL)
	{
		sqlite3_free(szErrMsg); 
	}
	if(!bInternal)
	{
		sqlite3_close_v2(db);
	}
	else
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			Close_UnLock_Database(DB_TYPE_APPLOG);
		}
	}

	Unlock_SqlDB_FileLocker();
}

int LogDump(char* szFile, BOOL bInternal)
{
	FILE *fp;
	fp = fopen(szFile, "w");
	if(fp == NULL)
	{
		return -1;
	}
	sqlite3_stmt * stat;
	sqlite3* db;
	if(bInternal)
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			db = Open_Lock_Database(DB_TYPE_APPLOG);
		}
	}
	else
	{
		char szDBFile[256] = {0};
		MakeFullFileName(szDBFile,\
			getenv("NCCU_DATA_PATH"), NULL, "AppLog.data");
		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READONLY, NULL);
		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
		if(rc != SQLITE_OK)
		{
			fclose(fp);
			return -1;
		}

		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
	}

	sqlite3_prepare_v2(db,"select * from (select * from Data_DumpLog order by Dump_Time DESC limit 5000) order by Dump_Time ASC;",-1,&stat,NULL);
	int nLogIdx = 1;
	char szTime[32];
	char szDumpType[16];
	int nLogTotal = 0;

	while (sqlite3_step(stat) == SQLITE_ROW)
	{
		nLogIdx = sqlite3_column_int(stat, 0);
		int nDumpSrc = sqlite3_column_int(stat, 1);
		int nDumpType = sqlite3_column_int(stat, 2);
		double fLogTime = sqlite3_column_double(stat, 3);
		AcurTimeToString(fLogTime, TIME_CHN_FMT, szTime, 32);
		const UCHAR *szLog = sqlite3_column_text(stat, 4);
		fprintf(fp, "[%04d]: %d\t%s\t%s\t%s\t%s\n", nLogTotal + 1, nLogIdx, "Usr_Rept",MakeDumpType(nDumpType, szDumpType),
			szTime, szLog);
		nLogTotal++;
	}

	sqlite3_finalize(stat);
	fclose(fp);

	if(!bInternal)
	{
		sqlite3_close_v2(db);
	}
	else
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			Close_UnLock_Database(DB_TYPE_APPLOG);
		}
	}
	return nLogTotal;
}

void LogOut_MBE(IN char *pszFormat, ...)
{
	Open_Lock_Database(DB_TYPE_APPLOG);
	if(!s_bIsInitMBEDBOK)
	{
		char* szErrMsg = NULL;
		//according to official doc, 'create table if not exits...' statement will still return error but has no error msg
		int nErrCode = sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"CREATE TABLE IF NOT EXISTS Data_MBELog("
			"Log_Idx INTEGER PRIMARY KEY, "
			"Log_Time DOUBLE, "
			"Log_Data BLOB NOT NULL)",
			NULL, NULL, &szErrMsg);
		if(szErrMsg != NULL)
		{
			//TRACEI("Can not create Log table, reason = [%s].\n", szErrMsg);
			sqlite3_free(szErrMsg);
			Close_UnLock_Database(DB_TYPE_APPLOG);
			return;
		}
		char szSql[256];
		int nGetIdx = -1;
		sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,  "SELECT count(*) FROM Data_MBELog", Get_SQlMaxEventIdx, &nGetIdx, NULL);
		if(nGetIdx > MAX_MBELOG_TOTAL)
		{
			s_nCurrMBELogIdx = 1;
			//Jimmy added 20210902 to shorten the volome 5000 -> 1000
			sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,  "DELETE from Data_MBELog", NULL, NULL, NULL);
		}
		else
		{
			nGetIdx = -1;
			snprintf(szSql, 256, "SELECT Log_Idx, max(Log_Time) FROM Data_MBELog WHERE Log_Time < %d", _NOW_ + 5);
			sqlite3_exec(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,  szSql, Get_SQlMaxEventIdx, &nGetIdx, NULL);
			if(nGetIdx > 0)
			{
				s_nCurrMBELogIdx = nGetIdx + 1;
			}			
		}

		//TRACEI("------The MBELogIdx => %d-----\n", s_nCurrMBELogIdx);
		s_bIsInitMBEDBOK = TRUE;
	}

	char szBuf[4096] = {0};
	va_list arglist;
	va_start(arglist, pszFormat);
	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
	vsnprintf(szBuf, 4096, pszFormat, arglist);
	va_end(arglist);
	LOGFD("LogOut_MBE:%s", szBuf);

	//replace means if exist then update, or else insert
	sqlite3_stmt * stat;
	sqlite3_prepare_v2(g_DbInfoArray[DB_TYPE_APPLOG].pDbHandle,"replace into Data_MBELog values (?,?,?);",-1,&stat,NULL);
	sqlite3_bind_int(stat, 1, s_nCurrMBELogIdx);
	sqlite3_bind_double(stat, 2, GetCurrentTime());
	sqlite3_bind_blob(stat, 3, (void*)szBuf, strlen(szBuf) + 1, NULL);
	int nSqlRet = sqlite3_step(stat);
	sqlite3_finalize(stat);//delete stat

	if(s_nCurrMBELogIdx < MAX_MBELOG_TOTAL)
	{
		s_nCurrMBELogIdx++;
	}
	else
	{
		s_nCurrMBELogIdx = 1;
	}

	Close_UnLock_Database(DB_TYPE_APPLOG);
}

int Log_Dump_MBE(char* szFile, BOOL bInternal)
{
	FILE *fp;
	fp = fopen(szFile, "w");
	if(fp == NULL)
	{
		return -1;
	}
	sqlite3_stmt * stat;
	sqlite3* db;
	if(bInternal)
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			db = Open_Lock_Database(DB_TYPE_APPLOG);
		}
	}
	else
	{
		char szDBFile[256] = {0};
		MakeFullFileName(szDBFile,\
			getenv("NCCU_DATA_PATH"), NULL, "AppLog.data");
		int rc = sqlite3_open_v2( szDBFile, &db, SQLITE_OPEN_READONLY, NULL);
		TRACE("Open LogDB[%s], rC = %d.\n",szDBFile, rc);
		if(rc != SQLITE_OK)
		{
			fclose(fp);
			return -1;
		}

		sqlite3_busy_timeout(db, 5000); //set default BUSY_HANDLER for sql database
	}

	sqlite3_prepare_v2(db,"select * from Data_MBELog order by Log_Time ASC",-1,&stat,NULL);
	int nLogIdx = 1;
	// char szTime[32];
	// char szDumpType[16];
	int nLogTotal = 0;

	while (sqlite3_step(stat) == SQLITE_ROW)
	{
		nLogIdx = sqlite3_column_int(stat, 0);
		const UCHAR *szLog = sqlite3_column_text(stat, 2);
		fprintf(fp, "[%04d]: %d\t%s\n", nLogTotal+ 1, nLogIdx, szLog);
		nLogTotal++;
	}

	sqlite3_finalize(stat);
	fclose(fp);

	if(!bInternal)
	{
		sqlite3_close_v2(db);
	}
	else
	{
		if(g_DbInfoArray[DB_TYPE_APPLOG].s_bIsSqlIn_RuntimMode)
		{
			Close_UnLock_Database(DB_TYPE_APPLOG);
		}
	}
	return nLogTotal;
}
