/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagMaker.c
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 电池故障诊断模块启动滤波接口
** @Instructions : 
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "DiagInterface.h"
#include "DiagFilter.h"
#include "DiagMaker.h"
#include "DiagCheck.h"
#include "DiagUser.h"
#include "CurrSample.h"
#include "GD25Q32.h"
#include "GroupPara.h"
#include "EnerInfoCalc.h"
#include "GroupProtect.h"
#include "GroupTactic.h"
#include "TacUser.h"
#include "InsuCheck.h"
#include "PortCheck.h"
#include <stdio.h>
#include <debug.h>
#include <cmsis_os2.h>
extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/
/*=================================================================================================
** @Name      : void DiagVoltLimFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电压限制类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagVoltLimFilMakerTask(void)
{
    u16 maxVolt = 0;
    u16 minVolt = 0;
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            break;
        }
        /*单体电压异常不检测高低压*/
        if(1 == DiagGetCellVoltExpStateHOOK(clunum))
        {
            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_CEL_VH1))
            {
                /*清除单体高压1级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CEL_VH1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_CEL_VH2))
            {
                /*清除单体高压2级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CEL_VH2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_CEL_VL1))
            {
                /*清除单体低压1级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CEL_VL1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_CEL_VL2))
            {
                /*清除单体低压2级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CEL_VL2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_CV_DIF1))
            {
                /*清除单体压差过大1级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CV_DIF1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_CV_DIF2))
            {
                /*清除单体压差过大2级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CV_DIF2, eErr_A);
            }
        }
        else
        {
            /*组单体最大电压/最小电压*/
            maxVolt = GetGCellMaxVoltAPI(clunum);
            minVolt = GetGCellMinVoltAPI(clunum);

            /*判断单体电压高2级-ID0*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_CEL_VH1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CEL_VH2, eErr_B);
            }
            /*到达单体电压高2级*/
            else if(maxVolt >= gGBmuGenPara_102[eBmuGenPara102_VH2Lim])
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(maxVolt <= (gGBmuGenPara_102[eBmuGenPara102_VH2Lim] - gGBmuGenPara_102[eBmuGenPara102_CVH2Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_CEL_VH2))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH2, eErr_A);
            }

            /*判断单体电压高1级-ID16*/
            if(maxVolt >= gGBmuGenPara_102[eBmuGenPara102_VH1Lim])
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH2, eErr_B);
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(maxVolt <= (gGBmuGenPara_102[eBmuGenPara102_VH1Lim] - gGBmuGenPara_102[eBmuGenPara102_CVH1Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_CEL_VH1))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VH1, eErr_A);
            }

            /*判断单体电压低2级-ID1*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_CEL_VL1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CEL_VL2, eErr_B);
            }
            /*达到单体电压低2级*/
            else if(minVolt <= gGBmuGenPara_102[eBmuGenPara102_VL2Lim])
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(minVolt >= (gGBmuGenPara_102[eBmuGenPara102_VL2Lim] + gGBmuGenPara_102[eBmuGenPara102_CVL2Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_CEL_VL2))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL2, eErr_A);
            }

            /*判断单体电压低1级-ID17*/
            if(minVolt <= gGBmuGenPara_102[eBmuGenPara102_VL1Lim])
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL2, eErr_B);
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(minVolt >= (gGBmuGenPara_102[eBmuGenPara102_VL1Lim] + gGBmuGenPara_102[eBmuGenPara102_CVL1Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_CEL_VL1))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CEL_VL1, eErr_A);
            }

            /*单体电压一致下降2级-ID12*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_CV_DIF1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_CV_DIF2, eErr_B);
            }
            /*达到单体电压一致下降2级*/
            else if(ABS(maxVolt, minVolt) >= DiagGetCellVDiffSubVHOOK(clunum, gGBmuGenPara_102[eBmuGenPara102_VDiff2SubV]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF2, eErr_B);
            }
            /*在回差点内则清除故障标志*/
            else if(ABS(maxVolt, minVolt) <= (gGBmuGenPara_102[eBmuGenPara102_VDiff2SubV] - 20))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_CV_DIF2))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF2, eErr_A);
            }

            /*单体电压一致下降1级-ID28*/
            if(ABS(maxVolt, minVolt) >= DiagGetCellVDiffSubVHOOK(clunum, gGBmuGenPara_102[eBmuGenPara102_VDiff1SubV]))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF2, eErr_B);
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF1, eErr_B);
            }
            /*在回差点内则清除故障标志*/
            else if(ABS(maxVolt, minVolt) <= gGBmuGenPara_102[eBmuGenPara102_VDiff2SubV])
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_CV_DIF1))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_CV_DIF1, eErr_A);
            }
        }

        /*总压异常不检测总压高低压*/
        if((eErr_B == DiagGetErrFilStateAPI(clunum,eERR_ID_HSAMP_EXP))
            || (eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_HSAMP_EXP))
            || (GetGSampSumVoltAPI(clunum) < gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV]))
        {
            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_SUM_VH1))
            {
                /*清除总压高压1级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_SUM_VH1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_SUM_VH2))
            {
                /*清除总压高压2级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_SUM_VH2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_SUM_VL1))
            {
                /*清除总压高压1级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_SUM_VL1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_SUM_VL2))
            {
                /*清除总压高压1级*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_SUM_VL2, eErr_A);
            }
        }
        else
        {
            /*判断总压高2级-ID2*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_SUM_VH1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_SUM_VH2, eErr_B);
            }
            /*达到总压高2级*/
            else if(GetGSampSumVoltAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_SumVH2Lim])
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampSumVoltAPI(clunum) <= (gGBmuGenPara_102[eBmuGenPara102_SumVH2Lim] - gGBmuGenPara_102[eBmuGenPara102_SVH2Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_SUM_VH2))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH2, eErr_A);
            }

            /*判断总压高1级-ID18*/
            if(GetGSampSumVoltAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_SumVH1Lim])
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH2, eErr_B);
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampSumVoltAPI(clunum) <= (gGBmuGenPara_102[eBmuGenPara102_SumVH1Lim] - gGBmuGenPara_102[eBmuGenPara102_SVH1Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_SUM_VH1))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VH1, eErr_A);
            }

            /*判断总压低2级-ID3*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum,eERR_ID_SUM_VL1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum,eERR_ID_SUM_VL2, eErr_B);
            }
            /*达到总压低2级*/
            else if(GetGSampSumVoltAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_SumVL2Lim])
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampSumVoltAPI(clunum) >= (gGBmuGenPara_102[eBmuGenPara102_SumVL2Lim] + gGBmuGenPara_102[eBmuGenPara102_SVL2Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_SUM_VL2))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL2, eErr_A);
            }

            /*判断总压低1级-ID19*/
            if(GetGSampSumVoltAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_SumVL1Lim])
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL2, eErr_B);
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampSumVoltAPI(clunum) >= (gGBmuGenPara_102[eBmuGenPara102_SumVL1Lim] + gGBmuGenPara_102[eBmuGenPara102_SVL1Clr]))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_SUM_VL1))
            {
                DiagSetErrNowStateAPI(clunum,eERR_ID_SUM_VL1, eErr_A);
            }
        }
        /*绝缘监测异常不检测绝缘告警*/
        if(eInsuErr_Exp == InsuGetErrState(clunum))
        {
    	    /*产生绝缘采样异常告警-ID71*/
		    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK_EXP, eErr_B);

    	    /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_LEAK1))
            {
                /*清除绝缘1级*/
        	    DiagSetErrFilStateAPI(clunum,eERR_ID_LEAK1, eErr_A);
            }

    	    /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum,eERR_ID_LEAK2))
            {
                /*清除绝缘2级*/
        	    DiagSetErrFilStateAPI(clunum,eERR_ID_LEAK2, eErr_A);
            }
        }
        else
        {
    	    /*消除绝缘采样异常告警-ID71*/
		    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK_EXP, eErr_A);

		    /*判断绝缘1级-ID27*/
		    if(eInsuErr_Lev1 == InsuGetErrState(clunum))
		    {
			    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK2, eErr_B);
			    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK1, eErr_B);
		    }
		    /*判断绝缘2级-ID11*/
		    else if(eInsuErr_Lev2 == InsuGetErrState(clunum))
		    {
			    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK2, eErr_B);
			    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK1, eErr_A);
		    }
		    /*绝缘正常*/
		    else
		    {
			    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK1, eErr_A);

			    /*一级消除二级才能消除*/
			    if(eErr_A == DiagGetErrLastStateAPI(clunum,eERR_ID_LEAK1))
			    {
				    DiagSetErrNowStateAPI(clunum,eERR_ID_LEAK2, eErr_A);
			    }
		    }
        }
    }
}
/*=================================================================================================
** @Name      : void DiagTempLimFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 温度限制类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagTempLimFilMakerTask(void)
{
    s16 maxTemp = 0;
    s16 minTemp = 0;

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            break;
        }
        /*单体温度异常不检测高低温*/
        if(1 == DiagGetCellTempExpStateHOOK(clunum))
        {
            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_TH1))
            {
                /*清除充电高温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_TH1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_TH2))
            {
                /*清除充电高温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_TH2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_TH1))
            {
                /*清除放电高温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_TH1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_TH2))
            {
                /*清除放电高温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_TH2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_TL1))
            {
                /*清除充电低温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_TL1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_TL2))
            {
                /*清除充电低温2级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_TL2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_TL1))
            {
                /*清除放电低温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_TL1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_TL2))
            {
                /*清除放电低温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_TL2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CT_DIF1))
            {
                /*清除单体温差过大1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CT_DIF1, eErr_A);
            }
        }
        else
        {
            /*组单体最大温度/最小温度*/
            maxTemp = GetGCellMaxTempAPI(clunum);
            minTemp = GetGCellMinTempAPI(clunum);

            /*判断充电高温2级-ID4*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_TH1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_TH2, eErr_B);
            }
            /*达到充电高温2级*/
            else if(maxTemp >= (gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim] * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(maxTemp <= ((s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH2Lim] - gGBmuGenPara_102[eBmuGenPara102_CTH2Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_TH2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH2, eErr_A);
            }

            /*判断充电高温1级-ID20*/
            if(maxTemp >= (gGBmuGenPara_102[eBmuGenPara102_ChgTH1Lim] * 10))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(maxTemp <= ((s16)gGBmuGenPara_102[eBmuGenPara102_ChgTH1Lim] - gGBmuGenPara_102[eBmuGenPara102_CTH1Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_TH1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TH1, eErr_A);
            }

            /*判断放电高温2级-ID5*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_TH1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_TH2, eErr_B);
            }
            /*达到放电高温2级*/
            else if(maxTemp >= (gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim] * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(maxTemp <= ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH2Lim] - gGBmuGenPara_102[eBmuGenPara102_DTH2Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_TH2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH2, eErr_A);
            }

            /*判断放电高温1级-ID21*/
            if(maxTemp >= (gGBmuGenPara_102[eBmuGenPara102_DhgTH1Lim] * 10))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(maxTemp <= ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgTH1Lim] - gGBmuGenPara_102[eBmuGenPara102_DTH1Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_TH1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TH1, eErr_A);
            }

            /*判断充电低温2级-ID6*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_TL1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_TL2, eErr_B);
            }
            /*达到充电低温2级*/
            else if(minTemp <= (s16)(gGBmuGenPara_102[eBmuGenPara102_ChgTL2Lim] * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(minTemp >= ((s16)gGBmuGenPara_102[eBmuGenPara102_ChgTL2Lim] + gGBmuGenPara_102[eBmuGenPara102_CTL2Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_TL2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL2, eErr_A);
            }

            /*判断充电低温1级-ID22*/
            if(minTemp <= (s16)(gGBmuGenPara_102[eBmuGenPara102_ChgTL1Lim] * 10))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(minTemp >= ((s16)gGBmuGenPara_102[eBmuGenPara102_ChgTL1Lim] + gGBmuGenPara_102[eBmuGenPara102_CTL1Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_TL1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_TL1, eErr_A);
            }

            /*判断放电低温2级-ID7*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_TL1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_TL2, eErr_B);
            }
            /*达到放电低温2级*/
            else if(minTemp <= (s16)(gGBmuGenPara_102[eBmuGenPara102_DhgTL2Lim] * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL2, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(minTemp >= ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgTL2Lim] + gGBmuGenPara_102[eBmuGenPara102_DTL2Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_TL2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL2, eErr_A);
            }

            /*判断放电低温1级-ID23*/
            if(minTemp <= (s16)(gGBmuGenPara_102[eBmuGenPara102_DhgTL1Lim] * 10))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL1, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(minTemp >= ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgTL1Lim] + gGBmuGenPara_102[eBmuGenPara102_DTL1Clr]) * 10)
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_TL1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_TL1, eErr_A);
            }

            /*单体温度温差过大2级-ID13*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CT_DIF1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CT_DIF2, eErr_B);
            }
            /*达到单体温度温差过大2级*/
            else if(ABS(maxTemp, minTemp) >= (gGBmuGenPara_102[eBmuGenPara102_TDiff2SubT] * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF2, eErr_B);
            }
            /*在回差点内则清除故障标志*/
            else if(ABS(maxTemp, minTemp) <= ((gGBmuGenPara_102[eBmuGenPara102_TDiff2SubT] - 3) * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CT_DIF2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF2, eErr_A);
            }

            /*单体温度温差过大1级-ID29*/
            if(ABS(maxTemp, minTemp) >= (gGBmuGenPara_102[eBmuGenPara102_TDiff1SubT] * 10))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF1, eErr_B);
            }
            /*在回差点内则清除故障标志*/
            else if(ABS(maxTemp, minTemp) <= ((gGBmuGenPara_102[eBmuGenPara102_TDiff1SubT] - 3) * 10))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CT_DIF1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CT_DIF1, eErr_A);
            }
        }

        /*外设温度异常不检测外设过温*/
        if(((eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_TSAMP_EXP))
            || (eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_TSAMP_EXP)))
        /*使能IMEU2通信交互*/
        #if(1 == BMU_IMEU_COM_EN)
        /*IMEU2通信中断*/
            && (1 == DiagGetPerImeuOffCommuStateAPI())
        #endif
            )
        {
            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_PER_TH1))
            {
                /*清除外围高温1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_PER_TH1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_PER_TH2))
            {
                /*清除外围高温2级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_PER_TH2, eErr_A);
            }
        }
        else
        {
            /*判断外围高温2级(环境温度或分流器温度)-ID10*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_PER_TH1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_PER_TH2, eErr_B);
            }
            /*达到外围高温2级(环境温度或分流器温度)*/
            else if((eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_TSAMP_EXP))
                && ((GetGEnvirTempAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_Envir2Lim])
                || (GetGShuntSampTempAPI(clunum) >= (s16)(gGBmuGenPara_102[eBmuGenPara102_Perip2Lim] * 10))))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_PER_TH2, eErr_B);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_PER_TH2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_PER_TH2, eErr_A);
            }

            /*判断外围高温1级(环境温度或分流器温度)-ID26*/
            if((eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_TSAMP_EXP))
                && ((GetGEnvirTempAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_Envir1Lim])
                || (GetGShuntSampTempAPI(clunum) >= (s16)(gGBmuGenPara_102[eBmuGenPara102_Perip1Lim] * 10))))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_PER_TH2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_PER_TH1, eErr_B);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_PER_TH1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_PER_TH1, eErr_A);
            }

            /*836无均衡板温、低压电源板温采样，保留协议位号但禁止以电芯温度或stub值代替。*/
            DiagSetErrNowStateAPI(clunum, eERR_ID_BAL_TH, eErr_A);
            DiagSetErrNowStateAPI(clunum, eERR_ID_DCV_TH, eErr_A);
        }
    }
}
/*=================================================================================================
** @Name      : void DiagCurrLimFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电流限制类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagCurrLimFilMakerTask(void)
{
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            break;
        }
        /*电流采样异常不检测电流相关告警*/
        if((eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_CSAMP_ERR))
            || (eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CSAMP_ERR)))
        {
            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_CO1))
            {
                /*清除充电过流1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_CO1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_CO2))
            {
                /*清除充电过流2级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_CO2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_CO1))
            {
                /*清除放电过流1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_CO1, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_CO2))
            {
                /*清除放电过流1级*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_CO2, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_DHG_COT))
            {
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_COT, eErr_A);
            }

            /*处于过渡阶段需要恢复正常*/
            if(eErr_AtoB == DiagGetErrFilStateAPI(clunum, eERR_ID_CHG_COT))
            {
                /*清除充电限流响应超时*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_COT, eErr_A);
            }
        }
        else
        {
            /*判断充电过流2级-ID8*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum,  eERR_ID_CHG_CO1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_CO2, eErr_B);
            }
            /*达到充电过流2级*/
            else if(GetGSampOutCurrAPI(clunum) <= (0 - (s16)gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim]))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO2, eErr_B);

                /*达到充电过流极限值则立即产生故障*/
                if(GetGSampOutCurrAPI(clunum) <= (0 - (s16)BATT_EXEREM_MAXC))
                {
                    DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_CO2, eErr_B);
                }
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampOutCurrAPI(clunum) >= (0 - (s16)gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim] + (s16)gGBmuGenPara_102[eBmuGenPara102_ChgCH2Clr]))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_CO2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO2, eErr_A);
            }

            /*判断充电过流1级-ID24*/
            if(GetGSampOutCurrAPI(clunum) <= (0 - (s16)gGBmuGenPara_102[eBmuGenPara102_ChgCH1Lim]))
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO1, eErr_B);

                /*达到充电过流极限值则立即产生故障*/
                if(GetGSampOutCurrAPI(clunum) <= (0 - (s16)BATT_EXEREM_MAXC))
                {
                    DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_CO2, eErr_B);
                    DiagSetErrFilStateAPI(clunum, eERR_ID_CHG_CO1, eErr_B);
                }
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampOutCurrAPI(clunum) >= (0 - (s16)gGBmuGenPara_102[eBmuGenPara102_ChgCH1Lim] + (s16)gGBmuGenPara_102[eBmuGenPara102_ChgCH1Clr]))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_CO1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_CO1, eErr_A);
            }

            /*判断放电过流2级-ID9*/
            if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_CO1))
            {
                /*一级已经发生(未消除)确保二级也发生(不消除)*/
                DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_CO2, eErr_B);
            }
            /*达到放电过流2级*/
            else if(GetGSampOutCurrAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim])
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO2, eErr_B);

                /*达到放电过流极限值则立即产生故障*/
                if(GetGSampOutCurrAPI(clunum) >= BATT_EXEREM_MAXC)
                {
                    DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_CO2, eErr_B);
                }
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampOutCurrAPI(clunum) <= ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim] - gGBmuGenPara_102[eBmuGenPara102_DhgCH2Clr]))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO2, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_CO2))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO2, eErr_A);
            }

            /*判断放电过流1级-ID25*/
            if(GetGSampOutCurrAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_DhgCH1Lim])
            {
                /*一级产生确保二级也要产生*/
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO2, eErr_B);
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO1, eErr_B);

                /*达到放电过流极限值则立即产生故障*/
                if(GetGSampOutCurrAPI(clunum) >= BATT_EXEREM_MAXC)
                {
                    DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_CO2, eErr_B);
                    DiagSetErrFilStateAPI(clunum, eERR_ID_DHG_CO1, eErr_B);
                }
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampOutCurrAPI(clunum) <= ((s16)gGBmuGenPara_102[eBmuGenPara102_DhgCH1Lim] - gGBmuGenPara_102[eBmuGenPara102_DhgCH1Clr]))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO1, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_CO1))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_CO1, eErr_A);
            }

            /*判断充电电流响应超时-ID34*/
            if(GetGChgLimCurrAPI(clunum) <= 50)
            {
                /*请求小于5A而输入大于10A*/
                if(GetGSampOutCurrAPI(clunum) <= (-100))
                {
                    DiagSetErrNowStateAPI(clunum,eERR_ID_CHG_COT, eErr_B);
                }
                else
                {
                    DiagSetErrNowStateAPI(clunum,eERR_ID_CHG_COT, eErr_A);
                }
            }
            /*判断充电电流响应超时(允许误差5A)*/
            else if(GetGSampOutCurrAPI(clunum) <= (0 - (s16)GetGChgLimCurrAPI(clunum) - 50))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_COT, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampOutCurrAPI(clunum) >= (0 - (s16)GetGChgLimCurrAPI(clunum)))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_COT, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CHG_COT))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_CHG_COT, eErr_A);
            }

            /*判断放电电流响应超时-ID35*/
            if(GetGDhgLimCurrAPI(clunum) <= 50)
            {
                /*请求小于5A而输入大于10A*/
                if(GetGSampOutCurrAPI(clunum) >= 100)
                {
                    DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_COT, eErr_B);
                }
                else
                {
                    DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_COT, eErr_A);
                }
            }
            /*判断充电电流响应超时(允许误差5A)*/
            else if(GetGSampOutCurrAPI(clunum) >= (GetGDhgLimCurrAPI(clunum) + 50))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_COT, eErr_B);
            }
            /*在回差点内则清除告警标志*/
            else if(GetGSampOutCurrAPI(clunum) <= GetGDhgLimCurrAPI(clunum))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_COT, eErr_A);
            }
            /*在回差区间未产生告警则清除告警标志*/
            else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_DHG_COT))
            {
                DiagSetErrNowStateAPI(clunum, eERR_ID_DHG_COT, eErr_A);
            }
        }

        /*判断SOC过高2级-ID14*/
        if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CAP_SH1))
        {
            /*一级已经发生(未消除)确保二级也发生(不消除)*/
            DiagSetErrFilStateAPI(clunum, eERR_ID_CAP_SH2, eErr_B);
        }
        /*达到SOC过高2级*/
        else if(GetGBattSocAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_CapHig2Soc])
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH2, eErr_B);
        }
        /*在回差点内则清除告警标志*/
        else if((GetGBattSocAPI(clunum) <= (gGBmuGenPara_102[eBmuGenPara102_CapHig2Soc] - 50))
            && (GetGBattSocAPI(clunum) <= (gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc] - gGBmuGenPara_102[eBmuGenPara102_ChgLimSubSoc])))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH2, eErr_A);
        }
        /*在回差区间未产生告警则清除告警标志*/
        else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CAP_SH2))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH2, eErr_A);
        }

        /*判断SOC过高1级-ID30*/
        if((GetGBattSocAPI(clunum) >= gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc])
            && (1000 != gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc]))
        {
            /*一级产生确保二级也要产生*/
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH2, eErr_B);
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH1, eErr_B);
        }
        /*在回差点内则清除告警标志*/
        else if(GetGBattSocAPI(clunum) <= (gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc] - gGBmuGenPara_102[eBmuGenPara102_ChgLimSubSoc]))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH1, eErr_A);
        }
        /*在回差区间未产生告警则清除告警标志*/
        else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CAP_SH1))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SH1, eErr_A);
        }

        /*判断SOC过低2级-ID15*/
        if(eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_CAP_SL1))
        {
            /*一级已经发生(未消除)确保二级也发生(不消除)*/
            DiagSetErrFilStateAPI(clunum, eERR_ID_CAP_SL2, eErr_B);
        }
        /*达到SOC过低2级*/
        else if(GetGBattSocAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_CapLow2Soc])
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SL2, eErr_B);
        }
        /*在回差点内则清除告警标志*/
        else if((GetGBattSocAPI(clunum) >= (gGBmuGenPara_102[eBmuGenPara102_CapLow2Soc] + 50))
            && (GetGBattSocAPI(clunum) >= (gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc] + gGBmuGenPara_102[eBmuGenPara102_DhgLimSubSoc])))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SL2, eErr_A);
        }
        /*在回差区间未产生告警则清除告警标志*/
        else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CAP_SL2))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SL2, eErr_A);
        }

        /*判断SOC过低1级-ID31*/
        if((GetGBattSocAPI(clunum) <= gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc])
            && (0 != gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc]))
        {
            /*一级产生确保二级也要产生*/
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SL2, eErr_B);
            DiagSetErrNowStateAPI(clunum,eERR_ID_CAP_SL1, eErr_B);
        }
        /*在回差点内则清除告警标志*/
        else if(GetGBattSocAPI(clunum) >= (gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc] + gGBmuGenPara_102[eBmuGenPara102_DhgLimSubSoc]))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SL1, eErr_A);
        }
        /*在回差区间未产生告警则清除告警标志*/
        else if(eErr_A == DiagGetErrLastStateAPI(clunum, eERR_ID_CAP_SL1))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_SL1, eErr_A);
        }
    }
}
/*=================================================================================================
** @Name      : void DiagPortExpFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 输入输出端口异常类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagPortExpFilMakerTask(void)
{
    u8 ctrlState = 0;
    u8 backState = 0;
    u8 maiErr = 0;                             /*主正接触器烧结/不能吸合聚合标志*/
    u8 negErr = 0;                             /*负极接触器烧结/不能吸合聚合标志*/

    /*簇1主正接触器控制状态与回检状态*/
    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK1_1Mai);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai);

    /*控制状态与回检状态不一致*/
    if(ctrlState != backState)
    {
    	DiagSetErrNowStateAPI(0, eERR_ID_PK0RE_EXP, eErr_B);
    }
    else
    {
    	DiagSetErrNowStateAPI(0, eERR_ID_PK0RE_EXP, eErr_A);
    }

    /*簇1负极接触器控制状态与回检状态*/
    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK2_1Neg);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg);

    /*控制状态与回检状态不一致*/
    if(ctrlState != backState)
    {
    	DiagSetErrNowStateAPI(0, eERR_ID_PK1RE_EXP, eErr_B);
    }
    else
    {
    	DiagSetErrNowStateAPI(0, eERR_ID_PK1RE_EXP, eErr_A);
    }

    /*簇2正极接触器控制状态与回检状态*/
    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK3_2Mai);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai);

    /*控制状态与回检状态不一致*/
    if(ctrlState != backState)
    {
    	DiagSetErrNowStateAPI(1, eERR_ID_PK2RE_EXP, eErr_B);
    }
    else
    {
    	DiagSetErrNowStateAPI(1, eERR_ID_PK2RE_EXP, eErr_A);
    }

    /*簇2负极接触器控制状态与回检状态*/
    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK4_2Neg);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg);
    /*控制状态与回检状态不一致*/
    if(ctrlState != backState)
    {
    	DiagSetErrNowStateAPI(1, eERR_ID_PK3RE_EXP, eErr_B);
    }
    else
    {
    	DiagSetErrNowStateAPI(1, eERR_ID_PK3RE_EXP, eErr_A);
    }

    /* TODO: 回检辅助触点常开/常闭极性待上板确认,如极性反转则更改判定条件 */

    /*主正接触器状态异常(烧结或不能吸合): 836回检走BMU本地DI1/DI3(非老平台IMEU3 DI8/9),簇1/簇2任一;
    命令闭合而回检断开=不能吸合,命令断开而回检闭合=烧结粘联;与PK0RE/PK2RE同源,聚合为系统级定性告警*/
    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK1_1Mai);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK1_1Mai);
    maiErr = (u8)(ctrlState != backState);

    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK3_2Mai);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK3_2Mai);
    maiErr |= (u8)(ctrlState != backState);

    /*负极接触器状态异常(烧结或不能吸合): 836回检走BMU本地DI2/DI4,簇1/簇2任一(同上双向判定)*/
    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK2_1Neg);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK2_1Neg);
    negErr = (u8)(ctrlState != backState);

    ctrlState = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK4_2Neg);
    backState = TacGetGroupSwhBackStateAPI(eGroupSwhPK4_2Neg);
    negErr |= (u8)(ctrlState != backState);

    /*系统级聚合条件广播到各簇上下文(滤波表按簇独立)*/
    for(u8 clunum = 0; clunum < (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            break;
        }

        DiagSetErrNowStateAPI(clunum, eERR_ID_K0MAI_EXP, (0 != maiErr) ? eErr_B : eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_K2NEG_EXP, (0 != negErr) ? eErr_B : eErr_A);
    }
}
/*=================================================================================================
** @Name      : void DiagInfoExpFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 数据信息异常类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagInfoExpFilMakerTask(void)
{
    /*均衡状态比对函数带有100ms去抖计数，整个诊断周期只能调用一次。*/
    u32 balErrBit = DiagGetBalErrCellBitAPI();

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            break;
        }
        /*SOC/SOE显示与真实值误差8%*/
        if((ABS(GetGBattSocAPI(clunum), GetGRealSocMilliAPI(clunum)) >= 80)
            || (ABS(GetGBattSoeAPI(clunum), GetGRealSoeMilliAPI(clunum)) >= 80))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SOC_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SOC_EXP, eErr_A);
        }

        /*容量检测异常(检测值与原值误差20%或长时间未检测到容量)*/
        if(GetCapEnerDropExpAPI(clunum) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CAP_EXP, eErr_A);
        }

        /*主控单体采样异常(采样通信无回复)*/
        if(DiagGetCellSampErrFlagAPI(clunum) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SAMP_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SAMP_EXP, eErr_A);
        }

        /*主控单体电压采样断线(包括未插线束)*/
        if(DiagGetCellVOffCellBitAPI(clunum) > 0)
        {
            DiagSetErrNowStateAPI(clunum,eERR_ID_SAMP_VOFF, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum,eERR_ID_SAMP_VOFF, eErr_A);
        }

        /*主控单体温度采样断线(包括未插线束)*/
        if(DiagGetCellTOffCellBitAPI(clunum) > 0)
        {
            DiagSetErrNowStateAPI(clunum,eERR_ID_SAMP_TOFF, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SAMP_TOFF, eErr_A);
        }

        /*主控单体信息缺失(未全部接收单体电压和温度)*/
        if(2 != DiagGetRcvAllCellFlagAPI(clunum))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CELL_MISS, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CELL_MISS, eErr_A);
        }

        /*主控外部EEPROM异常*/
        if(FALSE == DEVEXTEEPGetReadyFlag())
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_MEEP_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_MEEP_EXP, eErr_A);
        }

        /*836无均衡板温采样；保留故障ID160，固定为正常。*/
        DiagSetErrNowStateAPI(clunum, eERR_ID_BAL_TEXP, eErr_A);

        /*均衡控制异常：仅在本簇均衡命令与从控状态回报持续不一致时触发。*/
        if((balErrBit & (1UL << clunum)) > 0U)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_BAL_CEXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_BAL_CEXP, eErr_A);
        }

        /*主控外部Flash异常(仅emmc形态;XIP形态OSPI2为执行介质,编译期固定正常)*/
#if DIAG_MFLA_CHECK_EN
        if(FALSE == DEVEXTFLASHGetReadyFlag())
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_MFLA_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_MFLA_EXP, eErr_A);
        }
#else
        DiagSetErrNowStateAPI(clunum, eERR_ID_MFLA_EXP, eErr_A);
#endif

        /*电流采样电路异常：IMIU4告警字驱动的霍尔/分流器异常，或BMU本板校零/AFE异常。*/
        if(SampGetCurrSampExpFlagAPI(clunum) != 0U)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CSAMP_ERR, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_CSAMP_ERR, eErr_A);
        }

        /*主控低压采样异常：数据源为BMU本板ADC的采样启动/原始值有效性。*/
        if(SampGetLowVSampExpFlagAPI() > 0U)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_LSAMP_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_LSAMP_EXP, eErr_A);
        }

        /*高压采样异常：IMIU4告警信息1 bit4=母线Link采样异常，bit5=电池总压采样异常。*/
        if(((DiagGetRcvPerOffCommuBitAPI() & (1U << (clunum + 1U))) == 0U)
            && ((GetImiu4AlmInfoAPI(clunum, e_IMIUAlmInfo_Alm1) & 0x0030U) != 0U))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_HSAMP_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_HSAMP_EXP, eErr_A);
        }

        /*外围温度采样异常：ShuntT量纲0.1℃；沿用100.37边界，<=-41.0℃或>=182.0℃即异常。*/
        if(((DiagGetRcvPerOffCommuBitAPI() & (1U << (clunum + 1U))) == 0U)
            && ((GetGShuntSampTempAPI(clunum) <= -410) || (GetGShuntSampTempAPI(clunum) >= 1820)))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_TSAMP_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_TSAMP_EXP, eErr_A);
        }
    }
}
/*=================================================================================================
** @Name      : void DiagStateExpFilMakerTask(void)
** @Input     : void
** @Output    : void
** @Function  : 状态信息异常类故障诊断任务
** @The notes : 100ms周期
**===============================================================================================*/
void DiagStateExpFilMakerTask(void)
{
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
        {
            break;
        }
        u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

        /*836未配置下列硬件/回检链路；保留协议位号，运行时强制为正常。*/
        DiagSetErrNowStateAPI(clunum, eERR_ID_AIR_CON, eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_K1PRE_EXP, eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_STARTUP_VL, eErr_A);
        DiagSetErrNowStateAPI(clunum, eERR_ID_STARTUP_TH, eErr_A);

        /*软件急停*/
        if((TacGetGEmergStopFlagAPI(clunum) & 0x03) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SW_EMERG, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SW_EMERG, eErr_A);
        }
        /*按钮紧急停机*/
        if((1 == GetGEmergIoButtonStateAPI()) || ((TacGetGEmergStopFlagAPI(clunum) & 0x04) > 0))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_IO_EMERG, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_IO_EMERG, eErr_A);
        }
        /*电池组自行保护*/
        if(TacGetGHappenProFlagAPI(clunum) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SELF_PRO, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SELF_PRO, eErr_A);
        }
        #if(0)
        SOS后续再
        /*SOS诊断异常(已显示SOS异常原因)*/
        if(DiagGetGroupSosLevel() > 4)
        {
            DiagSetErrNowStateAPI(eERR_ID_SOS_EXP, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(eERR_ID_SOS_EXP, eErr_A);
        }
        #endif
        if(2 == TacGetGSmogAlarmFlagAPI())
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_BATT_SMOG, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_BATT_SMOG, eErr_A);
        }
        /*主控启动故障失败*/
        if((TacGetGStartFailFlagAPI(clunum) & 0x0f) > 0)
        {
            /*历史故障记录告警信息产生*/
            //if(eErr_A == DiagGetErrFilStateAPI(eERR_ID_STARTUP_ERR))
            //{
                //DiagRcdHisErrInfoRecordAPI(eERR_ID_STARTUP_ERR, eERR_RCDSORT_WARSET);
            //}

            DiagSetErrNowStateAPI(clunum, eERR_ID_STARTUP_ERR, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_STARTUP_ERR, eErr_A);
        }

        /*主控停机带载超时*/
        if(1 == BitGet(TacGetGStopFailFlagAPI(clunum), 1))
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_STOP_CH, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_STOP_CH, eErr_A);
        }

        /*主控停机故障失败*/
        if((TacGetGStopFailFlagAPI(clunum) & 0x0d) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_STOP_ERR, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_STOP_ERR, eErr_A);
        }

        /*高压端内外压差过大*/
        if(DiagGetHigVSubDiffFlagAPI(clunum) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_DC_HV_DIFF, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_DC_HV_DIFF, eErr_A);
        }

        /*接触器带载闭合*/
        if((DiagGetSwhChangCExpFlagAPI(clunum) & 0xf0) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SWHON_CH, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SWHON_CH, eErr_A);
        }

        /*接触器带载断开*/
        if((DiagGetSwhChangCExpFlagAPI(clunum) & 0x0f) > 0)
        {
            DiagSetErrNowStateAPI(clunum, eERR_ID_SWHOFF_CH, eErr_B);
        }
        else
        {
            DiagSetErrNowStateAPI(clunum,eERR_ID_SWHOFF_CH, eErr_A);
        }
        //使用自研BSU
        #if((1 == BMU_BSU_COM_TYPE) && (0 == BMU_BSU_CAN_TYPE))
        /*从控通信正常*/
        if(eErr_A == DiagGetErrFilStateAPI(clunum, eERR_ID_INT_COFF))
        {
                for(u8 packId = 0; packId < cfgBsuNum; packId++)
                {
                    u32 ecu = packId + clunum * cfgBsuNum;
                    u32 bit = (u32)1 << ecu;

                    if((DiagGetBsu02u1FloodErrorBitAPI() & bit) > 0)
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7), eErr_B);
                    }
                    else
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7), eErr_A);
                    }

                    if((DiagGetBsu02u1TemperErrorBitAPI() & ((u32)1 << (ecu * BSU02U1_PT1000_NUM))) > 0)
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 2), eErr_B);
                    }
                    else
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 2), eErr_A);
                    }

                    if((DiagGetBsu02u1TemperErrorBitAPI() & ((u32)1 << (ecu * BSU02U1_PT1000_NUM + 1))) > 0)
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 3), eErr_B);
                    }
                    else
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 3), eErr_A);
                    }

                    if((DiagGetBsu02u1AeroErrorBitAPI() & bit) > 0)
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 4), eErr_B);
                    }
                    else
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 4), eErr_A);
                    }

                    if((DiagGetBsu02u1MsdErrorBitAPI() & bit) > 0)
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 1), eErr_B);
                    }
                    else
                    {
                        DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 1), eErr_A);
                    }

                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 5), eErr_A);

                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 6), eErr_A);
                }
        }
        /*从控通信中断*/
        else
        {
                for(u8 packId = 0; packId < cfgBsuNum; packId++)
                {
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 0), eErr_A);
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 1), eErr_A);
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 2), eErr_A);
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 3), eErr_A);
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 4), eErr_A);
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 5), eErr_A);
                    DiagSetErrNowStateAPI(clunum, (e_ERR_ID)(eERR_ID_PACK1_WAT + packId * 7 + 6), eErr_A);
                }
        }
        #endif
    }
}
