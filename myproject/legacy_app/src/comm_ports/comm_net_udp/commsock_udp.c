
#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "commsock_udp.h"	/* The private head file for this module	*/


static BOOL UDP_ParseOpenParams(IN char	*pOpenParams, 
								  OUT DWORD *pdwIP, OUT int *piPort)
{
	char	*pszPort = NULL;
	char	*p = pOpenParams;
	char	szIP[64];	// to save ascii IP: 255.255.255.255:65536 
	int		i;
	BOOL	bDomainName = FALSE;

	for (i = 0; (i < (int)(sizeof(szIP)-1)) && (*p != 0); i++)
	{
		/* only 0-9, '.' and ':' are allowed	*/
		if (((*p >= '0') && (*p <= '9')) || (*p == '.'))
		{
			szIP[i] = *p;		
		}
		else if (*p == ':')
		{
			szIP[i] = 0;	/* replace the colon with '\0', IP end	*/
			pszPort = &szIP[i+1];		/* the port string start	*/
		}
		else if(((*p >= 'a') && (*p <= 'z')) || ((*p >= 'A') && (*p <= 'Z')))
		{
			bDomainName = TRUE;
			szIP[i] = *p;
		}
		else		// error character			
		{
			return	FALSE;
		}
		p++;
	}

	szIP[i] = 0;		// end flag				

	TRACE("1111 szIP=[%s] pszPort=[%s]\n", szIP, pszPort);
	if (pszPort == NULL)
	{
		TRACE("Err: pszPort == NULL!!!\n");
		return FALSE;	// error parameters		//
	}

	// convert port		//
	*piPort = atoi( pszPort );	// host format.

	TRACEX("222  szIP=[%s] pszPort=[%s]\n", szIP, pszPort);

#define STR_BROAD_IP	"255.255.255.255"
#define MAX_IP_PORT		65535

	// convert IP addr. inet_addr returns network format.
	*pdwIP = ntohl((DWORD)inet_addr(szIP));// we need host format!!!

	if ((*pdwIP == (DWORD)-1) &&	// got a -1 addr, but not a broadcast
		(strncmp(pOpenParams, STR_BROAD_IP, sizeof(STR_BROAD_IP)) != 0))
	{
		TRACE( "[UDP_ParseOpenParams] -- invalid IP address %s.\n", szIP );
		return FALSE;
	}

	// test port is too big or not, greater than 65335
	if (*piPort > MAX_IP_PORT)
	{
		TRACE( "[UDP_ParseOpenParams] -- invalid IP port %s.\n", pszPort );
		return FALSE;
	}
	
 	i = htonl(*pdwIP);	// convert IP to network format
	TRACE( "[UDP_ParseOpenParams] -- parse %s got ip: %s(%08x), port %d.\n",
		pOpenParams, 
		inet_ntoa(*(struct in_addr *)&i),
		(int)*pdwIP,
		(int)*piPort );
 
	return TRUE;
}

static int SetNonBlockSocket( SOCKET s, int nNonBlock )
{
    int rc = -1;

    if ( nNonBlock != 0 )
    {
        nNonBlock = 1;
    }

#ifdef WIN32
    rc = ioctlsocket(s, FIONBIO, &nNonBlock );
#endif

#ifdef _PSOS
    rc = ioctl(s, FIONBIO, &nNonBlock );
#endif

#ifdef unix
    rc = fcntl( s, F_GETFL );

    if ( nNonBlock )
    {
        rc |= O_NONBLOCK;     //NOBLOCK
    }
    else
    {
        rc &= ~O_NONBLOCK;
    }

    rc = fcntl(s, F_SETFL, rc);
#endif

	/* server socket is nonblocking */
	return rc;
}

static SOCKET CreateNetworkClient(IN UDP_PORT_DRV *pPort,
								  IN int nWorkMode,
								  IN DWORD dwServerAddr, 
								  IN short nServerPort,
								  IN BOOL nNonBlock )
{
	//struct 	sockaddr_in sAddr;
	long   	rc ;
    SOCKET  Client_fd;
	int		addr_len = sizeof(struct sockaddr_in);

    //--------------------------------------------------------------------*/
	// Create socket                                                      */
	//--------------------------------------------------------------------*/
    if ( nWorkMode != SOCK_DGRAM )
    {
        nWorkMode = SOCK_DGRAM;
    }

    Client_fd = socket ( AF_INET, nWorkMode, IPPROTO_IP );//IPPROTO_IP maybe equal to IPPROTO_TCP
	if ( Client_fd < 0 )
	{
        TRACE( "[CreateNetworkServer] -- call socket() fail: %s.\n",
            strerror(SOCK_ERRNO()) ); 
		return INVALID_SOCKET;
	}

	// make sure the socket is synchorous and non-blocking	//
	// if err = 1 ,set Client_fd to non-blocking mode. 		//
	// if err = 0 ,set Client_fd to blocking mode. 			//
    // under debug , to see sync or async 					//
    if (SetNonBlockSocket( Client_fd, nNonBlock ) != 0 )
    {
        TRACE( "[CreateNetworkServer] -- set non-block socket error: %s.\n",\
            strerror(SOCK_ERRNO()) );  
        CLOSE_SOCKET( Client_fd );
        return INVALID_SOCKET;
    }

	memset(&pPort->client_addr, 0, sizeof(pPort->client_addr));
    pPort->client_addr.sin_family = AF_INET;
    pPort->client_addr.sin_port = htons((u_short)nServerPort); 
    pPort->client_addr.sin_addr.s_addr = INADDR_ANY;//htonl( dwServerAddr ); //should be "0.0.0.0"
   
	TRACE("[%s] to %s:%d\n", \
		 __FUNCTION__, inet_ntoa(pPort->client_addr.sin_addr), ntohs(pPort->client_addr.sin_port));
		
	int ret = bind(Client_fd, (const struct sockaddr *)&pPort->client_addr, addr_len);//设置客户端地址
	if (ret < 0)
	{
		TRACE("%s: bind failed\n", __FUNCTION__);
		CLOSE_SOCKET(Client_fd);
		return 0;
	}

	TRACE("Client: Bind ClientID Success!!!\n");
	return Client_fd;
}


static SOCKET CreateNetworkServer(IN UDP_PORT_DRV *pPort, IN int nWorkMode,
								  IN DWORD dwServerAddr, IN short nServerPort,
								  IN BOOL nNonBlock )
{
	//struct sockaddr_in sAddr;
	long   rc ;
    SOCKET  Server_fd;
	int		addr_len = sizeof(struct sockaddr_in);

    /*--------------------------------------------------------------------*/
	/* Create socket                                                      */
	/*--------------------------------------------------------------------*/
    if ( nWorkMode != SOCK_DGRAM )
    {
        nWorkMode = SOCK_DGRAM;
    }

	//1.建立数据报套接字.Establish a datagram socket

    Server_fd = socket ( AF_INET, nWorkMode, IPPROTO_IP );
	if ( Server_fd < 0 )
	{
        TRACE( "[CreateNetworkServer] -- call socket() fail: %s.\n", strerror(SOCK_ERRNO()) ); 
 		return INVALID_SOCKET;
	}

	//2.修改数据报套接字的属性,允许广播.Modify the properties of the datagram socket to allow broadcasting
	rc = 1;
	if(setsockopt(Server_fd, SOL_SOCKET, SO_BROADCAST, (const char *)&rc, sizeof(rc)))
	{
		TRACE("setsockopt SO_BROADCAST is error\n");
	}
	
	memset(&pPort->server_addr, 0, sizeof(pPort->server_addr));
	pPort->server_addr.sin_family      = AF_INET ;
    pPort->server_addr.sin_port        = htons((u_short)nServerPort);
    pPort->server_addr.sin_addr.s_addr = INADDR_ANY;//htonl(dwServerAddr); //INADDR_ANY


    rc = bind ( Server_fd, (struct SC_SOCKADDR *)&pPort->server_addr, addr_len );//设置服务器地址
	if ( rc != 0 )
    {
        TRACE( "[CreateNetworkServer] -- bind() server fail: %s.\n",
            strerror(SOCK_ERRNO()) ); 
        CLOSE_SOCKET( Server_fd );
        return INVALID_SOCKET;
	}

	/* make sure the socket is synchorous and non-blocking	*/
	/* if err = 1 ,set Server_fd to non-blocking mode. */
	/* if err = 0 ,set Server_fd to blocking mode. */
    /* under debug , to see sync or async */
    if (SetNonBlockSocket( Server_fd, nNonBlock ) != 0 )  
    {
        TRACE( "[CreateNetworkServer] -- set non-block socket error: %s.\n",
            strerror(SOCK_ERRNO()) ); 
        CLOSE_SOCKET( Server_fd );
        return INVALID_SOCKET;
    }

	return Server_fd;
}
 

HANDLE UDP_CommOpen(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD	dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode )
{
	UDP_PORT_DRV *pPort = NEW(UDP_PORT_DRV, 1);

	COMM_UDP_SET * sUdpPeerInfo = (COMM_UDP_SET *)pPortDescriptor;

	TRACE("COMMUdpPeerInfo nobloclk[%d],nPeerType[%d],nPeerPort[%d], szIPAddr[%s]\n",
		sUdpPeerInfo->nNonBlock,
		sUdpPeerInfo->sPeerInfo.nPeerType,
		sUdpPeerInfo->sPeerInfo.nPeerPort,
		sUdpPeerInfo->sPeerInfo.szAddr);//字符串
	
	TRACE("local [%s]\n",pOpenParams);

	//BOOL nNonBlock = 1 ;	//默认为非阻塞

	//UNUSED(pPortDescriptor);

	// 1. get mem .
	if (pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return	NULL;
	}

	memset(pPort, 0, sizeof(UDP_PORT_DRV));

	// 2. parse IP and socket port info .
	if (!UDP_ParseOpenParams(pOpenParams, &pPort->dwHostAddr, 
		&pPort->nSockPort))
	{
		TRACE("[UDP_CommOpen] -- invalid parameters: \"%s\".\n",
			pOpenParams);

		DELETE( pPort );
		*pErrCode = ERR_COMM_OPENING_PARAM;

		return	NULL;
	}

	INIT_TIMEOUTS( pPort->toTimeouts, nTimeout, nTimeout);

	// //prase   pPortDescriptor  pPortDescriptor:若包含“BLOCK”字符，且必须为大写，为阻塞;默认状态（即不包含）为非阻塞。
	// if (pPortDescriptor != NULL)
	// {
	// 	printf("pPortDescriptor = [%s]\n");
	// 	if (strstr(pPortDescriptor, "BLOCK"))
	// 	{
	// 		printf("Found 'BLOCK' \n");
	// 		nNonBlock = 0; //阻塞
	// 	}
	// 	else
	// 	{
	// 		printf("Did not find 'BLOCK'\n");
	// 	}
	// }

	//to connect to a remote server, create a client port	
	if ((dwPortAttr & COMM_CLIENT_MODE) == COMM_CLIENT_MODE)
	{
		pPort->nSocket = CreateNetworkClient(
			pPort,
			SOCK_DGRAM,
			pPort->dwHostAddr,				// ignore the server addr
			pPort->nSockPort,
			sUdpPeerInfo->nNonBlock);

		if (pPort->nSocket == INVALID_SOCKET)
		{
			DELETE( pPort->pCurClients );
			DELETE( pPort );
			*pErrCode = ERR_COMM_CONNECT_SERVER;
			TRACE("Error: Create Socket Client Failed.Invalid Socket\n");
			return NULL;
		}

		pPort->nWorkMode = COMM_INCOMING_CLIENT;

	    memset(&pPort->server_addr, 0, sizeof(pPort->server_addr));
		pPort->server_addr.sin_family      = AF_INET ;
        pPort->server_addr.sin_port        = htons(sUdpPeerInfo->sPeerInfo.nPeerPort);
        //pPort->server_addr.sin_addr.s_addr = inet_addr(sUdpPeerInfo->sPeerInfo.szAddr); //服务器的ip  192.168.1.100:9999
		if (inet_pton(AF_INET, sUdpPeerInfo->sPeerInfo.szAddr, &pPort->server_addr.sin_addr) <= 0) 
		{
			return NULL;
    	}
	}

	// to create a server port.
	else
	{
		pPort->nSocket = CreateNetworkServer(
			pPort,
			SOCK_DGRAM,
			pPort->dwHostAddr,				// ignore the server addr
			pPort->nSockPort,
			sUdpPeerInfo->nNonBlock);

			TRACE("Open Server Sock_fd=[%d] nSockPort=[%d]\n", \
				pPort->nSocket, pPort->nSockPort);

		if (pPort->nSocket == INVALID_SOCKET)
		{
			DELETE( pPort->pCurClients );
			DELETE( pPort );
			*pErrCode = ERR_COMM_CREATE_SERVER;
			return NULL;
		}

		pPort->nWorkMode   = COMM_LOCAL_SERVER;
		pPort->nMaxClients = UDP_MAX_CLIENTS_DEFAULT;
		pPort->dwHostAddr =  INADDR_ANY;		 

		memset(&pPort->send_addr, 0, sizeof(pPort->send_addr));
		pPort->send_addr.sin_family = AF_INET;
    	pPort->send_addr.sin_port = htons(sUdpPeerInfo->sPeerInfo.nPeerPort);
    	//pPort->send_addr.sin_addr.s_addr = inet_addr(sUdpPeerInfo->sPeerInfo.szAddr); //客户端的ip 广播地址  192.168.1.255:10000
		if (inet_pton(AF_INET, sUdpPeerInfo->sPeerInfo.szAddr, &pPort->send_addr.sin_addr) <= 0) 
		{
			return NULL;
    	}
		//UDP_BROADCAST_ADDR
	}

	*pErrCode = ERR_COMM_OK;

	TRACE("\npOpenParams is: %s", pOpenParams);
	//printf("\npPort is: %d", pPort);
	return (HANDLE)pPort;
}

#define COM_RETRY_TIME	3
int UDP_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	UDP_PORT_DRV		*pPort = (UDP_PORT_DRV *)hPort;
	int					rc;
	int					fd   = pPort->nSocket;
	int			    	nTotalBytesRead = 0;	// total bytes of data being read
	int					nRetryTimes		= 0;
	//struct sockaddr_in *from = &pPort->inPeerAddr;
	int					addr_len = sizeof(struct sockaddr_in);


	//printf("[UdpRead]: fd=[%d] from->sin_port=[%d]\n", pPort->nSocket, pPort->sin_port);

	// clear last error 
	pPort->nLastErrorCode = ERR_COMM_OK;
	//printf("\n[%.3f]|Thread[%lu]: UDP_CommRead BEGIN\n", GetCurrentTime(), (int)pthread_self());
	
	double fTm_BeforeRead = GetCurrentTime();
	UINT nTimeCost = ABS((int)((GetCurrentTime() - fTm_BeforeRead) * 1000));

	/*rc = WaitFiledReadable(fd, pPort->toTimeouts.nReadTimeout);
	//printf("\n[%d, %d]|Thread[%lu]: UDP_CommRead.WaitFiledReadable BEGIN\n",  nTimeCost, pPort->toTimeouts.nReadTimeout, (int)pthread_self());

	//printf("[%.3f]|Thread[%lu]: UDP_CommRead 222222222222222\n", GetCurrentTime(), (int)pthread_self());
	if( rc <= 0 )
	{
		// 0: wait time out, -1: socket error on waiting.
		//
		// NOTE!: if socket is closed by peer, rc will be 1, 
		//        and the next recv will return 0!
		//

		// 0: wait time out, -1: fd error on waiting.
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_READ_DATA;

 		TRACE("[UDP_CommRead] -- Read data error(%x)\n", pPort->nLastErrorCode);
 
		return 0;	// nothing has been read
	}
	*/

    while (nBytesToRead > 0)
    {
		if(pPort->nWorkMode == COMM_LOCAL_SERVER)
		{
			rc = recvfrom(fd, pBuffer, nBytesToRead, 0,
                (struct sockaddr *)&pPort->client_addr, &addr_len);
			if(rc > 0)
			{
				TRACE("Server ReadData: rc=[%d] [%s] to %s:%d\n", \
					rc, pBuffer, inet_ntoa(pPort->client_addr.sin_addr), ntohs(pPort->client_addr.sin_port));
			}
		}
		else
		{
			rc = recvfrom(fd, pBuffer, nBytesToRead, 0,
                (struct sockaddr *)&pPort->server_addr, &addr_len);
			if(rc > 0)
			{
				TRACE("Client ReadData: rc=[%d] [%s] to %s:%d\n", \
					rc, pBuffer, inet_ntoa(pPort->server_addr.sin_addr), ntohs(pPort->server_addr.sin_port));
			}
		}

		//rc = recvfrom(fd, pBuffer, nBytesToRead, 0, 
		//	(struct sockaddr *)from, &addr_len);

        if (rc > 0)	// we got some data.
        {
			//printf("rc len:%d, data:[%s] from %s:%d\n", rc, pBuffer,
            //    inet_ntoa(from->sin_addr), ntohs(from->sin_port));
               
            // ok
            nBytesToRead	-= rc;
            pBuffer			+= rc;
			nTotalBytesRead += rc;

			if (nBytesToRead <= 0) //for test purpose
			{
				break;	// reading finished
			}

			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}

		else if ((rc < 0) && (errno != EAGAIN))		// read error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_READ_DATA;
				TRACE("[UDP_CommRead] -- Read data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
				break;	// error, quit now.
			}
		}
		else if (rc == 0)	// the socket is shutdown, gracefully closed.
		{
			pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
			TRACE("[UDP_CommRead] -- The socket is shutdown.\n");
			break;
		}

		nTimeCost = ABS((int)((GetCurrentTime() - fTm_BeforeRead) * 1000));
		//printf("\n[%d, %d]|Thread[%lu]: UDP_CommRead.ReadTimes [%d]\n",  nTimeCost, pPort->toTimeouts.nReadTimeout, (int)pthread_self(), nReadTimes);
		if(nTimeCost > pPort->toTimeouts.nReadTimeout)
		{
			break;
		}

		// if error, we also sleep a while and retry again.
		rc = WaitFiledReadable(fd, pPort->toTimeouts.nIntervalTimeout);
		if( rc <= 0 )
		{
			// 0: wait time out, -1: fd error on waiting.
			pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
				: ERR_COMM_READ_DATA;
 			TRACE("[UDP_CommRead] -- The socket failed %s after read %d "
				"bytes.\n", 
				rc == 0 ? "timeout" : "select" , nTotalBytesRead);
 			break;
		}
    }

 	TRACE("[UDP_CommRead] -- End The socket read %d bytes(err code: %d).\n",
		nTotalBytesRead, pPort->nLastErrorCode);
 
	return nTotalBytesRead;
}

/*********************************************************************************
*  
*  FUNCTION NAME : UDP_CommAccept
*  CREATOR       : John
*  CREATED DATE  : 2024-02-25 17:13:53
*  DESCRIPTION   :  Not USed, But Can't Remove it.
*  
*  CHANGE HISTORY: 
*  
***********************************************************************************/
HANDLE UDP_CommAccept( IN HANDLE hPort )
{
	UDP_PORT_DRV *pPort   = (UDP_PORT_DRV *)hPort;
	UDP_PORT_DRV *pClient = NULL;

	// clear last error .
	pPort->nLastErrorCode = ERR_COMM_OK;

	// check the socket whether a server port or not,
 	// only server port is allowed to call accept
	//
	if (pPort->nWorkMode != COMM_LOCAL_SERVER)
	{
		pPort->nLastErrorCode = ERR_COMM_SUPPORT_ACCEPT;
		return NULL;
	}

	return (HANDLE)pClient;
}

int UDP_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite)
{
	UDP_PORT_DRV	*pPort = (UDP_PORT_DRV *)hPort;
	int				rc;
	int				fd   = pPort->nSocket;
	int			    nTotalBytesWritten = 0;	// total bytes of data being written
	int				nRetryTimes		= 0;
	//struct sockaddr_in *to = &pPort->inPeerAddr;
	int				addr_len = sizeof(struct sockaddr_in);

	TRACE("UDP_CommWrite: nBytesToWrite=[%d] addr_len=[%d]\n", nBytesToWrite, addr_len);

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	/*rc = WaitFiledWritable(fd, pPort->toTimeouts.nReadTimeout);
	if( rc <= 0 )
	{
		// 0: wait time out, -1: socket error on waiting    
		//
		//NOTE!: if socket is closed by peer, rc will be 1, 
		//        and the next recv will return 0!
		//

		// 0: wait time out, -1: fd error on waiting.    
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_WRITE_DATA;

 		printf("[UDP_CommWrite] -- Write data error(%x)\n",
			pPort->nLastErrorCode);
		return 0;	// nothing has been read
	}*/

    while (nBytesToWrite > 0)
    {
		if(pPort->nWorkMode == COMM_LOCAL_SERVER)
		{
			rc = sendto(fd, pBuffer, nBytesToWrite, 0,
                (struct sockaddr *)&pPort->send_addr, addr_len);
		}
		else
		{
			rc = sendto(fd, pBuffer, nBytesToWrite, 0,
                (struct sockaddr *)&pPort->server_addr, addr_len);
		}
 		//rc = sendto(fd, pBuffer, nBytesToWrite, 0, 
		//	(struct sockaddr *)to, &nToLen);
		
		TRACE("[UDP_CommWrite] -- Write data rc=(%d)\n", rc);

        if (rc > 0)	//we wrote out some data.
        {
			 // printf("server send pBuffer: [%s] to %s:%d\n", \
             //   pBuffer, inet_ntoa(to->sin_addr), ntohs(to->sin_port));
       
            // ok
            nBytesToWrite	   -= rc;
            pBuffer			   += rc;
			nTotalBytesWritten += rc;

			if (nBytesToWrite <= 0)
			{
				break;	// writing finished
			}

			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}

		//else if ((rc < 0) && (errno != EAGAIN))		// write error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_WRITE_DATA;

 				TRACE("[UDP_CommWrite] -- Write data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
				break;	// error, quit now.
			}
		}
    }

	return nTotalBytesWritten;
}

/*
static int RecvDataFrom( SOCKET sock, 
			 char *pBuf, 
             int nLenToRecv, 
			 int nTimeout, int nIntervalTimeout,
			 struct sockaddr_in *from )
{
    int rc = 0;
	int nFromLen;
	int nBytesRead = 0;

	// changed: first package incoming timetout.
	rc = WaitFiledReadable( sock, nTimeout );

	if( rc <= 0 )
	{
		// 0: wait time out, -1: socket error on waiting    
		//
		// NOTE!: if socket is closed by peer, rc will be 1, 
		//        and the next recv will return 0!
		//
		return rc;
	}

    while (nLenToRecv > 0)
    {
		nFromLen = sizeof(struct sockaddr_in);

		rc = recvfrom(sock, pBuf, nLenToRecv, 0, 
			(struct sockaddr *)from, &nFromLen);

        if (rc <= 0)
        {
            // 0: socket is closed by peer, -1: recv error.
			rc = -1;
            break;
        }

        nLenToRecv -= rc;
        pBuf       += rc;
		nBytesRead += rc;

		if (nLenToRecv <= 0)
		{
			break;	// read done.
		}

		// to check the timeout between 2 bytes
		rc = WaitFiledReadable( sock, nIntervalTimeout );
		if( rc <= 0 )
		{
			// 0: wait time out, -1: socket error on waiting.
			//
			// NOTE!: if socket is closed by peer, rc will be 1, 
			//        and the next recv will return 0!
			//
			break;
		}
	}


	TRACE( "[UDP Read] -- got %d\n", nBytesRead );

	return (rc >= 0) ? nBytesRead : -1;
}
*/

/*
static int SendDataTo( SOCKET sock, char *pBuf, 
			int nLenToSend,
			int nTimeout, int nIntervalTimeout,
			struct sockaddr_in *to )
{
    int rc = 0;
	int nFromLen;
	int nBytesWritten = 0;

	// changed: first package sending timetout.
	rc = WaitFiledWritable( sock, nTimeout );

	if( rc <= 0 )
	{
		// 0: wait time out, -1: socket error on waiting.
		//
		// NOTE!: if socket is closed by peer, rc will be 1, 
		//        and the next recv will return 0!
		//

		TRACE( "[SendDataTo] --1, WaitFiledWritable got %d \n", rc );
		return rc;
	}

    while (nLenToSend > 0)
    {
		nFromLen = sizeof(struct sockaddr_in);

		rc = sendto(sock, pBuf, nLenToSend, 0, 
			(struct sockaddr *)to, &nFromLen);

        if (rc <= 0)
        {
            // 0: socket is closed by peer, -1: recv error.
			rc = -1;

			TRACE( "[UDP SendDataTo] --1, sendto got %d errno %d \n", rc,errno );
			break;
        }

        nLenToSend -= rc;
        pBuf       += rc;
		nBytesWritten += rc;

		if (nLenToSend <= 0)
		{
			break;	// write done.
		}

		// to check the timeout between 2 bytes
		rc = WaitFiledWritable( sock, nIntervalTimeout );
		if( rc <= 0 )
		{
			// 0: wait time out, -1: socket error on waiting.
			//
			// NOTE!: if socket is closed by peer, rc will be 1, 
			//        and the next recv will return 0!
			//

 			TRACE( "[SendDataTo] --2, WaitFiledWritable got %d \n", rc );
 			break;
		}
	}
	return (rc >= 0) ? nBytesWritten : -1;
}
*/

/*
int UDP_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	UDP_PORT_DRV *pPort = (UDP_PORT_DRV *)hPort;
	int			nBytesRead;

	// clear last error.
	pPort->nLastErrorCode = ERR_COMM_OK;

	nBytesRead = RecvDataFrom(pPort->nSocket,
		pBuffer,
		nBytesToRead,
		pPort->toTimeouts.nReadTimeout, 
		pPort->toTimeouts.nIntervalTimeout,
		(struct sockaddr_in *)&pPort->inPeerAddr );

	if (nBytesRead < 0)
	{
		pPort->nLastErrorCode = ERR_COMM_READ_DATA;
	}

	//timeout, the data length read out is less than we want to read.
	else if (nBytesRead < nBytesToRead)
	{
		pPort->nLastErrorCode = ERR_COMM_TIMEOUT;
	}
	return nBytesRead;
}
				
int UDP_CommWrite(IN HANDLE hPort, 
					IN char *pBuffer,	
					IN int nBytesToWrite)
{
	UDP_PORT_DRV *pPort = (UDP_PORT_DRV *)hPort;
	int			nBytesWritten;

	// clear last error 
	pPort->nLastErrorCode = ERR_COMM_OK;

	nBytesWritten = SendDataTo(pPort->nSocket,
		pBuffer,
		nBytesToWrite,
		pPort->toTimeouts.nWriteTimeout,
		pPort->toTimeouts.nIntervalTimeout,
		(struct sockaddr_in *)&pPort->inPeerAddr );

	if (nBytesWritten < 0)
	{
		pPort->nLastErrorCode = ERR_COMM_WRITE_DATA;
	}

	// timeout, the data length written out is less than we want to write.
	else if (nBytesWritten < nBytesToWrite )
	{
		pPort->nLastErrorCode = ERR_COMM_TIMEOUT;
	}

	return nBytesWritten;
}
 */

int UDP_CommControl(	IN HANDLE hPort, IN int	nCmd, 
					  IN OUT void *pBuffer,	IN int	nDataLength)
{
	/* Now we use switch to test the command, 
	 * we can optimize it if there are too many commands 
	 * which makes lower efficiency
	 */
	UDP_PORT_DRV *pPort = (UDP_PORT_DRV *)hPort;
	int nErrCode = ERR_COMM_OK;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	//ASSERT( pBuffer );

	switch (nCmd)
	{
	case COMM_GET_LAST_ERROR:	// get the last error code.	 
		{
			ASSERT(nDataLength == sizeof(int));

			*(int *)pBuffer = pPort->nLastErrorCode;
		}

		break;

	case COMM_GET_PEER_INFO:	//get the sender info.
		{
			COMM_PEER_INFO *pPeerInfo = (COMM_PEER_INFO *)pBuffer;

			ASSERT(nDataLength == sizeof(COMM_PEER_INFO));

			if (pPort->nWorkMode != COMM_LOCAL_SERVER)
			{
				pPeerInfo->nPeerPort = pPort->inPeerAddr.sin_port;
				pPeerInfo->nPeerType = pPort->nWorkMode;
				*(DWORD *)pPeerInfo->szAddr = *(DWORD *)&pPort->inPeerAddr.sin_addr;
			}
			else
			{
				//The server port does not support.
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_SET_TIMEOUTS:		// set the comm timeout in ms. 
		{	
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

			pPort->toTimeouts = *(COMM_TIMEOUTS *)pBuffer;
			//added by Jimmy just in case some one forget to initial the intervel_timeout
			if((pPort->toTimeouts.nIntervalTimeout < 20) || (pPort->toTimeouts.nIntervalTimeout > 100))
			{
				pPort->toTimeouts.nIntervalTimeout = TIMEOUT_INTERVAL_CHAR;
			}
		}

		break;

	case COMM_GET_TIMEOUTS:		//get the comm timeout in ms.
		{	
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

			*(COMM_TIMEOUTS *)pBuffer = pPort->toTimeouts;
		}
		break;

	case COMM_PURGE_TXCLEAR:	// clear the transmit buffer.
		{
			//no action.
			if (pPort->nWorkMode == COMM_LOCAL_SERVER)
			{
				//The server port does not support.
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_PURGE_RXCLEAR:	//clear the receive buffer.
		{
			if (pPort->nWorkMode != COMM_LOCAL_SERVER)
			{
				COMM_TIMEOUTS	toOld  = pPort->toTimeouts;	// save timeout
				char			szTemp[256];

				// set the new timeout before controlling
				INIT_TIMEOUTS_EX(pPort->toTimeouts, 100, 100, 10 );

				// to receive all data
				while (UDP_CommRead(hPort, szTemp, sizeof(szTemp)) > 0)
				{
					;
				}
			
				pPort->toTimeouts = toOld;	// restore timeouts
			}

			//The server port does not support.
			else // if (pPort->nWorkMode == COMM_LOCAL_SERVER) 
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_SET_MAX_CLIENTS:	//set the maximum allowed clients.
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER)	// Must be server	
			{	
				if ((*(int *)pBuffer > 0)				 // client must > 0	
					&& (*(int *)pBuffer <= UDP_MAX_CLIENT_LIMITAION))
				{
					pPort->nMaxClients = *(int *)pBuffer;
				}
				else
				{
					nErrCode = ERR_COMM_CTRL_PARAMS;
				}
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_GET_MAX_CLIENTS:		// get the maximum allowed clients 
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER) // Must be server
			{
				*(int *)pBuffer = pPort->nMaxClients;
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

		// get the number of the current connected clients 
	case COMM_GET_CUR_CLIENTS:
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER) // Must be server.
			{
				*(int *)pBuffer = *pPort->pCurClients;
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_GET_CLIENT_LIMITATION:	//get the maximum client limitation 
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER) // Must be server	 
			{
				*(int *)pBuffer = UDP_MAX_CLIENT_LIMITAION;
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	default:
		{
			nErrCode = ERR_COMM_CTRL_COMMAND;
		}
	}

	return nErrCode;
}

int UDP_CommClose( IN HANDLE hPort )
{
	UDP_PORT_DRV *pPort = (UDP_PORT_DRV *)hPort;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	//Jimmy note 20210302: we do not have multi-process case, so that we shut it even for server
	//if (pPort->nWorkMode != COMM_LOCAL_SERVER)
	{
		shutdown(pPort->nSocket, SD_BOTH);
	}

	CLOSE_SOCKET(pPort->nSocket);		//close the socket handle.

	// descrease the linkage, the pCurClients shall be deleted if no
	 // any clients. But the server and clients share the same pCurClients,
	 // and the initialing value of *pCurClients is 0, so the condition to
	 // delete pCurClients is *pCurClients < 0.
	 //
 	TRACE("After closing *pPort->pCurClients is %d\n", *pPort->pCurClients);
 	//(*pPort->pCurClients)--;
	//if (*pPort->pCurClients < 0)
	if(pPort->nWorkMode == COMM_LOCAL_SERVER) //changed by Jimmy 20210302
	{
 		TRACE("After2 closing *pPort->pCurClients is %d\n", *pPort->pCurClients);
 		DELETE(pPort->pCurClients);
	}
	else if(pPort->nWorkMode == COMM_INCOMING_CLIENT)
	{
		if (*pPort->pCurClients > 0)
		{
			(*pPort->pCurClients)--;
		}
	}

	DELETE(pPort);
	return ERR_COMM_OK;
}

