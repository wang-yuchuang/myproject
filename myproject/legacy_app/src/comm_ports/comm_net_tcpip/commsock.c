
#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "commsock.h"	/* The private head file for this module	*/

#ifdef _DEBUG
//#define _DEBUG_TCPIP_PORT
#endif //_DEBUG

//John.Note: Network Need Ping baidu.com OK.getaddrinfo Will ask remote Server Transfer DomainName To IP.
static char *Trans_Domain2IP(IN char *pOrgDomainName, OUT char *pOutIP)
{
	struct addrinfo hints;

    struct addrinfo *result, *result_pointer;

    int ret;

    // obtaining address matching host 

    memset(&hints, 0, sizeof(struct addrinfo));

    hints.ai_family = AF_UNSPEC;

    hints.ai_socktype = SOCK_STREAM;

    hints.ai_flags = AI_CANONNAME;

    hints.ai_protocol = 0; // any protocol 

	TRACEX("pOrgDomainName =[%s] \n", pOrgDomainName);

    ret = getaddrinfo(pOrgDomainName, NULL, &hints, &result);
    if (ret != 0)
    {
        //fprintf(stderr, "Err:getaddrinfo: %s\n", gai_strerror(ret));
        return NULL;
    }

    //traverse the returned list and output the ip addresses
	BOOL bGetIPOk = FALSE;
	char hostname[64] = "";
    for (result_pointer = result; result_pointer != NULL; result_pointer = result_pointer->ai_next)
    {
        ret = getnameinfo(result_pointer->ai_addr, result_pointer->ai_addrlen, hostname, sizeof(hostname), NULL, 0, NI_NUMERICHOST);
        if (ret != 0)
        {
            //fprintf(stderr, "error in getnameinfo: %s \n", gai_strerror(ret));
            continue;
        }
        else
        {
			memcpy(pOutIP, hostname, 64);
			TRACE("pOutIP: %s hostname=[%s]\n", pOutIP, hostname);

			bGetIPOk = TRUE;
			break;	//Only Get one IP,some Domain Have Two IP.
        }
    }

    freeaddrinfo(result);

	return (bGetIPOk == TRUE) ? pOutIP : NULL;
}

static BOOL TCPIP_ParseOpenParams(IN char	*pOpenParams, 
								  OUT DWORD *pdwIP, OUT int *piPort)
{
	char	*pszPort = NULL;
	char	*p = pOpenParams;
	char	szIP[64];	/* to save ascii IP: 255.255.255.255:65536  */
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
		else		/* error character			*/
		{
			return	FALSE;
		}
		p++;
	}

	szIP[i] = 0;		/* end flag				*/

	TRACE("1111 szIP=[%s] pszPort=[%s]\n", szIP, pszPort);
	if (pszPort == NULL)
	{
		TRACE("Err: pszPort == NULL!!!\n");
		return FALSE;	/* error parameters		*/
	}

	/* convert port		*/
	*piPort = atoi( pszPort );	// host format.

	//--- John.Add Domain Name Transfer To IP Addr ---//
	char szDomainIP[64]="0";
	char *pOutIP = NULL;
	if(bDomainName == TRUE)
	{
		pOutIP = Trans_Domain2IP(szIP, szDomainIP);
		if(pOutIP != NULL)
		{
			memcpy(szIP, pOutIP, 64);
		}
		else
		{
			return FALSE;	/* error parameters		*/
		}
	}
	//------               End                 -------//

	TRACEX("222  szIP=[%s] pszPort=[%s]\n", szIP, pszPort);

#define STR_BROAD_IP	"255.255.255.255"
#define MAX_IP_PORT		65535

	// convert IP addr. inet_addr returns network format.
	*pdwIP = ntohl((DWORD)inet_addr(szIP));// we need host format!!!

	if ((*pdwIP == (DWORD)-1) &&	// got a -1 addr, but not a broadcast
		(strncmp(pOpenParams, STR_BROAD_IP, sizeof(STR_BROAD_IP)) != 0))
	{
		TRACE( "[TCPIP_ParseOpenParams] -- invalid IP address %s.\n", 
			szIP );
		return FALSE;
	}

	// test port is too big or not, greater than 65335
	if (*piPort > MAX_IP_PORT)
	{
		TRACE( "[TCPIP_ParseOpenParams] -- invalid IP port %s.\n",
			pszPort );
		return FALSE;
	}
	
#ifdef _DEBUG_TCPIP_PORT
	i = htonl(*pdwIP);	// convert IP to network format
	TRACE( "[TCPIP_ParseOpenParams] -- parse %s got ip: %s(%08x), port %d.\n",
		pOpenParams, 
		inet_ntoa(*(struct in_addr *)&i),
		(int)*pdwIP,
		(int)*piPort );
#endif //_DEBUG_TCPIP_PORT

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

static SOCKET CreateNetworkServer(IN int nWorkMode,
								  IN DWORD dwServerAddr, IN short nServerPort )
{
	struct sockaddr_in sAddr;
	long   rc ;
    SOCKET  sServer;

    /*--------------------------------------------------------------------*/
	/* Create socket                                                      */
	/*--------------------------------------------------------------------*/
    if ( nWorkMode != SOCK_DGRAM )
    {
        nWorkMode = SOCK_STREAM;
    }

    sServer = socket ( AF_INET, nWorkMode, IPPROTO_IP );
	if ( sServer < 0 )
	{
#ifdef _DEBUG_TCPIP_PORT
        printf( "[CreateNetworkServer] -- call socket() fail: %s.\n",
            strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/
		return INVALID_SOCKET;
	}

	// let the address can be reused.
	rc = 1;
	setsockopt(sServer, SOL_SOCKET, SO_REUSEADDR, 
		(const char *)&rc, sizeof(rc));

	memset( ( void * )&sAddr, 0, sizeof(sAddr) );

    sAddr.sin_family      = AF_INET ;
	sAddr.sin_port        = htons((u_short)nServerPort);
	sAddr.sin_addr.s_addr = htonl(dwServerAddr);

    rc = bind ( sServer, (struct SC_SOCKADDR *)&sAddr, sizeof( sAddr ) );
	if ( rc != 0 )
    {
#ifdef _DEBUG_TCPIP_PORT
        printf( "[CreateNetworkServer] -- bind() server fail: %s.\n",
            strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/

        CLOSE_SOCKET( sServer );
		 
        return INVALID_SOCKET;
	}

	/* make sure the socket is synchorous and non-blocking	*/
	/* if err = 1 ,set sServer to non-blocking mode. */
	/* if err = 0 ,set sServer to blocking mode. */
    /* under debug , to see sync or async */
    if (SetNonBlockSocket( sServer, 1 ) != 0 )
    {
#ifdef _DEBUG_TCPIP_PORT
        printf( "[CreateNetworkServer] -- set non-block socket error: %s.\n",
            strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/
        
        CLOSE_SOCKET( sServer );
        return INVALID_SOCKET;
    }

    /* start to listen */
    if (nWorkMode == SOCK_STREAM)
    {
		/* accoring to experience, 5 is enough	*/
        if ( listen( sServer, 5 ) < 0 )
        {
#ifdef _DEBUG_TCPIP_PORT
            printf( "[CreateNetworkServer] -- listen() error: %s.\n",
                strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/

            CLOSE_SOCKET( sServer );
            return INVALID_SOCKET;
        }
    }

	return sServer;
}

static SOCKET ConnectNetworkServer(IN int nWorkMode, 
					  IN DWORD dwServerAddr, IN short nServerPort,
					  IN int nTimeout )
{
	SOCKET s; 		        /*socket descriptor and socket type */
    int rc = 0;
	
    if ( nWorkMode != SOCK_DGRAM )
    {
        nWorkMode = SOCK_STREAM;
    }

	/*Allocate a socket*/
    s = socket( AF_INET, nWorkMode, IPPROTO_IP );
	if ( s == INVALID_SOCKET )
    {
#ifdef _DEBUG_TCPIP_PORT
        printf( "[ConnectNetworkServer] -- call socket() error: : %s.\n",
            strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/
        return INVALID_SOCKET;
    }

    /* set the sock to non-blocking     */
    rc = SetNonBlockSocket( s, 1 /*bNonBlock*/ );
    if ( rc != 0 )
    {
#ifdef _DEBUG_TCPIP_PORT
        printf( "[ConnectNetworkServer] -- set non-block socket error: %s.\n",
            strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/
    }

    // do connect
    if ( rc == 0 ) // init ok
    {
        struct sockaddr_in sin; /*an internet endpoint address      */

       	memset ( ( void * )&sin, 0, sizeof(sin) );

        sin.sin_family = AF_INET ;
        sin.sin_port = htons( (u_short)nServerPort );
        sin.sin_addr.s_addr = htonl(dwServerAddr); 

	    /* Connect the socket.  the connect() will be return immediately 
         * because we set socket to non-blocking mode. 
         *
         * if connected ok, the select(FD_WRITE) will return write fds
         */
	    if ( (connect(s, (struct SC_SOCKADDR *)&sin, sizeof(sin)) == 0) ||
            (SOCK_ERRNO() == E_INPROGRESS) )    /* non-blocking */
        {
            if ((nTimeout <= 0) || /* do not wait connect return */
                (WaitFiledWritable( s, nTimeout ) > 0))
            {
				int	nSockError = 0;
				int nLen = sizeof(int);

				// if the sock is writable, to get the error code,
				// if SO_ERROR is 0, OK.
				if ((getsockopt(s, SOL_SOCKET, SO_ERROR, 
					(char *)&nSockError, &nLen) == 0) &&
					(nSockError == 0))
				{
	                /* connect ok ! */
		            return s;   /* teturn the socket descriptor */
				}

				TRACE( "[ConnectNetworkServer] -- SO_ERROR: %d(%s)\n",
					nSockError, strerror(nSockError));
            }
        }
    }

#ifdef _DEBUG_TCPIP_PORT
    printf( "[ConnectNetworkServer] -- failed on error: %s.\n",
        strerror(SOCK_ERRNO()) ); 
#endif 	/*_DEBUG_TCPIP_PORT	*/

    /* error occurred   */
    CLOSE_SOCKET(s);

    return INVALID_SOCKET;
}

HANDLE TCPIP_CommOpen(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD	dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode )
{
	TCPIP_PORT_DRV *pPort = NEW(TCPIP_PORT_DRV, 1);

	UNUSED(pPortDescriptor);

	/* 1. get mem */
	if (pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return	NULL;
	}

	memset(pPort, 0, sizeof(TCPIP_PORT_DRV));

	/* 2. parse IP and socket port info */
	if (!TCPIP_ParseOpenParams(pOpenParams, &pPort->dwHostAddr, 
		&pPort->nSockPort))
	{
#ifdef _DEBUG_TCPIP_PORT
		TRACE("[TCPIP_CommOpen] -- invalid parameters: \"%s\".\n",
			pOpenParams);

#endif //_DEBUG_TCPIP_PORT

		DELETE( pPort );
		*pErrCode = ERR_COMM_OPENING_PARAM;

		return	NULL;
	}

	INIT_TIMEOUTS( pPort->toTimeouts, nTimeout, nTimeout);
	
	pPort->pCurClients = NEW( int, 1 );	
	if (pPort->pCurClients == NULL)
	{
		DELETE( pPort );
		*pErrCode = ERR_COMM_NO_MEMORY;

		return NULL;
	}

	*pPort->pCurClients = 0;	/*	no linkages now		*/

	/* to connect to a remote server, create a client port	*/
	if ((dwPortAttr & COMM_CLIENT_MODE) == COMM_CLIENT_MODE)
	{
		pPort->nSocket = ConnectNetworkServer(
			SOCK_STREAM,
			pPort->dwHostAddr,	/* the server address	*/
			pPort->nSockPort,	
			nTimeout );

		if (pPort->nSocket == INVALID_SOCKET)
		{
			DELETE( pPort->pCurClients );
			DELETE( pPort );
			*pErrCode = ERR_COMM_CONNECT_SERVER;

			return NULL;
		}

		pPort->nWorkMode = COMM_OUTGOING_CLIENT;

		/* set the peer info */
        pPort->inPeerAddr.sin_family      = AF_INET ;
        pPort->inPeerAddr.sin_port        = htons(pPort->nSockPort);
        pPort->inPeerAddr.sin_addr.s_addr = htonl(pPort->dwHostAddr); 
	}

	/* to create a server port	*/
	else
	{
		pPort->dwHostAddr = INADDR_ANY;		// ignore this

		pPort->nSocket = CreateNetworkServer(
			SOCK_STREAM,
			pPort->dwHostAddr,				// ignore the server addr
			pPort->nSockPort);

		if (pPort->nSocket == INVALID_SOCKET)
		{
			DELETE( pPort->pCurClients );
			DELETE( pPort );
			*pErrCode = ERR_COMM_CREATE_SERVER;

			return NULL;
		}

		pPort->nWorkMode   = COMM_LOCAL_SERVER;
		pPort->nMaxClients = TCPIP_MAX_CLIENTS_DEFAULT;
	}

	*pErrCode = ERR_COMM_OK;

	//TRACE("\npOpenParams is: %s", pOpenParams);
	//TRACE("\npPort is: %d", pPort);
	return (HANDLE)pPort;
}

HANDLE TCPIP_CommAccept( IN HANDLE hPort )
{
	TCPIP_PORT_DRV *pPort   = (TCPIP_PORT_DRV *)hPort;
	TCPIP_PORT_DRV *pClient = NULL;

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	/* check the socket whether a server port or not,
 	 * only server port is allowed to call accept
	 */
	if (pPort->nWorkMode != COMM_LOCAL_SERVER)
	{
		pPort->nLastErrorCode = ERR_COMM_SUPPORT_ACCEPT;
		return NULL;
	}

	/* check the current client num exceeds the maximum connections or not */
	if ( *pPort->pCurClients >= pPort->nMaxClients)
	{
		pPort->nLastErrorCode = ERR_COMM_MANY_CLIENTS;
		return NULL;
	}

	/* If the socket is readable, means there is a client is pending	*/
	//Jimmy 20230529: don't use WaitFiledReadable here
	//if(WaitFiledReadable( pPort->nSocket, pPort->toTimeouts.nReadTimeout ) > 0)
	{
	    struct sockaddr_in	saAddr;
		int					n = sizeof(saAddr);
		SOCKET				sClient;

		sClient = accept( pPort->nSocket, (struct SC_SOCKADDR *)&saAddr, &n );
		if (sClient== INVALID_SOCKET)
		{
			pPort->nLastErrorCode = ERR_COMM_ACCEPT_FAILURE;
			return NULL;
		}

		/* Ok, create a client object */
		pClient = NEW(TCPIP_PORT_DRV,1);
		if (pClient == NULL)	/* out of memory error	*/
		{
			pPort->nLastErrorCode = ERR_COMM_NO_MEMORY;
			CLOSE_SOCKET( sClient );

			return NULL;
		}

		*pClient			= *pPort;	/* Copy all info from server port	*/
		pClient->nSocket	= sClient;
		pClient->nWorkMode	= COMM_INCOMING_CLIENT;
		pClient->inPeerAddr = saAddr;
		(*pPort->pCurClients)++;		/* increase the current linkage		*/
	}
	// else
	// {
	// 	pPort->nLastErrorCode = ERR_COMM_TIMEOUT;
	// 	return NULL;
	// }

	return (HANDLE)pClient;
}


#define __USE_OPTIMIZED_SOCK_CODE

#ifdef __USE_OPTIMIZED_SOCK_CODE

#define COM_RETRY_TIME	3

int TCPIP_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	TCPIP_PORT_DRV	*pPort = (TCPIP_PORT_DRV *)hPort;
	int				rc;
	int				fd   = pPort->nSocket;
	int			    nTotalBytesRead = 0;	// total bytes of data being read
	int				nRetryTimes		= 0;
#ifdef _IMPL_UDP_PORT
	struct sockaddr_in *from = &pPort->inPeerAddr;
	int				nFromLen;
#endif

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;
	//printf("\n[%.3f]|Thread[%lu]: TCPIP_CommRead BEGIN\n", GetCurrentTime(), (int)pthread_self());
	double fTm_BeforeRead = GetCurrentTime();
	rc = WaitFiledReadable(fd, pPort->toTimeouts.nReadTimeout);
	UINT nTimeCost = ABS((int)((GetCurrentTime() - fTm_BeforeRead) * 1000));
	//printf("\n[%d, %d]|Thread[%lu]: TCPIP_CommRead.WaitFiledReadable BEGIN\n",  nTimeCost, pPort->toTimeouts.nReadTimeout, (int)pthread_self());

	//printf("[%.3f]|Thread[%lu]: TCPIP_CommRead 222222222222222\n", GetCurrentTime(), (int)pthread_self());
	if( rc <= 0 )
	{
		/* 0: wait time out, -1: socket error on waiting    */
		/*
		* NOTE!: if socket is closed by peer, rc will be 1, 
		*        and the next recv will return 0!
		*/

		/* 0: wait time out, -1: fd error on waiting    */
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_READ_DATA;

#ifdef _DEBUG_TCPIP_PORT
		TRACE("[TCPIP_CommRead] -- Read data error(%x)\n", pPort->nLastErrorCode);
#endif //_DEBUG_TCPIP_PORT

		return 0;	// nothing has been read
	}

    while (nBytesToRead > 0)
    {
#ifdef _IMPL_UDP_PORT	// for UDP
		nFromLen = sizeof(struct sockaddr_in);
		rc = recvfrom(fd, pBuffer, nBytesToRead, 0, 
			(struct sockaddr *)from, &nFromLen);
#else	// for TCP/IP
		rc = recv(fd, pBuffer, (size_t)nBytesToRead, 0);
#endif //_IMPL_UDP_PORT

        if (rc > 0)	/* we got some data */
        {
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

#ifdef _DEBUG_TCPIP_PORT
				TRACE("[TCPIP_CommRead] -- Read data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
#endif //_DEBUG_TCPIP_PORT

				break;	// error, quit now.
			}
		}

#ifndef _IMPL_UDP_PORT	// for UDP
		else if (rc == 0)	// the socket is shutdown, gracefully closed.
		{
			pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
#ifdef _DEBUG_TCPIP_PORT
			TRACE("[TCPIP_CommRead] -- The socket is shutdown.\n");
#endif //_DEBUG_TCPIP_PORT

			break;
		}
#endif

		nTimeCost = ABS((int)((GetCurrentTime() - fTm_BeforeRead) * 1000));
		//printf("\n[%d, %d]|Thread[%lu]: TCPIP_CommRead.ReadTimes [%d]\n",  nTimeCost, pPort->toTimeouts.nReadTimeout, (int)pthread_self(), nReadTimes);
		if(nTimeCost > pPort->toTimeouts.nReadTimeout)
		{
			break;
		}

		// if error, we also sleep a while and retry again.
		rc = WaitFiledReadable(fd, pPort->toTimeouts.nIntervalTimeout);
		if( rc <= 0 )
		{
			/* 0: wait time out, -1: fd error on waiting    */
			pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
				: ERR_COMM_READ_DATA;
#ifdef _DEBUG_TCPIP_PORT
			TRACE("[TCPIP_CommRead] -- The socket failed %s after read %d "
				"bytes.\n", 
				rc == 0 ? "timeout" : "select" , nTotalBytesRead);
#endif //_DEBUG_TCPIP_PORT
			break;
		}
    }

#ifdef _DEBUG_TCPIP_PORT
	TRACE("[TCPIP_CommRead] -- End The socket read %d bytes(err code: %d).\n",
		nTotalBytesRead, pPort->nLastErrorCode);
#endif //_DEBUG_TCPIP_PORT

	return nTotalBytesRead;
}

int TCPIP_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite)
{
	TCPIP_PORT_DRV	*pPort = (TCPIP_PORT_DRV *)hPort;
	int				rc;
	int				fd   = pPort->nSocket;
	int			    nTotalBytesWritten = 0;	// total bytes of data being written
	int				nRetryTimes		= 0;
#ifdef _IMPL_UDP_PORT
	struct sockaddr_in *to = &pPort->inPeerAddr;
	int				nToLen;
#endif

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	rc = WaitFiledWritable(fd, pPort->toTimeouts.nReadTimeout);
	if( rc <= 0 )
	{
		/* 0: wait time out, -1: socket error on waiting    */
		/*
		* NOTE!: if socket is closed by peer, rc will be 1, 
		*        and the next recv will return 0!
		*/

		/* 0: wait time out, -1: fd error on waiting    */
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_WRITE_DATA;

#ifdef _DEBUG_TCPIP_PORT
		TRACE("[TCPIP_CommWrite] -- Write data error(%x)\n",
			pPort->nLastErrorCode);
#endif //_DEBUG_TCPIP_PORT

		return 0;	// nothing has been read
	}

    while (nBytesToWrite > 0)
    {

#ifdef _IMPL_UDP_PORT
		rc = sendto(fd, pBuffer, nBytesToWrite, 0, 
			(struct sockaddr *)to, &nToLen);
#else
		rc = send(fd, pBuffer, (size_t)nBytesToWrite, 0);
#endif //_IMPL_UDP_PORT

        if (rc > 0)	/* we wrote out some data */
        {
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

#ifdef _DEBUG_TCPIP_PORT
				TRACE("[TCPIP_CommWrite] -- Write data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
#endif //_DEBUG_TCPIP_PORT

				break;	// error, quit now.
			}
		}

		// if error, we also sleep a while and retry again.
		// rc = WaitFiledWritable(fd, pPort->toTimeouts.nIntervalTimeout);
		// if( rc <= 0 )
		// {
		// 	/* 0: wait time out, -1: fd error on waiting    */
		// 	pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
		// 		: ERR_COMM_READ_DATA;
		// 	break;
		// }
    }

	return nTotalBytesWritten;
}

#else

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
		/* 0: wait time out, -1: socket error on waiting    */
		/*
		* NOTE!: if socket is closed by peer, rc will be 1, 
		*        and the next recv will return 0!
		*/
		return rc;
	}

    while (nLenToRecv > 0)
    {
		nFromLen = sizeof(struct sockaddr_in);

#ifdef _IMPL_UDP_PORT
		rc = recvfrom(sock, pBuf, nLenToRecv, 0, 
			(struct sockaddr *)from, &nFromLen);
#else
		rc = recv(sock, pBuf, nLenToRecv, 0);
#endif //_IMPL_UDP_PORT

        if (rc <= 0)
        {
            /* 0: socket is closed by peer, -1: recv error      */
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
			/* 0: wait time out, -1: socket error on waiting    */
			/*
			* NOTE!: if socket is closed by peer, rc will be 1, 
			*        and the next recv will return 0!
			*/
			break;
		}
	}


#ifdef _DEBUG_TCPIP_PORT
	TRACE( "[Read] -- got %d\n", nBytesRead );
#endif //_DEBUG_TCPIP_PORT

	return (rc >= 0) ? nBytesRead : -1;
}

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
		/* 0: wait time out, -1: socket error on waiting    */
		/*
		* NOTE!: if socket is closed by peer, rc will be 1, 
		*        and the next recv will return 0!
		*/

#ifdef _DEBUG_TCPIP_PORT
		TRACE( "[SendDataTo] --1, WaitFiledWritable got %d \n", rc );
#endif //_DEBUG_TCPIP_PORT

		return rc;
	}

    while (nLenToSend > 0)
    {
		nFromLen = sizeof(struct sockaddr_in);


#ifdef _IMPL_UDP_PORT
		rc = sendto(sock, pBuf, nLenToSend, 0, 
			(struct sockaddr *)to, &nFromLen);
#else
		rc = send( sock, pBuf, nLenToSend, 0);
#endif //_IMPL_UDP_PORT

        if (rc <= 0)
        {
            /* 0: socket is closed by peer, -1: recv error      */
			rc = -1;

#ifdef _DEBUG_TCPIP_PORT
			TRACE( "[SendDataTo] --1, sendto got %d errno %d \n", rc,errno );
#endif //_DEBUG_TCPIP_PORT
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
			/* 0: wait time out, -1: socket error on waiting    */
			/*
			* NOTE!: if socket is closed by peer, rc will be 1, 
			*        and the next recv will return 0!
			*/

#ifdef _DEBUG_TCPIP_PORT
			TRACE( "[SendDataTo] --2, WaitFiledWritable got %d \n", rc );
#endif //_DEBUG_TCPIP_PORT
			break;
		}
	}

	return (rc >= 0) ? nBytesWritten : -1;
}

int TCPIP_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	TCPIP_PORT_DRV *pPort = (TCPIP_PORT_DRV *)hPort;
	int			nBytesRead;

	/* clear last error */
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

	/* timeout, the data length read out is less than we want to read	*/
	else if (nBytesRead < nBytesToRead)
	{
		pPort->nLastErrorCode = ERR_COMM_TIMEOUT;
	}


	return nBytesRead;
}
				
int TCPIP_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite)
{
	TCPIP_PORT_DRV *pPort = (TCPIP_PORT_DRV *)hPort;
	int			nBytesWritten;

	/* clear last error */
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

	/* timeout, the data length written out is less than we want to write */
	else if (nBytesWritten < nBytesToWrite )
	{
		pPort->nLastErrorCode = ERR_COMM_TIMEOUT;
	}

	return nBytesWritten;
}
#endif // ifdef __USE_OPTIMIZED_SOCK_CODE

int TCPIP_CommControl(	IN HANDLE hPort, IN int	nCmd, 
					  IN OUT void *pBuffer,	IN int	nDataLength)
{
	/* Now we use switch to test the command, 
	 * we can optimize it if there are too many commands 
	 * which makes lower efficiency
	 */
	TCPIP_PORT_DRV *pPort = (TCPIP_PORT_DRV *)hPort;
	int nErrCode = ERR_COMM_OK;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	//ASSERT( pBuffer );

	switch (nCmd)
	{
	case COMM_GET_LAST_ERROR:	/* get the last error code	*/
		{
			ASSERT(nDataLength == sizeof(int));

			*(int *)pBuffer = pPort->nLastErrorCode;
		}

		break;

	case COMM_GET_PEER_INFO:	/* get the sender info		*/
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
				/* The server port does not support		*/
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_SET_TIMEOUTS:		/* set the comm timeout in ms. */
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

	case COMM_GET_TIMEOUTS:		/* get the comm timeout in ms	*/
		{	
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

			*(COMM_TIMEOUTS *)pBuffer = pPort->toTimeouts;
		}
		break;

	case COMM_PURGE_TXCLEAR:	/* clear the transmit buffer	*/
		{
			/* no action	*/
			if (pPort->nWorkMode == COMM_LOCAL_SERVER)
			{
				/* The server port does not support		*/
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_PURGE_RXCLEAR:	/* clear the receive buffer		*/
		{
			if (pPort->nWorkMode != COMM_LOCAL_SERVER)
			{
				COMM_TIMEOUTS	toOld  = pPort->toTimeouts;	// save timeout
				char			szTemp[256];

				// set the new timeout before controlling
				INIT_TIMEOUTS_EX(pPort->toTimeouts, 100, 100, 10 );

				// to receive all data
				while (TCPIP_CommRead(hPort, szTemp, sizeof(szTemp)) > 0)
				{
					;
				}
			
				pPort->toTimeouts = toOld;	// restore timeouts
			}

			/* The server port does not support		*/
			else /* if (pPort->nWorkMode == COMM_LOCAL_SERVER) */
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_SET_MAX_CLIENTS:	/* set the maximum allowed clients	*/
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER)	/* Must be server	*/
			{	
				if ((*(int *)pBuffer > 0)				 /* client must > 0	*/
					&& (*(int *)pBuffer <= TCPIP_MAX_CLIENT_LIMITAION))
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

	case COMM_GET_MAX_CLIENTS:		/* get the maximum allowed clients */
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER) /* Must be server	*/
			{
				*(int *)pBuffer = pPort->nMaxClients;
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

		/* get the number of the current connected clients */
	case COMM_GET_CUR_CLIENTS:
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER) /* Must be server	*/
			{
				*(int *)pBuffer = *pPort->pCurClients;
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}

		break;

	case COMM_GET_CLIENT_LIMITATION:	/* get the maximum client limitation */
		{
			ASSERT(nDataLength == sizeof(int));

			if (pPort->nWorkMode == COMM_LOCAL_SERVER) /* Must be server	*/
			{
				*(int *)pBuffer = TCPIP_MAX_CLIENT_LIMITAION;
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

int TCPIP_CommClose( IN HANDLE hPort )
{
	TCPIP_PORT_DRV *pPort = (TCPIP_PORT_DRV *)hPort;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	//Jimmy note 20210302: we do not have multi-process case, so that we shut it even for server
	//if (pPort->nWorkMode != COMM_LOCAL_SERVER)
	{
		/* */
		shutdown(pPort->nSocket, SD_BOTH);
	}

	CLOSE_SOCKET(pPort->nSocket);		/* close the socket handle	*/

	/* descrease the linkage, the pCurClients shall be deleted if no
	 * any clients. But the server and clients share the same pCurClients,
	 * and the initialing value of *pCurClients is 0, so the condition to
	 * delete pCurClients is *pCurClients < 0.
	 */
#ifdef _DEBUG_TCPIP_PORT
	TRACE("After closing *pPort->pCurClients is %d\n", *pPort->pCurClients);
#endif
	//(*pPort->pCurClients)--;
	//if (*pPort->pCurClients < 0)
	if(pPort->nWorkMode == COMM_LOCAL_SERVER) //changed by Jimmy 20210302
	{
#ifdef _DEBUG_TCPIP_PORT
		TRACE("After2 closing *pPort->pCurClients is %d\n", *pPort->pCurClients);
#endif
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

