
#include "app_incs.h"
#include "comm_httpftp.h"

#ifdef _DEBUG
//#define _DEBUG_HTTPFTP_PORT
#endif //_DEBUG

static BOOL s_bCurlInit = FALSE;
static CURLSH *s_pShared_Curl; //jimmy 20210201: for DNS cache so that to be used to improve DNS parsing performance

HANDLE HTTPFTP_CommOpen(
	IN char		*pPortDescriptor,
	IN char		*pOpenParams,
	IN DWORD		dwPortAttr,
	IN int		nTimeout,
	OUT int		*pErrCode)
{
	HTTPFTP_PORT_DRV *pPort = NEW(HTTPFTP_PORT_DRV, 1);
	if (pPort == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		return NULL;
	}
	memset(pPort, 0, sizeof(HTTPFTP_PORT_DRV));

	//UNUSED(pPortDescriptor);
	//UNUSED(pOpenParams);
	char* szUrl = pPortDescriptor;
	char* szFileName = pOpenParams;
	if((strlen(szUrl) < 5) || (strlen(szFileName) < 5))
	{
		*pErrCode = ERR_COMM_OPENING_PARAM;
		DELETE(pPort);
		return NULL;
	}

	pPort->emCurlRtn = CURLE_GOT_NOTHING;

	snprintf(pPort->szLocalFileName, 255, "%s", szFileName);

	//Jimmy modified this 20201015 this is a bug that may lead UPLOAD Fail!!!
	//and this check should be put in 'Read' function
	pPort->fLocalFile = NULL;
	// pPort->fLocalFile = fopen(szFileName, "w+"); //clear prev file
	// if(pPort->fLocalFile == NULL) 
	// {
	// 	*pErrCode = ERR_COMM_OPENING_PARAM;
	// 	DELETE(pPort);
	// 	return NULL;
	// }

	if(!s_bCurlInit)
	{
		//curl_global_init(CURL_GLOBAL_ALL); //Jimmy 20210201: put it out in [Sampler_loader]
		s_pShared_Curl = curl_share_init();
		curl_share_setopt(s_pShared_Curl, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
		s_bCurlInit = TRUE;
	}

	pPort->pCurl = curl_easy_init();
	if(pPort->pCurl == NULL)
	{
		*pErrCode = ERR_COMM_NO_MEMORY;
		DELETE(pPort);
		//fclose(pPort->fLocalFile);
		//pPort->fLocalFile = NULL;
		return NULL;
	}

	curl_easy_setopt(pPort->pCurl, CURLOPT_SHARE, s_pShared_Curl);
	curl_easy_setopt(pPort->pCurl, CURLOPT_DNS_CACHE_TIMEOUT, 300); //set the DNS timeout => 5min so it will be re-uesed within 5min

	//added by Jimmy 20200409 for supporting HTTP(s)
	if((strstr(szUrl, "ftp://") == szUrl) || (strstr(szUrl, "FTP://") == szUrl) ||
		(strstr(szUrl, "ftps://") == szUrl) || (strstr(szUrl, "FTPS://") == szUrl)|| //begin with ftp:// prefix
		(strstr(szUrl, "http://") == szUrl) || (strstr(szUrl, "HTTP://") == szUrl)||
		(strstr(szUrl, "https://") == szUrl) || (strstr(szUrl, "HTTPS://") == szUrl)) 
	
	{
		pPort->nWorkMode  = COMM_FTP_MODE;
	}
	else
	{
		//will add support later
		curl_easy_cleanup(pPort->pCurl);
		DELETE(pPort);
		//fclose(pPort->fLocalFile);
		//pPort->fLocalFile = NULL;
		*pErrCode = ERR_COMM_OPENING_PARAM;
		return NULL;
	}

	*pErrCode = ERR_COMM_OK;

	INIT_TIMEOUTS_EX(pPort->toTimeouts, 0, 0,  nTimeout); //use internal timeout for the case
	snprintf(pPort->szUrl, 255, "%s", szUrl);
	//put the following into the thread
	// curl_easy_setopt(pPort->pCurl, CURLOPT_VERBOSE, 1); //for debug use
	// curl_easy_setopt(pPort->pCurl, CURLOPT_CONNECTTIMEOUT, 20); //just connection timeout
	// curl_easy_setopt(pPort->pCurl, CURLOPT_TIMEOUT, nTimeout); //should set a proper value especially when network is not so good
	// curl_easy_setopt(pPort->pCurl, CURLOPT_URL, szUrl); //URL must include the user:password section
	// curl_easy_setopt(pPort->pCurl, CURLOPT_NOSIGNAL, 1); //������signal�źŸ����̣�����������̶δ��������/**/
	// curl_easy_setopt(pPort->pCurl, CURLOPT_USE_SSL, CURLUSESSL_ALL); //enable SSL
	
	pPort->nRetryMaxLmt = dwPortAttr; //retry max count

	TRACE("Open URL[%s], for file[%s] OK\n", szUrl, szFileName);

	return (HANDLE)pPort;
}

HANDLE HTTPFTP_CommAccept( IN HANDLE hPort )
{
	UNUSED(hPort); //not supported
	return NULL;
}

void* _threadCurlHandler(void* pArgs)
{
	HTTPFTP_PORT_DRV	*pPort = (HTTPFTP_PORT_DRV *)pArgs;

	if(pPort->fLocalFile == NULL)
	{
		TRACE("_________Now retry for httpFTP lib____________\n");
		pPort->fLocalFile = fopen(pPort->szLocalFileName, "a+");
		if(pPort->fLocalFile == NULL) 
		{
			curl_easy_cleanup(pPort->pCurl);

			TRACE("\n--------Download ThreaD Exit because file can not be OPENED-----------\n");

			return NULL;
		}
	}

	int fd = fileno(pPort->fLocalFile); //�ļ�ָ��ת��Ϊ�ļ�������/**/
	if(fd == -1)
	{
		pPort->emCurlRtn = CURLE_FAILED_INIT;
		return NULL;
	}

	//curl_easy_setopt(pPort->pCurl, CURLOPT_VERBOSE, 1); //for debug use
	curl_easy_setopt(pPort->pCurl, CURLOPT_CONNECTTIMEOUT, 20); //just connection timeout
	//curl_easy_setopt(pPort->pCurl, CURLOPT_TIMEOUT, pPort->toTimeouts.nIntervalTimeout); //should set a proper value especially when network is not so good
	curl_easy_setopt(pPort->pCurl, CURLOPT_URL, pPort->szUrl); //URL must include the user:password section
	curl_easy_setopt(pPort->pCurl, CURLOPT_NOSIGNAL, 1); //������signal�źŸ����̣�����������̶δ��������/**/
	curl_easy_setopt(pPort->pCurl, CURLOPT_TIMEOUT_MS, pPort->toTimeouts.nIntervalTimeout* 1000);
	curl_easy_setopt(pPort->pCurl, CURLOPT_USE_SSL, CURLUSESSL_TRY); //enable SSL
	curl_easy_setopt(pPort->pCurl, CURLOPT_SSL_VERIFYPEER, FALSE); //added by Jimmy 20201106: do not veriry SSL cert
	curl_easy_setopt(pPort->pCurl, CURLOPT_SSL_VERIFYHOST, FALSE);
	//added by Jimmy 20210913 for get HTTP passwd
	if(strlen(pPort->szHTTPAuthPwd) > 1)
	{
		curl_easy_setopt(pPort->pCurl, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
		curl_easy_setopt(pPort->pCurl, CURLOPT_USERPWD, pPort->szHTTPAuthPwd);
	}

	do
	{
		if(pPort->nWorkMode == COMM_FTP_MODE)
		{
			struct stat file_info;
			curl_off_t fsize = 0;
			BOOL bUse_Resume = FALSE;
			if(fstat(fd, &file_info) == 0)
			{
				fsize =  file_info.st_size;
				bUse_Resume  = (fsize > 0) ? TRUE : FALSE;
			}

			if(!pPort->bIsInUpload_Mode) //download
			{
				//���öϵ�����/**/
				curl_easy_setopt(pPort->pCurl, CURLOPT_RESUME_FROM, bUse_Resume? fsize : 0);
				curl_easy_setopt(pPort->pCurl, CURLOPT_WRITEFUNCTION, fwrite);
				curl_easy_setopt(pPort->pCurl, CURLOPT_WRITEDATA, pPort->fLocalFile);
			}
			else //upload
			{
				curl_easy_setopt(pPort->pCurl, CURLOPT_UPLOAD, 1L);
				curl_easy_setopt(pPort->pCurl, CURLOPT_READFUNCTION, fread);
				curl_easy_setopt(pPort->pCurl, CURLOPT_READDATA, pPort->fLocalFile);
				curl_easy_setopt(pPort->pCurl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)fsize);
			}

			//curl_easy_setopt(pPort->pCurl, CURLOPT_VERBOSE, 1L); //���Կ���/**/
			pPort->emCurlRtn = curl_easy_perform(pPort->pCurl);
			if (pPort->emCurlRtn == CURLE_OK)
			{
				break;
			}
		}
		else
		{
			//not supported yet
		}

		pPort->nRetryCount++;
	} while (pPort->nRetryCount <= pPort->nRetryMaxLmt);

	fclose(pPort->fLocalFile);

	pPort->fLocalFile = NULL;

	TRACE("\n--------Download ThreaD eXIT, Result = %d------------\n", pPort->emCurlRtn);

	return NULL;
}

int HTTPFTP_CommRead(IN HANDLE hPort, OUT char *pBuffer, IN int nBytesToRead)
{
	//added by Jimmy 20210913 to do HTTP basic auth
	//UNUSED(pBuffer);
	//UNUSED(nBytesToRead);
	HTTPFTP_PORT_DRV	*pPort = (HTTPFTP_PORT_DRV *)hPort;
	memset(pPort->szHTTPAuthPwd, 0, 64);
	if((nBytesToRead == -999) && (pBuffer != NULL))
	{
		snprintf(pPort->szHTTPAuthPwd, 64, "%s", pBuffer);
	}	

	pPort->fLocalFile = fopen(pPort->szLocalFileName, "w+"); //clear prev file
	if(pPort->fLocalFile == NULL) 
	{
		return -1;
	}

	if(pPort->nWorkMode == COMM_FTP_MODE)
	{
		//read means download
		pPort->bIsInUpload_Mode = FALSE;
		//curl_easy_setopt(pPort->pCurl, CURLOPT_RESUME_FROM, bUse_Resume? fsize : 0);
		//curl_easy_setopt(pPort->pCurl,, CURLOPT_WRITEFUNCTION, fwrite);
		//curl_easy_setopt(pPort->pCurl,, CURLOPT_WRITEDATA, pPort->fLocalFile);
		pPort->emCurlRtn = CURLE_GOT_NOTHING;
		//use thread so that it won't block
		if (pthread_create( &pPort->hThread, 
			NULL, _threadCurlHandler,
			(void *)hPort) != 0)
		{
			return -1;
		}

		pthread_detach(pPort->hThread);

		Sleep_Ex(50);
		if((pPort->emCurlRtn != CURLE_GOT_NOTHING) && 
			(pPort->emCurlRtn != CURLE_OK))
		{
			return -1;
		}

		return 0;
	}
	
	return -1; //0 means OK, -1 means Failed
}

int HTTPFTP_CommWrite(IN HANDLE hPort, IN char *pBuffer,	IN int nBytesToWrite)
{
	HTTPFTP_PORT_DRV	*pPort = (HTTPFTP_PORT_DRV *)hPort;
	if(pPort->nWorkMode == COMM_FTP_MODE)
	{
		//read means download
		pPort->bIsInUpload_Mode = TRUE;
		//curl_easy_setopt(pPort->pCurl,, CURLOPT_READFUNCTION, fread);
		//curl_easy_setopt(pPort->pCurl,, CURLOPT_READDATA, pPort->fLocalFile);
		pPort->emCurlRtn = CURLE_GOT_NOTHING;
		//use thread so that it won't block
		if (pthread_create( &pPort->hThread, 
			NULL, _threadCurlHandler,
			(void *)hPort) != 0)
		{
			return -1;
		}

		pthread_detach(pPort->hThread);

		Sleep_Ex(50);
		if((pPort->emCurlRtn != CURLE_GOT_NOTHING) && 
			(pPort->emCurlRtn != CURLE_OK))
		{
			return -1;
		}

		return 0;
	}

	return -1; //0 means OK, -1 means Failed
}

int HTTPFTP_CommControl(IN HANDLE hPort, IN int	nCmd, 
					  IN OUT void *pBuffer,	IN int	nDataLength)
{
	HTTPFTP_PORT_DRV *pPort = (HTTPFTP_PORT_DRV *)hPort;
	int nErrCode = ERR_COMM_OK;

	if (hPort == NULL)
	{
		return ERR_COMM_PORT_HANDLE;
	}

	//ASSERT( pBuffer );

	switch (nCmd)
	{
	//case COMM_GET_LAST_ERROR:	/* get the last error code	*/
	//	{
	//		ASSERT(nDataLength == sizeof(int));

	//		*(int *)pBuffer = pPort->nLastErrorCode;
	//	}

	//	break;

	//case COMM_SET_TIMEOUTS:		/* set the comm timeout in ms. */
	//	{
	//		/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
	//		ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

	//		pPort->toTimeouts = *(COMM_TIMEOUTS *)pBuffer;
	//	}
	//	break;

	//case COMM_GET_TIMEOUTS:		/* get the comm timeout in ms	*/
	//	{
	//		/* bBuffer=COMM_TIMEOUTS, nDataLength=sizeof(COMM_TIMEOUTS)		*/
	//		ASSERT(nDataLength == sizeof(COMM_TIMEOUTS));

	//		*(COMM_TIMEOUTS *)pBuffer = pPort->toTimeouts;
	//	}
	//	break;

	case COMM_GET_HTTPFTP_PROCESS:
		if(pPort->emCurlRtn == CURLE_OK)
		{
			nErrCode = ERR_COMM_OK;
		}
		else if(pPort->emCurlRtn == CURLE_GOT_NOTHING) //means pending
		{
			nErrCode = ERR_COMM_HTTPFTP_PENDING;
		}
		else
		{
			nErrCode = ERR_COMM_CTRL_EXECUTION; //download/upload failed
		}
		break;
	default:
		{
			nErrCode = ERR_COMM_CTRL_COMMAND;
		}
	}

	return nErrCode;
}

int HTTPFTP_CommClose( IN HANDLE hPort )
{
	HTTPFTP_PORT_DRV *pPort = (HTTPFTP_PORT_DRV *)hPort;

	if(pPort->emCurlRtn == CURLE_GOT_NOTHING)
	{
		TRACE("\n__________Now need cancel the HTTPGTP thread___________\n");
		pthread_cancel(pPort->hThread);

		if(pPort->fLocalFile != NULL)
		{
			fclose(pPort->fLocalFile);
		}
	}

	TRACE("\n---Now clear the HTTPFTP curl handle and delte the port---\n");
	curl_easy_cleanup(pPort->pCurl);

	DELETE(pPort);

	return ERR_COMM_OK;
}

