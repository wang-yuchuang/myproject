/* memory management utility */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>
#include <pthread.h>
#include "base_defines/iNew.h"
#include "base_defines/iLog.h"

static pthread_mutex_t		hMemLock = PTHREAD_MUTEX_INITIALIZER;
//#define LOCK_MEMPOLL_INIT()		pthread_mutex_init(&hMemLock, NULL)
#define LOCK_MEMPOLL_SYNC()		pthread_mutex_lock(&hMemLock)
#define UNLOCK_MEMPOLL_SYNC()	pthread_mutex_unlock(&hMemLock)
#define DESTROY_MEMPOLL_SYNC()	pthread_mutex_destroy(&hMemLock)
//added for memory opt
//#define _TOTAL_MEMORY_BYTES_INCLUDE_FREED  1 //to check the memory peak

static MEMPOOL s_MemPool;

static BYTE* GetIdleBlockMemFromSector(POOL_SECTOR* pSector)
{
	BYTE* pOutMem = NULL;
	if(pSector->nUsedBlocks >= pSector->nBlockTotal)
	{
		return pOutMem;
	}
	POOL_BLOCK *pBlock = pSector->pBlocks;
	UINT n;
	for(n = 0; n < pSector->nBlockTotal; n++, pBlock++)
	{
		if(!pBlock->bInUse)
		{
			pOutMem = pBlock->pMemory;
			pBlock->bInUse = TRUE;
			pSector->nUsedBlocks++;
			break;
		}
	}
	return pOutMem;
}

static void PrintMap(BOOL bNew)
{
#ifdef _DEBUG
	int n;
	TRACE("\n\n------------------------------BEGIN PRINTING MEMORYMAP(%c)----------------------------\n", bNew ? '+' : '-');
	TRACE("MemSize \t\t TotalBlock \t\t UsedBlock \t\t Usage\n");
	for(n = 0; n < POLL_SECTOR_MAX; n++)
	{
		TRACE("%d \t\t %d \t\t %d \t\t %.2f\%\n", s_MemPool.pSector[n].nBlockSize,
			s_MemPool.pSector[n].nBlockTotal,
			s_MemPool.pSector[n].nUsedBlocks,
			(float)s_MemPool.pSector[n].nUsedBlocks * 100 / s_MemPool.pSector[n].nBlockTotal);
	}
	TRACE("-----------------------------END OF PRINTING MEMORYMAP----------------------------\n\n");
#endif
}

BYTE * Pool_Malloc(size_t size)
{
	if(!s_MemPool.bInitOK)
	{
		//LogOut(MAIN_MODULE_GUI, "New", LOG_TYPE_WARING,
		//	"Use malloc for size[%d]!", size);
		return (BYTE*)malloc(size);
	}
	int i;
	BYTE* pFoundMem = NULL;
	LOCK_MEMPOLL_SYNC();
	for (i = 0; i < POLL_SECTOR_MAX; i++)
	{
		if (size <= s_MemPool.pSector[i].nBlockSize)
		{
			pFoundMem = GetIdleBlockMemFromSector(&s_MemPool.pSector[i]);
			//if(pFoundMem == NULL)
			//{
			//	LogOut(MAIN_MODULE_PUBLIB, "New", LOG_TYPE_WARING,
			//		"Memory pool sector [%s] is used up, now use malloc instead!", s_MemPool.pSector[i].szSectorName);
			//}
			break;
		}
	}
	UNLOCK_MEMPOLL_SYNC();

	if(pFoundMem == NULL)
	{
		pFoundMem = (BYTE*)malloc(size);
		//now record this to log file
		static int nLastNewSize[64] = {0};
		static int nRecs = 0;
		BOOL bHasRecord = FALSE;
		int n;
		for(n = 0; n < nRecs; n++)
		{
			if(nLastNewSize[n] == size)
			{
				bHasRecord = TRUE;
				break;
			}
		}

		if(!bHasRecord)
		{
			if(nRecs < 64)
			{
				nLastNewSize[nRecs] = size;
				nRecs++;
				PrintMap(TRUE);
				//Jimmy uncomment it in case of dead-lock
				// LogOut(MAIN_MODULE_PUBLIB, "New", LOG_TYPE_WARING,
				// 	"Can NOT find sector in memPool for size[%d], now use malloc instead!", size);
			}
		}
	}

	//PrintMap(TRUE);

	return pFoundMem;
}

static int GetBlockSize_ByMem(void* pOldMem)
{
	UINT n;
	int i;
	for (i = 0; i < POLL_SECTOR_MAX; i++)
	{
		POOL_BLOCK *pBlock =  s_MemPool.pSector[i].pBlocks;
		for(n = 0; n < s_MemPool.pSector[i].nBlockTotal; n++, pBlock++)
		{
			if(pBlock->bInUse)
			{
				if(pBlock->pMemory == pOldMem)
				{
					return s_MemPool.pSector[i].nBlockSize;
				}
			}
		}
	}
	return 0;
}

BYTE * Pool_Realloc(void* pOldPtr, size_t size)
{
	if(!s_MemPool.bInitOK)
	{
		return NULL;
	}
	int i;
	BYTE* pFoundMem = NULL;
	LOCK_MEMPOLL_SYNC();
	int nOldSize = GetBlockSize_ByMem(pOldPtr);
	if(nOldSize >= size)
	{
		UNLOCK_MEMPOLL_SYNC();
		return pOldPtr;
	}
	for (i = 0; i < POLL_SECTOR_MAX; i++)
	{
		if (size <= s_MemPool.pSector[i].nBlockSize)
		{
			pFoundMem = GetIdleBlockMemFromSector(&s_MemPool.pSector[i]);
			//Jimmy uncomment it 20201221 in case of dead-lock
			// if(pFoundMem == NULL)
			// {
			// 	LogOut(MAIN_MODULE_PUBLIB, "New", LOG_TYPE_WARING,
			// 		"Memory pool sector [%s] is used up, now use malloc instead!", s_MemPool.pSector[i].szSectorName);
			// }
			break;
		}
	}
	UNLOCK_MEMPOLL_SYNC();

	if(pFoundMem == NULL)
	{
		pFoundMem = (BYTE*)malloc(size);
	}
	memcpy(pFoundMem, pOldPtr, nOldSize);
	//Now free the old memory
	Pool_Free(pOldPtr);
	return pFoundMem;
}

void Pool_Free(void *p)
{
	if(!s_MemPool.bInitOK)
	{
		free(p);
		return;
	}

	int i;
	int n;
	BOOL bFoundInPoll = FALSE;
	POOL_BLOCK *pBlock;
	LOCK_MEMPOLL_SYNC();
	for (i = 0; i < POLL_SECTOR_MAX; i++)
	{
		if(s_MemPool.pSector[i].nUsedBlocks > 0)
		{
			//TRACE("Now begingggggggg%d, %d, Total = %d\n", i, s_MemPool.pSector[i].nUsedBlocks, s_MemPool.pSector[i].nBlockTotal);
			pBlock = s_MemPool.pSector[i].pBlocks;
			for(n = 0; n < s_MemPool.pSector[i].nBlockTotal; n++, pBlock++)
			{
				if((void*)pBlock->pMemory == p)
				{
					//TRACE("\nFFFFFFFFFFFFFF\n");
					bFoundInPoll = TRUE;
					pBlock->bInUse = FALSE;
					s_MemPool.pSector[i].nUsedBlocks--;
					UNLOCK_MEMPOLL_SYNC();

					//PrintMap(FALSE);
					return;
				}
			}
		}

	}
	UNLOCK_MEMPOLL_SYNC();

	//do not find in pool, so free here
	//TRACE("\nGGGGGGGGGGG\n");
	//PrintMap(FALSE);
	free(p);
	
}

typedef struct tagMemPoolInitInfo
{
	POLL_SECTYPE	emPollType;
	char			szSectorName[16];
	UINT			nBlockSize;
	UINT			nSetorSize;
}MEMPOLL_INFO;

BOOL MemoryPool_Init()
{
#ifndef MEM_POOL_IMP
	return TRUE;
#endif
	//LOCK_MEMPOLL_INIT();
	s_MemPool.bInitOK = FALSE;
	MEMPOLL_INFO poll_info[] = 
	{
		{POLL_SECTOR_16,		"BYTE-16",		16,		512},
		{POLL_SECTOR_24,		"BYTE-24",		24,		256},
		{POLL_SECTOR_32,		"BYTE-32",		32,		128},
		{POLL_SECTOR_64,		"BYTE-64",		64,		128},
		{POLL_SECTOR_128,		"BYTE-128",	128,		64},
		{POLL_SECTOR_256,		"BYTE-256",	256,		64},
		{POLL_SECTOR_512,		"BYTE-512",	512,		32},
		{POLL_SECTOR_1024,	"BYTE-1024",	1024,	32},
		{POLL_SECTOR_4096,	"BYTE-4096",	4096,	8},
		{POLL_SECTOR_10240,	"BYTE-10240",	10240,	8},
	};
	int n;
	for(n = 0; n < POLL_SECTOR_MAX; n++)
	{
		s_MemPool.pSector[n].pBlocks = NULL;
		s_MemPool.pSector[n].nBlockTotal = 0;
	}	
	POOL_SECTOR *pSector;
	UINT k;
	POOL_BLOCK* pBlock;
	for(n = 0; n < ITEM_OF(poll_info); n++)
	{
		pSector = &s_MemPool.pSector[poll_info[n].emPollType];
		pSector->nBlockSize = poll_info[n].nBlockSize;
		pSector->nUsedBlocks = 0;
		pSector->nBlockTotal = poll_info[n].nSetorSize;
		memcpy(pSector->szSectorName, poll_info[n].szSectorName, 16);
		pSector->pBlocks = malloc(sizeof(POOL_BLOCK) * pSector->nBlockTotal);
		if(pSector->pBlocks == NULL)
		{
			return FALSE;
		}
		pBlock = pSector->pBlocks;
		for(k = 0; k < pSector->nBlockTotal; k++, pBlock++)
		{
			pBlock->pMemory = malloc(pSector->nBlockSize);
			if(pBlock->pMemory == NULL)
			{
				return FALSE;
			}
			pBlock->bInUse = FALSE;
		}
	}
	s_MemPool.bInitOK = TRUE;
	return TRUE;
}

void MemoryPool_Exit()
{
#ifndef MEM_POOL_IMP
	return;
#endif

	int n;
	POOL_SECTOR *pSector;
	UINT k;
	POOL_BLOCK* pBlock;
	for(n = 0; n < POLL_SECTOR_MAX; n++)
	{
		pSector = &s_MemPool.pSector[n];
		if(pSector->pBlocks == NULL)
		{
			continue;
		}
		pBlock = pSector->pBlocks;
		for(k = 0; k < pSector->nBlockTotal; k++, pBlock++)
		{
			if(pBlock->pMemory != NULL)
			{
				free(pBlock->pMemory);
			}
		}
		free(pSector->pBlocks);
		pSector->nBlockTotal = 0;
	}
}
