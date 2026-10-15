/**
 * @file serdes_vendor.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: serdes vendor define header
 *
 * Revision History:
 * -----------------
 */

#ifndef INCLUDE_SERDES_VENDOR_H
#define INCLUDE_SERDES_VENDOR_H

/* =========================== include file session ======================== */

#include "serdes.h"

/**
* @defgroup serdes
* @brief  serdes vendor header
* @{
*/
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */


/* =========================== enumeration define session ================== */
/** serdes vendor type enum */
enum ts_serdes_type {
    TI_SERDES,
    TS_SERDES_MAX,
};

/** ser work type enum */
enum ts_ser_type {
    TI941_SINGLE,
    TI941_SECOND,
    TI941_DUAL,
    TI947_SINGLE,
    TI947_SECOND,
    TI947_DUAL,

    MAX96789,
    MAX96735,

    TS_SER_MAX,
};

/** des enum */
enum ts_des_type {
    TI948,

    MAX96752,
    MAX96772_74,

    TS_DES_MAX
};



serdes_dev_t* ti9xx_serdes_probe(sdrv_serdes_handle* sdrvHandle, const serdes_config_t* cfgPtr);
serdes_dev_t* max967xx_serdes_probe(sdrv_serdes_handle* sdrvHandle, const serdes_config_t* cfgPtr);

/** @} */
#endif /* INCLUDE_SERDES_VENDOR_H */

/* end of header file serdes_vendor.h */