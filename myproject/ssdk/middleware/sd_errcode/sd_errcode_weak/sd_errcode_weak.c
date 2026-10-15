/*
 * sd_errcode.c
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * errcode library
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <debug.h>
#include "sd_errcode.h"

int system_errcode_early_init(void)
{
    return ERRCODE_OK;
}

int system_errcode_init(void)
{
    return ERRCODE_OK;
}

int system_errcode_deinit(void)
{
    return ERRCODE_OK;
}

ERR_HANDLE errcode_set(errcode_t* errcode)
{
    return ERRCODE_OK;
}

int errcode_transfer(const char* ip, errcode_t* errcode)
{
    return ERRCODE_OK;
}

int errcode_dump(storage_id_t strid, char* buf, int bufsize)
{
    return ERRCODE_OK;
}

int errcode_clear(storage_id_t strid)
{
    return ERRCODE_OK;
}

ERR_HANDLE errcode_set_by_strid(errcode_t* errcode, storage_id_t strid)
{
    return ERRCODE_OK;
}

int system_errcode_set_storage_id(storage_id_t strid)
{
    return ERRCODE_OK;
}

int system_errcode_get_storage_id()
{
    return ERRCODE_OK;
}
