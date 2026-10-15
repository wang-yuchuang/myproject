/*
 * serdes_common.h
 *
 * Semidrive platform csi header file
 *
 * Copyright (C) 2021, Semidrive  Communications Inc.
 *
 * This file is licensed under a dual GPLv2 or X11 license.
 */

#ifndef SERDES_COMMON_H
#define SERDES_COMMON_H

#include "exvdev.h"

#define MAX_SERAILIZER_NUM 4

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define PROBE_INIT

int serdes_read_reg_16(i2c_adap_dev_t *client, u8 address, u16 reg, u8 *val);
int serdes_write_reg_16(i2c_adap_dev_t *client, u8 address, u16 reg, u8 val);
int serdes_read_reg_8(i2c_adap_dev_t *client, u8 address, u8 reg, u8 *val);
int serdes_write_reg_8(i2c_adap_dev_t *client, u8 address, u8 reg, u8 val);

enum serdes_status {
    SERDES_IDLE,
    SERDES_INIT_LOCK_READY,
    SERDES_SENSOR_LOCK_READY,
    SERDES_SENSOR_READY,
    SERDES_DONE,
};

struct serdes_init_status {
    int        init_status;
    int        stream_cnt;
    int        ref_count;
};

typedef struct __serdes_param serdes_para_t;

/**
 *  \brief Register address and value pair, with delay.
 */
typedef struct {
    u16 nRegAddr;
    /**< Register Address */
    u8 nRegValue;
    /**< data */
    unsigned int nDelay;
    /**< Delay to be applied, after the register is programmed */
} serdes_param_reg16_t;

/**
 *  \brief Register address and value pair, with delay.
 */
typedef struct {
    u8 nRegAddr;
    /**< Register Address */
    u8 nRegValue;
    /**< data */
    unsigned int nDelay;
    /**< Delay to be applied, after the register is programmed */
} serdes_param_reg8_t;

struct serdes_dev_t {
    struct vdev_device
        vdev[MAX_SERAILIZER_NUM];//for sync=0 mode one vdev for one camera
    i2c_adap_dev_t *i2c_client;
    const char *module_name;
    char *gpioext_name;

    int last_lock_status;
    int current_lock_status;

    u8  serdes_id;
    int channel;
    u8 addr_des;
    u8 addr_ser;
    u8 addr_poc;
    u8 addr_extgpio;

    int power_gpio;
    int poc_gpio;
    int gpi_gpio;
    u32 sync;
    struct serdes_init_status serdes_status;

    int ser_online_bitmap;//the ser online bit map is the actual online serializer map
    int serdes_streamon_bitmap; //indicate which channel is stream on
    osMutexId_t lock;
    osMutexId_t dev_init_lock;
    //mutex_t dev_init_lock;

    void *hotplug_thread;
    int hotplug_should_exit;
    int hotplug_exited;

    serdes_para_t *priv;
};

struct __serdes_param {
    const char *module_name;
    u32 bus_type;
    u32 numerator;
    u32 colorspace;
    u32 field;
    u32 power_on_flag;

    //these fields are per camera
    u32 mbus_code[MAX_SERAILIZER_NUM];
    u32 fps[MAX_SERAILIZER_NUM];
    u8  remap_addr_ser[MAX_SERAILIZER_NUM];//the serializer i2c address. the remapped address within one i2c bus should not be the same

    void (*serdes_power) (struct serdes_dev_t *dev, bool on);
    int  (*serdes_init) (struct serdes_dev_t *dev);
    void (*serdes_start) (struct serdes_dev_t *dev, bool on);
    int  (*serdes_check_link_status)(struct serdes_dev_t *serdes);
    int  (*serdes_check_link_status_ex)(struct serdes_dev_t
                                 *serdes);//check link lock/video lock for specific channel to determine if a reinit is needed
    bool  (*serdes_check_video_lock_status)(struct serdes_dev_t
                                     *serdes);//for 96722 check video lock when link lock is ok
};
#endif
