#ifndef LMLU2_H
#define LMLU2_H

#include "../sampler_base.h"
#include "dev_485_2_sampler_main.h"

#define Led_Command_Buffer_Size 256
#define Raw_Data_Buffer_Size 100

#define Start_Flag_Address 0
#define Data_Area_Address 13

#define CHANGE_INTERVAL_TIME 1

enum Version_Address
{
    Version_Address_0 = 1,
    Version_Address_1
};

enum ADR_Address
{
    ADR_Address_0 = 3,
    ADR_Address_1
};

enum CID1_Address
{
    CID1_Address_0 = 5,
    CID1_Address_1
};

enum CID2_Address
{
    CID2_Address_0 = 7,
    CID2_Address_1
};

enum Length_Address
{
    Length_Address_Address_0 = 9,
    Length_Address_Address_1,
    Length_Address_Address_2,
    Length_Address_Address_3
};

typedef enum
{
    SIGNAL_IMLU2_HARD_VERSION = 0,
    SIGNAL_IMLU2_SOFT_VERSION,
    SIGNAL_IMLU2_COMM_FAIL_COUNT,
    SIGNAL_IMLU2_COMM_FAIL,
    SIGNAL_IMLU2_NUMBER,
} Enum_Imlu2SignalIdType;

enum Color_Type
{
    LEDS_RGB = 1,
    LEDS_RBG,
    LEDS_GRB,
    LEDS_GBR,
    LEDS_BRG,
    LEDS_BGR,
    LEDS_CUSTOM
};

typedef struct LEDInfo
{
    BYTE mode;
    BYTE ledstart;
    BYTE ledend;
    BYTE red;
    BYTE green;
    BYTE blue;
    BYTE interval_high; // 变换间隔高位
    BYTE interval_low;  // 变换间隔低位
    BYTE duty;          // 占空比
    BYTE red2;          // 静态颜色
    BYTE green2;
    BYTE blue2;
    BYTE direction; // 跑马灯方向
    BYTE u8Change;  // 渐变颜色开关
    BYTE layout;    // 格子内布局
    BYTE red3;      // 跑马底色
    BYTE green3;
    BYTE blue3;
    BYTE red4; // 渐变目标色
    BYTE green4;
    BYTE blue4;
} LED_Info;

/* typedef struct tagLEDRoughData
{

}LED_CTL_DATA; */

void SRV_IMLU2_SetLedPattern(IN RS485_2_SAMPLER_DATA* pDev_485_2_Rough, BYTE color_type, BYTE u8LenNum, BYTE u8Segment, const LED_Info *pstLedInfo, const LED_Info *pstLedInfo2);
//void LED_Set(BYTE color_type, BYTE color_type2);


#endif