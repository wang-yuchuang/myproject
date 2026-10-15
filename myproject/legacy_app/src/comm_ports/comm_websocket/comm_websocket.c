
#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "comm_websocket.h"	/* The private head file for this module	*/

#ifdef _DEBUG
//#define _DEBUG_WS_PORT
#endif //_DEBUG

static pthread_mutex_t s_WsGlobalLock = PTHREAD_MUTEX_INITIALIZER; //some APIs of libws are not thread-safe, so we need a public lock!
#define LOCK_WS_GLOCK()		pthread_mutex_lock(&s_WsGlobalLock)
#define UNLOCK_WS_GLOCK()		pthread_mutex_unlock(&s_WsGlobalLock)
static struct lws_context			*s_pContext = NULL; //this is public for all threads
static int						s_nWsTotal = 0; //when a new ws is created, this shall be increased
static int callback_lws_ocpp_client(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
static struct lws_protocols protocols[] =  //currently we only support 1 protocol
{
	{"es_iocpp",	callback_lws_ocpp_client,	0,	WS_RECIEVEBUFF_SIZE}, //ocpp1.6 is offical name according to OCPP-J doc
	{ NULL, NULL, 0, 0 } /* end */
};

static void* s_pCurrPort = NULL;

static char s_szTLS_CAFilePath[255] = ""; //Jimmy notice20190223: this should be assigned by Control API

#define WS_CONN_TIMEOUT_SEC	(5)

HANDLE WS_CommOpen(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD	dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode)
{
	struct lws_protocols* pCurrProtocol = NULL;
	int n;

	if(pPortDescriptor == NULL)
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}

	for(n = 0; n < ITEM_OF(protocols); n++)
	{
		if(protocols[n].name != NULL)
		{
			if(strcmp(protocols[n].name, pPortDescriptor) == 0)
			{
				pCurrProtocol = &protocols[n];
			}
		}
	}

	if(pCurrProtocol == NULL)
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}

	char szLwsAddr[256];
	memset(szLwsAddr, 0, 256);

	if (pOpenParams == NULL)
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}
	else
	{
		memcpy(szLwsAddr, pOpenParams, strlen(pOpenParams));
	}

	//open debug switch(for libwebsocket debug version only!)
#ifndef _DEBUG
	lws_set_log_level(0, NULL); //disable log
#endif

	//check and create the contex
	if(s_pContext == NULL)
	{
		TRACEX("lws Startinig...!\n");
		LOCK_WS_GLOCK();
		struct lws_context_creation_info info;
		memset(&info, 0, sizeof info);
		info.port = CONTEXT_PORT_NO_LISTEN; //here we only support ocpp client
		info.protocols = protocols;
		info.gid = -1;
		info.uid = -1;
		info.client_ssl_ca_filepath = s_szTLS_CAFilePath; //Jimmy added 20190223 for supporting root CA of SSL certs
		//info.timeout_secs = WS_CONN_TIMEOUT_SEC; //5 sec timeout
		//info.keepalive_timeout = SEC_PER_DAY; //keep alive ONE day
		//info.ka_time = 5; //no effects
		//info.ka_probes = 3;
		//info.ka_interval = 3;
		//info.ws_ping_pong_interval = pp_secs;
		//info.extensions = exts;

#if defined(LWS_WITH_TLS)
		info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
#endif
		s_pContext = lws_create_context(&info);
		UNLOCK_WS_GLOCK();

		if (s_pContext == NULL) 
		{
			TRACEX("Creating libwebsocket context failed\n");
			*pErrCode = ERR_COMM_OPENING_PORT;
			return NULL;
		}

		TRACEX("Now created lws context OK, rootCA = [%s]!\n", info.client_ssl_ca_filepath);
	}

	//now parse and create connection
	struct lws_client_connect_info conn;
	memset(&conn, 0, sizeof(conn));
	char *prot, *p;
	char path[300]; //path is sub pash for ws url
	TRACEX("Now begin parsing params[%s]\n", pOpenParams);
	if (lws_parse_uri(szLwsAddr, &prot, &conn.address, &conn.port, &p))
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}

	path[0] = '/';
	lws_strncpy(path + 1, p, sizeof(path) - 1);
	conn.path = path;
	int use_ssl = 0; //ssl support maybe tested later
	if (!strcmp(prot, "ws"))
	{
		use_ssl = 0;
	}
	else if (!strcmp(prot, "wss"))
	{
		use_ssl = LCCSCF_USE_SSL | LCCSCF_ALLOW_SELFSIGNED |
			LCCSCF_SKIP_SERVER_CERT_HOSTNAME_CHECK;
		TRACEX("-------Now use SSL flag = %d------\n", use_ssl);
	}
	else
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}
	
	WS_DRIVER_DATA *pPort = NEW(WS_DRIVER_DATA, 1);	
	if(pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return NULL;
	}
	ZERO_POBJS(pPort, 1);
	pPort->hMutex = Mutex_Create(TRUE);
	if(pPort->hMutex == NULL)
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		SAFELY_DELETE(pPort);
		return NULL;
	}

	TRACEX("Init done for lws, ssl flag = %d...\n", use_ssl);
	conn.context = s_pContext;
	conn.ssl_connection = use_ssl;
	conn.host = conn.address;
	//conn.origin = conn.address;
	conn.protocol = pCurrProtocol->name;
	//conn.ietf_version_or_minus_one = ietf_version;
	conn.pwsi = &pPort->pLws;
	conn.userdata = pPort; //this can be used by callback function
	s_pCurrPort = (void*)pPort;
	
	INIT_TIMEOUTS( pPort->toTimeouts, nTimeout, nTimeout);
	pPort->nBytesInRecBuffer = 0;
	pPort->pCurrReadPos = pPort->byRecieiveBuff;
	pPort->bConnected = 99;//if anything changed, then return
	
	/* to connect to a remote server, create a client port	*/
	if ((dwPortAttr & COMM_CLIENT_MODE) == COMM_CLIENT_MODE)
	{
		TRACEX("Now connect 2 server...\n");
		time_t tmBeginConnect = _NOW_;
		pPort->pLws = lws_client_connect_via_info(&conn);
		//lws_client_connect_via_info(&conn);
		
		if(pPort->bConnected == FALSE)
		{
			TRACEX(" pPort->pLws == NULL \n");
			*pErrCode = ERR_COMM_OPENING_PARAM;
			if(pPort->pLws != NULL)
			{
				lws_cancel_service_pt(pPort->pLws);
			}
			Mutex_Destroy(pPort->hMutex);
			SAFELY_DELETE(pPort);
			return NULL;
		}
		int nWait_mSec = /*WS_CONN_TIMEOUT_SEC * 1000;//*/nTimeout;
		nWait_mSec -= 1000 * ABS(_NOW_ - tmBeginConnect); //Jimmy noted 20190412: notice when DNS server wrong, the lws_client_connect_via_info could be blocked for too long.
		TRACEX("Now wait connction ready[%d]..\n", nWait_mSec);
		while (nWait_mSec > 0)
		{
			RUN_THREAD_HEARTBEAT();
			lws_service(s_pContext, 40);
			nWait_mSec -= 50;
			if(pPort->bConnected != 99)
			{
				break;
			}
			Sleep_Ex(10);
		}

		TRACEX("Now connected done[%d], nWait_mSec %d!\n", pPort->bConnected, nWait_mSec);

		if((pPort->bConnected == FALSE) || (pPort->bConnected == 99))
		{
			TRACEX(" Inside Now connected done[%d], nWait_mSec %d!\n", pPort->bConnected, nWait_mSec);
			*pErrCode = ERR_COMM_TIMEOUT;
			//if(s_nWsTotal < 1)
			//{
			//	LOCK_WS_GLOCK();
			//	lws_cancel_service(s_pContext);
			//	//lws_context_destroy(s_pContext);
			//	TRACEX(" Here i  am In the Now Connected \n ");
			//	//s_pContext = NULL;
			//	UNLOCK_WS_GLOCK();
			//}
			if(pPort->pLws != NULL)
			{
				lws_cancel_service_pt(pPort->pLws);
			}
			Mutex_Destroy(pPort->hMutex);
			SAFELY_DELETE(pPort);
			TRACEX(" DELETE Here i  am In the Now Connected \n ");
			return NULL;
		}
		else
		{
			LOCK_WS_GLOCK();
			s_nWsTotal++;
			UNLOCK_WS_GLOCK();
		}
	}
	else
	{
		//currently we do not support lws server!

		*pErrCode = ERR_COMM_OPENING_PARAM;
		DELETE(pPort);
		return NULL;
	}

	*pErrCode = ERR_COMM_OK;

	//TRACE("\npOpenParams is: %s", pOpenParams);
	//TRACE("\npPort is: %d", pPort);
	return (HANDLE)pPort;
}

HANDLE WS_CommAccept( IN HANDLE hPort )
{
	//do nothing
	return hPort;
}

int WS_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	WS_DRIVER_DATA	*pPort = (WS_DRIVER_DATA *)hPort;
	int				nTotalBytesRead = 0;	// total bytes of data being read
	int				nWait_mSec = pPort->toTimeouts.nReadTimeout;

	if(!pPort->bConnected)
	{
		pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
		return -1;
	}

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;
	int nReadoutBytesOnce = 0;
	memset(pBuffer, 0, nBytesToRead);

	while ((nBytesToRead - nTotalBytesRead >= 0) && (nWait_mSec > 0))
	{
		if(pPort->nBytesInRecBuffer > 0)
		{
			Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
			nReadoutBytesOnce = MIN(pPort->nBytesInRecBuffer, nBytesToRead - nTotalBytesRead);
			memcpy(pBuffer + nTotalBytesRead, pPort->byRecieiveBuff, MIN(pPort->nBytesInRecBuffer, nReadoutBytesOnce));
			nTotalBytesRead += nReadoutBytesOnce;
			pPort->nBytesInRecBuffer -= nReadoutBytesOnce;
			if(pPort->nBytesInRecBuffer > 0) //need to move data
			{
				memmove(pPort->byRecieiveBuff, pPort->byRecieiveBuff + nReadoutBytesOnce, pPort->nBytesInRecBuffer);
			}
			pPort->pCurrReadPos = pPort->byRecieiveBuff + pPort->nBytesInRecBuffer;
			Mutex_Unlock(pPort->hMutex);
		}
		//else
		//{
		//	if(nTotalBytesRead > 1)
		//	{
		//		break;
		//	}
		//}

		lws_service(s_pContext, 10);
		nWait_mSec -= 10;
	}

	if(nTotalBytesRead < nBytesToRead)
	{
		pPort->nLastErrorCode = 
			(nTotalBytesRead <= 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_READ_DATA;
	}

#ifdef _DEBUG_WS_PORT
	TRACE("[WS_CommRead] -- End The WS read %d bytes(err code: %d).\n",
		nTotalBytesRead, pPort->nLastErrorCode);
#endif //_DEBUG_WS_PORT

	return nTotalBytesRead;
}

int WS_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite)
{
	WS_DRIVER_DATA	*pPort = (WS_DRIVER_DATA *)hPort;
	int				nTotalBytesWritten = 0;	// total bytes of data being written

	if(!pPort->bConnected)
	{
		pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
		TRACEX("WS Send ERROR : ERR_COMM_CONNECTION_BROKEN \n");
		return 0;
	}

	if(nBytesToWrite > WS_SENDBUFF_SIZE)
	{
		pPort->nLastErrorCode = ERR_COMM_BUFFER_SIZE;
		TRACEX("WS Send ERROR : ERR_COMM_BUFFER_SIZE \n");
		return 0;
	}
	//first put the data into buffer, just overrite the old data
	Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
	memset(pPort->bySendBuff, 0, WS_SENDBUFF_SIZE);
	memcpy(pPort->bySendBuff, pBuffer, nBytesToWrite);
	pPort->nBytesInSendBuffer = nBytesToWrite;
	Mutex_Unlock(pPort->hMutex);

	int nWait_mSec = pPort->toTimeouts.nWriteTimeout;
	int writeInitiareRetValue = 0;
	nTotalBytesWritten = pPort->nBytesInSendBuffer;
	writeInitiareRetValue = lws_callback_on_writable_all_protocol(s_pContext, protocols);
	TRACEX("writeInitiareRetValue =%d\n", writeInitiareRetValue);
	while (nWait_mSec > 0)
	{
		lws_service(s_pContext, 50);
		if(pPort->nBytesInSendBuffer == 0)
		{
			break;
		}
		Sleep(10);
		nWait_mSec -= 10;
	}

	if(pPort->nBytesInSendBuffer > 0)
	{

		pPort->nLastErrorCode = ERR_COMM_TIMEOUT;
		TRACEX("WS Send ERROR : ERR_COMM_TIMEOUT  %d \n", pPort->nBytesInSendBuffer);
		return 0;
	}
	
	TRACEX("WS Send  :   %d \n", nTotalBytesWritten);

	nTotalBytesWritten = nTotalBytesWritten - pPort->nBytesInSendBuffer;
	return nTotalBytesWritten;
}


int WS_CommControl(IN HANDLE hPort, IN int	nCmd, 
					  IN OUT void *pBuffer,	IN int	nDataLength)
{
	/* Now we use switch to test the command, 
	 * we can optimize it if there are too many commands 
	 * which makes lower efficiency
	 */
	WS_DRIVER_DATA *pPort = (WS_DRIVER_DATA *)hPort;
	int nErrCode = ERR_COMM_OK;
	//Jimmy 20180223: this is for setting global CA file, but need to be called before lws open
	if(nCmd == COMM_SET_WS_GLOBAL_TLS_ROOT_CAFILE)
	{
		memset(s_szTLS_CAFilePath, 0, 255);
		if(pBuffer != NULL)
		{
			memcpy(s_szTLS_CAFilePath, pBuffer, MIN(255, strlen(pBuffer)));
		}
		TRACEX("---LWS: set TLS CAFile->%s---\n", s_szTLS_CAFilePath);
		return ERR_COMM_OK;
	}

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

	case COMM_SET_TIMEOUTS:		/* set the comm timeout in ms. */
		{
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

			pPort->toTimeouts = *(COMM_TIMEOUTS *)pBuffer;

			TRACEX("Set OCPP readTimout:%d, writeTimeout:%d\n", pPort->toTimeouts.nReadTimeout, pPort->toTimeouts.nWriteTimeout);
		}

		break;

	case COMM_GET_TIMEOUTS:		/* get the comm timeout in ms	*/
		{
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

			*(COMM_TIMEOUTS *)pBuffer = pPort->toTimeouts;
		}
		break;

	case COMM_SEND_WS_PING:
		{
			pPort->bNeedSendPing = TRUE;
			int writeInitiareRetValue = lws_callback_on_writable_all_protocol(s_pContext, protocols);
			TRACEX("ws_ping writeInitiareRetValue =%d, now send out ws ping\n", writeInitiareRetValue);
			lws_service(s_pContext, 200);
		}
		break;

	case COMM_GET_LASTTIME_RECEIVE_PONG:
		{
			ASSERT(nDataLength == sizeof(time_t));
			*(time_t *)pBuffer = pPort->tmLastRecievedPong;
		}
		break;

	default:
		{
			nErrCode = ERR_COMM_CTRL_COMMAND;
		}
	}

	return nErrCode;
}

int WS_CommClose( IN HANDLE hPort)
{
	WS_DRIVER_DATA *pPort = (WS_DRIVER_DATA *)hPort;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	TRACEX("----Now close existing wssock-----\n");

	//if (pPort-> bConnected == FALSE)
	//{
	//	DELETE(pPort);
	//	return ERR_COMM_OK;
	//}
	//else
	{
		if(pPort->pLws != NULL)
		{
			lws_cancel_service_pt(pPort->pLws);
		}
		//s_nWsTotal--;
		//if (s_nWsTotal <= 0)
		//{
		//	s_nWsTotal = 0;
		//	LOCK_WS_GLOCK();
		//	lws_cancel_service(s_pContext);
		//	//lws_context_destroy(s_pContext);
		//	UNLOCK_WS_GLOCK();
		//}
		Mutex_Destroy(pPort->hMutex);
		SAFELY_DELETE(pPort);
	}

	return ERR_COMM_OK;
}

static int callback_lws_ocpp_client(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	WS_DRIVER_DATA *pPort = (WS_DRIVER_DATA *)user;
	char sendBuf[LWS_PRE + WS_SENDBUFF_SIZE];
	if(pPort == NULL)
	{
		TRACEX("Invalid user pointer, for reason:%d...\n", reason);
		return 0;
	}
	else if(s_pCurrPort != pPort)//Jimmy note: old connection also use this callback, so should ignore old stuff
	{
		TRACEX("Wrong user pointer (guess it is old connection) , for reason:%d...\n", reason);
		//Jimmy note 20190112: can not call cancel or return -1, or there will be crash
		//if(reason != LWS_CALLBACK_EVENT_WAIT_CANCELLED)
		//{
		//	lws_cancel_service_pt(wsi);
		//	return -1;
		//}
		return 0;
	}

	TRACEX("Now received reason:%d\n", reason);

	int nRet = 0;
	int nWriten = 0;

	switch (reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		//Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
		TRACEX("LWS_CALLBACK_CLIENT_ESTABLISHED \n ");
		pPort->bConnected = TRUE;
		pPort->nBytesInRecBuffer = 0;
		pPort->nBytesInSendBuffer = 0;
		//Mutex_Unlock(pPort->hMutex);
		break;

	case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
		lwsl_notice("lws_http_client_http_response %d\n",
			lws_http_client_http_response(wsi));
#if defined(LWS_WITH_TLS)
		union lws_tls_cert_info_results ci;
		if (!lws_tls_peer_cert_info(wsi, LWS_TLS_CERT_INFO_COMMON_NAME,
			&ci, sizeof(ci.ns.name)))
			lwsl_notice(" Peer Cert CN        : %s\n", ci.ns.name);

		if (!lws_tls_peer_cert_info(wsi, LWS_TLS_CERT_INFO_ISSUER_NAME,
			&ci, sizeof(ci.ns.name)))
			lwsl_notice(" Peer Cert issuer    : %s\n", ci.ns.name);

		if (!lws_tls_peer_cert_info(wsi, LWS_TLS_CERT_INFO_VALIDITY_FROM,
			&ci, 0))
			lwsl_notice(" Peer Cert Valid from: %s", ctime(&ci.time));

		if (!lws_tls_peer_cert_info(wsi, LWS_TLS_CERT_INFO_VALIDITY_TO,
			&ci, 0))
			lwsl_notice(" Peer Cert Valid to  : %s", ctime(&ci.time));
		if (!lws_tls_peer_cert_info(wsi, LWS_TLS_CERT_INFO_USAGE,
			&ci, 0))
			lwsl_notice(" Peer Cert usage bits: 0x%x\n", ci.usage);
#endif
		break;

	case LWS_CALLBACK_CLIENT_CLOSED:
		//Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
		TRACEX("LWS_CALLBACK_CLIENT_CLOSED \n ");
		//pPort->pLws = NULL;
		//if not this session, then we should ignore it
		if(pPort->pLws != wsi)
		{
			return 0;
		}
		pPort->bConnected = FALSE;
		//Mutex_Unlock(pPort->hMutex);
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		//Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
		//pPort->pLws = NULL;
		//if not this session, then we should ignore it
		if(pPort->pLws != wsi)
		{
			return 0;
		}
		pPort->bConnected = FALSE;
		TRACEX("Connection failed, reason:[%s]\n", in ? (char*)in : "(null)");
		//lwsl_err("CLIENT_CONNECTION_ERROR: %s\n", in ? (char *)in : "(null)");
		//Mutex_Unlock(pPort->hMutex);
		break;

	case LWS_CALLBACK_WSI_DESTROY:
		//if not this session, then we should ignore it
		if(pPort->pLws != wsi)
		{
			return 0;
		}
		
		pPort->pLws = NULL;
		pPort->bConnected = FALSE;
		break;

	case LWS_CALLBACK_CLIENT_WRITEABLE:
		//if not this session, then we should ignore it
		if(pPort->pLws != wsi)
		{
			return 0;
		}

		if (pPort->nBytesInSendBuffer == 0)
		{
			if(pPort->bNeedSendPing)
			{
				pPort->bNeedSendPing = FALSE;
				lws_write(wsi, &sendBuf[LWS_PRE], 0, LWS_WRITE_PING); //an empty ping body!
			}
			break;
		}
		Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
		memset(sendBuf + LWS_PRE, 0, WS_SENDBUFF_SIZE);
		memcpy(sendBuf + LWS_PRE, pPort->bySendBuff, pPort->nBytesInSendBuffer);
		TRACEX("LWS_CALLBACK_CLIENT_WRITEABLE Before nWriten = [%s]  pPort->nBytesInSendBuffer = %d  \n", sendBuf + LWS_PRE, pPort->nBytesInSendBuffer);
		nWriten = lws_write(wsi, &sendBuf[LWS_PRE], pPort->nBytesInSendBuffer, LWS_WRITE_TEXT);
		if (nWriten < 0)
		{
			nRet = -1;
		}
		else
		{
			pPort->nBytesInSendBuffer -= nWriten;
			if(pPort->nBytesInSendBuffer > 0)
			{
				memmove(pPort->bySendBuff, pPort->bySendBuff + nWriten, pPort->nBytesInSendBuffer);
			}
			TRACEX("LWS_CALLBACK_CLIENT_WRITEABLE After nWriten = %d  pPort->nBytesInSendBuffer = %d  \n", nWriten, pPort->nBytesInSendBuffer);
		}
		Mutex_Unlock(pPort->hMutex);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE:
		//if not this session, then we should ignore it
		if(pPort->pLws != wsi)
		{
			return 0;
		}
		TRACEX("-----Now received wsMsg_%d:[%s]-----\n", len, (char*)in);
		if(pPort->nBytesInRecBuffer < WS_RECIEVEBUFF_SIZE - 1)
		{
			Mutex_Lock(pPort->hMutex, WAIT_INFINITE);
			memcpy(pPort->pCurrReadPos, in, MIN(len, WS_RECIEVEBUFF_SIZE - pPort->nBytesInRecBuffer));
			pPort->nBytesInRecBuffer += MIN(len, WS_RECIEVEBUFF_SIZE - pPort->nBytesInRecBuffer);
			pPort->pCurrReadPos = pPort->byRecieiveBuff + pPort->nBytesInRecBuffer;
			Mutex_Unlock(pPort->hMutex);
		}
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
		pPort->tmLastRecievedPong = _NOW_;
		TRACEX("-----Recieved pong from server now!-------\n");
		break;
	case LWS_CALLBACK_TIMER:
		break;
	case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
		//pPort->pLws = NULL;
		//lws_context_destroy(s_pContext);
		return -1;
		//break;
	
	default:
		break;
	}

	return nRet;
}

