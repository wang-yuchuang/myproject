/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagFilter.c
** @Founder      : ZQ808
** @Date         : 2026.03.05
** @Function     : 电池故障诊断模块故障滤波接口
** @Instructions : 
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "DiagInterface.h"
#include "DiagFilter.h"
#include "GroupPara.h"
#include "DiagCheck.h"
#include <stdio.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
#define DIAG_FILTER_SUB_SLOT_SIZE    72
#define DIAG_FILTER_NUM_CYCLES       ((eERR_ID_FINISH) / DIAG_FILTER_SUB_SLOT_SIZE)
//t_FilState *gErrFilTab = NULL;
t_FilState (*gErrFilTab)[eERR_ID_FINISH] = NULL;

extern const t_FilPara gErrFilPara[];                       /*故障诊断滤波参数(故障滤波时间和自恢复次数)*/
extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/
/*=================================================================================================
** @Name      : e_ErrState DiagGetErrLastStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号  id:故障ID
** @Output    : 当前稳定故障状态
** @Function  : 获取故障当前稳定状态
** @The notes : 不包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrLastStateAPI(u8 clunum, e_ERR_ID id);
static void DiagCheckErrResetClrTimes(void);
/*=============================================================================================
** @Name      :void DiagFilterInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void DiagFilterInfoInit(void)
{
	static u8 FirstInitFlag = 0;
	if(FirstInitFlag >= 1)
	{
		return;
	}
    gErrFilTab = ExtMalloc(CLUTER_MAX_NUM  * sizeof(*gErrFilTab));
    if(gErrFilTab == NULL)
    {
        printf("malloc gErrFilTab fail!!\r\n");
        return;
    }
    else
    {
        printf("malloc gErrFilTab %d success!!!\r\n",CLUTER_MAX_NUM  * sizeof(*gErrFilTab));
		memset(gErrFilTab, 0, CLUTER_MAX_NUM * sizeof(*gErrFilTab));  // 清零
    }
	FirstInitFlag = 1;
}
/*=================================================================================================
** @Name      : void DiagErrFilterInit(void)
** @Input     : swhSort:接触器名称[0,11] state:控制接触器状态 0:断开 1:闭合
** @Output    : void
** @Function  : 故障诊断滤波初始化
** @The notes : 滤波处理数组清零
**===============================================================================================*/
void DiagErrFilterInit(void)
{
    e_ERR_ID i = eERR_ID_FINISH;
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
		if(clunum >= 2)
		{
			break;
		}
		i = eERR_ID_FINISH;
        /*所有故障位*/
        for(i = (e_ERR_ID)0; i < eERR_ID_FINISH; i++)
        {
            /*根据故障检测类型设置是否使能故障检测*/
            if(gErrFilPara[i].errSort > 0)
            {
                gErrFilTab[clunum][i].enable = 1;
            }
            else
            {
                gErrFilTab[clunum][i].enable = 0;
            }

            /*复位所有故障滤波参数*/
            gErrFilTab[clunum][i].nowState = eErr_A;
            gErrFilTab[clunum][i].filState = eErr_A;
            gErrFilTab[clunum][i].resTimes = 0;
            gErrFilTab[clunum][i].timer = 0;
        }
    }
}
/*=================================================================================================
** @Name      : void DiagErrFilterTask(u8 from, u8 to)
** @Input     : from-to:检测故障滤波起始和结束ID(包括from不包to)
** @Output    : void
** @Function  : 故障诊断滤波任务
** @The notes : 50ms周期调用,与DIAG_FILTER_TICK一致
**===============================================================================================*/
void DiagErrFilterTask(e_ERR_ID from, e_ERR_ID to)
{
	e_ERR_ID i = eERR_ID_FINISH;
	e_ERR_ID temp = eERR_ID_FINISH;

    if(to > temp)                                          /*to必须小于最大值*/
    {
        to = temp;
    }

    if(from > temp)                                        /*from必须小于最大值*/
    {
        from = temp;
    }

    if(from > to)                                           /*from 必须小于等于to*/
    {
        temp = from;
        from = to;
        to = temp;
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
		if(clunum >= 2)
		{
			break;
		}
        for(u16 slot = 0; slot < DIAG_FILTER_NUM_CYCLES; slot++)
        {
            u16 subStart = (u16)slot * DIAG_FILTER_SUB_SLOT_SIZE;
            u16 subEnd = subStart + DIAG_FILTER_SUB_SLOT_SIZE;
            if(subEnd > (u16)to) subEnd = (u16)to;
            for(i = (e_ERR_ID)subStart; i < (e_ERR_ID)subEnd; i++)  /*逐个检测指定故障id*/
			{
				if(IsEqual(gErrFilTab[clunum][i].enable, 1))                /*该故障id使能检测*/
				{
					/*当前稳定状态*/
					switch(gErrFilTab[clunum][i].filState)
					{
						case eErr_A:                                /*正常状态*/
							if(IsEqual(gErrFilTab[clunum][i].nowState, eErr_B))/*如果其瞬态值是eErr_B则设置为正常到故障的过渡态*/
							{
								gErrFilTab[clunum][i].filState = eErr_AtoB;
								gErrFilTab[clunum][i].timer = 0;
							}
							break;

						case eErr_AtoB:                             /*正常到故障的过渡态*/
							if(IsEqual(gErrFilTab[clunum][i].nowState, eErr_B))
							{
								gErrFilTab[clunum][i].timer++;/*如果次数达到了设置值则变化为故障状态*/
								if(gErrFilTab[clunum][i].timer > (gErrFilPara[i].AtoBTime / DIAG_FILTER_TICK))/*50ms周期*/
								{
									/*计算A->B的次数*/
									if(gErrFilTab[clunum][i].resTimes < 254)/*防止越界*/
									{
										gErrFilTab[clunum][i].resTimes++;   /*次数加1*/
									}

									gErrFilTab[clunum][i].filState = eErr_B;
									gErrFilTab[clunum][i].timer = 0;
								}
							}
							else if(IsEqual(gErrFilTab[clunum][i].nowState, eErr_A))/*A->B过渡状态下如果其瞬态值是eErr_A的则设置其恢复为正常*/
							{
								gErrFilTab[clunum][i].filState = eErr_A;
								gErrFilTab[clunum][i].timer = 0;
							}
							break;

						case eErr_B:                                /*故障状态*/
							if(IsEqual(gErrFilTab[clunum][i].nowState, eErr_A))/*如果其瞬态值是eErr_A则设置为故障到正常的过渡态*/
							{
								gErrFilTab[clunum][i].filState = eErr_BtoA;
								gErrFilTab[clunum][i].timer = 0;
							}
							break;

						case eErr_BtoA:                             /*故障到正常的过渡态*/
							if(IsEqual(gErrFilTab[clunum][i].nowState, eErr_A))
							{
								gErrFilTab[clunum][i].timer++;/*如果次数达到了设置值则变化为正常状态*/
								if(gErrFilTab[clunum][i].timer > (gErrFilPara[i].BtoATime / DIAG_FILTER_TICK))/*50ms周期*/
								{
									gErrFilTab[clunum][i].filState = eErr_A;
									gErrFilTab[clunum][i].timer = 0;
								}
							}
							else if(IsEqual(gErrFilTab[clunum][i].nowState, eErr_B))/*B->A过渡状态下如果其瞬态值是eErr_B的则设置恢复为故障*/
							{
								gErrFilTab[clunum][i].filState = eErr_B;
								gErrFilTab[clunum][i].timer = 0;
							}
							break;

						default:
							break;
					}
				}
			}
			osDelay(1);
    	}
    }
    DiagCheckErrResetClrTimes();
}

/*=================================================================================================
** @Name      : void DiagErrFilStartAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : void
** @Function  : 启动该故障诊断滤波
** @The notes :
**===============================================================================================*/
void DiagErrFilStartAPI(u8 clunum, e_ERR_ID id)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}
	if(id < eERR_ID_FINISH)
	{
		gErrFilTab[clunum][id].enable = 1;
		gErrFilTab[clunum][id].nowState = eErr_A;
		gErrFilTab[clunum][id].filState = eErr_A;
		gErrFilTab[clunum][id].resTimes = 0;
		gErrFilTab[clunum][id].timer = 0;
	}
}
/*=================================================================================================
** @Name      : void DiagErrFilStopAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : void
** @Function  : 停止该故障诊断滤波
** @The notes :
**===============================================================================================*/
void DiagErrFilStopAPI(u8 clunum, e_ERR_ID id)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}
	if(id < eERR_ID_FINISH)
	{
		gErrFilTab[clunum][id].enable = 0;
		gErrFilTab[clunum][id].nowState = eErr_A;
		gErrFilTab[clunum][id].filState = eErr_A;
		gErrFilTab[clunum][id].resTimes = 0;
		gErrFilTab[clunum][id].timer = 0;
	}
}

/*=================================================================================================
** @Name      : void DiagSetErrNowStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state)
** @Input     : clunum:簇编号 id:故障ID state:目标故障状态
** @Output    : void
** @Function  : 设置故障诊断滤波当前瞬时状态
** @The notes :
**===============================================================================================*/
void DiagSetErrNowStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}
	if((id < eERR_ID_FINISH) && (1 == gErrFilTab[clunum][id].enable))
	{
	    /*检测是否可以设置恢复为A*/
	    if((255 != gErrFilPara[id].maxTimes)                /*非无限次恢复*/
	        && (eErr_B == gErrFilTab[clunum][id].filState)          /*正处于B状态*/
	        && (eErr_A == state))                           /*需要设置为A状态*/
	    {
	        if(gErrFilTab[clunum][id].resTimes >= gErrFilPara[id].maxTimes)/*达到最大恢复次数*/
	        {
	            return;                                     /*不能设置恢复为A*/
	        }
	    }

		gErrFilTab[clunum][id].nowState = state;
	}
}

/*=================================================================================================
** @Name      : void DiagSetErrFilStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state)
** @Input     : clunum:簇编号 id:故障ID state:目标故障状态
** @Output    : void
** @Function  : 设置故障诊断滤波当前稳定状态
** @The notes : 只能eErr_A或eErr_B
**===============================================================================================*/
void DiagSetErrFilStateAPI(u8 clunum, e_ERR_ID id, e_ErrState state)
{
    if(clunum >= 2)
    {
        return;
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}
	if((id < eERR_ID_FINISH) && (1 == gErrFilTab[clunum][id].enable)
		&& ((eErr_A == state) || (eErr_B == state)))
	{
	    /*检测是否可以设置恢复为A*/
	    if((255 != gErrFilPara[id].maxTimes)                /*非无限次恢复*/
	        && (eErr_B == gErrFilTab[clunum][id].filState)          /*正处于B状态*/
	        && (eErr_A == state))                           /*需要设置为A状态*/
	    {
	        if(gErrFilTab[clunum][id].resTimes >= gErrFilPara[id].maxTimes)/*达到最大恢复次数*/
	        {
	            return;                                     /*不能设置恢复为A*/
	        }
	    }

	    /*计算A->B的次数*/
	    if((eErr_A == DiagGetErrLastStateAPI(clunum,id))           /*正处于A状态*/
	        && (eErr_B == state))                           /*需要设置为B状态*/
	    {
	        if(gErrFilTab[clunum][id].resTimes < 254)               /*防止越界*/
	        {
	        	gErrFilTab[clunum][id].resTimes++;                  /*次数加1*/
	        }
	    }

		gErrFilTab[clunum][id].nowState = state;
		gErrFilTab[clunum][id].filState = state;
	}
}

/*=================================================================================================
** @Name      : void DiagClrErrNorStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID,0xff所有故障
** @Output    : void
** @Function  : 清除故障诊断滤波当前故障状态
** @The notes :
**===============================================================================================*/
void DiagClrErrNorStateAPI(u8 clunum, e_ERR_ID id)
{
	u16 i = 0;
    if(clunum >= 2)
    {
        return;
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}

	if(id < eERR_ID_FINISH)
	{
		gErrFilTab[clunum][id].nowState = eErr_A;
		gErrFilTab[clunum][id].filState = eErr_A;
		gErrFilTab[clunum][id].resTimes = 0;
		gErrFilTab[clunum][id].timer = 0;
	}
	else if(0xff == id)
	{
		for(i = 0; i < eERR_ID_FINISH; i++)
		{
			gErrFilTab[clunum][i].nowState = eErr_A;
			gErrFilTab[clunum][i].filState = eErr_A;
			gErrFilTab[clunum][i].resTimes = 0;
			gErrFilTab[clunum][i].timer = 0;
		}
	}
}
/*=================================================================================================
** @Name      : e_ErrSort DiagGetFilErrSortAPI(e_ERR_ID id)
** @Input     : id:故障ID
** @Output    : 故障类型[0,8]
** @Function  : 获取故障类型
** @The notes :
**===============================================================================================*/
e_ErrSort DiagGetFilErrSortAPI(e_ERR_ID id)
{
	if(id < eERR_ID_FINISH)
	{
		/*判断该故障为屏蔽保护故障则转为降低保护类型*/
        if((gErrFilPara[id].errSort >= eErr_ChgLim)
            && (1 == DiagGetErrProMaskStateHOOK(0,id)))
        {
        	/*充电限制放电限制类故障转为仅告警状态*/
			if((eErr_ChgLim == gErrFilPara[id].errSort)
				|| (eErr_DhgLim == gErrFilPara[id].errSort))
			{
				return(eErr_Warm);
			}
			/*充电保护转为充电限制状态*/
			else if(eErr_ChgPro == gErrFilPara[id].errSort)
			{
				return(eErr_ChgLim);
			}
			/*放电保护转为放电限制状态(确保紧急模式下也能强制紧急备电)*/
			else if(eErr_DhgPro == gErrFilPara[id].errSort)
			{
				return(eErr_DhgLim);
			}
			/*充放电限制类故障和充电保护转为仅告警状态(确保紧急模式下也能强制强制充放电)*/
			else if((eErr_SysLim == gErrFilPara[id].errSort)
				|| (eErr_SysPro == gErrFilPara[id].errSort)
				|| (eErr_SysErr == gErrFilPara[id].errSort))
			{
				return(eErr_Warm);
			}
		}

        /*返回故障类型*/
        return(gErrFilPara[id].errSort);
	}
	else
	{
		return(eErr_Norm);
	}
}

/*=================================================================================================
** @Name      : e_ErrState DiagGetErrNowStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : 当前瞬时故障状态
** @Function  : 获取故障当前瞬时状态
** @The notes : 不包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrNowStateAPI(u8 clunum, e_ERR_ID id)
{
    if(clunum >= 2)
    {
		printf("clunum para error\r\n");
        return(eErr_A);
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return(eErr_A);
	}
	if(id < eERR_ID_FINISH)
	{
		return(gErrFilTab[clunum][id].nowState);
	}

	return(eErr_A);
}
/*=================================================================================================
** @Name      : e_ErrState DiagGetErrFilStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : 当前滤波故障状态
** @Function  : 获取故障当前滤波状态
** @The notes : 包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrFilStateAPI(u8 clunum, e_ERR_ID id)
{
    if(clunum >= 2)
    {
        return(eErr_A);
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return(eErr_A);
	}
	if(id < eERR_ID_FINISH)
	{
		return(gErrFilTab[clunum][id].filState);
	}

	return(eErr_A);
}

/*=================================================================================================
** @Name      : e_ErrState DiagGetErrLastStateAPI(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号  id:故障ID
** @Output    : 当前稳定故障状态
** @Function  : 获取故障当前稳定状态
** @The notes : 不包括过渡状态
**===============================================================================================*/
e_ErrState DiagGetErrLastStateAPI(u8 clunum, e_ERR_ID id)
{
    if(clunum >= 2)
    {
        return (eErr_A);
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return (eErr_A);
	}
	if(id < eERR_ID_FINISH)
	{
	    /*如果a to b 则返回a*/
		if(IsEqual(gErrFilTab[clunum][id].filState, eErr_AtoB))
		{
		    return(eErr_A);
		}
	    /*如果b to a 则返回b*/
		else if(IsEqual(gErrFilTab[clunum][id].filState, eErr_BtoA))
		{
		    return(eErr_B);
		}

		return(gErrFilTab[clunum][id].filState);
	}

	return(eErr_A);
}

/*=================================================================================================
** @Name      : u16 DiagGetFilAtoBTimeAPI(u8 id)
** @Input     : id:故障ID
** @Output    : 故障滤波故障发生检测时长 0.1s
** @Function  : 获取故障滤波故障发生检测时长
** @The notes : 故障发生延时
**===============================================================================================*/
u16 DiagGetFilAtoBTimeAPI(u8 id)
{
	u16 time = 0;

	if((id < eERR_ID_FINISH) && (gErrFilPara[id].errSort > eErr_Norm))
	{
	    /*转化为0.1s单位*/
		time = gErrFilPara[id].AtoBTime / 100;
	}

	return(time);
}
/*=================================================================================================
** @Name      : e_ErrSort DiagGetGroupNowErrLevelAPI(u8 clunum, u8 type)
** @Input     : clunum:簇编号 type:故障类别:0::全部故障 1;充电类故障 2:放电类故障
** @Output    : 电池组当前故障级别[0,8]
** @Function  : 获取电池组当前故障级别
** @The notes : 当前故障的最高级别
**===============================================================================================*/
e_ErrSort DiagGetGroupNowErrLevelAPI(u8 clunum, u8 type)
{
	e_ERR_ID i = eERR_ID_FINISH;
	e_ErrSort sort = eErr_Norm;
    if(clunum >= 2)
    {
        return(0);
    }
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return(0);
	}
    for(i = (e_ERR_ID)0; i < eERR_ID_FINISH; i++)
    {
        if(eErr_Norm == DiagGetFilErrSortAPI(i))            /*不检测类型故障*/
        {
            ;                                               /*无故障*/
        }
        else if(eErr_Warm == DiagGetFilErrSortAPI(i))       /*提示类型故障*/
        {
            if(eErr_B == DiagGetErrLastStateAPI(clunum, i))         /*出现提示类异常*/
            {
                if(sort < eErr_Warm)                        /*未检测到级别更高的异常*/
                {
                    sort = eErr_Warm;                       /*设置为提示级别类型*/
                }
            }
        }
        else if(eErr_SysErr == DiagGetFilErrSortAPI(i))     /*故障类型故障*/
        {
            if(eErr_B == DiagGetErrLastStateAPI(clunum, i))         /*出现停机类异常*/
            {
                sort = eErr_SysErr;                         /*设置为停机级别类型*/
                break;                                      /*已是最高级,不需再检测*/
            }
        }
        else if(eErr_ChgLim == DiagGetFilErrSortAPI(i))     /*充电限制类型故障*/
        {
        	if((0 == type) || (1 == type))                  /*检测充电类故障*/
        	{
				if(eErr_B == DiagGetErrLastStateAPI(clunum, i))     /*出现充电限制类异常*/
				{
					if(sort == eErr_DhgLim)                 /*已检测到放电限制的异常*/
					{
						sort = eErr_SysLim;                 /*设置为充放电限制级别类型*/
					}
					else if(sort < eErr_ChgLim)             /*未检测到级别更高的异常*/
					{
						sort = eErr_ChgLim;                 /*设置为充电限制级别类型*/
					}
				}
        	}
        }
        else if(eErr_DhgLim == DiagGetFilErrSortAPI(i))     /*放电限制类型故障*/
        {
        	if((0 == type) || (2 == type))                  /*检测放电类故障*/
        	{
				if(eErr_B == DiagGetErrLastStateAPI(clunum, i))     /*出现放电限制类异常*/
				{
					if(sort == eErr_ChgLim)                 /*已检测到充电限制的异常*/
					{
						sort = eErr_SysLim;                 /*设置为充放电限制级别类型*/
					}
					else if(sort < eErr_DhgLim)             /*未检测到级别更高的异常*/
					{
						sort = eErr_DhgLim;                 /*设置为放电限制级别类型*/
					}
				}
        	}
        }
        else if(eErr_SysLim == DiagGetFilErrSortAPI(i))     /*充放电限制类型故障*/
        {
            if(eErr_B == DiagGetErrLastStateAPI(clunum, i))         /*出现充放电限制类异常*/
            {
                if(sort < eErr_SysLim)                      /*未检测到级别更高的异常*/
                {
                    sort = eErr_SysLim;                     /*设置为放电限制级别类型*/
                }
            }
        }
        else if(eErr_ChgPro == DiagGetFilErrSortAPI(i))     /*充电保护类型故障*/
        {
        	if((0 == type) || (1 == type))                  /*检测充电类故障*/
        	{
				if(eErr_B == DiagGetErrLastStateAPI(clunum, i))     /*出现充电保护类异常*/
				{
					if(sort == eErr_DhgPro)                 /*已检测到放电保护的异常*/
					{
						sort = eErr_SysPro;                 /*设置为充放电保护级别类型*/
					}
					else if(sort < eErr_ChgPro)             /*未检测到级别更高的异常*/
					{
						if(gErrFilTab[clunum][i].resTimes >= gErrFilPara[i].maxTimes)/*达到故障最大恢复次数*/
						{
							sort = eErr_SysPro;             /*设置为充放电保护级别类型*/
						}
						else                                /*未达到允许恢复次数*/
						{
							sort = eErr_ChgPro;             /*设置为充电保护级别类型*/
						}
					}
				}
        	}
        }
        else if(eErr_DhgPro == DiagGetFilErrSortAPI(i))     /*放电保护类型故障*/
        {
        	if((0 == type) || (2 == type))                  /*检测放电类故障*/
        	{
				if(eErr_B == DiagGetErrLastStateAPI(clunum, i))     /*出现放电保护类异常*/
				{
					if(sort == eErr_ChgPro)                 /*已检测到充电保护的异常*/
					{
						sort = eErr_SysPro;                 /*设置为充放电保护级别类型*/
					}
					else if(sort < eErr_DhgPro)             /*未检测到级别更高的异常*/
					{
						if(gErrFilTab[clunum][i].resTimes >= gErrFilPara[i].maxTimes)/*达到故障最大恢复次数*/
						{
							sort = eErr_SysPro;             /*设置为充放电保护级别类型*/
						}
						else                                /*未达到允许恢复次数*/
						{
							sort = eErr_DhgPro;             /*设置为放电保护级别类型*/
						}
					}
				}
        	}
        }
        else if(eErr_SysPro == DiagGetFilErrSortAPI(i))     /*充放电保护类型故障*/
        {
            if(eErr_B == DiagGetErrLastStateAPI(clunum, i))   /*出现充放电保护类异常*/
            {
                if(sort < eErr_SysPro)                      /*未检测到级别更高的异常*/
                {
                    sort = eErr_SysPro;                     /*设置为放电保护级别类型*/
                }
            }
        }
    }

    return(sort);
}

/*=================================================================================================
** @Name      : static void DiagCheckErrResetClrTimes(void)
** @Input     : void
** @Output    : void
** @Function  : 检测复位故障消除次数
** @The notes : 50ms周期调用
**===============================================================================================*/
static void DiagCheckErrResetClrTimes(void)
{
	e_ERR_ID i = eERR_ID_FINISH;
    static u32 sHisErrTime[2][eERR_ID_FINISH] = {0};
    if(gErrFilTab == NULL)
	{
		printf("gErrFilTab is null\r\n");
		return;
	}
	for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
            break;
        for(u16 slot = 0; slot < DIAG_FILTER_NUM_CYCLES; slot++)
        {
            u16 rstStart = (u16)slot * DIAG_FILTER_SUB_SLOT_SIZE;
            u16 rstEnd = rstStart + DIAG_FILTER_SUB_SLOT_SIZE;
            if(rstEnd > (u16)eERR_ID_FINISH) rstEnd = (u16)eERR_ID_FINISH;
            for(i = (e_ERR_ID)rstStart; i < (e_ERR_ID)rstEnd; i++)  /*记录每个告警/故障产生时间*/
        {
            if((255 != gErrFilPara[i].maxTimes)                             /*非无限次消除告警*/
                && (eErr_B == DiagGetErrLastStateAPI(clunum,i)))                   /*目前为告警状态*/
            {
                sHisErrTime[clunum][i] = GetSysSecondTimeAPI();                     /*记录告警时间*/
            }
            else
            {
                if((ABS(GetSysSecondTimeAPI(), sHisErrTime[clunum][i]) >= 600)      /*超过10Min*/
                    && (gErrFilTab[clunum][i].resTimes < gErrFilPara[i].maxTimes))  /*未达到最大恢复次数*/
                {
                    gErrFilTab[clunum][i].resTimes = 0;                             /*恢复次数清零*/
                }
            }
        }
        osDelay(1);
    }
    }
}





