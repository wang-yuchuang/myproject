/*
 * sd_errcode_ospi.h
 *
 * Copyright (c) 2022 SemiDrive Semiconductor.
 * All rights reserved.
 *
 * Description:
 * errcode to ospi head file
 *
 * Revision History:
 * -----------------
 */

#ifndef _SD_ERRCODE_OSPI_H_
#define _SD_ERRCODE_OSPI_H_

#include <sys/types.h>
#include "errcode.h"


ERR_HANDLE errcode_set_to_ospi(int strid, uint8_t *data_buffer, uint16_t length);
int errcode_dump_from_ospi(int strid, char* buf, int bufsize);
int errcode_clear_ospi(int strid);
int errcode_ospi_init(void);
#endif //_SD_ERRCODE_OSPI_H_
