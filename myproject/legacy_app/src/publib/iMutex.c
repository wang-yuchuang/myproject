
#define _XOPEN_SOURCE	600

#include "std_sys.h"

#ifndef __USE_XOPEN2K
//#error "__USE_XOPEN2K" shall be defined at first!
#endif

#ifdef _DEBUG
//#define _DEBUG_MUTEX	1
#endif
#include <semaphore.h>

#include "base_defines/basetypes.h"
#include "base_defines/iNew.h"
#include "base_defines/basefunc.h"
#include "base_defines/iThread.h"
#include "base_defines/iMutex.h"

typedef struct tagMutex
{				
	pthread_mutex_t	hInternalMutex;	//	the mutex lock
//	pthread_cond_t	hInternalCond;	//	the waitable condition
}MUTEX;	

HANDLE Mutex_Create(IN BOOL bOpenedLock)
{
	MUTEX			*pMutex = NEW( MUTEX, 1 );
	pthread_mutex_t mut		= PTHREAD_MUTEX_INITIALIZER;

	if (pMutex == NULL)
	{
#ifdef _DEBUG_MUTEX
	TRACE("[Mutex_Create] -- out of memory on new a MUTEX.\n");
#endif //_DEBUG_MUTEX

		return NULL;
	}

	pMutex->hInternalMutex = mut;
	
	// To lock the mutext if want to created a locked mutext
	if (!bOpenedLock)
	{
		Mutex_Lock((HANDLE)pMutex, 0);
	}
	
	return (HANDLE)pMutex;
}

#ifdef __USE_XOPEN2K
int Mutex_Lock(IN HANDLE hm, IN DWORD dwWaitTimeout)
{
	MUTEX	*pMutex = (MUTEX *)hm;
	struct	timespec ts;
	long	nWaitSeconds, nWaitNonaseconds;
	time_t	tmEndTime;
	
	if (hm == NULL)
	{
		return ERR_MUTEX_INVALID_ARG;
	}

	nWaitSeconds		= (long)MS_TO_SEC(dwWaitTimeout);			// sec
	nWaitNonaseconds	= (long)MS_TO_NS(dwWaitTimeout%MS_PER_SEC);	// nonasec

	clock_gettime(CLOCK_REALTIME, &ts);	// get the start time.
	
	ts.tv_nsec += nWaitNonaseconds;
	if (ts.tv_nsec > NS_PER_SEC)
	{
		ts.tv_sec  += 1;
		ts.tv_nsec -= NS_PER_SEC;
	}

	tmEndTime = ts.tv_sec;

#ifdef _DEBUG_MUTEX
	TRACE("[Mutex_Lock] -- lock and wait %d ms at %d\n", 
		(int)dwWaitTimeout, (int)tmEndTime);
#endif //_DEBUG_MUTEX

	while (nWaitSeconds >= 0)
	{
		// get the end of the real time.
		tmEndTime += ((nWaitSeconds < MAX_WAIT_INTERVAL) ? nWaitSeconds
			: MAX_WAIT_INTERVAL);

		ts.tv_sec = tmEndTime;
		// do NOT change ts.tv_nsec.

		if (pthread_mutex_timedlock(&pMutex->hInternalMutex, &ts) == 0)
		{
			return ERR_MUTEX_OK;
		}

		nWaitSeconds -= MAX_WAIT_INTERVAL;
		if (nWaitSeconds >= 0)
		{
			RUN_THREAD_HEARTBEAT();
		}
	}

#ifdef _DEBUG_MUTEX
	TRACE("[Mutex_Lock] -- lock timeouted at %d\n", 
		(int)time(NULL));
#endif //_DEBUG_MUTEX

	// if goes here, timeout on trying to lock.
	return ERR_MUTEX_TIMEOUT;
}

#else


#define WAIT_LOCK_INTERVAL	10000	// 10 ms modified by Jimmy 20180104 for performance improving

int Mutex_Lock(IN HANDLE hm, IN DWORD dwWaitTimeout)
{
	MUTEX	*pMutex = (MUTEX *)hm;
	double	tEnd, tLast, tNow;

	if (hm == NULL)
	{
		return ERR_MUTEX_INVALID_ARG;
	}

#ifdef _DEBUG_MUTEX
	TRACE("[Mutex_Lock] -- lock and wait %d ms at %d\n", 
		(int)dwWaitTimeout, (int)time(NULL));
#endif //_DEBUG_MUTEX

	tEnd = GetCurrentTime() + (double)dwWaitTimeout/1000.0;

	int nWaitUSec = WAIT_LOCK_INTERVAL;
	if(dwWaitTimeout <= 10)
	{
		nWaitUSec = 1000; //1ms
	}

	//added by Jimmy 20190404 just in case time changed within waiting process
	int nMaxWaitTimes = ((int)dwWaitTimeout * 1000 / nWaitUSec) + 1;
	int nHasWaitTimes = 0;

	//for(;;)
	for (nHasWaitTimes = 0; nHasWaitTimes < nMaxWaitTimes; nHasWaitTimes++)
	{
		if (pthread_mutex_trylock(&pMutex->hInternalMutex) == 0)
		{
			return ERR_MUTEX_OK;
		}

		// to test timeout.
		tNow = GetCurrentTime();
		if (tNow >= tEnd)
		{
			break;
		}

		// trigger heartbeart
		if (((int)tNow - (int)tLast) >= MAX_WAIT_INTERVAL)
		{
			RUN_THREAD_HEARTBEAT();
			tLast = tNow;
		}
			
		// sleep a while to try, always use the interval 10 ms, Jimmy note, shall wait 1ms if wait interval < 10ms
		usleep(nWaitUSec);	// unit is us
	}

#ifdef _DEBUG_MUTEX
	TRACE("[Mutex_Lock] -- lock timeouted at %d\n", 
		(int)time(NULL));
#endif //_DEBUG_MUTEX

	return ERR_MUTEX_TIMEOUT;
}
#endif

void Mutex_Unlock(HANDLE hm)
{
	if (hm != NULL)
	{
		pthread_mutex_unlock(&((MUTEX *)hm)->hInternalMutex);
	}
	//sched_yield(); //该函数让当前线程暂时让出CPU，避免此线程持续拥有该锁/**/
}

void Mutex_Destroy(IN HANDLE hm)
{
	if (hm != NULL)
	{
		pthread_mutex_destroy(&((MUTEX *)hm)->hInternalMutex);
		DELETE( hm );
	}
}


typedef struct tagSemahorne
{				
	sem_t	hInternalSem;
}SEMAPHORE;	

HANDLE Sem_Create(IN int  nInitValue)
{
	SEMAPHORE	*pSem = NEW(SEMAPHORE, 1);

	if (pSem == NULL)
	{
#ifdef _DEBUG_MUTEX
		TRACE("[Sem_Create] -- out of memory on new a SEMAPHORE.\n");
#endif //_DEBUG_MUTEX

		return NULL;
	}

	// no share, start count is 0 
	sem_init(&pSem->hInternalSem, 0, (unsigned int)nInitValue);
	
	return (HANDLE)pSem;
}


HANDLE Sem_CreateEx(char* pszName)
{
	SEMAPHORE	*pSem = NEW(SEMAPHORE, 1);

	if (pSem == NULL)
	{
#ifdef _DEBUG_MUTEX
		TRACE("[Sem_Create] -- out of memory on new a SEMAPHORE.\n");
#endif //_DEBUG_MUTEX

		return NULL;
	}

	sem_t* psem_t =  sem_open(pszName, O_CREAT,0666, 0);//信号量初始值为0
	if(psem_t == SEM_FAILED || psem_t == NULL)
	{
		return NULL;
	}
	pSem->hInternalSem = *psem_t;
	//sem_init(&pSem->hInternalSem, 0, (unsigned int)nInitValue);

	return (HANDLE)pSem;
}

#ifdef __USE_XOPEN2K
int Sem_Wait(IN HANDLE hSem, IN DWORD dwWaitTimeout)
{
	SEMAPHORE	*pSem = (SEMAPHORE *)hSem;
	struct	timespec ts;
	long	nWaitSeconds, nWaitNonaseconds;
	//time_t	tmEndTime;
	
	if (hSem == NULL)
	{
		return ERR_MUTEX_INVALID_ARG;
	}

	nWaitSeconds		= (long)MS_TO_SEC(dwWaitTimeout);			// sec
	nWaitNonaseconds	= (long)MS_TO_NS(dwWaitTimeout%MS_PER_SEC);	// nonasec

//	clock_gettime(CLOCK_REALTIME, &ts);	// get the start time.
//	
//	ts.tv_nsec += nWaitNonaseconds;
//	if (ts.tv_nsec > NS_PER_SEC)
//	{
//		ts.tv_sec  += 1;
//		ts.tv_nsec -= NS_PER_SEC;
//	}
//
//	tmEndTime = ts.tv_sec;

//#endif

#ifdef _DEBUG_MUTEX
	TRACE("[Sem_Wait] -- %p wait %d ms at %d\n", 
		RunThread_GetId(NULL), (int)dwWaitTimeout, (int)tmEndTime);
#endif //_DEBUG_MUTEX

	while (nWaitSeconds >= 0)
	{
		//new code for correcting the dead-error while time changed
		clock_gettime(CLOCK_REALTIME, &ts);	// get the start time.

		// adjust ns, do NOT change ts.tv_nsec if nWaitNonaseconds == 0.
		if (nWaitNonaseconds > 0)
		{
			ts.tv_nsec += nWaitNonaseconds;
			if (ts.tv_nsec > NS_PER_SEC)
			{
				ts.tv_sec  += 1;
				ts.tv_nsec -= NS_PER_SEC;
			}
	
			nWaitNonaseconds = 0;	// clear the ns
		}

 		// adjust the end-time of ts.tv_sec
		ts.tv_sec += ((nWaitSeconds < MAX_WAIT_INTERVAL) ? nWaitSeconds
			: MAX_WAIT_INTERVAL);

//		// get the end of the real time.
//		tmEndTime += ((nWaitSeconds < MAX_WAIT_INTERVAL) ? nWaitSeconds
//			: MAX_WAIT_INTERVAL);
//
//		ts.tv_sec = tmEndTime;
//		// do NOT change ts.tv_nsec.

		if (sem_timedwait(&pSem->hInternalSem, &ts) == 0)
		{
			return ERR_MUTEX_OK;//wait ok
		}

		//continue to try to wait.
		nWaitSeconds -= MAX_WAIT_INTERVAL;
		if (nWaitSeconds >= 0)
		{
			RUN_THREAD_HEARTBEAT();
		}
	}

#ifdef _DEBUG_MUTEX
	TRACE("[Sem_Wait] -- wait timeouted at %d\n", 
		(int)time(NULL));
#endif //_DEBUG_MUTEX

	// if goes here, timeout on trying to lock.
	return ERR_MUTEX_TIMEOUT;
}

#else

int Sem_Wait(IN HANDLE hSem, IN DWORD dwWaitTimeout)
{
	SEMAPHORE	*pSem = (SEMAPHORE *)hSem;
	double	tEnd, tLast, tNow;
	HANDLE	hSelf;

#define WAIT_LOCK_INTERVAL	10000	// 10 ms

	if (hSem == NULL)
	{
		return ERR_MUTEX_INVALID_ARG;
	}

#ifdef _DEBUG_MUTEX
	TRACE("[Sem_Wait] -- %p wait %d ms at %d\n", 
		RunThread_GetId(NULL), (int)dwWaitTimeout, (int)time(NULL));
#endif //_DEBUG_MUTEX

	tEnd = GetCurrentTime() + (double)dwWaitTimeout/1000.0;

	//added by Jimmy 20190404 just in case time changed within waiting process
	int nMaxWaitTimes = ((int)dwWaitTimeout * 1000 / WAIT_LOCK_INTERVAL) + 1;
	int nHasWaitTimes = 0;

	//for(;;)
	for (nHasWaitTimes = 0; nHasWaitTimes < nMaxWaitTimes; nHasWaitTimes++)
	{
		RunThread_Heartbeat(hSelf);

		if (sem_trywait(&pSem->hInternalSem) == 0)
		{
			return ERR_MUTEX_OK;//lock ok
		}

		//continue to try to lock.

		// to test timeout.
		tNow = GetCurrentTime();
		if (tNow >= tEnd)
		{
			break;
		}

		// trigger heartbeart
		TRACE("tLast1 is %d!!!\n", tLast);
		if (((int)tNow - (int)tLast) >= MAX_WAIT_INTERVAL)
		{
			RUN_THREAD_HEARTBEAT();
			tLast = tNow;
		}
			
		// sleep a while to try, always use the interval 20 ms
		usleep(WAIT_LOCK_INTERVAL);	// unit is us
	}

#ifdef _DEBUG_MUTEX
	TRACE("[Sem_Wait] -- wait timeouted at %d\n", 
		(int)time(NULL));
#endif //_DEBUG_MUTEX

	return ERR_MUTEX_TIMEOUT;
}

#endif


void Sem_Post(HANDLE hSem, int nPostCount)
{
	if (hSem != NULL)
	{
		while (nPostCount-- > 0)
		{
			if (sem_post(&((SEMAPHORE *)hSem)->hInternalSem) != 0)
			{
				break;
			}
		}
	}
}

void Sem_Destroy(IN HANDLE hSem)
{
	if (hSem != NULL)
	{
		sem_destroy(&((SEMAPHORE *)hSem)->hInternalSem);
		DELETE( hSem );
	}
}

//system-v

int Creat_SemID(const char *pathname,int proj_id,int nsems ,int semflg)
{
	key_t key = ftok(pathname, proj_id);
	int semid = semget(key, nsems, semflg);
	return semid;
}

int Init_Sem(int semID,int nsems,int cmd,int val)
{
	
	int Ret = semctl(semID,nsems,cmd,val);  
	return Ret;
}

void sem_wait_op(int semid) 
{
    struct sembuf sop = {0, -1, 0};
    semop(semid, &sop, 1);
}

void sem_post_op(int semid) 
{
    struct sembuf sop = {0, 1, 0};
    semop(semid, &sop, 1);
}
