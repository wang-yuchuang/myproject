/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */
#ifndef BOARDS_REMAP_H
#define BOARDS_REMAP_H

#ifdef CONFIG_BOARD_REMAP

extern void board_remap(void);
extern void vector_remap(addr_t vector_base);

#endif

#endif /* BOARDS_REMAP_H */