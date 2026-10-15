/**
 * @file dhry.c
 *
 * Copyright (c) 2021 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: dhrystone thread
 *
 * Revision History:
 * -----------------
 */

#include <stdlib.h>
#include <cmsis_os2.h>
#include "CLI_type.h"

extern void dhry_main(void *args);

int dhrystone_test(int argc, char *argv[])
{
  int loops = 0;

  if (argc > 0) {
    loops = strtoul(argv[0], NULL, 10);
  }

  dhry_main((void*)loops);

  return CLI_FALSE;
}

CLI_CMD("dhrystone", "start dhrystone test; dhrystone: <loops>", dhrystone_test);
