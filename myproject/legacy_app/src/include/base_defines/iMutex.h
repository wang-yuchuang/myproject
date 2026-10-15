#ifndef __RUN_MUTEX_H__
#define __RUN_MUTEX_H__

#define ERR_MUTEX_OK			0
#define ERR_MUTEX_TIMEOUT		1
#define ERR_MUTEX_INVALID_ARG	2

#define WAIT_INFINITE	((unsigned long)-1)

#include <sys/ipc.h>
#include <sys/sem.h>
#define SEM_CCU_485_PATH "/var"  
#define SEM_CCU_485_PROJ_ID  'S'
HANDLE Mutex_Create(IN BOOL bOpenedLock);
int Mutex_Lock(IN HANDLE hm, IN DWORD dwWaitTimeout);
void Mutex_Unlock(HANDLE hm);
void Mutex_Destroy(IN HANDLE hm);

HANDLE Sem_Create(IN int  nInitValue);
HANDLE Sem_CreateEx(char* pszName);
int Sem_Wait(IN HANDLE hSem, IN DWORD dwWaitTimeout);
void Sem_Post(HANDLE hSem, int nPostCount);
void Sem_Destroy(IN HANDLE hSem);

//system-v
int Creat_SemID(const char *pathname,int proj_id,int nsems ,int semflg);
int Init_Sem(int semID,int nsems,int cmd,int val);
void sem_wait_op(int semid);
void sem_post_op(int semid);
#endif

