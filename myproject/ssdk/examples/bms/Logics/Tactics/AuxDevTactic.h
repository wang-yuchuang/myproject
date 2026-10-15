/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : AuxDevTactic.c
** @Founder      : ZQD808
** @Date         : 2026.03.19
** @Function     : 辅助设备控制策略模块
** @Instructions :
**===========================================================================================*/
#ifndef _AUXDEVTACTIC_H
#define _AUXDEVTACTIC_H

#define MODBUS_SALVE_ADDR_AIR               (1)         /*液冷空调从机地址*/
#define HEAT_MANANG_MODBUS_PORT              (2)                        /*热管理modbus通讯端口号*/

/*************************MODBUS 同飞液冷空调通讯地址划分开始*************************************************/
#define MB_TF_AIR_WR_BASE_ADDR           (0x1000)                /*液冷空调读写信息起始地址(包括)*/
#define MB_TF_AIR_WR_END_ADDR            (0x1012)                /*液冷空调读写信息结束地址(不包括)*/

#define MB_TF_AIR_RO_BASE_ADDR           (0x0000)                /*液冷空调状态信息起始地址(0x0000-0x0015,含空洞)*/
#define MB_TF_AIR_RO_STATE_LEN            (22)                    /*液冷空调状态信息读取长度*/

#define MB_TF_AIR_RO_BASE1_ADDR           (0x0007)                /*液冷空调只读信息1起始地址(包括)*/
#define MB_TF_AIR_RO_BASE2_ADDR           (0x000B)                /*液冷空调只读信息2起始地址(包括)*/
#define MB_TF_AIR_RO_BASE3_ADDR           (0x0012)                /*液冷空调只读信息3起始地址(包括)*/
#define MB_TF_AIR_RO_BASE4_ADDR           (0x0015)                /*液冷空调只读信息4起始地址(包括)*/
#define MB_TF_AIR_RO_BASE5_ADDR           (0x0019)                /*液冷空调只读信息5起始地址(包括)*/
#define MB_TF_AIR_RO_BASE6_ADDR           (0x0022)                /*液冷空调只读信息6起始地址(包括)*/

#define MB_TF_AIR_RO_FAULT_ADDR           (0x0019)                /*液冷空调故障信息起始地址(0x0019-0x0026,含空洞)*/
#define MB_TF_AIR_RO_FAULT_LEN            (14)                    /*液冷空调故障信息读取长度*/

#define MB_TF_AIR_RO_STATS_ADDR           (0x003C)                /*液冷空调统计信息起始地址(功率/累计)*/
#define MB_TF_AIR_RO_STATS_LEN            (10)                    /*液冷空调统计信息读取长度*/
/*************************MODBUS 同飞液冷空调通讯地址划分结束*************************************************/














/*=============================================================================================
** @Name      : void TacAuxDevManagTask(void)
** @Input     : void
** @Output    : void
** @Function  : 辅助板管理任务
** @The notes : 500ms驱动1s周期,后续再补充其他内容
**===========================================================================================*/
void TacAuxDevManagTask(void);

/*=============================================================================================
** @Name      : void TacImiu4InfoSync(void)
** @Input     : void
** @Output    : void
** @Function  : IMIU4采样/告警直拷到ICC mIndex=67/68汇总数组
** @The notes : 500ms周期,挂OSUserHook(不受Imeu3En门控)
**===========================================================================================*/
void TacImiu4InfoSync(void);

#endif