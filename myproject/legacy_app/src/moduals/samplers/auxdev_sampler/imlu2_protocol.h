#ifndef __IMLU2_PROTOCOL_H__
#define __IMLU2_PROTOCOL_H__

/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : imlu2_protocol.h
** @Founder      : NJX01203
** @Date         : 2026.07.01
** @Function     : IMLU2 light-strip fixed ASCII frames (7 preset colors)
** @Instructions : 从零重写，参考 R-core AuxDevTactic.c 的 7 色固定帧。
**                 灯带使用私有 DMX-like 协议走 RS485，只需发送 7 个固定 ASCII 串之一。
**===============================================================================================*/

#include <stdint.h>

typedef uint8_t  u8;

/* 7 种预设颜色 (对应 R-core e_Imlu2Color enum) */
typedef enum {
    IMLU2_BLACK  = 0,
    IMLU2_WHITE  = 1,
    IMLU2_YELLOW = 2,
    IMLU2_BLUE   = 3,
    IMLU2_RED    = 4,
    IMLU2_GREEN  = 5,
    IMLU2_ORANGE = 6,
    IMLU2_COLOR_MAX
} IMLU2_COLOR;

/* 获取指定颜色的固定 ASCII 帧指针 (帧含尾部 '\r')。
 * color 越界返回 NULL。 */
const u8* imlu2_get_color_buffer(IMLU2_COLOR color);

/* 每个 IMLU2 帧的字节长度 (不含 C 字符串终止符)。
 * 7 个帧长度相同。 */
int imlu2_get_buffer_len(void);

/* 获取 41H 软件版本查询固定 ASCII 帧指针 (帧含尾部 '\r')。
 * 帧: ~20C362410000FD9B\r (SOI+VER+ADR(195)+CID1(62)+CID2(41)+LENGTH(0)+CHKSUM+EOI)
 * 用于在线检测心跳, LENID=00H 无 INFO。 */
const u8* imlu2_get_version_query_buffer(void);

/* 41H 版本查询帧字节长度 (不含 C 字符串终止符)。 */
int imlu2_get_version_query_len(void);

/* 解析 41H 版本查询响应帧。
 * 响应: ~21C36200<LENGTH><INFO><CHKSUM>\r, 前缀中 RTN=00H 表示正常。
 * 返回 1=在线(RTN=00) 0=不在线/解析失败。 */
int imlu2_parse_version_response(const u8 *rxBuf, int len);

/* 41H 响应最小帧长 (前缀9 + LENGTH4 + CHKSUM4 + EOI1) */
#define IMLU2_MIN_FRAME_LEN        18

#endif /* __IMLU2_PROTOCOL_H__ */
