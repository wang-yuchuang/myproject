#ifndef _DEV_MODEL_2018_03_27_
#define _DEV_MODEL_2018_03_27_

#include "base_defines/basetypes.h"
#include "data_model/sampler_model.h"

typedef struct tagDeviceModelInfo
{
	int	nDevModelId;
	char	szDevModelName[32]; //max supported 32-byte

}DEVICE_MODEL_INFO;

typedef struct tagResourceText
{
	int	nRes_Id;
	char	szName[32]; //max 32Bytes
}RESOURCE_TEXT;

typedef struct tagEnumItem
{
	int				nEnumVal;
	RESOURCE_TEXT	*pEnumRes;
}PARAM_ENUM_ITEM;

typedef struct tagParamInfo
{
	int					nParamId;
	PARAM_TYPE			emParamType;
	//char				szParamName[32]; //Max supported 32-byte
	//char				szParamUnit[16];
	RESOURCE_TEXT		*pRes_Name;
	RESOURCE_TEXT		*pRes_Unit;
	DEVICE_MODEL_INFO*	pBelonged_DevModel;

	_VALUE_TYPE			emValueType;
	UINT					nValueSize; //for var, set 4, for sz, set sz-max bytes
	int					nPrecision; //for float only, default 0
	float					fMaxLimit;
	float					fMinLimit;
	int					nEnumTotal;
	PARAM_ENUM_ITEM		*pEnumList;
	BOOL					bDisableDisplay; //disable display ,UI will not disp this
	CLUSTER_EXP_ELEMENT	displayExpression;
	int					nDisplayPriority; //0--default, and the greater, the lower prio
	//Jimmy added 20190814,mask for engineer/administrator authority level
#define LEVEL_AUTHORITY_MASK_BIT_READ	0x01
#define LEVEL_AUTHORITY_MASK_BIT_WRITE	0x02
	UINT				uMask_AuthorityLevel; //0x00--R/W, 0x01--Read Disabled 0x02--Write Diabled 0x03--Both Disabled
	UINT                Web_1;
	UINT				Web_2;
}PARAM_BASIC_INFO;

typedef enum
{
	PARAM_SRC_SA,		//from sampler
	PARAM_SRC_CA,		//from evaluExpression
	PARAM_SRC_LS			//from logic-set, by other samplers
}PARAM_SRC;

typedef struct tagStatusParamInfo
{
	PARAM_BASIC_INFO		info;
	PARAM_SRC			emDataSrc;
	CLUSTER_EXP_ELEMENT	equalExpression;
}STATUS_PARAM_INFO;

typedef struct tagControlParamInfo
{
	PARAM_BASIC_INFO		info;
	CLUSTER_EXP_ELEMENT	controlableExpression;
}CONTROL_PARAM_INFO;

typedef struct tagSettingParamInfo
{
	PARAM_BASIC_INFO		info;
	BOOL					bNeedSave;
	BOOL					bNeedSyncSampler;
	CLUSTER_EXP_ELEMENT	settableExpression;
	VAR_VALUE			varValDefault;
}SETTING_PARAM_INFO;

typedef enum tagAlarmLevel
{
	ALARM_LEVEL_NONE = 0,			/* NA */
	ALARM_LEVEL_OBSERVATION,		/* OA */
	ALARM_LEVEL_MAJOR,			/* MA */
	ALARM_LEVEL_CRITICAL,			/* CA */
	ALARM_LEVEL_MAX
}ALARM_LEVEL;

typedef struct tagAlarmInfo
{
	int					nAlarmId;
	//char				szAlarmName[32];
	RESOURCE_TEXT		*pRes_Name;
	DEVICE_MODEL_INFO	*pBelonged_DevModel;
	ALARM_LEVEL			emLevel;
	int					nExValue_ParamId; //Must be device from self

	CLUSTER_EXP_ELEMENT	alarmCondExpression;
	CLUSTER_EXP_ELEMENT	alarmMaskExpression;

}ALARM_INFO;

//typedef enum tagValueStatus
//{
//	VALUE_STATUS_NORMAL,
//	VALUE_STATUS_OOR, //Out of Range
//	VALUE_STATUS_NA, //Not specified
//}_VALUE_STATUS;

typedef struct tagA_Param
{
	PARAM_BASIC_INFO* pParamInfo;//can be translate into pointer to status/setting/ctrl type parameter
	HANDLE			hDevice; //pointer to device
	//VAR_VALUE		rawValue; //not processed value, Blob/String has no raw value
	//BOOL			bIsRawInvalid; //if raw value is valid
	VAR_VALUE		currValue;
	BOOL				bIsValueInvalid; //if the current value is invalid
	VAR_VALUE		lastValue; //store old value for use
	VAR_VALUE		ReadValue;
	//_VALUE_STATUS	emValueStatus;
	time_t			tmCurrValueRefreshed;
	time_t			tmLastValueChanged;
	//the following attributes are for UI
	BOOL				bDisplayAble;
	BOOL				bCtrlSetAble;
}PARAMETER;

typedef enum tagAlarmStatus
{
	ALARM_STATUS_CEASED,
	ALARM_STATUS_ACTIVE,
	ALARM_STATUS_SUPPRESSED,
	ALARM_STATUS_PROCESSED, //for pcs monitor web page, means this alarm is processed manually, but still active, so needed to be masked
}ALARM_STATUS;

typedef struct tagAn_Alarm
{
	ALARM_INFO		*pAlarmInfo;
	HANDLE			hDevice; //pointer to device
	ALARM_STATUS		emStatus;
	//float			fExParamVal;
	PARAMETER		*pExParam; //the related param value
	time_t			tmLastOccured;
	time_t			tmLastCeased;
	//this is for restoring event
	time_t			tmLastRestored;
}ALARM;

typedef enum tagDeviceStatus
{
	DEVICE_STATUS_NORMAL,
	DEVICE_STATUS_NOT_WORK,
	DEVICE_STATUS_COMM_FAIL
}DEVICE_STATUS;


typedef struct tagDeviceInfo
{
	int					nDeviceId;
	//char				szDeviceName[32];
	RESOURCE_TEXT		*pRes_Name;
	DEVICE_MODEL_INFO*	pBelonged_DevModel;
	SAMPLER*				pBelonged_Sampler;
}DEVICE_INFO;

DEFINE_CLUSTER(PARAMETER);
DEFINE_CLUSTER(ALARM);
typedef struct tagA_Device
{
	DEVICE_INFO			info;
	DEVICE_STATUS		emStatus;

	CLUSTER_PARAMETER	params_Status;
	CLUSTER_PARAMETER	params_Control;
	CLUSTER_PARAMETER	params_Setting;

	CLUSTER_ALARM		alarms;

}DEVICE;


#endif
