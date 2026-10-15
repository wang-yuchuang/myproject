/*
 * res_rstgen.h
 *
 * Copyright (c) 2019 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: MODULE HELPER rstgen.
 *
 * Revision History:
 * -----------------
 */
#ifndef _RES_RSTGEN_H
#define _RES_RSTGEN_H

#include <lib/list.h>
#include <assert.h>
//#include <module_helper_hal.h>
#include <module_helper_hal_internal.h>
#include "rstgen_cfg.h"
#define RES_RSTGEN_DEBUG 1
#ifndef REFCNT_NUM
#define REFCNT_NUM ((MAX_PER_ID-1)/8 +1)
#endif

enum rstgen_type {
    RST_TYPE_ISO,
    RST_TYPE_MODULE,
    RST_TYPE_CORE,
};

enum rstgen_param {
    RST_INVALID = 0xffffffff,
    RST_HOLD = 0,
    RST_RELEASE = 1,
};

#include "res_rstgen_id.h"

struct rstgen {
    uint32_t resid;//global res id
    reset_sig_t* reset_sig;
#ifdef RES_RSTGEN_DEBUG
    char name[20];
#endif

    uint8_t refcount[REFCNT_NUM];
    struct list_node node;  //dynmic, current node
    //struct list_node child;   //dynmic, head of child list

    uint16_t rstid;
    uint32_t has_common_ops : 1;
    int8_t polar : 7;//0: (0, hold, 1 release), 1: (1, hold, 0 release)
    uint8_t type;

    //debug
    int release_cnt;
    int hold_cnt;

#if 0
    int (*is_released)(struct rstgen *rst);
    int (*release)(struct rstgen *rst);
    int (*hold)(struct rstgen *rst);
#endif
};
void register_res_rstgens(void);

int res_rstgen_request(unsigned long per_id, unsigned long rstid,
                       unsigned long param);
bool res_rstgen_is_released(unsigned long rstid);
//for debug
void dump_rstgen(void);

#endif /* _RES_RSTGEN_H */


