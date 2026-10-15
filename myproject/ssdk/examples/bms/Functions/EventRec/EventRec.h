/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : EventRec.h
** @Founder      : WYC834
** @Date         : 2026.07.31
** @Function     : 电池事件记录处理模块
** @Instructions : 
**===============================================================================================*/
#ifndef _EVENT_H
#define _EVENT_H

/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"

/*=================================================================================================
** @Macro definition
**===============================================================================================*/
#define EVN_RCD_EVNINFO_LEN        6                        /*一条历史事件信息长度(Word)*/
#define EVN_RCD_INIOCODE_FST       0x0400                   /*事件记录输入IO变化事件码前缀*/
#define EVN_RCD_OUTIOCODE_FST      0x0800                   /*事件记录输出IO变化事件码前缀*/
#define EVN_RCD_STATECODE_FST      0x1000                   /*事件记录状态变化事件码前缀*/
#define EVN_RCD_CMDCODE_FST        0x2000                   /*事件记录控制命令类事件码前缀*/
#define EVN_RCD_DBGCODE_FST        0x4000                   /*事件记录控制命令类事件码前缀*/
#define EVN_RCD_CFGCODE_FST        0x8000                   /*事件记录参数配置类事件码前缀*/

#define EVN_RCD_OUTIO_BIT          0xffcf                   /*事件记录输出IO变化检测位(不检测LED控制)*/

/*=================================================================================================
** @Data type definition
**===============================================================================================*/
/*事件记录类型*/
typedef enum
{
    eEVN_RCDSORT_OUTPUT   = 5,               /*输入口变化事件*/
    eEVN_RCDSORT_INPUT  = 6,               /*输出口变化事件*/
    eEVN_RCDSORT_MUXOUT  = 7,               /*复用输出口变化事件*/
    eEVN_RCDSORT_MUXIN1  = 8,               /*复用输入口1变化事件*/
    eEVN_RCDSORT_MUXIN2  = 9,               /*复用输入口2变化事件*/
    eEVN_RCDSORT_STATE   = 10,               /*状态量变化事件*/
    eEVN_RCDSORT_CMD     = 11,               /*接收控制命令事件*/
    eEVN_RCDSORT_DEBUG   = 12,               /*接收调试命令事件*/
    eEVN_RCDSORT_CFG     = 13,              /*接收参数配置事件*/
    //eEVN_RCDSORT_SELF  = 14,              /*自身内部事件(事件记录包使用)*/
    eEVN_RCDSORT_USER100 = 100,             /*用户自定义事件*/

    eEVN_RCDSORT_LEN                        /*事件类型*/
}eEvent_RcdSort;

/*用户自定义事件记录ID*/
typedef enum
{
    eEvn_User_CLimChan = 1,                 /*充电电流限值变化事件*/
    eEvn_User_DLimChan = 2,                 /*放电电流限值变化事件*/
    eEvn_User_DodAdd,                       /*DOD记录增加事件*/
    eEvn_User_SysPwrOn,                     /*系统开机上电事件*/
    eEvn_User_DoorDI,                       /*电池柜门开闭事件*/
    eEvn_User_DeMode,                       /*除湿机工作模式事件*/
    eEvn_User_FanChan,                      /*百叶风机风速变化事件*/
    eEvn_User_IntTChan,                     /*柜内环境温度变化事件*/
    eEvn_User_ExtTChan,                     /*柜外环境温度变化事件*/
    eEvn_User_IntHChan,                     /*柜内环境湿度变化事件*/
    eEvn_User_GasChan,                      /*柜内气体浓度变化事件*/
    eEvn_User_PrChgFin,                     /*电池组预充完成事件*/

    eEvn_User_Len                        	/*用户自定义事件ID长度*/
}eUserEvn_RcdId;

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/

/*=================================================================================================
** @Name      : void EventRecordUserTask(void)
** @Input     : void
** @Output    : void
** @Function  : 事件记录用户任务(定周期检查变化事件)
** @The notes : 100ms周期
**===============================================================================================*/
void EventRecordUserTask(void);

/*=================================================================================================
** @Name      : void EventRcdDisplayUserAPI(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum)
** @Input     : sort:事件类型 id:事件码ID info2-5:附加信息2-5  clunum:簇编号
** @Output    : void
** @Function  : 事件记录与显示用户接口
** @The notes : 用户需产生事件的地方调用,事件发送到A核并记录存储
**===============================================================================================*/
void EventRcdDisplayUserAPI(eEvent_RcdSort sort, u8 id, u16 info2, u16 info3, u16 info4, u16 info5, u8 clunum);

#endif
