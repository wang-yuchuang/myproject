/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaInterface.c
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 参数存储模块用户处理接口
** @Instructions : 
**===============================================================================================*/
#include "GroupPara.h"
#include "PublicPara.h"
#include "ParaInterface.h"
#include "BSPUserAPI.h"
#include "ParaRead.h"
#include "ParaWrite.h"

extern u16 gSysGenParaRO_100[];  							/*系统通用参数值*/
extern u16 gSysHigParaRO_101[];  							/*系统硬件参数值*/
extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];  								/*主控硬件参数值*/
/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : void ParaStoreParaInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组存储的参数初始化
** @The notes : D9只有外部EEPROM可供使用
**===============================================================================================*/
void ParaStoreParaInfoInit(void)
{
	u8 i = 0;
	u8 para8[GINDEX104_LEN] = {0};
	u16 para16[GINDEX102_LEN] = {0};

	/*初始化系统通用参数*/
	if(TRUE == ParaReadStoreSystemGenParaInfo(para16, SINDEX100_LEN))
	{
		for(i = 0; i < SINDEX100_LEN; i++)
		{
			gSysGenParaRO_100[i] = para16[i];
		}
	}
	else
	{
		
	}

	/*初始化系统高级参数(V2布局101区已扩至16字含监控/IP/设备地址,全量读写)*/
	if(TRUE == ParaReadStoreSystemHWParaInfo(para16, PARA_SYSHW_PARA_LEN))
	{
		for(i = 0; i < PARA_SYSHW_PARA_LEN; i++)
		{
			gSysHigParaRO_101[i] = para16[i];
		}
	}

	/*初始化主控通用参数*/
	if(TRUE == ParaReadStoreGroupGenParaInfo(para16, GINDEX102_LEN))
	{
		for(i = 0; i < GINDEX102_LEN; i++)
		{
			gGBmuGenPara_102[i] = para16[i];
		}
	}

	/*初始化主控高级参数*/
	if(TRUE == ParaReadStoreGroupHigParaInfo(para16, GINDEX103_LEN))
	{
		for(i = 0; i < GINDEX103_LEN; i++)
		{
			gGBmuHigLevPara_103[i] = para16[i];
		}
	}

	/*初始化主控硬件参数*/
	if(TRUE == ParaReadStoreGroupHWParaInfo(para8, GINDEX104_LEN))
	{
		for(i = 0; i < GINDEX104_LEN; i++)
		{
		gGHardPara_104[i] = para8[i];
		}
	}
}

/*=================================================================================================
** @Name      : u8 ParaCheckExtEepReadyHook(void)
** @Input     : void
** @Output    : TRUE:就绪 FALSE:异常
** @Function  : 检测外部EEPROM正常就绪状态
** @The notes : 用户必须补充完善
**===============================================================================================*/
u8 ParaCheckExtEepReadyHook(void)
{
    /*外部EEPROM读写正常*/
	if(1 == DEVEXTEEPGetReadyFlag())
	{
        return(TRUE);
	}

	return(FALSE);
}

/*=================================================================================================
** @Name      : u16 ParaGetGroupGenParaInfo(u16 para)
** @Input     : para:参数
** @Output    : 参数值
** @Function  : 获取通用参数信息
** @The notes : 
**===============================================================================================*/
u16 ParaGetGroupGenParaInfo(u16 para)
{
	if(para >= GINDEX102_LEN)
	{
		return (0);
	}
	return (gGBmuGenPara_102[para]);
}
/*=================================================================================================
** @Name      : u16 ParaGetGroupGenParaInfo(u16 para)
** @Input     : para:参数
** @Output    : 参数值
** @Function  : 获取通用参数信息
** @The notes : 
**===============================================================================================*/
u16 ParaGetGroupHigParaInfo(u16 para)
{
	if(para >= GINDEX103_LEN)
	{
		return (0);
	}
	return (gGBmuHigLevPara_103[para]);
}

/*=================================================================================================
** @Name      : u8 ParaRefactoryUserHandleHook(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 参数存储区恢复出厂设置值用户处理
** @The notes : 擦除 各段参数配置标志
**===============================================================================================*/
u8 ParaRefactoryUserHandleHook(void)
{
	u8 result = TRUE;
	u8 default8 = 0xff;
	u16 default16 = 0xffff;
	u32 default32 = 0xffffffff;

    /*根据地址写参数*/
    /*设备序列号(u8)*/
	/*result &= ParaWriteStoreDevSerial(0, &default8, 1);*/

	/*系统硬件参数(u16)*/
	result &= ParaWriteStoreSystemHWParaInfo(0, &default16, 1);

    /*电流零飘修正值参数(u16)*/
	/*result &= ParaWriteStoreCorrZeroCurr(0, &default16, 1);*/
    /*充电电流校准KB值参数(u32)*/
	/*result &= ParaWriteStoreChgCurrCorrKB(0, &default32, 1);*/
    /*放电电流校准KB值参数(u32)*/
	/*result &= ParaWriteStoreDhgCurrCorrKB(0, &default32, 1);*/
    /*电池总电压校准KB值参数(u32)*/
	/*result &= ParaWriteStoreSumVoltCorrKB(0, &default32, 1);*/
    /*预充电压校准KB值参数(u32)*/
	/*result &= ParaWriteStorePreVoltCorrKB(0, &default32, 1);*/
    /*负载电压校准KB值参数(u32)*/
	/*result &= ParaWriteStoreLoaVoltCorrKB(0, &default32, 1);*/
    /*Link+电压校准KB值参数(u32)*/
	/*result &= ParaWriteStorePosVoltCorrKB(0, &default32, 1);*/
    /*Link-电压校准KB值参数(u32)*/
	/*result &= ParaWriteStoreNegVoltCorrKB(0, &default32, 1);*/

    /*系统时钟(北京时间)参数(u16)*/
	result &= ParaWriteStoreSysClock(0, &default16, 1);
    /*月故障统计(年月)参数(u16)*/
	result &= ParaWriteStoreMonErrTime(0, 0, &default16, 1);
	/*月故障统计(年月)参数(u16)*/
	result &= ParaWriteStoreMonErrTime(1, 0, &default16, 1);
    /*当前总容量SOH值(u16)*/
	result &= ParaWriteStoreNowCapSoh(0, 0, &default16, 1);
	/*当前总容量SOH值(u16)*/
	result &= ParaWriteStoreNowCapSoh(1, 0, &default16, 1);
    /*充放电次数检测状态(u32)*/
	result &= ParaWriteStoreChgTimesState(0, 0, &default32, 1);
	/*充放电次数检测状态(u32)*/
	result &= ParaWriteStoreChgTimesState(1, 0, &default32, 1);
    /*总容量总能量自动标定信息(u32)*/
	result &= ParaWriteStoreCapEnerCorrInfo(0, 0, &default32, 1);
	/*总容量总能量自动标定信息(u32)*/
	result &= ParaWriteStoreCapEnerCorrInfo(1, 0, &default32, 1);
    /*电量计算结构信息(u32)*/
	result &= ParaWriteStoreCapFormInfo(0, 0, &default32, 1);
	/*电量计算结构信息(u32)*/
	result &= ParaWriteStoreCapFormInfo(1, 0, &default32, 1);
    /*电能计算结构信息(u32)*/
	result &= ParaWriteStoreEnerFormInfo(0, 0, &default32, 1);
	/*电能计算结构信息(u32)*/
	result &= ParaWriteStoreEnerFormInfo(1, 0, &default32, 1);
    /*系统运行(上电)时长 1Hour(u16)*/
	result &= ParaWriteStoreSysRunTime(0, &default16, 1);
    /*充放电次数(u16)*/
	result &= ParaWriteStoreChgDhgTimes(0, 0, &default16, 1);
	/*充放电次数(u16)*/
	result &= ParaWriteStoreChgDhgTimes(1, 0, &default16, 1);
    /*历史故障告警数目(u16)*/
	result &= ParaWriteStoreHisErrNum(0, 0, &default16, 1);
	/*历史故障告警数目(u16)*/
	result &= ParaWriteStoreHisErrNum(1, 0, &default16, 1);
    /*放电深度统计(u16)*/
	result &= ParaWriteStoreDODStaInfo(0, 0, &default16, 1);
	/*放电深度统计(u16)*/
	result &= ParaWriteStoreDODStaInfo(1, 0, &default16, 1);
    /*安全等级SOS锁定(u16)*/
	result &= ParaWriteStoreSOSLockInfo(0, 0, &default16, 1);
	/*安全等级SOS锁定(u16)*/
	result &= ParaWriteStoreSOSLockInfo(1, 0, &default16, 1);
    /*累计充放电电量 1AH(u32)*/
	result &= ParaWriteStoreHisChgDhgCap(0, 0, &default32, 1);
	/*累计充放电电量 1AH(u32)*/
	result &= ParaWriteStoreHisChgDhgCap(1, 0, &default32, 1);
    /*累计充放电电能 0.1kWh(u32)*/
	result &= ParaWriteStoreHisChgDhgEner(0, 0, &default32, 1);
	/*累计充放电电能 0.1kWh(u32)*/
	result &= ParaWriteStoreHisChgDhgEner(1, 0, &default32, 1);

    /*系统公共通用参数信息参数(u16)*/
	result &= ParaWriteStoreSystemGenParaInfo(0, &default16, 1);
    /*电池组主控通用参数信息参数(u16)*/
	result &= ParaWriteStoreGroupGenParaInfo(0, &default16, 1);
    /*电池组主控高级参数信息参数(u16)*/
	result &= ParaWriteStoreGroupHigParaInfo(0, &default16, 1);
    /*电池组主控硬件参数信息(u8)*/
	result &= ParaWriteStoreGroupHWParaInfo(0, &default8, 1);
	/*复位软件*/

    return(result);
}

