/**
 * @file sdrv_pvt.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt lld driver implement file
 *
 * Revision History:
 * -----------------
*/
#include <debug.h>
#include <reg.h>
#include <types.h>

#include "sdrv_pvt.h"

#define PVT_REGISTER_DOUT_ADDR_OFFSET 0x4
#define PVT_REGISTER_HYST_H_OFFSET 0x8
#define PVT_REGISTER_HYST_L_OFFSET 0xC
#define PVT_REGISTER_HYST_R_OFFSET 0x10
#define PVT_REGISTER_HYST_F_OFFSET 0x14
#define PVT_REGISTER_HYST_TIME_OFFSET 0x18
#define PVT_REGISTER_INT_EN_OFFSET 0x1C
#define PVT_REGISTER_INT_STATUS_OFFSET 0x20
#define PVT_REGISTER_INT_CLR_OFFSET 0x24

#define PVT_CHAN_REGIONS_WIDTH      0x20

static bool check_channle(int32_t chan)
{
    if (chan < 0 ||
        chan >= PVT_SENSOR_ALARM_CHANNUM) {
        return false;
    }

    return true;
}

static inline void clear_regions(uint32_t*data, uint8_t startBit, uint8_t bitLen)
{
    // check parameter
    if ((startBit + bitLen) >= (sizeof(uint32_t) * 8)) {
        return;
    }

    // clear bit region
    uint32_t mask_win = 0;
    mask_win = (1 << bitLen) - 1;
    mask_win = mask_win << startBit;
    mask_win = ~mask_win;

    *data &= mask_win;
}

static inline void set_regions(uint32_t* data, uint8_t startBit, uint8_t bitLen, uint32_t setData)
{
    // check para valid
    if (((startBit + bitLen) >= (sizeof(uint32_t) * 8)) ||
        (setData > ((1 << bitLen) - 1))) {
        return;
    }

    // clear bit region
    clear_regions(data, startBit, bitLen);

    // set bit region
    *data |= setData << startBit;
}

static inline uint32_t get_regions(uint32_t data, uint8_t startBit, uint8_t bitLen)
{
    if ((startBit + bitLen) >= (sizeof(uint32_t) * 8)) {
        return 0;
    }

    uint32_t mask_win = 0;

    mask_win = (1 << bitLen) - 1;
    mask_win = mask_win << startBit;
    data &= mask_win;

    return (data >> startBit);
}

int32_t sdrv_pvt_lld_set_ctrl_ctlMode(paddr_t base, int32_t mode)
{
    uint32_t reg_addr = base + 0;
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 0, 1, mode);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_ctrl_enable(paddr_t base, int32_t enable)
{
    uint32_t reg_addr = base + 0;
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 4, 1, enable);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_ctrl_sample_mode(paddr_t base, pvt_sensor_type_enum_t devType, pvt_sample_mode_enum_t mode)
{
    uint32_t reg_addr = base + 0;
    uint32_t data = 0;
    uint8_t pmode = 0;
    uint32_t vmode = 0;

    if (mode == PVT_SENSOR_SAMPLE_MODE_TEMP) {
        pmode = 0;
        vmode = 0;
    }
    else if (mode == PVT_SENSOR_SAMPLE_MODE_VOLTAGE) {
        pmode = 0;
        vmode = 1;
    }
    else if (mode == PVT_SENSOR_SAMPLE_MODE_PROCESS) {
        pmode = devType + 1;
        vmode = 0;
    }
    else {
        // error mode
        return -1;
    }

    data = readl(reg_addr);
    set_regions(&data, 1, 1, vmode);
    set_regions(&data, 2, 2, pmode);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_ctrl_alarm_mode(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t type, int32_t enable)
{
    uint32_t reg_addr = base + 0;
    uint32_t data = 0;
    uint8_t start_bit = 0;
    uint8_t mode_en = 0;

    if (chan == PVT_SENSOR_ALARM_CHAN0) {
        start_bit = 16;
    }
    else if (chan == PVT_SENSOR_ALARM_CHAN1) {
        start_bit = 18;
    }
    else {
        // chan invalid
        return -1;
    }

    if (type == PVT_SENSOR_ALARM_TYPE_HYST_F ||
        type == PVT_SENSOR_ALARM_TYPE_HYST_R) {
        mode_en = 0x2;
    }
    else if (type == PVT_SENSOR_ALARM_TYPE_HYST_H ||
             type == PVT_SENSOR_ALARM_TYPE_HYST_L) {
        mode_en = 0x1;
    }
    else {
        // type invalid
        return -1;
    }

    data = readl(reg_addr);
    if (enable == PVT_REG_ENABLE) {
        set_regions(&data, start_bit, 2, mode_en);
    }
    else {
        set_regions(&data, start_bit, 2, 0);
    }
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_ctrl_trimo(paddr_t base, int32_t trimo)
{
    uint32_t reg_addr = base + 0;
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 10, 6, trimo);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_ctrl_trimg(paddr_t base, int32_t trimg)
{
    uint32_t reg_addr = base + 0;
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 5, 5, trimg);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_get_dout_data(paddr_t base, int32_t* dataOut)
{
    uint32_t reg_addr = base + PVT_REGISTER_DOUT_ADDR_OFFSET;
    uint32_t data = 0;

    data = readl(reg_addr);
    *dataOut = get_regions(data, 1, 10);

    return 0;
}

int32_t sdrv_pvt_lld_get_dout_valid(paddr_t base, int32_t* valid)
{
    uint32_t reg_addr = base + PVT_REGISTER_DOUT_ADDR_OFFSET;
    uint32_t data = 0;

    data = readl(reg_addr);
    *valid = get_regions(data, 0, 1);

    return 0;
}

int32_t sdrv_pvt_lld_set_hyst_high(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_H_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 0, 10, high);
    set_regions(&data, 10, 10, low);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_hyst_low(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_L_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 0, 10, high);
    set_regions(&data, 10, 10, low);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_hyst_raising(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_R_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 0, 10, high);
    set_regions(&data, 10, 10, low);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_set_hyst_falling(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t high, uint16_t low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_F_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    set_regions(&data, 0, 10, high);
    set_regions(&data, 10, 10, low);
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_get_hyst_high(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_H_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    *high = get_regions(data, 0, 10);
    *low = get_regions(data, 10, 10);

    return 0;
}

int32_t sdrv_pvt_lld_get_hyst_low(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_L_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    *high = get_regions(data, 0, 10);
    *low = get_regions(data, 10, 10);

    return 0;
}

int32_t sdrv_pvt_lld_get_hyst_raising(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_R_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    *high = get_regions(data, 0, 10);
    *low = get_regions(data, 10, 10);

    return 0;
}

int32_t sdrv_pvt_lld_get_hyst_falling(paddr_t base, pvt_alarm_chan_emum_t chan, uint16_t* high, uint16_t* low)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_F_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    *high = get_regions(data, 0, 10);
    *low = get_regions(data, 10, 10);

    return 0;
}

int32_t sdrv_pvt_lld_get_hyst_timer_f(paddr_t base, pvt_alarm_chan_emum_t chan, uint32_t* time)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_TIME_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    *time = get_regions(data, 16, 16);

    return 0;
}

int32_t sdrv_pvt_lld_get_hyst_timer_r(paddr_t base, pvt_alarm_chan_emum_t chan, uint32_t* time)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_HYST_TIME_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    *time = get_regions(data, 0, 16);

    return 0;
}

int32_t sdrv_pvt_lld_set_int(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType, int32_t enable)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_INT_EN_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;

    data = readl(reg_addr);
    if (enable == PVT_REG_ENABLE) {
        set_regions(&data, 0, 4, alarmType);
    }
    else if (enable == PVT_REG_DISABLE) {
        clear_regions(&data, 0, 4);
    }
    writel(data, reg_addr);

    return 0;
}

int32_t sdrv_pvt_lld_clr_int_sta(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType)
{
    uint32_t clr_addr = base + PVT_REGISTER_INT_CLR_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);

    uint32_t data = 0;
    data = readl(clr_addr);
    data = (data & (~(alarmType))) | alarmType;
    writel(data, clr_addr);
    return 0;
}

int32_t sdrv_pvt_lld_get_int(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType, int32_t* enableSta)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_INT_EN_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;
    uint32_t reg_val = 0;

    data = readl(reg_addr);
    reg_val = get_regions(data, 0, 4);
    if ((alarmType & reg_val) == 0) {
        *enableSta = PVT_REG_DISABLE;
    }
    else {
        *enableSta = PVT_REG_ENABLE;
    }

    return 0;
}

int32_t sdrv_pvt_lld_get_int_status(paddr_t base, pvt_alarm_chan_emum_t chan, pvt_sensor_alarm_type_enum_t alarmType, int32_t* status)
{
    if (!check_channle(chan)) {
        // channel error
        return -1;
    }

    uint32_t reg_addr = base + PVT_REGISTER_INT_STATUS_OFFSET + (chan * PVT_CHAN_REGIONS_WIDTH);
    uint32_t data = 0;
    uint32_t reg_val = 0;

    data = readl(reg_addr);
    reg_val = get_regions(data, 0, 4);
    if ((alarmType & reg_val) == 0) {
        *status = PVT_REG_DISABLE;
    }
    else {
        *status = PVT_REG_ENABLE;
    }

    return 0;
}

/* end of file sdrv_pvt.c */