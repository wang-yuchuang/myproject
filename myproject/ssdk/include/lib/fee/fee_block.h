/**
 * @file fee_block.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_FEE_BLCOK_H_
#define DISK_FEE_BLCOK_H_

#include "fee.h"

#define FEE_BLOCK_NAME(n) "fee_block"#n

struct fee_config_block {
    /* nor flash info */
    const char *disk_name;
    disk_addr_t addr;           /* nor flash sector_size aligned */
    disk_size_t size;           /* sector_size * page_number aligned */
    uint16_t mem_align_size;    /* nor flash read/write memory alignment size */
    uint16_t access_align_size; /* nor flash access alignment size */
    uint32_t sector_size;       /* erasing is based on sector_size */
    uint16_t pe_cycles;         /* flash sector erase the life,Unit ten thousand */

    /* block info */
    const char *fee_name;
    uint16_t block_number;      /* number of target virtual blocks */
    uint16_t block_length;      /* maximum size of the target virtual block, align size access_ALIGN_size */
    uint32_t block_data_size;   /* target virtual block data size */
};

struct fee_block {
    const char *fee_name;
    struct list_node node;              /* list node */
    osMutexId_t fee_mutex;
    struct fee_dev fee_dev;             /* fee device */
    struct fee_config_block fee_con;    /* fee block config */
};

/* fee block APIs */
int fee_init_block(void);
int fee_init_block(void);

int fee_register_block(struct fee_block *fee);
int fee_unregister_block(struct fee_block *fee);

/* single block can be accessed */
int fee_read_block(const char *fee_name, uint16_t block_number,
                   uint16_t block_offset, uint8_t *data_buffer, uint16_t length);
int fee_write_block(const char *fee_name, uint16_t block_number,
                    const uint8_t *data_buffer, uint16_t length);

#endif /* DISK_FEE_BLCOK_H_ */

