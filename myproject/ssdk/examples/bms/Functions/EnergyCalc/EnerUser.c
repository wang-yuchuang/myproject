/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EnerUser.c
** @Founder      : ZQD808
** @Date         : 2026.03.17
** @Function     : 电能量计算模块用户函数
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "CapInfoCalc.h"
#include "EnerInfoCalc.h"
#include "CellFadeCalc.h"
#include "SocDisplay.h"
#include "SoeDisplay.h"
#include "SohDisplay.h"
#include "EnerUser.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
extern u16 gGBmuGenPara_102[];  			/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	        /*主控高级参数值*/
extern u16 *gSysCapInfo_52;                 /*系统电量信息*/
extern u16 *gCurrCapEner_55;                /*当前能量统计信息*/
/*=============================================================================================
** @Name      : void EnerCapCalcAllInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电能量计算模块总初始化
** @The notes :
**===========================================================================================*/
void EnerCapCalcAllInit(void)
{
	/*SOC/SOE/SOH基础计算初始化*/
	GroupCapSocInit();
	GroupEnerSoeInit();
	GroupFadeSohInit();

	/*SOC/SOE/SOH显示初始化*/
	GroupSocDisplayInit();
	GroupSoeDisplayInit();
	GroupSohDisplayInit();
}

/*=============================================================================================
** @Name      : void GroupEnerDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电能量计算值显示任务
** @The notes : 100ms
**===========================================================================================*/
void GroupEnerDisplayTask(void)
{
    if(gSysCapInfo_52 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCapInfo_52 null!!!\r\n", __func__);
        return;
    }
    if(gCurrCapEner_55 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gCurrCapEner_55 null!!!\r\n", __func__);
        return;
    }
	/*SOC/SOE/SOH显示处理*/
	GroupSocDisplayTask();
	GroupSoeDisplayTask();
	GroupSohDisplayTask();

	/*电能电量显示处理*/
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        if(clunum == 0)
        {
            gSysCapInfo_52[eSysCapInfo52_1AllCap] = (u16)(GetGroupTotalCapAPI(clunum) / 100);
            gCurrCapEner_55[eCurrCapEner55_1NowEner] =  (u16)(GetGroupNowEnerAPI(clunum) / 100);
            gCurrCapEner_55[eCurrCapEner55_1AllEner] = (u16)(GetGroupTotalEnerAPI(clunum) / 100);
        }
        else
        {
            gSysCapInfo_52[eSysCapInfo52_2AllCap] = (u16)(GetGroupTotalCapAPI(clunum) / 100);
            gCurrCapEner_55[eCurrCapEner55_2NowEner] =  (u16)(GetGroupNowEnerAPI(clunum) / 100);
            gCurrCapEner_55[eCurrCapEner55_2AllEner] = (u16)(GetGroupTotalEnerAPI(clunum) / 100);
        }
       

    }
}