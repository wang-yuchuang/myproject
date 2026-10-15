/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : SlaveMsg.c
** @Founder      : ZQD808
** @Date         : 2025.01.07
** @Function     : 从控CAN报文处理模块
**===============================================================================================*/
#include "SlaveMsg.h"
#include "BSPUserHook.h"
#include "BSPConfig.h"
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "DEVCAN.h"
#include "CLI_type.h"
#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>
#include <debug.h>
#include "CLI.h"
#include "CellAFESamp.h"
#include "DiagInterface.h"
#include "BSPUserAPI.h"
#include "DiagFilter.h"
#include "DiagCheck.h"
#include "AuxDevMsg.h"
#include "BalanTactic.h"
#include "IccMsgHandle.h"
#include "IccMsgVerSrl.h"

extern u16 *gDevInfo_2;         /*设备信息*/
//电芯相关信息
extern u16 *gGCellVoltInfo_1;   /*组1电池单体电压信息*/
extern u16 *gGCellVoltInfo_2;   /*组2电池单体电压信息*/
extern s16 *gGCellTempInfo_1;   /*组1电池单体温度信息*/
extern s16 *gGCellTempInfo_2;   /*组2电池单体温度信息*/
extern u16 *gGCellResisInfo_1;  /*组1电池单体内阻信息*/
extern u16 *gGCellResisInfo_2;  /*组2电池单体内阻信息*/
extern u16 *gGChgBalanInfo_1;    /*组1充电均衡状态信息*/
extern u16 *gGChgBalanInfo_2;    /*组2充电均衡状态信息*/
extern u16 *gGDhgBalanInfo_1;    /*组1放电均衡状态信息*/
extern u16 *gGDhgBalanInfo_2;    /*组2放电均衡状态信息*/
extern u8 *gGAfeStateInfo_1;     /*组1AFE状态信息*/
extern u8 *gGAfeStateInfo_2;     /*组2AFE状态信息*/
extern s16 *gSysCharInfo_54;     /*系统特征信息*/

extern u16 gGBmuGenPara_102[];  			 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 			 /*主控高级参数值*/
extern u8 gGHardPara_104[];                  /*主控硬件参数配置信息*/

/*=================================================================================================
** @BSU版本号/序列号/条码查询缓存与回复标志(供ICC主索引81/82/83/84同步回读)
**===============================================================================================*/
u16 gBsuDevInfo[BSU_DEVINFO_LEN] = {0};                     /*主1 设备信息(u16)*/
u8  gBsuSerial[BSU_SERIAL_LEN] = {0};                       /*主230 单板序列号*/
u8  gBsuPackSerial[BSU_PACKSERIAL_LEN] = {0};               /*主231 PACK序列号*/
u8  gBsuCellSerial[BSU_CELLSERIAL_LEN] = {0};               /*主232 电芯序列号*/
volatile u8 gBsuDevInfoReplyFlag = 0;                       /*主1 查询回复标志*/
volatile u8 gBsuSerialReplyFlag = 0;                        /*主230 查询回复标志*/
volatile u8 gBsuPackSerialReplyFlag = 0;                    /*主231 查询/写入回复标志*/
volatile u8 gBsuCellSerialReplyFlag = 0;                    /*主232 查询回复标志*/
volatile u8 gBsuAuthReplyFlag = 0;                          /*主233 权限写入回复标志*/

/*=================================================================================================
** @Static function declaration
**===============================================================================================*/
static void RcvBsuDevInfo(u8 ecu, u8 *pdata, u8 len);
static void RcvBsuSerial(u8 ecu, u8 *pdata, u8 len);
static void RcvBsuPackSerial(u8 ecu, u8 *pdata, u8 len);
static void RcvBsuCellSerial(u8 ecu, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void SendCanSlaveBalanCtrlMsg86(u16 pack, u16 dhgBatt, u16 chgBatt)
** @Input     : pack:从控序号[0,31],0xff:所有电池箱  dhgBatt:放电均衡电池:每bit为1节电池  chgBatt:充电均衡电池:每bit为1节电池
** @Output    : void
** @Function  : 发送请求从控均衡控制命令CAN报文(按单个电池箱[16S]控制)
** @The notes : 50ms周期
**===============================================================================================*/
static void SendCanSlaveBalanCtrlMsg86(u16 pack, u64 dhgState64);

/*=================================================================================================
** @Name      : static void RcvSlaveCellVoltInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] cell:起始电池号[0,399] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体电压信息
** @The notes : 一个从机输多个电池箱信息(要求每个从机Pack电压数目必须相等)
**===============================================================================================*/
static void RcvSlaveCellVoltInfo(u8 ecu, u8 cell, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveCellTempInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] cell:起始温度号[0,399] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体温度信息(包括均衡板温)
** @The notes : 一个从机传输多个电池箱信息(要求每个从机Pack温度数目必须相等)
**===============================================================================================*/
static void RcvSlaveCellTempInfo(u8 ecu, u8 cell, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveChgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] index:起始索引号[0,24] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体充电均衡状态信息
** @The notes : 一个从机传输多个电池箱信息(要求每个从机Pack电池数目必须相等)
**===============================================================================================*/
static void RcvSlaveChgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveDhgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] index:起始索引号[0,24] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体放电均衡状态信息
** @The notes : 一个从机传输多个电池箱信息(要求每个从机Pack电池数目必须相等)
**===============================================================================================*/
static void RcvSlaveDhgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,399] pdata:接收的报文数据域 len:数据长度[0,64]
** @Output    : void
** @Function  : 接收从机单体内阻信息(u16,1μΩ)
** @The notes : 一个从机传输多个电池箱信息,索引[90,129],每帧32个u16
**===============================================================================================*/
static void RcvSlaveCellResisInfo(u8 ecu, u8 cell, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisRealInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,48] pdata:接收的报文数据域 len:数据长度[0,64]
** @Output    : void
** @Function  : 接收从机单体内阻实部信息(u32,1μΩ), 存gBsuCellResisReal后填ICC28-31
** @The notes : 索引[50,53],每帧16个u32
**===============================================================================================*/
static void RcvSlaveCellResisRealInfo(u8 ecu, u8 cell, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisImagInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,48] pdata:接收的报文数据域 len:数据长度[0,64]
** @Output    : void
** @Function  : 接收从机单体内阻虚部信息(u32,1μΩ), 存gBsuCellResisImag后填ICC32-35
** @The notes : 索引[90,93],每帧16个u32
**===============================================================================================*/
static void RcvSlaveCellResisImagInfo(u8 ecu, u8 cell, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveAfeStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) index:AFE帧序号[0,10] pdata:接收的报文数据域 len:数据长度
** @Output    : void
** @Function  : 接收从机AFE状态信息(每4bit代表1个AFE IC状态)
** @The notes : 索引[130,140],拆包后每个AFE IC存1个u8,状态值定义见e_AfeState
**===============================================================================================*/
static void RcvSlaveAfeStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void SendCanSlaveBalanCtrlMsg86(u16 pack, u16 dhgBatt, u16 chgBatt)
** @Input     : pack:从控序号[0,31],0xff:所有电池箱  dhgBatt:放电均衡电池:每bit为1节电池  chgBatt:充电均衡电池:每bit为1节电池
** @Output    : void
** @Function  : 发送请求从控均衡控制命令CAN报文(按单个电池箱[16S]控制)
** @The notes : 50ms周期
**===============================================================================================*/
static void SendCanSlaveBalanCtrlMsg86(u16 pack, u64 dhgState64);

/*=================================================================================================
** @Name      : static void RcvSlaveAllInOneInfo65(u8 ecu, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15)  pdata:CAN FD数据域指针  len:数据长度
** @Output    : void
** @Function  : 解析从机综合信息 (CAN FD, Index 65) 并分簇存储
** @The notes : 包含环境(PT1000/水浸)、安全(互锁/FG)、气溶胶及AFE等全量状态数据
**===============================================================================================*/
static void RcvSlaveAllInOneInfo65(u8 ecu, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void RcvSlaveParaInfo102(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] index:起始索引号[0,3] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机参数信息
**===============================================================================================*/
static void RcvSlaveParaInfo102(u8 ecu, u8 index, u8 *pdata, u8 len);

/*=================================================================================================
** @Name      : static void SendCanSlaveParaSetMsg107(u8 slave, u8 index, u16 para)
** @Input     : slave:从控序号[0,9],0xff:所有从控 index:参数子索引号[0,10] para:目标参数值
** @Output    : void
** @Function  : 发送请求从控参数设置命令CAN报文(按单个从控[通信板]控制)
** @The notes : 50ms周期
**===============================================================================================*/
static void SendCanSlaveParaSetMsg107(u8 slave, u8 index, u16 para);

/*=================================================================================================
** @Interface function definition
**===============================================================================================*/
/*=================================================================================================
** @Name      : void GroupCheckRcvBsuMsgAPI(void)
** @Input     : void
** @Output    : void
** @Function  : 主控判断从控信息采集完成任务
** @The notes : 用户50ms周期接口调用
**===============================================================================================*/
void GroupCheckRcvBsuMsgAPI(void)
{
    static u8 FirstPowerOnFlag = 0;
    static u16 sPack = 0; 
    
    static u16 sBalanTime[CLUTER_MAX_NUM] = {0};
    static u32 sIdleTime[CLUTER_MAX_NUM] = {0xfffffff0, 0xfffffff0};

    u16 bsuNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; // 单簇 PACK 数 
    u16 clusterNum = gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; // 簇数 
    u16 totalPackNum = bsuNum * clusterNum;

    if (totalPackNum == 0 || bsuNum == 0) return;

    //上电前5s等待从控信息
    if((BSPGetOSMillSecTimeAPI() < 5000) && (FirstPowerOnFlag == 0)) return;
    FirstPowerOnFlag = 1;

    // 实时算出当前轮询到的 PACK 属于哪一簇
    u8 curClu = (u8)(sPack / bsuNum);
    if(curClu >= CLUTER_MAX_NUM) { sPack = 0; return; }

    //系统空闲
    if(eCURR_IDLE == GetGChgDhgStateAPI(curClu)) 
    {
        if(sIdleTime[curClu] < 0xfffffff0) sIdleTime[curClu]++;
    } 
    else 
    {
        sIdleTime[curClu] = 0;
    }

    //*正常模式下采样失效和内部CAN断线故障*/
    if((eMODE_DEBUG != GetGWorkModeAPI(curClu))
        && ((eErr_B == DiagGetErrFilStateAPI(curClu, eERR_ID_SAMP_EXP))
        || (eErr_B == DiagGetErrFilStateAPI(curClu, eERR_ID_SAMP_VOFF))
        || (eErr_B == DiagGetErrFilStateAPI(curClu, eERR_ID_CELL_MISS))
        || (eErr_B == DiagGetErrFilStateAPI(curClu, eERR_ID_INT_COFF))))
    {
        //异常计时减慢发送周期
        sBalanTime[curClu]++;
        //先广播10次,然后1轮单个控制广播1次
        if((sBalanTime[curClu] <= 10) || (sBalanTime[curClu] == 120))
        {
            /*广播关闭所有均衡*/
            SendCanSlaveBalanCtrlMsg86(0xFFFF, 0ULL); // 放电全关
           // SendCanSlaveChgBalanCtrlMsg87(0xff, 0ULL); // 充电全关
            
            sPack = curClu * bsuNum; 
        }
        //逐个Pack关闭(防止从控不接收广播命令)
        else if((sBalanTime[curClu] <= 100) || ((sBalanTime[curClu] > 120) && (0 == ((sBalanTime[curClu] - 120) % 20))))
        {
            /*逐个向从控发送均衡命令*/
            u64 dhgCmd = GetGroupVoltPasBalanReqState(sPack);
         //   u64 chgCmd = GetGroupVoltPasChgBalanReqState(sPack); // 充电

            SendCanSlaveBalanCtrlMsg86(sPack, dhgCmd);
          //  SendCanSlaveChgBalanCtrlMsg87(sPack, chgCmd);

            sPack++;
            if(sPack >= totalPackNum) sPack = 0;
            if(sBalanTime[curClu] > 120) sBalanTime[curClu] = 119;
        }
        else 
        {
            sPack++;
            if(sPack >= totalPackNum) sPack = 0;
        }
    }
    /*非上电初始化且从控和单体电压正常*/
    else if((eWORK_INIT != GetGWorkStateAPI(curClu)) && (eWORK_OFF != GetGWorkStateAPI(curClu)))
    {
        /*逐个Pack向从控发送均衡命令*/
        u64 dhgCmd = GetGroupVoltPasBalanReqState(sPack);
      //  u64 chgCmd = GetGroupVoltPasChgBalanReqState(sPack);

        SendCanSlaveBalanCtrlMsg86(sPack, dhgCmd);
      //  SendCanSlaveChgBalanCtrlMsg87(sPack, chgCmd);

        sPack++;
        if(sPack >= totalPackNum) sPack = 0;
        sBalanTime[curClu] = 0; // 恢复正常，计时器清零
    }
    else 
    {
        sPack++;
        if(sPack >= totalPackNum) sPack = 0;
        sBalanTime[curClu] = 0;
    }


   if(eErr_A == DiagGetErrFilStateAPI(curClu, eERR_ID_INT_COFF))
    {
        u8 curSlave = sPack % bsuNum; // 当前扫描到的簇内从机号
        
        /* 参数同步：检测硬件参数、高级参数与底层 gGBsuGenPara_102 是否一致 */
        if((1 == DiagGetCellVoltExpStateHOOK(curClu)) || (1 == DiagGetCellTempExpStateHOOK(curClu)) || 
           (BSPGetOSMillSecTimeAPI() <= 60000 && FirstPowerOnFlag == 0))
        {
            /* 计算当前簇应该有的单机 PACK 内部配置 */
            u16 cfgValue = bsuNum / gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];

            /*  比对从控类型 (由主控软件版本最高位定义，例如 2 代表 DTNXP AFE) */
            //if(gGHardPara_104[eGHardPara104_BmuSWVerHH] != gGBsuGenPara_102[curClu][curSlave][eGBsuPara102_DaisyType])
            {
              //  SendCanSlaveParaSetMsg107(0xff, eGBsuPara102_DaisyType, gGHardPara_104[eGHardPara104_BmuSWVerHH]);
            }
            /*  比对模组数量 (DaisyNum) */
             if(gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] != gGBsuGenPara_102[curClu][curSlave][eGBsuPara102_DaisyNum])
            {
                SendCanSlaveParaSetMsg107(0xff, eGBsuPara102_DaisyNum, gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]);
            }
            /*  比对单模组电压采样节数 */
            else if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] != gGBsuGenPara_102[curClu][curSlave][eGBsuPara102_SampVnum])
            {
                SendCanSlaveParaSetMsg107(0xff, eGBsuPara102_SampVnum, gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]);
            }
            /* 比对单模组温度采样节数 */
            else if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] != gGBsuGenPara_102[curClu][curSlave][eGBsuPara102_SampTnum])
            {
                SendCanSlaveParaSetMsg107(0xff, eGBsuPara102_SampTnum, gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum]);
            }
        }
        
    }

    if((eErr_B == DiagGetErrFilStateAPI(curClu, eERR_ID_INT_COFF)) || ((BSPGetOSMillSecTimeAPI() >= 10000)
        && (eErr_A == DiagGetErrFilStateAPI(curClu, eERR_ID_INT_COFF)) && ((0 == DiagGetFilErrSortAPI(eERR_ID_INT_COFF))
        || (eErr_B == DiagGetErrFilStateAPI(curClu, eERR_ID_CELL_MISS)) || (0 == DiagGetFilErrSortAPI(eERR_ID_CELL_MISS)))))
    {
        WriteCellVSampFinFlagHook(curClu, 1);
        WriteCellTSampFinFlagHook(curClu, 1);
    }
}

/*=================================================================================================
** @Name      : void CanRcvSlaveMsgHandleAPI(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : 自研从控协议CAN报文接收处理
** @The notes : CAN接收回调函数调用
**===============================================================================================*/
void CanRcvSlaveMsgHandleAPI(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
{
	u8 rcvDev = 0;
	u8 aimDev = 0;
	u8 rcvFun = 0;
	u8 rcvIndex = 0;
	u16 subIndex = 0;

	rcvDev = ID & 0xff;           
	aimDev = (ID >> 8) & 0xff;     
	rcvIndex = (ID >> 16) & 0xff;  
	rcvFun = (ID >> 25) & 0x0f; 

    /*空指针判断*/
    if(gDevInfo_2 == NULL)
    {
        return;
    }
	/*空指针判断*/
	if((gGCellVoltInfo_1 == NULL) || (gGCellVoltInfo_2 == NULL) || (gGCellTempInfo_1 == NULL) || (gGCellTempInfo_2 == NULL)
	|| (gGCellResisInfo_1 == NULL) || (gGCellResisInfo_2 == NULL) || (gGChgBalanInfo_1 == NULL) || (gGChgBalanInfo_2 == NULL)
    || (gGDhgBalanInfo_1 == NULL) || (gGDhgBalanInfo_2 == NULL))
	{
		return;
	}
    /*只接收从控报文*/
	if((eCAN1 != canNum) || (0 == DLC) || (rcvDev > BSU02_BAS_MAXADDR)
		|| ((0xff != aimDev) && (gDevInfo_2[eICCDev2_Addr] != aimDev)) || ((rcvFun != 3)
		&& (rcvFun != 6) && (rcvFun != 8) && (rcvFun != 9) && (rcvFun != 10) && (rcvFun != 11) && (rcvFun != 12)))
	{
		return;
	}
	/*记录能接收到报文通信正常*/
	DiagRcvDevMsgAddAPI(rcvDev);
	//单体信息上传
	if(11 == rcvFun)
	{
		/*BSU单体电压信息报文，索引10-23表示最多416个电芯电压*/
		if((rcvIndex >= 10) && (rcvIndex < 23))
		{
			subIndex = (rcvIndex - 10) * 32;
			RcvSlaveCellVoltInfo(rcvDev, subIndex, pdata, DLC);
		}
		/*BSU单体温度信息报文，索引30-43表示最多416个电芯温度*/
		if((rcvIndex >= 30) && (rcvIndex < 43))
		{
			subIndex = (rcvIndex - 30) * 32;
			RcvSlaveCellTempInfo(rcvDev, subIndex, pdata, DLC);
		}
		/*BSU充电均衡状态信息报文*/
		else if(rcvIndex < 2)
		{
			subIndex = rcvIndex * 4;
			RcvSlaveChgBalanStateInfo(rcvDev, subIndex, pdata, DLC);
		}
		/*BSU放电均衡状态信息报文*/
		else if((rcvIndex >= 2) && (rcvIndex < 10))
		{
			subIndex = (rcvIndex - 2) * 4;
			RcvSlaveDhgBalanStateInfo(rcvDev, subIndex, pdata, DLC);
		}
		/*BSU单体内阻信息报文,索引[10,29]→协议index[90,129]映射(偏移80)*/
		else if((rcvIndex >= 10) && (rcvIndex < 30))
		{
			subIndex = (rcvIndex - 10) * 32;
			RcvSlaveCellResisInfo(rcvDev, subIndex, pdata, DLC);
		}
		/*BSU AFE状态信息报文,索引[30,40]→协议index[130,140]映射*/
		else if((rcvIndex >= 30) && (rcvIndex < 41))
		{
			RcvSlaveAfeStateInfo(rcvDev, rcvIndex - 30, pdata, DLC);
		}
		/*BSU单体内阻实部报文(u32): 每从机52颗分4帧(索引50-53), 每帧16颗, 功能码11 data0起直接是数据*/
		else if((rcvIndex >= 50) && (rcvIndex < 54))
		{
			subIndex = (rcvIndex - 50) * 16;    /*起始电芯号=(索引-50)×16*/
			RcvSlaveCellResisRealInfo(rcvDev, subIndex, pdata, DLC);
		}
		/*BSU单体内阻虚部报文(u32): 每从机52颗分4帧(索引90-93), 每帧16颗*/
		else if((rcvIndex >= 90) && (rcvIndex < 94))
		{
			subIndex = (rcvIndex - 90) * 16;    /*起始电芯号=(索引-90)×16*/
			RcvSlaveCellResisImagInfo(rcvDev, subIndex, pdata, DLC);
		}
	}
    else if(10 == rcvFun)
	{
		/*pack环境信息报文 (包含PT1000、水浸、互锁、气溶胶、AFE等)*/
		if(65 == rcvIndex)
		{
			RcvSlaveAllInOneInfo65(rcvDev, pdata, DLC);
		}

	}
    else if(12 == rcvFun)
	{
		/*BSU常规参数信息报文*/
		if((rcvIndex >= 50) && (rcvIndex < 75))
		{
			subIndex = (rcvIndex - 50) * 4;

			RcvSlaveParaInfo102(rcvDev, subIndex, pdata, DLC);
		}
	}
    else if(6 == rcvFun)
    {
        /*BSU查询回复报文(版本号/序列号/条码)*/
        if(BSU_MINDEX_DEVINFO == rcvIndex)
        {
            RcvBsuDevInfo(rcvDev, pdata, DLC);
        }
        else if(BSU_MINDEX_SERIAL == rcvIndex)
        {
            RcvBsuSerial(rcvDev, pdata, DLC);
        }
        else if(BSU_MINDEX_PACKSERIAL == rcvIndex)
        {
            RcvBsuPackSerial(rcvDev, pdata, DLC);
        }
        else if(BSU_MINDEX_CELLSERIAL == rcvIndex)
        {
            RcvBsuCellSerial(rcvDev, pdata, DLC);
        }
    }
    else if(3 == rcvFun)
    {
        /*BSU修改回复报文(PACK序列号写入/权限写入)*/
        if(BSU_MINDEX_PACKSERIAL == rcvIndex)
        {
            gBsuPackSerialReplyFlag = 1;
            ICCVerSrlReplyNotify(ICC_DEVTYPE_BSU, VSRL_EVT_PACKSERIAL_WR);
        }
        else if(BSU_MINDEX_AUTH == rcvIndex)
        {
            gBsuAuthReplyFlag = 1;
            ICCVerSrlReplyNotify(ICC_DEVTYPE_BSU, VSRL_EVT_AUTH);
        }
    }
}

/*=================================================================================================
** @Name      : static void RcvSlaveCellVoltInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] cell:起始电池号[0,399] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体电压信息
** @The notes : 一个从机输多个电池箱信息(要求每个从机Pack电压数目必须相等)
**===============================================================================================*/
static void RcvSlaveCellVoltInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
{
	u8 i = 0;
	u8 packId = 0;
	u8 cellId = 0;
	u16 cellVolt = 0;
	static u8 sRcvCellNum[CLUTER_MAX_NUM] = {0};
	static u32 sRcvCellBit[CLUTER_MAX_NUM] = {0};
	u8 CluterNum = 0; //第几簇
	u16 BsuCellMaxNum = 0; //单个从控(PACK)最大电芯节数

    /*参数错误*/
	if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) || (len < 2) || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
		|| (cell >= (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])))
	{
		return;
	}
	if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0)
	{
		CluterNum = ecu / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
	}

	BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];

	//u16每帧1~32节(根据帧长度判断)
	for(i = 0; i < (len / 2); i++)
	{
		/* 拦截多余的无效填充数据，防止后续取模运算将前段有效数据覆盖清零 */
		if ((cell + i) >= BsuCellMaxNum) 
        {
            break; 
        }

		if((cell + i) < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
		{
			if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0)
			{
				//转化为PACK号和PACK内电池号
				packId = ecu % gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
				cellId = (cell + i) % BsuCellMaxNum;
				/*Pack号和Pack内电池号正确正确*/
				if((packId < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]) && (cellId < BsuCellMaxNum))
				{
					//获取电压
					cellVolt = ((u16)pdata[1 + (i * 2)] << 8) | pdata[i * 2];

					/*采样电压值判断断线*/
					DiagCheckCellVOffLFlagAPI(CluterNum, packId, cellId, cellVolt);

					/*输出采样电压值*/
					WriteSampCellVoltHook(CluterNum,packId, cellId, cellVolt);

					//首次接收到该从控信息
					if(0 == BitGet(sRcvCellBit[CluterNum], packId))
					{
						//计数接收到从控数目
						sRcvCellNum[CluterNum]++;
					}
					//记录该从控已接收
					BitSet(sRcvCellBit[CluterNum], packId);
					//判断接收完成
					if((sRcvCellNum[CluterNum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
						&& (2 == DiagGetRcvAllCellFlagAPI(CluterNum)) 
						)
					{
						//记录单体电压采样完成一遍
						WriteCellVSampFinFlagHook(CluterNum,1);
					}
				}
				/*Pack号或Pack内电池号越界*/
				else
				{
					break;
				}
			}
		}
		/*组内电池号越界*/
		else
		{
			break;
		}
	}
}
/*=================================================================================================
** @Name      : static void RcvSlaveCellTempInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] cell:起始温度号[0,399] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体温度信息(包括均衡板温)
** @The notes : 一个从机传输多个电池箱信息(要求每个从机Pack温度数目必须相等)
**===============================================================================================*/
static void RcvSlaveCellTempInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
{
	u8 i = 0;
	u8 packId = 0;
	u8 cellId = 0;
	s16 cellTemp = 0;
	u8 slaveCelTNum = 0;//一个从机电池温度个数
	u8 slaveAllTNum = 0;//一个从机所有温度个数
	static u8 sRcvCellNum[CLUTER_MAX_NUM] = {0};
	static u32 sRcvCellBit[CLUTER_MAX_NUM] = {0};
	u8 CluterNum = 0; //第几簇
	u16 BsuCellMaxNum = 0; //单个从控(PACK)最大电芯节数

	if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) || (len < 2) || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
		|| (cell >= (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])))
	{
		return;
	}
	if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0)
	{
		CluterNum = ecu / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
	}

	BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];
	
	//u16每帧1~4节(根据帧长度判断)
	for(i = 0; i < (len / 2); i++)
	{
		/* 拦截多余的无效填充数据，防止后续取模运算将前段有效数据覆盖清零 */
		if ((cell + i) >= BsuCellMaxNum) 
        {
            break; 
        }
		/*单体电池温度*/
		if((cell + i) < gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
		{
			/*转化为电池箱号和箱内电池号*/
			packId = ecu % gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
			cellId = (cell + i) % BsuCellMaxNum;

			/*单体温度序号正确*/
			if((packId < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]) && (cellId < BsuCellMaxNum))
			{
				//获取温度(1位小数0.1℃)
				cellTemp = ((s16)pdata[1 + (i * 2)] << 8) | pdata[i * 2];
				/*采样温度值判断断线*/
				DiagCheckCellTOffLFlagAPI(CluterNum, packId, cellId, cellTemp/10);
				/*输出采样温度值*/
				WriteSampCellTempHook(CluterNum, packId, cellId, cellTemp);

				//首次接收到该从控信息
				if(0 == BitGet(sRcvCellBit[CluterNum], packId))
				{
					//计数接收到从控数目
					sRcvCellNum[CluterNum]++;
				}
				//记录该从控已接收
				BitSet(sRcvCellBit[CluterNum], packId);
				//判断接收完成
				if((sRcvCellNum[CluterNum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
					&& (2 == DiagGetRcvAllCellFlagAPI(CluterNum))
				  )
				{
					//记录单体温度采样完成一遍
					WriteCellTSampFinFlagHook(CluterNum,1);
				}
			}
		}
		/*均衡板温暂不考虑，后续实现*/
	}
}

/*=================================================================================================
** @Name      : static void RcvSlaveChgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] index:起始索引号[0,24] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体充电均衡状态信息
** @The notes : 一个从机传输多个电池箱信息(要求每个从机Pack电池数目必须相等)
**===============================================================================================*/
static void RcvSlaveChgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
{
    u8 i = 0;
    u16 cellId = 0;
    u16 state = 0;
    u16 BsuCellMaxNum = 0;
	u8 CluterNum = 0;
    u8 packId = 0;

    /* 参数错误 */
    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) || (len < 2))
    {
        return;
    }

	if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0) {
        CluterNum = ecu / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
        packId = ecu % gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    }
    else
    {
        return;
    }

    BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]; 
    if(BsuCellMaxNum == 0U)
    {
        return;
    }

    /*将全局ECU号转换为本簇Pack号，组2不能再把组1的Pack数重复计入cellId。*/
    cellId = (u16)packId * BsuCellMaxNum + index * 16;

    // u16每帧1~4节(根据帧长度判断)
    for(i = 0; i < (len / 2); i++)
    {
        u16 currentStartCellInPack = (index + i) * 16;

        if (currentStartCellInPack >= BsuCellMaxNum) 
        {
            break; 
        }

        /* 组内电池号合法校验 */
        if((cellId + (i * 16)) < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        {
            // 获取这 16 节的均衡状态 (16个 bit)
            state = ((u16)pdata[1 + (i * 2)] << 8) | pdata[i * 2];

            if (currentStartCellInPack + 16 > BsuCellMaxNum) 
            {
                u8 validBits = BsuCellMaxNum - currentStartCellInPack; 
                // 生成掩码：4位有效就是 0x000F，高12位强行清零
                u16 mask = (1 << validBits) - 1; 
                state = state & mask; 
            }

            // 输入充电均衡状态
            InputSlaveChgBalanState(CluterNum, (cellId + (i * 16)), state);
        }
        else
        {
            break;
        }
    }
}

/*=================================================================================================
** @Name      : static void RcvSlaveDhgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] index:起始索引号[0,24] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机单体放电均衡状态信息
** @The notes : 一个从机传输多个电池箱信息(要求每个从机Pack电池数目必须相等)
**===============================================================================================*/
static void RcvSlaveDhgBalanStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
{
    u8 i = 0;
    u16 cellId = 0;
    u16 state = 0;
    u16 BsuCellMaxNum = 0;
	u8 CluterNum = 0;
    u8 packId = 0;

    /*参数错误*/
    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) || (len < 2))
    {
        return;
    }

	if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0) {
        CluterNum = ecu / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
        packId = ecu % gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    }
    else
    {
        return;
    }
    
    BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]; 
    if(BsuCellMaxNum == 0U)
    {
        return;
    }

    /*将全局ECU号转换为本簇Pack号，保证组2状态落在组2本地数组。*/
    cellId = (u16)packId * BsuCellMaxNum + index * 16;
    
    // u16每帧1~4节(根据帧长度判断)
    for(i = 0; i < (len / 2); i++)
    {
        u16 currentStartCellInPack = (index + i) * 16;

        if (currentStartCellInPack >= BsuCellMaxNum) 
        {
            break; 
        }

        /*组内电池号合法校验*/
        if((cellId + (i * 16)) < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        {
            //获取均衡状态
            state = ((u16)pdata[1 + (i * 2)] << 8) | pdata[i * 2];

            /* 边界掩码保护：丢弃无效的高位补0 */
            if (currentStartCellInPack + 16 > BsuCellMaxNum) 
            {
                u8 validBits = BsuCellMaxNum - currentStartCellInPack; 
                u16 mask = (1 << validBits) - 1; 
                state = state & mask; 
            }

            //输入放电均衡状态
            InputSlaveDhgBalanState(CluterNum, (cellId + (i * 16)), state);
        }
        else
        {
            break;
        }
    }
}

/*=================================================================================================
** @Name      : static void SendCanSlaveBalanCtrlMsg86(u16 pack, u16 dhgBatt, u16 chgBatt)
** @Input     : pack:从控序号[0,31],0xff:所有电池箱  dhgBatt:放电均衡电池:每bit为1节电池  chgBatt:充电均衡电池:每bit为1节电池
** @Output    : void
** @Function  : 发送请求从控均衡控制命令CAN报文(按单个电池箱[16S]控制)
** @The notes : 50ms周期
**===============================================================================================*/
static void SendCanSlaveBalanCtrlMsg86(u16 pack, u64 dhgState64)
{
    can_frame_t msg;
    u16 targetPackAddr = pack;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    u16 celW0, celW1, celW2, celW3;

    if (0xFFFF == pack)
    {
        targetPackAddr = 0xff;
    }
    else
    {
        u16 bsuNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
        u16 clusterNum = gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
        targetPackAddr += GetGroupOffsetDevAddrAPI() * (bsuNum * clusterNum);
    }

    msg.id = CanBuildRupAuxMsgID(2, 86, (u8)targetPackAddr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd = 1;
    msg.len = 14;                                              /*3字节帧头 + 10字节子索引数据(sub0-sub4)*/

    celW0 = (u16)(dhgState64 & 0xFFFF);                       /*sub1: 1-16号电池 bit[15:0] u16*/
    celW1 = (u16)((dhgState64 >> 16) & 0xFFFF);               /*sub2: 17-32号电池 bit[31:16] u16*/
    celW2 = (u16)((dhgState64 >> 32) & 0xFFFF);               /*sub3: 33-48号电池 bit[47:32] u16*/
    celW3 = (u16)((dhgState64 >> 48) & 0xFFFF);               /*sub4: 49-64号电池 bit[63:48] u16*/

    msg.data[0] = 0x0C;                                       /*SOF: 长帧仅单帧, 无确认通信*/
    msg.data[1] = 0x05;                                           /*SDLC: 子索引数目 sub0-sub4共5个*/
    msg.data[2] = 0x00;                                           /*SPF: 起始子索引 从sub0开始*/
    msg.data[3] = 0x00;                                           /*预留*/


    msg.data[4] = (u8)(pack & 0xFF);                           /*sub0: Pack序号 低字节 u16, 广播0xFF*/
    msg.data[5] = (u8)((pack >> 8) & 0xFF);                   /*sub0: Pack序号 高字节 u16*/

    msg.data[6] = (u8)(celW0 & 0xFF);                          /*sub1: 1-16号电池 低字节*/
    msg.data[7] = (u8)((celW0 >> 8) & 0xFF);                  /*sub1: 1-16号电池 高字节*/
    msg.data[8] = (u8)(celW1 & 0xFF);                          /*sub2: 17-32号电池 低字节*/
    msg.data[9] = (u8)((celW1 >> 8) & 0xFF);                  /*sub2: 17-32号电池 高字节*/
    msg.data[10] = (u8)(celW2 & 0xFF);                          /*sub3: 33-48号电池 低字节*/
    msg.data[11] = (u8)((celW2 >> 8) & 0xFF);                 /*sub3: 33-48号电池 高字节*/
    msg.data[12] = (u8)(celW3 & 0xFF);                         /*sub4: 49-64号电池 低字节*/
    msg.data[13] = (u8)((celW3 >> 8) & 0xFF);                 /*sub4: 49-64号电池 高字节*/

    (void)BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : static void SendCanSlaveParaSetMsg107(u8 slave, u8 index, u16 para)
** @Input     : slave:从控序号[0,9],0xff:所有从控 index:参数子索引号[0,10] para:目标参数值
** @Output    : void
** @Function  : 发送请求从控参数设置命令CAN报文(按单个从控[通信板]控制)
** @The notes : 50ms周期
**===============================================================================================*/
static void SendCanSlaveParaSetMsg107(u8 slave, u8 index, u16 para)
{
    can_frame_t msg;
    u16 targetPackAddr = (slave == 0xff) ? 0xff : slave;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
  //  printf("[TX ParaSet] Slave: %d, Index: %d, Value: %d (0x%04X)\r\n", slave, index, para, para);
    msg.id = CanBuildRupAuxMsgID(2, 107, (u8)targetPackAddr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd = 1;
    msg.len = 8;

    msg.data[0] = 0xc1;
    msg.data[1] = index;
    msg.data[2] = para & 0xff;
    msg.data[3] = (para >> 8) & 0xff;
    msg.data[4] = 0;
    msg.data[5] = 0;
    msg.data[6] = 0;
    msg.data[7] = 0;

    (void)BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : static void RcvSlaveAllInOneInfo65(u8 ecu, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15)  pdata:CAN FD数据域指针  len:数据长度
** @Output    : void
** @Function  : 解析从机综合信息 (CAN FD, Index 65) 并分簇存储
** @The notes : 将数据按类型安全地拆分到 94(s16温度) 和 95(u16状态) 数组中
**===============================================================================================*/
static void RcvSlaveAllInOneInfo65(u8 ecu, u8 *pdata, u8 len)
{
    u8 cluNum = 0;
    u8 packId = 0;
    u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

    if (cfgBsuNum == 0 || pdata == NULL || len < 38) 
    {
        return;
    }

    cluNum = ecu / cfgBsuNum;
    packId = ecu % cfgBsuNum;

    /* 防越界 */
    if (cluNum >= CLUTER_MAX_NUM || packId >= MAX_BSU_NUM)
    {
        return;
    }

    gPackTemp_94[cluNum][packId][ePackTemp94_Pt1SelfTemp] = (s16)(((u16)pdata[1] << 8) | pdata[0]);
    gPackTemp_94[cluNum][packId][ePackTemp94_Pt2SelfTemp] = (s16)(((u16)pdata[3] << 8) | pdata[2]);
    gPackTemp_94[cluNum][packId][ePackTemp94_Pt1Temp]     = (s16)(((u16)pdata[13] << 8) | pdata[12]);
    gPackTemp_94[cluNum][packId][ePackTemp94_Pt2Temp]     = (s16)(((u16)pdata[15] << 8) | pdata[14]);

    gPackStat_95[cluNum][packId][ePackStat95_WaterSelfHz1] = ((u16)pdata[5] << 8) | pdata[4];
    gPackStat_95[cluNum][packId][ePackStat95_WaterSelfHz2] = ((u16)pdata[7] << 8) | pdata[6];
    gPackStat_95[cluNum][packId][ePackStat95_Pt5VPullUp]   = ((u16)pdata[9] << 8) | pdata[8];
    gPackStat_95[cluNum][packId][ePackStat95_Out12VCheck]  = ((u16)pdata[11] << 8) | pdata[10];
    
    gPackStat_95[cluNum][packId][ePackStat95_WaterHz1]     = ((u16)pdata[17] << 8) | pdata[16];
    gPackStat_95[cluNum][packId][ePackStat95_WaterHz2]     = ((u16)pdata[19] << 8) | pdata[18];
    
    gPackStat_95[cluNum][packId][ePackStat95_HvilPwmAmp]   = ((u16)pdata[21] << 8) | pdata[20];
    gPackStat_95[cluNum][packId][ePackStat95_HvilPwmDuty]  = ((u16)pdata[23] << 8) | pdata[22];
    gPackStat_95[cluNum][packId][ePackStat95_FgFreq]       = ((u16)pdata[25] << 8) | pdata[24];
    gPackStat_95[cluNum][packId][ePackStat95_FgDuty]       = ((u16)pdata[27] << 8) | pdata[26];
    
    gPackStat_95[cluNum][packId][ePackStat95_AeroConc]     = ((u16)pdata[29] << 8) | pdata[28];
    gPackStat_95[cluNum][packId][ePackStat95_AeroType]     = ((u16)pdata[31] << 8) | pdata[30];
    gPackStat_95[cluNum][packId][ePackStat95_AeroState]    = ((u16)pdata[33] << 8) | pdata[32];
    gPackStat_95[cluNum][packId][ePackStat95_AfeBotCount]  = ((u16)pdata[35] << 8) | pdata[34];
    gPackStat_95[cluNum][packId][ePackStat95_AfeTopCount]  = ((u16)pdata[37] << 8) | pdata[36];

    DiagJudgeBsu02u1TemperErrorAPI(ecu, 0, gPackTemp_94[cluNum][packId][ePackTemp94_Pt1Temp]);
    DiagJudgeBsu02u1TemperErrorAPI(ecu, 1, gPackTemp_94[cluNum][packId][ePackTemp94_Pt2Temp]);
    DiagJudgeBsu02u1FloodErrorAPI(ecu, 0, gPackStat_95[cluNum][packId][ePackStat95_WaterHz1]);
    DiagJudgeBsu02u1FloodErrorAPI(ecu, 1, gPackStat_95[cluNum][packId][ePackStat95_WaterHz2]);
    DiagJudgeBsu02u1MsdErrorAPI(ecu, gPackStat_95[cluNum][packId][ePackStat95_HvilPwmAmp], gPackStat_95[cluNum][packId][ePackStat95_HvilPwmDuty]);
    DiagJudgeBsu02u1AeroSenErrorAPI(ecu, gPackStat_95[cluNum][packId][ePackStat95_AeroConc]);
}

/*=================================================================================================
** @Name      : static void RcvSlaveParaInfo102(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : slave:从机节点号[0, 9] index:起始索引号[0,3] pdata:接收的报文数据域 len:数据长度[0,8]
** @Output    : void
** @Function  : 接收从机参数信息
**===============================================================================================*/
static void RcvSlaveParaInfo102(u8 ecu, u8 index, u8 *pdata, u8 len)
{
    u8 i = 0;
    u16 para = 0;
    u8 cluNum = 0;
    u8 packId = 0;
    u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

    if((gGBsuGenPara_102 == NULL) || (cfgBsuNum == 0))
    {
        return;
    }

    cluNum = ecu / cfgBsuNum;
    packId = ecu % cfgBsuNum;

    if(cluNum >= CLUTER_MAX_NUM || packId >= SLAVE_MAX_NUM)
    {
        return;
    }

    if(cluNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] || packId >= cfgBsuNum)
    {
        return;
    }

    if((pdata == NULL) || (len < 2))
    {
        return;
    }

    u8 paramCount = len / 2;
    // u16每帧1~4个索引(根据帧长度判断)
    for(i = 0; i < paramCount; i++)
    {
        u16 targetIndex = index + i;

        if(targetIndex >= eGBsuPara102_Fin)
        {
            break; // 后续索引肯定也越界，直接退出循环
        }
        // 移位拼接获取参数值 (低字节在前，高字节在后)
        para = ((u16)pdata[1 + (i * 2)] << 8) | pdata[i * 2];

        gGBsuGenPara_102[cluNum][packId][targetIndex] = para;
    }
}

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,399] pdata:接收的报文数据域 len:数据长度[0,64]
** @Output    : void
** @Function  : 接收从机单体内阻信息(u16,1μΩ)
** @The notes : 一个从机传输多个电池箱信息,索引[90,129],每帧32个u16
**===============================================================================================*/
static void RcvSlaveCellResisInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
{
    u8 i = 0;
    u8 packId = 0;
    u8 cellId = 0;
    u16 cellResis = 0;
    static u8 sRcvCellNum[CLUTER_MAX_NUM] = {0};
    static u32 sRcvCellBit[CLUTER_MAX_NUM] = {0};
    u8 CluterNum = 0;
    u16 BsuCellMaxNum = 0;

    if((ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum])
        || (len < 2) || (cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        || (cell >= (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])))
    {
        return;
    }

    if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0)
    {
        CluterNum = ecu / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    }

    BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];

    for(i = 0; i < (len / 2); i++)
    {
        if((cell + i) >= BsuCellMaxNum)
        {
            break;
        }

        if((cell + i) < gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
        {
            if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] != 0)
            {
                packId = ecu % gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
                cellId = (cell + i) % BsuCellMaxNum;

                if((packId < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]) && (cellId < BsuCellMaxNum))
                {
                    cellResis = ((u16)pdata[1 + (i * 2)] << 8) | pdata[i * 2];

                    WriteSampCellResisHook(CluterNum, packId, cellId, cellResis);

                    if(0 == BitGet(sRcvCellBit[CluterNum], packId))
                    {
                        sRcvCellNum[CluterNum]++;
                    }
                    BitSet(sRcvCellBit[CluterNum], packId);
                    if((sRcvCellNum[CluterNum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
                        && (2 == DiagGetRcvAllCellFlagAPI(CluterNum)))
                    {
                        WriteCellRSampFinFlagHook(CluterNum, 1);
                    }
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            break;
        }
    }
}

/*=================================================================================================
** @Name      : static void InsuResFill(void)
** @Input     : void
** @Output    : void
** @Function  : 填ICC28-31内阻实部与ICC32-35内阻虚部缓冲(时间+频点+内阻), A核查询时框架自动回ARQ(功能码06)
**===============================================================================================*/
static void InsuResFill(void)
{
    t_SysClock clock = gSysClock;
    u16 cellNum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];  /*总电芯数(832=52颗×16BSU)*/
    u32 hdrDate;
    u32 hdrTime;
    u32 hdrFreq;
    u8 grp;
    u16 i;

    if((gIccCellResisReal == NULL) || (gIccCellResisImag == NULL) ||
       (gBsuCellResisReal == NULL) || (gBsuCellResisImag == NULL))
    {
        return;
    }

    /*头部3个字索引(每个主索引都带): [0]=年月日 [1]=时分秒 [2]=检测频点(来自ICC85下发)*/
    hdrDate = ((u32)clock.year << 16) | ((u32)clock.month << 8) | (u32)clock.day;
    hdrTime = ((u32)clock.hour << 24) | ((u32)clock.minute << 16) | (u32)clock.second;
    hdrFreq = (u32)gIccInsuFreqIdx;

    /*832颗电芯拆4组每组208颗:
    **   28组→电芯1-208   29组→209-416   30组→417-624   31组→625-832
    ** 每组主索引布局 = 3头 + 208颗 = 211个子索引(实部28-31与虚部32-35结构一致)*/
    for(grp = 0; grp < ICC_RESIS_GRP_NUM; grp++)
    {
        u32 *pReal = gIccCellResisReal + grp * ICC_MINDEX28_IDX_LEN;  /*实部ICC第grp组首地址*/
        u32 *pImag = gIccCellResisImag + grp * ICC_MINDEX28_IDX_LEN;  /*虚部ICC第grp组首地址*/
        u16 base = (u16)(grp * ICC_MINDEX28_CELL_NUM);                /*本组对应全局电芯起始号*/

        pReal[0] = hdrDate;
        pReal[1] = hdrTime;
        pReal[2] = hdrFreq;
        pImag[0] = hdrDate;
        pImag[1] = hdrTime;
        pImag[2] = hdrFreq;

        /*把BSU原始缓冲(全局电芯0-831)按组搬进ICC缓冲的数据区[3..210]*/
        for(i = 0; i < ICC_MINDEX28_CELL_NUM; i++)
        {
            u16 src = (u16)(base + i);
            if(src < cellNum)
            {
                pReal[3 + i] = gBsuCellResisReal[src];
                pImag[3 + i] = gBsuCellResisImag[src];
            }
            else
            {
                /*总电芯数不足208整数倍时尾部补0*/
                pReal[3 + i] = 0;
                pImag[3 + i] = 0;
            }
        }
    }
}

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisInfoStore(u8 ecu, u8 cell, u8 *pdata, u8 len, u32 *pBuf)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,48] pdata:接收的报文数据域 len:数据长度[0,64] pBuf:目标缓冲
** @Output    : void
** @Function  : 接收从机单体内阻信息(u32,1μΩ)并按全局电芯索引存入pBuf
**===============================================================================================*/
static void RcvSlaveCellResisInfoStore(u8 ecu, u8 cell, u8 *pdata, u8 len, u32 *pBuf)
{
    u8 i;
    u16 CluterNum;      /*ecu所在簇号*/
    u16 packId;         /*ecu在簇内的从机号*/
    u16 cellId;         /*从机内电芯号*/
    u16 BsuCellMaxNum;  /*每从机电芯数(52=串联数×并联数)*/
    u16 bsuNum;         /*单簇从机数(16)*/
    u16 vSumNum;        /*总电芯数(832)*/

    if((pdata == NULL) || (pBuf == NULL) || (len < 4))
    {
        return;
    }

    bsuNum = (u16)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    if(0 == bsuNum)
    {
        return;
    }
    vSumNum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];
    BsuCellMaxNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];

    /*ecu(绝对从机ID)→(簇号, 簇内从机号)换算: 地址0-15对应电芯1-832*/
    CluterNum = ecu / bsuNum;
    packId = ecu % bsuNum;

    if((packId >= bsuNum) || (cell >= BsuCellMaxNum))
    {
        return;
    }

    /*每帧最多16个u32(64字节CAN FD), 逐个解析小端u32*/
    for(i = 0; i < (len / 4); i++)
    {
        cellId = (u16)(cell + i);
        if(cellId >= BsuCellMaxNum)
        {
            break;   /*本从机末帧不足16颗时(如52颗只剩4颗), 超出的位无效*/
        }
        {
            /*全局电芯索引 = 簇偏移 + 从机偏移 + 从机内电芯偏移*/
            u32 globalIdx = (u32)CluterNum * (u32)bsuNum * BsuCellMaxNum + (u32)packId * BsuCellMaxNum + cellId;
            if(globalIdx < vSumNum)
            {
                pBuf[globalIdx] = (u32)pdata[i * 4]
                                | ((u32)pdata[i * 4 + 1] << 8)
                                | ((u32)pdata[i * 4 + 2] << 16)
                                | ((u32)pdata[i * 4 + 3] << 24);
            }
        }
    }
}

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisRealInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,48] pdata:接收的报文数据域 len:数据长度[0,64]
** @Output    : void
** @Function  : 接收从机单体内阻实部信息(u32,1μΩ), 存gBsuCellResisReal后填ICC28-31
** @The notes : 索引[50,53],每帧16个u32
**===============================================================================================*/
static void RcvSlaveCellResisRealInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
{
    RcvSlaveCellResisInfoStore(ecu, cell, pdata, len, gBsuCellResisReal);
    InsuResFill();
}

/*=================================================================================================
** @Name      : static void RcvSlaveCellResisImagInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) cell:起始内阻号[0,48] pdata:接收的报文数据域 len:数据长度[0,64]
** @Output    : void
** @Function  : 接收从机单体内阻虚部信息(u32,1μΩ), 存gBsuCellResisImag后填ICC32-35
** @The notes : 索引[90,93],每帧16个u32
**===============================================================================================*/
static void RcvSlaveCellResisImagInfo(u8 ecu, u8 cell, u8 *pdata, u8 len)
{
    RcvSlaveCellResisInfoStore(ecu, cell, pdata, len, gBsuCellResisImag);
    InsuResFill();
}

/*=================================================================================================
** @Name      : static void RcvSlaveAfeStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
** @Input     : ecu:从机绝对ID(0~15) index:AFE帧序号[0,10] pdata:接收的报文数据域 len:数据长度
** @Output    : void
** @Function  : 接收从机AFE状态信息(每4bit代表1个AFE IC状态)
** @The notes : 索引[130,140],拆包后每个AFE IC存1个u8,状态值定义见e_AfeState
**===============================================================================================*/
static void RcvSlaveAfeStateInfo(u8 ecu, u8 index, u8 *pdata, u8 len)
{
    u8 i = 0;
    u8 CluterNum = 0;
    u8 packId = 0;
    u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];

    if(cfgBsuNum == 0 || pdata == NULL || len == 0)
    {
        return;
    }

    CluterNum = ecu / cfgBsuNum;
    packId = ecu % cfgBsuNum;

    if(CluterNum >= CLUTER_MAX_NUM || packId >= cfgBsuNum)
    {
        return;
    }

    for(i = 0; i < len; i++)
    {
        u8 lowNibble = pdata[i] & 0x0F;
        u8 highNibble = (pdata[i] >> 4) & 0x0F;

        WriteSampAfeStateHook(CluterNum, packId, index * len * 2 + i * 2, lowNibble);
        WriteSampAfeStateHook(CluterNum, packId, index * len * 2 + i * 2 + 1, highNibble);
    }
}

/*=================================================================================================
** @Name      : static void RcvBsuDevInfo(u8 ecu, u8 *pdata, u8 len)
** @Input     : ecu:从控地址  pdata:数据域指针  len:数据长度
** @Output    : void
** @Function  : 接收BSU主1设备信息查询回复
** @The notes : 数据帧 [SOF][SDLC][SPF][预留][u16数据...], 版本在子索引2/3/4
**===============================================================================================*/
static void RcvBsuDevInfo(u8 ecu, u8 *pdata, u8 len)
{
    u8 sdlc;
    u8 spf;
    u8 i;

    if((pdata == NULL) || (len < 4))
    {
        return;
    }
    sdlc = pdata[1];
    spf  = pdata[2];
    for(i = 0; (i < sdlc) && ((spf + i) < BSU_DEVINFO_LEN) && ((4 + 2 * i + 1) < len); i++)
    {
        gBsuDevInfo[spf + i] = (u16)((pdata[5 + 2 * i] << 8) | pdata[4 + 2 * i]);
    }
    gBsuDevInfoReplyFlag = 1;
    ICCVerSrlReplyNotify(ICC_DEVTYPE_BSU, VSRL_EVT_DEVINFO);
}

/*=================================================================================================
** @Name      : static void RcvBsuSerial(u8 ecu, u8 *pdata, u8 len)
** @Input     : ecu:从控地址  pdata:数据域指针  len:数据长度
** @Output    : void
** @Function  : 接收BSU主230单板序列号查询回复
** @The notes : 数据帧 [SOF][SDLC][SPF][预留][23字节ASCII...]
**===============================================================================================*/
static void RcvBsuSerial(u8 ecu, u8 *pdata, u8 len)
{
    u8 sdlc;
    u8 spf;
    u8 i;

    if((pdata == NULL) || (len < 4))
    {
        return;
    }
    sdlc = pdata[1];
    spf  = pdata[2];
    for(i = 0; (i < sdlc) && ((spf + i) < BSU_SERIAL_LEN) && ((4 + i) < len); i++)
    {
        gBsuSerial[spf + i] = pdata[4 + i];
    }
    gBsuSerialReplyFlag = 1;
    ICCVerSrlReplyNotify(ICC_DEVTYPE_BSU, VSRL_EVT_SERIAL);
}

/*=================================================================================================
** @Name      : static void RcvBsuPackSerial(u8 ecu, u8 *pdata, u8 len)
** @Input     : ecu:从控地址  pdata:数据域指针  len:数据长度
** @Output    : void
** @Function  : 接收BSU主231 PACK序列号查询回复
** @The notes : 数据帧 [SOF][SDLC][SPF][预留][20字节ASCII...]
**===============================================================================================*/
static void RcvBsuPackSerial(u8 ecu, u8 *pdata, u8 len)
{
    u8 sdlc;
    u8 spf;
    u8 i;

    if((pdata == NULL) || (len < 4))
    {
        return;
    }
    sdlc = pdata[1];
    spf  = pdata[2];
    for(i = 0; (i < sdlc) && ((spf + i) < BSU_PACKSERIAL_LEN) && ((4 + i) < len); i++)
    {
        gBsuPackSerial[spf + i] = pdata[4 + i];
    }
    gBsuPackSerialReplyFlag = 1;
    ICCVerSrlReplyNotify(ICC_DEVTYPE_BSU, VSRL_EVT_PACKSERIAL);
}

/*=================================================================================================
** @Name      : static void RcvBsuCellSerial(u8 ecu, u8 *pdata, u8 len)
** @Input     : ecu:从控地址  pdata:数据域指针  len:数据长度
** @Output    : void
** @Function  : 接收BSU主232电芯序列号查询回复
** @The notes : 数据帧 [SOF][SDLC][SPF][预留][电芯序号][24字节条码...]
**===============================================================================================*/
static void RcvBsuCellSerial(u8 ecu, u8 *pdata, u8 len)
{
    u8 i;

    if((pdata == NULL) || (len < 5))
    {
        return;
    }
    for(i = 0; (i < BSU_CELLSERIAL_LEN) && ((5 + i) < len); i++)
    {
        gBsuCellSerial[i] = pdata[5 + i];
    }
    gBsuCellSerialReplyFlag = 1;
    ICCVerSrlReplyNotify(ICC_DEVTYPE_BSU, VSRL_EVT_CELLSERIAL);
}

/*=================================================================================================
** @Name      : u8 BSU_SendDevInfoQuery(u8 addr)
** @Input     : addr:BSU从控地址
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主1设备信息查询(CAN1, func=5)
**===============================================================================================*/
u8 BSU_SendDevInfoQuery(u8 addr)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    msg.id  = CanBuildRupAuxMsgID(5, BSU_MINDEX_DEVINFO, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = 4;
    msg.data[0] = 0x0C;
    msg.data[1] = BSU_DEVINFO_LEN;
    msg.data[2] = 0;
    msg.data[3] = 0;

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : u8 BSU_SendSerialQuery(u8 addr)
** @Input     : addr:BSU从控地址
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主230单板序列号查询(CAN1, func=5)
**===============================================================================================*/
u8 BSU_SendSerialQuery(u8 addr)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    msg.id  = CanBuildRupAuxMsgID(5, BSU_MINDEX_SERIAL, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = 4;
    msg.data[0] = 0x0C;
    msg.data[1] = BSU_SERIAL_LEN;
    msg.data[2] = 0;
    msg.data[3] = 0;

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : u8 BSU_SendPackSerialQuery(u8 addr)
** @Input     : addr:BSU从控地址
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主231 PACK序列号查询(CAN1, func=5)
**===============================================================================================*/
u8 BSU_SendPackSerialQuery(u8 addr)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    msg.id  = CanBuildRupAuxMsgID(5, BSU_MINDEX_PACKSERIAL, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = 4;
    msg.data[0] = 0x0C;
    msg.data[1] = BSU_PACKSERIAL_LEN;
    msg.data[2] = 0;
    msg.data[3] = 0;

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : u8 BSU_SendPackSerialWrite(u8 addr, u8 *data)
** @Input     : addr:BSU从控地址  data:20字节PACK序列号
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主231 PACK序列号写入(CAN1, func=2)
**===============================================================================================*/
u8 BSU_SendPackSerialWrite(u8 addr, u8 *data)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    u8 i;

    if(data == NULL)
    {
        return 0;
    }
    msg.id  = CanBuildRupAuxMsgID(2, BSU_MINDEX_PACKSERIAL, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = (u8)(4 + BSU_PACKSERIAL_LEN);
    msg.data[0] = 0x0C;
    msg.data[1] = BSU_PACKSERIAL_LEN;
    msg.data[2] = 0;
    msg.data[3] = 0;
    for(i = 0; i < BSU_PACKSERIAL_LEN; i++)
    {
        msg.data[4 + i] = data[i];
    }

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : u8 BSU_SendCellSerialQuery(u8 addr, u8 cellIdx)
** @Input     : addr:BSU从控地址  cellIdx:电芯序号
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主232电芯序列号查询(CAN1, func=5)
**===============================================================================================*/
u8 BSU_SendCellSerialQuery(u8 addr, u8 cellIdx)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    msg.id  = CanBuildRupAuxMsgID(5, BSU_MINDEX_CELLSERIAL, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = 5;
    msg.data[0] = 0x0C;
    msg.data[1] = BSU_CELLSERIAL_LEN;
    msg.data[2] = 0;
    msg.data[3] = 0;
    msg.data[4] = cellIdx;

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : u8 BSU_SendAuthCtrl(u8 addr, u8 open)
** @Input     : addr:BSU从控地址  open:1开启权限 0关闭权限
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主233条码写入权限(CAN1, func=2)
**===============================================================================================*/
u8 BSU_SendAuthCtrl(u8 addr, u8 open)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    msg.id  = CanBuildRupAuxMsgID(2, BSU_MINDEX_AUTH, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = 8;
    msg.data[0] = 0x0C;
    msg.data[1] = 4;
    msg.data[2] = 0;
    msg.data[3] = 0;
    if(open)
    {
        msg.data[4] = BSU_AUTH_OPEN_PWD0;
        msg.data[5] = BSU_AUTH_OPEN_PWD1;
        msg.data[6] = BSU_AUTH_OPEN_PWD2;
        msg.data[7] = BSU_AUTH_OPEN_PWD3;
    }
    else
    {
        msg.data[4] = BSU_AUTH_CLOSE_PWD0;
        msg.data[5] = BSU_AUTH_CLOSE_PWD1;
        msg.data[6] = BSU_AUTH_CLOSE_PWD2;
        msg.data[7] = BSU_AUTH_CLOSE_PWD3;
    }

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Name      : u8 BSU_SendInsuScanCtrl(u8 addr, u16 freqIdx, u16 mode)
** @Input     : addr:BSU从控地址(广播0xff)  freqIdx:内阻检测频率索引[0,228]  mode:检测方式(0全部 1奇数 2偶数)
** @Output    : 发送结果 0失败 非0成功
** @Function  : 发送BSU主88内阻扫描控制指令(CAN1, func=2), 启停控制固定为1(触发)
**===============================================================================================*/
u8 BSU_SendInsuScanCtrl(u8 addr, u16 freqIdx, u16 mode)
{
    can_frame_t msg;
    u8 bmuId = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];

    msg.id  = CanBuildRupAuxMsgID(2, BSU_MINDEX_INSU_CTRL, addr, bmuId);
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    msg.fd  = 1;
    msg.len = 10;
    msg.data[0] = 0x0C;
    msg.data[1] = 3;
    msg.data[2] = 0;
    msg.data[3] = 0;
    msg.data[4] = 1;
    msg.data[5] = 0;
    msg.data[6] = (u8)freqIdx;
    msg.data[7] = (u8)(freqIdx >> 8);
    msg.data[8] = (u8)mode;
    msg.data[9] = (u8)(mode >> 8);

    return BSPCANSendQin(eCAN1, &msg);
}

/*=================================================================================================
** @Function    : slave_test_main
** @Instruction : 综合调试工具：包含架构配置、数据读取、底层日志控制
** @Usage       : 
** 1. slave_test config [簇数] [从机数] [模组/从机] [电芯/模组]  - 一键配置架构
** 2. slave_test read <volt|temp|chg|dhg> [组号]              - 读取解析后的内存数据
** 3. slave_test chara <1|2>                     - Read chara data (Max/Min/Avg)
** 4. slave_test packenv <cluster> <pack>        - 读取PACK环境信息
** 5. slave_test log <on|off>                    - Enable/Disable TX/RX debug print
**===============================================================================================*/
static int slave_test_main(int argc, char *argv[])
{

    if (argc == 0) {
        printf("Usage:\r\n");
        printf("  slave_test config <Clu> <Bsu> <Con> <VNum> : Config architecture\r\n");
        printf("  slave_test read <volt|temp|resis|chg|dhg> [group]: Read samp data\r\n");
        printf("  slave_test chara <1|2>                     : Read chara data (Max/Min/Avg)\r\n");
        printf("  slave_test packenv <cluster> <pack>        : Read PACK environment info\r\n");
        printf("  slave_test param <cluster> <pack>          : Read BSU parameters\r\n");
        printf("  slave_test afestate <cluster> <pack>       : Read AFE state info\r\n");
        printf("  slave_test log <on|off>                    : Enable/Disable TX/RX debug print\r\n");
        return CLI_FALSE;
    }

    if (!strcmp(argv[0], "clear"))
    {
        u16 vSum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];
        if (vSum == 0) vSum = 416; // 默认最大
        memset(gGCellVoltInfo_1, 0, vSum * 2);
        memset(gGCellVoltInfo_2, 0, vSum * 2);
        memset(gGChgBalanInfo_1, 0, (vSum/16 + 1) * 2);
        memset(gGDhgBalanInfo_1, 0, (vSum/16 + 1) * 2);
        if(gGCellResisInfo_1 != NULL) memset(gGCellResisInfo_1, 0, vSum * 2);
        if(gGCellResisInfo_2 != NULL) memset(gGCellResisInfo_2, 0, vSum * 2);
        if(gGAfeStateInfo_1 != NULL)  memset(gGAfeStateInfo_1, 0, GINDEX98_LEN);
        if(gGAfeStateInfo_2 != NULL)  memset(gGAfeStateInfo_2, 0, GINDEX98_LEN);
        printf(">>> All Buffers Cleared to 0 <<<\r\n");
    }
    // ---  config (架构一键配置并检查) ---
    else if (!strcmp(argv[0], "config"))
    {
        if (argc < 5) { printf("Error: Need [Cluster] [BsuNum] [ConNum] [CellVNum]\r\n"); return 0; }
        
        u16 clu  = (u16)atoi(argv[1]); // 簇数
        u16 bsu  = (u16)atoi(argv[2]); // 从机(Pack)数
        u16 con  = (u16)atoi(argv[3]); // 每个Pack几个模组
        u16 vnum = (u16)atoi(argv[4]); // 每个模组几节电芯

        if (clu < 1 || clu > 2)   { printf("Err: Cluster must be [1,2]\r\n"); return 0; }
        if (bsu < 1 || bsu > 30)  { printf("Err: BsuNum must be [1,30]\r\n"); return 0; }
        if (con < 1 || con > 30)  { printf("Err: ConNum must be [1,30]\r\n"); return 0; }
        if (vnum < 1 || vnum > 16){ printf("Err: BsuVNum must be [1,16]\r\n"); return 0; }

        u16 vSum = bsu * con * vnum; 
        
        if (vSum > 416) { 
            printf("Err: Total cells per cluster (%d) exceeds max limit 416!\r\n", vSum); 
            return 0; 
        }

        gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum] = clu;
        gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]     = bsu;
        gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]     = con;
        gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]    = vnum;
        gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum]    = vnum; // 温度跟随电芯数
        
        gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]    = vSum; 
        gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]    = vSum; 
        gGBmuHigLevPara_103[eBmuHigLevPara103_CfgFlag]    = 1;

        printf(">>> Config Success! Total Cells/Cluster: %d <<<\r\n", gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum]);
    }
    // ---  read (读取数据) ---
    else if (!strcmp(argv[0], "read"))
    {
        if (argc < 2) { printf("Use: read <volt|temp|chg|dhg> [group]\r\n"); return 0; }
        u8 group = (argc > 2) ? (u8)atoi(argv[2]) : 1;
        
        if (!strcmp(argv[1], "volt")) {
            u16 *pVolt = (group == 2) ? gGCellVoltInfo_2 : gGCellVoltInfo_1;
            u16 vSum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];
            if (pVolt == NULL) { printf("Err: Volt buf NULL\r\n"); return 0; }
            printf("--- Cluster %d Volt (Total %d) ---\r\n", group, vSum);
            for (u16 i = 0; i < vSum; i++) {
                printf("[%03d]: %4d ", i, pVolt[i]);
                if ((i + 1) % 4 == 0) printf("\r\n");
            }
        } 
        else if (!strcmp(argv[1], "temp")) {
            s16 *pTemp = (group == 2) ? gGCellTempInfo_2 : gGCellTempInfo_1;
            u16 tSum = gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum];
            if (pTemp == NULL) { printf("Err: Temp buf NULL\r\n"); return 0; }
            printf("--- Cluster %d Temp (Total %d) ---\r\n", group, tSum);
            for (u16 i = 0; i < tSum; i++) {
                printf("[%03d]: %d ", i, pTemp[i]);
                if ((i + 1) % 4 == 0) printf("\r\n");
            }
        }
        else if (!strcmp(argv[1], "chg") || !strcmp(argv[1], "dhg")) {
            u16 *pBalan = NULL;
            if (!strcmp(argv[1], "chg")) pBalan = (group == 2) ? gGChgBalanInfo_2 : gGChgBalanInfo_1;
            else                         pBalan = (group == 2) ? gGDhgBalanInfo_2 : gGDhgBalanInfo_1;
            
            u16 vSum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];
            u16 arrLen = (vSum + 15) / 16; 
            
            if (pBalan == NULL) { printf("Err: Balan buf NULL\r\n"); return 0; }
            printf("--- Cluster %d %s Balance State (Hex, 1bit=1cell) ---\r\n", group, argv[1]);
            
            for (u16 i = 0; i < arrLen; i++) {
                u16 startCell = i * 16;
                u16 endCell = startCell + 15;
                if (endCell >= vSum) endCell = vSum - 1;
                printf("[%03d-%03d]: 0x%04X  ", startCell, endCell, pBalan[i]);
                if ((i + 1) % 4 == 0) printf("\r\n");
            }
        }
        else if (!strcmp(argv[1], "resis")) {
            u16 *pResis = (group == 2) ? gGCellResisInfo_2 : gGCellResisInfo_1;
            u16 vSum = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum];
            if (pResis == NULL) { printf("Err: Resis buf NULL\r\n"); return 0; }
            printf("--- Cluster %d Resis (Total %d, unit 1μΩ) ---\r\n", group, vSum);
            for (u16 i = 0; i < vSum; i++) {
                printf("[%03d]: %5d ", i, pResis[i]);
                if ((i + 1) % 4 == 0) printf("\r\n");
            }
        }
        printf("\r\n");
    } 

    // --- chara (打印特征值) ---
    else if (!strcmp(argv[0], "chara"))
    {
        if (argc < 2) { printf("Use: chara <1|2> (1=Cluster1, 2=Cluster2)\r\n"); return 0; }
        u8 group = (u8)atoi(argv[1]);

        if (gSysCharInfo_54 == NULL) {
            printf("Err: gSysCharInfo_54 memory is NULL!\r\n");
            return 0;
        }

        if (group == 1) 
        {
            printf("\r\n=== Cluster 1 Characteristics ===\r\n");
            printf("[Volt] Max: %4dmV (Cell %03d) | Min: %4dmV (Cell %03d) | Avg: %4dmV | Sum: %d.%dV\r\n",
                gSysCharInfo_54[eSysCharnfo54_1MaxVolt], gSysCharInfo_54[eSysCharnfo54_1MaxVNum],
                gSysCharInfo_54[eSysCharnfo54_1MinVolt], gSysCharInfo_54[eSysCharnfo54_1MinVNum],
                gSysCharInfo_54[eSysCharnfo54_1AvgV],    
                gSysCharInfo_54[eSysCharnfo54_1SumV]/10, gSysCharInfo_54[eSysCharnfo54_1SumV]%10);
            
            printf("[Temp] Max: %4.1fC (Cell %03d) | Min: %4.1fC (Cell %03d) | Avg: %4.1fC\r\n",
                (float)gSysCharInfo_54[eSysCharnfo54_1MaxTemp]/10.0, gSysCharInfo_54[eSysCharnfo54_1MaxTNum],
                (float)gSysCharInfo_54[eSysCharnfo54_1MinTemp]/10.0, gSysCharInfo_54[eSysCharnfo54_1MinTNum],
                (float)gSysCharInfo_54[eSysCharnfo54_1AvgT]/10.0);
            printf("=================================\r\n");
        } 
        else if (group == 2) 
        {
            printf("\r\n=== Cluster 2 Characteristics ===\r\n");
            printf("[Volt] Max: %4dmV (Cell %03d) | Min: %4dmV (Cell %03d) | Avg: %4dmV | Sum: %d.%dV\r\n",
                gSysCharInfo_54[eSysCharnfo54_2MaxVolt], gSysCharInfo_54[eSysCharnfo54_2MaxVNum],
                gSysCharInfo_54[eSysCharnfo54_2MinVolt], gSysCharInfo_54[eSysCharnfo54_2MinVNum],
                gSysCharInfo_54[eSysCharnfo54_2AvgV],    
                gSysCharInfo_54[eSysCharnfo54_2SumV]/10, gSysCharInfo_54[eSysCharnfo54_2SumV]%10);
                
            printf("[Temp] Max: %4.1fC (Cell %03d) | Min: %4.1fC (Cell %03d) | Avg: %4.1fC\r\n",
                (float)gSysCharInfo_54[eSysCharnfo54_2MaxTemp]/10.0, gSysCharInfo_54[eSysCharnfo54_2MaxTNum],
                (float)gSysCharInfo_54[eSysCharnfo54_2MinTemp]/10.0, gSysCharInfo_54[eSysCharnfo54_2MinTNum],
                (float)gSysCharInfo_54[eSysCharnfo54_2AvgT]/10.0);
            printf("=================================\r\n");
        }
        else {
            printf("Err: Invalid group. Use 1 or 2.\r\n");
        }
    }
    // --- packenv (读取PACK环境信息) ---
    else if (!strcmp(argv[0], "packenv"))
    {
        if (argc < 3) { 
            printf("Use: packenv <cluster> <pack>\r\n"); 
            printf("Example: packenv 1 0 (Cluster 1, Pack 0)\r\n");
            return 0; 
        }
        
        u8 cluster = (u8)atoi(argv[1]);
        u8 pack = (u8)atoi(argv[2]);
        u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
        
        if (cluster < 1 || cluster > CLUTER_MAX_NUM) {
            printf("Err: Cluster must be [1, %d]\r\n", CLUTER_MAX_NUM);
            return 0;
        }
        
        if (cfgBsuNum == 0) {
            printf("Err: BsuNum not configured! Use 'slave_test config' first.\r\n");
            return 0;
        }
        
        if (pack >= cfgBsuNum) {
            printf("Err: Pack ID (%d) out of range [0, %d]\r\n", pack, cfgBsuNum - 1);
            return 0;
        }
        
        u8 cluIdx = cluster - 1;
        
        if (gPackTemp_94 == NULL || gPackStat_95 == NULL) {
            printf("Err: PACK environment buffer NULL!\r\n");
            return 0;
        }
        
        printf("\r\n=== Cluster %d, Pack %d Environment Info ===\r\n", cluster, pack);
        printf("--- Temperature (s16, 0.1°C) ---\r\n");
        printf("PT1 self Temp (PT1自检温度): %4.1f °C\r\n", (float)gPackTemp_94[cluIdx][pack][ePackTemp94_Pt1SelfTemp] / 10.0f);
        printf("PT2 self Temp (PT2自检温度): %4.1f °C\r\n", (float)gPackTemp_94[cluIdx][pack][ePackTemp94_Pt2SelfTemp] / 10.0f);
        printf("PT1000-1 Temp:           %4.1f °C\r\n", (float)gPackTemp_94[cluIdx][pack][ePackTemp94_Pt1Temp] / 10.0f);
        printf("PT1000-2 Temp:           %4.1f °C\r\n", (float)gPackTemp_94[cluIdx][pack][ePackTemp94_Pt2Temp] / 10.0f);
        
        printf("\r\n--- Status & Environment (u16) ---\r\n");
        printf("Water Self-Check Hz1:    %5d Hz\r\n", gPackStat_95[cluIdx][pack][ePackStat95_WaterSelfHz1]);
        printf("Water Self-Check Hz2:    %5d Hz\r\n", gPackStat_95[cluIdx][pack][ePackStat95_WaterSelfHz2]);
        printf("PT 5V Pull-up Check:     %5d mV\r\n", gPackStat_95[cluIdx][pack][ePackStat95_Pt5VPullUp]);
        printf("12V Output Check:        %5d mV\r\n", gPackStat_95[cluIdx][pack][ePackStat95_Out12VCheck]);
        printf("Water Detection Hz1:     %5d Hz\r\n", gPackStat_95[cluIdx][pack][ePackStat95_WaterHz1]);
        printf("Water Detection Hz2:     %5d Hz\r\n", gPackStat_95[cluIdx][pack][ePackStat95_WaterHz2]);
        printf("HVIL PWM Amplitude:      %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_HvilPwmAmp]);
        printf("HVIL PWM Duty Cycle:     %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_HvilPwmDuty]);
        printf("FG Frequency:            %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_FgFreq]);
        printf("FG Duty Cycle:           %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_FgDuty]);
        printf("Aerosol Concentration:   %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_AeroConc]);
        printf("Aerosol Sensor Type:     %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_AeroType]);
        printf("Aerosol State:           0x%04X\r\n", gPackStat_95[cluIdx][pack][ePackStat95_AeroState]);
        printf("AFE Bottom Count:        %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_AfeBotCount]);
        printf("AFE Top Count:           %5d\r\n", gPackStat_95[cluIdx][pack][ePackStat95_AfeTopCount]);
        printf("=========================================\r\n");
    }
    // --- param (读取从控参数信息) ---
    else if (!strcmp(argv[0], "param"))
    {
        if (argc < 3) { 
            printf("Use: param <cluster> <pack>\r\n"); 
            printf("Example: param 1 0 (Cluster 1, Pack 0)\r\n");
            return 0; 
        }
        
        u8 cluster = (u8)atoi(argv[1]);
        u8 pack = (u8)atoi(argv[2]);
        u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
        
        if (cluster < 1 || cluster > CLUTER_MAX_NUM) {
            printf("Err: Cluster must be [1, %d]\r\n", CLUTER_MAX_NUM);
            return 0;
        }
        
        if (cfgBsuNum == 0) {
            printf("Err: BsuNum not configured! Use 'slave_test config' first.\r\n");
            return 0;
        }
        
        if (pack >= cfgBsuNum) {
            printf("Err: Pack ID (%d) out of range [0, %d]\r\n", pack, cfgBsuNum - 1);
            return 0;
        }
        
        u8 cluIdx = cluster - 1;
        
        printf("\r\n=== Cluster %d, Pack %d BSU Parameters ===\r\n", cluster, pack);
        printf("[Config Flag]       CfgFlag:        %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_CfgFlag]);
        printf("[Device ID]         EcuId:          %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_EcuId]);
        printf("[Architecture]      SlaveNum:       %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SlaveNum]);
        printf("                    DaisyType:      %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyType]);
        printf("                    DaisyNum:       %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyNum]);
        printf("[Sampling]          SampVnum:       %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampVnum]);
        printf("                    SampTnum:       %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampTnum]);
        printf("                    TotalVnum:      %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalVnum]);
        printf("                    TotalTnum:      %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalTnum]);
        printf("[Balance]           PassiveDuty:    %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_PassiveDuty]);
        printf("                    ActiveDuty:     %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_ActiveDuty]);
        printf("                    BalanceMethod:  %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_BalanceMethod]);
        printf("[CAN]               CANCtrlVer:     %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_CANCtrlVer]);
        printf("                    CANBaudRate:    %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_CANBaudRate]);
        printf("[Shield]            CellShieldEn:   %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_CellShieldEn]);
        printf("[AFE]               AFELowPower:    %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_AFELowPowerMode]);
        printf("================================================\r\n");
        
        // 与配置参数对比
        u16 cfgValue = cfgBsuNum / gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];
        printf("\r\n=== Parameter Consistency Check ===\r\n");
        
        if (gGHardPara_104[eGHardPara104_BmuSWVerHH] != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyType]) {
            printf("[MISMATCH] DaisyType: Config=%d, Actual=%d\r\n", 
                   gGHardPara_104[eGHardPara104_BmuSWVerHH], gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyType]);
        } else {
            printf("[OK] DaisyType: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyType]);
        }
        
        if (cfgValue != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SlaveNum]) {
            printf("[MISMATCH] SlaveNum: Config=%d, Actual=%d\r\n", 
                   cfgValue, gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SlaveNum]);
        } else {
            printf("[OK] SlaveNum: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SlaveNum]);
        }
        
        if (gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyNum]) {
            printf("[MISMATCH] DaisyNum: Config=%d, Actual=%d\r\n", 
                   gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum], gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyNum]);
        } else {
            printf("[OK] DaisyNum: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_DaisyNum]);
        }
        
        if (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampVnum]) {
            printf("[MISMATCH] SampVnum: Config=%d, Actual=%d\r\n", 
                   gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum], gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampVnum]);
        } else {
            printf("[OK] SampVnum: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampVnum]);
        }
        
        if (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampTnum]) {
            printf("[MISMATCH] SampTnum: Config=%d, Actual=%d\r\n", 
                   gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum], gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampTnum]);
        } else {
            printf("[OK] SampTnum: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_SampTnum]);
        }
        
        u16 expectedTotalV = gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum] / cfgValue;
        if (expectedTotalV != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalVnum]) {
            printf("[MISMATCH] TotalVnum: Config=%d, Actual=%d\r\n", 
                   expectedTotalV, gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalVnum]);
        } else {
            printf("[OK] TotalVnum: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalVnum]);
        }
        
        u16 expectedTotalT = gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum] / cfgValue;
        if (expectedTotalT != gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalTnum]) {
            printf("[MISMATCH] TotalTnum: Config=%d, Actual=%d\r\n", 
                   expectedTotalT, gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalTnum]);
        } else {
            printf("[OK] TotalTnum: %d\r\n", gGBsuGenPara_102[cluIdx][pack][eGBsuPara102_TotalTnum]);
        }
        
        printf("================================================\r\n");
    }
    // --- afestate (读取AFE状态信息) ---
    else if (!strcmp(argv[0], "afestate"))
    {
        if (argc < 3) {
            printf("Use: afestate <cluster> <pack>\r\n");
            printf("Example: afestate 1 0 (Cluster 1, Pack 0)\r\n");
            return 0;
        }

        u8 cluster = (u8)atoi(argv[1]);
        u8 pack = (u8)atoi(argv[2]);
        u8 cfgBsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
        u16 conNum = gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];

        if (cluster < 1 || cluster > CLUTER_MAX_NUM) {
            printf("Err: Cluster must be [1, %d]\r\n", CLUTER_MAX_NUM);
            return 0;
        }

        if (cfgBsuNum == 0) {
            printf("Err: BsuNum not configured! Use 'slave_test config' first.\r\n");
            return 0;
        }

        if (pack >= cfgBsuNum) {
            printf("Err: Pack ID (%d) out of range [0, %d]\r\n", pack, cfgBsuNum - 1);
            return 0;
        }

        u8 cluIdx = cluster - 1;
        u8 *pAfe = (cluIdx == 0) ? gGAfeStateInfo_1 : gGAfeStateInfo_2;

        if (pAfe == NULL) {
            printf("Err: AFE state buffer NULL!\r\n");
            return 0;
        }

        u16 afeCount = (conNum > 0) ? conNum : 0;
        printf("\r\n=== Cluster %d, Pack %d AFE State (Total %d AFE ICs) ===\r\n", cluster, pack, afeCount);
        for (u16 i = 0; i < afeCount; i++) {
            u16 index = (u16)pack * conNum + i;
            if (index >= GINDEX98_LEN) break;
            u8 state = pAfe[index];
            const char *stateStr = "Unknown";
            switch(state) {
                case eAfeState_Sleep:     stateStr = "Sleep"; break;
                case eAfeState_Standby:   stateStr = "Standby"; break;
                case eAfeState_Normal:    stateStr = "Normal"; break;
                case eAfeState_SelfTest:  stateStr = "SelfTest"; break;
                case eAfeState_Safe:      stateStr = "Safe"; break;
                case eAfeState_SpiNormal: stateStr = "SPI-Normal"; break;
                default: break;
            }
            printf("[AFE %02d]: %2d (%s)  ", i, state, stateStr);
            if ((i + 1) % 4 == 0) printf("\r\n");
        }
        printf("\r\n================================================\r\n");
    }
    return 0;
}
/*# 1. 配置系统：1个簇，4个PACK，每个PACK有2个模组，每个模组16节电芯
slave_test config 1 4 2 16

# 2. 查看第1个簇的所有电芯电压
slave_test read volt 1

# 3. 查看第1个簇的所有电芯内阻
slave_test read resis 1

# 4. 查看第2个簇的特征值
slave_test chara 2

# 5. 查看第1个簇、第0个PACK的环境信息
slave_test packenv 1 0

# 6. 查看第1个簇、第0个PACK的从控参数
slave_test param 1 0

# 7. 查看第1个簇、第0个PACK的AFE状态
slave_test afestate 1 0

# 8. 清空所有数据
slave_test clear*/
CLI_CMD("slave_test", "BMS Unified Debug Tool", slave_test_main);
