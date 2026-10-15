/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : DiagCheck.c
** @Founder      : ZQD808
** @Date         : 2026.03.05
** @Function     : 电池故障诊断模块接口
** @Instructions : 
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "BSPUserAPI.h"
#include "PublicPara.h"
#include "GroupPara.h"
#include "PublicInfo.h"
#include "GroupInfo.h"
#include "DiagInterface.h"
#include "CellAFESamp.h"
#include "DiagFilter.h"
#include "DiagUser.h"
#include "TacUser.h"
#include "BalanTactic.h"
#include "DiagMaker.h"
#include "DiagCheck.h"
#include <stdio.h>
#include <debug.h>
extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/


static u8 sRcvIntCount[DIAG_SLAVE_NUM] = {0};               /*接收内部从控信息计数*/
static u8 sRcvPerImeu3_1Count = 0;                          /*接收IMEU3信息计数*/
static u8 sRcvPerImiu4_1Count = 0;                          /*接收1#IMIU4信息计数*/
static u8 sRcvPerImiu4_2Count = 0;                          /*接收2#IMIU4信息计数*/

static u8 sAFESErrFlag[2] = {0};                                              /*模拟前端采样异常从控内部故障(0:正常 1:异常)*/
static u8 sAFESErrSlaveId[2] = {0};                                           /*模拟前端采样异常从控序号[1-32]*/
uint64_t sRcvSlaCellVBit[CLUTER_MAX_NUM][MAX_BSU_NUM] = {0};                  /*接收各个从控单体电压的电池(按位表示:0未接收 1:已接收)*/
uint64_t sRcvSlaCellTBit[CLUTER_MAX_NUM][MAX_BSU_NUM] = {0};                  /*接收各个从控单体温度的电池(按位表示:0未接收 1:已接收)*/

static u16 sRcvAllCellVNum[CLUTER_MAX_NUM] = {0};                             /*接收全部单体电压节数计数[0,416]*/
static u16 sRcvAllCellTNum[CLUTER_MAX_NUM] = {0};                             /*接收全部单体温度节数计数[0,416]*/

static u8 sRcvAllCellFlag[CLUTER_MAX_NUM] = {0};                              /*接收全部单体信息标志(0未接收 1正在接收 2接收完成)*/
static u8 sRcvAllSlaveFlag[CLUTER_MAX_NUM] = {0};                             /*接收全部单体信息标志(0未接收 1正在接收 2接收完成)*/
static u8 sBalanStateMismatchCount[CLUTER_MAX_NUM][MAX_BSU_NUM] = {{0}};      /*均衡命令/状态回报不一致连续计数(100ms)*/
static u8 sCellVOffSlaveId[CLUTER_MAX_NUM] = {0};                             /*采样值检测单体电压采样断线从控序号[1-32]*/
static u32 sCellVOffCellBit[CLUTER_MAX_NUM] = {0};                            /*采样值检测单体电压采样断线从控内部故障位(按位表示1个电压通道)*/
static u8 sCellTOffSlaveId[CLUTER_MAX_NUM] = {0};                             /*采样值检测单体温度采样断线从控序号[1-32]*/
static u16 sCellTOffCellBit[CLUTER_MAX_NUM] = {0};                            /*采样值检测单体温度采样断线从控内部故障位(按位表示1个温度通道)*/
static u8 sPerOffCommuBit = 0;                                                /*内部外设子系统通信故障位(按位表示1类子设备:Bit0：IMEU3通信中断 BIT1: 1#IMIU4通信中断 BIT2:2#IMIU4通信中断)*/

static u8 sHigVoltExpFlag[CLUTER_MAX_NUM] = {0};                              /*高压采样值异常标志 bit0:总电压异常 bit1:输出端电压异常 [0:正常 1:异常]*/
static u8 sHVSubDiffFlag[CLUTER_MAX_NUM] = {0};                               /*高压采样内外压差过大标志 [0:正常 1:压差过大]*/
static u8 sSwhOnVExpFlag[CLUTER_MAX_NUM] = {0};                               /*电压判断接触器闭合异常标志 按位表示 Bit0:主接触器 bit1:预充接触器  bit2:负极接触器 [0:正常 1:异常]*/
static u8 sSwhOffVExpFlag[CLUTER_MAX_NUM] = {0};                              /*电压判断接触器断开异常标志 按位表示 Bit0:主接触器 bit1:预充接触器  bit2:负极接触器 [0:正常 1:异常]*/
static u8 sSwhOnCExpFlag[CLUTER_MAX_NUM] = {0};                               /*接触器带载闭合标志(按位表示各接触器)*/
static u8 sSwhOffCExpFlag[CLUTER_MAX_NUM] = {0};                              /*接触器带载断开标志(按位表示各接触器)*/


static u32 sBsu02u1FloodErrBit = 0;         /*从控水浸异常故障位（按bit表示，支持最大32路）*/
static u32 sBsu02u1PT1000ErrBit = 0;        /*从控PT1000温度异常故障位（按bit表示，支持最大32路）*/
static u32 sBsu02u1AeroSenErrBit = 0;       /*从控气溶胶传感器异常故障位（按bit表示，支持最大32路）*/
static u32 sBsu02u1MsdErrBit = 0;           /*从控MSD状态异常故障位（按bit表示，支持最大32路）*/
static u32 sBsu02u1AfeEnumErrbit = 0;       /*从控bsu枚举计数异常故障位（按bit表示，支持最大32路）*/
/*=================================================================================================
** @Name      : u16 DiagGetCellTOffCellBitAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体温度采样线断线的从控内温度点位(按位表示1个温度点[0正常,1异常])
** @Function  : 获取单体温度采样线断线的从控内温度点位
** @The notes : 按位表示1个温度点
**===============================================================================================*/
u16 DiagGetCellTOffCellBitAPI(u8 clunum);
/*=================================================================================================
** @Name      : u16 DiagGetCellSampErrFlagAPI(u8 clunum)
** @Input     : clunum
** @Output    : 按bit位表示单体信息采样异常标志[bit0:单体电压 bit1:单体温度](0正常,1异常)
** @Function  : 获取单体信息采样异常标志
** @The notes :
**===============================================================================================*/
u16 DiagGetCellSampErrFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u32 DiagGetCellVOffCellBitAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体电压采样线断线的从控内电压通道(按位表示1个电压通道[0正常,1异常])
** @Function  : 获取单体电压采样线断线的从控内电压通道
** @The notes : 按位表示1个电压通道
**===============================================================================================*/
u32 DiagGetCellVOffCellBitAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 DiagGetRcvAllSlaveFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 接收到全部从控报文标志[0:未接收,1:正在接收,2:全部接收]
** @Function  : 获取接收到全部从控报文标志
** @The notes : 任一帧报文
**===============================================================================================*/
u8 DiagGetRcvAllSlaveFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : u8 DiagGetRcvAllCellFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 接收到全部单体信息报文标志[0:未接收,1:正在接收,2:全部接收]
** @Function  : 获取接收到全部单体信息报文标志
** @The notes : 全部单体电压和温度
**===============================================================================================*/
u8 DiagGetRcvAllCellFlagAPI(u8 clunum);
/*=================================================================================================
** @Name      : void DiagRcvDevMsgAddAPI(u8 addr)
** @Input     : addr:设备地址
** @Output    : void
** @Function  : 接收外设通信信息报文计数
** @The notes : 接收到报文时调用
**===============================================================================================*/
void DiagRcvDevMsgAddAPI(u8 addr)
{
        /*内部从机(BSU,地址不含组间偏移量)*/
    if((addr < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]) && (addr < DIAG_SLAVE_NUM) && (sRcvIntCount[addr] < 254))
    {
        sRcvIntCount[addr]++;
    }
    /*内部外设(PDU内IMEU3，液冷系统地址默认30)*/
    else if((addr == IMEU3_PDUIN_DEVADDR) && (sRcvPerImeu3_1Count < 254))
    {
    	sRcvPerImeu3_1Count++;
    }
    /*1#IMIU4，地址默认20)*/
    else if((addr == IMIU4_1_DEVADDR) && (sRcvPerImiu4_1Count < 254))
    {
    	sRcvPerImiu4_1Count++;
    }
    /*2#IMIU4，地址默认21)*/
    else if((addr == IMIU4_2_DEVADDR) && (sRcvPerImiu4_2Count < 254))
    {
    	sRcvPerImiu4_2Count++;
    }
}
/*=================================================================================================
** @Name      : void DiagRcvCellMissCheckAPI(u8 type, u8 ecu, u8 cell)
** @Input     : clunum:簇编号 type:单体类型[0:单体电压 1:单体温度] ecu:从控号[0,31] cell:单体信息编号[0,17]
** @Output    : void
** @Function  : 判断是否接收所有单体信息
** @The notes :
**===============================================================================================*/
void DiagRcvCellMissCheckAPI(u8 clunum, u8 type, u8 ecu, u8 cell)
{
	u8 i = 0;
    if(clunum >= 2)
    {
        return;
    }
	if((type >= 2) || (ecu >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]))
	{
		return;
	}

	/*首次*/
	if(0 == sRcvAllCellFlag[clunum])
	{
    	for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; i++)
    	{
    		sRcvSlaCellVBit[clunum][i] = 0;
    		sRcvSlaCellTBit[clunum][i] = 0;
    	}
    	sRcvAllCellVNum[clunum] = 0;
    	sRcvAllCellTNum[clunum] = 0;
    	sRcvAllCellFlag[clunum] = 1;
	}
    /*单体电压*/
    if(0 == type)
    {
    	if(cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum])
    	{
    		return;
    	}
        /*新接收到该电芯电压*/
        if(0 == BitGet(sRcvSlaCellVBit[clunum][ecu], cell))
        {
            /*增加一节*/
            sRcvAllCellVNum[clunum]++;
            BitSet(sRcvSlaCellVBit[clunum][ecu], cell);
        }
    }
	/*单体温度*/
    else
    {
    	if(cell >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum])
    	{
    		return;
    	}

    	/*新接收到该电芯温度*/
    	if(0 == BitGet(sRcvSlaCellTBit[clunum][ecu], cell))
    	{
    		/*增加一节*/
    		sRcvAllCellTNum[clunum]++;
    		BitSet(sRcvSlaCellTBit[clunum][ecu], cell);
    	}
    }
    for(i = 0; i < 2; i++)
    {
        /*接收到最大节数*/
        if((sRcvAllCellVNum[i] >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
            && (sRcvAllCellTNum[i] >= gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum]))
        {
            sRcvAllCellFlag[i] = 2;
        }
    }
}
/*=================================================================================================
** @Name      : void DiagRecordAFESampErrFlagAPI(u8 clunum, u8 slave, u8 flag)
** @Input     : clunum:簇编号 slave:从控序号[0,31] flag:异常标志(0:电压正常 1:电压异常 2:温度正常 3:温度异常)
** @Output    : void
** @Function  : 记录模拟前端检测到的采样异常标志
** @The notes : 芯片读取到的数据异常
**===============================================================================================*/
void DiagRecordAFESampErrFlagAPI(u8 clunum, u8 slave, u8 flag)
{
	return;//后续再实现
}
/*=================================================================================================
** @Name      : u8 DiagGetCellSErrSlaveIdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体信息采样异常的从控序号[1,32]
** @Function  : 获取单体信息采样异常从控序号
** @The notes : 只记录1个从控
**===============================================================================================*/
u8 DiagGetCellSErrSlaveIdAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return (0);
	}
	return(sAFESErrSlaveId[clunum]);
}

/*=================================================================================================
** @Name      : u16 DiagGetCellSampErrFlagAPI(u8 clunum)
** @Input     : clunum
** @Output    : 按bit位表示单体信息采样异常标志[bit0:单体电压 bit1:单体温度](0正常,1异常)
** @Function  : 获取单体信息采样异常标志
** @The notes :
**===============================================================================================*/
u16 DiagGetCellSampErrFlagAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return (0);
	}
    return(sAFESErrFlag[clunum]);
}
/*=================================================================================================
** @Name      : void DiagCommuCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测通信状态任务
** @The notes : 1s周期调用
**===============================================================================================*/
static u8 DiagCommCheckCount;
void DiagCommuCheckTask(void)
{
    u8 i = 0;
    static u8 sPerImeu3Time = 0;
    static u8 sPerImiu4_1Time = 0;
    static u8 sPerImiu4_2Time = 0;
    DiagCommCheckCount++;
    /*CAN通信单体采样方式(才判断从控CAN通信状态)*/
    #if(1 == BMU_BSU_COM_TYPE)
    u8 slaNum = 0;
    u8 rcvSlaNum[2] = {0};
    static u8 sHisRcvSlaAll[2] = {0};
    static u8 sClrSlaOffTime[2] = {0};
    static u8 sSlaTime[DIAG_SLAVE_NUM] = {0};
    #endif

    for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
    {    
		if(clunum >= 2)
		{
			return;
		}
        //上电前3s等待单体信息
        if(((BSPGetOSMillSecTimeAPI() / 1000) <= DIAG_WAIT_CELL_T) && (2 != sRcvAllCellFlag[clunum]))
        {
			if(gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] >= 8)
			{
				gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] = 8;
			}
            for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; i++)
            {
                //未接收到信息则等待否则开始检测
                if(0 == sRcvIntCount[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]])
                {
                    #if(1 == BMU_BSU_COM_TYPE)
                    sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] = 0;
                    #endif
                    return;
                }
            }
        }
    }

    //IMEU3使能,新平台只有一个IMEU3
    if((gGHardPara_104[eGHardPara104_Imeu3En] & 0x01) != 0)
    {
        if(BSPGetOSMillSecTimeAPI() <= 3000)                                    //上电前3s等待通信信息
        {
        	sPerImeu3Time = 0;                                                   //不检测
        }
        else if(sRcvPerImeu3_1Count > 0)
        {
        	sRcvPerImeu3_1Count = 0;
        	sPerImeu3Time = 0;
        	BitClr(sPerOffCommuBit, 0);
        }
        else
        {
        	if(sRcvPerImeu3_1Count == 0)
        	{
        		if(sPerImeu3Time < 255)                                      //防止计时越界
        		{
        			sPerImeu3Time++;                                         //外部通讯异常延时
        		}

        		if(sPerImeu3Time >= 3)
        		{
        			if(0 == BitGet(sPerOffCommuBit, 0))
        			{
        				BitSet(sPerOffCommuBit, 0);
        				//DiagUserSelfErrRecord(eUDef_ErrRcd_ArComErr, eERR_RCDSORT_WARSET, 0,  0);
        				ClrImeu3AllInfoAPI();
        			}
        		}
        	}
        	else
        	{
        		sPerImeu3Time = 0;
        		sRcvPerImeu3_1Count = 0;
    			BitClr(sPerOffCommuBit, 0);
        	}
        }
    }
    else
    {
    	BitClr(sPerOffCommuBit, 0);
    	ClrImeu3AllInfoAPI();
    }
    //1#IMIU4使能
    if((gGHardPara_104[eGHardPara104_Imiu4En] & 0x01) != 0)
    {
        if(BSPGetOSMillSecTimeAPI() <= 3000)                                    //上电前3s等待通信信息
        {
        	sPerImiu4_1Time = 0;                                                   //不检测
        }
        else if(sRcvPerImiu4_1Count > 0)
        {
        	sRcvPerImiu4_1Count = 0;
        	sPerImiu4_1Time = 0;
        	BitClr(sPerOffCommuBit, 1);
        }
        else
        {
        	if(sRcvPerImiu4_1Count == 0)
        	{
        		if(sPerImiu4_1Time < 255)                                      //防止计时越界
        		{
        			sPerImiu4_1Time++;                                         //外部通讯异常延时
        		}

        		if(sPerImiu4_1Time >= 3)
        		{
        			if(0 == BitGet(sPerOffCommuBit, 1))
        			{
        				BitSet(sPerOffCommuBit, 1);
        				//DiagUserSelfErrRecord(eUDef_ErrRcd_ArComErr, eERR_RCDSORT_WARSET, 0,  0);
        				ClrImiu4AllInfoAPI(0);
        			}
        		}
        	}
        	else
        	{
        		sPerImiu4_1Time = 0;
        		sRcvPerImiu4_1Count = 0;
    			BitClr(sPerOffCommuBit, 1);
        	}
        }
    }
    else
    {
    	BitClr(sPerOffCommuBit, 1);
    	ClrImiu4AllInfoAPI(0);
    }
    if((gGHardPara_104[eGHardPara104_Imiu4En] & 0x02) != 0)
    {
        if(BSPGetOSMillSecTimeAPI() <= 3000)                                    //上电前3s等待通信信息
        {
        	sPerImiu4_2Time = 0;                                                   //不检测
        }
        else if(sRcvPerImiu4_2Count > 0)
        {
        	sRcvPerImiu4_2Count = 0;
        	sPerImiu4_2Time = 0;
        	BitClr(sPerOffCommuBit, 2);
        }
        else
        {
        	if(sRcvPerImiu4_2Count == 0)
        	{
        		if(sPerImiu4_2Time < 255)                                      //防止计时越界
        		{
        			sPerImiu4_2Time++;                                         //外部通讯异常延时
        		}

        		if(sPerImiu4_2Time >= 3)
        		{
        			if(0 == BitGet(sPerOffCommuBit, 2))
        			{
        				BitSet(sPerOffCommuBit, 2);
        				//DiagUserSelfErrRecord(eUDef_ErrRcd_ArComErr, eERR_RCDSORT_WARSET, 0,  0);
        				ClrImiu4AllInfoAPI(1);
        			}
        		}
        	}
        	else
        	{
        		sPerImiu4_2Time = 0;
        		sRcvPerImiu4_2Count = 0;
    			BitClr(sPerOffCommuBit, 2);
        	}
        }
    }
    else
    {
    	BitClr(sPerOffCommuBit, 2);
    	ClrImiu4AllInfoAPI(1);
    }

	
    /*IMIU4通信中断接故障1 bit10/bit11(协议表语义:bit10=1#IMIU4通信中断 bit11=2#IMIU4通信中断)
      按簇归属扇出(2026-09-18):1#(bit1)只写0簇,2#(bit2)只写1簇,与数据链路映射一致
      (IMIU4RcvHandle:1#→CalcSampleADToCurr(0)/2#→(1);DiagMaker高压/外围温度采样:1U<<(clunum+1)选位)。
      设备身份上报不受影响:故障记录附加信息直接取DiagGetRcvPerOffCommuBitAPI()全局位。
      原广播写法把同一设备故障灌进两簇诊断表,任一IMIU4断线双簇同时请求停机(跨簇误伤)。*/
    if(0 != DiagGetFilErrSortAPI(eERR_ID_PER_COFF))
    {
        DiagSetErrFilStateAPI(0, eERR_ID_PER_COFF, (0 != BitGet(sPerOffCommuBit, 1)) ? eErr_B : eErr_A);
    }
    if(0 != DiagGetFilErrSortAPI(eERR_ID_EXT_COFF))
    {
        DiagSetErrFilStateAPI(1, eERR_ID_EXT_COFF, (0 != BitGet(sPerOffCommuBit, 2)) ? eErr_B : eErr_A);
    }

    /*CAN通信单体采样方式(才判断从控CAN通信状态)*/
	#if(1 == BMU_BSU_COM_TYPE)
	for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
	{
		if(clunum >= 2)
		{
			break;
		}
		//slaNum = (gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum] / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]) /  LIQUID_PACK_EQUAL_NUM;
		//判断下层从机通讯状态(非每个Pack的从控,只是CAN通信的从机,每个从机管理多个Pack)
		slaNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
		if(slaNum >= 8)
		{
			slaNum = 8;
			gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] = 8;
		}
		for(i = 0; i < slaNum; i++)
		{
			if(0 == sRcvIntCount[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]])                                            //未接收到信息
			{
				sHisRcvSlaAll[clunum] = 0;                                              /*未收到全部报文*/

				if(sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] < 254)                                           //防止越界
				{
					sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]]++;                                              //通讯异常计时
				}

				if(0 != DiagGetFilErrSortAPI(eERR_ID_INT_COFF))                 //检测从控通讯故障
				{
					if(sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] >= 5)                                        //5s未收到信息
					{
						//新产生
						if((sRcvAllCellFlag[clunum] > 0) || (sRcvAllSlaveFlag[clunum] > 0))
						{
							//由正常转入断线
							if((2 == sRcvAllCellFlag[clunum]) || (2 == sRcvAllSlaveFlag[clunum]))
							{
								//记录事件
								//DiagUserSelfErrRecord(eUDef_ErrRcd_SlaveOff, eERR_RCDSORT_WARSET, 0,  0);
							}
							//接收标志清零
							sRcvAllCellFlag[clunum] = 0;
							sRcvAllSlaveFlag[clunum] = 0;

						}
					}

					if(sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] >= (DiagGetFilAtoBTimeAPI(eERR_ID_INT_COFF) / 10))//达到检测时间
					{
						DiagSetErrFilStateAPI(clunum, eERR_ID_INT_COFF, eErr_B);        //与从控通讯断线确定

						/*无需不停清零*/
						if(sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] <= ((DiagGetFilAtoBTimeAPI(eERR_ID_INT_COFF) / 10) + 5))
						{
							/*采样失效清除单体电压温度内阻信息(输出单体电压/内阻值为0/单体温度为-41)*/
							//for(j = 0; j < gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]; j++)
							//{
								ClrSampCellVoltHook(clunum, i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]);
								ClrSampCellTempHook(clunum, i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]);
								//后续实现 ClrSampCellResisHook(i * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] + j);
							//}
						}
					}
				}
				else                                                            //不检测从控通讯故障
				{
					rcvSlaNum[clunum]++;                                                //该从控当做接收到报文
				}
			}
			else                                                                //接收到信息
			{
				rcvSlaNum[clunum]++;
				sSlaTime[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] = 0;                                                //清除计时

				if(1 == sHisRcvSlaAll[clunum])                                       //上次也是接收全部
				{
					sRcvIntCount[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] = 0;                                        //清除报文计数
				}
			}
		}

		//if(rcvSlaNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])           //轮询一遍均接收到信息
		if(rcvSlaNum[clunum] >= slaNum)                                                 //轮询一遍均接收到信息
		{
			sHisRcvSlaAll[clunum] = 1;
			sRcvAllSlaveFlag[clunum] = 2;                                               //记录接收到所有从控信息
			DiagSetErrFilStateAPI(clunum, eERR_ID_INT_COFF, eErr_A);                    //清除与从控通讯断线故障
		}

		if(eErr_B == DiagGetErrFilStateAPI(clunum, eERR_ID_INT_COFF))                   //已上报故障
		{
			if(sClrSlaOffTime[clunum] >= 5)                                                //间隔5s
			{
				//for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; i++)
				for(i = 0; i < slaNum; i++)
				{
					sRcvIntCount[i + clunum * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]] = 0;                                        //清除所有从控报文计数
				}

				sClrSlaOffTime[clunum] = 0;
			}
			else                                                                //等待间隔时间
			{
				sClrSlaOffTime[clunum]++;
			}
		}
		else                                                                    //通信正常
		{
			sClrSlaOffTime[clunum] = 0;
		}
	}
//菊花链通信方式
	#else
	for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
	{
		if(clunum >= 2)
		{
			break;
		}
		//电压温度均采样完成则接收到所有从控信息
		if(1 == GetCellVSampFinFlagAPI())
		{
			if(1 == GetCellTSampFinFlagAPI())
			{
				sRcvAllSlaveFlag[clunum] = 2;
			}
			else
			{
				sRcvAllSlaveFlag[clunum] = 1;
			}
		}
		else if(1 == GetCellTSampFinFlagAPI())
		{
			sRcvAllSlaveFlag[clunum] = 1;
		}
		else
		{
			sRcvAllSlaveFlag[clunum] = 0;
		}
	}
	#endif
}
/*=============================================================================================
** @Name      : void DiagClrPerDevExpFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : void
** @Function  : 清除外部设备检测异常标志
** @The notes : 高压侧器件故障标志
**===========================================================================================*/
void DiagClrPerDevExpFlagAPI(u8 clunum)
{
	//后续补充
	if(clunum >= 2)
	{
		return;
	}
	sHigVoltExpFlag[clunum] = 0;                            /*高压采样值异常标志 bit0:总电压异常 bit1:输出端电压异常 [0:正常 1:异常]*/
	sHVSubDiffFlag[clunum] = 0;                             /*高压采样内外压差过大标志 [0:正常 1:压差过大]*/
	sSwhOnVExpFlag[clunum] = 0;                             /*电压判断接触器闭合异常标志 按位表示 Bit0:主接触器 bit1:预充接触器  bit2:负极接触器 [0:正常 1:异常]*/
	sSwhOffVExpFlag[clunum] = 0;                            /*电压判断接触器断开异常标志 按位表示 Bit0:主接触器 bit1:预充接触器  bit2:负极接触器 [0:正常 1:异常]*/
	sSwhOnCExpFlag[clunum] = 0;                             /*接触器带载闭合标志(按位表示各接触器)*/
	sSwhOffCExpFlag[clunum] = 0;                            /*接触器带载断开标志(按位表示各接触器)*/
	sAFESErrFlag[clunum] = 0;        

}

/*=================================================================================================
** @Name      : u8 DiagGetHigVoltExpFlagAPI(u8 clunum)
** @Input     : cluunum:簇编号
** @Output    : 按位表示高压采样值异常[bit0:总电压,bit1:预充电压]
** @Function  : 获取各高压采样值异常标志
** @The notes :
**===============================================================================================*/
u8 DiagGetHigVoltExpFlagAPI(u8 clunum)
{
	if(clunum >= 2)
		return (0);
	return(sHigVoltExpFlag[clunum]);
}

/*=================================================================================================
** @Name      : u8 DiagGetHigVSubDiffFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 高压内外压差过大标志[0:正常,1:压差过大]
** @Function  : 获取高压内外压差过大标志
** @The notes :
**===============================================================================================*/
u8 DiagGetHigVSubDiffFlagAPI(u8 clunum)
{
	if(clunum >= 2)
		return (0);
	return(sHVSubDiffFlag[clunum]);
}

/*=================================================================================================
** @Name      : u8 DiagGetSwhStateVExpFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 按位表示各接触器状态电压值异常[bit0:正极接触器断开状态,bit1:预充接触器断开状态,bit2:负极接触器断开状态;bit4:正极接触器闭合状态,bit5:预充接触器闭合状态,bit6:负极接触器闭合状态]
** @Function  : 获取各接触器状态电压值异常标志
** @The notes : 高4位闭合状态电压异常,低4位断开状态电压异常
**===============================================================================================*/
u8 DiagGetSwhStateVExpFlagAPI(u8 clunum)
{
	if(clunum >= 2)
		return (0);
	return((sSwhOnVExpFlag[clunum] << 4) | (sSwhOffVExpFlag[clunum] & 0x0f));
}

/*=================================================================================================
** @Name      : u8 DiagGetSwhChangCExpFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 按位表示各接触器动作带载异常[bit0:正极接触器断开,bit1:预充接触器断开,bit2:负极接触器断开;bit4:正极接触器闭合,bit5:预充接触器闭合,bit6:负极接触器闭合]
** @Function  : 获取各接触器带载动作异常标志
** @The notes : 高4位闭合带载异常,低4位断开带载异常
**===============================================================================================*/
u8 DiagGetSwhChangCExpFlagAPI(u8 clunum)
{
	if(clunum >= 2)
		return (0);
	return((sSwhOnCExpFlag[clunum] << 4) | (sSwhOffCExpFlag[clunum] & 0x0f));
}
/*=================================================================================================
** @Name      : u8 DiagGetRcvAllSlaveFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 接收到全部从控报文标志[0:未接收,1:正在接收,2:全部接收]
** @Function  : 获取接收到全部从控报文标志
** @The notes : 任一帧报文
**===============================================================================================*/
u8 DiagGetRcvAllSlaveFlagAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return 0;
	}

	//不检测从控通讯故障直接认为接收完成
	if(0 == DiagGetFilErrSortAPI(eERR_ID_INT_COFF))
	{
		return(2);
	}

	return(sRcvAllSlaveFlag[clunum]);
}
/*=================================================================================================
** @Name      : u8 DiagGetRcvAllPeripFlagAPI(void)
** @Input     : void
** @Output    : 接收到全部外设报文标志[0:未接收,1:正在接收,2:全部接收]
** @Function  : 获取接收到全部外设报文标志
** @The notes : 高压模块
**===============================================================================================*/
u8 DiagGetRcvAllPeripFlagAPI(void)
{
	//不检测内设通讯故障直接认为接收完成
	if(0 == DiagGetFilErrSortAPI(eERR_ID_PER_COFF))
	{
		return(2);
	}
	//后续优化
	return(2);
}
/*=================================================================================================
** @Name      : u32 DiagGetCellVOffCellBitAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体电压采样线断线的从控内电压通道(按位表示1个电压通道[0正常,1异常])
** @Function  : 获取单体电压采样线断线的从控内电压通道
** @The notes : 按位表示1个电压通道
**===============================================================================================*/
u32 DiagGetCellVOffCellBitAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return 0;
	}
    return(sCellVOffCellBit[clunum]);
}

/*=================================================================================================
** @Name      : u8 DiagGetCellTOffSlaveIdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体电压采样线断线的从控序号[1,32]
** @Function  : 获取均单体电压采样线断线从控序号
** @The notes : 只记录1个从控
**===============================================================================================*/
u8 DiagGetCellVOffSlaveIdAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return 0;
	}
	return(sCellVOffSlaveId[clunum]);
}

/*=================================================================================================
** @Name      : u16 DiagGetCellTOffCellBitAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体温度采样线断线的从控内温度点位(按位表示1个温度点[0正常,1异常])
** @Function  : 获取单体温度采样线断线的从控内温度点位
** @The notes : 按位表示1个温度点
**===============================================================================================*/
u16 DiagGetCellTOffCellBitAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return 0;
	}
    return(sCellTOffCellBit[clunum]);
}

/*=================================================================================================
** @Name      : u8 DiagGetCellTOffSlaveIdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 单体温度采样线断线的从控序号[1,32]
** @Function  : 获取单体温度采样线断线从控序号
** @The notes : 只记录1个从控
**===============================================================================================*/
u8 DiagGetCellTOffSlaveIdAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return 0;
	}
	return(sCellTOffSlaveId[clunum]);
}
/*=================================================================================================
** @Name      : u8 DiagGetRcvAllCellFlagAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 接收到全部单体信息报文标志[0:未接收,1:正在接收,2:全部接收]
** @Function  : 获取接收到全部单体信息报文标志
** @The notes : 全部单体电压和温度
**===============================================================================================*/
u8 DiagGetRcvAllCellFlagAPI(u8 clunum)
{
	if(clunum >= 2)
	{
		return 0;
	}
	//不检测从控通讯故障直接认为接收完成
	if(0 == DiagGetFilErrSortAPI(eERR_ID_INT_COFF))
	{
		return(2);
	}

	return(sRcvAllCellFlag[clunum]);
}
/*=================================================================================================
** @Name      : u8 DiagGetRcvPerOffCommuBitAPI(void)
** @Input     : void
** @Output    : 内部外设子系统通信故障位(按位表示1类子设备:Bit0：IMEU3通信中断 BIT1: 1#IMIU4通信中断 BIT2:2#IMIU4通信中断)
** @Function  : 获取通信中断的内部子系统号
** @The notes : 按BIT位表示
**===============================================================================================*/
u8 DiagGetRcvPerOffCommuBitAPI(void)
{
	return(sPerOffCommuBit);
}
/*=================================================================================================
** @Name      : u8 DiagGetInImeu3OffCommuStateAPI(void)
** @Input     : void
** @Output    : IMEU3通信中断状态[0:正常 1:通信中断]
** @Function  : 获取IMEU3通信中断状态
** @The notes :
**===============================================================================================*/
u8 DiagGetInImeu3OffCommuStateAPI(void)
{
	return(BitGet(sPerOffCommuBit, 0));
}

/*=================================================================================================
** @Name      : u8 DiagGetRcvSlaveMsgSlaveIdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 未接收到从控信息报文的从控序号[1,32]
** @Function  : 获取未接收到从控信息报文从控序号
** @The notes : 只记录1个从控
**===============================================================================================*/
u8 DiagGetRcvSlaveMsgSlaveIdAPI(u8 clunum)
{
	u8 i = 0;
	u8 slaNum = 0;

	if(clunum >= 2)
	{
		return (0);
	}
	/*接收到全部从控信息*/
	if(2 == sRcvAllSlaveFlag[clunum])
	{
		return(0);
	}

	//slaNum = (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] + gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] - 1) / gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];
	/*未接收完从控信息*/
	//for(i = 0; i < gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum]; i++)
	slaNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
	if(slaNum >= 8)
	{
		slaNum = 8;
		gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] = 8;
	}
	for(i = 0; i < slaNum; i++)
	{
		if(0 == sRcvIntCount[i + clunum * slaNum])
		{
			return(i + 1);
		}
	}

	return(0);	
}
/*=================================================================================================
** @Name      : void DiagCheckCellVOffLFlagAPI(u8 clunum, u8 slave, u8 cellNum, u16 volt)
** @Input     : clunum:簇编号 slave:从控序号[0,31] cellNum:从控内电池号[0,17] volt:电压值[0,5000]mv
** @Output    : void
** @Function  : 检测单体电压采样断线标志
** @The notes : 采样电压判断
**===============================================================================================*/
void DiagCheckCellVOffLFlagAPI(u8 clunum, u8 slave, u8 cellNum, u16 volt)
{
	//static u32 sRcdFlag = 0;事件记录后续再实现

	if(clunum >= 2)
	{
		return;
	}
	/*从控序号或电池号错误*/
	if((slave >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
		|| (cellNum >= (gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum])))
    {
        return;
    }

	//兼容CATL液冷PACK电池52个电压解析
	#if(4 == PRJ_PACK_CELL_INFO)
	{
		//每3个从控首个少2S(仅前16个温度有效)
		if((0 == (slave % 3)) && (cellNum >= 16))
		{
			return;
		}
	}
	#endif

    if(((0 == sCellVOffSlaveId[clunum]) && (0 == sCellVOffCellBit[clunum]))     /*当前无断线记录*/
        || (sCellVOffSlaveId[clunum] == (slave + 1)))                   /*已记录故障的从控*/
    {
        if((volt <= DEFAULT_LO_MINV)          					/*小于断线低压点阈值*/
            || (volt >= DEFAULT_LO_MAXV))     					/*大于断线高压点阈值*/
        {
            sCellVOffSlaveId[clunum] = slave + 1;                       /*记录芯片检测单体电压采样断线的从控号*/

    		//新产生
    		if(0 == BitGet(sCellVOffCellBit[clunum], cellNum))
    		{
    			BitSet(sCellVOffCellBit[clunum], cellNum);              /*记录芯片检测单体电压采样断线的从控内电压通道*/

        		//记录事件
    			//DiagUserSelfErrRecord(eUDef_ErrRcd_CellVErr, eERR_RCDSORT_WARSET, volt, 0);
    		}
    		//重复产生
    		else
    		{
    			//未记录事件
    			//if(0 == BitGet(sRcdFlag, cellNum))
    			//{
    				//记录事件
    				//DiagUserSelfErrRecord(eUDef_ErrRcd_CellVErr, eERR_RCDSORT_WARSET, volt, 0);
    			//	BitSet(sRcdFlag, cellNum);
    			//}
    		}
        }
        else
        {
            BitClr(sCellVOffCellBit[clunum], cellNum);                  /*记录芯片检测单体电压采样断线的从控内电压通道*/
            //BitClr(sRcdFlag, cellNum);

        	if(0 == sCellVOffCellBit[clunum])                           /*该从控无故障*/
        	{
        		sCellVOffSlaveId[clunum] = 0;                           /*清除从控记录*/
        	}
        }
    }
}

/*=================================================================================================
** @Name      : u8 DiagGetRcvCellInfoSlaveIdAPI(u8 clunum)
** @Input     : clunum:簇编号
** @Output    : 未接收到单体信息的从控序号[1,32]
** @Function  : 获取未接收到单体信息从控序号
** @The notes : 只记录1个从控
**===============================================================================================*/
u8 DiagGetRcvCellInfoSlaveIdAPI(u8 clunum)
{
	/*接收到全部单体信息*/
	if(2 == sRcvAllCellFlag[clunum])
	{
		return(0);
	}
	/*接收完单体电压信息*/
	else if(sRcvAllCellVNum[clunum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_VSumNum])
	{
		/*接收完单体温度信息*/
		if(sRcvAllCellTNum[clunum] >= gGBmuHigLevPara_103[eBmuHigLevPara103_TSumNum])
		{
			return(0);
		}
		/*未接收完单体温度信息*/
		else
		{
			/*返回未接收到温度的从控序号*/
			return(sRcvAllCellTNum[clunum] / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] + 1);
		}
	}
	/*未接收完单体电压信息*/
	else
	{
		/*返回未接收到电压的从控序号*/
		return(sRcvAllCellVNum[clunum] / gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum] + 1);
	}
}

/*=================================================================================================
** @Name      : void DiagCheckCellTOffLFlagAPI(u8 clunum, u8 slave, u8 cellNum, s8 temp)
** @Input     : clunum:簇编号 slave:从控序号[0,31] cellNum:从控内单体温度序号[0,15] temp:温度值[-40,105]℃
** @Output    : void
** @Function  : 检测单体温度采样断线标志
** @The notes : 单体电芯温度
**===============================================================================================*/
void DiagCheckCellTOffLFlagAPI(u8 clunum, u8 slave, u8 cellNum, s8 temp)
{
	//static u16 sRcdFlag = 0;事件记录后续再实现

	if(clunum >= 2)
	{
		return;
	}

	/*从控序号或温度序号错误*/
	if((slave >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum])
		|| (cellNum >= gGBmuHigLevPara_103[eBmuHigLevPara103_BsuTNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum]))
    {
        return;
    }

	//兼容CATL液冷PACK电池8个温度解析
	#if(4 == PRJ_PACK_CELL_INFO)
	{
		//每3个从控首个少1S(仅前2个温度有效)
		if((0 == (slave % 3)) && (cellNum >= 2))
		{
			return;
		}
	}
	#endif

	if(((0 == sCellTOffSlaveId[clunum]) && (0 == sCellTOffCellBit[clunum]))     /*当前无断线记录*/
		|| (sCellTOffSlaveId[clunum] == (slave + 1)))                   /*已记录故障的从控*/
	{
		if((temp <= -41)          					            /*小于断线点温度阈值*/
			|| (temp >= 126))     					            /*大于短路点温度阈值*/
		{
			sCellTOffSlaveId[clunum] = slave + 1;                       /*记录芯片检测单体电压采样断线的从控号*/

    		//新产生
    		if(0 == BitGet(sCellTOffCellBit[clunum], cellNum))
    		{
    			BitSet(sCellTOffCellBit[clunum], cellNum);              /*记录芯片检测单体电压采样断线的从控内温度通道*/

        		//记录事件
    			//DiagUserSelfErrRecord(eUDef_ErrRcd_CellTErr, eERR_RCDSORT_WARSET, (u16)temp, 0);
    		}
    		//重复产生
    		else
    		{
    			//未记录事件
    			//if(0 == BitGet(sRcdFlag, cellNum))
    			//{
    				//记录事件
    				//DiagUserSelfErrRecord(eUDef_ErrRcd_CellTErr, eERR_RCDSORT_WARSET, (u16)temp, 0);
    			//	BitSet(sRcdFlag, cellNum);
    			//}
    		}
		}
		else
		{
			BitClr(sCellTOffCellBit[clunum], cellNum);                  /*清除芯片检测单体温度采样断线的从控内温度通道*/
			//BitClr(sRcdFlag, cellNum);

			if(0 == sCellTOffCellBit[clunum])                           /*该从控无故障*/
			{
				sCellTOffSlaveId[clunum] = 0;                           /*清除从控记录*/
			}
		}
	}
}
/*=================================================================================================
** @Name      : void DiagPerDevCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测外围设备故障状态任务
** @The notes : 500ms周期调用
**===============================================================================================*/
void DiagPerDevCheckTask(void)
{
	u16 sumVolt = 0;
    u16 outVolt = 0;

	for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
	{
		if(clunum >= 2)
		{
			return;
		}
		sumVolt = GetGSampSumVoltAPI(clunum);
    	outVolt = GetGSampPreVoltAPI(clunum);
		//内外高压压差检测(判断是否内外压差过大不能上高压)
    	//非接触器变化状态(预充电压不确定)
    	//非快充枪插入请求停机(充电枪输出电压自检)
    	//总电压和外端电压均正常(单组运行不受其他组影响)
    	//if((eWORK_START != GetGWorkStateAPI())
        //&& (eWORK_STOP != GetGWorkStateAPI())
		if((eWORK_RUN == GetGWorkStateAPI(clunum))
			//&& (0 == GetFstChgStopReqFlagAPI(clunum))
			//&& (gSysGenParaRO_100[eSGenPara100_AllGNum] <= 1)
			&& (sumVolt >= gGBmuHigLevPara_103[eBmuHigLevPara103_HigVOffV])
			&& (outVolt >= gGBmuHigLevPara_103[eBmuHigLevPara103_MSwhOffV]))
		{
			//内外压差超过安全启动压差
			if(ABS(sumVolt, outVolt) > gGBmuHigLevPara_103[eBmuHigLevPara103_SafeSubV])
			{
				//内外电压差异过大异常
				sHVSubDiffFlag[clunum] = 1;
			}
			//总电压或外端电压采样无电压
			else if(ABS(sumVolt, outVolt) <= (gGBmuHigLevPara_103[eBmuHigLevPara103_SafeSubV] - 15))
			{
				//无内外电压差异过大异常
				sHVSubDiffFlag[clunum] = 0;
			}
		}
		//总电压或外端电压采样无电压
		else
		{
			//无内外电压差异过大异常
			sHVSubDiffFlag[clunum] = 0;
		}
	}
	//后续再补充外设及绝缘检测相关故障判断
}
/*=============================================================================================
** @Name      : void DiagSwhChangCStateCheckTask(void)
** @Input     : void
** @Output    : void
** @Function  : 检测接触器动作时电流状态(是否带载动作)
** @The notes : 100ms调用,检测主正/主负接触器
**===========================================================================================*/
void DiagSwhChangCStateCheckTask(void)
{
    static u8 sMaiOnTime[2] = {0};
    static u8 sNegOnTime[2] = {0};
    static u8 sHisSwhState[2] = {0};
    static s16 sHisOutCurr[2] = {0};
    static u16 sTime[2] = {0};

	for(u8 clunum = 0; clunum < gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum]; clunum++)
	{
		if(clunum >= 2)
		{
			return;
		}
		if(clunum == 0)
		{
			/*主正接触器闭合,后续改为使用反馈点判断是否真的闭合或断开*/
			if(ePSTATE_ON == TacGetGroupSwhCtrlStateAPI(eGroupSwhPK1_1Mai))
			{
				/*历史状态断开且当前有不安全电流*/
				if((0 == BitGet(sHisSwhState[clunum], eGroupSwhPK1_1Mai))
					&& (ABS(GetGSampOutCurrAPI(clunum), 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载闭合*/
					BitSet(sSwhOnCExpFlag[clunum], eGroupSwhPK1_1Mai);
				}

				/*清除主正接触器带载断开告警*/
				BitClr(sSwhOffCExpFlag[clunum], eGroupSwhPK1_1Mai);

				/*闭合连续检测1s*/
				if(sMaiOnTime[clunum] >= 10)
				{
					/*记录确认主正接触器闭合历史状态*/
					BitSet(sHisSwhState[clunum], eGroupSwhPK1_1Mai);
				}
				else
				{
					sMaiOnTime[clunum]++;
				}
			}
			/*主正接触器断开*/
			else
			{
				/*历史状态闭合且有不安全历史电流*/
				if((1 == BitGet(sHisSwhState[clunum], eGroupSwhPK1_1Mai))
					&& (ABS(sHisOutCurr[clunum], 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载断开*/
					BitSet(sSwhOffCExpFlag[clunum], eGroupSwhPK1_1Mai);
				}

				/*清除主正接触器带载闭合告警*/
				BitClr(sSwhOnCExpFlag[clunum], eGroupSwhPK1_1Mai);

				/*记录主正接触器断开历史状态*/
				BitClr(sHisSwhState[clunum], eGroupSwhPK1_1Mai);
				sMaiOnTime[clunum] = 0;
			}

			//有负极接触器
			#if(1 == BMU_NEG_SWH_EN)
			/*主负接触器闭合*/
			if(ePSTATE_ON == TacGetGroupSwhCtrlStateAPI(eGroupSwhPK2_1Neg))
			{
				/*历史状态断开且当前有不安全电流*/
				if((0 == BitGet(sHisSwhState[clunum], eGroupSwhPK2_1Neg))
					&& (ABS(GetGSampOutCurrAPI(clunum), 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载闭合*/
					BitSet(sSwhOnCExpFlag[clunum], eGroupSwhPK2_1Neg);
				}

				/*清除主正接触器带载断开告警*/
				BitClr(sSwhOffCExpFlag[clunum], eGroupSwhPK2_1Neg);

				/*闭合连续检测1s*/
				if(sNegOnTime[clunum] >= 10)
				{
					/*记录主正接触器闭合历史状态*/
					BitSet(sHisSwhState[clunum], eGroupSwhPK2_1Neg);
				}
				else
				{
					sNegOnTime[clunum]++;
				}
			}
			/*负极接触器断开*/
			else
			{
				/*历史状态闭合且有不安全历史电流*/
				if((1 == BitGet(sHisSwhState[clunum], eGroupSwhPK2_1Neg))
					&& (ABS(sHisOutCurr[clunum], 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载断开*/
					BitSet(sSwhOffCExpFlag[clunum], eGroupSwhPK2_1Neg);
				}

				/*清除主正接触器带载闭合告警*/
				BitClr(sSwhOnCExpFlag[clunum], eGroupSwhPK2_1Neg);

				/*记录负极接触器断开历史状态*/
				BitClr(sHisSwhState[clunum], eGroupSwhPK2_1Neg);
				sNegOnTime[clunum] = 0;
			}
			#endif
		}
		//第2簇
		else
		{
			/*主正接触器闭合,后续改为使用反馈点判断是否真的闭合或断开,故障位按簇1BIT上报*/
			if(ePSTATE_ON == TacGetGroupSwhCtrlStateAPI(eGroupSwhPK3_2Mai))
			{
				/*历史状态断开且当前有不安全电流*/
				if((0 == BitGet(sHisSwhState[clunum], eGroupSwhPK1_1Mai))
					&& (ABS(GetGSampOutCurrAPI(clunum), 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载闭合*/
					BitSet(sSwhOnCExpFlag[clunum], eGroupSwhPK1_1Mai);
				}

				/*清除主正接触器带载断开告警*/
				BitClr(sSwhOffCExpFlag[clunum], eGroupSwhPK1_1Mai);

				/*闭合连续检测1s*/
				if(sMaiOnTime[clunum] >= 10)
				{
					/*记录确认主正接触器闭合历史状态*/
					BitSet(sHisSwhState[clunum], eGroupSwhPK1_1Mai);
				}
				else
				{
					sMaiOnTime[clunum]++;
				}
			}
			/*主正接触器断开*/
			else
			{
				/*历史状态闭合且有不安全历史电流*/
				if((1 == BitGet(sHisSwhState[clunum], eGroupSwhPK1_1Mai))
					&& (ABS(sHisOutCurr[clunum], 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载断开*/
					BitSet(sSwhOffCExpFlag[clunum], eGroupSwhPK1_1Mai);
				}

				/*清除主正接触器带载闭合告警*/
				BitClr(sSwhOnCExpFlag[clunum], eGroupSwhPK1_1Mai);

				/*记录主正接触器断开历史状态*/
				BitClr(sHisSwhState[clunum], eGroupSwhPK1_1Mai);
				sMaiOnTime[clunum] = 0;
			}

			//有负极接触器
			#if(1 == BMU_NEG_SWH_EN)
			/*主负接触器闭合*/
			if(ePSTATE_ON == TacGetGroupSwhCtrlStateAPI(eGroupSwhPK4_2Neg))
			{
				/*历史状态断开且当前有不安全电流*/
				if((0 == BitGet(sHisSwhState[clunum], eGroupSwhPK2_1Neg))
					&& (ABS(GetGSampOutCurrAPI(clunum), 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载闭合*/
					BitSet(sSwhOnCExpFlag[clunum], eGroupSwhPK2_1Neg);
				}

				/*清除主正接触器带载断开告警*/
				BitClr(sSwhOffCExpFlag[clunum], eGroupSwhPK2_1Neg);

				/*闭合连续检测1s*/
				if(sNegOnTime[clunum] >= 10)
				{
					/*记录主正接触器闭合历史状态*/
					BitSet(sHisSwhState[clunum], eGroupSwhPK2_1Neg);
				}
				else
				{
					sNegOnTime[clunum]++;
				}
			}
			/*负极接触器断开*/
			else
			{
				/*历史状态闭合且有不安全历史电流*/
				if((1 == BitGet(sHisSwhState[clunum], eGroupSwhPK2_1Neg))
					&& (ABS(sHisOutCurr[clunum], 0) > gGBmuHigLevPara_103[eBmuHigLevPara103_MainSwhC]))
				{
					/*记录主正接触器带载断开*/
					BitSet(sSwhOffCExpFlag[clunum], eGroupSwhPK2_1Neg);
				}

				/*清除主正接触器带载闭合告警*/
				BitClr(sSwhOnCExpFlag[clunum], eGroupSwhPK2_1Neg);

				/*记录负极接触器断开历史状态*/
				BitClr(sHisSwhState[clunum], eGroupSwhPK2_1Neg);
				sNegOnTime[clunum] = 0;
			}
			#endif
		}

		/*记录历史电流*/
		sHisOutCurr[clunum] = GetGSampOutCurrAPI(clunum);

		/*告警30s*/
		if(((eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SWHON_CH))
			|| (eErr_B == DiagGetErrLastStateAPI(clunum, eERR_ID_SWHOFF_CH)))
			&& ((sSwhOnCExpFlag[clunum] > 0) || (sSwhOffCExpFlag[clunum] > 0)))
		{
			if(sTime[clunum] >= 300)
			{
				/*延时30s清除告警*/
				sSwhOnCExpFlag[clunum] = 0;
				sSwhOffCExpFlag[clunum] = 0;
			}
			else
			{
				sTime[clunum]++;
			}
		}
		else
		{
			sTime[clunum] = 0;
		}
	}
}

/*=================================================================================================
** @Name      : void DiagJudgeBsu02u1FloodErrorAPI(u8 ecu,u8 id,u16 data)
** @Input     : ecu:绝对节点号[0,15], id:通道[0,1], data:水浸频率
** @Output    : void
** @Function  : 获取从控水浸异常标志(按位表示1个通道)
** @The notes : 支持两簇架构，按位表示1个通道
**===============================================================================================*/
void DiagJudgeBsu02u1FloodErrorAPI(u8 ecu, u8 id, u16 data)
{

    u16 totalPackNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    
    /* 参数越界安全拦截 */
    if(ecu >= totalPackNum || id >= BSU02U1_FLOOD_NUM)
    {
        return;
    }
    
    if((data < BMU_EU_FLOOD_HZ) &&  (data > 0))
    {
        BitSet(sBsu02u1FloodErrBit, ecu * BSU02U1_FLOOD_NUM + id);
    }
    else
    {
        BitClr(sBsu02u1FloodErrBit, ecu * BSU02U1_FLOOD_NUM + id);
    }
}

/*=================================================================================================
** @Name      : void DiagJudgeBsu02u1TemperErrorAPI(u8 ecu,u8 id,s16 data)
** @Input     : ecu:绝对节点号[0,15], id:通道[0,1], data:温度
** @Output    : void
** @Function  : 获取从控PT1000异常标志(按位表示1个通道)
** @The notes : 支持两簇架构，带回差判断
**===============================================================================================*/
void DiagJudgeBsu02u1TemperErrorAPI(u8 ecu, u8 id, s16 data)
{
    u16 totalPackNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    
    /* 参数越界安全拦截 */
    if(ecu >= totalPackNum || id >= BSU02U1_PT1000_NUM)
    {
        return;
    }
    
    if(data > (s16)gGHardPara_104[eGHardPara104_PackConnTH])
    {
        BitSet(sBsu02u1PT1000ErrBit, ecu * BSU02U1_PT1000_NUM + id);
    }
    else if(data < (s16)gGHardPara_104[eGHardPara104_PackConnTH] - 5)
    {
        BitClr(sBsu02u1PT1000ErrBit, ecu * BSU02U1_PT1000_NUM + id);
    }
}

/*=================================================================================================
** @Name      : void DiagJudgeBsu02u1AeroSenErrorAPI(u8 ecu,u16 data)
** @Input     : ecu:绝对节点号[0,15], data:浓度
** @Output    : void
** @Function  : 获取从控气溶胶传感器异常标志(按机器记录)
** @The notes : 支持两簇架构
**===============================================================================================*/
void DiagJudgeBsu02u1AeroSenErrorAPI(u8 ecu, u16 data)
{
    u16 totalPackNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    
    /* 参数越界安全拦截 */
    if(ecu >= totalPackNum)
    {
        return;
    }
    
    if(data > BSU02U1_AERO_ALARM + 1000) 
    {
        BitSet(sBsu02u1AeroSenErrBit, ecu);
    }
    else if(data < BSU02U1_AERO_ALARM)
    {
        BitClr(sBsu02u1AeroSenErrBit, ecu);
    }
}

/*=================================================================================================
** @Name      : void DiagJudgeBsu02u1AfeEnumErrorAPI(u8 ecu,u16 botcount, u16 topcount)
** @Input     : ecu:从机节点号, botcount:bot端枚举数据, topcount:top端枚举数目
** @Output    : void
** @Function  : 判断从控afe枚举计数异常(按机器记录)
** @The notes : 支持两簇架构
**===============================================================================================*/
void DiagJudgeBsu02u1AfeEnumErrorAPI(u8 ecu, u16 botcount, u16 topcount)
{
    u16 totalPackNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    
    /* 参数越界安全拦截 */
    if(ecu >= totalPackNum)
    {
        return;
    }
    
    /* 采样两端中间断线或BOT端采样不满足数目 */
    if(topcount > 0 || botcount != gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]) 
    {
        BitSet(sBsu02u1AfeEnumErrbit, ecu);
    }
    else 
    {
        BitClr(sBsu02u1AfeEnumErrbit, ecu);
    }
}

/*=================================================================================================
** @Name      : void DiagJudgeBsu02u1MsdErrorAPI(u8 ecu,u16 amplitute, u16 duty)
** @Input     : ecu:从机节点号, amplitute:幅值, duty:占空比
** @Output    : void
** @Function  : 获取MSD异常标志(按机器记录)
** @The notes : 支持两簇架构
**===============================================================================================*/
void DiagJudgeBsu02u1MsdErrorAPI(u8 ecu, u16 amplitute, u16 duty)
{
    u16 totalPackNum = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum] * gGBmuHigLevPara_103[eBmuHigLevPara103_ClusterNum];
    
    /* 参数越界安全拦截 */
    if(ecu >= totalPackNum)
    {
        return;
    }

    if((amplitute >= BSU02U1_MSDAMP_ALARM1) && (amplitute <= BSU02U1_MSDAMP_ALARM2) && (duty >= BSU02U1_MSDDUTY_ALARM1) && (duty <= BSU02U1_MSDDUTY_ALARM2))
    {
        BitClr(sBsu02u1MsdErrBit, ecu);
    }
    else
    {
        BitSet(sBsu02u1MsdErrBit, ecu);
    }
}

/*=================================================================================================
** @Name      : u32 DiagGetBsu02u1FloodErrorBitAPI(void)
** @Input     : void
** @Output    : 从控水浸信息异常标志(按位表示1个通道[0,1])
** @Function  : 获取从控水浸异常标志(按位表示1个通道[0,1])
** @The notes : 按位表示1个通道
**===============================================================================================*/
u32 DiagGetBsu02u1FloodErrorBitAPI(void)
{
	return (sBsu02u1FloodErrBit);
}
/*=================================================================================================
** @Name      : u32 DiagGetBsu02u1TemperErrorBitAPI(void)
** @Input     : void
** @Output    : 从控PT1000温度信息异常标志(按位表示1个通道[0,1])
** @Function  : 获取从控PT1000温度异常标志(按位表示1个通道[0,1])
** @The notes : 按位表示1个通道
**===============================================================================================*/
u32 DiagGetBsu02u1TemperErrorBitAPI(void)
{
	return (sBsu02u1PT1000ErrBit);
}
/*=================================================================================================
** @Name      : u32 DiagGetBsu02u1AeroErrorBitAPI(void)
** @Input     : void
** @Output    : 从控气溶胶浓度异常标志(按位表示1个通道[0,1])
** @Function  : 获取气溶胶浓度异常标志(按位表示1个通道[0,1])
** @The notes : 按位表示1个通道
**===============================================================================================*/
u32 DiagGetBsu02u1AeroErrorBitAPI(void)
{
	return (sBsu02u1AeroSenErrBit);
}
/*=================================================================================================
** @Name      : u32 DiagGetBsu02u1MsdErrorBitAPI(void)
** @Input     : void
** @Output    : 从控MSD异常标志(按位表示1个通道[0,1])
** @Function  : 获取MSD异常标志(按位表示1个通道[0,1])
** @The notes : 按位表示1个通道
**===============================================================================================*/
u32 DiagGetBsu02u1MsdErrorBitAPI(void)
{
	return (sBsu02u1MsdErrBit);
}
/*=================================================================================================
** @Name      : u32 DiagGetBsu02u1AfeEnumErrorBitAPI(void)
** @Input     : void
** @Output    : 从控MSD异常标志(按位表示1个通道[0,1])
** @Function  : 获取MSD异常标志(按位表示1个通道[0,1])
** @The notes : 按位表示1个通道
**===============================================================================================*/
u32 DiagGetBsu02u1AfeEnumErrorBitAPI(void)
{
	return (sBsu02u1AfeEnumErrbit);
}
/*=================================================================================================
** @Name      : u32 DiagGetBalTOffCellBitAPI(void)
** @Input     : void
** @Output     : 均衡温度断线单体位图(按位表示1节电池)
** @Function  : 获取均衡温度采样断线的单体位图
** @The notes : 836无均衡板温采样，ID160仅保留协议位号；本接口固定返回0。
**===============================================================================================*/
u32 DiagGetBalTOffCellBitAPI(void)
{
	return (0);
}
/*=================================================================================================
** @Name      : u32 DiagGetBalErrCellBitAPI(void)
** @Input     : void
** @Output     : 均衡控制异常簇位图(bit0=簇1，bit1=簇2)
** @Function  : 获取均衡命令与从控状态回报不一致的簇位图
** @The notes : 请求状态来自BalanTactic，回报状态来自从控状态报文；连续1s不一致才置位。
**===============================================================================================*/
u32 DiagGetBalErrCellBitAPI(void)
{
    u32 errBit = 0U;
    u8 bsuNum = (u8)gGBmuHigLevPara_103[eBmuHigLevPara103_BsuNum];
    u16 cellPerBsu = gGBmuHigLevPara_103[eBmuHigLevPara103_BsuVNum]
        * gGBmuHigLevPara_103[eBmuHigLevPara103_ConNum];

    if((bsuNum == 0U) || (cellPerBsu == 0U))
    {
        return 0U;
    }

    for(u8 clunum = 0U; clunum < CLUTER_MAX_NUM; clunum++)
    {
        for(u8 slave = 0U; (slave < bsuNum) && (slave < MAX_BSU_NUM); slave++)
        {
            u8 mismatch = 0U;
            u64 reqState = GetGroupVoltPasBalanReqState((u8)(clunum * bsuNum + slave));

            if(GetGDhgBalanStateRcvAPI(clunum, slave) == 0U)
            {
                /*无回报时，仅在已有均衡请求的情况下视为控制失败，避免上电无请求误报。*/
                mismatch = (reqState != 0U) ? 1U : 0U;
            }
            else
            {
                for(u8 cell = 0U; (cell < cellPerBsu) && (cell < 64U); cell++)
                {
                    u8 req = (u8)((reqState >> cell) & 0x01U);
                    u8 actual = GetGDhgBalanCellStateAPI(clunum, (u16)(slave * cellPerBsu + cell));
                    if(req != actual)
                    {
                        mismatch = 1U;
                        break;
                    }
                }
            }

            if(mismatch != 0U)
            {
                if(sBalanStateMismatchCount[clunum][slave] < 10U)
                {
                    sBalanStateMismatchCount[clunum][slave]++;
                }
            }
            else
            {
                sBalanStateMismatchCount[clunum][slave] = 0U;
            }

            if(sBalanStateMismatchCount[clunum][slave] >= 10U)
            {
                errBit |= (1UL << clunum);
            }
        }
    }

    return errBit;
}
