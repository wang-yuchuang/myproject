/**
 * @file state_machine.h
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv launcher application.
 *
 * Revision History:
 * -----------------
 */
#ifndef __SD_STATE_MACHINE_H__
#define __SD_STATE_MACHINE_H__
#include <debug.h>

typedef int _STM_STATE;
typedef int _STM_TRIGGER;

typedef struct _stm_t {
    _STM_STATE current;
    _STM_TRIGGER event;
    _STM_STATE destiny;
} stm_t;

typedef struct _stm_func_t {
    _STM_STATE st;
    void (*pf)(void* param);
} stm_func_t;

typedef struct _stm_ctx_t {
    _STM_STATE current;
    _STM_STATE previous;
    _STM_STATE next;
    _STM_TRIGGER event;
    stm_func_t* enter_tbl;
    stm_func_t* exit_tbl;
    stm_func_t* refresh_tbl;
} stm_ctx_t;

static inline int stm_func_run(stm_func_t* tbl, _STM_STATE st, void* param)
{
    if (!tbl) return -1;
    for (int i = 0; tbl[i].st != -1; i++) {
        if (st == tbl[i].st) {
            if (tbl[i].pf) {
                tbl[i].pf(param);
            }
            break;
        }
    }
    return 0;
}

/**
 * [IN] stm: stm_t pointer
 * [IN] ctx: stm_ctx_t pointer
 * [IN] evt: trigger event
 * [RET] 0-success
 */
static inline int stm_set_event(void* stm, void* ctx, int evt, void* param)
{
    if (!stm || !ctx) {
        return -1;
    }
    stm_t* s = (stm_t*)stm;
    stm_ctx_t* c = (stm_ctx_t*)ctx;
    for (int i = 0; s[i].current >= 0; i++) {
        if (s[i].current == c->current) {
            if (s[i].event == evt) {
                c->event = evt;
                c->next = s[i].destiny;
                if (c->current == s[i].destiny) {
                    if (stm_func_run(c->refresh_tbl, c->current, param)) {
                        stm_func_run(c->enter_tbl, c->current, param);
                    }
                }
                else {
                    stm_func_run(c->exit_tbl, c->current, param);
                    stm_func_run(c->enter_tbl, c->next, param);
                    c->previous = c->current;
                    c->current = c->next;
                }
                break;
            }
        }
    }
    return 0;
}

#endif /* __SD_STATE_MACHINE_H__ */
/* EOF */
