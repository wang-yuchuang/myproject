#include <bits.h>
/* Division number limits. */
#define PFPLL_REFDIV_MIN                1
#define PFPLL_REFDIV_MAX                63

#define PFPLL_POSTDIV_MIN               1
#define PFPLL_POSTDIV_MAX               7

#define PFPLL_FBDIV_INTEGER_MIN         16
#define PFPLL_FBDIV_INTEGER_MAX         640
#define PFPLL_FBDIV_FRAC_MIN            20
#define PFPLL_FBDIV_FRAC_MAX            320

#define PFPLL_FRAC_MIN                  0
#define PFPLL_FRAC_MAX                  (int)BIT_MASK(24)
#define PLL_SIGNOFF                     2700000000UL

#define MAX_DOT_LEN 9
struct complex_value {
    unsigned long intv;
    bool isfloat;
    uint8_t f[MAX_DOT_LEN];
};
#if 0
struct complex_value *complex_div(unsigned long a, unsigned long b,
                                  struct complex_value *r)
{
    int i, v;
    unsigned long mod = 0;
    v = a / b;
    r->intv = v;
    mod = (a - v * b);

    if (mod == 0) { //int
        r->isfloat = false;
        return r;
    }

    r->isfloat = true;

    for (i = 0; i < MAX_DOT_LEN; i++) {
        v = mod * 10 / b;
        r->f[i] = v;
        mod = (mod * 10) - (v * b);

        if (mod == 0)
            break;
    }

    i++;

    for (; i < MAX_DOT_LEN; i++)
        r->f[i] = 0;

    return r;
}
static unsigned long ten(int c)
{
    int i = 0;
    unsigned long v;

    if (c == 0)
        return 1;

    v = 10;

    for (i = 0; i < c - 1; i++)
        v = v * 10;

    return v;
}
unsigned long complex_mul(unsigned long a, const struct complex_value *r)
{
    int i;
    unsigned long temp = a * r->intv;
    unsigned long mod = 0, h, l, mm;

    if (a == 0) return 0;

    if (!r->isfloat)
        return temp;

    //frac
    for (i = 0; i < MAX_DOT_LEN; i++) {
        h = a * r->f[i] / ten(i + 1);
        temp += h;
        l = a * r->f[i] - h * ten(i + 1);
        mod += l * ten(MAX_DOT_LEN - i - 1);
        mm = mod / ten(MAX_DOT_LEN);

        if (mm > 0) {
            temp += mm;
            mod = mod - mm * ten(MAX_DOT_LEN);
        }
    }

    return temp;
}

const struct complex_value cv65536div46875 = { //1.398101333333333
    .intv = 1,
    .isfloat = true,
    .f = {3, 9, 8, 1, 0, 1, 3, 3, 4},
};
const struct complex_value cv32768div46875 = { //0.6990506666666667
    .intv = 0,
    .isfloat = true,
    .f = {6, 9, 9, 0, 5, 0, 6, 6, 7},
};
const struct complex_value cv715255737 = { //0.7152557373046875
    .intv = 0,
    .isfloat = true,
    .f = {7, 1, 5, 2, 5, 5, 7, 3, 8},
};
const struct complex_value cv1dot4305114746 = { //1.430511474609375
    .intv = 1,
    .isfloat = true,
    .f = {4, 3, 0, 5, 1, 1, 4, 7, 5},
};

static unsigned long calculate_pll_rate(pll_config_t *config, int plldiv)
{
    unsigned long freq = 0;
    unsigned long freql = 0;
    unsigned long freqh = config->fbdiv * 1000000 * (24 /  config->refdiv);

    if (config->integer == 0) {
        /* frac * (1000000/2^24)   * (24/refdiv)*/
        if (config->refdiv == 2) {
            /* 0.7152557373046875 */
            freql = complex_mul(config->frac, &cv715255737);
        }
        else {
            /* 1.430511474609375 */
            freql = complex_mul(config->frac, &cv1dot4305114746);
        }
    }

    freq = freqh + freql;

    if (plldiv == PLL_ROOT) {//root
        freq = freq / config->postdiv[0] / config->postdiv[1];
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        freq = freq / config->out_div[plldiv - 1];
    }
    else if (plldiv == PLL_DUMMY_ROOT) {
        return freq;
    }
    else {
        freq = 0;
    }

    return freq;
}

const pll_config_t *find_config_in_database(struct clk *clk, int plldiv,
        unsigned long freq)
{
    int i;
    int size = clk->pll.ratetable_size;

    for (i = 0; i < size; i++)
        if (calculate_pll_rate(clk->pll.ratetable[i], plldiv) == freq) {
            return clk->pll.ratetable[i];
        }

    return NULL;
}

static int set_pll_endis(unsigned long resid, int plldiv, bool isenable)
{
//TODO
    return 0;
}

static int set_pll_rate(struct clk *clk,
                        unsigned long prate, unsigned long rate)
{
    const pll_config_t *config;
    pll_config_t cur = {0};
    int div;
    int plldiv = clk->pll.plldiv;
    pll_handle_t pll =  hal_pll_create_handle(clk->resid);

    if (pll == (pll_handle_t)0) {
        return 0;
    }

    hal_pll_get_config(pll, &cur);
    stuff_valid_value(&cur);

    if (plldiv == PLL_DUMMY_ROOT) { //will ignore the prate
        if (clk->pll.moreprecise) {
            config = &clk->pll.config;
            //ASSERT(rate == calculate_pll_rate(&clk->pll.config, PLL_DUMMY_ROOT));
        }
        else {
            config = find_config_in_database(clk, plldiv, rate);
        }

        ASSERT(config != NULL);
        cur.integer = config->integer;
        cur.frac = config->frac;

        //cur.spread_spectrum = config->spread_spectrum;
        if (cur.spread_spectrum) {
            //cur.downspread = config->downspread;
            //cur.spread = config->spread;
            if (cur.integer) {
                cur.integer = false;
                cur.frac = 0;
            }
        }

        cur.refdiv = config->refdiv;
        cur.fbdiv = config->fbdiv;
    }
    else if (plldiv == PLL_ROOT) {
        int i, j;
        int post0div_max = 7;
        int post1div_max = 7;
        unsigned int div = 0;
        div = prate / rate;

        for (i = 1; i <= post0div_max; i++) {
            for (j = 1; j <= post1div_max; j++) {
                if (i < j) { continue; } //post0 >= post1

                if ((i * j) == (int)div) {
                    goto found;
                }
            }
        }

        ASSERT(i <= post0div_max && j <= post1div_max);
        ASSERT(i >= j);
found:
        cur.postdiv[0] = i;
        cur.postdiv[1] = j;
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        div = prate / rate;
        cur.out_div[plldiv - 1] = div;
    }

    hal_pll_config(pll, &cur);
    hal_pll_delete_handle(pll);
    return 0;
}
static bool is_valid_pll_config(struct clk *clk, pll_config_t *conf)
{
    if (clk->pll.plldiv == PLL_DUMMY_ROOT) { // check the pll limit
        //fvco 800MHZ~3200MHZ
        unsigned long fvco;
        unsigned pfd;
        fvco = calculate_pll_rate(conf, PLL_DUMMY_ROOT);

        if (fvco < 800000000 || fvco > 3200000000) {
            return false;
        }

        //pfd >10Mhz in fac mode. and >5Mhz in int mode
        pfd = 24000000 / conf->refdiv;

        if (conf->integer && pfd <= 5000000) {
            return false;
        }

        if (conf->integer == 0 && pfd <= 10000000) {
            return false;
        }

        if (conf->integer == 0 && conf->frac >= 16777216) {
            return false;
        }

        //signoff limit
        if (fvco > PLL_SIGNOFF) {
            return false;
        }
    }

    return true;
}
/* . 6x*/
static unsigned long get_int(unsigned long freq, int refdiv)
{
    return (freq / 24) * refdiv;
}

static unsigned long get_pll_root_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i, j;
    int postdiv0div_max = 7;
    int postdiv1div_max = 7;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    struct clk *p = NULL;

    if (pindex != -1) {
        p = get_clk_from_all_clks(clk->parents[pindex]);
    }

    maxdiv = MIN((int)clk->maxdiv, postdiv0div_max * postdiv1div_max);
    maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
    mindiv = MAX((int)clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = 1; i <= postdiv0div_max; i++) {
        for (j = 1; j <= postdiv1div_max && j <= i; j++) {
            if ((i * j) < (int)mindiv) {
                continue;
            }

            if ((i * j) > (int)maxdiv) {
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
    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}


static unsigned long get_pll_div_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    int div_max = 16;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    unsigned int div = (*prate) / freq;
    struct clk *p = NULL;

    if (pindex != -1) {
        p =  get_clk_from_all_clks(clk->parents[pindex]);
    }

    maxdiv = MIN((int)clk->maxdiv, div_max);
    maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
    mindiv = MAX((int)clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;
    prate_cur = *prate;

    //prefer current
    if (*prate != 0) {
        if (div >= mindiv && div <= maxdiv) {
            diff = abs_clk(*prate, div * freq);

            if (diff == 0) {
                *prate = prate_cur;
                return freq;
            }

            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = div;
        }
    }

    for (i = mindiv; i <= (int)maxdiv; i++) {
        //if (i == (int)div)continue;
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / i);

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
    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}

unsigned long get_pll_dummy_root_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int size = clk->pll.ratetable_size;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, rate;
    unsigned long diff;
    ASSERT(clk->pll.plldiv == PLL_DUMMY_ROOT);

    if (clk->pll.moreprecise) {
        pll_config_t conf = {0};
        int fbdiv = 1, refdiv = 1;
        int refdiv_max;
        int fbdiv_max, fbdiv_min;
        pll_handle_t pll =  hal_pll_create_handle(clk->resid);

        if (pll == (pll_handle_t)0) {
            return 0;
        }

        hal_pll_get_config(pll, &clk->pll.config);
        hal_pll_delete_handle(pll);
        stuff_valid_value(&clk->pll.config);
        conf.integer = clk->pll.config.integer;
        conf.frac = clk->pll.config.frac;
        conf.spread_spectrum = clk->pll.config.spread_spectrum;
        conf.downspread = clk->pll.config.downspread;
        conf.spread = clk->pll.config.spread;
        bestrate = calculate_pll_rate(&clk->pll.config, clk->pll.plldiv);
        freq = MIN(freq, 3200000000UL);
        freq = MIN(freq, PLL_SIGNOFF);
        freq = MAX(freq, 800000000UL);
        freq = MAX(freq, clk->minfreq);
        freq = MIN(freq, clk->maxfreq);

        //try integer mode
        for (refdiv = 4; refdiv >= 1; refdiv--) {
            if ((freq / (24000000 / refdiv)) * (24000000 / refdiv) ==
                    freq) { //integer
                conf.integer = 1;
                conf.frac = 0;
                conf.refdiv = refdiv;
                conf.fbdiv = (freq / (24000000 / refdiv));

                if (conf.fbdiv < 16 || conf.fbdiv > 640) continue;

                if (!is_valid_pll_config(clk, &conf)) continue;

                rate = calculate_pll_rate(&conf, clk->pll.plldiv);

                if (!res_clk_is_valid_round_rate(clk, -1, rate, 0)) continue;

                //printf("bestrate %lu freq %lu frac %u\n", rate, freq, conf.frac);
                clk->pll.config = conf;
                return rate;
            }
        }

        conf.integer = clk->pll.config.integer;
        conf.frac = clk->pll.config.frac;
        conf.spread_spectrum = clk->pll.config.spread_spectrum;
        conf.downspread = clk->pll.config.downspread;
        conf.spread = clk->pll.config.spread;

        if (conf.integer) {
            fbdiv_min = 16;
            fbdiv_max = 640;
            refdiv_max = 4;
        }
        else {
            fbdiv_min = 20;
            fbdiv_max = 320;
            refdiv_max = 2;
        }

        for (refdiv = refdiv_max; refdiv >= 1; refdiv--) {
            int fbdivmin, fbdivmax;
            int fbtmp = get_int(freq, refdiv) / 1000000;
            fbdivmin = MAX(fbtmp, fbdiv_min);
            fbdivmax = MIN(fbtmp, fbdiv_max);

            for (fbdiv = fbdivmin; fbdiv <= fbdivmax; fbdiv++) {
                conf.fbdiv = fbdiv;
                conf.refdiv = refdiv;
                unsigned long freqh = conf.fbdiv * (24000000 / conf.refdiv);
                unsigned long freql = freq - freqh;

                /* frac = freql * conf.refdiv/24000000 * 16777216 */
                if (conf.refdiv ==
                        2) {/* freql * 65536/46875 ------------1.398101333333333*/
                    conf.frac = complex_mul(freql, &cv65536div46875);
                }
                else {   /* freql * 32768/46875 ------------ 0.6990506666666667*/
                    conf.frac = complex_mul(freql, &cv32768div46875);
                }

                if (conf.frac) {
                    conf.integer = 0;
                }

                if (!is_valid_pll_config(clk, &conf)) { continue; }

                rate = calculate_pll_rate(&conf, clk->pll.plldiv);

                if (!res_clk_is_valid_round_rate(clk, -1, rate, 0)) { continue; }

                diff = abs_clk(freq, rate);

                if (conf.frac && diff < 5) {
                    if (rate < freq)
                        conf.frac += (conf.refdiv == 2 ? complex_mul(diff,
                                      &cv65536div46875) : complex_mul(diff, &cv32768div46875));
                    else
                        conf.frac -= (conf.refdiv == 2 ? complex_mul(diff,
                                      &cv65536div46875) : complex_mul(diff, &cv32768div46875));

                    clk->pll.config = conf;
                    return freq;
                }
                else {
                    clk->pll.config = conf;
                    return rate;
                }
            }
        }

        return bestrate;
    }
    else {
        int i;

        for (i = 0; i < size; i++) {
            rate = calculate_pll_rate(clk->pll.ratetable[i], clk->pll.plldiv);

            //dprintf(CRITICAL, "pll %s req %lu round get rate %lu index %d\n", clk->name, freq, rate, i);
            if (!res_clk_is_valid_round_rate(clk, -1, rate, 0)) { continue; }

            diff = abs_clk(freq, rate);

            //dprintf(CRITICAL, "pll %s req %lu round get rate %lu index %d diff %lu \n", clk->name, freq, rate, i, diff);
            if (diff == 0) {
                return freq;
            }

            if (diff < bestratediff) {
                bestratediff = diff;
                bestrate = rate;
            }
        }
    }

    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, *prate);
    return bestrate;
}

static unsigned long get_pll_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int plldiv = clk->pll.plldiv;

    if (plldiv == PLL_DUMMY_ROOT) { // ignore prate
        return get_pll_dummy_root_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (plldiv == PLL_ROOT) {
        return get_pll_root_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        return get_pll_div_round_rate_clk(clk, pindex, prate, freq);
    }
    else {
        //could not be here
        ASSERT(0);
    }

    return 0;
}

static unsigned long get_pll_rate(struct clk *clk,
                                  unsigned long prate, bool bymonitor)
{
    pll_handle_t pll;
    unsigned long rate = 0;
    pll_config_t conf = {0};
    int plldiv = clk->pll.plldiv;
    //int ret=0;
    pll =  hal_pll_create_handle(clk->resid);

    if (pll == (pll_handle_t)0) {
        return 0;
    }

    hal_pll_get_config(pll, &conf);

    if (bymonitor) {
        rate = calculate_pll_rate(&conf, plldiv);
    }
    else {
        if (plldiv == PLL_DUMMY_ROOT) {
            rate = calculate_pll_rate(&conf, plldiv);
            //rate = (rate / 1000) * 1000;
        }
        else if (plldiv == PLL_ROOT) {
            rate = prate / conf.postdiv[0] / conf.postdiv[1];
        }
        else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
            if (conf.out_div[plldiv - 1] != 0) {
                rate = prate / conf.out_div[plldiv - 1];
            }
            else {
                rate = 0;
            }
        }
    }

    hal_pll_delete_handle(pll);
    return rate;
}
#endif

static unsigned long get_pll_root_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i, j;
    int postdiv0div_max = 7;
    int postdiv1div_max = 7;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    struct clk *p = NULL;

    if (pindex != -1) {
        p = get_clk_from_all_clks(clk->parents[pindex]);
    }

    maxdiv = MIN((int)clk->maxdiv, postdiv0div_max * postdiv1div_max);
    maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
    mindiv = MAX((int)clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;

    for (i = 1; i <= postdiv0div_max; i++) {
        for (j = 1; j <= postdiv1div_max && j <= i; j++) {
            if ((i * j) < (int)mindiv) {
                continue;
            }

            if ((i * j) > (int)maxdiv) {
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
    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}

static unsigned long get_pll_div_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int i;
    int div_max = 16;
    unsigned long bestrate = 0, bestratediff = UINT32_MAX, bestprate = *prate,
                  prate_cur = *prate, rate;
    unsigned long diff;
    unsigned int maxdiv = 0, mindiv, bestdiv;
    unsigned int div = (*prate) / freq;
    struct clk *p = NULL;

    if (pindex != -1) {
        p =  get_clk_from_all_clks(clk->parents[pindex]);
    }

    maxdiv = MIN((int)clk->maxdiv, div_max);
    maxdiv = MIN(UINT32_MAX / freq, (unsigned long)maxdiv);
    mindiv = MAX((int)clk->mindiv, 1);
    maxdiv = MAX(maxdiv, mindiv);
    //initial value of bestdiv is the fix div or current div.
    bestdiv = mindiv;
    prate_cur = *prate;

    //prefer current
    if (*prate != 0) {
        if (div >= mindiv && div <= maxdiv) {
            diff = abs_clk(*prate, div * freq);

            if (diff == 0) {
                *prate = prate_cur;
                return freq;
            }

            bestratediff = diff;
            bestprate = prate_cur;
            bestdiv = div;
        }
    }

    for (i = mindiv; i <= (int)maxdiv; i++) {
        //if (i == (int)div)continue;
        prate_cur = *prate;
        rate = res_clk_div_round_rate(clk, p, i, &prate_cur, freq);

        if (!res_clk_is_valid_round_rate(clk, pindex, rate, prate_cur)) { continue; }

        diff = abs_clk(freq, prate_cur / i);

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
    //dprintf(CRITICAL, "pll %s round get best %lu prate %lu\n", clk->name, bestrate, bestprate);
    return bestrate;
}

static unsigned long get_pll_round_rate_clk(struct clk *clk,
        int pindex, unsigned long *prate, unsigned long freq)
{
    int plldiv = clk->pll.plldiv;

    if (plldiv == PLL_DUMMY_ROOT) { // ignore prate
        //return get_pll_dummy_root_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (plldiv == PLL_ROOT) {
        return get_pll_root_round_rate_clk(clk, pindex, prate, freq);
    }
    else if (plldiv >= PLL_DIVA && plldiv <= PLL_DIVD) {
        return get_pll_div_round_rate_clk(clk, pindex, prate, freq);
    }
    else {
        //could not be here
        ASSERT(0);
    }

    return 0;
}
