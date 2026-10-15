/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IccMsgTabOper.h
** @Founder      : WYC834
** @Date         : 2026.03.12
** @Function     : 核间通信包表格读写操作接口
** @Instructions :
**===============================================================================================*/
#ifndef _ICCMSGTABOPER_H_
#define _ICCMSGTABOPER_H_
/*=================================================================================================
** @Header file definition
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "IccMsgInterface.h"
/*=================================================================================================
** @Macro definition
**===============================================================================================*/

/*=================================================================================================
** @Data type definition
**===============================================================================================*/

/*=================================================================================================
** @Interface function declaration
**===============================================================================================*/
/*=================================================================================================
** @Name      : const t_ICCIndexTab *IccScanIndexTabInfo(u8 devaddr, u8 mindex, u16 sIndex, u16 sIndexNum, u8 *errorCode)
** @Input     : devaddr:索引表设备地址 mindex:主索引号  sindex:子索引号  sIndexNum:子索引数目 *errorCode:存储错误码地址
** @Output    : 对应主索引信息
** @Function  : 查找主索引表信息
** @The notes : 并且判断子索引是否越界
**===============================================================================================*/
const t_ICCIndexTab *IccScanIndexTabInfo(u8 devaddr, u8 mindex, u16 sIndex, u16 sIndexNum, u8 *errorCode);

/*=================================================================================================
** @Name      : void ICCClrTxRcvTabInfo(u8 id)
** @Input     : id:核间通信收发通信表序号
** @Output    : void
** @Function  : 清除核间通信收发通信表中的记录
** @The notes :
**===============================================================================================*/
void ICCClrTxRcvTabInfo(u8 id);

/*=================================================================================================
** @Name      : u8 ICCReqIdleTxRcvTab(e_ICCFunCode funcode, void *pdata, u8 dataType, u8 mIndex, u16 sIndex, u16 sIndexNum, u8 conflag, u8 daddr, e_ICCChannelDef iccnum)
** @Input     : funcode:功能码 pdata:数据首地址 dataType:数据类型 mIndex:主索引  sindex:子索引 sindexnum:子索引数目 conflag:确认标志[修改/查询]或映射起始主索引[单体/参数]  daddr:目的地址, iccnum:核间通信通道
** @Output    : 申请结果(0:成功 其它:错误码)
** @Function  : 请求空闲的收发表记录发送信息
** @The notes : 设置定时发送
**===============================================================================================*/
u8 ICCReqIdleTxRcvTab(e_ICCFunCode funcode, void *pdata, u8 dataType, u8 mIndex, u16 sIndex, u16 sIndexNum, u8 conflag, u8 daddr, e_ICCChannelDef iccnum);

/*=================================================================================================
** @Name      : u8 ICCSendTxRcvTabMsgPack(u8 id)
** @Input     : id:ICC收发通信表序号
** @Output    : 发送结果(0:正常,其它:错误码)
** @Function  : 发送ICC收发通信表中需要发送的数据包
** @The notes : ICC收发通信表有报文需要发送时调用
**===============================================================================================*/
u8 ICCSendTxRcvTabMsgPack(u8 id);

/*=================================================================================================
** @Name      : void ICCReadTxRcvTabData(u8 id, u16 readlen, u8 *pdata)
** @Input     : id:ICC收发通信表序号 readlen:读数据长度 *pdata:读到的数据
** @Output    : void
** @Function  : 读取ICC收发通信表中缓存区的数据
** @The notes : 确保读取长度必须有效
**===============================================================================================*/
void ICCReadTxRcvTabData(u8 id, u16 readlen, u8 *pdata);

/*=================================================================================================
** @Name      : u8 ICCWriteCanIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data, u8 iccnum)
** @Input     : *ptrTable:主索引信息  sindex:子索引号  sIndexNum:子索引数目 *Data:需写入的数据(多字节则低位在前) iccnum:核间通道号(暂时没用到)
** @Output    : 写结果(0:成功 其它:错误码)
** @Function  : 向索引表中对应子索引写入数据值
** @The notes : 必须确保主索引存在且子索引长度合法
**===============================================================================================*/
u8 ICCWriteCanIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data, u8 iccnum);

/*=================================================================================================
** @Name      : void ICCReadIccIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data)
** @Input     : *ptrTable:主索引信息  sindex:子索引号  sIndexNum:子索引数目 *Data:读出的数据(多字节则低位在前)
** @Output    : void
** @Function  : 读索引表中对应子索引当前数据值
** @The notes : 必须确保主索引存在且子索引长度合法
**===============================================================================================*/
void ICCReadIccIndexData(const t_ICCIndexTab *ptrTable, u8 subIndex, u8 sIndexNum, u8 *Data);

/*=================================================================================================
** @Name      : u8 ICCFindTxRcvTabMatching(u8 destAddr, e_ICCFunCode funcode, u8 mindex, u8 sindex)
** @Input     : destAddr:源地址 funcode:功能码 mindex:主索引 sindex:子索引
** @Output    : void
** @Function  : 查找核间通信收发通信表中与输入参数匹配的序号
** @The notes : 查找是否正处于收发状态
**===============================================================================================*/
u8 ICCFindTxRcvTabMatching(u8 destAddr, e_ICCFunCode funcode, u8 mindex, u8 sindex);

/*=================================================================================================
** @Name      : void ICCClrTxRcvTabTimer(u8 id)
** @Input     : id:ICC收发通信表序号
** @Output    : void
** @Function  : 清除ICC收发通信表中的计时
** @The notes : 两帧交互正常计时结束
**===============================================================================================*/
void ICCClrTxRcvTabTimer(u8 id);
#endif
