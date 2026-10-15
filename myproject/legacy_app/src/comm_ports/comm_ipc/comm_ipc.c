
#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "comm_ipc.h"	/* The private head file for this module	*/

#ifdef _DEBUG
//#define _DEBUG_TCPIP_PORT
#endif //_DEBUG

static BOOL IPC_ParseOpenParams(IN char	*pOpenParams, IN OUT IPC_PORT_DRV* pPort)
{
	char	*pszPort = NULL;
	int nLen = (int)strlen(pOpenParams);
	if((nLen < 6) || (nLen > 63))
	{
		TRACE("IPC file length-->[%d] INVALID! shall be 6~63!\n", nLen);
		return FALSE;
	}
	BOOL bHeadCorrect = TRUE;
	bHeadCorrect = bHeadCorrect && (pOpenParams[0] == 'i');
	bHeadCorrect = bHeadCorrect && (pOpenParams[1] == 'p');
	bHeadCorrect = bHeadCorrect && (pOpenParams[2] == 'c');
	bHeadCorrect = bHeadCorrect && (pOpenParams[3] == ':');
	bHeadCorrect = bHeadCorrect && (pOpenParams[4] == '/');
	bHeadCorrect = bHeadCorrect && (pOpenParams[5] == '/');
	ZERO_POBJS(pPort->szIPC_File_RR, 64);
	ZERO_POBJS(pPort->szIPC_File_PP, 64);
	snprintf(pPort->szIPC_File_RR, 64, "%s.RR", pOpenParams);
	snprintf(pPort->szIPC_File_PP, 64, "%s.PP", pOpenParams);

	return bHeadCorrect;
}

HANDLE IPC_CommOpen(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD		dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode )
{
	IPC_PORT_DRV *pPort = NEW(IPC_PORT_DRV, 1);

	UNUSED(pPortDescriptor);

	/* 1. get mem */
	if (pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return	NULL;
	}

	memset(pPort, 0, sizeof(IPC_PORT_DRV));

	/* 2. parse IP and socket port info */
	if (!IPC_ParseOpenParams(pOpenParams, pPort))
	{
		TRACE("[IPC_CommOpen] -- invalid parameters: \"%s\".\n",
			pOpenParams);

		DELETE( pPort );
		*pErrCode = ERR_COMM_OPENING_PARAM;

		return NULL;
	}

	INIT_TIMEOUTS( pPort->toTimeouts, nTimeout, nTimeout);

	//Jimmy note: REQ-REP shall be "paired", meaning only one message can be sent out to client after "req"
	//so that we must use NN_PAIR mode instead.
	//pPort->nSocket_RR = nn_socket(AF_SP, NN_PAIR); //Jimmy note:pair mode does not seem OK, so we must use REQ_REP & PUSH-PULL together
	if((dwPortAttr & COMM_CLIENT_MODE) == COMM_CLIENT_MODE)
	{
		pPort->nSocket_RR = nn_socket(AF_SP, NN_REQ);
		pPort->nSocket_PP = nn_socket(AF_SP, NN_PULL);
	}
	else
	{
		pPort->nSocket_RR = nn_socket(AF_SP, NN_REP);
		pPort->nSocket_PP = nn_socket(AF_SP, NN_PUSH);
	}


	if ((pPort->nSocket_RR == INVALID_SOCKET) || (pPort->nSocket_PP == INVALID_SOCKET))
	{
		DELETE( pPort );
		*pErrCode = ERR_COMM_OPENING_PARAM;

		return NULL;
	}
	
	/* to connect to a remote server, create a client port	*/
	if((dwPortAttr & COMM_CLIENT_MODE) == COMM_CLIENT_MODE)
	{
		pPort->nWorkMode = COMM_INCOMING_CLIENT;
		TRACEX("IPC Comm Client Opened!!!\n");
		if(nn_connect(pPort->nSocket_RR, pPort->szIPC_File_RR) < 0)
		{
			DELETE( pPort );
			*pErrCode = ERR_COMM_CONNECT_SERVER;
			return NULL;
		}
		//pull is server
		int nErrCode = nn_bind(pPort->nSocket_PP, pPort->szIPC_File_PP);
		if(nErrCode < 0)
		{
			TRACEX("Bind Socket_PP Failed:%d, %s.\n",nErrCode, nn_strerror(nn_errno()));
			DELETE( pPort );
			*pErrCode = ERR_COMM_CONNECT_SERVER;
			return NULL;
		}
	}
	else
	{
		pPort->nWorkMode = COMM_LOCAL_SERVER;
		TRACEX("IPC Comm Server Opened!!!\n");
		int nErrCode = nn_bind(pPort->nSocket_RR, pPort->szIPC_File_RR);
		if(nErrCode < 0)
		{
			TRACEX("Bind Socket_RR Failed:%d, %s.\n",nErrCode, nn_strerror(nn_errno()));
			DELETE( pPort );
			*pErrCode = ERR_COMM_CREATE_SERVER;
			return NULL;
		}
		//push is client
		if(nn_connect(pPort->nSocket_PP, pPort->szIPC_File_PP) < 0)
		{
			DELETE( pPort );
			*pErrCode = ERR_COMM_CONNECT_SERVER;
			return NULL;
		}
	}

	pPort->nIPCMode = IPC_MODE_REQ_REP;//default work on REQ-REP mode

	*pErrCode = ERR_COMM_OK;

	//TRACE("\npOpenParams is: %s", pOpenParams);
	//TRACE("\npPort is: %d", pPort);
	return (HANDLE)pPort;
}

HANDLE IPC_CommAccept( IN HANDLE hPort)
{
	return hPort;
}

static int WaitIPCRead_Writeable(HANDLE hPort, int timeout, BOOL bWaitRead)
{
	IPC_PORT_DRV *pPort = (IPC_PORT_DRV *)hPort;
	struct nn_pollfd pfd;
	pfd.fd = (pPort->nIPCMode == IPC_MODE_REQ_REP) ? pPort->nSocket_RR : pPort->nSocket_PP;
	pfd.events = (bWaitRead) ? NN_POLLIN : NN_POLLOUT;

	//TRACE("\nnnPoll wait %d msec.\n", timeout);

	int rc = nn_poll(&pfd, 1, timeout);

	if(rc > 0)
	{
		if(((pfd.revents & pfd.events) == pfd.events))
		{
			return rc;
		}
		else
		{
			return -1;
		}
	}

	return rc;
}

#define COM_RETRY_TIME	3

int IPC_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	IPC_PORT_DRV		*pPort = (IPC_PORT_DRV *)hPort;
	int				rc;
	int				fd;// = pPort->nSocket;
	int				nTotalBytesRead = 0;	// total bytes of data being read
	int				nRetryTimes		= 0;

	char* szBuff = pBuffer;
	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;
	if(pPort->nIPCMode == IPC_MODE_REQ_REP)
	{
		fd = pPort->nSocket_RR;
	}
	else
	{
		if(pPort->nWorkMode == COMM_LOCAL_SERVER)
		{
			pPort->nLastErrorCode = ERR_COMM_WRONG_IPC_MODE;
			return 0;
		}
		fd = pPort->nSocket_PP;
	}
	rc = WaitIPCRead_Writeable(hPort, pPort->toTimeouts.nReadTimeout, TRUE);
	if(rc <= 0)
	{
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_READ_DATA;

		//TRACE("[IPC_CommRead,Mode:%d] -- Read data error(%x)\n", pPort->nIPCMode, pPort->nLastErrorCode);

		return 0;	// nothing has been read
	}

	//----------------------Jimmy Note: NN_Msg has to send/receive pair. CANNOT read many times, or else there will be error------------------------------

	//int nWatiTime = pPort->toTimeouts.nReadTimeout;
	BYTE* pReadBuff = (BYTE*) pBuffer;
	//while ((nBytesToRead > 0)/* && (nWatiTime > 0)*/)
	{
		rc = nn_recv(fd, (void*)pReadBuff, (size_t)nBytesToRead, NN_DONTWAIT);
		
		if(rc > 0)	/* we got some data */
		{
			// ok
			nBytesToRead	-= rc;
			pReadBuff		+= rc;
			nTotalBytesRead += rc;

			if (nBytesToRead <= 0)
			{
				//break;// reading finished
			}

			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}
		//else if (rc < 0)
		//{
		//	if(nTotalBytesRead > 0) //has already read all
		//	{
		//		//break;
		//	}
		//	else if(nn_errno() != EAGAIN) //other err besides 'no message ready'
		//	{
		//		nRetryTimes++;		// let's retry again
		//		if (nRetryTimes >= COM_RETRY_TIME)
		//		{
		//			pPort->nLastErrorCode = ERR_COMM_READ_DATA;

		//			TRACE("[IPC_CommRead] -- Read data error(%d), "
		//				"has retried %d times.\n", nn_errno(), nRetryTimes);

		//			//break;	// error, quit now.
		//		}
		//	}
		//}
		//else if (rc == 0)	// the socket is shutdown, gracefully closed.
		//{
		//	pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
		//	TRACE("[IPC_CommRead] -- The socket is shutdown.\n");
		//	//break;
		//}

		// if error, we also sleep a while and retry again.
		//rc = WaitIPCRead_Writeable(hPort, pPort->toTimeouts.nReadTimeout, TRUE);
		//if( rc <= 0 )
		//{
		//	/* 0: wait time out, -1: fd error on waiting    */
		//	pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
		//		: ERR_COMM_READ_DATA;

		//	TRACE("[IPC_CommRead] -- The socket failed %s after read %d "
		//		"bytes.\n", rc == 0 ? "timeout" : "select" , nTotalBytesRead);

		//	break;
		//}
		//nWatiTime -= 10;
	}

	//TRACE("[IPC_CommRead] -- End The socket read %d bytes(err code: %d), szBuff = [%s].\n",
	//	nTotalBytesRead, pPort->nLastErrorCode, szBuff);

	return nTotalBytesRead;
}

int IPC_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite)
{
	IPC_PORT_DRV		*pPort = (IPC_PORT_DRV *)hPort;
	int				rc;
	int				fd;//   = pPort->nSocket;
	int				nTotalBytesWritten = 0;	// total bytes of data being written
	int				nRetryTimes		= 0;

	char* szBuff = pBuffer;

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;
	if(pPort->nIPCMode == IPC_MODE_REQ_REP)
	{
		fd = pPort->nSocket_RR;
	}
	else
	{
		if(pPort->nWorkMode != COMM_LOCAL_SERVER)
		{
			pPort->nLastErrorCode = ERR_COMM_WRONG_IPC_MODE;
			return 0;
		}
		fd = pPort->nSocket_PP;
	}

	//rc = WaitIPCRead_Writeable(hPort, pPort->toTimeouts.nWriteTimeout, FALSE);
	//if( rc <= 0 )
	//{
	//	pPort->nLastErrorCode = 
	//		(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_WRITE_DATA;

	//	TRACE("[IPC_CommWrite, Mode:%d] -- Write data error(%x)\n", pPort->nIPCMode, pPort->nLastErrorCode);

	//	return 0;	// nothing has been read
	//}

	//----------------------Jimmy Note: NN_Msg has to send/receive pair. CANNOT nn_poll(do not know why??) many times, or else there will be error------------------------------
	
	while (nBytesToWrite > 0)
	{

		rc = nn_send(fd, pBuffer, (size_t)nBytesToWrite, NN_DONTWAIT);
		
		if (rc > 0)	/* we wrote out some data */
		{
			// ok
			nBytesToWrite	   -= rc;
			pBuffer			   += rc;
			nTotalBytesWritten += rc;

			if (nBytesToWrite == 0)
			{
				break;	// writing finished
			}

			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}
		else if ((rc < 0) && (nn_errno() != EAGAIN))		// write error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_WRITE_DATA;

				TRACE("[IPC_CommWrite] -- Write data error(%d), "
					"has retried %d times.\n", nn_errno(), nRetryTimes);

				break;	// error, quit now.
			}
		}

		// if error, we also sleep a while and retry again.
		rc = WaitIPCRead_Writeable(hPort, pPort->toTimeouts.nWriteTimeout, FALSE);
		if( rc <= 0 )
		{
			/* 0: wait time out, -1: fd error on waiting    */
			pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
				: ERR_COMM_WRITE_DATA;
			TRACE("[IPC_CommWrite, Mode:%d] -- Write data error(%x)\n", pPort->nIPCMode, pPort->nLastErrorCode);
			break;
		}
	}

	//TRACE("[IPC_CommWrite] -- End The socket write %d bytes(err code: %d), szBuff = [%s].\n",
	//	nTotalBytesWritten, pPort->nLastErrorCode, szBuff);

	return nTotalBytesWritten;
}


int IPC_CommControl(IN HANDLE hPort, IN int	nCmd, 
					  IN OUT void *pBuffer,	IN int	nDataLength)
{
	IPC_PORT_DRV *pPort = (IPC_PORT_DRV *)hPort;
	int nErrCode = ERR_COMM_OK;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	ASSERT( pBuffer );

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
		}

		break;

	case COMM_SET_IPC_MODE:		/* set IPC Mode. */
		{
			ASSERT(nDataLength == sizeof(int));

			pPort->nIPCMode = *(int *)pBuffer;
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
				while (IPC_CommRead(hPort, szTemp, sizeof(szTemp)) > 0)
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

	default:
		{
			nErrCode = ERR_COMM_CTRL_COMMAND;
		}
	}

	return nErrCode;
}

int IPC_CommClose( IN HANDLE hPort )
{
	IPC_PORT_DRV *pPort = (IPC_PORT_DRV *)hPort;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	nn_close(pPort->nSocket_RR);		/* close the socket handle	*/
	nn_close(pPort->nSocket_PP);		/* close the socket handle	*/

	DELETE(pPort);

	return ERR_COMM_OK;
}

