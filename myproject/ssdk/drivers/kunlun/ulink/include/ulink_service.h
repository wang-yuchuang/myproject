#pragma once

enum ulink_copy_dir {
    COPY_DIR_SSA_TO_SSB,
    COPY_DIR_SSB_TO_SSA,
};

enum ulink_copy_priority {
    COPY_PRI_HIGHEST = -50,
    COPY_PRI_DISPLAY = -40,
    COPY_PRI_CAMERA = -30,
    COPY_PRI_DEFAULT = 0,
    COPY_PRI_LOWEST = 50,
};

struct ulink_copy_param {
    enum ulink_copy_dir dir;
    enum ulink_copy_priority pri;
    uint64_t dst;
    uint64_t src;
    uint32_t size;
};

typedef void (*ulink_copy_callback)(int retval, void *data);

#ifdef ULINK_HPL_DMA_COPY_SERVER
int ulink_copy(struct ulink_copy_param *param);

int ulink_copy_async(struct ulink_copy_param *param, ulink_copy_callback cb,
                     void *data);

/* compatibility */
static int ulink_hpl_dma_copy(int dir, uint64_t dst, uint64_t src,
                              uint32_t size) {
    struct ulink_copy_param p = {
        .dir = dir,
        .pri = COPY_PRI_DEFAULT,
        .dst = dst,
        .src = src,
        .size = size,
    };

    return ulink_copy(&p);
}

#else
static inline int ulink_copy(struct ulink_copy_param *param) { return -1; }

static inline int ulink_copy_async(struct ulink_copy_param *param,
                                   ulink_copy_callback cb, void *data) {
    return -1;
}

static inline int ulink_hpl_dma_copy(int dir, uint64_t dst, uint64_t src,
                                     uint32_t size) {
    return -1;
}


void ulink_service_init();

#endif
