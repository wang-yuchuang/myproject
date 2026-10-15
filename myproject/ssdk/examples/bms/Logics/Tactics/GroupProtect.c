/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupProtect.c
** @Founder      : ZQD808
** @Date         : 2026.03.19
** @Function     : 电池组故障保护模块接口
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "TacUser.h"
#include "TacInterface.h"
#include "GroupTactic.h"
#include "GroupProtect.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
//static u16 sGProTimer[eProSort_Num] = {0};                  /*各类故障保护计时 0.1s*/
static u16 (*sGProTimer)[eProSort_Num] = NULL;
/*请求外部充放电设备保护标志(按位表示[bit0:充电限制保护 bit1:放电限制保护 bit2:充电保护 bit3:放电保护 bit4:充放电保护 bit5:故障状态保护[0未请求 1请求])*/
static u16 sGroupAskProFlag[2] = {0};
/*故障保护执行标志(按位表示[bit0:充电限制保护 bit1:放电限制保护 bit2:充电保护 bit3:放电保护 bit4:充放电保护 bit5:故障状态保护[0未请求 1请求])*/
static u16 sGroupHapProFlag[2] = {0};

static u8 sGSamProSort[2] = {0};                                 /*重复保护的故障类型(0无保护 1充电保护 2放电保护)*/
static u8 sGSamProTimes[2] = {0};                                /*重复保护次数(10分钟连续3次则不恢复)*/
static u32 sGSamProTimer[2] = {0};                               /*重复保护起始时间*/


extern u16 gGBmuGenPara_102[];                                  /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];                               /*主控高级参数值*/

static void GroupProCtrlCheck(u8 clunum, e_ErrProSort proSort);
static void GroupProCtrlClr(u8 clunum, e_ErrProSort proSort);
static void GroupProCtrlBySwitch(u8 clunum);

/*=============================================================================================
** @Name      : void CapInfoCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 调用此函数时，参数配置还没初始化，先按2簇的内存分配
**===========================================================================================*/
void GroupProtectDataInit(void)
{
    static u8 sFirstInitFlag = 0;
    if(sFirstInitFlag >= 1)
    {
        return;
    }
    // 分配 2 行 × eProSort_Num 个 u16 的连续内存
    sGProTimer = ExtMalloc(2 * sizeof(u16[eProSort_Num]));
    if (sGProTimer == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sGProTimer null!!!\r\n", __func__);
        return;
    }
    else
    {
        // 清零整个二维数组（模拟原静态数组的 {0} 初始化）
        memset(sGProTimer, 0, 2 * sizeof(u16[eProSort_Num]));
        printf("malloc sGProTimer %d success\r\n", 2 * sizeof(u16[eProSort_Num]));
    }
    sFirstInitFlag = 1;
}
/*=================================================================================================
** @Name      : void GroupProtectInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组保护策略初始化
** @The notes :
**===============================================================================================*/
void GroupProtectInit(void)
{
    u8 i = 0;
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) 
    {
        if(clunum >= 2)
        {
            return;
        }
        /*故障保护参数清零*/
        for(i = 0; i < eProSort_Num; i++)
        {
            sGProTimer[clunum][i] = 0;
        }

        sGroupAskProFlag[clunum] = 0;
        sGroupHapProFlag[clunum] = 0;
        sGSamProSort[clunum] = 0;
        sGSamProTimes[clunum] = 0;
        sGSamProTimer[clunum] = 0;
    }
}
/*=================================================================================================
** @Name      : void GroupProtectTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组保护策略执行任务
** @The notes : 100ms周期调用
**===============================================================================================*/
void GroupProtectTask(void)
{
    u8 errSort = 0;
    static u32 sDhgProTime[2] = {0};
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        if(clunum >= 2)
        {
            return;
        }
        /*系统调试模式不执行保护*/
        if(eMODE_DEBUG == GetGWorkModeAPI(clunum))
        {
            /*故障保护参数清零*/
            TacClrGroupProFlagAPI(clunum);
            continue;
        }

        /*系统待命非空闲保护检测*/
        if((eWORK_IDLE == GetGWorkStateAPI(clunum))               /*电池组系统待命*/
            && (eCURR_IDLE != GetGChgDhgStateAPI(clunum))         /*系统非空闲*/
            && ((GetGSampOutCurrAPI(clunum) <= -50)               /*系统有电流*/
            || (GetGSampOutCurrAPI(clunum) >= 50)))               /*系统有电流*/
        {
            GroupProCtrlCheck(clunum, eProSort_IdlExp);             /*检测是否保护*/
        }
        else if(eWORK_ERR != GetGWorkStateAPI(clunum))            /*电池组非故障*/
        {
            GroupProCtrlClr(clunum, eProSort_IdlExp);
        }

        /*获取当前全部故障级别*/
        errSort = TacGetGNowErrLevelHook(clunum, 0);

        /*系统故障保护检测*/
        if((eWORK_ERR == GetGWorkStateAPI(clunum))                /*电池组系统故障*/
            || (eErr_SysErr == errSort))                    /*系统故障类型故障*/
        {
            GroupProCtrlCheck(clunum, eProSort_SysErr);             /*检测是否保护*/
        }
        else
        {
            GroupProCtrlClr(clunum, eProSort_SysErr);
        }

        /*充放电保护检测(产生充放电保护类型告警故障[非故障状态但接触器需断开])*/
        if(eErr_SysPro == errSort)                          /*充放电保护类型故障*/
        {
            GroupProCtrlCheck(clunum, eProSort_SysPro);             /*检测是否保护*/
        }
        else
        {
            GroupProCtrlClr(clunum, eProSort_SysPro);
        }

        /*获取当前充电故障级别*/
        errSort = TacGetGNowErrLevelHook(clunum, 1);

        /*充电保护检测*/
        if((eErr_ChgPro == errSort)                         /*充电保护类型故障*/
            && (eCURR_CHG == GetGChgDhgStateAPI(clunum)))         /*仍在充电*/
        {
            GroupProCtrlCheck(clunum, eProSort_ChgPro);             /*检测是否保护*/
        }
        else if((eErr_ChgPro != errSort)                    /*非充电保护类型故障*/
            && (eErr_SysPro != errSort)                     /*非充放电保护类型故障*/
            && (eErr_SysErr != errSort))                    /*非故障保护类型故障*/
        {
            GroupProCtrlClr(clunum, eProSort_ChgPro);
        }

        /*有电池充满电压高(仍在充电)保护检测*/
        if(ePRO_FULL == GetGProtectStateAPI(clunum))              /*电池组充满*/
        {
            if((eCURR_CHG == GetGChgDhgStateAPI(clunum))          /*仍在充电*/
                && ((GetGCellMaxVoltAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_VH2Lim])/*电压仍过二级告警(电压回落不保护)*/
                || (GetGSampOutCurrAPI(clunum) <= -100)))         /*系统有大电流*/
            {
                GroupProCtrlCheck(clunum, eProSort_ChgLim);         /*检测是否保护*/
            }
            else
            {
                GroupProCtrlClr(clunum, eProSort_ChgLim);
            }
        }
        /*有充电限制故障(仍在充电)保护检测*/
        else if(((eErr_SysLim == errSort)                    /*充放电限制故障*/
            || (eErr_ChgLim == errSort))                    /*充电限制故障*/
            && (eCURR_CHG == GetGChgDhgStateAPI(clunum)))         /*仍在充电*/
        {
            GroupProCtrlCheck(clunum, eProSort_ChgLim);             /*检测是否保护*/
        }
        else
        {
            GroupProCtrlClr(clunum, eProSort_ChgLim);
        }

        /*获取当前放电故障级别*/
        errSort = TacGetGNowErrLevelHook(clunum, 2);

        /*放电保护检测*/
        if((eErr_DhgPro == errSort)                         /*放电保护类型故障*/
            && (eCURR_DHG == GetGChgDhgStateAPI(clunum)))         /*仍在放电*/
        {
            GroupProCtrlCheck(clunum, eProSort_DhgPro);             /*检测是否保护*/
        }
        else if((eErr_DhgPro == errSort)                    /*放电保护类型故障*/
            && (GetGCellMinVoltAPI(clunum) <= (gGBmuGenPara_102[eBmuGenPara102_VL1Lim] - 200)))/*最低电压低于保护值*/
        {
            if(sDhgProTime[clunum] >= 18000)                        /*延时30min*/
            {
                GroupProCtrlCheck(clunum, eProSort_DhgPro);         /*检测是否保护*/
            }
            else
            {
                sDhgProTime[clunum]++;
            }
        }
        else if((eErr_DhgPro != errSort)                    /*非放电保护类型故障*/
            && (eErr_SysPro != errSort)                     /*非充放电保护类型故障*/
            && (eErr_SysErr != errSort))                    /*非故障保护类型故障*/
        {
            sDhgProTime[clunum] = 0;
            GroupProCtrlClr(clunum, eProSort_DhgPro);
        }

        /*有电池放电截止电压低(仍在放电)保护检测*/
        if(ePRO_EMPTY == GetGProtectStateAPI(clunum))             /*电池组放空*/
        {
            if((eCURR_DHG == GetGChgDhgStateAPI(clunum))          /*仍在放电*/
                && ((GetGCellMinVoltAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_VL2Lim])/*电压仍低于二级告警(电压回升不保护)*/
                || (GetGSampOutCurrAPI(clunum) >= 100)))          /*系统有大电流*/
            {
                GroupProCtrlCheck(clunum, eProSort_DhgLim);         /*检测是否保护*/
            }
            else
            {
                GroupProCtrlClr(clunum, eProSort_DhgLim);
            }
        }
        /*有放电限制故障或(仍在放电)保护检测*/
        else if(((eErr_SysLim == errSort)                   /*充放电限制故障*/
            || (eErr_DhgLim == errSort))                    /*放电限制故障*/
            && (eCURR_DHG == GetGChgDhgStateAPI(clunum)))   /*仍在放电*/
        {
            GroupProCtrlCheck(clunum, eProSort_DhgLim);             /*检测是否保护*/
        }
        else
        {
            GroupProCtrlClr(clunum, eProSort_DhgLim);
        }
    }
}
/*=================================================================================================
** @Name      : void TacClrGroupProFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除电池组故障保护标志
** @The notes :
**===============================================================================================*/
void TacClrGroupProFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    sGroupAskProFlag[clunum] = 0;
    sGroupHapProFlag[clunum] = 0;
    sGSamProSort[clunum] = 0;
    sGSamProTimer[clunum] = 0;
    sGSamProTimes[clunum] = 0;
}
/*=================================================================================================
** @Name      : u8 TacGetGAskChgProFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组请求充电保护标志 0:未请求 1:请求
** @Function  : 获取电池组请求充电保护标志
** @The notes :
**===============================================================================================*/
u8 TacGetGAskChgProFlagAPI(u8 clunum)
{
    u8 state = 0;
    if(clunum >= 2)
    {
        return (0);
    }
    if((1 == BitGet(sGroupAskProFlag[clunum], eProSort_SysErr))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_SysPro))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_ChgPro))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_ChgLim)))
    {
        state = 1;
    }

    return(state);
}
/*=================================================================================================
** @Name      : u8 TacGetGAskDhgProFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组请求放电保护标志 0:未请求 1:请求
** @Function  : 获取电池组请求放电保护标志
** @The notes :
**===============================================================================================*/
u8 TacGetGAskDhgProFlagAPI(u8 clunum)
{
    u8 state = 0;
    if(clunum >= 2)
    {
        return (0);
    }

    if((1 == BitGet(sGroupAskProFlag[clunum], eProSort_SysErr))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_SysPro))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_DhgPro))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_DhgLim)))
    {
        state = 1;
    }

    return(state);
}
/*=================================================================================================
** @Name      : u8 TacGetGAskSysProFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组请求充放电保护标志 0:未请求 1:请求
** @Function  : 获取电池组请求充放电保护标志
** @The notes : 同时需要充放电保护
**===============================================================================================*/
u8 TacGetGAskSysProFlagAPI(u8 clunum)
{
    //只考虑充放电均保护的故障保护
    u8 state = 0;

    if(clunum >= 2)
    {
        return (0);
    }
    if((1 == BitGet(sGroupAskProFlag[clunum], eProSort_SysErr))         /*请求故障保护*/
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_SysPro))     /*请求充放电保护*/
        || (((1 == BitGet(sGroupAskProFlag[clunum], eProSort_ChgPro))   /*同时请求充电保护和放电保护*/
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_ChgLim)))
        && ((1 == BitGet(sGroupAskProFlag[clunum], eProSort_DhgPro))
        || (1 == BitGet(sGroupAskProFlag[clunum], eProSort_DhgLim)))))
    {
        state = 1;
    }

    return(state);
}

/*=================================================================================================
** @Name      : u16 TacGetGHappenProFlagAPI(u8 clunum)
** @Input     : void
** @Output    : 电池组产生充放电保护标志,按位表示:bit0:充电截止保护 bit1:放电截止保护 bit2:充电故障保护 bit3:放电故障保护 bit4:充放电故障保护 bit5:系统故障保护 (0:正常 1:产生保护)
** @Function  : 获取电池组产生充放电保护标志
** @The notes :
**===============================================================================================*/
u16 TacGetGHappenProFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sGroupHapProFlag[clunum]);
}
/*=================================================================================================
** @Name      : static void GroupProCtrlCheck(u8 clunum, e_ErrProSort proSort)
** @Input     : clunum:簇编号 proSort:故障保护类型[0,5]
** @Output    : void
** @Function  : 电池组故障保护检测
** @The notes : 判断是否达到保护延时和执行保护
**===============================================================================================*/
static void GroupProCtrlCheck(u8 clunum, e_ErrProSort proSort)
{
    if(sGProTimer == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sGProTimer null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    switch(proSort)
    {
        /*系统待命保护*/
        case eProSort_IdlExp:
            BitSet(sGroupAskProFlag[clunum], eProSort_IdlExp);                                                  /*设置请求保护*/
            if(sGProTimer[clunum][eProSort_IdlExp] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ErrProOffT] * 10)) /*达到保护时间*/
            {
                GroupProCtrlBySwitch(clunum);                                                                 /*断开相应运行接触器保护*/
                BitSet(sGroupHapProFlag[clunum], eProSort_IdlExp);                                              /*设置执行保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_IdlExp]++;
            }
            break;

        /*系统故障保护*/
        case eProSort_SysErr:
            BitSet(sGroupAskProFlag[clunum], eProSort_SysErr);                                                  /*设置请求保护*/
            if(((GetGChgLimCurrAPI(clunum) >= -50) && (GetGChgLimCurrAPI(clunum) <= 50)                             /*瞬时电流小(确保实际空闲)*/
            	&& (eCURR_IDLE == GetGChgDhgStateAPI(clunum)) && (sGProTimer[clunum][eProSort_SysErr] >= 10))         /*空闲状态只等待1s*/
                || (sGProTimer[clunum][eProSort_SysErr] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ErrProOffT] * 10)))/*达到保护时间*/
            {
                GroupProCtrlBySwitch(clunum);                                                                 /*断开相应运行接触器保护*/
                BitSet(sGroupHapProFlag[clunum], eProSort_SysErr);                                              /*设置执行保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_SysErr]++;
            }
            break;

        /*充放电保护*/
        case eProSort_SysPro:
            BitSet(sGroupAskProFlag[clunum], eProSort_SysPro);                                                  /*设置请求保护*/
            if(((GetGChgLimCurrAPI(clunum) >= -50) && (GetGChgLimCurrAPI(clunum) <= 50)                             /*瞬时电流小(确保实际空闲)*/
                && (eCURR_IDLE == GetGChgDhgStateAPI(clunum)) && (sGProTimer[clunum][eProSort_SysPro] >= 10))         /*空闲状态只等待1s*/
                || (sGProTimer[clunum][eProSort_SysPro] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ErrProOffT] * 10)))/*达到保护时间*/
            {
                GroupProCtrlBySwitch(clunum);                                                                 /*断开相应运行接触器保护*/
                BitSet(sGroupHapProFlag[clunum], eProSort_SysPro);                                              /*设置执行保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_SysPro]++;
            }
            break;

        /*放电保护*/
        case eProSort_DhgPro:
            BitSet(sGroupAskProFlag[clunum], eProSort_DhgPro);                                                  /*设置请求保护*/
            if(((GetGChgLimCurrAPI(clunum) >= -50) && (GetGChgLimCurrAPI(clunum) <= 50)                             /*瞬时电流小(确保实际空闲)*/
                && (eCURR_IDLE == GetGChgDhgStateAPI(clunum)) && (sGProTimer[clunum][eProSort_DhgPro] >= 10))         /*空闲状态只等待1s*/
                || (sGProTimer[clunum][eProSort_DhgPro] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_DhgProOffT] * 10)))/*达到保护时间*/
            {
                if(2 == sGSamProSort[clunum])                                                                   /*已发生过放电保护*/
                {
                    if(0 == BitGet(sGroupHapProFlag[clunum], eProSort_DhgPro))                                  /*首次控制保护计算保护1次*/
                    {
                        sGSamProTimes[clunum]++;
                    }
                }
                else if((0 == sGSamProSort[clunum]) || (sGSamProTimes[clunum] < ERR_PRO_RE_TIMES))                      /*首次产生放电保护(其他保护未达到最大保护次数)*/
                {
                    sGSamProSort[clunum] = 2;
                    sGSamProTimes[clunum] = 1;
                    sGSamProTimer[clunum] = GetSysSecondTimeAPI();
                }

                GroupProCtrlBySwitch(clunum);                                                                 /*断开相应运行接触器保护*/
                BitSet(sGroupHapProFlag[clunum], eProSort_DhgPro);                                              /*设置执行保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_DhgPro]++;
            }
            break;

        /*充电保护*/
        case eProSort_ChgPro:
            BitSet(sGroupAskProFlag[clunum], eProSort_ChgPro);                                                  /*设置请求保护*/
            if(((GetGChgLimCurrAPI(clunum) >= -50) && (GetGChgLimCurrAPI(clunum) <= 50)                             /*瞬时电流小(确保实际空闲)*/
                && (eCURR_IDLE == GetGChgDhgStateAPI(clunum)) && (sGProTimer[clunum][eProSort_ChgPro] >= 10))         /*空闲状态只等待1s*/
                || (sGProTimer[clunum][eProSort_ChgPro] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ChgProOffT] * 10)))/*达到保护时间*/
            {
                if(1 == sGSamProSort[clunum])                                                                   /*已发生过充电保护*/
                {
                    if(0 == BitGet(sGroupHapProFlag[clunum], eProSort_ChgPro))                                  /*首次控制保护计算保护1次*/
                    {
                        sGSamProTimes[clunum]++;
                    }
                }
                else if((0 == sGSamProSort[clunum]) || (sGSamProTimes[clunum] < ERR_PRO_RE_TIMES))                      /*首次产生充电保护(其他保护未达到最大保护次数)*/
                {
                    sGSamProSort[clunum] = 1;
                    sGSamProTimes[clunum] = 1;
                    sGSamProTimer[clunum] = GetSysSecondTimeAPI();
                }

                GroupProCtrlBySwitch(clunum);                                                                 /*断开相应运行接触器保护*/
                BitSet(sGroupHapProFlag[clunum], eProSort_ChgPro);                                              /*设置执行保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_ChgPro]++;
            }
            break;

        /*放电限制保护(只请求停机不断接触器)*/
        case eProSort_DhgLim:
            if(sGProTimer[clunum][eProSort_DhgLim] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_DhgLimOffT] * 10)) /*达到保护时间*/
            {
                /*GroupProCtrlBySwitch();*/                                                             /*断开相应运行接触器保护*/
                BitSet(sGroupAskProFlag[clunum], eProSort_DhgLim);                                              /*设置请求保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_DhgLim]++;
            }
            break;

        /*充电截止保护(只请求停机不断接触器)*/
        case eProSort_ChgLim:
            if(sGProTimer[clunum][eProSort_ChgLim] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ChgLimOffT] * 10)) /*达到保护时间*/
            {
                /*GroupProCtrlBySwitch();*/                                                             /*断开相应运行接触器保护*/
                BitSet(sGroupAskProFlag[clunum], eProSort_ChgLim);                                              /*设置请求保护*/
            }
            else
            {
                sGProTimer[clunum][eProSort_ChgLim]++;
            }
            break;

        default:
            break;
    }
}
/*=================================================================================================
** @Name      : static void GroupProCtrlClr(u8 clunum, e_ErrProSort proSort)
** @Input     : clunum:簇编号 proSort:故障保护类型[0,5]
** @Output    : void
** @Function  : 电池组故障保护清零复位
** @The notes : 计数器和保护标志清零
**===============================================================================================*/
static void GroupProCtrlClr(u8 clunum, e_ErrProSort proSort)
{
    if(sGProTimer == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sGProTimer null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    /*类型有效*/
    if(proSort < eProSort_Num)
    {
        /*已发生过保护*/
        if(sGSamProSort[clunum] > 0)
        {
            //距离上次保护时间超过10min未达到2次保护*/
            if((ABS(GetSysSecondTimeAPI(), sGSamProTimer[clunum]) > 600)
                && (sGSamProTimes[clunum] < ERR_PRO_RE_TIMES))
            {
                /*参数复位*/
                sGSamProSort[clunum] = 0;
                sGSamProTimes[clunum] = 0;
                sGSamProTimer[clunum] = 0;
            }
            /*达到2次保护*/
            else if(sGSamProTimes[clunum] >= ERR_PRO_RE_TIMES)
            {
                /*故障保护间隔1小时*/
                if(ABS(GetSysSecondTimeAPI(), sGSamProTimer[clunum]) > 3600)
                {
                    /*允许再尝试一次*/
                    /*sGSamProSort = 0;*/
                    sGSamProTimes[clunum] = ERR_PRO_RE_TIMES - 1;
                    sGSamProTimer[clunum] =  GetSysSecondTimeAPI();
                }
                /*不清除本类故障保护*/
                else if(((2 == sGSamProSort[clunum]) && ((eProSort_DhgPro == proSort) || (eProSort_SysPro == proSort)))
                    || ((1 == sGSamProSort[clunum]) && ((eProSort_ChgPro == proSort) || (eProSort_SysPro == proSort))))
                {
                    /*充电保护或放电保护均上升为充放电保护(防止再启动)*/
                    BitSet(sGroupAskProFlag[clunum], eProSort_SysPro);
                    BitSet(sGroupHapProFlag[clunum], eProSort_SysPro);
                    return;
                }
            }
        }

        sGProTimer[clunum][proSort] = 0;
        BitClr(sGroupAskProFlag[clunum], proSort);
        BitClr(sGroupHapProFlag[clunum], proSort);
    }
}
/*=================================================================================================
** @Name      : static void GroupProCtrlBySwitch(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 电池组控制接触器执行故障保护
** @The notes : 断接触器
**===============================================================================================*/
static void GroupProCtrlBySwitch(u8 clunum)
{
    static u16 sTime[2] = {0};

    if(clunum >= 2)
    {
        return;
    }

    if(clunum == 0)
    {
        /*控制预充接触器断开,新项目没有预充接触器，暂时屏蔽*/
        //TacCtrlGroupSwhStateAPI(eGroupSwh_Pre, ePSTATE_OFF);
        /*根据电流方向确定先断哪个接触器*/
        if(eCURR_CHG == GetGChgDhgStateAPI(clunum))                               /*正在充电*/
        {
            if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg))     /*负极接触器未断开*/
            {
                sTime[clunum] = 0;                                                  /*断接触器计时清零*/
                TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);                /*先断开负极接触器*/
            }
            else                                                                    /*负极接触器已断开*/
            {
                if((1 == TacCheckCurrCutStateHook(clunum))                        /*负极接触器断开无电流*/
                    || (sTime[clunum] >= ERR_PRO_SWH_CURR_T))                       /*延迟500ms仍未切断有电流*/
                {
                    if(0 == TacCheckCurrCutStateHook(clunum))                     /*负极接触器断开超时仍电流*/
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK2_1Neg);           /*设置负极接触器断开失败*/
                    }

                    sTime[clunum] = ERR_PRO_SWH_CURR_T;

                    TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);    /*确保断开负极接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);    /*再断开正极主接触器*/
                }
                else                                                        /*仍有电流*/
                {
                    sTime[clunum]++;                                                /*计时*/
                }
            }
        }
        else                                                                /*无电流或放电停机*/
        {
            if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai))     /*正极主接触器未断开*/
            {
                sTime[clunum] = 0;                                                  /*断接触器计时清零*/
                TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);        /*先断开正极主接触器*/
            }
            else                                                            /*正极主接触器已断开*/
            {
                if((1 == TacCheckCurrCutStateHook(clunum))                        /*正极主接触器断开无电流*/
                    || (sTime[clunum] >= ERR_PRO_SWH_CURR_T))                       /*延迟500ms仍未切断有电流*/
                {
                    if(0 == TacCheckCurrCutStateHook(clunum))                     /*正极主接触器断开超时仍有电流*/
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);           /*设置主接触器断开失败*/
                    }

                    sTime[clunum] = ERR_PRO_SWH_CURR_T;

                    TacCtrlGroupSwhStateAPI(eGroupSwhPK1_1Mai, ePSTATE_OFF);    /*确保断开正极主接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK2_1Neg, ePSTATE_OFF);    /*再断开负极接触器*/
                }
                else                                                        /*仍有电流*/
                {
                    sTime[clunum]++;                                                /*计时*/
                }
            }
        }

        if(sTime[clunum] >= ERR_PRO_SWH_CURR_T)                                     /*已控制接触器先断*/
        {
            TacCtrlGroupRunSwhAllOffAPI(clunum);                                  /*断开所有运行接触器*/
        }

        if(1 == TacGetGroupRunSwhAllOffStateAPI(clunum))                          /*运行接触器全断开*/
        {
            if(0 == TacCheckCurrCutStateHook(clunum))                             /*接触器断开仍有电流*/
            {
                TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK1_1Mai);                   /*设置主接触器断开失败*/
            }
        }
    }
    else
    {
        /*控制预充接触器断开,新项目没有预充接触器，暂时屏蔽*/
        //TacCtrlGroupSwhStateAPI(eGroupSwh_Pre, ePSTATE_OFF);
        /*根据电流方向确定先断哪个接触器*/
        if(eCURR_CHG == GetGChgDhgStateAPI(clunum))                               /*正在充电*/
        {
            if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg))     /*负极接触器未断开*/
            {
                sTime[clunum] = 0;                                                  /*断接触器计时清零*/
                TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);                /*先断开负极接触器*/
            }
            else                                                            /*负极接触器已断开*/
            {
                if((1 == TacCheckCurrCutStateHook(clunum))                        /*负极接触器断开无电流*/
                    || (sTime[clunum] >= ERR_PRO_SWH_CURR_T))                       /*延迟500ms仍未切断有电流*/
                {
                    if(0 == TacCheckCurrCutStateHook(clunum))                     /*负极接触器断开超时仍电流*/
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK4_2Neg);           /*设置负极接触器断开失败*/
                    }

                    sTime[clunum] = ERR_PRO_SWH_CURR_T;

                    TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);    /*确保断开负极接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);    /*再断开正极主接触器*/
                }
                else                                                        /*仍有电流*/
                {
                    sTime[clunum]++;                                                /*计时*/
                }
            }
        }
        else                                                                /*无电流或放电停机*/
        {
            if(ePSTATE_ON == TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai))     /*正极主接触器未断开*/
            {
                sTime[clunum] = 0;                                                  /*断接触器计时清零*/
                TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);        /*先断开正极主接触器*/
            }
            else                                                            /*正极主接触器已断开*/
            {
                if((1 == TacCheckCurrCutStateHook(clunum))                        /*正极主接触器断开无电流*/
                    || (sTime[clunum] >= ERR_PRO_SWH_CURR_T))                       /*延迟500ms仍未切断有电流*/
                {
                    if(0 == TacCheckCurrCutStateHook(clunum))                     /*正极主接触器断开超时仍有电流*/
                    {
                        TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);           /*设置主接触器断开失败*/
                    }

                    sTime[clunum] = ERR_PRO_SWH_CURR_T;

                    TacCtrlGroupSwhStateAPI(eGroupSwhPK3_2Mai, ePSTATE_OFF);    /*确保断开正极主接触器*/
                    TacCtrlGroupSwhStateAPI(eGroupSwhPK4_2Neg, ePSTATE_OFF);    /*再断开负极接触器*/
                }
                else                                                        /*仍有电流*/
                {
                    sTime[clunum]++;                                                /*计时*/
                }
            }
        }

        if(sTime[clunum] >= ERR_PRO_SWH_CURR_T)                                     /*已控制接触器先断*/
        {
            TacCtrlGroupRunSwhAllOffAPI(clunum);                                  /*断开所有运行接触器*/
        }

        if(1 == TacGetGroupRunSwhAllOffStateAPI(clunum))                          /*运行接触器全断开*/
        {
            if(0 == TacCheckCurrCutStateHook(clunum))                             /*接触器断开仍有电流*/
            {
                TacSetGSwhOffCtrlFailFlag(clunum, eGroupSwhPK3_2Mai);                   /*设置主接触器断开失败*/
            }
        }
    }
}

