/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagUser.c
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 电池故障诊断模块用户接口
** @Instructions : 
**===============================================================================================*/
#include <cmsis_os2.h>
#include "DiagUser.h"
#include "DiagInterface.h"
#include "BSPTypeDef.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "GroupPara.h"
#include "GroupTactic.h"
#include "GroupProtect.h"
#include "DiagCheck.h"
#include "InsuCheck.h"
#include "TacUser.h"
#include "CapInfoCalc.h"
#include "CurrSample.h"
#include "EnerInfoCalc.h"
#include "DiagFilter.h"
#include "TimesStatis.h"
#include "DiagSos.h"
#include <string.h>
#include <stdio.h>
#include <debug.h>
#include "IccMsgInterface.h"
#include <CLI.h>
extern u16 *gAlmFault1_57;        /*组1告警故障信息*/
extern u16 *gAlmFault2_58;        /*组2告警故障信息*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u16 *gGHisErrInfo1_10;     /*组1历史故障信息*/
extern u16 *gGHisErrInfo2_11;     /*组2历史故障信息*/
/*=================================================================================================
** @Global variable definition
**===============================================================================================*/
const t_FilPara gErrFilPara[eERR_ID_FINISH] =               /*故障诊断滤波参数(故障滤波时间和自恢复次数)*/
{
	/*故障类型/检测滤波时间(ms)/恢复滤波时间(ms)/允许自恢复次数*/
	/*故障类型:[0:正常(不检测),1:仅提示类，2:充电限制类,3:放电限制类,4:充放电限制类,5:充电保护类,6:放电保护类,7:充放电保护类,8:系统故障类]*/
	/*以下为2级一般告警*/
	{eErr_ChgLim,  1000, 10000, 255},                       /*[0]单体过压2级充电限制*/
	{eErr_DhgLim,  1000, 10000, 255},                       /*[1]单体欠压2级放电限制*/
	{eErr_ChgLim,  1000, 10000, 255},                       /*[2]电池总电压过压2级充电限制*/
	{eErr_DhgLim,  1000, 10000, 255},                       /*[3]电池总电压欠压2级放电限制*/
	{eErr_ChgLim,  3000, 10000, 255},                       /*[4]充电高温2级充电限制*/
	{eErr_ChgLim,  3000, 10000, 255},                       /*[5]充电低温2级充电限制*/
	{eErr_DhgLim,  3000, 10000, 255},                       /*[6]放电高温2级放电限制*/
	{eErr_DhgLim,  3000, 10000, 255},                       /*[7]放电低温2级放电限制*/
	{eErr_ChgLim,  3000, 10000, 255},                       /*[8]充电过流2级限制充电*/
	{eErr_DhgLim,  3000, 10000, 255},                       /*[9]放电过流2级限制放电*/
	{eErr_SysLim,  5000, 10000, 255},                       /*[10]外设高温2级充放电限制*/
	{eErr_SysLim,  1000, 10000, 255},                       /*[11]绝缘2级充放电限制*/
	{eErr_Warm,    5000, 10000, 255},                       /*[12]单体电压压差过大2级预警*/
	{eErr_Warm,    5000, 10000, 255},                       /*[13]单体温度温差过大2级预警*/
	{eErr_Warm,    5000, 10000, 255},                       /*[14]剩余容量SOC高2级预警*/
	{eErr_Warm,    5000, 10000, 255},                       /*[15]剩余容量SOC低2级预警*/

	/*以下为1级严重告警*/
	{eErr_SysPro,  1000, 10000,   6},                       /*[16]单体过压1级充电保护(需要断接触器)*/
	{eErr_DhgPro,  1000, 10000,   6},                       /*[17]单体欠压1级放电保护(放电断接触器)*/
	{eErr_SysPro,  1000, 10000,   6},                       /*[18]电池总电压过压1级充电保护(需要断接触器)*/
	{eErr_DhgPro,  1000, 10000,   6},                       /*[19]电池总电压欠压1级放电保护(放电断接触器)*/
	{eErr_ChgPro,  3000, 10000,  51},                       /*[20]充电高温1级充电保护(充电断接触器)*/
	{eErr_ChgPro,  3000, 10000,  51},                       /*[21]充电低温1级充电保护(充电断接触器)*/
	{eErr_SysPro,  3000, 30000,  51},                       /*[22]放电高温1级停机保护*(需要断接触器)*/
	{eErr_SysPro,  3000, 30000,  51},                       /*[23]放电低温1级停机保护(需要断接触器)*/
	{eErr_SysPro,  3000, 30000,   1},                       /*[24]充电过流1级充电保护(充电断接触器)*/
	{eErr_SysPro,  3000, 30000,   1},                       /*[25]放电过流1级放电保护(放电断接触器)*/
	{eErr_SysPro,  5000, 10000, 255},                       /*[26]外设高温1级停机保护(需要断接触器)*/
	{eErr_SysErr,  1000, 10000,   6},                       /*[27]绝缘1级故障保护*/
	{eErr_SysLim,  5000, 10000, 255},                       /*[28]单体电压压差过大1级充放电限制*/
	{eErr_SysLim,  5000, 10000, 255},                       /*[29]单体温度温差过大1级充放电限制*/
	{eErr_ChgLim,  5000, 10000, 255},                       /*[30]剩余容量SOC高1级充电限制*/
	{eErr_DhgLim,  5000, 10000, 255},                       /*[31]剩余容量SOC低1级放电限制*/

	/*以下为提示类告警*/
	{eErr_Warm,    1000,  1000, 255},                       /*[32]SOC显示异常(需要修正)*/
	{eErr_Warm,    1000,  1000, 255},                       /*[33]容量检测异常(需要校准)*/
	{eErr_Warm,   10000,  5000, 255},                       /*[34]充电限流响应超时(实测电流超过允许值)*/
	{eErr_Warm,   10000,  5000, 255},                       /*[35]放电限流响应超时(实测电流超过允许值)*/
	{eErr_Warm,    3000,  3000, 255},                       /*[36]均衡板温过高(关闭均衡)*/
	{eErr_Warm,    3000,  3000, 255},                       /*[37]低压电源板温异常(提示)*/
	{eErr_Warm,    1000,  1000, 255},                       /*[38]SOS诊断告警*/
	{eErr_Warm,   10000, 10000, 255},                       /*[39]电池箱风扇告警;历史遗留僵尸位,已由FAN3_WAR(54)细分替代,定性废弃*/
	{eErr_Warm,   20000, 10000, 255},                       /*[40]电池柜百叶风机空调异常告警(提示);历史遗留僵尸位,语义已被THC_OFF(41)覆盖,定性废弃*/
	{eErr_Warm,    5000, 10000, 255},                       /*[41]电池柜辅助设备通信中断告警(提示)*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[42]电池柜SPD异常告警（需要断接触器）*/
	{eErr_SysErr,  3000, 5000,    6},                       /*[43]电池柜倾倒告警(提示;836原理图查无物理硬件,定性废弃)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[44]1#IMEU3通信中断*/
	{eErr_Warm,    3000, 5000,  255},                       /*[45]2#IMEU3通信中断(836单IMEU3(EU1),无2#硬件,占位不接入)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[46]漏电流传感器通信中断(836无此传感器,R核未实现,枚举保留)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[47]灯带通信中断(836灯带=LU1(IMLU2)独立单元,A核控制灯带,R核无通路,枚举占位)*/

	{eErr_Warm,    3000, 5000,  255},                       /*[48]空调通信中断*/
	{eErr_Warm,    3000, 5000,  255},                       /*[49]温湿度传感器通信中断*/
	{eErr_Warm,    3000, 5000,  255},                       /*[50]除湿机通信中断*/
	{eErr_Warm,    3000, 5000,  255},                       /*[51]可燃气体传感器通信中断*/
	{eErr_Warm,    3000, 5000,  255},                       /*[52]PDU扰流风机回检告警(836原理图查无物理硬件,定性废弃)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[53]PCS散热风扇回检告警(836原理图查无物理硬件,定性废弃)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[54]排气扇回检告警(836=交流舱风机三台聚合,EU1 DI21接入)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[55]泄爆阀回检异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[56]漏电流异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[57]消防一级告警(告警信息4 bit9,L1禁充禁放,方案V1.2一级电流置0;源头一次恢复机会,再触发锁存)*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[58]消防二级告警(告警信息4 bit10,L2禁充禁放)*/
	{eErr_SysLim,  3000, 30000, 255},                       /*[59]消防三级告警(告警信息4 bit11,L3灭火级,承接原[163]滤波)*/
	{eErr_SysLim,  7000, 10000,    1},                       /*[60]消防四级告警(告警信息4 bit12,L4下电级,承接原[195]滤波)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[61]PDU环境温度高2级告警(2026-09-14接入:NTC6,Alm2/3.b5故障屏蔽)*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[62]PDU环境温度高1级告警(2026-09-14接入:一级级联二级,限功率)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[63]PDU线缆温度高2级告警(2026-09-14接入:NTC8,Alm2/3.b7故障屏蔽)*/

	{eErr_SysLim,  3000, 5000,  255},                       /*[64]PDU线缆温度高1级告警(2026-09-14接入:一级级联二级)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[65]PCS出风口温度高2级告警;836无出风口通道,定性废弃*/
	{eErr_Warm,    3000, 5000,  255},                       /*[66]PCS出风口温度高1级告警;836无出风口通道,定性废弃*/
	{eErr_Warm,    3000, 5000,  255},                       /*[67]电池仓温度1高温告警(2026-09-14接入:舱1max(NTC1/2/3),阈值CompTH)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[68]电池仓温度2高温告警(2026-09-14接入:舱2max(NTC4/5),阈值CompTH)*/
	{eErr_Warm,    3000, 5000,  255},                       /*[69]电池仓温度3高温告警;836仅两舱,保留不置位*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[70]1#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[71]1#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[72]1#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[73]1#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[74]1#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[75]1#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[76]1#PACKH2浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[77]2#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[78]2#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[79]2#PACK正极高压连接器高温告警*/

	{eErr_SysLim,  3000, 5000,  255},                       /*[80]2#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[81]2#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[82]2#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[83]2#PACKH2浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[84]3#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[85]3#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[86]3#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[87]3#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[88]3#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[89]3#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[90]3#PACKH2浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[91]4#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[92]4#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[93]4#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[94]4#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[95]4#PACK气溶胶浓度异常*/

	{eErr_SysLim,  3000, 5000,  255},                       /*[96]4#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[97]4#PACKH2浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[98]5#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[99]5#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[100]5#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[101]5#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[102]5#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[103]5#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[104]5#PACKH2浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[105]6#PACK水浸告警*/
	{eErr_Warm,    3000, 5000,  255},                       /*[106]6#PACKMSD维修开关异常*/
	{eErr_Warm,    3000, 5000,  255},                       /*[107]6#PACK正极高压连接器高温告警*/
	{eErr_Warm,    3000, 5000,  255},                       /*[108]6#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[109]6#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[110]6#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[111]6#PACKH2浓度异常（预留）*/

    {eErr_SysLim,  3000, 5000,  255},                       /*[112]7#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[113]7#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[114]7#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[115]7#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[116]7#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[117]7#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[118]7#PACKH2浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[119]8#PACK水浸告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[120]8#PACKMSD维修开关异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[121]8#PACK正极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[122]8#PACK负极高压连接器高温告警*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[123]8#PACK气溶胶浓度异常*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[124]8#PACKCO浓度异常（预留）*/
	{eErr_SysLim,  3000, 5000,  255},                       /*[125]8#PACKH2浓度异常（预留）*/
    {eErr_Warm,   10000, 5000,  255},     				    /*[126]预留*/
    {eErr_Warm,   10000, 5000,  255},                       /*[127]预留*/

    {eErr_Norm,       0,    0,  255},                       /*[128]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[129]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[130]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[131]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[132]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[133]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[134]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[135]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[136]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[137]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[138]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[139]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[140]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[141]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[142]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[143]预留*/

    {eErr_Norm,       0,    0,  255},                       /*[144]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[145]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[146]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[147]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[148]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[149]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[150]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[151]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[152]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[153]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[154]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[155]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[156]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[157]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[158]预留*/
    {eErr_Norm,       0,    0,  255},                       /*[159]预留*/



	/*以下为故障类故障*/
	{eErr_SysLim,  5000, 10000, 255},                       /*[160]均衡温度采样异常*/
	{eErr_SysLim,  5000, 10000, 255},                       /*[161]均衡控制电路异常*/
	{eErr_SysLim,  3000, 10000, 255},                       /*[162]水浸告警*/
	{eErr_SysLim,  3000, 30000, 255},                       /*[163]电池气体检测浓度异常(消防L3曾占用,已迁[59],现空闲)*/
	{eErr_SysLim,  3000, 10000, 255},                       /*[164]从控采样电路异常*/
	{eErr_SysLim,  3000, 10000, 255},                       /*[165]从控电压采样断线*/
	{eErr_SysLim,  3000, 10000, 255},                       /*[166]从控温度采样断线*/
	{eErr_SysLim, 30000,  3000, 255},                       /*[167]从控单体信息缺失*/

	//菊花链通信才有级联线
	#if(0 == BMU_BSU_COM_TYPE)
	{eErr_Warm,    8000,  3000, 255},                       /*[168]从控级联线束异常*/
	#else
	{eErr_Norm,       0,     0, 255},                       /*[168]从控级联线束异常*/
	#endif

    #if(0 == BMU_BSU_COM_TYPE)                              /*菊花链单体采样方式*/
	{eErr_Norm,    8000,  3000, 255},                       /*[169]内部主从通信中断*/
    #else                                                   /*CAN通信单体采样方式*/
	{eErr_SysErr, 10000,  3000, 255},                       /*[169]内部主从通信中断*/
    #endif
	{eErr_Warm,    5000,  3000, 255},                       /*[170]1#IMIU4通信中断(5s滤波,2026-09-12由15s缩短)*/

	/*主控外部与BSMU通讯*/
	#if(2 == BMU_EXT_COM_DEV)
	{eErr_SysErr, 15000,  2000, 255},                       /*[171]外部BSMU通讯断线*/

	/*主控外部与PCS通讯*/
	#elif(1 == BMU_EXT_COM_DEV)
	{eErr_SysLim, 30000,  1000, 255},                       /*[171]外部PCS通讯断线*/

	/*不监测外部通讯*/
	#else
	{eErr_Warm,    5000,  3000, 255},                       /*[171]2#IMIU4通信中断(5s滤波,2026-09-12由15s缩短)*/
	#endif

	{eErr_SysErr,  5000, 10000, 255},                       /*[172]主控设备地址冲突;历史遗留僵尸位,双侧无逻辑,定性废弃*/
	{eErr_Norm,       0,     0, 255},                       /*[173]主控主MCU软件异常(辅助MCU检测)*/
	{eErr_SysErr,     0,  5000, 255},                       /*[174]电池组软件急停控制*/
	{eErr_Warm,    3000,  3000, 255},                       /*[175]电池组自行保护*/

	{eErr_Warm,    5000, 10000, 255},                       /*[176]主控内部RTC异常(836无RTC芯片,占位不接入)*/
	{eErr_Warm,    5000, 10000, 255},                       /*[177]主控外部EEPROM存储异常*/
	{eErr_Warm,    5000, 10000, 255},                       /*[178]主控外部Flash存储异常*/
	{eErr_SysLim,  3000,  5000,   5},                       /*[179]电池组电流采样电路异常*/
	{eErr_SysErr,  3000, 10000,   5},                       /*[180]输出端高压采样异常*/
	{eErr_Warm,    3000, 10000, 255},                       /*[181]硬件板低压采样异常*/
	{eErr_Warm,    3000, 10000, 255},                       /*[182]外设温度采样异常*/
	{eErr_SysErr,  3000, 10000,   5},                       /*[183]绝缘采样异常*/

	{eErr_SysErr,    3000,  5000, 255},                       /*[184]PK1回检异常(簇1主正接触器)*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[185]PK2回检异常(簇1负极接触器)*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[186]PK3回检异常(簇2主正接触器)*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[187]PK4回检异常(簇2负极接触器)*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[188]NK1回检异常(簇1禁充禁放);僵尸位,836无NK独立回检DI(见枚举188注),定性废弃*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[189]NK2回检异常(簇2禁充禁放);僵尸位定性废弃(见枚举188注)*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[190]NK3回检异常(急停);僵尸位,急停实际走ESD1→DI9→IO_EMERG(203),定性废弃*/
	{eErr_SysErr,    3000,  5000, 255},                       /*[191]NK4回检异常(放电干接点);僵尸位定性废弃(见枚举188注)*/

	{eErr_SysErr,  3000, 20000,  255},                       /*[192]主正接触器异常(可能主接触器烧结或不能吸合;836回检走BMU本地DI1/DI3,2026-09-15接入)*/
	{eErr_SysErr,  3000, 10000,  255},                       /*[193]预充电路异常(可能预充电阻断开或预充接触器不能吸合)*/
	{eErr_SysErr,  3000, 20000,  255},                       /*[194]负极接触器异常(可能负极接触器烧结或不能吸合;836回检走BMU本地DI2/DI4,2026-09-15接入)*/

	/*气溶胶(化学反应生热产生告警信号仅持续30s)*/
	{eErr_SysLim,  7000, 10000,   1},                       /*[195]消防系统告警(气溶胶EU1 DI5接入2026-09-15;消防L4曾占用,已迁[60])*/
	/*七氟丙烷(火探管压力产生告警信号一直持续)*/
	//{eErr_SysLim,  3000, 10000,   1},                     /*[195]消防系统告警*/

	{eErr_SysLim,  2000,  5000, 255},                       /*[196]电池组熔断器熔断(836原理图查无物理硬件,定性废弃)*/
	{eErr_SysLim,  1000,  5000, 255},                       /*[197]电池组烟雾报警*/
	{eErr_SysErr,  1000, 30000,   3},                       /*[198]电池组启动预充未满(可能带载启动)*/
	{eErr_SysErr,  1000, 30000,   3},                       /*[199]电池组启动预充过温(预充电阻过温)*/
	{eErr_SysErr,  1000, 20000,   5},                       /*[200]电池组启动失败(系统故障)*/
	{eErr_SysErr,  1000, 20000, 255},                       /*[201]电池组停机带载超时*/
	{eErr_SysErr,  1000, 15000, 255},                       /*[202]电池组停机失败(系统故障)*/
	{eErr_SysErr,     0,  5000,   2},                       /*[203]电池组按钮紧急停机*/
	{eErr_SysPro,  3000,  3000, 255},                       /*[204]断路器未合闸(836原理图查无物理硬件,定性废弃)*/
	{eErr_SysPro,  5000,  3000, 255},                       /*[205]电池组内外压差过大*/
	{eErr_Warm,       0,  3000, 255},                       /*[206]电池组带载闭合*/
	{eErr_Warm,       0,  3000, 255}                        /*[207]电池组带载切断*/
};

const u16 cGAlarmDispBit[ALARM_INDEX_LEN] =                 /*需要对外显示的告警位置1*/
{
    0x0ffc, 0xffff, 0xfff0, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,0xffff
};

const u16 cGFaultDispBit[FAULT_INDEX_LEN] =                 /*需要对外显示的故障位置1*/
{
    0xfeff, 0xffff, 0x3fff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

u16 (*pGRealAlarmInfo)[ALARM_INDEX_LEN] = NULL;
u16 (*pGRealFaultInfo)[FAULT_INDEX_LEN] = NULL;

extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/
/*=================================================================================================
** @Name      : static u8 DiagGetExpBefState(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障码ID
** @Output    : 故障前置状态:0:正常 1:故障/告警
** @Function  : 获取故障前置状态
** @The notes : 当前显示状态
**===============================================================================================*/
static u8 DiagGetExpBefState(u8 clunum, e_ERR_ID id);
/*=================================================================================================
** @Name      : static void DiagSetErrDisplayState(u8 clunum,e_ERR_ID id, u8 state)
** @Input     : clunum:簇编号 id:故障码ID state:设置状态:0清除状态 1:发生状态
** @Output    : void
** @Function  : 设置显示故障码状态
** @The notes : 清0或置1
**===============================================================================================*/
static void DiagSetErrDisplayState(u8 clunum,e_ERR_ID id, u8 state);
/*=================================================================================================
** @Name      : static u8 DiagBuildErrEventAddInfo(e_ERR_ID id, u16 *addInfo, u8 clunum)
** @Input     : id:故障码ID addInfo:构建的附加信息数组(长度必须是5) clunum:簇编号
** @Output    : 构建结果:TRUE成功 FALSE失败
** @Function  : 构建故障事件记录附加信息
** @The notes :
**===============================================================================================*/
static u8 DiagBuildErrEventAddInfo(e_ERR_ID id, u16 *addInfo, u8 clunum);
/*=================================================================================================
** @Name      : static void DiagRecordHisErrInfo(e_ERR_ID id, eErr_RcdSort sort, u8 clunum)
** @Input     : id:故障码ID sort:记录故障类型  1告警产生 2告警消除 3故障产生 4故障消除 clunum:簇编号
** @Output    : void
** @Function  : 记录历史故障信息
** @The notes : 显示历史故障记录并故障事件记录
**===============================================================================================*/
static void DiagRecordHisErrInfo(e_ERR_ID id, eErr_RcdSort sort, u8 clunum);
/*=================================================================================================
** @Name      : static u8 SendErrRcdInfoAPI(t_ RcdCreateInfo *rcdInfo, u8 clunum)
** @Input     : rcdInfo:记录信息 clunum:簇编号
** @Output    : TRUE成功 FALSE失败
** @Function  : 创建一条故障记录,并通过核间通信发送
** @The notes : 用户需要发送一条记录时调用
**===============================================================================================*/
static u8 SendErrRcdInfoAPI(t_RcdCreateInfo *rcdInfo, u8 clunum);


/*=============================================================================================
** @Name      : void DiagUserInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void DiagUserInfoInit(void)
{
	static u8 FirstInitFlag = 0;
	if(FirstInitFlag >= 1)
	{
		return;
	}
    // 分配内存：CLUTER_MAX_NUM 行，每行 ALARM_INDEX_LEN 个 u16
    pGRealAlarmInfo = ExtMalloc(CLUTER_MAX_NUM * sizeof(*pGRealAlarmInfo));
    if (pGRealAlarmInfo == NULL) 
    {
		printf("malloc pGRealAlarmInfo fail!!\r\n");
		return;
        // 处理分配失败
    }
    else
    {
        printf("malloc pGRealAlarmInfo %d success\r\n",CLUTER_MAX_NUM * sizeof(*pGRealAlarmInfo));
        memset(pGRealAlarmInfo, 0, CLUTER_MAX_NUM * sizeof(*pGRealAlarmInfo));
    }
    // 分配内存：CLUTER_MAX_NUM 行，每行 ALARM_INDEX_LEN 个 u16
    pGRealFaultInfo = ExtMalloc(CLUTER_MAX_NUM * sizeof(*pGRealFaultInfo));
    if (pGRealFaultInfo == NULL) 
    {
		printf("malloc pGRealFaultInfo fail!!\r\n");
		return;
        // 处理分配失败
    }
    else
    {
        printf("malloc pGRealFaultInfo %d success\r\n",CLUTER_MAX_NUM * sizeof(*pGRealFaultInfo));
        memset(pGRealFaultInfo, 0, CLUTER_MAX_NUM * sizeof(*pGRealFaultInfo));
    }
	FirstInitFlag = 1;
}
/*=================================================================================================
** @Name      : void DiagErrDisplayTask(void)
** @Input     : void
** @Output    : void
** @Function  : 故障显示任务
** @The notes : 100ms周期,将滤波故障转入故障显示位
**===============================================================================================*/
void DiagErrDisplayTask(void)
{
	u16 flag = 0;
	e_ERR_ID i = eERR_ID_FINISH;
	e_ERR_ID index = eERR_ID_FINISH;
	static u8 sFlag[2] = {0};

	//test
//	DiagRecordHisErrInfo(eERR_ID_BAL_TEXP, eERR_RCDSORT_ERRSET, 0);
//	DiagRecordHisErrInfo(eERR_ID_BAL_TEXP, eERR_RCDSORT_ERRSET, 1);
	for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
	{
		if(clunum >= 2)
		{
			break;
		}
		i = eERR_ID_FINISH;
		index = eERR_ID_FINISH;
		for(i = (e_ERR_ID)0; i < eERR_ID_FINISH; i++)           /*检测滤波最大个数*/
		{
			if(eErr_B == DiagGetErrLastStateAPI(clunum,i))             /*出现告警*/
			{
				if(0 == DiagGetExpBefState(clunum,i))                  /*前置状态不是异常*/
				{
					/*产生级别更高的故障*/
					if(DiagGetFilErrSortAPI(i) > DiagGetFilErrSortAPI(index))
					{
						index = i;                              /*记录故障定位码*/
						//flag = DiagGetErrDetailInfo(i);         /*记录故障定位详细信息*/
					}

					/*产生告警*/
					if((eErr_ChgLim == DiagGetFilErrSortAPI(i))
						|| (eErr_DhgLim == DiagGetFilErrSortAPI(i))
						|| (eErr_SysLim == DiagGetFilErrSortAPI(i))
						|| (eErr_ChgPro == DiagGetFilErrSortAPI(i))
						|| (eErr_DhgPro == DiagGetFilErrSortAPI(i)))
					{
						/*告警数目计数*/
						GroupHisErrNumAddHandle(clunum, 0);

						/*记录显示已初始化*/
						if(sFlag[clunum] > 0)
						{
							/*历史故障记录告警信息产生*/
							DiagRecordHisErrInfo(i, eERR_RCDSORT_WARSET, clunum);
						}
					}
					/*产生故障*/
					else if((eErr_SysPro == DiagGetFilErrSortAPI(i))
						|| (eErr_SysErr == DiagGetFilErrSortAPI(i)))
					{
						/*故障数目计数*/
						GroupHisErrNumAddHandle(clunum, 1);

						/*记录显示已初始化*/
						if(sFlag[clunum] > 0)
						{
							/*历史故障记录故障信息产生*/
							DiagRecordHisErrInfo(i, eERR_RCDSORT_ERRSET, clunum);
						}
					}
					/*产生提醒*/
					else if(eErr_Warm == DiagGetFilErrSortAPI(i))
					{
						/*记录显示已初始化*/
						if(sFlag[clunum] > 0)
						{
							/*历史故障记录告警信息产生*/
							DiagRecordHisErrInfo(i, eERR_RCDSORT_WARSET, clunum);
						}
					}
				}

				DiagSetErrDisplayState(clunum, i, 1);                   /*设置产生异常状态*/
			}
			else                                                		/*无异常*/
			{
				if(1 == DiagGetExpBefState(clunum, i))                  /*前置状态异常*/
				{
					/*记录显示已初始化*/
					if(sFlag[clunum] > 0)
					{
						/*消除告警*/
						if((eErr_ChgLim == DiagGetFilErrSortAPI(i))
							|| (eErr_DhgLim == DiagGetFilErrSortAPI(i))
							|| (eErr_SysLim == DiagGetFilErrSortAPI(i))
							|| (eErr_ChgPro == DiagGetFilErrSortAPI(i))
							|| (eErr_DhgPro == DiagGetFilErrSortAPI(i)))
						{
							/*历史故障记录告警信息消除*/
							DiagRecordHisErrInfo(i, eERR_RCDSORT_WARCLR, clunum);
						}
						/*消除故障*/
						else if((eErr_SysPro == DiagGetFilErrSortAPI(i))
							|| (eErr_SysErr == DiagGetFilErrSortAPI(i)))
						{
							/*历史故障记录故障信息消除*/
							DiagRecordHisErrInfo(i, eERR_RCDSORT_ERRCLR, clunum);
						}
						/*消除提示*/
						else if(eErr_Warm == DiagGetFilErrSortAPI(i))
						{
							/*历史故障记录告警信息产生*/
							DiagRecordHisErrInfo(i, eERR_RCDSORT_WARCLR, clunum);
						}
					}
				}

				DiagSetErrDisplayState(clunum, i, 0);                   /*清除异常状态*/
			}
		}
	}
}
/*=================================================================================================
** @Name      : static u8 DiagGetExpBefState(u8 clunum, e_ERR_ID id)
** @Input     : clunum:簇编号 id:故障码ID
** @Output    : 故障前置状态:0:正常 1:故障/告警
** @Function  : 获取故障前置状态
** @The notes : 当前显示状态
**===============================================================================================*/
static u8 DiagGetExpBefState(u8 clunum, e_ERR_ID id)
{
    u8 state = 0;
    u8 index = 0;
    u8 expbit = 0;

    if(pGRealAlarmInfo == NULL || pGRealFaultInfo == NULL)
    {
         printf("pGRealAlarmInfo || pGRealFaultInfo = NULL\r\n");
         return (0);
    }
    if(clunum >= 2)
    {
         printf("clunum >= 2\r\n");
         return (0);
    }

    if(id < eERR_ID_WARFIN)                                 //告警ID(按位显示)
    {
        //计算故障位
        index = (u32)id / ALARM_DATATYPE;                   //获得告警id的数组子索引
        if(index >= ALARM_INDEX_LEN)                        //告警数组子索引越界
        {
            return(0xff);                                   //返回无效
        }

        expbit = id % ALARM_DATATYPE;                       //获得告警相应位

        state = (u8)BitGet(pGRealAlarmInfo[clunum][index], expbit); //获得告警位状态
    }
    else if(id < eERR_ID_FINISH)                            //故障ID(按位显示)
    {
        //计算故障位
        index = (u32)(id - eERR_ID_ERRSTART) / FAULT_DATATYPE;//获得故障id的数组子索引
        if(index >= FAULT_INDEX_LEN)                        //故障数组子索引越界
        {
            return(0xff);                                   //返回无效
        }

        expbit = id % FAULT_DATATYPE;                       //获得故障相应位

        state = (u8)BitGet(pGRealFaultInfo[clunum][index], expbit); //获得故障位状态
    }
    else                                                    //ID无效
    {
        return(0xff);                                       //返回无效
    }

    return(state);
}
/*=================================================================================================
** @Name      : static void DiagSetErrDisplayState(u8 clunum,e_ERR_ID id, u8 state)
** @Input     : clunum:簇编号 id:故障码ID state:设置状态:0清除状态 1:发生状态
** @Output    : void
** @Function  : 设置显示故障码状态
** @The notes : 清0或置1
**===============================================================================================*/
static void DiagSetErrDisplayState(u8 clunum,e_ERR_ID id, u8 state)
{
    u8 index = 0;
    u8 expbit = 0;
    if(pGRealAlarmInfo == NULL || pGRealFaultInfo == NULL)
    {
         printf("pGRealAlarmInfo || pGRealFaultInfo = NULL\r\n");
         return;
    }
	if(gAlmFault1_57 == NULL || gAlmFault2_58 == NULL)
    {
         printf("gAlmFault1_57 || gAlmFault2_58 = NULL\r\n");
         return;
    }
    if(clunum >= 2)
    {
         printf("clunum >= 2\r\n");
         return;
    }

    if(id < eERR_ID_WARFIN)                                 /*告警ID(按位显示)*/
    {
        /*计算故障位*/
        index = (u32)id / ALARM_DATATYPE;                   /*获得告警id的数组子索引*/
        if(index >= ALARM_INDEX_LEN)                        /*告警数组子索引越界*/
        {
            return;                                         /*无效*/
        }

        expbit = id % ALARM_DATATYPE;                       /*获得告警相应位*/

        if(1 == state)                                      /*告警发生*/
        {
            BitSet(pGRealAlarmInfo[clunum][index], expbit);         /*设置告警位状态置1*/

            if((1 == BitGet(cGAlarmDispBit[index], expbit)) /*策略需要显示该告警位状态*/
                || (TRUE == DiagCheckErrNeedDispHOOK(clunum,id)))  /*用户需要显示该告警位状态*/
            {
                if(clunum == 0)
                    BitSet(gAlmFault1_57[index], expbit);
                else
                    BitSet(gAlmFault2_58[index], expbit);
            }
        }
        else
        {
            BitClr(pGRealAlarmInfo[clunum][index], expbit);         /*消除告警位状态清0*/
			if(clunum == 0)
                BitClr(gAlmFault1_57[index], expbit);          /*消除告警显示位状态清0*/
			else
				BitClr(gAlmFault2_58[index], expbit);          /*消除告警显示位状态清0*/
        }
    }
    else if(id < eERR_ID_FINISH)                            /*故障ID(按位显示)*/
    {
        /*计算故障位*/
        index = (u32)(id - eERR_ID_ERRSTART) / FAULT_DATATYPE;/*获得故障id的数组子索引*/
        if(index >= FAULT_INDEX_LEN)                        /*故障数组子索引越界*/
        {
            return;                                         /*无效*/
        }

        expbit = id % FAULT_DATATYPE;                       /*获得故障相应位*/

        if(1 == state)                                      /*故障发生*/
        {
            BitSet(pGRealFaultInfo[clunum][index], expbit);         /*设置故障位状态置1*/

            if(1 == BitGet(cGFaultDispBit[index], expbit))  /*显示该故障位状态*/
            {
				if(clunum == 0)
                    BitSet(gAlmFault1_57[index+ALARM_INDEX_LEN], expbit);      /*设置故障显示位状态置1*/
				else
					BitSet(gAlmFault2_58[index+ALARM_INDEX_LEN], expbit);      /*设置故障显示位状态置1*/
            }
        }
        else
        {
            BitClr(pGRealFaultInfo[clunum][index], expbit); /*消除故障位状态清0*/
			if(clunum == 0)
        		BitClr(gAlmFault1_57[index + ALARM_INDEX_LEN], expbit);          /*消除故障显示位状态清0*/
			else
				BitClr(gAlmFault2_58[index + ALARM_INDEX_LEN], expbit);          /*消除故障显示位状态清0*/
        }
    }
}


/*=================================================================================================
** @Name      : static u8 DiagBuildErrEventAddInfo(e_ERR_ID id, u16 *addInfo, u8 clunum)
** @Input     : id:故障码ID addInfo:构建的附加信息数组(长度必须是5) clunum:簇编号
** @Output    : 构建结果:TRUE成功 FALSE失败
** @Function  : 构建故障事件记录附加信息
** @The notes :
**===============================================================================================*/
static u8 DiagBuildErrEventAddInfo(e_ERR_ID id, u16 *addInfo, u8 clunum)
{
    //当前保护状态和工作状态
    addInfo[0] = ((u16)GetGProtectStateAPI(clunum) << 8) | (GetGWorkStateAPI(clunum) & 0x00ff);

    //当前输出电流
    addInfo[1] = (u16)GetGSampOutCurrAPI(clunum);

    /*根据故障码选择相应自定义附加信息*/
    switch(id)
    {
    	/*单体电压高2级(充电截止)*/
    	case eERR_ID_CEL_VH2:
    		addInfo[2] = GetGCellMaxVoltAPI(clunum);
    		addInfo[3] = GetGCellMaxVNumAPI(clunum);
    		addInfo[4] = GetGCellAvgVoltAPI(clunum);
    		break;

    	/*单体电压低2级(放电截止)*/
    	case eERR_ID_CEL_VL2:
    		addInfo[2] = GetGCellMinVoltAPI(clunum);
    		addInfo[3] = GetGCellMinVNumAPI(clunum);
    		addInfo[4] = GetGCellAvgVoltAPI(clunum);
    		break;

    	/*电池总电压过压2级(充电截止)*/
    	case eERR_ID_SUM_VH2:
    		addInfo[2] = GetGCellMaxVoltAPI(clunum);
    		addInfo[3] = GetGCellMaxVNumAPI(clunum);
    		addInfo[4] = GetGSampSumVoltAPI(clunum);
    		break;

    	/*电池总电压欠压2级(放电截止)*/
    	case eERR_ID_SUM_VL2:
    		addInfo[2] = GetGCellMinVoltAPI(clunum);
    		addInfo[3] = GetGCellMinVNumAPI(clunum);
    		addInfo[4] = GetGSampSumVoltAPI(clunum);
    		break;

    	/*充电温度高2级(充电截止)*/
    	case eERR_ID_CHG_TH2:
    		addInfo[2] = GetGCellMaxTempAPI(clunum);
    		addInfo[3] = GetGCellMaxTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
    		break;

		/*充电温度低2级(充电截止)*/
		case eERR_ID_CHG_TL2:
    		addInfo[2] = GetGCellMinTempAPI(clunum);
    		addInfo[3] = GetGCellMinTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
			break;

    	/*放电温度高2级(放电截止)*/
    	case eERR_ID_DHG_TH2:
    		addInfo[2] = GetGCellMaxTempAPI(clunum);
    		addInfo[3] = GetGCellMaxTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*放电温度低2级(放电截止)*/
    	case eERR_ID_DHG_TL2:
    		addInfo[2] = GetGCellMinTempAPI(clunum);
    		addInfo[3] = GetGCellMinTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*充电过流2级(限制充电)*/
    	case eERR_ID_CHG_CO2:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim];
        	break;

        /*放电过流2级(限制放电)*/
    	case eERR_ID_DHG_CO2:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = GetGDhgLimCurrAPI(clunum);
    		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim];
        	break;

        /*外设温度高2级(充放电限制)*/
    	case eERR_ID_PER_TH2:
    		addInfo[2] = (u16)GetGEnvirTempAPI(clunum);
    		addInfo[3] = (u16)GetGShuntSampTempAPI(clunum);
    		if(GetGEnvirTempAPI(clunum) > GetGShuntSampTempAPI(clunum))
    		{
        		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_Envir2Lim];
    		}
    		else
    		{
        		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_Perip2Lim];
    		}
        	break;

        /*电池组绝缘2级(充放电限制)*/
    	case eERR_ID_LEAK2:
		//未完善绝缘功能
    		//addInfo[2] = GetGPasInsuResistAPI(clunum);
    		//addInfo[3] = GetGNegInsuResistAPI(clunum);
    		//addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_Leak2Lim];
    		//addInfo[4] = GetGPgndNegVoltAPI(clunum);
        	break;

        /*单体电压一致性下降2级(预警)*/
    	case eERR_ID_CV_DIF2:
    		addInfo[2] = GetGCellMaxVoltAPI(clunum);
    		addInfo[3] = GetGCellMinVoltAPI(clunum);
    		addInfo[4] = GetGCellAvgVoltAPI(clunum);
        	break;

        /*单体温度温差过大2级(预警)*/
    	case eERR_ID_CT_DIF2:
    		addInfo[2] = GetGCellMaxTempAPI(clunum);
    		addInfo[3] = GetGCellMinTempAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*充电容量SOC高2级(预警)*/
    	case eERR_ID_CAP_SH2:
    		addInfo[2] = GetGBattSocAPI(clunum);
    		addInfo[3] = GetGBattSoeAPI(clunum);
    		addInfo[4] = GetGCellMaxVoltAPI(clunum);
        	break;

        /*剩余容量SOC低2级(预警)*/
    	case eERR_ID_CAP_SL2:
    		addInfo[2] = GetGBattSocAPI(clunum);
    		addInfo[3] = GetGBattSoeAPI(clunum);
    		addInfo[4] = GetGCellMinVoltAPI(clunum);
        	break;

    	/*以下为电池1级告警ID*/
        /*单体电压高1级(充电保护)*/
    	case eERR_ID_CEL_VH1:
    		addInfo[2] = GetGCellMaxVoltAPI(clunum);
    		addInfo[3] = GetGCellMaxVNumAPI(clunum);
    		addInfo[4] = GetGCellAvgVoltAPI(clunum);
			break;

		/*单体电压低1级(放电保护)*/
    	case eERR_ID_CEL_VL1:
    		addInfo[2] = GetGCellMinVoltAPI(clunum);
    		addInfo[3] = GetGCellMinVNumAPI(clunum);
    		addInfo[4] = GetGCellAvgVoltAPI(clunum);
        	break;

        /*电池总电压过压1级(充电保护)*/
    	case eERR_ID_SUM_VH1:
    		addInfo[2] = GetGCellMaxVoltAPI(clunum);
    		addInfo[3] = GetGCellMaxVNumAPI(clunum);
    		addInfo[4] = GetGSampSumVoltAPI(clunum);
        	break;

        /*电池总电压欠压1级(放电保护)*/
    	case eERR_ID_SUM_VL1:
    		addInfo[2] = GetGCellMinVoltAPI(clunum);
    		addInfo[3] = GetGCellMinVNumAPI(clunum);
    		addInfo[4] = GetGSampSumVoltAPI(clunum);
        	break;

        /*充电温度高1级(充电保护)*/
    	case eERR_ID_CHG_TH1:
    		addInfo[2] = GetGCellMaxTempAPI(clunum);
    		addInfo[3] = GetGCellMaxTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*充电温度低1级(充电保护)*/
    	case eERR_ID_CHG_TL1:
    		addInfo[2] = GetGCellMinTempAPI(clunum);
    		addInfo[3] = GetGCellMinTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

		/*放电温度高1级(放电保护)*/
		case eERR_ID_DHG_TH1:
    		addInfo[2] = GetGCellMaxTempAPI(clunum);
    		addInfo[3] = GetGCellMaxTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
			break;

        /*放电温度低1级(放电保护)*/
    	case eERR_ID_DHG_TL1:
    		addInfo[2] = GetGCellMinTempAPI(clunum);
    		addInfo[3] = GetGCellMinTNumAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*充电过流1级(充电延时保护)*/
    	case eERR_ID_CHG_CO1:
    		addInfo[2] = (u16)GetGSampOutCurrAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_ChgCH1Lim];
        	break;

        /*放电过流1级(放电延时保护)*/
    	case eERR_ID_DHG_CO1:
    		addInfo[2] = (u16)GetGSampOutCurrAPI(clunum);
    		addInfo[3] = GetGDhgLimCurrAPI(clunum);
    		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_DhgCH1Lim];
        	break;

        /*外设温度过高1级(充放电保护)*/
    	case eERR_ID_PER_TH1:
    		addInfo[2] = (u16)GetGEnvirTempAPI(clunum);
    		addInfo[3] = (u16)GetGShuntSampTempAPI(clunum);
    		if(GetGEnvirTempAPI(clunum) > GetGShuntSampTempAPI(clunum))
    		{
        		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_Envir1Lim];
    		}
    		else
    		{
        		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_Perip1Lim];
    		}
        	break;

        /*电池组绝缘1级(充放电保护)*/
    	case eERR_ID_LEAK1:
		//绝缘功能未完善
    		//addInfo[2] = GetGPasInsuResistAPI(clunum);
    		//addInfo[3] = GetGNegInsuResistAPI(clunum);
    		//addInfo[4] = GetGPgndNegVoltAPI(clunum);
        	break;

        /*单体电压一致性下降1级(充放电限制)*/
    	case eERR_ID_CV_DIF1:
    		addInfo[2] = GetGCellMaxVoltAPI(clunum);
    		addInfo[3] = GetGCellMinVoltAPI(clunum);
    		addInfo[4] = GetGCellAvgVoltAPI(clunum);
        	break;

        /*单体温度温差过大1级(充放电限制)*/
    	case eERR_ID_CT_DIF1:
    		addInfo[2] = GetGCellMaxTempAPI(clunum);
    		addInfo[3] = GetGCellMinTempAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*充电容量SOC高1级(充电截止)*/
    	case eERR_ID_CAP_SH1:
    		addInfo[2] = GetGBattSocAPI(clunum);
    		addInfo[3] = GetGBattSoeAPI(clunum);
    		addInfo[4] = GetGCellMaxVoltAPI(clunum);
        	break;

        /*剩余容量SOC低1级(放电截止)*/
    	case eERR_ID_CAP_SL1:
    		addInfo[2] = GetGBattSocAPI(clunum);
    		addInfo[3] = GetGBattSoeAPI(clunum);
    		addInfo[4] = GetGCellMinVoltAPI(clunum);
    		break;

		/*以下为提示类告警ID*/
    	/*SOC显示异常(需要修正)*/
    	case eERR_ID_SOC_EXP:
    		addInfo[2] = GetGBattSocAPI(clunum);
    		addInfo[3] = GetGBattSoeAPI(clunum);
    		addInfo[4] = GetGRealSocTenThousAPI(clunum);
    		break;

    	/*容量检测异常(需要校准)*/
    	case eERR_ID_CAP_EXP:
    		addInfo[2] = GetCapEnerDropExpAPI(clunum);
    		addInfo[3] = GetGroupTotalCapAPI(clunum) / 1000;
    		addInfo[4] = GetGroupTotalEnerAPI(clunum) / 100;
        	break;

        /*充电过流响应超时(电流超限值)*/
    	case eERR_ID_CHG_COT:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_ChgCH2Lim];
        	break;

        /*放电过流响应超时(电流超限值)*/
    	case eERR_ID_DHG_COT:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = GetGDhgLimCurrAPI(clunum);
    		addInfo[4] = gGBmuGenPara_102[eBmuGenPara102_DhgCH2Lim];
        	break;

        /*均衡板温过高(关闭均衡)*/
    	case eERR_ID_BAL_TH:
		//均衡板温未完善
    		// addInfo[2] = GetGBalanMaxTSlaveAPI();
    		// addInfo[3] = (u16)GetGBalanMaxTempAPI();
    		// //菊花链通信从控或自研CAN通信从控
    		// #if((0 == BMU_BSU_COM_TYPE) || (0 == BMU_BSU_CAN_TYPE))
    		// addInfo[4] = GetGroupVoltPasBalanReqState(addInfo[2] - 1);
    		// if((GetGroupVoltPasBalanReqState(addInfo[2] - 1) & 0x0f0000) > 0)
    		// {
    		// 	//超过16串显示在第16串
    		// 	BitSet(addInfo[4], 15);
    		// }
            // #endif
        	break;

        /*低压电源板温异常(提示)*/
    	case eERR_ID_DCV_TH:
    		//addInfo[2] = (u16)GetGDcPwrSampTempAPI();//未完善
    		addInfo[3] = (u16)GetGEnvirTempAPI(clunum);
    		addInfo[4] = (u16)GetGShuntSampTempAPI(clunum);
        	break;

		/*SOS诊断异常(提示)*/
		case eERR_ID_SOS_EXP:
		//sos未实现
			// addInfo[2] = DiagGetGroupSosState(0);
			// addInfo[3] = DiagGetGroupSosResion(0);
			// addInfo[4] = 0;
			break;

		/*电池箱风扇异常(提示)*/
		case eERR_ID_FAN_EXP:
		//无该功能
			// addInfo[2] = TacGetPwmFanErrFlagAPI();
			// addInfo[3] = GetGPerImlu1SignInfoAPI(eIMLU1Info202_FanSign);
			// addInfo[4] = DiagGetRcvPerOffCommuBitAPI();
			break;

		/*电池柜配电板外设(百叶风机空调)状态异常告警(提示)*/
		case eERR_ID_SHU_EXP:
		//无该功能
			// addInfo[2] = TacGetAuxComErrFlagAPI();
			// if(1 == TacGetAuxDevAirTypeAPI())//同飞空调
			// {
			// 	addInfo[3] = ((u16)GetGPerImeu2TempInfoAPI(eIMEU2Info205_TFAirAlarm) << 8) | GetGPerImeu2TempInfoAPI(eIMEU2Info205_TFAirState);
			// }
			// else
			// {
			// 	addInfo[3] = ((u16)GetGPerImeu2TempInfoAPI(eIMEU2Info205_AirAlarm) << 8) | GetGPerImeu2TempInfoAPI(eIMEU2Info205_AirState);
			// }
			// addInfo[4] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_FanSpeed);
			break;

			/*电池柜配电板外设(温湿度采集器/气体检测/空调)通信中断告警(提示)*/
		case eERR_ID_THC_OFF:
			addInfo[2] = GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1);   /*IMEU3告警字1(bit0可燃气体/bit1温湿度/bit2多一体气体聚合)*/
			// addInfo[3] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_DiSign);
			// addInfo[4] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_SenCommu);
			break;

			/*电池柜配电板外设(SPD)告警(充放电保护)*/
		case eERR_ID_SPD_EXP:
			// addInfo[2] = TacGetSpdErrFlagAPI();
			// addInfo[3] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_DiSign);
			// addInfo[4] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_SenCommu);
			break;

			/*电池柜配电板外设(除湿机)通信中断告警(提示)*/
		case eERR_ID_DE_CON:
			addInfo[2] = GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1);   /*IMEU3告警字1(bit3=除湿机失联)*/
			break;

		case eERR_ID_TH_CON:
			addInfo[2] = GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1);   /*IMEU3告警字1(bit1=温湿度失联)*/
			break;

			/*可燃气体传感器通信中断告警(提示)*/
		case eERR_ID_COM_CON:
			addInfo[2] = GetImeu3AlarmInfoAPI(eIMEUAlmInfo_Alm1);   /*IMEU3告警字1(bit0=可燃气体失联)*/
			break;

		/*泄爆阀回检异常告警(提示)*/
		case eERR_ID_EXP_EXP:
			// addInfo[2] = 泄爆阀卡死标志(bit0开/bit1关),static于AuxDevTactic.c,如需上报再开getter
			break;

		/*消防三级告警(充放电保护,事件附加信息随迁自[163])*/
	case eERR_ID_FIRE3_WAR:
		addInfo[2] = GetGCellMinVoltAPI(clunum);
		addInfo[3] = GetGCellMaxTempAPI(clunum);
		addInfo[4] = 0;
		break;

		/*消防四级告警(充放电保护,事件附加信息随迁自[195])*/
	case eERR_ID_FIRE4_WAR:
		addInfo[2] = GetGEnvirTempAPI(clunum);
		addInfo[3] = 0;
		addInfo[4] = 0;
		break;

    	/*故障类型开始*/
		/*以下为从控告警ID(48-51对应[0-3])*/
        /*均衡温度采样异常*/
    	case eERR_ID_BAL_TEXP:
		//test
    		addInfo[2] = 1;
    		addInfo[3] = 1;
    		addInfo[4] = 1;
        	break;

        /*均衡控制异常*/
    	case eERR_ID_BAL_CEXP:
    		//addInfo[2] = DiagGetBalErrSlaveIdAPI();
    		//addInfo[3] = DiagGetBalErrCellBitAPI();
    		// if((DiagGetBalErrCellBitAPI() & 0x0f0000) > 0)
    		// {
    		// 	//超过16串显示在第16串
    		// 	BitSet(addInfo[3], 15);
    		// }
    		// addInfo[4] = GetGBalanMaxTempAPI();
        	break;

        /*PDU环境/功率线缆温度告警附加值(51表℃+50解码;负温u8回绕仅显示用)*/
        case eERR_ID_PDU_TH2:
        case eERR_ID_PDU_TH1:
		addInfo[2] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc6Tem) - 50);
		addInfo[3] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc8Tem) - 50);
		addInfo[4] = 0;
            break;

        case eERR_ID_CAB_TH2:
        case eERR_ID_CAB_TH1:
		addInfo[2] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc8Tem) - 50);
		addInfo[3] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc6Tem) - 50);
		addInfo[4] = 0;
            break;

        /*电池舱温度告警附加值(舱1:NTC1顶/2左/3右,舱2:NTC4右/5顶)*/
        case eERR_ID_COMP_TH1:
		addInfo[2] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc1Tem) - 50);
		addInfo[3] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc2Tem) - 50);
		addInfo[4] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc3Tem) - 50);
            break;

        case eERR_ID_COMP_TH2:
		addInfo[2] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc4Tem) - 50);
		addInfo[3] = (u8)((s16)GetImeu3BoardInfo1API(eIMEUBoardInfo1_Ntc5Tem) - 50);
		addInfo[4] = 0;
            break;

        /*水浸告警*/
        case eERR_ID_FLOOD_ERR:
		addInfo[2] = GetImeu3BoardInfo2API(eIMEUBoardInfo2_WaterFreq);   /*水浸频率Hz(<300告警,0=无效/断线保持)*/
    		addInfo[3] = DiagGetRcvPerOffCommuBitAPI();
    		addInfo[4] = 0;
            break;

        /*电池气体检测浓度异常*/
    	case eERR_ID_CGAS_EXP:
    		addInfo[2] = GetGCellMinVoltAPI(clunum);
    		addInfo[3] = GetGCellMaxTempAPI(clunum);
			addInfo[4] = 0;
    		// if(1 == BitGet(DiagGetPerGasErrFlagAPI(), 2))//气体2告警
    		// {
    		// 	addInfo[4] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_GasConc2);
    		// }
    		// else
    		// {
    		// 	addInfo[4] = GetGPerImeu2SignInfoAPI(eIMEU2Info204_GasConc1);
    		// }
        	break;

    	/*以下为从控内部故障ID(52-55对应[4-7])*/
        /*单体采样电路异常*/
    	case eERR_ID_SAMP_EXP:
    		addInfo[2] = DiagGetCellSErrSlaveIdAPI(clunum);
    		addInfo[3] = DiagGetCellSampErrFlagAPI(clunum);
    		addInfo[4] = GetGCellSumVoltAPI(clunum);
        	break;

        /*单体电压采样断线*/
    	case eERR_ID_SAMP_VOFF:
    		addInfo[2] = DiagGetCellVOffSlaveIdAPI(clunum);
    		addInfo[3] = DiagGetCellVOffCellBitAPI(clunum);
    		if((DiagGetCellVOffCellBitAPI(clunum) & 0x0f0000) > 0)
    		{
    			//超过16串显示在第16串
    			BitSet(addInfo[3], 15);
    		}
    		addInfo[4] = GetGCellSumVoltAPI(clunum);
        	break;

        /*单体温度采样断线*/
    	case eERR_ID_SAMP_TOFF:
    		addInfo[2] = DiagGetCellTOffSlaveIdAPI(clunum);
    		addInfo[3] = DiagGetCellTOffCellBitAPI(clunum);
    		addInfo[4] = GetGCellAvgTempAPI(clunum);
        	break;

        /*单体信息缺失*/
    	case eERR_ID_CELL_MISS:
    		addInfo[2] = DiagGetRcvCellInfoSlaveIdAPI(clunum);
    		addInfo[3] = DiagGetRcvSlaveMsgSlaveIdAPI(clunum);
    		addInfo[4] = GetGCellSumVoltAPI(clunum);
        	break;

		/*以下为主从通信故障ID(56-59对应[8-11])*/
       	/*内部主从级联异常(与下级从控连接)*/
        case eERR_ID_INT_LOFF:
    		addInfo[2] = 0;
    		addInfo[3] = 0;
    		addInfo[4] = GetGCellSumVoltAPI(clunum);
        	break;

        /*内部主从CAN断线故障(与下级从控通讯)*/
    	case eERR_ID_INT_COFF:
    		addInfo[2] = DiagGetRcvCellInfoSlaveIdAPI(clunum);
    		addInfo[3] = DiagGetRcvSlaveMsgSlaveIdAPI(clunum);
    		addInfo[4] = GetGCellSumVoltAPI(clunum);
        	break;

        /*外设主从CAN断线故障(与内部子系统通讯)*/
    	case eERR_ID_PER_COFF:
    		addInfo[2] = DiagGetRcvPerOffCommuBitAPI();
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = GetGDhgLimCurrAPI(clunum);
        	break;

       	/*外部通讯断线(与上级或充放电设备CAN/RS485通讯)*/
    	case eERR_ID_EXT_COFF:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = GetGDhgLimCurrAPI(clunum);
        	break;

		/*以下为主控软件报警ID(60-63对应[12-15])*/
       	/*主控设备地址冲突*/
    	case eERR_ID_MECU_REP:
    		addInfo[2] = gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = GetGDhgLimCurrAPI(clunum);
        	break;

        /*故障位17预留*/
    	case eERR_ID_ERR17_KEPP:
    		addInfo[2] = 0;
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*电池组软件急停*/
    	case eERR_ID_SW_EMERG:
    		addInfo[2] = TacGetGEmergStopFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*电池组自行保护*/
    	case eERR_ID_SELF_PRO:
    		addInfo[2] = TacGetGHappenProFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

		/*以下为主控内部故障ID(64-79对应[16-31])*/
        /*主控内部RTC异常*/
        case eERR_ID_MRTC_EXP:
    		addInfo[2] = (u16)(BSPGetOSMillSecTimeAPI() / 1000);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*主控EEPROM存储异常*/
    	case eERR_ID_MEEP_EXP:
    		addInfo[2] = (u16)(BSPGetOSMillSecTimeAPI() / 1000);
    		addInfo[3] = DEVEXTEEPGetReadyFlag();
    		addInfo[4] = 0;
        	break;

       	/*主控外部Flash异常*/
    	case eERR_ID_MFLA_EXP:
    		addInfo[2] = (u16)(BSPGetOSMillSecTimeAPI() / 1000);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*电池组电流采样电路异常*/
    	case eERR_ID_CSAMP_ERR:
    		addInfo[2] = SampGetCurrSampExpFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

       	/*输出端高压采样异常*/
    	case eERR_ID_HSAMP_EXP:
    		addInfo[2] = DiagGetHigVoltExpFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);

    		//预充电压值异常
    		if(1 == BitGet(DiagGetHigVoltExpFlagAPI(clunum), 1))
    		{
    			//显示预充电压
    			addInfo[4] = GetGSampPreVoltAPI(clunum);
    		}
    		//总压采样值异常
    		else
    		{
    			//显示累计总压
    			addInfo[4] = GetGCellSumVoltAPI(clunum);
    		}
        	break;

        /*硬件板低压侧采样异常*/
    	case eERR_ID_LSAMP_EXP:
    		//addInfo[2] = SampGetLowVSampExpFlagAPI();
    		//addInfo[3] = GetGHallSignVoltAPI();
    		addInfo[4] = GetGEnvirTempAPI(clunum);
        	break;

        /*外围温度采样异常*/
    	case eERR_ID_TSAMP_EXP:
    		//addInfo[2] = SampGetLowVSampExpFlagAPI();
    		addInfo[3] = GetGShuntSampTempAPI(clunum);
    		addInfo[4] = GetGEnvirTempAPI(clunum);
        	break;

        /*绝缘采样异常*/
    	case eERR_ID_LEAK_EXP:
    		addInfo[2] = InsuGetErrState(clunum);
    		addInfo[3] = InsuGetPasValue(clunum);
    		addInfo[4] = InsuGetNegValue(clunum);
    		break;

    	/*PK1+回检异常(簇1主正接触器)*/
    	case eERR_ID_PK0RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK1_1Mai);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*PK2回检异常(簇1负极接触器)*/
    	case eERR_ID_PK1RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK2_1Neg);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*PK3+回检异常(簇2主正接触器)*/
    	case eERR_ID_PK2RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK3_2Mai);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*PK4回检异常(簇2负极接触器)*/
    	case eERR_ID_PK3RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhPK4_2Neg);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*NK1回检异常(簇1PCS禁充禁放)*/
    	case eERR_ID_NK0RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhNK1_1Pcs);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*NK2回检异常(簇2PCS禁充禁放)*/
    	case eERR_ID_NK1RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhNK2_2Pcs);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*NK3回检异常(急停)*/
    	case eERR_ID_NK2RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhNK3_Emer);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*NK4回检异常(空调通讯控制)*/
    	case eERR_ID_NK3RE_EXP:
    		addInfo[2] = TacGetGroupSwhCtrlStateAPI(eGroupSwhNK4_2Air);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

		//以下为主控外围故障ID(80-95对应[32-47])
    	/*主正接触器状态异常(可能主接触器烧结或不能吸合)*/
    	case eERR_ID_K0MAI_EXP:
    		addInfo[2] = ((u16)TacGetGSwhCtrlFailFlagAPI(clunum) << 8) | TacGetGroupRunSwhStateAPI(clunum);
    		addInfo[3] = ((u16)DiagGetSwhStateVExpFlagAPI(clunum) << 8);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);//记录外端电压采样值
        	break;

        /*预充接触器异常(可能预充电阻断开或预充接触器不能吸合)*/
    	case eERR_ID_K1PRE_EXP:
    		addInfo[2] = ((u16)TacGetGSwhCtrlFailFlagAPI(clunum) << 8) | TacGetGroupRunSwhStateAPI(clunum);
    		addInfo[3] = ((u16)DiagGetSwhStateVExpFlagAPI(clunum) << 8);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);//记录外端电压采样值
        	break;

        /*负极接触器异常(可能负极接触器烧结或不能吸合)*/
    	case eERR_ID_K2NEG_EXP:
    		addInfo[2] = ((u16)TacGetGSwhCtrlFailFlagAPI(clunum) << 8) | TacGetGroupRunSwhStateAPI(clunum);
    		addInfo[3] = ((u16)DiagGetSwhStateVExpFlagAPI(clunum) << 8);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);//记录外端电压采样值
        	break;

        /*消防告警异常*/
    	case eERR_ID_FIRE_EXP:
    		addInfo[2] = GetGEnvirTempAPI(clunum);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*电池组熔断器熔断*/
    	case eERR_ID_FUSE_OFF:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*电池组烟雾报警*/
    	case eERR_ID_BATT_SMOG:
    		addInfo[2] = GetGSampPreVoltAPI(clunum);
    		addInfo[3] = TacGetGSmogAlarmFlagAPI();
    		addInfo[4] = 0;
        	break;

        /*电池组启动预充未满(可能带载启动)*/
    	case eERR_ID_STARTUP_VL:
    		addInfo[2] = ((u16)TacGetGroupRunSwhStateAPI(clunum) << 8) | TacGetGStartFailFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

       	/*电池组启动预充过温(预充接触器过热)*/
    	case eERR_ID_STARTUP_TH:
    		addInfo[2] = ((u16)TacGetGroupRunSwhStateAPI(clunum) << 8) | TacGetGStartFailFlagAPI(clunum);
    		addInfo[3] = (u16)GetGEnvirTempAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*电池组启动失败(系统故障)*/
    	case eERR_ID_STARTUP_ERR:
    		addInfo[2] = ((u16)TacGetGroupRunSwhStateAPI(clunum) << 8) | TacGetGStartFailFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*电池组停机带载超时*/
    	case eERR_ID_STOP_CH:
    		addInfo[2] = ((u16)TacGetGroupRunSwhStateAPI(clunum) << 8) | TacGetGStopFailFlagAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = GetGDhgLimCurrAPI(clunum);
        	break;

        /*电池组停机失败(系统故障)*/
    	case eERR_ID_STOP_ERR:
    		addInfo[2] = ((u16)TacGetGroupRunSwhStateAPI(clunum) << 8) | TacGetGStopFailFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*电池组按钮紧急停机*/
    	case eERR_ID_IO_EMERG:
    		addInfo[2] = TacGetGEmergStopFlagAPI(clunum);
    		addInfo[3] = GetGSampSumVoltAPI(clunum);
    		addInfo[4] = GetGSampPreVoltAPI(clunum);
        	break;

        /*断路器未合闸*/
    	case eERR_ID_ISO_OFF:
    		addInfo[2] = GetGSampPreVoltAPI(clunum);
    		addInfo[3] = 0;
    		addInfo[4] = 0;
        	break;

        /*电池组内外压差过大*/
    	case eERR_ID_DC_HV_DIFF:
    		addInfo[2] = GetGSampSumVoltAPI(clunum);
    		addInfo[3] = GetGSampPreVoltAPI(clunum);
    		//addInfo[4] = GetGPgndNegVoltAPI();
			addInfo[4] = 0;
        	break;

        /*电池组带载闭合*/
    	case eERR_ID_SWHON_CH:
    		addInfo[2] = GetGSampPreVoltAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = GetGDhgLimCurrAPI(clunum);
        	break;

        /*电池组带载断开*/
    	case eERR_ID_SWHOFF_CH:
    		addInfo[2] = GetGSampPreVoltAPI(clunum);
    		addInfo[3] = GetGChgLimCurrAPI(clunum);
    		addInfo[4] = GetGDhgLimCurrAPI(clunum);
        	break;

    	default:
    		return(FALSE);
    }

    return(TRUE);
}


/*=================================================================================================
** @Name      : static void DiagRecordHisErrInfo(e_ERR_ID id, eErr_RcdSort sort, u8 clunum)
** @Input     : id:故障码ID sort:记录故障类型  1告警产生 2告警消除 3故障产生 4故障消除 clunum:簇编号
** @Output    : void
** @Function  : 记录历史故障信息
** @The notes : 显示历史故障记录并故障事件记录
**===============================================================================================*/
static void DiagRecordHisErrInfo(e_ERR_ID id, eErr_RcdSort sort, u8 clunum)
{
    t_RcdCreateInfo rcdInfo = {0};

    //参数错误
    if((id >= eERR_ID_FINISH) || (sort >= eERR_RCDSORT_LEN))
    {
        return;
    }

    /*取到相应故障记录附加信息*/
    if(TRUE == DiagBuildErrEventAddInfo(id, &rcdInfo.info1, clunum))
    {
		rcdInfo.type = sort;

		//告警类型
		if((eERR_RCDSORT_WARSET == sort) || (eERR_RCDSORT_WARCLR == sort))
		{
			rcdInfo.code = ERR_RCD_WARCODE_FST + id;        //告警记录码
		}
		//故障类型
		else
		{
			rcdInfo.code = ERR_RCD_ERRCODE_FST + id;        //故障记录码
		}
		//记录并发送簇故障信息
		SendErrRcdInfoAPI(&rcdInfo, clunum);

	
    }

	//更新显示故障记录数目
	//ErrEvntRcdUpdateStateAPI();
}


/*=================================================================================================
** @Name      : static u8 SendErrRcdInfoAPI(t_ RcdCreateInfo *rcdInfo, u8 clunum)
** @Input     : rcdInfo:记录信息 clunum:簇编号
** @Output    : TRUE成功 FALSE失败
** @Function  : 创建一条故障记录,并通过核间通信发送
** @The notes : 用户需要发送一条记录时调用
**===============================================================================================*/
static u8 SendErrRcdInfoAPI(t_RcdCreateInfo *rcdInfo, u8 clunum)
{
	t_SysClock sysClock = {0};
	if(clunum == 0)
	{
		//记录类型
		gGHisErrInfo1_10[0] = rcdInfo->type;

		//告警类型
		if((eERR_RCDSORT_WARSET == rcdInfo->type) || (eERR_RCDSORT_WARCLR == rcdInfo->type))
		{
			gGHisErrInfo1_10[1] = rcdInfo->code;      //告警记录码
		}
		//故障类型
		else
		{
			gGHisErrInfo1_10[1] = rcdInfo->code;      //故障记录码
		}

		/*记录时间信息*/
		sysClock = GetSysDateClockAPI();

		gGHisErrInfo1_10[2] = sysClock.year;                                 //记录时间-年
		gGHisErrInfo1_10[3] = ((u16)sysClock.month << 8) | sysClock.day;     //记录时间-月日
		gGHisErrInfo1_10[4] = ((u16)sysClock.hour << 8) | sysClock.minute;   //记录时间-时分
		gGHisErrInfo1_10[5] = sysClock.second;                               //记录时间-秒

		//显示附加信息
		gGHisErrInfo1_10[6] = rcdInfo->info1;
		gGHisErrInfo1_10[7] = rcdInfo->info2;
		gGHisErrInfo1_10[8] = rcdInfo->info3;
		gGHisErrInfo1_10[9] = rcdInfo->info4;
		gGHisErrInfo1_10[10] = rcdInfo->info5;
	}else if(clunum == 1)
	{
		//记录类型
		gGHisErrInfo2_11[0] = rcdInfo->type;

		//告警类型
		if((eERR_RCDSORT_WARSET == rcdInfo->type) || (eERR_RCDSORT_WARCLR == rcdInfo->type))
		{
			gGHisErrInfo2_11[1] = rcdInfo->code;      //告警记录码
		}
		//故障类型
		else
		{
			gGHisErrInfo2_11[1] = rcdInfo->code;      //故障记录码
		}

		/*记录时间信息*/
		sysClock = GetSysDateClockAPI();

		gGHisErrInfo2_11[2] = sysClock.year;                                 //记录时间-年
		gGHisErrInfo2_11[3] = ((u16)sysClock.month << 8) | sysClock.day;     //记录时间-月日
		gGHisErrInfo2_11[4] = ((u16)sysClock.hour << 8) | sysClock.minute;   //记录时间-时分
		gGHisErrInfo2_11[5] = sysClock.second;                               //记录时间-秒

		//显示附加信息
		gGHisErrInfo2_11[6] = rcdInfo->info1;
		gGHisErrInfo2_11[7] = rcdInfo->info2;
		gGHisErrInfo2_11[8] = rcdInfo->info3;
		gGHisErrInfo2_11[9] = rcdInfo->info4;
		gGHisErrInfo2_11[10] = rcdInfo->info5;
	}

	/*触发发送历史故障信息显示*/
	return(SendIccGroupHisErrInfoMsg(clunum));
}



static int diag_test1(int argc, char *argv[])
{
    if (argc == 0) {
        printf("diag test para error\n");
    }
    if (!strcmp(argv[0], "0"))
    {
		DiagRecordHisErrInfo(eERR_ID_BAL_TEXP, eERR_RCDSORT_ERRSET, 0);
    }else if(!strcmp(argv[0], "1"))
	{
		DiagRecordHisErrInfo(eERR_ID_BAL_TEXP, eERR_RCDSORT_ERRSET, 1);
	}
    else
    {
        printf("the parameters cannot be recognized\n");
        
    }
	return 0;
}
CLI_CMD("diag_test1",
        "diag_test1"
        "\r\n\t diag_test1 0/1",
        diag_test1);