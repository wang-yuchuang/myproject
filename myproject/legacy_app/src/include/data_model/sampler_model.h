#ifndef _SAMPLER_2018_04_04_
#define _SAMPLER_2018_04_04_

#include "base_defines/basetypes.h"
#include "comm_ports/comm_port.h"

typedef enum tagSamplerErrCode
{
	ERR_SAMPLE_OK = 0,	
	ERR_SAMPLE_TIMEOUT,
	ERR_SAMPLE_NEED_RESAMP,
	ERR_SAMPLE_FAILED,

	ERR_CONTROL_OK = 0,
	ERR_CONTROL_FAILED,
	ERR_CONTROL_TIMEOUT,
	ERR_CONTROL_COMFAIL,
	ERR_CONTROL_NOTSUPPORTED //added by Jimmy 20230625
}ERRCODE_SAMPLER;

typedef struct tagSamplerInfo
{
	MODULE_TYPE	emModuleType; //shall put in first place
	int 			nSamplerID; //shall put in second place
	char			szSamplerName[32];
	char			szSamplerDriver[32];
	char			szSamplerSetting[64];
	int			nSamplePeriod_mSec;

}SAMPLER_INFO;

#define SAMPLER_INIT_PROC_NAME	"Initialize"
typedef BOOL (*SAMPLER_INIT_PROC)  /* query function prototype */
	(
		HANDLE	hSamplerSelf	//point to this Sampler so that we can use it
	);

#define SAMPLER_SAMPLE_PROC_NAME	"Sample"
typedef int (*SAMPLER_SAMPLE_PROC)  /* query function prototype */
	(
		HANDLE	 hSamplerSelf		//point to this Sampler so that we can use it
	);

#define SAMPLER_JIG_TEST_PROC_NAME	"JigTest"
typedef int (*SAMPLER_JIG_TEST_PROC)  /* for jig test function prototype */
	(
		HANDLE	hSamplerSelf,
		void* 	pJigData			/* parameter can used by test */
	);

#define SAMPLER_CONTROL_PROC_NAME	"Control"
#define CONTROL_TIMEOUT_INFINIT	(UINT)(-1)
typedef int (*SAMPLER_CONTROL_PROC)  /* control function prototype */
	(
		HANDLE		hSamplerThis,		//point to this Sampler so that we can use it
		PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut		//how many milliseconds on timeout, (ulong)-1 means infinite
	);

#define SAMPLER_NOTIFY_PROC_NAME	"Notify"
typedef int (*SAMPLER_NOTIFY_PROC)  /* notify function prototype */
	(
		HANDLE		hSamplerThis,		//point to this Sampler so that we can use it
		int			nNoticeId,
		void*			pMsg,
		int			nSizeMsg
	);
#define MAX_CTRL_CMD_NUM	32
	
typedef struct tagSampler
{
	SAMPLER_INFO				info;
	COMMPORT*				pUsedCommPort;

	/* runtime info */
	HANDLE					hCommPort;
	HANDLE					hSamplerLib;
	HANDLE					hSampleThread;
	HANDLE					hSamplerLock;

	HANDLE					hDPR; //Use dataprocessor API to Get/Set data

	void*						pDataHeap; //used to point to sampler private data

	SAMPLER_INIT_PROC			Init;
	SAMPLER_CONTROL_PROC		Control;
	SAMPLER_SAMPLE_PROC		Sample;

	SAMPLER_JIG_TEST_PROC		JigTest; //Optional

	SAMPLER_NOTIFY_PROC		Notify; //used by DPR to notify the sampler for msg

	// max pending command in a port, see MAX_PENDING_CTRL_CMD
	HANDLE			hqControlCommand;	  /* command queue of the port */
	
//#define MAX_TIME_WAITING_PORT_LOCK		10000
	//HANDLE			hSyncLock;			/* the synchronization lock */
#define  SAMPLER_IS_WORK		0
#define  SAMPLER_IS_DISCON		1
#define  SAMPLER_NOT_WORK		2
	BYTE			emWorkStatus;// sampler work status, 0----Working, 1---Disconnected, 2----None-working(non-working does not mean disconnected)

	double		tmLastSampled;// time last sample data.
	double		fElapsedTime;	// the elapsed time from the last sampling action.
	// NOTE: the time changing is NOT considered.
	BOOL			bJustControlled;// The sampler just sent a control, Need sample data now.
	BOOL			bLastSampleInterrupted; //If last sample interrupted, need re-sample in next loop

	BOOL			bIsInitlized; //Is Init OK/**/

	BOOL			bExit; //exit flag for the thread

}SAMPLER;
#endif //__CFG_MODEL_H_2004_09_09__
