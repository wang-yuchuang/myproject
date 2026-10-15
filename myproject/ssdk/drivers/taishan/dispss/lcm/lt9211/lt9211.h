/******************************************************************************
  * @project: LT9211
  * @file: lt9211.h
  * @author: zll
  * @company: LONTIUM COPYRIGHT and CONFIDENTIAL
  * @date: 2019.04.10
******************************************************************************/

#ifndef _LT9211_H
#define _LT9211_H

#include <stdint.h>
#include <driver/taishan/dispss/dispss_log.h>
#include <driver/taishan/dispss/disp.h>

#ifndef udelay
     #define udelay(x)                                      \
          {                                                  \
              volatile uint64_t count = 50ull * (uint64_t)x; \
              while (count != 0ull)                          \
                  count--;                                   \
          }
#endif

#define mdelay(x)   udelay(x * 1000)

/******************* LVDS Input Config ********************/
#define INPUT_PORTA
//#define INPUT_PORTB

#define INPUT_PORT_NUM 1

enum LVDS_FORMAT_ENUM
{
    VESA_FORMAT = 0,
    JEDIA_FORMAT
};
#define LVDS_FORMAT VESA_FORMAT

enum LVDS_MODE_ENUM
{
    DE_MODE = 0,
    SYNC_MODE
};
#define LVDS_MODE DE_MODE


/******************* Output Config ********************/
enum LT9211_OUTPUTMODE_ENUM
{
    MIPI_DSI=0,
    MIPI_CSI
};
#define LT9211_OutputMode  MIPI_DSI

enum MIPI_LANE_NUM
{
    MIPI_1LANE=0x10,
    MIPI_2LANE=0x20,
    MIPI_4LANE=0x00
};
#define MIPI_LaneNum MIPI_4LANE

enum MIPI_VIDEO_MODE
{
    MIPI_BurstMode,
    MIPI_NonBurst_SyncPulse_Mode,
    MIPI_NonBurst_SyncEvent_Mode
};
#define MIPI_VideoMode MIPI_BurstMode


struct video_timing {
    uint16_t hfp;
    uint16_t hs;
    uint16_t hbp;
    uint16_t hact;
    uint16_t htotal;
    uint16_t vfp;
    uint16_t vs;
    uint16_t vbp;
    uint16_t vact;
    uint16_t vtotal;
    uint32_t pclk_khz;
};

struct Timing {
    uint16_t hfp;
    uint16_t hs;
    uint16_t hbp;
    uint16_t hact;
    uint16_t htotal;
    uint16_t vfp;
    uint16_t vs;
    uint16_t vbp;
    uint16_t vact;
    uint16_t vtotal;
    uint32_t pclk_khz;
};

enum VideoFormat
{
    video_1280x720_60Hz_vic=1,
    video_1366x768_60Hz_vic,
    video_1280x1024_60Hz_vic,
    video_1920x1080_60Hz_vic,
    video_1920x1200_60Hz_vic,
    video_none
};

struct Lane_No {
    uint8_t swing_high_byte;
    uint8_t swing_low_byte;
    uint8_t emph_high_byte;
    uint8_t emph_low_byte;
};

extern void LT9211_Config(void);
extern void LT9211_MainLoop(void);
extern void LT9211_Main(void);
#endif
