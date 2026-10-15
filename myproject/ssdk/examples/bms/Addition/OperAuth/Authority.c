/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : Authority.c
** @Founder      : ZQD808
** @Date         : 2026.03.24
** @Function     : 权限管理模块接口函数
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicPara.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
//#include "EventUser.h"
#include "Authority.h"

#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>

extern u16 *gGAuthorInfo_63;      /*权限级别状态信息*/
extern u16 *gGAuthorCtrl_64;      /*权限密码登录信息数组*/
extern u16 gSysGenParaRO_100[];  							/*系统通用设置*/
extern u16 gSysHigParaRO_101[];  							/*系统硬件设置*/
extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];  								/*主控硬件参数值*/
extern u16 gSysGenParaRW_105[];                             /*系统通用设置*/
/*=================================================================================================
** @Name      : void AuthorityInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组管理权限初始化
** @The notes : 默认权限处理
**===============================================================================================*/
void AuthorityInit(void)
{
    if (gGAuthorInfo_63 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGAuthorInfo_63 null!!!\r\n", __func__);
        return;
    }
	/*不开放端口权限*/
	if(0 == gSysHigParaRO_101[eSHWPara101_PermPort])
	{
		/*普通用户权限*/
		gGAuthorInfo_63[0] = eAuthL_User;
		gGAuthorInfo_63[1] = eAuthP_Can0;
	}
	/*开放所有端口权限*/
	else if(1 == gSysHigParaRO_101[eSHWPara101_PermPort])
	{
		/*默认开放一级运维权限*/
		gGAuthorInfo_63[0] = eAuthL_Operater;
		gGAuthorInfo_63[1] = eAuthP_All;
	}
	/*开放CAN端口权限*/
	else if(gSysHigParaRO_101[eSHWPara101_PermPort] < 6)
	{
		/*默认开放一级运维权限*/
		gGAuthorInfo_63[0] = eAuthL_Operater;
		gGAuthorInfo_63[1] = eAuthP_Can0 + (gSysHigParaRO_101[eSHWPara101_PermPort] - 2);
	}
	/*开放串口端口权限*/
	else if(gSysHigParaRO_101[eSHWPara101_PermPort] < 10)
	{
		/*默认开放一级运维权限*/
		gGAuthorInfo_63[0] = eAuthL_Operater;
		gGAuthorInfo_63[1] = eAuthP_Sci0 + (gSysHigParaRO_101[eSHWPara101_PermPort] - 6);
	}
}

/*=================================================================================================
** @Name      : void RcvSysAuthorityCmdAPI(u8 port)
** @Input     : port:接收命令端口:[0,4]CAN口 [5,9]RS485口
** @Output    : void
** @Function  : 系统接收切换管理权限命令处理函数
** @The notes : 通信口命令,按钮和内部口默认最高权限不需切换
**===============================================================================================*/
void RcvSysAuthorityCmdAPI(u8 port)
{
    if (gGAuthorInfo_63 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGAuthorInfo_63 null!!!\r\n", __func__);
        return;
    }
    if (gGAuthorCtrl_64 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGAuthorCtrl_64 null!!!\r\n", __func__);
        return;
    }

	/*清除权限密码正确*/
	if((AUTHORITY_NAMECLR == gGAuthorCtrl_64[0]) && (AUTHORITY_PWORDCLR == gGAuthorCtrl_64[1]))
	{
	    /*RS485#2口可清除任意端口权限到默认权限,通信所在口可清除任意端口权限到默认权限*/
		if((eAuthP_Sci1 == port) || (gGAuthorCtrl_64[1] == port))
		{
		    /*复位默认权限*/
		    AuthorityInit();
		}

		/*通信所在口为默认权限口则切换为最低权限*/
		if(gGAuthorInfo_63[1] == port)
		{
			/*退回0级普通权限*/
			gGAuthorInfo_63[0] = eAuthL_User;
		}
	}
	/*一级运维权限密码正确*/
	else if((AUTHORITY_NAME1 == gGAuthorCtrl_64[0]) && (AUTHORITY_PWORD1 == gGAuthorCtrl_64[1]))
	{
		/*开放一级运维权限*/
		gGAuthorInfo_63[0] = eAuthL_Operater;
		gGAuthorInfo_63[1] = port;
	}
	/*二级高级用户权限密码正确*/
	else if((AUTHORITY_NAME2 == gGAuthorCtrl_64[0]) && (AUTHORITY_PWORD2 == gGAuthorCtrl_64[1]))
	{
		/*开放二级高级用户权限*/
		gGAuthorInfo_63[0] = eAuthL_Service;
		gGAuthorInfo_63[1] = port;
	}
	/*三级超级用户权限密码正确*/
	else if(((AUTHORITY_NAME3 == gGAuthorCtrl_64[0]) && (AUTHORITY_PWORD3 == gGAuthorCtrl_64[1]))
		|| ((AUTHORITY_NAME4 == gGAuthorCtrl_64[0]) && (AUTHORITY_PWORD4 == gGAuthorCtrl_64[1])))
	{
		/*开放三级超级用户权限*/
		gGAuthorInfo_63[0] = eAuthL_Admin;
		gGAuthorInfo_63[1] = port;
	}

	/*产生事件,后续再实现*/
	//EventRcdDisplayUserAPI(eEVN_RCDSORT_CMD, 0, GetGSampOutCurrAPI(), gGAuthorInfo_80[0], gGAuthorInfo_80[1], port);
}
/*=================================================================================================
** @Name      : u8 GetSysNowAuthorityAPI(e_AuthOpenPort port)
** @Input     : port:接收命令端口:[0,4]CAN口 [5,9]RS485口 [10,12]按钮或内部口 0xff:所有口
** @Output    : 系统当前权限级别[0,3]
** @Function  : 获取系统相应端口当前权限级别
** @The notes :
**===============================================================================================*/
u8 GetSysNowAuthorityAPI(e_AuthOpenPort port)
{
    if (gGAuthorInfo_63 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGAuthorInfo_63 null!!!\r\n", __func__);
        return (eAuthL_User);
    }
	/*为权限端口*/
	if((port == gGAuthorInfo_63[1])
		|| (eAuthP_All == gGAuthorInfo_63[1]))
	{
		/*当前权限*/
		return(gGAuthorInfo_63[0]);
	}
	/*按钮或内部*/
	else if((eAuthP_BUTT == port)
		|| (eAuthP_FSTC == port)
		|| (eAuthP_INTE == port))
	{
		/*最高权限*/
		return(eAuthL_Admin);
	}
	/*未获得权限端口*/
	else
	{
		/*普通权限*/
		return(eAuthL_User);
	}
}
/*=================================================================================================
** @Name      : u8 GetSysAuthorityInfoAPI(u8 index)
** @Input     : index:信息索引
** @Output    : 系统相权限级别信息
** @Function  : 获取系统相权限级别信息
** @The notes :
**===============================================================================================*/
u8 GetSysAuthorityInfoAPI(u8 index)
{
    if (gGAuthorInfo_63 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGAuthorInfo_63 null!!!\r\n", __func__);
        return (0);
    }

	/*信息索引正确*/
	if(index  < GINDEX63_LEN)
	{
		return(gGAuthorInfo_63[index]);
	}

	return(0);
}