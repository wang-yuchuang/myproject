/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : GroupPara.c
** @Founder      : ZQD808
** @Date         : 2025.12.13
** @Function     : 电池组参数模块
** @Instructions : 
**===============================================================================================*/
#include "GroupPara.h"
#include "GroupInfo.h"
#include "ParaInterface.h"
u16 gGBmuGenPara_102[GINDEX102_LEN] =  	/*主控通用参数值*/
{
    23130,							/*系统通用参数配置标志	23130*/
	50,								/*设备地址[50,66]*/
	500,							/*组1当前SOC	0.10%	[0,1000]*/
	500,							/*组2当前SOC	0.10%	[0,1000]*/
	480,							/*组1当前SOE	0.10%	[0,1000]*/
	480,							/*组2当前SOE	0.10%	[0,1000]*/
	2700,							/*组1实际总容量0.1AH	[1,5000]*/
	2700,							/*组2实际总容量0.1AH	[1,5000]*/
	2150,							/*组1实际总电能0.1kWh(整数215.0kWh为配置标称电能[电流用]/小数208.1kWh为配置总电能[剩余电能用])[1,5000]*/
	2150,							/*组2实际总电能0.1kWh(整数215.0kWh为配置标称电能[电流用]/小数208.1kWh为配置总电能[剩余电能用])[1,5000]*/
	2800,							/*出厂额定容量0.1AH(整数280.0AH为配置标称容量[电流用]/小数260.1AH为配置额定容量[SOH用])		[1,5000]*/
	5000,							/*额定衰减次数[1,15000]*/

	3600,							/*单体高压二级告警限值[2000,5000]	*/
	3650,							/*单体高压一级保护限值1mv [2000,5000]*/
	2900,							/*单体低压二级告警限值1mv	 [1000,4000]*/
	2800,							/*单体低压一级保护限值1mv	 [1000,4000]*/

	50,								/*充电高温二级告警限值1℃	 [20,80]*/
	55,								/*充电高温一级保护限值1℃	 [20,80]*/
	55,								/*放电高温二级告警限值1℃	 [20,80]*/
	60,								/*放电高温一级保护限值1℃	 [20,80]*/
	0,                              /*充电低温二级告警限值1℃	[-40,30]*/
	(u16)-10,                       /*充电低温一级保护限值1℃ [-40,30]*/
	(u16)-10,                       /*放电低温二级告警限值1℃ [-40,30]*/
	(u16)-20,                       /*放电低温一级保护限值1℃ [-40,30]*/

	8520,							/*总压高压二级告警限值0.1V [100,10000]*/
	8640,							/*总压高压一级保护限值0.1V [100,10000]*/
	7080,							/*总压低压二级告警限值0.1V [100,10000]*/
	6840,							/*总压低压一级保护限值0.1V [100,10000]*/

	2100,							/*充电过流二级告警限值0.1A [10,10000]*/
	2300,							/*充电过流一级保护限值0.1A [10,10000]*/
	2100,							/*放电过流二级告警限值0.1A [10,10000]*/
	2300,							/*放电过流一级保护限值0.1A [10,10000]*/

	1400,							/*充电最大请求电流值0.1A	[10,10000]*/
	1400,							/*放电最大请求电流值0.1A	[10,10000]*/
	300,							/*充电截止电流值0.1A	[10,10000]*/
	500,							/*放电截止电流值0.1A	[10,10000]*/
	2150,							/*充电最大请求功率值0.1kW	 [1,5000*/
	2150,							/*放电最大请求功率值0.1kW	 [1,5000]*/
	1000,							/*充电输入额定功率值0.1kW	 [1,5000]*/
	1000,							/*放电输出额定功率值0.1kW	 [1,5000]*/

	300,							/*绝缘二级告警限值1Ω/V	[1,1000]*/
	100,							/*绝缘一级保护限值1Ω/V	[1,1000]*/

	80,								/*环境温度高温二级告警限值1℃ [10,200]*/
	100,							/*环境温度高温一级保护限值1℃ [10,200]*/
	130,							/*外设温度高温二级告警限值1℃ [10,200]*/
	150,							/*外设温度高温一级保护限值1℃ [10,200]*/

	200,							/*单体压差过大二级限值1mv	 [20,5000]*/
	300,							/*单体压差过大一级限值1mv	 [20,5000]*/
	15,								/*单体温差过大二级限值1℃	 [3,60]*/
	30,								/*单体温差过大一级限值1℃	 [3,60]*/

	970,							/*电池充电容量高二级限值0.10% [500,1000]*/
	1000,							/*电池充电容量高一级限值0.10%	 [500,1000]*/
	100,							/*电池剩余容量低二级限值0.10%	 [0,500]*/
	0,								/*电池剩余容量低一级限值0.10%	 [0,500]*/

	3420,							/*电池容量高一级预警对应电压值[充电容量高必须到达的电压]1mv	[2000,5000]*/
	3280,							/*电池容量低一级预警对应电压值[放电容量低必须到达的电压]1mv	[1000,4000]*/

	200,							/*单体高压二级回差(释放)值1mv	 [1,1000]*/
	270,							/*单体高压一级回差(释放)值1mv	 [1,1000]*/
	150,							/*单体低压二级回差(释放)值1mv	 [1,1000]*/
	270,							/*单体低压一级回差(释放)值1mv	 [1,1000]*/

	5,								/*充电高温二级回差(释放)值1℃	 [1,60]*/
	5,								/*充电高温一级回差(释放)值1℃	 [1,60]*/
	5,								/*放电高温二级回差(释放)值1℃	 [1,60]*/
	5,								/*放电高温一级回差(释放)值1℃	 [1,60]*/

	5,								/*充电低温二级回差(释放)值1℃	 [1,60]*/
	5,								/*充电低温一级回差(释放)值1℃	 [1,60]*/
	5,								/*放电低温二级回差(释放)值1℃	 [1,60]*/
	5,								/*放电低温一级回差(释放)值1℃    [1,60]*/

	100,							/*总压高压二级消除回差值0.1v [10,1000]*/
	150,							/*总压高压一级消除回差值0.1v [10,1000]*/
	100,							/*总压低压二级消除回差值0.1v [10,1000]*/
	150,							/*总压低压一级消除回差值0.1v [10,1000]*/

	50,								/*充电二级过流消除回差值0.1A [10,10000]*/
	200,							/*充电一级过流消除回差值值0.1A [10,10000]*/
	50,								/*放电二级过流消除回差值值0.1A [10,10000]*/
	200,							/*放电一级过流消除回差值0.1A [10,10000]*/

	10,								/*环境温度高温二级消除回差值1℃ [1,60]*/
	20,								/*环境温度高温一级消除回差值1℃ [1,60]*/
	30,								/*外设温度高温二级消除回差值1℃ [1,60]*/
	30,								/*外设温度高温一级消除回差值1℃ [1,60]*/

	30,								/*释放充电限制SOC回差值0.10%	[1,500]*/
	30,								/*释放放电限制SOC回差值0.10%	[1,500]*/
	3300,							/*强制允许充电最大电压值1mv [2000,5000]*/
	3250,							/*强制允许放电最小电压值1mv [1000,4000*/
	3450,							/*单体电压平台区间上限1mv	[2000,5000]*/
	3150							/*单体电压平台区间下限1mv	[1000,4000]*/
};

u16 gGBmuHigLevPara_103[GINDEX103_LEN] =/*主控高级参数值*/
{
    23130,							/*系统扩展参数配置标志*/
	1,								/*单板管理簇数 [1,2]*/
	8,								/*组内管理从控数目(PACK数量) [1,32]*/
	4,								/*单PACK内模组数量 [1,10]*/
	13,								/*一个从控单体电压采样节数 [1,18]*/
	13,								/*一个从控单体温度采样节数 [1,4]*/
	0,								/*一个从控采集均衡板温节数 [0,2]*/
	416,							/*单体电压采样总节数 [1,400]*/
	416,								/*单体温度采样总节数 [1,128]*/
	0,								/*均衡板温上传总节数 [0,64](0表示只检测不上传,否则跟随单体温度上传)*/

	13,								/*充电限制请求保护时间1s	[0,6000]*/
	13,								/*放电限制请求保护时间1s	[0,6000]*/
	8,								/*充电保护延迟保护时间1s	[0,6000]*/
	8,								/*放电保护延迟保护时间1s	[0,6000]*/
	8,								/*系统故障延迟保护时间1s	[0,6000]*/
	10,								/*外部通讯故障延迟保护时间1s	[0,6000](1#/2#IMIU4任一断线停机联动,默认10s)*/
	1800,							/*低压二级延迟关机时间1s	[0,6000]*/
	600,							/*低压一级延迟关机时间1s	[0,6000]*/
	0,								/*系统故障延迟关机时间1s	[0,6000]*/
	0,								/*外部通讯故障延迟关机时间1s	[0,6000]*/

	38,								/*散热风扇开启温度值1℃	[-20,60]*/
	31,								/*散热风扇关闭温度值1℃	[-20,60]*/
	0,								/*加热膜开启温度值1℃	[-40,40]*/
	15,								/*加热膜关闭温度值1℃	[-40,40]*/

	1,								/*充电末端限流恒压使能状态[0,1]	0:禁止 1:使能*/
	1,								/*放电末端限流恒压使能状态[0,1]	0:禁止 1:使能*/
	1,								/*充电高温降功率使能状态[0,1] 	0:禁止 1:使能*/
	1,								/*放电高温降功率使能状态[0,1]	 	0:禁止 1:使能*/
	1,								/*充电低温降功率使能状态[0,1]		0:禁止 1:使能*/
	1,								/*放电低温降功率使能状态[0,1]		0:禁止 1:使能*/
	0,								/*温度调节高低压限值使能状态[0,1]	0:禁止 1:使能*/

	1,								/*开机按钮信号检测端口	按位表示，每bit表示一个IO*/
	4,								/*断路器信号检测端口		按位表示，每bit表示一个IO口*/
	0,								/*熔断器信号检测端口		按位表示，每bit表示一个IO口*/
	8,								/*烟感故障信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*急停按钮信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*防雷器故障信号检测端口	按位表示，每bit表示一个IO口*/
	32,								/*消防告警信号检测端口	按位表示，每bit表示一个IO口*/
	2,								/*气体浓度信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*风扇状态信号检测端口	按位表示，每bit表示一个IO口*/

	4,								/*绝缘检测方式[0,8] 0:不检测 1:非运行检测不报故障 2:非运行检测上报故障 3:全程检测不报故障 4:全程检测非运行上报故障 5:全程检测全程上报故障 6:仅运行检测不上报(请求时检测上报) 7:仅运行检测上报(请求时检测上报) 8:仅请求时检测上报*/
	3,								/*粘连检测方式[0,3] 0:不检测 1:Link电路 2:外端电压 3:Link电路和外端电压,默认1*/

	200,							/*主接触器安全电流值0.1A	[0,5000]*/
	100,							/*预充接触器安全电流值0.1A	[0,500]*/
	100,							/*预充电阻阻值1Ω	[1,1000]*/
	100,							/*预充电阻额定功率值1W	[1,1000]*/
	200,							/*允许启动内外安全压差0.1V [0,10000]*/
	100,							/*预充完成判定内外压差0.1V [0,10000]*/
	20,								/*电流至少预充时间0.1s	[0,600]*/
	100,							/*电流预充超时时间0.1s	[0,600]*/
	200,							/*自启动前等待时间0.1s	[0,600]*/
	500,							/*断开接触器外端残存电压值0.1V	[0,10000]*/
	100,							/*总电压正常至少电压值0.1V	[0,1000]*/
	100,							/*总电压一致允许差值0.1V	[0,1000]*/

	1,								/*绝缘检测后PE继电器状态 [0,1](0:断开 1:闭合)*/
	21600,							/*绝缘检测周期1s [1,64800]默认6H(21600s)*/
	1000,							/*绝缘采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认1500ms])*/
	5000,							/*负母线电压采样(无绝缘检测时)周期1ms [10,60000](更新负母线电压时间[默认1000ms])*/
	1,								/*粘连检测后Link继电器状态 [0,1](0:断开 1:闭合)*/
	21600,							/*粘连检测周期1s [1,64800]默认6H(21600s)*/
	700,							/*负载电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认700ms])*/
	600,							/*Link+电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认600ms])*/
	1500,							/*Link-电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认2000ms])*/
	500,							/*板上继电器切换至少间隔时间1ms [10,10000](防止继电器动作太快[默认500ms])*/

	34,								/*充电高温降功率起始温度1℃ [1,80]*/
	7,								/*充电高温降功率起始温差1℃ [1,80]*/
	100,							/*充电高温降功率下降比率0.1%/℃ [1,500]*/
	700,							/*充电高温降功率最低电流0.1A [10,5000]*/

	35,								/*放电高温降功率起始温度1℃ [1,80]*/
	8,								/*放电高温降功率起始温差1℃ [1,80]*/
	100,							/*放电高温降功率下降比率0.1%/℃ [1,500]*/
	800								/*放电高温降功率最低电流0.1A [10,5000]*/
};

u8 gGHardPara_104[GINDEX104_LEN] =  	/*主控硬件参数值*/
{
	90,                             //硬件参数配置标志
	1,                              //均衡策略算法 [0禁止均衡 1压差积分均衡]
	0,                              //均衡电路     [0被动放电均衡 1主动充电+主动放电均衡]
	2,                              //均衡检测周期     0.1Min
	10,                             //均衡采样周期     0.1s
	30,                             //充电均衡压差 1mv
	50,                             //放电均衡压差 1mv

	3,                              /*电流采集通道[0:外部高压模块,1霍尔,2分流器,3霍尔和分流器]*/
	75,                             /*分流器单向电压范围[1,200]1mV*/
	30,                             /*分流器单向电流量程[1,200]10A*/
	25,                             /*霍尔信号零点电压[1,200]0.1V*/
	20,                             /*霍尔单向电压范围[1,200]0.1V*/
	30,                             /*霍尔单向电流量程[1,200]10A*/
	1,                              /*电流滤波方式[0不滤波 1瞬时平均值 2瞬时有效值 3积分平均值]*/
	50,                             /*电流滤波次数[1,200]1次*/

	55,                             //充电积分零点电流值[1,100]10mA
	65,                             //放电积分零点电流值[1,100]10mA
	6,                              //充电显示零点电流值[1,100]0.1A
	7,                              //放电显示零点电流值[1,100]0.1A
	8,                              //待机时充电零点电流值[1,100]0.1A
	9,                              //待机时放电零点电流值[1,100]0.1A

	0,                              //从控软件版本号最低位(支线变更版本)
	1,                              //从控软件版本号次低位(支线归档版本)
    0,                              //从控软件版本号次高位(主线变更版本[识别电池PACK类型[预留]])
    2,                              //从控软件版本号最高位(主线归档版本[识别从控采集板类型:0:检测程序所有采集板 1:BSU02S1[BYD AFE] 2:BSU02S2[DTNXP AFE] 3:BSU02S3[TI AFE] 4:BSU01S1[ADI AFE]])

    32,           					/*空调开启制冷电池温度[10,60]1℃(默认34℃)*/
    27,           					/*空调停止制冷电池温度[10,60]1℃(默认30℃)*/
    5,           					/*空调开启制热电池温度[-30,20]1℃(默认5℃)*/
    10,           					/*空调停止制热电池温度[-30,20]1℃(默认10℃)*/

    75,           					/*PDU环境温度高二级告警值*/
    80,           					/*PDU环境温度高一级告警值*/
    80,           					/*PDU线缆温度高二级告警值*/
    85,           					/*PDU线缆温度高一级告警值*/
    75,           					/*PCS风扇出风口温度高二级告警*/
    80,           					/*PCS风扇出风口温度高一级告警*/
    90,           					/*空调开启除湿湿度[50,99]1%,(默认80%)*/
    80,           					/*空调停止除湿湿度[45,95]1%,(默认70%)*/

    1,           					/*空调类型 0：未安装 1：YFY自研  2：TF  [0,2]*/
    1,           					/*全氟己酮功能 0：不使能  1：使能                     [0,1]*/
    1,           					/*灯带控制功能 0：不使能  1：使能                     [0,1]*/
    0,           						/*漏电流传感器类型 0：未安装 1：柯力三电(836无此传感器,默认未安装) [0,1]*/
    45,           					/*Pdu散热风扇开启温度                                         [20,80]*/
    70,           					/*Pdu散热风扇全速温度                                         [20,80]*/
    45,           					/*Pcs散热风扇开启温度                                         [20,80]*/
    70,           					/*Pcs散热风扇全速温度                                         [20,80]*/
    30,           					/*泄爆阀开启间隔，单位：天                                   [1,90]*/
    3,           					/*泄爆阀开启时长 单位：min           [1,30]*/
    0,           					/*IMEU3使能*/
    80,           					/*电池仓高温告警值*/
    40,           					/*漏电流传感器告警电流值*/
    80,           					/*PACK连接器高温告警值*/
    0,           					/*项目编码*/
    3           					/*IMIU4使能状态*/
};
u16 gGBmuGenParaWR_107[GINDEX107_LEN] =  	/*主控通用参数值*/
{
	23130,							/*系统通用参数配置标志	23130*/
	50,								/*设备地址[50,66]*/
	500,							/*组1当前SOC	0.10%	[0,1000]*/
	500,							/*组2当前SOC	0.10%	[0,1000]*/
	480,							/*组1当前SOE	0.10%	[0,1000]*/
	480,							/*组2当前SOE	0.10%	[0,1000]*/
	2700,							/*组1实际总容量0.1AH	[1,5000]*/
	2700,							/*组2实际总容量0.1AH	[1,5000]*/
	1811,							/*组1实际总电能0.1kWh(整数188.0kWh为配置标称电能[电流用]/小数181.1kWh为配置总电能[剩余电能用])[1,5000]*/
	1811,							/*组2实际总电能0.1kWh(整数188.0kWh为配置标称电能[电流用]/小数181.1kWh为配置总电能[剩余电能用])[1,5000]*/
	2601,							/*出厂额定容量0.1AH(整数280.0AH为配置标称容量[电流用]/小数260.1AH为配置额定容量[SOH用])	[1,5000]*/
	5000,							/*出厂衰减次数[1,15000]*/

	3600,							/*单体高压二级告警限值[2000,5000]	*/
	3650,							/*单体高压一级保护限值1mv [2000,5000]*/
	2900,							/*单体低压二级告警限值1mv	 [1000,4000]*/
	2800,							/*单体低压一级保护限值1mv	 [1000,4000]*/

	50,								/*充电高温二级告警限值1℃	 [20,80]*/
	55,								/*充电高温一级保护限值1℃	 [20,80]*/
	55,								/*放电高温二级告警限值1℃	 [20,80]*/
	60,								/*放电高温一级保护限值1℃	 [20,80]*/
	0,                              /*充电低温二级告警限值1℃	[-40,30]*/
	(u16)-10,                       /*充电低温一级保护限值1℃ [-40,30]*/
	(u16)-10,                       /*放电低温二级告警限值1℃ [-40,30]*/
	(u16)-20,                       /*放电低温一级保护限值1℃ [-40,30]*/

	560,							/*总压高压二级告警限值0.1V [100,10000]*/
	580,							/*总压高压一级保护限值0.1V [100,10000]*/
	480,							/*总压低压二级告警限值0.1V [100,10000]*/
	460,							/*总压低压一级保护限值0.1V [100,10000]*/

	600,							/*充电过流二级告警限值0.1A [10,10000]*/
	900,							/*充电过流一级保护限值0.1A [10,10000]*/
	600,							/*放电过流二级告警限值0.1A [10,10000]*/
	900,							/*放电过流一级保护限值0.1A [10,10000]*/

	580,							/*充电最大请求电流值0.1A	[10,10000]*/
	580,							/*放电最大请求电流值0.1A	[10,10000]*/
	300,							/*充电截止电流值0.1A	[10,10000]*/
	500,							/*放电截止电流值0.1A	[10,10000]*/
	100,							/*充电最大请求功率值0.1kW	 [1,5000*/
	100,							/*放电最大请求功率值0.1kW	 [1,5000]*/
	50,								/*充电输入额定功率值0.1kW	 [1,5000]*/
	50,								/*放电输出额定功率值0.1kW	 [1,5000]*/

	300,							/*绝缘二级告警限值1Ω/V	[1,1000]*/
	100,							/*绝缘一级保护限值1Ω/V	[1,1000]*/

	80,								/*环境温度高温二级告警限值1℃ [10,200]*/
	100,							/*环境温度高温一级保护限值1℃ [10,200]*/
	130,							/*外设温度高温二级告警限值1℃ [10,200]*/
	150,							/*外设温度高温一级保护限值1℃ [10,200]*/

	200,							/*单体压差过大二级限值1mv	 [20,5000]*/
	300,							/*单体压差过大一级限值1mv	 [20,5000]*/
	15,								/*单体温差过大二级限值1℃	 [3,60]*/
	30,								/*单体温差过大一级限值1℃	 [3,60]*/

	970,							/*电池充电容量高二级限值0.10% [500,1000]*/
	1000,							/*电池充电容量高一级限值0.10%	 [500,1000]*/
	100,							/*电池剩余容量低二级限值0.10%	 [0,500]*/
	0,								/*电池剩余容量低一级限值0.10%	 [0,500]*/

	3420,							/*电池容量高一级预警对应电压值1mv	[2000,5000]*/
	3280,							/*电池容量低一级预警对应电压值1mv	[1000,4000]*/

	200,							/*单体高压二级回差(释放)值1mv	 [1,1000]*/
	270,							/*单体高压一级回差(释放)值1mv	 [1,1000]*/
	150,							/*单体低压二级回差(释放)值1mv	 [1,1000]*/
	270,							/*单体低压一级回差(释放)值1mv	 [1,1000]*/

	5,								/*高温一级回差(释放)值1℃	 [1,60]*/
	5,								/*高温二级回差(释放)值1℃	 [1,60]*/
	5,								/*高温三级回差(释放)值1℃	 [1,60]*/
	5,								/*高温四级回差(释放)值1℃	 [1,60]*/

	5,								/*低温一级回差(释放)值1℃	 [1,60]*/
	5,								/*低温二级回差(释放)值1℃	 [1,60]*/
	5,								/*低温三级回差(释放)值1℃	 [1,60]*/
	5,								/*低温四级回差(释放)值1℃    [1,60]*/

	100,							/*总压高压二级消除回差值0.1v [10,1000]*/
	150,							/*总压高压一级消除回差值0.1v [10,1000]*/
	100,							/*总压低压二级消除回差值0.1v [10,1000]*/
	150,							/*总压低压一级消除回差值0.1v [10,1000]*/

	50,								/*充电二级过流消除回差值0.1A [10,10000]*/
	200,							/*充电一级过流消除回差值值0.1A [10,10000]*/
	50,								/*放电二级过流消除回差值值0.1A [10,10000]*/
	200,							/*放电一级过流消除回差值0.1A [10,10000]*/

	10,								/*环境温度高温二级消除回差值1℃ [1,60]*/
	20,								/*环境温度高温一级消除回差值1℃ [1,60]*/
	30,								/*外设温度高温二级消除回差值1℃ [1,60]*/
	30,								/*外设温度高温一级消除回差值1℃ [1,60]*/

	30,								/*释放充电限制SOC回差值0.10%	[1,500]*/
	30,								/*释放放电限制SOC回差值0.10%	[1,500]*/
	3300,							/*强制允许充电最大电压值1mv [2000,5000]*/
	3250,							/*强制允许放电最小电压值1mv [1000,4000*/
	3450,							/*单体电压平台区间上限1mv	[2000,5000]*/
	3150							/*单体电压平台区间下限1mv	[1000,4000]*/
};

u16 gGBmuHigLevParaWR_108[GINDEX108_LEN] =/*主控高级参数值*/
{
	23130,							/*系统扩展参数配置标志*/
	1,								/*单板管理簇数 [1,2]*/
	2,								/*组内管理从控数目 [1,32]*/
	1,								/*组内从控并联数目 [1,10]*/
	18,								/*一个从控单体电压采样节数 [1,18]*/
	4,								/*一个从控单体温度采样节数 [1,4]*/
	2,								/*一个从控采集均衡板温节数 [0,2]*/
	18,								/*单体电压采样总节数 [1,400]*/
	4,								/*单体温度采样总节数 [1,128]*/
	2,								/*均衡板温上传总节数 [0,64](0表示只检测不上传,否则跟随单体温度上传)*/

	15,								/*充电限制请求保护时间1s	[0,6000]*/
	15,								/*放电限制请求保护时间1s	[0,6000]*/
	13,								/*充电保护延迟保护时间1s	[0,6000]*/
	13,								/*放电保护延迟保护时间1s	[0,6000]*/
	10,								/*系统故障延迟保护时间1s	[0,6000]*/
	600,							/*外部通讯故障延迟保护时间1s	[0,6000]*/
	1800,							/*低压二级延迟关机时间1s	[0,6000]*/
	600,							/*低压一级延迟关机时间1s	[0,6000]*/
	3600,							/*系统故障延迟关机时间1s	[0,6000]*/
	5400,							/*外部通讯故障延迟关机时间1s	[0,6000]*/

	36,								/*散热风扇开启温度值1℃	[-20,60]*/
	28,								/*散热风扇关闭温度值1℃	[-20,60]*/
	0,								/*加热膜开启温度值1℃	[-40,40]*/
	15,								/*加热膜关闭温度值1℃	[-40,40]*/

	1,								/*充电末端限流恒压使能状态[0,1]	0:禁止 1:使能*/
	1,								/*放电末端限流恒压使能状态[0,1]	0:禁止 1:使能*/
	1,								/*充电高温降功率使能状态[0,1] 	0:禁止 1:使能*/
	1,								/*放电高温降功率使能状态[0,1]	 	0:禁止 1:使能*/
	1,								/*充电低温降功率使能状态[0,1]		0:禁止 1:使能*/
	1,								/*放电低温降功率使能状态[0,1]		0:禁止 1:使能*/
	0,								/*温度调节高低压限值使能状态[0,1]	0:禁止 1:使能*/

	1,								/*开机按钮信号检测端口	按位表示，每bit表示一个IO*/
	0,								/*断路器信号检测端口		按位表示，每bit表示一个IO口*/
	0,								/*熔断器信号检测端口		按位表示，每bit表示一个IO口*/
	0,								/*烟感故障信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*急停按钮信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*防雷器故障信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*绝缘告警信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*绝缘故障信号检测端口	按位表示，每bit表示一个IO口*/
	0,								/*风扇状态信号检测端口	按位表示，每bit表示一个IO口*/

	5,								/*绝缘检测方式[0,8] 0:不检测 1:非运行检测不报故障 2:非运行检测上报故障 3:全程检测不报故障 4:全程检测非运行上报故障 5:全程检测全程上报故障 6:仅运行检测不上报(请求时检测上报) 7:仅运行检测上报(请求时检测上报) 8:仅请求时检测上报*/
	1,								/*粘连检测方式[0,3] 0:不检测 1:Link电路 2:外端电压 3:Link电路和外端电压,默认1*/

	100,							/*主接触器安全电流值0.1A	[0,5000]*/
	50,								/*预充接触器安全电流值0.1A	[0,500]*/
	100,							/*预充电阻阻值1Ω	[1,1000]*/
	100,							/*预充电阻额定功率值1W	[1,1000]*/
	500,							/*允许启动内外安全压差0.1V [0,10000]*/
	300,							/*预充完成判定内外压差0.1V [0,10000]*/
	20,								/*电流至少预充时间0.1s	[0,600]*/
	100,							/*电流预充超时时间0.1s	[0,600]*/
	200,							/*自启动前等待时间0.1s	[0,600]*/
	2000,							/*断开接触器外端残存电压值0.1V [0,10000]*/
	100,							/*高压正常至少电压值0.1V	[0,1000]*/
	100,							/*总电压一致允许差值0.1V	[0,1000]*/

	1,								/*绝缘检测后PE继电器状态 [0,1](0:断开 1:闭合)*/
	21600,							/*绝缘检测周期1s [1,64800]默认6H(21600s)*/
	1500,							/*绝缘采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认1500ms])*/
	5000,							/*负母线电压采样(无绝缘检测时)周期1ms [10,60000](更新负母线电压时间[默认1000ms])*/
	1,								/*粘连检测后Link继电器状态 [0,1](0:断开 1:闭合)*/
	21600,							/*粘连检测周期1s [1,64800]默认6H(21600s)*/
	700,							/*负载电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认700ms])*/
	600,							/*Link+电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认600ms])*/
	2000,							/*Link-电压采样(闭合继电器后)延迟时间1ms [10,10000](等待采样口稳定时间[默认2000ms])*/
	500,							/*板上继电器切换至少间隔时间1ms [10,10000](防止继电器动作太快[默认500ms])*/

	33,								/*充电高温降功率起始温度1℃ [1,80]*/
	7,								/*充电高温降功率起始温差1℃ [1,80]*/
	150,							/*充电高温降功率下降比率0.1%/℃ [1,500]*/
	700,							/*充电高温降功率最低电流0.1A [10,5000]*/

	34,								/*放电高温降功率起始温度1℃ [1,80]*/
	8,								/*放电高温降功率起始温差1℃ [1,80]*/
	100,							/*放电高温降功率下降比率0.1%/℃ [1,500]*/
	800								/*放电高温降功率最低电流0.1A [10,5000]*/
};

u8 gGHardParaWR_109[GINDEX109_LEN] =  	/*主控硬件参数值*/
{
    90,                             //硬件参数配置标志
    1,                              //均衡策略算法 [0禁止均衡 1压差积分均衡]
    0,                              //均衡电路     [0被动放电均衡 1主动充电+主动放电均衡]
    2,                              //均衡检测周期     0.1Min
    10,                             //均衡采样周期     0.1s
    30,                             //充电均衡压差 1mv
    50,                             //放电均衡压差 1mv

    3,                              /*电流采集通道[0:外部高压模块,1霍尔,2分流器,3霍尔和分流器]*/
    75,                             /*分流器单向电压范围[1,200]1mV*/
    30,                             /*分流器单向电流量程[1,200]10A*/
    25,                             /*霍尔信号零点电压[1,200]0.1V*/
    20,                             /*霍尔单向电压范围[1,200]0.1V*/
    30,                             /*霍尔单向电流量程[1,200]10A*/
    1,                              /*电流滤波方式[0不滤波 1瞬时平均值 2瞬时有效值 3积分平均值]*/
    50,                             /*电流滤波次数[1,200]1次*/

    55,                             //充电积分零点电流值[1,100]10mA
    65,                             //放电积分零点电流值[1,100]10mA
    6,                              //充电显示零点电流值[1,100]0.1A
    7,                              //放电显示零点电流值[1,100]0.1A
    8,                              //待机时充电零点电流值[1,100]0.1A
    9,                              //待机时放电零点电流值[1,100]0.1A

    0,                              //从控软件版本号最低位(支线变更版本)
    1,                              //从控软件版本号次低位(支线归档版本)
    0,                              //从控软件版本号次高位(主线变更版本[识别电池PACK类型[预留]])
    2,                              //从控软件版本号最高位(主线归档版本[识别从控采集板类型:0:检测程序所有采集板 1:BSU02S1[BYD AFE] 2:BSU02S2[DTNXP AFE] 3:BSU02S3[TI AFE] 4:BSU01S1[ADI AFE]])

    32,           					/*空调开启制冷电池温度[10,60]1℃(默认34℃)*/
    27,           					/*空调停止制冷电池温度[10,60]1℃(默认30℃)*/
    5,           					/*空调开启制热电池温度[-30,20]1℃(默认5℃)*/
    10,           					/*空调停止制热电池温度[-30,20]1℃(默认10℃)*/

    30,           					/*空调开启制冷环境温度[16,48]1℃,(触发启动制冷温度,默认28℃)*/
    23,           					/*空调停止制冷环境温度[16,38]1℃,(制冷回差温度=启动温度-停止温度,默认23℃)*/
    8,           					/*空调开启制热环境温度[5,26]1℃,(触发开启制热温度,默认8℃)*/
    15,           					/*空调制热偏移环境温度[5,36]1℃,(制热回差温度=停止温度-启动温度,默认15℃)*/
    20,           					/*空调开启除湿温度[20,40]1℃,(即除湿加热开启温度,建议低于制冷设定温度,默认20℃)*/
    25,           					/*空调停止除湿温度[20,40]1℃,(即除湿加热停止温度,建议低于制冷设定温度,默认25℃)*/
    95,           					/*空调开启除湿湿度[50,99]1%,(默认80%)*/
    80,           					/*空调停止除湿湿度[45,95]1%,(默认70%)*/

    1,           					/*空调类型 0：未安装 1：YFY自研  2：TF  [0,2]*/
    1,           					/*全氟己酮功能 0：不使能  1：使能                     [0,1]*/
    1,           					/*灯带控制功能 0：不使能  1：使能                     [0,1]*/
    0,            						/*漏电流传感器类型 0：未安装 1：柯力三电(836无此传感器,默认未安装) [0,1]*/
    45,           					/*Pdu散热风扇开启温度                                         [20,80]*/
    70,           					/*Pdu散热风扇全速温度                                         [20,80]*/
    45,           					/*Pcs散热风扇开启温度                                         [20,80]*/
    70,           					/*Pcs散热风扇全速温度                                         [20,80]*/
    30,           					/*泄爆阀开启间隔，单位：天                                     [1,90]*/
    3,           					/*泄爆阀开启时长 单位：min           [1,30]*/
    0,           					/*IMEU3使能*/
    80,           					/*电池仓高温告警值*/
    40,           					/*漏电流传感器告警电流值*/
    80,           					/*PACK连接器高温告警值*/
    0,           					/*项目编码*/
    3           					/*IMIU4使能状态*/
};

//后续根据实际项目用到的电芯类型根据电芯手册匹配
u8 gGroupParaRO_115[GINDEX115_LEN] =		/*不同温度充电倍率表*/
{
	0,      /*-10°C以下充电倍率[0,200]0.01C*/
	0,      /*-5°C以下充电倍率[0,200]0.01C*/
	5,      /*0°C以下充电倍率[0,200]0.01C*/
	15,     /*5°C以下充电倍率[0,200]0.01C*/
	30,     /*10°C以下充电倍率[0,200]0.01C*/
	50,     /*15°C以下充电倍率[0,200]0.01C*/
	80,     /*20°C以下充电倍率[0,200]0.01C*/
	120,    /*25°C以下充电倍率[0,200]0.01C*/
	120,    /*35°C以下充电倍率[0,200]0.01C*/
	120,    /*40°C以下充电倍率[0,200]0.01C*/
	80,     /*45°C以下充电倍率[0,200]0.01C*/
	50,     /*50°C以下充电倍率[0,200]0.01C*/
	35,     /*55°C以下充电倍率[0,200]0.01C*/
	25,     /*60°C以下充电倍率[0,200]0.01C*/
	0       /*60°C以上充电倍率[0,200]0.01C*/
};

u8 gGroupParaRO_116[GINDEX116_LEN] = 		/*不同温度放电倍率表*/
{
	0,      /*-10°C以下放电倍率[0,200]0.01C*/
	20,     /*-5°C以下放电倍率[0,200]0.01C*/
	25,     /*0°C以下放电倍率[0,200]0.01C*/
	30,     /*5°C以下放电倍率[0,200]0.01C*/
	50,     /*10°C以下放电倍率[0,200]0.01C*/
	120,    /*15°C以下放电倍率[0,200]0.01C*/
	120,    /*20°C以下放电倍率[0,200]0.01C*/
	120,    /*25°C以下放电倍率[0,200]0.01C*/
	120,    /*35°C以下放电倍率[0,200]0.01C*/
	120,    /*40°C以下放电倍率[0,200]0.01C*/
	120,    /*45°C以下放电倍率[0,200]0.01C*/
	50,     /*50°C以下放电倍率[0,200]0.01C*/
	50,     /*55°C以下放电倍率[0,200]0.01C*/
	30,     /*60°C以下放电倍率[0,200]0.01C*/
	0       /*60°C以上放电倍率[0,200]0.01C*/
};

u8 gGroupParaRO_119[GINDEX119_LEN] = 		/*EEP电压平滑电流限值(限流恒压)参数表*/
{
	90, 	/*充电电流限值平滑比率表配置标志*/
	80,     /*第1个点平滑充电电流限值比率[0,100]%*/
	70,     /*第2个点平滑充电电流限值比率[0,100]%*/
	60,     /*第3个点平滑充电电流限值比率[0,100]%*/
	50,     /*第4个点平滑充电电流限值比率[0,100]%*/
	40,     /*第5个点平滑充电电流限值比率[0,100]%*/
	30,     /*第6个点平滑充电电流限值比率[0,100]%*/
	20,     /*第7个点平滑充电电流限值比率[0,100]%*/
	15,     /*第8个点平滑充电电流限值比率[0,100]%*/
	10,     /*第9个点平滑充电电流限值比率[0,100]%*/
	5,      /*第10个点平滑充电电流限值比率[0,100]%*/
	90,     /*放电电流限值平滑比率表配置标志*/
	80,     /*第1个点平滑放电电流限值比率[0,100]%*/
	70,     /*第2个点平滑放电电流限值比率[0,100]%*/
	60,     /*第3个点平滑放电电流限值比率[0,100]%*/
	50,     /*第4个点平滑放电电流限值比率[0,100]%*/
	40,     /*第5个点平滑放电电流限值比率[0,100]%*/
	30,     /*第6个点平滑放电电流限值比率[0,100]%*/
	20,     /*第7个点平滑放电电流限值比率[0,100]%*/
	15,     /*第8个点平滑放电电流限值比率[0,100]%*/
	10,     /*第9个点平滑放电电流限值比率[0,100]%*/
	5,      /*第10个点平滑放电电流限值比率[0,100]%*/

	90,     /*充电电流限值恒压平滑压差表配置标志*/
	11,     /*第1个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	10,     /*第2个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	9,     /*第3个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	8,     /*第4个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	7,     /*第5个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	6,      /*第6个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	5,      /*第7个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	4,      /*第8个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	3,      /*第9个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	2,      /*第10个点恒压平滑充电电流限值目标压差[0,100]10mv*/
	90,     /*放电电流限值恒压平滑压差表配置标志*/
	15,     /*第1个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	13,     /*第2个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	12,     /*第3个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	10,     /*第4个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	8,      /*第5个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	6,      /*第1个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	4,      /*第2个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	3,      /*第3个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	2,      /*第4个点恒压平滑放电电流限值目标压差[0,100]10mv*/
	1       /*第5个点恒压平滑放电电流限值目标压差[0,100]10mv*/
};

/*=================================================================================================
** @Name      : u8 ParaCheckGGenParaEffect(u16 addr, u16 para)
** @Input     : addr:参数地址 para:参数值
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测电池组通用参数值是否有效
** @The notes :
**===============================================================================================*/
u8 ParaCheckGGenParaEffect(u16 addr, u16 para)
{
	/*根据地址判断有效性*/
	switch(addr)
	{
		/*设备通用参数配置标志*/
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CfgFlag):
			/*不需检有效性*/
			return(TRUE);

		/*设备地址节点号*/
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_BmuDev):
			if((MIN_BMU_ADDR_CFG <= para) && (para <= MAX_BMU_ADDR_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//当前SOC
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_1NowSoc):
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_2NowSoc):
		//当前SOE
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_1NowSoe):
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_2NowSoe):
			if(para <= 1000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//总容量(0.1AH)
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_1ReaAllCap):
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_2ReaAllCap):
			if(para <= (MAX_ALL_CAP_CFG / 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//总电能(0.1kWh)
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_1ReaAllEner):
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_2ReaAllEner):
			if(para <= (MAX_ALL_ENER_CFG / 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//额定容量(0.1AH)
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_RateCap):
			if(para <= (MAX_ALL_CAP_CFG / 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//有效衰减次数
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DampNum):
			if((1 <= para) && (para <= 15000))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//单体高压2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_VH2Lim):
		//单体高压1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_VH1Lim):
			if((para >= MIN_VOLT_H_LIM) && (para <= MAX_VOLT_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//单体低压2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_VL2Lim):
		//单体低压1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_VL1Lim):
			if((para >= MIN_VOLT_L_LIM) && (para <= MAX_VOLT_L_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电高温2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgTH2Lim):
		//充电高温1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgTH1Lim):
		//放电高温2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgTH2Lim):
		//放电高温1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgTH1Lim):
			if((para >= MIN_TEMP_H_LIM) && (para <= MAX_TEMP_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电低温2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgTL2Lim):
		//充电低温1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgTL1Lim):
		//放电低温2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgTL2Lim):
		//放电低温1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgTL1Lim):
			if(((s16)para >= MIN_TEMP_L_LIM) && ((s16)para <= MAX_TEMP_L_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//总压高压2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SumVH2Lim):
		//总压高压1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SumVH1Lim):
			if((para >= MIN_SUMV_H_LIM) && (para <= MAX_SUMV_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//总压低压2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SumVL2Lim):
		//总压低压1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SumVL1Lim):
			if((para >= MIN_SUMV_L_LIM) && (para <= MAX_SUMV_L_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电过流2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgCH2Lim):
		//充电过流1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgCH1Lim):
		//放电过流2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgCH2Lim):
		//放电1级过流限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgCH1Lim):
		//充电最大请求电流值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgCMaxLim):
		//放电最大请求电流值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgCMaxLim):
			if((para >= MIN_CURR_H_LIM) && (para <= MAX_CURR_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电截止电流值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgCFinLim):
			if(para <= MAX_CURR_H_LIM)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//放电截止电流值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgCFinLim):
			if(para <= MAX_CURR_H_LIM)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电最大请求功率值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgPMaxLim):
		//放电最大请求功率值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgPMaxLim):
		//充电额定功率值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgPRatLim):
		//放电额定功率值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgPRatLim):
			if((para >= MIN_POWER_H_LIM) && (para <= MAX_POWER_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//绝缘2级预警电限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Leak2Lim):
		//绝缘1级告警电限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Leak1Lim):
			if((para >= 1) && (para <= 1000))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//环境高温2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Envir2Lim):
		//环境高温1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Envir1Lim):
		//外设高温2级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Perip2Lim):
		//外设高温1级限值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Perip1Lim):
			if((para >= MIN_PERT_H_LIM) && (para <= MAX_PERT_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//电压一致性检测2级预警值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_VDiff2SubV):
		//电压一致性检测1级告警值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_VDiff1SubV):
			if((para >= 20) && (para <= MAX_VOLT_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//温度一致性检测2级预警值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_TDiff2SubT):
		//温度一致性检测1级告警值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_TDiff1SubT):
			if((para >= 3) && (para <= 60))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//容量高2级预警SOC值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CapHig2Soc):
		//容量高1级告警SOC值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CapHig1Soc):
			if(para >= 500)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//容量低1级预警SOC值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CapLow2Soc):
		//容量低2级告警SOC值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CapLow1Soc):
			if(para <= 500)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//容量高1级预警SOC点对应电压值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CapHig1Volt):
			if((para >= MIN_VOLT_H_LIM) && (para <= MAX_VOLT_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//容量低1级预警SOC点对应电压值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CapLow1Volt):
			if((para >= MIN_VOLT_L_LIM) && (para <= MAX_VOLT_L_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//单体高压2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CVH2Clr):
		//单体高压1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CVH1Clr):
		//单体低压2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CVL2Clr):
		//单体低压1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CVL1Clr):
			if((para >= 1) && (para <= MAX_ALM_CLR_SUB_CV))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电高温2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CTH2Clr):
		//充电高温1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CTH1Clr):
		//放电高温2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DTH2Clr):
		//放电高温1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DTH1Clr):
		//充电低温2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CTL2Clr):
		//充电低温1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CTL1Clr):
		//放电低温2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DTL2Clr):
		//放电低温1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DTL1Clr):
			if((para >= 1) && (para <= MAX_ALM_CLR_SUB_T))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//总压高压2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SVH2Clr):
		//总压高压1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SVH1Clr):
		//总压低压2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SVL2Clr):
		//总压低压1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_SVL1Clr):
			if((para >= 1) && (para <= MAX_ALM_CLR_SUB_SV))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电2级过流消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgCH2Clr):
		//充电1级过流消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgCH1Clr):
		//放电2级过流消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgCH2Clr):
		//放电1级过流消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgCH1Clr):
			if((para >= 10) && (para <= MAX_ALM_CLR_SUB_C))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//环境高温2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Envir2Clr):
		//环境高温1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Envir1Clr):
		//外设高温2级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Perip2Clr):
		//外设高温1级消除值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_Perip1Clr):
			if((para >= 1) && (para <= MAX_ALM_CLR_SUB_T))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电限值释放的SOC回差值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgLimSubSoc):
		//放电限值释放的SOC回差值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgLimSubSoc):
			if(para <= 500)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电电流限值释放的电压值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_ChgLimClrV):
			if((para >= MIN_VOLT_H_LIM) && (para <= MAX_VOLT_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//放电电流限值释放的电压值
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_DhgLimClrV):
			if((para >= MIN_VOLT_L_LIM) && (para <= MAX_VOLT_L_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//单体电压平台区间上边界
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CellPlaUpV):
			if((para >= MIN_VOLT_H_LIM) && (para <= MAX_VOLT_H_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//单体电压平台区间下边界
		case PARA_DEVGEN_PARA_ADDR + (2 * eBmuGenPara102_CellPlaDnV):
			if((para >= MIN_VOLT_L_LIM) && (para <= MAX_VOLT_L_LIM))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		default:
			break;
	}

	return(FALSE);
}
/*=================================================================================================
** @Name      : u8 ParaCheckGHigParaEffect(u16 addr, u16 para)
** @Input     : addr:参数地址 para:参数值
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测电池组高级参数值是否有效
** @The notes :
**===============================================================================================*/
u8 ParaCheckGHigParaEffect(u16 addr, u16 para)
{
	/*根据地址判断有效性*/
	switch(addr)
	{
		/*设备高级参数配置标志*/
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_CfgFlag):
			/*不需检有效性*/
			return(TRUE);

		/*单板管理簇数*/
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ClusterNum):
		if((para >= MIN_CLUSTER_NUM_CFG) && (para <= MAX_CLUSTER_NUM_CFG))
		{
			/*参数合法*/
			return(TRUE);
		}
		break;

		/*管理从控数目*/
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_BsuNum):
			if((para >= MIN_BSU_NUM_CFG) && (para <= MAX_BSU_NUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*一个从机包含电池箱(从控)数目 [1,30]*/
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ConNum):
			if((para >= MIN_BSU_CONNUM_CFG) && (para <= MAX_BSU_CONNUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//一个从控内电压采样节数值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_BsuVNum):
			if((para >= MIN_BSU_CELVNUM_CFG) && (para <= MAX_BSU_CELVNUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//一个从控内温度采样节数值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_BsuTNum):
			if((para >= MIN_BSU_CELTNUM_CFG) && (para <= MAX_BSU_CELTNUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//一个从控内均衡板温采样节数值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_BalTNum):
			if((para >= MIN_BSU_BALTNUM_CFG) && (para <= MAX_BSU_BALTNUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//组内电压采样总节数值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_VSumNum):
			if((para >= MIN_BMU_CELVNUM_CFG) && (para <= MAX_BMU_CELVNUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//组内温度采样总节数值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_TSumNum):
			if((para >= MIN_BMU_CELTNUM_CFG) && (para <= MAX_BMU_CELTNUM_CFG))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//组内均衡板温采样总节数值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_BTSumNum):
			if(para <= MAX_BMU_BALTNUM_CFG)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电限制超时时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgLimOffT):
		//放电限制超时时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgLimOffT):
		//充电保护延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgProOffT):
		//放电保护延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgProOffT):
		//系统故障保护延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ErrProOffT):
		//外部通讯故障保护延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_CommProOffT):
		//2级低压关机延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_VL2PwrOffT):
		//1级低压关机延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_VL1PwrOffT):
		//系统故障关机延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ErrPWrOffT):
		//外部通讯故障关机延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_CommPwrOffT):
			if(para <= MAX_PRO_TIME_1S)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//控制风扇开启温度值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_FanOpenTemp):
		//控制风扇关闭温度值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_FanCloseTemp):
			if(((s16)para >= -20) && ((s16)para <= 60))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//控制加热开启温度值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_HotOpenTemp):
		//控制加热关闭温度值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_HotCloseTemp):
			if(((s16)para >= -40) && ((s16)para <= 40))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电末端恒压使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgFinCVEn):
		//放电末端恒压使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgFinCVEn):
		//充电高温降功率使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgTHLimEn):
		//放电高温降功率使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgTHLimEn):
		//充电低温降功率使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgTLLimEn):
		//放电低温降功率使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgTLLimEn):
		//温度平滑电压限值使能状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_TSloVLimEn):
			if(para <= 1)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//开机按钮检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PwrPor):
		//断路器检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_IsoPort):
		//熔断器检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_FusPort):
		//烟雾故障检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_SmogPort):
		//急停按钮检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_EmergPort):
		//防雷器故障检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ThundPort):
		//消防告警检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_FirePort):
		//气体告警检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_GasPort):
		//风扇状态检测端口
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_FanPort):
			if(para != 0xffff)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//绝缘监测方式状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_LeakMonitType):
			if(para <= 8)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//粘连检测方式状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_AdhMonitType):
			if(para <= 3)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//主接触器安全电流
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_MainSwhC):
			if(para <= 5000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//预充接触器安全电流
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PChgSwhC):
			if(para <= 500)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//预充电阻阻值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PChgResis):
			if((para >= 1) && (para <= 1000))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//预充电阻功率限值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PChgPower):
			if((para >= 1) && (para <= 1000))
			{
				/*参数合法*/
				return(TRUE);
		}
			break;

		//允许启动安全压差
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_SafeSubV):
			if(para <= 10000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//预充完成安全压差
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PChgFinV):
			if(para <= 10000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//预充等待时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PChgWaitT):
			if(para <= 600)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//预充超时时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PChgOffT):
			if(para <= 600)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//自启动前等待时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_SelfWaitT):
			if(para <= 600)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//断开接触器外端残存电压值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_MSwhOffV):
			if(para <= 10000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//高压采样正常至少电压值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_HigVOffV):
			if(para <= 1000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//总电压一致允许差值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_SumDiffV):
			if(para <= 1000)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//绝缘检测后PE继电器状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_LedkRlyState):
		//粘连检测后Link继电器状态
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_AdhRlyState):
			if(para <= 1)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//绝缘监测周期值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_LeakMonitCycl):
		//粘连检测周期值
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_AdhMonitCycl):
			if((para > 0) && (para <= 64800))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//绝缘采样(闭合继电器后)延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_LedkSampDelyT):
		//负载电压采样(闭合继电器后)延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_Lk0SampDelyT):
		//Link+电压采样(闭合继电器后)延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_Lk1SampDelyT):
		//Link-电压采样(闭合继电器后)延迟时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_Lk2SampDelyT):
		//板上继电器切换至少间隔时间
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_RlyCtrlGapT):
			if((para >= 10) && (para <= 10000))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//负母线电压采样(无绝缘检测时)周期
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_PgndVSampGapT):
			if((para >= 10) && (para <= 60000))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电高温降功率起始温度
		//放电高温降功率起始温度
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgHTDnFstT):
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgHTDnFstT):
			if((para >= 1) && (para <= 80))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电高温降功率起始温差
		//放电高温降功率起始温差
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgHTDnDifT):
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgHTDnDifT):
			if((para >= 1) && (para <= 80))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电高温降功率下降比率
		//放电高温降功率下降比率
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgHTDnRate):
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgHTDnRate):
			if((para >= 1) && (para <= 500))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电高温降功率下降比率
		//放电高温降功率下降比率
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_ChgHTDnMinC):
		case PARA_DEVHIG_PARA_ADDR + (2 * eBmuHigLevPara103_DhgHTDnMinC):
			if((para >= 10) && (para <= 5000))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		default:
			break;
	}

	return(FALSE);
}

/*=================================================================================================
** @Name      : u8 ParaCheckGHWParaEffect(u16 addr, u8 para)
** @Input     : addr:参数):址 para:参数值 len:参数长度
** @Output    : TRUE:有效合法 FALSE:非法无效
** @Function  : 检测电池组硬件参数值是否有效
** @The notes : 用户必须补充完善
**===============================================================================================*/
u8 ParaCheckGHWParaEffect(u16 addr, u8 para)
{
	/*根据地址判断有效性*/
	switch(addr)
	{
		/*设备硬件参数配置标志*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_CfgFlag:
			/*不需检有效性*/
			return(TRUE);

		//均衡策略算法[0禁止均衡 1压差积分均衡]
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BalTact:
			if(para <= 1)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//均衡电路[0被动放电均衡 1主动充电+主动放电均衡]
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BalCirc:
			if(para <= 1)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//均衡检测周期 0.1Min
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BalCycT:
			if((para >= 1) && (para <= 250))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//均衡采样周期0.1s
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BalSamT:
			if((para >= 1) && (para <= 250))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电均衡压差 1mv
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BalChgV:
			if((para >= 10) && (para <= 250))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//放电均衡压差 1mv
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BalDhgV:
			if((para >= 10) && (para <= 250))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*电流采样通道*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_CSampChan:
			if((para >= 0) && (para <= 3))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*分流器电压[1,200]0.01V*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ShuntRate:
			if((para >= 1) && (para <= 200))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*分流器量程[1,200]10A*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ShuntRang:
			if((para >= 1) && (para <= 200))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*霍尔信号零点电压[1,200]0.1V*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_HallSign:
			if((para >= 1) && (para <= 200))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*霍尔信号电压范围[1,200]0.1V*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_HallRate:
			if((para >= 1) && (para <= 200))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		/*霍尔电流量程[1,200]10A*/
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_HallRang:
			if((para >= 1) && (para <= 200))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//电流滤波方式[0不滤波 1瞬时平均值 2瞬时有效值 3积分平均值]
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_CFilType:
			if(para <= 3)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//电流滤波次数[1,200]1次
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_CFilTime:
			if((para > 0) && (para <= 200))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电积分零点电流值[1,100]10mA
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ChgIntPoint:
			if((para >= 1) && (para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//放电积分零点电流值[1,100]10mA
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_DhgIntPoint:
			if((para >= 1) && (para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//充电显示零点电流值[1,100]0.1A
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ChgShowPoint:
			if((para >= 1) && (para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//放电显示零点电流值[1,100]0.1A
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_DhgShowPoint:
			if((para >= 1) && (para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//待机时充电零点电流值[1,100]0.1A
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ChgIdlePoint:
			if((para >= 1) && (para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//待机时放电零点电流值[1,100]0.1A
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_DhgIdlePoint:
			if((para >= 1) && (para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//BMU软件版本号最低位(支线变更版本)
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BmuSWVerLL:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BmuSWVerML:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BmuSWVerMH:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_BmuSWVerHH:
			if(0xff != para)
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//空调开启制冷电池温度[10,60]1℃
		//空调停止制冷电池温度[10,60]1℃
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirColdOn_CT:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirColdOff_CT:
			if(((s8)para >= 10) && ((s8)para <= 60))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//空调开启制热电池温度[-30,20]1℃
		//空调停止制热电池温度[-30,20]1℃
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirHeatOn_CT:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirHeatOff_CT:
			if(((s8)para >= -30) && ((s8)para <= 20))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//PDU环境温度高二级告警值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PduEnvTH2:
			if(((s8)para >= 0) && ((s8)para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//PDU环境温度高一级告警值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PduEnvTH1:
			if(((s8)para >= 0) && ((s8)para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//PDU线缆温度高二级告警值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PduCabTH2:
			if(((s8)para >= 0) && ((s8)para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//PDU线缆温度高一级告警值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PduCabTH1:
			if(((s8)para >= 0) && ((s8)para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//PCS风扇出风口温度高二级告警
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PcsFanTH2:
			if(((s8)para >= 0) && ((s8)para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//PCS风扇出风口温度高二级告警
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PcsFanTH1:
			if(((s8)para >= 0) && ((s8)para <= 100))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//空调开启除湿湿度[50,99]1%
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirDeHumOn_H:
			if((para >= 50) && (para <= 99))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//空调停止除湿湿度[45,95]1%
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirDeHumOff_H:
			if((para >= 45) && (para <= 95))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//液冷空调类型
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_AirType:
			if((para >= 0) && (para <= 16))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;
		//全氟己酮消防功能使能
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PerfluorEn:
			if((para >= 0) && (para <= 16))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

	    //灯带通信功能使能
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_Imlu2En:
			if((para >= 0) && (para <= 16))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//漏电流传感器类型
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_LeakageType:
			if((para >= 0) && (para <= 16))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//Pdu和Pcs散热风扇起转速度和全速温度
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PduFanOpenT:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PduFanFullT:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PcsFanOpenT:
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PcsFanFullT:
			if((para >= 0) && (para <= 80))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//泄爆阀开启间隔
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ExplosInter:
			if((para >= 0) && (para <= 90))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;

		//泄爆阀开启时长
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ExplosTime:
			if((para >= 0) && (para <= 255))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;
		//IMEU3使能
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_Imeu3En:
			if((para >= 0) && (para <= 255))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;
		//电池仓高温告警值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_CompTH:
			if((para >= 0) && (para <= 255))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;
		//漏电流传感器告警电流值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_LeakageC:
			if((para >= 0) && (para <= 255))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;
		//PACK连接器高温告警值
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_PackConnTH:
			if((para >= 0) && (para <= 255))
			{
				/*参数合法*/
				return(TRUE);
			}
			break;
		//项目编码
		case PARA_DEVHW_PARA_ADDR + eGHardPara104_ProjectCode:
		if((para >= 0) && (para <= 16))
		{
			/*参数合法*/
			return(TRUE);
		}
		break;
        case PARA_DEVHW_PARA_ADDR + eGHardPara104_Imiu4En:
            if((para >= 0) && (para <= 3))
            {
                /*参数合法*/
                return(TRUE);
            }
            break;

        default:
            break;
    }

    return(FALSE);
}
