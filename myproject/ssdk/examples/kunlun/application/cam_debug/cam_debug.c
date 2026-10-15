#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "CLI_type.h"

#include "cam_hal.h"

int cam_strcmp(const char *cs, const char *ct)
{
    unsigned char c1, c2;

    while (1) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}

static int camera_debug(int argc,  char *argv[])
{
    if (argc == 0) {
        hal_cam_debug(CAM_SHOW, 0);
        return 0;
    }
    if (argc == 1) {
        if (!cam_strcmp(argv[1], "loge")) {
            hal_cam_debug(CAM_LOG, 1);
        }

        if (!cam_strcmp(argv[1], "logw")) {
            hal_cam_debug(CAM_LOG, 2);
        }

        if (!cam_strcmp(argv[1], "logi")) {
            hal_cam_debug(CAM_LOG, 3);
        }

        if (!cam_strcmp(argv[1], "logd")) {
            hal_cam_debug(CAM_LOG, 4);
        }
    }
    return 0;
}


CLI_CMD("cam_debug", "Start cam debug", camera_debug);
