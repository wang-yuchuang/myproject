#ifndef __U3Z_PROTOCOL_H__
#define __U3Z_PROTOCOL_H__

/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : u3z_protocol.h
** @Founder      : NJX01203
** @Date         : 2026.07.01
** @Function     : U3Z water-chiller custom protocol (build frame / parse frame / data-len LRC)
** @Instructions : 从零重写，参考 R-core AuxDevTactic.c 的 U3Z 帧结构。
**                 纯函数，无框架依赖，可独立单元测试。
**===============================================================================================*/

#include <stdint.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  s32;

/* U3Z 解析后的状态数据 */
typedef struct {
    float ntc1Temp;      /* 回液温度 degC */
    float ntc2Temp;      /* 供液温度 degC */
    float ntc3Temp;      /* 进风温度 degC */
    float ntc4Temp;      /* NTC4 温度 degC */
    u8    di;            /* 数字输入 */
    u8    doVal;         /* 数字输出 */
    u8    pwm1;          /* PWM1 占空比 % */
    u8    pwm3;          /* PWM3 占空比 % */
    u16   pt1000State;   /* PT1000 状态位 */
    u16   alarm;         /* 告警位 */
    u8    pumpState;     /* 泵状态: 0=Idle 1=Run 2=Fault */
    float ps1Pres;       /* 压力1 MPa */
    float ps2Pres;       /* 压力2 MPa */
} U3Z_STATE;

/* U3Z 命令码 */
#define U3Z_CMD_QUERY_STATUS    0x21
#define U3Z_CMD_UPPER_STATUS    0x11

/* U3Z 0x21 查询数据类型 (协议 V1.02: 查询帧 DataLen=1, Data=DataType) */
#define U3Z_DATATYPE_STATUS     0x00    /* 状态信息 */
#define U3Z_DATATYPE_VERSION    0x01    /* 版本信息 */

/* U3Z 帧常量 */
#define U3Z_SOI_BYTE0           0xAA
#define U3Z_SOI_BYTE1           0x7E
#define U3Z_SOI_BYTE2           0xAA
#define U3Z_EOI                 0x0D
#define U3Z_DEST_HIGH           0xB7
#define U3Z_DEST_LOW            0x00
#define U3Z_SRC_HIGH            0xF0
#define U3Z_SRC_LOW             0x00
#define U3Z_MIN_FRAME_LEN       15
#define U3Z_MAX_TX_FRAME_LEN    128

/* 12 位数据长度字段的半字节 LRC 计算 */
u16 u3z_calc_data_len(u16 len);

/* 组建 U3Z 帧，返回帧字节数；失败返回 0 */
int u3z_build_frame(u8 cmd, const u8 *data, int dataLen, u8 frameId, u8 *outBuf, int outBufSize);

/* 解析 U3Z 状态响应帧；成功返回 1，失败返回 0 */
int u3z_parse_frame(const u8 *rxBuf, int len, U3Z_STATE *outState);

#endif /* __U3Z_PROTOCOL_H__ */
