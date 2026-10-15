#ifndef __RUNTHREAD_H__
#define __RUNTHREAD_H__

// max wait time interval in sleep() or select().
#define MAX_WAIT_INTERVAL			5	//the maximum wait time in seconds.
#define LEN_RUN_THREAD_NAME		16	// the maximum length of the run thread name	


#define RUN_THREAD_INIT_HEARTBEATS		2	// the init heartbeats is 2
#define RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL	500		// 500ms 

//#ifdef  _DEBUG
//#define RUN_THREAD_HEARTBEAT_CHECK_INTERVAL		10000	// 10s. //Jimmy uncommented this line 20190412 because of OCPP timeout caused no response
//#else
#define RUN_THREAD_HEARTBEAT_CHECK_INTERVAL		60000	// 60s.//Jimmy changed to 60*3 20190527 because of wslib has 102s timeout for 3-network card
//#endif

#if (RUN_THREAD_HEARTBEAT_CHECK_INTERVAL < (2*MAX_WAIT_INTERVAL*1000))
#error The value "RUN_THREAD_HEARTBEAT_CHECK_INTERVAL" is too small!
#endif

typedef void *(*PTHREAD_START_ROUTINE)(void *);

typedef DWORD (*RUN_THREAD_START_PROC)(void* pArgs);

typedef DWORD (*RUN_THREAD_EVENT_HANDLER)
(				
	DWORD	dwThreadEvent,		//	the thread event. below.
	HANDLE	hThread,			//	The thread id.
	const char	*pThreadName	//  the thread name.
);

// dwThreadEvent enumerate of RUN_THREAD_EVENT_HANDLER:
enum _THREAD_EVENT_ENUM
{
	THREAD_EVENT_HEARTBEAT			  =	1,// The heartbeat event from thread
	THREAD_EVENT_NO_RESPONSE		  = 2,// the thread is no response event.
	THREAD_EVENT_MANAGER_HAS_STARTED  = 3,
	THREAD_EVENT_IS_STOPPED	          =	4,// the thread is stooped.
	THREAD_EVENT_CANNOT_BE_STOPPED	  = 5,// the thread can NOT be stooped.
	THREAD_EVENT_MANAGER_HAS_STOPPED  =	6
};

// The return result of RUN_THREAD_EVENT_HANDLER
#define THREAD_CONTINUE_RUN			0	// to continue run the thread
#define THREAD_CANCEL_THIS			1	// to cancel this running thread(hThread)
#define THREAD_CANCEL_ALL			2	// to stop all running thread(quit system)

#define ERR_THREAD_OK				0
#define ERR_THREAD_KILLED			1
#define ERR_THREAD_STILL_RUNNING	2
#define ERR_THREAD_INIT_MANAGER	1
#define ERR_THREAD_EXIT_MANAGER	1


HANDLE RunThread_Create(IN const char *pszThreadName, 
						IN RUN_THREAD_START_PROC pfnThreadProc,
						IN void *pThreadArg,
						OUT DWORD *pdwExitCode);



HANDLE RunThread_GetId(IN const char *pszThreadName);
char *RunThread_GetName(IN HANDLE hThread, OUT char *pszName, IN int nNameLen);
int RunThread_GetStatus(IN HANDLE hThread);

// status of run thread
#define RUN_THREAD_IS_RUNNING		0	// is running
#define RUN_THREAD_TO_QUIT			1	// thread need to quit.
#define RUN_THREAD_IS_INVALID		2	// the thread is invalid

#define THREAD_IS_RUNNING(hSelf)		(RunThread_GetStatus((hSelf)) == RUN_THREAD_IS_RUNNING)
#define THREAD_IS_QUITED(hSelf)		(RunThread_GetStatus((hSelf)) == RUN_THREAD_IS_INVALID)

int RunThread_GetThreadCount(void);
UINT RunThread_Heartbeat(IN HANDLE hThread);
#define RUN_THREAD_HEARTBEAT()	RunThread_Heartbeat(RunThread_GetId(NULL))	

int RunThread_Stop(IN HANDLE hThread, IN int nTimeToWaitThreadQuit, IN BOOL bKillTimeoutedThread);
int RunThread_ManagerInit(IN RUN_THREAD_EVENT_HANDLER pfnEventHandler);
int RunThread_ManagerExit(IN int nTimeToWaitThreadQuit, IN BOOL bKillTimeoutedThread);

#define RUN_THREAD_NO_EXIT		0

#endif //__RUNTHREAD_H__

