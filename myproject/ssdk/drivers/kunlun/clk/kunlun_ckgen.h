/**
 * @file kunlun_ckgen.h
 * @brief kunlun ckgen lld header
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *            All rights reserved.
 */

#ifndef KUNLUN_CKGEN_H_
#define KUNLUN_CKGEN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <types.h>

//#define CONFIG_CLK_MONITOR

/* CKGEN index */
#define KUNLUN_CKGEN_ID_NUM_LSB  (0u)
#define KUNLUN_CKGEN_ID_NUM_VCO  (0u)
#define KUNLUN_CKGEN_ID_NUM_ROOT (1u)
#define KUNLUN_CKGEN_ID_NUM_DIVA (2u)
#define KUNLUN_CKGEN_ID_NUM_DIVB (3u)
#define KUNLUN_CKGEN_ID_NUM_DIVC (4u)
#define KUNLUN_CKGEN_ID_NUM_DIVD (5u)
#define KUNLUN_CKGEN_ID_NUM_MASK (0xFF)

/* IP control register */
#define CKGEN_IP_CTRL               (0x0u)
#define CKGEN_IP_CTRL_CGEN          (0u)
#define CKGEN_IP_CTRL_SRC_SEL_LSB   (1u)
#define CKGEN_IP_CTRL_SRC_SEL_MASK  (0x7u)
#define CKGEN_IP_CTRL_PRE_DIV_LSB   (4u)
#define CKGEN_IP_CTRL_PRE_DIV_MASK  (0x7u)
#define CKGEN_IP_CTRL_POST_DIV_LSB  (10u)
#define CKGEN_IP_CTRL_POST_DIV_MASK (0x3Fu)
#define CKGEN_IP_CTRL_CGEN_STATUS   (28u)
#define CKGEN_IP_CTRL_PRE_BUSY      (30u)
#define CKGEN_IP_CTRL_POST_BUSY     (31u)

/* BUS control register */
#define CKGEN_BUS_CTRL               (0x200u)
#define CKGEN_BUS_CTRL_CGEN_A        (0u)
#define CKGEN_BUS_CTRL_CGEN_B        (16u)
#define CKGEN_BUS_CTRL_SRC_SEL_A_LSB (1u)
#define CKGEN_BUS_CTRL_SRC_SEL_B_LSB (17u)
#define CKGEN_BUS_CTRL_SRC_SEL_MASK  (0x7u)
#define CKGEN_BUS_CTRL_PRE_DIV_A_LSB (4u)
#define CKGEN_BUS_CTRL_PRE_DIV_B_LSB (20u)
#define CKGEN_BUS_CTRL_PRE_DIV_MASK  (0x7u)
#define CKGEN_BUS_CTRL_AB_SEL        (9u)
#define CKGEN_BUS_CTRL_POST_DIV_LSB  (10u)
#define CKGEN_BUS_CTRL_POST_DIV_MASK (0x3Fu)
#define CKGEN_BUS_CTRL_CGEN_STATUS_B (27u)
#define CKGEN_BUS_CTRL_CGEN_STATUS_A (28u)
#define CKGEN_BUS_CTRL_PRE_BUSY_B    (29u)
#define CKGEN_BUS_CTRL_PRE_BUSY_A    (30u)
#define CKGEN_BUS_CTRL_POST_BUSY     (31u)

/* BUS gasket register */
#define CKGEN_BUS_GASKET            (0x204u)
#define CKGEN_BUS_GASKET_Q_DIV_LSB  (0u)
#define CKGEN_BUS_GASKET_P_DIV_LSB  (3u)
#define CKGEN_BUS_GASKET_N_DIV_LSB  (6u)
#define CKGEN_BUS_GASKET_M_DIV_LSB  (9u)
#define CKGEN_BUS_GASKET_DIV_MASK   (0x7u)
#define CKGEN_BUS_GASKET_Q_DIV_BUSY (28u)
#define CKGEN_BUS_GASKET_P_DIV_BUSY (29u)
#define CKGEN_BUS_GASKET_N_DIV_BUSY (30u)
#define CKGEN_BUS_GASKET_M_DIV_BUSY (31u)

/* CORE control register */
#define CKGEN_CORE_CTRL               (0x300u)
#define CKGEN_CORE_CTRL_CGEN_A        (0u)
#define CKGEN_CORE_CTRL_CGEN_B        (16u)
#define CKGEN_CORE_CTRL_SRC_SEL_A_LSB (1u)
#define CKGEN_CORE_CTRL_SRC_SEL_B_LSB (17u)
#define CKGEN_CORE_CTRL_AB_SEL        (9u)
#define CKGEN_CORE_CTRL_POST_DIV_LSB  (10u)
#define CKGEN_CORE_CTRL_POST_DIV_MASK (0x3Fu)
#define CKGEN_CORE_CTRL_CGEN_STATUS_B (27u)
#define CKGEN_CORE_CTRL_CGEN_STATUS_A (28u)
#define CKGEN_CORE_CTRL_POST_BUSY     (31u)

/* Low power gating control register */
#define CKGEN_LP_GATING        (0x400u)
#define CKGEN_LP_GATING_SW_DIS (1u)
#define CKGEN_LP_GATING_LOCK   (31u)

/* UUU wrapper register */
#define CKGEN_UUU_CTRL               (0x600u)
#define CKGEN_UUU_CTRL_Q_DIV_LSB     (0u)
#define CKGEN_UUU_CTRL_P_DIV_LSB     (4u)
#define CKGEN_UUU_CTRL_N_DIV_LSB     (8u)
#define CKGEN_UUU_CTRL_M_DIV_LSB     (12u)
#define CKGEN_UUU_CTRL_DIV_MASK      (0xFu)
#define CKGEN_UUU_CTRL_SEL0          (16u)
#define CKGEN_UUU_CTRL_SEL1          (17u)
#define CKGEN_UUU_CTRL_GATING_ACK    (20u)
#define CKGEN_UUU_CTRL_DBG_LSB       (27u)
#define CKGEN_UUU_CTRL_DBG_MASK      (0xFu)
#define CKGEN_UUU_CTRL_DEG_GATING_EN (31u)

/* Monitor register */
#define CKGEN_IP_MON_CTRL   ((0x720u) << 10u)
#define CKGEN_BUS_MON_CTRL  ((0x724u) << 10u)
#define CKGEN_CORE_MON_CTRL ((0x728u) << 10u)

#define CKGEN_MON_CTRL_SLICE_LSB  (0u)
#define CKGEN_MON_CTRL_SLICE_MASK (0xFFFFu)

#define CKGEN_MON_CTRL             ((0x72Cu) << 10u)
#define CKGEN_MON_CTRL_DIV_LSB     (0u)
#define CKGEN_MON_CTRL_DIV_MASK    (0xFu)
#define CKGEN_MON_CTRL_DIV_BUSY    (16u)
#define CKGEN_MON_CTRL_CLK_SRC     (25u)
#define CKGEN_MON_CTRL_CLK_DIS_STA (26u)
#define CKGEN_MON_CTRL_FREQ_READY  (27u)
#define CKGEN_MON_CTRL_VAL_UPDATE  (28u)
#define CKGEN_MON_CTRL_SEL_LSB     (29u)
#define CKGEN_MON_CTRL_SEL_MASK    (0x3u)
#define CKGEN_MON_CTRL_SEL_IP      (0x0u)
#define CKGEN_MON_CTRL_SEL_BUS     (0x1u)
#define CKGEN_MON_CTRL_SEL_CORE    (0x2u)
#define CKGEN_MON_CTRL_CLK_DIS     (31u)

#define CKGEN_MON_AVE_FREQ      ((0x734u) << 10u)
#define CKGEN_MON_AVE_FREQ_LSB  (0u)
#define CKGEN_MON_AVE_FREQ_MASK (0xFFFFu)

/* BUS index */
#define CKGEN_BUS_ID_TYPE_LSB      (24u)
#define CKGEN_BUS_ID_TYPE_MASK     (0xFFu)
#define CKGEN_BUS_ID_TYPE_DIV_ROOT (0x0u)
#define CKGEN_BUS_ID_TYPE_DIV_Q    (0x1u)
#define CKGEN_BUS_ID_TYPE_DIV_P    (0x2u)
#define CKGEN_BUS_ID_TYPE_DIV_N    (0x3u)
#define CKGEN_BUS_ID_TYPE_DIV_M    (0x4u)
#define CKGEN_BUS_ID_NUM_LSB       (0u)
#define CKGEN_BUS_ID_NUM_MASK      (0xFFFFu)

/* UUU index */
#define CKGEN_UUU_ID_TYPE_LSB   (24u)
#define CKGEN_UUU_ID_TYPE_MASK  (0xFFu)
#define CKGEN_UUU_ID_TYPE_DIV_Q (0x0u)
#define CKGEN_UUU_ID_TYPE_DIV_P (0x1u)
#define CKGEN_UUU_ID_TYPE_DIV_N (0x2u)
#define CKGEN_UUU_ID_TYPE_DIV_M (0x3u)
#define CKGEN_UUU_ID_NUM_LSB    (0u)
#define CKGEN_UUU_ID_NUM_MASK   (0xFFFFu)

/* UUU source index */
#define CKGEN_UUU_SRC_ID_CKGEN (0x0u)
#define CKGEN_UUU_SRC_ID_PLL   (0x1u)

#define CKGEN_IP_CTRL_BASE(base, id)                                           \
    ((base) + ((CKGEN_IP_CTRL + (id)*4u) << 10u))

#define CKGEN_BUS_CTRL_BASE(base, id)                                          \
    ((base) + ((CKGEN_BUS_CTRL + ((id)&CKGEN_BUS_ID_NUM_MASK) * 8u) << 10u))

#define CKGEN_BUS_ID_TYPE(base, id) ((id) >> CKGEN_BUS_ID_TYPE_LSB)

#define CKGEN_BUS_GASKET_BASE(base, id)                                        \
    ((base) + ((CKGEN_BUS_GASKET + ((id)&CKGEN_BUS_ID_NUM_MASK) * 8u) << 10u))

#define CKGEN_CORE_CTRL_BASE(base, id)                                         \
    ((base) + ((CKGEN_CORE_CTRL + (id)*4u) << 10u))

#define CKGEN_LP_GATING_BASE(base, id)                                         \
    ((base) + ((CKGEN_LP_GATING + (id)*4u) << 10u))

#define CKGEN_UUU_CTRL_BASE(base, id)                                          \
    ((base) + ((CKGEN_UUU_CTRL + ((id)&CKGEN_UUU_ID_NUM_MASK) * 4u) << 10u))

#define CKGEN_UUU_ID_TYPE(base, id) ((id) >> CKGEN_UUU_ID_TYPE_LSB)

typedef uint32_t ckgen_rate_t;

int ckgen_ip_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                     ckgen_rate_t src_rate, ckgen_rate_t rate);

ckgen_rate_t ckgen_ip_get_rate(paddr_t ckgen_base, uint32_t id,
                               ckgen_rate_t src_rate);

int ckgen_ip_set_rate(paddr_t ckgen_base, uint32_t id,
                      ckgen_rate_t src_rate, ckgen_rate_t rate);

int ckgen_bus_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                      ckgen_rate_t src_rate, ckgen_rate_t rate);

ckgen_rate_t ckgen_bus_get_rate(paddr_t ckgen_base, uint32_t id,
                                ckgen_rate_t src_rate);

int ckgen_bus_set_rate(paddr_t ckgen_base, uint32_t id,
                        ckgen_rate_t src_rate, ckgen_rate_t rate);

int ckgen_core_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                       ckgen_rate_t src_rate, ckgen_rate_t rate);

ckgen_rate_t ckgen_core_get_rate(paddr_t ckgen_base, uint32_t id,
                                 ckgen_rate_t src_rate);

int ckgen_core_set_rate(paddr_t ckgen_base, uint32_t id,
                        ckgen_rate_t src_rate, ckgen_rate_t rate);

int ckgen_uuu_prepare(paddr_t ckgen_base, uint32_t id, uint32_t mux,
                      ckgen_rate_t src_rate, ckgen_rate_t rate);

ckgen_rate_t ckgen_uuu_get_rate(paddr_t ckgen_base, uint32_t id,
                                ckgen_rate_t src_rate);

int ckgen_uuu_set_rate(paddr_t ckgen_base, uint32_t id,
                       ckgen_rate_t src_rate, ckgen_rate_t rate);

int ckgen_lp_gating_enable(paddr_t ckgen_base, uint32_t id);

int ckgen_lp_gating_disable(paddr_t ckgen_base, uint32_t id);

#ifdef CONFIG_CLK_MONITOR
ckgen_rate_t ckgen_ip_mon_get_rate(paddr_t ckgen_base, uint32_t id,
                                   ckgen_rate_t src_rate);

ckgen_rate_t ckgen_bus_mon_get_rate(paddr_t ckgen_base, uint32_t id,
                                    ckgen_rate_t src_rate);

ckgen_rate_t ckgen_core_mon_get_rate(paddr_t ckgen_base, uint32_t id,
                                     ckgen_rate_t src_rate);
#endif

#ifdef __cplusplus
}
#endif

#endif /* KUNLUN_CKGEN_H_ */
