/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : CapEnerCorr.h
** @Founder      : ZQD808
** @Date         : 2026.03.13
** @Function     : 总容量总电能自动标定模块
** @Instructions :
**===========================================================================================*/
#ifndef _CAPENERCORR_H
#define _CAPENERCORR_H
#include "BSPTypeDef.h"
/*=============================================================================================
** @Header file definition
**===========================================================================================*/

/*=============================================================================================
** @Macro definition
**===========================================================================================*/
#define  CAP_CHECK_RECORD_T     600     //容量检测时间记录时间间隔(1s)
#define  CAP_CHECK_MIN_T        600     //容量检测充放电最小有效时间(1s,默认10分钟)
#define  CAP_CHECK_MAX_T        216000  //容量检测充放电最大有效时间(1s,默认60小时[20个充放电循环])
#define  CAP_CHECK_CYC_T        40      //容量检测间隔周期[周期内无满充满放的强制满充满放](1天,默认40天)

/*=============================================================================================
** @Data type definition
**===========================================================================================*/
//容量能量检测记录信息(顺序不能修改,否则需改变EEP记录顺序)
typedef struct
{
    u32 effect;         //是否有效(0/1)
    u32 timer;          //经历时间(1s)
    u32 chgCap;         //充电电量(1AH)
    u32 dhgCap;         //放电电量(1AH)
    u32 chgEner;        //充电电能(0.1kWh)
    u32 dhgEner;        //放电电能(0.1kWh)
    u32 allCap;         //检测到的总容量(1mAH)
    u32 allEner;        //检测到的总能量(1Wh)
    u32 date;           //记录日期(20220610)
}t_CapECorr;

extern size_t ExtxPortGetFreeHeapSize( void );
extern void *ExtMalloc( size_t xWantedSize );
extern void ExtFree( void *pv );

/*=============================================================================================
** @Name      :void CapEnerCorrInfoInit(void)
** @Input     : void
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 全局信息初始化
** @The notes : 由于片内内存限制，使用外部EMMC空间，全部使用ExtMalloc申请内存
**===========================================================================================*/
void CapEnerCorrInfoInit(void);
/*=============================================================================================
** @Name      : void TotalCapEnerCorrTask(void)
** @Input     : void
** @Output    : void
** @Function  : 电池组总容量总能量自动标定任务
** @The notes : 1s
**===========================================================================================*/
void TotalCapEnerCorrTask(void);
/*=============================================================================================
** @Name      : u8 GetCapEnerDropExpAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 容量衰减异常标志(Bit0:充电总容量检测衰减 bit1:放电总容量检测衰减 bit2:充电总能量衰减 bit3:放电总能量衰减  bit4:充电检测超时 bit5:放电检测超时)[0正常 1异常]
** @Function  : 获取总容量总能量衰减异常标志
** @The notes :
**===========================================================================================*/
u8 GetCapEnerDropExpAPI(u8 clunum);
/*=============================================================================================
** @Name      : static void DhgCorrTotalCapEnerDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 放电总容量总能量自动标定处理
** @The notes :
**===========================================================================================*/
static void DhgCorrTotalCapEnerDeal(u8 clunum);
/*=============================================================================================
** @Name      : static void ClrAllCapEnerCorrInfo(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除所有总容量总能量检测记录信息
** @The notes : 清除EEPROM并复位当前检测值
**===========================================================================================*/
static void ClrAllCapEnerCorrInfo(u8 clunum);
/*=============================================================================================
** @Name      : static u8 CheckHisCapEnerClrState(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 0:未清零 1已清零
** @Function  : 检测记录高低压点后历史电量是否清零
** @The notes : 被清零后原记录点无效
**===========================================================================================*/
static u8 CheckHisCapEnerClrState(u8 clunum);
/*=============================================================================================
** @Name      : static u32 CalcTotalCapEnerByCTCorr(u8 clunum, u8 sort, u32 data, s16 curr, s8 temp)
** @Input     : clunum:簇编号 sort:类型(0:总容量 1:总能量) data:修正前检测到的总容量(1mAH)或总能量(1Wh) curr:标定电流(0.1A) temp:标定温度(1℃)
** @Output    : 电流温度修正后的总容量(1mAH)总能量(1Wh)
** @Function  : 计算总容量或总能量标定值经电流和温度修正后的真实值
** @The notes : 根据总容量标定终止时的电流和温度
**===========================================================================================*/
static u32 CalcTotalCapEnerByCTCorr(u8 clunum, u8 sort, u32 data, s16 curr, s8 temp);
/*=============================================================================================
** @Name      : static u32 GetCapCheckNowDate(void)
** @Input     : void
** @Output    : 当时记录日期
** @Function  : 获取容量检测过欠压当时记录日期
** @The notes : 年月日(如20220610)
**===========================================================================================*/
static u32 GetCapCheckNowDate(void);
/*=============================================================================================
** @Name      : static u8 GetCapCheckTimeoutReq(u8 clunum, u8 sort)
** @Input     : clunum:簇编号 sort:容量检测点 0低压点(放空请求) 1高压电(充满请求)
** @Output    : 需满充满放请求
** @Function  : 获取容量检测超时需满充满放请求
** @The notes : 30天未满充满放
**===========================================================================================*/
static u8 GetCapCheckTimeoutReq(u8 clunum, u8 sort);
/*=============================================================================================
** @Name      : static void ChgCorrTotalCapEnerDeal(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 充电总容量总能量自动标定处理
** @The notes :
**===========================================================================================*/
static void ChgCorrTotalCapEnerDeal(u8 clunum);
/*=============================================================================================
** @Name      : void TotalCapEnerCorrInit(void)
** @Input     : void
** @Output    : void
** @Function  : 总容量总能量自动标定初始化
** @The notes :
**===========================================================================================*/
void TotalCapEnerCorrInit(void);

#endif