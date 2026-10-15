// #define _TRACE_PRINT_
#include "sampler_system.h"
#include "3rd_party/curl/curl.h"

#define GET_IP_MASK	0x01
#define GET_NM_MASK	0x02
#define GET_GW_MASK	0x04
#define GET_MAC_MASK	0x10

static int GetNetInfo(char* szEthId, NET_INFO* pNetInfo)
{
	int nMask = 0;
	int nSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (nSock == -1)
	{
		TRACEX("[GetNetInfo] -- create socket fail %x\n", errno);
		return nMask;
	}

	struct ifreq		ifr;
	struct sockaddr_in	*p = (struct sockaddr_in *)&(ifr.ifr_addr);

	memset(&ifr,0,sizeof(struct ifreq));

	p->sin_family = AF_INET;
	strcpy(ifr.ifr_name, szEthId);

	//IP
	if (ioctl(nSock, SIOCGIFADDR, &ifr) == -1)
	{
		//return errno;
		pNetInfo->ulIpV4_IP = 0;
	}
	else
	{
		TRACEX("[GetNetInfo] -- ip:%s\n",
			inet_ntoa(*((struct in_addr*)&p->sin_addr.s_addr)));

		pNetInfo->ulIpV4_IP = (ULONG)p->sin_addr.s_addr;

		nMask |= GET_IP_MASK;
	}

	//Net mask
	if (ioctl(nSock, SIOCGIFNETMASK, &ifr) == -1)
	{
		//return errno;
		pNetInfo->ulIpV4_Mask = 0;
	}
	else
	{

		TRACEX("[GetNetInfo] -- mask:%s\n",
			inet_ntoa(*((struct in_addr*)&p->sin_addr.s_addr)));

		pNetInfo->ulIpV4_Mask= (ULONG)p->sin_addr.s_addr;

		nMask |= GET_NM_MASK;
	}

	//Get mac address
	if(ioctl(nSock, SIOCGIFHWADDR, &ifr) == -1) 
	{
		//return errno;
		ZERO_POBJS(pNetInfo->szMacAddr, 18);
	}
	else
	{
		//memcpy(pNetInfo->szMacAddr, ifr.ifr_hwaddr.sa_data, 6);
		snprintf(pNetInfo->szMacAddr, 18, "%02X-%02X-%02X-%02X-%02X-%02X",
			ifr.ifr_hwaddr.sa_data[0] & 0xFF, ifr.ifr_hwaddr.sa_data[1] & 0xFF, 
			ifr.ifr_hwaddr.sa_data[2] &0xFF, ifr.ifr_hwaddr.sa_data[3] & 0xFF,
			ifr.ifr_hwaddr.sa_data[4] & 0xFF, ifr.ifr_hwaddr.sa_data[5] & 0xFF);

		TRACEX("[GetNetInfo] -- mac:%02X-%02X-%02X-%02X-%02X-%02X\n",
			pNetInfo->szMacAddr[0] & 0xFF, pNetInfo->szMacAddr[1] & 0xFF, 
			pNetInfo->szMacAddr[2] &0xFF,	pNetInfo->szMacAddr[3] & 0xFF,
			pNetInfo->szMacAddr[4] & 0xFF, pNetInfo->szMacAddr[5] & 0xFF);


		nMask |= GET_MAC_MASK;
	}

	//Jimmy added here 20190307: found that the fd handles increases in run-time
	close(nSock);

	//Gateway
	FILE *in;
	struct rtentry rtent;
	char line [256];
	char name[16];

	int rt_flags;
	int refcnt;
	unsigned int use;

	if (! (in = fopen ("/proc/net/route", "r")))
	{
		TRACEX("cannot open /proc/net/route - burps\n\r");
		pNetInfo->ulIpV4_Gateway = 0;
		return nMask;
	}

	while (fgets (line, sizeof(line), in))
	{
		/*
		* as with 1.99.14:
		* Iface Dest GW Flags RefCnt Use Metric Mask MTU Win IRTT
		* eth0 0A0A0A0A 00000000 05 0 0 0 FFFFFFFF 1500 0 0 
		*/
		if (8 != sscanf (line, "%15s %x %x %x %u %d %d %x %*d %*d %*d\n",
			/*rt->rt_dev*/name,
			&(((struct sockaddr_in *) &(rtent.rt_dst))->sin_addr.s_addr),
			&(((struct sockaddr_in *) &(rtent.rt_gateway))->sin_addr.s_addr),
			/* XXX: fix type of the args */
			&(rt_flags), &refcnt, &use, ((int *)&(rtent.rt_metric)),
			&(((struct sockaddr_in *) &(rtent.rt_genmask))->sin_addr.s_addr)))
		{
			continue;
		}

		if (strcmp(name, szEthId) == 0)
		{
			if ((rt_flags & RTF_GATEWAY) && (rt_flags & RTF_UP))
			{
				pNetInfo->ulIpV4_Gateway = ((struct sockaddr_in *) 
					&(rtent.rt_gateway))->sin_addr.s_addr;

				nMask |= GET_GW_MASK;

				TRACEX("[GetNetInfo] -- gw:%s\n", inet_ntoa(*((struct in_addr*)&pNetInfo->ulIpV4_Gateway)));

				break;
			}
		}
	}

	fclose(in);

	if((nMask & GET_GW_MASK) != GET_GW_MASK)
	{
		TRACEX("Get[%s] GateWay error %d, so set it to 0...\n", szEthId, nMask);
		pNetInfo->ulIpV4_Gateway = 0;
	}

	return nMask;
}

static void SaveNetInfo_IntoScriptFile(char* szEthId, char*pFile, BOOL bDHCPOn, NET_INFO* pNetInfo)
{
	char szCmd[512] = {0};

	//Clear the content of the file
	char szCmd1[128] = {0};
	snprintf(szCmd1, sizeof(szCmd1), "> %s;", pFile);
	strcat(szCmd, szCmd1);

	//Jimmy optimized 20200615: use sing "'" instead of '"' will solve the
	//found "#!bin/sh" wrongly recognized as command so not saved problems.
	//Write #!/bin/sh
	memset(szCmd1, 0, 128);
	snprintf(szCmd1, sizeof(szCmd1), "echo '#!/bin/sh' >> %s;", pFile);
	strcat(szCmd, szCmd1);

	if(bDHCPOn)
	{
		memset(szCmd1, 0, 128);
		snprintf(szCmd1, sizeof(szCmd1), "echo 'dhclient %s -nw&' >> %s;", szEthId, pFile);
		strcat(szCmd, szCmd1);
	}

	if(pNetInfo->ulIpV4_IP != 0) //if not get iP from DHCP, do not save setting to DHCP file
	{
		//Write /sbin/ifconfig eth0 xxx.xxx.xxx.xxx ->IP addr
		char* szIP = /*(pNetInfo->ulIpV4_IP == 0) ? "192.168.1.100" :*/
			inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_IP)));
		memset(szCmd1, 0, 128);
		snprintf(szCmd1, sizeof(szCmd1), "echo '/sbin/ifconfig %s %s' >> %s;", szEthId,
			szIP, pFile);
		strcat(szCmd, szCmd1);

		//Write /sbin/ifconfig eth0 xxx.xxx.xxx.xxx ->Netmask
		char* szNetMask = (pNetInfo->ulIpV4_IP == 0) ? "255.255.255.0" :
			inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_Mask)));
		memset(szCmd1, 0, 128);
		snprintf(szCmd1, sizeof(szCmd1), "echo '/sbin/ifconfig %s netmask %s' >> %s;", szEthId,
			szNetMask, pFile);
		strcat(szCmd, szCmd1);
	}

	//Write route add default gw xxx.xxx.xxx.xxx ->Netmask
	if(pNetInfo->ulIpV4_Gateway != 0)
	{
		memset(szCmd1, 0, 128);
		snprintf(szCmd1, sizeof(szCmd1), "echo '/sbin/route add default gw %s %s' >> %s;", 
			inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_Gateway))), szEthId, pFile);
		strcat(szCmd, szCmd1);

		// //added by Jimmy 20190404 for solving the DNS issue on static IP
		// memset(szCmd1, 0, 128);
		// //Jimmy note 20210817: we define only ONE eth can have DNS server
		// snprintf(szCmd1, sizeof(szCmd1), "echo \"echo 'nameserver %s' > /etc/resolv.conf\" >> %s",
		// 	inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_Gateway))), pFile);
		// strcat(szCmd, szCmd1);
	}

	//Save DNS server(only used for eth0)
	if(strcmp(szEthId, ETHNAME_FIRST) == 0)
	{
		if(pNetInfo->ulIpV4_DNSServer != 0)
		{
			memset(szCmd1, 0, 128);
			//Jimmy note 20210817: we define only ONE eth can have DNS server
			snprintf(szCmd1, sizeof(szCmd1), "echo \"echo 'nameserver %s' > /etc/resolv.conf\" >> %s",
				inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_DNSServer))), pFile);
			strcat(szCmd, szCmd1);
		}
		else if(!bDHCPOn)
		{
			memset(szCmd1, 0, 128);
			//Jimmy note 20210817: we should remove the user-defined DNS when set is 0
			snprintf(szCmd1, sizeof(szCmd1), "echo \"cat /dev/null > /etc/resolv.conf\" >> %s", pFile);
			strcat(szCmd, szCmd1);
		}		
	}


	TRACEX("Now save netChange, script[%s] -> [%s]\n",szCmd, pFile);
#ifdef _DEBUG
	system(szCmd);
#else
	_SYSTEM(szCmd);
#endif
}

static int SetNetInfo(SYSTEM_SAMPLER_DATA* pRough, char* szEthId, NET_INFO* pNetInfo, NET_INFO* pSRCNetInfo)
{
	char szCmd[128];

	//Set IP
	if(pSRCNetInfo->ulIpV4_IP != pNetInfo->ulIpV4_IP)
	{
		snprintf(szCmd, sizeof(szCmd), "/sbin/ifconfig %s %s",
			szEthId, inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_IP))));
#ifdef _DEBUG
		system(szCmd);
#else
		_SYSTEM(szCmd);
#endif
	}

	//Set netMask
	if(pSRCNetInfo->ulIpV4_Mask != pNetInfo->ulIpV4_Mask)
	{
		snprintf(szCmd, sizeof(szCmd), "/sbin/ifconfig %s netmask %s",
			szEthId, inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_Mask))));
#ifdef _DEBUG
		system(szCmd);
#else
		_SYSTEM(szCmd);
#endif
	}
	
	//Set GateWay
	// if(pSRCNetInfo->ulIpV4_Gateway != pNetInfo->ulIpV4_Gateway)
	// {
		if(pSRCNetInfo->ulIpV4_Gateway != 0)
		{
			snprintf(szCmd, sizeof(szCmd), "/sbin/route del default gw %s", 
				inet_ntoa(*((struct in_addr*)(&pSRCNetInfo->ulIpV4_Gateway))));
	#ifdef _DEBUG
			system(szCmd);
	#else
			_SYSTEM(szCmd);
	#endif
		}

		if(pNetInfo->ulIpV4_Gateway != 0)
		{
	// 		//added by Jimmy 20190404 for solving the static IP can not do DNS issue
	// 		snprintf(szCmd, sizeof(szCmd), "echo \"nameserver %s\" > /etc/resolv.conf",
	// 			inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_Gateway))));
	// #ifdef _DEBUG
	// 		system(szCmd);
	// #else
	// 		_SYSTEM(szCmd);
	// #endif

			snprintf(szCmd, sizeof(szCmd), "/sbin/route add default gw %s %s",
				inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_Gateway))), szEthId);
	#ifdef _DEBUG
			system(szCmd);
	#else
			_SYSTEM(szCmd);
	#endif
		}
	// }//xsy

	//set DNS server(only use for ETH0)
	if(strcmp(szEthId, ETHNAME_FIRST) != 0)
	{
		return 0;
	}
	if((pSRCNetInfo->ulIpV4_DNSServer != pNetInfo->ulIpV4_DNSServer) ||
		(pNetInfo->ulIpV4_DNSServer == 0))
	{
		pSRCNetInfo->ulIpV4_DNSServer = pNetInfo->ulIpV4_DNSServer;
		if(pNetInfo->ulIpV4_DNSServer == 0)
		{
			snprintf(szCmd, sizeof(szCmd), "cat /dev/null > /etc/resolv.conf");
		}
		else
		{
			snprintf(szCmd, sizeof(szCmd), "echo \"nameserver %s\" > /etc/resolv.conf",
						inet_ntoa(*((struct in_addr*)(&pNetInfo->ulIpV4_DNSServer))));
		}		
#ifdef _DEBUG
		system(szCmd);
#else
		_SYSTEM(szCmd);
#endif
	}

	return 0;
}



static void SetDHCP(char* szEthId, BOOL bOn)
{
	char szSysCmd[64] = {0};
	//Jimmy 20190323: this is for killing the previous dhclient background process
	snprintf(szSysCmd, 64, "ps | grep \"dhclient %s\" | sed 's/root.*$//g' | xargs kill -9", szEthId);
#ifdef _DEBUG
	system(szSysCmd);
#else
	_SYSTEM(szSysCmd);
#endif

	if(bOn)
	{
		snprintf(szSysCmd, 64, "dhclient %s -nw&", szEthId);
#ifdef _DEBUG
		system(szSysCmd);
#else
		_SYSTEM(szSysCmd);
#endif
	}
}

static BOOL GetDHCP_Status(char* szEthId)
{
	char szSysCmd[64] = {0};
	snprintf(szSysCmd, 64, "ps | grep -c \"dhclient %s\"", szEthId);
// #ifdef _DEBUG
// 	FILE   *stream;
// 	char buf[64] = {0};
// 	stream = popen(szSysCmd, "r");
// 	if(stream == NULL)
// 	{
// 		return -1;
// 	}
// 	fread(&buf, 64, 1,stream);
// 	pclose( stream );

// 	int nRtn = atoi(buf);
// 	TRACEX("Get DHCP Status for [%s],cmdRet = %d, buf = [%s]\n",szSysCmd, nRtn, buf);
// 	return (nRtn >= 3); //do not know why 3, supposed to be 2???
// #else
	int nCmdRet = _SYSTEM_EX(szSysCmd);
	return (nCmdRet >= 3); //do not know why 3, supposed to be 2???
//#endif
}

static int Event_Handler(HANDLE	hModuleSeif,
	_EVENT_TYPE	emEvType,
	_EVENT_MSG	emMsgType,
	UINT			nEvDataSize,
	void*			pEvData,
	BOOL			bBlockMode,
	int			nMaxBlockMSec)
{
	SAMPLER* pSampler = (SAMPLER*)hModuleSeif;
	SYSTEM_SAMPLER_DATA* pRough = (SYSTEM_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR *pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;

	if(EVENT_TYPE_FIRMWARE_UPDATE == emEvType) //restore firmware update event
	{
		if((nEvDataSize != sizeof(EVENT_FIRMWAREUPDATE_STATUS)) || (emMsgType != EVENT_MSG_RESTORE))
		{
			return EVENT_HANDED_FAILED;
		}

		Mutex_Lock(pRough->hSamplerLock, WAIT_INFINITE);

		TRACE("-----------now received restore event for firmware update-----------\n");
		EVENT_FIRMWAREUPDATE_STATUS firmStat;
		memcpy(&firmStat, pEvData, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
		EVENT_FIRMWAREUPDATE_STATUS* pEvFirmUpdt = &firmStat;
		
		TRACE("\n--------------FIRMWARE UPDATE SUCCESS, old ver[%s], url[%s], status = %d----------------\n",
				pEvFirmUpdt->szOldFirmwareVer, pEvFirmUpdt->stUpdateCmd.szURL, 
				pEvFirmUpdt->emUpdateStatus);

		pDPR->Get(hModuleSeif, GETTYPE_APPVER,
			0, 0, pEvFirmUpdt->szNewFirmwareVer, MAX_LEN_MU2SWVER);

		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
			"%s UPDATED app version: [%s -> %s]!", 
			(pEvFirmUpdt->emUpdateStatus == FIRMUPDATE_Installing) ? "Successfully" : "Failed", 
			pEvFirmUpdt->szOldFirmwareVer, pEvFirmUpdt->szNewFirmwareVer);

		pEvFirmUpdt->emUpdateStatus = (pEvFirmUpdt->emUpdateStatus == FIRMUPDATE_Installing) ? FIRMUPDATE_Installed : FIRMUPDATE_InstallFailed;
		SendMsg(hModuleSeif, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
			pEvFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));

		Mutex_Unlock(pRough->hSamplerLock);
	}
	else if(EVENT_TYPE_CFG_CHANGED == emEvType)
	{
		//printf("EVENT_TYPE_CFG_CHANGED == emEvType nEvDataSize=[%d]\n", nEvDataSize);
		if(nEvDataSize != sizeof(EVENT_PARAM_CHG))
		{
			return EVENT_HANDED_FAILED;
		}
		EVENT_PARAM_CHG *pCfgChange = (EVENT_PARAM_CHG*)pEvData;

		TRACE("nId=[%d] nSubId=[%d]\n", pCfgChange->parmPTR.nId, pCfgChange->parmPTR.nSubId);
	 	if((pCfgChange->parmPTR.nId == 1) && (pCfgChange->parmPTR.nSubId == 17))
		{
			// printf("BootInfo config Changed, Do Update!\n");
 			varV.blobValue = (BYTE *)&pRough->blobOCPP_BootInfo;
			//copy setting to current data
			ERRCDDE_DPR err = pDPR->Get(hModuleSeif, GETTYPE_ST_VAR,
				1, 17, &varV, sizeof(A_BLOB_CHARGER_BOOTINFO));
			if(err != ERR_DPR_OK)
			{
				return EVENT_HANDED_FAILED;
			}
			TRACE("!!! BootInfo Changed success !!!\n");
		}
	}

	return EVENT_HANDED_OK;
}

static BOOL Judge_IfNetportExisted(char* szEthName)
{
	if(strlen(szEthName) < 2)
	{
		return FALSE;
	}
	char szSysCmd[64] = {0};
	snprintf(szSysCmd, 64, "/sbin/ifconfig -a | grep -c %s", szEthName);
	int nRet = _SYSTEM_EX(szSysCmd);

	TRACEX("--Exist status of [%s] is %d---\n",szEthName, nRet);
	return (nRet > 0);
}

static BOOL Judge_IfNetwork_Connected(char* szEthName)
{
	if(strlen(szEthName) < 2)
	{
		return FALSE;
	}
	char szSysCmd[64] = {0};
	snprintf(szSysCmd, 64, "/sbin/ifconfig %s | grep -c RUNNING", szEthName);
	int nRet = _SYSTEM_EX(szSysCmd);

	TRACEX("--Now the running status of [%s] is %d---\n",szEthName, nRet);
	return (nRet > 0);
}

static int Refresh_NetworkInfo(SAMPLER* pSampler, BOOL bIinitLoad)
{
	//DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	SYSTEM_SAMPLER_DATA* pRough = (SYSTEM_SAMPLER_DATA*)pSampler->pDataHeap;
	//VAR_VALUE varV;
	//if(bIinitLoad)
	//{
	//	varV.blobValue = (BYTE *)&pRough->currNetWorkInfo;
	//	//copy setting to current data
	//	ERRCDDE_DPR err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
	//		1, 3, &varV, sizeof(A_BLOB_NETWORKINFO));
	//	if(err != ERR_DPR_OK)
	//	{
	//		return err;
	//	}
	//	TRACE("\n----------The initial DHCP status = %d------\n", pRough->currNetWorkInfo.bDHCP_On_eth0);
	//	SetDHCP(pRough->currNetWorkInfo.bDHCP_On_eth0); //restore DHCP setting
	//}

	if(bIinitLoad)
	{
		pRough->currNetWorkInfo.netInfo_eht0.emNetPort_Status = Judge_IfNetportExisted(ETHNAME_FIRST);
		pRough->currNetWorkInfo.netInfo_eht1.emNetPort_Status = Judge_IfNetportExisted(ETHNAME_SECOND);
		//pRough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = Judge_IfNetportExisted("usb0");
	}

	//get network info
	time_t tmNow = _NOW_;
	//if(pRough->currNetWorkInfo.netInfo_eht0.emNetPort_Status)
	{
		GetNetInfo(ETHNAME_FIRST, &pRough->currNetWorkInfo.netInfo_eht0);
		pRough->currNetWorkInfo.bDHCP_On_eth0 = GetDHCP_Status(ETHNAME_FIRST);
		pRough->currNetWorkInfo.netInfo_eht0.emNetPort_Status = Judge_IfNetwork_Connected(ETHNAME_FIRST) ? NETPORT_STATUS_CONNECTED : NETPORT_STATUS_DISCONN;
		//Jimmy note: in case DHCP failure, we do this periodly
		if(pRough->currNetWorkInfo.bDHCP_On_eth0 && 
			(pRough->currNetWorkInfo.netInfo_eht0.emNetPort_Status == NETPORT_STATUS_CONNECTED) &&
			(pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_IP == 0) &&
			(ABS(tmNow - pRough->tm_LastSetDHCPOn_Eth0) >= 30))
		{
			pRough->tm_LastSetDHCPOn_Eth0 = tmNow;
			TRACEX("-----ETH0 DHCP failed, so reset it once again----\n");
			SetDHCP(ETHNAME_FIRST, TRUE);
		}
		//added by Jimmy 20210928 for default DNS when use router without DNS server
		if(bIinitLoad && (!pRough->currNetWorkInfo.bDHCP_On_eth0) && (pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_Gateway != 0) 
			&& (pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer == 0))
		{
			//do not need save it into init_eth0.sh file as each time will init here
			pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer = pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_Gateway;
			char szSysCmd[64] = {0};
			snprintf(szSysCmd, sizeof(szSysCmd), "echo \"search lan\n nameserver %s\" > /etc/resolv.conf",
					inet_ntoa(*((struct in_addr*)(&pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer))));
			_SYSTEM_EX(szSysCmd);
			SaveNetInfo_IntoScriptFile(ETHNAME_FIRST, "/home/app_script/init_"ETHNAME_FIRST".sh", pRough->currNetWorkInfo.bDHCP_On_eth0, 
				&pRough->currNetWorkInfo.netInfo_eht0);
		}
		//added by Jimmy 20210107
		if((pRough->currNetWorkInfo.bDHCP_On_eth0) && (pRough->currNetWorkInfo.netInfo_eht0.emNetPort_Status == NETPORT_STATUS_CONNECTED) &&
			(pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_Gateway != 0) && (pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_IP != 0) && 
			(pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer == 0))
		{
			pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer = pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_Gateway;
			char szSysCmd[64] = {0};
			snprintf(szSysCmd, sizeof(szSysCmd), "echo \"search lan\n nameserver %s\" > /etc/resolv.conf",
					inet_ntoa(*((struct in_addr*)(&pRough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer))));
			_SYSTEM_EX(szSysCmd);
			SaveNetInfo_IntoScriptFile(ETHNAME_FIRST, "/home/app_script/init_"ETHNAME_FIRST".sh", pRough->currNetWorkInfo.bDHCP_On_eth0, 
				&pRough->currNetWorkInfo.netInfo_eht0);
		}
	}

	if(pRough->currNetWorkInfo.netInfo_eht1.emNetPort_Status)
	{
		GetNetInfo(ETHNAME_SECOND, &pRough->currNetWorkInfo.netInfo_eht1);
		pRough->currNetWorkInfo.bDHCP_On_eth1 = GetDHCP_Status(ETHNAME_SECOND);
		pRough->currNetWorkInfo.netInfo_eht1.emNetPort_Status = Judge_IfNetwork_Connected(ETHNAME_SECOND) ? NETPORT_STATUS_CONNECTED : NETPORT_STATUS_DISCONN;
		//Jimmy note: in case DHCP failure, we do this periodly
		if(pRough->currNetWorkInfo.bDHCP_On_eth1 && 
			(pRough->currNetWorkInfo.netInfo_eht1.emNetPort_Status == NETPORT_STATUS_CONNECTED) &&
			(pRough->currNetWorkInfo.netInfo_eht1.ulIpV4_IP == 0) &&
			(ABS(tmNow - pRough->tm_LastSetDHCPOn_Eth1) >= 30))
		{
			pRough->tm_LastSetDHCPOn_Eth1 = tmNow;
			TRACEX("-----ETH1 DHCP failed, so reset it once again----\n");
			SetDHCP(ETHNAME_SECOND, TRUE);
		}
	}

	//if(pRough->currNetWorkInfo.netInfo_usb0.emNetPort_Status)
	//{
	//	GetNetInfo("usb0", &pRough->currNetWorkInfo.netInfo_usb0);
	//	pRough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = Judge_IfNetwork_Connected("usb0") ? NETPORT_STATUS_CONNECTED : NETPORT_STATUS_DISCONN;
	//}

	//now set raw data to setting param
	Push_BlobValue(SETTYPE_RAW_ST, (HANDLE)pSampler, 
		1, 3, (void*)&pRough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);

	return ERR_DPR_OK;
}

static void SetDateTime(time_t* pTimeRet)
{
	char szTime[64];
	char szCommand[128];

	char szTime_Old[64];
	TimeToString(_NOW_, TIME_CHN_FMT, 
		szTime_Old, sizeof(szTime_Old));	

	TimeToString(*pTimeRet, TIME_CHN_FMT, 
		szTime, sizeof(szTime));

	LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
			"Adj Sys Time[%s]->[%s]!", szTime_Old, szTime);

	sprintf(szCommand, "date -s \"%s\"", szTime);
	TRACE("\n***UpdateTime: [%s]\n", szCommand);
#ifdef _DEBUG
	system(szCommand);
	system("hwclock -w;/etc/init.d/save-rtc.sh;sync"); //added by Jimmy 20200910 for RTC saving problem
#else
	_SYSTEM(szCommand);
	_SYSTEM("hwclock -w;/etc/init.d/save-rtc.sh;sync &"); //added by Jimmy 20200910 for RTC saving problem
#endif
	stime(pTimeRet);		//2.Update OS time
}


void* _thread_WLAN_AvailbleCheck(void* pArgs)
{
	SAMPLER* pSampler = (SAMPLER *)pArgs;
	SYSTEM_SAMPLER_DATA* pRough = (SYSTEM_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	CURL *curl;
	CURLcode res;
	int nErr = -99;
	//Sleep 30 sec first
	//Sleep_Ex(30000);
	time_t tmBefore = _NOW_;
	//curl_global_init(CURL_GLOBAL_ALL);//Jimmy 20210201: put it out in [Sampler_loader]
	TRACEX("\n-----The checking WLAN thread was initilized------\n");
	//added by Jimmy 20210201 for share DNS cache
	int nDisconnCounter = 0;
	//added by Jimmy 20210928: if history connOK with WLAN, then skip detection when OCPP is online
	static BOOL s_bIsEverWWAN_Available = FALSE;
	while (!pSampler->bExit)
	{
		varV.emValue = 0;
		pDPR->Get((HANDLE)pSampler, GETTYPE_SP_VAR, 1, 5, &varV, 4);
		if((varV.emValue == 0) || (pRough->nPingNetwork == 1))
		{
			Sleep_Ex(10000);
			if(s_bIsEverWWAN_Available)
			{
				TRACEX("\n Server is connected, so quit the checking\n");
				pRough->bIsWWLAN_Available = TRUE;				
				continue;					
			}	
		}
		
		//Sleep 15 sec first
		if(ABS(tmBefore - _NOW_) < 15)
		{
			Sleep_Ex(2000);
			continue;
		}
		//WLD01206 之前的HTTPS连接太消耗流量，改为ping网络
		const char *host;
        if(pRough->nBaiduOrGoogle == 0)
		{
            host = "www.baidu.com";
		    //TRACEX("Using baidu to check network\n");
        } 
		else 
		{
            host = "www.google.com";
			//TRACEX("Using google to check network\n");
        }

        char cmd[256] = {0};
		snprintf(cmd, sizeof(cmd) - 1, "ping -c 1 -W 10 %s > /dev/null 2>&1", host);

        int ping_ret = system(cmd);
        if(ping_ret == 0) 
		{
            nDisconnCounter = 0;
            nErr = 0;
        } 
		else 
		{
            nDisconnCounter++;
            if(nDisconnCounter >= 3)
			{
                nDisconnCounter = 3;
                nErr = -1;
            }               
        }   

		//added by Jimmy: if discon, we check quicker
		if(nErr == -99)
		{
			Sleep_Ex(5000);
			continue;
		}

		pRough->bIsWWLAN_Available = (nErr == 0) ? TRUE : FALSE;
		s_bIsEverWWAN_Available = pRough->bIsWWLAN_Available;
		//wait 10 sec to do another check
		Sleep_Ex(10000);
	}
	
	return NULL;
}

static BOOL Init_blobFeeRateCfg(SYSTEM_SAMPLER_DATA *pSystem_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSystem_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	int 					n = 0;
	VAR_VALUE 				varV;
	BOOL 					bNeedUpdtFeeRate = FALSE;
	//now use the default plan
	A_BLOB_CHARGE_FEESET 			blobFeeRateSet;
	ZERO_POBJS(&blobFeeRateSet, 	1);

 	varV.blobValue = (BYTE *)&pSystem_Rough->blobFeeRateCfg;
	pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR, 1, 12, &varV, sizeof(A_BLOB_CHARGE_FEESET));

	TRACE("nTotal_FeeRate_Sectors=%d\n", pSystem_Rough->blobFeeRateCfg.nTotal_FeeRate_Sectors);
 	//judge if the plan need to be initialized
	if(pSystem_Rough->blobFeeRateCfg.nTotal_FeeRate_Sectors < 1)
	{
		//default plan has 2 sectors:
		blobFeeRateSet.nTotal_FeeRate_Sectors = 9;

		//1: 00:00—07:00
		blobFeeRateSet.feeRateSectors[0].tmStart = StringToTime("2000-01-01 00:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[0].nInterval = 3600 * 7;
		blobFeeRateSet.feeRateSectors[0].emFeeRate = FEERATE_VALLEY;
		blobFeeRateSet.feeRateSectors[0].fPrice = 0.0;

		//2: 7:00—9:00 平
		blobFeeRateSet.feeRateSectors[1].tmStart = StringToTime("2000-01-01 07:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[1].nInterval = 3600 * 2;
 		blobFeeRateSet.feeRateSectors[1].emFeeRate = FEERATE_FLAT;
 		blobFeeRateSet.feeRateSectors[1].fPrice = 0.0;

		//3:9:00—11:30 峰
		blobFeeRateSet.feeRateSectors[2].tmStart = StringToTime("2000-01-01 09:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[2].nInterval = 3600 * 2.5;
 		blobFeeRateSet.feeRateSectors[2].emFeeRate = FEERATE_PEAK;
 		blobFeeRateSet.feeRateSectors[2].fPrice = 0.0;

		//4. 11:30—14:00 平
 		blobFeeRateSet.feeRateSectors[3].tmStart = StringToTime("2000-01-01 11:30:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[3].nInterval = 3600 * 2.5;
 		blobFeeRateSet.feeRateSectors[3].emFeeRate = FEERATE_FLAT;
 		blobFeeRateSet.feeRateSectors[3].fPrice = 0.0;


		//5:14:00—16:30 峰
		blobFeeRateSet.feeRateSectors[4].tmStart = StringToTime("2000-01-01 14:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[4].nInterval = 3600 * 2.5;
 		blobFeeRateSet.feeRateSectors[4].emFeeRate = FEERATE_PEAK;
 		blobFeeRateSet.feeRateSectors[4].fPrice = 0.0;

		
		//6. 16:30—19:00 平
 		blobFeeRateSet.feeRateSectors[5].tmStart = StringToTime("2000-01-01 16:30:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[5].nInterval = 3600 * 2.5;
 		blobFeeRateSet.feeRateSectors[5].emFeeRate = FEERATE_FLAT;
 		blobFeeRateSet.feeRateSectors[5].fPrice = 0.0;

		
		//7:19:00—21:00 峰
 		blobFeeRateSet.feeRateSectors[6].tmStart = StringToTime("2000-01-01 19:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[6].nInterval = 3600 * 2;
 		blobFeeRateSet.feeRateSectors[6].emFeeRate = FEERATE_PEAK;
 		blobFeeRateSet.feeRateSectors[6].fPrice = 0.0;


		//8:21:00-23:00 平
 		blobFeeRateSet.feeRateSectors[7].tmStart = StringToTime("2000-01-01 21:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[7].nInterval = 3600 * 2;
 		blobFeeRateSet.feeRateSectors[7].emFeeRate = FEERATE_FLAT;
 		blobFeeRateSet.feeRateSectors[7].fPrice = 0.0;


		//9: 23:00—00:00 谷
		blobFeeRateSet.feeRateSectors[8].tmStart = StringToTime("2000-01-01 23:00:00", TIME_CHN_FMT);
		blobFeeRateSet.feeRateSectors[8].nInterval = 3600 * 1;
 		blobFeeRateSet.feeRateSectors[8].emFeeRate = FEERATE_VALLEY;
 		blobFeeRateSet.feeRateSectors[8].fPrice = 0.0;

		bNeedUpdtFeeRate = TRUE;
		TRACE("Need Update FeeRate Config!\n");
	}

	if(bNeedUpdtFeeRate)
	{
		Push_BlobValue(SETTYPE_RAW_ST, (HANDLE)pSampler, 1, 12, 
				(void*)&blobFeeRateSet, sizeof(A_BLOB_CHARGE_FEESET), BLOB_TYPE_CHARGE_FEE_SET);
		memcpy(&pSystem_Rough->blobFeeRateCfg, &blobFeeRateSet, sizeof(A_BLOB_CHARGE_FEESET));
	}
	return TRUE;
}

static BOOL Write_Config_to_File(A_BLOB_KEEPALIVEDINFO * pKeepalivedCfg)
{
    FILE *file = fopen(CONFIG_FILE, "w");

    if (file == NULL) 
	{
       return FALSE; 
    }

    fprintf(file, "global_defs {\n");
    fprintf(file, "   router_id %s\n", pKeepalivedCfg->szRouter_Id);
    fprintf(file, "}\n\n");

    fprintf(file, "vrrp_instance VI_1 {\n");
    fprintf(file, "    state %s\n", pKeepalivedCfg->bState == 0 ? "MASTER" : "BACKUP");
    fprintf(file, "    interface %s\n", pKeepalivedCfg->nInterface == 0 ? "eth0" : "eth1");
    fprintf(file, "    virtual_router_id %d\n", pKeepalivedCfg->nVirtual_Router_Id);
    fprintf(file, "    priority %d\n", pKeepalivedCfg->nPriority);
    fprintf(file, "    advert_int %d\n", pKeepalivedCfg->nAdvert_Int);
    fprintf(file, "    authentication {\n");
    fprintf(file, "        auth_type %s\n", pKeepalivedCfg->bAuth_Type == 0 ? "PASS" : "AH");
    fprintf(file, "        auth_pass %s\n", pKeepalivedCfg->szAuth_Pass);
    fprintf(file, "    }\n");
    fprintf(file, "    virtual_ipaddress {\n");
    fprintf(file, "        %s\n", pKeepalivedCfg->szVirtual_Ipaddress);
    fprintf(file, "    }\n");

	fprintf(file, "    notify_master \"/home/app_script/notify.sh MASTER\"\n");
	fprintf(file, "    notify_backup \"/home/app_script/notify.sh BACKUP\"\n");
	fprintf(file, "}\n");

    fclose(file);

	return TRUE; 
}

static BOOL Init_KeepalivedCfg(SYSTEM_SAMPLER_DATA *pSystem_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSystem_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;

	if (access(CONFIG_FILE, F_OK) == 0)	
	{
		printf("File %s exists.\n", CONFIG_FILE);		
		//get keepalivedCFgBlob
		varV.blobValue = (BYTE *)&pSystem_Rough->blobKeepalivedConf;
		ERRCDDE_DPR err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
				1, 86, &varV, sizeof(A_BLOB_KEEPALIVEDINFO));
		
		//run keepalived 
		if (pSystem_Rough->blobKeepalivedConf.bEnable)
		{
			_SYSTEM("keepalived");
		}
    } 
	else 
	{
		printf("File %s not exists.\n", CONFIG_FILE);	
		//now use the default plan
		A_BLOB_KEEPALIVEDINFO 			blobKeepalivedCfgSet;
		ZERO_POBJS(&blobKeepalivedCfgSet, 	1);

		blobKeepalivedCfgSet.bEnable = 0;
		strcpy(blobKeepalivedCfgSet.szRouter_Id, "LVS_MASTER");
		blobKeepalivedCfgSet.bState = 0;
		blobKeepalivedCfgSet.nInterface = 0;
		blobKeepalivedCfgSet.nVirtual_Router_Id = 51;
		blobKeepalivedCfgSet.nPriority = 100;
		blobKeepalivedCfgSet.nAdvert_Int = 1;
		blobKeepalivedCfgSet.bAuth_Type = 0;
		strcpy(blobKeepalivedCfgSet.szAuth_Pass, "1111");
		strcpy(blobKeepalivedCfgSet.szVirtual_Ipaddress, "192.168.1.252");

		Push_BlobValue(SETTYPE_RAW_ST, (HANDLE)pSampler, 1, 86, 
				(void*)&blobKeepalivedCfgSet, sizeof(A_BLOB_KEEPALIVEDINFO), BLOB_TYPE_KEEPALIVEDINFO);
		memcpy(&pSystem_Rough->blobKeepalivedConf, &blobKeepalivedCfgSet, sizeof(A_BLOB_KEEPALIVEDINFO));

		//write keepalived.conf
		if (Write_Config_to_File(&pSystem_Rough->blobKeepalivedConf))
		{
			_SYSTEM("chmod 0644 /etc/keepalived/keepalived.conf");
		}
	}

	return TRUE;
}


DLLExport BOOL Initialize(HANDLE hSamplerSelf)
{
 	SAMPLER* pSampler = (SAMPLER*)hSamplerSelf;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;

	pSampler->pDataHeap = (void*)NEW(SYSTEM_SAMPLER_DATA, 1);
	if(pSampler->pDataHeap == NULL)
	{
		return FALSE;
	}

	SYSTEM_SAMPLER_DATA *pSystem_Rough = (SYSTEM_SAMPLER_DATA*)pSampler->pDataHeap;
	ZERO_POBJS(pSystem_Rough, 1);
	pSystem_Rough->hSampler = hSamplerSelf;
	pSystem_Rough->nSamplerId = pSampler->info.nSamplerID;
	pSystem_Rough->hSamplerLock = pSampler->hSamplerLock;

	//Re-Initial all net port in case they are not initilized OK
	_SYSTEM("killall -9 dhclient");
	_SYSTEM("/home/app_script/init_"ETHNAME_FIRST".sh");
	_SYSTEM("/home/app_script/init_"ETHNAME_SECOND".sh");
	pSystem_Rough->tm_LastSetDHCPOn_Eth0 = _NOW_;
	pSystem_Rough->tm_LastSetDHCPOn_Eth1 = _NOW_;
	//printf("----heeeinitia eht000111----\n");

	//init 4G modem
	//_SYSTEM("/app/modem_reset sim0");
	//Jimmy 20201214 removed it because we disable the LTE default
	//_SYSTEM("/app/modem_reset sim0 &"); //now reset modem
	// pSystem_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
	// pSystem_Rough->tmLastQueryInstallStatus = _NOW_ + 5;

	//Initialize network info. 
	//now get the saved APN
	varV.blobValue = (BYTE *)&pSystem_Rough->currNetWorkInfo;
	ERRCDDE_DPR err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
			1, 3, &varV, sizeof(A_BLOB_NETWORKINFO));
	// if(err != ERR_DPR_OK)
	// {
	// 	return FALSE;
	// }
	if(Refresh_NetworkInfo(pSampler, TRUE) != ERR_DPR_OK)
	{
		return FALSE;
	}

	EVHAND_TAGS tags[] = 
	{
		{EVENT_TYPE_FIRMWARE_UPDATE,	EVENT_MSG_RESTORE},
		{EVENT_TYPE_CFG_CHANGED,		EVENT_MSG_BEGIN},
		{EVENT_TYPE_REFRESH_ALARM,		EVENT_MSG_BEGIN},

	};
	int n;
	for(n = 0; n < ITEM_OF(tags); n++)
	{
		pDPR ->Register_EvHandler("Sys_EvHandler",
			hSamplerSelf,
			Event_Handler,
			tags[n].nEvTag,
			tags[n].nMsgTag,
			FALSE);
	}

	Init_blobFeeRateCfg(pSystem_Rough);

	A_BLOB_MANUALCTRL_RECT_CMD blobManualCtrlRect;
	ZERO_POBJS(&blobManualCtrlRect, 1);
	varV.blobValue = (BYTE *)&blobManualCtrlRect;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_CL_VAR,
		200, 214, &varV, sizeof(A_BLOB_MANUALCTRL_RECT_CMD));
	if(err != ERR_DPR_OK)
	{
		return FALSE;
	}

	TRACE("emState=%d nGrp=%d\n",blobManualCtrlRect.emState, blobManualCtrlRect.nGrp);
	if((blobManualCtrlRect.emState < 0) || (blobManualCtrlRect.nGrp < 0))
	{
		blobManualCtrlRect.emState = 0;	
		blobManualCtrlRect.nGrp = 0;		
		blobManualCtrlRect.nStopSOC = 100;		
		
		Push_BlobValue(SETTYPE_INTERN_CTRL, (HANDLE)pSampler, 200, 214, 
			(void*)&blobManualCtrlRect, sizeof(A_BLOB_MANUALCTRL_RECT_CMD), BLOB_TYPE_MANUALCTRL_RECT);
	}
	//Now read ES_iOCPP settings and set default
	A_BLOB_ES_BACKEND_CFG	blobBackendCfg;
	ZERO_POBJS(&blobBackendCfg, 1);
	varV.blobValue = (BYTE *)&blobBackendCfg;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 26, &varV, sizeof(A_BLOB_ES_BACKEND_CFG));
	if(err != ERR_DPR_OK)
	{
		TRACE("A_BLOB_ES_BACKEND_CFG Get Failed\n");
		return FALSE;
	}

	TRACED("22222 .fMin_ACLoadPwr_Start_Dischg=%f nCmd_101_Interval=%d blobBackendCfg.isACDC_Grp_Exist:%d %d %d %d "
		"fSysACInput_PwrLmt:%.1f %.1f %.1f",
		blobBackendCfg.fMin_ACLoadPwr_Start_Dischg, blobBackendCfg.nCmd_101_Interval, blobBackendCfg.isACDC_Grp_Exist[0], blobBackendCfg.isACDC_Grp_Exist[1], blobBackendCfg.isACDC_Grp_Exist[2], blobBackendCfg.isACDC_Grp_Exist[3],
		blobBackendCfg.fSysACInput_PwrLmt, blobBackendCfg.fEnter_UrgetDischrgPwr, blobBackendCfg.fExit_UrgetDischrgPwr);
	// old if((blobBackendCfg.nCmd_101_Interval == 0)|| 
	//   ((blobBackendCfg.isACDC_Grp_Exist[0] == FALSE)&&(blobBackendCfg.isACDC_Grp_Exist[1] == FALSE))||
	//   (blobBackendCfg.fEnter_UrgetDischrgPwr <= 0.1)||
	//   (blobBackendCfg.fExit_UrgetDischrgPwr <= 0.1))

	// LJH250226 fix:SEMS isACDC_Grp_Exist==FALSE
	if (blobBackendCfg.fSysACInput_PwrLmt<0.1f && blobBackendCfg.fOutput2AC_PwrRate<0.1f)
	{
		//TRACE("######## Update BlobBackendCfg#############\n");
		TRACE("333 now change defaule value of blobBackendCfg\n");
		ZERO_POBJS(&blobBackendCfg, 1);

		blobBackendCfg.fSysACInput_PwrLmt = 2.0f;

		blobBackendCfg.fMax_Output2AC_PwrLmt = 0.1f; // abort

		blobBackendCfg.fOutput2AC_PwrRate = 0.9f; // old 0.8f; LJH260106 modified by zyf建议

		blobBackendCfg.fMin_Output2AC_PwrLmt = 0.0f;

		blobBackendCfg.fPreProt4RefluxAndOverLoad = 20.0f; // old 50.0f; LJH260106 modified by zyf建议
		
		blobBackendCfg.fUrgetAlterDischrgCurr = 500.0f;

		blobBackendCfg.fAdjustUrgetModeCurr = 50.0f;

		blobBackendCfg.fEnter_UrgetDischrgPwr = 99999999.0f;//KW
		
		blobBackendCfg.fExit_UrgetDischrgPwr = 40.0f; 

		blobBackendCfg.fACLoadPwrProtcPoint = 99999999.0f;

		blobBackendCfg.fMin_ACLoadPwr_Start_Dischg = 0.1f;

		blobBackendCfg.fQmaxOutput2ACLmt =    0.0f;
		blobBackendCfg.fPfactory_SettDemand = 0.0f;
		// LJH240705 AitiOverLoad HVMeter
		blobBackendCfg.fSysACInput_PwrLmtHV = 0.0f; //KW 为了兼容其他项目默认0不防高压侧超需量（目前只有众树的中顺、丰邦智造园需要此功能）
		// LJH240911 AvgPwrFirst
		blobBackendCfg.emPowCtrlMode =         POW_CTRL_MODE_EFFICIENCY;
		blobBackendCfg.fPwrLmtMin_AvgPwrMode = 0.0f;
		// LJH241007 AntiPhase
		blobBackendCfg.fMaxChrgCurLmt_Phase =   0.0f;
		blobBackendCfg.fPreProt4Reflux_Phase = 0.0f;
		// Jeffery241204 Dynamic Grid Input Limit
		blobBackendCfg.emDynamicGridInputLimit_Mode = 0;
		blobBackendCfg.fFixed_GridInput_Limit = 0.0;
		// Jeffery250305 PCS Hot Stand-by
		blobBackendCfg.emPCS_HotStandby_Mode = 0;

		blobBackendCfg.backend_reserved = 0;

		blobBackendCfg.bEnableQuickResponseChannel = 0;

		// blobBackendCfg.nFilterPointNum = 20;
		ZERO_POBJS(blobBackendCfg.szResv, sizeof(blobBackendCfg.szResv));

		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			blobBackendCfg.isACDC_Grp_Exist[n] = TRUE;
			blobBackendCfg.isMPPT_Grp_Exist[n] = FALSE;
			blobBackendCfg.isDCDC_Grp_Exist[n] = FALSE;
		}
		
		blobBackendCfg.nCmd_101_Interval = 10; //edit by kuangyue 20250313
		blobBackendCfg.nCmd_102_Interval = 60; 
		blobBackendCfg.nCmd_103_Interval = 60; 
		blobBackendCfg.nCmd_104_Interval = 600; 
		blobBackendCfg.nCmd_105_Interval = 25; 
		blobBackendCfg.nCmd_106_Interval = 600; 
		blobBackendCfg.nCmd_107_Interval = 35;
		blobBackendCfg.nCmd_108_Interval = 600; 
		blobBackendCfg.nCmd_109_Interval = 600; 
		blobBackendCfg.nCmd_110_Interval = 0;  //Default Not Send To Cloud PlatForm. If Requst Suggest 600S.
		blobBackendCfg.nCmd_112_Interval = 0; //默认不上送
		blobBackendCfg.nCmd_200_Interval = 0; //默认不上送
		blobBackendCfg.nCmd_202_Interval = 180; 
		blobBackendCfg.nCmd_204_Interval = 300;
		blobBackendCfg.nCmd_155_Interval = 0; //默认不上送
		blobBackendCfg.nCmd_157_Interval = 0; //默认不上送
		blobBackendCfg.nCmd_114_Interval = 0; //默认不上送
		blobBackendCfg.nCmd_115_Interval = 60;
					
		Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 26, 
			(void*)&blobBackendCfg, sizeof(A_BLOB_ES_BACKEND_CFG), BLOB_TYPE_ES_BACKEND_CFG);
	}
	// LJH250703 分开充放电保护/跟随比(充电)
	if (blobBackendCfg.PowerRate_Chrg < 0.1f && blobBackendCfg.fPreProt_Chrg < 0.1f)
	{
		blobBackendCfg.PowerRate_Chrg = blobBackendCfg.fOutput2AC_PwrRate;
		blobBackendCfg.fPreProt_Chrg =  blobBackendCfg.fPreProt4RefluxAndOverLoad;
		TRACEW("sampler_system::Initialize PowerRate_Chrg:%.1f fPreProt_Chrg:%.1f", blobBackendCfg.PowerRate_Chrg, blobBackendCfg.fPreProt_Chrg);
		Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 26, 
			(void*)&blobBackendCfg, sizeof(A_BLOB_ES_BACKEND_CFG), BLOB_TYPE_ES_BACKEND_CFG);
	}
	//do not forget to sync the settings to the existed params
	//Sync_ESiOCPPConfig(pSystem_Rough, &stESiOCPPSettings);

	varV.blobValue = (BYTE *)&pSystem_Rough->blobOCPP_BootInfo;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 17, &varV, sizeof(A_BLOB_CHARGER_BOOTINFO));
	if(err != ERR_DPR_OK)
	{
		return FALSE;
	}
 
	if((strlen(pSystem_Rough->blobOCPP_BootInfo.szFirmwareVer) < 1) &&
		(strlen(pSystem_Rough->blobOCPP_BootInfo.szChargePointModel) < 1))
	{
		A_BLOB_CHARGER_BOOTINFO bootIn;
		ZERO_POBJS(&bootIn, 1);
		sprintf(bootIn.szChargePointModel, "Test_ChargeModel");
		sprintf(bootIn.szChargePointSerualNuml, "Test_123456"); //this is always required
		sprintf(bootIn.szChargePointVendor, "Infypower");
		//app version
		bootIn.emBootInfoOptions.firmwareVer = 1;
		bootIn.emBootInfoOptions.chargerPointSN = 1;//this is always required
		pDPR->Get((HANDLE)pSampler, GETTYPE_APPVER,
			0, 0, bootIn.szFirmwareVer, MAX_LEN_MU2SWVER);
		memcpy(pSystem_Rough->blobOCPP_BootInfo.szFirmwareVer, bootIn.szFirmwareVer, 16);
		Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 17, 
			(void*)&bootIn, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
	}
	else
	{
		char szNewFirmwareVer[MAX_LEN_MU2SWVER] = {0};
		pDPR->Get((HANDLE)pSampler, GETTYPE_APPVER,
			0, 0, szNewFirmwareVer, MAX_LEN_MU2SWVER);
		//compare app version and judge firmware update
		if(strcmp(pSystem_Rough->blobOCPP_BootInfo.szFirmwareVer, szNewFirmwareVer) != 0)
		{
			EVENT_UI_NOTICE evNote;
			evNote.emNotoceType = UI_NOTICETYPE_FIRMWARE_UPDATE;
			SendMsg(hSamplerSelf, EVENT_TYPE_UI_NOTICE, EVENT_MSG_BEGIN,
				&evNote, sizeof(EVENT_UI_NOTICE));
			TRACE("\n-----Found APP update Event, send out notice now...\n");
			memcpy(pSystem_Rough->blobOCPP_BootInfo.szFirmwareVer, szNewFirmwareVer, MAX_LEN_MU2SWVER);
			Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 17, 
				(void*)&pSystem_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
		}
	}

	
	//use ftp/http port
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_COMMPORT,
		8, NULL, &pSystem_Rough->commHttpFtp, sizeof(COMMPORT));
	if(err != ERR_DPR_OK)
	{
		return FALSE;
	}
	TRACE("\n Now successfully get HTTP_FTP commport: [%s, %s]\n",
		pSystem_Rough->commHttpFtp.info.szPortName, pSystem_Rough->commHttpFtp.info.szPortDriver);

	pSystem_Rough->bNeedRefreshNetInfo = FALSE;

	//Initial system date time
	varV.ulValue = (UINT)_NOW_;
	Push_VarValue(SETTYPE_RAW_ST, (HANDLE)pSampler, 1, 1, varV);

	pSystem_Rough->tmPowerOn = _NOW_;

	//increase restart times
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 13, &varV, 4);
	if(err != ERR_DPR_OK)
	{
		//return FALSE;
		varV.ulValue = 0;
	}
	//else
	{
		TRACEX("System has restarted %d times\n", varV.ulValue);
		varV.ulValue += 1;
		Push_VarValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 13, varV);
	}

	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 89, &varV, 4);
	pSystem_Rough->emControlMode = (COMTROL_MODE)varV.emValue;
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 87, &varV, 4);
	pSystem_Rough->emCustomType = (CUSTOM_TYPE)varV.emValue;
	if(pSystem_Rough->emCustomType == GH_CUSTOM)
	{
		A_BLOB_EMS_CONFIG_PLAN EmsPlan1, EmsPlan2;
		ZERO_POBJS(&EmsPlan1, 1);
		ZERO_POBJS(&EmsPlan2, 1);
		VAR_VALUE EmsCfg1, EmsCfg2;
		EmsCfg1.blobValue = (BYTE*)&EmsPlan1;
		EmsCfg2.blobValue = (BYTE*)&EmsPlan2;
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 201, &EmsCfg1, sizeof(A_BLOB_EMS_CONFIG_PLAN));
		pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 224, &EmsCfg2, sizeof(A_BLOB_EMS_CONFIG_PLAN));
		if(EmsPlan1.nEMSMgmt_Ver != 2102 && EmsPlan2.nEMSMgmt_Ver != 2102)
		{
			EmsPlan2.nEMSMgmt_Ver = 2102; //都不是港华的版本号时，把备用的EMS管理计划初始化 by edit WLD01206 25/07/02
			Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 224, 
				(void*)&EmsPlan2, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
		}

	}	
	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 34, &varV, 4);
	pSystem_Rough->nBaiduOrGoogle = (UINT)varV.emValue;

	err = pDPR->Get((HANDLE)pSampler, GETTYPE_ST_VAR,
		1, 103, &varV, 4);	
	pSystem_Rough->nPingNetwork = (UINT)varV.emValue;
	// LJH241101 MU3
#ifdef IMMU2
	//Jimmy added 20201212 for LCD login status
	varV.ulValue = 0;
	Push_VarValue(SETTYPE_LOGIC_SP, (HANDLE)pSampler, 1, 16, varV);
#endif
	//now set DO1(Power) on
	//Set_IMMU_DO(IOC_SET_DO1, 1);
	pSystem_Rough->emESSDevType = GetESSDeviceType_Sampler(hSamplerSelf);
	pSystem_Rough->lcCfg = GetLCCfg_Sampler(pSystem_Rough->hSampler); // LJH250508 LCCfg

	pSystem_Rough->bWebOneClivkupgrade = FALSE;
	pSystem_Rough->bUpdFlag = FALSE;
	pSystem_Rough->tm_Updsendtime = _NOW_;
	pSystem_Rough->tm_PCSsendtime = _NOW_;
	pSystem_Rough->tm_Updsendtimefirst = _NOW_;
	pSystem_Rough->tm_Updsendtimelast = _NOW_;
	pSystem_Rough->tm_PCSsendtimefirst = _NOW_;
	pSystem_Rough->tm_PCSsendtimelast = _NOW_;
	pSystem_Rough->tm_RefreshStatus = _NOW_;
	pSystem_Rough->t_LastRefreshAlamStatus = _NOW_;
	pSystem_Rough->nRefreshTimes = 0;
	pSystem_Rough->tLastSaveTime = _NOW_;

	varV.emValue = 0;
	Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 1, 5, varV);

	//init keepalived
	Init_KeepalivedCfg(pSystem_Rough);

	//Jimmy added 20200803: for WLAN avaible check
	pthread_t hThread;
	if (pthread_create(&hThread, 
		NULL, _thread_WLAN_AvailbleCheck,
		(void *)pSampler) != 0)
	{
		TRACEX("\n----Init the check WLAN error, so always consider it connected---\n");
		pSystem_Rough->bIsWWLAN_Available = TRUE;
		//return FALSE;
	}
	
	
	return TRUE;
}

DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData)
{
	return 0;
}

static void RefreshSysAvailableStatus(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	//now judge system available status by reading 3-connector status
	time_t tmNow = time(NULL);
	VAR_VALUE varV;
	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	int n;
	METER_TYPE emTp;
	UINT nInstWhNum = 0;
	for(n = 0; n < 8; n++) //System Total MAX_WH_NUM = 8
	{
		pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR,
			401 + n, 401, &varV, 4);
		emTp = (METER_TYPE)varV.emValue;
		if(emTp != METER_TYPE_NotInstalled)
		{
			nInstWhNum++;
		}
	}
	/*if(GetESSysType(pSampler) == ESSYSTYPE_SLAVE)
	{
		varV.ulValue = 1;//default 1, otherwise system will alarm
	}
	else*/
	{
		varV.ulValue = nInstWhNum;
	}
	Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 1, 11, varV);


	//------------- Judge System OverTemp & BAT Single OverTemp Alarm -----------//
	if(ABS(tmNow - pSys_Rough->tm_LastJudgeOverTemp) > 120)
	{
		pSys_Rough->tm_LastJudgeOverTemp = tmNow;

		pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 1, 21, &varV, 4); //BAT OverTemp PPT
		float fBAT_OverTempPPt = varV.fValue;

		for(n = 0; n < MAX_SINGLE_STBATT_NUM; n++)
		{
			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_RAW_VAL, 501 + n, 558, &varV, 4);
			
			float fBATSigl_MaxTemp = varV.fValue;

			(fBATSigl_MaxTemp > fBAT_OverTempPPt) ? (varV.emValue = 1) : (varV.emValue = 0);
		
			Push_VarValue(SETTYPE_RAW_SP, pSampler, 501 + n, 556, varV);

			TRACE("n=[%d] fBATSigl_MaxTemp=[%.1f] fBAT_OverTempPPt=[%.1f]\n", 
				n ,fBATSigl_MaxTemp, fBAT_OverTempPPt);
		}

		pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 1, 22, &varV, 4); //Power Cube OverTemp PPT
		float fPwrCube_OverTempPPt = varV.fValue;

		pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_RAW_VAL, 1, 3, &varV, 4);
		float fPwrCube_MaxTemp = varV.fValue;
		
		(fPwrCube_MaxTemp > fPwrCube_OverTempPPt) ? (varV.emValue = 1) : (varV.emValue = 0);
		
		TRACE("fPwrCube_MaxTemp=[%.1f] fPwrCube_OverTempPPt=[%.1f] fBAT_OverTempPPt=[%.1f]\n", 
			fPwrCube_MaxTemp, fPwrCube_OverTempPPt ,fBAT_OverTempPPt);

		Push_VarValue(SETTYPE_RAW_SP, pSampler, 1, 10, varV);

		//----- Leakage Current Detection -----//
		if (SYSTYPE_ESS_AIR == pSys_Rough->lcCfg.emSystemType)
		{
			pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 1, 19, &varV, 4);
			float fLeakageCurr_THD = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_RAW_VAL, 1, 40, &varV, 4);
			float fLeakageCurr_RealTime = fabs(varV.fValue);
			
			(fLeakageCurr_RealTime > fLeakageCurr_THD) ? (varV.emValue = 1) : (varV.emValue = 0);
			
			TRACE("fLeakageCurr_RealTime=[%.1f] fLeakageCurr_THD=[%.1f] \n", fLeakageCurr_RealTime, fLeakageCurr_THD);

			Push_VarValue(SETTYPE_RAW_SP, pSampler, 1, 39, varV);
		}
	}

// LJH241101 MU3
#ifdef IMMU2
	USHORT usPVSPD_DI1 = 0;
	static USHORT nPVSPDTimes = 0;

	pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 1, 29, &varV, 4);
	if(varV.emValue == 1) //0-Low Volt Level, 1-High Volt Level
 	{
		usPVSPD_DI1 = (Get_IMMU2_DI(IMMU2_DI1) == 1) ? 1 : 0; //low Volt Alarm
	}
	else
	{
		usPVSPD_DI1 = (Get_IMMU2_DI(IMMU2_DI1) == 1) ? 0 : 1; //High Volt alarm
	}
	if(usPVSPD_DI1 == 1)
	{
		nPVSPDTimes++;
	}
	else
	{
		nPVSPDTimes = 0;
	}
	if(nPVSPDTimes >= 5)
	{
		nPVSPDTimes = 5;
		usPVSPD_DI1 = 1;
	}
	else
	{
		usPVSPD_DI1 = 0;
	}
	varV.emValue = usPVSPD_DI1;
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 1, 17, varV);
	
	//---DI2---//
	USHORT usDI2 = 0;
	static USHORT nDI2Times = 0;

	pDPR->Get(pSys_Rough->hSampler, GETTYPE_ST_VAR, 1, 28, &varV, 4);
	if(varV.emValue == 1) //0-Low Volt Level, 1-High Volt Level
 	{
		usDI2 = (Get_IMMU2_DI(IMMU2_DI2) == 1) ? 1 : 0; //low Volt Alarm
	}
	else
	{
		usDI2 = (Get_IMMU2_DI(IMMU2_DI2) == 1) ? 0 : 1; //High Volt alarm
	}
	if(usDI2 == 1)
	{
		nDI2Times++;
	}
	else
	{
		nDI2Times = 0;
	}
	if(nDI2Times >= 5)
	{
		nDI2Times = 5;
		usDI2 = 1;
	}
	else
	{
		usDI2 = 0;
	}
	varV.emValue = usDI2;
	Push_VarValue(SETTYPE_RAW_SP, pSampler, 1, 21, varV);
#endif

	return;
}

static BOOL IsInclued_CCUFimrwarePkg(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	char szFoundCCUBin[64] = {0};
	if(!FindMatchFile_FromDir(CCU_FIRMWARE_UPGRADE_PATH, "IMSU_APP", szFoundCCUBin))
	{
		return FALSE;
	}

	//now push control cmd => CCU
	if(ERR_DPR_OK != Push_TextValue(SETTYPE_INTERN_CTRL, pSys_Rough->hSampler, 
		200, 94, szFoundCCUBin, 64))
	{
		return FALSE;
	}

	return TRUE;
}

static void HandleFirmwareUpgrading(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
 	A_BLOB_UPDATEFIRMWARE_CMD* pCmd = &pSys_Rough->blobPendingFirmwareUpdateCmd;
	time_t tmNow = _NOW_;
	EVENT_FIRMWAREUPDATE_STATUS	evFirmUpdt;
	ZERO_POBJS(&evFirmUpdt, 1);
	int n;
	VAR_VALUE varV;
	memcpy(&evFirmUpdt.stUpdateCmd, pCmd, sizeof(A_BLOB_UPDATEFIRMWARE_CMD));
	memcpy(evFirmUpdt.szOldFirmwareVer, pSys_Rough->blobOCPP_BootInfo.szFirmwareVer, MAX_LEN_MU2SWVER);
	if(pSys_Rough->emDownloadStatus == UPDOWN_WAIT_BEGIN)
	{
		if(((pSys_Rough->nDownloadRetryCount == 0) && (ABS(tmNow - pCmd->dtRetrieveDate) > 0)) ||
			((pSys_Rough->nDownloadRetryCount > 0) && (ABS(tmNow - pSys_Rough->tmLastDownloadRetry) >= pCmd->nRetryInterv)))
		{
			TRACEX("-------Now begin downloading, reTry = %d-------\n", pSys_Rough->nDownloadRetryCount);
			int nErrCode = ERR_COMM_OK;
			if(pSys_Rough->nDownloadRetryCount == 0)
			{
				//Jimmy 20190830: need first send downloading status(OCT test use case)
				evFirmUpdt.emUpdateStatus = FIRMUPDATE_Downloading;
				SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_BEGIN,
					&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));

				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
						"Begin download firmware from URL[%s]!", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
				pSys_Rough->hPort_DownloadInUse = pSys_Rough->commHttpFtp.Open(pSys_Rough->blobPendingFirmwareUpdateCmd.szURL,
					LOCAL_UPGRADE_FIRMWARE_NAME, 
					0, 1800, &nErrCode); //30min timeout SHALL be enough!
				if(pSys_Rough->hPort_DownloadInUse == NULL)
				{
					//download fail
					pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
						"Can NOT downloadapp from URL[%s]!", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
					evFirmUpdt.emUpdateStatus = FIRMUPDATE_DownloadFailed;
					SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
						&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
					return;
				}
			}

			if(pSys_Rough->commHttpFtp.Read(pSys_Rough->hPort_DownloadInUse, NULL, NULL) != 0)
			{
				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
					"Can NOT START donwloading from URL[%s]!", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
				evFirmUpdt.emUpdateStatus = FIRMUPDATE_DownloadFailed;
				SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
					&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
				pSys_Rough->commHttpFtp.Close(pSys_Rough->hPort_DownloadInUse);

				pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
				return;
			}

			pSys_Rough->emDownloadStatus = UPDOWN_PENDING;

		}
	}
	else if(pSys_Rough->emDownloadStatus == UPDOWN_PENDING)
	{
		int nStatus = pSys_Rough->commHttpFtp.Control(pSys_Rough->hPort_DownloadInUse, COMM_GET_HTTPFTP_PROCESS, NULL, NULL);
		TRACEX("-----Get the downloading status = %d-------\n", nStatus);
		if(nStatus == ERR_COMM_OK)
		{
			pSys_Rough->emDownloadStatus = UPDOWN_FINISHED;
			evFirmUpdt.emUpdateStatus = FIRMUPDATE_Downloaded;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_STATUS_CHANGED,
				&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			pSys_Rough->commHttpFtp.Close(pSys_Rough->hPort_DownloadInUse);
		}
		else if(nStatus != ERR_COMM_HTTPFTP_PENDING)
		{
			int nRetryCount = (pCmd->nDownloadRetries > 0) ? pCmd->nDownloadRetries : 3;

			pSys_Rough->nDownloadRetryCount++;
			if(pSys_Rough->nDownloadRetryCount < nRetryCount)
			{
				pSys_Rough->tmLastDownloadRetry = _NOW_;
				pSys_Rough->emDownloadStatus = UPDOWN_WAIT_BEGIN;
				return;
			}

			pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"Retry %d, Still can NOT donwload from URL[%s]!", pSys_Rough->nDownloadRetryCount,
				pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
			evFirmUpdt.emUpdateStatus = FIRMUPDATE_DownloadFailed;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
				&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			pSys_Rough->commHttpFtp.Close(pSys_Rough->hPort_DownloadInUse);
			return;
		}
	}
	else if(pSys_Rough->emDownloadStatus == UPDOWN_FINISHED)
	{
		//now extract pkg and check it
		char szCmd[256];
		sprintf(szCmd, "tar -zxf %s -C /opt/appdata/data", LOCAL_UPGRADE_FIRMWARE_NAME);
		_SYSTEM(szCmd);
		if((access("/opt/appdata/data/app_nccu.tar.gz", R_OK) != 0) ||
			(access("/opt/appdata/data/pkg.sum", R_OK) != 0)) //extracted pkg file and pkg.sum existed??
		{
			TRACEX("----------No pkg. and checksum file found-------\n");
			pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"Wrong pkg found after downloaded from URL[%s]!", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
			evFirmUpdt.emUpdateStatus = FIRMUPDATE_InstallFailed;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
				&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			return;
		}

		if(!DoMD5Chk_ForRemotePkg(FALSE))
		{
			pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"MD5 chk FAILED after downloaded from URL[%s]!", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
			evFirmUpdt.emUpdateStatus = FIRMUPDATE_InstallFailed;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
				&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
			return;
		}

		//now overwrite old app dir??
		_SYSTEM("tar -zxf /opt/appdata/data/app_nccu.tar.gz -C /app;sync"); //Jimmy added 20190529: should call 'SYNC' 
																		//because UBI fs will not save data to flash 
																		//if controller lost power
		// //for IMSU-D update
		// if(access("/opt/appdata/data/IMSU_APP.bin", R_OK) == 0)
		// {
		// 	_SYSTEM("cp /opt/appdata/data/IMSU_APP.bin /app -f;sync");
		// }

		evFirmUpdt.emUpdateStatus = FIRMUPDATE_Installing;
		SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_STATUS_CHANGED, 
			&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));

		//Sleep(2000);
		//pSys_Rough->tmStartNotifySlaveUpgrade = _NOW_;

		pSys_Rough->emDownloadStatus = UPDOWN_WAIT_REBOOT;
		
		return;
	}
	else if(pSys_Rough->emDownloadStatus == UPDOWN_WAIT_REBOOT)
	{
		DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
		for(n = 0; n < 3; n++)
		{
			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR,
				201 + n, 101, &varV, 4);
			if((varV.emValue >= GUN_STATE_RESERVE_PENDING) &&
				(varV.emValue <= GUN_STATE_WAITING_END))
			{
				pSys_Rough->emDownloadStatus = UPDOWN_WAIT_REBOOT;
				return;
			}
		}

		//judge if has CCU firmware & wait CCU upgrade
		if(IsInclued_CCUFimrwarePkg(pSys_Rough))
		{
			pSys_Rough->emDownloadStatus = UPDOWN_WAIT_REBOOT_AND_CCU;
			pSys_Rough->tmLast_EnterWaitCCUUpgrade = _NOW_;
			return;
		}
		//if proper time, then reboot system
		EVENT_SYSTEM_EXIT evExit;
		evExit.nExitType = SYS_EXIT_FIRMWAREUPDT;
		//temporary shield Exit Msg For System 
		SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
			&evExit, sizeof(EVENT_SYSTEM_EXIT));
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
			"Downloaded firmware SUCCESSFULLY from URL[%s]!", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);

		pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
	}
	else //UPDOWN_WAIT_REBOOT_AND_CCU
	{
		static int s_nCCU_UpdtStatusRemainCounter = 0;
		BOOL bIsNeedExitNow = FALSE;
		DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
		pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR,
				200, 130, &varV, 4);
		if((varV.emValue >= 3) ||
			(varV.emValue == 0)) //ccu status success/fail or not started yet
		{
			if(ABS(_NOW_ - pSys_Rough->tmLast_EnterWaitCCUUpgrade) > 3)
			{
				s_nCCU_UpdtStatusRemainCounter++;
				pSys_Rough->tmLast_EnterWaitCCUUpgrade = _NOW_;
			}
			if(s_nCCU_UpdtStatusRemainCounter >= 3)
			{
				bIsNeedExitNow = TRUE;
			}
		}
		else
		{
			s_nCCU_UpdtStatusRemainCounter = 0;
		}

		if(bIsNeedExitNow)
		{
			EVENT_SYSTEM_EXIT evExit;
			evExit.nExitType = SYS_EXIT_FIRMWAREUPDT;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
				&evExit, sizeof(EVENT_SYSTEM_EXIT));
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
				"Now reboot the system after CCU upgrade done!");

			pSys_Rough->emDownloadStatus = UPDOWN_IDLE;
		}
	}
}

static void HandleDiagnoseUploadiing(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
#define LOCAL_DIAGNOSTIC_FILE_PREFIX	"/opt/appdata/data/"
	A_BLOB_UPLOADDIAGN_CMD* pCmd = &pSys_Rough->blobPendingDiagnosUploadCmd;
	time_t tmNow = _NOW_;
	EVENT_DIAGNOUPLOAD_STATUS	evDiagUploadStat;
	ZERO_POBJS(&evDiagUploadStat, 1);
	int n;
	//VAR_VALUE varV;
	static BOOL s_bIsUploadOK = FALSE;
	//memcpy(&evFirmUpdt.stUpdateCmd, pCmd, sizeof(A_BLOB_UPDATEFIRMWARE_CMD));
	if(pSys_Rough->emUploadStatus == UPDOWN_WAIT_BEGIN)
	{
		s_bIsUploadOK = FALSE;
		if((pSys_Rough->nUploadRetryCount == 0) ||
			((pSys_Rough->nUploadRetryCount > 0) && (ABS(tmNow - pSys_Rough->tmLastUploadRetry) >= pCmd->nRetrieveInterv)))
		{
			TRACEX("-------Now begin upload the file, reTry = %d-------\n", pSys_Rough->nUploadRetryCount);
			int nErrCode = ERR_COMM_OK;
			if(pSys_Rough->nUploadRetryCount == 0)
			{
				char szLocalFile[256];
				snprintf(szLocalFile, 256, "%s%s", LOCAL_DIAGNOSTIC_FILE_PREFIX, 
					pSys_Rough->blobPendingDiagnosUploadCmd.szDiagnosticFileName);
				char szCmd[256];
				sprintf(szCmd, "cp -rf /data/nccu.data %s;sync", szLocalFile);
				//sprintf(szCmd, "tar -zcvf %s /data/nccu.data;sync", szLocalFile);
				_SYSTEM(szCmd);
				//JImmy added 20200521: need time for tar to run
				//can not work beacuse if > 100M, the time cost is very long
				//Sleep(15000);

				//Raw URL shall be pointed to a folder
				char szCompleteUrl[255];
				int nRawUrlLen = strlen(pSys_Rough->blobPendingDiagnosUploadCmd.szURL);
				TRACEX("URL For upload Dignostics %s\n", pSys_Rough->blobPendingDiagnosUploadCmd.szURL);
				if (pSys_Rough->blobPendingDiagnosUploadCmd.szURL[nRawUrlLen - 1] == '/')
				{
					snprintf(szCompleteUrl, 255, "%s%s", pSys_Rough->blobPendingDiagnosUploadCmd.szURL, pSys_Rough->blobPendingDiagnosUploadCmd.szDiagnosticFileName);
				}
				else
				{
					snprintf(szCompleteUrl, 255, "%s/%s", pSys_Rough->blobPendingDiagnosUploadCmd.szURL, pSys_Rough->blobPendingDiagnosUploadCmd.szDiagnosticFileName);
				}

				evDiagUploadStat.emUploadStatus = DIAGNOUPLOAD_Uploading;
				SendMsg(pSys_Rough->hSampler, EVENT_TYPE_DIAGNOSTIC_UPLOAD, EVENT_MSG_BEGIN, 
					&evDiagUploadStat, sizeof(EVENT_DIAGNOUPLOAD_STATUS));

				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
						"Begin upload diagnostic file into URL[%s]!", szCompleteUrl);
				pSys_Rough->hPort_UploadInUse = pSys_Rough->commHttpFtp.Open(szCompleteUrl, 
						szLocalFile, 0, 1800, &nErrCode); //30min timeout SHALL be enough!
				if(pSys_Rough->hPort_UploadInUse == NULL)
				{
					//upload fail
					pSys_Rough->emUploadStatus = UPDOWN_IDLE;
					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
						"Can NOT upload diagnostic file into URL[%s]!", pSys_Rough->blobPendingDiagnosUploadCmd.szURL);
					evDiagUploadStat.emUploadStatus = DIAGNOUPLOAD_UploadFailed;
					SendMsg(pSys_Rough->hSampler, EVENT_TYPE_DIAGNOSTIC_UPLOAD, EVENT_MSG_END, 
						&evDiagUploadStat, sizeof(EVENT_DIAGNOUPLOAD_STATUS));
					return;
				}
			}

			if(pSys_Rough->commHttpFtp.Write(pSys_Rough->hPort_UploadInUse, NULL, NULL) != 0)
			{
				pSys_Rough->emUploadStatus = UPDOWN_FINISHED;
				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
					"Can NOT start uploading diagnostic file into URL[%s]!", pSys_Rough->blobPendingDiagnosUploadCmd.szURL);
				evDiagUploadStat.emUploadStatus = DIAGNOUPLOAD_UploadFailed;
				SendMsg(pSys_Rough->hSampler, EVENT_TYPE_DIAGNOSTIC_UPLOAD, EVENT_MSG_END, 
					&evDiagUploadStat, sizeof(EVENT_DIAGNOUPLOAD_STATUS));
				pSys_Rough->commHttpFtp.Close(pSys_Rough->hPort_UploadInUse);

				return;
			}

			pSys_Rough->emUploadStatus = UPDOWN_PENDING;
		}
	}
	else if(pSys_Rough->emUploadStatus == UPDOWN_PENDING)
	{
		int nStatus = pSys_Rough->commHttpFtp.Control(pSys_Rough->hPort_UploadInUse, COMM_GET_HTTPFTP_PROCESS, NULL, NULL);
		TRACEX("-----Get the uploading status = %d-------\n", nStatus);
		if(nStatus == ERR_COMM_OK)
		{
			pSys_Rough->emUploadStatus = UPDOWN_FINISHED;
			s_bIsUploadOK = TRUE;
			evDiagUploadStat.emUploadStatus = DIAGNOUPLOAD_Uploaded;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_DIAGNOSTIC_UPLOAD, EVENT_MSG_END, 
				&evDiagUploadStat, sizeof(EVENT_DIAGNOUPLOAD_STATUS));
			pSys_Rough->commHttpFtp.Close(pSys_Rough->hPort_UploadInUse);
		}
		else if(nStatus != ERR_COMM_HTTPFTP_PENDING)
		{
			int nRetryCount = (pCmd->nRetries > 0) ? pCmd->nRetries : 3;

			pSys_Rough->nUploadRetryCount++;
			if(pSys_Rough->nUploadRetryCount < nRetryCount)
			{
				pSys_Rough->tmLastUploadRetry = _NOW_;
				pSys_Rough->emUploadStatus = UPDOWN_WAIT_BEGIN;
				return;
			}

			pSys_Rough->emUploadStatus = UPDOWN_FINISHED;
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"Retry %d, Still can NOT upload into URL[%s]!", pSys_Rough->nUploadRetryCount,
				pSys_Rough->blobPendingDiagnosUploadCmd.szURL);
			evDiagUploadStat.emUploadStatus = DIAGNOUPLOAD_UploadFailed;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_DIAGNOSTIC_UPLOAD, EVENT_MSG_END, 
				&evDiagUploadStat, sizeof(EVENT_DIAGNOUPLOAD_STATUS));
			pSys_Rough->commHttpFtp.Close(pSys_Rough->hPort_UploadInUse);
		}
	}
	else if(pSys_Rough->emUploadStatus == UPDOWN_FINISHED)
	{
		//now extract pkg and check it
		char szLocalFile[256];
		snprintf(szLocalFile, 256, "%s%s", LOCAL_DIAGNOSTIC_FILE_PREFIX, 
			pSys_Rough->blobPendingDiagnosUploadCmd.szDiagnosticFileName);
		char szCmd[256];
		sprintf(szCmd, "rm -f %s", szLocalFile);
		_SYSTEM(szCmd);
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
			"Uploaded diagnostic %s to URL[%s]!", 
			s_bIsUploadOK ? "SUCCESSFULLY" : "FAILED",
			pSys_Rough->blobPendingDiagnosUploadCmd.szURL);

		pSys_Rough->emUploadStatus = UPDOWN_IDLE;
		return;
	}
}

// static int ReadATResp_FromModem(char* szCmd, char* szCmdPost, char*szResponse, int nTimeout)
// {
// 	int fd_ttyUSB2 = open(TTYS_USB2, O_RDWR | O_NONBLOCK | O_NOCTTY | O_EXCL); //added by Jimmy 20200722 fpr 'O_TRUNC' to cut history msgs
// 	if(fd_ttyUSB2 < 0)
// 	{
// 		TRACEX("\nopen /dev/ttyUSB2 failed!\n");
// 		return -1;
// 	}

// 	if(WaitFiledWritable(fd_ttyUSB2, 10) <= 0)
// 	{
// 		TRACEX("\n/dev/ttyUSB2 is NOT writtable!\n");
// 		close(fd_ttyUSB2);
// 		return -1;
// 	}

// 	//char szReadOnce[64] = {0};
// 	char szRead[1025] = {0};
// 	read(fd_ttyUSB2, szRead, 1024); //added by Jimmy 20210409 for emptying the previous read-buffer	
// 	int nReadLen = 0;
// 	//Jimmy addeed, first need clear the buffer if need
// 	//does not make sense, because some AT cmd reply too late and we do not clear the buff here
// // 	if(WaitFiledReadable(fd_ttyUSB2, 10) > 0)
// // 	{		
// // //#include <termios.h>
// // 		//read(fd_ttyUSB2, szRead, 512);	
// // 		//tcflush(fd_ttyUSB2, TCOFLUSH);
// // 		//if(nReadLen > 1)
// // 		{
// // 			printf("\nOOPS modem got older buffer[%s]\n", szRead);	
// // 		}			
// // 	}

// 	char szWrite[64] = {0};
// 	//Jimmy added \r 20210428 as read from ZTE AT maunual that '\r' is for modem to know cmd ending
// 	snprintf(szWrite, 64, "\rAT%s%s\r\n", szCmd, szCmdPost);
// 	int nWriteLen = strlen(szWrite) + 1;
// 	nWriteLen = write(fd_ttyUSB2, szWrite, nWriteLen);
// 	if(nWriteLen < strlen(szWrite))
// 	{
// 		TRACEX("Can not write data to modem, please check!\n");
// 		close(fd_ttyUSB2);
// 		return -2;
// 	}
// 	TRACEX("Now write [%s] -> %d to dev\n", szWrite, nWriteLen);
// 	//Jimmy note: 10000 means not limit the count, but we just use time to limit the read process
// 	//since select always return 'ready' for read, we do not rely on it!!!
// 	int nSleepTimes = 10000; //MAX(nTimeout / 100, 5); //max wait 500ms when timeout = 0
// 	int nRet = -99;
// 	//int nNo_ReadCount = 0;
// 	//BOOL bHasRead = FALSE;
// 	int rc = 0;
// 	memset(szRead, 0, 512);
// 	TRACEX("\nmodem_write done for [%s, %.3f]", szWrite, GetCurrentTime());	
// 	double tmWaitBegin = GetCurrentTime();
// 	double tmDiff = (double)nTimeout / 1000.0;
// 	double tmLeft = tmDiff;
// 	char* szRepos;
// 	int nReadErrCounter = 0;
// 	while(nSleepTimes >= 0)	
// 	{
// 		//memset(szReadOnce, 0, 64);
// 		//nReadLen = read(fd_ttyUSB2, szReadOnce, 63);	
// 		//Jimmy changed 20210505 for sometimes there maybe huge data on comm port
// 		memset(szRead, 0, 1025);
// 		nReadLen = read(fd_ttyUSB2, szRead, 1024);	
// 		TRACEX("\nmodem_read times[%d, %.3f]", nSleepTimes, GetCurrentTime());	
// 		if(nReadLen > 1)
// 		{	
// 			// if(strlen(szReadOnce) + strlen(szRead) < 512)
// 			// {
// 			// 	strcat(szRead, szReadOnce);
// 			// }
// 			// else
// 			// {
// 			// 	//nSleepTimes = -1; //do not read any longer since there are too many error
// 			// 	break;
// 			// }
			
// 			TRACEX("\nNow get the resp[%d], strTotal[%s]\n!", nSleepTimes, szRead);
// 			if((szRepos = strstr(szRead, szCmd)) != NULL)
// 			{
// 				if(GetSplitStringCount(szRepos, ':') > 1)
// 				{					
// 					TRACEX("Now get the right response [%s] and repos[%s]!", szRead, szRepos);
// 					char *pos; 
// 					char *szRead1 = SplitString(szRepos, &pos, ':');
// 					SplitString(szRead1, &pos, ':');
// 					if(pos != NULL)
// 					{
// 						char *possss = pos;
// 						while (*possss != 0)
// 						{
// 							if((*possss == '\r') || (*possss == '\n'))
// 							{
// 								*possss = '\0';
// 								break;
// 							}
// 							possss++;
// 						}
// 						snprintf(szResponse, 64, "%s", pos);
// 						//printf("\n-------The response for ATcmd=>[%s] is [%s]------\n", szCmd, szResponse);
// 						nRet = 0;						
// 					}
// 				}
// 				else if(strstr(szRead, "AT+") != NULL) //Jimmy added 20200720, file is wrong
// 				{
// 					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 						"LTE modem received REPEATED reply for AT=>[%s], tmLeft=>%.3f!!!", szCmd, tmLeft);
// 					nRet = -88;
// 					write(fd_ttyUSB2, "\r\rATE0\r\n", 8); //added by Jimmy 20210429 to help modem recover from AT failing
// 					//break; //Jimmy 20210427: don't break but continue
// 				}
// 				else
// 				{
// 					TRACEX("ReadRRR other resp[%s] for AT cmd[%s]!\n", szRead, szWrite);
// 					nRet = -4;
// 				}
// 			}
// 			else if(strstr(szRead, "ERROR") != NULL)
// 			{
// 				TRACEX("Read ERROR for AT cmd[%s]!\n", szWrite);
// 				nRet = -3;
// 			}
// 			else
// 			{
// 				if((strcmp(szCmd, "+CIMI") == 0) && 
// 					(strstr(szRead, "+") == NULL)) //CIMI直锟接凤拷锟截斤拷锟�/**/
// 				{
// 					nRet = 0;
// 					//sprintf(szResponse, szRead);
// 					Trim_String(szRead, '\r');
// 					Trim_String(szRead, '\n');					
// 					snprintf(szResponse, 64, "%s", szRead);
// 					//printf("\n-------The response for AT+CIMI is [%s]------\n", szResponse);
// 				}
// 				else
// 				{
// 					TRACEX("Read other resp[%s] for AT cmd[%s]!\n", szRead, szWrite);
// 					nRet = -5;
// 				}
// 			}
// 			//if(nTimeout != 0) //wait until read
// 			//{
// 			//	break;
// 			//}
// 			//else //wait last line
// 			//{
// 			//	nNo_ReadCount = 0;
// 			//	bHasRead = TRUE;
// 			//}
// 		}
// 		// else
// 		// {
// 		// 	nNo_ReadCount++;
// 		// 	if((nNo_ReadCount > 1) && (bHasRead))
// 		// 	{
// 		// 		break;
// 		// 	}
// 		// 	TRACEX("Now get empty resp [%d],[%s]!\n", nReadLen, szRead);
// 		// }

// 		//aded by Jimmy for error read
// 		if(nRet == 0)
// 		{
// 			break;
// 		}

// 		nSleepTimes--;
// 		//Sleep(100);
// 		rc = WaitFiledReadable(fd_ttyUSB2, 100);
// 		if(rc <= 0)
// 		{
// 			TRACEX("\n %s happens on reading back ttyUSB2, so quit!!!\n", 
// 				(rc < 0) ? "error" : "timeout");
// 			//only when error, we quit
// 			if(rc < 0)
// 			{
// 				break;
// 			}			
// 		}

// 		tmLeft = tmDiff - (GetCurrentTime() - tmWaitBegin);
// 		if((tmLeft < 0.001) || (tmLeft > (tmDiff + 0.1))) //in case sysem timestamp error
// 		{
// 			break;
// 		}

// 		nReadErrCounter++;
// 	}

// 	close(fd_ttyUSB2);

// 	return nRet;
// }

// static BOOL QueryModemSignal(SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bIsInDialing)
// {
// 	char szATResp[128] = {0};
// 	int nRet = ReadATResp_FromModem("+CSQ" , "", szATResp, /*500*/bIsInDialing ? 5000 : 500);
// 	static int s_nTTYUSB_NoReply_Counter = 0; //if ttyUSB no repsonse for a long time; we do not reset modem since it makes no sense
// 	if(nRet!= 0)
// 	{
// 		int nERR_Type = 0;		
// 		pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0; //no signal
// 		if(nRet == -88) //added by Jimmy 20200720, if file not created, we consider modem is failed
// 		{
// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 					"LTE modem got REPEATED AT+CSQ Reply, errCounter=>%d!!!", pSys_Rough->nReadModem_ErrorCounter);
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = -177;
// 			nERR_Type = 1;	
// 		}
// 		else if(nRet == -3)
// 		{
// 			//added by Jimmy 20210505: if received 'ERROR', we do not care it but see if it can be recovered later(for testing purpose)
// 			if(!bIsInDialing)
// 			{
// 				pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = -188;
// 				nERR_Type = 2;					
// 			}
// 			else
// 			{
// 				s_nTTYUSB_NoReply_Counter++;
// 				int nERRCOUNTER_QUERYSIGNAL = pSys_Rough->bIsWWLAN_Available ? 30 : 5;
// 				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 					"LTE modem got ERROR reply for AT+CSQ [%d of %d]!", s_nTTYUSB_NoReply_Counter, nERRCOUNTER_QUERYSIGNAL);
// 				if(s_nTTYUSB_NoReply_Counter >= nERRCOUNTER_QUERYSIGNAL)
// 				{
// 					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 						"Too many ERROR reply so we RESET the modem now!");
// 					s_nTTYUSB_NoReply_Counter = 0;
// 					pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_RESET;					
// 				}
// 			}
// 		}
// 		//Jimmy added 20200611, just found that there is chance that we get no response
// 		else if(nRet == -99)
// 		{
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = -199;
// 			nERR_Type = 3;	
// 		}
// 		// else
// 		// {
// 		// 	pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = -999;
// 		// }

// 		if(nERR_Type > 0)
// 		{
// 			//added by Jimmy 20200905: if in dialing, query can be of NO response for long time!
// 			int nERRCOUNTER_QUERYSIGNAL = pSys_Rough->bIsWWLAN_Available ? 30 : 5;
// 			pSys_Rough->nReadModem_ErrorCounter++;
// 			if(pSys_Rough->nReadModem_ErrorCounter >= nERRCOUNTER_QUERYSIGNAL)
// 			{
// 				//s_nTTYUSB_NoReply_Counter++;
// 				pSys_Rough->nReadModem_ErrorCounter = 0;
// 				pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_RESET;

// 				if(nERR_Type == 1)
// 				{
// 					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 						"LTE modem went into error status## and need hard reset!");
// 				}
// 				else if(nERR_Type == 2)
// 				{
// 					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 						"LTE modem went into error status and need hard reset!");
// 				}
// 				else if(nERR_Type == 3)
// 				{
// 					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 						"LTE modem is of no response and need hard reset!");
// 				}
// 			}

// 			// if(s_nTTYUSB_NoReply_Counter >= 3)
// 			// {	
// 			// 	//err >= 3: no need reset any more!			
// 			// 	pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
// 			// 	if(s_nTTYUSB_NoReply_Counter == 3)
// 			// 	{
// 			// 		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 			// 			"Panic! LTE modem run into ERROR status, need REBOOT systm to recover it!");
// 			// 		//_SYSTEM("reboot&");
// 			// 	}
// 			// 	s_nTTYUSB_NoReply_Counter = 5;
// 			// }		
// 		}
		
// 		return FALSE;
// 	}
// 	else
// 	{
// 		s_nTTYUSB_NoReply_Counter = 0;
// 		pSys_Rough->nReadModem_ErrorCounter = 0;
// 		int nSigndBm = atoi(szATResp);
// 		nSigndBm = nSigndBm * 2 - 113;
// 		//pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0; //no signal
// 		if((nSigndBm <= -105) || (nSigndBm > -50))
// 		{
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = -106;
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0; //no signal
// 		}
// 		else
// 		{
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = nSigndBm;//nSigndBm * 2 - 113;
// 			// if(nSigndBm < -105)
// 			// {
// 			// 	pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0;
// 			// }
// 			//else if(nSigndBm <= -100)
// 			if(nSigndBm <= -100)
// 			{
// 				pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 1;
// 			}
// 			else if(nSigndBm <= -95)
// 			{
// 				pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 2;
// 			}
// 			else if(nSigndBm <= -90)
// 			{
// 				pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 3;
// 			}
// 			else if(nSigndBm <= -85)
// 			{
// 				pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 4;
// 			}
// 			else
// 			{
// 				pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 5;
// 			}
// 		}

// 		TRACEX("Now SignalStrength = %ddBm, and Level = %d\n",
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm, pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level);
// 		////now set raw data to setting param
// 		//Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 		//	1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 	}

// 	return TRUE;
// }

// static BOOL Check_IfSIMInsert_AndResetModem(SYSTEM_SAMPLER_DATA *pSys_Rough)
// {
// 	char szATResp[128] = {0};
// 	int nGGGErr = ReadATResp_FromModem("+CPIN" , "?", szATResp, 500);
// 	BOOL bSIM_Detected = TRUE;
// 	if(nGGGErr != 0)
// 	{
// 		//pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
// 		TRACEX("\nNow get modem sim inst_status error[%d,%s]\n", nGGGErr, szATResp);
// 		//return;
// 		bSIM_Detected = FALSE;
// 	}
// 	else if(strstr(szATResp, "READY") == NULL)
// 	{
// 		TRACEX("\nSIM card is NOT READY, so we quit the dialing!\n");
// 		//return;
// 		bSIM_Detected = FALSE;
// 	}

// 	static int s_SIM_ErrCounter = 0;
// 	if(!bSIM_Detected)
// 	{
// 		s_SIM_ErrCounter++;
// 		if(s_SIM_ErrCounter >= 5)
// 		{
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_dBm = -333;
// 			pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0; //no signal			
// 			if(s_SIM_ErrCounter == 5)
// 			{
// 				pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
// 				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 					"SIM card NOT detected, will reset the modem!");
// 			}
// 			//just to show on GUI so that we know the SIM not identified
// 			//if still NO SIM after reset, will not reset it again
// 			s_SIM_ErrCounter = 10;	
					
// 			Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 				1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 		}
// 	}
// 	else
// 	{
// 		s_SIM_ErrCounter = 0;
// 	}	

// 	return bSIM_Detected;
// }

// static void Manage_LTEModem(SYSTEM_SAMPLER_DATA *pSys_Rough)
// {
// 	time_t tmNow = _NOW_;
// 	char szATResp[128] = {0};

// 	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
// 	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

// 	static int s_nContinus_DialFail_Counter = 0; //added by Jimmy 20210812
// 	static int nUSB0DisConnectCounter = 0;
// 	//static int nDialErrorCounter = 0; //if dial error 5 times continuely, then reset modem

// 	//added by Jimmy 20201214 for disable/enable LTE
// 	static BOOL s_bIsLastEnabledLTEModem = 99;
// 	if(pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem != s_bIsLastEnabledLTEModem)
// 	{
// 		s_bIsLastEnabledLTEModem = pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem;
// 		if(s_bIsLastEnabledLTEModem == 1)
// 		{			
// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
// 					"LTE modem was ENABLED, so we reset it and begin dialing!");
// 			_SYSTEM("/app/modem_reset sim0 &");
// 		}
// 		else 
// 		{			
// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
// 					"LTE modem was DISABLED, so we power it off!");
// 			_SYSTEM("/app/modem_reset stop &");
// 		}
// 		pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
// 		pSys_Rough->tmLastQueryInstallStatus = _NOW_ + 5;
// 		//now set raw data to setting param
// 		pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0;
// 		pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_NOTINST;
// 		Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 			1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 		//clear the imsi and iccid info
// 		pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi = 0;
// 		pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid = 0;
// 		Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 17, 
// 						(void*)&pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
// 	}
// 	if(s_bIsLastEnabledLTEModem != 1) //do not dial when disabled
// 	{
// 		if(s_bIsLastEnabledLTEModem > 1)
// 		{
// 			Manage_ExternalRouter(pSys_Rough);
// 		}
// 		return;
// 	}
	
// 	//first get CSQ
// 	if(LTEMODEM_STATUS_NOT_INSTALLED == pSys_Rough->emModemStatus)
// 	{
// 		pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_NOTINST;
// 		if(ABS(tmNow - pSys_Rough->tmLastQueryInstallStatus) < 6)
// 		{
// 			return;
// 		}
// 		pSys_Rough->tmLastQueryInstallStatus = tmNow;
// 		pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level = 0;
// 		if(QueryModemSignal(pSys_Rough, FALSE))
// 		{
// 			//added by Jimmy 20200731: if signal is too weak, NO dial !!!
// 			if(pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level < 1)
// 			{
// 				Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 					1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 				return;
// 			}

// 			//pSys_Rough->tmLastQueryInstallStatus += 5; //dev found, need query it immediately
// 			//Now get SIM status
// 			//added by Jimmy 20200730 for SIM-card hot-plug purpose
// 			if(!Check_IfSIMInsert_AndResetModem(pSys_Rough))
// 			{
// 				Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 					1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 				return;
// 			}
// 			else
// 			{
// 				//added by Jimmy 20210428 just for ignoring the ECHO by 'ATE0'(see ZTE AT manual)
// 				ReadATResp_FromModem("E0" , "", szATResp, 100); 
// 				// if(strstr(szATResp, "READY") != NULL)
// 				// {
// 				// 	TRACEX("Now SIM card is READY!\n");
// 				// }
// 				//get IMSI and ICCID
// 				BOOL bNeedUpdateBoootInfo = FALSE;
//  				if(ReadATResp_FromModem("+CIMI" , "", szATResp, 500) == 0)
// 				{
// 					TRACEX("IMSI = [%s]!\n", szATResp);					
// 					if((strcmp(pSys_Rough->blobOCPP_BootInfo.szIMSI, szATResp) != 0) ||
// 						(!pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi))
// 					{
// 						pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.imsi = 1;
// 						snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szIMSI, 20, "%s", szATResp);
// 						snprintf(pSys_Rough->blobOCPP_BootInfo.szIMSI, 20, "%s", szATResp);
// 						bNeedUpdateBoootInfo = TRUE;
// 						//Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 17, 
// 						//	(void*)&pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
// 					}
// 				}
// 				if(ReadATResp_FromModem("+ZGETICCID" , "", szATResp, 500) == 0)
// 				{
// 					TRACEX("ICCID = [%s]!\n", szATResp);					
// 					if((strcmp(pSys_Rough->blobOCPP_BootInfo.szICCID, szATResp) != 0) ||
// 						(!pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid))
// 					{
// 						pSys_Rough->blobOCPP_BootInfo.emBootInfoOptions.iccid = 1;
// 						snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szICCID, 32, "%s", szATResp);
// 						snprintf(pSys_Rough->blobOCPP_BootInfo.szICCID, 32, "%s", szATResp);
// 						bNeedUpdateBoootInfo = TRUE;
// 						//Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 17, 
// 						//	(void*)&pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
// 					}
// 				}
				
// 				//Need update bootInfo
// 				if(bNeedUpdateBoootInfo)
// 				{
// 						Push_BlobValue(SETTYPE_INTERN_SETT, (HANDLE)pSampler, 1, 17, 
// 							(void*)&pSys_Rough->blobOCPP_BootInfo, sizeof(A_BLOB_CHARGER_BOOTINFO), BLOB_TYPE_CHARGER_BOOTINFO);
// 				}

// 				//Set APN Param
// 				char szAPNParam[64]={0};
// 				if(pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN)
// 				{
// 					memcpy(szAPNParam, pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN, 64);
// 				}

// 				// if(szAPNParam[0] != '\0')//Jimmy note: auto mode do not need set
// 				// {
// 				// 	TRACEX("Manage_LTEModem szAPNParam=%s\n",szAPNParam);
// 				// 	char szATCmd[128]={0};
// 				// 	sprintf(szATCmd, "=1,\"IP\",\"%s\"", szAPNParam);
// 				// 	ReadATResp_FromModem("+CGDCONT", szATCmd , szATResp, 500);
// 				// 	TRACEX("****CGDCONT = [%s]!\n", szATResp);
// 				// }

// 				if(!pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN)
// 				{
// 					//now get back current APN param
// 					//if(ReadATResp_FromModem("+CGDCONT", "?", szATResp, 0) == 0)
// 					if(ReadATResp_FromModem("+CGDCONT", "?", szATResp, 500) == 0)
// 					{
// 						if(GetSplitStringCount(szATResp, ',') > 2)
// 						{
// 							char *pos; 
// 							char *szRead1 = SplitString(szATResp, &pos, ',');
// 							char *szRead2 = SplitString(szRead1, &pos, ',');
// 							SplitString(szRead2, &pos, ',');
// 							if(strlen(pos) > 2)
// 							{
// 								snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN, 64, "%s", pos + 1);
// 								TRACEX("The current CGDCONT = [%s]!\n", pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN);
// 							}
// 							else
// 							{
// 								memset(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN, 0 , 64);
// 							}
// 						}
// 					}
// 					// else
// 					// {
// 					// 	return; //neet wait?
// 					// }
// 				}

// 				pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_CONNECT;
// 			}
// 		}
// 		else
// 		{
// 			//printf("Now get modem_signal error\n");
// 		}
		
// 		//now set raw data to setting param
// 		Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 			1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 	}
// 	else if(LTEMODEM_STATUS_WAIT_CONNECT == pSys_Rough->emModemStatus)
// 	{
// 		//Jimmy note 20190902: found that the reset script can cause a crash, do we do not do this
// 		// nDialErrorCounter++;
// 		// if(nDialErrorCounter > 5)
// 		// {
// 		// 	nDialErrorCounter = 0;
// 		// 	pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_RESET;
// 		// 	LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 		// 			"LTE modem try dialing FAILED 5 times, so reset it now!");
// 		// 	return;
// 		// }

// 		//------------added by Jimmy 20200730 for SIM hot-plug-------
// 		if(!Check_IfSIMInsert_AndResetModem(pSys_Rough))
// 		{
// 			pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
// 			return;
// 		}

// 		//if signal is weak, we need dial later
// 		BOOL bIsModemSignalOK = FALSE;
// 		if(QueryModemSignal(pSys_Rough, FALSE))
// 		{
// 			if(pSys_Rough->currNetWorkInfo.lteModemINfo.nSignal_Level >= 1)
// 			{
// 				bIsModemSignalOK = TRUE;
// 			}
// 		}
		
// 		if(!bIsModemSignalOK)
// 		{
// 			pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
// 			return;
// 		}		
// 		//-------------end add for SIM hot-plug detection--------
		
// 		pSys_Rough->emModemStatus = LTEMODEM_STATUS_DISCONECTED;
// 		pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_DISCONN;
// 		pSys_Rough->tmLastDial = tmNow;
// 		char szModemDial[256];
// 		//Jimmy note 20200609: the system call shall better run background
// 		if(pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN &&
// 			(strlen(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN) > 0))
// 		{
// 			if(strlen(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_User) == 0)
// 			{
// 				sprintf(szModemDial, "/app/modem_dial %s &", pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN);
// 			}
// 			else
// 			{
// 				if(strlen(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_Passwd) > 0)
// 				{
// 					sprintf(szModemDial, "/app/modem_dial %s %s %s &", 
// 						pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN, 
// 						pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_User,
// 						pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_Passwd);		
// 				}
// 				else
// 				{
// 					sprintf(szModemDial, "/app/modem_dial %s %s &", 
// 						pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN, 
// 						pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_User);		
// 				}
// 			}

// 		}
// 		else
// 		{
// 			sprintf(szModemDial, "/app/modem_dial &");
// 		}
		
// 		_SYSTEM(szModemDial); //now dial and wait
// 		TRACEX("Now send dial cmd to LTE-Modem\n");
// 		if(s_nContinus_DialFail_Counter < 10)
// 		{
// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
// 				"Begin dialing LTE modem!");
// 		}
// 		else if(s_nContinus_DialFail_Counter == 10)
// 		{
// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
// 				"Too many dial failure!!! Will still try dialing in every 1 min but NOT log!");
// 		}
// 		else
// 		{
// 			s_nContinus_DialFail_Counter = 11;
// 		}
// 	}
// 	else if(LTEMODEM_STATUS_DISCONECTED == pSys_Rough->emModemStatus)
// 	{
// 		if((ABS(tmNow - pSys_Rough->tmLastQueryInstallStatus) < 5) ||
// 			(ABS(tmNow - pSys_Rough->tmLastDial) < 5))
// 		{
// 			return;
// 		}
// 		pSys_Rough->tmLastQueryInstallStatus = tmNow;

// 		//do not query AT here, as modem may not reply AT while dialing
// 		// if(QueryModemSignal(pSys_Rough))
// 		// {
// 		// 	//added by Jimmy 20200722 for getting net-type quickly
// 		// 	ReadATResp_FromModem("+ZPAS" , "?", szATResp, 500);
// 		// 	pSys_Rough->tmLastQuerySignalStrength = tmNow;
// 		// 	//now set raw data to setting param
// 		// 	Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 		// 		1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 		// }
// 		// else
// 		// {
// 		// 	//now set raw data to setting param
// 		// 	Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 		// 		1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 		// 	return;
// 		// }		

// 		if(ABS(tmNow - pSys_Rough->tmLastDial) >= 60) //wait max 30s for dialing， Jimmy changed it 30 => 60 20210812
// 		{
// 			//pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_CONNECT;
// 			//should go to not_install state instead of re-dialing
// 			pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
// 			s_nContinus_DialFail_Counter++;
// 			return;
// 		}

// 		if(GetNetInfo("usb0", &pSys_Rough->currNetWorkInfo.netInfo_usb0) == 0)
// 		{
// 			return;
// 		}
// 		pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = Judge_IfNetwork_Connected("usb0") ? NETPORT_STATUS_CONNECTED : NETPORT_STATUS_DISCONN;
// 		memset(pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, 0, 32);

// 		TRACEX("NewStat[%d], tmLast vs Now[%d->%d], tmLastQury[%d]\n", pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status,
// 			pSys_Rough->tmLastDial, tmNow, pSys_Rough->tmLastQuerySignalStrength);
			
// 		if(NETPORT_STATUS_CONNECTED == pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status)
// 		{
// 			s_nContinus_DialFail_Counter = 0;
// 			nUSB0DisConnectCounter = 0;
// 			pSys_Rough->emModemStatus = LTEMODEM_STATUS_READY;
// 			pSys_Rough->tmLastQueryNetworkTp = 0;
// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
// 					"LTE modem is READY now!");
// 			return;
// 		}
// 	}
// 	else if(LTEMODEM_STATUS_READY == pSys_Rough->emModemStatus)
// 	{
// 		//changed by Jimmy 20210413
// 		static time_t s_tmLastRefresh_ModemDisconnStatus = 0;
// 		if(ABS(tmNow - s_tmLastRefresh_ModemDisconnStatus) < 5)
// 		{
// 			return;
// 		}		
// 		s_tmLastRefresh_ModemDisconnStatus = tmNow;

// 		GetNetInfo("usb0", &pSys_Rough->currNetWorkInfo.netInfo_usb0);
// 		//added by Jimmy 20200819: if IP not abtained, should dial again!!!
// 		if(pSys_Rough->currNetWorkInfo.netInfo_usb0.ulIpV4_IP == 0)
// 		{
// 			pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = NETPORT_STATUS_DISCONN;
// 		}
// 		else
// 		{
// 			pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status = Judge_IfNetwork_Connected("usb0") ? NETPORT_STATUS_CONNECTED : NETPORT_STATUS_DISCONN;
// 		}	
		
// 		if(NETPORT_STATUS_DISCONN == pSys_Rough->currNetWorkInfo.netInfo_usb0.emNetPort_Status)
// 		{
// 			//if WLAN is not accesable, then we need re-dial
// 			if(!pSys_Rough->bIsWWLAN_Available)
// 			{
// 				nUSB0DisConnectCounter++;
// 				if(nUSB0DisConnectCounter > 3) //if in 4*5s, we found the USB0 discon, then we need re-dial
// 				{
// 					LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
// 							"USB0 is NOT running, need re-dial!");
// 					nUSB0DisConnectCounter = -9; //wait 60sec for another notify
// 					pSys_Rough->emModemStatus = LTEMODEM_STATUS_WAIT_CONNECT;
// 				}
// 				return;
// 			}
// 		}
// 		else
// 		{
// 			nUSB0DisConnectCounter = 0;
// 			//nDialErrorCounter = 0;
// 		}

// 		//changed by Jimmy 20210413: changed interval for QueryModemSignal: 5 => 30s
// 		static int S_QUERYINTERV = 30;
// 		if(ABS(tmNow - pSys_Rough->tmLastQuerySignalStrength) < S_QUERYINTERV)
// 		{
// 			return;
// 		}		
// 		pSys_Rough->tmLastQuerySignalStrength = tmNow;

// 		//changed by Jimmy 20200905, in dialing process, there can be of NO response for a long time
// 		//found the case in Romania test 20200904, so change timeout 125sec !
// 		if(QueryModemSignal(pSys_Rough, TRUE))
// 		{
// 			S_QUERYINTERV = 30;
// 			if(ABS(tmNow - pSys_Rough->tmLastQueryNetworkTp) >= 120)//Jimmy changed 15 => 120
// 			{
// 				if(ReadATResp_FromModem("+ZPAS" , "?", szATResp, 5000) == 0)//Jimmy changed 500 => 5000
// 				{
// 					pSys_Rough->tmLastQueryNetworkTp = tmNow;
// 					TRACEX("NetType = [%s]!\n", szATResp);
// 					snprintf(pSys_Rough->currNetWorkInfo.lteModemINfo.szNetwork_Type, 32, "%s", szATResp);
// 				}
// 			}			
// 			//now set raw data to setting param
// 			Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 				1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 		}
// 		else
// 		{
// 			S_QUERYINTERV = 10; //changed by Jimmy when failed last time we need query it immediately after 10 seconds
// 			//now set raw data to setting param
// 			Push_BlobValue(SETTYPE_RAW_ST, pSys_Rough->hSampler, 
// 				1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
// 			return;
// 		}
// 	}
// 	else //need reset
// 	{
// 		_SYSTEM("/app/modem_reset sim0 &"); //now reset modem
// 		pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
// 		pSys_Rough->tmLastQueryInstallStatus = tmNow + 5;
// 	}
// }

static void Manage_LTEModem(SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bIsJigTestMode)
{
	Manage_InternalLTEModem(pSys_Rough, bIsJigTestMode);
	Manage_ExternalRouter(pSys_Rough);
}

static void Check_Keepalived_Status(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	VAR_VALUE varV;
	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	if (access(MASTER_CONFIG_FILE, F_OK) == 0)	
	{
		varV.emValue = 1;
		
	}
	else if (access(BACKUP_CONFIG_FILE, F_OK) == 0)	
	{
		varV.emValue = 2;
	}
	else
	{
		varV.emValue = 0;
	}

	Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 1, 34, varV);
}

//港华客户，并且在远程调试模式下，查询现在时间是否在远程调试时间段内，不在的话就切换到本地管理计划
static void RefreshEmsPlan(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;

	A_BLOB_EMS_CONFIG_PLAN EmsPlan1, EmsPlan2;
	A_BLOB_EMS_CONFIG_PLAN Ems1, Ems2;
	ZERO_POBJS(&EmsPlan1, 1);
	ZERO_POBJS(&EmsPlan2, 1);
	VAR_VALUE EmsCfg1, EmsCfg2;
	EmsCfg1.blobValue = (BYTE*)&EmsPlan1;
	EmsCfg2.blobValue = (BYTE*)&EmsPlan2;
	pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 201, &EmsCfg1, sizeof(A_BLOB_EMS_CONFIG_PLAN));
	pDPR->Get(pSampler, GETTYPE_ST_VAR, 200, 224, &EmsCfg2, sizeof(A_BLOB_EMS_CONFIG_PLAN));
	
	// printf("emControlMode = [%d]\n", pSys_Rough->emControlMode);
	int n = 0;
	BOOL Flag = FALSE;
	if(pSys_Rough->emControlMode == REMOTE_CTRL_MODE)
	{//远程计划版本号默认都是2102，本地管理计划版本号和远程不同都可以
		if(EmsPlan1.nEMSMgmt_Ver == 2102)
		{//当时版本号是远程计划，看看时间是否在远程计划内， 不在就切换管理计划
			for(n = 0; n < EmsPlan1.nTotal_EmsLmt_Sectors; ++n)
			{
				// printf("Ems1.emsLmtSectors[%d].tmStart = %ld, tmEndTime = %ld\n", n, 
				// 	EmsPlan1.emsLmtSectors[n].tmStart, EmsPlan1.emsLmtSectors_ext[n].tmEndTime);
				struct tm gmTime_Now;;
				gmtime_r(&EmsPlan1.emsLmtSectors_ext[n].tmEndTime, &gmTime_Now);

				gmTime_Now.tm_hour = 23;
				gmTime_Now.tm_min = 59;
				gmTime_Now.tm_sec = 59;
				time_t tmNewend = mktime(&gmTime_Now);	

				time_t tmNow = _NOW_;
				if((tmNow >= EmsPlan1.emsLmtSectors[n].tmStart) && (tmNow <= tmNewend))
				{//大于开始时间小于当天23：59:59的结束时间，表示在当前管理计划内
					Flag = TRUE; 
					break;
				}
			} 

			if(Flag == FALSE)
			{//现在本地计划是远程，但是当前时间段不在远程就切换管理计划
				Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 201, 
					(void*)&EmsPlan2, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
				Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 224, 
					(void*)&EmsPlan1, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
			}
		}
		else if(EmsPlan2.nEMSMgmt_Ver == 2102)
		{//当前运行的是本地计划，但是现在时间处于远程计划时间内，切换管理计划运行远程计划
			for(n = 0; n < EmsPlan2.nTotal_EmsLmt_Sectors; ++n)
			{
				// printf("Ems1.emsLmtSectors[%d].tmStart = %ld, tmEndTime = %ld\n", n, 
				// 	EmsPlan2.emsLmtSectors[n].tmStart, EmsPlan2.emsLmtSectors_ext[n].tmEndTime);
				struct tm gmTime_Now;;
				gmtime_r(&EmsPlan2.emsLmtSectors_ext[n].tmEndTime, &gmTime_Now);

				gmTime_Now.tm_hour = 23;
				gmTime_Now.tm_min = 59;
				gmTime_Now.tm_sec = 59;
				time_t tmNewend = mktime(&gmTime_Now);	

				time_t tmNow = _NOW_;
				if((tmNow >= EmsPlan2.emsLmtSectors[n].tmStart) && (tmNow <= tmNewend))
				{
					Flag = TRUE; 
					break;
				}
			}
			if(Flag == TRUE)
			{//当前在远程时间内，切换计划
				Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 201, 
					(void*)&EmsPlan2, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
				Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 224, 
					(void*)&EmsPlan1, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
			}
		}
	}
	else if(pSys_Rough->emControlMode == LOCAL_CTRL_MODE)
	{//本地模式下，但是计划还是远程计划，切换管理计划
		if(EmsPlan1.nEMSMgmt_Ver == 2102)
		{
			Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 201, 
					(void*)&EmsPlan2, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
			Push_BlobValue(SETTYPE_RAW_ST, pSampler, 200, 224, 
					(void*)&EmsPlan1, sizeof(A_BLOB_EMS_CONFIG_PLAN), BLOB_TYPE_EMS_CONFIG_PLAN);
		}
	}

	return 0;
}

/*********************************************************************************
*  FUNCTION NAME : RefreshSysAlarmStatus
*  CREATED DATE  : 2025-12-19 
*  DESCRIPTION   : 主要解决MBE告警一直无法消除问题，只有在系统启动时才会出现
***********************************************************************************/
static BOOL RefreshSysAlarmStatus(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	//系统线程启动15秒后，让MBE自己去刷新下当前告警状态，避免告警一直无法消除
	if((ABS(_NOW_ - pSys_Rough->t_LastRefreshAlamStatus) <= MAX_REFRESH_ALAM_INTERVAL) || (pSys_Rough->nRefreshTimes >= MAX_REFRESH_ALAM_TIMES))
	{
		return FALSE;
	}
	VAR_VALUE varV;
	if(SYSTYPE_EMS_STATION == pSys_Rough->lcCfg.emSystemType)
	{
		varV.emValue = 0;
		Push_VarValue(SETTYPE_INTERN_CTRL, (HANDLE)pSampler, 200, 253, varV);

	}

	pSys_Rough->nRefreshTimes++;
	pSys_Rough->t_LastRefreshAlamStatus = _NOW_;
	BOOL Flag = TRUE;
	SendMsg(pSys_Rough->hSampler, EVENT_TYPE_REFRESH_ALARM, EVENT_MSG_BEGIN,
					&Flag, sizeof(Flag));
	return TRUE;
}

static UINT s_GetFeeRateKWHDevMap[GetFeeRateKWHDevNum]={407, 401};
static BOOL RefreshCloudPlatformData(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	//判断下时间
	time_t t_Currrent = _NOW_;
	struct tm gmTime_Now;
	gmtime_r(&t_Currrent, &gmTime_Now);
	if(!(((gmTime_Now.tm_min == 29) || (gmTime_Now.tm_min == 59)) && (gmTime_Now.tm_sec == 59)) || (ABS(_NOW_ - pSys_Rough->tLastSaveTime) < 3))
	{//半个小时保存一段数据，一天48段数据       需要判断下，半小时内只保存一次，就不用再保存了，线程一秒内可能运行很多次
		return FALSE;
	}
	pSys_Rough->tLastSaveTime = _NOW_;   //59秒时只保存一次，
	// printf("保存数据大小 = %d", sizeof(A_BLOB_TYPE_DATASTORAGE));

	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	
	A_BLOB_ES_STATISTIC_DATA m_ESStatData;
	memset(&m_ESStatData, 0, sizeof(A_BLOB_ES_STATISTIC_DATA));
	varV.blobValue = (BYTE*)&m_ESStatData;
	pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR,
			200, 79, &varV, sizeof(A_BLOB_ES_STATISTIC_DATA));

	A_BLOB_TYPE_DATASTORAGE blobDataStorage;
	memset(&blobDataStorage, 0, sizeof(A_BLOB_TYPE_DATASTORAGE));
	
	blobDataStorage.t_SaveTime = t_Currrent;
	memcpy(&blobDataStorage.hours_Data, &m_ESStatData.es_Data.hours_Data, sizeof(ES_24HOUR_STAT_DATA));
	
	struct tm gmTime;
	gmtime_r(&t_Currrent, &gmTime);
    UINT nCurr_Day = gmTime.tm_mday - 1;
	if(nCurr_Day > MAX_DAYS_SECTOR)
	{
		// printf("nCurr_Day=[%d] is Outof Range, GoBack!\n");
		return FALSE;
	}

	blobDataStorage.days_Data.fAC_CurrKWH_Reading = m_ESStatData.es_Data.days_Data.fAC_CurrKWH_Reading[nCurr_Day];
	blobDataStorage.days_Data.fACTotalCostKWH_D = m_ESStatData.es_Data.days_Data.fACTotalCostKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fOrgACLoad_CostKWH_D = m_ESStatData.es_Data.days_Data.fOrgACLoad_CostKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fAC_RevrtOutPutKWH_D = m_ESStatData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_INPUTKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_OUTPUTKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fDCLoad_CostKWH_D = m_ESStatData.es_Data.days_Data.fDCLoad_CostKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Sharp_InputKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Peak_InputKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Flat_InputKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Valley_InputKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Sharp_OutPutKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Peak_OutPutKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Flat_OutputKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fLocalAC_Valley_OutputKWH_D = m_ESStatData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fOrgACLoad_Sharp_CostKWH_D = m_ESStatData.es_Data.days_Data.fOrgACLoad_Sharp_CostKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fOrgACLoad_Peak_CostKWH_D = m_ESStatData.es_Data.days_Data.fOrgACLoad_Peak_CostKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fOrgACLoad_Flat_CostKWH_D = m_ESStatData.es_Data.days_Data.fOrgACLoad_Flat_CostKWH_D[nCurr_Day];
	blobDataStorage.days_Data.fOrgACLoad_Valley_CostKWH_D = m_ESStatData.es_Data.days_Data.fOrgACLoad_Valley_CostKWH_D[nCurr_Day];
	int n = 0;
	for(n = 0; n < MAX_SINGLE_STBATT_NUM; ++n)
	{
		blobDataStorage.days_Data.fMpptTotalGKWH_D[n] = m_ESStatData.es_Data.days_Data.fMpptTotalGKWH_D[n][nCurr_Day];
		blobDataStorage.days_Data.fBattTotalChrgKWH_D[n] = m_ESStatData.es_Data.days_Data.fBattTotalChrgKWH_D[n][nCurr_Day];
		blobDataStorage.days_Data.fBattTotalDisChrgKWH_D[n] = m_ESStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[n][nCurr_Day];
		blobDataStorage.days_Data.fBattChrgTimes_D[n] = m_ESStatData.es_Data.days_Data.fBattChrgTimes_D[n][nCurr_Day];
		blobDataStorage.days_Data.fBattDisChrgTimes_D[n] = m_ESStatData.es_Data.days_Data.fBattDisChrgTimes_D[n][nCurr_Day];
		blobDataStorage.days_Data.fMppt_Sharp_GKWH_D[n] = m_ESStatData.es_Data.days_Data.fMppt_Sharp_GKWH_D[n][nCurr_Day];
		blobDataStorage.days_Data.fMppt_Peak_GKWH_D[n] = m_ESStatData.es_Data.days_Data.fMppt_Peak_GKWH_D[n][nCurr_Day];
		blobDataStorage.days_Data.fMppt_Flat_GKWH_D[n] = m_ESStatData.es_Data.days_Data.fMppt_Flat_GKWH_D[n][nCurr_Day];
		blobDataStorage.days_Data.fMppt_Valley_GKWH_D[n] = m_ESStatData.es_Data.days_Data.fMppt_Valley_GKWH_D[n][nCurr_Day];
	}
	
	UINT nWhichWh = 0, nDev = 0, nTimeNum = 0;
	for(nWhichWh = 0; nWhichWh < GetFeeRateKWHDevNum; nWhichWh++)
	{
		nDev = s_GetFeeRateKWHDevMap[nWhichWh];
		for (nTimeNum = 0; nTimeNum < GetFeeRateKWHTimeNum; nTimeNum++)
		{
			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1491 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fPosSumKwh[nTimeNum][0] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1492 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fPosSharpKwh[nTimeNum][1] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1493 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fPosPeakKwh[nTimeNum][2] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1494 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fPosFlatKwh[nTimeNum][3] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1495 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fPosWaveKwh[nTimeNum][4] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1496 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fNegSumKwh[nTimeNum][5] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1497 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fNegSharpKwh[nTimeNum][6] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1498 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fNegPeakKwh[nTimeNum][7] = varV.fValue;
			
			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1499 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fNegFlatKwh[nTimeNum][8] = varV.fValue;

			pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR, nDev, 1500 + nTimeNum * 10, &varV, 4);
			blobDataStorage.feeRateKWH[nWhichWh].fNegWaveKwh[nTimeNum][9] = varV.fValue;
		}
	}
	Save_CloudToDb(t_Currrent, &blobDataStorage, sizeof(A_BLOB_TYPE_DATASTORAGE));
	
	return TRUE;
}

DLLExport int Sample(HANDLE hSamplerThis)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	SYSTEM_SAMPLER_DATA *pSys_Rough = (SYSTEM_SAMPLER_DATA*)pSampler->pDataHeap;
	VAR_VALUE varV;

	int nErr = Mutex_Lock(pSys_Rough->hSamplerLock, 100);
	if(nErr != ERR_MUTEX_OK)
	{
		return ERR_SAMPLE_NEED_RESAMP;
	}
	
	Manage_LTEModem(pSys_Rough,FALSE); //manage modem

	//Jimmy note: in case DHCP status can be changed unexpectedly, here we refresh status in every 5 seconds
	//if(pSys_Rough->currNetWorkInfo.bDHCP_On_eth0 || pSys_Rough->bNeedRefreshNetInfo)
	{
		static time_t s_tmLastRefreshNetwork = 0;
		time_t tmNow = _NOW_;
		if(ABS(tmNow - s_tmLastRefreshNetwork) >= 5)
		{
			s_tmLastRefreshNetwork = tmNow;
			pSys_Rough->bNeedRefreshNetInfo = FALSE;
			Refresh_NetworkInfo(pSampler, FALSE);
		}
	}

	//added by Jimmy 20200803: refresh WLAN status
	if(pSys_Rough->bIsWWLAN_Available != pSys_Rough->currNetWorkInfo.bIsWLAN_Reachable)
	{
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
			"The WLAN connection status became => %s!",
			pSys_Rough->bIsWWLAN_Available ? "AVAILABLE" : "UNAVAILABLE");
		pSys_Rough->currNetWorkInfo.bIsWLAN_Reachable = pSys_Rough->bIsWWLAN_Available;
		Push_BlobValue(SETTYPE_RAW_ST, (HANDLE)pSampler, 
			1, 3, (void*)&pSys_Rough->currNetWorkInfo, sizeof(A_BLOB_NETWORKINFO), BLOB_TYPE_NETWORKPARAMS);
	}

	RefreshSysAvailableStatus(pSys_Rough);
	if(pSys_Rough->emCustomType == GH_CUSTOM)
	{
		RefreshEmsPlan(pSys_Rough);
	}
	
	RefreshSysAlarmStatus(pSys_Rough);

	RefreshCloudPlatformData(pSys_Rough);

	if(pSys_Rough->emDownloadStatus != UPDOWN_IDLE)
	{
		HandleFirmwareUpgrading(pSys_Rough);
	}

	if(pSys_Rough->emUploadStatus != UPDOWN_IDLE)
	{
		HandleDiagnoseUploadiing(pSys_Rough);
	}

	//refresh system DataTime every sec
	TRACE("Now refresh system datatime...\n");	
	varV.ulValue = (UINT)_NOW_;
	Push_VarValue(SETTYPE_RAW_ST, (HANDLE)pSampler, 1, 1, varV);

	//refresh system work total min
	static int s_nLastTime_Refresh = 0;
	if(s_nLastTime_Refresh == 0)
	{
		s_nLastTime_Refresh = _NOW_;//pSys_Rough->tmPowerOn;
	}
	else
	{
		time_t tmNow = _NOW_;
		int nDiff = ABS(tmNow - s_nLastTime_Refresh);
		if(nDiff <= 5)
		{
			pSys_Rough->fRunSincePowerOn_Min += (float)nDiff / 60;
			varV.fValue = pSys_Rough->fRunSincePowerOn_Min; //(float)(_NOW_ - pSys_Rough->tmPowerOn) / 60;
			Push_VarValue(SETTYPE_RAW_SP, (HANDLE)pSampler, 1, 1, varV);
		}
		s_nLastTime_Refresh = tmNow; //in case system time is changed
	}

	//Jimmy added 20200117: do can reset incase of system hang
	static time_t s_tmLastResetCanbus = 0;
	static time_t s_tmLastSearchSegFault = 0;
	time_t tmNow = _NOW_;
	BOOL bFoundError = FALSE;
	//Jimmy 20200730: NO use!!!
	// static time_t s_tmLastFoundErr = 0;
	// if(s_tmLastResetCanbus == 0)
	// {
	// 	s_tmLastResetCanbus = tmNow;
	// 	s_tmLastSearchSegFault = tmNow;
	// }	
	// //Jimmy added for capturing the system hang-up
	// else if((ABS(tmNow - s_tmLastSearchSegFault) >= 5) && //capture it in every 5 sec
	// 		(ABS(tmNow - s_tmLastFoundErr) >= 5 * 60)) //record in every 5 min
	// {
	// 	s_tmLastSearchSegFault = tmNow;
	// 	FILE *f = popen("dmesg | grep 'Stack: (0x'", "r");
	// 	char szBuff[64];
	// 	memset(szBuff, 0, 64);		
	// 	if(f)
	// 	{
	// 		memset(szBuff, 0, 64);
	// 		fread(szBuff, 64, 1, f);
	// 		//printf("Readout dmeg for Stack[%d]\n", strlen(szBuff));
	// 		if(strlen(szBuff) > 1)
	// 		{
	// 			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
	// 				"STACK Found:[%s]!", szBuff);					
	// 			bFoundError = 11;
	// 		}
	// 		pclose(f);	
	// 	}
	// 	if(!bFoundError)
	// 	{
	// 		f = popen("dmesg | grep 'Backtrace:'", "r");
	// 		if(f)
	// 		{
	// 			memset(szBuff, 0, 64);
	// 			fread(szBuff, 64, 1, f);
	// 			//printf("Readout dmeg for back-trace[%d]\n", strlen(szBuff));
	// 			if(strlen(szBuff) > 1)
	// 			{
	// 				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
	// 					"TRACE Found:[%s]!", szBuff);	
	// 				bFoundError = 22;
	// 			}
	// 			pclose(f);	
	// 		}
	// 	}

	// 	if(bFoundError)
	// 	{
	// 		s_tmLastFoundErr = tmNow;
	// 		//now store the error into error file, need clear the log to avoid repeatedly rec
	// 		sprintf(szBuff, "dmesg > %d.err; dmesg -C", _NOW_);
	// 		//use raw system call so that won't be impacted by syshand
	// 		system(szBuff);
	// 	}
	// }

	if((ABS(tmNow - s_tmLastResetCanbus) >= 3600) || (bFoundError)) //reset it in every 60min
	{
		TRACEX("-------Now reset can for IMMU2 in case of hang----\n");
		//Jimmy added 20200506 here, for monitoring the system CPU usage and find root-cause of crash
		FILE *f = popen("top -n 1 | awk 'NR==2' | tr -d ' \n'", "r");
		if(f)
		{
			char szBuff[64];
			memset(szBuff, 0, 64);
			fread(szBuff, 48, 1, f);
			LogOut4c(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
					"CPU usage:[%s]!", szBuff);	
			pclose(f);	
			//now get memory usage also
			f = popen("free | awk 'NR==2' | tr -d '\n'", "r");
			if(f)
			{
				memset(szBuff, 0, 64);
				fread(szBuff, 48, 1, f);
				LogOut4c(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
					"Memory usage:[%s]!", szBuff);	
				pclose(f);	
			}
		}

		s_tmLastResetCanbus = tmNow;
		//Jimmy removed 20200718, do not do this again
		//_SYSTEM("canconfig can0 stop; canconfig can0 start");
		//_SYSTEM("canconfig can1 stop; canconfig can1 start");
	}

	Check_Keepalived_Status(pSys_Rough);

	if(pSys_Rough->bUpdFlag == TRUE)
	{
		BatSideupgrade(pSys_Rough);
		ModSideupgrade(pSys_Rough);
		if(refreshstatus(pSys_Rough) == FALSE)
		{
			pSys_Rough->bUpdFlag = FALSE;
		}
	}

	Mutex_Unlock(pSys_Rough->hSamplerLock);

	return ERR_SAMPLE_OK;
}

/*
static BOOL Handle_WebUpdate_IMMU2Firmware(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	//if another downloading is in-process, we need wait
	if((pSys_Rough->emDownloadStatus != UPDOWN_IDLE) &&
		(pSys_Rough->emDownloadStatus != UPDOWN_WAIT_REBOOT_AND_CCU))
	{
		return FALSE; 
	}
	//now extract pkg and check it
	char szCmd[256];
	sprintf(szCmd, "mkdir -p /opt/appdata/data/firm_tmp_extract; tar -zxf %s -C /opt/appdata/data/firm_tmp_extract/", 
			IMMU2_FIRMWARE_WEB_IMPORT_FILENAME);
	_SYSTEM(szCmd);
	if((access("/opt/appdata/data/firm_tmp_extract/app_nccu.tar.gz", R_OK) != 0) ||
		(access("/opt/appdata/data/firm_tmp_extract/pkg.sum", R_OK) != 0)) //extracted pkg file and pkg.sum existed??
	{
		TRACEX("----------No pkg. and checksum file found-------\n");
		_SYSTEM("rm -rf /opt/appdata/data/firm_tmp_extract/ *");
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"Wrong pkg found for WEB firmware upgrade of IMMU2!");
		//SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
		//	&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
		return FALSE;
	}
	if(!DoMD5Chk_ForRemotePkg(TRUE))
	{
		_SYSTEM("rm -rf /opt/appdata/data/firm_tmp_extract/ *");
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"MD5 chk FAILED for WEB firmware upgrade of IMMU2!");
		//SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
		//	&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
		return FALSE;
	}
	//now overwrite old app dir??
	_SYSTEM("tar -zxf /opt/appdata/data/firm_tmp_extract/app_nccu.tar.gz -C /app;sync"); //Jimmy added 20190529: should call 'SYNC' 
																	//because UBI fs will not save data to flash 
																	//if controller lost power
	// //for IMSU-D update
	// if(access("/opt/appdata/data/IMSU_APP.bin", R_OK) == 0)
	// {
	// 	_SYSTEM("cp /opt/appdata/data/IMSU_APP.bin /app -f;sync");
	// }
	//evFirmUpdt.emUpdateStatus = FIRMUPDATE_Installing;
	//SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_STATUS_CHANGED, 
	//	&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
	//Sleep(2000);
	// DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
	// for(n = 0; n < 3; n++)
	// {
	// 	pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR,
	// 		201 + n, 101, &varV, 4);
	// 	if((varV.emValue >= GUN_STATE_RESERVE_PENDING) &&
	// 		(varV.emValue <= GUN_STATE_WAITING_END))
	// 	{
	// 		pSys_Rough->emDownloadStatus = UPDOWN_WAIT_REBOOT;
	// 		return;
	// 	}
	// }

	//judge if has CCU firmware & wait CCU upgrade
	if(IsInclued_CCUFimrwarePkg(pSys_Rough))
	{
		pSys_Rough->emDownloadStatus = UPDOWN_WAIT_REBOOT_AND_CCU;
		pSys_Rough->tmLast_EnterWaitCCUUpgrade = _NOW_;
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
			"Parsed firmware SUCCESSFULLY from WEB and wait CCU to be upgraded!");
		return TRUE;
	}

	//if proper time, then reboot system
	EVENT_SYSTEM_EXIT evExit;
	evExit.nExitType = SYS_EXIT_FIRMWAREUPDT;
	SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
		&evExit, sizeof(EVENT_SYSTEM_EXIT));
	LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
		"Parsed firmware SUCCESSFULLY from WEB and will reboot soon!");
	
	return TRUE;
}
*/
static BOOL USB_SafelyWrite(char *pMakeFileBuf,int istrlen, int istep, FILE *fp)
{
	char szPathDev[64];
 	char* szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, NULL, szPathDev, 64);					
	if(szPath == NULL)
	{
		szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, NULL, szPathDev, 64);			
	}

	if(szPath != NULL)
	{
		fwrite(pMakeFileBuf,istrlen, istep, fp);
		return TRUE;
	}
 
	printf(" Write File Break,USB not exist!!!");
	return FALSE;
}

static void WriteHeader2Html(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp)
{
	char	szFileTitle[1024];
	memset(szFileTitle, 0, sizeof(szFileTitle));
	// 1 full title
	snprintf(szFileTitle, sizeof(szFileTitle),
		"<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
		"<head><meta	http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /></head><body>\n"
		"<style>body,html{padding:10px;margin:0;font-size:14px;line-height:25px;font-family:Arial;}table{border-left:#ddd solid 1px;border-top:#ddd solid 1px;margin-top:20px;}table td{border-right:#ddd solid 1px;border-bottom:#ddd solid 1px;height:30px;line-height:30px;padding:0 0 0 10px;font-size:14px;color:#333}table tr:first-child td{font-weight:bold;height:40px;line-height:40px;}</style>\n"\
		);
	
 	USB_SafelyWrite(szFileTitle,strlen(szFileTitle), 1, fp);

	return;
}

static void WriteEnd2Html(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp)
{
	char szFileTitle[64];
	memset(szFileTitle, 0, strlen(szFileTitle));
	snprintf(szFileTitle, 64, "\t</body>\n</html>\n");
	USB_SafelyWrite(szFileTitle, strlen(szFileTitle), 1, fp);
	return;
}

static void WriteHeader2Table(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp, HISDATA_TABLETYPE emTableType)
{
	char	szFileTitle[1024];
	memset(szFileTitle, 0, sizeof(szFileTitle));

	char szTableName[][20]={{"Hour Statistic KWH"}, {"Daily Statistic KWH"}, {"Yearly Statistic Kwh"}};

	//2 new a table for Chagrge log
	if(emTableType == TP_HISDATA_WEEKHOURS)
	{
		snprintf(szFileTitle, sizeof(szFileTitle),
		"<h2 align=\"center\">%20s</h2>\n" 
		"<table border=\"2\" width=\"100%%\" cellpadding=\"0\" cellspacing=\"0\">\n" 
		"\n<tr><td>%18s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%18s</td> \t<td>%18s</td> \t<td>%18s</td> \t<td>%18s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> </tr>\n", 
		szTableName[emTableType],
		"Date",
		"User ACLoad(Used)",
		"User ACMeter IN/OUT",
		"Cab ACMeter IN/OUT",
		"PV#1/#2/#3/#4",
 		"BAT#1 Chrg/Dischrg",
		"BAT#2 Chrg/Dischrg",
		"BAT#3 Chrg/Dischrg", 
		"BAT#4 Chrg/Dischrg",
		"1KV DCLoad(Used)KWH",
		"User ACMeter RealTime");
	}
	else if((emTableType == TP_HISDATA_DAILY) || (emTableType == TP_HISDATA_YEARLY))
	{
	 	snprintf(szFileTitle, sizeof(szFileTitle),
		"<h2 align=\"center\">%20s</h2>\n" 
		"<table border=\"2\" width=\"100%%\" cellpadding=\"0\" cellspacing=\"0\">\n" 
		"\n<tr><td>%18s</td> \t<td>%8s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%18s</td> \t<td>%18s</td> \t<td>%18s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> </tr>\n", 
		szTableName[emTableType],
		"Date",
		"User ACLoad(Used)",
		"User ACMeter IN/OUT",
		"Cab ACMeter IN/OUT",
 		"PV#1/#2/#3/#4",
 		"Cab ACMeter Sharp/Peak/Flat/Valley IN",
		"Cab ACMeter Sharp/Peak/Flat/Valley OUT",
		"BAT#1 Chrg/Dischrg",
		"BAT#2 Chrg/Dischrg",
		"BAT#3 Chrg/Dischrg", 
		"BAT#4 Chrg/Dischrg",
		"1KV DCLoad(Used)KWH",
		"User ACMeter RealTime");
	}
 	USB_SafelyWrite(szFileTitle, strlen(szFileTitle), 1, fp);

	TRACE("Write Content To Table szTableName[%d]=%s\n", emTableType, szTableName[emTableType]);

	return;
}

/*
static void WriteHeader2Table(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp, HISDATA_TABLETYPE emTableType)
{
	char	szFileTitle[1024];
	memset(szFileTitle, 0, sizeof(szFileTitle));

	char szTableName[][20]={{"时段电量统计"}, {"日电量统计"}, {"年电量统计"}};

	//2 new a table for Chagrge log
	snprintf(szFileTitle, sizeof(szFileTitle),
		"<h2 align=\"center\">%20s</h2>\n" 
		"<table border=\"2\" width=\"100%\" cellpadding=\"0\" cellspacing=\"0\">\n" 
		"\n<tr><td>%8s</td> \t<td>%18s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%12s</td> \t<td>%-12s</td> \t<td>%-12s</td> </tr>\n", 
		szTableName[emTableType],
		"编号",
		"时间",
		"电网AC(实时)KWH",
		"电网AC(消耗)KWH",
		"电网AC(输出)KWH",
		"柜内AC(消耗)KWH",
		"柜内AC(输出)KWH",
		"企业负载(消耗)KWH",
		"DC负载(消耗)KWH",
		"MPPT_1(放)KWH",
		"MPPT_2(放)KWH",
		"电池组1(充)KWH",
		"电池组1(放电)KWH",
		"电池组2(充)KWH", 
		"电池组2(放电)KWH");

 	USB_SafelyWrite(szFileTitle, strlen(szFileTitle), 1, fp);

	TRACE("Write Content To Table szTableName[%d]=%s\n", emTableType, szTableName[emTableType]);

	return;
}
*/

 static void WriteContent2Table(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp, HISDATA_TABLETYPE emTableType)
{
	#define MAX_SPRINTF_LINE_LEN	512
	#define STEP_RECORD				20

	SAMPLER* pSampler = (SAMPLER*)pSys_Rough->hSampler;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE 	varV;

	char szTime[32]={0};
	char szFileTitle[MAX_SPRINTF_LINE_LEN*STEP_RECORD];
	memset(szFileTitle, 0, sizeof(szFileTitle));
	
	//1.Get ES Statistic History Data
	A_BLOB_ES_STATISTIC_DATA m_ESStatData;
	varV.blobValue = (BYTE*)&m_ESStatData;
	pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR,
			200, 79, &varV, sizeof(A_BLOB_ES_STATISTIC_DATA));
	
 	int n = 0, m = 0;
	int nFileLen = 0;
	int nStepCount = 0;

	//int nWeekHourCount = 0;
	int nDailyCount  = 0 ;
	int nYearlyCount = 0;

	int nHourSetor = 0, nMinSector = 0;
    int nHourAdd1_Sector = 0, nMinAdd1_Sector = 0;


	if(emTableType == TP_HISDATA_WEEKHOURS)
	{
        for(m = 0; m < MAX_HOURS_SECTOR; m++)
        {
            time_t saveTime = m_ESStatData.es_Data.tmTimeSave;
			if(saveTime != 0)
			{
				TimeToString(saveTime, "%Y-%m-%d ", szTime, sizeof(szTime));
				
				nStepCount++;

				TRACE("n=%d m=%d szTime=%s nStepCount=%d nFileLen=%d\n", n, m, szTime, nStepCount, nFileLen);
				nHourSetor = m/2; 
				nMinSector = m%2;
    			
				nHourAdd1_Sector = (m+1)/2; 
				nMinAdd1_Sector = (m+1)%2;

				nFileLen += snprintf(&szFileTitle[nFileLen], MAX_SPRINTF_LINE_LEN, "<tr><td>%s %02d:%02d~%02d:%02d</td> \t<td>%.1f</td> \t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f|%.1f|%.1f</td>\t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f</td> \t<td>%.1f</td> </tr>\n", \
				szTime,\
				nHourSetor,\
				nMinSector*30,\
				nHourAdd1_Sector,\
				nMinAdd1_Sector*30,\
				m_ESStatData.es_Data.hours_Data.fOrgACLoad_CostKWH_Sector[m],\
 				m_ESStatData.es_Data.hours_Data.fAC_CostKWH_Sector[m],\
				 m_ESStatData.es_Data.hours_Data.fAC_RevrtOutPutKWH_Sector[m],\
				m_ESStatData.es_Data.hours_Data.fLocalAC_INPUTKWH_Sector[m],\
				m_ESStatData.es_Data.hours_Data.fLocalAC_OUTPUTKWH_Sector[m],\
				m_ESStatData.es_Data.hours_Data.fMppt_GKWH_Sector[0][m],\
				m_ESStatData.es_Data.hours_Data.fMppt_GKWH_Sector[1][m],\
				m_ESStatData.es_Data.hours_Data.fMppt_GKWH_Sector[2][m],\
				m_ESStatData.es_Data.hours_Data.fMppt_GKWH_Sector[3][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[0][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[0][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[1][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[1][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[2][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[2][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_ChrgKWH_Sector[3][m],\
				m_ESStatData.es_Data.hours_Data.fBatt_DisChrgKWH_Sector[3][m],\
				m_ESStatData.es_Data.hours_Data.fDCLoad_CostKWH_Sector[m],\					
				m_ESStatData.es_Data.hours_Data.fAC_CurrKWH_Reading[m]);
	
				if(nStepCount >= STEP_RECORD)
				{
					TRACE("nStepCount =%d nFileLen=%d\n", nStepCount, nFileLen);
					nStepCount = 0;
					nFileLen = 0;
					USB_SafelyWrite(szFileTitle, strlen(szFileTitle),1, fp);
					memset(szFileTitle, 0, sizeof(szFileTitle));
				}
			}
		}

		TRACE("---USB_SafelyWrite Weekly Hours Data---\n");
		USB_SafelyWrite(szFileTitle, strlen(szFileTitle),1, fp);
		
 	}
	else if(emTableType == TP_HISDATA_DAILY)
	{
		for(n = 0; n < MAX_DAYS_SECTOR; n++)
        {
            time_t saveTime = m_ESStatData.es_Data.days_Data.tmTimeStamp_D[n];
			if(saveTime != 0)
			{
				TimeToString(saveTime, TIME_CHN_FMT, szTime, sizeof(szTime));
 				//nDailyCount++;
				nStepCount++;

				//TRACE("n=%d m=%d szTime=%s nDailyCount=%d nStepCount=%d nFileLen=%d\n",  n, m, szTime, nDailyCount, nStepCount, nFileLen);

				nFileLen += snprintf(&szFileTitle[nFileLen], MAX_SPRINTF_LINE_LEN, "<tr><td>%s</td> \t<td>%.1f</td> \t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f|%.1f|%.1f</td>\t<td>%.1f|%.1f|%.1f|%.1f</td> \t<td>%.1f|%.1f|%.1f|%.1f</td>\t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f</td> \t<td>%.1f</td> </tr>\n", \
				szTime,\
				m_ESStatData.es_Data.days_Data.fOrgACLoad_CostKWH_D[n],\
 				m_ESStatData.es_Data.days_Data.fACTotalCostKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fAC_RevrtOutPutKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_INPUTKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_OUTPUTKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fMpptTotalGKWH_D[0][n],\
				m_ESStatData.es_Data.days_Data.fMpptTotalGKWH_D[1][n],\
				m_ESStatData.es_Data.days_Data.fMpptTotalGKWH_D[2][n],\
				m_ESStatData.es_Data.days_Data.fMpptTotalGKWH_D[3][n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Sharp_InputKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Peak_InputKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Flat_InputKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Valley_InputKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Sharp_OutPutKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Peak_OutPutKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Flat_OutputKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fLocalAC_Valley_OutputKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fBattTotalChrgKWH_D[0][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[0][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalChrgKWH_D[1][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[1][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalChrgKWH_D[2][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[2][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalChrgKWH_D[3][n],\
				m_ESStatData.es_Data.days_Data.fBattTotalDisChrgKWH_D[3][n],\
				m_ESStatData.es_Data.days_Data.fDCLoad_CostKWH_D[n],\
				m_ESStatData.es_Data.days_Data.fAC_CurrKWH_Reading[n]);

				if(nStepCount >= STEP_RECORD)
				{
					TRACE("nStepCount =%d nFileLen=%d\n", nStepCount, nFileLen);
					nStepCount = 0;
					nFileLen = 0;
					USB_SafelyWrite(szFileTitle, strlen(szFileTitle),1, fp);
					memset(szFileTitle, 0, sizeof(szFileTitle));
				}
			}	
		}
		TRACE("---USB_SafelyWrite Daily Data---\n");
		USB_SafelyWrite(szFileTitle, strlen(szFileTitle),1, fp);
	}
	else if(emTableType == TP_HISDATA_YEARLY)
	{
		for(n = 0; n < MAX_YEARS_SECTOR; n++)
        {
            time_t saveTime = m_ESStatData.es_Data.years_Data.tmTimeStamp_Y[n];
			if(saveTime != 0)
			{
				TimeToString(saveTime, TIME_CHN_FMT, szTime, sizeof(szTime));
				
				//nYearlyCount++;
				nStepCount++;

				TRACE("n=%d m=%d szTime=%s nYearlyCount=%d nStepCount=%d nFileLen=%d\n",  n,m, szTime, nYearlyCount, nStepCount, nFileLen);

				nFileLen += snprintf(&szFileTitle[nFileLen], MAX_SPRINTF_LINE_LEN, "<tr><td>%s</td> \t<td>%.1f</td> \t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f</td>\t<td>%.1f|%.1f|%.1f|%.1f</td>\t<td>%.1f|%.1f|%.1f|%.1f</td>\t<td>%.1f|%.1f|%.1f|%.1f</td>\t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f|%.1f</td> \t<td>%.1f</td> \t<td>%.1f</td> </tr>\n", \
				szTime,\
				m_ESStatData.es_Data.years_Data.fOrgACLoad_CostKWH_Y[n],\
 				m_ESStatData.es_Data.years_Data.fACTotalCostKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fAC_RevrtOutPutKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_INPUTKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_OUTPUTKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fMpptTotalGKWH_Y[0][n],\
				m_ESStatData.es_Data.years_Data.fMpptTotalGKWH_Y[1][n],\
				m_ESStatData.es_Data.years_Data.fMpptTotalGKWH_Y[2][n],\
				m_ESStatData.es_Data.years_Data.fMpptTotalGKWH_Y[3][n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Sharp_InputKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Peak_InputKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Flat_InputKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Valley_InputKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Sharp_OutPutKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Peak_OutPutKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Flat_OutputKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fLocalAC_Valley_OutputKWH_Y[n],\
				m_ESStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[0][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[0][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[1][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[1][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[2][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[2][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalChrgKWH_Y[3][n],\
				m_ESStatData.es_Data.years_Data.fBattTotalDisChrgKWH_Y[3][n],\
				m_ESStatData.es_Data.years_Data.fDCLoad_CostKWH_Y[n],\	
				m_ESStatData.es_Data.years_Data.fAC_CurrKWH_Reading[n]);
	
				if(nStepCount >= STEP_RECORD)
				{
					TRACE("nStepCount =%d nFileLen=%d\n", nStepCount, nFileLen);
					nStepCount = 0;
					nFileLen = 0;
					USB_SafelyWrite(szFileTitle, strlen(szFileTitle),1, fp);
					memset(szFileTitle, 0, sizeof(szFileTitle));
				}
			}	
		}
		TRACE("---USB_SafelyWrite Yearly Data---\n");
		USB_SafelyWrite(szFileTitle, strlen(szFileTitle),1, fp);
	}

	return;
}

static void WriteEnd2Table(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp, HISDATA_TABLETYPE emTableType)
{
	char szFileTitle[64];
	memset(szFileTitle, 0, strlen(szFileTitle));
	snprintf(szFileTitle, 64, "\t</table><br/>\n");
	USB_SafelyWrite(szFileTitle, strlen(szFileTitle), 1, fp);
	return;
}



static void	WriteWeekly_Table2Html(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp)
{
	WriteHeader2Table(pSys_Rough, fp, TP_HISDATA_WEEKHOURS);
	WriteContent2Table(pSys_Rough, fp, TP_HISDATA_WEEKHOURS);
	WriteEnd2Table(pSys_Rough, fp, TP_HISDATA_WEEKHOURS);
	return;
}

static void	WriteDaily_Table2Html(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp)
{
	WriteHeader2Table(pSys_Rough, fp, TP_HISDATA_DAILY);
	WriteContent2Table(pSys_Rough, fp, TP_HISDATA_DAILY);
	WriteEnd2Table(pSys_Rough, fp, TP_HISDATA_DAILY);

	return;
}

static void WriteYearly_Table2Html(SYSTEM_SAMPLER_DATA *pSys_Rough, FILE *fp)
{
	WriteHeader2Table(pSys_Rough, fp, TP_HISDATA_YEARLY);
	WriteContent2Table(pSys_Rough, fp, TP_HISDATA_YEARLY);
	WriteEnd2Table(pSys_Rough, fp, TP_HISDATA_YEARLY);
	return;
}
/*********************************************************************************
*  
*  FUNCTION NAME : Exported_HisStatisData
*  CREATOR       :  
*  CREATED DATE  : 2020-11-21 14:33:53
*  DESCRIPTION   :  
*  
*  CHANGE HISTORY: 
*	NOTE: If Not Close File FP, the File would be Empty
*	  
*  
***********************************************************************************/
static BOOL Exported_HisStatisData(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	// 1. Create  file 
	char szLogFile[128];
	memset(szLogFile, 0, 128);

	time_t	tmNow  = _NOW_;
	struct	tm gmTime;
	gmtime_r(&tmNow, &gmTime);

	char szTime[32];

	TimeToString(time(NULL), "%y%m%d_%H%M%S", szTime, sizeof(szTime));

	char szPathDev[64];
 	char* szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, NULL, szPathDev, 64);					
	if(szPath == NULL)
	{
		szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, NULL, szPathDev, 64);			
	}
  
	if(szPath != NULL)
	{
		snprintf(szLogFile, 128, "%s/SolarBackup_HisData_%s.html",szPath, szTime);
	}
	else
	{
		return FALSE;
	}

	FILE *fp = fopen(szLogFile, "wb");
	if(fp == NULL)
	{
		return FALSE;
	}

	WriteHeader2Html(pSys_Rough, fp);

	WriteYearly_Table2Html(pSys_Rough, fp);

	WriteDaily_Table2Html(pSys_Rough, fp);

	WriteWeekly_Table2Html(pSys_Rough, fp);

	WriteEnd2Html(pSys_Rough, fp);

	if(fp != NULL)
	{
		fclose(fp);
		fp = NULL;
	}
	return TRUE;
}
static BOOL Handle_WebUpdate_IMMU2Firmware(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	//now extract pkg and check it
	char szCmd[256];
	sprintf(szCmd, "mkdir -p /opt/appdata/data/firm_tmp_extract; tar -zxf %s -C /opt/appdata/data/firm_tmp_extract/", 
			IMMU2_FIRMWARE_WEB_IMPORT_FILENAME);
	_SYSTEM(szCmd);
	if((access("/opt/appdata/data/firm_tmp_extract/app_nccu.tar.gz", R_OK) != 0) ||
		(access("/opt/appdata/data/firm_tmp_extract/pkg.sum", R_OK) != 0)) //extracted pkg file and pkg.sum existed??
	{
		TRACEX("----------No pkg. and checksum file found-------\n");
		_SYSTEM("rm -rf /opt/appdata/data/firm_tmp_extract/*");
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"Wrong pkg found for WEB firmware upgrade of IMMU2!");
		//SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
		//	&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
		return FALSE;
	}
	if(!DoMD5Chk_ForRemotePkg(TRUE))
	{
		_SYSTEM("rm -rf /opt/appdata/data/firm_tmp_extract/*");
		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
				"MD5 chk FAILED for WEB firmware upgrade of IMMU2!");
		//SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_END, 
		//	&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
		return FALSE;
	}
	//now overwrite old app dir??
	_SYSTEM("tar -zxf /opt/appdata/data/firm_tmp_extract/app_nccu.tar.gz -C /app;sync"); //Jimmy added 20190529: should call 'SYNC' 
																	//because UBI fs will not save data to flash 
																	//if controller lost power
	// //for IMSU-D update
	// if(access("/opt/appdata/data/IMSU_APP.bin", R_OK) == 0)
	// {
	// 	_SYSTEM("cp /opt/appdata/data/IMSU_APP.bin /app -f;sync");
	// }
	//evFirmUpdt.emUpdateStatus = FIRMUPDATE_Installing;
	//SendMsg(pSys_Rough->hSampler, EVENT_TYPE_FIRMWARE_UPDATE, EVENT_MSG_STATUS_CHANGED, 
	//	&evFirmUpdt, sizeof(EVENT_FIRMWAREUPDATE_STATUS));
	//Sleep(2000);
	// DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)((SAMPLER*)pSys_Rough->hSampler)->hDPR;
	// for(n = 0; n < 3; n++)
	// {
	// 	pDPR->Get(pSys_Rough->hSampler, GETTYPE_SP_VAR,
	// 		201 + n, 101, &varV, 4);
	// 	if((varV.emValue >= GUN_STATE_RESERVE_PENDING) &&
	// 		(varV.emValue <= GUN_STATE_WAITING_END))
	// 	{
	// 		pSys_Rough->emDownloadStatus = UPDOWN_WAIT_REBOOT;
	// 		return;
	// 	}
	// }
	//if proper time, then reboot system
	EVENT_SYSTEM_EXIT evExit;
	evExit.nExitType = SYS_EXIT_FIRMWAREUPDT;
	SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
		&evExit, sizeof(EVENT_SYSTEM_EXIT));
	LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO,
		"Parsed firmware SUCCESSFULLY from WEB and will reboot soon!");
	
	return TRUE;
}

static BOOL Handle_WebOneClick_Upgrade(SYSTEM_SAMPLER_DATA *pSys_Rough)
{
	char szCmd[256];
	//remove previous upgrade file
	sprintf(szCmd, "rm -rf /opt/appdata/data/upgradefile");
	_SYSTEM(szCmd);

	sprintf(szCmd, "unzip %s -d /opt/appdata/data/", 
			ONECLICK_FIRMWARE_WEB_IMPORT_FILENAME);
	_SYSTEM(szCmd);

	return TRUE;
}


DLLExport int Control(HANDLE hSamplerThis, PARAM_TYPE	emParaType,
		int			nDevId,
		int			nParamId,
		VAR_VALUE	*pVarVal,
		int			nValSize,
		int			nTimeOut)
{
	SAMPLER* pSampler = (SAMPLER*)hSamplerThis;
	SYSTEM_SAMPLER_DATA *pSys_Rough = (SYSTEM_SAMPLER_DATA*)pSampler->pDataHeap;
	DATA_PROCESSOR* pDPR = (DATA_PROCESSOR*)pSampler->hDPR;
	VAR_VALUE varV;
	int n;

	if(Mutex_Lock(pSys_Rough->hSamplerLock, nTimeOut) != ERR_MUTEX_OK)
	{
		return ERR_CONTROL_TIMEOUT;
	}

	if(emParaType == PARAM_TYPE_SETTING)
	{
		if((nParamId == 1) && (nDevId == 1))
		{
			//set system time
			SetDateTime((time_t*)&pVarVal->ulValue);
		}
		else if((nParamId == 3) && (nDevId == 1))
		{
			//now set system network info
			A_BLOB_NETWORKINFO* pNetInfo = (A_BLOB_NETWORKINFO*)pVarVal->blobValue;
			if(pSys_Rough->currNetWorkInfo.netInfo_eht1.emNetPort_Status &&
				(pNetInfo->netInfo_eht0.ulIpV4_IP == pNetInfo->netInfo_eht1.ulIpV4_IP))
			{
				//added by Jimmy 20200905: eth0 and eth1 can set DHCP on at the  same time
				if(!(pNetInfo->bDHCP_On_eth0 && pNetInfo->bDHCP_On_eth1))
				{
					//eth0 and eth1 should have different IP
					Mutex_Unlock(pSys_Rough->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			//eth0
			if(pNetInfo->bDHCP_On_eth0 != pSys_Rough->currNetWorkInfo.bDHCP_On_eth0)
			{
				//update dhcp
				TRACEX("Now change DHCP eth0->%d...\n", pNetInfo->bDHCP_On_eth0);
				SetDHCP(ETHNAME_FIRST, pNetInfo->bDHCP_On_eth0);
			}
			//Jimmy20190306: will always set IP, do not care whether IP has changed
			//else //update ip/netmask/gateway
			{
				// printf("Now change netinfo eth0, DNS[%d, %d, %d]...\n", 
				// 	pNetInfo->netInfo_eht0.ulIpV4_DNSServer, pSys_Rough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer,
				// 	pNetInfo->bDHCP_On_eth0);
				if((pNetInfo->netInfo_eht0.ulIpV4_DNSServer == 0) && (pNetInfo->netInfo_eht0.ulIpV4_Gateway != 0) &&
					(pSys_Rough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer == 0))
				{
					pNetInfo->netInfo_eht0.ulIpV4_DNSServer = pNetInfo->netInfo_eht0.ulIpV4_Gateway;
					//pSys_Rough->currNetWorkInfo.netInfo_eht0.ulIpV4_DNSServer = pNetInfo->netInfo_eht0.ulIpV4_DNSServer;
				}
				SetNetInfo(pSys_Rough, ETHNAME_FIRST, &pNetInfo->netInfo_eht0, &pSys_Rough->currNetWorkInfo.netInfo_eht0);
				//if DNS-Server is cleared, should double open the DHCP for ETH1 & wireless
				if(pNetInfo->netInfo_eht0.ulIpV4_DNSServer == 0)
				{
					SetDHCP(ETHNAME_FIRST, pNetInfo->bDHCP_On_eth0);
					//pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
					Notice_LTEModem_4Redial();
				}
			}

			if(pSys_Rough->currNetWorkInfo.netInfo_eht1.emNetPort_Status)
			{
				//eth1
				if(pNetInfo->bDHCP_On_eth1 != pSys_Rough->currNetWorkInfo.bDHCP_On_eth1)
				{
					//update dhcp
					TRACEX("Now change DHCP eth1->%d...\n", pNetInfo->bDHCP_On_eth1);
					SetDHCP(ETHNAME_SECOND, pNetInfo->bDHCP_On_eth1);
				}
				//Jimmy20190306: will always set IP, do not care whether IP has changed
				//else //update ip/netmask/gateway
				{
					TRACEX("Now change netinfo eth1...\n");
					SetNetInfo(pSys_Rough, ETHNAME_SECOND, &pNetInfo->netInfo_eht1, &pSys_Rough->currNetWorkInfo.netInfo_eht1);
				}
			}

			//enable/disable LTE modem (added by Jimmy 20201214)
			if(pNetInfo->bIsEnable_LTEModem!= pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem)
			{
				pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem = pNetInfo->bIsEnable_LTEModem;
				if((pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem == 2) && (!pNetInfo->bDHCP_On_eth0))
				{
					pNetInfo->bDHCP_On_eth0 = TRUE;
					SetDHCP(ETHNAME_FIRST, TRUE);
				}
				// else if((pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem == 3) && (!pNetInfo->bDHCP_On_eth1))
				// {
				// 	pNetInfo->bDHCP_On_eth2 = TRUE;
				// 	SetDHCP(ETHNAME_SECOND, TRUE);
				// }
			}

			if (pNetInfo->netInfo_eht0.ulIpV4_Gateway != pSys_Rough->currNetWorkInfo.netInfo_eht0.ulIpV4_Gateway)
			{
				if (pSys_Rough->currNetWorkInfo.bIsEnable_LTEModem == 2)
				{
					NeedReconnectToRouteServer();//Reinit route serverip
				}
			}

			//APN
			if((pNetInfo->lteModemINfo.bIsManulSet_APN != pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN) ||
				(strcmp(pNetInfo->lteModemINfo.szAPN, pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN) != 0) ||
				(strcmp(pNetInfo->lteModemINfo.szAPN_User, pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_User) != 0) ||
				(strcmp(pNetInfo->lteModemINfo.szAPN_Passwd, pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_Passwd) != 0))
			{
				// BOOL bNeedReDial = FALSE;
				// if(pNetInfo->lteModemINfo.bIsManulSet_APN != pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN)
				// {
				// 	bNeedReDial = TRUE;
				// }
				// else if(pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN)
				// {
				// 	bNeedReDial = TRUE;
				// }
				pSys_Rough->currNetWorkInfo.lteModemINfo.bIsManulSet_APN = pNetInfo->lteModemINfo.bIsManulSet_APN;
				memcpy(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN, pNetInfo->lteModemINfo.szAPN, 64);
				memcpy(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_User, pNetInfo->lteModemINfo.szAPN_User, 64);
				memcpy(pSys_Rough->currNetWorkInfo.lteModemINfo.szAPN_Passwd, pNetInfo->lteModemINfo.szAPN_Passwd, 64);
				//if(bNeedReDial)
				{
					//pSys_Rough->emModemStatus = LTEMODEM_STATUS_NOT_INSTALLED;
					Notice_LTEModem_4Redial();
				}
			}

			TRACEX("Now refresh and save setting...\n");
			pSys_Rough->bNeedRefreshNetInfo = TRUE;
			Refresh_NetworkInfo(pSampler, FALSE); //the saved values are just for reference ,use fetched values instead!!!
			SaveNetInfo_IntoScriptFile(ETHNAME_FIRST, "/home/app_script/init_"ETHNAME_FIRST".sh", pSys_Rough->currNetWorkInfo.bDHCP_On_eth0, 
				&pSys_Rough->currNetWorkInfo.netInfo_eht0);
			if(pSys_Rough->currNetWorkInfo.netInfo_eht1.emNetPort_Status)
			{
				SaveNetInfo_IntoScriptFile(ETHNAME_SECOND, "/home/app_script/init_"ETHNAME_SECOND".sh", pSys_Rough->currNetWorkInfo.bDHCP_On_eth1, 
					&pSys_Rough->currNetWorkInfo.netInfo_eht1);
			}
		}
		else if((nParamId == 8) && (nDevId == 1)) //xsy add in 20241025
		{
			A_BLOB_NETWORKINFO netInfo = pSys_Rough->currNetWorkInfo;
			char ip[128];
			int slvNum = 0;

			// printf("pVarVal->emValue = %d\n", (ULONG)pVarVal->emValue);
			// set ess type
			if((ESSTYPE_UNINSTALL <= pVarVal->emValue) && (pVarVal->emValue < ESSTYPE_MAX))
			{
				if(pVarVal->emValue == ESSTYPE_UNINSTALL)
				{
					// set kwh meter install type
					varV.emValue = METER_TYPE_NotInstalled;
					Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 408, 401, varV);
					// set abu System parallel cabinet, devId：705 ABU
					varV.emValue = 0; // paramId:7140 System parallel cabinet, 0:disable, 1:enable
					Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 705, 7140, varV);

					// update dhcp
					if(pSys_Rough->currNetWorkInfo.bDHCP_On_eth0 != FALSE)
					{
						pSys_Rough->currNetWorkInfo.bDHCP_On_eth0 = FALSE;
						SetDHCP(ETHNAME_FIRST, pSys_Rough->currNetWorkInfo.bDHCP_On_eth0);
					}
					// set network
					// inet_aton(ip, ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_IP)));
					inet_aton("255.255.255.0", ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_Mask)));
					inet_aton("192.168.1.1", ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_Gateway)));
					inet_aton("192.168.1.1", ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_DNSServer)));
					SetNetInfo(pSys_Rough, ETHNAME_FIRST, &netInfo.netInfo_eht0, &pSys_Rough->currNetWorkInfo.netInfo_eht0);

					// refresh
					pSys_Rough->bNeedRefreshNetInfo = TRUE;
					Refresh_NetworkInfo(pSampler, FALSE);
					SaveNetInfo_IntoScriptFile(ETHNAME_FIRST, "/home/app_script/init_"ETHNAME_FIRST".sh", FALSE, 
						&pSys_Rough->currNetWorkInfo.netInfo_eht0);
				}
				else // Master and Slave #1~#9
				{
					// set kwh meter install type
					if(pVarVal->emValue == ESSTYPE_UDP_SERVER) // Master
					{
						varV.emValue = METER_TYPR_ac;
					}
					else // Slave #1~#9
					{
						varV.emValue = METER_TYPR_DataFromMaster;
					}
					Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 408, 401, varV);
					// set abu System parallel cabinet, devId：705 ABU
					varV.emValue = 1; // paramId:7140 System parallel cabinet, 0:disable, 1:enable
					Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 705, 7140, varV);
					
					if(pSys_Rough->currNetWorkInfo.netInfo_eht0.emNetPort_Status)
					{
						ULONG essType = (ULONG)pVarVal->emValue;
						slvNum = (int)(essType - (ULONG)(ESSTYPE_UDP_SERVER)); // server:0, #1~#9:1~9
						snprintf(ip, sizeof(ip), "192.168.1.10%d", slvNum);
						
						// update dhcp
						if(pSys_Rough->currNetWorkInfo.bDHCP_On_eth0 != FALSE)
						{
							pSys_Rough->currNetWorkInfo.bDHCP_On_eth0 = FALSE;
							SetDHCP(ETHNAME_FIRST, pSys_Rough->currNetWorkInfo.bDHCP_On_eth0);
						}
						// set network
						inet_aton(ip, ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_IP)));
						inet_aton("255.255.255.0", ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_Mask)));
						inet_aton("192.168.1.1", ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_Gateway)));
						inet_aton("192.168.1.1", ((struct in_addr*)(&netInfo.netInfo_eht0.ulIpV4_DNSServer)));
						SetNetInfo(pSys_Rough, ETHNAME_FIRST, &netInfo.netInfo_eht0, &pSys_Rough->currNetWorkInfo.netInfo_eht0);

						// refresh
						pSys_Rough->bNeedRefreshNetInfo = TRUE;
						Refresh_NetworkInfo(pSampler, FALSE);
						SaveNetInfo_IntoScriptFile(ETHNAME_FIRST, "/home/app_script/init_"ETHNAME_FIRST".sh", FALSE, 
							&pSys_Rough->currNetWorkInfo.netInfo_eht0);
					}
				}
			}
			else
			{
				// ESSTYPE error
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

		}
		else if((nParamId == 100) && (nDevId == 1))
		{
			// LJH241101 MU3
			SYSTEM_TYPE emSystemType = (ULONG)pVarVal->emValue;
#ifdef IMMU2
#else
			if (emSystemType == SYSTYPE_EMS_STATION)
			{
				TRACEE("sampler_system:Control ERR_CONTROL_NOTSUPPORTED Changed to [%d] 2:SYSTYPE_EMS_STATION", emSystemType);
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_NOTSUPPORTED;
			}
#endif
			if (pSys_Rough->lcCfg.emSystemType != emSystemType)
			{
				TRACEW("sampler_system:Control SystemType=[%d] Changed to [%d] rebooting...", pSys_Rough->lcCfg.emSystemType, emSystemType);
				LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
						"SystemType=[%d] Changed to [%d] rebooting", pSys_Rough->lcCfg.emSystemType, emSystemType);
				// LJH241202 SEMS reboot manually
				// EVENT_SYSTEM_EXIT evExit;
				// evExit.nExitType = SYS_INTERNAL_ERROR;
				// SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
				// 	&evExit, sizeof(EVENT_SYSTEM_EXIT));
			}
		}
		else if((nParamId == 86) && (nDevId == 1))
		{
			A_BLOB_KEEPALIVEDINFO* pKeepalivedCfg = (A_BLOB_KEEPALIVEDINFO*)pVarVal->blobValue;
			BOOL nNeedReoot = 0;

			if((strcmp(pKeepalivedCfg->szRouter_Id, pSys_Rough->blobKeepalivedConf.szRouter_Id) == 0)
				&&(pKeepalivedCfg->bState == pSys_Rough->blobKeepalivedConf.bState)
				&&(pKeepalivedCfg->nInterface == pSys_Rough->blobKeepalivedConf.nInterface)
				&&(pKeepalivedCfg->nVirtual_Router_Id == pSys_Rough->blobKeepalivedConf.nVirtual_Router_Id)
				&&(pKeepalivedCfg->nPriority == pSys_Rough->blobKeepalivedConf.nPriority)
				&&(pKeepalivedCfg->nAdvert_Int == pSys_Rough->blobKeepalivedConf.nAdvert_Int)
				&&(pKeepalivedCfg->bAuth_Type == pSys_Rough->blobKeepalivedConf.bAuth_Type)
				&&(strcmp(pKeepalivedCfg->szAuth_Pass, pSys_Rough->blobKeepalivedConf.szAuth_Pass) == 0)
				&&(strcmp(pKeepalivedCfg->szVirtual_Ipaddress, pSys_Rough->blobKeepalivedConf.szVirtual_Ipaddress) == 0))
			{
				printf("The new configuration file is the same as the old configuration file ");
			}
			else
			{
				printf("The new configuration file is different from the old configuration file ");
				//Rewrite keepalived.conf
				Write_Config_to_File(pKeepalivedCfg);
				nNeedReoot = 1;
			}

			if (pSys_Rough->blobKeepalivedConf.bEnable != pKeepalivedCfg->bEnable)
			{
				if (pKeepalivedCfg->bEnable)//0->1
				{
					printf("start keepalived...");
					_SYSTEM("keepalived");
				}
				else//1->0
				{
					printf("stop keepalived...");
					_SYSTEM("killall keepalived");
					if (access(MASTER_CONFIG_FILE, F_OK) == 0)
					{
						_SYSTEM("rm -f /opt/appdata/data/keepalived_master");
					}	
					if (access(BACKUP_CONFIG_FILE, F_OK) == 0)
					{
						_SYSTEM("rm -f /opt/appdata/data/keepalived_backup");
					}	
				}
				
			}
			else
			{
				if (pSys_Rough->blobKeepalivedConf.bEnable)
				{
					if (nNeedReoot)//cfg was changed。keepalived need reboot
					{
						printf("Restart keepalived...");
						_SYSTEM("killall keepalived");
						_SYSTEM("keepalived");
					}
				}
			}

			memcpy(&pSys_Rough->blobKeepalivedConf, pKeepalivedCfg, sizeof(A_BLOB_KEEPALIVEDINFO));
			
		}
		else if((nParamId == 25) && (nDevId == 1))
		{
			VAR_VALUE varVTmp;
			pDPR->Get(pSampler, GETTYPE_ST_VAR, 1, 10, &varVTmp, 4);

			int iCloundServerType = (int)varVTmp.emValue;
			if(pSys_Rough->emCustomType == GH_CUSTOM)
			{//GH的MQTT URL不是MQTT起始开头的，不用效验
				return ERR_CONTROL_OK;
			}
			
			//1:FB01_MQTT+TLS  2:FB02_MQTT+SelfTopic(IFY)
			if (1 == iCloundServerType || 2 == iCloundServerType)
			{
				char szURL[256] = {0};
				strncpy(szURL, pVarVal->blobValue, 255);
				
				char *pUppercase = strupr(szURL);
				if ((NULL == strstr(pUppercase, "MQTT://")) && (NULL == strstr(pUppercase, "MQTTS://")) && (NULL == strstr(pUppercase, "TCP://")) && (NULL == strstr(pUppercase, "WS://")) && (NULL == strstr(pUppercase, "WSS://")))
				{
					TRACEE("MQTT URL not support this protocol\n");
					Mutex_Unlock(pSys_Rough->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
				
			}

		}
		else if((nParamId == 89) && (nDevId == 1))
		{
			if((pVarVal->emValue != LOCAL_CTRL_MODE) && (pVarVal->emValue != REMOTE_CTRL_MODE))
			{
				//printf("Invalide control mode value!!!!!\n");
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			pSys_Rough->emControlMode = (COMTROL_MODE)pVarVal->emValue;
		}
		else if((nParamId == 34) && (nDevId == 1))
		{
			pSys_Rough->nBaiduOrGoogle = (UINT)pVarVal->emValue;
		}
		else if((nParamId == 103) && (nDevId == 1))
		{
			pSys_Rough->nPingNetwork = (UINT)pVarVal->emValue;
		}
		else if((nParamId == 104) && (nDevId == 1))
		{
			A_BLOB_DEVINSTALLCONFIG* pDevInstallConfig = (A_BLOB_DEVINSTALLCONFIG*)pVarVal->blobValue;
			int i = 0;
			for(i = 0; i < 2; ++i)
			{
				varV.emValue = pDevInstallConfig->AIR[i];
				Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 951 + i, 448, varV);				
			}
			for(i = 0; i < 2; ++i)
			{
				varV.emValue = pDevInstallConfig->PACK[i];
				Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 981 + i, 449, varV);				
			}
			for(i = 0; i < MAX_BATTCABINET_NUM; ++i)
			{
				varV.emValue = pDevInstallConfig->Bat_IMEU[i];
				Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 601 + i, 450, varV);				
			}
			for(i = 0; i < 2; ++i)
			{
				varV.emValue = pDevInstallConfig->Power_EU3[i];
				Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 611 + i, 451, varV);				
			}
			for(i = 0; i < 2; ++i)
			{
				varV.emValue = pDevInstallConfig->MulCube_EU3[i];
				Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 621 + i, 409, varV);				
			}
			varV.emValue = pDevInstallConfig->UartServer;
			Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 901, 910, varV);	
			varV.emValue = pDevInstallConfig->TempControllType;
			Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 911 , 915, varV); 	
			varV.emValue = pDevInstallConfig->WaterChiller;
			Push_VarValue(SETTYPE_INTERN_SETT, pSampler, 961, 2099, varV);
			// varV.emValue = pDevInstallConfig->PVEnable;
		}		
		/*
		else if((nParamId == 26) && (nDevId == 1))
		{
			//need consider system is in idle status
			A_BLOB_ES_BACKED_CFG *pOCPPSets = (A_BLOB_ES_iOCPP_CONFIGURAION*)pVarVal->blobValue;
			if((pOCPPSets->nChargerStatusReportInterval < 0) || (pOCPPSets->nChargerStatusReportInterval > 300))
			{
				TRACEX("Invalide ES_iOCPP key values!!!!!\n");
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			//now need sync the para to others 
			//Sync_ESiOCPPConfig(pSys_Rough, pOCPPSets);
		}
		*/
	}
	else if(emParaType == PARAM_TYPE_CONTROL)
	{
		TRACEX("Now received system control cmd\n");
		if((nParamId == 1) && (nDevId == 1))
		{
			if(Handle_OneClickSettins(pSys_Rough))
			{
            // printf("to 444\n");
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

			// printf("oneclickset    PARAM_TYPE_CONTROL~~~~~~~~~~~~~~~\n");
			// return ERR_CONTROL_OK;
		}
		else if((nParamId == 2) && (nDevId == 1)) //reboot system
		{
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
				"Received reset command, system will reboot soon!");
			//send msg to other moduals
			EVENT_SYSTEM_EXIT evExit;
			evExit.nExitType = (pVarVal->emValue == 1) ? SYS_EXIT_HARD_REBOOT : SYS_EXIT_SOFT_REBOOT;
			SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
				&evExit, sizeof(EVENT_SYSTEM_EXIT));
			//if(pVarVal->emValue == 1) //hard reset
			//{
			//	//Sleep(5000);
			//	//_SYSTEM("reboot");
			//}
		}
		else if((nParamId == 3) && (nDevId == 1)) //restore Default 
		{
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
				"Received RESTORE DEFAULT command, system will reboot soon!");
			//remove the database
			_SYSTEM("rm -rf /data/nccu.data");
			//Sleep(5000);
			_SYSTEM("reboot");
		}
		else if((nParamId == 23) && (nDevId == 1)) //restore BMS History Record 
		{
		 	char szCmd[128];
  			char szTime[32];
			time_t tmFileGenerated = pVarVal->ulValue;

			TimeToString(tmFileGenerated, "%y%m%d", szTime, sizeof(szTime));

			snprintf(szCmd, 128, "cd /var; tar -czf %s *%s.trc &", BMSHISREC_WEB_EXPORT_FILENAME, szTime);
			
			printf("Backup BMS History Rec ulValue=[%d] szTime=[%s] szCmd=[%s]\n", pVarVal->ulValue, szTime ,szCmd);

			_SYSTEM(szCmd);
		}
		else if(((nParamId == 4) || (nParamId == 14) || (nParamId == 16) || (nParamId == 17)) && (nDevId == 1)) //data Backup / Restore
		{
			if(pVarVal->emValue == 0)
			{
				char szCmd[128];
				if((nParamId == 16) || (nParamId == 17))
				{					
					//snprintf(szCmd, 128, "cp -rf /data/nccu.data %s;sync", SYSDB_WEB_EXPORT_FILENAME);	
					//Jimmy note: shall zip it background and wait it in web, or else will timeout!!!				
					snprintf(szCmd, 128, "cd /data; tar -czf %s nccu.data &", SYSDB_WEB_EXPORT_FILENAME);
					_SYSTEM(szCmd);
				}
				else
				{
					// if(access("/dev/mmcblk0", F_OK) != 0) //judge if SD card is inserted
					// {
					// 	//Jimmy 20200722: disable this for kernel problem tempriraly
					// 	if(access("/dev/sda1", F_OK) != 0) //judge if SD card is inserted
					// 	{
					// 		Mutex_Unlock(pSys_Rough->hSamplerLock);
					// 		return ERR_CONTROL_FAILED;
					// 	}
					// 	else
					// 	{
					// 		_SYSTEM("cp -rf /data/nccu.data /media/sda1;sync");
					// 	}
					// }
					// else
					// {
					// 	_SYSTEM("cp -rf /data/nccu.data /media/mmcblk0p1;sync");
					// }	
					char szPathDev[64];
					char* szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, NULL, szPathDev, 64);					
					if(szPath == NULL)
					{
						szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, NULL, szPathDev, 64);			
					}
					if(szPath != NULL)
					{
						snprintf(szCmd, 128, "cp -rf /data/nccu.data %s;sync", szPath);		
						_SYSTEM(szCmd);		
					}
					else
					{
						Mutex_Unlock(pSys_Rough->hSamplerLock);
						return ERR_CONTROL_FAILED;
					}
				}	
			}
			else if(pVarVal->emValue == 1) //restore settings
			{
				if((nParamId == 14) || (nParamId == 17)) //clone all data from SD card
				{
					// if(access("/media/mmcblk0p1/nccu.data", R_OK) != 0) //judge if backup file existed
					// {
					// 	if(access("/media/sda1/nccu.data", R_OK) != 0)
					// 	{
					// 		TRACEX("----------No SD or USB db found-------\n");
					// 		Mutex_Unlock(pSys_Rough->hSamplerLock);
					// 		return ERR_CONTROL_FAILED;
					// 	}
					// 	else
					// 	{
					// 		LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
					// 			"Received USB-Disk RESTORE DATABASE Command, data will be overwritten!");
					// 		_SYSTEM("cp -rf /media/sda1/nccu.data /data;sync");
					// 		_SYSTEM("reboot"); //need reboot system
					// 	}						
					// }
					// else
					// {
					// 	LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
					// 		"Received SD-Card RESTORE DATABASE Command, data will be overwritten!");
					// 	_SYSTEM("cp -rf /media/mmcblk0p1/nccu.data /data;sync");
					// 	_SYSTEM("reboot"); //need reboot system
					// }	
					char* szPath = NULL;
					char szPathDev[64];
					if(nParamId != 17)
					{
						szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, "nccu.data", szPathDev, 64);					
						if(szPath == NULL)
						{
							szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, "nccu.data", szPathDev, 64);			
						}
						if(szPath == NULL)
						{
							Mutex_Unlock(pSys_Rough->hSamplerLock);
							return ERR_CONTROL_FAILED;	
						}						
					}

					//Jimmy note: USB existence already judged in DPR
					Mutex_Unlock(pSys_Rough->hSamplerLock); //Jimmy note: need unlock so the DPR can lock here
					int nRet = pDPR->Set(hSamplerThis, SETTYPE_OVERWRITE_SET_BEFORE_CLONE, NULL, 
						(nParamId == 17) ? 99 : 0);
					if(nRet != ERR_DPR_INVALID_ARG)
					{
						if(nParamId == 17)
						{
							char szCmd[128];
							snprintf(szCmd, 128, "cp -rf %s /data/nccu.data;sync", SYSDB_WEB_IMPORT_FILENAME);
							_SYSTEM(szCmd);
						}
						else
						{
							// if(access("/media/mmcblk0p1/nccu.data", R_OK) == 0)						
							// {
							// 	_SYSTEM("cp -rf /media/mmcblk0p1/nccu.data /data;sync");							
							// }
							// //Jimmy 20200722: disable this for kernel problem tempriraly
							// else //if(access("/media/sda1/nccu.data", R_OK) == 0)
							// {
							// 	_SYSTEM("cp -rf /media/sda1/nccu.data /data;sync");
							// }	
							char szCmd[128];
							snprintf(szCmd, 128, "cp -rf %s/nccu.data /data;sync", szPath);
							_SYSTEM(szCmd);
						}	
						
						LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
							"Received %s sync setting before restore all-data, result %s!",
							(nParamId == 17) ? "WEB" : "LCD",
							(nRet == ERR_DPR_OK) ? "Successfylly" : "FAILED");						
						//_SYSTEM("sleep 1; reboot &"); //need reboot system
						//send msg to other moduals
						// EVENT_SYSTEM_EXIT evExit;
						// evExit.nExitType = (pVarVal->emValue == 1) ? SYS_EXIT_HARD_REBOOT : SYS_EXIT_SOFT_REBOOT;
						// SendMsg(pSys_Rough->hSampler, EVENT_TYPE_SYSTEM_EXIT, EVENT_MSG_BEGIN, 
						// 	&evExit, sizeof(EVENT_SYSTEM_EXIT));
						_SYSTEM("reboot &"); //need reboot system
						return ERR_CONTROL_OK;
						//return (nRet == ERR_DPR_OK) ? ERR_CONTROL_OK : ERR_CONTROL_FAILED;
					}
					else
					{
						LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
							"Received %s RESTORE all-data Command but FAILED to execute!",
							(nParamId == 17) ? "WEB" : "LCD");
						//Mutex_Unlock(pSys_Rough->hSamplerLock);
						return ERR_CONTROL_FAILED;
					}
				}
				else
				{
					Mutex_Unlock(pSys_Rough->hSamplerLock); //Jimmy note: need unlock so the DPR can lock here
					//if(pDPR->Set(hSamplerThis, SETTYPE_RESTORE_SETTINGS_FROMDB, NULL, 0) != ERR_DPR_OK)
					int nRet = pDPR->Set(hSamplerThis, SETTYPE_RESTORE_SETTINGS_FROMDB, NULL, 
						(nParamId == 16) ? 99 : 0);
					if(nRet != ERR_DPR_INVALID_ARG)
					{
						LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_ERROR,
							"%s to execute %s RESTORE SETTING Command, system will reboot!",
							(nParamId == 16) ? "WEB" : "LCD",
							(nRet == ERR_DPR_OK) ? "Successfylly" : "FAILED");
						
						//Still need to reboot though failed
						_SYSTEM("reboot &"); //need reboot system

						return ERR_CONTROL_OK;
					}
					else
					{
						LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
							"Received %s RESTORE SETTING Command but FAILED to execute it!",
							(nParamId == 16) ? "WEB" : "LCD");
						return ERR_CONTROL_FAILED;
					}					
				}
			}
		}
		else if((nParamId == 11) && (nDevId == 1)) //Clear Factory Debug data
		{
			if(pDPR->Set(hSamplerThis, SETTYPE_CLEAR_FACTOY_DEBUG_DATA, NULL, 0) != ERR_DPR_OK)
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
				"Received CLEAR FACTORY DATA command, system will reboot soon!");
			_SYSTEM("reboot &");
		}
		else if(nParamId == 12) //Load user-defined Startup Gif from SD card
		{
			if(pVarVal->emValue == 0)
			{
				if(access("/media/mmcblk0p1/splash.gif", F_OK) != 0) //judge if file is existed
				{
					if(access("/media/sda1/splash.gif", F_OK) != 0)
					{
						Mutex_Unlock(pSys_Rough->hSamplerLock);
						return ERR_CONTROL_FAILED;
					}
					else
					{
						_SYSTEM("rm -rf /app/empty_splash;cp -rf /media/sda1/splash.gif /app;sync");
					}					
					
				}
				else
				{
					_SYSTEM("rm -rf /app/empty_splash;cp -rf /media/mmcblk0p1/splash.gif /app;sync");
				}
			}
			else if(pVarVal->emValue == 1)
			{
				_SYSTEM("rm -rf /app/splash.gif;touch /app/empty_splash;sync");
			}
			else
			{
				_SYSTEM("rm -rf /app/splash.gif;rm -rf /app/empty_splash;sync");
			}
		}
		else if(nParamId == 8) //update firmware
		{
			if(nValSize != sizeof(A_BLOB_UPDATEFIRMWARE_CMD))
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			//if(pSys_Rough->emDownloadStatus == UPDOWN_PENDING) //when pending, cancel??
			//{
			//	Mutex_Unlock(pSys_Rough->hSamplerLock);
			//	return ERR_CONTROL_FAILED;
			//}			
			memcpy(&pSys_Rough->blobPendingFirmwareUpdateCmd, pVarVal->blobValue, sizeof(A_BLOB_UPDATEFIRMWARE_CMD));
			pSys_Rough->emDownloadStatus = (pSys_Rough->emDownloadStatus == UPDOWN_PENDING) ?
				pSys_Rough->emDownloadStatus : UPDOWN_WAIT_BEGIN;
			pSys_Rough->nDownloadRetryCount = 0;
			TRACEX("--------Received Firmware updateCmd, URL[%s]-----\n", pSys_Rough->blobPendingFirmwareUpdateCmd.szURL);
		}
		else if(nParamId == 18) //update firmware from WEB
		{
			if(!Handle_WebUpdate_IMMU2Firmware(pSys_Rough))
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;				
			}
		}
		else if(nParamId == 9) //upload diagnoseFile
		{
			if(nValSize != sizeof(A_BLOB_UPLOADDIAGN_CMD))
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
			//if(pSys_Rough->emUploadStatus == UPDOWN_PENDING) //when pending, cancel??
			//{
			//	Mutex_Unlock(pSys_Rough->hSamplerLock);
			//	return ERR_CONTROL_FAILED;
			//}
			memcpy(&pSys_Rough->blobPendingDiagnosUploadCmd, pVarVal->blobValue, sizeof(A_BLOB_UPLOADDIAGN_CMD));
			pSys_Rough->emUploadStatus = (pSys_Rough->emUploadStatus == UPDOWN_PENDING) ?
				pSys_Rough->emUploadStatus : UPDOWN_WAIT_BEGIN;
			pSys_Rough->nUploadRetryCount = 0;
			TRACEX("--------Received Diagnostic upload cmd, URL[%s], retries = %d, interV = %d-----\n", pSys_Rough->blobPendingDiagnosUploadCmd.szURL,
				pSys_Rough->blobPendingDiagnosUploadCmd.nRetries, pSys_Rough->blobPendingDiagnosUploadCmd.nRetrieveInterv);
		}
		else if (nParamId == 10) //Calibrate touch screen
		{
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_INFO, "Start CALIBRATING of touch screen!");
			_SYSTEM("killall -9 nccu_ui");
			_SYSTEM("echo 8 > /sys/class/backlight/backlight/brightness;/usr/lib/tslib/bin/ts_calibrate; reboot");
		}

		else if (nParamId == 13) //His Statistic Data Exported
		{
			TRACE("Enter Exported His Data To SD Card!\n");
			char szPathDev[64];
			char* szPath = FindFilePath_In_USB_OrSD_MountedDir(TRUE, NULL, szPathDev, 64);					
			if(szPath == NULL)
			{
				szPath = FindFilePath_In_USB_OrSD_MountedDir(FALSE, NULL, szPathDev, 64);			
			}
			if(szPath != NULL)
			{
				BOOL bRet = Exported_HisStatisData(pSys_Rough);
				if(bRet == FALSE)
				{
					Mutex_Unlock(pSys_Rough->hSamplerLock);
					return ERR_CONTROL_FAILED;
				}
			}
			else
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}
		}
		else if(nParamId == 19)
		{
			//printf("Control:Ctrl Jig Test enValue = %d\n", pVarVal->emValue);
			//push the value => StatusParam
			varV.ulValue = pVarVal->ulValue;
			Push_VarValue(SETTYPE_LOGIC_SP, (HANDLE)pSampler, 1, 16, varV);  
		}
#ifdef IMMU2
		else if(nParamId == 20) //added by Jimmy 20201221 for RESET LCD POWER
		{
			//Reset LCD Power
			Set_LCDPWR(FALSE);
			_SYSTEM("echo 0 > /sys/class/backlight/backlight/brightness"); //close backlight
			Sleep_Ex(300);
			_SYSTEM("echo 8 > /sys/class/backlight/backlight/brightness"); //open backlight
			if(!Set_LCDPWR(TRUE))
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;				
			}
		}
#endif
		else if(nParamId == 21) //added by Jimmy 20210319 for VPN start 
		{
			char *szConn_Args = (char*)pVarVal->blobValue;
			if(szConn_Args == NULL)
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;						
			}
			_SYSTEM("killall vcon_client"); //kill previous connection
			char szConnString[128];
			snprintf(szConnString, 128, "/app/bin/vcon_client %s &", szConn_Args);
			_SYSTEM(szConnString);
			//now wait the execute result
			Sleep_Ex(2000);
			int nRet = _SYSTEM_EX("ps | grep -v grep | grep -c vcon_client");
			if(nRet < 1)
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;				
			}
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
				"VC-OK[%s]", szConnString);
		}
		else if(nParamId == 22) //added by Jimmy 20210319 for VPN stop 
		{
			_SYSTEM("killall vcon_client"); //kill previous connection
			//now wait the execute result
			Sleep_Ex(2000);
			int nRet = _SYSTEM_EX("ps | grep -v grep | grep -c vcon_client");
			if(nRet >= 1)
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;				
			}
			LogOut(MAIN_MODULE_SAMPLER, "Sys", LOG_TYPE_WARING,
				" V-Conn has been stopped!");
		}
		else if(nParamId == 24)
		{// WLD250526 USB Flash one click upgrade
			pSys_Rough->bWebOneClivkupgrade = FALSE;
			pSys_Rough->bUpdFlag = TRUE;
			OneClick_Upgrade(pSys_Rough);
		}
		else if(nParamId == 25)
		{//Web one click upgrade
			varV.lValue = 0; 
			Push_VarValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 200, 82, varV);
			Push_VarValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 200, 85, varV);
			Handle_WebOneClick_Upgrade(pSys_Rough);
			pSys_Rough->bWebOneClivkupgrade = TRUE;
			pSys_Rough->bUpdFlag = TRUE;
			OneClick_Upgrade(pSys_Rough);
		}
	 	else if(nParamId == 31)  
		{
			Set_IMMU_DO(IOC_SET_DO1, pVarVal->emValue);
		}
	 	else if(nParamId == 32)  
		{
			Set_IMMU_DO(IOC_SET_DO2, pVarVal->emValue);
		}
		else if(nParamId == 33)  
		{
			Set_IMMU_DO(IOC_SET_DO3, pVarVal->emValue);
		}
		else if(nParamId == 34)  
		{
			Set_IMMU_DO(IOC_SET_DO4, pVarVal->emValue);
		}
		else if(nParamId == 35)  
		{
			Set_IMMU_DO(IOC_SET_DO5, pVarVal->emValue);
		}
		else if(nParamId == 36)  
		{
			Set_IMMU_DO(IOC_SET_DO6, pVarVal->emValue);
		}
		else if(nParamId == 37)//Added By John 2023.1105
		{
			TRACE("---Start Ping---\n");
			char *szConn_Args = (char*)pVarVal->blobValue;
			if(szConn_Args == NULL)
			{
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;						
			}
			char szArr[64];
			snprintf(szArr, 64, "ping %s -c 2 -w 2", szConn_Args); //-c ping Times; -w:ping Timeout Second
			TRACE("ping addr:=[%s]\n", szArr);
			FILE *fp = popen(szArr , "r");
			if (fp == NULL) 
			{
				TRACEX("Warning: popen Fail!!!\n");
				Mutex_Unlock(pSys_Rough->hSamplerLock);
				return ERR_CONTROL_FAILED;
			}

			// Read ping Output From Pipe
			char buffer[160];
			int n;
			while ((n = fread(buffer, 1, sizeof(buffer), fp)) > 0) //Each Times Read 16 Bytes
			{
				TRACE("---Start:%s:End---", buffer);
				if(n >= 160)
				{
					buffer[n-1] = '\0';
					break;
				}
			}
			pclose(fp); //Don't Forget close

			if(n <= 0)
			{
				snprintf(buffer, 160, "ping:bad address %s", szConn_Args);  
			}
			Push_TextValue(SETTYPE_LOGIC_SP, pSys_Rough->hSampler, 1, 45, buffer, 160);
			TRACE("---- End Ping ---\n");
		}
	}

	Mutex_Unlock(pSys_Rough->hSamplerLock);
	return ERR_CONTROL_OK;
}

BOOL GetLTENetworkInfo_AndJudgeIfConnected(SYSTEM_SAMPLER_DATA *pSys_Rough, char* szEthName)
{
	if(GetNetInfo(szEthName, &pSys_Rough->currNetWorkInfo.netInfo_usb0) == 0)
	{
		return FALSE;
	}
	//if already connected to WWLAN, consider it connected
	if(pSys_Rough->bIsWWLAN_Available)
	{
		return TRUE;
	}

	return Judge_IfNetwork_Connected(szEthName);
}

