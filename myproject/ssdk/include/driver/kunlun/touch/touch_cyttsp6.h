/**
 * @file touch_cyttsp6.h
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: cyttsp6 touch driver header file
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdint.h>
#include <debug.h>

#include <driver/kunlun/touch/touch_vendor.h>
#include <touch_cfg.h>

#ifndef __CYTTSP6_TP_H__
#define __CYTTSP6_TP_H__

// #define CY_PRINT_SHOW
#define ALWAYS  SSDK_CRIT
#ifdef CY_PRINT_SHOW
#define CY_PRINT ALWAYS
#else
#define CY_PRINT SSDK_INFO
#endif

#define MAX_XFER_LEN (256)
#define CY_REG_BASE (0x00)

#define IS_BOOTLOADER(hst_mode, reset_detect) \
    ((hst_mode) & 0x01 || (reset_detect) != 0)

#define IS_BOOTLOADER_IDLE(hst_mode, reset_detect) \
    ((hst_mode) & 0x01 && (reset_detect) & 0x01)

#define min(a, b) ((a) < (b) ? (a) : (b))

#define cy_offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define GET_NUM_TOUCH_RECORDS(x)    ((x) & 0x1F)
#define IS_LARGE_AREA(x)            ((x) & 0x20)
#define IS_BAD_PKT(x)               ((x) & 0x20)
#define IS_TTSP_VER_GE(p, maj, min) \
    ((p)->si_ptrs.cydata == NULL ? \
        0 : ((p)->si_ptrs.cydata->ttsp_ver_major < (maj) ? \
        0 : ((p)->si_ptrs.cydata->ttsp_ver_minor < (min) ? 0 : 1)))

#define HI_BYTE(x)  (uint8_t)(((x) >> 8) & 0xFF)
#define LO_BYTE(x)  (uint8_t)((x) & 0xFF)

#define GET_HSTMODE(reg)    ((reg & 0x70) >> 4)
#define GET_TOGGLE(reg)     ((reg & 0x80) >> 7)

#define CY_NORMAL_ORIGIN    0/* upper, left corner */
#define CY_INVERT_ORIGIN    1/* lower, right corner */

/* Timeout in ms. */
#define CY_COMMAND_COMPLETE_TIMEOUT             5000
#define CY_CALIBRATE_COMPLETE_TIMEOUT           10000
#define CY_WATCHDOG_TIMEOUT                     1000
#define CY_CORE_REQUEST_EXCLUSIVE_TIMEOUT       1000
#define CY_DA_REQUEST_EXCLUSIVE_TIMEOUT         1000
#define CY_LDR_REQUEST_EXCLUSIVE_TIMEOUT        10000
#define CY_CORE_SLEEP_REQUEST_EXCLUSIVE_TIMEOUT 10000
#define CY_CORE_WAIT_SYSINFO_MODE_TIMEOUT       4000
#define CY_CORE_MODE_CHANGE_TIMEOUT             2000
#define CY_CORE_RESET_AND_WAIT_TIMEOUT          1000
#define CY_CORE_WAKEUP_TIMEOUT                  500
#define CY_LDR_CMD_TIMEOUT                      1000
#define CY_LDR_CMD_INIT_TIMEOUT                 20000
#define CY_CORE_CMD_WAIT_FOR_EVENT_TIMEOUT      50

#define CY_POST_CODEL_WDG_RST           0x01
#define CY_POST_CODEL_CFG_DATA_CRC_FAIL 0x02
#define CY_POST_CODEL_PANEL_TEST_FAIL   0x04
#define CY_POST_CODEL_SCAN_STATUS       0x08

/* x-axis resolution of panel in pixels */
#define CY_PCFG_RESOLUTION_X_MASK 0x7F

/* y-axis resolution of panel in pixels */
#define CY_PCFG_RESOLUTION_Y_MASK 0x7F

/* x-axis, 0:origin is on left side of panel, 1: right */
#define CY_PCFG_ORIGIN_X_MASK 0x80

/* y-axis, 0:origin is on top side of panel, 1: bottom */
#define CY_PCFG_ORIGIN_Y_MASK 0x80

/* button to keycode support */
#define CY_NUM_BTN_PER_REG      4
#define CY_BITS_PER_BTN         2

/* touch record system information offset masks and shifts */
#define CY_BYTE_OFS_MASK        0x1F
#define CY_BOFS_MASK            0xE0
#define CY_BOFS_SHIFT           5

#define CY_CMD_LDR_HOST_SYNC    0xFF
#define CY_START_OF_PACKET      0x01
#define CY_END_OF_PACKET        0x17

/* SYSINFO REGISTERS */
#define CY_NUM_REVCTRL          8
#define CY_NUM_TCH_FIELDS       7
#define CY_NUM_EXT_TCH_FIELDS   3

/* ttconfig block */
#define CY_TTCONFIG_VERSION_OFFSET  8
#define CY_TTCONFIG_VERSION_SIZE    2
#define CY_TTCONFIG_VERSION_ROW     0

#define CY_CONFIG_LENGTH_INFO_OFFSET    0
#define CY_CONFIG_LENGTH_INFO_SIZE      4
#define CY_CONFIG_LENGTH_OFFSET         0
#define CY_CONFIG_LENGTH_SIZE           2
#define CY_CONFIG_MAXLENGTH_OFFSET      2
#define CY_CONFIG_MAXLENGTH_SIZE        2
#define CY_CONFIG_GESTURE_ENABLE_ADDR   0x04F1

/* RAM ID */
#define CY_RAM_ID_SCAN_TYPE         0x4B
#define CY_RAM_ID_REFRESH_INTERVAL  0x1B
#define CY_RAM_ID_TOUCHMODE_ENABLED 0x02 /* Enable proximity */

/* Operational Mode Command Sizes */
/* NULL Command */
#define CY_CMD_OP_NULL_CMD_SZ           1
#define CY_CMD_OP_NULL_RET_SZ           0
/* Get Parameter */
#define CY_CMD_OP_GET_PARAM_CMD_SZ      2
#define CY_CMD_OP_GET_PARAM_RET_SZ      6
/* Set Parameter */
#define CY_CMD_OP_SET_PARAM_CMD_SZ      7
#define CY_CMD_OP_SET_PARAM_RET_SZ      2
/* Get Config Block CRC */
#define CY_CMD_OP_GET_CFG_BLK_CRC_CMD_SZ    2
#define CY_CMD_OP_GET_CFG_BLK_CRC_RET_SZ    3
/* Wait For Event */
#define CY_CMD_OP_WAIT_FOR_EVENT_CMD_SZ     2

/* CaT Mode Command Sizes */
/* NULL Command */
#define CY_CMD_CAT_NULL_CMD_SZ          1
#define CY_CMD_CAT_NULL_RET_SZ          0
/* Get Config Row Size */
#define CY_CMD_CAT_GET_CFG_ROW_SIZE_CMD_SZ  1
#define CY_CMD_CAT_GET_CFG_ROW_SIZE_RET_SZ  2
/* Read Config Block */
#define CY_CMD_CAT_READ_CFG_BLK_CMD_SZ      6
#define CY_CMD_CAT_READ_CFG_BLK_RET_SZ      7 /* + Data */
#define CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ  5
/* Write Config Block */
#define CY_CMD_CAT_WRITE_CFG_BLK_CMD_SZ     8 /* + Data + Security Key */
#define CY_CMD_CAT_WRITE_CFG_BLK_RET_SZ     5
#define CY_CMD_CAT_WRITE_CFG_BLK_CMD_HDR_SZ 6
/* Load BIST Self-Test Data */
#define CY_CMD_CAT_LOAD_BIST_ST_DATA_CMD_SZ 6 /* + Data */
#define CY_CMD_CAT_LOAD_BIST_ST_DATA_RET_SZ 4
/* Load Shorts Self-Test Data */
#define CY_CMD_CAT_LOAD_SHORTS_ST_DATA_CMD_SZ   6 /* + Data */
#define CY_CMD_CAT_LOAD_SHORTS_ST_DATA_RET_SZ   4
/* Run BIST Self-Test */
#define CY_CMD_CAT_RUN_BIST_ST_CMD_SZ       2
#define CY_CMD_CAT_RUN_BIST_ST_RET_SZ       3
/* Run Shorts Self-Test */
#define CY_CMD_CAT_RUN_SHORTS_ST_CMD_SZ     2
#define CY_CMD_CAT_RUN_BIST_ST_RET_SZ       3
/* Run Opens Self-Test */
#define CY_CMD_CAT_RUN_OPENS_ST_CMD_SZ      2
#define CY_CMD_CAT_RUN_OPENS_ST_RET_SZ      3
/* Run Capacitance Self-Test */
#define CY_CMD_CAT_RUN_CAP_ST_CMD_SZ        2
#define CY_CMD_CAT_RUN_CAP_ST_RET_SZ        3
/* Run Auto Short Self-Test */
#define CY_CMD_CAT_RUN_AUTOSHORTS_ST_CMD_SZ 2
#define CY_CMD_CAT_RUN_AUTOSHORTS_ST_RET_SZ 3
/* Get BIST Self-Test Results */
#define CY_CMD_CAT_GET_BIST_ST_RES_CMD_SZ   6
#define CY_CMD_CAT_GET_SELFTEST_RES_RET_SZ  9
/* Get Shorts Self-Test Results */
#define CY_CMD_CAT_GET_SHORTS_ST_RES_CMD_SZ 6
#define CY_CMD_CAT_GET_SHORTS_ST_RES_RET_SZ 6 /* + Data */
/* Get Opens Self-Test Results */
#define CY_CMD_CAT_GET_OPENS_ST_RES_CMD_SZ  6
#define CY_CMD_CAT_GET_OPENS_ST_RES_RET_SZ  5 /* + Data */
/* Get Capacitance Self-Test Results */
#define CY_CMD_CAT_GET_CAP_ST_RES_CMD_SZ    6
#define CY_CMD_CAT_GET_CAP_ST_RES_RET_SZ    5 /* + Data */
/* Get Auto Shorts Self-Test Results */
#define CY_CMD_CAT_GET_AUTOSHORTS_ST_RES_CMD_SZ 6
#define CY_CMD_CAT_GET_AUTOSHORTS_ST_RES_RET_SZ 6 /* + Data */
/* Calibrate IDACs */
#define CY_CMD_CAT_CALIBRATE_IDAC_CMD_SZ    2
#define CY_CMD_CAT_CALIBRATE_IDAC_RET_SZ    1
/* Initialize Baselines */
#define CY_CMD_CAT_INIT_BASELINE_CMD_SZ     2
#define CY_CMD_CAT_INIT_BASELINE_RET_SZ     1
/* Execute Panel Scan */
#define CY_CMD_CAT_EXECUTE_PANEL_SCAN_CMD_SZ    1
#define CY_CMD_CAT_EXECUTE_PANEL_SCAN_RET_SZ    1
/* Retrieve Panel Scan */
#define CY_CMD_CAT_RETRIEVE_PANEL_SCAN_CMD_SZ   6
#define CY_CMD_CAT_RETRIEVE_PANEL_SCAN_RET_SZ   5 /* + Data */
/* Start Sensor Data Mode */
#define CY_CMD_CAT_START_SENSOR_MODE_CMD_SZ 1 /* + Data */
#define CY_CMD_CAT_START_SENSOR_MODE_RET_SZ 0 /* + Data */
/* Stop Sensor Data Mode */
#define CY_CMD_CAT_STOP_SENSOR_MODE_CMD_SZ  1
#define CY_CMD_CAT_STOP_SENSOR_MODE_RET_SZ  0
/* Interrupt Pin Override */
#define CY_CMD_CAT_INT_PIN_OVERRIDE_CMD_SZ  2
#define CY_CMD_CAT_INT_PIN_OVERRIDE_RET_SZ  1
/* Retrieve Data Structure */
#define CY_CMD_CAT_RETRIEVE_DATA_STRUCT_CMD_SZ  6
#define CY_CMD_CAT_RETRIEVE_DATA_STRUCT_RET_SZ  5 /* + Data */
/* Verify Config Block CRC */
#define CY_CMD_CAT_VERIFY_CFG_BLK_CRC_CMD_SZ    2
#define CY_CMD_CAT_VERIFY_CFG_BLK_CRC_RET_SZ    5

/* CAT MODE REGISTERS */
#define CY_REG_CAT_CMD          2
#define CY_REG_CAT_CMD_DATA_MAX 511
#define CY_CMD_COMPLETE_MASK    (1 << 6)
#define CY_CMD_MASK             0x3F

/* Touch record registers */
enum cyttsp6_object_id {
    CY_OBJ_STANDARD_FINGER,
    CY_OBJ_PROXIMITY,
    CY_OBJ_STYLUS,
    CY_OBJ_HOVER,
    CY_OBJ_GLOVE,
};

enum cyttsp6_event_id {
    CY_EV_NO_EVENT,
    CY_EV_TOUCHDOWN,
    CY_EV_MOVE,     /* significant displacement (> act dist) */
    CY_EV_LIFTOFF,  /* record reports last position */
};

enum cyttsp6_startup_state {
    STARTUP_NONE,
    STARTUP_QUEUED,
    STARTUP_RUNNING,
};

enum cyttsp_cmd_bits {
    CY_CMD_COMPLETE = (1 << 6),
};

enum cyttsp6_cmd_op {
    CY_CMD_OP_NULL,
    CY_CMD_OP_RESERVED_1,
    CY_CMD_OP_GET_PARAM,
    CY_CMD_OP_SET_PARAM,
    CY_CMD_OP_RESERVED_2,
    CY_CMD_OP_GET_CRC,
    CY_CMD_OP_WAIT_FOR_EVENT,
};

/* DRIVER STATES */
enum cyttsp6_mode {
    CY_MODE_UNKNOWN      = 0,
    CY_MODE_BOOTLOADER   = (1 << 0),
    CY_MODE_OPERATIONAL  = (1 << 1),
    CY_MODE_SYSINFO      = (1 << 2),
    CY_MODE_CAT          = (1 << 3),
    CY_MODE_STARTUP      = (1 << 4),
    CY_MODE_LOADER       = (1 << 5),
    CY_MODE_CHANGE_MODE  = (1 << 6),
    CY_MODE_CHANGED      = (1 << 7),
    CY_MODE_CMD_COMPLETE = (1 << 8),
};

enum cyttsp6_int_state {
    CY_INT_NONE        = 0,
    CY_INT_IGNORE      = (1 << 0),
    CY_INT_MODE_CHANGE = (1 << 1),
    CY_INT_EXEC_CMD    = (1 << 2),
    CY_INT_AWAKE       = (1 << 3),
};

/* CMD */
enum cyttsp6_cmd_bl {
    CY_CMD_LDR_VERIFY_CHKSUM = 0x31,
    CY_CMD_LDR_ERASE_ROW = 0x34,
    CY_CMD_LDR_SEND_DATA = 0x37,
    CY_CMD_LDR_ENTER,
    CY_CMD_LDR_PROG_ROW,
    CY_CMD_LDR_VERIFY_ROW,
    CY_CMD_LDR_EXIT,
    CY_CMD_LDR_FAST_EXIT,
    CY_CMD_LDR_INIT = 0x48,
};

enum cyttsp6_hst_mode_bits {
    CY_HST_TOGGLE      = (1 << 7),
    CY_HST_MODE_CHANGE = (1 << 3),
    CY_HST_DEVICE_MODE = (7 << 4),
    CY_HST_OPERATE     = (0 << 4),
    CY_HST_SYSINFO     = (1 << 4),
    CY_HST_CAT         = (2 << 4),
    CY_HST_LOWPOW      = (1 << 2),
    CY_HST_SLEEP       = (1 << 1),
    CY_HST_RESET       = (1 << 0),
};

enum cyttsp6_tch_abs {
    /* for ordering within the extracted touch data array */
    CY_TCH_X,   /* X */
    CY_TCH_Y,   /* Y */
    CY_TCH_P,   /* P (Z) */
    CY_TCH_T,   /* TOUCH ID */
    CY_TCH_E,   /* EVENT ID */
    CY_TCH_O,   /* OBJECT ID */
    CY_TCH_W,   /* SIZE */
    CY_TCH_MAJ, /* TOUCH_MAJOR */
    CY_TCH_MIN, /* TOUCH_MINOR */
    CY_TCH_OR,  /* ORIENTATION */
    CY_TCH_NUM_ABS
};

enum cyttsp6_cmd_cat {
    CY_CMD_CAT_NULL,
    CY_CMD_CAT_RESERVED_1,
    CY_CMD_CAT_GET_CFG_ROW_SZ,
    CY_CMD_CAT_READ_CFG_BLK,
    CY_CMD_CAT_WRITE_CFG_BLK,
    CY_CMD_CAT_RESERVED_2,
    CY_CMD_CAT_LOAD_SELF_TEST_DATA,
    CY_CMD_CAT_RUN_SELF_TEST,
    CY_CMD_CAT_GET_SELF_TEST_RESULT,
    CY_CMD_CAT_CALIBRATE_IDACS,
    CY_CMD_CAT_INIT_BASELINES,
    CY_CMD_CAT_EXEC_PANEL_SCAN,
    CY_CMD_CAT_RETRIEVE_PANEL_SCAN,
    CY_CMD_CAT_START_SENSOR_DATA_MODE,
    CY_CMD_CAT_STOP_SENSOR_DATA_MODE,
    CY_CMD_CAT_INT_PIN_MODE,
    CY_CMD_CAT_RETRIEVE_DATA_STRUCTURE,
    CY_CMD_CAT_VERIFY_CFG_BLK_CRC,
    CY_CMD_CAT_RESERVED_N,
};

/* FLASH BLOCKS */
enum cyttsp6_ic_ebid {
    CY_TCH_PARM_EBID,
    CY_MDATA_EBID,
    CY_DDATA_EBID,
};

enum cyttsp6_cmd_status {
    CY_CMD_STATUS_SUCCESS,
    CY_CMD_STATUS_FAILURE,
};

struct cyttsp6_touch {
    int abs[CY_TCH_NUM_ABS];
};

struct cyttsp6_sysinfo_data {
    uint8_t hst_mode;
    uint8_t reserved;
    uint8_t map_szh;
    uint8_t map_szl;
    uint8_t cydata_ofsh;
    uint8_t cydata_ofsl;
    uint8_t test_ofsh;
    uint8_t test_ofsl;
    uint8_t pcfg_ofsh;
    uint8_t pcfg_ofsl;
    uint8_t opcfg_ofsh;
    uint8_t opcfg_ofsl;
    uint8_t ddata_ofsh;
    uint8_t ddata_ofsl;
    uint8_t mdata_ofsh;
    uint8_t mdata_ofsl;
} __attribute__((packed));

/* TTSP System Information interface definitions */
struct cyttsp6_cydata {
    uint8_t ttpidh;
    uint8_t ttpidl;
    uint8_t fw_ver_major;
    uint8_t fw_ver_minor;
    uint8_t revctrl[CY_NUM_REVCTRL];
    uint8_t blver_major;
    uint8_t blver_minor;
    uint8_t jtag_si_id3;
    uint8_t jtag_si_id2;
    uint8_t jtag_si_id1;
    uint8_t jtag_si_id0;
    uint8_t mfgid_sz;
    uint8_t cyito_idh;
    uint8_t cyito_idl;
    uint8_t cyito_verh;
    uint8_t cyito_verl;
    uint8_t ttsp_ver_major;
    uint8_t ttsp_ver_minor;
    uint8_t device_info;
    uint8_t mfg_id[];
} __attribute__((packed));

struct cyttsp6_test {
    uint8_t post_codeh;
    uint8_t post_codel;
} __attribute__((packed));

struct cyttsp6_pcfg {
    uint8_t electrodes_x;
    uint8_t electrodes_y;
    uint8_t len_xh;
    uint8_t len_xl;
    uint8_t len_yh;
    uint8_t len_yl;
    uint8_t res_xh;
    uint8_t res_xl;
    uint8_t res_yh;
    uint8_t res_yl;
    uint8_t max_zh;
    uint8_t max_zl;
    uint8_t panel_info0;
} __attribute__((packed));

struct cyttsp6_tch_rec_params {
    uint8_t loc;
    uint8_t size;
} __attribute__((packed));

struct cyttsp6_opcfg {
    uint8_t cmd_ofs;
    uint8_t rep_ofs;
    uint8_t rep_szh;
    uint8_t rep_szl;
    uint8_t num_btns;
    uint8_t tt_stat_ofs;
    uint8_t obj_cfg0;
    uint8_t max_tchs;
    uint8_t tch_rec_size;
    struct cyttsp6_tch_rec_params tch_rec_old[CY_NUM_TCH_FIELDS];
    uint8_t btn_rec_size;    /* btn record size (in bytes) */
    uint8_t btn_diff_ofs;    /* btn data loc ,diff counts, (Op-Mode byte ofs) */
    uint8_t btn_diff_size;   /* btn size of diff counts (in bits) */
    struct cyttsp6_tch_rec_params tch_rec_new[CY_NUM_EXT_TCH_FIELDS];
    uint8_t noise_data_ofs;
    uint8_t noise_data_sz;
} __attribute__((packed));

struct cyttsp6_sysinfo_ptr {
    struct cyttsp6_cydata *cydata;
    struct cyttsp6_test *test;
    struct cyttsp6_pcfg *pcfg;
    struct cyttsp6_opcfg *opcfg;
    struct cyttsp6_ddata *ddata;
    struct cyttsp6_mdata *mdata;
};

struct cyttsp6_tch_abs_params {
    uint32_t ofs;    /* abs byte offset */
    uint32_t size;   /* size in bytes */
    uint32_t max;    /* max value */
    uint32_t bofs;   /* bit offset */
};

struct cyttsp6_sysinfo_ofs {
    uint32_t chip_type;
    uint32_t cmd_ofs;
    uint32_t rep_ofs;
    uint32_t rep_sz;
    uint32_t num_btns;
    uint32_t num_btn_regs;   /* ceil(num_btns/4) */
    uint32_t tt_stat_ofs;
    uint32_t tch_rec_size;
    uint32_t obj_cfg0;
    uint32_t max_tchs;
    uint32_t mode_size;
    uint32_t data_size;
    uint32_t rep_hdr_size;
    uint32_t map_sz;
    uint32_t max_x;
    uint32_t x_origin;   /* left or right corner */
    uint32_t max_y;
    uint32_t y_origin;   /* upper or lower corner */
    uint32_t max_p;
    uint32_t len_x;
    uint32_t cydata_ofs;
    uint32_t test_ofs;
    uint32_t pcfg_ofs;
    uint32_t opcfg_ofs;
    uint32_t ddata_ofs;
    uint32_t mdata_ofs;
    uint32_t cydata_size;
    uint32_t test_size;
    uint32_t pcfg_size;
    uint32_t opcfg_size;
    uint32_t ddata_size;
    uint32_t mdata_size;
    uint32_t btn_keys_size;
    struct cyttsp6_tch_abs_params tch_abs[CY_TCH_NUM_ABS];
    uint32_t btn_rec_size;   /* btn record size (in bytes) */
    uint32_t btn_diff_ofs;   /* btn data loc ,diff counts, (Op-Mode byte ofs) */
    uint32_t btn_diff_size;  /* btn size of diff counts (in bits) */
    uint32_t noise_data_ofs;
    uint32_t noise_data_sz;
};

struct cyttsp6_ttconfig {
    uint16_t version;
    uint16_t length;
    uint16_t max_length;
    uint8_t  gesture_enable;
    uint16_t crc;
};

struct cyttsp6_btn {
    bool enabled;
    int state;  /* CY_BTN_PRESSED, CY_BTN_RELEASED */
    int key_code;
};

struct cyttsp6_sysinfo {
    bool ready;
    struct cyttsp6_sysinfo_data si_data;
    struct cyttsp6_sysinfo_ptr si_ptrs;
    struct cyttsp6_sysinfo_ofs si_ofs;
    struct cyttsp6_ttconfig ttconfig;
    //struct cyttsp6_btn *btn;  /* button states */
    //u8 *btn_rec_data;         /* button diff count data */
    uint8_t *xy_mode;                /* operational mode and status regs */
    uint8_t *xy_data;                /* operational touch regs */
};

/* Define vendor spcific touchscreen data */
struct cyttsp6_ts_data {
    touch_dev_t dev;
    uint16_t id;
    uint16_t version;
    uint16_t vendor;
    uint16_t instance;
    uint32_t dev_irq;
    void *i2c_handle;
    // const struct ts_board_config *conf;
    struct safe_ts_device *safe_ts_dev;
    int32_t cmd_toggle;
    int32_t int_status;
    enum cyttsp6_startup_state startup_state;
    enum cyttsp6_mode mode;
    bool invalid_touch_app;
    bool bl_fast_exit;
    osMutexId_t cy_i2c_mutex;
    osMutexId_t cy_sys_mutex;
    int32_t max_xfer_len;
    osEventFlagsId_t irq_event;
    osEventFlagsId_t reset_event;
    osEventFlagsId_t sysinfo_event;
    osEventFlagsId_t setmode_event;
    osEventFlagsId_t cmd_event;
    struct cyttsp6_sysinfo sysinfo;
    uint32_t active_refresh_cycle_ms;
    uint8_t rep_stat_counter;
    int32_t num_prv_rec; /* Number of previous touch records */

    touch_cb_chan_t cb[TOUCH_NOTICER_MAX_NUM];
};

#endif

/* end of file touch_cyttsp6.h */