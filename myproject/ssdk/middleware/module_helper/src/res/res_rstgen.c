/*
 * res_rstgen.c
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER RSTGEN.
 *
 * Revision History:
 * -----------------
 */
#include <res/res_rstgen.h>
#include <module_helper_hal_internal.h>
#include <string.h>
#include <debug.h>
#include <cmsis_os2.h>
#include "CLI_type.h"

struct list_node g_rstgen_root = LIST_INITIAL_VALUE(g_rstgen_root);

//internal for pll & ckgen
static bool rstgen_is_iso(struct rstgen *rst)
{
    return (rst->rstid >= RSTGEN_TYPE_ISO_START
            && rst->rstid <= RSTGEN_TYPE_ISO_END);
}

static bool rstgen_is_core(struct rstgen *rst)
{
    return (rst->rstid >= RSTGEN_TYPE_CORE_START
            && rst->rstid <= RSTGEN_TYPE_CORE_END);
}

static bool rstgen_is_module(struct rstgen *rst)
{
    return (rst->rstid >= RSTGEN_TYPE_MODULE_START
            && rst->rstid <= RSTGEN_TYPE_MODULE_END);
}

static enum rstgen_param rstgen_get_stat_from_hw(struct rstgen *rst)
{
    int ret = 0;

    reset_sig_t* rst_sig = rst->reset_sig;
    if (rst_sig)
        ret = reset_ctl_status(rst_sig);

    if (ret == 0) {
        return RST_RELEASE;
    }
    else if (ret > 0) {
        return RST_HOLD;
    }
    else {
        return RST_INVALID;
    }
}

static int rstgen_is_released(struct rstgen *rst)
{
    return rst->release_cnt > rst->hold_cnt;
}

static int rstgen_endis(struct rstgen *rst, bool isrelease)
{
    int ret = 0;

    reset_sig_t* rst_sig = rst->reset_sig;
    if (!rst_sig) {
        return -1;
    }

    if (isrelease) {
        ret = reset_ctl_deassert(rst_sig);
    }
    else {
        ret = reset_ctl_assert(rst_sig);
    }

    if (ret) {
        ret = 0;
    }
    else {
        ret = -1;
    }

    if (ret == 0) {
        isrelease ? rst->release_cnt++ : rst->hold_cnt++;
    }

    return ret;
}

static int rstgen_release(struct rstgen *rst)
{
    return rstgen_endis(rst, true);
}

static int rstgen_hold(struct rstgen *rst)
{
    return rstgen_endis(rst, false);
}

static struct rstgen *get_rstgen_by_id_internal(struct list_node *list,
        unsigned long rstid)
{
    struct rstgen *rst;

    if (!list) {
        return get_rstgen_by_id_internal(&g_rstgen_root, rstid);
    }

    if (list_is_empty(list)) {
        return NULL;
    }

    list_for_every_entry(list, rst, struct rstgen, node) {
        //ssdk_printf(SSDK_CRIT,"clk %s id %d, p %ld\n", clk->name, clk->clkid, clkid);
        if (rst->rstid ==
                rstid) { //TODO:also can check if this clk allowed to be accessed by user
            return rst;
        }
    }
    return NULL;
}

static struct rstgen *get_rstgen_by_id(unsigned long rstid)
{
    return get_rstgen_by_id_internal(NULL, rstid);
}

static int rstgen_init_refcount_bitmap(struct rstgen *rst)
{
    int i;

    for (i = 0; i < REFCNT_NUM; i++) {
        rst->refcount[i] = 0;
    }

    return 0;
}
static int rstgen_get_refcount_bits(struct rstgen *rst)
{
    int i;
    uint8_t n;
    int count = 0;

    for (i = 0; i < REFCNT_NUM; i++) {
        int j;
        n = rst->refcount[i];

        for (j = 0; j < 8; j++) {
            if ((1 << j)&n) {
                count++;
            }
        }
    }

    return count;
}

static int rstgen_set_refcount_bit(struct rstgen *rst, int per_id)
{
    int i = per_id / 8;
    ASSERT(i < REFCNT_NUM);
    rst->refcount[i] |= 1 << (per_id % 8);
    return 0;
}

static int rstgen_clear_refcount_bit(struct rstgen *rst, int per_id)
{
    int i = per_id / 8;
    ASSERT(i < REFCNT_NUM);
    rst->refcount[i] &= ~(1 << (per_id % 8));
    return 0;
}

static int rstgen_test_refcount_bit(struct rstgen *rst, int per_id)
{
    int i = per_id / 8;
    ASSERT(i < REFCNT_NUM);
    return (rst->refcount[i] & (1 << (per_id % 8)));
}


static int rstgen_get_refcount(struct rstgen *rst)
{
    int count = 0;
    //count refcount bitmap
    count += rstgen_get_refcount_bits(rst);
    return count;
}



/* child must call this function after add itself refcount,
*  and must change from 0 ->1.
*/
static int res_rstgen_inc_refcount(struct rstgen *rst,
                                   unsigned long per_id)
{
    int ret = 0;
    bool is_released = false;

    if (per_id != INVALID_PER_ID) {
        rstgen_set_refcount_bit(rst, per_id);
    }

    //TODO, have not check the refcount
    int cnt = rstgen_get_refcount(rst);

    if (cnt == 1) {
        if (rst->polar) {
            if (rst->has_common_ops) {
                is_released = rstgen_is_released(rst);
            }

            if (is_released && rst->has_common_ops) {
                ret = rstgen_hold(rst);
            }
        }
        else {
            if (rst->has_common_ops) {
                is_released = rstgen_is_released(rst);
            }

            if (!is_released && rst->has_common_ops) {
                ret |= rstgen_release(rst);
            }
        }
    }

    return ret;
}
static int res_rstgen_dec_refcount(struct rstgen *rst,
                                   unsigned long per_id)
{
    int ret = 0;
    bool is_released = true;

    if (per_id != INVALID_PER_ID) {
        rstgen_clear_refcount_bit(rst, per_id);
    }

    int cnt = rstgen_get_refcount(rst);

    if (cnt == 0) {
        if (rst->polar) {
            if (rst->has_common_ops) {
                is_released = rstgen_is_released(rst);
            }

            if (!is_released && rst->has_common_ops) {
                ret |= rstgen_release(rst);
            }
        }
        else {
            if (rst->has_common_ops) {
                is_released = rstgen_is_released(rst);
            }

            if (is_released && rst->has_common_ops) {
                ret = rstgen_hold(rst);
            }
        }
    }

    return ret;
}
bool res_rstgen_is_released(unsigned long rstid)
{
	int rst_status=-1;
	struct rstgen *rst = get_rstgen_by_id(rstid);
	if (!rst) {
        	ssdk_printf(SSDK_CRIT, "no such rst %ld\n", rstid);
        	return false;
    	}

	rst_status = rstgen_get_stat_from_hw(rst);

    if (rst_status == RST_RELEASE) {
        return true;
    }
    else if (rst_status == RST_HOLD) {
        return false;
    }
	return false;
}
int res_rstgen_request(unsigned long per_id, unsigned long rstid,
                       unsigned long param)
{
    struct rstgen *rst = get_rstgen_by_id(rstid);
    ssdk_printf(SSDK_CRIT, "req from %ld : rst %p id %ld param %lu\n", per_id, rst,
            rstid, param);

    if (!rst) {
        ssdk_printf(SSDK_CRIT, "no such rst %ld\n", rstid);
        return -1;
    }

    enum rstgen_param p = param;

    if (rst->polar) {
        if (p == RST_HOLD) {
            return res_rstgen_inc_refcount(rst, per_id);
        }
        else if (p == RST_RELEASE) {
            return res_rstgen_dec_refcount(rst, per_id);
        }
    }
    else {
        if (p == RST_HOLD) {
            return res_rstgen_dec_refcount(rst, per_id);
        }
        else if (p == RST_RELEASE) {
            return res_rstgen_inc_refcount(rst, per_id);
        }
    }

    ASSERT(0);
    return 0;
}

#define RSTGEN_ITEM_ISO(_rstid, _reset_sig, _name)  \
     {   \
     .rstid = _rstid,    \
     .reset_sig= _reset_sig,     \
      .name = _name, \
      .polar = 0,   \
     }

#define RSTGEN_ITEM(_rstid, _reset_sig, _name)  \
      {   \
      .rstid = _rstid,    \
      .reset_sig= _reset_sig,     \
       .name = _name, \
       .polar = 0,   \
      }

#define RSTGEN_INVALID_ID (RSTGEN_ID_END+1)
#define RSTGEN_ITEM_NULL RSTGEN_ITEM(RSTGEN_INVALID_ID, 0, "")

#include "rstgen_res_table.h"

static bool is_res_belong_this_domain(uint32_t resid)
{

    return true;
}

void register_res_rstgens(void)
{
    int i;
    int num = ARRAYSIZE(init_rstgens_table);
    printf("%s: init_rstgens_table num = %d\n", __func__, num);
    struct rstgen *rst;
    list_initialize(&g_rstgen_root);

    for (i = 0; i < (num - 1); i++) {
        enum rstgen_param rst_status;
        rst = &init_rstgens_table[i];

        if(rst->rstid == RSTGEN_INVALID_ID)
            break;
        //ssdk_printf(SSDK_CRIT, "register rst %s \n", rst->name);
        if (!is_res_belong_this_domain(rst->resid)) {
            continue;
        }

        list_clear_node(&rst->node);
        //list_initialize(&rst->child);
        //init refcount
        rstgen_init_refcount_bitmap(rst);

        rst->has_common_ops = 1;
#if 0
        rst->is_released = rstgen_is_released;
        rst->release = rstgen_release;
        rst->hold = rstgen_hold;
#endif
        rst_status = rstgen_get_stat_from_hw(rst);

        if (rst_status == RST_RELEASE) {
            rst->release_cnt = 1;
        }
        else if (rst_status == RST_HOLD) {
            rst->hold_cnt = 1;
        }
        else { //invalid
            continue;
        }

        list_add_tail(&g_rstgen_root, &rst->node);
    }

}

static void print_prefix(int depth, const char *s)
{
    int i;

    for (i = 0; i < depth; i++) {
        ssdk_printf(SSDK_CRIT, "%s", s);
    }
}

void dump_rstgen_internal(struct rstgen *rst, int depth)
{
    int i;

    if (!rst) { //mean dump whole clk tree
        struct rstgen *n = NULL;

        if (list_is_empty(&g_rstgen_root)) {
            printf("%s: g_rstgen_root is empty.\n", __func__);
            return;
        }

        list_for_every_entry(&g_rstgen_root, n, struct rstgen, node) {
            dump_rstgen_internal(n, depth);
        }
        return ;
    }

    int refcount = rstgen_get_refcount(rst);
    //format dump
    //print_prefix(depth, "\t");
    //content
    ssdk_printf(SSDK_CRIT, "%s", rst->name);

    if (depth > (int)strlen(rst->name) / 8) {
        print_prefix(depth - strlen(rst->name) / 8, "\t");
    }

    ssdk_printf(SSDK_CRIT, "ref:%d\t release:%d\t hold:%d\n", refcount,
            rst->release_cnt, rst->hold_cnt);

    //print peripheral reference
    for (i = 0; i < MAX_PER_ID; i++) {
        char perstr[100];

        if (rstgen_test_refcount_bit(rst, i)) {
            print_prefix(1, "\t");
            snprintf(perstr, 100, "%s(%d)", module_get_per_name_by_id(i), i);
            ssdk_printf(SSDK_CRIT, "%s", perstr);

            if (depth > (int)strlen(perstr) / 8) {
                print_prefix(depth - strlen(perstr) / 8 - 1, "\t");
            }

            ssdk_printf(SSDK_CRIT, "ref:1\t\n");
        }
    }

}

void dump_rstgen()
{
    dump_rstgen_internal(NULL, 3);
}

static int cmd_dumprstgen(int argc, char *argv[])
{
    dump_rstgen();
    return 0;
}

CLI_CMD("dumprstgen", "dump rst gen", cmd_dumprstgen);
