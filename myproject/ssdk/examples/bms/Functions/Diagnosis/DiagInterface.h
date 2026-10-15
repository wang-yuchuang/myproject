/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagInterface.h
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 电池故障诊断模块用户接口
** @Instructions : 
**===============================================================================================*/
#ifndef __DIAGINTERFACE_H_
#define __DIAGINTERFACE_H_
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "DiagUser.h"

/*故障诊断滤波状态*/
typedef enum
{
    eErr_A    = 0,                      /*正常状态*/
    eErr_B    = 1,                      /*异常状态*/
    eErr_AtoB = 2,                      /*内部使用*/
    eErr_BtoA = 3,                      /*内部使用*/
    eErr_State
}e_ErrState;

/*故障诊断滤波状态*/
typedef struct                          /*定义故障滤波状态结构*/
{
    u8 enable;                          /*使能*/
    u16 timer;                          /*滤波计数器*/
    e_ErrState nowState;                /*滤波瞬时实时状态*/
    e_ErrState filState;                /*滤波稳定状态*/
    u8 resTimes;                        /*已自恢复次数(10min内)*/
} __attribute__((aligned(4))) t_FilState;

/*故障诊断滤波参数*/
typedef struct                          /*定义故障滤波参数结构*/
{
    e_ErrSort errSort;                  /*故障类型[0:正常(不检测),1:仅提示类，2:充电限制类,3:放电限制类,4:充放电限制类,5:充电保护类,6:放电保护类,7:充放电保护类,8:系统故障类]*/
    u16 AtoBTime;                       /*A to B的时间*/
    u16 BtoATime;                       /*B to A的时间*/
    u8 maxTimes;                        /*允许自恢复最大次数(10min内)[0:不允许自恢复,255:无限次,1-254:允许自恢复次数]*/
}t_FilPara;

/*告警ID号*/
typedef enum
{
	/*以下为电池2级告警ID*/
    eERR_ID_CEL_VH2 = 0,                /*单体电压高2级(充电截止)*/
    eERR_ID_CEL_VL2 = 1,                /*单体电压低2级(放电截止)*/
    eERR_ID_SUM_VH2 = 2,                /*电池总电压过压2级(充电截止)*/
    eERR_ID_SUM_VL2 = 3,                /*电池总电压欠压2级(放电截止)*/
    eERR_ID_CHG_TH2 = 4,                /*充电温度高2级(充电截止)*/
    eERR_ID_CHG_TL2 = 5,                /*充电温度低2级(充电截止)*/
    eERR_ID_DHG_TH2 = 6,                /*放电温度高2级(放电截止)*/
    eERR_ID_DHG_TL2 = 7,                /*放电温度低2级(放电截止)*/
    eERR_ID_CHG_CO2 = 8,                /*充电过流2级(限制充电)*/
    eERR_ID_DHG_CO2 = 9,                /*放电过流2级(限制放电)*/
    eERR_ID_PER_TH2 = 10,               /*外设温度高2级(充放电限制)*/
    eERR_ID_LEAK2   = 11,               /*电池组绝缘2级(充放电限制)*/
    eERR_ID_CV_DIF2 = 12,               /*单体电压一致性下降2级(预警)*/
    eERR_ID_CT_DIF2 = 13,               /*单体温度温差过大2级(预警)[包括IMEU2的环境温度]*/
    eERR_ID_CAP_SH2 = 14,               /*充电容量SOC高2级(预警)*/
    eERR_ID_CAP_SL2 = 15,               /*剩余容量SOC低2级(预警)*/

    /*以下为电池1级告警ID*/
    eERR_ID_CEL_VH1 = 16,               /*单体电压高1级(充电保护)*/
    eERR_ID_CEL_VL1 = 17,               /*单体电压低1级(放电保护)*/
    eERR_ID_SUM_VH1 = 18,               /*电池总电压过压1级(充电保护)*/
    eERR_ID_SUM_VL1 = 19,               /*电池总电压欠压1级(放电保护)*/
    eERR_ID_CHG_TH1 = 20,               /*充电温度高1级(充电保护)*/
    eERR_ID_CHG_TL1 = 21,               /*充电温度低1级(充电保护)*/
    eERR_ID_DHG_TH1 = 22,               /*放电温度高1级(放电保护)*/
    eERR_ID_DHG_TL1 = 23,               /*放电温度低1级(放电保护)*/
    eERR_ID_CHG_CO1 = 24,               /*充电过流1级(充电延时保护)*/
    eERR_ID_DHG_CO1 = 25,               /*放电过流1级(放电延时保护)*/
    eERR_ID_PER_TH1 = 26,               /*外设温度过高1级(充放电保护)[包括IMEU2的环境温度]*/
    eERR_ID_LEAK1   = 27,               /*电池组绝缘1级(充放电保护)*/
    eERR_ID_CV_DIF1 = 28,               /*单体电压一致性下降1级(充放电限制)*/
    eERR_ID_CT_DIF1 = 29,               /*单体温度温差过大1级(充放电限制)*/
    eERR_ID_CAP_SH1 = 30,               /*充电容量SOC高1级(充电截止)*/
    eERR_ID_CAP_SL1 = 31,               /*剩余容量SOC低1级(放电截止)*/

    /*以下为提示类告警ID*/
    eERR_ID_SOC_EXP = 32,               /*SOC显示异常(需要修正)*/
    eERR_ID_CAP_EXP = 33,               /*容量检测异常(需要校准)*/
    eERR_ID_CHG_COT = 34,               /*充电限流响应超时(实测充电电流超过允许电流)*/
    eERR_ID_DHG_COT = 35,               /*放电限流响应超时(实测放电电流超过允许电流)*/
    eERR_ID_BAL_TH  = 36,               /*均衡板温过高：836无均衡板温采样，预留并固定为正常*/
    eERR_ID_DCV_TH  = 37,               /*低压电源板温高：温度输入待硬件确认，预留并固定为正常*/
    eERR_ID_SOS_EXP = 38,               /*SOS诊断异常告警(提示)*/
    eERR_ID_FAN_EXP = 39,               /*电池箱风扇异常告警(提示);历史遗留僵尸位,836风机回检已细分至FAN3_WAR(54),此位无setter,定性废弃*/
    eERR_ID_SHU_EXP = 40,               /*电池柜配电板外设(百叶风机空调)状态异常告警(提示);历史遗留僵尸位,语义已被THC_OFF(41)外设失联覆盖,定性废弃*/
    eERR_ID_THC_OFF = 41,               /*电池柜配电板外设(温湿度采集器/气体检测/空调)通信中断告警(提示)*/
    eERR_ID_SPD_EXP = 42,               /*电池柜配电板外设（防雷器）告警(充放电保护)*/
    eERR_ID_DUMP_EXP = 43,              /*电池柜配电板外设（倾倒）告警;836原理图查无物理硬件(倾倒开关查无),定性废弃*/
	eERR_ID_EU31_CON = 44,              /*1#IMEU3通信中断(Imeu3En bit0门控,2026-09-15接入)*/
	eERR_ID_EU32_CON = 45,              /*2#IMEU3通信中断(836单IMEU3(EU1),无2#硬件无解析设施,占位不接入)*/
	eERR_ID_LEAK_CON = 46,              /*漏电流传感器通信中断(836方案无漏电流传感器,R核未实现,保留枚举占位)*/
	/*47: 灯带通信中断 — 836硬件灯带=LU1(IMLU2)独立单元,A核控制灯带,R核无通路;如需R核上报须A核经ICC转发*/

	eERR_ID_AIR_CON  = 48,              /*空调通信中断：空调不在R核通信域，836预留并固定为正常*/
	eERR_ID_TH_CON   = 49,              /*温湿度传感器通信中断*/
	eERR_ID_DE_CON   = 50,              /*除湿机通信中断*/
	eERR_ID_COM_CON   = 51,             /*可燃气体传感器通信中断*/
	eERR_ID_FAN1_WAR = 52,              /*PDU扰流风机回检告警;836原理图查无物理硬件(PDU扰流风机查无),定性废弃*/
	eERR_ID_FAN2_WAR = 53,              /*PCS散热风扇回检告警;836原理图查无物理硬件(PCS风扇不外引回检),定性废弃*/
	eERR_ID_FAN3_WAR = 54,              /*排气扇回检告警;836=交流舱散热风机三台聚合回检(EU1 DI21,IMEU3 DI17_22 bit4),2026-09-15接入*/
	eERR_ID_EXP_EXP  = 55,              /*泄爆阀回检异常*/
	eERR_ID_LEAK_ERR = 56,              /*漏电流异常*/
	eERR_ID_FIRE_EWAR = 57,             /*消防一级告警(告警信息4 bit9,L1)*/
	eERR_ID_FIRE1_WAR = 58,             /*消防二级告警(告警信息4 bit10,L2)*/
	eERR_ID_FIRE3_WAR = 59,             /*消防三级告警(告警信息4 bit11,协议预留位,L3灭火级)*/
	eERR_ID_FIRE4_WAR = 60,             /*消防四级告警(告警信息4 bit12,协议预留位,L4下电级)*/
	eERR_ID_PDU_TH2 = 61,               /*PDU环境温度高2级告警(2026-09-14接入:NTC6交流柜PDU环境,51表℃+50解码,Alm2/3.b5故障屏蔽)*/
	eERR_ID_PDU_TH1 = 62,               /*PDU环境温度高1级告警(2026-09-14接入:一级越限级联二级,阈值PduEnvTH1/2)*/
	eERR_ID_CAB_TH2 = 63,               /*PDU线缆温度高2级告警(2026-09-14接入:NTC8交流功率线缆,Alm2/3.b7故障屏蔽)*/

	eERR_ID_CAB_TH1 = 64,               /*PDU线缆温度高1级告警(2026-09-14接入:一级越限级联二级,阈值PduCabTH1/2)*/
	eERR_ID_PCS_TH2 = 65,               /*PCS出风口温度高2级告警;836图纸EU1无出风口测温通道无数据链路,定性废弃(增探头后再启用)*/
	eERR_ID_PCS_TH1 = 66,               /*PCS出风口温度高1级告警;836图纸EU1无出风口测温通道无数据链路,定性废弃(增探头后再启用)*/
	eERR_ID_COMP_TH1 = 67,              /*电池仓温度1高温告警(2026-09-14接入:舱1最高温max(NTC1/2/3),探头故障剔除,阈值CompTH)*/
	eERR_ID_COMP_TH2 = 68,              /*电池仓温度2高温告警(2026-09-14接入:舱2最高温max(NTC4/5),探头故障剔除,阈值CompTH)*/
	eERR_ID_COMP_TH3 = 69,              /*电池仓温度3高温告警;836仅两电池舱,保留ID不置位*/
    eERR_ID_PACK1_WAT = 70,             /*1#PACK水浸告警*/
	eERR_ID_PACK1_MSD = 71,             /*1#PACKMSD维修开关异常*/
    eERR_ID_PACK1_TH1 = 72,             /*1#PACK正极高压连接器高温告警*/
	eERR_ID_PACK1_TH2 = 73,             /*1#PACK负极高压连接器高温告警*/
	eERR_ID_PACK1_AERS = 74,            /*1#PACK气溶胶浓度异常*/
	eERR_ID_PACK1_CO = 75,              /*1#PACKCO浓度异常（预留）*/
	eERR_ID_PACK1_H2 = 76,              /*1#PACKH2浓度异常（预留）*/
    eERR_ID_PACK2_WAT = 77,             /*2#PACK水浸告警*/
	eERR_ID_PACK2_MSD = 78,             /*2#PACKMSD维修开关异常*/
    eERR_ID_PACK2_TH1 = 79,             /*2#PACK正极高压连接器高温告警*/

	eERR_ID_PACK2_TH2 = 80,             /*2#PACK负极高压连接器高温告警*/
	eERR_ID_PACK2_AERS = 81,            /*2#PACK气溶胶浓度异常*/
	eERR_ID_PACK2_CO = 82,              /*2#PACKCO浓度异常（预留）*/
	eERR_ID_PACK2_H2 = 83,              /*2#PACKH2浓度异常（预留）*/
    eERR_ID_PACK3_WAT = 84,             /*3#PACK水浸告警*/
	eERR_ID_PACK3_MSD = 85,             /*3#PACKMSD维修开关异常*/
    eERR_ID_PACK3_TH1 = 86,             /*3#PACK正极高压连接器高温告警*/
	eERR_ID_PACK3_TH2 = 87,             /*3#PACK负极高压连接器高温告警*/
	eERR_ID_PACK3_AERS = 88,            /*3#PACK气溶胶浓度异常*/
	eERR_ID_PACK3_CO = 89,              /*3#PACKCO浓度异常（预留）*/
	eERR_ID_PACK3_H2 = 90,              /*3#PACKH2浓度异常（预留）*/
    eERR_ID_PACK4_WAT = 91,             /*4#PACK水浸告警*/
	eERR_ID_PACK4_MSD = 92,             /*4#PACKMSD维修开关异常*/
    eERR_ID_PACK4_TH1 = 93,             /*4#PACK正极高压连接器高温告警*/
	eERR_ID_PACK4_TH2 = 94,             /*4#PACK负极高压连接器高温告警*/
	eERR_ID_PACK4_AERS = 95,            /*4#PACK气溶胶浓度异常*/

	eERR_ID_PACK4_CO = 96,              /*4#PACKCO浓度异常（预留）*/
	eERR_ID_PACK4_H2 = 97,              /*4#PACKH2浓度异常（预留）*/
    eERR_ID_PACK5_WAT = 98,             /*5#PACK水浸告警*/
	eERR_ID_PACK5_MSD = 99,             /*5#PACKMSD维修开关异常*/
    eERR_ID_PACK5_TH1 = 100,            /*5#PACK正极高压连接器高温告警*/
	eERR_ID_PACK5_TH2 = 101,            /*5#PACK负极高压连接器高温告警*/
	eERR_ID_PACK5_AERS = 102,           /*5#PACK气溶胶浓度异常*/
	eERR_ID_PACK5_CO = 103,             /*5#PACKCO浓度异常（预留）*/
	eERR_ID_PACK5_H2 = 104,             /*5#PACKH2浓度异常（预留）*/
    eERR_ID_PACK6_WAT = 105,            /*6#PACK水浸告警*/
	eERR_ID_PACK6_MSD = 106,            /*6#PACKMSD维修开关异常*/
    eERR_ID_PACK6_TH1 = 107,            /*6#PACK正极高压连接器高温告警*/
	eERR_ID_PACK6_TH2 = 108,            /*6#PACK负极高压连接器高温告警*/
	eERR_ID_PACK6_AERS = 109,           /*6#PACK气溶胶浓度异常*/
	eERR_ID_PACK6_CO = 110,             /*6#PACKCO浓度异常（预留）*/
	eERR_ID_PACK6_H2 = 111,             /*6#PACKH2浓度异常（预留）*/


    eERR_ID_PACK7_WAT = 112,             /*7#PACK水浸告警*/
	eERR_ID_PACK7_MSD = 113,             /*7#PACKMSD维修开关异常*/
    eERR_ID_PACK7_TH1 = 114,             /*7#PACK正极高压连接器高温告警*/
	eERR_ID_PACK7_TH2 = 115,             /*7#PACK负极高压连接器高温告警*/
	eERR_ID_PACK7_AERS = 116,            /*7#PACK气溶胶浓度异常*/
	eERR_ID_PACK7_CO = 117,              /*7#PACKCO浓度异常（预留）*/
	eERR_ID_PACK7_H2 = 118,              /*7#PACKH2浓度异常（预留）*/
    eERR_ID_PACK8_WAT = 119,             /*8#PACK水浸告警*/
	eERR_ID_PACK8_MSD = 120,             /*8#PACKMSD维修开关异常*/
    eERR_ID_PACK8_TH1 = 121,             /*8#PACK正极高压连接器高温告警*/
	eERR_ID_PACK8_TH2 = 122,             /*8#PACK负极高压连接器高温告警*/
	eERR_ID_PACK8_AERS = 123,            /*8#PACK气溶胶浓度异常*/
	eERR_ID_PACK8_CO = 124,              /*8#PACKCO浓度异常（预留）*/
	eERR_ID_PACK8_H2 = 125,              /*8#PACKH2浓度异常（预留）*/
    eERR_ID_ACDC_CON = 126,              /*ACDC模块组通信中断*/
    eERR_ID_DCDC_CON = 127,              /*DCDC模块组通信中断*/

    eERR_ID_WARFIN  = 160,               /*最大160个告警*/



    /*故障类型开始*/
    /*以下为从控告警ID(46-57对应[0-3])*/
    eERR_ID_ERRSTART  = 160,             /*故障起始ID*/
    eERR_ID_BAL_TEXP  = 160,             /*均衡温度采样异常：836无均衡板温采样，预留并固定为正常*/
    eERR_ID_BAL_CEXP  = 161,             /*均衡控制异常：被动均衡命令与从控状态回报不一致*/
	eERR_ID_FLOOD_ERR = 162,             /*水浸告警*/
    eERR_ID_CGAS_EXP  = 163,             /*电池气体检测浓度异常*/

    /*以下为从控内部故障ID(68-71对应[4-7])*/
    eERR_ID_SAMP_EXP  = 164,             /*单体采样电路异常*/
    eERR_ID_SAMP_VOFF = 165,             /*单体电压采样断线*/
    eERR_ID_SAMP_TOFF = 166,             /*单体温度采样断线*/
    eERR_ID_CELL_MISS = 167,             /*单体信息缺失*/

    /*以下为主从通信故障ID(72-75对应[8-11])*/
    eERR_ID_INT_LOFF = 168,              /*内部主从级联异常(与下级从控连接)*/
    eERR_ID_INT_COFF = 169,              /*内部主从CAN断线故障(与下级从控通讯)*/
    eERR_ID_PER_COFF = 170,              /*1#IMIU4通信中断(协议表故障1 bit10;原注释:外设主从CAN断线-与内部绝缘检测仪通讯)*/
    eERR_ID_EXT_COFF = 171,              /*2#IMIU4通信中断(协议表故障1 bit11;原注释:外部通讯断线-与上级或充放电设备CAN/RS485通讯)*/

    /*以下为主控软件报警ID(76-79对应[12-15])*/
    eERR_ID_MECU_REP   = 172,            /*主控设备地址冲突;历史遗留僵尸位,新老平台双侧无setter无逻辑,定性废弃*/
    eERR_ID_ERR17_KEPP = 173,            /*主控主MCU异常(辅助MCU检测)*/
    eERR_ID_SW_EMERG   = 174,            /*电池组软件急停*/
    eERR_ID_SELF_PRO   = 175,            /*电池组自行保护*/

    /*以下为主控内部故障ID(80-95对应[16-31])*/
    eERR_ID_MRTC_EXP   = 176,            /*主控内部RTC异常(836无RTC芯片,SoC内部时钟,占位不接入)*/
    eERR_ID_MEEP_EXP   = 177,            /*主控外部EEPROM异常*/
    eERR_ID_MFLA_EXP   = 178,            /*主控外部Flash异常：非XIP形态接入上电及周期就绪检测，XIP形态固定正常*/
    eERR_ID_CSAMP_ERR  = 179,            /*电池组电流采样电路异常：IMIU4告警字及BMU本板采样异常*/
    eERR_ID_HSAMP_EXP  = 180,            /*直流高压采样异常：IMIU4告警信息1 bit4/bit5*/
    eERR_ID_LSAMP_EXP  = 181,            /*硬件板低压侧采样异常：BMU本板ADC采样链路*/
    eERR_ID_TSAMP_EXP  = 182,            /*外围温度采样异常：IMIU4 ShuntT有效范围（0.1℃，沿用100.37阈值）*/
    eERR_ID_LEAK_EXP   = 183,            /*绝缘采样异常*/

    eERR_ID_PK0RE_EXP   = 184,            /*PK1回检异常*/
    eERR_ID_PK1RE_EXP   = 185,            /*PK2回检异常*/
    eERR_ID_PK2RE_EXP   = 186,            /*PK3回检异常*/
    eERR_ID_PK3RE_EXP   = 187,            /*PK4回检异常*/
    eERR_ID_NK0RE_EXP   = 188,            /*NK1回检异常;历史遗留僵尸位,836无NK类继电器(禁充禁放/急停/放电干接点)独立回检DI,仅PK1-4接触器回检入BMU DI1-4,急停走ESD1→DI9→IO_EMERG(203),定性废弃*/
    eERR_ID_NK1RE_EXP   = 189,            /*NK2回检异常;历史遗留僵尸位(定性见188注),定性废弃*/
    eERR_ID_NK2RE_EXP   = 190,            /*NK3回检异常;历史遗留僵尸位(急停实际走ESD1→DI9→IO_EMERG 203,定性见188注),定性废弃*/
    eERR_ID_NK3RE_EXP   = 191,            /*NK4回检异常;历史遗留僵尸位(定性见188注),定性废弃*/

    //以下为主控外围故障ID(80-95对应[32-47])
    eERR_ID_K0MAI_EXP  = 192,            /*主正接触器状态异常(可能主接触器烧结或不能吸合);836回检走BMU本地DI1/DI3,2026-09-15接入*/
    eERR_ID_K1PRE_EXP  = 193,            /*预充接触器异常：836无预充回检链路，预留并固定为正常*/
    eERR_ID_K2NEG_EXP  = 194,            /*负极接触器异常(可能负极接触器烧结或不能吸合);836回检走BMU本地DI2/DI4,2026-09-15接入*/

	eERR_ID_FIRE_EXP   = 195,            /*消防系统告警;836=气溶胶状态(FFD1/FFD2常闭接点入EU1 DI5,IMEU3 DI1_16 bit4),2026-09-15接入*/
    eERR_ID_FUSE_OFF   = 196,            /*电池组熔断器熔断;836原理图查无物理硬件(熔断器无辅助触点),定性废弃*/
    eERR_ID_BATT_SMOG  = 197,            /*电池组烟雾报警*/

    eERR_ID_STARTUP_VL  = 198,           /*电池组启动预充未满：836无预充电压闭环，预留并固定为正常*/
    eERR_ID_STARTUP_TH  = 199,           /*电池组启动预充过温：836无预充电阻温度采样，预留并固定为正常*/
    eERR_ID_STARTUP_ERR = 200,           /*电池组启动失败(系统故障)*/
    eERR_ID_STOP_CH     = 201,           /*电池组停机带载超时*/
    eERR_ID_STOP_ERR    = 202,           /*电池组停机失败(系统故障)*/

    eERR_ID_IO_EMERG    = 203,           /*电池组按钮紧急停机*/
    eERR_ID_ISO_OFF     = 204,           /*断路器未合闸;836原理图查无物理硬件(隔离开关/微断无状态回检),定性废弃*/
    eERR_ID_DC_HV_DIFF  = 205,           /*电池组内外压差过大*/
    eERR_ID_SWHON_CH    = 206,           /*电池组带载闭合*/
    eERR_ID_SWHOFF_CH   = 207,           /*电池组带载断开*/

    eERR_ID_FINISH      = 288            /*最大160个故障诊断(故障和告警)*/
}e_ERR_ID;
/*=================================================================================================
** @Name      : void DiagErrFilterInit(void)
** @Input     : swhSort:接触器名称[0,11] state:控制接触器状态 0:断开 1:闭合
** @Output    : void
** @Function  : 故障诊断滤波初始化
** @The notes : 滤波处理数组清零
**===============================================================================================*/
extern void DiagErrFilterInit(void);
/*=================================================================================================
** @Name      : u8 DiagGetErrProMaskStateHOOK(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障ID
** @Output    : 故障是否屏蔽故障保护 0不屏蔽 1屏蔽
** @Function  : 获取对应ID故障是否屏蔽故障保护
** @The notes : 用户完善
**===============================================================================================*/
u8 DiagGetErrProMaskStateHOOK(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : u8 DiagCheckErrNeedDispHOOK(u8 clunum, e_ERR_ID id)
** @Input     : clunum簇编号 id:故障ID
** @Output    : 是否需要显示:TRUE:显示 FALSE:不显示
** @Function  : 判断对应故障告警是否向客户显示
** @The notes : 仅检测默认不显示的提示类告警
**===============================================================================================*/
u8 DiagCheckErrNeedDispHOOK(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : u8 DiagGetCellVTExpStateHOOKk(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 电池组单体采样异常状态: 0:正常 1:异常
** @Function  : 获取电池组单体采样异常状态
** @The notes : 从控电压采样失效/断线/CAN断线时从控采样异常
**===============================================================================================*/
u8 DiagGetCellVTExpStateHOOK(u8 clunum);
/*=================================================================================================
** @Name      : u8 DiagGetCellVoltExpStateHOOK(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体电压异常状态 0正常 1异常
** @Function  : 获取单体电压异常状态
** @The notes : 用户完善
**===============================================================================================*/
u8 DiagGetCellVoltExpStateHOOK(u8 clunum);
/*=================================================================================================
** @Name      : u8 DiagGetCellTempExpStateHOOK(u8 clunum)
** @Input     : clunum:簇 编号
** @Output    : 单体温度异常状态 0正常 1异常
** @Function  : 获取单体温度异常状态
** @The notes : 用户完善
**===============================================================================================*/
u8 DiagGetCellTempExpStateHOOK(u8 clunum);
/*=================================================================================================
** @Name      : u16 DiagGetCellVDiffSubVHOOK(u8 clunum, u16 value)
** @Input     : 转换前的单体电压压差过大压差
** @Output    : 转换后的单体电压压差过大压差 1mV
** @Function  : 获取单体电压压差过大压差值
** @The notes : 根据电压区间转换
**===============================================================================================*/
u16 DiagGetCellVDiffSubVHOOK(u8 clunum, u16 value);

#endif
