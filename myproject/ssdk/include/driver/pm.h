/**
 * @file pm.h
 * @brief Common power manager driver header.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef POWER_MANAGER_H_
#define POWER_MANAGER_H_

#include <lib/list.h>
#include <cmsis_os2.h>

#define pm_lock()   osMutexAcquire(g_pmglobal.mutex, osWaitForever)
#define pm_unlock() osMutexRelease(g_pmglobal.mutex)

enum pm_state_e {
    PM_ACTIVE = 0,
    PM_IDLE,
    PM_LIGHT_SLEEP,
    PM_DEEP_SLEEP,
    PM_STANDBY,
    PM_SHUTDOWN,
    PM_COUNT,
};

struct pm_global_s {
    osMutexId_t mutex;

    struct list_node registry;

    uint8_t pm_state;

    uint16_t state_req[PM_COUNT];

    struct pm_governor_s *governor;
};

struct pm_callback_s {
    struct list_node l_node;

    void *cb_arg;

    int (*suspend)(struct pm_callback_s *cb, enum pm_state_e state);

    void (*resume)(struct pm_callback_s *cb, enum pm_state_e state);
};

struct pm_governor_s {
    int (*init)(void);

    enum pm_state_e (*check_state)(void);

    int (*sleep)(enum pm_state_e state);

    int (*change_state)(enum pm_state_e newstate);

    void *priv;
};

extern struct pm_global_s g_pmglobal;

int pm_initialize(struct pm_governor_s *governor);

int pm_register(struct pm_callback_s *callback);

int pm_unregister(struct pm_callback_s *callback);

void pm_request(enum pm_state_e state);

void pm_release(enum pm_state_e state);

enum pm_state_e pm_checkstate(void);

int pm_changestate(enum pm_state_e newstate);

int pm_suspend_all(enum pm_state_e pmstate, bool force);

int pm_resume_all(enum pm_state_e pmstate);

#endif /* POWER_MANAGER_H_ */
