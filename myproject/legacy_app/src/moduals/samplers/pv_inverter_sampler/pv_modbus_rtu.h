#ifndef _PV_MODBUS_RTU_H__
#define _PV_MODBUS_RTU_H__

#include "../sampler_base.h"

//#define BOOL int
//#define TRUE 1
//#define FALSE 0
#define NULL ((void *)0)

#define     MAX_RECEIVE_BUF_LEN             1024
#define		MAX_MODBUSFRAME_LEN				256
#define 	MODBUS_LOG_TEXT_LEN				256

#define 	MODBUS_READCOM_TIMEOUT			1000
#define 	MODBUS_WRITECOM_TIMEOUT			1000
 
#define		MODBUS_CMD_NUM					4
#define		MODBUS_READDISCRETE_CMD			0x02
#define		MODBUS_GETDATA_CMD				0x03
#define		MODBUS_GETDATA_CMD_04			0x04

#define		MODBUS_SETSINGLEDATA_CMD		0x06
#define		MODBUS_SETMULTIDATA_CMD			0x10

#define		MODBUS_READ_CMD_NUM				1

#define		MAX_MODBUS_SIGNAL_NUM			65536

#define 	MODBUS_RTU_MIN_LEN              5      
#define 	MODBUS_RTU_MAX_LEN              256    
#define		MODBUS_RTU_SEND_COMMAND_LEN		8         

#define 	MODBUS_CRC16_POLY               0xA001
#define 	MODBUS_CRC16_INIT               0xFFFF

#define     MAX_WRITE_NUM                   125


typedef BOOL (*MODBUSRTU_PARSE_PROC)(int iStartAddr, BYTE*   pDataIn, void*   pDataOut); //解析数据回调函数

typedef struct  __MODBUS_REQUEST_INFO   //请求数据结构体信息
{
    BYTE byFunCode;     //读功能码
    USHORT usRegAddr;   //读寄存器起始地址
    USHORT usRegNum;    //读寄存器数量

    MODBUSRTU_PARSE_PROC fnParseModbusRtuData;  //解析数据的回调函数
    
}MODBUS_REQUEST_INFO;

typedef struct __MODBUS_WRITE_SIGNAL_REGISTER   //写单个寄存器结构体信息
{   
    USHORT usRegAddr;   //写寄存器地址
    USHORT usValue;     //写入的值

}MODBUS_WRITE_SIGNAL_REGISTER;

typedef struct __MODBUS_WRITE_MULTI_REGISTER    //写多个寄存器结构体信息
{
    USHORT usRegAddr;      //写寄存器起始地址
    USHORT usWriteNum;      //写寄存器数量
    USHORT usValue[MAX_WRITE_NUM];  //写入数据的值

}MODBUS_WRITE_MULTI_REGISTER;

unsigned short getTwoBytes(int iRegAddr, int iBufStartAddr, BYTE *pBuf);
unsigned int getFourBytes(int iRegAddr, int iBufStartAddr, BYTE *pBuf);


BOOL sampleModbusRtuData(int iSlaveAddr, MODBUS_REQUEST_INFO* pReqInfo, COMMPORT* pCOMPort, HANDLE hPort, void* pOutData);

BOOL writeModbusRtuSignalRegister(int iSlaveAddr, USHORT usRegAddr, SHORT usWriteVal, COMMPORT* pCOMPort, HANDLE hPort);
BOOL writeModbusRtuMultipleRegister(int iSlaveAddr, USHORT usRegAddr, USHORT iWriteRegNum, SHORT* usValue, COMMPORT* pCOMPort, HANDLE hPort);

#endif