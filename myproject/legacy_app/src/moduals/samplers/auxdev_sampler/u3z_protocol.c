/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : u3z_protocol.c
** @Founder      : NJX01203
** @Date         : 2026.07.01
** @Function     : U3Z water-chiller custom protocol implementation
** @Instructions : 从零重写，参考 R-core AuxDevTactic.c 的帧结构和温度/压力解码逻辑。
**===============================================================================================*/

#include "u3z_protocol.h"

/*=================================================================================================
** @Name      : u3z_calc_data_len
** @Input     : len - 数据字段长度 (12-bit 范围)
** @Output    : 16-bit 值 = 4-bit LRC (高 nibble) + 12-bit 长度 (低 3 nibbles)
** @Function  : 半字节求和、取反加 1、取高 nibble 作为校验
**===============================================================================================*/
u16 u3z_calc_data_len(u16 len)
{
    u8 d0_d3   = (u8)(len & 0x0F);
    u8 d4_d7   = (u8)((len >> 4) & 0x0F);
    u8 d8_d11  = (u8)((len >> 8) & 0x0F);
    u8 sum     = (u8)((d0_d3 + d4_d7 + d8_d11) % 16);
    u8 lchksum = (u8)((~sum + 1) & 0x0F);
    return (u16)((lchksum << 12) | (len & 0x0FFF));
}

/*=================================================================================================
** @Name      : u3z_build_frame
** @Input     : cmd, data, dataLen, frameId, outBuf, outBufSize
** @Output    : 帧字节数 (正), 或 0 (缓冲区不足)
** @Function  : 组建 U3Z 帧: SOI(3) + DataLen(2) + FrameID(2) + Dest(2) + Src(2) + CMD(1)
**              + Data + Checksum(2) + EOI(1)
**===============================================================================================*/
int u3z_build_frame(u8 cmd, const u8 *data, int dataLen, u8 frameId, u8 *outBuf, int outBufSize)
{
    int idx = 0;
    int i;
    u16 checksum = 0;
    u16 calcLen;

    if(outBuf == 0 || dataLen < 0 || (data == 0 && dataLen > 0))
        return 0;

    /* 3+2+2+2+2+1+dataLen+2+1 = 15+dataLen 字节 */
    if(outBufSize < 15 + dataLen)
        return 0;

    calcLen = u3z_calc_data_len((u16)dataLen);

    outBuf[idx++] = U3Z_SOI_BYTE0;              /* [0] AA */
    outBuf[idx++] = U3Z_SOI_BYTE1;              /* [1] 7E */
    outBuf[idx++] = U3Z_SOI_BYTE2;              /* [2] AA */
    outBuf[idx++] = (u8)((calcLen >> 8) & 0xFF); /* [3] DataLen Hi */
    outBuf[idx++] = (u8)(calcLen & 0xFF);        /* [4] DataLen Lo */
    outBuf[idx++] = (u8)(frameId & 0xFF);        /* [5] FrameID Lo */
    outBuf[idx++] = 0;                           /* [6] FrameID Hi */
    outBuf[idx++] = U3Z_DEST_HIGH;               /* [7] Dest Hi (B7) */
    outBuf[idx++] = U3Z_DEST_LOW;                /* [8] Dest Lo (00) */
    outBuf[idx++] = U3Z_SRC_HIGH;                /* [9] Src Hi (F0) */
    outBuf[idx++] = U3Z_SRC_LOW;                 /* [10] Src Lo (00) */
    outBuf[idx++] = cmd;                         /* [11] CMD */

    for(i = 0; i < dataLen; i++)
        outBuf[idx++] = data[i];                 /* [12..] Data */

    /* 校验和: 字节 [3..idx-1] 的累加，大端 */
    for(i = 3; i < idx; i++)
        checksum += outBuf[i];
    outBuf[idx++] = (u8)((checksum >> 8) & 0xFF);
    outBuf[idx++] = (u8)(checksum & 0xFF);

    outBuf[idx++] = U3Z_EOI;                     /* 0x0D */

    return idx;
}

/*=================================================================================================
** @Name      : u3z_parse_frame
** @Input     : rxBuf, len, outState
** @Output    : 成功返回 1, 失败返回 0
** @Function  : 解析 U3Z 状态响应帧到 U3Z_STATE。
**              温度解码: (raw - 500) / 10
**              压力解码: raw / 100
**===============================================================================================*/
int u3z_parse_frame(const u8 *rxBuf, int len, U3Z_STATE *outState)
{
    int idx;
    u16 raw;

    if(rxBuf == 0 || outState == 0)
        return 0;

    /* 帧头校验 */
    if(len < U3Z_MIN_FRAME_LEN)
        return 0;
    if(rxBuf[0] != U3Z_SOI_BYTE0 || rxBuf[1] != U3Z_SOI_BYTE1 || rxBuf[2] != U3Z_SOI_BYTE2)
        return 0;
    /* 响应帧地址校验: 实测水冷机不交换 Dest/Src, 响应帧 [7]=B7(同 TX Dest),
     * 与标准协议不同。接受 B7(不交换) 或 F0(标准交换) 两种布局 */
    if(rxBuf[7] != U3Z_DEST_HIGH && rxBuf[7] != U3Z_SRC_HIGH)
        return 0;
    /* CMD 必须是查询状态响应 */
    if(rxBuf[11] != U3Z_CMD_QUERY_STATUS)
        return 0;

    idx = 12;   /* 跳过帧头 (SOI+DataLen+FrameID+Dest+Src+CMD = 12 字节) */
    /* DataType [12] 必须 = 0x00 (状态信息) */
    if(rxBuf[idx] != 0x00)
        return 0;
    idx++;

    /* 有效载荷至少 23 字节: 4*2(ntc) + 2(di,do) + 4(pwm1/2/3/4) + 2*2(pt1000,alarm)
     *                       + 1(pump) + 2*2(ps1,ps2) */
    if(len < 13 + 23)
        return 0;

    raw = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->ntc1Temp = (float)((s32)raw - 500) / 10.0f;

    raw = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->ntc2Temp = (float)((s32)raw - 500) / 10.0f;

    raw = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->ntc3Temp = (float)((s32)raw - 500) / 10.0f;

    raw = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->ntc4Temp = (float)((s32)raw - 500) / 10.0f;

    outState->di      = rxBuf[idx++];
    outState->doVal   = rxBuf[idx++];
    outState->pwm1    = rxBuf[idx++];
    idx++;                          /* 跳过 PWM2 */
    outState->pwm3    = rxBuf[idx++];
    idx++;                          /* 跳过 PWM4 */

    outState->pt1000State = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->alarm       = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;

    outState->pumpState = rxBuf[idx++];

    raw = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->ps1Pres = (float)raw / 100.0f;

    raw = (u16)((rxBuf[idx] << 8) | rxBuf[idx + 1]); idx += 2;
    outState->ps2Pres = (float)raw / 100.0f;

    return 1;
}
