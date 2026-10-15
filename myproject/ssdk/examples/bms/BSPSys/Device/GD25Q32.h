/*==========================================Copyright(c)=======================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information--------------------------------------
** @File Name    : GD25Q32.h
** @Founder      : ZQD808
** @Date         : 2025.11.25
** @Function     : 外部NOR FLash GD25Q32驱动模块
** @Instructions : 独立硬件OSPI通讯
**===========================================================================================*/
#ifndef __GD25Q32_H__
#define __GD25Q32_H__
#include "BSPTypeDef.h"
#include "GroupInfo.h"


#define EXT_FLA_PAGE_SIZE               4096            //外部FLASH一个扇区(页)大小(4k)
#define EXT_FLA_BLOCK_SIZE              32768           //外部FLASH划分块大小(32k,8页)
#define EXT_FLA_ADDR_SIZE               4194304         //外部FLASH地址容量大小(4Mbyte)
#define EXT_FLA_WR_MAX_SIZE             256             //外部FLASH一次最多写大小
#define EXT_FLA_RO_MAX_SIZE             1024            //外部FLASH一次最多读大小(1kbyte[需防止读耗时太长])


#define EXT_FLA_CRC_ADDR                4190208         //外部FLASH就绪校验地址(最后1页)
#define EXT_FLA_CRC_FLAG                0x5a5a5a5a      //外部FLASH就绪校验标志


/*=============================================================================================
** @Name      : u8 NORFlashGD25Init(void)
** @Input     : void
** @Output    : void
** @Function  : 外部Flash初始化
** @The notes : 检测外部Flash芯片是否就绪
**===========================================================================================*/
u8 NORFlashGD25Init(void);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHWriteByte(u32 addr, u8 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:写入Flash数据块 size:数据长度[1,256]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 将8位字节数据写入到外部Flash中
** @The notes :
**===========================================================================================*/
u8 DEVEXTFLASHWriteByte(u32 addr, u8 *pData, u32 size);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHWriteWord(u32 addr, u16 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:写入Flash数据块 size:数据长度[1,128]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 将16位单字数据写入到外部Flash中
** @The notes :
**===========================================================================================*/
u8 DEVEXTFLASHWriteWord(u32 addr, u16 *pData, u32 size);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHWriteLong(u32 addr, u32 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:写入Flash数据块 size:数据长度[1,64]
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 将32位双字数据写入到外部Flash中
** @The notes :
**===========================================================================================*/
u8 DEVEXTFLASHWriteLong(u32 addr, u32 *pData, u32 size);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHReadByte(u32 addr, u8 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:读出的数据块指针 size:数据长度(最大一块32k)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从外部Flash指定地址读取任意长度的8位字节数据
** @The notes : 读出的数据是若干个8位数
**===========================================================================================*/
u8 DEVEXTFLASHReadByte(u32 addr, u8 *pData, u32 size);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHReadWord(u32 addr, u16 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:读出的数据块指针 size:数据长度(最大一块32k)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从外部Flash指定地址读取任意长度的16位单字数据
** @The notes : 读出的数据是若干个16位数
**===========================================================================================*/
u8 DEVEXTFLASHReadWord(u32 addr, u16 *pData, u32 size);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHReadLong(u32 addr, u32 *pData, u32 size)
** @Input     : addr:Flash地址(0-0x3FFFFFF) *pData:读出的数据块指针 size:数据长度(最大一块32k)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 从外部Flash指定地址读取任意长度的16位单字数据
** @The notes : 读出的数据是若干个16位数
**===========================================================================================*/
u8 DEVEXTFLASHReadLong(u32 addr, u32 *pData, u32 size);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHEraseSector(u32 addr)
** @Input     : addr:Flash地址(0-0x3FFFFFF,必须是4k倍率)
** @Output    : TRUE:成功 FALSE:失败
** @Function  : 外部Flash擦除1扇区Flash(1页)
** @The notes : 擦除指定地址起4K
**===========================================================================================*/
u8 DEVEXTFLASHEraseSector(u32 addr);
/*=============================================================================================
** @Name      : u8 DEVEXTFLASHGetReadyFlag(void)
** @Input     : void
** @Output    : 0:未就绪 1:就绪
** @Function  : 获取外部Flash就绪状态标志
** @The notes : 与外部Flash芯片通讯状态
**===========================================================================================*/
u8 DEVEXTFLASHGetReadyFlag(void);
/*=============================================================================================
** @Name      : void EXTFlashCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 外部Flash检测任务
** @The notes : 1s
**===========================================================================================*/
void EXTFlashCheckTask(void);

#endif