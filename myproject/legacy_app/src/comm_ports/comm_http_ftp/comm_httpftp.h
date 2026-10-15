#ifndef __NET_HTTP_FTP_H_DEFINED
#define __NET_HTTP_FTP_H_DEFINED

#include "std_sys.h"		/* all standard system head files			*/
#include "3rd_party/curl/curl.h"

/* 
 * If want to test the module with static mode,
 * please comment out the next line.
 */
#define _MAKE_SHARED_LIB

#ifdef _MAKE_SHARED_LIB

#define HTTPFTP_CommOpen			HAL_CommOpen
#define HTTPFTP_CommAccept			HAL_CommAccept
#define HTTPFTP_CommRead			HAL_CommRead
#define HTTPFTP_CommWrite			HAL_CommWrite
#define HTTPFTP_CommControl			HAL_CommControl
#define HTTPFTP_CommClose			HAL_CommClose

#endif //_MAKE_SHARED_LIB



struct SHttpFtpDriver
{
	//int					nLastErrorCode;		//NOTE: MUST BE THE FIRST FIELD! 
										//  the last error code.

	int					nWorkMode;		/* COMM_LOCAL_SERVER,
										 * COMM_CONNECTED_CLIENT,
										 * or COMM_ACCEPTED_CLIENT
										 */
	CURL*				pCurl;
	CURLcode				emCurlRtn;
	pthread_t				hThread;			//use thread to handle data
	FILE					*fLocalFile;

	DWORD				nRetryMaxLmt;		//from app setting
	DWORD				nRetryCount;

	COMM_TIMEOUTS		toTimeouts;
	BOOL					bIsInUpload_Mode;

	char					szUrl[255];
	char					szLocalFileName[255];

	//added by Jimmy for HTTP auth
	char					szHTTPAuthPwd[64];

};
typedef struct SHttpFtpDriver		HTTPFTP_PORT_DRV;


#endif

