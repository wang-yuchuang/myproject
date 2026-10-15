//#include <clkgen_hal.h>
#include "reg.h"
#include <debug.h>
#ifdef MODULE_HELPER_IN_ATF
//static int force_no_monitor=1;
#else
//static int force_no_monitor=0;
#endif

#define MODULE_HELPER_CKGEN_SEC 1
#define MODULE_HELPER_CKGEN_DISP 1
#ifndef CONFIG_MP
#define MODULE_HELPER_CKGEN_SAF 1
#endif
#define MODULE_HELPER_CKGEN_SOC 1
#define MODULE_HELPER_CKGEN_UUU 1

#include "clk_res_table.h"

#if 0
/* gating*/
static int set_ckgen_gate_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    bool result;
    void *g_handle;
    ret = hal_clock_creat_handle(&g_handle);

    if (!ret) {
        printf("do set_ckgen_gate_endis creat handle failed\n");
        return -1;
    }

    result = enable ? hal_clock_enable(g_handle,
                                       resid) : hal_clock_disable(g_handle, resid);

    if (!result) {
        ret = -1;
    }

    hal_clock_release_handle(g_handle);

    return ret;
}

/*BUS*/
static int set_ckgen_bus_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    if (!enable) {
        ctl.cg_en_a = ctl.cg_en_b = 0;
    }
    else {
        ctl.cg_en_a = ctl.cg_en_b = 1;
    }

    ret |= set_ckgen_bus_ctl(resid, &ctl, NULL);
    return ret;
}
static bool get_ckgen_bus_endis(unsigned long resid)
{
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return false;
    }

    if (ctl.a_b_sel == 0) {
        return ctl.cg_en_a == 1;
    }
    else {
        return ctl.cg_en_b == 1;
    }
}

static unsigned long get_ckgen_bus_rate(unsigned long resid,
                                        unsigned long prate, bool bymonitor)
{
    unsigned long rate;
    int ret;

    if (bymonitor) {
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_bus_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_busclk_get(g_handle, resid,
                                    mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, need monitor with 32k
            rate = hal_clock_busclk_get(g_handle, resid,
                                        mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
    }
    else {
        clkgen_bus_ctl ctl;
        int div;

        if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
            return -1;
        }

        if (ctl.a_b_sel == 0) { //A PATH
            div = (ctl.pre_div_num_a + 1) * (ctl.post_div_num + 1);
        }
        else { //B PATH
            div = (ctl.pre_div_num_b + 1) * (ctl.post_div_num + 1);
        }

        rate = prate / div;
    }

    return rate;
}
static int set_ckgen_bus_rate(unsigned long resid, unsigned long prate,
                              unsigned long freq)
{
    int ret = 0;
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    int i, j;
    int prediv_max = 1 << 3;
    int postdiv_max = 1 << 6;
    unsigned int div = 0;
    div = prate / freq;

    for (i = 1; i <= prediv_max; i++) {
        for (j = 1; j <= postdiv_max; j++) {
            if ((i * j) == (int)div) {
                goto found;
            }
        }

        //dprintf(CRITICAL, "div %d i %d, j %d\n",div, i,j);
    }

found:
    ASSERT(i <= prediv_max && j <= postdiv_max);
    //dprintf(CRITICAL, "div %d found i %d, j %d\n",div, i,j);
    ctl.pre_div_num_a = ctl.pre_div_num_b = i - 1;
    ctl.post_div_num = j - 1;
    ret |= set_ckgen_bus_ctl(resid, &ctl, NULL);
    return ret;
}

static int get_ckgen_bus_parent(unsigned long resid)
{
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    return (ctl.a_b_sel == 0) ? ctl.src_sel_a : ctl.src_sel_b;
}
static int set_ckgen_bus_parent(unsigned long resid, int p_index)
{
    int ret = 0;
    clkgen_bus_ctl ctl;

    if (get_ckgen_bus_ctl(resid, &ctl, NULL) < 0) {
        return -1;
    }

    ctl.src_sel_a = ctl.src_sel_b = p_index;
    ret |= set_ckgen_bus_ctl(resid, &ctl, NULL);
    return ret;
}

/*CORE */

static int set_ckgen_core_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (!enable) {
        ctl.cg_en_a = 0;
        ctl.cg_en_b = 0;
    }
    else {
        if (ctl.a_b_sel == 0) {
            ctl.cg_en_a = 1;
        }
        else {
            ctl.cg_en_b = 1;
        }
    }

    ret |= set_ckgen_core_ctl(resid, &ctl);
    return ret;
}
static bool get_ckgen_core_endis(unsigned long resid)
{
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return false;
    }

    if (ctl.a_b_sel == 0) {
        return ctl.cg_en_a == 1;
    }
    else {
        return ctl.cg_en_b == 1;
    }
}

static unsigned long get_ckgen_core_rate(unsigned long resid,
        unsigned long prate, bool bymonitor)
{
    unsigned long rate;
    int ret;

    if (bymonitor) {
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_core_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_coreclk_get(g_handle, resid,
                                     mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, need monitor with 32k
            rate = hal_clock_coreclk_get(g_handle, resid,
                                         mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
    }
    else {
        clkgen_core_ctl ctl;
        int div;

        if (get_ckgen_core_ctl(resid, &ctl) < 0) {
            return -1;
        }

        div = ctl.post_div_num + 1;
        rate = prate / div;
    }

    return rate;
}

static int set_ckgen_core_rate(unsigned long resid, unsigned long prate,
                               unsigned long freq)
{
    int ret = 0;
    clkgen_core_ctl ctl;
    unsigned int postdiv_max = 1 << 6;
    unsigned int div = 0;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    div = prate / freq;
    ASSERT(div <= postdiv_max);
    ctl.post_div_num = div - 1;
    ret |= set_ckgen_core_ctl(resid, &ctl);
    return ret;
}

static int get_ckgen_core_parent(unsigned long resid)
{
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    return (ctl.a_b_sel == 0) ? ctl.src_sel_a : ctl.src_sel_b;
}
static int set_ckgen_core_parent(unsigned long resid, int p_index)
{
    int ret = 0;
    clkgen_core_ctl ctl;

    if (get_ckgen_core_ctl(resid, &ctl) < 0) {
        return -1;
    }

    ctl.src_sel_a = ctl.src_sel_b = p_index;
    ret |= set_ckgen_core_ctl(resid, &ctl);
    return ret;
}



/*IP*/

static int set_ckgen_ip_endis(unsigned long resid, bool enable)
{
    int ret = 0;
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (!enable) {
        ctl.cg_en = 0;
    }
    else {
        ctl.cg_en = 1;
    }

    ret |= set_ckgen_ip_ctl(resid, &ctl);
    return ret;
}
static bool get_ckgen_ip_endis(unsigned long resid)
{
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return false;
    }

    return ctl.cg_en == 1;
}

static unsigned long get_ckgen_ip_rate(unsigned long resid,
                                       unsigned long prate, bool bymonitor)
{
    unsigned long rate;
    int ret;

    if (bymonitor) {
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_ip_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_ipclk_get(g_handle, resid,
                                   mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, try monitor with 32k
            rate = hal_clock_ipclk_get(g_handle, resid,
                                       mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
    }
    else {
        clkgen_ip_ctl ctl;
        int div;

        if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
            return -1;
        }

        div = (ctl.pre_div_num + 1) * (ctl.post_div_num + 1);
        rate = prate / div;
    }

    return rate;
}

static int set_ckgen_ip_rate(unsigned long resid, unsigned long prate,
                             unsigned long freq)
{
    int ret = 0;
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    int i, j;
    int prediv_max = 1 << 3;
    int postdiv_max = 1 << 6;
    unsigned int div = 0;
    div = prate / freq;

    for (i = 1; i <= prediv_max; i++) {
        for (j = 1; j <= postdiv_max; j++) {
            if ((i * j) == (int)div) {
                goto found;
            }
        }

        //dprintf(CRITICAL, "div %d i %d, j %d\n",div, i,j);
    }

    ASSERT(i <= prediv_max && j <= postdiv_max);
found:
    //dprintf(CRITICAL, "div %d found i %d, j %d\n",div, i,j);
    ctl.pre_div_num = i - 1;
    ctl.post_div_num = j - 1;
    ret |= set_ckgen_ip_ctl(resid, &ctl);
    return ret;
}

static int get_ckgen_ip_parent(unsigned long resid)
{
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    return ctl.src_sel;
}
static int set_ckgen_ip_parent(unsigned long resid, int p_index)
{
    int ret = 0;
    clkgen_ip_ctl ctl;

    if (get_ckgen_ip_ctl(resid, &ctl) < 0) {
        return -1;
    }

    ctl.src_sel = p_index;
    ret |= set_ckgen_ip_ctl(resid, &ctl);
    return ret;
}

/*UUU*/
static unsigned long get_ckgen_uuu_rate(unsigned long resid, int uuu_type,
                                        unsigned long prate, bool bymonitor);

static int set_ckgen_uuu_endis(unsigned long resid, bool enable)
{
    //int ret;
    //uuu always enabled
    return 0;
}

static bool get_ckgen_uuu_endis(unsigned long resid, int uuu_type)
{
    if (force_no_monitor) return true;

    //int ret;
    if (uuu_type == UUU_SEL0
            || uuu_type == UUU_SEL1) { //dummy node, return true.
        return true;
    }

    unsigned long freq = get_ckgen_uuu_rate(resid, uuu_type, 0, true);

    if (freq) {
        return true;
    }
    else {
        return false;
    }
}


static unsigned long get_ckgen_uuu_rate(unsigned long resid, int uuu_type,
                                        unsigned long prate, bool bymonitor)
{
    int ret;
    clkgen_uuu_ctl ctl;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0
            || uuu_type == UUU_SEL1) { //dummy node, return parent rate.
        return prate;
    }

    if (bymonitor) {
        unsigned long rate;
        void *g_handle;
        ret = hal_clock_creat_handle(&g_handle);

        if (!ret) {
            printf("do get_ckgen_uuu_rate creat handle failed\n");
            return 0;
        }

        rate = hal_clock_uuuclk_get(g_handle, resid,
                                    mon_ref_clk_24M, 0);

        if (rate == 0) //maybe it's too slow, try monitor with 32k
            rate = hal_clock_uuuclk_get(g_handle, resid,
                                        mon_ref_clk_32K, 0);

        hal_clock_release_handle(g_handle);
        //the monitor point is after the M, so...
        prate = rate * (ctl.m_div + 1);
    }

    if (uuu_type == UUU_M) {
        return prate / (ctl.m_div + 1);
    }
    else if (uuu_type == UUU_N) {
        return prate / (ctl.n_div + 1);
    }
    else if (uuu_type == UUU_P) {
        return prate / (ctl.p_div + 1);
    }
    else if (uuu_type == UUU_Q) {
        return prate / (ctl.q_div + 1);
    }

    return 0;
}

static int set_ckgen_uuu_rate(unsigned long resid, int uuu_type,
                              unsigned long prate, unsigned long freq)
{
    int ret = 0;
    clkgen_uuu_ctl ctl;
    unsigned int maxdiv = 0;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0 || uuu_type == UUU_SEL1) { //dummy node.
        return 0;
    }
    else if (uuu_type >= UUU_M && uuu_type <= UUU_Q) {
        maxdiv = 1 << 4;
    }
    else {
        //could not be here
        ASSERT(0);
    }

    unsigned int div = 0;
    div = prate / freq;
    ASSERT(div <= maxdiv);

    //dprintf(CRITICAL, "uuu div %d found.\n",div);
    if (uuu_type == UUU_M) {
        ctl.m_div = div - 1;
    }
    else if (uuu_type == UUU_N) {
        ctl.n_div = div - 1;
    }
    else if (uuu_type == UUU_P) {
        ctl.p_div = div - 1;
    }
    else if (uuu_type == UUU_Q) {
        ctl.q_div = div - 1;
    }

    ret |= set_ckgen_uuu_ctl(resid, &ctl);
    return ret;
}

static int get_ckgen_uuu_parent(unsigned long resid, int uuu_type)
{
    clkgen_uuu_ctl ctl;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0) {
        return ctl.uuu_sel0;
    }
    else if (uuu_type >= UUU_M
             && uuu_type <= UUU_Q) {//mnpq only have one parent.
        return 0;
    }
    else {  // sel1
        return ctl.uuu_sel1;
    }
}

static int set_ckgen_uuu_parent(unsigned long resid, int uuu_type,
                                int p_index)
{
    int ret = 0;
    clkgen_uuu_ctl ctl;

    if (get_ckgen_uuu_ctl(resid, &ctl) < 0) {
        return -1;
    }

    if (uuu_type == UUU_SEL0) {
        ctl.uuu_sel0 = p_index;
    }
    else if (uuu_type >= UUU_M
             && uuu_type <= UUU_Q) {//mnpq only have one parent.
        return 0;
    }
    else {  // sel1
        ctl.uuu_sel1 = p_index;
    }

    ret |= set_ckgen_uuu_ctl(resid, &ctl);
    return ret;
}


static void ckgen_clk_gating_disable(unsigned int ckgen_base, unsigned int idx,
                              unsigned int value)
{
    const uint32_t gating_en_offset = 0x400 << 10;
    addr_t addr = ckgen_base + gating_en_offset + ((idx * 4) << 10);
    uint32_t rval = readl(addr);
    ssdk_printf(SSDK_CRIT, "addr: 0x%lx, srcval: 0x%x\n", addr, rval);
    writel(((rval & (~(0x1 << 1))) | (value << 1)), addr);
    rval = readl(addr);
    ssdk_printf(SSDK_CRIT, "addr: 0x%lx, newval: 0x%x\n", addr, rval);
}

static void clk_disable_gating(const char *type, int id, int val)
{
    unsigned int base = 0x0;

    if (id >= 128) {
        ssdk_printf(SSDK_CRIT, "invalid id: %d\n", id);
        return;
    }

    if (strncmp(type, "soc", 3) == 0) {
        base = APB_CKGEN_SOC_BASE;
    }
    else if (strncmp(type, "sec", 3) == 0) {
        base = APB_CKGEN_SEC_BASE;
    }
    else if (strncmp(type, "saf", 3) == 0) {
        base = APB_CKGEN_SAF_BASE;
    }
    else if (strncmp(type, "disp", 4) == 0) {
        base = APB_CKGEN_DISP_BASE;
    }
    else {
        ssdk_printf(SSDK_CRIT, "invalid type: %s\n", type);
        return;
    }

    ssdk_printf(SSDK_CRIT, "type: %s, id: %d, val:%d\n", type, id, val);
    ckgen_clk_gating_disable(base, id, val);
}
#endif

static unsigned long get_ckgen_bus_ip_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i, j;
    int prediv_max = 1 << 3;
    int postdiv_max = 1 << 6;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = prediv_max * postdiv_max, mindiv, bestdiv;
    struct clk *p = get_clk_from_all_clks(clk->parents[pindex]);
    maxdiv = MIN((int)clk->maxdiv, prediv_max * postdiv_max);
    maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
    mindiv = MAX((int)clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = 1; i <= prediv_max; i++) {
        for (j = 1; j <= postdiv_max; j++) {
            if (((i * j) < (int)mindiv) || ((i * j) > (int)maxdiv)) {
                break;
            }

            prate_cur = *prate;
            rate = res_clk_div_round_rate(clk, p, i * j, &prate_cur, freq);

            if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

            diff = abs_clk(freq, prate_cur / (i * j));

            if (diff == 0) {
                *prate = prate_cur;
                return freq;
            }

            if (diff < bestratediff) {
                bestratediff = diff;
                bestprate = prate_cur;
                bestdiv = (i * j);
            }
        }
    }

    *prate = bestprate;
    bestrate = (bestprate) / (bestdiv);
    return bestrate;
}

static unsigned long get_ckgen_core_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    int postdiv_max = 1 << 6;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned int maxdiv, mindiv, bestdiv;
    unsigned long diff;
    struct clk *p = get_clk_from_all_clks(clk->parents[pindex]);
    maxdiv = MIN((int)clk->maxdiv, postdiv_max);
    maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
    mindiv = MAX((int)clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = (int)mindiv; i <= (int)maxdiv; i++) {
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / (i));

        if (diff == 0) {
            *prate = prate_cur;
            return freq;
        }

        if (diff < bestratediff) {
            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = i;
        }
    }

    *prate = bestprate;
    bestrate = (bestprate) / (bestdiv);
    return bestrate;
}

static unsigned long get_ckgen_uuu_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned int mindiv = 1, maxdiv = 0, bestdiv = 0;
    unsigned long diff;
    int uuu_type = clk->uuu.uuu_type;
    struct clk *p = get_clk_from_all_clks(clk->parents[pindex]);

    if (uuu_type == UUU_SEL0 || uuu_type == UUU_SEL1) {
        struct clk_request req;
        req.request = freq;
        bestrate = res_clk_round_rate(get_clk_from_all_clks(clk->parents[pindex]), &req);

        if (!res_clk_is_valid_round_rate(clk, pindex, bestrate, bestrate)) {
            //dprintf(CRITICAL,"uuusel%d %s p %d bestrate %lu refcnt %d not valid\n", uuu_type, clk->name, pindex, bestrate, clk_get_refcount(clk));
            bestrate = *prate;
        }
        else {
            //dprintf(CRITICAL,"uuusel%d %s p %d bestrate %lu refcnt %d\n", uuu_type, clk->name, pindex, bestrate, clk_get_refcount(clk, true));
        }

        *prate = bestrate;
        return bestrate;
    }
    else if (uuu_type >= UUU_M && uuu_type <= UUU_Q) {
        maxdiv = MIN((int)clk->maxdiv, 1 << 4);
        maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
        mindiv = MAX((int)clk->mindiv, 1);
        maxdiv = MAX(maxdiv, mindiv);
        //initial value of bestdiv is the fix div or current div.
        bestdiv = mindiv;
    }
    else {
        //could not be here
        ASSERT(0);
    }

    for (i = (int)mindiv; i <= (int)maxdiv; i++) {
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        //dprintf(CRITICAL, "ckgen uuu %s old p %lu req %lu round get p %lu, rate %lu div %d\n", clk->name, *prate, freq, prate_cur, rate, i);
        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / (i));

        //dprintf(CRITICAL, "ckgen uuu %s old p %lu req %lu round get p %lu ,diff %lu div %d\n", clk->name, *prate, freq, prate_cur, diff, i);
        if (diff == 0) {
            *prate = prate_cur;
            return freq;
        }

        if (diff < bestratediff) {
            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = i;
        }
    }

    *prate = bestprate;
    bestrate = bestprate / bestdiv;
    //dprintf(CRITICAL, "ckgen uuu %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}
