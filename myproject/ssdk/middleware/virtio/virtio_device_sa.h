#pragma once

#include <driver/kunlun/mbox/sdrv_mbox_drv.h>
#include <virtio_device.h>

void *virtio_device_create_mmio_sa(struct virtio_device_id id, u64 features,
        struct virtio_device_ops *ops, u32 nvqs, u32 max_sizes[],
        u32 reg_base, u32 cfg_size, sdrv_mbox_rproc_t mbox_rproc, int mbox_addr, void *priv);
u32 virtio_device_num_get_sa(void *vdev, u32 vq);
void *virtio_device_desc_get_sa(void *vdev, u32 vq);
void *virtio_device_avail_get_sa(void *vdev, u32 vq);
void *virtio_device_used_get_sa(void *vdev, u32 vq);
void *virtio_device_config_area_get_sa(void *vdev);
void virtio_device_config_updated_sa(void *vdev);
void virtio_device_notify_sa(void *vdev);


