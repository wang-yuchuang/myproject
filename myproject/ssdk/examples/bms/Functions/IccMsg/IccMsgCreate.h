/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : IccMsgCreate.h
** @Founder      : ZQD808
** @Date         : 2026.01.16
** @Function     : 核间通信通道创建模块
** @Instructions : 根据核间通信协议创建通信通道
**===========================================================================================*/
#ifndef _ICCMSGCREATE_H__
#define _ICCMSGCREATE_H__
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"



/*=================================================================================================
** @Macro definition
**===============================================================================================*/
#define  ICCMSG_CELL_MSG_HEAD              (11)
#define  ICCMSG_CELL_MSG_TAIL              (0xA0)
#define  ICCMSG_CELL_MSG_REMOTE_NAME       "cell_info"//端点号：504


#define  ICCMSG_CYCLE_MSG_HEAD              (8)
#define  ICCMSG_CYCLE_MSG_TAIL              (0xA1)
#define  ICCMSG_CYCLE_MSG_REMOTE_NAME       "cycle_info"//端点号：506

#define  ICCMSG_FAQ_MSG_HEAD                (2)
#define  ICCMSG_FAQ_MSG_TAIL                (0xA2)
#define  ICCMSG_FAQ_MSG_REMOTE_NAME         "faq_info"//端点号：508

#define  ICCMSG_UPDATE_MSG_HEAD             (13)
#define  ICCMSG_UPDATE_MSG_TAIL             (0xA3)
#define  ICCMSG_UPDATE_MSG_REMOTE_NAME      "update_info"//端点号：510

// #define  ICCMSG_PCS_MSG_HEAD                (15)
// #define  ICCMSG_PCS_MSG_TAIL                (0xA4)
// #define  ICCMSG_PCS_MSG_REMOTE_NAME         "pcs_info"//端点号：512

// #define  ICCMSG_PCSCFG_MSG_HEAD              (17)
// #define  ICCMSG_PCSCFG_MSG_TAIL                 (0xA5)
// #define  ICCMSG_PCSCFG_MSG_REMOTE_NAME       "pcscfg_info"//端点号：514
/*=================================================================================================
** @Data type definition
**===============================================================================================*/

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : void IccMsgInit(void)
** @Input     : void
** @Output    : void
** @Function  : 创建核间通信通道
** @The notes :
**===============================================================================================*/
void IccMsgInit(void);

#endif
