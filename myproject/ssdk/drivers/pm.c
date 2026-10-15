/**
 * @file pm.c
 * @brief Common power manager source.
 *
 * @copyright Copyright (c) 2020  Semidrive Semiconductor.
 *            All rights reserved.
 */

#include <driver/pm.h>
#include <arch/irq.h>
#include <debug.h>
#include <param.h>

struct pm_global_s g_pmglobal = {
    .registry = LIST_INITIAL_VALUE(g_pmglobal.registry),
};

int pm_suspend_all(enum pm_state_e pmstate, bool force)
{
    struct pm_callback_s *cb;
    int ret = osOK;

    if (pmstate <= PM_IDLE) {
        return osOK;
    }

    list_for_every_entry(&g_pmglobal.registry, cb, struct pm_callback_s, l_node) {
        if (cb->suspend) {
            ret = cb->suspend(cb, pmstate);
            ret = force ? osOK : ret;

            if (ret != osOK) {
                struct list_node *cur, *pre;

                cur = &cb->l_node;
                pre = list_prev(&g_pmglobal.registry, cur);

                while (pre) {
                    struct pm_callback_s *tmp;

                    tmp = containerof(pre, struct pm_callback_s, l_node);
                    if (tmp->resume) {
                        tmp->resume(tmp, pmstate);
                    }

                    cur = pre;
                    pre = list_prev(&g_pmglobal.registry, cur);
                }

                break;
            }
        }
    }

    return ret;
}

int pm_resume_all(enum pm_state_e pmstate)
{
    struct pm_callback_s *cb;

    if (pmstate > PM_IDLE) {
        list_for_every_entry(&g_pmglobal.registry, cb, struct pm_callback_s, l_node) {
            if (cb->resume) {
                cb->resume(cb, pmstate);
            }
        }
    }

    return osOK;
}

int pm_register(struct pm_callback_s *callback)
{
    int ret = osOK;

    if (osKernelGetState() >= osKernelRunning) {
        ret = pm_lock();
        if (ret == osOK) {
            list_add_tail(&g_pmglobal.registry, &callback->l_node);
            pm_unlock();
        }
    }
    else {
        list_add_tail(&g_pmglobal.registry, &callback->l_node);
    }

    return ret;
}

int pm_unregister(struct pm_callback_s *callback)
{
    int ret;

    ret = pm_lock();
    if (ret == osOK) {
        list_delete(&callback->l_node);
        pm_unlock();
    }

    return ret;
}

void pm_request(enum pm_state_e state)
{
    irq_state_t irq_state;

    irq_state = arch_irq_save();

    ASSERT(state < PM_COUNT);
    ASSERT(g_pmglobal.state_req[state] < UINT16_MAX);

    g_pmglobal.state_req[state]++;

    arch_irq_restore(irq_state);
}

void pm_release(enum pm_state_e state)
{
    irq_state_t irq_state;

    irq_state = arch_irq_save();

    ASSERT(state < PM_COUNT);
    ASSERT(g_pmglobal.state_req[state] > 0);

    g_pmglobal.state_req[state]--;

    arch_irq_restore(irq_state);
}

enum pm_state_e pm_checkstate(void)
{
    ASSERT(g_pmglobal.governor &&
           g_pmglobal.governor->check_state);

    return g_pmglobal.governor->check_state();
}

int pm_changestate(enum pm_state_e newstate)
{
    ASSERT(g_pmglobal.governor &&
           g_pmglobal.governor->sleep &&
           g_pmglobal.governor->change_state);

    return g_pmglobal.governor->change_state(newstate);
}

int pm_initialize(struct pm_governor_s *governor)
{
    int ret = osOK;

    g_pmglobal.mutex = osMutexNew(NULL);
    g_pmglobal.governor = governor;

    if (g_pmglobal.governor->init) {
        ret = g_pmglobal.governor->init();
    }

    return ret;
}
