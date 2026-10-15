#ifndef RPMSG_INTERNAL_H_
#define RPMSG_INTERNAL_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <types.h>
#include <cmsis_os2.h>
#include <stddef.h>
#include "rpmsg.h"
#include "compiler.h"

#define RPMSG_MS_2_TICK(ms)  ((ms) * osKernelGetTickFreq() / 1000)
#define RPMSG_STD_MSG_FROM_BUF(buf) \
    (struct rpmsg_std_msg *)((char *)buf - offsetof(struct rpmsg_std_msg, data))

//! @name Configuration options
//@{
//! @def RPMSG_MS_PER_INTERVAL
//!
//! Delay in milliseconds used in non-blocking API functions for polling.
//! The default value is 1.
#ifndef RPMSG_MS_PER_INTERVAL
#define RPMSG_MS_PER_INTERVAL (1)
#endif

/*!
 * Common header for all rpmsg messages.
 * Every message sent/received on the rpmsg bus begins with this header.
 */
struct rpmsg_std_hdr
{
    uint32_t src;      /*!< source endpoint address */
    uint32_t dst;      /*!< destination endpoint address */
    uint32_t reserved; /*!< reserved for future use */
    uint16_t len;     /*!< length of payload (in bytes) */
    uint16_t flags;   /*!< message flags */
} __PACKED;

/*!
 * Common message structure.
 * Contains the header and the payload.
 */
struct rpmsg_std_msg
{
    struct rpmsg_std_hdr hdr; /*!< RPMsg message header */
    uint8_t data[1];    /*!< bytes of message payload data */
} __PACKED;

/* rpmsg_std_hdr contains a reserved field,
 * this implementation of RPMSG uses this reserved
 * field to hold the idx and totlen of the buffer
 * not being returned to the vring in the receive
 * callback function. This way, the no-copy API
 * can use this field to return the buffer later.
 */
struct rpmsg_hdr_reserved
{
    uint16_t rfu; /* reserved for future usage */
    uint16_t idx;
};

static inline int rpmsg_is_master(struct rpmsg_instance *ins)
{
    return ins->ins_ops->is_master(ins);
}

static inline int rpmsg_get_buffer_num(struct rpmsg_instance *ins)
{
    return ins->ins_ops->get_buffer_num(ins);
}

static inline int rpmsg_get_payload_size(struct rpmsg_instance *ins)
{
    return ins->ins_ops->get_payload_size(ins);
}

struct rpmsg_endpoint *rpmsg_get_ept(struct rpmsg_instance *ins,
                                     unsigned long addr);

#if defined(__cplusplus)
}
#endif

#endif /* RPMSG_INTERNAL_H_ */
