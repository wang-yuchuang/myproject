/*
* sdrv_asw_lut.c
*
* Copyright (c) 2019 Semidrive Semiconductor.
* All rights reserved.
*
* Description:
*
* Revision History:
* -----------------
* 011, 12/08/2021 BILLY create this file
*/

#include <debug.h>
#include <stdlib.h>
#include <string.h>
#include <reg.h>

#include <driver/taishan/asw/asw_log.h>
#include <driver/taishan/asw/asw_lut_hvk.h>
#include <arch/cache.h>

static int m_parameter_calc_alpha(int lut_size, float *alpha)
{
    for (int i = 0; i < lut_size; i++) {
        if ((i < 2) || (i >= (lut_size -2)))
            alpha[i] = 1.0;
        else if (i == 2)
            alpha[i] = 0.25;
        else
            alpha[i] = 1.0 / (4.0 -alpha[i -1]);
    }

    return 0;
}

static int m_parameter_calc_1d(float *lut_ld, float *m_param_ld, float *alpha, int lut_size, bool is_rt1)
{
    float *delta, *beta;
    float c_one_third = 1.0 / 3.0;
    float c_one_sixth = 1.0 / 6.0;

    delta = (float *)malloc(sizeof(float) * lut_size);
    if (!delta) {
        ASW_LOG_ERR("malloc delta failed\n");
        return -1;
    }
    beta = (float *)malloc(sizeof(float) * lut_size);
    if (!beta) {
        ASW_LOG_ERR("malloc beta failed\n");
        return -1;
    }

    // -1  2 -1  0  0  ...  0  0  0		0
    //  1  4  1  0  0  ...  0  0  0		6 * (y[2] + y[0] - 2 * y[1])
    //  0  1  4  1  0  ...  0  0  0		6 * (y[3] + y[1] - 2 * y[2])
    //  0  0  1  4  1  ...  0  0  0		6 * (y[4] + y[2] - 2 * y[3])
    //  .................................
    //  0  0  0  0  0  ...  1  4  1		6 * (y[n-1] + y[n-3] - 2 * y[n-2])
    //  0  0  0  0  0  ... -1  2 -1		0

    for (int i = 0; i < lut_size; i++) {
        if ((i == 0) || (i == (lut_size -1))) {
            delta[i] = 0.0;
        } else {
            if (!is_rt1) {
                delta[i] = 6.0 * (lut_ld[i + 1] + lut_ld[i - 1] - 2.0 * lut_ld[i]);
            } else {
                delta[i] = lut_ld[i + 1] + lut_ld[i - 1];
                delta[i] = fma(-2.0, lut_ld[i], delta[i]);
                delta[i] = 6.0 * delta[i];
            }

        }
    }

    for (int i = 0; i < lut_size; i++) {
        if (i == 0) {
            beta[i] = delta[1] * c_one_third;
        } else if (i == 1) {
            beta[i] = delta[1] * c_one_sixth;
        } else if (i == (lut_size -2)) {
            beta[i] = delta[lut_size - 2] * c_one_sixth;
        } else if (i == (lut_size -1)) {
            beta[i] = delta[lut_size - 2] * c_one_third;
        } else {
            if (!is_rt1) {
                beta[i] = (delta[i] - beta[i - 1]) * alpha[i];
            } else {
                beta[i] = delta[i] * alpha[i];
                beta[i] = fma(-beta[i - 1], alpha[i], beta[i]);
            }
        }
    }

    for (int i = lut_size -2; i >= 0; i--) {
        if (i == 0) {
            m_param_ld[i] = beta[i] - m_param_ld[i + 2];
        } else if (i == 1) {
            m_param_ld[i] = beta[i];
        } else if (i == (lut_size -2)) {
            m_param_ld[i] = beta[i];
        } else {
            if (!is_rt1) {
                m_param_ld[i] = beta[i] - m_param_ld[i + 1] * alpha[i];
            } else {
                m_param_ld[i] = fma(-m_param_ld[i + 1], alpha[i], beta[i]);
            }
        }
    }
    m_param_ld[lut_size - 1] = beta[lut_size - 1] - m_param_ld[lut_size - 3];

    free(delta);
    free(beta);

    return 0;
}
static float hvk_pos_calc(float x, float y,struct hvkt_param *hvkt, bool calc_x, int c_sel)
{
    float pa, pb, pc, pd, pe, pf;
    float x_mag, y_mag;

    struct hvkt_fabc *fabc = hvkt->fabc;

    pa = fabc->fc0[c_sel] * x - fabc->fa0[c_sel];
    pb = fabc->fc1[c_sel] * x - fabc->fa1[c_sel];
    pc = fabc->fa2[c_sel] - fabc->fc2[c_sel] * x;

    pd = fabc->fc0[c_sel] * y - fabc->fb0[c_sel];
    pe = fabc->fc1[c_sel] * y - fabc->fb1[c_sel];
    pf = fabc->fb2[c_sel] - fabc->fc2[c_sel] * y;

    if (calc_x) {
        if (hvkt->fw_map)
            x_mag = (fabc->fa0[c_sel] * x + fabc->fa1[c_sel] * y + fabc->fa2[c_sel])
                    / (fabc->fc0[c_sel] * x + fabc->fc1[c_sel] * y + fabc->fc2[c_sel]);
        else
            x_mag = (pc * pe - pb * pf) / (pa * pe - pb * pd);

        return x_mag * (float)(hvkt->hsize_in - 1) / (float)(hvkt->hsize - 1);
    } else {
        if (hvkt->fw_map)
            y_mag = (fabc->fb0[c_sel] * x + fabc->fb1[c_sel] * y + fabc->fb2[c_sel])
                    / (fabc->fc0[c_sel] * x + fabc->fc1[c_sel] * y + fabc->fc2[c_sel]);
        else
            y_mag = (pa * pf - pc * pd) / (pa * pe - pb * pd);

        return y_mag * (float)(hvkt->vsize_in - 1) / (float)(hvkt->vsize - 1);
    }
}

static int hvk_table_gen(struct hvkt_param *hvkt)
{
    float delta_x0, delta_y0;
    float delta_x1, delta_y1;
    float sum_x, sum_y;
    float poly0, poly1;

    float a0, a1, a2;
    float b0, b1, b2;
    float c0, c1, c2;

    int lut_hsize, lut_vsize;
    float n, m;
    int extend;
    int chn;
    float *lut;
    int *lut_32[6];
    short *lut_16[6];

    struct hvkt_fabc fabc;
    memset(&fabc, 0, sizeof(struct hvkt_fabc));

    hvkt->fabc = &fabc;
    lut_hsize = hvkt->lut_h_size;
    lut_vsize = hvkt->lut_v_size;

    lut = (float *)hvkt->lut_addr;

    chn = hvkt->chn;
    for (int i = 0; i < chn; i++) {
        delta_x0 = hvkt->top_right_x[i] - hvkt->bottom_right_x[i];
        delta_y0 = hvkt->top_right_y[i] - hvkt->bottom_right_y[i];
        delta_x1 = hvkt->bottom_left_x[i] - hvkt->bottom_right_x[i];
        delta_y1 = hvkt->bottom_left_y[i] - hvkt->bottom_right_y[i];

        sum_x = hvkt->top_left_x[i] - hvkt->top_right_x[i] + hvkt->bottom_right_x[i] - hvkt->bottom_left_x[i];
        sum_y = hvkt->top_left_y[i] - hvkt->top_right_y[i] + hvkt->bottom_right_y[i] - hvkt->bottom_left_y[i];

        poly0 = (sum_x * delta_y1 - sum_y * delta_x1);
        poly0 = poly0 / (delta_x0 * delta_y1 - delta_x1 * delta_y0);
        poly1 = (sum_y * delta_x0 - sum_x * delta_y0) / (delta_x0 * delta_y1 - delta_x1 * delta_y0);

        a0 = ((hvkt->top_right_x[i] - hvkt->top_left_x[i]) + poly0 * hvkt->top_right_x[i]) / (hvkt->hsize - 1);
        a1 = ((hvkt->bottom_left_x[i] - hvkt->top_left_x[i]) + poly1 * hvkt->bottom_left_x[i]) / (hvkt->vsize - 1);
        a2 = hvkt->top_left_x[i];

        b0 = ((hvkt->top_right_y[i] - hvkt->top_left_y[i]) + poly0 * hvkt->top_right_y[i]) / (hvkt->hsize - 1);
        b1 = ((hvkt->bottom_left_y[i] - hvkt->top_left_y[i]) + poly1 * hvkt->bottom_left_y[i]) / (hvkt->vsize - 1);
        b2 = hvkt->top_left_y[i];

        c0 = poly0 / (hvkt->hsize - 1);
        c1 = poly1 / (hvkt->vsize - 1);
        c2 = 1;

        fabc.fa0[i] = a0;
        fabc.fa1[i] = a1;
        fabc.fa2[i] = a2;

        fabc.fb0[i] = b0;
        fabc.fb1[i] = b1;
        fabc.fb2[i] = b2;

        fabc.fc0[i] = c0;
        fabc.fc1[i] = c1;
        fabc.fc2[i] = c2;
    }

    m = (float)hvkt->lut_h_step;
    n = (float)hvkt->lut_v_step;


    if (!hvkt->is_external_lut) {
        extend = hvkt->extend;
        for (int v = - extend; v < lut_vsize - extend; v++) {
            for (int i = 0; i < chn; i++) {
                for (int h = -extend; h < lut_hsize - extend; h++) {
                    float x = h * m;
                    float y = v * n;
                    int c_sel = i;

                    /*lut[v][i][0][h]*/
                    int pos_x = (v + extend) * (lut_hsize * chn * 2 ) + i * (lut_hsize * 2) + (h + extend);
                    /*lut[v][i][1][h]*/
                    int pos_y = (v + extend) * (lut_hsize * chn * 2 ) + i * (lut_hsize * 2) + lut_hsize + (h + extend);
                    //printf("pos_x=%d, pos_y=%d\n", pos_x, pos_y);
                    lut[pos_x] = hvk_pos_calc(x, y, hvkt, true, c_sel); //x
                    lut[pos_y] = hvk_pos_calc(x, y, hvkt, false, c_sel); //y
                }
            }
        }
    }else {//bilinear
        for (int i = 0; i < 6; i++) {
            lut_32[i] = (int *)hvkt->lut_real_addr[i];
            lut_16[i] = (short *)hvkt->lut_real_addr[i];
        }
        for (int i = 0; i < chn; i++) {
            for (int v = 0; v < lut_vsize; v++) {
                for (int h = 0; h < lut_hsize; h++) {
                    float x = h * m;
                    float y = v * n;
                    int c_sel = i;

                    lut[h] = hvk_pos_calc(x, y, hvkt, true, c_sel); //x
                    lut[lut_hsize + h] = hvk_pos_calc(x, y, hvkt, false, c_sel); //y

                    if (hvkt->external_lut_format == 1) {//32bits
                        int tmp = (int)(lut[h] * 256);
                        tmp = (tmp > 524287) ? 524287 : (tmp < -524288) ? -524288 : tmp;
                        lut_32[i * 2][v * lut_hsize + h] = tmp;

                        tmp = (int)(lut[lut_hsize + h] * 256);
                        tmp = (tmp > 524287) ? 524287 : (tmp < -524288) ? -524288 : tmp;
                        lut_32[i * 2 + 1][v * lut_hsize + h] = tmp;
                        //GAMAMSG("(%f, %f) --(0x%d, 0x%d)\n", lut[pos_x], lut[pos_y], lut_32[pos_x], lut_32[pos_y]);
                    } else {
                        int tmp16 = (int)(lut[h] * 16);
                        tmp16 = (tmp16 > 32767) ? 32767 : (tmp16 < -32768) ? -32768 : tmp16;
                        lut_16[i * 2][v * lut_hsize + h] = (short)tmp16;

                        tmp16 = (int)(lut[lut_hsize + h] * 16);
                        tmp16 = (tmp16 > 32767) ? 32767 : (tmp16 < -32768) ? -32768 : tmp16;
                        lut_16[i * 2 + 1][v * lut_hsize + h] = (short)tmp16;
                        //GAMAMSG("(%d, %d), (%f, %f) --(0x%04x, 0x%04x)\n", pos_x, pos_y, lut[pos_x], lut[pos_y], lut_16[pos_x], lut_16[pos_y]);
                    }
                }
            }
        }

        for (int i = 0; i < 6; i++) {
            if (hvkt->external_lut_format == 1) //32bits
                arch_clean_cache_range((int)lut_32[i], lut_hsize * lut_vsize *4);
            else
                arch_clean_cache_range((int)lut_16[i], lut_hsize * lut_vsize *2);
        }
    }

    return 0;
}

static int m_v_parameter_calc(struct hvkt_param *hvkt)
{
    int lut_hsize, lut_vsize;
    float *lut_ld, *m_param_ld;
    float *lut = (float *)hvkt->lut_addr;
    float *m_v = (float *)hvkt->m_v_addr;
    float *alpha_addr = (float *)hvkt->alpha_addr;
    int chn = hvkt->chn;

    lut_hsize = hvkt->lut_h_size;
    lut_vsize = hvkt->lut_v_size;

    lut_ld = (float *)malloc(sizeof(float) * hvkt->lut_v_size);
    if (!lut_ld) {
        ASW_LOG_ERR("malloc lut_ld failed\n");
        return -1;
    }

    m_param_ld = (float *)malloc(sizeof(float) * hvkt->lut_v_size);
    if (!m_param_ld) {
        ASW_LOG_ERR("malloc m_param_ld failed\n");
        return -1;
    }

    m_parameter_calc_alpha(lut_vsize, alpha_addr);

    for (int h = 0; h < lut_hsize; h++) {
        for (int i = 0; i < chn; i++) {
            /*x*/
            for (int v = 0; v < lut_vsize; v++) {
                lut_ld[v] = lut[v * lut_hsize * chn * 2 + i * lut_hsize * 2 + h];//lut[v][i][0][h];
            }
            m_parameter_calc_1d(lut_ld, m_param_ld, alpha_addr, lut_vsize, false);

            for (int v = 0; v < lut_vsize; v++) {
                /*m_v[i][v][0][h]*/
                m_v[v * lut_hsize * chn * 2 + i * lut_hsize * 2 + h] = m_param_ld[i];
            }

            /*y*/
            for (int v = 0; v < lut_vsize; v++) {
                lut_ld[v] = lut[v * lut_hsize * chn * 2 + i * lut_hsize * 2 + lut_hsize + h];//lut[v][i][1][h]
            }
            m_parameter_calc_1d(lut_ld, m_param_ld, alpha_addr, lut_vsize, false);

            for (int v = 0; v < lut_vsize; v++) {
                /*m_v[i][v][1][h]*/
                m_v[v * lut_hsize * chn * 2 + i * lut_hsize * 2 + lut_hsize + h] = m_param_ld[i];
            }
        }
    }

    free(lut_ld);
    free(m_param_ld);

    return 0;
}

static double sinc(double phase)
{
    if (fabs(phase) < 0.0001)
        return 1.0;
    else
        return sin(phase) / phase;
}

static int rotation(double angle, struct hvkt_param *hvkt)
{
    double r, theta, rot;
    double x, y;
    int transpose_en;
    double rotate_angle;

    double PI = acos(-1);

    rotate_angle = (angle * PI) / 180.0;
    x = (hvkt->hsize_in - 1.0) / 2.0;
    y = (hvkt->vsize_in - 1.0) / 2.0;
    theta = atan(y / x);
    r = sqrt(x * x + y * y);

    //rotate_angle = 2.0 * PI - rotate_angle;

    if ((rotate_angle >= 0.0 && rotate_angle <= (PI / 4.0)) || (rotate_angle > (PI * 7.0 / 4.0) && rotate_angle < (PI * 8.0 / 4.0))) {
        rot = rotate_angle;
        hvkt->flip_type = 0;
        transpose_en = 0;
    } else if (rotate_angle > (3.0 * PI / 4.0) && rotate_angle <= (5.0 * PI / 4.0)) {
        rot = rotate_angle - PI;
        hvkt->flip_type = 3;
        transpose_en = 0;
    } else if (rotate_angle > (PI / 4.0) && rotate_angle <= (PI * 3.0 / 4.0)) {
        rot = PI / 2.0 - rotate_angle;
        hvkt->flip_type = 1;
        transpose_en = 1;
    }
    else if (rotate_angle > (PI * 5.0 / 4.0) && rotate_angle < (PI * 7.0 / 4.0)) {
        rot = 6.0 * PI / 4.0 - rotate_angle;
        hvkt->flip_type = 2;
        transpose_en = 1;
    } else {//illigal
        rot = rotate_angle;
        hvkt->flip_type = 0;
        transpose_en = 0;
    }

    rot = -rot;

    hvkt->top_left_x[0] = cos(1.0 * PI + theta - rot) * r + x;
    hvkt->top_left_y[0] = sin(1.0 * PI + theta - rot) * r + y;

    hvkt->top_right_x[0] = cos(-theta - rot) * r + x;
    hvkt->top_right_y[0] = sin(-theta - rot) * r + y;

    hvkt->bottom_left_x[0] = cos(PI - theta - rot) * r + x;
    hvkt->bottom_left_y[0] = sin(PI - theta - rot) * r + y;

    hvkt->bottom_right_x[0] = cos(theta - rot) * r + x;
    hvkt->bottom_right_y[0] = sin(theta - rot) * r + y;
    ASW_LOG_DEBUG("top-left(%f, %f), top-right(%f, %f), bottom-left(%f, %f), bottom-right(%f, %f)\n",
        hvkt->top_left_x[0], hvkt->top_left_y[0], hvkt->top_right_x[0], hvkt->top_right_y[0],
        hvkt->bottom_left_x[0], hvkt->bottom_left_y[0], hvkt->bottom_right_x[0], hvkt->bottom_right_y[0]);

    return transpose_en;
}

static int rotate_lut_table_gen(double angle, struct hvkt_param *hvkt)
{
    int lut_step_h, lut_step_v;

    lut_step_h = (hvkt->hsize + 31) / 32;
    lut_step_v = (hvkt->vsize + 31) / 32;

    hvkt->lut_h_step = lut_step_h;
    hvkt->lut_v_step = lut_step_v;

    rotation(angle, hvkt);
    hvk_table_gen(hvkt);

    return 0;
}

struct sdrv_lut_hvk_ops lut_hvk_ops = {
    .lut_table_gen = hvk_table_gen,
    .lut_m_v_gen = m_v_parameter_calc,
    .lut_rotate_table_gen = rotate_lut_table_gen,
};

