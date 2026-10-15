
#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "comm_socketcan.h"	/* The private head file for this module	*/
#include "3rd_party/libsocketcan/can_netlink.h"
#include "3rd_party/libsocketcan/libsocketcan.h"

#define CAN_FRAME_LENGTH 16
#define CAN_HEAD_LEN	4
typedef struct tagCan_bitratemap
{
	COMM_CAN_BAUDRATE	emLevel;
	UINT					ulBaudRate;
}BAUD_MAP;

static UINT GetBaud_ByLevel(DWORD lvel)
{
	static BAUD_MAP map[] = 
	{
		{COMM_CAN_BAUDRATE_50K,		50000},
		{COMM_CAN_BAUDRATE_100K,		100000},
		{COMM_CAN_BAUDRATE_125K,		125000},
		{COMM_CAN_BAUDRATE_250K,		250000},
		{COMM_CAN_BAUDRATE_500K,		500000},
		{COMM_CAN_BAUDRATE_800K,		800000},
		{COMM_CAN_BAUDRATE_1M,		1000000}

	};

	int n;
	for(n = 0; n < ITEM_OF(map); n++)
	{
		if(lvel == map[n].emLevel)
		{
			return map[n].ulBaudRate;
		}
	}

	return 0;
}

HANDLE CAN_CommOpen(IN char *pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD		dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode)
{
	CAN_PORT_DRV *pPort = NEW(CAN_PORT_DRV, 1);
	if (pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return	NULL;
	}
	
	memset(pPort, 0, sizeof(CAN_PORT_DRV));

	UNUSED(pOpenParams);

	pPort->nSocket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if(pPort->nSocket < 0)
	{
		TRACE("[Open CAN Socket] -- call socket() fail: %s.\n",
			strerror(SOCK_ERRNO()));
		*pErrCode = ERR_COMM_OPENING_PORT;
		return NULL;
	}

	//1. parse can1/can2
	struct sockaddr_can addr;
	struct ifreq ifr;

	if(strcmp(pPortDescriptor, "can1") == 0)
	{
		strcpy(ifr.ifr_name, "can0");
		//Jimmy added 20200704, enlarge the queue so that
		//write buffer is enough to hold more CAN frames!
		//default 10 is too small
		//Jimmy 20200704: does not work here
		//_SYSTEM("echo 1000 > /sys/class/net/can1/tx_queue_len");
	}
	else if(strcmp(pPortDescriptor, "can2") == 0)
	{
		strcpy(ifr.ifr_name, "can1");
		//Jimmy added 20200704, enlarge the queue so that
		//write buffer is enough to hold more CAN frames!
		//default 10 is too small
		//Jimmy 20200704: does not work here
		//_SYSTEM("echo 1000 > /sys/class/net/can1/tx_queue_len");
	}
	else
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}
	
	ifr.ifr_qlen = 500; // 设置期望的队列长度
    if (ioctl(pPort->nSocket, SIOCSIFTXQLEN, &ifr) < 0) 
    {
        printf("[Open CAN Socket] -- Set tx_queue_len fail: %s.\n", strerror(errno));
    }
    else
    {
        printf("[Open CAN Socket] -- Successfully set %s tx_queue_len to %d.\n", ifr.ifr_name, ifr.ifr_qlen);
    }

	strncpy(pPort->szCanPort, ifr.ifr_name, 8);
	ioctl(pPort->nSocket, SIOCGIFINDEX, &ifr);
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	
	//close loopback
	int loopback = 0; /* 0 = disabled, 1 = enabled (default) */ 
	setsockopt(pPort->nSocket, SOL_CAN_RAW, 
		CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)); 

	//2. baudrate
	UINT nBaud = GetBaud_ByLevel(dwPortAttr);
	if(nBaud == 0)
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}
	pPort->emBaudRateLevel = (COMM_CAN_BAUDRATE)dwPortAttr;
	can_do_stop(ifr.ifr_name);
	can_set_bitrate(ifr.ifr_name, nBaud);
	can_do_start(ifr.ifr_name);
	UINT nRestartTimes = 0;
	can_get_restart_ms(ifr.ifr_name, &nRestartTimes);
	struct can_bittiming bitTime;
	can_get_bittiming(ifr.ifr_name, &bitTime);
	// printf("---The restartTimes for %s is => %d, set-bauddatae => %d, get-bitTiming => [%d, %d, %d, %d, %d, %d, %d, %d]---\n", 
	// 	ifr.ifr_name, nRestartTimes, nBaud, 
	// 	bitTime.bitrate, bitTime.sample_point, bitTime.tq, bitTime.prop_seg,
	// 	bitTime.phase_seg1, bitTime.phase_seg2, bitTime.sjw, bitTime.brp);

	//3. init read/write timeout
	INIT_TIMEOUTS_EX(pPort->toTimeouts, nTimeout, nTimeout, 10);

	bind(pPort->nSocket, (struct sockaddr *)&addr, sizeof(addr));
		
	//added by Jimmy 20200703: need set socket as none-block mode
	int rc;  
	rc = fcntl(pPort->nSocket, F_GETFL, 0);  
	fcntl(pPort->nSocket, F_SETFL, rc | O_NONBLOCK);

	*pErrCode = ERR_COMM_OK;

	return (HANDLE)pPort;
}

HANDLE CAN_CommAccept( IN HANDLE hPort )
{
	return (HANDLE)hPort;
}

#define COM_CAN_RETRY_TIME	3

void CAN_ParseReadFrameData(OUT char *pBuffer,IN struct can_frame r_frame)
{
	//TRACE("[CAN_ParseReadFrameData] r_frame.can_dlc = %d r_frame.can_id =0x%X\n",r_frame.can_dlc,r_frame.can_id);

	BYTE byErrCode;
	byErrCode = (BYTE)(r_frame.can_id >> 29) & 0x07; //0x111
	
	if(byErrCode == 0x01) //0x0111 | 4-EFF/SFF is set in the MSB | 2-remote transmission request | 1-error frame
	{
		TRACE("byErrCode =%02X\n",byErrCode);
		return; //Err Frame skip
	}
	pBuffer[0] = (BYTE)r_frame.can_dlc;
	pBuffer[1] = (BYTE)((r_frame.can_id >> 24) & 0x1F); //扩展帧用0~28位，29,30,31定义帧类型
	pBuffer[2] = (BYTE)(r_frame.can_id >> 16);
	pBuffer[3] = (BYTE)(r_frame.can_id >> 8);
	pBuffer[4] = (BYTE)r_frame.can_id;

	r_frame.can_dlc = MIN(r_frame.can_dlc, 8);
	memcpy(&pBuffer[5],r_frame.data,r_frame.can_dlc);

	return;
}
int CAN_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	CAN_PORT_DRV	*pPort = (CAN_PORT_DRV *)hPort;
	int				rc;
	int				fd   = pPort->nSocket;
	int				nTotalBytesRead = 0;	// total bytes of data being read
	int				nRetryTimes		= 0;
	
	struct can_frame r_frame = {0};
	if(pPort == NULL)
	{
		return 0;
	}

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	if(pPort->toTimeouts.nReadTimeout > 0)
	{
		rc = WaitFiledReadable(fd, pPort->toTimeouts.nReadTimeout);
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

			TRACE("[CAN_CommRead] -- Read data error(%x)\n", pPort->nLastErrorCode);

			return 0;	// nothing has been read
		}
	}

	while (nBytesToRead > 0)
	{
//#ifdef _IMPL_UDP_PORT	// for UDP
//		nFromLen = sizeof(struct sockaddr_in);
//		rc = recvfrom(fd, pBuffer, nBytesToRead, 0, 
//			(struct sockaddr *)from, &nFromLen);
//#else	// for TCP/IP
		//rc = recv(fd, pBuffer, (size_t)nBytesToRead, 0);
		//rc = read(fd, pBuffer, (size_t)nBytesToRead);
		memset(&r_frame,0,sizeof(r_frame));
		rc = read(fd, &r_frame, sizeof(r_frame));

		TRACE("read rc = %d sizeof(r_frame) =%d\n",rc,sizeof(r_frame));
//#endif //_IMPL_UDP_PORT
		//if (rc > 0)	/* we got some data */
		if(rc == sizeof(r_frame))
		{
			CAN_ParseReadFrameData(pBuffer ,r_frame);
			/* int i;
			for(i =0;i < rc;i++)
			{
				printf("%02X",pBuffer[i]);
			}
			printf("\n");
			*/
			// ok
			nBytesToRead	-= rc;
			pBuffer		+= rc;
			nTotalBytesRead += rc;

			if (nBytesToRead < CAN_FRAME_LENGTH)
			{
				break;	// reading finished
			}

			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}
		else //if(rc == 0)// if ((rc < 0) && (errno != EAGAIN))		// read error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_CAN_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_READ_DATA;

				TRACE("[CAN_CommRead] -- Read data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);

				break;	// error, quit now.
			}
			else if(nTotalBytesRead > CAN_FRAME_LENGTH)
			{
				pPort->nLastErrorCode = ERR_COMM_OK;
				TRACE("[CAN_CommRead] -- The socket nTotalBytesRead=[%d].\n",nTotalBytesRead);
				break;
			}
		}
		// else //has error, must cease!
		// {
		// 	TRACE("[CAN_CommRead] -- There must be some error here.\n");
		// 	break;
		// }		

// #ifndef _IMPL_UDP_PORT	// for UDP
// 		else if (rc == 0)	// the socket is shutdown, gracefully closed.
// 		{
// 			pPort->nLastErrorCode = ERR_COMM_CONNECTION_BROKEN;
			
// 			TRACE("[CAN_CommRead] -- The socket is shutdown.\n");

// 			break;
// 		}
// #endif
		// if(pPort->toTimeouts.nReadTimeout <= 0)
		// {
		// 	break;
		// }
		// else
		// {
		// 	pPort->toTimeouts.nReadTimeout -= pPort->toTimeouts.nIntervalTimeout;
		// }
		
		// // if error, we also sleep a while and retry again.
		// rc = WaitFiledReadable(fd, pPort->toTimeouts.nIntervalTimeout);
		// if( rc <= 0 )
		// {
		// 	/* 0: wait time out, -1: fd error on waiting    */
		// 	pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
		// 		: ERR_COMM_READ_DATA;

		// 	TRACE("[CAN_CommRead] -- The socket failed %s after read %d "
		// 		"bytes.\n", 
		// 		rc == 0 ? "timeout" : "select" , nTotalBytesRead);

		// 	break;
		// }
    }


	TRACE("[CAN_CommRead] -- End The socket read %d bytes(err code: %d).\n",
		nTotalBytesRead, pPort->nLastErrorCode);

	return nTotalBytesRead;
}

void CAN_ParseWriteFrame(OUT struct can_frame *pw_frame, IN char *pBuffer)
{
	UINT uiRawHead = 0;
	uiRawHead = ((UINT)pBuffer[1]) << 24;
	uiRawHead += ((UINT)pBuffer[2]) << 16;
	uiRawHead += ((UINT)pBuffer[3]) << 8;
	uiRawHead += ((UINT)pBuffer[4]);
	uiRawHead = uiRawHead | CAN_EFF_FLAG ; //Extend Frame 0~28 For Data

	TRACE("----The Parsed Write Frame Head: [%08X]----------------\n",uiRawHead);
	pw_frame->can_id = uiRawHead;
	pw_frame->can_dlc = MIN(8, (UINT)pBuffer[0]);
	memcpy(pw_frame->data, &pBuffer[5], pw_frame->can_dlc);
}

int CAN_CommWrite(IN HANDLE hPort, IN char *pBuffer, IN int nBytesToWrite)
{
	CAN_PORT_DRV	*pPort = (CAN_PORT_DRV *)hPort;
	int				rc;	
	int				nTotalBytesWritten = 0;	// total bytes of data being written
	int				nRetryTimes		= 0;
	struct can_frame w_frame;
	if(pPort == NULL)
	{
		return 0;
	}

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;
	int	fd = pPort->nSocket;
	if(pPort->toTimeouts.nWriteTimeout > 0)
	{
		rc = WaitFiledWritable(fd, pPort->toTimeouts.nWriteTimeout);
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

			TRACE("[CAN_CommWrite] -- Write data error(%x)\n",
				pPort->nLastErrorCode);

			return 0;	// nothing has been read
		}		
	}

	 /*
	printf("Write nBytesToWrite =%d\n",nBytesToWrite);
	int i;
	for(i = 0;i < nBytesToWrite; i++)
	{
		printf("%02X",pBuffer[i]);
	}
	printf("\n");
	*/

	//Jimmy note 20200704: sizeof(can_frame) = 16, so nBytes2Wirte can use this to write one by one
	while (nBytesToWrite > 0)
	{
//#ifdef _IMPL_UDP_PORT
//		rc = sendto(fd, pBuffer, nBytesToWrite, 0, 
//			(struct sockaddr *)to, &nToLen);
//#else
		//rc = send(fd, pBuffer, (size_t)nBytesToWrite, 0);
		//rc = write(fd, pBuffer, (size_t)nBytesToWrite);
		memset(&w_frame, 0, sizeof(w_frame));
		CAN_ParseWriteFrame(&w_frame, pBuffer);

		//Jimmy 20200704: wait does not work here
		// rc = WaitFiledWritable(fd, pPort->toTimeouts.nIntervalTimeout);
		// if(rc <= 0)
		// {
		// 	/* 0: wait time out, -1: fd error on waiting    */
		// 	pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
		// 		: ERR_COMM_READ_DATA;
		// 	break;
		// }

		rc = write(fd, &w_frame, sizeof(w_frame)); //发送 frame[0]
		
//#endif //_IMPL_UDP_PORT
		
		if (rc == sizeof(w_frame))	/* we wrote out some data */
		{
			// ok
			nBytesToWrite		-= rc;
			pBuffer				+= rc;
			nTotalBytesWritten	+= rc;

			if(nBytesToWrite < CAN_FRAME_LENGTH)
			{
				break;	// writing finished
			}

			//added by Jimmy as if no sleep, will not be able to send more than 10 frames
			Sleep_Ex(1);

			// Need clear retry times? We don't clear it!
		}
		else //if(rc <= 0) //if ((rc < 0) && (errno != EAGAIN))		// write error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_CAN_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_WRITE_DATA;

				TRACE("[CAN_CommWrite] -- Write data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);

				break;	// error, quit now.
			}
		}
		// else //has error, must cease!
		// {
		// 	TRACE("[CAN_CommWrite] -- There must be some error here.\n");
		// 	break;
		// }
		

		// if error, we also sleep a while and retry again.
	}

	return nTotalBytesWritten;
}



int CAN_CommControl(IN HANDLE hPort, IN int nCmd, 
					  IN OUT void *pBuffer,	IN int nDataLength)
{
	/* Now we use switch to test the command, 
	 * we can optimize it if there are too many commands 
	 * which makes lower efficiency
	 */
	CAN_PORT_DRV *pPort = (CAN_PORT_DRV *)hPort;
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
			if(nDataLength != sizeof(int))
			{
				return ERR_COMM_BUFFER_SIZE;
			}

			*(int *)pBuffer = pPort->nLastErrorCode;
		}

		break;

	case COMM_SET_TIMEOUTS:		/* set the comm timeout in ms. */
		{	
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			if(nDataLength != sizeof(COMM_TIMEOUTS))
			{
				return ERR_COMM_BUFFER_SIZE;
			}

			pPort->toTimeouts = *(COMM_TIMEOUTS *)pBuffer;
		}

		break;

	case COMM_GET_TIMEOUTS:		/* get the comm timeout in ms	*/
		{
			/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
			if(nDataLength != sizeof(COMM_TIMEOUTS))
			{
				return ERR_COMM_BUFFER_SIZE;
			}

			*(COMM_TIMEOUTS *)pBuffer = pPort->toTimeouts;
		}
		break;

	case COMM_PURGE_RXCLEAR:	/* clear the receive buffer		*/
		{
			COMM_TIMEOUTS	toOld  = pPort->toTimeouts;	// save timeout			

			// set the new timeout before controlling
			INIT_TIMEOUTS_EX(pPort->toTimeouts, 100, 100, 10 );

			// to receive all data
			// while (CAN_CommRead(hPort, szTemp, sizeof(szTemp)) > 0)
			// {
			// 	;
			// }
			char			szTemp[4096];
			CAN_CommRead(hPort, szTemp, sizeof(szTemp));
			
			pPort->toTimeouts = toOld;	// restore timeouts
		}

		break;

	case COMM_GET_CAN_BAUDRATE:	/* set the maximum allowed clients	*/
		{
			if(nDataLength != sizeof(int))
			{
				return ERR_COMM_BUFFER_SIZE;
			}

			struct can_bittiming btt;
			can_get_bittiming(pPort->szCanPort, &btt);
			*(UINT *)pBuffer = (UINT)btt.bitrate;

			break;
		}

	case COMM_SET_CAN_BAUDRATE:
		{
			if(nDataLength != sizeof(int))
			{
				return ERR_COMM_BUFFER_SIZE;
			}

			UINT nBaud = GetBaud_ByLevel(*(DWORD *)pBuffer);
			if(nBaud == 0)
			{
				nErrCode = ERR_COMM_CTRL_PARAMS;
			}
			else
			{
				can_do_stop(pPort->szCanPort);
				can_set_bitrate(pPort->szCanPort, nBaud);
				can_do_start(pPort->szCanPort);
			}
			break;
		}
		
	case COMM_RESET_CAN:
		{
			can_do_restart(pPort->szCanPort);
			break;
		}

	case COMM_SET_CAN_FILTER:
		{
			//can be multiple filters
			int nFilterSize = nDataLength;
			struct can_filter *pFF = (struct can_filter*)pBuffer;
			do 
			{
				TRACE("can_id =%08X can_mask =%08X nDataLength=%d\n", pFF->can_id,pFF->can_mask, nDataLength);
				nDataLength -= sizeof(COMM_CAN_FILTER);
				pFF = (struct can_filter*)(pBuffer + sizeof(COMM_CAN_FILTER));				
			} while (nDataLength > 0);
			
			TRACE("CAN Control nDataLength =%d sizeof(COMM_CAN_FILTER) =%d sizeof(can_filter) = %d\n",
				nDataLength, sizeof(COMM_CAN_FILTER), sizeof(struct can_filter));
			
			if(nDataLength != 0)
			{
				return ERR_COMM_BUFFER_SIZE;
			}

			setsockopt(pPort->nSocket, SOL_CAN_RAW, CAN_RAW_FILTER, (struct can_filter*)pBuffer, nFilterSize);
			break;
		}

	default:
		{
			nErrCode = ERR_COMM_CTRL_COMMAND;
		}
	}

	return nErrCode;
}

int CAN_CommClose( IN HANDLE hPort )
{
	CAN_PORT_DRV *pPort = (CAN_PORT_DRV *)hPort;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	CLOSE_SOCKET(pPort->nSocket);		/* close the socket handle	*/

	can_do_stop(pPort->szCanPort); //added by Jimmy 20210107： should set canx => down after use

	//printf("----Now the %s has been CLOSED----\n", pPort->szCanPort);

	DELETE(pPort);
	

	return ERR_COMM_OK;
}

