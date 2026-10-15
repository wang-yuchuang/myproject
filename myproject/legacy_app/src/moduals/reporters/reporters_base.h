#ifndef __REPORTER_API_H_
#define __REPORTER_API_H_
#include "std_sys.h"
#include "app_incs.h"
#include "../cJSON.h"


#define INVALID_VALUE (-999)
#define AIR_MODE_COOLING		0	//0-制冷
#define AIR_MODE_HEATING		1	//1-制热
#define AIR_MODE_CRICULATION	2	//2-自循环
#define AIR_MODE_STANDBY		3	//3-待机
#define AIR_MODE_AUTO			4	//4-自动

#define METER_NO_INSTALL		0	//0-未安装
#define METER_NORMAL			1	//1-正常
#define METER_COM_FALSE			2	//2-通讯失败
typedef struct _CTREAT_SEMS_JSON_INFO
{
    int iClientIdx;
    int nCurrDataFrom;
    int iBattIdx;
    int iBattGroupNum;
    
}CTREAT_SEMS_JSON_INFO;

DLLExport BOOL Initialize(HANDLE pReporterThis);

LCCfg GetLCCfg_Reporter(REPORTER* pReporter); // LJH250508 LCCfg
cJSON* CreateJSON_C155(REPORTER* pReporter, LCCfg* pLCCfg, UINT nIntv, A_BLOB_ES_RUN_DATA* pRunData);
BOOL GetMeterValue_C157(REPORTER* pReporter, int nDev, SC6LDEV_CMD157* pCMD157);
cJSON* CreateJSON_C157(REPORTER* pReporter, LCCfg* pLCCfg, UINT nIntv);

cJSON* CreateJSON_SEMS_C103(A_BLOB_SEMS_CMD103* pBlobSemsCMD103Data, CTREAT_SEMS_JSON_INFO* pCreateInfo);
cJSON* CreateJSON_SEMS_C110(A_BLOB_SEMS_CMD110* pBlobSemsCMD110Data, CTREAT_SEMS_JSON_INFO* pCreateInfo);
cJSON* CreateJSON_SEMS_C202(A_BLOB_SEMS_CMD202* pBlobSemsCMD202Data, CTREAT_SEMS_JSON_INFO* pCreateInfo);
COMMPORT* Reporter_GetCommPort_ById(HANDLE hReporter, int nPortId, COMMPORT* pComm);
cJSON* CreateJSON_C202(REPORTER* pReporter, A_BLOB_TYPE_DATASTORAGE* blobDataStorage, UINT nCfgBMSNum);
cJSON* CreateJSON_C204(REPORTER* pReporter, A_BLOB_TYPE_DATASTORAGE* blobDataStorage, UINT nCfgBMSNum);
cJSON* CreateJSON_C206(REPORTER* pReporter, A_BLOB_TYPE_DATASTORAGE* blobDataStorage);

#endif
