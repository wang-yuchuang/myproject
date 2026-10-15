#include "std_sys.h"		/* all standard system head files			*/
#include "app_incs.h"
#include "commserial.h"	/* The private head file for this module	*/
#include <linux/serial.h>

#ifdef _DEBUG
	#define _DEBUG_SERIAL_PORT
#endif

//#define RTS_MAJOR_WITH232			249  
//#define IOC_SET_SEND  			_IOW(RTS_MAJOR_WITH232, 1, unsigned long)
//#define IOC_SET_RECEIVE   		_IOW(RTS_MAJOR_WITH232, 2, unsigned long)
//#define RTS_NAME_WITH232		"/dev/rts2"

// the linux supported baud rates of serial
struct SUnixBaudList
{
    unsigned long ulUnixBaud;
    int     nNormalBaud;
};

static struct SUnixBaudList s_UnixBaudList[] =
{
    { B50	  ,  50 },
    { B75	  ,  75 },
    { B110	  ,  110 },
    { B134	  ,  134 },
    { B150	  ,  150 },
    { B200	  ,  200 },
    { B300	  ,  300 },
    { B600	  ,  600 },
    { B1200	  ,  1200 },
    { B1800	  ,  1800 },
    { B2400	  ,  2400 },
    { B4800	  ,  4800 },
    { B9600	  ,  9600 },
    { B19200  ,  19200 },
    { B38400  ,  38400 },
    { B57600  ,  57600 },
    { B115200 ,  115200 },
    { B230400 ,  230400 },
    { B460800 ,  460800 },
    { B500000 ,  500000 },
    { B576000 ,  576000 },
    { B921600 ,  921600 },
    { B1000000,  1000000 },
    { B1152000,  1152000 },
    { B1500000,  1500000 },
    { B2000000,  2000000 },
    { B2500000,  2500000 },
    { B3000000,  3000000 },
    { B3500000,  3500000 },
    { B4000000,  4000000 },
    { 0,         0       } /* end flag */
};

#define MIN_SERIAL_BAUD		(s_UnixBaudList[0].nNormalBaud)
#define MAX_SERIAL_BAUD		(s_UnixBaudList[ITEM_OF(s_UnixBaudList)-2].nNormalBaud)
#define MAX_SETTING_LEN		64

static unsigned long ConvertBaudrateToUnix(int baud)
{
	struct SUnixBaudList    *p = s_UnixBaudList;
	while((p->nNormalBaud != baud) && (p->nNormalBaud != 0))
		p ++;
	
	return p->ulUnixBaud;
}


static BOOL Serial_ParseOpenParams(IN char *pSettings, OUT SERIAL_BAUD_ATTR *pAttr)
{
	char *p;
	//char *s = pSettings;
	if ((pSettings == NULL) || (*pSettings == 0) 
		|| strlen(pSettings) >= MAX_SETTING_LEN)	// too long.
	{
		return FALSE;
	}
	
	//Jimmy note: FOUND old code will change src string content!
	char szSett[32];
	snprintf(szSett, 32, "%s", pSettings);
	char *s = szSett;

	// get the baud rate set
	s = SplitString(s, &p, COMMA);

	pAttr->nBaud = atoi(p);

	pAttr->ulUnixBaud = ConvertBaudrateToUnix(pAttr->nBaud);
	
	if (pAttr->ulUnixBaud == 0)
	{
#ifdef _DEBUG_SERIAL_PORT
		TRACE("[Serial_ParseOpenParams] -- System does not support"
			" the baud rate %d.", pAttr->nBaud);
#endif
		return FALSE;
	}
	
	// odd or even check
	s = SplitString(s, &p, COMMA);

	//if (p[1] != 0) // too long
	if(strlen(p) != 1)
	{
		return FALSE;
	}
	if ((p[0] == 'N') || (p[0] == 'n'))
	{
		pAttr->nOdd = 0;
	}
	else if ((p[0] == 'O') || (p[0] == 'o'))
	{
		pAttr->nOdd = 1;
	}
	else if ((p[0] == 'E') || (p[0] == 'e'))
	{
		pAttr->nOdd = 2;
	}
	else
	{
		return FALSE;
	}
	
	// data bit - only one byte 5-8
	s = SplitString(s, &p, COMMA);
	if ((p[0] < '5') || (p[0] > '8') || (p[1] != 0))
	{
		TRACE("[Serial_ParseOpenParams] -- error on parsing data bit.\n");
		return FALSE;
	}
	pAttr->nData = p[0] - '0';

	// stop bit
	s = SplitString(s, &p, COMMA);
	if (((p[0] == '1') || (p[0] == '2')) && (p[1] == 0))
	{
		pAttr->nStop = p[0] - '0';  // 1 or 2 stop bits
	}
	else if ((p[0] == '1') && (p[1] == '.') && (p[2] == '5') && (p[3] == 0))
	{
		pAttr->nStop = 3;      // 1.5 stop bits
	}
	else
	{
		TRACE("[Serial_ParseOpenParams] -- error on parsing stop bit.\n");
		return FALSE;
	}

#ifdef _DEBUG_SERIAL_PORT
	TRACE("[Serial_ParseOpenParams] -- Parse serail settings and"
		" got %d,%d,%d,%d\n",
		pAttr->nBaud, pAttr->nOdd, pAttr->nData, pAttr->nStop);
#endif

	return TRUE;
}

static int Serial_GetDescriptor(IN char *pPortDescriptor, OUT char *pStdDescriptor)
{
	char	*pComNum = NULL;
	int	nComNumStart, nComNum;

	// 1. Test if the name is full name or not. 
	//    this comparison is case sensitive
	if (strncmp(pPortDescriptor, TTYS_FULL_PREFIX,
		CONST_STRLEN(TTYS_FULL_PREFIX)) == 0)
	{
		// the full name is used.
		pComNum = pPortDescriptor + CONST_STRLEN(TTYS_FULL_PREFIX);
		nComNumStart = 0;	// /dev/ttySn, the n starts from 0.
	}

	// 2. to test if only the name, the /dev/ does not exist. 
	//    this comparison is case sensitive
	else if (strncmp(pPortDescriptor, TTYS_ONLY_NAME,
		CONST_STRLEN(TTYS_ONLY_NAME)) == 0)
	{
		// only the name is used.
		pComNum = pPortDescriptor + CONST_STRLEN(TTYS_ONLY_NAME);
		nComNumStart = 0;	// ttySn, the n starts from 0.
	}

	// 3. to the alias name COMx is used or not.
	//    this comparison is NOT case sensitive
	else if (strncmp(pPortDescriptor, TTYS_ALIAS_NAME,
		CONST_STRLEN(TTYS_ALIAS_NAME)) == 0)
	{
		// the alias name is used.
		pComNum = pPortDescriptor + CONST_STRLEN(TTYS_ALIAS_NAME);
		nComNumStart = 1;	// COMn, the n starts from 1.
	}

	// 4. we consider only the port num such as 1, 2,... is passed in.
	else 
	{
		pComNum = pPortDescriptor;
		nComNumStart = 1;	// only the com num 'n', the n starts from 1.
	}

	// 5. to test the comm number is correct or not.
	//    the comm num shall be a digital string.
	//    ACU only support 0-9 serial port at tatol
	nComNum = (pComNum[0]-'0') - nComNumStart;	// the actual serial port num

	if ((nComNum < 0) || (pComNum[0] < '0') || (pComNum[0] > '9')
		|| (pComNum[1] != '\0'))
	{
		TRACE("[Serial_GetDescriptor] -- The port descriptor %s is invalid.\n",
			pPortDescriptor);

		return -1;
	}

    // 6. merge the full port name. The buffer is big enough.
	sprintf(pStdDescriptor, "%s%d",
		TTYS_FULL_PREFIX,	// /dev/ttyS
		nComNum);			// actual comm num from 0

#ifdef _DEBUG_SERIAL_PORT
	TRACE("[Serial_GetDescriptor] -- convert %s got %s\n",
		pPortDescriptor, pStdDescriptor);
#endif
	
	return nComNum;
}

static int cfsetdatabits(struct termios *options, int nDataBits)
{
	int n;
	int nUnixDataBits[] = { CS5, CS6, CS7, CS8 };
	n = ((nDataBits >= 5) && (nDataBits <= 8)) ? nUnixDataBits[nDataBits-5] : CS8;
	options->c_cflag &= ~CSIZE;
	options->c_cflag |= n;
	
	return n;
}


static int cfsetparity(struct termios *options, int nParity)
{
	options->c_cflag &= ~PARENB; /* default, no parity */
	if (nParity == 1) // odd
	{
		options->c_cflag |= PARENB;
		options->c_cflag |= PARODD;
	}
	else if (nParity == 2) // even
	{
		options->c_cflag |= PARENB;
		options->c_cflag &= ~PARODD;
	}

	if (nParity != 0)	// enable parity, 2004-10-8
	{
		options->c_iflag |= INPCK;
	}
	else	// disable
	{
		options->c_iflag &= ~INPCK;
	}
	
	return nParity;
}

static int cfsetstopbits(struct termios *options, int nStop)
{
	if (nStop == 1)
	{
		options->c_cflag &= ~CSTOPB;    // default is 1 stop bits
	}
	else if (nStop == 2)
	{
		options->c_cflag |= CSTOPB;
	}
	
	return nStop;
}

BOOL Serial_SetCommAttributes(IN int fd, IN SERIAL_BAUD_ATTR *pAttr)
{
	struct termios options;
	
	/* Get the current options for the port */
	tcgetattr(fd, &options);

	cfsetispeed(&options, pAttr->ulUnixBaud);	// Set the input baud rate
	cfsetospeed(&options, pAttr->ulUnixBaud);	// set the output baud rate
	
	/* Enable the receiver and set local mode */
	options.c_cflag |= (CLOCAL | CREAD);

	// data bits
	cfsetdatabits(&options, pAttr->nData);

	// stop bits
	cfsetstopbits(&options, pAttr->nStop);

	// parity
	cfsetparity(&options, pAttr->nOdd);

	options.c_cflag &= ~CRTSCTS;	/* Disable hardware flow control */  

	/* Enable data to be processed as raw input */
	options.c_lflag &= ~(ICANON | ECHO | ISIG);

	//add for the test result of YANGYUNLIANG. 
	//for can not send out the 0x11,0xD,0xA,0x13 chars.
	options.c_iflag  = 0;
	options.c_oflag &= ~OPOST;   /*Output*/	
	// end add for the test result of YANGYUNLIANG

	/* Set the new options for the port */
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		TRACE("[Serial_SetCommAttributes] -- set baud rate fail on %d.\n\r", errno);
		return FALSE;
	}

	return TRUE;
}


static int Serial_OpenPort(IN char *pszComName, IN SERIAL_BAUD_ATTR *pAttr)
{
	TRACED("Serial_OpenPort pszComName:%s", pszComName);
	int fd = open(pszComName, O_RDWR | O_NOCTTY | O_NONBLOCK | O_EXCL); 
	if (fd < 0)
	{
#ifdef _DEBUG
		perror("[Serial_OpenPort] -- open serial");
		TRACE("[Serial_OpenPort] -- open %s fail, got error %d.\n\r", 
			pszComName, errno);
		TRACE("\n Err1!!\n");
#endif 
		return -1;
	}

	// lock the com to avoid other process open again
//	if( flock( fd, LOCK_EX|LOCK_NB ) < 0 )
//	{
//#ifdef _DEBUG
//		TRACE("[Serial_OpenPort] -- try to lock %s fail, got %d\n",
//			pszComName, errno);
//#endif 	//_DEBUG
//
//		TRACE("\n Err1!!\n");
//		close( fd );
//		return -1;
//	}
	
	if (!Serial_SetCommAttributes(fd, pAttr))
	{
		close(fd);
		TRACE("\n Err2!!\n");
		return -1;
	}
	
	return fd;
}

HANDLE Serial_CommOpen(IN char *pPortDescriptor,IN char *pOpenParams,IN DWORD dwPortAttr,IN int nTimeout, OUT int *pErrCode)
{
	SERIAL_PORT_DRV *pPort = NEW(SERIAL_PORT_DRV, 1);
	
	// full port device name, like: /dev/ttyS0
	char	szFullDescriptor[sizeof(TTYS_FULL_PREFIX)+5];
	
	/* 1. get mem */
	if (pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return	NULL;
	}

	memset(pPort, 0, sizeof(SERIAL_PORT_DRV));

	/* 2. parse descriptor */
	pPort->nSerialPort = Serial_GetDescriptor(pPortDescriptor, szFullDescriptor);
	if(pPort->nSerialPort < 0)
	{
#ifdef _DEBUG_SERIAL_PORT
		TRACE("[Serial_CommOpen] -- invalid port descriptor: \"%s\".\n",
			pPortDescriptor);
#endif

		DELETE(pPort);
		*pErrCode = ERR_COMM_OPENING_PARAM;

		return NULL;
	}

	memset(pPort->szPortName, 0 , 16);
	memcpy(pPort->szPortName, szFullDescriptor, MIN(strlen(szFullDescriptor), 15));

	/* 2.1. parse comm settings */
	if (!Serial_ParseOpenParams(pOpenParams, &pPort->attr))
	{
#ifdef _DEBUG_SERIAL_PORT
		TRACE("[Serial_CommOpen] -- invalid parameters: \"%s\".\n",
			pOpenParams);
#endif
		DELETE(pPort);
		*pErrCode = ERR_COMM_OPENING_PARAM;

		return NULL;
	}

	INIT_TIMEOUTS(pPort->toTimeouts, nTimeout, nTimeout);
	//INIT_TIMEOUTS_EX(pPort->toTimeouts, nTimeout, nTimeout, 20);
	
	pPort->pCurClients = NEW(int, 1);
	if(pPort->pCurClients == NULL)
	{
		DELETE(pPort);
		*pErrCode = ERR_COMM_NO_MEMORY;

		return NULL;
	}

	*pPort->pCurClients = 0;	/*	no linkages now		*/

	// open the serial port.
	pPort->fdSerial = Serial_OpenPort(szFullDescriptor, &pPort->attr);
	if(pPort->fdSerial < 0)
	{
#ifdef _DEBUG_SERIAL_PORT
		TRACE("[Serial_CommOpen] -- Fails on opening port \"%s\" with %s.\n",
			szFullDescriptor, pOpenParams);
#endif

		DELETE(pPort->pCurClients);
		DELETE(pPort);
		*pErrCode = ERR_COMM_OPENING_PORT;

		return	NULL;
	}

	//pPort->fdRtsFor485 = open(RTS_NAME_WITH232, O_RDWR);
	//if (pPort->fdRtsFor485 < 0)
	//{
	//	printf("\n\n	@@@@@Can't open device %s.	###\n\n\n", RTS_NAME_WITH232);
	//	
	//	DELETE(pPort->pCurClients);	
	//	DELETE(pPort);
	//	*pErrCode = ERR_COMM_OPENING_PORT;
	//	return	NULL;
	//}

	//ioctl(pPort->fdRtsFor485, IOC_SET_RECEIVE, 0);

	/* to connect to a remote server, create a client port	*/
	if((dwPortAttr & COMM_CLIENT_MODE) == COMM_CLIENT_MODE)
	{
		pPort->nWorkMode = COMM_OUTGOING_CLIENT;
	}
	else
	{
		pPort->nWorkMode = COMM_LOCAL_SERVER;
		pPort->nMaxClients = SERIAL_MAX_CLIENTS_DEFAULT;
	}

	*pErrCode = ERR_COMM_OK;

	return (HANDLE)pPort;
}

HANDLE Serial_CommAccept(IN HANDLE hPort)
{
	SERIAL_PORT_DRV *pPort   = (SERIAL_PORT_DRV *)hPort;
	SERIAL_PORT_DRV *pClient = NULL;

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	Serial_SetCommAttributes(pPort->fdSerial, &pPort->attr);

	/* check the socket whether a server port or not,
 	 * only server port is allowed to call accept
	 */
	if (pPort->nWorkMode != COMM_LOCAL_SERVER)
	{
		pPort->nLastErrorCode = ERR_COMM_SUPPORT_ACCEPT;
		return NULL;
	}

	/* check the current client num exceeds the maximum connections or not */
	if (*pPort->pCurClients >= pPort->nMaxClients)
	{
		pPort->nLastErrorCode = ERR_COMM_MANY_CLIENTS;
		return NULL;
	}

	// For serial, the accept calling always return the handle directly	
	/* Ok, create a client object */
	pClient = NEW(SERIAL_PORT_DRV,1);
	if (pClient == NULL)	/* out of memory error	*/
	{
		pPort->nLastErrorCode = ERR_COMM_NO_MEMORY;
		return NULL;
	}

	*pClient			= *pPort;	/* Copy all info from server port	*/
	pClient->nWorkMode	= COMM_INCOMING_CLIENT;
	(*pPort->pCurClients)++;		/* increase the current linkage		*/

	return (HANDLE)pClient;
}

int Serial_CommRead(IN HANDLE hPort, OUT char *p, IN int nBytesToRead)
{
	SERIAL_PORT_DRV *pPort     = (SERIAL_PORT_DRV *)hPort;
	if(!pPort)
	{
		return 0;
	}
	int				fd		= pPort->fdSerial;
	int				nBytesRead = 0;
	int				nTotalBytesRead = 0;	// total bytes of data being read
	int				nRetryTimes = 0;
	int				rc;

	char*		pBuffer = p;
	//ioctl(pPort->fdRtsFor485, IOC_SET_RECEIVE, 0);

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	//printf("Return here---0\n, pPort->toTimeouts.nReadTimeout = %d time is =%d\n", pPort->toTimeouts.nReadTimeout,time(NULL));
	double fTm_BeforeRead = GetCurrentTime();
	rc = WaitFiledReadable(fd, pPort->toTimeouts.nReadTimeout);
	UINT nTimeCost = ABS((int)((GetCurrentTime() - fTm_BeforeRead) * 1000));
	// printf("\n[%d, %d]|Thread[%lu]: Serial_CommRead.WaitFiledReadable BEGIN, rc=%d\n",  
	// 	nTimeCost, pPort->toTimeouts.nReadTimeout, (int)pthread_self(), rc);
	//printf("Return here---00\n, pPort->toTimeouts.nReadTimeout = %d time is =%d\n", pPort->toTimeouts.nReadTimeout,time(NULL));
	if( rc <= 0 )
	{
		/* 0: wait time out, -1: fd error on waiting    */
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_READ_DATA;

#ifdef _DEBUG_SERIAL_PORT
			TRACE("[Serial_CommRead]-%s-- Read data error(%x)\n", pPort->szPortName,
				pPort->nLastErrorCode);
#endif
		//printf("Return here---1\n");
		return 0;	// nothing has been read
	}
	//printf("Return here--%d-2 time is %d\n", rc, time(NULL));
	while (nBytesToRead > 0)
	{
		nBytesRead = read(fd, pBuffer, (size_t)nBytesToRead);

		if (nBytesRead > 0)	/* we got some data */
		{
			// ok
			nBytesToRead	-= nBytesRead;
			pBuffer		+= nBytesRead;
			nTotalBytesRead += nBytesRead;

			if (nBytesToRead <= 0)
			{
				break;	// reading finished
			}

			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}
		else if ((nBytesRead < 0) && (errno != EAGAIN))		// read error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_READ_DATA;

#ifdef _DEBUG_SERIAL_PORT
				TRACE("[Serial_CommRead] -- Read data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
#endif
				break;	// error, quit now.
			}
		}

		nTimeCost = ABS((int)((GetCurrentTime() - fTm_BeforeRead) * 1000));
		//printf("\n[%d, %d]|Thread[%lu]: Serial_CommRead.ReadTimes [%d]\n",  nTimeCost, pPort->toTimeouts.nReadTimeout, (int)pthread_self(), nReadTimes++);
		if(nTimeCost > pPort->toTimeouts.nReadTimeout)
		{
			break;
		}

		// nothing received, data not ready? wait a while
		// if error, we also sleep a while and retry again.
		rc = WaitFiledReadable(fd, pPort->toTimeouts.nIntervalTimeout);
		if( rc <= 0 )
		{
			/* 0: wait time out, -1: fd error on waiting    */
			pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
				: ERR_COMM_READ_DATA;
			break;
		}
	}

	// if(strcmp(pPort->szPortName, "/dev/ttyS1") == 0)
	// {
	// 	//TRACE("\n------------Now begin printing the Readof232-------------\n");
	// 	printf("\nCCU-Receive:\n");
	// 	for(rc = 0; rc < nTotalBytesRead; rc++)
	// 	{
	// 		//TRACE("%02X", p[rc]);
	// 		printf("%c", p[rc]);
	// 	}
	// 	//TRACE("\n-------------------End of reading----------------------\n");
	// }

	//printf("End time is %d\n", rc, time(NULL));
	return nTotalBytesRead;
}

int Serial_CommWrite(IN HANDLE hPort, IN char *p,	IN int nBytesToWrite)
{
	SERIAL_PORT_DRV *pPort     = (SERIAL_PORT_DRV *)hPort;
	if(!pPort)
	{
		return 0;
	}
	int				fd		   = pPort->fdSerial;
	int				nBytesWritten = 0;
	int				nTotalBytesWritten = 0;	// total bytes of data being read
	int				nRetryTimes = 0;
	int				rc;
	int				iWaitTime = 0;

	iWaitTime = (int)(140 * (9600.0/pPort->attr.nBaud));
	//TRACE("iWaitTime %d\n",iWaitTime);
	char*		pBuffer = p;

	/* clear last error */
	pPort->nLastErrorCode = ERR_COMM_OK;

	rc = WaitFiledWritable(fd, pPort->toTimeouts.nReadTimeout);
	if( rc <= 0 )
	{
		/* 0: wait time out, -1: fd error on waiting    */
		pPort->nLastErrorCode = 
			(rc == 0) ? ERR_COMM_TIMEOUT  : ERR_COMM_WRITE_DATA;

#ifdef _DEBUG_SERIAL_PORT
			TRACE("[Serial_CommWrite] -%s-- Write data error(%x)\n", pPort->szPortName,
				pPort->nLastErrorCode);
#endif
		return 0;	// nothing has been read
	}
/*
	double tTime3;
	if(nBytesToWrite > 0)
	{
	   tTime3  = GetCurrentTime();
	}
	*/
	/*int i = 0,j;
	int nWLen[10000];*/
	//ioctl(iRTS, IOC_SET_SEND,0);
	//ioctl(pPort->fdRtsFor485, IOC_SET_SEND, 0);

	//TRACE("[Serial_CommWrite] -- RTS Control!!!!!\n\r");
	//TRACE("[Serial_CommWrite] -- set RTS to 1!!!!!.\n\r");
	//TRACE("[Serial_CommWrite] -- write--- %s !!!!!.\n\r",pBuffer);

	//Sleep(2);
	while (nBytesToWrite > 0)
	{
		nBytesWritten = write(fd, pBuffer, (size_t)nBytesToWrite);

		/*if(nBytesWritten > 0)
		{
		   nWLen[i] = nBytesWritten;
		   i++;
		}*/
		if (nBytesWritten > 0)	/* we got some data */
		{
			// ok
			nBytesToWrite	-= nBytesWritten;
			pBuffer		+= nBytesWritten;
			nTotalBytesWritten += nBytesWritten;

			if (nBytesToWrite <= 0)
			{
				//TRACE("time3 is %5.3f\n", tTime3);
				//TRACE("time4 is %5.3f\n", GetCurrentTime());
				/*for(j = 0;j < i; j++)
					TRACE("nWLen[%d] = %d\n",j, nWLen[j]);*/
				break;	// writing finished
			}

			if(nBytesWritten >= 128)
			   Sleep(iWaitTime);
			// Need clear retry times? We don't clear it!
			// nRetryTimes = 0;
		}
		else if ((nBytesWritten < 0) && (errno != EAGAIN))		// read error?
		{
			nRetryTimes++;		// let's retry again
			if (nRetryTimes >= COM_RETRY_TIME)
			{
				pPort->nLastErrorCode = ERR_COMM_WRITE_DATA;

#ifdef _DEBUG_SERIAL_PORT
				TRACE("[Serial_CommWrite] -- Write data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
#endif
				break;	// error, quit now.
			}
			TRACE("[Serial_CommWrite] -- Write data error(%d), "
					"has retried %d times.\n", errno, nRetryTimes);
		}

		// nothing received, data not ready? wait a while
		// if error, we also sleep a while and retry again.
		rc = WaitFiledWritable(fd, pPort->toTimeouts.nIntervalTimeout);
		if( rc <= 0 )
		{
			/* 0: wait time out, -1: fd error on waiting    */
			pPort->nLastErrorCode = (rc == 0) ? ERR_COMM_TIMEOUT
				: ERR_COMM_READ_DATA;
			break;
		}
    }

	//int iRtn = 0;
	//while(!iRtn)
	//{
	// ioctl(pPort->fdSerial, TIOCSERGETLSR, &iRtn);
	// //printf("the status of 485 is %d (0 is not over)\n ",iRtn);
	//}

	//send over then change the status of rts to receive state!

	//ioctl(pPort->fdRtsFor485, IOC_SET_RECEIVE, 0);

	//ioctl(iRTS, IOC_SET_RECEIVE,0);

	// if(strcmp(pPort->szPortName, "/dev/ttyS1") == 0)
	// {
	// 	printf("\nCCU-Send:\n");
	// 	for(rc = 0; rc < nTotalBytesWritten; rc++)
	// 	{
	// 		//TRACE("%02X", p[rc]);
	// 		printf("%c", p[rc]);
	// 	}
	// 	//TRACE("\n-------------------End of Writeing----------------------\n");
	// }

	return nTotalBytesWritten;
}

int Serial_CommControl(IN HANDLE hPort, IN int nCmd, IN OUT void *pBuffer, IN int nDataLength)
{
	/* Now we use switch to test the command, 
	 * we can optimize it if there are too many commands 
	 * which maybe make lower efficiency
	 */
	SERIAL_PORT_DRV *pPort = (SERIAL_PORT_DRV *)hPort;
	int nErrCode = ERR_COMM_OK;
	COMM_PEER_INFO *pPeerInfo;
	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	switch (nCmd)
	{
	case COMM_GET_LAST_ERROR:	/* get the last error code	*/
		if((pBuffer == NULL) || (nDataLength != sizeof(int)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
			*(int *)pBuffer = pPort->nLastErrorCode;
		}
		break;

	case COMM_GET_PEER_INFO:	/* get the sender info		*/
		pPeerInfo = (COMM_PEER_INFO *)pBuffer;
		if((pBuffer == NULL) || (nDataLength != sizeof(COMM_PEER_INFO)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
			if (pPort->nWorkMode != COMM_LOCAL_SERVER)
			{
				pPeerInfo->nPeerPort = pPort->nSerialPort;
				pPeerInfo->nPeerType = pPort->nWorkMode;
				pPeerInfo->szAddr[0] = 0;
			}
			else
			{
				/* The server port does not support		*/
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}
		break;

	case COMM_SET_TIMEOUTS:		/* set the comm timeout in ms. */
		/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
		if((pBuffer == NULL) || (nDataLength != sizeof(COMM_TIMEOUTS)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
			pPort->toTimeouts = *(COMM_TIMEOUTS *)pBuffer;
			//added by Jimmy just in case some one forget to initial the intervel_timeout
			if((pPort->toTimeouts.nIntervalTimeout < 20) || (pPort->toTimeouts.nIntervalTimeout > 100))
			{
				pPort->toTimeouts.nIntervalTimeout = TIMEOUT_INTERVAL_CHAR;
			}
		}
		break;

	case COMM_GET_TIMEOUTS:		/* get the comm timeout in ms	*/
		/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
		if((pBuffer == NULL) || (nDataLength != sizeof(COMM_TIMEOUTS)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
			*(COMM_TIMEOUTS *)pBuffer = pPort->toTimeouts;
		}
		break;

	case COMM_PURGE_TXCLEAR:	/* clear the transmit buffer	*/
		//if (pPort->nWorkMode != COMM_LOCAL_SERVER)
		{
			//tcflush(pPort->fdSerial,TCOFLUSH);
			Sleep(50);	// do NOTHING.
		}
		//else
		//{
		//	/* The server port does not support		*/
		//	nErrCode = ERR_COMM_CTRL_COMMAND;
		//}
		break;

	case COMM_PURGE_RXCLEAR:	/* clear the receive buffer		*/
		//if (pPort->nWorkMode != COMM_LOCAL_SERVER)
		{
			//char c;
			//changed by Jimmy 20210427 to improve the performance
			char cccccc[8192];
			read(pPort->fdSerial, cccccc, 8192);			
			//int nMaxReadCount = 0;
			//tcflush(pPort->fdSerial,TCIFLUSH);
			// while (read(pPort->fdSerial, &c, 1) > 0)
			// {
			// 	nMaxReadCount++;
			// 	if(nMaxReadCount > 4096)
			// 	{
			// 		break;
			// 	}
			// }
		}
		//else
		//{
		//	/* The server port does not support		*/
		//	nErrCode = ERR_COMM_CTRL_COMMAND;
		//}
		break;

	case COMM_SET_MAX_CLIENTS:	/* set the maximum allowed clients	*/
		if((pBuffer == NULL) || (nDataLength != sizeof(int)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
			if (pPort->nWorkMode == COMM_LOCAL_SERVER)	/* Must be server	*/
			{
				if ((*(int *)pBuffer > 0)				 /* client must > 0	*/
					&& (*(int *)pBuffer <= SERIAL_MAX_CLIENT_LIMITAION))
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
		if((pBuffer == NULL) || (nDataLength != sizeof(int)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
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
		if((pBuffer == NULL) || (nDataLength != sizeof(int)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
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
		if((pBuffer == NULL) || (nDataLength != sizeof(int)))
		{
			nErrCode = ERR_COMM_CTRL_PARAMS;
		}
		else
		{
			if (pPort->nWorkMode == COMM_LOCAL_SERVER) /* Must be server	*/
			{
				*(int *)pBuffer = SERIAL_MAX_CLIENT_LIMITAION;
			}
			else
			{
				nErrCode = ERR_COMM_CTRL_COMMAND;
			}
		}
		break;

	case COMM_SET_SERIAL_ATTRIB:
		{
			if(nDataLength < 8)
			{
				nErrCode = ERR_COMM_CTRL_PARAMS;
			}
			else
			{
				if (!Serial_ParseOpenParams((char*)pBuffer, &pPort->attr))
				{
#ifdef _DEBUG_SERIAL_PORT
					TRACE("[Serial_CommOpen] -- invalid parameters: \"%s\".\n",
						(char*)pBuffer);
#endif
					nErrCode = ERR_COMM_CTRL_COMMAND;
				}
				else 
				{
					if (!Serial_SetCommAttributes(pPort->fdSerial, &pPort->attr))
					{
						nErrCode = ERR_COMM_CTRL_COMMAND;
					}
				}
			}

			break;
		}

	case COMM_SET_SERIAL_485_To232:
	{
		struct serial_rs485 rs485conf;

		/* Enable RS485 mode: */
		rs485conf.flags |= SER_RS485_ENABLED;

		/* Set logical level for RTS pin equal to 1 when sending: */
		rs485conf.flags |= SER_RS485_RTS_ON_SEND;
		/* or, set logical level for RTS pin equal to 0 when sending: */
		rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);

		/* Set logical level for RTS pin equal to 1 after sending: */
		rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
		/* or, set logical level for RTS pin equal to 0 after sending: */
		rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);

		// /* Set rts delay before send, if needed: */
		// rs485conf.delay_rts_before_send = ...;

		// /* Set rts delay after send, if needed: */
		// rs485conf.delay_rts_after_send = ...;

		/* Set this flag if you want to receive data even whilst sending data */
		rs485conf.flags |= SER_RS485_RX_DURING_TX;
		if (ioctl(pPort->fdSerial, TIOCSRS485, &rs485conf) < 0)
		{
			/* Error handling. See errno. */
			TRACEE("Serial_CommControl ioctl failed <%x:%s>", pPort->fdSerial, pPort->szPortName);
		}
	}
		break;

	default:
		nErrCode = ERR_COMM_CTRL_COMMAND;
	}

	return nErrCode;
}

int Serial_CommClose(IN HANDLE hPort)
{
	SERIAL_PORT_DRV *pPort = (SERIAL_PORT_DRV *)hPort;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	(*pPort->pCurClients)--;
	if (*pPort->pCurClients < 0)
	{
		TRACE("\nClose port!!!\n");
		close(pPort->fdSerial);	// to close fd if it is not used yet.
		//close(pPort->fdRtsFor485);
		DELETE(pPort->pCurClients);
	}

	DELETE(pPort);

	return ERR_COMM_OK;
}

