/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : PublicInfo.h
** @Founder      : ZSY342
** @Date         : 2019.10.30
** @Function     : 系统公共信息定义模块
** @Instructions : 
**===============================================================================================*/
#ifndef __PUBLICINFO_H_
#define __PUBLICINFO_H_

#include "BSPTypeDef.h"

/*系统时间定义*/
typedef struct
{
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u16 msecond;
}t_SysClock;

extern t_SysClock gSysClock;    /*系统时钟(北京时间), 定义于PublicInfo.c*/

/*故障/事件记录信息格式*/
typedef struct
{
    u8 type;                            /*事件类型*/
    u16 code;                           /*事件识别码(包括前缀码)*/
    u16 info1;                          /*事件附加信息1*/
    u16 info2;                          /*事件附加信息2*/
    u16 info3;                          /*事件附加信息3*/
    u16 info4;                          /*事件附加信息4*/
    u16 info5;                          /*事件附加信息5*/
}t_RcdCreateInfo;

/*故障诊断类型/级别*/
typedef enum
{
    eErr_Norm   = 0,                    /*正常(不检测)*/
    eErr_Warm   = 1,                    /*仅提示类*/
    eErr_ChgLim = 2,                    /*充电限制类(发生时限制充电,继续充电则请求停机[不断接触器])*/
    eErr_DhgLim = 3,                    /*放电限制类(发生时限制放电,继续放电则请求停机[不断接触器])*/
    eErr_SysLim = 4,                    /*充放电限制类(发生时限制充放电,继续充/放电则请求停机[不断接触器])*/
    eErr_ChgPro = 5,                    /*充电保护类(发生时限制充电,继续充电则请求停机和断接触器)*/
    eErr_DhgPro = 6,                    /*放电保护类(发生时限制放电,继续放电则请求停机和断接触器)*/
    eErr_SysPro = 7,                    /*充放电保护类(发生时限制充放电,请求停机和断接触器[需要断接触器的充电保护或放电保护需要设置为充放电保护故障类型]) */
    eErr_SysErr = 8,                    /*系统故障类(发生时限制充放电,系统状态故障,请求停机和断接触器)*/
}e_ErrSort;
/*=================================================================================================
** @Data type definition
**===============================================================================================*/
/*CAN通信校时信息索引定义*/
typedef enum
{
    eICCTim0_Y = 0,                     //校时信息-年
	eICCTim0_MD = 1,                    //校时信息-月日
	eICCTim0_HM = 2,                    //校时信息-时分
	eICCTim0_MS = 3                     //校时信息-毫秒
}e_ICCTimeInfo0;
/*系统保护状态值*/
typedef enum
{
    ePRO_WAIT   = 0,                    /*未就绪状态       [电池正常但直流回路未就绪]*/
    ePRO_READY  = 1,                    /*正常就绪状态    [电池正常且允许充电和放电]*/
    ePRO_FULL   = 2,                    /*充电截止状态    [电池充满仅允许放电]*/
    ePRO_EMPTY  = 3,                    /*放电截止状态    [电池放空仅允许充电]*/
    ePRO_SYSLIM = 4,                    /*充放电限制状态[直流回路正常但不允许充电或放电]*/
    ePRO_SYSPRO = 5,                    /*充放电保护状态[充放电回路断开故障保护]*/

    ePRO_STATE                          /*系统保护状态取值长度*/
}e_ProState;

/*系统工作状态值*/
typedef enum
{
    eWORK_INIT  = 0,                    /*开机初始化状态*/
    eWORK_IDLE  = 1,                    /*待命空闲状态*/
    eWORK_START = 2,                    /*正在启动状态*/
    eWORK_RUN   = 3,                    /*运行状态*/
    eWORK_STOP  = 4,                    /*正在停机状态*/
    eWORK_ERR   = 5,                    /*故障状态*/
    eWORK_OFF   = 6,                    /*关机下电状态*/

    eWORK_STATE                         /*系统工作状态*/
}e_WorkState;

/*系统充放电状态值*/
typedef enum
{
    eCURR_IDLE = 0,                     /*空闲状态*/
    eCURR_CHG  = 1,                     /*充电状态*/
    eCURR_DHG  = 2,                     /*放电状态*/

    eCURR_STATE                         /*系统充放电状态*/
}e_CurrState;

/*系统工作模式值*/
typedef enum
{
    eMODE_AUTO  = 0,                    /*自动模式*/
    eMODE_HAND  = 1,                    /*手动模式*/
    eMODE_EMERG = 2,                    /*紧急模式*/
    eMODE_DEBUG = 5,                    /*调试模式*/

    eMODE_STATE                         /*系统工作模式*/
}e_WorkMode;

/*系统控制命令值*/
typedef enum
{
    eCTRL_NORSTOP  = 0,                 /*正常停机命令*/
    eCTRL_PRECHG   = 1,                 /*启动预充命令*/
    eCTRL_NORSTART = 2,                 /*正常启动命令*/
    eCTRL_CHGSTART = 3,                 /*充电启动命令*/
    eCTRL_DHGSTART = 4,                 /*放电启动命令*/
    eCTRL_FORCSTOP = 5,                 /*强制停机命令*/
    eCTRL_EMERSTOP = 6,                 /*紧急停机命令*/
    eCTRL_POWEROFF = 7,                 /*下电关机命令*/
    eCTRL_POWERON  = 8,                 /*上电开机命令*/

    eCTRL_SYSCMD                        /*系统控制命令*/
}e_SysCtrlCmd;

/*系统清除命令值*/
typedef enum
{
    eCLR_NONE    = 0,                   /*无效命令*/
    eCLR_NOWERR  = 1,                   /*清除当前故障*/
    eCLR_ERRRCD  = 2,                   /*清除故障记录*/
    eCLR_ERRNUM  = 3,                   /*清除故障数目*/
    eCLR_RUNTIME = 4,                   /*清除运行计时*/
    eCLR_HISCAP  = 5,                   /*清除累计电量*/
    eCLR_DODRCD  = 6,                   /*清除DOD记录*/
    eCLR_EVENRCD = 7,                   /*清除事件记录*/
    eCLR_WAVERCD = 8,                   /*清除故障录波*/
    eCLR_HISDATA = 9,                   /*清除数据记录*/
    eCLR_FACTORY = 10,                  /*恢复出厂设置*/
    eCLR_FALCLIM = 11,                  /*复位充放电降功率限制*/
    eCLR_CDHGLIM = 12,                  /*复位充放电截止限制*/
    eCLR_SOSLOCK = 13,                  /*清除SOS锁定*/

    eCLR_SYSCLR                         /*系统清除命令*/
}e_SysClrCmd;

/*系统状态检测结果值*/
typedef enum
{
    eSYSRESULT_KEEP = 0,                /*系统检测等待*/
    eSYSRESULT_FIN  = 1,                /*系统检测正常*/
    eSYSRESULT_ERR  = 2,                /*系统检测故障*/
}e_SysStateResult;

/*系统控制命令端口值*/
typedef enum
{
    eCMDPORT_CAN0 = 0,                  /*CAN0口*/
	eCMDPORT_CAN1 = 1,                  /*CAN1口*/
	eCMDPORT_CAN2 = 2,                  /*CAN2口*/
	eCMDPORT_SCI0 = 5,                  /*RS485-0口*/
	eCMDPORT_SCI1 = 6,                  /*RS485-1口*/
	eCMDPORT_SCI2 = 7,                  /*RS485-2口*/
	eCMDPORT_BUTT = 10,                 /*按钮口*/
	eCMDPORT_FSTC = 11,                 /*快充口*/
	eCMDPORT_INTE = 12,                 /*内部*/
}e_SysCmdPortt;


#define BOARDX1_TYPE        (1)            /*底板类型 0：BMU02X1  1:BMU05X1*/    
/*=============================================================================================
** @Macro definition
**===========================================================================================*/
/*项目编号(识别电池PACK[电压采样解析/均衡/温感表]与默认参数)[常用配置:0/2/4(标准PACK16S-SPI通信0); 0/1/4(标准PACK16S-CAN通信1); 2/3/6(CATL-PACK14S-SPI通信0)]*/
#define PRJ_PACK_NTC_INFO	(0)				/*项目PACK温度NTC类型信息:0:标准PACK10K(项目:测试工装/标准PACK280AH) 1:亿纬PACK10K(项目:领亚亿纬PACK210AH) 2:CATL-PACK10K(项目:CATL-PACK240AH领亚储能/CATL液冷PACK270AH/CATL-PACK280AH新百城/星云PACK280AH康亿创) 3:CATL-PACK100K(项目:CATL-PACK240AH旧百城改造)*/
#define PRJ_PACK_CELL_INFO	(1)				/*项目PACK单体电压解析与均衡通道选择:0:18S[18+0S](项目:ADI-AFE菊花链测试工装) 1:16S[16+空2+0S](项目:领亚亿纬PACK210AH/BYD-AFE16S/CAN通信BSU) 2:16S[8+空1+8S](项目:ADI-AFE标准PACK280AH) 3:14S[7+空1+7S](项目:CATL-PACK240AH领亚储能/CATL-PACK240AH旧百城改造/CATL-PACK280AH新百城/星云PACK280AH康亿创) 4:52S[16+18+18S](项目:CATL液冷270AH)*/
#define PRJ_PARA_NUM_INFO	(4)				/*项目参数配置信息:0:测试工装-18S*1PACK,1:亿纬PACK210AH-16S*13PACK项目,2:CATL-PACK240AH-14S*15PACK项目 3:标准PACK280AH-14S*15PACK项目 4:标准PACK280AH-16S*15PACK项目 5:CATL液冷270AH-52S*4PACK项目 6:CATL-PACK280AH-14S*15PACK项目*/

#define PRJ_CELL_CAP        (314)           /*电芯容量，单位AH*/

#define BMU_DC_PWR_TYPE		(1)				/*BMU直流供电方式(0:自供电按钮 1:自供电手动开关  2:市电)*/

#define LIQUID_PACK_CELL_NUM  (48)           /*液冷PACK电芯数量*/
#define LIQUID_PACK_EQUAL_NUM (3)            /*一个液冷PACK等效于风冷系统PACK数量*/
#define BSU02U1_FLOOD_NUM     (1)            /*一个BSU02U1上的水浸传感器数量*/
#define BSU02U1_PT1000_NUM    (2)            /*一个BSU02U1上的PT1000数量*/
#define BSU02U1_MSD_NUM       (1)            /*一个BSU02U1上的MSD数量*/

#define BMU_BSU_COM_TYPE	(1)				/*BMU主从通信方式(0:菊花链 1:CAN)*/
#define BMU_BSU_CAN_TYPE	(0)				/*BMU主从CAN通信协议(0:自研从控 1:高特从控)*/

/*设备版本信息*/
#define BMU_DEV_ID			(19011)			/*BMU设备识别码(个位设备类型:0:从控采集板 1:从控通信板 2:主控 3:堆控 4:主控辅助芯片 5:高压模块 6:配电板)*/
#define BMU_HW_VER			(0x00a)			/*BMU硬件版本号[低8位有效]*/

#if(0 == BMU_BSU_COM_TYPE)                  /*使能自研从控BSU菊花链通信*/
#define BMU_SW_VERL			(0x031A)		/*BMU软件版本号低位(归档版本[0~99菊花链].变更号)*/
#elif((1 == BMU_BSU_COM_TYPE) && (0 == BMU_BSU_CAN_TYPE))/*使能自研从控BSU CAN通信*/
#define BMU_SW_VERL			(0x641F)		/*BMU软件版本号低位(归档版本[100~149自研CAN].变更号)*/
#else                                       /*使能高特BMM CAN通信*/
#define BMU_SW_VERL			(0x9615)		/*BMU软件版本号低位(归档版本[150~199高特CAN].变更号)*/
#endif

//#define BMU_SW_VERH		(0x102)			/*BMU软件版本号高位*/
#define BMU_SW_VERH			(((((u16)PILE_MANAG_EN * 100) + (BMU_DC_PWR_TYPE * 10) + 3) << 8) | ((((PRJ_PACK_NTC_INFO % 3) * 100) + ((PRJ_PACK_CELL_INFO % 5) * 10) + PRJ_PARA_NUM_INFO) & 0xff))/*BMU软件版本号高位(区别软件配置[并机/供电/外设.项目信息(NTC类型/PACK类型/项目配置)])*/

#define CELL_CHEM_TYPE		(2)				/*电芯类型(0:铅酸 1:三元  2:磷酸铁锂  3:钛酸锂)*/
#define MAX_BSU_NUM			(8)			    /*组最大从控数目[1,32]*/
#define MAX_BSU_CELL		(52)			/*从控最大单体数目[8,18]*/
#define MAX_CELL_NUM		(MAX_BSU_NUM * MAX_BSU_CELL)/*组最大单体数目[8,400]*/

#define BMU_BAS_DEVADDR		(50)			/*BMU起始设备地址[50,65]*/
#define SMU_BAS_DEVADDR		(70)			/*BPU起始设备地址[70,79]*/
#define PCS_BAS_DEVADDR		(80)			/*PCS起始设备地址[80,89]*/
#define IMEU_BAS_DEVADDR	(40)			/*主控内部IMEU地址[40,49]*/
#define IMLU_BAS_DEVADDR	(66)			/*主控内部IMLU地址[66,69]*/

#define BSU02_BAS_MAXADDR	(15)			/*主控内部BSU02最大地址[0,15]*/
#define CLUTER_MAX_NUM	    (2)			    /*单板最大管理簇数*/

#define IMEU3_PDUIN_DEVADDR	 (30)			/*PDU内部IMEU3地址[30,39]*/
#define IMEU3_PDUOUT_DEVADDR (31)			/*PDU外部IMEU3地址[30,39]*/

#define IMIU4_1_DEVADDR	     (20)			/*1#IMIU4通信地址*/
#define IMIU4_2_DEVADDR      (21)			/*2#IMIU4通信地址*/


#define LEAKAGE_BAS_DEVADDR	(1)				/*漏电流传感器通信地址*/

/*主控作主从并机功能*/
#define PILE_MANAG_EN		(0)				/*电池堆并机管理功能使能(0:禁用 1:使能)*/
#define PILE_M_MOV_EN		(0)				/*电池堆并机主机变换使能(0:只在1#电池组 1:选择开机最小组号电池组)*/
#define PILE_MOR_F_EN		(1)				/*电池堆最大功能使能(0:仅含并机功能[减小代码量] 1:含其他功能[较大代码量])*/
#define PILE_DIAG_F_EN		(1)				/*电池堆绝缘粘连检测管理功能使能(0:主控自行诊断 1:堆控协调控制)*/
#define PILE_MAX_G_NUM		(4)				/*电池堆并机最大电池组数*/
#define PILE_RCV_G_NUM		(PILE_MAX_G_NUM-1)/*电池堆并机接收信息电池组数*/

/*通信协议选择*/
#define	BMU_SELF_CAN_EN		(1)				/*BMU自身内部CAN协议使能(0:禁止 1:使能)*/
#define	BMU_CATL_CAN_EN		(0)				/*BMU兼容CATL电池CAN协议使能(0:禁止 1:使能)*/
#define	BMU_FROCK_CAN_EN	(1)				/*BMU测试工装CAN协议使能(0:禁止 1:使能)*/

#define	BMU_CATL_CAN_PORT	(0)				/*BMU兼容CATL电池CAN协议通信CAN口*/

/*项目参数定义*/
#define PRJ_PAR_DEV_ID		(0x0103)		/*设备识别码(H:RTU编号[0,255]  L:设备类型[H：[0,255] L：0EMS,1PCS,2BSMU,3:BMU)])*/
#define PRJ_PAR_PRD_DAT		(22010)			/*设备出厂日期(年月-2000+流水号,例如19090,最后一位为批次流水号)*/
#define PRJ_PAR_INS_SIT		(0x0000)		/*项目编码   H:安装地点 L:安装地编号*/
#define PRJ_PAR_SYS_ID		(0x0101)		/*系统识别码 H:机组号(0非机组)  L:电池组号(0非电池组)*/
#define PRJ_PAR_SYS_TYP		(0x0032)		/*系统定位码 H:系统类型(0二级系统 1扩展三级系统 2独立三级系统) L:系统所在设备号*/
#define PRJ_PAR_SYS_NUM		(0x0101)		/*系统信息  H(系统总数) L:电池组数*/

/*电池极限值定义*/
#define BATT_EXEREM_MAXT     80             /*电芯最高极限温度(1℃)*/
#define BATT_EXEREM_MINT     -30            /*电芯最低极限温度(1℃)*/
#define BATT_EXEREM_MAXC     4000           /*电芯最大极限电流(0.1A)*/

/*默认值定义*/
#if(3 == CELL_CHEM_TYPE)                    /*钛酸锂电芯默认值*/
#define DEFAULT_STANDCV     2100            /*电芯标称电压(1mV)*/
#define DEFAULT_LO_MAXV     2500            /*电芯电压断线高端异常电压(1mV)*/
#define DEFAULT_LO_MINV     1800            /*电芯电压断线低端异常电压(1mV)*/
#define DEFAULT_EX_MAXV     2300            /*电芯最高极限电压(1mV)*/
#define DEFAULT_EX_MINV     1900            /*电芯最低极限电压(1mV)*/

#elif(2 == CELL_CHEM_TYPE)                  /*磷酸铁锂电芯默认值*/
#define DEFAULT_STANDCV     3300            /*电芯标称电压(1mV)*/
#define DEFAULT_LO_MAXV     4500            /*电芯电压断线高端异常电压(1mV)*/
#define DEFAULT_LO_MINV     1300            /*电芯电压断线低端异常电压(1mV)*/
#define DEFAULT_EX_MAXV     3850            /*电芯最高极限电压(1mV)*/
#define DEFAULT_EX_MINV     2000            /*电芯最低极限电压(1mV)*/

#elif(1 == CELL_CHEM_TYPE)                  /*三元电芯默认值*/
#define DEFAULT_STANDCV     3700            /*电芯标称电压(1mV)*/
#define DEFAULT_LO_MAXV     4500            /*电芯电压断线高端异常电压(1mV)*/
#define DEFAULT_LO_MINV     2500            /*电芯电压断线低端异常电压(1mV)*/
#define DEFAULT_EX_MAXV     4250            /*电芯最高极限电压(1mV)*/
#define DEFAULT_EX_MINV     2900            /*电芯最低极限电压(1mV)*/

#else                                       /*铅酸电芯*/
#define DEFAULT_STANDCV     3000            /*电芯标称电压(1mV)*/
#define DEFAULT_LO_MAXV     5000            /*电芯电压断线高端异常电压(1mV)*/
#define DEFAULT_LO_MINV     1000            /*电芯电压断线低端异常电压(1mV)*/
#define DEFAULT_EX_MAXV     2900            /*电芯最高极限电压(1mV)*/
#define DEFAULT_EX_MINV     1100            /*电芯最低极限电压(1mV)*/

#endif

/*主索引长度定义*/
#define SINDEX40_LEN		(15)			/*项目信息数组长度*/

/*=================================================================================================
** @Name      : void SysPublicInfoInit(void)
** @Input     : void
** @Output    : void
** @Function  : 系统系统通用信息初始化
** @The notes :
**===============================================================================================*/
void SysPublicInfoInit(void);
/*=================================================================================================
** @Name      : void SysClockDisplayAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 更新系统时钟信息到显示区
** @The notes : 查询系统时间时调用
**===============================================================================================*/
void SysClockDisplayAPI(void);
/*=================================================================================================
** @Name      : void SysClockRecordTask(void)
** @Input     : void
** @Output    : void
** @Function  : 系统时钟时间记录任务(包括记录运行时间)
** @The notes : 2s周期调用
**===============================================================================================*/
void SysClockRecordTask(void);
/*=================================================================================================
** @Name      : void ClrSysSecondTimeAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 清除电池组系统运行计时
** @The notes :
**===============================================================================================*/
void ClrSysSecondTimeAPI(void);
/*=================================================================================================
** @Name      : u32 GetSysSecondTimeAPI(void)
** @Input     : void
** @Output    : 系统计时 1s
** @Function  : 获取电池组系统运行计时
** @The notes : 上电开始计时,考虑可能会外部清零复位
**===============================================================================================*/
u32 GetSysSecondTimeAPI(void);
/*=================================================================================================
** @Name      : void SysClockCountTask(void)
** @Input     : void
** @Output    : void
** @Function  : 系统时钟(北京时间)计数任务,ms计时
** @The notes : 1ms定时中断调用
**===============================================================================================*/
void SysClockCountTask(void);
/*=================================================================================================
** @Name      : void SysClockUpdateAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 将显示区校时信息更新到系统时钟信息
** @The notes : 校时信息变化时调用
**===============================================================================================*/
void SysClockUpdateAPI(void);
t_SysClock GetSysDateClockAPI(void);
/*=================================================================================================
** @Name      : void SysClockExtVerifyAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 系统时钟外部校时处理
** @The notes : 接收校时报文时调用
**===============================================================================================*/
void SysClockExtVerifyAPI(void);

#endif