
#ifndef __COMM_PORT_H__
#define __COMM_PORT_H__

#include "base_defines/basetypes.h"

/* 
 *  The following are the API to access communication ports:
 */

#if defined(linux) || defined(unix)

#include <netinet/in.h>			/* sockaddr_in, sockaddr */
#include <arpa/inet.h>			/* inet_ntoa */
#include <sys/resource.h>		/* setrlimit */
#include <sys/types.h>			/* socket, bind, accept */
#include <sys/socket.h>			/* socket, bind, accept, setsockopt, */
// #include <syslog.h>
#include <netdb.h>

#endif
struct SCommTimeouts
{				
	int	nReadTimeout;	/* The total timeout on reading. -1: wait for ever	*/
	int	nWriteTimeout;	/* The total timeout on writing. -1: wait for ever	*/

	/* timeout between 2 chars after first 1 bytes sent out or received		*/
#define TIMEOUT_INTERVAL_CHAR	50 /*ms, minimum BPS is 1000/(50/10)=20bps	*/
	int	nIntervalTimeout;	/* the timeout between 2 char.					*/
};
typedef struct SCommTimeouts		COMM_TIMEOUTS;

#ifndef WIN32
/*
 * The new type to be used in all
 * instances which refer to sockets.
 */
typedef int           SOCKET;

#endif
struct SUdpPortDriver					/*	UDP Port HAL driver			*/
{				
	int					nLastErrorCode;	//	NOTE: MUST BE THE FIRST FIELD! 
										//  the last error code.

	int					nWorkMode;		/* COMM_LOCAL_SERVER,
										 * COMM_CONNECTED_CLIENT,
										 */
	int					nSockPort;
	DWORD				dwHostAddr;		/* local host addr, addr of myself	*/
	
	struct sockaddr_in	inPeerAddr;
	
	struct sockaddr_in	server_addr;
	struct sockaddr_in	client_addr;
	struct sockaddr_in	send_addr;

	SOCKET				nSocket;
	COMM_TIMEOUTS		toTimeouts;

	int					nMaxClients;
	int					*pCurClients;
};				
typedef struct SUdpPortDriver		UDP_PORT_DRV;
enum COMM_ERROR
{
	ERR_COMM_OK,
	ERR_COMM_NO_MEMORY,
	ERR_COMM_TIMEOUT,
	ERR_COMM_OPENING_PORT,
	ERR_COMM_OPENING_PARAM,
	ERR_COMM_SUPPORT_ACCEPT,
	ERR_COMM_MANY_CLIENTS,
	ERR_COMM_ACCEPT_FAILURE,
	ERR_COMM_READ_DATA,
	ERR_COMM_WRITE_DATA,
	ERR_COMM_CONNECT_SERVER,
	ERR_COMM_CREATE_SERVER,
	ERR_COMM_PORT_HANDLE,
	ERR_COMM_CTRL_COMMAND,
	ERR_COMM_CTRL_PARAMS,
	ERR_COMM_CTRL_EXECUTION,
	ERR_COMM_BUFFER_SIZE,
	ERR_COMM_CONNECTION_BROKEN,
	ERR_COMM_STD_PORT,
	ERR_COMM_WRONG_IPC_MODE,
	ERR_COMM_HTTPFTP_PENDING,
};

#define COMM_SERVER_MODE	0x00
#define COMM_CLIENT_MODE	0x01

//for ws
#define COMM_OCPP1_6_MODE		0x00
#define COMM_INFYMBE_MODE		0x01
#define COMM_OCPP2_0_1_MODE		0x02
#define COMM_BINARY_MODE		0x03

//for HTTP/FTP only
#define COMM_HTTP_MODE	0x10
#define COMM_HTTPS_MODE	0x20
#define COMM_FTP_MODE		0x30
#define COMM_FTPS_MODE	0x40

typedef enum tagCommCanBaud
{
	COMM_CAN_BAUDRATE_50K,
	COMM_CAN_BAUDRATE_100K,
	COMM_CAN_BAUDRATE_125K,
	COMM_CAN_BAUDRATE_250K,
	COMM_CAN_BAUDRATE_500K,
	COMM_CAN_BAUDRATE_800K,
	COMM_CAN_BAUDRATE_1M
}COMM_CAN_BAUDRATE;

//The following structs are compilable with <Linux/can.h> canframe definition
typedef struct tagCommCanFrame
{
	UINT		ulCanID;
	BYTE		nDataLen;
	BYTE		byData[8];
}COMM_CANFRAME;

typedef struct tagCommCanFilter
{
	UINT		ulCANId;
	UINT		ulCANMask;
}COMM_CAN_FILTER;

/* the work mode of the comm port */
enum COMM_WORK_MODE_ENUM
{
	COMM_LOCAL_SERVER		= 0,// The local server port
	COMM_OUTGOING_CLIENT	= 1,// Client port to connect to remote server
	COMM_INCOMING_CLIENT	= 2	// Client port has connected to local server
};


#define IPC_MODE_REQ_REP	0
#define IPC_MODE_PUSH_PULL	1

// get the last error of the port. 
// in the internal, nErr = hPort->hInstance->nLastErrorCode;
#define CommGetLastError(hPort)		(**(int **)(hPort))


/* The control commands: nCmd */
enum COMM_CONTROL_COMMAND_ENUM
{
	COMM_GET_LAST_ERROR         = 1,   /* get the last error code */
	COMM_GET_PEER_INFO          = 2,   /* get the sender info, 
											bBuffer=COMM_PEER_INFO, 
											nDataLength=sizeof(COMM_PEER_INFO)*/
	COMM_SET_TIMEOUTS           = 3,   /* set the comm timeout in ms. 
											bBuffer=COMM_TIMEOUTS, 
											nDataLength=sizeof(COMM_TIMEOUTS)*/
	COMM_GET_TIMEOUTS           = 4,   /* get the comm timeout in ms
											bBuffer=COMM_TIMEOUTS, 
											nDataLength=sizeof(COMM_TIMEOUTS)*/
	COMM_PURGE_TXCLEAR          = 5,	  /* clear the transmit buffer */
	COMM_PURGE_RXCLEAR          = 6,   /* clear the receive buffer */
	COMM_SET_MAX_CLIENTS        = 7,   /* set the maximum allowed clients */
	COMM_GET_MAX_CLIENTS        = 8,   /* get the maximum allowed clients */
	COMM_GET_CUR_CLIENTS        = 9,   /* get the number of the current 
											connected clients */
	COMM_GET_CLIENT_LIMITATION  = 10,  /* get the capacity maximum of 
											* the client connections */
	COMM_SET_SERIAL_ATTRIB,  //set attribute, e,g 9600,n,8,1
	COMM_SET_SERIAL_485_To232, // LJH241123
	COMM_SET_SERIAL_485,
		
	COMM_GET_CAN_BAUDRATE,
	COMM_SET_CAN_BAUDRATE,
	COMM_SET_CAN_FILTER,
	COMM_RESET_CAN, //reinitialize CAN bus

	COMM_SET_IPC_MODE, //set IPC mode, just for IPC

	COMM_GET_HTTPFTP_PROCESS, //get download/upload process

	COMM_SEND_WS_PING, //send PING so that OCPP server can know it is alive

	COMM_GET_LASTTIME_RECEIVE_PONG, //Get last time that recieved pong from OCPP server

	COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE, //Set global TLS root CA file for libwebsocket

	COMM_PRIVATE_COMMAND		= 255  /* >255:defined by the HAL comm drv*/
};

/* the comm peer info */
#define MAX_LEN_PEER_ADDR	32
struct SCommPeerInfo
{
	int		nPeerType;
	int		nPeerPort;
	char	szAddr[MAX_LEN_PEER_ADDR];	/* the addr of peer:
						  1)IP: for TCP/IP and UDP. IP=*(DWORD *)szAddr
						  2)Phone Number:for dialling port(depends on modem)*/
};
typedef struct SCommPeerInfo		COMM_PEER_INFO;

struct SCommUDPSetting
{
	COMM_PEER_INFO sPeerInfo;
	BOOL 		   nNonBlock;
	
};
typedef struct SCommUDPSetting		COMM_UDP_SET;
/* the struct of timeout */
// struct SCommTimeouts
// {				
// 	int	nReadTimeout;	/* The total timeout on reading. -1: wait for ever	*/
// 	int	nWriteTimeout;	/* The total timeout on writing. -1: wait for ever	*/

// 	/* timeout between 2 chars after first 1 bytes sent out or received		*/
// #define TIMEOUT_INTERVAL_CHAR	50 /*ms, minimum BPS is 1000/(50/10)=20bps	*/
// 	int	nIntervalTimeout;	/* the timeout between 2 char.					*/
// };
// typedef struct SCommTimeouts		COMM_TIMEOUTS;

#define INIT_TIMEOUTS(to, rto, wto)	((to).nReadTimeout = (rto),		\
									 (to).nWriteTimeout = (wto),	\
									 (to).nIntervalTimeout = TIMEOUT_INTERVAL_CHAR)


#define INIT_TIMEOUTS_EX(to, rto, wto, ito)	((to).nReadTimeout = (rto), \
									 (to).nWriteTimeout = (wto),		\
									 (to).nIntervalTimeout = (ito))

/* do NOT directly use the following interfaces */
/* The open proc named "CommOpen" shall be provided by each HAL comm driver */
typedef HANDLE (*COMM_OPEN_PROC)
(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD		dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode
);

/* The "CommAccept" proc for comm driver */
typedef HANDLE (*COMM_ACCEPT_PROC)
(				
	IN HANDLE	hPort
);

/* The "CommRead" proc for comm driver */
typedef int (*COMM_READ_PROC)
(				
	IN HANDLE	hPort,		
	OUT char		*pBuffer,
	IN int		nBytesToRead
);				

/* The "CommWrite" proc for comm driver */
typedef int (*COMM_WRITE_PROC)
(				
	IN HANDLE	hPort,
	IN char		*pBuffer,
	IN int		nBytesToWrite
);				

/* The "CommControl" proc for comm driver */
typedef int (*COMM_CONTROL_PROC)
(				
	IN HANDLE	hPort,
	IN int		nCmd,
	IN OUT void	*pBuffer,
	IN int		nDataLength
);
//Return:	0: OK


/* The "CommClose" proc for comm driver */
typedef int (*COMM_CLOSE_PROC)
(
	IN HANDLE	hPort
);

/* 
 * The following protocols are used to develop HAL communication port drivers,
 * The application service shall NOT call them directly
 */
HANDLE HAL_CommOpen( IN char *pPortDescriptor, IN char *pOpenParams,
	IN DWORD dwPortAttr, IN int nTimeout, OUT int *pErrCode );
HANDLE HAL_CommAccept( IN HANDLE hPort );
int HAL_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead);
int HAL_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite);
int HAL_CommControl(	IN HANDLE hPort, IN int	nCmd, 
	IN OUT void *pBuffer,	IN int	nDataLength);
int HAL_CommClose( IN HANDLE hPort );

/* 
 * Define the export function names(symbole name) for 
 * all HAL communication drivers
 */
#define SYM_HAL_COMMOPEN		"HAL_CommOpen"
#define SYM_HAL_COMMACCEPT		"HAL_CommAccept"
#define SYM_HAL_COMMREAD		"HAL_CommRead"
#define SYM_HAL_COMMWRITE		"HAL_CommWrite"
#define SYM_HAL_COMMCONTROL	"HAL_CommControl"
#define SYM_HAL_COMMCLOSE		"HAL_CommClose"

typedef struct tagCommPortInfo
{
	int		nCommPort;
	char		szPortName[32];
	BOOL		bIsExlusive;
	char		szPortAttri[64];
	char		szPortDriver[64];
}COMMPORT_INFO;

typedef struct tagCommPort
{
	COMMPORT_INFO		info;

	COMM_OPEN_PROC		Open;
	COMM_CLOSE_PROC		Close;
	COMM_ACCEPT_PROC		Accept;
	COMM_READ_PROC		Read;
	COMM_WRITE_PROC		Write;
	COMM_CONTROL_PROC	Control;

	BOOL					bInUsed;
	HANDLE				hLib;

}COMMPORT;

// LJH241101 MU3
#define TTYS_ALIAS_NAME			"COM"
#define CONST_STRLEN(const_str)		(sizeof(const_str)-1)

#define TTYS_USB2                   "/dev/ttyUSB2"
#ifdef IMMU3
	#define TTYS_ONLY_NAME				"ttyO"
	#define TTYS_FULL_PREFIX			"/dev/ttyO"
	#define TTYS_RS485_1                "/dev/ttyO3"
	#define TTYS_IEC101                 "/dev/ttyO1"
#else
	#define TTYS_ONLY_NAME				"ttyS"
	#define TTYS_FULL_PREFIX			"/dev/ttyS"
	#define TTYS_RS485_1                "/dev/ttyS3"
	#define TTYS_IEC101                 "/dev/ttyS1"
#endif

#endif /*__HALCOMM_H__*/
