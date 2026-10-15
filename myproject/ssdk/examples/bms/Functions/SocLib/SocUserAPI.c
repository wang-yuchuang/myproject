/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocUserAPI.c
** @Founder      : ZQD808
** @Date         : 2026.03.16
** @Function     : SOC算法包用户接口
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "CapEnerCorr.h"
#include "SocSoeCorr.h"
#include "SocSoeSync.h"
#include "SocUserAPI.h"
extern u16 gGBmuGenPara_102[];		/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
/*=============================================================================================
** @Name      : void SocSoeLibAllInit(void)
** @Input     : void
** @Output    : void
** @Function  : SOC/SOE算法包模块总初始化
** @The notes :
**===========================================================================================*/
void SocSoeLibAllInit(void)
{
	/*总容量总能量自动标定*/
	TotalCapEnerCorrInit();
}
/*=============================================================================================
** @Name      : void SocSoeLibAllTask(void)
** @Input     : void
** @Output    : void
** @Function  : SOC/SOE算法包模块总任务
** @The notes : 50ms
**===========================================================================================*/
void SocSoeLibAllTask(void)
{
	static u8 sTime = 0;

	switch(sTime)
	{
		/*1s周期*/
		case 0:
			/*总容量总能量自动标定*/
			TotalCapEnerCorrTask();
			break;

		/*500ms周期*/
		case 2:
		case 12:
			/*SOC、SOE扩展卡尔曼算法任务*/
			;
			break;

		/*500ms周期*/
		case 4:
		case 14:
			/*SOC、SOE相互同步任务*/
			SocSoeTogetherSyncTask();
			break;

		/*500ms周期*/
		case 6:
		case 16:
			/*SOC、SOE组间同步任务*/
			//SocSoeGroupSyncTask();
			break;

		/*500ms周期*/
		case 8:
		case 18:
			/*SOC、SOE修正总任务*/
			SocSoeAllCorrTask();
			break;

		default:
			break;
	}

	sTime++;
	if(sTime >= 20)
	{
		sTime = 0;
	}
}

