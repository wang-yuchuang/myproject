#ifndef __NEW_H_
#define __NEW_H_

#include <sys/types.h>
#include <malloc.h>

#include "basetypes.h"

//added for memory opt
#define MEM_POOL_IMP   1
//#define MEM_POOL_DEBUG  1

typedef enum tagPoolSectorType
{
	POLL_SECTOR_16 = 0,
	POLL_SECTOR_24,
	POLL_SECTOR_32,
	POLL_SECTOR_64,
	POLL_SECTOR_128,
	POLL_SECTOR_256,
	POLL_SECTOR_512,
	POLL_SECTOR_1024,
	POLL_SECTOR_4096,
	POLL_SECTOR_10240,
	POLL_SECTOR_MAX
}POLL_SECTYPE;


typedef struct tagPoolBlock
{
	BYTE *pMemory;
	BYTE bInUse;
}POOL_BLOCK;

typedef struct tagPoolSector
{
	char			szSectorName[16]; //just for debugging
	POOL_BLOCK	*pBlocks;
	UINT			nBlockSize;
	UINT			nBlockTotal;
	UINT			nUsedBlocks;
}POOL_SECTOR;

typedef struct tagMemPool
{
	POOL_SECTOR pSector[POLL_SECTOR_MAX];
	BOOL		bInitOK;
}MEMPOOL;

BYTE* Pool_Malloc(size_t size);
BYTE* Pool_Realloc(void* pOldPtr, size_t size);
void Pool_Free(void *p);

BOOL MemoryPool_Init(void);
void MemoryPool_Exit(void);

#ifdef MEM_POOL_IMP
	#define NEW(T, nItems)					\
		(T *)Pool_Malloc((size_t)sizeof(T)*(nItems))

	#define DELETE(p)					\
		Pool_Free( (p) )

	#define RENEW(T, pOldPtr, nNewItems)    \
		(T *)Pool_Realloc((pOldPtr), (size_t)sizeof(T)*(nNewItems))

#else
	#define new_free(p)					free(p)
	#define new_malloc(s)				malloc(s)
	#define new_realloc(ptr, size)		realloc((ptr), (size))

	#define NEW(T, nItems)					\
		(T *)new_malloc( sizeof(T)*(nItems) )

	#define RENEW(T, pOldPtr, nNewItems)    \
		(T *)new_realloc((pOldPtr), sizeof(T)*(nNewItems))

	#define DELETE(p)						new_free(p)
#endif

#define SAFELY_DELETE(p)	\
	do { if ((p) != NULL) { DELETE(p); (p) = NULL; } } while(0)

#endif
