/*
 * module_helper_hal.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER HAL.
 *
 * Revision History:
 * -----------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <module_helper_hal.h>
#include <module_helper_hal_internal.h>
#include <assert.h>
#include <debug.h>
#include <cmsis_os2.h>
#include "CLI_type.h"

#define PER_ITEM_INVALID   \
    PER_ITEM(INVALID_PER_ID, "", -1, -1, NULL, NULL)

static struct module_per_item module_per_table[MAX_PER_ID + 1] = {
#if CONFIG_MODULE_HELPER_PER_TEST
    PER_ITEM_TEST,
#endif
#if CONFIG_MODULE_HELPER_PER_SYS
    PER_ITEM_SYS,
#endif
#if CONFIG_MODULE_HELPER_PER_DDR
    PER_ITEM_DDR,
#endif
#if CONFIG_MODULE_HELPER_PER_DISP
    PER_ITEM_DISP,
#endif

    PER_ITEM_INVALID
};
static struct module_per_item *get_per_item_by_id(int per_id);

static int init_per_items(void)
{
    struct module_per_item *item = NULL;
    int i;
    printf("%s: init per num = %d\n", __func__, MAX_PER_ID);
    for (i = 0; i < MAX_PER_ID; i++) {
        item = get_per_item_by_id(i);

        if (!item) {
            continue;  //not found
        }

        item->cur_state = 0;

        if (item->init) {
            item->init(item);
        }
    }

    return 0;
}
static bool have_init = false;
int module_helper_init(void)
{
    if (!have_init) {
        //register res
        register_res_rstgens();
        register_res_clks();
        //init per devices
        init_per_items();
        have_init = true;
    }

    return 0;
}

static struct module_per_item *get_per_item_by_id(int per_id)
{
    struct module_per_item *item = NULL;
    int i;
    int arraynum = ARRAYSIZE(module_per_table);

    // find moudle node in table
    for (i = 0; i < arraynum; i++) {
        item = &module_per_table[i];

        if ((int)item->per_id == per_id) { //found
            break;
        }
    }

    if (i == arraynum) {
        return NULL;  //not found
    }

    return item;
}

static int module_state_change_res_handle(unsigned long per_id,
        struct res_handle_item *res_items)
{
    struct res_handle_item *res = res_items;
    int ret = 0;

    for (; res != NULL && res->resid != RES_INVALID; res++) {
        switch (res->resid) {
            case (RES_CLK_START)...(RES_CLK_END):
                //call clk handle
                ssdk_printf(SSDK_CRIT, "handle res clk, %ld, %ld\n", res->resid,
                        res->resparams);
                res_clk_request(per_id, res->resid, res->resparams);
                break;

            case (RES_RSTGEN_START)...(RES_RSTGEN_END):
                //call rst gen handle
                ssdk_printf(SSDK_CRIT, "handle res rst gen, %ld, %ld\n", res->resid,
                        res->resparams);
                res_rstgen_request(per_id, res->resid, res->resparams);
                break;
#if 0

            case RES_REGULATOR:
                //call rst gen handle
                ssdk_printf(SSDK_CRIT, "handle res regulator, %ld, %ld\n", res->resid,
                        res->resparams);
                break;

            case RES_PINCTRL:
                //call rst gen handle
                ssdk_printf(SSDK_CRIT, "handle res pinctrl, %ld, %ld\n", res->resid,
                        res->resparams);
                break;
#endif

            default:
                ssdk_printf(SSDK_CRIT, "no such res %ld\n", res->resid);
                ASSERT(0);
                break;
        }
    }

    return ret;
}
int module_get_cur_state(enum module_per_id per_id)
{
    struct module_per_item *item = NULL;
    item = get_per_item_by_id(per_id);

    if (!item) {
        ssdk_printf(SSDK_CRIT, "not found this per %d\n", per_id);
        return -1;  //not found
    }

    return item->cur_state;
}
int module_set_state(enum module_per_id per_id, int state)
{
    struct module_per_item *item = NULL;
    struct res_handle_item **res_handle = NULL;
    int ret = 0;
    module_helper_init();
    item = get_per_item_by_id(per_id);

    if (!item) {
        ssdk_printf(SSDK_CRIT, "not found this per %d\n", per_id);
        return -1;  //not found
    }

    //call module state change sequence
    if (state >= item->state_num) {
        ssdk_printf(SSDK_CRIT, "out of state table %d max is %d\n", state,
                item->state_num - 1);
        return -1; //out of state table
    }

    ssdk_printf(SSDK_CRIT, "handle request: change per %s state from %d to %d\n",
            item->name, item->cur_state, state);

    if (item->cur_state == state) {
        ssdk_printf(SSDK_CRIT, "cur state is same as request, do nothing\n");
        return 0;
    }

    res_handle = (struct res_handle_item **)((uint8_t *)item->state_table + \
                ((item->cur_state * item->state_num) + state) * (sizeof(void *)));

    if ((*res_handle) == NULL) {
        ssdk_printf(SSDK_CRIT, "not allowed change state from %d to %d\n",
                item->cur_state, state);
        return -1;
    }

    ret = module_state_change_res_handle(item->per_id, *res_handle);

    //update cur state
    if (ret == 0) {
        item->cur_state = state;
    }

    return ret;
}
char *module_get_per_name_by_id(int per_id)
{
    struct module_per_item *item = NULL;
    item = get_per_item_by_id(per_id);

    if (!item) {
        return NULL;
    }

    return item->name;
}

int dump_per()
{
    int i, j;
    int state;
    int arraynum = ARRAYSIZE(module_per_table);
    struct module_per_item *item = NULL;

    // find moudle node in table
    for (i = 0; i < arraynum; i++) {
        item = &module_per_table[i];
        state = module_get_cur_state(item->per_id);
        ssdk_printf(SSDK_CRIT, "%s(%d), cur state:%d\n", item->name, item->per_id,
                state);

        for (j = 0; j < item->state_num; j++) {
            ssdk_printf(SSDK_CRIT, "\t state:%d : %s\n", j,
                    (item->get_state_name ? item->get_state_name(j) : "NULL"));
        }
    }

    return 0;
}

static int cmd_listper(int argc, char *argv[])
{
    dump_per();
    return CLI_FALSE;
}
static int cmd_setper(int argc, char *argv[])
{
    int state;
    int per_id = 0;

    if (argc != 2) {
        ssdk_printf(SSDK_CRIT, "no enough args: \r\nsetper perid state\n");
        return CLI_FALSE;
    }

    per_id = strtoul(argv[0], NULL, 10);
    state = strtoul(argv[1], NULL, 10);
    module_set_state(per_id, state);
    return CLI_FALSE;
}
static int cmd_module_helper_init(int argc, char *argv[])
{
    have_init = false;/* force reinit */
    module_helper_init();
    return CLI_FALSE;
}

//CLI_CMD("listper", "list per", cmd_listper);
//CLI_CMD("setper", "change per state; setper perid state", cmd_setper);
CLI_CMD("modulehelper_init", "init module helper", cmd_module_helper_init);




