/**
 * @file lv_demo_cluster.h
 *
 */

#ifndef LV_DEMO_CLUSTER_H
#define LV_DEMO_CLUSTER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <driver/taishan/dispss/disp_data_type.h>

/*********************
 *      DEFINES
 *********************/

//#define DEMO_DISPLAY_CONFIG

#ifdef DEMO_DISPLAY_CONFIG
// #define DISP_WIDTH 800
// #define DISP_HEIGHT 600
// #define DISP_SP 0.416667f //800*300
#define DISP_WIDTH 1920
#define DISP_HEIGHT 720
#define DISP_SP 1 //1920*720
#else
#define DISP_WIDTH 1440
#define DISP_HEIGHT 540
#define DISP_SP 0.75f //1440*540
#endif
//0:fg, 1:bg
#define DISPLAY_CLUSTER_TEMPLATE {\
{\
    0,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ARGB8888,/*fmt*/\
    {0,0,DISP_WIDTH,DISP_HEIGHT},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {DISP_WIDTH*4,0,0,0},/*stride*/ \
    {0,0,DISP_WIDTH,DISP_HEIGHT},/*start*/ \
    {0,0,DISP_WIDTH,DISP_HEIGHT},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    1,/*alpha_en*/\
    0xff,/*alpha*/\
    0,/*z-order*/ \
    0/*security*/\
},\
{\
    1,/*layer*/\
    0,/*layer_dirty*/\
    1,/*layer_en*/\
    COLOR_ARGB8888,/*fmt*/\
    {0,0,DISP_WIDTH,DISP_HEIGHT},/*x,y,w,h src */ \
    {0x0,0x0,0x0,0x0},/*y,u,v,a*/ \
    {DISP_WIDTH*4,0,0,0},/*stride*/ \
    {0,0,DISP_WIDTH,DISP_HEIGHT},/*start*/ \
    {0,0,DISP_WIDTH,DISP_HEIGHT},/*dst*/ \
    0,/*ckey_en*/\
    0,/*ckey*/\
    0,/*alpha_en*/\
    0x0,/*alpha*/\
    1,/*z-order*/ \
    0/*security*/\
}\
}

/**********************
 *      TYPEDEFS
 **********************/
#define LV_PNG_USE_LV_FILESYSTEM 1
/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_cluster(void);
void lv_cluster_init(void);
void lv_cluster_start(void);
void lv_cluster_stop(void);
void lv_cluster_deinit(void);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_CLUSTER_H*/
