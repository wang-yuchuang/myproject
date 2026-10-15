#ifndef __NET_SOCK_UDP_2001_03_04_DEFINED
#define __NET_SOCK_UDP_2001_03_04_DEFINED

#if defined(linux) || defined(unix)

#include <netinet/in.h>			/* sockaddr_in, sockaddr */
#include <arpa/inet.h>			/* inet_ntoa */
#include <sys/resource.h>		/* setrlimit */
#include <sys/types.h>			/* socket, bind, accept */
#include <sys/socket.h>			/* socket, bind, accept, setsockopt, */
#include <syslog.h>
#include <netdb.h>

#endif

/* 
 * If want to test the module with static mode,
 * please comment out the next line.
 */
#define _MAKE_SHARED_LIB

#ifdef _MAKE_SHARED_LIB

#define UDP_CommOpen			HAL_CommOpen
#define UDP_CommAccept			HAL_CommAccept
#define UDP_CommRead			HAL_CommRead
#define UDP_CommWrite			HAL_CommWrite
#define UDP_CommControl			HAL_CommControl
#define UDP_CommClose			HAL_CommClose

#endif //_MAKE_SHARED_LIB


#ifndef WIN32
/*
 * The new type to be used in all
 * instances which refer to sockets.
 */
typedef int           SOCKET;

#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR    -1
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif

#ifndef SD_BOTH
#define SD_BOTH         2
#endif

#if defined(WIN32)

#define CLOSE_SOCKET(sock)  closesocket(sock)
#define SC_SOCKADDR         sockaddr
#define TCP                 6
#define UDP                 17
#define E_INPROGRESS        WSAEWOULDBLOCK
#define SOCK_ERRNO()        WSAGetLastError()
#endif

#ifdef unix
#define SC_SOCKADDR         sockaddr
#define SOCK_ERRNO()        errno
#define E_INPROGRESS        EINPROGRESS
#define CLOSE_SOCKET(sock)  close(sock)
#endif

#if defined(_PSOS)  || defined(unix)
#define SOCK_ERRNO()        errno
#define E_INPROGRESS        EINPROGRESS
#define CLOSE_SOCKET(sock)  close(sock)
#endif


/**********************************/
#define UDP_MAX_CLIENTS_DEFAULT		10	/* The default UDP connections	*/
#define UDP_MAX_CLIENT_LIMITAION	64	/* It's enough for NCCU				*/

#define CLIENT_PORT					10000
#define UDP_BROADCAST_ADDR			"192.168.1.255"
//#define BROADCAST_ADDR			"255.255.255.255" Not Used This Addr Yet.
#define BROADCAST_ADDR_ZERO 		"0.0.0.0"

// struct SUdpPortDriver					/*	UDP Port HAL driver			*/
// {				
// 	int					nLastErrorCode;	//	NOTE: MUST BE THE FIRST FIELD! 
// 										//  the last error code.

// 	int					nWorkMode;		/* COMM_LOCAL_SERVER,
// 										 * COMM_CONNECTED_CLIENT,
// 										 */
// 	int					nSockPort;
// 	DWORD				dwHostAddr;		/* local host addr, addr of myself	*/
	
// 	struct sockaddr_in	inPeerAddr;
	
// 	struct sockaddr_in	server_addr;
// 	struct sockaddr_in	client_addr;
// 	struct sockaddr_in	send_addr;

// 	SOCKET				nSocket;
// 	COMM_TIMEOUTS		toTimeouts;

// 	int					nMaxClients;
// 	int					*pCurClients;
// };				
// typedef struct SUdpPortDriver		UDP_PORT_DRV;


#endif

