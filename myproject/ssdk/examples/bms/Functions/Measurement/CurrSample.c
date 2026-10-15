/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : CurrSample.c
** @Founder      : SBK326
** @Date         : 2019.11.1
** @Function     : 参数存储模块读写参数初始化
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "ParaInterface.h"
#include "ParaExtEep.h"
#include "CurrSample.h"
#include "CurrIntegral.h"
#include "ParaWrite.h"
#include "CharactCalc.h"
#include "TacUser.h"
#include "ParaRead.h"
#include "DEVADC.h"

extern u16 gGBmuGenPara_102[];  							/*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[];							/*主控高级参数值*/
extern u8 gGHardPara_104[];  								/*主控硬件参数值*/

static u8 sCSampExpFlag[2] = {0};            /*电流采样异常标志 按e_CSampExp_ID置位: bit0:霍尔过载回检 bit1:霍尔供电 bit2:霍尔AD值 bit3:分流器AD值 bit4:霍尔/分流器不一致 bit5:分流器模拟前端(预留,无置位源) bit6:校零 bit7:自检(IMIU4上报)*/
static s32 sRealCurr10mA[2] = {0};           /*实时瞬时电流采样值 10mA*/
//static u32 sRealVolt10uV[2] = {0};           /*实时瞬时电压采样值 0.01mV*/

static s16 sCurrZeroMax[2] = {0};          /*电流零点最大值(10mA)*/
static s16 sCurrZeroMin[2] = {0};            /*电流零点最大值(10mA)*/
static s16 sCurrZeroCorr[2] = {0};           /*电流零点修正值(10mA)*/
static u8 sCurrZeroFin[2] = {0};             /*电流零点修正完成标志 1完成 2失败*/

/*同时使能霍尔和分流器时以分流器为主,霍尔作为参考单独记录*/
//static s32 sHallRealCurr10mA[2] = {0};       /*霍尔实时瞬时电流采样值 10mA*/
//static s32 sHallRealVolt10uV[2] = {0};       /*实时瞬时霍尔电压采样值 0.01mV*/
static s16 sHallZeroCorr[2] = {0};           /*霍尔电流零点修正值(10mA)*/

static void InputSampRealCurr10mA(u8 clunum, s32 curr);
static void CalcCurrZeroCorr(u8 clunum, s32 curr);

/*=============================================================================================
** @Name      : void CurrSampleInit(void)
** @Input     : void
** @Output    : void
** @Function  : 电流信息采样初始化
** @The notes :
**===========================================================================================*/
void CurrSampleInit(void)
{
	u16 zeroCurr[2] = {0};

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {    
		if(clunum >= 2)
		{
			return;
		}
        /*读取电流零点误差*/
        if(TRUE == ParaReadStoreCorrZeroCurr(clunum, zeroCurr, 2))
        {
            /*电流零点误差*/
            sCurrZeroCorr[clunum] = zeroCurr[1];
        }

        sCurrZeroMax[clunum] = (s16)gGHardPara_104[eGHardPara104_DhgIntPoint];
        sCurrZeroMin[clunum] = (0 - (s16)gGHardPara_104[eGHardPara104_ChgIntPoint]);

        sCurrZeroFin[clunum] = 0;
    }
}
/*=============================================================================================
** @Name      : void ISRCurrCapAddTask(void)
** @Input     : void
** @Output    : void
** @Function  : 定时中断电流积分任务
** @The notes : 必须1ms定时中断调用
**===========================================================================================*/
void ISRCurrCapAddTask(void)
{
    /*使能定时中断积分*/
    #if(1 == CURR_INT_ADD_EN)
    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {
        if(clunum >= 2)
		{
			return;
		}
        /*放电积分*/
        if((0 == SampGetCurrSampExpFlagAPI(clunum))                 /*电流采样正常*/
            && (sRealCurr10mA[clunum] > sCurrZeroMax[clunum])
            && (sRealCurr10mA[clunum] >= (s16)gGHardPara_104[eGHardPara104_DhgIntPoint]))
        {
            DhgCapIntTask(clunum, sRealCurr10mA[clunum], 1);

            /*单体采样正常用单体累计总压*/
            if(ABS(GetGCellSumVoltAPI(clunum), GetGSampSumVoltAPI(clunum)) <= gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV])
            {
                DhgEnerIntTask(clunum, sRealCurr10mA[clunum], GetGCellSumVoltAPI(clunum), 1);
            }
            /*单体采样异常用单体采样总压*/
            else
            {
                DhgEnerIntTask(clunum, sRealCurr10mA[clunum], GetGSampSumVoltAPI(clunum), 1);
            }
        }
        /*充电积分(正值)*/
        else if((0 == SampGetCurrSampExpFlagAPI(clunum))            /*电流采样正常*/
            && (sRealCurr10mA[clunum] < sCurrZeroMin[clunum])
            && (sRealCurr10mA[clunum] <= (0 - (s16)gGHardPara_104[eGHardPara104_ChgIntPoint])))
        {
            ChgCapIntTask(clunum, (0 - sRealCurr10mA[clunum]), 1);

            /*单体采样正常用单体累计总压*/
            if(ABS(GetGCellSumVoltAPI(clunum), GetGSampSumVoltAPI(clunum)) <= gGBmuHigLevPara_103[eBmuHigLevPara103_SumDiffV])
            {
                ChgEnerIntTask(clunum, (0 - sRealCurr10mA[clunum]), GetGCellSumVoltAPI(clunum), 1);
            }
            /*单体采样异常用单体采样总压*/
            else
            {
                ChgEnerIntTask(clunum, (0 - sRealCurr10mA[clunum]), GetGSampSumVoltAPI(clunum), 1);
            }
        }
    }
    #endif
}
/*=============================================================================================
** @Name      : void CalcSampleADToCurr(u8 clunum, s32 currAD)
** @Input     :clunum：簇编号  currAD:电流AD
** @Output    : void
** @Function  : 将采样AD值转化为电流值
** @The notes : 高压模块采电流时输入为电流值0.01A
**===========================================================================================*/
void CalcSampleADToCurr(u8 clunum, s32 currAD)
{
    u8 flag = 0;
    s32 curr10mA = 0;               /*主回路采样电流(带零飘)*/
    s32 realCurr10mA = 0;   	    /*主回路实际电流(无零飘)*/
    static s32 sHisCurr10mA[2] = {0};    /*上次采样电流值(无零飘)*/
    /*高压模块HVCU分流器采集电流,暂不实现自身分流器和霍尔数据采集和计算*/
    /*输入AD值即为电流值*/
    curr10mA = currAD;

    #if(1 == CURR_AUT_ZERO_EN)

    if((TRUE == TacGetGroupRunSwhAllOffStateAPI(clunum))                 /*电池组运行接触器都断开*/
		&& (eMODE_DEBUG != GetGWorkModeAPI(clunum))                       /*非调试模式(避免电流校准小电流也校零)*/
		&& (eWORK_ERR != GetGWorkStateAPI(clunum)))                       /*非故障状态(避免单板调试小电流也校零)*/
		//|| (1 == BitGet(sCSampExpFlag, eCExp_ZroErr))))             /*电流校零失败故障*/
		//|| (eCORR_HANDZRO == gGCalibrResult_83[eCorrRes83_CorrType]))/*手动电流校零*/
		{
			CalcCurrZeroCorr(clunum, curr10mA * 10);                            /*计算电流零飘值*/

			if(0 == sCurrZeroFin[clunum])                                       /*未完成电流零飘值计算*/
			{
				if(eWORK_INIT == GetGWorkStateAPI(clunum))                    /*系统自检状态*/
				{
					curr10mA = sCurrZeroCorr[clunum];                         /*强制真实电流为0*/
				}
			}
		}
        realCurr10mA = curr10mA - sCurrZeroCorr[clunum];                  /*去除电流零飘值*/
    #endif
        /*放电积分*/
    if((0 == SampGetCurrSampExpFlagAPI(clunum))                 /*电流采样正常*/
        && (realCurr10mA > sCurrZeroMax[clunum])
        && (realCurr10mA >= (s16)gGHardPara_104[eGHardPara104_DhgIntPoint])  /*当前电流符合积分条件*/
        && ((realCurr10mA >= (s16)gGHardPara_104[eGHardPara104_DhgShowPoint])/*当前电流符合显示条件*/
        || (sHisCurr10mA[clunum] >= (s16)gGHardPara_104[eGHardPara104_DhgIntPoint])))/*或上次电流符合积分条件(排除偶然跳动的电流)*/
    {
        sHisCurr10mA[clunum] = realCurr10mA;                                /*记录上次采样值*/

        /*未使能定时中断积分*/
        #if(0 == CURR_INT_ADD_EN)
        DhgCapIntTask(realCurr10mA, 1);
        DhgEnerIntTask(realCurr10mA, GetGSampSumVoltAPI(), 1);
        #endif
    }
    /*充电积分(正值)*/
    else if((0 == SampGetCurrSampExpFlagAPI(clunum))            /*电流采样正常*/
        && (realCurr10mA < sCurrZeroMin[clunum])
        && (realCurr10mA <= (0 - (s16)gGHardPara_104[eGHardPara104_ChgIntPoint]))  /*当前电流符合积分条件*/
        && ((realCurr10mA <= (0 - (s16)gGHardPara_104[eGHardPara104_ChgShowPoint]))/*当前电流符合显示条件*/
        || (sHisCurr10mA[clunum] <= (0 - (s16)gGHardPara_104[eGHardPara104_ChgIntPoint]))))/*或上次电流符合积分条件(排除偶然跳动的电流)*/
    {
        sHisCurr10mA[clunum] = realCurr10mA;                                /*记录上次采样值*/

        /*未使能定时中断积分*/
        #if(0 == CURR_INT_ADD_EN)
        ChgCapIntTask((0 - realCurr10mA), 1);
        ChgEnerIntTask((0 - realCurr10mA), GetGSampSumVoltAPI(), 1);
        #endif
    }
    else
    {
        sHisCurr10mA[clunum] = realCurr10mA;                        /*记录上次采样值*/

        if(0 == SampGetCurrSampExpFlagAPI(clunum))                  /*电流采样正常*/
        {
            realCurr10mA = 0;
        }
    }

	/*判断是否突变(本次采样超过极限值但上次未过流)*/
	if(((realCurr10mA >= ((s32)BATT_EXEREM_MAXC * 10))
		|| (realCurr10mA <= ((0 - (s32)BATT_EXEREM_MAXC) * 10)))
		&& (sRealCurr10mA[clunum] > (0 - (s32)gGBmuGenPara_102[eBmuGenPara102_ChgCH1Lim]))
		&& (sRealCurr10mA[clunum] < gGBmuGenPara_102[eBmuGenPara102_DhgCH1Lim]))
	{
		flag = 1;                                               /*电流突变则直接显示采样值(用于过流诊断)*/

		/*放电*/
		if(realCurr10mA >= ((s32)BATT_EXEREM_MAXC * 10))
		{
			/*放电过流一级告警故障录波*/
			//RcdSetFaultWaveTrigAPI(eFatWavSort_DCO1);
		}
		else
		{
			/*充电过流一级告警故障录波*/
			//RcdSetFaultWaveTrigAPI(eFatWavSort_CCO1);
		}
	}

	InputSampRealCurr10mA(clunum, realCurr10mA);                        /*输入电流采样实际电流值 0.01A*/
	//InputCSampRealVolt10uV(currVolt);                                 /*输入电流采样实际电压值 0.01mV*/

    /*显示电流采样值(包括滤波显示)*/
    if(realCurr10mA < 0)
    {
        CurrFilterDeal(clunum, (realCurr10mA - 5) / 10, flag);              /*输出电流单位0.1A*/
    }
    else
    {
        CurrFilterDeal(clunum, (realCurr10mA + 5) / 10, flag);              /*输出电流单位0.1A*/
    }
}
/*=============================================================================================
** @Name      : void ClrCurrZeroFinFlagAPI(u8 clunum)
** @Input     : clunum: 簇编号
** @Output    : void
** @Function  : 清除电流零点校零标志
** @The notes : 重新校零
**===========================================================================================*/
void ClrCurrZeroFinFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return;
    }
    sCurrZeroFin[clunum] = 0;
    sCurrZeroCorr[clunum] = 0;
    sHallZeroCorr[clunum] = 0;
}

/*=============================================================================================
** @Name      : void SetCurrSampExpFlagAPI(u8 clunum, u8 id, u8 state, s16 currAD)
** @Input     : clunum: 簇编号id:电流采样异常ID(0:霍尔供电5V异常 1:霍尔回检信号异常 2:霍尔ADC通道异常 3:霍尔AD值异常 4:分流器模拟前端异常 5:分流器AD值异常 6:电流校零异常 7:俩通道采样值误差异常 sate:设置状态[0:正常 1:异常]) currAD:读取到的AD值
** @Output    : void
** @Function  : 设置电流采样异常标志
** @The notes :
**===========================================================================================*/
void SetCurrSampExpFlagAPI(u8 clunum, u8 id, u8 state, s16 currAD)
{
    if(id >= eCExp_IDLen)
    {
        return;
    }
    if(clunum >= 2)
    {
        return;
    }

    /*设置异常*/
    if(1 == state)
    {
    	//新产生异常
    	if(0 == BitGet(sCSampExpFlag[clunum], id))
    	{
    		//记录异常
    		BitSet(sCSampExpFlag[clunum], id);

    		//俩通道误差异常
    		if(eCExp_HSDiff == id)
    		{
				//记录俩通道误差异常事件
    			//DiagUserSelfErrRecord(eUDef_ErrRcd_TWOCDiff, eERR_RCDSORT_WARSET, 0,  0);
    		}
    		//校零异常
    		else if(eCExp_ZroErr == id)
    		{
    			//未使能分流器
    			if(0 == BitGet(gGHardPara_104[eGHardPara104_CSampChan], eCAFE_Shunt))
    			{
    				//记录霍尔校零异常事件
    				//DiagUserSelfErrRecord(eUDef_ErrRcd_HallCErr, eERR_RCDSORT_WARSET, currAD,  0);
    			}
    			else
    			{
    				//记录分流器校零异常事件
    				//DiagUserSelfErrRecord(eUDef_ErrRcd_ShunCErr, eERR_RCDSORT_WARSET, currAD,  0);
    			}
    		}
    		//分流器采样异常
    		else if(id >= eCExp_AfeErr)
    		{
				//记录分流器采样异常事件
				//DiagUserSelfErrRecord(eUDef_ErrRcd_ShunCErr, eERR_RCDSORT_WARSET, currAD,  0);
    		}
    		//霍尔采样异常
    		else
    		{
				//记录霍尔采样异常事件
				//DiagUserSelfErrRecord(eUDef_ErrRcd_HallCErr, eERR_RCDSORT_WARSET, currAD,  0);
    		}
    	}

        /*设置电流校零失败*/
        sCurrZeroFin[clunum] = 2;
    }
    /*清除异常*/
    else if(0 == state)
    {
        BitClr(sCSampExpFlag[clunum], id);
    }
}

/*=============================================================================================
** @Name      : void ClrRemoteCurrSampExpFlagAPI(u8 clunum)
** @Input     : clunum: 簇编号
** @Output    : void
** @Function  : 清除由IMIU4告警帧维护的电流采样异常位
** @The notes : IMIU4离线后其最后一次告警值不能继续保持。AFE本地异常和校零异常不是
**              IMIU4报文来源，因此不得在这里清除。
**===========================================================================================*/
void ClrRemoteCurrSampExpFlagAPI(u8 clunum)
{
    if(clunum >= 2U)
    {
        return;
    }

    BitClr(sCSampExpFlag[clunum], eCExp_HalErr);
    BitClr(sCSampExpFlag[clunum], eCExp_HalOff);
    BitClr(sCSampExpFlag[clunum], eCExp_HADErr);
    BitClr(sCSampExpFlag[clunum], eCExp_SADErr);
    BitClr(sCSampExpFlag[clunum], eCExp_HSDiff);
    BitClr(sCSampExpFlag[clunum], eCExp_SefErr);
}

/*=============================================================================================
** @Name      : static void InputSampRealCurr10mA(u8 clunum, s32 curr)
** @Input     : clunum:簇编号 curr:实际电流 10mA
** @Output    : void
** @Function  : 输出记录电流采样实际电流值
** @The notes :
**===========================================================================================*/
static void InputSampRealCurr10mA(u8 clunum, s32 curr)
{
    if(clunum >= 2)
    {
        return;
    }
    sRealCurr10mA[clunum] = curr;
}

/*=============================================================================================
** @Name      : u8 SampGetCurrZeroFinFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电流采样零飘校准完成标志[0未校准 1校准成功 2校准失败]
** @Function  : 获取电流采样零飘校准完成标志
** @The notes :
**===========================================================================================*/
u8 SampGetCurrZeroFinFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return(sCurrZeroFin[clunum]);
}
/*=============================================================================================
** @Name      : u8 SampGetCurrSampExpFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 按位表示采样异常标志(bit0:霍尔供电5V异常 bit1:霍尔回检信号异常 bit2:霍尔ADC通道异常 bit3:霍尔AD值异常 bit4:分流器模拟前端采样异常 bit5:分流器AD值异常 bit6:电流校零异常 bit7:两通道采样误差异常[0正常 1异常])
** @Function  : 获取指定采样通道电流采样异常标志
** @The notes :
**===========================================================================================*/
u8 SampGetCurrSampExpFlagAPI(u8 clunum)
{
    if(clunum >= 2)
    {
        return (0);
    }
    return (sCSampExpFlag[clunum]);
}
/*=============================================================================================
** @Static function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : static void CalcCurrZeroCorr(u8 clunum, s32 curr)
** @Input     : clunum:簇编号 curr:空闲时采样电流零飘值 单位:1mA
** @Output    : void
** @Function  : 计算电流零飘值
** @The notes : 必须在高压接触器全断开(无电流)时调用
**===========================================================================================*/
static void CalcCurrZeroCorr(u8 clunum, s32 curr)
{
    u16 flag = 0;           //EEPROM标志
    s32 curZero = 0;        //零飘值 1mA
    static u16 num[2] = {0};     //积分次数
    static u16 time[2] = {0};    //误差次数
    static s32 curSum[2] = {0};  //零飘积分
    static s32 curMax[2] = {0};  //零飘最大值
    static s32 curMin[2] = {0};  //零飘最小值

    if(clunum >= 2)
    {
        return;
    }
    //电流校零完成
    if(1 == sCurrZeroFin[clunum])
    {
        //校零后误差仍超过0.3A则计算是否连续误差
        //if(ABS(curr, (sCurrZeroCorr * 10)) >= 300)
        if(ABS(curr, (sCurrZeroCorr[clunum] * 10)) >= 500)
        {
            time[clunum]++;
        }
        else
        {
            time[clunum] = 0;
        }

        //校零后误差仍超过0.5A则重新初始化和校零
        //if((ABS(curr, (sCurrZeroCorr * 10)) >= 500)
        if((ABS(curr, (sCurrZeroCorr[clunum] * 10)) >= 800)
            || (time[clunum] >= 50))
        {
            time[clunum] = 0;
            ///CurrSampleInit();
            //sCurrZeroFin = 0;
        }

        curSum[clunum] = 0;
        num[clunum] = 0;
        return;
    }

    //计算1s电流积分求平均
    if(num[clunum] < 50)
    {
        if((curr > curMax[clunum]) || (0 == num[clunum]))
        {
            curMax[clunum] = curr;
        }

        if((curr < curMin[clunum]) || (0 == num[clunum]))
        {
            curMin[clunum] = curr;
        }

        curSum[clunum] += curr;
        num[clunum]++;
    }
    else
    {
        curZero = curSum[clunum] / num[clunum];

        if(((0 - (s16)MAX_CURR_CORR_10MA) <= (curZero / 10))
            && ((curZero / 10) <= (s16)MAX_CURR_CORR_10MA)) //计算正确±10A
        {
            if(curZero < 0)                                 //负数
            {
                sCurrZeroCorr[clunum] = (s16)((curZero - 5) / 10);  //零飘值 10mA
            }
            else                                            //正数
            {
                sCurrZeroCorr[clunum] = (s16)((curZero + 5) / 10);  //零飘值 10mA
            }

            sCurrZeroMax[clunum] = (s16)((curMax[clunum] - curZero) / 10);  //零飘最大值 10mA
            sCurrZeroMin[clunum] = (s16)((curMin[clunum] - curZero) / 10);  //零飘最小值 10mA

            ParaWriteStoreCorrZeroCurr(clunum, 1, (u16*)&sCurrZeroCorr, 1);//修改EEP值
            flag = PARA_CFG_FLAG_U16;
            ParaWriteStoreCorrZeroCurr(clunum, 0, &flag, 1);        //修改EEP标志

            SetCurrSampExpFlagAPI(clunum, eCExp_ZroErr, 0, (curZero / 100));//清除电流校零异常标志
            sCurrZeroFin[clunum] = 1;                               //电流校零完成
        }
        else                                                //计算错误重新计算
        {
        	SetCurrSampExpFlagAPI(clunum, eCExp_ZroErr, 1, (curZero / 100));//记录电流校零异常标志
            sCurrZeroFin[clunum] = 2;                               //电流校零失败
        }

        curSum[clunum] = 0;                                         //零飘求和值
        curMax[clunum] = 0;                                         //零飘最大值
        curMin[clunum] = 0;                                         //零飘最小值
        time[clunum] = 0;
        num[clunum] = 0;
    }
}

/*=============================================================================================
** @Name      : u8 SampGetLowVSampExpFlagAPI(void)
** @Input     : void
** @Output    : 低压采样异常标志[0:正常 1:异常]
** @Function  : 获取硬件板低压侧采样异常标志
** @The notes : 836由BMU本板DEVADC提供ADC启动/原始值有效性；电源轨容差门限待硬件确认后另行增加。
**===========================================================================================*/
u8 SampGetLowVSampExpFlagAPI(void)
{
    return DEVADCGetSampExpFlagAPI();
}
