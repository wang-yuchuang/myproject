/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : ParaExtEep.c
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 参数存储模块读写外部EEPROM参数
** @Instructions : 
**===============================================================================================*/

#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "PublicPara.h"
#include "ParaInterface.h"
#include "ParaExtEep.h"
#include "M2514.h"
#include "DEVTIMER.h"
#include <stdio.h>
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
extern u16 gSysGenParaRO_100[];  							/*系统通用设置*/
extern u16 gSysHigParaRO_101[];  							/*系统硬件设置*/
extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];  								/*主控硬件参数值*/
extern u16 gSysGenParaRW_105[];                             /*系统通用设置*/
/*=================================================================================================
** @Name      : u8 ReadExtEepMonErrTime(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取月故障数目统计时间(年月)
** @The notes :
**===============================================================================================*/
u8 ReadExtEepMonErrTime(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_MONERR_TIME_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            (void)DEVEXTEEPReadWord(PARA_MONERR1_TIME_ADDR, data, len);
        }
        else
        {
            (void)DEVEXTEEPReadWord(PARA_MONERR2_TIME_ADDR, data, len);
        }
       

        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0]) && (data[1] >= 2000))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认时间20/01*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0x1401;
    if(clunum == 0)
    {
        (void)DEVEXTEEPWriteWord(PARA_MONERR1_TIME_ADDR + 2, &data[1], (len - 1));
        (void)DEVEXTEEPWriteWord(PARA_MONERR1_TIME_ADDR, &data[0], 1);
    }
    else
    {
        (void)DEVEXTEEPWriteWord(PARA_MONERR2_TIME_ADDR + 2, &data[1], (len - 1));
        (void)DEVEXTEEPWriteWord(PARA_MONERR2_TIME_ADDR, &data[0], 1);
    }


    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepCorrZeroCurr(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电流零飘修正值(有符号,单位10mA)
** @The notes :
**===============================================================================================*/
u8 ReadExtEepCorrZeroCurr(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_ZERO_C_CORR_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            (void)DEVEXTEEPReadWord(PARA_ZERO_C1_CORR_ADDR, data, len);
        }
        else
        {
            (void)DEVEXTEEPReadWord(PARA_ZERO_C2_CORR_ADDR, data, len);
        }
       

        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0])
            && ((0 - (s16)MAX_CURR_CORR_10MA) <= (s16)data[1])
			&& ((s16)data[1] <= (s16)MAX_CURR_CORR_10MA))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认电流零飘修正值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0;
    if(clunum == 0)
    {
        (void)DEVEXTEEPWriteWord(PARA_ZERO_C1_CORR_ADDR + 2, &data[1], (len - 1));
        (void)DEVEXTEEPWriteWord(PARA_ZERO_C1_CORR_ADDR, &data[0], 1);
    }
    else
    {
        (void)DEVEXTEEPWriteWord(PARA_ZERO_C2_CORR_ADDR + 2, &data[1], (len - 1));
        (void)DEVEXTEEPWriteWord(PARA_ZERO_C2_CORR_ADDR, &data[0], 1);
    }


    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepNowCapSoh(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组标号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取当前总容量SOH值
** @The notes :
**===============================================================================================*/
u8 ReadExtEepNowCapSoh(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_NOW_CAP_SOH_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadWord(PARA_NOW_CAP1_SOH_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadWord(PARA_NOW_CAP2_SOH_ADDR, data, len);
        }


        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0]) && (data[1] <= 1000))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值100%*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 1000;
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_NOW_CAP1_SOH_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_NOW_CAP1_SOH_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_NOW_CAP2_SOH_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_NOW_CAP2_SOH_ADDR, &data[0], 1);
    }
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepChgTimesState(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号  data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取充放电次数检测状态
** @The notes :
**===============================================================================================*/
u8 ReadExtEepChgTimesState(u8 clunum, u32 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_SYS_CLOCK_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadLong(PARA_CHGTIMES1_STA_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadLong(PARA_CHGTIMES2_STA_ADDR, data, len);
        }

        /*读取到有效参数*/
        if(PARA_CFG_FLAG_U32 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U32;
    data[1] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteLong(PARA_CHGTIMES1_STA_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_CHGTIMES1_STA_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteLong(PARA_CHGTIMES2_STA_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_CHGTIMES2_STA_ADDR, &data[0], 1);
    }
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepCapEnerCorrInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum: 组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取总容量总能量自动标定信息值
** @The notes :
**===============================================================================================*/
u8 ReadExtEepCapEnerCorrInfo(u8 clunum, u32 *data, u8 len)
{
    u8 i = 0;
    u8 trytime = 5;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_CAPENER_CORR_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadLong(PARA_CAPENER1_CORR_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadLong(PARA_CAPENER2_CORR_ADDR, data, len);
        }


        if(PARA_CFG_FLAG_U32 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*写默认值,防止每次初始化占用太多读时间*/
	data[0] = PARA_CFG_FLAG_U32;
	for(i = 1; i < len; i++)
	{
		data[i] = 0;
	}
    if(clunum == 0)
    {
        DEVEXTEEPWriteLong(PARA_CAPENER1_CORR_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_CAPENER1_CORR_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteLong(PARA_CAPENER2_CORR_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_CAPENER2_CORR_ADDR, &data[0], 1);
    }


    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepCapFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电量计算结构信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepCapFormInfo(u8 clunum, u32 *data, u8 len)
{
    u8 trytime = 5;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_CAP_FORM_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadLong(PARA_CAP1_FORM_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadLong(PARA_CAP2_FORM_ADDR, data, len);
        }


        if((PARA_CFG_FLAG_U32 == data[0]) && (data[1] <= data[3]) && (data[2] <= data[3])
            && (data[3] <= MAX_ALL_CAP_CFG) && (data[4] <= MAX_ALL_CAP_CFG))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }   
    /*配置标志*/
	data[0] = PARA_CFG_FLAG_U32;
    /*当前电量(SOC)取配置值[0.1AH转1mAH]*/
    if(clunum == 0)
    {
	    data[1] = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] * gGBmuGenPara_102[eBmuGenPara102_1NowSoc] / 10;
    }
    else
    {
        data[1] = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] * gGBmuGenPara_102[eBmuGenPara102_2NowSoc] / 10;
    }
    /*剩余电量1mAH*/
	data[2] = data[1];
    /*总容量[0.1AH转1mAH]*/
    if(clunum == 0)
    {
        data[3] = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] * 100;
    }
    else
    {
        data[3] = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllCap] * 100;
    }
    /*标称容量[0.1AH转1mAH]*/
	//data[4] = (u32)gGBmuGenPara_102[eBmuGenPara102_RateCap] * 100;
	//额定容量无小数才为标称容量
	if(0 == (gGBmuGenPara_102[eBmuGenPara102_RateCap] % 10))
	{
		data[4] = gGBmuGenPara_102[eBmuGenPara102_RateCap] * 100;
	}
	//额定容量有小数取实际总容量
	else
	{
		data[4] = gGBmuGenPara_102[eBmuGenPara102_1ReaAllCap] * 100;
	}

     if(clunum == 0)
     {    
        /*未读取到有效参数，写入默认值*/
	    DEVEXTEEPWriteLong(PARA_CAP1_FORM_ADDR + 4, &data[1], (len - 1));
	    DEVEXTEEPWriteLong(PARA_CAP1_FORM_ADDR, &data[0], 1);
     }
     else
     {
                /*未读取到有效参数，写入默认值*/
	    DEVEXTEEPWriteLong(PARA_CAP2_FORM_ADDR + 4, &data[1], (len - 1));
	    DEVEXTEEPWriteLong(PARA_CAP2_FORM_ADDR, &data[0], 1);
     }
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepEnerFormInfo(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电能计算结构信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepEnerFormInfo(u8 clunum, u32 *data, u8 len)
{
    u8 trytime = 5;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_ENER_FORM_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadLong(PARA_ENER1_FORM_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadLong(PARA_ENER2_FORM_ADDR, data, len);
        }

        if((PARA_CFG_FLAG_U32 == data[0]) && (data[1] <= data[3]) && (data[2] <= data[3])
            && (data[3] <= MAX_ALL_ENER_CFG) && (data[4] <= MAX_ALL_ENER_CFG))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }
        /*配置标志*/
	data[0] = PARA_CFG_FLAG_U32;
    /*当前电能(SOE)取配置值[0.1kWh转1Wh]*/
    if(clunum == 0)
    {
        data[1] = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * gGBmuGenPara_102[eBmuGenPara102_1NowSoe] / 10;
    }
    else
    {
        data[1] = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * gGBmuGenPara_102[eBmuGenPara102_2NowSoe] / 10;
    }
    /*剩余电能1Wh*/
	data[2] = data[1];
    /*总电能[0.1kWh转1Wh]*/
	//data[3] = (u32)gGBmuGenPara_102[eBmuGenPara102_ReaAllEner] * 100;
    /*标称电能[0.1kWh转1Wh]*/
	//data[4] = (u32)gGBmuGenPara_102[eBmuGenPara102_ReaAllEner] * 100;
	//总电能无小数则为标称电能(总电能为97%)
    if(clunum == 0)
    {
        if(0 == (gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] % 10))
        {
            data[3] = gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 9700 / 100;
            data[4] = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 100;
        }
        //有小数,未知标称电能认为总电能为97%
        else
        {
            data[3] = gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 100;
            data[4] = (u32)gGBmuGenPara_102[eBmuGenPara102_1ReaAllEner] * 10000 / 97;
        }
    }
    else
    {
        if(0 == (gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] % 10))
        {
            data[3] = gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 9700 / 100;
            data[4] = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 100;
        }
        //有小数,未知标称电能认为总电能为97%
        else
        {
            data[3] = gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 100;
            data[4] = (u32)gGBmuGenPara_102[eBmuGenPara102_2ReaAllEner] * 10000 / 97;
        }
    }
    if(clunum == 0)
    {
        /*未读取到有效参数，写入默认值*/
        DEVEXTEEPWriteLong(PARA_ENER1_FORM_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_ENER1_FORM_ADDR, &data[0], 1);
    }
    else
    {
        /*未读取到有效参数，写入默认值*/
        DEVEXTEEPWriteLong(PARA_ENER2_FORM_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_ENER2_FORM_ADDR, &data[0], 1);
    }
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepSysClock(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统时钟(北京时间)
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSysClock(u16 *data, u8 len)
{
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_SYS_CLOCK_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        (void)DEVEXTEEPReadWord(PARA_SYS_CLOCK_ADDR, data, len);

        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0]) && (data[1] >= 2000) && (data[1] <= 2200)
            && (data[2] >= 0x0101) && (data[2] <= 0x1f1f) && (data[3] <= 0x3c3c) && (data[4] <= 0xea60))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

        /*未读取到有效参数，写入默认时间2020/01/01 00:00:00*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 2020;
    data[2] = 0x0101;
    data[3] = 0;
    data[4] = 0;
    (void)DEVEXTEEPWriteWord(PARA_SYS_CLOCK_ADDR + 2, &data[1], (len - 1));
    (void)DEVEXTEEPWriteWord(PARA_SYS_CLOCK_ADDR, &data[0], 1);

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepSysRunTime(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统运行(上电)时长 1Hour
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSysRunTime(u16 *data, u8 len)
{
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_SYS_RUNTIME_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        DEVEXTEEPReadWord(PARA_SYS_RUNTIME_ADDR, data, len);

        /*读取到有效参数*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0;
    DEVEXTEEPWriteWord(PARA_SYS_RUNTIME_ADDR + 2, &data[1], (len - 1));
    DEVEXTEEPWriteWord(PARA_SYS_RUNTIME_ADDR, &data[0], 1);

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepChgDhgTimes(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取充放电次数
** @The notes :
**===============================================================================================*/
u8 ReadExtEepChgDhgTimes(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_CHG_TIMES_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadWord(PARA_CHG1_TIMES_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadWord(PARA_CHG2_TIMES_ADDR, data, len);
        }

        /*读取到有效参数*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 1;
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_CHG1_TIMES_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_CHG1_TIMES_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_CHG2_TIMES_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_CHG2_TIMES_ADDR, &data[0], 1);
    }
    return(FALSE);
}
/**=================================================================================================
** @Name      : u8 ReadExtEepHisDodInfoV2(u8 clunum, u16 *data, u16 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度(u16=PARA_DOD_HIS_V2_LEN,502)
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取历史DOD记录信息V3(标志+Id+50条环形记录×10字)
** @The notes : 逐条校验:标志有效+Id范围+记录id瓤槽不变量+DOD越界;失败回写默认值
**               驱动内部自动分段(每次64B),1004字节整块读安全
**===============================================================================================*/
u8 ReadExtEepHisDodInfoV2(u8 clunum, u16 *data, u16 len)
{
    u8 trytime = 3;
    u8 valid = TRUE;
    u16 i = 0;
    u16 id = 0;
    u16 rid = 0;
    u16 dod = 0;
    u16 baseAddr = 0;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }
    /*输入参数无效*/
    if((NULL == data) || (len != PARA_DOD_HIS_V2_LEN))
    {
        return(FALSE);
    }
    baseAddr = (0 == clunum) ? PARA_DOD1_HIS_V2_ADDR : PARA_DOD2_HIS_V2_ADDR;

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        DEVEXTEEPReadWord(baseAddr, data, len);
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            id = data[eHisDodRing_Id];
            /*累计次数上限(回绕保护)*/
            if(id > 60000)
            {
                break;
            }
            /*逐槽校验:记录id瓤槽不变量slot=(id-1)%N,记录id≤头Id,DOD越界*/
            for(i = 0; i < HISDOD_REC_NUM; i++)
            {
                rid = data[eHisDodRing_Rec + (i * eHisDodRec_Len) + eHisDodRec_Id];
                dod = data[eHisDodRing_Rec + (i * eHisDodRec_Len) + eHisDodRec_Dod];
                if(0 == rid)
                {
                    continue;   /*空槽(仅Id<N时存在)*/
                }
                if((rid > id) || (((rid - 1) % HISDOD_REC_NUM) != i) || (dod > PARA_DOD_HIS_VAL_MAX))
                {
                    valid = FALSE;
                    break;
                }
            }
            if(TRUE == valid)
            {
                return(TRUE);
            }
            /*数据损坏,重试同样读出坏数据,直接跳出写默认*/
            break;
        }
    }

    /*未读取到有效参数,写入默认值(标志+全零)*/
    data[0] = PARA_CFG_FLAG_U16;
    for(i = 1; i < len; i++)
    {
        data[i] = 0;
    }
    DEVEXTEEPWriteWord(baseAddr + 2, &data[1], (len - 1));
    DEVEXTEEPWriteWord(baseAddr, &data[0], 1);
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepHisErrNum(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取历史故障告警数目
** @The notes :
**===============================================================================================*/
u8 ReadExtEepHisErrNum(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_HIS_ERRNUM_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadWord(PARA_HIS1_ERRNUM_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadWord(PARA_HIS2_ERRNUM_ADDR, data, len);
        }


        /*读取到有效参数*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_HIS1_ERRNUM_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_HIS1_ERRNUM_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_HIS2_ERRNUM_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_HIS2_ERRNUM_ADDR, &data[0], 1);
    }
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepDODStaInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum: 组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取放电深度DOD统计信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepDODStaInfo(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_DOD_INFO_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadWord(PARA_DOD1_INFO_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadWord(PARA_DOD2_INFO_ADDR, data, len);
        }

        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0]) && (data[1] <= 1000)
        	&& (data[2] <= 1000) && (data[3] <= 1000) && (data[4] <= 1000))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_DOD1_INFO_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_DOD1_INFO_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_DOD2_INFO_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_DOD2_INFO_ADDR, &data[0], 1);
    }

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepSOSLockInfo(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取安全等级SOS锁定信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSOSLockInfo(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_SOS_INFO_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadWord(PARA_SOS1_INFO_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadWord(PARA_SOS2_INFO_ADDR, data, len);
        }


        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0]) && (data[1] <= 4) && (data[3] <= 4))
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }
    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_SOS1_INFO_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_SOS1_INFO_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_SOS2_INFO_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_SOS2_INFO_ADDR, &data[0], 1);
    }

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepHisChgDhgCap(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取累计充放电电量 1AH
** @The notes :
**===============================================================================================*/
u8 ReadExtEepHisChgDhgCap(u8 clunum, u32 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_HIS_CHGCAP_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadLong(PARA_HIS1_CHGCAP_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadLong(PARA_HIS2_CHGCAP_ADDR, data, len);
        }


        /*读取到有效参数*/
        if(PARA_CFG_FLAG_U32 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U32;
    data[1] = 0;
    data[2] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteLong(PARA_HIS1_CHGCAP_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_HIS1_CHGCAP_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteLong(PARA_HIS2_CHGCAP_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_HIS2_CHGCAP_ADDR, &data[0], 1);
    }


    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepHisChgDhgEner(u8 clunum, u32 *data, u8 len)
** @Input     : clunum:组号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取累计充放电电能 0.1kWh
** @The notes :
**===============================================================================================*/
u8 ReadExtEepHisChgDhgEner(u8 clunum, u32 *data, u8 len)
{
    u8 trytime = 5;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_HIS_CHGENER_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadLong(PARA_HIS1_CHGENER_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadLong(PARA_HIS2_CHGENER_ADDR, data, len);
        }


        /*读取到有效参数*/
        if(PARA_CFG_FLAG_U32 == data[0])
        {
            return(TRUE);
        }
        //BSPWatchDogOutput();
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U32;
    data[1] = 0;
    data[2] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteLong(PARA_HIS1_CHGENER_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_HIS1_CHGENER_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteLong(PARA_HIS2_CHGENER_ADDR + 4, &data[1], (len - 1));
        DEVEXTEEPWriteLong(PARA_HIS2_CHGENER_ADDR, &data[0], 1);
    }
    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepGroupGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电池组主控通用参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepGroupGenParaInfo(u16 *data, u8 len)
{
	u8 i = 0;
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len <= 1) || (len > PARA_DEVGEN_PARA_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
    	DEVEXTEEPReadWord(PARA_DEVGEN_PARA_ADDR, &data[0], 1);
        /*读取到有效参数标志*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            /*读取所有参数值*/
            DEVEXTEEPReadWord(PARA_DEVGEN_PARA_ADDR + 2, &data[1], (len - 1));
            /*逐个检测参数是否有效*/
        	for(i = 1; i < len; i++)
        	{
                /*读取到无效参数值*/
                if(FALSE == ParaCheckGGenParaEffect(PARA_DEVGEN_PARA_ADDR + (2 * i), data[i]))
                {
                	/*取默认值,清除配置标志*/
                	data[i] = gGBmuGenPara_102[i];
                	data[0] = 0xffff;
                }
        	}
            break;
        }
        //BSPWatchDogOutput();
    }

    /*读取到所有参数有效*/
    if(PARA_CFG_FLAG_U16 == data[0])
    {
    	return(TRUE);
    }
    /*未读取到有效参数*/
    else
    {
        /*未读取到配置标志*/
        if((0 == trytime) || (0xff == trytime))
        {
            /*写全部默认值*/
            DEVEXTEEPWriteWord(PARA_DEVGEN_PARA_ADDR + 2, &gGBmuGenPara_102[1], (len - 1));
        }
        else
        {
            /*写修改后的有效值*/
            DEVEXTEEPWriteWord(PARA_DEVGEN_PARA_ADDR + 2, &data[1], (len -1));
        }
        DEVEXTEEPWriteWord(PARA_DEVGEN_PARA_ADDR, &gGBmuGenPara_102[0], 1);
        return(FALSE);
    }

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepGroupHigParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电池组主控高级参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepGroupHigParaInfo(u16 *data, u8 len)
{
	u8 i = 0;
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len <= 1) || (len > PARA_DEVHIG_PARA_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
    	DEVEXTEEPReadWord(PARA_DEVHIG_PARA_ADDR, &data[0], 1);
        /*读取到有效参数标志*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            /*读取所有参数值*/
            DEVEXTEEPReadWord(PARA_DEVHIG_PARA_ADDR + 2, &data[1], (len - 1));
            /*逐个检测参数是否有效*/
        	for(i = 1; i < len; i++)
        	{
                /*读取到无效参数值*/
                if(FALSE == ParaCheckGHigParaEffect(PARA_DEVHIG_PARA_ADDR + (2 * i), data[i]))
                {
                	/*取默认值,清除配置标志*/
                	data[i] = gGBmuHigLevPara_103[i];
                	data[0] = 0xffff;
                }
        	}
            break;
        }
        //BSPWatchDogOutput();
    }

    /*读取到所有参数有效*/
    if(PARA_CFG_FLAG_U16 == data[0])
    {
    	return(TRUE);
    }
    /*未读取到有效参数*/
    else
    {
        /*未读取到配置标志*/
        if((0 == trytime) || (0xff == trytime))
        {
            /*写全部默认值*/
            DEVEXTEEPWriteWord(PARA_DEVHIG_PARA_ADDR + 2, &gGBmuHigLevPara_103[1], (len - 1));
        }
        else
        {
            /*写修改后的有效值*/
            DEVEXTEEPWriteWord(PARA_DEVHIG_PARA_ADDR + 2, &data[1], (len - 1));
        }
        DEVEXTEEPWriteWord(PARA_DEVHIG_PARA_ADDR, &gGBmuHigLevPara_103[0], 1);
        return(FALSE);
    }

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepGroupHWParaInfo(u8 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取电池组主控硬件参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepGroupHWParaInfo(u8 *data, u8 len)
{
	u8 i = 0;
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len <= 1) || (len > PARA_DEVHW_PARA_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
    	DEVEXTEEPReadByte(PARA_DEVHW_PARA_ADDR, &data[0], 1);
        /*读取到有效参数标志*/
        if(PARA_CFG_FLAG_U8 == data[0])
        {
            /*读取所有参数值*/
            DEVEXTEEPReadByte(PARA_DEVHW_PARA_ADDR + 1, &data[1], (len - 1));
            /*逐个检测参数是否有效*/
        	for(i = 1; i < len; i++)
        	{
                /*读取到无效参数值*/
                if(FALSE == ParaCheckGHWParaEffect(PARA_DEVHW_PARA_ADDR + i, data[i]))
                {
                	/*取默认值,清除配置标志*/
                	data[i] = gGHardPara_104[i];
                	data[0] = 0xff;
                }
        	}
            break;
        }
        //BSPWatchDogOutput();
    }

    /*读取到所有参数有效*/
    if(PARA_CFG_FLAG_U8 == data[0])
    {
    	return(TRUE);
    }
    /*未读取到有效参数*/
    else
    {
        /*未读取到配置标志*/
        if((0 == trytime) || (0xff == trytime))
        {
            /*写全部默认值*/
            DEVEXTEEPWriteByte(PARA_DEVHW_PARA_ADDR + 1, &gGHardPara_104[1], (len - 1));
        }
        else
        {
            /*写修改后的有效值*/
            DEVEXTEEPWriteByte(PARA_DEVHW_PARA_ADDR + 1, &data[1], (len - 1));
        }
        DEVEXTEEPWriteByte(PARA_DEVHW_PARA_ADDR, &gGHardPara_104[0], 1);
        return(FALSE);
    }

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepSystemWorkMode(u8 clunum, u16 *data, u8 len)
** @Input     : clunum:簇编号 data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统工作模式
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSystemWorkMode(u8 clunum, u16 *data, u8 len)
{
    u8 trytime = 5;

    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len != PARA_SYSWORK_MOD_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
        if(clunum == 0)
        {
            DEVEXTEEPReadWord(PARA_SYSWORK1_MOD_ADDR, data, len);
        }
        else
        {
            DEVEXTEEPReadWord(PARA_SYSWORK2_MOD_ADDR, data, len);
        }
        

        /*读取到有效参数*/
        if((PARA_CFG_FLAG_U16 == data[0]) && (data[1] <= 1))
        {
            return(TRUE);
        }
    }

    /*未读取到有效参数，写入默认值*/
    data[0] = PARA_CFG_FLAG_U16;
    data[1] = 0;
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_SYSWORK1_MOD_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_SYSWORK1_MOD_ADDR, &data[0], 1);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_SYSWORK2_MOD_ADDR + 2, &data[1], (len - 1));
        DEVEXTEEPWriteWord(PARA_SYSWORK2_MOD_ADDR, &data[0], 1);
    }


    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepSystemGenParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统通用参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSystemGenParaInfo(u16 *data, u8 len)
{
	u8 i = 0;
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len <= 1) || (len > PARA_SYSGEN_PARA_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
    	DEVEXTEEPReadWord(PARA_SYSGEN_PARA_ADDR, &data[0], 1);
        printf("PARA_SYSGEN_PARA_ADDR data[0] = %d\r\n",data[0]);
        /*读取到有效参数标志*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            /*读取所有参数值*/
            DEVEXTEEPReadWord(PARA_SYSGEN_PARA_ADDR + 2, &data[1], (len - 1));
            /*逐个检测参数是否有效*/
        	for(i = 1; i < len; i++)
        	{
                /*读取到无效参数值*/
                if(FALSE == ParaCheckSGenParaEffect(PARA_SYSGEN_PARA_ADDR + (2 * i), data[i]))
                {
                	/*取默认值,清除配置标志*/
                	data[i] = gSysGenParaRW_105[i];
                	data[0] = 0xffff;
                }
        	}
            break;
        }
    }


    /*读取到所有参数有效*/
    if(PARA_CFG_FLAG_U16 == data[0])
    {
    	return(TRUE);
    }
    /*未读取到有效参数*/
    else
    {
        /*未读取到配置标志*/
        if((0 == trytime) || (0xff == trytime))
        {
            /*写全部默认值*/
            DEVEXTEEPWriteWord(PARA_SYSGEN_PARA_ADDR + 2, &gSysGenParaRW_105[1], (len - 1));
        }
        else
        {
            /*写修改后的有效值*/
            DEVEXTEEPWriteWord(PARA_SYSGEN_PARA_ADDR + 2, &data[1], (len - 1));
        }
        DEVEXTEEPWriteWord(PARA_SYSGEN_PARA_ADDR, &gSysGenParaRW_105[0], 1);
        return(FALSE);
    }

    return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ReadExtEepSystemHWParaInfo(u16 *data, u8 len)
** @Input     : data:读取到的数据 len:读取长度
** @Output    : 读取成功或失败
** @Function  : 从外部EEPROM读取系统设备硬件参数信息
** @The notes :
**===============================================================================================*/
u8 ReadExtEepSystemHWParaInfo(u16 *data, u8 len)
{
	u8 i = 0;
    u8 trytime = 5;

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((NULL == data) || (len <= 1) || (len > PARA_SYSHW_PARA_LEN))
    {
        return(FALSE);
    }

    /*从外部EEPROM读取参数*/
    while(trytime--)
    {
    	DEVEXTEEPReadWord(PARA_SYSHW_PARA_ADDR, &data[0], 1);
        /*读取到有效参数标志*/
        if(PARA_CFG_FLAG_U16 == data[0])
        {
            /*读取所有参数值*/
            DEVEXTEEPReadWord(PARA_SYSHW_PARA_ADDR + 2, &data[1], (len - 1));
            /*逐个检测参数是否有效*/
        	for(i = 1; i < len; i++)
        	{
                /*读取到无效参数值*/
                if(FALSE == ParaCheckSHWParaEffect(PARA_SYSHW_PARA_ADDR + (2 * i), data[i]))
                {
                	/*取默认值,清除配置标志*/
                	data[i] = gSysHigParaRO_101[i];
                	data[0] = 0xffff;
                }
        	}
            break;
        }
        //BSPWatchDogOutput();
    }

    /*读取到所有参数有效*/
    if(PARA_CFG_FLAG_U16 == data[0])
    {
    	return(TRUE);
    }

    /*未读取到有效参数*/
    else
    {
        /*未读取到配置标志*/
        if((0 == trytime) || (0xff == trytime))
        {
            /*写全部默认值*/
            DEVEXTEEPWriteWord(PARA_SYSHW_PARA_ADDR + 2, &gSysHigParaRO_101[1], (len - 1));
        }
        else
        {
            /*写修改后的有效值*/
            DEVEXTEEPWriteWord(PARA_SYSHW_PARA_ADDR + 2, &data[1], (len - 1));
        }
        DEVEXTEEPWriteWord(PARA_SYSHW_PARA_ADDR, &gSysHigParaRO_101[0], 1);
        return(FALSE);
    }

    return(FALSE);
}
//BMU01未使用外部EEPROM单个写，可能会造成失败
#if(1)
/*=================================================================================================
** @Name      : u8 WriteExtEepNowCapSoh(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入当前总容量SOH值
** @The notes :
**===============================================================================================*/
u8 WriteExtEepNowCapSoh(u8 clunum, u8 index, u16 *data, u8 len)
{
    /*外部EEPROM读写异常*/
     if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_NOW_CAP_SOH_LEN))
    {
        return(FALSE);
    }

	/*向外部EEPROM写入参数值*/
    if(clunum == 0)
    {
	    DEVEXTEEPWriteWord(PARA_NOW_CAP1_SOH_ADDR + (2 * index), data, len);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_NOW_CAP2_SOH_ADDR + (2 * index), data, len);
    }

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepCorrZeroCurr(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电流零飘修正值
** @The notes :
**===============================================================================================*/
u8 WriteExtEepCorrZeroCurr(u8 clunum, u8 index, u16 *data, u8 len)
{
    /*外部EEPROM读写异常*/
     if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }


    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_ZERO_C_CORR_LEN))
    {
        return(FALSE);
    }

    /*逐个检测参数有效性*/
    for(u8 i = 0; i < len; i++)
    {
		/*写零飘误差值*/
		if(1 == (index + i))
		{
			/*需写入的参数无效*/
			if(((s16)data[i] < (0 - (s16)MAX_CURR_CORR_10MA))
				|| ((s16)data[i] > (s16)MAX_CURR_CORR_10MA))
			{
				return(FALSE);
			}
		}
    }

	/*向外部EEPROM写入参数值*/
    if(clunum == 0)
    {
	    DEVEXTEEPWriteWord(PARA_ZERO_C1_CORR_ADDR + (2 * index), data, len);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_ZERO_C2_CORR_ADDR + (2 * index), data, len);
    }

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepChgTimesState(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入充放电次数检测状态
** @The notes :
**===============================================================================================*/
u8 WriteExtEepChgTimesState(u8 clunum, u8 index, u32 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_CHGTIMES_STA_LEN))
    {
        return(FALSE);
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_CHGTIMES1_STA_ADDR + (4 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_CHGTIMES2_STA_ADDR + (4 * index), data, len);
    }


    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入总容量总能量自动标定信息值
** @The notes :
**===============================================================================================*/
u8 WriteExtEepCapEnerCorrInfo(u8 clunum, u8 index, u32 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }
    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_CAPENER_CORR_LEN))
    {
        return(FALSE);
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_CAPENER1_CORR_ADDR + (4 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_CAPENER2_CORR_ADDR + (4 * index), data, len);
    }

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电量计算结构信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepCapFormInfo(u8 clunum, u8 index, u32 *data, u8 len)
{
	u8 i = 0;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_CAP_FORM_LEN))
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*需写入的参数无效*/
		if((0 != (index + i)) && (data[i] > MAX_ALL_CAP_CFG))
		{
			return(FALSE);
		}
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_CAP1_FORM_ADDR + (4 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_CAP2_FORM_ADDR + (4 * index), data, len);
    }


    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepEnerFormInfo(u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电能计算结构信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepEnerFormInfo(u8 clunum,u8 index, u32 *data, u8 len)
{
    u8 i = 0;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_ENER_FORM_LEN))
    {
        return(FALSE);
    }
	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*需写入的参数无效*/
    	if((0 != (index + i)) && (data[i] > MAX_ALL_ENER_CFG))
		{
			return(FALSE);
		}
    }
    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_ENER1_FORM_ADDR + (4 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_ENER2_FORM_ADDR + (4 * index), data, len);
    }

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepSysClock(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统运行(上电)时长 1Hour
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSysClock(u8 index, u16 *data, u8 len)
{
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_SYS_RUNTIME_LEN))
    {
        return(FALSE);
    }

	/*延时 1ms*/
	//BSPSetOSMicrosecDelayAPI(1000);

	/*向外部EEPROM写入参数值*/
	DEVEXTEEPWriteWord(PARA_SYS_CLOCK_ADDR + (2 * index), data, len);

    return(TRUE);
}

/*=================================================================================================
** @Name      : u8 WriteExtEepSysRunTime(u8 index, u16 data)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统运行(上电)时长 1Hour
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSysRunTime(u8 index, u16 *data, u8 len)
{
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_SYS_RUNTIME_LEN))
    {
        return(FALSE);
    }

	/*延时 1ms*/
	//BSPSetOSMicrosecDelayAPI(1000);

	/*向外部EEPROM写入参数值*/
	DEVEXTEEPWriteWord(PARA_SYS_RUNTIME_ADDR + (2 * index), data, len);

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepChgDhgTimes(u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入充放电次数
** @The notes :
**===============================================================================================*/
u8 WriteExtEepChgDhgTimes(u8 clunum,u8 index, u16 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_CHG_TIMES_LEN))
    {
        return(FALSE);
    }
    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_CHG1_TIMES_ADDR + (2 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_CHG2_TIMES_ADDR + (2 * index), data, len);
    }


    return(TRUE);
}
/**=================================================================================================
** @Name      : u8 WriteExtEepHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len)
** @Input     : clunum:组号 index:字偏移 data:需要写入的数据 len:写入长度(u16)
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入历史DOD记录信息V2(整块或部分写入)
** @The notes : 底层驱动自动处理跨页/跨64字节分块
**===============================================================================================*/
u8 WriteExtEepHisDodInfoV2(u8 clunum, u16 index, u16 *data, u16 len)
{
    u16 baseAddr = 0;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }
    /*输入参数无效*/
    if((NULL == data) || (0 == len) || ((u32)index + len > PARA_DOD_HIS_V2_LEN))
    {
        return(FALSE);
    }
    baseAddr = (0 == clunum) ? PARA_DOD1_HIS_V2_ADDR : PARA_DOD2_HIS_V2_ADDR;
    DEVEXTEEPWriteWord(baseAddr + (2 * index), data, len);
    return(TRUE);
}

/**=================================================================================================
** @Name      : u8 WriteExtEepHisDodInfoRec(u8 clunum, u16 slot, u16 *rec)
** @Input     : clunum:组号 slot:记录槽位[0,HISDOD_REC_NUM) rec:单条记录(10个u16)
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入单条DOD记录V3(仅20字节,减少E2磨损)
** @The notes : 供环形缓冲增量写入;写头(Id)必须在写记录之后(掉电一致性)
**===============================================================================================*/
u8 WriteExtEepHisDodInfoRec(u8 clunum, u16 slot, u16 *rec)
{
    u16 baseAddr = 0;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }
    /*输入参数无效*/
    if((NULL == rec) || (slot >= HISDOD_REC_NUM))
    {
        return(FALSE);
    }
    baseAddr = (0 == clunum) ? PARA_DOD1_HIS_V2_ADDR : PARA_DOD2_HIS_V2_ADDR;
    DEVEXTEEPWriteWord(baseAddr + (2 * (eHisDodRing_Rec + (slot * eHisDodRec_Len))), rec, eHisDodRec_Len);
    return(TRUE);
}

/*=================================================================================================
** @Name      : u8 WriteExtEepHisErrNum(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入历史故障告警数目
** @The notes :
**===============================================================================================*/
u8 WriteExtEepHisErrNum(u8 clunum, u8 index, u16 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_HIS_ERRNUM_LEN))
    {
        return(FALSE);
    }
    
    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_HIS1_ERRNUM_ADDR + (2 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_HIS2_ERRNUM_ADDR + (2 * index), data, len);
    }


    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepDODStaInfo(u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入放电深度统计信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepDODStaInfo(u8 clunum, u8 index, u16 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_DOD_INFO_LEN))
    {
        return(FALSE);
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_DOD1_INFO_ADDR + (2 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_DOD2_INFO_ADDR + (2 * index), data, len);
    }


    return(TRUE);
}

/*=================================================================================================
** @Name      : u8 WriteExtEepSOSLockInfo(u8 clunum,u8 index, u16 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入安全等级信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSOSLockInfo(u8 clunum,u8 index, u16 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_SOS_INFO_LEN))
    {
        return(FALSE);
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_SOS1_INFO_ADDR + (2 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteWord(PARA_SOS2_INFO_ADDR + (2 * index), data, len);
    }


    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入累计充放电电量 1AH
** @The notes :
**===============================================================================================*/
u8 WriteExtEepHisChgDhgCap(u8 clunum, u8 index, u32 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_HIS_CHGCAP_LEN))
    {
        return(FALSE);
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_HIS1_CHGCAP_ADDR + (4 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_HIS2_CHGCAP_ADDR + (4 * index), data, len);
    }


    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len)
** @Input     : clunum:组号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入累计充放电电能 0.1kWh
** @The notes :
**===============================================================================================*/
u8 WriteExtEepHisChgDhgEner(u8 clunum, u8 index, u32 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_HIS_CHGENER_LEN))
    {
        return(FALSE);
    }

    if(clunum == 0)
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_HIS1_CHGENER_ADDR + (4 * index), data, len);
    }
    else
    {
        /*向外部EEPROM写入参数值*/
        DEVEXTEEPWriteLong(PARA_HIS2_CHGENER_ADDR + (4 * index), data, len);
    }

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepMonErrTime(u8 index, u16 data)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入月历史故障统计时间(年月)
** @The notes :
**===============================================================================================*/
u8 WriteExtEepMonErrTime(u8 clunum, u8 index, u16 *data, u8 len)
{
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }

    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_MONERR_TIME_LEN))
    {
        return(FALSE);
    }
	/*延时 1ms*/
	//BSPSetOSMicrosecDelayAPI(1000);

	/*向外部EEPROM写入参数值*/
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_MONERR1_TIME_ADDR + (2 * index), data, len);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_MONERR2_TIME_ADDR + (2 * index), data, len);
    }
	

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepGroupGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电池组主控通用参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepGroupGenParaInfo(u8 index, u16 *data, u8 len)
{
    u8 i = 0;
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_DEVGEN_PARA_LEN))
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*参数非有效值*/
		if(FALSE == ParaCheckGGenParaEffect(PARA_DEVGEN_PARA_ADDR + (2 * (index + i)), data[i]))
		{
			return(FALSE);
		}
    }

	/*向外部EEPROM写入参数值*/
	DEVEXTEEPWriteWord(PARA_DEVGEN_PARA_ADDR + (2 * index), data, len);

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepGroupHigParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电池组主控高级参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepGroupHigParaInfo(u8 index, u16 *data, u8 len)
{
    u8 i = 0;
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_DEVHIG_PARA_LEN))
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*参数非有效值*/
		if(FALSE == ParaCheckGHigParaEffect(PARA_DEVHIG_PARA_ADDR + (2 * (index + i)), data[i]))
		{
			return(FALSE);
		}
    }

	/*向外部EEPROM写入参数值*/
	DEVEXTEEPWriteWord(PARA_DEVHIG_PARA_ADDR + (2 * index), data, len);

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepGroupHWParaInfo(u8 index, u8 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入电池组主控硬件参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepGroupHWParaInfo(u8 index, u8 *data, u8 len)
{
    u8 i = 0;
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_DEVHW_PARA_LEN))
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*参数非有效值*/
		if(FALSE == ParaCheckGHWParaEffect(PARA_DEVHW_PARA_ADDR + index + i, data[i]))
		{
			return(FALSE);
		}
    }

	/*向外部EEPROM写入参数值*/
	DEVEXTEEPWriteByte(PARA_DEVHW_PARA_ADDR + index, data, len);

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len)
** @Input     : clunum:簇编号 index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统工作模式信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSystemWorkMode(u8 clunum, u8 index, u16 *data, u8 len)
{
    u8 i = 0;
    if(clunum >= CLUTER_MAX_NUM)
    {
        return(FALSE);
    }
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_SYSWORK_MOD_LEN))
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*参数非有效值*/
		if((1 == (index + i)) && (data[i] > 1))
		{
			return(FALSE);
		}
    }

    /*延时 1ms*/
    //BSPSetOSMicrosecDelayAPI(1000);

	/*向外部EEPROM写入参数值*/
    if(clunum == 0)
    {
        DEVEXTEEPWriteWord(PARA_SYSWORK1_MOD_ADDR + (2 * index), data, len);
    }
    else
    {
        DEVEXTEEPWriteWord(PARA_SYSWORK2_MOD_ADDR + (2 * index), data, len);
    }
	

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepSystemGenParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统设备通用参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSystemGenParaInfo(u8 index, u16 *data, u8 len)
{
    u8 i = 0;
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if((0 == len) || ((index + len) > PARA_SYSGEN_PARA_LEN))
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*参数非有效值*/
		if(FALSE == ParaCheckSGenParaEffect(PARA_SYSGEN_PARA_ADDR + (2 * (index + i)), data[i]))
		{
			return(FALSE);
		}
    }

	/*向外部EEPROM写入参数值*/
	(void)DEVEXTEEPWriteWord(PARA_SYSGEN_PARA_ADDR + (2 * index), data, len);

    return(TRUE);
}
/*=================================================================================================
** @Name      : u8 WriteExtEepSystemHWParaInfo(u8 index, u16 *data, u8 len)
** @Input     : index:参数索引 data:需要写入的数据
** @Output    : 写入成功或失败
** @Function  : 向外部EEPROM写入系统设备硬件参数信息
** @The notes :
**===============================================================================================*/
u8 WriteExtEepSystemHWParaInfo(u8 index, u16 *data, u8 len)
{
    u8 i = 0;
    /*外部EEPROM读写异常*/
    if(FALSE == ParaCheckExtEepReadyHook())
    {
        return(FALSE);
    }

    /*输入参数无效*/
    if(index >= PARA_SYSHW_PARA_LEN)
    {
        return(FALSE);
    }

	/*逐个检测参数有效性*/
    for(i = 0; i < len; i++)
    {
		/*参数非有效值*/
		if(FALSE == ParaCheckSHWParaEffect(PARA_SYSHW_PARA_ADDR + (2 * (index + i)), data[i]))
		{
			return(FALSE);
		}
    }

	/*向外部EEPROM写入参数值*/
	DEVEXTEEPWriteWord(PARA_SYSHW_PARA_ADDR + (2 * index), data, len);

    return(TRUE);
}
#endif



