/**
 * @file dshow.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <string.h>
#include <stdlib.h>
#include <debug.h>
#include <stdio.h>
#include <types.h>
#include "CLI_type.h"
#include <reg.h>
#include <sdm_display.h>
#include <disp_data_type.h>
#include <clock_cfg.h>
#include <board.h>

#include "buffer.h"
#ifdef CONFIG_GPU_HAL
#include "g2dlite.h"
#endif

#include "disp_service_defs.h"

#define DISPLAY_NUM    5
bool noblock;

static struct idx_to_fmt {
    int idx;
    unsigned int fmt;
} itof[] = {
    {0, COLOR_RGB565},
    {1, COLOR_BGR565},
    {2, COLOR_ARGB4444},
    {3, COLOR_BGRA4444},
    {4, COLOR_ARGB1555},
    {5, COLOR_RGB888},
    {6, COLOR_BGR888},
    {7, COLOR_ARGB8888},
    {8, COLOR_BGRA8888},
    {9, COLOR_ARGB2101010},
    {10, COLOR_BGRA2101010},
    {11, COLOR_NV21},
    {12, COLOR_NV12},
    {13, COLOR_YUV420P},
    {14, COLOR_YUYV},
    {-1, 0},
};

enum {
    SCREEN_CLEAR = 0,
    SHOW_PATTERN,
    SHOW_MEM,
    DUMP_REGISTERS,
    WRITE_REGISTERS,
    DUMP_CLOCKS,
    G2DLITE_SHOW,
    SERVICE_TEST,
};

static int get_fmt_by_idx(int idx)
{
    return itof[idx].fmt;
}

void read_registers(uint32_t start, int end)
{
    int count = 0;
    for ( ; start <= end; start += 4) {
        if (count == 0) {
            printf("0x%08x: ", start);
        }
        uint32_t val = readl(start);
        printf("%08x ", val);
        count += 4;
        if (count == 16) {
            printf("\n");
            count = 0;
        }
    }
    if (count != 0) {
        printf("\n");
    }
}

void write_registers(uint32_t addr, int val)
{
    writel(val, addr);
}

void dump_registers(uint32_t start, int size)
{

    uint32_t end = start + size;

    printf("----dump registers----\n");
    read_registers(start, end);
}

void dump_clocks(int clk_num, int freq)
{
    uint clk_freq, clk_freq1, clk_freq2, clk_freq3;

    switch (clk_num)
    {
        case 1:
            lvds_clk_set_rate(&g_pll_lvds1_div_a, freq);
            printf("g_pll_lvds1_div_a freq = %u %d\n", freq, g_pll_lvds1_div_a.id);
            break;
        case 2:
            lvds_clk_set_rate(&g_pll_lvds2_div_a, freq);
            printf("g_pll_lvds2_div_a freq = %u\n", freq);
            break;
        case 3:
            lvds_clk_set_rate(&g_pll_lvds3_div_a, freq);
            printf("g_pll_lvds3_div_a freq = %u\n", freq);
            break;
        case 4:
            lvds_clk_set_rate(&g_pll_lvds4_div_a, freq);
            printf("g_pll_lvds4_div_a freq = %u\n", freq);
            break;
        default:
            clk_freq = clk_get_rate(&g_pll_lvds1_div_a);
            printf("g_pll_lvds1_div_a freq = %u\n", clk_freq);
            clk_freq1 = clk_get_rate(&g_pll_lvds2_div_a);
            printf("g_pll_lvds2_div_a freq = %u\n", clk_freq1);
            clk_freq2 = clk_get_rate(&g_pll_lvds3_div_a);
            printf("g_pll_lvds3_div_a freq = %u\n", clk_freq2);
            clk_freq3 = clk_get_rate(&g_pll_lvds4_div_a);
            printf("g_pll_lvds4_div_a freq = %u\n", clk_freq3);
    }
}

void dshow_service_test(struct bo *bo, int display_id)
{
    int ret = 0;
    ret = bo_init(bo);
    if (ret < 0) {
        printf("bo init failed\n");
        return;
    }
    // dshow_post(display_id, bo);
    display_service_test(bo->addr[0]);
}

#ifdef CONFIG_GPU_HAL
void dshow_g2dlite(struct bo *bo, int display_id, int func)
{
    int ret = 0;
    void *output = NULL;

    ret = bo_init(bo);
    if (ret < 0) {
        printf("bo init failed\n");
        return;
    }

    dshow_post(display_id, bo);

    udelay(5*1000*1000);

    switch(func)
    {
        case 0:
            output = (void *)blend_gpipe_bypass(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 1:
            output = (void *)yuv_to_rgb(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            bo->fmt = COLOR_ARGB8888;
            dshow_post(display_id, bo);
            break;
        case 2:
            output = (void *)yuv_to_rgb_cpu(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 3:
            output = (void *)blend_2_pipe(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 4:
            output = (void *)scaler_gpipe(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            bo->src_width = 2*(bo->src_width);
            bo->src_height = 2*(bo->src_height);
            bo->dst_width = 2*(bo->dst_width);
            bo->dst_height = 2*(bo->dst_height);
            dshow_post(display_id, bo);
            break;
        case 5:
            output = (void *)rotation_test(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 6:
            output = (void *)fill_rect(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 7:
            output = (void *)fastcopy(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 8:
            output = (void *)porter_duff(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        case 9:
            output = (void *)yuv_to_yuv(bo->fmt, bo->src_width, bo->src_height, bo->addr[0]);
            bo->addr[0] = output;
            dshow_post(display_id, bo);
            break;
        default:
            break;
    }
    return;
}
#endif

void dshow_pattern(struct bo *bo, int display_id)
{
    int ret = 0;
    ret = bo_init(bo);
    if (ret < 0) {
        printf("bo init failed\n");
        return;
    }

    dshow_post(display_id, bo);
}

static int dshow(int argc, char *argv[])
{
    int8_t i = 0;
    struct bo bo;
    display_handle *handle;
    uint32_t addr = 0x0, val = 0;

    int clk_num = 0, display_id = 0;
    uint8_t nplane = 0;
    int func = SCREEN_CLEAR;
    int freq = 0;

    if (argc == 0) {
        printf("clear screen:\n");
        printf("dshow [display_id]\n");
        printf("dshow [format] [src_width] [src_heigt] [dst_width] [dst_heigt] [base_addr] [displayid]\n");
        printf("format:\n");
        printf("{0, COLOR_RGB565}\n");
        printf("{1, COLOR_BGR565}\n");
        printf("{2, COLOR_ARGB4444}\n");
        printf("{3, COLOR_BGRA4444}\n");
        printf("{4, COLOR_ARGB1555}\n");
        printf("{5, COLOR_RGB888}\n");
        printf("{6, COLOR_BGR888}\n");
        printf("{7, COLOR_ARGB8888}\n");
        printf("{8, COLOR_BGRA8888}\n");
        printf("{9, COLOR_ARGB2101010}\n");
        printf("{10, COLOR_BGRA2101010}\n");
        printf("{11, COLOR_NV21}\n");
        printf("{12, COLOR_NV12}\n");
        printf("{13, COLOR_YUV420P}\n");
        printf("{14, COLOR_YUYV}\n");
        return 0;
    }

    if (!strcmp(argv[0], "-reg")) {
        addr = strtoul(argv[2], NULL, 0);
        val = strtoul(argv[3], NULL, 0);
        if (!strcmp(argv[1], "r")) {
            func = DUMP_REGISTERS;
            goto exe_func;
        } else if (!strcmp(argv[1], "w")) {
            func = WRITE_REGISTERS;
            goto exe_func;
        }
    }

    if (!strcmp(argv[0], "-clk")) {
        if (argc == 3) {
            clk_num = strtoul(argv[++i], NULL, 0);
            freq = strtoul(argv[++i], NULL, 0);
        }
        func = DUMP_CLOCKS;
        goto exe_func;
    }

    if (argc == 1) {
        display_id = strtoul(argv[i], NULL, 0);
        if ((display_id > DISPLAY_NUM-1) || (display_id < 0))
            display_id = 0;
        printf("post display id:%d str:%s\n", display_id, argv[i]);
        func = SCREEN_CLEAR;
        goto exe_func;
    }

    memset(&bo, 0, sizeof(struct bo));

    bo.fmt = itof[strtoul(argv[i++], NULL, 0)].fmt;
    bo.src_width = strtoul(argv[i++], NULL, 0);
    bo.src_height = strtoul(argv[i++], NULL, 0);
    bo.dst_width = strtoul(argv[i++], NULL, 0);
    bo.dst_height = strtoul(argv[i++], NULL, 0);

    printf("fmt:%d, src_w:%d, src_h:%d, d_w:%d, d_h:%d\n", bo.fmt, bo.src_width, bo.src_height, bo.dst_width, bo.dst_height);

    nplane = COLOR_GET_PLANE_COUNT(bo.fmt);

    for (int j = 0; j < nplane; j++) {
        if (argv[i] != NULL) {
            bo.addr[j] = (void *)strtoul(argv[i++], NULL, 0);
            printf("plane[%d]:%p\n", j, bo.addr[j]);
        }

        if ((nplane > 1) && (bo.addr[j] == NULL)) {
            printf("addr input invalid %d,%p\n", j, bo.addr[j]);
            return -1;
        }
    }

    if (argv[i] != NULL) {
        display_id = strtoul(argv[i], NULL, 0);
        if ((display_id > DISPLAY_NUM-1) || (display_id < 0))
            display_id = 0;
        printf("post display id:%d\n", display_id);
    }
    ++i;
#ifdef CONFIG_GPU_HAL
    int g2d_func = 0;
    if (!strcmp(argv[i], "-g2d")) {
        g2d_func = strtoul(argv[++i], NULL, 0);
        printf("g2d function %d run.\n", g2d_func);
        func = G2DLITE_SHOW;
        goto exe_func;
    } else if (!strcmp(argv[i], "-stest")) {
#else
    if (!strcmp(argv[i], "-stest")) {
#endif
        func = SERVICE_TEST;
        goto exe_func;
    }
    else if (argv[i] != NULL) {
        noblock = strtoul(argv[i], NULL, 0);
    }
    else {}

    if (bo.addr[0] == NULL) {
        printf("base_addr is null, will show pattern\n");
        func = SHOW_PATTERN;
    } else func = SHOW_MEM;

exe_func:
    switch (func) {
        case SHOW_PATTERN:
            dshow_pattern(&bo, display_id);
            if (!noblock)
                bo_free(&bo);
            break;
        case SHOW_MEM:
            dshow_post(display_id, &bo);
            if (!noblock)
                bo_free(&bo);
            break;
        case SCREEN_CLEAR:
            handle = hal_get_display_handle(display_id);
            sdm_clear_display(handle);
            break;
        case DUMP_REGISTERS:
            dump_registers(addr, val);
            break;
        case WRITE_REGISTERS:
            write_registers(addr, val);
            break;
        case DUMP_CLOCKS:
            dump_clocks(clk_num, freq);
            break;
#ifdef CONFIG_GPU_HAL
        case G2DLITE_SHOW:
            dshow_g2dlite(&bo, display_id, g2d_func);
            break;
#endif
        case SERVICE_TEST:
            dshow_service_test(&bo, display_id);
            break;
        default:
            break;
    }

    return 0;
}

CLI_CMD("dshow", "display show test", dshow);
