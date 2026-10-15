/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupPara.h
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 电池组参数模块头文件
** @Instructions : 
**===============================================================================================*/
#ifndef __GROUPPARA_H__
#define __GROUPPARA_H__
#include "BSPTypeDef.h"
#include "GroupInfo.h"
#include "ParaInterface.h"
#define GINDEX102_LEN			(eBmuGenPara102_CfgFin)		/*主控通用参数值数组长度*/
#define GINDEX103_LEN			(eBmuHigLevPara103_CfgFin)	/*主控高级参数值数组长度*/
#define GINDEX104_LEN			(eGHardPara104_Fin)			/*主控硬件参数值数组长度*/

#define GINDEX107_LEN			(eBmuGenPara102_CfgFin)		/*主控通用参数值数组长度*/
#define GINDEX108_LEN			(eBmuHigLevPara103_CfgFin)	/*主控高级参数值数组长度*/
#define GINDEX109_LEN			(eGHardPara104_Fin)			/*主控硬件参数值数组长度*/

/*********************************参数配置值范围**************************************************/
#define MIN_BMU_ADDR_CFG                (50)                /*主控设备地址最小配置值*/
#define MAX_BMU_ADDR_CFG                (65)        		/*主控设备地址最大配置值*/

#define MAX_CURR_CORR_10MA              (5000)              /*电流零飘修正最大值10mA*/

#define MAX_ALL_CAP_CFG                 (500000)            /*总容量最大配置值mAH*/
#define MAX_ALL_ENER_CFG                (500000)            /*总能量最大配置值WH*/

#define MIN_CLUSTER_NUM_CFG             (1)                 /*簇个数最小配置值*/
#define MAX_CLUSTER_NUM_CFG             (2)                 /*簇个数最大配置值*/

#define MIN_BSU_NUM_CFG                 (1)                 /*从控个数最小配置值*/
#define MAX_BSU_NUM_CFG                 (SLAVE_MAX_NUM)     /*从控个数最大配置值*/

#define MIN_BSU_CONNUM_CFG              (1)                 /*从机包含电池箱(从控)数最小配置值*/
#define MAX_BSU_CONNUM_CFG              (30)                /*从机包含电池箱(从控)数最大配置值*/
#define MIN_BSU_CELVNUM_CFG             (1)                 /*从控单体电压采样节数最小配置值*/
#define MAX_BSU_CELVNUM_CFG             (SLAVE_MAX_CELVNUM) /*从控单体电压采样节数最大配置值*/

#define MIN_BSU_CELTNUM_CFG             (1)                 /*从控单体温度采样节数最小配置值*/
#define MAX_BSU_CELTNUM_CFG             (SLAVE_MAX_CELTNUM) /*从控单体温度采样节数最大配置值*/
#define MIN_BSU_BALTNUM_CFG             (0)                 /*从控均衡板温采样节数最小配置值*/
#define MAX_BSU_BALTNUM_CFG             (SLAVE_MAX_BALTNUM) /*从控均衡板温采样节数最大配置值*/

#define MIN_BMU_CELVNUM_CFG             (1)                 /*主控单体电压采样节数最小配置值*/
#define MAX_BMU_CELVNUM_CFG             (GROUP_MAX_CELVNUM) /*主控单体电压采样节数最大配置值*/

#define MIN_BMU_CELTNUM_CFG             (1)                 /*主控单体温度采样总节数最小配置值*/
#define MAX_BMU_CELTNUM_CFG             (GROUP_MAX_CELTNUM) /*主控单体温度采样总节数最大配置值*/
#define MAX_BMU_BALTNUM_CFG             (GROUP_MAX_BALTNUM) /*主控均衡板温采样总节数最大配置值*/

#define MIN_VOLT_H_LIM                  (2000)              /*单体高压最小配置值*/
#define MAX_VOLT_H_LIM                  (5000)              /*单体高压最大配置值*/
#define MIN_VOLT_L_LIM                  (1000)              /*单体低压最小配置值*/
#define MAX_VOLT_L_LIM                  (4000)              /*单体低压最大配置值*/

#define MIN_SUMV_H_LIM                  (100)               /*总压高压最小配置值*/
#define MAX_SUMV_H_LIM                  (16000)             /*总压高压最大配置值*/
#define MIN_SUMV_L_LIM                  (100)               /*总压低压最小配置值*/
#define MAX_SUMV_L_LIM                  (16000)             /*总压低压最大配置值*/

#define MIN_CURR_H_LIM                  (10)                /*过流最小配置值*/
#define MAX_CURR_H_LIM                  (10000)             /*过流最大配置值*/

#define MIN_TEMP_H_LIM                  (20)                /*高温最小配置值*/
#define MAX_TEMP_H_LIM                  (80)                /*高温最大配置值*/
#define MIN_TEMP_L_LIM                  (-40)               /*低温最小配置值*/
#define MAX_TEMP_L_LIM                  (30)                /*低温最大配置值*/

#define MIN_PERT_H_LIM                  (10)                /*外围高温最小配置值*/
#define MAX_PERT_H_LIM                  (200)               /*外围高温最大配置值*/

#define MIN_POWER_H_LIM                 (1)                 /*请求功率最小配置值*/
#define MAX_POWER_H_LIM                 (5000)              /*请求功率最大配置值*/

#define MIN_ALLOW_CLIM_V                (2000)              /*释放充电电流限值电压最小配置值*/
#define MAX_ALLOW_CLIM_V                (5000)              /*释放充电电流限值电压最大配置值*/
#define MIN_ALLOW_DLIM_V                (1000)              /*释放放电电流限值电压最小配置值*/
#define MAX_ALLOW_DLIM_V                (4000)              /*释放放电电流限值电压最大配置值*/

#define MIN_CURR_OLIM_PER               (50)                /*电流超限告警比率最小配置值*/
#define MAX_CURR_OLIM_PER               (500)               /*电流超限告警比率最大配置值*/

#define MAX_ALM_CLR_SUB_CV              (1000)              /*单体电压告警释放回差取值最大配置值*/
#define MAX_ALM_CLR_SUB_SV              (1000)              /*总压电压告警释放回差取值最大配置值*/
#define MAX_ALM_CLR_SUB_C               (1000)              /*过流告警释放回差取值最大配置值*/
#define MAX_ALM_CLR_SUB_T               (60)                /*温度告警释放回差取值最大配置值*/

#define MAX_PRO_TIME_1S                 (6000)              /*故障保护时间最大配置值*/

/*协议102子索引列表*/
typedef enum
{
	eBmuGenPara102_CfgFlag = 0,								/*系统通用参数配置标志	23130*/
	eBmuGenPara102_BmuDev = 1,								/*设备地址[50,66]*/
	eBmuGenPara102_1NowSoc = 2,								/*组1当前SOC	0.10%	[0,1000]*/
	eBmuGenPara102_2NowSoc = 3,								/*组2当前SOC	0.10%	[0,1000]*/
	eBmuGenPara102_1NowSoe = 4,								/*组1当前SOE	0.10%	[0,1000]*/
	eBmuGenPara102_2NowSoe = 5,								/*组2当前SOE	0.10%	[0,1000]*/
	eBmuGenPara102_1ReaAllCap = 6,							/*组1实际总容量0.1AH	[1,5000]*/
	eBmuGenPara102_2ReaAllCap = 7,							/*组2实际总容量0.1AH	[1,5000]*/
	eBmuGenPara102_1ReaAllEner = 8,							/*组1实际总电能0.1kWh	[1,5000]*/
	eBmuGenPara102_2ReaAllEner = 9,							/*组2实际总电能0.1kWh	[1,5000]*/
	eBmuGenPara102_RateCap = 10,						    /*出厂额定容量0.1AH	[1,5000]*/
	eBmuGenPara102_DampNum = 11,							/*出厂衰减次数[1,15000]*/

	eBmuGenPara102_VH2Lim = 12,								/*单体高压二级告警限值[2000,5000]	*/
	eBmuGenPara102_VH1Lim = 13,								/*单体高压一级保护限值1mv [2000,5000]*/

	eBmuGenPara102_VL2Lim = 14,								/*单体低压二级告警限值1mv	 [1000,4000]*/
	eBmuGenPara102_VL1Lim = 15,								/*单体低压一级保护限值1mv	 [1000,4000]*/

	eBmuGenPara102_ChgTH2Lim = 16,							/*充电高温二级告警限值1℃	 [20,80]*/
	eBmuGenPara102_ChgTH1Lim = 17,							/*充电高温一级保护限值1℃	 [20,80]*/
	eBmuGenPara102_DhgTH2Lim = 18,							/*放电高温二级告警限值1℃	 [20,80]*/
	eBmuGenPara102_DhgTH1Lim = 19,							/*放电高温一级保护限值1℃	 [20,80]*/
	eBmuGenPara102_ChgTL2Lim = 20,							/*充电低温二级告警限值1℃	 [-40,30]*/
	eBmuGenPara102_ChgTL1Lim = 21,							/*充电低温一级保护限值1℃	 [-40,30]*/
	eBmuGenPara102_DhgTL2Lim = 22,							/*放电低温二级告警限值1℃	 [-40,30]*/
	eBmuGenPara102_DhgTL1Lim = 23,							/*放电低温一级保护限值1℃	 [-40,30]*/

	eBmuGenPara102_SumVH2Lim = 24,							/*总压高压二级告警限值0.1V [100,10000]*/
	eBmuGenPara102_SumVH1Lim = 25,							/*总压高压一级保护限值0.1V [100,10000]*/
	eBmuGenPara102_SumVL2Lim = 26,							/*总压低压二级告警限值0.1V [100,10000]*/
	eBmuGenPara102_SumVL1Lim = 27,							/*总压低压一级保护限值0.1V [100,10000]*/

	eBmuGenPara102_ChgCH2Lim = 28,							/*充电过流二级告警限值0.1A [10,10000]*/
	eBmuGenPara102_ChgCH1Lim = 29,							/*充电过流一级保护限值0.1A [10,10000]*/
	eBmuGenPara102_DhgCH2Lim = 30,							/*放电过流二级告警限值0.1A [10,10000]*/
	eBmuGenPara102_DhgCH1Lim = 31,							/*放电过流一级保护限值0.1A [10,10000]*/

	eBmuGenPara102_ChgCMaxLim = 32,							/*充电最大请求电流值0.1A	[10,10000]*/
	eBmuGenPara102_DhgCMaxLim = 33,							/*放电最大请求电流值0.1A	[10,10000]*/
	eBmuGenPara102_ChgCFinLim = 34,							/*充电截止电流值0.1A	[10,10000]*/
	eBmuGenPara102_DhgCFinLim = 35,							/*放电截止电流值0.1A	[10,10000]*/
	eBmuGenPara102_ChgPMaxLim = 36,							/*充电最大请求功率值0.1kW	 [1,5000*/
	eBmuGenPara102_DhgPMaxLim = 37,							/*放电最大请求功率值0.1kW	 [1,5000]*/
	eBmuGenPara102_ChgPRatLim = 38,							/*充电输入额定功率值0.1kW	 [1,5000]*/
	eBmuGenPara102_DhgPRatLim = 39,							/*放电输出额定功率值0.1kW	 [1,5000]*/

	eBmuGenPara102_Leak2Lim = 40,							/*绝缘二级告警限值1Ω/V	[1,1000]*/
	eBmuGenPara102_Leak1Lim = 41,							/*绝缘一级保护限值1Ω/V	[1,1000]*/

	eBmuGenPara102_Envir2Lim = 42,							/*环境温度高温二级告警限值1℃ [10,200]*/
	eBmuGenPara102_Envir1Lim = 43,							/*环境温度高温一级保护限值1℃ [10,200]*/
	eBmuGenPara102_Perip2Lim = 44,							/*外设温度高温二级告警限值1℃ [10,200]*/
	eBmuGenPara102_Perip1Lim = 45,							/*外设温度高温一级保护限值1℃ [10,200]*/

	eBmuGenPara102_VDiff2SubV = 46,							/*单体压差过大二级限值1mv	 [20,5000]*/
	eBmuGenPara102_VDiff1SubV = 47,							/*单体压差过大一级限值1mv	 [20,5000]*/
	eBmuGenPara102_TDiff2SubT = 48,							/*单体温差过大二级限值1℃	 [3,60]*/
	eBmuGenPara102_TDiff1SubT = 49,							/*单体温差过大一级限值1℃	 [3,60]*/

	eBmuGenPara102_CapHig2Soc = 50,							/*电池充电容量高二级限值0.10% [500,1000]*/
	eBmuGenPara102_CapHig1Soc = 51,							/*电池充电容量高一级限值0.10%	 [500,1000]*/
	eBmuGenPara102_CapLow2Soc = 52,							/*电池剩余容量低二级限值0.10%	 [0,500]*/
	eBmuGenPara102_CapLow1Soc = 53,							/*电池剩余容量低一级限值0.10%	 [0,500]*/

	eBmuGenPara102_CapHig1Volt = 54,						/*电池容量高一级预警对应电压值1mv	[2000,5000]*/
	eBmuGenPara102_CapLow1Volt = 55,						/*电池容量低一级预警对应电压值1mv	[1000,4000]*/

	eBmuGenPara102_CVH2Clr = 56,							/*单体高压二级消除回差值1mv [1,1000]*/
	eBmuGenPara102_CVH1Clr = 57,							/*单体高压一级消除回差值1mv [1,1000]*/
	eBmuGenPara102_CVL2Clr = 58,							/*单体低压二级消除回差值1mv [1,1000]*/
	eBmuGenPara102_CVL1Clr = 59,							/*单体低压一级消除回差值1mv [1,1000]*/

	eBmuGenPara102_CTH2Clr = 60,							/*充电高温二级消除回差值1℃ [1,80]*/
	eBmuGenPara102_CTH1Clr = 61,							/*充电高温一级消除回差值1℃ [1,80]*/
	eBmuGenPara102_DTH2Clr = 62,							/*放电高温二级消除回差值1℃ [1,80]*/
	eBmuGenPara102_DTH1Clr = 63,							/*放电高温一级消除回差值1℃ [1,80]*/

	eBmuGenPara102_CTL2Clr = 64,							/*充电低温二级消除回差值1℃ [-40,30]*/
	eBmuGenPara102_CTL1Clr = 65,							/*充电低温一级消除回差值1℃ [-40,30]*/
	eBmuGenPara102_DTL2Clr = 66,							/*放电低温二级消除回差值1℃ [-40,30]*/
	eBmuGenPara102_DTL1Clr = 67,							/*放电低温一级消除回差值1℃ [-40,30]*/

	eBmuGenPara102_SVH2Clr = 68,							/*总压高压二级消除回差值0.1v [10,1000]*/
	eBmuGenPara102_SVH1Clr = 69,							/*总压高压一级消除回差值0.1v [10,1000]*/
	eBmuGenPara102_SVL2Clr = 70,							/*总压低压二级消除回差值0.1v [10,1000]*/
	eBmuGenPara102_SVL1Clr = 71,							/*总压低压一级消除回差值0.1v [10,1000]*/

	eBmuGenPara102_ChgCH2Clr = 72,							/*充电过流二级消除回差值0.1A [10,10000]*/
	eBmuGenPara102_ChgCH1Clr = 73,							/*充电过流一级消除回差值0.1A [10,10000]*/
	eBmuGenPara102_DhgCH2Clr = 74,							/*放电过流二级消除回差值0.1A [10,10000]*/
	eBmuGenPara102_DhgCH1Clr = 75,							/*放电过流一级消除回差值0.1A [10,10000]*/

	eBmuGenPara102_Envir2Clr = 76,							/*环境温度高温二级消除回差值1℃ [1,100]*/
	eBmuGenPara102_Envir1Clr = 77,							/*环境温度高温一级消除回差值1℃ [1,100]*/
	eBmuGenPara102_Perip2Clr = 78,							/*外设温度高温二级消除回差值1℃ [1,100]*/
	eBmuGenPara102_Perip1Clr = 79,							/*外设温度高温一级消除回差值1℃ [1,100]*/

	eBmuGenPara102_ChgLimSubSoc = 80,						/*释放充电限制SOC回差值0.10%	[1,500]*/
	eBmuGenPara102_DhgLimSubSoc = 81,						/*释放放电限制SOC回差值0.10%	[1,500]*/
	eBmuGenPara102_ChgLimClrV = 82,							/*强制允许充电最大电压值1mv [2000,5000]*/
	eBmuGenPara102_DhgLimClrV = 83,							/*强制允许放电最小电压值1mv [1000,4000]*/
	eBmuGenPara102_CellPlaUpV = 84,							/*单体压差过大检测区间上限1mv	[2000,5000]*/
	eBmuGenPara102_CellPlaDnV = 85,							/*单体压差过大检测区间下限1mv	[1000,4000]*/

	eBmuGenPara102_CfgFin           						/*设备通用参数配置结束*/
}e_GGenPara102;


/*协议103子索引列表*/
typedef enum
{
	eBmuHigLevPara103_CfgFlag = 0,							/*设备高级参数配置标志*/
	eBmuHigLevPara103_ClusterNum = 1,					    /*单个主控管理簇数数目 [1,2]*/
	eBmuHigLevPara103_BsuNum = 2,							/*单组管理电池箱(BSU02)数目(PACK数量) [1,30]*/
	eBmuHigLevPara103_ConNum = 3,							/*一个PACK包含几个模组 [1,30]*/
	eBmuHigLevPara103_BsuVNum = 4,							/*一个模组电压采样节数 [1,16]*/
	eBmuHigLevPara103_BsuTNum = 5,							/*一个模组温度采样节数 [1,16]*/
	eBmuHigLevPara103_BalTNum = 6,							/*一个模组板温采样节数 [0,2]*/
	eBmuHigLevPara103_VSumNum = 7,							/*单体电压采样总节数 [1,416]*/
	eBmuHigLevPara103_TSumNum = 8,							/*单体温度采样总节数 [1,416]*/
	eBmuHigLevPara103_BTSumNum = 9,							/*均衡板温采样总节数 [0,64]*/

	eBmuHigLevPara103_ChgLimOffT = 10,						/*充电限制请求保护时间1s	[0,6000]*/
	eBmuHigLevPara103_DhgLimOffT = 11,						/*放电限制请求保护时间1s	[0,6000]*/
	eBmuHigLevPara103_ChgProOffT = 12,						/*充电保护延迟保护时间1s	[0,6000]*/
	eBmuHigLevPara103_DhgProOffT = 13,						/*放电保护延迟保护时间1s	[0,6000]*/
	eBmuHigLevPara103_ErrProOffT = 14,						/*系统故障延迟保护时间1s	[0,6000]*/
	eBmuHigLevPara103_CommProOffT = 15,						/*外部通讯故障延迟保护时间1s	[0,6000]*/
	eBmuHigLevPara103_VL2PwrOffT = 16,						/*低压二级延迟关机时间1s	[0,6000]*/
	eBmuHigLevPara103_VL1PwrOffT = 17,						/*欠压一级延迟关机时间1s	[0,6000]*/
	eBmuHigLevPara103_ErrPWrOffT = 18,						/*系统故障延迟关机时间1s	[0,6000]*/
	eBmuHigLevPara103_CommPwrOffT = 19,						/*外部通讯故障延迟关机时间1s	[0,6000]*/

	eBmuHigLevPara103_FanOpenTemp = 20,						/*散热风扇开启温度值1℃	[-20,60]*/
	eBmuHigLevPara103_FanCloseTemp = 21,					/*散热风扇关闭温度值1℃	[-20,60]*/
	eBmuHigLevPara103_HotOpenTemp = 22,						/*加热膜开启温度值1℃	[-40,40]*/
	eBmuHigLevPara103_HotCloseTemp = 23,					/*加热膜关闭温度值1℃	[-40,40]*/

	eBmuHigLevPara103_ChgFinCVEn = 24,						/*充电末端限流恒压使能状态[0,1]	0:禁止 1:使能*/
	eBmuHigLevPara103_DhgFinCVEn = 25,						/*放电末端限流恒压使能状态[0,1]	0:禁止 1:使能*/
	eBmuHigLevPara103_ChgTHLimEn = 26,						/*充电高温降功率使能状态[0,1] 	0:禁止 1:使能*/
	eBmuHigLevPara103_DhgTHLimEn = 27,						/*放电高温降功率使能状态[0,1]	 	0:禁止 1:使能*/
	eBmuHigLevPara103_ChgTLLimEn = 28,						/*充电低温降功率使能状态[0,1]		0:禁止 1:使能*/
	eBmuHigLevPara103_DhgTLLimEn = 29,						/*放电低温降功率使能状态[0,1]		0:禁止 1:使能*/
	eBmuHigLevPara103_TSloVLimEn = 30,						/*温度调节高低压限值使能状态[0,1]	0:禁止 1:使能*/

	eBmuHigLevPara103_PwrPor = 31,							/*开机按钮信号检测端口	按位表示，每bit表示一个IO*/
	eBmuHigLevPara103_IsoPort = 32,							/*断路器信号检测端口	按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_FusPort = 33,							/*熔断器信号检测端口	按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_SmogPort = 34,						/*烟感故障信号检测端口	按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_EmergPort = 35,						/*急停按钮信号检测端口	按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_ThundPort = 36,						/*防雷器故障信号检测端口按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_FirePort = 37,						/*消防告警信号检测端口	按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_GasPort = 38,							/*气体浓度信号检测端口	按位表示，每bit表示一个IO口*/
	eBmuHigLevPara103_FanPort = 39,							/*风扇状态信号检测端口	按位表示，每bit表示一个IO口*/

	eBmuHigLevPara103_LeakMonitType = 40,					/*绝缘检测方式[0,8] 0:不检测 1:非运行检测不报故障 2:非运行检测上报故障 3:全程检测不报故障 4:全程检测非运行上报故障 5:全程检测全程上报故障 6:运行检测不上报 7:运行检测上报 8:请求时检测上报*/
	eBmuHigLevPara103_AdhMonitType = 41,					/*粘连检测方式 [0,3],0:不检测 1:Link电路 2:外端电压 3:Link电路和外端电压,默认1*/

	eBmuHigLevPara103_MainSwhC = 42,						/*主接触器安全电流值0.1A	[0,5000]*/
	eBmuHigLevPara103_PChgSwhC = 43,						/*预充接触器安全电流值0.1A	[0,500]*/
	eBmuHigLevPara103_PChgResis = 44,						/*预充电阻阻值1Ω	[1,1000]*/
	eBmuHigLevPara103_PChgPower = 45,						/*预充电阻额定功率值1W	[1,1000]*/
	eBmuHigLevPara103_SafeSubV = 46,						/*允许启动内外安全压差0.1V [0,10000]*/
	eBmuHigLevPara103_PChgFinV = 47,						/*预充完成判定内外压差0.1V [0,10000]*/

	eBmuHigLevPara103_PChgWaitT = 48,						/*电流至少预充时间0.1s	[0,600]*/
	eBmuHigLevPara103_PChgOffT = 49,						/*电流预充超时时间0.1s	[0,600]*/
	eBmuHigLevPara103_SelfWaitT = 50,						/*自启动前等待时间0.1s	[0,600]*/
	eBmuHigLevPara103_MSwhOffV = 51,						/*断开接触器外端残存电压值0.1V	[0,10000]*/
	eBmuHigLevPara103_HigVOffV = 52,						/*高压采样正常至少电压值0.1V	[0,1000]*/
	eBmuHigLevPara103_SumDiffV = 53,						/*总电压一致允许差值0.1V	[0,1000]*/

	eBmuHigLevPara103_LedkRlyState = 54,					/*绝缘检测后PE继电器状态 [0,1](0:断开 1:闭合)*/
	eBmuHigLevPara103_LeakMonitCycl = 55,					/*绝缘检测周期1s [1,64800]默认6H(21600s)*/
	eBmuHigLevPara103_LedkSampDelyT = 56,					/*绝缘采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认1500ms])*/
	eBmuHigLevPara103_PgndVSampGapT = 57,					/*负母线电压采样(无绝缘检测时)周期1ms [10,60000](更新负母线电压时间[默认1000ms])*/
	eBmuHigLevPara103_AdhRlyState = 58,						/*粘连检测后Link继电器状态 [0,1](0:断开 1:闭合)*/
	eBmuHigLevPara103_AdhMonitCycl = 59,					/*粘连检测周期1s [1,64800]默认6H(21600s)*/
	eBmuHigLevPara103_Lk0SampDelyT = 60,					/*负载电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认700ms])*/
	eBmuHigLevPara103_Lk1SampDelyT = 61,					/*Link+电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认600ms])*/
	eBmuHigLevPara103_Lk2SampDelyT = 62,					/*Link-电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认2000ms])*/
	eBmuHigLevPara103_RlyCtrlGapT = 63,						/*板上继电器切换至少间隔时间1ms [10,10000](防止继电器动作太快[默认500ms])*/

	eBmuHigLevPara103_ChgHTDnFstT = 64,						/*充电高温降功率起始温度1℃ [1,80]*/
	eBmuHigLevPara103_ChgHTDnDifT = 65,						/*充电高温降功率起始温差1℃ [1,80]*/
	eBmuHigLevPara103_ChgHTDnRate = 66,						/*充电高温降功率下降比率0.1%/℃ [1,500]*/
	eBmuHigLevPara103_ChgHTDnMinC = 67,						/*充电高温降功率最低电流0.1A [10,5000]*/

	eBmuHigLevPara103_DhgHTDnFstT = 68,						/*放电高温降功率起始温度1℃ [1,80]*/
	eBmuHigLevPara103_DhgHTDnDifT = 69,						/*放电高温降功率起始温差1℃ [1,80]*/
	eBmuHigLevPara103_DhgHTDnRate = 70,						/*放电高温降功率下降比率0.1%/℃ [1,500]*/
	eBmuHigLevPara103_DhgHTDnMinC = 71,						/*放电高温降功率最低电流0.1A [10,5000]*/

	eBmuHigLevPara103_CfgFin								/*主控高级参数配置结束*/
}e_BmuHigLevPara103;

/*协议104子索引列表*/
typedef enum
{
    eGHardPara104_CfgFlag = 0,          					/*硬件参数配置标志*/
    eGHardPara104_BalTact,             	 					/*均衡策略算法 [0禁止均衡 1压差积分均衡]*/
    eGHardPara104_BalCirc,              					/*均衡电路     [0被动放电均衡 1主动充电+主动放电均衡 2被动放电+主动充放电均衡 ]*/
    eGHardPara104_BalCycT,              					/*均衡检测周期     0.1Min*/
    eGHardPara104_BalSamT,              					/*均衡采样周期     0.1s*/
    eGHardPara104_BalChgV,              					/*充电均衡压差 1mv*/
    eGHardPara104_BalDhgV,              					/*放电均衡压差 1mv*/

	eGHardPara104_CSampChan,                                /*电流采集通道[0:外部高压模块,1霍尔,2分流器,3霍尔和分流器]*/
    eGHardPara104_ShuntRate,            					/*分流器单向电压范围[1,200]1mV*/
    eGHardPara104_ShuntRang,            					/*分流器单向电流量程[1,200]10A*/
    eGHardPara104_HallSign,            						/*霍尔信号零点电压[1,200]0.1V*/
    eGHardPara104_HallRate,            						/*霍尔单向电压范围[1,200]0.1V*/
    eGHardPara104_HallRang,            						/*霍尔单向电流量程[1,200]10A*/
    eGHardPara104_CFilType,             					/*电流滤波方式[0不滤波 1瞬时平均值 2瞬时有效值 3积分平均值]*/
    eGHardPara104_CFilTime,             					/*电流滤波次数[1,200]1次*/

    eGHardPara104_ChgIntPoint,          					/*充电积分零点电流值[1,100]10mA*/
    eGHardPara104_DhgIntPoint,          					/*放电积分零点电流值[1,100]10mA*/
    eGHardPara104_ChgShowPoint,         					/*充电显示零点电流值[1,100]0.1A*/
    eGHardPara104_DhgShowPoint,         					/*放电显示零点电流值[1,100]0.1A*/
    eGHardPara104_ChgIdlePoint,         					/*待机时充电零点电流值[1,100]0.1A*/
    eGHardPara104_DhgIdlePoint,         					/*待机时放电零点电流值[1,100]0.1A*/

    eGHardPara104_BmuSWVerLL,           					/*BMU软件版本号最低位(支线变更版本)*/
    eGHardPara104_BmuSWVerML,           					/*BMU软件版本号次低位(支线归档版本)*/
    eGHardPara104_BmuSWVerMH,           					/*BMU软件版本号次高位(主线变更版本)*/
    eGHardPara104_BmuSWVerHH,           					/*BMU软件版本号最高位(主线归档版本)*/

    eGHardPara104_AirColdOn_CT,           					/*空调开启制冷电池温度[10,60]1℃(默认34℃)*/
    eGHardPara104_AirColdOff_CT,           					/*空调停止制冷电池温度[10,60]1℃(默认30℃)*/
    eGHardPara104_AirHeatOn_CT,           					/*空调开启制热电池温度[-30,20]1℃(默认5℃)*/
    eGHardPara104_AirHeatOff_CT,           					/*空调停止制热电池温度[-30,20]1℃(默认10℃)*/

    eGHardPara104_PduEnvTH2,           					    /*PDU环境温度高二级告警值*/
    eGHardPara104_PduEnvTH1,           					    /*PDU环境温度高一级告警值*/
    eGHardPara104_PduCabTH2,           					    /*PDU线缆温度高二级告警值*/
    eGHardPara104_PduCabTH1,           					    /*PDU线缆温度高一级告警值*/
    eGHardPara104_PcsFanTH2,           					    /*PCS风扇出风口温度高二级告警*/
    eGHardPara104_PcsFanTH1,           					    /*PCS风扇出风口温度高一级告警*/
    eGHardPara104_AirDeHumOn_H,           					/*除湿机开启除湿湿度[50,99]1%,(默认80%)*/
    eGHardPara104_AirDeHumOff_H,           					/*除湿机停止除湿湿度[45,95]1%,(默认70%)*/

    eGHardPara104_AirType,           					    /*空调类型 0：未安装 1：YFY自研  2：TF  [0,2]*/
    eGHardPara104_PerfluorEn,           					/*全氟己酮功能 0：不使能  1：使能                     [0,1]*/
    eGHardPara104_Imlu2En,           					    /*灯带控制功能 0：不使能  1：使能                     [0,1]*/
    eGHardPara104_LeakageType,           					/*漏电流传感器类型 0：未安装 1：柯力三电      [0,1]*/
    eGHardPara104_PduFanOpenT,           					/*Pdu散热风扇开启温度                                         [20,80]*/
    eGHardPara104_PduFanFullT,           					/*Pdu散热风扇全速温度                                         [20,80]*/
    eGHardPara104_PcsFanOpenT,           					/*Pcs散热风扇开启温度                                         [20,80]*/
    eGHardPara104_PcsFanFullT,           					/*Pcs散热风扇全速温度                                         [20,80]*/
    eGHardPara104_ExplosInter,           					/*泄爆阀开启间隔，单位：天                                   [1,90]*/
    eGHardPara104_ExplosTime,           					/*泄爆阀开启时长 单位：min           [1,30]*/
    eGHardPara104_Imeu3En,           					    /*IMEU3使能 0：不使能 1：使能1#IMEU3 2:使能2#IMEU3 3:使能1#和2#IMEU3*/
    eGHardPara104_CompTH,           					    /*电池仓高温告警值*/
    eGHardPara104_LeakageC,           					    /*漏电流传感器告警电流值*/
    eGHardPara104_PackConnTH,           					/*PACK连接器高温告警值*/
    eGHardPara104_ProjectCode,           					/*项目编号 0：A2509 1：M2501*/
	eGHardPara104_Imiu4En,           					    /*IMiu4使能 0：不使能 1：使能1#IMIU4 2:使能2#IMEU4 3:使能1#和2#IMIU4*/

    eGHardPara104_Fin                   					/*硬件参数配置结束*/
}e_GHardPara104;


/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : u8 ParaCheckGGenParaEffect(u16 addr, u16 para)
** @Input     : addr:参数地址 para:参数值
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测电池组通用参数值是否有效
** @The notes :
**===============================================================================================*/
u8 ParaCheckGGenParaEffect(u16 addr, u16 para);

/*=================================================================================================
** @Name      : u8 ParaCheckGHigParaEffect(u32 addr, u16 para)
** @Input     : addr:参数地址 para:参数值
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测电池组高级参数值是否有效
** @The notes :
**===============================================================================================*/
u8 ParaCheckGHigParaEffect(u16 addr, u16 para);

/*=================================================================================================
** @Name      : u8 ParaCheckGHWParaEffect(u16 addr, u8 para)
** @Input     : addr:参数地址 para:参数值 len:参数长度
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测电池组硬件参数值是否有效
** @The notes : 用户必须补充完善
**===============================================================================================*/
u8 ParaCheckGHWParaEffect(u16 addr, u8 para);
#endif

