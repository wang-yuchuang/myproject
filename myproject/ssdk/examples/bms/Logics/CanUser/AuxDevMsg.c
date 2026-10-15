/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : AuxDevMsg.c
** @Founder      : ZQD808
** @Date         : 2026.01.01
** @Function     : 电池组与辅助设备通信CAN报文处理模块
** @Instructions : 
**===============================================================================================*/
#include "AuxDevMsg.h"
#include "BSPUserHook.h"
#include "BSPConfig.h"
#include "BSPTypeDef.h"
#include "GroupPara.h"
#include "GroupInfo.h"
#include "CurrSample.h"
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
#include "IMEU3Interface.h"
#include "IMIU4Interface.h"
#include "IMIU4Protocol.h"
extern u16 gGBmuGenPara_102[];  	 /*主控通用参数值*/
extern u16 gGBmuHigLevPara_103[]; 	 /*主控高级参数值*/
extern u8 gGHardPara_104[];          /*主控硬件参数配置信息*/

extern u8  *gImeu3BoardInfo1;             /*IMEU3板级信息1*/
extern u16 *gImeu3BoardInfo2;             /*IMEU3板级信息2*/
extern u16 *gImeu3AlmInfo;                /*IMEU3告警信息*/
extern gImiu4_data *gImiu4_Info[e_IMIU4_NUM]; 
/*=================================================================================================
** @Name      : void CanRcvImeu3MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : CAN通信IMEU3接收处理任务
** @The notes : 用户放在CAN接收处理HOOK接口调用
**===============================================================================================*/
void CanRcvImeu3MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
{
    IMEU3_RcvHandle(ID, pdata, DLC, canNum);
    return;

    u8 i = 0;
	u8 saddr = 0;
	u8 daddr = 0;
	u8 index = 0;
	u8 funct = 0;
	/*解析CAN帧ID*/
	saddr = ID & 0xff;
	daddr = (ID >> 8) & 0xff;
	index = (ID >> 16) & 0xff;
	funct = (ID >> 25) & 0x0f;

    /*非IMEU3报文,非发送给BMU报文,非对应CAN通道*/
	if((canNum != eCAN2) || ((IMEU3_PDUIN_DEVADDR != saddr) && (IMEU3_PDUOUT_DEVADDR != saddr))
    || ((gGBmuGenPara_102[eBmuGenPara102_BmuDev] != daddr) && (0xff  != daddr))
    || (DLC < 8) || ((funct != 10) && (funct !=8)))
    {
        return;
    }
	/*指针未分配空间*/
	if((gImeu3BoardInfo1 == NULL) || (gImeu3BoardInfo2 == NULL) || (gImeu3AlmInfo == NULL))
		return;
	switch(index)
	{
		/*板级采样信息1,暂时先把收据都接收，后续做数据阈值判断*/
		case 51:
		for(i = 0; i < eIMEUBoardInfo1_Len;i++)
		{
			gImeu3BoardInfo1[i] = (u16)((pdata[2 * i + 1] << 8) | (pdata[2 * i]));
		}
		break;
		/*板级采样信息2,暂时先把收据都接收，后续做数据阈值判断*/
		case 52:
		for(i = 0; i < eIMEUBoardInfo2_Len;i++)
		{
			gImeu3BoardInfo2[i] = (u16)((pdata[2 * i + 1] << 8) | (pdata[2 * i]));
		}
		break;

		/*故障信息,暂时先把数据都接收，后续做数据阈值判断*/
		case 53:
		for(i = 0; i < eIMEUAlmInfo_Len;i++)
		{
			gImeu3AlmInfo[i] = (u16)((pdata[2 * i + 1] << 8) | (pdata[2 * i]));
		}
		break;

		default:
		break;
	}
}

/*=================================================================================================
** @Name      : CanRcvImiu4MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
** @Input     : ID:CANID *pdata:CAN数据域  DLC:数据域长度 canNum:CAN通道号
** @Output    : void
** @Function  : CAN通信IMIU4接收处理任务
** @The notes : 用户放在CAN接收处理HOOK接口调用,调用IMIU4模块接口处理接收数据
**===============================================================================================*/
void CanRcvImiu4MsgHandle_API(u32 ID, u8 *pdata, u8 DLC, u8 canNum)
{
    /*调用IMIU4模块接收处理接口*/
    IMIU4_RcvHandle(ID, pdata, DLC, canNum);
}
/*=================================================================================================
** @Name      : u32 CanBuildRupAuxMsgID(u8 func, u8 index, u8 dadrr, u8 saddr)
** @Input     : func:功能码 index:主索引 dadrr:目标地址 saddr:源地址
** @Output    : CAN报文ID
** @Function  : 用户创建上传CAN报文ID
** @The notes : 电池组内部下行报文
**===============================================================================================*/
u32 CanBuildRupAuxMsgID(u8 func, u8 index, u8 dadrr, u8 saddr)
{
	u32 id = 0;

	id = ((u32)func << 25) | ((u32)1 << 24) | ((u32)index << 16) | ((u32)dadrr << 8) | saddr;

	return(id);
}

/*=================================================================================================
** @Name      : void SendCanImeu3PwmCtrlMsg(u8 addr,u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：主索引  data：数据
** @Output    : void
** @Function  : 发送IMEU3PWM控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3PwmCtrlMsg(u8 addr,u8 index, u8 data)
{
	IMEU3_SendPwmCtrl(addr, index, data);
}
/*=================================================================================================
** @Name      : void SendCanImeu3DoCtrlMsg(u8 addr, u8 ctrlnum, u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：DO控制开始序号  data：数据
** @Output    : void
** @Function  : 发送IMEU3DO控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3DoCtrlMsg(u8 addr, u8 ctrlnum, u8 index, u8 data)
{
	IMEU3_SendDoCtrl(addr, ctrlnum, index, data);
}

/*=================================================================================================
** @Name      : void SendCanImeu3MotorCtrlMsg(u8 addr, u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：主索引  data：数据
** @Output    : void
** @Function  : 发送IMEU3泄爆阀控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3MotorCtrlMsg(u8 addr, u8 index, u8 data)
{
	IMEU3_SendMotorCtrl(addr, index, data);
}

/*=================================================================================================
** @Name      : void SendCanImeu3OCCtrlMsg(u8 addr, u8 index, u8 data)
** @Input     : addr:IMEU3通信地址，index：主索引  data：数据
** @Output    : void
** @Function  : 发送IMEU3OC门控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3OCCtrlMsg(u8 addr, u8 index, u8 data)
{
	IMEU3_SendOcCtrl(addr, index, data);
}

/*=================================================================================================
** @Name      : void SendCanImeu3DeCtrlMsg(u8 addr, u8 openhum, u8 closehum, u8 mode)
** @Input     : addr:IMEU3通信地址，openhum:开启除湿湿度 closehum: 关闭除湿湿度 mode:工作模式
** @Output    : void
** @Function  : 发送IMEU3除湿机控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImeu3DeCtrlMsg(u8 addr, u8 openhum, u8 closehum, u8 mode)
{
	IMEU3_SendDeCtrl(addr, openhum, closehum, mode);
}

/*=================================================================================================
** @Name      : void SendCanImeu3DeCtrlMsg(u8 addr, u8 openhum, u8 closehum, u8 mode)
** @Input     : addr:IMIU4通信地址，readynum:继电器选择  state:继电器状态
** @Output    : void
** @Function  : 发送IMIU4继电器控制报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImiu4ReadyCtrlMsg(u8 addr, u8 readynum, u8 state)
{
	can_frame_t msg;

	//参数有效
	if(readynum > 2)
	{
		return;
	}
	if(state > 1)
	{
		return;
	}
	msg.id = CanBuildRupAuxMsgID(2, 55, addr, (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev]);
    msg.fd = 1;
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
	msg.data[0] = 0x0c;
	msg.data[1] = 0x01;
	msg.data[2] = 0x01;
	msg.data[3] = 0x00;  //预留 
	msg.data[4] = readynum;  
	msg.data[5] = state;  
	msg.len = 6;

	BSPCANSendQin(eCAN2, &msg);
}

/*=================================================================================================
** @Name      : void SendCanImiu4InsuCtrlMsg(u8 addr, u8 cmd)
** @Input     : addr:IMIU4通信地址，cmd:
** @Output    : void
** @Function  : 发送IMEU3除湿机控制信息CAN报文
** @The notes : 主索引86
**===============================================================================================*/
void SendCanImiu4InsuCtrlMsg(u8 addr, u8 cmd)
{
	can_frame_t msg;

	//参数有效
	if(cmd > 2)
	{
		return;
	}
	msg.id = CanBuildRupAuxMsgID(2, 55, addr, (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev]);
    msg.fd = 1;
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
	msg.data[0] = 0x0c;
	msg.data[1] = 0x01;
	msg.data[2] = 0x00;
	msg.data[3] = 0x00;  //预留 
	msg.data[4] = cmd;  
	msg.len = 5;

	BSPCANSendQin(eCAN2, &msg);
}

#if(0)
static int imeu3_test_main(int argc, char *argv[])
{
    if (argc == 0) {
        printf("imeu3_test para error\n");
        return CLI_FALSE;
    }
	if (!strcmp(argv[0], "read_board1"))
	{
		for(u8 i = 0; i<eIMEUBoardInfo1_Len; i++)
		{
			printf("gImeu3BoardInfo1[%d] = %d\r\n",i,gImeu3BoardInfo1[i]);
		}
		return 0;
	}
	else if (!strcmp(argv[0], "read_board2"))
	{
		for(u8 i = 0; i<eIMEUBoardInfo2_Len; i++)
		{
			printf("gImeu3BoardInfo1[%d] = %d\r\n",i,gImeu3BoardInfo2[i]);
		}
		return 0;
	}
	else if (!strcmp(argv[0], "read_alm"))
	{
		for(u8 i = 0; i<eIMEUAlmInfo_Len; i++)
		{
			printf("gImeu3AlmInfo[%d] = %d\r\n",i,gImeu3AlmInfo[i]);
		}
		return 0;
	}
	else if(!strcmp(argv[0], "write_pwm"))
	{
		u8 arg1 = strtoul(argv[1], NULL, 0);
		u8 arg2 = strtoul(argv[2], NULL, 0);
		SendCanImeu3PwmCtrlMsg(30,arg1,arg2);
	}
	else if(!strcmp(argv[0], "write_do"))
	{
		u8 arg1 = strtoul(argv[1], NULL, 0);
		u8 arg2 = strtoul(argv[2], NULL, 0);
		u8 arg3 = strtoul(argv[3], NULL, 0);
		SendCanImeu3DoCtrlMsg(30,arg1,arg2,arg3);
	}
	else if(!strcmp(argv[0], "write_motor"))
	{
		u8 arg1 = strtoul(argv[1], NULL, 0);
		u8 arg2 = strtoul(argv[2], NULL, 0);
		SendCanImeu3MotorCtrlMsg(30,arg1,arg2);
	}
	else if(!strcmp(argv[0], "write_oc"))
	{
		u8 arg1 = strtoul(argv[1], NULL, 0);
		u8 arg2 = strtoul(argv[2], NULL, 0);
		SendCanImeu3OCCtrlMsg(30,arg1,arg2);
	}
	else if(!strcmp(argv[0], "write_de"))
	{
		u8 arg1 = strtoul(argv[1], NULL, 0);
		u8 arg2 = strtoul(argv[2], NULL, 0);
		u8 arg3 = strtoul(argv[3], NULL, 0);
		SendCanImeu3DeCtrlMsg(30,arg1,arg2,arg3);
	}
	return 0;
}
CLI_CMD("imeu3_test", "imeu3_test", imeu3_test_main);
#endif


#if(1)
static int imiu4_test_main(int argc, char *argv[])
{
    if (argc == 0) {
        printf("imiu4_test para error\n");
        return CLI_FALSE;
    }
        if (!strcmp(argv[0], "read_samp1"))
        {
                for(u8 i = 0; i<eIMIUSampInfo_Len; i++)
                {
                //        printf("gImiu4_1SampInfo[%d] = %d\r\n",i,gImiu4_1SampInfo[i]);
                }
                return 0;
        }
        else if (!strcmp(argv[0], "read_samp2"))
        {
                for(u8 i = 0; i<eIMIUSampInfo_Len; i++)
                {
                        printf("gImiu4_2SampInfo[%d] = %d\r\n",i,gImiu4_Info[e_IMIU4_2]->SampInfo[i]);
                }
                return 0;
        }
        else if (!strcmp(argv[0], "read_alm1"))
        {
                for(u8 i = 0; i<eIMIUAlmInfo_Len; i++)
                {
                        printf("gImiu4_1AlmInfo[%d] = %d\r\n",i,gImiu4_Info[e_IMIU4_1]-> AlmFault[i]);
                }
                return 0;
        }
        else if (!strcmp(argv[0], "read_alm2"))
        {
                for(u8 i = 0; i<eIMIUAlmInfo_Len; i++)
                {
                        printf("gImiu4_2AlmInfo[%d] = %d\r\n",i,gImiu4_Info[e_IMIU4_2]-> AlmFault[i]);
                }
                return 0;
        }
        else if(!strcmp(argv[0], "write_insu"))
        {
                u8 arg1 = strtoul(argv[1], NULL, 0);
                SendCanImiu4InsuCtrlMsg(20, arg1);
        }
        else if(!strcmp(argv[0], "write_ready"))
        {
                u8 arg1 = strtoul(argv[1], NULL, 0);
                u8 arg2 = strtoul(argv[2], NULL, 0);
                SendCanImiu4ReadyCtrlMsg(20, arg1, arg2);
        }
        return 0;
}
CLI_CMD("imiu4_test", "imiu4_test", imiu4_test_main);
#endif
