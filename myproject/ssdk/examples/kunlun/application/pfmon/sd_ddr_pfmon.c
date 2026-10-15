/*
 * sd_ddr_pfmon.c
 * Copyright (c) 2020 Semidrive Semiconductor.
 * All rights reserved.
 *
 * Description: Pfm driver.
 *
 **/

#include <reg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arch/cache.h>
#include <errno.h>
#include <debug.h>
#include <cmsis_os2.h>
#include <board.h>
#include "CLI_type.h"
#include "sd_ddr_pfmon.h"
#include <driver/kunlun/pfm/dw_pfm.h>


#define START_BY_CMD 1
#define PFM_RECORD_MASTER_ID_NUM_MAX 16

#define PFM_SEC_DIV_WINDOW_TIME 2 // window time 500ms, 1s/500ms = 2
#define PFM_WINDOW_TIME_MS 500 //window time 500ms

static uint32_t pfm_serial_print = 0;
static uint32_t pfm_serial_print_time = 0; //sec
static uint32_t pfm_print_name = 0;

#if PFM_RECORD_ON_DDR
static uint32_t pfm_dump_addr = 0;
static int pfm_dump_size = 0;
static int pfm_mem_full = 0;
static uint32_t rounds;
#endif
static int pfm_mem_auto_dump = 0;


struct pfm_record_info {
    uint32_t rd_cnt;
    uint32_t wr_cnt;
    uint32_t rd_bcntl;
    uint32_t wr_bcntl;
};

struct pfm_out_print_group_info {
    struct pfm_record_info rec_inf[PFM_SEC_DIV_WINDOW_TIME];
};

struct pfm_out_print_info {
    uint32_t is_fill;
    uint32_t write_index;
    struct pfm_out_print_group_info group_inf[PFM_RECORD_MASTER_ID_NUM_MAX];
};

struct pfm_out_print_info out_info[2];

static struct pfm_record_irq *current;

static osThreadAttr_t pfm_task_attr = {
    .name = "pfm_server_task",
    .stack_size = 4096,
    .priority = osPriorityNormal,
};
static osThreadId_t pfm_thread = NULL;
static void pfm_server(void *para);

pfm_observer_t def_observer[PFM_RECORD_MASTER_ID_NUM_MAX] = {
    {AP1,  0x7f,  AP1, 0x7f},  /* AP 3*/
    {AP2,  0x7f,  AP2, 0x7f},  /* AP 4*/
    {GPU1_OS1, 0x78, GPU1_OS1, 0x78},  /* GPU1 56*/
    {GPU2_OS1, 0x78, GPU2_OS1, 0x78},  /* GPU2 64*/
    {VPU1, 0x7f, VPU1, 0x7f,},  /* VPU1 29*/
    {VPU2, 0x7f, VPU2, 0x7f,},  /* VPU2 30*/
    {DP1, 0x7f, DP1, 0x7f},  /* DP1 23*/
    {DP2, 0x7f, DP2, 0x7f},  /* DP2 24*/
    {DP3, 0x7f, DP3, 0x7f},  /* DP3 25*/
    {DC1, 0x7f, DC1, 0x7f},  /* DC1 19*/
    {DC2, 0x7f, DC2, 0x7f},  /* DC2 20*/
    {DC3, 0x7f, DC3, 0x7f},  /* DC3 21*/
    {DC4, 0x7f, DC4, 0x7f},  /* DC4 22*/
    {CSI1, 0x7e, CSI3, 0x7f},  /* CSI 16,18*/
    {VDSP,  0x7f,  VDSP, 0x7f},  /* VDSP 5*/
    {0, 0, 0, 0} /* ALL */
};
extern const char *master_name[];

static uint32_t pfm_master_observer(uint32_t group, pfm_observer_t *observer)
{
    if ((group > PFM_RECORD_MASTER_ID_NUM_MAX) || (observer == NULL))
        return -1;
    pfm_set_axi_id0(group, (observer->master0)<<6, (observer->msk0)<<6);
    pfm_set_axi_id1(group, (observer->master1)<<6, (observer->msk1)<<6);

    return 0;
}

static uint32_t pfm_stop_trigger(uint32_t group, pfm_stop_condition_t *condition)
{
    if ((group > PFM_RECORD_MASTER_ID_NUM_MAX) || (condition == NULL))
        return -1;
    pfm_set_cnt_thr(group, condition->rd_thr, condition->wr_thr);
    pfm_set_bcnt_thr(group, condition->rd_bcnt_thr, condition->wr_bcnt_thr);

    return 0;
}

static uint32_t pfm_transaction_record(uint32_t nr_group, struct pfm_record_irq *record)
{
    uint32_t i = 0;
    struct pfm_out_print_info * info_temp = NULL;
    uint32_t write_index;

    if(out_info[0].is_fill == 0){
        info_temp = &out_info[0];
    }else if(out_info[1].is_fill == 0){
        info_temp = &out_info[1];
    }else{
        info_temp = NULL;
    }

    if (nr_group > 16){
        return -1;
    }

    arch_invalidate_cache_range(PFM_MON_AXI_RD_CNT(0), nr_group * 24);

#if PFM_RECORD_ON_DDR
    if (record == NULL){
        return -1;
    }
    if(info_temp != NULL){

        write_index = info_temp->write_index;

        for (i = 0; i < nr_group; i++)
        {
            record[i].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].rd_cnt = record[i].rd_cnt;
            record[i].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_cnt = record[i].wr_cnt;
            record[i].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
            info_temp->group_inf[i].rec_inf[write_index].rd_bcntl = record[i].rd_bcntl;
            record[i].rd_bcnth = readl(PFM_MON_AXI_RD_BCNTH(i));
            record[i].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_bcntl = record[i].wr_bcntl;
            record[i].wr_bcnth = readl(PFM_MON_AXI_WR_BCNTH(i));
        }

        info_temp->write_index = ((info_temp->write_index) + 1)%PFM_SEC_DIV_WINDOW_TIME;
        if(info_temp->write_index == 0){
            info_temp->is_fill = 1;
        }

    }else{
        for (i = 0; i < nr_group; i++)
        {
            record[i].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
            record[i].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
            record[i].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
            record[i].rd_bcnth = readl(PFM_MON_AXI_RD_BCNTH(i));
            record[i].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
            record[i].wr_bcnth = readl(PFM_MON_AXI_WR_BCNTH(i));
        }
    }
#else
    if(info_temp != NULL){

        write_index = info_temp->write_index;

        for (i = 0; i < nr_group; i++)
        {
            info_temp->group_inf[i].rec_inf[write_index].rd_cnt = readl(PFM_MON_AXI_RD_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_cnt = readl(PFM_MON_AXI_WR_CNT(i));
            info_temp->group_inf[i].rec_inf[write_index].rd_bcntl = readl(PFM_MON_AXI_RD_BCNTL(i));
            info_temp->group_inf[i].rec_inf[write_index].wr_bcntl = readl(PFM_MON_AXI_WR_BCNTL(i));
        }

        info_temp->write_index = ((info_temp->write_index) + 1)%PFM_SEC_DIV_WINDOW_TIME;
        if(info_temp->write_index == 0){
            info_temp->is_fill = 1;
        }

    }
#endif
    return 0;
}

#if PFM_RECORD_ON_DDR
static void pfm_save_config(pfm_head_t *pfm_head)
{
    pfm_head->magic = PFM_RECORD_MAGIC;
    pfm_head->time_window = readl(PFM_MON_BASE_CNT_CMP)/time_window_per_ms;
    pfm_head->group_nr = pfm_group_nr();
    for (uint32_t i = 0; i < pfm_head->group_nr; i++)
    {
        pfm_head->observer_config[i].master0 = readl(PFM_MON_AXI_ID0(i)) >> 6;
        pfm_head->observer_config[i].msk0 = readl(PFM_MON_AXI_ID0_MSK(i)) >> 6;
        pfm_head->observer_config[i].master1 = readl(PFM_MON_AXI_ID1(i)) >> 6;
        pfm_head->observer_config[i].msk1 = readl(PFM_MON_AXI_ID1_MSK(i))  >> 6;
    }

    for (uint32_t i = 0; i < pfm_head->group_nr && i < 8; i++)
    {
        pfm_head->condition[i].rd_thr = readl(PFM_MON_AXI_RD_THR(i));
        pfm_head->condition[i].wr_thr = readl(PFM_MON_AXI_WR_THR(i));
        pfm_head->condition[i].rd_bcnt_thr = readl(PFM_MON_AXI_RD_BCNT_THR(i));
        pfm_head->condition[i].wr_bcnt_thr = readl(PFM_MON_AXI_WR_BCNT_THR(i));
    }

    pfm_head->mode = readl(PFM_MON_MISC_INT_CTL) & 1;
    pfm_head->reach_thr = 0;
}

void pfm_dump_mem(const char* info, const void* content, uint32_t content_len)
{
    printf("%s: \n", info);
    uint32_t i = 0;
    uint32_t left_num = 0;

    left_num = content_len % 4;

    if(left_num != 0){
        printf("input content_len must 4 bytes rund \n");
        return;
    }

    for (i =0 ; i < content_len; ) {
        printf("%02x", *((uint8_t*)(content) + 1 + i));
        printf("%02x", *((uint8_t*)(content) + i));
        printf("%02x", *((uint8_t*)(content) + 3 + i));
        printf("%02x", *((uint8_t*)(content) + 2 + i));
        printf("\n");
        i = i + 4;
    }
}
#endif
static void pfm_irq_show(void)
{
    uint32_t group;
    //struct pfm_record_irq *record, *base;
    group = pfm_group_nr();
    if (group == 0)
    {
        printf("no channel enabled\n");
        return;
    }
#if PFM_RECORD_ON_DDR
    if (!current) {
        printf("no pfm record buffer specified\n");
        return;
    }
    if(pfm_mem_full == 0){
        pfm_transaction_record(group, current);
    }

    current += group;
    rounds++;

    if ((uint32_t)current >= (PFM_RECORD_BUFFER + PFM_RECORD_BUFFER_SIZE)) {
        pfm_mem_full = 1;
        current = (struct pfm_record_irq *)(PFM_RECORD_BUFFER + PFM_MON_HEAD_OFFSET);
    }
#else
    pfm_transaction_record(group, current);
#endif
}

static int pfm_enable(int argc, char *argv[])
{
    if (argc != 1) {
       printf("not enough arguments\n");
       return CLI_FALSE;
    }
    if (!strcmp(argv[0], "irq")) {
        printf("run pfm irq mode\n");
#if PFM_RECORD_ON_DDR
        current = (struct pfm_record_irq *)(PFM_RECORD_BUFFER + PFM_MON_HEAD_OFFSET);
        rounds = 0;
        pfm_head_t *head = (pfm_head_t *)(PFM_RECORD_BUFFER);
        head->start_time = 0;
        pfm_enable_irq_mode();
        pfm_reg_base_cnt_callback(pfm_irq_show);
        pfm_enable_counter();
#else
        pfm_enable_irq_mode();
        pfm_reg_base_cnt_callback(pfm_irq_show);
        pfm_enable_counter();
#endif
        printf("run PFM via irq mode\n");
        return CLI_FALSE;
    } else if (!strcmp(argv[0], "dma")) {
        pfm_enable_dma_mode();
        pfm_enable_counter();
        return CLI_FALSE;
    } else if(!strcmp(argv[0], "auto_dump")){
        pfm_mem_auto_dump = 1;
        return CLI_FALSE;
    }

    printf("usage: pfm_enable irq|dma|auto_dump \n");
    return CLI_FALSE;
}

int pfm_disable_internal(void)
{
    pfm_disable_counter();
#if PFM_RECORD_ON_DDR
    pfm_head_t *head = (pfm_head_t *)(PFM_RECORD_BUFFER);
    pfm_save_config(head);
    head->rounds = rounds;
    head->pool_size = PFM_RECORD_BUFFER_SIZE;
    head->record_offset = PFM_MON_HEAD_OFFSET;
    arch_clean_cache_range(PFM_RECORD_BUFFER, PFM_RECORD_BUFFER_SIZE);
#endif
    return 0;
}

static int pfm_disable(int argc, char *argv[])
{
    int ret = CLI_FALSE;
    ret = pfm_disable_internal();
    return ret;
}

static int pfm_time_window(int argc, char *argv[])
{
    char *end_ptr;
    if (argc != 1) {
        printf("bad arguments\n");
        return CLI_FALSE;
    }
    uint32_t time_window = strtoul(argv[0], &end_ptr, 10);
    if (*end_ptr || errno) {
        printf("pfm_window: invalid time_window = %d, errno = %d", time_window, errno);
        return CLI_FALSE;
    }
    pfm_cfg_time_window(time_window);
    return CLI_FALSE;
}

static int pfm_setup_group(int argc, char *argv[])
{
    char *end_ptr;
    if (argc != 1) {
        printf("bad arguments\n");
        return CLI_FALSE;
    }
    uint32_t group = strtoul(argv[0], &end_ptr, 10);
    if (group > PFM_RECORD_MASTER_ID_NUM_MAX || *end_ptr || errno) {
        printf("invalied group\n");
        return CLI_FALSE;
    }
    pfm_enable_group(group);
    return CLI_FALSE;
}

static int pfm_channel_configure(int argc, char *argv[])
{
    char *end_ptr;
    if (argc != 2) {
        printf("bad arguments\n");
        return CLI_FALSE;
    }
    uint32_t channel = strtoul(argv[0], &end_ptr, 10);
    if (channel >= (PFM_RECORD_MASTER_ID_NUM_MAX - 1)) {
        printf("channel %d over %d, last channel used to channel [ALL]\n",
            channel, PFM_RECORD_MASTER_ID_NUM_MAX - 1);
    }
    pfm_observer_t *observer = &def_observer[channel];
    observer->master0 = strtoul(argv[1], &end_ptr, 10);
    observer->msk0 = 0x7f;
    observer->master1 = strtoul(argv[1], &end_ptr, 10);
    observer->msk1 = 0x7f;
    uint32_t ret = pfm_master_observer(channel, observer);
    if (ret) {
        printf("failed to config channel\n");
        return CLI_FALSE;
    }
    return CLI_FALSE;
}

static int pfm_default(int argc, char *argv[])
{
    pfm_cfg_time_window(PFM_WINDOW_TIME_MS);
    //pfm_cfg_time_window(1);
    pfm_enable_group(16);

    for (uint32_t i = 0; i < 16; i++)
    {
        pfm_master_observer(i, &def_observer[i]);
    }
    pfm_stop_condition_t condition;
    condition.rd_thr = 0xffffffff;
    condition.wr_thr = 0xffffffff;
    condition.rd_bcnt_thr = 0xffffffff;
    condition.wr_bcnt_thr = 0xffffffff;
    for (uint32_t i = 0; i < 8; i++)
    {
        pfm_stop_trigger(i, &condition);
    }
    return CLI_FALSE;
}

static int pfm_channel_tigger(int argc, char *argv[])
{
    char *end_ptr;
    if (argc != 5) {
        printf("bad arguments\n");
        return CLI_FALSE;
    }
    pfm_stop_condition_t condition = {0};
    condition.rd_thr = strtoul(argv[1], &end_ptr, 10);
    condition.wr_thr = strtoul(argv[2], &end_ptr, 10);
    condition.rd_bcnt_thr = strtoul(argv[3], &end_ptr, 10);
    condition.wr_bcnt_thr = strtoul(argv[4], &end_ptr, 10);
    pfm_stop_trigger(strtoul(argv[0], &end_ptr, 10), &condition);
    return CLI_FALSE;
}
#if PFM_RECORD_ON_DDR
static int pfm_debug(int argc, char *argv[])
{
    pfm_head_t *head = calloc(1, sizeof(pfm_head_t));
    pfm_save_config(head);
    printf("dump config:\n");
    printf("group_nr:%u\n", head->group_nr);
    printf("time window: %u (ms)\n", head->time_window);
    for(uint32_t i = 0; i < 16; i++)
    {
        printf("channel %u, master0: %u\n", i, head->observer_config[i].master0);
        printf("channel %u, msk0: %x\n", i, head->observer_config[i].msk0);
        printf("channel %u, master1: %u\n", i, head->observer_config[i].master1);
        printf("channel %u, msk1: %x\n", i, head->observer_config[i].msk1);
    }
    for (uint32_t i = 0; i < 8; i++)
    {
        printf("channel %u, rd_thr: %x\n", i, head->condition[i].rd_thr);
        printf("channel %u, wr_thr: %x\n", i, head->condition[i].wr_thr);
        printf("channel %u, rd_bcnt_thr: %x\n", i, head->condition[i].rd_bcnt_thr);
        printf("channel %u, wr_bcnt_thr: %x\n", i, head->condition[i].wr_bcnt_thr);
    }

    printf("mode %u\n", head->mode);
    printf("rounds %u\n", rounds);
    free(head);
    return CLI_FALSE;
}


static int pfm_dump(int argc, char *argv[])
{
    int dump_size = 0;
    char *end_ptr;

    if (argc != 2) {
        printf("input argv error\n");
        return CLI_FALSE;
    }

    dump_size = strtoul(argv[1], &end_ptr, 10);

    if((dump_size > PFM_RECORD_BUFFER_SIZE)||(dump_size < 0)){
        printf("input dump len error\n");
        return CLI_FALSE;
    }else{
        pfm_dump_size = dump_size;
    }

    pfm_dump_addr = strtoul(argv[0], &end_ptr, 10);

    return CLI_FALSE;
}
#endif

static int pfm_print(int argc, char *argv[])
{
    char *end_ptr;

    if (pfm_thread == NULL) {
        pfm_thread = osThreadNew(pfm_server, NULL, &pfm_task_attr);
        if (pfm_thread == NULL) {
            ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
            return CLI_FALSE;
        }
    }

    if (argc == 1) {
        pfm_serial_print = strtoul(argv[0], &end_ptr, 10);
        pfm_serial_print_time = 0xffffffff;
    }else if(argc == 2){
        pfm_serial_print = strtoul(argv[0], &end_ptr, 10);
        pfm_serial_print_time = strtoul(argv[1], &end_ptr, 10);
    }else{
        printf("input argv error\n");
        return CLI_FALSE;
    }

    pfm_print_name = 1;

    return CLI_FALSE;
}

void pfm_print_name_line(void){
    printf("    %s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s  %14s\n",
        master_name[def_observer[0].master0], master_name[def_observer[1].master0],
        master_name[def_observer[2].master0], master_name[def_observer[3].master0],
        master_name[def_observer[4].master0], master_name[def_observer[5].master0],
        master_name[def_observer[6].master0], master_name[def_observer[7].master0],
        master_name[def_observer[8].master0], master_name[def_observer[9].master0],
        master_name[def_observer[10].master0], master_name[def_observer[11].master0],
        master_name[def_observer[12].master0], master_name[def_observer[13].master0],
        master_name[def_observer[14].master0], "ALL");
}

void pfm_print_value_line(void){
    uint32_t i = 0;
    struct pfm_out_print_info * info_temp = NULL;
    int rd_val = 0;
    int wr_val = 0;

    if(out_info[0].is_fill == 1){
        info_temp = &out_info[0];
    }else if(out_info[1].is_fill == 1){
        info_temp = &out_info[1];
    }else{
        info_temp = NULL;
    }

    if(info_temp != NULL){

        for(i = 0; i < PFM_RECORD_MASTER_ID_NUM_MAX; i++)
        {
            rd_val = ((info_temp->group_inf[i].rec_inf[0].rd_bcntl)>>20)+((info_temp->group_inf[i].rec_inf[1].rd_bcntl)>>20);
            wr_val = ((info_temp->group_inf[i].rec_inf[0].wr_bcntl)>>20)+((info_temp->group_inf[i].rec_inf[1].wr_bcntl)>>20);
            printf("%4d M %4d M : ", rd_val, wr_val);
        }
        printf("\n");
        info_temp->write_index = 0;
        info_temp->is_fill = 0;
    }
}

static void pfm_server(void *para)
{
    char *cmd = "irq";

    osDelay(3000); //wait for ddr init done
    pfm_default(0, NULL);
    pfm_enable(1, &cmd);

    printf("start pfm daemon service\n");

    while(1) {
#if PFM_RECORD_ON_DDR
        if(pfm_dump_size != 0){
            printf("input dump addr is =%x, len is =%d\n", pfm_dump_addr, pfm_dump_size);
            if(pfm_dump_addr == 0){
                pfm_dump_mem("begin dump", (void *)PFM_RECORD_BUFFER, pfm_dump_size);
            }else{
                pfm_dump_mem("begin dump", (void *)pfm_dump_addr, pfm_dump_size);
            }
            pfm_dump_size = 0;
        }else if((pfm_mem_full != 0)&&(pfm_mem_auto_dump == 1)){
            pfm_disable_internal();
            pfm_dump_mem("pfm mem is full, begin dump", (void *)PFM_RECORD_BUFFER, PFM_RECORD_BUFFER_SIZE);
            pfm_mem_full = 0;
            pfm_enable(1, &cmd);
        }
#endif
        if(pfm_serial_print == 1)
        {
            if(pfm_print_name == 1){
                printf("\n");
                //print name line
                pfm_print_name_line();
                pfm_print_name = 0;
            }

           //print value
           if(pfm_serial_print_time != 0){
                pfm_print_value_line();
                pfm_serial_print_time--;
           }
        }else{
            pfm_print_name = 1;
        }

        osDelay(500);
    }

    osThreadExit();
}

void start_pfm_service(void)
{
#if !START_BY_CMD
    pfm_thread = osThreadNew(pfm_server, NULL, &pfm_task_attr);
    if (pfm_thread == NULL) {
        ssdk_printf(SSDK_ALERT, "%s: create thread failed.\r\n", __func__);
        return;
    }
#endif
}

CLI_CMD("pfm_enable", "pfm_enable type, enable perf monitor\r\n\tpfm_enable irq|dma|auto_dump", pfm_enable);
CLI_CMD("pfm_disable", "disable perf monitor", pfm_disable);
CLI_CMD("pfm_window", "pfm time window(ms)\r\n\tpfm_window time_ms", pfm_time_window);
CLI_CMD("pfm_group", "ebable channel number\r\n\tpfm_group group", pfm_setup_group);
CLI_CMD("pfm_channel", "observer config\r\n\tpfm_channal cid mid", pfm_channel_configure);
CLI_CMD("pfm_trigger", "stop condition trigger\r\n\tpfm_trigger group rd wr rd_bcnt wr_bcnt", pfm_channel_tigger);
CLI_CMD("pfm_default", "pfm default observer", pfm_default);
#if PFM_RECORD_ON_DDR
CLI_CMD("pfm_debug",  "pfm debug", pfm_debug);
CLI_CMD("pfm_dump",  "pfm_dump addr size\r\n\tpfm_dump 0 16; dump from PFM_RECORD_BUFFER_START_ADDR 16 bytes", pfm_dump);
#endif
CLI_CMD("pfm_print",  "pfm_print 1 60; print on 60s", pfm_print);
