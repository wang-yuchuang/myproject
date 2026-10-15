#ifndef __WEB_SOCK_EX_H_2018_03_04_DEFINED
#define __WEB_SOCK_EX_H_2018_03_04_DEFINED

#include "3rd_party/nopoll/nopoll.h"

/* 
 * If want to test the module with static mode,
 * please comment out the next line.
 */
#define _MAKE_SHARED_LIB

#ifdef _MAKE_SHARED_LIB

#define WS_CommOpen			    HAL_CommOpen
#define WS_CommAccept			HAL_CommAccept
#define WS_CommRead			    HAL_CommRead
#define WS_CommWrite			HAL_CommWrite
#define WS_CommControl			HAL_CommControl
#define WS_CommClose			HAL_CommClose

#endif //_MAKE_SHARED_LIB



typedef struct stWSDriverData
{
	int					nLastErrorCode;	//	NOTE: MUST BE THE FIRST FIELD! 
										//  the last error code.

	int					nWorkMode;		/* COMM_LOCAL_SERVER,
										 * COMM_CONNECTED_CLIENT,
										 * or COMM_ACCEPTED_CLIENT
										 */

	noPollCtx  			*pCtx; //current noPoll ctx
	noPollConn 			*pConn; //current noPoll conn

	//HANDLE			hMutex; //for internal locking

	COMM_TIMEOUTS		toTimeouts;

// #define WS_RECIEVEBUFF_SIZE	1024*256 //256K
// 	BYTE					byRecieiveBuff[WS_RECIEVEBUFF_SIZE];
// 	int				     	nBytesInRecBuffer;

// #define WS_SENDBUFF_SIZE		4096
// 	BYTE					bySendBuff[WS_SENDBUFF_SIZE];
// 	int					nBytesInSendBuffer;

// 	BYTE*				pCurrReadPos; //for continues reading purpose

 	BOOL					bConnected; //ws connection is ready
	
	DWORD					dwPortAttr;
	//BOOL					bFailed; //failed to connect

	//BOOL					bNeedSendPing;

	//time_t				tmLastRecievedPong;

}WS_DRIVER_DATA;


#endif /* ifndef __NET_SOCK_H_2001_03_04_DEFINED    */

