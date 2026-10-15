#ifndef __RUN_QUEUE_H__
#define __RUN_QUEUE_H__

#define ERR_QUEUE_OK			0
#define ERR_QUEUE_FULL			1
#define ERR_QUEUE_EMPTY		2
#define ERR_QUEUE_INVALID_ARG	3
#define ERR_QUEUE_NO_MEMORY	4

HANDLE Queue_Create( IN int nMaxCapacity, IN int nElementSize,
					 IN int nIncStep);
int Queue_Put( HANDLE hq, void *pElement, BOOL bUrgent );
int Queue_GetCount(IN HANDLE hq, OUT int *pMaxElements);
int Queue_Get(HANDLE hq, void *pElement, BOOL bPeek, DWORD dwTimeout);
int Queue_GetEx(HANDLE hq, int nWhich, void *pElement, BOOL bPeek);
void Queue_Empty(HANDLE hq);
void Queue_Destroy( HANDLE hq);

#endif
