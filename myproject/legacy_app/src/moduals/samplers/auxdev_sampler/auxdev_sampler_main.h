#ifndef __AUXDEV_SAMPLER_MAIN_H__
#define __AUXDEV_SAMPLER_MAIN_H__

/*==========================================Copyright(c)===========================================
**                                  Shenzhen Infypower Co., Ltd.
**                                  Power conversion Department
**                                       BMS Product Line
**---------------------------------------File Information------------------------------------------
** @File Name    : auxdev_sampler_main.h
** @Founder      : NJX01203
** @Date         : 2026.07.01
** @Function     : Combined RS485 sampler for U3Z water chiller + IMLU2 light strip on shared UART5
** @Instructions : 从零重写，参考备份的框架结构。
**                 绑定 Sampler_ID=12, CommPort_ID=14 (RS485_AUXDEV), UART5 (ttyS5)。
**                 总线仲裁: 水冷机轮询与灯带刷新之间 200ms 时间片，
**                 匹配 R-core sUart5BusBusy / sUart5BusFreeTime 逻辑。
**===============================================================================================*/

#include "../sampler_base.h"
#include "u3z_protocol.h"
#include "imlu2_protocol.h"

/* U3Z 原始帧缓冲区大小 */
#define AUXDEV_U3Z_RX_BUF_LEN     128
#define AUXDEV_U3Z_TX_BUF_LEN     128

/* UART5 总线仲裁: 水冷机和灯带之间 200ms 时间片 */
#define AUXDEV_UART5_TIMESLICE_MS 200

/* 通信故障阈值 (连续失败次数) */
#define AUXDEV_COMM_FAULT_LIMIT   3

/* Sample() 内部周期 (框架按 T1 Sample_Period=500ms 调用) */
#define AUXDEV_CHILLER_POLL_EVERY_N_CYCLES   2    /* 每 1000ms 轮询一次水冷机 */

/* RS485_AUXDEV 的 CommPort ID */
#define AUXDEV_COMMPORT_ID         14

/* 数据库身份 (与 System.sql 一致) */
#define CHILLER_DEV_ID             961
#define CHILLER_DEV_MODEL          960
#define LED_DEV_ID                 966
#define LED_DEV_MODEL              970

/* 水冷机 StatusParameter IDs (1923-1936) */
#define CHILLER_PARAM_NTC1TEMP     1923
#define CHILLER_PARAM_NTC2TEMP     1924
#define CHILLER_PARAM_NTC3TEMP     1925
#define CHILLER_PARAM_NTC4TEMP     1926
#define CHILLER_PARAM_DI           1927
#define CHILLER_PARAM_DO           1928
#define CHILLER_PARAM_PWM1         1929
#define CHILLER_PARAM_PWM3         1930
#define CHILLER_PARAM_PT1000STATE  1931
#define CHILLER_PARAM_ALARM        1932
#define CHILLER_PARAM_PUMPSTATE    1933
#define CHILLER_PARAM_PS1PRES      1934
#define CHILLER_PARAM_PS2PRES      1935
#define CHILLER_PARAM_COMMSTAT     1936
#define CHILLER_PARAM_POWER_STATE    1937   /* 工作状态显示 (0=关机 1=开机) */
#define CHILLER_PARAM_MODE_STATE     1938   /* 控制模式显示 (0=自动 1=手动) */
#define CHILLER_PARAM_INSTALL_TYPE 2099
#define CHILLER_PARAM_POWER_SWITCH   2201
#define CHILLER_PARAM_CTRL_MODE     2202

/* 灯带 StatusParameter ID (通信状态由 41H 版本查询心跳独立判定) */
#define LED_PARAM_COMMSTAT          1939


/* LED 自动判决读取的 DPR 参数 (由 st_batt_sampler RPMSG 路径推送 param 550/552) */
#define LED_DPR_DEV_BATTGRP         501     /* 第一块电池 devId (st_batt_sampler 推送) */
#define LED_DPR_DEV_BATTGRP2        502     /* 第二块电池 devId (两簇共用水冷机/灯带) */
#define LED_DPR_BATT_GROUP_NUM      2       /* 电池簇数量 */
#define LED_DPR_PARAM_RUNSTATUS     550     /* Per-Batt Run Status, ENUM: 4=充电 5=放电 */
#define LED_DPR_PARAM_WORKSTATUS    552     /* Per-Batt Alarm Status, ENUM: 3=故障 */
#define LED_DPR_DEV_CCU             200     /* CCU/STS 设备 (ccu_sampler 推送) */
#define LED_DPR_PARAM_SYSSTATUS     103     /* CCU System Status, ENUM: 2=Faulted */

typedef enum tagAuxDevRole
{
    AUXDEV_ROLE_NONE    = 0,
    AUXDEV_ROLE_CHILLER = 1,
    AUXDEV_ROLE_LED     = 2
} AUXDEV_ROLE;

/* 单设备运行时状态 */
typedef struct tagAuxDev
{
    int           nDevId;                                 /* 961 或 966 */
    int           nDevModel;                              /* 960 或 970 */
    AUXDEV_ROLE   role;
    int           nCommFailCnt;
    BOOL          bCommOK;
    BOOL          bInstalled;                             /* 是否已安装 (仅水冷机使用, 灯带忽略) */
    /* 水冷机缓冲区 */
    u8            rxBuf[AUXDEV_U3Z_RX_BUF_LEN];
    int           nRxLen;
    U3Z_STATE     chillerState;
    u8            powerSwitch;                            /* 0=off 1=on (CMD 0x11 A枪充电状态) */
    u8            ctrlMode;                               /* 0=auto 1=manual (水冷机控制模式) */
    /* 灯带状态 */
    IMLU2_COLOR   lastSentColor;
    IMLU2_COLOR   pendingColor;
    BOOL          bColorDirty;
} AUX_DEV;

/* sampler 运行时状态 */
typedef struct tagAuxDevSampler
{
    HANDLE         hSampler;
    int            nSamplerId;

    AUX_DEV        chiller;                               /* 设备 961 */
    AUX_DEV        led;                                   /* 设备 966 */
    u8             frameId;                               /* U3Z 滚动帧 id (0..255) */
    int            nCycleCnt;                             /* 每次 Sample() 自增 */
    BOOL           bUart5BusBusy;
    UINT           nUart5BusFreeTimeMs;                   /* 总线释放的单调 ms 时间戳 */
    BOOL           bInited;
} AUXDEV_SAMPLER;

#endif /* __AUXDEV_SAMPLER_MAIN_H__ */
