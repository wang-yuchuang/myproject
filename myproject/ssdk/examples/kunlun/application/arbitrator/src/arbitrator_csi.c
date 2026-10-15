/**
 * @file arbitrator_csi.c
 *
 * Copyright (c) 2022 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: sdrv arbitrator library.
 *
 * Revision History:
 * -----------------
 */
typedef struct _csi_slave_t {
    sdm_display_t* disp;
    int tk;
    int preempt;
    int accessor_num;
    struct list_node accessor_list;
} csi_slave_t;
/* EOF */

