/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : EventRec.c
** @Founder      : WYC834
** @Date         : 2026.07.31
** @Function     : 电池事件记录处理模块
** @Instructions : 
**===============================================================================================*/

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "PortCheck.h"
#include "AuxDevTactic.h"
#include "DiagInterface.h"
#include "TacInterface.h"
#include "TacUser.h"
#include "EventRec.h"
#include "IccMsgInterface.h"
#include "DiagCheck.h"
#include <CLI.h>
#include <string.h>
#include <stdio.h>
#define CLUSTER_NUM 2 //总簇数
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/



extern u16 *gVerTime_0;                                 /*校时信息*/

extern u16 *gGHisEventInfo1_12;	                            /*组1历史事件信息*/
extern u16 *gGHisEventInfo2_13;								/*组2历史事件信息*/
extern u16 gGUserCtrl_87[];	    	                        /*用户控制命令信息数组*/
extern u16 *gSysCtrlCmd_65;      							/*组1系统控制命令*/
extern u16 *gSysCtrlCmd_66;       							/*组2系统控制命令*/
extern u16 gGBmuGenPara_102[];                              /*主控通用参数值*/
extern u8 gGHardPara_104[]; 			                    /*主控硬件参数配置信息*/

extern u16 *gDevSampInfo_14;                       /*板级采样信息*/
/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : static u8 EventRcdCheckRepeatedState(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum)
** @Input     : sort:事件类型 id:事件码ID info2-5:附加信息2-5  clunum:簇编号
** @Output    : TRUE:重复事件(不需记录) FALSE:非重复事件(需要记录)
** @Function  : 判断事件记录是否重复
** @The notes : 跟上次比较重复则不再记录
**===============================================================================================*/
static u8 EventRcdCheckRepeatedState(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum);

/*=================================================================================================
** @Name      : static void EventRecordAuxDevDeal(void)
** @Input     : void
** @Output    : void
** @Function  : 事件记录辅助设备处理任务(定周期检查辅助设备变化事件)
** @The notes : 100ms周期
**===============================================================================================*/
static void EventRecordAuxDevDeal(void);

/*=================================================================================================
** @Interface function definition
**===============================================================================================*/


/*=================================================================================================
** @Name      : void EventRecordUserTask(void)
** @Input     : void
** @Output    : void
** @Function  : 事件记录用户任务(定周期检查变化事件)
** @The notes : 100ms周期
**===============================================================================================*/
void EventRecordUserTask(void)
{
	static u8 sFlag = 0;
	static u8 sPwFlag[CLUSTER_NUM] = {0};
	static u8 sHisProState[CLUSTER_NUM] = {0};
	static u8 sHisChgState[CLUSTER_NUM] = {0};
	static u8 sHisModeState[CLUSTER_NUM] = {0};
	static u8 sHisWorkState[CLUSTER_NUM] = {0};
	static u16 sHisInPorState = 0;
	static u16 sHisOutPorState = 0;
	static u16 sHisMuxGpioOutState = 0;
	static u16 sHisMuxGpioInState_1 = 0;
	static u16 sHisMuxGpioInState_2 = 0;
	static u32 sHisOutClock = 0;
	static u32 sHisChgClock[CLUSTER_NUM] = {0};

	static u16 sHisChgCurr[CLUSTER_NUM] = {0};
	static u16 sHisDhgCurr[CLUSTER_NUM] = {0};

	// /*记录显示未初始化*/
    // if(0 == sFlag)
	// {
    // 	/*记录存储模块未初始化*/
    //     if(0 == RcdGetEventRcdEnStateAPI())
    // 	{
    //     	/*记录存储读写指针初始化*/
    //     	RcdStoreStateInit();
    // 	}
    //     else
    //     {
    //     	/*记录显示初始化*/
    //     	EventRcdDisplayInit();
    //     	sFlag = 1;
    //     }
    // 	return;
	// }
	for(int i = 0; i < CLUSTER_NUM; i++)
	{
		/*记录开机上电事件*/
		if(0 == sPwFlag[i])
		{
			/*第一簇产生开机上电事件*/
			EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_SysPwrOn, GetGSampOutCurrAPI(i), GetGSampSumVoltAPI(i), GetGSampPreVoltAPI(i), GetGBattSocAPI(i), i);
			sPwFlag[i] = 1;
		}

		/*保护状态变化*/
		if(sHisProState[i] != GetGProtectStateAPI(i))
		{
			/*产生事件*/
			EventRcdDisplayUserAPI(eEVN_RCDSORT_STATE, 0, GetGSampOutCurrAPI(i), sHisProState[i], GetGSampSumVoltAPI(i), GetGSampPreVoltAPI(i), i);
		}

		/*工作状态变化*/
		if(sHisWorkState[i] != GetGWorkStateAPI(i))
		{
			/*产生事件*/
			EventRcdDisplayUserAPI(eEVN_RCDSORT_STATE, 1, GetGSampOutCurrAPI(i), sHisWorkState[i], GetGSampSumVoltAPI(i), GetGSampPreVoltAPI(i), i);
		}

		/*充放电状态变化(至少间隔30min[防止非恒流反复记录])*/
		if((sHisChgState[i] != GetGChgDhgStateAPI(i)) && ((ABS(GetSysSecondTimeAPI(), sHisChgClock[i]) >= 1800) || ((ABS(GetSysSecondTimeAPI(), sHisChgClock[i]) >= 1200)
			&& ((GetGSampOutCurrAPI(i) >= 500) || (GetGSampOutCurrAPI(i) <= -500))) || ((ABS(GetSysSecondTimeAPI(), sHisChgClock[i]) >= 600) && (eCURR_IDLE != sHisChgState[i]) && (eCURR_IDLE != GetGChgDhgStateAPI(i)))))
		{
			/*产生事件*/
			EventRcdDisplayUserAPI(eEVN_RCDSORT_STATE, 2, GetGSampOutCurrAPI(i), sHisChgState[i], GetGChgDhgStateAPI(i), GetGSampPreVoltAPI(i), i);
			sHisChgState[i] = GetGChgDhgStateAPI(i);
			sHisChgClock[i] = GetSysSecondTimeAPI();
		}

		/*工作模式状态变化*/
		if(sHisModeState[i] != GetGWorkModeAPI(i))
		{
			/*产生事件*/
			EventRcdDisplayUserAPI(eEVN_RCDSORT_STATE, 3, GetGSampOutCurrAPI(i), sHisModeState[i], GetGWorkModeAPI(i), GetGSampPreVoltAPI(i), i);
		}

		sHisProState[i] = GetGProtectStateAPI(i);
		sHisWorkState[i] = GetGWorkStateAPI(i);
		sHisModeState[i] = GetGWorkModeAPI(i);

		/*非上电初始化状态*/
		if(eWORK_INIT != GetGWorkStateAPI(i))
		{
			/*充电电流限值变化*/
			if(((0 == sHisChgCurr[i]) && (0 != GetGChgLimCurrAPI(i)))
				|| ((0 != sHisChgCurr[i]) && (0 == GetGChgLimCurrAPI(i)))
				|| (ABS(sHisChgCurr[i], GetGChgLimCurrAPI(i)) >= 500))
			{
				/*产生事件*/
				EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_CLimChan, GetGSampOutCurrAPI(i), sHisChgCurr[i], GetGChgLimCurrAPI(i), GetGCellMaxVoltAPI(i), i);
				sHisChgCurr[i] = GetGChgLimCurrAPI(i);
			}

			/*放电电流限值变化*/
			if(((0 == sHisDhgCurr[i]) && (0 != GetGDhgLimCurrAPI(i)))
				|| ((0 != sHisDhgCurr[i]) && (0 == GetGDhgLimCurrAPI(i)))
				|| (ABS(sHisDhgCurr[i], GetGDhgLimCurrAPI(i)) >= 500))
			{
				/*产生事件*/
				EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_DLimChan, GetGSampOutCurrAPI(i), sHisDhgCurr[i], GetGDhgLimCurrAPI(i), GetGCellMinVoltAPI(i), i);
				sHisDhgCurr[i] = GetGDhgLimCurrAPI(i);
			}

		}
		/*初始化状态获取初值*/
		else
		{
			sHisChgCurr[i] = GetGChgLimCurrAPI(i);
			sHisDhgCurr[i] = GetGDhgLimCurrAPI(i);
		}
	}

	/*输出IO口变化*/
	if(sHisOutPorState != GetDevSampInfoAPI(eDevSamp_GpioOut))
	{
		/*产生事件*/
		EventRcdDisplayUserAPI(eEVN_RCDSORT_OUTPUT, 0, GetGSampOutCurrAPI(0), sHisOutPorState, GetDevSampInfoAPI(eDevSamp_GpioOut), GetGWorkModeAPI(0), 0);//以第一簇的接口发送
		//sHisOutPorState = GetDevSampInfoAPI(eDevSamp_GpioOut) & 0x7fff;
		sHisOutPorState = GetDevSampInfoAPI(eDevSamp_GpioOut);
	}

	/*输入IO口变化*/
	if(sHisInPorState != GetDevSampInfoAPI(eDevSamp_GpioIn))
	{
		/*产生事件*/
		EventRcdDisplayUserAPI(eEVN_RCDSORT_INPUT, 0, GetGSampOutCurrAPI(0), sHisInPorState, GetDevSampInfoAPI(eDevSamp_GpioIn), GetGWorkModeAPI(0), 0);//以第一簇的接口发送
		//sHisInPorState = GetDevSampInfoAPI(eDevSamp_GpioIn) & 0x7fff;
		sHisInPorState = GetDevSampInfoAPI(eDevSamp_GpioIn);
	}

	/*复用GPIO输出口变化*/
	if(sHisMuxGpioOutState != GetDevSampInfoAPI(eDevSamp_MuxGpioOut))
	{
		/*产生事件*/
		EventRcdDisplayUserAPI(eEVN_RCDSORT_MUXOUT, 0, GetGSampOutCurrAPI(0), sHisMuxGpioOutState, GetDevSampInfoAPI(eDevSamp_MuxGpioOut), GetGWorkModeAPI(0), 0);//以第一簇的接口发送
		//sHisMuxGpioOutState = GetDevSampInfoAPI(eDevSamp_MuxGpioOut) & 0x7fff;
		sHisMuxGpioOutState = GetDevSampInfoAPI(eDevSamp_MuxGpioOut);
	}

	/*复用GPIO输入口1变化*/
	if(sHisMuxGpioInState_1 != GetDevSampInfoAPI(eDevSamp_MuxGpioIn1))
	{
		/*产生事件*/
		EventRcdDisplayUserAPI(eEVN_RCDSORT_MUXIN1, 0, GetGSampOutCurrAPI(0), sHisMuxGpioInState_1, GetDevSampInfoAPI(eDevSamp_MuxGpioIn1), GetGWorkModeAPI(0), 0);//以第一簇的接口发送
		//sHisMuxGpioInState_1 = GetDevSampInfoAPI(eDevSamp_MuxGpioIn1) & 0x7fff;
		sHisMuxGpioInState_1 = GetDevSampInfoAPI(eDevSamp_MuxGpioIn1);
	}

	/*复用GPIO输入口2变化*/
	if(sHisMuxGpioInState_2 != GetDevSampInfoAPI(eDevSamp_MuxGpioIn2))
	{
		/*产生事件*/
		EventRcdDisplayUserAPI(eEVN_RCDSORT_MUXIN2, 0, GetGSampOutCurrAPI(0), sHisMuxGpioInState_2, GetDevSampInfoAPI(eDevSamp_MuxGpioIn2), GetGWorkModeAPI(0), 0);//以第一簇的接口发送
		//sHisMuxGpioInState_2 = GetDevSampInfoAPI(eDevSamp_MuxGpioIn2) & 0x7fff;
		sHisMuxGpioInState_2 = GetDevSampInfoAPI(eDevSamp_MuxGpioIn2);
	}

	//辅助设备事件记录
	EventRecordAuxDevDeal();


}

/*=================================================================================================
** @Name      : void EventRcdDisplayUserAPI(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum)
** @Input     : sort:事件类型 id:事件码ID info2-5:附加信息2-5  clunum:簇编号
** @Output    : void
** @Function  : 事件记录与显示用户接口
** @The notes : 用户需产生事件的地方调用,事件发送到A核并记录存储
**===============================================================================================*/
void EventRcdDisplayUserAPI(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum)
{
    t_SysClock sysClock = {0};
    t_RcdCreateInfo rcdInfo = {0};


    /*不记录连续重复的事件*/
    if(TRUE == EventRcdCheckRepeatedState(sort, id, info2, info3, info4, info5, clunum))
    {
        return;
    }

    /*第一簇*/
    if(0 == clunum)
    {
		//记录新历史事件记录
		//记录类型
		gGHisEventInfo1_12[0] = sort;

		//输入IO类型
		if(eEVN_RCDSORT_INPUT == sort)
		{
			gGHisEventInfo1_12[1] = EVN_RCD_INIOCODE_FST + id;     	//事件记录码
		}
		//输出IO类型
		else if(eEVN_RCDSORT_OUTPUT == sort)
		{
			gGHisEventInfo1_12[1] = EVN_RCD_OUTIOCODE_FST + id;     	//事件记录码
		}
		//状态类型
		else if(eEVN_RCDSORT_STATE == sort)
		{
			gGHisEventInfo1_12[1] = EVN_RCD_STATECODE_FST + id;     	//事件记录码
		}
		//控制命令类型
		else if(eEVN_RCDSORT_CMD == sort)
		{
			gGHisEventInfo1_12[1] = EVN_RCD_CMDCODE_FST + id;    	//事件记录码
		}
		//调试命令类型
		else if(eEVN_RCDSORT_DEBUG == sort)
		{
			gGHisEventInfo1_12[1] = EVN_RCD_DBGCODE_FST + id;    	//事件记录码
		}
		//参数配置类型
		else if(eEVN_RCDSORT_CFG == sort)
		{
			gGHisEventInfo1_12[1] = EVN_RCD_CFGCODE_FST + id;    	//事件记录码
		}
		//用户自定义类型
		else
		{
			gGHisEventInfo1_12[1] = id;                          	//事件记录码
		}

		/*记录时间信息*/
		sysClock = GetSysDateClockAPI();

		gGHisEventInfo1_12[2] = sysClock.year;                                 //记录时间-年
		gGHisEventInfo1_12[3] = ((u16)sysClock.month << 8) | sysClock.day;     //记录时间-月日
		gGHisEventInfo1_12[4] = ((u16)sysClock.hour << 8) | sysClock.minute;   //记录时间-时分
		gGHisEventInfo1_12[5] = sysClock.second;                               //记录时间-秒

		//事件附件信息
		gGHisEventInfo1_12[6] = ((u16)GetGProtectStateAPI(clunum) << 8) | (GetGWorkStateAPI(clunum) & 0x00ff);
		gGHisEventInfo1_12[7] = info2;
		gGHisEventInfo1_12[8] = info3;
		gGHisEventInfo1_12[9] = info4;
		gGHisEventInfo1_12[10] = info5;


    }else if(1 == clunum)//第二簇
	{
		//记录新历史事件记录
		//记录类型
		gGHisEventInfo2_13[0] = sort;

		//输入IO类型
		if(eEVN_RCDSORT_INPUT == sort)
		{
			gGHisEventInfo2_13[1] = EVN_RCD_INIOCODE_FST + id;     	//事件记录码
		}
		//输出IO类型
		else if(eEVN_RCDSORT_OUTPUT == sort)
		{
			gGHisEventInfo2_13[1] = EVN_RCD_OUTIOCODE_FST + id;     	//事件记录码
		}
		//状态类型
		else if(eEVN_RCDSORT_STATE == sort)
		{
			gGHisEventInfo2_13[1] = EVN_RCD_STATECODE_FST + id;     	//事件记录码
		}
		//控制命令类型
		else if(eEVN_RCDSORT_CMD == sort)
		{
			gGHisEventInfo2_13[1] = EVN_RCD_CMDCODE_FST + id;    	//事件记录码
		}
		//调试命令类型
		else if(eEVN_RCDSORT_DEBUG == sort)
		{
			gGHisEventInfo2_13[1] = EVN_RCD_DBGCODE_FST + id;    	//事件记录码
		}
		//参数配置类型
		else if(eEVN_RCDSORT_CFG == sort)
		{
			gGHisEventInfo2_13[1] = EVN_RCD_CFGCODE_FST + id;    	//事件记录码
		}
		//用户自定义类型
		else
		{
			gGHisEventInfo2_13[1] = id;                          	//事件记录码
		}

		/*记录时间信息*/
		sysClock = GetSysDateClockAPI();

		gGHisEventInfo2_13[2] = sysClock.year;                                 //记录时间-年
		gGHisEventInfo2_13[3] = ((u16)sysClock.month << 8) | sysClock.day;     //记录时间-月日
		gGHisEventInfo2_13[4] = ((u16)sysClock.hour << 8) | sysClock.minute;   //记录时间-时分
		gGHisEventInfo2_13[5] = sysClock.second;                               //记录时间-秒

		//事件附件信息
		gGHisEventInfo2_13[6] = ((u16)GetGProtectStateAPI(clunum) << 8) | (GetGWorkStateAPI(clunum) & 0x00ff);
		gGHisEventInfo2_13[7] = info2;
		gGHisEventInfo2_13[8] = info3;
		gGHisEventInfo2_13[9] = info4;
		gGHisEventInfo2_13[10] = info5;	
	}
	/*触发发送历史事件信息*/
	SendIccGroupHisEventInfoMsg(clunum);

}

/*=================================================================================================
** @Static function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : static void EventRecordAuxDevDeal(void)
** @Input     : void
** @Output    : void
** @Function  : 事件记录辅助设备处理任务(定周期检查辅助设备变化事件)
** @The notes : 100ms周期
**===============================================================================================*/
static void EventRecordAuxDevDeal(void)
{
	u16 info3 = 0;
	u16 info4 = 0;
	u16 info5 = 0;
	static u8 FirstPowerOnFlag = 0;
	static u8 sHisTempState = 0;//按位表示历史值是否变化到异常值 bit0:无 bit1:柜内温度 bit2:柜外温度 bit3:柜内湿度 bit4:气体1浓度 bit5:气体2浓度
	static u8 sHisDoorState = 0;//门禁状态 0:关闭 1:开启

	#if(1 == BMU_EU_SHUTFAN_EN) //使能控制电池柜百叶风机
	static u8 sHisFanState = 0;//电池柜风机状态 0:关闭 1:开启
	#endif
	static u8 sHisDeState = 0;//电池柜除湿机机状态 0:关闭 1:开启

	for(int i = 0; i < CLUSTER_NUM; i++)
	{
		/*非上电初始化状态*/
		if((eWORK_INIT != GetGWorkStateAPI(i))
			&& (BSPGetOSMillSecTimeAPI() >= 10000 || FirstPowerOnFlag == 1))
		{
			FirstPowerOnFlag = 1;
			//记录温度
			info4 = ((u16)GetGPerInsidMaxTempAPI(i) << 8) | GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeTem);

			//记录湿度
			info5 = ((u16)GetGPerInsidMaxHumAPI(i) << 8) | GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeHum);

			/*门禁状态变化*/
			if(sHisDoorState != BitGet(GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16), eIMEUBoardInfoDI_Door))
			{
				/*产生事件*/
				EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_DoorDI, GetGSampOutCurrAPI(i), BitGet(GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16), eIMEUBoardInfoDI_Door), info4, info5, i);
				//记录门禁状态
				sHisDoorState = BitGet(GetImeu3BoardInfo2API(eIMEUBoardInfo2_DI1_16), eIMEUBoardInfoDI_Door);
			}
			//检测柜内温度是否达到过温温度即记录变化事件
			if(GetGPerInsidMaxTempAPI(i) >= gGBmuGenPara_102[eBmuGenPara102_Envir1Lim])
			{
				//记录温度
				info3 = ((u16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc1Tem) << 8) | GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc2Tem);
				info4 = ((u16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc3Tem) << 8) | GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc4Tem);
				info5 = ((u16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc5Tem) << 8) | GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc6Tem);

				//未记录
				if(0 == BitGet(sHisTempState, 2))
				{
					/*产生事件*/
					EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_ExtTChan, GetGSampOutCurrAPI(i), info3, info4, info5, i);
					BitSet(sHisTempState, 2);
				}
			}
			else if(GetGPerInsidMaxTempAPI(i) <= ((s8)gGBmuGenPara_102[eBmuGenPara102_Envir1Lim] - gGBmuGenPara_102[eBmuGenPara102_Envir1Clr]))
			{
				BitClr(sHisTempState, 2);
			}

		}
		/*初始化状态获取初值*/
		else
		{
			//温湿度浓度历史状态
			sHisTempState = 0;
		}

	}

	//不用分簇的信息记录,全部归为第一簇信息
	/*IMEU3通讯和和除湿机工作均正常*/
	if(0 == DiagGetInImeu3OffCommuStateAPI())
	{
		//记录除湿机工作状态变更
		if(sHisDeState != GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeState))
		{
			/*产生事件*/
			EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_DeMode, GetGSampOutCurrAPI(0), GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeState), sHisDeState, 0, 0);
			//记录除湿机工作状态
			sHisDeState = GetImeu3BoardInfo1API(eIMEUBoardInfo1_DeState);
		}



		#if(1 == BMU_EU_SHUTFAN_EN)
		//开电池柜风机
		if(GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM1) > 0 || GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM2) > 0 ||
			GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM3) > 0 ||GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM4) > 0 ||
			GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM5) > 0 ||GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM6) > 0)
		{
			//未记录
			if(1 != sHisFanState)
			{
				info3 = ((u16)GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM1) << 8) | GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM2);
				info4 = ((u16)GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM3) << 8) | GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM4);
				info5 = ((u16)GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM5) << 8) | GetImeu3BoardInfo2API(eIMEUBoardInfo2_PWM6);
				/*产生事件*/
				EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_FanChan, GetGSampOutCurrAPI(0), info3, info4, info5, 0);
				sHisFanState = 1;
			}
		}
		//关电池柜风机
		else
		{
			//未记录
			if(0 != sHisFanState)
			{
				/*产生事件*/
				EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_FanChan, GetGSampOutCurrAPI(0), info3, info4, info5, 0);
				sHisFanState = 0;
			}
		}
		#endif




		//检测柜内可燃气体1是否达到告警浓度即记录变化事件
		if((GetImeu3BoardInfo2API(eIMEUBoardInfo2_H2Raw) >= BMU_EU_GAS1_CONC) || (GetImeu3BoardInfo2API(eIMEUBoardInfo2_CORaw) >= BMU_EU_GAS2_CONC))
		{
			//记录可燃气体浓度
			info3 = GetImeu3BoardInfo2API(eIMEUBoardInfo2_H2Raw);
			info4 = GetImeu3BoardInfo2API(eIMEUBoardInfo2_CORaw);
			info5 = GetGPerInsidMaxTempAPI(0);//只记录第一簇环境最高温度

			//未记录
			if(0 == BitGet(sHisTempState, 4))
			{
				/*产生事件*/
				EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, eEvn_User_GasChan, GetGSampOutCurrAPI(0), info3, info4, info5, 0);
				BitSet(sHisTempState, 4);
			}
		}
		else if(GetImeu3BoardInfo2API(eIMEUBoardInfo2_H2Raw) <= (BMU_EU_GAS1_CONC / 2) && GetImeu3BoardInfo2API(eIMEUBoardInfo2_CORaw) <= (BMU_EU_GAS2_CONC / 2))
		{
			BitClr(sHisTempState, 4);
		}
		
	}

}

/*=================================================================================================
** @Name      : static u8 EventRcdCheckRepeatedState(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum)
** @Input     : sort:事件类型 id:事件码ID info2-5:附加信息2-5  clunum:簇编号
** @Output    : TRUE:重复事件(不需记录) FALSE:非重复事件(需要记录)
** @Function  : 判断事件记录是否重复
** @The notes : 跟上次比较重复则不再记录
**===============================================================================================*/
static u8 EventRcdCheckRepeatedState(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum)
{
    static u16 sHisAuth[3] = {0};
    static u16 sHisStat[3] = {0};
    static u16 sHisMode[3] = {0};
    static u16 sHisClr[3] = {0};
    static u16 sHisShut[3] = {0};
    static u16 sHisSDbg[3] = {0};
    static u16 sHisMDbg[3] = {0};
    static u16 sHisPara[5] = {0};
    static u16 sHisInfo[6] = {0};

    /*外部触发的事件(控制)单独处理*/
    if(eEVN_RCDSORT_CMD == sort)
    {
    	//权限切换控制
    	if(0 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisAuth[0]) && (info4 == sHisAuth[1]) && (info5 == sHisAuth[2]))
    		{
    			return(TRUE);
    		}

    		sHisAuth[0] = info3;
    		sHisAuth[1] = info4;
    		sHisAuth[2] = info5;
    	}
    	//工作状态切换控制
    	else if(1 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisStat[0]) && (info4 == sHisStat[1]) && (info5 == sHisStat[2]))
    		{
    			return(TRUE);
    		}

    		//空闲状态非启动命令不记录
    		if((eWORK_IDLE == GetGWorkStateAPI(clunum)) && (eCTRL_PRECHG != info4) && (eCTRL_NORSTART != info4)
    			&& (eCTRL_CHGSTART != info4) && (eCTRL_DHGSTART != info4))
    		{
    			return(TRUE);
    		}
    		//运行状态非停机命令不记录
    		else if((eWORK_RUN == GetGWorkStateAPI(clunum)) && (eCTRL_NORSTOP != info4) && (eCTRL_FORCSTOP != info4)
    			&& (eCTRL_EMERSTOP != info4) && (eCTRL_POWEROFF != info4))
    		{
    			return(TRUE);
    		}
    		//故障状态启动命令不记录
    		else if((eWORK_ERR == GetGWorkStateAPI(clunum)) && ((eCTRL_PRECHG == info4) || (eCTRL_NORSTART == info4)
        		|| (eCTRL_CHGSTART == info4) || (eCTRL_DHGSTART == info4)))
    		{
    			return(TRUE);
    		}
    		//故障状态接触器全断开时停止命令不记录
    		else if((eWORK_ERR == GetGWorkStateAPI(clunum)) && (TRUE == TacGetGroupRunSwhAllOffStateAPI(clunum)) && ((eCTRL_NORSTOP == info4)
    			|| (eCTRL_FORCSTOP == info4) || (eCTRL_EMERSTOP == info4) || (eCTRL_POWEROFF == info4)))
    		{
    			return(TRUE);
    		}

    		sHisStat[0] = info3;
    		sHisStat[1] = info4;
    		sHisStat[2] = info5;
    	}
    	//工作模式切换控制
    	else if(2 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisMode[0]) && (info4 == sHisMode[1]) && (info5 == sHisMode[2]))
    		{
    			return(TRUE);
    		}

    		sHisMode[0] = info3;
    		sHisMode[1] = info4;
    		sHisMode[2] = info5;
    	}
    	//系统清除命令控制
    	else if(3 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisClr[0]) && (info4 == sHisClr[1]) && (info5 == sHisClr[2]))
    		{
    			return(TRUE);
    		}

    		sHisClr[0] = info3;
    		sHisClr[1] = info4;
    		sHisClr[2] = info5;
    	}
    	//百叶控制
    	else if(12 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisShut[0]) && (info4 == sHisShut[1]) && (info5 == sHisShut[2]))
    		{
    			return(TRUE);
    		}

    		sHisShut[0] = info3;
    		sHisShut[1] = info4;
    		sHisShut[2] = info5;
    	}
    	//其它控制
    	else
    	{
    		//不记录
    		//return(TRUE);

            /*所有信息均一样才不记录(类型/序号/电流/其它信息)*/
            if((sort == sHisInfo[0]) && (id == sHisInfo[1]) /*&& (info2 == sHisInfo[2])*/
                && (info3 == sHisInfo[3]) && (info4 == sHisInfo[4]) && (info5 == sHisInfo[5]))
            {
                return(TRUE);
            }

    		/*更新上次记录信息*/
    		sHisInfo[0] = sort;
    		sHisInfo[1] = id;
    		sHisInfo[2] = info2;
    		sHisInfo[3] = info3;
    		sHisInfo[4] = info4;
    		sHisInfo[5] = info5;
    	}
    }
    /*外部触发的事件(调试)单独处理*/
    else if(eEVN_RCDSORT_DEBUG == sort)
    {
    	//从控调试
    	if(0 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisSDbg[0]) && (info4 == sHisSDbg[1]) && (info5 == sHisSDbg[2]))
    		{
    			return(TRUE);
    		}

    		sHisSDbg[0] = info3;
    		sHisSDbg[1] = info4;
    		sHisSDbg[2] = info5;
    	}
    	//主控调试
    	else if(1 == id)
    	{
    		//信息完全一样不记录
    		if((info3 == sHisMDbg[0]) && (info4 == sHisMDbg[1]) && (info5 == sHisMDbg[2]))
    		{
    			return(TRUE);
    		}

    		sHisMDbg[0] = info3;
    		sHisMDbg[1] = info4;
    		sHisMDbg[2] = info5;
    	}
    	//其它调试
    	else
    	{
    		//不记录
    		//return(TRUE);

            /*所有信息均一样才不记录(类型/序号/电流/其它信息)*/
            if((sort == sHisInfo[0]) && (id == sHisInfo[1]) /*&& (info2 == sHisInfo[2])*/
                && (info3 == sHisInfo[3]) && (info4 == sHisInfo[4]) && (info5 == sHisInfo[5]))
            {
                return(TRUE);
            }

    		/*更新上次记录信息*/
    		sHisInfo[0] = sort;
    		sHisInfo[1] = id;
    		sHisInfo[2] = info2;
    		sHisInfo[3] = info3;
    		sHisInfo[4] = info4;
    		sHisInfo[5] = info5;
        }
    }
    /*外部触发的事件(参数配置)单独处理*/
    else if(eEVN_RCDSORT_CFG == sort)
    {
		//信息完全一样不记录
		if((id == sHisPara[0]) && (info3 == sHisPara[1]) && (info4 == sHisPara[2]) && (info5 == sHisPara[3]))
		{
			return(TRUE);
		}

		sHisPara[0] = id;
		sHisPara[1] = info3;
		sHisPara[2] = info4;
		sHisPara[3] = info5;
    }
    /*其它事件统一处理*/
    else
    {
        /*所有信息均一样才不记录(类型/序号/电流/其它信息)*/
        if((sort == sHisInfo[0]) && (id == sHisInfo[1]) /*&& (info2 == sHisInfo[2])*/
            && (info3 == sHisInfo[3]) && (info4 == sHisInfo[4]) && (info5 == sHisInfo[5]))
        {
            return(TRUE);
        }

		/*更新上次记录信息*/
		sHisInfo[0] = sort;
		sHisInfo[1] = id;
		sHisInfo[2] = info2;
		sHisInfo[3] = info3;
		sHisInfo[4] = info4;
		sHisInfo[5] = info5;
    }

    return(FALSE);
}
static int diag_test2(int argc, char *argv[])
{
    if (argc == 0) {
        printf("diag test para error\n");
    }
    if (!strcmp(argv[0], "0"))
    {
		EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, 1, 1, 1, 1, 1, 0);
    }else if(!strcmp(argv[0], "1"))
	{
		EventRcdDisplayUserAPI(eEVN_RCDSORT_USER100, 1, 1, 1, 1, 1, 1);
	}
    else
    {
        printf("the parameters cannot be recognized\n");
       
    }
	 return 0;
}
CLI_CMD("diag_test2",
        "diag_test2"
        "\r\n\t diag_test2 0/1",
        diag_test2);