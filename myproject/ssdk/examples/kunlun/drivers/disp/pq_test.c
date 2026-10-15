/**
 * @file pq_test.c
 * @brief display subsystem source file.
 *
 * @copyright Copyright (c) 2019-2029 Semidrive Semiconductor.
 *            All rights reserved.
 */
#include <sdm_display.h>
#include <string.h>
#include "CLI_type.h"

#define DISPLAY_NUM    4

static double strtof(const char *str)
{
    double s = 0.0;
    double d = 10.0;
    bool nflag = false;

    while (*str == ' ')
        str++;

    if (*str == '-') {
        nflag = true;
        str++;
    }

    if (!((*str >= '0') && (*str <= '9')))
        return s;

    while ((*str >= '0') && (*str <= '9') && (*str != '.')) {
        s = s * 10.0 + *str - '0';
        str++;
    }

    if (*str == '.')
        str++;

    while ((*str >= '0') && (*str <= '9'))
    {
        s = s + (*str - '0') / d;
        d *= 10.0;
        str++;
    }

    return s * (nflag ? -1.0 : 1.0);
}

static int pq_set(int argc, char *argv[])
{
    #define PI  3.14159

    display_handle *handle;
    struct sdm_pq_params pq;
    int id = 0;

    pq.contrast = 1; //k
    pq.luminance = 0; //l
    pq.saturation = 1; //s
    pq.chroma = 0; //h

    if (argc < 3) {
        printf("pq test\n");
        printf("pq [id] [-k] [-l] [-s] [-h] \n");
        printf("   -k: contrast\n");
        printf("   -l: luminance\n");
        printf("   -s: saturation\n");
        printf("   -h: chroma\n");
        return 0;
    }

    id = strtof(argv[0]);;
    if ((id > (DISPLAY_NUM - 1)) || (id < 0)) {
        id = 0;
        LOGE("id is illegal, using default 0.\n");
    }

    for (int i = 0; i < argc / 2; i++) {
        if (!strcmp(argv[1 + 2 * i], "-k"))
            pq.contrast = strtof(argv[1 + 2 * i + 1]);

        if (!strcmp(argv[1 + 2 * i], "-l"))
            pq.luminance = strtof(argv[1 + 2 * i + 1]);

        if (!strcmp(argv[1 + 2 * i], "-s"))
            pq.saturation = strtof(argv[1 + 2 * i + 1]);

        if (!strcmp(argv[1 + 2 * i], "-h"))
            pq.chroma = strtof(argv[1 + 2 * i + 1]) * PI / 180;
    }

    printf("k:%f, l:%f, s:%f, h:%f\n", pq.contrast, pq.luminance, pq.saturation, pq.chroma);
    handle = hal_get_display_handle(id);
    if (!handle) {
        LOGE("display:%d is invalid\n", id);
        return -1;
    }

    sdm_pq_set(handle, &pq);
    return 0;
}

CLI_CMD("pq", "set display panel param\r\n\tpq [id] [-k] [-l] [-s] [-h]", pq_set);