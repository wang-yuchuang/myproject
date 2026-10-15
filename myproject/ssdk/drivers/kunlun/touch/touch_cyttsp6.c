/*
 * Copyright (c) 2021 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

/**
 * @file touch_cyttsp6.c
 *
 * Copyright (c) 2023 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdlib.h>
#include <assert.h>
#include <bits.h>
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <cmsis_os2.h>

#include <driver/i2c.h>
#include <driver/gpio.h>
#include <debug.h>
#include <driver/kunlun/touch/touch.h>
#include <driver/kunlun/touch/touch_vendor.h>
#include <driver/kunlun/touch/touch_cyttsp6.h>

/* =========================== marco define session ======================== */
#define dprintf(lev, fmt, ...)    ssdk_printf(lev, fmt, ##__VA_ARGS__)

/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/** i2c transfer structure */
typedef struct i2c_transfer {
    uint8_t reg;
    uint8_t val[MAX_XFER_LEN];
} __PACKED i2c_transfer_t;

/* =========================== function declare session ==================== */
static int32_t cyttsp6_init(touch_dev_t* dev);
static int32_t cyttsp6_deinit(touch_dev_t* dev);
// static int32_t cyttsp6_online_check(touch_dev_t* dev);
static int32_t cyttsp6_register_callback(touch_dev_t* dev
                                        , touch_event_notice cb
                                        , void* usrCtx);
// static int32_t cyttsp6_touch_control(touch_dev_t* dev
//                                     , touch_ctrl_type_enum_t type
//                                     , void* data
//                                     , int32_t* len);
static int32_t cyttsp6_register_read(touch_dev_t* dev
                                    , uint32_t reg
                                    , uint32_t* regVal);
static int32_t cyttsp6_register_write(touch_dev_t* dev
                                    , uint32_t reg
                                    , uint32_t regVal);
/* =========================== global value define session ================= */

/* =========================== static value define session ================= */
/** cyttsp6 touch device opts */
static touch_ops_t s_cyttsp6_ops = {
    .init = cyttsp6_init,
    .deinit = cyttsp6_deinit,
    .online_check = NULL,
    .register_callback = cyttsp6_register_callback,
    .device_control = NULL,
    .reg_read = cyttsp6_register_read,
    .reg_write = cyttsp6_register_write,
};

static int32_t cy_dbg_flag = 0;

static const char *cyttsp6_tch_abs_string[] = {
    [CY_TCH_X]  = "X",
    [CY_TCH_Y]  = "Y",
    [CY_TCH_P]  = "P",
    [CY_TCH_T]  = "T",
    [CY_TCH_E]  = "E",
    [CY_TCH_O]  = "O",
    [CY_TCH_W]  = "W",
    [CY_TCH_MAJ] = "MAJ",
    [CY_TCH_MIN] = "MIN",
    [CY_TCH_OR] = "OR",
    [CY_TCH_NUM_ABS] = "INVALID"
};

/*
static uint8_t security_key[] = {
    0xA5, 0x01, 0x02, 0x03, 0xFF, 0xFE, 0xFD, 0x5A
};
*/

static uint8_t ldr_exit[] = {
    CY_CMD_LDR_HOST_SYNC, CY_START_OF_PACKET, CY_CMD_LDR_EXIT, 0x00, 0x00,
    0x4F, 0x6D, CY_END_OF_PACKET
};

/*
static uint8_t ldr_fast_exit[] = {
    CY_CMD_LDR_HOST_SYNC, CY_START_OF_PACKET, CY_CMD_LDR_FAST_EXIT,
    0x00, 0x00, 0xC3, 0x68, CY_END_OF_PACKET
};
*/

static uint8_t ldr_err_app[] = {
    CY_START_OF_PACKET, 0x02, 0x00, 0x00, 0x55, 0xDD, CY_END_OF_PACKET
};

/* =========================== function implementation session ============= */
static inline int32_t mutex_acquire(osMutexId_t* mutex)
{
    if (mutex == NULL) {
        dprintf(ALWAYS, "parameter error for %s.\n", __func__);
        return -1;
    }

    return osMutexAcquire(*mutex, osWaitForever);
}

static inline int32_t mutex_release(osMutexId_t* mutex)
{
    if (mutex == NULL) {
        dprintf(ALWAYS, "parameter error for %s.\n", __func__);
        return -1;
    }

    return osMutexRelease(*mutex);
}

static inline int32_t is_crc_stat_failed(struct cyttsp6_sysinfo *si)
{
    /* Check if device POST config CRC test failed */
    if (!(si->si_ptrs.test->post_codel & CY_POST_CODEL_CFG_DATA_CRC_FAIL))
        return 1;

    return 0;
}

static inline uint32_t merge_bytes(uint8_t high, uint8_t low)
{
    return (high << 8) + low;
}

static inline int32_t cyttsp6_bits_2_bytes(int32_t nbits, size_t *max)
{
    *max = 1 << nbits;
    return (nbits + 7) / 8;
}

static int32_t cyttsp6_adap_read(struct cyttsp6_ts_data *cyttsp6, uint16_t reg,
                             void *buf, int32_t length)
{
    if (cyttsp6 == NULL || buf == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    int32_t ret = 0;
    uint16_t trans_len;
    uint16_t slave_addr = cyttsp6->dev.cfg_ptr->i2c_addr;
    uint8_t client_addr;
    uint8_t addr_lo;

    mutex_acquire(&cyttsp6->cy_i2c_mutex);
    while (length > 0) {
        client_addr = slave_addr | ((reg >> 8) & 0x1);
        addr_lo = reg & 0xFF;
        trans_len = min(length, cyttsp6->max_xfer_len);
        ret = i2c_write(cyttsp6->i2c_handle, client_addr, &addr_lo, 1);
        if (ret < 0) {
            dprintf(ALWAYS, "%s: transmit error, ret=%d\n", __func__, ret);
            break;
        }

        ret = i2c_read(cyttsp6->i2c_handle, client_addr, buf, trans_len);
        if (ret < 0) {
            dprintf(ALWAYS, "%s: read error, ret=%d\n", __func__, ret);
            return ret;
        }

        length -= trans_len;
        buf += trans_len;
        reg += trans_len;
    }
    mutex_release(&cyttsp6->cy_i2c_mutex);

    return ret;
}

static int32_t cyttsp6_adap_write(struct cyttsp6_ts_data *cyttsp6, uint16_t reg,
                              void *buf, int32_t length)
{
    if (cyttsp6 == NULL || buf == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s.\n", __func__);
        return -1;
    }

    int32_t ret = 0;
    uint16_t trans_len;
    uint16_t slave_addr = cyttsp6->dev.cfg_ptr->i2c_addr;
    uint8_t client_addr;
    uint8_t addr_lo;
    i2c_transfer_t i2c_trans = {
        .reg = reg & 0xFF,
    };

    mutex_acquire(&cyttsp6->cy_i2c_mutex);
    while (length > 0) {
        client_addr = slave_addr | ((reg >> 8) & 0x1);
        addr_lo = reg & 0xFF;
        trans_len = min(length, cyttsp6->max_xfer_len);

        i2c_trans.reg = addr_lo;
        memcpy(i2c_trans.val, buf, trans_len);
        ret = i2c_write(cyttsp6->i2c_handle, client_addr, (void *)&i2c_trans, trans_len + 1);
        if (ret < 0) {
            dprintf(ALWAYS, "%s: transmit error, ret=%d\n", __func__, ret);
            break;
        }

        length -= trans_len;
        buf += trans_len;
        reg += trans_len;
    }
    mutex_release(&cyttsp6->cy_i2c_mutex);

    return ret;
}

static char const *mode2str(int32_t mode)
{
    switch (mode) {
        case CY_MODE_UNKNOWN:
            return "unknown";

        case CY_MODE_BOOTLOADER:
            return "bootloader";

        case CY_MODE_OPERATIONAL:
            return "operational";

        case CY_MODE_SYSINFO:
            return "sysinfo";

        case CY_MODE_CAT:
            return "cat";

        case CY_MODE_STARTUP:
            return "startup";

        case CY_MODE_LOADER:
            return "loader";

        case CY_MODE_CHANGE_MODE:
            return "change_mode";

        case CY_MODE_CHANGED:
            return "changed";

        case CY_MODE_CMD_COMPLETE:
            return "cmd_complete";

        default:
            return "mode_failure";
    }
}

static void cyttsp6_irq_handler(struct gpio_dev *dev, int pin, void *arg)
{
    struct cyttsp6_ts_data *cyttsp6 = arg;
    // mask_gpio_interrupt(cyttsp6->dev_irq);
    gpio_int_enable(cyttsp6->dev.cfg_ptr->irq_pin.pin_dev
                    , cyttsp6->dev.cfg_ptr->irq_pin.pin_num
                    , false);
    // event_signal(&cyttsp6->irq_event, false);
    osEventFlagsSet(cyttsp6->irq_event, 1);
    // unmask_gpio_interrupt(cyttsp6->dev_irq);
    gpio_int_enable(cyttsp6->dev.cfg_ptr->irq_pin.pin_dev
                    , cyttsp6->dev.cfg_ptr->irq_pin.pin_num
                    , true);
    return ;
}

static void cyttsp6_parse_mode(uint8_t mode, enum cyttsp6_mode *cur_mode)
{
    switch (mode & CY_HST_DEVICE_MODE) {
        case CY_HST_OPERATE:
            *cur_mode = CY_MODE_OPERATIONAL;
            dprintf(CY_PRINT, "%s: operational\n", __func__);
            break;

        case CY_HST_CAT:
            *cur_mode = CY_MODE_CAT;
            dprintf(CY_PRINT, "%s: CaT\n", __func__);
            break;

        case CY_HST_SYSINFO:
            *cur_mode = CY_MODE_SYSINFO;
            dprintf(CY_PRINT, "%s: sysinfo\n", __func__);
            break;

        default:
            *cur_mode = CY_MODE_UNKNOWN;
            dprintf(CY_PRINT, "%s: unknown HST mode 0x%02X\n", __func__, mode);
            break;
    }
}

static int32_t cyttsp6_handshake(struct cyttsp6_ts_data *cyttsp6, uint8_t mode)
{
    uint8_t cmd = mode ^ CY_HST_TOGGLE;
    int32_t rc;

    if (mode & CY_HST_MODE_CHANGE) {
        dprintf(CY_PRINT, "%s: Host mode change bit set, NO handshake\n", __func__);
        return 0;
    }

    rc = cyttsp6_adap_write(cyttsp6, CY_REG_BASE, &cmd, sizeof(cmd));
    if (rc < 0)
        dprintf(ALWAYS, "%s: write fail on handshake rc=%d\n", __func__, rc);

    return rc;
}

static int32_t cyttsp6_load_status_and_touch_regs(struct cyttsp6_ts_data
        *cyttsp6, bool optimize)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t first_read_len = si->si_ofs.rep_hdr_size;;
    int32_t second_read_off;
    int32_t num_read_rec;
    uint8_t num_cur_rec;
    uint8_t hst_mode;
    uint8_t rep_len;
    uint8_t rep_stat;
    uint8_t tt_stat;
    int32_t rc;

    /* Read one touch record additionally */
    if (optimize)
        first_read_len += si->si_ofs.tch_rec_size;

    rc = cyttsp6_adap_read(cyttsp6, si->si_ofs.rep_ofs,
                           &si->xy_mode[si->si_ofs.rep_ofs], first_read_len);

    if (rc < 0) {
        dprintf(ALWAYS, "%s: reading touch record failed rc=%d\n", __func__, rc);
        return rc;
    }

    /* print xy mode */
    for (uint32_t i = 0; i < si->si_ofs.mode_size; i++)
        dprintf(CY_PRINT, "%s: xy_mode 0x%X\n", __func__, (si->xy_mode)[i]);

    hst_mode = si->xy_mode[CY_REG_BASE];
    rep_len = si->xy_mode[si->si_ofs.rep_ofs];
    rep_stat = si->xy_mode[si->si_ofs.rep_ofs + 1];
    tt_stat = si->xy_mode[si->si_ofs.tt_stat_ofs];
    dprintf(CY_PRINT, "%s: %s%02X %s%d %s%02X %s%02X\n", __func__, "hst_mode=",
            hst_mode, "rep_len=", rep_len, "rep_stat=", rep_stat, "tt_stat=", tt_stat);
#ifdef CY_PRINT_SHOW
    uint8_t rep_stat_counter;
    cyttsp6->rep_stat_counter = ((cyttsp6->rep_stat_counter + 1) & 0x3);
    rep_stat_counter = rep_stat >> 6;

    if (cyttsp6->rep_stat_counter != rep_stat_counter) {
        dprintf(CY_PRINT, "Possibly missed a touch record expected:%d device:%d\n",
                cyttsp6->rep_stat_counter, rep_stat_counter);
        cyttsp6->rep_stat_counter = rep_stat_counter;
    }

#endif
    num_cur_rec = GET_NUM_TOUCH_RECORDS(tt_stat);
    dprintf(CY_PRINT, "%s: num_cur_rec=%d\n", __func__, num_cur_rec);

    if (rep_len == 0 && num_cur_rec > 0) {
        dprintf(ALWAYS, "%s: report length error rep_len=%d num_rec=%d\n",
                __func__, rep_len, num_cur_rec);
        return -EIO;
    }

    if (num_cur_rec > si->si_ofs.max_tchs) {
        dprintf(ALWAYS, "%s: %s (n=%d c=%zu)\n", __func__,
                "too many tch; set to max tch", num_cur_rec, si->si_ofs.max_tchs);
        num_cur_rec = si->si_ofs.max_tchs;
    }

    num_read_rec = num_cur_rec;
    second_read_off = si->si_ofs.tt_stat_ofs + 1;

    if (optimize) {
        num_read_rec--;
        second_read_off += si->si_ofs.tch_rec_size;
    }

    if (num_read_rec <= 0)
        goto exit_print;

    rc = cyttsp6_adap_read(cyttsp6, second_read_off,
                           &si->xy_mode[second_read_off], num_read_rec * si->si_ofs.tch_rec_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: read fail on touch regs rc=%d\n", __func__, rc);
        return rc;
    }

exit_print:

    /* print xy data */
    for (uint32_t i = 0; i < num_cur_rec * si->si_ofs.tch_rec_size; i++)
        dprintf(CY_PRINT, "%s: xy_data 0x%x\n", __func__, (si->xy_data)[i]);

    return 0;
}

static void cyttsp6_get_touch_axis(struct cyttsp6_ts_data *cyttsp6,
                                   int32_t *axis, int32_t size, int32_t max, uint8_t *xy_data, int32_t bofs)
{
    int32_t nbyte;
    int32_t next = 0;

    for (nbyte = 0, *axis = 0; nbyte < size; nbyte++) {
        dprintf(CY_PRINT,
                "%s: *axis=%02X(%d) size=%d max=%08X xy_data=%p xy_data[%d]=%02X(%d) bofs=%d\n",
                __func__, *axis, *axis, size, max, xy_data, next, xy_data[next], xy_data[next], bofs);
        *axis = (*axis << 8) + (xy_data[next] >> bofs);
        next++;
    }

    *axis &= max - 1;
    dprintf(CY_PRINT,
            "%s: *axis=%02X(%d) size=%d max=%08X xy_data=%p xy_data[%d]=%02X(%d)\n",
            __func__, *axis, *axis, size, max, xy_data, next, xy_data[next], xy_data[next]);
}

/*
 * Fills touch info for a touch record specified by rec_no
 * Should only be called in Operational mode IRQ attention and
 * rec_no should be less than the number of current touch records
 */
void cyttsp6_get_touch_record(struct cyttsp6_ts_data *cyttsp6, int32_t rec_no, int32_t *rec_abs)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    uint8_t *xy_data = si->xy_data + (rec_no * si->si_ofs.tch_rec_size);
    enum cyttsp6_tch_abs abs;
    memset(rec_abs, 0, CY_TCH_NUM_ABS * sizeof(int32_t));

    for (abs = CY_TCH_X; abs < CY_TCH_NUM_ABS; abs++) {
        uint32_t ofs = si->si_ofs.tch_abs[abs].ofs;
        uint32_t size = si->si_ofs.tch_abs[abs].size;
        uint32_t max = si->si_ofs.tch_abs[abs].max;
        uint32_t bofs = si->si_ofs.tch_abs[abs].bofs;
        cyttsp6_get_touch_axis(cyttsp6, &rec_abs[abs], size, max, xy_data + ofs, bofs);
        dprintf(CY_PRINT, "%s: get %s=%04X(%d)\n", __func__,
                cyttsp6_tch_abs_string[abs], rec_abs[abs], rec_abs[abs]);
    }
}

static void cyttsp6_get_mt_touches(struct cyttsp6_ts_data *cyttsp6, int32_t num_cur_rec)
{
    // int32_t len;
    // struct touch_report_data report_data;
    ts_coord_info_t report_data;
    touch_notice_t  notice_data;
    struct cyttsp6_touch tch;
    report_data.key_value = 0;
    report_data.touch_num = num_cur_rec;

    for (int32_t i = 0; i < num_cur_rec; i++) {
        cyttsp6_get_touch_record(cyttsp6, i, tch.abs);

        /* Discard proximity event */
        if (tch.abs[CY_TCH_O] == CY_OBJ_PROXIMITY) {
            dprintf(ALWAYS, "%s: Discarding proximity event\n", __func__);
            continue;
        }

        /* Validate track_id */
        if (tch.abs[CY_TCH_T] > cyttsp6->dev.cfg_ptr->coord_config.max_touch_num) {
            dprintf(ALWAYS, "%s: tch=%d -> bad trk_id=%d\n", __func__, i,
                    tch.abs[CY_TCH_T]);
            continue;
        }

        /* Lift-off */
        if (tch.abs[CY_TCH_E] == CY_EV_LIFTOFF) {
            dprintf(CY_PRINT, "%s: t=%d e=%d lift-off\n", __func__, tch.abs[CY_TCH_T],
                    tch.abs[CY_TCH_E]);
            continue;
        }

        report_data.point[i].id = tch.abs[CY_TCH_T];
        report_data.point[i].x = tch.abs[CY_TCH_X];
        report_data.point[i].y = tch.abs[CY_TCH_Y];
        report_data.point[i].w = tch.abs[CY_TCH_W];

        if (cy_dbg_flag) {
            dprintf(ALWAYS, "%s: [%d] id=%u (x=%u, y=%u)\n", __func__,
                    cyttsp6->instance, report_data.point[i].id,
                    report_data.point[i].x, report_data.point[i].y);
        }
    }

    // invoke callback four notice touch point
    notice_data.evt_type = TS_NOTICE_EVT_COORD;
    memcpy(&notice_data.coord, &report_data, sizeof(report_data));
    for (int32_t i = 0; i < TOUCH_NOTICER_MAX_NUM; i++) {
        if (!cyttsp6->cb[i].is_valid &&
            cyttsp6->cb[i].cb != NULL) {
            cyttsp6->cb[i].cb(cyttsp6->dev.cfg_ptr->screen_id
                              , &notice_data
                              , cyttsp6->cb[i].usrCtx);
        }
    }
    // len = 2 + num_cur_rec * TS_COORD_METADATA_SIZE;
    // safe_ts_report_data(cyttsp6->safe_ts_dev, &report_data, len);
}

/* read xy_data for all current touches */
static int32_t cyttsp6_xy_worker(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    uint8_t num_cur_rec;
    uint8_t rep_len;
    uint8_t rep_stat;
    uint8_t tt_stat;
    int32_t rc = 0;
    /*
     * Get event data from cyttsp6 device.
     * The event data includes all data
     * for all active touches.
     * Event data also includes button data
     */
    rep_len = si->xy_mode[si->si_ofs.rep_ofs];
    rep_stat = si->xy_mode[si->si_ofs.rep_ofs + 1];
    tt_stat = si->xy_mode[si->si_ofs.tt_stat_ofs];
    num_cur_rec = GET_NUM_TOUCH_RECORDS(tt_stat);

    if (rep_len == 0 && num_cur_rec > 0) {
        dprintf(ALWAYS, "%s: report length error rep_len=%d num_tch=%d\n",
                __func__, rep_len, num_cur_rec);
        goto cyttsp6_xy_worker_exit;
    }

    /* check any error conditions */
    if (IS_BAD_PKT(rep_stat)) {
        dprintf(ALWAYS, "%s: Invalid buffer detected\n", __func__);
        goto cyttsp6_xy_worker_exit;
    }

    if (IS_LARGE_AREA(tt_stat)) {
        dprintf(ALWAYS, "%s: Large area detected\n", __func__);
        goto cyttsp6_xy_worker_exit;
    }

    if (num_cur_rec > si->si_ofs.max_tchs) {
        dprintf(ALWAYS, "%s: %s (n=%d c=%zu)\n", __func__,
                "too many tch; set to max tch", num_cur_rec, si->si_ofs.max_tchs);
        num_cur_rec = si->si_ofs.max_tchs;
    }

    cyttsp6_get_mt_touches(cyttsp6, num_cur_rec);
cyttsp6_xy_worker_exit:
    return rc;
}

static void cyttsp6_ts_work_func(void *arg)
{
    struct cyttsp6_ts_data *cyttsp6 = arg;
    enum cyttsp6_mode cur_mode;
    uint8_t mode[3];
    int32_t rc;

    dprintf(ALWAYS, "entry %s.", __func__);
    while (1) {
        // event_wait(&cyttsp6->irq_event);
        osEventFlagsWait(cyttsp6->irq_event, 1, osFlagsWaitAny, osWaitForever);
        uint8_t cmd_ofs = cyttsp6->sysinfo.si_ofs.cmd_ofs;
        bool command_complete = false;
        mutex_acquire(&cyttsp6->cy_sys_mutex);
        rc = cyttsp6_adap_read(cyttsp6, CY_REG_BASE, mode, sizeof(mode));
        if (rc < 0) {
            dprintf(ALWAYS, "%s: fail read adapter ret=%d\n", __func__, rc);
            goto cyttsp6_irq_exit;
        }

        dprintf(CY_PRINT, "%s mode[0-2]:0x%X 0x%X 0x%X\n", __func__,
                mode[0], mode[1], mode[2]);

        if (IS_BOOTLOADER(mode[0], mode[1])) {
            cur_mode = CY_MODE_BOOTLOADER;
            dprintf(CY_PRINT, "%s: bl running\n", __func__);

            /* switch to bootloader */
            if (cyttsp6->mode != CY_MODE_BOOTLOADER)
                dprintf(CY_PRINT, "%s: restart switch bl m=%s -> m=%s\n", __func__,
                        mode2str(cyttsp6->mode), mode2str(cur_mode));

            cyttsp6->mode = cur_mode;
            // event_signal(&cyttsp6->reset_event, false);
            osEventFlagsSet(cyttsp6->reset_event, 1);
            goto cyttsp6_irq_exit;
        }

        cyttsp6_parse_mode(mode[0], &cur_mode);
#if 0
        /* Check whether this IRQ should be ignored (internal) */
        if (cyttsp6->int_status & CY_INT_IGNORE) {
            dprintf(ALWAYS, "%s: CY_INT_IGNORE\n", __func__);

            if (IS_DEEP_SLEEP_CONFIGURED(cd->easy_wakeup_gesture)) {
                /* Put device back to sleep on premature wakeup */
                dev_err(dev, "%s: Put device back to sleep\n",
                        __func__);
                _cyttsp6_put_device_into_deep_sleep(cd, mode[0]);
                goto cyttsp6_irq_exit;
            }

            /* Check for Wait for Event command */
            if (IS_TTSP_VER_GE(&cd->sysinfo, 2, 5)) {
                uint8_t cmd_code = mode[cmd_ofs] & CY_CMD_MASK;

                if (cmd_code == CY_CMD_OP_WAIT_FOR_EVENT
                        && mode[cmd_ofs] & CY_CMD_COMPLETE) {
                    cd->wake_initiated_by_device = 1;
                    dev_err(dev, "%s: Wake by device\n", __func__);
                    call_atten_cb(cd, CY_ATTEN_WAKE, 0);
                    goto cyttsp6_irq_handshake;
                }
            }
        }

        /* Check for wake up interrupt */
        if (cd->int_status & CY_INT_AWAKE) {
            cd->int_status &= ~CY_INT_AWAKE;
            wake_up(&cd->wait_q);
            dev_err(dev, "%s: Received wake up interrupt\n", __func__);
            goto cyttsp6_irq_handshake;
        }
#endif

        /* Expecting mode change interrupt */
        if ((cyttsp6->int_status & CY_INT_MODE_CHANGE)
                && (mode[0] & CY_HST_MODE_CHANGE) == 0) {
            cyttsp6->int_status &= ~CY_INT_MODE_CHANGE;
            dprintf(CY_PRINT, "%s: finish mode switch m=%s -> m=%s\n", __func__,
                    mode2str(cyttsp6->mode), mode2str(cur_mode));
            cyttsp6->mode = cur_mode;

            if (cyttsp6->mode == CY_MODE_SYSINFO)
                // event_signal(&cyttsp6->sysinfo_event, false);
                osEventFlagsSet(cyttsp6->sysinfo_event, 1);
            else
                osEventFlagsSet(cyttsp6->setmode_event, 1);
                // event_signal(&cyttsp6->setmode_event, false);

            goto cyttsp6_irq_handshake;
        }

        dprintf(CY_PRINT, "%s: cd->mode=%s cur_mode=%s\n", __func__,
                mode2str(cyttsp6->mode), mode2str(cur_mode));

        /* compare current core mode to current device mode */
        if ((mode[0] & CY_HST_MODE_CHANGE) == 0 && cyttsp6->mode != cur_mode) {
            /* Unexpected mode change occurred */
            dprintf(CY_PRINT, "%s %s->%s int32_t:%d\n", __func__, mode2str(cyttsp6->mode),
                    mode2str(cur_mode), cyttsp6->int_status);
            goto cyttsp6_irq_exit;
        }

        /* Expecting command complete interrupt */
        dprintf(CY_PRINT, "%s: command byte:0x%x, toggle:0x%x\n", __func__,
                mode[cmd_ofs], cyttsp6->cmd_toggle);

        if ((cyttsp6->int_status & CY_INT_EXEC_CMD) && mode[cmd_ofs] & CY_CMD_COMPLETE) {
            command_complete = true;
            cyttsp6->int_status &= ~CY_INT_EXEC_CMD;
            // event_signal(&cyttsp6->cmd_event, false);
            osEventFlagsSet(cyttsp6->cmd_event, 1);
            dprintf(CY_PRINT, "%s: Received command complete interrupt\n", __func__);
            /*
             * It is possible to receive a single interrupt for
             * command complete and touch/button status report.
             * Continue processing for a possible status report.
             */
        }

        if (!cyttsp6->sysinfo.xy_mode) {
            dprintf(CY_PRINT, "%s: xy_mode enter\n", __func__);
            goto cyttsp6_irq_handshake;
        }

        /* Copy the mode registers */
        memcpy(cyttsp6->sysinfo.xy_mode, mode, sizeof(mode));

        /* This should be status report, read status and touch regs */
        if (cyttsp6->mode == CY_MODE_OPERATIONAL
                && cyttsp6->startup_state != STARTUP_RUNNING) {
            dprintf(CY_PRINT, "%s: Read status and touch registers\n", __func__);

            rc = cyttsp6_load_status_and_touch_regs(cyttsp6, !command_complete);
            if (rc < 0)
                dprintf(ALWAYS, "%s: fail read mode/touch regs rc=%d\n", __func__, rc);

            /* attention IRQ */
            //call_atten_cb(cyttsp6, CY_ATTEN_IRQ, cyttsp6->mode);
            cyttsp6_xy_worker(cyttsp6);
        }

cyttsp6_irq_handshake:
        /* handshake the event */
        dprintf(CY_PRINT, "%s: Handshake mode=0x%02X rc=%d\n", __func__, mode[0], rc);
        rc = cyttsp6_handshake(cyttsp6, mode[0]);
        if (rc < 0)
            dprintf(ALWAYS, "%s: Fail handshake mode=0x%02X rc=%d\n", __func__, mode[0], rc);

        /*
         * a non-zero udelay period is required for using
         * IRQF_TRIGGER_LOW in order to delay until the
         * device completes isr deassert
         */
        //udelay(cd->cpdata->level_irq_udelay);
cyttsp6_irq_exit:
        dprintf(CY_PRINT, "%s: irq event done\n\n", __func__);
        mutex_release(&cyttsp6->cy_sys_mutex);
    }

    osThreadExit();
}

static int32_t cyttsp6_set_inited(void *arg)
{
    //struct cyttsp6_ts_data *cyttsp6 = arg;
    return 0;
}

static int32_t cyttsp6_hw_reset(struct cyttsp6_ts_data *cyttsp6)
{
    if (cyttsp6->dev.cfg_ptr->serdes_config.enable) {
        serdes_gpio_ctrl_t gpio_ctrl;
        uint8_t len = sizeof(gpio_ctrl);
        gpio_ctrl.io_num = cyttsp6->dev.cfg_ptr->serdes_config.reset_pin;
        gpio_ctrl.dir = 0;
        gpio_ctrl.val = 1;
        serdes_control(*cyttsp6->dev.cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);
        osDelay(40);

        gpio_ctrl.val = 0;
        serdes_control(*cyttsp6->dev.cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);
        osDelay(40);

        gpio_ctrl.val = 1;
        serdes_control(*cyttsp6->dev.cfg_ptr->serdes_config.serdes
                            , DES_CONTROL_GPIO_CTRL
                            , &gpio_ctrl
                            , &len);
        osDelay(40);

        serdes_port_passthrough_ctl_t port_ctl;
        port_ctl.dir = 1;   /**< des -> ser */
        port_ctl.d_port = cyttsp6->dev.cfg_ptr->serdes_config.irq_pin;
        port_ctl.s_port = cyttsp6->dev.cfg_ptr->serdes_config.sirq_pin;
        uint8_t ctl_len = sizeof(port_ctl);
        serdes_control(*cyttsp6->dev.cfg_ptr->serdes_config.serdes
                        , SERDES_PORT_PASSTHROUGH_CTL
                        , &port_ctl
                        , &ctl_len);
    }

    return 0;
}

static int32_t cyttsp6_reset_and_wait(struct cyttsp6_ts_data *cyttsp6)
{
    int32_t ret = 0;
    /* reset hardware */
    mutex_acquire(&cyttsp6->cy_sys_mutex);
    ret = cyttsp6_hw_reset(cyttsp6);
    cyttsp6->mode = CY_MODE_UNKNOWN;
    mutex_release(&cyttsp6->cy_sys_mutex);

    if (ret < 0) {
        dprintf(ALWAYS, "Fail hw reset\n");
        return ret;
    }

    // return event_wait_timeout(&cyttsp6->reset_event, CY_CORE_RESET_AND_WAIT_TIMEOUT);
    return osEventFlagsWait(cyttsp6->reset_event, 1, osFlagsWaitAny, CY_CORE_RESET_AND_WAIT_TIMEOUT);
}

static int32_t exit_bootloader(struct cyttsp6_ts_data *cyttsp6)
{
    int32_t rc;
    cyttsp6->int_status &= ~CY_INT_IGNORE;
    cyttsp6->int_status |= CY_INT_MODE_CHANGE;

    rc = cyttsp6_adap_write(cyttsp6, CY_REG_BASE, ldr_exit, sizeof(ldr_exit));
    if (rc < 0)
        dprintf(ALWAYS, "%s: Fail to write rc=%d\n", __func__, rc);

    return rc;
}

static int32_t cyttsp6_wait_sysinfo_mode(struct cyttsp6_ts_data *cyttsp6)
{
    int32_t ret;

    // ret = event_wait_timeout(&cyttsp6->sysinfo_event, CY_CORE_WAIT_SYSINFO_MODE_TIMEOUT);
    ret = osEventFlagsWait(cyttsp6->sysinfo_event, 1, osFlagsWaitAny, CY_CORE_WAIT_SYSINFO_MODE_TIMEOUT);
    if (ret < 0) {
        dprintf(ALWAYS, "%s: tmo wait exit bl cd->mode=%d\n", __func__, cyttsp6->mode);
        mutex_acquire(&cyttsp6->cy_sys_mutex);
        cyttsp6->int_status &= ~CY_INT_MODE_CHANGE;
        mutex_release(&cyttsp6->cy_sys_mutex);
    }

    return ret;
}

static int32_t cyttsp6_si_data_offsets(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t rc;

    rc = cyttsp6_adap_read(cyttsp6, CY_REG_BASE, &si->si_data, sizeof(si->si_data));
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail read sysinfo data offsets rc=%d\n", __func__, rc);
        return rc;
    }

    /* convert sysinfo data offset bytes into integers */
    si->si_ofs.map_sz = merge_bytes(si->si_data.map_szh, si->si_data.map_szl);
    si->si_ofs.cydata_ofs = merge_bytes(si->si_data.cydata_ofsh, si->si_data.cydata_ofsl);
    si->si_ofs.test_ofs = merge_bytes(si->si_data.test_ofsh, si->si_data.test_ofsl);
    si->si_ofs.pcfg_ofs = merge_bytes(si->si_data.pcfg_ofsh, si->si_data.pcfg_ofsl);
    si->si_ofs.opcfg_ofs = merge_bytes(si->si_data.opcfg_ofsh, si->si_data.opcfg_ofsl);
    si->si_ofs.ddata_ofs = merge_bytes(si->si_data.ddata_ofsh, si->si_data.ddata_ofsl);
    si->si_ofs.mdata_ofs = merge_bytes(si->si_data.mdata_ofsh, si->si_data.mdata_ofsl);

    for (uint32_t i = 0; i < sizeof(si->si_data); i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)(&si->si_data))[i]);

    dprintf(CY_PRINT, "%s: si->si_ofs.map_sz 0x%x\n", __func__, si->si_ofs.map_sz);
    dprintf(CY_PRINT, "%s: si->si_ofs.cydata_ofs 0x%x\n", __func__, si->si_ofs.cydata_ofs);
    dprintf(CY_PRINT, "%s: si->si_ofs.test_ofs 0x%x\n", __func__, si->si_ofs.test_ofs);
    dprintf(CY_PRINT, "%s: si->si_ofs.pcfg_ofs 0x%x\n", __func__, si->si_ofs.pcfg_ofs);
    dprintf(CY_PRINT, "%s: si->si_ofs.opcfg_ofs 0x%x\n", __func__, si->si_ofs.opcfg_ofs);
    dprintf(CY_PRINT, "%s: si->si_ofs.ddata_ofs 0x%x\n", __func__, si->si_ofs.ddata_ofs);
    dprintf(CY_PRINT, "%s: si->si_ofs.mdata_ofs 0x%x\n", __func__, si->si_ofs.mdata_ofs);
    return rc;
}

static int32_t cyttsp6_si_get_cydata(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t rc = 0;
    int32_t read_offset, read_size;
    int32_t mfgid_sz, calc_mfgid_sz;
    si->si_ofs.cydata_size = si->si_ofs.test_ofs - si->si_ofs.cydata_ofs;

    if (si->si_ofs.cydata_size <= 0) {
        rc = -EINVAL;
        goto err;
    }

    dprintf(CY_PRINT, "%s: cydata size: %u\n", __func__, si->si_ofs.cydata_size);

    if (si->si_ptrs.cydata)
        free(si->si_ptrs.cydata);

    si->si_ptrs.cydata = malloc(si->si_ofs.cydata_size);
    if (!si->si_ptrs.cydata) {
        rc = -EINVAL;
        goto err;
    }

    read_offset = si->si_ofs.cydata_ofs;
    read_size = cy_offsetof(struct cyttsp6_cydata, mfgid_sz)
                + sizeof(si->si_ptrs.cydata->mfgid_sz);

    /* Read the CYDA registers up to MFGID field */
    rc = cyttsp6_adap_read(cyttsp6, read_offset, si->si_ptrs.cydata, read_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail1 read cydata rc=%d\n", __func__, rc);
        goto err1;
    }

    /* Check MFGID size */
    mfgid_sz = si->si_ptrs.cydata->mfgid_sz;
    calc_mfgid_sz = si->si_ofs.cydata_size - sizeof(struct cyttsp6_cydata);

    if (mfgid_sz != calc_mfgid_sz) {
        dprintf(ALWAYS, "%s: mismatch in MFGID size, reported:%d calculated:%d\n",
                __func__, mfgid_sz, calc_mfgid_sz);
        rc = -EINVAL;
        goto err1;
    }

    read_offset += read_size;
    read_size = si->si_ptrs.cydata->mfgid_sz;

    /* Read the CYDA registers for MFGID field */
    rc = cyttsp6_adap_read(cyttsp6, read_offset, si->si_ptrs.cydata->mfg_id, read_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail2 read cydata rc=%d\n", __func__, rc);
        goto err1;
    }

    read_offset += read_size;
    read_size = sizeof(struct cyttsp6_cydata) - cy_offsetof(
                    struct cyttsp6_cydata, cyito_idh);
    /* Read the rest of the CYDA registers */
    rc = cyttsp6_adap_read(cyttsp6, read_offset,
                           &si->si_ptrs.cydata->cyito_idh, read_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail3 read cydata rc=%d\n", __func__, rc);
        goto err1;
    }

    for (uint32_t i = 0; i < si->si_ofs.cydata_size; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)si->si_ptrs.cydata)[i]);

    return rc;
err1:
    free(si->si_ptrs.cydata);
    si->si_ptrs.cydata = NULL;
err:
    return rc;
}

static int32_t cyttsp6_si_get_test_data(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t rc = 0;
    si->si_ofs.test_size = si->si_ofs.pcfg_ofs - si->si_ofs.test_ofs;

    if (si->si_ofs.test_size <= 0) {
        rc = -EINVAL;
        goto err;
    }

    if (si->si_ptrs.test)
        free(si->si_ptrs.test);

    si->si_ptrs.test = malloc(si->si_ofs.test_size);
    if (!si->si_ptrs.test) {
        rc = -EINVAL;
        goto err;
    }

    rc = cyttsp6_adap_read(cyttsp6, si->si_ofs.test_ofs, si->si_ptrs.test,
                           si->si_ofs.test_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail read test data rc=%d\n", __func__, rc);
        goto err1;
    }

    if (si->si_ptrs.test->post_codel & CY_POST_CODEL_WDG_RST)
        dprintf(CY_PRINT, "%s: Reset was a WATCHDOG RESET codel=%02X\n", __func__,
                si->si_ptrs.test->post_codel);

    if (is_crc_stat_failed(si))
        dprintf(CY_PRINT, "%s: Config Data CRC FAIL codel=%02X\n", __func__,
                si->si_ptrs.test->post_codel);

    if (!(si->si_ptrs.test->post_codel & CY_POST_CODEL_PANEL_TEST_FAIL))
        dprintf(CY_PRINT, "%s: PANEL TEST Status=0 codel=%02X\n", __func__,
                si->si_ptrs.test->post_codel);

    dprintf(CY_PRINT, "%s: SCANNING is %s codel=%02X\n", __func__,
            si->si_ptrs.test->post_codel & CY_POST_CODEL_SCAN_STATUS ? "ENABLED" :
            "DISABLED", si->si_ptrs.test->post_codel);

    for (uint32_t i = 0; i < si->si_ofs.test_size; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)si->si_ptrs.test)[i]);

    return rc;
err1:
    free(si->si_ptrs.test);
    si->si_ptrs.test = NULL;
err:
    return rc;
}

static int32_t cyttsp6_si_get_pcfg_data(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t rc = 0;
    si->si_ofs.pcfg_size = si->si_ofs.opcfg_ofs - si->si_ofs.pcfg_ofs;

    if (si->si_ofs.pcfg_size <= 0) {
        rc = -EINVAL;
        goto err;
    }

    if (si->si_ptrs.pcfg)
        free(si->si_ptrs.pcfg);

    si->si_ptrs.pcfg = malloc(si->si_ofs.pcfg_size);
    if (!si->si_ptrs.pcfg) {
        rc = -EINVAL;
        goto err;
    }

    rc = cyttsp6_adap_read(cyttsp6, si->si_ofs.pcfg_ofs, si->si_ptrs.pcfg,
                           si->si_ofs.pcfg_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail read pcfg data rc=%d\n", __func__, rc);
        goto err1;
    }

    si->si_ofs.max_x = merge_bytes((si->si_ptrs.pcfg->res_xh &
                                    CY_PCFG_RESOLUTION_X_MASK), si->si_ptrs.pcfg->res_xl);
    si->si_ofs.x_origin = !!(si->si_ptrs.pcfg->res_xh & CY_PCFG_ORIGIN_X_MASK);
    si->si_ofs.max_y = merge_bytes((si->si_ptrs.pcfg->res_yh &
                                    CY_PCFG_RESOLUTION_Y_MASK), si->si_ptrs.pcfg->res_yl);
    si->si_ofs.y_origin = !!(si->si_ptrs.pcfg->res_yh & CY_PCFG_ORIGIN_Y_MASK);
    si->si_ofs.max_p = merge_bytes(si->si_ptrs.pcfg->max_zh,
                                   si->si_ptrs.pcfg->max_zl);
    si->si_ofs.len_x = merge_bytes((si->si_ptrs.pcfg->len_xh &
                                    CY_PCFG_RESOLUTION_X_MASK), si->si_ptrs.pcfg->len_xl);

    for (uint32_t i = 0; i < si->si_ofs.pcfg_size; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)si->si_ptrs.pcfg)[i]);

    dprintf(CY_PRINT, "%s: si->si_ofs.max_x 0x%x\n", __func__, si->si_ofs.max_x);
    dprintf(CY_PRINT, "%s: si->si_ofs.x_origin 0x%x\n", __func__, si->si_ofs.x_origin);
    dprintf(CY_PRINT, "%s: si->si_ofs.max_y 0x%x\n", __func__, si->si_ofs.max_y);
    dprintf(CY_PRINT, "%s: si->si_ofs.y_origin 0x%x\n", __func__, si->si_ofs.y_origin);
    dprintf(CY_PRINT, "%s: si->si_ofs.max_p 0x%x\n", __func__, si->si_ofs.max_p);
    dprintf(CY_PRINT, "%s: si->si_ofs.len_x 0x%x\n", __func__, si->si_ofs.len_x);
    return rc;
err1:
    free(si->si_ptrs.pcfg);
    si->si_ptrs.pcfg = NULL;
err:
    return rc;
}

static int32_t cyttsp6_si_get_opcfg_data(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t i, rc = 0;
    enum cyttsp6_tch_abs abs;
    si->si_ofs.opcfg_size = si->si_ofs.ddata_ofs - si->si_ofs.opcfg_ofs;

    if (si->si_ofs.opcfg_size <= 0) {
        rc = -EINVAL;
        goto err;
    }

    if (si->si_ptrs.opcfg)
        free(si->si_ptrs.opcfg);

    si->si_ptrs.opcfg = malloc(si->si_ofs.opcfg_size);
    if (!si->si_ptrs.opcfg) {
        rc = -EINVAL;
        goto err;
    }

    rc = cyttsp6_adap_read(cyttsp6, si->si_ofs.opcfg_ofs, si->si_ptrs.opcfg,
                           si->si_ofs.opcfg_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail read opcfg data rc=%d\n", __func__, rc);
        goto err1;
    }

    si->si_ofs.cmd_ofs = si->si_ptrs.opcfg->cmd_ofs;
    si->si_ofs.rep_ofs = si->si_ptrs.opcfg->rep_ofs;
    si->si_ofs.rep_sz = (si->si_ptrs.opcfg->rep_szh * 256) + si->si_ptrs.opcfg->rep_szl;
    si->si_ofs.num_btns = si->si_ptrs.opcfg->num_btns;
    si->si_ofs.num_btn_regs = (si->si_ofs.num_btns + CY_NUM_BTN_PER_REG - 1) / CY_NUM_BTN_PER_REG;
    si->si_ofs.tt_stat_ofs = si->si_ptrs.opcfg->tt_stat_ofs;
    si->si_ofs.obj_cfg0 = si->si_ptrs.opcfg->obj_cfg0;
    si->si_ofs.max_tchs = si->si_ptrs.opcfg->max_tchs & CY_BYTE_OFS_MASK;
    si->si_ofs.tch_rec_size = si->si_ptrs.opcfg->tch_rec_size & CY_BYTE_OFS_MASK;

    /* Get the old touch fields */
    for (abs = CY_TCH_X; abs < CY_NUM_TCH_FIELDS; abs++) {
        si->si_ofs.tch_abs[abs].ofs = si->si_ptrs.opcfg->tch_rec_old[abs].loc & CY_BYTE_OFS_MASK;
        si->si_ofs.tch_abs[abs].size = cyttsp6_bits_2_bytes(
                                           si->si_ptrs.opcfg->tch_rec_old[abs].size, &si->si_ofs.tch_abs[abs].max);
        si->si_ofs.tch_abs[abs].bofs = (si->si_ptrs.opcfg->tch_rec_old[abs].loc & CY_BOFS_MASK) >> CY_BOFS_SHIFT;
    }

    /* button fields */
    si->si_ofs.btn_rec_size = si->si_ptrs.opcfg->btn_rec_size;
    si->si_ofs.btn_diff_ofs = si->si_ptrs.opcfg->btn_diff_ofs;
    si->si_ofs.btn_diff_size = si->si_ptrs.opcfg->btn_diff_size;

    if (IS_TTSP_VER_GE(si, 2, 3)) {
        /* Get the extended touch fields */
        for (i = 0; i < CY_NUM_EXT_TCH_FIELDS; abs++, i++) {
            si->si_ofs.tch_abs[abs].ofs = si->si_ptrs.opcfg->tch_rec_new[i].loc & CY_BYTE_OFS_MASK;
            si->si_ofs.tch_abs[abs].size = cyttsp6_bits_2_bytes(
                                               si->si_ptrs.opcfg->tch_rec_new[i].size, &si->si_ofs.tch_abs[abs].max);
            si->si_ofs.tch_abs[abs].bofs = (si->si_ptrs.opcfg->tch_rec_new[i].loc & CY_BOFS_MASK) >> CY_BOFS_SHIFT;
        }
    }

    if (IS_TTSP_VER_GE(si, 2, 4)) {
        si->si_ofs.noise_data_ofs = si->si_ptrs.opcfg->noise_data_ofs;
        si->si_ofs.noise_data_sz = si->si_ptrs.opcfg->noise_data_sz;
    }

    for (abs = 0; abs < CY_TCH_NUM_ABS; abs++) {
        dprintf(CY_PRINT, "%s: tch_rec_%s\n", __func__, cyttsp6_tch_abs_string[abs]);
        dprintf(CY_PRINT, "%s: ofs =%2zu\n", __func__, si->si_ofs.tch_abs[abs].ofs);
        dprintf(CY_PRINT, "%s: siz =%2zu\n", __func__, si->si_ofs.tch_abs[abs].size);
        dprintf(CY_PRINT, "%s: max =%2zu\n", __func__, si->si_ofs.tch_abs[abs].max);
        dprintf(CY_PRINT, "%s: bofs=%2zu\n", __func__, si->si_ofs.tch_abs[abs].bofs);
    }

    si->si_ofs.mode_size = si->si_ofs.tt_stat_ofs + 1;
    si->si_ofs.data_size = si->si_ofs.max_tchs * si->si_ptrs.opcfg->tch_rec_size;
    si->si_ofs.rep_hdr_size = si->si_ofs.mode_size - si->si_ofs.rep_ofs;

    for (uint32_t i = 0; i < si->si_ofs.opcfg_size; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)si->si_ptrs.opcfg)[i]);

    return rc;
err1:
    free(si->si_ptrs.opcfg);
    si->si_ptrs.opcfg = NULL;
err:
    return rc;
}

static int32_t cyttsp6_si_get_ddata(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t rc = 0;
    si->si_ofs.ddata_size = si->si_ofs.mdata_ofs - si->si_ofs.ddata_ofs;

    if (si->si_ofs.ddata_size <= 0) {
        rc =  -EINVAL;
        goto err;
    }

    if (si->si_ptrs.ddata)
        free(si->si_ptrs.ddata);

    si->si_ptrs.ddata = malloc(si->si_ofs.ddata_size);
    if (!si->si_ptrs.ddata) {
        rc =  -EINVAL;
        goto err;
    }

    rc = cyttsp6_adap_read(cyttsp6, si->si_ofs.ddata_ofs, si->si_ptrs.ddata,
                           si->si_ofs.ddata_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail read ddata data rc=%d\n", __func__, rc);
        goto err1;
    }

    for (uint32_t i = 0; i < si->si_ofs.ddata_size; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)si->si_ptrs.ddata)[i]);

    return rc;
err1:
    free(si->si_ptrs.ddata);
    si->si_ptrs.ddata = NULL;
err:
    return rc;
}

static int32_t cyttsp6_si_get_mdata(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    int32_t rc;
    si->si_ofs.mdata_size = si->si_ofs.map_sz - si->si_ofs.mdata_ofs;

    if (si->si_ofs.mdata_size <= 0) {
        rc =  -EINVAL;
        goto err;
    }

    if (si->si_ptrs.mdata)
        free(si->si_ptrs.mdata);

    si->si_ptrs.mdata = malloc(si->si_ofs.mdata_size);
    if (!si->si_ptrs.mdata) {
        rc =  -EINVAL;
        goto err;
    }

    rc = cyttsp6_adap_read(cyttsp6, si->si_ofs.mdata_ofs, si->si_ptrs.mdata,
                           si->si_ofs.mdata_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail read mdata data rc=%d\n", __func__, rc);
        goto err1;
    }

    for (uint32_t i = 0; i < si->si_ofs.mdata_size; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, ((uint8_t *)si->si_ptrs.mdata)[i]);

    return rc;
err1:
    free(si->si_ptrs.mdata);
    si->si_ptrs.mdata = NULL;
err:
    return rc;
}

static int32_t cyttsp6_si_get_op_data_ptrs(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;

    if (si->xy_mode)
        free(si->xy_mode);

    si->xy_mode = malloc(si->si_ofs.mode_size + si->si_ofs.data_size);
    if (!si->xy_mode) {
        return -ENOMEM;
    }

    si->xy_data = &si->xy_mode[si->si_ofs.tt_stat_ofs + 1];
    return 0;
}

static void cyttsp6_si_put_log_data(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    dprintf(CY_PRINT, "%s: cydata_ofs =%4zu siz=%4zu\n", __func__,
            si->si_ofs.cydata_ofs, si->si_ofs.cydata_size);
    dprintf(CY_PRINT, "%s: test_ofs   =%4zu siz=%4zu\n", __func__,
            si->si_ofs.test_ofs, si->si_ofs.test_size);
    dprintf(CY_PRINT, "%s: pcfg_ofs   =%4zu siz=%4zu\n", __func__,
            si->si_ofs.pcfg_ofs, si->si_ofs.pcfg_size);
    dprintf(CY_PRINT, "%s: opcfg_ofs  =%4zu siz=%4zu\n", __func__,
            si->si_ofs.opcfg_ofs, si->si_ofs.opcfg_size);
    dprintf(CY_PRINT, "%s: ddata_ofs  =%4zu siz=%4zu\n", __func__,
            si->si_ofs.ddata_ofs, si->si_ofs.ddata_size);
    dprintf(CY_PRINT, "%s: mdata_ofs  =%4zu siz=%4zu\n", __func__,
            si->si_ofs.mdata_ofs, si->si_ofs.mdata_size);
    dprintf(CY_PRINT, "%s: cmd_ofs       =%4zu\n", __func__,
            si->si_ofs.cmd_ofs);
    dprintf(CY_PRINT, "%s: rep_ofs       =%4zu\n", __func__,
            si->si_ofs.rep_ofs);
    dprintf(CY_PRINT, "%s: rep_sz        =%4zu\n", __func__,
            si->si_ofs.rep_sz);
    dprintf(CY_PRINT, "%s: num_btns      =%4zu\n", __func__,
            si->si_ofs.num_btns);
    dprintf(CY_PRINT, "%s: num_btn_regs  =%4zu\n", __func__,
            si->si_ofs.num_btn_regs);
    dprintf(CY_PRINT, "%s: tt_stat_ofs   =%4zu\n", __func__,
            si->si_ofs.tt_stat_ofs);
    dprintf(CY_PRINT, "%s: tch_rec_size   =%4zu\n", __func__,
            si->si_ofs.tch_rec_size);
    dprintf(CY_PRINT, "%s: max_tchs      =%4zu\n", __func__,
            si->si_ofs.max_tchs);
    dprintf(CY_PRINT, "%s: mode_size     =%4zu\n", __func__,
            si->si_ofs.mode_size);
    dprintf(CY_PRINT, "%s: data_size     =%4zu\n", __func__,
            si->si_ofs.data_size);
    dprintf(CY_PRINT, "%s: rep_hdr_size  =%4zu\n", __func__,
            si->si_ofs.rep_hdr_size);
    dprintf(CY_PRINT, "%s: map_sz        =%4zu\n", __func__,
            si->si_ofs.map_sz);
    dprintf(CY_PRINT, "%s: btn_rec_size   =%2zu\n", __func__,
            si->si_ofs.btn_rec_size);
    dprintf(CY_PRINT, "%s: btn_diff_ofs  =%2zu\n", __func__,
            si->si_ofs.btn_diff_ofs);
    dprintf(CY_PRINT, "%s: btn_diff_size  =%2zu\n", __func__,
            si->si_ofs.btn_diff_size);
    dprintf(CY_PRINT, "%s: max_x    = 0x%0zX (%zu)\n", __func__,
            si->si_ofs.max_x, si->si_ofs.max_x);
    dprintf(CY_PRINT, "%s: x_origin = %zu (%s)\n", __func__,
            si->si_ofs.x_origin,
            si->si_ofs.x_origin == CY_NORMAL_ORIGIN ?
            "left corner" : "right corner");
    dprintf(CY_PRINT, "%s: max_y    = 0x%04zX (%zu)\n", __func__,
            si->si_ofs.max_y, si->si_ofs.max_y);
    dprintf(CY_PRINT, "%s: y_origin = %zu (%s)\n", __func__,
            si->si_ofs.y_origin,
            si->si_ofs.y_origin == CY_NORMAL_ORIGIN ?
            "upper corner" : "lower corner");
    dprintf(CY_PRINT, "%s: max_p    = 0x%04zX (%zu)\n", __func__,
            si->si_ofs.max_p, si->si_ofs.max_p);
    dprintf(CY_PRINT, "%s: xy_mode=%p xy_data=%p\n", __func__,
            si->xy_mode, si->xy_data);
}

static int32_t cyttsp6_get_sysinfo_regs(struct cyttsp6_ts_data *cyttsp6)
{
    int32_t rc;
    rc = cyttsp6_si_data_offsets(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_cydata(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_test_data(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_pcfg_data(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_opcfg_data(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_ddata(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_mdata(cyttsp6);
    if (rc < 0)
        return rc;

    rc = cyttsp6_si_get_op_data_ptrs(cyttsp6);
    if (rc < 0) {
        return rc;
    }

    cyttsp6_si_put_log_data(cyttsp6);
    /* provide flow control handshake */
    rc = cyttsp6_handshake(cyttsp6, cyttsp6->sysinfo.si_data.hst_mode);
    if (rc < 0)
        dprintf(ALWAYS, "%s: handshake fail on sysinfo reg\n", __func__);

    mutex_acquire(&cyttsp6->cy_sys_mutex);
    cyttsp6->sysinfo.ready = true;
    mutex_release(&cyttsp6->cy_sys_mutex);
    return rc;
}

/*
 * returns err if refused or timeout; block until mode change complete
 * bit is set (mode change interrupt)
 */
static int32_t set_mode(struct cyttsp6_ts_data *cyttsp6, int32_t new_mode)
{
    uint8_t new_dev_mode;
    uint8_t mode;
    int32_t rc = 0;

    switch (new_mode) {
        case CY_MODE_OPERATIONAL:
            new_dev_mode = CY_HST_OPERATE;
            break;

        case CY_MODE_SYSINFO:
            new_dev_mode = CY_HST_SYSINFO;
            break;

        case CY_MODE_CAT:
            new_dev_mode = CY_HST_CAT;
            break;

        default:
            dprintf(ALWAYS, "%s: invalid mode: %02X(%d)\n", __func__, new_mode, new_mode);
            return -EINVAL;
    }

    /* change mode */
    dprintf(CY_PRINT, "%s: new_dev_mode=%02x new_mode=%s\n", __func__,
            new_dev_mode, mode2str(new_mode));
    mutex_acquire(&cyttsp6->cy_sys_mutex);
    rc = cyttsp6_adap_read(cyttsp6, CY_REG_BASE, &mode, sizeof(mode));
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Fail read mode rc=%d\n", __func__, rc);
        mutex_release(&cyttsp6->cy_sys_mutex);
        goto exit;
    }

    /* Clear device mode bits and set to new mode */
    mode &= ~CY_HST_DEVICE_MODE;
    mode |= new_dev_mode | CY_HST_MODE_CHANGE;
    cyttsp6->int_status |= CY_INT_MODE_CHANGE;
    rc = cyttsp6_adap_write(cyttsp6, CY_REG_BASE, &mode, sizeof(mode));
    mutex_release(&cyttsp6->cy_sys_mutex);

    if (rc < 0) {
        dprintf(ALWAYS, "%s: Fail write mode change rc=%d\n", __func__, rc);
        goto exit;
    }

    /* wait for mode change done interrupt */
    // rc = event_wait_timeout(&cyttsp6->setmode_event, CY_CORE_MODE_CHANGE_TIMEOUT);
    rc = osEventFlagsWait(cyttsp6->setmode_event, 1, osFlagsWaitAny, CY_CORE_MODE_CHANGE_TIMEOUT);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: tmo waiting mode change\n", __func__);
        mutex_acquire(&cyttsp6->cy_sys_mutex);
        cyttsp6->int_status &= ~CY_INT_MODE_CHANGE;
        mutex_release(&cyttsp6->cy_sys_mutex);
        goto exit;
    }

    dprintf(CY_PRINT, "%s: set mode back from wait cd->mode=%s\n", __func__,
            mode2str(cyttsp6->mode));
exit:
    return rc;
}

static int32_t cyttsp6_wait_cmd_exec(struct cyttsp6_ts_data *cyttsp6, int32_t timeout_ms)
{
    int32_t rc;
    // rc = event_wait_timeout(&cyttsp6->cmd_event, timeout_ms);
    rc = osEventFlagsWait(cyttsp6->cmd_event, 1, osFlagsWaitAny, timeout_ms);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Command execution time out\n", __func__);
        cyttsp6->int_status &= ~CY_INT_EXEC_CMD;
    }

    return rc;
}

static int32_t get_cmd_offs_and_cmd_data_len(struct cyttsp6_ts_data *cyttsp6,
        uint8_t mode, int32_t *cmd_ofs, int32_t *cmd_data_len)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;

    switch (mode) {
        case CY_MODE_CAT:
            *cmd_ofs = CY_REG_CAT_CMD;
            *cmd_data_len = CY_REG_CAT_CMD_DATA_MAX - CY_REG_CAT_CMD;
            break;

        case CY_MODE_OPERATIONAL:
            *cmd_ofs = si->si_ofs.cmd_ofs;
            *cmd_data_len = si->si_ofs.rep_ofs - si->si_ofs.cmd_ofs - 1;
            break;

        default:
            dprintf(ALWAYS, "%s: Unsupported mode %x for exec cmd\n", __func__, mode);
            return -EACCES;
    }

    return 0;
}

/*
 * Send command to device for CAT and OP modes
 * return negative value on error, 0 on success
 */
static int32_t _cyttsp6_exec_cmd(struct cyttsp6_ts_data *cyttsp6, uint8_t mode,
                             uint8_t *cmd_buf, size_t cmd_size)
{
    int32_t cmd_ofs;
    int32_t cmd_data_len;
    int32_t cmd_param_ofs;
    uint8_t command;
    uint8_t *cmd_param_buf;
    int32_t cmd_param_size;
    int32_t rc;

    if (mode != cyttsp6->mode) {
        dprintf(ALWAYS,
                "%s: attempt to exec cmd in missing mode (having %x while %x requested)\n",
                __func__, cyttsp6->mode, mode);
        return -EACCES;
    }

    rc = get_cmd_offs_and_cmd_data_len(cyttsp6, mode, &cmd_ofs, &cmd_data_len);
    if (rc < 0)
        return rc;

    cmd_param_ofs = cmd_ofs + 1;
    cmd_param_buf = cmd_buf + 1;
    cmd_param_size = cmd_size - 1;

    /* Check for parameter size overflow */
    if (cmd_param_size > cmd_data_len) {
        dprintf(ALWAYS, "%s: Parameter size overflow:%zu max:%d\n", __func__,
                cmd_param_size, cmd_data_len);
        return -EINVAL;
    }

    /* Check if complete is set, so write new command */
    rc = cyttsp6_adap_read(cyttsp6, cmd_ofs, &command, 1);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Error on read rc=%d\n", __func__, rc);
        return rc;
    }

    cyttsp6->cmd_toggle = GET_TOGGLE(command);
    cyttsp6->int_status |= CY_INT_EXEC_CMD;

    if ((command & CY_CMD_COMPLETE_MASK) == 0)
        return -EBUSY;

    /*
     * Write new command
     * Only update command bits 0:5
     * Clear command complete bit & toggle bit
     */
    cmd_buf[0] = cmd_buf[0] & CY_CMD_MASK;

    /* Write command parameters first */
    if (cmd_size > 1) {
        rc = cyttsp6_adap_write(cyttsp6, cmd_param_ofs, cmd_param_buf, cmd_param_size);
        if (rc < 0) {
            dprintf(ALWAYS, "%s: Error on write command parameters rc=%d\n", __func__, rc);
            return rc;
        }
    }

    /* Write the command */
    rc = cyttsp6_adap_write(cyttsp6, cmd_ofs, cmd_buf, 1);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Error on write command rc=%d\n", __func__, rc);
        return rc;
    }

    return 0;
}

static int32_t cyttsp6_exec_cmd(struct cyttsp6_ts_data *cyttsp6, uint8_t mode,
                            uint8_t *cmd_buf, size_t cmd_size, uint8_t *return_buf,
                            int32_t param_ret_size, int32_t timeout_ms)
{
    int32_t cmd_ofs;
    int32_t cmd_data_len;
    int32_t cmd_return_ofs;
    int32_t rc;
    mutex_acquire(&cyttsp6->cy_sys_mutex);
    rc = _cyttsp6_exec_cmd(cyttsp6, mode, cmd_buf, cmd_size);
    mutex_release(&cyttsp6->cy_sys_mutex);

    if (rc == -EBUSY) {
        rc = cyttsp6_wait_cmd_exec(cyttsp6, CY_COMMAND_COMPLETE_TIMEOUT);
        if (rc)
            return rc;

        mutex_acquire(&cyttsp6->cy_sys_mutex);
        rc = _cyttsp6_exec_cmd(cyttsp6, mode, cmd_buf, cmd_size);
        mutex_release(&cyttsp6->cy_sys_mutex);
    }

    if (rc < 0)
        return rc;

    if (timeout_ms == 0)
        return 0;

    /* Wait command to be completed */
    rc = cyttsp6_wait_cmd_exec(cyttsp6, timeout_ms);
    if (rc < 0)
        return rc;

    if (param_ret_size == 0 || return_buf == NULL)
        return 0;

    rc = get_cmd_offs_and_cmd_data_len(cyttsp6, mode, &cmd_ofs, &cmd_data_len);
    if (rc < 0)
        return rc;

    /* Check for return buffer size overflow */
    if (param_ret_size > cmd_data_len) {
        dprintf(ALWAYS, "%s: Return data size overflow:%zu max:%d\n", __func__,
                param_ret_size, cmd_data_len);
        return -EINVAL;
    }

    cmd_return_ofs = cmd_ofs + 1;
    rc = cyttsp6_adap_read(cyttsp6, cmd_return_ofs, return_buf, param_ret_size);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Error on read rc=%d\n", __func__, rc);
        return rc;
    }

    return 0;
}

static uint16_t cyttsp6_calc_app_crc(const uint8_t *data, int32_t size)
{
    int32_t i, j;
    uint16_t crc = 0xFFFF;

    for (i = 0; i < size; i++) {
        crc ^= ((uint16_t)data[i] << 8);

        for (j = 8; j > 0; j--)
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
    }

    return crc;
}

static int32_t cyttsp6_read_config_block(struct cyttsp6_ts_data *cyttsp6,
                                     uint8_t ebid, uint16_t row, uint8_t *data, uint16_t length)
{
    uint8_t command_buf[CY_CMD_CAT_READ_CFG_BLK_CMD_SZ];
    uint8_t *return_buf;
    int32_t return_buf_sz;
    uint16_t crc;
    int32_t rc;
    /* Allocate buffer for read config block command response
     * Header(5) + Data(length) + CRC(2)
     */
    return_buf_sz = CY_CMD_CAT_READ_CFG_BLK_RET_SZ + length;

    return_buf = malloc(return_buf_sz);
    if (!return_buf)
        return -ENOMEM;

    command_buf[0] = CY_CMD_CAT_READ_CFG_BLK;
    command_buf[1] = HI_BYTE(row);
    command_buf[2] = LO_BYTE(row);
    command_buf[3] = HI_BYTE(length);
    command_buf[4] = LO_BYTE(length);
    command_buf[5] = ebid;
    rc = cyttsp6_exec_cmd(cyttsp6, CY_MODE_CAT, command_buf,
                          CY_CMD_CAT_READ_CFG_BLK_CMD_SZ, return_buf, return_buf_sz,
                          CY_COMMAND_COMPLETE_TIMEOUT);
    if (rc) {
        dprintf(ALWAYS, "%s: Error executing command rc=%d\n", __func__, rc);
        goto free_buffer;
    }

    crc = cyttsp6_calc_app_crc(&return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ], length);

    /* Validate response */
    if (return_buf[0] != CY_CMD_STATUS_SUCCESS || return_buf[1] != ebid
            || return_buf[2] != HI_BYTE(length) || return_buf[3] != LO_BYTE(length)
            || return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ + length] != HI_BYTE(crc)
            || return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ + length + 1] != LO_BYTE(
                crc)) {
        dprintf(ALWAYS, "%s: Fail executing command\n", __func__);
        rc = -EINVAL;
        goto free_buffer;
    }

    memcpy(data, &return_buf[CY_CMD_CAT_READ_CFG_BLK_RET_HDR_SZ], length);

    for (uint32_t i = 0; i < length; i++)
        dprintf(CY_PRINT, "%s: 0x%x\n", __func__, data[i]);

free_buffer:
    free(return_buf);
    return rc;
}

static int32_t cyttsp6_get_config_length(struct cyttsp6_ts_data *cyttsp6,
                                     uint8_t ebid, uint16_t *length, uint16_t *max_length)
{
    uint8_t data[CY_CONFIG_LENGTH_INFO_SIZE];
    int32_t rc;
    rc = cyttsp6_read_config_block(cyttsp6, ebid, CY_CONFIG_LENGTH_INFO_OFFSET,
                                   data, sizeof(data));
    if (rc) {
        dprintf(ALWAYS, "%s: Error on read config block\n", __func__);
        goto exit;
    }

    *length = data[0] | data[1] << 8;
    *max_length = data[2] | data[3] << 8;
exit:
    return rc;
}

static int32_t cyttsp6_read_gesture_enable(struct cyttsp6_ts_data *cyttsp6,
                                       uint8_t *gesture_enable)
{
    *gesture_enable = 0;
    return 0;
}

static int32_t cyttsp6_get_config_block_crc(struct cyttsp6_ts_data *cyttsp6,
                                        uint8_t ebid, uint16_t *crc)
{
    uint8_t command_buf[CY_CMD_OP_GET_CFG_BLK_CRC_CMD_SZ];
    uint8_t return_buf[CY_CMD_OP_GET_CFG_BLK_CRC_RET_SZ];
    int32_t rc;
    command_buf[0] = CY_CMD_OP_GET_CRC;
    command_buf[1] = ebid;
    rc = cyttsp6_exec_cmd(cyttsp6, CY_MODE_OPERATIONAL, command_buf,
                          CY_CMD_OP_GET_CFG_BLK_CRC_CMD_SZ, return_buf,
                          CY_CMD_OP_GET_CFG_BLK_CRC_RET_SZ, CY_COMMAND_COMPLETE_TIMEOUT);
    if (rc) {
        dprintf(ALWAYS, "%s: Error executing command rc=%d\n", __func__, rc);
        goto exit;
    }

    /* Validate response */
    if (return_buf[0] != CY_CMD_STATUS_SUCCESS) {
        dprintf(ALWAYS, "%s: Fail executing command\n", __func__);
        rc = -EINVAL;
        goto exit;
    }

    //*crc = get_unaligned_be16(&return_buf[1]);
    *crc = return_buf[1] << 8 | return_buf[2];
exit:
    return rc;
}

static int32_t cyttsp6_get_ttconfig_info(struct cyttsp6_ts_data *cyttsp6)
{
    struct cyttsp6_sysinfo *si = &cyttsp6->sysinfo;
    uint16_t length = 0, max_length = 0;
    uint16_t version = 0;
    uint16_t crc = 0;
    int32_t rc, rc2;
    uint8_t gesture_enable;

    rc = set_mode(cyttsp6, CY_MODE_CAT);
    if (rc < 0) {
        dprintf(ALWAYS, "%s:fail set mode to CAT rc=%d\n", __func__, rc);
        return rc;
    }

    /* CY_ITO regs already has TTConfig version. */
    version = si->si_ptrs.cydata->cyito_verh << 8;
    version |= si->si_ptrs.cydata->cyito_verl;
    rc = cyttsp6_get_config_length(cyttsp6, CY_TCH_PARM_EBID, &length, &max_length);
    if (rc < 0) {
        dprintf(ALWAYS, "%s:fail get ttconfig properties rc=%d\n", __func__, rc);
        goto exit;
    }

    rc = cyttsp6_read_gesture_enable(cyttsp6, &gesture_enable);
    if (rc < 0) {
        dprintf(ALWAYS, "%s:fail get ttconfig gesture rc=%d\n", __func__, rc);
        goto exit;
    }

    rc = set_mode(cyttsp6, CY_MODE_OPERATIONAL);
    if (rc < 0) {
        dprintf(ALWAYS, "%s:fail set mode to Operational rc=%d\n", __func__, rc);
        return rc;
    }

    rc = cyttsp6_get_config_block_crc(cyttsp6, CY_TCH_PARM_EBID, &crc);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail get ttconfig crc rc=%d\n", __func__, rc);
        return rc;
    }

    mutex_acquire(&cyttsp6->cy_sys_mutex);
    si->ttconfig.version = version;
    si->ttconfig.length = length;
    si->ttconfig.max_length = max_length;
    si->ttconfig.crc = crc;
    si->ttconfig.gesture_enable = gesture_enable;
    mutex_release(&cyttsp6->cy_sys_mutex);

    dprintf(CY_PRINT,
            "%s:TT Config Version:%04X Length:%d Max Length:%d CRC:%04X\n", __func__,
            si->ttconfig.version, si->ttconfig.length, si->ttconfig.max_length,
            si->ttconfig.crc);
    return 0;
exit:
    rc2 = set_mode(cyttsp6, CY_MODE_OPERATIONAL);
    if (rc2 < 0)
        dprintf(ALWAYS, "%s: exit fail set mode to Operational rc=%d\n", __func__, rc2);

    return rc;
}

static int32_t cyttsp6_get_parameter(struct cyttsp6_ts_data *cyttsp6,
                                 uint8_t param_id, uint32_t *param_value)
{
    uint8_t command_buf[CY_CMD_OP_GET_PARAM_CMD_SZ];
    uint8_t return_buf[CY_CMD_OP_GET_PARAM_RET_SZ];
    uint8_t param_size;
    uint8_t *value_buf;
    int32_t rc;
    command_buf[0] = CY_CMD_OP_GET_PARAM;
    command_buf[1] = param_id;
    rc = cyttsp6_exec_cmd(cyttsp6, CY_MODE_OPERATIONAL, command_buf,
                          CY_CMD_OP_GET_PARAM_CMD_SZ, return_buf, CY_CMD_OP_GET_PARAM_RET_SZ,
                          CY_COMMAND_COMPLETE_TIMEOUT);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Unable to execute get parameter command\n", __func__);
        return rc;
    }

    if (return_buf[0] != param_id) {
        dprintf(ALWAYS, "%s: Fail to execute get parameter command\n", __func__);
        return -EIO;
    }

    param_size = return_buf[1];
    value_buf = &return_buf[2];
    *param_value = 0;

    while (param_size--)
        *param_value += *(value_buf++) << (8 * param_size);

    return 0;
}

static int32_t cyttsp6_get_active_refresh_cycle(struct cyttsp6_ts_data *cyttsp6)
{
    int32_t rc;
    uint32_t value;
    rc = cyttsp6_get_parameter(cyttsp6, CY_RAM_ID_REFRESH_INTERVAL, &value);
    if (!rc)
        cyttsp6->active_refresh_cycle_ms = (uint8_t)value;

    return rc;
}

static int32_t cyttsp6_startup(struct cyttsp6_ts_data *cyttsp6)
{
    int32_t rc = 0;
    mutex_acquire(&cyttsp6->cy_sys_mutex);
    cyttsp6->startup_state = STARTUP_RUNNING;
    mutex_release(&cyttsp6->cy_sys_mutex);

    /* hw reset */
    rc = cyttsp6_reset_and_wait(cyttsp6);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Fail hw reset rc=%d\n", __func__, rc);
        goto exit;
    }

    /* exit bootloader mode */
    mutex_acquire(&cyttsp6->cy_sys_mutex);
    rc = exit_bootloader(cyttsp6);
    mutex_release(&cyttsp6->cy_sys_mutex);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Fail exit bootloader rc=%d\n", __func__, rc);
        goto exit;
    }

    /* wait into sysinfo mode */
    rc = cyttsp6_wait_sysinfo_mode(cyttsp6);
    if (rc < 0) {
        uint8_t buf[sizeof(ldr_err_app)];
        int32_t rc1;
        /* Check for invalid/corrupted touch application */
        rc1 = cyttsp6_adap_read(cyttsp6, CY_REG_BASE, buf, sizeof(ldr_err_app));

        if (rc1) {
            dprintf(ALWAYS, "%s: Fail to read rc1=%d\n", __func__, rc1);
        }
        else if (!memcmp(buf, ldr_err_app, sizeof(ldr_err_app))) {
            dprintf(ALWAYS, "%s: Fail launching touch app\n", __func__);
            mutex_acquire(&cyttsp6->cy_sys_mutex);
            cyttsp6->invalid_touch_app = true;
            mutex_release(&cyttsp6->cy_sys_mutex);
        }

        goto exit;
    }

    mutex_acquire(&cyttsp6->cy_sys_mutex);
    cyttsp6->invalid_touch_app = false;
    mutex_release(&cyttsp6->cy_sys_mutex);

    /* read sysinfo data */
    rc = cyttsp6_get_sysinfo_regs(cyttsp6);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: Fail get sysinfo regs rc=%d\n", __func__, rc);
        goto exit;
    }

    dprintf(CY_PRINT, "TTSP Protocol Version is %u.%u\n",
            cyttsp6->sysinfo.si_ptrs.cydata->ttsp_ver_major,
            cyttsp6->sysinfo.si_ptrs.cydata->ttsp_ver_minor);

    rc = set_mode(cyttsp6, CY_MODE_OPERATIONAL);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail set mode to operational rc=%d\n", __func__, rc);
        goto exit;
    }

    rc = cyttsp6_get_ttconfig_info(cyttsp6);
    if (rc < 0) {
        dprintf(ALWAYS, "%s: fail to get ttconfig info rc=%d\n", __func__, rc);
        goto exit;
    }

    rc = cyttsp6_get_active_refresh_cycle(cyttsp6);
    if (rc < 0)
        dprintf(ALWAYS, "%s: fail get refresh cycle time rc=%d\n", __func__, rc);

exit:
    mutex_acquire(&cyttsp6->cy_sys_mutex);
    cyttsp6->startup_state = STARTUP_NONE;
    mutex_release(&cyttsp6->cy_sys_mutex);
    return rc;
}

static void cyttsp6_wdg_work_func(void *arg)
{
    int32_t rc = 0;
    struct cyttsp6_ts_data *cyttsp6 = arg;
    dprintf(ALWAYS, "entry %s.\n", __func__);
    while (1) {
        // thread_sleep(3000);
        osDelay(CONFIG_WDT_MONITOR_PERIOD);
        // dprintf(CY_PRINT, "%s: enter\n", __func__);
        uint8_t mode[2] = {0};

        rc = cyttsp6_adap_read(cyttsp6, CY_REG_BASE, &mode, sizeof(mode));
        if (rc) {
            dprintf(ALWAYS, "%s: fail access device rc=%d\n", __func__, rc);
            cyttsp6_startup(cyttsp6);
            continue;
        }

        if (IS_BOOTLOADER(mode[0], mode[1])) {
            dprintf(ALWAYS, "%s: device in bootloader mode\n", __func__);
            cyttsp6_startup(cyttsp6);
            continue;
        }
    }
    osThreadExit();
}

static int32_t cyttsp6_init(touch_dev_t* dev)
{
    if (dev == NULL || dev->priv == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s\n", __func__);
        return -1;
    }
    struct cyttsp6_ts_data *cyttsp6 = dev->priv;
    int32_t ret = 0;

    /* init system resource */
    cyttsp6->cy_sys_mutex = osMutexNew(NULL);
    cyttsp6->cy_i2c_mutex = osMutexNew(NULL);
    cyttsp6->max_xfer_len = MAX_XFER_LEN;

    cyttsp6->irq_event = osEventFlagsNew(NULL);
    cyttsp6->reset_event = osEventFlagsNew(NULL);
    cyttsp6->sysinfo_event = osEventFlagsNew(NULL);
    cyttsp6->setmode_event = osEventFlagsNew(NULL);
    cyttsp6->cmd_event = osEventFlagsNew(NULL);

    /* init callback channel */
    for (int32_t i = 0; i < TOUCH_NOTICER_MAX_NUM; i++) {
        cyttsp6->cb[i].is_valid = true;
    }

    /* init work thread */
    osThreadAttr_t tattr;
    memset(&tattr, 0x00, sizeof(tattr));

    tattr.name = "tp_thread";
    tattr.priority = osPriorityRealtime;
    tattr.stack_size = CONFIG_CYTTSP_DRIVER_STACK_SIZE;
    osThreadNew(cyttsp6_ts_work_func, cyttsp6, &tattr);

    /* init gpio config and irq */
    gpio_config(cyttsp6->dev.cfg_ptr->irq_pin.pin_dev
                , cyttsp6->dev.cfg_ptr->irq_pin.pin_num
                , GPIO_INTR_FALLING_EDGE);
    gpio_attach_irq(cyttsp6->dev.cfg_ptr->irq_pin.pin_dev
                    , cyttsp6->dev.cfg_ptr->irq_pin.pin_num
                    , cyttsp6_irq_handler
                    , cyttsp6);

    for (int32_t cnt = 0; cnt < 3; cnt++) {
        ret = cyttsp6_startup(cyttsp6);
        if (ret >= 0)
            break;
    }

    if (ret < 0)
        return -1;

    memset(&tattr, 0x00, sizeof(tattr));
    tattr.name = "wdg_thread";
    tattr.priority = osPriorityRealtime;
    tattr.stack_size = CONFIG_CYTTSP_DRIVER_STACK_SIZE;
    // osThreadId_t wdg_thread = osThreadNew(cyttsp6_wdg_work_func, cyttsp6, &tattr);
    osThreadNew(cyttsp6_wdg_work_func, cyttsp6, &tattr);

    return 0;
}

static int32_t cyttsp6_deinit(touch_dev_t* dev)
{

    //@TODO deinit is not implement for current driver
    return 0;
}

static int32_t cyttsp6_register_callback(touch_dev_t* dev
                                        , touch_event_notice cb
                                        , void* usrCtx)
{
    if (dev == NULL || dev->priv == NULL || cb == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s\n", __func__);
        return -1;
    }

    struct cyttsp6_ts_data *cyttsp6 = (struct cyttsp6_ts_data *)dev->priv;
    int32_t ret = -1;

    mutex_acquire(cyttsp6->cy_sys_mutex);
    for (int32_t i = 0; i < TOUCH_NOTICER_MAX_NUM; i++) {
        if (cyttsp6->cb[i].is_valid) {
            cyttsp6->cb[i].cb = cb;
            cyttsp6->cb[i].usrCtx = usrCtx;
            cyttsp6->cb[i].is_valid = false;
            ret = 0;
            dprintf(CY_PRINT, "[debug]register touch data callback with chan: %d\n", i);
            break;
        }
    }

    if (ret != 0) {
        dprintf(ALWAYS, "there is no invalid callback channel for %s \n", __func__);
    }
    mutex_release(cyttsp6->cy_sys_mutex);

    return ret;
}

static int32_t cyttsp6_register_read(touch_dev_t* dev
                                    , uint32_t reg
                                    , uint32_t* regVal)
{
    if (dev == NULL || dev->priv == NULL || regVal == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s\n", __func__);
        return -1;
    }
    struct cyttsp6_ts_data *cyttsp6 = (struct cyttsp6_ts_data *)dev->priv;

    return cyttsp6_adap_read(cyttsp6, reg, regVal, 1);
}

static int32_t cyttsp6_register_write(touch_dev_t* dev
                                    , uint32_t reg
                                    , uint32_t regVal)
{
    if (dev == NULL || dev->priv == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s\n", __func__);
        return -1;
    }
    struct cyttsp6_ts_data *cyttsp6 = (struct cyttsp6_ts_data *)dev->priv;

    return cyttsp6_adap_write(cyttsp6, reg, &regVal, 1);

}

void touch_test_send_flg(touch_dev_t* dev)
{
    struct cyttsp6_ts_data* sdrv_touch = (struct cyttsp6_ts_data *)dev->priv;
    osEventFlagsSet(sdrv_touch->irq_event, 1);
}

touch_dev_t* cyttsp6_touch_probe(sdrv_touch_handle* touchDrv, const touch_config_t* cfgPtr)
{
    struct cyttsp6_ts_data *cyttsp6 = NULL;
    touch_dev_t* dev = NULL;

    if (cfgPtr == NULL) {
        dprintf(ALWAYS, "invalid parameter for %s \n", __func__);
        return dev;
    }

    if (*touchDrv != NULL) {
        dprintf(ALWAYS, "already probed touch device for %s.\n", cfgPtr->name);
        return &((struct cyttsp6_ts_data *)(*touchDrv))->dev;
    }

    cyttsp6 = malloc(sizeof(struct cyttsp6_ts_data));
    if (!cyttsp6) {
        dprintf(ALWAYS, "%s, alloc mem fail\n", __func__);
        return NULL;
    }
    memset(cyttsp6, 0, sizeof(struct cyttsp6_ts_data));
    cyttsp6->dev.cfg_ptr = cfgPtr;
    cyttsp6->dev.ops = &s_cyttsp6_ops;
    cyttsp6->dev.priv = cyttsp6;
    cyttsp6->i2c_handle = cfgPtr->i2c_dev;

    *touchDrv = cyttsp6;
    dev = &cyttsp6->dev;

    if (touch_init(dev) != 0) {
        dprintf(ALWAYS, "cyttsp6 touch driver init error\n");
    }

    return dev;
}

/* end of file touch_cyttsp6.c */
