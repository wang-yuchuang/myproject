/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CurrLimPID.c
** @Founder      : ZQD808
** @Date         : 2026.03.12
** @Function     : 电流调节PID算法模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "CurrLimPID.h"

/*=============================================================================================
** @Global variable definition
**===========================================================================================*/

/*=============================================================================================
** @Static function declaration
**===========================================================================================*/
/*=============================================================================================
** @Name      : static s32 UtilLimitRange(s32 value, s32 low, s32 high)
** @Input     : value:当前值low:下限;high:上限
** @Output    : 修正后的值
** @Function  : 修正输入值(int16)处于封闭区间
** @The notes :
**===========================================================================================*/
static s32 UtilLimitRange(s32 value, s32 low, s32 high);

/*=============================================================================================
** @Interface function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : u16 UtilPIAdjust(t_PIIO *pPiIO, const t_PICfg *pPiCfg)
** @Input     : t_PIIO *pPiIO, t_PICfg *pPiCfg
** @Output    : 0:正常1：异常
** @Function  : PI调节通用函数
** @The notes :
**===========================================================================================*/
u16 UtilPIAdjust(t_PIIO *pPiIO, const t_PICfg *pPiCfg)
{ 
	s32 temp16;         /* 16位临时变量*/
	s32 temp32;			/* 32位临时变量*/
	s32 tempPiOut = 0;	/* 积分中间计算变量*/
	s32 tempPiSum = 0;	/* 积分累加中间计算变量*/
	
	/*参数有效性检测*/
	if((pPiIO == 0) || (pPiCfg == 0))
	{
		return (1);
	}

	pPiIO->piIn = UtilLimitRange(pPiIO->piIn ,pPiCfg->piInMin,pPiCfg->piInMax);
	

	//========================P调节=============================================
	tempPiOut = (pPiIO->piIn * pPiCfg->piKp) >> (u8)(pPiCfg->piKpR);

	//========================I调节=============================================
	/*计算当前积分项*/
	temp32 = pPiIO->piIn * pPiCfg->piKi;			    
	temp16 = temp32 >> (u8)(pPiCfg->piKiR);

   	/*修改当前积分项*/
 	if(temp16 == 0)
	{
		if(temp32 > pPiCfg->piKiPrecision)
		{
			/*当前积分项在(pPiCfg->piKiPrecision) 到2的(pPiCfg->piKiR)次方之间修正为1.*/
	   		temp16 = 1;
		}
	}
	else if(temp16 == -1)
	{
		if(temp32 >= (0-(pPiCfg->piKiPrecision)))
		{
			/*当前积分项在0 到(-pPiCfg->piKiPrecision)之间修改为0*/
			temp16 = 0;
		}
	}
	else
	{
		/*不用修改*/
	}	 

	/*计算积分累加项*/
	tempPiSum = pPiIO->piSum + temp16;	
	tempPiSum = UtilLimitRange(tempPiSum,pPiCfg->piSumMin,pPiCfg->piSumMax);

	/*计算PI输出量*/
	tempPiOut  += tempPiSum;
    tempPiOut = UtilLimitRange(tempPiOut,pPiCfg->piOutMin,pPiCfg->piOutMax);

	/*修改输出参数*/
	pPiIO->piSum = tempPiSum;
	pPiIO->piOut =  tempPiOut;

	return (0);
}


/*=============================================================================================
** @Static function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : static s32 UtilLimitRange(s32 value, s32 low, s32 high)
** @Input     : value:当前值low:下限;high:上限
** @Output    : 修正后的值
** @Function  : 修正输入值(int16)处于封闭区间
** @The notes :
**===========================================================================================*/
static s32 UtilLimitRange(s32 value, s32 low, s32 high)
{
	/*判断上下限是否正常*/
	if(low < high)
	{
		value = (value < low)?low:value;              /*小于下限则返回下限值*/
		value = (value > high)?high:value;            /*大于上限则返回上限值*/
	}

	return(value);
}
