/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CellAFESamp.c
** @Founder      : ZQD808
** @Date         : 2025.01.23
** @Function     : 单体电压采样模块
** @Instructions :
**===========================================================================================*/
#include "CellAFESamp.h"
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "BSPUserAPI.h"
#include <stdio.h>
#include <debug.h>
#include "DiagInterface.h"
#include "DiagCheck.h"
#include "DiagFilter.h"

//电芯相关信息
extern u16 *gGCellVoltInfo_1;        /*组1电池单体电压信息*/
extern u16 *gGCellVoltInfo_2;        /*组2电池单体电压信息*/
extern s16 *gGCellTempInfo_1;        /*组1电池单体温度信息*/
extern s16 *gGCellTempInfo_2;        /*组2电池单体温度信息*/
extern u16 *gGCellResisInfo_1;       /*组1电池单体内阻信息*/
extern u16 *gGCellResisInfo_2;       /*组2电池单体内阻信息*/
extern u16 *gGChgBalanInfo_1;        /*组1充电均衡状态信息*/
extern u16 *gGChgBalanInfo_2;        /*组2充电均衡状态信息*/
extern u16 *gGDhgBalanInfo_1;        /*组1放电均衡状态信息*/
extern u16 *gGDhgBalanInfo_2;        /*组2放电均衡状态信息*/
extern u8 *gGAfeStateInfo_1;         /*组1AFE状态信息*/
extern u8 *gGAfeStateInfo_2;         /*组2AFE状态信息*/

extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/


static u8 sVSampFinFlag[CLUTER_MAX_NUM] = {0};      /*电压采样完成标志，簇1和簇2*/
static u8 sTSampFinFlag[CLUTER_MAX_NUM] = {0};      /*温度采样完成标志，簇1和簇2*/
static u8 sRSampFinFlag[CLUTER_MAX_NUM] = {0};      /*内阻采样完成标志，簇1和簇2*/

/*=============================================================================================
** @Name      : void WriteSampCellVoltHook( u8 CluNum,u8 ic_Num, u8 num, u16 volt)
** @Input     : CluNum:第几簇  ic_Num:第几个从控[0,31]  num:单体电池号 volt:单节电压值 1mv
** @Output    : void
** @Function  : 输出单节电压采样值
** @The notes : 用户将输出值放到单体寄存器保存
**===========================================================================================*/
void WriteSampCellVoltHook(u8 CluNum, u8 ic_Num, u8 num, u16 volt)
{
    u32 voltage = 0;
    static u8 FirstPowerFlag = 0;
	static u8 FirstPowerFlag1 = 0;
    #if(0 == BMU_BSU_COM_TYPE)/*菊花链单体采样方式*/
    static u8 voltErrTime[MAX_LTC6813_NUM][MAXVOLTNUM] = {0};/*软硬写死最大支持22个LTC6813，如有增加软件需要变更*/
    #else/*CAN通信单体采样方式*/
    static u8 voltErrTime[CLUTER_MAX_NUM][MAX_BSU_NUM][MAX_BSU_CELL] = {0};  /*簇1软硬写死最大支持416个电芯*/
    //static u8 voltErrTime2[MAX_BSU_NUM][MAX_BSU_CELL] = {0};  /*簇2软硬写死最大支持416个电芯*/
    #endif

    //一个PACK最大电芯数量为单个PACK模组数量*单个模组电芯数量
    if(num >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]))
    {
        return;
    }
    //簇序号大于参数设置的簇数
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return;
    }
    if(BSPGetOSMillSecTimeAPI() > 5000 && FirstPowerFlag == 0)
	{
		FirstPowerFlag = 1;
	}
	if(BSPGetOSMillSecTimeAPI() > 30000 && FirstPowerFlag1 == 0)
	{
		FirstPowerFlag1 = 1;
	}

    /*检测程序模式不考虑异常*/
    //if(eMODE_DEBUG == GetGWorkModeAPI())
    {

        if((sVSampFinFlag[CluNum] == 0)                             /*未完成一遍采样*/
            || (BSPGetOSMillSecTimeAPI() < 5000 && FirstPowerFlag == 0)            /*上电前5s*/
            || (ABS(GetBsuCellVolt(CluNum,ic_Num, num), volt) > 100))/*出现电压突变100mv*/
        {
            InputBsuCellVolt(CluNum, ic_Num, num, volt);			/*不滤波*/
        }
        else                                            	/*正常采样*/
        {
            if(ABS(GetBsuCellVolt(CluNum, ic_Num, num), volt) > 2)
            {
                voltage = ((u32)GetBsuCellVolt(CluNum,ic_Num, num) * 20 + (u32)volt * 12) / 32;
            }
            else
            {
                voltage = volt;
            }

            InputBsuCellVolt(CluNum, ic_Num, num, voltage);
        }
        return;
    }
    /*出现采样电路异常但断线故障未反馈到BMU则不更新单体信息(保证BMU出现电压失效后不报高低压)*/
    if((DiagGetCellSampErrFlagAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum, eERR_ID_SAMP_EXP)))
    {
        return;
    }

    /*出现电压断线但断线故障未反馈到BMU则不更新单体信息(保证BMU出现电压断线后不报高低压)*/
    if((DiagGetCellVOffCellBitAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum, eERR_ID_SAMP_VOFF)))
    {
        return;
    }

    /*刚上电未完成一次采样或未接收全部单体信息不进行滤波*/
    if((sVSampFinFlag[CluNum] == 0) || (BSPGetOSMillSecTimeAPI() < 5000 && FirstPowerFlag == 0) 
    || (2 != DiagGetRcvAllCellFlagAPI(CluNum))
    )
    {
    	InputBsuCellVolt(CluNum, ic_Num, num, volt);
    }
    else
    {
        if((BSPGetOSMillSecTimeAPI() > 30000 || FirstPowerFlag1 == 1)                /*系统已运行30s*/
            && (ABS(GetBsuCellVolt(CluNum, ic_Num, num), volt) > 30))/*出现电压突变30mv*/
        {

            voltErrTime[CluNum][ic_Num][num]++;                     /*异常次数*/
            if(voltErrTime[CluNum][ic_Num][num] >= 3)               /*异常连续3次*/
            {
            	InputBsuCellVolt(CluNum, ic_Num, num, volt);
                voltErrTime[CluNum][ic_Num][num] = 0;
            }
        }
        else                                            	/*上电前期或电压正常*/
        {
            if(ABS(GetBsuCellVolt(CluNum, ic_Num, num), volt) > 2)
            {
                voltage = ((u32)GetBsuCellVolt(CluNum, ic_Num, num) * 20 + (u32)volt * 12) / 32;
            }
            else
            {
                voltage = volt;
            }

            InputBsuCellVolt(CluNum, ic_Num, num, (u16)voltage);
            voltErrTime[CluNum][ic_Num][num] = 0;
        }
    }
}

/*=============================================================================================
** @Name      : void WriteSampCellTempHook(u8 ic_Num, u8 num, s16 temp)
** @Input     : ic_Num:第几个从控[0,31] num:单体温度序号[0,3] temp:单节温度值 0.1℃
** @Output    : void
** @Function  : 输出单体电池温度采样值
** @The notes : 用户将输出值放到单体寄存器保存
**===========================================================================================*/
void WriteSampCellTempHook(u8 CluNum, u8 ic_Num, u8 num, s16 temp)
{
    s16 temperature = 0;
	static u8 FirstPowerFlag = 0;	
    if(num >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum]*gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum])
    {
        return;
    }
	if(BSPGetOSMillSecTimeAPI() > 10000 && FirstPowerFlag == 0)
	{
		FirstPowerFlag = 1;
	}
	/*检测程序模式不滤波*/
	if(eMODE_DEBUG == GetGWorkModeAPI(CluNum))
	{
		InputBsuCellTemp(CluNum, ic_Num, num, temp);
		return;
	}

    /*出现采样电路异常但断线故障未反馈到BMU则不更新单体信息(保证BMU出现温度失效后不报高低温)*/
    if((DiagGetCellSampErrFlagAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum, eERR_ID_SAMP_EXP)))
    {
        return;
    }

	/*出现温度断线但断线故障未反馈到BMU则不更新单体信息(保证BMU出现温度断线后不报高低温)*/
	if((DiagGetCellTOffCellBitAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum,eERR_ID_SAMP_TOFF)))
	{
		return;
	}

	/*刚上电未完成一次采样或未接收全部单体信息不进行滤波*/
	if((sTSampFinFlag[CluNum] == 0) || ((BSPGetOSMillSecTimeAPI() < 10000) && (FirstPowerFlag == 0)) 
    || (2 != DiagGetRcvAllCellFlagAPI(CluNum))
    )
	{
		InputBsuCellTemp(CluNum, ic_Num, num, temp);
	}
	else
	{
		/*跳动超过2℃进行滤波*/
		if(ABS(GetBsuCellTemp(CluNum, ic_Num, num), temp) > 20)
		{
			temperature = ((s16)GetBsuCellTemp(CluNum, ic_Num, num) * 20 + (s16)temp * 12) / 32;
		}
		else
		{
			temperature = temp;
		}

		InputBsuCellTemp(CluNum,ic_Num, num, temperature);
	}
}

/*=============================================================================================
** @Name      : void WriteCellVSampFinFlagHook(u8 CluNum,u8 flag)
** @Input     : flag:完成标志 0未完成 1完成
** @Output    : void
** @Function  : 输出单节电压采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
void WriteCellVSampFinFlagHook(u8 CluNum,u8 flag)
{
    if(flag > 1)
    {
        return;
    }
    //簇序号大于参数设置的簇数
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return;
    }

    /*出现采样电路异常但断线故障未反馈到BMU则不更新单体信息(保证BMU出现电压失效后不报高低压)*/
    if((DiagGetCellSampErrFlagAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum, eERR_ID_SAMP_EXP)))
    {
        return;
    }

    /*出现电压断线但断线故障未反馈到BMU则不更新单体信息(保证BMU出现电压断线后不报高低压)*/
    if((DiagGetCellVOffCellBitAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum, eERR_ID_SAMP_VOFF)))
    {
        return;
    }
    sVSampFinFlag[CluNum] = flag;
}
/*=============================================================================================
** @Name      :u8 GetCellVSampFinFlagAPI(u8 CluNum)
** @Input     : void
** @Output    : 完成标志 0未完成 1完成
** @Function  : 获取单节电压采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
u8 GetCellVSampFinFlagAPI(u8 CluNum)
{
    //簇序号大于参数设置的簇数
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return 0;
    }
    return(sVSampFinFlag[CluNum]);
}

/*=============================================================================================
** @Name      : void WriteCellTSampFinFlagHook(u8 CluNum, u8 flag)
** @Input     : flag:完成标志 0未完成 1完成
** @Output    : void
** @Function  : 输出单节温度采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
void WriteCellTSampFinFlagHook(u8 CluNum, u8 flag)
{
    if(flag > 1)
    {
        return;
    }

    //簇序号大于参数设置的簇数
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return;
    }

    /*出现温度断线但断线故障未反馈到BMU则不更新单体信息(保证BMU出现温度断线后不报高低温),后续等该函数实现再取消屏蔽*/
    if((DiagGetCellTOffCellBitAPI(CluNum) > 0) && (eErr_A == DiagGetErrLastStateAPI(CluNum, eERR_ID_SAMP_TOFF)))
    {
        return;
    }

    sTSampFinFlag[CluNum] = flag;
}

/*=============================================================================================
** @Name      : u8 GetCellTSampFinFlagAPI(u8 CluNum)
** @Input     : CluNum ：簇编号
** @Output    : 完成标志 0未完成 1完成
** @Function  : 获取单节温度采样第一遍完成标志
** @The notes : 菊花链或CAN通信接收完成
**===========================================================================================*/
u8 GetCellTSampFinFlagAPI(u8 CluNum)
{
        //簇序号大于参数设置的簇数
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return 0;
    }
    return(sTSampFinFlag[CluNum]);
}
/*=============================================================================================
** @Name      : void ClrSampCellVoltHook(u8 CluNum, u8 ic_Num)
** @Input     : CluNum:簇编号 ic_Num:第几个从控[0,31]
** @Output    : void
** @Function  : 清除指定从控AFE所有单体电压采样值为0
** @The notes : AFE单体电压采样失效或CAN通信中断超时时调用
**===========================================================================================*/
void ClrSampCellVoltHook(u8 CluNum, u8 ic_Num)
{
    u8 i = 0;
    if(CluNum >= 2)
        return;
    if(gGBmuHigLevPara_103 == NULL)
    {
        printf("gGBmuHigLevPara_103 is null !!!\r\n");
        return;
    }
    /*所有电池单体电压*/
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]; i++)
    {
        /*输出单体电压值为0*/
        //WriteSampCellVoltHook(ic_Num, i, 0);
        /*不滤波不平滑*/
        InputBsuCellVolt(CluNum,ic_Num, i, 0);
    }
}
/*=============================================================================================
** @Name      : void ClrSampCellTempHook(u8 CluNum, u8 ic_Num)
** @Input     : CluNum:簇编号 ic_Num:第几个从控[0,31]
** @Output    : void
** @Function  : 清除指定从控AFE所有单体温度和均衡板温温度采样值为0
** @The notes : AFE单体温度采样失效或CAN通信中断超时时调用
**===========================================================================================*/
void ClrSampCellTempHook(u8 CluNum, u8 ic_Num)
{
    u8 i = 0;
    if(CluNum >= 2)
        return;
    if(gGBmuHigLevPara_103 == NULL)
    {
        printf("gGBmuHigLevPara_103 is null !!!\r\n");
        return;
    }
    /*所有电池单体电压*/
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]; i++)
    {
        /*输出单体电压值为0*/
        //WriteSampCellVoltHook(ic_Num, i, 0);
        /*不滤波不平滑*/
        InputBsuCellTemp(CluNum,ic_Num, i,-410);
    }
}

/*=============================================================================================
** @Name      : void WriteSampCellResisHook(u8 CluNum, u8 ic_Num, u8 num, u16 resis)
** @Input     : CluNum:第几簇 ic_Num:第几个从控[0,31] num:单体电池号 resis:内阻值 1μΩ
** @Output    : void
** @Function  : 输出单节内阻采样值
** @The notes : 用户将输出值放到单体寄存器保存
**===========================================================================================*/
void WriteSampCellResisHook(u8 CluNum, u8 ic_Num, u8 num, u16 resis)
{
    if(num >= (gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]))
    {
        return;
    }
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return;
    }
    InputBsuCellResis(CluNum, ic_Num, num, resis);
}

/*=============================================================================================
** @Name      : void WriteCellRSampFinFlagHook(u8 CluNum, u8 flag)
** @Input     : CluNum:第几簇 flag:完成标志 0未完成 1完成
** @Output    : void
** @Function  : 输出单节内阻采样第一遍完成标志
** @The notes : CAN通信接收完成
**===========================================================================================*/
void WriteCellRSampFinFlagHook(u8 CluNum, u8 flag)
{
    if(flag > 1)
    {
        return;
    }
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return;
    }
    sRSampFinFlag[CluNum] = flag;
}

/*=============================================================================================
** @Name      : u8 GetCellRSampFinFlagAPI(u8 CluNum)
** @Input     : CluNum:第几簇
** @Output    : 完成标志 0未完成 1完成
** @Function  : 获取单节内阻采样第一遍完成标志
** @The notes : CAN通信接收完成
**===========================================================================================*/
u8 GetCellRSampFinFlagAPI(u8 CluNum)
{
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return 0;
    }
    return(sRSampFinFlag[CluNum]);
}

/*=============================================================================================
** @Name      : void WriteSampAfeStateHook(u8 CluNum, u8 ic_Num, u8 afeIndex, u8 afeState)
** @Input     : CluNum:第几簇 ic_Num:第几个从控[0,31] afeIndex:AFE序号[0,26] afeState:AFE状态值(4bit)
** @Output    : void
** @Function  : 输出AFE状态采样值
** @The notes : 每个AFE IC占1个u8,状态值定义见e_AfeState
**===========================================================================================*/
void WriteSampAfeStateHook(u8 CluNum, u8 ic_Num, u8 afeIndex, u8 afeState)
{
    if(CluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || CluNum >= CLUTER_MAX_NUM)
    {
        return;
    }
    InputBsuAfeState(CluNum, ic_Num, afeIndex, afeState);
}

/*=============================================================================================
** @Name      : void ClrSampCellResisHook(u8 CluNum, u8 ic_Num)
** @Input     : CluNum:簇编号 ic_Num:第几个从控[0,31]
** @Output    : void
** @Function  : 清除指定从控AFE所有单体内阻采样值为0
** @The notes : CAN通信中断超时时调用
**===========================================================================================*/
void ClrSampCellResisHook(u8 CluNum, u8 ic_Num)
{
    u8 i = 0;
    if(CluNum >= 2)
        return;
    if(gGBmuHigLevPara_103 == NULL)
    {
        return;
    }
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]; i++)
    {
        InputBsuCellResis(CluNum, ic_Num, i, 0);
    }
}

