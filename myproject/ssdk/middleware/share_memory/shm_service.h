#pragma once

typedef void *shm_handle_t;

typedef struct {
    int (*alloc_mem)(shm_handle_t h, uint32_t align, uint32_t size,
                     uint16_t flags, uint16_t mode, const char *name,
                     uint64_t *addr);
    void (*free_mem)(shm_handle_t h, uint64_t addr);
    int (*find_mem)(shm_handle_t h, const char *name, uint64_t *addr);
    int (*begin_access)(shm_handle_t h, uint64_t addr, uint32_t *size,
                        char *name);
    void (*end_access)(shm_handle_t h, uint64_t addr);

    int (*init)(shm_handle_t *h);
    void (*dump)(shm_handle_t h);
} shm_ops_t;

const shm_ops_t *shm_get_ops(void);
