/*
 * spinlock.h
 *
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: ARM spinlock interface.
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_ARM_SPINLOCK_H
#define INCLUDE_ARM_SPINLOCK_H

#define SPIN_LOCK_INITIAL_VALUE (0)

#ifndef ASSEMBLY

__BEGIN_CDECLS

/* spin lock type */
typedef unsigned long spin_lock_t;

/*
 * arch spinlock init.
 *
 * @spinlock    spinlock address.
 */
#ifdef CONFIG_ARCH_WITH_SMP
void arch_spin_lock_init(spin_lock_t *spinlock);
#else
static inline void arch_spin_lock_init(spin_lock_t *spinlock)
{
    *spinlock = SPIN_LOCK_INITIAL_VALUE;
}
#endif

/*
 * arch spin lock.
 *
 * @spinlock    spinlock address.
 */
#ifdef CONFIG_ARCH_WITH_SMP
void arch_spin_lock(spin_lock_t *spinlock);
#else
static inline void arch_spin_lock(spin_lock_t *spinlock)
{
    *spinlock = 1;
}
#endif

/*
 * arch spin trylock.
 *
 * @spinlock    spinlock address.
 */
#ifdef CONFIG_ARCH_WITH_SMP
int arch_spin_trylock(spin_lock_t *spinlock);
#else
static inline int arch_spin_trylock(spin_lock_t *spinlock)
{
    *spinlock = 1;
    return 0;
}
#endif

/*
 * arch spin unlock.
 *
 * @spinlock    spinlock address.
 */
#ifdef CONFIG_ARCH_WITH_SMP
void arch_spin_unlock(spin_lock_t *spinlock);
#else
static inline void arch_spin_unlock(spin_lock_t *spinlock)
{
    *spinlock = 0;
}
#endif

__END_CDECLS

#endif

#endif