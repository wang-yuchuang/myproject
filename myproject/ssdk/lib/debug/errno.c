/*
 * errno.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: errno defines.
 *
 * Revision History:
 * -----------------
 */

#include <errno.h>

#if !defined(__GNUC__) && !defined(__ICCARM__)

static int ssdk_errno;

int *ssdk_geterrno(void)
{
    return &ssdk_errno;
}
#endif
