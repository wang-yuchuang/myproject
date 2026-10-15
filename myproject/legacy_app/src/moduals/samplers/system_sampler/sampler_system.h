#ifndef __SYSTEM_SAMPLER_MAIN_H
#define __SYSTEM_SAMPLER_MAIN_H

#include "../sampler_base.h"

#define CONFIG_FILE "/etc/keepalived/keepalived.conf"
#define MASTER_CONFIG_FILE "/opt/appdata/data/keepalived_master"
#define BACKUP_CONFIG_FILE "/opt/appdata/data/keepalived_backup"

#define MAX_REFRESH_ALAM_INTERVAL	15
#define MAX_REFRESH_ALAM_TIMES	3
// typedef enum tagLTEModemStatus
// {
// 	LTEMODEM_STATUS_NOT_INSTALLED, //modem not installed, or no signal
// 	//LTEMODEM_STATUS_NO_SIGNAL, //Signal low or SIM not installed	
// 	LTEMODEM_STATUS_WAIT_CONNECT, //sent out dial cmd and wait for connection
// 	LTEMODEM_STATUS_READY,
// 	LTEMODEM_STATUS_DISCONECTED, //error dialing
// 	LTEMODEM_STATUS_WAIT_RESET, //sent reset and wait its re-initialization
// }LTEMODEM_STATUS;

typedef struct stSystem_Rough
{
	HANDLE						hSampler; //pointer to sampler
	int							nSamplerId; //copy from sampler
	HANDLE						hSamplerLock;
	A_BLOB_NETWORKINFO			currNetWorkInfo; //current network info

	int							nSysRestartTimes;
	time_t						tmPowerOn;
	float						fRunSincePowerOn_Min;

	BOOL						bNeedRefreshNetInfo;

	A_BLOB_CHARGE_FEESET 		blobFeeRateCfg;

	A_BLOB_CHARGER_BOOTINFO		blobOCPP_BootInfo;

	//A_BLOB_RESREVE_CMD		blobReserveCmdConn0;

	HIS_METER_VALUE_REC			blobInHandling_MeterValRec;

	COMMPORT					commHttpFtp; //http/ftp download/upload
	HANDLE						hPort_UploadInUse;
	HF_UPDOWN_STATUS			emUploadStatus;
	int							nUploadRetryCount;
	time_t						tmLastUploadRetry;
	A_BLOB_UPLOADDIAGN_CMD 		blobPendingDiagnosUploadCmd;

	HANDLE						hPort_DownloadInUse;
	HF_UPDOWN_STATUS			emDownloadStatus;
	int							nDownloadRetryCount;
	time_t						tmLastDownloadRetry;
	A_BLOB_UPDATEFIRMWARE_CMD	blobPendingFirmwareUpdateCmd;

	time_t 						tmStartNotifySlaveUpgrade;
	
	time_t						tmLast_EnterWaitCCUUpgrade; //last time enter wait CCU firmware updt status
	
	time_t						tm_LastJudgeOverTemp;

	//modem management parameters
	// LTEMODEM_STATUS				emModemStatus;
	// int							nReadModem_ErrorCounter;
	// time_t						tmLastQueryInstallStatus;
	// time_t						tmLastQuerySignalStrength;
	// time_t						tmLastQueryNetworkTp;
	// time_t						tmLastDial;

	//following 2 param are for judging if DHCP not properly started up
	time_t				 		tm_LastSetDHCPOn_Eth0;
	time_t						tm_LastSetDHCPOn_Eth1;
 	//if WLAN real-time connection status(run in thread)
	BOOL						bIsWWLAN_Available;
	ESS_DEVICE_TYPE                     emESSDevType;

	//add by lpl 250315 for keepalived.conf
	A_BLOB_KEEPALIVEDINFO		blobKeepalivedConf;
	//WLD250526 add one-click upgrade function
	SUBDEVS_UPGRADE_INFO		sUpdInfo[UPGRADED_DVE_BatSide];
	time_t						tm_Updsendtime;
	time_t						tm_Updsendtimefirst;
	time_t						tm_Updsendtimelast;
	SUBDEVS_UPGRADE_INFO		sUpdInfo2[UPGRADED_DEV_ModSide];
	time_t						tm_PCSsendtime;
	time_t						tm_PCSsendtimefirst;
	time_t						tm_PCSsendtimelast;
	BOOL 						bUpdFlag;
	BOOL						bWebOneClivkupgrade;	
	time_t					    tm_RefreshStatus;
	LCCfg                       lcCfg; // LJH250508 LCCfg
	COMTROL_MODE				emControlMode;
	CUSTOM_TYPE					emCustomType; 
	UINT						nBaiduOrGoogle;
	UINT						nPingNetwork;
	time_t						t_LastRefreshAlamStatus;
	UINT						nRefreshTimes;
	time_t						tLastSaveTime;
}SYSTEM_SAMPLER_DATA;

void Manage_ExternalRouter(SYSTEM_SAMPLER_DATA *pSys_Rough);
void Manage_InternalLTEModem(SYSTEM_SAMPLER_DATA *pSys_Rough, BOOL bIsJigTestMode);
void Notice_LTEModem_4Redial(void);
BOOL GetLTENetworkInfo_AndJudgeIfConnected(SYSTEM_SAMPLER_DATA *pSys_Rough, char* szEthName);
BOOL NeedReconnectToRouteServer(void);

void OneClick_Upgrade(SYSTEM_SAMPLER_DATA *pSys_Rough);
void BatSideupgrade(SYSTEM_SAMPLER_DATA *pSys_Rough);
void ModSideupgrade(SYSTEM_SAMPLER_DATA *pSys_Rough);
BOOL refreshstatus(SYSTEM_SAMPLER_DATA *pSys_Rough);

USHORT Handle_OneClickSettins(SYSTEM_SAMPLER_DATA *pSys_Rough);

#endif


