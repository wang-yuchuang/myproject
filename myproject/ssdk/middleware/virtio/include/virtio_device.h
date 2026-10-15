#pragma once

#include <cmsis_os2.h>

struct virtio_device_id {
    u32 device;
    u32 vendor;
};

struct virtio_device_ops {
    /*
     * Frontend has written something to device config area.
     * Leave it to NULL if device doesn't care.
     *
     * priv:        device private in virtio_device_create_mmio()
     * offset:      where frontend has written in config area
     * len:         how many bytes frontend has written
     */
    void (*set_cfg)(void *dev, u32 offset, u32 len);

    /*
     * Frontend wants to do a device reset.
     * Can be NULL if device doesn't need to be reset.
     *
     * priv:        device private in virtio_device_create_mmio()
     */
    void (*reset)(void *dev);

    /*
     * Frontend notifies a virtqueue update.
     * Can be NULL if device doesn't need notification.
     *
     * priv:        device private in virtio_device_create_mmio()
     */
    void (*notify)(void *dev);
};

/*
 * Create a MMIO transport type virtio device.
 *
 * id:          virtio device id
 * features:    device features
 * ops:         device operations
 * nvqs:        virtqueue count of this device
 * max_sizes:   max size of each virtqueue, not actual driver used size
 * reg_base:    MMIO register base physical address
 * cfg_size:    size of config area in bytes
 * mbox_rproc:  mailbox remote processor
 * mbox_addr:   mailbox address
 * priv:        device private, will be passed in virtio_device_ops
 *
 * Returns a token for virtio device.
 */
void *virtio_device_create_mmio(struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[],
        u32 reg_base, u32 cfg_size, sdrv_mbox_rproc_t mbox_rproc, int mbox_addr, void *priv);

/*
 * Create a MMIO transport type virtio device.
 *
 * id:          virtio device id
 * features:    device features
 * ops:         device operations
 * nvqs:        virtqueue count of this device
 * max_sizes:   max size of each virtqueue, not actual driver used size
 * rproc:       remote processor this device serves for
 * name:        virtio device name
 * priority:    thread priority for this device
 * priv:        device private, will be passed in virtio_device_ops
 *
 * Returns a token for virtio device.
 */
void *virtio_device_create_common(struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[], sdrv_mbox_rproc_t rproc,
        const char *name, osPriority_t priority, void *priv);

/*
 * Destroy a virtio device.
 *
 * vdev:        token for virtio device
 */
void virtio_device_destroy(void *vdev);

/*
 * Get num of a virtqueue.
 *
 * vdev:        token for virtio device
 * vq:          virtqueue id
 *
 * Returns capacity of a virtqueue.
 */
u32 virtio_device_num_get(void *vdev, u32 vq);

/*
 * Get desc area base address of a virtqueue.
 *
 * vdev:        token for virtio device
 * vq:          virtqueue id
 *
 * Returns virtual address of desc area.
 */
void *virtio_device_desc_get(void *vdev, u32 vq);

/*
 * Get avail area base address of a virtqueue.
 *
 * vdev:        token for virtio device
 * vq:          virtqueue id
 *
 * Returns virtual address of avail area.
 */
void *virtio_device_avail_get(void *vdev, u32 vq);

/*
 * Get used area base address of a virtqueue.
 *
 * vdev:        token for virtio device
 * vq:          virtqueue id
 *
 * Returns virtual address of used area.
 */
void *virtio_device_used_get(void *vdev, u32 vq);

/*
 * Get config area base address.
 *
 * vdev:        token for virtio device
 *
 * Returns virtual address of config area.
 */
void *virtio_device_config_area_get(void *vdev);

/*
 * Notify virtio service that we've updated config area.
 *
 * vdev:        token for virtio device
 */
void virtio_device_config_updated(void *vdev);

/*
 * Send a virtqueue update notification to frontend.
 *
 * vdev:        token for virtio device
 */
void virtio_device_notify(void *vdev);


