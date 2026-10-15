/**
 * @file boardinfo.h
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 *
 * Revision History:
 * -----------------
 */

#ifndef BOARD_INFO_H_
#define BOARD_INFO_H_

#include <types.h>
#include <compiler.h>

#define HW_ID_MAGIC     0x5

struct version1 {//24 bit
    uint32_t chipid: 5; //chip version
    uint32_t featurecode: 2; //feature code
    uint32_t speed_grade: 2; //
    uint32_t temp_grade: 2; //
    uint32_t pkg_type: 2; //
    uint32_t revision: 2; //
    uint32_t board_type: 2; //
    uint32_t board_id_major:3; //major, in ms board, this will be core boardid,
    uint32_t board_id_minor:4; //minor, in ms board, this will be base boardid,
} __PACKED;

union version {
    struct version1 v1;
};

// on system for user
struct sd_hwid_usr {
    uint32_t magic: 4; // must be 0x5
    uint32_t ver: 4; // encode version
    union version v;
} __PACKED;

enum sd_board_type_e {//2 bit
    BOARD_TYPE_UNKNOWN,
    BOARD_TYPE_EVB,
    BOARD_TYPE_REF,
    BOARD_TYPE_MS,
};

enum sd_boardid_major_e {//3 bit
    BOARDID_MAJOR_UNKNOWN,
    BOARDID_MAJOR_A,
    BOARDID_MAJOR_G9A
};
enum sd_boardid_ms_major_e {//3 bit
    BOARDID_MAJOR_MPS = 1,
    BOARDID_MAJOR_TI_A01,
    BOARDID_MAJOR_TI_A02,
};

enum part_e {
    PART_CHIPID,
    PART_FEATURE,
    PART_SPEED,
    PART_TEMP,
    PART_PKG,
    PART_REV,
    PART_BOARD_TYPE,
    PART_BOARD_ID_MAJ,
    PART_BOARD_ID_MIN,
};

#define GET_PART_ID(part, v) \
    switch(part)    \
        {   \
            case PART_CHIPID:   \
                return v->chipid;   \
            case PART_FEATURE:  \
                return v->featurecode;  \
            case PART_SPEED:    \
                return v->speed_grade;  \
            case PART_TEMP: \
                return v->temp_grade;   \
            case PART_PKG:  \
                return v->pkg_type; \
            case PART_REV:  \
                return v->revision; \
            case PART_BOARD_TYPE:   \
                return v->board_type;   \
            case PART_BOARD_ID_MAJ: \
                return v->board_id_major;   \
            case PART_BOARD_ID_MIN: \
                return v->board_id_minor;   \
            default:    \
                printf("not recognize the  part\n");    \
                goto err;   \
        }

struct sd_hwid_usr get_fullhw_id(void);
int get_part_id(enum part_e part);

#endif /* BOARD_INFO_H_ */
