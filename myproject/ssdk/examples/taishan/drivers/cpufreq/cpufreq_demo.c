#include <cmsis_os2.h>
#include <CLI_type.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <param.h>
#include <cpufreq_cfg.h>

static void cpufreq_usage(void)
{
    printf("Usage: cpufreq_demo {high,medium,low} \n");
}

static void printf_cpufreq(struct cpufreq *cpuf)
{
    printf("\n");

#ifdef CONFIG_PART_SF
    printf("SF frequency:       %d\n", cpuf->sf_freq);
#endif

#ifdef CONFIG_PART_SP
    printf("SP frequency:       %d\n", cpuf->sp_freq);
#endif

#ifdef CONFIG_PART_SX
    printf("SX frequency:       %d\n", cpuf->sx_freq);
#endif

    printf("AP bus frequency:   %d\n", cpuf->ap_bus);
    printf("Disp bus frequency: %d\n", cpuf->disp_bus);
    printf("Seip bus frequency: %d\n", cpuf->seip_bus);
    printf("\n");
}

static int cpufreq_demo_main(int argc, char *argv[])
{
    int ret;
    struct cpufreq cpuf_set;
    struct cpufreq cpuf_get;

    if (argc == 0) {
        cpufreq_usage();
        goto out;
    }

    if (!strcmp(argv[0], "high")) {
        cpuf_set = g_cpufreq_high;
    }
    else if (!strcmp(argv[0], "medium")) {
        cpuf_set = g_cpufreq_medium;
    }
    else if (!strcmp(argv[0], "low")) {
        cpuf_set = g_cpufreq_low;
    }
    else {
        cpufreq_usage();
        goto out;
    }

    ret = cpufreq_set(&cpuf_set);
    if (!ret) {
        printf("cpufreq set %s success\n", argv[0]);
    }
    else {
        printf("cpufreq set %s fail\n", argv[0]);
    }

out:
    cpufreq_get(&cpuf_get);
    printf_cpufreq(&cpuf_get);

    return CLI_FALSE;
}

CLI_CMD("cpufreq_demo", "\r\ncpufreq_demo:\r\n Change core frequency demo\r\n", cpufreq_demo_main);
