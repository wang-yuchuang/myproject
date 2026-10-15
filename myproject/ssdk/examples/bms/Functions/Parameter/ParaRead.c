/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaRead.c
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 参数存储模块读取参数处理
** @Instructions : 
**===============================================================================================*/
#include "GroupPara.h"
#include "ParaInterface.h"
#include "ParaRead.h"
#include "PublicPara.h"
#include "M2514.h"
#include "ParaExtEep.h"
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];  								/*主控硬件参数值*/

/*=================================================================================================
** @Name      : u8 ParaReadStoreDataByte(u16 addr, u8 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取指定长度字节数
** @The notes : 读取先后顺序:外部EEPROM
**===============================================================================================*/
u8 ParaReadStoreDataByte(u16 addr, u8 *data, u8 len)
{
    u8 result = 0;

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xff;
        result = DEVEXTEEPReadByte(addr, data, len);
    }

    /*读内部EEPROM失败*/
    if(FALSE == result)
    {

    }
    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreNowCapSoh(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取当前容量SOH值初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreNowCapSoh(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepNowCapSoh(clunum,data, len);

        /*读外部EEPROM成功*/
    	if(TRUE == result)
    	{
            /*将参数重新写入内部EEPROM*/
    		//WriteIntEepNowCapSoh(0, data, len);
    	}
        /*读外部EEPROM失败*/
    	else
    	{
    		/*请求备份到外部EEPROM*/
    		//ParaSetEepCopyReqFlag(PARA_NOW_CAP_SOH_ADDR, len, 0);
    	}
    }
    /*读外部EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashNowCapSoh(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreCorrZeroCurr(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电流零飘修正值初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreCorrZeroCurr(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepCorrZeroCurr(clunum, data, len);

        /*读外部EEPROM成功*/
    	if(TRUE == result)
    	{
            /*将参数重新写入内部EEPROM*/
    		//WriteIntEepNowCapSoh(0, data, len);
    	}
        /*读外部EEPROM失败*/
    	else
    	{
    		/*请求备份到外部EEPROM*/
    		//ParaSetEepCopyReqFlag(PARA_NOW_CAP_SOH_ADDR, len, 0);
    	}
    }
    /*读外部EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashNowCapSoh(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreChgTimesState(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取充放电次数检测状态初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreChgTimesState(u8 clunum, u32 *data, u8 len)
{
    u8 result = 0;
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffffffff;
        result = ReadExtEepChgTimesState(clunum,data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepChgTimesState(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_CHGTIMES_STA_ADDR, len, 0);
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffffffff;
    	//result = ReadIntFlashChgTimesState(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreCapEnerCorrInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取总容量总能量自动标定信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreCapEnerCorrInfo(u8 clunum, u32 *data, u8 len)
{
    u8 result = 0;
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffffffff;
        result = ReadExtEepCapEnerCorrInfo(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepCapEnerCorrInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
           // ParaSetEepCopyReqFlag(PARA_CAPENER_CORR_ADDR, len, 0);
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffffffff;
    	//result = ReadIntFlashCapEnerCorrInfo(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreCapFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电量计算结构信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreCapFormInfo(u8 clunum, u32 *data, u8 len)
{
    u8 result = 0;
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffffffff;
        result = ReadExtEepCapFormInfo(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepCapFormInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_CAP_FORM_ADDR, len, 0);
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffffffff;
    	//result = ReadIntFlashCapFormInfo(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreEnerFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电能计算结构信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreEnerFormInfo(u8 clunum, u32 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepEnerFormInfo(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffffffff;
        result = ReadExtEepEnerFormInfo(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepEnerFormInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_ENER_FORM_ADDR, len, 0);
        }
    }
    
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffffffff;
    	//result = ReadIntFlashEnerFormInfo(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreSysClock(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统时钟(北京时间)初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSysClock(u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepSysClock(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepSysClock(data, len);
        /*读外部EEPROM成功*/
    	if(TRUE == result)
    	{
            /*将参数重新写入内部EEPROM*/
    	    //WriteIntEepSysClock(0, data, len);
    	}
        /*读外部EEPROM失败*/
    	else
    	{
    		/*请求备份到外部EEPROM*/
    		//ParaSetEepCopyReqFlag(PARA_SYS_CLOCK_ADDR, len, 0);
    	}
    }
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashSysClock(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreMonErrTime(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取月故障统计时间(年月)初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreMonErrTime(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepSysClock(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepMonErrTime(clunum, data, len);
        /*读外部EEPROM成功*/
    	if(TRUE == result)
    	{
            /*将参数重新写入内部EEPROM*/
    	    //WriteIntEepSysClock(0, data, len);
    	}
        /*读外部EEPROM失败*/
    	else
    	{
    		/*请求备份到外部EEPROM*/
    		//ParaSetEepCopyReqFlag(PARA_SYS_CLOCK_ADDR, len, 0);
    	}
    }
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashSysClock(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreSysRunTime(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统运行(上电)时长初始化 1Hour
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSysRunTime(u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepSysRunTime(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepSysRunTime(data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepSysRunTime(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_SYS_RUNTIME_ADDR, len, 0);
        }
    }
    
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashSysRunTime(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreChgDhgTimes(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function(u: 从存储区读取充放电次数初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreChgDhgTimes(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepChgDhgTimes(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepChgDhgTimes(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepChgDhgTimes(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_CHG_TIMES_ADDR, len, 0);
        }
    }


    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashChgDhgTimes(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreHisErrNum(u8 clunum,u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取历史故障告警数目初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreHisErrNum(u8 clunum,u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepHisErrNum(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepHisErrNum(clunum,data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepHisErrNum(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_HIS_ERRNUM_ADDR, len, 0);
        }
    }
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashHisErrNum(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreDODStaInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取放电深度统计信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreDODStaInfo(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepDODStaInfo(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepDODStaInfo(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepDODStaInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_DOD_INFO_ADDR, len, 0);
        }
    }
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashDODStaInfo(data, len);
    }

    return(result);
}
/**=================================================================================================
** @Name      : u8 ParaReadStoreHisDodInfoV2(u8 clunum, u16 *data, u16 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度(u16)
** @Output    : 读取成功或失败
** @Function  : 从存储区读取历史DOD记录信息V2初始化
** @The notes : 读取先后顺序:外部EEPROM(环形缓冲区)
**===============================================================================================*/
u8 ParaReadStoreHisDodInfoV2(u8 clunum, u16 *data, u16 len)
{
    u8 result = 0;

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        result = ReadExtEepHisDodInfoV2(clunum, data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreSOSLockInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取安全等级SOS锁定信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSOSLockInfo(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepSOSLockInfo(data, len);
    /*读外部EEPROM*/
    data[0] = 0xffff;
    result = ReadExtEepSOSLockInfo(clunum, data, len);

    /*读外部EEPROM成功*/
    if(TRUE == result)
    {
        /*将参数重新写入内部EEPROM*/
        //WriteIntEepSOSLockInfo(0, data, len);
    }
    /*读外部EEPROM失败*/
    else
    {
        /*请求备份到外部EEPROM*/
        //ParaSetEepCopyReqFlag(PARA_SOS_INFO_ADDR, len, 0);
    }

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashSOSLockInfo(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreHisChgDhgCap(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取累计充放电电量初始化 1AH
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreHisChgDhgCap(u8 clunum, u32 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepHisChgDhgCap(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepHisChgDhgCap(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepHisChgDhgCap(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_HIS_CHGCAP_ADDR, len, 0);
        }
    }
    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashHisChgDhgCap(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreHisChgDhgEner(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取累计充放电电能初始化 0.1kWh
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreHisChgDhgEner(u8 clunum, u32 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepHisChgDhgEner(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffffffff;
        result = ReadExtEepHisChgDhgEner(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepHisChgDhgEner(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_HIS_CHGENER_ADDR, len, 0);
        }
    }

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffffffff;
    	//result = ReadIntFlashHisChgDhgEner(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreGroupGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电池组主控通用参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreGroupGenParaInfo(u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepGroupGenParaInfo(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepGroupGenParaInfo(data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepGroupGenParaInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_DEVGEN_PARA_ADDR, len, 0);
        }
    }

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashGroupGenParaInfo(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreGroupHigParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电池组主控高级参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreGroupHigParaInfo(u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepGroupHigParaInfo(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepGroupHigParaInfo(data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepGroupHigParaInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_DEVHIG_PARA_ADDR, len, 0);
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashGroupHigParaInfo(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreGroupHWParaInfo(u8 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取电池组主控硬件参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreGroupHWParaInfo(u8 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepGroupHWParaInfo(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xff;
        result = ReadExtEepGroupHWParaInfo(data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepGroupHWParaInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_DEVHW_PARA_ADDR, len, 0);
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xff;
    	//result = ReadIntFlashGroupHWParaInfo(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreSystemWorkMode(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统工作模式
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSystemWorkMode(u8 clunum, u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepSystemWorkMode(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepSystemWorkMode(clunum, data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepSystemWorkMode(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_SYSWORK_MOD_ADDR, len, 0);
        }
    }

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	data[0] = 0xffff;
    	//result = ReadIntFlashSystemWorkMode(data, len);
    }

    return(result);
}
/*=================================================================================================
** @Name      : u8 ParaReadStoreSystemGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统通用参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSystemGenParaInfo(u16 *data, u8 len)
{
    u8 result = 0;

    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepSystemGenParaInfo(data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepSystemGenParaInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_SYSGEN_PARA_ADDR, len, 0);
            
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
    	//data[0] = 0xffff;
    	//result = ReadIntFlashSystemGenParaInfo(data, len);
    }

    return(result);
}

/*=================================================================================================
** @Name      : u8 ParaReadStoreSystemHWParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从存储区读取系统硬件参数信息初始化
** @The notes : 读取先后顺序:内部EEPROM->外部EEPROM->内部FLASH
**===============================================================================================*/
u8 ParaReadStoreSystemHWParaInfo(u16 *data, u8 len)
{
    u8 result = 0;

    /*直接读内部EEPROM*/
	//result = ReadIntEepSystemHWParaInfo(data, len);
    /*外部EEPROM读写正常*/
    if(TRUE == ParaCheckExtEepReadyHook())
    {
        /*读外部EEPROM*/
        data[0] = 0xffff;
        result = ReadExtEepSystemHWParaInfo(data, len);

        /*读外部EEPROM成功*/
        if(TRUE == result)
        {
            /*将参数重新写入内部EEPROM*/
            //WriteIntEepSystemHWParaInfo(0, data, len);
        }
        /*读外部EEPROM失败*/
        else
        {
            /*请求备份到外部EEPROM*/
            //ParaSetEepCopyReqFlag(PARA_SYSHW_PARA_ADDR, len, 0);
        }
    }
    

    /*读内外EEPROM失败*/
    if(FALSE == result)
    {
        /*读内部FLASH*/
        data[0] = 0xffff;
        //result = ReadIntFlashSystemHWParaInfo(data, len);
    }

    return(result);
}
