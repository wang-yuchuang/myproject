#ifndef _LOG_MODEL_2018_04_03_
#define _LOG_MODEL_2018_04_03_

#include "base_defines/basetypes.h"
#include "3rd_party/sqlite/sqlite3.h"

typedef enum tagLogType
{
	LOG_TYPE_INFO,
	LOG_TYPE_MILESTONE,
	LOG_TYPE_WARING,
    LOG_TYPE_ERROR
}LOG_TYPE;

typedef enum tagBattDegrLogType
{
	LOG_FULLY_CHG,
	LOG_FULLY_DISCHG
}BATTDEGR_LOG_TYPE;

typedef enum tagDumpType
{
	DUMP_TYPE_OPEN_COMM,
	DUMP_TYPE_CLOSE_COMM,
	DUMP_TYPE_INFO,
	DUMP_TYPE_READ,
	DUMP_TYPE_WRITE,
}DUMP_TYPE;

#define MAIN_MODULE_PUBLIB		"PUBL"
#define MAIN_MODULE_MAIN		"MAIN"
#define MAIN_MODULE_DPR		"DPR"
#define MAIN_MODULE_DAI		"DAI"
#define MAIN_MODULE_SAMPLER	"SAMP"
#define MAIN_MODULE_REPORTER	"REPT"
#define MAIN_MODULE_GUI        "QT-GUI"
#define MAIN_MODULE_WEB        "WEB"
#define MAIN_MODULE_SYSHAND    "PROC-SYSHAND"
#define MAIN_MODULE_APPUPDT    "PROC-APPUPDT"
#define APP_LOG_SIZE	(10000) 	//Max 10000 records
#define APP_DATADUMP_SIZE	(10000) 	//Max 10000 records ,John revised 30000->10000 20211215

BOOL Init_LogRepo(HANDLE hRepo);
void Set_SqliteDB_2RunTime_Mode(void); //Jimmy added 20190706, for Runtime mode: shall lock/unlock the DB
sqlite3* Open_Lock_SqliteDB(void); //Jimmy added 20190706 for sqlite Lock
void Close_UnLock_SqliteDB(void);
void Exit_LogRepo(void);
//for log
void LogOut_A(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...);
BOOL LogOut(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...);
void LogOut_Ex(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...);
int LogCat(char* szFile, BOOL bInternal); //bInternal = FALSE means from other process.
int LogDump(char* szFile, BOOL bInternal); //bInternal = FALSE means from other process.
BOOL LogOut4c(const IN char *szModualName,const IN char *szSubModualName,IN LOG_TYPE emLogType, IN char *pszFormat, ...);
#define LOG_DELTE_SYSLOG	0
#define LOG_DELTE_DUMPLOG	1
#define LOG_DELTE_MBELOG	2
void Log_Delete(BOOL bInternal, int nLogDeleteType);

//for data dump
#define DUMP_SRC_OCPP		0
#define DUMP_SRC_ES_iOCPP	1
#define MAX_DUM_DATASIZE	65535
BOOL DumpOut(const IN int nDumpSrc, IN DUMP_TYPE emDumpType, IN char *szDumpData);

//added by Jimmy 20210714 for MBE logging
void LogOut_MBE(IN char *pszFormat, ...);
int Log_Dump_MBE(char* szFile, BOOL bInternal);

//added by Jeffery 20240717 for Battery Degradation logging
void LogOut_BattDegr(IN BATTDEGR_LOG_TYPE logType, IN const char *logDate, IN char *pszFormat, ...);

//Jimmy put the two interfaces here 20210729
BOOL Lock_SqlDB_FileLocker(int nTimeOut_MSec);
void Unlock_SqlDB_FileLocker(void);

typedef enum tagDataBaseType
{
	DB_TYPE_APPLOG = 0,
	DB_TYPE_BMSDATA,
	DB_TYPE_CLOUDDATA,

	DB_TYPE_MAX = 3
}DATABASE_TYPE;

typedef struct tagDataBase_Info
{
	char szDbPath[256];
	const char* szDbFileName;
	sqlite3* pDbHandle;
	pthread_t hLastLockThread;
	int nLockFd;
	const char* szLockFileName;
	BOOL s_bIsSelLock_Locked;
	BOOL s_bInRecrusive;
	UINT s_nCurrLogIdx;
	BOOL s_bIsSqlIn_RuntimMode;
	pthread_mutex_t	s_sqLiteDBLock;
}DATABASE_INFO;

BOOL InitCreatDataBase(void);
void ExitDatabase(void);
BOOL Lock_DB_Private_FileLocker(DATABASE_INFO* pCurrDbInfo, int nTimeOut_MSec);
void Unlock_DB_Private_FileLocker(DATABASE_INFO* pCurrDbInfo);
sqlite3* Open_Lock_Database(DATABASE_TYPE WhichDatabase);
void Close_UnLock_Database(DATABASE_TYPE WhichDatabase);
BOOL Init_Database(void);
void Database_Switch_RunMode(void);

#endif
