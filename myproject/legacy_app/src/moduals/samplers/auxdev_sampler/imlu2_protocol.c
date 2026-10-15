/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : imlu2_protocol.c
** @Founder      : NJX01203
** @Date         : 2026.07.01
** @Function     : IMLU2 light-strip fixed ASCII frame buffers (7 preset colors)
** @Instructions : 从零重写，帧字节与 R-core AuxDevTactic.c 677-683 行完全一致。
**===============================================================================================*/

#include "imlu2_protocol.h"
#include <stddef.h>  /* NULL */

/* 7 色固定帧。帧格式: ~<header>0D<RGB>07D0...193F<CRC>\r
 * RGB 字段决定颜色 (black=000000 white=FFFFFF yellow=FFFF00
 * blue=0000FF red=00FF00 green=FF0000 orange=A5FF00) */
static const u8 g_imlu2_frame_black[]  = "~20C362427036FF0E0101000D00000007D014101400000079101010000000000193F2CD\r";
static const u8 g_imlu2_frame_white[]  = "~20C362427036FF0E0101000DFFFFFF07D014101400000079101010000000000193F249\r";
static const u8 g_imlu2_frame_yellow[] = "~20C362427036FF0E0101000DFFFF0007D014101400000079101010000000000193F275\r";
static const u8 g_imlu2_frame_blue[]   = "~20C362427036FF0E0101000D0000FF07D014101400000079101010000000000193F2A1\r";
static const u8 g_imlu2_frame_red[]    = "~20C362427036FF0E0101000D00FF0007D014101400000079101010000000000193F2A1\r";
static const u8 g_imlu2_frame_green[]  = "~20C362427036FF0E0101000DFF000007D014101400000079101010000000000193F2A1\r";
static const u8 g_imlu2_frame_orange[] = "~20C362427036FF0E0101000DA5FF0007D014101400000079101010000000000193F28B\r";

static const u8* g_color_buffers[] = {
    g_imlu2_frame_black,
    g_imlu2_frame_white,
    g_imlu2_frame_yellow,
    g_imlu2_frame_blue,
    g_imlu2_frame_red,
    g_imlu2_frame_green,
    g_imlu2_frame_orange
};

const u8* imlu2_get_color_buffer(IMLU2_COLOR color)
{
    if(color >= IMLU2_COLOR_MAX)
        return NULL;
    return g_color_buffers[color];
}

int imlu2_get_buffer_len(void)
{
    /* 7 个缓冲区长度相同 (不含 C 字符串终止符) */
    return (int)(sizeof(g_imlu2_frame_black) - 1);
}

/* 41H 软件版本查询固定帧 (协议 V1.09 6.2 节, 示例报文):
 * SOI(~) + VER(20) + ADR(C3=195) + CID1(62) + CID2(41) + LENGTH(0000, LENID=00H) + CHKSUM(FD9B) + EOI(\r)
 * ADR 与 7 色固定帧一致 (195), 故可硬编码。 */
static const u8 g_imlu2_version_query[] = "~20C362410000FD9B\r";

const u8* imlu2_get_version_query_buffer(void)
{
    return g_imlu2_version_query;
}

int imlu2_get_version_query_len(void)
{
    return (int)(sizeof(g_imlu2_version_query) - 1);
}

/* 响应帧前缀 ~21C36200: SOI(~) + VER(21) + ADR(C3=195) + CID1(62) + RTN(00=正常)。
 * 帧尾部以 EOI(0x0D) 结束。仅做在线判定所需的最小校验:
 * SOI + VER + ADR + CID1 + RTN=00 + EOI, 不解析 INFO/CHKSUM (与 u3z_parse_frame 风格一致)。
 * 注: VER 次版本位 (rxBuf[2]) 宽松处理, 仅要求主版本 '2'。 */
int imlu2_parse_version_response(const u8 *rxBuf, int len)
{
    /* 最小帧长: 前缀(9) + LENGTH(4) + CHKSUM(4) + EOI(1) = 18 (RTN=00 且 INFO 为空的极端情况) */
    if(rxBuf == NULL || len < 18)
        return 0;

    /* SOI = '~' (7EH) */
    if(rxBuf[0] != '~')
        return 0;

    /* VER = '2' */
    if(rxBuf[1] != '2')
        return 0;

    /* ADR = 'C3' (195), CID1 = '62' */
    if(rxBuf[3] != 'C' || rxBuf[4] != '3')
        return 0;
    if(rxBuf[5] != '6' || rxBuf[6] != '2')
        return 0;

    /* RTN = '00' (正常) */
    if(rxBuf[7] != '0' || rxBuf[8] != '0')
        return 0;

    /* EOI = CR (0DH) */
    if(rxBuf[len - 1] != '\r')
        return 0;

    return 1;
}
