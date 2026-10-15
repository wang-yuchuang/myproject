#pragma once

#include <string.h>
#include <share_memory.h>

/* semidrive share memory */
#define SHM_MAGIC (('s' << 24) | ('d' << 16) | ('s' << 8) | ('m' << 0))

typedef struct {
    uint32_t magic;
    uint16_t id;
    uint16_t cmd;
    int32_t retval;

    uint64_t addr;
    uint32_t align;
    uint32_t size;
    uint16_t flags;
    uint16_t mode;
    char name[SHM_NAME_MAXLEN + 1];
} shm_msg_t;

static inline void shm_msg_init(shm_msg_t *msg, uint16_t id, uint16_t cmd) {
    memset(msg, 0, sizeof(*msg));
    msg->magic = SHM_MAGIC;
    msg->id = id;
    msg->cmd = cmd;
    msg->retval = 0;
}

static inline int shm_rx_check(void *buf, size_t len, uint32_t from) {
    shm_msg_t *msg = buf;
    return (from == SHM_EPT) && (len == sizeof(shm_msg_t)) &&
           (msg->magic == SHM_MAGIC);
}

enum {
    SHM_CMD_HANDSHAKE = 0x1,
    SHM_CMD_ALLOC,
    SHM_CMD_FREE,
    SHM_CMD_FIND,
    SHM_CMD_LOCK,
    SHM_CMD_UNLOCK,
};

#define SHM_SVC_CMD_START SHM_CMD_ALLOC
#define SHM_SVC_CMD_STOP SHM_CMD_FREE
