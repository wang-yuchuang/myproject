#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h> 

#include "main.h"

static int Sig_InstallHandlers(BOOL bIgnoreOthers, 
						void *pfnSigHandler1, int nSigNo1, ...)
{
	int				nSigNo;
	__sighandler_t		pSighandler;

	va_list				sigList;
	int					nResult = 0;

  	struct sigaction	act;

	if (bIgnoreOthers)
	{
		// ignore all signal at first
		for (nSigNo = 1; nSigNo < _NSIG; nSigNo++)
		{
			sigemptyset(&act.sa_mask);
			act.sa_flags					 = SA_SIGINFO;
			//(__sighandler_t)act.sa_sigaction = (__sighandler_t)SIG_IGN;
			pSighandler = SIG_IGN;
			memcpy(&act.sa_sigaction, &pSighandler, sizeof(__sighandler_t));

			sigaction((int)nSigNo, &act, NULL);
		}
	}

	// set the signal handler we want to handle.
	va_start(sigList, nSigNo1);   /* Initialize variable arguments. */

	nSigNo      = nSigNo1;
	pSighandler = (__sighandler_t)pfnSigHandler1;

	while ((nSigNo > 0) && (pSighandler != NULL))
	{
		/* Set up the structure to specify the new action. */
		sigemptyset(&act.sa_mask);
		act.sa_flags		             = SA_SIGINFO;
		//(__sighandler_t)act.sa_sigaction = (__sighandler_t)pSighandler;
		memcpy(&act.sa_sigaction, &pSighandler, sizeof(__sighandler_t));

#ifdef _DEBUG
		//printf("Installing signal #%d handler %p...\n",
		//		nSigNo, pSighandler);
#endif //_DEBUG

		if (sigaction((int)nSigNo, &act, NULL) < 0)	// ignore old.
		{
#ifdef _DEBUG
			int		nErrNo = errno;
			printf("Installing signal #%d handler %p got error %d(%s).\n",
				nSigNo, pSighandler,
				nErrNo, strerror(nErrNo));
#endif //_DEBUG

			nResult = -1;
		}

		// get the next signal handle info.
		nSigNo		= va_arg(sigList, int);
		if (nSigNo == 0)// to get next handler.
		{
			pSighandler = va_arg(sigList, __sighandler_t);
			if (pSighandler != NULL)
			{
				nSigNo		= va_arg(sigList, int);
			}
		}
	}

	va_end(sigList);              /* Reset variable arguments.      */

	return nResult;
}

static void Main_HandleChildSignal(IN int nSig)
{
	UNUSED(nSig);
//	TRACE("Handle Child Signal.\n");
}
static void Main_HandleFaultSignals(IN int nSig, IN siginfo_t *pSigInfo, 
							   IN void *pSigVal)
{
	UNUSED(pSigVal);

	TRACEX("Received a segment fault, system will crash soon....\n");
	LogOut(MAIN_MODULE_MAIN, "Sig_handler", LOG_TYPE_ERROR,
		"PANIC: Received fault sig(signo:%d,code=%d) at addr:%p. System is rebooting...",
		nSig, pSigInfo->si_code, pSigInfo->si_addr);

	Main_Exit(SYS_EXIT_BY_FAULT);
}

static void Main_HandleStopSignals(IN int nSig)
{
	UNUSED(nSig);
	static int		nKillCount = 0;
	static time_t	tmLastSignaled = 0;
	time_t			tmNow = time(NULL);

	if (tmNow - tmLastSignaled > 10)	// 10 seconds
	{
		nKillCount = 0;
	}

	tmLastSignaled = tmNow;

	nKillCount++;

	//printf("\n-----Received a kill command, now exit the app...------\n");
	switch (nKillCount)
	{
	case 1:
		//Jimmy注释:直接一次性退出，省的麻烦
//#define SYS_PREQUIT_MSG	"Received a quit command, please send "\
//			"command again to stop system.\n"
//		AppLogOut(SYS_MAIN, APP_LOG_UNUSED, "%s", SYS_PREQUIT_MSG);
//		printf("%s", SYS_PREQUIT_MSG);
//
//		break;

	case 2:
#define SYS_INTR_MSG	"Received user kill command, system is now quiting..."
		LogOut(MAIN_MODULE_MAIN, "Sig_handler", LOG_TYPE_WARING, "%s", SYS_INTR_MSG);

		printf("%s", SYS_INTR_MSG);
		Main_Exit(SYS_EXIT_BY_USER);

		//if (nSig == SIGUSR2)
		//{
		//	s_nSystemExitingCode  = SYS_EXIT_BY_REBOOT_REQ;
		//	AppLogOut(SYS_MAIN, APP_LOG_WARNING, 
		//		"And the System will reboot after quiting for received reboot cmd.\n");
		//}
		//else
		//{
		//	Main_AckStopCmd();
		//}

		break;

	default:		
		printf( "System is quiting, please wait... \n" );
		system("/sbin/reboot");
		break;
	}
}

static void Main_HandleSysHandErr(IN int nSig, IN siginfo_t *pSigInfo, 
	IN void *pSigVal)
{
	UNUSED(pSigVal);

	TRACEX("Received a syshand err signal....\n");
	LogOut(MAIN_MODULE_MAIN, "Sig_handler", LOG_TYPE_ERROR,
		"PANIC: SysHand init or execute error! System is rebooting...");

	Main_Exit(SYS_EXIT_BY_FAULT);
}

BOOL Install_SigHandlers(void* pArgs)
{
	UNUSED(pArgs);
	//s_pChargeStub = (CHARGE_STUB*)pArgs;
	int nError = Sig_InstallHandlers(TRUE, // ignore all signal at first.
		Main_HandleStopSignals,  SIGTERM, SIGQUIT, 0,			// quit
		Main_HandleFaultSignals, SIGILL,  SIGFPE,  SIGSEGV, SIGBUS, SIGABRT, 0,	// error
		Main_HandleChildSignal,	 SIGCHLD, 0,
		Main_HandleSysHandErr,	 SIGUSR1, 0,							//syshand process will send this when error
		NULL);

	return (nError >= 0);
}


