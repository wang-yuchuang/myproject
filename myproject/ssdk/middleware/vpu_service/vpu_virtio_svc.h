/*
 * Copyright (c) 2022 Semidrive Semiconductor, Inc.
 * All rights reserved.
 */

#ifndef __VPU_VIRTIO_SVC_H__
#define __VPU_VIRTIO_SVC_H__

#ifndef u16
typedef uint16_t u16;
#endif

#ifndef u32
typedef uint32_t u32;
#endif

#ifndef u64
typedef uint64_t u64;
#endif

#ifndef le16
typedef u16 le16;
#endif

#ifndef le32
typedef u32 le32;
#endif

#ifndef le64
typedef u64 le64;
#endif

void vpu_virtio_svc_entry();

#endif // __VPU_VIRTIO_SVC_H__