/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : StaInterface.c
** @Founder      : ZQD808
** @Date         : 2026.03.18
** @Function     : 统计模块用户接口
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "DiagFilter.h"
#include "ParaInterface.h"
#include "DiagInterface.h"
//#include "TacInterface.h"
#include "StaInterface.h"
#include "GroupState.h"

/*=============================================================================================
** @Name      : u8 StaGetBattVStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组电压状态:0正常 1充满 2放空
** @Function  : 获取电池组电压状态
** @The notes :
**===========================================================================================*/
u8 StaGetBattVStateHook(u8 clunum)
{
    return(TacGetGroupBattStateAPI(clunum));
}
/*=============================================================================================
** @Name      : u8 StaGetVHLimStateHook(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 高压截止状态:0正常 1高压充电截止
** @Function  : 获取高压截止状态
** @The notes : 单体电压高2级(充电截止)
**===========================================================================================*/
u8 StaGetVHLimStateHook(u8 clunum)
{
    u8 state = 0;

    if(clunum >= 2)
    {
        return (0);
    }

    if(eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_CEL_VH2))
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    return(state);
}
/*=============================================================================================
** @Name      : u8 StaGetVLLimStateHook(void)
** @Input     : clunum:簇编号
** @Output    : 低压截止状态:0正常 1低压放电截止
** @Function  : 获取低压截止状态
** @The notes : 低压2级,限制放电
**===========================================================================================*/
u8 StaGetVLLimStateHook(u8 clunum)
{
    u8 state = 0;

    if(clunum >= 2)
    {
        return (0);
    }

    if(eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_CEL_VL2))
    {
        state = 1;
    }
    else
    {
        state = 0;
    }

    return(state);
}
