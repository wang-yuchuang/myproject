/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupInfo.h
** @Founder      : ZQD808
** @Date         : 2025.01.22
** @Function     : 电池组全局信息定义模块
**===============================================================================================*/
#ifndef __GROUPINFO_H__
#define __GROUPINFO_H__
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "stddef.h"
#include <cmsis_os2.h>
#include "CapEnerCorr.h"
#include "IMIU4Protocol.h"

#define SLAVE_MAX_NUM			(MAX_BSU_NUM)											/*从控数目*/
#define SLAVE_MAX_CELVNUM		(MAX_BSU_CELL)											/*一个从控单体电压数目*/
#if(0 == BMU_BSU_COM_TYPE)                  /*BMU主从菊花链通信*/
#define SLAVE_MAX_CELTNUM		(4)														/*一个从控单体温度数目*/
#define SLAVE_MAX_BALTNUM		(2)														/*一个从控板温温度数目*/
#elif(0 == BMU_BSU_CAN_TYPE)                /*BMU主从自研CAN通信*/
#define SLAVE_MAX_CELTNUM		(52)													/*一个从控单体温度数目*/
#define SLAVE_MAX_BALTNUM		(2)														/*一个从控板温温度数目(BYD AFE4+2时均衡温度占用单体温度空间)*/
#else                                       /*BMU主从高特CAN通信*/
#define SLAVE_MAX_CELTNUM		(10)													/*一个从控单体温度数目*/
#define SLAVE_MAX_BALTNUM		(0)														/*一个从控板温温度数目*/
#endif
#define GROUP_MAX_CELVNUM		(SLAVE_MAX_NUM*SLAVE_MAX_CELVNUM)						/*组内单体电压总数目*/
#define GROUP_MAX_CELTNUM		(SLAVE_MAX_NUM*SLAVE_MAX_CELTNUM)						/*组内单体温度总数目*/
#define GROUP_MAX_BALTNUM		(SLAVE_MAX_NUM*SLAVE_MAX_BALTNUM)						/*组内均衡板温总数目*/
#if(SLAVE_MAX_CELTNUM >= 16)
#define GROUP_MAX_SUMTNUM		(SLAVE_MAX_NUM*SLAVE_MAX_CELTNUM)						/*组内从控温度总数目(满16节单体温度时没有均衡板温)*/
#else
#define GROUP_MAX_SUMTNUM		(SLAVE_MAX_NUM*(SLAVE_MAX_CELTNUM+SLAVE_MAX_BALTNUM))	/*组内从控温度总数目(包括单体温度和均衡板温)*/
#endif

#define BMU_EXT_COM_DEV			(0)			/*主控外部(上层)通信设备(0:不检测 1:PCS 2BSMU)*/

#define BMU_PWR_SWH_TYPE		(1)			/*BMU电源接触器类型(0:无电源接触器1:独立电源接触器2:共用负极接触器)*/
#define BMU_DC_VH_TYPE			(2)			/*BMU直流端高压采样值获取方式(0:无高压采样 1:自身采样 2:通讯获取)*/
#define	BMU_NEG_SWH_EN			(1)			/*BMU负极接触器控制使能(0:禁止 1:使能)*/
#define	BMU_FAN_SWH_EN			(1)			/*BMU风扇接触器控制使能(0:禁止 1:使能)*/
#define	BMU_HOT_SWH_EN			(1)			/*BMU加热接触器控制使能(0:禁止 1:使能)*/
#define	BMU_SMG_SWH_EN			(1)			/*BMU烟感接触器控制使能(0:禁止 1:使能)*/
#define	BMU_LED_SWH_EN			(1)			/*BMU状态LED显示控制使能(0:禁止 1:使能)*/
#define	BMU_CDHG_DO_EN			(1)			/*BMU充放电干接点同一DO3使能(0:禁止 1:使能)*/
#define BMU_SWH_BACK_TYPE		(3)			/*BMU接触器回检方式(0:不回检(检控制信号) 1:板级信号回检 2:板级采样回检(驱动电流) 3:外部信号回检*/

#define BMU_CELL_R_TRIG_T		(604800000)	/*BMU触发从机电芯内阻检测周期(7天,单位1ms*/

#define BMU_FAN_PWR_EN			(0)			/*BMU控制PWM调速风扇电源使能[复用风扇接触器管脚](0:禁止(主控全速风扇控制) 1:使能)*/
#define BMU_AIR_PWR_EN			(0)			/*BMU控制空调电源使能[复用加热接触器管脚](0:禁止 1:使能)*/

#define BMU_PWM_FAN_EN			(0)			/*BMU控制IMLU散热PWM调速风扇使能[IMLU1控制](0:禁止 1:使能)*/
#define BMU_PWM_FAN_NUM			(16)		/*BMU控制IMEU3风扇通道数*/
#define BMU_LU_FLOOD_HZ			(0)			/*BMU检测IMLU水浸告警频率(1Hz,0表示不检测)*/

#define BMU_IMEU_COM_EN			(0)			/*BMU与IMEU2通信控制交互使能(0:禁止 1:使能)*/
#define BMU_EU_SHUTFAN_EN		(1)			/*BMU控制IMEU百叶风机使能*/
#define BMU_EU_FIRELIG_EN		(1)			/*BMU控制IMEU照明/声光报警/消防等使能*/
#define BMU_EU_FLOOD_HZ			(300)		/*BMU检测IMEU水浸告警频率(1Hz,0表示不检测)*/
#define BMU_EU_GAS1_CONC		(2500)		/*BMU检测IMEU可燃气体1(H2)浓度告警值(0.01%,0xffff表示不检测;100.37沿用值,消防L3已改800ppm,事件记录口径保持老平台2500,2026-08-28定)*/
#define BMU_EU_GAS2_CONC		(2500)		/*BMU检测IMEU可燃气体2(CO)浓度告警值(0.01%,0xffff表示不检测;100.37沿用值,消防L3已改600ppm,事件记录口径保持老平台2500,2026-08-28定)*/
//ZQD808添加，宏定义改为参数
//#define BMU_IMEU3_COM_EN		(0)			/*BMU与IMEU3通信控制交互使能(0:禁止 1:使能)*/

//#define BSU02U1_TEPMER_ALARM    (150)		/*PT1000铜排温度告警温度*/
#define BSU02U1_AERO_ALARM      (5000)		/*PT1000气溶胶浓度告警浓度*/

#define BSU02U1_MSDDUTY_ALARM1   (30)		/*MSD告警占空比下限*/
#define BSU02U1_MSDDUTY_ALARM2   (85)		/*MSD告警占空比上限*/
#define BSU02U1_MSDAMP_ALARM1    (100)		/*MSD告警幅值下限*/
#define BSU02U1_MSDAMP_ALARM2    (200)		/*MSD告警幅值上限*/

#define GINDEX0_LEN			    (4)		    /*校时信息数组长度*/
#define GINDEX2_LEN			    (24)		    /*设备信息数组长度(0-7标准字段+8-23共16条ssdk git信息)*/
#define GINDEX3_LEN                            (23)  

#define GINDEX10_LEN            (12)        /*组1历史故障信息数组长度*/          
#define GINDEX11_LEN            (12)        /*组2历史故障信息数组长度*/ 
#define GINDEX12_LEN            (12)        /*组1历史事件信息数组长度*/          
#define GINDEX13_LEN            (12)        /*组2历史事件信息数组长度*/ 

#define GINDEX14_LEN			(eDevSamp_Len)		    /*板级采样信息数组长度*/
#define GINDEX15_LEN			(eHisDodRec_Len)		/*组1历史DOD记录信息数组长度(最新一条)*/
#define GINDEX16_LEN			(eHisDodRec_Len)		/*组2历史DOD记录信息数组长度(最新一条)*/

#define GINDEX45_LEN			(11)		/*历史故障记录信息查询数组长度*/
#define GINDEX46_LEN			(11)		/*历史事件记录信息查询数组长度*/
#define GINDEX47_LEN			(56)		/*历史故障录波信息查询数组长度*/
#define HISDOD_REC_NUM		(50)	/*历史DOD记录容量(环形缓冲条数)*/
#define HISDOD_RING_LEN			(2 + HISDOD_REC_NUM * 10)	/*历史DOD环形缓冲长度V3(标志+Id+50条×10字)*/

#define GINDEX50_LEN			(eSysStateInfo50_Len)	/*系统状态信息数组长度*/
#define GINDEX51_LEN			(eSysMainInfo51_Len)	/*系统整体信息数组长度*/
#define GINDEX52_LEN			(eSysCapInfo52_Len)		/*系统电量信息数组长度*/
#define GINDEX53_LEN			(eSysLimInfo53_Len)		/*系统限值信息数组长度*/
#define GINDEX54_LEN			(eSysCharInfo54_Len)	/*系统特征信息数组长度*/
#define GINDEX55_LEN			(eCurrCapEner55_Len)    /*当前能量统计信息长度*/
#define GINDEX56_LEN			(eHisCapEner56_Len)		/*历史能量统计信息长度*/
#define GINDEX57_LEN			(eAlmFault_Len)		    /*组1故障告警信息长度*/
#define GINDEX58_LEN			(eAlmFault_Len)		    /*组2故障告警信息长度*/
#define GINDEX59_LEN			(eTime59_Len)		    /*时间统计信息长度*/
#define GINDEX60_LEN			(eErr60_Len)		    /*故障统计长度*/
#define GINDEX61_LEN			(eDiagSos61_Len)		/*系统电量信息数组长度*/
#define GINDEX62_LEN			(eDodStat62_Len)		/*放电深度信息数组长度*/
#define GINDEX63_LEN			(2)			            /*权限级别状态信息*/
#define GINDEX64_LEN			(2)			            /*权限密码登录信息*/
#define GINDEX65_LEN			(eSysCtrlCmd_Len)		/*组1系统控制命令*/
#define GINDEX66_LEN			(eSysCtrlCmd_Len)		/*组2系统控制命令*/

#define GINDEX67_LEN			(eImiu4Info67_Len)		/*组一IMIU4信息数组长度(ICC mIndex=67)*/
#define GINDEX68_LEN			(eImiu4Info68_Len)		/*组二IMIU4信息数组长度(ICC mIndex=68)*/
#define GINDEX69_LEN			(2)		                /*用户控制命令数组长度*/
#define GINDEX70_LEN			(2)		                /*控制命令反馈数组长度*/
#define GINDEX71_LEN			(10)		            /*系统调试命令数组长度 8→10 扩展4个cellWord支持48/52串*/
#define GINDEX72_LEN			(4)		                /*采样校准命令数组长度(ICC mIndex=72)*/
#define GINDEX73_LEN			(eSampCalFbInfo73_Len)	/*采样校准反馈信息数组长度(ICC mIndex=73)*/
#define GINDEX74_LEN			(eBsudPSamp74_Len)		/*BSU02U2板级及PACK采样信息数组长度(ICC mIndex=74)*/
#define GINDEX75_LEN			(eImeu3Cfg_Len)			/*IMEU3硬件参数当前值(ICC mIndex=75)*/
#define GINDEX76_LEN			(eImeu3Cfg_Len)			/*IMEU3硬件参数配置值(ICC mIndex=76)*/
#define GINDEX77_LEN			(53)		            /*IMEU3采样和故障信息(ICC mIndex=77)*/



#define GINDEX90_LEN			(26)		                /*充电均衡状态信息数组长度*/
#define GINDEX91_LEN            (26)                        /*放电均衡状态信息数组长度*/
#define GINDEX92_LEN			(GROUP_MAX_CELVNUM)			/*电池单体电压信息数组长度*/
#define GINDEX93_LEN			(GROUP_MAX_CELTNUM)			/*电池单体温度信息数组长度*/

#define GINDEX20_21_LEN			(26)		                /*充电均衡状态信息数组长度*/
#define GINDEX22_23_LEN			(26)		                /*放电均衡状态信息数组长度*/
#define GINDEX24_25_LEN			(GROUP_MAX_CELVNUM)			/*电池单体电压信息数组长度*/
#define GINDEX26_27_LEN			(GROUP_MAX_CELTNUM)			/*电池单体温度信息数组长度*/
#define GINDEX28_29_LEN			(GROUP_MAX_CELTNUM)			/*电池单体内阻信息数组长度*/


#define GINDEX94_LEN			(ePackTemp94_Len)			/**/
#define GINDEX95_LEN			(ePackStat95_Len)			/**/
#define GINDEX96_LEN			(GROUP_MAX_CELVNUM)			/*电池单体内阻信息数组长度*/
#define GINDEX98_LEN            (MAX_BSU_NUM * 27)          /* 组内IC状态数组总长度 */

/*内阻查询指令(ICC85)与内阻信息(ICC28-35)子索引长度配置*/
#define ICC_MINDEX85_INSQUERY_LEN   (8)     /*ICC 85 内阻查询指令子索引数(u16×8)*/
#define ICC_MINDEX28_HDR_LEN        (3)     /*ICC 28 头部子索引数(年月日+时分秒+频点)*/
#define ICC_MINDEX28_CELL_NUM       (208)   /*ICC 内阻每主索引电芯数(实部/虚部一致)*/
#define ICC_MINDEX28_IDX_LEN        (ICC_MINDEX28_HDR_LEN + ICC_MINDEX28_CELL_NUM) /*211=3头+208*/
#define ICC_RESIS_GRP_NUM           (4)     /*内阻实部/虚部各4个主索引(28-31/32-35)*/
#define GINDEX105_LEN			(eGBsuPara102_Fin)			/**/


#define GINDEX115_LEN       	(15)						/*不同温度充电倍率表长度*/
#define GINDEX116_LEN       	(15)						/*不同温度放电倍率表长度*/
#define GINDEX119_LEN      		(44)						/*电压/SOC平滑电流限值参数表长度*/

#define GINDEX212_LEN           (eATFUserInfo212_Len)     	/*同飞空调用户参数信息*/
#define GINDEX213_LEN           (eATFUserState213_Len)     	/*同飞空调用户状态信息*/
#define GINDEX214_LEN           (eATFErrInfo214_Len)     	/*同飞空调故障信息*/

/*CAN通信设备信息索引定义*/
typedef enum
{
    eICCDev2_ID = 0,                    //设备识别码
	eICCDev2_Addr = 1,                  //设备地址
	eICCDev2_HW = 2,                    //硬件版本号
	eICCDev2_SWL = 3,                   //软件版本号L
	eICCDev2_SWH = 4,                   //软件版本号H
	eICCDev2_BSP = 5,                   //BSP版本号
	eICCDev2_CAN = 6,                   //CAN通信包版本号
	eICCDev2_Mdb = 7,                   //Modbus通信包版本号
    eICCDev2_SSDKW1 = 8,                //ssdk的版本号兼git信息1
    eICCDev2_SSDKW2 = 9,                //ssdk的版本号兼git信息2
    eICCDev2_SSDKW3 = 10,                //ssdk的版本号兼git信息3
    eICCDev2_SSDKW4 = 11,                //ssdk的版本号兼git信息4
    eICCDev2_SSDKW5 = 12,                //ssdk的版本号兼git信息5
    eICCDev2_SSDKW6 = 13,                //ssdk的版本号兼git信息6
    eICCDev2_SSDKW7 = 14,                //ssdk的版本号兼git信息7
    eICCDev2_SSDKW8 = 15,                //ssdk的版本号兼git信息8
    eICCDev2_SSDKW9 = 16,                //ssdk的版本号兼git信息9
    eICCDev2_SSDKW10 = 17,               //ssdk的版本号兼git信息10
    eICCDev2_SSDKW11 = 18,               //ssdk的版本号兼git信息11
    eICCDev2_SSDKW12 = 19,               //ssdk的版本号兼git信息12
    eICCDev2_SSDKW13 = 20,               //ssdk的版本号兼git信息13
    eICCDev2_SSDKW14 = 21,               //ssdk的版本号兼git信息14
    eICCDev2_SSDKW15 = 22,               //ssdk的版本号兼git信息15
    eICCDev2_SSDKW16 = 23                //ssdk的版本号兼git信息16
}e_DevInfo2;

/*板级采样信息索引定义*/
typedef enum
{
    eDevSamp_1V8 = 0,                    /*1.8V回检电压*/
	eDevSamp_3V3,                        /*3.3V回检电压*/
    eDevSamp_3V5,                        /*3.5V回检电压*/
    eDevSamp_5V0,                        /*5V回检电压*/
    eDevSamp_5V1,                        /*VCCH5V回检电压*/
    eDevSamp_12V,                        /*12V回检电压*/
    eDevSamp_TempAD,                     /*温度采集1*/
    eDevSamp_TempAD1,                    /*温度采集2*/
    eDevSamp_GpioOut,                    /*输出GPIO信息(子索引8,协议02表)*/
    eDevSamp_GpioIn,                     /*输入GPIO信息(子索引9,协议02表)*/
    eDevSamp_MuxGpioOut,                 /*复用GPIO输出信息*/
    eDevSamp_MuxGpioIn1,                 /*复用GPIO输入信息1*/
    eDevSamp_MuxGpioIn2,                 /*复用GPIO输入信息2*/
    eDevSamp_Reserved,                            /*预留*/

    eDevSamp_Len                         /*板级采样信息长度*/

}eDevSampInfo14;

/*协议子索引列表，IMEU3板级信息*/
typedef enum
{
	eIMEUBoardInfo1_Ntc1Tem = 0,								    /*NTC1温度*/
	eIMEUBoardInfo1_Ntc2Tem,								        /*NTC2温度*/
	eIMEUBoardInfo1_Ntc3Tem,								        /*NTC3温度*/
	eIMEUBoardInfo1_Ntc4Tem,								        /*NTC4温度*/
	eIMEUBoardInfo1_Ntc5Tem,										/*NTC5温度*/
    eIMEUBoardInfo1_Ntc6Tem,										/*NTC6温度*/
    eIMEUBoardInfo1_Ntc7Tem,										/*NTC7温度*/
    eIMEUBoardInfo1_Ntc8Tem,										/*NTC8温度*/
    eIMEUBoardInfo1_Ntc9Tem,										/*NTC9温度*/
    eIMEUBoardInfo1_Ntc10Tem,										/*NTC10温度*/
    eIMEUBoardInfo1_Ntc11Tem,										/*NTC11温度*/
    eIMEUBoardInfo1_Ntc12Tem,										/*NTC12温度*/

    eIMEUBoardInfo1_SenTem,										    /*485温湿度传感器温度*/
    eIMEUBoardInfo1_SenHum,										    /*485温湿度传感器湿度*/
    eIMEUBoardInfo1_485Tem1,										/*485温湿度传感器温度*/
    eIMEUBoardInfo1_485Hum1,										/*485温湿度传感器湿度*/
    eIMEUBoardInfo1_485Tem2,										/*485温湿度传感器温度*/
    eIMEUBoardInfo1_485Hum2,										/*485温湿度传感器湿度*/
    eIMEUBoardInfo1_485Tem3,										/*485温湿度传感器温度*/
    eIMEUBoardInfo1_485Hum3,										/*485温湿度传感器湿度*/
    eIMEUBoardInfo1_485Tem4,										/*485温湿度传感器温度*/
    eIMEUBoardInfo1_485Hum4,										/*485温湿度传感器湿度*/

    eIMEUBoardInfo1_DeTem,										    /*除湿机温度*/
    eIMEUBoardInfo1_DeHum,										    /*除湿机湿度*/
    eIMEUBoardInfo1_DeOpenHum,										/*除湿机启动除湿湿度*/
    eIMEUBoardInfo1_DeCloseHum,										/*除湿机关闭除湿湿度*/
    eIMEUBoardInfo1_DeState,										/*除湿机工作状态*/
    eIMEUBoardInfo1_DeAddr,										    /*除湿机通信地址*/

	eIMEUBoardInfo1_Len   			        				       /*IMEU3温湿度信息长度*/
}e_IMEU3BoardInfo1;

typedef enum
{
	eIMEUBoardInfo2_DI1_16 = 0,								        /*DI1-16状态*/
	eIMEUBoardInfo2_DI17_22,								        /*DI17-22状态*/
	eIMEUBoardInfo2_DO1_16,								            /*DO1-16状态*/
    eIMEUBoardInfo2_DO17_24,								        /*DO17-24状态*/
    eIMEUBoardInfo2_X1,								                /*4路X1板连接情况*/
    
    eIMEUBoardInfo2_H2Raw,								            /*可燃气体传感器H2读数*/
    eIMEUBoardInfo2_CORaw,								            /*可燃气体传感器CO读数*/
    eIMEUBoardInfo2_H2Raw2,								            /*可燃气体传感器读数(预留)*/
    eIMEUBoardInfo2_CORaw2,								            /*可燃气体传感器读数(预留)*/
    
    eIMEUBoardInfo2_WaterFreq,								            /*水浸传感器信号频率*/
    
    eIMEUBoardInfo2_Pres1,								            /*1#通道压力传感器电压*/
    eIMEUBoardInfo2_Pres2,								            /*2#通道压力传感器电压*/
    eIMEUBoardInfo2_Pres3,								            /*3#通道压力传感器电压*/
    eIMEUBoardInfo2_Pres4,								            /*4#通道压力传感器电压*/
    eIMEUBoardInfo2_Pres5,								            /*5#通道压力传感器电压*/
    eIMEUBoardInfo2_Pres6,								            /*6#通道压力传感器电压*/
    
    eIMEUBoardInfo2_PWM1,								            /*1#通道PWM频率*/
    eIMEUBoardInfo2_PWM2,								            /*2#通道PWM频率*/
    eIMEUBoardInfo2_PWM3,								            /*3#通道PWM频率*/
    eIMEUBoardInfo2_PWM4,								            /*4#通道PWM频率*/
    eIMEUBoardInfo2_PWM5,								            /*5#通道PWM频率*/
    eIMEUBoardInfo2_PWM6,								            /*6#通道PWM频率*/
    
    eIMEUBoardInfo2_SoftVer,								            /*软件版本*/
    eIMEUBoardInfo2_Reserved,								            /*预留*/

	eIMEUBoardInfo2_Len   			        				       /*IMEU3采样信息二长度*/
}e_IMEU3BoardInfo2;

/*IMEU3DI1_16bit位对应信息(eIMEUBoardInfo2_DI1_16)*/
typedef enum
{
    eIMEUBoardInfoDI_Reserved = 0,                  /*保留DI1*/
	eIMEUBoardInfoDI_Spd1,                    	    /*主回路防雷器DI2*/
	eIMEUBoardInfoDI_Door,                    	    /*门禁告警DI3*/
    eIMEUBoardInfoDI_Smog,                          /*烟感告警DI4*/
    eIMEUBoardInfoDI_Ffd,                           /*气溶胶告警DI5*/
    eIMEUBoardInfoDI_Spd2,                          /*支路防雷器DI6*/
	eIMEUBoardInfoDI_DcPow,                    	    /*直流辅源状态反馈DI5*/

    eIMEUBoardInfoDI_Bit                         	/*IMEU3DIbit位长度*/
}e_IMEUBoardInfoDiBit;


/*协议子索引列表，IMEU3板级信息*/
typedef enum
{
	eIMEUAlmInfo_Alm1 = 0,								    /*告警信息1*/
	eIMEUAlmInfo_Alm2,								        /*告警信息2*/
	eIMEUAlmInfo_Alm3,								        /*告警信息3*/
    //eIMEUAlmInfo_Reserved,                                  /*预留*/

	eIMEUAlmInfo_Len   			        				    /*IMEU3告警信息长度*/
} e_IMEU3AlmInfo;

/*=============================================================================================
** @Name      : IMEU3设备信息枚举 (mIndex=1, u8, 10项)
** @Function  : 设备标识、版本信息、运行时间
**===========================================================================================*/
typedef enum
{
    eIMEUDevAddr_DevId = 0,                                         /*设备标识*/
    eIMEUDevAddr_DevAddr,                                           /*设备地址*/
    eIMEUDevAddr_HwVer,                                             /*硬件版本*/
    eIMEUDevAddr_SwVerL,                                            /*软件版本L*/
    eIMEUDevAddr_SwVerH,                                            /*软件版本H*/
    eIMEUDevAddr_BSPVer,                                            /*BSP版本*/
    eIMEUDevAddr_CANVer,                                            /*CAN通讯包版本*/
    eIMEUDevAddr_UARTVer,                                           /*UART通讯包版本*/
    eIMEUDevAddr_RunTimeL,                                          /*软件运行时间L*/
    eIMEUDevAddr_RunTimeH,                                          /*软件运行时间H*/

    eIMEUDevAddr_Len                                                /**/
} e_IMEUDevAddr;



/*=============================================================================================
** @Name      : IMEU3单板序列号枚举 (mIndex=2 subIdx 7-22, u8, ASCII)
** @Function  : 单板序列号字符(最高字符在低位)
**===========================================================================================*/
typedef enum
{
    eIMEU3DevSerial_Serial1 = 0,                                    /*设备序列号1(最高字符)*/
    eIMEU3DevSerial_Serial2,                                        /*设备序列号2*/
    eIMEU3DevSerial_Serial3,                                        /*设备序列号3*/
    eIMEU3DevSerial_Serial4,                                        /*设备序列号4*/
    eIMEU3DevSerial_Serial5,                                        /*设备序列号5*/
    eIMEU3DevSerial_Serial6,                                        /*设备序列号6*/
    eIMEU3DevSerial_Serial7,                                        /*设备序列号7*/
    eIMEU3DevSerial_Serial8,                                        /*设备序列号8*/
    eIMEU3DevSerial_Serial9,                                        /*设备序列号9*/
    eIMEU3DevSerial_Serial10,                                       /*设备序列号10*/
    eIMEU3DevSerial_Serial11,                                       /*设备序列号11*/
    eIMEU3DevSerial_Serial12,                                       /*设备序列号12*/
    eIMEU3DevSerial_Serial13,                                       /*设备序列号13*/
    eIMEU3DevSerial_Serial14,                                       /*设备序列号14*/
    eIMEU3DevSerial_Serial15,                                       /*设备序列号15*/
    eIMEU3DevSerial_Serial16,                                       /*设备序列号16*/
    eIMEU3DevSerial_Serial17,                                       /*设备序列号17*/
    eIMEU3DevSerial_Serial18,                                       /*设备序列号18*/
    eIMEU3DevSerial_Serial19,                                       /*设备序列号19*/
    eIMEU3DevSerial_Serial20,                                       /*设备序列号20*/
    eIMEU3DevSerial_Serial21,                                       /*设备序列号21*/
    eIMEU3DevSerial_Serial22,                                       /*设备序列号22*/
    eIMEU3DevSerial_Serial23,                                       /*设备序列号23(最低字符)*/

    eIMEUDevSerial_Len                                              /**/
} e_IMEU3DevSerial;

/*=============================================================================================
** @Name      : IMEU3系统状态枚举 (mIndex=50, u16, 1项)
**===========================================================================================*/
typedef enum
{
    eIMEU3SysStat_Mode = 0,                                         /*系统工作模式*/
    eIMEU3SysStat_Len                                               /**/
} e_IMEU3SysStatus;

/*=============================================================================================
** @Name      : IMEU3常规参数枚举 (mIndex=102/110, u16)
** @Function  : 温度通道/PWM/可燃气体/除湿机等配置参数
**===========================================================================================*/
typedef enum
{
    eIMEU3Cfg_CfgFlag = 0,                                          /*常规参数配置标志*/
    eIMEU3Cfg_CANBaud,                                              /*CAN通信波特率*/
    eIMEU3Cfg_485Baud,                                              /*RS485通信波特率*/
    eIMEU3Cfg_CANVer,                                               /*CAN控制器版本*/
    eIMEU3Cfg_TempCh,                                               /*温度通道参数配置*/
    eIMEU3Cfg_DICh,                                                 /*DI通道参数配置*/
    eIMEU3Cfg_PWM,                                                  /*PWM参数配置*/
    eIMEU3Cfg_Press,                                                /*压力传感器配置*/
    eIMEU3Cfg_Gas,                                                  /*可燃气体传感器配置*/
    eIMEU3Cfg_485Num,                                               /*485温湿度传感器个数*/
    eIMEU3Cfg_Dehum,                                                /*除湿机配置*/
    eIMEU3Cfg_Light,                                                /*灯带配置*/
    eIMEU3Cfg_Explos,                                               /*泄爆阀配置*/

    eIMEU3Cfg_Len                                                   /*IMEU3常规参数长度 */
} e_IMEU3CfgPara;

/*协议子索引列表，用户控制命令数组*/
typedef enum
{
	eUserCtrlCmd69_1LeakCheck,					/*组1绝缘检测控制命令 0:自动检测 1:开始检测 2:停止检测 3:无动作*/
    eUserCtrlCmd69_2LeakCheck,					/*组2绝缘检测控制命令 0:自动检测 1:开始检测 2:停止检测 3:无动作*/

	eUserCtrlCmd69_Len   			        				    /*控制结果状态信息数组长度*/
}e_UserCtrlCmd69;

/*协议子索引列表，控制命令反馈信息数组*/
typedef enum
{
	eCtrlCmdInfo70_1LeakCheckResult,					/*组1绝缘检测控制结果 0:自动检测 1:正在检测 2:停止检测 3:检测完成 4:检测超时 5:检测失败*/
    eCtrlCmdInfo70_2LeakCheckResult,					/*组2绝缘检测控制结果 0:自动检测 1:正在检测 2:停止检测 3:检测完成 4:检测超时 5:检测失败*/

	eCtrlCmdInfo70_Len   			        				    /*控制结果状态信息数组长度*/
}e_CtrlCmdInfo70;

/*协议子索引列表，采样校准反馈信息数组(1/2对应IMIU4单元1/2)*/
typedef enum
{
	eSampCalFbInfo73_1Type = 0,					/*单元1采样校准类型 0:无校准 1~13:校准类型(电流零飘/霍尔/分流器/总压/Link/绝缘) */
	eSampCalFbInfo73_1State,					/*单元1采样校准状态 0:未校准 1~6:校准流程 100:校准成功 101:校准失败 */
	eSampCalFbInfo73_1Result,					/*单元1采样校准结果 按位表示,bit1~bit13对应校准类型 0:未校准 1:已校准 */
	eSampCalFbInfo73_2Type,					/*单元2采样校准类型 0:无校准 1~13:校准类型 */
	eSampCalFbInfo73_2State,					/*单元2采样校准状态 0:未校准 1~6:校准流程 100:校准成功 101:校准失败 */
	eSampCalFbInfo73_2Result,					/*单元2采样校准结果 按位表示,bit1~bit13对应校准类型 0:未校准 1:已校准 */

	eSampCalFbInfo73_Len   			        				    /*采样校准反馈信息数组长度*/
}e_SampCalFbInfo73;

/*协议子索引列表，BSU02U2板级及PACK采样信息 ICC索引=74（从控板级数据转发，填充未实现，上报恒0）*/
typedef enum
{
	eBsudPSamp74_Pt1000Self1 = 0,		/*第1路PT1000自检温度值 1℃ [-40,200]*/
	eBsudPSamp74_Pt1000Self2,			/*第2路PT1000自检温度值 1℃ [-40,200]*/
	eBsudPSamp74_WaterSelf1,			/*第1路水浸自检频率值 1Hz [0,32767]*/
	eBsudPSamp74_WaterSelf2,			/*第2路水浸自检频率值 1Hz [0,32767]*/
	eBsudPSamp74_PullUp5V,				/*PT1000上拉电压5V回检值 0.01V [0,500]*/
	eBsudPSamp74_Out12V,				/*12V电压输出回检值 0.01V [0,3600]*/
	eBsudPSamp74_Rsv1,					/*预留(子索引6,协议未定义字段)*/
	eBsudPSamp74_Rsv2,					/*预留(子索引7)*/
	eBsudPSamp74_Rsv3,					/*预留(子索引8)*/
	eBsudPSamp74_Rsv4,					/*预留(子索引9)*/
	eBsudPSamp74_Pt1000_1,				/*第1路PT1000温度值 1℃ [-40,200]*/
	eBsudPSamp74_Pt1000_2,				/*第2路PT1000温度值 1℃ [-40,200]*/
	eBsudPSamp74_Water1,				/*第1路水浸频率值 1Hz [0,32767]*/
	eBsudPSamp74_Water2,				/*第2路水浸频率值 1Hz [0,32767]*/
	eBsudPSamp74_HvInterlockAmp,		/*高压互锁PWM幅值 0.01V [0,500]*/
	eBsudPSamp74_HvInterlockDuty,		/*高压互锁PWM占空比 0.01 [0,100]*/
	eBsudPSamp74_FgAmp,					/*FG检测PWM幅值 0.01V [0,500]*/
	eBsudPSamp74_FgDuty,				/*FG检测PWM占空比 0.01 [0,100]*/
	eBsudPSamp74_AerosolConc,			/*气溶胶当前浓度值 0.01 [0,65535]*/
	eBsudPSamp74_AerosolType,			/*气溶胶传感器类型 [0,10]*/
	eBsudPSamp74_AerosolStat,			/*气溶胶状态及故障信息 [0,65535]*/
	eBsudPSamp74_AfeBotNum,				/*AFE BOT端枚举数量*/
	eBsudPSamp74_AfeTopNum,				/*AFE TOP端枚举数量*/
	eBsudPSamp74_Rsv5,					/*预留(子索引23)*/
	eBsudPSamp74_Rsv6,					/*预留(子索引24)*/
	eBsudPSamp74_Rsv7,					/*预留(子索引25)*/
	eBsudPSamp74_Rsv8,					/*预留(子索引26)*/
	eBsudPSamp74_Rsv9,					/*预留(子索引27)*/
	eBsudPSamp74_Rsv10,					/*预留(子索引28)*/
	eBsudPSamp74_Rsv11,					/*预留(子索引29)*/
	eBsudPSamp74_Rsv12,					/*预留(子索引30)*/
	eBsudPSamp74_Rsv13,					/*预留(子索引31)*/

	eBsudPSamp74_Len					/*BSU02U2板级及PACK采样信息数组长度*/
}e_BsudPSamp74;

/*协议子索引列表，IMEU3硬件参数 ICC索引=75(当前值RO)/76(配置值WR)，字段一致共用枚举
** 值域以 IMEU3 外部模块协议为准，SEC 透传 */
typedef enum
{
	eImeu3Cfg_CanBaud = 0,			/*CAN通信波特率 [0,3]*/
	eImeu3Cfg_Rs485Baud,			/*RS485通信波特率 [0,4]*/
	eImeu3Cfg_CanVer,				/*CAN控制器版本 [0,1]*/
	eImeu3Cfg_TempChCfg,			/*温度通道参数配置 [0,31]*/
	eImeu3Cfg_DiChCfg,				/*DI通道参数配置 [0,63]*/
	eImeu3Cfg_PwmCfg,				/*PWM参数配置 [0,2]*/
	eImeu3Cfg_PressSens,			/*压力传感器配置 [0,2]*/
	eImeu3Cfg_GasSens,				/*可燃气体传感器配置 [0,4]*/
	eImeu3Cfg_Sen485Num,			/*485温湿度传感器个数配置 [0,4]*/
	eImeu3Cfg_DehumCfg,			/*除湿机配置 [0,2]*/
	eImeu3Cfg_LightCfg,			/*灯带配置 [0,1]*/
	eImeu3Cfg_VentValveCfg,		/*泄爆阀配置 [0,1]*/

	eImeu3Cfg_Len					/*IMEU3硬件参数字段数(75/76共用)*/
}e_Imeu3Cfg;

/*协议子索引列表，组二IMIU4信息数组(子索引0~16采样,17工作模式,18~20告警,21~23故障)*/
/*协议子索引列表，组一IMIU4信息 ICC索引=67（字段布局与组二68一致）*/
typedef enum
{
	eImiu4Info67_BatVolt = 0,					/*电池总压采样值 0.1V*/
	eImiu4Info67_LinkVolt,					/*Link电压采样值 0.1V*/
	eImiu4Info67_Hall5V,						/*霍尔5V采样值 0.1V*/
	eImiu4Info67_HallSamp,					/*霍尔信号采样值 0.1V*/
	eImiu4Info67_HallCurrL,					/*霍尔电流低 0.001A(与高字组成s32)*/
	eImiu4Info67_HallCurrH,					/*霍尔电流高 0.001A*/
	eImiu4Info67_ShuntCurrL,					/*分流器电流低 0.001A(与高字组成s32)*/
	eImiu4Info67_ShuntCurrH,					/*分流器电流高 0.001A*/
	eImiu4Info67_ShuntT,						/*分流器温度 0.1℃*/
	eImiu4Info67_Afe5V1,						/*高压采集芯片+5V回检 0.1V*/
	eImiu4Info67_Afe5V2,						/*高压采集芯片-5V回检 0.1V*/
	eImiu4Info67_PasInsu,					/*正极绝缘阻值 1kΩ 初始值0(未检测)*/
	eImiu4Info67_NegInsu,					/*负极绝缘阻值 1kΩ 初始值0(未检测)*/
	eImiu4Info67_Ntc1T,						/*NTC1温度 0.1℃*/
	eImiu4Info67_Ntc2T,						/*NTC2温度 0.1℃*/
	eImiu4Info67_GpioSta,					/*DIDO及GPIO信号值 bit0~bit14*/
	eImiu4Info67_Board5V,					/*板子供电5V回检值 0.1V*/
	eImiu4Info67_WorkMode,					/*系统工作模式 0:正常 1:工装 5:调试*/
	eImiu4Info67_AlmInfo1,					/*告警信息1 按位表示*/
	eImiu4Info67_AlmInfo2,					/*告警信息2 按位表示*/
	eImiu4Info67_AlmInfo3,					/*告警信息3(预留)*/
	eImiu4Info67_FaultInfo1,					/*故障信息1 按位表示*/
	eImiu4Info67_FaultInfo2,					/*故障信息2 按位表示*/
	eImiu4Info67_FaultInfo3,					/*故障信息3(预留)*/

	eImiu4Info67_Len   			        				    /*组一IMIU4信息数组长度*/
}e_Imiu4Info67;

typedef enum
{
	eImiu4Info68_BatVolt = 0,					/*电池总压采样值 0.1V*/
	eImiu4Info68_LinkVolt,					/*Link电压采样值 0.1V*/
	eImiu4Info68_Hall5V,						/*霍尔5V采样值 0.1V*/
	eImiu4Info68_HallSamp,					/*霍尔信号采样值 0.1V*/
	eImiu4Info68_HallCurrL,					/*霍尔电流低 0.001A(与高字组成s32)*/
	eImiu4Info68_HallCurrH,					/*霍尔电流高 0.001A*/
	eImiu4Info68_ShuntCurrL,					/*分流器电流低 0.001A(与高字组成s32)*/
	eImiu4Info68_ShuntCurrH,					/*分流器电流高 0.001A*/
	eImiu4Info68_ShuntT,						/*分流器温度 0.1℃*/
	eImiu4Info68_Afe5V1,						/*高压采集芯片+5V回检 0.1V*/
	eImiu4Info68_Afe5V2,						/*高压采集芯片-5V回检 0.1V*/
	eImiu4Info68_PasInsu,					/*正极绝缘阻值 1kΩ 初始值0(未检测)*/
	eImiu4Info68_NegInsu,					/*负极绝缘阻值 1kΩ 初始值0(未检测)*/
	eImiu4Info68_Ntc1T,						/*NTC1温度 0.1℃*/
	eImiu4Info68_Ntc2T,						/*NTC2温度 0.1℃*/
	eImiu4Info68_GpioSta,					/*DIDO及GPIO信号值 bit0~bit14*/
	eImiu4Info68_Board5V,					/*板子供电5V回检值 0.1V*/
	eImiu4Info68_WorkMode,					/*系统工作模式 0:正常 1:工装 5:调试*/
	eImiu4Info68_AlmInfo1,					/*告警信息1 按位表示*/
	eImiu4Info68_AlmInfo2,					/*告警信息2 按位表示*/
	eImiu4Info68_AlmInfo3,					/*告警信息3(预留)*/
	eImiu4Info68_FaultInfo1,					/*故障信息1 按位表示*/
	eImiu4Info68_FaultInfo2,					/*故障信息2 按位表示*/
	eImiu4Info68_FaultInfo3,					/*故障信息3(预留)*/

	eImiu4Info68_Len   			        				    /*组二IMIU4信息数组长度*/
}e_Imiu4Info68;


//整体信息
/*协议子索引列表，系统状态信息*/
typedef enum
{
    eSysStateInfo50_1SysPro = 0,                                   /*组1系统保护状态[0,5]		0:未就绪 1:正常 2:充电截止 3:放电截止 4:充放电限制 5:充放电保护*/
    eSysStateInfo50_1SysWor,                                       /*组1系统工作状态[0,6]		0:开机,1:待命,2:启动,3:运行,4:停机,5:故障 6:关机*/
    eSysStateInfo50_1SysChg,                                       /*组1电池充放电状态[0,2]	    0:空闲 1:充电 2:放电*/
    eSysStateInfo50_1SysMod,                                       /*组1系统工作模式[0,5]		0:自动 1:手动 2:紧急 5:调试*/
    
    eSysStateInfo50_2SysPro,                                       /*组2系统保护状态[0,5]		0:未就绪 1:正常 2:充电截止 3:放电截止 4:充放电限制 5:充放电保护*/
    eSysStateInfo50_2SysWor,                                       /*组2系统工作状态[0,6]		0:开机,1:待命,2:启动,3:运行,4:停机,5:故障 6:关机*/
    eSysStateInfo50_2SysChg,                                       /*组2电池充放电状态[0,2]	    0:空闲 1:充电 2:放电*/
    eSysStateInfo50_2SysMod,                                       /*组2系统工作模式[0,5]		0:自动 1:手动 2:紧急 5:调试*/

    eSysStateInfo50_Len   			        				        /*系统状态信息长度*/
}e_SysStateInfo50;

/*协议子索引列表，系统整体信息*/
typedef enum
{
    eSysMainInfo51_1SumV = 0,                                   /*组1总电压采样值[0,15000]0.1V*/
    eSysMainInfo51_1OutC,                                       /*组1总电流采样值[-30000,30000]0.1A*/
    eSysMainInfo51_1OutP,                                       /*组1输出功率值[-30000,30000]0.1kW*/
    eSysMainInfo51_1PreV,                                       /*组1预充电压值[0,15000]0.1V*/

    eSysMainInfo51_2SumV,                                       /*组2总电压采样值[0,15000]0.1V*/
    eSysMainInfo51_2OutC,                                       /*组2总电流采样值[-30000,30000]0.1A*/
    eSysMainInfo51_2OutP,                                       /*组2输出功率值[-30000,30000]0.1kW*/
    eSysMainInfo51_2PreV,                                       /*组2预充电压值[0,15000]0.1V*/
    
    eSysMainInfo51_Len                                          /*系统整体信息长度*/
}e_SysMainInfo51;

/*协议子索引列表，系统电量信息*/
typedef enum
{
    eSysCapInfo52_1SOC = 0,                                       /*组1电池SOC值[0,1000]0.1%*/
    eSysCapInfo52_1SOE,                                           /*组1电池SOE值[0,1000]0.1%*/
    eSysCapInfo52_1SOH,                                           /*组1电池SOH值[0,1000]0.1%*/
    eSysCapInfo52_1AllCap,                                        /*组1电池组总容量[0,10000]0.1AH*/

    eSysCapInfo52_2SOC,                                            /*组2电池SOC值[0,1000]0.1%*/
    eSysCapInfo52_2SOE,                                            /*组2电池SOE值[0,1000]0.1%*/
    eSysCapInfo52_2SOH,                                            /*组2电池SOH值[0,1000]0.1%*/
    eSysCapInfo52_2AllCap,                                         /*组2电池组总容量[0,10000]0.1AH*/
    
    eSysCapInfo52_Len                                              /*系统电量信息长度*/
}e_SysCapInfo52;

/*协议子索引列表，系统限值信息*/
typedef enum
{
    eSysLimInfo53_1ChgC = 0,                                       /*组1最大允许充电电流限值(PCS充电输入最大电流)[0,10000]0.1A*/
    eSysLimInfo53_1DhgC,                                           /*组1最大允许放电电流限值(PCS放电输出最大电流)[0,10000]0.1A*/
    eSysLimInfo53_1ChgP,                                           /*组1最大允许充电功率限值(PCS充电输入最大功率)[0,10000]0.1kW*/
    eSysLimInfo53_1DhgP,                                           /*组1最大允许放电功率限值(PCS放电输出最大功率)[0,10000]0.1kW*/
    eSysLimInfo53_1ChgV,                                           /*组1最大允许充电电压限值(PCS充电截止电压)[0,15000]0.1V*/
    eSysLimInfo53_1DhgV,                                           /*组1最大允许放电电压限值(PCS放电截止电压)[0,15000]0.1V*/

    eSysLimInfo53_2ChgC,                                           /*组2最大允许充电电流限值(PCS充电输入最大电流)[0,10000]0.1A*/
    eSysLimInfo53_2DhgC,                                           /*组2最大允许放电电流限值(PCS放电输出最大电流)[0,10000]0.1A*/
    eSysLimInfo53_2ChgP,                                           /*组2最大允许充电功率限值(PCS充电输入最大功率)[0,10000]0.1kW*/
    eSysLimInfo53_2DhgP,                                           /*组2最大允许放电功率限值(PCS放电输出最大功率)[0,10000]0.1kW*/
    eSysLimInfo53_2ChgV,                                           /*组2最大允许充电电压限值(PCS充电截止电压)[0,15000]0.1V*/
    eSysLimInfo53_2DhgV,                                           /*组2最大允许放电电压限值(PCS放电截止电压)[0,15000]0.1V*/


    
    eSysLimInfo53_Len                                              /*系统限值信息长度*/
}e_SysLimInfo53;

/*协议子索引列表，系统特征信息*/
typedef enum
{
    eSysCharnfo54_1SumV = 0,                                       /*组1单体总电压值[0,15000]0.1V*/
    eSysCharnfo54_1AvgV,                                           /*组1单体平均电压值[0,5000]1mV*/
    eSysCharnfo54_1AvgT,                                           /*组1单体平均温度值[-400,1250]0.1℃*/
    eSysCharnfo54_1EnvT,                                           /*组1系统环境温度值[-400,1250]0.1℃*/
    eSysCharnfo54_1MaxVNum,                                        /*组1单体最大电压序号[1,400]*/
    eSysCharnfo54_1MaxVolt,                                        /*组1单体最大电压值[0,5000]1mV*/
    eSysCharnfo54_1MinVNum,                                        /*组1单体最小电压序号[1,400]*/
    eSysCharnfo54_1MinVolt,                                        /*组1单体最小电压值[0,5000]1mV*/
    eSysCharnfo54_1MaxTNum,                                        /*组1单体最高温度序号[1,400]*/
    eSysCharnfo54_1MaxTemp,                                        /*组1单体最高温度值[-400,1250]0.1℃*/
    eSysCharnfo54_1MinTNum,                                        /*组1单体最低温度序号[1,400]*/
    eSysCharnfo54_1MinTemp,                                        /*组1单体最低温度值[-400,1250]0.1℃*/

    eSysCharnfo54_2SumV,                                           /*组2单体总电压值[0,15000]0.1V*/
    eSysCharnfo54_2AvgV,                                           /*组2单体平均电压值[0,5000]1mV*/
    eSysCharnfo54_2AvgT,                                           /*组2单体平均温度值[-400,1250]0.1℃*/
    eSysCharnfo54_2EnvT,                                           /*组2系统环境温度值[-400,1250]0.1℃*/
    eSysCharnfo54_2MaxVNum,                                        /*组2单体最大电压序号[1,400]*/
    eSysCharnfo54_2MaxVolt,                                        /*组2单体最大电压值[0,5000]1mV*/
    eSysCharnfo54_2MinVNum,                                        /*组2单体最小电压序号[1,400]*/
    eSysCharnfo54_2MinVolt,                                        /*组2单体最小电压值[0,5000]1mV*/
    eSysCharnfo54_2MaxTNum,                                        /*组2单体最高温度序号[1,400]*/
    eSysCharnfo54_2MaxTemp,                                        /*组2单体最高温度值[-400,1250]0.1℃*/
    eSysCharnfo54_2MinTNum,                                        /*组2单体最低温度序号[1,400]*/
    eSysCharnfo54_2MinTemp,                                        /*组2单体最低温度值[-400,1250]0.1℃*/

    eSysCharInfo54_Len                                              /*系统特征信息长度*/
}e_SysCharInfo54;

/*协议子索引列表，当前能量统计信息*/
typedef enum
{
    eCurrCapEner55_1NowEner = 0,                                       /*组1电池组当前电能[0,65535]0.1kWh*/
    eCurrCapEner55_1AllEner,                                           /*组1电池组总电能[0,65535]0.1kWh*/
    eCurrCapEner55_1ChgCap,                                            /*组1电池组单次充电电量[0,65535]1AH*/
    eCurrCapEner55_1DhgCap,                                            /*组1电池组单次放电电量[0,65535]1AH*/

    eCurrCapEner55_2NowEner,                                           /*组2电池组当前电能[0,65535]0.1kWh*/
    eCurrCapEner55_2AllEner,                                           /*组2电池组总电能[0,65535]0.1kWh*/
    eCurrCapEner55_2ChgCap,                                            /*组2电池组单次充电电量[0,65535]1AH*/
    eCurrCapEner55_2DhgCap,                                            /*组2电池组单次放电电量[0,65535]1AH*/

    eCurrCapEner55_Len                                                  /*系统当前能量统计信息长度*/
}e_CurrCapEnerInfo55;



/*协议子索引列表，历史能量统计信息*/
typedef enum
{
    eHisCapEner56_1HisChgCap = 0,                                            /*电池组1累计充电电量[0,4294967295]1AH*/
    eHisCapEner56_1HisDhgCap,                                                /*电池组1累计放电电量[0,4294967295]1AH*/
    eHisCapEner56_1HisChgEner,                                               /*电池组1累计充电电能[0,4294967295]0.1kWh*/
    eHisCapEner56_1HisDhgEner,                                               /*电池组1累计放电电能[0,4294967295]0.1kWh*/

    eHisCapEner56_2HisChgCap,                                                /*电池组1累计充电电量[0,4294967295]1AH*/
    eHisCapEner56_2HisDhgCap,                                                /*电池组1累计放电电量[0,4294967295]1AH*/
    eHisCapEner56_2HisChgEner,                                               /*电池组1累计充电电能[0,4294967295]0.1kWh*/
    eHisCapEner56_2HisDhgEner,                                               /*电池组1累计放电电能[0,4294967295]0.1kWh*/

    eHisCapEner56_Len                                                        /*系统历史能量统计信息长度*/
}e_HisCapEnerInfo56;

/*协议子索引列表，故障告警信息*/
typedef enum
{
    eAlmFault_Alm1 = 0,                                                      /*告警信息1*/
    eAlmFault_Alm2,                                                          /*告警信息2*/
    eAlmFault_Alm3,                                                          /*告警信息3*/
    eAlmFault_Alm4,                                                          /*告警信息4*/
    eAlmFault_Alm5,                                                          /*告警信息5*/
    eAlmFault_Alm6,                                                          /*告警信息6*/
    eAlmFault_Alm7,                                                          /*告警信息7*/
    eAlmFault_Alm8,                                                          /*告警信息8*/
    eAlmFault_Alm9,                                                          /*告警信息9*/
    eAlmFault_Alm10,                                                         /*告警信息10*/
    eAlmFault_Fault1,                                                        /*故障信息1*/
    eAlmFault_Fault2,                                                        /*故障信息2*/
    eAlmFault_Fault3,                                                        /*故障信息3*/
    eAlmFault_Fault4,                                                        /*故障信息4*/
    eAlmFault_Fault5,                                                        /*故障信息5*/
    eAlmFault_Fault6,                                                        /*故障信息6*/
    eAlmFault_Fault7,                                                        /*故障信息7*/
    eAlmFault_Fault8,                                                        /*故障信息8*/

    eAlmFault_Len                                                            /*故障告警信息长度*/
}e_AlmFaultInfo5758;

/*协议子索引列表，时间统计信息*/
typedef enum
{
	eTime59_1SysRun = 0,						/*组1系统运行时长[0,65535]1Hour*/
	eTime59_1CanChg,						    /*组1可充电时间[0,65535]1Min*/
	eTime59_1CanDhg,						    /*组1可放电时间[0,65535]1Min*/
	eTime59_1ChgCycle,				            /*组1充放电循环次数[0,65535]1次*/

    eTime59_2SysRun,						    /*组2系统运行时长[0,65535]1Hour*/
	eTime59_2CanChg,						    /*组2可充电时间[0,65535]1Min*/
	eTime59_2CanDhg,						    /*组2可放电时间[0,65535]1Min*/
	eTime59_2ChgCycle,				            /*组2充放电循环次数[0,65535]1次*/

    eTime59_Len                                 /*时间统计信息长度*/

}e_StatistTime59;

/*协议子索引列表，故障统计信息*/
typedef enum
{
	eErr60_1MonWarNum = 0,					/*组1单月告警数目[0,65535]*/
	eErr60_1MonErrNum,					    /*组1单月故障数目[0,65535]*/
	eErr60_1HisWarNum,					    /*组1累计告警数目[0,65535]*/
	eErr60_1HisErrNum,					    /*组1累计故障数目[0,65535]*/

    eErr60_2MonWarNum,  					/*组2单月告警数目[0,65535]*/
	eErr60_2MonErrNum,					    /*组2单月故障数目[0,65535]*/
	eErr60_2HisWarNum,					    /*组2累计告警数目[0,65535]*/
	eErr60_2HisErrNum,					    /*组2累计故障数目[0,65535]*/

    eErr60_Len                              /*故障统计信息长度*/

}e_StatistErr60;

/*协议子索引列表，电池安全等级统计信息*/
typedef enum
{
	eDiagSos61_1NowSos = 0,				   /*组1当前安全等级SOS[0,4](等级越低越安全)*/
	eDiagSos61_1NowRsn,				       /*组1当前SOS等级异常原因[按bit位表示]*/
	eDiagSos61_1PreSos,				       /*组1预测安全等级SOS[0,4](等级越低越安全)*/
	eDiagSos61_1PreRsn,				       /*组1预测SOS等级异常原因[按bit位表示]*/

    eDiagSos61_2NowSos,				       /*组2当前安全等级SOS[0,4](等级越低越安全)*/
	eDiagSos61_2NowRsn,				       /*组2当前SOS等级异常原因[按bit位表示]*/
	eDiagSos61_2PreSos,				       /*组2预测安全等级SOS[0,4](等级越低越安全)*/
	eDiagSos61_2PreRsn,				       /*组2预测SOS等级异常原因[按bit位表示]*/

    eDiagSos61_Len                         /*电池安全等级统计信息长度*/


}e_StatistSosInfo61;


/*协议子索引列表，放电深度统计信息*/
typedef enum
{
	eDodStat62_1NowDod = 0,				   /*最新放电深度DOD(单次)[0,1000]0.1%*/
	eDodStat62_1FstDod,				       /*最新放电起始DOD(单次)[0,1000]0.1%*/
	eDodStat62_1MaxDod,				       /*最大放电深度DOD(单次)[0,1000]0.1%*/
	eDodStat62_1AvgDod,				       /*平均放电深度DOD(平均)[0,1000]0.1%*/

    eDodStat62_2NowDod,				       /*最新放电深度DOD(单次)[0,1000]0.1%*/
	eDodStat62_2FstDod,				       /*最新放电起始DOD(单次)[0,1000]0.1%*/
	eDodStat62_2MaxDod,				       /*最大放电深度DOD(单次)[0,1000]0.1%*/
	eDodStat62_2AvgDod,				       /*平均放电深度DOD(平均)[0,1000]0.1%*/

    eDodStat62_Len                         /*电池安全等级统计信息长度*/

}e_StatistDodInfo62;

/*历史DOD环形缓冲内部布局(非协议表,协议出口为mIndex15/16单条推送)*/
typedef enum
{
	eHisDodRing_Flag = 0,                       /*配置标志(E2有效标志PARA_CFG_FLAG_U16,RAM镜像同步保留)*/
	eHisDodRing_Id,                             /*全局自增序号(=累计记录次数,清零复位;有效条数=min(Id,HISDOD_REC_NUM))*/
	eHisDodRing_Rec,                            /*记录区起始(每条10个u16,共HISDOD_REC_NUM条)*/

	eHisDodRing_Len = HISDOD_RING_LEN           /*历史DOD环形缓冲信息长度*/
}e_HisDodRingInfo;

/*单条记录内部偏移(相对记录起始),与协议mIndex15/16子索引0~9一一对应*/
typedef enum
{
	eHisDodRec_Id = 0,                          /*子0:记录序号(全局自增,A核靠它去重/排序/检测丢帧)*/
	eHisDodRec_Y,                               /*子1:记录年份*/
	eHisDodRec_MD,                              /*子2:记录月日(month<<8|day)*/
	eHisDodRec_HM,                              /*子3:记录时分(hour<<8|minute)*/
	eHisDodRec_S,                               /*子4:记录秒*/
	eHisDodRec_Dod,                             /*子5:DOD值[0,1000]0.1%(协议原子5,笔误前移)*/
	eHisDodRec_DhgCap,                          /*子6:放电量(单次放电过程差值,单位同mIndex56累计电量)*/
	eHisDodRec_ChgCap,                          /*子7:充电量(单次放电过程差值,单位同mIndex56累计电量)*/
	eHisDodRec_DhgEner,                         /*子8:放电能(单次放电过程差值,单位同mIndex56累计电能)*/
	eHisDodRec_ChgEner,                         /*子9:充电能(单次放电过程差值,单位同mIndex56累计电能)*/
	eHisDodRec_Len                              /*单条记录长度(10个u16)*/
}e_HisDodRec;

/*协议子索引列表，系统控制命令*/
typedef enum
{
	eSysCtrlCmd_WorkSta = 0,			       /*工作状态控制*/
	eSysCtrlCmd_WorkMode,				       /*工作模式控制*/
	eSysCtrlCmd_SysClear,				       /*系统清除命令*/
    //eSysCtrlCmd_Reserved,                      /*预留*/

    eSysCtrlCmd_Len                            /*系统控制命令信息长度*/

}e_SysCtrlCmd6566;

/*协议71子索引枚举 gSysDbgCmd_71[] 用
** 每个子索引为u16类型,含义随eDbgCmd71_DbgType变化
** 操作流程: A核先RUP写sub0-8缓存参数,再RUP写eDbgCmd71_Affirm=1/2确认执行
** 2026-07-28: 同步BMU05核间通信协议 vA02
*/
typedef enum
{
    eDbgCmd71_DbgType   = 0,                                /*调试类型:0=BSU均衡 1=BSU E2P 2=BSU AFE 3=BSU内阻 4=R核接触器 5=R核干接点 6=R核E2P 7=EU3外设*/
    eDbgCmd71_CluNum    = 1,                                /*簇编号:0=所有簇 1=第一簇 2=第二簇*/
    eDbgCmd71_DevNum    = 2,                                /*调试设备地址:0-16=BSU地址, IMIU4#1:20 IMIU4#2:21, EU3:40/41, R核:50*/
    eDbgCmd71_FuncP1    = 3,                                /*功能参数1:TYPE4接触器代号(0=全部,1=PK1,2=PK2,3=PK3,4=PK4) TYPE5干接点代号(0=全部,1=NK1,2=NK2) TYPE7外设编号(1泄爆阀排气扇1# 2泄爆阀排气扇2# 3泄爆阀电源1# 4泄爆阀电源2# 5除湿机 6消防 7声光告警 8烟感电源 9QS1 10远程急停)*/
    eDbgCmd71_FuncP2    = 4,                                /*功能参数2:TYPE4/5=0断开 1闭合, TYPE0/1/2/3/6见各自子协议, TYPE7外设=0关 1开*/
    eDbgCmd71_CellW0    = 5,                                /*数据1:TYPE0均衡bit[15:0] TYPE6 E2P数据L16 TYPE7按位风扇控制高16位(bit[31:16])*/
    eDbgCmd71_CellW1    = 6,                                /*数据2:TYPE0均衡bit[31:16] TYPE6 E2P数据H16 TYPE7按位风扇控制低16位(bit[15:0])*/
    eDbgCmd71_CellW2    = 7,                                /*数据3:TYPE0均衡bit[47:32](48/52串有效,16/32串填0) 其他类型填0*/
    eDbgCmd71_CellW3    = 8,                                /*数据4:TYPE0均衡bit[63:48](仅52串有效,bit48-51有效,bit52-63保留填0) 其他类型填0*/
    eDbgCmd71_Affirm    = 9                                 /*确认命令:1=确认功能调试/确认读EEPROM 2=确认写EEPROM(两段式:A核先写参数,再写本字段触发执行)*/
} e_SysDbgCmd71;

/*协议94子索引列表，PACK温度信息 (有符号 s16)*/
typedef enum
{
    ePackTemp94_Pt1SelfTemp = 0,                            /*PACK正极铜排温度*/
    ePackTemp94_Pt2SelfTemp,                                /*PACK负极铜排温度*/
    ePackTemp94_Pt1Temp,                                    /*PT1000温度1*/
    ePackTemp94_Pt2Temp,                                    /*PT1000温度2*/

    ePackTemp94_Len                                         /*PACK温度信息长度*/
}e_PackTemp94;

/*协议95子索引列表，PACK环境与综合状态信息 (无符号 u16)*/
typedef enum
{
    ePackStat95_WaterSelfHz1 = 0,                           /*水浸自检频率1*/
    ePackStat95_WaterSelfHz2,                               /*水浸自检频率2*/
    ePackStat95_Pt5VPullUp,                                 /*PT1000上拉5V回检*/
    ePackStat95_Out12VCheck,                                /*12V输出回检*/
    ePackStat95_WaterHz1,                                   /*水浸频率1*/
    ePackStat95_WaterHz2,                                   /*水浸频率2*/
    ePackStat95_HvilPwmAmp,                                 /*高压互锁PWM幅值*/
    ePackStat95_HvilPwmDuty,                                /*高压互锁PWM占空比*/
    ePackStat95_FgFreq,                                     /*FG检测频率/幅值*/
    ePackStat95_FgDuty,                                     /*FG检测占空比*/
    ePackStat95_AeroConc,                                   /*气溶胶浓度*/
    ePackStat95_AeroType,                                   /*气溶胶传感器类型*/
    ePackStat95_AeroState,                                  /*气溶胶状态和故障信息*/
    ePackStat95_AfeBotCount,                                /*AFE Bottom端枚举数量*/
    ePackStat95_AfeTopCount,                                /*AFE Top端枚举数量*/

    ePackStat95_Len                                         /*PACK环境与状态信息长度*/
}e_PackStat95;

typedef enum
{
    eAfeState_Sleep = 1,
    eAfeState_Standby = 3,
    eAfeState_Normal = 4,
    eAfeState_SelfTest = 6,
    eAfeState_Safe = 8,
    eAfeState_SpiNormal = 12,
}e_AfeState;

//从控常规配置参数
typedef enum
{
	eGBsuPara102_CfgFlag = 0,           //常规参数配置标志
	eGBsuPara102_EcuId,                 //从控设备地址

	eGBsuPara102_SlaveNum,              //组内从控数目
	eGBsuPara102_DaisyType,             //菊花链类型
	eGBsuPara102_DaisyNum,              //模组数量

	eGBsuPara102_SampVnum,              //一个模组电芯数量（电压）
	eGBsuPara102_SampTnum,              //一个模组电芯数量（温度）

	eGBsuPara102_TotalVnum,             //电压采样总数
	eGBsuPara102_TotalTnum,             //温度采样总数

	eGBsuPara102_PassiveDuty,           // 被动均衡占空比
	eGBsuPara102_ActiveDuty,            // 主动均衡占空比
	eGBsuPara102_BalanceMethod,         // 均衡方式
	eGBsuPara102_CANCtrlVer,            // CAN控制器版本
	eGBsuPara102_CANBaudRate,           // CAN通信波特率
	eGBsuPara102_CellShieldEn,          // 单电芯异常屏蔽使能
	eGBsuPara102_AFELowPowerMode,       // AFE低功耗模式选择

    eGBsuPara102_Fin                    //从控参数102子索引数
}e_GBsuParaCfg102;                      //从控参数102

/*协议212子索引列表，同飞空调用户参数信息*/
typedef enum
{
	eATFUserInfo212_StartMode = 0,							/*启动模式*/
	eATFUserInfo212_WorkMode,								/*工作模式*/
	eATFUserInfo212_ControlObj,								/*供回水控制目标*/
	eATFUserInfo212_ColdTemp,								/*制冷温度*/
	eATFUserInfo212_ColdDiff,								/*制冷温差*/
	eATFUserInfo212_HeatTemp,								/*制热温度*/
	eATFUserInfo212_HeatDiff,								/*制热温差*/

	eATFUserInfo212_Len   			        				/*同飞空调用户参数信息*/
}e_ATFUserInfo212;
/*协议213子索引列表，同飞空调状态信息*/
typedef enum
{
	eATFUserState213_RunSta = 0,							/*机组运行状态*/
	eATFUserState213_SwitchIn,								/*开关量输入*/
	eATFUserState213_Relays,								/*继电器输出*/
	eATFUserState213_FeedTemp,								/*供液温度，单位0.1℃*/
	eATFUserState213_ReturnTemp,							/*回液温度，单位0.1℃ */
	eATFUserState213_SteamTemp,								/*系统蒸进，单位0.1℃*/
	eATFUserState213__rsv_0006,								/* 寄存器空洞 0x0006 */
	eATFUserState213_EvapTemp,								/*蒸出温度，单位0.1℃*/
	eATFUserState213_AmbiTemp,								/*环境温度，单位0.1℃*/
	eATFUserState213_CondTemp,								/*系统冷凝温度，单位0.1℃*/
	eATFUserState213__rsv_000A,								/* 寄存器空洞 0x000A */
	eATFUserState213_OpenExpen,								/*节流电膨当前开度，单位步*/
	eATFUserState213_PumpSpeed,								/*水泵当前转速，单位1%*/
	eATFUserState213_ComSpeed,								/*压缩机当前转速，单位1RPS*/
	eATFUserState213_FanSpeed,								/*风机转速，单位1RPS*/
	eATFUserState213__rsv_000F,								/* 寄存器空洞 0x000F */
	eATFUserState213__rsv_0010,								/* 寄存器空洞 0x0010 */
	eATFUserState213__rsv_0011,								/* 寄存器空洞 0x0011 */
	eATFUserState213_SuppWatPre,							/*供水压力，单位0.1bar*/
	eATFUserState213_RetpWatPre,							/*回水压力，单位0.1bar*/
	eATFUserState213__rsv_0014,								/* 寄存器空洞 0x0014 */
	eATFUserState213_ExhPre,							    /*排气压力，单位0.1bar*/
	eATFUserState213_InvPower,							    /*变频器输出功率，单位0.01kW*/
	eATFUserState213_PumpRunHours,							/*水泵累计运行时间，单位h*/
	eATFUserState213_PumpRunCount,							/*水泵累计运行次数*/
	eATFUserState213_CompRunHours,							/*压缩机累计运行时间，单位h*/
	eATFUserState213_CompRunCount,							/*压缩机累计运行次数*/
	eATFUserState213_FanRunHours,							/*冷凝风机累计运行时间，单位h*/
	eATFUserState213_FanRunCount,							/*冷凝风机累计运行次数*/
	eATFUserState213_Pump2RunHours,							/*水泵2累计运行时间，单位h*/
	eATFUserState213_Pump2RunCount,							/*水泵2累计运行次数*/

	eATFUserState213_Len   			        				/*同飞空调状态信息*/
}e_ATFUserState;

/*协议214子索引列表，同飞空调故障信息*/
typedef enum
{
	eATFErrInfo214_ErrSta1 = 0,							    /*故障状态1*/
	eATFErrInfo214_ErrSta2,								    /*故障状态2*/
	eATFErrInfo214_ErrSta3,								    /*故障状态3*/
	eATFErrInfo214_ErrSta4,								    /*故障状态4*/
	eATFErrInfo214_ErrSta5,							        /*故障状态5*/
	eATFErrInfo214_ErrSta6,								 	/*故障状态6*/
	eATFErrInfo214_ErrSta7,								 	/*故障状态7*/
	eATFErrInfo214_FreConvert,								/*变频器故障代码*/
	eATFErrInfo214__rsv_0021,								/* 寄存器空洞 0x0021 */
	eATFErrInfo214_Level,								    /*系统总故障等级*/
	eATFErrInfo214_TotalCode,								/*系统总故障代码*/
	eATFErrInfo214_Fan1,								    /*变频风机故障1*/
	eATFErrInfo214_Fan2,								    /*变频风机故障2*/
	eATFErrInfo214_PFC,								        /*三相PFC故障号*/

	eATFErrInfo214_Len   			        				/*同飞空调故障信息长度*/
}e_ATFErrSta;






/*=============================================================================================
** @Name      : u8 GroupInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
u8 GroupInfoInit(void);
/*=================================================================================================
** @Name      : u8 GetGroupOffsetDevAddrAPI(void)
** @Input     : void
** @Output    : 电池组自身设备地址偏移量
** @Function  : 获取电池组自身偏移设备地址
** @The notes : 主控设备地址偏移量
**===============================================================================================*/
u8 GetGroupOffsetDevAddrAPI(void);
/*=============================================================================================
** @Name      : u16 GetBsuCellVolt(u8 ecu, u8 cell)
** @Input     : ecu:从控节点号[0,31] cell:从控内电池号[0,52]
** @Output    : 从控单体当前电压值:1mV
** @Function  : 获取指定从控内部单体当前电压值
**===========================================================================================*/
u16 GetBsuCellVolt(u8 clunum, u8 ecu, u8 cell);
/*=================================================================================================
** @Name      : u16 ChangSlaveVidToGroupVid(u8 slave, u8 cell)
** @Input     : slave:从控号[0,31] cell:从控内电池号[0,17]
** @Output    : 从控内电压序号对应组内电压序号[0,399],0xffff:错误序号
** @Function  : 将从控内电压序号转换成电池组内电压序号
** @The notes : 每个从控电压数目不一样的需要特殊处理
**===============================================================================================*/
u16 ChangSlaveVidToGroupVid(u8 slave, u8 cell);
/*=============================================================================================
** @Name      : void InputBsuCellVolt(u8 clunum, u8 ecu, u8 cell, u16 volt)
** @Input     : clunum:第几簇 ecu:从控节点号[0,31] cell:从控内电池号[0,15] volt:电压值 1mv
** @Output    : void
** @Function  : 输入接收到从控单体电压值
**===========================================================================================*/
void InputBsuCellVolt(u8 clunum, u8 ecu, u8 cell, u16 volt);
/*=============================================================================================
** @Name      : s16 GetBsuCellTemp(u8 ecu, u8 num)
** @Input     : ecu:从控节点号[0,31] num:从控内单体温度序号[0,3]
** @Output    : 从控单体当前温度值:1℃
** @Function  : 获取指定从控内部单体当前温度值
**===========================================================================================*/
s16 GetBsuCellTemp(u8 clunum, u8 ecu, u8 num);
/*=================================================================================================
** @Name      : u16 ChangSlaveTidToGroupTid(u8 slave, u8 cell)
** @Input     : slave:从控号[0,31] cell:从控内电池号[0,17]
** @Output    : 从控内温度序号对应组内温度序号[0,399],0xffff:错误序号
** @Function  : 将从控内温度序号转换成电池组内温度序号
** @The notes : 每个从控电压数目不一样的需要特殊处理
**===============================================================================================*/
u16 ChangSlaveTidToGroupTid(u8 slave, u8 cell);
/*=============================================================================================
** @Name      : void InputBsuCellTemp(u8 clunum, u8 ecu, u8 num, s16 temp)
** @Input     : ecu:从控节点号[0,31] num:从控内单体温度序号[0,3] temp:温度值(-40~126℃)
** @Output    : void
** @Function  : 输入接收到BSU单体温度值
** @The notes : 不包括均衡板温
**===========================================================================================*/
void InputBsuCellTemp(u8 clunum, u8 ecu, u8 num, s16 temp);

/*=============================================================================================
** @Name      : void InputSlaveChgBalanState(u16 cell, u16 state)
** @Input     : cell:组内起始电池号[0,415] state:连续16节电池充电均衡开启状态值[0关闭 1:开启(均充)]
** @Output    : void
** @Function  : 输入接收到从机充电均衡状态值(支持52串跨界拼接)
** @The notes : 按连续16Bit位表示连续16节电池顺序，采用掩码，避免跨从控覆盖
**===========================================================================================*/
void InputSlaveChgBalanState(u8 clunum, u16 cell, u16 state);

/* 获取从控回报的单体放电均衡状态及该从控最新回报是否仍在时效期内。 */
u8 GetGDhgBalanCellStateAPI(u8 clunum, u16 cell);
u8 GetGDhgBalanStateRcvAPI(u8 clunum, u8 slave);

/*=============================================================================================
** @Name      : void InputSlaveDhgBalanState(u8 cell, u8 state)
** @Input     : cell:组内电池号[0,399] state:连续16节电池放电均衡开启状态值[0关闭 1:开启(均放)]
** @Output    : void
** @Function  : 输入接收到从机放电均衡状态值
** @The notes : 按连续16Bit位表示连续16节电池顺序(非一个数组索引表示1个从控)
**===========================================================================================*/
void InputSlaveDhgBalanState(u8 clunum, u16 cell, u16 state);

/*=================================================================================================
** @Name      : u16 GetDevSampInfoAPI(u8 index)
** @Input     : index:数组下标
** @Output    : 板级采样信息
** @Function  : 获取板级采样信息
** @The notes : eDevSampInfo14
**===============================================================================================*/
u16 GetDevSampInfoAPI(u8 index);

/*=================================================================================================
** @Name      : u16 GetGProtectStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组保护状态[0,5]
** @Function  : 获取电池组当前保护状态
** @The notes : e_ProState
**===============================================================================================*/
u16 GetGProtectStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGWorkStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组工作状态[0,6]
** @Function  : 获取电池组当前工作状态
** @The notes : e_WorkState
**===============================================================================================*/
u16 GetGWorkStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGChgDhgStateAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充放电状态[0,2]
** @Function  : 获取电池组当前充放电状态
** @The notes : e_CurrState
**===============================================================================================*/
u16 GetGChgDhgStateAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGWorkModeAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组工作模式[0,5]
** @Function  : 获取电池组当前工作模式
** @The notes : e_WorkMode
**===============================================================================================*/
u16 GetGWorkModeAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGChgLimCurrAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 实时充电电流限值 0.1A
** @Function  : 获取电池组当前充电电流限值
** @The notes : 最大充电电流请求值
**===============================================================================================*/
u16 GetGChgLimCurrAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGDhgLimCurrAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 实时放电电流限值 0.1A
** @Function  : 获取电池组当前放电电流限值
** @The notes : 最大放电电流请求值
**===============================================================================================*/
u16 GetGDhgLimCurrAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGChgLimPowerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 充电功率限值 0.1kW
** @Function  : 返回充电功率限值
** @The notes : 最大充电功率请求值
**===========================================================================================*/
u16 GetGChgLimPowerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGDhgLimPowerAPI(void)
** @Input     : clunum:簇编号
** @Output    : 放电功率限值 0.1kW
** @Function  : 返回放电功率限值
** @The notes : 最大放电功率请求值
**===========================================================================================*/
u16 GetGDhgLimPowerAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGChgLimSumVAPI(void)
** @Input     : clunum:簇编号
** @Output    : 充电总压限值 0.1V
** @Function  : 返回充电总压限值
**===========================================================================================*/
u16 GetGChgLimSumVAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGDhgLimSumVAPI(void)
** @Input     : clunum:簇编号
** @Output    : 放电总压限值 0.1V
** @Function  : 返回放电总压限值
**===========================================================================================*/
u16 GetGDhgLimSumVAPI(u8 clunum);
/*=============================================================================================
** @Name      : u16 GetGAlarmFaultInfoAPI(u8 clunum, u8 index)
** @Input     : clunum,簇编号 index:告警信息索引号[0,2]
** @Output    : 电池组告警信息
** @Function  : 返回电池组告警信息
** @The notes : 16位告警码
**===========================================================================================*/
u16 GetGAlarmFaultInfoAPI(u8 clunum, u8 index);
/*=================================================================================================
** @Name      : u16 GetGSampSumVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组采样总电压 0.1V
** @Function  : 获取电池组采样总电压
** @The notes : 采样值
**===============================================================================================*/
u16 GetGSampSumVoltAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGSampOutCurrAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组采样输出电流 0.1A
** @Function  : 获取电池组采样输出电流
** @The notes : 负号为充电
**===============================================================================================*/
s16 GetGSampOutCurrAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGSampOutPowerAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组输出功率 0.1kW
** @Function  : 获取电池组输出功率值
** @The notes : 负号为充电
**===============================================================================================*/
s16 GetGSampOutPowerAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGSampPreVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组外端预充电压 0.1V
** @Function  : 获取电池组外端预充电压值
** @The notes : 采样值
**===============================================================================================*/
u16 GetGSampPreVoltAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGBattSocAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组SOC 0.1%
** @Function  : 获取电池组SOC值
**===============================================================================================*/
u16 GetGBattSocAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGBattSoeAPI(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : 电池组SOE 0.1%
** @Function  : 获取电池组SOE值
**===============================================================================================*/
u16 GetGBattSoeAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGBattSoeAPI(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : 电池组SOH 0.1%
** @Function  : 获取电池组SOH值
**===============================================================================================*/
u16 GetGBattSohAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGBattAllCapAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组总容量 0.1AH
** @Function  : 获取电池组总容量值
** @The notes : 实测值
**===============================================================================================*/
u16 GetGBattAllCapAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellSumVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组求和总电压 0.1V
** @Function  : 获取电池组求和总电压
** @The notes : 单体累加值
**===============================================================================================*/
u16 GetGCellSumVoltAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellAvgVoltAPI(void)
** @Input     : clunum:簇编号
** @Output    : 电池组单体平均电压 1mV
** @Function  : 获取电池组单体平均电压
** @The notes : 单体累加平均值
**===============================================================================================*/
u16 GetGCellAvgVoltAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGCellAvgTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体平均温度 1℃
** @Function  : 获取电池组单体平均温度
** @The notes : 单体累加平均值
**===============================================================================================*/
s16 GetGCellAvgTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGEnvirTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组环境温度 1℃
** @Function  : 获取电池组环境温度
** @The notes : 第1路外围温度
**===============================================================================================*/
s16 GetGEnvirTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellMaxVNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最大电压电池号[1,400]
** @Function  : 获取电池组单体最大电压电池号
**===============================================================================================*/
u16 GetGCellMaxVNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellMaxVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最大电压值 1mV
** @Function  : 获取电池组单体最大电压值
**===============================================================================================*/
u16 GetGCellMaxVoltAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellMinVNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最小电压电池号[1,400]
** @Function  : 获取电池组单体最小电压电池号
**===============================================================================================*/
u16 GetGCellMinVNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellMinVoltAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最小电压值 1mV
** @Function  : 获取电池组单体最小电压值
**===============================================================================================*/
u16 GetGCellMinVoltAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellMaxTNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最高温度序号[1,416]
** @Function  : 获取电池组单体最高温度序号
**===============================================================================================*/
u16 GetGCellMaxTNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGCellMaxTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最高温度值 0.1℃
** @Function  : 获取电池组单体最高温度值
**===============================================================================================*/
s16 GetGCellMaxTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellMinTNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体最低温度序号[1,400]
** @Function  : 获取电池组单体最低温度序号
**===============================================================================================*/
u16 GetGCellMinTNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGCellMinTempAPI(u8 clunum)
** @Input     : clunum：簇编号
** @Output    : 电池组单体最低温度值 1℃
** @Function  : 获取电池组单体最低温度值
**===============================================================================================*/
s16 GetGCellMinTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCanChgTimeAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组可充电时间 1min
** @Function  : 获取电池组可充电时间
**===============================================================================================*/
u16 GetGCanChgTimeAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCanDhgTimeAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组可放电时间 1min
** @Function  : 获取电池组可放电时间
**===============================================================================================*/
u16 GetGCanDhgTimeAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGChgCycleTimesAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组充放电循环次数 1次
** @Function  : 获取电池组充放电循环次数
**===============================================================================================*/
u16 GetGChgCycleTimesAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGMonWarNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当月告警数目 1个
** @Function  : 获取电池组当月告警数目
**===============================================================================================*/
u16 GetGMonWarNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGMonErrNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当月故障数目 1个
** @Function  : 获取电池组当月故障数目
**===============================================================================================*/
u16 GetGMonErrNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGHisWarNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计告警数目 1个
** @Function  : 获取电池组累计告警数目
**===============================================================================================*/
u16 GetGHisWarNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGHisErrNumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计故障数目 1个
** @Function  : 获取电池组累计故障数目
**===============================================================================================*/
u16 GetGHisErrNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u32 GetGHisErrSumNumAPI(void)
** @Input     : void
** @Output    : 电池组累计故障总数目 1个
** @Function  : 获取电池组累计故障总数目
** @The notes : 包括告警数目和故障数目
**===============================================================================================*/
u32 GetGHisErrSumNumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGRemainEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前剩余电能 0.1kWh
** @Function  : 获取电池组当前剩余电能
**===============================================================================================*/
u16 GetGRemainEnerAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGBattAllEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组电池总电能 0.1kWh
** @Function  : 获取电池组电池总电能
**===============================================================================================*/
u16 GetGBattAllEnerAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGNowChgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前单次充电量 1Ah
** @Function  : 获取电池组当前单次充电量
**===============================================================================================*/
u16 GetGNowChgCapAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGNowDhgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前单次放电量 1Ah
** @Function  : 获取电池组当前单次放电量
**===============================================================================================*/
u16 GetGNowDhgCapAPI(u8 clunum);
/*=================================================================================================
** @Name      : u32 GetGHisChgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计充电电量 1Ah
** @Function  : 获取电池组累计充电电量
**===============================================================================================*/
u32 GetGHisChgCapAPI(u8 clunum);
/*=================================================================================================
** @Name      : u32 GetGHisDhgCapAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计放电电量 1Ah
** @Function  : 获取电池组累计放电电量
**===============================================================================================*/
u32 GetGHisDhgCapAPI(u8 clunum);
/*=================================================================================================
** @Name      : u32 GetGHisChgEnerAPI(u8 clunum)
** @Input     : clunum:簇标号
** @Output    : 电池组累计充电电能 0.1kWh
** @Function  : 获取电池组累计充电电能
**===============================================================================================*/
u32 GetGHisChgEnerAPI(u8 clunum);
/*=================================================================================================
** @Name      : u32 GetGHisDhgEnerAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组累计放电电能 0.1kWh
** @Function  : 获取电池组累计放电电能
**===============================================================================================*/
u32 GetGHisDhgEnerAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGStatistNowDodAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组当前放电深度DOD 0.1%
** @Function  : 获取电池组当前放电深度DOD
**===============================================================================================*/
u16 GetGStatistNowDodAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGStatistMaxDodAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组最大放电深度DOD 0.1%
** @Function  : 获取电池组最大放电深度DOD
**===============================================================================================*/
u16 GetGStatistMaxDodAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetGCellVoltAPI(u8 clunum, u16 cell)
** @Input     : clunum:簇编号 cell:电池号[0,399]
** @Output    : 电池组单体电压值 1mV
** @Function  : 获取电池组单体电压值
**===============================================================================================*/
u16 GetGCellVoltAPI(u8 clunum, u16 cell);
/*=================================================================================================
** @Name      : s16 GetGCellTempAPI(u8 clunum, u16 cell)
** @Input     : clunum:簇编号 cell:电池号[0,399]
** @Output    : 电池组单体温度值 1℃
** @Function  : 获取电池组单体温度值
**===============================================================================================*/
s16 GetGCellTempAPI(u8 clunum, u16 cell);
/*=============================================================================================
** @Name      : void ClrPduInImeu3AllInfoAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 清除IMEU3信息
** @The notes : IMEU3信息接收中断时使用
**===========================================================================================*/
void ClrImeu3AllInfoAPI(void);
/*=============================================================================================
** @Name      : void ClrImiu4AllInfoAPI(u8 iu4num)
** @Input     : iu4num:iu4序号
** @Output    : void
** @Function  : 清除IMIU4信息
** @The notes : IMIU4信息接收中断时使用
**===========================================================================================*/
void ClrImiu4AllInfoAPI(u8 iu4num);
/*=================================================================================================
** @Name      : s16 GetGShuntSampTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组分流器温度采样值 0.1℃
** @Function  : 获取电池组分流器温度采样值
**===============================================================================================*/
s16 GetGShuntSampTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : s16 GetGDcPwrSampTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 低压电源板温度采样值 1℃
** @Function  : 获取低压电源板温度采样值
** @The notes : 2026-09-15接入DCV_TH(37)判定;温度通道待补,当前恒0℃
**===============================================================================================*/
s16 GetGDcPwrSampTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 GetImeu3BoardInfo1API(e_IMEU3BoardInfo1 index)
** @Input     : index:IMEU3采样信息1
** @Output    : IMEU3采样信息1
** @Function  : 获取IMEU3采样信息1信息值
**===============================================================================================*/
u8 GetImeu3BoardInfo1API(e_IMEU3BoardInfo1 index);
/*=================================================================================================
** @Name      : u16 GetImeu3BoardInfo2API(e_IMEU3BoardInfo2 index)
** @Input     : index:IMEU3采样信息2
** @Output    : IMEU3采样信息2
** @Function  : 获取IMEU3采样信息2信息值
**===============================================================================================*/
u16 GetImeu3BoardInfo2API(e_IMEU3BoardInfo2 index);
/*=================================================================================================
** @Name      : s8 GetGPerInsidMaxTempAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池柜内部最高环境温度值 1℃
** @Function  : 获取电池柜内部最高环境温度(IMEU3采集)
** @The notes : 多个温度传感器取最大值,for循环
**===============================================================================================*/
u8 GetGPerInsidMaxTempAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 GetGPerInsidMaxHumAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池柜内部最高环境湿度值 1℃
** @Function  : 获取电池柜内部最高环境湿度(IMEU3采集)
** @The notes : 多个湿度传感器取最大值,for循环
**===============================================================================================*/
u8 GetGPerInsidMaxHumAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Len index)
** @Input     : index:IMEU3告警信息
** @Output    : IMEU3告警信息
** @Function  : 获取IMEU3告警信息值
**===============================================================================================*/
u16 GetImeu3AlarmInfoAPI(e_IMEU3AlmInfo index);

/*=============================================================================================
** @Name      : u8 *gImeu3DevSerial / u16 *gImeu3SysStatus / u16 *gImeu3CfgPara
** @Function  : IMEU3单板序列号(u8) / 系统状态(u16) / 常规参数(u16)
**===========================================================================================*/
extern u8  *gImeu3DevSerial;
extern u16 *gImeu3SysStatus;
extern u16 *gImeu3CfgPara;
extern u16 *gImeu3DevAddr;                           /*mIndex=1*/
extern u8  *gImeu3CfgSet_76;                         /*A核配置缓存(ICC mIndex=76, u8×12)*/
extern u16 *gImeu3Info_77;                           /*采样+故障汇总(ICC mIndex=77, u16×53)*/
extern u8  *gImeu3CfgRO_75;                          /*回读参数缓存(ICC mIndex=75, u8×12)*/
extern u8   gEU3CfgReadbackFlag;                     /*下发后回读标志*/
extern s16 *gImiu4Info_67;                           /*1#IMIU4采样+告警汇总(ICC mIndex=67, s16×24)*/
extern s16 *gImiu4Info2_68;                          /*2#IMIU4采样+告警汇总(ICC mIndex=68, s16×24,2026-09-14接入)*/
/*=============================================================================================
** @Name      : gIccInsuQuery_85 / gIccCellResisReal / gIccCellResisImag / gBsuCellResisReal / gBsuCellResisImag
** @Function  : ICC85内阻查询缓冲(u16×8) / ICC28-31内阻实部缓冲(u32×4×211) / ICC32-35内阻虚部缓冲(u32×4×211) / BSU内阻实部/虚部原始缓冲(u32×N)
**===========================================================================================*/
extern u16 *gIccInsuQuery_85;                        /*ICC mIndex=85 内阻查询指令缓冲(u16×8)*/
extern u32 *gIccCellResisReal;                       /*ICC mIndex=28-31 内阻实部缓冲(u32×4×211)*/
extern u32 *gIccCellResisImag;                       /*ICC mIndex=32-35 内阻虚部缓冲(u32×4×211)*/
extern u32 *gBsuCellResisReal;                       /*BSU 内阻实部原始缓冲(u32×N)*/
extern u32 *gBsuCellResisImag;                       /*BSU 内阻虚部原始缓冲(u32×N)*/
extern u16  gIccInsuFreqIdx;                         /*ICC85下发的内阻检测频率索引(填ICC28-35子2)*/
/*=================================================================================================
** @Name      : s16 GetImiu4SampInfoAPI(u8 iu4num, e_IMIUSampInfo index)
** @Input     : iu4num:imiu4序号 index:IMIU4采样信息
** @Output    : IMIU4采样信息
** @Function  : 获取IMIU4采样信息值
**===============================================================================================*/
s16 GetImiu4SampInfoAPI(u8 iu4num, e_IMIUSampInfo index);
/*=================================================================================================
** @Name      : u16 GetImiu4AlmInfoAPI(u8 iu4num, e_IMIUAlmInfo index)
** @Input     : iu4num:imiu4序号 index:IMIU4采样信息
** @Output    : IMIU4采样信息
** @Function  : 获取IMIU4采样信息值
**===============================================================================================*/
u16 GetImiu4AlmInfoAPI(u8 iu4num, e_IMIUAlmInfo index);
/*=================================================================================================
** @Name      : u16 GetImiu4WorkModeAPI(u8 iu4num)
** @Input     : iu4num:imiu4序号
** @Output    : IMIU4系统工作模式
** @Function  : 获取IMIU4系统工作模式(ICC67/68 sub17转发用)
**===============================================================================================*/
u16 GetImiu4WorkModeAPI(u8 iu4num);
/*=============================================================================================
** @Name      : void InputBsuCellResis(u8 clunum, u8 ecu, u8 num, u16 resis)
** @Input     : clunum:第几簇 ecu:从控节点号[0,31] num:从控内电池号[0,15] resis:内阻值 1μΩ
** @Output    : void
** @Function  : 输入接收到从控单体内阻值
**===========================================================================================*/
void InputBsuCellResis(u8 clunum, u8 ecu, u8 num, u16 resis);
/*=============================================================================================
** @Name      : void InputBsuAfeState(u8 clunum, u8 ecu, u8 afeIndex, u8 afeState)
void ACDCSimChgDisCtrl(void);
** @Input     : clunum:第几簇 ecu:从控节点号[0,31] afeIndex:AFE序号[0,26] afeState:AFE状态值(4bit)
** @Output    : void
** @Function  : 输入接收到从控AFE状态值
** @The notes : 每个AFE IC占1个u8,状态值定义见e_AfeState
**===========================================================================================*/
void InputBsuAfeState(u8 clunum, u8 ecu, u8 afeIndex, u8 afeState);
extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );
/* PACK信息三维数组声明：[簇号][PACK号][数据索引] */
extern s16 (*gPackTemp_94)[MAX_BSU_NUM][GINDEX94_LEN];    /* 指向 94 号协议(温度)的数据池 */
extern u16 (*gPackStat_95)[MAX_BSU_NUM][GINDEX95_LEN];    /* 指向 95 号协议(状态)的数据池 */
extern u16 (*gGBsuGenPara_102)[SLAVE_MAX_NUM][GINDEX105_LEN];
/*调试结果索引枚举*/
typedef enum
{
    eDbgRes_DbgResult = 0,                                  /*调试结果:0=未执行 1=成功 2=非调试模式 3=参数错误 4=通信中断*/
    eDbgRes_RDataLW   = 1,                                  /*读数据低16位*/
    eDbgRes_RDataHW   = 2,                                  /*读数据高16位*/
    eDbgRes_Fin                                              /*结果数组长度*/
} e_DbgResultIdx;
#define GDBGRESULT_LEN (eDbgRes_Fin)

/*调试结果状态值枚举*/
typedef enum
{
    eDBGRES_UNEXE = 0,                                      /*未执行调试*/
    eDBGRES_SUCCE = 1,                                      /*调试成功*/
    eDBGRES_NOMOD = 2,                                      /*非调试模式*/
    eDBGRES_ERRPA = 3,                                      /*参数错误*/
    eDBGRES_CANOF = 4                                       /*通信中断*/
} e_DbgResultState;

/*调试结果全局数组 gGDebugResult[GDBGRESULT_LEN]*/
extern u16 gGDebugResult[];

#endif
