/**
 * @file rpmsg_service.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef RPMSG_SERVICE_H
#define RPMSG_SERVICE_H

#include <types.h>
#include <rpmsg.h>

typedef struct rpmsg_service {
    uint8_t     rpmsg_type;
    uint8_t     self_proc;
    uint8_t     remote_proc;
    paddr_t     shm_phys_base;      /* only for virtio*/
    uint32_t    shm_size;           /* only for virtio*/
    bool        is_master;
} rpmsg_service_t;

/**
 * @brief rpmsg service init.
 *
 * @return init result.
 */
int rpmsg_service_init(uint8_t num, rpmsg_service_t *services);

/**
 * @brief rpmsg service deinit.
 *
 */
void rpmsg_service_deinit(void);

/**
 * @brief get rpmsg dev.
 *
 * @param[in] rpmsg_type rpmsg type.
 * @param[in] rproc remote rproc.
 *
 * @return init result.
 */
struct rpmsg_instance *rpmsg_get_instance(uint8_t rpmsg_type, uint8_t rproc);

#endif /* RPMSG_SERVICE_H */
