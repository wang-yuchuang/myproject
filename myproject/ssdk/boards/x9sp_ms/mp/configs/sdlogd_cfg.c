/*
 * Copyright (c) 2021-2029 Semidrive Incorporated.
 * All rights reserved.
 * Software License Agreement.
 */

#ifdef CONFIG_BOARD_SDLOGD

#ifdef ARRAY_SIZE
#undef ARRAY_SIZE
#endif
#define ARRAY_SIZE(x)   (sizeof(x)/sizeof(x[0]))

void sdlogd_entry(int argc, char* argv[]);

void board_sdlogd_init(void)
{
    char* argv[] = {
        "sdlogd",
#ifdef CONFIG_SDLOGD_POLL_SAF
        "-o", "safety",
#endif
#ifdef CONFIG_SDLOGD_POLL_SEC
        "-o", "secure",
#endif
#ifdef CONFIG_SDLOGD_POLL_MP
        "-o", "mp",
#endif
#ifdef CONFIG_SDLOGD_POLL_AP1
        "-o", "ap1",
#endif
#ifdef CONFIG_SDLOGD_POLL_AP2
        "-o", "ap2",
#endif
        "-t", CONFIG_SDLOGD_POLL_INTERVAL_MS,
        "-l", CONFIG_SDLOGD_UPDATE_THRESHOLD,
    };
    int argc = ARRAY_SIZE(argv);
    sdlogd_entry(argc, argv);
}

#endif /* CONFIG_BOARD_SDLOGD */
/* EOF */
