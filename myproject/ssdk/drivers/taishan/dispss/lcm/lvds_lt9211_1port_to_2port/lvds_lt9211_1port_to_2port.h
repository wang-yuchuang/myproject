/******************************************************************************
  * @project: LT9211
  * @file: lt9211.h
  * @author: zll
  * @company: LONTIUM COPYRIGHT and CONFIDENTIAL
  * @date: 2019.04.10
 ******************************************************************************/

#ifndef _LT9211_LVDS_1PORT_TO_2PORT_H
#define _LT9211_LVDS_1PORT_TO_2PORT_H

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
//#define INPUT_PORTA
#define INPUT_PORTB

#define INPUT_PORT_NUM 1

enum LVDS_FORMAT_ENUM
{
    VESA_FORMAT = 0,
    JEDIA_FORMAT
};
#define LVDS_FORMAT VESA_FORMAT

/******************* Lvds Output Config ********************/
enum LT9211_LVDSPORT_ENUM
{
    LVDS_1PORT = 0,
    LVDS_2PORT = 1
};
#define LVDS_PORTNUM LVDS_2PORT

enum LT9211_LVDSMODE_ENUM
{
    DE_MODE = 0,
    SYNC_MODE = 1
};
#define LVDS_MODE SYNC_MODE

enum LT9211_LVDSDATAFORMAT_ENUM
{
    VESA = 0,
    JEIDA = 1
};
#define LVDS_DATAFORMAT VESA

enum LT9211_LVDSCOLORDEPTH_ENUM
{
    DEPTH_6BIT = 0,
    DEPTH_8BIT = 1
};
#define LVDS_COLORDEPTH DEPTH_8BIT

//#define LVDS_2PORT_SWAP

/******************* Lvds Output Config ********************/

struct video_timing{
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

void LT9211_LVDS2LVDS_MainLoop(void);
void LT9211_LVDS2LVDS_Main(void);
#endif
