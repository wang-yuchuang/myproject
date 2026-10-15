/**
 * @file fee_eeprom.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef DISK_FEE_EEPROM_H_
#define DISK_FEE_EEPROM_H_

#include "fee.h"

#define FEE_EEPROM_NAME(n) "fee_eeprom"#n

#define FEE_EEPROM_SIZE_AVG 2   /* average access size,unit is eeprom_align_size */

struct fee_config_eeprom {
    /* nor flash info */
    const char *disk_name;
    disk_addr_t addr;           /* nor flash sector_size aligned */
    disk_size_t size;           /* sector_size * page_number aligned */
    uint16_t mem_align_size;    /* nor flash read/write memory alignment size */
    uint16_t access_align_size; /* nor flash access alignment size */
    uint32_t sector_size;       /* erasing is based on sector_size */
    uint16_t pe_cycles;         /* flash sector erase the life,Unit ten thousand */

    /* eeprom info */
    const char *fee_name;
    uint16_t eeprom_size;      /* The capacity of the virtual EEPROM */
    uint16_t eeprom_align_size;/* virtual EEPROM access alignment size, alignment access_align_size */
    uint16_t eeprom_pe_cycles; /* virtual EEPROM lifetime */
};

struct fee_eeprom {
    const char *fee_name;
    struct list_node node;              /* list node */
    osMutexId_t fee_mutex;
    struct fee_dev fee_dev;             /* fee device */
    struct fee_config_eeprom fee_con;   /* fee eeprom config */
};

/* fee eeprom APIs */
int fee_init_eeprom(void);
int fee_exit_eeprom(void);

int fee_register_eeprom(struct fee_eeprom *fee);
int fee_unregister_eeprom(struct fee_eeprom *fee);

/* access to multiple blocks, address and length must be aligned eeprom_align_size */
int fee_read_eeprom(const char *fee_name, uint32_t addr, uint8_t *data_buffer,
                    uint16_t length);
int fee_write_eeprom(const char *fee_name, uint32_t addr,
                     const uint8_t *data_buffer, uint16_t length);

#endif /* DISK_FEE_EEPROM_H_ */

