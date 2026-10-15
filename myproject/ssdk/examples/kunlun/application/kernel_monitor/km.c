#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cmsis_os2.h>
#include <param.h>
#include "CLI_type.h"


void sdrvDumpTaskInfo(uint32_t interval, uint32_t count);
void sdvrTaskStateInfo(void);

struct km_options_t;
typedef int(*km_options_cb_t)(int,char *argv[],struct km_options_t*);

struct km_options_t
{
    const char* pattern;
    const char* help;
    km_options_cb_t opt_cb;
    struct km_options_t* subopt;
};

int km_heap_dump_cb(int argc, char *argv[],struct km_options_t* me)
{
    //heap_info();

    return 0;
}


int km_task_dump_cb(int argc, char *argv[],struct km_options_t* me)
{
    if(argc < 1)
    {
        sdvrTaskStateInfo();
    }
    else
    {
        return -1;
    }


    return 0;
}

struct km_options_t heap =
{
    "heap",
    "dump heap debug info & list.",
    km_heap_dump_cb,
    NULL,
};

struct km_options_t task =
{
    "task",
    "dump task runtime info. args:\nnone - show info\n(sr) [s] - set sample rate in s",
    km_task_dump_cb,
    NULL,
};

struct km_options_t* kmopt[] =
{
    &heap,
    &task,
};

int kernel_monitor(int argc, char *argv[])
{
    if(argc < 1)
    {
        printf("km [options]\noptions:\n");
        for(uint32_t i=0;i<sizeof(kmopt)/sizeof(kmopt[0]);i++)
        {
            printf("--------------------------\n");
            printf("%s : %s\n",kmopt[i]->pattern,kmopt[i]->help);
        }
    }
    else
    {
        uint32_t i = 0;

        for(i=0;i<sizeof(kmopt)/sizeof(kmopt[0]);i++)
        {
            if(!strcmp(argv[0],kmopt[i]->pattern))
            {
                if(kmopt[i]->opt_cb(argc-1,argv+1,kmopt[i]))
                {
                    printf("Invalid args of %s\n",kmopt[i]->pattern);
                    printf("--------------------------\n");
                    printf("%s : %s\n",kmopt[i]->pattern,kmopt[i]->help);
                }

                break;
            }
        }

        if(i==sizeof(kmopt)/sizeof(kmopt[0]))
        {
            printf("KM:Invalid cmd:%s\n",argv[0]);
        }

    }

    return 0;

}

int kernel_top_cmd(int argc, char *argv[])
{
    uint32_t interval_second = 1;
    uint32_t count = 1;

    if (argc == 1) {
        interval_second = strtoul(argv[0], NULL, 10);
    }

    if (argc == 2) {
        interval_second = strtoul(argv[0], NULL, 10);
        count = strtoul(argv[1], NULL, 10);
    }

    interval_second = MIN(interval_second, 5);
    count = MIN(count, 10);

    printf("top -d %d -n %d\n", interval_second, count);
    sdrvDumpTaskInfo(interval_second, count);

    return 0;
}

void dumpTaskStats(void);
int kernel_ps_cmd(int argc, char *argv[])
{
    dumpTaskStats();

    return 0;
}

CLI_CMD("km",  "kernel monitor tools", kernel_monitor);
CLI_CMD("top", "dump top stats\r\n\ttop [interval_second] [count]", kernel_top_cmd);
CLI_CMD("ps",  "dump task stats", kernel_ps_cmd);


