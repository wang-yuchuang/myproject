/*****************************************************************************
 *
 *
 *Copyright (c) 2021-2029 Semidrive Incorporated.  All rights reserved.
 *Software License Agreement
 *
 ******************************************************************************
 */
#ifndef REGISTER_BASE_ADDR_H
#define REGISTER_BASE_ADDR_H

/*including int32_t definition*/
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*just for temporary use*/
/* iram3 0x600000 */
#define CACC_TEST_DMA_RAM_BASE                (0x600000UL)

/*register base address*/
/*PKE register base address*/
#define PKE_BASE_ADDR               (0x021C0000UL)
/*TRNG register base address*/
#define TRNG_BASE_ADDR              (0x021C4000UL)
/*SKE_HP register base address*/
#define SKE_HP_BASE_ADDR            (0x021C8000UL)
/*HASH register base address*/
#define HASH_BASE_ADDR              (0x021CC000UL)

#define APB_SEIP_BASE_ADDR          (0xF3100000UL)

#define OTP_KEY_CTRL                (*((volatile uint32_t *)(0x021DD804UL)))

#ifdef __cplusplus
}
#endif

#endif
