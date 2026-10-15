#define _GNU_SOURCE	1

#include <signal.h>
#include <setjmp.h>

#include "std_sys.h"
#include "base_defines/basetypes.h"
#include "base_defines/iNew.h"
#include "base_defines/basefunc.h"
#include "base_defines/iQueue.h"
#include "base_defines/iThread.h"
#include "base_defines/iLog.h"

#ifdef _DEBUG
//#define _DEBUG_RUN_THREAD	1
#endif //_DEBUG

// the thread entry of mamanger can be enlarged dynamically
#define _RUN_THREAD_DYNAMIC_ENTRY		1
#define NAME_THREAD_MANAGER		"Thread Manager"

// total created threads to be used as a magic code in making thread id
static UINT s_nTotalCreatedThreads = 0;

// (x|1)<<16 to make sure NULL(0) will not be returned.
#define MAKE_THREAD_ID(idx)		((((++s_nTotalCreatedThreads)|1)<<16) \
									+(USHORT)(idx))
#define GET_THREAD_INDEX(id)	((int)(short)((int)id))

typedef struct tagThreaEntry	//	The thread info
{
	DWORD	dwThreadId;							// hiword is a magic word, low word is the index in thread manager
	char		szThreadName [LEN_RUN_THREAD_NAME];
	pthread_t	hSystemThread;							//The ID of pThread in Linux OS
	DWORD	volatile dwThreadHeartbeat;				//current thread heartbeat count
	int volatile	nStatus;								//The running status.
	DWORD	*pdwExitCode;							// exit code ptr.
	int		nFailTimes;
}RUN_THREAD_ENTRY;

#define MAX_THREAD_ENTRY		256	// The maximum thread can be created.
#define THREAD_ENTRY_INC_STEP	32	// The step to increase the pThreadEntries

#ifdef _RUN_THREAD_DYNAMIC_ENTRY
#define THREAD_INIT_ENTRY		THREAD_ENTRY_INC_STEP
#else
#define THREAD_INIT_ENTRY		MAX_THREAD_ENTRY
#endif

typedef struct tagThreadManager	//	The run-thread manager
{
	int	  	 				nThreadEntries;		//max threads can be created
	RUN_THREAD_ENTRY			*pThreadEntries;	//entries to save created thread
	int	volatile				nRunningThreads;	//the num of running threads
	RUN_THREAD_EVENT_HANDLER	pfnEventHandler;	//handler to process thread event
	pthread_mutex_t			hSyncLock;		//lock to protect the entry
	int						nLastEmptyEntry;	//the last release entry idx
	pthread_t					hManagerThread;	//thread id of the manager
	int volatile					nStatus;			//status of the manager.
}RUN_THREAD_MANAGER;

static RUN_THREAD_MANAGER	s_runMgr =
{
	0,							//nThreadEntries
	NULL,						//pThreadEntries
	0,							//nRunningThreads
	NULL,						//pfnEventHandler
//	PTHREAD_MUTEX_INITIALIZER,		//mutex
	PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP,// can be locked recursively
	0,							//nLastEmptyEntry
	0,							//hManagerThread
	RUN_THREAD_IS_INVALID			//nStatus:
};

#define PMGR_SYNC_LOCK				&s_runMgr.hSyncLock
#define LOCK_THREAD_MANAGER()		pthread_mutex_lock(PMGR_SYNC_LOCK)
#define UNLOCK_THREAD_MANAGER()	pthread_mutex_unlock(PMGR_SYNC_LOCK)
#define DESTROY_THREAD_MANAGER()	pthread_mutex_destroy(PMGR_SYNC_LOCK)

typedef struct tagThreadStartArg
{
#ifdef _DEBUG_RUN_THREAD
	RUN_THREAD_ENTRY			*pEntry;		// thread entry in manager
#endif //_DEBUG_RUN_THREAD
	HANDLE					hThread;		// thread handle
	RUN_THREAD_START_PROC	pfnThreadProc;// start proc
	void						*pThreadArg;	// start arg
	pthread_mutex_t			hSyncLock;	// sync lock for creating thread
}RUN_THREAD_START_ARG;

#define LOCK_THREAD_SYNC(p)		pthread_mutex_lock(&((p)->hSyncLock))
#define UNLOCK_THREAD_SYNC(p)	pthread_mutex_unlock(&((p)->hSyncLock))
#define DESTROY_THREAD_SYNC(p)	pthread_mutex_destroy(&((p)->hSyncLock))


#define RUN_THREAD_IS_VALID(pEntry)	((pEntry)->dwThreadId != 0)

static RUN_THREAD_ENTRY *RunThread_GetEntryById(IN HANDLE hThread);
static int RunThread_CancelThreads(IN OUT RUN_THREAD_ENTRY *pEntry, IN int nEntry);

static BOOL RunThread_EnlargeEntries(IN int nEntryToAdd)
{
	RUN_THREAD_ENTRY	*pNewEntries;
	int					nNewEntries;

	nNewEntries = s_runMgr.nThreadEntries+nEntryToAdd;

	// to enlarge the entry buffer
	pNewEntries = RENEW(RUN_THREAD_ENTRY, 
		s_runMgr.pThreadEntries, (UINT)nNewEntries);
	if (pNewEntries == NULL)
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_EnlargeEntries] -- "
			"out of memory on enlarging thread entries to %d.\n",
			nNewEntries);
#endif

		return FALSE;
	}

	// ZERO_POBJS(&pNewEntries[s_runMgr.nThreadEntries], (UINT)nNewEntries);	// old : clean the new allocated buffer
	ZERO_POBJS(&pNewEntries[s_runMgr.nThreadEntries], (UINT)nEntryToAdd);	// new : clean the new allocated buffer

	s_runMgr.pThreadEntries = pNewEntries;	// set new entry ptr.
	s_runMgr.nThreadEntries = nNewEntries;

	return TRUE;
}

static int RunThread_GetEmptyEntry(void)
{
	RUN_THREAD_ENTRY *pEntry;
	int i;

	// test the entries is large enough or not,
	// if not enough, to enlarge the thread entry buffer.
	if (s_runMgr.nRunningThreads >= s_runMgr.nThreadEntries)
	{
#ifdef _RUN_THREAD_DYNAMIC_ENTRY
		if (!RunThread_EnlargeEntries(THREAD_ENTRY_INC_STEP))
		{
			return -1;
		}
#else
		if ((s_runMgr.nThreadEntries != 0) ||				// alread init'd
			!RunThread_EnlargeEntries(THREAD_INIT_ENTRY))	// or init fail.
		{
			return -1;
		}
#endif
	}

	// to search a empty entry
	if (s_runMgr.nLastEmptyEntry >=  s_runMgr.nThreadEntries)
	{
		s_runMgr.nLastEmptyEntry = 0;
	}

	i = s_runMgr.nLastEmptyEntry;
	pEntry = &s_runMgr.pThreadEntries[i];

	// we always can find a empty entry.
	while (RUN_THREAD_IS_VALID(pEntry))
	{
		i++;	// move to the next entry
		pEntry++;

		if (i >= s_runMgr.nThreadEntries)
		{
			// reach end of the last entry, to the first entry. 
			i = 0;
			pEntry = &s_runMgr.pThreadEntries[i];
		}
	}

	// we simply consider the next entry is free.
	s_runMgr.nLastEmptyEntry = i+1;
	s_runMgr.nRunningThreads++;	// increase the running thread num

#ifdef _DEBUG_RUN_THREAD
	TRACEX("Current running threa num is %d.\n", s_runMgr.nRunningThreads);
#endif //_DEBUG_RUN_THREAD

	return i;
}

static void RunThread_ReleaseEntry(IN OUT RUN_THREAD_ENTRY *pEntry)
{
	if (pEntry == NULL)
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_ReleaseEntry] -- Try to release a NULL entry.\n");
#endif //_DEBUG_RUN_THREAD
		return;
	}

	pEntry->nStatus = RUN_THREAD_IS_INVALID;

	s_runMgr.nLastEmptyEntry = GET_THREAD_INDEX(pEntry->dwThreadId);
	s_runMgr.nRunningThreads--;		// the running thread num

#ifdef _DEBUG_RUN_THREAD
	TRACEX("Now release thread %s(%d), Current running thread num is %d.\n", pEntry->szThreadName, pEntry->dwThreadId,
		s_runMgr.nRunningThreads);
#endif //_DEBUG_RUN_THREAD

	pEntry->dwThreadId = 0;			// release it
}

static RUN_THREAD_START_ARG *Init_ThreadStartArg(IN const char *pszThreadName, 
						IN RUN_THREAD_START_PROC pfnThreadProc,
						IN void *pThreadArg)
{
	int						nEntryId;
	RUN_THREAD_START_ARG		*pArg;
	RUN_THREAD_ENTRY			*pEntry;

	if (pfnThreadProc == NULL)
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_InitThread] -- The thread proc can NOT be NULL!.\n");
#endif //_DEBUG_RUN_THREAD
		return NULL;
	}

	//1. to get a empty thread entry
	nEntryId = RunThread_GetEmptyEntry();
	if (nEntryId < 0)	// -1 for error.
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_InitThread] -- No empty entry to create thread %s.\n",
			pszThreadName);
#endif //_DEBUG_RUN_THREAD

		return NULL;
	}

	// init thread startup arg
	pArg = NEW(RUN_THREAD_START_ARG, 1);
	if (pArg == NULL)
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_InitThread] -- Out of memory on creating"
			"thread %s.\n",	pszThreadName);
#endif //_DEBUG_RUN_THREAD

		return NULL;
	}

	// set the parameters of the run thread entry
	pEntry = &s_runMgr.pThreadEntries[nEntryId];

	if ((pszThreadName == NULL) || (*pszThreadName == '\0'))
	{
		pszThreadName = "__NoName__";
		TRACEX("Warning! Create thread without a name!\n");
	}

	strncpyz(pEntry->szThreadName, pszThreadName, 
		sizeof(pEntry->szThreadName));

	// give 2 heartbeat.
	pEntry->dwThreadHeartbeat = RUN_THREAD_INIT_HEARTBEATS;
	pEntry->dwThreadId = (DWORD)MAKE_THREAD_ID(nEntryId);
	pEntry->nStatus    = RUN_THREAD_IS_RUNNING;

	// init the startup arg
	pthread_mutex_t		 hSyncLockInit = PTHREAD_MUTEX_INITIALIZER;
	pArg->hSyncLock		= hSyncLockInit;
#ifdef _DEBUG_RUN_THREAD
	pArg->pEntry			= pEntry;
#endif //_DEBUG_RUN_THREAD
	pArg->hThread			= (HANDLE)pEntry->dwThreadId;
	pArg->pfnThreadProc		= pfnThreadProc;
	pArg->pThreadArg		= pThreadArg;

	return pArg;
}


static int RunThread_ProcessThreadEvent(IN DWORD dwEvent, IN OUT RUN_THREAD_ENTRY *pEntry)
{
	int rc;
	int nEntry;
	//	RUN_THREAD_MSG msg;

	rc = (int)s_runMgr.pfnEventHandler(dwEvent, 
		(HANDLE)pEntry->dwThreadId,	pEntry->szThreadName);

	switch (rc)
	{
	case THREAD_CONTINUE_RUN:
		return rc;

	case THREAD_CANCEL_THIS:
		{
			nEntry = 1;
		}
		break;

	case THREAD_CANCEL_ALL:
		{
			pEntry = s_runMgr.pThreadEntries;
			nEntry = s_runMgr.nThreadEntries;
		}
		break;

	default:
		return rc;
	}

	//to KILL the threads
	//Jimmy 20210219: do not need cancel it anymore, as the threads should have been already stopped by 'arg-exit'
	//RunThread_CancelThreads(pEntry, nEntry);

	return rc;
}

static void *RunThread_HookEntry(IN RUN_THREAD_START_ARG *pArg)
{
	RUN_THREAD_START_PROC pfnThreadProc = pArg->pfnThreadProc;// start proc
	void		*pThreadArg	= pArg->pThreadArg;	// start arg
	HANDLE	hSelf			= pArg->hThread;
	DWORD	dwExitCode;

#ifdef _DEBUG_RUN_THREAD
	char		szThreadName [LEN_RUN_THREAD_NAME];

	strncpyz(szThreadName, pArg->pEntry->szThreadName, 
		sizeof(szThreadName));

	TRACEX("Thread %s(ThreadID=%p,ProcessID=%d) started. Now total threads is %d\n",
		szThreadName, hSelf, getpid(), s_runMgr.nRunningThreads);
#endif //_DEBUG_RUN_THREAD

	// set the thread is cancelable
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	//set the thread can be canceled immediately
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	// Sync. do NOT continue, till creator work done.
	LOCK_THREAD_SYNC(pArg);

	DESTROY_THREAD_SYNC(pArg);  // remove lock, it's no use.
	DELETE(pArg);				// delete the memory.

	// to run the thread routine now.
	dwExitCode = pfnThreadProc(pThreadArg);

#ifdef _DEBUG_RUN_THREAD
	TRACEX("Thread %s is stopped.\n", szThreadName);
#endif //_DEBUG_RUN_THREAD

	// the thread run done, release the entry to free.
	LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

	if(dwExitCode != THREAD_CONTINUE_RUN)
	{
		RunThread_ProcessThreadEvent(THREAD_EVENT_IS_STOPPED, 
			&s_runMgr.pThreadEntries[GET_THREAD_INDEX(hSelf)]);
	}

	RunThread_ReleaseEntry(&s_runMgr.pThreadEntries[GET_THREAD_INDEX(hSelf)]);

	UNLOCK_THREAD_MANAGER();

	return (void *)dwExitCode;
}

HANDLE RunThread_Create(IN const char *pszThreadName, 
						IN RUN_THREAD_START_PROC pfnThreadProc,
						IN void *pThreadArg, IN OUT DWORD *pdwExitCode)
{
	HANDLE hThread = NULL;

	LOCK_THREAD_MANAGER();	// lock the thread manager data structure.
	//1. get a empty thread entry and init the thread startup argument
	RUN_THREAD_START_ARG *pArg = Init_ThreadStartArg(pszThreadName, pfnThreadProc, pThreadArg);
	if(pArg != NULL)
	{
		RUN_THREAD_ENTRY *pEntry;
		pEntry = &s_runMgr.pThreadEntries[GET_THREAD_INDEX(pArg->hThread)];
		pEntry->pdwExitCode = pdwExitCode;
		//if(pdwExitCode != NULL)
		//{
		//	*pdwExitCode = RUN_THREAD_NO_EXIT;
		//}
		// lock the sync lock at first.
		LOCK_THREAD_SYNC(pArg);
		pEntry->nFailTimes = 0;
		// to create the thread
		if (pthread_create( &pEntry->hSystemThread, 
				NULL,//&attr
				(PTHREAD_START_ROUTINE)RunThread_HookEntry,
				(void *)pArg) == 0)
		{
			hThread = (HANDLE)pEntry->dwThreadId;

			// create the thread OK. detach the thread
			pthread_detach(pEntry->hSystemThread);

			UNLOCK_THREAD_SYNC(pArg); // sync. let thread to go.
			
			TRACE("Thread %s was created, Thread Id = %08x.\n",
				pszThreadName, (UINT)hThread);

			LogOut(MAIN_MODULE_PUBLIB, "ThreadManager", LOG_TYPE_INFO,
				"Thread %s was created, Thread Id = 0x%08x, pThread => %lu.",
				pszThreadName, hThread, pEntry->hSystemThread);
			// the pArg will be deleted in the thread just created,
			// do NOT use pArg any more!
		}
		else
		{
			LogOut(MAIN_MODULE_PUBLIB, "ThreadManager", LOG_TYPE_ERROR,
				"Failed to create thread %s.",
				pszThreadName);
			// release the allocated entry to free
			RunThread_ReleaseEntry(pEntry);

			DESTROY_THREAD_SYNC(pArg);  // remove lock, it's no use.
			DELETE(pArg);
		}
	}

	UNLOCK_THREAD_MANAGER();

	return hThread;
}

static RUN_THREAD_ENTRY *RunThread_GetEntryById(IN HANDLE hThread)
{
	RUN_THREAD_ENTRY *pEntry;
	int				 nEntryIndex;

	if (hThread == NULL)
	{
		return NULL;
	}

	nEntryIndex = GET_THREAD_INDEX(hThread);
	if ((nEntryIndex < 0) || (nEntryIndex >= s_runMgr.nThreadEntries))
	{
		return NULL;
	}

	pEntry = &s_runMgr.pThreadEntries[nEntryIndex];

	if (pEntry->dwThreadId != (DWORD)hThread)
	{
		//it is an invalid thread id.
		return NULL;
	}

	return pEntry;
}

UINT RunThread_Heartbeat(IN HANDLE hThread)
{
	UINT					nBeatCount = 0;
	RUN_THREAD_ENTRY	*pEntry;
	int					nCurStatus;

	//Jimmy 20210311: removed this lock because it is careating block issue on comm-port(though do not know why)
	//LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

	// test the thread id is valid or not
	pEntry = RunThread_GetEntryById(hThread);

	if (pEntry != NULL)
	{
//#define _DEBUG_RUN_THREAD  //Only for test purpose

#ifdef _DEBUG_RUN_THREAD 
		TRACEX("[RunThread_Heartbeat] -- Thread %s(%p) is heartbeating.\n",
			pEntry->szThreadName, (HANDLE)pEntry->dwThreadId);
#endif //_DEBUG_RUN_THREAD

		// increase the thread heartbeat count
		pEntry->dwThreadHeartbeat++;
	
		nBeatCount = pEntry->dwThreadHeartbeat;

//		not need call thread event handler.
//		RunThread_ProcessThreadEvent(THREAD_EVENT_HEARTBEAT, pEntry);
		nCurStatus = pEntry->nStatus;
	}
	else
	{
		nCurStatus = RUN_THREAD_IS_RUNNING;
	}

	//UNLOCK_THREAD_MANAGER();

	if (nCurStatus == RUN_THREAD_TO_QUIT)
	{
		pthread_testcancel();	// test the cancel state....
	}

	return nBeatCount;
}

HANDLE RunThread_GetId(IN const char *pszThreadName)
{
	int				 i;
	RUN_THREAD_ENTRY *p;
	HANDLE			 hThread = NULL;

	//Jimmy removed the lock here 20210218 'cause it can cause deadlock when createThread.
	//LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

	p = s_runMgr.pThreadEntries;

	if (pszThreadName == NULL)	// get the ID of the caller.
	{
		pthread_t hSelf = pthread_self();

		for (i = 0; i < s_runMgr.nThreadEntries; i++, p++)
		{
			if (RUN_THREAD_IS_VALID(p) &&
				(pthread_equal(hSelf, p->hSystemThread) != 0)) // equal
			{
				hThread = (HANDLE)p->dwThreadId;
				break;
			}
		}
	}

	else
	{
		for (i = 0; i < s_runMgr.nThreadEntries; i++, p++)
		{
			if (strcmp(p->szThreadName, pszThreadName) == 0)
			{
				hThread = (HANDLE)p->dwThreadId;
				break;
			}
		}
	}

	//UNLOCK_THREAD_MANAGER();

	return hThread;
}

char *RunThread_GetName(IN HANDLE hThread, OUT char *pszName, IN int nNameLen)
{
	RUN_THREAD_ENTRY *pEntry;

	if (hThread == NULL)
	{
		// get self-name, this call must be located outsize LOCK/UNLOCK.
		hThread = RunThread_GetId(NULL);
	}

	//Jimmy removed the lock here 20210218 'cause it can cause deadlock when createThread.
	//LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

	pEntry = RunThread_GetEntryById(hThread);
	if (pEntry != NULL)
	{
		if (pszName != NULL)
		{
			strncpyz(pszName, (const char *)pEntry->szThreadName, nNameLen);
		}
		else
		{
			pszName = pEntry->szThreadName;
		}
	}
	else	// not found the thread.
	{
		if (pszName != NULL)
		{
			//pszName[0] = 0;
			snprintf(pszName, nNameLen, "%u", (UINT)pthread_self());
		}
		else
		{
			pszName = "";
		}
	}

	//UNLOCK_THREAD_MANAGER();

	return pszName;
}

int RunThread_GetStatus(IN HANDLE hThread)
{
	RUN_THREAD_ENTRY *pEntry;

	//LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

	// test the thread id is valid or not
	pEntry = RunThread_GetEntryById(hThread);

	//UNLOCK_THREAD_MANAGER();
	return (pEntry != NULL) ? pEntry->nStatus : RUN_THREAD_IS_INVALID;
}

int RunThread_GetThreadCount(void)
{
	int n;

	//LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

	n = s_runMgr.nRunningThreads;

	//UNLOCK_THREAD_MANAGER();

	return n;
}

static int RunThread_CancelThreads(IN OUT RUN_THREAD_ENTRY *pEntry,
							IN int nEntry)
{
	int nStatus = ERR_THREAD_STILL_RUNNING;

	if (pEntry == NULL)
	{
		return ERR_THREAD_KILLED;
	}

#ifdef _DEBUG_RUN_THREAD
	TRACEX("Trying to cancel %d threads...\n", nEntry);
#endif //_DEBUG_RUN_THREAD

	for (; nEntry > 0; nEntry--, pEntry++)
	{
		if (RUN_THREAD_IS_VALID(pEntry))
		{
			switch(pthread_cancel(pEntry->hSystemThread))
			{
			case 0:		// ok
#ifdef _DEBUG_RUN_THREAD
				TRACE( "[RunThread_CancelThreads] -- Thread %s(%p) is canceled.\n",
					pEntry->szThreadName, (HANDLE)pEntry->dwThreadId);
#endif //_DEBUG_RUN_THREAD
				//break;	//! No break here

			case ESRCH:	// it has stopped
#ifdef _DEBUG_RUN_THREAD
				TRACE( "[RunThread_CancelThreads] -- Thread %s(%p) is canceled (but ESRCH).\n",
					pEntry->szThreadName, (HANDLE)pEntry->dwThreadId);
#endif //_DEBUG_RUN_THREAD
				RunThread_ReleaseEntry(pEntry);
				nStatus = ERR_THREAD_KILLED;
				break;

			default:
				nStatus = ERR_THREAD_STILL_RUNNING;
			}
		}
	}

	return nStatus;
}

int RunThread_Stop(IN HANDLE hThread, IN int nTimeToWaitThreadQuit, IN BOOL bKillTimeoutedThread)
{
	int				nStatus;
	int				nTimeWaited;
	HANDLE			hKiller = NULL;

	nStatus = RunThread_GetStatus(hThread);

	RUN_THREAD_ENTRY* pEntry = RunThread_GetEntryById(hThread);
	if(pEntry == NULL)
	{
		return ERR_THREAD_OK;
	}

	// the thread is not running. OK.
	if (nStatus == RUN_THREAD_IS_INVALID)
	{
		return ERR_THREAD_OK;
	}

	// the thread is running, send it a quit msg
	//if (nStatus == RUN_THREAD_IS_RUNNING)
	{
		*(pEntry->pdwExitCode) = 1; //RUN_THREAD_NO_EXIT means not exit ,other code means exit!!!
	}

	if (nTimeToWaitThreadQuit >= MAX_WAIT_INTERVAL*1000)
	{
		hKiller = RunThread_GetId(NULL);
	}

	// to test the thread status again. 
	// if the thread quit done, the RUN_THREAD_IS_INVALID will be returned.
	for (nTimeWaited = 0; ; )
	{
		nStatus = RunThread_GetStatus(hThread);
		if (nStatus == RUN_THREAD_IS_INVALID)
		{
			// thread quit done now.
			return ERR_THREAD_OK;
		}

		if (nTimeWaited >= nTimeToWaitThreadQuit)
		{
			// do NOT move this condition to for(;;), 
			// or will lose one time to test the Status after Sleep().
			break;	// timeout
		}

		// wait a while.
#define RUN_THREAD_CHECK_STATUS_INTERVAL		50 /* ms */
		Sleep(RUN_THREAD_CHECK_STATUS_INTERVAL);
		nTimeWaited += RUN_THREAD_CHECK_STATUS_INTERVAL;

		// touch heartbeat
		if ((hKiller != NULL) && (nTimeWaited >= MAX_WAIT_INTERVAL*1000))
		{
			RunThread_Heartbeat(hKiller);
		}
	}

	// the thread is still running. to kill it!
	nStatus = ERR_THREAD_STILL_RUNNING;
	if(bKillTimeoutedThread)
	{
		LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

		nStatus = RunThread_CancelThreads(RunThread_GetEntryById(hThread), 1);

		UNLOCK_THREAD_MANAGER();
	}

	return nStatus;
}

static DWORD RunThread_Manager(IN void *pArgNoUse )
{
	int t;
	RUN_THREAD_ENTRY *pEntry;
	//HANDLE			hMgr;		// the manager itself

	UNUSED(pArgNoUse);

#ifdef _DEBUG_RUN_THREAD
	TRACE("[RunThread_Manager] -- thread manager started.\n");
#endif //_DEBUG_RUN_THREAD
	
	// set the thread is cancelable
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	
	//set the thread can be canceled immediately
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	while (s_runMgr.nStatus == RUN_THREAD_IS_RUNNING)
	{
		t = RUN_THREAD_HEARTBEAT_CHECK_INTERVAL;
		
		// wait for a period before to check heartbeat.
		while ((t > 0) && (s_runMgr.nStatus == RUN_THREAD_IS_RUNNING))
		{
			s_runMgr.pfnEventHandler(THREAD_EVENT_HEARTBEAT, NULL,
				NAME_THREAD_MANAGER);

			Sleep(RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL);
			t -= RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL;
		}

		// now to check the heartbeat of the threads
		LOCK_THREAD_MANAGER();	// lock the thread manager data structure.

		pEntry = s_runMgr.pThreadEntries;
		for (t = 0; t < s_runMgr.nThreadEntries; t++, pEntry++)
		{
			if (RUN_THREAD_IS_VALID(pEntry) && 
				(pEntry->dwThreadHeartbeat == 0)) // no heartbeat!
			{
				pEntry->nFailTimes++;
				TRACEX("Thread[%s] no resp times[%d]\n", pEntry->szThreadName, pEntry->nFailTimes);
				if(pEntry->nFailTimes >= 3)
				{
#ifdef _DEBUG_RUN_THREAD
					TRACE("[RunThread_Manager] -- Thread %s(%p) is no response.\n",
						pEntry->szThreadName, (HANDLE)pEntry->dwThreadId);
#endif //_DEBUG_RUN_THREAD

					RunThread_ProcessThreadEvent(THREAD_EVENT_NO_RESPONSE, 
						pEntry);
				}
			}
			else
			{
				pEntry->nFailTimes = 0;
			}

#ifdef _DEBUG_RUN_THREAD
			TRACE("[RunThread_Manager] -- %d Thread %s(%p) heartbeat = %d.\n", t,
				pEntry->szThreadName, (HANDLE)pEntry->dwThreadId, pEntry->dwThreadHeartbeat);
#endif //_DEBUG_RUN_THREAD
			pEntry->dwThreadHeartbeat = 0;
		}

		UNLOCK_THREAD_MANAGER();
	}

#ifdef _DEBUG_RUN_THREAD
	TRACE("[RunThread_Manager] -- thread manager exited.\n");
#endif //_DEBUG_RUN_THREAD

	s_runMgr.nStatus = RUN_THREAD_IS_INVALID;	// quit done.

	return 0;
}

int RunThread_ManagerInit(IN RUN_THREAD_EVENT_HANDLER pfnEventHandler)
{
#ifdef _DEBUG_RUN_THREAD
	TRACE("[RunThread_ManagerInit] -- Run thread manager is starting...\n");
#endif //_DEBUG_RUN_THREAD

	if (pfnEventHandler == NULL)
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_ManagerInit] -- pfnEventHandler is NULL error.\n");
#endif //_DEBUG_RUN_THREAD

		return ERR_THREAD_INIT_MANAGER;
	}

	s_runMgr.pfnEventHandler = pfnEventHandler;

	if (!RunThread_EnlargeEntries(THREAD_INIT_ENTRY))
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_ManagerInit] -- Fails on init %d thread entries.\n",
			THREAD_INIT_ENTRY);
#endif //_DEBUG_RUN_THREAD

		return ERR_THREAD_INIT_MANAGER;
	}

	s_runMgr.nStatus = RUN_THREAD_IS_RUNNING;

	// use pthread_create instead of RunThread_Create.
	if (pthread_create( &s_runMgr.hManagerThread, 
		NULL,//&attr
		(PTHREAD_START_ROUTINE)RunThread_Manager,
		(void *)&s_runMgr) != 0)
	{
#ifdef _DEBUG_RUN_THREAD
		TRACE("[RunThread_ManagerInit] -- Fails on creating thread manager"
			" thread.\n");
#endif //_DEBUG_RUN_THREAD

		s_runMgr.nStatus = RUN_THREAD_IS_INVALID;
		return ERR_THREAD_INIT_MANAGER;
	}

	// create the thread OK. detach the thread
	pthread_detach(s_runMgr.hManagerThread);

	return ERR_THREAD_OK;
}

static void RunThread_LogPanicThreads(BOOL bNotify)
{
	RUN_THREAD_ENTRY *pEntry;
	int i;

	TRACEX("Panic! There are %d threads are still running before "
		"thread manager exiting...\n",
		s_runMgr.nRunningThreads);

	// this shall never happen!
	pEntry = s_runMgr.pThreadEntries;
	for (i = 0; i < s_runMgr.nThreadEntries; i++, pEntry++)
	{
		if (RUN_THREAD_IS_VALID(pEntry))
		{	
			TRACEX("Panic! The thread %s(%p) is still running.\n",
				pEntry->szThreadName, (HANDLE)pEntry->dwThreadId);

			//	RunThread_ReleaseEntry(pEntry);	// do NOT release!

			if(bNotify)
			{
				s_runMgr.pfnEventHandler(
					THREAD_EVENT_CANNOT_BE_STOPPED,
					(HANDLE)pEntry->dwThreadId,
					pEntry->szThreadName);
			}
		}
	}

	TRACEX("End of showing PANIC threads.\n");
}

static int RunThread_WaitThreadsQuit(IN int nTimeToWaitThreadQuit)
{
	int nTimeWaited = 0;

	// check the running thread in given time.
	while ((s_runMgr.nRunningThreads > 0) 
		&& (nTimeWaited < nTimeToWaitThreadQuit))
	{
		// send manager heartbeat event.
		s_runMgr.pfnEventHandler(THREAD_EVENT_HEARTBEAT, NULL,
			NAME_THREAD_MANAGER);

		Sleep(RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL);

		// increase the waited time.
		nTimeWaited += RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL;
	}

	if (s_runMgr.nRunningThreads > 0)
	{
		return ERR_THREAD_STILL_RUNNING;
	}

	// all threads quit done
	return ERR_THREAD_OK;
}

int RunThread_ManagerExit(IN int nTimeToWaitThreadQuit, IN BOOL bKillTimeoutedThread)
{
	int nResult;

#ifdef _DEBUG_RUN_THREAD
	TRACE("[RunThread_ManagerExit] -- Run thread manager is exiting...\n");
#endif //_DEBUG_RUN_THREAD

	s_runMgr.nStatus = RUN_THREAD_TO_QUIT;	// to quit manager thread.

	// stop all running threads
	// -- NOT need, the threads shall quit done before calling RunThread_ManagerExit().
	//	RunThread_PostQuitMessage((HANDLE)-1);

	s_runMgr.pfnEventHandler(THREAD_EVENT_HEARTBEAT, NULL,
		NAME_THREAD_MANAGER);

	// before canceling threads, post quit message.
	int n = 0;
	RUN_THREAD_ENTRY *pEntry = s_runMgr.pThreadEntries;
	for(n = 0; n < s_runMgr.nThreadEntries; n++, pEntry++)
	{
		if(RUN_THREAD_IS_VALID(pEntry) && (pEntry->pdwExitCode != NULL))
		{
			*(pEntry->pdwExitCode) = 1; //RUN_THREAD_NO_EXIT means not exit ,other code means exit!!!
		}
	}

	// wait threads quit.
	nResult = RunThread_WaitThreadsQuit(nTimeToWaitThreadQuit);
	if (nResult == ERR_THREAD_STILL_RUNNING)
	{
		// to kill the still running threads
		if(bKillTimeoutedThread)
		{
			TRACEX("Can not stop all threads, there are still running threads!\n");
			// to kill the still running threads.
			// we don't hope this happens, 
			// it may cause memory leak and other problems
			LOCK_THREAD_MANAGER();

			TRACEX("now log panic\n");

			RunThread_LogPanicThreads(TRUE);

			TRACEX("now cancel all\n");

			RunThread_CancelThreads(s_runMgr.pThreadEntries,
				s_runMgr.nThreadEntries);
		
			UNLOCK_THREAD_MANAGER();
		}

		nResult = ERR_THREAD_KILLED;
	}

	TRACEX("Waiting for manager to quit...\n");

	// wait for the manager to quit.
	while ((s_runMgr.nStatus != RUN_THREAD_IS_INVALID) &&
		(nTimeToWaitThreadQuit >= 0))	// wait a period at least.
	{
		s_runMgr.pfnEventHandler(THREAD_EVENT_HEARTBEAT, NULL,
			NAME_THREAD_MANAGER);

		Sleep(RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL);
		nTimeToWaitThreadQuit -= RUN_THREAD_MANAGER_HEARTBEAT_INTERVAL;
	}

	// error? cancel it.
	if (s_runMgr.nStatus != RUN_THREAD_IS_INVALID)
	{
		TRACEX("Thread manager is no response. canceled.\n");

		pthread_cancel(s_runMgr.hManagerThread);
		nResult = ERR_THREAD_EXIT_MANAGER;
	}

	TRACEX("Thread manager quited.\n");

	// cleanup the data of the thread manager
	DELETE(s_runMgr.pThreadEntries);
	DESTROY_THREAD_MANAGER();

	s_runMgr.nThreadEntries = 0;
	s_runMgr.pThreadEntries = NULL;
	s_runMgr.nRunningThreads= 0;
	s_runMgr.nLastEmptyEntry= 0;
	s_runMgr.hManagerThread = 0;

#ifdef _DEBUG_RUN_THREAD
	TRACE("[RunThread_ManagerExit] -- Run thread manager is exited.\n");
#endif //_DEBUG_RUN_THREAD

	return nResult;
}
