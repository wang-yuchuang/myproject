/**
 * @file disp_drv.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <board.h>
#include <irq_num.h>
#include <disp_config.h>
#include <disp_ctrl.h>
#include <sys/types.h>
#include <disp_panel.h>
#include <disp_interface.h>
#include <disp_drv.h>
#include <disp_general_header.h>
#include <disp_drv_log.h>
#include <driver/irq.h>
#include <disp_hal.h>
#include <disp_data_type.h>
#include <reg.h>
#include "disp_errcode.h"

extern const struct dc_operations kunlun_dc_ops;
extern bool vsync_r;
struct dc_device dc_dev[MAX_DISP_NUM] = {0};
struct intf_device intf_dev[MAX_DISP_NUM] = {0};

#define POLYNOMIAL 0x04c11db7L
static uint32_t crc_table[256];

static unsigned long disp_get_addr(paddr_t addr)
{
    return (unsigned long ) _ioaddr(addr);
}

static struct dc_operations *get_dc_ops(void)
{
    return &kunlun_dc_ops;
}

static int get_if_type_by_res(int if_res)
{
    switch(if_res) {
    case IF_RES_MIPI_DSI1:
    case IF_RES_MIPI_DSI2:
    case IF_RES_DSI1_DSI2:
        return IF_TYPE_DSI;
    case IF_RES_LVDS1:
    case IF_RES_LVDS2:
    case IF_RES_LVDS3:
    case IF_RES_LVDS4:
    case IF_RES_LVDS1_LVDS2:
    case IF_RES_LVDS3_LVDS4:
    case IF_RES_LVDS1_4:
        return IF_TYPE_LVDS;
    case IF_RES_PARALLEL:
        return IF_TYPE_PARALLEL;
    default:
         DISPERR("not support this if res [%d]\n", if_res);
         return IF_TYPE_INVALID;
    }
}

static int get_if_module_by_panel(struct sdm_panel *panel)
{
    switch(panel->if_type) {
    case IF_TYPE_DSI:
        return DISP_IF_MODULE_DSI;
    case IF_TYPE_LVDS:
        return DISP_IF_MODULE_LVDS;
    case IF_TYPE_PARALLEL:
        return DISP_IF_MODULE_PARALLEL;
    default:
         DISPERR("not support this if type [%d]\n", panel->if_type);
         return DISP_IF_MODULE_UNKNOWN;
    }
}

static int get_tcon_irq(mod_res_t *res_info)
{
    switch (res_info->res_id) {
    case DC_RES_DC1:
        return DC1_DC_1_INTR_NUM;
    case DC_RES_DC2:
        return DC2_DC_1_INTR_NUM;
    case DC_RES_DC3:
        return DC3_DC_1_INTR_NUM;
    case DC_RES_DC4:
        return DC4_DC_1_INTR_NUM;
    case DC_RES_DC5:
        return DC5_DC_1_INTR_NUM;
    default:
        DISPERR("irq:not support this res [%d]\n", res_info->res_id);
        return -1;
    }
}

static int get_dc_index(mod_res_t *res_info)
{
    return res_info->res_id;
}

static int get_mlc_select(int dc_num, int dc_id, int dp)
{
    switch (dp) {
        case DP_RES_DP1:
        case DP_RES_DP3:
            return 0x1;
        case DP_RES_DP2:
            return 0x2;
        case DP_RES_DP1_DP2:
            if (dc_num == 1)
                return 0x3;
            else if (dc_id)
                return 0x2;
            else
                return 0x1;
        default:
            return 0;
    }
}

static int get_dsp_clk(int clk, int lvds_mode)
{
    if (lvds_mode == LVDS_MODE_DUAL)
        return clk / 3.5;

    return clk / 7.0;
}

static struct intf_device* handle_to_intf(display_handle *handle)
{
    int dc_id;
    mod_res_t *dc_res;

    dc_res = &handle->dm->dc[0];
    dc_id = get_dc_index(dc_res);
    intf_dev[dc_id].display_id = handle->display_id;
    intf_dev[dc_id].intf_res = handle->res->if_res;
    intf_dev[dc_id].lvds_cmb = handle->res->lvds_cmb;
    intf_dev[dc_id].lvds_mode = handle->res->lvds_mode;
    intf_dev[dc_id].pixel_bpp = handle->res->pixel_bpp;
    intf_dev[dc_id].tcon_clk = get_dsp_clk(handle->res->clk, handle->res->lvds_mode);

    return &intf_dev[dc_id];
}

int display_if_init(display_handle *handle, struct sdm_panel *panel)
{
    int if_module = DISP_IF_MODULE_UNKNOWN;
    struct intf_device *dev = NULL;

    DISPFUNC();

    dev = handle_to_intf(handle);

    if_module = get_if_module_by_panel(panel);
    disp_interface_init(if_module, dev);
    disp_interface_config(if_module, dev, panel);
    disp_interface_power_on(if_module, dev);

    return if_module;
}

int display_if_deinit(display_handle *handle, struct sdm_panel *panel)
{
    int if_module = DISP_IF_MODULE_UNKNOWN;
    struct intf_device *dev = NULL;

    DISPFUNC();

    dev = handle_to_intf(handle);
    if_module = get_if_module_by_panel(panel);
    disp_interface_deinit(if_module, dev);

    return 0;
}

int panel_init(display_handle *handle, struct sdm_panel *panel)
{
    struct intf_device *dev = NULL;

    dev = handle_to_intf(handle);

    switch (panel->if_type) {
    case IF_TYPE_DSI:
        mipi_lcm_init(dev->dsi_index, panel);
        break;
    case IF_TYPE_LVDS:
        break;
    case IF_TYPE_PARALLEL:
        break;
    case IF_TYPE_HDMI:
    case IF_TYPE_I2C:
    case IF_TYPE_SPI:
        DISPERR("External interface not init here\n");
        break;
    default:
        DISPERR("not support this if type [%d]\n", panel->if_type);
        return -1;
    }
    return 0;
}

int callback_vsync_register(display_handle *handle, CALLBACK_EVENT_T func)
{
    handle->vsync = func;

    return 0;
}

int disp_vsync_callback(int display_id) {
    display_handle *handle = hal_get_display_handle(display_id);

    if (handle->vsync)
        return handle->vsync(display_id, 0);
    return 0;
}

unsigned int disp_crc32_status(int display_id) {
    int dc_id = 0;
    struct dc_device *dev = NULL;
    unsigned int val, val_r, val_e;
    mod_res_t *dc_res;

    display_handle *handle = hal_get_display_handle(display_id);
    dc_res = &handle->dm->dc[0];
    dc_id = get_dc_index(dc_res);

    dev = &dc_dev[dc_id];

    val = readl(dev->base + 0xe004);
    writel(val, dev->base + 0xe004);

    val_e = readl(dev->base + 0xe018);
    val_r = readl(dev->base + 0xe01c);

    printf("%d, e:%08x r:%08x\n", display_id, val_e, val_r);
    val = val >> 8;
    return val;
}

int disp_init(display_handle *handle)
{
    int tcon_irq, dc_id;
    int mlc_select;
    double ratio, dsp_clk;
    mod_res_t *dc_res;
    struct sdm_panel *panel;
    struct dc_device *dev = NULL;
    const double pix_clk = 415800000;//415.8MHz

    if (!handle) {
        DISPERR("%s handle is null\n", __func__);
        return -1;
    }

    dsp_clk = get_dsp_clk(handle->res->clk, handle->res->lvds_mode);
    panel = handle->panels_info[handle->found_panel]->panel;;
    ratio = pix_clk / dsp_clk;

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        tcon_irq = get_tcon_irq(dc_res);
        mlc_select = get_mlc_select(handle->dm->num_dc, dc_id, handle->res->dp_res);
        dev = &dc_dev[dc_id];

        dev->display_id = handle->display_id;
        dev->dc_idx = dc_id;

        if (handle->dm->num_dc == 1) {
            dev->ms_info.ms_en = 0;
        } else {
            dev->ms_info.ms_en = 1;
            dev->ms_info.start = 0;
            dev->ms_info.end = panel->display_timing->hactive / 2;
        }
        dev->ms_info.ms_mode = MASTER_MODE;

        dev->irq = tcon_irq;
        dev->base = (unsigned long)_ioaddr(dc_res->base);
        dev->mlc_select = mlc_select;
        dev->dsp_clk = dsp_clk;
        dev->ratio = ratio;
        dev->vsync_wait = 0;
        dev->mutex_refresh = osMutexNew(NULL);
        dev->dp_res = handle->res->dp_res;
        dev->ops = get_dc_ops();

        if (handle->os_type == DIL2)
            dev->ops->init(dev, panel);

        if (dev->ops->check_tcon_status && (dev->ops->check_tcon_status(dev) != 0)) {
            dev->run_status = true;
            DISPMSG("run_status true\n");
        }
        if (handle->is_need_register_irq) {
            irq_attach(dev->irq, dev->ops->irq_handler, dev);
            irq_enable(dev->irq);
        }
    }

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        tcon_irq = get_tcon_irq(dc_res);
        mlc_select = get_mlc_select(handle->dm->num_dc, dc_id, handle->res->dp_res);
        dev = &dc_dev[dc_id];

        dev->display_id = handle->display_id;
        dev->dc_idx = dc_id;
        dev->ms_info.ms_en = 1;
        dev->ms_info.ms_mode = SLAVE_MODE;
        dev->ms_info.start = panel->display_timing->hactive / 2;
        dev->ms_info.end = panel->display_timing->hactive;
        dev->irq = tcon_irq;
        dev->base = (unsigned long)_ioaddr(dc_res->base);
        dev->mlc_select = mlc_select;
        dev->dsp_clk = dsp_clk;
        dev->ratio = ratio;
        dev->ops = get_dc_ops();
        dev->mutex_refresh = osMutexNew(NULL);

        if (handle->os_type == DIL2)
            dev->ops->init(dev, panel);
    }

    return 0;
}

int disp_uninit(display_handle *handle)
{
    return 0;
}

int disp_clear_layers(display_handle *handle, uint8_t mask, uint8_t z_order)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dev->ops->clear_layers(dev, mask, z_order);
        dev->ops->triggle(dev);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dev->ops->clear_layers(dev, mask, z_order);
        dev->ops->triggle(dev);
    }

    return 0;
}

int disp_post_config(display_handle *handle, struct sdm_post_config *post)
{
    int dc_id = 0;
    int res;
    int retry_cnt = 0;
    mod_res_t *dc_res = NULL;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        res = osMutexAcquire(dev->mutex_refresh, 1000);
        if (res) {
            DISPMSG("display[%d] got mutex timeout: %d\n", handle->display_id, res);
        }

        dev->ops->update(dev, post);
        dev->ops->triggle(dev);

        if (!dev->run_status) {
            dev->ops->enable(dev);
            dev->run_status = true;
        }
        if (!res)
            osMutexRelease(dev->mutex_refresh);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        res = osMutexAcquire(dev->mutex_refresh, 1000);
        if (res) {
            DISPMSG("display[%d] got mutex timeout: %d\n", handle->display_id, res);
        }
        dev->ops->update(dev, post);

        if (!dev->run_status) {
            dev->ops->vsync_enable(dev, true);
        }

        dev->ops->triggle(dev);

        if (!dev->run_status) {
            dev->ops->enable(dev);
            dev->run_status = true;
        }

        if (!res)
            osMutexRelease(dev->mutex_refresh);
    }

    if (post->psot_no_wait_vsync) {
        return 0;
    }
    while(dev->ops->check_triggle_status(dev)) {
        udelay(1000);

        retry_cnt++;
        if (retry_cnt >= 100) { //func return after don't have interrupt over 100ms
            DISPERR("[disp] dc[%d] clear flc time out\n", dc_id);
            break;
        }
    }

    return 0;
}

bool disp_panel_type_check(display_handle *handle, struct sdm_panel *panel)
{
    int if_type;
    struct intf_device *dev = NULL;

    dev = handle_to_intf(handle);
#ifdef PROJECT_BF200
    if (dev->display_id == 2)
        dev->intf_res = IF_RES_PARALLEL;
#endif

    /*check if type*/
    if_type = get_if_type_by_res(dev->intf_res);
    if (panel->if_type != if_type) {
        DISPERR("Interface is not compatible\n");
        return false;
    }

    return true;
}


int disp_panel_disconnect(display_handle *handle, int sub_id,
        struct sdm_panel *panel)
{
    return 0;
}

int disp_pq_config(display_handle *handle, struct sdm_pq_params *pq)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dev->ops->csc_set(dev, pq);
        dev->ops->triggle(dev);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dev->ops->csc_set(dev, pq);
        dev->ops->triggle(dev);

    }
    return 0;
}

static void gen_crc_table(void)
{
    register int i, j;
    register unsigned int crc_accum;

    for (i = 0; i < 256; i++) {
        crc_accum = ((unsigned int) i << 24);
        for (j = 0; j < 8; j++) {
            if (crc_accum & 0x80000000L)
                crc_accum = (crc_accum << 1) ^ POLYNOMIAL;
            else
                crc_accum = (crc_accum << 1);
        }
        crc_table[i] = crc_accum;
    }
}

static unsigned int update_crc(unsigned int crc_accum, void *data_blk_ptr, int data_blk_size)
{
    unsigned char *p;
    p = (unsigned char*)data_blk_ptr;

    register int i, j;
    for (j = 0; j < data_blk_size; j++) {
        i = ((int) (crc_accum >> 24) ^ *p) & 0xff;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
        p++;
    }
    return crc_accum;
}

unsigned int sdrv_expect_crc32(int mode, struct sdm_buffer *layer, struct crc32block *crc32block)
{
    unsigned int crc = 0;
    unsigned int R;
    unsigned int G;
    unsigned int B;
    unsigned int indata;
    unsigned int *pindata;
    unsigned int indata_b0;
    unsigned int indata_b1;
    unsigned int indata_b2;
    unsigned int indata_b3;
    unsigned int indata_MSB;
    int stride = layer->src_stride[0];
    int off;
    unsigned int start_x = crc32block->cfg_start_x - layer->dst.x;
    unsigned int start_y = crc32block->cfg_start_y - layer->dst.y;
    unsigned int end_x = crc32block->cfg_end_x - layer->dst.x;
    unsigned int end_y = crc32block->cfg_end_y - layer->dst.y;
    char *addr = (char *)layer->addr[0];
    gen_crc_table();
    for (unsigned int i = start_y; i <= end_y; i++) {
        for (unsigned int j = start_x; j <= end_x; j++) {
            switch (layer->fmt) {
                case COLOR_ARGB8888:
                    off = layer->src.x * 4 + stride * layer->src.y;
                    B = addr[off + i * stride + j * 4];
                    G = addr[off + i * stride + j * 4 + 1];
                    R = addr[off + i * stride + j * 4 + 2];
                    break;
                case COLOR_BGRA8888:
                    off = layer->src.x * 4 + stride * layer->src.y;
                    R = addr[off + i * stride + j * 4 + 1];
                    G = addr[off + i * stride + j * 4 + 2];
                    B = addr[off + i * stride + j * 4 + 3];
                    break;
                case COLOR_ABGR8888:
                    off = layer->src.x * 4 + stride * layer->src.y;
                    R = addr[off + i * stride + j * 4];
                    G = addr[off + i * stride + j * 4 + 1];
                    B = addr[off + i * stride + j * 4 + 2];
                    break;
                case COLOR_RGB888:
                    off = layer->src.x * 3 + stride * layer->src.y;
                    B = addr[off + i * stride + j * 3];
                    G = addr[off + i * stride + j * 3 + 1];
                    R = addr[off + i * stride + j * 3 + 2];
                    break;
                case COLOR_BGR888:
                    off = layer->src.x * 3 + stride * layer->src.y;
                    R = addr[off + i * stride + j * 3];
                    G = addr[off + i * stride + j * 3 + 1];
                    B = addr[off + i * stride + j * 3 + 2];
                    break;
               default:
                    crc32block->block_use = 0;
                    return 0;
            }

            if (mode == CRC32_MODE_INTERNAL) {//internal
                B = (B << 2) | (B >> 6);
                G = (G << 2) | (G >> 6);
                R = (R << 2) | (R >> 6);
            } else {//external
                B = (B << 2);
                G = (G << 2);
                R = (R << 2);
            }

            indata = 0x00000000 | (R<<22) | (G<<12) | (B<<2);
            indata_b0 = (indata & 0x000000FF)<<24;
            indata_b1 = (indata & 0x0000FF00)<<8;
            indata_b2 = (indata & 0x00FF0000)>>8;
            indata_b3 = (indata & 0xFF000000)>>24;
            indata_MSB = indata_b0 | indata_b1 | indata_b2 | indata_b3;
            pindata = &indata_MSB;
            crc = update_crc(crc, pindata, 4);
        }
    }
    crc32block->expect_crc = crc;

    return 1;
}

int disp_crc32_set(display_handle *handle, struct sdm_buffer *buf, struct crc_info *info)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        //arch_clean_cache_range((addr_t)(buf->addr[0]), buf->src_stride[0] * buf->src.h);
        for (int i = 0; i < MAX_CRC_BLOCK_NUM; i++) {
            if (info->block[i].block_use) {
                sdrv_expect_crc32(CRC32_MODE_INTERNAL, buf, &info->block[i]);
                dev->ops->crc32_enable_irq(dev, i);
            }
        }

        dev->ops->crc32_set(dev, info);
    }

    return 0;
}

int disp_crc32_disable(display_handle *handle)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;
    struct crc_info info;

    info.global_enable = 0;

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dev->ops->crc32_disable(dev, &info);
    }

    return 0;
}

u32 disp_crc32_get_value(display_handle *handle, uint8_t index)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        return dev->ops->crc32_get_result(dev, index);
    }

    return 0;
}

int disp_safety_config(display_handle *handle, struct sdm_buffer *buf)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        if (dev->ops->safety_config)
            dev->ops->safety_config(dev, buf);
    }

    return 0;
}

void disp_safety_exit(display_handle *handle)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        if (dev->ops->safety_exit)
            dev->ops->safety_exit(dev);
    }
}

static void dump_registers(struct dc_device *dev, int start, int end)
{
    int i, count;

    count = (end - start) / 4 + 1;
    count = (count + 3) / 4;
    DISPERR("-----[dc:%d]-[0x%04x~0x%04x]-----\n",
        dev->dc_idx+1, start, end);
    for (i = 0; i < count; i++)
        DISPERR("0x%08x 0x%08x 0x%08x 0x%08x\n",
            readl(dev->base + start + 0x10 * i),
            readl(dev->base + start + 0x10 * i + 0x4),
            readl(dev->base + start + 0x10 * i + 0x8),
            readl(dev->base + start + 0x10 * i + 0xc));
}

static void dc_dump_registers(struct dc_device *dev)
{
    DISPERR("@@@@@@@@-[dc:%d]-start-@@@@@@@@\n", dev->dc_idx+1);
    dump_registers(dev, 0x0000, 0x002c);

    dump_registers(dev, 0x1000, 0x107c);
    dump_registers(dev, 0x1100, 0x1100);
    dump_registers(dev, 0x1200, 0x124c);

    dump_registers(dev, 0x2000, 0x204c);
    dump_registers(dev, 0x2200, 0x222c);
    dump_registers(dev, 0x2f00, 0x2f00);

    dump_registers(dev, 0x5000, 0x504c);
    dump_registers(dev, 0x5100, 0x514c);
    dump_registers(dev, 0x5200, 0x521c);
    dump_registers(dev, 0x5f00, 0x5f00);

    dump_registers(dev, 0x7000, 0x70bc);
    dump_registers(dev, 0x7200, 0x724c);

    dump_registers(dev, 0x9000, 0x90bc);
    dump_registers(dev, 0x9100, 0x912c);
    dump_registers(dev, 0xa000, 0xa02c);
    dump_registers(dev, 0xc000, 0xc00c);
    dump_registers(dev, 0xe000, 0xe08c);

    DISPERR("@@@@@@@@-[dc:%d]-end-@@@@@@@@\n", dev->dc_idx+1);
}

void disp_dump_register(display_handle *handle)
{
    int dc_id;
    mod_res_t *dc_res;
    struct dc_device *dev = NULL;

    if (handle->dm->num_dc > 1) {
        dc_res = &handle->dm->dc[1];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dc_dump_registers(dev);
    }

    if (handle->dm->num_dc > 0) {
        dc_res = &handle->dm->dc[0];
        dc_id = get_dc_index(dc_res);
        dev = &dc_dev[dc_id];

        dc_dump_registers(dev);
    }
}

