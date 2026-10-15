/**
 * @file pvt_app.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: pvt application implement file
 *
 * Revision History:
 * -----------------
 */
/* =========================== include file session ======================== */
#include <stdlib.h>

#include "pvt_app.h"
/* =========================== marco define session ======================== */
/* =========================== enumeration define session ================== */
/* =========================== structure define session ==================== */
/* =========================== function declare session ==================== */
/* =========================== global value define session ================= */
/* =========================== static value define session ================= */
/* =========================== function implementation session ============= */

int32_t pvt_app_init()
{
#ifdef CONFIG_PVT_APPLICATION_PRINT
    pvt_app_print_init();
#endif /* end CONFIG_PVT_APPLICATION_PRINT */

#ifdef CONFIG_PVT_APPLICATION_MONITOR
    pvt_monitor_init();
#endif /* end CONFIG_PVT_APPLICATION_MONITOR */
    return 0;
}


int32_t pvt_app_deinit()
{
#ifdef CONFIG_PVT_APPLICATION_PRINT
#endif /* end CONFIG_PVT_APPLICATION_PRINT */

#ifdef CONFIG_PVT_APPLICATION_MONITOR
    pvt_monitor_deinit();
#endif /* end CONFIG_PVT_APPLICATION_MONITOR */

    return 0;
}
/* end of file pvt_app.c */