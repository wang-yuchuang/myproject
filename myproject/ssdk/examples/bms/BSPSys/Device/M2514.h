/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : EepM24512.H
** @Founder      : ZQD808
** @Date         : 2025.11.20
** @Function     : 外部EEPROM驱动模块
** @Instructions : 
**===========================================================================================*/
#ifndef  __M2514_H__
#define  __M2514_H__

#include "BSPTypeDef.h"
#include "GroupInfo.h"

#define EepRpmI2C           10               //EEPROM所在I2C

#define Eep_Page_Size		128				//页字节128个
#define Eep_Max_Addr		0xffff			//最大可访问地址

#define M24512_Devic_Addr   0x50    		//器件设备地址

#define Eep_Ver_Addr		0xfff8			//用户校验EEPROM有效性访问地址(必须4字节对齐,addr+4<=Eep_Max_Addr)
#define Eep_Ver_Flag		0x5aa5a55a		//用户校验EEPROM有效性标志

#define MAX_I2C_WRITE_SIZE  64              //I2C驱动单次最大写入字节数
#define MAX_WORDS_PER_CALL  128             // 每次调用最大支持的字数（256字节）

#define MAX_I2C_READ_SIZE  64    // I2C驱动单次最大读取字节数



/*=============================================================================================
** @Name      : u8 EXTEep24M512Init(void)
** @Input     : void
** @Output    : void
** @Function  : 外部EEPROM-M24512初始化
** @The notes :
**===========================================================================================*/
u8 EXTEep24M512Init(void);
/*=============================================================================================
** @Name      : int DEVEXTEEPWriteByte(u16 addr, u8 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要写入的数据  length:要写入数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 向外部EEPROM指定地址写入length个字节
** @The notes : 尽量不要跨页写(一页128Byte)
**===========================================================================================*/
int DEVEXTEEPWriteByte(u16 addr, u8 *buffer, int length);
/*=============================================================================================
** @Name      : int DEVEXTEEPWriteWord(u16 addr, u16 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要写入的数据  length:要写入数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 向外部EEPROM指定地址写入length个字节
** @The notes : 尽量不要跨页写(一页128Byte)
**===========================================================================================*/
int DEVEXTEEPWriteWord(u16 addr, u16 *buffer, int length);
/*=============================================================================================
** @Name      : int DEVEXTEEPWriteLong(u16 addr, u32 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要写入的数据  length:要写入数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 向外部EEPROM指定地址写入length个字节
** @The notes : 尽量不要跨页写(一页128Byte)
**===========================================================================================*/
int DEVEXTEEPWriteLong(u16 addr, u32 *buffer, int length);
/*=============================================================================================
** @Name      : int DEVEXTEEPReadByte(u16 addr, u8 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要读取的数据  length:读取数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 从外部EEPROM指定地址读出length个字节
** @The notes :
**===========================================================================================*/
int DEVEXTEEPReadByte(u16 addr, u8 *buffer, int length);
/*=============================================================================================
** @Name      : int DEVEXTEEPReadWord(u16 addr, u16 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要读取的数据  length:读取数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 从外部EEPROM指定地址读出length个字节
** @The notes :
**===========================================================================================*/
int DEVEXTEEPReadWord(u16 addr, u16 *buffer, int length);
/*=============================================================================================
** @Name      : int DEVEXTEEPReadLong(u16 addr, u32 *buffer, int length)
** @Input     : addr:EEPROM地址[0,65535] buffer:要读取的数据  length:读取数据的长度
** @Output    : 操作结果 TRUE:成功 FALSE:失败
** @Function  : 从外部EEPROM指定地址读出length个字节
** @The notes :
**===========================================================================================*/
int DEVEXTEEPReadLong(u16 addr, u32 *buffer, int length);
/*=============================================================================================
** @Name      : u8 DEVEXTEEPGetReadyFlag(void)
** @Input     : void
** @Output    : 0:未就绪 1:就绪
** @Function  : 获取外部EEPROm就绪状态标志
** @The notes : 与外部EEPROM芯片通讯状态
**===========================================================================================*/
u8 DEVEXTEEPGetReadyFlag(void);
/*=============================================================================================
** @Name      : void DEVEXTEEPCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 外部EEPROM检测任务
** @The notes : 4s
**===========================================================================================*/
void DEVEXTEEPCheckTask(void);
#endif
