#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
//#include "EnerInterface.h"
#include "CurrIntegral.h"
#include "PublicInfo.h"
#include "CalcInterface.h"
#include <stdio.h>
#include <debug.h>
#include "DiagCheck.h"
#include "CharactCalc.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>
#include <debug.h>


extern u16 gGBmuGenPara_102[];          /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];       /*主控高级参数值*/
extern u8 gGHardPara_104[];             /*主控硬件参数值*/
extern u16 *gGCellVoltInfo_1;           /*组1电池单体电压信息*/
extern u16 *gGCellVoltInfo_2;           /*组2电池单体电压信息*/
extern s16 *gGCellTempInfo_1;           /*组1电池单体温度信息*/
extern s16 *gGCellTempInfo_2;           /*组2电池单体温度信息*/
extern s16 *gSysMainInfo_51;            /*系统整体信息*/
extern s16 *gSysCharInfo_54;            /*系统特征信息*/
static s16 sClusAvgCurr[CLUTER_MAX_NUM] = {0};            	/*电池组平均电流(0.1A)*/
s32 (*sAvgCurTbl)[CUR_FIT_LEN] = NULL;   // 指向二维数组的指针


/*=============================================================================================
** @Name      :void CharactCalcDataInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void CharactCalcDataInit(void)
{
    static u8 FirstInitFlag = 0;
    if(FirstInitFlag == 1)
    {
        return;
    }
    // 分配 2 行 × CUR_FIT_LEN 个 s32 元素的连续内存
    sAvgCurTbl = ExtMalloc(2 * sizeof(*sAvgCurTbl));
    if (sAvgCurTbl == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sAvgCurTbl null\r\n", __func__);
        return;
    }
    else
    {
        printf("malloc sAvgCurTbl %d success!!!\r\n",2 * sizeof(*sAvgCurTbl));
        // 清零分配的内存
        memset(sAvgCurTbl, 0, 2 * sizeof(*sAvgCurTbl));
    }
    FirstInitFlag = 1;
}
/*=============================================================================================
** @Name      : void GroupDataInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电池数据信息初始化
** @The notes :
**===========================================================================================*/
void GroupDataInfoInit(void)
{
    u16 i = 0;
    if(gGCellVoltInfo_1 == NULL || gGCellTempInfo_1 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGCellVoltInfo_1 or gGCellTempInfo_1 null\r\n", __func__);
        return;
    }
    if(gGCellVoltInfo_2 == NULL || gGCellTempInfo_2 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGCellVoltInfo_2 or gGCellTempInfo_2 null\r\n", __func__);
        return;
    }
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]; i++)
    {
        gGCellVoltInfo_1[i] = DEFAULT_STANDCV;
    }
    //这里暂时先不考虑均衡温度，后续再补充
    for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]; i++)
    {
        gGCellTempInfo_1[i] = 250;
    }
    //管理两簇电池，初始化第二簇电压和温度信息
    if(gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] == 2)
    {
        for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]; i++)
        {
            gGCellVoltInfo_2[i] = DEFAULT_STANDCV;
        }
        //这里暂时先不考虑均衡温度，后续再补充
        for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]; i++)
        {
            gGCellTempInfo_2[i] = 250;
        }
    }
}
/*=============================================================================================
** @Name      : void CheckVoltCharaTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测电压特征信息任务
** @The notes : 100ms
**===========================================================================================*/
void CheckVoltCharaTask(void)
{
    u16 num = 0;
    u16 maxNum = 0;
    u16 minNum = 0;
    u16 nowVolt = 0;
    u16 maxVolt = 0;
    u16 minVolt = 0xffff;
    u32 sumVolt = 0;
    u8 clunum = 0;
    if(gGCellVoltInfo_1 == NULL || gGCellTempInfo_1 == NULL)
    {
        //printf("gGCellVoltInfo_1 is null or gGCellTempInfo_1 == null\r\n");
        ssdk_printf(SSDK_WARNING, "%s: gGCellVoltInfo_1 or gGCellTempInfo_1 null\r\n", __func__);
        return;
    }
    if(gGCellVoltInfo_2 == NULL || gGCellTempInfo_2 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGCellVoltInfo_2 or gGCellTempInfo_2 null\r\n", __func__);
        return;
    }
    if(gSysCharInfo_54 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCharInfo_54 null\r\n", __func__);
        return;
    }
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        num = 0;
        maxNum = 0;
        minNum = 0;
        nowVolt = 0;
        maxVolt = 0;
        minVolt = 0xffff;
        sumVolt = 0;
        //记录总电压采样值和预充电压采样值
        SampSumVoltFilterDeal(clunum, CalcGetDcSumSampVolt(clunum));
        OutPreVoltFilterDeal(clunum, CalcGetDcPreSampVolt(clunum));
        
        /*系统初始化时信息不齐全不检测最值*/
        if((eWORK_INIT == GetGWorkStateAPI(clunum))           /*初始化时信息不齐全不检测*/
            && ((2 != DiagGetRcvAllSlaveFlagAPI(clunum))      /*未接收到全部从控信息*/
            || (2 != DiagGetRcvAllCellFlagAPI(clunum))))      /*未接收到全部单体信息*/
        {
            continue;
        }

        for(num = 0; num < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]; num++)
        {
            if(clunum == 0)
            {
                nowVolt = gGCellVoltInfo_1[num];
            }
            else
            {
                nowVolt = gGCellVoltInfo_2[num];
            }
           

            sumVolt += nowVolt;                         /*计算总电压 */

            if(maxVolt < nowVolt)                       /*求单体最大电压和电池号*/
            {
                maxVolt = nowVolt;
                maxNum = num;
            }

            if(minVolt > nowVolt)                       /*求单体最小电压和电池号*/
            {
                minVolt = nowVolt;
                minNum = num;
            }
        }
        if(clunum == 0)
        {
            gSysCharInfo_54[eSysCharnfo54_1MaxVolt] = maxVolt;       /*组最大电压*/
            gSysCharInfo_54[eSysCharnfo54_1MinVolt] = minVolt;       /*组最小电压*/
            gSysCharInfo_54[eSysCharnfo54_1MaxVNum] = maxNum + 1;    /*组电压最大电池号(从1开始)*/
            gSysCharInfo_54[eSysCharnfo54_1MinVNum] = minNum + 1;    /*组电压最小电池号(从1开始)*/

            gSysCharInfo_54[eSysCharnfo54_1SumV] = (u16)(sumVolt / 100);/*组总电压单位为0.1V*/

            if(0 == CalcGetGroupCellVExpStateHook(0))               		/*从控单体正常才计算总电压和平均电压*/
            {
                gSysCharInfo_54[eSysCharnfo54_1AvgV] = (u16)(sumVolt / gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]);/*单节电池平均电压,单位mV*/
            }
            else
            {
                gSysCharInfo_54[eSysCharnfo54_1AvgV] = 0;		        /*单节电池平均电压,单位mV*/
            }
        }
        else
        {
            gSysCharInfo_54[eSysCharnfo54_2MaxVolt] = maxVolt;       /*组最大电压*/
            gSysCharInfo_54[eSysCharnfo54_2MinVolt] = minVolt;       /*组最小电压*/
            gSysCharInfo_54[eSysCharnfo54_2MaxVNum] = maxNum + 1;    /*组电压最大电池号(从1开始)*/
            gSysCharInfo_54[eSysCharnfo54_2MinVNum] = minNum + 1;    /*组电压最小电池号(从1开始)*/

            gSysCharInfo_54[eSysCharnfo54_2SumV] = (u16)(sumVolt / 100);/*组总电压单位为0.1V*/

            if(0 == CalcGetGroupCellVExpStateHook(1))               		/*从控单体正常才计算总电压和平均电压*/
            {
                gSysCharInfo_54[eSysCharnfo54_2AvgV] = (u16)(sumVolt / gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]);/*单节电池平均电压,单位mV*/
            }
            else
            {
                gSysCharInfo_54[eSysCharnfo54_2AvgV] = 0;		        /*单节电池平均电压,单位mV*/
            }
        }
    }
}

/*=============================================================================================
** @Name      : void CheckTempCharaTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测温度特征信息任务
** @The notes : 100ms
**===========================================================================================*/
void CheckTempCharaTask(void)
{
    u8 id = 0;
    u8 cell = 0;
    u16 num = 0;
    u16 maxNum = 0;
    u16 minNum = 0;
    s16 nowTemp = 0;
    s16 maxTemp = -410;
    s16 minTemp = 1270;
    s32 sumTemp = 0;
    u8 clunum = 0;
    if(gGCellVoltInfo_1 == NULL || gGCellTempInfo_1 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGCellVoltInfo_1 or gGCellTempInfo_1 null\r\n", __func__);
        return;
    }
    if(gGCellVoltInfo_2 == NULL || gGCellTempInfo_2 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gGCellVoltInfo_2 or gGCellTempInfo_2 null\r\n", __func__);
        return;
    }
    if(gSysCharInfo_54 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCharInfo_54 null\r\n", __func__);
        return;
    }
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        id = 0;
        cell = 0;
        num = 0;
        maxNum = 0;
        minNum = 0;
        nowTemp = 0;
        maxTemp = -410;
        minTemp = 1270;
        sumTemp = 0;
        /*系统初始化时信息不齐全不检测最值*/
        if((eWORK_INIT == GetGWorkStateAPI(clunum))           /*初始化时信息不齐全不检测*/
            && ((2 != DiagGetRcvAllSlaveFlagAPI(clunum))      /*未接收到全部从控信息*/
            || (2 != DiagGetRcvAllCellFlagAPI(clunum))))      /*未接收到全部单体信息*/
        {
            continue;
        }

        /*检测单体温度*/
        for(num = 0; num < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]; num++)
        {
            if(clunum == 0)
            {
                nowTemp = gGCellTempInfo_1[num];
            }
            else
            {
                nowTemp = gGCellTempInfo_2[num];
            }
            

            sumTemp += nowTemp;                         /*温度求和*/

            if(maxTemp < nowTemp)                       /*求单体最大温度和电池号*/
            {
                maxTemp = nowTemp;
                maxNum = num;
            }

            if(minTemp > nowTemp)                       /*求单体最小温度和电池号*/
            {
                minTemp = nowTemp;
                minNum = num;
            }
        }
        if(clunum == 0)
        {
            gSysCharInfo_54[eSysCharnfo54_1MaxTemp] = maxTemp;      /*组最大温度*/
            gSysCharInfo_54[eSysCharnfo54_1MinTemp] = minTemp;      /*组最小温度*/
            gSysCharInfo_54[eSysCharnfo54_1MaxTNum] = maxNum + 1;   /*组温度最大电池号(从1开始)*/
            gSysCharInfo_54[eSysCharnfo54_1MinTNum] = minNum + 1;   /*组温度最小电池号(从1开始)*/



            if(0 == CalcGetGroupCellTExpStateHook(clunum))               	   /*从控单体温度正常才计算平均温度*/
            {
                //四舍五入求组平均温度
                gSysCharInfo_54[eSysCharnfo54_1AvgT] = (s16)(sumTemp / gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]);
            }
            else
            {
                //显示异常值
                gSysCharInfo_54[eSysCharnfo54_1AvgT] = -410;
            }
        }
        else
        {
            gSysCharInfo_54[eSysCharnfo54_2MaxTemp] = maxTemp;      /*组最大温度*/
            gSysCharInfo_54[eSysCharnfo54_2MinTemp] = minTemp;      /*组最小温度*/
            gSysCharInfo_54[eSysCharnfo54_2MaxTNum] = maxNum + 1;   /*组温度最大电池号(从1开始)*/
            gSysCharInfo_54[eSysCharnfo54_2MinTNum] = minNum + 1;   /*组温度最小电池号(从1开始)*/



            if(0 == CalcGetGroupCellTExpStateHook(clunum))               	   /*从控单体温度正常才计算平均温度*/
            {
                //四舍五入求组平均温度
                gSysCharInfo_54[eSysCharnfo54_2AvgT] = (s16)(sumTemp / gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]);
            }
            else
            {
                //显示异常值
                gSysCharInfo_54[eSysCharnfo54_2AvgT] = -410;
            }
        }
    }
}

/*=============================================================================================
** @Name      : void CheckAvgCurrTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测电池组平均电流任务
** @The notes : 100ms
**===========================================================================================*/
void CheckAvgCurrTask(void)
{
    s16 curr = 0;
    s32 inteCap = 0;
    s32 changCap = 0;
    static u16 sTime[2] = {0};
    static s32 sHisCap[2] = {0};
    
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            return;
        }
        curr = 0;
        inteCap = 0;
        changCap = 0;
        if((sTime[clunum] / 10) >= AVG_CURR_TIME)
        {
            inteCap = GetChgDhgChangCapAPI(clunum);   /*当前容量变化值*/
            
            changCap = inteCap - sHisCap[clunum];     /*间隔时间变化量(1mAH)*/
            
            curr = (s16)((changCap * 360) / (s16)sTime[clunum]);
            
            sHisCap[clunum] = inteCap;
            sTime[clunum] = 0;
        }
        else
        {
            curr = sClusAvgCurr[clunum];
            sTime[clunum]++;
        }
        sClusAvgCurr[clunum] = curr;
    }
}
/*=============================================================================================
** @Name      : void CurrFilterDeal(u8 clunum, s32 curr, u8 flag)
** @Input     : clunum:簇编号 cur:采样电流值(0.1A) flag:立即显示标志(1:立即显示)
** @Output    : void
** @Function  : 对电流进行滤波
** @The notes : 电流采样调用
**===========================================================================================*/
void CurrFilterDeal(u8 clunum, s32 curr, u8 flag)
{
    u16 i = 0;
    s32 curSum = 0;
    s32 nowCurr = 0;
    s32 maxCurr = 0x80000000;
    s32 minCurr = 0x7fffffff;
    static u16 sLen[2] = {0};
    static u16 sTime[2] = {0};
    static s32 sAddCur[2] = {0};
    
    if(clunum >= 2)
    {
        return;
    }
    if(gSysMainInfo_51 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysMainInfo_51 null\r\n", __func__);
        return;
    }
    if(sAvgCurTbl == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: sAvgCurTbl null\r\n", __func__);
        return;
    }
    if(gSysCharInfo_54 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCharInfo_54 null\r\n", __func__);
        return;
    }
    /*不滤波*/
    if((0 == gGHardPara_104[eGHardPara104_CFilType]) || (1 == flag))
    {
         sLen[clunum] = 0;
         sTime[clunum] = 0;
         sAddCur[clunum] = 0;
         nowCurr = curr;                    /*电流值*/
    }
    /*采样值滑动求平均滤波*/
    else if(1 == gGHardPara_104[eGHardPara104_CFilType])
    {
		/*滤波次数不合法*/
		if(gGHardPara_104[eGHardPara104_CFilTime] < 1)
		{
			gGHardPara_104[eGHardPara104_CFilTime] = 1;
		}
		else if(gGHardPara_104[eGHardPara104_CFilTime] > CUR_FIT_LEN)
		{
			gGHardPara_104[eGHardPara104_CFilTime] = CUR_FIT_LEN;
		}

        sAvgCurTbl[clunum][sTime[clunum]] = curr;
        sTime[clunum]++;
        if(sTime[clunum] < gGHardPara_104[eGHardPara104_CFilTime])
        {
            /*未取满第一遍*/
            if(sLen[clunum] < gGHardPara_104[eGHardPara104_CFilTime])
            {
                sLen[clunum] = sTime[clunum];
            }
        }
        else
        {
            sLen[clunum] = gGHardPara_104[eGHardPara104_CFilTime];
            sTime[clunum] = 0;
        }

        for(i = 0; i < sLen[clunum]; i++)
        {
            curSum += sAvgCurTbl[clunum][i];

            if(sAvgCurTbl[clunum][i] > maxCurr)
            {
                maxCurr = sAvgCurTbl[clunum][i];
            }

            if(sAvgCurTbl[clunum][i] < minCurr)
            {
                minCurr = sAvgCurTbl[clunum][i];
            }
        }

        if(sLen[clunum] >= 4)
        {
        	if(curSum >= 0)
        	{
        		nowCurr = (((curSum - maxCurr - minCurr) * 10 / (sLen[clunum] - 2)) + 5) / 10;
        	}
        	else
        	{
        		nowCurr = (((curSum - maxCurr - minCurr) * 10 / (sLen[clunum] - 2)) - 5) / 10;
        	}
        }
        else
        {
        	if(curSum >= 0)
        	{
        		nowCurr = ((curSum * 10 / sLen[clunum]) + 5) / 10;
        	}
        	else
        	{
        		nowCurr = ((curSum * 10 / sLen[clunum]) - 5) / 10;
        	}
        }
    }
    /*采样值有效值滤波(暂不实现)*/
    else if(2 == gGHardPara_104[eGHardPara104_CFilType])
    {
        nowCurr = curr;
    }
    /*采样值积分求平均滤波*/
	else /*(3 == CURR_FIL_TYPE)*/
	{
		/*滤波次数不合法*/
		if(gGHardPara_104[eGHardPara104_CFilTime] < 1)
		{
			gGHardPara_104[eGHardPara104_CFilTime] = 1;
		}
		else if(gGHardPara_104[eGHardPara104_CFilTime] > CUR_FIT_LEN)
		{
			gGHardPara_104[eGHardPara104_CFilTime] = CUR_FIT_LEN;
		}

        sAddCur[clunum] += curr;

        sTime[clunum]++;
        if(sTime[clunum] >= gGHardPara_104[eGHardPara104_CFilTime])
        {
            nowCurr = (s16)(((sAddCur[clunum] * 10) / (s16)sTime[clunum] + 5) / 10);
            sAddCur[clunum] = 0;
            sTime[clunum] = 0;
        }
        else
        {
            if(clunum == 0)
            {
                nowCurr = gSysMainInfo_51[eSysMainInfo51_1OutC];
            }
            else
            {
                nowCurr = gSysMainInfo_51[eSysMainInfo51_2OutC];
            }
        }
    }

    /*在显示零飘值内*/
    if((0 - (s8)gGHardPara_104[eGHardPara104_ChgShowPoint] < nowCurr)
        && (nowCurr < (s8)gGHardPara_104[eGHardPara104_DhgShowPoint]))
    {
        nowCurr = 0;
    }
    if(clunum == 0)
    {
        gSysMainInfo_51[eSysMainInfo51_1OutC] = (s16)nowCurr;
        //计算功率(0.1KW)
        gSysMainInfo_51[eSysMainInfo51_1OutP] = (s16)(nowCurr * (s32)gSysCharInfo_54[eSysCharnfo54_1SumV] / 10000);
    }
    else
    {
        gSysMainInfo_51[eSysMainInfo51_2OutC] = (s16)nowCurr;
        //计算功率(0.1KW)
        gSysMainInfo_51[eSysMainInfo51_2OutP] = (s16)(nowCurr * (s32)gSysCharInfo_54[eSysCharnfo54_2SumV] / 10000);
    }
}

/*=============================================================================================
** @Name      : void SampSumVoltFilterDeal(u8 clunum, u16 volt)
** @Input     : clunum:簇编号 volt:采样电压值(0.1V)
** @Output    : void
** @Function  : 对电池端总电压采样值进行滤波
** @The notes : 电池输出端总电压采样值处理调用
**===========================================================================================*/
void SampSumVoltFilterDeal(u8 clunum, u16 volt)
{
    u32 nowVolt = 0;
    if(gSysMainInfo_51 == NULL)
    {
        printf("gSysMainInfo_51 is null\r\n");
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    if(clunum == 0)
    {
        //变化值在0.3V—10.0V之间
        if((ABS(gSysMainInfo_51[eSysMainInfo51_1SumV], volt) >= 3)
            && (ABS(gSysMainInfo_51[eSysMainInfo51_1SumV], volt) <= 100))
        {
            nowVolt = ((u32)gSysMainInfo_51[eSysMainInfo51_1SumV] * 12 + (u32)volt * 20) / 32;
        }
        else
        {
            nowVolt = volt;
        }

        gSysMainInfo_51[eSysMainInfo51_1SumV] = (u16)nowVolt;
    }
    else
    {
        //变化值在0.3V—10.0V之间
        if((ABS(gSysMainInfo_51[eSysMainInfo51_2SumV], volt) >= 3)
            && (ABS(gSysMainInfo_51[eSysMainInfo51_2SumV], volt) <= 100))
        {
            nowVolt = ((u32)gSysMainInfo_51[eSysMainInfo51_2SumV] * 12 + (u32)volt * 20) / 32;
        }
        else
        {
            nowVolt = volt;
        }

        gSysMainInfo_51[eSysMainInfo51_2SumV] = (u16)nowVolt;
    }

}
/*=============================================================================================
** @Name      : void OutPreVoltFilterDeal(u8 clunum, u16 volt)
** @Input     : clunum:簇编号 volt:采样电压值(0.1V)
** @Output    : void
** @Function  : 对输出端预充电压采样值进行滤波
** @The notes : 电池输出端预充电压采样值处理调用
**===========================================================================================*/
void OutPreVoltFilterDeal(u8 clunum, u16 volt)
{
    u32 nowVolt = 0;
    if(clunum >= 2)
    {
        return;
    }
    if(clunum == 0)
    {
        //变化值在0.3V—10.0V之间
        if((ABS(gSysMainInfo_51[eSysMainInfo51_1PreV], volt) >= 3)
            && (ABS(gSysMainInfo_51[eSysMainInfo51_1PreV], volt) <= 100))
        {
            nowVolt = ((u32)gSysMainInfo_51[eSysMainInfo51_1PreV] * 12 + (u32)volt * 20) / 32;
        }
        else
        {
            nowVolt = volt;
        }
        
        gSysMainInfo_51[eSysMainInfo51_1PreV] = (u16)nowVolt;
    }
    else
    {
        //变化值在0.3V—10.0V之间
        if((ABS(gSysMainInfo_51[eSysMainInfo51_2PreV], volt) >= 3)
            && (ABS(gSysMainInfo_51[eSysMainInfo51_2PreV], volt) <= 100))
        {
            nowVolt = ((u32)gSysMainInfo_51[eSysMainInfo51_2PreV] * 12 + (u32)volt * 20) / 32;
        }
        else
        {
            nowVolt = volt;
        }
            
        gSysMainInfo_51[eSysMainInfo51_2PreV] = (u16)nowVolt;
    }
}

/*=============================================================================================
** @Name      : s16 GetGroupAvgCurrAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组平均电流值(单位:0.1A)
** @Function  : 获取电池组平均电流值
** @The notes :
**===========================================================================================*/
s16 GetGroupAvgCurrAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sClusAvgCurr[clunum]);
}

