/**
 * @file au_hw_device.h
 * @brief audio hardware device header file.
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef __AU_HW_DEVICE_H__
#define __AU_HW_DEVICE_H__

#include <stdint.h>
#include <stdlib.h>

/* A short period (low latency) 20 ms */
#define OUT_PERIOD_MS 10
#define DMA_PERIOD_COUNT 4
/* (48000*2*2*OUT_PERIOD_MS/1000) (10ms 1920) (20ms 3840)  (10.666ms 2048
 * total 42.667 ms) (21.332  4906  total  85ms )  */
#define DMA_PERIOD_BUFLEN 3840
#define DMA_BUF_TOTAL_LEN (DMA_PERIOD_BUFLEN * DMA_PERIOD_COUNT)

#define get_bit(bitmap, i) ((bitmap & (0x0001 << i)) ? 1 : 0)
#define set_bit(bitmap, i) ((bitmap) |= (0x0001 << (i)))
#define clear_bit(bitmap, i) ((bitmap) &= (~(0x0001 << (i))))

typedef enum {
    AU_HWD_ST_OPENED = 1,
    AU_HWD_ST_IDLE = 2,
    AU_HWD_ST_RUNNING = 3,
    AU_HWD_ST_STOP = 4,
} au_hw_device_status_t;

typedef enum {
    AU_HWD_TYPE_PLAYBACK = 0,
    AU_HWD_TYPE_CAPTURE = 1,
} au_hw_device_type_t;

typedef struct {
    uint8_t *dma_addr;
    uint32_t dma_size;
    uint32_t period_len;
    uint32_t period_count;
    uint32_t boundary;
    volatile uint32_t app_ptr;
    volatile uint32_t hw_ptr;
} au_dmabuf_t;

typedef struct au_hw_device *hw_device_handle_t;
typedef void (*hw_device_callback)(void *cb_param);

typedef struct au_hw_device_ops {
    int (*hw_open)(hw_device_handle_t hw_handle, hw_device_callback hw_callback,
                   void *cb_param);
    int (*hw_close)(hw_device_handle_t hw_handle);
    int (*hw_start)(hw_device_handle_t hw_handle);
    int (*hw_stop)(hw_device_handle_t hw_handle);
    int (*hw_write)(hw_device_handle_t hw_handle, uint8_t *in, uint16_t len);
    int (*hw_read)(hw_device_handle_t hw_handle, uint8_t *out, uint16_t len);
    int (*hw_pause)(hw_device_handle_t hw_handle);
    int (*hw_resume)(hw_device_handle_t hw_handle);
    int (*hw_available)(hw_device_handle_t hw_handle);

} au_hw_device_ops_t;

typedef struct au_hw_device {

    au_hw_device_ops_t *hw_ops;
    void *cb_param;
    void *private_data;
    au_hw_device_type_t type;
    au_hw_device_status_t status;

} au_hw_device_t;

void hw_device_default(au_hw_device_t *hw_device);

int hw_device_open(au_hw_device_t *hw_device, hw_device_callback hw_callback,
                   void *cb_param);
int hw_device_close(au_hw_device_t *hw_device);
int hw_device_start(au_hw_device_t *hw_device);
int hw_device_stop(au_hw_device_t *hw_device);
int hw_device_write(au_hw_device_t *hw_device, uint8_t *in, uint16_t len);
int hw_device_read(au_hw_device_t *hw_device, uint8_t *out, uint16_t len);
int hw_device_pause(au_hw_device_t *hw_device);
int hw_device_resume(au_hw_device_t *hw_device);
int hw_device_available(au_hw_device_t *hw_device);

#endif
