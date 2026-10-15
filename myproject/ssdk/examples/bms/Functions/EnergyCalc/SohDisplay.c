/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SohDisplay.c
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : soh平滑显示模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "EnerInterface.h"
#include "SohDisplay.h"
#include "CellFadeCalc.h"
#include "GroupPara.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
extern u16 gGBmuGenPara_102[];		    /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	    /*主控高级参数值*/
extern u16 *gSysCapInfo_52;             /*系统电量信息*/

/*=============================================================================================
** @Name      : void GroupSohDisplayInit(void)
** @Input     : void
** @Output    : void
** @Function  : SOH平滑显示初始化
** @The notes : 用户初始化调用
**===========================================================================================*/
void GroupSohDisplayInit(void)
{
    if(gSysCapInfo_52 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCapInfo_52 null!!!\r\n", __func__);
        return;
    }
	gSysCapInfo_52[eSysCapInfo52_1SOH] = GetGRealSohMilliAPI(0);
    gSysCapInfo_52[eSysCapInfo52_2SOH] = GetGRealSohMilliAPI(1);
}

/*=============================================================================================
** @Name      : void GroupSohDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : SOH平滑显示任务
** @The notes : 100ms
**===========================================================================================*/
void GroupSohDisplayTask(void)
{
    u16 soh = 0;
    if(gSysCapInfo_52 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCapInfo_52 null!!!\r\n", __func__);
        return;
    }
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        soh = GetGRealSohMilliAPI(clunum);
        if(clunum == 0)
        {
            gSysCapInfo_52[eSysCapInfo52_1SOH] = soh;
        }
        else
        {
            gSysCapInfo_52[eSysCapInfo52_2SOH] = soh;
        }
    }
}

