/**
 * @file xf6020.c
 * @brief xf6020 source file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <debug.h>
#include <param.h>
#include <res_loader.h>
#include "xf6020.h"
#include "driver/i2c.h"

#define CODEC_XF6020_DBG_PRT SSDK_DEBUG

static am_reg_t xf6020_regs[] = {{XF6020_00_VOICE_MODE, 0x0, CACHE_INVALID},
    {XF6020_01_PARAM, 0x0, CACHE_INVALID},
    {XF6020_02_BYPASS, 0x0, CACHE_INVALID},
    {XF6020_0C_VERSION, 0x0, CACHE_INVALID},
    {XF6020_0D_VIR_RESET, 0x0, CACHE_INVALID}
};

static am_ctl_t xf6020_ctls[] = {
    {XF6020_00_FF_VOICEMODE, XF6020_00_VOICE_MODE, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_01_FF_PARAM, XF6020_01_PARAM, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_02_FF_BYPASS, XF6020_02_BYPASS, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_0C_FF_VERSION, XF6020_0C_VERSION, 0xff, 0x0, CTL_NORMAL_TYPE},
    {XF6020_0D_01_RESET, XF6020_0D_VIR_RESET, 0x01, 0x0, CTL_GPIO_TYPE},
};

static int xf6020_write_vir_reg(am_codec_t *dev, unsigned int reg,
                                unsigned int val)
{
    am_ctl_t *am_ctl = am_codec_find_virctl(dev, reg);
    bool ret = false;

    if (!am_ctl) {
        ssdk_printf(SSDK_ERR, " %s can't find am_ctl \n", __FUNCTION__);
        return false;
    }

    if (am_ctl->put) {
        ret = am_ctl->put(dev, am_ctl->ctl_id, val);
        ssdk_printf(CODEC_XF6020_DBG_PRT, "[XF6020] %s (VIR%XH, %XH)\n",
                __FUNCTION__, reg, val);
        return ret;
    }

    return true;
}

static int xf6020_read_vir_reg(am_codec_t *dev, unsigned int reg,
                               unsigned int *val)
{
    am_ctl_t *am_ctl = am_codec_find_virctl(dev, reg);
    bool ret = false;

    if (!am_ctl) {
        ssdk_printf(SSDK_ERR, " %s can't find am_ctl \n", __FUNCTION__);
        return false;
    }

    if (am_ctl->get) {
        ret = am_ctl->get(dev, am_ctl->ctl_id, val);
        ssdk_printf(CODEC_XF6020_DBG_PRT, "[XF6020] %s (VIR%XH, %XH)\n",
                __FUNCTION__, reg, *val);
        return ret;
    }

    return true;
}

static int xf6020_read_reg(void *handle, unsigned int addr, unsigned int reg,
                           void *data)
{
    unsigned char tx[3] = {0}, rx[4] = {0};
    int wlen, rlen;
    int ret;
    char *p = (char *)data;

    if (reg != XF6020_0C_VERSION) {

        wlen = 2;
        rlen = 1;
        tx[0] = (unsigned char)(0xFF & (reg >> 8));
        tx[1] = (unsigned char)(0xFF & reg);

        ret = i2c_write_read(handle, addr, tx, wlen, rx, rlen);

        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "%s: error: read xf6020 reg=%x,ret=%x\n", __func__, reg,
                    ret);
            return ret;
        }

        ssdk_printf(CODEC_XF6020_DBG_PRT,
                "%s: result: read i2c ext addr=0x%x,reg=0x%x 0x%x\n", __func__,
                addr, reg, rx[0]);
        p[0] = rx[0] & 0xff;

        return ret;
    }

    /* xf6020 reg version will return 4 bytes data */
    wlen = 2;
    rlen = 4;
    tx[0] = (unsigned char)(0xFF & (reg >> 8));
    tx[1] = (unsigned char)(0xFF & reg);

    ret = i2c_write_read(handle, addr, tx, wlen, rx, rlen);

    if (ret < 0) {
        ssdk_printf(SSDK_ERR, "%s: error: read xf6020 reg=%x,ret=%x\n", __func__, reg,
                ret);
        return ret;
    }

    p[0] = rx[0] & 0xff;
    p[1] = rx[1] & 0xff;
    p[2] = rx[2] & 0xff;
    p[3] = rx[3] & 0xff;

    return ret;
}

static int xf6020_write_reg(void *handle, unsigned int addr, unsigned int reg,
                            unsigned int val)
{
    uint8_t tx[4];
    int wlen = 1;
    int regcnt = 2;
    int ret = 0;

    ssdk_printf(CODEC_XF6020_DBG_PRT, "[xf6020] %s (%XH, %XH,%XH)\n", __FUNCTION__,
            reg, val, addr);

    if (reg != XF6020_0C_VERSION) {
        tx[0] = (unsigned char)(0xFF & (reg >> 8));
        tx[1] = (unsigned char)(0xFF & reg);
        tx[2] = (unsigned char)(0xFF & val);

        ret = i2c_write_read(handle, addr, tx, regcnt, &tx[2], wlen);

        if (ret < 0) {
            ssdk_printf(SSDK_ERR, "%s: error: %p reg=0x%x, val=0x%x\n", __func__, handle,
                    reg, val);
        }
    }

    return ret;
}

/**
 * @brief init xf6020 function
 *
 * @param dev
 * @return true
 * @return false
 */
static bool sdrv_xf6020_initialize(am_codec_t *dev)
{
    if (!dev) {
        ssdk_printf(SSDK_ERR, " %s dev is null\n", __FUNCTION__);
        return false;
    }

    return true;
}

/**
 * @brief read a register value from xf6020
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_xf6020_read_reg(am_codec_t *dev, unsigned int reg,
                                 unsigned int *val)
{

    xf6020_drvdata_t *drvdata = (xf6020_drvdata_t *)am_codec_get_drvdata(dev);
    int ret;

    if (!drvdata) {
        ssdk_printf(SSDK_ERR, " %s drvdata is null\n", __FUNCTION__);
        return false;
    }

    if (reg >= XF6020_0D_VIR_RESET) {
        return xf6020_read_vir_reg(dev, reg, val);
    }

    ret = xf6020_read_reg(drvdata->handle, drvdata->addr, reg, val);

    if (ret < 0) {
        return false;
    }

    return true;
}

/**
 * @brief write a val to an xf6020 register.
 *
 * @param dev
 * @param reg
 * @param val
 * @return true
 * @return false
 */
static bool sdrv_xf6020_write_reg(am_codec_t *dev, unsigned int reg,
                                  unsigned int val)
{

    xf6020_drvdata_t *drvdata = (xf6020_drvdata_t *)am_codec_get_drvdata(dev);
    int ret;

    if (!drvdata) {
        ssdk_printf(SSDK_ERR, " %s drvdata is null\n", __FUNCTION__);
        return false;
    }

    if (reg >= XF6020_0D_VIR_RESET) {
        return xf6020_write_vir_reg(dev, reg, val);
    }

    ret = xf6020_write_reg(drvdata->handle, drvdata->addr, reg, val);

    if (ret < 0) {
        return false;
    }

    return true;
}

static bool sdrv_xf6020_writeable_reg(am_codec_t *dev, unsigned int reg)
{
    if (!dev) {
        ssdk_printf(SSDK_ERR, " %s dev is null\n", __FUNCTION__);
        return false;
    }

    return true;
}

static bool sdrv_xf6020_readable_reg(am_codec_t *dev, unsigned int reg)
{
    if (!dev) {
        ssdk_printf(SSDK_ERR, " %s dev is null\n", __FUNCTION__);
        return false;
    }

    switch (reg) {
        case XF6020_0D_VIR_RESET:
            return false;

        default:
            return true;
    }

    return true;
}

static am_codec_ops_t xf6020_drv_ops = {
    .initialize = sdrv_xf6020_initialize,
    .read_reg = sdrv_xf6020_read_reg,
    .write_reg = sdrv_xf6020_write_reg,
    .writeable_reg = sdrv_xf6020_writeable_reg,
    .readable_reg = sdrv_xf6020_readable_reg,
};

am_codec_t *xf6020_codec_create(const char *codec_name)
{

    unsigned int regs_numb = ARRAY_SIZE(xf6020_regs);
    unsigned int ctls_numb = ARRAY_SIZE(xf6020_ctls);
    am_codec_t *codec = NULL;

    codec = am_codec_create(codec_name, &xf6020_drv_ops, xf6020_regs, regs_numb,
                            xf6020_ctls, ctls_numb);

    if (!codec) {
        ssdk_printf(SSDK_ERR, "\t[xf6020s] failed creat codec \r\n");
    }

    return codec;
}

void xf6020_codec_destroy(am_codec_t *codec)
{
    if (codec) {
        am_codec_destroy(codec);
    }
}
