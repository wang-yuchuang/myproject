/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaWrite.c
** @Founder      : ZQD808
** @Date         : 2025.12.15
** @Function     : 参数存储模块写入参数处理
** @Instructions : 
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "PublicPara.h"
#include "ParaExtEep.h"
#include "ParaInterface.h"
#include "ParaWrite.h"
#include "M2514.h"
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
extern u16 gSysGenParaRO_100[];  							/*系统通用设置*/
extern u16 gSysHigParaRO_101[];  							/*系统硬件设置*/
extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];  								/*主控硬件参数值*/

/*=================================================================================================
** @Name      : u8 ParaWriteStoreNowCapSoh(u8 clunum,u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入当前总容量SOH值
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreNowCapSoh(u8 clunum,u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepNowCapSoh(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepNowCapSoh(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_NOW_CAP_SOH_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    	//ParaSetFlashCopyReqFlag(PARA_NOW_CAP_SOH_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreCorrZeroCurr(u8 clunum,u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入点流零飘修正值
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreCorrZeroCurr(u8 clunum,u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepNowCapSoh(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepCorrZeroCurr(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_NOW_CAP_SOH_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    	//ParaSetFlashCopyReqFlag(PARA_NOW_CAP_SOH_ADDR, 0);
    //}

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreChgTimesState(u8 clunum,u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入充放电次数检测状态
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreChgTimesState(u8 clunum,u8 index, u32 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepChgTimesState(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepChgTimesState(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_CHGTIMES_STA_ADDR + (4 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    	//ParaSetFlashCopyReqFlag(PARA_CHGTIMES_STA_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入总容量总能量自动标定信息值
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepCapEnerCorrInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepCapEnerCorrInfo(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_CAPENER_CORR_ADDR + (4 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_CAPENER_CORR_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电量计算结构信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepCapFormInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepCapFormInfo(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_CAP_FORM_ADDR + (4 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_CAP_FORM_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreEnerFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电能计算结构信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreEnerFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepEnerFormInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepEnerFormInfo(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_ENER_FORM_ADDR + (4 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_ENER_FORM_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSysClock(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统时钟(北京时间)
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSysClock(u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepSysClock(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepSysClock(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_SYS_CLOCK_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    if(TRUE == result)
    {
        /*请求备份FLASH*/
    	//ParaSetFlashCopyReqFlag(PARA_SYS_CLOCK_ADDR, 0);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSysRunTime(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统运行(上电)时长 1Hour
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSysRunTime(u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepSysRunTime(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepSysRunTime(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_SYS_RUNTIME_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_SYS_RUNTIME_ADDR, 0);
    //}

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreChgDhgTimes(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入充放电次数
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreChgDhgTimes(u8 clunum, u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepChgDhgTimes(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepChgDhgTimes(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_CHG_TIMES_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_CHG_TIMES_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreHisErrNum(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入历史故障告警数目
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreHisErrNum(u8 clunum, u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepHisErrNum(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepHisErrNum(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_HIS_ERRNUM_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_HIS_ERRNUM_ADDR, 0);
    //}

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreDODStaInfo(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入放电深度统计信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreDODStaInfo(u8 clunum, u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepDODStaInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepDODStaInfo(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_DOD_INFO_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_DOD_INFO_ADDR, 0);
    //}

    return(result);
}
/**=================================================================================================
** @Name      : u8 ParaWriteStoreHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len)
** @Input     : clunum:组号 index:字偏移 data:需要写入的数据 len:写入长度(u16)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入历史DOD记录信息V2(整块或部分)
** @The notes : 写入外部EEPROM(新环形缓冲巨)
**===============================================================================================*/
u8 ParaWriteStoreHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len)
{
	u8 result  = 0;

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
		result = WriteExtEepHisDodInfoV2(clunum, index, data, len);
    }

    return(result);
}

/**=================================================================================================
** @Name      : u8 ParaWriteStoreHisDodInfoRecV2(u8 clunum, u16 slot, u16 *rec)
** @Input     : clunum:组号 slot:记录槽位[0,HISDOD_REC_NUM) rec:单条记录(10个u16)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入单条历史DOD记录V2(增量写入减少E2磨损)
** @The notes : 写入外部EEPROM;写头(Id)须在写记录之后
**===============================================================================================*/
u8 ParaWriteStoreHisDodInfoRecV2(u8 clunum, u16 slot, u16 *rec)
{
	u8 result  = 0;

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
		result = WriteExtEepHisDodInfoRec(clunum, slot, rec);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreSOSLockInfo(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入安全状态SOS锁定信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSOSLockInfo(u8 clunum, u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepSOSLockInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepSOSLockInfo(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_SOS_INFO_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_SOS_INFO_ADDR, 0);
    //}

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入累计充放电电量 1AH
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepHisChgDhgCap(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepHisChgDhgCap(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_HIS_CHGCAP_ADDR + (4 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_HIS_CHGCAP_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum: 组号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入累计充放电电能 0.1kWh
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepHisChgDhgEner(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepHisChgDhgEner(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_HIS_CHGENER_ADDR + (4 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_HIS_CHGENER_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreMonErrTime(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:簇编号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入月故障统计时间(年月)
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreMonErrTime(u8 clunum, u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepMonErrTime(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepMonErrTime(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_MONERR_TIME_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    	//ParaSetFlashCopyReqFlag(PARA_MONERR_TIME_ADDR, 0);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreGroupGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电池组主控通用参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreGroupGenParaInfo(u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepGroupGenParaInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepGroupGenParaInfo(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_DEVGEN_PARA_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_DEVGEN_PARA_ADDR, 1);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreGroupHigParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电池组主控高级参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreGroupHigParaInfo(u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result  = WriteIntEepGroupHigParaInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepGroupHigParaInfo(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_DEVHIG_PARA_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_DEVHIG_PARA_ADDR, 1);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreGroupHWParaInfo(u8 index, u8 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入电池组主控硬件参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreGroupHWParaInfo(u8 index, u8 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepGroupHWParaInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepGroupHWParaInfo(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_DEVHW_PARA_ADDR + index, len, 1);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_DEVHW_PARA_ADDR, 1);
    //}

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaWriteStoreSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:簇编号 index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统系统工作模式信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepSystemWorkMode(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepSystemWorkMode(clunum, index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_SYSWORK_MOD_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_SYSWORK_MOD_ADDR, 0);
    //}

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreSystemGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统设备通用参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSystemGenParaInfo(u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepSystemGenParaInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepSystemGenParaInfo(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_DEVGEN_PARA_ADDR + (2 * index), len, 0);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //	ParaSetFlashCopyReqFlag(PARA_DEVGEN_PARA_ADDR, 0);
    //}

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaWriteStoreSystemHWParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据 len:写入数据长度(个数)
** @Output    : 写入成功或失败
** @Function  : 向存储区写入系统设备硬件参数信息
** @The notes : 写入内部EEPROM,请求备份到外部EEPROM和内部FLASH
**===============================================================================================*/
u8 ParaWriteStoreSystemHWParaInfo(u8 index, u16 *data, u8 len)
{
	u8 result  = 0;

    /*直接写入内部EEPROM*/
	//result = WriteIntEepSystemHWParaInfo(index, data, len);

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*直接写入外部EEPROM*/
    	result = WriteExtEepSystemHWParaInfo(index, data, len);

    	/*请求备份到外部EEPROM*/
    	//ParaSetEepCopyReqFlag(PARA_SYSHW_PARA_ADDR + (2 * index), len, 1);
    }

    /*写入成功*/
    //if(TRUE == result)
    //{
        /*请求备份FLASH*/
    //    ParaSetFlashCopyReqFlag(PARA_SYSHW_PARA_ADDR, 1);
    //}

    return(result);
}
