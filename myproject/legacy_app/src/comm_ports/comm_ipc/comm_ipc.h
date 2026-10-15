#ifndef __COMM_IPC_2018_06_20__
#define __COMM_IPC_2018_06_20__

#if defined(linux) || defined(unix)

#include <netinet/in.h>			/* sockaddr_in, sockaddr */
#include <arpa/inet.h>			/* inet_ntoa */
#include <sys/resource.h>		/* setrlimit */
#include <sys/types.h>			/* socket, bind, accept */
#include <sys/socket.h>			/* socket, bind, accept, setsockopt, */
#include <syslog.h>
#include <netdb.h>

#endif

#include "3rd_party/nanomsg/nn.h"
#include "3rd_party/nanomsg/ipc.h"
#include "3rd_party/nanomsg/pipeline.h"
#include "3rd_party/nanomsg/reqrep.h"
#include "3rd_party/nanomsg/pair.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif

#define IPC_CommOpen		HAL_CommOpen
#define IPC_CommAccept		HAL_CommAccept
#define IPC_CommRead		HAL_CommRead
#define IPC_CommWrite		HAL_CommWrite
#define IPC_CommControl		HAL_CommControl
#define IPC_CommClose		HAL_CommClose


typedef struct SIPCPortDriver					/*	IPC Port HAL driver			*/
{
	int					nLastErrorCode;	//	NOTE: MUST BE THE FIRST FIELD! 
										//  the last error code.

	char					szIPC_File_RR[64];		//example: ipc:///app/data/test.ipc.RR
	char					szIPC_File_PP[64];		//example: ipc:///app/data/test.ipc.PP

	int					nWorkMode;		/* COMM_LOCAL_SERVER,
										 * COMM_CONNECTED_CLIENT,
										 */
	int					nIPCMode;			//0--req-rep, 1--push-pull
	int					nSocket_RR;		//socket for req-rep
	int					nSocket_PP;		//socket for push-pull
	COMM_TIMEOUTS		toTimeouts;
}IPC_PORT_DRV;


#endif

