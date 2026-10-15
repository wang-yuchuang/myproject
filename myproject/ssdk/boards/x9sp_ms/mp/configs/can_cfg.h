/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifdef CONFIG_VCAN_CLIENT
#ifndef __CAN_RES__
#define __CAN_RES__

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

extern void board_vcan_init(void);
#endif /*__CAN_RES__*/
#endif /* end CONFIG_VCAN_CLIENT */