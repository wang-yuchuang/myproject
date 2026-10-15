/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocDisplay.c
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : SOC平滑显示模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "ParaInterface.h"
#include "CapInfoCalc.h"
#include "CurrIntegral.h"
#include "EnerInterface.h"
#include "SocDisplay.h"
#include <debug.h>
#include <cmsis_os2.h>
#include <string.h>
static u16 sSlowDisplaySoc[2] = {0xffff,0xffff};    /*平滑显示SOC中间计算值(万分之一)*/

extern u16 gGBmuGenPara_102[];		    /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	    /*主控高级参数值*/
extern u16 *gSysCapInfo_52;             /*系统电量信息*/
/*=============================================================================================
** @Name      : void GroupSocDisplayInit(void)
** @Input     : void
** @Output    : void
** @Function  : SOC平滑显示初始化
** @The notes : 用户初始化调用
**===========================================================================================*/
void GroupSocDisplayInit(void)
{
    u16 soc = 0;

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        soc = 0;
        if(clunum >= 2)
        {
            return;
        }
        if(clunum == 0)
        {
            soc = gGBmuGenPara_102[eBmuGenPara102_1NowSoc] * 10;
        }
        else
        {
            soc = gGBmuGenPara_102[eBmuGenPara102_2NowSoc] * 10;
        }

        
        /*上电不显示100%和0%*/
        if(soc > 9940)
        {
            soc = 9940;
        }
        else if(soc < 100)
        {
            soc = 100;
        }

        sSlowDisplaySoc[clunum] = soc;
    }
}
/*=============================================================================================
** @Name      : void GroupSocDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : SOC平滑显示任务
** @The notes : 100ms
**===========================================================================================*/
void GroupSocDisplayTask(void)
{
    u16 aimSoc = 0;
    u16 slowSoc = 0;
    
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        aimSoc = 0;
        slowSoc = 0;
        if(clunum >= 2)
        {
            return;
        }
        aimSoc = GetGRealSocTenThousAPI(clunum);          /*取当前SOC*/
    
        aimSoc = CalcSocChgDhgEndCorr(clunum, aimSoc);      /*充放电末端修正当前SOC*/

        if(0xffff == sSlowDisplaySoc[clunum])               /*平滑值未初始化*/
        {
            slowSoc = aimSoc;
        }
        else
        {
            slowSoc = CalcSocSlowChang(clunum, aimSoc);     /*计算SOC平滑变化值*/
        }
        
        UpdateSocSlowDisplay(clunum, slowSoc);              /*将平滑值更新到显示值*/
    }
}
/*=============================================================================================
** @Name      : void CorrGDisplaySocByUser(u8 clunum, u16 soc)
** @Input     : clunum:簇编号 soc:目标SOC(0.01%)
** @Output    : void
** @Function  : 用户修正当前显示SOC
** @The notes :
**===========================================================================================*/
void CorrGDisplaySocByUser(u8 clunum, u16 soc)
{
    if(soc > 10000)
    {
        return;
    }
    if(clunum >= 2)
    {
        return;
    }
    
    sSlowDisplaySoc[clunum] = soc;
    
    UpdateSocSlowDisplay(clunum, soc);
}
/*=============================================================================================
** @Name      : static u16 CalcSocChgDhgEndCorr(u8 clunum, u16 nowSoc)
** @Input     : clunum:簇编号 aimSoc:当前SOC(万分之一)
** @Output    : 修正后的SOC值(万分之一)
** @Function  : 计算SOC充放电末端修正值
** @The notes :
**===========================================================================================*/
static u16 CalcSocChgDhgEndCorr(u8 clunum, u16 nowSoc)
{
	u16 aimSoc = nowSoc;
    static u8 sCorrFlag[2] = {0};
    static u32 sIdleTime[2] = {0};

    if(clunum >= 2)
    {
        return (0);
    }

    if(0 == EnerGetGroupBattReadyHook(clunum))                    /*单体信息异常未就绪*/
	{
		/*不执行末端修正*/
		sCorrFlag[clunum] = 0;
		sIdleTime[clunum] = 0;
		aimSoc = nowSoc;                                    /*返回原值*/
	}
	else if(EnerGetGroupVHProStateHook(clunum) > 0)               /*高压一级保护*/
    {
    	/*SOC未达到目标值且未执行修正*/
    	if((1 != sCorrFlag[clunum])
    		&& ((nowSoc < SOC_SLOW_VHER_SOC)
    		|| (sSlowDisplaySoc[clunum] < SOC_SLOW_VHER_SOC)))
    	{
    		CorrGRemainCapBySocAPI(clunum, SOC_SLOW_VHER_SOC / 10); /*强制修正为100%*/
    		CorrGNowCapBySocAPI(clunum, SOC_SLOW_VHER_SOC / 10);    /*强制修正为100%*/
    		sSlowDisplaySoc[clunum] = SOC_SLOW_VHER_SOC;            /*强制显示为100%*/
    		aimSoc = SOC_SLOW_VHER_SOC;                     /*记录显示目标100%*/
    		sCorrFlag[clunum] = 1;                                  /*记录已修正显示*/
    	}
    	else
    	{
    		aimSoc = nowSoc;                               /*记录显示当前值*/
    	}
    	sIdleTime[clunum] = 0;
    }
    else if(EnerGetGroupVLProStateHook(clunum) > 0)               /*低压一级保护*/
    {
    	/*SOC未达到目标值且未执行修正*/
    	if((2 != sCorrFlag[clunum])
    		&& ((nowSoc > SOC_SLOW_VLER_SOC)
    		|| (sSlowDisplaySoc[clunum] > SOC_SLOW_VLER_SOC)))
    	{
    		CorrGRemainCapBySocAPI(clunum, SOC_SLOW_VLER_SOC / 10); /*强制修正为0%*/
    		CorrGNowCapBySocAPI(clunum, SOC_SLOW_VLER_SOC / 10);    /*强制修正为0%*/
    		sSlowDisplaySoc[clunum] = SOC_SLOW_VLER_SOC;            /*强制显示为0%*/
    		aimSoc = SOC_SLOW_VLER_SOC;                     /*记录显示目标0%*/
    		sCorrFlag[clunum] = 2;                                  /*记录已修正显示*/
    	}
    	else
    	{
    		aimSoc = nowSoc;                               /*记录显示当前值*/
    	}
    	sIdleTime[clunum] = 0;
    }
    else if(EnerGetGroupVHLimStateHook(clunum) > 0)               /*高压二级保护*/
    {
    	/*SOC未达到目标值且未执行修正*/
    	if((1 != sCorrFlag[clunum]) && (3 != sCorrFlag[clunum])
    		&& ((nowSoc < SOC_SLOW_VH_SOC)
    		|| (sSlowDisplaySoc[clunum] < SOC_SLOW_VH_SOC)))
    	{
    		CorrGNowCapBySocAPI(clunum, SOC_SLOW_VH_SOC / 10);      /*强制修正为100%*/
    		aimSoc = SOC_SLOW_VH_SOC;                       /*记录显示目标100%*/
    		sCorrFlag[clunum] = 3;                                  /*记录已修正显示*/

    		if(GetGSampOutCurrAPI(clunum) < -5)                   /*正在充电*/
    		{
    			sSlowDisplaySoc[clunum] = SOC_SLOW_VH_SOC;          /*强制显示为100%*/
    		}
    	}
    	else
    	{
    		aimSoc = nowSoc;                               /*记录显示当前值*/
    	}
    	sIdleTime[clunum] = 0;
    }
    else if(EnerGetGroupVLLimStateHook(clunum) > 0)               /*低压二级保护*/
    {
    	/*SOC未达到目标值且未执行修正*/
    	if((2 != sCorrFlag[clunum]) && (4 != sCorrFlag[clunum])
    		&& ((nowSoc > SOC_SLOW_VL_SOC)
    		|| (sSlowDisplaySoc[clunum] > SOC_SLOW_VL_SOC)))
    	{
    		CorrGNowCapBySocAPI(clunum, SOC_SLOW_VL_SOC / 10);      /*强制修正为3%*/
    		aimSoc = SOC_SLOW_VL_SOC;                       /*记录显示目标3%*/
    		sCorrFlag[clunum] = 4;                                  /*记录已修正显示*/

    		if(GetGSampOutCurrAPI(clunum) > 5)                    /*正在放电*/
    		{
    			sSlowDisplaySoc[clunum] = SOC_SLOW_VL_SOC;          /*强制显示为3%*/
    		}
    	}
    	else
    	{
    		aimSoc = nowSoc;                                /*记录显示当前值*/
    	}
    	sIdleTime[clunum] = 0;
    }
    else
    {
		sCorrFlag[clunum] = 0;                                      /*清除已修正显示*/

    	/*系统待机时修正SOC可信度*/
		if(eCURR_IDLE == GetGChgDhgStateAPI(clunum))
		{
	    	/*达到静置时长*/
            if(sIdleTime[clunum] >= SOC_SLOW_CORR_IDLE_T)
            {
            	/*电压很低处于下区间*/
                if(GetGCellMaxVoltAPI(clunum) < SOC_V_SEC_DN_MAX_V)
                {
                	/*当前SOC过高*/
                    if(nowSoc > SOC_V_SEC_DN_MOS_SOC)
                    {
                        /*修正当前SOC为至大可能值*/
                		CorrGNowCapBySocAPI(clunum, SOC_V_SEC_DN_MOS_SOC / 10);
                		aimSoc = SOC_V_SEC_DN_MOS_SOC;
                    }
                	/*显示SOC过高*/
                    else if(sSlowDisplaySoc[clunum] > SOC_V_SEC_DN_MOS_SOC)
                    {
                        /*修正当前SOC为至大可能值*/
                    	sSlowDisplaySoc[clunum] = SOC_V_SEC_DN_MOS_SOC;
                		aimSoc = nowSoc;
                    }
                    else
                    {
                		aimSoc = nowSoc;
                    }
                }
            	/*电压很高处于上区间*/
                else if(GetGCellMinVoltAPI(clunum) > SOC_V_SEC_UP_MIN_V)
                {
                	/*当前SOC过低*/
                    if(nowSoc < SOC_V_SEC_UP_LES_SOC)
                    {
                        /*修正当前SOC为至小可能值*/
                		CorrGNowCapBySocAPI(clunum, SOC_V_SEC_UP_LES_SOC / 10);
                		aimSoc = SOC_V_SEC_UP_LES_SOC;
                    }
                	/*显示SOC过低*/
                    else if(sSlowDisplaySoc[clunum] < SOC_V_SEC_UP_LES_SOC)
                    {
                        /*修正当前SOC为至大可能值*/
                    	sSlowDisplaySoc[clunum] = SOC_V_SEC_UP_LES_SOC;
                		aimSoc = nowSoc;
                    }
                    else
                    {
                		aimSoc = nowSoc;
                    }
                }
            	/*电压稍高高于下区间*/
                else if(GetGCellMinVoltAPI(clunum) > SOC_V_SEC_DN_MIN_V)
                {
                	/*当前SOC过低*/
                    if(nowSoc < SOC_V_SEC_DN_LES_SOC)
                    {
                        /*修正当前SOC为至小可能值*/
                		CorrGNowCapBySocAPI(clunum, SOC_V_SEC_DN_LES_SOC / 10);
                		aimSoc = SOC_V_SEC_DN_LES_SOC;
                    }
                	/*显示SOC过低*/
                    else if(sSlowDisplaySoc[clunum] < SOC_V_SEC_DN_LES_SOC)
                    {
                        /*修正当前SOC为至大可能值*/
                    	sSlowDisplaySoc[clunum] = SOC_V_SEC_DN_LES_SOC;
                		aimSoc = nowSoc;
                    }
                    else
                    {
                		aimSoc = nowSoc;
                    }
                }
            	/*电压稍低低于上区间*/
                else if(GetGCellMaxVoltAPI(clunum) < SOC_V_SEC_UP_MAX_V)
                {
                	/*当前SOC过高*/
                    if(nowSoc > SOC_V_SEC_UP_MOS_SOC)
                    {
                        /*修正当前SOC为至大可能值*/
                		CorrGNowCapBySocAPI(clunum, SOC_V_SEC_UP_MOS_SOC / 10);
                		aimSoc = SOC_V_SEC_UP_MOS_SOC;
                    }
                	/*显示SOC过高*/
                    else if(sSlowDisplaySoc[clunum] > SOC_V_SEC_UP_MOS_SOC)
                    {
                        /*修正当前SOC为至大可能值*/
                    	sSlowDisplaySoc[clunum] = SOC_V_SEC_UP_MOS_SOC;
                		aimSoc = nowSoc;
                    }
                    else
                    {
                		aimSoc = nowSoc;
                    }
                }
                else
                {
            		aimSoc = nowSoc;
                }
            }
            else
            {
            	sIdleTime[clunum]++;
        		aimSoc = nowSoc;
            }
		}
    	else
    	{
        	sIdleTime[clunum] = 0;

        	/*充电末端修正SOC可信度*/
        	if(eCURR_CHG == GetGChgDhgStateAPI(clunum))
        	{
            	/*充电已到达末端电压,当前SOC过小*/
        		if((GetGCellMaxVoltAPI(clunum) >= SOC_V_CHG_UP_MAX_V) && (nowSoc < SOC_V_CHG_UP_LES_SOC))
        		{
                    /*修正为末端至小值*/
            		CorrGNowCapBySocAPI(clunum, SOC_V_CHG_UP_LES_SOC / 10);
        			aimSoc = SOC_V_CHG_UP_LES_SOC;
        		}
            	/*充电只到达起始端电压,当前SOC过小*/
        		else if((GetGCellMaxVoltAPI(clunum) >= SOC_V_CHG_DN_MAX_V) && (nowSoc < SOC_V_CHG_DN_LES_SOC))
        		{
                    /*修正为起始端至小值*/
            		CorrGNowCapBySocAPI(clunum, SOC_V_CHG_DN_LES_SOC / 10);
        			aimSoc = SOC_V_CHG_DN_LES_SOC;
        		}
        		else
        		{
        			/*使用SOC控制截止充电,电压未达到截止电压而SOC提前到达(SOC跑快了)*/
        	    	if((gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc] < (SOC_SLOW_VH_SOC / 10))
        	    		&& (GetGCellMaxVoltAPI(clunum) < gGBmuGenPara_102[eBmuGenPara102_CapHig1Volt])
						&& (nowSoc >= ((gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc] - 6) * 10)))
        	    	{
        	    		/*不能到达截止SOC*/
        	    		aimSoc = (gGBmuGenPara_102[eBmuGenPara102_CapHig1Soc] - 6) * 10;
        	    	}
        			/*未使用SOC控制截止充电*/
        	    	else
        	    	{
        	    		aimSoc = nowSoc;
        	    	}
        		}
        	}
        	/*放电末端修正SOC可信度*/
        	else
        	{
            	/*放电已到达末端电压,当前SOC过大*/
        		if((GetGCellMinVoltAPI(clunum) <= SOC_V_DHG_DN_MAX_V) && (nowSoc > SOC_V_DHG_DN_MOS_SOC))
        		{
                    /*修正为末端至大值*/
            		CorrGNowCapBySocAPI(clunum, SOC_V_DHG_DN_MOS_SOC / 10);
        			aimSoc = SOC_V_DHG_DN_MOS_SOC;
        		}
            	/*放电只到达起始端电压,当前SOC过大*/
        		else if((GetGCellMinVoltAPI(clunum) <= SOC_V_DHG_UP_MAX_V) && (nowSoc > SOC_V_DHG_UP_MOS_SOC))
        		{
                    /*修正为起始端至大值*/
            		CorrGNowCapBySocAPI(clunum, SOC_V_DHG_UP_MOS_SOC / 10);
        			aimSoc = SOC_V_DHG_UP_MOS_SOC;
        		}
        		else
        		{
        			/*使用SOC控制截止放电,电压未达到截止电压而SOC提前到达(SOC跑快了)*/
        	    	if((gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc] > (SOC_SLOW_VL_SOC / 10))
        	    		&& (GetGCellMinVoltAPI(clunum) > gGBmuGenPara_102[eBmuGenPara102_CapLow1Volt])
						&& (nowSoc <= ((gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc] + 10) * 10)))
        	    	{
        	    		/*不能到达截止SOC*/
        	    		aimSoc = (gGBmuGenPara_102[eBmuGenPara102_CapLow1Soc] + 10) * 10;
        	    	}
        			/*未使用SOC控制截止充电*/
        	    	else
        	    	{
        	    		aimSoc = nowSoc;
        	    	}
        		}
        	}
    	}
    }

    return(aimSoc);
}
/*=============================================================================================
** @Name      : static u16 CalcSocSlowChang(u8 clunum, u16 aimSoc)
** @Input     : clunum:簇编号 aimSoc:目标SOC(万分之一)
** @Output    : 平滑后的SOC值(万分之一)
** @Function  : 计算SOC平滑值
** @The notes :
**===========================================================================================*/
static u16 CalcSocSlowChang(u8 clunum, u16 aimSoc)
{
    u16 state = 0;                                  /*充放电状态*/
    s32 inteCap = 0;                                /*积分容量变化量(1mAH)*/
    u32 changCap = 0;                               /*容量变化量(1mAH)*/
    u32 totalCap = 0;                               /*总容量(1mAH)*/
    u16 changSoc = 0;                               /*变化SOC(万分之一)*/
    u16 copySoc = 0;                                /*计算的SOC(万分之一)*/
    u16 stepSoc = 0;                                /*SOC增长步长*/
    static u16 sHisState[2] = {0};                  /*历史充放电状态*/
    static s32 sHisCap[2] = {0};                    /*历史充放电量*/
    static u8 sTime[2] = {0};                       /*修正变化0.01%的时间*/

    if(clunum >= 2)
    {
        return (0);
    }
    
    copySoc = sSlowDisplaySoc[clunum];                      /*备份平滑值*/


    totalCap = GetGroupTotalCapAPI(clunum);               /*总容量*/
    inteCap = GetChgDhgChangCapAPI(clunum);               /*积分容量变化量*/
    
    if(GetGSampOutCurrAPI(clunum) < 0)
    {
        state = 1;                                  /*充电状态*/
    }
    else if(GetGSampOutCurrAPI(clunum) > 0)
    {
        state = 2;                                  /*放电状态*/
    }
    else
    {
        state = 0;
    }
    
    if((1 == state) && (1 == sHisState[clunum]))            /*持续充电(SOC增大)*/
    {
		changCap = ABS(sHisCap[clunum], inteCap);           /*充电负数绝对值增大*/

		changSoc = (u16)(changCap * 10000 / totalCap);/*实际变化的SOC*/

        if(0 == changSoc)                           /*变化过小*/
        {
            inteCap = sHisCap[clunum];                      /*不更新本次容量变化值*/
        }

        if(copySoc < aimSoc)                        /*显示平滑值低于实际目标值*/
        {
        	if(aimSoc >= 9000)                      /*目标SOC大于90%(快要充满)*/
        	{
            	if(aimSoc >= 9750)                  /*目标SOC大于97.5%(接近充满)*/
            	{
            		stepSoc = (aimSoc - copySoc) / 50;/*计算5s内增长到真实值步长*/
            	}
            	else if(aimSoc >= 9500)             /*目标SOC大于95%(快要充满)*/
            	{
            		stepSoc = (aimSoc - copySoc) / 150; /*计算15s内增长到真实值步长*/
            	}
            	else if(aimSoc >= 9300)             /*目标SOC大于93%(快要充满)*/
            	{
            		stepSoc = (aimSoc - copySoc) / 200; /*计算20s内增长到真实值步长*/
            	}
            	else                                /*目标SOC大于90%(快要充满)*/
            	{
            		stepSoc = (aimSoc - copySoc) / 300; /*计算30s内增长到真实值步长*/
            	}

				if(0 == stepSoc)                    /*必须要有增量*/
				{
					stepSoc = 1;
				}
        	}
        	else if(aimSoc >= 8000)                 /*目标SOC大于80%*/
        	{
        		stepSoc = (aimSoc - copySoc) / 600; /*计算60s内增长到真实值步长*/
				if(0 == stepSoc)
				{
	        		if(sTime[clunum] >= 2)                  /*计时0.2s*/
	        		{
	    				stepSoc = 1;                /*增加0.01%*/
	    				sTime[clunum] = 0;
	        		}
	        		else                            /*累计时间*/
	        		{
	    				stepSoc = 0;                /*SOC不变化*/
	    				sTime[clunum]++;
	        		}
				}
        	}
        	else                                    /*目标SOC比较低*/
        	{
        		stepSoc = (aimSoc - copySoc) / 1200;/*计算120s内增长到真实值步长*/
				if(0 == stepSoc)
				{
	        		if(sTime[clunum] >= 3)                  /*计时0.3s*/
	        		{
	    				stepSoc = 1;                /*增加0.01%*/
	    				sTime[clunum] = 0;
	        		}
	        		else                            /*累计时间*/
	        		{
	    				stepSoc = 0;                /*SOC不变化*/
	    				sTime[clunum]++;
	        		}
				}
        	}

        	changSoc += stepSoc;                    /*原变化基础上增加加速变化量*/
            
            copySoc += changSoc;                    /*计算本次变化SOC*/
            
            if(copySoc > aimSoc)                    /*防止追赶过界*/
            {
                copySoc = aimSoc;
            }

            if(copySoc > 10000)
            {
                copySoc = 10000;
            }
        }
        else if(copySoc > aimSoc)                   /*显示平滑值高于真实目标值*/
        {
            changSoc = changSoc / SOC_SLOW_SPEED;   /*减缓速度变化的SOC*/
            
            if(0 == changSoc)                       /*变化过小*/
            {
                inteCap = sHisCap[clunum];                  /*不更新本次容量变化值*/
            }
            
            copySoc += changSoc;                    /*平滑值放慢上升速度等待真实值*/
            
            if(copySoc > 10000)
            {
                copySoc = 10000;
            }
        }
    }
    else if((2 == state) && (2 == sHisState[clunum]))       /*持续放电(SOC减小)*/
    {
        changCap = ABS(sHisCap[clunum], inteCap);           /*放电正数值增大*/

        changSoc = (u16)(changCap * 10000 / totalCap);/*实际变化的SOC*/

        if(0 == changSoc)                           /*变化过小*/
        {
            inteCap = sHisCap[clunum];                      /*不更新本次容量变化值*/
        }

        if(copySoc > aimSoc)                        /*显示平滑值高于实际目标值*/
        {
        	if(aimSoc <= 2000)                      /*目标SOC低于20%(快要放空)*/
        	{
            	if(aimSoc <= 350)                   /*目标SOC低于3.5%(接近放空)*/
            	{
            		stepSoc = (copySoc - aimSoc) / 50;  /*计算5s内增长到真实值步长*/
            	}
            	else if(aimSoc <= 800)              /*目标SOC低于8%(快要放空)*/
            	{
            		stepSoc = (copySoc - aimSoc) / 150; /*计算15s内增长到真实值步长*/
            	}
            	else if(aimSoc <= 1400)             /*目标SOC低于14%(快要放空)*/
            	{
            		stepSoc = (copySoc - aimSoc) / 200; /*计算20s内增长到真实值步长*/
            	}
            	else                                /*目标SOC低于20%(快要放空)*/
            	{
            		stepSoc = (copySoc - aimSoc) / 300; /*计算30s内增长到真实值步长*/
            	}

				if(0 == stepSoc)                    /*必须要有增量*/
				{
					stepSoc = 1;
				}
        	}
        	else if(aimSoc <= 2500)                 /*目标SOC低于25%*/
        	{
        		stepSoc = (copySoc - aimSoc) / 600; /*计算60s内增长到真实值步长*/
				if(0 == stepSoc)
				{
	        		if(sTime[clunum] >= 2)                  /*计时0.2s*/
	        		{
	    				stepSoc = 1;                /*增加0.01%*/
	    				sTime[clunum] = 0;
	        		}
	        		else                            /*累计时间*/
	        		{
	    				stepSoc = 0;                /*SOC不变化*/
	    				sTime[clunum]++;
	        		}
				}
        	}
        	else                                    /*目标SOC比较高*/
        	{
        		stepSoc = (copySoc - aimSoc) / 1200;/*计算120s内增长到真实值步长*/
				if(0 == stepSoc)
				{
	        		if(sTime[clunum] >= 3)                  /*计时0.3s*/
	        		{
	    				stepSoc = 1;                /*增加0.01%*/
	    				sTime[clunum] = 0;
	        		}
	        		else                            /*累计时间*/
	        		{
	    				stepSoc = 0;                /*SOC不变化*/
	    				sTime[clunum]++;
	        		}
				}
        	}
            
        	changSoc += stepSoc;                    /*原变化基础上增加加速变化量*/

            if(copySoc > changSoc)                  /*防止减过界*/
            {
                copySoc -= changSoc;                /*计算本次变化SOC*/
            }
            else
            {
                copySoc = 0;
            }
            
            if(copySoc < aimSoc)                    /*平滑值不应该下降低于真实值*/
            {
                copySoc = aimSoc;
            }
        }
        else if(copySoc < aimSoc)                   /*平滑值低于实际值*/
        {
            changSoc = changSoc / SOC_SLOW_SPEED;   /*平滑值放慢下降速度等待真实值*/
            
            if(0 == changSoc)                       /*变化过小*/
            {
                inteCap = sHisCap[clunum];                  /*不更新本次容量变化值*/
            }
            
            if(copySoc > changSoc)                  /*防止减过界*/
            {
                copySoc -= changSoc;
            }
            else
            {
                copySoc = 0;
            }
        }
    }
    
    sHisState[clunum] = state;                              /*记录历史状态*/
    sHisCap[clunum] = inteCap;                              /*记录此次积分容量*/

    return(copySoc);
}
/*=============================================================================================
** @Name      : static void UpdateSocSlowDisplay(u8 clunum, u16 slowSoc)
** @Input     : slowSoc:平滑后待显示的值
** @Output    : void
** @Function  : 将平滑值更新到显示值
** @The notes :
**===========================================================================================*/
static void UpdateSocSlowDisplay(u8 clunum, u16 slowSoc)
{
    static u16 sShowCopy[2] = {0};

    if(gSysCapInfo_52 == NULL)
    {
        ssdk_printf(SSDK_WARNING, "%s: gSysCapInfo_52 null!!!\r\n", __func__);
        return;
    }
    if(clunum >= 2)
    {
        return;
    }

    if(slowSoc > 10000)
    {
        slowSoc = 10000;
    }
    
    if(EnerGetGroupVHProStateHook(clunum) > 0)                    /*高压一级保护*/
    {
        sSlowDisplaySoc[clunum] = 10000;                            /*高压保护直接显示为100%*/
    }
    else if(EnerGetGroupVLProStateHook(clunum) > 0)               /*低压一级保护*/
    {
        sSlowDisplaySoc[clunum] = 0;                                /*低压保护直接显示为0%*/
    }
    else if((slowSoc >= 9900)                               /*平滑值高于99%*/
        && (0 == EnerGetGroupVHLimStateHook(clunum)))             /*SOC高于99%但是未到一般高压报警*/
    {
        if(sSlowDisplaySoc[clunum] <= 9900)                         /*显示值未高于99%*/
        {
            sSlowDisplaySoc[clunum] = 9900;                         /*保持显示99%*/
        }
        else if(slowSoc < sSlowDisplaySoc[clunum])                  /*平滑值变小(放电)*/
        {
            sSlowDisplaySoc[clunum] = slowSoc;                      /*显示平滑值*/
        }
    }
    else if((slowSoc <= 100)                                /*平滑值低于1%*/
        && (0 == EnerGetGroupVLLimStateHook(clunum)))             /*SOC低于1%但是未到低压放电截止报警 */
    {
        if(sSlowDisplaySoc[clunum] >= 100)                          /*显示值未低于1%*/
        {
            sSlowDisplaySoc[clunum] = 100;                          /*保持显示为1%*/
        }
        else if(slowSoc > sSlowDisplaySoc[clunum])                  /*平滑值变大(充电)*/
        {
            sSlowDisplaySoc[clunum] = slowSoc;                      /*显示平滑值*/
        }
    }
    else
    {
        sSlowDisplaySoc[clunum] = slowSoc;                          /*显示平滑值*/
    }
    
    if(FALSE == EnerGetUserSocStopReqHook())                /*不需要停止显示*/
    {
        if(clunum == 0)
        {
            gSysCapInfo_52[eSysCapInfo52_1SOC] = (sSlowDisplaySoc[clunum] + 5) / 10;/*四舍五入转化为显示单位(千分之一)*/
        }
        else
        {
            gSysCapInfo_52[eSysCapInfo52_2SOC] = (sSlowDisplaySoc[clunum] + 5) / 10;/*四舍五入转化为显示单位(千分之一)*/
        }
    }

    if((sShowCopy[clunum] / 100) != (sSlowDisplaySoc[clunum] / 100))        /*变化超过1%*/
    {
        EnerChangEepGNowSocHook(clunum, (sSlowDisplaySoc[clunum] / 10));    /*保存显示SOC*/
        sShowCopy[clunum] =  sSlowDisplaySoc[clunum];
    } 
}

