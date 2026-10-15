/**
 * @file ex_ti926.h
 * @brief export header for TI DS90UB926Q
 *
 * @copyright Copyright (c) 2021  Semidrive Semiconductor.
 *			  All rights reserved.
 */

#ifndef __EX_TI926_H__
#define __EX_TI926_H__

#include "exvdev.h"

struct vdev_device *ti926_init(void *init_data);

/* for debug only */
int user_test(int read, uint8_t reg, uint8_t *val);
/* for debug only */
extern int test_pattern;

#endif /* __EX_TI926_H__ */
