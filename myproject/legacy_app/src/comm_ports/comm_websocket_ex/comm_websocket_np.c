
#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "comm_websocket_np.h"	/* The private head file for this module	*/
#include "nopoll_private.h"

#ifdef _DEBUG
	#define _DEBUG_WS_PORT
#endif //_DEBUG
static unsigned long          tmLastReceivePong;
//copy from libwebsockets
BOOL Pasre_WSAddr_FromUrl(char *p, const char **prot, const char **ads, int *port, const char **path)
{
	const char *end;
	static const char *slash = "/";

	/* cut up the location into address, port and path */
	*prot = p;
	while (*p && (*p != ':' || p[1] != '/' || p[2] != '/'))
		p++;
	if (!*p) 
	{
		end = p;
		p = (char *)*prot;
		*prot = end;
	} 
	else 
	{
		*p = '\0';
		p += 3;
	}
	*ads = p;
	if (!strcmp(*prot, "http") || !strcmp(*prot, "ws"))
	{
		*port = 80;
	}		
	else if (!strcmp(*prot, "https") || !strcmp(*prot, "wss"))
	{
		*port = 443;
	}
	else
	{
		return FALSE;
	}	

    if (*p == '[')
    {
        ++(*ads);
        while (*p && *p != ']')
                p++;
        if (*p)
                *p++ = '\0';
    }
    else
    {
        while (*p && *p != ':' && *p != '/')
        	p++;
    }
	if (*p == ':') {
		*p++ = '\0';
		*port = atoi(p);
		while (*p && *p != '/')
			p++;
	}
	*path = slash;
	if (*p) {
		*p++ = '\0';
		if (*p)
			*path = p;
	}

	return TRUE;
}

void cb_loghandler(noPollCtx * ctx, noPollDebugLevel level, const char * log_msg, noPollPtr user_data)
{
	UNUSED(ctx);
	UNUSED(user_data);
	//Jimmy will think if need log into db in future
    // if (level == NOPOLL_LEVEL_CRITICAL) 
	// {
    //     printf ("[%.1f] CRITICAL: %s\n", GetCurrentTime(), log_msg);
    // }
	// else 
	// {
	// 	printf ("[%.1f] INFO: %s\n", GetCurrentTime(), log_msg);
	// }
}

noPollCtx * create_ctx(BOOL bNeedLogCB) 
{
    /* create a context */
    noPollCtx * ctx = nopoll_ctx_new ();
#ifdef _DEBUG
    nopoll_log_enable (ctx, nopoll_true);
    nopoll_log_color_enable (ctx, nopoll_true);
#endif
    /* configure handler */
    if (bNeedLogCB)
	{
		nopoll_log_set_handler (ctx, cb_loghandler, NULL);
	}
            
    return ctx;
}


static char s_szTLS_CAFilePath[255] = ""; //Jimmy notice20190223: this should be assigned by Control API
static char s_szBase64_RawOutstring[256] = {0};
//remove the '\r\n' char for the base64 string 
char* Remove_RN_4Base64Str(OUT char* szOut)
{
	int n;
	int s = 0;
	for(n = 0; n < 256; n++)
	{
		if(s_szBase64_RawOutstring[n] == '\0')
		{
			szOut[s] = '\0';
			break;
		}
		else if((s_szBase64_RawOutstring[n] != '\r') && (s_szBase64_RawOutstring[n] != '\n'))
		{
			szOut[s] = s_szBase64_RawOutstring[n];
			s++;
		}
	}

	return szOut;
}

HANDLE WS_CommOpen(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD	dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode)
{
	//Jimmy 20210819: use portDescriotion for HTTP Baisic param
	//Jimmy note: portDescriotion is used for protocol name
	// if(pPortDescriptor == NULL)
	// {
	// 	*pErrCode = ERR_COMM_OPENING_PARAM;
	// 	return NULL;
	// }

	char szLwsAddr[256];
	memset(szLwsAddr, 0, 256);

	if (pOpenParams == NULL)
	{
		//TRACE("----error #11111111----\n");
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}
	else
	{
		memcpy(szLwsAddr, pOpenParams, strlen(pOpenParams));
	}

	//use this as the protocol
	char szProtocol[16] = {"ocpp1.6"};
	if(dwPortAttr == COMM_OCPP1_6_MODE)
	{
		snprintf(szProtocol, 16, "ocpp1.6");
	}
	else if(dwPortAttr == COMM_INFYMBE_MODE)
	{
		snprintf(szProtocol, 16, "infy_mbe");
	}
	else if(dwPortAttr == COMM_OCPP2_0_1_MODE)
	{
		snprintf(szProtocol, 16, "ocpp2.0.1");
	}
	else if(dwPortAttr == COMM_BINARY_MODE)
	{
		snprintf(szProtocol, 16, "websocketBinary");
	}
	else
	{
		//printf("----error #222222222222----\n");
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}

	WS_DRIVER_DATA *pPort = NEW(WS_DRIVER_DATA, 1);	
	if(pPort == NULL)
	{
		//printf("----error #333333333----\n");
		*pErrCode = ERR_COMM_NO_MEMORY;
		return NULL;
	}
	ZERO_POBJS(pPort, 1);

	//John 20220424 Added For Compress() binary Data.
	pPort->dwPortAttr = dwPortAttr;

	//now create the ctx
	pPort->pCtx = create_ctx(TRUE);
	if(pPort->pCtx == NULL)
	{
		//printf("----error #44444444----\n");
		*pErrCode = ERR_COMM_NO_MEMORY;
		SAFELY_DELETE(pPort);
		return NULL;
	}

	//set the conn timeout (in microSec)	
	//!!!Jimmy note: this is just timeout for nopoll lib to send handshake but not wait complete,shall using wait ready later
	nopoll_conn_connect_timeout(pPort->pCtx, nTimeout * 1000);

	//parse url => conn params
	char *address;
	int port = 80;
	char *prot, *path;
	char szPath[128] = {0};
	//printf("Now begin parsing params[%s]\n", pOpenParams);
	if(!Pasre_WSAddr_FromUrl(szLwsAddr, &prot, &address, &port, &path))
	{
		//printf("----error #5555555----\n");
		*pErrCode = ERR_COMM_OPENING_PARAM;
		nopoll_ctx_unref(pPort->pCtx); //do not forget to release ctx
		SAFELY_DELETE(pPort);
		return NULL;
	}
	
	//Jimmy note: need add '/' prefix here
	if(path[0] != '/')
	{
		snprintf(szPath, 128, "/%s", path);
	}
	else
	{
		snprintf(szPath, 128, "%s", path);
	}

	//handle the conn
	// printf("-------now begin conn to addr[%s], port[%d], prot[%s], szPath[%s], protool[%s]------\n",
	// 	address, port, prot, szPath, pPortDescriptor);
	char szPort[32] = {0};	
	snprintf(szPort, 32, "%d", port);

	//for HTTP basic
	char szHTTPBasic_Base64[256] = {0};
	char szHTTPBasic_Header[300] = {0};
	BOOL bHasHTTPBasic = FALSE;
	if(pPortDescriptor != NULL)
	{
		int nOutLen = 256;
		memset(s_szBase64_RawOutstring, 0, 256);
		if(nopoll_base64_encode(pPortDescriptor, strlen(pPortDescriptor), s_szBase64_RawOutstring, &nOutLen))
		{
			memset(szHTTPBasic_Base64, 0, 256);
			Remove_RN_4Base64Str(szHTTPBasic_Base64);
			//printf("\n-------The encoded string is [%s], len => %d------\n", szHTTPBasic_Base64, nOutLen);
			bHasHTTPBasic = TRUE;
			//Jimmy: don't forget the '\r\n'(refer nopoll src code)
			snprintf(szHTTPBasic_Header, 300, "\r\nAUTHORIZATION: Basic %s", szHTTPBasic_Base64);
			// printf("-----The BASE-64 for [%s, %d] => [%d, %s]----\n", 
			// 	pPortDescriptor, strlen(pPortDescriptor), strlen(szHTTPBasic_Base64), szHTTPBasic_Header);
		}
	}

	if (!strcmp(prot, "wss"))
	{
		//for SSL
		noPollConnOpts *opts = nopoll_conn_opts_new ();
		if(bHasHTTPBasic)
		{
			nopoll_conn_opts_set_extra_headers(opts, szHTTPBasic_Header);
		}

		if(!nopoll_conn_opts_set_ssl_certs (opts,
                    /* certificate */
                    NULL,
                    /* private key */
                    NULL,
					/*charin certificate*/
                    NULL,
                    /* ca certificate */
                    s_szTLS_CAFilePath))
		{
			nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);
			//printf("-----Can not load CA certificate so we disabled the peer-verify-----\n");
		}
		//printf("-------Now successfylly loaded the CA-certificate=>%s, and set the SSL verify to FALSE-------\n", s_szTLS_CAFilePath);
		nopoll_conn_opts_ssl_peer_verify (opts, nopoll_false);
		pPort->pConn = nopoll_conn_tls_new (pPort->pCtx, opts, address, szPort, NULL, szPath, szProtocol, NULL);
	}
	else if (!strcmp(prot, "ws"))
	{		
		if(bHasHTTPBasic)
		{
			noPollConnOpts *opts = nopoll_conn_opts_new ();
			nopoll_conn_opts_set_extra_headers(opts, szHTTPBasic_Header);
			pPort->pConn = nopoll_conn_new_opts(pPort->pCtx, opts, address, szPort, NULL, szPath, szProtocol, NULL);
		}
		else
		{
			pPort->pConn = nopoll_conn_new(pPort->pCtx, address, szPort, NULL, szPath, szProtocol, NULL);
		}		
	}

	//check if port&its socket is created OK
	if(!nopoll_conn_is_ok(pPort->pConn))
	{
		//printf("----error #6666666----\n");
		nopoll_conn_close(pPort->pConn);
		nopoll_ctx_unref(pPort->pCtx); //do not forget to release ctx
		SAFELY_DELETE(pPort);
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}	

	BOOL bConnOK = FALSE;
	/*
		pravite param. not nopoll.
		pPort->pConn->tmLastReceivePong = 0;

	*/
	tmLastReceivePong = 0;
	//still wait nTimeout msec
    do
	{
        if (!nopoll_conn_is_ok (pPort->pConn)) 
		{
            //printf ("ERROR (4.1 jg72): expected to find proper connection handshake finished, but found connection is broken: session=%d, errno=%d : %s..\n",
            //    (int) nopoll_conn_socket (pPort->pConn), errno, strerror (errno));
            bConnOK = FALSE;
			break;
        } 
		else 
		{
			if(nopoll_conn_is_ready (pPort->pConn))
			{
				bConnOK = TRUE;
				break;
			}
		}

        /* wait a bit 10ms */
        nopoll_sleep (10000);
		nTimeout -= 10;

    }while(nTimeout > 0);

	//check if connecton ready
	if(!bConnOK)
	{
		//printf("----error #777777----\n");
		nopoll_conn_close(pPort->pConn);
		nopoll_ctx_unref(pPort->pCtx); //do not forget to release ctx
		SAFELY_DELETE(pPort);
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}

	INIT_TIMEOUTS(pPort->toTimeouts, nTimeout, nTimeout);

	*pErrCode = ERR_COMM_OK;

	pPort->bConnected = TRUE;

	return (HANDLE)pPort;
}

HANDLE WS_CommAccept( IN HANDLE hPort )
{
	//do nothing
	return NULL; //do not support yet
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
	else if(!nopoll_conn_is_ok (pPort->pConn)) //means connection broken
	{
		pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
		return -1;
	}

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;
	int nReadoutBytesOnce = 0;
	memset(pBuffer, 0, nBytesToRead);
	//true means block mode, do NOT use block mode as it shall return soon after read done
	nTotalBytesRead = nopoll_conn_read(pPort->pConn, pBuffer, nBytesToRead, nopoll_false, nWait_mSec);

	if(nTotalBytesRead < nBytesToRead)
	{
		pPort->nLastErrorCode = 
			(nTotalBytesRead <= 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_READ_DATA;
	}

	//added by Jimmy as -1 is already used for judge connection broken
	if(nTotalBytesRead < 0)
	{
		nTotalBytesRead = 0;
	}	

	//printf("[WS_CommRead] -- End The WS read[%s], %d bytes(err code: %d).\n",
	//	pBuffer, nTotalBytesRead, pPort->nLastErrorCode);

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
	else if(!nopoll_conn_is_ok (pPort->pConn)) //means connection broken
	{
		pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
		TRACEX("WS Send ERROR##1 : ERR_COMM_CONNECTION_BROKEN \n");
		return 0;
	}

	// if(nBytesToWrite > WS_SENDBUFF_SIZE)
	// {
	// 	pPort->nLastErrorCode = ERR_COMM_BUFFER_SIZE;
	// 	TRACEX("WS Send ERROR : ERR_COMM_BUFFER_SIZE \n");
	// 	return 0;
	// }

	if(pPort->dwPortAttr == COMM_BINARY_MODE) //John 20220424,Compress()data Use Binary Send To Server.Header Opcode Binary(2)
	{
		nTotalBytesWritten = nopoll_conn_send_binary(pPort->pConn, pBuffer, nBytesToWrite);
	}
	else
	{
		nTotalBytesWritten = nopoll_conn_send_text(pPort->pConn, pBuffer, nBytesToWrite);
	}
	//TRACEX("-----Now send the text=>[%s], totalSent=>%d, %d dwPortAttr=[%d]-----\n", pBuffer, nBytesToWrite, nTotalBytesWritten, pPort->dwPortAttr);
	if(nTotalBytesWritten != nBytesToWrite)	
	{
		pPort->nLastErrorCode = ERR_COMM_WRITE_DATA;
		return 0;
	}

	pPort->nLastErrorCode = ERR_COMM_OK;
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
			if(nopoll_conn_is_ok (pPort->pConn))
			{
				nopoll_conn_send_ping(pPort->pConn);
				TRACEX("now send out ws ping\n");				
			}
		}
		break;

	case COMM_GET_LASTTIME_RECEIVE_PONG:
		{
			ASSERT(nDataLength == sizeof(time_t));
			if(nopoll_conn_is_ok (pPort->pConn))
			{
				//*(time_t *)pBuffer = pPort->pConn->tmLastReceivePong;
				*(time_t *)pBuffer = tmLastReceivePong;
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

int WS_CommClose( IN HANDLE hPort)
{
	WS_DRIVER_DATA *pPort = (WS_DRIVER_DATA *)hPort;

	if(hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	TRACEX("----Now close existing wssock-----\n");
	nopoll_conn_close(pPort->pConn);
	nopoll_ctx_unref(pPort->pCtx); //do not forget to release ctx
	SAFELY_DELETE(pPort);	

	return ERR_COMM_OK;
}

