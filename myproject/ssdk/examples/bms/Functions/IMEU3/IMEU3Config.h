/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMEU3Config.h
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMEU3模块配置参数定义
** @Instructions : 
**===============================================================================================*/
#ifndef __IMEU3CONFIG_H_
#define __IMEU3CONFIG_H_

#include "BSPTypeDef.h"
#include "DEVCAN.h"

/*IMEU3模块配置参数*/
#define IMEU3_DEV_ADDR          (30)        /*IMEU3设备通信地址*/
#define IMEU3_CAN_CH            (eCAN2)     /*IMEU3通信CAN通道*/
#define IMEU3_TARGET_ADDR       (50)        /*目标设备地址(BMU)*/
#define IMEU3_REPORT_PERIOD_MS  (1000)      /*周期上报间隔(ms)*/
#define IMEU3_RESP_TIMEOUT_MS   (500)       /*响应超时时间(ms)*/
#define IMEU3_MAX_RETRY         (3)         /*最大重试次数*/

#endif