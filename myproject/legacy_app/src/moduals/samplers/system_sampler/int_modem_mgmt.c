//#define _TRACE_PRINT_
#include "sampler_system.h"

// #define TRACE	 printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf
// #define TRACEX	 printf("[%s-->%s:%d] [%d]-- ", __FILE__, __FUNCTION__, __LINE__, time(NULL)), printf

typedef enum 
{
	//ATCMD_CLOSEDISP_ANDSAVE = 0, //ate0 to close loop display
	ATCMD_QUERY_SIMSTATUS = 0, //query SIM card status
	ATCMD_QUERY_CSQ, //query signal strength
	ATCMD_QUERY_IMSI,
	ATCMD_QUERY_ICID,
	ATCMD_STOP_ECMCALL, //stop previous ECM call
	ATCMD_PDP_SET, //set PDP
	ATCMD_PDP_ACT, //act the PDP
	ATCMD_PDP_DEACT, //de-act the old PDP
	ATCMD_START_ECMCALL,
	ATCMD_SOFT_RESET_MODEM, //soft-reset modem. hard-reset will be called by script
	ATCMD_SOFT_TURNOFF_MODEM, //soft-turn-off modem
	ATCMD_QUERY_NETINFO, //quety network info

	ATCMD_MAXTYPE
}ATCMD_TYPE;

typedef struct 
{
	ATCMD_TYPE	emCmdType;
	char 		szCmd_Keyword[32]; //keyword for sending
	char 		szResp_Keyword[32]; //keyword for identifying the response
	int			nTimeOutSec; //time out in second
}ATCMD_INFO;

typedef enum
{
	LTEMODEM_STATUS_NOT_INIT, //modem not initialized (e,g not installded, no hardware detected)
	LTEMODEM_STATUS_IDLE, //initialzed, and not ready for dialing
	LTEMODEM_STATUS_DIALING, //start dialing process
	LTEMODEM_STATUS_READY, //dial finished and ready for network
	LTEMODEM_STATUS_DIAL_FAIL, //error dialing	
	LTEMODEM_STATUS_WAIT_RESET, //sent reset and wait its re-initialization
	LTEMODEM_STATUS_DISABLING, //this means the LTE-Modem is disabling due to config change
}LTEMODEM_STATUS;

typedef void (*ON_LETMODEM_STATE)(void);
static void On_NotInit(void);
static void On_Idle(void);
static void On_Dailing(void);
static void On_Ready(void);
static void On_DialFailed(void);
static void On_WaitReset(void);
static void On_Disabling(void);
#define LTEMODEM_ERRORTYE_PORTOPEN		0
#define LTEMODEM_ERRORTYE_ATERROR		1
#define LTEMODEM_ERRORTYE_ATWRONGRESP	2
#define LTEMODEM_ERRORTYE_ATTIMEOUT		3
#define LTEMODEM_ERRORTYE_SIMNOTREADY	4
#define LTEMODEM_ERRORTYE_DIALERROR		5
#define LTEMODEM_ERRORTYE_PORT_NOTFOUND		6
static void On_LTEMODEM_Error(int emERRORType);

typedef struct
{
	SYSTEM_SAMPLER_DATA *pSys_Rough;

	ATCMD_INFO atCmdList[ATCMD_MAXTYPE];
	int		   fdTTYUSB; //for ttyUSB file port
	int 	   emPendingATCmd; //-1 means no pending
	double	   fATNoResponseTime; //for judgding the timeout
	char	   szReadBuffer[1024]; //for all historic buffer

	LTEMODEM_STATUS	emModemStatus;

	int		   emLTEModemType; //1-ME3630, 2-EC200U, 3-EC200A

	BOOL		bATE0_ProcessOK; //send it in first time
	int			nErrorCount; //error counter
	BOOL		bIsSIMCardReady;

	int 		nCurrentCSQ; //current CSQ
	time_t		tmLastTimeGetCSQ; //lastTime get CSQ

	BOOL		bGetIMSIOK;
	BOOL		bGetICCIDOK;

	double	    fNotConnectedTime; //for judgding if need re-dial
	int			nReDialFailCount; //re-dial fail count
	double	    fWaitResetTime;  //just if need hard reset
	BOOL		bIsFinishSoftReset;

	BOOL		bIsFinishTurnOffModem; //judge if finish modem power-off
	time_t		tmLastEnterTurnOff;

	BOOL		bNeedRedial;
	BOOL		bIsNetConneced; //is dialedOK

	time_t		tmLastHardResetModem; //last time hard reset modem
	time_t		tmLastEnterIdle;
	time_t		tmLastStartDial;
	BOOL 		bDialOK;
	int			nGetCSQErrorCounterOndle;
	int 		nNoSimCardFoundCounter;
	time_t		tmLastGetNetInfo; //lastTime get NetInfo
	int			nOpenTtyPort_ErrorCount; //Open TTYPort error counter
	BOOL 		bIsSentHardResetCmdAlready; //already sent hard-reset cmd?
	int			nDialFailCounter;

}INT_MODEM_ROUGHDATA;

static INT_MODEM_ROUGHDATA g_LTEModemData;
static int s_bIsInitialized = -99;

typedef struct
{
	int			nLogType;
	double		tmTimeOccur;
#define MAX_LTEMODEM_LOG_CONTSIZE	64
	char		szLog[MAX_LTEMODEM_LOG_CONTSIZE];
}LTEMODEM_LOG_ITEM;
typedef struct
{
#define MAX_LTEMODEM_LOG_ITEMS	1000
	LTEMODEM_LOG_ITEM 	logs[MAX_LTEMODEM_LOG_ITEMS];
	UINT				nCurrPos; //pos from 0~999
	UINT				nLogTotal;
	time_t				tmLastWrite;
	UINT				nLastWriteTotal;
}LTEMODEM_LOGS;

#define LOGMODEM_TYPE_OPENPORT			0
#define LOGMODEM_TYPE_CLOSEPORT			1
#define LOGMODEM_TYPE_FLUSH				2
#define LOGMODEM_TYPE_READ				3
#define LOGMODEM_TYPE_WEIRE				4
#define LOGMODEM_TYPE_ERROR				5
#define LOGMODEM_TYPE_TTYPORTCHANGE		6
#define LOGMODEM_TYPE_STOP_ORRESETMODEM			7
static char* Make_LTELogBuffer(LTEMODEM_LOG_ITEM* pLog, char* szBuff, int nBuffLen)
{
	memset(szBuff, 0, nBuffLen);
	char szLogType[16];
	switch (pLog->nLogType)
	{
	case LOGMODEM_TYPE_OPENPORT:
		strncpy(szLogType, "OPEN", 16);
		break;
	case LOGMODEM_TYPE_CLOSEPORT:
		strncpy(szLogType, "CLOSE", 16);
		break;
	case LOGMODEM_TYPE_FLUSH:
		strncpy(szLogType, "FLUSH", 16);
		break;
	case LOGMODEM_TYPE_READ:
		strncpy(szLogType, "READ", 16);
		break;
	case LOGMODEM_TYPE_WEIRE:
		strncpy(szLogType, "WRITE", 16);
		break;
	case LOGMODEM_TYPE_ERROR:
		strncpy(szLogType, "ERROR", 16);
		break;
	case LOGMODEM_TYPE_TTYPORTCHANGE:
		strncpy(szLogType, "TTY-CHANGE", 16);
		break;
	case LOGMODEM_TYPE_STOP_ORRESETMODEM:
		strncpy(szLogType, "STOP-OR-RESET", 16);
		break;
	default:
		strncpy(szLogType, "OTHER", 16);
		break;
	}

	char szTime[32];
	memset(szTime, 0, 32);
	snprintf(szBuff, nBuffLen, "%s\t\t%s\t\t%s\n", AcurTimeToString(pLog->tmTimeOccur, TIME_CHN_FMT, szTime, 32), 
		szLogType, pLog->szLog);

	return szBuff;
}

static void Log_Modem(int nLogType, char *pszFormat, ...)
{
	//save it in memory first
	static LTEMODEM_LOGS s_LTELogs;
	static BOOL s_bIsInitLogOK = FALSE;
	if(!s_bIsInitLogOK)
	{
		s_bIsInitLogOK = TRUE;
		ZERO_POBJS(&s_LTELogs, 1);		
		s_LTELogs.nCurrPos = 0;
		s_LTELogs.nLogTotal = 0;
	}

	//now save it first 
	LTEMODEM_LOG_ITEM log;
	log.tmTimeOccur = GetCurrentTime();
	log.nLogType = nLogType;
	memset(log.szLog, 0, MAX_LTEMODEM_LOG_CONTSIZE);
	va_list arglist;
	va_start(arglist, pszFormat);
	//vsnprintf(szBuf + strlen(szBuf), sizeof(szBuf), pszFormat, arglist);
	vsnprintf(log.szLog, MAX_LTEMODEM_LOG_CONTSIZE, pszFormat, arglist);
	va_end(arglist);
	//judge where to save
	if(s_LTELogs.nCurrPos >= MAX_LTEMODEM_LOG_ITEMS)
	{		
		s_LTELogs.nCurrPos = 0;
	}
	memcpy(&s_LTELogs.logs[s_LTELogs.nCurrPos], &log, sizeof(LTEMODEM_LOG_ITEM));
	s_LTELogs.nCurrPos++;	
	s_LTELogs.nLogTotal++;	
	if(s_LTELogs.nLogTotal > MAX_LTEMODEM_LOG_ITEMS)
	{
		s_LTELogs.nLogTotal = MAX_LTEMODEM_LOG_ITEMS;
	}

#define LTEODEL_LOG_WRITE_LIMIT		100
	if(s_LTELogs.nLastWriteTotal < LTEODEL_LOG_WRITE_LIMIT)
	{
		s_LTELogs.nLastWriteTotal++;
	}

	//now judge it need write log and then write it one by one
	if((ABS(_NOW_ - s_LTELogs.tmLastWrite) < 60) ||
		(s_LTELogs.nLastWriteTotal < LTEODEL_LOG_WRITE_LIMIT))
	{
		//if there is sth new for a long time, then still write
		if((ABS(_NOW_ - s_LTELogs.tmLastWrite) >= 180) && 
			(s_LTELogs.nLastWriteTotal >= 1))
		{
			//contine write
		}
		//when not connected, should write more frequently
		else if((!g_LTEModemData.bIsNetConneced) && (s_LTELogs.nLastWriteTotal >= 5) &&
			(ABS(_NOW_ - s_LTELogs.tmLastWrite) >= 10))
		{
			//contine write
		}
		else if(s_LTELogs.tmLastWrite != 0) //first time should write one item
		{
			return;
		}		
	}

	//now write all the content back to log file
	FILE *fp = fopen("/data/lte_modem.log","w+t");
	if (fp == NULL)
	{
		return;
	}

	int n;
	int nTotal = 0;
	//write old part
	char szBuff[512];
	for(n = s_LTELogs.nCurrPos; n < s_LTELogs.nLogTotal; n++)
	{
		fprintf(fp, "%d.\t%s", nTotal + 1, Make_LTELogBuffer(&s_LTELogs.logs[n], szBuff, 512));	
		nTotal++;
	}
	//wite new part
	for(n = 0; n < s_LTELogs.nCurrPos; n++)
	{
		fprintf(fp, "%d.\t%s", nTotal + 1, Make_LTELogBuffer(&s_LTELogs.logs[n], szBuff, 512));	
		nTotal++;
	}

	s_LTELogs.tmLastWrite = _NOW_;
	s_LTELogs.nLastWriteTotal = 0;

	fclose(fp);
}

static char* Remove_Enter_4ATCmd(char* szSrc)
{
	TRACEW("Remove_Enter_4ATCmd");
	char *p = szSrc;
	while (*p)
	{
		if((*p == '\r') || (*p == '\n'))
		{
			*p = '|';
		}
		p++;
	}
	return szSrc;
}

static char* GetTTYUSBPort_ByModemType(void)
{
	TRACEW("GetTTYUSBPort_ByModemType");
	static char s_strUSBTtyPort[32] = {0};
	char szTempPort[32] = {0};
	int n;
	if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U)
	{
		//Jimmy note： in msb strick error, the USB port may change, so find the first ttyUSB port		
		BOOL bFound = FALSE;
		for(n = 0; n < 64; n++)
		{
			snprintf(szTempPort, 32, "/dev/ttyUSB%d", n);
			if(access(szTempPort, F_OK) == 0)
			{
				bFound = TRUE;
				break;
			}
		}
		if(!bFound)
		{
			snprintf(szTempPort, 32, "%s", "/dev/ttyUSB0");
		}
	}
	else if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_ME3630)
	{
		//Jimmy note： in msb strick error, the USB port may change, so find the last ttyUSB port	
		BOOL bFound = FALSE;
		for(n = 2; n < 64; n++)
		{
			snprintf(szTempPort, 32, "/dev/ttyUSB%d", n);
			if(access(szTempPort, F_OK) == 0)
			{
				bFound = TRUE;
				break;
			}
		}		
		if(!bFound)
		{
			snprintf(szTempPort, 32, "%s", "/dev/ttyUSB2");
		}
	}
	else if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200A)
	{
		//Jimmy note： in msb strick error, the USB port may change, so find the first ttyUSB port		
		BOOL bFound = FALSE;
		for(n = 1; n < 64; n++)
		{
			snprintf(szTempPort, 32, "/dev/ttyUSB%d", n);
			if(access(szTempPort, F_OK) == 0)
			{
				bFound = TRUE;
				break;
			}
		}
		if(!bFound)
		{
			snprintf(szTempPort, 32, "%s", "/dev/ttyUSB1");
		}
	}
	else if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200M)
	{
		//Jimmy note： in msb strick error, the USB port may change, so find the first ttyUSB port		
		BOOL bFound = FALSE;
		for(n = 1; n < 64; n++)
		{
			snprintf(szTempPort, 32, "/dev/ttyUSB%d", n);
			if(access(szTempPort, F_OK) == 0)
			{
				bFound = TRUE;
				break;
			}
		}
		if(!bFound)
		{
			snprintf(szTempPort, 32, "%s", "/dev/ttyUSB1");
		}
	}

	if(strncmp(s_strUSBTtyPort, szTempPort, 32) != 0)
	{
		Log_Modem(LOGMODEM_TYPE_TTYPORTCHANGE, "[%s] -> [%s]!", s_strUSBTtyPort, szTempPort);
		snprintf(s_strUSBTtyPort, 32, "%s", szTempPort);	
	}

	return s_strUSBTtyPort;
}

static void Close_TTYUSBPort(void)
{
	TRACEW("Close_TTYUSBPort");
	if(g_LTEModemData.fdTTYUSB == -1)
	{
		return;
	}
	
	//should not call 'close(fd)' when ttyUSB not exists as this may crash IMMU2
	//if(access((g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U) ? "/dev/ttyUSB0" : "/dev/ttyUSB2", F_OK) == 0)
	//{			
		if(g_LTEModemData.fdTTYUSB > 0)
		{
			Log_Modem(LOGMODEM_TYPE_CLOSEPORT, "OK");	
			close(g_LTEModemData.fdTTYUSB);
		}		
	//}
	// else
	// {
	// 	Log_Modem(LOGMODEM_TYPE_CLOSEPORT, "ABNORMALLY!");
	// }

	g_LTEModemData.fdTTYUSB = -1;
}

static BOOL Open_TTYUSBPort(void)
{
	TRACEW("Open_TTYUSBPort");
	if(g_LTEModemData.fdTTYUSB != -1)
	{
		Close_TTYUSBPort();
	}

	char* szTTYUSBPort = GetTTYUSBPort_ByModemType();
	if(strlen(szTTYUSBPort) < 2)
	{
		return FALSE;
	}

	if(access(szTTYUSBPort, F_OK) != 0)
	{
		//should not call 'close(fd)' as this may crash IMMU2
		On_LTEMODEM_Error(LTEMODEM_ERRORTYE_PORT_NOTFOUND);
		return FALSE;
	}

	g_LTEModemData.fdTTYUSB = open(szTTYUSBPort, 
			O_RDWR | O_NONBLOCK | O_NOCTTY/* | O_EXCL */| O_NDELAY); //added by Jimmy 20200722 fpr 'O_TRUNC' to cut history msgs
	if(g_LTEModemData.fdTTYUSB < 0)
	{
		On_LTEMODEM_Error(LTEMODEM_ERRORTYE_PORTOPEN);
		//printf("\nOpen %s FAILED!\n", (g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U) ? "/dev/ttyUSB0" : "/dev/ttyUSB2");
		return FALSE;
	}
	//printf("\nOpen %s OK!\n", (g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U) ? "/dev/ttyUSB0" : "/dev/ttyUSB2");
	Log_Modem(LOGMODEM_TYPE_OPENPORT, "%s", szTTYUSBPort);
	g_LTEModemData.nOpenTtyPort_ErrorCount = 0;
	return TRUE;
}

static int Get_LTEModemInstType(SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bTestMode)
{
	TRACEW("Get_LTEModemInstType");
	int nInstallType = pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem;
	if(bTestMode)
	{
		return LTEMODE_INSTTYPE_ME3630;
	}
	else if(nInstallType == 0)
	{
		return LTEMODE_INSTTYPE_NOTINST;
	}
	else if(nInstallType == 1)
	{
		return LTEMODE_INSTTYPE_ME3630;
	}
	else if(nInstallType == 2)
	{
		return LTEMODE_EXTTTYPE_ZR2000;
	}
	else if(nInstallType == 3)
	{
		return LTEMODE_INSTTYPE_EC200U;
	}
	else if(nInstallType == 4)
	{
		return LTEMODE_INSTTYPE_EC200A;
	}
	else if(nInstallType == 5)
	{
		return LTEMODE_INSTTYPE_EC200M;
	}

	//return LTEMODE_INSTTYPE_NOTINST;
}

#define  INIT_ATCMD(p, _cmdType, _keyWord, _respKeyword, _mSecTimeout) \
					((p)->emCmdType = (_cmdType), \
					strncpy((p)->szCmd_Keyword, (_keyWord), 32), \
					strncpy((p)->szResp_Keyword, (_respKeyword), 32), \
					(p)->nTimeOutSec = (_mSecTimeout),  \
					(p)++)

static void Init_LTEModemStatus(void)
{	
	TRACEW("Init_LTEModemStatus");
	g_LTEModemData.fdTTYUSB = -1;
	g_LTEModemData.bATE0_ProcessOK = FALSE;
	g_LTEModemData.nErrorCount = 0;
	g_LTEModemData.bIsSIMCardReady = FALSE;
	g_LTEModemData.nCurrentCSQ = 0;
	g_LTEModemData.bGetIMSIOK = FALSE;
	g_LTEModemData.bGetICCIDOK = FALSE;
	g_LTEModemData.nReDialFailCount = 0;
	g_LTEModemData.bNeedRedial = FALSE;
	g_LTEModemData.bIsNetConneced = FALSE;
	g_LTEModemData.bIsFinishTurnOffModem = (s_bIsInitialized == -99) ? 99 : FALSE; //99means first time
	g_LTEModemData.tmLastEnterTurnOff = 0;
	g_LTEModemData.tmLastEnterIdle = 0;
	g_LTEModemData.nGetCSQErrorCounterOndle = 0;
	g_LTEModemData.bDialOK = FALSE;
	g_LTEModemData.nNoSimCardFoundCounter = 0;
	g_LTEModemData.bIsSentHardResetCmdAlready = FALSE;
	ZERO_POBJS(&g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0, 1);
	//ZERO_POBJS(&g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo, 1);
	LTEMODEM_INFO* pLTE = &g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo;
	memset(pLTE->szIMSI, 0, 20);
	memset(pLTE->szICCID, 0, 32);
	memset(pLTE->szNetwork_Type, 0, 32);
	if(pLTE->bIsManulSet_APN == VPN_AUTHTYPE_NONE)
	{
		memset(pLTE->szAPN, 0, 20);
		memset(pLTE->szAPN_User, 0, 32);
		memset(pLTE->szAPN_Passwd, 0, 32);
	}
	pLTE->nSignal_dBm = 0;pLTE->nSignal_Level = 0;
	// added by jinhuang 2023-11-28 sn、version
	memset(pLTE->szId, 0, 64); 
	memset(pLTE->szSWVer, 0, 16);
}
static void ReInitialize_LTEModemRoughData(SYSTEM_SAMPLER_DATA *pSys_Rough, int nModemInstatType)
{
	TRACEW("ReInitialize_LTEModemRoughData");
	if(s_bIsInitialized == -99) //means first initialize
	{
		ZERO_POBJS(&g_LTEModemData, 1);
		g_LTEModemData.emPendingATCmd = -1;
		g_LTEModemData.nDialFailCounter = 0;		
	}	
	g_LTEModemData.pSys_Rough = pSys_Rough;
	//initial the cmdInfo
	ATCMD_INFO* p = &g_LTEModemData.atCmdList[0];
	if(nModemInstatType == LTEMODE_INSTTYPE_ME3630)
	{
		//INIT_ATCMD(p,		ATCMD_CLOSEDISP_ANDSAVE,		"E0;&W",			"OK",			10);
		INIT_ATCMD(p,		ATCMD_QUERY_SIMSTATUS,			"+CPIN?",			"+CPIN:",		30);
		INIT_ATCMD(p,		ATCMD_QUERY_CSQ,				"E0;&W;+CSQ",		"+CSQ:",		10);
		INIT_ATCMD(p,		ATCMD_QUERY_IMSI,				"+CIMI",			"OK",			10);
		INIT_ATCMD(p,		ATCMD_QUERY_ICID,				"+ZGETICCID",		"+ZGETICCID:",	10);
		INIT_ATCMD(p,		ATCMD_STOP_ECMCALL,				"+ZECMCALL=0",		"OK",			15);
		INIT_ATCMD(p,		ATCMD_PDP_SET,					"+CGDCONT=1,\"IP\"","OK",			60);
		INIT_ATCMD(p,		ATCMD_PDP_ACT,					"+CGACT=1,1",		"OK",			60);
		INIT_ATCMD(p,		ATCMD_PDP_DEACT,				"+CGACT=0,1",		"OK",			60);
		INIT_ATCMD(p,		ATCMD_START_ECMCALL,			"+ZECMCALL=1",		"+ZECMCALL:",	60); //need add extra paramter for APN/user/paswd if have
		INIT_ATCMD(p,		ATCMD_SOFT_RESET_MODEM,			"+ZRST",			"OK",			5);
		INIT_ATCMD(p,		ATCMD_SOFT_TURNOFF_MODEM,		"+ZTURNOFF",		"OK",			5);
		INIT_ATCMD(p,		ATCMD_QUERY_NETINFO,			"E0;+ZPAS?",		"+ZPAS:",		10);
	}
	else if((nModemInstatType == LTEMODE_INSTTYPE_EC200U) || (nModemInstatType == LTEMODE_INSTTYPE_EC200A) || (nModemInstatType == LTEMODE_INSTTYPE_EC200M))
	{
		//INIT_ATCMD(p,		ATCMD_CLOSEDISP_ANDSAVE,		"E0;&W",			"OK",			10);
		INIT_ATCMD(p,		ATCMD_QUERY_SIMSTATUS,			"+CPIN?",			"+CPIN:",		30);
		INIT_ATCMD(p,		ATCMD_QUERY_CSQ,				"E0;&W;+CSQ",		"+CSQ:",		10);
		INIT_ATCMD(p,		ATCMD_QUERY_IMSI,				"+CIMI",			"OK",			10);
		INIT_ATCMD(p,		ATCMD_QUERY_ICID,				"+QCCID",			"+QCCID:",		10);
		INIT_ATCMD(p,		ATCMD_STOP_ECMCALL,				"+QNETDEVCTL=0",	"OK",			15);
		INIT_ATCMD(p,		ATCMD_PDP_SET,					"+QICSGP=1,1",		"OK",			60);//need add extra paramter for APN/user/paswd if have
		INIT_ATCMD(p,		ATCMD_PDP_ACT,					"+QIACT=1",			"OK",			60);
		INIT_ATCMD(p,		ATCMD_PDP_DEACT,				"+QIDEACT=1",		"OK",			60);
		INIT_ATCMD(p,		ATCMD_START_ECMCALL,			"+QNETDEVCTL=1,1,1","+QNETDEVSTATUS:",	60); 
		INIT_ATCMD(p,		ATCMD_SOFT_RESET_MODEM,			"+CFUN=1,1",		"OK",			5);
		INIT_ATCMD(p,		ATCMD_SOFT_TURNOFF_MODEM,		"+QPOWD=0",			"OK",			5);
		INIT_ATCMD(p,		ATCMD_QUERY_NETINFO,			"+QNWINFO",			"+QNWINFO:",	10);
	}
	//initial the status
	g_LTEModemData.emModemStatus = LTEMODEM_STATUS_DISABLING; //initial status should be this
	g_LTEModemData.emLTEModemType = nModemInstatType;
	Init_LTEModemStatus();
}

static char* MakeCmdSendBuffer(ATCMD_TYPE tp, IN char* szOut, IN int nSize)
{	
	TRACEW("MakeCmdSendBuffer");
	char szContent[128];
	memset(szContent, 0, 128);
	LTEMODEM_INFO* pLTE = &g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo;
	if((pLTE->bIsManulSet_APN > VPN_AUTHTYPE_NONE) /*&& (strlen(pLTE->szAPN) > 0)*/)
	{
		if((tp == ATCMD_PDP_SET) && ((g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U) ||
			(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200A) || (g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200M)))
		{
			//snprintf(szContent, 64, ",\"%s\"", pLTE->szAPN);	
			//if(strlen(pLTE->szAPN_User) > 0)
			{
				//snprintf(szContent, 64, ",\"%s\",\"%s\"", pLTE->szAPN, pLTE->szAPN_User);	
				//if(strlen(pLTE->szAPN_Passwd) > 0)
				{
					snprintf(szContent, 128, ",\"%s\",\"%s\",\"%s\",%d", 
							pLTE->szAPN, 
							pLTE->szAPN_User, 
							pLTE->szAPN_Passwd,
							(pLTE->bIsManulSet_APN == VPN_AUTHTYPE_CHAP) ? 2 : 
								((pLTE->bIsManulSet_APN == VPN_AUTHTYPE_PAP) ? 1 : 0));	
				}
			}
		}
		else if((tp == ATCMD_START_ECMCALL) && (g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_ME3630))
		{
			//snprintf(szContent, 64, ",\"%s\"", pLTE->szAPN);
			//if(strlen(pLTE->szAPN_User) > 0)
			{
				//snprintf(szContent, 64, ",\"%s\",1,\"%s\"", pLTE->szAPN, pLTE->szAPN_User);	
				//if(strlen(pLTE->szAPN_Passwd) > 0)
				{
					snprintf(szContent, 128, ",\"%s\",1,\"%s\",\"%s\",%d", 
							pLTE->szAPN, 
							pLTE->szAPN_User, 
							pLTE->szAPN_Passwd,
							(pLTE->bIsManulSet_APN == VPN_AUTHTYPE_CHAP) ? 2 : 1);	
				}
			}
		}	
	}
	
	snprintf(szOut, nSize, "\rAT%s%s\r\n", g_LTEModemData.atCmdList[tp].szCmd_Keyword, szContent);
	return szOut;
}

static void On_LTEMODEM_Error(int emERRORType)
{	
	TRACEW("On_LTEMODEM_Error");
	g_LTEModemData.emPendingATCmd = -1;
	BOOL bIsNeedResetModem = FALSE;
	if(LTEMODEM_ERRORTYE_PORTOPEN == emERRORType)
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[Open port Error】, COUNT=>%d, errNo->[%s]!", 
			g_LTEModemData.nOpenTtyPort_ErrorCount, strerror(errno));	
		g_LTEModemData.nOpenTtyPort_ErrorCount++;
		if(g_LTEModemData.nOpenTtyPort_ErrorCount >= 50) //wait 300*50 = 15sec
		{
			bIsNeedResetModem = 2;
		}
	}
	else if(LTEMODEM_ERRORTYE_SIMNOTREADY == emERRORType)
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[SIM not ready】, COUNT=>%d!", g_LTEModemData.nErrorCount);
		g_LTEModemData.nErrorCount++;
		if(g_LTEModemData.nErrorCount >= 3)
		{
			bIsNeedResetModem = TRUE;
		}
	}
	else if(LTEMODEM_ERRORTYE_ATERROR == emERRORType)
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[AT Error】, COUNT=>%d!", g_LTEModemData.nErrorCount);
		g_LTEModemData.nErrorCount++;
		if(g_LTEModemData.nErrorCount >= 3)
		{
			bIsNeedResetModem = TRUE;
		}
	}
	else if(LTEMODEM_ERRORTYE_ATTIMEOUT == emERRORType)
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[AT Timeout], COUNT=>%d!", g_LTEModemData.nErrorCount);
		if(g_LTEModemData.emPendingATCmd == ATCMD_QUERY_IMSI)
		{
			g_LTEModemData.bGetIMSIOK = TRUE; //just in case get IMSI error
		}
		else if(g_LTEModemData.emPendingATCmd == ATCMD_QUERY_ICID)
		{
			g_LTEModemData.bGetICCIDOK = TRUE; //just in case get IMSI error
		}
		else
		{
			g_LTEModemData.nErrorCount++;
			int TIMES_ERROR = g_LTEModemData.bIsNetConneced ? 5 : 2;
			if(g_LTEModemData.nErrorCount >= 2)
			{
				bIsNeedResetModem = 2;
			}
		}

	}
	else if(LTEMODEM_ERRORTYE_ATWRONGRESP == emERRORType)
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[AT WrongResp], COUNT=>%d!", g_LTEModemData.nErrorCount);
		g_LTEModemData.nErrorCount++;
		if(g_LTEModemData.nErrorCount >= 3)
		{
			bIsNeedResetModem = TRUE;
		}
	}
	else if(LTEMODEM_ERRORTYE_DIALERROR == emERRORType)
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[Dial Error], COUNT=>%d!", g_LTEModemData.nErrorCount);
		g_LTEModemData.nErrorCount++;
		if(g_LTEModemData.nErrorCount >= 3)
		{
			bIsNeedResetModem = TRUE;
		}
	}
	else if(LTEMODEM_ERRORTYE_PORT_NOTFOUND == emERRORType)
	{
		//usually means the modem itself has reset, so we wait for the reset done			
		g_LTEModemData.nOpenTtyPort_ErrorCount++;
		if(g_LTEModemData.nOpenTtyPort_ErrorCount >= 100) //wait 0.3*100 = 30sec
		{
			Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[Port NoExist Error】, COUNT=>%d!", g_LTEModemData.nOpenTtyPort_ErrorCount);
			bIsNeedResetModem = 2;
		}
	}
	else
	{
		Log_Modem(LOGMODEM_TYPE_ERROR, "REASON->[Unkown Error], COUNT=>%d!", g_LTEModemData.nErrorCount);
		bIsNeedResetModem = TRUE;
	}	
	
	if(bIsNeedResetModem)
	{
		//now goto the disabling process
		if((g_LTEModemData.emModemStatus != LTEMODEM_STATUS_DISABLING) &&
			(g_LTEModemData.emModemStatus != LTEMODEM_STATUS_WAIT_RESET))
		{
			g_LTEModemData.emModemStatus = (bIsNeedResetModem == 2) ? LTEMODEM_STATUS_DISABLING : LTEMODEM_STATUS_WAIT_RESET;	
			g_LTEModemData.fWaitResetTime = 0.0;
			g_LTEModemData.bIsFinishSoftReset = FALSE;
			g_LTEModemData.bIsFinishTurnOffModem = FALSE;			
			g_LTEModemData.nErrorCount = 0;		
		}
		else
		{
			if(g_LTEModemData.nErrorCount >= 30)
			{
				g_LTEModemData.emModemStatus = LTEMODEM_STATUS_DISABLING;	
				g_LTEModemData.fWaitResetTime = 0.0;
				g_LTEModemData.bIsFinishSoftReset = FALSE;
				g_LTEModemData.bIsFinishTurnOffModem = 99;			
				g_LTEModemData.nErrorCount = 0;				
			}
		}
	}
}

static BOOL Parse_IMSI_or_ICCID(char* szSrc, char* szOut, int nOutLen)
{
	TRACEW("Parse_IMSI_or_ICCID");
	char szParsedLen[32] = {0};
	memset(szParsedLen, 0, 32);
	char* p = szSrc;
	int nLen = 0;
	BOOL bFoundStart = FALSE;
	while (*p)
	{
		BOOL bNeedContinue = FALSE;
		if((*p == '\r') || (*p == '\n') || (*p == '|') || (*p == ' '))
		{
			if(!bFoundStart)
			{
				if((*(p + 1) >= '0') && (*(p + 1) <= '9'))
				{
					bFoundStart = TRUE;
				}
			}
			else
			{
				break;
			}
			bNeedContinue = TRUE;
		}
		else if(!bFoundStart)
		{
			if((*p >= '0') && (*p <= '9'))
			{
				bFoundStart = TRUE;
			}
		}
		
		if(bFoundStart && (!bNeedContinue))
		{
			if(nLen < 32)
			{
				szParsedLen[nLen] = *p;
				nLen++;				
			}
		}

		p++;
		if(!p)
		{
			break;
		}
	}

	if(nLen > 0)
	{
		memset(szOut, 0, nOutLen);
		strncpy(szOut, szParsedLen, nLen);
		return TRUE;
	}

	return FALSE;	
}

#define ATCMD_PROCESS_STAT_PENDING		0
#define ATCMD_PROCESS_STAT_TIMEOUT		1
#define ATCMD_PROCESS_STAT_ERROR		2
#define ATCMD_PROCESS_STAT_FINISHED		3
static BOOL Parse_ATRespContent(char* szSrc, char* szPos, ATCMD_TYPE tp)
{
	TRACEW("Parse_ATRespContent");
	//printf("----RESPCMD->[%d], TheCONTRNT->{%s},{%s}-----\n", tp, szSrc, szPos);
	g_LTEModemData.emPendingATCmd = -1;
	// if(ATCMD_CLOSEDISP_ANDSAVE == tp)
	// {
	// 	g_LTEModemData.bATE0_ProcessOK = TRUE;
	// 	return ATCMD_PROCESS_STAT_FINISHED;
	// }
	//else if(ATCMD_QUERY_SIMSTATUS == tp)
	if(ATCMD_QUERY_SIMSTATUS == tp)
	{
		ConvertString_2UpperCase(szPos);
		if(strstr(szPos, "READY") != NULL)
		{
			g_LTEModemData.bIsSIMCardReady = TRUE;
			return ATCMD_PROCESS_STAT_FINISHED;
		}
		else
		{
			g_LTEModemData.bIsSIMCardReady = FALSE;
			g_LTEModemData.nNoSimCardFoundCounter++;
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_SIMNOTREADY);
			return ATCMD_PROCESS_STAT_FINISHED;
		}
	}
	else if(ATCMD_QUERY_CSQ == tp)
	{
		//parse CSQ
		if((strlen(szPos) > 1) && (strstr(szPos, ",") != NULL))
		{			
			g_LTEModemData.bATE0_ProcessOK = TRUE;
			int nCSQ = atoi(szPos);			
			int nSigLevel = 5;//RSSI, 31~15 => 5, 14~12 => 4, 12~10 => 3, 9~7 => 2, 7~5 => 1, else => 0
			if((nCSQ >= 15) && (nCSQ <= 31))
			{
				nSigLevel = 5;
			}
			else if((nCSQ > 12) && (nCSQ <= 14))
			{
				nSigLevel = 4;
			}
			else if((nCSQ >= 10) && (nCSQ <= 12))
			{
				nSigLevel = 3;
			}
			else if((nCSQ >= 7) && (nCSQ <= 9))
			{
				nSigLevel = 2;
			}
			else if((nCSQ >= 5) && (nCSQ < 7))
			{
				nSigLevel = 1;
			}
			else
			{
				nSigLevel = 0;
				g_LTEModemData.nGetCSQErrorCounterOndle++;
			}

			g_LTEModemData.nCurrentCSQ = nCSQ;
			//printf("----TheCSQ->%d, nSigLevel->%d----\n", nCSQ, nSigLevel);

			if((g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm != nCSQ) ||
				(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level != nSigLevel))
			{
				//now update the parameter
				g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = nCSQ;
				g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = nSigLevel;
				Push_BlobValue(SETTYPE_RAW_ST, g_LTEModemData.pSys_Rough->hSampler,
					1, 3, (void*)&g_LTEModemData.pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
			}
			return ATCMD_PROCESS_STAT_FINISHED;
		}
		else
		{
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_ATWRONGRESP);
			return ATCMD_PROCESS_STAT_ERROR;
		}		
	}
	else if(ATCMD_QUERY_IMSI == tp)
	{
		//imsi get from SRC-string
		char szNewIMSI[20]; 
		if(Parse_IMSI_or_ICCID(szSrc, szNewIMSI, 20))
		{
			//printf("\n---IMSI->{%s}---\n", szNewIMSI);
			g_LTEModemData.bGetIMSIOK = TRUE;			
			if(strncmp(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szIMSI, szNewIMSI, 20) != 0)
			{
				strncpy(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szIMSI, szNewIMSI, 20); 	
				Push_BlobValue(SETTYPE_RAW_ST, g_LTEModemData.pSys_Rough->hSampler,
					1, 3, (void*)&g_LTEModemData.pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
			}
			if((strncmp(g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.szIMSI, szNewIMSI, 20) != 0) ||
					(!g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi))
			{
				g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi = 1;
				snprintf(g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.szIMSI, 20, "%s",  szNewIMSI);
				Push_BlobValue(SETTYPE_INTERN_SETT, g_LTEModemData.pSys_Rough->hSampler, 1, 17, 
					(void*)&g_LTEModemData.pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
			}
			return ATCMD_PROCESS_STAT_FINISHED;
		}		
	}
	else if(ATCMD_QUERY_ICID == tp)
	{
		//iccid get from POS-string 
		char szNewICCID[32]; 
		if(Parse_IMSI_or_ICCID(szPos, szNewICCID, 32))
		{				
			//printf("\n---ICCID->{%s}---\n", szNewICCID);	
			g_LTEModemData.bGetICCIDOK = TRUE;
			if(strncmp(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szICCID, szNewICCID, 32) != 0)
			{
				strncpy(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szICCID, szNewICCID, 32); 	
				Push_BlobValue(SETTYPE_RAW_ST, g_LTEModemData.pSys_Rough->hSampler,
					1, 3, (void*)&g_LTEModemData.pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
			}
			if((strncmp(g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.szICCID, szNewICCID, 32) != 0) ||
					(!g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid))
			{
				g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid = 1;
				snprintf(g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.szICCID, 32, "%s",  szNewICCID);
				Push_BlobValue(SETTYPE_INTERN_SETT, g_LTEModemData.pSys_Rough->hSampler, 1, 17, 
					(void*)&g_LTEModemData.pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
			}
			return ATCMD_PROCESS_STAT_FINISHED;
		}	
	}
	else if(ATCMD_START_ECMCALL == tp)
	{
		if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_ME3630)
		{
			ConvertString_2UpperCase(szPos);
			if((strstr(szPos, "CONNECT") != NULL))
			{
				g_LTEModemData.bDialOK = TRUE;
				return ATCMD_PROCESS_STAT_FINISHED;
			}
			g_LTEModemData.bDialOK = FALSE;
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_DIALERROR);
			return ATCMD_PROCESS_STAT_ERROR;
		}
		else if((g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U) ||
			(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200A) || (g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200M))
		{
			if((strstr(szPos, "1") != NULL))
			{
				g_LTEModemData.bDialOK = TRUE;
				return ATCMD_PROCESS_STAT_FINISHED;
			}
			g_LTEModemData.bDialOK = FALSE;
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_DIALERROR);
			return ATCMD_PROCESS_STAT_ERROR;
		}
	}
	else if(ATCMD_QUERY_NETINFO == tp)
	{
		ConvertString_2UpperCase(szPos);
		//if(strstr(szPos, "NO SERVICE") != NULL)
		if((strstr(szPos, "SERVICE") != NULL) && (strstr(szPos, "SERVICE") != NULL)) 
		{
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_DIALERROR);
			return ATCMD_PROCESS_STAT_ERROR;
		}
		else
		{
			char szNetwork_Type[32] = {0};
			strncpy(szNetwork_Type, szPos, 31);
			if(strncmp(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, szNetwork_Type, 32) != 0)
			{
				strncpy(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, szNetwork_Type, 31); 	
				Push_BlobValue(SETTYPE_RAW_ST, g_LTEModemData.pSys_Rough->hSampler,
					1, 3, (void*)&g_LTEModemData.pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
			} 
		}
	}
	else
	{
		return ATCMD_PROCESS_STAT_FINISHED;
	}

	//should wait until timeout if not received proper answer
	return ATCMD_PROCESS_STAT_PENDING;
}

static int Wait_ATCmd(BOOL bRightAfterSend)
{
	TRACEW("Wait_ATCmd");
	//if no cmd, then no need wait
	if((g_LTEModemData.emPendingATCmd < ATCMD_QUERY_SIMSTATUS) || 
		(g_LTEModemData.emPendingATCmd >= ATCMD_MAXTYPE))
	{
		TRACEW("1");
		Close_TTYUSBPort();
		g_LTEModemData.emPendingATCmd = -1;
		return ATCMD_PROCESS_STAT_ERROR;
	}
	
	if(g_LTEModemData.fdTTYUSB == -1)
	{
		TRACEW("2");
		if(!Open_TTYUSBPort())
		{
			TRACEW("3");
			g_LTEModemData.emPendingATCmd = -1;
			return ATCMD_PROCESS_STAT_ERROR;
		}
	}
	TRACEW("4");
	//wait
	int nRet = ATCMD_PROCESS_STAT_PENDING; 
	int tp = g_LTEModemData.emPendingATCmd;
	static double s_fLastTimeTick = 0.0;
	double ftmNow = GetCurrentTime();
	double fDiff = ftmNow - s_fLastTimeTick;
	char szTemp[64] = {0};
	write(g_LTEModemData.fdTTYUSB, "\r\n", 2); //write 2bytes to wakeup reading???
	int nBytes = read(g_LTEModemData.fdTTYUSB, szTemp, 64); 
	//Close_TTYUSBPort();
	// printf("------ReadBytes->%d, Diff->%.1f, ATNoRespTotal->%.1f, nTimeOut ->%d-------\n", nBytes,
	// 		fDiff, g_LTEModemData.fATNoResponseTime, g_LTEModemData.atCmdList[tp].nTimeOutSec);
	char* szRead = g_LTEModemData.szReadBuffer;
	if(nBytes > 0)
	{
		TRACEW("5");
		int nOldLen = strlen(szRead);
		if(nOldLen + nBytes < 1023)
		{
			strncpy(szRead + nOldLen, szTemp, nBytes);
		}
		else
		{
			szRead = szTemp;
		}
		
		//printf("READ[%d] -> {%s}\n", nBytes, szRead);		
		Log_Modem(LOGMODEM_TYPE_READ, "%s", Remove_Enter_4ATCmd(szRead));
		char* szRepos = strstr(szRead, g_LTEModemData.atCmdList[tp].szResp_Keyword);		
		if(szRepos != NULL)
		{
			TRACEW("6");
			szRepos += strlen(g_LTEModemData.atCmdList[tp].szResp_Keyword);
			nRet = Parse_ATRespContent(szRead, szRepos, tp);
			if(nRet == ATCMD_PROCESS_STAT_FINISHED)
			{
				g_LTEModemData.nErrorCount = 0; //to clear the error counter
			}
		}
		else if(strstr(szRead, "ERROR") != NULL)
		{
			TRACEW("7");
			memset(g_LTEModemData.szReadBuffer, 0, 1024);
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_ATERROR);
			nRet = ATCMD_PROCESS_STAT_ERROR;
		}
		else if((strstr(szRead, "+ZREADY") != NULL) && 
			(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_ME3630)) //added by Jimmy for first time reset of modem
		{
			TRACEW("8");
			nRet = ATCMD_PROCESS_STAT_FINISHED;
		}
		// else if((strstr(szRead, "READY") != NULL) && 
		// 	(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U)) //added by Jimmy for first time reset of modem
		// {
		// 	nRet = ATCMD_PROCESS_STAT_FINISHED;
		// }
		//just wait timeout
		// else //if(strstr(szRead, "AT+") != NULL)
		// {
		// 	//wrong response
		// 	On_LTEMODEM_Error(LTEMODEM_ERRORTYE_ATWRONGRESP);
		// 	nRet = ATCMD_PROCESS_STAT_ERROR;
		// }		
		//if pending, needs to check timeout below
		if(ATCMD_PROCESS_STAT_PENDING != nRet)
		{
			TRACEW("9");
			//printf("\n@@@@@@ RET->%d @@@@@@\n", nRet);
			Close_TTYUSBPort();
			return nRet;
		}		
	}
	else
	{
		TRACEW("10");
		int nErr = errno;
		//printf("------errno->%d, errDescrip->[%s]------\n", nErr, strerror(nErr));
		if(errno != EAGAIN) //Jimmy: just in case the ttyPort was destroyed and we did not know here
		{
			TRACEW("11");
			Close_TTYUSBPort();
		}
	}
	s_fLastTimeTick = ftmNow;
	if((fDiff > 0.0) && (fDiff < 5.0))
	{
		g_LTEModemData.fATNoResponseTime += fDiff;
		if(g_LTEModemData.fATNoResponseTime - g_LTEModemData.atCmdList[tp].nTimeOutSec > 0.0)
		{
			TRACEW("12");
			Close_TTYUSBPort();
			On_LTEMODEM_Error(LTEMODEM_ERRORTYE_ATTIMEOUT);			
			return ATCMD_PROCESS_STAT_TIMEOUT;
		}
	}

	return ATCMD_PROCESS_STAT_PENDING;
}

static int Send_ATCmd(ATCMD_TYPE tp)
{
	TRACEW("Send_ATCmd");
	if(!Open_TTYUSBPort())
	{
		g_LTEModemData.emPendingATCmd = -1;
		return ATCMD_PROCESS_STAT_ERROR;
	}

	//first clear the previous read buffer
	char szRead[1025] = {0};
	int nBytes = read(g_LTEModemData.fdTTYUSB, szRead, 1024); 
	if(nBytes > 0)
	{
		//printf("Now flush bytes->%d\n", nBytes);
		Log_Modem(LOGMODEM_TYPE_FLUSH, "%s", Remove_Enter_4ATCmd(szRead));
	}
	memset(g_LTEModemData.szReadBuffer, 0, 1024);
	char szSendBuff[256];
	MakeCmdSendBuffer(tp, szSendBuff, 256);
	//printf("Now write->{%s}\n", szSendBuff);
	nBytes = write(g_LTEModemData.fdTTYUSB, szSendBuff, strlen(szSendBuff));
	//Close_TTYUSBPort();
	Log_Modem(LOGMODEM_TYPE_WEIRE, "%s", Remove_Enter_4ATCmd(szSendBuff));
	g_LTEModemData.emPendingATCmd = (int)tp;
	g_LTEModemData.fATNoResponseTime = 0.0;
	if(tp == ATCMD_QUERY_CSQ)
	{
		g_LTEModemData.tmLastTimeGetCSQ = _NOW_;
	}
	else if((tp == ATCMD_START_ECMCALL) || (tp == ATCMD_STOP_ECMCALL))
	{
		g_LTEModemData.bDialOK = FALSE;
	}
	else if(tp == ATCMD_QUERY_NETINFO)
	{
		g_LTEModemData.tmLastGetNetInfo = _NOW_;
	}
	else if(ATCMD_SOFT_TURNOFF_MODEM == tp)
	{
		//don't wait AT response for this
		g_LTEModemData.emPendingATCmd = -1;
		Close_TTYUSBPort();
		g_LTEModemData.bIsSentHardResetCmdAlready = TRUE;
		return ATCMD_PROCESS_STAT_FINISHED;
	}
	return Wait_ATCmd(TRUE);
}


static void On_NotInit()
{
	TRACEW("On_NotInit");
	//now send display close cmd
	if(!g_LTEModemData.bATE0_ProcessOK)
	{
		//if(Send_ATCmd(ATCMD_CLOSEDISP_ANDSAVE) != ATCMD_PROCESS_STAT_FINISHED)
		Send_ATCmd(ATCMD_QUERY_CSQ);
		return;
	}

	//now set -> idle for dialing
	g_LTEModemData.emModemStatus = LTEMODEM_STATUS_IDLE;
	g_LTEModemData.nGetCSQErrorCounterOndle = 0;
	g_LTEModemData.tmLastEnterIdle = _NOW_;
}

static void On_Idle()
{	
	TRACEW("On_Idle");
	//step 1: check SIM card status 
	if(!g_LTEModemData.bIsSIMCardReady)
	{
		g_LTEModemData.nCurrentCSQ = 0;
		g_LTEModemData.tmLastTimeGetCSQ = 0;
		Send_ATCmd(ATCMD_QUERY_SIMSTATUS);
	}

	if(!g_LTEModemData.bIsSIMCardReady)
	{
		if(g_LTEModemData.nNoSimCardFoundCounter >= 3)
		{
			g_LTEModemData.emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
			g_LTEModemData.fWaitResetTime = 0.0;
		}
		return;
	}
	else
	{
		g_LTEModemData.nNoSimCardFoundCounter = 0;
	}

	//step 3: now check if CSQ is good enough for dialing
	if((g_LTEModemData.nCurrentCSQ >= 2) && (g_LTEModemData.nCurrentCSQ <= 31))
	{
		g_LTEModemData.nGetCSQErrorCounterOndle = 0;
		//get IMSI & ICCID
		if(!g_LTEModemData.bGetIMSIOK)
		{
			Send_ATCmd(ATCMD_QUERY_IMSI);		
		}
		else if(!g_LTEModemData.bGetICCIDOK)
		{
			Send_ATCmd(ATCMD_QUERY_ICID);		
		}
		else
		{
			g_LTEModemData.emModemStatus = LTEMODEM_STATUS_DIALING;
			g_LTEModemData.tmLastStartDial = 0;
		}
	}
	else
	{
		if(g_LTEModemData.nGetCSQErrorCounterOndle >= 10)
		{
			g_LTEModemData.emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
			g_LTEModemData.fWaitResetTime = 0.0;
		}
		else
		{
			//step 2: get CSQ
			if(ABS(g_LTEModemData.tmLastTimeGetCSQ - _NOW_) > 5)
			{
				Send_ATCmd(ATCMD_QUERY_CSQ);
			}
		}
	}
}

static void On_Dailing(void)
{
	TRACEW("On_Dailing");
#define DIALING_PROCESS_DEACTPDP	0
#define DIALING_PROCESS_SETPDP		1
//#define DIALING_PROCESS_ACTPDP		2 //Jimmy note:do NOT need this
#define DIALING_PROCESS_ECMCALL		3
#define DIALING_PROCESS_WAITDHCP	4
	static int s_emDialingProcess = DIALING_PROCESS_DEACTPDP; 
	static int s_nErrCount = 0;
	if(g_LTEModemData.tmLastStartDial == 0)
	{		
		s_nErrCount = 0;
		s_emDialingProcess = DIALING_PROCESS_DEACTPDP; 
		g_LTEModemData.tmLastStartDial = _NOW_;
	}
	else if(ABS(g_LTEModemData.tmLastStartDial - _NOW_) > 120)
	{
		s_nErrCount++;
		if(s_nErrCount >= 20)
		{
			g_LTEModemData.emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
			g_LTEModemData.fWaitResetTime = 0.0;
			s_nErrCount = 0;
		}
	}
	//step 1: De-Act PDP
	if(s_emDialingProcess == DIALING_PROCESS_DEACTPDP)
	{
		Send_ATCmd(ATCMD_PDP_DEACT);
		s_emDialingProcess = DIALING_PROCESS_SETPDP;			
	}
	else if(s_emDialingProcess == DIALING_PROCESS_SETPDP)
	{
		Send_ATCmd(ATCMD_PDP_SET);	
		g_LTEModemData.bDialOK = FALSE;
		//s_emDialingProcess = DIALING_PROCESS_ACTPDP;		
		s_emDialingProcess = DIALING_PROCESS_ECMCALL;
	}
	// else if(s_emDialingProcess == DIALING_PROCESS_ACTPDP)
	// {
	// 	Send_ATCmd(ATCMD_PDP_ACT);
	// 	g_LTEModemData.bDialOK = FALSE;
	// 	s_emDialingProcess = DIALING_PROCESS_ECMCALL;
	// }
	else if(s_emDialingProcess == DIALING_PROCESS_ECMCALL)
	{		
		if(g_LTEModemData.bDialOK)
		{
			s_emDialingProcess = DIALING_PROCESS_WAITDHCP;
			return;
		}	
		Send_ATCmd(ATCMD_START_ECMCALL);		
	}
	else if(s_emDialingProcess == DIALING_PROCESS_WAITDHCP)
	{
		//now do the dhcp
		_SYSTEM("ps|grep udhcp|grep -v grep|cut -c 0-5|xargs kill -9; udhcpc -b -i usb0&");
		s_emDialingProcess = DIALING_PROCESS_DEACTPDP;
		Send_ATCmd(ATCMD_QUERY_NETINFO); //query netInfo first
		g_LTEModemData.emModemStatus = LTEMODEM_STATUS_READY;
		g_LTEModemData.fNotConnectedTime = 0.0;
		g_LTEModemData.bIsNetConneced = FALSE;
		s_nErrCount = 0;
	}
	else
	{
		s_emDialingProcess = DIALING_PROCESS_DEACTPDP;
		s_nErrCount = 0;
	}
}

static void On_Ready(void)
{
	TRACEW("On_Ready");
	//judge if connected OK
	BOOL bIsConneced = GetLTENetworkInfo_AndJudgeIfConnected(g_LTEModemData.pSys_Rough, "usb0");
	if(bIsConneced)
	{
		g_LTEModemData.nDialFailCounter = 0;
		if(!g_LTEModemData.bIsNetConneced)
		{
			LogOut(MAIN_MODULE_SAMPLER, "SystemSamp", LOG_TYPE_WARING, "LTE modem dialed OK, now READY for WLAN!!!");
		}
		g_LTEModemData.nGetCSQErrorCounterOndle = 0;
		g_LTEModemData.bIsNetConneced = TRUE;
		g_LTEModemData.nReDialFailCount = 0;
		if(ABS(g_LTEModemData.tmLastTimeGetCSQ - _NOW_) >= 30)
		{
			Send_ATCmd(ATCMD_QUERY_CSQ);	
		}
		else if(ABS(g_LTEModemData.tmLastGetNetInfo - _NOW_) >= 30)
		{
			Send_ATCmd(ATCMD_QUERY_NETINFO);	
		}
	}
	else
	{
		static double s_tmLastTimeTick = 0.0;
		double tmNow = GetCurrentTime();
		double diff = tmNow - s_tmLastTimeTick;
		if((diff > 0.0) && (diff < 10.0))
		{
			g_LTEModemData.fNotConnectedTime += diff;
		}
		s_tmLastTimeTick = tmNow;		
		if(g_LTEModemData.fNotConnectedTime > 60.0)
		{
			Send_ATCmd(ATCMD_STOP_ECMCALL);
			g_LTEModemData.bIsNetConneced = FALSE;
			g_LTEModemData.emModemStatus = LTEMODEM_STATUS_DIAL_FAIL;
			return;
		}
		else 
		{
			if(ABS(g_LTEModemData.tmLastTimeGetCSQ - _NOW_) >= 10)
			{
				Send_ATCmd(ATCMD_QUERY_CSQ);	
			}
			else if(ABS(g_LTEModemData.tmLastGetNetInfo - _NOW_) >= 10)
			{
				Send_ATCmd(ATCMD_QUERY_NETINFO);	
			}
		}
	}
}

static void On_DialFailed(void)
{
	TRACEW("On_DialFailed");
	g_LTEModemData.nGetCSQErrorCounterOndle = 0;
	g_LTEModemData.nReDialFailCount++;
	if(g_LTEModemData.nReDialFailCount >= 3)
	{
		g_LTEModemData.emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
		g_LTEModemData.fWaitResetTime = 0.0;
		g_LTEModemData.bIsFinishSoftReset = FALSE;
		return;
	}
	//need goto idle to judge SIM card status again
	g_LTEModemData.emModemStatus = LTEMODEM_STATUS_IDLE;	
	g_LTEModemData.bIsSIMCardReady = FALSE;
}

static void On_WaitReset(void)
{
	TRACEW("On_WaitReset");
	//soft-reset the modem is usually OK enough?
	// int nResetRet = Process_ATCmd(ATCMD_SOFT_RESET_MODEM);
	// if(nResetRet == ATCMD_PROCESS_STAT_PENDING)
	// {
	// 	return;
	// }
	// else if(nResetRet == ATCMD_PROCESS_STAT_TIMEOUT)
	// {
	// 	return;
	// }
	// else if(nResetRet == ATCMD_PROCESS_STAT_TIMEOUT)
	// {
	// 	g_LTEModemData.bIsFinishSoftReset = TRUE;
	// }	
	// g_LTEModemData.bIsNetConneced = FALSE;
	// if(g_LTEModemData.fdTTYUSB != -1)
	// {
	// 	Send_ATCmd(ATCMD_SOFT_RESET_MODEM);
	// 	Log_Modem(LOGMODEM_TYPE_CLOSEPORT, "For soft-reset...");
	// 	g_LTEModemData.bIsFinishSoftReset = TRUE;
	// }

	// static double s_tmLastTimeTick = 0.0;
	// double tmNow = GetCurrentTime();
	// double diff = tmNow - s_tmLastTimeTick;
	// s_tmLastTimeTick = tmNow;		
	// if((diff > 0.0) && (diff < 10.0))
	// {
	// 	g_LTEModemData.fWaitResetTime += diff;
	// }
	// if(g_LTEModemData.fWaitResetTime > 10.0)
	// {
	// 	g_LTEModemData.bIsFinishTurnOffModem = 99;
	// 	g_LTEModemData.emModemStatus = LTEMODEM_STATUS_DISABLING;
	// }
	// else if(g_LTEModemData.bIsFinishSoftReset && (g_LTEModemData.fWaitResetTime > 5.0))
	// {
	// 	//judge if ttyUSB2 exixts for the case
	// 	if(access((g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_EC200U) ? "/dev/ttyUSB0" : "/dev/ttyUSB2", F_OK) == 0)
	// 	{
	// 		g_LTEModemData.emModemStatus = LTEMODEM_STATUS_NOT_INIT;
	// 		Init_LTEModemStatus();
	// 	}	
	// }	

	//changed by Jimmy as RST is of no use here
	g_LTEModemData.bIsFinishTurnOffModem = FALSE;
	g_LTEModemData.emModemStatus = LTEMODEM_STATUS_DISABLING;
}

static void On_Disabling(void)
{
	TRACEW("On_Disabling");
	time_t tmNOW = _NOW_;
	g_LTEModemData.bIsNetConneced = FALSE;
	if(!g_LTEModemData.bIsFinishTurnOffModem)
	{
		TRACEW("13");
		static time_t s_tmLastCallHardReset = 0;
		if(ABS(s_tmLastCallHardReset - tmNOW) > 10) //in case of multi call that crash the system
		{
			if(!g_LTEModemData.bIsSentHardResetCmdAlready)
			{
				TRACEW("14");
				Send_ATCmd(ATCMD_SOFT_TURNOFF_MODEM);
				Log_Modem(LOGMODEM_TYPE_CLOSEPORT, "For turn-off...");
				s_tmLastCallHardReset = tmNOW;				
			}
		}
		else
		{
			TRACEW("15");
			return;
		}

		g_LTEModemData.bIsFinishTurnOffModem = 2; //means the turn off AT cmd exe OK/fail/timeout
		g_LTEModemData.tmLastEnterTurnOff = _NOW_;
	}	
	else if (g_LTEModemData.bIsFinishTurnOffModem == 2)
	{
		TRACEW("16");
		if(ABS(tmNOW - g_LTEModemData.tmLastEnterTurnOff) >= 5)
		{
			TRACEW("17");
			g_LTEModemData.bIsFinishTurnOffModem = 99;
		}
	}		
	else if(g_LTEModemData.bIsFinishTurnOffModem == 99)
	{
		TRACEW("18");
		if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_NOTINST)
		{	
			TRACEW("19");	
			_SYSTEM("/app/modem_reset stop &");
			Log_Modem(LOGMODEM_TYPE_STOP_ORRESETMODEM, "********Hard-stopping the modem********");
			LogOut(MAIN_MODULE_SAMPLER, "SystemSamp", LOG_TYPE_WARING, "LTE modem has been STOPPED!!!");
		}	
		else
		{
			TRACEW("20");
			//don't reset the modem too frequently
			int TIMOUTTT = MIN(300, g_LTEModemData.nDialFailCounter * 60);
			TIMOUTTT = MAX(60, TIMOUTTT);
			if(ABS(g_LTEModemData.tmLastHardResetModem - _NOW_) < TIMOUTTT)
			{
				return;
			}
			_SYSTEM("/app/modem_reset sim0 &");
			Log_Modem(LOGMODEM_TYPE_STOP_ORRESETMODEM, "********Hard-resetting the modem, modemType->%d********", g_LTEModemData.emLTEModemType);
			LogOut(MAIN_MODULE_SAMPLER, "SystemSamp", LOG_TYPE_WARING, "LTE modem has been HARD-RESET for modemType->%d!!!",g_LTEModemData.emLTEModemType);
			g_LTEModemData.tmLastHardResetModem = _NOW_;
			g_LTEModemData.nDialFailCounter++;
			if(g_LTEModemData.nDialFailCounter > 3) //to avoid reseting too frequently
			{
				g_LTEModemData.nDialFailCounter = 3;
			}
		}	
		g_LTEModemData.bIsFinishTurnOffModem = TRUE;	
	}
	else
	{
		TRACEW("21");
		if(g_LTEModemData.emLTEModemType != LTEMODE_INSTTYPE_NOTINST)
		{
			TRACEW("22");
			if(ABS(tmNOW - g_LTEModemData.tmLastHardResetModem) >= 10)
			{
				TRACEW("23");
				if(access(GetTTYUSBPort_ByModemType(), F_OK) == 0)
				{
					TRACEW("24");
					g_LTEModemData.emModemStatus = LTEMODEM_STATUS_NOT_INIT;	
					Init_LTEModemStatus();		
				}					
			}
			else if(ABS(tmNOW - g_LTEModemData.tmLastHardResetModem) >= 300)
			{
				TRACEW("25");
				g_LTEModemData.bIsFinishTurnOffModem = 99;
			}
		}		
	}
}

void Notice_LTEModem_4Redial(void)
{
	TRACEW("Notice_LTEModem_4Redial");
	g_LTEModemData.bNeedRedial = TRUE;
}

static void Update_USBNetInfo(SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bForceNotInstall)
{
	TRACEW("Update_USBNetInfo");
	//now update netInfo
	NET_INFO* pUSBNetInfo = &pSys_Rough->currNetWorkInfo.netInfo_usb0;	
	int nNetStatus = NETPORT_STATUS_NOTINST;
	if((g_LTEModemData.emModemStatus != LTEMODEM_STATUS_DISABLING) && (!bForceNotInstall))
	{
		nNetStatus = g_LTEModemData.bIsNetConneced ? NETPORT_STATUS_CONNECTED : NETPORT_STATUS_DISCONN;
	}
	static NET_INFO s_OldUSBNetInfo;
	if((pUSBNetInfo->emNetPort_Status != nNetStatus) ||
		(s_OldUSBNetInfo.ulIpV4_IP != pUSBNetInfo->ulIpV4_IP) ||
		(s_OldUSBNetInfo.ulIpV4_Gateway != pUSBNetInfo->ulIpV4_Gateway))
	{
		pUSBNetInfo->emNetPort_Status = nNetStatus;
		Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler,
				1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
	}	
	memcpy(&s_OldUSBNetInfo, pUSBNetInfo, sizeof(NET_INFO));
}

void Manage_InternalLTEModem(SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bIsJigTestMode)
{
	TRACEW("=========================================");
	TRACEW("Manage_InternalLTEModem");
	typedef struct
	{
		LTEMODEM_STATUS 	emStat;
		ON_LETMODEM_STATE 	pfnState;
	}LTEMODEM_STATEMACHINE_ITEM;
	static LTEMODEM_STATEMACHINE_ITEM s_StatMachineList[] = 
	{
		{LTEMODEM_STATUS_NOT_INIT,		On_NotInit},
		{LTEMODEM_STATUS_IDLE,			On_Idle},
		{LTEMODEM_STATUS_DIALING,		On_Dailing},
		{LTEMODEM_STATUS_READY,			On_Ready},
		{LTEMODEM_STATUS_DIAL_FAIL,		On_DialFailed},
		{LTEMODEM_STATUS_WAIT_RESET,	On_WaitReset},
		{LTEMODEM_STATUS_DISABLING,		On_Disabling}
	};

	int nModemInstType = Get_LTEModemInstType(pSys_Rough, bIsJigTestMode);
	TRACEI("Get_LTEModemInstType = [%d]",nModemInstType);
	
	//if(g_LTEModemData.emLTEModemType == LTEMODE_INSTTYPE_NOTINST)
	if(nModemInstType == LTEMODE_INSTTYPE_NOTINST)
	{
		//need close old session if have
		if(s_bIsInitialized == TRUE)
		{		
			_SYSTEM("/app/modem_reset stop &");
			Log_Modem(LOGMODEM_TYPE_STOP_ORRESETMODEM, "********Hard-stopping(disable) the modem********");
			LogOut(MAIN_MODULE_SAMPLER, "SystemSamp", LOG_TYPE_WARING, "LTE modem has been DISABLED!!!");	
			Close_TTYUSBPort();
			g_LTEModemData.tmLastHardResetModem = 0;
			s_bIsInitialized = FALSE;

			g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = 0;
			g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0;
			memset(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szIMSI, 0, 20);
			memset(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szICCID, 0, 32);
			memset(g_LTEModemData.pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, 0, 32);
			g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_NOTINST;
			g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0.ulIpV4_IP = 0;
			g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0.ulIpV4_Mask = 0;
			g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0.ulIpV4_Gateway = 0;
			g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0.ulIpV4_DNSServer = 0;
			memset(g_LTEModemData.pSys_Rough->currNetWorkInfo.netInfo_usb0.szMacAddr, 0, 6);

			Push_BlobValue(SETTYPE_RAW_ST, g_LTEModemData.pSys_Rough->hSampler,
				1, 3, (void*)&g_LTEModemData.pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS); 
		
			g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi = 0;
			memset(g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.szIMSI, 0, 20);
			g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid = 0;
			memset(g_LTEModemData.pSys_Rough->blobOCPP_BootInfo.szICCID, 0, 32);

			Push_BlobValue(SETTYPE_INTERN_SETT, g_LTEModemData.pSys_Rough->hSampler, 
				1, 17, (void*)&g_LTEModemData.pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
		}	
		//Update_USBNetInfo(pSys_Rough, TRUE);	
		return;
	}
	else if (nModemInstType == LTEMODE_EXTTTYPE_ZR2000)
	{
		return;
	}
	else if((nModemInstType != g_LTEModemData.emLTEModemType) && (s_bIsInitialized != -99))
	{
		s_bIsInitialized = FALSE;
	}

	if(s_bIsInitialized != TRUE)
	{
		ReInitialize_LTEModemRoughData(pSys_Rough, nModemInstType);
		s_bIsInitialized = TRUE;		
	}

	//shall wait AT to be process before doing the StateMachine check
	if(Wait_ATCmd(FALSE) != ATCMD_PROCESS_STAT_PENDING)
	{
		//notice re-dial (maybe due to APN changed)
		if(g_LTEModemData.bNeedRedial)
		{
			if(g_LTEModemData.bIsNetConneced)
			{
				Send_ATCmd(ATCMD_STOP_ECMCALL);
			}
			Init_LTEModemStatus();
			g_LTEModemData.emModemStatus = LTEMODEM_STATUS_NOT_INIT; //LTEMODEM_STATUS_DIALING;	
			return;
		}
		
		int n;
		static int s_nOldStat = -1;		
		for(n = 0; n < ITEM_OF(s_StatMachineList); n++)
		{
			if(s_StatMachineList[n].emStat == g_LTEModemData.emModemStatus)
			{
				if(s_nOldStat != g_LTEModemData.emModemStatus)
				{
					// printf("-----[%.3f]: Now perform pfnStat change-> [%d -> %d]----\n", GetCurrentTime(), 
					// 	s_nOldStat, g_LTEModemData.emModemStatus);
					s_nOldStat = g_LTEModemData.emModemStatus;
				}				
				s_StatMachineList[n].pfnState();
				break;
			}
		}		
	}

	//now update net info
	Update_USBNetInfo(pSys_Rough, FALSE);

}
