/**
 * @file lv_demo_cluster.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_cluster.h"
#include "lvgl_gui.h"
#include <cmsis_os2.h>
#include <board.h>
#include <debug.h>
#include <arch/cache.h>
#include <driver/taishan/dispss/disp.h>
#include <driver/taishan/dispss/disp_data_type.h>
#if LV_USE_GPU_SEMIDRIVE_G2D
#include "lv_gpu_sdrv_dma2d.h"
#endif
#include "sd_graphics.h"
#include <stdio.h>
#include <reg.h>
#include "CLI_type.h"
/*********************
 *      DEFINES
 *********************/
#define CLUSTER_BG_PATH     "S:/early_app/cluster/cluster_bg.png"
#define NEEDLE_PATH         "S:/early_app/cluster/needle.png"
#define LEFT_TURN_PATH         "S:/early_app/cluster/LeftTurn.png"
#define RIGHT_TURN_PATH         "S:/early_app/cluster/rightTurn.png"
#define RES_PATH         "S:/early_app/cluster/res.bin"
#define RPM_SMALL_PATH         "S:/early_app/cluster/rpm_bg_small.png"
#define SPEED_SMALL_PATH         "S:/early_app/cluster/speed_bg_small.png"

#define number_size 32
#define RES_DATA_SIZE number_size*number_size*4*11

#define GUI_ALIGN_DOWN(a, size) ((a) & (~((size)-1)) )
#define GUI_ALIGN_UP(a, size)   (((a)+(size)-1) & (~ ((size)-1)))
#define IRAM_MEM_SIZE (0x1D0000)
extern char IRAM_AP_DATA[IRAM_MEM_SIZE] __SECTION(".apdata");

//#define USE_DUMMY_DECODE
/**********************
 *      TYPEDEFS
 **********************/
enum {
    LV_METER_STATE_LARGE = 0,
    LV_METER_STATE_SMALL = 100,
    LV_METER_STATE_LARGE_TO_SMALL,
    LV_METER_STATE_SMALL_TO_LARGE,
};

/**********************
 *  STATIC PROTOTYPES
 **********************/
int load_file(const void *src, uint8_t *data, uint32_t read_len);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool has_init = false;
static int disp_width = DISP_WIDTH;
static int disp_height = DISP_HEIGHT;
static int rpm_x = 76;//104*DISP_SP;
static int rpm_y = 76*DISP_SP;
static int rpm_width = 452;//600*DISP_SP;
static int rpm_height = 600*DISP_SP;
static int rpm_cx = 404*DISP_SP;
static int rpm_cy = 376*DISP_SP;
static int speed_x = 1216*DISP_SP;
static int speed_y = 76*DISP_SP;
static int speed_width = 452;//600*DISP_SP;
static int speed_height = 600*DISP_SP;
static int speed_cx = 1516*DISP_SP;
static int speed_cy = 376*DISP_SP;
static int needle_x = 288*DISP_SP;
static int needle_y = 64*DISP_SP;
static int needle_width = 24*DISP_SP;
static int needle_height = 256*DISP_SP;
static int leftturn_x = 496;//440*DISP_SP;
static int rightturn_x = 912;//1436*DISP_SP;
static int turnlight_y = 76*DISP_SP+16;
static int leftturn_width = 32;//44*DISP_SP;
static int leftturn_height = 27;//37*DISP_SP;
static int rightturn_width = 32;//44*DISP_SP;
static int rightturn_height = 27;//37*DISP_SP;

static int topbar_x = 380*DISP_SP;
static int topbar_y = 0*DISP_SP;
static int topbar_width = 1160*DISP_SP;
static int topbar_height = 110*DISP_SP;
static int bottombar_x = 480*DISP_SP;
static int bottombar_y = 600*DISP_SP + 1;// move down 1 px
static int bottombar_width = 960*DISP_SP;
static int bottombar_height = 119*DISP_SP;// move down 1 px
static bool is_displaysharing = false;
static int animstate = LV_METER_STATE_LARGE;
static int anim_frame = 50;
static int cur_anim_frame = 0;
static int start_needle_angle = -1350;
static int end_needle_angle = 1350;
static int cur_needle_angle = -1350;
static bool up = true;
static int frame_index = 0;
static float fps = 60.0f;
static int fps_x = 30;
static bool should_show_turnlight = false;
static lv_area_t full_area;
static lv_area_t bg_area;
static lv_area_t rpm_area;
static lv_area_t rpm_draw_area;
static lv_area_t speed_area;
static lv_area_t speed_draw_area;
static lv_area_t meter_small_area;
static lv_area_t rpm_small_draw_area;
static lv_area_t speed_small_draw_area;
lv_img_decoder_dsc_t bg_dsc;
lv_img_decoder_dsc_t needle_dsc;
lv_img_decoder_dsc_t leftturn_dsc;
lv_img_decoder_dsc_t rightturn_dsc;
lv_img_decoder_dsc_t rpm_small_dsc;
lv_img_decoder_dsc_t speed_small_dsc;


static uint8_t* res_data = NULL;
static bool cluster_start = false;

uint8_t* bg_fb1 = NULL;
uint8_t* bg_fb2 = NULL;
uint8_t* fg_fb1 = NULL;
uint8_t* fg_fb2 = NULL;
uint8_t* bg_buf_act = NULL;
uint8_t* fg_buf_act = NULL;

uint8_t* fg_blank_buf = NULL;
SD1_POINT_T* Point_Add = NULL;

uint32_t fg_thread_time = 0;

/**********************
 *      MACROS
 **********************/
#ifdef CONFIG_LVGL_SDRV_DISPLAY
extern struct dc_dev g_dc_dev;
#endif
#if LV_USE_GPU_SEMIDRIVE_G2D
extern struct g2dlite_device g2dlite_dev;
#endif
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
static void cluster_run_thread(void *arg)
{
    lv_cluster_init();
    lv_cluster_start();
}

void lv_cluster(void)
{
    osThreadId_t tx_thread;
    osThreadAttr_t attr = {
        .name = "cluster",
        .stack_size = 4096,
        .priority = osPriorityNormal
    };
    ssdk_printf(SSDK_INFO, "lv_cluster start\n");
    tx_thread = osThreadNew(cluster_run_thread, NULL, &attr);
    //ASSERT(tx_thread);
    ssdk_printf(SSDK_INFO, "lv_cluster end\n");
}
void lv_cluster_init(void)
{
    printf(" lv_demo_cluster !!!!!!\n");
    //start lvgl
    lvgl_init();

    /* cluster on the 3rd display */
    //start decoder
    uint32_t app_start_time = current_time();
#ifndef USE_DUMMY_DECODE
    printf(" decoder open bg\n");
    lv_res_t ret = lv_img_decoder_open(&bg_dsc, CLUSTER_BG_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || bg_dsc.img_data == NULL)
    {
        printf("open bg failed \n");
        return;
    }
    printf(" decoder open bg end\n");

    //decode needle img
    ret = lv_img_decoder_open(&needle_dsc, NEEDLE_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || needle_dsc.img_data == NULL)
    {
        printf("open needle failed \n");
        return;
    }
    //decode turn light img
    ret = lv_img_decoder_open(&leftturn_dsc, LEFT_TURN_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || leftturn_dsc.img_data == NULL)
    {
        printf("open leftturn failed \n");
        return;
    }
    ret = lv_img_decoder_open(&rightturn_dsc, RIGHT_TURN_PATH, LV_COLOR_RED);
    if (ret != LV_RES_OK || rightturn_dsc.img_data == NULL)
    {
        printf("open rightturn failed \n");
        return;
    }

    // ret = lv_img_decoder_open(&rpm_small_dsc, RPM_SMALL_PATH, LV_COLOR_RED);
    // if (ret != LV_RES_OK || rpm_small_dsc.img_data == NULL)
    // {
    //     printf("open rpm small failed \n");
    //     return;
    // }
    // ret = lv_img_decoder_open(&speed_small_dsc, SPEED_SMALL_PATH, LV_COLOR_RED);
    // if (ret != LV_RES_OK || speed_small_dsc.img_data == NULL)
    // {
    //     printf("open speed small failed \n");
    //     return;
    // }

#else
    //bg_dsc
    bg_dsc.header.w = DISP_WIDTH;
    bg_dsc.header.h = DISP_HEIGHT;
    bg_dsc.img_data = osAllocAlign(0x1000, bg_dsc.header.w*bg_dsc.header.h*4);
    memset(bg_dsc.img_data, 0x11, bg_dsc.header.w * bg_dsc.header.h * 4);

    //needle_dsc
    needle_dsc.header.w = needle_width;
    needle_dsc.header.h = needle_height;
    needle_dsc.img_data = osAllocAlign(0x1000, needle_dsc.header.w*needle_dsc.header.w*4);
    memset(needle_dsc.img_data, 0x44, needle_dsc.header.w * needle_dsc.header.w * 4);

    //leftturn_dsc
    leftturn_dsc.header.w = leftturn_width;
    leftturn_dsc.header.h = leftturn_height;
    leftturn_dsc.img_data = osAllocAlign(0x1000, leftturn_dsc.header.w*leftturn_dsc.header.w*4);
    memset(leftturn_dsc.img_data, 0x55, leftturn_dsc.header.w * leftturn_dsc.header.w * 4);

    //rightturn_dsc
    rightturn_dsc.header.w = rightturn_width;
    rightturn_dsc.header.h = rightturn_height;
    rightturn_dsc.img_data = osAllocAlign(0x1000, rightturn_dsc.header.w*rightturn_dsc.header.w*4);
    memset(rightturn_dsc.img_data, 0x66, rightturn_dsc.header.w * rightturn_dsc.header.w * 4);

#endif
    uint32_t decode_time = current_time() - app_start_time;

    needle_width = needle_dsc.header.w;
    needle_height = needle_dsc.header.h;
    // leftturn_width = leftturn_dsc.header.w;
    // leftturn_height = leftturn_dsc.header.h;
    // rightturn_width = rightturn_dsc.header.w;
    // rightturn_height = rightturn_dsc.header.h;

    full_area.x1 = 0;
    full_area.y1 = 0;
    full_area.x2 = disp_width - 1;
    full_area.y2 = disp_height - 1;

    bg_area.x1 = 0;
    bg_area.y1 = 0;
    bg_area.x2 = disp_width - 1;
    bg_area.y2 = disp_height - 1;

    rpm_area.x1 = 0;
    rpm_area.y1 = 0;
    rpm_area.x2 = rpm_width - 1;
    rpm_area.y2 = rpm_height - 1;

    rpm_draw_area.x1 = rpm_x;
    rpm_draw_area.y1 = rpm_y;
    rpm_draw_area.x2 = rpm_x + rpm_width - 1;
    rpm_draw_area.y2 = rpm_y + rpm_height - 1;


    speed_area.x1 = 0;
    speed_area.y1 = 0;
    speed_area.x2 = speed_width - 1;
    speed_area.y2 = speed_height - 1;

    speed_draw_area.x1 = speed_x;
    speed_draw_area.y1 = speed_y;
    speed_draw_area.x2 = speed_x + speed_width - 1;
    speed_draw_area.y2 = speed_y + speed_height - 1;

    res_data = osAllocAlign(0x1000, RES_DATA_SIZE);
    load_file(RES_PATH, res_data, RES_DATA_SIZE);

    printf("decode and show bg time: %d\n", decode_time);
    has_init = true;
}

void lv_cluster_start(void) {
    printf("lv_cluster_start start\n");
    if (!has_init) {
        printf("lv_cluster_start no init\n");
        return;
    }

    uint8_t* iram_rpm_buf = NULL;
    uint8_t* iram_speed_buf = NULL;
    uint8_t* iram_needle_buf = NULL;
    //bg_fb1 = osAllocAlign(0x1000, disp_width*disp_height*4);
    //bg_fb2 = osAllocAlign(0x1000, disp_width*disp_height*4);
    fg_fb1 = osAllocAlign(0x1000, disp_width*disp_height*4);
    fg_fb2 = osAllocAlign(0x1000, disp_width*disp_height*4);

    lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_fb1), disp_width, (lv_color_t *)(bg_dsc.img_data), disp_width, disp_width, disp_height);
    lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_fb2), disp_width, (lv_color_t *)(bg_dsc.img_data), disp_width, disp_width, disp_height);

    //fg_blank_buf = osAllocAlign(0x1000, disp_width*disp_height*4);
    printf("buf_info:%p,%p,%p,%p,%p\n",bg_fb1,bg_fb2,fg_fb1,fg_fb2,fg_blank_buf);
    //memset(fg_blank_buf, 0xff, disp_width * disp_height * 4);
    //arch_clean_cache_range((addr_t)fg_blank_buf, disp_width*disp_height*4);
#if 1
    //!!!!!!! iram hard code !!!!!!
    uint32_t iram_buf_next = (uint32_t)IRAM_AP_DATA;

    Point_Add = (SD1_POINT_T*)GUI_ALIGN_UP(iram_buf_next, 32);
    iram_buf_next = (uint32_t)((uint8_t*)Point_Add + 10*1024*sizeof(SD1_POINT_T));

    iram_rpm_buf = (uint8_t*)GUI_ALIGN_UP(iram_buf_next, 128);
    iram_buf_next = (uint32_t)(iram_rpm_buf + rpm_width*rpm_height*4);

    iram_speed_buf = (uint8_t*)GUI_ALIGN_UP(iram_buf_next, 128);
    iram_buf_next = (uint32_t)(iram_speed_buf + speed_width*speed_height*4);

    iram_needle_buf = (uint8_t*)GUI_ALIGN_UP(iram_buf_next, 32);
    iram_buf_next = (uint32_t)(iram_needle_buf + needle_width*needle_height*4);
    printf("iram_buf_next %p\n", iram_buf_next);
    memcpy((uint8_t*)iram_needle_buf, (uint8_t*)needle_dsc.img_data, needle_width*needle_height*4);
#else
    Point_Add = osAllocAlign(0x1000, 10*1024*sizeof(SD1_POINT_T));
    iram_rpm_buf = osAllocAlign(0x1000, rpm_width*rpm_height*4);
    iram_speed_buf = osAllocAlign(0x1000, speed_width*speed_height*4);
    iram_needle_buf = (uint8_t*)needle_dsc.img_data;
#endif

    lv_gpu_sdrv_g2d_copy((lv_color_t *)(iram_rpm_buf), rpm_width, (lv_color_t *)(bg_dsc.img_data) + rpm_y*disp_width + rpm_x, disp_width, rpm_width, rpm_height);
    lv_gpu_sdrv_g2d_copy((lv_color_t *)(iram_speed_buf), speed_width, (lv_color_t *)(bg_dsc.img_data) + speed_y*disp_width + speed_x, disp_width, speed_width, speed_height);


    //arch_clean_cache_range((addr_t)iram_needle_buf, needle_width*needle_height*4);

    struct surface layers[2] = DISPLAY_CLUSTER_TEMPLATE;
    struct post_cfg post_data;
    memset(&post_data, 0, sizeof(struct post_cfg));

    cluster_start = true;

    uint32_t stage1_time = 0;
    uint32_t stage2_time = 0;
    uint32_t stage3_time = 0;
    uint32_t all_stage_time = 0;
    uint32_t frame_start_time = 0;
    uint32_t cur_time = current_time();
    uint32_t pre_time = current_time();
    float fps = 59.9;
    uint32_t fps_cur = (uint32_t)(fps*10);
    uint32_t fps_start_time = current_time();
    uint32_t fps_gap_time = 0;
    bg_buf_act = bg_fb1;
    fg_buf_act = fg_fb1;
    int bg_changed = 2;//2-1-0
    bool should_change_fmt = false;
    bool animal_flag = false;
    int32_t z_param = 4096;
    SD_SIZE_T window = {disp_width, disp_height};
    SD_SIZE_T img = {needle_width, needle_height};
    SD_POINT_T img_mid = {needle_width/2, needle_height - 14*DISP_SP};

    printf("start while\n");
    while(cluster_start) {
        frame_start_time = cur_time = current_time();
        pre_time = cur_time;
        //printf("new while %d\n", frame_index);
        //swap buffer
        // if(bg_fb1 && bg_fb2) {
        //     if(bg_buf_act == bg_fb1)
        //         bg_buf_act = bg_fb2;
        //     else
        //         bg_buf_act = bg_fb1;
        // }

        if(fg_fb1 && fg_fb2) {
            if(fg_buf_act == fg_fb1)
                fg_buf_act = fg_fb2;
            else
                fg_buf_act = fg_fb1;
        }
#if 1
        //printf("start g2dcopy fg buffer:%p,%p,%d,%d\n",fg_buf_act, fg_blank_buf,disp_width,disp_height);
        //clear fg framebuffer
        //lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act), disp_width, (lv_color_t *)(bg_dsc.img_data), disp_width, disp_width, disp_height);

        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + rpm_y*disp_width + rpm_x, disp_width, (lv_color_t *)(iram_rpm_buf), rpm_width, rpm_width, rpm_height);
        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + speed_y*disp_width + speed_x, disp_width, (lv_color_t *)(iram_speed_buf), speed_width, speed_width, speed_height);

        // lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + rpm_y*disp_width + rpm_x, disp_width, (lv_color_t *)(bg_dsc.img_data) + rpm_y*disp_width + rpm_x, disp_width, rpm_width, rpm_height);
        // lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + speed_y*disp_width + speed_x, disp_width, (lv_color_t *)(bg_dsc.img_data) + speed_y*disp_width + speed_x, disp_width, speed_width, speed_height);
        //printf("start g2dcopy fg buffer end\n");
#endif
#if 0
        //draw fps to fg
        fps_cur = (uint32_t)(fps*10);
        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x, disp_width, (lv_color_t *)(res_data + number_size*number_size*(fps_cur/100)*4), number_size, number_size, number_size);
        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x + number_size, disp_width, (lv_color_t *)(res_data + number_size*number_size*((fps_cur%100)/10)*4), number_size, number_size, number_size);
        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x + 2*number_size, disp_width, (lv_color_t *)(res_data + number_size*number_size*10*4), number_size, number_size, number_size);
        lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + 10*disp_width + fps_x + 3*number_size, disp_width, (lv_color_t *)(res_data + number_size*number_size*(fps_cur%10)*4), number_size, number_size, number_size);
#endif
#if 1
        //draw turn light to fg
        if (should_show_turnlight) {
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + turnlight_y*disp_width + leftturn_x, disp_width, (lv_color_t *)(leftturn_dsc.img_data), leftturn_width, leftturn_width, leftturn_height);
            lv_gpu_sdrv_g2d_copy((lv_color_t *)(fg_buf_act) + turnlight_y*disp_width + rightturn_x, disp_width, (lv_color_t *)(rightturn_dsc.img_data), rightturn_width, rightturn_width, rightturn_height);
        }
#endif
        cur_time = current_time();
        stage1_time = cur_time - pre_time;
        pre_time = cur_time;
#if 1
        //TODO: MOVE fg thread draw to here
        //draw needle
        memset(Point_Add, 0 , 10*1024*sizeof(SD1_POINT_T));
        SD_POINT_T rpm_window_mid = {rpm_cx-4,  rpm_cy-16};
        SD_rotation(window,img,rpm_window_mid,img_mid,cur_needle_angle,(uint32_t*)iram_needle_buf,(uint32_t*)fg_buf_act,z_param,Point_Add);

        memset(Point_Add, 0 , 10*1024*sizeof(SD1_POINT_T));
        SD_POINT_T speed_window_mid = {speed_cx+4,  speed_cy-16};
        SD_rotation(window,img,speed_window_mid,img_mid,-cur_needle_angle,(uint32_t*)iram_needle_buf,(uint32_t*)fg_buf_act,z_param,Point_Add);

        //clean cache
        arch_clean_cache_range((addr_t)(fg_buf_act + (int)(100*DISP_SP*disp_width*4)), disp_width*(disp_height - 200*DISP_SP)*4);

#endif

        cur_time = current_time();
        stage2_time = cur_time - pre_time;
        pre_time = cur_time;

        cur_time = current_time();
        all_stage_time = cur_time - frame_start_time;
        pre_time = cur_time;

        //post
        layers[1].addr[0] = (unsigned long)fg_buf_act;
        layers[0].addr[0] = (unsigned long)bg_buf_act;

        layers[1].src_stride[0] = disp_width*4;
        layers[0].src_stride[1] = disp_width*4;

        post_data.layers             = layers + 1;
        post_data.num_layers           = 1;
        if (!animal_flag) {
            animal_flag = true;
        }
        //printf("post start \n");
        sdrv_display_post(&g_dc_dev, &post_data);
        //printf("post end \n");
        //angle

        if (cur_needle_angle >= end_needle_angle){
            up = false;
        } else if (cur_needle_angle <= start_needle_angle){
            up = true;
        }

        if (up) {
            cur_needle_angle += 10;
        } else {
            cur_needle_angle -= 10;
        }

        frame_index++;
        if (frame_index >= 3600) {
            frame_index = 0;
        }

        if (frame_index <= 600  && frame_index%30 == 0 ) {
            should_show_turnlight = !should_show_turnlight;
        } else if (frame_index > 600) {
            should_show_turnlight = false;
        }

        if (frame_index%120 == 0) {
            fps_gap_time = current_time() - fps_start_time;
            if (fps_gap_time != 0) {
                fps = 120*1000.0f/fps_gap_time;
                if (fps > 99) {
                    fps = 99.9f;
                }
            }
            fps_start_time = current_time();
            //printf("fps:%f, stage1_time:%d, stage2_time:%d, fg_thread_time:%d, all_stage_time:%d\n", fps, stage1_time, stage2_time, fg_thread_time, all_stage_time);
        }

    }
}

void lv_cluster_stop(void) {
    cluster_start = false;
}

void lv_cluster_deinit(void) {
    has_init = false;
    if (bg_fb1 != NULL) {
        osFree(bg_fb1);
        bg_fb1 = NULL;
    }
    if (bg_fb2 != NULL) {
        osFree(bg_fb2);
        bg_fb2 = NULL;
    }
    if (fg_fb1 != NULL) {
        osFree(fg_fb1);
        fg_fb1 = NULL;
    }
    if (fg_fb2 != NULL) {
        osFree(fg_fb2);
        fg_fb2 = NULL;
    }

    if (fg_blank_buf != NULL) {
        osFree(fg_blank_buf);
        fg_blank_buf = NULL;
    }
    if (Point_Add != NULL) {
        osFree(Point_Add);
        Point_Add = NULL;
    }
    if (res_data != NULL) {
        osFree(res_data);
        res_data = NULL;
    }

    lv_img_decoder_close(&bg_dsc);
    lv_img_decoder_close(&needle_dsc);
    lv_img_decoder_close(&leftturn_dsc);
    lv_img_decoder_close(&rightturn_dsc);
    lv_img_decoder_close(&rpm_small_dsc);
    lv_img_decoder_close(&speed_small_dsc);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
int load_file(const void *src, uint8_t *data, uint32_t read_len) {
    lv_fs_file_t file;
    lv_fs_res_t res;
    res = lv_fs_open(&file, src, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        printf("open failed\n");
        return res;
    }

    uint32_t read_num;
    res = lv_fs_read(&file, data, read_len, &read_num);
    lv_fs_close(&file);

    if(res != LV_FS_RES_OK || read_num != read_len) {
        printf("read failed read_num: %d, read_len: %d\n", read_num, read_len);
    }

    return res;
}
