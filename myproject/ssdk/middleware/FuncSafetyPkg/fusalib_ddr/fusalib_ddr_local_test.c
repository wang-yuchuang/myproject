#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <debug.h>
#ifdef CONFIG_CLI
#include <CLI.h>
#endif
#include "../../../middleware/dw_umctl2/inc/dw_umctl2.h"
#include "../../../include/driver/kunlun/pfm/dw_pfm.h"
#include "../adapter/archEx.h"
#include "fusalib_ddr.h"
//#include "config.h"
#include "driver/dma.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof((a)[0]))
#endif

#ifndef ROUNDDOWN
#define ROUNDDOWN(a, b) ((a) & ~((b)-1))
#endif

#ifndef writell
#define writell(v, a)   (*(volatile uint64*)(a)) = (uint64) (v)
#endif

#ifndef readll
#define readll(a)       (*(volatile uint64*)(a))
#endif

extern void udelay(uint32_t usecs);
extern int readl_(addr_t reg);
extern int writel_(uint32_t v, addr_t reg);



/*
* bit 3 0x210 MAP4 31
* bit 4,5,6,7 0X208 MAP2
* bit 32 31 30 29  28  27  26  25  24  23  22  21 20 19 18 17 16 15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
*     c9 c8 c7 cs0 r16 r15 r14 r13 r12 r11 r10 r9 r8 r7 r6 r5 r4 r3 r2 r1 r0 b2 b1 b0 c6 c5 c4 c3 c2
*/
static int get_cs_bitmap(void)
{
    uint32_t addrmap0 = readl(APB_DDRCTRL_BASE + 0x200);
    int csbit = addrmap0 & 0x1f;

    if (csbit == 0 || csbit == 0x1f)
        return -1;

    return csbit + 8;
}
static int get_bank_bitmap(int bit)
{
    uint32_t v = readl(APB_DDRCTRL_BASE + 0x204); //addrmap1
    static uint8_t internal_base[3] = {2, 3, 4};
    static uint8_t shift[3] = {0, 8, 16};
    static uint8_t width[3] = {6, 6, 6};

    if (bit >= 0 && bit <= 2) { /*b0~b2*/
        v = ((v >> shift[bit]) & ((1 << width[bit]) - 1));

        if (v == (uint32_t)((1 << width[bit]) - 1)) //none
            return -1;

        return v + internal_base[bit] + 2;
    }

    return -1;
}
/*c2~c11*/
static int get_col_bitmap(int bit)
{
    uint32_t v = 0;
    static uint8_t internal_base[10] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
    static uint8_t shift[10] = {0, 8, 16, 24, 0, 8, 16, 24, 0, 8};
    static uint8_t width[10] = {4, 5, 4, 4, 5, 5, 5, 5, 5, 5};
    int i = 0;

    if (bit < 2 || bit > 11)
        return -1;

    i = bit - 2;

    if (bit >= 2 && bit <= 5) { /*c2~c5*/
        v = readl(APB_DDRCTRL_BASE + 0x208); //addrmap2
    }

    if (bit >= 6 && bit <= 9) { /*c6~c9*/
        v = readl(APB_DDRCTRL_BASE + 0x20c); //addrmap3
    }

    if (bit >= 10 && bit <= 11) { /*c10~c11*/
        v = readl(APB_DDRCTRL_BASE + 0x210); //addrmap4
    }

    v = ((v >> shift[i]) & ((1 << width[i]) - 1));

    if (v == (uint32_t)((1 << width[i]) - 1)) //none
        return -1;

    return v + internal_base[i];
}

/*r0~r17*/
static int get_row_bitmap(int bit)
{
    uint32_t v = 0;
    static uint8_t internal_base[18] = {6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    static uint8_t shift[18] = {0, 8, 0, 8, 16, 24, 0, 8, 16, 24, 0, 24, 0, 8, 16, 24, 0, 8};
    static uint8_t width[18] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    int i = 0;
    i = bit;

    if ((bit >= 0 && bit <= 1) || bit == 11) { /*r0~r1, r11*/
        v = readl(APB_DDRCTRL_BASE + 0x214); //addrmap5
    }

    if (bit >= 2 && bit <= 5) { /*r2~r5*/
        v = readl(APB_DDRCTRL_BASE + 0x224); //addrmap9
    }

    if (bit >= 6 && bit <= 9) { /*r6~r9*/
        v = readl(APB_DDRCTRL_BASE + 0x228); //addrmap10
    }

    if (bit == 10) { /*r10*/
        v = readl(APB_DDRCTRL_BASE + 0x22c); //addrmap11
    }

    if (bit >= 12 && bit <= 15) { /*r12~r15*/
        v = readl(APB_DDRCTRL_BASE + 0x218); //addrmap6
    }

    if (bit >= 16 && bit <= 17) { /*r16~r17*/
        v = readl(APB_DDRCTRL_BASE + 0x21c); //addrmap7
    }

    v = ((v >> shift[i]) & ((1 << width[i]) - 1));

    if (v == (uint32_t)((1 << width[i]) - 1)) //none
        return -1;

    return v + internal_base[i] + 2;
}

static uint32_t convert_ddr_to_addr(uint32_t row, uint32_t col,
                                    uint32_t bank, uint32_t cs, uint32_t *l, uint32_t *h)
{
    uint32_t addr = 0, addr_h = 0;
    int i = 0, bit = 0;
    /*cs*/
    bit = get_cs_bitmap();

    if ((cs & 0x1) && bit != -1) //cs0
        addr += 1 << bit;

    /*bank 0 ~ 2*/
    for (i = 0; i <= 2; i++) {
        if ((bank & (1 << i))) {
            bit = get_bank_bitmap(i);

            if (bit >= 0 && bit <= 31)
                addr += 1 << bit;
            else
                addr_h += 1 << (bit - 32);
        }
    }

    /* col c0~c1 */
    addr += (col & 0x3) << 2;
    addr -= 8;

    /* col c2~c11 */
    for (i = 2; i <= 11; i++) {
        if ((col & (1 << i))) {
            bit = get_col_bitmap(i);

            if (bit >= 0 && bit <= 31)
                addr += 1 << bit;
            else
                addr_h += 1 << (bit - 32);
        }
    }

    /*row 0 ~ 17*/
    for (i = 0; i <= 17; i++) {
        if ((row & (1 << i))) {
            bit = get_row_bitmap(i);
            if (bit >= 0 && bit <= 31)
                addr += 1 << bit;
            else
                addr_h += 1 << (bit - 32);
        }
    }

    *l = addr;
    *h = addr_h;
    return addr;
}
static uint32_t convert_addr_to_ddr(uint32_t h, uint32_t l, uint32_t *row,
                                    uint32_t *col,
                                    uint32_t *bank, uint32_t *cs)
{
    int i = 0;
    int bit = 0;

    /*row 0 ~ 17*/
    for (i = 0; i <= 17; i++) {
        bit = get_row_bitmap(i);

        if (bit >= 0 && bit <= 31 && (l & 1 << bit))
            (*row) += 1 << i;

        if (bit == 32 && (h == 1))
            (*row) += 1 << i;
    }

    /*col 2 ~ 11*/
    for (i = 2; i <= 11; i++) {
        bit = get_col_bitmap(i);

        if (bit >= 0 && bit <= 31 && (l & 1 << bit))
            (*col) += 1 << i;

        if (bit == 32 && (h == 1))
            (*col) += 1 << i;
    }

    /*bank 0 ~ 2*/
    for (i = 0; i <= 2; i++) {
        bit = get_bank_bitmap(i);

        if (bit >= 0 && bit <= 31 && (l & 1 << bit))
            (*bank) += 1 << i;

        if (bit == 32 && (h == 1))
            (*bank) += 1 << i;
    }

    /*cs*/
    bit = get_cs_bitmap();

    if (bit >= 0 && bit <= 31 && (l & 1 << bit))
        (*cs) += 1;

    if (bit == 32 && (h == 1))
        (*cs) += 1;

    return 0;
}


void ddr_report_error(void)
{
   uint32_t row     = readl(APB_DDRCTRL_BASE + 0xa4);
   uint32_t col     = readl(APB_DDRCTRL_BASE + 0xa8) & 0x7ff;
   uint32_t bank    = ((readl(APB_DDRCTRL_BASE + 0xa8) >> 16) & 0x7);
   uint32_t cs      = ((readl(APB_DDRCTRL_BASE + 0xa4) >> 24) & 1);
   uint32_t addr_l  = 0, addr_h = 0;
   convert_ddr_to_addr(row, col, bank, cs, &addr_l, &addr_h);

   if (addr_l >= 0xd0000000) { //overflow
        addr_l -= 0xd0000000;
        addr_h += 1;
   }
   else {
        addr_l += 0x30000000;
   }

   printf("ddr first error happend during access 0x%d%x-0x%d%x\n",addr_h,addr_l, addr_h, ROUNDDOWN(addr_l + 64, 64));
}


/*
* start_addr: address according dma view.
* size: currently size should be 8
*/
extern dma_dev_t dma1;
uint32_t dma_done_flag  =0u;

static void dma_transfer_done(uint32_t status, uint32_t param, void *context)
{
    if (DMA_COMPLETED == status)
    {
        dma_done_flag = 1;
    }
}

static int write_ddr_range_by_hw(uint32_t start_addr, uint32_t size, uint64_t *plnew_data, uint64_t *plold_data)
{
    uint32_t ddr_align=0u;
    struct dma_chan *dma_chan;

    ddr_align = ROUNDDOWN(start_addr, CACHE_LINE);
    //dma_chan=dma_req_chan(&dma1);
    dma_chan=dma_req_chan_with_id(&dma1,2);                 /*使用DMA1模块，通道2*/
    if (NULL == dma_chan)
    {
        printf("alloc channel failed.\r\n");
        return 0;
    }
    dma_chan->context   = NULL;
    dma_chan->irq_cb    = dma_transfer_done;
    if(plold_data!=NULL)
    {
        dma_prep_memcpy(dma_chan, (void *)plold_data, (void *)start_addr, size, DMA_INTERRUPT);
        dma_flush_cache((addr_t)ddr_align, CACHE_LINE);     /*操作地址需要32字节对齐*/
        dma_invalidate_cache((addr_t)plold_data, size);

        dma_done_flag=0u;
        dma_start(dma_chan);
        while (dma_done_flag!=1)
        {
        }
        dma_stop(dma_chan);
    }

    if(plnew_data!=NULL)
    {
        dma_prep_memcpy(dma_chan, (void *)start_addr, (void *)plnew_data, size, DMA_INTERRUPT);
        dma_flush_cache((addr_t)plnew_data, size);
        dma_invalidate_cache((addr_t)ddr_align, CACHE_LINE);/*操作地址需要32字节对齐*/

        dma_done_flag=0u;
        dma_start(dma_chan);
        while (dma_done_flag!=1)
        {
        }
        dma_stop(dma_chan);
    }
    dma_free_chan(dma_chan);
    return 0;
}



/*注意 old_data,new_data 和 testdata 需要64字节对齐*/
extern uint64_t  __ALIGNED(64) old_data;
extern uint64_t  __ALIGNED(64) new_data;
uint64_t  __ALIGNED(64) testdata[8];
/* a << b */
static uint64_t lshift(int a, int b)
{
	int i=0;
	uint64_t v=a;

	for(i=0;i<b;i++)
	{
		v=v*2;
	}
	return v;
}
/*
col[6:4]->col[9:7]
*/
static uint64_t convert_ddraddr_to_ecc_addr(uint64_t ddraddr)
{
    uint64_t ddr_align = ROUNDDOWN(ddraddr, 0x200);
    uint64_t ddr_mod512 = ddraddr - ddr_align;
    uint64_t ddr_mod64 = ddr_mod512 - ((ddr_mod512 / 64) * 64);
    uint64_t ddr_mod16 = ddr_mod64 - ((ddr_mod64 / 16) * 16);
    uint64_t ddr_mod8 = ddr_mod16 - ((ddr_mod16 / 8) * 8);
    uint64_t eccaddr = ddr_align;
    int bit = 0, bit2 = 0, isbitset = 0;

    eccaddr += ((ddr_mod512 / 64) * 8) + ((ddr_mod64 / 16) * 16) / 8 +
               ddr_mod16 / 8 + ddr_mod8 / 8;
    printf("align 0x%llx 0x%llx 0x%llx 0x%llx 0x%llx ddr 0x%llx\n", ddr_align,
           ddr_mod512, ddr_mod64, ddr_mod16, ddr_mod8, eccaddr);

    bit2 = get_col_bitmap(7);
    printf("bit7 %d\n", bit2);
    eccaddr |= lshift(1, bit2);
    bit2 = get_col_bitmap(8);
    printf("bit8 %d\n", bit2);
    eccaddr |= lshift(1,bit2);
    bit2 = get_col_bitmap(9);
    printf("bit9 %d\n", bit2);
    eccaddr |= lshift(1, bit2);

    bit = get_col_bitmap(7);
    bit2 = get_col_bitmap(4);

    if (bit != -1) {
        isbitset = ((ddraddr >> bit) & 1);
        eccaddr &= ~lshift(1, bit2);
        eccaddr |= lshift(isbitset, bit2);
    }

    bit = get_col_bitmap(8);
    bit2 = get_col_bitmap(5);

    if (bit != -1) {
        isbitset = ((ddraddr >> bit) & 1);
        eccaddr &= ~lshift(1, bit2);
        eccaddr |= lshift(isbitset, bit2);
    }

    bit = get_col_bitmap(9);
    bit2 = get_col_bitmap(6);

    if (bit != -1) {
        isbitset = ((ddraddr >> bit) & 1);
        eccaddr &= ~lshift(1, bit2);
        eccaddr |= lshift(isbitset, bit2);
    }
    printf("eccaddr 0x%llx \n", eccaddr);
    return eccaddr;
}


void fusalib_ddr_ecc_inject_err_dma(void)
{
    vaddr_t ddr_base = (vaddr_t)&testdata[0];

#ifdef WITH_KERNEL_VM
    vaddr_t base = (vaddr_t)paddr_to_kvaddr(APB_DDRCTRL_BASE);
    vaddr_t ddrtestbase = (vaddr_t)paddr_to_kvaddr(ddr_base);
#else
    vaddr_t base = (APB_DDRCTRL_BASE);
    vaddr_t ddrtestbase = (ddr_base);
#endif
    writel(0x71f, base + UMCTL2_REGS_ECCCTL_OFF);
    arch_disable_cache(UCACHE);
    printf("memset ddr test base 0x%x ~0x%x\n", ddrtestbase, ddrtestbase + 64);
    memset((void *)ddrtestbase, 0, 64);
    printf("inj 2bit err at 0x%x\n", ddrtestbase);
    uint64_t ddr = convert_ddraddr_to_ecc_addr(ddrtestbase - 0x30000000);
    ddr += 0x30000000;         /* change addr for dma 0x30000000 + 0x10000000*/
    printf("ddr 0x%llx\n", ddr);
    write_ddr_range_by_hw(ddr, 8, &new_data, &old_data);
    //todo: using R5 modify the eccdata;
    printf("out 0x%llx\n", old_data);
    printf("read back 0x%x\n", readl_(ddrtestbase));
    printf("check ecc stat 0x%x\n", readl(base + UMCTL2_REGS_ECCSTAT_OFF));
    write_ddr_range_by_hw(ddr, 8, &old_data, 0);

    writel(0x71f, base + UMCTL2_REGS_ECCCTL_OFF);
    printf("clear ecc stat 0x%x\n", (uint32_t)readl(base + UMCTL2_REGS_ECCSTAT_OFF));
    printf("read back 0x%x\n", (uint32_t)readl_(ddrtestbase));

    arch_enable_cache(UCACHE);
}


/*直接注入错误的方式*/
void fusalib_ddr_ecc_inject_err(void)
{
    vaddr_t base = (APB_DDRCTRL_BASE);
    vaddr_t ddrecctest = 0u;

    vaddr_t ddrtestbase = (vaddr_t)&testdata[0];
    printf("ddrtestbase=0x%x\n", ddrtestbase);

    writel(0x71f, base + UMCTL2_REGS_ECCCTL_OFF);
    arch_disable_cache(UCACHE);
    printf("memset ddr test base 0x%x ~0x%x\n", ddrtestbase, ddrtestbase + 64);
    //要求:ddr [ddrtestbase, ddrtestbase + 64] 没有被别的代码或变量占用

    memset((void *)ddrtestbase, 0, 64);
    printf("inj 2bit err at 0x%x\n", ddrtestbase);

    ddrecctest=(uint32_t)convert_ddraddr_to_ecc_addr((uint64_t)(ddrtestbase - 0x30000000));
    ddrecctest= ddrecctest + 0x30000000;                    //0x30000000为DDR address

    printf("ddr=0x%x\n", ddrecctest);
    old_data=(uint64_t)readll((volatile uint64*)ddrecctest);
    printf("old=0x%llx\n", old_data);
    writell(new_data,ddrecctest);
    printf("new=0x%llx\n", new_data);
    printf("read back 0x%x\n", (uint32_t)readl_(ddrtestbase));
    printf("check ecc stat 0x%x\n", (uint32_t)readl(base + UMCTL2_REGS_ECCSTAT_OFF));
    writell(old_data,ddrecctest);

    writel(0x71f, base + UMCTL2_REGS_ECCCTL_OFF);
    printf("clear ecc stat 0x%x\n", (uint32_t)readl(base + UMCTL2_REGS_ECCSTAT_OFF));
    printf("read back 0x%x\n", (uint32_t)readl_(ddrtestbase));

    arch_enable_cache(UCACHE);
}


static int fusalib_ddr_ecc_inject_err_cmd(int argc, char *argv[])
{
    fusalib_ddr_ecc_inject_err();
    return 0;
}

static int fusalib_ddr_ecc_inject_err_dma_cmd(int argc, char *argv[])
{
    fusalib_ddr_ecc_inject_err_dma();
    return 0;
}


void fusalib_ddr_ca_inject_err(void)
{
    RMWREG32(APB_DDRCTRL_BASE + UMCTL2_REGS_OCCAPCFG_OFF, 27, 1, 1);
}

static int fusalib_ddr_ca_inject_err_cmd(int argc, char *argv[])
{
    fusalib_ddr_ca_inject_err();
    return 0;
}

static uint8_t ddrmrr(uint32_t mr_num, uint32_t rank_bits)
{
    unsigned int  time_out = 5000;
    // exception
    writel(1,APB_DDRCTRL_BASE+0x00020060);
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    do
    {
        udelay(1);
    }while((readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF) & BM_MRSTAT_MR_WR_BUSY) && time_out--);
    if(!time_out){
        printf("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }

    writel(((mr_num & 0xFF)<<8),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL1_OFF);
    writel((0<<31)+(0<<30)+((mr_num&0xF)<<12)+(rank_bits<<4)+(1<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF);
    writel((1<<31)+(0<<30)+((mr_num&0xF)<<12)+(rank_bits<<4)+(1<<0),APB_DDRCTRL_BASE+UMCTL2_REGS_MRCTRL0_OFF);
    //Poll MRSTAT.mr_wr_busy until it is 0
    //0 mr_wr_busy : The Soc might initiate a MR write operation only if this signal is low
    time_out = 5000;
    do
    {
        udelay(1);
    }while((!(readl(APB_DDRCTRL_BASE+UMCTL2_REGS_MRSTAT_OFF)&BM_MRSTAT_MR_WR_BUSY)) && time_out--);
    if(!time_out){
        printf("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }
    // poll avild
    time_out = 5000;
    do
    {
        udelay(1);
    }while(((readl(APB_DDRCTRL_BASE+0x00020060) & 0x8) != 0x8) && time_out--);
    if(!time_out){
        printf("Opps, Running timeout please check mr_addr or rank_num.\n");
        return 0;
    }

    return (readl(APB_DDRCTRL_BASE+0x00020064)>>8) & 0xFF;
}

uint8_t dummy;
void fusalib_ddr_reg_par_inject_err(void)
{
    RMWREG32(APB_DDRCTRL_BASE + UMCTL2_REGS_REGPARCFG_OFF, 8, 1, 1);
    dummy = ddrmrr(5, 1);
}

static int fusalib_ddr_reg_par_inject_err_cmd(int argc, char *argv[])
{
    fusalib_ddr_reg_par_inject_err();
    return 0;
}

void fusalib_ddr_read_timeout_inject_err(void)
{
    pfm_enable_read_wdg(1);
}

static int fusalib_ddr_read_timeout_inject_err_cmd(int argc, char *argv[])
{
    fusalib_ddr_read_timeout_inject_err();
    return 0;
}

/*case 1 SM-DDR-01 Inline ECC for Data Protection*/
CLI_CMD("fusalib_ddr_ecc_inject_err",           "\r\nfusalib_ddr_ecc_inject_err\r\n",           fusalib_ddr_ecc_inject_err_cmd);
CLI_CMD("fusalib_ddr_ecc_inject_err_dma",       "\r\nfusalib_ddr_ecc_inject_err_dma\r\n",       fusalib_ddr_ecc_inject_err_dma_cmd);

/*case 3 SM-DDR-03 On-chip Command and Address Paths Protection*/
CLI_CMD("fusalib_ddr_ca_inject_err",            "\r\nfusalib_ddr_ca_inject_err\r\n",            fusalib_ddr_ca_inject_err_cmd);

/*case 4 SM-DDR-04 Configuration and Status Registers Parity Protection*/
CLI_CMD("fusalib_ddr_reg_par_inject_err",       "\r\nfusalib_ddr_reg_par_inject_err\r\n",       fusalib_ddr_reg_par_inject_err_cmd);

/*case 7 SM-DDR-07 Watchdog Timer for Read handshake*/
CLI_CMD("fusalib_ddr_read_timeout_inject_err",  "\r\nfusalib_ddr_read_timeout_inject_err\r\n",  fusalib_ddr_read_timeout_inject_err_cmd);
