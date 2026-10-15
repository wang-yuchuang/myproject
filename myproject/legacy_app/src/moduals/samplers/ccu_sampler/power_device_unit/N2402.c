// #define _TRACE_PRINT_
#include "../ccu_sampler_main.h"
#include <string.h>

static ROUGH_CCU_DATA* pCCU = NULL;

static void CanRcvHandleMsg_ModuleGrp_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, BYTE u8Cmd, canmsg_rx *msg);
static void CanRcvReadReplyGrpMsg_0x23(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, canmsg_rx *msg);
static void CanRcvWriteReplyGrpMsg_0x24(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, canmsg_rx *msg);
void CanRcvHandleMsg_ModuleAddr_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, BYTE u8Cmd, canmsg_rx *msg);
static void Reflash_CAN_SelftAddr(ROUGH_CCU_DATA* pCCU);
static void Set235kwPCSPara(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSWrokMode_0X23(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSInfo_2S(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSInfo_8S(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSInfo_12S(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSInfo_18S(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSInfo_Fast(ROUGH_CCU_DATA* pCCU);
static void Read235kWPCSInfo_Slow(ROUGH_CCU_DATA* pCCU, BYTE step);

void CanRcvModuleMsgHandle_API(ROUGH_CCU_DATA* pCCU, can_frame_t *RcvMsg)
{
    Module_CanID id;
    canmsg_rx msg;

    msg.Id = RcvMsg->id;
    msg.Len = RcvMsg->len;
    for(BYTE i = 0; i < msg.Len; i++)
    {
        msg.Data[i] = RcvMsg->data[i];
    }

    if (msg.Len != 8)
    {
        return;
    }

    id.u8RcvDev = (BYTE)(msg.Id & 0x000000FF) - 1; // todo 836 暂时这么设置，之后设置为参数配置
    id.u8AimDev = (BYTE)((msg.Id & 0x0000FF00) >> 8);
    id.u8Cmd = (BYTE)((msg.Id & 0x003F0000) >> 16);
    id.u8DevNo = (BYTE)((msg.Id & 0x03C00000) >> 22);
    id.u8ErrorNum = (BYTE)((msg.Id & 0x1C000000) >> 26);

    // TRACEI("u8RcvDev = [%d], u8AimDev = [%d], u8Cmd = [%d], u8DevNo = [%d], u8ErrorNum = [%d]\n",\
    id.u8RcvDev, id.u8AimDev, id.u8Cmd, id.u8DevNo, id.u8ErrorNum);

    if(id.u8AimDev != R_PROTO_SELF_ADDR)
    {
        return;
    }

    // switch (pCCU->acdc_data.gs_ModSystemInfo.u8CfgModeType)
    // {
    //     case MODTYPE_ACDC_N2402:
    CANRcvMsgHandle_N2402(pCCU, id.u8RcvDev, id.u8Cmd, id.u8DevNo, id.u8ErrorNum, &msg);
    //         break;

    //     case MODTYPE_DCDC_N2302:
    //         CANRcvMsgHandle_N2302(id.u8RcvDev, id.u8Cmd, id.u8DevNo, id.u8ErrorNum, &msg);
    //         break;

    //     default:
    //         break;
    // }
}



void CANRcvMsgHandle_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvDev, BYTE u8Cmd, BYTE u8DevNo, BYTE u8ErrorNum, canmsg_rx *msg)
{
    if(u8ErrorNum != 0)
    {
        TRACEI("11111\n");
        return;
    }

    if(u8DevNo == R_PRPTO_DEVNO_0B)
    {//模块组数据
         TRACEI("22222\n");
        CanRcvHandleMsg_ModuleGrp_N2402(pCCU, u8RcvDev, u8Cmd, msg);
    }
    else if(u8DevNo == R_PRPTO_DEVNO_0A)
    {
         TRACEI("33333\n");
        CanRcvHandleMsg_ModuleAddr_N2402(pCCU, u8RcvDev, u8Cmd, msg);
    }
}

void CanRcvHandleMsg_ModuleAddr_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, BYTE u8Cmd, canmsg_rx *msg)
{
    if(u8Cmd != 0x23 && u8Cmd != 0x24)
    {
        TRACEI("[RX-Addr] 忽略非0x23/0x24命令, Mod%d, cmd=0x%02X\n", u8RcvSrcAddr, u8Cmd);
        return;
    }
    if(u8RcvSrcAddr >= MODNUM_N2402)
    {
        TRACEI("[RX-Addr] 地址越界: mod=%d, max=%d\n", u8RcvSrcAddr, MODNUM_N2402);
        return;
    }
    if(0)
    {
        return;
    }

    /* 标记模块存在 */
    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bIsExisted = TRUE;
    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bCommFail = FALSE;
    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u8CommFailTimes = 0;
    TRACEI("[RX-Addr] Mod%d 上线, 主命令=0x%02X, 子命令=0x%02X\n", u8RcvSrcAddr, msg->Data[0], msg->Data[1]);

    /* 数据解析 */
    switch(msg->Data[0])
    {
        case 0x21:
            switch(msg->Data[1])
            {
                case 0x10:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bGridModeSucc = TRUE;
                    TRACEI("[RX-Addr] Mod%d: 并离网模式设置成功\n", u8RcvSrcAddr);
                    break;
                case 0x18:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bStorModeSucc = TRUE;
                    TRACEI("[RX-Addr] Mod%d: 储能模式设置成功\n", u8RcvSrcAddr);
                    break;
                default: break;
            }
            break;
        case 0x31:
            if(msg->Data[1] == 0x11)
            {
                pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bDispatchModeSucc = TRUE;
                TRACEI("[RX-Addr] Mod%d: 调度模式设置成功\n", u8RcvSrcAddr);
            }
            break;
        case 0x11:
            switch(msg->Data[1])
            {
                case 0x20:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u16BelongedGrp = (USHORT)((msg->Data[6] << 8) + msg->Data[7]);
                    TRACEI("[RX-Addr] Mod%d: 所属组号=%d\n", u8RcvSrcAddr,
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u16BelongedGrp);
                    break;
                case 0x27:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bStartupModeSucc = TRUE;
                    TRACEI("[RX-Addr] Mod%d: 启机方式设置成功\n", u8RcvSrcAddr);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

static void CanRcvHandleMsg_ModuleGrp_N2402(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, BYTE u8Cmd, canmsg_rx *msg)
{
    switch(u8Cmd)
    {
        case 0x23:
            CanRcvReadReplyGrpMsg_0x23(pCCU, u8RcvSrcAddr, msg);
            break;
        case 0x24:
            CanRcvWriteReplyGrpMsg_0x24(pCCU, u8RcvSrcAddr, msg);
            break;
        case 0x11:
            /* 0x11: handled as Data[0] sub-command inside CanRcvReadReplyGrpMsg_0x23() */
            break;
        default:
            break;
    }
}

static void CanRcvReadReplyGrpMsg_0x23(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, canmsg_rx *msg)
{
    int u32Temp;
    union 
    {
        UINT u32buf;
        int i32buf;
    }union_temp;
    
    /* raw UINT from CAN buffer; consumer applies gain/scaling conversion */
    union_temp.u32buf = (msg->Data[4] << 24) + (msg->Data[5] << 16) + (msg->Data[6] << 8) + msg->Data[7];
    u32Temp = union_temp.i32buf;
    
    /* ICC_MINDEX_ACDC_MOD_INFO = 87，ACDC单模块信息，防止ExtMalloc分配失败后空指针访问 */
    // if (gPCSACDCModInfo_85 == NULL || gPCSACDCModInfo_86 == NULL)
    // {
    //     return;
    // }
    
    switch(msg->Data[0])
    {
        case 0x10:
            u8RcvSrcAddr--;
            if(u8RcvSrcAddr >= N2402_MAX_GRP_NUM)
            {
                return;
            }
            switch(msg->Data[1])
            {
                case 0x01:
                    pCCU->acdc_data.gPCSACDCModInfo[u8RcvSrcAddr][0].u32ModDCVolt = u32Temp;
                    TRACEI("[RX-Grp%d] 组直流侧电压=%umV\n", u8RcvSrcAddr+1, u32Temp);
                    break;
                case 0x02:
                    pCCU->acdc_data.gPCSACDCModInfo[u8RcvSrcAddr][0].u32ModDCCurr = u32Temp;
                    TRACEI("[RX-Grp%d] 组直流侧电流=%umA\n", u8RcvSrcAddr+1, u32Temp);
                    break;
                case 0x10:
                    {
                        USHORT modNo = (USHORT)((msg->Data[6] << 8) + msg->Data[7]);
                        pCCU->acdc_data.gPCSACDCModInfo[u8RcvSrcAddr][0].u32ModNo = modNo;
                        TRACEI("[RX-Grp%d] 模块数量=%d\n", u8RcvSrcAddr+1, modNo);
                    }
                    break;
                default:
                    break;
            }
            break;
            
        case 0x11:
            if(u8RcvSrcAddr >= MODNUM_N2402)
            {
                return;
            }
            if(0)
            {
                return;
            }
            switch(msg->Data[1])
            {
                case 0x01:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModDCVolt = u32Temp;
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bIsExisted = TRUE;
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bCommFail = FALSE;
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u8CommFailTimes = 0;
                    TRACEI("[RX-Mod%d] 直流侧电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x02:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModBattCurr = u32Temp;
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bIsExisted = TRUE;
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bCommFail = FALSE;
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u8CommFailTimes = 0;
                    TRACEI("[RX-Mod%d] 直流侧电流=%umA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x03:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACLineVolt_AB = u32Temp;
                    TRACEI("[RX-Mod%d] 交流AB线电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x04:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACLineVolt_BC = u32Temp;
                    TRACEI("[RX-Mod%d] 交流BC线电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x05:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACLineVolt_CA = u32Temp;
                    TRACEI("[RX-Mod%d] 交流CA线电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x06:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32EnvTemp = u32Temp;
                    TRACEI("[RX-Mod%d] 模块环温=%umC\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x12:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[0] = msg->Data[7];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[1] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[2] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[3] = msg->Data[4];
                    TRACEI("id[%08X],src[%d],1112:%02X%02X%02X%02X\n",
                        msg->Id,
                        u8RcvSrcAddr,
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[0],
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[1],
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[2],
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BYTE_FIELD[3]);
                    if(msg->Data[4] || msg->Data[5] || msg->Data[6] || msg->Data[7])
                    {
                        TRACEI("[RX-Mod%d] 状态表12=0x%02X%02X%02X%02X | 状态:运行码[%d] 离网[%d] 模式[%d] 开关机[%d] | 母线UVP[%d] 高低穿[%d] DOD[%d] EOD[%d] 参数错[%d] 继电错[%d] 软启错[%d] 辅源错[%d] | EPO[%d] ID重[%d] 不均流[%d] 风扇[%d] 过温[%d] 锁相[%d] 孤岛[%d] 通讯[%d] | 降载[%d] 过载[%d] 短路[%d] 内过流[%d] 母线[%d] 直流[%d] 交流[%d] 系统[%d]\n", u8RcvSrcAddr,
                            msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7],
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bRunState,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bOffGridSta,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bPCSWorkMode,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bPCSWorkStatus,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bBusUvp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bHorLVRTErr,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bBatDOD,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bBatEOD,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bUserSetErr,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bRlyBad,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bSoftFail,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bAuxPowerFail,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bEPOErr,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bIDDup,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bOutUnBal,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bFanErr,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bSysOtp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bPllFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bIslandPro,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bComFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bOPDerate,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bOPInc,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bOutShort,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bOcpInter,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bBusErr,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bDcFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bAcFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_12.BIT_FIELD.bSysFault);
                    }
                    break;
                case 0x13:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BYTE_FIELD[0] = msg->Data[7];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BYTE_FIELD[1] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BYTE_FIELD[2] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BYTE_FIELD[3] = msg->Data[4];
                    if(msg->Data[4] || msg->Data[5] || msg->Data[6] || msg->Data[7])
                    {
                        TRACEI("[RX-Mod%d] 状态表13告警=0x%02X%02X%02X%02X | 通讯:SCI[%d] SPI[%d] | AC:过压[%d] 欠压[%d] 过频[%d] 欠频[%d] 母线异[%d] 过压[%d] 欠压[%d] 没电[%d] 缺相[%d] 闪断[%d] 采样过流[%d] TZ过流[%d] 不均[%d] 相序错[%d] 锁相[%d] AC继[%d] 短路[%d] 孤岛[%d] ESTOP[%d] 电池反[%d] 出过压[%d] 出欠压[%d] 出短路[%d] 网继开[%d] 网继短[%d] 逆继开[%d] 逆继短[%d] 直继[%d] 同步[%d]\n", u8RcvSrcAddr,
                            msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7],
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bSCIComFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bSPIComFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACOverVoltage,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACUnderVoltage,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACOverFreq,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACUnderFreq,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bBusAbnormal,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bBusOverVoltage,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bBusUnderVoltage,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bInputNoPower,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bInputPhaseLoss,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bInputFlicker,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bSoftwareOverCurrent,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACTZOverCurrent,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bInputVolUnbalance,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bPhaseSeqError,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bPLLError_Bit7 | pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bPLLError_Bit6,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACRelayFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bACShortCircuit,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bIslandProtection,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bESTOP,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bBatteryReverse,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bOutputOverVoltage,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bOutputUnderVolProt,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bOutputShortWarn,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bGridRelayOpen,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bGridRelayShort,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bInvRelayOpen,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bInvRelayShort,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bDCRelayFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_13.BIT_FIELD.bSynchroAbnormal);
                    }
                    break;
                case 0x14:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BYTE_FIELD[0] = msg->Data[7];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BYTE_FIELD[1] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BYTE_FIELD[2] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BYTE_FIELD[3] = msg->Data[4];
                    if(msg->Data[4] || msg->Data[5] || msg->Data[6] || msg->Data[7])
                    {
                        TRACEI("[RX-Mod%d] 状态表14告警=0x%02X%02X%02X%02X | 并机通讯[%d] 直软继[%d] Bus软启[%d] 逆软启[%d] 输出不均[%d]\n", u8RcvSrcAddr,
                            msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7],
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BIT_FIELD.bParallelComFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BIT_FIELD.bDCRelaySoftStartFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BIT_FIELD.bBusSoftStartFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BIT_FIELD.bInvSoftStartFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_14.BIT_FIELD.bOutputCurrentImbalance);
                    }
                    break;
                case 0x15:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BYTE_FIELD[0] = msg->Data[7];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BYTE_FIELD[1] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BYTE_FIELD[2] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BYTE_FIELD[3] = msg->Data[4];
                    if(msg->Data[4] || msg->Data[5] || msg->Data[6] || msg->Data[7])
                    {
                        TRACEI("[RX-Mod%d] 状态表15告警=0x%02X%02X%02X%02X | 通讯:CAN[%d] | 温度:进水[%d] 出水[%d] IPMA[%d] IPMB[%d] IPMC[%d] IPMD[%d] 环温[%d] | 辅源[%d] 绝缘[%d] 漏流[%d] 风扇[%d] Vo超高[%d] Vo偏低[%d] ID重[%d] 软起[%d]\n", u8RcvSrcAddr,
                            msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7],
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bCANComFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bInletOverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bOutletOverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bIPMA_OverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bIPMB_OverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bIPMC_OverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bIPMD_OverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bAmbientOverTemp,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bAuxPowerFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bInsulationDetectFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bLeakageCurrentFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bFanFault,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bVoSettingOverProt,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bVoSettingLow,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bIDDuplication,
                            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].StatCode_15.BIT_FIELD.bSoftStartResOverTemp);
                    }
                    break;
                case 0x20:
                {
                    USHORT grp = (USHORT)((msg->Data[6] << 8) + msg->Data[7]);
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u16BelongedGrp = grp;
                    TRACEI("[RX-Mod%d] 所属组号=%d\n", u8RcvSrcAddr, grp);
                    break;
                }
                case 0x22:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32DC_Ver = u32Temp;
                    TRACEI("[RX-Mod%d] DC软件版本号=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x23:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32AC_Ver = u32Temp;
                    TRACEI("[RX-Mod%d] AC软件版本号=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x32:
                {
                    float fMaxCurr = (float)u32Temp;
                    if(fMaxCurr > 0.0f)
                    {
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].fMax_OutCurr = fMaxCurr / 1000.0f;
                        TRACEI("[RX-Mod%d] 最大输出电流=%.1fA\n", u8RcvSrcAddr, fMaxCurr / 1000.0f);
                    }
                    break;
                }
                case 0x33:
                {
                    float fRatedPower = (float)u32Temp;
                    if(fRatedPower > 0.0f)
                    {
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].fRatedPower = fRatedPower;
                        TRACEI("[RX-Mod%d] 额定功率=%.2fkW\n", u8RcvSrcAddr, fRatedPower / 1000.0f);
                    }
                    break;
                }
                case 0x24:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_24[0] = msg->Data[4];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_24[1] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_24[2] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_24[3] = msg->Data[7];
                    TRACEI("[RX-Mod%d] 条码0=%02X%02X%02X%02X\n", u8RcvSrcAddr,
                        msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7]);
                    break;
                case 0x25:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_25[0] = msg->Data[4];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_25[1] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_25[2] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_25[3] = msg->Data[7];
                    TRACEI("[RX-Mod%d] 条码1=%02X%02X%02X%02X\n", u8RcvSrcAddr,
                        msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7]);
                    break;
                case 0x26:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_26[0] = msg->Data[4];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_26[1] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_26[2] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_26[3] = msg->Data[7];
                    TRACEI("[RX-Mod%d] 条码2=%02X%02X%02X%02X\n", u8RcvSrcAddr,
                        msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7]);
                    break;
                case 0x27:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_27[0] = msg->Data[4];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_27[1] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_27[2] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_27[3] = msg->Data[7];
                    TRACEI("[RX-Mod%d] 条码3=%02X%02X%02X%02X\n", u8RcvSrcAddr,
                        msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7]);
                    break;
                case 0x28:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_28[0] = msg->Data[4];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_28[1] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_28[2] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_28[3] = msg->Data[7];
                    TRACEI("[RX-Mod%d] 条码4=%02X%02X%02X%02X\n", u8RcvSrcAddr,
                        msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7]);
                    break;
                case 0x29:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_29[0] = msg->Data[4];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_29[1] = msg->Data[5];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_29[2] = msg->Data[6];
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bBarcode_29[3] = msg->Data[7];
                    TRACEI("[RX-Mod%d] 条码5=%02X%02X%02X%02X\n", u8RcvSrcAddr,
                        msg->Data[4], msg->Data[5], msg->Data[6], msg->Data[7]);
                    break;
                default:
                    break;
            }
            break;
            
        case 0x21:
            if(u8RcvSrcAddr >= MODNUM_N2402)
            {
                return;
            }
            if(0)
            {
                return;
            }
            switch(msg->Data[1])
            {
                case 0x01:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACPhaseVolt_A = u32Temp;
                    TRACEI("[RX-Mod%d] 交流A相相电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x02:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACPhaseVolt_B = u32Temp;
                    TRACEI("[RX-Mod%d] 交流B相相电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x03:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACPhaseVolt_C = u32Temp;
                    TRACEI("[RX-Mod%d] 交流C相相电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x04:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACPhaseCurr_A = u32Temp;
                    TRACEI("[RX-Mod%d] 交流A相电流=%umA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x05:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACPhaseCurr_B = u32Temp;
                    TRACEI("[RX-Mod%d] 交流B相电流=%umA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x06:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACPhaseCurr_C = u32Temp;
                    TRACEI("[RX-Mod%d] 交流C相电流=%umA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x07:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModACFreq = u32Temp;
                    TRACEI("[RX-Mod%d] 交流频率=%umHz\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x08:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModActivePower_Total = u32Temp;
                    TRACEI("[RX-Mod%d] 总有功功率=%umW\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x09:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModActivePower_A = u32Temp;
                    TRACEI("[RX-Mod%d] A相有功功率=%umW\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x0A:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModActivePower_B = u32Temp;
                    TRACEI("[RX-Mod%d] B相有功功率=%umW\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x0B:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModActivePower_C = u32Temp;
                    TRACEI("[RX-Mod%d] C相有功功率=%umW\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x0C:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModReActivePower_Total = u32Temp;
                    TRACEI("[RX-Mod%d] 总无功功率=%umVar\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x0D:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModReActivePower_A = u32Temp;
                    TRACEI("[RX-Mod%d] A相无功功率=%umVar\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x0E:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModReActivePower_B = u32Temp;
                    TRACEI("[RX-Mod%d] B相无功功率=%umVar\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x0F:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModReActivePower_C = u32Temp;
                    TRACEI("[RX-Mod%d] C相无功功率=%umVar\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x10:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModApparentPower_Total = u32Temp;
                    TRACEI("[RX-Mod%d] 总视在功率=%umVA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x11:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModApparentPower_A = u32Temp;
                    TRACEI("[RX-Mod%d] A相视在功率=%umVA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x12:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModApparentPower_B = u32Temp;
                    TRACEI("[RX-Mod%d] B相视在功率=%umVA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x13:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModApparentPower_C = u32Temp;
                    TRACEI("[RX-Mod%d] C相视在功率=%umVA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x14:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModPowerFactor_Total = u32Temp;
                    TRACEI("[RX-Mod%d] 总功率因数=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x15:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModPowerFactor_A = u32Temp;
                    TRACEI("[RX-Mod%d] A相功率因数=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x16:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModPowerFactor_B = u32Temp;
                    TRACEI("[RX-Mod%d] B相功率因数=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x17:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModPowerFactor_C = u32Temp;
                    TRACEI("[RX-Mod%d] C相功率因数=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                default:
                    break;
            }
            break;
            
        default:
            break;
    }
}

static void CanRcvWriteReplyGrpMsg_0x24(ROUGH_CCU_DATA* pCCU, BYTE u8RcvSrcAddr, canmsg_rx *msg)
{
    UINT u32Temp;
    union 
    {
        UINT u32buf;
        int i32buf;
    }union_temp;
    
    union_temp.u32buf = (msg->Data[4] << 24) + (msg->Data[5] << 16) + (msg->Data[6] << 8) + msg->Data[7];
    u32Temp = union_temp.u32buf;
    
    switch(msg->Data[0])
    {
        case 0x21:
            switch(msg->Data[1])
            {
                case 0x10:
                    if(u8RcvSrcAddr < MODNUM_N2402 && 1)
                    {
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bGridModeSucc = TRUE;  /* 并离网模式设置成功 */
                        TRACEI("[0x24-RX] Mod%d: cmd=0x21 sub=0x10 并离网模式设置成功, val=%u\n", u8RcvSrcAddr, u32Temp);
                    }
                    break;
                case 0x18:
                    if(u8RcvSrcAddr < MODNUM_N2402 && 1)
                    {
                        pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bStorModeSucc = TRUE;  /* 储能模式设置成功 */
                        TRACEI("[0x24-RX] Mod%d: cmd=0x21 sub=0x18 储能模式设置成功, val=%u\n", u8RcvSrcAddr, u32Temp);
                    }
                    break;
                default:
                    TRACEI("[0x24-RX] Mod%d: cmd=0x21 未知子命令=0x%02X\n", u8RcvSrcAddr, msg->Data[1]);
                    break;
            }
            break;
        case 0x31:
            if(msg->Data[1] == 0x11 && u8RcvSrcAddr < MODNUM_N2402 && 1)
            {
                pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bDispatchModeSucc = TRUE;  /* 调度模式设置成功 */
                TRACEI("[0x24-RX] Mod%d: cmd=0x31 sub=0x11 调度模式设置成功, val=%u\n", u8RcvSrcAddr, u32Temp);
            }
            else
            {
                TRACEI("[0x24-RX] Mod%d: cmd=0x31 sub=0x%02X 调度模式设置失败/忽略\n", u8RcvSrcAddr, msg->Data[1]);
            }
            break;
        case 0x11:
            if(u8RcvSrcAddr >= MODNUM_N2402)
            {
                TRACEI("[0x24-RX] Mod%d: cmd=0x11 地址越界, max=%d\n", u8RcvSrcAddr, MODNUM_N2402);
                return;
            }
            if(0)
            {
                return;
            }
            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bIsExisted = TRUE;
            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bCommFail = FALSE;
            pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u8CommFailTimes = 0;
            switch(msg->Data[1])
            {
                case 0x01:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModDCVolt = u32Temp;
                    TRACEI("[0x24-RX] Mod%d: cmd=0x11 sub=0x01 直流侧电压=%umV\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x02:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].u32ModBattCurr = u32Temp;
                    TRACEI("[0x24-RX] Mod%d: cmd=0x11 sub=0x02 电池电流=%umA\n", u8RcvSrcAddr, u32Temp);
                    break;
                case 0x27:
                    pCCU->acdc_data.gs_N2402RectInfo[u8RcvSrcAddr].bStartupModeSucc = TRUE;  /* 启机方式设置成功 */
                    TRACEI("[0x24-RX] Mod%d: cmd=0x11 sub=0x27 启机方式设置成功, val=%u\n", u8RcvSrcAddr, u32Temp);
                    break;
                default:
                    TRACEI("[0x24-RX] Mod%d: cmd=0x11 未知子命令=0x%02X\n", u8RcvSrcAddr, msg->Data[1]);
                    break;
            }
            break;
        default:
            TRACEI("[0x24-RX] Mod%d: 未知主命令=0x%02X\n", u8RcvSrcAddr, msg->Data[0]);
            break;
    }
}

/************************************************************************************************************** */
/************************************************************************************************************** */
/************************************************************************************************************** */
/************************************************************************************************************** */
/************************************************************************************************************** */
/************************************************************************************************************** */
/************************************************************************************************************** */

/*=================================================================================================
** @Name      : void InitN2402Info(void)
** @Input     : void
** @Output    : void
** @Function  : 初始化N2402相关数据
** @The notes : 
**===============================================================================================*/
void InitN2402Info(void)
{
    /* ICC_MINDEX_ACDC_MOD_INFO = 84，ACDC单模块信息 */
    /* 设置CAN接收路由类型为N2402，否则所有CAN回复在CanRcvModuleMsgHandle_API被丢弃 */
    if(1)
    {
        pCCU->acdc_data.gs_ModSystemInfo.u8CfgModeType = MODTYPE_ACDC_N2402;
    }

    pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSWrokModeDelay = 1;
    pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSWrokModeInterval = 5;
    pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeDelay = 1;
    pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeInterval = 5;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SDelay = 5;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SInterval = 10;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SDelay = 1;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SInterval = 20;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SDelay = 10;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SInterval = 10;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SDelay = 10;
    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SInterval = 20;
    pCCU->acdc_data.gs_SysN2402Info.u16DelayCount_5S = 500;
    pCCU->acdc_data.gs_SysN2402Info.u16DelayCountInterval_5S = 500;
    pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeDelay = 1;        //FIX: was never initialized
    pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeInterval = 10;    //FIX: was never initialized

}



void SetPcsparameter(ROUGH_CCU_DATA* pCCU, UINT SGNo)
{
    USHORT dispatchMode = 0; //1  交流调度
    USHORT startupMode  = 0; //1 交流启机
    USHORT gridMode     = 0; //并网整流
    USHORT energyStor   = 1; //

    BYTE canDispatch = (pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode == 1) ? 0xA1 : 0xA0; //0是直流调度，对应A0直流调度
    BYTE canStartup  = (pCCU->pNewSet.ccuSettings.nStartupMode == 1)  ? 0xA1 : 0xA0; //0是直流启机，对应A0
    BYTE canGrid     = (pCCU->pNewSet.ccuSettings.nworkMode == 1)     ? 0xA2 : 0xA0; //默认整流就行
    BYTE canStor     = (pCCU->pNewSet.ccuSettings.nessMode == 1)   ? 0xA1 : 0xA0; 

    SetEnergyDispatchMode_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, canDispatch);
    SetStartupMode_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, canStartup);
    SetOnOffGridWrokMode_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, canGrid);
    SetEnergyStorMode_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, canStor);

    // /* 校验PCS模块是否确认模式参数（等待组内任一模块回复） */
    {
        for(BYTE m = 0; m < MODNUM_N2402; m++)
        {
            if(pCCU->acdc_data.gs_N2402RectInfo[m].bIsExisted
                && pCCU->acdc_data.gs_N2402RectInfo[m].u16BelongedGrp == (SGNo + 1))  /* 组号从1开始 */
            {
                if(pCCU->acdc_data.gs_N2402RectInfo[m].bStartupModeSucc
                && pCCU->acdc_data.gs_N2402RectInfo[m].bGridModeSucc
                && pCCU->acdc_data.gs_N2402RectInfo[m].bStorModeSucc
                && pCCU->acdc_data.gs_N2402RectInfo[m].bDispatchModeSucc)
                {
                    pCCU->bAllConfirmed = TRUE;
                    break;
                }
            }
        }
    }
}

/*=============================================================================================
** @Name      : static BYTE Set235kWPCS_WrokMode(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块充放电设置（开机前检查故障，无故障才开机）
** @The notes : 协议要求: 查询组信息->检查故障->设置参数->开机; 故障时发关机
**===========================================================================================*/
void Set235kWPCS_WrokMode(ROUGH_CCU_DATA* pCCU, UINT SGNo, t_ACDCControlData ctrlData)
{
    SGNo -= 1;

    if(SGNo < 0 || SGNo > 2)
    {
        printf("不存在组号 %d\n", SGNo);
    }

    printf("SGNo = [%d]\n", SGNo);

    if(!pCCU->bAllConfirmed)
    {
            printf("停止开机 SGNo = [%d]\n", SGNo);
        return;  /* 模式参数未全部确认，暂缓开机 */
    }

    // if(直流)
    {
        pCCU->strLSYSInfo.blobGrpRectData.emCtrlStatus[SGNo] = (CTRL_STATUS)ctrlData.eWorkMode;
        pCCU->strLSYSInfo.blobGrpRectData.fCtrlVolt[SGNo] = (float)ctrlData.u32CtrlVolt*0.001f;
        pCCU->strLSYSInfo.blobGrpRectData.fCtrlCurr[SGNo] = (float)(ABS((int)ctrlData.u32CtrlCurr)) * 0.001f;
    }
        // todo 交流调度没做
    //else {}

    printf("u32CtrlVolt = [%d], u32CtrlCurr = [%d], u32CtrlPower = [%d]\n",\
    ctrlData.u32CtrlVolt, ctrlData.u32CtrlCurr, ctrlData.u32CtrlPower);
    /* 根据eWorkMode和调度模式选择控制策略 */
    if (ctrlData.eWorkMode == eCDMode_Stop)
    {
        printf("停机\n");
        SetBattSideOutVolt_0x24(pCCU, SGNo, 0);
        SetBattSideOutCurr_0x24(pCCU, SGNo, 0);
        SetActivePower_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, 0);
        SetOnOff_0x24(pCCU, SGNo, 1);  /* 关机 */
        return;
    }
    else if (pCCU->pNewSet.ccuSettings.byDCorAC_AdjMode == 0)
    {
        printf("充放\n");
        /* 直流调度: 电压/电流/功率控制（模块按最小值输出） */
        SetBattSideOutVolt_0x24(pCCU, SGNo, ctrlData.u32CtrlVolt);
        SetBattSideOutCurr_0x24(pCCU, SGNo, ctrlData.u32CtrlCurr);  //负充  正放
        // SetBattSideCurrLimit_0x24(pCCU, SGNo, (UINT)(ctrlData.u32CtrlCurr > 2000000000 ? (-(int)ctrlData.u32CtrlCurr) : ctrlData.u32CtrlCurr));
        SetActivePower_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, ctrlData.u32CtrlPower);
    }
    else
    {
        printf("交流调度");
        /* 交流调度: 功率控制 (充电用负功率, 放电用正功率) */
        SetActivePower_0x24(pCCU, SGNo, R_PRPTO_DEVNO_0B, ctrlData.u32CtrlPower);
        SetBattSideOutCurr_0x24(pCCU, SGNo, 0);
    }

    /* 设置工作模式 */
    if(pCCU->acdc_data.gs_SysN2402Info.u8TaskCtrl235KwPCSCMD[SGNo] == IMMU2ManualCtrl235kwCmd_Chging)
    {
    }
    else if(pCCU->acdc_data.gs_SysN2402Info.u8TaskCtrl235KwPCSCMD[SGNo] == IMMU2ManualCtrl235kwCmd_Dischging)
    {
    }
    else if(pCCU->acdc_data.gs_SysN2402Info.u8TaskCtrl235KwPCSCMD[SGNo] == IMMU2ManualCtrl235kwCmd_OffGrid)
    {
    }
    else
    {
        printf("开机\n");
        /* 直接开机，模块有故障会自己关机 */
        SetOnOff_0x24(pCCU, SGNo, 0);  /* 开机 */
    }
}
/*=============================================================================================
** @Name      : static void Reflash_CAN_SelftAddr(void)
** @Input     : void
** @Output    : void
** @Function  : 设置CAN报文地址和组号（ACDC组1=地址1，组2=地址2）
** @The notes : u8PackSendMsg 使用 byGrpIdx 作为目标地址
**===========================================================================================*/
static void Reflash_CAN_SelftAddr(ROUGH_CCU_DATA* pCCU)
{
    // static BYTE firstFlag = 1;
    // if (firstFlag)
    if (1)
    {
        // firstFlag = 0;
        // if(pCCU->pNewSet.ccuSettings.byPCSType == 3)
        {
        pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[0] = 1;  /* ACDC组1目标地址=1 */
        pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[1] = 2;  /* ACDC组2目标地址=2 */
        // pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[0] = 0;  /* ACDC组1目标地址=0 */
        // pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[1] = 1;  /* ACDC组2目标地址=1 */
        }
        pCCU->acdc_data.gs_ModSystemInfo.AddrInfo[0] = 0;   /* 单模块地址(拨码) */
        pCCU->acdc_data.gs_ModSystemInfo.AddrInfo[1] = 1;
        pCCU->acdc_data.gs_ModSystemInfo.AddrInfo[2] = 2;
        pCCU->acdc_data.gs_ModSystemInfo.AddrInfo[3] = 3;
        /* 设置所有PCS模块为地址自动分配方式（拨码确定组号） */ // 如果 组1barcode 排序在 组2barcode 之前，单模块页面会乱? 处理会很麻烦
        // 改用 拨码 设置 地址，再设置组号
        canmsg SendMsg;
        {
            SendMsg.Id = (R_PRPTO_DEVNO_0B << 22) | (0x24 << 16) | (0x3F << 8) | R_PROTO_SELF_ADDR;
            SendMsg.Data[0] = 0x11230000;
            SendMsg.Data[1] = 0xA1;
            TxCAN3Buff(pCCU, &SendMsg, 8);
        }
		Sleep(50);
        {
            SendMsg.Id = (R_PRPTO_DEVNO_0B << 22) | (0x24 << 16) | (0x01 << 8) | R_PROTO_SELF_ADDR;
            SendMsg.Data[0] = 0x11030000;
            SendMsg.Data[1] = pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[0];
            TxCAN3Buff(pCCU, &SendMsg, 8);
        }
		Sleep(50);
        {
            SendMsg.Id = (R_PRPTO_DEVNO_0B << 22) | (0x24 << 16) | (0x02 << 8) | R_PROTO_SELF_ADDR;
            SendMsg.Data[0] = 0x11030000;
            SendMsg.Data[1] = pCCU->acdc_data.gs_ModSystemInfo.byGrpIdx[1];
            TxCAN3Buff(pCCU, &SendMsg, 8);
        }


    }
}

/*=============================================================================================
** @Name      : static void Set235kwPCSPara(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块设置参数
** @The notes : 所有向235KW模块发送的除了充放电以外的其它设置类报文都在这个函数内
**===========================================================================================*/
static void Set235kwPCSPara(ROUGH_CCU_DATA* pCCU)
{
	BYTE iGIdx = 0;
    int i;
	if (pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeDelay == 0)
	{
		pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeDelay = pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeInterval;
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			SettModePara_Time(pCCU, iGIdx, R_PRPTO_DEVNO_0B);
		}
        for(i = 1; i <= 2; i++)
        {
            SetPcsparameter(pCCU, i);
        }
	}
}
/*=============================================================================================
** @Name      : static void Read235kWPCSWrokMode_0X23(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块读取
** @The notes : 所有向235KW模块发送的读取类报文都在这个函数内
**===========================================================================================*/
static void Read235kWPCSWrokMode_0X23(ROUGH_CCU_DATA* pCCU)
{
	//2S周期读任务
	if (pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SDelay == 0)
	{
		pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SDelay = pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SInterval;

		Read235kWPCSInfo_2S(pCCU);
	}
	//8S周期读任务
	if (pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SDelay == 0)
	{
		pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SDelay = pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SInterval;

		Read235kWPCSInfo_8S(pCCU);
		for (BYTE i = 0; i < N2402_MAX_GRP_NUM; i++)
		{
			GetGrp_MaxCapacity_0x23(pCCU, i);
			GetGrp_MaxCurr_0x23(pCCU, i);
		}
	}
	//12S周期读任务
	if (pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SDelay == 0)
	{
		pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SDelay = pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SInterval;

		Read235kWPCSInfo_12S(pCCU);
	}
	//18S周期读任务
	if (pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SDelay == 0)
	{
		pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SDelay = pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SInterval;

		Read235kWPCSInfo_18S(pCCU);
	}
}

/*=============================================================================================
** @Name      : static void Read235kWPCSInfo_2S(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块500ms周期信息读取任务0X23，2S周期读完一轮
** @The notes : 模块直流侧电压/电流1101、1102，逆变状态-1112、1113、1114、1115
**===========================================================================================*/
static void Read235kWPCSInfo_2S(ROUGH_CCU_DATA* pCCU)
{
    /* ICC_MINDEX_ACDC_MOD_INFO = 84，ACDC单模块信息 */

	BYTE iGIdx = 0;
	BYTE u8Idx;
	static BYTE s_u8Step_2S = 0;

	switch (s_u8Step_2S)
	{
		case READ_INFO_CMD_2S_NO_0:
		case READ_INFO_CMD_2S_NO_1:
		case READ_INFO_CMD_2S_NO_2:
		case READ_INFO_CMD_2S_NO_3:
			s_u8Step_2S++;
			break;
		default:
			break;
	}
	if(s_u8Step_2S >= READ_INFO_CMD_2S_NO_MAX)
	{
		s_u8Step_2S = 0; 
	}
	
}
/*=============================================================================================
** @Name      : static void Read235kWPCSInfo_8S(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块8S周期信息读取任务 0X23 4S进入一次
** @The notes : 模块数1010、模块环境温度1106
**===========================================================================================*/
static void Read235kWPCSInfo_8S(ROUGH_CCU_DATA* pCCU)
{
    /* ICC_MINDEX_ACDC_MOD_INFO = 84，ACDC单模块信息 */

	BYTE iGIdx = 0;
	BYTE u8Idx;
	static BYTE s_u8Step_8S = 0;

	switch (s_u8Step_8S)
	{
		case READ_INFO_CMD_8S_NO_0:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				GetModeInfo_ModeNum(pCCU, iGIdx);
			}
			s_u8Step_8S++;
		break;

		case READ_INFO_CMD_8S_NO_1:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				GetModeInfo_ModeAmbTmp(pCCU, iGIdx, R_PRPTO_DEVNO_0B);
			}
			s_u8Step_8S++;
		break;

		default:
		break;
	}
	if(s_u8Step_8S >= READ_INFO_CMD_8S_NO_MAX)
	{
		s_u8Step_8S = 0; 
	}

	for(u8Idx = 0; u8Idx < MODNUM_N2402; u8Idx++)
	{
		if(0)
		{
			continue;
		}
		if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bIsExisted)
		{
			pCCU->acdc_data.gs_N2402RectInfo[u8Idx].u8CommFailTimes++;
			if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].u8CommFailTimes > 10)
			{
				pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bCommFail = TRUE;
			}
		}
	}
	
	TRACEI("[12S-Step%d] <<< Exit Read235kWPCSInfo_12S\n", s_u8Step_8S);
}
/*=============================================================================================
** @Name      : static void Read235kWPCSInfo_12S(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块12S周期信息读取任务 0X23 500ms进入一次
** @The notes : 交流AB/BC/CA线电压1103/1104/1105、交流A/B/C相电压/电流2101-2106、交流频率2107、
                总/A/B/C相有功功率2108-210B、总A/B/C相视在功率2111-2113
**===========================================================================================*/
static void Read235kWPCSInfo_12S(ROUGH_CCU_DATA* pCCU)
{
    /* ICC_MINDEX_ACDC_MOD_INFO = 84，ACDC单模块信息 */

	BYTE iGIdx = 0;
	BYTE u8Idx;
	static BYTE s_u8Step_12S = 0;
	
	TRACEI("[12S-Step%d] >>> Enter Read235kWPCSInfo_12S\n", s_u8Step_12S);

	switch (s_u8Step_12S)
	{
	case READ_INFO_CMD_12S_NO_0:
		TRACEI("[12S-Step0: PCS_ReadActivePower_0x23(eType_Total)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadActivePower_0x23(pCCU, iGIdx, eType_Total);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_1:
		TRACEI("[12S-Step1: PCS_ReadActivePower_0x23(eType_AB)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadActivePower_0x23(pCCU, iGIdx, eType_AB);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_2:
		TRACEI("[12S-Step2: PCS_ReadActivePower_0x23(eType_BC)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadActivePower_0x23(pCCU, iGIdx, eType_BC);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_3:
		TRACEI("[12S-Step3: PCS_ReadActivePower_0x23(eType_CA)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadActivePower_0x23(pCCU, iGIdx, eType_CA);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_4:
		TRACEI("[12S-Step4: PCS_ReadApparentPower_0x23(eType_Total)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadApparentPower_0x23(pCCU, iGIdx, eType_Total);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_5:
		TRACEI("[12S-Step5: PCS_ReadApparentPower_0x23(eType_AB)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadApparentPower_0x23(pCCU, iGIdx, eType_AB);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_6:
		TRACEI("[12S-Step6: PCS_ReadApparentPower_0x23(eType_BC)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadApparentPower_0x23(pCCU, iGIdx, eType_BC);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_7:
		TRACEI("[12S-Step7: PCS_ReadApparentPower_0x23(eType_CA)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			PCS_ReadApparentPower_0x23(pCCU, iGIdx, eType_CA);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_8:
		TRACEI("[12S-Step8: GetSingle_Barcode(0x11240000)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			GetSingle_Barcode(pCCU, iGIdx, 0x11240000);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_9:
		TRACEI("[12S-Step9: GetSingle_Barcode(0x11250000)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			GetSingle_Barcode(pCCU, iGIdx, 0x11250000);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_10:
		TRACEI("[12S-Step10: GetSingle_Barcode(0x11270000)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			GetSingle_Barcode(pCCU, iGIdx, 0x11270000);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_11:
		TRACEI("[12S-Step11: GetSingle_Barcode(0x11280000)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			GetSingle_Barcode(pCCU, iGIdx, 0x11280000);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_12:
		TRACEI("[12S-Step12: GetSingle_Barcode(0x11220000)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			GetSingle_Barcode(pCCU, iGIdx, 0x11220000);
		}
		s_u8Step_12S++;
		break;

	case READ_INFO_CMD_12S_NO_13:
		TRACEI("[12S-Step13: GetSingle_Barcode(0x11230000)]\n");
		for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
		{
			
			GetSingle_Barcode(pCCU, iGIdx, 0x11230000);
		}
		s_u8Step_12S++;
		break;

	default:
		break;
	}
	if(s_u8Step_12S >= READ_INFO_CMD_12S_NO_MAX)
	{
		TRACEI("[12S] Round complete, reset step to 0\n");
		s_u8Step_12S = 0; 
	}

	for(u8Idx = 0; u8Idx < MODNUM_N2402; u8Idx++)
	{
		if(0)
		{
			continue;
		}
		if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bIsExisted)
		{
			pCCU->acdc_data.gs_N2402RectInfo[u8Idx].u8CommFailTimes++;
			if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].u8CommFailTimes > 10)
			{
				pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bCommFail = TRUE;
			}
		}
	}
	
}
/*=============================================================================================
** @Name      : static void Read235kWPCSInfo_18S(void)
** @Input     : void
** @Output    : void
** @Function  : 235kw模块18S周期信息读取任务 2S进入一次 0X23
** @The notes : 总/A/B/C相无功功率210C-210F、总/A/B/C相功率因素2114-2117、模块组号1120
**===========================================================================================*/
static void Read235kWPCSInfo_18S(ROUGH_CCU_DATA* pCCU)
{
    /* ICC_MINDEX_ACDC_MOD_INFO = 84，ACDC单模块信息 */
	BYTE iGIdx = 0;
	BYTE u8Idx;
	static BYTE s_u8Step_18S = 0;
	
	switch (s_u8Step_18S)
	{
		case READ_INFO_CMD_18S_NO_0:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadReactivePower_0x23(pCCU, iGIdx, eType_Total);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_1:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadReactivePower_0x23(pCCU, iGIdx, eType_AB);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_2:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadReactivePower_0x23(pCCU, iGIdx, eType_BC);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_3:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadReactivePower_0x23(pCCU, iGIdx, eType_CA);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_4:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadPowerFactor_0x23(pCCU, iGIdx, eType_Total);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_5:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadPowerFactor_0x23(pCCU, iGIdx, eType_AB);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_6:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadPowerFactor_0x23(pCCU, iGIdx, eType_BC);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_7:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				PCS_ReadPowerFactor_0x23(pCCU, iGIdx, eType_CA);
			}
			s_u8Step_18S++;
			break;

		case READ_INFO_CMD_18S_NO_8:
			for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
			{

				GetSingle_GrpNum(pCCU, iGIdx);
			}
			s_u8Step_18S++;
			break;

		default:
			break;
	}
	if(s_u8Step_18S >= READ_INFO_CMD_18S_NO_MAX)
	{
		s_u8Step_18S = 0; 
	}

	for(u8Idx = 0; u8Idx < MODNUM_N2402; u8Idx++)
	{
		if(0)
		{
			continue;
		}
		if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bIsExisted)
		{
			pCCU->acdc_data.gs_N2402RectInfo[u8Idx].u8CommFailTimes++;
			if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].u8CommFailTimes > 10)
			{
				pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bCommFail = TRUE;
			}
		}
	}
}


/*=================================================================================================
** @Name      : static void Read235kWPCSInfo_Fast(void)
** @Input     : void
** @Output    : void
** @Function  : N2402快速周期关键数据读取任务
** @The notes : 每主循环调用一次,自旋转5槽: DC V/I,逆变状态12-15,AC频率,AC线电压+A相电压
**===============================================================================================*/
/* 20ms minimum inter-frame guard for PCS CAN communication */
// static UINT sCanLastSendTick = 0;
// static void CanSendDelay20ms(void)
// {
//     UINT now = osKernelGetTickCount();
//     UINT elapsed = now - sCanLastSendTick;
//     if(elapsed < 20)
//     {
//         osDelay(20 - elapsed);
//     }
//     sCanLastSendTick = osKernelGetTickCount();
// }

static void Read235kWPCSInfo_Fast(ROUGH_CCU_DATA* pCCU)
{
    BYTE iGIdx = 0;
    static BYTE s_u8Step_Fast = 0;

	/* Fast group number query: poll grp=0 modules (before group-level reads) */
	{
		BYTE modIdx;
		for(modIdx = 0; modIdx < MODNUM_N2402; modIdx++)
		{
			if(0)
				continue;
			if(pCCU->acdc_data.gs_N2402RectInfo[modIdx].bIsExisted == FALSE)
				continue;
			if(pCCU->acdc_data.gs_N2402RectInfo[modIdx].u16BelongedGrp != 0)
				continue;
			
			u8PackSendMsg(pCCU, R_PRPTO_DEVNO_0A, 0x23, modIdx, 0x11200000, 0);
		}
	}

    /* DC V/I: every loop, not rotated - highest priority */
    for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
    {
        
        GetModeInfo_DCVolt(pCCU, iGIdx, R_PRPTO_DEVNO_0B);
        
        GetModeInfo_DCCurr(pCCU, iGIdx, R_PRPTO_DEVNO_0B);
    }

    /* 2-slot rotation: full cycle ~500ms (was ~1.4s) */
    switch (s_u8Step_Fast)
    {
    case 0:
        /* Merged slots 1+2: Status codes 12-15 */
        for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
        {
            
            GetSingle_ConvertStat_12(pCCU, iGIdx);
            
            GetSingle_ConvertStat_13(pCCU, iGIdx);
            
            GetSingle_ConvertStat_14(pCCU, iGIdx);
            
            GetSingle_ConvertStat_15(pCCU, iGIdx);
        }
        break;

    case 1:
        /* Merged slots 3+4: AC freq + AC line voltage + phase current */
        for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
        {
            
            
            GetGrp_ACFreq_0x23(pCCU, iGIdx);
            
            GetModeInfo_LineABCVolt(pCCU, iGIdx, eType_AB);
            
            GetModeInfo_LineABCVolt(pCCU, iGIdx, eType_BC);
            
            GetModeInfo_LineABCVolt(pCCU, iGIdx, eType_CA);
            
            PCS_ReadPhaseCurr_0x23(pCCU, iGIdx, eType_AB);
            
            PCS_ReadPhaseVolt_0x23(pCCU, iGIdx, eType_AB);
        }
        break;

    default:
        break;
    }
    s_u8Step_Fast++;
	if(s_u8Step_Fast >= 2) { s_u8Step_Fast = 0; }
}

/*=================================================================================================
** @Name      : static void Read235kWPCSInfo_Slow(BYTE step)
** @Input     : step:当前槽位
** @Output    : void
** @Function  : N2402中速周期AC数据读取任务
** @The notes : 每主循环调用一次,自旋转2槽: B/C相电压,A/B/C相电流
**===============================================================================================*/
static void Read235kWPCSInfo_Slow(ROUGH_CCU_DATA* pCCU, BYTE step)
{
    BYTE iGIdx = 0;

    switch (step)
    {
    case READ_INFO_CMD_SLOW_NO_0:
        for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
        {
            
            PCS_ReadPhaseVolt_0x23(pCCU, iGIdx, eType_BC);
            PCS_ReadPhaseVolt_0x23(pCCU, iGIdx, eType_CA);
            PCS_ReadPhaseCurr_0x23(pCCU, iGIdx, eType_AB);
        }
        break;

    case READ_INFO_CMD_SLOW_NO_1:
        for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
        {
           
            PCS_ReadPhaseCurr_0x23(pCCU, iGIdx, eType_BC);
            PCS_ReadPhaseCurr_0x23(pCCU, iGIdx, eType_CA);
        }
        break;

    default:
        break;
    }
}
/*=================================================================================================
** @Name      : void N2402_SetCtrlDCVolt(BYTE grpIdx, UINT volt)
** @Input     : grpIdx - group index, volt - voltage value
** @Output    : void
** @Function  : Set DC voltage control value
** @The notes : 
**===============================================================================================*/
void N2402_SetCtrlDCVolt(BYTE grpIdx, UINT volt)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return;
    if(grpIdx == 0) pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlDCVolt = volt; else pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlDCVolt = volt;
}

void N2402_SetCtrlDCCurr(BYTE grpIdx, UINT curr)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return;
    if(grpIdx == 0) pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlDCCurr = curr; else pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlDCCurr = curr;
}

void N2402_SetCtrlACPower(BYTE grpIdx, UINT power)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return;
    if(grpIdx == 0) pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlPower = power; else pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlPower = power;
}

UINT N2402_GetCtrlDCVolt(BYTE grpIdx)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return 0;
    return grpIdx == 0 ? pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlDCVolt : pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlDCVolt;
}

UINT N2402_GetCtrlDCCurr(BYTE grpIdx)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return 0;
    return grpIdx == 0 ? pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlDCCurr : pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlDCCurr;
}

UINT N2402_GetCtrlACPower(BYTE grpIdx)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return 0;
    return grpIdx == 0 ? pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlPower : pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlPower;
}

void N2402_SetCtrlACRePower(BYTE grpIdx, UINT rePower)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return;
    if(grpIdx == 0) pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlPower = rePower; else pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlPower = rePower;
}

UINT N2402_GetCtrlACRePower(BYTE grpIdx)
{
    if(grpIdx >= N2402_MAX_GRP_NUM) return 0;
    return grpIdx == 0 ? pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlPower : pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlPower;
}

/*=================================================================================================
** @Name      : void TaskN2402_CommFailHandle(void)
** @Input     : void
** @Output    : void
** @Function  : N2402通信故障处理
** @The notes : 
**===============================================================================================*/
void TaskN2402_CommFailHandle(void)
{
    BYTE u8Idx;
    BYTE uctemp;

    uctemp = TRUE;
    for(u8Idx = 0; u8Idx < MODNUM_N2402; u8Idx++)
    {
        if(0)
        {
            continue;
        }
        if(pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bIsExisted
            && pCCU->acdc_data.gs_N2402RectInfo[u8Idx].bCommFail == FALSE)
        {
            uctemp = FALSE;
            break;
        }
    }
    pCCU->acdc_data.gs_SysN2402Info.u8IsAllCommFail = uctemp;
}

/*=================================================================================================
** @Name      : void ModCalcInfo_N2402(void)
** @Input     : void
** @Output    : void
** @Function  : 根据单模块信息计算ACDC模块组数据，更新ICC 80/81/82数组
** @The notes : 1S周期计算一次，汇总组内在线模块的电压/电流/功率/在线数
**===============================================================================================*/
void ModCalcInfo_N2402(void)
{
    BYTE iGIdx, modIdx;
    // UINT dcVoltSum, dcCurrSum, activePowerSum;
    int dcVoltSum, dcCurrSum, activePowerSum;
    BYTE onlineCount;
    float ratedPowerSum, ratedCurrSum;
    BYTE bAllCommFail = TRUE;
    BYTE runStateMax;

    if (0) { return; }
    if(pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeDelay != 0)
    {
        return;
    }
    pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeDelay = pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeInterval;

    for (iGIdx = 0; iGIdx < N2402_MAX_GRP_NUM; iGIdx++)
    {
        dcVoltSum = 0;
        dcCurrSum = 0;
        activePowerSum = 0;
        onlineCount = 0;
        ratedPowerSum = 0;
        ratedCurrSum = 0;
        runStateMax = 0;

        for (modIdx = 0; modIdx < MODNUM_N2402; modIdx++)
        {
            if (0)
                continue;
            if (pCCU->acdc_data.gs_N2402RectInfo[modIdx].bIsExisted == FALSE)
                continue;
            if (pCCU->acdc_data.gs_N2402RectInfo[modIdx].u16BelongedGrp != (iGIdx + 1))
                continue;

            if (pCCU->acdc_data.gs_N2402RectInfo[modIdx].bCommFail == FALSE)
                bAllCommFail = FALSE;

            dcVoltSum += pCCU->acdc_data.gs_N2402RectInfo[modIdx].u32ModDCVolt;
            dcCurrSum += pCCU->acdc_data.gs_N2402RectInfo[modIdx].u32ModBattCurr;
            activePowerSum += pCCU->acdc_data.gs_N2402RectInfo[modIdx].u32ModActivePower_Total;
            ratedPowerSum += pCCU->acdc_data.gs_N2402RectInfo[modIdx].fRatedPower;
            ratedCurrSum += pCCU->acdc_data.gs_N2402RectInfo[modIdx].fMax_OutCurr;
            onlineCount++;
            /* 收集模块运行状态 */
            if(pCCU->acdc_data.gs_N2402RectInfo[modIdx].StatCode_12.BIT_FIELD.bRunState == 1)
            {
                if(dcCurrSum < 0)
                {
                    runStateMax = RUNNING_STATUS_ACDC_RECTIFY;
                }
                else
                {
                    runStateMax = RUNNING_STATUS_ACDC_INVERTER;
                }
            }
            else
            {
                runStateMax = RUNNING_STATUS_ACDC_OFF;
            }
            // runStateMax |= pCCU->acdc_data.gs_N2402RectInfo[modIdx].StatCode_12.BIT_FIELD.bRunState;
        }

        if (iGIdx == 0)
        {
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32OnlineModNum  = onlineCount;
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32RatedPower    = (UINT)ratedPowerSum;
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32RatedCurr     = (UINT)(ratedCurrSum * 1000);
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32MaxCurr       = (UINT)(ratedCurrSum * 1000);
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CurrDCVolt    = (onlineCount > 0) ? (dcVoltSum / onlineCount) : 0;
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CurrDCCurr    = dcCurrSum;
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CurrSysPower  = activePowerSum;
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32WorkState     = runStateMax;//todo
            pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32CtrlState     = pCCU->acdc_data.gs_SysN2402Info.u8TaskCtrl235KwPCSCMD[0];

            pCCU->acdc_data.gPCSACDCModInfo[0][0].u32ModDCVolt          = (onlineCount > 0) ? (dcVoltSum / onlineCount) : 0;
            pCCU->acdc_data.gPCSACDCModInfo[0][0].u32ModDCCurr          = dcCurrSum;
            pCCU->acdc_data.gPCSACDCModInfo[0][0].u32ActivePowerTotal   = activePowerSum;
        }
        else if (iGIdx == 1)
        {
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32OnlineModNum  = onlineCount;
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32RatedPower    = (UINT)ratedPowerSum;
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32RatedCurr     = (UINT)(ratedCurrSum * 1000);
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32MaxCurr       = (UINT)(ratedCurrSum * 1000);
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CurrDCVolt    = (onlineCount > 0) ? (dcVoltSum / onlineCount) : 0;
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CurrDCCurr    = dcCurrSum;
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CurrSysPower  = activePowerSum;
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32WorkState     = runStateMax;
            pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32CtrlState     = pCCU->acdc_data.gs_SysN2402Info.u8TaskCtrl235KwPCSCMD[1];

            pCCU->acdc_data.gPCSACDCModInfo[1][0].u32ModDCVolt          = (onlineCount > 0) ? (dcVoltSum / onlineCount) : 0;
            pCCU->acdc_data.gPCSACDCModInfo[1][0].u32ModDCCurr          = dcCurrSum;
            pCCU->acdc_data.gPCSACDCModInfo[1][0].u32ActivePowerTotal   = activePowerSum;
        }
    }

    pCCU->acdc_data.gs_SysN2402Info.u8IsAllCommFail = bAllCommFail;
    pCCU->acdc_data.gSysOverallInfo.u32ModuleGrpOnline =
        (pCCU->acdc_data.gPCSACDCCtrlGrp[0].u32OnlineModNum << 0) |
        (pCCU->acdc_data.gPCSACDCCtrlGrp[1].u32OnlineModNum << 8);
}

/*=================================================================================================
** @Name      : BYTE NoSampDeal_N2402(void)
** @Input     : void
** @Output    : 采样状态
** @Function  : N2402无采样处理
** @The notes : 
**===============================================================================================*/
BYTE NoSampDeal_N2402(void)
{
    /* 递减各延迟计数器 */
    if(pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSWrokModeDelay > 0)  pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSWrokModeDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeDelay > 0)       pCCU->acdc_data.gs_SysN2402Info.u8Set235KWPCSTimeDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SDelay > 0)    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo2SDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SDelay > 0)    pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo8SDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SDelay > 0)  pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo12SDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SDelay > 0)  pCCU->acdc_data.gs_SysN2402Info.u8Read235KWPCSInfo18SDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeDelay > 0)      pCCU->acdc_data.gs_SysN2402Info.u8GetModeCalcTimeDelay--;
    if(pCCU->acdc_data.gs_SysN2402Info.u8AddressScanDelay > 0)
    {
        pCCU->acdc_data.gs_SysN2402Info.u8AddressScanDelay--;
        return 0;
    }

    if(pCCU->acdc_data.gs_SysN2402Info.u8SingleRectScanCmdCnt > 0)
    {
        return 0;
    }

    if(pCCU->acdc_data.gs_SysN2402Info.u8IsAllCommFail == TRUE)
    {
        return 1;
    }
    /* 填充单模块信息至 ICC 数组 */
    return 0;
}

// 模块故障判断
BYTE ModFaultJudge_N2402(void)
{
    int i;
    for(i=0; i<MODNUM_N2402; ++i)
    {
        if(!pCCU->acdc_data.gs_N2402RectInfo[i].bIsExisted) continue;
        pCCU->acdc_data.gs_N2402RectInfo[i].u8IsFault = TRUE;
    }

    for(i=0; i<MODNUM_N2402; ++i)
    {
        if(pCCU->acdc_data.gs_N2402RectInfo[i].bIsExisted   /*存在*/
            && !pCCU->acdc_data.gs_N2402RectInfo[i].bCommFail   /*通信正常*/
            && pCCU->acdc_data.gs_N2402RectInfo[i].StatCode_12.BIT_FIELD.bRunState != 3 /*未故障*/
        )
        pCCU->acdc_data.gs_N2402RectInfo[i].u8IsFault = FALSE;
    }

    // for(i=0; i<MODNUM_N2402; ++i)
    // {
    //     if(!pCCU->acdc_data.gs_N2402RectInfo[i].bIsExisted) continue;

        // todo 需要重写strLSYSInfo结构体
        pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_1_Fail = pCCU->acdc_data.gs_N2402RectInfo[0].u8IsFault;
        pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_1_CommFail = pCCU->acdc_data.gs_N2402RectInfo[0].bCommFail;
        pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_2_Fail = pCCU->acdc_data.gs_N2402RectInfo[1].u8IsFault;
        pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_2_CommFail = pCCU->acdc_data.gs_N2402RectInfo[1].bCommFail;

        if(pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_1_Fail
            && pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_2_Fail)
        {
            pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.AllRectFail = 1;
        }
        else
        {
            pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.AllRectFail = 0;
        }

        if(pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_1_CommFail
            && pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.Grp_2_CommFail)
        {
            pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.AllRectCommFail = 1;
        }
        else
        {
            pCCU->strLSYSInfo.pmGrpAlmMap.Alarm.AllRectCommFail = 0;
        }
    // }


}


/*=================================================================================================
** @Name      : void TASK_N2402_Proc(void *argument)
** @Input     : void
** @Output    : void
** @Function  : ACDC单模块通信任务
** @The notes : 
**===============================================================================================*/
void TASK_N2402_Proc(void *argument)
{
    pCCU = (ROUGH_CCU_DATA*)argument;
    // InitN2402Info();
        /* 首次运行：排CAN地址 */
        static BYTE firstRun = 1;
        if (firstRun)
        {
            InitN2402Info();

            Reflash_CAN_SelftAddr(pCCU);
            /* 启动地址扫描 */
            pCCU->acdc_data.gs_SysN2402Info.u8AddressScanDelay = 50;   /* ~2s 模块上电初始化 */
            firstRun = 0;
        }

        /* ===== 等待模块上电初始化完成 ===== */
        if (pCCU->acdc_data.gs_SysN2402Info.u8AddressScanDelay > 0)
        {
            NoSampDeal_N2402();
            ModCalcInfo_N2402();
        }

        //充放电拿到其他地方去

        Set235kwPCSPara(pCCU);

        //235kw模块读取信息
        Read235kWPCSWrokMode_0X23(pCCU);

    //快速周期查询: 每圈一槽,自旋转5槽全量~300ms
    Read235kWPCSInfo_Fast(pCCU);

    //中速周期查询: 自旋转2槽, B/C相电压 + 相电流
    {
        static BYTE s_u8Step_Slow = 0;
        Read235kWPCSInfo_Slow(pCCU, s_u8Step_Slow);
        s_u8Step_Slow++;
        if(s_u8Step_Slow >= READ_INFO_CMD_SLOW_NO_MAX) { s_u8Step_Slow = 0; }
    }
        ModFaultJudge_N2402();
        NoSampDeal_N2402();
        ModCalcInfo_N2402();

}