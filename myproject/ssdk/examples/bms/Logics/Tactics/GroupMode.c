/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupMode.c
** @Founder      : ZQD808
** @Date         : 2026.03.19
** @Function     : 电池工作模式信息模块接口
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "ParaInterface.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "PublicInfo.h"
#include "TacInterface.h"
#include "GroupProtect.h"
#include "TacUser.h"
#include "GroupMode.h"
#include "ParaRead.h"
#include "ParaWrite.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
static u16 sHisNorMode[2] = {0};                                 /*原始正常模式 0自动模式 1手动模式*/
static u32 sRcvUnorModeTime[2] = {0};                            /*接收进入非正常模式时间 1ms*/


extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];                                 /*主控硬件参数配置信息*/
extern u16 *gSysStaInfo_50;                                 /*系统状态信息*/
extern u16 *gSysCtrlCmd_65;                                 /*组1系统控制命令*/
extern u16 *gSysCtrlCmd_66;                                 /*组2系统控制命令*/
/*=================================================================================================
** @Name      : void GroupModeInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组模式信息初始化
** @The notes : 读EEPROM写入值
**===============================================================================================*/
void GroupModeInfoInit(void)
{
	u16 data[2] = {0};

    if (gSysStaInfo_50 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysStaInfo_50 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        if(clunum >= 2)
        {
            return;
        }
        /*读存储值设置自动模式还是手动模式*/
        if(TRUE == ParaReadStoreSystemWorkMode(clunum, data, 2))
        {
            if(clunum == 0)
            {
                gSysStaInfo_50[eSysStateInfo50_1SysMod] = data[1];
            }
            else
            {
                gSysStaInfo_50[eSysStateInfo50_2SysMod] = data[1];
            } 
        }
        /*读设置值失败*/
        else
        {
            if(clunum == 0)
            {
                /*重写默认值*/
                data[0] = PARA_CFG_FLAG_U16;
                data[1] = gSysStaInfo_50[eSysStateInfo50_1SysMod];
                ParaWriteStoreSystemWorkMode(clunum, 1, &data[1], 1);
                ParaWriteStoreSystemWorkMode(clunum, 0, &data[0], 1);
                sHisNorMode[clunum] = gSysStaInfo_50[eSysStateInfo50_1SysMod];
            }
            else
            {
                /*重写默认值*/
                data[0] = PARA_CFG_FLAG_U16;
                data[1] = gSysStaInfo_50[eSysStateInfo50_2SysMod];
                ParaWriteStoreSystemWorkMode(clunum, 1, &data[1], 1);
                ParaWriteStoreSystemWorkMode(clunum, 0, &data[0], 1);
                sHisNorMode[clunum] = gSysStaInfo_50[eSysStateInfo50_2SysMod];
            }
        }
    }
}
/*=================================================================================================
** @Name      : void GroupWorkModeTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组工作模式检测任务
** @The notes : 1s周期调用
**===============================================================================================*/
void GroupWorkModeTask(void)
{
    static u8 sTimer[2] = {0};
    if (gSysCtrlCmd_65 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCtrlCmd_65 null!!!\r\n", __func__);
        return;
    }
    if (gSysCtrlCmd_66 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCtrlCmd_66 null!!!\r\n", __func__);
        return;
    }
    if (gSysStaInfo_50 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysStaInfo_50 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++) //按两簇初始化
    {
        if(clunum >= 2)
        {
            return;
        }
        /*处于调试模式*/
        if(eMODE_DEBUG == GetGWorkModeAPI(clunum))
        {
            /*特殊模式保持10min后自动退出*/
            if(ABS(BSPGetOSMillSecTimeAPI(), sRcvUnorModeTime[clunum]) >= MODE_KEEP_UNORMODE_T)
            {
                //切换到原始正常模式命令
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                }
               
                RcvGroupWorkModeCmdAPI(clunum, eCMDPORT_INTE); 
            }
        }
        /*处于紧急模式*/
        if(eMODE_EMERG == GetGWorkModeAPI(clunum))
        {
            /*紧急模式保持30min后自动退出*/
            if(ABS(BSPGetOSMillSecTimeAPI(), sRcvUnorModeTime[clunum]) >= MODE_KEEP_UNORMODE_T * 3)
            {
                /*切换到原始正常模式命令*/
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                }
                RcvGroupWorkModeCmdAPI(clunum, eCMDPORT_INTE); 
            }
            /*紧急放电模式下进行充电直接退出紧急模式并进入故障状态*/
            if((GetGCellMaxVoltAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_VH1Lim]) && (eCURR_CHG == GetGChgDhgStateAPI(clunum)))
            {
                /*切换到原始正常模式命令*/
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                    gSysStaInfo_50[eSysStateInfo50_1SysWor] = eWORK_ERR;
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                    gSysStaInfo_50[eSysStateInfo50_2SysWor] = eWORK_ERR;
                }
                RcvGroupWorkModeCmdAPI(clunum, eCMDPORT_INTE);
            }
            /*最小电压>3100且最大电压<3500自动退出紧急模式*/
            if((GetGCellMinVoltAPI(clunum) >= 3100) && (GetGCellMaxVoltAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_VH2Lim] - 100))
            {
                /*切换到原始正常模式命令*/
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_WorkMode] = sHisNorMode[clunum];
                }
                RcvGroupWorkModeCmdAPI(clunum, eCMDPORT_INTE);
            }
        }
        /*自动模式或紧急模式*/
        if((eMODE_AUTO == GetGWorkModeAPI(clunum))
            || (eMODE_EMERG == GetGWorkModeAPI(clunum)))
        {
            u8 fitStart;
            fitStart = TacGetGroupFitStartStateHook(clunum);
            //系统处于空闲且适合启动
            if((eWORK_IDLE == GetGWorkStateAPI(clunum))
                && (1 == fitStart))
            {
                //达到启动等待延时或上电首次启动
                if((sTimer[clunum] >= (gGBmuHigLevPara_103[eBmuHigLevPara103_SelfWaitT] / 40))
                    || (BSPGetOSMillSecTimeAPI() <= 10000)
                    )
                {
                    //设置正常启动命令
                    if(clunum == 0)
                    {
                        gSysCtrlCmd_65[eSysCtrlCmd_WorkSta] = eCTRL_NORSTART;
                    }
                    else
                    {
                        gSysCtrlCmd_66[eSysCtrlCmd_WorkSta] = eCTRL_NORSTART;
                    }
                    RcvGroupWorkStateCmdAPI(clunum, eCMDPORT_INTE);
                }
                //未达到启动延时
                else
                {
                    sTimer[clunum]++;
                }
            }
            //系统正在启动未闭合接触器时不适合启动则停机否则由故障保护启动失败
            else if((eWORK_START == GetGWorkStateAPI(clunum))
                && (0 == fitStart)
                && (1 == TacGetGroupRunSwhAllOffStateAPI(clunum)))
            {
                //立即停止系统
                if(clunum == 0)
                {
                    gSysCtrlCmd_65[eSysCtrlCmd_WorkSta] = eCTRL_NORSTOP;
                }
                else
                {
                    gSysCtrlCmd_66[eSysCtrlCmd_WorkSta] = eCTRL_NORSTOP;
                }
                RcvGroupWorkStateCmdAPI(clunum, eCMDPORT_INTE);
                sTimer[clunum] = 0;
            }
            //系统非空闲或充放电保护
            else
            {
                //启动延时清零
                sTimer[clunum] = 0;
            }
        }
        else
        {
            //启动延时清零
            sTimer[clunum] = 0;
        }
    }
}
/*=================================================================================================
** @Name      : void TacSetGroupWorkModeAPI(e_WorkMode mode)
** @Input     : mode:电池组工作模式[0,5]
** @Output    : void
** @Function  : 设置电池组工作模式
** @The notes : 非正常模式只能退回正常模式,非正常模式之间不能跳转
**===============================================================================================*/
void TacSetGroupWorkModeAPI(u8 clunum, e_WorkMode mode)
{
	u16 data[2] = {0};

    if(clunum >= 2)
    {
        return;
    }
	if(mode >= eMODE_STATE)
    {
        return;
    }
    if (gSysStaInfo_50 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysStaInfo_50 null!!!\r\n", __func__);
        return;
    }
    if(clunum == 0)
    {
        /*正处于调试模式或紧急模式*/
        if((eMODE_DEBUG == gSysStaInfo_50[eSysStateInfo50_1SysMod])
            || (eMODE_EMERG == gSysStaInfo_50[eSysStateInfo50_1SysMod]))
        {
            /*退出调试模式或紧急模式到原来模式*/
            if(eMODE_AUTO == mode)
            {
                /*模式切换用户处理*/
                TacGroupWorkModeChangDealHook(clunum, (u8)gSysStaInfo_50[eSysStateInfo50_1SysMod], (u8)sHisNorMode[clunum]);

                /*只能由特殊模式切换至原来正常模式*/
                gSysStaInfo_50[eSysStateInfo50_1SysMod] = sHisNorMode[clunum];
            }
            /*退出调试模式或紧急模式到手动模式*/
            else if(eMODE_HAND == mode)
            {
                /*原来非手动模式*/
                if(eMODE_HAND != sHisNorMode[clunum])
                {
                    /*记录新模式*/
                    data[1] = mode;
                    data[0] = PARA_CFG_FLAG_U16;
                    ParaWriteStoreSystemWorkMode(clunum, 1, &data[1], 1);
                    ParaWriteStoreSystemWorkMode(clunum, 0, &data[0], 1);
                }

                /*模式切换用户处理*/
                TacGroupWorkModeChangDealHook(clunum, (u8)gSysStaInfo_50[eSysStateInfo50_1SysMod], (u8)mode);

                /*只能由特殊模式切换至正常模式*/
                gSysStaInfo_50[eSysStateInfo50_1SysMod] = mode;
            }
        }
        /*正处于手动模式或自动模式*/
        else
        {
            /*模式切换*/
            if( mode != gSysStaInfo_50[eSysStateInfo50_1SysMod])
            {
                /*手动/自动切换*/
                if((eMODE_HAND == mode) || (eMODE_AUTO == mode))
                {
                    /*记录新模式*/
                    data[1] = mode;
                    data[0] = PARA_CFG_FLAG_U16;
                    ParaWriteStoreSystemWorkMode(clunum, 1, &data[1], 1);
                    ParaWriteStoreSystemWorkMode(clunum, 0, &data[0], 1);
                }

                /*记录原始模式和模式切换时间*/
                sHisNorMode[clunum] = gSysStaInfo_50[eSysStateInfo50_1SysMod];

                /*模式切换用户处理*/
                TacGroupWorkModeChangDealHook(clunum, (u8)gSysStaInfo_50[eSysStateInfo50_1SysMod], (u8)mode);

                /*切换当前模式接收模式切换时间*/
                gSysStaInfo_50[eSysStateInfo50_1SysMod] = mode;
            }
        }

        /*记录接收到切换调试模式的时间*/
        sRcvUnorModeTime[clunum] = BSPGetOSMillSecTimeAPI();
    }
    else
    {
        /*正处于调试模式或紧急模式*/
        if((eMODE_DEBUG == gSysStaInfo_50[eSysStateInfo50_2SysMod])
            || (eMODE_EMERG == gSysStaInfo_50[eSysStateInfo50_2SysMod]))
        {
            /*退出调试模式或紧急模式到原来模式*/
            if(eMODE_AUTO == mode)
            {
                /*模式切换用户处理*/
                TacGroupWorkModeChangDealHook(clunum, (u8)gSysStaInfo_50[eSysStateInfo50_2SysMod], (u8)sHisNorMode[clunum]);

                /*只能由特殊模式切换至原来正常模式*/
                gSysStaInfo_50[eSysStateInfo50_2SysMod] = sHisNorMode[clunum];
            }
            /*退出调试模式或紧急模式到手动模式*/
            else if(eMODE_HAND == mode)
            {
                /*原来非手动模式*/
                if(eMODE_HAND != sHisNorMode[clunum])
                {
                    /*记录新模式*/
                    data[1] = mode;
                    data[0] = PARA_CFG_FLAG_U16;
                    ParaWriteStoreSystemWorkMode(clunum, 1, &data[1], 1);
                    ParaWriteStoreSystemWorkMode(clunum, 0, &data[0], 1);
                }

                /*模式切换用户处理*/
                TacGroupWorkModeChangDealHook(clunum, (u8)gSysStaInfo_50[eSysStateInfo50_2SysMod], (u8)mode);

                /*只能由特殊模式切换至正常模式*/
                gSysStaInfo_50[eSysStateInfo50_2SysMod] = mode;
            }
        }
        /*正处于手动模式或自动模式*/
        else
        {
            /*模式切换*/
            if( mode != gSysStaInfo_50[eSysStateInfo50_2SysMod])
            {
                /*手动/自动切换*/
                if((eMODE_HAND == mode) || (eMODE_AUTO == mode))
                {
                    /*记录新模式*/
                    data[1] = mode;
                    data[0] = PARA_CFG_FLAG_U16;
                    ParaWriteStoreSystemWorkMode(clunum, 1, &data[1], 1);
                    ParaWriteStoreSystemWorkMode(clunum, 0, &data[0], 1);
                }

                /*记录原始模式和模式切换时间*/
                sHisNorMode[clunum] = gSysStaInfo_50[eSysStateInfo50_2SysMod];

                /*模式切换用户处理*/
                TacGroupWorkModeChangDealHook(clunum, (u8)gSysStaInfo_50[eSysStateInfo50_2SysMod], (u8)mode);

                /*切换当前模式接收模式切换时间*/
                gSysStaInfo_50[eSysStateInfo50_2SysMod] = mode;
            }
        }

        /*记录接收到切换调试模式的时间*/
        sRcvUnorModeTime[clunum] = BSPGetOSMillSecTimeAPI();
    }
}
