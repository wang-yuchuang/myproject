/********************************************************
 *          Copyright(c) 2018   Semidrive               *
 ********************************************************/

#ifndef __FUSE_CTRL_H__
#define __FUSE_CTRL_H__

#include <compiler.h>
#include <types.h>

#define FUSE_PROG_KEY 0x9458u

#define PART_FUSE_ADDRESS 0x6
#define PART_FUSE_MASK 0x0000ffff
#ifdef CONFIG_PART_ID
#define PART_SSDK_MARCO CONFIG_PART_ID
#else
#define PART_SSDK_MARCO 0x3640
#endif 

typedef enum {
    PLOCK = 1,
    RLOCK = 2,
    OLOCK = 4,
    HLOCK = 8,
} fuse_lock_bits_e;

typedef enum {
    FUSE_ACC_SAFE,
    FUSE_ACC_AP,
} fuse_acc_domain_e;

typedef enum {
    PARITY_INVALID,
    PARITY_NONE,
    PARITY_ECC,
    PARITY_RED,
} fuse_pari_type_e;

uint32_t fuse_read(uint32_t index);
uint32_t fuse_sense(uint32_t index, uint32_t *data);
uint32_t fuse_program(uint32_t index, uint32_t v);
uint32_t fuse_program_raw(uint32_t index, uint32_t val, bool ecc);
uint32_t fuse_reload(void);
void fuse_set_sticky_bit(fuse_acc_domain_e dom, uint32_t id);
void fuse_lock_bank(fuse_acc_domain_e dom, uint32_t bank, uint32_t lock_bits);
uint32_t fuse_get_bank_lock(fuse_acc_domain_e dom, uint32_t bank);
void fuse_ctl_cfg_timing(uint32_t freq_mhz);
void fuse_ctl_trigger_sw_vio(uint32_t msk);
uint32_t fuse_repair(void);
void fuse_part_load(void);

#endif /* __FUSE_CTRL_H__ */
