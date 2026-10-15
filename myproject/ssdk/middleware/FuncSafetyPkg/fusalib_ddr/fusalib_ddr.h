#ifndef __FUSALIB_DDR_H__
#define __FUSALIB_DDR_H__

#include <stdlib.h>
#include "reg.h"
//#include <__regs_base.h>

#define FUSA_DDR_NOERR 0
#define FUSA_DDR_ERR  (-1)

void fusalib_ddr_en(void);
void fusalib_ddr_check(void);

/* case1 */
typedef struct {
    uint32_t corrected_err_num;
    uint32_t uncorrected_err_num;
}fusalib_ddr_check_ecc_status;
int fusalib_ddr_check_ecc(fusalib_ddr_check_ecc_status *status);
#if SUPPORT_FUSA_DDR_LOCAL_TEST_CASE
void fusalib_ddr_ecc_inject_err(void);
#endif
/* case2 */
typedef struct {
    uint16_t err_write_ports;
    uint16_t err_read_ports;
    uint32_t detail_info;
}fusalib_ddr_check_parity_status;
void fusalib_ddr_parity_en(void);
int fusalib_ddr_check_parity(fusalib_ddr_check_parity_status *status);
/* case3 */
void fusalib_ddr_ca_en(void);
int fusalib_ddr_check_ca(void);
#if SUPPORT_FUSA_DDR_LOCAL_TEST_CASE
void fusalib_ddr_ca_inject_err(void);
#endif
/* case4 */
void fusalib_ddr_reg_par_en(void);
int fusalib_ddr_check_reg_par(void);
#if SUPPORT_FUSA_DDR_LOCAL_TEST_CASE
void fusalib_ddr_reg_par_inject_err(void);
#endif
/* case5 */
int fusalib_ddr_check_addr_ecc(void);
/* case7 */
void fusalib_ddr_read_timeout_en(uint32_t value);
int fusalib_ddr_check_read_timeout(void);
#if SUPPORT_FUSA_DDR_LOCAL_TEST_CASE
void fusalib_ddr_read_timeout_inject_err(void);
#endif
/* case9 */
void fusalib_ddr_read_fifo_not_empty_en(void);
int fusalib_ddr_read_fifo_not_empty_check(void);
/* case8 */
void fusalib_ddr_readback_check_en(vaddr_t base, size_t size, int interval_ms);
void fusalib_ddr_readback_check_stop(void);
/* case12 */
void fusalib_ddr_phy_readback_check_en(vaddr_t base, size_t size, int interval_ms);
void fusalib_ddr_phy_readback_check_stop(void);
#endif
