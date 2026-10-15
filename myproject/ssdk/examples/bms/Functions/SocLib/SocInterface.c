
/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocInterface.c
** @Founder      : ZQD808
** @Date         : 2026.03.13
** @Function     : SOC算法包模块接口函数
** @Instructions :
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupInfo.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include "ParaInterface.h"
#include "DiagInterface.h"
#include "CalcInterface.h"
#include "SocInterface.h"
#include "ParaRead.h"
#include "ParaWrite.h"
const u16 cTempCapPercent[CAPENER_T_PER_LEN] =  /*不同温度下可用总容量百分比(0.1%)*/
{
    820, 860, 890, 910, 950,                    /*-30/-20/-10/0/10℃对应的可用总容量百分比*/
    980, 1000, 1000, 980, 950                   /*20/30/40/50/60℃对应的可用总容量百分比*/
};

const u16 cTempEnerPercent[CAPENER_T_PER_LEN] = /*不同温度下可用总能量百分比(0.1%)*/
{
    800, 850, 880, 900, 950,                    /*-30/-20/-10/0/10℃对应的可用总能量百分比*/
    980, 1000, 1000, 980, 950                   /*20/30/40/50/60℃对应的可用总能量百分比*/
};

const u16 cCurrCapPercent[CAPENER_C_PER_LEN] =  /*不同电流下可用总容量百分比(0.1%)*/
{
    990, 980, 973, 969, 966,                    /*0.2C/0.4C/0.6C/0.8C/1C对应的可用总容量百分比*/
    963, 961, 959, 957, 955                     /*1.2C/1.4C/1.6C/1.8C/2C对应的可用总容量百分比*/
};

const u16 cCurrEnerPercent[CAPENER_C_PER_LEN] = /*不同电流下可用总能量百分比(0.1%)*/
{
    985, 975, 970, 965, 960,                    /*0.2C/0.4C/0.6C/0.8C/1C对应的可用总能量百分比*/
    958, 955, 952, 949, 945                     /*1.2C/1.4C/1.6C/1.8C/2C对应的可用总能量百分比*/
};

extern u16 gGBmuGenPara_102[];                  /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/

/*=============================================================================================
** @Name      : u8 SocReadCapEnerCorrInfoHook(u8 clunum, u32 *info, u8 len)
** @Input     : clunum:簇编号 info:读取到的信息 len:读取长度
** @Output    : 读取结果:TRUE:成功 FALSE:失败
** @Function  : 读取容量和能量标定检测信息
** @The notes : 从EEPROM读取
**===========================================================================================*/
u8 SocReadCapEnerCorrInfoHook(u8 clunum, u32 *info, u8 len)
{
    /*参数合法*/
    if((info != NULL) && (len == PARA_CAPENER_CORR_LEN) && (clunum <= 1))
    {
        return(ParaReadStoreCapEnerCorrInfo(clunum,info, len));
    }

    return(FALSE);
}

/*=============================================================================================
** @Name      : u8 SocWriteCapEnerCorrInfoHook(u8 clunum, u8 sort, u32 *info, u8 len)
** @Input     : clunum:簇编号 sort:记录类型(0:低压点信息 1:高压点信息) info:需记录的信息 len:数据长度
** @Output    : 记录结果:TRUE:成功 FALSE:失败
** @Function  : 记录容量能量自动标定检测信息
** @The notes : 记录到EEPROM存储(要求存储器必须已经写记录标志)
**===========================================================================================*/
u8 SocWriteCapEnerCorrInfoHook(u8 clunum, u8 sort, u32 *info, u8 len)
{
    if(clunum >= 2)
    {
        return(FALSE);
    }
    /*记录低压点信息*/
    if(0 == sort)
    {
        /*参数合法*/
        if((info != NULL) && (len < (PARA_CAPENER_CORR_LEN / 2)))
        {
            return(ParaWriteStoreCapEnerCorrInfo(clunum, 1, info, len));
        }
    }
    /*记录高压点信息*/
    else if(1 == sort)
    {
        /*参数合法*/
        if((info != NULL) && (len < (PARA_CAPENER_CORR_LEN / 2)))
        {
            return(ParaWriteStoreCapEnerCorrInfo(clunum,(1 + (PARA_CAPENER_CORR_LEN / 2)), info, len));
        }
    }

    return(FALSE);
}
/*=============================================================================================
** @Name      : u8 SocWriteCorrTotalCapEnerHook(u8 clunum, u8 sort, u32 *info, u8 len)
** @Input     : clunum:簇编号 sort:记录类型(0:低压点信息 1:高压点信息) info:需记录的信息 len:数据长度
** @Output    : 记录结果:TRUE:成功 FALSE:失败
** @Function  : 记录容量能量自动标定检测到的总容量总能量值
** @The notes : 记录到EEPROM存储(要求存储器必须已经写记录标志)
**===========================================================================================*/
u8 SocWriteCorrTotalCapEnerHook(u8 clunum, u8 sort, u32 *info, u8 len)
{
    if(clunum >= 2)
    {
        return(FALSE);
    }
    /*记录低压点信息*/
    if(0 == sort)
    {
        /*参数合法*/
        if((info != NULL) && (len <= 3))
        {
            return(ParaWriteStoreCapEnerCorrInfo(clunum,7, info, len));
        }
    }
    /*记录高压点信息*/
    else if(1 == sort)
    {
        /*参数合法*/
        if((info != NULL) && (len <= 3))
        {
            return(ParaWriteStoreCapEnerCorrInfo(clunum,16, info, len));
        }
    }

    return(FALSE);
}
/*=============================================================================================
** @Name      : u8 SocWriteCapEnerCorrTimeHook(u8 clunum, u8 sort, u32 time)
** @Input     : clunum:簇编号 sort:记录类型(0:低压点信息 1:高压点信息) time:标定时长(1s)
** @Output    : 记录结果:TRUE:成功 FALSE:失败
** @Function  : 记录容量能量自动标定时长信息
** @The notes : 记录到EEPROM存储(要求存储器必须已经写记录标志)
**===========================================================================================*/
u8 SocWriteCapEnerCorrTimeHook(u8 clunum, u8 sort, u32 time)
{
    if(clunum >= 2)
    {
        return(FALSE);
    }
    /*记录低压点信息*/
    if(0 == sort)
    {
        return(ParaWriteStoreCapEnerCorrInfo(clunum, 2, &time, 1));
    }
    /*记录高压点信息*/
    else if(1 == sort)
    {
        return(ParaWriteStoreCapEnerCorrInfo(clunum, 11, &time, 1));
    }

    return(FALSE);
}

