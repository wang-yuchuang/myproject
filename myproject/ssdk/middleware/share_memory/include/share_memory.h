#pragma once

#include <stddef.h>
#include <stdint.h>

/*
 * Permission control of share memory. The read/write permission can be set for
 * each domain separately. Support 5 domains: saf, sec, mp, ap1 and ap2.
 */
#define SHM_M_SAF_READ (1u << 0)
#define SHM_M_SAF_WRITE (1u << 1)
#define SHM_M_SEC_READ (1u << 2)
#define SHM_M_SEC_WRITE (1u << 3)
#define SHM_M_MP_READ (1u << 4)
#define SHM_M_MP_WRITE (1u << 5)
#define SHM_M_AP1_READ (1u << 6)
#define SHM_M_AP1_WRITE (1u << 7)
#define SHM_M_AP2_READ (1u << 8)
#define SHM_M_AP2_WRITE (1u << 9)

/*
 * Physical address of share memory must below 4G.
 */
#define SHM_F_PHYS_ADDR_BELOW_4G (1u << 0)

/*
 * Share memory can be accessed by other subsystem, like SSA/SSB sharing.
 */
#define SHM_F_SCOPE_SUBSYSTEM (1u << 1)

/*
 * Max length of share memory name.
 */
#define SHM_NAME_MAXLEN (15)

/*
 * Allocate a share memory.
 *
 * @align:      alignment of share memory
 * @size:       size of share memory
 * @flags:      flags of share memory, like SHM_F_xxx
 * @mode:       (optional) mode of share memory, like SHM_M_xxx
 * @name:       (optional) name of share memory
 * @addr:       pointer to physical address
 *
 * Return 0 on success (physical address stored in @addr) or errno on failure.
 */
int shm_alloc(uint32_t align, uint32_t size, uint16_t flags, uint16_t mode,
              const char *name, uint64_t *addr);

/*
 * Free a share memory.
 *
 * @addr:       physical address of share memory
 */
void shm_free(uint64_t addr);

/*
 * Find an allocated share memory.
 *
 * @name:       name of share memory
 * @addr:       pointer to physical address
 *
 * Return 0 on success (physical address stored in @addr) or errno on failure.
 */
int shm_find(const char *name, uint64_t *addr);

/*
 * Lock a share memory to prevent de-allocation.
 *
 * @addr:       physical address of share memory
 *
 * Return 0 on success or errno on failure.
 */
int shm_lock(uint64_t addr);

/*
 * Unlock a share memory.
 *
 * @addr:       physical address of share memory
 */
void shm_unlock(uint64_t addr);
