#ifndef __SAMPLER_API_H__
#define __SAMPLER_API_H__
#include "std_sys.h"
#include "app_incs.h"

#define MIN_PWR_CHRG            (2)  // 最小充电功率，判断PCS是否正在充电
#define MIN_PWR_DISC            (-2) // 最小放电功率，判断PCS是否正在放电
#define MIN_PWR_LMT             (1)
#define MAX_PWR_LMT             (99999)
#define	RS485_RECV_BUF_LEN			4096
#define	RS485_SEND_BUF_LEN			1024
#define	RS232_BUFF_LEN				1024
#define TCP_BUFF_LEN				10240
// LJH241101 MU3
#define	CCU_BUFF_LEN_232            8192

#define  INIT_YFYBATT_CMD(p, _emMsgType, _emFuncCode, _nSubIdx, _pfnHandler) \
	((p)->emMsgType = (_emMsgType), \
	(p)->nFuncCode = (_emFuncCode), \
	(p)->nSubIndex = (_nSubIdx), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)++);

#define  INIT_YFY_RPMSG_BATT_CMD(p, _emMsgType, _emFuncCode, _nMainIdx, _pfnHandler) \
	((p)->emMsgType = (_emMsgType), \
	(p)->nFuncCode = (_emFuncCode), \
	(p)->nMainIndex = (_nMainIdx), \
	(p)->pfnCmdHandler = (_pfnHandler), \
	(p)++);
	
typedef enum tagMeterType
{
	METER_TYPE_NotInstalled = 0,
	METER_TYPR_dc,
	METER_TYPR_ac,
	METER_TYPR_DataFromMaster,
	METER_TYPR_UartServer
}METER_TYPE;

typedef enum tagIMEU2InstalledType
{
	IMEU2_TYPE_NotInstalled = 0,
	IMEU2_TYPE_Installed,

}IMEU_INSTALLED_TYPE;


typedef struct tagEvHandTags
{
	int nEvTag;
	int nMsgTag;
}EVHAND_TAGS;

// #define AR_MAX_DATA_LEN 1024
#define AR_MAX_DATA_LEN 2048

typedef struct
{
    UINT  ID_Datalen;      // BYTE0-3 数据长度
    BYTE  ID_FunCode;      // BYTE4 功能码
    BYTE  ID_MainIndex;    // BYTE5 信息类型（主索引）
    BYTE  ID_DstAddr;      // BYTE6 目标地址
    BYTE  ID_SrcAddr;      // BYTE7 源地址

    BYTE  Data[AR_MAX_DATA_LEN];  // DATA域，不同功能码对应的DATA不同，需要具体解析

} AR_FRAME;

DLLExport BOOL Initialize(HANDLE hSamplerSelf);
DLLExport int Control(HANDLE hSamplerThis,		//point to this Sampler so that we can use it
			PARAM_TYPE	emParaType,
			int			nDevId,
			int			nParamId,
			VAR_VALUE	*pVarVal,
			int			nValSize,
			int			nTimeOut);
DLLExport int Sample(HANDLE hSamplerThis);
DLLExport int JigTest(HANDLE hSamplerThis, void* pJigData);

//public use for all samplers
int SetCommStatus(HANDLE hSampler, int nDevId, BOOL bCommOK);
int Push_VarValue(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, VAR_VALUE varV);
int Push_VarValue_RereadPCS(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, VAR_VALUE RereadvarV, int nWhichSingleModule);
//int Pull_VarValue(HANDLE hSampler, PARAM_TYPE emParamTp, int nDevId, int nParamId, VAR_VALUE *pVar, BOOL bGetRaw);
int Push_BlobValue(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, void* pBlob, int nValSize, BLOB_TYPE tp);
int Push_TextValue(INTERSET_TYPE setType, HANDLE hSampler, int nDevId, int nParamId, char* pString, int nValSize);
int SendMsg(HANDLE hSampler, _EVENT_TYPE evTp, _EVENT_MSG msgTp, void* pMsgData, int nMsgDataSize);
int SendMsg_Urgent(HANDLE hSampler, _EVENT_TYPE evTp, _EVENT_MSG msgTp, void* pMsgData, int nMsgDataSize);
ESSYS_TYPE GetESSysType(HANDLE hSampler);
UINT GetClientIndexLC_Sampler(HANDLE hSampler);
BOOL IsClientLC_Sampler(HANDLE hSampler);
LCCfg GetLCCfg_Sampler(HANDLE hSampler); // LJH250508 LCCfg
ESS_DEVICE_TYPE GetESSDeviceType_Sampler(HANDLE hSampler);
BOOL Wait_BlobCtrlCmd(HANDLE hSampler, int nDevId, int nParamId, void* pBlob, int nValSize, BLOB_TYPE tp, int nTimeout_ms);
//int Get_BattMgmt_Sector(A_BLOB_BATT_MGMT_PLAN * pBattMgmtPlan);
BOOL DoMD5Chk_ForRemotePkg(BOOL bIs4Web);
int Get_FeeRate_Setctor_Accroding2Time(A_BLOB_CHARGE_FEESET *pFeeRate, time_t  tm_Now);
int Get_FeeRate_Setctor(A_BLOB_CHARGE_FEESET *pFeeRate);
// LJH250808 从控只取管理计划第一段
int Get_EmsCfgPlan_Setctor(A_BLOB_EMS_CONFIG_PLAN *pEmsCfgPlan, BOOL bFromOrg);
float GetTimeZone_OffSetHrs(HANDLE hSampler);
float Samp_Avg_Filter(FILTER_TYPE emType, float fRawVal, int nPointNum);

COMMPORT* GetCommPort_ById(HANDLE hSampler, int nPortId, COMMPORT* pComm);
float CalcMeterByApplyScenery(HANDLE hSampler, METER_TYPES MeterType);
UINT AR_ProcessFrame(BYTE *rxBuf, AR_FRAME *frame);

void DecodeRectBarcode(const UINT* puiBarCode, char* pszDest, int nDestLen);

#define CCU_FIRMWARE_UPGRADE_PATH "/app/ccu_bin/"
// LJH241101 MU3
BOOL Set_IMMU_DO(UINT ulWhichDO, BOOL bOn);
#define DIDO_NAME 		"/dev/dido"
#ifdef IMMU3
	#define DIDO_IOCTL_BASE 	'D'
	#define IOC_SET_DIDO(num)	_IOW(DIDO_IOCTL_BASE, num, int)
	#define IOC_SET_DO1 		1
	#define IOC_SET_DO2 		2
	#define IOC_SET_DO3 		3
	#define IOC_SET_DO4 		4
	#define IOC_SET_DO5 		5
	#define IOC_SET_DO6 		1
	#define IMMU3_DEV_BEEP_PATH 	"/sys/class/timed_output/beep/enable"

	BOOL Set_IMMU3_DO(UINT ulWhichDO, BOOL bOn);
	void Set_IMMU3_Beep(UINT nMSec);
#else
	//---------Following is for A02 DIDO controlling-----------
	#define DIDO_MAJOR			240
	/* ################## IMMU2 DO1~DO6  #############*/
	#define IOC_SET_DO1  	 _IOW(DIDO_MAJOR, 1, unsigned long)
	#define IOC_SET_DO2  	 _IOW(DIDO_MAJOR, 2, unsigned long)
	#define IOC_SET_DO3  	 _IOW(DIDO_MAJOR, 3, unsigned long)
	#define IOC_SET_DO4  	 _IOW(DIDO_MAJOR, 4, unsigned long)
	#define IOC_SET_DO5  	 _IOW(DIDO_MAJOR, 5, unsigned long)
	#define IOC_SET_DO6  	 _IOW(DIDO_MAJOR, 6, unsigned long)
	#define IOC_SET_LCDPWR   _IOW(DIDO_MAJOR, 19, unsigned long)

	/* ################## IMMU2 DI1~DO4  #############*/
	#define IMMU2_DI1		0
	#define IMMU2_DI2		1
	#define IMMU2_DI3		2
	#define IMMU2_DI4		3

	BOOL Set_IMMU2_DO(UINT ulWhichDO, BOOL bOn);
	BOOL Get_IMMU2_DI(int nWhichDI);
	BOOL Set_LCDPWR(BOOL bOn);
#endif

#endif