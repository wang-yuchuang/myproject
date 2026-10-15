#include <time.h>
#include "../ccu_sampler_main.h"

#define CAN_BCAST_ADDR  0xFA

/*=============================================================================================
** @Interface function definition
**===========================================================================================*/
/*=============================================================================================
** @Name      : BYTE TxCAN3Buff(canmsg * pRcvMsg, BYTE u8Len)
** @Input     : pRcvMsg:发送报文 u8Len:报文长度
** @Output    : FALSE:发送失败 TRUE：发送成功
** @Function  : CAN1报文发送
** @The notes :
**===========================================================================================*/


BYTE TxCAN3Buff(ROUGH_CCU_DATA* pCCU, canmsg * pRcvMsg, BYTE u8Len)
{
    if (u8Len <= 0 || u8Len > 8)
    {
        return FALSE;
    }

    // 1. 定义一个符合底层要求的通信缓冲区
    // 格式：1字节长度 + 4字节ID + 8字节数据 = 13字节。这里安全起见给 16 字节
    BYTE byRawData[16];
    memset(byRawData, 0, sizeof(byRawData));  

    // 2. 填入第 0 字节：数据长度（对应底层 pBuffer[0]）
    byRawData[0] = u8Len;

    // 3. 填入第 1~4 字节：大端序的 CAN ID（对应底层 pBuffer[1]~[4] 的 uiRawHead 解析）
    // 注意：底层的解析是 pBuffer[1]<<24 + pBuffer[2]<<16 + pBuffer[3]<<8 + pBuffer[4]
    byRawData[1] = (BYTE)((pRcvMsg->Id >> 24) & 0xFF);
    byRawData[2] = (BYTE)((pRcvMsg->Id >> 16) & 0xFF);
    byRawData[3] = (BYTE)((pRcvMsg->Id >> 8) & 0xFF);
    byRawData[4] = (BYTE)(pRcvMsg->Id & 0xFF);

    // 4. 填入第 5~12 字节：将应用层 32位 数组拆分成大端序的数据字节（对应底层 &pBuffer[5]）
    byRawData[5]  = (BYTE)((pRcvMsg->Data[0] >> 24) & 0xFF);
    byRawData[6]  = (BYTE)((pRcvMsg->Data[0] >> 16) & 0xFF);
    byRawData[7]  = (BYTE)((pRcvMsg->Data[0] >> 8) & 0xFF);
    byRawData[8]  = (BYTE)(pRcvMsg->Data[0] & 0xFF);

    byRawData[9]  = (BYTE)((pRcvMsg->Data[1] >> 24) & 0xFF);
    byRawData[10] = (BYTE)((pRcvMsg->Data[1] >> 16) & 0xFF);
    byRawData[11] = (BYTE)((pRcvMsg->Data[1] >> 8) & 0xFF);
    byRawData[12] = (BYTE)(pRcvMsg->Data[1] & 0xFF);

    // 5. 调用底层发送接口
    // 注意：传入的 nBytesToWrite 至少要包含整个打包协议长度（1 + 4 + 8 = 13 字节）
    // 底层会进入 while 循环，只要这个值大于 0，它就会解析并调用 write(fd, &w_frame, 16)
    // 成功后底层会返回 16 (sizeof(w_frame))
    int nBytesToSend = 16; 
    ssize_t nWriteSize = 0;
    nWriteSize = pCCU->pComm.Write(pCCU->hCanDevHanle, (char *)byRawData, nBytesToSend);

    if(nWriteSize == 0)
    {
        return TRUE;
        printf("TxCAN3Buff Send: ");
        for (int i = 0; i < nBytesToSend; i++) {
            printf("%02X ", byRawData[i]);
        }
        printf("\n");
    }

    // 6. 返回值校验：底层成功写入 16 字节则代表发送成功
    if (nWriteSize > 0)
    {
        return TRUE;
    }
    
    return FALSE;
}

/*=============================================================================================
** @Name      : BYTE u8PackSendMsg(BYTE u8DevNo,BYTE u8Cmd, BYTE u8GSNo,UINT Dh,UINT Dl)
** @Input     : u8DevNo:设备号；u8Cmd:命令号；u8GSNo:模块地址；Dh:发送报文高32位；Dl:发送报文低32位
** @Output    : FALSE:发送失败 TRUE：发送成功
** @Function  : 发送信息写入标准的发送报文
** @The notes :
**===========================================================================================*/
BYTE u8PackSendMsg(ROUGH_CCU_DATA* pCCU, BYTE u8DevNo,BYTE u8Cmd, BYTE u8GSNo,UINT Dh,UINT Dl)
{
    BYTE u8Rst = TRUE;
    static UINT sLastCanTxTick = 0;

    // UNUSED(u8Rst);
    canmsg SendMsg;

    if (u8GSNo >= mRECTNUM && R_PRPTO_DEVNO_0A == u8DevNo)
    {
        return FALSE;
    }
    if (u8GSNo >= _MAX_GRP_NUM && R_PRPTO_DEVNO_0B == u8DevNo)      //这个
    {
        return FALSE;
    }
    if (!(R_PRPTO_DEVNO_0A == u8DevNo || R_PRPTO_DEVNO_0B == u8DevNo))
    {
        return FALSE;
    }

    // if(gs_ModSystemInfo == NULL)
    // {
    //     return FALSE;
    // }
    if (u8DevNo == R_PRPTO_DEVNO_0A)
    {
        SendMsg.Id = (R_PRPTO_DEVNO_0A << 22) + (u8Cmd << 16) + (pCCU->acdc_data.gs_ModSystemInfo.AddrInfo[u8GSNo] << 8) + R_PROTO_SELF_ADDR;			//模块地址 0 1 2 3
    }
    else
    {
        SendMsg.Id = (R_PRPTO_DEVNO_0B << 22) + (u8Cmd << 16) + (pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[u8GSNo] << 8) + R_PROTO_SELF_ADDR;			//组 1 2 
    }
    SendMsg.Data[0] = Dh;
    SendMsg.Data[1] = Dl;


    /* CAN帧间间隔 ≥20ms，满足英飞源PCS协议要求 */
    // {
    //     UINT now = osKernelGetTickCount();
    //     if (sLastCanTxTick != 0 && (now - sLastCanTxTick) < 20)
    //     {
    //         osDelay(20 - (now - sLastCanTxTick));
    //     }
    //     sLastCanTxTick = osKernelGetTickCount();
    // }
    if (!TxCAN3Buff(pCCU, &SendMsg, 8))
    {
        printf("TxCAN3Buff failed in u8PackSendMsg\r\n");
        return FALSE;
    }

    return TRUE;
}
/*=================================================================================================
** @Name      : void ABU_PackSendMsg(BYTE u8DevNo, BYTE u8Cmd, UINT Dh, UINT Dl)
** @Input     : u8DevNo:设备号 u8Cmd：命令号 Dh：发送报文高32位 Dl：发送报文低32位
** @Output    : void
** @Function  : 向ABU发送报文信息
** @The notes : 
**===============================================================================================*/
void ABU_PackSendMsg(ROUGH_CCU_DATA* pCCU, BYTE u8DevNo, BYTE u8Cmd, UINT Dh, UINT Dl)
{
    canmsg SendMsg;
    SendMsg.Id = (u8DevNo << 22) + (u8Cmd << 16) + (ABU_Addr << 8) + R_PROTO_SELF_ADDR;

    SendMsg.Data[0] = Dh;
    SendMsg.Data[1] = Dl;
    if (TxCAN3Buff(pCCU, &SendMsg, 8) != TRUE)
    {
        printf("TxCAN3Buff failed in ABU_PackSendMsg\r\n");
    }
}

/*=================================================================================================
** @Name      : void GetGrp_VoltCurr(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读系统类信息报文
** @The notes : 内部协议 CMD = 0X01：读取系统电压和电流
**===============================================================================================*/
void GetGrp_VoltCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
      //命令号
      BYTE u8CMD = 0x01;

      if(SGNo < _MAX_GRP_NUM)
      {
          u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0);
      }
}
/*=================================================================================================
** @Name      : void GetGrp_NumAndStatus(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读系统类信息报文
** @The notes : 内部协议 CMD = 0X02：读取协议转换器版本、系统状态表、模块数
**===============================================================================================*/
void GetGrp_NumAndStatus(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    //命令号
    BYTE u8CMD = 0x02;

    if(SGNo < _MAX_GRP_NUM)
    {
        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
    }
}
/*=============================================================================================
** @Name      : void GetSingle_VoltCurr(BYTE SGNo)
** @Input     : SGNo:模块地址0-1-2-3...  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : CMD = 0X03：读取模块N电压、读取模块N电流
**===========================================================================================*/
void GetSingle_VoltCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    BYTE u8CMD = 0x03;	 
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_GrpAndStatus(BYTE SGNo)
** @Input     : SGNo:模块号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X04：读取模块组号、高低压模式、模块环温、模块状态表0-2  8S周期读
**===============================================================================================*/
void GetSingle_GrpAndStatus(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    //命令号
    BYTE u8CMD = 0x04;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_VerInfo(BYTE SGNo)
** @Input     : SGNo:模块号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X07：读取DC版本、AC版本、HW版本
**===============================================================================================*/
void GetSingle_VerInfo(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    //命令号
    BYTE u8CMD = 0x07;	 

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_MaxCapacity(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0A：读取最大电压、最小电压、最大电流、额定功率（N2002无）
**===============================================================================================*/
void GetGrp_MaxCapacity(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    //命令号
    BYTE u8CMD = 0x0A;

    if(SGNo < _MAX_GRP_NUM)
    {
        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
    }
}
/*=================================================================================================
** @Name      : void GetSingle_SerialNo(BYTE SGNo)
** @Input     : SGNo:模块号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0B：读取产品系列、条码信息	
**===============================================================================================*/
void GetSingle_SerialNo(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    //命令号
    BYTE u8CMD = 0x0B;	 

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_ExtVolt(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0C：读取模块电压、最大允许电流
**===============================================================================================*/
void GetGrp_ExtVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    //命令号
    BYTE u8CMD = 0x0C;

    if(SGNo < _MAX_GRP_NUM)
    {
        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, 0, 0); 
    }
}
/*=================================================================================================
** @Name      : void SetGrp_SerialParellMode(BYTE SGNo, BYTE u8Mode)
** @Input     : SGNo:组号  u8Mode:高低压模式 0-不需要设置 1-低压模式 2-高压模式
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 内部协议 CMD = 0X0F：写扩展	
**===============================================================================================*/
void SetGrp_SerialParellMode(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8Mode)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x0F;

    u32DataH = 0x11140000;

    //20211120 lkf 增加1KV模块的高低压模式切换处理
    if(u8Mode == GRP_SerialParallelMode_SET_Serial)
    {
        u32DataL = 0xA1; 
    }
    else 
    {
        u32DataL = 0xA2; 
    }

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetDCDCMode_0x0F(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD = 0X0F：写扩展	  (//test 设置DCDC模块模式？)
**===============================================================================================*/
void SetDCDCMode_0x0F(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;
    BYTE u8CMD = 0x0F;

    u32DataH = 0x11110000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetMPPTMode_0x0F(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD = 0X0F：写扩展	  (//test 设置光伏模块模式？)
**===============================================================================================*/
void SetMPPTMode_0x0F(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;
    BYTE u8CMD = 0x0F;

    u32DataH = 0x11150000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetGrp_WalkIn(BYTE SGNo, USHORT u16RectLoadStartTime)
** @Input     : SGNo:组号 u16RectLoadStartTime:Walkin时间
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD = 0X13：设置Walkin使能和Walkin时间
**===============================================================================================*/
void SetGrp_WalkIn(ROUGH_CCU_DATA* pCCU, BYTE SGNo, USHORT u16RectLoadStartTime)
{
    BYTE u8CMD = 0x13;     //命令号
    __XXX_UNION_VALUE untemp;
    __16BITS_UNION_VALUE untemp_16bits;
    UINT uiDH,uiDL;

    if (SGNo < _MAX_GRP_NUM)
    {
        untemp.u32Value = 0;
        if(u16RectLoadStartTime == 0)
        {
            untemp.chtemp[3] = 0;                                                //禁止
        }
        else
        {
            untemp.chtemp[3] = 1;                                                //使能
        }

        uiDH = untemp.u32Value;
        untemp_16bits.u16Temp = u16RectLoadStartTime;

        untemp.u32Value = 0;
        untemp.chtemp[1] = untemp_16bits.chtemp[1];
        untemp.chtemp[0] = untemp_16bits.chtemp[0];
        uiDL = untemp.u32Value;

        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, uiDH, uiDL); 
    }
}
/*=================================================================================================
** @Name      : void SetGrp_OnOff_No10(BYTE SGNo)
** @Input     : SGNo:组号  u8OnOff:开关机状态 
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD = 0X1A：开关机 1为关机，0为开机 
//test 这里要变成开关机参数直接传进来
**===============================================================================================*/
void SetGrp_OnOff_No10(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8OnOff)
{
    UINT u32Val = 0;
    BYTE u8CMD = 0x1A;

    u32Val = ((UINT)u8OnOff<<24);

    //test 这里后续再看怎么办
    // if(PData->u8GRP_RectOnOff[SGNo] == RECT_OFF)
	// {
	// 	gs_CANComm.GrpData[SGNo].u8PowerofThisVoltMaxIsValidFlag = 0;
	// }
	// else if(gs_CANComm.GrpData[SGNo].u8PowerofThisVoltMaxIsValidFlag == 0)
	// {
	// 	gs_CANComm.GrpData[SGNo].u8PowerofThisVoltMaxIsValidFlag = 1;
	// }

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32Val, 0); 
}
/*=================================================================================================
** @Name      : void SetGrp_AdjVoltCurr_No13(BYTE SGNo, UINT u32Volt, UINT u32Curr)
** @Input     : SGNo:组号  u32Volt:设置电压值 u32Curr:设置电流值 
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 内部协议 CMD =  0X1B：设置模块电压（mV）、模块电流（mA）
**===============================================================================================*/
void SetGrp_AdjVoltCurr_No13(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Volt, UINT u32Curr)
{
    BYTE u8CMD = 0x1B;
    
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32Volt, u32Curr); 
}
/*=================================================================================================
** @Name      : void SettModePara_Time(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号  u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送设置信息报文 
** @The notes : 内部协议 CMD = 0X2D：设置模块时间 年月日时 分秒
**===============================================================================================*/
void SettModePara_Time(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo)
{
    co_dword dword;
    UINT u32DataH, u32DataL;

    dword.b[3] = 0;
    dword.b[2] = 0;
    dword.b[1] = 0;
    dword.b[0] = 0;
    u32DataH = dword.l;

    dword.b[3] = 0;
    dword.b[2] = 0;
    dword.b[1] = 0;
    dword.b[0] = 0;
    u32DataL = dword.l;

    const BYTE u8CMD = 0x2D;
    u8PackSendMsg(pCCU, u8DevNo, u8CMD, SGNo, u32DataH, u32DataL); 
}

/*=================================================================================================
** @Name      : void GetGrp_ActivePower_0X23(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取系统(0X0B)总有功功率（mW）
**===============================================================================================*/
void GetGrp_ActivePower_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x01020000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_ReactivePower_0X23(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取系统(0X0B)总无功功率（mVar）（mW）
**===============================================================================================*/
void GetGrp_ReactivePower_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x01030000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_SysDCVolt(BYTE SGNo)
** @Input     : SGNo:组号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取系统直流侧/电池侧电压mV
**===============================================================================================*/
void GetModeInfo_SysDCVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    u32DataH = 0x10010000;

    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_SysDCCurr(BYTE SGNo);
** @Input     : SGNo:组号
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取系统直流侧/电池侧总电流mA
**===============================================================================================*/
void GetModeInfo_SysDCCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    u32DataH = 0x10020000;

    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_ModeNum(BYTE SGNo)
** @Input     : SGNo:组号 u8DevNo:设备号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块数
**===============================================================================================*/
void GetModeInfo_ModeNum(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    u32DataH = 0x10100000;

    BYTE u8CMD = 0x23;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_DCVolt(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号 u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块直流侧电压1101
**===============================================================================================*/
void GetModeInfo_DCVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo)
{
    UINT u32DataH; 
    u32DataH = 0x11010000;

    BYTE u8CMD = 0x23;
    u8PackSendMsg(pCCU, u8DevNo, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_DCCurr(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号 u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块直流侧电流1102
**===============================================================================================*/
void GetModeInfo_DCCurr(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo)
{
    UINT u32DataH; 
    u32DataH = 0x11020000;

    BYTE u8CMD = 0x23;
    u8PackSendMsg(pCCU, u8DevNo, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_LineABCVolt(BYTE SGNo, BYTE u8Num)
** @Input     : SGNo:组号 u8Num：A/B/C相
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取交流AB、BC、CA线电压
**===============================================================================================*/
void GetModeInfo_LineABCVolt(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8Num)
{
    UINT u32DataH = 0; 
    BYTE u8CMD = 0x23;

    switch (u8Num)
    {
        case eType_AB:
            u32DataH = 0x11030000;
            break;

        case eType_BC:
            u32DataH = 0x11040000;
            break;

        case eType_CA:
            u32DataH = 0x11050000;
            break;

        default:
            break;
    }
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetModeInfo_ModeAmbTmp(BYTE SGNo, BYTE u8DevNo)
** @Input     : SGNo:组号 u8DevNo:设备号
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取模块环境温度
**===============================================================================================*/
void GetModeInfo_ModeAmbTmp(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8DevNo)
{
    UINT u32DataH; 
    u32DataH = 0x11060000;

    BYTE u8CMD = 0x23;
    u8PackSendMsg(pCCU, u8DevNo, u8CMD, SGNo, u32DataH, 0); 
}

/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_10(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表
**===============================================================================================*/
void GetSingle_ConvertStat_10(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    BYTE u8CMD = 0x23;

    u32DataH = 0x11100000;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_11(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表
**===============================================================================================*/
void GetSingle_ConvertStat_11(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    BYTE u8CMD = 0x23;

    u32DataH = 0x11110000;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_12(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表
**===============================================================================================*/
void GetSingle_ConvertStat_12(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x11120000; 
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}

/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_13(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表2_13
**===============================================================================================*/
void GetSingle_ConvertStat_13(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x11130000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_14(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表3_14
**===============================================================================================*/
void GetSingle_ConvertStat_14(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x11140000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_ConvertStat_15(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取逆变状态表4_15
**===============================================================================================*/
void GetSingle_ConvertStat_15(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x11150000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_GrpNum(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块组号
**===============================================================================================*/
void GetSingle_GrpNum(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x11200000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetSingle_Barcode(BYTE SGNo, UINT u32DataH)
** @Input     : SGNo:组号  u32DataH:
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mA）条码信息	0x1124-0x1129
**===============================================================================================*/
void GetSingle_Barcode(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32DataH)
{
    BYTE u8CMD = 0x23;	 

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_MaxCapacity_0x23(int SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读取信息报文
** @The notes : 外部协议 CMD = 0X23：直流额定功率（mW） 
**===============================================================================================*/
void GetGrp_MaxCapacity_0x23(ROUGH_CCU_DATA* pCCU, int SGNo)
{
    BYTE u8CMD = 0x23;
    UINT u32DataH = 0x11330000;

    if(SGNo < _MAX_GRP_NUM)
    {
        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
    }
}
/*=================================================================================================
** @Name      : void GetGrp_MaxCurr_0x23(int SGNo)
** @Input     : SGNo : group number
** @Output    : void
** @Function  : Read DC max output current from module
** @The notes : PCS protocol CMD=0x23, register 0x1132, unit mA
**===============================================================================================*/
void GetGrp_MaxCurr_0x23(ROUGH_CCU_DATA* pCCU, int SGNo)
{
    BYTE u8CMD = 0x23;
    UINT u32DataH = 0x11320000;

    if(SGNo < _MAX_GRP_NUM)
    {
        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0);
    }
}

/*=================================================================================================
** @Name      : void PCS_ReadPhaseVolt_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
** @Output    : void
** @Function  : 向模块发送读信息报文
** @The notes : 外部协议 CMD = 0X23：读取（mV）交流A相相电压、交流B相相电压、交流C相相电压
**===============================================================================================*/
void PCS_ReadPhaseVolt_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd)
{
    co_dword dword;

    dword.v = 0u;
    if (cmd == eType_AB)
    {
        dword.b[2] = 0x01;
    }
    else if (cmd == eType_BC)
    {
        dword.b[2] = 0x02;
    }
    else if (cmd == eType_CA)
    {
        dword.b[2] = 0x03;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, 0x23, ch, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void PCS_ReadPhaseCurr_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mA）交流A相电流、交流B相电流、交流C相电流
**===============================================================================================*/
void PCS_ReadPhaseCurr_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd)
{
    co_dword dword;

    dword.v = 0u;
    if (cmd == eType_AB)
    {
        dword.b[2] = 0x04;
    }
    else if (cmd == eType_BC)
    {
        dword.b[2] = 0x05;
    }
    else if (cmd == eType_CA)
    {
        dword.b[2] = 0x06;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, 0x23, ch, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_ACFreq_0x23(int SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读取信息报文
** @The notes : 外部协议 CMD = 0X23：读取交流频率（mHZ）
**===============================================================================================*/
void GetGrp_ACFreq_0x23(ROUGH_CCU_DATA* pCCU, int SGNo)
{
    BYTE u8CMD = 0x23;
    UINT u32DataH = 0x21070000;

    if(SGNo < _MAX_GRP_NUM)
    {
        u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
    }
}

/*=================================================================================================
** @Name      : void PCS_ReadActivePower_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mW）总有功功率、A相有功功率、B相有功功率、C相有功功率
**===============================================================================================*/
void PCS_ReadActivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd)
{
    co_dword dword;

    dword.v = 0u;
    if (cmd == eType_Total)
    {
        dword.b[2] = 0x08;
    }
    else if (cmd == eType_AB)
    {
        dword.b[2] = 0x09;
    }
    else if (cmd == eType_BC)
    {
        dword.b[2] = 0x0A;
    }
    else if (cmd == eType_CA)
    {
        dword.b[2] = 0x0B;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, 0x23, ch, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void PCS_ReadReactivePower_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mVA）总无功功率、A相无功功率、B相无功功率、C相无功功率
**===============================================================================================*/
void PCS_ReadReactivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd)
{
    co_dword dword;

    dword.v = 0u;
    if (cmd == eType_Total)
    {
        dword.b[2] = 0x0C;
    }
    else if (cmd == eType_AB)
    {
        dword.b[2] = 0x0D;
    }
    else if (cmd == eType_BC)
    {
        dword.b[2] = 0x0E;
    }
    else if (cmd == eType_CA)
    {
        dword.b[2] = 0x0F;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, 0x23, ch, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void PCS_ReadApparentPower_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（mVA）总视在功率、A相视在功率、B相视在功率、C相视在功率
**===============================================================================================*/
void PCS_ReadApparentPower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd)
{
    co_dword dword;

    dword.v = 0u;
    if (cmd == eType_Total)
    {
        dword.b[2] = 0x10;
    }
    else if (cmd == eType_AB)
    {
        dword.b[2] = 0x11;
    }
    else if (cmd == eType_BC)
    {
        dword.b[2] = 0x12;
    }
    else if (cmd == eType_CA)
    {
        dword.b[2] = 0x13;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, 0x23, ch, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void PCS_ReadPowerFactor_0x23(BYTE ch, BYTE cmd)
** @Input     : ch:模块地址  cmd:从命令号
 ** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取（0.001）总功率因素、A相功率因素、B相功率因素、C相功率因素
**===============================================================================================*/
void PCS_ReadPowerFactor_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch, BYTE cmd)
{
    co_dword dword;

    dword.v = 0u;
    if (cmd == eType_Total)
    {
        dword.b[2] = 0x14;
    }
    else if (cmd == eType_AB)
    {
        dword.b[2] = 0x15;
    }
    else if (cmd == eType_BC)
    {
        dword.b[2] = 0x16;
    }
    else if (cmd == eType_CA)
    {
        dword.b[2] = 0x17;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, 0x23, ch, dword.v, 0); 
}

/*=================================================================================================
** @Name      : void GetSys_BusVolt_0X23(BYTE SGNo)
** @Input     : SGNo:组号  u32CmdDataH
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : (DCDC模块调用) 系统母线侧电压（mV）
**===============================================================================================*/
void GetSys_BusVolt_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    u32DataH = 0x40010000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_BusVolt_0X23(BYTE SGNo)
** @Input     : SGNo:组号  u32CmdDataH
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : (DCDC模块调用)  模块直流母线侧电压（mV）
**===============================================================================================*/
void GetGrp_BusVolt_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH; 
    u32DataH = 0x41010000;
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void GetGrp_BusCurr_0X23(BYTE SGNo)
** @Input     : SGNo:组号  
** @Output    : void
** @Function  : 向模块发送读信息报文 
** @The notes : 外部协议 CMD = 0X23：读取模块直流母线侧电流（mA）（用于DC模块）
**===============================================================================================*/
void GetGrp_BusCurr_0X23(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH = 0x41020000;	
    BYTE u8CMD = 0x23;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, 0); 
}
/*=================================================================================================
** @Name      : void SetGrp_AdjVolt_0x24_BEG(BYTE SGNo, UINT Volt)
** @Input     : SGNo:组号  Volt:电压值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置系统(0X0B)直流电压
**===============================================================================================*/
void SetGrp_AdjVolt_0x24_BEG(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT Volt)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x10010000;
    u32DataL = Volt;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetGrp_AdjCurr_0x24_BEG(BYTE SGNo, UINT Curr)
** @Input     : SGNo:组号  Curr:电流值
 ** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置系统(0X0B)直流总电流
**===============================================================================================*/
void SetGrp_AdjCurr_0x24_BEG(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT Curr)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x10020000;

    u32DataL = Curr;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetSysPower_0x24(BYTE SGNo, UINT u32Power)
** @Input     : SGNo:组号 u32Power:功率值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置系统(0X0B)总有功功率（mW）
**===============================================================================================*/
void SetSysPower_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Power)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x10030000;
    u32DataL = u32Power;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetBattSideOutVolt_0x24(BYTE SGNo, UINT u32Volt)
** @Input     : SGNo:组号 u32Volt:电压值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧输出电压（mV）
**===============================================================================================*/
void SetBattSideOutVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Volt)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x10010000;
    u32DataL = u32Volt;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL);

}
/*=================================================================================================
** @Name      : void SetBattSideOutCurr_0x24(BYTE SGNo,UINT u32Curr)
** @Input     : SGNo:组号 u32Curr:电流值
** @Output    : void
** @Function  : 向模块发送设置信息报文 
** @The notes : 外部协议 CMD = 0X24：设置电池侧输出电流（mV）
**===============================================================================================*/
void SetBattSideOutCurr_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo,UINT u32Curr)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x10020000;
    u32DataL = u32Curr;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL);

}
/*=================================================================================================
** @Name      : void SetOnOff_0x24(BYTE SGNo, BYTE u8OnOff)
** @Input     : SGNo:组号  u8OnOff:开关机状态 1为关机，0为开机
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置模块开关机 0xA1 为关机 0xA0 为开机
**===============================================================================================*/
void SetOnOff_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8OnOff)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x11100000;
    if(u8OnOff == 0)
    {
        u32DataL = 0x000000A0;
    }
    else
    {
        u32DataL = 0x000000A1;
    }

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL);

}
/*=================================================================================================
** @Name      : void SetBattSideCurrLimit_0x24(BYTE SGNo, UINT u32Curr)
** @Input     : SGNo:组号  u32Curr:电流值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧限流值
**===============================================================================================*/
void SetBattSideCurrLimit_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Curr)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x11310000;
    u32DataL = u32Curr;

    printf("u32Curr = %d\n", u32Curr);

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL);

}
/*=================================================================================================
** @Name      : void SetStartupMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置起机模式  A0-直流起机模式 A1-交流起机模式
**===============================================================================================*/
void SetStartupMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x11270000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 	/*A0-直流起机模式 A1-交流起机模式*/
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetBattSideCutOffVolt_0x24(BYTE SGNo, UINT u32CutOffVolt)
** @Input     : SGNo:组号 u32CutOffVolt:截止电压
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置电池侧放电截止电压
**===============================================================================================*/
void SetBattSideCutOffVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32CutOffVolt)
{
    float ftemp;
    UINT u32DataH, u32DataL;

    BYTE u8CMD = 0x24;
    u32DataH = 0x11320000;

    //20211102 lkf 1KV22KW模块，涉及模块内部串并联模式切换，因电池电压最低不于520V，所以工作电压不能低于520V
    //20240512 dqb 1KV22KW模块，直流侧电压下发900，否则会出现Vo设置偏低
    
    //ftemp = PData->fRect_GRP_NeedOutVolt[SGNo];

    u32DataL = u32CutOffVolt;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetACOutPhaseVolt_0x24(BYTE SGNo, UINT u32Volt)
** @Input     : SGNo:组号 u32Volt:交流输出相电压mV
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置交流输出相电压mV
**===============================================================================================*/
void SetACOutPhaseVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Volt)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x21060000;
    u32DataL = u32Volt;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetModeACFreq_0x24(BYTE SGNo, UINT u32Freq)
** @Input     : SGNo:组号 u32Freq:交流输出频率mHZ
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置交流输出频率mHZ
**===============================================================================================*/
void SetModeACFreq_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Freq)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x21070000;
    u32DataL = u32Freq;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetOnOffGridWrokMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置工作模式  A0-整流(默认) A1-并网逆变 A2-离网
**===============================================================================================*/
void SetOnOffGridWrokMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x21100000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 	/*A0-整流(默认) A1-并网逆变 A2-离网*/
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetEnergyStorMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文 
** @The notes : 外部协议 CMD = 0X24：设置储能模式使能 A0-不使能  A1-使能 
**===============================================================================================*/
void SetEnergyStorMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x21180000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode;
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}

/*=================================================================================================
** @Name      : void SetOnOffGridSysType_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置并离网系统  A0-并离网系统；A1-纯并网系统
**===============================================================================================*/
void SetOnOffGridSysType_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x21410000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 	/*A0-并离网系统；A1-纯并网系统*/
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetEnergyDispatchMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置能量调度模式   A1-交流调度，A0-直流调度  
**===============================================================================================*/
void SetEnergyDispatchMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x31110000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 	/*A1-交流调度 A0-直流调度 */
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetChargMode_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置充电模式模式   A1-恒功率模式，A0-恒流模式  
**===============================================================================================*/
void SetChargMode_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x31120000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 	/*A1-恒功率模式，A0-恒流模式 */
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetActivePower_0x24(BYTE SGNo, BYTE u8GSNo, UINT u32Power)
** @Input     : SGNo:组号  u8GSNo:设备号  u32Power:有功功率设置值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置交流有功功率（mW）  按照单模块的有功功率设置值下发
**===============================================================================================*/
void SetActivePower_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, UINT u32Power)
{
    UINT u32DataH, u32DataL;

    BYTE u8CMD = 0x24;
    u32DataH = 0x31230000;
    
    u32DataL = u32Power;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetRePowerEnable_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置无功使能   A1-无功使能，A0-禁止 
**===============================================================================================*/
void SetRePowerEnable_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x31DD0000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 	/*A1-无功使能，A0-禁止 */
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetRePowerType_0x24(BYTE SGNo, BYTE u8GSNo, BYTE Mode)
** @Input     : SGNo:组号  u8GSNo:设备号  Mode:模式
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置无功功率设置类型   
                0xA0：不设置无功功率输出功能 （默认）
                0xA1：通过PF设置命令 ,对应的控制命令小类为0x21 05
                0xA2：通过无功功率设置命令，对应的控制命令小类为0x21 08
                0xA3：N2003 离网老化占用，增加AC过载阈值以便满足老化要求，不对客户开放
                0xA4：通过cos(p)自动调节无功模式 -4105要求
                0xA5：通过Q(U)自动调节无功模式 -4105要求                                         
                0xA6：active power-Reactive power模式-IEEE1547要求 
**===============================================================================================*/
void SetRePowerType_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, BYTE Mode)
{
    UINT u32DataH, u32DataL;
    __XXX_UNION_VALUE untemp;

    BYTE u8CMD = 0x24;
    u32DataH = 0x21170000;

    untemp.u32Value = 0;
    untemp.chtemp[0] = Mode; 
    u32DataL = untemp.u32Value;

    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetRePower_0x24(BYTE SGNo, BYTE u8GSNo)
** @Input     : SGNo:组号  u8GSNo:设备号 u32RePower:无功功率值
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置无功功率（正负代表感性容性）（mW）		
**===============================================================================================*/
void SetRePower_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, BYTE u8GSNo, UINT u32RePower)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x31600000;
    u32DataL = u32RePower;
    
    u8PackSendMsg(pCCU, u8GSNo, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetGrp_AdjVolt_0x24(BYTE SGNo)
** @Input     : SGNo:组号
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置DCDC模块直流高压侧（母线侧）电压(mV)
**===============================================================================================*/
void SetGrp_AdjVolt_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT volt)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x41010000;
    u32DataL = volt;
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SetGrp_AdjCurr_0x24(BYTE SGNo, UINT u32Curr)
** @Input     : SGNo:组号  u32Curr:电流值 
** @Output    : void
** @Function  : 向模块发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置DCDC模块直流高压侧（母线侧）电流(mA)
**===============================================================================================*/
void SetGrp_AdjCurr_0x24(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32Curr)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x41020000;
    u32DataL = 0;
    
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=============================================================================================
** @Name      : void ManualClearPCSFaltCmd(void)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 向模块发送设置信息报文 外部协议 CMD = 0X24
** @The notes : 故障清除：0xA0-不清除(默认) 0xA1-故障清除(锁死故障清除，不下电重新开机)
**===========================================================================================*/
void ManualClearPCSFaltCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    UINT u32DataH, u32DataL;
    BYTE u8CMD = 0x24;

    u32DataH = 0x714C0000;
    u32DataL = 0x000000A1;

    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0B, u8CMD, SGNo, u32DataH, u32DataL); 
}
/*=================================================================================================
** @Name      : void SendAddressingCmd(void)
** @Input     : void
** @Output    : void
** @Function  : 模块之间互相通信的通讯协议 
** @The notes : 发送并机信息
**===============================================================================================*/
void SendAddressingCmd(ROUGH_CCU_DATA* pCCU)
{
    canmsg SendMsg;

    SendMsg.Id = 0x757f803;

    SendMsg.Data[0] = 0x04000001;
    SendMsg.Data[1] = 0x00;

    if (TxCAN3Buff(pCCU, &SendMsg, 8) != TRUE)
    {
        printf("TxCAN3Buff failed in SendAddressingCmd\r\n");
    }
}
/*=================================================================================================
** @Name      : void SendRectSillenceModeBroadcast(BYTE byRectAddress, BYTE u8Data)
** @Input     : void
** @Output    : void
** @Function  : 屏蔽模块DC CAN数据  0xA0-模块间数据正常 0xA1-屏蔽模块均流数据 0xA2-屏蔽模块间数据
** @The notes : 
**===============================================================================================*/
void SendRectSillenceModeBroadcast(ROUGH_CCU_DATA* pCCU, BYTE byRectAddress, BYTE u8Data)
{
    canmsg SendMsg;
    SendMsg.Id = 0x02A4FAF0;
    SendMsg.Data[0] = (0x11 << 24) + (0x1E << 16);
    SendMsg.Data[1] = u8Data;		//before OTA:0xA0	//after OTA:0xA1

    if (TxCAN3Buff(pCCU, &SendMsg, 8) != TRUE)
    {
        printf("TxCAN3Buff failed in SendRectSillenceModeBroadcast\r\n");
    }
}



/*----------------------------------------------ABU----------------------------------------------*/
/*=================================================================================================
** @Name      : void ABU_ReadACLineVoltage_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块交流线电压
**===============================================================================================*/
void ABU_ReadACLineVoltage_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_AB)
    {
        dword.b[2] = 0x03;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x04;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x05;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x11;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ReadPowerTemperature_0x23(void)
** @Input     : void
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块环境温度
**===============================================================================================*/
void ABU_ReadPowerTemperature_0x23(ROUGH_CCU_DATA* pCCU)
{
    co_dword dword;

    dword.v = 0u;
    dword.b[3] = 0x11;
    dword.b[2] = 0x06;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}

/*=================================================================================================
** @Name      : void ABU_Set_OnOff(BYTE Cmd)
** @Input     : Cmd：设置值
** @Output    : void
** @Function  : 向ABU发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置ABU开关机
**===============================================================================================*/
void ABU_Set_OnOff(ROUGH_CCU_DATA* pCCU, BYTE Cmd)
{
    co_dword dword;
    co_dword dword1;

    dword.v = 0u;
    dword.b[3] = 0x11;
    dword.b[2] = 0x10;

    dword1.v = 0u;
    if (Cmd == eType_Off)
    {
        dword1.b[0] = 0xA1;
    }
    else
    {
        dword1.b[0] = 0xA0;
    }

    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x24, dword.v, dword1.v); 
}
/*=================================================================================================
** @Name      : void ABU_CheckABUIsConfig(void)
** @Input     : void
** @Output    : void
** @Function  : 检查ABU是否使能
** @The notes : 
**===============================================================================================*/
void ABU_CheckABUIsConfig(ROUGH_CCU_DATA* pCCU)
{
    co_dword dword;

    // if(ABU->u8ConfigDelay > 60)
    // {
    //     ABU->u8ConfigDelay = 0;
    // }
    // else
    // {
    //     return;
    // }

    //check ABU soft version 
    dword.v = 0u;
    dword.b[3] = 0x11;
    dword.b[2] = 0x22;
    // ABU->ConfigFailed = FALSE;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ReadSoftWareVersion(BYTE ch)
** @Input     : ch：
** @Output    : void
** @Function  : 读取ABU软件版本号
** @The notes : 
**===============================================================================================*/
void ABU_ReadSoftWareVersion(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
      co_dword dword;

      dword.v = 0u;
      dword.b[3] = 0x11;
      if (ch == 0u)
      {
          dword.b[2] = 0x22;		//DC软件版本号
      }
      else if (ch == 1u)
      {
          dword.b[2] = 0x23;		//AC软件版本号
      }
      ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0);
}
/*=================================================================================================
** @Name      : void ABU_ReadACVoltage_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块交流A、B、C相相电压
**===============================================================================================*/
void ABU_ReadACVoltage_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_AB)
    {
        dword.b[2] = 0x01;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x02;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x03;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ReadACCurrent_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取模块交流A、B、C相电流
**===============================================================================================*/
void ABU_ReadACCurrent_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_AB)
    {
        dword.b[2] = 0x04;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x05;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x06;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ReadACFrequency_0x23(void)
** @Input     : void
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取交流频率
**===============================================================================================*/
void ABU_ReadACFrequency_0x23(ROUGH_CCU_DATA* pCCU)
{
    co_dword dword;

    dword.v = 0u;
    dword.b[3] = 0x21;
    dword.b[2] = 0x07;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ReadActivePower_0x23(BYTE ch)
** @Input     : void
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总有功功率,A、B、C相有功功率（mW）
**===============================================================================================*/
void ABU_ReadActivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_Total)
    {
        dword.b[2] = 0x08;
    }
    else if (ch == eType_AB)
    {
        dword.b[2] = 0x09;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x0A;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x0B;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0);
}
/*=================================================================================================
** @Name      : void ABU_ReadReactivePower_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总无功功率,A、B、C相无功功率（mW）
**===============================================================================================*/
void ABU_ReadReactivePower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_Total)
    {
        dword.b[2] = 0x0C;
    }
    else if (ch == eType_AB)
    {
        dword.b[2] = 0x0D;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x0E;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x0F;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ApparentPower_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总视在功率,A、B、C相视在功率（mW）
**===============================================================================================*/
void ABU_ApparentPower_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_Total)
    {
        dword.b[2] = 0x10;
    }
    else if (ch == eType_AB)
    {
        dword.b[2] = 0x11;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x12;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x13;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_ReadPowerFactor_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：读取总功率因素,A、B、C相功率因素（0.001）
**===============================================================================================*/
void ABU_ReadPowerFactor_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    if (ch == eType_Total)
    {
        dword.b[2] = 0x14;
    }
    else if (ch == eType_AB)
    {
        dword.b[2] = 0x15;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x16;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x17;
    }
    else
    {
        return;
    }

    dword.b[3] = 0x21;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}

/*=================================================================================================
** @Name      : void ABU_Read_status(BYTE Byte7, BYTE Byte6)
** @Input     : Byte7：报送报文高八位  Byte6：发送报文低八位
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：（这条内部外部协议里面都没有）
**===============================================================================================*/
void ABU_Read_status(ROUGH_CCU_DATA* pCCU, BYTE Byte7, BYTE Byte6)
{
    co_dword dword;

    dword.v = 0u;
    dword.b[3] = Byte7;
    dword.b[2] = Byte6;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}
/*=================================================================================================
** @Name      : void ABU_Read_alm(BYTE Byte7, BYTE Byte6)
** @Input     : Byte7：报送报文高八位  Byte6：发送报文低八位
** @Output    : void
** @Function  : 向ABU发送读信息报文，并读取解析回复报文
** @The notes : 外部协议 CMD = 0X23：（这条内部外部协议里面都没有）
**===============================================================================================*/
void ABU_Read_alm(ROUGH_CCU_DATA* pCCU, BYTE Byte7, BYTE Byte6)
{
    co_dword dword;

    dword.v = 0u;
    dword.b[3] = Byte7;
    dword.b[2] = Byte6;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}

/*=================================================================================================
** @Name      : void ABU_Set_OnOffGridMode(BYTE Cmd)
** @Input     : Cmd：设置值
** @Output    : void
** @Function  : 向ABU发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置ABU工作模式，并网逆变/离网（模块协议里无）
**===============================================================================================*/
void ABU_Set_OnOffGridMode(ROUGH_CCU_DATA* pCCU, BYTE Cmd)
{
    co_dword dword;
    co_dword dword1;

    dword.v = 0u;
    dword.b[3] = 0x21;
    dword.b[2] = 0x10;

    dword1.v = 0u;
    dword1.b[0] = Cmd;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x24, dword.v, dword1.v); 
}
/*=================================================================================================
** @Name      : void ABU_Set_OnOffGridModeSwitchAuto(BYTE Cmd)
** @Input     : Cmd：设置值
** @Output    : void
** @Function  : 向ABU发送设置信息报文
** @The notes : 外部协议 CMD = 0X24：设置ABU并离网自动切换模式（模块协议里无）
**===============================================================================================*/
void ABU_Set_OnOffGridModeSwitchAuto(ROUGH_CCU_DATA* pCCU, BYTE Cmd)
{
    co_dword dword;
    co_dword dword1;

    dword.v = 0u;
    dword.b[3] = 0x21;
    dword.b[2] = 0x19;

    dword1.v = 0u;
    dword1.b[0] = Cmd;
    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x24, dword.v, dword1.v); 
}

/*=================================================================================================
** @Name      : void ABU_ReadOutputVolt_0x23(BYTE ch)
** @Input     : ch：通道
** @Output    : void
** @Function  : 输出A、B、C相电压
** @The notes : 协议里面是预留？？？？
**===============================================================================================*/
void ABU_ReadOutputVolt_0x23(ROUGH_CCU_DATA* pCCU, BYTE ch)
{
    co_dword dword;

    dword.v = 0u;
    dword.b[3] = 0x31;
    if (ch == eType_AB)
    {
        dword.b[2] = 0x12;
    }
    else if (ch == eType_BC)
    {
        dword.b[2] = 0x13;
    }
    else if (ch == eType_CA)
    {
        dword.b[2] = 0x14;
    }

    ABU_PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, dword.v, 0); 
}


/*----------------------------------------------EEPROM----------------------------------------------*/

/*=================================================================================================
** @Name      : void ReadModuleRAMCmd(BYTE SGNo)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 读取模块RAM信息，地址为0x89E0
** @The notes : 读取当前模块的实际运行信息(故障录波过程中停止读取)
**===============================================================================================*/
void ReadModuleRAMCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo)
{
    canmsg SendMsg;
    __XXX_UNION_VALUE untemp;
    
    untemp.chtemp[3] = 0x02;
    untemp.chtemp[2] = 0x71;
    untemp.chtemp[1] = SGNo;
    untemp.chtemp[0] = 0xF0;
    SendMsg.Id = untemp.u32Value;
    
    SendMsg.Data[0] = 0x00000000;
    SendMsg.Data[1] = 0x000089E0;

    if (TxCAN3Buff(pCCU, &SendMsg, 8) != TRUE)
    {
        printf("TxCAN3Buff failed in ReadModuleRAMCmd\r\n");
    }
}
/*=================================================================================================
** @Name      : void ReadModuleACRAMCmd(BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 读取模块AC侧RAM信息，地址为0xA000-0x10000
** @The notes : 读取当前模块AC侧的RAM信息
**===============================================================================================*/
void ReadModuleACRAMCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress)
{
    canmsg SendMsg;
    __XXX_UNION_VALUE untemp;
    
    untemp.chtemp[3] = 0x02;
    untemp.chtemp[2] = 0x74;
    untemp.chtemp[1] = SGNo;
    untemp.chtemp[0] = 0xF0;
    SendMsg.Id = untemp.u32Value;
    
    SendMsg.Data[0] = u32StartAddress;
    SendMsg.Data[1] = u32EndAddress;

    if (TxCAN3Buff(pCCU, &SendMsg, 8) != TRUE)
    {
        printf("TxCAN3Buff failed in ReadModuleACRAMCmd\r\n");
    }
}
/*=================================================================================================
** @Name      : void ReadModuleDCRAMCmd(BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress)
** @Input     : SGNo：组号
** @Output    : void
** @Function  : 读取模块DC侧RAM信息，地址为0xA000-0x10000
** @The notes : 读取当前模块DC侧的RAM信息
**===============================================================================================*/
void ReadModuleDCRAMCmd(ROUGH_CCU_DATA* pCCU, BYTE SGNo, UINT u32StartAddress, UINT u32EndAddress)
{
    canmsg SendMsg;
    __XXX_UNION_VALUE untemp;
    
    untemp.chtemp[3] = 0x02;
    untemp.chtemp[2] = 0x75;
    untemp.chtemp[1] = SGNo;
    untemp.chtemp[0] = 0xF0;
    SendMsg.Id = untemp.u32Value;
    
    SendMsg.Data[0] = u32StartAddress;
    SendMsg.Data[1] = u32EndAddress;

    if (TxCAN3Buff(pCCU, &SendMsg, 8) != TRUE)
    {
        printf("TxCAN3Buff failed in ReadModuleDCRAMCmd\r\n");
    }
}

/*=================================================================================================
** @Name      : void N2402_SetModuleGrpNum(BYTE modAddr, BYTE grpNo)
** @Input     : modAddr - 模块地址（拨码值），grpNo - 组号
** @Output    : void
** @Function  : 设置 N2402 模块组号
** @The notes : 协议 02 A4 xx F0 11 03 00 00 00 00 yy yy
**===============================================================================================*/
void N2402_SetModuleGrpNum(ROUGH_CCU_DATA* pCCU, BYTE modAddr, BYTE grpNo)
{
    u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x24, modAddr, 0x11030000, grpNo);
}
