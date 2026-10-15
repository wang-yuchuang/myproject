#include <compiler.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <types.h>

#include "CLI_type.h"

#include "driver/kunlun/fuse/fuse.h"

int cmd_fuse(int argc, char *argv[])
{
    char *end_ptr;
    uint32_t index = 0;
    uint32_t val = 0;

    if ((argc != 3) && (argc != 2)) {
        return CLI_FALSE;
    }

    errno = 0;
    index = strtoul(argv[1], &end_ptr, 10);
    if (*end_ptr || errno) {
        printf("invalid fuse index %s %d %d", argv[1], index, errno);
        return CLI_FALSE;
    }

    if ((0 == strcmp(argv[0], "prog")) && (argc == 3)) {
        errno = 0;
        val = strtoul(argv[2], &end_ptr, 16);
        if (*end_ptr || errno) {
            printf("invalid fuse value %s 0x%x %d", argv[1], (unsigned int)val,
                   errno);
            return CLI_FALSE;
        }
        if (0 != fuse_program(index, val)) {
            printf("Opps, failed to program fuse %d\n", index);
            return CLI_FALSE;
        }
    } else if ((0 == strcmp(argv[0], "sense")) && (argc == 2)) {
        if (0 != fuse_sense(index, &val)) {
            printf("Opps, failed to sense fuse %d\n", index);
            return CLI_FALSE;
        } else {
            printf("fuse_%d = 0x%08x\n", index, (unsigned int)val);
        }
    } else {
        printf("usage example:\n\t fuse sense 144\n\tfuse prog 144 0x1234\n");
        return CLI_FALSE;
    }

    return CLI_TRUE;
}

CLI_CMD("fuse", "fuse prog/[sense] index [val]", cmd_fuse);
