#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "CLI_type.h"

#include <backlight.h>

int cmd_bl(int argc, char *argv[])
{
    char *end_ptr;
    uint32_t index = 0;
    uint32_t val = 0;

    if ((argc != 3) && (argc != 2)) {
        printf("cmd_bl: invalid param.\n");
        printf("bl get/[set] index [val]\n");
        return CLI_FALSE;
    }

    errno = 0;
    index = strtoul(argv[1], &end_ptr, 10);
    if (*end_ptr || errno) {
        printf("invalid backlight index %s %d %d", argv[1], index, errno);
        return CLI_FALSE;
    }

    if ((0 == strcmp(argv[0], "set")) && (argc == 3)) {
        errno = 0;
        val = strtoul(argv[2], &end_ptr, 10);
        if (*end_ptr || errno) {
            printf("invalid duty value %s 0x%x %d", argv[1], (unsigned int)val,
                   errno);
            return CLI_FALSE;
        }
        backlight_duty_set(index, val);
    } else if ((0 == strcmp(argv[0], "get")) && (argc == 2)) {
        val = backlight_duty_get(index);
        printf("screenid[%d] = %d\n", index, val);
    } else {
        printf("usage example:\n\t bl get 0\n\tbl set 0 100\n");
        return CLI_FALSE;
    }

    return CLI_TRUE;
}

CLI_CMD("bl", "bl get/[set] index [val]", cmd_bl);
