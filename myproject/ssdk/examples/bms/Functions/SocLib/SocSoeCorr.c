/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : SocSoeCorr.c
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : SOC/SOE修正模块
** @Instructions :
**===========================================================================================*/

/*=============================================================================================
** @Header file definition
**===========================================================================================*/
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "SocInterface.h"
#include "SocUserAPI.h"
#include "SocSoeCorr.h"
#include "EnerInfoCalc.h"
#include "CapInfoCalc.h"
#include "EnerInterface.h"

static u16 sTCapCorrSoc[2] = {0};        /*温度修正总容量计算的SOC*/
static u16 sTEnerCorrSoe[2] = {0};       /*温度修正总容量计算的SOE*/
static u16 sVStepCorrSoc[2] = {0};       /*电压末端按步长修正计算的SOC*/
static u16 sVStepCorrSoe[2] = {0};       /*电压末端按步长修正计算的SOE*/

extern const u16 cTempCapPercent[]; /*不同温度下可用总容量百分比(0.1%)*/
extern const u16 cTempEnerPercent[];/*不同温度下可用总能量百分比(0.1%)*/

extern u16 gGBmuGenPara_102[];		/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
/*=============================================================================================
** @Name      : void SocSoeAllCorrTask(void)
** @Input     : void
** @Output    : void
** @Function  : SOC/SOE修正总任务
** @The notes : 500ms
**===========================================================================================*/
void SocSoeAllCorrTask(void)
{
	u16 nowSoc = 0;
	u16 nowSoe = 0;
	u16 calcSoc = 0;
	u16 calcSoe = 0;
    u8 clunum = 0;
    for(clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        nowSoc = 0;
        nowSoe = 0;
        calcSoc = 0;
        calcSoe = 0;
        if(clunum >= 2)
        {
            return;
        }
        /*单体信息未就绪*/
        if(0 == EnerGetGroupBattReadyHook(clunum))
        {
            continue;
        }

        /*温度影响总容量修正SOC/SOE*/
        SocSoeTempCorrDeal(clunum);

        /*充放电末端按步长修正SOC/SOE*/
        SocSoeVStepCorrDeal(clunum);

        nowSoc = GetGRealSocMilliAPI(clunum);
        nowSoe = GetGRealSoeMilliAPI(clunum);

        /*只有温度修正使能*/
        #if((0 == SOCSOE_VHLSTEP_CORR_EN) && (1 == SOCSOE_TCAPENER_CORR_EN))
        sVStepCorrSoc[clunum] = sTCapCorrSoc[clunum];
        sVStepCorrSoe[clunum] = sTEnerCorrSoe[clunum];
        #endif

        /*只有步长修正使能*/
        #if((0 == SOCSOE_TCAPENER_CORR_EN) && (1 == SOCSOE_VHLSTEP_CORR_EN))
        sTCapCorrSoc[clunum] = sVStepCorrSoc[clunum];
        sTEnerCorrSoe[clunum] = sVStepCorrSoe[clunum];
        #endif

        /*温度总容量修正SOC大于0.5%*/
        /*充放电末端电压修正SOC大于0.5%*/
        if((ABS(sTCapCorrSoc[clunum], nowSoc) > 5)
            || (ABS(sVStepCorrSoc[clunum], nowSoc) > 5))
        {
            /*温度总容量修正SOC值与充放电末端电压修正SOC值一致(3%内)*/
            if(ABS(sTCapCorrSoc[clunum], sVStepCorrSoc[clunum]) <= 30)
            {
                calcSoc = (sTCapCorrSoc[clunum] + sVStepCorrSoc[clunum]) / 2;
            }
            /*温度总容量修正SOC值与当前SOC值一致(3%内)*/
            else if((ABS(sTCapCorrSoc[clunum], nowSoc) <= 30)
                && (ABS(sTCapCorrSoc[clunum], nowSoc) > 10))
            {
                calcSoc = (sTCapCorrSoc[clunum] + nowSoc) / 2;
            }
            /*充放电末端电压修正SOC值与当前SOC值一致(3%内)*/
            else if((ABS(sVStepCorrSoc[clunum], nowSoc) <= 30)
                && (ABS(sVStepCorrSoc[clunum], nowSoc) > 10))
            {
                calcSoc = (sVStepCorrSoc[clunum] + nowSoc) / 2;
            }
            /*修正SOC值间有误差*/
            else
            {
                calcSoc = (sTCapCorrSoc[clunum] + sVStepCorrSoc[clunum] + nowSoc) / 3;
            }

            /*修正SOC值超过0.5%才执行修正*/
            if(ABS(calcSoc, nowSoc) >= 5)
            {
                /*执行soc修正*/
                CorrGNowCapBySocAPI(clunum, calcSoc);
            }
        }

        /*温度总容量修正SOE大于0.5%*/
        /*充放电末端电压修正SOE大于0.5%*/
        if((ABS(sTEnerCorrSoe[clunum], nowSoe) > 5)
            || (ABS(sVStepCorrSoe[clunum], nowSoe) > 5))
        {
            /*温度总容量修正SOE值与充放电末端电压修正SOE值一致(3%内)*/
            if(ABS(sTEnerCorrSoe[clunum], sVStepCorrSoe[clunum]) <= 30)
            {
                calcSoe = (sTEnerCorrSoe[clunum] + sVStepCorrSoe[clunum]) / 2;
            }
            /*温度总容量修正SOE值与当前SOE值一致(3%内)*/
            else if((ABS(sTEnerCorrSoe[clunum], nowSoe) <= 30)
                && (ABS(sTEnerCorrSoe[clunum], nowSoe) > 10))
            {
                calcSoe = (sTEnerCorrSoe[clunum] + nowSoe) / 2;
            }
            /*充放电末端电压修正SOE值与当前SOE值一致(3%内)*/
            else if((ABS(sVStepCorrSoe[clunum], nowSoe) <= 30)
                && (ABS(sVStepCorrSoe[clunum], nowSoe) > 10))
            {
                calcSoe = (sVStepCorrSoe[clunum] + nowSoe) / 2;
            }
            /*修正SOE值间有误差*/
            else
            {
                calcSoe = (sTEnerCorrSoe[clunum] + sVStepCorrSoe[clunum] + nowSoe) / 3;
            }

            /*修正SOC值超过0.5%才执行修正*/
            if(ABS(calcSoe, nowSoe) > 5)
            {
                /*执行soe修正*/
                CorrGNowEnerBySoeAPI(clunum, calcSoe);
            }
        }
    }
}
/*=============================================================================================
** @Static function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : static void SocSoeTempCorrDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : SOC/SOE温度修正处理任务
** @The notes : 温度影响总容量则按比例修正当前容量
**===========================================================================================*/
static void SocSoeTempCorrDeal(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
	#if(1 == SOCSOE_TCAPENER_CORR_EN)
	s8 temp = 0;
	u8 index = 0;
	u16 capPer = 0;
	u16 enerPer = 0;
	u32 nowCap = 0;
	u32 nowEner = 0;

    /*当前温度取电芯平均值*/
	temp = (GetGCellAvgTempAPI(clunum) + 5) / 10;

	/*将温度转化为百分率表索引([-30,60]对应0~9)*/
	if(temp <= (s8)CAPENER_PER_FST_T)
	{
		index = 0;
		capPer = cTempCapPercent[index];
		enerPer = cTempEnerPercent[index];
	}
	else if(temp >= ((s8)CAPENER_PER_FST_T + (CAPENER_PER_SUB_T * (CAPENER_T_PER_LEN - 1))))
	{
		index = CAPENER_T_PER_LEN - 1;
		capPer = cTempCapPercent[index];
		enerPer = cTempEnerPercent[index];
	}
	else
	{
		/*取下端*/
		index = (temp + (0 - (s8)CAPENER_PER_FST_T)) / CAPENER_PER_SUB_T;
		capPer = cTempCapPercent[index] + (((s16)cTempCapPercent[index + 1] - (s16)cTempCapPercent[index]) * ((temp + (0 - (s8)CAPENER_PER_FST_T)) % CAPENER_PER_SUB_T) / CAPENER_PER_SUB_T);
		enerPer = cTempEnerPercent[index] + (((s16)cTempEnerPercent[index + 1] - (s16)cTempEnerPercent[index]) * ((temp + (0 - (s8)CAPENER_PER_FST_T)) % CAPENER_PER_SUB_T) / CAPENER_PER_SUB_T);
	}

	nowCap = GetGroupRemainCapAPI(clunum) * capPer / 1000;

	sTCapCorrSoc[clunum] = nowCap * 1000 / GetGroupTotalCapAPI(clunum);
	if(ABS(sTCapCorrSoc[clunum], GetGRealSocMilliAPI(clunum)) > 30)
	{
		if(sTCapCorrSoc[clunum] > GetGRealSocMilliAPI(clunum))
		{
			if(GetGRealSocMilliAPI(clunum) < 970)
			{
				sTCapCorrSoc[clunum] = GetGRealSocMilliAPI(clunum) + 30;
			}
			else
			{
				sTCapCorrSoc[clunum] = GetGRealSocMilliAPI(clunum);
			}
		}
		else
		{
			if(GetGRealSocMilliAPI(clunum) > 30)
			{
				sTCapCorrSoc[clunum] = GetGRealSocMilliAPI(clunum) - 30;
			}
			else
			{
				sTCapCorrSoc[clunum] = GetGRealSocMilliAPI(clunum);
			}
		}
	}
	else
	{
		sTCapCorrSoc[clunum] = GetGRealSocMilliAPI(clunum);
	}

	nowEner = GetGroupRemainEnerAPI(clunum)* enerPer / 1000;

	sTEnerCorrSoe[clunum] = nowEner * 1000 / GetGroupTotalEnerAPI();
	if(ABS(sTEnerCorrSoe[clunum], GetGRealSoeMilliAPI(clunum)) > 30)
	{
		if(sTEnerCorrSoe[clunum] > GetGRealSoeMilliAPI(clunum))
		{
			if(GetGRealSoeMilliAPI(clunum) < 970)
			{
				sTEnerCorrSoe[clunum] = GetGRealSoeMilliAPI(clunum) + 30;
			}
			else
			{
				sTEnerCorrSoe[clunum] = GetGRealSoeMilliAPI(clunum);
			}
		}
		else
		{
			if(GetGRealSoeMilliAPI(clunum) > 30)
			{
				sTEnerCorrSoe[clunum] = GetGRealSoeMilliAPI(clunum) - 30;
			}
			else
			{
				sTEnerCorrSoe[clunum] = GetGRealSoeMilliAPI(clunum);
			}
		}
	}
	else
	{
		sTEnerCorrSoe[clunum] = GetGRealSoeMilliAPI(clunum);
	}
	#else
	sTCapCorrSoc[clunum] = GetGRealSocMilliAPI(clunum);
	sTEnerCorrSoe[clunum] = GetGRealSoeMilliAPI(clunum);
	#endif
}
/*=============================================================================================
** @Name      :  static void SocSoeVStepCorrDeal(u8 clunum)
** @Input     :  clunum:簇编号
** @Output    :  void
** @Function  :  SOC/SOE充放电末端按电压步长修正
** @The notes :  确保充放电截止时SOC平滑到达目标值
**===========================================================================================*/
static void SocSoeVStepCorrDeal(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
	#if(1 == SOCSOE_VHLSTEP_CORR_EN)
    u16 nowSoc = 0;
    u16 nowSoe = 0;
    u16 finVolt = 0;
    u16 fstVolt = 0;
    u32 voltStepSoc = 0xffff;
    u32 voltStepSoe = 0xffff;

    static u16 sHisState[2] = {0};
    static u16 sSocStartVolt[2] = {0};      //SOC电压步长修正计算kB值起始电压
    static u16 sSoeStartVolt[2] = {0};      //SOE电压步长修正计算kB值起始电压
    static u32 sSocVoltCorrK[2] = {0};      //SOC电压步长修正K值
    static u32 sSocVoltCorrB[2] = {0};      //SOC电压步长修正B值
    static u32 sSoeVoltCorrK[2] = {0};      //SOE电压步长修正K值
    static u32 sSoeVoltCorrB[2] = {0};      //SOE电压步长修正B值

    //获取当前SOC、SOE
    nowSoc = GetGRealSocMilliAPI(clunum);
    nowSoe = GetGRealSoeMilliAPI(clunum);

    //持续充电状态(检测充电末端)
    if((eCURR_CHG == GetGChgDhgStateAPI(clunum))
    	&& (sHisState[clunum] == GetGChgDhgStateAPI(clunum)))
    {
    	//充电末端起止点电压
        finVolt = gGBmuGenPara_102[eBmuGenPara102_VH2Lim] - 10;
        fstVolt = gGBmuGenPara_102[eBmuGenPara102_VH2Lim] - STEP_CORR_CHG_SUBV;

        //未到达修正起始点时计算SOC和SOE修正K\B值
        if(GetGCellMaxVoltAPI(clunum) <= fstVolt)
        {
        	//SOC未达到目标值(低于目标值)
        	if(nowSoc < STEP_CORR_FIN_HSOC)
        	{
	        	//记录SOC起始电压点
	        	sSocStartVolt[clunum] = fstVolt;

				//计算KB值:当前SOC升至101%理论上单位电压变化应有的SOC变化值
				sSocVoltCorrK[clunum] = (u32)(STEP_CORR_FIN_HSOC - nowSoc) * 1000 / (u32)STEP_CORR_CHG_SUBV;
				sSocVoltCorrB[clunum] = nowSoc;
        	}
        	//SOC到达或接近目标值
        	else
        	{
        		//不需修正SOC
        	    sSocStartVolt[clunum] = 0;
				sSocVoltCorrK[clunum] = 0;
				sSocVoltCorrB[clunum] = 0;
        	}

        	//SOE未达到目标值(低于目标值2%)
        	if(nowSoe < STEP_CORR_FIN_HSOE)
        	{
	        	//记录SOE起始电压点
	        	sSoeStartVolt[clunum] = fstVolt;

				//计算KB值:当前SOE升至101%理论上单位电压变化应有的SOE变化值
				sSoeVoltCorrK[clunum] = (u32)(STEP_CORR_FIN_HSOE - nowSoe) * 1000 / (u32)STEP_CORR_CHG_SUBV;
				sSoeVoltCorrB[clunum] = nowSoe;
        	}
        	//SOE到达或接近目标值
        	else
        	{
        		//不需修正SOE
        	    sSoeStartVolt[clunum] = 0;
				sSoeVoltCorrK[clunum] = 0;
				sSoeVoltCorrB[clunum] = 0;
        	}
        }
        //电压到达起始电压
        else
        {
        	//电压未到达截止点
        	if(GetGCellMaxVoltAPI(clunum) < finVolt)
        	{
            	//当前SOC未达到目标值(低于目标值)
            	if(nowSoc < STEP_CORR_FIN_HSOC)
            	{
					//超过修正起始点时未计算SOC修正K\B值或电压回降
					if(((0 == sSocVoltCorrK[clunum]) && (0 == sSocVoltCorrB[clunum]))
						|| (GetGCellMaxVoltAPI(clunum) < sSocStartVolt[clunum]))
					{
						//重新计算KB值:当前SOC升至101%理论上单位电压变化应有的SOC变化值
						sSocVoltCorrK[clunum] = (u32)(STEP_CORR_FIN_HSOC - nowSoc) * 1000 / (u32)(finVolt - GetGCellMaxVoltAPI(clunum));
						sSocVoltCorrB[clunum] = nowSoc;

						//记录SOC起始电压点
						sSocStartVolt[clunum] = GetGCellMaxVoltAPI(clunum);
					}
					//超过修正起始点且已计算SOC修正K\B值
					else
					{
						//通过y=kx+b计算当前电压对应的理论SOC
						voltStepSoc = sSocVoltCorrK[clunum] * (GetGCellMaxVoltAPI(clunum) - sSocStartVolt[clunum]) / 1000 + sSocVoltCorrB[clunum];

						//计算理论SOC合法[50,100]%
						if((500 <= voltStepSoc) && (voltStepSoc <= STEP_CORR_FIN_HSOC))
						{
							//当前SOC比理论SOC大则不修正
							if(nowSoc >= voltStepSoc)
							{
								//不需修正
								voltStepSoc = 0xffff;
							}
						}
						//计算理论SOC不合法
						else
						{
							//KB值异常清零
			        	    sSocStartVolt[clunum] = 0;
							sSocVoltCorrK[clunum] = 0;
							sSocVoltCorrB[clunum] = 0;
							//不需修正
							voltStepSoc = 0xffff;
						}
					}
            	}
            	//当前SOC接近目标值
            	else
            	{
            		//不需修正SOC
            	    sSocStartVolt[clunum] = 0;
    				sSocVoltCorrK[clunum] = 0;
    				sSocVoltCorrB[clunum] = 0;
            	}

            	//当前SOE未达到目标值(低于目标值)
            	if(nowSoe < STEP_CORR_FIN_HSOE)
            	{
					//超过修正起始点时未计算SOE修正K\B值或电压回降
					if(((0 == sSoeVoltCorrK[clunum]) && (0 == sSoeVoltCorrB[clunum]))
						|| (GetGCellMaxVoltAPI(clunum) < sSoeStartVolt[clunum]))
					{
						//记录SOE起始电压点
						sSoeStartVolt[clunum] = GetGCellMaxVoltAPI(clunum);

						//重新计算KB值:当前SOC升至101%理论上单位电压变化应有的SOC变化值
						sSoeVoltCorrK[clunum] = (u32)(STEP_CORR_FIN_HSOE - nowSoe) * 1000 / (u32)(finVolt - GetGCellMaxVoltAPI(clunum));
						sSoeVoltCorrB[clunum] = nowSoe;
					}
					//超过修正起始点且已计算SOE修正K\B值
					else
					{
						//通过y=kx+b计算当前电压对应的理论SOE
						voltStepSoe = sSoeVoltCorrK[clunum] * (GetGCellMaxVoltAPI(clunum) - sSoeStartVolt[clunum]) / 1000 + sSoeVoltCorrB[clunum];

						//计算理论SOE合法[50,100]%
						if((500 <= voltStepSoe) && (voltStepSoe <= STEP_CORR_FIN_HSOE))
						{
							//当前SOE比理论SOE大则不修正
							if(nowSoe >= voltStepSoe)
							{
								//不需修正
								voltStepSoe = 0xffff;
							}
						}
						//计算理论SOC不合法
						else
						{
							//KB值异常清零
			        	    sSoeStartVolt[clunum] = 0;
							sSoeVoltCorrK[clunum] = 0;
							sSoeVoltCorrB[clunum] = 0;
							//不需修正SOE
							voltStepSoe = 0xffff;
						}
					}
            	}
            	//当前SOE到达目标值
            	else
            	{
            		//不需修正SOE
            	    sSoeStartVolt[clunum] = 0;
    				sSoeVoltCorrK[clunum] = 0;
    				sSoeVoltCorrB[clunum] = 0;
            	}
        	}
        	//电压到达截止电压值
        	else
        	{
        		//不需计算修正SOC/SOE
        	    sSocStartVolt[clunum] = 0;
        	    sSoeStartVolt[clunum] = 0;
				sSocVoltCorrK[clunum] = 0;
				sSocVoltCorrB[clunum] = 0;
				sSoeVoltCorrK[clunum] = 0;
				sSoeVoltCorrB[clunum] = 0;

				//当前SOC未达到目标值
            	if(nowSoc < STEP_CORR_FIN_HSOC)
            	{
					voltStepSoc = STEP_CORR_FIN_HSOC;
            	}

				//当前SOE未达到目标值
            	if(nowSoe < STEP_CORR_FIN_HSOE)
            	{
					voltStepSoe = STEP_CORR_FIN_HSOE;
            	}
        	}
        }
    }
    //持续放电状态(检测放电末端)
    else if((eCURR_DHG == GetGChgDhgStateAPI(clunum))
    	&& (sHisState[clunum] == GetGChgDhgStateAPI(clunum)))
    {
    	//放电末端起止点电压
        finVolt = gGBmuGenPara_102[eBmuGenPara102_VL2Lim] + 10;
        fstVolt = gGBmuGenPara_102[eBmuGenPara102_VL2Lim] + STEP_CORR_DHG_SUBV;

        //未到达修正起始点时计算SOC和SOE修正K\B值
        if(GetGCellMinVoltAPI(clunum) >= fstVolt)
        {
        	//SOC未达到目标值(高于目标值)
        	if(nowSoc > STEP_CORR_FIN_LSOC)
        	{
	        	//记录SOC起始电压点
	        	sSocStartVolt[clunum] = fstVolt;

				//计算KB值:当前SOC升至101%理论上单位电压变化应有的SOC变化值
				sSocVoltCorrK[clunum] = (u32)(nowSoc - STEP_CORR_FIN_LSOC) * 1000 / (u32)STEP_CORR_DHG_SUBV;
				sSocVoltCorrB[clunum] = nowSoc;
        	}
        	//SOC到达或接近目标值
        	else
        	{
        		//不需修正SOC
        	    sSocStartVolt[clunum] = 0;
				sSocVoltCorrK[clunum] = 0;
				sSocVoltCorrB[clunum] = 0;
        	}

        	//SOE未达到目标值(高于目标值)
        	if(nowSoe > STEP_CORR_FIN_LSOE)
        	{
	        	//记录SOE起始电压点
	        	sSoeStartVolt[clunum] = fstVolt;

				//计算KB值:当前SOE升至101%理论上单位电压变化应有的SOE变化值
				sSoeVoltCorrK[clunum] = (u32)(nowSoe - STEP_CORR_FIN_LSOE) * 1000 / (u32)STEP_CORR_DHG_SUBV;
				sSoeVoltCorrB[clunum] = nowSoe;
        	}
        	//SOE到达或接近目标值
        	else
        	{
        		//不需修正SOE
        	    sSoeStartVolt[clunum] = 0;
				sSoeVoltCorrK[clunum] = 0;
				sSoeVoltCorrB[clunum] = 0;
        	}
        }
        //电压到达起始电压
        else
        {
        	//电压未到达截止点
        	if(GetGCellMinVoltAPI(clunum) > finVolt)
        	{
            	//当前SOC未达到目标值(高于目标值)
            	if(nowSoc > STEP_CORR_FIN_LSOC)
            	{
					//超过修正起始点时未计算SOC修正K\B值或电压回升
					if(((0 == sSocVoltCorrK[clunum]) && (0 == sSocVoltCorrB[clunum]))
						|| (GetGCellMinVoltAPI(clunum) > sSocStartVolt[clunum]))
					{
						//记录SOC起始电压点
						sSocStartVolt[clunum] = GetGCellMinVoltAPI(clunum);

						//重新计算KB值:当前SOC升至101%理论上单位电压变化应有的SOC变化值
						sSocVoltCorrK[clunum] = (u32)(nowSoc - STEP_CORR_FIN_LSOC) * 1000 / (u32)(GetGCellMinVoltAPI(clunum) - finVolt);
						sSocVoltCorrB[clunum] = nowSoc;
					}
					//超过修正起始点且已计算SOC修正K\B值
					else
					{
						//通过y=-kx+b计算当前电压对应的理论SOC
						voltStepSoc = sSocVoltCorrB[clunum] - (s32)(sSocVoltCorrK[clunum] * (sSocStartVolt[clunum] - GetGCellMinVoltAPI(clunum)) / 1000);

						//计算理论SOC合法[0,50]%
						if((STEP_CORR_FIN_LSOC <= voltStepSoc) && (voltStepSoc <= 500))
						{
							//当前SOC比理论SOC小则不修正
							if(nowSoc <= voltStepSoc)
							{
								//不需修正
								voltStepSoc = 0xffff;
							}
						}
						//计算理论SOC不合法
						else
						{
							//KB值异常清零
			        	    sSocStartVolt[clunum] = 0;
							sSocVoltCorrK[clunum] = 0;
							sSocVoltCorrB[clunum] = 0;
							//不需修正
							voltStepSoc = 0xffff;
						}
					}
            	}
            	//当前SOC接近目标值
            	else
            	{
            		//不需修正SOC
            	    sSocStartVolt[clunum] = 0;
    				sSocVoltCorrK[clunum] = 0;
    				sSocVoltCorrB[clunum] = 0;
            	}

            	//当前SOE未达到目标值(高于目标值)
            	if(nowSoe > STEP_CORR_FIN_LSOE)
            	{
					//超过修正起始点时未计算SOE修正K\B值或电压回升
					if(((0 == sSoeVoltCorrK[clunum]) && (0 == sSoeVoltCorrB[clunum]))
						|| (GetGCellMinVoltAPI(clunum) > sSoeStartVolt[clunum]))
					{
						//记录SOE起始电压点
						sSoeStartVolt[clunum] = GetGCellMinVoltAPI(clunum);

						//重新计算KB值:当前SOC升至101%理论上单位电压变化应有的SOC变化值
						sSoeVoltCorrK[clunum] = (u32)(nowSoe - STEP_CORR_FIN_LSOE) * 1000 / (u32)(GetGCellMinVoltAPI(clunum) - finVolt);
						sSoeVoltCorrB[clunum] = nowSoe;
					}
					//超过修正起始点且已计算SOE修正K\B值
					else
					{
						//通过y=-kx+b计算当前电压对应的理论SOE
						voltStepSoe = sSoeVoltCorrB[clunum] - (s32)(sSoeVoltCorrK[clunum] * (sSoeStartVolt[clunum] - GetGCellMinVoltAPI(clunum)) / 1000);

						//计算理论SOE合法[0,50]%
						if((STEP_CORR_FIN_LSOE <= voltStepSoe) && (voltStepSoe <= 500))
						{
							//当前SOE比理论SOE小则不修正
							if(nowSoe <= voltStepSoe)
							{
								//不需修正
								voltStepSoe = 0xffff;
							}
						}
						//计算理论SOC不合法
						else
						{
							//KB值异常清零
			        	    sSoeStartVolt[clunum] = 0;
							sSoeVoltCorrK[clunum] = 0;
							sSoeVoltCorrB[clunum] = 0;
							//不需修正SOE
							voltStepSoe = 0xffff;
						}
					}
            	}
            	//当前SOE接近目标值
            	else
            	{
            		//不需修正SOE
            	    sSoeStartVolt[clunum] = 0;
    				sSoeVoltCorrK[clunum] = 0;
    				sSoeVoltCorrB[clunum] = 0;
            	}
        	}
        	//电压到达截止电压值
        	else
        	{
        		//不需计算修正SOC/SOE
        	    sSocStartVolt[clunum] = 0;
        	    sSoeStartVolt[clunum] = 0;
				sSocVoltCorrK[clunum] = 0;
				sSocVoltCorrB[clunum] = 0;
				sSoeVoltCorrK[clunum] = 0;
				sSoeVoltCorrB[clunum] = 0;

				//当前SOC未达到目标值
            	if(nowSoc > STEP_CORR_FIN_LSOC)
            	{
					voltStepSoc = STEP_CORR_FIN_LSOC;
            	}

				//当前SOE未达到目标值
            	if(nowSoe > STEP_CORR_FIN_LSOE)
            	{
					voltStepSoe = STEP_CORR_FIN_LSOE;
            	}
        	}
        }
    }
    //空闲状态或状态变化
    else
    {
		//不需修正SOC/SOE
	    sSocStartVolt[clunum] = 0;
	    sSoeStartVolt[clunum] = 0;
		sSocVoltCorrK[clunum] = 0;
		sSocVoltCorrB[clunum] = 0;
		sSoeVoltCorrK[clunum] = 0;
		sSoeVoltCorrB[clunum] = 0;

		//记录状态
		sHisState[clunum] = GetGChgDhgStateAPI(clunum);
    }

    //不需修正SOC
    if(0xffff == voltStepSoc)
    {
    	//取当前SOC值
    	sVStepCorrSoc[clunum] = nowSoc;
    }
    else
    {
    	//记录需修正SOC目标值
    	sVStepCorrSoc[clunum] = voltStepSoc;
    }

    //不需修正SOE
    if(0xffff == voltStepSoe)
    {
    	//取当前SOE值
    	sVStepCorrSoe[clunum] = nowSoe;
    }
    else
    {
    	//记录需修正SOE目标值
    	sVStepCorrSoe[clunum] = voltStepSoe;
    }
	#else
	sVStepCorrSoc = GetGRealSocMilliAPI(clunum);
	sVStepCorrSoe = GetGRealSoeMilliAPI(clunum);
	#endif
}