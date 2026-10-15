/**
 * @file arbitrator_disp.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv arbitrator library.
 *
 * Revision History:
 * -----------------
 */
#include <stdlib.h>
#include <string.h>
#include <sdm_display.h>
#include <lib/list.h>
#include "arbitrator.h"
#include "app_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "arbitrator"

typedef struct _disp_slave_t {
    int init;
    sdm_display_t* disp;
    int tk;
    int preempt;
    int accessor_num;
    struct list_node accessor_list;
} disp_slave_t;

#define CHECK_PREEMPT(s, ap) \
        ({ \
            int rlt = -1; \
            if (s) rlt = ((s)->preempt == arb_preempt_always) ? 0 : \
                       ((s)->preempt == arb_preempt_bfap) ? ((ap) ? -1 : 0) : \
                       ((s)->preempt == arb_preempt_afap) ? ((ap) ? 0 : -1) : \
                       -1; \
            rlt; \
        })

static void disp_slave_init(arb_master_t* master)
{
    if (master) {
        master->slave_num = hal_display_num();
        master->slave = (void*)malloc(master->slave_num*sizeof(disp_slave_t));
        if (!master->slave)
            return;
        memset(master->slave, 0, master->slave_num*sizeof(disp_slave_t));
        if (master->slave) {
            struct list_node* disp_list = sdm_get_display_list();
            sdm_display_t* disp = list_peek_head_type(disp_list, sdm_display_t, node);
            for (int i = 0; i < master->slave_num && disp; i++, disp = list_next_type(disp_list, &disp->node, sdm_display_t, node)) {
                disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
                if (slave) {
                    APP_INFO("disp id:%d", disp->id);
                    slave->init = 1;
                    slave->disp = disp;
                    slave->tk = disp->id;
                    slave->preempt = arb_preempt_never;
                    slave->accessor_num = 0;
                    list_initialize(&slave->accessor_list);
                }
            }
        }
    }
}

static void disp_slave_request_gain(arb_master_t* master, int token, arb_accessor_t* accessor)
{
    APP_INFO("token:%d, accessor:%d", token, accessor->token);
    if (master && accessor) {
        for (int i = 0; i < master->slave_num; i++) {
            disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
            if (slave && slave->init && slave->tk == token) {
                int ap_running = arb_is_ap_running();
                if (!CHECK_PREEMPT(slave, ap_running)) {
                    arb_accessor_t* peak_accessor = list_peek_head_type(&slave->accessor_list, arb_accessor_t, node);
                    if (!peak_accessor) { /* no accessor gain */
                        list_add_tail(&slave->accessor_list, &accessor->node);
                        slave->accessor_num++;
                        if (accessor->p_start) {
                            accessor->p_start(master->magic, token, accessor);
                        }
                    }
                    else { /* another accessor gain */
                        if (accessor->prio > peak_accessor->prio) {
                            /* replace the peak accessor gain the slave */
                            list_add_head(&slave->accessor_list, &accessor->node);
                            slave->accessor_num++;
                            if (peak_accessor->p_suspend) {
                                APP_INFO("notify %d accessor suspend", peak_accessor->token);
                                peak_accessor->p_suspend(master->magic, token, peak_accessor);
                            }
                            if (accessor->p_start) {
                                accessor->p_start(master->magic, token, accessor);
                            }
                        }
                        else {
                            /* insert to back list */
                            arb_accessor_t* bg_accessor = containerof(peak_accessor->node.next, arb_accessor_t, node);
                            while (bg_accessor) {
                                if (accessor->prio > bg_accessor->prio) {
                                    break;
                                }
                                bg_accessor = list_next_type(&slave->accessor_list, &bg_accessor->node, arb_accessor_t, node);
                            }
                            if (!bg_accessor) {
                                list_add_tail(&slave->accessor_list, &accessor->node);
                            }
                            else {
                                list_add_before(&bg_accessor->node, &accessor->node);
                            }
                            if (accessor->p_suspend) {
                                accessor->p_suspend(master->magic, token, accessor);
                            }
                        }
                    }
                }
                break;
            }
        }
    }
}

static void disp_slave_request_release(arb_master_t* master, int token, arb_accessor_t* accessor)
{
    if (master && master->slave && accessor) {
        APP_INFO("token:%d, accessor:%d", token, accessor->token);
        for (int i = 0; i < master->slave_num; i++) {
            disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
            if (slave && slave->init && slave->tk == token) {
                int resume = 0;
                arb_accessor_t* select = list_peek_head_type(&slave->accessor_list, arb_accessor_t, node);
                if (select && select->token == accessor->token) {
                    resume = 1;
                }
                while (select) {
                    if (select->token == accessor->token) {
                        list_delete(&select->node);
                        if (select->p_stop) {
                            select->p_stop(master->magic, token, select);
                        }
                        break;
                    }
                    select = list_next_type(&slave->accessor_list, &select->node, arb_accessor_t, node);
                }
                if (resume) {
                    arb_accessor_t* peak = list_peek_head_type(&slave->accessor_list, arb_accessor_t, node);
                    if (peak && peak->p_resume) {
                        peak->p_resume(master->magic, token, peak);
                    }
                }
                break;
            }
        }
    }
}

static void disp_slave_get_handle(arb_master_t* master, int token, void** handle)
{
    if (master && handle) {
        for (int i = 0; i < master->slave_num; i++) {
            disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
            if (slave && slave->init && token == slave->tk) {
                *handle = slave->disp;
                break;
            }
        }
    }
}

static void disp_slave_preempt_change(arb_master_t* master, int token, int preempt)
{
    if (master) {
        for (int i = 0; i < master->slave_num; i++) {
            disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
            if (slave && slave->init && token == slave->tk) {
                slave->preempt = preempt;
            }
        }
    }
}

static void disp_slave_update(arb_master_t* master, int token)
{
    if (master) {
        for (int i = 0; i < master->slave_num; i++) {
            disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
            if (slave && slave->init && token == slave->tk) {
                int ap_running = arb_is_ap_running();
                if (CHECK_PREEMPT(slave, ap_running)) {
                    arb_accessor_t* accessor = list_peek_head_type(&slave->accessor_list, arb_accessor_t, node);
                    if (accessor) {
                        if (accessor->p_start) {
                            accessor->p_start(master->magic, token, accessor);
                        }
                        accessor = list_next_type(&slave->accessor_list, &accessor->node, arb_accessor_t, node);
                        while (accessor) {
                            if (accessor->p_suspend) {
                                accessor->p_suspend(master->magic, token, accessor);
                            }
                            accessor = list_next_type(&slave->accessor_list, &accessor->node, arb_accessor_t, node);
                        }
                    }
                }
                else {
                    while (1) {
                        arb_accessor_t* accessor = list_peek_head_type(&slave->accessor_list, arb_accessor_t, node);
                        if (!accessor) {
                            break;
                        }
                        list_delete(&accessor->node);
                        if (accessor->p_stop) {
                            accessor->p_stop(master->magic, token, accessor);
                        }
                    }
                }
                break;
            }
        }
    }
}

static void disp_slave_query_gain_accessor(arb_master_t* master, int token, int* accessor_tk)
{
    if (master && accessor_tk) {
        for (int i = 0; i < master->slave_num; i++) {
            disp_slave_t* slave = &((disp_slave_t*)master->slave)[i];
            if (slave && slave->init && token == slave->tk) {
                arb_accessor_t* accessor = list_peek_head_type(&slave->accessor_list, arb_accessor_t, node);
                if (accessor) {
                    *accessor_tk = accessor->token;
                }
                break;
            }
        }
    }
}

arb_ops_t arb_disp_ops = {
    .p_init = disp_slave_init,
    .p_request = disp_slave_request_gain,
    .p_release = disp_slave_request_release,
    .p_getHandle = disp_slave_get_handle,
    .p_preemptChange = disp_slave_preempt_change,
    .p_update = disp_slave_update,
    .p_query = disp_slave_query_gain_accessor,
};
/* EOF */
