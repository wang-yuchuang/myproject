/*
 * res_clk.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER CLK.
 *
 * Revision History:res_pll.h
 * -----------------
 */
#ifndef _RES_CLK_H
#define _RES_CLK_H

#include <lib/list.h>
#include <assert.h>
#include <module_helper_hal.h>
#include <cmsis_os2.h>
#include <clock_cfg.h>

#define RES_CLK_DEBUG 1

#define MAX_PARENTS_NUM 8
#ifndef REFCNT_NUM
#define REFCNT_NUM ((MAX_PER_ID-1)/8 +1)
#endif
#include "res_clk_id.h"
#if 1
struct pll_spec {
    //pll_config_t **ratetable;

    //only pll need this
    uint8_t ratetable_size;
    uint8_t plldiv : 7; /*indicate pll type, 0, root; 1~4:a,b,c,d; 5 dummy root*/
    uint8_t moreprecise : 1;
    uint16_t dependson;

    //pll_config_t config;
};
struct uuu_spec {
    //only uuu need this
    uint8_t uuu_type;   /*indicate uuu type, 0~3:M,N,P,Q */
};
#endif
struct clk {
    uint32_t resid;//global res id
    clk_node_t* clk;
    clk_node_t* clk_gating;
#ifdef RES_CLK_DEBUG
    char name[16];
#endif
    osMutexId_t lock;

    uint8_t refcount[REFCNT_NUM];  //care about rate
    uint8_t refcount_en[REFCNT_NUM]; //only care about en/dis status

    uint16_t clkid;
    /* MAX_PARENTS_NUM is 8, so four bits is enough */
    int8_t cur_parent_index : 4;
    uint8_t parent_nums : 4;
    int8_t ptable_size;

    /* array of parent clock ids */
    int16_t parents[MAX_PARENTS_NUM];
    //struct clk *parents[MAX_PARENTS_NUM];   //const
    /* array of parent clock node */
    clk_node_t *parents_clk[MAX_PARENTS_NUM];

    struct list_node node;  //dynmic, current node
    struct list_node child; //dynmic, head of child list
    unsigned long rate;//dummy rate for dummy node;

    //can be optimized
    int *ptable;
    uint32_t gate_resid;//global res id

    unsigned long minfreq;//0, means no limit;
    unsigned long maxfreq;//UINT32_MAX, means no limit;
    int16_t mindiv;//1, means no limit;
    int16_t maxdiv;//INT16_MAX, means no limit;
#if 1
    union {
        struct pll_spec pll;
        struct uuu_spec uuu;
    };
#endif
    //debug
    uint8_t enable_cnt;
    uint8_t disable_cnt;

#if 0
    int (*is_enable)(struct clk *clk);
    int (*enable)(struct clk *clk);
    int (*disable)(struct clk *clk);
    int (*set_rate)(struct clk *clk, unsigned long prate, unsigned long freq);
    unsigned long (*get_rate)(struct clk *clk, unsigned long prate,
                              bool bymonitor);
    unsigned long (*round_rate)(struct clk *clk, int pindex,
                                unsigned long *prate, unsigned long freq);
    int (*set_parent)(struct clk *clk, int parent_index);
#endif
};
void register_res_clks(void);
int res_clk_request(enum module_per_id per_id, enum clk_id clkid,
                    unsigned long rate);
unsigned long res_clk_get_rate_by_id(enum clk_id clkid, bool bymonitor);
unsigned long res_clk_round_rate_by_id(enum clk_id clkid, unsigned long rate);
bool get_min_max_by_id_from_board(enum clk_id clkid, unsigned long *min, unsigned long *max);
//for debug
void dump_clktree(struct clk *clk, bool bymonitor);
void dump_clktree_by_name(const char *name, bool bymonitor);

#endif /* _RES_CLK_H */


