/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**
**---------------------------------------File Information------------------------------------------
** @File Name    : IMIU4SendInterface.c
** @Founder      : ZQD808
** @Date         : 2026.04.15
** @Function     : IMIU4 CAN帧发送接口模块(BMU→IMIU4)
** @Instructions : IMIU4是BMU下级设备(电流测量单元),本模块实现BMU下发给IMIU4的控制命令发送
**                 主索引55: 继电器控制、绝缘检测控制
**                 主索引59: 采样校准命令
**                 主索引60: 权限切换命令
**                 主索引62: 系统调试命令
**===============================================================================================*/
#include "BSPTypeDef.h"
#include "IMIU4Interface.h"
#include "IMIU4Protocol.h"
#include "IMIU4Config.h"
#include "DEVCAN.h"
#include "driver/can.h"
#include "PublicInfo.h"
#include "GroupPara.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "CLI.h"
#include "CLI_type.h"
/*=================================================================================================
** 外部变量声明
**===============================================================================================*/
/*主控通用参数值*/
extern u16 gGBmuGenPara_102[];

/*=================================================================================================
** 公共函数: IMIU4_BuildCanId
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_BuildCanId
** @Input     : pr:功能码  dp:数据页  pf:主索引  ps:目标地址  sa:源地址
** @Output    : 29位CAN帧ID
** @Function  : 构建IMIU4 CAN帧ID
** @The notes : ID格式: PR(4位)+DP(1位)+PF(8位)+PS(8位)+SA(8位)
**===============================================================================================*/
u32 IMIU4_BuildCanId(u8 pr, u8 dp, u8 pf, u8 ps, u8 sa)
{
    return IMIU4_BUILD_ID(pr, dp, pf, ps, sa);
}

/*=================================================================================================
** @Name      : IMIU4_SendSingleFrame
** @Input     : pr:功能码  dp:数据页  pf:主索引  ps:目标地址  sa:源地址  data:数据指针  len:数据长度
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4单帧数据(最大8字节)
** @The notes : 适用标准CAN帧
**===============================================================================================*/
u8 IMIU4_SendSingleFrame(u8 pr, u8 dp, u8 pf, u8 ps, u8 sa, u8 *data, u8 len)
{
    can_frame_t msg;
    u32 canId;
    
    if(len > 64)
    {
        return FALSE;
    }
    if(data == NULL && len > 0)
    {
        return FALSE;
    }
    
    canId = IMIU4_BuildCanId(pr, dp, pf, ps, sa);
    msg.id = canId;
    msg.fd = 1;
    msg.ide = CAN_EXTID;
    msg.rtr = CAN_DTR;
    
    if(len > 0)
    {
        memcpy(msg.data, data, len);
        msg.len = len;
    }
    else
    {
        msg.len = 0;
    }
    
    return BSPCANSendQin(IMIU4_CAN_CH, &msg);
}

/*=================================================================================================
** 公共函数: IMIU4_SendFrame
**===============================================================================================*/
/*=================================================================================================
** @Name      : IMIU4_SendFrame
** @Input     : frame:IMIU4帧结构指针
** @Output    : void
** @Function  : IMIU4 CAN帧发送接口
** @The notes : 支持CANFD 64字节长帧
**===============================================================================================*/
void IMIU4_SendFrame(t_IMIU4Frame *frame)
{
    can_frame_t msg;
    u32 canId;
    
    /*参数有效性检查*/
    if(frame == NULL)
    {
        return;
    }
    /*数据长度检查*/
    if(frame->len > 64)
    {
        return;
    }
    
    /*构建CAN帧ID*/
    canId = IMIU4_BuildCanId(frame->pr, frame->dp, frame->pf, frame->ps, frame->sa);
    msg.id = canId;
    msg.fd = 1;                    /*CANFD帧*/
    msg.ide = CAN_EXTID;           /*扩展帧ID*/
    msg.rtr = CAN_DTR;             /*数据帧*/
    msg.len = frame->len;
    
    /*填充数据域*/
    if(frame->len > 0)
    {
        memcpy(msg.data, frame->data, frame->len);
    }
    
    /*调用底层CAN发送函数*/
    BSPCANSendQin(IMIU4_CAN_CH, &msg);
}

/*=================================================================================================
** @Name      : IMIU4_SendDeviceInfoQuery
** @Input     : targetAddr:IMIU4目标地址(20或21)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4设备信息查询(BMU→IMIU4)
** @The notes : 主索引1,功能码5(请求查询),查询子索引数目7,起始子索引0
**===============================================================================================*/
u8 IMIU4_SendDeviceInfoQuery(u8 targetAddr)
{
    u8 data[4];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 0x0a;
    data[2] = 0;
    data[3] = 0;
    
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestQuery, 1, eIMIU4Idx_DeviceInfo, targetAddr, bmuAddr, data, 4);
}

/*=================================================================================================
** @Name      : IMIU4_SendSerNumQuery
** @Input     : targetAddr:IMIU4目标地址(20或21)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4序列号查询(BMU→IMIU4)
** @The notes : 主索引2,功能码5(请求查询),查询23项u8,起始子索引0
**===============================================================================================*/
u8 IMIU4_SendSerNumQuery(u8 targetAddr)
{
    u8 data[4];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 23;
    data[2] = 0;
    data[3] = 0;
    
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestQuery, 1, eIMIU4Idx_SerNum, targetAddr, bmuAddr, data, 4);
}

/*=================================================================================================
** @Name      : IMIU4_SendSystemStatusSet
** @Input     : targetAddr:IMIU4目标地址(20或21)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4采样信息查询(BMU→IMIU4)
** @The notes : 主索引50,功能码2(请求修改),查询子索引数目1,起始子索引0
**===============================================================================================*/
u8 IMIU4_SendSystemStatusSet(u8 targetAddr, u8 workmode)
{
    u8 data[6];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 1;
    data[2] = 0;
    data[3] = 0;
    data[4] = workmode;
    data[5] = 0;
   
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestModify, 1, eIMIU4Idx_SystemStatus, targetAddr, bmuAddr, data, 6);
}

/*=================================================================================================
** @Name      : IMIU4_SendSystemStatusSet
** @Input     : targetAddr:IMIU4目标地址(20或21)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4采样校准命令(BMU→IMIU4)
** @The notes : 主索引52,功能码2(请求修改),查询子索引数目1,起始子索引0
**===============================================================================================*/
u8 IMIU4_SendSampCalibrationCmd(u8 targetAddr, u16 SampCalType, u16 CalValue)
{
    u8 data[8];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 2;
    data[2] = 0;
    data[3] = 0;
    data[4] = (u8)(SampCalType & 0xff);
    data[5] = (u8)((SampCalType>>8) & 0xff);
    data[6] = (u8)(CalValue & 0xff);
    data[7] = (u8)((CalValue>>8) & 0xff);
    
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestModify, 1, eIMIU4Idx_SampCalCmd, targetAddr, bmuAddr, data, 8);
}

/*=================================================================================================
** @Name      : IMIU4_SendAuthCtrl
** @Input     : targetAddr:IMIU4目标地址  authCmd:权限命令  authPwdH:密码高16位  authPwdL:密码低16位
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4权限切换命令(BMU→IMIU4)
** @The notes : 主索引53,功能码2(请求修改),用于权限级别切换
**===============================================================================================*/
u8 IMIU4_SendAuthCtrl(u8 targetAddr, u16 authPwdH, u16 authPwdL)
{
    u8 data[8];
    u8 bmuAddr;
    
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 0x02;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = (u8)(authPwdH & 0xFF);
    data[5] = (u8)((authPwdH >> 8) & 0xFF);
    data[6] = (u8)(authPwdL & 0xFF);
    data[7] = (u8)((authPwdL >> 8) & 0xFF);
    
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestModify, 1, eIMIU4Idx_AuthCtrl, targetAddr, bmuAddr, data, 8);
}


/*=================================================================================================
** @Name      : IMIU4_SendReadyCtrl
** @Input     : targetAddr:IMIU4目标地址  readyNum:继电器选择(0/1/2)  state:状态(0断开/1闭合)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4继电器控制命令(BMU→IMIU4)
** @The notes : 主索引55,功能码2(请求修改),子索引1,用于DO继电器开关控制
**===============================================================================================*/
u8 IMIU4_SendReadyCtrl(u8 targetAddr, u8 readyNum, u8 state)
{
    u8 data[10];
    u8 bmuAddr;
    
    if(readyNum > IMIU4_READY_DO2)
    {
        return FALSE;
    }
    if(state > IMIU4_READY_ON)
    {
        return FALSE;
    }
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 0x03;
    data[2] = 0x01;
    data[3] = 0x00;
    data[4] = readyNum;
    data[5] = 0;
    data[6] = state;
    data[7] = 0;
    data[8] = 1;
    data[9] = 0;
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestModify, 1, eIMIU4Idx_Control, targetAddr, bmuAddr, data, 10);
}

/*=================================================================================================
** @Name      : IMIU4_SendInsuCtrl
** @Input     : targetAddr:IMIU4目标地址  cmd:控制命令(0自动/1开始/2停止)
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4绝缘检测控制命令(BMU→IMIU4)
** @The notes : 主索引55,功能码2(请求修改),子索引0,用于绝缘检测控制
**===============================================================================================*/
u8 IMIU4_SendInsuCtrl(u8 targetAddr, u8 cmd)
{
    u8 data[6];
    u8 bmuAddr;
    
    if(cmd > IMIU4_INSU_STOP)
    {
        return FALSE;
    }
    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 0x01;
    data[2] = 0x00;
    data[3] = 0x00;
    data[4] = cmd;
    data[5] = 0;
    
    return IMIU4_SendSingleFrame(eIMIU4Func_RequestModify, 1, eIMIU4Idx_Control, targetAddr, bmuAddr, data, 6);
}


/*=================================================================================================
** @Name      : IMIU4_SendSysDebug
** @Input     : targetAddr:IMIU4目标地址  debugType:调试类型  debugPort:调试端口  debugState:调试状态
** @Output    : 发送结果: 0失败  1成功
** @Function  : 发送IMIU4系统调试命令(BMU→IMIU4)
** @The notes : 主索引60,功能码2(请求修改),用于干节点/继电器/EEPROM调试
**===============================================================================================*/
u8 IMIU4_SendSysDebug(u8 targetAddr, u16 debugType, u16 debugPort, u16 debugState, u32 debugEEPara)
{
    u8 data[16];
    u8 bmuAddr;
    
    if(debugType > 2)
    {
        return FALSE;
    }

    if(gGBmuGenPara_102 == NULL)
    {
        return FALSE;
    }
    bmuAddr = (u8)gGBmuGenPara_102[eBmuGenPara102_BmuDev];
    
    data[0] = IMIU4_FRAME_SINGLE;
    data[1] = 0x06;
    data[2] = 0x00;
    data[3] = 0x00;

    data[4] = (u8)(debugType & 0xFF);
    data[5] = (u8)((debugType >> 8) & 0xFF);

    data[6] = (u8)(debugPort & 0xFF);
    data[7] = (u8)((debugPort >> 8) & 0xFF);

    data[8] = (u8)(debugState & 0xFF);
    data[9] = (u8)((debugState >> 8) & 0xFF);

    data[10] = (u8)((debugEEPara >> 16) & 0xFF);
    data[11] = (u8)((debugEEPara >> 24) & 0xFF);  
    data[12] = (u8)(debugEEPara& 0xFF);
    data[13] = (u8)((debugEEPara >> 8) & 0xFF);  

    if(debugType == 2)
    {
        data[14] = 2;
        data[15] = 0;
    }else
    {
        data[14] = 1;
        data[15] = 0;
    }


    return IMIU4_SendSingleFrame(eIMIU4Func_RequestModify, 1, eIMIU4Idx_SysDebug, targetAddr, bmuAddr, data, 16);
}


#if(1)
/*=================================================================================================
** @Name      : imeu3_unit_test_main
** @Input     : argc - 参数个数, argv - 参数数组
** @Output    : CLI_FALSE - 参数错误或未知命令, 0 - 执行成功
** @Function  : IMEU3单元测试CLI命令入口
** @The notes : 支持两个子命令：test_send、test_rcv
**===============================================================================================*/
static int imeu4_send_test_main(int argc, char *argv[])
{
    /*检查参数有效性*/
    if(argc == 0)
    {
       
        return CLI_FALSE;
    }
    
    /*根据参数执行对应测试*/
    if(!strcmp(argv[0], "ganjiedian"))
    {
        IMIU4_SendSysDebug((u8)IMIU4_1_DEV_ADDR,0,0,1,0);  
    }
    else if(!strcmp(argv[0], "ready"))
    {
        IMIU4_SendSysDebug((u8)IMIU4_1_DEV_ADDR,1,0,1,0);  
    }
    else if(!strcmp(argv[0], "eeprom"))
    {
        IMIU4_SendSysDebug((u8)IMIU4_1_DEV_ADDR,2,0,2,0x11111111);  
    }
    else if(!strcmp(argv[0], "insulation"))
    {
        IMIU4_SendInsuCtrl((u8)IMIU4_1_DEV_ADDR,1);  
    }
    else if(!strcmp(argv[0], "do"))
    {
        IMIU4_SendReadyCtrl((u8)IMIU4_1_DEV_ADDR,0,1);  
    }
    else if(!strcmp(argv[0], "password"))
    {
        IMIU4_SendAuthCtrl((u8)IMIU4_1_DEV_ADDR,0,0);  
    }
    else if(!strcmp(argv[0], "calibration"))
    {
        IMIU4_SendSampCalibrationCmd((u8)IMIU4_1_DEV_ADDR,8,0);  
    }
    else if(!strcmp(argv[0], "workmode"))
    {
        IMIU4_SendSystemStatusSet((u8)IMIU4_1_DEV_ADDR,5);  
    }
    else
    {
        printf("Unknown command: %s\r\n", argv[0]);
        printf("Usage: imiu4_unit_test <test_send|test_rcv>\r\n");
        return CLI_FALSE;
    }
    
    return 0;
}

/*注册CLI命令：imeu3_unit_test*/
CLI_CMD("imeu4_test", "IMEU4 send test commands", imeu4_send_test_main);

#endif